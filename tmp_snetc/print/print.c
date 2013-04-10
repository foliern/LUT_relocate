/*******************************************************************************
 *
 * $Id: print.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.01.2007
 * -----
 *
 *******************************************************************************/

#include <string.h>
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "memory.h"
#include "ctinfo.h"

static FILE *file = NULL;

struct INFO {
  bool noElse;
  enum {PRT_TM_NONE, PRT_TM_LHS, PRT_TM_RHS} side;
  int indentLevel;
  bool printmd;
  bool top_level;
  bool isdeterm;
  bool is_first_metadatadef;
};

#define INFO_NOELSE(arg_info) (arg_info->noElse)
#define INFO_SIDE(arg_info) (arg_info->side)
#define INFO_INDENTLEVEL(arg_info) (arg_info->indentLevel)
#define INFO_PRINTMD(arg_info) (arg_info->printmd)
#define INFO_TOPLEVEL(arg_info) (arg_info->top_level)
#define INFO_ISDETERM(arg_info) (arg_info->isdeterm)
#define INFO_IS_FIRST_METADATADEF(arg_info) (arg_info->is_first_metadatadef)

static info *infoMake(void)
{
  info *i = NULL;

  DBUG_ENTER("infoMake");

  i = MEMmalloc(sizeof(info));
  INFO_NOELSE(i) = FALSE;
  INFO_SIDE(i) = PRT_TM_NONE;
  INFO_INDENTLEVEL(i) = 0;
  INFO_PRINTMD(i) = TRUE;
  INFO_TOPLEVEL(i) = FALSE;
  INFO_ISDETERM(i) = FALSE;
  INFO_IS_FIRST_METADATADEF(i) = TRUE;

  DBUG_RETURN(i);
}

static info *infoFree(info *arg_info)
{
  info *i = NULL;

  DBUG_ENTER("infoFree");

  i = MEMfree(arg_info);

  DBUG_RETURN(i);
}


#define INDENT_SIZE 2

static void indent(info *arg_info)
{
  int i;
  int indent_level = INFO_INDENTLEVEL(arg_info);

  for(i = 0; i < (indent_level*INDENT_SIZE); ++i) {
    fprintf(file," ");
  }
}

node *PRTboxbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTboxbody");

  if(BOXBODY_LANGNAME(arg_node) == NULL) {
    /* Empty body */
    fprintf(file,";\n");
  }
  else {
    fprintf(file," {\n");
    indent(arg_info);
    fprintf(file,"<<< %s |", BOXBODY_LANGNAME(arg_node));
    fprintf(file,"%s", BOXBODY_CODE(arg_node));
    fprintf(file,">>>\n");
    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    INFO_INDENTLEVEL(arg_info)++;
    fprintf(file,"}\n");
  }

  DBUG_RETURN(arg_node);
}

node *PRTboxdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTboxdef");

  if(BOXDEF_METADATA(arg_node) != NULL) {

    fprintf(file,"\n");
    indent(arg_info);
    fprintf(file,"<metadata>\n");
    INFO_INDENTLEVEL(arg_info)++;
    indent(arg_info);
    if(BOXDEF_NAME(arg_node) != NULL) {
      fprintf(file,"<box name=\"%s\">\n", BOXDEF_NAME(arg_node));
    }

    INFO_INDENTLEVEL(arg_info)++;

    TRAVdo(BOXDEF_METADATA(arg_node), arg_info);

    INFO_INDENTLEVEL(arg_info)--;

    indent(arg_info);
    fprintf(file,"</box>\n");
    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    fprintf(file,"</metadata>\n");
  }

  fprintf(file,"\n");
  indent(arg_info);
  fprintf(file,"box %s ", BOXDEF_NAME(arg_node));

  if(global.compiler_phase >= PH_postproc
     && BOXDEF_REALNAME(arg_node) != NULL) {

    fprintf(file,"%s ", BOXDEF_REALNAME(arg_node));
  }

  fprintf(file,"(");
  TRAVdo(BOXDEF_SIGN(arg_node), arg_info);
  fprintf(file,")");

  INFO_INDENTLEVEL(arg_info)++;
  TRAVdo(BOXDEF_BODY(arg_node), arg_info);
  INFO_INDENTLEVEL(arg_info)--;

  DBUG_RETURN(arg_node);
}

node *PRTboxref(node *arg_node, info *arg_info)
{
  node *ptr = NULL;

  DBUG_ENTER("PRTboxref");

  ptr = BOXREF_BOX(arg_node);

  fprintf(file,"%s", BOXDEF_NAME(ptr));

  if(BOXREF_LOCATION(arg_node) >= 0) {
     fprintf(file,"@%d", BOXREF_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *PRTboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTboxsign");

  if (BOXSIGN_INTYPE(arg_node) != NULL) {
    INFO_SIDE(arg_info) = PRT_TM_LHS;
    TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);
  }

  fprintf(file," -> ");

  if (BOXSIGN_OUTTYPES(arg_node) != NULL) {
    INFO_SIDE(arg_info) = PRT_TM_RHS;
    TRAVdo(BOXSIGN_OUTTYPES(arg_node), arg_info);
  }

  INFO_SIDE(arg_info) = PRT_TM_NONE;

  DBUG_RETURN(arg_node);
}

node *PRTboxtypes(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTboxtypes");

  fprintf(file,"(");

  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
  }
  fprintf(file,")");

  if(BOXTYPES_NEXT(arg_node) != NULL) {
    fprintf(file," | ");
    TRAVdo(BOXTYPES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTbtagref(node *arg_node, info *arg_info)
{
  node *ptr = NULL;
  node *pkg = NULL;

  DBUG_ENTER("PRTbtagref");

  ptr = BTAGREF_BTAG(arg_node);

  fprintf(file,"#");

  if(global.compiler_phase >= PH_postproc) {
    pkg = BTAGS_PKG(ptr);

    if(pkg != NULL) {
      fprintf(file,"%s::", NETDEF_PKGNAME(pkg));
    }
  }

  fprintf(file,"%s", BTAGS_NAME(ptr));

  DBUG_RETURN(arg_node);
}

node *PRTbtags(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTbtags");

  DBUG_ASSERT(FALSE, "This function should never be called.");

  DBUG_RETURN(arg_node);
}

node *PRTbtaginit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTbtaginit");

  TRAVdo(BTAGINIT_BTAG(arg_node), arg_info);

  switch(BTAGINIT_TYPE(arg_node)) {

  case FILT_stag:
  case FILT_btag:
    fprintf(file," <- ");
    TRAVdo(BTAGINIT_INIT(arg_node), arg_info);
    break;

  case FILT_int:
    fprintf(file," <- %d", BTAGINIT_NUM(arg_node));
    break;

  default:
    /* Empty */
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PRTchoice(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTchoice");

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,"(");
  }

  INFO_ISDETERM(arg_info) = CHOICE_ISDETERM(arg_node);
  TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,")");
  }

  DBUG_RETURN(arg_node);
}

node *PRTbranchlist(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTbranchlist");

  if (BRANCHLIST_TYPED(arg_node) || BRANCHLIST_ATTRACTS(arg_node) != NULL) {
    fprintf(file, "< ");
    if (BRANCHLIST_ATTRACTS(arg_node) != NULL) {
      TRAVdo(BRANCHLIST_ATTRACTS(arg_node), arg_info);
      fprintf(file, " ");
    }
    fprintf(file, "> ");
  }
  TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);

  if (BRANCHLIST_NEXT(arg_node) != NULL) {
    if(INFO_ISDETERM(arg_info)) {
      fprintf(file," || ");
    }
    else {
      fprintf(file," | ");
    }
    TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTdefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTdefs");

  if(DEFS_DEF(arg_node) != NULL) {
    TRAVdo(DEFS_DEF(arg_node), arg_info);
  }

  if(DEFS_NEXT(arg_node) != NULL) {
    TRAVdo(DEFS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTerror(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTerror");

  fprintf(file,"%s\n", ERROR_MESSAGE(arg_node));

  if(ERROR_NEXT(arg_node) != NULL) {
    TRAVdo(ERROR_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);

}

node *PRTfieldref(node *arg_node, info *arg_info)
{
  node *ptr = NULL;
  node *pkg = NULL;

  DBUG_ENTER("PRTfieldref");

 ptr = FIELDREF_FIELD(arg_node);

  if(global.compiler_phase >= PH_postproc) {
    pkg = FIELDS_PKG(ptr);

    if(pkg != NULL) {
      fprintf(file,"%s::", NETDEF_PKGNAME(pkg));
    }
  }

  fprintf(file,"%s", FIELDS_NAME(ptr));

  DBUG_RETURN(arg_node);
}

node *PRTfields(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfields");

  DBUG_ASSERT(FALSE, "This function should never be called.");

  DBUG_RETURN(arg_node);
}

node *PRTfieldinit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfieldinit");

  TRAVdo(FIELDINIT_FIELD(arg_node), arg_info);

  if(FIELDINIT_INIT(arg_node) != NULL) {
    fprintf(file," <- ");
    TRAVdo(FIELDINIT_INIT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTmodule(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmodule");

  switch(global.compiler_phase) {

  case PH_parse:
    fprintf(file,"//! snet disp\n\n");
    break;
  case PH_preproc:
    switch(global.compiler_subphase) {
    case SUBPH_dcr:
      fprintf(file,"//! snet dcr\n\n");
      break;
    case SUBPH_tcheck:
      fprintf(file,"//! snet tcheck\n\n");
      break;
    case SUBPH_boxex:
      fprintf(file,"//! snet boxex\n\n");
      break;
    case SUBPH_tres:
      fprintf(file,"//! snet tres\n\n");
      break;
    case SUBPH_mod:
      fprintf(file,"//! snet core\n\n");
      break;
    default:
      fprintf(file,"*** Unsupported compilation subphase ***\n");
      break;
    }
    break;
  case PH_norm:
    switch(global.compiler_subphase) {
      case SUBPH_topoflat:
        fprintf(file,"//! snet flat\n\n");
        break;
      default:
        fprintf(file,"*** Unsupported compilation subphase ***\n");
        break;
    }
    break;
  case PH_typesys:
    switch(global.compiler_subphase) {
      case SUBPH_sip:
        fprintf(file,"//! snet siginfd\n\n");
        break;
      case SUBPH_ri:
        fprintf(file,"//! snet routed\n\n");
        break;
      case SUBPH_muli:
        fprintf(file,"//! snet mulinfd\n\n");
        break;
      default:
        fprintf(file,"*** Unsupported compilation subphase ***\n");
        break;
    }
    break;
  case PH_opt:
    fprintf(file,"//! snet opt\n\n");
    break;
  case PH_postproc:
    switch(global.compiler_subphase) {
    case SUBPH_tclean:
      fprintf(file,"//! snet tclned\n\n");
      break;
    case SUBPH_ptran:
      fprintf(file,"//! snet ptran\n\n");
      break;
    case SUBPH_disam:
      fprintf(file,"//! snet disam\n\n");
      break;
    case SUBPH_ploc:
      fprintf(file,"//! snet ploc\n\n");
      break;
    case SUBPH_netren:
      fprintf(file,"//! snet netren\n\n");
      break;
    case SUBPH_netflat:
      fprintf(file,"//! snet final\n\n");
      break;
    default:
      fprintf(file,"*** Unsupported compilation subphase ***\n");
      break;
    }
    break;
  case PH_final:
  case PH_codegen:
    fprintf(file,"//! snet final\n\n");
    break;
  default:
    fprintf(file,"*** Unsupported compilation phase ***\n");
    break;
  }

  if(MODULE_INTERFACES(arg_node) != NULL) {
    TRAVdo(MODULE_INTERFACES(arg_node), arg_info);
  }


  if(MODULE_DEFS(arg_node) != NULL) {
    TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTnetbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTnetbody");

  if(NETBODY_DEFS(arg_node) != NULL) {
    fprintf(file," {\n");
    INFO_INDENTLEVEL(arg_info)++;

    TRAVdo(NETBODY_DEFS(arg_node), arg_info);

    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    fprintf(file,"}");
  }

  if(NETBODY_CONNECT(arg_node) == NULL) {
    fprintf(file,";\n");
  }
  else {
    fprintf(file,"\n");
    indent(arg_info);
    fprintf(file,"connect ");
    TRAVdo(NETBODY_CONNECT(arg_node), arg_info);
    fprintf(file,";\n");
  }
  DBUG_RETURN(arg_node);
}

node *PRTnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTnetdef");

  if(NETDEF_METADATA(arg_node) != NULL) {
    fprintf(file,"\n");
    indent(arg_info);
    fprintf(file,"<metadata>\n");
    INFO_INDENTLEVEL(arg_info)++;
    indent(arg_info);
    fprintf(file,"<net name=\"%s\">\n", NETDEF_NAME(arg_node));

    INFO_INDENTLEVEL(arg_info)++;

    TRAVdo(NETDEF_METADATA(arg_node), arg_info);

    INFO_INDENTLEVEL(arg_info)--;

    indent(arg_info);
    fprintf(file,"</net>\n");
    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    fprintf(file,"</metadata>\n");
  }

  fprintf(file,"\n");
  indent(arg_info);
  fprintf(file,"net ");

  if(global.compiler_phase >= PH_postproc) {
    if(NETDEF_EXTERNAL(arg_node)) {
      fprintf(file,"%s::", NETDEF_PKGNAME(arg_node));
    }
  }
  fprintf(file,"%s", NETDEF_NAME(arg_node));

  if(NETDEF_SIGN(arg_node) != NULL) {

    if (global.compiler_phase < PH_typesys || NETDEF_SIGNED(arg_node)) {
      fprintf(file," (\n");
    }
    else {
      fprintf(file," !(\n");
    }
    INFO_INDENTLEVEL(arg_info)++;
    indent(arg_info);
    TRAVdo(NETDEF_SIGN(arg_node), arg_info);
    fprintf(file,"\n");
    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    fprintf(file,")");
  }

  if(NETDEF_TOPLEVEL(arg_node)) {
    INFO_TOPLEVEL(arg_info) = TRUE;
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }
  else { // For external nets
    fprintf(file,";\n");
  }

  if(NETDEF_TOPLEVEL(arg_node)) {
    INFO_TOPLEVEL(arg_info) = FALSE;
  }


  DBUG_RETURN(arg_node);
}

node *PRTnetrefs(node *arg_node, info *arg_info)
{
  node *ptr = NULL;

  DBUG_ENTER("PRTnetrefs");

  ptr = NETREFS_NET(arg_node);

  if(global.compiler_phase >= PH_postproc) {
    if(NETDEF_EXTERNAL(ptr)) {
      fprintf(file,"%s::", NETDEF_PKGNAME(ptr));
    }
  }
  fprintf(file,"%s", NETDEF_NAME(ptr));

  if(NETREFS_LOCATION(arg_node) >= 0) {
    fprintf(file,"@%d", NETREFS_LOCATION(arg_node));
  }

  if(NETREFS_NEXT(arg_node) != NULL) {
    fprintf(file," ");
    TRAVdo(NETREFS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTrange(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTrange");

  switch(RANGE_STARTTYPE(arg_node)) {
  case FILT_stag:
    fprintf(file,"<");
    TRAVdo(RANGE_STAGSTART(arg_node), arg_info);
    fprintf(file,">");
    break;
  case FILT_btag:
    fprintf(file,"<");
    TRAVdo(RANGE_BTAGSTART(arg_node), arg_info);
    fprintf(file,">");
    break;
  default:
    fprintf(file,"*** Unsupported range start type ***\n");
    break;
  }
/* -- no second (b)tag.
  switch(RANGE_STOPTYPE(arg_node)) {
  case FILT_stag:
    fprintf(file," : <");
    TRAVdo(RANGE_STAGSTOP(arg_node), arg_info);
    fprintf(file,">");
    break;
  case FILT_btag:
    fprintf(file,": <");
    TRAVdo(RANGE_BTAGSTOP(arg_node), arg_info);
    fprintf(file,">");
    break;
  default:
    break;
  }
*/
  DBUG_RETURN(arg_node);
}

node *PRTrecentries(node *arg_node, info *arg_info)
{
  bool printPassAfter = FALSE;

  DBUG_ENTER("PRTrecentries");

  switch (RECENTRIES_QUALIFIER(arg_node)) {
  case LQUA_none:
    break;
  case LQUA_disc:
    if (INFO_SIDE(arg_info) == PRT_TM_RHS) {
      fprintf(file,"\\");
    }
    break;
  case LQUA_pass:
    if (INFO_SIDE(arg_info) == PRT_TM_RHS) {
      fprintf(file,"=");
    }
    else {
      printPassAfter = (INFO_SIDE(arg_info) == PRT_TM_LHS);
    }
    break;
  }

  if(RECENTRIES_FIELD(arg_node) != NULL) {
    TRAVdo(RECENTRIES_FIELD(arg_node), arg_info);
  }
  else if(RECENTRIES_STAG(arg_node) != NULL) {
    fprintf(file,"<");
    TRAVdo(RECENTRIES_STAG(arg_node), arg_info);
    fprintf(file,">");
  }
  else if(RECENTRIES_BTAG(arg_node) != NULL) {
    fprintf(file,"<");
    TRAVdo(RECENTRIES_BTAG(arg_node), arg_info);
    fprintf(file,">");
  }

  if (printPassAfter) {
    fprintf(file,"=");
  }

  if(RECENTRIES_NEXT(arg_node) != NULL) {
    fprintf(file,",");
    TRAVdo(RECENTRIES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTrectype");

  fprintf(file,"{");

  if(RECTYPE_ENTRIES(arg_node) != NULL) {
    TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
  }

  fprintf(file,"}");

  DBUG_RETURN(arg_node);
}

node *PRTserial(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTserial");

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,"(");
  }

  TRAVdo(SERIAL_LEFT(arg_node), arg_info);

  fprintf(file," .. ");

  TRAVdo(SERIAL_RIGHT(arg_node), arg_info);

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,")");
  }

  DBUG_RETURN(arg_node);
}

node *PRTat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTat");

  if(AT_LOCATION(arg_node) >= 0) {
    fprintf(file,"(");
  }

  TRAVdo(AT_LEFT(arg_node), arg_info);

  if(AT_LOCATION(arg_node) >= 0) {
    fprintf(file,")@%d", AT_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *PRTsplit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTsplit");

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,"(");
  }

  TRAVdo(SPLIT_LEFT(arg_node), arg_info);

  if(SPLIT_ISDETERM(arg_node)) {
    fprintf(file," !!");
  }
  else {
    fprintf(file," !");
  }

  if(SPLIT_ISDISTRIBUTED(arg_node)) {
    fprintf(file,"@ ");
  } else {
    fprintf(file," ");
  }

  TRAVdo(SPLIT_RANGE(arg_node), arg_info);

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,")");
  }

  DBUG_RETURN(arg_node);
}

node *PRTstagref(node *arg_node, info *arg_info)
{
  node *ptr = NULL;
  node *pkg = NULL;

  DBUG_ENTER("PRTstagref");

  ptr = STAGREF_STAG(arg_node);

  if(global.compiler_phase >= PH_postproc) {
    pkg = STAGS_PKG(ptr);

    if(pkg != NULL) {
      fprintf(file,"%s::", NETDEF_PKGNAME(pkg));
    }
  }

  fprintf(file,"%s", STAGS_NAME(ptr));

  DBUG_RETURN(arg_node);
}

node *PRTstags(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTstags");

  DBUG_ASSERT(FALSE, "This function should never be called.");

  DBUG_RETURN(arg_node);
}

node *PRTstaginit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTstaginit");

  TRAVdo(STAGINIT_STAG(arg_node), arg_info);

  switch(STAGINIT_TYPE(arg_node)) {
  case FILT_stag:
  case FILT_btag:
    fprintf(file," <- ");
    TRAVdo(STAGINIT_INIT(arg_node), arg_info);
    break;
  case FILT_int:
    fprintf(file," <- %d", STAGINIT_NUM(arg_node));
    break;
  default:
    /* Empty */
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PRTstar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTstar");

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,"(");
  }

  TRAVdo(STAR_LEFT(arg_node), arg_info);

  if(STAR_ISDETERM(arg_node)) {
    fprintf(file," ** ");
  }
  else {
    fprintf(file," * ");
  }

  TRAVdo(STAR_TERM(arg_node), arg_info);

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,")");
  }

  DBUG_RETURN(arg_node);
}

node *PRTfeedback(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfeedback");

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,"(");
  }

  TRAVdo(FEEDBACK_LEFT(arg_node), arg_info);

  fprintf(file," \\ ");

  TRAVdo(FEEDBACK_BACK(arg_node), arg_info);

  if(global.compiler_phase < PH_norm
      || (global.compiler_phase == PH_norm && global.compiler_subphase < SUBPH_topoflat)) {
    fprintf(file,")");
  }

  DBUG_RETURN(arg_node);
}

node *PRTsync(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTsync");

  fprintf(file,"[|");

  TRAVdo(SYNC_MAINPATTERN(arg_node), arg_info);
  fprintf(file, ", ");
  TRAVdo(SYNC_AUXPATTERNS(arg_node), arg_info);

  fprintf(file,"|]");

  if(SYNC_LOCATION(arg_node) >= 0) {
    fprintf(file,"@%d", SYNC_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *PRTtrans(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtrans");

  fprintf(file,"[");

  TRAVdo(TRANS_LEFT(arg_node), arg_info);

  fprintf(file," -> ");

  TRAVdo(TRANS_RIGHT(arg_node), arg_info);

  fprintf(file,"]");

  if(TRANS_LOCATION(arg_node) >= 0) {
    fprintf(file,"@%d", TRANS_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}


node *PRTtypedef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtypedef");

  indent(arg_info);

  fprintf(file,"type %s = ", TYPEDEF_NAME(arg_node));

  TRAVdo(TYPEDEF_TYPE(arg_node), arg_info);

  fprintf(file,";\n");

  DBUG_RETURN(arg_node);
}

node *PRTtyperef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtyperef");

  fprintf(file,"%s", TYPEDEF_NAME(TYPEREF_TYPE(arg_node)));

  DBUG_RETURN(arg_node);
}

node *PRTtypes(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtypes");

  if(TYPES_TYPE(arg_node) != NULL) {
    TRAVdo(TYPES_TYPE(arg_node), arg_info);
  }

  if(TYPES_NEXT(arg_node) != NULL) {

    if (INFO_SIDE(arg_info) == PRT_TM_RHS) {
      fprintf(file,"\n");
      INFO_INDENTLEVEL(arg_info)--;
      indent(arg_info);
      fprintf(file,"| ");
      INFO_INDENTLEVEL(arg_info)++;
    }
    else {
      fprintf(file," | ");
    }
    TRAVdo(TYPES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTtagexpr(node *arg_node, info *arg_info)
{

  DBUG_ENTER("PRTtagexpr");

  if(TAGEXPR_CONDITION(arg_node) != NULL &&
     TAGEXPR_LEFT(arg_node)      != NULL &&
     TAGEXPR_RIGHT(arg_node)     != NULL &&
     TAGEXPR_OPERATOR(arg_node)  == OPER_COND) {

    fprintf(file,"(");
    TRAVdo(TAGEXPR_CONDITION(arg_node), arg_info);
    fprintf(file," ? ");
    TRAVdo(TAGEXPR_LEFT(arg_node), arg_info);
    fprintf(file," : ");
    TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
    fprintf(file,")");
  }

  if(TAGEXPR_CONDITION(arg_node) == NULL &&
     TAGEXPR_LEFT(arg_node)      == NULL &&
     TAGEXPR_RIGHT(arg_node)     != NULL) {

    switch(TAGEXPR_OPERATOR(arg_node)) {
    case OPER_NOT:
      fprintf(file,"!");
      TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
      break;
    case OPER_ABS:
      fprintf(file,"abs(");
      TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
      fprintf(file,")");
      break;
    default:
      // error!
      break;
    }
  }

  if(TAGEXPR_CONDITION(arg_node) == NULL &&
     TAGEXPR_LEFT(arg_node)      != NULL &&
     TAGEXPR_RIGHT(arg_node)     != NULL) {

    fprintf(file,"(");
    TRAVdo(TAGEXPR_LEFT(arg_node), arg_info);

    switch(TAGEXPR_OPERATOR(arg_node)) {
    case OPER_MUL:
      fprintf(file," * ");
      break;
    case OPER_DIV:
      fprintf(file," / ");
      break;
    case OPER_REM:
      fprintf(file," %% ");
      break;
    case OPER_ADD:
      fprintf(file," + ");
      break;
    case OPER_SUB:
      fprintf(file," - ");
      break;
    case OPER_MIN:
      fprintf(file," min ");
      break;
    case OPER_MAX:
      fprintf(file," max ");
      break;
    case OPER_EQ:
      fprintf(file," == ");
      break;
    case OPER_NEQ:
      fprintf(file," != ");
      break;
    case OPER_LT:
      fprintf(file," < ");
      break;
    case OPER_LTQ:
      fprintf(file," <= ");
      break;
    case OPER_GT:
      fprintf(file," > ");
      break;
    case OPER_GTQ:
      fprintf(file," >= ");
      break;
    case OPER_AND:
      fprintf(file," && ");
      break;
    case OPER_OR:
      fprintf(file," || ");
      break;
    default:
      /* Rrror */
      break;
    }

    TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
    fprintf(file,")");
  }

  if(TAGEXPR_CONDITION(arg_node) == NULL &&
     TAGEXPR_LEFT(arg_node)      == NULL &&
     TAGEXPR_RIGHT(arg_node)     == NULL) {

    if(TAGEXPR_STAG(arg_node) != NULL) {
      TRAVdo(TAGEXPR_STAG(arg_node), arg_info);
    }
    else if(TAGEXPR_BTAG(arg_node) != NULL) {
      TRAVdo(TAGEXPR_BTAG(arg_node), arg_info);
    }
    else {
      fprintf(file,"%d", TAGEXPR_INTEGERCONST(arg_node));
    }
  }

  DBUG_RETURN(arg_node);
}

node *PRTguardpatterns(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTguardpatterns");

  if(GUARDPATTERNS_ENTRIES(arg_node) != NULL) {
    TRAVdo(GUARDPATTERNS_ENTRIES(arg_node), arg_info);
  }

  if(GUARDPATTERNS_CONDITION(arg_node) != NULL) {
    fprintf(file," if <");
    TRAVdo(GUARDPATTERNS_CONDITION(arg_node), arg_info);
    fprintf(file,">");
  }

  if(GUARDPATTERNS_NEXT(arg_node) != NULL) {
    fprintf(file,",");
    TRAVdo(GUARDPATTERNS_NEXT(arg_node), arg_info);
 }

  DBUG_RETURN(arg_node);
}

node *PRTguardactions(node *arg_node, info *arg_info)
{

  DBUG_ENTER("PRTguardactions");

  if(GUARDACTIONS_TAGEXPR(arg_node) != NULL) {
    fprintf(file," if <");

    TRAVdo(GUARDACTIONS_TAGEXPR(arg_node), arg_info);
    fprintf(file,">");
  }

  if (GUARDACTIONS_TAGEXPR(arg_node) != NULL || INFO_NOELSE(arg_info)) {
    fprintf(file," -> ");
  }
  else {
    fprintf(file," else -> ");
  }

  if(GUARDACTIONS_ACTION(arg_node) != NULL) {
    TRAVdo(GUARDACTIONS_ACTION(arg_node), arg_info);
  }

  INFO_NOELSE(arg_info) = FALSE;

  if(GUARDACTIONS_NEXT(arg_node) != NULL) {
    TRAVdo(GUARDACTIONS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTrecouts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTrecouts");

  fprintf(file,"{");
  if(RECOUTS_FIELDS(arg_node) != NULL) {
    TRAVdo(RECOUTS_FIELDS(arg_node), arg_info);
  }
  fprintf(file,"}");

  if(RECOUTS_NEXT(arg_node) != NULL) {
    fprintf(file,"; ");
    TRAVdo(RECOUTS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);

}

node *PRToutputfields(node *arg_node, info *arg_info)
{

  DBUG_ENTER("PRToutputfields");

  if(OUTPUTFIELDS_LEFTFIELD(arg_node) != NULL) {
    TRAVdo(OUTPUTFIELDS_LEFTFIELD(arg_node), arg_info);

    if(OUTPUTFIELDS_RIGHTFIELD(arg_node) != NULL) {
      fprintf(file,"=");
      TRAVdo(OUTPUTFIELDS_RIGHTFIELD(arg_node), arg_info);
    }
  }

  if(OUTPUTFIELDS_STAG(arg_node) != NULL) {
    fprintf(file,"<");

    TRAVdo(OUTPUTFIELDS_STAG(arg_node), arg_info);

    if(OUTPUTFIELDS_TAGEXPR(arg_node) != NULL) {
      fprintf(file,"=");

      TRAVdo(OUTPUTFIELDS_TAGEXPR(arg_node), arg_info);
    }
      fprintf(file,">");

  }
  else if(OUTPUTFIELDS_BTAG(arg_node) != NULL) {
    fprintf(file,"<");

    TRAVdo(OUTPUTFIELDS_BTAG(arg_node), arg_info);

    if(OUTPUTFIELDS_TAGEXPR(arg_node) != NULL) {
      fprintf(file,"=");

      TRAVdo(OUTPUTFIELDS_TAGEXPR(arg_node), arg_info);
    }
    fprintf(file,">");
  }

  if(OUTPUTFIELDS_NEXT(arg_node) != NULL) {
    fprintf(file,",");
    TRAVdo(OUTPUTFIELDS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTfilt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfilt");

  fprintf(file,"[" );

  if(FILT_PATTERN(arg_node) != NULL) {
    TRAVdo(FILT_PATTERN(arg_node), arg_info);
  }

  INFO_NOELSE(arg_info) = TRUE;

  if(FILT_GUARDACTIONS(arg_node) != NULL) {
    TRAVdo(FILT_GUARDACTIONS(arg_node), arg_info);
  }

  fprintf(file,"]" );

  if(FILT_LOCATION(arg_node) > 0) {
    fprintf(file,"@%d", FILT_LOCATION(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *PRTtypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL) {
    INFO_SIDE(arg_info) = PRT_TM_LHS;
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  fprintf(file," -> ");

  if(TYPEMAP_OUTTYPE(arg_node) != NULL) {
    fprintf(file,"\n");
    INFO_INDENTLEVEL(arg_info) += 2;
    indent(arg_info);
    INFO_SIDE(arg_info) = PRT_TM_RHS;
    TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 2;
  }
  else if (TYPEMAP_OUTTYPETOINFER(arg_node)){
    fprintf(file,"...");
  }

  INFO_SIDE(arg_info) = PRT_TM_NONE;
  DBUG_RETURN(arg_node);
}

node *PRTtypesigdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTtypesigdef");

  indent(arg_info);
  fprintf(file,"typesig ");

  if(TYPESIGDEF_NAME(arg_node) != NULL) {
    fprintf(file,"%s", TYPESIGDEF_NAME(arg_node));
  }

  fprintf(file," = ");

  if(TYPESIGDEF_TYPESIGN(arg_node) != NULL) {
    TRAVdo(TYPESIGDEF_TYPESIGN(arg_node), arg_info);
  }

  fprintf(file,";\n");

  DBUG_RETURN(arg_node);
}

node *PRTtypesigns(node *arg_node, info *arg_info)
{

  DBUG_ENTER("PRTtypesigns");

  if(TYPESIGNS_TYPESIG(arg_node) != NULL) {
    TRAVdo(TYPESIGNS_TYPESIG(arg_node), arg_info);
  }

  if(TYPESIGNS_NEXT(arg_node) != NULL) {
    fprintf(file,",\n");
    indent(arg_info);
    TRAVdo(TYPESIGNS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTtypesigref(node *arg_node, info *arg_info)
{
  node *ptr = NULL;

  DBUG_ENTER("PRTtypesigref");

  ptr = TYPESIGREF_TYPESIG(arg_node);

  fprintf(file,"%s", TYPESIGDEF_NAME(ptr));

  DBUG_RETURN(arg_node);
}

node *PRTmetadatadefs( node *arg_node, info *arg_info)
{
  bool first;

  DBUG_ENTER("PRTmetadatadefs");

  first = INFO_IS_FIRST_METADATADEF(arg_info);

  if(first) {
    fprintf(file,"\n");
    indent(arg_info);
    fprintf(file,"<metadata>\n");
    INFO_INDENTLEVEL(arg_info)++;
    INFO_IS_FIRST_METADATADEF(arg_info) = FALSE;
  }

  if(METADATADEFS_DEF(arg_node) != NULL) {
    TRAVdo(METADATADEFS_DEF(arg_node), arg_info);
  }

  if(METADATADEFS_NEXT(arg_node) != NULL) {
    TRAVdo(METADATADEFS_NEXT(arg_node), arg_info);
  }

  if(first) {
    INFO_INDENTLEVEL(arg_info)--;
    indent(arg_info);
    fprintf(file,"</metadata>\n");
    INFO_IS_FIRST_METADATADEF(arg_info) = TRUE;
  }

  DBUG_RETURN(arg_node);
}

node *PRTmetadatakeylist( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTetadatakeylist");

  indent(arg_info);

  if(METADATAKEYLIST_VALUE(arg_node) != NULL) {
    fprintf(file,"<%s value=\"%s\" />\n", METADATAKEYLIST_KEY(arg_node), METADATAKEYLIST_VALUE(arg_node));
  } else {
    fprintf(file,"<%s />\n", METADATAKEYLIST_KEY(arg_node));
  }

  if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
    TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTmetadatadefaultdef( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmetadatadefaultdef");

  indent(arg_info);

  switch(METADATADEFAULTDEF_TYPE(arg_node)) {
  case MD_net:
    fprintf(file,"<netdefault>\n");
    break;
  case MD_box:
    fprintf(file,"<boxdefault>\n");
    break;
  case MD_all:
  default:
    fprintf(file,"<default>\n");
    break;
  }

  INFO_INDENTLEVEL(arg_info)++;

  if(METADATADEFAULTDEF_KEYS(arg_node) != NULL) {
    TRAVdo(METADATADEFAULTDEF_KEYS(arg_node), arg_info);
  }

  INFO_INDENTLEVEL(arg_info)--;
  indent(arg_info);

  switch(METADATADEFAULTDEF_TYPE(arg_node)) {
  case MD_net:
    fprintf(file,"</netdefault>\n");
    break;
  case MD_box:
    fprintf(file,"</boxdefault>\n");
    break;
  case MD_all:
  default:
    fprintf(file,"</default>\n");
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PRTmetadatanetdef( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmetadatanetdef");

  indent(arg_info);
  fprintf(file,"<net name=\"%s\">\n", METADATANETDEF_NAME(arg_node));

  INFO_INDENTLEVEL(arg_info)++;

  if(METADATANETDEF_KEYS(arg_node) != NULL) {
    TRAVdo(METADATANETDEF_KEYS(arg_node), arg_info);
  }

  if(METADATANETDEF_CHILDS(arg_node) != NULL) {
    TRAVdo(METADATANETDEF_CHILDS(arg_node), arg_info);
  }

  if(METADATANETDEF_DEFINITIONS(arg_node) != NULL) {
    TRAVdo(METADATANETDEF_DEFINITIONS(arg_node), arg_info);
  }

  INFO_INDENTLEVEL(arg_info)--;
  indent(arg_info);
  fprintf(file,"</net>\n");

  DBUG_RETURN(arg_node);
}

node *PRTmetadataboxdef( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmetadataboxdef");

  indent(arg_info);
  fprintf(file,"<box name=\"%s\">\n", METADATABOXDEF_NAME(arg_node));

  INFO_INDENTLEVEL(arg_info)++;

  if(METADATABOXDEF_KEYS(arg_node) != NULL) {
    TRAVdo(METADATABOXDEF_KEYS(arg_node), arg_info);
  }

  INFO_INDENTLEVEL(arg_info)--;
  indent(arg_info);
  fprintf(file,"</box>\n");

  DBUG_RETURN(arg_node);
}

node *PRTmetadatalist( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmetadatalist");

  if(METADATALIST_ENTRY(arg_node) != NULL) {
    TRAVdo(METADATALIST_ENTRY(arg_node), arg_info);
  }

  if(METADATALIST_NEXT(arg_node) != NULL) {
    TRAVdo(METADATALIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTlanguageinterfaces( node *arg_node, info *arg_info)
{
  DBUG_ENTER("languageinterfaces");

  /* Do nothing */

  if(LANGUAGEINTERFACES_NEXT(arg_node) != NULL) {
    TRAVdo(LANGUAGEINTERFACES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *PRTdoPrint(node *syntax_tree)
{
  info *i = NULL;

  DBUG_ENTER("PRTdoPrint");

  DBUG_ASSERT((syntax_tree!= NULL), "PRTdoPrint called with empty syntaxtree");

  file = stdout;

  fprintf( file, "\n\n------------------------------\n\n");

  TRAVpush(TR_prt);

  i = infoMake();

  syntax_tree = TRAVdo(syntax_tree, i);

  i = infoFree(i);

  TRAVpop();

  fprintf(file,"\n\n------------------------------\n\n");

  file = NULL;

  DBUG_RETURN(syntax_tree);
}
