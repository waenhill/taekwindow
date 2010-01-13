<?xml version="1.0" encoding="utf-8"?>

<stylesheet version="1.0" xmlns="http://www.w3.org/1999/XSL/Transform">

	<import href="..\version.xslt"/>
	<import href="..\meta.xslt"/>
	
	<output method="text"/>
	
	<template match="/">
		#ifndef VERSION_HPP
		#define VERSION_HPP
    
		/* This file is automatically generated from version.h.xslt. Do not edit. */
		#define APPLICATION_TITLE             "<value-of select="$apptitle"/>"
		#define APPLICATION_AUTHOR            "<value-of select="$author"/>"
		#define APPLICATION_AUTHOR_FULL       "<value-of select="$authorfull"/>"
		#define APPLICATION_COPYRIGHT         "<value-of select="$copyright"/>"
		#define APPLICATION_VERSION            <value-of select="$fullversioncommas"/>
		#define APPLICATION_VERSION_STRING    "<value-of select="$fullversion"/>"
		#define APPLICATION_WEBSITE           "<value-of select="$website"/>"
		#define APPLICATION_EMAIL             "<value-of select="$email"/>"
		#define APPLICATION_LICENSE_BRIEF     "<value-of select="$licensebrief"/>"
		#define APPLICATION_README_FILE       "<value-of select="$readmefile"/>"
		#define MAIN_EXE_FILE                 "<value-of select="$mainexefile"/>"

		#endif
	</template>
	
</stylesheet>