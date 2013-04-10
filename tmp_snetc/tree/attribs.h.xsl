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

<!--  $Id: attribs.h.xsl 1723 2007-11-12 15:09:56Z cg $  -->

<!-- This xslt script generates a definition for type node from ast.xml. To
     generate attribs.h using the Sablotron XSLT Engine, execute
     > sabcmd ast2node.xslt ast.xml node.h
-->
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
        <xsl:value-of select="'attribs.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Defines the AttribUnion and attrib structures.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id: attribs.h.xsl 1723 2007-11-12 15:09:56Z cg $'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_ATTRIBS_H_
#define _SAC_ATTRIBS_H_

#include "types.h"

    </xsl:text>
    <!-- start phase that generates a struct of attributes for each node -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-structs"/>
    <!-- start phase that unites all attribute structs to one union -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-union"/>
    <xsl:text>
#endif /* _SAC_ATTRIBS_H_ */
    </xsl:text>
  </xsl:template>

  <!-- this template starts generation of attribute structs -->
  <xsl:template match="syntaxtree" mode="generate-attrib-structs">
     <xsl:text>
/******************************************************************************
 * For each node a structure of its attributes is defined, named 
 * ATTRIBS_&lt;nodename&gt;
 *****************************************************************************/
     </xsl:text>
     <xsl:apply-templates select="node" mode="generate-attrib-structs">
       <xsl:sort select="@name"/>
     </xsl:apply-templates>
  </xsl:template>

  <!-- generate a attribute structure for a son -->
  <xsl:template match="node" mode="generate-attrib-structs">
    <xsl:value-of select="'struct ATTRIBS_N_'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' { '"/>
    <xsl:apply-templates select="attributes/attribute" mode="generate-attrib-structs"/>
    <xsl:apply-templates select="flags" mode="generate-attrib-structs"/>
    <xsl:value-of select="' } ;'"/>
  </xsl:template>

  <!-- generate an entry for an attribute within the attribute structure -->
  <xsl:template match="attribute" mode="generate-attrib-structs">
    <xsl:value-of select="key( &quot;types&quot;, ./type/@name)/@ctype"/>
    <xsl:value-of select="' '"/>
    <xsl:value-of select="@name"/>
    <xsl:if test="key( &quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'['" />
      <xsl:value-of select="key( &quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="']'" />
    </xsl:if>
    <xsl:value-of select="'; '"/>
  </xsl:template>

  <!-- generate the fields required for a flag -->
  <xsl:template match="flags[flag]" mode="generate-attrib-structs">
    <xsl:value-of select="'struct { '" />
    <xsl:apply-templates select="flag" mode="generate-attrib-structs" />
    <xsl:value-of select="'} flags;'" />
  </xsl:template>

  <xsl:template match="flags" mode="generate-attrib-structs">
  </xsl:template>

  <xsl:template match="flag" mode="generate-attrib-structs">
    <xsl:value-of select="'unsigned int '" />
    <xsl:value-of select="@name" />
    <xsl:value-of select="' : 1;'" />
  </xsl:template>

  <!-- this template starts generation of the attribstruct union -->
  <xsl:template match="syntaxtree" mode="generate-attrib-union">
    <xsl:text>
/*****************************************************************************
 * This union handles all different types of attributes. Its members are
 * called N_nodename.
 ****************************************************************************/
#ifdef CLEANMEM
    </xsl:text>
    <xsl:value-of select="'struct ATTRIBUNION { '"/>
    <xsl:text>
#else
    </xsl:text>
    <xsl:value-of select="'union ATTRIBUNION { '"/>
    <xsl:text>
#endif
    </xsl:text>
    <xsl:apply-templates select="node" mode="generate-attrib-union">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'} ; '"/>
  </xsl:template>
 
  <!-- generate an entry for each node within the union -->
  <xsl:template match="node" mode="generate-attrib-union">
    <xsl:value-of select="'struct ATTRIBS_N_'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' *'" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'; '"/>
  </xsl:template>
</xsl:stylesheet>
