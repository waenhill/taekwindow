<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:import href="metadata.xslt"/>
	<xsl:output method="text"/>
	
	<xsl:template match="information">
		/* This file is automatically generated from metadata-cs.xslt and information.xml. Do not edit. */
		namespace TaekwindowConfig {

			public static class VersionInfo
			{
				public const string Title = "<xsl:value-of select="apptitle"/>";
				public const string Author = "<xsl:value-of select="author"/>";
				public const string AuthorFull = "<xsl:value-of select="author"/> &lt;<xsl:value-of select="email"/>&gt;";
				public const string Copyright = "<xsl:value-of select="copyright"/>";
				public const string VersionString = "<xsl:call-template name="fullversion"/>";
				public const string ShortVersionString = "<xsl:call-template name="shortversion"/>";
				public const string Website = "<xsl:value-of select="website"/>";
				public const string Email = "<xsl:value-of select="email"/>";
				public const string ReadmeFile = "<xsl:value-of select="readmefile"/>";
			}

		}
	</xsl:template>

</xsl:stylesheet> 
