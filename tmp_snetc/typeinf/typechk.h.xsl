<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: typeinf.h.xsl 1792 2008-02-01 10:21:49Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   08.06.2007
 *
 * Description:
 *
 * Type Inference - Main
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a codegen.h file implementing all
     functions needed to generate code from a node -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='TC']" />

</xsl:template>

</xsl:stylesheet>
