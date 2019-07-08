/*
 * Copyright 2019 Seungbin Song
 */

#include "headerConverter.h"

namespace PSDN {

HeaderConverter::HeaderConverter(ConversionContext* ctxt) : ctxt(ctxt) {
    setName("HeaderConverter");
    CHECK_NULL(ctxt);
}

Visitor::profile_t HeaderConverter::init_apply(const IR::Node* node) {
  if(!ctxt->structure->variables.empty()) {
    for (auto v : ctxt->structure->variables) {
      std::cout << "Global variable " << v << std::endl;
      auto type = ctxt->typeMap->getType(v, true);
      if (auto st = type->to<IR::Type_StructLike>()) {
        auto metadata_type = st->controlPlaneName();
        if (type->is<IR::Type_Header>()){
          (void)metadata_type;
        }
      }
    }
  }
  return Inspector::init_apply(node);
}

bool HeaderConverter::isHeaders(const IR::Type_StructLike* st) {
  bool result = false;
  for (auto f : st->fields) {
    if (f->type->is<IR::Type_Header>() || f->type->is<IR::Type_Stack>()) {
      result = true;
    }
  }
  return result;
}

void HeaderConverter::mapStruct(const IR::Type_StructLike* st, cstring str) {
  map.emplace(st, str);
}

void HeaderConverter::addStruct(const IR::Type_StructLike* st) {
  cstring str = "{\n";
  for (auto f : st->fields) {
    auto ft = ctxt->typeMap->getType(f, true);
    if (ft->is<IR::Type_StructLike>()) {
      ::error(ErrorType::ERR_INVALID, "Cannot contain nested structures", st);
      return;
    } else if (ft->is<IR::Type_Bits>()) {
      auto tb = ft->to<IR::Type_Bits>();
      str += "\t" + f->name + " : " + std::to_string(tb->size);
    } else if (ft->is<IR::Type_Boolean>()) {
      str += "\t" + f->name + " : 1";
    } else {
      ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET, "Cannot convert fields", f);
      return;
    }
    if (f == *(st->fields.rbegin()))
      str += "\n}\n";
    else
      str += ",\n";
  }
  std::cout << st->getName() << " " << str << std::endl;
  mapStruct(st, str);
}

bool HeaderConverter::preorder(const IR::Parameter* param) {
  auto type = ctxt->typeMap->getType(param->getNode(), true);
  if (type->is<IR::Type_Struct>()) {
    auto st = type->to<IR::Type_Struct>();
    if (visitedHeaders.find(st->getName()) != visitedHeaders.end())
      return false;
    else
      visitedHeaders.emplace(st->getName());
    if (isHeaders(st)) { //header
      //addHeader(st);
    } else { //metadata
      addStruct(st);
    }
  }
  return false;
}


}; //namespace PSDN
