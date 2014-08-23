//-----------------------------------------------------------------------------
// ----------------------
// File ....: PMAlpha.cpp
// ----------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Pre-Multiplied Alpha Compositor
//
// History .: Sep 27 1995 - Started
//            Apr 09 1997 - Added G Channel Support (GG)
//            Aug 19 1997 - Added Blur's Code
//
// Note ....: (c) 1996 Steven C. Blackmon / Blur Studio Inc.
//            
//-----------------------------------------------------------------------------
        
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "PMAlpha.h"
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

//-----------------------------------------------------------------------------
// Helper

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// PMAlpha Class Description

class PMALPHAClassDesc:public ClassDesc {
     
     public:

        int             IsPublic     ( )                   { return 1;                }
        void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_PMAlpha; }
        const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_PMALPHA);     }
        SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
        Class_ID        ClassID      ( )                   { return Class_ID(ALPHACLASSID,0);    }
        const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static PMALPHAClassDesc PMALPHADesc;

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
        case  0: return &PMALPHADesc; break;
        default: return 0;        break;
     }
}

DLLEXPORT ULONG LibVersion ( )  { 
     return ( VERSION_3DSMAX ); 
}

const TCHAR *ImageFilter_PMAlpha::Description( ) {
	return GetString(IDS_DB_PMALPHA_COMP);
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
// #> ImageFilter_PMAlpha::ShowAbout()

void ImageFilter_PMAlpha::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_PMALPHA_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)this);
}

//-----------------------------------------------------------------------------
// *> fLerp()
//

static _inline float fLerp(float a, float b, int i) {
   float f = i / 65535.0f;
   if (f > 1.0f) f = 1.0f;
   return((1.0f-f)*a + f*b);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_PMAlpha::Render()
//
//    Private member Bitmap *srcmap has source bitmap
//    Private member Bitmap *frgmap has foreground bitmap
//    Private member Bitmap *mskmap has mask (if any)
//

BOOL ImageFilter_PMAlpha::Render(HWND hWnd) {

	BMM_Color_64 *s64,*src64	= NULL;
	BMM_Color_64 *f64,*fgd64	= NULL;
	WORD         *m16,*mask16	= NULL;

	float bak_r, for_r, res_r;
	float bak_g, for_g, res_g;
	float bak_b, for_b, res_b;
	float bak_a, for_a, res_a;
	float i_for_a;

	int mwidth;
     
	if (!srcmap)
		return (FALSE);

	if (!frgmap)
		return (FALSE);

	int width	= srcmap->Width();
	int height	= srcmap->Height();

	//-- Allocate Background Line
     
	if ((src64=(BMM_Color_64 *)calloc(width,sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	//-- Allocate Foreground Line
     
	if ((fgd64=(BMM_Color_64 *)calloc(width,sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	//-- Allocate Mask Line
     
	if (mskmap) {
		mask16 = (WORD *)calloc(width,sizeof(WORD));
		mwidth = min(width,mskmap->Width());
	}  

	BOOL result = TRUE;
	BOOL abort  = FALSE;
     
	//----------------------------------------------------------------
	//-- Handle G Channels (if any)

	DWORD src_channels = srcmap->ChannelsPresent();
	DWORD frg_channels = frgmap->ChannelsPresent();

	if (frg_channels || src_channels) {

		//------------------------------------------------------------
		//-- Z Channel
				
		if (frg_channels & BMM_CHAN_Z && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_Z))
				srcmap->CreateChannels(BMM_CHAN_Z);
		
			DWORD type;
			float *sbuffer = (float *)srcmap->GetChannel(BMM_CHAN_Z,type);
			float *fbuffer = (float *)frgmap->GetChannel(BMM_CHAN_Z,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_Z));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						if (*m16 > 0x7FFF) {
							if (f64->a > 0x7FFF)
								*sbuffer = *fbuffer;
						}
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						if (f64->a > 0x7FFF)
							*sbuffer = *fbuffer;
					}
				}

			}

		}

		//------------------------------------------------------------
		//-- Material Effects Channel

		if (frg_channels & BMM_CHAN_MTL_ID && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_MTL_ID))
				srcmap->CreateChannels(BMM_CHAN_MTL_ID);
		
			DWORD type;
			BYTE *sbuffer = (BYTE *)srcmap->GetChannel(BMM_CHAN_MTL_ID,type);
			BYTE *fbuffer = (BYTE *)frgmap->GetChannel(BMM_CHAN_MTL_ID,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_MTLID));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						if (*m16 > 0x7FFF) {
							if (f64->a > 0x7FFF)
								*sbuffer = *fbuffer;
						}
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						if (f64->a > 0x7FFF)
							*sbuffer = *fbuffer;
					}
				}

			}

		}

		//------------------------------------------------------------
		//-- Node ID Channel

		if (frg_channels & BMM_CHAN_NODE_ID && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_NODE_ID))
				srcmap->CreateChannels(BMM_CHAN_NODE_ID);
		
			DWORD type;
			WORD *sbuffer = (WORD *)srcmap->GetChannel(BMM_CHAN_NODE_ID,type);
			WORD *fbuffer = (WORD *)frgmap->GetChannel(BMM_CHAN_NODE_ID,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_NODEID));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						if (*m16 > 0x7FFF) {
							if (f64->a > 0x7FFF)
								*sbuffer = *fbuffer;
						}
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						if (f64->a > 0x7FFF)
							*sbuffer = *fbuffer;
					}
				}

			}

		}

		//------------------------------------------------------------
		//-- UV Coordinates Channel


		if (frg_channels & BMM_CHAN_UV && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_UV))
				srcmap->CreateChannels(BMM_CHAN_UV);
		
			DWORD type;
			Point2 *sbuffer = (Point2 *)srcmap->GetChannel(BMM_CHAN_UV,type);
			Point2 *fbuffer = (Point2 *)frgmap->GetChannel(BMM_CHAN_UV,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_UV));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						if (*m16 > 0x7FFF) {
							if (f64->a > 0x7FFF)
								*sbuffer = *fbuffer;
						}
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						if (f64->a > 0x7FFF)
							*sbuffer = *fbuffer;
					}
				}

			}

		}

		//------------------------------------------------------------
		//-- Normals Channel

		if (frg_channels & BMM_CHAN_NORMAL && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_NORMAL))
				srcmap->CreateChannels(BMM_CHAN_NORMAL);
		
			DWORD type;
			DWORD *sbuffer = (DWORD *)srcmap->GetChannel(BMM_CHAN_NORMAL,type);
			DWORD *fbuffer = (DWORD *)frgmap->GetChannel(BMM_CHAN_NORMAL,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_NORMAL));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						if (*m16 > 0x7FFF) {
							if (f64->a > 0x7FFF)
								*sbuffer = *fbuffer;
						}
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						if (f64->a > 0x7FFF)
							*sbuffer = *fbuffer;
					}
				}

			}

		}

		//------------------------------------------------------------
		//-- Real Colors Channel

		if (frg_channels & BMM_CHAN_REALPIX && !abort && result) {
		
			if (!(src_channels & BMM_CHAN_REALPIX))
				srcmap->CreateChannels(BMM_CHAN_REALPIX);
		
			DWORD type;
			RealPixel *sbuffer = (RealPixel *)srcmap->GetChannel(BMM_CHAN_REALPIX,type);
			RealPixel *fbuffer = (RealPixel *)frgmap->GetChannel(BMM_CHAN_REALPIX,type);

			//-- Display what's going on

			SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_REALPIX));

			for (int iy = 0; iy < height; iy++) {
				
				//-- Progress Report
        
				SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

				//-- Check for Abort
        
				SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

				if (abort) {
					result = FALSE;
					break;
				}   
        
				f64 = fgd64;

				//-- Get Foreground line (RGBA Image)
        
				if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
					result = FALSE;
					break;
				}

				//-- Handle Mask -------------------------------------
		        
				if (mask16) {
					
					if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
						result = FALSE;
						break;
					}
		           
					m16 = mask16;
					for (int ix = 0; ix < width; ix++,f64++,m16++,sbuffer++,fbuffer++) {
						float sr,sg,sb;
						float fr,fg,fb;
						float tr,tg,tb;
						ExpandRealPixel(*sbuffer,sr,sg,sb);
						ExpandRealPixel(*fbuffer,fr,fg,fb);
						for_a = ((float)f64->a / 65530.0f);
						i_for_a = 1.0f - for_a;
						tr = (sr * i_for_a) + fr;
						tg = (sg * i_for_a) + fg;
						tb = (sb * i_for_a) + fb;
						if (*m16 < 65530) {
							if (*m16) {
								sr = fLerp(tr,sr,*m16);
								sg = fLerp(tg,sg,*m16);
								sb = fLerp(tb,sb,*m16);
							} else {
								sr = tr;
								sg = tg;
								sb = tb;
							}
						}
						*sbuffer = MakeRealPixel(sr,sg,sb);
					}

				//-- Unmasked ----------------------------------------
		
				} else {
					for (int ix = 0; ix < width; ix++,f64++,sbuffer++,fbuffer++) {
						float sr,sg,sb;
						float fr,fg,fb;
						ExpandRealPixel(*sbuffer,sr,sg,sb);
						ExpandRealPixel(*fbuffer,fr,fg,fb);
						for_a = ((float)f64->a / 65530.0f);
						i_for_a = 1.0f - for_a;
						sr = (sr * i_for_a) + fr;
						sg = (sg * i_for_a) + fg;
						sb = (sb * i_for_a) + fb;
						*sbuffer = MakeRealPixel(sr,sg,sb);
					}
				}

			}

		}

	}

	//----------------------------------------------------------------
	//-- Handle RGBA Image

	if (!abort && result) {

		//-- Display what's going on

		SendMessage(hWnd,FLT_TEXTMSG,0,(LPARAM)GetString(IDS_PROCESS_RGBA));

		for (int iy = 0; iy < height; iy++) {

			//-- Progress Report
	        
			SendMessage(hWnd,FLT_PROGRESS,iy,height-1);

			//-- Check for Abort
	        
			SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

			if (abort) {
				result = FALSE;
				break;
			}   
	        
			s64 = src64;
			f64 = fgd64;

			//-- Get Source line
	        
			if (srcmap->GetLinearPixels(0,iy,width,src64)!=1) {
				result = FALSE;
				break;
			}

			//-- Get Foreground line
	        
			if (frgmap->GetLinearPixels(0,iy,width,fgd64)!=1) {
				result = FALSE;
				break;
			}

			//-- Handle Mask ----------------------------------
	        
			if (mask16) {
				
				WORD r,g,b,a;
	           
				if (mskmap->Get16Gray(0,iy,mwidth,mask16)!=1) {
					result = FALSE;
					break;
				}
	           
				m16 = mask16;
				for (int ix = 0; ix < width; ix++,s64++,f64++,m16++) {

					bak_r = ((float)s64->r / 65530.0f);
					bak_g = ((float)s64->g / 65530.0f);
					bak_b = ((float)s64->b / 65530.0f);
					bak_a = ((float)s64->a / 65530.0f);

					for_r = ((float)f64->r / 65530.0f);
					for_g = ((float)f64->g / 65530.0f);
					for_b = ((float)f64->b / 65530.0f);
					for_a = ((float)f64->a / 65530.0f);

					i_for_a = 1.0f - for_a;

					res_r = (bak_r * i_for_a) + for_r;
					res_g = (bak_g * i_for_a) + for_g;
					res_b = (bak_b * i_for_a) + for_b;
					res_a = bak_a + for_a;

					res_r = min(max(res_r,0.0f),1.0f);
					res_g = min(max(res_g,0.0f),1.0f);
					res_b = min(max(res_b,0.0f),1.0f);
					res_a = min(max(res_a,0.0f),1.0f);
					
					r = (WORD)(res_r * 65530.0f);
					g = (WORD)(res_g * 65530.0f);
					b = (WORD)(res_b * 65530.0f);
					a = (WORD)(res_a * 65530.0f);

					if (*m16 < 65530) {
						if (*m16) {
							s64->r = Lerp(r,(int)s64->r,(int)*m16);
							s64->g = Lerp(g,(int)s64->g,(int)*m16);
							s64->b = Lerp(b,(int)s64->b,(int)*m16);
							s64->a = Lerp(a,(int)s64->a,(int)*m16);
						} else {
							s64->r = r;
							s64->g = g;
							s64->b = b;
							s64->a = a;
						}
					}
	           
				}
	           
			//-- Unmasked -------------------------------------
	        
			} else {
	        
				for (int ix = 0; ix < width; ix++,f64++,s64++) {


					bak_r = ((float)s64->r / 65530.0f);
					bak_g = ((float)s64->g / 65530.0f);
					bak_b = ((float)s64->b / 65530.0f);
					bak_a = ((float)s64->a / 65530.0f);

					for_r = ((float)f64->r / 65530.0f);
					for_g = ((float)f64->g / 65530.0f);
					for_b = ((float)f64->b / 65530.0f);
					for_a = ((float)f64->a / 65530.0f);

					i_for_a = 1.0f - for_a;

					res_r = (bak_r * i_for_a) + for_r;
					res_g = (bak_g * i_for_a) + for_g;
					res_b = (bak_b * i_for_a) + for_b;
					res_a = bak_a + for_a;

					res_r = min(max(res_r,0.0f),1.0f);
					res_g = min(max(res_g,0.0f),1.0f);
					res_b = min(max(res_b,0.0f),1.0f);
					res_a = min(max(res_a,0.0f),1.0f);
				
					s64->r = (WORD)(res_r * 65530.0f);
					s64->g = (WORD)(res_g * 65530.0f);
					s64->b = (WORD)(res_b * 65530.0f);
					s64->a = (WORD)(res_a * 65530.0f);

				}

	        }

	        //-- Output Line
	        
	        if (srcmap->PutPixels(0,iy,width,src64)!=1) {
	           result = FALSE;
	           break;
	        }

		}

	}
     
	if (mask16)
		free(mask16);
        
	if (src64)
		free(src64);
        
	if (fgd64)
		free(fgd64);
        
	return(result);

}

//-- EOF: PMAlpha.cpp --------------------------------------------------------
