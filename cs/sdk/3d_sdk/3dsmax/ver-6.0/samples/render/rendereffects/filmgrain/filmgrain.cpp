//-----------------------------------------------------------------------------
// ------------------------
// File ....: FilmGrain.cpp
// ------------------------
// Author...: Gus J Grubba
// Date ....: October 1998
//
// Implementation of Film Grain Render Effect
//
//-----------------------------------------------------------------------------
      
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <iparamm2.h>
#include "FilmGrain.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst	= NULL;
static int controlsInit = FALSE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	
	hInst = hDLLInst;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		InitCustomControls(hInst);
		InitCommonControls();
	}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return(TRUE);
}

//-----------------------------------------------------------------------------
// *> GetString()
//

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
// Class Description

class FGClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic	()				{ return 1; }
	void*			Create		(BOOL loading)	{ return new FilmGrain; }
	const TCHAR*	ClassName	()				{ return GetString(IDS_FILM_GRAIN_EFFECT); }
	SClass_ID		SuperClassID()				{ return RENDER_EFFECT_CLASS_ID; }
	Class_ID        ClassID		()				{ return fgClassID; }
	const TCHAR* 	Category	()				{ return _T("");  }
	const TCHAR*	InternalName()				{ return _T("FilmGrain");}
	HINSTANCE		HInstance	()				{ return hInst; }
};

static FGClassDesc FGDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR *LibDescription() { 
	return GetString(IDS_LIBDESCR);
}

DLLEXPORT int LibNumberClasses() {
	return 1;
}

DLLEXPORT ClassDesc* LibClassDesc(int i) {
	switch(i) {
		case 0:		return &FGDesc;
		default:	return 0;
	}
}

DLLEXPORT ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}

//-----------------------------------------------------------------------------
// Parameters

#define PBLOCK_REF 0

enum { fg_params };

//-- fg_params param IDs

enum { fg_grain, fg_mask /*, fg_mono*/ };

static ParamBlockDesc2 fg_param_blk ( fg_params, _T("Film Grain Parameters"),0,&FGDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//-- Rollout
	IDD_SAMPLE_EFFECT, IDS_FG_PARAMETERS, 0, 0, NULL, 
	// params
	fg_grain, _T("Grain"), TYPE_FLOAT, P_ANIMATABLE, IDS_GRAIN,	
		p_default,		0.2,
		p_range,		0.0, 10.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAIN, IDC_GRAIN_SPIN, SPIN_AUTOSCALE, 
		end,
	fg_mask, _T("Mask_Background"), TYPE_BOOL, P_ANIMATABLE, IDS_MASK,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_MASK,
		end,
//	fg_mono, _T("Monochrome"), TYPE_BOOL, P_ANIMATABLE, IDS_MONOCHROME,
//		p_default, FALSE,
//		p_ui, TYPE_SINGLECHEKBOX, IDC_MONOCHROME,
//		end,
	end
	);

FilmGrain::FilmGrain() {
	FGDesc.MakeAutoParamBlocks(this);
	assert(pblock);
}

IOResult FilmGrain::Load(ILoad *iload) {
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *FilmGrain::CreateParamDialog(IRendParams *ip) {	
	return FGDesc.CreateParamDialogs(ip, this);
}

Animatable* FilmGrain::SubAnim(int i) {
	switch (i) {
		case 0:		return pblock;
		default:	return NULL;
	}
}

TSTR FilmGrain::SubAnimName(int i) {
	switch (i) {
		case 0:		return GetString(IDS_PARAMETERS);
		default:	return _T("");
	}
}

RefTargetHandle FilmGrain::GetReference(int i) {
	switch (i) {
		case 0:		return pblock;
		default:	return NULL;
	}
}

void FilmGrain::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
		case 0: pblock	= (IParamBlock2*)rtarg; break;
	}
}

RefResult FilmGrain::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,PartID& partID,  RefMessage message) {
	switch (message) {
		case REFMSG_CHANGE:
			fg_param_blk.InvalidateUI();
			break;
	}
	return REF_SUCCEED;
}

void FilmGrain::Update(TimeValue t, Interval& valid)	{
}

int FilmGrain::RenderBegin(TimeValue t, ULONG flags)	{
	return 0;
}

int FilmGrain::RenderEnd(TimeValue t) {
	return 0;
}

static BOOL WINAPI fxCallBack(LPVOID lpparam, int /*done*/, int /*total*/, TCHAR* /*msg*/) {
	CheckAbortCallback*  checkAbort = (CheckAbortCallback*)lpparam;
	if (checkAbort && checkAbort->Check()) 
		return FALSE;
	return TRUE;
}
		
void FilmGrain::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc,CheckAbortCallback *checkAbort) {
	Interval valid;		
	float	grain;
	BOOL	mask;
//	BOOL	mono;
	pblock->GetValue(fg_grain,t,grain,valid);
	pblock->GetValue(fg_mask,t,mask,valid);
//	pblock->GetValue(fg_mono,t,mono,valid);
	//-- A negative value means affect all three channels alike (Monochrome)
//	if (mono)
//		grain *= -1.0f;
	bm->FilmGrain(grain,mask,::fxCallBack,checkAbort);
}



//-- EOF: FilmGrain.cpp -------------------------------------------------------
