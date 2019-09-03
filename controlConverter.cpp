/*
 * Copyright 2019 Seungbin Song
 */

#include "controlConverter.h"

#include "controlFlowGraph.h"

namespace PSDN {

cstring ControlConverter::getKeyMatchType(const IR::KeyElement *ke) {
  auto path = ke->matchType->path;
  auto mt = ctxt->refMap->getDeclaration(path, true)->to<IR::Declaration_ID>();
  BUG_CHECK(mt != nullptr, "%1%: could not find declaration", ke->matchType);

  if (mt->name.name == corelib.exactMatch.name)
    return "EM";
  if (mt->name.name == corelib.ternaryMatch.name)
    return "TCAM";
  if (mt->name.name == corelib.lpmMatch.name)
    return "LPM";
  if (ctxt->structure->matchKinds.count(mt->name.name)) {
    if (mt->name.name == "direct")
      return "DIRECT";
    return mt->name.name;
  }   

  ::error(ErrorType::ERR_UNSUPPORTED, "match type not supported on this target", mt);
  return "INVALID";
}

cstring ControlConverter::convertKey(const IR::Key* key) {
  cstring keyResult = "";
  cstring tableMatchType = "";

  for (auto ke : key->keyElements) {
    auto expr = ke->expression;
    auto type = ctxt->typeMap->getType(expr, true);
    if (!type->is<IR::Type_Bits>() && !type->is<IR::Type_Boolean>())
      ::error(ErrorType::ERR_UNSUPPORTED, "%1%: unsupported key type %2%. "
          "Supported key types are bit<> or boolean.", expr, type);

    auto matchType = getKeyMatchType(ke);
    if (tableMatchType != "" && tableMatchType != matchType)
      ::error(ErrorType::ERR_UNSUPPORTED, "%1%: matchType should be same.", ke);
    tableMatchType = matchType;
  }
  return keyResult;
}

cstring ControlConverter::convertTable(const CFG::TableNode* node) {
  cstring tableResult = "";
  auto table = node->table;
  auto key = table->getKey();
  if (key != nullptr)
    tableResult += convertKey(key);

  return tableResult;
}


bool ControlConverter::preorder(const IR::P4Control* control) {
  auto cfg = new CFG();
  cfg->build(control, ctxt->refMap, ctxt->typeMap);
  bool success = cfg->checkImplementable();
  if (!success) {
    ::error(ErrorType::ERR_UNSUPPORTED, "Control-flow graph is complicated", control);
    return false;
  }

  std::set<const IR::P4Table*> done;

  for (auto node :cfg->allNodes) {
    if (node->is<CFG::TableNode>()) {
      auto tableNode = node->to<CFG::TableNode>();
      if (done.find(tableNode->table) != done.end())
        continue;
      done.emplace(tableNode->table);

      result += convertTable(tableNode);
      
    } else if (node->is<CFG::IfNode>()) {
      auto ifNode = node->to<CFG::IfNode>();
    }
  }

  return false;
}

} // namespace PSDN
