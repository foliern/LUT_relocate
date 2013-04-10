<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: csplit.h.xsl 1790 2008-01-25 11:55:35Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   08.06.2007
 *
 * Description:
 *
 * Code generation file for code generation function definitions.
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

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='CSPLIT']" />

</xsl:template>

</xsl:stylesheet>
