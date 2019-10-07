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

cstring ControlConverter::convertTable(const CFG::TableNode* node) {
  cstring tableResult = "";
  auto table = node->table;

  // Get key information.
  auto key = table->getKey();
  std::vector<unsigned> keyWidths;
  
  if (key != nullptr) {
    for (auto ke : key->keyElements) {
      auto expr = ke->expression;
      auto type = ctxt->typeMap->getType(expr, true);

      // Get the number of bits
      if (type->is<IR::Type_Bits>()) {
        auto tb = type->to<IR::Type_Bits>();
        keyWidths.push_back(tb->size);
      } else if (type->is<IR::Type_Boolean>()) {
        keyWidths.push_back(1);
      } else {
        ::error(ErrorType::ERR_UNSUPPORTED, "%1%: unsupported key type %2%. "
            "Supported key types are bit<> or boolean.", expr, type);
      }

      // All match types should be same.
      auto firstMatchType = getKeyMatchType(*(key->keyElements.begin()));
      auto matchType = getKeyMatchType(ke);
      if (matchType != "" && matchType != firstMatchType)
        ::error(ErrorType::ERR_UNSUPPORTED, "%1%: matchType should be same.", ke);
    }
  }

  std::cout << "Key widths of table " << table->name.toString() << std::endl;
  for (auto kw : keyWidths)
    std::cout << kw << " ";
  std::cout << std::endl;

  // Get action information.
  auto actionList = table->getActionList();

  if (actionList != nullptr) {
    for (auto ae : actionList->actionList) {
      auto decl = ctxt->refMap->getDeclaration(ae->getPath(), true);
      BUG_CHECK(decl->is<IR::P4Action>(), "%1%: should be an action name", ae);
      auto action = decl->to<IR::P4Action>();

      cstring structDecl = "struct " + action->name.toString() + "_0_cp {";
      bool addComma = false;
      for (auto param : action->getParameters()->parameters) {
        auto name = param->name;
        auto type = param->type;

        // Get the number of bits
        int typeWidth = ctxt->typeMap->minWidthBits(type, param);

        if (addComma)
          structDecl += ",";
        structDecl += "\n\t" + name.toString() + " : " + std::to_string(typeWidth);
        addComma = true;
      }
      structDecl += "\n}";
    std::cout << structDecl << std::endl;
    }
  }

  // If there is no key, make request tuple with 1.
  

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
