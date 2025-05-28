
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_supp.h"
#include "handle_flags.h"

//
// Two symbols from the semantic analyzer (semant.cc) are used.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
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
       length,
       Main,
       main_meth,
       No_class,
       No_type,
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
static void initialize_constants(void)
{
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
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
//   _no_class is a symbol that can't be the name of any
//   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
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

static const char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static const char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };

//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//
// Note that Spim wants comments to start with '#'. For example:
// os << "# start of generated code\n";
// os << "\n# end of generated code\n";
//
//*********************************************************
void program_class::cgen(ostream &os) {
   initialize_constants();
   CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);
}

//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

// Added for consistent label support.
static std::string get_label_ref(int l)
{ std::stringstream ss;
  ss << l;
  std::string lbl = "label" + ss.str();
  return lbl;
}

static void emit_load(const char *dest_reg, int offset, const char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")"
    << std::endl;
}

static void emit_store(const char *source_reg, int offset, const char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << std::endl;
}

static void emit_load_imm(const char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << std::endl; }

static void emit_load_address(const char *dest_reg, const char *address, ostream& s)
{ s << LA << dest_reg << " " << address << std::endl; }

static void emit_partial_load_address(const char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(const char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << std::endl;
}

static void emit_load_string(const char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << std::endl;
}

static void emit_load_int(const char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << std::endl;
}

static void emit_move(const char *dest_reg, const char *source_reg, ostream& s)
{
  s << MOVE << dest_reg << " " << source_reg << std::endl;
}

static void emit_neg(const char *dest, const char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << std::endl; }

static void emit_add(const char *dest, const char *src1, const char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << std::endl; }

static void emit_addu(const char *dest, const char *src1, const char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << std::endl; }

static void emit_addiu(const char *dest, const char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << std::endl; }

static void emit_div(const char *dest, const char *src1, const char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << std::endl; }

static void emit_mul(const char *dest, const char *src1, const char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << std::endl; }

static void emit_sub(const char *dest, const char *src1, const char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << std::endl; }

static void emit_sll(const char *dest, const char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << std::endl; }

static void emit_jalr(const char *dest, ostream& s)
{ s << JALR << "\t" << dest << std::endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << std::endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << std::endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << get_label_ref(l); }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << std::endl;
}

static void emit_beqz(const char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_beq(const char *src1, const char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_bne(const char *src1, const char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_bleq(const char *src1, const char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_blt(const char *src1, const char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_blti(const char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_bgti(const char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << std::endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << std::endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(const char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

static void emit_pop(const char *reg, ostream& str)
{
  emit_addiu(SP,SP,4,str);
  emit_load(reg,0,SP,str);
}


//
// Fetch the integer value in an Int object. Emits code to fetch the integer
// value of the Integer object pointed to by register source into the register dest
//
static void emit_fetch_int(const char *dest, const char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(const char *source, const char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }

static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(const char *source, ostream &s)
{
  if (strcmp(source, A1)) emit_move(A1, source, s);
  s << JAL << "_gc_check" << std::endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << std::endl;

  code_ref(s);
  s  << LABEL                                                               // label
     << WORD << stringclasstag << std::endl                                 // tag
     << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << std::endl // size
     << WORD;

  /***** Add dispatch information for class String ******/
  s << "String_dispTab" << std::endl;                                              // dispatch table
  s << WORD;  lensym->code_ref(s);  s << std::endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag) {
  for (auto entry : tbl) {
    entry.code_def(s, stringclasstag);
  }
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << std::endl;

  code_ref(s);
  s << LABEL                                // label
    << WORD << intclasstag << std::endl                      // class tag
    << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << std::endl  // object size
    << WORD;

  /***** Add dispatch information for class Int ******/

  s << "Int_dispTab" << std::endl;                                          // dispatch table
  s << WORD << str << std::endl;                           // integer value
}

//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag) {
  for (auto entry : tbl) {
    entry.code_def(s,intclasstag);
  }
}

//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}

//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << std::endl;

  code_ref(s);
  s << LABEL                                  // label
    << WORD << boolclasstag << std::endl                       // class tag
    << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << std::endl   // object size
    << WORD;

  /***** Add dispatch information for class Bool ******/

  s << "Bool_dispTab" << std::endl;                                            // dispatch table
  s << WORD << val << std::endl;                             // value (0 or 1)
}

void CgenClassTable::code_class_prototypes() {
  // run through each user input class and add prototypes
  // recurse through the tree, start at object and look at kids

}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//
// Define global names for some of basic classes and their tags.
//
void CgenClassTable::code_global_data()
{

  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << std::endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << std::endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << std::endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << std::endl;
  str << GLOBAL; falsebool.code_ref(str);  str << std::endl;
  str << GLOBAL; truebool.code_ref(str);   str << std::endl;
  str << GLOBAL << INTTAG << std::endl;
  str << GLOBAL << BOOLTAG << std::endl;
  str << GLOBAL << STRINGTAG << std::endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //


  int stringclasstag = *class_to_tag_table.lookup(string);
  int intclasstag = *class_to_tag_table.lookup(integer);
  int boolclasstag = *class_to_tag_table.lookup(boolc);
  

  str << INTTAG << LABEL
      << WORD << intclasstag << std::endl;
  str << BOOLTAG << LABEL
      << WORD << boolclasstag << std::endl;
  str << STRINGTAG << LABEL
      << WORD <<  stringclasstag
      << std::endl;
}

//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << std::endl
      << HEAP_START << LABEL
      << WORD << 0 << std::endl
      << "\t.text" << std::endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << std::endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << std::endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << std::endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << std::endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << std::endl;
}

void CgenClassTable::code_bools()
{
  int boolclasstag = *class_to_tag_table.lookup(idtable.add_string(BOOLNAME));
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

//
// Generate GC choice constants (pointers to GC functions)
//
void CgenClassTable::code_select_gc()
{
  str << GLOBAL << "_MemMgr_INITIALIZER" << std::endl;
  str << "_MemMgr_INITIALIZER:" << std::endl;
  str << WORD << gc_init_names[cgen_Memmgr] << std::endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << std::endl;
  str << "_MemMgr_COLLECTOR:" << std::endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << std::endl;
  str << GLOBAL << "_MemMgr_TEST" << std::endl;
  str << "_MemMgr_TEST:" << std::endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << std::endl;
}

//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//********************************************************
void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  int stringclasstag = *class_to_tag_table.lookup(idtable.lookup_string(STRINGNAME));
  int intclasstag = *class_to_tag_table.lookup(idtable.lookup_string(INTNAME));

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools();
}

CgenClassTable::CgenClassTable(Classes classes, ostream& s) : str(s) {

  // make sure the various tables have a scope
  class_to_tag_table.enterscope();
  stack_map.enterscope();
  enterscope();
  if (cgen_debug) std::cerr << "Building CgenClassTable" << std::endl;
  install_basic_classes();
  install_classes(classes);
  build_inheritance_tree();

  code();
  exitscope();
}


void CgenClassTable::install_basic_classes() {
  Symbol filename = stringtable.add_string("<basic class>");

  //
  // A few special class names are installed in the lookup table but not
  // the class list.  Thus, these classes exist, but are not part of the
  // inheritance hierarchy.
  // No_class serves as the parent of Object and the other special classes.
  // SELF_TYPE is the self class; it cannot be redefined or inherited.
  // prim_slot is a class known to the code generator.
  //
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
		     Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
		     Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
		     Basic,this));

  //
  // The Object class has no parent class. Its methods are
  //        cool_abort() : Object    aborts the program
  //        type_name() : Str        returns a string representation of class name
  //        copy() : SELF_TYPE       returns a copy of the object
  //
  // There is no need for method bodies in the basic classes---these
  // are already built in to the runtime system.
  //
  install_class(
   new CgenNode(
    class_(Object,
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(::copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

//
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
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
	   filename),
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer.
//
   install_class(
    new CgenNode(
     class_(Int,
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  the string's length
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//
   install_class(
    new CgenNode(
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
	     filename),
        Basic,this));
}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
// The following possible errors are checked:
//       - a class called SELF_TYPE
//       - redefinition of a basic class
//       - redefinition of another previously defined class
//
void CgenClassTable::install_class(CgenNodeP nd) {
  Symbol name = nd->get_name();

  int* class_tag = new int;
  *class_tag = current_class_num;
  current_class_num++;
  class_to_tag_table.addid(name, class_tag);
  stringtable.add_string(name->get_string());


  // stringtable.add_string(idtable.lookup(name));
  
  // add string name to Class_nameTab 


  if (probe(name)) {
    return;
  }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds.push_back(nd);
  addid(name, nd);
}

void CgenClassTable::install_classes(Classes cs) {
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i), NotBasic, this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree() {
  for (auto nd : nds) {
    set_relations(nd);
  }
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd) {
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n) {
  children.push_front(n);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

CgenNodeP CgenNode::get_parentnd()
{
  return parentnd;
}


void CgenClassTable::code()
{
  
    if (cgen_debug) std::cerr << "coding global data" << std::endl;
    code_global_data();


    if (cgen_debug) std::cerr << "choosing gc" << std::endl;
    code_select_gc();

    if (cgen_debug) std::cerr << "coding constants" << std::endl;
    code_constants();

    // TWO PASSES HERE, FIRST DEFINED AS SUCH

    // set everything up, DO NOT EMIT ANY CODE YET

    code_class_name_tab();

    code_objects_tab();

    code_max_tag();

    code_parent_tab();

    code_class_attr_tab();

    code_attr_tabs();

    code_disp_tabs();

    code_prototypes();

    //                 Add your code to emit
    //                   - prototype objects
    //                   - class_nameTab
    //                   - dispatch tables
    //                   - 
    //


    if (cgen_debug) std::cerr << "coding global text" << std::endl;
    code_global_text();

    // now we need to create all of the object inits 
    // need to deal with the fuckass stacka and shit now
    code_inits();

    // set up tree traversal 

    traverse_cgen();

    //                 Add your code to emit
    //                   - object initializer
    //                   - the class methods
    //                   - etc...

}

void CgenClassTable::code_inits() {
  for (auto nd : nds) {
    const char* class_string = nd->get_name()->get_string();
    str << class_string << CLASSINIT_SUFFIX << LABEL;
    emit_addiu("$sp", "$sp", -12, str);
    emit_store("$fp", 3, "$sp", str);
    emit_store("$s0", 2, "$sp", str);
    emit_store("$ra", 1, "$sp", str);
    emit_addiu("$fp", "$sp", 16, str);
    emit_move("$s0", "$a0", str);
    CgenNodeP parent = nd->get_parentnd();
    Symbol parent_name = parent->get_name();
    if (parent_name != No_class) {
      const char* parent_string = parent_name->get_string();
      str << JAL << parent_string << CLASSINIT_SUFFIX << std::endl;
    }

    std::vector<Feature> attributes = attr_map[nd->get_name()];
    for (int i = 0 ; i < (int) attributes.size() ; i++) {
      // check if current attribute is initialized
      Feature feature = attributes[i];
      // need to code the expression
      feature->get_expr()->code(str, this, nd);

      // we need to do stuff with each return value, should be returned in accumulator a0
    }

    emit_move("$a0", "$s0", str);
    emit_load("$fp", 3, "$sp", str);
    emit_load("$s0", 2, "$sp", str);
    emit_load("$ra", 1, "$sp", str);
    emit_addiu("$sp", "$sp", 12, str);
    emit_return(str);
  }
}

void CgenClassTable::code_max_tag() {
  str << "_max_tag" << LABEL 
  << WORD << current_class_num - 1 << std::endl;
}

void CgenClassTable::code_class_attr_tab() {
  str << "class_attrTabTab" << LABEL;      
  for (auto nd : nds) {
    const char* node_string = nd->get_name()->get_string();
    str << WORD << node_string << "_attrTab" << std::endl;
  }
}

void CgenClassTable::code_prototypes() {
  for (auto nd : nds) {
    Symbol node_name = nd->get_name();
    const char* node_string = node_name->get_string();
    str << WORD << -1 << std::endl;
    str << node_string << PROTOBJ_SUFFIX << LABEL;
    str << WORD << *class_to_tag_table.lookup(node_name) << std::endl;
    str << WORD << nd->attributes.size() + 3 << std::endl;
    str << WORD << node_string << DISPTAB_SUFFIX << std::endl;
    for (auto attr : nd->attributes) {
      Symbol type = attr->get_type();
      str << WORD;
      if (type == Int) {
        inttable.lookup_string("0")->code_ref(str);
        // str << "int_const0";
      } else if (type == Str) {
        // NEED TO FIX -- LOOK UP NULL STRING LABEL NAME INSTEAD OF APPLYING SET OFFSET
        stringtable.lookup_string("")->code_ref(str);
        // str << "str_const" << nds.size() + 2;
      } else if (type == Bool) {
        BoolConst* temp = new BoolConst(0);
        temp->code_ref(str);
        // str << "bool_const0";
      } else {
        // str << 0;
      }
      str << std::endl;
    }
  }
}

void CgenClassTable::code_attr_tabs() {
  for (auto nd : nds) {
    CgenNodeP curr_node = nd;
    const char* node_string = curr_node->get_name()->get_string();
    str << node_string << "_attrTab" << LABEL;
    std::stack<CgenNodeP> inheritance_nodes;
    // create stack, we need to reverse later
    while (curr_node->get_name() != No_class) {
      inheritance_nodes.push(curr_node);
      curr_node = curr_node->get_parentnd();
    }
    
    while (!inheritance_nodes.empty()) {
      curr_node = inheritance_nodes.top();
      inheritance_nodes.pop();
      Features features = curr_node->get_features();
      for (int i = features->first() ; features->more(i) ; i = features->next(i)) {
        Feature feature = features->nth(i);
        if (!feature->is_method()) {
          // we know its an attr
          int* attr_type_index_ptr = class_to_tag_table.lookup(feature->get_type());
          int index;
          // if attribute is a literal (in the case of bool, int, and str), we don't really have a type for it, so return -2
          attr_type_index_ptr ? (index = *attr_type_index_ptr) : (index = -2);
          str << WORD << index << std::endl;
          nd->attributes.push_back(feature);
          attr_map[nd->get_name()].push_back(feature);
        }
      }
    }
  }
}

void CgenClassTable::code_disp_tabs() {
  for (auto nd : nds) {
    CgenNodeP curr_node = nd;
    const char* node_string = curr_node->get_name()->get_string();
    str << node_string << "_dispTab" << LABEL;
    std::vector<CgenNodeP> inheritance_nodes;
    std::vector<std::set<Symbol>> class_methods;
    // create stack, we need to reverse later, also create stack of sets of element names to check for definition
    while (curr_node->get_name() != No_class) {
      Features features = curr_node->get_features();
      std::set<Symbol> curr_classes;
      for (int i = features->first() ; features->more(i) ; i = features->next(i)) {
        Feature feature = features->nth(i);
        if (feature->is_method()) { curr_classes.insert(feature->get_name()); }
      }

      inheritance_nodes.insert(inheritance_nodes.begin(), curr_node);
      class_methods.insert(class_methods.begin(), curr_classes);
      curr_node = curr_node->get_parentnd();
    }

    for (int j = 0 ; j < (int) inheritance_nodes.size() ; j++) {
      curr_node = inheritance_nodes[j];
      std::set<Symbol> curr_method_set = class_methods[j];
      Features features = curr_node->get_features();
      for (int i = features->first() ; features->more(i) ; i = features->next(i)) {
        Feature feature = features->nth(i);
        if (feature->is_method()) {
          // we know its a method
          Symbol method_name = feature->get_name();
          // if parent class has same method name (after type checking we know that it must be a valid redefinition),
          // then don't inherit method from parent
          if (j != 0 and class_methods[j - 1].count(method_name)) { continue; }
          int k = j;
          while (k < (int) class_methods.size() and class_methods[k].count(method_name)) { k++; }
          k--;
          // k is index of smallest child with this method
          // otherwise add to method table
          const char* method_string = method_name->get_string();
          const char* class_string = inheritance_nodes[k]->get_name()->get_string();
          str << WORD << class_string << "." << method_string << std::endl;
          method_map[nd->get_name()].push_back(feature);
        }
      }
    }
  }
}

void CgenClassTable::code_parent_tab() {
  str << "class_parentTab" << LABEL;      
  for (auto nd : nds) {
    CgenNodeP parent = nd->get_parentnd();
    int* parent_index_ptr = class_to_tag_table.lookup(parent->get_name());
    int index;
    // if there is no parent (in the case of Object), return -2
    parent_index_ptr ? (index = *parent_index_ptr) : (index = -2);
    str << WORD << index << std::endl;
  }
}

void CgenClassTable::code_class_name_tab() {
  // first, emit the name of the classname table:
  str << "Class_nameTab" << LABEL;                       
  for (auto nd : nds) {
    // has to be i + 2 because we initialize 2 strings before 
    // ie: the class that is at index 0 was actually the second string created, 1st index was 3rd, and so on
    // NEED TO FIX -- COULD PROBABLY DO MORE ELEGANTLY WITH LOOKUP

    const char* class_name = nd->get_name()->get_string();
    // str << class_name << std::endl;
    str << WORD;
    stringtable.lookup_string(class_name)->code_ref(str);
    str << std::endl;
    // str << WORD << "str_const" << *class_to_tag_table.lookup(nd->get_name()) + 2 << std::endl;
  }
}

void CgenClassTable::code_objects_tab() {
  str << "class_objTab" << LABEL;
  for (auto nd : nds) {
    const char* node_string = nd->get_name()->get_string();
    str << WORD << node_string << "_protObj" << std::endl;
    str << WORD << node_string << "_init" << std::endl;
  }
}

// recursive helper
void CgenClassTable::traverse_cgen() {
  // start at object, so look up OBJECT in synbol table and begin at its cgennode
  CgenNodeP object_node = lookup(Object);
  traverse_cgen(object_node);
}

// recursive implementation
void CgenClassTable::traverse_cgen(CgenNodeP class_node) {
  std::list<CgenNodeP> children = class_node->get_children();

  // do stuff here, look at all features of class and start going down the tree
  class_node->code(str, this, class_node);

  // recurse on children
  for (auto child : children) {
    traverse_cgen(child);
  }
}

CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd,Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   basic_status(bstatus)
{
  // stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************



void class__class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // for each feature, generate code
  for (int i = features->first() ; features->more(i) ; i = features->next(i)) {
    Feature feature = features->nth(i);
    feature->code(s, infra, current);
  }
}

void attr_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // codegen the init 
  init->code(s, infra, current);
}

void method_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // do stuff with formals to create method table (use offsets and shit idk i havent gotten here yet)
  // ACTUALLY, in regards to the statement above—this table should already be generated by now, so idt we need to do this here. ill leave the code tho
  // for (int i = formals->first() ; formals->more(i) ; i = formals->next(i)) {
  //   Formal formal = formals->nth(i);
  // }

  expr->code(s, infra, current);
}


int CgenClassTable::get_unique_label() {
  return label_index++;
}

void branch_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  expr->code(s, infra, current);
}

void assign_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // FIRST CHECK THE STACK—if it is there, use that
  


  // otherwise check attributes
  expr->code(s, infra, current);
  std::vector<Feature> current_attributes = infra->attr_map[current->get_name()];
  int index = 0;
  while (current_attributes[index]->get_name() != name) { index++; }
  // index is now at the current attribute
  int offset = 4 * index;
  emit_store(ACC, offset + DEFAULT_OBJFIELDS, SELF, s);
}

void static_dispatch_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // TODO: finish this
}

void dispatch_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // push the arguments in reverse order
  // so we can pop them in the correct (left to right) order
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    actual->nth(i)->code(s, infra, current);
    emit_push(ACC, s);
  }
  expr->code(s, infra, current);

  // check for null
  int label_ok = infra->get_unique_label();
  emit_bne(ACC, ZERO, label_ok, s);
  emit_load_address(ACC, "dispatch_abort", s);
  emit_jal("dispatch_abort", s);
  emit_label_def(label_ok, s);

  emit_load(T1, DISPTABLE_OFFSET, ACC, s); // load address of dispatch table

  // We have no idea what expr's type is—it could literally be anything. We need to check at runtime

  // we can figure out the dynamic type with 
  // jal Object.type_name
  // returns class tag into accumulator
  // we can use class tag to get string name of class
  // with name of class we can 

  // we have class name dynamically and method name statically —— what do we do?
  // can we create a label 
  
  Symbol static_type = expr->get_type();

  std::vector<Feature> expr_methods = infra->method_map[static_type];
  int offset = 0;
  while(expr_methods[offset]->get_name() != name) { offset++; }

  // index points to current method in current class
  emit_load(T1, offset, T1, s); // add memory address

  emit_jalr(T1, s); // Call the function
}

void cond_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  int label_false = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  pred->code(s, infra, current);
  emit_fetch_int(T1, ACC, s);
  emit_beqz(T1, label_false, s); // if pred is false, go to label_false
  then_exp->code(s, infra, current); 
  emit_branch(label_end, s); // skip else branch

  emit_label_def(label_false, s); // else branch
  else_exp->code(s, infra, current);
  emit_label_def(label_end, s);
}

void loop_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  int label_loop = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_label_def(label_loop, s); // loop:
  pred->code(s, infra, current); 
  emit_fetch_int(T1, ACC, s); 
  emit_beqz(T1, label_end, s);// if pred is false (0), go to loop_end

  body->code(s, infra, current); 
  emit_branch(label_loop, s);// go back to loop

  emit_label_def(label_end, s); /// loop_end
  emit_move(ACC, ZERO, s); //result of while loop is void
}

void typcase_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  expr->code(s, infra, current);

  // check for null
  int label_abort = infra->get_unique_label();
  emit_beqz(ACC, label_abort, s);

  emit_load(T1, 0, ACC, s);

  // TODO: finish this
  // std::vector<int> branch_labels;
}

void block_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  for (int i = body->first() ; body->more(i) ; i = body->next(i)) {
    (body->nth(i))->code(s, infra, current);
  }
}

// TODO: check if this makes sense?
void let_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  if (init==no_expr()) {
    if (type_decl == Int)
      emit_load_int(ACC, inttable.lookup_string("0"), s);
    else if (type_decl == Bool)
      emit_load_bool(ACC, falsebool, s);
    else if (type_decl == Str)
      emit_load_string(ACC, stringtable.lookup_string(""), s);
    else
      emit_move(ACC, ZERO, s);
  } else {
    init->code(s, infra, current);
  }
  emit_push(ACC, s); // add to the environment 
  body->code(s, infra, current);
  emit_addiu(SP,SP,4,s);
}

void plus_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);
  emit_add(ACC,T1,ACC,s);
}

void sub_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);
  emit_sub(ACC,T1,ACC,s);
}

void mul_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);
  emit_mul(ACC,T1,ACC,s);
}

void divide_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);
  emit_div(ACC,T1,ACC,s);
}

void neg_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_fetch_int(T1,ACC,s);
  emit_neg(T1,T1,s);
  emit_store_int(T1,ACC,s);
}

void lt_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);

  emit_fetch_int(T1, T1, s);
  emit_fetch_int(T2, ACC, s);

  int label_true = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_blt(T1, T2, label_true, s); // If T1<T2, go to label_true
  emit_load_bool(ACC, falsebool, s); // load False
  emit_branch(label_end, s);

  emit_label_def(label_true, s); // label_true:
  emit_load_bool(ACC, truebool, s); // load True

  emit_label_def(label_end, s); // label_end:
}

void eq_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);

  emit_fetch_int(T1, T1, s);
  emit_fetch_int(T2, ACC, s);

  int label_true = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_beq(T1, T2, label_true, s); // If T1==T2, go to label_true
  emit_load_bool(ACC, falsebool, s); // load False
  emit_branch(label_end, s);

  emit_label_def(label_true, s); // label_true:
  emit_load_bool(ACC, truebool, s); // load True

  emit_label_def(label_end, s); // label_end:
}

void leq_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_push(ACC,s);
  e2->code(s, infra, current);
  emit_pop(T1,s);

  emit_fetch_int(T1, T1, s);
  emit_fetch_int(T2, ACC, s);

  int label_true = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_bleq(T1, T2, label_true, s); // If T1<=T2, go to label_true
  emit_load_bool(ACC, falsebool, s); // load False
  emit_branch(label_end, s);

  emit_label_def(label_true, s); // label_true:
  emit_load_bool(ACC, truebool, s); // load True

  emit_label_def(label_end, s); // label_end:
}

void comp_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);
  emit_fetch_int(T1, ACC, s);

  int label_true = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_beqz(T1, label_true, s); // If False, go to True
  emit_load_bool(ACC, falsebool, s); // Else, go to False
  emit_branch(label_end, s);

  emit_label_def(label_true, s); // label_true:
  emit_load_bool(ACC, truebool, s); 
  emit_label_def(label_end, s);
}

void int_const_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current)
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  // TODO: finish this
}

void isvoid_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  e1->code(s, infra, current);

  int label_true = infra->get_unique_label();
  int label_end = infra->get_unique_label();

  emit_beqz(ACC, label_true, s);
  emit_load_bool(ACC, falsebool, s); // void is False
  emit_branch(label_end, s);

  emit_label_def(label_true, s); // label_true:
  emit_load_bool(ACC, truebool, s);

  emit_label_def(label_end, s); // label_end: 
}

void no_expr_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  emit_move(ACC, ZERO, s); // set to 0 to avoid void
}

void object_class::code(ostream &s, CgenClassTableP infra, CgenNodeP current) {
  if (name == self) {
    emit_move(ACC, SELF, s); // move SELF to ACC
  } 
  else {
    int offset;
    // TODO: add offset
  }
}
