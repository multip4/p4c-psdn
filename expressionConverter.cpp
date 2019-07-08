/*
 * Copyright 2019 Seungbin Song
 */

#include "expressionConverter.h"

#include "lib/algorithm.h"

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

/// Convert bools: there is no boolean value in SDNet, so change boolean literals into 1 or 0.
void ExpressionConverter::postorder(const IR::BoolLiteral* expression) {
  std::string* str = nullptr;
  if (*expression == IR::BoolLiteral(true))
    str = new std::string("1");
  else 
    str = new std::string("0");
  mapExpression(expression,str);
}

/// TODO: lookahead() / extract() needs method update & increment_offset.
void ExpressionConverter::postorder(const IR::MethodCallExpression* expression) {
  (void)expression;
}

/// Convert cast: there is no type casting in SDNet.
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
  mapExpression(expression, str);
}

/// Convert constant
void ExpressionConverter::postorder(const IR::Constant* expression) {
  auto bitWidth = expression->type->width_bits();
  std::string* str = new std::string(stringRepr(expression->value, ROUNDUP(bitWidth, 8)));
  mapExpression(expression, str);
}

/// TODO: there is no array in SDNet, expect array is converted into _index.
void ExpressionConverter::postorder(const IR::ArrayIndex* expression) {
  std::string* str;
  if (expression->left->is<IR::Member>()) {
    // Header stack type
    auto mem = expression->left->to<IR::Member>();
    auto parentType = typeMap->getType(mem->expr, true);
    auto type = parentType->to<IR::Type_StructLike>();
    auto field = type->getField(mem->member);
    str = new std::string(field->controlPlaneName());
  } else if (expression->left->is<IR::PathExpression>()) {
    // Temporary variable
    auto path = expression->left->to<IR::PathExpression>();
    str = new std::string(path->path->name.name);
  }

  if (!expression->right->is<IR::Constant>()) {
    ::error(ErrorType::ERR_INVALID, "All array indices must be constant", expression->right);
  } else {
    int index = expression->right->to<IR::Constant>()->asInt();
    str->append("_");
    str->append(std::to_string(index));
  }
  mapExpression(expression, str);
}

/// Non-null if expression refers to a parameter from the enclosing control
const IR::Parameter* 
ExpressionConverter::enclosingParamReference(const IR::Expression* expression) {
  CHECK_NULL(expression);
  if (!expression->is<IR::PathExpression>())
    return nullptr;

  auto pe = expression->to<IR::PathExpression>();
  auto decl = refMap->getDeclaration(pe->path, true);
  auto param = decl->to<IR::Parameter>();

  if (param !=nullptr && structure->nonActionParams.count(param) > 0)
    return param;
  return nullptr;
}

void ExpressionConverter::postorder(const IR::Member* expression) {
  std::string* str;

  auto parentType = typeMap->getType(expression->expr, true);
  cstring fieldName = expression->member.name;
  auto type = typeMap->getType(expression, true);

  if (parentType->is<IR::Type_StructLike>()) {
    auto st = parentType->to<IR::Type_StructLike>();
    auto field = st->getField(expression->member);
    if (field != nullptr)
      fieldName = field->controlPlaneName();
  }

  // Handle error types: find declaration in ctxt->errorStringMap and convert it
  if (type->is<IR::Type_Error>() && expression->expr->is<IR::TypeNameExpression>()) {
    auto decl = type->to<IR::Type_Error>()->getDeclByName(expression->member.name);
    auto converted = ctxt->errorStringMap.at(decl);
    str = new std::string(converted);
    mapExpression(expression, str);
    return;
  }
  
  // TODO: operations on header stacks & header unions
  if (parentType->is<IR::Type_Struct>() || parentType->is<IR::Type_Header>()) {
    auto param = enclosingParamReference(expression);
    auto parentName = param->getName().name;
    str = new std::string(parentName + "." + fieldName);
    mapExpression(expression, str);
    return;
  }

  if (expression->expr->is<IR::Member>()) {
    auto l = get(expression->expr);
    str = new std::string(*l + "." + fieldName);
    mapExpression(expression, str);
    return;
  }

  ::error(ErrorType::ERR_UNSUPPORTED_ON_TARGET, "Cannot convert member expression", expression);
}


void ExpressionConverter::dumpExpressionMap() {
  std::cout << "Dump Expressions" << std::endl;
  for (auto i : map) {
    std::cout << "\t";
    i.first->dbprint(std::cout);
    std::cout << " to " << i.second << std::endl;
  }
}




}; //namespace PSDN
