/*
 * Copyright 2019 Seungbin Song
 */

#include "sdnetProgram.h"

namespace PSDN {

namespace SDNet {

cstring Tuple::emit() {
  cstring str = "class " + name + "::Tuple(" + direction + ") {\n";
  str += addIndent(body);
  str += "\n}\n";
  return str;
}

cstring LookupEngine::emit() {
  cstring str = "class " + name + "::LookupEngine(";
  str += matchType + "," + std::to_string(capacity) + ",";
  str += std::to_string(keyWidth) + "," + std::to_string(valueWidth) + ",";
  str += std::to_string(responseType) + "," + (external ? "1" : "0");
  str += ") {\n";

  str += addIndent(requestTuple->emit()) + "\n";
  str += addIndent(responseTuple->emit()) + "\n";
  
  str += "\t" + requestTuple->name + " request;\n";
  str += "\t" + responseTuple->name + " response;\n";

  str += "\t send_request = { key = request }\n";
  str += "\t receive_response = { response = value }\n";
  str += "}";

  return str;
}

cstring addIndent(cstring str, unsigned n) {
  cstring indent = "";
  for (unsigned i = 0; i < n; i++) {
    indent += "\t";
  }
  return indent + str.replace("\n","\n"+indent);
}

unsigned getMaxPacketRegion(const IR::Type_ArchBlock* block) {
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

cstring Section::emit() {
  cstring result = "class " + name + "::Section(" + std::to_string(number) + ") {\n";
  if (structDecl != "")
    result += SDNet::addIndent(structDecl) + "\n";
  if (!mapDecl.empty()) {
    result += "\tmap transition {\n";
    for (auto i = mapDecl.begin(); i != mapDecl.end(); i++) {
      if (i != mapDecl.end() - 1)
        result += "\t\t" + (*i) + ",\n";
      else
        result += "\t\t" + (*i) + "\n";
    }
    result += "\t}\n";
  }
  if (!methodUpdate.empty()) {
    result += "\tmethod update = {\n";
    for (auto i = methodUpdate.begin(); i != methodUpdate.end(); i++) {
      if (i != methodUpdate.end()-1)
        result += "\t\t" + (*i) + ",\n";
      else
        result += "\t\t" + (*i) + "\n";
    }
    result += "\t}\n";
  }
  if (methodMove != "")
    result += "\tmethod move_to_section = " + methodMove + ";\n";
  else
    result += "\tmethod move_to_section = done(0);\n";
  result += SDNet::addIndent("method increment_offset = "+std::to_string(methodIncrement)+";");
  result += "\n}\n";
  return result;
}

}; // namespace SDNet 

}; // namespace PSDN
