/*
 * Copyright 2019 Seungbin Song
 */

#include "expressionConverter.h"

namespace PSDN {

std::string* ExpressionConverter::convert(const IR::Expression* e) {
  e->apply(*this);
  auto result = ::get(map, e->to<IR::Expression>());
  if (result == nullptr)
    BUG("%1%: Cannot convert expression", e);
  return result;
};

void ExpressionConverter::mapExpression(const IR::Expression* e, std::string* str) {
  map.emplace(e, str);
}

std::string* ExpressionConverter::get(const IR::Expression* e) const {
  auto result = ::get(map, e);
  if (result == nullptr)
    BUG("%1%: There is no expression mapping", e);
  return result;
}

/// There is no boolean value in SDNet, so change boolean literals into 1 or 0.
void ExpressionConverter::postorder(const IR::BoolLiteral* expression) {
  std::string* str = nullptr;
  if (*expression == IR::BoolLiteral(true))
    str = new std::string("1");
  else 
    str = new std::string("0");
  mapExpression(expression,str);
}

void ExpressionConverter::postorder(const IR::MethodCallExpression* expression) {
  auto instance = P4::MethodInstance::resolve(expression, refMap, typeMap);
}

/// Ther is no type casting in SDNet
void ExpressionConverter::postorder(const IR::Cast* expression) {
  /*
  std::string* str = nullptr;
  // Get original type from expression, and destination type from destType.
  int originalWidth = expression->expr->type->width_bits();
  int destWidth = expression->destType->width_bits();
  // If destination type is larger than original type, extend with zero.
  if (destWidth > originalWidth) {
    str = new std::string(std::to_string(destWidth-originalWidth));
    str->append();
  }
  */
  std::string* str = new std::string("");
  mapExpression(expression,str);
}

};
