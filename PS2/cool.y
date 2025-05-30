/*
 *  cool.y
 *              Parser definition for the COOL language.
 *
 */
%{
#include "cool-tree.h"
#include "stringtab.h"
#include "utilities.h"

/* Set the size of the parser stack to be sufficient large to accomodate
   our tests.  There seems to be some problem with Bison's dynamic
   reallocation of the parser stack, as it should resize to a max
   size of YYMAXDEPTH, which is by default 10000. */
#define YYINITDEPTH 3000

/* Locations */
#define YYLTYPE int              /* the type of locations */
#define cool_yylloc curr_lineno  /* use the curr_lineno from the lexer
                                    for the location of tokens */
extern int node_lineno;          /* set before constructing a tree node
                                    to whatever you want the line number
                                    for the tree node to be */

/* The default action for locations.  Use the location of the first
   terminal/non-terminal and set the node_lineno to that value. */
#define YYLLOC_DEFAULT(Current, Rhs, N)		  \
  Current = (Rhs)[1];                             \
  node_lineno = Current;

#define SET_NODELOC(Current)			\
  node_lineno = Current;

/* IMPORTANT NOTE ON LINE NUMBERS
*********************************
* The above definitions and macros cause every terminal in your grammar to
* have the line number supplied by the lexer. The only task you have to
* implement for line numbers to work correctly, is to use SET_NODELOC()
* before constructing any constructs from non-terminals in your grammar.
* Example: Consider you are matching on the following very restrictive
* (fictional) construct that matches a plus between two integer constants.
* (SUCH A RULE SHOULD NOT BE  PART OF YOUR PARSER):

plus_consts : INT_CONST '+' INT_CONST

* where INT_CONST is a terminal for an integer constant. Now, a correct
* action for this rule that attaches the correct line number to plus_const
* would look like the following:

plus_consts : INT_CONST '+' INT_CONST
{
// Set the line number of the current non-terminal:
// ***********************************************
// You can access the line numbers of the i'th item with @i, just
// like you acess the value of the i'th exporession with $i.
//
// Here, we choose the line number of the last INT_CONST (@3) as the
// line number of the resulting expression (@$). You are free to pick
// any reasonable line as the line number of non-terminals. If you
// omit the statement @$=..., bison has default rules for deciding which
// line number to use. Check the manual for details if you are interested.
@$ = @3;


// Observe that we call SET_NODELOC(@3); this will set the global variable
// node_lineno to @3. Since the constructor call "plus" uses the value of
// this global, the plus node will now have the correct line number.
SET_NODELOC(@3);

// construct the result node:
$$ = plus(int_const($1), int_const($3));
}

*/

extern char *curr_filename;

void yyerror(const char *s);  /*  defined below; called for each parse error */
extern int yylex();           /*  the entry point to the lexer  */
Program ast_root;	      /* the result of the parse  */
Classes parse_results;        /* for use in semantic analysis */
int omerrs = 0;               /* number of erros in lexing and parsing */
%}

/* A union of all the types that can be the result of parsing actions. */
%union {
  bool boolean;
  Symbol symbol;
  Program program;
  Class_ class_;
  Classes classes;
  Feature feature;
  Features features;
  Formal formal;
  Formals formals;
  Case case_;
  Cases cases;
  Expression expression;
  Expressions expressions;
  const char *error_msg;
}

/*
   Declare the terminals; a few have types for associated lexemes.
   The token ERROR is never used in the parser; thus, it is a parse
   error when the lexer returns it.

   The integer following token declaration is the numeric constant used
   to represent that token internally.  Typically, Bison generates these
   on its own, but we give explicit numbers to prevent version parity
   problems (bison 1.25 and earlier start at 258, later versions -- at
   257)

   NOTE: these numbers need to match what's in cool-parse.h !
 */
%token CLASS 258 ELSE 259 FI 260 IF 261 IN 262
%token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
%token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
%token <symbol>  STR_CONST 275 INT_CONST 276
%token <boolean> BOOL_CONST 277
%token <symbol>  TYPEID 278 OBJECTID 279
%token ASSIGN 280 NOT 281 LE 282 ERROR 283

/*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
/**************************************************************************/

/* Complete the nonterminal list below, giving a type for the semantic
   value of each non terminal. (See section 3.6 in the bison 
   documentation for details). */

/* Declare types for the grammar's non-terminals. */
%type <program> program
%type <classes> class_list
%type <class_> class
%type <feature> feature
%type <features> feature_list
%type <features> optional_feature_list
%type <formal> formal
%type <formals> formal_list
%type <formals> optional_formal_list
%type <expression> expression
%type <expressions> optional_expression_list_comma
%type <expressions> expression_list_block /* Not optional */
%type <expressions> expression_list_comma

/* %type <expression> empty_expression */

%type <cases> case_list
%type <case_> branch
%type <expression> lets

/* Precedence declarations go here. */
%nonassoc IN
%right ASSIGN
%left NOT
%nonassoc LE '<' '='
%left '+' '-'
%left '*' '/'
%left ISVOID
%left '~'
%nonassoc '@'
%nonassoc '.'

%%
// Save the root of the abstract syntax tree in a global variable.
program	: 
  class_list	
{ @$ = @1; ast_root = program($1); }
;

class_list: 
  class			/* single class */
{ $$ = single_Classes($1);
  parse_results = $$; }
| class_list class	/* several classes */
{ $$ = append_Classes($1,single_Classes($2));
  parse_results = $$; }
| error ';' { yyerrok; $$ = nil_Classes(); }
| class_list error ';' {yyerrok; $$ = $1;}
;

/* If no parent is specified, the class inherits from the Object class. */
class	: 
  CLASS TYPEID '{' optional_feature_list '}' ';'
{ $$ = class_($2,idtable.add_string("Object"),$4,
	      stringtable.add_string(curr_filename)); }
| CLASS TYPEID INHERITS TYPEID '{' optional_feature_list '}' ';'
{ $$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
;


/* FEATURES */
/* SINGLE FEATURE */
feature: 
  OBJECTID '(' optional_formal_list ')' ':' TYPEID '{' expression '}' ';'
{ $$ = method($1,$3,$6,$8); SET_NODELOC(@8); }
| OBJECTID ':' TYPEID ';'
{ $$ = attr($1,$3,no_expr()); SET_NODELOC(@3); }
| OBJECTID ':' TYPEID ASSIGN expression ';'
{ $$ = attr($1,$3,$5); SET_NODELOC(@5); }
;

/* 1, or more */
feature_list:
  feature 
{ $$ = single_Features($1); SET_NODELOC(@1); }
| feature_list feature 
{ $$ = append_Features($1,single_Features($2)); SET_NODELOC(@2); }
| error ';' { yyerrok; $$ = nil_Features(); }
| feature_list error ';' { yyerrok; $$ = $1; }
;

/* Make it optional */
optional_feature_list:
{ $$ = nil_Features(); }
| feature_list
{ $$ = $1; SET_NODELOC(@1); }
;

/* FORMALS */
/* single formal */
formal: 
  OBJECTID ':' TYPEID 
{ $$ = formal($1,$3); SET_NODELOC(@3); }
;

/* 1 or more */
formal_list: 
  formal 
{ $$ = single_Formals($1); SET_NODELOC(@1); }
| formal_list ',' formal 
{ $$ = append_Formals($1,single_Formals($3)); SET_NODELOC(@3); }
;

/* Make it optional */
optional_formal_list: 
{ $$ = nil_Formals(); }
| formal_list
{ $$ = $1; SET_NODELOC(@1); }
;


/* EXPRESSIONS */
/* empty_expression: 
{ $$ = no_expr(); } */

/* single expression */
expression: OBJECTID ASSIGN expression 
{ $$ = assign($1,$3); SET_NODELOC(@3); }
| expression '@' TYPEID '.' OBJECTID '(' optional_expression_list_comma ')'
{ $$ = static_dispatch($1,$3,$5,$7); SET_NODELOC(@7); }
| expression '.' OBJECTID '(' optional_expression_list_comma ')'
{ $$ = dispatch($1,$3,$5); SET_NODELOC(@5); }
| OBJECTID '(' optional_expression_list_comma ')'
{ $$ = dispatch(object(idtable.add_string("self")),$1,$3); SET_NODELOC(@3); }
| IF expression THEN expression ELSE expression FI
{ $$ = cond($2,$4,$6); SET_NODELOC(@6); }
| WHILE expression LOOP expression POOL
{ $$ = loop($2,$4); SET_NODELOC(@4); }
| '{' expression_list_block '}'
{ $$ = block($2); SET_NODELOC(@2); }
| LET lets
{ $$ = $2; SET_NODELOC(@2); }
| CASE expression OF case_list ESAC
{ $$ = typcase($2,$4); SET_NODELOC(@4); } 
| NEW TYPEID
{ $$ = new_($2); SET_NODELOC(@2); }
| ISVOID expression
{ $$ = isvoid($2); SET_NODELOC(@2); }
| expression '+' expression
{ $$ = plus($1,$3); SET_NODELOC(@3); }
| expression '-' expression
{ $$ = sub($1,$3); SET_NODELOC(@3); }
| expression '*' expression
{ $$ = mul($1,$3); SET_NODELOC(@3); }
| expression '/' expression
{ $$ = divide($1,$3); SET_NODELOC(@3); }
| '~' expression
{ $$ = neg($2); SET_NODELOC(@2); }
| expression '<' expression
{ $$ = lt($1,$3); SET_NODELOC(@3); }
| expression LE expression
{ $$ = leq($1,$3); SET_NODELOC(@3); }
| expression '=' expression
{ $$ = eq($1,$3); SET_NODELOC(@3); }
| NOT expression
{ $$ = comp($2); SET_NODELOC(@2); }
| '(' expression ')'
{ $$ = $2; SET_NODELOC(@2); }
| OBJECTID
{ $$ = object($1); SET_NODELOC(@1); }
| INT_CONST
{ $$ = int_const($1); SET_NODELOC(@1); }
| STR_CONST
{ $$ = string_const($1); SET_NODELOC(@1); }
| BOOL_CONST
{ $$ = bool_const($1); SET_NODELOC(@1); }
;


/* optional expression list with comma */
expression_list_comma:
  expression
{ $$ = single_Expressions($1); SET_NODELOC(@1); }
| expression_list_comma ',' expression
{ $$ = append_Expressions($1, single_Expressions($3)); SET_NODELOC(@3); }
;

optional_expression_list_comma:
{ $$ = nil_Expressions(); }
| expression_list_comma 
{ $$ = $1; SET_NODELOC(@1); }
;

/* 1 or more expression list with semicolon */
expression_list_block:
 expression ';'
{ $$ = single_Expressions($1); SET_NODELOC(@1); }
| expression_list_block expression ';'
{ $$ = append_Expressions($1, single_Expressions($2)); SET_NODELOC(@2); }
| error ';' { yyerrok; $$ = nil_Expressions(); }
| expression_list_block error ';' { yyerrok; $$ = $1; }
;


/* 1 or more case list */
case_list: branch
{ $$ = single_Cases($1); SET_NODELOC(@1); }
| case_list branch
{ $$ = append_Cases($1,single_Cases($2)); SET_NODELOC(@2); }
;

/* branch for case */
branch: OBJECTID ':' TYPEID DARROW expression ';'
{ $$ = branch($1,$3,$5); SET_NODELOC(@5); }
;

/* Multiple lets */
lets: OBJECTID ':' TYPEID ASSIGN expression ',' lets
{ $$ = let($1,$3,$5,$7); SET_NODELOC(@7); }
| OBJECTID ':' TYPEID ',' lets
{ $$ = let($1,$3,no_expr(),$5); SET_NODELOC(@5); }
| OBJECTID ':' TYPEID ASSIGN expression IN expression
{ $$ = let($1,$3,$5,$7); SET_NODELOC(@7); }
| OBJECTID ':' TYPEID IN expression
{ $$ = let($1,$3,no_expr(),$5); SET_NODELOC(@5); }
| error IN expression { yyerrok; $$ = no_expr(); }
| error ',' lets { yyerrok; $$ = $3; }
;



/* end of grammar */
%%

/* This function is called automatically when Bison detects a parse error. */
void yyerror(const char *s)
{
  extern int curr_lineno;

  std::cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
    << s << " at or near ";
  print_cool_token(yychar);
  std::cerr << std::endl;
  omerrs++;
  if(omerrs>50) { std::cerr << "More than 50 errors" << std::endl; exit(1);}
}
