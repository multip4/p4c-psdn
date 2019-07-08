/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_EXPRESSION_CONVERTER_H
#define PSDN_EXPRESSION_CONVERTER_H

#include <string>

#include "ir/ir.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/p4/typeMap.h"

#include "programStructure.h"
#include "conversionContext.h"

namespace PSDN {


cstring stringRepr(mpz_class value, unsigned bytes) {
	cstring sign = ""; 
	const char* r;
	cstring filler = ""; 
	if (value < 0) {
		value =- value;
		r = mpz_get_str(nullptr, 16, value.get_mpz_t());
		sign = "-";
	} else {
		r = mpz_get_str(nullptr, 16, value.get_mpz_t());
	}   

	if (bytes > 0) {
		int digits = bytes * 2 - strlen(r);
		BUG_CHECK(digits >= 0, "Cannot represent %1% on %2% bytes", value, bytes);
		filler = std::string(digits, '0');
	}   
	return sign + "0x" + filler + r;
}

class ExpressionConverter : public Inspector {
  P4::ReferenceMap* refMap;
  P4::TypeMap* typeMap;
  ProgramStructure* structure;
  ConversionContext* ctxt;
  P4::P4CoreLibrary& coreLibrary;
  
  /// Save result into map
  std::map<const IR::Expression*, std::string*> map;
  
  public:
  ExpressionConverter(P4::ReferenceMap* refMap, P4::TypeMap* typeMap,
      ProgramStructure* structure, ConversionContext* ctxt) : 
        refMap(refMap), typeMap(typeMap), structure(structure), 
        ctxt(ctxt), coreLibrary(P4::P4CoreLibrary::instance),
        singleExpressionOnly(false) {}


  /// If true, convert only single expressions like table keys
  bool singleExpressionOnly;

  /// @return a parameter which expression refers to
  const IR::Parameter* enclosingParamReference(const IR::Expression* e);

  /**
   * Convert an expression into string that represents an SDNet expression
   * @param e   expression to convert
   * 
   */
  std::string* convert(const IR::Expression* e);

  std::string* get(const IR::Expression* e) const;

	void mapExpression(const IR::Expression* e, std::string* str);

	void postorder(const IR::BoolLiteral* expression) override;
 	void postorder(const IR::MethodCallExpression* expression) override;
 	void postorder(const IR::Cast* expression) override;
// 	void postorder(const IR::Slice* expression) override;
// 	void postorder(const IR::AddSat* expression) override;
// 	void postorder(const IR::SubSat* expression) override;
 	void postorder(const IR::Constant* expression) override;
 	void postorder(const IR::ArrayIndex* expression) override;
 	void postorder(const IR::Member* expression) override;
// 	void postorder(const IR::Mux* expression) override;
// 	void postorder(const IR::IntMod* expression) override;
// 	void postorder(const IR::Operation_Binary* expression) override;
// 	void postorder(const IR::ListExpression* expression) override;
// 	void postorder(const IR::StructInitializerExpression* expression) override;
// 	void postorder(const IR::Operation_Unary* expression) override;
// 	void postorder(const IR::PathExpression* expression) override;
// 	void postorder(const IR::TypeNameExpression* expression) override;
// 	void postorder(const IR::Expression* expression) override;

  /**
   * Dump expression mapping for debugging purpose
   */
  void dumpExpressionMap();

};

}; //namespace PSDN


#endif
