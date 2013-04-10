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

<!--  $Id: traverse_tables.h.xsl 1723 2007-11-12 15:09:56Z cg $  -->

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
        <xsl:value-of select="'traverse_tables.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the function tables for traversal.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id: traverse_tables.h.xsl 1723 2007-11-12 15:09:56Z cg $'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_TRAVERSE_TABLES_H_
#define _SAC_TRAVERSE_TABLES_H_

#include "types.h"

typedef travfun_p travfunarray_t [ </xsl:text><xsl:value-of select="count(//syntaxtree/node)+1" /><xsl:text>] ;

typedef travfunarray_t travtables_t [</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>] ;

typedef travfun_p preposttable_t [</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>] ;

extern travtables_t travtables;
extern preposttable_t pretable;
extern preposttable_t posttable;
extern const char *travnames[</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>];

#endif /* _SAC_TRAVERSE_TABLES_H_ */
    </xsl:text>
  </xsl:template>

</xsl:stylesheet>
