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
  

  return false;
}

cstring ParserConverter::emitParser() {
  return classDef + tupleDef + tupleInst;
}

}; //namespace PSDN
