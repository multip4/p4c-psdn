/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_BACKEND_H
#define PSDN_BACKEND_H

#include "error.h"
#include "options.h"
#include "conversionContext.h"

#include "ir/ir.h"
#include "midend/convertEnums.h"
#include "frontends/p4/coreLibrary.h"


#include <iostream>
#include <fstream>

namespace PSDN {

class Backend {
  public:
    PSDNOptions& options;
    P4::ReferenceMap* refMap;
    P4::TypeMap* typeMap;
    P4::ConvertEnums::EnumMapping* enumMap;
    P4::P4CoreLibrary& corelib;
    const IR::ToplevelBlock* toplevel;

    // Conversion Context
    ConversionContext *ctxt;

    // Final output
    std::ofstream output;

    Backend(PSDNOptions& options, P4::ReferenceMap* refMap, 
        P4::TypeMap* typeMap, P4::ConvertEnums::EnumMapping* enumMap) :
      options(options), refMap(refMap), typeMap(typeMap), enumMap(enumMap),
      corelib(P4::P4CoreLibrary::instance) { 
        refMap->setIsV1(false); //Assume sume_switch, not a V1 model.

      // Add new error/warning types of PSDN Compiler
      auto errorCatalog = ErrorCatalog::getCatalog();
      errorCatalog.add(PSDN::PSDNErrorType::WARN_TABLE_KEY_PADDING, "table_key_padding", "");

    }
    void convert(const IR::ToplevelBlock* _toplevel);
};

}; //namespace PSDN

#endif
