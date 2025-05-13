#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <set>
#include <vector>
#include <algorithm>

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
       val;
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
      semant_error(lookup(current_name)->get_node()) << "Class " << current_name << " cannot inherit class SELF_TYPE." << endl;
      break;
    }

    // fix floating nodes with no valid parents
    if (current_name != Object and parent == No_class) {
      classes_problem.insert(classes_seen.begin(), classes_seen.end());
      broken = true; 
      semant_error(lookup(current_name)->get_node()) << "Floating node" << endl; 
      break;
    }

    // undefined parent class
    if (lookup(parent) == nullptr) {
      classes_problem.insert(classes_seen.begin(), classes_seen.end());
      broken = true; 
      semant_error(lookup(current_name)->get_node()) << "Class " << current_name << " inherits from an undefined class " << parent << "." << endl; 
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
  EnvironmentP base_environment = new Environment(root_inheritance->get_node());
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
  EnvironmentP current_environment = new Environment(current_node, *last_environment);
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
  // first, type check features
  InheritanceNodeP curr_inheritance = lookup(class_name);
  Class_ curr_class = curr_inheritance->get_node();
  EnvironmentP curr_env = curr_inheritance->get_env();
  Features feature_list = curr_class->get_features();
  for (int i = feature_list->first() ; feature_list->more(i) ; i = feature_list->next(i)) {
    Feature curr_feature = feature_list->nth(i);
    curr_feature->type_check(this, curr_env);
  }
}

void ClassTable::type_check() {
  // start at object and recurse down its children
}


// method to add features from given class AST node
void Environment::add_features(Class_ curr_class) {
  Features features = curr_class->get_features();
  for (int j = features->first(); features->more(j); j = features->next(j)) { // go through each feature
    Feature curr_feature = features->nth(j);

    // if feature is a method, cast it to method_class* and add it to env
    if (curr_feature->is_method()) { 
      add_method(curr_feature->get_name(), (method_class*)curr_feature);
    } 

    // if feature is an attribute, cast it to attr_class* and add it to env
    else { 
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
	     single_Features(attr(val, prim_slot, no_expr())),
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
//    ostream& ClassTable::semant_error()
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


}
