<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id: invokecc.h.xsl 1806 2008-03-05 09:31:12Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   08.06.2007
 *
 * Description:
 *
 * Code generation file for  function definitions of C-compiler 
 * calls to compile S-Net runtime 
 *
 ******************************************************************************
-->


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_make_trav_header_file.xsl" />

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a invokecc.h file -->

<xsl:template match="/" >
  <xsl:apply-templates select="/definition/phases/general/traversal[@id='CC']" />
</xsl:template>

</xsl:stylesheet>
