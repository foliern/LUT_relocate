<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: typeinf.h.xsl 1792 2008-02-01 10:21:49Z jju $
 *
 * Primitive Sig Inference header file generator
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='TSIP']" />

</xsl:template>

</xsl:stylesheet>
