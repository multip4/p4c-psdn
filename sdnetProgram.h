/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_SDNET_PROGRAM_H
#define PSDN_SDNET_PROGRAM_H

#include "lib/cstring.h"
#include "ir/ir.h"

namespace PSDN {

class SDNetSection {
  public:
    unsigned number;
    cstring name;
    cstring structDecl;
    std::vector<cstring> mapDecl;
    std::vector<cstring> methodUpdate;
    cstring methodMove;
    int methodIncrement;
    SDNetSection() : number(0), name(""), structDecl(""),
      methodMove(""), methodIncrement(0) {}

    cstring emit();
};

class SDNetProgram {
  public:
    cstring addIndent(cstring str, unsigned n = 1);
    cstring generateTuple(cstring name, cstring direction, cstring content);
    unsigned getMaxPacketRegion(const IR::Type_ArchBlock* block);
};

}; //namespace PSDN


#endif
