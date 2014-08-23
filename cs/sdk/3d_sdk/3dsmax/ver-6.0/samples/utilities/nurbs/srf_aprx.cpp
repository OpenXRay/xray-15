/**********************************************************************
 *<
    FILE: srf_aprx.cpp

    DESCRIPTION:  Surf Approx Setting Utility for NURBS

    CREATED BY: Charlie Thaeler

    HISTORY: created 13 August, 1997

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "nutil.h"


#define MAX_F 1000.0f
#define MAX_POS	((float)1e30)


static SurfApproxUtilDesc sSurfApproxUtilDesc;

ClassDesc*
GetSurfApproxUtilDesc()
{
	return &sSurfApproxUtilDesc;
}



const TCHAR *
SurfApproxUtilDesc::ClassName()
{
	return GetString(IDS_SURF_APPROX);
}

const TCHAR *
SurfApproxUtilDesc::Category()
{
	return GetString(IDS_NURBS);
}




SurfApproxUtil::SurfApproxUtil(void)
{
	mpSurfView = NULL;
	mpSurfRend = NULL;
	mpDispRend = NULL;
	mpCurveView = NULL;
	mpCurveRend = NULL;
	mClearSurfaces = TRUE;

	ResetTess();
	ResetDisplay();
}

SurfApproxUtil::~SurfApproxUtil(void)
{
	if (mpCurveView)
		delete mpCurveView;
	mpCurveView = NULL;

	if (mpCurveRend)
		delete mpCurveRend;
	mpCurveRend = NULL;

	if (mpSurfView)
		delete mpSurfView;
	mpSurfView = NULL;

	if (mpSurfRend)
		delete mpSurfRend;
	mpSurfRend = NULL;

	if (mpDispRend)
		delete mpDispRend;
	mpDispRend = NULL;
}


void
SurfApproxUtil::SetSurfApprox()
{
	for (int i=0; i < mpIp->GetSelNodeCount(); i++) {
		// Get a selected node
		INode* node = mpIp->GetSelNode(i);
		Object* obj = node->GetObjectRef();
		SetSurfaceApprox(obj, TRUE, mpSurfView, mClearSurfaces);
		SetSurfaceApprox(obj, FALSE, mpSurfRend, mClearSurfaces);
		SetCurveApprox(obj, TRUE, mpCurveView, mClearSurfaces);
		SetCurveApprox(obj, FALSE, mpCurveRend, mClearSurfaces);
		SetDispApprox(obj, mpDispRend, mClearSurfaces);
	}
	mpIp->RedrawViews(mpIp->GetTime());
}

void
SurfApproxUtil::SetSurfDisplay()
{
	NURBSResult retval = kNOk;
	for (int i=0; i < mpIp->GetSelNodeCount(); i++) {
		// Get a selected node
		INode* node = mpIp->GetSelNode(i);
		Object* obj = node->GetObjectRef();
		retval = SetSurfaceDisplaySettings(obj, mDisplay);
	}
	mpIp->RedrawViews(mpIp->GetTime());
}

void
SurfApproxUtil::ResetTess()
{
	mClearSurfaces = TRUE;

	mSettingType = kSurfView;

	if (mpSurfRend == NULL)
		mpSurfRend = new TessApprox;
    *mpSurfRend = *GetTessPreset((int) kSurfRend, 1);

    // Default for viewport tessellation
	if (mpSurfView == NULL)
		mpSurfView = new TessApprox;
    *mpSurfView = *GetTessPreset((int) kSurfView, 1);

	if (mpDispRend)
		delete mpDispRend;
	mpDispRend = NULL;

	if (mpCurveView)
		delete mpCurveView;
	mpCurveView = NULL;

	if (mpCurveRend)
		delete mpCurveRend;
	mpCurveRend = NULL;
}


void
SurfApproxUtil::ResetDisplay()
{
	mDisplay.mDisplayCurves = TRUE;
	mDisplay.mDisplaySurfaces = TRUE;
	mDisplay.mDisplayLattices = FALSE;
	mDisplay.mDisplaySurfCVLattices = TRUE;
	mDisplay.mDisplayCurveCVLattices = TRUE;
	mDisplay.mDisplayDependents = TRUE;
	mDisplay.mDisplayTrimming = TRUE;
	mDisplay.mDegradeOnMove = TRUE;
}


static HWND hApprox = NULL;
static HWND hDisplay = NULL;
INT_PTR CALLBACK SurfApproxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SurfDisplayDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void
SurfApproxUtil::BeginEditParams(Interface *ip,IUtil *iu)
{
	this->mpIu = iu;
	this->mpIp = (IObjParam*)ip;
	hApprox = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SURF_APPROX),
		SurfApproxDlgProc,
		GetString(IDS_SURF_APPROX),
		(LPARAM)this);
	hDisplay = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SURF_DISPLAY),
		SurfDisplayDlgProc,
		GetString(IDS_SURF_DISPLAY),
		(LPARAM)this);
}


void
SurfApproxUtil::EndEditParams(Interface *ip,IUtil *iu)
{
	this->mpIu = NULL;
	this->mpIp = NULL;
	ip->DeleteRollupPage(hApprox);
	hApprox = NULL;
	ip->DeleteRollupPage(hDisplay);
	hDisplay = NULL;
}





// Display toggles
void
SurfApproxUtil::SetupDisplayUI(HWND hDlg)
{
	CheckDlgButton( hDlg, IDC_DISPLATTICE, mDisplay.mDisplayLattices);
	CheckDlgButton( hDlg, IDC_DISP_CURVE, mDisplay.mDisplayCurves);
	CheckDlgButton( hDlg, IDC_DISP_SURFACE, mDisplay.mDisplaySurfaces);
	CheckDlgButton( hDlg, IDC_DISP_DEP, mDisplay.mDisplayDependents);
	CheckDlgButton( hDlg, IDC_DISP_TRIM, mDisplay.mDisplayTrimming);
	CheckDlgButton( hDlg, IDC_DEGRADE, mDisplay.mDegradeOnMove);
}

static INT_PTR CALLBACK
SurfDisplayDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SurfApproxUtil *pUtil = (SurfApproxUtil*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (msg) {
    case WM_INITDIALOG: {
        pUtil = (SurfApproxUtil*) lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		pUtil->SetupDisplayUI(hWnd);
		break; }

    case WM_COMMAND:
		switch ( LOWORD(wParam) ) {
		case IDC_SURF_DISPLAY:
			pUtil->SetSurfDisplay();
			break;
		case IDC_RESET:
			pUtil->ResetDisplay();
			pUtil->SetupDisplayUI(hWnd);
			break;
		case IDC_DISPLATTICE:
			pUtil->mDisplay.mDisplayLattices = IsDlgButtonChecked(hWnd, IDC_DISPLATTICE);
			break;
		case IDC_DISP_CURVE:
			pUtil->mDisplay.mDisplayCurves = IsDlgButtonChecked(hWnd, IDC_DISP_CURVE);
			break;
		case IDC_DISP_SURFACE:
			pUtil->mDisplay.mDisplaySurfaces = IsDlgButtonChecked(hWnd, IDC_DISP_SURFACE);
			break;
		case IDC_DISP_DEP:
			pUtil->mDisplay.mDisplayDependents = IsDlgButtonChecked(hWnd, IDC_DISP_DEP);
			break;
		case IDC_DISP_TRIM:
			pUtil->mDisplay.mDisplayTrimming = IsDlgButtonChecked(hWnd, IDC_DISP_TRIM);
			break;
		case IDC_DEGRADE:
			pUtil->mDisplay.mDegradeOnMove = IsDlgButtonChecked(hWnd, IDC_DEGRADE);
			break;
		}
		break;
	}
    return FALSE;
}







// Approximation
class AdvParams {
public:
	TessSubdivStyle mStyle;
	int mMin, mMax;
	int mTris;
};

static AdvParams sParams;



static ISpinnerControl* psMergeSpin = NULL;
static ISpinnerControl* psUSpin = NULL;
static ISpinnerControl* psVSpin = NULL;
static ISpinnerControl* psEdgeSpin = NULL;
static ISpinnerControl* psDistSpin = NULL;
static ISpinnerControl* psAngSpin = NULL;
static ISpinnerControl* psIsoUSpin = NULL;
static ISpinnerControl* psIsoVSpin = NULL;


INT_PTR CALLBACK ApproxParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK AdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );



static void
SetupUI(HWND hDlg, SurfApproxUtil *pUtil)
{
	// clear out the settings of the radio groups so they
	// will work properly
	CheckDlgButton( hDlg, IDC_ISO_ONLY, FALSE);
	CheckDlgButton( hDlg, IDC_ISO_AND_MESH, FALSE);
	CheckDlgButton( hDlg, IDC_MESH_ONLY, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_VIEW, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_RENDERER, FALSE);

	// turn on the isoline controls
	if (pUtil->mpSurfView->vpt_cfg == MESH_ONLY) {
		psIsoUSpin->Disable();
		psIsoVSpin->Disable();
	} else {
		psIsoUSpin->Enable();
		psIsoVSpin->Enable();
		psIsoUSpin->SetValue(pUtil->mpSurfView->u_iso, FALSE);
		psIsoVSpin->SetValue(pUtil->mpSurfView->v_iso, FALSE);
	}

	switch (pUtil->mpSurfView->vpt_cfg) {
	case ISO_ONLY:
		CheckDlgButton( hDlg, IDC_ISO_ONLY, TRUE);
		break;
	case ISO_AND_MESH:
		CheckDlgButton( hDlg, IDC_ISO_AND_MESH, TRUE);
		break;
	case MESH_ONLY:
		CheckDlgButton( hDlg, IDC_MESH_ONLY, TRUE);
		break;
	}


	// start off showing everything
	EnableWindow(GetDlgItem(hDlg, IDC_DISPLACEMENT), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_TXT), TRUE);
    psMergeSpin->Enable(TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_MERGE_TXT), TRUE);

	// clear out the settings of the radio groups so they
	// will work properly
	CheckDlgButton( hDlg, IDC_TESS_VIEW, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_RENDERER, FALSE);
	CheckDlgButton( hDlg, IDC_MESH, FALSE);
	CheckDlgButton( hDlg, IDC_TRIM, FALSE);
	CheckDlgButton( hDlg, IDC_DISPLACEMENT, FALSE);

	CheckDlgButton( hDlg, IDC_TESS_REGULAR, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_PARAM, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_SPATIAL, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_CURV, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_LDA, FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET1), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET2), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET3), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRESET1), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRESET2), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRESET3), TRUE);

	switch (pUtil->mSettingType) {
	case kSurfView:
		EnableWindow(GetDlgItem(hDlg, IDC_DISPLACEMENT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), FALSE);

		CheckDlgButton( hDlg, IDC_TESS_VIEW, TRUE);
		CheckDlgButton( hDlg, IDC_MESH, TRUE);
		break;

	case kSurfRend:
		CheckDlgButton( hDlg, IDC_TESS_RENDERER, TRUE);
		CheckDlgButton( hDlg, IDC_MESH, TRUE);
		break;

	case kSurfDisp:
        psMergeSpin->Enable(FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_MERGE_TXT), FALSE);

		CheckDlgButton( hDlg, IDC_TESS_RENDERER, TRUE);
		CheckDlgButton( hDlg, IDC_DISPLACEMENT, TRUE);
		break;

	case kCurveView:
		EnableWindow(GetDlgItem(hDlg, IDC_DISPLACEMENT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_TXT), FALSE);

        psMergeSpin->Enable(FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_MERGE_TXT), FALSE);

		CheckDlgButton( hDlg, IDC_TESS_VIEW, TRUE);
		CheckDlgButton( hDlg, IDC_TRIM, TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET1), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET3), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET1), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET3), FALSE);
		break;

	case kCurveRend:
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_TXT), FALSE);

        psMergeSpin->Enable(FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_MERGE_TXT), FALSE);

		CheckDlgButton( hDlg, IDC_TESS_RENDERER, TRUE);
		CheckDlgButton( hDlg, IDC_TRIM, TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET1), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SAVE_PRESET3), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET1), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRESET3), FALSE);
		break;
	}


	TessApprox *pTess = pUtil->GetTess(pUtil->mSettingType);

	TessApprox tess;
	tess.type = TESS_CURVE;

	if (pTess)
		tess = *pTess;
	// start by enabling and setting the values of all the spinners
	psUSpin->Enable();
	psVSpin->Enable();
	psEdgeSpin->Enable();
	psDistSpin->Enable();
	psAngSpin->Enable();

	psVSpin->SetValue(tess.v, FALSE);
	psUSpin->SetValue(tess.u, FALSE);
	psEdgeSpin->SetValue(tess.edge, FALSE);
	psDistSpin->SetValue(tess.dist, FALSE);
	psAngSpin->SetValue(tess.ang, FALSE);


	switch (tess.type) {
	case TESS_SET:
	case TESS_ISO:
		assert(0);
		break;

	case TESS_REGULAR:
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();

		CheckDlgButton( hDlg, IDC_TESS_REGULAR, TRUE);
		break;

	case TESS_PARAM:
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();

		CheckDlgButton( hDlg, IDC_TESS_PARAM, TRUE);
		break;

	case TESS_SPATIAL:
		psUSpin->Disable();
		psVSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();

		CheckDlgButton( hDlg, IDC_TESS_SPATIAL, TRUE);
		break;

	case TESS_CURVE:
		psUSpin->Disable();
		psVSpin->Disable();
		psEdgeSpin->Disable();

		CheckDlgButton( hDlg, IDC_TESS_CURV, TRUE);
		break;

	case TESS_LDA:
		psUSpin->Disable();
		psVSpin->Disable();

		CheckDlgButton( hDlg, IDC_TESS_LDA, TRUE);
		break;
	}

	CheckDlgButton( hDlg, IDC_TESS_VIEW_DEP, tess.view);

}

TessApprox*
SurfApproxUtil::GetPreset(int preset)
{
    return GetTessPreset((int) mSettingType, preset);
}


void
SurfApproxUtil::SetPreset(int preset, TessApprox& tess)
{
    SetTessPreset((int) mSettingType, preset, tess);
}



INT_PTR CALLBACK
SurfApproxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SurfApproxUtil *pUtil = (SurfApproxUtil*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

	TessApprox tess, *pTess;
	tess.type = TESS_CURVE;
	if (pUtil) {
		TessApprox *pTess = pUtil->GetTess(pUtil->mSettingType);
		if (pTess)
			tess = *pTess;
	}

    switch (msg) {
    case WM_INITDIALOG: {
        pUtil = (SurfApproxUtil*) lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		TessApprox vTess;
		vTess = *pUtil->mpSurfView;
		psIsoUSpin = SetupIntSpinner( hWnd, IDC_ISO_U_SPINNER, IDC_ISO_U, 0, 100, vTess.u_iso);
		psIsoVSpin = SetupIntSpinner( hWnd, IDC_ISO_V_SPINNER, IDC_ISO_V, 0, 100, vTess.v_iso);
		CheckDlgButton( hWnd, IDC_CLEAR_SURF, pUtil->mClearSurfaces);

		// the values will be reset by SetupUI()
		psUSpin = SetupIntSpinner( hWnd, IDC_TESS_U_SPINNER, IDC_TESS_U, 1, 100, vTess.u);
		psVSpin = SetupIntSpinner( hWnd, IDC_TESS_V_SPINNER, IDC_TESS_V, 1, 100, vTess.v);

		psEdgeSpin = SetupFloatSpinner( hWnd, IDC_TESS_EDGE_SPINNER, IDC_TESS_EDGE, 0.0f, MAX_F, vTess.edge);
		psDistSpin = SetupFloatSpinner( hWnd, IDC_TESS_DIST_SPINNER, IDC_TESS_DIST, 0.0f, MAX_F, vTess.dist);
		psAngSpin =  SetupFloatSpinner( hWnd, IDC_TESS_ANG_SPINNER,  IDC_TESS_ANG, 0.0f, 180.0f, vTess.ang);
		psMergeSpin = SetupFloatSpinner( hWnd, IDC_MERGE_SPINNER, IDC_MERGE, 0.0f, MAX_POS, vTess.merge);
		// since angle is more confined (0->180) don't autoscale it.
		psEdgeSpin->SetAutoScale(TRUE);
		psDistSpin->SetAutoScale(TRUE);
		psMergeSpin->SetAutoScale(TRUE);
		SetupUI(hWnd, pUtil);
        break; }


    case WM_DESTROY:
		if( psIsoUSpin ) {
			ReleaseISpinner(psIsoUSpin);
			psIsoUSpin = NULL;
		}
		if( psIsoVSpin ) {
			ReleaseISpinner(psIsoVSpin);
			psIsoVSpin = NULL;
		}
		if( psMergeSpin ) {
			ReleaseISpinner(psMergeSpin);
			psMergeSpin = NULL;
		}
		if( psUSpin ) {
			ReleaseISpinner(psUSpin);
			psUSpin = NULL;
		}
		if( psVSpin ) {
			ReleaseISpinner(psVSpin);
			psVSpin = NULL;
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



    case CC_SPINNER_CHANGE:
		switch ( LOWORD(wParam) ) {
		case IDC_ISO_U_SPINNER:
			pUtil->mpSurfView->u_iso = psIsoUSpin->GetIVal();
			SetupUI(hWnd, pUtil);
			break;
		case IDC_ISO_V_SPINNER:
			pUtil->mpSurfView->v_iso = psIsoVSpin->GetIVal();
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_U_SPINNER:
			tess.u = psUSpin->GetIVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		case IDC_TESS_V_SPINNER:
			tess.v = psVSpin->GetIVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		case IDC_TESS_EDGE_SPINNER:
			tess.edge = psEdgeSpin->GetFVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		case IDC_TESS_DIST_SPINNER:
			tess.dist = psDistSpin->GetFVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		case IDC_TESS_ANG_SPINNER:
			tess.ang = psAngSpin->GetFVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		case IDC_MERGE_SPINNER:
			tess.merge = psMergeSpin->GetFVal();
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;
		}
        break;


    case WM_COMMAND:
		switch ( LOWORD(wParam) ) {
		case IDC_SURF_APPROX:
			pUtil->SetSurfApprox();
			break;
		case IDC_RESET:
			pUtil->ResetTess();
			SetupUI(hWnd, pUtil);
			break;

		case IDC_CLEAR_SURF:
			pUtil->mClearSurfaces = IsDlgButtonChecked(hWnd, IDC_CLEAR_SURF);
			break;

		case IDC_ISO_ONLY:
			pUtil->mpSurfView->vpt_cfg = ISO_ONLY;
			SetupUI(hWnd, pUtil);
			break;

		case IDC_ISO_AND_MESH:
			pUtil->mpSurfView->vpt_cfg = ISO_AND_MESH;
			SetupUI(hWnd, pUtil);
			break;

		case IDC_MESH_ONLY:
			pUtil->mpSurfView->vpt_cfg = MESH_ONLY;
			SetupUI(hWnd, pUtil);
			break;


		case IDC_TESS_VIEW:
			if (IsDlgButtonChecked(hWnd, IDC_MESH))
				pUtil->mSettingType = kSurfView;
			else
				pUtil->mSettingType = kCurveView;
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_RENDERER:
			if (IsDlgButtonChecked(hWnd, IDC_MESH)) {
				pUtil->mSettingType = kSurfRend;
			} else if (IsDlgButtonChecked(hWnd, IDC_TRIM)) {
				pUtil->mSettingType = kCurveRend;
			} else {
				pUtil->mSettingType = kSurfDisp;
			}
			SetupUI(hWnd, pUtil);
			break;
		case IDC_MESH:
			if (IsDlgButtonChecked(hWnd, IDC_TESS_VIEW))
				pUtil->mSettingType = kSurfView;
			else
				pUtil->mSettingType = kSurfRend;
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TRIM:
			if (IsDlgButtonChecked(hWnd, IDC_TESS_VIEW))
				pUtil->mSettingType = kCurveView;
			else
				pUtil->mSettingType = kCurveRend;
			SetupUI(hWnd, pUtil);
			break;
		case IDC_DISPLACEMENT:
			pUtil->mSettingType = kSurfDisp;
			SetupUI(hWnd, pUtil);
			break;

		case IDC_TESS_REGULAR:
			tess.type = TESS_REGULAR;
			pUtil->SetTess(tess, pUtil->mSettingType);
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_PARAM:
			tess.type = TESS_PARAM;
			pUtil->SetTess(tess, pUtil->mSettingType);
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_SPATIAL:
			tess.type = TESS_SPATIAL;
			pUtil->SetTess(tess, pUtil->mSettingType);
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_CURV:
			tess.type = TESS_CURVE;
			pUtil->SetTess(tess, pUtil->mSettingType);
			SetupUI(hWnd, pUtil);
			break;
		case IDC_TESS_LDA:
			tess.type = TESS_LDA;
			pUtil->SetTess(tess, pUtil->mSettingType);
			SetupUI(hWnd, pUtil);
			break;

		case IDC_TESS_VIEW_DEP:
			tess.view = IsDlgButtonChecked(hWnd, IDC_TESS_VIEW_DEP);
			pUtil->SetTess(tess, pUtil->mSettingType);
			break;

        case IDC_PRESET1:
            pTess = pUtil->GetPreset(0);
            if (pTess) {
                tess = *pTess;
                pUtil->SetTess(tess, pUtil->mSettingType);
                SetupUI(hWnd, pUtil);
            }
            break;

        case IDC_PRESET2:
            pTess = pUtil->GetPreset(1);
            if (pTess) {
                tess = *pTess;
                pUtil->SetTess(tess, pUtil->mSettingType);
                SetupUI(hWnd, pUtil);
            }
            break;

        case IDC_PRESET3:
            pTess = pUtil->GetPreset(2);
            if (pTess) {
                tess = *pTess;
                pUtil->SetTess(tess, pUtil->mSettingType);
                SetupUI(hWnd, pUtil);
            }
            break;

        case IDC_SAVE_PRESET1:
			pUtil->SetPreset(0, tess);
            break;

        case IDC_SAVE_PRESET2:
			pUtil->SetPreset(1, tess);
            break;

        case IDC_SAVE_PRESET3:
			pUtil->SetPreset(2, tess);
            break;

		case IDC_ADVANCED: {
			sParams.mStyle = tess.subdiv;
			sParams.mMin = tess.minSub;
			sParams.mMax = tess.maxSub;
			sParams.mTris = tess.maxTris;
			int retval = DialogBox( hInstance,
						MAKEINTRESOURCE(IDD_SURF_APPROX_ADV),
						hWnd, AdvParametersDialogProc);
			if (retval == 1) {
				BOOL confirm = FALSE;
				if ((sParams.mStyle == SUBDIV_DELAUNAY && sParams.mTris > 200000) ||
					(sParams.mStyle != SUBDIV_DELAUNAY && sParams.mMax > 5)) {
					// warning!
					TSTR title = GetString(IDS_ADV_SURF_APPROX_WARNING_TITLE),
						warning = GetString(IDS_ADV_SURF_APPROX_WARNING);
					if (MessageBox(hWnd, warning, title,
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 ) == IDYES)
						confirm = TRUE;
 
				} else
					confirm = TRUE;
				if (confirm) {
					tess.subdiv = sParams.mStyle;
					tess.minSub = sParams.mMin;
					tess.maxSub = sParams.mMax;
					tess.maxTris = sParams.mTris;
					pUtil->SetTess(tess, pUtil->mSettingType);
				}
			}
			break; }
		}
        break;
    }
    return FALSE;
}








static ISpinnerControl* psMinSpin = NULL;
static ISpinnerControl* psMaxSpin = NULL;
static ISpinnerControl* psMaxTrisSpin = NULL;
// this max matches the MI max.
#define MAX_SUBDIV 7


static BOOL initing = FALSE; // this is a hack but CenterWindow causes bad commands

INT_PTR CALLBACK
AdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg) {
    case WM_INITDIALOG: {
		initing = TRUE;
        CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
		initing = FALSE;
		psMinSpin = SetupIntSpinner( hDlg, IDC_TESS_MIN_REC_SPINNER, IDC_TESS_MIN_REC, 0, sParams.mMax, sParams.mMin);
		psMaxSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_REC_SPINNER, IDC_TESS_MAX_REC, sParams.mMin, MAX_SUBDIV, sParams.mMax);
		psMaxTrisSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_TRIS_SPINNER, IDC_TESS_MAX_TRIS, 0, 2000000, sParams.mTris);
		switch (sParams.mStyle) {
		case SUBDIV_GRID:
			CheckDlgButton( hDlg, IDC_GRID, TRUE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_TREE:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_DELAUNAY:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
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
			break;
		case IDC_TREE:
			sParams.mStyle = SUBDIV_TREE;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case IDC_DELAUNAY:
			sParams.mStyle = SUBDIV_DELAUNAY;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
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





TessApprox *
SurfApproxUtil::GetTess(SurfApproxType type)
{
	switch(type) {
	case kCurveRend:
		if (mpCurveRend)
			return mpCurveRend;
		break;
	case kCurveView:
		if (mpCurveView)
			return mpCurveView;
		break;

	case kSurfView:
		if (mpSurfView)
			return mpSurfView;
		break;
	case kSurfDisp:
		if (mpDispRend)
			return mpDispRend;
		break;
	case kSurfRend:
		if (mpSurfRend)
			return mpSurfRend;
		break;
	default:
		assert(0);
	}

	return NULL;
}


void
SurfApproxUtil::SetTess(TessApprox &tess, SurfApproxType type)
{
	switch(type) {
	case kCurveView:
		if (mpCurveView == NULL) {
			mpCurveView = new TessApprox();
			mpCurveView->type = TESS_CURVE;
		}
		*mpCurveView = tess;
		break;
	case kCurveRend:
		if (mpCurveRend == NULL) {
			mpCurveRend = new TessApprox();
			mpCurveRend->type = TESS_CURVE;
		}
		*mpCurveRend = tess;
		break;
	case kSurfView:
		if (mpSurfView == NULL) {
			mpSurfView = new TessApprox();
			mpSurfView->type = TESS_CURVE;
		}
		*mpSurfView = tess;
		break;
	case kSurfRend:
		if (mpSurfRend == NULL) {
			mpSurfRend = new TessApprox();
			mpSurfRend->type = TESS_CURVE;
		}
		*mpSurfRend = tess;
		break;
	case kSurfDisp:
		if (mpDispRend == NULL) {
			mpDispRend = new TessApprox();
			mpDispRend->type = TESS_CURVE;
		}
		*mpDispRend = tess;
		break;
	default:
		assert(0);
	}
}


void
SurfApproxUtil::ClearTess(SurfApproxType type)
{
	switch(type) {
	case kCurveView:
		if (mpCurveView) {
			delete mpCurveView;
			mpCurveView = NULL;
		}
		break;
	case kCurveRend:
		if (mpCurveRend) {
			delete mpCurveRend;
			mpCurveRend = NULL;
		}
		break;

	case kSurfView:
		if (mpSurfView) {
			// don't really clear this one just reset it.
			mpSurfView->u       = 2;
			mpSurfView->v       = 2;
			mpSurfView->u_iso   = 2;
			mpSurfView->v_iso   = 3;
			mpSurfView->view    = FALSE;
			mpSurfView->ang     = 20.0f;
			mpSurfView->dist    = 10.0f;
			mpSurfView->edge    = 10.0f;
			mpSurfView->type    = TESS_CURVE;
			mpSurfView->merge   = 0.0f;
			mpSurfView->minSub  = 0;
			mpSurfView->maxSub  = 5;
			mpSurfView->subdiv = SUBDIV_TREE;
			mpSurfView->merge = 0.0f;
		}
		break;
	case kSurfRend:
		if (mpSurfRend) {
			delete mpSurfRend;
			mpSurfRend = NULL;
		}
		break;
	case kSurfDisp:
		if (mpDispRend) {
			delete mpDispRend;
			mpDispRend = NULL;
		}
		break;
	default:
		assert(0);
	}
}

