/**********************************************************************
 *<
	FILE: XMLUtility.cpp

	DESCRIPTION:	Handy functions for XML, to keep the code a bit cleaner

	CREATED BY:		Neil Hazzard

	HISTORY:		Summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "IGameExporter.h"
#include "atlbase.h"
#include "msxml2.h"

BOOL InitialiseXML(IXMLDOMNode  ** root, IXMLDOMDocument ** doc)
{

	HRESULT hr;
	hr = CoInitialize(NULL); 
	// Check the return value, hr...
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&doc);
	if(FAILED(hr))
		return false;
	// Check the return value, hr...
//	hr = (IXMLDOMDocument &)doc->QueryInterface(IID_IXMLDOMNode, (void **)&root);
	if(FAILED(hr))
		return false;

	return true;
	
}


BOOL CreateXMLNode(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR *nodeName, IXMLDOMNode ** newNode)
{

	IXMLDOMNode * sceneNode;
	doc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(nodeName), NULL, &sceneNode);
	node->appendChild(sceneNode,newNode);
	return true;
}

BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, TCHAR * value)
{
	CComQIPtr<IXMLDOMElement> element;
	element = node;
	element->setAttribute(CComBSTR(name), CComVariant(value));
	return true;
}

BOOL AddXMLText(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR * text)
{
	CComPtr <IXMLDOMText> keydata = NULL;
	doc->createTextNode(CComBSTR(text), &keydata);
	node->appendChild(keydata,NULL);
	return true;

}