<?xml version="1.0"?>

<!--
 *******************************************************************************
 *
 * $Id: copy_node.c.xsl 2501 2009-08-03 10:10:09Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   06.06.2007
 *
 * Description:
 *
 * Code generation file for tree copy function implementations.
 *
 *******************************************************************************
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_c_code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/">
  <!-- generate file header and doxygen group -->
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'copy_node.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by copy traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'copy_node.c.xsl'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'copy'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Copy Tree Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by copy traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#include "copy.h"
#include "copy_attribs.h"
#include "copy_node.h"
#include "traverse.h"
#include "dbug.h"
#include "tree_basic.h"
#include "str.h"


#define COPYTRAV( node, info) (node != NULL) ? TRAVdo( node, info) : node

  </xsl:text>
  <!-- functions -->
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>


<!-- generate copy functions -->
<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- variable and new empty node for result -->    
  <xsl:value-of select="'node *result = '"/>
  <xsl:apply-templates select="." mode="make-function-call"/>
  <xsl:value-of select="';'"/>
  <!-- give hint we start to copy now -->
  <xsl:value-of select="'DBUG_ENTER(&quot;COPY'"/>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'&quot;);'"/>
  <!-- Copy err_code -->
  <xsl:value-of select="'NODE_ERRCODE(result) = COPYattribString(NODE_ERRCODE(arg_node), arg_node);'"/>
  <!-- call copy for attributes -->   
  <xsl:if test="count(attributes/attribute) > 0">
    <xsl:text>
    /* Copy attributes */
    </xsl:text>
  </xsl:if>
  <xsl:apply-templates select="attributes/attribute"/> 
  <!-- call copy for all sons --> 
  <xsl:if test="count(sons/son[@name]) > 0">
    <xsl:text>
    /* Copy sons */
    </xsl:text>
  </xsl:if>
  <xsl:apply-templates select="sons/son[@name]"/>
  <!-- copy location stack is not necessary: TRAVdo+TBmake* will handle it -->
  <!-- return value -->
  <xsl:text>
  /* Return value */
  </xsl:text>
  <xsl:value-of select="'DBUG_RETURN( result);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>


<!-- generate a comment and a function header -->
<xsl:template match="@name">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">COPY</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Copies the node and its sons/attributes</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">COPY</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>


<!-- generate copy calls for sons-->
<xsl:template match="son">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">result</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = COPYTRAV( '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_info);'"/>
</xsl:template>


<!-- generate copying attributes -->
<xsl:template match="attribute">
  <!-- left side of assignment -->
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'result'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' = '" />
  <!-- right side of assignment -->
  <xsl:choose>
    <!-- literal attributes are just copied (assigment) -->
    <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
      <xsl:call-template name="node-access">
        <xsl:with-param name="node">
          <xsl:value-of select="'arg_node'" />
        </xsl:with-param>
        <xsl:with-param name="nodetype">
          <xsl:value-of select="../../@name" />
        </xsl:with-param>
        <xsl:with-param name="field">
          <xsl:value-of select="@name" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="';'" />
    </xsl:when>
    <!-- string attributes are deep copied -->
    <xsl:otherwise>
      <xsl:value-of select="'COPYattrib'"/>
      <xsl:value-of select="./type/@name"/>
      <xsl:value-of select="'('"/>
      <xsl:call-template name="node-access">
        <xsl:with-param name="node">
          <xsl:value-of select="'arg_node'" />
        </xsl:with-param>
        <xsl:with-param name="nodetype">
          <xsl:value-of select="../../@name" />
        </xsl:with-param>
        <xsl:with-param name="field">
          <xsl:value-of select="@name" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="', arg_node);'"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!-- generate a make function call -->
<xsl:template match="node" mode="make-function-call">
  <!-- function name -->
  <xsl:value-of select="'TBmake'"/>
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="substring( @name, 1, 1)" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="substring( @name, 2, 30)" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'( '"/>
  <!-- permanent attributes without default value first --> 
  <xsl:apply-templates select="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]" mode="make-function-call"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]">
    <xsl:if test="sons/son[ not( @default)]">
      <xsl:value-of select="' ,'"/>
    </xsl:if>
  </xsl:if>
  <!-- sons without default value are last parameters -->
  <xsl:apply-templates select="sons/son[ not( @default)]" mode="make-function-call"/>
  <xsl:value-of select="')'"/>
</xsl:template>


<!-- generate default value for node argument (NULL) -->
<xsl:template match="son" mode="make-function-call">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <!-- all sons are NULL by default -->
  <xsl:value-of select="'NULL'"/>
</xsl:template>


<!-- generate an attribute argument -->
<xsl:template match="attribute" mode="make-function-call">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>  
  </xsl:if>
  <xsl:apply-templates select="@name" mode="make-function-call" />
</xsl:template>


<!-- generate default values for non-node arguments -->
<xsl:template match="@name" mode="make-function-call">
  <xsl:value-of select="key(&quot;types&quot;, ../type/@name)/@init"/>
</xsl:template>

</xsl:stylesheet>
