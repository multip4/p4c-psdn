/*
 * Copyright 2019 Seungbin Song
 */

#include "sdnetProgram.h"

namespace PSDN {

cstring SDNetProgram::generateTuple(cstring name, cstring direction, cstring body) {
  cstring str = "class " + name + "::Tuple(" + direction + ") {\n";
  str += addIndent(body);
  str += "\n}\n";
  return str;
}

cstring SDNetProgram::addIndent(cstring str, unsigned n) {
  cstring indent = "";
  for (unsigned i = 0; i < n; i++) {
    indent += "\t";
  }
  return indent + str.replace("\n","\n"+indent);
}

unsigned SDNetProgram::getMaxPacketRegion(const IR::Type_ArchBlock* block) {
  unsigned maxPacketRegion = 12144;
  auto annotations = block->getAnnotations();
  if (annotations->annotations.size() == 0) {
    ::warning(ErrorType::WARN_INVALID,
        "Set Xilinx_MaxPacketRegion %1% as 12144", block);
  }
  else {
    auto a = annotations->annotations[0];
    if (a->name.name != "Xilinx_MaxPacketRegion") {
      ::warning(ErrorType::WARN_INVALID,
          "Set MaxPacketRegion of %1% as 12144", a);
    } else if (a->needsParsing) { 
      ::error(ErrorType::ERR_INVALID,
          "Annotation is not parsed by frontend", a);
    } else {
      if (a->expr.size() == 0) {
        ::warning(ErrorType::WARN_INVALID, 
            "# of annotation argument == 0\n Set %1% as 12144", a);
      } else {
        if (a->expr.size() > 1) {
          ::warning(ErrorType::WARN_INVALID,
              "# of annotation arguments > 1\n set %1% as 12144", a);
        }
        auto e = a->expr[0];
        if (!e->is<IR::Constant>()) {
          ::warning(ErrorType::WARN_INVALID,
              "Annotation argument is not a uint\n set %1% as 12144", e);
        } else {
          maxPacketRegion = e->to<IR::Constant>()->asUnsigned();
        }
      }
    }
  }
  return maxPacketRegion;
}

};
