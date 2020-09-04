/*
 * Copyright 2019 Seungbin Song
 */

#include "controlConverter.h"

#include "controlFlowGraph.h"
#include "error.h"

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
  
  cstring externalStructDecl = "";
  auto table = node->table;

  // Get key information.
  auto key = table->getKey();
  unsigned keyWidth = 0;
  std::vector<unsigned> keyWidths;
  cstring matchType = "INVALID";

  if (key != nullptr) {
    for (auto ke : key->keyElements) {
      auto expr = ke->expression;
      auto type = ctxt->typeMap->getType(expr, true);

      // Get the number of bits
      if (type->is<IR::Type_Bits>()) {
        auto tb = type->to<IR::Type_Bits>();
        keyWidths.push_back(tb->size);
        keyWidth += tb->size;
      } else if (type->is<IR::Type_Boolean>()) {
        keyWidths.push_back(1);
        keyWidth += 1;
      } else {
        ::error(ErrorType::ERR_UNSUPPORTED, "%1%: unsupported key type %2%. "
            "Supported key types are bit<> or boolean.", expr, type);
      }

      // All match types should be same.
      auto firstMatchType = getKeyMatchType(*(key->keyElements.begin()));
      matchType = getKeyMatchType(ke);
      if (matchType != "" && matchType != firstMatchType)
        ::error(ErrorType::ERR_UNSUPPORTED, "%1%: matchType should be same.", ke);
    }
  }


  // Get action information.
  auto actionList = table->getActionList();
  std::vector<cstring> responseValues;
  unsigned valueWidth = 0;

  if (actionList != nullptr) {
    for (auto ae : actionList->actionList) {
      auto decl = ctxt->refMap->getDeclaration(ae->getPath(), true);
      BUG_CHECK(decl->is<IR::P4Action>(), "%1%: should be an action name", ae);
      auto action = decl->to<IR::P4Action>();
      auto params = action->getParameters();

      if (!params->empty()) {
        cstring structDecl = "struct " + action->name.name + "_0_cp {";
        responseValues.push_back(action->name.toString() + "_0");
        bool addComma = false;
        for (auto param : params->parameters) {
          auto name = param->name;
          auto type = param->type;

          // Get the number of bits
          unsigned typeWidth = ctxt->typeMap->minWidthBits(type, param);
          valueWidth += typeWidth;

          if (addComma)
            structDecl += ",";
          structDecl += "\n\t" + name.toString() + " : " + std::to_string(typeWidth);
          addComma = true;
        }
        structDecl += "\n}";
        externalStructDecl += structDecl;
      }
    }
  }

  // Cases of table conversion:
  // 1. If there is no key or no action, convert nothing.
  // 2. If there are keys only, generate lookup engine and make a new
  //      tuple engine.
  // 3. If there are keys and combined actions, generate a lookup engine 
  //      and make a new tuple engine.
  // 4. If there are actions with parameters, generate a lookup engine 
  //      (without hit) to get parameters from control plane.
  // 5. If there are actions with no parameter, append the current tuple 
  //      engine. In this case, the table is originally come from 
  //      explicit instructions in the control.
  

  if (key != nullptr && !key->keyElements.empty()) {
    // Generate request tuple
    cstring requestTupleName = table->getName().name + "_req_t";
    cstring requestTupleBody = "struct {\n";

    // If the total key width is less than 12 bits, add padding.
    if (keyWidth < 12) {
      requestTupleBody += "\tlookup_request_padding : " + std::to_string(12-keyWidth) + ",\n";
      ::warning(PSDNErrorType::WARN_TABLE_KEY_PADDING,
          "Table %1% key pre-padded with %2% bits",
          table, 12-keyWidth);
    }

    for (unsigned i = 1; i <= keyWidths.size(); i++) {
      requestTupleBody += "\tlookup_request_key_" + std::to_string(i) 
        + " : " + std::to_string(keyWidths[i-1]);
      if (i != keyWidths.size())
        requestTupleBody += ",";
      requestTupleBody += "\n";
    }
    requestTupleBody += "}";

    // Generate response tuple
    cstring responseTupleName = table->getName().name + "_resp_t";
    cstring responseTupleBody = "struct {\n";
    responseTupleBody += "\thit : 1,\n";

    // Calculate action_run bits
    unsigned actionRunBits = 1;
    if (actionList != nullptr && !actionList->actionList.empty()) {
      unsigned actionNumber = actionList->actionList.size();
      while (actionNumber >>= 1) actionRunBits++;
    }
    responseTupleBody += "\taction_run : " + std::to_string(actionRunBits);

    for (auto rv : responseValues)
      responseTupleBody += ",\n\t" + rv + " : " + rv + "_cp";
    responseTupleBody += "\n}";


    // Generate Lookup engine
    auto lookupEngine = new SDNet::LookupEngine();
    lookupEngine->name = table->name + "_t";
    lookupEngine->matchType = matchType;
    lookupEngine->keyWidth = (keyWidth < 12) ? 12 : keyWidth;
    lookupEngine->valueWidth = valueWidth + actionRunBits;


    lookupEngine->requestTuple = new SDNet::Tuple(requestTupleName, "in", requestTupleBody);
    lookupEngine->responseTuple = new SDNet::Tuple(responseTupleName, "out", responseTupleBody);

    std::cout << lookupEngine->emit() << std::endl;

    lookupEngines.push_back(lookupEngine);
    
    // Makeup Tuple engine


  } else if (actionList != nullptr && !actionList->actionList.empty()) {

  }

  return "";
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

  // Make the first (empty) tuple engine
  auto firstTE = new SDNet::TupleEngine();
  firstTE->name = control->name.name + "_Start";
  tupleEngines.push_back(firstTE);

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
