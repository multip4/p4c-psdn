/*
 * Copyright 2019 Seungbin Song
 */

#include "backend.h"
#include "programStructure.h"
#include "lower.h"
#include "controlPolicy.h"

#include "lib/null.h"
#include "frontends/p4/simplify.h"
#include "frontends/p4/unusedDeclarations.h"

namespace PSDN {

void Backend::convert(const IR::ToplevelBlock* _toplevel) {
  CHECK_NULL(_toplevel);

  auto main = _toplevel->getMain();
  if (!main) { ::error("Cannot get main package"); return; }

  // Build program structure
  auto structure = new ProgramStructure();
  auto structureBuilder = new ProgramStructureBuilder(structure);

  main->apply(*structureBuilder);
  if(::errorCount() > 0) { ::error("Cannot generate program structure"); return; }

  // Apply Backend optimizations
  auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
  auto program = _toplevel->getProgram();

  PassManager simplify = {
    new P4::SynthesizeActions(refMap, typeMap,
        new PSDN::SkipControls(&structure->nonPipelineControls)),
    new P4::MoveActionsToTables(refMap, typeMap),
    new P4::TypeChecking(refMap, typeMap),
    new P4::SimplifyControlFlow(refMap, typeMap),
    new PSDN::LowerExpressions(typeMap),
    new P4::ConstantFolding(refMap, typeMap, false),
    new P4::TypeChecking(refMap, typeMap),
    new PSDN::RemoveComplexExpressions(refMap, typeMap,
        new PSDN::ProcessControls(&structure->pipelineControls)),
    new P4::SimplifyControlFlow(refMap, typeMap),
    new P4::RemoveAllUnusedDeclarations(refMap),
    evaluator,
    new VisitFunctor([this, evaluator, structure]() {
        toplevel = evaluator->getToplevelBlock(); }),
  };

  auto hook = options.getDebugHook();
  simplify.addDebugHook(hook);
  program->apply(simplify);
}

}; //namespace PSDN

