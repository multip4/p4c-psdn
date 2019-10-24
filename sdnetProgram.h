/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_SDNET_PROGRAM_H
#define PSDN_SDNET_PROGRAM_H

#include "lib/cstring.h"
#include "ir/ir.h"

namespace PSDN {

namespace SDNet {

class Tuple {
  public:
    cstring name;
    cstring direction;
    cstring body;
    Tuple() : name(""), direction(""), body("") {}
    Tuple(cstring name, cstring direction, cstring body)
      : name(name), direction(direction), body(body) {}

    cstring emit();
};

class Section {
  public:
    unsigned number;
    cstring name;
    cstring structDecl;
    std::vector<cstring> mapDecl;
    std::vector<cstring> methodUpdate;
    cstring methodMove;
    int methodIncrement;
    Section() : number(0), name(""), structDecl(""),
      methodMove(""), methodIncrement(0) {}

    cstring emit();
};

class TupleEngine {
  public:
    cstring name;
    cstring externalStructDecl;
    std::vector<Tuple*> tuples;
    std::vector<Section*> sections;
};

class LookupEngine {
  public:
    cstring name;
    cstring matchType;
    unsigned capacity;
    unsigned keyWidth;
    unsigned valueWidth;
    unsigned responseType;
    bool external;
    Tuple* requestTuple;
    Tuple* responseTuple;

    cstring emit();

    LookupEngine() : name(""), matchType(""), capacity(64),
        keyWidth(0), valueWidth(0), responseType(1), external(false) {}
};

  cstring addIndent(cstring str, unsigned n = 1);
  unsigned getMaxPacketRegion(const IR::Type_ArchBlock* block);
};

}; //namespace PSDN


#endif
