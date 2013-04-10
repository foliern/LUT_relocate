<?xml version="1.0"?>

<!--
 ***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2007                        *
 *         SAC Research Foundation (http://www.sac-home.org/)          *
 *                                                                     *
 *                        All Rights Reserved                          *
 *                                                                     *
 *   The copyright holder makes no warranty of any kind with respect   *
 *   to this product and explicitly disclaims any implied warranties   *
 *   of merchantability or fitness for any particular purpose.         *
 *                                                                     *
 ***********************************************************************
 -->

<!--  $Id: common_name_to_nodeenum.xsl 1723 2007-11-12 15:09:56Z cg $  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- here, the renaming scheme from a node name (as used in name attribute)
     to the nodetype enum name for that node (N_xxx) is defined. All other
     traversals use this template to perform that renaming -->

<xsl:template name="name-to-nodeenum">
  <xsl:param name="name"/>
  <!-- they all start with a leading N_ -->
  <xsl:value-of select="'N_'" />
  <xsl:call-template name="lowercase">
    <xsl:with-param name="string">
      <xsl:value-of select="$name" />
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>
