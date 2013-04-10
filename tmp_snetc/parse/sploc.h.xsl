<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: sploc.h.xsl 2176 2008-12-09 09:38:19Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   21.08.2007
 *
 * Description:
 *
 * 
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a cploc.h file implementing all
     functions needed to ...   -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='SPLOC']" />

</xsl:template>

</xsl:stylesheet>