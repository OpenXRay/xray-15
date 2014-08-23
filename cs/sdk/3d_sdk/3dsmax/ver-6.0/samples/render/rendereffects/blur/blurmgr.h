/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurMgr.h

	 DESCRIPTION: blur manager - class declarations

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLUR_H_INCLUDED_)
#define _BLUR_H_INCLUDED_

// maxsdk includes and predeclarations
#include <max.h>
#include <iparamm2.h>
#include <bmmlib.h>
#include <icurvctl.h>

// local includes and predeclarations
#include "globals.h"
#include "resource.h"
#include "dllMain.h"
#include "dlgProcs.h"
#include "_blurTypes\blurUniform.h"
#include "_blurTypes\blurDirectional.h"
#include "_blurTypes\blurRadial.h"
#include "_selectTypes\selImage.h"
#include "_selectTypes\selIgnBack.h"
#include "_selectTypes\selLum.h"
#include "_selectTypes\selMaps.h"
#include "_selectTypes\selObjIds.h"
#include "_selectTypes\selMatIds.h"

#define NUM_SUBS	numIDs			// paramblocks + curve control
#define NUM_REFS	NUM_SUBS		// paramblocks + curve control
#define NUM_PBLOCKS	NUM_SUBS - 1	// paramblocks only

// ----------------------------------------
// blur effect - class declaration
// ----------------------------------------
class BlurMgr : public Effect, public ResourceMakerCallback
{
protected:
	DWORD m_lastBMModifyID;
	CheckAbortCallback *m_checkAbort;
	int m_imageSz, m_imageW, m_imageH;
	CompMap m_compMap;
	bool m_compValid;

	// because the current implementation of the curve control manages it's curve parameters internally,
	// each blur must build and maintain it's own control. since multiple blur instances share a common rollup in the
	// render effects dialog, these curve controls will be switched via SetThing() in the MasterDlgProc class.
	ICurveCtl *mp_CCtl;

	// blur type instances
	BlurBase *m_blurs[numBlurs];

	// to hold param ids of selActive parameters
	// this is to incorporate new seltypes which, when added in later versions, will break the sequential enumeration originally used
	int m_selActiveIds[numSels];

	// selection type instances
	SelBase *m_sels[numSels];

	// local copies of paramBlock elements
	BOOL m_activeSels[numSels];

	void updateSelections(TimeValue t, Bitmap *bm, RenderGlobalContext *gc);
	void blur(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, BOOL composite);

public:
	// paramblocks are instance specific
	IParamBlock2 *pbMaster, *pbBlurData, *pbSelData;

	// parammaps are shared among instances
	static IParamMap2 *pmMaster;
	static IParamMap2 *pmBlurData;
	static IParamMap2 *pmSelData;

	// class descriptor is shared among instances
	static const Class_ID blurMgrClassID;

	// dialog procs are shared among instances
	static MasterDlgProc masterDlgProc;
	static BlurDataDlgProc blurDataDlgProc;
	static SelDataDlgProc selDataDlgProc;

	// paramblock descriptors are shared among instances
	static ParamBlockDesc2 pbdMaster;
	static ParamBlockDesc2 pbdBlurData;
	static ParamBlockDesc2 pbdSelData;

	BlurMgr();
	~BlurMgr();
	BOOL progress(const TCHAR *title,int done, int total) { m_checkAbort->SetTitle(title); return m_checkAbort->Progress(done,total); }
	BOOL getBlurValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0) { return pbBlurData->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL getBlurValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0) { return pbBlurData->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL getBlurValue(ParamID id, TimeValue t, INode*& v, Interval &ivalid, int tabIndex=0) { return pbBlurData->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL setBlurValue(ParamID id, TimeValue t, INode*& v, int tabIndex=0) { return pbBlurData->SetValue(id, t, v, tabIndex); }
	BOOL getSelValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0) { return pbSelData->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL getSelValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0) { return pbSelData->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL getSelValue(ParamID id, TimeValue t, Texmap*& v, Interval &ivalid, int tabIndex=0) { return pbSelData->GetValue(id, t, v, ivalid, tabIndex); }
	int  getSelTabCount(ParamID id) { return pbSelData->Count(id); }
	void blurEnable(ParamID id, BOOL onOff, int tabIndex=0) { pmBlurData->Enable(id, onOff, tabIndex); }
	void selEnable(ParamID id, BOOL onOff, int tabIndex=0) { pmSelData->Enable(id, onOff, tabIndex); }
	HWND getBlurHWnd() { return pmBlurData->GetHWnd(); }
	HWND getSelHWnd() { return pmBlurData->GetHWnd(); }
	ICurveCtl* getCCtrl() { return mp_CCtl; }
	ICurve* getBrightenCurve() { return (mp_CCtl) ? mp_CCtl->GetControlCurve(0) : NULL; }
	ICurve* getBlendCurve()    { return (mp_CCtl) ? mp_CCtl->GetControlCurve(1) : NULL; }
	void blurError(int stringId) { GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, DISPLAY_DIALOG, GetString(IDS_ERR_TITLE), GetString(stringId), _T("")); }

	// Animatable/Reference
	void *GetInterface(ULONG id);
	int NumSubs() { return NUM_SUBS; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	int NumRefs() { return NUM_REFS; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	Class_ID ClassID() { return blurMgrClassID; }
	void GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
	int NumParamBlocks() { return NUM_PBLOCKS; }
	IParamBlock2* GetParamBlock(int i) { return (IParamBlock2 *)GetReference(i); }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (IParamBlock2 *)GetReference(id); }
	IOResult Load(ILoad *iload);

	// Effect
	TSTR GetName() { return GetString(IDS_NAME); }
	EffectParamDlg *CreateParamDialog(IRendParams *ip);
	DWORD GBufferChannelsRequired(TimeValue t);
	void Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *_checkAbort);

	// ResourceMakerCallback
	BOOL SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl) { return FALSE; }
	BOOL GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl) { return FALSE; }
	void ResetCallback(int curvenum, ICurveCtl *pCCtl);
	void NewCurveCreatedCallback(int curvenum, ICurveCtl *pCCtl);
};

// --------------------------------------------------
// blur class descriptor - class declaration
// --------------------------------------------------
class BlurMgrClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BlurMgr; }
	const TCHAR *	ClassName() { return GetString(IDS_CDESC_CLASS_NAME); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID		ClassID() { return BlurMgr::blurMgrClassID; }
	const TCHAR*	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("Blur"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

extern BlurMgrClassDesc blurMgrCD;

#endif // !defined(_BLUR_H_INCLUDED_)
