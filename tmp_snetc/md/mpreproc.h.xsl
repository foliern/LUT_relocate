<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: mpreproc.h.xsl 2483 2009-07-30 06:49:20Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   30.09.2009
 *
 * Description:
 *
 * This stylesheet generates a mpreproc.h file implementing all
 * functions needed to preprocess meta data
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a mpreproc.h file implementing all
     functions needed to preprocess meta data  -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='MDPREPROC']" />

</xsl:template>

</xsl:stylesheet>