#include "codegen.h"
#include <string.h>
#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "lookup_table.h"
#include "str.h"
#include "print.h"

#include "cchoice.h"
#include "ctext.h"
#include "cfeedback.h"
#include "cfilt.h"
#include "csplit.h"
#include "cserial.h"
#include "csync.h"
#include "cwrap.h"
#include "cgenwrap.h"
#include "ctrans.h"
#include "cshield.h"
#include "cstar.h"
#include "cexpr.h"
#include "netid.h"

#include "mutil.h"

typedef enum{CTunknown, CTboxInclude, CTinterfcount,
	     CTinterfinclude, CTinterfinit} CodeType;

/* Look up table for checking if box-code is already generated
   (there might exist multiple boxes with equal name) */
static lut_t *boxLUT = NULL;

/*
 * INFO structure
 */
struct INFO {
  CodeType codeType;
  node *def;
  int num_interfaces;
};


#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_DEF(n)          (n->def)
#define INFO_NUM_INTERFACES(n)     (n->num_interfaces)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTunknown;
  INFO_DEF(inf)      = NULL;
  INFO_NUM_INTERFACES(inf) = 0;
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

static unsigned int guardpatternSize(node *patterns)
{
  unsigned int size = 0;

  while(patterns != NULL) {
    size++;
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return size;
}

static unsigned int guardpatternConditionCount(node *patterns)
{
  unsigned int size = 0;

  while(patterns != NULL) {
    if(GUARDPATTERNS_CONDITION(patterns) != NULL) {
      size++;
    }
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return size;
}

static bool hasObservers(node *md)
{
  const char *value;

  if(md != NULL) {
    value = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER);

    if(value != NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

static bool writeStart(node *net)
{
  bool hasObs = hasObservers(NETDEF_METADATA(net));

  if(hasObs == TRUE) {
    CODEFILEwriteObservedNetStart(NETDEF_NAME(net));
  }
  else if (NETDEF_TOPLEVEL(net)) {
    CODEFILEwriteTopLevelNetStart(NETDEF_NAME(net));
  }
  else{
    CODEFILEwriteNetStart(NETDEF_NAME(net));
  }

  return hasObs;
}

static void writeObservers(node *net, node *md, const char *name, int location)
{
  const char *type = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_TYPE);
  const char *file = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_FILE);
  const char *data = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_DATA_LEVEL);
  const char *address = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_ADDRESS);
  const char *code = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_CODE);
  const char *interactive = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_INTERACTIVE);
  const char *port = MDUtilMetadataGetKey(md, METADATA_KEY_OBSERVER_PORT);

  if(STReq(type, METADATA_VALUE_OBSERVER_TYPE_BEFORE)) {

    if(file == NULL) {
      CODEFILEwriteSocketObserverBefore(NETDEF_NAME(net), name, interactive,
					data, code, address, port);
    } else {
      CODEFILEwriteFileObserverBefore(NETDEF_NAME(net), name,
				      data, code, file);
    }

    if(NETDEF_TOPLEVEL(net)) {
      CODEFILEwriteTopLevelNetStart(NETDEF_NAME(net));
    }
    else{
      CODEFILEwriteNetStart(NETDEF_NAME(net));
    }

    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();
    CODEFILEwriteSnetSerialStart();
    CODEFILEwriteLocation(location);
    CODEFILEwriteNext();
    CODEFILEwriteObserverBeforeSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteNext();
    CODEFILEwriteObserverSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteFunctionFullStop();
    CODEFILEwriteOutBufReturn();
    CODEFILEwriteScopeStop();

  } else if(STReq(type, METADATA_VALUE_OBSERVER_TYPE_AFTER)) {
    /* AFTER */

    if(file == NULL) {
      CODEFILEwriteSocketObserverAfter(NETDEF_NAME(net), name, interactive,
					data, code, address, port);
    }
    else {
      CODEFILEwriteFileObserverAfter(NETDEF_NAME(net), name,
				     data, code, file);
    }

    if(NETDEF_TOPLEVEL(net)) {
      CODEFILEwriteTopLevelNetStart(NETDEF_NAME(net));
    }
    else{
      CODEFILEwriteNetStart(NETDEF_NAME(net));
    }

    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();
    CODEFILEwriteSnetSerialStart();
    CODEFILEwriteLocation(location);
    CODEFILEwriteNext();
    CODEFILEwriteObserverSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteNext();
    CODEFILEwriteObserverAfterSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteFunctionFullStop();
    CODEFILEwriteOutBufReturn();
    CODEFILEwriteScopeStop();
  } else { /* METADATA_VALUE_OBSERVER_TYPE_BOTH*/

    if(file == NULL) {
      CODEFILEwriteSocketObserverBefore(NETDEF_NAME(net), name, interactive,
					data, code, address, port);

      CODEFILEwriteSocketObserverAfter(NETDEF_NAME(net), name, interactive,
				      data, code, address, port);

    } else {
      CODEFILEwriteFileObserverBefore(NETDEF_NAME(net), name,
				      data, code, file);

      CODEFILEwriteFileObserverAfter(NETDEF_NAME(net), name,
				     data, code, file);

    }

    CODEFILEwriteObserverAuxStart(NETDEF_NAME(net));
    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();
    CODEFILEwriteSnetSerialStart();
    CODEFILEwriteLocation(location);
    CODEFILEwriteNext();
    CODEFILEwriteObserverBeforeSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteNext();
    CODEFILEwriteObserverSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteFunctionFullStop();
    CODEFILEwriteOutBufReturn();
    CODEFILEwriteScopeStop();

    if(NETDEF_TOPLEVEL(net)) {
      CODEFILEwriteTopLevelNetStart(NETDEF_NAME(net));
    }
    else{
      CODEFILEwriteNetStart(NETDEF_NAME(net));
    }

    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();
    CODEFILEwriteSnetSerialStart();
    CODEFILEwriteLocation(location);
    CODEFILEwriteNext();
    CODEFILEwriteObserverAuxSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteNext();
    CODEFILEwriteObserverAfterSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    CODEFILEwriteFunctionFullStop();
    CODEFILEwriteOutBufReturn();
    CODEFILEwriteScopeStop();
  }

}

node *CODEboxdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEboxdef");

  if(INFO_CODETYPE(arg_info) == CTboxInclude) {
    if(LUTsearchInLutS(boxLUT, BOXDEF_REALNAME(arg_node)) == NULL) {
      LUTinsertIntoLutS(boxLUT, BOXDEF_REALNAME(arg_node), arg_node);
    }
  }

  DBUG_RETURN(arg_node);
}

node *CODElanguageinterfaces(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODElanguageinterface");

  switch(INFO_CODETYPE(arg_info)) {
  case CTinterfcount:

    if(INFO_NUM_INTERFACES(arg_info) != 0) {
      CODEFILEwriteNext();
    }

    CODEFILEwriteInterfaceDefine(LANGUAGEINTERFACES_NAME(arg_node));
    CODEFILEwriteInterface(LANGUAGEINTERFACES_NAME(arg_node));

    INFO_NUM_INTERFACES(arg_info)++;
    break;
  case CTinterfinclude:
    CODEFILEwriteSrcInclude(LANGUAGEINTERFACES_NAME(arg_node), "h");
    break;
  case CTinterfinit:
    CODEFILEwriteMainInterfaceInit(LANGUAGEINTERFACES_NAME(arg_node));
    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *CODEboxref(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEboxref");

  net = INFO_DEF(arg_info);

  hasObs = hasObservers(BOXDEF_METADATA(BOXREF_BOX(arg_node)));

  if(hasObs == TRUE) {
    CODEFILEwriteObservedNetStart(NETDEF_NAME(net));
  }else{
    if(NETDEF_TOPLEVEL(net)) {
      CODEFILEwriteTopLevelNetStart(NETDEF_NAME(net));
    }
    else{
      CODEFILEwriteNetStart(NETDEF_NAME(net));
    }
  }

  CWRAPdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = BOXDEF_METADATA(BOXREF_BOX(arg_node));
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, BOXREF_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *CODEbtags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEbtags");

  pkg = BTAGS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteBtagLabel(NETDEF_PKGNAME(pkg), BTAGS_NAME(arg_node));
    CODEFILEwriteBtagDefine(NETDEF_PKGNAME(pkg), BTAGS_NAME(arg_node));
  }
  else {
    CODEFILEwriteBtagLabel(NULL, BTAGS_NAME(arg_node));
    CODEFILEwriteBtagDefine(NULL, BTAGS_NAME(arg_node));
  }


  if(BTAGS_NEXT(arg_node) != NULL) {
    TRAVdo(BTAGS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CODEchoice(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEchoice");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CCHOICEdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, CHOICE_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEdefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEdefs");

  if(DEFS_DEF(arg_node) != NULL) {
    if(INFO_CODETYPE(arg_info) != CTboxInclude) {
      infoClear(arg_info);
    }
    TRAVdo(DEFS_DEF(arg_node), arg_info);
  }

  if(DEFS_NEXT(arg_node) != NULL) {
    TRAVdo(DEFS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CODEfields(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEfields");

  pkg = FIELDS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteFieldDefine(NETDEF_PKGNAME(pkg), FIELDS_NAME(arg_node));
    CODEFILEwriteFieldLabel(NETDEF_PKGNAME(pkg), FIELDS_NAME(arg_node));
  }
  else {
    CODEFILEwriteFieldDefine(NULL, FIELDS_NAME(arg_node));
    CODEFILEwriteFieldLabel(NULL, FIELDS_NAME(arg_node));
  }


  if(FIELDS_NEXT(arg_node) != NULL) {
    TRAVdo(FIELDS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CODEfilt(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEfilt");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CFILTdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, FILT_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEmodule(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEmodule");

  if(!CODEFILEopenFiles(global.filebase)) {
    CTIabort(CTI_ERRNO_FILE_ACCESS_ERROR,
	     "*** Error: Cannot open generated files for writing ***\n");
  }

  CODEFILEwriteSrcInclude("type", "h");

  if(MODULE_INTERFACES(arg_node) != NULL) {
    TRAVdo(MODULE_INTERFACES(arg_node), arg_info);
  }

  CODEFILEwriteSrcInclude("networkinterface", "h");

  if(MODULE_HASOBSERVERS(arg_node) == TRUE) {
    CODEFILEwriteSrcInclude("observers", "h");
  }

  INFO_CODETYPE(arg_info) = CTinterfinclude;

  if(MODULE_INTERFACES(arg_node) != NULL) {
    TRAVdo(MODULE_INTERFACES(arg_node), arg_info);
  }

  INFO_CODETYPE(arg_info) = CTunknown;


  CODEFILEwriteSrcInclude("distribution", "h");
  CODEFILEwriteSrcInclude("list", "h");

  if(MODULE_DEFS(arg_node) != NULL) {

    INFO_CODETYPE(arg_info) = CTboxInclude;
    TRAVdo(MODULE_DEFS(arg_node), arg_info);
    LUTremoveContentLut(boxLUT);
    INFO_CODETYPE(arg_info) = CTunknown;
    infoClear(arg_info);
  }

  /* Print define directives for label numbers */
  CODEFILEwriteNoneDefine();
  CODEFILEwriteSnetLabelsStart();

  if(MODULE_FIELDS(arg_node) != NULL) {
    TRAVdo(MODULE_FIELDS(arg_node), arg_info);
  }
  if(MODULE_STAGS(arg_node) != NULL) {
    TRAVdo(MODULE_STAGS(arg_node), arg_info);
  }
  if(MODULE_BTAGS(arg_node) != NULL) {
    TRAVdo(MODULE_BTAGS(arg_node), arg_info);
  }
  CODEFILEwriteSnetLabelsEnd();

  CODEFILEwriteNumberOfLabels();

  /* Print define directives for interface numbers */
  INFO_CODETYPE(arg_info) = CTinterfcount;

  CODEFILEwriteSnetInterfacesStart();

  if(MODULE_INTERFACES(arg_node) != NULL) {
    TRAVdo(MODULE_INTERFACES(arg_node), arg_info);
  }

  CODEFILEwriteSnetInterfacesEnd();

  CODEFILEwriteNumberOfInterfaces();

  if(MODULE_DEFS(arg_node) != NULL) {
    CTEXTdoCode(MODULE_DEFS(arg_node));
  }

  CODEFILEwriteSnetLabelsDefinition();

  CODEFILEwriteSnetInterfacesDefinition();

  CODEFILEwriteTopLevelNetDeclaration(global.filebase);

  if(MODULE_DEFS(arg_node) != NULL) {
    infoClear(arg_info);

    TRAVdo(MODULE_DEFS(arg_node), arg_info);

    LUTremoveContentLut(boxLUT);
  }

  NETIDdoLookup(arg_node);

  CODEFILEwriteMainStart();

  INFO_CODETYPE(arg_info) = CTinterfinit;

  if(MODULE_INTERFACES(arg_node) != NULL) {
    TRAVdo(MODULE_INTERFACES(arg_node), arg_info);
  }

  CODEFILEwriteMainEnd();

  INFO_CODETYPE(arg_info) = CTunknown;

  CODEFILEcloseFiles();

  DBUG_RETURN(arg_node);
}


node *CODEnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEnetdef");

  if(INFO_CODETYPE(arg_info) == CTunknown) {
    if(NETDEF_BODY(arg_node) != NULL) {
      INFO_DEF(arg_info) = arg_node;
      TRAVdo(NETDEF_BODY(arg_node), arg_info);

      if ((!NETDEF_EXTERNAL(arg_node)
	            && NETDEF_SIGNED(arg_node) && !NETDEF_NOSHIELDS(arg_node)) 
          || NETDEF_TOPLEVEL(arg_node)){
        CSHIELDdoCode(arg_node);	
      }
    }
  }else{
    if(NETDEF_EXTERNAL(arg_node)) {
      CODEFILEwriteHdrInclude(NETDEF_NAME(arg_node), "h");

    }
  }

  DBUG_RETURN(arg_node);
}

node *CODEnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;
  node *def = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEnetrefs");

  net = NETREFS_NET(arg_node);

  def = INFO_DEF(arg_info);
  
  if (!((NETDEF_SIGNED(def) && !NETDEF_NOSHIELDS(def)) 
       || NETDEF_TOPLEVEL(def))) {
    hasObs = writeStart(def);
    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();
    CODEFILEwriteSnetAlias(NETDEF_PKGNAME(net), NETDEF_NAME(net), NETREFS_LOCATION(arg_node));
    CODEFILEwriteOutBufReturn();
    CODEFILEwriteScopeStop();

    if(hasObs == TRUE) {
      md = NETDEF_METADATA(def);
      name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
      writeObservers(def, md, name, NETREFS_LOCATION(arg_node));
    }

  }
  DBUG_RETURN(arg_node);
}

node *CODEserial(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEserial");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CSERIALdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, SERIAL_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEsplit(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEsplit");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CSPLITdoCode(arg_node);

  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, SPLIT_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEstags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEstags");

  pkg = STAGS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteStagDefine(NETDEF_PKGNAME(pkg), STAGS_NAME(arg_node));
    CODEFILEwriteStagLabel(NETDEF_PKGNAME(pkg), STAGS_NAME(arg_node));
  }
  else {
    CODEFILEwriteStagDefine(NULL, STAGS_NAME(arg_node));
    CODEFILEwriteStagLabel(NULL, STAGS_NAME(arg_node));
  }

  if(STAGS_NEXT(arg_node) != NULL) {
    TRAVdo(STAGS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CODEstar(node *arg_node, info *arg_info)
{
  int i;
  node *net = NULL;
  unsigned int guardNum;
  unsigned int condNum;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEstar");

  net = INFO_DEF(arg_info);

  guardNum = guardpatternSize(STAR_TERM(arg_node));
  condNum = guardpatternConditionCount(STAR_TERM(arg_node));

  CODEFILEwriteNetStarIncStart(NETDEF_NAME(net));
  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();

  if(STAR_ISDETERM(arg_node)) {
    CODEFILEwriteSnetStarDetIncStart();
  } else {
    CODEFILEwriteSnetStarIncStart();
  }

  CODEFILEwriteLocation(STAR_LOCATION(arg_node));

  CODEFILEwriteNext();
  CODEFILEwriteVariantListStart(guardNum);

  CSTARdoCode(STAR_TERM(arg_node));

  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();

  CODEFILEwriteExprListStart(guardNum);
  if(STAR_TERM(arg_node) != NULL && condNum > 0) {
    CEXPRdoCode(STAR_TERM(arg_node));
  }

  for (i = condNum; i < guardNum; i++) {
    CODEFILEwriteNext();
    CODEFILEwriteSnetEconstbTrue();
  }

  CODEFILEwriteFunctionStop();

  if(STAR_LEFT(arg_node) != NULL) {
    CSTARdoCode(STAR_LEFT(arg_node));
  }

  CODEFILEwriteNext();
  CODEFILEwriteSnetStarIncEntity(NETDEF_NAME(net));
  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();
  CODEFILEwriteScopeStop();

  hasObs = writeStart(net);

  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();

  if(STAR_ISDETERM(arg_node)) {
    CODEFILEwriteSnetStarDetStart();
  } else {
    CODEFILEwriteSnetStarStart();
  }
  CODEFILEwriteLocation(STAR_LOCATION(arg_node));

  CODEFILEwriteNext();
  CODEFILEwriteVariantListStart(guardNum);

  CSTARdoCode(STAR_TERM(arg_node));

  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();

  CODEFILEwriteExprListStart(guardNum);
  if(STAR_TERM(arg_node) != NULL && condNum > 0) {
    CEXPRdoCode(arg_node);
  }

  for (i = condNum; i < guardNum; i++) {
    CODEFILEwriteNext();
    CODEFILEwriteSnetEconstbTrue();
  }

  CODEFILEwriteFunctionStop();

  if(STAR_LEFT(arg_node) != NULL) {
    CSTARdoCode(STAR_LEFT(arg_node));
  }

  CODEFILEwriteNext();
  CODEFILEwriteSnetStarIncEntity(NETDEF_NAME(net));
  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, STAR_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}



node *CODEfeedback(node *arg_node, info *arg_info)
{
  int i;
  node *net = NULL;
  unsigned int guardNum;
  unsigned int condNum;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEfeedback");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  guardNum = guardpatternSize(FEEDBACK_BACK(arg_node));
  condNum = guardpatternConditionCount(FEEDBACK_BACK(arg_node));

  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();

  CODEFILEwriteSnetFeedbackStart();

  CODEFILEwriteLocation(FEEDBACK_LOCATION(arg_node));

  CODEFILEwriteNext();
  CODEFILEwriteVariantListStart(guardNum);

  CFEEDBACKdoCode(FEEDBACK_BACK(arg_node));

  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();


  CODEFILEwriteExprListStart(guardNum);
  if(FEEDBACK_BACK(arg_node) != NULL && condNum > 0) {
    CEXPRdoCode(FEEDBACK_BACK(arg_node));
  }

  for (i = condNum; i < guardNum; i++) {
    CODEFILEwriteNext();
    CODEFILEwriteSnetEconstbTrue();
  }

  CODEFILEwriteFunctionStop();

  if(FEEDBACK_LEFT(arg_node) != NULL) {
    CFEEDBACKdoCode(FEEDBACK_LEFT(arg_node));
  }

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, FEEDBACK_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}



node *CODEsync(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEsync");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CSYNCdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, SYNC_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEtrans(node *arg_node, info *arg_info)
{
  node *net = NULL;
  bool hasObs;
  node *md = NULL;
  const char *name = NULL;

  DBUG_ENTER("CODEtrans");

  net = INFO_DEF(arg_info);

  hasObs = writeStart(net);

  CTRANSdoCode(arg_node);
  CODEFILEwriteScopeStop();

  if(hasObs == TRUE) {
    md = NETDEF_METADATA(net);
    name = MDUtilMetadataGetKey(md, METADATA_KEY_SNET_ORIGINAL_NAME);
    writeObservers(net, md, name, TRANS_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *CODEdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CODEdoCode");

  DBUG_ASSERT((syntax_tree != NULL), "CODEdoCode called with empty syntaxtree");

  boxLUT = LUTgenerateLut();

  inf = infoMake();

  TRAVpush(TR_code);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  LUTremoveLut(boxLUT);

  DBUG_RETURN(syntax_tree);
}

