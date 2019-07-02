/*
 * Copyright 2019 Seungbin Song
 *
 */

#ifndef PSDN_CONVERSION_CONTEXT_H
#define PSDN_CONVERSION_CONTEXT_H

#include <cstring>

#include "ir/ir.h"

#include "programStructure.h"

namespace PSDN {


/**
 * Conversion context from P4 IR to SDNet
 */
class ConversionContext {
public:
  P4::ReferenceMap* refMap;
  P4::TypeMap* typeMap;
  
  const IR::ToplevelBlock* toplevel;
  
  ProgramStructure* structure;

  // Constant string maps
  std::map<const IR::Type_Enum*, std::map<cstring, cstring>*> enumStringMap;
  std::map<const IR::IDeclaration *, cstring> errorStringMap;

  ConversionContext(P4::ReferenceMap* refMap, P4::TypeMap* typeMap, 
      const IR::ToplevelBlock* toplevel, ProgramStructure* structure)
    : refMap(refMap), typeMap(typeMap), toplevel(toplevel), structure(structure) {}
};


}; // namespace PSDN

#endif
