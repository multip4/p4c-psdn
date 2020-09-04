/*
 * Copyright 2019 Seungbin Song
 */


#include <iostream>
#include <fstream>
#include <string>

#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/gc.h"
#include "lib/log.h"
#include "lib/nullstream.h"
#include "control-plane/p4RuntimeSerializer.h"
#include "frontends/common/applyOptionsPragmas.h"
#include "frontends/common/parseInput.h"
#include "frontends/p4/frontend.h"
#include "ir/ir.h"
#include "ir/json_loader.h"

#include "version.h"
#include "options.h"
#include "midend.h"
#include "backend.h"

#include "tableAnalyzer.h"

int main(int argc, char *const argv[]) {
  setup_gc_logging();

  AutoCompileContext autoPSDNContext(new PSDN::PSDNContext);
  auto& options = PSDN::PSDNContext::get().options();
  options.langVersion = CompilerOptions::FrontendVersion::P4_16;
  options.compilerVersion = PSDN_VERSION_STRING;

  if (options.process(argc, argv) != nullptr) {
    if (options.loadIRFromJson == false)
      options.setInputFile();
  }
  if(::errorCount() > 0)
    return 1;

  auto hook = options.getDebugHook();

  const IR::P4Program *program = nullptr;
  const IR::ToplevelBlock *toplevel = nullptr;

  //Parse P4 program and run frontend
  if (options.loadIRFromJson == false) {
    program = P4::parseP4File(options);
    if (program == nullptr || ::errorCount() > 0) {
      std::cerr << "Failed to parse P4 program." << std::endl;
      return 1;
    }
    try {
      P4::P4COptionPragmaParser pragmaParser;
      program->apply(P4::ApplyOptionsPragmas(pragmaParser));

      P4::FrontEnd frontend;
      frontend.addDebugHook(hook);
      program = frontend.run(options, program);
    } catch (const Util::P4CExceptionBase &err) {
      ::error(err.what());
      return 1;
    }
    if (program == nullptr || ::errorCount() > 0) {
      ::error("Failed to process P4 program on frontend.");
      return 1;
    }

  } else {
    //If IR is given, load IR.
    std::filebuf fb;
    if (fb.open(options.file, std::ios::in) == nullptr) {
      ::error("%s: No such file or directory.", options.file);
      return 1;
    }
    std::istream inJson(&fb);
    JSONLoader jsonFileLoader(inJson);
    if (jsonFileLoader.json == nullptr) {
      ::error("%s: Not a valid input file.", options.file);
      return 1;
    }
    program = new IR::P4Program(jsonFileLoader);
    fb.close();
  }

  P4::serializeP4RuntimeIfRequired(program, options);
  if(::errorCount() > 0) {
    ::error("Failed to generate P4Runtime API.");
    return 1;
  }

  PSDN::MidEnd midEnd(options);
  midEnd.addDebugHook(hook);
  try {
    toplevel = midEnd.process(program);
    if(::errorCount() > 1 || toplevel == nullptr || toplevel->getMain() == nullptr) {
      ::error("Failed to run midend optimizations.");
      return 1;
    }
    if (options.dumpJsonFile)
      JSONGenerator(*openFile(options.dumpJsonFile, true), true) << program << std::endl;
  } catch (const Util::P4CExceptionBase &err) {
    ::error(err.what());
    return 1;
  }
  if (::errorCount() > 0) {
    ::error("Failed to run midend optimizations.");
    return 1;
  }

  auto dda = new PSDN::TableAnalyzer(&midEnd.refMap, &midEnd.typeMap, options.file);


  auto backend = new PSDN::Backend(options, &midEnd.refMap, &midEnd.typeMap, &midEnd.enumMap);

  try {
    toplevel->getMain()->apply(*dda);
    backend->convert(toplevel);
  } catch (const Util::P4CExceptionBase &err) {
    ::error(err.what());
    return 1;
  }
  if (::errorCount() > 0) {
    ::error("Failed to run backends.");
    return 1;
  }

  return 0;
}














