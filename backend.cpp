/*
 * Copyright 2019 Seungbin Song
 */

#include "backend.h"
#include "programStructure.h"
#include "lower.h"
#include "annotationParser.h"
#include "controlPolicy.h"
#include "expressionConverter.h"
#include "headerConverter.h"
#include "parserConverter.h"
#include "controlConverter.h"

#include "lib/null.h"
#include "lib/cstring.h"
#include "frontends/p4/simplify.h"
#include "frontends/p4/unusedDeclarations.h"
#include "frontends/p4/parseAnnotations.h"

namespace PSDN {

void Backend::convert(const IR::ToplevelBlock* _toplevel) {
  CHECK_NULL(_toplevel);

  // Get main & build program structure
  auto main = _toplevel->getMain();
  if (!main) { ::error("Cannot get main"); return; }

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
    new P4::ParseAnnotationBodies(new AnnotationParser(), typeMap),
    evaluator,
    new VisitFunctor([this, evaluator, structure]() {
        toplevel = evaluator->getToplevelBlock(); }),
  };

  auto hook = options.getDebugHook();
  simplify.addDebugHook(hook);
  program->apply(simplify);

  if(::errorCount() > 0) { ::error("Cannot apply backend optimizations"); return; }

  // Map IR node to resource blocks
  toplevel->apply(*new PSDN::ResourceMapBuilder(&structure->resourceMap));
  if(::errorCount() > 0) { ::error("Cannot generate resource map"); return; }

  // Get main & build program structure
  main = toplevel->getMain();
  if (!main) { ::error("Cannot get main"); return; }
  main->apply(*structureBuilder);
  program = toplevel->getProgram();
  program->apply(*structureBuilder);
  if(::errorCount() > 0) { ::error("Cannot generate program structure"); return; }

  // Generate conversion context
  ctxt = new ConversionContext(refMap, typeMap, toplevel, structure);

  // Open filestream
  output.open(options.outputFile);

  // Convert enums into constant
  // 
  // enum X { a, b };
  //
  // const ENUM_X_a = 0;
  // const ENUM_X_b = 1;
  for (const auto &enums : *enumMap) {
    std::map<cstring, cstring>* enumString  = new std::map<cstring, cstring>;
    auto enumName = enums.first->getName();
    for (const auto &enumEntry : *enums.second) {
      cstring str =  "ENUM_" + enumName + "_" + enumEntry.first;
      output << "const " << str << " = " << enumEntry.second << ";" << std::endl;
      enumString->emplace(enumEntry.first, str.c_str()); 
    }
    ctxt->enumStringMap.emplace(enums.first, enumString);
  }
  output << std::endl;

  // Convert error codes into constant
  //
  // error { a, b };
  //
  // const ERROR_a = 1;
  // const ERROR_b = 2;
  for (const auto &errorCode : structure->errorCodeMap) {
    auto name = errorCode.first->toString();
    cstring str = "ERROR_" + name;
    output << "const " << str << " = " << errorCode.second << ";" << std::endl;
    ctxt->errorStringMap.emplace(errorCode.first, str);
  }
  output << std::endl;

  auto econv = new ExpressionConverter(refMap, typeMap, structure, ctxt);

  auto hconv = new HeaderConverter(ctxt);
  program->apply(*hconv);

  output << hconv->emitTypeDef() << std::endl;

  auto pconv = new ParserConverter(ctxt, hconv, econv);
  program->apply(*pconv);

  output << pconv->emitParser() << std::endl;

  auto cconv = new ControlConverter(ctxt, hconv, econv);



  output.close();

}

}; //namespace PSDN

