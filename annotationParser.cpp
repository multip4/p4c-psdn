/*
 * Copyright 2019 Seungbin Song
 */

#include "annotationParser.h"

namespace PSDN {

AnnotationParser::HandlerMap AnnotationParser::xilinxHandlers() {
  return {
    // @Xilinx_MaxPacketRegion, @Xilinx_MaxLatency, @Xilinx_ControlWidth
    // have a constant argument.
    PARSE("Xilinx_MaxPacketRegion", Constant),
    PARSE("Xilinx_MaxLatency", Constant),
    PARSE("Xilinx_ControlWidth", Constant),

    // @Xilinx_ExternallyConnected has an empty body.
    PARSE_EMPTY("Xilinx_ExternallyConnected"),
  };
}

}; //namespace PSDN
