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

    cstring classDef;
    std::vector<SDNet::Tuple*> tuples;
    cstring tupleInst;

    unsigned sectionPathLength;

    std::map<cstring, SDNet::Section*> stateMap;

    std::map<const IR::Declaration*, cstring> localvarMap;

  public:
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt, HeaderConverter* hconv, ExpressionConverter* econv) : 
      ctxt(ctxt), hconv(hconv), econv(econv), corelib(P4::P4CoreLibrary::instance), 
      classDef(""), tupleInst(""), sectionPathLength(0) {
      setName("ParserConverter");
    }
    cstring emitParser();

  private:
    bool convertParams(const IR::Parameter* p);
    bool convertStatement(const IR::StatOrDecl* s, SDNet::Section* section);
    void convertSelectExpression(const IR::SelectExpression* expr, SDNet::Section* section);
    void convertPathExpression(const IR::PathExpression* expr, SDNet::Section* section);
    void setSectionNumber(cstring nextState, SDNet::Section* section);
    SDNet::Section* getOrInsertState(cstring name);
};


}; // namespace PSDN


#endif
