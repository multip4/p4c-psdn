/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_ANNOTATION_PARSER_H
#define PSDN_ANNOTATION_PARSER_H

#include "frontends/p4/parseAnnotations.h"

namespace PSDN {

class AnnotationParser : public P4::ParseAnnotations {
  public:
    HandlerMap xilinxHandlers();

    AnnotationParser() : ParseAnnotations("Xilinx", true, xilinxHandlers(), true) {}
};

}; //namespace PSDN

#endif
