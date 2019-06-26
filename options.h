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

    PSDNOptions () {
      registerOption("-o", "FILEPATH",
          [this](const char* arg) { outputFile = arg; return true; },
          "Specify output file path as FILEPATH");
    }
};

using PSDNContext = P4CContextWithOptions<PSDNOptions>;

}; //namespace PSDN


#endif
