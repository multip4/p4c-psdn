/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_PROGRAM_STRUCTURE_H
#define PSDN_PROGRAM_STRUCTURE_H


#include "frontends/common/model.h"
#include "ir/ir.h"

#include "directMeterMap.h"
#include "resourceMap.h"

#define SUME_SWITCH_TYPE_NAME "SimpleSumeSwitch"
#define SUME_PARSER_PAR_NAME "p"
#define SUME_PIPE_PAR_NAME "map"
#define SUME_DEPARSER_PAR_NAME "d"

namespace PSDN {

// Represent compile-time information about a P4 program
class ProgramStructure {

  public:
    // Map each action to parent control
    ordered_map<const IR::P4Action*, const IR::P4Control*> actionMap;
    // Map each parameter of an action to its positional index
    ordered_map<const IR::Parameter*, unsigned> actionParamMap;
		// Parameters of controls or parsers
    ordered_set<const IR::Parameter*> nonActionParams;
    // local variables
    std::vector<const IR::Declaration_Variable*> variables;
    // error codes
    ordered_map<const IR::IDeclaration*, unsigned int> errorCodeMap;
    // All the direct meters
    DirectMeterMap directMeterMap;
    // All the direct counters
    ordered_map<cstring, const IR::P4Table*> directCounterMap;
    // All match kinds
    std::set<cstring>  matchKinds;
    // map IR node to compile-time allocated resource blocks.
    ResourceMap resourceMap;


    // We place scalar user metadata fields (i.e., bit<>, bool)
    // in the scalarsName metadata object, so we may need to rename
    // these fields.  This map holds the new names.
    std::map<const IR::StructField*, cstring> userMetadataMap;

    const IR::P4Parser* parser;
    const IR::P4Control* pipe;
    const IR::P4Control* deparser;

    std::set<cstring> nonPipelineControls;
    std::set<cstring> pipelineControls;

    ProgramStructure() {}

};

class ProgramStructureBuilder : public Inspector {
    ProgramStructure* structure;

  public: 
    explicit ProgramStructureBuilder(ProgramStructure* structure) :
      structure(structure) { CHECK_NULL(structure); }

    void postorder(const IR::ParameterList *paramList) override;
    void postorder(const IR::P4Action *action) override;
    void postorder(const IR::Declaration_Variable *decl) override;
    void postorder(const IR::Type_Error *errors) override;
    void postorder(const IR::Declaration_MatchKind* kind) override;

    bool preorder(const IR::PackageBlock* main) override;
};


}; //namespace PSDN


#endif
