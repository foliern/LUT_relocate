<?xml version="1.0"?>

<!--
 ******************************************************************************
 *
 * $Id:
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   08.06.2007
 *
 * Description:
 *
 * Common xsl-templates to create header files for user defined
 * traversal funtions
 *
 ******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="./common_travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="traversal">

  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="@include"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: '" />
      <xsl:value-of select="@include" />
      <xsl:value-of select="'.xsl'" />
    </xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name="newline" />
  <xsl:value-of select="'#ifndef _SNETC_'"/>
  <xsl:value-of select="@id"/>
  <xsl:value-of select="'_H'"/>
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#define _SNETC_'"/>
  <xsl:value-of select="@id"/>
  <xsl:value-of select="'_H'"/>
  <xsl:call-template name="newline" />
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#include &quot;types.h&quot;'"/>
  <xsl:call-template name="newline" />
  <xsl:call-template name="newline" />

  <xsl:if test="./topfunction">
    <xsl:call-template name="newline" />
    <xsl:value-of select="'/* ************************ Top level function: ************************  */'" />
    <xsl:call-template name="newline" />
    <xsl:call-template name="newline" />

    <xsl:if test="./topfunction/description" >
      <xsl:value-of select="'/* '"/>
      <xsl:value-of select="./topfunction/description" />

      <xsl:value-of select="' */'" />
      <xsl:call-template name="newline" />
      <xsl:call-template name="newline" />
    </xsl:if>

    <xsl:value-of select="'extern node *'" />
    <xsl:value-of select="./topfunction/@name" />
    <xsl:value-of select="'(node *syntax_tree);'" />
    <xsl:call-template name="newline" />
    <xsl:call-template name="newline" />
    <xsl:value-of select="'/* ********************************************************************** */'" />
    <xsl:call-template name="newline" />
    <xsl:call-template name="newline" />
  </xsl:if>

  <xsl:choose>
    <xsl:when test="@default='sons'">
      <xsl:apply-templates select="./travuser/node">
        <xsl:with-param name="identifier" select="@id" />
        <xsl:sort select="@name"/>	
      </xsl:apply-templates>
   </xsl:when>
   <xsl:otherwise>
     <xsl:apply-templates select="../../../syntaxtree/node">
       <xsl:with-param name="identifier" select="@id" />
       <xsl:sort select="@name"/>
     </xsl:apply-templates>
   </xsl:otherwise>
  </xsl:choose>

  <xsl:call-template name="newline" />
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#endif /* _SNETC_'" />
  <xsl:value-of select="@id"/>
  <xsl:value-of select="'_H */'"/>
  <xsl:call-template name="newline" />
</xsl:template>

<xsl:template match="node">
  <xsl:param name="identifier" />
  <xsl:value-of select="'extern '" />
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix" select="$identifier" />
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
</xsl:template>

<xsl:template name="newline">
 <xsl:text>&#xa;</xsl:text>
</xsl:template>

</xsl:stylesheet>
