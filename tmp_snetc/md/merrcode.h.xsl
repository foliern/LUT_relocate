<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: merrcode.h.xsl 2501 2009-08-03 10:10:09Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   03.08.2009
 *
 * Description:
 *
 * This stylesheet generates a mredist.h file implementing all
 * functions needed to propagate user-defined error codes to all nodes
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a mredist.h file implementing all
     functions needed to propagate user-defined error codes to all nodes -->

<xsl:template match="/" >

  <xsl:apply-templates select="/definition/phases/general/traversal[@id='MERRCODE']" />

</xsl:template>

</xsl:stylesheet>