/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_MIDEND_H
#define PSDN_MIDEND_H

#include "ir/ir.h"
#include "midend/convertEnums.h"

namespace PSDN {

class PSDNMidEnd : public PassManager {
  public:
    P4::ReferenceMap refMap;
    P4::TypeMap typeMap;
    const IR::ToplevelBlock *toplevel = nullptr;
    P4::ConvertEnums::EnumMapping enumMap;

    const IR::ToplevelBlock* process(const IR::P4Program *&program);
    PSDNMidEnd(CompilerOptions& options);
};

}; //namespace PSDN

#endif
