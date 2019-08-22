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

    unsigned statePath;

    std::map<cstring, SDNetSection*> stateMap;

    std::map<const IR::Declaration*, cstring> localvarMap;

  public:
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt, HeaderConverter* hconv, ExpressionConverter* econv) : 
      ctxt(ctxt), hconv(hconv), econv(econv), corelib(P4::P4CoreLibrary::instance), 
      tupleDef(""), tupleInst(""), statePath(0) {
      setName("ParserConverter");
    }
    cstring emitParser();

  private:
    bool convertParams(const IR::Parameter* p);
    bool convertStatement(const IR::StatOrDecl* s, SDNetSection* section);
    void convertSelectExpression(const IR::SelectExpression* expr, SDNetSection* section);
    void convertPathExpression(const IR::PathExpression* expr, SDNetSection* section);
    void setSectionNumber(cstring nextState, SDNetSection* section);
    SDNetSection* getOrInsertState(cstring name);
};


}; // namespace PSDN


#endif
