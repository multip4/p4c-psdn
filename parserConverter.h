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

#include "sdnetProgram.h"

namespace PSDN {

class ParserConverter : public Inspector {
  private:
    ConversionContext* ctxt;
    HeaderConverter* hconv;
    ExpressionConverter* econv;
    P4::P4CoreLibrary& corelib;

    cstring tupleDef;
    cstring tupleInst;
    cstring sectionDef;

    std::map<const IR::Declaration*, cstring> localvarMap;

  public:
    bool convertParams(const IR::Parameter* p);
    bool convertStatement(const IR::StatOrDecl* s, SDNetSection& section);
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt, HeaderConverter* hconv, ExpressionConverter* econv) : 
      ctxt(ctxt), hconv(hconv), econv(econv), corelib(P4::P4CoreLibrary::instance), 
      tupleDef(""), tupleInst(""), sectionDef("") {
      setName("ParserConverter");
    }
    cstring emitParser();
};


}; // namespace PSDN


#endif
