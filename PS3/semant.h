#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include <list>
#include <vector>

#define TRUE 1
#define FALSE 0

class InheritanceNode;
class Environment;
typedef Environment *EnvironmentP;

class InheritanceNode
{
private:
  Class_ node;
  Symbol name;
  Symbol parent;
  std::vector<Symbol> children;
  Environment* env;

public:
  InheritanceNode(Class_ node)
  {
    name = node->get_name();
    parent = node->get_parent();
    this->node = node;
    env = nullptr;
  }

  Symbol get_name() { return name; }
  Symbol get_parent() { return parent; }
  Class_ get_node() { return node; }
  Environment* get_env() { return env; }
  void set_env(EnvironmentP curr_env) { env = curr_env; }
  void add_child(Symbol child) { children.push_back(child); }
  std::vector<Symbol> get_children() { return children; }
};

typedef InheritanceNode *InheritanceNodeP;
class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.
class ClassTable : public SymbolTable<Symbol, InheritanceNode>
{
private:
  int semant_errors; // counts the number of semantic errors
  void install_basic_classes();
  void install_new_classes(Classes classes);
  void check_inheritance(Classes classes);
  void create_environments(Symbol class_name, EnvironmentP environment);
  void check_parents(Classes classes);
  void check_main();
  void abort();
  std::ostream &error_stream;
  void type_check_class(Symbol class_name);

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  std::ostream &semant_error();
  std::ostream &semant_error(Class_ c);
  std::ostream &semant_error(Symbol filename, tree_node *t);
  void create_environments();
  void type_check();
  bool is_ancestor(Symbol child, Symbol parent, EnvironmentP env);
  Symbol lub(Symbol class1, Symbol class2, EnvironmentP env);
};

class Environment {
public:
  SymbolTable<Symbol, Symbol> objects_table;
  SymbolTable<Symbol, method_class> methods_table;
  Class_ current_class;
  ClassTableP classtable;

public:
  Environment(Class_ c, ClassTableP classt) : current_class(c), classtable(classt) {
    enter_scope();
    add_features(c, classt);
  }

  // child inherits the environment from parent
  Environment(Class_ c, const Environment &parent, ClassTableP classt) : current_class(c), classtable(classt) {
    enter_scope();
    objects_table = parent.objects_table;
    methods_table = parent.methods_table;

    add_features(c, classt);
  }

  void enter_scope() {
    objects_table.enterscope();
    methods_table.enterscope();
  }

  void exit_scope() {
    objects_table.exitscope();
    methods_table.exitscope();
  }

  void add_variable(Symbol name, Symbol type) {
    objects_table.addid(name, new Symbol(type));
  }

  void add_method(Symbol name, method_class *method) {
    methods_table.addid(name, method);
  }

  Symbol* lookup_variable(Symbol name) {
    return objects_table.lookup(name);
  }

  method_class* lookup_method(Symbol name) {
    return methods_table.lookup(name);
  }

  Symbol get_class_type() {
    return current_class->get_name();
  }

  Class_ get_class_node() const { return current_class; }

  void add_features(Class_ curr_class, ClassTableP classtable);
};

#endif
