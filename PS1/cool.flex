/*
 * The scanner definition for COOL.
 */
%option noyywrap
/*
 * Stuff enclosed in %{ %} in the first section is copied verbatim to the
 * output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include "cool-parse.h"

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */
int string_length = 0;
static int comment_nests = 0;
%}

/* Keep track of string state */
%Start STRING 
%Start COMMENT
%Start ERROR_STR

/*
 * Define names for regular expressions here.
 */

DARROW =>

IF            [iI][fF]
FI            [fF][iI]
CLASS         [cC][lL][aA][sS][sS]
ELSE          [eE][lL][sS][eE]
IN            [iI][nN]
INHERITS      [iI][nN][hH][eE][rR][iI][tT][sS]
LET           [lL][eE][tT]
LOOP          [lL][oO][oO][pP]
POOL          [pP][oO][oO][lL]
THEN          [tT][hH][eE][nN]
WHILE         [wW][hH][iI][lL][eE]
CASE          [cC][aA][sS][eE]
ESAC          [eE][sS][aA][cC]
OF            [oO][fF]
NEW           [nN][eE][wW]
NOT           [nN][oO][tT]
ISVOID        [iI][sS][vV][oO][iI][dD]
WHITESPACE    [ \f\r\t\v]+
BOOL_TRUE     t[rR][uU][eE]
BOOL_FALSE    f[aA][lL][sS][eE]
OBJECTID      [a-z][a-zA-Z0-9_]* 
TYPEID        [A-Z][a-zA-Z0-9_]* 
INT_CONST     [0-9]+
DIGIT         [0-9]
LETTER        [a-zA-Z]
NEWLINE       \n
LE <=
ASSIGN <-

%%

 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */
<INITIAL>{WHITESPACE}  { };
<INITIAL>{DARROW}        { return (DARROW); }
<INITIAL>{IF}        { return (IF); }
<INITIAL>{FI}        { return (FI); }
<INITIAL>{CLASS}     { return (CLASS); }
<INITIAL>{ELSE}      { return (ELSE); }
<INITIAL>{IN}        { return (IN); }
<INITIAL>{INHERITS}  { return (INHERITS); }
<INITIAL>{LET}       { return (LET); }
<INITIAL>{LOOP}      { return (LOOP); }
<INITIAL>{POOL}      { return (POOL); }
<INITIAL>{THEN}      { return (THEN); }
<INITIAL>{WHILE}     { return (WHILE); }
<INITIAL>{CASE}      { return (CASE); }
<INITIAL>{ESAC}      { return (ESAC); }
<INITIAL>{OF}        { return (OF); }
<INITIAL>{NEW}       { return (NEW); }
<INITIAL>{NOT}       { return (NOT); }
<INITIAL>{ISVOID}    { return (ISVOID); }
<INITIAL>{LE}       { return LE; }
<INITIAL>{ASSIGN}        { return (ASSIGN); }
<INITIAL>"="         { return '='; }
<INITIAL>"-"         { return '-'; }
<INITIAL>"+"         { return '+'; }
<INITIAL>"<"         { return '<'; }
<INITIAL>":"         { return ':'; }
<INITIAL>";"         { return ';'; }
<INITIAL>"("         { return '('; }
<INITIAL>")"         { return ')'; }
<INITIAL>"{"         { return '{'; }
<INITIAL>"}"         { return '}'; }
<INITIAL>"."         { return '.'; }
<INITIAL>","         { return ','; }
<INITIAL>"*"        { return '*'; }
<INITIAL>"@"        { return '@'; }
<INITIAL>"/"        { return '/'; }
<INITIAL>"~"        { return '~'; }


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

  /* Comments in dashes -- */
<INITIAL>"--"[^\n]* {}

  /* Comments in (* *) */
<INITIAL,COMMENT>"(*" {
  comment_nests += 1; 
  BEGIN(COMMENT);
}
<COMMENT>"*)" {
  comment_nests -= 1;
  if (comment_nests == 0) {
    BEGIN(INITIAL);
  }
}
<COMMENT>\n {
  curr_lineno += 1;
}
<COMMENT>[^\n] { }
<COMMENT><<EOF>> {
  yylval.error_msg = "EOF in comment";
  comment_nests = 0;
  BEGIN(INITIAL);
  return ERROR;
}
<INITIAL>"*)" {
  yylval.error_msg = "Unmatched *)";
  comment_nests = 0;
  BEGIN(INITIAL);
  return ERROR;
}

 /*
  *  String constants (C syntax, taken from lexdoc(1) )
  *  Escape sequence \c is accepted for all characters c. Except for
  *  \n \t \b \f, the result is c.
  *  (but note that 'c' can't be the NUL character)
  *
  */

  /* Consumer the rest of the erroneous string */

<ERROR_STR><<EOF>> {
  BEGIN(INITIAL);
  return ERROR;
}

<ERROR_STR>\" {
    BEGIN(INITIAL);
    return ERROR;
}

<ERROR_STR>\n {
  curr_lineno++;
  BEGIN(INITIAL);
  return ERROR;
}

<ERROR_STR>[^\\"\n]+ { }

  /* Must add line number here */
<ERROR_STR>\\\n {
  curr_lineno++;
}

<ERROR_STR>\\. { } 

  /* Special cases \b \t \n \f */
<STRING>\\n {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\n';
}
<STRING>\\t {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\t';
}
<STRING>\\b {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\b';
}
<STRING>\\f {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\f';
}
<STRING>\\\" {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\"';
}
<STRING>\\\\ {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\\';
}

  /* Invalid null character \0 */
<STRING>\0 {
  yylval.error_msg = "String contains null character";
  BEGIN(INITIAL);
  return ERROR;
}

  /* Invalid EOF error */
<STRING><<EOF>> {
  yylval.error_msg = "EOF in string constant";
  BEGIN(INITIAL);
  return ERROR;
}

  /* Escaped newline */
<STRING>\\\n {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = '\n';
  curr_lineno += 1;
}

  /* Unescaped newline error */
<STRING>\n {
  curr_lineno++;
  yylval.error_msg = "Unterminated string constant";
  BEGIN(INITIAL);
  return ERROR;
}

  /* Add string to string table */  
<STRING>\" {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length] = '\0'; 
  yylval.symbol = stringtable.add_string(string_buf);
  BEGIN(INITIAL);
  return STR_CONST;
}

  /* Single character \c */
<STRING>\\[^nbtf] {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = yytext[1];
}

  /* All other single characters */
<STRING>. {
  if (string_length >= MAX_STR_CONST - 1) {
    yylval.error_msg = "String constant too long";
    BEGIN(ERROR_STR);
  }
  string_buf[string_length++] = yytext[0];
}

  /* Begin string state */
<INITIAL>\" {
  string_buf_ptr = string_buf;
  string_length = 0;
  BEGIN(STRING);
}

 /* Other rules */
<INITIAL>{BOOL_TRUE} {
  cool_yylval.boolean = 1;
  return BOOL_CONST;
}
<INITIAL>{BOOL_FALSE} {
  cool_yylval.boolean = 0;
  return BOOL_CONST;
}
<INITIAL>{OBJECTID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return OBJECTID;
}
<INITIAL>{TYPEID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return TYPEID;
}
<INITIAL>{INT_CONST} {
  cool_yylval.symbol = inttable.add_string(yytext);
  return INT_CONST;
}
<INITIAL>\n {
  curr_lineno += 1;
}

  /* Error handling for invalid character (one that can’t begin any token) */
<INITIAL>[^\n] {
  cool_yylval.error_msg = yytext;
	return ERROR;
}

%% 