/**********************************************************************
 *<
	FILE: XMLUtility.h

	DESCRIPTION:	XML utilites

	CREATED BY:		Neil Hazzard	

	HISTORY:		summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "atlbase.h"
#include "msxml2.h"


//BOOL InitialiseXML(IXMLDOMNode ** root, IXMLDOMDocument ** doc);
BOOL CreateXMLNode(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR *nodeName, IXMLDOMNode ** newNode);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, TCHAR * value);
BOOL AddXMLText(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR * text);
