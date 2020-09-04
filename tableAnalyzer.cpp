/* 
 * Written by Seungbin Song 
 */


#include "tableAnalyzer.h"
#include "graphs.h"

#include "frontends/p4/methodInstance.h"
#include "frontends/p4/tableApply.h"
#include "lib/log.h"
#include "lib/nullstream.h"
#include "lib/path.h"

namespace PSDN {

  static ExprSet unionExprSet (const ExprSet& e1, const ExprSet& e2) {
    ExprSet result = e1;
    for (auto i2 : e2) {
      result.insert(i2);
    }
    return result;
  }

  static ExprSet subtractExprSet (const ExprSet& e1, const ExprSet& e2) {
    ExprSet result = e1;
    for (auto i2 : e2) {
      result.erase(i2);
    }
    return result;
  }

  void Action::print() {
     std::cout << "    Def: " << std::endl;
    for(auto e : this->def)
      std::cout << "      " << e << std::endl;
    std::cout << "    Use: " << std::endl;
    for(auto e : this->use)
      std::cout << "      " << e << std::endl;
 }

  void Table::print () {
    std::cout << "Name: " << this->name << std::endl;
    for (auto k : this->keys)
      std::cout << "    Key: " << k << std::endl;
    for (auto a : this->actions) {
      std::cout << "    Action: " << a.first << std::endl;
      a.second->print();
    }
  }

  void Stat::print () {
    std::cout << "File Name, Pipeline Name, # of Table, # of Table Indep. Pair, # of Action Indep. Pair" << std::endl;
    std::cout << fileName << ", " << pipelineName << ", " << numTable << ", "
      << numTableIndependentPair << ", " << numActionIndependentPair << std::endl;
  }

  Stat::Stat(cstring name, cstring fname) : numTable(0), 
    numTableIndependentPair(0), numActionIndependentPair(0),
    pipelineName(name), fileName(fname) {}

  Action::Action() : action(nullptr), def({}), use({}) {}

  Dependency::Dependency(Table* _first, Table* _second, DependencyType _type, 
      bool _isTableDependency, cstring _dataName) {
    firstTable = _first;
    secondTable = _second;
    type = _type;
    isTableDependency = _isTableDependency;
    dataName = _dataName;
  }

  void Dependency::print() {
    std::cout << "Table1: " << firstTable->name << ", ";
    std::cout << "Table2: " << secondTable->name << " ";
    if(isTableDependency)
      std::cout << "[Table ";
    else 
      std::cout << "[Action ";
    if(type == DependencyType::UseDef)
      std::cout << "Use-Def] ";
    else if(type == DependencyType::DefUse)
      std::cout << "Def-Use] ";
    else 
      std::cout << "Def-Def] ";
    std::cout << "id: " << dataName << std::endl;
  }

  TableAnalyzer::TableAnalyzer(P4::ReferenceMap *refMap, P4::TypeMap *typeMap, cstring file)
    : refMap(refMap), typeMap(typeMap), fileName(file), curAction(new Action()), 
      curActionMap(new ActionMap()), curTable(new Table()), 
      tableStack(new TableStack()), dependencies(new Dependencies()), graph(new Graphs()){}

  void TableAnalyzer::setCurrentAction(const IR::P4Action *action) {
    curAction->action = action;
    curAction->def = {};
    curAction->use = {};
  }

  void TableAnalyzer::saveCurrentAction() {
    (*curActionMap)[curAction->action->toString()] = curAction;
    curAction = new Action();
  }

  void TableAnalyzer::clearCurrentActionMap() {
    delete(curActionMap);
    curActionMap = new ActionMap();
  }

  ExprSet TableAnalyzer::findId(const IR::Expression *expr) {
    if (expr->is<IR::ListExpression>()) {
      auto exprList = expr->to<IR::ListExpression>()->components;
      if (exprList.empty()) {
        return {};
      } else {
        auto lastExpr = exprList.back();
        exprList.pop_back();
        const IR::Expression *rest = new IR::ListExpression(exprList);
        return unionExprSet(findId(lastExpr), findId(rest));
      }
    }
    if (expr->is<IR::Operation_Binary>()) {
      const IR::Operation_Binary *bexpr = expr->to<IR::Operation_Binary>();
      ExprSet lresult = findId(bexpr->left);
      ExprSet rresult = findId(bexpr->right);
      return unionExprSet(lresult, rresult);
    }
    if (expr->is<IR::Operation_Ternary>()) {
      const IR::Operation_Ternary *texpr = expr->to<IR::Operation_Ternary>();
      ExprSet e0r = findId(texpr->e0);
      ExprSet e1r = findId(texpr->e1);
      ExprSet e2r = findId(texpr->e2);
      return unionExprSet(unionExprSet(e0r, e1r), e2r);
    }
    if (expr->is<IR::Operation_Unary>()) {
      if (expr->is<IR::Member>()) {
        auto m = expr->to<IR::Member>();
        if (m->expr->is<IR::TypeNameExpression>()) {
          return {};
        } else {
          ExprSet result = {expr->toString()};
          return result;
        }
      } else {
      return findId(expr->to<IR::Operation_Unary>()->expr);
      }
    }
    if (expr->is<IR::AttribLocal>()) {
      ExprSet result = {expr->toString()};
      return result;
    }
    ExprSet v = {};
    return v;
  }

  void TableAnalyzer::buildDependenceGraph() {
    if (std::find(tableStack->begin(), tableStack->end(), curTable) != tableStack->end()) {
      ::error("[ERROR] curTable already exists in the tableStack");
      return;
    }

    //find table dependency
    for (auto t = tableStack->rbegin(); t != tableStack->rend(); ++t) {
      for (auto a : (*t)->actions) {
        for (auto k : curTable->keys) {
          if (a.second->def.find(k) != a.second->def.end()) {
            dependencies->push_back(Dependency(*t, curTable, DependencyType::DefUse, true, k));
            graph->add_edge((*t)->vertex, curTable->vertex, k, Graphs::EdgeType::TABLE);
          }
        }
      }
    }

    //find action dependency
    for (auto t = tableStack->rbegin(); t != tableStack->rend(); ++t) {
      for (auto firstAction : (*t)->actions) {
        for (auto secondAction : curTable->actions) {
          for (auto d : secondAction.second->def) {
            if (firstAction.second->def.find(d) != firstAction.second->def.end()) {
              dependencies->push_back(Dependency(*t, curTable, DependencyType::DefDef, false, d));
              graph->add_edge((*t)->vertex, curTable->vertex, d, Graphs::EdgeType::ACTION);
            }
            if (firstAction.second->use.find(d) != firstAction.second->use.end()) {
              dependencies->push_back(Dependency(*t, curTable, DependencyType::UseDef, false, d));
              graph->add_edge((*t)->vertex, curTable->vertex, d, Graphs::EdgeType::ACTION);
            }
          }
          for (auto u : secondAction.second->use) {
            if (firstAction.second->def.find(u) != firstAction.second->def.end()) {
              dependencies->push_back(Dependency(*t, curTable, DependencyType::DefUse, false, u));
              graph->add_edge((*t)->vertex, curTable->vertex, u, Graphs::EdgeType::ACTION);
            }
          }
        }
      }
    }

  }

  void TableAnalyzer::findIndependentTables(Stat& stat) {
    for(auto i = tableStack->begin(); i != tableStack->end(); ++i){
      if (graph->isCondition((*i)->vertex))
        continue;

      stat.numTable++;

      //for(auto j = (i+1); j != tableStack->end(); ++j) {
      auto j = (i+1);
        if ((*j) == nullptr || j == tableStack->end())
          continue;

        if (graph->isCondition((*j)->vertex))
          continue;

        if(graph->isTableIndependent((*i)->vertex, (*j)->vertex)) {
          stat.numTableIndependentPair++;
          std::cout << "Table " << (*i)->name << " and "
            "Table " << (*j)->name << " are table-independent." << std:: endl;
        }
        if(graph->isActionIndependent((*i)->vertex, (*j)->vertex)) {
          stat.numActionIndependentPair++;
          std::cout << "Table " << (*i)->name << " and "
            "Table " << (*j)->name << " are action-independent." << std:: endl;
        }
      //}
    }
  }

  bool TableAnalyzer::preorder(const IR::PackageBlock *block) {
    for (auto it : block->constantValue) {
      if(it.second->is<IR::ControlBlock>()) {
        auto name = it.second->to<IR::ControlBlock>()->container->name;
        std::cout << "\nAnalyzing top-level control " << name << std::endl;
        visit(it.second->getNode());
        clearCurrentActionMap();
        
        std::cout << "Printing Tables..." << std::endl;
        for(auto i = tableStack->begin(); i != tableStack->end(); ++i) 
          (*i)->print();
        std::cout << "Printing Dependencies..." << std::endl;
        for(auto i = dependencies->begin(); i != dependencies->end(); ++i) 
          (*i).print();
        graph->writeGraphToFile(fileName + "." + name);

        Stat stat(name, fileName);
        findIndependentTables(stat);
        stat.print();

        tableStack = new TableStack();
        dependencies = new Dependencies();
        graph = new Graphs();
      }
    }


    return false;
  }


  bool TableAnalyzer::preorder(const IR::ControlBlock *block) {
    visit(block->container);
    return false;
  }

  bool TableAnalyzer::preorder(const IR::P4Control *cont) {
    auto decls = cont->getDeclarations();
    for (auto i = decls->begin(); i != decls->end(); ++i) {
      auto p4action = (*i)->to<IR::P4Action>();
      visit(p4action);
    }
    visit(cont->body);
    return false;
  }

  bool TableAnalyzer::preorder(const IR::BlockStatement *statement) {
    for (const auto component : statement->components) {
      visit(component);
    }
    return false;
  }

  bool TableAnalyzer::preorder(const IR::IfStatement *statement) {
    //Insert if statement as a table
    std::ostringstream _stream;
    statement->condition->dbprint(_stream);
    curTable->name = _stream.str();
    curTable->keys = findId(statement->condition);
    curTable->vertex = graph->add_vertex(curTable->name, Graphs::VertexType::CONDITION);
    buildDependenceGraph();
    tableStack->push_back(curTable);
    curTable = new Table();

    //Copy the current tableStack
    int size = (int)tableStack->size();
    TableStack *savedTableStack = new TableStack();
    savedTableStack->resize(size);
    std::copy(tableStack->begin(),tableStack->end(),savedTableStack->begin());
    visit(statement->ifTrue);

    //Restore tableStack
    if(statement->ifFalse != nullptr) {
      if (*savedTableStack != *tableStack) {
        TableStack *tmp = tableStack;
        tableStack = savedTableStack;
        savedTableStack = tmp;
      }
      visit(statement->ifFalse);
      //Merge tableStack of true and false
      tableStack->insert(tableStack->end(), savedTableStack->begin()+size, savedTableStack->end());
    }

    return false;
  }

  bool TableAnalyzer::preorder(const IR::SwitchStatement *statement) {
    auto tbl = P4::TableApplySolver::isActionRun(statement->expression, refMap, typeMap);
    if (tbl != nullptr) {
      visit(tbl);
    }

    for (auto scase : statement->cases) {
      if(scase->statement != nullptr) {
        visit(scase->statement);
      }
      if(scase->label->is<IR::DefaultExpression>()) {
        break;
      }
    }

    return false;
  }

  void TableAnalyzer::visitExterns(const P4::MethodInstance *instance) {
    auto args = instance->expr->arguments;
    auto params = instance->getActualParameters();
    ExprSet inExprs = {};
    ExprSet outExprs = {};
    
    std::cout << "    Extern: " << instance->expr->method << std::endl;
    if (args->size() != params->size()) {
      ::error("ERROR: the number of args / params does not match");
      return;
    }

    for (unsigned i = 0; i < args->size(); i++) {
      auto a = (*args)[i];
      auto p = params->getParameter(i);
      if (p->hasOut()){
        std::cout << "      OUT: " << a << std::endl;
        outExprs = unionExprSet(findId(a->expression), outExprs);
      } else {
        std::cout << "      IN:  " << a << std::endl;
        inExprs = unionExprSet(findId(a->expression), inExprs);
      }
    }

    if (curAction->action != nullptr) {
      curAction->def = unionExprSet(curAction->def, outExprs);
      curAction->use = unionExprSet(curAction->use,
          subtractExprSet(inExprs, curAction->def));
    }
  }

  bool TableAnalyzer::preorder(const IR::MethodCallStatement *statement) {
    auto instance = P4::MethodInstance::resolve(statement->methodCall, refMap, typeMap);
    if(instance->is<P4::ApplyMethod>()) {
      auto am = instance->to<P4::ApplyMethod>();
      if (am->object->is<IR::P4Table>()) {
        visit(am->object->to<IR::P4Table>());
      } else if (am->applyObject->is<IR::Type_Control>()) {
        if (am->object->is<IR::Parameter>()) {
          ::error("%1%: control parameters ar not supported by this target", am->object);
          return false;
        } 
        BUG_CHECK(am->object->is<IR::Declaration_Instance>(),
            "Unsupported control invocation: %1%", am->object);
        auto instantiation = am->object->to<IR::Declaration_Instance>();
        auto type = instantiation->type;
        if (type->is<IR::Type_Name>()) {
          auto tn = type->to<IR::Type_Name>();
          auto decl = refMap->getDeclaration(tn->path, true);
          visit(decl->to<IR::P4Control>());
        }   
      } else {
          BUG("Unsupported apply method: %1%", instance);
      }
    }
    else if (curAction->action != nullptr && instance->is<P4::ExternFunction>()) {
      visitExterns(instance);
    }
    else if (curAction->action != nullptr && instance->is<P4::ExternMethod>()) {
      visitExterns(instance);
    }
    return false;
  }

  bool TableAnalyzer::preorder(const IR::AssignmentStatement *statement) {
    if (curAction->action != nullptr) {
      if (statement->left->is<IR::Member>()) {
        curAction->def = unionExprSet(curAction->def, {statement->left->toString()});
      } else if (statement->left->is<IR::AttribLocal>()) {
        curAction->def = unionExprSet(curAction->def, {statement->left->toString()});
      }
      curAction->use = unionExprSet(curAction->use, 
          subtractExprSet(findId(statement->right), curAction->def));
    }
    return false;
  }

  bool TableAnalyzer::preorder(const IR::ReturnStatement *) {
    return false;
  }

  bool TableAnalyzer::preorder(const IR::ExitStatement *) {
    return false;
  }

  bool TableAnalyzer::preorder(const IR::P4Table *table) {
    //Name
    std::cout << "  P4Table: " << table->controlPlaneName() << std::endl;
    curTable->name = table->controlPlaneName();

    //Key
    const auto keys = table->getKey();
    if (keys != nullptr) {
      if (keys->keyElements.empty() == false) {
        std::cout << "Keys:" << std::endl;
        for (const auto key : keys->keyElements)
          visit(key);
      }
    }

    //Action
    const auto actions = table->getActionList();
    if (actions != nullptr) {
      if (actions->actionList.empty() == false) {
        std::cout << "Actions:" << std::endl;
        for (const auto action : actions->actionList)
          visit(action);
      }
    }

    //curTable->print();
    curTable->vertex = graph->add_vertex(curTable->name, Graphs::VertexType::TABLE);
    buildDependenceGraph();
    tableStack->push_back(curTable);
    curTable = new Table();
    return false;
  }

  bool TableAnalyzer::preorder(const IR::ActionListElement *action) {
    auto a = refMap->getDeclaration(action->getPath(), true)->to<IR::P4Action>();
    std::cout << "  ActionListElement: " << a->toString() << std::endl;
    
    if ((*curActionMap)[a->toString()] != nullptr) {
      curTable->actions[a->toString()] = (*curActionMap)[a->toString()];
    }
    return false;
  }

  bool TableAnalyzer::preorder(const IR::P4Action *action) {
    std::cout << "  P4Action: " << action->toString() << std::endl;
    setCurrentAction(action);
    visit(action->body);
    saveCurrentAction();
    return false;
  }

  bool TableAnalyzer::preorder(const IR::KeyElement *key) {
    if (key->expression != nullptr) {
      std::cout << "  Key: " << key->expression->toString() << std::endl;
      curTable->keys.insert(key->expression->toString());
    }
    return false;
  }

} //namsepace multip4
