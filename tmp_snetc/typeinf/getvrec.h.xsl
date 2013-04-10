<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: typeinf.h.xsl 1792 2008-02-01 10:21:49Z jju $
 *
 * Sig Getter - called when compiler is resumed after route inference, where
 * the NETDEF_NTYPESIG attributes are NULL but NETDEF_SIGN nodes contain all
 * we need to regenerate NETDEF_NTYPESIG.
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='TGV']" />
  <xsl:text>
  #include "typing.h"
  TYPvrectype *TGVdoGetVRec(node *syntax_tree);
  TYPvrectype *TGVdoGetVRecUC(node *syntax_tree, bool collectUTs, bool collectCTs);
  </xsl:text>
</xsl:template>

</xsl:stylesheet>
