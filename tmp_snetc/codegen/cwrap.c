#include <string.h>
#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "mutil.h"

#include "cwrap.h"

/*
 * INFO structure
 */

struct INFO {  
  filttype filter;
  bool do_map_vector;
};

#define INFO_FILTER(n)        (n->filter)
#define INFO_DO_MAP_VECTOR(n) (n->do_map_vector)

static void infoClear(info *inf)
{
  INFO_FILTER(inf)   = FILT_empty;
  INFO_DO_MAP_VECTOR( inf) = TRUE;
}

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  infoClear(result);

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

static unsigned int boxOutSize(node *boxsign)
{
  unsigned int size = 0;
  node *outtypes = BOXSIGN_OUTTYPES(boxsign);
  
  while (outtypes != NULL) {
    size++;
    outtypes = BOXTYPES_NEXT(outtypes);
  }
  
  return size;
}

static unsigned int fieldCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_FIELD(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

static unsigned int stagCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_STAG(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

static unsigned int btagCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_BTAG(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

node *CWRAPboxref(node *arg_node, info *arg_info)
{
  node *box = NULL;
  node *md = NULL;
  const char *boxname = NULL;

  DBUG_ENTER("CWRAPboxref");
 
  box = BOXREF_BOX(arg_node);
  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();
  
  /* metadata element 'snet_originalname' is present as it is
   * inserted automatically by the compiler 
   */
  DBUG_ASSERT( BOXDEF_METADATA( box) != NULL,
               "Found box without any metadata annotation");
  md = BOXDEF_METADATA( box);

  boxname = MDUtilMetadataGetKey( md, "boxfun");
  if( boxname == NULL) {
    boxname = MDUtilMetadataGetKey( md, "snet_originalname");
  }
  
  DBUG_ASSERT( boxname != NULL, "Metadata value 'snet_originalname' "
                                "not set");

  CODEFILEwriteSnetBoxStart();
  CODEFILEwriteLocation(BOXREF_LOCATION(arg_node));
  CODEFILEwriteNext();
  CODEFILEwriteBoxname( boxname);
  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(box));
  CODEFILEwriteNext();
  CODEFILEwriteRealmCreate(boxname);
  CODEFILEwriteNext();
  CODEFILEwriteRealmUpdate(boxname);
  CODEFILEwriteNext();
  CODEFILEwriteRealmDestroy(boxname);
  CODEFILEwriteNext();
  CODEFILEwriteIntListListStart(boxOutSize(BOXDEF_SIGN(box)));
  TRAVdo(BOXDEF_SIGN(box), arg_info);
  CODEFILEwriteFunctionStop();
  CODEFILEwriteFunctionFullStop();

  CODEFILEwriteOutBufReturn();
  
  DBUG_RETURN(arg_node);
}

node *CWRAPboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CWRAPboxsign");

  if(BOXSIGN_OUTTYPES(arg_node) != NULL) {
    TRAVdo(BOXSIGN_OUTTYPES(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CWRAPboxtypes(node *arg_node, info *arg_info)
{
  unsigned int fieldNum;
  unsigned int stagNum;
  unsigned int btagNum ;

  DBUG_ENTER("CWRAPboxtypes");
 
  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    fieldNum = fieldCount(BOXTYPES_ENTRIES(arg_node));
    stagNum  = stagCount(BOXTYPES_ENTRIES(arg_node));
    btagNum  = btagCount(BOXTYPES_ENTRIES(arg_node));
    
    if( INFO_DO_MAP_VECTOR( arg_info)) {
      CODEFILEwriteNext();
      CODEFILEwriteIntListStart( 2*(fieldNum + stagNum + btagNum));
      TRAVdo( BOXTYPES_ENTRIES( arg_node), arg_info);    
      CODEFILEwriteFunctionStop();
    } else {
      CODEFILEwriteNext();
      CODEFILEwriteVariantStart();
      INFO_FILTER(arg_info) = FILT_field;
      CODEFILEwriteIntListStart(fieldNum);
      TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
      CODEFILEwriteFunctionStop();
    
      INFO_FILTER(arg_info) = FILT_stag;
      CODEFILEwriteNext();
      CODEFILEwriteIntListStart(stagNum);
      TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
      CODEFILEwriteFunctionStop();
    
      INFO_FILTER(arg_info) = FILT_btag;
      CODEFILEwriteNext();
      CODEFILEwriteIntListStart(btagNum);
      TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
      CODEFILEwriteFunctionStop();
      CODEFILEwriteFunctionStop();
    }
  } else {
    CODEFILEwriteNext();
    // CODEFILEwriteEmptyVariant();
    CODEFILEwriteIntListStart(0);
    CODEFILEwriteFunctionStop();
  }
  
  if(BOXTYPES_NEXT(arg_node) != NULL) {
    TRAVdo(BOXTYPES_NEXT(arg_node), arg_info);
  }
 
  DBUG_RETURN(arg_node);
}

node *CWRAPbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CWRAPbtagref");
  
  btag = BTAGREF_BTAG(arg_node);
  pkg = BTAGS_PKG(btag);

  if( INFO_DO_MAP_VECTOR( arg_info)) {
    CODEFILEwriteNext();
    CODEFILEwriteBoxSignBTag();
  }
     if(pkg == NULL) {
       CODEFILEwriteNext();
       CODEFILEwriteBtag(NULL, BTAGS_NAME(btag));
     }
      else {
        CODEFILEwriteNext();
        CODEFILEwriteBtag(NETDEF_PKGNAME(pkg), BTAGS_NAME(btag));
      }
  DBUG_RETURN(arg_node);
}

node *CWRAPfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CWRAPfieldref");
  
  field = FIELDREF_FIELD(arg_node);
  pkg = FIELDS_PKG(field);

  if( INFO_DO_MAP_VECTOR( arg_info)) {
    CODEFILEwriteNext();
    CODEFILEwriteBoxSignField();
  }
      if(pkg == NULL) {
       CODEFILEwriteNext();
       CODEFILEwriteField(NULL, FIELDS_NAME(field));
     }
     else {
       CODEFILEwriteNext();
       CODEFILEwriteField(NETDEF_NAME(pkg), FIELDS_NAME(field));
     }
  DBUG_RETURN(arg_node);
}

node *CWRAPstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CWRAPstagref");
  
  stag = STAGREF_STAG(arg_node);
  pkg = STAGS_PKG(stag);
    
  if( INFO_DO_MAP_VECTOR( arg_info)) {
    CODEFILEwriteNext();
    CODEFILEwriteBoxSignTag();
  }
     if(pkg == NULL) {
       CODEFILEwriteNext();
       CODEFILEwriteStag(NULL, STAGS_NAME(stag));
      }
      else {
        CODEFILEwriteNext();
        CODEFILEwriteStag(NETDEF_PKGNAME(pkg), STAGS_NAME(stag));
     }

  DBUG_RETURN(arg_node);
}

node *CWRAPdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CWRAPdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CWRAPdoCode called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_cwrap);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

