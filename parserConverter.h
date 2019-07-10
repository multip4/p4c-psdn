/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_PARSER_CONVERTER_H
#define PSDN_PARSER_CONVERTER_H

#include "ir/ir.h"

#include "conversionContext.h"
#include "frontends/p4/coreLibrary.h"

namespace PSDN {

class ParserConverter : public Inspector {
  private:
    ConversionContext* ctxt;
    P4::P4CoreLibrary& corelib;

  public:
    bool preorder(const IR::P4Parser* parser) override;
    ParserConverter(ConversionContext* ctxt) : ctxt(ctxt), 
      corelib(P4::P4CoreLibrary::instance) {
      setName("ParserConverter");
    }
};


}; // namespace PSDN


#endif
