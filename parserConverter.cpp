/*
 * Copyright 2019 Seungbin Song
 */

#include "parserConverter.h"
#include "sdnetProgram.h"

namespace PSDN {

bool ParserConverter::convertStatement(const IR::StatOrDecl* s, SDNetSection& section) {
  if (s->is<IR::AssignmentStatement>()) {
    auto as = s->to<IR::AssignmentStatement>();
    //auto type = ctxt->typeMap->getType(as->left, true);
    auto l = econv->convert(as->left);
    auto r = econv->convert(as->right);
    section.methodUpdate.push_back(l + " = " + r);
  }
  else if (s->is<IR::MethodCallStatement>()) {
    auto mc = s->to<IR::MethodCallStatement>()->methodCall;
    int numArgs = mc->arguments->size();
    auto minst = P4::MethodInstance::resolve(mc, ctxt->refMap, ctxt->typeMap);
    if (minst->is<P4::ExternMethod>()) {
      auto m = minst->to<P4::ExternMethod>();

      // Convert extract method.
      if (m->method->name.name == corelib.packetIn.extract.name) {
        if (numArgs != 1) {
          ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
              "%1%: unknown # of arguments in extract method", mc);
          return true;
        }
        auto arg = mc->arguments->at(0);
        auto argtype = ctxt->typeMap->getType(arg->expression, true);
        if (!argtype->is<IR::Type_Header>()) {
          ::error(ErrorType::ERR_INVALID,
              "%1%: extract only accepts arguments with header types, not %2%",
              arg, argtype);
          return true;
        }
        if (auto mem = arg->expression->to<IR::Member>()) {
          auto baseType = ctxt->typeMap->getType(mem->expr, true);
          if (baseType->is<IR::Type_Stack>() || baseType->is<IR::Type_HeaderUnion>()) {
            ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                "%1%: type stack / header union is unsupported", mem->expr);
            return true;
          } else {
            std::cout << mem->member << std::endl;
            auto type = baseType->to<IR::Type_StructLike>();
            if (!type) {
              ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                  "%1%: type is not a structlike", mem->expr);
              return true;
            }
            auto memberType = type->getField(mem->member)->type;
            if (memberType->is<IR::Type_Stack>() || memberType->is<IR::Type_HeaderUnion>()) {
              ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                  "%1%: type stack / header union is unsupported", mem->member);
              return true;
            }
            auto mt = memberType->to<IR::Type_StructLike>();
            if (!mt) {
              ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                  "%1%: type is not a structlike", mem->expr);
              return true;
            }

            // Get struct definition and delete 'isValid'.
            auto str = std::string(hconv->getDefinition(mt,false));
            auto structDecl = cstring(str.replace(str.find("\tisValid : 1,\n"), 14, ""));
            section.structDecl += structDecl;

            // Add to method update.
            int size = 0;
            auto memberStr = mem->toString();
            section.methodUpdate.push_back(memberStr + "." + "isValid = 1");
            for (auto field : mt->fields) {
              auto fieldStr = field->toString();
              section.methodUpdate.push_back(memberStr + "." + fieldStr + " = " + fieldStr);
              auto fieldType = field->type->to<IR::Type_Bits>();
              size += fieldType->size;
            }
            // Increase extract size.
            section.methodUpdate.push_back("TopParser_extracts.size = (TopParser_extracts.size + "
              + std::to_string(size) + ")");
            section.methodIncrement = size;
          }
        }
      } // End of extract conversion.
    } // End of extern method.
  } // End of method call statement.
  return false;
}

bool ParserConverter::convertParams(const IR::Parameter* p) {
  // Expect type of p is packet_in, header, or metadata.
  auto sdnet = SDNetProgram();
  auto pt = ctxt->typeMap->getTypeType(p->type, true);
  if (pt->is<IR::Type_Extern>()) {
    auto packetInType = pt->to<IR::Type_Extern>();
    if (packetInType->name.name == "packet_in")
      return false;
    else {
      ::error(ErrorType::ERR_INVALID, "Extern object is not a packet_in.", packetInType);
      return true;
    }
  } else if (pt->is<IR::Type_StructLike>()) {
    auto st = pt->to<IR::Type_StructLike>();
    auto name = p->getName().name;
    std::ostringstream os;
    os << p->direction;
    cstring direction = os.str();
    auto body = hconv->getDefinition(st, false);
    tupleDef += sdnet.generateTuple(name + "_t", direction, body);
    tupleInst += name + "_t " + name + ";\n";
  } else {
    ::error(ErrorType::ERR_INVALID, "Parameter is not a packet_in or struct type.", pt);
    return true;
  }
  return false;
}

bool ParserConverter::preorder(const IR::P4Parser* parser) {
  if (!parser->type->is<IR::Type_Parser>()) {
    ::error(ErrorType::ERR_INVALID, "Parser is not a parser type.", parser);
    return false;
  }

  auto parserType = parser->type->to<IR::Type_Parser>();
  auto sdnet = SDNetProgram();
  unsigned maxPacketRegion = sdnet.getMaxPacketRegion(parserType);

  // Get parameters and change them into tuples.
  auto params = parser->getApplyParameters();
  for (auto p : params->parameters) {
    if(convertParams(p))
      return false;
  }
  // Add extract tuple.
  tupleDef += sdnet.generateTuple("TopParser_extracts_t", "out", "struct { size : 32 }");
  tupleInst += "TopParser_extracts_t TopParser_extracts;\n";
  

  // Convert local variables.
  std::vector<cstring> localDefs;
  if (!parser->parserLocals.empty()) {
    for (auto localvar : parser->parserLocals) {
      if (auto inst = localvar->to<IR::P4ValueSet>()) {
        auto bitwidth = inst->elementType->width_bits();
        auto name = inst->controlPlaneName();
        localDefs.push_back(cstring(name + " : " + std::to_string(bitwidth)));
        localvarMap.emplace(localvar, "local." + name);
      }
    }
    cstring localBody = "struct {\n";
    for (auto l : localDefs) {
      if (l == *(localDefs.rbegin()))
        localBody += "\t" + l + "\n}";
      else
        localBody += "\t" + l + ",\n";
    }
    tupleDef += sdnet.generateTuple("local_t","",localBody);
    tupleInst += "\tlocal_t local;\n";
  }

  // Convert parser states.
  auto numStates = parser->states.size();
  for (auto state : parser->states) {
    if (state->name == IR::ParserState::reject || state->name == IR::ParserState::accept) {
      SDNetSection section;
      section.name = state->name;
      sectionDef += section.emit();
    } else {
      // Convert statements.
      SDNetSection section;
      section.name = state->name;
      for (auto s : state->components) {
        if(convertStatement(s, section))
          return false;
      }
      sectionDef += section.emit();
    }
  }
  
  std::cout << emitParser() << std::endl;

  return false;
}

cstring ParserConverter::emitParser() {
  return tupleDef + tupleInst + sectionDef;
}

}; //namespace PSDN
