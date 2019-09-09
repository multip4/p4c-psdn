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
    std::vector<Tuple*> tuples;
    std::vector<Section*> sections;
};

  cstring addIndent(cstring str, unsigned n = 1);
  unsigned getMaxPacketRegion(const IR::Type_ArchBlock* block);
};

}; //namespace PSDN


#endif
