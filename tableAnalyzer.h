/*
Written by Seungbin Song
*/

#ifndef MULTIP4_TABLE_ANALYZER_H
#define MULTIP4_TABLE_ANALYZER_H

#include "ir/ir.h"
#include "ir/visitor.h"
#include "frontends/p4/methodInstance.h"

#include "graphs.h"

namespace P4 {
  class ReferenceMap;
  class TypeMap;

} //namespace P4

namespace PSDN {

  typedef std::set<cstring> ExprSet;

  class Action {
    public:
      const IR::P4Action *action;
      ExprSet def;
      ExprSet use;

      Action();
      void print();
  };

  typedef std::map<cstring, Action*> ActionMap;

  class Table {
    public:
      cstring name;
      ExprSet keys;
      ActionMap actions;
      Graphs::vertex_t vertex;

      void print();
  };

  class Stat {
    public:
      int numTable;
      int numTableIndependentPair;
      int numActionIndependentPair;
      cstring pipelineName;
      cstring fileName;

      Stat(cstring name, cstring fname);
      void print();
  };


  typedef enum DependencyType { UseDef, DefUse, DefDef } DependencyType;

  class Dependency {
    public:
      Table* firstTable;
      Table* secondTable;
      DependencyType type;
      bool isTableDependency;
      cstring dataName;

      Dependency(Table* _first, Table* _second, DependencyType _type, 
          bool _isTableDependency, cstring _dataName);

      void print();
  };

  typedef std::vector<Table*> TableStack;
  typedef std::vector<Dependency> Dependencies;

  class TableAnalyzer : public Inspector {
    public:
      TableAnalyzer(P4::ReferenceMap *refMap, P4::TypeMap *typeMap, cstring file);

      void setCurrentAction(const IR::P4Action *action);
      void saveCurrentAction();
      void clearCurrentActionMap();
      void buildDependenceGraph();
      void findIndependentTables(Stat& stat);

      ExprSet findId(const IR::Expression *expr);
      void visitExterns(const P4::MethodInstance *instance);
      
      bool preorder(const IR::PackageBlock *block) override;
      bool preorder(const IR::ControlBlock *block) override;
      bool preorder(const IR::P4Control *cont) override;
      bool preorder(const IR::BlockStatement *statement) override;
      bool preorder(const IR::IfStatement *statement) override;
      bool preorder(const IR::SwitchStatement *statement) override;
      bool preorder(const IR::MethodCallStatement *statement) override;
      bool preorder(const IR::AssignmentStatement *statement) override;
      bool preorder(const IR::ReturnStatement *) override;
      bool preorder(const IR::ExitStatement *) override;
      bool preorder(const IR::P4Table *table) override;
      bool preorder(const IR::ActionListElement *action) override;
      bool preorder(const IR::P4Action *action) override;
      bool preorder(const IR::KeyElement *key) override;

    private:
      P4::ReferenceMap *refMap; P4::TypeMap *typeMap;
      cstring fileName;
      Action *curAction;
      ActionMap *curActionMap;
      Table *curTable;
      TableStack *tableStack;
      Dependencies *dependencies;
      Graphs *graph;
  };

} //namespace multip4

#endif
