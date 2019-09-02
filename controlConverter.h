/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_CONTROL_CONVERTER_H
#define PSDN_CONTROL_CONVERTER_H

#include "ir/ir.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "midend/convertEnums.h"

#include "expressionConverter.h"
#include "headerConverter.h"

namespace PSDN {

class ControlConverter : public Inspector {
private:
  ConversionContext* ctxt;
  P4::P4CoreLibrary& corelib;
  HeaderConverter* hconv;
  ExpressionConverter* econv;

public:
  bool preorder(const IR::P4Control* control) override;
  ControlConverter(ConversionContext* ctxt, HeaderConverter* hconv, ExpressionConverter* econv) :
    ctxt(ctxt), corelib(P4::P4CoreLibrary::instance), hconv(hconv), econv(econv)
  { setName("ControlConverter"); }

};


} // namespace PSDN


#endif /* PSDN_CONTROL_CONVERTER_H */

