#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include <list>

#define TRUE 1
#define FALSE 0

class InheritanceNode;

class InheritanceNode
{
private:
  Class_ node;
  Symbol name;
  Symbol parent;
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
  void set_env(Environment* curr_env) { env = curr_env; }
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
  std::ostream &error_stream;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  std::ostream &semant_error();
  std::ostream &semant_error(Class_ c);
  std::ostream &semant_error(Symbol filename, tree_node *t);
};

class Environment {
public:
  SymbolTable<Symbol, Symbol> objects_table;
  SymbolTable<Symbol, method_class> methods_table;
  Class_ current_class;

public:
  Environment(Class_ c) : current_class(c) {
    objects_table.enterscope();
    methods_table.enterscope();
  }

  // child inherits the environment from parent
  Environment(const Environment &parent) {
    objects_table = parent.objects_table;
    methods_table = parent.methods_table;
    current_class = parent.current_class;
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

  Symbol lookup_variable(Symbol name) {
    Symbol *result = objects_table.lookup(name);
    return result ? *result : nullptr;
  }

  method_class *lookup_method(Symbol name) {
    return methods_table.lookup(name);
  }

  Symbol get_class_type() {
    return current_class->get_name();
  }
};

#endif
