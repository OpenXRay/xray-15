/**********************************************************************
 *<
	FILE: camera.cpp

	DESCRIPTION:  A Simple Camera implementation

	CREATED BY: Dan Silva

	HISTORY: created 14 December 1994


	NOTE: 

	    To ensure that the camera has a valid targDist during
	    network rendering, be sure to call:

		UpdateTargDistance( TimeValue t, INode* inode );

		This call should be made prior to cameraObj->EvalWorldState(...)


 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
#include "gencam.h"
#include "camera.h"
#include "target.h"
#include "macrorec.h"
#include "decomp.h"
#include "iparamb2.h"

#include "Max.h"
#include "maxscrpt\MAXScrpt.h"
#include "bitmap.h"
#include "guplib.h"
#include "gup.h"
#include "maxscrpt\Strings.h"
#include "maxscrpt\Parser.h"


// Parameter block indices
#define PB_FOV						0
#define PB_TDIST					1
#define PB_HITHER					2
#define PB_YON						3
#define PB_NRANGE					4
#define PB_FRANGE					5
#define PB_MP_EFFECT_ENABLE			6	// mjm - 07.17.00
#define PB_MP_EFF_REND_EFF_PER_PASS	7	// mjm - 07.17.00

// Depth of Field parameter block indicies
#define PB_DOF_ENABLE	0
#define PB_DOF_FSTOP	1

#define MIN_FSTOP		0.0001f
#define MAX_FSTOP		100.0f

#define WM_SET_TYPE		WM_USER + 0x04002

#define MIN_CLIP	0.000001f
#define MAX_CLIP	1.0e32f

#define NUM_CIRC_PTS	28
#define SEG_INDEX		7

#define RELEASE_SPIN(x)   if (so->x) { ReleaseISpinner(so->x); so->x = NULL;}
#define RELEASE_BUT(x)   if (so->x) { ReleaseICustButton(so->x); so->x = NULL;}

static int waitPostLoad = 0;
static void resetCameraParams();
static HIMAGELIST hCamImages = NULL;

//------------------------------------------------------
class SimpleCamClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SimpleCamera; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_FREE_CLASS); }
	SClass_ID		SuperClassID() { return CAMERA_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(SIMPLE_CAM_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetCameraParams(); }
	};

static SimpleCamClassDesc simpleCamDesc;

ClassDesc* GetSimpleCamDesc() { return &simpleCamDesc; }


// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// xavier robitaille | 03.02.07 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define DEF_HITHER_CLIP		1.0f
#define DEF_YON_CLIP		1000.0f
#else
#define DEF_HITHER_CLIP	0.075f
#define DEF_YON_CLIP		250.0f
#endif

// Class variables of SimpleCamera
Mesh SimpleCamera::mesh;
short SimpleCamera::meshBuilt=0;
float SimpleCamera::dlgFOV = DegToRad(45.0);
short SimpleCamera::dlgShowCone =0;
short SimpleCamera::dlgShowHorzLine =0;
float SimpleCamera::dlgTDist = FIXED_CONE_DIST;
short SimpleCamera::dlgClip = 0;
float SimpleCamera::dlgHither = DEF_HITHER_CLIP;
float SimpleCamera::dlgYon = DEF_YON_CLIP;
float SimpleCamera::dlgNearRange = 0.0f;
float SimpleCamera::dlgFarRange = 1000.0f;
short SimpleCamera::dlgRangeDisplay = 0;
short SimpleCamera::dlgIsOrtho = 0;

short SimpleCamera::dlgDOFEnable = 1;
float SimpleCamera::dlgDOFFStop = 2.0f;
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
short SimpleCamera::dlgMultiPassEffectEnable = 0;
short SimpleCamera::dlgMPEffect_REffectPerPass = 0;
Tab<ClassEntry*> SimpleCamera::smCompatibleEffectList;
// mjm - end
#endif // NO_CAMERA_MULTIPASS

BOOL SimpleCamera::inCreate=FALSE;

void resetCameraParams() 
{
	SimpleCamera::dlgFOV = DegToRad(45.0);
	SimpleCamera::dlgShowCone =0;
	SimpleCamera::dlgShowHorzLine =0;
	SimpleCamera::dlgTDist = FIXED_CONE_DIST;
	SimpleCamera::dlgClip = 0;
	SimpleCamera::dlgHither = DEF_HITHER_CLIP;
	SimpleCamera::dlgYon = DEF_YON_CLIP;
	SimpleCamera::dlgNearRange = 0.0f;
	SimpleCamera::dlgFarRange = 1000.0f;
	SimpleCamera::dlgRangeDisplay = 0;
	SimpleCamera::dlgIsOrtho = 0;

	SimpleCamera::dlgDOFEnable = 1;
	SimpleCamera::dlgDOFFStop = 2.0f;
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
	SimpleCamera::dlgMultiPassEffectEnable = 0;
	SimpleCamera::dlgMPEffect_REffectPerPass = 0;
// mjm - end
#endif // NO_CAMERA_MULTIPASS
}

SimpleCamera * SimpleCamera::currentEditCam = NULL;
HWND SimpleCamera::hSimpleCamParams = NULL;
HWND SimpleCamera::hDepthOfFieldParams = NULL;
IObjParam *SimpleCamera::iObjParams;
ISpinnerControl *SimpleCamera::fovSpin = NULL;
ISpinnerControl *SimpleCamera::lensSpin = NULL;
ISpinnerControl *SimpleCamera::tdistSpin = NULL;
ISpinnerControl *SimpleCamera::hitherSpin = NULL;
ISpinnerControl *SimpleCamera::yonSpin = NULL;
#ifndef NO_CAMERA_ENVRANGE
ISpinnerControl *SimpleCamera::envNearSpin = NULL;
ISpinnerControl *SimpleCamera::envFarSpin = NULL;
#endif // NO_CAMERA_ENVRANGE
ISpinnerControl *SimpleCamera::fStopSpin = NULL;
ICustButton *SimpleCamera::iFovType = NULL;

static float mmTab[9] = {
	15.0f, 20.0f, 24.0f, 28.0f, 35.0f, 50.0f, 85.0f, 135.0f, 200.0f
	};


static float GetAspect() {
	return GetCOREInterface()->GetRendImageAspect();
	};

static float GetApertureWidth() {
	return GetCOREInterface()->GetRendApertureWidth();
	}


static int GetTargetPoint(TimeValue t, INode *inode, Point3& p) {
	Matrix3 tmat;
	if (inode->GetTargetTM(t,tmat)) {
		p = tmat.GetTrans();
		return 1;
		}
	else 
		return 0;
	}

static int is_between(float a, float b, float c) 
{
	float t;
	if (b>c) { t = b; b = c; c = t;}
	return((a>=b)&&(a<=c));
}

static float interp_vals(float m, float *mtab, float *ntab, int n) 
{
	float frac;
	for (int i=1; i<n; i++) {
		if (is_between(m,mtab[i-1],mtab[i])) {
			frac = (m - mtab[i-1])/(mtab[i]-mtab[i-1]);
			return((1.0f-frac)*ntab[i-1] + frac*ntab[i]);
		}
	}
	return 0.0f;
}


// This changes the relation between FOV and Focal length.
void SimpleCamera::RenderApertureChanged(TimeValue t) {
	UpdateUI(t);
	}

void SimpleCamera::SetFOVType(int ft) {
	fovType = ft;
	if (fovSpin&&iObjParams)
		fovSpin->SetValue( RadToDeg(WFOVtoCurFOV(GetFOV(iObjParams->GetTime()))), FALSE );
	}


float SimpleCamera::MMtoFOV(float mm) {
	return float(2.0f*atan(0.5f*GetApertureWidth()/mm));
	}

float SimpleCamera::FOVtoMM(float fov)	{
	float w = GetApertureWidth();
	float mm = float((0.5f*w)/tan(fov/2.0f));
	return mm;
	}


float SimpleCamera::CurFOVtoWFOV(float cfov) {
	switch (fovType) {
		case FOV_H: {
			return float(2.0*atan(GetAspect()*tan(cfov/2.0f)));
			}
		case FOV_D: {
			float w = GetApertureWidth();
			float h = w/GetAspect();
			float d = (float)sqrt(w*w + h*h);	
			return float(2.0*atan((w/d)*tan(cfov/2.0f)));
			}
		default:
			return cfov;
		}
	}	


float SimpleCamera::WFOVtoCurFOV(float fov) {
	switch (fovType) {
		case FOV_H: {
			return float(2.0*atan(tan(fov/2.0f)/GetAspect()));
			}
		case FOV_D: {
			float w = GetApertureWidth();
			float h = w/GetAspect();
			float d = (float)sqrt(w*w + h*h);	
			return float(2.0*atan((d/w)*tan(fov/2.0f)));
			}
		default:
			return fov;
		}
	}	


static void LoadCamResources() {
	static BOOL loaded=FALSE;
	if (loaded) return;
	HBITMAP hBitmap;
	HBITMAP hMask;
	hCamImages = ImageList_Create(14,14, ILC_COLOR4|ILC_MASK, 3, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_FOV));
	hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_FOVMASK));
	ImageList_Add(hCamImages,hBitmap,hMask);
	DeleteObject(hBitmap);	
	DeleteObject(hMask);	
	}

class DeleteCamResources {
	public:
		~DeleteCamResources() {
			ImageList_Destroy(hCamImages);
			}
	};

static DeleteCamResources theDelete;

static int typeName[NUM_CAM_TYPES] = {
	IDS_DB_FREE_CAM,
	IDS_DB_TARGET_CAM
	};


static Value* LocalExecuteScript(CharStream* source, bool *res) {

	*res = true;

	init_thread_locals();
	push_alloc_frame();
	three_typed_value_locals(Parser* parser, Value* code, Value* result);
	CharStream* out = thread_local(current_stdout);
	vl.parser = new Parser (out);
	
	try	{

		source->flush_whitespace();
		while (!source->at_eos()) {
			vl.code		= vl.parser->compile(source);
			vl.result	= vl.code->eval()->get_heap_ptr();
			source->flush_whitespace();
		}
		source->close();

	} catch (...) {
		*res = false;
	}

	if (vl.result == NULL)
		vl.result = &ok;
	
	pop_alloc_frame();
	return_value(vl.result);

}

// CallMaxscript
// Send the string to maxscript 
//
static void CallMaxscript(TCHAR *s)
{
	static bool script_initialized = false;
	if (!script_initialized) {
		init_MAXScript();
		script_initialized = TRUE;
	}
	init_thread_locals();
	
	push_alloc_frame();
	two_typed_value_locals(StringStream* ss, Value* result);
	
	vl.ss = new StringStream (s);
	bool res = false;
	try	{
		vl.result = LocalExecuteScript(vl.ss,&res);
	} catch (...) {
		res = false;
	}
	thread_local(current_result) = vl.result;
	thread_local(current_locals_frame) = vl.link;
	pop_alloc_frame();
}

INT_PTR CALLBACK SimpleCamParamDialogProc( 
	HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	SimpleCamera *so = (SimpleCamera *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !so && message != WM_INITDIALOG ) return FALSE;
	TimeValue t = so->iObjParams->GetTime();

	float tmpSmall, tmpLarge;

	switch ( message )
	{
		case WM_INITDIALOG:
		{
			LoadCamResources();
			so = (SimpleCamera *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)so );
			SetDlgFont( hDlg, so->iObjParams->GetAppHFont() );
//			if(so->iObjParams->GetCommandMode()->ID() == CID_OBJSELECT)
//				EnableWindow(GetDlgItem(hDlg, IDC_CREATE_VIEW), FALSE);
			
			
			CheckDlgButton( hDlg, IDC_SHOWCAMCONE, so->coneState );
			CheckDlgButton( hDlg, IDC_SHOWHORZLINE, so->horzLineState );
#ifndef NO_CAMERA_ENVRANGE
			CheckDlgButton( hDlg, IDC_SHOW_RANGES, so->rangeDisplay );
#endif // NO_CAMERA_ENVRANGE
			CheckDlgButton( hDlg, IDC_IS_ORTHO, so->isOrtho );
			CheckDlgButton( hDlg, IDC_MANUAL_CLIP, so->manualClip );
			EnableWindow( GetDlgItem(hDlg, IDC_HITHER), so->manualClip);
			EnableWindow( GetDlgItem(hDlg, IDC_HITHER_SPIN), so->manualClip);
			EnableWindow( GetDlgItem(hDlg, IDC_YON), so->manualClip);
			EnableWindow( GetDlgItem(hDlg, IDC_YON_SPIN), so->manualClip);

			HWND hwndType = GetDlgItem(hDlg, IDC_CAM_TYPE);
			for (int i=0; i<NUM_CAM_TYPES; i++)
				SendMessage(hwndType, CB_ADDSTRING, 0, (LPARAM)GetString(typeName[i]));
			SendMessage( hwndType, CB_SETCURSEL, i, (LPARAM)0 );
			EnableWindow(hwndType,!so->inCreate);		

			// DS: 8/15/00  
//			if(so->hasTarget)
//			{
//				TCHAR buf[40];
//				_stprintf(buf,_T("%0.3f"),so->targDist);
//				SetWindowText(GetDlgItem(hDlg,IDC_TARG_DISTANCE),buf);
//			}
			
			CheckDlgButton( hDlg, IDC_DOF_ENABLE, so->GetDOFEnable(t) );

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
			CheckDlgButton(hDlg, IDC_ENABLE_MP_EFFECT, so->GetMultiPassEffectEnabled(t) );
			CheckDlgButton(hDlg, IDC_MP_EFFECT_REFFECT_PER_PASS, so->GetMPEffect_REffectPerPass() );

			// build list of multi pass effects
			int selIndex = -1;
			IMultiPassCameraEffect *pIMultiPassCameraEffect = so->GetIMultiPassCameraEffect();
			HWND hEffectList = GetDlgItem(hDlg, IDC_MP_EFFECT);
			SimpleCamera::FindCompatibleMultiPassEffects(so);
			int numClasses = SimpleCamera::smCompatibleEffectList.Count();
			for (i=0; i<numClasses; i++)
			{
				int index = SendMessage( hEffectList, CB_ADDSTRING, 0, (LPARAM)SimpleCamera::smCompatibleEffectList[i]->CD()->ClassName() );
				if ( pIMultiPassCameraEffect && ( pIMultiPassCameraEffect->ClassID() == SimpleCamera::smCompatibleEffectList[i]->CD()->ClassID() ) )
				{
					selIndex = index;
				}
			}
			SendMessage(hEffectList, CB_SETCURSEL, selIndex, (LPARAM)0);
			EnableWindow( GetDlgItem(hDlg, IDC_PREVIEW_MP_EFFECT), so->GetMultiPassEffectEnabled(t) );
// mjm - end
#endif	// NO_CAMERA_MULTIPASS
			return FALSE;
		}

		case WM_DESTROY:
			RELEASE_SPIN ( fovSpin );
			RELEASE_SPIN ( lensSpin );
			RELEASE_SPIN ( hitherSpin );
			RELEASE_SPIN ( yonSpin );
#if !defined (WEBVERSION) && !defined(NO_CAMERA_ENVRANGE)
			RELEASE_SPIN ( envNearSpin );
			RELEASE_SPIN ( envFarSpin );
#endif
			RELEASE_SPIN ( fStopSpin );
			RELEASE_BUT ( iFovType );
			// DS 8/15/00 
//			if(!so->hasTarget)                
				RELEASE_SPIN ( tdistSpin );
			return FALSE;

		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();
			switch ( LOWORD(wParam) ) {
				case IDC_FOVSPINNER:
					so->SetFOV(t, so->CurFOVtoWFOV(DegToRad(so->fovSpin->GetFVal())));	
					so->lensSpin->SetValue(so->FOVtoMM(so->GetFOV(t)), FALSE);
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
				case IDC_LENSSPINNER:
					so->SetFOV(t, so->MMtoFOV(so->lensSpin->GetFVal()));	
					so->fovSpin->SetValue(RadToDeg(so->WFOVtoCurFOV(so->GetFOV(t))), FALSE);
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
				case IDC_TDISTSPINNER:
					so->SetTDist(t, so->tdistSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
#if 0	// this section leaves the spinners unconstrained
				case IDC_HITHER_SPIN:
					so->SetClipDist(t, CAM_HITHER_CLIP, so->hitherSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
				case IDC_YON_SPIN:
					so->SetClipDist(t, CAM_YON_CLIP, so->yonSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
#else	// here, we constrain hither <= yon
				case IDC_HITHER_SPIN:
				case IDC_YON_SPIN:
					tmpSmall = so->hitherSpin->GetFVal();
					tmpLarge = so->yonSpin->GetFVal();

					if(tmpSmall > tmpLarge) {
						if(LOWORD(wParam) == IDC_HITHER_SPIN) {
							so->yonSpin->SetValue(tmpSmall, FALSE);
							so->SetClipDist(t, CAM_YON_CLIP, so->yonSpin->GetFVal());	
						}
						else	{
							so->hitherSpin->SetValue(tmpLarge, FALSE);
							so->SetClipDist(t, CAM_HITHER_CLIP, so->hitherSpin->GetFVal());	
						}
					}
					if(LOWORD(wParam) == IDC_HITHER_SPIN)
						so->SetClipDist(t, CAM_HITHER_CLIP, so->hitherSpin->GetFVal());	
					else
						so->SetClipDist(t, CAM_YON_CLIP, so->yonSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
#endif
#ifndef NO_CAMERA_ENVRANGE
#if 0	// similar constraint comments apply here
				case IDC_NEAR_RANGE_SPIN:
					so->SetEnvRange(t, ENV_NEAR_RANGE, so->envNearSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
				case IDC_FAR_RANGE_SPIN:
					so->SetEnvRange(t, ENV_FAR_RANGE, so->envFarSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
#else
				case IDC_NEAR_RANGE_SPIN:
				case IDC_FAR_RANGE_SPIN:
					tmpSmall = so->envNearSpin->GetFVal();
					tmpLarge = so->envFarSpin->GetFVal();

					if(tmpSmall > tmpLarge) {
						if(LOWORD(wParam) == IDC_NEAR_RANGE_SPIN) {
							so->envFarSpin->SetValue(tmpSmall, FALSE);
							so->SetEnvRange(t, ENV_FAR_RANGE, so->envFarSpin->GetFVal());	
						}
						else	{
							so->envNearSpin->SetValue(tmpLarge, FALSE);
							so->SetEnvRange(t, ENV_NEAR_RANGE, so->envNearSpin->GetFVal());	
						}
					}
					if(LOWORD(wParam) == IDC_NEAR_RANGE_SPIN)
						so->SetEnvRange(t, ENV_NEAR_RANGE, so->envNearSpin->GetFVal());	
					else
						so->SetEnvRange(t, ENV_FAR_RANGE, so->envFarSpin->GetFVal());	
					so->UpdateKeyBrackets(t);
					so->iObjParams->RedrawViews(t,REDRAW_INTERACTIVE);
					break;
#endif
#endif // NO_CAMERA_ENVRANGE
				case IDC_DOF_FSTOP_SPIN:
					so->SetDOFFStop ( t, so->fStopSpin->GetFVal ());
					so->UpdateKeyBrackets (t);
					break;
				}
			return TRUE;

		case WM_SET_TYPE:
			theHold.Begin();
			so->SetType(wParam);
			theHold.Accept(GetString(IDS_DS_SETCAMTYPE));
			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			if ((message == CC_SPINNER_BUTTONUP) && (LOWORD(wParam) == IDC_FOVSPINNER))
			{
				TCHAR cmdString[512];
				_stprintf(cmdString,_T(" InvalidateAllBackgrounds() "));	
				CallMaxscript(cmdString);
			}
			so->iObjParams->RedrawViews(t,REDRAW_END);
			return TRUE;

		case WM_MOUSEACTIVATE:
			so->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
			so->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:			
			switch( LOWORD(wParam) )
			{
#if 0	// no longer support creating cams from views
				case IDC_CREATE_VIEW:
				{
					ViewExp *vpt;
					SimpleCamera *camObject;
					TargetObject *targObject;
					INode *camNode, *targNode;
					TSTR targName;
					Matrix3 m, targM;
					vpt = so->iObjParams->GetActiveViewport();
					if(!vpt->IsPerspView()) {
						MessageBox(hDlg, _T("No way, Jose"), _T("Camera Creation Failure"), MB_OK);
						break;
					}
					camObject = new SimpleCamera(so->hasTarget);
				   	theHold.Begin();	 // begin hold for undo
					// link it up
					so->iObjParams->SelectNode(camNode = so->iObjParams->CreateObjectNode( camObject));
					camNode->InvalidateTM();
					vpt->GetAffineTM(m);
					m = Inverse(m);
					camObject->SetFOV(t, vpt->GetFOV());
					if(!so->hasTarget)
						camObject->SetTDist(t, vpt->GetFocalDist());
					else {
						// Create target object and node
						targObject = new TargetObject;
						assert(targObject);
						targNode = so->iObjParams->CreateObjectNode( targObject);
						targName = camNode->GetName();
						targName += GetString(IDS_DB_DOT_TARGET);
						targNode->SetName(targName);

						// hook up camera to target using lookat controller.
						so->iObjParams->BindToTarget(camNode,targNode);

						// set the target point based on the lookat direction and dist
						targM = m;
						targM.PreTranslate(Point3(0.0f, 0.0f, -vpt->GetFocalDist()));
						targNode->SetNodeTM(t, targM);
					}
					camNode->SetNodeTM(t, m);
					camObject->Enable(1);
					so->iObjParams->RedrawViews(so->iObjParams->GetTime());  
				    theHold.Accept(GetString(IDS_DS_CREATE));	 
					so->iObjParams->ReleaseViewport(vpt);
					break;
				}
#endif
				case IDC_MANUAL_CLIP:
					so->SetManualClip( IsDlgButtonChecked( hDlg, IDC_MANUAL_CLIP) );
					EnableWindow( GetDlgItem(hDlg, IDC_HITHER), so->manualClip);
					EnableWindow( GetDlgItem(hDlg, IDC_HITHER_SPIN), so->manualClip);
					EnableWindow( GetDlgItem(hDlg, IDC_YON), so->manualClip);
					EnableWindow( GetDlgItem(hDlg, IDC_YON_SPIN), so->manualClip);
					so->iObjParams->RedrawViews(t);
					break;
				case IDC_SHOWCAMCONE:
					so->SetConeState( IsDlgButtonChecked( hDlg, IDC_SHOWCAMCONE ) );
					so->iObjParams->RedrawViews(t);
					break;
				case IDC_SHOWHORZLINE:
					so->SetHorzLineState( IsDlgButtonChecked( hDlg, IDC_SHOWHORZLINE ) );
					so->iObjParams->RedrawViews(t);
					break;
#ifndef NO_CAMERA_ENVRANGE
				case IDC_SHOW_RANGES:
					so->SetEnvDisplay( IsDlgButtonChecked( hDlg, IDC_SHOW_RANGES ) );
					so->iObjParams->RedrawViews(t);
					break;
#endif // NO_CAMERA_ENVRANGE
				case IDC_IS_ORTHO:
					so->SetOrtho( IsDlgButtonChecked( hDlg, IDC_IS_ORTHO ) );
					so->iObjParams->RedrawViews(t);
					break;
				case IDC_15MM:
				case IDC_20MM:
				case IDC_24MM:
				case IDC_28MM:
				case IDC_35MM:
				case IDC_50MM:
				case IDC_85MM:
				case IDC_135MM:
				case IDC_200MM:
					theHold.Begin();
					so->SetFOV(t, so->MMtoFOV(mmTab[LOWORD(wParam) - IDC_15MM]));	
					so->fovSpin->SetValue(RadToDeg(so->WFOVtoCurFOV(so->GetFOV(t))), FALSE);
					so->lensSpin->SetValue(so->FOVtoMM(so->GetFOV(t)), FALSE);
					so->iObjParams->RedrawViews(t,REDRAW_END);
					theHold.Accept(GetString(IDS_DS_CAMPRESET));
					break;		   
				case IDC_FOV_TYPE: 
					so->SetFOVType(so->iFovType->GetCurFlyOff());
					break;
				case IDC_CAM_TYPE:
				{
					int code = HIWORD(wParam);
					if (code==CBN_SELCHANGE) {
						int newType = SendMessage( GetDlgItem(hDlg,IDC_CAM_TYPE), CB_GETCURSEL, 0, 0 );
						PostMessage(hDlg,WM_SET_TYPE,newType,0);
						}
					break;
				}
// nac - begin - 12.08.00
//				case IDC_DOF_ENABLE:
//					so->SetDOFEnable(t, IsDlgButtonChecked ( hDlg, IDC_DOF_ENABLE ));
//					break;
//nac - end
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
				case IDC_ENABLE_MP_EFFECT:
					so->SetMultiPassEffectEnabled( t, IsDlgButtonChecked(hDlg, IDC_ENABLE_MP_EFFECT) );
					break;
				case IDC_MP_EFFECT_REFFECT_PER_PASS:
					so->SetMPEffect_REffectPerPass( IsDlgButtonChecked(hDlg, IDC_MP_EFFECT_REFFECT_PER_PASS) );
					break;
				case IDC_PREVIEW_MP_EFFECT:
					GetCOREInterface()->DisplayActiveCameraViewWithMultiPassEffect();
					break;
				case IDC_MP_EFFECT:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int index = SendMessage( GetDlgItem(hDlg,IDC_MP_EFFECT), CB_GETCURSEL, 0, 0 );
						Tab<ClassEntry*> &effectList = SimpleCamera::GetCompatibleEffectList();
						if ( (index >= 0) && ( index < effectList.Count() ) )
							{
							IMultiPassCameraEffect *pPrevCameraEffect = so->GetIMultiPassCameraEffect();
							if ( !pPrevCameraEffect || ( pPrevCameraEffect->ClassID() != effectList[index]->CD()->ClassID() ) ) {
								IMultiPassCameraEffect *mpce = reinterpret_cast<IMultiPassCameraEffect *>( effectList[index]->CD()->Create(0) );
								theHold.Begin();
								so->SetIMultiPassCameraEffect(mpce);
								theHold.Accept(GetString(IDS_DS_MULTIPASS));
								}
							}
						else
						{
							DbgAssert(0);
						}
					}
					break;
				}
// mjm - end
#endif // NO_CAMERA_MULTIPASS
			}
			return FALSE;

		default:
			return FALSE;
		}
	}



void SimpleCamera::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	iObjParams = ip;	
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	inCreate = (flags & BEGIN_EDIT_CREATE) ? 1 : 0;
#else
	inCreate = 0;
#endif

	currentEditCam = this;

	if ( !hSimpleCamParams )
	{
		hSimpleCamParams = ip->AddRollupPage(
				hInstance,
				MAKEINTRESOURCE(IDD_FCAMERAPARAM),  // DS 8/15/00 
				//hasTarget ? MAKEINTRESOURCE(IDD_SCAMERAPARAM) : MAKEINTRESOURCE(IDD_FCAMERAPARAM),
				SimpleCamParamDialogProc,
				GetString(IDS_RB_PARAMETERS),
				(LPARAM)this);

		ip->RegisterDlgWnd(hSimpleCamParams);

		{
			iFovType = GetICustButton(GetDlgItem(hSimpleCamParams,IDC_FOV_TYPE));
			iFovType->SetType(CBT_CHECK);
			iFovType->SetImage(hCamImages,0,0,0,0,14,14);
			FlyOffData fod[3] = {
				{ 0,0,0,0 },
				{ 1,1,1,1 },
				{ 2,2,2,2 }
				};
			iFovType->SetFlyOff(3,fod,0/*timeout*/,0/*init val*/,FLY_DOWN);
			iFovType->SetCurFlyOff(GetFOVType());  // DS 8/8/00
//			ttips[2] = GetResString(IDS_DB_BACKGROUND);
//			iFovType->SetTooltip(1, ttips[2]);
		}

		fovSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_FOVSPINNER));
		fovSpin->SetLimits( MIN_FOV, MAX_FOV, FALSE );
		fovSpin->SetValue(RadToDeg(WFOVtoCurFOV(GetFOV(ip->GetTime()))), FALSE);
		fovSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_FOV), EDITTYPE_FLOAT );
			
		lensSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_LENSSPINNER));
		lensSpin->SetLimits( MIN_LENS, MAX_LENS, FALSE );
		lensSpin->SetValue(FOVtoMM(GetFOV(ip->GetTime())), FALSE);
		lensSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_LENS), EDITTYPE_FLOAT );
			
		hitherSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_HITHER_SPIN));
		hitherSpin->SetLimits( MIN_CLIP, MAX_CLIP, FALSE );
		hitherSpin->SetValue(GetClipDist(ip->GetTime(), CAM_HITHER_CLIP), FALSE);
		// xavier robitaille | 03.02.07 | increments proportional to the spinner value
		hitherSpin->SetAutoScale();
		hitherSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_HITHER), EDITTYPE_UNIVERSE );
			
		yonSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_YON_SPIN));
		yonSpin->SetLimits( MIN_CLIP, MAX_CLIP, FALSE );
		yonSpin->SetValue(GetClipDist(ip->GetTime(), CAM_YON_CLIP), FALSE);
		// xavier robitaille | 03.02.07 | increments proportional to the spinner value
		yonSpin->SetAutoScale();
		yonSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_YON), EDITTYPE_UNIVERSE );

#if !defined (WEBVERSION) && !defined(NO_CAMERA_ENVRANGE)
		envNearSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_NEAR_RANGE_SPIN));
		envNearSpin->SetLimits( 0.0f, 999999.0f, FALSE );
		envNearSpin->SetValue(GetEnvRange(ip->GetTime(), ENV_NEAR_RANGE), FALSE);
		// alexc | 03.06.09 | increments proportional to the spinner value
        envNearSpin->SetAutoScale();
		envNearSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_NEAR_RANGE), EDITTYPE_UNIVERSE );
			
		envFarSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_FAR_RANGE_SPIN));
		envFarSpin->SetLimits( 0.0f, 999999.0f, FALSE );
		envFarSpin->SetValue(GetEnvRange(ip->GetTime(), ENV_FAR_RANGE), FALSE);
		// alexc | 03.06.09 | increments proportional to the spinner value
        envFarSpin->SetAutoScale();
		envFarSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_FAR_RANGE), EDITTYPE_UNIVERSE );
#endif

// DS- 8/15/00 -   
//		if(!hasTarget)
//		{
			tdistSpin = GetISpinner(GetDlgItem(hSimpleCamParams,IDC_TDISTSPINNER));
			tdistSpin->SetLimits( MIN_TDIST, MAX_TDIST, FALSE );
			tdistSpin->SetValue(GetTDist(ip->GetTime()), FALSE);
			tdistSpin->LinkToEdit( GetDlgItem(hSimpleCamParams,IDC_TDIST), EDITTYPE_UNIVERSE );
			// xavier robitaille | 03.02.07 | increments proportional to the spinner value
			tdistSpin->SetAutoScale();
//		}
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01			
// mjm - begin - 07.17.00
		// display possible multipass camera effect rollup
		IMultiPassCameraEffect *pCurCameraEffect = GetIMultiPassCameraEffect();
		if (pCurCameraEffect)
		{
			pCurCameraEffect->BeginEditParams(ip, flags, prev);
		}
// mjm - end
#endif // NO_CAMERA_MULTIPASS

// nac - begin - 12.08.00
//		if ( IsCompatibleRenderer() )
//		{
//			hDepthOfFieldParams = ip->AddRollupPage( 
//								hInstance, 
//								MAKEINTRESOURCE(IDD_CAMERA_DOF),
//								SimpleCamParamDialogProc,
//								GetString(IDS_DOF),
//								(LPARAM)this
//								);
//			ip->RegisterDlgWnd(hDepthOfFieldParams);			
//			fStopSpin = GetISpinner(GetDlgItem(hDepthOfFieldParams,IDC_DOF_FSTOP_SPIN));
//			fStopSpin->SetLimits( MIN_FSTOP, MAX_FSTOP, FALSE );
//			fStopSpin->SetValue(GetDOFFStop(ip->GetTime()), FALSE);
//			fStopSpin->SetAutoScale( TRUE );
//			fStopSpin->LinkToEdit( GetDlgItem(hDepthOfFieldParams,IDC_DOF_FSTOP), EDITTYPE_FLOAT );
//		}
// nac - end
	}
	else
	{
		SetWindowLongPtr( hSimpleCamParams, GWLP_USERDATA, (LONG_PTR)this );
		
		fovSpin->SetValue(RadToDeg(WFOVtoCurFOV(GetFOV(ip->GetTime()))),FALSE);
		hitherSpin->SetValue(GetClipDist(ip->GetTime(), CAM_HITHER_CLIP),FALSE);
		yonSpin->SetValue(GetClipDist(ip->GetTime(), CAM_YON_CLIP),FALSE);
#if !defined (WEBVERSION) && !defined(NO_CAMERA_ENVRANGE)
		envNearSpin->SetValue(GetEnvRange(ip->GetTime(), ENV_NEAR_RANGE),FALSE);
		envFarSpin->SetValue(GetEnvRange(ip->GetTime(), ENV_FAR_RANGE),FALSE);
#endif
		// DS 8/15/00 
//		if(!hasTarget)
			tdistSpin->SetValue(GetTDist(ip->GetTime()),FALSE);

		SetConeState( IsDlgButtonChecked(hSimpleCamParams,IDC_SHOWCAMCONE) );
		SetHorzLineState( IsDlgButtonChecked(hSimpleCamParams,IDC_SHOWHORZLINE) );
#ifndef NO_CAMERA_ENVRANGE
		SetEnvDisplay( IsDlgButtonChecked(hSimpleCamParams,IDC_SHOW_RANGES) );
#endif // NO_CAMERA_ENVRANGE
		SetManualClip( IsDlgButtonChecked(hSimpleCamParams,IDC_MANUAL_CLIP) );

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
		// display possible multipass camera effect rollup
		IMultiPassCameraEffect *pCurCameraEffect = GetIMultiPassCameraEffect();
		if (pCurCameraEffect)
		{
			pCurCameraEffect->BeginEditParams(ip, flags, prev);
		}
// mjm - end
#endif // NO_CAMERA_MULTIPASS

// nac - begin - 12.08.00
//		if ( hDepthOfFieldParams )
//		{
//			fStopSpin->SetValue ( GetDOFFStop (ip->GetTime()), FALSE );
//			fStopSpin->SetAutoScale( TRUE );
//		}
// nac - end
	}
	SendMessage( GetDlgItem(hSimpleCamParams, IDC_CAM_TYPE), CB_SETCURSEL, hasTarget, (LPARAM)0 );
}
		
void SimpleCamera::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	dlgFOV = GetFOV(ip->GetTime());
	dlgShowCone = IsDlgButtonChecked(hSimpleCamParams, IDC_SHOWCAMCONE );
#ifndef NO_CAMERA_ENVRANGE
	dlgRangeDisplay = IsDlgButtonChecked(hSimpleCamParams, IDC_SHOW_RANGES );
#endif // NO_CAMERA_ENVRANGE
	dlgIsOrtho = IsDlgButtonChecked(hSimpleCamParams, IDC_IS_ORTHO);
	dlgShowHorzLine = IsDlgButtonChecked(hSimpleCamParams, IDC_SHOWHORZLINE );
	dlgClip = IsDlgButtonChecked(hSimpleCamParams, IDC_MANUAL_CLIP );
	dlgTDist = GetTDist(ip->GetTime());
	dlgHither = GetClipDist(ip->GetTime(), CAM_HITHER_CLIP);
	dlgYon = GetClipDist(ip->GetTime(), CAM_YON_CLIP);
	dlgNearRange = GetEnvRange(ip->GetTime(), ENV_NEAR_RANGE);
	dlgFarRange = GetEnvRange(ip->GetTime(), ENV_FAR_RANGE);

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
	dlgMultiPassEffectEnable = GetMultiPassEffectEnabled( ip->GetTime() );
	dlgMPEffect_REffectPerPass = GetMPEffect_REffectPerPass();

	IMultiPassCameraEffect *pCurCameraEffect = GetIMultiPassCameraEffect();
	if (pCurCameraEffect)
	{
		pCurCameraEffect->EndEditParams(ip, flags, prev);
	}
// mjm - end
#endif // NO_CAMERA_MULTIPASS

	// Depth of Field
// nac - begin - 12.08.00
//	dlgDOFEnable = GetDOFEnable( ip->GetTime() );
//	dlgDOFFStop = GetDOFFStop( ip->GetTime() );
// nac - end
	currentEditCam = NULL;

	if ( flags&END_EDIT_REMOVEUI )
	{
		if ( hDepthOfFieldParams )
		{
			ip->UnRegisterDlgWnd ( hDepthOfFieldParams );
			ip->DeleteRollupPage ( hDepthOfFieldParams );
			hDepthOfFieldParams = NULL;
		}

		ip->UnRegisterDlgWnd(hSimpleCamParams);
		ip->DeleteRollupPage(hSimpleCamParams);
		hSimpleCamParams = NULL;				
	}
	else
	{
		SetWindowLongPtr( hSimpleCamParams, GWLP_USERDATA, 0 );
		SetWindowLongPtr( hDepthOfFieldParams, GWLP_USERDATA, 0 );
	}
	iObjParams = NULL;
}


static void MakeQuad(Face *f, int a,  int b , int c , int d, int sg, int dv = 0) {
	f[0].setVerts( a+dv, b+dv, c+dv);
	f[0].setSmGroup(sg);
	f[0].setEdgeVisFlags(1,1,0);
	f[1].setVerts( c+dv, d+dv, a+dv);
	f[1].setSmGroup(sg);
	f[1].setEdgeVisFlags(1,1,0);
	}

void SimpleCamera::BuildMesh()	{
#ifndef USE_VIZR3_CAMERA_MESH
	mesh.setNumVerts(170+2);	//pw need to add 2 points to make the bounding box symmetrical
	mesh.setNumFaces(284);

	mesh.setVert(0, Point3(3.051641,3.476140,19.865290));
	mesh.setVert(1, Point3(3.051641,7.351941,19.865288));
	mesh.setVert(2, Point3(3.051641,10.708482,17.927383));
	mesh.setVert(3, Point3(3.051642,12.646385,14.570845));
	mesh.setVert(4, Point3(3.051642,12.646385,10.695045));
	mesh.setVert(5, Point3(3.051642,10.718940,7.343246));
	mesh.setVert(6, Point3(3.051642,7.362399,5.405346));
	mesh.setVert(7, Point3(3.051642,3.476141,5.400602));
	mesh.setVert(8, Point3(3.051642,0.119599,7.338503));
	mesh.setVert(9, Point3(3.051641,-1.818303,10.695044));
	mesh.setVert(10, Point3(3.051641,-1.818301,14.570848));
	mesh.setVert(11, Point3(3.051641,0.119600,17.927389));
	mesh.setVert(12, Point3(-2.927488,3.476141,19.865290));
	mesh.setVert(13, Point3(-2.927488,7.351941,19.865288));
	mesh.setVert(14, Point3(-2.927488,10.708485,17.927383));
	mesh.setVert(15, Point3(-2.927488,12.646383,14.570839));
	mesh.setVert(16, Point3(-2.927486,12.646383,10.695044));
	mesh.setVert(17, Point3(-2.927485,10.718940,7.343246));
	mesh.setVert(18, Point3(-2.927486,7.362398,5.405346));
	mesh.setVert(19, Point3(-2.927488,3.476141,5.400604));
	mesh.setVert(20, Point3(-2.927486,0.119599,7.338502));
	mesh.setVert(21, Point3(-2.927488,-1.818302,10.695044));
	mesh.setVert(22, Point3(-2.927488,-1.818301,14.570842));
	mesh.setVert(23, Point3(-2.927488,0.119600,17.927389));
	mesh.setVert(24, Point3(3.051642,14.605198,7.347991));
	mesh.setVert(25, Point3(3.051642,17.961739,5.410092));
	mesh.setVert(26, Point3(3.051642,19.899639,2.053550));
	mesh.setVert(27, Point3(3.051643,19.899641,-1.822252));
	mesh.setVert(28, Point3(3.051643,17.961742,-5.178796));
	mesh.setVert(29, Point3(3.051643,14.605200,-7.116695));
	mesh.setVert(30, Point3(3.051644,10.729396,-7.116697));
	mesh.setVert(31, Point3(3.051644,7.372853,-5.178795));
	mesh.setVert(32, Point3(3.051642,5.434954,-1.822254));
	mesh.setVert(33, Point3(3.051642,5.434955,2.053549));
	mesh.setVert(34, Point3(-2.927486,14.605195,7.347991));
	mesh.setVert(35, Point3(-2.927485,17.961739,5.410092));
	mesh.setVert(36, Point3(-2.927485,19.899643,2.053545));
	mesh.setVert(37, Point3(-2.927485,19.899641,-1.822254));
	mesh.setVert(38, Point3(-2.927485,17.961742,-5.178796));
	mesh.setVert(39, Point3(-2.927485,14.605194,-7.116697));
	mesh.setVert(40, Point3(-2.927485,10.729397,-7.116697));
	mesh.setVert(41, Point3(-2.927485,7.372854,-5.178796));
	mesh.setVert(42, Point3(-2.927485,5.434955,-1.822254));
	mesh.setVert(43, Point3(-2.927485,5.434955,2.053546));
	mesh.setVert(44, Point3(-4.024377,-5.645106,-6.857333));
	mesh.setVert(45, Point3(-4.024381,-5.645108,5.771017));
	mesh.setVert(46, Point3(4.298551,-5.645106,-6.857333));
	mesh.setVert(47, Point3(4.298550,-5.645107,5.771020));
	mesh.setVert(48, Point3(-4.024377,5.452132,-5.590975));
	mesh.setVert(49, Point3(4.298551,5.452132,-5.590975));
	mesh.setVert(50, Point3(-4.024381,-2.215711,8.686206));
	mesh.setVert(51, Point3(4.298549,-2.215710,8.686206));
	mesh.setVert(52, Point3(4.298550,5.452131,4.260479));
	mesh.setVert(53, Point3(-4.024381,5.452131,4.260478));
	mesh.setVert(54, Point3(4.298552,-2.673011,-9.176138));
	mesh.setVert(55, Point3(-4.024377,-2.673013,-9.176139));
	mesh.setVert(56, Point3(-4.024377,2.446853,-9.185402));
	mesh.setVert(57, Point3(4.298551,2.446853,-9.185402));
	mesh.setVert(58, Point3(-3.974360,3.286706,-5.111160));
	mesh.setVert(59, Point3(-3.974360,3.778829,-5.883637));
	mesh.setVert(60, Point3(-3.974360,3.580588,-6.777846));
	mesh.setVert(61, Point3(-3.974360,2.808110,-7.269970));
	mesh.setVert(62, Point3(-3.974360,1.913902,-7.071730));
	mesh.setVert(63, Point3(-3.974360,1.421778,-6.299250));
	mesh.setVert(64, Point3(-3.974360,1.620020,-5.405043));
	mesh.setVert(65, Point3(-3.974360,2.392498,-4.912918));
	mesh.setVert(66, Point3(-4.842994,3.493080,-4.816429));
	mesh.setVert(67, Point3(-5.090842,4.044086,-5.504812));
	mesh.setVert(68, Point3(-5.689195,3.988004,-6.195997));
	mesh.setVert(69, Point3(-6.287547,3.357684,-6.485096));
	mesh.setVert(70, Point3(-6.535392,2.522360,-6.202761));
	mesh.setVert(71, Point3(-6.287547,1.971353,-5.514376));
	mesh.setVert(72, Point3(-5.689195,2.027436,-4.823191));
	mesh.setVert(73, Point3(-5.090842,2.657756,-4.534092));
	mesh.setVert(74, Point3(-5.202796,3.991309,-4.104883));
	mesh.setVert(75, Point3(-5.553304,4.684474,-4.590243));
	mesh.setVert(76, Point3(-6.399501,4.971593,-4.791286));
	mesh.setVert(77, Point3(-7.245704,4.684475,-4.590243));
	mesh.setVert(78, Point3(-7.596208,3.991309,-4.104883));
	mesh.setVert(79, Point3(-7.245700,3.298144,-3.619523));
	mesh.setVert(80, Point3(-6.399501,3.011024,-3.418482));
	mesh.setVert(81, Point3(-5.553304,3.298143,-3.619523));
	mesh.setVert(82, Point3(-4.700181,4.326933,-3.625562));
	mesh.setVert(83, Point3(-5.197900,5.311228,-4.314773));
	mesh.setVert(84, Point3(-6.399501,5.718936,-4.600252));
	mesh.setVert(85, Point3(-7.601106,5.311228,-4.314773));
	mesh.setVert(86, Point3(-8.098825,4.326933,-3.625562));
	mesh.setVert(87, Point3(-7.601106,3.342639,-2.936350));
	mesh.setVert(88, Point3(-6.399501,2.934930,-2.650871));
	mesh.setVert(89, Point3(-5.197900,3.342638,-2.936352));
	mesh.setVert(90, Point3(-2.228243,-0.091865,-9.172746));
	mesh.setVert(91, Point3(-2.073412,0.786230,-9.172744));
	mesh.setVert(92, Point3(-1.627592,1.558412,-9.172746));
	mesh.setVert(93, Point3(-0.944555,2.131548,-9.172746));
	mesh.setVert(94, Point3(-0.106688,2.436507,-9.172744));
	mesh.setVert(95, Point3(0.784952,2.436507,-9.172744));
	mesh.setVert(96, Point3(1.622821,2.131547,-9.172746));
	mesh.setVert(97, Point3(2.305858,1.558413,-9.172746));
	mesh.setVert(98, Point3(2.751678,0.786228,-9.172744));
	mesh.setVert(99, Point3(2.906510,-0.091866,-9.172746));
	mesh.setVert(100, Point3(2.751678,-0.969961,-9.172746));
	mesh.setVert(101, Point3(2.305857,-1.742143,-9.172744));
	mesh.setVert(102, Point3(1.622820,-2.315279,-9.172744));
	mesh.setVert(103, Point3(0.784952,-2.620237,-9.172746));
	mesh.setVert(104, Point3(-0.106688,-2.620237,-9.172746));
	mesh.setVert(105, Point3(-0.944556,-2.315279,-9.172746));
	mesh.setVert(106, Point3(-1.627593,-1.742142,-9.172744));
	mesh.setVert(107, Point3(-2.073413,-0.969958,-9.172747));
	mesh.setVert(108, Point3(-2.228243,-0.091864,-14.836016));
	mesh.setVert(109, Point3(-2.073412,0.786230,-14.836016));
	mesh.setVert(110, Point3(-1.627591,1.558412,-14.836016));
	mesh.setVert(111, Point3(-0.944554,2.131549,-14.836016));
	mesh.setVert(112, Point3(-0.106687,2.436507,-14.836020));
	mesh.setVert(113, Point3(0.784954,2.436507,-14.836020));
	mesh.setVert(114, Point3(1.622822,2.131548,-14.836020));
	mesh.setVert(115, Point3(2.305858,1.558412,-14.836016));
	mesh.setVert(116, Point3(2.751678,0.786230,-14.836020));
	mesh.setVert(117, Point3(2.906510,-0.091865,-14.836020));
	mesh.setVert(118, Point3(2.751679,-0.969959,-14.836016));
	mesh.setVert(119, Point3(2.305858,-1.742141,-14.836016));
	mesh.setVert(120, Point3(1.622821,-2.315277,-14.836016));
	mesh.setVert(121, Point3(0.784952,-2.620236,-14.836016));
	mesh.setVert(122, Point3(-0.106688,-2.620236,-14.836016));
	mesh.setVert(123, Point3(-0.944555,-2.315276,-14.836016));
	mesh.setVert(124, Point3(-1.627592,-1.742141,-14.836016));
	mesh.setVert(125, Point3(-2.073412,-0.969958,-14.836016));
	mesh.setVert(126, Point3(-2.774064,-0.091865,-16.139450));
	mesh.setVert(127, Point3(-2.586314,0.972911,-16.139450));
	mesh.setVert(128, Point3(-2.045713,1.909259,-16.139450));
	mesh.setVert(129, Point3(-1.217464,2.604243,-16.139450));
	mesh.setVert(130, Point3(-0.201468,2.974035,-16.139450));
	mesh.setVert(131, Point3(0.879735,2.974035,-16.139450));
	mesh.setVert(132, Point3(1.895732,2.604242,-16.139450));
	mesh.setVert(133, Point3(2.723981,1.909260,-16.139450));
	mesh.setVert(134, Point3(3.264582,0.972910,-16.139450));
	mesh.setVert(135, Point3(3.452331,-0.091865,-16.139450));
	mesh.setVert(136, Point3(3.264582,-1.156641,-16.139450));
	mesh.setVert(137, Point3(2.723980,-2.092987,-16.139450));
	mesh.setVert(138, Point3(1.895732,-2.787971,-16.139450));
	mesh.setVert(139, Point3(0.879734,-3.157765,-16.139450));
	mesh.setVert(140, Point3(-0.201468,-3.157763,-16.139450));
	mesh.setVert(141, Point3(-1.217465,-2.787971,-16.139450));
	mesh.setVert(142, Point3(-2.045714,-2.092988,-16.139450));
	mesh.setVert(143, Point3(-2.586316,-1.156639,-16.139450));
	mesh.setVert(144, Point3(-2.774064,-0.091864,-19.194777));
	mesh.setVert(145, Point3(-2.586314,0.972911,-19.194771));
	mesh.setVert(146, Point3(-2.045713,1.909259,-19.194773));
	mesh.setVert(147, Point3(-1.217464,2.604243,-19.194773));
	mesh.setVert(148, Point3(-0.201467,2.974036,-19.194771));
	mesh.setVert(149, Point3(0.879735,2.974034,-19.194771));
	mesh.setVert(150, Point3(1.895733,2.604243,-19.194773));
	mesh.setVert(151, Point3(2.723981,1.909260,-19.194773));
	mesh.setVert(152, Point3(3.264582,0.972910,-19.194771));
	mesh.setVert(153, Point3(3.452332,-0.091865,-19.194773));
	mesh.setVert(154, Point3(3.264582,-1.156642,-19.194771));
	mesh.setVert(155, Point3(2.723981,-2.092989,-19.194773));
	mesh.setVert(156, Point3(1.895732,-2.787971,-19.194773));
	mesh.setVert(157, Point3(0.879734,-3.157765,-19.194771));
	mesh.setVert(158, Point3(-0.201468,-3.157763,-19.194771));
	mesh.setVert(159, Point3(-1.217464,-2.787970,-19.194777));
	mesh.setVert(160, Point3(-2.045714,-2.092988,-19.194777));
	mesh.setVert(161, Point3(-2.586316,-1.156640,-19.194773));
	mesh.setVert(162, Point3(5.033586,-3.077510,-19.223640));
	mesh.setVert(163, Point3(-4.204867,-3.077511,-19.223642));
	mesh.setVert(164, Point3(5.033587,3.017310,-19.223642));
	mesh.setVert(165, Point3(-4.204867,3.017310,-19.223642));
	mesh.setVert(166, Point3(8.950690,-5.661717,-23.902903));
	mesh.setVert(167, Point3(-8.121970,-5.661717,-23.902897));
	mesh.setVert(168, Point3(8.950689,5.601515,-23.902897));
	mesh.setVert(169, Point3(-8.121970,5.601515,-23.902897));

//pw need to ceate 2 points to make the bounding box symmetrical
	Box3 bbox;
	bbox.Init();
	for (int i =0; i < 170; i++)
		{
		bbox += mesh.getVert(i);
		}
	Point3 minVec = bbox.pmin;
	Point3 maxVec = bbox.pmax;
	float minLen = Length(minVec);
	float maxLen = Length(maxVec);
	if (fabs(minVec.x) > fabs(maxVec.x))
		{
		maxVec.x = -minVec.x;
		}
	else
		{
		minVec.x = -maxVec.x;
		}

	if (fabs(minVec.y) > fabs(maxVec.y))
		{
		maxVec.y = -minVec.y;
		}
	else
		{
		minVec.y = -maxVec.y;
		}

	if (fabs(minVec.z) > fabs(maxVec.z))
		{
		maxVec.z = -minVec.z;
		}
	else
		{
		minVec.z = -maxVec.z;
		}


	mesh.setVert(170, maxVec);
	mesh.setVert(171, minVec);

	Face f;

	f.v[0] = 10;  f.v[1] = 9;  f.v[2] = 8;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[0] = f;
	f.v[0] = 8;  f.v[1] = 7;  f.v[2] = 6;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[1] = f;
	f.v[0] = 6;  f.v[1] = 5;  f.v[2] = 4;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[2] = f;
	f.v[0] = 8;  f.v[1] = 6;  f.v[2] = 4;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[3] = f;
	f.v[0] = 4;  f.v[1] = 3;  f.v[2] = 2;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[4] = f;
	f.v[0] = 2;  f.v[1] = 1;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[5] = f;
	f.v[0] = 4;  f.v[1] = 2;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[6] = f;
	f.v[0] = 8;  f.v[1] = 4;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[7] = f;
	f.v[0] = 10;  f.v[1] = 8;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[8] = f;
	f.v[0] = 11;  f.v[1] = 10;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 65541; mesh.faces[9] = f;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 13;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[10] = f;
	f.v[0] = 13;  f.v[1] = 12;  f.v[2] = 0;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[11] = f;
	f.v[0] = 1;  f.v[1] = 2;  f.v[2] = 14;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[12] = f;
	f.v[0] = 14;  f.v[1] = 13;  f.v[2] = 1;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[13] = f;
	f.v[0] = 2;  f.v[1] = 3;  f.v[2] = 15;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[14] = f;
	f.v[0] = 15;  f.v[1] = 14;  f.v[2] = 2;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[15] = f;
	f.v[0] = 3;  f.v[1] = 4;  f.v[2] = 16;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[16] = f;
	f.v[0] = 16;  f.v[1] = 15;  f.v[2] = 3;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[17] = f;
	f.v[0] = 4;  f.v[1] = 5;  f.v[2] = 17;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[18] = f;
	f.v[0] = 17;  f.v[1] = 16;  f.v[2] = 4;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[19] = f;
	f.v[0] = 6;  f.v[1] = 7;  f.v[2] = 19;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[20] = f;
	f.v[0] = 19;  f.v[1] = 18;  f.v[2] = 6;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[21] = f;
	f.v[0] = 8;  f.v[1] = 9;  f.v[2] = 21;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[22] = f;
	f.v[0] = 21;  f.v[1] = 20;  f.v[2] = 8;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[23] = f;
	f.v[0] = 10;  f.v[1] = 11;  f.v[2] = 23;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[24] = f;
	f.v[0] = 23;  f.v[1] = 22;  f.v[2] = 10;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[25] = f;
	f.v[0] = 11;  f.v[1] = 0;  f.v[2] = 12;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[26] = f;
	f.v[0] = 12;  f.v[1] = 23;  f.v[2] = 11;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[27] = f;
	f.v[0] = 12;  f.v[1] = 13;  f.v[2] = 14;  	f.smGroup = 1;  f.flags = 3; mesh.faces[28] = f;
	f.v[0] = 14;  f.v[1] = 15;  f.v[2] = 16;  	f.smGroup = 1;  f.flags = 3; mesh.faces[29] = f;
	f.v[0] = 16;  f.v[1] = 17;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 3; mesh.faces[30] = f;
	f.v[0] = 14;  f.v[1] = 16;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 0; mesh.faces[31] = f;
	f.v[0] = 18;  f.v[1] = 19;  f.v[2] = 20;  	f.smGroup = 1;  f.flags = 3; mesh.faces[32] = f;
	f.v[0] = 20;  f.v[1] = 21;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 3; mesh.faces[33] = f;
	f.v[0] = 18;  f.v[1] = 20;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 0; mesh.faces[34] = f;
	f.v[0] = 14;  f.v[1] = 18;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 0; mesh.faces[35] = f;
	f.v[0] = 12;  f.v[1] = 14;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 0; mesh.faces[36] = f;
	f.v[0] = 23;  f.v[1] = 12;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 5; mesh.faces[37] = f;
	f.v[0] = 33;  f.v[1] = 32;  f.v[2] = 31;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[38] = f;
	f.v[0] = 31;  f.v[1] = 30;  f.v[2] = 29;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[39] = f;
	f.v[0] = 29;  f.v[1] = 28;  f.v[2] = 27;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[40] = f;
	f.v[0] = 31;  f.v[1] = 29;  f.v[2] = 27;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[41] = f;
	f.v[0] = 27;  f.v[1] = 26;  f.v[2] = 25;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[42] = f;
	f.v[0] = 25;  f.v[1] = 24;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[43] = f;
	f.v[0] = 27;  f.v[1] = 25;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[44] = f;
	f.v[0] = 31;  f.v[1] = 27;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[45] = f;
	f.v[0] = 33;  f.v[1] = 31;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[46] = f;
	f.v[0] = 6;  f.v[1] = 33;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 65541; mesh.faces[47] = f;
	f.v[0] = 5;  f.v[1] = 24;  f.v[2] = 34;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[48] = f;
	f.v[0] = 34;  f.v[1] = 17;  f.v[2] = 5;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[49] = f;
	f.v[0] = 24;  f.v[1] = 25;  f.v[2] = 35;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[50] = f;
	f.v[0] = 35;  f.v[1] = 34;  f.v[2] = 24;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[51] = f;
	f.v[0] = 25;  f.v[1] = 26;  f.v[2] = 36;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[52] = f;
	f.v[0] = 36;  f.v[1] = 35;  f.v[2] = 25;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[53] = f;
	f.v[0] = 26;  f.v[1] = 27;  f.v[2] = 37;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[54] = f;
	f.v[0] = 37;  f.v[1] = 36;  f.v[2] = 26;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[55] = f;
	f.v[0] = 27;  f.v[1] = 28;  f.v[2] = 38;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[56] = f;
	f.v[0] = 38;  f.v[1] = 37;  f.v[2] = 27;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[57] = f;
	f.v[0] = 28;  f.v[1] = 29;  f.v[2] = 39;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[58] = f;
	f.v[0] = 39;  f.v[1] = 38;  f.v[2] = 28;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[59] = f;
	f.v[0] = 29;  f.v[1] = 30;  f.v[2] = 40;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[60] = f;
	f.v[0] = 40;  f.v[1] = 39;  f.v[2] = 29;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[61] = f;
	f.v[0] = 30;  f.v[1] = 31;  f.v[2] = 41;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[62] = f;
	f.v[0] = 41;  f.v[1] = 40;  f.v[2] = 30;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[63] = f;
	f.v[0] = 31;  f.v[1] = 32;  f.v[2] = 42;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[64] = f;
	f.v[0] = 42;  f.v[1] = 41;  f.v[2] = 31;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[65] = f;
	f.v[0] = 33;  f.v[1] = 6;  f.v[2] = 18;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[66] = f;
	f.v[0] = 18;  f.v[1] = 43;  f.v[2] = 33;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[67] = f;
	f.v[0] = 17;  f.v[1] = 34;  f.v[2] = 35;  	f.smGroup = 1;  f.flags = 3; mesh.faces[68] = f;
	f.v[0] = 35;  f.v[1] = 36;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 3; mesh.faces[69] = f;
	f.v[0] = 37;  f.v[1] = 38;  f.v[2] = 39;  	f.smGroup = 1;  f.flags = 3; mesh.faces[70] = f;
	f.v[0] = 35;  f.v[1] = 37;  f.v[2] = 39;  	f.smGroup = 1;  f.flags = 0; mesh.faces[71] = f;
	f.v[0] = 39;  f.v[1] = 40;  f.v[2] = 41;  	f.smGroup = 1;  f.flags = 3; mesh.faces[72] = f;
	f.v[0] = 41;  f.v[1] = 42;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 3; mesh.faces[73] = f;
	f.v[0] = 39;  f.v[1] = 41;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 0; mesh.faces[74] = f;
	f.v[0] = 35;  f.v[1] = 39;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 0; mesh.faces[75] = f;
	f.v[0] = 17;  f.v[1] = 35;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 0; mesh.faces[76] = f;
	f.v[0] = 18;  f.v[1] = 17;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 5; mesh.faces[77] = f;
	f.v[0] = 47;  f.v[1] = 45;  f.v[2] = 44;  	f.smGroup = 4;  f.flags = 65539; mesh.faces[78] = f;
	f.v[0] = 44;  f.v[1] = 46;  f.v[2] = 47;  	f.smGroup = 4;  f.flags = 65539; mesh.faces[79] = f;
	f.v[0] = 52;  f.v[1] = 53;  f.v[2] = 50;  	f.smGroup = 16;  f.flags = 196611; mesh.faces[80] = f;
	f.v[0] = 50;  f.v[1] = 51;  f.v[2] = 52;  	f.smGroup = 16;  f.flags = 196611; mesh.faces[81] = f;
	f.v[0] = 56;  f.v[1] = 57;  f.v[2] = 54;  	f.smGroup = 64;  f.flags = 131075; mesh.faces[82] = f;
	f.v[0] = 54;  f.v[1] = 55;  f.v[2] = 56;  	f.smGroup = 64;  f.flags = 131075; mesh.faces[83] = f;
	f.v[0] = 51;  f.v[1] = 50;  f.v[2] = 45;  	f.smGroup = 0;  f.flags = 3; mesh.faces[84] = f;
	f.v[0] = 45;  f.v[1] = 47;  f.v[2] = 51;  	f.smGroup = 0;  f.flags = 3; mesh.faces[85] = f;
	f.v[0] = 55;  f.v[1] = 54;  f.v[2] = 46;  	f.smGroup = 0;  f.flags = 3; mesh.faces[86] = f;
	f.v[0] = 46;  f.v[1] = 44;  f.v[2] = 55;  	f.smGroup = 0;  f.flags = 3; mesh.faces[87] = f;
	f.v[0] = 57;  f.v[1] = 56;  f.v[2] = 48;  	f.smGroup = 0;  f.flags = 3; mesh.faces[88] = f;
	f.v[0] = 48;  f.v[1] = 49;  f.v[2] = 57;  	f.smGroup = 0;  f.flags = 3; mesh.faces[89] = f;
	f.v[0] = 52;  f.v[1] = 49;  f.v[2] = 48;  	f.smGroup = 0;  f.flags = 3; mesh.faces[90] = f;
	f.v[0] = 48;  f.v[1] = 53;  f.v[2] = 52;  	f.smGroup = 0;  f.flags = 3; mesh.faces[91] = f;
	f.v[0] = 56;  f.v[1] = 55;  f.v[2] = 44;  	f.smGroup = 0;  f.flags = 3; mesh.faces[92] = f;
	f.v[0] = 56;  f.v[1] = 44;  f.v[2] = 45;  	f.smGroup = 0;  f.flags = 2; mesh.faces[93] = f;
	f.v[0] = 56;  f.v[1] = 45;  f.v[2] = 50;  	f.smGroup = 0;  f.flags = 2; mesh.faces[94] = f;
	f.v[0] = 56;  f.v[1] = 50;  f.v[2] = 53;  	f.smGroup = 0;  f.flags = 2; mesh.faces[95] = f;
	f.v[0] = 48;  f.v[1] = 56;  f.v[2] = 53;  	f.smGroup = 0;  f.flags = 5; mesh.faces[96] = f;
	f.v[0] = 51;  f.v[1] = 47;  f.v[2] = 46;  	f.smGroup = 0;  f.flags = 3; mesh.faces[97] = f;
	f.v[0] = 51;  f.v[1] = 46;  f.v[2] = 54;  	f.smGroup = 0;  f.flags = 2; mesh.faces[98] = f;
	f.v[0] = 51;  f.v[1] = 54;  f.v[2] = 57;  	f.smGroup = 0;  f.flags = 2; mesh.faces[99] = f;
	f.v[0] = 51;  f.v[1] = 57;  f.v[2] = 49;  	f.smGroup = 0;  f.flags = 2; mesh.faces[100] = f;
	f.v[0] = 52;  f.v[1] = 51;  f.v[2] = 49;  	f.smGroup = 0;  f.flags = 5; mesh.faces[101] = f;
	f.v[0] = 22;  f.v[1] = 21;  f.v[2] = 9;  	f.smGroup = 0;  f.flags = 3; mesh.faces[102] = f;
	f.v[0] = 9;  f.v[1] = 10;  f.v[2] = 22;  	f.smGroup = 0;  f.flags = 3; mesh.faces[103] = f;
	f.v[0] = 60;  f.v[1] = 59;  f.v[2] = 58;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[104] = f;
	f.v[0] = 58;  f.v[1] = 65;  f.v[2] = 64;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[105] = f;
	f.v[0] = 64;  f.v[1] = 63;  f.v[2] = 62;  	f.smGroup = 1;  f.flags = 65539; mesh.faces[106] = f;
	f.v[0] = 58;  f.v[1] = 64;  f.v[2] = 62;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[107] = f;
	f.v[0] = 60;  f.v[1] = 58;  f.v[2] = 62;  	f.smGroup = 1;  f.flags = 65536; mesh.faces[108] = f;
	f.v[0] = 60;  f.v[1] = 62;  f.v[2] = 61;  	f.smGroup = 1;  f.flags = 65542; mesh.faces[109] = f;
	f.v[0] = 59;  f.v[1] = 60;  f.v[2] = 68;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[110] = f;
	f.v[0] = 59;  f.v[1] = 68;  f.v[2] = 67;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[111] = f;
	f.v[0] = 67;  f.v[1] = 66;  f.v[2] = 58;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[112] = f;
	f.v[0] = 59;  f.v[1] = 67;  f.v[2] = 58;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[113] = f;
	f.v[0] = 61;  f.v[1] = 62;  f.v[2] = 70;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[114] = f;
	f.v[0] = 61;  f.v[1] = 70;  f.v[2] = 69;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[115] = f;
	f.v[0] = 69;  f.v[1] = 68;  f.v[2] = 60;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[116] = f;
	f.v[0] = 61;  f.v[1] = 69;  f.v[2] = 60;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[117] = f;
	f.v[0] = 63;  f.v[1] = 64;  f.v[2] = 72;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[118] = f;
	f.v[0] = 63;  f.v[1] = 72;  f.v[2] = 71;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[119] = f;
	f.v[0] = 71;  f.v[1] = 70;  f.v[2] = 62;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[120] = f;
	f.v[0] = 63;  f.v[1] = 71;  f.v[2] = 62;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[121] = f;
	f.v[0] = 65;  f.v[1] = 58;  f.v[2] = 66;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[122] = f;
	f.v[0] = 65;  f.v[1] = 66;  f.v[2] = 73;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[123] = f;
	f.v[0] = 73;  f.v[1] = 72;  f.v[2] = 64;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[124] = f;
	f.v[0] = 65;  f.v[1] = 73;  f.v[2] = 64;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[125] = f;
	f.v[0] = 67;  f.v[1] = 68;  f.v[2] = 76;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[126] = f;
	f.v[0] = 67;  f.v[1] = 76;  f.v[2] = 75;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[127] = f;
	f.v[0] = 75;  f.v[1] = 74;  f.v[2] = 66;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[128] = f;
	f.v[0] = 67;  f.v[1] = 75;  f.v[2] = 66;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[129] = f;
	f.v[0] = 69;  f.v[1] = 70;  f.v[2] = 78;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[130] = f;
	f.v[0] = 69;  f.v[1] = 78;  f.v[2] = 77;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[131] = f;
	f.v[0] = 77;  f.v[1] = 76;  f.v[2] = 68;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[132] = f;
	f.v[0] = 69;  f.v[1] = 77;  f.v[2] = 68;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[133] = f;
	f.v[0] = 71;  f.v[1] = 72;  f.v[2] = 80;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[134] = f;
	f.v[0] = 71;  f.v[1] = 80;  f.v[2] = 79;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[135] = f;
	f.v[0] = 79;  f.v[1] = 78;  f.v[2] = 70;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[136] = f;
	f.v[0] = 71;  f.v[1] = 79;  f.v[2] = 70;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[137] = f;
	f.v[0] = 73;  f.v[1] = 66;  f.v[2] = 74;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[138] = f;
	f.v[0] = 73;  f.v[1] = 74;  f.v[2] = 81;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[139] = f;
	f.v[0] = 81;  f.v[1] = 80;  f.v[2] = 72;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[140] = f;
	f.v[0] = 73;  f.v[1] = 81;  f.v[2] = 72;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[141] = f;
	f.v[0] = 86;  f.v[1] = 87;  f.v[2] = 88;  	f.smGroup = 1;  f.flags = 3; mesh.faces[142] = f;
	f.v[0] = 88;  f.v[1] = 89;  f.v[2] = 82;  	f.smGroup = 1;  f.flags = 3; mesh.faces[143] = f;
	f.v[0] = 82;  f.v[1] = 83;  f.v[2] = 84;  	f.smGroup = 1;  f.flags = 3; mesh.faces[144] = f;
	f.v[0] = 88;  f.v[1] = 82;  f.v[2] = 84;  	f.smGroup = 1;  f.flags = 0; mesh.faces[145] = f;
	f.v[0] = 86;  f.v[1] = 88;  f.v[2] = 84;  	f.smGroup = 1;  f.flags = 0; mesh.faces[146] = f;
	f.v[0] = 86;  f.v[1] = 84;  f.v[2] = 85;  	f.smGroup = 1;  f.flags = 6; mesh.faces[147] = f;
	f.v[0] = 75;  f.v[1] = 76;  f.v[2] = 84;  	f.smGroup = 0;  f.flags = 3; mesh.faces[148] = f;
	f.v[0] = 75;  f.v[1] = 84;  f.v[2] = 83;  	f.smGroup = 0;  f.flags = 2; mesh.faces[149] = f;
	f.v[0] = 83;  f.v[1] = 82;  f.v[2] = 74;  	f.smGroup = 0;  f.flags = 3; mesh.faces[150] = f;
	f.v[0] = 75;  f.v[1] = 83;  f.v[2] = 74;  	f.smGroup = 0;  f.flags = 4; mesh.faces[151] = f;
	f.v[0] = 77;  f.v[1] = 78;  f.v[2] = 86;  	f.smGroup = 0;  f.flags = 3; mesh.faces[152] = f;
	f.v[0] = 77;  f.v[1] = 86;  f.v[2] = 85;  	f.smGroup = 0;  f.flags = 2; mesh.faces[153] = f;
	f.v[0] = 85;  f.v[1] = 84;  f.v[2] = 76;  	f.smGroup = 0;  f.flags = 3; mesh.faces[154] = f;
	f.v[0] = 77;  f.v[1] = 85;  f.v[2] = 76;  	f.smGroup = 0;  f.flags = 4; mesh.faces[155] = f;
	f.v[0] = 79;  f.v[1] = 80;  f.v[2] = 88;  	f.smGroup = 0;  f.flags = 3; mesh.faces[156] = f;
	f.v[0] = 79;  f.v[1] = 88;  f.v[2] = 87;  	f.smGroup = 0;  f.flags = 2; mesh.faces[157] = f;
	f.v[0] = 87;  f.v[1] = 86;  f.v[2] = 78;  	f.smGroup = 0;  f.flags = 3; mesh.faces[158] = f;
	f.v[0] = 79;  f.v[1] = 87;  f.v[2] = 78;  	f.smGroup = 0;  f.flags = 4; mesh.faces[159] = f;
	f.v[0] = 81;  f.v[1] = 74;  f.v[2] = 82;  	f.smGroup = 0;  f.flags = 3; mesh.faces[160] = f;
	f.v[0] = 81;  f.v[1] = 82;  f.v[2] = 89;  	f.smGroup = 0;  f.flags = 2; mesh.faces[161] = f;
	f.v[0] = 89;  f.v[1] = 88;  f.v[2] = 80;  	f.smGroup = 0;  f.flags = 3; mesh.faces[162] = f;
	f.v[0] = 81;  f.v[1] = 89;  f.v[2] = 80;  	f.smGroup = 0;  f.flags = 4; mesh.faces[163] = f;
	f.v[0] = 108;  f.v[1] = 125;  f.v[2] = 107;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[164] = f;
	f.v[0] = 108;  f.v[1] = 107;  f.v[2] = 90;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[165] = f;
	f.v[0] = 90;  f.v[1] = 91;  f.v[2] = 109;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[166] = f;
	f.v[0] = 108;  f.v[1] = 90;  f.v[2] = 109;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[167] = f;
	f.v[0] = 92;  f.v[1] = 93;  f.v[2] = 111;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[168] = f;
	f.v[0] = 92;  f.v[1] = 111;  f.v[2] = 110;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[169] = f;
	f.v[0] = 110;  f.v[1] = 109;  f.v[2] = 91;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[170] = f;
	f.v[0] = 92;  f.v[1] = 110;  f.v[2] = 91;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[171] = f;
	f.v[0] = 94;  f.v[1] = 95;  f.v[2] = 113;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[172] = f;
	f.v[0] = 94;  f.v[1] = 113;  f.v[2] = 112;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[173] = f;
	f.v[0] = 112;  f.v[1] = 111;  f.v[2] = 93;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[174] = f;
	f.v[0] = 94;  f.v[1] = 112;  f.v[2] = 93;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[175] = f;
	f.v[0] = 96;  f.v[1] = 97;  f.v[2] = 115;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[176] = f;
	f.v[0] = 96;  f.v[1] = 115;  f.v[2] = 114;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[177] = f;
	f.v[0] = 114;  f.v[1] = 113;  f.v[2] = 95;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[178] = f;
	f.v[0] = 96;  f.v[1] = 114;  f.v[2] = 95;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[179] = f;
	f.v[0] = 98;  f.v[1] = 99;  f.v[2] = 117;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[180] = f;
	f.v[0] = 98;  f.v[1] = 117;  f.v[2] = 116;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[181] = f;
	f.v[0] = 116;  f.v[1] = 115;  f.v[2] = 97;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[182] = f;
	f.v[0] = 98;  f.v[1] = 116;  f.v[2] = 97;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[183] = f;
	f.v[0] = 100;  f.v[1] = 101;  f.v[2] = 119;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[184] = f;
	f.v[0] = 100;  f.v[1] = 119;  f.v[2] = 118;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[185] = f;
	f.v[0] = 118;  f.v[1] = 117;  f.v[2] = 99;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[186] = f;
	f.v[0] = 100;  f.v[1] = 118;  f.v[2] = 99;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[187] = f;
	f.v[0] = 102;  f.v[1] = 103;  f.v[2] = 121;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[188] = f;
	f.v[0] = 102;  f.v[1] = 121;  f.v[2] = 120;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[189] = f;
	f.v[0] = 120;  f.v[1] = 119;  f.v[2] = 101;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[190] = f;
	f.v[0] = 102;  f.v[1] = 120;  f.v[2] = 101;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[191] = f;
	f.v[0] = 104;  f.v[1] = 105;  f.v[2] = 123;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[192] = f;
	f.v[0] = 104;  f.v[1] = 123;  f.v[2] = 122;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[193] = f;
	f.v[0] = 122;  f.v[1] = 121;  f.v[2] = 103;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[194] = f;
	f.v[0] = 104;  f.v[1] = 122;  f.v[2] = 103;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[195] = f;
	f.v[0] = 106;  f.v[1] = 107;  f.v[2] = 125;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[196] = f;
	f.v[0] = 106;  f.v[1] = 125;  f.v[2] = 124;  	f.smGroup = 8;  f.flags = 131074; mesh.faces[197] = f;
	f.v[0] = 124;  f.v[1] = 123;  f.v[2] = 105;  	f.smGroup = 8;  f.flags = 131075; mesh.faces[198] = f;
	f.v[0] = 106;  f.v[1] = 124;  f.v[2] = 105;  	f.smGroup = 8;  f.flags = 131076; mesh.faces[199] = f;
	f.v[0] = 110;  f.v[1] = 111;  f.v[2] = 129;  	f.smGroup = 0;  f.flags = 3; mesh.faces[200] = f;
	f.v[0] = 110;  f.v[1] = 129;  f.v[2] = 128;  	f.smGroup = 0;  f.flags = 2; mesh.faces[201] = f;
	f.v[0] = 128;  f.v[1] = 127;  f.v[2] = 109;  	f.smGroup = 0;  f.flags = 3; mesh.faces[202] = f;
	f.v[0] = 110;  f.v[1] = 128;  f.v[2] = 109;  	f.smGroup = 0;  f.flags = 4; mesh.faces[203] = f;
	f.v[0] = 112;  f.v[1] = 113;  f.v[2] = 131;  	f.smGroup = 0;  f.flags = 3; mesh.faces[204] = f;
	f.v[0] = 112;  f.v[1] = 131;  f.v[2] = 130;  	f.smGroup = 0;  f.flags = 2; mesh.faces[205] = f;
	f.v[0] = 130;  f.v[1] = 129;  f.v[2] = 111;  	f.smGroup = 0;  f.flags = 3; mesh.faces[206] = f;
	f.v[0] = 112;  f.v[1] = 130;  f.v[2] = 111;  	f.smGroup = 0;  f.flags = 4; mesh.faces[207] = f;
	f.v[0] = 114;  f.v[1] = 115;  f.v[2] = 133;  	f.smGroup = 0;  f.flags = 3; mesh.faces[208] = f;
	f.v[0] = 114;  f.v[1] = 133;  f.v[2] = 132;  	f.smGroup = 0;  f.flags = 2; mesh.faces[209] = f;
	f.v[0] = 132;  f.v[1] = 131;  f.v[2] = 113;  	f.smGroup = 0;  f.flags = 3; mesh.faces[210] = f;
	f.v[0] = 114;  f.v[1] = 132;  f.v[2] = 113;  	f.smGroup = 0;  f.flags = 4; mesh.faces[211] = f;
	f.v[0] = 116;  f.v[1] = 117;  f.v[2] = 135;  	f.smGroup = 0;  f.flags = 3; mesh.faces[212] = f;
	f.v[0] = 116;  f.v[1] = 135;  f.v[2] = 134;  	f.smGroup = 0;  f.flags = 2; mesh.faces[213] = f;
	f.v[0] = 134;  f.v[1] = 133;  f.v[2] = 115;  	f.smGroup = 0;  f.flags = 3; mesh.faces[214] = f;
	f.v[0] = 116;  f.v[1] = 134;  f.v[2] = 115;  	f.smGroup = 0;  f.flags = 4; mesh.faces[215] = f;
	f.v[0] = 118;  f.v[1] = 119;  f.v[2] = 137;  	f.smGroup = 0;  f.flags = 3; mesh.faces[216] = f;
	f.v[0] = 118;  f.v[1] = 137;  f.v[2] = 136;  	f.smGroup = 0;  f.flags = 2; mesh.faces[217] = f;
	f.v[0] = 136;  f.v[1] = 135;  f.v[2] = 117;  	f.smGroup = 0;  f.flags = 3; mesh.faces[218] = f;
	f.v[0] = 118;  f.v[1] = 136;  f.v[2] = 117;  	f.smGroup = 0;  f.flags = 4; mesh.faces[219] = f;
	f.v[0] = 120;  f.v[1] = 121;  f.v[2] = 139;  	f.smGroup = 0;  f.flags = 3; mesh.faces[220] = f;
	f.v[0] = 120;  f.v[1] = 139;  f.v[2] = 138;  	f.smGroup = 0;  f.flags = 2; mesh.faces[221] = f;
	f.v[0] = 138;  f.v[1] = 137;  f.v[2] = 119;  	f.smGroup = 0;  f.flags = 3; mesh.faces[222] = f;
	f.v[0] = 120;  f.v[1] = 138;  f.v[2] = 119;  	f.smGroup = 0;  f.flags = 4; mesh.faces[223] = f;
	f.v[0] = 122;  f.v[1] = 123;  f.v[2] = 141;  	f.smGroup = 0;  f.flags = 3; mesh.faces[224] = f;
	f.v[0] = 122;  f.v[1] = 141;  f.v[2] = 140;  	f.smGroup = 0;  f.flags = 2; mesh.faces[225] = f;
	f.v[0] = 140;  f.v[1] = 139;  f.v[2] = 121;  	f.smGroup = 0;  f.flags = 3; mesh.faces[226] = f;
	f.v[0] = 122;  f.v[1] = 140;  f.v[2] = 121;  	f.smGroup = 0;  f.flags = 4; mesh.faces[227] = f;
	f.v[0] = 124;  f.v[1] = 125;  f.v[2] = 143;  	f.smGroup = 0;  f.flags = 3; mesh.faces[228] = f;
	f.v[0] = 124;  f.v[1] = 143;  f.v[2] = 142;  	f.smGroup = 0;  f.flags = 2; mesh.faces[229] = f;
	f.v[0] = 142;  f.v[1] = 141;  f.v[2] = 123;  	f.smGroup = 0;  f.flags = 3; mesh.faces[230] = f;
	f.v[0] = 124;  f.v[1] = 142;  f.v[2] = 123;  	f.smGroup = 0;  f.flags = 4; mesh.faces[231] = f;
	f.v[0] = 108;  f.v[1] = 109;  f.v[2] = 127;  	f.smGroup = 0;  f.flags = 3; mesh.faces[232] = f;
	f.v[0] = 108;  f.v[1] = 127;  f.v[2] = 126;  	f.smGroup = 0;  f.flags = 2; mesh.faces[233] = f;
	f.v[0] = 126;  f.v[1] = 143;  f.v[2] = 125;  	f.smGroup = 0;  f.flags = 3; mesh.faces[234] = f;
	f.v[0] = 108;  f.v[1] = 126;  f.v[2] = 125;  	f.smGroup = 0;  f.flags = 4; mesh.faces[235] = f;
	f.v[0] = 128;  f.v[1] = 129;  f.v[2] = 147;  	f.smGroup = 0;  f.flags = 3; mesh.faces[236] = f;
	f.v[0] = 128;  f.v[1] = 147;  f.v[2] = 146;  	f.smGroup = 0;  f.flags = 2; mesh.faces[237] = f;
	f.v[0] = 146;  f.v[1] = 145;  f.v[2] = 127;  	f.smGroup = 0;  f.flags = 3; mesh.faces[238] = f;
	f.v[0] = 128;  f.v[1] = 146;  f.v[2] = 127;  	f.smGroup = 0;  f.flags = 4; mesh.faces[239] = f;
	f.v[0] = 130;  f.v[1] = 131;  f.v[2] = 149;  	f.smGroup = 0;  f.flags = 3; mesh.faces[240] = f;
	f.v[0] = 130;  f.v[1] = 149;  f.v[2] = 148;  	f.smGroup = 0;  f.flags = 2; mesh.faces[241] = f;
	f.v[0] = 148;  f.v[1] = 147;  f.v[2] = 129;  	f.smGroup = 0;  f.flags = 3; mesh.faces[242] = f;
	f.v[0] = 130;  f.v[1] = 148;  f.v[2] = 129;  	f.smGroup = 0;  f.flags = 4; mesh.faces[243] = f;
	f.v[0] = 132;  f.v[1] = 133;  f.v[2] = 151;  	f.smGroup = 0;  f.flags = 3; mesh.faces[244] = f;
	f.v[0] = 132;  f.v[1] = 151;  f.v[2] = 150;  	f.smGroup = 0;  f.flags = 2; mesh.faces[245] = f;
	f.v[0] = 150;  f.v[1] = 149;  f.v[2] = 131;  	f.smGroup = 0;  f.flags = 3; mesh.faces[246] = f;
	f.v[0] = 132;  f.v[1] = 150;  f.v[2] = 131;  	f.smGroup = 0;  f.flags = 4; mesh.faces[247] = f;
	f.v[0] = 134;  f.v[1] = 135;  f.v[2] = 153;  	f.smGroup = 0;  f.flags = 3; mesh.faces[248] = f;
	f.v[0] = 134;  f.v[1] = 153;  f.v[2] = 152;  	f.smGroup = 0;  f.flags = 2; mesh.faces[249] = f;
	f.v[0] = 152;  f.v[1] = 151;  f.v[2] = 133;  	f.smGroup = 0;  f.flags = 3; mesh.faces[250] = f;
	f.v[0] = 134;  f.v[1] = 152;  f.v[2] = 133;  	f.smGroup = 0;  f.flags = 4; mesh.faces[251] = f;
	f.v[0] = 136;  f.v[1] = 137;  f.v[2] = 155;  	f.smGroup = 0;  f.flags = 3; mesh.faces[252] = f;
	f.v[0] = 136;  f.v[1] = 155;  f.v[2] = 154;  	f.smGroup = 0;  f.flags = 2; mesh.faces[253] = f;
	f.v[0] = 154;  f.v[1] = 153;  f.v[2] = 135;  	f.smGroup = 0;  f.flags = 3; mesh.faces[254] = f;
	f.v[0] = 136;  f.v[1] = 154;  f.v[2] = 135;  	f.smGroup = 0;  f.flags = 4; mesh.faces[255] = f;
	f.v[0] = 138;  f.v[1] = 139;  f.v[2] = 157;  	f.smGroup = 0;  f.flags = 3; mesh.faces[256] = f;
	f.v[0] = 138;  f.v[1] = 157;  f.v[2] = 156;  	f.smGroup = 0;  f.flags = 2; mesh.faces[257] = f;
	f.v[0] = 156;  f.v[1] = 155;  f.v[2] = 137;  	f.smGroup = 0;  f.flags = 3; mesh.faces[258] = f;
	f.v[0] = 138;  f.v[1] = 156;  f.v[2] = 137;  	f.smGroup = 0;  f.flags = 4; mesh.faces[259] = f;
	f.v[0] = 140;  f.v[1] = 141;  f.v[2] = 159;  	f.smGroup = 0;  f.flags = 3; mesh.faces[260] = f;
	f.v[0] = 140;  f.v[1] = 159;  f.v[2] = 158;  	f.smGroup = 0;  f.flags = 2; mesh.faces[261] = f;
	f.v[0] = 158;  f.v[1] = 157;  f.v[2] = 139;  	f.smGroup = 0;  f.flags = 3; mesh.faces[262] = f;
	f.v[0] = 140;  f.v[1] = 158;  f.v[2] = 139;  	f.smGroup = 0;  f.flags = 4; mesh.faces[263] = f;
	f.v[0] = 142;  f.v[1] = 143;  f.v[2] = 161;  	f.smGroup = 0;  f.flags = 3; mesh.faces[264] = f;
	f.v[0] = 142;  f.v[1] = 161;  f.v[2] = 160;  	f.smGroup = 0;  f.flags = 2; mesh.faces[265] = f;
	f.v[0] = 160;  f.v[1] = 159;  f.v[2] = 141;  	f.smGroup = 0;  f.flags = 3; mesh.faces[266] = f;
	f.v[0] = 142;  f.v[1] = 160;  f.v[2] = 141;  	f.smGroup = 0;  f.flags = 4; mesh.faces[267] = f;
	f.v[0] = 126;  f.v[1] = 127;  f.v[2] = 145;  	f.smGroup = 0;  f.flags = 3; mesh.faces[268] = f;
	f.v[0] = 126;  f.v[1] = 145;  f.v[2] = 144;  	f.smGroup = 0;  f.flags = 2; mesh.faces[269] = f;
	f.v[0] = 144;  f.v[1] = 161;  f.v[2] = 143;  	f.smGroup = 0;  f.flags = 3; mesh.faces[270] = f;
	f.v[0] = 126;  f.v[1] = 144;  f.v[2] = 143;  	f.smGroup = 0;  f.flags = 4; mesh.faces[271] = f;
	f.v[0] = 165;  f.v[1] = 163;  f.v[2] = 162;  	f.smGroup = 4;  f.flags = 65539; mesh.faces[272] = f;
	f.v[0] = 162;  f.v[1] = 164;  f.v[2] = 165;  	f.smGroup = 4;  f.flags = 65539; mesh.faces[273] = f;
	f.v[0] = 169;  f.v[1] = 168;  f.v[2] = 166;  	f.smGroup = 2;  f.flags = 3; mesh.faces[274] = f;
	f.v[0] = 166;  f.v[1] = 167;  f.v[2] = 169;  	f.smGroup = 2;  f.flags = 3; mesh.faces[275] = f;
	f.v[0] = 167;  f.v[1] = 166;  f.v[2] = 162;  	f.smGroup = 8;  f.flags = 262147; mesh.faces[276] = f;
	f.v[0] = 162;  f.v[1] = 163;  f.v[2] = 167;  	f.smGroup = 8;  f.flags = 262147; mesh.faces[277] = f;
	f.v[0] = 169;  f.v[1] = 167;  f.v[2] = 163;  	f.smGroup = 16;  f.flags = 196611; mesh.faces[278] = f;
	f.v[0] = 163;  f.v[1] = 165;  f.v[2] = 169;  	f.smGroup = 16;  f.flags = 196611; mesh.faces[279] = f;
	f.v[0] = 168;  f.v[1] = 169;  f.v[2] = 165;  	f.smGroup = 32;  f.flags = 327683; mesh.faces[280] = f;
	f.v[0] = 165;  f.v[1] = 164;  f.v[2] = 168;  	f.smGroup = 32;  f.flags = 327683; mesh.faces[281] = f;
	f.v[0] = 166;  f.v[1] = 168;  f.v[2] = 164;  	f.smGroup = 64;  f.flags = 131075; mesh.faces[282] = f;
	f.v[0] = 164;  f.v[1] = 162;  f.v[2] = 166;  	f.smGroup = 64;  f.flags = 131075; mesh.faces[283] = f;

	mesh.buildNormals();
	mesh.EnableEdgeList(1);
	meshBuilt = 1;
#else // USE_VIZR3_CAMERA_MESH
	int nverts = 16;
	int nfaces = 24;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	float len = (float)5.0;
	float w = (float)8.0;
	float d = w*(float).8;
	float e = d*(float).5;
	float f = d*(float).8;
	float l = w*(float).8;

	mesh.setVert(0, Point3( -d, -d, -len));
	mesh.setVert(1, Point3(  d, -d, -len));
	mesh.setVert(2, Point3( -d,  d, -len));
	mesh.setVert(3, Point3(  d,  d, -len));
	mesh.setVert(4, Point3( -d, -d,  len));
	mesh.setVert(5, Point3(  d, -d,  len));
	mesh.setVert(6, Point3( -d,  d,  len));
	mesh.setVert(7, Point3(  d,  d,  len));
	MakeQuad(&(mesh.faces[ 0]), 0,2,3,1,  1);
	MakeQuad(&(mesh.faces[ 2]), 2,0,4,6,  2);
	MakeQuad(&(mesh.faces[ 4]), 3,2,6,7,  4);
	MakeQuad(&(mesh.faces[ 6]), 1,3,7,5,  8);
	MakeQuad(&(mesh.faces[ 8]), 0,1,5,4, 16);
	MakeQuad(&(mesh.faces[10]), 4,5,7,6, 32);
	
	mesh.setVert(8+0, Point3( -e, -e, len));
	mesh.setVert(8+1, Point3(  e, -e, len));
	mesh.setVert(8+2, Point3( -e,  e, len));
	mesh.setVert(8+3, Point3(  e,  e, len));
	mesh.setVert(8+4, Point3( -f, -f, len+l));
	mesh.setVert(8+5, Point3(  f, -f, len+l));
	mesh.setVert(8+6, Point3( -f,  f, len+l));
	mesh.setVert(8+7, Point3(  f,  f, len+l));

	Face* fbase = &mesh.faces[12];
	MakeQuad(&fbase[0],0,2,3,1,   1, 8);
	MakeQuad(&fbase[2], 2,0,4,6,  2, 8);
	MakeQuad(&fbase[4], 3,2,6,7,  4, 8);
	MakeQuad(&fbase[6], 1,3,7,5,  8, 8);
	MakeQuad(&fbase[8], 0,1,5,4, 16, 8);
	MakeQuad(&fbase[10],4,5,7,6, 32, 8);

	// whoops- rotate 180 about x to get it facing the right way
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.RotateX(DegToRad(180.0));
	for (int i=0; i<nverts; i++)
		mesh.getVert(i) = mat*mesh.getVert(i);
	mesh.buildNormals();
	mesh.EnableEdgeList(1);
	meshBuilt = 1;
#endif
	}

void SimpleCamera::UpdateKeyBrackets(TimeValue t) {
	fovSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_FOV,t));
	lensSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_FOV,t));
	hitherSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_HITHER,t));
	yonSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_YON,t));
#if !defined (WEBVERSION) && !defined(NO_CAMERA_ENVRANGE)
	envNearSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_NRANGE,t));
	envFarSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_FRANGE,t));
#endif

// nac - begin - 12.08.00
//	if ( fStopSpin != NULL )
		// Could be NULL if maxtrans not installed
//		fStopSpin->SetKeyBrackets(depthOfFieldPB->KeyFrameAtTime(PB_DOF_FSTOP,t));
// nac - end
	}

void SimpleCamera::UpdateUI(TimeValue t)
{
	if ( hSimpleCamParams && !waitPostLoad && GetWindowLongPtr(hSimpleCamParams,GWLP_USERDATA)==(LONG_PTR)this 
	&& pblock )
	{
		fovSpin->SetValue( RadToDeg(WFOVtoCurFOV(GetFOV(t))), FALSE );
		lensSpin->SetValue(FOVtoMM(GetFOV(t)), FALSE);
		hitherSpin->SetValue( GetClipDist(t, CAM_HITHER_CLIP), FALSE );
		yonSpin->SetValue( GetClipDist(t, CAM_YON_CLIP), FALSE );
#if !defined (WEBVERSION) && !defined(NO_CAMERA_ENVRANGE)
		envNearSpin->SetValue( GetEnvRange(t, ENV_NEAR_RANGE), FALSE );
		envFarSpin->SetValue( GetEnvRange(t, ENV_FAR_RANGE), FALSE );
#endif
// nac - begin - 12.08.00
//		if ( fStopSpin != NULL )
			// Could be NULL if maxtrans not installed
//			fStopSpin->SetValue( GetDOFFStop ( t ), FALSE );
// nac - end		
		UpdateKeyBrackets(t);

#ifndef NO_CAMERA_MULTIPASS
		CheckDlgButton(hSimpleCamParams, IDC_ENABLE_MP_EFFECT, GetMultiPassEffectEnabled(t) );
		CheckDlgButton(hSimpleCamParams, IDC_MP_EFFECT_REFFECT_PER_PASS, GetMPEffect_REffectPerPass() );
#endif // NO_CAMERA_MULTIPASS

		// DS 8/15/00 
//		if(!hasTarget)
			tdistSpin->SetValue( GetTDist(t), FALSE );

		// DS 8/28/00
		SendMessage( GetDlgItem(hSimpleCamParams, IDC_CAM_TYPE), CB_SETCURSEL, hasTarget, (LPARAM)0 );
	}
}

#define CAMERA_VERSION 4	// current version // mjm - 07.17.00
#define DOF_VERSION 2		// current version // nac - 12.08.00

#define CAMERA_PBLOCK_COUNT	8

static ParamBlockDescID descV0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// FOV
	{ TYPE_FLOAT, NULL, TRUE, 2 } };	// TDIST

static ParamBlockDescID descV1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// FOV
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// TDIST
	{ TYPE_FLOAT, NULL, TRUE, 3 },		// HITHER
	{ TYPE_FLOAT, NULL, TRUE, 4 } };	// YON

static ParamBlockDescID descV2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// FOV
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// TDIST
	{ TYPE_FLOAT, NULL, TRUE, 3 },		// HITHER
	{ TYPE_FLOAT, NULL, TRUE, 4 },		// YON
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// NEAR ENV RANGE
	{ TYPE_FLOAT, NULL, TRUE, 6 } };	// FAR ENV RANGE

// mjm - begin - 07.17.00
static ParamBlockDescID descV3[] = {
	{ TYPE_FLOAT, NULL, TRUE,  1 },		// FOV
	{ TYPE_FLOAT, NULL, TRUE,  2 },		// TDIST
	{ TYPE_FLOAT, NULL, TRUE,  3 },		// HITHER
	{ TYPE_FLOAT, NULL, TRUE,  4 },		// YON
	{ TYPE_FLOAT, NULL, TRUE,  5 },		// NEAR ENV RANGE
	{ TYPE_FLOAT, NULL, TRUE,  6 },		// FAR ENV RANGE
	{ TYPE_BOOL,  NULL, FALSE, 7 } };	// MULTI PASS EFFECT - RENDER EFFECTS PER PASS

static ParamBlockDescID descV4[] = {
	{ TYPE_FLOAT, NULL, TRUE,  1 },		// FOV
	{ TYPE_FLOAT, NULL, TRUE,  2 },		// TDIST
	{ TYPE_FLOAT, NULL, TRUE,  3 },		// HITHER
	{ TYPE_FLOAT, NULL, TRUE,  4 },		// YON
	{ TYPE_FLOAT, NULL, TRUE,  5 },		// NEAR ENV RANGE
	{ TYPE_FLOAT, NULL, TRUE,  6 },		// FAR ENV RANGE
	{ TYPE_BOOL,  NULL, FALSE, 7 },		// MULTI PASS EFFECT ENABLE
	{ TYPE_BOOL,  NULL, FALSE, 8 } };	// MULTI PASS EFFECT - RENDER EFFECTS PER PASS

// mjm - end

static ParamBlockDescID dofV0[] = {
	{ TYPE_BOOL,  NULL, TRUE,  1 },		// ENABLE
	{ TYPE_FLOAT, NULL, TRUE, 2 } };	// FSTOP

// nac - begin - 12.08.00 - no changes, just obsolete all parameters
static ParamBlockDescID dofV1[] = {
	{ TYPE_BOOL,  NULL, TRUE,  1 },		// ENABLE
	{ TYPE_FLOAT, NULL, TRUE, 2 } };	// FSTOP
// nac - end

// mjm - begin - 07.17.00
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
SimpleCamera::SimpleCamera(int look) : targDist(0.0f), mpIMultiPassCameraEffect(NULL)
#else
SimpleCamera::SimpleCamera(int look) : targDist(0.0f)
#endif // NO_CAMERA_MULTIPASS
{
	hasTarget = look;

	MakeRefByID( FOREVER, 0, CreateParameterBlock( descV4, CAMERA_PBLOCK_COUNT, CAMERA_VERSION ) );

// mjm - end
// nac - begin - 12.08.00
//	MakeRefByID( FOREVER, DOF_REF, CreateParameterBlock( dofV0, 2, DOF_VERSION ) );
	depthOfFieldPB = NULL;
// nac - end
	SetFOV( TimeValue(0), dlgFOV );
	SetTDist( TimeValue(0), dlgTDist );
	SetClipDist( TimeValue(0), CAM_HITHER_CLIP, dlgHither );
	SetClipDist( TimeValue(0), CAM_YON_CLIP, dlgYon );
	SetEnvRange( TimeValue(0), ENV_NEAR_RANGE, dlgNearRange );
	SetEnvRange( TimeValue(0), ENV_FAR_RANGE, dlgFarRange );
// nac - begin - 12.08.00
//	SetDOFEnable ( TimeValue(0), dlgDOFEnable );
//	SetDOFFStop ( TimeValue(0), dlgDOFFStop );
// nac - end

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
	ReplaceReference( MP_EFFECT_REF, CreateDefaultMultiPassEffect(this) );

	SetMultiPassEffectEnabled(TimeValue(0), dlgMultiPassEffectEnable);
	SetMPEffect_REffectPerPass(dlgMPEffect_REffectPerPass);
// mjm - end
#endif // NO_CAMERA_MULTIPASS

	fovType = FOV_W;

	enable = 0;
	coneState = dlgShowCone;
	rangeDisplay = dlgRangeDisplay;
	horzLineState = dlgShowHorzLine;
	manualClip = dlgClip;
	isOrtho = dlgIsOrtho;

	BuildMesh();
}

BOOL SimpleCamera::IsCompatibleRenderer()
{
// nac - begin - 12.08.00
//	Class_ID rclass = Class_ID( 0x58f67d6c, 0x4fcf3bc3 ); // class id for mental ray translator
//	ClassDesc *cd = GetCOREInterface()->GetDllDir().ClassDir().FindClass(RENDERER_CLASS_ID, rclass );
//
//	if ( cd != NULL )
//		return TRUE;
// nac - end
	return FALSE;
}

BOOL SimpleCamera::SetFOVControl(Control *c)
	{
	pblock->SetController(PB_FOV,c);
	return TRUE;
	}
Control * SimpleCamera::GetFOVControl() {
	return 	pblock->GetController(PB_FOV);
	}

void SimpleCamera::SetConeState(int s) {
	coneState = s;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void SimpleCamera::SetHorzLineState(int s) {
	horzLineState = s;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}



//--------------------------------------------

static INode* FindNodeRef(ReferenceTarget *rt);

static INode* GetNodeRef(ReferenceMaker *rm) {
	if (rm->SuperClassID()==BASENODE_CLASS_ID) return (INode *)rm;
	else return rm->IsRefTarget()?FindNodeRef((ReferenceTarget *)rm):NULL;
	}

static INode* FindNodeRef(ReferenceTarget *rt) {
	DependentIterator di(rt);
	ReferenceMaker *rm;
	INode *nd = NULL;
	while (rm=di.Next()) {	
		nd = GetNodeRef(rm);
		if (nd) return nd;
		}
	return NULL;
	}

/*
static INode* FindSelNodeRef(ReferenceTarget *rt) {
	DependentIterator di(rt);
	ReferenceMaker *rm;
	INode *nd = NULL;
	while (rm=di.Next()) {	
		if (rm->SuperClassID()==BASENODE_CLASS_ID) {
			nd = (INode *)rm;
			if(nd->Selected()) return nd;
			}
		else {
			if (rm->IsRefTarget()) {
				nd = FindSelNodeRef((ReferenceTarget *)rm);
				if (nd)
					return nd;
				}
			}
		}
	return NULL;
	}
*/

//----------------------------------------------------------------

class SetCamTypeRest: public RestoreObj {
	public:
		SimpleCamera *theCam;
		int oldHasTarget;
		SetCamTypeRest(SimpleCamera *lt, int newt) {
			theCam = lt;
			oldHasTarget = lt->hasTarget;
			}
		~SetCamTypeRest() { }
		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		virtual TSTR Description() { return TSTR(_T("Set Camera Type")); }
	};


void SetCamTypeRest::Restore(int isUndo) {
	theCam->hasTarget = oldHasTarget;
	theCam->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	theCam->NotifyDependents(FOREVER, PART_OBJ, REFMSG_NUM_SUBOBJECTTYPES_CHANGED); // to redraw modifier stack
	}

void SetCamTypeRest::Redo() {
	theCam->hasTarget = !oldHasTarget;
	theCam->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	theCam->NotifyDependents(FOREVER, PART_OBJ, REFMSG_NUM_SUBOBJECTTYPES_CHANGED); // to redraw modifier stack
	}



class ParamsRest: public RestoreObj {
	public:
		SimpleCamera *theCam;
		BOOL showit;
		ParamsRest(SimpleCamera *c, BOOL show) {
			theCam = c;
			showit = show;
			}
		void ParamsRest::Restore(int isUndo) {
			theCam->NotifyDependents(FOREVER, PART_OBJ, showit?REFMSG_END_MODIFY_PARAMS:REFMSG_BEGIN_MODIFY_PARAMS);
			}
		void ParamsRest::Redo() {
			theCam->NotifyDependents(FOREVER, PART_OBJ, showit?REFMSG_BEGIN_MODIFY_PARAMS:REFMSG_END_MODIFY_PARAMS);
			}
		int Size() { return 1; }
		virtual TSTR Description() { return TSTR(_T("Camera Params")); }
	};


/*----------------------------------------------------------------*/

void SimpleCamera::SetType(int tp) {     
	if (hasTarget == tp) 
		return;

	Interface *iface = GetCOREInterface();
	TimeValue t = iface->GetTime();
	INode *nd = FindNodeRef(this);
	if (nd==NULL) 
		return;

	BOOL paramsShowing = FALSE;
	if (hSimpleCamParams && (currentEditCam == this)) { // LAM - 8/13/02 - defect 511609
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_END_MODIFY_PARAMS);
		paramsShowing = TRUE;
		if (theHold.Holding()) 
			theHold.Put(new ParamsRest(this,0));
		}

	if (theHold.Holding())
		theHold.Put(new SetCamTypeRest(this,tp));

	int oldtype = hasTarget;
	Interval v;
	float tdist = GetTDist(t,v);
	hasTarget = tp;

	if(oldtype==TARGETED_CAMERA) {
		tdist = targDist;
		// get rid of target, assign a PRS controller for all instances
		DependentIterator di(this);
		ReferenceMaker *rm;
		// iterate through the instances
		while (rm=di.Next()) {
			nd = GetNodeRef(rm);
			if (nd) {
				INode* tn = nd->GetTarget(); 
				Matrix3 tm = nd->GetNodeTM(0);
				if (tn) iface->DeleteNode(tn);  // JBW, make it safe if no target
				Control *tmc = NewDefaultMatrix3Controller();
				tmc->Copy(nd->GetTMController()); // doesn't copy rotation, only scale and position.
				nd->SetTMController(tmc);
				nd->SetNodeTM(0,tm);		// preserve rotation if not animated at least
				SetTDist(0,tdist);	 //?? which one should this be for
				}
			}

		}
	else  {
		DependentIterator di(this);
		ReferenceMaker *rm;
		// iterate through the instances
		while (rm=di.Next()) {	
			nd = GetNodeRef(rm);
			if (nd) {
				// create a target, assign lookat controller
				Matrix3 tm = nd->GetNodeTM(t);
				Matrix3 targtm = tm;
				targtm.PreTranslate(Point3(0.0f,0.0f,-tdist));
				Object *targObject = new TargetObject;
				INode *targNode = iface->CreateObjectNode( targObject);
				TSTR targName;
				targName = nd->GetName();
				targName += GetString(IDS_DB_DOT_TARGET);
				targNode->SetName(targName);
				targNode->SetNodeTM(0,targtm);
				Control *laControl= CreateLookatControl();
				laControl->SetTarget(targNode);
				laControl->Copy(nd->GetTMController());
				nd->SetTMController(laControl);
				targNode->SetIsTarget(1);   
				}
			}
		}

	if (paramsShowing) {
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_BEGIN_MODIFY_PARAMS);
		if (theHold.Holding()) 
			theHold.Put(new ParamsRest(this,1));
		}

	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_NUM_SUBOBJECTTYPES_CHANGED); // to redraw modifier stack
	iface->RedrawViews(iface->GetTime());
	}


//---------------------------------------
void SimpleCamera::SetFOV(TimeValue t, float f) {
	pblock->SetValue( PB_FOV, t, f );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float SimpleCamera::GetFOV(TimeValue t,Interval& valid) {	
	float f;
	pblock->GetValue( PB_FOV, t, f, valid );
	if ( f < float(0) ) f = float(0);
	return f;
	}

void SimpleCamera::SetTDist(TimeValue t, float f) {
// DS 8/15/00  begin
	if (hasTarget) {
		INode *nd = FindNodeRef(this);
		if (nd==NULL) return;
		INode* tn = nd->GetTarget(); 
		if (tn==NULL) return;
		Point3 ptarg;
		GetTargetPoint(t, nd, ptarg);
		Matrix3 tm = nd->GetObjectTM(t);
		float dtarg = Length(tm.GetTrans()-ptarg)/Length(tm.GetRow(2));

		Point3 delta(0,0,0);
		delta.z = dtarg-f;
		Matrix3 tmAxis = nd->GetNodeTM(t);
		tn->Move(t, tmAxis, delta);
		}
	else 
// DS 8/15/00  end
		{
		pblock->SetValue( PB_TDIST, t, f );
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		}
	}

float SimpleCamera::GetTDist(TimeValue t,Interval& valid) {	
	float f;
	if (hasTarget) {
		return targDist;
		}
	else {
		pblock->GetValue( PB_TDIST, t, f, valid );
		if ( f < MIN_TDIST ) f = MIN_TDIST;
		return f;
		}
	}

void SimpleCamera::SetManualClip(int onOff)
{
	manualClip = onOff;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float SimpleCamera::GetClipDist(TimeValue t, int which, Interval &valid)
{
	float f;
	pblock->GetValue( PB_HITHER+which-1, t, f, valid );
	if ( f < MIN_CLIP ) f = MIN_CLIP;
	if ( f > MAX_CLIP ) f = MAX_CLIP;
	return f;
}

void SimpleCamera::SetClipDist(TimeValue t, int which, float f)
{
	pblock->SetValue( PB_HITHER+which-1, t, f );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}


void SimpleCamera::SetEnvRange(TimeValue t, int which, float f)
{
	pblock->SetValue( PB_NRANGE+which, t, f );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float SimpleCamera::GetEnvRange(TimeValue t, int which, Interval &valid)
{
	float f;
	pblock->GetValue( PB_NRANGE+which, t, f, valid );
	return f;
}

void SimpleCamera::SetEnvDisplay(BOOL b, int notify) {
	rangeDisplay = b;
	if(notify)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void SimpleCamera::SetDOFEnable (TimeValue t, BOOL onOff) {
// nac - begin - 12.08.00
//	depthOfFieldPB->SetValue( PB_DOF_ENABLE, t, onOff );
//	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
// nac - end
	}

BOOL SimpleCamera::GetDOFEnable(TimeValue t,Interval& valid) {	
// nac - begin - 12.08.00
//	BOOL f;
//	depthOfFieldPB->GetValue( PB_DOF_ENABLE, t, f, valid );
//	return f;
	return FALSE;
// nac - end
	}

void SimpleCamera::SetDOFFStop (TimeValue t, float f) {
// nac - begin - 12.08.00
//	depthOfFieldPB->SetValue( PB_DOF_FSTOP, t, f );
//	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
// nac - end
	}

float SimpleCamera::GetDOFFStop(TimeValue t,Interval& valid) {	
// nac - begin - 12.08.00
//	float f;
//	depthOfFieldPB->GetValue( PB_DOF_FSTOP, t, f, valid );
//	return f;
	return 2.0f;
// nac - end
	}

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
void SimpleCamera::SetMultiPassEffectEnabled(TimeValue t, BOOL enabled)
{
	pblock->SetValue(PB_MP_EFFECT_ENABLE, t, enabled);
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	if (hSimpleCamParams)
		EnableWindow( GetDlgItem(hSimpleCamParams, IDC_PREVIEW_MP_EFFECT), enabled );
}

BOOL SimpleCamera::GetMultiPassEffectEnabled(TimeValue t, Interval& valid)
{
	BOOL enabled;
	pblock->GetValue(PB_MP_EFFECT_ENABLE, t, enabled, valid);
	return enabled;
}

void SimpleCamera::SetMPEffect_REffectPerPass(BOOL enabled)
{
	pblock->SetValue(PB_MP_EFF_REND_EFF_PER_PASS, 0, enabled);
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

BOOL SimpleCamera::GetMPEffect_REffectPerPass()
{
	BOOL enabled;
	pblock->GetValue(PB_MP_EFF_REND_EFF_PER_PASS, 0, enabled, FOREVER);
	return enabled;
}
#endif // NO_CAMERA_MULTIPASS




#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01

class MultiPassRestore: public RestoreObj {
	public:
		SimpleCamera *theCam;
		SingleRefMaker oldmp;
		SingleRefMaker newmp;
		MultiPassRestore(SimpleCamera *sc, IMultiPassCameraEffect *pOldEffect, IMultiPassCameraEffect *pNewEffect) {
			theCam = sc;
			oldmp.SetRef(pOldEffect);
			newmp.SetRef(pNewEffect);
			};
		void Restore(int isUndo) {
			theCam->SetIMultiPassCameraEffect(reinterpret_cast<IMultiPassCameraEffect *>(oldmp.GetRef()));
			}
		void Redo() {
			theCam->SetIMultiPassCameraEffect(reinterpret_cast<IMultiPassCameraEffect *>(newmp.GetRef()));
			}
		int Size() { return 1; }
		virtual TSTR Description() { return TSTR(_T("Multi Pass Effect Change")); }
	};



// DS: 11/14/00 Made this undoable to fix Defect 268181.
void SimpleCamera::SetIMultiPassCameraEffect(IMultiPassCameraEffect *pIMultiPassCameraEffect)
{

	IMultiPassCameraEffect *pCurCameraEffect = GetIMultiPassCameraEffect();
	if (pCurCameraEffect == pIMultiPassCameraEffect) return; // LAM - 8/12/03
	if (theHold.Holding()) 
		theHold.Put( new MultiPassRestore(this, pCurCameraEffect, pIMultiPassCameraEffect));
	theHold.Suspend();
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	if (iObjParams && pCurCameraEffect && (currentEditCam == this)) 
		pCurCameraEffect->EndEditParams(iObjParams, END_EDIT_REMOVEUI);
#endif
	ReplaceReference(MP_EFFECT_REF, pIMultiPassCameraEffect);
	if (iObjParams && pIMultiPassCameraEffect && (currentEditCam == this)) {
		pIMultiPassCameraEffect->BeginEditParams(iObjParams, inCreate ? BEGIN_EDIT_CREATE : 0, NULL);

		HWND hEffectList = GetDlgItem(hSimpleCamParams, IDC_MP_EFFECT);
		SendMessage(hEffectList, CB_RESETCONTENT, 0, (LPARAM)0);
		int numClasses = smCompatibleEffectList.Count();
		int selIndex = -1;
		for (int i=0; i<numClasses; i++)
		{
			int index = SendMessage( hEffectList, CB_ADDSTRING, 0, (LPARAM)smCompatibleEffectList[i]->CD()->ClassName() );
			if ( pIMultiPassCameraEffect && ( pIMultiPassCameraEffect->ClassID() == smCompatibleEffectList[i]->CD()->ClassID() ) )
			{
				selIndex = index;
			}
		}
		SendMessage(hEffectList, CB_SETCURSEL, selIndex, (LPARAM)0);
	}
	theHold.Resume();
}

IMultiPassCameraEffect *SimpleCamera::GetIMultiPassCameraEffect()
{
	return reinterpret_cast<IMultiPassCameraEffect *>( GetReference(MP_EFFECT_REF) );
}

// static methods
IMultiPassCameraEffect *SimpleCamera::CreateDefaultMultiPassEffect(CameraObject *pCameraObject)
{
	FindCompatibleMultiPassEffects(pCameraObject);

	IMultiPassCameraEffect *pIMultiPassCameraEffect = NULL;
	int numClasses = smCompatibleEffectList.Count();
	for (int i=0; i<numClasses; i++)
	{
		// MultiPassDOF camera effect is the default
		if ( smCompatibleEffectList[i]->CD()->ClassID() == Class_ID(0xd481815, 0x687d799c) )
		{
			pIMultiPassCameraEffect = reinterpret_cast<IMultiPassCameraEffect *>( smCompatibleEffectList[i]->CD()->Create(0) );
		}
	}
	return pIMultiPassCameraEffect;
}

void SimpleCamera::FindCompatibleMultiPassEffects(CameraObject *pCameraObject)
{
	smCompatibleEffectList.ZeroCount();
	SubClassList *subList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(MPASS_CAM_EFFECT_CLASS_ID);
	if (subList)
	{
		IMultiPassCameraEffect *pIMultiPassCameraEffect;
		int i = subList->GetFirst(ACC_PUBLIC);
		theHold.Suspend(); // LAM: added 11/12/00
		while (i >= 0)
		{			
			ClassEntry *c = &(*subList)[i];

			if ( pIMultiPassCameraEffect = reinterpret_cast<IMultiPassCameraEffect *>( c->CD()->Create(0) ) )
			{
				if ( pIMultiPassCameraEffect->IsCompatible(pCameraObject) )
				{
					smCompatibleEffectList.Append(1, &c);
				}
				pIMultiPassCameraEffect->DeleteThis();
			}

			i = subList->GetNext(ACC_PUBLIC);
		}
		theHold.Resume(); // LAM: added 11/12/00
	}
}
// mjm - end
#endif // NO_CAMERA_MULTIPASS

void SimpleCamera::SetOrtho(BOOL b) {
	isOrtho = b;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}


class SCamCreateCallBack: public CreateMouseCallBack {
	public:
	SimpleCamera *ob;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(SimpleCamera *obj) { ob = obj; }
	};

int SCamCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	if (ob)
		ob->enable = 1;
#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
#endif
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {

		// since we're now allowing the user to set the color of
		// the color wire-frames, we need to set the camera 
		// color to blue instead of the default random object color.
		if ( point == 0 )
		{
			ULONG handle;
			ob->NotifyDependents( FOREVER, (PartID)&handle, REFMSG_GET_NODE_HANDLE );
			INode * node;
			node = GetCOREInterface()->GetINodeByHandle( handle );
			if ( node ) 
			{
				Point3 color = GetUIColor( COLOR_CAMERA_OBJ );
				node->SetWireColor( RGB( color.x*255.0f, color.y*255.0f, color.z*255.0f ) );
			}
		}
		
		//mat[3] = vpt->GetPointOnCP(m);
#ifdef _3D_CREATE
		mat.SetTrans( vpt->SnapPoint(m,m,NULL,SNAP_IN_3D) );
#else
		mat.SetTrans( vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE) );
#endif
		if (point==1 && msg==MOUSE_POINT) 
			return 0;
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;

	return TRUE;
	}

static SCamCreateCallBack sCamCreateCB;

SimpleCamera::~SimpleCamera() {
	sCamCreateCB.ob = NULL;	
	DeleteAllRefsFromMe();
	pblock = NULL;
	}

CreateMouseCallBack* SimpleCamera::GetCreateMouseCallBack() {
	sCamCreateCB.SetObj(this);
	return(&sCamCreateCB);
	}

static void RemoveScaling(Matrix3 &tm) {
	AffineParts ap;
	decomp_affine(tm, &ap);
	tm.IdentityMatrix();
	tm.SetRotate(ap.q);
	tm.SetTrans(ap.t);
	}

void SimpleCamera::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm) {
	tm = inode->GetObjectTM(t);
	//tm.NoScale();
	RemoveScaling(tm);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	if (scaleFactor!=(float)1.0)
		tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
	}

void SimpleCamera::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
	{
	box = mesh.getBoundingBox(tm);
	}

void SimpleCamera::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ){
	Matrix3 m = inode->GetObjectTM(t);
	Point3 pt;
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/(float)360.0;
	box = mesh.getBoundingBox();
#if 0
	if (!hasTarget) {
		if (coneState) {
			Point3 q[4];
			GetConePoints(t,q,scaleFactor*FIXED_CONE_DIST);
			box.IncludePoints(q,4);
			}
		}
#endif
	box.Scale(scaleFactor);
	Point3 q[4];

	if (GetTargetPoint(t,inode,pt)){
		float d = Length(m.GetTrans()-pt)/Length(inode->GetObjectTM(t).GetRow(2));
		box += Point3(float(0),float(0),-d);
		if (coneState || (extDispFlags & EXT_DISP_ONLY_SELECTED)) {
			if(manualClip) {
				GetConePoints(t,q,GetClipDist(t,CAM_YON_CLIP));
				box.IncludePoints(q,4);
			}
			GetConePoints(t,q,d);
			box.IncludePoints(q,4);
			}
		}
#if 1
	else {
		if (coneState || (extDispFlags & EXT_DISP_ONLY_SELECTED)) {
			float d = GetTDist(t);
			box += Point3(float(0),float(0),-d);
			GetConePoints(t,q,d);
			box.IncludePoints(q,4);
			}
		}
#endif
	if( rangeDisplay) {
		Point3 q[4];
		float rad = max(GetEnvRange(t, ENV_NEAR_RANGE), GetEnvRange(t, ENV_FAR_RANGE));
		GetConePoints(t, q, rad);
		box.IncludePoints(q,4);
		}
	}

void SimpleCamera::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
	int i,nv;
	Matrix3 tm;
	float dtarg;
	Point3 pt;
	GetMat(t,inode,vpt,tm);
	nv = mesh.getNumVerts();
	box.Init();
	if(!(extDispFlags & EXT_DISP_ZOOM_EXT))
		for (i=0; i<nv; i++) 
			box += tm*mesh.getVert(i);
	else
		box += tm.GetTrans();

	tm = inode->GetObjectTM(t);
	if (hasTarget) {
		if (GetTargetPoint(t,inode,pt)) {
			dtarg = Length(tm.GetTrans()-pt)/Length(tm.GetRow(2));
			box += tm*Point3(float(0),float(0),-dtarg);
			}
		}
#if 0
	else dtarg = FIXED_CONE_DIST;
#else
	else dtarg = GetTDist(t);
#endif
	if (coneState || (extDispFlags & EXT_DISP_ONLY_SELECTED)) {
		Point3 q[4];
		if(manualClip) {
			GetConePoints(t,q,GetClipDist(t,CAM_YON_CLIP));
			box.IncludePoints(q,4,&tm);
			}
		GetConePoints(t,q,dtarg);
		box.IncludePoints(q,4,&tm);
		}
	if( rangeDisplay) {
		Point3 q[4];
		float rad = max(GetEnvRange(t, ENV_NEAR_RANGE), GetEnvRange(t, ENV_FAR_RANGE));
		GetConePoints(t, q,rad);
		box.IncludePoints(q,4,&tm);
		}
	}
 
void SimpleCamera::UpdateTargDistance(TimeValue t, INode* inode) {
	if (hasTarget /*&&hSimpleCamParams*/) {						// 5/29/01 2:31pm --MQM-- move this test case down, so we will still compute the target dist
		                                                        //   even for network rendering (when there is no hSimpleCamParams window
		Point3 pt,v[2];
		if (GetTargetPoint(t,inode,pt)){
			Matrix3 tm = inode->GetObjectTM(t);
			float den = Length(tm.GetRow(2));
			targDist = (den!=0)?Length(tm.GetTrans()-pt)/den : 0.0f;

		// DS 8/15/00   
//			TCHAR buf[40];
//			_stprintf(buf,_T("%0.3f"),targDist);
//			SetWindowText(GetDlgItem(hSimpleCamParams,IDC_TARG_DISTANCE),buf);

			if ( hSimpleCamParams && (currentEditCam == this))		// 5/29/01 2:31pm --MQM--, LAM - 8/13/02 - defect 511609
				tdistSpin->SetValue(GetTDist(t), FALSE);
			}
		}
	}

int SimpleCamera::DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) {
	Matrix3 tm = inode->GetObjectTM(t);
	gw->setTransform(tm);
	gw->clearHitCode();
	if (hasTarget) {
		Point3 pt,v[3];
		if (GetTargetPoint(t,inode,pt)){
			float den = Length(tm.GetRow(2));
			float dist = (den!=0)?Length(tm.GetTrans()-pt)/den : 0.0f;
			targDist = dist;
			if (hSimpleCamParams&&(currentEditCam==this)) {
				// DS 8/15/00 
//				TCHAR buf[40];
//				_stprintf(buf,_T("%0.3f"),targDist);
//				SetWindowText(GetDlgItem(hSimpleCamParams,IDC_TARG_DISTANCE),buf);
				tdistSpin->SetValue(GetTDist(t), FALSE);
				}
			if ((drawing != -1) && (coneState || (extDispFlags & EXT_DISP_ONLY_SELECTED))) {
				if(manualClip) {
					DrawCone(t, gw, GetClipDist(t, CAM_HITHER_CLIP),COLOR_CAMERA_CLIP,0,1);
					DrawCone(t, gw, GetClipDist(t, CAM_YON_CLIP),COLOR_CAMERA_CLIP,1,1);
					}
				else
					DrawCone(t,gw,dist,COLOR_CAMERA_CONE,TRUE);
			}
			if(!inode->IsFrozen() && !inode->Dependent())
			{
				// 6/25/01 2:33pm --MQM--
				// if user has changed the color of the camera,
				// use that color for the target line too
				Color color(inode->GetWireColor());
				if ( color != GetUIColor(COLOR_CAMERA_OBJ) )
					gw->setColor( LINE_COLOR, color );
				else
					gw->setColor( LINE_COLOR, GetUIColor(COLOR_TARGET_LINE)); // old method
			}
			v[0] = Point3(0,0,0);
			if (drawing == -1)
				v[1] = Point3(0.0f, 0.0f, -0.9f * dist);
			else
				v[1] = Point3(0.0f, 0.0f, -dist);
			gw->polyline( 2, v, NULL, NULL, FALSE, NULL );	
			}
		}
	else {
		if (coneState || (extDispFlags & EXT_DISP_ONLY_SELECTED))
			if(manualClip) {
				DrawCone(t, gw, GetClipDist(t, CAM_HITHER_CLIP),COLOR_CAMERA_CLIP,0,1);
				DrawCone(t, gw, GetClipDist(t, CAM_YON_CLIP),COLOR_CAMERA_CLIP,1,1);
				}
			else
				DrawCone(t,gw,GetTDist(t),COLOR_CAMERA_CONE,TRUE);
		}
	return gw->checkHitCode();
	}

// From BaseObject
int SimpleCamera::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {
	HitRegion hitRegion;
	DWORD	savedLimits;
	int res;
	Matrix3 m;
	if (!enable) return  0;
	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	gw->clearHitCode();
	res = mesh.select( gw, gw->getMaterial(), &hitRegion, flags & HIT_ABORTONHIT ); 
	// if not, check the target line, and set the pair flag if it's hit
	if( !res )	{
		// this special case only works with point selection of targeted lights
		if((type != HITTYPE_POINT) || !inode->GetTarget())
			return 0;
		// don't let line be active if only looking at selected stuff and target isn't selected
		if((flags & HIT_SELONLY) && !inode->GetTarget()->Selected() )
			return 0;
		gw->clearHitCode();
		if(res = DrawConeAndLine(t, inode, gw, -1))
			inode->SetTargetNodePair(1);
	}
	gw->setRndLimits(savedLimits);
	return res;
}

void SimpleCamera::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;
		Point3 vert(0.0f,0.0f,0.0f);

		gw->wTransPoint(&vert,&screen3);

		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			else // Closer than the best of this priority?
			if(len < snap->bestDist) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			}
		}
	}


int SimpleCamera::DrawRange(TimeValue t, INode *inode, GraphicsWindow *gw)
{
	if(!rangeDisplay)
		return 0;
	gw->clearHitCode();
	Matrix3 tm = inode->GetObjectTM(t);
	gw->setTransform(tm);
	int cnear = 0;
	int cfar = 0;
	if(!inode->IsFrozen() && !inode->Dependent()) { 
		cnear = COLOR_NEAR_RANGE;
		cfar = COLOR_FAR_RANGE;
		}
	DrawCone(t, gw, GetEnvRange(t, ENV_NEAR_RANGE),cnear);
	DrawCone(t, gw, GetEnvRange(t, ENV_FAR_RANGE), cfar, TRUE);
	return gw->checkHitCode();
	}

#define MAXVP_DIST 1.0e8f

void SimpleCamera::GetConePoints(TimeValue t, Point3* q, float dist) {
	if (dist>MAXVP_DIST)
		dist = MAXVP_DIST;
	float ta = (float)tan(0.5*(double)GetFOV(t));
	float w = dist * ta;
//	float h = w * (float).75; //  ASPECT ??
	float h = w / GetAspect();
	q[0] = Point3( w, h,-dist);				
	q[1] = Point3(-w, h,-dist);				
	q[2] = Point3(-w,-h,-dist);				
	q[3] = Point3( w,-h,-dist);				
	}

void SimpleCamera::DrawCone(TimeValue t, GraphicsWindow *gw, float dist, int colid, BOOL drawSides, BOOL drawDiags) {
	Point3 q[5], u[3];
	GetConePoints(t,q,dist);
	if (colid)	gw->setColor( LINE_COLOR, GetUIColor(colid));
	if (drawDiags) {
		u[0] =  q[0];	u[1] =  q[2];	
		gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
		u[0] =  q[1];	u[1] =  q[3];	
		gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
		}
	gw->polyline( 4, q, NULL, NULL, TRUE, NULL );	
	if (drawSides) {
		gw->setColor( LINE_COLOR, GetUIColor(COLOR_CAMERA_CONE));
		u[0] = Point3(0,0,0);
		for (int i=0; i<4; i++) {
			u[1] =  q[i];	
			gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
			}
		}
	}

void SimpleCamera::SetExtendedDisplay(int flags)
{
	extDispFlags = flags;
}


int SimpleCamera::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	if (!enable) return  0;

	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	if (inode->Selected())
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())
	{
		// 6/25/01 2:27pm --MQM--
		// use wire-color to draw the camera
		Color color(inode->GetWireColor());
		gw->setColor( LINE_COLOR, color );	
	}
	mesh.render( gw, gw->getMaterial(), NULL, COMP_ALL);	
	DrawConeAndLine(t,inode,gw,1);
#if 0
	if(horzLineState) {
		Point3 eye, tgt;
		eye = inode->GetObjTMAfterWSM(t).GetTrans();
		if(inode->GetTarget())
			tgt = inode->GetTarget()->GetObjTMAfterWSM(t).GetTrans();
		else {
			m = inode->GetObjTMAfterWSM(t);
			m.PreTranslate(Point3(0.0f, 0.0f, -GetTDist(t)));
			tgt = m.GetTrans();
		}
		tgt[1] = eye[1];
        Point3 pt[10];
        float camDist;
		float ta = (float)tan(0.5*(double)GetFOV(t));

        camDist = (float)sqrt((tgt[0]-eye[0]) * (tgt[0]-eye[0]) + (tgt[2]-eye[2]) * (tgt[2]-eye[2]));
               
        pt[0][0] = -camDist * ta;
        pt[0][1] = 0.0f;
        pt[0][2] = -camDist;
        pt[1][0] = camDist * ta;
        pt[1][1] = 0.0f;
        pt[1][2] = -camDist;
		gw->polyline(2, pt, NULL, NULL, 0, NULL);

	}
#endif
	DrawRange(t, inode, gw);
	gw->setRndLimits(rlim);
	return(0);
	}


RefResult SimpleCamera::EvalCameraState(TimeValue t, Interval& valid, CameraState* cs) {
	cs->isOrtho = IsOrtho();	
	cs->fov = GetFOV(t,valid);
	cs->tdist = GetTDist(t,valid);
	cs->horzLine = horzLineState;
	cs->manualClip = manualClip;
	cs->hither = GetClipDist(t, CAM_HITHER_CLIP, valid);
	cs->yon = GetClipDist(t, CAM_YON_CLIP, valid);
	cs->nearRange = GetEnvRange(t, ENV_NEAR_RANGE, valid);
	cs->farRange = GetEnvRange(t, ENV_FAR_RANGE, valid);
	return REF_SUCCEED;
	}

//
// Reference Managment:
//

RefResult SimpleCamera::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	PartID& partID, RefMessage message ) 
	{
	switch (message) {		
		case REFMSG_WANT_SHOWPARAMLEVEL: {
			BOOL	*pb = ( BOOL * )partID;
			if ( hTarget == ( RefTargetHandle )depthOfFieldPB )
				*pb = TRUE;
			else
				*pb = FALSE;

			return REF_STOP;
			}

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			if ( hTarget == ( RefTargetHandle )pblock )
			{
				switch (gpd->index) {
					case PB_FOV:
						gpd->dim = stdAngleDim;
						break;				
					case PB_TDIST:
					case PB_HITHER:
					case PB_YON:
					case PB_NRANGE:
					case PB_FRANGE:
						gpd->dim = stdWorldDim;
						break;				
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
					case PB_MP_EFFECT_ENABLE:
					case PB_MP_EFF_REND_EFF_PER_PASS:
						gpd->dim = defaultDim;
						break;				
// mjm - end
#endif // NO_CAMERA_MULTIPASS
					}
			}
// nac - begin - 12.08.00
//			else
//			if ( hTarget == ( RefTargetHandle )depthOfFieldPB )
//			{
//				switch (gpd->index) {
//					case PB_DOF_ENABLE:
//					case PB_DOF_FSTOP:
//						gpd->dim = defaultDim;
//						break;		
//				}
//			}
// nac - end
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			if ( hTarget == ( RefTargetHandle )pblock )
			{
				switch (gpn->index) {
					case PB_FOV:
						gpn->name = TSTR(GetString(IDS_RB_FOV));
						break;												
					case PB_TDIST:
						gpn->name = TSTR(GetString(IDS_DB_TDIST));
						break;												
					case PB_HITHER:
						gpn->name = TSTR(GetString(IDS_RB_NEARPLANE));
						break;												
					case PB_YON:
						gpn->name = TSTR(GetString(IDS_RB_FARPLANE));
						break;												
					case PB_NRANGE:
						gpn->name = TSTR(GetString(IDS_DB_NRANGE));
						break;												
					case PB_FRANGE:
						gpn->name = TSTR(GetString(IDS_DB_FRANGE));
						break;												
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
					case PB_MP_EFFECT_ENABLE:
						gpn->name = TSTR(GetString(IDS_MP_EFFECT_ENABLE));
						break;				
					case PB_MP_EFF_REND_EFF_PER_PASS:
						gpn->name = TSTR(GetString(IDS_MP_EFF_REND_EFF_PER_PASS));
						break;				
// mjm - end
#endif // NO_CAMERA_MULTIPASS
					}
			}
// nac - begin - 12.08.00
//			else
//			if ( hTarget == ( RefTargetHandle )depthOfFieldPB )
//			{
//				switch (gpn->index) {
//					case PB_DOF_ENABLE:
//						gpn->name = TSTR(GetString(IDS_DOF_ENABLE));
//						break;												
//					case PB_DOF_FSTOP:
//						gpn->name = TSTR(GetString(IDS_DOF_FSTOP));
//						break;
//				}
//			}
// nac - end
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}


ObjectState SimpleCamera::Eval(TimeValue time){
	// UpdateUI(time);
	return ObjectState(this);
	}

Interval SimpleCamera::ObjectValidity(TimeValue time) {
	Interval ivalid;
	ivalid.SetInfinite();
	if (!waitPostLoad) {
		GetFOV(time,ivalid);
		GetTDist(time,ivalid);
		GetClipDist(time, CAM_HITHER_CLIP, ivalid);
		GetClipDist(time, CAM_YON_CLIP, ivalid);
		GetEnvRange(time, ENV_NEAR_RANGE, ivalid);
		GetEnvRange(time, ENV_FAR_RANGE, ivalid);
// nac - begin - 12.08.00
//		GetDOFEnable(time, ivalid);
//		GetDOFFStop(time, ivalid);
// nac - end
#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
		GetMultiPassEffectEnabled(time, ivalid);
// mjm - end
#endif // NO_CAMERA_MULTIPASS
		UpdateUI(time);
		}
	return ivalid;	
	}


//********************************************************
// LOOKAT CAMERA
//********************************************************


//------------------------------------------------------
class LACamClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SimpleCamera(1); }
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	const TCHAR *	ClassName() { return GetString(IDS_DB_TARGET_CLASS); }
    SClass_ID		SuperClassID() { return CAMERA_CLASS_ID; }
   	Class_ID		ClassID() { return Class_ID(LOOKAT_CAM_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetCameraParams(); }
	};

static LACamClassDesc laCamClassDesc;

extern ClassDesc* GetLookatCamDesc() {return &laCamClassDesc; }

class LACamCreationManager : public MouseCallBack, ReferenceMaker {
	private:
		CreateMouseCallBack *createCB;	
		INode *camNode,*targNode;
		SimpleCamera *camObject;
		TargetObject *targObject;
		int attachedToNode;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 pt0;
		int ignoreSelectionChange;
		int lastPutCount;

		void CreateNewObject();	

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)camNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { camNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		LACamCreationManager()
			{
			ignoreSelectionChange = FALSE;
			}
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_SIMPLECAMCREATE	CID_USER + 1

class LACamCreateMode : public CommandMode {
		LACamCreationManager proc;
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }

		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_SIMPLECAMCREATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1000000; return &proc; }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() {}
		void ExitMode() {}
		BOOL IsSticky() { return FALSE; }
	};

static LACamCreateMode theLACamCreateMode;

//LACamCreationManager::LACamCreationManager( IObjCreate *ioc, ClassDesc *desc )
void LACamCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	camNode         = NULL;
	targNode        = NULL;
	camObject       = NULL;
	targObject      = NULL;
	CreateNewObject();
	}

//LACamCreationManager::~LACamCreationManager
void LACamCreationManager::End()
	{
	if ( camObject ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		camObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) {
			// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete camObject;
			camObject = NULL;
			theHold.Resume();
			// RB 7/28/97: If something has been put on the undo stack since this object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
				}
			macroRec->Cancel();  // JBW 4/23/99
		} else if ( camNode ) {
			 // Get rid of the reference.
			theHold.Suspend();
			DeleteReference(0);  // sets camNode = NULL
			theHold.Resume();
			}
		}	
	}

RefResult LACamCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_PRENOTIFY_PASTE:
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
				}
		 	if ( camObject && camNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if ( camObject && camNode==hTarget ) {
				endEdit:
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				camObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
				camObject  = NULL;				
				camNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			else if (targNode==hTarget) {
				targNode = NULL;
				targObject = NULL;
				}
			break;		
		}
	return REF_SUCCEED;
	}


void LACamCreationManager::CreateNewObject()
	{
	camObject = (SimpleCamera*)cDesc->Create();
	lastPutCount = theHold.GetGlobalPutCount();

    macroRec->BeginCreate(cDesc);  // JBW 4/23/99
	// Start the edit params process
	if ( camObject ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		camObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
		}	
	}

static BOOL needToss;
			
int LACamCreationManager::proc( 
				HWND hwnd,
				int msg,
				int point,
				int flag,
				IPoint2 m )
	{	
	int res;	
	TSTR targName;
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					pt0 = m;
					assert( camObject );					
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					// if cameras were hidden by category, re-display them
					GetCOREInterface()->SetHideByCategoryFlags(
							GetCOREInterface()->GetHideByCategoryFlags() & ~HIDE_CAMERAS);

					if ( attachedToNode ) {
				   		// send this one on its way
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				   		camObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
						macroRec->EmitScript();  // JBW 4/23/99
						
						// Get rid of the reference.
						if (camNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}

						// new object
						CreateNewObject();   // creates camObject
						}

					needToss = theHold.GetGlobalPutCount()!=lastPutCount;

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					camNode = createInterface->CreateObjectNode( camObject);
					attachedToNode = TRUE;
					assert( camNode );					
					createCB = camObject->GetCreateMouseCallBack();
					createInterface->SelectNode( camNode );
					
						// Create target object and node
						targObject = new TargetObject;
						assert(targObject);
						targNode = createInterface->CreateObjectNode( targObject);
						assert(targNode);
						targName = camNode->GetName();
						targName += GetString(IDS_DB_DOT_TARGET);
						targNode->SetName(targName);

						// hook up camera to target using lookat controller.
						createInterface->BindToTarget(camNode,targNode);					

					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, camNode);
					theHold.Resume();

					// Position camera and target at first point then drag.
					mat.IdentityMatrix();
					//mat[3] = vpx->GetPointOnCP(m);
#ifdef _3D_CREATE
					mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
#else
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
#endif
					createInterface->SetNodeTMRelConstPlane(camNode, mat);
						createInterface->SetNodeTMRelConstPlane(targNode, mat);
						camObject->Enable(1);

				   		ignoreSelectionChange = TRUE;
				   		createInterface->SelectNode( targNode,0);
				   		ignoreSelectionChange = FALSE;
						res = TRUE;

					// 6/25/01 2:57pm --MQM-- 
					// set our color to the default camera color
					if ( camNode ) 
					{
						Point3 color = GetUIColor( COLOR_CAMERA_OBJ );
						camNode->SetWireColor( RGB( color.x*255.0f, color.y*255.0f, color.z*255.0f ) );
					}
					break;
					
				case 1:
					if (Length(m-pt0)<2)
						goto abort;
					//mat[3] = vpx->GetPointOnCP(m);
#ifdef _3D_CREATE
					mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
#else
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
#endif
					macroRec->Disable();   // JBW 4/23/99
					createInterface->SetNodeTMRelConstPlane(targNode, mat);
					macroRec->Enable();
				   	ignoreSelectionChange = TRUE;
				   	createInterface->SelectNode( camNode);
				   	ignoreSelectionChange = FALSE;
					
					createInterface->RedrawViews(createInterface->GetTime());  

				    theHold.Accept(GetString(IDS_DS_CREATE));	 

					res = FALSE;	// We're done
					break;
				}			
			break;

		case MOUSE_MOVE:
			//mat[3] = vpx->GetPointOnCP(m);
#ifdef _3D_CREATE
			mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
#else
			mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
#endif
			macroRec->Disable();   // JBW 4/23/99
				createInterface->SetNodeTMRelConstPlane(targNode, mat);
			macroRec->Enable();
			createInterface->RedrawViews(createInterface->GetTime());	   

			macroRec->SetProperty(camObject, _T("target"),   // JBW 4/23/99
				mr_create, Class_ID(TARGET_CLASS_ID, 0), GEOMOBJECT_CLASS_ID, 1, _T("transform"), mr_matrix3, &mat);

			res = TRUE;
			break;

		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
#ifdef _OSNAP
			//Snap Preview
			#ifdef _3D_CREATE
			vpx->SnapPreview(m,m,NULL, SNAP_IN_3D);
			#else
			vpx->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
			#endif
#endif
			break;

	    case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;

		case MOUSE_ABORT:
			abort:
			assert( camObject );
#ifndef NO_CREATE_TASK	// russom - 12/04/01
			camObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
#endif
			macroRec->Cancel();  // JBW 4/23/99
			theHold.Cancel();	 // deletes both the camera and target.
			// Toss the undo stack if param changes have been made
			if (needToss) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			camNode = NULL;			
			targNode = NULL;	 	
			createInterface->RedrawViews(createInterface->GetTime()); 
			CreateNewObject();	
			attachedToNode = FALSE;
			res = FALSE;						
		}
	
	done:
	createInterface->ReleaseViewport(vpx); 
	return res;
	}

int LACamClassDesc::BeginCreate(Interface *i)
	{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	
	//iob->SetMouseProc( new LACamCreationManager(iob,this), 1000000 );

	theLACamCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theLACamCreateMode );
	
	return TRUE;
	}

int LACamClassDesc::EndCreate(Interface *i)
	{
	
	ResumeSetKeyMode();
	theLACamCreateMode.End();
	i->RemoveMode( &theLACamCreateMode );
	macroRec->EmitScript();  // JBW 4/23/99

	return TRUE;
	}

RefTargetHandle SimpleCamera::Clone(RemapDir& remap) {
	SimpleCamera* newob = new SimpleCamera();
	newob->ReplaceReference(0,pblock->Clone(remap));

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
// mjm - begin - 07.17.00
	if ( GetReference(MP_EFFECT_REF) )
	{
		newob->ReplaceReference( MP_EFFECT_REF, remap.CloneRef( GetReference(MP_EFFECT_REF) ) );
	}
// mjm - end
#endif // NO_CAMERA_MULTIPASS

	newob->enable = enable;
	newob->hasTarget = hasTarget;
	newob->coneState = coneState;
	newob->manualClip = manualClip;
	newob->rangeDisplay = rangeDisplay;
	newob->isOrtho = isOrtho;
	BaseClone(this, newob, remap);
	return(newob);
	}

#define CAMERA_FOV_CHUNK 	0x2680
#define CAMERA_TARGET_CHUNK 0x2682
#define CAMERA_CONE_CHUNK 	0x2684
#define CAMERA_MANUAL_CLIP	0x2686
#define CAMERA_HORIZON		0x2688
#define CAMERA_RANGE_DISP	0x268a
#define CAMERA_IS_ORTHO		0x268c

// IO
IOResult SimpleCamera::Save(ISave *isave) {
		
#if 0
	ULONG nb;
	Interval valid;
	float fov;
	pblock->GetValue( 0, 0, fov, valid );
	
	isave->BeginChunk(CAMERA_FOV_CHUNK);
	isave->Write(&fov, sizeof(FLOAT), &nb);
	isave->EndChunk();
#endif

	if (hasTarget) {
		isave->BeginChunk(CAMERA_TARGET_CHUNK);
		isave->EndChunk();
		}
	if (coneState) {
		isave->BeginChunk(CAMERA_CONE_CHUNK);
		isave->EndChunk();
		}
	if (rangeDisplay) {
		isave->BeginChunk(CAMERA_RANGE_DISP);
		isave->EndChunk();
		}
	if (isOrtho) {
		isave->BeginChunk(CAMERA_IS_ORTHO);
		isave->EndChunk();
		}
	if (manualClip) {
		isave->BeginChunk(CAMERA_MANUAL_CLIP);
		isave->EndChunk();
		}
	if (horzLineState) {
		isave->BeginChunk(CAMERA_HORIZON);
		isave->EndChunk();
		}
	return IO_OK;
	}

class CameraPostLoad : public PostLoadCallback {
public:
	SimpleCamera *sc;
	Interval valid;
	CameraPostLoad(SimpleCamera *cam) { sc = cam;}
	void proc(ILoad *iload) {
		if (sc->pblock->GetVersion() != CAMERA_VERSION) {
			switch (sc->pblock->GetVersion()) {
// mjm - begin - 07.17.00
			case 0:
				sc->ReplaceReference(0,
						UpdateParameterBlock(
							descV0, 2, sc->pblock,
							descV4, CAMERA_PBLOCK_COUNT, CAMERA_VERSION));
				iload->SetObsolete();
				break;

			case 1:
				sc->ReplaceReference(0,
						UpdateParameterBlock(
							descV1, 4, sc->pblock,
							descV4, CAMERA_PBLOCK_COUNT, CAMERA_VERSION));
				iload->SetObsolete();
				break;

			case 2:
				sc->ReplaceReference(0,
						UpdateParameterBlock(
							descV2, 6, sc->pblock,
							descV4, CAMERA_PBLOCK_COUNT, CAMERA_VERSION));
				iload->SetObsolete();
				break;

			case 3:
				sc->ReplaceReference(0,
						UpdateParameterBlock(
							descV3, 7, sc->pblock,
							descV4, CAMERA_PBLOCK_COUNT, CAMERA_VERSION));
				iload->SetObsolete();
				break;
// mjm - end

			default:
				assert(0);
				break;
			}
		}
// nac - begin - 12.08.00

#ifndef NO_CAMERA_MULTIPASS	// russom - 05/20/01
		if (( sc->depthOfFieldPB != NULL ) && ( sc->depthOfFieldPB->GetVersion() != DOF_VERSION )) {
			switch (sc->depthOfFieldPB->GetVersion()) {
			case 1:
			{
				sc->ReplaceReference(DOF_REF,
						UpdateParameterBlock(
							descV0, 2, sc->depthOfFieldPB,
							descV1, 2, DOF_VERSION));
				iload->SetObsolete();

				// Replicate the existing data into an mr dof mp effect as much as possible
				IMultiPassCameraEffect *pIMultiPassCameraEffect = NULL;
				int numClasses = SimpleCamera::smCompatibleEffectList.Count();
				for (int i=0; i<numClasses; i++)
				{
					// mental ray DOF mp effect
					if ( SimpleCamera::smCompatibleEffectList[i]->CD()->ClassID() == Class_ID (0x6cc9546e, 0xb1961b9) ) // from dll\mentalray\translator\src\mrDOF.cpp
					{
						BOOL		dofEnable;
						TimeValue   t = 0;
						Interval	valid;

						sc->depthOfFieldPB->GetValue ( PB_DOF_ENABLE, t, dofEnable, valid );
						if ( dofEnable )
						{
							sc->SetMultiPassEffectEnabled ( t, dofEnable );

							pIMultiPassCameraEffect = reinterpret_cast<IMultiPassCameraEffect *>( SimpleCamera::smCompatibleEffectList[i]->CD()->Create(0) );
							sc->SetIMultiPassCameraEffect(pIMultiPassCameraEffect);
							IParamBlock2	*pb = pIMultiPassCameraEffect->GetParamBlock ( 0 );

							if ( pb )
							{
								float	fstop;
								enum
								{
									prm_fstop,
								};

								sc->depthOfFieldPB->GetValue ( PB_DOF_FSTOP, t, fstop, valid );
								pb->SetValue ( prm_fstop, t, fstop );
							}
						}
					}
				}

				break;
			}
			default:
				assert(0);
				break;
			}
		}
#endif // NO_CAMERA_MULTIPASS

		waitPostLoad--;
		delete this;
	}
};

IOResult  SimpleCamera::Load(ILoad *iload) {
	IOResult res;
#if 0
	ULONG nb;
	float fov;
#endif
	enable = TRUE;
	coneState = 0;
	manualClip = 0;
	horzLineState = 0;
	rangeDisplay = 0;
	isOrtho = 0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
#if 0
			case CAMERA_FOV_CHUNK:
				res = iload->Read(&fov,sizeof(FLOAT), &nb);
				pblock->SetValue( 0, 0, fov );
				break;
#endif
			case CAMERA_TARGET_CHUNK:
				hasTarget = 1;
				break;
			case CAMERA_CONE_CHUNK:
				coneState = 1;
				break;
// The display environment range property is not loaded and always set to false
#ifndef NO_CAMERA_ENVRANGE
			case CAMERA_RANGE_DISP:
				rangeDisplay = 1;
				break;
#endif // NO_CAMERA_ENVRANGE
			case CAMERA_IS_ORTHO:
				isOrtho = 1;
				break;
			case CAMERA_MANUAL_CLIP:
				manualClip = 1;
				break;
			case CAMERA_HORIZON:
				horzLineState = 1;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	waitPostLoad++;
	iload->RegisterPostLoadCallback(new CameraPostLoad(this));
	return IO_OK;
	}

#ifdef DESIGN_VER
// parallel only camera classes

#include <iparamm.h>
#include <float.h>
#include <polyshp.h>

const Class_ID PARALLEL_CAMERA_CLASS_ID(0x2bc22a1e, 0x621207d);

class ParallelCamera;

class PcamCustomPanel : public ParamMapUserDlgProc // abstract base class
{
private:
  HWND _hPanel;
  ParallelCamera *_pCam;

public:
/*  void FillCombo(const int toFill,const int numItems,const int items[],const int startItem = 0) const;
  void Disable(const int item) const {EnableWindow(GetDlgItem(_hPanel,item),FALSE);}
  void Enable(const int item) const {EnableWindow(GetDlgItem(_hPanel,item),TRUE);}
  void DisableButton(int item) const;
  void EnableButton(int item) const;*/
  void DisableEdit(int item) const;
  void EnableEdit(int item) const;
  void DisableSpinner(int item) const;
  void EnableSpinner(int item) const;

  // inherited from ParamMapUserDlgProc
  BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() {delete this;}
  void SetCam(ParallelCamera *pc) {_pCam = pc;}
};

INT_PTR CALLBACK StandardPanelDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

class ParallelCameraCreateCallBack : public CreateMouseCallBack // singleton pattern
{
private:
  static ParallelCameraCreateCallBack* _instance;
  ParallelCamera *_pCam;

protected:
  ParallelCameraCreateCallBack(void) {;}

public:
  static ParallelCameraCreateCallBack* Instance(void);
  ParallelCamera *GetObj(void) const {return _pCam;}

  // --- Inherited virtual methods of CreateMouseCallBack ---
  int proc(ViewExp *vpt,int msg,int point,int flags,IPoint2 m,Matrix3& mat);
  void SetCam(ParallelCamera *pc) {_pCam = pc;}
};

class ParallelCamera: public GenCamera
{
  friend INT_PTR CALLBACK StandardPanelDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
  friend int ParallelCameraCreateCallBack::proc(ViewExp *vpt,int msg,int point,int flags,IPoint2 m,Matrix3& mat);
	friend BOOL PcamCustomPanel::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

private:
  static ParamBlockDescID _parameterBlock[];
  static ParamUIDesc _customPanelData[];
  static ParamVersionDesc *_versions,_curVersion;
  //  static ParamVersionDesc _versions[],_curVersion; // if more than 0 versions

  static IParamMap *_customMap;
  static PcamCustomPanel *_customPanel;

  static float _axisAngleX;
  static float _axisAngleY;
  static float _axisAngleZ;
  static float _clipNear;
  static float _clipFar;
  static bool _showSection;
  static float _height;
  static float _width;
  static float _aspect;
  static bool _showVolume;
  static float _nearEnv;
  static float _farEnv;
  static bool _showEnv;

	IObjParam *_ip;
  bool _isCreating;
  static HWND _hStandardPanel;

  IParamBlock *_pblock;
	Mesh _mesh;
	BOOL _suspendSnap;
	int _extDispFlags;		
  int _enable;
  Box3 _selBox;

	enum Fixed{FIX_ASPECT,FIX_HEIGHT,FIX_WIDTH};
	Fixed _fixed;

	// sectioning front view plane
	PolyShape _shape;
	TimeValue	_curTime;			// the current time
	Matrix3	_sliceTM_inv;		// inverse of tm
	INode* _curNode;			// the curent node being sliced
	Point3 _tmBbox[8];		// corners of an object's bbox, in slice space
	Mesh* _curMesh;			// the mesh of an object
	PolyLine _faceLines[3];		// for computing face intersection lines
	Tab<Point3>	_pointList;			// for storing up the segment list points
	bool _update;

public:
  ParallelCamera(void);
  ~ParallelCamera(void){;}

  // inherited virtual methods

  // inherited from GenCamera
  GenCamera *NewCamera(int type) {return new SimpleCamera(type);}
  void SetFOV(TimeValue t, float f);
  float GetFOV(TimeValue t, Interval& valid = Interval(0,0));
  void SetLens(TimeValue t, float f); // unipmlemented
  float GetLens(TimeValue t, Interval& valid = Interval(0,0)); // unipmlemented
  void SetTDist(TimeValue t, float f);
  float GetTDist(TimeValue t, Interval& valid = Interval(0,0));
  void SetConeState(int s);
  int GetConeState();
  void SetHorzLineState(int s){;} // no horizon by definition
  int GetHorzLineState(){return 0;} // ditto
  int GetManualClip(){return !0;} // always on
  void SetManualClip(int onOff){;} // ditto
  float GetClipDist(TimeValue t, int which, Interval &valid = Interval(0,0));
  void SetClipDist(TimeValue t, int which, float f);
  float MMtoFOV(float mm) {return 0.0f;}
  float FOVtoMM(float fov)  {return 0.0f;}
  float CurFOVtoWFOV(float cfov) {return cfov;} // always FOV_W
  float WFOVtoCurFOV(float wfov) {return wfov;} // ditto
  void SetEnvRange(TimeValue time, int which, float f);
  float GetEnvRange(TimeValue t, int which, Interval& valid = Interval(0,0));
  void SetEnvDisplay(BOOL b, int notify=TRUE);
  BOOL GetEnvDisplay(void);
  void Enable(int e) {_enable = e;}
  BOOL SetFOVControl(Control *c){return FALSE;} //!!
  Control *GetFOVControl(){return 	_pblock->GetController(0);} //!!
  void  SetFOVType(int ft) {;} // always FOV_W
  int GetFOVType() {return FOV_W;} // ditto
	int  Type() {return PARALLEL_CAMERA;}
	void SetType(int tp);

  // inherited from CameraObject
  void SetOrtho(BOOL b) {;} // true by definition
  BOOL IsOrtho() {return TRUE;} // ditto
  void RenderApertureChanged(TimeValue t){;} // parallel cam setting will override this

  // From BaseObject
	int HitTest(TimeValue t,INode* inode,int type,int crossing,int flags,IPoint2 *p,ViewExp *vpt);
	void Snap(TimeValue t,INode* inode,SnapInfo *snap,IPoint2 *p,ViewExp *vpt);
  void SetExtendedDisplay(int flags) {_extDispFlags = flags;}
	int Display(TimeValue t,INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack(void);
	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
  TCHAR *GetObjectName(void) {return GetString(IDS_PARALLEL_CAMERA);}
	void GetWorldBoundBox(TimeValue t,INode *mat,ViewExp* vpt,Box3& box);
	void GetLocalBoundBox(TimeValue t,INode *mat,ViewExp* vpt,Box3& box);
	void GetDeformBBox(TimeValue t,Box3& box,Matrix3 *tm,BOOL useSel);

	BOOL HasViewDependentBoundingBox() { return true; }

	// From Object
	ObjectState Eval(TimeValue time);
	void InitNodeName(TSTR& s) {s = GetString(IDS_DB_CAMERA);}		
	int DoOwnSelectHilite(void) {return 1;}
		
	// From Camera
	RefResult EvalCameraState(TimeValue t,Interval& valid,CameraState* cs);

	// Animatable methods
	void DeleteThis(void) {delete this;}
  Class_ID ClassID(void) {return PARALLEL_CAMERA_CLASS_ID;}
  void GetClassName(TSTR& s) {s = TSTR(GetString(IDS_PARALLEL_CAMERA_CLASS));}
		
	int NumSubs() {return 1;}  
	Animatable* SubAnim(int i) {return _pblock;}
	TSTR SubAnimName(int i) {return TSTR(GetString(IDS_RB_PARAMETERS));}

	// From ref
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	int NumRefs(void) {return 1;}
	RefTargetHandle GetReference(int i) {return _pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {_pblock = (IParamBlock*)rtarg;}
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID,RefMessage message);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

  LRESULT CALLBACK TrackViewWinProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){return(0);}

  // NOT inherited
  IObjParam *Ip(void) const {return _ip;}
  static void Reset(void);
  void InvalidateUI(void) {if(_customMap) _customMap->Invalidate();}

  // temp
	void Set(Point3 dir,Box3 &box,TimeValue t);

private:
	void SceneGeomBoundBox(INode *node,Box3 &box);

  // viewport utilities
  void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
  void ComputeRectPoints(TimeValue t,float dist,Point3 *pts);
  int DrawWires(TimeValue t,INode *inode,GraphicsWindow *gw);
  void DrawVolume(TimeValue t,GraphicsWindow *gw,float dist,int colid,bool drawSides,bool drawDiags);
  void MakeQuad(Face *f,int a,int b,int c,int d,int sg,int dv = 0);
  void BuildMesh(void);

	// sectioning front clip plane - predominantly stolen from section & simplified
	void	TraverseWorld(INode *node,const int level,Box3& boundBox);
	void	DoMeshSlice(void);				// computes the slice thru an object's mesh
	int		CrossCheck(const Point3& pt1,	// checks that a line segment p1-p2 crosses the plane
		               const Point3& pt2,	// if so, puts cross in cross_pt	
					   Point3* cross_pt);	// returns 0 for no slice, 1 for one pt, 2 for coplanar
	void	CrossPoint(const Point3& pt1,	// same as CrossCheck, but we know the points cross the xy plane
		               const Point3& pt2, 
					   Point3* cross_pt);
	bool EqualPoint3(const Point3 p1,const Point3 p2);
	TriObject *GetTriObjectFromNode(INode *node,const TimeValue t,bool &deleteIt);
	void SliceASceneNode(void); // compute slice thru a node
	void SliceFace(Point3* t_verts, const DWORD f_index); // compute a slice trun a mesh's face
	void UpdateSlice(INode *sectionNode,const TimeValue t,const bool force = false);
	bool EqualMatrices(const Matrix3& m1,const Matrix3& m2); 
	// FLT_EPSILON from float.h: 1.192092896e-07F	- smallest such that 1.0+FLT_EPSILON != 1.0
  int Sign(float f) {return fabs(f)<=FLT_EPSILON?0:(f>0.0f?1:-1);}
};

// class descriptor, singleton pattern
class ParallelCamClassDesc : public ClassDesc
{
public:
  static ParallelCamClassDesc *Instance(void);
  //  int IsPublic() {return(GetAppID()==kAPP_VIZ?1:0);} //plug-in is for VIZ only
  int IsPublic() {return(GetAppID()==0);} //make this unavailable from the creation panel
  void *Create(BOOL loading = FALSE) {return new ParallelCamera;}
  const TCHAR	*ClassName() {return GetString(IDS_PARALLEL_CAMERA_CLASS);}
  SClass_ID	SuperClassID() {return CAMERA_CLASS_ID;}
  Class_ID ClassID() {return PARALLEL_CAMERA_CLASS_ID;}
  const TCHAR	*Category() {return _T("Standard");}
  void ResetClassParams(BOOL fileReset) {ParallelCamera::Reset();}
protected:
  ParallelCamClassDesc(void) {;}
private:
  static ParallelCamClassDesc *_instance;
};

// declare the single instance of the class descriptor.
ParallelCamClassDesc *ParallelCamClassDesc::_instance = NULL;

ParallelCamClassDesc *ParallelCamClassDesc::Instance(void)
{
  if(!_instance)
    _instance = new ParallelCamClassDesc;
  return _instance;
}

ClassDesc* GetParallelCamDesc() {return ParallelCamClassDesc::Instance();}

// parameter block indices
const int PB_PCAM_AXIS_ANGLE_X = 0;
const int PB_PCAM_AXIS_ANGLE_Y = 1;
const int PB_PCAM_AXIS_ANGLE_Z = 2;
const int PB_PCAM_CLIP_NEAR = 3;
const int PB_PCAM_CLIP_FAR = 4;
const int PB_PCAM_SHOW_SECTION = 5;
const int PB_PCAM_HEIGHT = 6;
const int PB_PCAM_WIDTH = 7;
const int PB_PCAM_ASPECT = 8;
const int PB_PCAM_SHOW_VOLUME = 9;
const int PB_PCAM_NEAR_ENV = 10;
const int PB_PCAM_FAR_ENV = 11;
const int PB_PCAM_SHOW_ENV = 12;

const int PCAM_PBLOCK_LENGTH	= 13;

const int PCAM_CURRENT_VERSION = 0;
const int PCAM_OLDVERSIONS = 0;

ParamBlockDescID ParallelCamera::_parameterBlock[] = 
{
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_AXIS_ANGLE_X},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_AXIS_ANGLE_Y},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_AXIS_ANGLE_Z},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_CLIP_NEAR},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_CLIP_FAR},
  {TYPE_BOOL,NULL,FALSE,PB_PCAM_SHOW_SECTION},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_HEIGHT},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_WIDTH},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_ASPECT},
  {TYPE_BOOL,NULL,FALSE,PB_PCAM_SHOW_VOLUME},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_NEAR_ENV},
  {TYPE_FLOAT,NULL,TRUE,PB_PCAM_FAR_ENV},
  {TYPE_BOOL,NULL,FALSE,PB_PCAM_SHOW_ENV}
};

// Array of old versions
ParamVersionDesc *ParallelCamera::_versions = NULL;
//ParamVersionDesc ParallelCamera::versions[] = {};

// Current version
ParamVersionDesc ParallelCamera::_curVersion(_parameterBlock,PCAM_PBLOCK_LENGTH,0);

static const float NINETY = 90.0f;
static const float MAX_FLT = 1.0e30f;

ParamUIDesc ParallelCamera::_customPanelData[] =
{
  ParamUIDesc(PB_PCAM_AXIS_ANGLE_X,EDITTYPE_UNIVERSE,
		IDC_PCAM_ANGLE_X,IDC_PCAM_ANGLE_X_SPINNER,
		-NINETY,NINETY,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_AXIS_ANGLE_Y,EDITTYPE_UNIVERSE,
		IDC_PCAM_ANGLE_Y,IDC_PCAM_ANGLE_Y_SPINNER,
		-NINETY,NINETY,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_AXIS_ANGLE_Z,EDITTYPE_UNIVERSE,
		IDC_PCAM_ANGLE_Z,IDC_PCAM_ANGLE_Z_SPINNER,
		-NINETY,NINETY,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_CLIP_NEAR,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_CLIP_NEAR,IDC_PCAM_CLIP_NEAR_SPINNER,
		MIN_CLIP,MAX_CLIP,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_CLIP_FAR,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_CLIP_FAR,IDC_PCAM_CLIP_FAR_SPINNER,
		MIN_CLIP,MAX_CLIP,SPIN_AUTOSCALE),
	ParamUIDesc(PB_PCAM_SHOW_SECTION,TYPE_SINGLECHEKBOX,IDC_PCAM_SHOW_SECTION),
  ParamUIDesc(PB_PCAM_HEIGHT,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_HEIGHT,IDC_PCAM_HEIGHT_SPINNER,
		0.0f,MAX_FLT,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_WIDTH,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_WIDTH,IDC_PCAM_WIDTH_SPINNER,
		0.0f,MAX_FLT,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_ASPECT,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_ASPECT,IDC_PCAM_ASPECT_SPINNER,
		0.0f,MAX_FLT,SPIN_AUTOSCALE),
	ParamUIDesc(PB_PCAM_SHOW_VOLUME,TYPE_SINGLECHEKBOX,IDC_PCAM_SHOW_VOLUME),
  ParamUIDesc(PB_PCAM_NEAR_ENV,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_NEAR_ENV,IDC_PCAM_NEAR_ENV_SPINNER,
		0.0f,MAX_FLT,SPIN_AUTOSCALE),
  ParamUIDesc(PB_PCAM_FAR_ENV,EDITTYPE_POS_UNIVERSE,
		IDC_PCAM_FAR_ENV,IDC_PCAM_FAR_ENV_SPINNER,
		0.0f,MAX_FLT,SPIN_AUTOSCALE),
	ParamUIDesc(PB_PCAM_SHOW_ENV,TYPE_SINGLECHEKBOX,IDC_PCAM_SHOW_ENV)
};

const int CUSTOM_PANEL_LENGTH = 13;

IParamMap *ParallelCamera::_customMap = NULL;
PcamCustomPanel *ParallelCamera::_customPanel = NULL;
HWND ParallelCamera::_hStandardPanel = NULL;

float ParallelCamera::_axisAngleX = 0.0f;
float ParallelCamera::_axisAngleY = 0.0f;
float ParallelCamera::_axisAngleZ = 0.0f;
float ParallelCamera::_clipNear = 1.0f;
float ParallelCamera::_clipFar = 1000.0f;
bool ParallelCamera::_showSection = false;
float ParallelCamera::_height = 100.0f;
float ParallelCamera::_width = 100.0f;
float ParallelCamera::_aspect = 1.0f;
bool ParallelCamera::_showVolume = true;
float ParallelCamera::_nearEnv = 1.0f;
float ParallelCamera::_farEnv = 1000.0f;
bool ParallelCamera::_showEnv = false;

ParallelCameraCreateCallBack *ParallelCameraCreateCallBack::_instance = NULL;

ParallelCameraCreateCallBack *ParallelCameraCreateCallBack::Instance(void)
{
  if(!_instance)
    _instance = new ParallelCameraCreateCallBack;
  return _instance;
}

int ParallelCameraCreateCallBack::proc(ViewExp *vpt,int msg,int point,int flags,IPoint2 m,Matrix3& mat)
{
  Point3 p0,p1;
  Box3 box;
  
  int ret = CREATE_CONTINUE;

	if(msg == MOUSE_FREEMOVE)
    vpt->SnapPreview(m,m,NULL,SNAP_IN_3D);

	if(msg==MOUSE_POINT||msg==MOUSE_MOVE)
  {
    switch(point)
    {
    case 0:  // only happens with MOUSE_POINT msg
		{
      _pCam->_suspendSnap = TRUE;				
      p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);

			p0.z = _pCam->_selBox.Center().z;

			Point3 diff = _pCam->_selBox.Center()-p0;
			diff = Normalize(diff);
			// build a new basis matrix
			mat.SetRow(2,-diff);
			mat.SetRow(1,Point3(0.0f,0.0f,1.0f));
			mat.SetRow(0,diff^mat.GetRow(1));

      _pCam->Set(Normalize(-p0),(_pCam->_selBox)*Inverse(mat),_pCam->Ip()->GetTime());

			mat.SetTrans(p0);

      if(msg==MOUSE_POINT)
      {
        _pCam->_suspendSnap = FALSE;
        ret = CREATE_STOP;
      }
      _pCam->InvalidateUI();

      break;
    }
		case 1:
			/*p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
			Point3 diff = p1-p0;
			diff = Normalize(diff);
			// build a new basis matrix
			mat.SetRow(0,diff);
			mat.SetRow(1,diff^mat.GetRow(2));
			_pCam->InvalidateUI();*/
      break;
    default:
      ret = CREATE_ABORT;
      break;
    }
  }

	else if(msg == MOUSE_ABORT)
		ret = CREATE_ABORT;
  return ret;
}





void ParallelCamera::Set(Point3 dir,Box3 &box,TimeValue t)
{
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_X,t,(float)RadToDeg(acos(dir.x)));
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_Y,t,(float)RadToDeg(acos(dir.y)));
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_Z,t,(float)RadToDeg(acos(dir.z)));

	float width = box.pmax.x-box.pmin.x,height = box.pmax.y-box.pmin.y;
  _pblock->SetValue(PB_PCAM_HEIGHT,t,height);
  _pblock->SetValue(PB_PCAM_WIDTH,t,width);
  _pblock->SetValue(PB_PCAM_ASPECT,t,width/height);

  //_pblock->SetValue(PB_PCAM_CLIP_NEAR,t,);
  //_pblock->SetValue(PB_PCAM_CLIP_FAR,t,);
}





ParallelCamera::ParallelCamera(void)
{
  // Create the parameter block and make a reference to it
  MakeRefByID(FOREVER,0,_pblock = CreateParameterBlock(_parameterBlock,PCAM_PBLOCK_LENGTH,PCAM_CURRENT_VERSION));
  assert(_pblock);
  // initial values
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_X,0,_axisAngleX);
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_Y,0,_axisAngleY);
  _pblock->SetValue(PB_PCAM_AXIS_ANGLE_Z,0,_axisAngleZ);
  _pblock->SetValue(PB_PCAM_CLIP_NEAR,0,_clipNear);
  _pblock->SetValue(PB_PCAM_CLIP_FAR,0,_clipFar);
  _pblock->SetValue(PB_PCAM_SHOW_SECTION,0,_showSection);
  _pblock->SetValue(PB_PCAM_HEIGHT,0,_height);
  _pblock->SetValue(PB_PCAM_WIDTH,0,_width);
  _pblock->SetValue(PB_PCAM_ASPECT,0,_aspect);
  _pblock->SetValue(PB_PCAM_SHOW_VOLUME,0,_showVolume);
  _pblock->SetValue(PB_PCAM_NEAR_ENV,0,_nearEnv);
  _pblock->SetValue(PB_PCAM_FAR_ENV,0,_farEnv);
  _pblock->SetValue(PB_PCAM_SHOW_ENV,0,_showEnv);

  BuildMesh(); // for the display

	// used for construction face slices
	_faceLines[0].SetNumPts(2,FALSE);
	_faceLines[1].SetNumPts(2,FALSE);
	_faceLines[2].SetNumPts(2,FALSE);

	_curTime = TimeValue(0);
	_curNode = NULL;
	_curMesh = NULL;
	_update = false;

	_fixed = FIX_ASPECT; // initially aspect is fixed
}

void ParallelCamera::Reset(void)
{
  _axisAngleX = 0.0f;
  _axisAngleY = 0.0f;
  _axisAngleZ = 0.0f;
  _clipNear = 1.0f;
  _clipFar = 1000.0f;
  _showSection = false;
  _height = 100.0f;
  _width = 100.0f;
  _aspect = 1.0f;
  _showVolume = true;
  _nearEnv = 1.0f;
  _farEnv = 1000.0f;
  _showEnv = false;
}

void ParallelCamera::SetFOV(TimeValue t,float fov)
{
  Interval valid;
  float front;
  _pblock->GetValue(PB_PCAM_CLIP_NEAR,t,front,valid);
  _pblock->SetValue(PB_PCAM_WIDTH,t,float(tan(fov*0.5)*2.0*front));
}

float ParallelCamera::GetFOV(TimeValue t,Interval& valid)
 {	
  float front,width;
  _pblock->GetValue(PB_PCAM_CLIP_NEAR,t,front,valid);
	_pblock->GetValue(PB_PCAM_WIDTH,t,width,valid);
	float fov = float(2.0f*atan(0.5f*width/front));
	if(fov < 0.0f)
    fov = 0.0f;
  return fov;
}

void ParallelCamera::SetTDist(TimeValue t,float f)
{
	_pblock->SetValue(PB_PCAM_CLIP_NEAR,t,f);
}

float ParallelCamera::GetTDist(TimeValue t,Interval& valid)
 {	
	float f;
	_pblock->GetValue(PB_PCAM_CLIP_NEAR,t,f,valid);
	if(f < MIN_CLIP)
    f = MIN_CLIP;
	if(f > MAX_CLIP)
    f = MAX_CLIP;
	return f;
}

void ParallelCamera::SetClipDist(TimeValue t,int which,float f)
{
	_pblock->SetValue(PB_PCAM_CLIP_NEAR+which-1,t,f);
}

float ParallelCamera::GetClipDist(TimeValue t,int which,Interval &valid)
{
	float f;
	_pblock->GetValue(PB_PCAM_CLIP_NEAR+which-1,t,f,valid);
	if(f < MIN_CLIP)
    f = MIN_CLIP;
	if(f > MAX_CLIP)
    f = MAX_CLIP;
	return f;
}

float ParallelCamera::GetEnvRange(TimeValue t,int which,Interval &valid)
{
	float f;
	_pblock->GetValue(PB_PCAM_NEAR_ENV+which,t,f,valid);
	return f;
}

void ParallelCamera::SetEnvRange(TimeValue t,int which,float f)
{
	_pblock->SetValue(PB_PCAM_NEAR_ENV+which,t,f);
}

BOOL ParallelCamera::GetEnvDisplay(void)
{
	int b;
  Interval valid;
	_pblock->GetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),b,valid);
	return b;
}

void ParallelCamera::SetEnvDisplay(BOOL b,int notify)
{
	_pblock->SetValue(PB_PCAM_SHOW_ENV,TimeValue(0),b);
}

int ParallelCamera::GetConeState(void)
{
	int s;
  Interval valid;
	_pblock->GetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),s,valid);
	return s;
}

void ParallelCamera::SetConeState(int s)
{
	_pblock->SetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),s);
}

void ParallelCamera::SceneGeomBoundBox(INode *node,Box3 &box)
{
	// find this node's box
	Box3 dBox;
	Interval valid;
	ObjectState os = node->EvalWorldState(_ip->GetTime(),FALSE);
	if(os.obj && os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{
		Matrix3	m = node->GetObjectTM(_ip->GetTime(),&valid);
		os.obj->GetDeformBBox(_ip->GetTime(),dBox,&m);
	
		// add it to the running sum
		box += dBox;
	}

  for(int c = 0;c<node->NumberOfChildren();c++)
	{
		// now add in the children's boxes
		SceneGeomBoundBox(node->GetChildNode(c),box);
	}
}

void ParallelCamera::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip = ip;
	_isCreating = (flags&BEGIN_EDIT_CREATE)?true:false;

  // save the size of the current selection bounding box
  _selBox.Init();
  _ip->GetSelectionWorldBox(_ip->GetTime(),_selBox);

	// if empty use the entire scene
	if(_selBox.IsEmpty())
		SceneGeomBoundBox(_ip->GetRootNode(),_selBox);

  if(_customMap) // left over from last time
  {
    SetWindowLongPtr(_hStandardPanel,GWLP_USERDATA,(LONG_PTR)this);
    _customMap->SetParamBlock(_pblock);
		_customPanel->SetCam(this);
  }
  else
  {
		// make new ones
    _hStandardPanel = ip->AddRollupPage(hInstance,MAKEINTRESOURCE(IDD_PCAM_STANDARD),StandardPanelDlgProc,
                                        GetString(IDS_PCAM_STANDARD_GROUP),(LPARAM)this,_isCreating?0:APPENDROLL_CLOSED);
    ip->RegisterDlgWnd(_hStandardPanel);    

    _customMap = CreateCPParamMap(_customPanelData,CUSTOM_PANEL_LENGTH,
      _pblock,ip,hInstance,
      MAKEINTRESOURCE(IDD_PCAM_CUSTOM),
      GetString(IDS_PCAM_CUSTOM_GROUP),_isCreating?APPENDROLL_CLOSED:0);

    if(_customMap)
		{
      _customMap->SetUserDlgProc(_customPanel = new PcamCustomPanel());
			_customPanel->SetCam(this);
		}
  } 
}

void ParallelCamera::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{		
	_ip = NULL;

	if(flags&END_EDIT_REMOVEUI)
  {
		// Remove the rollup pages from the command panel
    // which in turn deletes the idividual panels
		if(_customMap)
    {
      DestroyCPParamMap(_customMap);
    }
	  if(_hStandardPanel)
    {
      ip->UnRegisterDlgWnd(_hStandardPanel);
      ip->DeleteRollupPage(_hStandardPanel);
    }
    _customMap = NULL;
		_customPanel = NULL;
    _hStandardPanel = NULL;
	}
  else
		SetWindowLongPtr(_hStandardPanel,GWLP_USERDATA,0);

	// Save these values in static variables so the next object created will inherit them.
//  _pblock->GetValue(PB_PCAM_AXIS_ANGLE_X,ip->GetTime(),_axisAngleX,FOREVER);
//  _pblock->GetValue(PB_PCAM_AXIS_ANGLE_Y,ip->GetTime(),_axisAngleY,FOREVER);
//  _pblock->GetValue(PB_PCAM_AXIS_ANGLE_Z,ip->GetTime(),_axisAngleZ,FOREVER);
  _pblock->GetValue(PB_PCAM_CLIP_NEAR,ip->GetTime(),_clipNear,FOREVER);
  _pblock->GetValue(PB_PCAM_CLIP_FAR,ip->GetTime(),_clipFar,FOREVER);
  _pblock->GetValue(PB_PCAM_SHOW_SECTION,ip->GetTime(),(int&)_showSection,FOREVER);
//  _pblock->GetValue(PB_PCAM_HEIGHT,ip->GetTime(),_height,FOREVER);
//  _pblock->GetValue(PB_PCAM_WIDTH,ip->GetTime(),_width,FOREVER);
//  _pblock->GetValue(PB_PCAM_ASPECT,ip->GetTime(),_aspect,FOREVER);
  _pblock->GetValue(PB_PCAM_SHOW_VOLUME,ip->GetTime(),(int&)_showVolume,FOREVER);
  _pblock->GetValue(PB_PCAM_NEAR_ENV,ip->GetTime(),_nearEnv,FOREVER);
  _pblock->GetValue(PB_PCAM_FAR_ENV,ip->GetTime(),_farEnv,FOREVER);
  _pblock->GetValue(PB_PCAM_SHOW_ENV,ip->GetTime(),(int&)_showEnv,FOREVER);
}

void ParallelCamera::SetType(int tp)
{
  return;
}

int ParallelCamera::HitTest(TimeValue t,INode* inode,int type,int crossing,int flags,IPoint2 *p,ViewExp *vpt)
{
	HitRegion hitRegion;
	DWORD	savedLimits;
	int res;
	Matrix3 m;

	if(!_enable)
    return  0;

	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	gw->clearHitCode();
	
  res = _mesh.select(gw,gw->getMaterial(),&hitRegion,flags & HIT_ABORTONHIT); 

	if(!res)
  {
    gw->clearHitCode();
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);
		if(res = DrawWires(t,inode,gw))
			inode->SetTargetNodePair(1);
	}
	
  gw->setRndLimits(savedLimits);
	
  return res;
}

void ParallelCamera::Snap(TimeValue t,INode* inode,SnapInfo *snap,IPoint2 *p,ViewExp *vpt)
{ // stolen from simple camera
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority)
  {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Point2 fp = Point2((float)p->x,(float)p->y);
		IPoint3 screen3;
		Point2 screen2;
		Point3 vert(0.0f,0.0f,0.0f);

		gw->wTransPoint(&vert,&screen3);

		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2-fp);
		if(len <= snap->strength)
    {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority)
      {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert*tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			else // Closer than the best of this priority?
			if(len < snap->bestDist)
      {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert*tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
      }
    }
  }
}

void ParallelCamera::GetLocalBoundBox(TimeValue t,INode* inode,ViewExp* vpt,Box3& box)
{
	Matrix3 m = inode->GetObjectTM(t);
	Point3 pt;
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/(float)360.0;
	box = _mesh.getBoundingBox();
	box.Scale(scaleFactor);
	Point3 q[4];

  // the camera volume
  Interval valid;
  int showVolume;
  _pblock->GetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),showVolume,valid);
	if(showVolume || (_extDispFlags & EXT_DISP_ONLY_SELECTED))
  {
    ComputeRectPoints(t,0.0f,q);
    box.IncludePoints(q,4);
		ComputeRectPoints(t,max(GetClipDist(t,CAM_HITHER_CLIP),GetClipDist(t,CAM_YON_CLIP)),q);
		box.IncludePoints(q,4);
  }

  // the environment range
  int showEnv;
  _pblock->GetValue(PB_PCAM_SHOW_ENV,TimeValue(0),showEnv,valid);
	if(showEnv)
  {
		ComputeRectPoints(t,max(GetEnvRange(t,ENV_NEAR_RANGE),GetEnvRange(t,ENV_FAR_RANGE)),q);
		box.IncludePoints(q,4);
	}
}

void ParallelCamera::GetWorldBoundBox(TimeValue t,INode* inode,ViewExp* vpt,Box3& box)
{
	Matrix3 tm;
	Point3 q[4];
  
  box.Init();

  // the camera itself
  GetMat(t,inode,vpt,tm);
	int nv = _mesh.getNumVerts();

  if(!(_extDispFlags & EXT_DISP_ZOOM_EXT))
  {
    for(int i=0; i<nv; i++) 
			box += tm*_mesh.getVert(i);
  }
	else
		box += tm.GetTrans();

	tm = inode->GetObjectTM(t);

  // the camera volume
  Interval valid;
  int showVolume;
  _pblock->GetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),showVolume,valid);
	if(showVolume || (_extDispFlags & EXT_DISP_ONLY_SELECTED))
  {
    ComputeRectPoints(t,0.0f,q);
    box.IncludePoints(q,4,&tm);
		ComputeRectPoints(t,max(GetClipDist(t,CAM_HITHER_CLIP),GetClipDist(t,CAM_YON_CLIP)),q);
		box.IncludePoints(q,4,&tm);
  }

  // the environment range
  int showEnv;
  _pblock->GetValue(PB_PCAM_SHOW_ENV,TimeValue(0),showEnv,valid);
	if(showEnv)
  {
		ComputeRectPoints(t,max(GetEnvRange(t,ENV_NEAR_RANGE),GetEnvRange(t,ENV_FAR_RANGE)),q);
		box.IncludePoints(q,4,&tm);
	}
}

void ParallelCamera::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm)
{ // stolen from simple camera - used to keep constant sizes despite viewport zoom changes
	tm = inode->GetObjectTM(t);
	tm.NoScale();
	float scale = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	if(scale != 1.0f)
		tm.Scale(Point3(scale,scale,scale));
}

void ParallelCamera::MakeQuad(Face *f,int a,int b,int c,int d,int sg,int dv)
{ // stolen from simple camera
	f[0].setVerts(a+dv,b+dv,c+dv);
	f[0].setSmGroup(sg);
	f[0].setEdgeVisFlags(1,1,0);
	f[1].setVerts(c+dv,d+dv,a+dv);
	f[1].setSmGroup(sg);
	f[1].setEdgeVisFlags(1,1,0);
}

void ParallelCamera::BuildMesh(void)
{ // stolen from simple camera
	int nverts = 16;
	int nfaces = 24;
	_mesh.setNumVerts(nverts);
	_mesh.setNumFaces(nfaces);
	float len = (float)5.0;
	float w = (float)8.0;
	float d = w*(float).8;
	float e = d*(float).5;
	float f = d*(float).8;
	float l = w*(float).8;

	_mesh.setVert(0,Point3(-d,-d,-len));
	_mesh.setVert(1,Point3( d,-d,-len));
	_mesh.setVert(2,Point3(-d, d,-len));
	_mesh.setVert(3,Point3( d, d,-len));
	_mesh.setVert(4,Point3(-d,-d, len));
	_mesh.setVert(5,Point3( d,-d, len));
	_mesh.setVert(6,Point3(-d, d, len));
	_mesh.setVert(7,Point3( d, d, len));
	MakeQuad(&(_mesh.faces[ 0]),0,2,3,1, 1);
	MakeQuad(&(_mesh.faces[ 2]),2,0,4,6, 2);
	MakeQuad(&(_mesh.faces[ 4]),3,2,6,7, 4);
	MakeQuad(&(_mesh.faces[ 6]),1,3,7,5, 8);
	MakeQuad(&(_mesh.faces[ 8]),0,1,5,4,16);
	MakeQuad(&(_mesh.faces[10]),4,5,7,6,32);
	
	_mesh.setVert(8+0,Point3(-e,-e,len));
	_mesh.setVert(8+1,Point3( e,-e,len));
	_mesh.setVert(8+2,Point3(-e, e,len));
	_mesh.setVert(8+3,Point3( e, e,len));
	_mesh.setVert(8+4,Point3(-f,-f,len+l));
	_mesh.setVert(8+5,Point3( f,-f,len+l));
	_mesh.setVert(8+6,Point3(-f, f,len+l));
	_mesh.setVert(8+7,Point3( f, f,len+l));

	Face* fbase = &_mesh.faces[12];
	MakeQuad(&fbase[0], 0,2,3,1,  1,8);
	MakeQuad(&fbase[2], 2,0,4,6,  2,8);
	MakeQuad(&fbase[4], 3,2,6,7,  4,8);
	MakeQuad(&fbase[6], 1,3,7,5,  8,8);
	MakeQuad(&fbase[8], 0,1,5,4, 16,8);
	MakeQuad(&fbase[10],4,5,7,6, 32,8);

	// whoops- rotate 180 about x to get it facing the right way
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.RotateX(DegToRad(180.0));
	for (int i=0;i<nverts;i++)
		_mesh.getVert(i) = mat*_mesh.getVert(i);
	_mesh.buildNormals();
	_mesh.EnableEdgeList(1);
	}

void ParallelCamera::ComputeRectPoints(TimeValue t,float dist,Point3 *pts)
{
  // a cross section of the volume
  Interval valid;
  float w,h;
  _pblock->GetValue(PB_PCAM_WIDTH,t,w,valid);
  _pblock->GetValue(PB_PCAM_HEIGHT,t,h,valid);

  // halve since centred
  w *= 0.5f;
  h *= 0.5f;
  
  // quadrilateral
	pts[0] = Point3( w, h,-dist);				
	pts[1] = Point3(-w, h,-dist);				
	pts[2] = Point3(-w,-h,-dist);				
	pts[3] = Point3( w,-h,-dist);				
}

void ParallelCamera::DrawVolume(TimeValue t,GraphicsWindow *gw,float dist,int colid,bool drawSides,bool drawDiags)
{
	Point3 pts[4],tmp[2];

  ComputeRectPoints(t,dist,pts);

  // set the colour
  if(colid)
    gw->setColor(LINE_COLOR,GetUIColor(colid));

  // diagonals if desired
  if(drawDiags)
  {
    tmp[0] = pts[0];
    tmp[1] = pts[2];	
		gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);	
		tmp[0] = pts[1];
    tmp[1] = pts[3];	
		gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);	
	}

  // draw the quad
	gw->polyline(4,pts,NULL,NULL,TRUE,NULL);	

  // sides of volume (always camera colour)
  if(drawSides)
  {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_CAMERA_CONE));
		for(int i=0;i<4;i++)
    {
			tmp[0] = pts[i];	
			tmp[1] = pts[i];
      tmp[1].z = 0.0f;
			gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);	
    }
	}
}

int ParallelCamera::DrawWires(TimeValue t,INode *inode,GraphicsWindow *gw)
{
    Interval valid;
    int showVolume,showSection;
    _pblock->GetValue(PB_PCAM_SHOW_VOLUME,TimeValue(0),showVolume,valid);
    _pblock->GetValue(PB_PCAM_SHOW_SECTION,TimeValue(0),showSection,valid);

	gw->clearHitCode();

    // the camera volume
		if(showVolume || (_extDispFlags & EXT_DISP_ONLY_SELECTED))
    {
			DrawVolume(t,gw,0.0f,COLOR_CAMERA_CONE,false,true);
			DrawVolume(t,gw,GetClipDist(t,CAM_HITHER_CLIP),COLOR_CAMERA_CLIP,false,!bool(showSection&1));
			DrawVolume(t,gw,GetClipDist(t,CAM_YON_CLIP),COLOR_CAMERA_CLIP,true,true);
    }

    // the front clip section
		if(showSection)
		{
			//  Make sure the slice is up to date.
			UpdateSlice(inode,t,_update);
			_update = false;

			// draw the slice with the right color
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_CAMERA_CLIP));
			//			_shape.Render(gw,gw->getMaterial(),NULL,COMP_ALL);  //commented out since this stopped compiling. mdl 3/8/99
			//(flags&USE_DAMAGE_RECT)?&vpt->GetDammageRect():NULL,COMP_ALL|(inode->Selected()?COMP_OBJSELECTED:0));	
		}

    // the environment range
    int showEnv;
    _pblock->GetValue(PB_PCAM_SHOW_ENV,TimeValue(0),showEnv,valid);
    if(showEnv)
    {
      int cnear = 0;
      int cfar = 0;
      if(!inode->IsFrozen() && !inode->Dependent())
      { 
        cnear = COLOR_NEAR_RANGE;
        cfar = COLOR_FAR_RANGE;
      }
      DrawVolume(t,gw,GetEnvRange(t,ENV_NEAR_RANGE),cnear,false,false);
      DrawVolume(t,gw,GetEnvRange(t,ENV_FAR_RANGE),cfar,true,false);
    }

    return gw->checkHitCode();
}

int ParallelCamera::Display(TimeValue t,INode* inode,ViewExp *vpt,int flags)
{
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	if(_enable)
  {
    // a matrix that is viewport scale independent
    GetMat(t,inode,vpt,m);
    gw->setTransform(m);

    // save the current rendering settings, then set to wireframe
    DWORD rlim = gw->getRndLimits();
    gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);

    // set an appropriate line colour
    if(inode->Selected())
      gw->setColor(LINE_COLOR,GetSelColor());
    else
    {
      if(!inode->IsFrozen() && !inode->Dependent())
        gw->setColor(LINE_COLOR,GetUIColor(COLOR_CAMERA_OBJ));
    }

    // draw the camera itself
    _mesh.render(gw,gw->getMaterial(),NULL,COMP_ALL);	

    // a matrix that scales with the viewport
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);

    DrawWires(t,inode,gw);

    // restore the rendering settings saved previously
	  gw->setRndLimits(rlim);
  }
  return 0;
}

CreateMouseCallBack* ParallelCamera::GetCreateMouseCallBack() 
{
  ParallelCameraCreateCallBack::Instance()->SetCam(this);
	return(ParallelCameraCreateCallBack::Instance());
}

void ParallelCamera::GetDeformBBox(TimeValue t,Box3& box,Matrix3 *tm,BOOL useSel)
{
//	box = mesh.getBoundingBox(tm);
}

ObjectState ParallelCamera::Eval(TimeValue time)
{
	return ObjectState(this);
}

RefResult ParallelCamera::EvalCameraState(TimeValue t,Interval& valid,CameraState* cs)
{
	cs->isOrtho = true;	
	cs->fov = GetFOV(t,valid);
	cs->tdist = GetTDist(t,valid);
	cs->horzLine = false;
	cs->manualClip = true;
	cs->hither = GetClipDist(t,CAM_HITHER_CLIP,valid);
	cs->yon = GetClipDist(t,CAM_YON_CLIP,valid);
	cs->nearRange = GetEnvRange(t,ENV_NEAR_RANGE,valid);
	cs->farRange = GetEnvRange(t,ENV_FAR_RANGE,valid);
  return REF_SUCCEED;
}

IOResult ParallelCamera::Load(ILoad *iload) 
{
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the file being loaded.
  iload->RegisterPostLoadCallback(new ParamBlockPLCB(_versions,PCAM_OLDVERSIONS,&_curVersion,this,0));
  
	return IO_OK;
}

IOResult ParallelCamera::Save(ISave *isave)
{
 	return IO_OK;
}

RefTargetHandle ParallelCamera::Clone(RemapDir& remap)
{
	ParallelCamera* newob = new ParallelCamera();
	newob->ReplaceReference(0,_pblock->Clone(remap));	
	newob->_enable = _enable;
	BaseClone(this, newob, remap);
	return(newob);
}

RefResult ParallelCamera::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID,RefMessage message) 
{
  switch(message)
  {		
	case REFMSG_GET_PARAM_DIM:
    {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch(gpd->index)
			{
			case PB_PCAM_AXIS_ANGLE_X:
			case PB_PCAM_AXIS_ANGLE_Y:
			case PB_PCAM_AXIS_ANGLE_Z:
				gpd->dim = stdAngleDim;
				break;				
			case PB_PCAM_CLIP_NEAR:
			case PB_PCAM_CLIP_FAR:
			case PB_PCAM_HEIGHT:
			case PB_PCAM_WIDTH:
			case PB_PCAM_ASPECT:
			case PB_PCAM_NEAR_ENV:
			case PB_PCAM_FAR_ENV:
				gpd->dim = stdWorldDim;
				break;				
			}
			return REF_STOP; 
		}
		    
    case REFMSG_GET_PARAM_NAME:
      {
        GetParamName *gpn = (GetParamName*)partID;
        switch(gpn->index)
        {
        case PB_PCAM_AXIS_ANGLE_X:
          gpn->name = TSTR(GetString(IDS_PCAM_AXIS_ANGLE_X));
          break;
        case PB_PCAM_AXIS_ANGLE_Y:
          gpn->name = TSTR(GetString(IDS_PCAM_AXIS_ANGLE_Y));
          break;
        case PB_PCAM_AXIS_ANGLE_Z:
          gpn->name = TSTR(GetString(IDS_PCAM_AXIS_ANGLE_Z));
          break;
        case PB_PCAM_HEIGHT:
          gpn->name = TSTR(GetString(IDS_PCAM_HEIGHT));
          break;
        case PB_PCAM_WIDTH:
          gpn->name = TSTR(GetString(IDS_PCAM_WIDTH));
          break;
        case PB_PCAM_ASPECT:
          gpn->name = TSTR(GetString(IDS_PCAM_ASPECT));
          break;
        case PB_PCAM_CLIP_NEAR:
          gpn->name = TSTR(GetString(IDS_RB_NEARPLANE));
          break;												
        case PB_PCAM_CLIP_FAR:
          gpn->name = TSTR(GetString(IDS_RB_FARPLANE));
          break;												
        case PB_PCAM_NEAR_ENV:
          gpn->name = TSTR(GetString(IDS_DB_NRANGE));
          break;												
        case PB_PCAM_FAR_ENV:
          gpn->name = TSTR(GetString(IDS_DB_FRANGE));
          break;												
        }
        return REF_STOP; 
      }
		}
  return(REF_SUCCEED);
}

static INT_PTR CALLBACK StandardPanelDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  ICustButton *btn = NULL;
	Matrix3 tm(true);
	bool buttonMsg;
	ParallelCamera *pCam = NULL;

  switch(msg)
  {
  case WM_INITDIALOG:
    // create button tops
    btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_UPDATE));
    if(btn)
    {
      btn->SetTooltip(TRUE,GetString(IDS_SECTION_UPDATE));
      ReleaseICustButton(btn);
    }
		pCam = (ParallelCamera*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)pCam);
		break;
  case WM_COMMAND:
		buttonMsg = true;
    switch(LOWORD(wParam))
    {
    case IDC_PCAM_PLAN:
			tm.SetRow(0,Point3(1.0f,0.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,1.0f,0.0f));
			tm.SetRow(2,Point3(0.0f,0.0f,1.0f));
			break;
    case IDC_PCAM_BOTTOM:
			tm.SetRow(0,Point3(1.0f,0.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,-1.0f,0.0f));
			tm.SetRow(2,Point3(0.0f,0.0f,-1.0f));
      break;
    case IDC_PCAM_LEFT:
			tm.SetRow(0,Point3(0.0f,-1.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,0.0f,1.0f));
			tm.SetRow(2,Point3(-1.0f,0.0f,0.0f));
      break;
    case IDC_PCAM_RIGHT:
			tm.SetRow(0,Point3(0.0f,1.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,0.0f,1.0f));
			tm.SetRow(2,Point3(1.0f,0.0f,0.0f));
      break;
    case IDC_PCAM_FRONT:
			tm.SetRow(0,Point3(1.0f,0.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,0.0f,1.0f));
			tm.SetRow(2,Point3(0.0f,-1.0f,0.0f));
      break;
    case IDC_PCAM_BACK:
			tm.SetRow(0,Point3(-1.0f,0.0f,0.0f));
			tm.SetRow(1,Point3(0.0f,0.0f,1.0f));
			tm.SetRow(2,Point3(0.0f,1.0f,0.0f));
      break;
    case IDC_PCAM_QUADRANT1:
			tm.SetRow(0,Normalize(Point3(1.0f,-1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(1.0f,1.0f,1.0f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    case IDC_PCAM_QUADRANT2:
			tm.SetRow(0,Normalize(Point3(-1.0f,-1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(-1.0f,1.0f,1.0f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    case IDC_PCAM_QUADRANT3:
			tm.SetRow(0,Normalize(Point3(1.0f,-1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(-1.0f,-1.0f,1.0f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    case IDC_PCAM_QUADRANT4:
			tm.SetRow(0,Normalize(Point3(1.0f,1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(1.0f,-1.0f,1.0f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    case IDC_PCAM_DIMETRIC15:
			tm.SetRow(0,Normalize(Point3(1.0f,1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(1.0f,-1.0f,0.5f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    case IDC_PCAM_DIMETRIC45:
			tm.SetRow(0,Normalize(Point3(1.0f,1.0f,0.0f)));
			tm.SetRow(2,Normalize(Point3(1.0f,-1.0f,2.0f)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
      break;
    default:
			buttonMsg = false;
      break;
    }
		if(buttonMsg)
		{
			pCam = (ParallelCamera*)GetWindowLongPtr(hWnd,GWLP_USERDATA); // retrieve current
			TimeValue t = pCam->_ip?pCam->_ip->GetTime():GetCOREInterface()->GetTime();
			Box3 box = (pCam->_selBox)*Inverse(tm);

			float width = box.pmax.x-box.pmin.x,height = box.pmax.y-box.pmin.y;
			pCam->_pblock->SetValue(PB_PCAM_HEIGHT,t,height);
			pCam->_pblock->SetValue(PB_PCAM_WIDTH,t,width);
			pCam->_pblock->SetValue(PB_PCAM_ASPECT,t,width/height);

			tm.SetTrans(box.Center()*tm+(tm.GetRow(2)*(box.pmax.z-box.pmin.z)));
			pCam->_ip->NonMouseCreate(tm);
			pCam->_suspendSnap = FALSE;
			return TRUE;
		}			
	default:
    break;
  }
  return TRUE;
}

void PcamCustomPanel::DisableEdit(const int item) const
{
  ICustEdit *edit = GetICustEdit(GetDlgItem(_hPanel,item));
  if(edit)
  {
    edit->Disable();
    ReleaseICustEdit(edit);
  }
}

void PcamCustomPanel::EnableEdit(const int item) const
{
  ICustEdit *edit = GetICustEdit(GetDlgItem(_hPanel,item));
  if(edit)
  {
    edit->Enable();
    ReleaseICustEdit(edit);
  }
}

void PcamCustomPanel::DisableSpinner(const int item) const
{
  ISpinnerControl *spin = GetISpinner(GetDlgItem(_hPanel,item));
  if(spin)
  {
    spin->Disable();
    ReleaseISpinner(spin);
  }
}

void PcamCustomPanel::EnableSpinner(const int item) const
{
  ISpinnerControl *spin = GetISpinner(GetDlgItem(_hPanel,item));
  if(spin)
  {
    spin->Enable();
    ReleaseISpinner(spin);
  }
}

BOOL PcamCustomPanel::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  ICustButton *btn = NULL;
  HIMAGELIST hList;
  HBITMAP	hBMP;

  switch(msg)
  {
  case WM_INITDIALOG:
		// create button tops
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_LOCK_X2Z));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_LOCK_SEPERATED));
      hList = ImageList_Create(16,32,ILC_COLORDDB,2,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,16,32);
      btn->SetTooltip(TRUE,GetString(IDS_LOCK_X2Z));
      ReleaseICustButton(btn);
    }
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_LOCK_X2Y));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_LOCK_ADJACENT));
      hList = ImageList_Create(16,15,ILC_COLORDDB,2,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,16,15);
      btn->SetTooltip(TRUE,GetString(IDS_LOCK_X2Y));
      ReleaseICustButton(btn);
    }
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_LOCK_Y2Z));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_LOCK_ADJACENT));
      hList = ImageList_Create(16,15,ILC_COLORDDB,2,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,16,15);
      btn->SetTooltip(TRUE,GetString(IDS_LOCK_Y2Z));
      ReleaseICustButton(btn);
    }
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_HEIGHT));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PUSHPIN));
      hList = ImageList_Create(12,11,ILC_COLORDDB,4,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,12,12);
      btn->SetTooltip(TRUE,GetString(IDS_PCAM_FIX_HEIGHT));
			btn->SetType(CBT_CHECK);
      ReleaseICustButton(btn);
    }
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_WIDTH));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PUSHPIN));
      hList = ImageList_Create(12,11,ILC_COLORDDB,4,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,12,12);
      btn->SetTooltip(TRUE,GetString(IDS_PCAM_FIX_WIDTH));
			btn->SetType(CBT_CHECK);
      ReleaseICustButton(btn);
    }
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_ASPECT));
		if(btn)
    {
      hBMP = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PUSHPIN));
      hList = ImageList_Create(12,11,ILC_COLORDDB,4,0);
      ImageList_Add(hList,hBMP,NULL);
      DeleteObject(hBMP);
      btn->SetImage(hList,0,1,0,0,12,12);
      btn->SetTooltip(TRUE,GetString(IDS_PCAM_FIX_ASPECT));
			btn->SetType(CBT_CHECK);
			btn->SetCheck(TRUE); // initially aspect is fixed
      ReleaseICustButton(btn);
    }
		_hPanel = hWnd;

		// initially aspect is fixed
		DisableEdit(IDC_PCAM_ASPECT);
		DisableSpinner(IDC_PCAM_ASPECT_SPINNER);
    break;
	case CC_SPINNER_CHANGE: 
		{
			ISpinnerControl *widthSpin = GetISpinner(GetDlgItem(_hPanel,IDC_PCAM_WIDTH_SPINNER));
			ISpinnerControl *heightSpin = GetISpinner(GetDlgItem(_hPanel,IDC_PCAM_HEIGHT_SPINNER));
			ISpinnerControl *aspectSpin = GetISpinner(GetDlgItem(_hPanel,IDC_PCAM_ASPECT_SPINNER));
			switch(LOWORD(wParam))
			{
			case IDC_PCAM_WIDTH_SPINNER:
				if(_pCam->_fixed == ParallelCamera::FIX_ASPECT)
				{
					float height = widthSpin->GetFVal()/aspectSpin->GetFVal();
					heightSpin->SetValue(height,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_HEIGHT,t,height);
				}
				else // _fixed == FIX_HEIGHT
				{
					float aspect = widthSpin->GetFVal()/heightSpin->GetFVal();
					aspectSpin->SetValue(aspect,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_ASPECT,t,aspect);
				}
				break;
			case IDC_PCAM_HEIGHT_SPINNER:
				if(_pCam->_fixed == ParallelCamera::FIX_ASPECT)
				{
					float width = heightSpin->GetFVal()*aspectSpin->GetFVal();
					widthSpin->SetValue(width,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_WIDTH,t,width);
				}
				else // _fixed == FIX_WIDTH
				{
					float aspect = widthSpin->GetFVal()/heightSpin->GetFVal();
					aspectSpin->SetValue(aspect,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_ASPECT,t,aspect);
				}
				break;
			case IDC_PCAM_ASPECT_SPINNER:
				if(_pCam->_fixed == ParallelCamera::FIX_WIDTH)
				{
					float height = widthSpin->GetFVal()/aspectSpin->GetFVal();
					heightSpin->SetValue(height,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_HEIGHT,t,height);
				}
				else // _fixed == FIX_HEIGHT
				{
					float width = heightSpin->GetFVal()*aspectSpin->GetFVal();
					widthSpin->SetValue(width,FALSE);
					_pCam->_pblock->SetValue(PB_PCAM_WIDTH,t,width);
				}
				break;
			}
		}
	case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_PCAM_UPDATE:
			_pCam->_update = true;
			_pCam->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
      break;
    case IDC_PCAM_LOCK_X2Z:
      break;
    case IDC_PCAM_LOCK_X2Y:
      break;
    case IDC_PCAM_LOCK_Y2Z:
      break;
    case IDC_PCAM_FIX_HEIGHT:
			if(_pCam->_fixed != ParallelCamera::FIX_HEIGHT)
			{
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_ASPECT));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_WIDTH));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				_pCam->_fixed = ParallelCamera::FIX_HEIGHT;
				EnableEdit(IDC_PCAM_ASPECT);
				EnableSpinner(IDC_PCAM_ASPECT_SPINNER);
				EnableEdit(IDC_PCAM_WIDTH);
				EnableSpinner(IDC_PCAM_WIDTH_SPINNER);
				DisableEdit(IDC_PCAM_HEIGHT);
				DisableSpinner(IDC_PCAM_HEIGHT_SPINNER);
			}
      break;
    case IDC_PCAM_FIX_WIDTH:
			if(_pCam->_fixed != ParallelCamera::FIX_WIDTH)
			{
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_ASPECT));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_HEIGHT));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				_pCam->_fixed = ParallelCamera::FIX_WIDTH;
				EnableEdit(IDC_PCAM_ASPECT);
				EnableSpinner(IDC_PCAM_ASPECT_SPINNER);
				DisableEdit(IDC_PCAM_WIDTH);
				DisableSpinner(IDC_PCAM_WIDTH_SPINNER);
				EnableEdit(IDC_PCAM_HEIGHT);
				EnableSpinner(IDC_PCAM_HEIGHT_SPINNER);
			}
      break;
    case IDC_PCAM_FIX_ASPECT:
			if(_pCam->_fixed != ParallelCamera::FIX_ASPECT)
			{
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_WIDTH));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				btn = GetICustButton(GetDlgItem(hWnd,IDC_PCAM_FIX_HEIGHT));
				if(btn)
				{
					btn->SetCheck(FALSE);
					ReleaseICustButton(btn);
				}
				_pCam->_fixed = ParallelCamera::FIX_ASPECT;
				DisableEdit(IDC_PCAM_ASPECT);
				DisableSpinner(IDC_PCAM_ASPECT_SPINNER);
				EnableEdit(IDC_PCAM_WIDTH);
				EnableSpinner(IDC_PCAM_WIDTH_SPINNER);
				EnableEdit(IDC_PCAM_HEIGHT);
				EnableSpinner(IDC_PCAM_HEIGHT_SPINNER);
			}
      break;
    default:
      break;
    }
  default:
    break;
  }
  return TRUE;
}

// sectioning front clip plane below here
void ParallelCamera::UpdateSlice(INode *sectionNode,const TimeValue t,const bool force)
{
	static float lastLength = 0.0f,lastWidth = 0.0f,lastDist = 0.0f;
	static Matrix3 sliceTM; // the slice's tm

	if(!force) // do we need to update anyway
	{
		// update if relevant parameters have changed since last update
		float length,width,dist;
		_pblock->GetValue(PB_PCAM_HEIGHT,t,length,FOREVER);
		_pblock->GetValue(PB_PCAM_WIDTH,t,width,FOREVER);
		_pblock->GetValue(PB_PCAM_CLIP_NEAR,t,dist,FOREVER);
		
		Matrix3 curTM = sectionNode->GetNodeTM(t);
		if(EqualMatrices(curTM,sliceTM) && length == lastLength && width == lastWidth && dist == lastDist)
		{
			sliceTM = curTM;
			return;
		}
	}

	assert(_pointList.Count()==0);

	sliceTM	= sectionNode->GetNodeTM(t);
	_pblock->GetValue(PB_PCAM_CLIP_NEAR,t,lastDist,FOREVER);

	// allow for clip distance in sectioning plane
	Matrix3 shift(sliceTM);
	shift.PreTranslate(Point3(0.0f,0.0f,-lastDist));
	_sliceTM_inv = Inverse(shift);
	_curTime = t;

	// to slice only nodes within the section rectangle
	_pblock->GetValue(PB_PCAM_HEIGHT,t,lastLength,FOREVER);
	_pblock->GetValue(PB_PCAM_WIDTH,t,lastWidth,FOREVER);

	float w2 = lastWidth*0.5f;
	float h2 = lastLength*0.5f;

	// include the bounding rectangle
	Box3 boundBox;
	boundBox += Point3(-w2,-h2,0.0f);
	boundBox += Point3( w2,-h2,0.0f);
	boundBox += Point3(-w2, h2,0.0f);
	boundBox += Point3( w2, h2,0.0f);

	boundBox = boundBox*shift;

	INode *rootNode = GetCOREInterface()->GetRootNode();
	assert(rootNode);

	// generate the point list
  TraverseWorld(rootNode,0,boundBox);

	_shape.NewShape();
	_shape.bdgBox.Init();
	// largest possible shape uses every point pair
	_shape.SetNumLines(_pointList.Count()/2,FALSE);

	PolyLine pl;
	pl.SetNumPts(2);

	for(int lines = 0,vert = 0;vert< _pointList.Count()/2;++vert)
	{
		// traverse the list of points pairwise
		pl.pts[0] = _pointList[vert*2];
		pl.pts[1] = _pointList[vert*2+1];

		// shift them 
		pl.pts[0].p.z = -lastDist;
		pl.pts[1].p.z = -lastDist;

		// if adjacent vertices are approx. distinct make a polyline of them
		if(!EqualPoint3(pl.pts[0].p,pl.pts[1].p))
    {
			_shape.bdgBox += pl.pts[0].p;
			_shape.bdgBox += pl.pts[1].p;
			_shape.lines[lines++] = pl; // and add it to the shape
		} // otherwise point pair is discarded
	}

	_shape.SetNumLines(lines,TRUE);
	_shape.UpdateSels();
	_shape.InvalidateGeomCache(FALSE);
	_shape.BuildBoundingBox();

	_pointList.Resize(0); // blows away the memory associated with the point list
}

// slice this node and recurse thru it's children
void ParallelCamera::TraverseWorld(INode *node,const int level,Box3& boundBox)
{
	Object *obj = node->EvalWorldState(_curTime).obj;
	if(obj)
  {
		Box3 nodeBBox;
		obj->GetDeformBBox(_curTime,nodeBBox,&node->GetObjectTM(_curTime));
		if(boundBox.Contains(nodeBBox))
		{
			_curNode = node;
			SliceASceneNode();
		}
	}

	// TraverseWorld other nodes
	const int num = node->NumberOfChildren();
	for(int i = 0;i<num;++i)
	{
		INode* child = node->GetChildNode(i);
		TraverseWorld(child,level+1,boundBox);
	}
}

// slice this node
void ParallelCamera::SliceASceneNode(void)
{
	// Get the object from the node
	ObjectState os = _curNode->EvalWorldState(_curTime);
	if(!_curNode->IsHidden() && 
		 !_curNode->IsFrozen() && 
		 os.obj &&
     os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) 
	{
		Box3 bbox;
		const float LOCAL_MAX_FLOAT = 1e30F;
		float	minz = LOCAL_MAX_FLOAT,maxz = -LOCAL_MAX_FLOAT;
		
		GeomObject* geo_obj = (GeomObject*)os.obj; // the current object being sliced
		ViewExp* view = GetCOREInterface()->GetActiveViewport();
		geo_obj->GetWorldBoundBox(_curTime,_curNode,view,bbox);
		GetCOREInterface()->ReleaseViewport(view);

		// transform the corners of the object's world bbox in slice space
		// store into _t_bbox and get min, max z
		for(int corner = 0;corner<8;corner++)
		{
			_tmBbox[corner] = _sliceTM_inv * bbox[corner];
			if(_tmBbox[corner].z < minz)
				minz = _tmBbox[corner].z;
			if(_tmBbox[corner].z > maxz)
				maxz = _tmBbox[corner].z;
		}

		// see if we touch or intersect the bounding box
		int sminz = Sign(minz);
		int smaxz = Sign(maxz);
		if(sminz == 0 || smaxz == 0 || sminz != smaxz)
		{
      bool needDelete;
			TriObject* tri = GetTriObjectFromNode(_curNode,_curTime,needDelete);
			if(tri)
      {
				_curMesh = &tri->GetMesh();
				if(_curMesh) 
					DoMeshSlice();

				if(needDelete)
					delete tri;
			}
		}
	}
}

// slice the current mesh 
void ParallelCamera::DoMeshSlice(void)
{
	const Matrix3 objToWorldTM = _curNode->GetObjTMAfterWSM(_curTime);
	const Matrix3 objToSliceTM = objToWorldTM * _sliceTM_inv;

	// make and fill an array of mesh points in slice coordinate system
	// also compute the sign's of all the z coords
	// TODO: move this memory allocation outside object traversal
	Point3* t_verts  = new Point3[_curMesh->numVerts];
	assert(t_verts);

  // transform the vertices of the mesh to slice space
	for(int vert = 0;vert<_curMesh->numVerts;vert++)
		t_verts[vert] = objToSliceTM*_curMesh->getVert(vert);

	// slice each face
	for(int face = 0;face<_curMesh->numFaces;face++)
  {
		if(!(_curMesh->faces[face].flags & FACE_HIDDEN))
			SliceFace(t_verts,face);
  }

	delete[] t_verts;
}

//-------------------------------------------------------------------
// slice the cutting plane thru a mesh face
//
// if the face is on the slice, add all the edges to the slice edge list
// if one edge is on the slice, add that edge
// if one vertex is on the slice, see if the opposite edge is sliced
//     if so, add that segment
// if no vertices are on the slice, find the two points of intersection and
//		add the segment between those two points
// 
// add the resulting line segment(s) to the section object's _shape list 
//
void ParallelCamera::SliceFace(Point3* t_verts, const DWORD f_index)
{
	Face*		face;						// the face being intersected
	Point3*	vv[3];						// pointers to vertices of each face
	int			nu_coplanar = 0;
	int			cnt,check;
	int			nonz_verts[2];
	int			sign_vvz[3];
	Point3	crossPt;
	int			nu_neg=0,nu_pos=0;
	face	= &_curMesh->faces[f_index];

	// find number of coplanar vertices 
	// store sign of vertices z values in sign_vvz
	for(int jj = 0;jj<3;++jj)
	{
		vv[jj] = &t_verts[face->v[jj]];
		sign_vvz[jj] = Sign(vv[jj]->z);
		if(sign_vvz[jj] == 0)
			nu_coplanar++;
		else if(sign_vvz[jj] < 0)
			nu_neg++;
		else
			nu_pos++;
	}

	// if all on one side or other, return
	if(nu_neg==3 || nu_pos==3)
		return;

	// decide which segment to add based on how many vertices are coplanar to the slice
	switch (nu_coplanar)
	{
	case 3:
		// the whole face is on the slice
		// add all edges to the segment list (as long as they're visible)
		cnt=0;
		if(face->getEdgeVis(0))
		{
			_faceLines[cnt].pts[0] = *vv[0];
			_faceLines[cnt].pts[1] = *vv[1];
			cnt++;
		}
		if(face->getEdgeVis(1))
		{
			_faceLines[cnt].pts[0] = *vv[1];
			_faceLines[cnt].pts[1] = *vv[2];
			cnt++;
		}
		if(face->getEdgeVis(2))
		{
			_faceLines[cnt].pts[0] = *vv[2];
			_faceLines[cnt].pts[1] = *vv[0];
			cnt++;
		}

		for(jj=0;jj<cnt;jj++)
		{
			_pointList.Append(1,&_faceLines[jj].pts[0].p,256);
			_pointList.Append(1,&_faceLines[jj].pts[1].p,256);
		}
		break;

	case 2:
		// one edge is on the slice
		// add the edge who's z values are zero (as long as it's visible)
		cnt = 0;
		if(sign_vvz[0]==0)
			_faceLines[0].pts[cnt++] = *vv[0];
		else if(!face->getEdgeVis(1))
				break;
		if(sign_vvz[1]==0)
			_faceLines[0].pts[cnt++] = *vv[1];
		else if(!face->getEdgeVis(0))
				break;
		if(sign_vvz[2]==0)
			_faceLines[0].pts[cnt++] = *vv[2];
		else if(!face->getEdgeVis(2))
				break;

		assert(cnt==2);

		_pointList.Append(1,&_faceLines[0].pts[0].p,256);
		_pointList.Append(1,&_faceLines[0].pts[1].p,256);
    break;

	case 1:
		// one vertex is on the slice
		// if the signs of the other vertices are not different,
		//      have a single point intersection and ignore it
		// otherwise, 
		//      compute the intersection point between the vertices
		//      add line between intersection point and point on slice
		cnt=0;
		if(sign_vvz[0]!=0)
			nonz_verts[cnt++] = 0;
		else
			_faceLines[0].pts[0] = *vv[0];
		if(sign_vvz[1]!=0)
			nonz_verts[cnt++] = 1;
		else
			_faceLines[0].pts[0] = *vv[1];
		if(sign_vvz[2]!=0)
			nonz_verts[cnt++] = 2;
		else
			_faceLines[0].pts[0] = *vv[2];

		assert(cnt==2);
		assert(sign_vvz[nonz_verts[0]]!=0 && sign_vvz[nonz_verts[1]]!=0);

		if(sign_vvz[nonz_verts[0]] == sign_vvz[nonz_verts[1]])
			break;

		CrossPoint(*vv[nonz_verts[0]],*vv[nonz_verts[1]],&crossPt);

		_faceLines[0].pts[1] = PolyPt(crossPt);

		_pointList.Append(1,&_faceLines[0].pts[0].p,256);
		_pointList.Append(1,&_faceLines[0].pts[1].p,256);
		break;

	case 0:
		// no vertices are on the plane
		// look for zero or two intersection points
		cnt = 0;

		check = CrossCheck(*vv[0],*vv[1],&crossPt);
		assert(check != 2);
		if(check==1)
			_faceLines[0].pts[cnt++] = PolyPt(crossPt);

		check = CrossCheck(*vv[0],*vv[2],&crossPt);
		assert(check != 2);
		if(check==1)
			_faceLines[0].pts[cnt++] = PolyPt(crossPt);

		check = CrossCheck(*vv[1],*vv[2],&crossPt);
		assert(check != 2);
		if(check==1)
			_faceLines[0].pts[cnt++] = PolyPt(crossPt);

		assert(cnt==0 || cnt == 2);

		if(cnt==2)
		{
			_pointList.Append(1,&_faceLines[0].pts[0].p,256);
			_pointList.Append(1,&_faceLines[0].pts[1].p,256);

		}
		break;

	default:
		assert(FALSE);
	}
}

// return a pointer to a TriObject given an INode or NULL if inconvertable
TriObject *ParallelCamera::GetTriObjectFromNode(INode *node,const TimeValue t,bool &deleteIt)
{
	deleteIt = false;
	Object *obj = node->EvalWorldState(t).obj;
	if(obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0)))
  { 
		TriObject *tri = (TriObject *) obj->ConvertToType(0,Class_ID(TRIOBJ_CLASS_ID,0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if(obj != tri)
      deleteIt = true;
		return tri;
	}
	else
		return NULL;
}

// Not truly the correct way to compare floats of arbitary magnitude...
bool ParallelCamera::EqualPoint3(const Point3 p1,const Point3 p2)
{
	const float FUZZ_FACTOR  = 1.0e-6f;

	if(fabs(p1.x - p2.x) > FUZZ_FACTOR)
		return false;
	if(fabs(p1.y - p2.y) > FUZZ_FACTOR)
		return false;
	if(fabs(p1.z - p2.z) > FUZZ_FACTOR)
		return false;

	return true;
}

// checks that the two points cross the XY plane.  
// If so, computes the crossing point.
// 
// return values:
//		2  both points are on the plane (cross_pt is not changed)
//		1  cross_pt contains the one point of intersection
//		0  no points of intersection (cross_pt not changed)
int ParallelCamera::CrossCheck(const Point3& pt1,const Point3& pt2,Point3* cross_pt)
{
	const float	z1 = pt1.z;
	const float	z2 = pt2.z;
	const int	sign_z1 = Sign(z1);
	const int	sign_z2 = Sign(z2);

	// check for corners straddling the plane
	if(sign_z1 == 0 && sign_z2 == 0)
		return 2;
	else if(sign_z1 != sign_z2)
	{
		// straddle!  compute intersection pt
		if(cross_pt)
		{
			float dd = z1 / (z1 - z2);
			cross_pt->x = pt1.x + dd * (pt2.x - pt1.x);
			cross_pt->y = pt1.y + dd * (pt2.y - pt1.y);
			cross_pt->z = 0.0f;
		}
		return 1;
	}
	return 0;
}

// computes crossing point when we know that the two points cross the XY plane
void ParallelCamera::CrossPoint(const Point3& pt1,const Point3& pt2,Point3* cross_pt)
{
	const float	z1 = pt1.z;
	const float	z2 = pt2.z;
	const int	sign_z1 = Sign(z1);
	const int	sign_z2 = Sign(z2);

	assert(Sign(pt1.z) != Sign(pt2.z));

	float dd = z1 / (z1 - z2);
	cross_pt->x = pt1.x + dd * (pt2.x - pt1.x);
	cross_pt->y = pt1.y + dd * (pt2.y - pt1.y);
	cross_pt->z = 0.0f;
}

bool ParallelCamera::EqualMatrices(const Matrix3& m1,const Matrix3& m2) 
{
	return m1.GetRow(0)==m2.GetRow(0) && 
		     m1.GetRow(1)==m2.GetRow(1) && 
		     m1.GetRow(2)==m2.GetRow(2) && 
		     m1.GetRow(3)==m2.GetRow(3);
}
#endif // DESIGN_VER
