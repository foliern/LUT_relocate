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

<!--  $Id: common_key_tables.xsl 1723 2007-11-12 15:09:56Z cg $  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- as we make lookups to attributetypes quite often, we create a key -->
<xsl:key name="types" match="//attributetypes/type" use="@name"/>
<xsl:key name="arraytypes" match="//attributetypes/type[@size]" use="@name"/>

<!-- the same for traversal defaults -->
<xsl:key name="traversals" match="//phases//traversal" use="@id" />

<!-- and one for all nodes -->
<xsl:key name="nodes" match="/definition/syntaxtree/node" use="@name" />

<!-- and one for all nodesets -->
<xsl:key name="nodesets" match="/definition/nodesets/nodeset" use="@name" />

</xsl:stylesheet>
