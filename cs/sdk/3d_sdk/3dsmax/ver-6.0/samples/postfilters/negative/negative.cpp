//-----------------------------------------------------------------------------
// -----------------------
// File ....: Negative.cpp
// -----------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Negative Image Filter
//
// History .: Sep, 07 1995 - Started
//            
//-----------------------------------------------------------------------------
		 
//-- Include files

#include <Max.h>
#include <tvnode.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Negative.h"
#include "resource.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;
static BOOL controlsInit = FALSE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			if (hInst)
				return(FALSE);
			hInst = hDLLInst;
			if ( !controlsInit ) {
				controlsInit = TRUE;
				InitCustomControls(hInst);
			}
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

//-----------------------------------------------------------------------------
//-- Helper

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Negative Class Description

class NEGClassDesc:public ClassDesc {
	  
	  public:

		 int             IsPublic     ( )                   { return 1;                }
		 void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_Negative; }
		 const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_NEGATIVE);     }
		 SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
		 Class_ID        ClassID      ( )                   { return Class_ID(NEGATIVECLASSID,0);    }
		 const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static NEGClassDesc NEGDesc;

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
		 case  0: return &NEGDesc; break;
		 default: return 0;        break;
	  }
}

DLLEXPORT ULONG LibVersion ( )  { 
	  return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

const TCHAR *ImageFilter_Negative::Description( ) 
{
	return GetString(IDS_DB_NEGATIVE);
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
// *> ControlCtrlDlgProc()
//

INT_PTR CALLBACK ControlCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static ImageFilter_Negative *f = NULL;
	if (message == WM_INITDIALOG) 
		f = (ImageFilter_Negative *)lParam;
	if (f) 
		return (f->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Negative::ShowControl()

BOOL ImageFilter_Negative::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_NEG_CONTROL),
		hWnd,
		(DLGPROC)ControlCtrlDlgProc,
		(LPARAM)this));
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Negative::Control()

BOOL ImageFilter_Negative::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	//-- Undo State Flag
	
	static bool hold;
	static bool undo;

	switch (message) {
		
		//------------------------------------------------------------
		//-- Init ----------------------------------------------------

		case WM_INITDIALOG: {
				
				hold = false;	
				undo = false;

				//-- Make Dialogue Interactive
				
				MakeDlgInteractive(hWnd);
				
				//-- Center it

				HWND hWndParent = TheManager->AppWnd();
				CenterWindow(hWnd,hWndParent);
				
				//-- Setup User Interface

				FilterUpdate();
				float blend = DEFAULT_BLEND_VALUE;
				if (Node()) {
					undonotify = new UndoNotify(hWnd);
					if (undonotify)
						Node()->RegisterTVNodeNotify((TVNodeNotify *)undonotify);
					blendcontrol = Node()->GetController(BLEND_CLASS_ID);
					if (blendcontrol) {
						Interval valid = Max()->GetAnimRange();
						blendcontrol->GetValue(Max()->GetTime(),&blend,valid);
					}
				}

				ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
				if (blendspin) {
					blendspin->LinkToEdit(GetDlgItem(hWnd,IDC_NEG_BLEND_ED),EDITTYPE_FLOAT);
					blendspin->SetLimits(0.0f,100.0f,FALSE);
					blendspin->SetValue(blend,FALSE);
				}

			}
			return 1;

		//------------------------------------------------------------
		//-- Undo ----------------------------------------------------
		//
		//   The filter manager sends this message (if you register
		//   for the notification as in above with RegisterTVNodeNotify())
		//   and an undo operation was performed. Instead of updating the 
		//	 controls here, we just set a flag and wait for a paint message.
				  
		case FLT_UNDO:
			undo = true;
			break;

		//------------------------------------------------------------
		//-- Paint ---------------------------------------------------
				  
		case WM_PAINT:
			if (undo) {
				float blend = DEFAULT_BLEND_VALUE;
				if (Node()) {
					blendcontrol = Node()->GetController(BLEND_CLASS_ID);
					if (blendcontrol) {
						Interval valid = Max()->GetAnimRange();
						blendcontrol->GetValue(Max()->GetTime(),&blend,valid);
					}
				}

				ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
				if (blendspin) {
					blendspin->SetValue(blend,FALSE);
				}
				undo = false;
			}
			break;

		//------------------------------------------------------------
		//-- Spinners ------------------------------------------------
				  
		case CC_SPINNER_CHANGE:
			if ( LOWORD(wParam) == IDC_NEG_BLEND_SP ) {
				if (!hold) {
					theHold.Begin();
					hold = true;
				}
				ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
				if (blendspin) {
					float blend = blendspin->GetFVal();
					if (Node()) {
						blendcontrol = Node()->GetController(BLEND_CLASS_ID);
						if (blendcontrol) {
							Interval valid = Max()->GetAnimRange();
							blendcontrol->SetValue(Max()->GetTime(),&blend);
						}
					}
				}
			}
			break;

		case CC_SPINNER_BUTTONDOWN:
			if ( LOWORD(wParam) == IDC_NEG_BLEND_SP ) {
				theHold.Begin();
				hold = true;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			if ( LOWORD(wParam) == IDC_NEG_BLEND_SP ) {
				if (hold) {
					if (!HIWORD(wParam))
						theHold.Cancel();
					else
						theHold.Accept(GetString(IDS_BLEND_VALUE));
					hold = false;
				}
			}
			break;
			
		case WM_CUSTEDIT_ENTER:
			if ((LOWORD(wParam) == IDC_NEG_BLEND_ED) && hold) {
				theHold.Accept(GetString(IDS_BLEND_VALUE));
				hold = false;
			}
			break;

		//------------------------------------------------------------
		//-- Time Change Notification --------------------------------
				  
		case FLT_TIMECHANGED: {
				ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
				if (blendspin) {
					float blend = DEFAULT_BLEND_VALUE;
					if (Node()) {
						blendcontrol = Node()->GetController(BLEND_CLASS_ID);
						if (blendcontrol) {
							Interval valid = Max()->GetAnimRange();
							blendcontrol->GetValue((TimeValue)lParam,&blend,valid);
							ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
							if (blendspin) {
								blendspin->SetValue(blend,FALSE);
								blendspin->SetKeyBrackets(blendcontrol->IsKeyAtTime(Max()->GetTime(),0));
							}
						}
					}
				}
			}
			break;

		//------------------------------------------------------------
		//-- Buttons -------------------------------------------------
				  
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

		//------------------------------------------------------------
		//-- Kaput ---------------------------------------------------
				  
		case WM_DESTROY: {
				ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,IDC_NEG_BLEND_SP));
				if (blendspin)
					ReleaseISpinner(blendspin);
				if (Node() && undonotify) {
					Node()->UnRegisterTVNodeNotify((TVNodeNotify *)undonotify);
					delete undonotify;
					undonotify = NULL;
				}
			}
			break;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Negative::FilterUpdate()

void ImageFilter_Negative::FilterUpdate() {
	if (!CreateNode())
		return;
	if (Node()->FindItem(BLEND_CLASS_ID) == -1) {
		float value = DEFAULT_BLEND_VALUE;
		blendcontrol = NewDefaultFloatController();
		if (blendcontrol) {
			blendcontrol->SetValue(0,&value,1);
			Node()->AddController(blendcontrol,GetString(IDS_BLEND),BLEND_CLASS_ID);
		}
	}
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Negative::ShowAbout()

void ImageFilter_Negative::ShowAbout(HWND hWnd) {
	  DialogBoxParam(
		 hInst,
		 MAKEINTRESOURCE(IDD_NEG_ABOUT),
		 hWnd,
		 (DLGPROC)AboutCtrlDlgProc,
		 (LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Negative::Render()
//
//    Private member Bitmap *srcmap has source bitmap
//    Private member Bitmap *mskmap has mask (if any)

BOOL ImageFilter_Negative::Render(HWND hWnd) {

	BMM_Color_64 *l64,*line64 = NULL;
	WORD         *m16,*mask16 = NULL;
	
	int mwidth,r,g,b,a;

	if (!srcmap)
		return (FALSE);

	if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	if (mskmap) {
		mask16 = (WORD *)calloc(srcmap->Width(),sizeof(WORD));
		mwidth = min(srcmap->Width(),mskmap->Width());
	}  

	BOOL result = TRUE;
	BOOL abort  = FALSE;
	  
	//-- Get controler value

	float blend = DEFAULT_BLEND_VALUE;

	if (Node()) {

		Interval valid;
		int tpf = GetTicksPerFrame();
		valid.Set(ifi->FilterRange.First() * tpf, ifi->FilterRange.Last() * tpf);
		TimeValue now = (TimeValue)(ifi->FilterRange.Current() * tpf);
		blendcontrol = Node()->GetController(BLEND_CLASS_ID);
		if (blendcontrol)
			blendcontrol->GetValue(now,&blend,valid);

	}

	blend = 1.0f - (blend / 100.0f);

	//-- Mess up the image

	for (int iy = 0; iy < srcmap->Height(); iy++) {

		//-- Progress Report
		 
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

		//-- Check for Abort
		 
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

		if (abort) {
			result = FALSE;
			break;
		}   
		 
		if (blend == 0.0f)
			continue;

		//-- Get line
		 
		l64 = line64;

		if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

		//-- Handle Mask
		 
		if (mask16) {

			if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
				result = FALSE;
				break;
			}
			
			m16 = mask16;

			for (int ix = 0; ix < srcmap->Width(); ix++,l64++,m16++) {

				r = 65535 - l64->r;
				g = 65535 - l64->g;
				b = 65535 - l64->b;
				a = 65535 - l64->a;

				if (*m16 < 65530) {
					
					if (*m16) {
						r = Lerp(r,(int)l64->r,(int)*m16);
						g = Lerp(g,(int)l64->g,(int)*m16);
						b = Lerp(b,(int)l64->b,(int)*m16);
						a = Lerp(a,(int)l64->a,(int)*m16);
					}
					
					if (blend < 1.0f) {
						/*l64->r = r;
						l64->g = g;
						l64->b = b;
						l64->a = a;
					} else {*/
						l64->r = Lerp(l64->r,r,blend);
						l64->g = Lerp(l64->g,g,blend);
						l64->b = Lerp(l64->b,b,blend);
						l64->a = Lerp(l64->a,a,blend);
					}
					
				}
			}
			
		//-- Unmasked
		 
		} else {
		 
			if (blend < 1.0f) {

				for (int ix = 0; ix < srcmap->Width(); ix++,l64++) {
					r = 65535 - l64->r;
					g = 65535 - l64->g;
					b = 65535 - l64->b;
					a = 65535 - l64->a;
					l64->r = Lerp(l64->r,r,blend);
					l64->g = Lerp(l64->g,g,blend);
					l64->b = Lerp(l64->b,b,blend);
					l64->a = Lerp(l64->a,a,blend);
				}

			} else {

				for (int ix = 0; ix < srcmap->Width(); ix++,l64++) {
					l64->r = 65535 - l64->r;
	            	l64->g = 65535 - l64->g;
	            	l64->b = 65535 - l64->b;
	            	l64->a = 65535 - l64->a;
				}

			}
			
		}

		//-- Output Line
		 
		if (srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

	}
	  
	if (mask16)
		free(mask16);
		 
	if (line64)
		free(line64);
		 
	return(result);

}

//-- EOF: negative.cpp --------------------------------------------------------


































