<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: tres.h.xsl 1723 2007-11-12 15:09:56Z cg $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   08.06.2007
 *
 * Description:
 *
 * Code generation file for type resolution function definitions.
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a tres.h file implementing all
     functions needed to resolve types in a syntax trees -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='PREPTRES']" />

</xsl:template>

</xsl:stylesheet>