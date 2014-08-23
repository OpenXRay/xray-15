/*===========================================================================*\
 | 
 |  FILE:	CVDExport.cpp
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

#include "CVDExport.h"




/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class CVDExportClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic()				{ return TRUE; }
	void *			Create( BOOL loading )	{ return new CVDExporter; }
	const TCHAR *	ClassName()				{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()			{ return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID()				{ return CVDEXP_CLASSID; }
	const TCHAR* 	Category()				{ return _T("");  }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()			{ return _T("CVDExporter"); }
	HINSTANCE		HInstance()				{ return hInstance; }
};

static CVDExportClassDesc CVDExportCD;
ClassDesc* GetCVDExportDesc() {return &CVDExportCD;}



/*===========================================================================*\
 |	Basic implimentation of a dialog handler
\*===========================================================================*/

static BOOL CALLBACK CustomDialogHandler(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	switch (msg) 
	{
		case WM_INITDIALOG:
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			break;
	}
	return FALSE;
}



/*===========================================================================*\
 |  Constructor/Destructor - just initialize any variables or memory
\*===========================================================================*/

CVDExporter::CVDExporter()
{
	searchtype = 1;
}

CVDExporter::~CVDExporter()
{
}

/*===========================================================================*\
 |  Return how many extensions we support, and what they are
\*===========================================================================*/

int CVDExporter::ExtCount() { return 1; }

const TCHAR * CVDExporter::Ext(int n)
{
	switch(n) {
		case 0:
			return GetString(IDS_EXT_01);
		}
	return _T("");

}


/*===========================================================================*\
 |  Return various information about our scene exporter
\*===========================================================================*/

const TCHAR * CVDExporter::LongDesc()
{
	return GetString(IDS_LONGDESC);
}

const TCHAR * CVDExporter::ShortDesc()
{
	return GetString(IDS_SHORTDESC);
}

const TCHAR * CVDExporter::AuthorName()
{
	return GetString(IDS_AUTHOR);
}

const TCHAR * CVDExporter::CopyrightMessage()
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR * CVDExporter::OtherMessage1() { return _T(""); }
const TCHAR * CVDExporter::OtherMessage2() { return _T(""); }

// Version number = (version * 100)
unsigned int CVDExporter::Version()
{
	return 100;
}



/*===========================================================================*\
 |  Show about box
\*===========================================================================*/

static INT_PTR CALLBACK AboutDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	CVDExporter *se = (CVDExporter*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!se && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:	
			// Update class pointer
			se = (CVDExporter*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					EndDialog(hWnd,1);
				break;
			}
			break;

		default:
			return FALSE;
		}	
	return TRUE;
	}

void CVDExporter::ShowAbout(HWND hWnd)
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ABOUT),
		hWnd,
		AboutDlgProc,
		(LPARAM)this);
}
