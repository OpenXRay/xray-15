//-----------------------------------------------------------------------------
// -------------------
// File	....:	stars.cpp
// -------------------
// Author...:	Tom Hudson
// Date	....:	April 1996
// Descr....:	Stars Image Filter
//
// History	.:	April 25 1996 -	Started
//				
//-----------------------------------------------------------------------------
		
//--	Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include <maxapi.h>
#include "resource.h"
#include "stars.h"
#include "pixelbuf.h"

//--	Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

static ISpinnerControl	*dimspin = NULL;
static ISpinnerControl	*britespin = NULL;
static ISpinnerControl	*sizespin = NULL;
static ISpinnerControl	*amtspin = NULL;
static ISpinnerControl	*dimmingspin = NULL;
static ISpinnerControl	*seedspin = NULL;
static ISpinnerControl	*countspin = NULL;

//--    Handy Stuff --------------------------------------------------------------

#define MAXIV 0xff00	// 255 * 256
static BMM_Color_48 white = {MAXIV,MAXIV,MAXIV};

#define MAX(a,b) ((a>b) ? a:b)

/* Some trig defines */

#define PI_45 0.7853981f		/* Some angles in radians */
#define PI_90 1.5707963f
#define PI_135 2.3561945f
#define PI_180 3.1415927f
#define PI_225 3.9269908f
#define PI_270 4.712389f
#define PI_315 5.4977871f
#define PI_360 6.2831853f

#define RAD_DEG 57.29578f	/* Radians -> degrees conversion */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--	DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)	{
	switch (fdwReason) {
		 case	DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hDLLInst;
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

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Stars	Class	Description

class STARSClassDesc:public ClassDesc {
	
	public:

		int				IsPublic ( )					{ return	1; }
		void			*Create (BOOL loading=FALSE)	{ return	new ImageFilter_Stars; }
		const	TCHAR	*ClassName ( )					{ return	GetString(IDS_STARS);	}
		SClass_ID		SuperClassID ( )				{ return	FLT_CLASS_ID; }
		Class_ID		ClassID	( )						{ return	STARS_CLASS_ID; }
		const	TCHAR	*Category ( )					{ return	GetString(IDS_IMAGE_FILTER); }

};

static STARSClassDesc STARSDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription (	) { 
 	return GetString(IDS_STARS_FILTER); 
}

DLLEXPORT int	LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc	*LibClassDesc(int	i) {
	switch(i) {
		case 0:		return &STARSDesc;	break;
		default:	return 0;			break;
	}
}

DLLEXPORT ULONG LibVersion (	)	{ 
	return (VERSION_3DSMAX);	
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::ImageFilter_Stars()
//

ImageFilter_Stars::ImageFilter_Stars() {
	data.version = STARSVERSION;
	data.camera[0] = 0;
	data.dimmest = 0;
	data.brightest = 255;
	data.briteType = BRITE_LINEAR;
	data.size = 1.5f;
	data.useBlur = TRUE;
	data.blurAmount = 75;
	data.blurDim = 40;
	data.dataType = DATA_RANDOM;
	data.seed = 12345;
	data.count = 15000;
	strcpy(data.filename,"earth.stb");
	data.compositing = COMP_BACK;
	stardata = NULL;
	pos1 = pos2 = NULL;
	}

void ImageFilter_Stars::FreeData() {
	if(stardata) {
		delete [] stardata;
		stardata = NULL;
		}
	if(pos1) {
		delete [] pos1;
		pos1 = NULL;
		}
	if(pos2) {
		delete [] pos2;
		pos2 = NULL;
		}
	}

void ImageFilter_Stars::EnableStarControls(HWND hWnd) {
	BOOL random = (data.dataType == DATA_RANDOM) ? TRUE : FALSE;
	if(random) {
		seedspin->Enable();
		countspin->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_CUST_FILENAME), FALSE);
		}
	else {
		seedspin->Disable();
		countspin->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_CUST_FILENAME), TRUE);
		}
	}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::Control()
//

BOOL ImageFilter_Stars::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{

	switch (message) {

		case WM_INITDIALOG:	{
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			// Get the cameras in the scene and load 'em into the listbox
			LoadCameras(hWnd);
						
			CheckRadioButton(
				hWnd,
				IDC_LINEAR,
				IDC_LOG,
				data.briteType == BRITE_LINEAR ? IDC_LINEAR : IDC_LOG
			);
			
			CheckRadioButton(
				hWnd,
				IDC_RANDOM,
				IDC_CUSTOM,
				data.dataType == DATA_RANDOM ? IDC_RANDOM : IDC_CUSTOM
			);

			CheckDlgButton(hWnd, IDC_USEBLUR, data.useBlur);
			
			dimspin	= GetISpinner(GetDlgItem(hWnd, IDC_DIM_SPIN));
			dimspin->LinkToEdit( GetDlgItem(hWnd,IDC_DIM_EDIT), EDITTYPE_INT );
			dimspin->SetLimits(	0,255, FALSE );
			dimspin->SetValue(data.dimmest,FALSE);

			britespin	= GetISpinner(GetDlgItem(hWnd, IDC_BRITE_SPIN));
			britespin->LinkToEdit( GetDlgItem(hWnd,IDC_BRITE_EDIT), EDITTYPE_INT );
			britespin->SetLimits(	0,255, FALSE );
			britespin->SetValue(data.brightest,FALSE);

			sizespin	= GetISpinner(GetDlgItem(hWnd, IDC_SIZE_SPIN));
			sizespin->LinkToEdit( GetDlgItem(hWnd,IDC_SIZE_EDIT), EDITTYPE_FLOAT );
			sizespin->SetLimits(	0.001f, 100.0f, FALSE );
			sizespin->SetValue(data.size,FALSE);

			amtspin	= GetISpinner(GetDlgItem(hWnd, IDC_AMOUNT_SPIN));
			amtspin->LinkToEdit( GetDlgItem(hWnd,IDC_AMOUNT_EDIT), EDITTYPE_INT );
			amtspin->SetLimits(	0,100, FALSE );
			amtspin->SetValue(data.blurAmount,FALSE);
			
			dimmingspin	= GetISpinner(GetDlgItem(hWnd, IDC_DIMMING_SPIN));
			dimmingspin->LinkToEdit( GetDlgItem(hWnd,IDC_DIMMING_EDIT), EDITTYPE_INT );
			dimmingspin->SetLimits(	0,100, FALSE );
			dimmingspin->SetValue(data.blurDim,FALSE);

			seedspin	= GetISpinner(GetDlgItem(hWnd, IDC_SEED_SPIN));
			seedspin->LinkToEdit( GetDlgItem(hWnd,IDC_SEED_EDIT), EDITTYPE_INT );
			seedspin->SetLimits(0, 99999, FALSE );
			seedspin->SetValue(data.seed, FALSE);

			countspin	= GetISpinner(GetDlgItem(hWnd, IDC_COUNT_SPIN));
			countspin->LinkToEdit( GetDlgItem(hWnd,IDC_COUNT_EDIT), EDITTYPE_INT );
			countspin->SetLimits(1, 1000000, FALSE );
			countspin->SetValue(data.count,FALSE);

			SetDlgItemText(hWnd, IDC_CUST_FILENAME, data.filename);

			EnableStarControls(hWnd);

			CheckRadioButton(
				hWnd,
				IDC_BACK,
				IDC_FORE,
				data.compositing == COMP_BACK ? IDC_BACK : IDC_FORE
				);
			}
			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDC_CAMERAS: {
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE: {
							int selection = SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_GETCURSEL, 0, 0);
							if(selection != CB_ERR) {
								SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_GETLBTEXT, selection, (LPARAM)data.camera);
								}
							}
							break;
						}
					}
					break;

				case IDC_RANDOM:
				case IDC_CUSTOM:
					data.dataType = IsDlgButtonChecked(hWnd,IDC_RANDOM) ? DATA_RANDOM : DATA_CUSTOM;
					EnableStarControls(hWnd);
					break;
					
				case IDOK: {
					data.dimmest = dimspin->GetIVal();
					data.brightest = britespin->GetIVal();
					data.briteType = IsDlgButtonChecked(hWnd,IDC_LINEAR) ? BRITE_LINEAR : BRITE_LOG;
					data.size = sizespin->GetFVal();
					data.useBlur = IsDlgButtonChecked(hWnd,IDC_USEBLUR) ? TRUE : FALSE;
					data.blurAmount = amtspin->GetIVal();
					data.blurDim = dimmingspin->GetIVal();
					data.dataType = IsDlgButtonChecked(hWnd,IDC_RANDOM) ? DATA_RANDOM : DATA_CUSTOM;
					data.seed= seedspin->GetIVal();					
					data.count= countspin->GetIVal();
					GetDlgItemText(hWnd, IDC_CUST_FILENAME, data.filename, 255);
					data.compositing = IsDlgButtonChecked(hWnd,IDC_BACK) ? COMP_BACK : COMP_FORE;
					int selection = SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_GETCURSEL, 0, 0);
					if(selection != CB_ERR) {
						SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_GETLBTEXT, selection, (LPARAM)data.camera);
						}
					data.compositing = IsDlgButtonChecked(hWnd,IDC_BACK) ? COMP_BACK : COMP_FORE;
					EndDialog(hWnd,1);
					}
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;

			}

			return 1;

		case WM_DESTROY:
			  if (dimspin) {
				  ReleaseISpinner(dimspin);
				  dimspin = NULL;
			  }
			  if (britespin) {
				  ReleaseISpinner(britespin);
				  britespin = NULL;
			  }
			  if (sizespin) {
				  ReleaseISpinner(sizespin);
				  sizespin = NULL;
			  }
			  if (amtspin) {
				  ReleaseISpinner(amtspin);
				  amtspin = NULL;
			  }
			  if (dimmingspin) {
				  ReleaseISpinner(dimmingspin);
				  dimmingspin = NULL;
			  }
			  if (seedspin) {
				  ReleaseISpinner(seedspin);
				  seedspin = NULL;
			  }
			  if (countspin) {
				  ReleaseISpinner(countspin);
				  countspin = NULL;
			  }
			  break;

	}

	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlDlgProc()
//

INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
     static ImageFilter_Stars *flt = NULL;
     if (message == WM_INITDIALOG) 
        flt = (ImageFilter_Stars *)lParam;
     if (flt) 
        return (flt->Control(hWnd,message,wParam,lParam));
     else
        return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::ShowControl()

BOOL ImageFilter_Stars::ShowControl(HWND hWnd) {
	InitCustomControls(hInst);
     return (DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_STARS_CONTROL),
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
// #> ImageFilter_Stars::ShowAbout()

void ImageFilter_Stars::ShowAbout(HWND hWnd)	{
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_STARS_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)this);
}

static void
RemoveScaling(Matrix3 &m) {
	for (int i=0; i<3; i++)
		m.SetRow(i,Normalize(m.GetRow(i)));
	}

void ImageFilter_Stars::ComputeCamera(INode *node, TimeValue t, CameraInfo *ci) {
	const ObjectState& os = node->EvalWorldState(t);
	
	// compute camera transform
	CameraState cs;
	Interval iv;
	CameraObject *cam = (CameraObject *)os.obj;
	iv.SetInfinite();
	Matrix3 camtm = node->GetObjTMAfterWSM(t,&iv);
	RemoveScaling(camtm);
	camtm.NoTrans();		// Make sure it's centered about the camera!
	ci->tm = Inverse(camtm);
	cam->EvalCameraState(t,iv,&cs);
	ci->fov = cs.fov;  
	}

/* Plot the pixel in the specified color at v luminosity (0-255) */

void ImageFilter_Stars::Plot(int x, int y, int v, BMM_Color_48 *c) {
	UINT r,g,b;
	BMM_Color_64 mappix;

	if((field_type==EVEN_LINES && (y&1)) || (field_type==ODD_LINES && ((y&1)==0)))
	 return;

	if(x<=0 || y<=0 || x>=srcmap->Width() || y>=srcmap->Height())
	 return;

	if (srcmap->GetLinearPixels(x,y,1,&mappix)!=1) {
		assert(0);
		return;
		}

	unsigned int pr,pg,pb;

	if(data.compositing == COMP_BACK && mappix.a != 0) {
		UINT a = mappix.a >> 8;
		// If totally opaque, forget it!
		if(a == 255)
			return;
		UINT ia = 255 - a;
		pr = (UINT)c->r * v * ia / 65025;
		pg = (UINT)c->g * v * ia / 65025;
		pb = (UINT)c->b * v * ia / 65025;
		}
	else {
		pr = (UINT)c->r * v / 255;
		pg = (UINT)c->g * v / 255;
		pb = (UINT)c->b * v / 255;
		}
	r=(UINT)mappix.r+pr;
	g=(UINT)mappix.g+pg;
	b=(UINT)mappix.b+pb;
	mappix.r=(WORD)((r>MAXIV) ? MAXIV:r);
	mappix.g=(WORD)((g>MAXIV) ? MAXIV:g);
	mappix.b=(WORD)((b>MAXIV) ? MAXIV:b);

    srcmap->PutPixels(x,y,1,&mappix);
	}


/*------------------------------------------------------------
Dan's Handy-Dandy Floating-Point Rectancle Plotter (DHDFPRP) (TM)...
	
	Render an anti-aliased floating point rectangle on pixel grid.
	x,y,w,h are floating point screen coords. 

	Tom's modified version: x&y are center of rectangle

	This could be sped up using fixed point.

-------------------------------------------------------------*/

#define frac(z) fmod(z,1.0f)
#define whole(z) (z-fmod(z,1.0f))

void ImageFilter_Stars::FRect(float x, float y, float w, float h,int brite) {
	int xa,xb,ya,yb,i,j;
	float x2,y2,lfrac,rfrac,tfrac,bfrac,xfrac,yfrac;
	x -= (w/2.0f);
	y -= (h/2.0f);
	x2 = x + w;
	y2 = y + h;
  	xa = (int)x;
	xb = (int)x2;
  	ya = (int)y;
	yb = (int)y2;
	lfrac = (float)(1.0-frac(x));	
	rfrac = (float)frac(x2);
	tfrac = (float)(1.0-frac(y));
	bfrac = (float)frac(y2);
	if (xa==xb) {
		xfrac = lfrac+rfrac-1.0f;
		if (ya==yb) { 
			/* --- only one pixel effected */
			Plot(xa,ya,(int)(xfrac*(tfrac+bfrac-1.0f)*brite),&white);	
			}
		else {  
			/* --- column of pixels */
			Plot(xa,ya,(int)(xfrac*tfrac*brite),&white);
			for (j=ya+1; j<yb; j++) Plot(xa,j,(int)(xfrac*brite),&white);
			Plot(xa,yb,(int)(xfrac*bfrac*brite),&white);
			}
		}
	else {
		if (ya==yb) { 
			/* --- row of pixels */
			yfrac = tfrac+bfrac-1.0f;
			Plot(xa,ya,(int)(yfrac*lfrac*brite),&white);
			for (i=xa+1; i<xb; i++) Plot(i,ya,(int)(yfrac*brite),&white);
			Plot(xb,ya,(int)(yfrac*rfrac*brite),&white);
			}
		else {  
			/* general case */

			/* --- top row */
			Plot(xa,ya,(int)(tfrac*lfrac*brite),&white);
			for (i=xa+1; i<xb; i++) Plot(i,ya,(int)(tfrac*brite),&white);
			Plot(xb,ya,(int)(tfrac*rfrac*brite),&white);

			/* --- middle (whole pixels ) */
			for (j=ya+1; j<yb; j++) {
				Plot(xa,j,(int)(lfrac*brite),&white);
				for (i=xa+1; i<xb; i++) Plot(i,j,(int)(brite),&white);
				Plot(xb,j,(int)(rfrac*brite),&white);
				}

			/* --- bottom row */
			Plot(xa,yb,(int)(bfrac*lfrac*brite),&white);
			for (i=xa+1; i<xb; i++) Plot(i,yb,(int)(bfrac*brite),&white);
			Plot(xb,yb,(int)(bfrac*rfrac*brite),&white);
			}
		}
	}

/* Find angle for given vector, and its 90-degree offset	*/
/* Returns angles in radians					*/
/* Returns 0 if angle indeterminate				*/

int ImageFilter_Stars::FindAngle(float deltax,float deltay,float *angle,float *angle90) {
	float awk,a90wk;

	if(deltax==0.0)
	 {
	 if(deltay==0.0)
	  {
	  if(angle)
	   *angle=0.0f;
	  if(angle90)
	   *angle90=0.0f;
	  return(0);
	  }

	 if(deltay<0.0f)
	  awk=PI_270;
	 else
	  awk=PI_90;
	 goto get_90;
	 }
	if(deltay==0.0)
	 {
	 if(deltax<0.0)
	  awk=PI_180;
	 else
	  awk=0.0f;
	 goto get_90;
	 }

	awk=(float)atan(deltay/deltax);
	if(deltax<0)
	 awk+=PI_180;
	while(awk<0.0f)
	 awk+=PI_360;
	while(awk>=PI_360)
	 awk-=PI_360;

	get_90:
	a90wk=awk+PI_90;
	while(a90wk>=PI_360)
	 a90wk-=PI_360;

	if(angle)
	 *angle=awk;
	if(angle90)
	 *angle90=a90wk;

	return(1);
	}


/* Antialiased line routine */
/* Draws lines from floating-point positions */

// russom - 09/10/03 - defect 502028
//
// This defect is a release build VC7 compiler bug. 
// After the "if(data.blurDim)" block in ImageFilter_Stars::AALine, 
// brite will always be zero. Either re-arranging part of the 
// method or turning of optimization fixes the problem. We decided 
// to go the re-arrangement route.  #define REARRANGE_AALINE shows 
// the changes required to alter the compiled code enough to 
// eliminate the optimization problem.
#define REARRANGE_AALINE

void ImageFilter_Stars::AALine(float x1,float y1,float x2,float y2,float width,int brite) {
	int ix,midct;
	float dx,dy,adx,ady,minorax,temp,angle,slope,length;
	float leftmost,rightmost,topmost,botmost,lfrac,rfrac,tfrac,bfrac;
	float lxpos,lypos,rxpos,rypos,txpos,typos,bxpos,bypos,xpos,ypos;

	/* Adjust blur coordinates using percentage of shutter-open time */

#ifdef REARRANGE_AALINE
	dx=x2-x1;
	dy=y2-y1;
#else
	dx=x2-x1;
	x1+=blurpct*dx;
	dy=y2-y1;
	y1+=blurpct*dy;
#endif

	/* Dim line based on length */

	if(data.blurDim)
	 {
	 float bdmult=(float)data.blurDim/100.0f;
	 length=(float)sqrt(dx*dx+dy*dy)*bdmult+width;
	 brite=(int)((float)brite/(length/width));
	 }

#ifdef REARRANGE_AALINE
	x1+=blurpct*dx;
	y1+=blurpct*dy;
#endif

#ifdef NEWCODE
	/* Dim line based on length */

	if(data.blurDim)
	 {
	 // Calc new length
	 dx = x2-x1;
	 dy = y2-y1;

	 float bdmult=(float)data.blurDim/100.0f;
	 length=(float)sqrt(dx*dx+dy*dy);
	 if(length > 0.0f) {
		float effectiveLength = length * bdmult;
		if(effectiveLength < 1.0f)
			effectiveLength = 1.0f;
		brite=(int)((float)brite/effectiveLength);
		}
	 }
#endif

	/* Special cases -- dot, hline, vline */

	if(x1==x2)
	 {
	 if(y1==y2)	/* No line, really - just plot a dot */
	  FRect(x1,y1,width,width,brite);
	 else		/* Vertical line */
	  FRect(x1,(y1+y2)/2.0f,width,(float)fabs(y2-y1)+width,brite);
	 return;
	 }
	else
	 {
	 if(y1==y2)	/* Horizontal line */
	  {
	  FRect((x1+x2)/2.0f,y1,(float)fabs(x2-x1)+width,width,brite);
	  return;
	  }
	 }

	/* orient line left->right */

	if(x2<x1)
	 {
	 temp=x1;
	 x1=x2;
	 x2=temp;
	 temp=y1;
	 y1=y2;
	 y2=temp; 
	 }
	dx=x2-x1;
	dy=y2-y1;
	adx=(float)fabs(dx);
	ady=(float)fabs(dy);
	FindAngle(dx,dy,&angle,NULL);
	minorax=(float)(fabs(sin(angle))+fabs(cos(angle)));
	if(adx>ady)	/* Horizontally major */
	 {
	 slope=dy/dx;

	 /* Plot leftmost pixels */
	 leftmost=x1-(width/2.0f);
	 lfrac=1.0f-(float)frac(leftmost);
	 lxpos=(float)((int)(leftmost+0.5f));
	 lypos=y1-(x1-lxpos)*slope;
	 FRect(lxpos,lypos,1.0f,width,(int)(brite*lfrac));

	 /* Plot rightmost pixels */
	 rightmost=x2+(width/2.0f);
	 rfrac=(float)frac(rightmost);
	 rxpos=(int)rightmost+0.5f;
	 rypos=y2+(rxpos-x2)*slope;
	 FRect(rxpos,rypos,1.0f,width,(int)(brite*rfrac));

	 xpos=lxpos+1.0f;
	 ypos=lypos+slope;
	 midct=(int)rightmost-1-(int)leftmost;
	 for(ix=0; ix<midct; ++ix,xpos+=1.0f,ypos+=slope)
	  FRect(xpos,ypos,1.0f,width,brite);
	 }
	else		/* vertically major */
	 {
	 if(y2<y1)	/* Make sure line is top->bottom */
	  {
	  temp=y1;
	  y1=y2;
	  y2=temp;
	  temp=x1;
	  x1=x2;
	  x2=temp; 
	  }

	 slope=dx/dy;

	 /* Plot topmost pixels */
	 topmost=y1-(width/2.0f);
	 tfrac=(float)(1.0-frac(topmost));
	 typos=(float)((int)(topmost+0.5f));
	 txpos=x1-(y1-typos)*slope;
	 FRect(txpos,typos,width,1.0f,(int)(brite*tfrac));

	 /* Plot bottommost pixels */
	 botmost=y2+(width/2.0f);
	 bfrac=(float)frac(botmost);
	 bypos=(float)((int)(botmost+0.5f));
	 bxpos=x2+(bypos-y2)*slope;
	 FRect(bxpos,bypos,width,1.0f,(int)(brite*bfrac));

	 ypos=typos+1.0f;
	 xpos=txpos+slope;
	 midct=abs((int)botmost-1-(int)topmost);
	 for(ix=0; ix<midct; ++ix,ypos+=1.0f,xpos+=slope)
	  FRect(xpos,ypos,width,1.0f,brite);
	 }
	}

/* Tom's Floating-point antialiased circle plotter	*/
/* X,Y are center of circle				*/

int ImageFilter_Stars::FCirc(float x, float y, float radius, float aspect, int brite, BMM_Color_48 *c) {
	int ix,iy,minx,miny,maxx,maxy,in1,in2,in3,in4,layout,obrite;
	float xf,yf,pxl,pxh,pyl,pyh,dxl,dxh,dyl,dyh,d1,d2,d3,d4,v1,v2,v3,v4;
	float rsqr,alpha,alpha1,alpha2;

	/* Find screen coords that bound the floating-point circle */
	minx=(int)(x-radius);
	maxx=(int)(x+radius);
	miny=(int)(y-radius*aspect);
	maxy=(int)(y+radius*aspect);

	/* Calc radius squared */
	rsqr=radius*radius;

	/* Go thru pixel box, processing each */
	for(ix=minx,xf=minx-x; ix<=maxx; ++ix,xf+=1.0f)
		{
		pxl=xf;
		pxh=xf+1.0f;
		dxl=pxl*pxl;
		dxh=pxh*pxh;
		for(iy=miny,yf=miny-y; iy<=maxy; ++iy,yf+=1.0f)
			{
			pyl=yf/dev_aspect;
			pyh=(yf+1.0f)/dev_aspect;
			dyl=pyl*pyl;
			dyh=pyh*pyh;

			/* See how far each corner of pixel is from center */
			d1=dxl+dyl;
			d2=dxl+dyh;
			d3=dxh+dyh;
			d4=dxh+dyl;
			obrite=brite;

			if(d1<rsqr && d2<rsqr && d3<rsqr && d4<rsqr)
				{
				Plot(ix,iy,obrite,c);
				}
			else
			if(d1<rsqr || d2<rsqr || d3<rsqr || d4<rsqr)
				{
				/* Figure out pixel coverage */
				d1=(float)sqrt(d1)-radius;
				d2=(float)sqrt(d2)-radius;
				d3=(float)sqrt(d3)-radius;
				d4=(float)sqrt(d4)-radius;
				in1=(d1<0.0) ? 1:0;
				in2=(d2<0.0) ? 1:0;
				in3=(d3<0.0) ? 1:0;
				in4=(d4<0.0) ? 1:0;
				layout=in1 | (in2<<1) | (in3<<2) | (in4<<3);
				switch(layout)
					{
					case 1:	/* D1 in, rest out */
						v1= -d1;
						v2= d4;
						v3= d2;
						goto onein;
					case 2:	/* D2 in, rest out */
						v1= -d2;
						v2= d1;
						v3= d3;
						goto onein;
					case 4:	/* D3 in, rest out */
						v1= -d3;
						v2= d2;
						v3= d4;
						goto onein;
					case 8:	/* D4 in, rest out */
						v1= -d4;
						v2= d3;
						v3= d1;

						onein:
						alpha1= v1/(v1+v2);
						alpha2= v1/(v1+v3);
						alpha=MAX(alpha1,alpha2)/2.0f;
						break;
					case 3: /* D1/D2 in */
						v1=-d1;
						v2=-d2;
						v3=d4;
						v4=d3;
						goto twoin;
					case 6: /* D2/D3 in */
						v1=-d2;
						v2=-d3;
						v3=d1;
						v4=d4;
						goto twoin;
					case 9: /* D4/D1 in */
						v1=-d4;
						v2=-d1;
						v3=d3;
						v4=d2;
						goto twoin;
					case 12: /* D3/D4 in */
						v1=-d3;
						v2=-d4;
						v3=d2;
						v4=d1;

						twoin:
						alpha1= v1/(v1+v3);
						alpha2= v2/(v2+v4);
						alpha=(alpha1+alpha2)/2.0f;
						break;
					case 14: /* D1 out, rest in */
						v1= d1;
						v2= -d4;
						v3= -d2;
						goto oneout;
					case 13: /* D2 out, rest in */
						v1= d2;
						v2= -d1;
						v3= -d3;
						goto oneout;
					case 11: /* D3 out, rest in */
						v1= d3;
						v2= -d4;
						v3= -d2;
						goto oneout;
					case 7: /* D4 out, rest in */
						v1= d4;
						v2= -d3;
						v3= -d1;

						oneout:
						alpha1= v2/(v2+v1);
						alpha2= v3/(v3+v1);
						alpha=MAX(alpha1,alpha2)/2.0f+0.5f;
						break;
					}
				Plot(ix,iy,(int)((float)obrite*alpha),c);
				}
			}
		}
	return(1);
	}

static BOOL EnumerateNodes(INode *start, NodeCallBack &findCam) {
	int nodes = start->NumberOfChildren();
	for(int i = 0; i < nodes; ++i) {
		INode *node = start->GetChildNode(i);
		if(!findCam.CallBack(node))
			return FALSE;		// Stop enumerating
		if(!EnumerateNodes(node,findCam))
			return FALSE;
		}
	return TRUE;
	}

BOOL FindCameraNode::CallBack(INode *n) {
	TSTR camName(n->GetName());
	if(camName == name) {
		Object *obj = n->GetObjectRef();
		if(obj && obj->SuperClassID() == CAMERA_CLASS_ID) {
			node = n;
			return FALSE;	// Got it -- No more enumerating
			}
		}
	return TRUE;	// This isn't it -- keep going!
	}

BOOL CameraFiller::CallBack(INode *n) {
	Object *obj = n->GetObjectRef();
	if(obj && obj->SuperClassID() == CAMERA_CLASS_ID) {
		if(obj && obj->SuperClassID() == CAMERA_CLASS_ID)
			SendDlgItemMessage(hWnd, id, CB_ADDSTRING, 0, (LPARAM)n->GetName());
		ok = TRUE;
		}
	return TRUE;	// This isn't it -- keep going!
	}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::Render()
//

BOOL ImageFilter_Stars::Render(HWND hWnd) {
	BOOL			result			= TRUE;

	// Grab info on the view and such
	RenderInfo *rinfo = srcmap->GetRenderInfo();
	if(!rinfo) {
		ErrorMessage(IDS_NO_RENDERINFO);
		return FALSE;
		}
	if(rinfo->projType != ProjPerspective) {
		ErrorMessage(IDS_NOT_PERSPECTIVE);
		return FALSE;
		}
	dev_aspect = srcmap->Aspect();
	blurpct=1.0f - ((float)data.blurAmount / 100.0f);

	// We have a valid condition for generating a starfield -- do it!

	// Get the stars

	if(data.dataType == DATA_CUSTOM) {
		FILE *istream;
		int p;

		// Try just the raw name they gave
		if((istream=fopen(data.filename,"rb"))!=NULL)
			goto got_file;

		// Search thru the plugin paths
		for(p = 0; p < Max()->GetPlugInEntryCount(); ++p) {
			TCHAR *dir = Max()->GetPlugInDir(p);
			TCHAR fname[256];
			_tcsncpy(fname,dir,255);
			_tcsncat(fname,data.filename,255);
			if((istream=fopen(fname,"rb"))!=NULL)
				goto got_file;
			}

		ErrorMessage(IDS_CANT_OPEN);
		return FALSE;

		got_file:
		if(fread(&starcount,sizeof(int),1,istream)!=1)
			{
			bad_infile:
			ErrorMessage(IDS_BAD_FILE);
			fclose(istream);
			FreeData();
			return FALSE;
			}
		// Establish how big the file is so we can verify its authenticity as a star file
		fseek(istream, 0L, SEEK_END);
		int filesize = ftell(istream);
		fseek(istream, 4L, SEEK_SET);

		// Make sure the file is valid
		if(starcount == 0 || filesize != (int)((starcount * sizeof(Star)) + 4))
			goto bad_infile;

		/* Allocate room for stars */

		stardata = new Star[starcount];
		if(!stardata) {
			no_ram:
			ErrorMessage(IDS_OUT_OF_MEMORY);
			fclose(istream);
			FreeData();
			return FALSE;
			}
		pos1 = new StarPos[starcount];
		if(!pos1)
			goto no_ram;
		if(data.useBlur) {
			pos2 = new StarPos[starcount];
			if(!pos2)
				goto no_ram;
			}
		else
			pos2 = NULL;

		if(fread(stardata,sizeof(Star),starcount,istream) != (size_t)starcount)
			goto bad_infile;

		fclose(istream);
		}
	else {
		starcount = data.count;
		stardata = new Star[starcount];
		pos1 = new StarPos[starcount];
		if(data.useBlur)
			pos2 = new StarPos[starcount];
		else
			pos2 = NULL;

		rand.Seed(data.seed);
		for(int ix=0; ix<starcount; ++ix)
			{
			Point3 p;

			p.x=(float)(rand.Random() & 0x0fff)-2048;
			p.y=(float)(rand.Random() & 0x0fff)-2048;
			p.z=(float)(rand.Random() & 0x0fff)-2048;

			/* Make sure all stars are equidistant at 1000 units */

			stardata[ix].p = 1000.0f * Normalize(p);
			stardata[ix].fmag=10.0f - (((float)(rand.Random() & 0x0fff)/409.5f) *
				((float)(rand.Random() & 0x0fff) / 4095.0f));
			stardata[ix].mag = 0;
			}
		}

	// Set up some stuff...

	float factor2=((float)data.brightest - (float)data.dimmest) / 10.0f;
	for(int ix=0; ix<starcount; ++ix) {
		float mag=stardata[ix].fmag;
		if(data.briteType == BRITE_LOG)
			stardata[ix].mag=(int)(pow(10.0f - mag, 2.407f));
		else
			stardata[ix].mag=(int)((10.0f - mag) * factor2) + data.dimmest;
		}

	// Grab the camera that's used for the shot
	
	INode *camNode;
	TSTR ourCamera(data.camera);
	FindCameraNode findCam(ourCamera);
	EnumerateNodes(Max()->GetRootNode(), findCam);
	if(!findCam.Found()) {
		ErrorMessage(IDS_CAMERA_NOT_FOUND);
		FreeData();
		return FALSE;
		}

	// If we get here, we have a valid camera node -- Grab the transform(s) for it
	camNode = findCam.node;
	if(rinfo->fieldRender)
		{
		CameraInfo prevcam, cam1, cam2;

		ComputeCamera(camNode, rinfo->renderTime[0]-(rinfo->renderTime[1]-rinfo->renderTime[0]), &prevcam);
		ComputeCamera(camNode, rinfo->renderTime[0], &cam1);
		ComputeCamera(camNode, rinfo->renderTime[1], &cam2);

		/* First, even scan lines */
		Process((rinfo->fieldOdd) ? ODD_LINES:EVEN_LINES, rinfo, &cam1, &prevcam);

		/* Next, odd scan lines */
		Process((rinfo->fieldOdd) ? EVEN_LINES:ODD_LINES, rinfo, &cam2, &cam1);
		}
	else
		{
		CameraInfo prevcam, camera;

		ComputeCamera(camNode, rinfo->renderTime[0] - 160, &prevcam);
		ComputeCamera(camNode, rinfo->renderTime[0], &camera);

		Process(ALL_LINES, rinfo, &camera, &prevcam);
		}

	FreeData();
	return(TRUE);

}

void ImageFilter_Stars::TransformStars(CameraInfo *c, StarPos *p) {
	float fsx,fsy;
	float centerx,centery,camzfac,camw2,camh2;
	int dwidth,dheight;
	int ix,sx,sy;

	camzfac=(float)tan((PI_180 - c->fov)/2.0f);
	camw2=srcmap->Width()/2.0f;
	camh2=camw2 * -dev_aspect;

	dwidth = srcmap->Width();
	dheight = srcmap->Height();
	centerx = (float)dwidth / 2.0f;
	centery = (float)dheight / 2.0f;

	for(ix=0; ix<starcount; ++ix)
		{
		Point3 tp = stardata[ix].p * c->tm;
		if(tp.z < -100.0f)
			{
			p[ix].x=fsx=centerx + tp.x / -tp.z * camzfac * camw2;
			p[ix].y=fsy=centery + tp.y / -tp.z * camzfac * camh2;
			sx=(int)fsx;
			sy=(int)fsy;
			if(sx>=0 && sx<dwidth && sy>=0 && sy<dheight) {
				p[ix].vis=1;	/* Onscreen */
				}
			else
				p[ix].vis=0;	/* Offscreen */
			}
		else
			p[ix].vis= -1;	/* Behind -- no useful coord */
		}
	}

void ImageFilter_Stars::Process(int type, RenderInfo *rinfo, CameraInfo *i1, CameraInfo *i2) {
	int ix;
	StarPos *p1,*p2;
	field_type=type;

	/* If using motion blur, calc last star positions */

	if(data.useBlur)
		TransformStars(i2,pos2);

	/* Now calc current star positions */

	TransformStars(i1,pos1);

	/* Now plot each star if it's visible */

	if(data.useBlur)
		{
		for(ix=0,p1=pos1,p2=pos2; ix<starcount; ++ix,++p1,++p2)
			{
			if((p1->vis+p2->vis)>0)
				AALine(p2->x,p2->y,p1->x,p1->y,data.size,stardata[ix].mag);
			}
		}
	else
		{
		float ysize=data.size*dev_aspect;
		for(ix=0,p1=pos1; ix<starcount; ++ix,++p1)
			{
			if(p1->vis==1)
				{
				if(data.size<1.0)
					FRect(p1->x,p1->y,data.size,ysize,
						stardata[ix].mag);
				else
					FCirc(p1->x,p1->y,data.size/2.0f,dev_aspect,
						stardata[ix].mag,&white);
				}
			}
		}
	}

BOOL ImageFilter_Stars::LoadCameras(HWND hWnd)	{
	BOOL ok = FALSE;
	CameraFiller camFiller(hWnd, IDC_CAMERAS);
	SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_RESETCONTENT, 0, 0);
	EnumerateNodes(Max()->GetRootNode(), camFiller);
	
	if(camFiller.OK()) {
		// See if our data matches one of 'em
		int item = SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)data.camera);
		if(item != CB_ERR)
			SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_SETCURSEL, item, 0);
		else
			SendDlgItemMessage(hWnd, IDC_CAMERAS, CB_SETCURSEL, 0, 0);
		}
	return camFiller.OK();
	}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::LoadConfigure()

BOOL ImageFilter_Stars::LoadConfigure ( void *ptr ) {
     STARSDATA *buf = (STARSDATA *)ptr;
     if (buf->version == STARSVERSION) {
        memcpy((void *)&data,ptr,sizeof(STARSDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::SaveConfigure()

BOOL ImageFilter_Stars::SaveConfigure ( void *ptr ) {
     if (ptr) {
        memcpy(ptr,(void *)&data,sizeof(STARSDATA));
		return (TRUE);
	} else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Stars::EvaluateConfigure()

DWORD ImageFilter_Stars::EvaluateConfigure ( ) {
	return (sizeof(STARSDATA));
}

void ImageFilter_Stars::ErrorMessage(int id) {
	TSTR s1(GetString(IDS_STARFIELD_ERROR));
	TSTR s2(GetString(id));
	Max()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,s1.data(),s2.data());
	}


//--	EOF: Stars.cpp --------------------------------------------------------
