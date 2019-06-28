/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_RESOURCE_MAP_H
#define PSDN_RESOURCE_MAP_H

#include "lib/null.h"
#include "ir/ir.h"

namespace PSDN {

using ResourceMap = ordered_map<const IR::Node*, const IR::CompileTimeValue*>;

/**
 * Resource map builder builds resource map that represents mapping from IR::Node to IR::Block.
 */
class ResourceMapBuilder : public Inspector {
  private:
    ResourceMap *resourceMap;

  public:
    explicit ResourceMapBuilder(ResourceMap *resourceMap) : resourceMap(resourceMap) {
      CHECK_NULL(resourceMap);
    }

    bool preorder(const IR::ControlBlock* control) override;
    bool preorder(const IR::ParserBlock* parser) override;
    bool preorder(const IR::TableBlock* table) override;
    bool preorder(const IR::PackageBlock* package) override;
    bool preorder(const IR::ToplevelBlock* toplevel) override;
};


}; //namespace PSDN

#endif
