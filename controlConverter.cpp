/*
 * Copyright 2019 Seungbin Song
 */

#include "controlConverter.h"

#include "controlFlowGraph.h"

namespace PSDN {

bool ControlConverter::preorder(const IR::P4Control* control) {
  auto cfg = new CFG();
  cfg->build(control, ctxt->refMap, ctxt->typeMap);
  bool success = cfg->checkImplementable();
  if (!success) {
    ::error(ErrorType::ERR_UNSUPPORTED, "Control-flow graph is complicated", control);
    return false;
  }
  return false;
}

} // namespace PSDN
