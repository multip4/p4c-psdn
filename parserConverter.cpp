/*
 * Copyright 2019 Seungbin Song
 */

#include "parserConverter.h"
#include "sdnetProgram.h"

namespace PSDN {

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
    // Expect type of p is packet_in, header, or metadata.
    auto pt = ctxt->typeMap->getTypeType(p->type, true);
    if (pt->is<IR::Type_Extern>()) {
      auto packetInType = pt->to<IR::Type_Extern>();
      if (packetInType->name.name == "packet_in")
        continue;
      else {
        ::error(ErrorType::ERR_INVALID, "Extern object is not a packet_in.", packetInType);
        return false;
      }
    } else if (pt->is<IR::Type_StructLike>()) {
      auto st = pt->to<IR::Type_StructLike>();
      auto name = p->getName().name;
      std::ostringstream os;
      os << p->direction;
      cstring direction = os.str();
      auto body = hconv->getDefinition(st, false);
      tupleDef += sdnet.generateTuple(name + "_t", direction, body);
      tupleInst += "\t" + name + "_t " + name + ";\n";
    } else {
      ::error(ErrorType::ERR_INVALID, "Parameter is not a packet_in or struct type.", pt);
      return false;
    }
  }

  // Convert local variables.
  std::vector<cstring> localDefs;
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

  // TODO: Convert parser states.
  auto numStates = parser->states.size();
  for (auto state : parser->states) {
    if (state->name == IR::ParserState::reject) {
      cstring str = "class reject::Section(" + std::to_string(numStates) + ") {\n";
      str += "\tmethod move_to_section = done(0);\n";
      str += "\tmethod increment_offset = 0;\n";
      str += "}\n";
      classDef += sdnet.addIndent(str);
    } else if (state->name == IR::ParserState::accept) {
      cstring str = "class accept::Section(" + std::to_string(numStates+1) + ") {\n";
      str += "\tmethod move_to_section = done(0);\n";
      str += "\tmethod increment_offset = 0;\n";
      str += "}\n";
      classDef += sdnet.addIndent(str);
    } else {
      // Convert statements.
      SDNetSection section;
      for (auto s : state->components) {
        if (s->is<IR::AssignmentStatement>()) {
          auto as = s->to<IR::AssignmentStatement>();
          auto type = ctxt->typeMap->getType(as->left, true);
          auto l = econv->convert(as->left);
          auto r = econv->convert(as->right);
          section.methodUpdate += l + " = " + r + ";\n";
          std::cout << section.methodUpdate << std::endl;
        }
        else if (s->is<IR::MethodCallStatement>()) {
          auto mc = s->to<IR::MethodCallStatement>()->methodCall;
          int numArgs = mc->arguments->size();
          auto minst = P4::MethodInstance::resolve(mc, ctxt->refMap, ctxt->typeMap);
          if (minst->is<P4::ExternMethod>()) {
            auto m = minst->to<P4::ExternMethod>();
            if (m->method->name.name == corelib.packetIn.extract.name) {
              if (numArgs != 1) {
                ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                    "%1%: unknown # of arguments in extract method", mc);
                return false;
              }
              auto arg = mc->arguments->at(0);
              auto argtype = ctxt->typeMap->getType(arg->expression, true);
              if (!argtype->is<IR::Type_Header>()) {
                ::error(ErrorType::ERR_INVALID,
                    "%1%: extract only accepts arguments with header types, not %2%",
                    arg, argtype);
                return false;
              }
              if (auto mem = arg->expression->to<IR::Member>()) {
                auto baseType = ctxt->typeMap->getType(mem->expr, true);
                if (baseType->is<IR::Type_Stack>()) {
                  ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                      "%1%: type stack is unsupported", mem->expr);
                } else if (baseType->is<IR::Type_HeaderUnion>()) {
                  ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET,
                      "%1%: header union is unsupported", mem->expr);
                } else {
                  std::cout << mem->member << std::endl;
                }
              }

            }
          }
        }
      }

    }
  }
  

  return false;
}

cstring ParserConverter::emitParser() {
  return classDef + tupleDef + tupleInst;
}

}; //namespace PSDN
