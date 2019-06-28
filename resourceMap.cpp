/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#include "resourceMap.h"

namespace PSDN {

bool ResourceMapBuilder::preorder(const IR::ControlBlock* control) {
  resourceMap->emplace(control->container, control);
  for (auto cv : control->constantValue) {
    resourceMap->emplace(cv.first, cv.second);
  }

  for (auto c : control->container->controlLocals) {
    if (c->is<IR::InstantiatedBlock>()) {
      resourceMap->emplace(c, control->getValue(c));
    } }
  return false;
}

bool ResourceMapBuilder::preorder(const IR::ParserBlock* parser) {
  resourceMap->emplace(parser->container, parser);
  for (auto cv : parser->constantValue) {
    resourceMap->emplace(cv.first, cv.second);
    if (cv.second->is<IR::Block>()) {
      visit(cv.second->getNode());
    } }

  for (auto c : parser->container->parserLocals) {
    if (c->is<IR::InstantiatedBlock>()) {
      resourceMap->emplace(c, parser->getValue(c));
    } }
  return false;
}

bool ResourceMapBuilder::preorder(const IR::TableBlock* table) {
  resourceMap->emplace(table->container, table);
  for (auto cv : table->constantValue) {
    resourceMap->emplace(cv.first, cv.second);
    if (cv.second->is<IR::Block>()) {
      visit(cv.second->getNode()); } }
  return false;
}

bool ResourceMapBuilder::preorder(const IR::PackageBlock* package) {
  for (auto cv : package->constantValue) {
    if (cv.second->is<IR::Block>()) {
      visit(cv.second->getNode()); } }
  return false;
}

bool ResourceMapBuilder::preorder(const IR::ToplevelBlock* toplevel) {
  auto package = toplevel->getMain();
  visit(package);
  return false;
}

}; //namespace PSDN
