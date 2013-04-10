/*******************************************************************************
 *
 * $Id: codefile.h 3576 2012-07-31 12:56:35Z fpz $
 *
 *******************************************************************************/


#ifndef _SNETC_CODEFILE_H_
#define _SNETC_CODEFILE_H_

#include "types.h"

/*******************************************************************************
 *
 * Description: Open C-files (.h and .c).
 *
 * Parameters: - base, base file name
 *
 * Return: - TRUE, OK
 *         - FALSE, error
 *
 *******************************************************************************/
extern bool CODEFILEopenFiles(const char *base);

/*******************************************************************************
 *
 * Description: Close C-files (.h and .c).
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEcloseFiles();

/*******************************************************************************
 *
 * Description: Write include to header file.
 *
 * Parameters: - fileBase, base name of include file
 *             - fileType, type of include file
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteHdrInclude(const char *fileBase, const char *fileType);

/*******************************************************************************
 *
 * Description: Write include to source file.
 *
 * Parameters: - fileBase, base name of include file
 *             - fileType, type of include file
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSrcInclude(const char *fileBase, const char *fileType);

/*******************************************************************************
 *
 * Description: Write field definition to header file.
 *
 * Parameters: - pkg, package name
 *             - field, field name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteFieldDefine(const char *pgk, const char *field);

/*******************************************************************************
 *
 * Description: Write simple tag definition to header file.
 *
 * Parameters: - pkg, package name
 *             - stag, simple tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteStagDefine(const char *pgk, const char *stag);

/*******************************************************************************
 *
 * Description: Write binding tag definition to header file.
 *
 * Parameters: - pkg, package name
 *             - btag, binding tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBtagDefine(const char *pgk, const char *btag);

/*******************************************************************************
 *
 * Description: Start writing box section to source file.
 *
 * Parameters: - box, box name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxStart(const char *box);

/*******************************************************************************
 *
 * Description: Start writing net section to source file.
 *
 * Parameters: - net, net name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteNetStart(const char *net);
extern void CODEFILEwriteTopLevelNetStart(const char *net);

/*******************************************************************************
 *
 * Description: Start writing STAR_INCARNATE net section to source file.
 *
 * Parameters: - net, net name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteNetStarIncStart();

/*******************************************************************************
 *
 * Description: Start writing box compute function to source file.
 *
 * Parameters: - box, box name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxCompStart(const char *box);

/*******************************************************************************
 *
 * Description: Write box compute function field name to source file.
 *
 * Parameters: - field, field name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxCompField(const char *field);

/*******************************************************************************
 *
 * Description: Write box compute function simple tag name to source file.
 *
 * Parameters: - stag, simple tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxCompStag(const char *stag);

/*******************************************************************************
 *
 * Description: Write box compute function binding tag name to source file.
 *
 * Parameters: - btag, binding tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxCompBtag(const char *btag);

/*******************************************************************************
 *
 * Description: Write entity name to source file.
 *
 * Parameters: - entity, box or net name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetEntity(const char *pkg, const char *entity);

/*******************************************************************************
 *
 * Description: Write name of box as quoted string
 *
 * Parameters: box name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBoxname( const char *name);

/*******************************************************************************
 *
 * Description: Start writing SNetBox-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetBoxStart();

/*******************************************************************************
 *
 * Description: Stop writing SNetBox-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetBoxStop();

/*******************************************************************************
 *
 * Description: Write SNetAlias-function to source file.
 *
 * Parameters: - net, name of alias network
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetAlias(const char *pkg, const char *net, int location);

/*******************************************************************************
 *
 * Description: Start writing SNetSerial-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetSerialStart();

/*******************************************************************************
 *
 * Description: Start writing SNetSplit-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetSplitStart();

/*******************************************************************************
 *
 * Description: Start writing SNetSplitDet-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetSplitDetStart();

/*******************************************************************************
 *
 * Description: Start writing SNetStar-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetStarStart();

/*******************************************************************************
 *
 * Description: Start writing SNetStarDet-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetStarDetStart();

/*******************************************************************************
 *
 * Description: Write SNetStarIncarnate-entity to source file.
 *
 * Parameters: - entity, box or net name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetStarIncEntity(const char *entity);

/*******************************************************************************
 *
 * Description: Start writing SNetStarIncarnate-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetStarIncStart();

/*******************************************************************************
 *
 * Description: Start writing SNetStarDetIncarnate-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetStarDetIncStart();

/*******************************************************************************
 *
 * Description: Start writing SNetFeedback-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetFeedbackStart();

/*******************************************************************************
 *
 * Description: Start writing SNetSync-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetSyncStart();

/*******************************************************************************
 *
 * Description: Start writing SNetFilter-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetFilterStart();

/*******************************************************************************
 *
 * Description: Start writing SNetTranslate-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetTranslateStart();

/*******************************************************************************
 *
 * Description: Start writing SNetParallel-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetParallelStart();

/*******************************************************************************
 *
 * Description: Start writing SNetParallelDet-function to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetParallelDetStart();

/*******************************************************************************
 *
 * Description: Start writing variant list to source file.
 *
 * Parameters: - variantCount, number of variants
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteVariantListStart(unsigned int variantCount);

/*******************************************************************************
 *
 * Description: Start writing variant list list to source file.
 *
 * Parameters: - variantListCount, number of variant lists
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteVariantListListStart(unsigned int variantCount);

/*******************************************************************************
 *
 * Description: Write an empty variant to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteEmptyVariant();

/*******************************************************************************
 *
 * Description: Start writing variant to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteVariantStart();

/*******************************************************************************
 *
 * Description: Start writing int list to source file.
 *
 * Parameters: - count, number of entries
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteIntListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Start writing int list list to source file.
 *
 * Parameters: - count, number of entries
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteIntListListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Write field taking to source file.
 *
 * Parameters: - field, field name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteTakeField(const char*pkg, const char *field);

/*******************************************************************************
 *
 * Description: Write field reading to source file.
 *
 * Parameters: - field, field name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteGetField(const char*pkg, const char *field);

/*******************************************************************************
 *
 * Description: Write simple tag taking to source file.
 *
 * Parameters: - stag, simple tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteTakeStag(const char*pkg, const char *stag);

/*******************************************************************************
 *
 * Description: Write simple tag reading to source file.
 *
 * Parameters: - stag, simple tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteGetStag(const char*pkg, const char *stag);

/*******************************************************************************
 *
 * Description: Write binding tag taking to source file.
 *
 * Parameters: - btag, binding tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteTakeBtag(const char*pkg, const char *btag);

/*******************************************************************************
 *
 * Description: End of scope to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteScopeStop();

/*******************************************************************************
 *
 * Description: End of function (with semicolon) to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteFunctionFullStop();

/*******************************************************************************
 *
 * Description: End of function (without semicolon) to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteFunctionStop();

/*******************************************************************************
 *
 * Description: Constant integer expression to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetEconsti(int consti);

/*******************************************************************************
 *
 * Description: Constant true expression to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetEconstbTrue();

/*******************************************************************************
 *
 * Description: Expression list start to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteSnetEStart(const char *operator);

/*******************************************************************************
 *
 * Description: Write comma to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteNext();

/*******************************************************************************
 *
 * Description: Write field to source file.
 *
 * Parameters: - pkg   - pagkace name
 *               field - fieldname
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteField(const char *pkg, const char *field);

/*******************************************************************************
 *
 * Description: Write simple tag to source file.
 *
 * Parameters: - pkg   - pagkace name
 *               stag  - tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteStag(const char *pkg, const char *stag);

/*******************************************************************************
 *
 * Description: Write binding tag to source file.
 *
 * Parameters: - pkg   - pagkace name
 *               btag  - tag name
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteBtag(const char *pkg, const char *btag);

/*******************************************************************************
 *
 * Description: Write NULL to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteNULL();

/*******************************************************************************
 *
 * Description: Write start of type encoding list to source file.
 *
 * Parameters: - entry count - number of type encoding in the list
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteTypeEncListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Write start of expression list to source file.
 *
 * Parameters: - entry count - number of expressions in the list
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteExprListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Write start of filter instruction set list to source file.
 *
 * Parameters: - entry count - number of filter instructions sets in the list
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteFilterInstrListListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Write start of filter instruction set to source file.
 *
 * Parameters: - entry count - number of filter instructions in the set
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteFilterInstrListStart(unsigned int entryCount);

/*******************************************************************************
 *
 * Description: Write function call to get record to source file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteGetRecord();

/*******************************************************************************
 *
 * Description: Write initialization of a new field to source file.
 *
 * Parameters: - field - name of the field
 *
 * Return: -
 *
 *******************************************************************************/
extern void CODEFILEwriteInitField(const char *field);

/*******************************************************************************
 *
 * Description: Write initialization of a new simple tag to source file.
 *
 * Parameters: - stag - name of the simple tag
 *
 * Return: -
 *
 *******************************************************************************/

extern void CODEFILEwriteInitStag(const char *stag);

/*******************************************************************************
 *
 * Description: Write initialization of a new binding tag to source file.
 *
 * Parameters: - btag - name of the tag
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteInitBtag(const char *btag);

/*******************************************************************************
 *
 * Description: Write returning of the out buffer to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteOutBufReturn();

/*******************************************************************************
 *
 * Description: Write new line and indent the next line in source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteNewlineAndIndent();

/*******************************************************************************
 *
 * Description: Increase indent level which is used to indent lines in source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void increaseIndentLevel();

/*******************************************************************************
 *
 * Description: Decrease indent level which is used to indent lines in source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void decreaseIndentLevel();

/*******************************************************************************
 *
 * Description: Write initialization of out buffer to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteOutBufDecl();

/*******************************************************************************
 *
 * Description: Write initialization of out type type encoding to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/

extern void CODEFILEwriteBoxSignEncStart();
extern void CODEFILEwriteOutTypeDecl();

extern void CODEFILEwriteBoxSignField();
extern void CODEFILEwriteBoxSignTag();
extern void CODEFILEwriteBoxSignBTag();

/*******************************************************************************
 *
 * Description: Write initialization of box signature encoding to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteBoxSignDecl();

/*******************************************************************************
 *
 * Description: Write initialization of record to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteRecDecl();

/*******************************************************************************
 *
 * Description: Write new line to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteNewline();

/*******************************************************************************
 *
 * Description: Write starting of simple tag in expression to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteSnetEStagStart();

/*******************************************************************************
 *
 * Description: Write starting of binding tag in expression to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteSnetEBtagStart();

/*******************************************************************************
 *
 * Description: Write top level snet function to header file.
 *
 * Parameters: - net - name of the top level net
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteTopLevelNetDeclaration(const char *net);


/*******************************************************************************
 *
 * Description: Write starting of filter instruction snet_field to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/

extern void CODEFILEwriteFilterInstrFieldStart();


/*******************************************************************************
 *
 * Description: Write starting of filter instruction snet_tag to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteFilterInstrTagStart();


/*******************************************************************************
 *
 * Description: Write starting of filter instruction snet_btag to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteFilterInstrBtagStart();


/*******************************************************************************
 *
 * Description: Write filter instruction create_record to source file.
 *
 * Parameters: - 
 *
 * Return: -
 *
 ********************************************************************************/
extern void CODEFILEwriteFilterInstrCreateRecord();

/*********************************************************
 * 
 * Functions for name shields.
 * 
 * Parameters: - net : name of the signed internal net
 *
 *********************************************************/

extern void CODEFILEwriteShieldInPart1(const char *net, int location);
extern void CODEFILEwriteShieldBodyPart1(const char *net, int location);
extern void CODEFILEwriteShieldOutPart1(const char *net, int location);
extern void CODEFILEwriteShieldPart2(void);
extern void CODEFILEwriteShieldNet(const char *net, int location);
extern void CODEFILEwriteShieldTopLevelNet(const char *net, int location);

extern void CODEFILEwriteObservedNetStart(const char *net);
extern void CODEFILEwriteSocketObserverBefore(const char *net, const char *realname, const char *interactive, 
					      const char * data, const char *code, const char *address, const char *port);
extern void CODEFILEwriteSocketObserverAfter(const char *net, const char *realname, const char *interactive, 
					     const char * data, const char *code, const char *address, const char *port);
extern void CODEFILEwriteFileObserverBefore(const char *net, const char *realname, const char *data, 
					    const char *code, const char *file);
extern void CODEFILEwriteFileObserverAfter(const char *net, const char *realname, const char *data, 
					   const char *code, const char *file);
extern void CODEFILEwriteObserverAuxStart(const char *net);
extern void CODEFILEwriteObserverSnetEntity(const char *pkg, const char *entity);
extern void CODEFILEwriteObserverBeforeSnetEntity(const char *pkg, const char *entity);
extern void CODEFILEwriteObserverAfterSnetEntity(const char *pkg, const char *entity);
extern void CODEFILEwriteObserverAuxSnetEntity(const char *pkg, const char *entity);

extern void CODEFILEwriteSnetLabelsDefinition();
extern void CODEFILEwriteSnetLabelsStart();
extern void CODEFILEwriteFieldLabel(const char *pkg, const char *field);
extern void CODEFILEwriteStagLabel(const char *pkg, const char *stag);
extern void CODEFILEwriteBtagLabel(const char *pkg, const char *btag);
extern void CODEFILEwriteSnetLabelsEnd();

extern void CODEFILEwriteNoneDefine();
extern void CODEFILEwriteNumberOfLabels();

extern void CODEFILEwriteInterfaceDefine(const char *interface);
extern void CODEFILEwriteNumberOfInterfaces();
extern void CODEFILEwriteSnetInterfacesDefinition();
extern void CODEFILEwriteSnetInterfacesStart();


extern void CODEFILEwriteInterface(const char *interface);
extern void CODEFILEwriteSnetInterfacesEnd();

extern void CODEFILEwriteMainStart();
extern void CODEFILEwriteMainInterfaceInit(const char *interface);
extern void CODEFILEwriteMainEnd();

/*******************************************************************************
 *
 * Description: Write wrapper code generated by plugin to source file
 *
 * Parameters: generated wrapper code
 *
 * Return: -
 *
 ******************************************************************************/

void CODEFILEwriteBoxWrap( const char *wrap);


void CODEFILEwriteLocation(int location);

void CODEFILEwriteSnetLocSplitStart();
void CODEFILEwriteSnetLocSplitDetStart();

void CODEFILEwriteSnetEntityName(const char *pkg, const char *entity);
void CODEFILEwriteNetToIdFunStart();
void CODEFILEwriteIdToNetFunStart();
void CODEFILEwriteNetToId(int count, const char *pkg, const char *entity);
void CODEFILEwriteIdToNet(int count, const char *pkg, const char *entity);
void CODEFILEwriteNetToIdFunEnd();
void CODEFILEwriteIdToNetFunEnd();

void CODEFILEwriteRealmCreate(const char *name);
void CODEFILEwriteRealmUpdate(const char *name);
void CODEFILEwriteRealmDestroy(const char *name);
void CODEFILEwriteBoxFullStop();

#endif /* _SNETC_CODEFILE_H_ */
