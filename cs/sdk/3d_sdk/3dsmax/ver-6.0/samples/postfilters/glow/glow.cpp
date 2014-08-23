//-----------------------------------------------------------------------------
// -------------------
// File	....:	Glow.cpp
// -------------------
// Author...:	Gus J	Grubba
// Date	....:	September 1995
// Descr....:	Glow Image Filter
//
// History	.:	Feb, 17 1996 -	Started
//				
//-----------------------------------------------------------------------------
		
//--	Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Glow.h"
#include "resource.h"

//--	Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

static ISpinnerControl	*mtlspin = NULL;
static ISpinnerControl	*nodspin = NULL;
static ISpinnerControl	*sizespin = NULL;
static IColorSwatch		*colorSwatch = NULL;
COLORREF colorref;

static BOOL controlsInit = FALSE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--	DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)	{
	switch (fdwReason) {
		 case	DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hDLLInst;
				if ( !controlsInit ) {
					controlsInit = TRUE;
					InitCustomControls(hInst);
					//InitCommonControls();
				}
				break;
		 case	DLL_PROCESS_DETACH:
				hInst =	NULL;
				break;
		 case	DLL_THREAD_ATTACH:
				break;
		 case	DLL_THREAD_DETACH:
				break;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Helpers

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Glow	Class	Description

class GLOWClassDesc:public ClassDesc {
	
	public:

		int				IsPublic ( )					{ return	1; }
		void			*Create (BOOL loading=FALSE)	{ return	new ImageFilter_Glow; }
		const	TCHAR	*ClassName ( )					{ return	GetString(IDS_DB_GLOW);	}
		SClass_ID		SuperClassID ( )				{ return	FLT_CLASS_ID; }
		Class_ID		ClassID	( )						{ return	Class_ID(GLOWCLASSID,0); }
		const	TCHAR	*Category ( )					{ return	GetString(IDS_DB_IMAGE_FILTER); }

};

static GLOWClassDesc GLOWDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription (	) { 
 	return GetString(IDS_DB_GLOW_FILTER); 
}

DLLEXPORT int	LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc	*LibClassDesc(int	i) {
	switch(i) {
		case 0:		return &GLOWDesc;	break;
		default:	return 0;			break;
	}
}

DLLEXPORT ULONG LibVersion (	)	{ 
	return (VERSION_3DSMAX);	
}


//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::ImageFilter_Glow()
//

ImageFilter_Glow::ImageFilter_Glow() {
	data.version = GLOWVERSION;
	data.type = IDC_MTLID_BUTT;
	data.mtl  = 0;
	data.node = 0;
	data.size = 20;	
	data.color.r = 0xFFFF;
	data.color.g = 0xFFFF;
	data.color.b = 0xFFFF;
	data.color.a = 0xFFFF;	
	data.colorsrc = IDC_MTLCOLOR_BUTT;	
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::Description()
//

const TCHAR *ImageFilter_Glow::Description( void ) {
	return GetString(IDS_GG_DESCRIPTION);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::HandleInputs()
//

void ImageFilter_Glow::HandleInputs( HWND hWnd ) {
	HWND hDlg;

	BOOL flag = IsDlgButtonChecked(hWnd,IDC_MTLID_BUTT);

	hDlg = GetDlgItem(hWnd,IDC_MTLID_EDIT);
	EnableWindow(hDlg,flag);
	hDlg = GetDlgItem(hWnd,IDC_MTLID_SPIN);
	EnableWindow(hDlg,flag);

	flag = !flag;

	hDlg = GetDlgItem(hWnd,IDC_NODEID_EDIT);
	EnableWindow(hDlg,flag);
	hDlg = GetDlgItem(hWnd,IDC_NODEID_SPIN);
	EnableWindow(hDlg,flag);
	
	flag = IsDlgButtonChecked(hWnd,IDC_USERCOLOR_BUTT);
	hDlg = GetDlgItem(hWnd,IDC_COLOR_SWATCH);
	EnableWindow(hDlg,flag); 	
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::Control()
//

BOOL ImageFilter_Glow::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{

	switch (message) {

		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			CheckRadioButton(
				hWnd,
				IDC_MTLID_BUTT,
				IDC_NODEID_BUTT,
				data.type
			);
			
			CheckRadioButton(
				hWnd,
				IDC_MTLCOLOR_BUTT,
				IDC_USERCOLOR_BUTT,
				data.colorsrc
			);
						
			colorSwatch = GetIColorSwatch(
				GetDlgItem(hWnd, IDC_COLOR_SWATCH),
				RGB(data.color.r>>8,data.color.g>>8,data.color.b>>8),
				GetString(IDS_DB_GLOW_COLOR));						

			mtlspin	= GetISpinner(GetDlgItem(hWnd, IDC_MTLID_SPIN));
			mtlspin->LinkToEdit( GetDlgItem(hWnd,IDC_MTLID_EDIT), EDITTYPE_INT );
			mtlspin->SetLimits(	0,15, FALSE );
			mtlspin->SetValue(data.mtl,FALSE);

			nodspin	= GetISpinner(GetDlgItem(hWnd, IDC_NODEID_SPIN));
			nodspin->LinkToEdit( GetDlgItem(hWnd,IDC_NODEID_EDIT), EDITTYPE_INT );
			nodspin->SetLimits(	1,65535, FALSE );
			nodspin->SetValue(data.node,FALSE);
			
			sizespin = GetISpinner(GetDlgItem(hWnd, IDC_SIZE_SPIN));
			sizespin->LinkToEdit( GetDlgItem(hWnd,IDC_SIZE_EDIT), EDITTYPE_INT );
			sizespin->SetLimits( 1,100, FALSE );
			sizespin->SetValue(data.size,FALSE);			

			HandleInputs(hWnd);

			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDC_MTLID_BUTT:
				case IDC_NODEID_BUTT:				
				case IDC_MTLCOLOR_BUTT:
				case IDC_USERCOLOR_BUTT:				
					HandleInputs(hWnd);
					break;

				case IDOK:
					data.type= (IsDlgButtonChecked(hWnd,IDC_MTLID_BUTT) ? IDC_MTLID_BUTT : IDC_NODEID_BUTT);
					data.mtl= mtlspin->GetIVal();
					data.node= nodspin->GetIVal();					
					data.size= sizespin->GetIVal();
					data.colorsrc= (IsDlgButtonChecked(hWnd, IDC_MTLCOLOR_BUTT) ? IDC_MTLCOLOR_BUTT : IDC_USERCOLOR_BUTT);					 
					colorref = colorSwatch->GetColor();
					data.color.r = GetRValue(colorref)<<8|0xFF;
					data.color.g = GetGValue(colorref)<<8|0xFF;
					data.color.b = GetBValue(colorref)<<8|0xFF;					
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;

			}

			return 1;

		case WM_DESTROY:
			  if (mtlspin) {
				  ReleaseISpinner(mtlspin);
				  mtlspin = NULL;
			  }
			  if (nodspin) {
				  ReleaseISpinner(nodspin);
				  nodspin = NULL;
			  }			  
			  if (sizespin) {
				  ReleaseISpinner(sizespin);
				  sizespin = NULL;
			  }

			  if (colorSwatch) {
					ReleaseIColorSwatch(colorSwatch);
					colorSwatch = NULL;
			  }			  
			  break;

	}

	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlDlgProc()
//

INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
     static ImageFilter_Glow *flt = NULL;
     if (message == WM_INITDIALOG) 
        flt = (ImageFilter_Glow *)lParam;
     if (flt) 
        return (flt->Control(hWnd,message,wParam,lParam));
     else
        return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::ShowControl()

BOOL ImageFilter_Glow::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_GLOW_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this));
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
	switch (message) {
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:				  
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
			}
			return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::ShowAbout()

void ImageFilter_Glow::ShowAbout(HWND hWnd)	{
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_GLOW_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::Render()
//

BOOL ImageFilter_Glow::Render(HWND hWnd) {

	BMM_Color_64	*l64,*line64	= NULL;	
	BOOL			result			= TRUE;
	BOOL			abort			= FALSE;

	BYTE			*mtlbuf			= NULL;
	WORD			*nodbuf			= NULL;
	DWORD			type;
	int				gpix,iy,ix;


	if	(!srcmap)
		return (FALSE);

	//-- Prepare Line Buffers ------------------------------------------------

	if	((line64 = (BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
		goto done;

	if (data.type == IDC_MTLID_BUTT) {
		mtlbuf = (BYTE *)srcmap->GetChannel(BMM_CHAN_MTL_ID,type);
		if (!mtlbuf)
			goto done;
	} else {
		nodbuf = (WORD *)srcmap->GetChannel(BMM_CHAN_NODE_ID,type);
		if (!nodbuf)
			goto done;
	}	

	
	//------------------------------------------------------------------------
	//-- Process -------------------------------------------------------------

	for (iy = 0; iy < srcmap->Height(); iy++) {

		gpix = iy * srcmap->Width();

		//-- Progress Report
		
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

		//-- Check for	Abort
		
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL	*)&abort);

		if (abort)
			goto done;
		
		//-- Get	line
		
		l64 = line64;

		if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;
		
		
		//-- Material ID -------------------------------
		
		if (data.type == IDC_MTLID_BUTT) {
			for ( ix = 0; ix < srcmap->Width();	ix++,l64++)	{
				if (mtlbuf[gpix+ix] == data.mtl) {              		
					GlowPixels(ix, iy, l64);
				}
			}

		//-- Node ID -----------------------------------

		} else {
			for ( ix = 0; ix < srcmap->Width();	ix++,l64++)	{					
				if (nodbuf[gpix+ix] == data.node) {
					GlowPixels(ix, iy, l64);
				}
			}
		}
	}

	result = TRUE;

	done:	
			
	if (line64)
		free(line64);
		
	return(result);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::GlowPixels()

void ImageFilter_Glow::GlowPixels(int sx, int sy, BMM_Color_64 *curPix) {
	BMM_Color_64 glowColor, g64;
	float glowBrite, fTmp;	
	int glowRad, sy2, sx2, gx, gy;
	int glowRadSqr, curRadSqr;	

	glowRad = data.size;
	glowRadSqr = glowRad * glowRad;					
	if(data.colorsrc == IDC_MTLCOLOR_BUTT)						
		glowColor = *curPix;
	else {
		glowColor = data.color;	
		glowColor.a = curPix->a;
	}
	for(sy2 = -glowRad; sy2 <= glowRad; sy2++) {
		for(sx2 = -glowRad; sx2 <= glowRad; sx2++) {
			curRadSqr = (sy2*sy2) + (sx2*sx2);
			if(curRadSqr > 0 && curRadSqr <= glowRadSqr) {								
				if((gy=sy+sy2) > -1 && gy < srcmap->Height()) {
					if((gx=sx+sx2) > -1 && gx < srcmap->Width()) {
						glowBrite = (float)(glowRadSqr-curRadSqr)/glowRadSqr;											
						fTmp = glowBrite*glowBrite*(3.0f-2.0f*glowBrite);											
						glowBrite = fTmp*fTmp*0.01f;										
						srcmap->GetLinearPixels(gx,gy,1,&g64);
						g64.r = min(g64.r + WORD(glowColor.r*glowBrite),0xFFFF);
						g64.g = min(g64.g + WORD(glowColor.g*glowBrite),0xFFFF);
						g64.b = min(g64.b + WORD(glowColor.b*glowBrite),0xFFFF);												
						g64.a = min(g64.a + WORD(glowColor.a*glowBrite),0xFFFF);										
						srcmap->PutPixels(gx,gy,1,&g64);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::LoadConfigure()

BOOL ImageFilter_Glow::LoadConfigure ( void *ptr ) {
     GLOWDATA *buf = (GLOWDATA *)ptr;
     if (buf->version == GLOWVERSION) {
        memcpy((void *)&data,ptr,sizeof(GLOWDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::SaveConfigure()

BOOL ImageFilter_Glow::SaveConfigure ( void *ptr ) {
     if (ptr) {
        memcpy(ptr,(void *)&data,sizeof(GLOWDATA));
		return (TRUE);
	} else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::EvaluateConfigure()

DWORD ImageFilter_Glow::EvaluateConfigure ( ) {
	return (sizeof(GLOWDATA));
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Glow::ChannelsRequired()

DWORD ImageFilter_Glow::ChannelsRequired ( ) {

	if (data.type == IDC_MTLID_BUTT)
		return(BMM_CHAN_MTL_ID);
	else
		return(BMM_CHAN_NODE_ID);

}

//--	EOF: Glow.cpp --------------------------------------------------------
