/*******************************************************************************
 *
 * $Id: snet.y 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.01.2007
 * -----
 *
 * Description:
 * ------------
 *
 * Parser of S-NET compiler.
 *
 *******************************************************************************/

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>

#include "globals.h"
#include "types.h"
#include "tree_basic.h"
#include "ctinfo.h"
#include "free.h"
#include "lookup_table.h"
#include "dbug.h"
#include "str.h"
#include "sploc.h"

/* These macros are used for tag name resolution for debug prints */
#define STAG_NAME(n) STAGS_NAME(STAGREF_STAG(n))
#define BTAG_NAME(n) BTAGS_NAME(BTAGREF_BTAG(n))

/* copy location of a token to the location of the nonterminal (node) */
#define COPYLOCTO(node, token) { \
    NODE_FILE(node) = global.filename; \
    NODE_LINE(node) = token.line; \
    NODE_COL(node)  = token.col; \
  }

/* copy location of a node */
#define NODE_COPYLOCTO(dest, src) { \
    NODE_FILE(dest) = NODE_FILE(src); \
    NODE_LINE(dest) = NODE_LINE(src); \
    NODE_COL(dest)  = NODE_COL(src); \
  }

#define ASSIGNLOC(node) { \
    NODE_FILE(node) = global.filename; \
    NODE_LINE(node) = linenum; \
    NODE_COL(node) = charpos - yyleng; \
  }

extern int yylex(void);

int yyerror(char *error);
int linenum = 1;
int charpos = 1;
extern int yyleng;

void YYparseError(const char *f, ...)
{
  char err[100];
  va_list argList;

  va_start(argList, f);  
  vsnprintf(err, 100, f, argList);

  yyerror(err);

  va_end(argList);
}

/* Stack of nested networks lookup-tables for mapping net/box names to definitions */
typedef struct netLUTnode{
  lut_t  *netLut; /* Net names to definitions mapping */
  lut_t  *boxLut; /* Box names to definitions mapping */
  struct netLUTnode *next;
}NetLUTnode;

static node *parseResult       = NULL; /* Result AST-tree */
static lut_t *fieldLUT         = NULL; /* Field names to node mapping */
static lut_t *stagLUT          = NULL; /* Simple tag names to node mapping */
static lut_t *btagLUT          = NULL; /* Binding tag names to node mapping */
static NetLUTnode *netLUTstack = NULL; /* Net names to definitions mapping */
static lut_t *boxLUT           = NULL; /* Box names to definitions mapping */
static lut_t *eNetLUT          = NULL; /* External net names to defs mapping */
static node *lastDef           = NULL; /* Last definition */
static node *lastField         = NULL; /* Last field */
static node *lastStag          = NULL; /* Last simple tag */
static node *lastBtag          = NULL; /* Last binding tag */

/* Stack of nested networks lookup-tables for mapping typeSig names to definitions */
typedef struct typeSigLUTnode{
  lut_t  *typeSigLut; /* TypeSig names to definitions mapping */
  struct typeSigLUTnode *next;
}TypeSigLUTnode;

static TypeSigLUTnode *typeSigLUTstack = NULL; /* TypeSig names to definitions mapping */

/* Stack of nested networks lookup-tables for mapping typeSig names to definitions */
typedef struct typeLUTnode{
  lut_t  *typeLut; /* TypeSig names to definitions mapping */
  struct typeLUTnode *next;
}TypeLUTnode;

static TypeLUTnode *typeLUTstack = NULL; /* TypeSig names to definitions mapping */


/* Add new node to top of netLUTstack */
static void netLUTpush()
{
  NetLUTnode *newNode = (NetLUTnode *)MEMmalloc(sizeof(NetLUTnode));
  newNode->netLut = LUTgenerateLut();
  newNode->boxLut = LUTgenerateLut();
  newNode->next = NULL;

  if(netLUTstack == NULL) {
    netLUTstack = newNode;
  }
  else {
    newNode->next = netLUTstack;
    netLUTstack = newNode;
  }
}

/* Remove netLUTstack's top node */
static void netLUTpop()
{
  NetLUTnode *topNode = netLUTstack;
  if(topNode == NULL) {
    YYparseError("*** Trying to remove NULL node ***\n");
  }
  netLUTstack = topNode->next;
  LUTremoveLut(topNode->netLut);
  LUTremoveLut(topNode->boxLut);
  MEMfree(topNode);
}

/* Search item (with net name as key) from eNetLUT */
static void **eNetLUTsearch(char *key)
{
  return LUTsearchInLutS(eNetLUT, key);
}

/* Insert new item (net name to net definition mapping) to eNetLUT */
static void eNetLUTinsert(char *key, void *value)
{
  if(eNetLUTsearch(key) != NULL) {
    YYparseError("External network %s already defined elsewhere", key);
  }
  if(LUTsearchInLutS(boxLUT, key) != NULL) {
    YYparseError("External network %s has the same name as previously defined box", key);
  }
  if (STReq(key, global.filebase)) {
    YYparseError("Attempt to refer to self via external network");
  }
  LUTinsertIntoLutS(eNetLUT, key, value);
}

/* Insert new item (net name to net definition mapping) to netLUTstack's top node */
static void netLUTinsert(char *key, node *value)
{
  if(netLUTstack == NULL) {
    YYparseError("*** Trying to add item to NULL node ***\n");
  }
  if(LUTsearchInLutS(netLUTstack->netLut, key) != NULL) {
    YYparseError("Multiple network %s definitions", key);
  }
  if(LUTsearchInLutS(netLUTstack->boxLut, key) != NULL) {
    YYparseError("Network %s has the same name as previously defined box", key);
  }
  if(eNetLUTsearch(key) != NULL) {
    YYparseError("Network %s has the same name as previously defined external network", key);
  }
  LUTinsertIntoLutS(netLUTstack->netLut, key, value);
  if (NETDEF_EXTERNAL(value)) {
    eNetLUTinsert(key, value);
  }
}

/* Insert new item (box name to box definition mapping) to boxLUT */
static void boxLUTinsert(char *key, void *value)
{
  if(LUTsearchInLutS(boxLUT, key) != NULL) {
    YYparseError("Box %s already defined elsewhere", key);
  }
  LUTinsertIntoLutS(boxLUT, key, value);
}

/* Insert new item (box name to box definition mapping) to netLUTstack's top node */
static void netLUTinsertBox(char *key, void *value)
{
  if(netLUTstack == NULL) {
    YYparseError("*** Trying to add item to NULL node ***\n");
  }
  if(LUTsearchInLutS(netLUTstack->boxLut, key) != NULL) {
    YYparseError("Multiple box %s definitions", key);
  }
  if(LUTsearchInLutS(netLUTstack->netLut, key) != NULL) {
    YYparseError("Box %s has the same name as previously defined network", key);
  }
  if(eNetLUTsearch(key) != NULL) {
    YYparseError("Box %s has the same name as previously defined external network", key);
  }
  LUTinsertIntoLutS(netLUTstack->boxLut, key, value);
  boxLUTinsert(key, value);
}

/* Search item (with net name as key) from netLUTstack */
static void **netLUTsearch(char *key, bool *isBox)
{
  NetLUTnode *node = netLUTstack;
  void **retValue = NULL;

  while(node != NULL) {
    if((retValue = LUTsearchInLutS(node->netLut, key)) != NULL) {
      *isBox = FALSE;
      break;
    }
    if ((retValue = LUTsearchInLutS(node->boxLut, key)) != NULL) {
      *isBox = TRUE;
      break;
    }
    node = node->next;
  }
  return retValue;
}

/* Search just nets (with net name as key) from netLUTstack */
static void **netLUTsearchIgnoreBoxes(char *key)
{
  NetLUTnode *node = netLUTstack;
  void **retValue = NULL;

  while(node != NULL) {
    if((retValue = LUTsearchInLutS(node->netLut, key)) != NULL) {
      break;
    }
    node = node->next;
  }
  return retValue;
}

/* Add new node to top of typeSigLUTstack */
static void typeSigLUTpush()
{
  TypeSigLUTnode *newNode = (TypeSigLUTnode *)MEMmalloc(sizeof(TypeSigLUTnode));
  newNode->typeSigLut = LUTgenerateLut();
  newNode->next = NULL;

  if(typeSigLUTstack == NULL) {
    typeSigLUTstack = newNode;
  }
  else {
    newNode->next = typeSigLUTstack;
    typeSigLUTstack = newNode;
  }
}

/* Remove typeSigLUTstack's top node */
static void typeSigLUTpop()
{
  TypeSigLUTnode *topNode = typeSigLUTstack;
  if(topNode == NULL) {
    YYparseError("*** Trying to remove NULL node ***\n");
  }
  typeSigLUTstack = topNode->next;
  LUTremoveLut(topNode->typeSigLut);
  MEMfree(topNode);
}

/* Insert new item to typeSigLUTstack's top node */
static void typeSigLUTinsert(char *key, void *value)
{
  if(typeSigLUTstack == NULL) {
    YYparseError("*** Trying to add item to NULL node ***\n");
  }
  if(LUTsearchInLutS(typeSigLUTstack->typeSigLut, key) != NULL) {
    YYparseError("Multiple type signature %s definitions", key);
  }
  LUTinsertIntoLutS(typeSigLUTstack->typeSigLut, key, value);
}

/* Search item (with net name as key) from typeSigLUTstack */
static void **typeSigLUTsearch(char *key)
{
  TypeSigLUTnode *node = typeSigLUTstack;
  void **retValue = NULL;

  while(node != NULL) {
    if((retValue = LUTsearchInLutS(node->typeSigLut, key)) != NULL) {
      break;
    }
    node = node->next;
  }
  return retValue;
}

/* Add new node to top of typeLUTstack */
static void typeLUTpush()
{
  TypeLUTnode *newNode = (TypeLUTnode *)MEMmalloc(sizeof(TypeLUTnode));
  newNode->typeLut = LUTgenerateLut();
  newNode->next = NULL;

  if(typeLUTstack == NULL) {
    typeLUTstack = newNode;
  }
  else {
    newNode->next = typeLUTstack;
    typeLUTstack = newNode;
  }
}

/* Remove typeLUTstack's top node */
static void typeLUTpop()
{
  TypeLUTnode *topNode = typeLUTstack;
  if(topNode == NULL) {
    YYparseError("*** Trying to remove NULL node ***\n");
  }
  typeLUTstack = topNode->next;
  LUTremoveLut(topNode->typeLut);
  MEMfree(topNode);
}

/* Insert new item to typeLUTstack's top node */
static void typeLUTinsert(char *key, void *value)
{
  if(typeLUTstack == NULL) {
    YYparseError("*** Trying to add item to NULL node ***\n");
  }
  if(LUTsearchInLutS(typeLUTstack->typeLut, key) != NULL) {
    YYparseError("Multiple type %s definitions", key);
  }
  LUTinsertIntoLutS(typeLUTstack->typeLut, key, value);
}

/* Search item (with net name as key) from typeLUTstack */
static void **typeLUTsearch(char *key)
{
  TypeLUTnode *node = typeLUTstack;
  void **retValue = NULL;

  while(node != NULL) {
    if((retValue = LUTsearchInLutS(node->typeLut, key)) != NULL) {
      break;
    }
    node = node->next;
  }
  return retValue;
}

/* Add new definition(s) to the end of module definitions */
static void addDef(node *n)
{
  if(lastDef == NULL) {
    /* First node */
    lastDef = n;
    MODULE_DEFS(parseResult) = lastDef;
    return;
  }
  DEFS_NEXT(lastDef) = n;
  lastDef = n;
}

/* Add new field to the end of module fields */
static void addField(node *n)
{
  if(lastField == NULL) {
    /* First node */
    lastField = n;
    MODULE_FIELDS(parseResult) = n;
    return;
  }
  FIELDS_NEXT(lastField) = n;
  lastField = n;
}

/* Add new stag to the end of module stags */
static void addStag(node *n)
{
  if(lastStag == NULL) {
    /* First node */
    lastStag = n;
    MODULE_STAGS(parseResult) = n;
    return;
  }
  STAGS_NEXT(lastStag) = n;
  lastStag = n;
}

/* Add new btag to the end of module btags */
static void addBtag(node *n)
{
  if(lastBtag == NULL) {
    /* First node */
    lastBtag = n;
    MODULE_BTAGS(parseResult) = n;
    return;
  }
  BTAGS_NEXT(lastBtag) = n;
  lastBtag = n;
}

static void init() 
{
  lastDef    = NULL;
  lastField  = NULL;
  lastStag   = NULL;
  lastBtag   = NULL;

  fieldLUT   = LUTgenerateLut();
  stagLUT    = LUTgenerateLut();
  btagLUT    = LUTgenerateLut();
  netLUTpush();
  typeSigLUTpush();
  typeLUTpush();
  boxLUT     = LUTgenerateLut();
  eNetLUT    = LUTgenerateLut();
}

static void terminate()
{
  lastDef   = NULL;
  lastField = NULL;
  lastStag  = NULL;
  lastBtag  = NULL;

  LUTremoveLut(fieldLUT);
  LUTremoveLut(stagLUT);
  LUTremoveLut(btagLUT);
  netLUTpop();
  typeSigLUTpop();
  typeLUTpop();
  LUTremoveLut(boxLUT);
  LUTremoveLut(eNetLUT);
}

%}

%union {
  int       cint;
  char      *str;
  node      *node;
  struct {
    int line;
    int col;
  }         loc;
}


/* There should be 18 shift/reduce conflicts from TagExpr/TagExprFree rules. 
 * Bison resolves these correctly! */
%expect 18

/* common data */
%token <cint> INTEGER
%token <str>  IDENTIFIER CODE MD_VALUE

/* snet stages */
%token <loc> SNET_COMMENT SNET DISP DCR TCHECK BOXEX TRES CORE FLAT SIGINFD ROUTED MULINFD OPT TCLNED PTRAN DISAM PLOC NETREN FINAL

/* Metadata */
%token <loc> METADATA_END METADATA_BOXDEFAULT_BEGIN METADATA_NETDEFAULT_BEGIN METADATA_BOX_BEGIN METADATA_NET_BEGIN 
%token <loc> METADATA_DEFAULT_BEGIN METADATA_BOXDEFAULT_END METADATA_NETDEFAULT_END METADATA_BOX_END METADATA_NET_END 
%token <loc> METADATA_DEFAULT_END METADATA_ENDTAG_START METADATA_TAG_START METADATA_TAG_END METADATA_TAG_SHORT_END 
%token <loc> NAME VALUE DOUBLE_QUOTE METADATA_BEGIN

/* keywords */
%token <loc> BOX NET TYPE TYPESIG CONNECT IF ELSE THEN

/* combinators (and punctuations which can mark the start of network elements */
%token <loc> OPEN_BRACKET OPEN_BRACKET_VERTICAL_BAR
%token <loc> DOUBLE_PERIOD VERTICAL_BAR
%token <loc> ASTERISK EXLAMATION_POINT
%token <loc> DOUBLE_ASTERISK DOUBLE_VERTICAL_BAR DOUBLE_EXLAMATION_POINT
%token <loc> AT

/* punctuations */
%token <loc> OPEN_PARENTHESIS CLOSE_PARENTHESIS CLOSE_BRACKET OPEN_BRACE CLOSE_BRACE
%token <loc> COLON SEMI_COLON COMMA NUMBER_SIGN EQUAL_SIGN RIGHT_ARROW TRIPLE_LESS_THAN
%token <loc> TRIPLE_GREATER_THAN BACKSLASH DOUBLE_COLON VERTICAL_BAR_CLOSE_BRACKET
%token <loc> TRIPLE_PERIOD

/* tag operators */
%token <loc> LESS_THAN GREATER_THAN ABSOLUTE_VALUE DIVISION REMAINDER ADDITION
%token <loc> SUBTRACTION MINIMUM MAXIMUM DOUBLE_EQUAL NOT_EQUAL LESS_THAN_OR_EQUAL
%token <loc> GREATER_THAN_OR_EQUAL DOUBLE_AND QUESTION_POINT

/* nonterminals */
%type <node> Defs Def BoxDef BoxSign RightBoxTypes RightBoxType LeftBoxType BoxBody
%type <node> NetDef NetSign Type RecType RecEntries RecEntry
%type <node> Field STag BTag TypeDef NetBody TopoExpr Filt Trans
%type <node> Sync Pattern Combination Serial Star Feedback Term Choice Split Range At
%type <node> GuardPattern GuardPatterns GuardActions GuardAction Action RecOuts RecOut OutputField
%type <node> OutputFields TypeSigDef TypeSign TypeMap TagExpr TagExprFree TagExprRest TagExprEnd
%type <node> LeftRecEntries LeftRecEntry RightType RightRecType
%type <node> RightRecEntries RightRecEntry Branch DetBList DetTypedBList NondetBList NondetTypedBList 

%type <node> MetadataDef Constructs Construct BoxElement NetElement NetChilds DefaultElement Keys Key

/* associativity and precedence */
%nonassoc LOW
%left QUESTION_POINT COLON
%left VERTICAL_BAR
%left DOUBLE_VERTICAL_BAR 
%left DOUBLE_PERIOD
%left DOUBLE_AND
%left DOUBLE_EQUAL NOT_EQUAL 
%left LESS_THAN_OR_EQUAL GREATER_THAN_OR_EQUAL LESS_THAN GREATER_THAN 
%left ADDITION SUBTRACTION 
%left ASTERISK DIVISION REMAINDER DOUBLE_ASTERISK EXLAMATION_POINT DOUBLE_EXLAMATION_POINT
%left MINIMUM MAXIMUM AT
%left OPEN_PARENTHESIS CLOSE_PARENTHESIS 
%right ABSOLUTE_VALUE NEGATION
%nonassoc BACKSLASH
%nonassoc EQUAL_SIGN

%start Module

%%

Module:
      CompStage Defs 
      {
        DBUG_PRINT("SP", ("Module CompStage Defs"));
        addDef($2);
      }
    | { 
        DBUG_PRINT("SP", ("Module"));
      }
    ;

CompStage:
      /* empty */
      {
        DBUG_PRINT("SP", ("CompStage original"));
      }
    | SNET_COMMENT SNET DISP
      {
        DBUG_PRINT("SP", ("CompStage //! snet DISP"));
        global.compiler_phase = PH_preproc;
        global.compiler_subphase = SUBPH_dcr;

      }
    | SNET_COMMENT SNET DCR
      {
        DBUG_PRINT("SP", ("CompStage //! snet DCR"));
        global.compiler_phase = PH_preproc;
        global.compiler_subphase = SUBPH_tcheck;
      }
    | SNET_COMMENT SNET TCHECK
      {
        DBUG_PRINT("SP", ("CompStage //! snet THECK"));
        global.compiler_phase = PH_preproc;
        global.compiler_subphase = SUBPH_boxex;
      }
    | SNET_COMMENT SNET BOXEX
      {
        DBUG_PRINT("SP", ("CompStage //! snet BOXEX"));
        global.compiler_phase = PH_preproc;
        global.compiler_subphase = SUBPH_tres;
      }
    | SNET_COMMENT SNET TRES
      {
        DBUG_PRINT("SP", ("CompStage //! snet TRES"));
        global.compiler_phase = PH_preproc;
        global.compiler_subphase = SUBPH_mod;
      }
    | SNET_COMMENT SNET CORE
      {
        DBUG_PRINT("SP", ("CompStage //! snet CORE"));
        global.compiler_phase = PH_norm;
        global.compiler_subphase = SUBPH_topoflat;
      }
    | SNET_COMMENT SNET FLAT
      {
        DBUG_PRINT("SP", ("CompStage //! snet FLAT"));
        global.compiler_phase = PH_typesys;
        global.compiler_subphase = SUBPH_sip;
      }
    | SNET_COMMENT SNET SIGINFD
      {
        DBUG_PRINT("SP", ("CompStage //! snet SIGINFD"));
        global.compiler_phase = PH_typesys;
        global.compiler_subphase = SUBPH_ri;
      }
    | SNET_COMMENT SNET ROUTED
      {
        DBUG_PRINT("SP", ("CompStage //! snet ROUTED"));
        global.compiler_phase = PH_typesys;
        global.compiler_subphase = SUBPH_muli;
      }
    | SNET_COMMENT SNET MULINFD
      {
        DBUG_PRINT("SP", ("CompStage //! snet MULINFD"));
        global.compiler_phase = PH_opt;
      }
    | SNET_COMMENT SNET OPT
      {
        DBUG_PRINT("SP", ("CompStage //! snet OPT"));
        global.compiler_phase = PH_postproc;
      }
    | SNET_COMMENT SNET TCLNED
      {
        DBUG_PRINT("SP", ("CompStage //! snet TCLNED"));
        global.compiler_phase = PH_postproc;
        global.compiler_subphase = SUBPH_ptran;
      }
    | SNET_COMMENT SNET PTRAN
      {
        DBUG_PRINT("SP", ("CompStage //! snet PTRAN"));
        global.compiler_phase = PH_postproc;
        global.compiler_subphase = SUBPH_disam;
      }
    | SNET_COMMENT SNET DISAM
      {
        DBUG_PRINT("SP", ("CompStage //! snet DISAM"));
        global.compiler_phase = PH_postproc;
        global.compiler_subphase = SUBPH_ploc;
      }
    | SNET_COMMENT SNET PLOC
      {
        DBUG_PRINT("SP", ("CompStage //! snet PLOC"));
        global.compiler_phase = PH_postproc;
        global.compiler_subphase = SUBPH_netren;
      }
    | SNET_COMMENT SNET NETREN
      {
        DBUG_PRINT("SP", ("CompStage //! snet NETREN"));
        global.compiler_phase = PH_postproc;
        global.compiler_subphase = SUBPH_netflat;
      }
    | SNET_COMMENT SNET FINAL
      {
        DBUG_PRINT("SP", ("CompStage //! snet FINAL"));
        global.compiler_phase = PH_codegen;
      }
    ;

Defs:
      Def
      {
        DBUG_PRINT("SP", ("Defs Def"));
        $$ = TBmakeDefs($1, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | Def Defs
      {
        DBUG_PRINT("SP", ("Defs Def Defs"));
        $$ = TBmakeDefs($1, $2);
        NODE_COPYLOCTO($$, $1);
      }
    ;

Def:
      BoxDef 
      { 
        DBUG_PRINT("SP", ("Def BoxDef"));
        $$ = $1;
      }
    | NetDef 
      { 
        DBUG_PRINT("SP", ("Def NetDef"));
        $$ = $1;
      }
    | TypeDef
      { 
        DBUG_PRINT("SP", ("Def TypeDef"));
        $$ = $1;
      }
    | TypeSigDef
      {
        DBUG_PRINT("SP", ("Def TypeSigDef"));
        $$ = $1;
      }
    | MetadataDef
      {
        DBUG_PRINT("SP", ("Def MetadataDef"));
        $$ = $1;
      }
    ;

BoxDef:
      BOX IDENTIFIER OPEN_PARENTHESIS BoxSign CLOSE_PARENTHESIS BoxBody 
      {
        DBUG_PRINT("SP", ("BoxDef %s ( BoxSign ) BoxBody", $2));
        $$ = TBmakeBoxdef($2, STRcpy($2), $4, $6, NULL);
        COPYLOCTO($$, $1);
        netLUTinsertBox($2, $$);
      }
    | BOX IDENTIFIER IDENTIFIER OPEN_PARENTHESIS BoxSign CLOSE_PARENTHESIS BoxBody 
      {
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported multiple box names '%s %s'", $2, $3);
        }
        DBUG_PRINT("SP", ("BoxDef %s ( BoxSign ) BoxBody", $2));
        $$ = TBmakeBoxdef($2, $3, $5, $7, NULL); 
        COPYLOCTO($$, $1);
        netLUTinsertBox($2, $$);
      }
    ;

BoxSign:
      LeftBoxType RIGHT_ARROW RightBoxTypes 
      {
        DBUG_PRINT("SP", ("BoxSign LeftBoxType -> RightBoxTypes"));
        $$ = TBmakeBoxsign($1, $3);
        COPYLOCTO($$, $2);
      }
    | LeftBoxType RIGHT_ARROW 
      {
        DBUG_PRINT("SP", ("BoxSign LeftBoxType ->"));
        $$ = TBmakeBoxsign($1, NULL);
        COPYLOCTO($$, $2);
      }
    ;

RightBoxTypes:
      RightBoxType 
      {
        DBUG_PRINT("SP", ("RightBoxTypes RightBoxType"));
        $$ = $1;
      }
    | RightBoxType VERTICAL_BAR RightBoxTypes 
      { 
        DBUG_PRINT("SP", ("RightBoxTypes RightBoxType | RightBoxTypes"));
        BOXTYPES_NEXT($1) = $3;
        $$ = $1;
      }
    ;

LeftBoxType:
      OPEN_PARENTHESIS CLOSE_PARENTHESIS 
      { 
        DBUG_PRINT("SP", ("LeftBoxType ( )"));
        $$ = TBmakeBoxtypes(NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | OPEN_PARENTHESIS LeftRecEntries CLOSE_PARENTHESIS 
      {
        DBUG_PRINT("SP", ("LeftBoxType ( LeftRecEntries )"));
        $$ = TBmakeBoxtypes($2, NULL);
        COPYLOCTO($$, $1);
      }
    | /* nothing: initialiser box */
      {
        DBUG_PRINT("SP", ("LeftBoxType _nothing_"));
        $$ = NULL;
      }
    ;

LeftRecEntries: /* for box's left only, allowing "Entry=" syntax */
      LeftRecEntry 
      {
        DBUG_PRINT("SP", ("LeftRecEntries LeftRecEntry"));
        $$ = $1;
      }
    | LeftRecEntry COMMA LeftRecEntries  
      { 
        DBUG_PRINT("SP", ("LeftRecEntries LeftRecEntry, LeftRecEntries"));
        RECENTRIES_NEXT($1) = $3;
        $$ = $1;
      }
    ;

LeftRecEntry: /* for box's left only, allowing "Entry=" syntax */
      Field 
      {
        DBUG_PRINT("SP", ("LeftRecEntry Field"));
        $$ = TBmakeRecentries($1, NULL, NULL, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | LESS_THAN STag GREATER_THAN
      {
        DBUG_PRINT("SP", ("LeftRecEntry <%s>", STAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, $2, NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | LESS_THAN BTag GREATER_THAN
      {
        DBUG_PRINT("SP", ("LeftRecEntry <#%s>", BTAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, NULL, $2, NULL);
        COPYLOCTO($$, $1);
      }
    | LeftRecEntry EQUAL_SIGN
      {
        DBUG_PRINT("SP", ("LeftRecEntry LeftRecEntry ="));
        if (RECENTRIES_BTAG($1) != NULL) {
          YYparseError("Pass-through qualifiers not permitted with binding tags");
        }
        else if (RECENTRIES_QUALIFIER($1) != LQUA_none) {
          YYparseError("Multiple label qualifiers");
        }
        else {
          RECENTRIES_QUALIFIER($1) = LQUA_pass;
          $$ = $1;
        }
      }
    | LESS_THAN STag GREATER_THAN_OR_EQUAL
      {
        /* If ">=" is written as is without space it will be read as
         * GREATER_THAN_OR_EQUAL instead of GREATER_THAN and EQUAL_SIGN */
        DBUG_PRINT("SP", ("LeftRecEntry <%s>=", STAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, $2, NULL, NULL);
        COPYLOCTO($$, $1);
        RECENTRIES_QUALIFIER($$) = LQUA_pass;
      }
    | LESS_THAN BTag GREATER_THAN_OR_EQUAL
      {
        YYparseError("Pass-through qualifiers not permitted with binding tags");
      }
    ;

RightBoxType:
      OPEN_PARENTHESIS CLOSE_PARENTHESIS 
      { 
        DBUG_PRINT("SP", ("RightBoxType ( )"));
        $$ = TBmakeBoxtypes(NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | OPEN_PARENTHESIS RecEntries CLOSE_PARENTHESIS 
      {
        DBUG_PRINT("SP", ("RightBoxType ( RecEntries )"));
        $$ = TBmakeBoxtypes($2, NULL);
        COPYLOCTO($$, $1);
      }
    ;

RecEntries: /* standard record type, no qualifiers allowed */
      RecEntry 
      {
        DBUG_PRINT("SP", ("RecEntries RecEntry"));
        $$ = $1;
      }
    | RecEntry COMMA RecEntries  
      { 
        DBUG_PRINT("SP", ("RecEntries RecEntry, RecEntries"));
        RECENTRIES_NEXT($1) = $3;
        $$ = $1;
      }
    ;

BoxBody:
    OPEN_BRACE TRIPLE_LESS_THAN IDENTIFIER VERTICAL_BAR CODE TRIPLE_GREATER_THAN CLOSE_BRACE 
      { 
        DBUG_PRINT("SP", ("BoxBody { <<< %s | Code >>> }", $3));
        $$ = TBmakeBoxbody($3, $5);
        COPYLOCTO($$, $1);
      }
    | SEMI_COLON 
      {
        DBUG_PRINT("SP", ("BoxBody ;"));
        $$ = TBmakeBoxbody(NULL, NULL);
        COPYLOCTO($$, $1);
      }
    ;

NetDef:
      NET IDENTIFIER NetBody 
      { 
        DBUG_PRINT("SP", ("NetDef %s NetBody", $2));
        if($3 == NULL) {
          $$ = TBmakeNetdef($2, NULL, TRUE, FALSE, NULL, $3, NULL);
        }
        else {
          $$ = TBmakeNetdef($2, NULL, FALSE, FALSE, NULL, $3, NULL);
        }
        COPYLOCTO($$, $1);
        netLUTinsert($2, $$);
      }
    | 
      NET IDENTIFIER OPEN_PARENTHESIS NetSign CLOSE_PARENTHESIS NetBody
      {
        DBUG_PRINT("SP", ("NetDef %s ( NetSign ) NetBody", $2));
        if($6 == NULL) {
          $$ = TBmakeNetdef($2, NULL, TRUE, TRUE, $4, $6, NULL);
        }
        else {
          $$ = TBmakeNetdef($2, NULL, FALSE, TRUE, $4, $6, NULL);
        }
        COPYLOCTO($$, $1);
        netLUTinsert($2, $$);
      }
    | NET IDENTIFIER
      EXLAMATION_POINT OPEN_PARENTHESIS NetSign CLOSE_PARENTHESIS NetBody
      {
        DBUG_PRINT("SP", ("NetDef %s !( NetSign ) NetBody", $2));
        if(global.compiler_phase != PH_typesys) {
          YYparseError("Unsupported net type-signature");
        }
        if($7 == NULL) {
          $$ = TBmakeNetdef($2, NULL, TRUE, TRUE, $5, $7, NULL);
        }
        else {
          $$ = TBmakeNetdef($2, NULL, FALSE, TRUE, $5, $7, NULL);
        }
        COPYLOCTO($$, $1);
        netLUTinsert($2, $$);
      }

    | NET IDENTIFIER DOUBLE_COLON IDENTIFIER
      OPEN_PARENTHESIS NetSign CLOSE_PARENTHESIS NetBody 
      {
        DBUG_PRINT("SP", ("NetDef %s::%s ( NetSign ) NetBody", $2, $4));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported '::' net identifier");
        }
        char *key = STRcat($2, "::");
        $$ = TBmakeNetdef($4, $2, TRUE, TRUE, $6, $8, NULL);
        COPYLOCTO($$, $1);
        netLUTinsert(key, $$);
        MEMfree(key);
      }
    ;

NetSign:
      TypeSign
      {
        DBUG_PRINT("SP", ("NetSign TypeSign"));
        $$ = $1;
      }
    ;

TypeSigDef:
      TYPESIG IDENTIFIER EQUAL_SIGN TypeSign SEMI_COLON
      {
        DBUG_PRINT("SP", ("TypeSigDef typesig %s = TypeSignature ;", $2));
        $$ = TBmakeTypesigdef($2, $4);
        COPYLOCTO($$, $1);
        typeSigLUTinsert($2, $$);
      }
    ;

TypeSign: 
      TypeMap
      { 
        DBUG_PRINT("SP", ("TypeSign TypeMap"));
        $$ = TBmakeTypesigns($1 , NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | TypeMap COMMA TypeSign
      { 
        DBUG_PRINT("SP", ("TypeSign TypeMap , TypeSign"));
        $$ = TBmakeTypesigns($1 , $3);
        NODE_COPYLOCTO($$, $1);
      }
    | IDENTIFIER
      { 
        DBUG_PRINT("SP", ("TypeSign %s", $1));
        
        void **p = typeSigLUTsearch($1);
        if(p != NULL) {
          node * temp = TBmakeTypesigref(*p);
          ASSIGNLOC(temp);
          $$ = TBmakeTypesigns(temp, NULL);
          NODE_COPYLOCTO($$, temp);
        }
        else {
          YYparseError("Cannot find type signature definition: %s", $1);
        }
        MEMfree($1);
      }
    | IDENTIFIER COMMA TypeSign
      { 

        DBUG_PRINT("SP", ("TypeSign %s , TypeSign", $1));
        
        void **p = typeSigLUTsearch($1);
        if(p != NULL) {
          node * temp = TBmakeTypesigref(*p);
          COPYLOCTO(temp, $2);
          $$ = TBmakeTypesigns(temp, $3);
          COPYLOCTO($$, $2);
        }
        else {
          YYparseError("Cannot find type signature definition: %s", $1);
        }
        MEMfree($1);
        
      }
    ;


TypeMap:
      Type RIGHT_ARROW RightType 
      {
        DBUG_PRINT("SP", ("TypeMap Type -> RightType"));
        $$ = TBmakeTypemap(FALSE, $1, $3);
        COPYLOCTO($$, $2);
      }
    | Type RIGHT_ARROW
      {
        DBUG_PRINT("SP", ("TypeMap Type ->"));
        $$ = TBmakeTypemap(FALSE, $1, NULL);
        COPYLOCTO($$, $2);
      }
    | Type RIGHT_ARROW TRIPLE_PERIOD
      {
        DBUG_PRINT("SP", ("TypeMap Type -> ..."));
        $$ = TBmakeTypemap(TRUE, $1, NULL);
        COPYLOCTO($$, $2);
      }
    | RIGHT_ARROW RightType
      {
        DBUG_PRINT("SP", ("TypeMap -> RightType"));
        $$ = TBmakeTypemap(FALSE, NULL, $2);
        COPYLOCTO($$, $1);
      }
    | RIGHT_ARROW
      {
        DBUG_PRINT("SP", ("TypeMap ->"));
        $$ = TBmakeTypemap(FALSE, NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | RIGHT_ARROW TRIPLE_PERIOD
      {
        DBUG_PRINT("SP", ("TypeMap -> ..."));
        $$ = TBmakeTypemap(TRUE, NULL, NULL);
        COPYLOCTO($$, $1);
      }
    ;

RightType:
      IDENTIFIER
      {
        DBUG_PRINT("SP", ("RightType %s", $1));

        void **p = typeLUTsearch($1);
        if(p != NULL) {
          node *temp = TBmakeTyperef(*p);
          ASSIGNLOC(temp);
          $$ = TBmakeTypes(temp, NULL);
          NODE_COPYLOCTO($$, temp);
        }
        else {          
          YYparseError("Cannot find type definition: %s", $1);
        }
        MEMfree($1);
      }
    | IDENTIFIER VERTICAL_BAR RightType
      {
        DBUG_PRINT("SP", ("RightType %s | RightType", $1));

        void **p = typeLUTsearch($1);
        if(p != NULL) {
           node *temp = TBmakeTyperef(*p);
           COPYLOCTO(temp, $2);
           $$ = TBmakeTypes(temp, $3);
           COPYLOCTO($$, $2);
        }
        else {
          YYparseError("Cannot find type definition: %s", $1);
        }
        MEMfree($1);
      }
    | RightRecType 
      {
        DBUG_PRINT("SP", ("RightType RightRecType"));
        $$ = TBmakeTypes($1, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | RightRecType VERTICAL_BAR RightType
      {
        DBUG_PRINT("SP", ("RightType RightRecType | RightType"));
        $$ = TBmakeTypes($1, $3);
        NODE_COPYLOCTO($$, $1);
      }
    ;

RightRecType: /* sig output types, allow "=Entry" and "\Entry" */
      OPEN_BRACE CLOSE_BRACE 
      { 
        DBUG_PRINT("SP", ("RightRecType { }"));
        $$ = TBmakeRectype(NULL);
        COPYLOCTO($$, $1);
      }
    | OPEN_BRACE RightRecEntries CLOSE_BRACE 
      {
        DBUG_PRINT("SP", ("RightRecType { RightRecEntries }"));
        $$ = TBmakeRectype($2);
        COPYLOCTO($$, $1);
      }        
    ;

RightRecEntries: /* sig output types, allow "=Entry" and "\Entry" */
      RightRecEntry 
      {
        DBUG_PRINT("SP", ("RightRecEntries RightRecEntry"));
        $$ = $1;
      }
    | RightRecEntry COMMA RightRecEntries  
      { 
        DBUG_PRINT("SP", ("RightRecEntries RightRecEntry, RightRecEntries"));
        RECENTRIES_NEXT($1) = $3;
        $$ = $1;
      }
    ;

RightRecEntry: /* sig output types, allow "=Entry" and "\Entry" */
      Field 
      {
        DBUG_PRINT("SP", ("RightRecEntry Field"));
        $$ = TBmakeRecentries($1, NULL, NULL, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | LESS_THAN STag GREATER_THAN
      {
        DBUG_PRINT("SP", ("RightRecEntry <%s>", STAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, $2, NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | LESS_THAN BTag GREATER_THAN
      {
        DBUG_PRINT("SP", ("RightRecEntry <#%s>", BTAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, NULL, $2, NULL);
        COPYLOCTO($$, $1);
      }
    | EQUAL_SIGN RightRecEntry
      {
        DBUG_PRINT("SP", ("RightRecEntry = RightRecEntry"));
        if (RECENTRIES_BTAG($2) != NULL) {
          YYparseError("Pass-through qualifiers not permitted with binding tags");
        }
        else if (RECENTRIES_QUALIFIER($2) != LQUA_none) {
          YYparseError("Multiple label qualifiers");
        }
        else {
          RECENTRIES_QUALIFIER($2) = LQUA_pass;
          $$ = $2;
        }
      }
    | BACKSLASH RightRecEntry
      {
        DBUG_PRINT("SP", ("RightRecEntry \\ RightRecEntry"));
        if (global.compiler_phase < PH_typesys
         || global.compiler_phase > PH_postproc
         || (global.compiler_phase == PH_postproc &&
             global.compiler_subphase > SUBPH_tclean)) {
          YYparseError("Discard qualifiers not permitted at this phase");
        }
        else if (RECENTRIES_BTAG($2) != NULL) {
          YYparseError("Discard qualifiers not permitted with binding tags");
        }
        else if (RECENTRIES_QUALIFIER($2) != LQUA_none) {
          YYparseError("Multiple label qualifiers");
        }
        else {
          RECENTRIES_QUALIFIER($2) = LQUA_disc;
          $$ = $2;
        }
      }
    ;

Type:
      IDENTIFIER 
      {
        DBUG_PRINT("SP", ("Type %s", $1));

        void **p = typeLUTsearch($1);
        if(p != NULL) {
          node *temp = TBmakeTyperef(*p);
          ASSIGNLOC(temp);
          $$ = TBmakeTypes(temp, NULL);
          NODE_COPYLOCTO($$, temp);
        }
        else {          
          YYparseError("Cannot find type definition: %s", $1);
        }
        MEMfree($1);
      }
    | IDENTIFIER VERTICAL_BAR Type
      {
        DBUG_PRINT("SP", ("Type %s | Type", $1));

        void **p = typeLUTsearch($1);
        if(p != NULL) {
           node *temp = TBmakeTyperef(*p);
           COPYLOCTO(temp, $2);
           $$ = TBmakeTypes(temp, $3);
           COPYLOCTO($$, $2);
        }
        else {
          YYparseError("Cannot find type definition: %s", $1);
        }
        MEMfree($1);
      }
    | RecType 
      {
        DBUG_PRINT("SP", ("Type RecType"));
        $$ = TBmakeTypes($1, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | RecType VERTICAL_BAR Type
      {
        DBUG_PRINT("SP", ("Type RecType | Type"));
        $$ = TBmakeTypes($1, $3);
        NODE_COPYLOCTO($$, $1);
      }
    ;

RecType: /* standard record type, not allowing any qualifiers */
      OPEN_BRACE CLOSE_BRACE 
      { 
        DBUG_PRINT("SP", ("RecType { }"));
        $$ = TBmakeRectype(NULL);
        COPYLOCTO($$, $1);
      }
    | OPEN_BRACE RecEntries CLOSE_BRACE 
      {
        DBUG_PRINT("SP", ("RecType { RecEntries }"));
        $$ = TBmakeRectype($2);
        COPYLOCTO($$, $1);
      }        
    ;

RecEntry: /* standard record type, not allowing any qualifiers */
      Field 
      {
        DBUG_PRINT("SP", ("RecEntry Field"));
        $$ = TBmakeRecentries($1, NULL, NULL, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | LESS_THAN STag GREATER_THAN
      {
        DBUG_PRINT("SP", ("RecEntry <%s>", STAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, $2, NULL, NULL);
        COPYLOCTO($$, $1);
      }
    | LESS_THAN BTag GREATER_THAN
      {
        DBUG_PRINT("SP", ("RecEntry <#%s>", BTAG_NAME($2)));
        $$ = TBmakeRecentries(NULL, NULL, $2, NULL);
        COPYLOCTO($$, $1);
      }
    ;

Field:
    // NOTICE: Unlike in the grammar FieldName(s) in OutputField are 
    // implemented as Field (Field -> FieldName), because of ease of
    // implementation. If the Field definition is changed, these 
    // should be reconsidered also. 

      IDENTIFIER 
      {
        DBUG_PRINT("SP", ("Field %s", $1));
        void **p = LUTsearchInLutS(fieldLUT, $1);
        if(p == NULL) {
          /* New field */
          node *n = TBmakeFields($1, NULL, NULL, NULL);
          LUTinsertIntoLutS(fieldLUT, $1, n);
          addField(n);
          $$ = TBmakeFieldref(n, STRcpy($1));
        }
        else {
          $$ = TBmakeFieldref(*p, $1);
        }
        ASSIGNLOC($$);
      }
    | IDENTIFIER DOUBLE_COLON IDENTIFIER
      {
        DBUG_PRINT("SP", ("Field %s::%s", $1, $3));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported '::' field identifier");
        }
        char *key = STRcatn(3, $1, "::", $3);
        void **p = LUTsearchInLutS(fieldLUT, key);
        if(p == NULL) {
          /* New field */
          node *n = TBmakeFields($3, $1, NULL, NULL);
          LUTinsertIntoLutS(fieldLUT, key, n);
          addField(n);
          $$ = TBmakeFieldref(n, NULL);
        }
        else {
          MEMfree($1);
          MEMfree($3);
          $$ = TBmakeFieldref(*p, NULL);
        }
        MEMfree(key);
        COPYLOCTO($$, $2);
      }
    ;

STag:
    IDENTIFIER
      {
        DBUG_PRINT("SP", ("STag %s", $1));
        void **p = LUTsearchInLutS(stagLUT, $1);
        if(p == NULL) {
          /* New stag */
          node *n = TBmakeStags($1, NULL, NULL, NULL);
          LUTinsertIntoLutS(stagLUT, $1, n);
          addStag(n);
          $$ = TBmakeStagref(n, STRcpy($1));
        }
        else {
          $$ = TBmakeStagref(*p, $1);
        }
        ASSIGNLOC($$);
      }
    | IDENTIFIER DOUBLE_COLON IDENTIFIER
      {
        DBUG_PRINT("SP", ("STag %s::%s", $1, $3));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported '::' simple tag identifier");
        }
        char *key = STRcatn(3, $1, "::", $3);
        void **p = LUTsearchInLutS(stagLUT, key);
        if(p == NULL) {
          /* New stag */
          node *n = TBmakeStags($3, $1, NULL, NULL);
          LUTinsertIntoLutS(stagLUT, key, n);
          addStag(n);
          $$ = TBmakeStagref(n, NULL);
        }
        else {
          MEMfree($1);
          MEMfree($3);
          $$ = TBmakeStagref(*p, NULL);
        }
        MEMfree(key);
        COPYLOCTO($$, $2);
      }
    ;

BTag:
      NUMBER_SIGN IDENTIFIER 
      {
        DBUG_PRINT("SP", ("BTag %s", $2));
        void **p = LUTsearchInLutS(btagLUT, $2);
        if(p == NULL) {
          /* New btag */
          node *n = TBmakeBtags($2, NULL, NULL, NULL);
          LUTinsertIntoLutS(btagLUT, $2, n);
          addBtag(n);
          $$ = TBmakeBtagref(n, STRcpy($2));
        }
        else {
          $$ = TBmakeBtagref(*p, $2);
        }
        ASSIGNLOC($$);
      }
    | NUMBER_SIGN IDENTIFIER DOUBLE_COLON IDENTIFIER
      {
        DBUG_PRINT("SP", ("BTag %s::%s", $2, $4));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported '::' binding tag identifier");
        }
        char *key = STRcatn(3, $2, "::", $4);
        void **p = LUTsearchInLutS(btagLUT, key);
        if(p == NULL) {
          /* New btag */
          node *n = TBmakeBtags($4, $2, NULL, NULL);
          LUTinsertIntoLutS(btagLUT, key, n);
          addBtag(n);
          $$ = TBmakeBtagref(n, NULL);
        }
        else {
          MEMfree($2);
          MEMfree($4);
          $$ = TBmakeBtagref(*p, NULL);
        }
        MEMfree(key);
        COPYLOCTO($$, $1);
      }
    ;

TypeDef:
      TYPE IDENTIFIER EQUAL_SIGN Type SEMI_COLON 
      {
        DBUG_PRINT("SP", ("TypeDef %s = Type;", $2));
        $$ = TBmakeTypedef($2, $4);
        typeLUTinsert($2, $$);
        COPYLOCTO($$, $1);
      }
    ;

NetBody:
      NetBodyStart NetBodyStop CONNECT TopoExpr SEMI_COLON
      {
        DBUG_PRINT("SP", ("NetBody connect TopoExpr ;"));
        $$ = TBmakeNetbody(NULL, $4);
        COPYLOCTO($$,$3);
        netLUTpop();
        typeSigLUTpop();
        typeLUTpop();
      }
    | NetBodyStart Defs NetBodyStop CONNECT TopoExpr SEMI_COLON
      {
        DBUG_PRINT("SP", ("NetBody Defs connect TopoExpr ;"));
        $$ = TBmakeNetbody($2, $5);
        COPYLOCTO($$,$4);
        netLUTpop();
        typeSigLUTpop();
        typeLUTpop();
      }
    |  CONNECT TopoExpr SEMI_COLON
      {
        DBUG_PRINT("SP", ("NetBody connect TopoExpr ;"));
        $$ = TBmakeNetbody(NULL, $2);
        COPYLOCTO($$,$1);
      }
    | SEMI_COLON // This isn't in the spesification but is needed for external nets in later phases
      {
        DBUG_PRINT("SP", ("NetBody connect ;"));
        if(global.compiler_phase < PH_preproc) {
          YYparseError("Connect undeclared");
        }
        $$ = NULL;
      }
    ;

NetBodyStart:
      OPEN_BRACE
      {
        DBUG_PRINT("SP", ("NetBodyStart"));
        netLUTpush();
        typeSigLUTpush();
        typeLUTpush();
      }
    ;

NetBodyStop:
      CLOSE_BRACE
      {
        DBUG_PRINT("SP", ("NetBodyStop"));
      }
    ;

TopoExpr:
      IDENTIFIER
      {
        DBUG_PRINT("SP", ("TopoExpr %s", $1));
        bool isBox;
        void **p = netLUTsearch($1, &isBox);
        if(p != NULL) {
          if (isBox) {
            /* Box reference */
            $$ = TBmakeBoxref(*p);
          }
          else {
            /* Net reference */
            $$ = TBmakeNetrefs(*p, NULL);
          }
          MEMfree($1);
        }
        else {
          /* External network */
          void **n = eNetLUTsearch($1);
          if(n == NULL){
            node *temp = TBmakeNetdef($1, NULL, TRUE, FALSE, NULL, NULL, NULL);
            ASSIGNLOC(temp);
            eNetLUTinsert($1, temp);
            addDef(TBmakeDefs(temp, NULL));
            $$ = TBmakeNetrefs(temp, NULL);
          }else{
            MEMfree($1);
            $$ = TBmakeNetrefs(*n, NULL);
          }
        }
        ASSIGNLOC($$);
      }
    | IDENTIFIER DOUBLE_COLON IDENTIFIER
      {
        DBUG_PRINT("SP", ("TopoExpr %s::%s", $1, $3));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported '::' identifier");
        }
        char *key = STRcat($1, "::");
        void **p = netLUTsearchIgnoreBoxes(key);
        if(p != NULL) {
          $$ = TBmakeNetrefs(*p, NULL);
        }
        else {
          YYparseError("Identifier '%s' undeclared", key);
        }
        MEMfree($1);
        MEMfree($3);
        MEMfree(key);
        COPYLOCTO($$, $2);
      }
    | Sync
      {
        DBUG_PRINT("SP", ("TopoExpr Sync"));
        $$ = $1;
      }
    | Filt 
      {
        DBUG_PRINT("SP", ("TopoExpr Filt"));
        $$ = $1;
      }
    | Trans 
      {
        DBUG_PRINT("SP", ("TopoExpr Trans"));
        $$ = $1;
      }
    | Combination
      {
        DBUG_PRINT("SP", ("TopoExpr Combination"));
        $$ = $1;
      }
    | OPEN_PARENTHESIS TopoExpr CLOSE_PARENTHESIS 
      {
        DBUG_PRINT("SP", ("TopoExpr (TopoExpr)"));
        $$ = $2;
      }
    ;

Filt:
      OPEN_BRACKET Pattern RIGHT_ARROW Action CLOSE_BRACKET
      {
        DBUG_PRINT("SP", ("Filt [ Pattern -> Action ]"));
        node *ga = TBmakeGuardactions(NULL, $4, NULL);
        if ($4 == NULL) {
          COPYLOCTO(ga, $3);
        }
        else {
          NODE_COPYLOCTO(ga, $4);
        }
        $$ = TBmakeFilt($2, ga);
        COPYLOCTO($$, $1);
      }
    | OPEN_BRACKET Pattern RIGHT_ARROW GuardActions Action CLOSE_BRACKET
      {
        DBUG_PRINT("SP", ("Filt [ Pattern -> GuardActions Action ]"));
        node *ga = TBmakeGuardactions(NULL, $5, NULL);
        if ($5 == NULL) {
          COPYLOCTO(ga, $6);
        }
        else {
          NODE_COPYLOCTO(ga, $5);
        }
        node *gas = $4;
        while (GUARDACTIONS_NEXT(gas) != NULL) {
          gas = GUARDACTIONS_NEXT(gas);
        }
        GUARDACTIONS_NEXT(gas) = ga;
        $$ = TBmakeFilt($2, $4);
        COPYLOCTO($$, $1);
      }
    | OPEN_BRACKET CLOSE_BRACKET
      {
        DBUG_PRINT("SP", ("Filt [ ]"));
        $$ = TBmakeFilt(NULL, NULL);
        COPYLOCTO($$, $1);
      }
    ;

Action:
      RecOuts
      {
        DBUG_PRINT("SP", ("Action RecOuts"));
        $$ = $1;
      }
    | /* Empty*/
      {
        DBUG_PRINT("SP", ("Action _nothing_"));
        $$ = NULL;
      }
    ;

GuardActions:
      GuardAction
      {
        DBUG_PRINT("SP", ("GuardActions GuardAction"));
        $$ = $1;
      }
    | GuardAction GuardActions
      {
        DBUG_PRINT("SP", ("GuardActions GuardAction GuardActions"));
        GUARDACTIONS_NEXT($1) = $2;
        $$ = $1;
      }
    ;

GuardAction:
      IF LESS_THAN TagExpr THEN Action ELSE
      {
        DBUG_PRINT("SP", ("GuardAction if < TagExpr then Action else"));
        $$ =  TBmakeGuardactions($3, $5, NULL);
        COPYLOCTO($$, $1);
      }
    ;

Trans:
      OPEN_BRACKET RightBoxType RIGHT_ARROW RightBoxType CLOSE_BRACKET 
      {
        DBUG_PRINT("SP", ("Trans [ RightBoxType -> RightBoxType ]"));
        if(global.compiler_phase < PH_postproc) {
          YYparseError("Unsupported [RightBoxType -> RightBoxType] definition");
        }
        $$ = TBmakeTrans($2, $4);
        COPYLOCTO($$, $1);
      }
    ;

RecOuts:
      RecOut
      {
        DBUG_PRINT("SP", ("RecOuts RecOut"));
        $$ = TBmakeRecouts($1, NULL);
        if ($1 == NULL) {
          ASSIGNLOC($$);
        }
        else {
          NODE_COPYLOCTO($$, $1);
        }
      }
    | RecOut SEMI_COLON RecOuts
      {
        DBUG_PRINT("SP", ("RecOuts RecOut ; RecOuts"));
        $$ = TBmakeRecouts($1, $3);
        if ($1 == NULL) {
          ASSIGNLOC($$);
        }
        else {
          NODE_COPYLOCTO($$, $1);
        }
      }
    ;

RecOut:
      OPEN_BRACE CLOSE_BRACE
      {
        DBUG_PRINT("SP", ("RecOut { }"));
        $$ = NULL;
      }
    | OPEN_BRACE OutputFields CLOSE_BRACE
      {
        DBUG_PRINT("SP", ("RecOut { OutputFields }"));
        $$ = $2;
      }
    ;

OutputFields:
      OutputField 
      {
        DBUG_PRINT("SP", ("OutputFields OutputField"));
        $$ = $1;
      }
    | OutputField COMMA OutputFields 
      { 
        DBUG_PRINT("SP", ("OutputFields OutputField, OutputFields"));
        OUTPUTFIELDS_NEXT($1) = $3;
        $$ = $1;
      }
    ;

OutputField:
    // NOTICE: In the Grammar what are now Fields in this rule are 
    // actually FieldNames, but Field -> FieldName. If Field rule 
    // is changed, these should be reconsidered!

      Field 
      { 
        DBUG_PRINT("SP", ("OutputField %s", $1));
        $$ = TBmakeOutputfields($1, NULL, NULL, NULL, NULL, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    |  Field EQUAL_SIGN Field
      { 
        DBUG_PRINT("SP", ("OutputField %s = %s", $1, $3));
        $$ = TBmakeOutputfields($1, $3, NULL, NULL, NULL, NULL);
        NODE_COPYLOCTO($$, $1);
      }
    | LESS_THAN STag GREATER_THAN
      { 
        DBUG_PRINT("SP", ("OutputField <%s>", STAG_NAME($2)));
        $$ = TBmakeOutputfields(NULL, NULL, $2, NULL, NULL, NULL);
        COPYLOCTO($$, $1);
      }
   | LESS_THAN BTag GREATER_THAN
      { 
        DBUG_PRINT("SP", ("OutputField <#%s>", BTAG_NAME($2)));
        $$ = TBmakeOutputfields(NULL, NULL, NULL, $2, NULL, NULL);
        COPYLOCTO($$, $1);
      }
   | LESS_THAN STag EQUAL_SIGN TagExpr
      { 
        DBUG_PRINT("SP", ("OutputField <%s = TagExpr", STAG_NAME($2)));
        $$ = TBmakeOutputfields(NULL, NULL, $2, NULL, $4, NULL);
        COPYLOCTO($$, $1);
      }
   | LESS_THAN BTag EQUAL_SIGN TagExpr
      { 
        DBUG_PRINT("SP", ("OutputField <#%s = TagExpr", BTAG_NAME($2)));
        $$ = TBmakeOutputfields(NULL, NULL, NULL, $2, $4 , NULL);
        COPYLOCTO($$, $1);
      }
    ;

Sync:
      OPEN_BRACKET_VERTICAL_BAR GuardPattern COMMA GuardPatterns VERTICAL_BAR_CLOSE_BRACKET 
      { 
        DBUG_PRINT("SP", ("Sync [| GuardPattern, GuardPatterns|]"));
        $$ = TBmakeSync($2, $4);
        COPYLOCTO($$, $1);
      }
    ;

Pattern:
      // NOTICE: Unlike in the grammar Pattern is defined as 
      // RecType as their definition is the same

      RecType
      {
        DBUG_PRINT("SP", ("Pattern RecType"));
        $$ = $1;
      }
      ;

GuardPatterns:
      GuardPattern 
      { 
        DBUG_PRINT("SP", ("GuardPatterns GuardPattern"));
        $$ = $1;
      }
    | GuardPattern COMMA GuardPatterns 
      {
        DBUG_PRINT("SP", ("GuardPatterns GuardPattern, GuardPatterns"));
        GUARDPATTERNS_NEXT($1) = $3;
        $$ = $1;
      }
    ;

GuardPattern:
      Pattern
      {
        DBUG_PRINT("SP", ("GuardPattern Pattern"));
        $$ = TBmakeGuardpatterns($1 , NULL , NULL);  
        NODE_COPYLOCTO($$, $1);
      }
    | Pattern IF LESS_THAN TagExpr

      {
        DBUG_PRINT("SP", ("GuardPattern Pattern if < TagExpr"));
        $$ = TBmakeGuardpatterns($1 , $4 , NULL);
        NODE_COPYLOCTO($$, $1);
      }
    ;


TagExpr:
    EXLAMATION_POINT TagExprRest %prec NEGATION
      { 
        DBUG_PRINT("SP", ("TagExpr ! TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , $2);
        TAGEXPR_OPERATOR(temp) = OPER_NOT;
        $$ = temp;
        COPYLOCTO($$, $1);
      }
    | ABSOLUTE_VALUE TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr abs TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , $2);
        TAGEXPR_OPERATOR(temp) = OPER_ABS;
        $$ = temp;
        COPYLOCTO($$, $1);
      }
    | TagExprFree ASTERISK TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree * TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MUL;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DIVISION TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr  TagExprFree / TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_DIV;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree REMAINDER TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree %% TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_REM;        
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree ADDITION TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree + TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_ADD;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree SUBTRACTION TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree - TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_SUB;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree MINIMUM TagExprRest
      {
        DBUG_PRINT("SP", ("TagExpr TagExprFree min TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MIN;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree MAXIMUM TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree max TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MAX;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_EQUAL TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree == TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_EQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree NOT_EQUAL TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree != TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_NEQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree LESS_THAN TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree < TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_LT;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree LESS_THAN_OR_EQUAL TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree <= TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_LTQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree GREATER_THAN TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree > TagExprRest"));
        if($3 != NULL) {
          node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
          TAGEXPR_OPERATOR(temp) = OPER_GT;
          $$ = temp;
          COPYLOCTO($$, $2);
        }else{ // End ">" of the TagExpr
          $$ = $1;
        }
      }
    | TagExprFree GREATER_THAN_OR_EQUAL TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree >= TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_GTQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_AND TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree && TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_AND;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_VERTICAL_BAR TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree || TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_OR;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree QUESTION_POINT TagExprFree COLON TagExprRest
      { 
        DBUG_PRINT("SP", ("TagExpr TagExprFree ? TagExprFree : TagExprRest"));
        node *temp = TBmakeTagexpr(NULL, NULL, $1 , $3 ,$5);
        TAGEXPR_OPERATOR(temp) = OPER_COND;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    ;

// These rules are used to cope with greater than expression and tagExpr end ">"
TagExprRest: 
      TagExpr
      {
        DBUG_PRINT("SP", ("TagExprRest TagExpr"));
        $$ = $1;
      }
    | /* empty */
      {
        DBUG_PRINT("SP", ("TagExprRest 'empty'"));
        $$ = NULL;
      }
    ;

TagExprEnd:
      STag
      { 
        DBUG_PRINT("SP", ("TagExprEnd %s", STAG_NAME($1)));
        node *temp = TBmakeTagexpr($1, NULL, NULL, NULL , NULL);
        TAGEXPR_OPERATOR(temp) = OPER_NONE;
        $$ = temp;
        NODE_COPYLOCTO($$, $1);
      }
    | BTag
      { 
        DBUG_PRINT("SP", ("TagExprEnd #%s", BTAG_NAME($1)));
        node *temp = TBmakeTagexpr(NULL, $1, NULL, NULL , NULL);
        TAGEXPR_OPERATOR(temp) = OPER_NONE;
        $$ = temp;
        NODE_COPYLOCTO($$, $1);
      }
    | INTEGER
      { 
        DBUG_PRINT("SP", ("TagExprEnd %d", $1));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , NULL);
        TAGEXPR_INTEGERCONST(temp) = $1;
        TAGEXPR_OPERATOR(temp) = OPER_NONE;
        $$ = temp;
        ASSIGNLOC($$);
      }
    | SUBTRACTION INTEGER
      { 
        DBUG_PRINT("SP", ("TagExprEnd -%d", $2));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , NULL);
        TAGEXPR_INTEGERCONST(temp) = -($2);
        TAGEXPR_OPERATOR(temp) = OPER_NONE;
        $$ = temp;
        ASSIGNLOC($$);
      }
    | OPEN_PARENTHESIS TagExprFree CLOSE_PARENTHESIS
      { 
        DBUG_PRINT("SP", ("TagExprEnd ( TagExprFree )"));
        $$ = $2;
      }
    ;

// These rules are used for tagExprs that don't have to end with tag ">"
TagExprFree:
      TagExprEnd
      {
        DBUG_PRINT("SP", ("TagExprFree TagExprEnd"));
        $$ = $1;
      }
    | EXLAMATION_POINT TagExprFree %prec NEGATION

      { 
        DBUG_PRINT("SP", ("TagExprFree  ! TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , $2);
        TAGEXPR_OPERATOR(temp) = OPER_NOT;
        $$ = temp;
        COPYLOCTO($$, $1);
      }
    | ABSOLUTE_VALUE TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree abs TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, NULL , $2);
        TAGEXPR_OPERATOR(temp) = OPER_ABS;
        $$ = temp;
        COPYLOCTO($$, $1);
      }
    | TagExprFree ASTERISK TagExprFree 
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree * TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MUL;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DIVISION TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree / TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_DIV;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree REMAINDER TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree %% TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_REM;        
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree ADDITION TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree + TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_ADD;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree SUBTRACTION TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree - TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_SUB;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree MINIMUM TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree min TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MIN;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree MAXIMUM TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree max TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_MAX;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_EQUAL TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree  TagExprFree == TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_EQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree NOT_EQUAL TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree != TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_NEQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree LESS_THAN TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree < TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_LT;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree LESS_THAN_OR_EQUAL TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree <= TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_LTQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree GREATER_THAN TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree > TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_GT;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree GREATER_THAN_OR_EQUAL TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree >= TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_GTQ;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_AND TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree && TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_AND;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    | TagExprFree DOUBLE_VERTICAL_BAR TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree || TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, NULL, $1 , $3);
        TAGEXPR_OPERATOR(temp) = OPER_OR;
        $$ = temp;
        COPYLOCTO($$, $2);
      }

    | TagExprFree QUESTION_POINT TagExprFree COLON TagExprFree
      { 
        DBUG_PRINT("SP", ("TagExprFree TagExprFree ? TagExprFree : TagExprFree"));
        node *temp = TBmakeTagexpr(NULL, NULL, $1 , $3 ,$5);
        TAGEXPR_OPERATOR(temp) = OPER_COND;
        $$ = temp;
        COPYLOCTO($$, $2);
      }
    ;



Combination:
      Serial
      { 
        DBUG_PRINT("SP", ("Combination Serial"));
        $$ = $1;
      }
    | Star
      { 
        DBUG_PRINT("SP", ("Combination Star"));
        $$ = $1;
      }
    | Choice
      { 
        DBUG_PRINT("SP", ("Combination Choice"));
        $$ = $1;
      }
    | Split
      { 
        DBUG_PRINT("SP", ("Combination Split"));
        $$ = $1;
      }
    | Feedback
      { 
        DBUG_PRINT("SP", ("Combination Feedback"));
        $$ = $1;
      }
    | At
      { 
        DBUG_PRINT("SP", ("Combination At"));
        $$ = $1;
      }
    ;

Serial:
      TopoExpr DOUBLE_PERIOD TopoExpr 
      { 
        DBUG_PRINT("SP", ("Serial TopoExpr .. TopoExpr"));
        $$ = TBmakeSerial($1, $3);
        COPYLOCTO($$, $2);
      }
    ;

Star:
      TopoExpr DOUBLE_ASTERISK Term
      {
        DBUG_PRINT("SP", ("Star TopoExpr ** Term"));
        $$ = TBmakeStar($1, $3);
        COPYLOCTO($$, $2);
        STAR_ISDETERM($$) = TRUE;        
      }
    | TopoExpr ASTERISK Term 
      {
        DBUG_PRINT("SP", ("Star TopoExpr * Term"));
        $$ = TBmakeStar($1, $3);
        COPYLOCTO($$, $2);
        STAR_ISDETERM($$) = FALSE;        
      }
    ;

Feedback:
      TopoExpr BACKSLASH Term
      {
        DBUG_PRINT("SP", ("Feedback TopoExpr \\ Term"));
        $$ = TBmakeFeedback($1, $3);
        COPYLOCTO($$, $2);
      }
    ;

Term:
      GuardPatterns
      {
        DBUG_PRINT("SP", ("Term GuardPatterns"));
        $$ = $1;
      }
    ;

Choice:
      DetBList %prec LOW /* lower than DetBList = DetBList || TopoExpr */
      {
        DBUG_PRINT("SP", ("Choice DetBList"));
        $$ = TBmakeChoice($1);
        NODE_COPYLOCTO($$, $1);
        CHOICE_ISDETERM($$) = TRUE;
      }
    | DetTypedBList
      {
        DBUG_PRINT("SP", ("Choice DetTypedBList"));
        $$ = TBmakeChoice($1);
        NODE_COPYLOCTO($$, $1);
        CHOICE_ISDETERM($$) = TRUE;
      }
    | NondetBList %prec LOW /* lower than NondetBLlist = NondetBList | TopoExpr */
      {
        DBUG_PRINT("SP", ("Choice NondetBList"));
        $$ = TBmakeChoice($1);
        NODE_COPYLOCTO($$, $1);
        CHOICE_ISDETERM($$) = FALSE;
      }
    | NondetTypedBList
      {
        DBUG_PRINT("SP", ("Choice NondetTypedBList"));
        $$ = TBmakeChoice($1);
        NODE_COPYLOCTO($$, $1);
        CHOICE_ISDETERM($$) = FALSE;
      }
    ;

DetBList:
      TopoExpr DOUBLE_VERTICAL_BAR TopoExpr
      {
        DBUG_PRINT("SP", ("DetBList TopoExpr || TopoExpr"));
        node *temp = TBmakeBranchlist(NULL, $3, NULL);
        NODE_COPYLOCTO(temp, $3);
        $$ = TBmakeBranchlist(NULL, $1, temp);
        NODE_COPYLOCTO($$, $1);
        BRANCHLIST_COUNT($$) = 2;
      }
    | DetBList DOUBLE_VERTICAL_BAR TopoExpr
       /* note: this rule is not Topo || DetBList because in the latter case
          the parser confuses between reducing just the DetBList into Choice
          (making the stack read "Topo || Topo") and reducing the whole to
          DetBList. A reduce/reduce conflict can't be solved with precedence. */
      {
        DBUG_PRINT("SP", ("DetBList DetBList || TopoExpr"));
        node *temp = TBmakeBranchlist(NULL, $3, NULL);
        NODE_COPYLOCTO(temp, $3);
        node *temp2 = $1;
        while (BRANCHLIST_NEXT(temp2) != NULL) {
          BRANCHLIST_COUNT(temp2)++;
          temp2 = BRANCHLIST_NEXT(temp2);
        }
        BRANCHLIST_COUNT(temp2)++;
        BRANCHLIST_NEXT(temp2) = temp;
        $$ = $1;
      }
    ;

DetTypedBList:
      Branch DOUBLE_VERTICAL_BAR Branch
      {
        DBUG_PRINT("SP", ("DetTypedBList Branch || Branch"));
        BRANCHLIST_NEXT($1) = $3;
        BRANCHLIST_COUNT($1) = 2;
        $$ = $1;
      }
    | Branch DOUBLE_VERTICAL_BAR DetTypedBList
      {
        DBUG_PRINT("SP", ("DetTypedBList Branch || DetTypedBList"));
        BRANCHLIST_NEXT($1) = $3;
        BRANCHLIST_COUNT($1) = BRANCHLIST_COUNT($3) + 1;
        $$ = $1;
      }
    ;
    
NondetBList:
      TopoExpr VERTICAL_BAR TopoExpr
      {
        DBUG_PRINT("SP", ("NondetBList TopoExpr | TopoExpr"));
        node *temp = TBmakeBranchlist(NULL, $3, NULL);
        NODE_COPYLOCTO(temp, $3);
        $$ = TBmakeBranchlist(NULL, $1, temp);
        NODE_COPYLOCTO($$, $1);
        BRANCHLIST_COUNT($$) = 2;
      }
    | NondetBList VERTICAL_BAR TopoExpr /* check out comments in DetBList. */
      {
        DBUG_PRINT("SP", ("NondetBList NondetBList | TopoExpr"));
        node *temp = TBmakeBranchlist(NULL, $3, NULL);
        NODE_COPYLOCTO(temp, $3);
        node *temp2 = $1;
        while (BRANCHLIST_NEXT(temp2) != NULL) {
          BRANCHLIST_COUNT(temp2)++;
          temp2 = BRANCHLIST_NEXT(temp2);
        }
        BRANCHLIST_COUNT(temp2)++;
        BRANCHLIST_NEXT(temp2) = temp;
        $$ = $1;
      }
    ;

NondetTypedBList:
      Branch VERTICAL_BAR Branch
      {
        DBUG_PRINT("SP", ("NondetTypedBList Branch | Branch"));
        BRANCHLIST_NEXT($1) = $3;
        BRANCHLIST_COUNT($1) = 2;
        $$ = $1;
      }
    | Branch VERTICAL_BAR NondetTypedBList
      {
        DBUG_PRINT("SP", ("NondetTypedBList Branch | NondetTypedBList"));
        BRANCHLIST_NEXT($1) = $3;
        BRANCHLIST_COUNT($1) = BRANCHLIST_COUNT($3) + 1;
        $$ = $1;
      }
    ;
    
Branch: /* typed branch. TopoExpr should be just NetRefs but the distributed
         * S-Net may wrap it in an At node, so let's just use the superclass */
      LESS_THAN Type GREATER_THAN TopoExpr
      {
        DBUG_PRINT("SP", ("Branch < Type > TopoExpr"));
        $$ = TBmakeBranchlist($2, $4, NULL);
        BRANCHLIST_TYPED($$) = TRUE;
        COPYLOCTO($$, $1);
      }
    | LESS_THAN GREATER_THAN TopoExpr
      {
        DBUG_PRINT("SP", ("Branch < > TopoExpr", $3));
        $$ = TBmakeBranchlist(NULL, $3, NULL);
        BRANCHLIST_TYPED($$) = TRUE;
        COPYLOCTO($$, $1);
      }
    ;

Split:
      TopoExpr DOUBLE_EXLAMATION_POINT Range
      {
        DBUG_PRINT("SP", ("Split TopoExpr !! Range"));
        $$ = TBmakeSplit($1, $3);
        COPYLOCTO($$, $2);
        SPLIT_ISDETERM($$) = TRUE;
      }
    | TopoExpr EXLAMATION_POINT Range
      {
        DBUG_PRINT("SP", ("Split TopoExpr ! Range"));
        $$ = TBmakeSplit($1, $3);
        COPYLOCTO($$, $2);
        SPLIT_ISDETERM($$) = FALSE;
      }
    | TopoExpr DOUBLE_EXLAMATION_POINT AT Range
      {
        DBUG_PRINT("SP", ("Split TopoExpr !! Range"));
        $$ = TBmakeSplit($1, $4);
        COPYLOCTO($$, $2);
        SPLIT_ISDETERM($$) = TRUE;
        SPLIT_ISDISTRIBUTED($$) = TRUE;
      }
    | TopoExpr EXLAMATION_POINT AT Range
      {
        DBUG_PRINT("SP", ("Split TopoExpr ! Range"));
        $$ = TBmakeSplit($1, $4);
        COPYLOCTO($$, $2);
        SPLIT_ISDETERM($$) = FALSE;
        SPLIT_ISDISTRIBUTED($$) = TRUE;
      }
    ;

Range:
      LESS_THAN STag GREATER_THAN
      {
        DBUG_PRINT("SP", ("Range <%s>", STAG_NAME($2)));
        $$ = TBmakeRange(FILT_stag, FILT_empty, $2, NULL, NULL, NULL); 
        COPYLOCTO($$, $1);       
      }
    | LESS_THAN BTag GREATER_THAN
      {
        DBUG_PRINT("SP", ("Range <#%s>", BTAG_NAME($2)));
        $$ = TBmakeRange(FILT_btag, FILT_empty, NULL, $2, NULL, NULL);  
              COPYLOCTO($$, $1);  
      }
//    | LESS_THAN STag GREATER_THAN COLON LESS_THAN STag GREATER_THAN
//      {
//        DBUG_PRINT("SP", ("Range <%s> : <%s>", STAG_NAME($2), STAG_NAME($6)));
//        $$ = TBmakeRange(FILT_stag, FILT_stag, $2, NULL, $6, NULL);     
//           COPYLOCTO($$, $1);  
//      }
//    | LESS_THAN STag GREATER_THAN COLON LESS_THAN BTag GREATER_THAN
//      {
//        DBUG_PRINT("SP", ("Range <%s> : <#%s>", STAG_NAME($2), BTAG_NAME($6)));
//        $$ = TBmakeRange(FILT_stag, FILT_btag, $2, NULL, NULL, $6); 
//               COPYLOCTO($$, $1);  
//      }
//    | LESS_THAN BTag GREATER_THAN COLON LESS_THAN STag GREATER_THAN
//      {
//        DBUG_PRINT("SP", ("Range <#%s> : <%s>", BTAG_NAME($2), STAG_NAME($6)));
//        $$ = TBmakeRange(FILT_btag, FILT_stag, NULL, $2, $6, NULL);  
//              COPYLOCTO($$, $1);  
//      }
//    | LESS_THAN BTag GREATER_THAN COLON LESS_THAN BTag GREATER_THAN
//      {
//        DBUG_PRINT("SP", ("Range <#%s> : <#%s>", BTAG_NAME($2), BTAG_NAME($6)));
//        $$ = TBmakeRange(FILT_btag, FILT_btag, NULL, $2, NULL, $6); 
//               COPYLOCTO($$, $1);  
//      }
    ;

At:
      TopoExpr AT INTEGER 
      { 
        DBUG_PRINT("SP", ("At TopoExpr @ INTEGER"));

        if($3 < 0) {
          YYparseError("Invalid location argument %d", $3);
        }

        $$ = TBmakeAt($3, $1);

        if(!(global.compiler_phase <= PH_postproc
           && global.compiler_subphase <= SUBPH_ploc)) {
          /* At nodes do not exists in these phases! */
          $$ = SPdoLocate($$);
        }
        
        COPYLOCTO($$, $2);
      }
    ;

MetadataDef:
      METADATA_BEGIN Constructs METADATA_END
      {

	DBUG_PRINT("SP", ("MetadataDef <metadata> Constructs </metadata>"));

        $$ = $2;
      }
     ;

Constructs:
      Construct Constructs
      {
	DBUG_PRINT("SP", ("Constructs Construct Constructs"));

        $$ = TBmakeMetadatadefs($1, $2);
	NODE_COPYLOCTO($$, $1);
      }
     | Construct
      {
	DBUG_PRINT("SP", ("Constructs Construct"));

        $$ = TBmakeMetadatadefs($1, NULL);
	NODE_COPYLOCTO($$, $1);
      }
     ;

Construct:
      BoxElement
      {
	DBUG_PRINT("SP", ("Construct BoxElement"));

        $$ = $1;
      }
     | NetElement
      {
	DBUG_PRINT("SP", ("Construct NetElement"));

        $$ = $1;
      }
     | DefaultElement 
      {
	DBUG_PRINT("SP", ("Construct DefaultElement"));

        $$ = $1;
      }
     ;

BoxElement:
     METADATA_BOX_BEGIN NAME EQUAL_SIGN DOUBLE_QUOTE MD_VALUE DOUBLE_QUOTE METADATA_TAG_END Keys METADATA_BOX_END
      {

       DBUG_PRINT("SP", ("BoxElement <box name=\"MDVALUE\" > Keys </box >"));

       $$ = TBmakeMetadataboxdef($5, $8);
       COPYLOCTO($$, $1);
      }
     ;

NetElement:
      METADATA_NET_BEGIN NAME EQUAL_SIGN DOUBLE_QUOTE MD_VALUE DOUBLE_QUOTE METADATA_TAG_END NetChilds METADATA_NET_END
      {
       DBUG_PRINT("SP", ("NetElement <net name=\"MDVALUE\" > NetChilds </net >"));

       $$ = TBmakeMetadatanetdef($5, NULL, NULL, $8);
       COPYLOCTO($$, $1);
      }
     ;

NetChilds:
      Keys
      {
	DBUG_PRINT("SP", ("NetChilds Keys"));

	$$ = TBmakeMetadatalist($1, NULL);
	NODE_COPYLOCTO($$, $1);
      }
    | Keys Construct NetChilds
      {
	DBUG_PRINT("SP", ("NetChilds Keys Construct NetChilds"));

	$$ = TBmakeMetadatalist($1, TBmakeMetadatalist($2, $3));
	NODE_COPYLOCTO($$, $1);
      }     
    | Construct NetChilds
      {	
        DBUG_PRINT("SP", ("NetChilds Construct NetChilds"));

	$$ = TBmakeMetadatalist($1, $2);
	NODE_COPYLOCTO($$, $1);
      }
    | /* EMPTY */
      {
        DBUG_PRINT("SP", ("NetChilds 'empty'"));
	$$ = NULL;
      }      
    ;

DefaultElement:
      METADATA_DEFAULT_BEGIN Keys METADATA_DEFAULT_END     
      {
        DBUG_PRINT("SP", ("DefaultElement <default > Keys </default >"));

        $$ = TBmakeMetadatadefaultdef(MD_all, $2);
	COPYLOCTO($$, $1);
      }
     | METADATA_BOXDEFAULT_BEGIN Keys METADATA_BOXDEFAULT_END   
      {
        DBUG_PRINT("SP", ("DefaultElement <boxdefault > Keys </boxdefault >"));

        $$ = TBmakeMetadatadefaultdef(MD_box, $2);
	COPYLOCTO($$, $1);
      }
     | METADATA_NETDEFAULT_BEGIN Keys METADATA_NETDEFAULT_END  
      {
        DBUG_PRINT("SP", ("DefaultElement <netdefault > Keys </netdefault >"));

        $$ = TBmakeMetadatadefaultdef(MD_net, $2);
	COPYLOCTO($$, $1);
      }
      ;

Keys:
      Key Keys
      {
        DBUG_PRINT("SP", ("Keys Key Keys"));

        METADATAKEYLIST_NEXT($1) = $2;

        $$ = $1;
      }
     | Key
      {
        DBUG_PRINT("SP", ("Keys Key"));

        $$ = $1;
      }

     ;      

Key:
     METADATA_TAG_START IDENTIFIER METADATA_TAG_END MD_VALUE METADATA_ENDTAG_START IDENTIFIER METADATA_TAG_END
      {
        DBUG_PRINT("SP", ("Key < IDENTIFIER > MD_VALUE </ IDEINTIFIER >"));

	if(!STReq($2, $6)) {
	  yyerror("Metadata tags do not match"); 
	}

	MEMfree($6);

        $$ = TBmakeMetadatakeylist($2, $4, NULL);
	COPYLOCTO($$, $1);
      }
     | METADATA_TAG_START IDENTIFIER VALUE EQUAL_SIGN DOUBLE_QUOTE MD_VALUE DOUBLE_QUOTE METADATA_TAG_SHORT_END
      {
        DBUG_PRINT("SP", ("Key < IDENTIFIER VALUE=\"MD_VALUE\" />"));

        $$ = TBmakeMetadatakeylist($2, $6, NULL);
	COPYLOCTO($$, $1);
      }
     | METADATA_TAG_START IDENTIFIER METADATA_TAG_END METADATA_ENDTAG_START IDENTIFIER METADATA_TAG_END
      {
        DBUG_PRINT("SP", ("Key < IDENTIFIER > </ IDEINTIFIER >"));

	if(!STReq($2, $5)) {
	  yyerror("Metadata tags do not match"); 
	}

	MEMfree($5);

        $$ = TBmakeMetadatakeylist($2, STRcpy(""), NULL);
	COPYLOCTO($$, $1);
      }
     | METADATA_TAG_START IDENTIFIER VALUE EQUAL_SIGN DOUBLE_QUOTE DOUBLE_QUOTE METADATA_TAG_SHORT_END
      {
        DBUG_PRINT("SP", ("Key < IDENTIFIER VALUE=\"\" />"));

        $$ = TBmakeMetadatakeylist($2, STRcpy(""), NULL);
	COPYLOCTO($$, $1);
      }
     | METADATA_TAG_START IDENTIFIER METADATA_TAG_SHORT_END
      {
        DBUG_PRINT("SP", ("Key < IDENTIFIER />"));

        $$ = TBmakeMetadatakeylist($2, STRcpy(""), NULL);
	COPYLOCTO($$, $1);
      }
      ;

%%

int yyerror(char *error) 
{
  CTIabort(CTI_ERRNO_PARSING_ERROR,
	   "*** Error parsing source code. %s in line %d, column %d\n",  
           error, linenum, charpos);
  return 0;
}

static void pkgNamesToDefs()
{
  node *n = MODULE_FIELDS(parseResult);
  while(n != NULL) {
    char *pkg = FIELDS_PKGNAME(n);
    if(pkg != NULL) {
      char *key = STRcat(pkg, "::");
      void **p = netLUTsearchIgnoreBoxes(key);
      if(p == NULL) {
        YYparseError("Cannot resolve package name %s to definition", key);
      }
      MEMfree(key);
      FIELDS_PKG(n) = *p;
    }
    n = FIELDS_NEXT(n);
  }

  n = MODULE_STAGS(parseResult);
  while(n != NULL) {
    char *pkg = STAGS_PKGNAME(n);
    if(pkg != NULL) {
      char *key = STRcat(pkg, "::");
      void **p = netLUTsearchIgnoreBoxes(key);
      if(p == NULL) {
        YYparseError("Cannot resolve package name %s to definition", pkg);
      }
      MEMfree(key);
      STAGS_PKG(n) = *p;
    }
    n = STAGS_NEXT(n);
  }

  n = MODULE_BTAGS(parseResult);
  while(n != NULL) {
    char *pkg = BTAGS_PKGNAME(n);
    if(pkg != NULL) {
      char *key = STRcat(pkg, "::");
      void **p = netLUTsearchIgnoreBoxes(key);
      if(p == NULL) {
        YYparseError("Cannot resolve package name %s to definition", pkg);
      }
      MEMfree(key);
      BTAGS_PKG(n) = *p;
    }
    n = BTAGS_NEXT(n);
  }
}

node *YYparseTree()
{
  init();

  parseResult = TBmakeModule(NULL, NULL, NULL, NULL, NULL); 

  yyparse();

  pkgNamesToDefs();

  terminate();

  return parseResult;
}

