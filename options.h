/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_OPTIONS_H
#define PSDN_OPTIONS_H

#include <getopt.h>
#include "frontends/common/options.h"

namespace PSDN {

class PSDNOptions : public CompilerOptions {
  public:
    //Output file
    cstring outputFile = nullptr;
    //Input file is IR in Json format
    bool loadIRFromJson = false;

    PSDNOptions () {
      registerOption("-o", "FILE",
          [this](const char* arg) { outputFile = arg; return true; },
          "Save output to FILE");
      registerOption("--fromJSON", "FILE",
          [this](const char* arg) { loadIRFromJson = true; file = arg; return true; },
          "Use IR representation from JSON file FILE dumped previously,"\
          "the compilation bypasses frontend.");
    }
};

using PSDNContext = P4CContextWithOptions<PSDNOptions>;

}; //namespace PSDN


#endif
