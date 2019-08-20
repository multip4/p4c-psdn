/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_PARSER_CONVERTER_H
#define PSDN_PARSER_CONVERTER_H

#include "ir/ir.h"

#include "conversionContext.h"
#include "headerConverter.h"
#include "expressionConverter.h"
#include "frontends/p4/coreLibrary.h"

namespace PSDN {

class ParserConverter : public Inspector {
  private:
    ConversionContext* ctxt;
    HeaderConverter* hconv;
    ExpressionConverter* econv;
    P4::P4CoreLibrary& corelib;

    cstring classDef;
    cstring tupleDef;
    cstring tupleInst;

    std::map<const IR::Declaration*, cstring> localvarMap;

  public:
    cstring convertStatement(const IR::StatOrDecl* stat);
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt, HeaderConverter* hconv, ExpressionConverter* econv) : 
      ctxt(ctxt), hconv(hconv), econv(econv), corelib(P4::P4CoreLibrary::instance), 
      classDef(""), tupleDef(""), tupleInst("") {
      setName("ParserConverter");
    }
    cstring emitParser();
};


}; // namespace PSDN


#endif
