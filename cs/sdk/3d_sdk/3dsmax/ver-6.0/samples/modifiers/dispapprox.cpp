/**********************************************************************
 *<
	FILE: dispApprox.cpp

	DESCRIPTION:  Displacement Approximation OSM

	CREATED BY: Charlie Thaeler

	HISTORY: created 14 December, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_DISP_APPROX // JP Morel - July 23th 2002

#include "MeshDLib.h"

#define DISP_APPROX_CLASSID Class_ID(0x79f61cd0, 0x75511f8c)
#define MESH_MESHER_CLASSID Class_ID(0x291e236b, 0x24474d5d)



class DispApproxMod : public Modifier {
protected:
	static HWND mshWnd;
	TessApprox mApprox;
	bool mDoSubdiv;
	bool mSplitMesh;
	bool mCustomApprox;
public:
	DispApproxMod();
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message ) { return REF_SUCCEED; }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
	ChannelMask ChannelsUsed()  { return PART_GEOM | PART_TOPO | PART_DISP_APPROX | PART_TEXMAP; }
	ChannelMask ChannelsChanged() { return PART_DISP_APPROX; }

	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	Class_ID ClassID() { return DISP_APPROX_CLASSID; }
	Class_ID InputType() { return triObjectClassID; }
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	TCHAR *GetObjectName() {return GetString(IDS_CT_DISP_APPROX);}

	TessApprox& DisplacmentApprox() { return mApprox; }
	bool& DoSubdivisionDisplacment() { return mDoSubdiv; }
	bool& SplitMeshForDisplacement() { return mSplitMesh; }
	void SetDisplacmentApproxToPreset(int preset);
	bool& CustomApprox() { return mCustomApprox; }
};

class MeshMesherMod : public DispApproxMod {
public:
	MeshMesherMod();
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message ) { return REF_SUCCEED; }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
	ChannelMask ChannelsUsed()  { return PART_GEOM | PART_TOPO | PART_DISP_APPROX | PART_TEXMAP; }
	ChannelMask ChannelsChanged() { return PART_ALL; }

	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Class_ID ClassID() { return MESH_MESHER_CLASSID; }
    SClass_ID SuperClassID() { return WSM_CLASS_ID; }
	Class_ID InputType() { return triObjectClassID; }
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	TCHAR *GetObjectName() {return GetString(IDS_CT_MESH_MESHERBINDING);}
	Interval LocalValidity(TimeValue t);
	Interval GetValidity(TimeValue t) { return FOREVER; }

	TessApprox& DisplacmentApprox() { return mApprox; }
	bool &DoSubdivisionDisplacment() { return mDoSubdiv; }
	bool& SplitMeshForDisplacement() { return mSplitMesh; }
	bool &UseCustomApprox() { return mCustomApprox; }
	void SetDisplacmentApproxToPreset(int preset);
	bool CustomApprox() { return mCustomApprox; }
};




class DispApproxClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return GetSystemSetting(SYSSET_ENABLE_EDITMESHMOD); }
	void *			Create(BOOL loading = FALSE ) { return new DispApproxMod; }
	const TCHAR *	ClassName() { return GetString(IDS_CT_DISPAPPROX_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return DISP_APPROX_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFEDIT);}
	void			ResetClassParams(BOOL fileReset) { /* Noop */ }
};
class MeshMesherClassDesc : public ClassDesc {
public:
	int 			IsPublic() { return GetSystemSetting(SYSSET_ENABLE_EDITMESHMOD); }
	void *			Create(BOOL loading = FALSE ) { return new MeshMesherMod; }
	const TCHAR *	ClassName() { return GetString(IDS_CT_MESH_MESHER_CLASS); }
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() { return MESH_MESHER_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFEDIT);}
	void			ResetClassParams(BOOL fileReset) { /* Noop */ }
};



static DispApproxClassDesc sDispApproxDesc;
extern ClassDesc* GetDispApproxModDesc() { return &sDispApproxDesc; }

static MeshMesherClassDesc sMeshMesherDesc;
extern ClassDesc* GetMeshMesherWSMDesc() { return &sMeshMesherDesc; }







HWND DispApproxMod::mshWnd = NULL;

DispApproxMod::DispApproxMod()
{
	SetDisplacmentApproxToPreset(1);
	mDoSubdiv = TRUE;
	mSplitMesh = TRUE;
	mCustomApprox = TRUE;
}

#define APPROX_CHUNKID		5000
#define DO_SUBDIV_CHUNKID	5001
#define SPLITMESH_CHUNKID	5002
#define MOD_CHUNK           5003

IOResult
DispApproxMod::Save(ISave *isave)
{
	IOResult res;
	ULONG nb;

	isave->BeginChunk(APPROX_CHUNKID);
	res = mApprox.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(DO_SUBDIV_CHUNKID);
	BOOL subdiv = mDoSubdiv?TRUE:FALSE;
	isave->Write(&subdiv, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(SPLITMESH_CHUNKID);
	BOOL split = mSplitMesh?TRUE:FALSE;
	isave->Write(&split, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(MOD_CHUNK);
    Modifier::Save(isave);
	isave->EndChunk();

	return res;
}


IOResult
DispApproxMod::Load(ILoad *iload)
{
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case APPROX_CHUNKID:
			mApprox.Load(iload);
			break;
        case DO_SUBDIV_CHUNKID: {
			BOOL subdiv;
            iload->Read(&subdiv, sizeof(BOOL), &nb);
			mDoSubdiv = subdiv?true:false;
            break; }
        case SPLITMESH_CHUNKID: {
			BOOL split;
            iload->Read(&split, sizeof(BOOL), &nb);
			mSplitMesh = split?true:false;
            break; }

        case MOD_CHUNK:
            Modifier::Load(iload);
            break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

void
DispApproxMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	TriObject *pTriObj = (TriObject*)os->obj;
	pTriObj->DisplacmentApprox() = mApprox;
	pTriObj->DoSubdivisionDisplacment() = mDoSubdiv;
	pTriObj->SplitMeshForDisplacement() = mSplitMesh;
	pTriObj->SetChannelValidity(DISP_APPROX_CHAN_NUM, FOREVER);
}

RefTargetHandle 
DispApproxMod::Clone(RemapDir& remap)
{
    DispApproxMod* pMod = new DispApproxMod();
	pMod->mApprox = mApprox;
	pMod->mDoSubdiv = mDoSubdiv;
	pMod->mSplitMesh = mSplitMesh;
	pMod->mCustomApprox = mCustomApprox;
	BaseClone(this, pMod, remap);
    return pMod;
}

class AdvParams {
public:
	TessSubdivStyle mStyle;
	int mMin, mMax;
	int mTris;
};
static AdvParams sParams;

static ISpinnerControl* psUSpin = NULL;
static ISpinnerControl* psEdgeSpin = NULL;
static ISpinnerControl* psDistSpin = NULL;
static ISpinnerControl* psAngSpin = NULL;

static HWND sHwnd = NULL;

static void
SetupApproxUI(HWND hWnd, TessApprox tapprox, bool dosubdiv, bool custom, bool splitmesh)
{
	if (GetDlgItem(hWnd, IDC_CUSTOM)) {
		EnableWindow( GetDlgItem(hWnd, IDC_CUSTOM), TRUE);
		CheckDlgButton( hWnd, IDC_CUSTOM, custom?TRUE:FALSE);
	}

	if (custom) {
		EnableWindow( GetDlgItem(hWnd, IDC_DO_SUBDIV), TRUE);
	} else {
		EnableWindow( GetDlgItem(hWnd, IDC_DO_SUBDIV), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_SPLITMESH), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), FALSE);

		EnableWindow( GetDlgItem(hWnd, IDC_PRESET1), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_PRESET2), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_PRESET3), FALSE);

		EnableWindow( GetDlgItem(hWnd, IDC_TESS_REGULAR), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_SPATIAL), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_CURV), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_LDA), FALSE);
		psUSpin->Disable();
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		return;
	}
	CheckDlgButton( hWnd, IDC_DO_SUBDIV, dosubdiv);
	CheckDlgButton( hWnd, IDC_SPLITMESH, splitmesh);
	if (!dosubdiv) {
		EnableWindow( GetDlgItem(hWnd, IDC_SPLITMESH), FALSE);

		EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), FALSE);

		EnableWindow( GetDlgItem(hWnd, IDC_PRESET1), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_PRESET2), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_PRESET3), FALSE);

		EnableWindow( GetDlgItem(hWnd, IDC_TESS_REGULAR), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_SPATIAL), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_CURV), FALSE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_LDA), FALSE);
		psUSpin->Disable();
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		return;
	}
	EnableWindow( GetDlgItem(hWnd, IDC_SPLITMESH), TRUE);

    EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), FALSE);
    EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), FALSE);
	CheckDlgButton( hWnd, IDC_TESS_REGULAR, FALSE);
	CheckDlgButton( hWnd, IDC_TESS_SPATIAL, FALSE);
	CheckDlgButton( hWnd, IDC_TESS_CURV, FALSE);
	CheckDlgButton( hWnd, IDC_TESS_LDA, FALSE);

	psUSpin->Enable();
	psEdgeSpin->Enable();
	psDistSpin->Enable();
	psAngSpin->Enable();

	EnableWindow( GetDlgItem(hWnd, IDC_PRESET1), TRUE);
	EnableWindow( GetDlgItem(hWnd, IDC_PRESET2), TRUE);
	EnableWindow( GetDlgItem(hWnd, IDC_PRESET3), TRUE);

	EnableWindow( GetDlgItem(hWnd, IDC_TESS_REGULAR), TRUE);
	EnableWindow( GetDlgItem(hWnd, IDC_TESS_SPATIAL), TRUE);
	EnableWindow( GetDlgItem(hWnd, IDC_TESS_CURV), TRUE);
	EnableWindow( GetDlgItem(hWnd, IDC_TESS_LDA), TRUE);

	psUSpin->SetValue(tapprox.u, FALSE);
	psEdgeSpin->SetValue(tapprox.edge, FALSE);
	psDistSpin->SetValue(tapprox.dist, FALSE);
	psAngSpin->SetValue(tapprox.ang, FALSE);

	switch(tapprox.type) {
	case TESS_REGULAR:
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		CheckDlgButton( hWnd, IDC_TESS_REGULAR, TRUE);
		break;
	case TESS_SPATIAL:
        EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), TRUE);
		psUSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		CheckDlgButton( hWnd, IDC_TESS_SPATIAL, TRUE);
		break;
	case TESS_CURVE:
        EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), TRUE);
		psEdgeSpin->Disable();
		psUSpin->Disable();
		CheckDlgButton( hWnd, IDC_TESS_CURV, TRUE);
		break;
	case TESS_LDA:
        EnableWindow( GetDlgItem(hWnd, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hWnd, IDC_TESS_VIEW_DEP), TRUE);
		psUSpin->Disable();
		CheckDlgButton( hWnd, IDC_TESS_LDA, TRUE);
		break;
	}
}

class UIApproxRestore : public RestoreObj {
public:		
	DispApproxMod *mpMod;
	TessApprox mApprox, mApproxR;
	bool mDoSubdiv, mDoSubdivR;
	bool mSplitMesh, mSplitMeshR;
	bool mCustom, mCustomR;

    UIApproxRestore(DispApproxMod *pMod);

    void Restore(int isUndo);
    void Redo();
};

UIApproxRestore::UIApproxRestore(DispApproxMod *pMod)
{
	mpMod = pMod;
	mApprox = pMod->DisplacmentApprox();
	mDoSubdiv = pMod->DoSubdivisionDisplacment();
	mSplitMesh = pMod->DoSubdivisionDisplacment();
	mCustom = pMod->CustomApprox();
}

void
UIApproxRestore::Restore(int isUndo)
{
	if (isUndo) {
		mApproxR = mpMod->DisplacmentApprox();
		mDoSubdiv = mpMod->DoSubdivisionDisplacment();
		mSplitMesh = mpMod->SplitMeshForDisplacement();
	}
	mpMod->DisplacmentApprox() = mApprox;
	mpMod->DoSubdivisionDisplacment() = mDoSubdiv;
	mpMod->SplitMeshForDisplacement() = mSplitMesh;
	mpMod->CustomApprox() = mCustom;
	if (sHwnd)
		SetupApproxUI(sHwnd, mApprox, mDoSubdiv, mCustom, mSplitMesh);
}

void
UIApproxRestore::Redo()
{
	mpMod->DisplacmentApprox() = mApproxR;
	mpMod->DoSubdivisionDisplacment() = mDoSubdivR;
	mpMod->SplitMeshForDisplacement() = mSplitMeshR;
	mpMod->CustomApprox() = mCustomR;
	if (sHwnd)
		SetupApproxUI(sHwnd, mApproxR, mDoSubdivR, mCustomR, mSplitMeshR);
}



static void
Notify(DispApproxMod *pMod)
{
    pMod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


#define MAX_F 1000.0f
INT_PTR CALLBACK MeshAdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

INT_PTR CALLBACK
DispApproxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DispApproxMod *pMod = (DispApproxMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);


	switch (msg) {
	case WM_INITDIALOG: {
		pMod = (DispApproxMod*)lParam;
		sHwnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		psUSpin = SetupIntSpinner( hWnd, IDC_TESS_U_SPINNER, IDC_TESS_U, 1, 100,
									pMod->DisplacmentApprox().u);
		psEdgeSpin = SetupFloatSpinner( hWnd, IDC_TESS_EDGE_SPINNER, IDC_TESS_EDGE, 0.0f, MAX_F,
									pMod->DisplacmentApprox().edge);
		psDistSpin = SetupFloatSpinner( hWnd, IDC_TESS_DIST_SPINNER, IDC_TESS_DIST, 0.0f, MAX_F,
									pMod->DisplacmentApprox().dist);
		psAngSpin =  SetupFloatSpinner( hWnd, IDC_TESS_ANG_SPINNER,  IDC_TESS_ANG, 0.0f, 180.0f,
									pMod->DisplacmentApprox().ang);
		CheckDlgButton(hWnd, IDC_TESS_VIEW_DEP, pMod->DisplacmentApprox().view);
		SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
							pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
		break; }

	case CC_SPINNER_BUTTONDOWN:
		theHold.Begin();
		theHold.Put(new UIApproxRestore(pMod));
		break;


    case CC_SPINNER_CHANGE:
		if (!HIWORD(wParam)) {
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
		}
		switch ( LOWORD(wParam) ) {
		case IDC_TESS_U_SPINNER:
			pMod->DisplacmentApprox().u = psUSpin->GetIVal();
			break;
		case IDC_TESS_EDGE_SPINNER:
			pMod->DisplacmentApprox().edge = psEdgeSpin->GetFVal();
			break;
		case IDC_TESS_DIST_SPINNER:
			pMod->DisplacmentApprox().dist = psDistSpin->GetFVal();
			break;
		case IDC_TESS_ANG_SPINNER:
			pMod->DisplacmentApprox().ang = psAngSpin->GetFVal();
			break;
		}
  		if (!HIWORD(wParam)) {
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
		}
      break;

	case CC_SPINNER_BUTTONUP:
		if (HIWORD(wParam)) {
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
		} else {
			theHold.Cancel();
		}
		break;

    case WM_COMMAND:
		switch ( LOWORD(wParam) ) {
        case IDC_UPDATE:
			Notify(pMod);
            break;

		case IDC_CUSTOM:
			assert(GetDlgItem(hWnd, IDC_CUSTOM));
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->CustomApprox() = IsDlgButtonChecked(hWnd, IDC_CUSTOM)?true:false;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;

		case IDC_DO_SUBDIV:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DoSubdivisionDisplacment() = IsDlgButtonChecked(hWnd, IDC_DO_SUBDIV)?true:false;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_SPLITMESH:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->SplitMeshForDisplacement() = IsDlgButtonChecked(hWnd, IDC_SPLITMESH)?true:false;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_PRESET1:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->SetDisplacmentApproxToPreset(0);
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_PRESET2:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->SetDisplacmentApproxToPreset(1);
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_PRESET3:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->SetDisplacmentApproxToPreset(2);
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;

		case IDC_TESS_REGULAR:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DisplacmentApprox().type = TESS_REGULAR;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_TESS_SPATIAL:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DisplacmentApprox().type = TESS_SPATIAL;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_TESS_CURV:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DisplacmentApprox().type = TESS_CURVE;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;
		case IDC_TESS_LDA:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DisplacmentApprox().type = TESS_LDA;
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			SetupApproxUI(hWnd, pMod->DisplacmentApprox(), pMod->DoSubdivisionDisplacment(),
								pMod->CustomApprox(), pMod->SplitMeshForDisplacement());
			break;

		case IDC_TESS_VIEW_DEP:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(pMod));
			pMod->DisplacmentApprox().view = IsDlgButtonChecked(hWnd, IDC_TESS_VIEW_DEP);
			Notify(pMod);
			theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
			break;
		case IDC_ADVANCED: {
			sParams.mStyle = pMod->DisplacmentApprox().subdiv;
			sParams.mMin = pMod->DisplacmentApprox().minSub;
			sParams.mMax = pMod->DisplacmentApprox().maxSub;
			sParams.mTris = pMod->DisplacmentApprox().maxTris;
			int retval = DialogBoxParam( hInstance,
						MAKEINTRESOURCE(IDD_DISP_APPROX_ADV),
						hWnd, MeshAdvParametersDialogProc, (LPARAM)pMod);
			if (retval == 1) {
				BOOL confirm = FALSE;
				if ((sParams.mStyle == SUBDIV_DELAUNAY && sParams.mTris > 200000) ||
					(sParams.mStyle != SUBDIV_DELAUNAY && sParams.mMax > 5)) {
					// warning!
					TSTR title = GetString(IDS_ADV_DISP_APPROX_WARNING_TITLE),
						warning = GetString(IDS_ADV_DISP_APPROX_WARNING);
					if (MessageBox(hWnd, warning, title,
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 ) == IDYES)
						confirm = TRUE;
 
				} else
					confirm = TRUE;
				if (confirm) {
					theHold.Begin();
					theHold.Put(new UIApproxRestore(pMod));
					pMod->DisplacmentApprox().subdiv = sParams.mStyle;
					pMod->DisplacmentApprox().minSub = sParams.mMin;
					pMod->DisplacmentApprox().maxSub = sParams.mMax;
					pMod->DisplacmentApprox().maxTris = sParams.mTris;
					Notify(pMod);
					theHold.Accept(GetString(IDS_CT_DISP_APPROX_CHANGE));
				}
			}
			break; }
		}
        break;

		
	case WM_DESTROY:
		sHwnd = NULL;
		if( psUSpin ) {
			ReleaseISpinner(psUSpin);
			psUSpin = NULL;
		}
		if( psEdgeSpin ) {
			ReleaseISpinner(psEdgeSpin);
			psEdgeSpin = NULL;
		}
		if( psDistSpin ) {
			ReleaseISpinner(psDistSpin);
			psDistSpin = NULL;
		}
		if( psAngSpin ) {
			ReleaseISpinner(psAngSpin);
			psAngSpin = NULL;
		}
        break;
	default:
		return FALSE;
	}
	return TRUE;
}

static ISpinnerControl* psMinSpin = NULL;
static ISpinnerControl* psMaxSpin = NULL;
static ISpinnerControl* psMaxTrisSpin = NULL;
// this max matches the MI max.
#define MAX_SUBDIV 7

static BOOL initing = FALSE; // this is a hack but CenterWindow causes bad commands

INT_PTR CALLBACK
MeshAdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg) {
    case WM_INITDIALOG: {
		initing = TRUE;
        CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
		initing = FALSE;
		psMinSpin = SetupIntSpinner( hDlg, IDC_TESS_MIN_REC_SPINNER, IDC_TESS_MIN_REC,
			0, sParams.mMax, sParams.mMin);
		psMaxSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_REC_SPINNER, IDC_TESS_MAX_REC,
			sParams.mMin, MAX_SUBDIV, sParams.mMax);
		psMaxTrisSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_TRIS_SPINNER, IDC_TESS_MAX_TRIS,
			0, 2000000, sParams.mTris);
		switch (sParams.mStyle) {
		case SUBDIV_GRID:
			CheckDlgButton( hDlg, IDC_GRID, TRUE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			break;
		case SUBDIV_TREE:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			break;
		case SUBDIV_DELAUNAY:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			psMinSpin->Disable();
			psMaxSpin->Disable();
			psMaxTrisSpin->Enable();
			break;
		}
		break; }

    case WM_COMMAND:
		if (initing) return FALSE;
		switch ( LOWORD(wParam) ) {
		case IDOK:
			EndDialog(hDlg, 1);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDC_GRID:
			sParams.mStyle = SUBDIV_GRID;
			CheckDlgButton( hDlg, IDC_GRID, TRUE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			break;
		case IDC_TREE:
			sParams.mStyle = SUBDIV_TREE;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			break;
		case IDC_DELAUNAY:
			sParams.mStyle = SUBDIV_DELAUNAY;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			psMinSpin->Disable();
			psMaxSpin->Disable();
			psMaxTrisSpin->Enable();
			break;
		}
		break;

    case CC_SPINNER_CHANGE:
		switch ( LOWORD(wParam) ) {
		case IDC_TESS_MIN_REC_SPINNER:
			sParams.mMin = psMinSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_REC_SPINNER:
			sParams.mMax = psMaxSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_TRIS_SPINNER:
			sParams.mTris = psMaxTrisSpin->GetIVal();
			break;
		}
		break;

	case WM_DESTROY:
		if( psMinSpin ) {
			ReleaseISpinner(psMinSpin);
			psMinSpin = NULL;
		}
		if( psMaxSpin ) {
			ReleaseISpinner(psMaxSpin);
			psMaxSpin = NULL;
		}
		if( psMaxTrisSpin ) {
			ReleaseISpinner(psMaxTrisSpin);
			psMaxTrisSpin = NULL;
		}
		break;
	}

	return FALSE;
}

void
DispApproxMod::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
    if (!mshWnd) {
		mshWnd = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_DISP_APPROX),
			DispApproxDlgProc,
			GetString(IDS_CT_DISP_APPROX), 
			(LPARAM)this, 0);
	}
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	SetAFlag(A_MOD_BEING_EDITED);
}

void
DispApproxMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
	ip->ClearPickMode ();
    if (flags & END_EDIT_REMOVEUI) {
        ip->DeleteRollupPage(mshWnd);
        mshWnd = NULL;
    }

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
}


void
DispApproxMod::SetDisplacmentApproxToPreset(int preset)
{
	TriObject t;
	t.SetDisplacmentApproxToPreset(preset);
	TessApprox approx = t.DisplacmentApprox();
	mApprox = approx;
}














MeshMesherMod::MeshMesherMod()
{
	SetDisplacmentApproxToPreset(1);
	mDoSubdiv = TRUE;
	mCustomApprox = FALSE;
}

#define MESHER_APPROX_CHUNKID		5000
#define MESHER_DO_SUBDIV_CHUNKID	5001
#define MESHER_CUSTOM_CHUNKID		5002
#define MESHER_SPLITMESH_CHUNKID	5003
#define MESHER_MOD_CHUNK            5004

IOResult
MeshMesherMod::Save(ISave *isave)
{
	IOResult res;
	ULONG nb;
	isave->BeginChunk(MESHER_APPROX_CHUNKID);
	res = mApprox.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(MESHER_DO_SUBDIV_CHUNKID);
	BOOL subdiv = mDoSubdiv?TRUE:FALSE;
	isave->Write(&subdiv, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(MESHER_SPLITMESH_CHUNKID);
	BOOL split = mSplitMesh?TRUE:FALSE;
	isave->Write(&split, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(MESHER_CUSTOM_CHUNKID);
	BOOL cust = mCustomApprox?TRUE:FALSE;
	isave->Write(&cust, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(MESHER_MOD_CHUNK);
    Modifier::Save(isave);
	isave->EndChunk();
	return res;
}


IOResult
MeshMesherMod::Load(ILoad *iload)
{
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case MESHER_APPROX_CHUNKID:
			mApprox.Load(iload);
			break;
        case MESHER_DO_SUBDIV_CHUNKID: {
			BOOL subdiv;
            iload->Read(&subdiv, sizeof(BOOL), &nb);
			mDoSubdiv = subdiv?true:false;
            break; }
        case MESHER_SPLITMESH_CHUNKID: {
			BOOL split;
            iload->Read(&split, sizeof(BOOL), &nb);
			mSplitMesh = split?true:false;
            break; }
        case MESHER_CUSTOM_CHUNKID: {
			BOOL cust;
            iload->Read(&cust, sizeof(BOOL), &nb);
			mCustomApprox = cust?true:false;
            break; }
        case MESHER_MOD_CHUNK:
            Modifier::Load(iload);
            break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

void
MeshMesherMod::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
    if (!mshWnd) {
		mshWnd = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_MESHER_DISP_APPROX),
			DispApproxDlgProc,
			GetString(IDS_CT_DISP_APPROX), 
			(LPARAM)this, 0);
	}
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	SetAFlag(A_MOD_BEING_EDITED);
}

void
MeshMesherMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
	ip->ClearPickMode ();
    if (flags & END_EDIT_REMOVEUI) {
        ip->DeleteRollupPage(mshWnd);
        mshWnd = NULL;
    }

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
}


void
MeshMesherMod::SetDisplacmentApproxToPreset(int preset)
{
	TriObject t;
	t.SetDisplacmentApproxToPreset(preset);
	TessApprox approx = t.DisplacmentApprox();
	mApprox = approx;
}


class MesherOrthoView : public View {
public:
    MesherOrthoView() {
        projType = 1; // orthographic
        pixelSize = 1.0f;
        screenH = 10.0f;
        screenW = 10.0f;
    }
    virtual Point2 ViewToScreen(Point3 p) { return Point2(0.0f,0.0f); }
};

static int LoadAllMapFiles(Texmap *tm,TimeValue t) 
{
	tm->LoadMapFiles(t);
	for (int i=0; i<tm->NumSubTexmaps(); i++) {
		Texmap *st = tm->GetSubTexmap(i); 
		if (st) 
			LoadAllMapFiles(st,t);
    }
	return 1;
}

// CALL THIS TO LOAD ALL DISPLACEMENT MAPS IN THE MATERIAL
static int LoadDisplaceMaps(MtlBase *mb, TimeValue t) 
{
	for (int i=0; i<mb->NumSubTexmaps(); i++) {
		Texmap *st = mb->GetSubTexmap(i); 
		if (st) {
			if (mb->MapSlotType(i)==MAPSLOT_DISPLACEMENT) 
				LoadAllMapFiles(st,t);
			else
				LoadDisplaceMaps(st,t);
        }
    }
	if (IsMtl(mb)) {
		Mtl *m = (Mtl *)mb;
		for (i=0; i<m->NumSubMtls(); i++) {
			Mtl *sm = m->GetSubMtl(i);
			if (sm) 
				LoadDisplaceMaps(sm,t);
        }
    }
	return 1;
}



void
MeshMesherMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
    // Update the material.  When loading a file, it might not be up to date yet.
    if (node) {
        Mtl* pMtl = node->GetMtl();
        if (pMtl) {
            pMtl->Validity(t);
            LoadDisplaceMaps(pMtl, t);
        }
    }

	TriObject *pInTriObj = (TriObject*)os->obj;
	if (!pInTriObj->CanDoDisplacementMapping())
		return;

	if (mCustomApprox) {
		pInTriObj->DoSubdivisionDisplacment() = mDoSubdiv;
		pInTriObj->SplitMeshForDisplacement() = mSplitMesh;
		pInTriObj->DisplacmentApprox() = mApprox;
	}

    TriObject *pTriObj = CreateNewTriObject();
	pTriObj->DisableDisplacementMapping(TRUE);

    MesherOrthoView view;
	BOOL needDelete = FALSE;
    Mesh* pMesh = pInTriObj->GetRenderMesh(t, node, view, needDelete);
    pTriObj->GetMesh() = *pMesh;

	if (needDelete)
		delete pMesh;

	Interval tmValid = FOREVER;

	// Make sure the object's points are in world space. We do this
	// by multiplying by the ObjectState TM. If the points are
	// already in world space this matrix will be NULL so
	// there is no need to do this. Otherwise we will transform
	// the points by this TM thus putting them in world space.
	if (os->GetTM()) {
		Matrix3 tm = *(os->GetTM());
        int nVerts = pTriObj->GetMesh().getNumVerts();
		for (int i=0; i<nVerts; i++)
			pTriObj->GetMesh().verts[i] = pTriObj->GetMesh().verts[i]*tm;

		// Set the geometry channel interval to be the same
		// as the ObjectState TM interval since its validity
		// now governs the interval of the modified points
        tmValid &= os->tmValid();
		// Once the points are transformed the matrix needs to set to NULL
		os->SetTM(NULL, FOREVER);
	}

	Interval valid = FOREVER;

    valid &= os->obj->ChannelValidity(t, GEOM_CHAN_NUM);
    valid &= os->obj->ChannelValidity(t, TOPO_CHAN_NUM);
    valid &= os->obj->ChannelValidity(t, TEXMAP_CHAN_NUM);
    valid &= os->obj->ChannelValidity(t, DISP_APPROX_CHAN_NUM);

	pTriObj->SetChannelValidity(TOPO_CHAN_NUM, valid);
	pTriObj->SetChannelValidity(GEOM_CHAN_NUM, valid);
	pTriObj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);

    pTriObj->SetChannelValidity(MTL_CHAN_NUM, valid);
    pTriObj->SetChannelValidity(SELECT_CHAN_NUM, valid);
    pTriObj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
    
	pTriObj->SetChannelValidity(DISP_APPROX_CHAN_NUM, valid);

    os->obj = pTriObj;
    os->obj->UpdateValidity(GEOM_CHAN_NUM, GetValidity(t) & tmValid);	
	
	// RB 3/9/99: Gotta unlock the object since nobody owns it yet.
	os->obj->UnlockObject();
}


RefTargetHandle 
MeshMesherMod::Clone(RemapDir& remap)
{
    MeshMesherMod* pMod = new MeshMesherMod();
	pMod->mApprox = mApprox;
	pMod->mDoSubdiv = mDoSubdiv;
	pMod->mSplitMesh = mSplitMesh;
	pMod->mCustomApprox = mCustomApprox;
	BaseClone(this, pMod, remap);
    return pMod;
}

Interval MeshMesherMod::LocalValidity(TimeValue t)
{
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t); // No local animatables - for now.
}

#endif // MODIFIER_DISP_APPROX 