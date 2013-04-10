<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: mremdup.h.xsl 2483 2009-07-30 06:49:20Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   30.07.2009
 *
 * Description:
 *
 * This stylesheet generates a mremdup.h file implementing all
 * functions needed to remove duplicate meta data keys
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a mremdup.h file implementing all
     functions needed to remove duplicate meta data keys  -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='MDREMDUP']" />

</xsl:template>

</xsl:stylesheet>