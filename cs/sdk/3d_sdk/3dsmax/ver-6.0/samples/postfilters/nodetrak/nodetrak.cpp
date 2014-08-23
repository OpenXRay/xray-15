//-----------------------------------------------------------------------------
// -----------------------
// File ....: NodeTrak.cpp
// -----------------------
// Author...: Gus J Grubba
// Date ....: March 1996
// Descr....: NodeTrak Image Filter
//
// History .: Mar, 5 1995 - Started
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "NodeTrak.h"
#include "resource.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		 case DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hDLLInst;
				break;
		 case DLL_PROCESS_DETACH:
				hInst  = NULL;
				break;
		 case DLL_THREAD_ATTACH:
				break;
		 case DLL_THREAD_DETACH:
				break;
	}
	return TRUE;
}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// NodeTrak Class Description

class NODETRAKClassDesc:public ClassDesc {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_NodeTrak; }
		const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_NODETRAK);     }
		SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(1,0);    }
		const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static NODETRAKClassDesc NODETRAKDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_LIBDESCRIPTION); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &NODETRAKDesc; break;
		default: return 0;        break;
	}
}

DLLEXPORT ULONG LibVersion ( )  { 
	return ( VERSION_3DSMAX ); 
}

const TCHAR *ImageFilter_NodeTrak::Description( ) 
{
	return GetString(IDS_DB_NODE_TRACKER);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::ImageFilter_NodeTrak()
//

ImageFilter_NodeTrak::ImageFilter_NodeTrak()   {
	data.version= NODETRAKVERSION; 
	data.nodename[0] = 0;
}

//-----------------------------------------------------------------------------
// #> NodeDlg::proc()
//

void NodeDlg::proc(INodeTab &nodeTab) {
	if (flt) {
		if (nodeTab.Count())
			flt->SetNodeName(nodeTab[0]->GetName());
	}
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::Control()
//

BOOL ImageFilter_NodeTrak::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)   {

	static NODETRAKDATA tempdata;

	switch (message) {

		case WM_INITDIALOG:

			tempdata = data;
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			if (data.nodename[0])
				SetDlgItemText(hWnd,IDC_NODENAME,data.nodename);
			else
				SetDlgItemText(hWnd,IDC_NODENAME,"Undefined");

			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDC_PICKNODE:
					nodeDlg = new NodeDlg(this);
					if (Max()->DoHitByNameDialog(nodeDlg)) {
						if (data.nodename[0])
							SetDlgItemText(hWnd,IDC_NODENAME,data.nodename);
					}
					break;

				case IDOK:
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					data = tempdata;
					EndDialog(hWnd,0);
					break;

			}

			return 1;

		case WM_DESTROY:
			if (nodeDlg) {
				delete nodeDlg;
				nodeDlg = NULL;
			}
			return 1;

	}

	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlDlgProc()
//

INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)   {
	static ImageFilter_NodeTrak *flt = NULL;
	if (message == WM_INITDIALOG) 
		flt = (ImageFilter_NodeTrak *)lParam;
	if (flt) 
		return (flt->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::ShowControl()

BOOL ImageFilter_NodeTrak::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_NODETRAK_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this));
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		case WM_INITDIALOG: {
			  CenterWindow(hWnd,GetParent(hWnd));
			  SetCursor(LoadCursor(NULL,IDC_ARROW));
			  return 1;
		}

		case WM_COMMAND:

			  switch (LOWORD(wParam)) {
				  
				  case IDOK:              
						 EndDialog(hWnd,1);
						 break;

				  case IDCANCEL:
						 EndDialog(hWnd,0);
						 break;
		
			  }
			  return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::ShowAbout()

void ImageFilter_NodeTrak::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_NODETRAK_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::Render()
//

BOOL ImageFilter_NodeTrak::Render(HWND hWnd) {

	if (!srcmap)
		return(FALSE);

	//-- Get Render Info from Bitmap

	RenderInfo *ri = srcmap->GetRenderInfo();

	if (!ri)
		return(FALSE);

	//-- Get Inode

	INode *node = Max()->GetINodeByName(data.nodename);

	if (!node)
		return(FALSE);

	//-- Track Node

	Matrix3 m         = node->GetObjTMAfterWSM(ri->renderTime[0]);
	Point3 worldPos   = m.GetRow(3);
	Point3 camPos     = worldPos * ri->worldToCam[0];

	//-- At this point, camPos.z is the Z depth. The camera looks
	//   down in the negative z direction.

	
	
	//-- Now map to screen coordinates

	Point2 screenPos = ri->MapCamToScreen(camPos);

	int x = (int)(screenPos.x+0.5f);
	int y = (int)(screenPos.y+0.5f);

	//-- Place a dot at the location

	BMM_Color_64 white = {0xFFFF,0xFFFF,0xFFFF,0};

	srcmap->PutPixels(x,y,1,&white);

	return(TRUE);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::LoadConfigure()

BOOL ImageFilter_NodeTrak::LoadConfigure ( void *ptr ) {
	NODETRAKDATA *buf = (NODETRAKDATA *)ptr;
	if (buf->version == NODETRAKVERSION) {
		memcpy((void *)&data,ptr,sizeof(NODETRAKDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::SaveConfigure()

BOOL ImageFilter_NodeTrak::SaveConfigure ( void *ptr ) {
	if (ptr) {
		memcpy(ptr,(void *)&data,sizeof(NODETRAKDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_NodeTrak::EvaluateConfigure()

DWORD ImageFilter_NodeTrak::EvaluateConfigure ( ) {
	 return (sizeof(NODETRAKDATA));
}

//-- EOF: NodeTrak.cpp ------------------------------------------------------------
