/*===========================================================================*\
 | 
 |  FILE:	CVDExport.h
 |			A simple exporter that scans the scene for our custom data
 |			and if found, exports it to an ASCII file
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 16-4-99
 | 
\*===========================================================================*/
/*===========================================================================*\
 |	Most of this project is a converted Skeleton Exporter
 |	The main functionality and the area of most interest is in DoExport.cpp
\*===========================================================================*/


#ifndef __CVDEXP__H
#define __CVDEXP__H

#include "max.h"
#include "resource.h"
#include "iparamm2.h"
#include "modstack.h"


#define	CVDEXP_CLASSID		Class_ID(0x3e816f85, 0x92f7904)


TCHAR *GetString(int id);
extern ClassDesc* GetCVDExportDesc();

/*===========================================================================*\
 |	CVDExporter class defn
\*===========================================================================*/

class CVDExporter : public SceneExport {
public:
	CVDExporter();
	~CVDExporter();

	// Preferences values
	int searchtype;

	// Used in DoExport
	BOOL exportSelected;
	FILE *fileStream;
	Interface* ip;



	// Number of extensions we support
	int ExtCount();
	const TCHAR * Ext(int n);

	// The bookkeeping functions
	const TCHAR * LongDesc();
	const TCHAR * ShortDesc();
	const TCHAR * AuthorName();
	const TCHAR * CopyrightMessage();
	const TCHAR * OtherMessage1();
	const TCHAR * OtherMessage2();

	// Version number of this exporter
	unsigned int Version();

	// Show an about box
	void ShowAbout(HWND hWnd);

	// Do the actual export
	int DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

	// Returns whether we support the extended exporter options
	BOOL SupportsOptions(int ext, DWORD options);

	// Scene enumeration
	BOOL nodeEnum(INode* node,Interface *ip);


	// Configuration file management
	BOOL LoadExporterConfig();
	void SaveExporterConfig();
	TSTR GetConfigFilename();

};

#endif