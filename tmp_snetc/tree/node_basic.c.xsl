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

<!--  $Id: node_basic.c.xsl 2489 2009-07-30 13:52:55Z hxc $  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_make_head.xsl"/>
<xsl:import href="../xml/common_make_head_checkmem.xsl"/>
<xsl:import href="../xml/common_make_body.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.c file implementing all
     the make functions for nodes                                    -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'node_basic.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to allocate node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: node_basic.c.xsl 2489 2009-07-30 13:52:55Z hxc $'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#include "node_basic.h"
#include "tree_basic.h"
#include "dbug.h"
#include "check_mem.h"
#include "memory.h"
#include "globals.h"
#include "ctinfo.h"
  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
  /* end of file */

  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:text>

/*****************************************************************************
 * N_</xsl:text><xsl:value-of select="@name"/><xsl:text> :
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="." mode="make-head-checkmem-ifdef"/>
  <xsl:apply-templates select="." mode="make-head-checkmem"/>
  <xsl:apply-templates select="." mode="make-head-checkmem-else"/>
  <xsl:apply-templates select="." mode="make-head"/>
  <xsl:apply-templates select="." mode="make-head-checkmem-endif"/>
  <xsl:apply-templates select="." mode="make-body"/>
</xsl:template>

</xsl:stylesheet>
