/*
 * Copyright 2019 Seungbin Song
 */


#include <iostream>
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

#include "version.h"
#include "options.h"

int main(int argc, char *const argv[]) {
  setup_gc_logging();

  AutoCompileContext autoPSDNContext(new PSDN::PSDNContext);
  auto& options = PSDN::PSDNContext::get().options();
  options.langVersion = CompilerOptions::FrontendVersion::P4_16;
  options.compilerVersion = PSDN_VERSION_STRING;

  if (options.process(argc, argv) != nullptr) {
    options.setInputFile();
  }
  if(::errorCount() > 0)
    return 1;

  auto hook = options.getDebugHook();

  const IR::P4Program *program = nullptr;
  const IR::ToplevelBlock *toplevel = nullptr;

  //Parse P4 program and run frontend
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
    std::cerr << err.what() << std::endl;
    return 1;
  }
  if (program == nullptr || ::errorCount() > 0) {
    std::cerr << "Failed to process P4 program on frontend." << std::endl;
    return 1;
  }

  P4::serializeP4RuntimeIfRequired(program, options);
  if(::errorCount() > 0) {
    std::cerr << "Failed to generate P4Runtime API." << std::endl;
    return 1;
  }

}














