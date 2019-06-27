/* 
 * Copyright 2019 Seungbin Song
 */

#include "midend.h"

#include "frontends/common/options.h"
#include "frontends/common/constantFolding.h"
#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/fromv1.0/v1model.h"
#include "frontends/p4/moveDeclarations.h"
#include "frontends/p4/simplify.h"
#include "frontends/p4/simplifyParsers.h"
#include "frontends/p4/strengthReduction.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/uniqueNames.h"
#include "frontends/p4/unusedDeclarations.h"
#include "midend/actionSynthesis.h"
#include "midend/complexComparison.h"
#include "midend/convertEnums.h"
#include "midend/copyStructures.h"
#include "midend/eliminateTuples.h"
#include "midend/eliminateNewtype.h"
#include "midend/eliminateSerEnums.h"
#include "midend/flattenHeaders.h"
#include "midend/flattenInterfaceStructs.h"
#include "midend/local_copyprop.h"
#include "midend/nestedStructs.h"
#include "midend/removeLeftSlices.h"
#include "midend/removeParameters.h"
#include "midend/removeUnusedParameters.h"
#include "midend/simplifyKey.h"
#include "midend/simplifySelectCases.h"
#include "midend/simplifySelectList.h"
#include "midend/removeSelectBooleans.h"
#include "midend/validateProperties.h"
#include "midend/compileTimeOps.h"
#include "midend/orderArguments.h"
#include "midend/predication.h"
#include "midend/expandLookahead.h"
#include "midend/expandEmit.h"
#include "midend/tableHit.h"
#include "midend/midEndLast.h"
#include "midend/fillEnumMap.h"
#include "midend/removeAssertAssume.h"

namespace PSDN {

// Convert all enums to 32 bits.
class PSDNEnumOn32Bits : public P4::ChooseEnumRepresentation {
  private:
    bool convert(__attribute__((unused)) const IR::Type_Enum* type) const override { return true; }
    unsigned enumSize(unsigned) const override { return 32; }

  public:
    explicit PSDNEnumOn32Bits() {}
};

const IR::ToplevelBlock* PSDNMidEnd::process(const IR::P4Program *&program) {
  program = program->apply(*this);
  return toplevel;
}

PSDNMidEnd::PSDNMidEnd(CompilerOptions& options) {
	auto convertEnums = new P4::ConvertEnums(&refMap, &typeMap, new PSDNEnumOn32Bits());
	auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
	//Add midend optimizations if a program is parsed from frontend.
	if (PSDN::PSDNContext::get().options().loadIRFromJson == false) {
		addPasses({
				options.ndebug ? new P4::RemoveAssertAssume(&refMap, &typeMap) : nullptr,
				new P4::EliminateNewtype(&refMap, &typeMap),
				new P4::EliminateSerEnums(&refMap, &typeMap),
				new P4::RemoveActionParameters(&refMap, &typeMap),
				convertEnums,
				new VisitFunctor([this, convertEnums]() { enumMap = convertEnums->getEnumMapping(); }),
				new P4::OrderArguments(&refMap, &typeMap),
				new P4::TypeChecking(&refMap, &typeMap),
				new P4::SimplifyKey(&refMap, &typeMap,
						new P4::OrPolicy(
							new P4::IsValid(&refMap, &typeMap),
							new P4::IsMask())),
				new P4::ConstantFolding(&refMap, &typeMap),
				new P4::StrengthReduction(&refMap, &typeMap),
				new P4::SimplifySelectCases(&refMap, &typeMap, true),  // require constant keysets
				new P4::ExpandLookahead(&refMap, &typeMap),
				new P4::ExpandEmit(&refMap, &typeMap),
				new P4::SimplifyParsers(&refMap),
				new P4::StrengthReduction(&refMap, &typeMap),
				new P4::EliminateTuples(&refMap, &typeMap),
				new P4::SimplifyComparisons(&refMap, &typeMap),
				new P4::CopyStructures(&refMap, &typeMap),
				new P4::NestedStructs(&refMap, &typeMap),
				new P4::SimplifySelectList(&refMap, &typeMap),
				new P4::RemoveSelectBooleans(&refMap, &typeMap),
				new P4::FlattenHeaders(&refMap, &typeMap),
				new P4::FlattenInterfaceStructs(&refMap, &typeMap),
				new P4::Predication(&refMap),
				new P4::MoveDeclarations(),  // more may have been introduced
				new P4::ConstantFolding(&refMap, &typeMap),
				new P4::LocalCopyPropagation(&refMap, &typeMap),
				new P4::ConstantFolding(&refMap, &typeMap),
				new P4::MoveDeclarations(),
				new P4::ValidateTableProperties({ "psa_implementation",
						"psa_direct_counter",
						"psa_direct_meter",
						"psa_idle_timeout",
						"size" }), 
				new P4::SimplifyControlFlow(&refMap, &typeMap),
				new P4::CompileTimeOperations(),
				new P4::TableHit(&refMap, &typeMap),
				new P4::MoveActionsToTables(&refMap, &typeMap),
				new P4::RemoveLeftSlices(&refMap, &typeMap),
				new P4::TypeChecking(&refMap, &typeMap),
				new P4::MidEndLast(),
				evaluator,
				new VisitFunctor([this, evaluator]() { toplevel = evaluator->getToplevelBlock(); }), 
    });
  } else {
    // Add minimum midends if file is IR in Json format.
		auto fillEnumMap = new P4::FillEnumMap(new PSDNEnumOn32Bits(), &typeMap);
		addPasses({
				new P4::ResolveReferences(&refMap),
				new P4::TypeChecking(&refMap, &typeMap),
				fillEnumMap,
				new VisitFunctor([this, fillEnumMap]() { enumMap = fillEnumMap->repr; }),
				evaluator,
				new VisitFunctor([this, evaluator]() { toplevel = evaluator->getToplevelBlock(); }),
		});
  }
};

}; //namespace PSDN

