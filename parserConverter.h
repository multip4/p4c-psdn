/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_PARSER_CONVERTER_H
#define PSDN_PARSER_CONVERTER_H

#include "ir/ir.h"

#include "conversionContext.h"
#include "headerConverter.h"
#include "frontends/p4/coreLibrary.h"

namespace PSDN {

class ParserConverter : public Inspector {
  private:
    ConversionContext* ctxt;
    HeaderConverter* hconv;
    P4::P4CoreLibrary& corelib;

    cstring classDef;
    cstring tupleDef;
    cstring tupleInst;

    std::map<const IR::Declaration*, cstring> localvarMap;

  public:
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt, HeaderConverter* hconv) : 
      ctxt(ctxt), hconv(hconv), corelib(P4::P4CoreLibrary::instance), 
      classDef(""), tupleDef(""), tupleInst("") {
      setName("ParserConverter");
    }
    cstring emitParser();
};


}; // namespace PSDN


#endif
