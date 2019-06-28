/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_BACKEND_H
#define PSDN_BACKEND_H

#include "options.h"

#include "ir/ir.h"
#include "midend/convertEnums.h"
#include "frontends/p4/coreLibrary.h"

namespace PSDN {

class Backend {
  public:
    PSDNOptions& options;
    P4::ReferenceMap* refMap;
    P4::TypeMap* typeMap;
    P4::ConvertEnums::EnumMapping* enumMap;
    P4::P4CoreLibrary& corelib;
    const IR::ToplevelBlock* toplevel;

    Backend(PSDNOptions& options, P4::ReferenceMap* refMap, 
        P4::TypeMap* typeMap, P4::ConvertEnums::EnumMapping* enumMap) :
      options(options), refMap(refMap), typeMap(typeMap), enumMap(enumMap),
      corelib(P4::P4CoreLibrary::instance) { 
        refMap->setIsV1(false); //Assume sume_switch, not a V1 model.
    }
    void convert(const IR::ToplevelBlock* toplevel);
};

}; //namespace PSDN

#endif
