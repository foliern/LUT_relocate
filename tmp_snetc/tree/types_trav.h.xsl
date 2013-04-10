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

<!--  $Id: types_trav.h.xsl 1723 2007-11-12 15:09:56Z cg $  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="../xml/common_key_tables.xsl"/>
  <xsl:import href="../xml/common_travfun.xsl"/>
  <xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'types_trav.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the trav_t phase enumeration.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id: types_trav.h.xsl 1723 2007-11-12 15:09:56Z cg $'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_TYPES_TRAV_H_
#define _SAC_TYPES_TRAV_H_

    </xsl:text>
    <xsl:apply-templates select="/definition/phases" />
    <xsl:text>
#endif /* _SAC_TYPES_TRAV_H_ */
    </xsl:text>
  </xsl:template>

  <xsl:template match="phases" >
    <xsl:value-of select="'typedef enum { TR_undefined = 0'" />
    <xsl:apply-templates select=".//traversal" />
    <xsl:value-of select="'} trav_t; '" />
  </xsl:template>

  <xsl:template match="traversal" >
    <xsl:value-of select="', TR_'" />
    <xsl:call-template name="lowercase" >
      <xsl:with-param name="string" >
        <xsl:value-of select="@id" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' = '" />
    <xsl:value-of select="position()" />
  </xsl:template>

</xsl:stylesheet>
