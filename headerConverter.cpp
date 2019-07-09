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
  if (visitedHeaders.find(st->getName()) != visitedHeaders.end())
    return;

  cstring str = "{\n";
  // If this type is header, add isValid.
  if (st->is<IR::Type_Header>()) {
    str += "\tisValid : 1,\n";
  }
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
  //std::cout << st->getName() << " " << str << std::endl;
  mapStruct(st, str);
  visitedHeaders.emplace(st->getName());
}

void HeaderConverter::addHeader(const IR::Type_StructLike* st) {
  if (visitedHeaders.find(st->getName()) != visitedHeaders.end())
    return;

  cstring str = "{\n";
  for (auto f : st->fields) {
    auto ft = ctxt->typeMap->getType(f, true);
    auto fst = ft->to<IR::Type_StructLike>();
    addStruct(fst);
    str += "\t" + f->name + " : " + fst->getName().name;
    if (f == *(st->fields.rbegin()))
      str += "\n}\n";
    else
      str += ",\n";
  }
  //std::cout << st->getName() << " " << str << std::endl;
  mapStruct(st, str);
  visitedHeaders.emplace(st->getName());
}

bool HeaderConverter::preorder(const IR::Parameter* param) {
  auto type = ctxt->typeMap->getType(param->getNode(), true);
  if (type->is<IR::Type_Struct>()) {
    auto st = type->to<IR::Type_Struct>();
    if (isHeaders(st)) { //header
      addHeader(st);
    } else { //metadata
      addStruct(st);
    }
  }
  return false;
}

cstring HeaderConverter::getDefinition(const IR::Type_StructLike* st, bool withName) {
  cstring str = (withName) ? "struct " + st->getName() + " " : "struct ";
  auto result = ::get(map, st);
  if (result == nullptr)
    BUG("%1%: There is no definition mapping", st);
  str += result;
  return str;
}

cstring HeaderConverter::emitTypeDef() {
  cstring str = "";
  for (auto st : map)
    str += getDefinition(st.first, true);
  return str;
}


}; //namespace PSDN
