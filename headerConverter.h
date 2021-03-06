/*
 * Copyright 2019 Seungbin Song
 */
#ifndef PSDN_HEADER_CONVERTER_H
#define PSDN_HEADER_CONVERTER_H

#include "ir/ir.h"
#include "lib/cstring.h"
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"

#include "programStructure.h"
#include "conversionContext.h"

namespace PSDN {

class HeaderConverter : public Inspector {
  private:
    ConversionContext* ctxt;
    std::set<cstring> visitedHeaders;

    bool isHeaders(const IR::Type_StructLike* st);

    void addHeader(const IR::Type_StructLike* st);
    void addStruct(const IR::Type_StructLike* st);

    void mapStruct(const IR::Type_StructLike* st, cstring str);

    std::map<const IR::Type_StructLike*, cstring> map;

  public:
    HeaderConverter(ConversionContext* ctxt);

    bool preorder(const IR::Parameter* param) override;
    Visitor::profile_t init_apply(const IR::Node* node) override;
    
    cstring getDefinition(const IR::Type_StructLike* st, bool withName);
    cstring emitTypeDef();
};


}; //namespace PSDN 

#endif
