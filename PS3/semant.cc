#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <set>
#include <vector>
#include <algorithm>
#include "cool-tree.h"

extern int semant_debug;
extern char *curr_filename;
extern int node_lineno;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
       arg,
       arg2,
       Bool,
       concat,
       cool_abort,
       copy,
       Int,
       in_int,
       in_string,
       IO,
       isProto,
       length,
       Main,
       main_meth,
       No_class,
       No_type,
       _BOTTOM_,
       Object,
       out_int,
       out_string,
       prim_slot,
       self,
       SELF_TYPE,
       Str,
       str_field,
       substr,
       type_name,
       val,
       ERROR_RETURN;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void) {
  arg         = idtable.add_string("arg");
  arg2        = idtable.add_string("arg2");
  Bool        = idtable.add_string("Bool");
  concat      = idtable.add_string("concat");
  cool_abort  = idtable.add_string("abort");
  ::copy      = idtable.add_string("copy");
  Int         = idtable.add_string("Int");
  in_int      = idtable.add_string("in_int");
  in_string   = idtable.add_string("in_string");
  IO          = idtable.add_string("IO");
  isProto     = idtable.add_string("isProto");
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
  //   _no_class is a symbol that can't be the name of any
  //   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
  // _BOTTOM_ is the symbol for the bottom of the lattice of types
  _BOTTOM_    = idtable.add_string("_bottom");
  Object      = idtable.add_string("Object");
  out_int     = idtable.add_string("out_int");
  out_string  = idtable.add_string("out_string");
  prim_slot   = idtable.add_string("_prim_slot");
  self        = idtable.add_string("self");
  SELF_TYPE   = idtable.add_string("SELF_TYPE");
  Str         = idtable.add_string("String");
  str_field   = idtable.add_string("_str_field");
  substr      = idtable.add_string("substr");
  type_name   = idtable.add_string("type_name");
  val         = idtable.add_string("_val");
}

Symbol object_class::type_check(ClassTableP classtable, EnvironmentP env) {
  // "name" is objects name, look up in env
  env->add_variable(self, SELF_TYPE);
  Symbol* object_type = env->lookup_variable(name);
  if (object_type == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Undeclared identifier " << name << "." << endl;
    this->set_type(Object);
    return Object;
  }
  this->set_type(*object_type);
  return *object_type;
}


// TODO: ask about
Symbol no_expr_class::type_check(ClassTableP classtable, EnvironmentP env) {
  this->set_type(No_type);
  return No_type;
}

Symbol isvoid_class::type_check(ClassTableP classtable, EnvironmentP env) {
  e1->type_check(classtable, env);
  this->set_type(Bool);
  return Bool;
}

Symbol new__class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol type_name = this->type_name;
  Symbol current_class = env->get_class_type();
  if (type_name == SELF_TYPE) { 
    this->set_type(SELF_TYPE);
    return SELF_TYPE; 
  }
  else {
    if (classtable->lookup(type_name) == nullptr) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "'new' used with undefined class " << type_name << endl;
      this->set_type(Object);
      return Object;
    } else {
      this->set_type(type_name);
      return type_name;
    }
  }
}

Symbol string_const_class::type_check(ClassTableP classtable, EnvironmentP env) {
  this->set_type(Str);
  return Str;
}

Symbol bool_const_class::type_check(ClassTableP classtable, EnvironmentP env) {
  this->set_type(Bool);
  return Bool;
}

Symbol int_const_class::type_check(ClassTableP classtable, EnvironmentP env) {
  this->set_type(Int);
  return Int;
}

Symbol comp_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  if (e1_type != Bool) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Argument of 'not' has type " << e1_type << " instead of Bool." << endl;
  }
  this->set_type(Bool);
  return Bool;
}

Symbol leq_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);
  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " <= " << e2_type << endl;
  }
  this->set_type(Bool);
  return Bool;
}

Symbol lt_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);
  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " < " << e2_type << endl;
  }
  this->set_type(Bool);
  return Bool;
}

Symbol eq_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);
  // cout << "e1 type is " << e1_type << " . e2 type is " << e2_type << endl;
  bool basic1 = (e1_type == Int || e1_type == Bool || e1_type == Str);
  bool basic2 = (e2_type == Int || e2_type == Bool || e2_type == Str);

  if ((!basic1 and basic2) or (basic1 and !basic2)) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Illegal comparison with a basic type." << endl;
  }

  else if (basic1 and basic2 and (e1_type != e2_type)) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Argument 1 is " << e1_type << " and argument 2 is " << e2_type << " . They are not of the same type and cannot be compared." << endl;
  }

  this->set_type(Bool);
  return Bool;
}

Symbol neg_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol expression_type = e1->type_check(classtable, env);
  if (expression_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Argument of '~' has type " << expression_type << " instead of Int." << endl;
  }
  this->set_type(Int);
  return Int;
}

Symbol divide_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);

  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " / " << e2_type << endl;
  }
  this->set_type(Int);
  return Int;
}

Symbol mul_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);

  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " * " << e2_type << endl;
  }
  this->set_type(Int);
  return Int;
}

Symbol sub_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);

  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " - " << e2_type << endl;
  }
  this->set_type(Int);
  return Int;
}

Symbol plus_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol e1_type = e1->type_check(classtable, env);
  Symbol e2_type = e2->type_check(classtable, env);
  
  if (e1_type != Int || e2_type != Int) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "non-Int arguments: " << e1_type << " + " << e2_type << endl;
  }
  this->set_type(Int);
  return Int;
}

// Static type of the block is the static type of the last expression (from Cool manual sec 7.7)
Symbol block_class::type_check(ClassTableP classtable, EnvironmentP env) {
  if (body == NULL) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "This block has no body" << endl;
    return No_type; //TODO: do we return No_type or Object?
  }
  Symbol type = Object;
  for(int i = body->first(); body->more(i); i = body->next(i)) {
    type = body->nth(i)->type_check(classtable, env);
  }
  this->set_type(type);
  return type;
}

// Cool manual sec 7.9
Symbol typcase_class::type_check(ClassTableP classtable, EnvironmentP env) {
  bool broken = false;
  // TOOD: is void the same as NULL?
  Symbol expr_type = expr->type_check(classtable, env);
  if (expr_type == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "This expression has no valid type" << endl;
    return Object;
  }
  // every case must have at least one branch
  if (cases == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Case expressions has no branch" << endl;
    return Object;
  }

  std::set<Symbol> seen_types;
  std::vector<Symbol> branch_expr_types;
  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    Case curr_branch = cases->nth(i);
    Symbol branch_type = curr_branch->get_type_decl();

    // cout << "currently looking at line number " << curr_branch->get_line_number() << endl;

    // ensure that no branch has duplicate type
    if (seen_types.count(branch_type)) {
      classtable->semant_error(env->get_class_node()->get_filename(), curr_branch) << "Duplicate branch " << branch_type << " in case statement." << endl;
      // return Object; //TODO: should i do this here? prob right? since the end return statement won't exist for this case
      broken = true;
    } else {
      seen_types.insert(branch_type);
    }

    if (branch_type != SELF_TYPE && classtable->lookup(branch_type) == nullptr) {
      classtable->semant_error(env->get_class_node()->get_filename(), curr_branch) << "Class " << branch_type << " of case branch is undefined." << endl;
      // return Object; //TODO: should i do this here? same question as above
      broken = true;
      continue;
    }

    // add seen branch type to vector branch_expr_types
    Symbol branch_expr_type = curr_branch->type_check(classtable, env);
    branch_expr_types.push_back(branch_expr_type);
  }

  if (broken) { return Object; }


  // The static type of a case expression is lub(all branches)
  Symbol case_expr_type = branch_expr_types[0];
  for (size_t i = 1; i < branch_expr_types.size(); i++) {
    case_expr_type = classtable->lub(case_expr_type, branch_expr_types[i], env); //TODO: error for lub? where to call this? is Object the lub?
  }
  this->set_type(case_expr_type);
  return case_expr_type;
}

Symbol branch_class::type_check(ClassTableP classtable, EnvironmentP env) {
  if (type_decl != SELF_TYPE && classtable->lookup(type_decl) == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Branch declared with undefined type" << endl;
  }
  env->enter_scope();
  env->add_variable(name, type_decl);
  Symbol type = expr->type_check(classtable, env);
  env->exit_scope();
  return type;
}

Symbol loop_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol pred_type = pred->type_check(classtable, env);
  if (pred_type != Bool) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Loop condition does not have type Bool." << endl;
  }
  body->type_check(classtable, env);
  this->set_type(Object);
  return Object;
}

Symbol cond_class::type_check(ClassTableP classtable, EnvironmentP env) {
  // pred must have type Bool
  Symbol pred_type = pred->type_check(classtable, env);
  if (pred_type != Bool) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Predicate of 'if' does not have type Bool." << endl;
    return Object;
  }

  Symbol then_type = then_exp->type_check(classtable, env);
  Symbol else_type = else_exp->type_check(classtable, env);

  Symbol cond_type = classtable->lub(then_type, else_type, env);
  this->set_type(cond_type);
  return cond_type;
}

Symbol let_class::type_check(ClassTableP classtable, EnvironmentP env) {
  env->enter_scope();
  //  The type of let is the type of the body

  if (type_decl != SELF_TYPE && classtable->lookup(type_decl) == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Class " <<  type_decl << " of let-bound identifier " << identifier << " is undefined." << endl;
    env->add_variable(identifier, type_decl);
    init->type_check(classtable, env);
    body->type_check(classtable, env);
    return Object;
  }

  Symbol init_type = init->type_check(classtable, env);
  if (init_type != No_type && !classtable->is_ancestor(init_type, type_decl, env)) { //init_type is a child of type_decl
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Initializer type does not match declared type" << endl;
    return Object;
  }

  env->add_variable(identifier, type_decl);
  Symbol let_body_type = body->type_check(classtable, env);
  this->set_type(let_body_type); //set the final type of let statement in AST
  env->exit_scope();
  return let_body_type;
}

Symbol dispatch_class::type_check(ClassTableP classtable, EnvironmentP env) {
  // type of e0
  Symbol expr_type = expr->type_check(classtable, env);
  Symbol dispatch_class;
  if (expr_type == SELF_TYPE) {
    dispatch_class = env->get_class_type();
  } else {
    dispatch_class = expr_type;
  }

  // Check if class exists
  InheritanceNodeP curr_node_class = classtable->lookup(dispatch_class);
  if (curr_node_class == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Dispatch on undefined class " << dispatch_class << "." << endl;
    this->set_type(Object);
    return Object;
  }
  // Check if method exists in class
  method_class* method = curr_node_class->get_env()->lookup_method(name);
  if (method == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Dispatch to undefined method " << name << "." << endl;
    this->set_type(Object);
    return Object;
  }

  //  the dispatch and the definition of f must have the same number of arguments
  Formals formals = method->get_formals();
  if (formals->len() != actual->len()) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Method " << name << " called with wrong number of arguments." << endl;
    this->set_type(Object);
    return Object;
  }
  
  //  the static type of the ith actual parameter must conform to the declared type of the ith formal parameter
  for(int i = actual->first(), j = formals->first(); actual->more(i); i = actual->next(i), j = formals->next(j)) {
    Symbol actual_type = actual->nth(i)->type_check(classtable, env);
    Symbol formal_type = formals->nth(j)->get_formal_type();
    if (!classtable->is_ancestor(actual_type, formal_type, env)) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "In call of method " << name << ", type " << actual_type << " of parameter " << formals->nth(j)->get_formal_name() << " does not conform to declared type " << formal_type << "." << endl;
    }
  }
  
  // If f has return type B and B is a class name, then the static type of the dispatch is B. 
  // Otherwise, if f has return type SELF_TYPE, then static type of the dispatch is A.
  Symbol method_return_type = method->get_return_type();
  Symbol final_type;

  if (method_return_type != SELF_TYPE) {
    final_type = method_return_type;  // B (refer to Cool manual)
  } else {
    final_type = expr_type;  // A (refer to Cool manual)
  }
  this->set_type(final_type);
  return final_type;
}

Symbol static_dispatch_class::type_check(ClassTableP classtable, EnvironmentP env) {
  // type of e0
  Symbol expr_type = expr->type_check(classtable, env);
  Symbol dispatch_class;
  if (expr_type == SELF_TYPE) {
    dispatch_class = env->get_class_type();
  } else {
    dispatch_class = expr_type;
  }

  // Check if class exists
  InheritanceNodeP T_node_class = classtable->lookup(type_name);
  if (T_node_class == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Dispatch on undefined class " << dispatch_class << "." << endl;
    this->set_type(Object);
    return Object;
  }
  // Check if e0 class is a child of T
  if (!classtable->is_ancestor(dispatch_class, type_name, env)) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Expression type " << dispatch_class << " does not conform to declared static dispatch type " << type_name << "." << endl;
  }
  // Check if method exists in class
  method_class* method = T_node_class->get_env()->lookup_method(name);
  if (method == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Static dispatch to undefined method " << name << "." << endl;
    this->set_type(Object);
    return Object;
  }
  // Check if parameters match
  Formals formals = method->get_formals();
  if (formals->len() != actual->len()) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Method " << name << " called with wrong number of arguments." << endl;
  }
  //  the static type of the ith actual parameter must conform to the declared type of the ith formal parameter
  for(int i = actual->first(), j = formals->first(); actual->more(i); i = actual->next(i), j = formals->next(j)) {
    Symbol actual_type = actual->nth(i)->type_check(classtable, env);
    Symbol formal_type = formals->nth(j)->get_formal_type();
    if (!classtable->is_ancestor(actual_type, formal_type, env)) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "In call of method " << name << ", type " << actual_type << " of parameter " << formals->nth(j)->get_formal_name() << " does not conform to declared type " << formal_type << "." << endl;
    }
  }
  // If f has return type B and B is a class name, then the static type of the dispatch is B. 
  // Otherwise, if f has return type SELF_TYPE, then static type of the dispatch is A.
  Symbol method_return_type = method->get_return_type();
  Symbol final_type;

  if (method_return_type != SELF_TYPE) {
    final_type = method_return_type;  // B (refer to Cool manual)
  } else {
    final_type = expr_type;  // A (refer to Cool manual)
  }
  this->set_type(final_type);
  return final_type;
}



// STOPPED HERE

Symbol assign_class::type_check(ClassTableP classtable, EnvironmentP env) {
  Symbol* object_type = env->lookup_variable(name);
  if (object_type == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Assignment to undeclared variable "<< name << "." << endl;
    return Object;  //TODO: do we need this? // no cascading error by defining a bottom_type (global variable)
  }
  Symbol expression_type = expr->type_check(classtable, env);
  if (!classtable->is_ancestor(expression_type, *object_type, env)) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "expr and variable dont match" << endl;
    return Object;  //TODO: do we need this?
  }
  this->set_type(expression_type);
  return expression_type;
}

// TODO: confirm init can be No_type
void attr_class::type_check(ClassTableP classtable, EnvironmentP env) {
  if (type_decl != SELF_TYPE && classtable->lookup(type_decl) == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "In attr_class " << name << " type not defined" << endl;
  }
  Symbol init_type = init->type_check(classtable, env); //init does not have to have an expression
  if (init_type == ERROR_RETURN) { return; }
  if (init_type!= No_type && type_decl != init_type) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "type not consistent" << endl;
  }
}

void method_class::type_check(ClassTableP classtable, EnvironmentP env) {
  env->enter_scope();
  env->add_variable(self, SELF_TYPE);

  // return type is defined
  if (return_type != SELF_TYPE && classtable->lookup(return_type) == nullptr) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Return type " << return_type << " of method " << name << " is not a valid type" << endl;
  }

  // identifiers used in the formal parameter list must be distinct
  std::set<Symbol> formals_so_far;
  for (int i = formals->first() ; formals->more(i) ; i = formals->next(i)) {
    Formal curr = formals->nth(i);
    Symbol curr_type = curr->get_formal_type();
    Symbol curr_name = curr->get_formal_name();

    // check for duplicate parameter identifiers
    if (formals_so_far.count(curr_name) > 0) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "There are two of the same formal parameter " << curr_name << " in method " << name << endl;
    }
    else {
      formals_so_far.insert(curr_name);
    }

    // check for undefined formal parameters
    if (curr_type != SELF_TYPE && classtable->lookup(curr_type) == nullptr) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "Formal parameter type " << curr_type << " in method " << name << " is undefined "<< endl;
    }

    // formal parameters cannot be named "self"
    if (curr_name == self) {
      classtable->semant_error(env->get_class_node()->get_filename(), this) << "Method " << name << " has a formal parameter named 'self' which is not allowed." << endl;
    }

    env->add_variable(curr_name, curr_type);
  }

  // type of the method body must conform to the declared return type
  // body_type <= return_type
  Symbol final_return_type = return_type;
  Symbol body_type = expr->type_check(classtable, env);
  if (final_return_type == SELF_TYPE) {
    final_return_type = env->get_class_type();
  }
  if (body_type == SELF_TYPE) {
    body_type = env->get_class_type();
  }
  if (body_type != No_type && !classtable->is_ancestor(body_type, final_return_type, env)) {
    classtable->semant_error(env->get_class_node()->get_filename(), this) << "Method " << name << " has a return type " << return_type << " that does not conform to body type " << body_type << endl;
  }
  env->exit_scope();

  return;
}

// void formal_class::type_check(ClassTableP classtable, EnvironmentP env) {
//   return;
// }

// Lub - least upper bound for two classes
Symbol ClassTable::lub(Symbol class1, Symbol class2, EnvironmentP env) {
  if (class1 == nullptr || class2 == nullptr) {
    semant_error() << "trying to find the least upper bound of a non-existent class" << endl;
    return Object;
  } 
  if (class1 == class2) { return class1; }
  if (class1 == SELF_TYPE) { class1 = env->get_class_type(); };
  if (class2 == SELF_TYPE) { class2 = env->get_class_type(); };

  // Add all ancestors of class1 to a set, named ancestors1
  std::set<Symbol> ancestors1;
  InheritanceNodeP node1 = lookup(class1);
  while (node1 != nullptr) {
    ancestors1.insert(node1->get_name());
    Symbol node1_parent = node1->get_parent();
    node1 = lookup(node1_parent);
  }

  // Go through ancestors of class2 in order
  // Returns when its ancestor is in the set of ancestors1
  InheritanceNodeP node2 = lookup(class2);
  while (node2 != nullptr) {
    if (ancestors1.count(node2->get_name()) > 0) {
      return node2->get_name();
    }
    Symbol node2_parent = node2->get_parent();
    node2 = lookup(node2_parent);
  }

  // semant_error(env->get_class_node()->get_filename(), this) << "least upper bound does not exist" << endl;
  return Object;
}


bool ClassTable::is_ancestor(Symbol child, Symbol parent, EnvironmentP env) {
  // traverse through child's parents
  if (child == NULL || parent == NULL) {
    return false;
  }
  if (child == SELF_TYPE) child = env->get_class_type();
  if (parent == SELF_TYPE) parent = env->get_class_type();

  Symbol curr_node = child;
  while (true) {
    if (curr_node == parent) {
      return true;
    }
    if (curr_node == No_class) {
      return false;
    }

    InheritanceNodeP curr_inheritance = lookup(curr_node);
    if (curr_inheritance == nullptr) {
      semant_error() << "When checking for inheritance, type " << curr_node << " does not exist" << endl;
      return false;
    }

    curr_node = curr_inheritance->get_parent();
  }
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {
  enterscope();
  
  // install base classes
  install_basic_classes();
  
  // install user-added new classes
  install_new_classes(classes);
  if (abort) { return; }
  
  // check inheritance
  check_inheritance(classes);
}

void ClassTable::install_new_classes(Classes classes) {
  for (int i = classes->first() ; classes->more(i) ; i = classes->next(i)) {
    Class_ current = classes->nth(i);
    Symbol current_name = current->get_name();

    if(lookup(current_name) != nullptr) {
      // throw error if node is alr in inheritance graph
      semant_error(current) << "Class " << current_name << " was previously defined." << endl;
    } else {
      InheritanceNodeP append = new InheritanceNode(current);

      if (append->get_parent() == Str or append->get_parent() == Bool or append->get_parent() == Int) {
        semant_error(current) << "Class " << current_name << " cannot inherit class " << append->get_parent() << "." << endl;
      }

      addid(current_name, append);
    }
  }

  
  if (lookup(Main) == nullptr) { semant_error() << "Class Main is not defined." << endl; }
}




// duplicates 
void ClassTable::check_inheritance(Classes classes) {
  // run my inheritance checking through the map
  // here is my premature plan:
  // FOR EACH class in our symbol table:
  // 1. traverse up its parents and keep track of the things weve seen
  // 2. if we see something we have already seen then that means we have a loop somehow, throw an error
  // 3. if we get all the way up to no_class, then we're good

  // if we get to a class which we know terminates correctly in our tree, we can immediately terminate 
  std::set<Symbol> classes_confirmed;
  std::set<Symbol> classes_problem;
  bool broken = false;
  for (int i = classes->first() ; classes->more(i) ; i = classes->next(i)) {
    Class_ current = classes->nth(i);
    Symbol current_name = current->get_name();
    Symbol parent = current->get_parent();
    std::set<Symbol> classes_seen;

    // self type parent is invalid
    if (parent == SELF_TYPE) {
      classes_problem.insert(classes_seen.begin(), classes_seen.end());
      broken = true; 
      if (lookup(current_name)) { semant_error(lookup(current_name)->get_node()) << "Class " << current_name << " cannot inherit class SELF_TYPE." << endl; }
      break;
    }

    // fix floating nodes with no valid parents
    if (current_name != Object and parent == No_class) {
      classes_problem.insert(classes_seen.begin(), classes_seen.end());
      broken = true; 
      if (lookup(current_name)) { semant_error(lookup(current_name)->get_node()) << "Floating node" << endl; }
      break;
    }

    // undefined parent class
    if (lookup(parent) == nullptr) {
      classes_problem.insert(classes_seen.begin(), classes_seen.end());
      broken = true; 
      if (lookup(current_name)) { semant_error(lookup(current_name)->get_node()) << "Class " << current_name << " inherits from an undefined class " << parent << "." << endl; }
      break;
    }

    // add children if parent is defined
    if (parent != No_class) {
      InheritanceNodeP parent_node = lookup(parent);
      parent_node->add_child(current_name);
    }

    while(true) {
      // check if node has been seen already, or if the current node is a known bad node
      // If this is not already a known , the program is broken and throw an error
      if (classes_seen.count(current_name) != 0 or classes_problem.count(current_name) != 0) { 
        classes_problem.insert(classes_seen.begin(), classes_seen.end());
        broken = true; 
        semant_error(current) << "Class " << current->get_name() << ", or an ancestor of " << current->get_name() << ", is involved in an inheritance cycle." << endl; 
        break; 
      } 
      if (current_name == No_class or classes_confirmed.count(current_name) != 0) { 
        // add seen nodes to list of safe nodes
        classes_confirmed.insert(classes_seen.begin(), classes_seen.end());
        break;
      }
      // otherwise insert the current node into seen nodes
      classes_seen.insert(current_name);

      InheritanceNodeP current_inheritance = lookup(current_name);
      if (current_inheritance == nullptr) { break; }
      // find the current node's parent
      Symbol current_parent = current_inheritance->get_parent();
      if (current_parent == SELF_TYPE) { break; }

      // iterate over parent
      current_name = current_parent;
    }
  }

  // no need to populate environments if broken
  if (broken) { return; }


  // CHECK KIDS
  // for (int i = classes->first() ; classes->more(i) ; i = classes->next(i)) {
  //   cout << "Class " << classes->nth(i)->get_name() << " has children: " << endl;
  //   InheritanceNodeP node = lookup((classes->nth(i))->get_name());
  //   if (node == nullptr) { continue; }
  //   std::vector<Symbol> kids = node->get_children();
  //   for (long unsigned int j = 0 ; j < kids.size() ; j++) {
  //     cout << kids[j] << endl;
  //   }
  // }



// Traverse the class list: 
  // Add an environment structure as an entry to your class table 
  // Build a method table - what redifinitions of methods are valid, ... 
  // Build an attribute table - attr and type declerations 

// def type_check(class): 
//     features 
//     child nodes
// for feature in features: feature->type_check()
// for each child in child_nodes, type_check(child)

// feature
    // method_class::type_check()
    // enterscope - add formals 
    // exitscope
    // attr_class::type_check()







  



    // change this function so that it starts at object and creates environment for every class and works down from there
  







  /*

  // Create an environment for each class
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Class_ curr_class = classes->nth(i);
    Environment* curr_env = new Environment(curr_class);
    InheritanceNodeP node = lookup(curr_class->get_name());
  
    // Takes parent node's environment
    Symbol parent_name = node->get_parent();
    if (parent_name != No_class && parent_name != SELF_TYPE) {
      InheritanceNodeP parent_node = lookup(parent_name);
      if (parent_node && parent_node->get_env()) {
        *curr_env = Environment(*parent_node->get_env());
      }
    }
  
    // Populate env with current class' methods and attributes
    Features features = curr_class->get_features();
    for (int j = features->first(); features->more(j); j = features->next(j)) { // go through each feature
      Feature curr_feature = features->nth(j);

      // if feature is a method, cast it to method_class* and add it to env
      if (curr_feature->is_method()) { 
        curr_env->add_method(curr_feature->get_name(), (method_class*)curr_feature);
      } 

      // if feature is an attribute, cast it to attr_class* and add it to env
      else { 
        attr_class* new_attribute = (attr_class*)curr_feature;
        curr_env->add_variable(new_attribute->get_name(), new_attribute->get_type_decl());
      }
    }
    // Set current class's env to curr_env
    node->set_env(curr_env);
  }

  */
}


// SEPERATE ENVIRONMENTS

void ClassTable::create_environments() {
  // start at object, and recurse from there
  InheritanceNodeP root_inheritance = lookup(Object);
  std::vector<Symbol> root_children = root_inheritance->get_children();
  EnvironmentP base_environment = new Environment(root_inheritance->get_node(), this);
  root_inheritance->set_env(base_environment);

  for (long unsigned int i = 0 ; i < root_children.size() ; i++) {
    Symbol child = root_children[i];
    create_environments(child, base_environment);
  }
}

void ClassTable::create_environments(Symbol class_name, EnvironmentP last_environment) {
  InheritanceNodeP current_inheritance = lookup(class_name);
  std::vector<Symbol> current_children = current_inheritance->get_children();
  Class_ current_node = current_inheritance->get_node();
  EnvironmentP current_environment = new Environment(current_node, *last_environment, this);
  current_environment->add_features(current_node, this);
  current_inheritance->set_env(current_environment);

  for (long unsigned int i = 0 ; i < current_children.size() ; i++) {
    Symbol child = current_children[i];
    create_environments(child, current_environment);
  }
}


// SCOPING (ITH THIS IS BETTER ?)

// void ClassTable::create_environments() {
//   // start at object, and recurse from there
//   InheritanceNodeP root_node = lookup(Object);
//   std::vector<Symbol> root_children = root_node->get_children();
//   EnvironmentP base_environment = new Environment(root_node->get_node());
//   root_node->set_env(base_environment);

//   for (long unsigned int i = 0 ; i < root_children.size() ; i++) {
//     Symbol child = root_children[i];
//     create_environments(child, base_environment);
//   }
// }

// void ClassTable::create_environments(Symbol class_name, EnvironmentP environment) {
//   InheritanceNodeP current_inheritance = lookup(class_name);
//   std::vector<Symbol> current_children = current_inheritance->get_children();
//   environment->enter_scope();
//   Class_ current_node = current_inheritance->get_node();
//   environment->add_features(current_node);
//   current_inheritance->set_env(environment);

  
//   for (long unsigned int i = 0 ; i < current_children.size() ; i++) {
//     Symbol child = current_children[i];
//     create_environments(child, environment);
//   }
// }


void ClassTable::type_check_class(Symbol class_name) {
  InheritanceNodeP curr_inheritance = lookup(class_name);
  if (!curr_inheritance) { return; }
  Class_ curr_class = curr_inheritance->get_node();
  EnvironmentP curr_env = curr_inheritance->get_env();
  Features feature_list = curr_class->get_features();

  for (int i = feature_list->first() ; feature_list->more(i) ; i = feature_list->next(i)) {
    Feature curr_feature = feature_list->nth(i);
    curr_feature->type_check(this, curr_env);
  }
}

void ClassTable::type_check() {
  // Start at Object
  if(!lookup(Object)) { return; }

  // Recurse down the children of Object
  std::vector<Symbol> q = { Object };
  while (!q.empty()) {
    Symbol current_class = q.back();
    q.pop_back();
    if (current_class != Object && current_class != Int && current_class != Bool && current_class != Str && current_class != IO) {
      type_check_class(current_class);
    }

    InheritanceNodeP node = lookup(current_class);
    if (!node) { continue; }

    std::vector<Symbol> children = node->get_children();
    q.insert(q.end(), children.begin(), children.end());
  }
}


// method to add features from given class AST node
// TODO: add classtable, self
void Environment::add_features(Class_ curr_class, ClassTableP classtable) {
  Features features = curr_class->get_features();
  std::set<Symbol> current_class_methods;
  std::set<Symbol> current_class_attributes;

  for (int j = features->first(); features->more(j); j = features->next(j)) { // go through each feature
    Feature curr_feature = features->nth(j);

    // if feature is a method, cast it to method_class* and add it to env
    if (curr_feature->is_method()) { 
      // check if method is already in method table
      Symbol method_name = curr_feature->get_name();
      if (current_class_methods.count(method_name)) {
        classtable->semant_error(curr_class->get_filename(), curr_feature) << "Method " << method_name << " is multiply defined in class " << curr_class->get_name() << endl;
        continue;
      }

      current_class_methods.insert(method_name);

      method_class* parent_method = lookup_method(method_name);
      method_class* child_method = (method_class*)curr_feature;

      // check both methods have the same # of parameters, parameter types, and return types
      if (parent_method != nullptr) {
        if (current_class_attributes.count(method_name)) {
          classtable->semant_error(curr_class->get_filename(), curr_feature) << "Attribute " << method_name << " is multiply defined in class " << curr_class->get_name() << endl;
          continue;
        }

        if (parent_method->get_return_type() != child_method->get_return_type()) {
          classtable->semant_error(curr_class->get_filename(), curr_feature) << "Redefinition of function has different return type" << endl;
          continue;
        }

        Formals parent_formals = parent_method->get_formals();
        Formals child_formals = child_method->get_formals();
        if (parent_formals->len() != child_formals->len()) {
          classtable->semant_error(curr_class->get_filename(), curr_feature) << "Redefinition of function has different number of parameters" << endl;
          continue;
        }

        for(int i = parent_formals->first(); parent_formals->more(i); i = parent_formals->next(i)) {
          if (parent_formals->nth(i)->get_formal_type() != child_formals->nth(i)->get_formal_type()) {
            classtable->semant_error(curr_class->get_filename(), curr_feature) << "Redefinition of function has different parameter types" << endl;
            break;
          }
        }
      }
  
      add_method(method_name, child_method);
    } 

    // if feature is an attribute, cast it to attr_class* and add it to env
    else { 
      // Check if variable already declared
      Symbol attr_name = curr_feature->get_name();
      if (current_class_attributes.count(attr_name)) {
        classtable->semant_error(curr_class->get_filename(), curr_feature) << "Attribute " << attr_name << " is multiply defined in class " << curr_class->get_name() << endl;
        continue;
      }
      current_class_attributes.insert(attr_name);

      // Add it if it doesn't already exist
      attr_class* new_attribute = (attr_class*)curr_feature;
      add_variable(new_attribute->get_name(), new_attribute->get_type_decl());
    }
  }
}



void ClassTable::install_basic_classes() {
  // The tree package uses these globals to annotate the classes built below.
  node_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

  // The following demonstrates how to create dummy parse trees to
  // refer to basic Cool classes.  There's no need for method
  // bodies -- these are already built into the runtime system.

  // IMPORTANT: The results of the following expressions are
  // stored in local variables.  You will want to do something
  // with those variables at the end of this method to make this
  // code meaningful.


  //
  // The Object class has no parent class. Its methods are
  //        cool_abort() : Object    aborts the program
  //        type_name() : Str        returns a string representation of class name
  //        copy() : SELF_TYPE       returns a copy of the object
  //
  // There is no need for method bodies in the basic classes---these
  // are already built in to the runtime system.
  //

  Class_ Object_class =
    class_(Object,
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(::copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename);

  //
  // The IO class inherits from Object. Its methods are
  //        out_string(Str) : SELF_TYPE          writes a string to the output
  //        out_int(Int) : SELF_TYPE               "    an int    "  "     "
  //        in_string() : Str                    reads a string from the input
  //        in_int() : Int                         "   an int     "  "     "
  //

  Class_ IO_class =
     class_(IO,
            Object,
            append_Features(
            append_Features(
            append_Features(
            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                        SELF_TYPE, no_expr())),
            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                        SELF_TYPE, no_expr()))),
            single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	    filename);

  //
  // The Int class has no methods and only a single attribute, the
  // "val" for the integer.
  //

  Class_ Int_class =
      class_(Int,
	     Object,
	     single_Features(attr(val, prim_slot, no_expr())), // TODO: ask Gautham
	     filename);

  //
  // Bool also has only the "val" slot.
  //

  Class_ Bool_class =
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

  //
  // The class Str has a number of slots and operations:
  //       val                                  the length of the string
  //       str_field                            the string itself
  //       length() : Int                       returns length of the string
  //       concat(arg: Str) : Str               performs string concatenation
  //       substr(arg: Int, arg2: Int): Str     substring selection
  //

  Class_ Str_class =
      class_(Str,
	     Object,
             append_Features(
             append_Features(
             append_Features(
             append_Features(
             single_Features(attr(val, Int, no_expr())),
            single_Features(attr(str_field, prim_slot, no_expr()))),
            single_Features(method(length, nil_Formals(), Int, no_expr()))),
            single_Features(method(concat,
				   single_Formals(formal(arg, Str)),
				   Str,
				   no_expr()))),
	    single_Features(method(substr,
				   append_Formals(single_Formals(formal(arg, Int)),
						  single_Formals(formal(arg2, Int))),
				   Str,
				   no_expr()))),
	     filename);

       // Install basic classes into class table, add them with inhertianceNode so that we can traverse inheritance later
      InheritanceNodeP Object_inheritance = new InheritanceNode(Object_class);
      // Manually set Object's base children here
      Object_inheritance->add_child(IO); Object_inheritance->add_child(Int); Object_inheritance->add_child(Bool); Object_inheritance->add_child(Str);   
      addid(Object, Object_inheritance);
      InheritanceNodeP IO_inheritance = new InheritanceNode(IO_class);
      addid(IO, IO_inheritance);
      InheritanceNodeP Int_inheritance = new InheritanceNode(Int_class);
      addid(Int, Int_inheritance);
      InheritanceNodeP Bool_inheritance = new InheritanceNode(Bool_class);
      addid(Bool, Bool_inheritance);
      InheritanceNodeP Str_inheritance = new InheritanceNode(Str_class);
      addid(Str, Str_inheritance);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error(env->get_class_node()->get_filename(), this)
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//       (line number is extracted from tree_node)
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{
  return semant_error(c->get_filename(),c);
}

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
  error_stream << filename << ":" << t->get_line_number() << ": ";
  return semant_error();
}

ostream& ClassTable::semant_error()
{
    semant_errors++;
    return error_stream;
}


/*
 * This is the entry point to the semantic checker.
 *
 * Your checker should do the following two things:
 *
 *   1) Check that the program is semantically correct
 *   2) Decorate the abstract syntax tree with type information
 *      by setting the `type' field in each Expression node.
 *      (see `tree.h')
 *
 *   You are free to first do 1), make sure you catch all semantic
 *   errors. Part 2) can be done in a second stage, when you want
 *   to build mycoolc.
 */
void program_class::semant() {
   initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
   ClassTableP classtable = new ClassTable(classes);

   if (classtable->errors()) {
      cerr << "Compilation halted due to static semantic errors." << endl;
      exit(1);
   }

   classtable->create_environments();
   classtable->type_check();

   if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
   }

}