/*
 * Copyright 2019 Seungbin Song
 */

#include "programStructure.h"

namespace PSDN {

  void ProgramStructureBuilder::postorder(const IR::ParameterList *paramList) {
    bool inAction = findContext<IR::P4Action>() != nullptr;
    unsigned index = 0;
    for (auto p : *paramList->getEnumerator()) {
      structure->actionParamMap.emplace(p, index);
      if (!inAction)
        structure->nonActionParams.emplace(p);
      index++;
    }   
  }

  void ProgramStructureBuilder::postorder(const IR::P4Action *action) {
    LOG2("discovered action " << action);
    auto control = findContext<IR::P4Control>();
    structure->actionMap.emplace(action, control);
  }

  void ProgramStructureBuilder::postorder(const IR::Declaration_Variable *decl) {
    structure->variables.push_back(decl);
  }

  void ProgramStructureBuilder::postorder(const IR::Type_Error *errors) {
    auto &map = structure->errorCodeMap;
    for (auto m : *errors->getDeclarations()) {
      BUG_CHECK(map.find(m) == map.end(), "Duplicate error");
      map[m] = map.size();
    }   
  }

  void ProgramStructureBuilder::postorder(const IR::Declaration_MatchKind* kind) {
    for (auto member : kind->members) {
      structure->matchKinds.insert(member->name);
    }   
  }

  /**
   * Find parser, pipe, and deparser
   */
  bool ProgramStructureBuilder::preorder(const IR::PackageBlock* package) {
    if (package->type->name != "SimpleSumeSwitch") {
      ::warning(ErrorType::WARN_INVALID, "%1%: the type name of main package should be"
          "SimpleSumeSwitch; are you using a different architecture?", package->type->name);
    }

    auto parser = package->getParameterValue(SUME_PARSER_PAR_NAME);
    auto pipe = package->getParameterValue(SUME_PIPE_PAR_NAME);
    auto deparser = package->getParameterValue(SUME_DEPARSER_PAR_NAME);
    structure->parser = parser->to<IR::ParserBlock>()->container;
    structure->pipe = pipe->to<IR::ControlBlock>()->container;
    structure->deparser = deparser->to<IR::ControlBlock>()->container;

    structure->pipelineControls.emplace(structure->pipe->name);
    structure->nonPipelineControls.emplace(structure->deparser->name);

    return false;
  }

}; // namespace PSDN
