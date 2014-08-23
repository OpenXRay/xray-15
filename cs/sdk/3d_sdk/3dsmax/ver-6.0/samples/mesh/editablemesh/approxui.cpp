/**********************************************************************
 *<
	FILE: approxui.cpp

	DESCRIPTION: Editable Triangle Mesh Displacment Approximation code

	CREATED BY: Charlie Thaeler

	HISTORY: created 8 December 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "pack1.h"
#include "triobjed.h"




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

void EditTriObject::UpdateApproxUI () {
	if (!hApprox) return;
	TessApprox tapprox = DisplacmentApprox ();
	BOOL dosubdiv = DoSubdivisionDisplacment ();
	BOOL splitMesh = SplitMeshForDisplacement ();
	CheckDlgButton( hApprox, IDC_DO_SUBDIV, dosubdiv);
	CheckDlgButton( hApprox, IDC_SPLITMESH, splitMesh);
	if (!dosubdiv) {
		EnableWindow( GetDlgItem(hApprox, IDC_SPLITMESH), FALSE);

		EnableWindow( GetDlgItem(hApprox, IDC_ADVANCED), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_VIEW_DEP), FALSE);

		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIVISION_PRESET_BOX), false);
		EnableWindow( GetDlgItem(hApprox, IDC_PRESET1), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_PRESET2), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_PRESET3), FALSE);

		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIVISION_METHOD_BOX), false);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_REGULAR), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_SPATIAL), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_CURV), FALSE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_LDA), FALSE);
		psUSpin->Disable();
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_STEPS_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_EDGE_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_DIST_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_ANGLE_TEXT), false);
		return;
	}
	EnableWindow( GetDlgItem(hApprox, IDC_SPLITMESH), TRUE);

    EnableWindow( GetDlgItem(hApprox, IDC_ADVANCED), FALSE);
    EnableWindow( GetDlgItem(hApprox, IDC_TESS_VIEW_DEP), FALSE);
	CheckDlgButton( hApprox, IDC_TESS_REGULAR, FALSE);
	CheckDlgButton( hApprox, IDC_TESS_SPATIAL, FALSE);
	CheckDlgButton( hApprox, IDC_TESS_CURV, FALSE);
	CheckDlgButton( hApprox, IDC_TESS_LDA, FALSE);

	psUSpin->Enable();
	psEdgeSpin->Enable();
	psDistSpin->Enable();
	psAngSpin->Enable();

	EnableWindow (GetDlgItem (hApprox, IDC_SUBDIVISION_PRESET_BOX), true);
	EnableWindow( GetDlgItem(hApprox, IDC_PRESET1), TRUE);
	EnableWindow( GetDlgItem(hApprox, IDC_PRESET2), TRUE);
	EnableWindow( GetDlgItem(hApprox, IDC_PRESET3), TRUE);

	EnableWindow (GetDlgItem (hApprox, IDC_SUBDIVISION_METHOD_BOX), true);
	EnableWindow( GetDlgItem(hApprox, IDC_TESS_REGULAR), TRUE);
	EnableWindow( GetDlgItem(hApprox, IDC_TESS_SPATIAL), TRUE);
	EnableWindow( GetDlgItem(hApprox, IDC_TESS_CURV), TRUE);
	EnableWindow( GetDlgItem(hApprox, IDC_TESS_LDA), TRUE);

	psUSpin->SetValue(tapprox.u, FALSE);
	psEdgeSpin->SetValue(tapprox.edge, FALSE);
	psDistSpin->SetValue(tapprox.dist, FALSE);
	psAngSpin->SetValue(tapprox.ang, FALSE);

	switch(tapprox.type) {
	case TESS_REGULAR:
		psEdgeSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		CheckDlgButton( hApprox, IDC_TESS_REGULAR, TRUE);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_STEPS_TEXT), true);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_EDGE_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_DIST_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_ANGLE_TEXT), false);
		break;
	case TESS_SPATIAL:
        EnableWindow( GetDlgItem(hApprox, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_VIEW_DEP), TRUE);
		psUSpin->Disable();
		psDistSpin->Disable();
		psAngSpin->Disable();
		CheckDlgButton( hApprox, IDC_TESS_SPATIAL, TRUE);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_STEPS_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_EDGE_TEXT), true);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_DIST_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_ANGLE_TEXT), false);
		break;
	case TESS_CURVE:
        EnableWindow( GetDlgItem(hApprox, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_VIEW_DEP), TRUE);
		psEdgeSpin->Disable();
		psUSpin->Disable();
		CheckDlgButton( hApprox, IDC_TESS_CURV, TRUE);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_STEPS_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_EDGE_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_DIST_TEXT), true);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_ANGLE_TEXT), true);
		break;
	case TESS_LDA:
        EnableWindow( GetDlgItem(hApprox, IDC_ADVANCED), TRUE);
		EnableWindow( GetDlgItem(hApprox, IDC_TESS_VIEW_DEP), TRUE);
		psUSpin->Disable();
		CheckDlgButton( hApprox, IDC_TESS_LDA, TRUE);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_STEPS_TEXT), false);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_EDGE_TEXT), true);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_DIST_TEXT), true);
		EnableWindow (GetDlgItem (hApprox, IDC_SUBDIV_ANGLE_TEXT), true);
		break;
	}
}

class UIApproxRestore : public RestoreObj {
public:		
	EditTriObject *mpEO;
	TessApprox mApprox, mApproxR;
	bool mDoSubdiv, mDoSubdivR;
	bool mSplitMesh, mSplitMeshR;

    UIApproxRestore(EditTriObject *pEO);

    void Restore(int isUndo);
    void Redo();
};

UIApproxRestore::UIApproxRestore(EditTriObject *pEO)
{
	mpEO = pEO;
	mApprox = pEO->DisplacmentApprox();
	mDoSubdiv = pEO->DoSubdivisionDisplacment();
	mSplitMesh = pEO->SplitMeshForDisplacement();
}

void
UIApproxRestore::Restore(int isUndo)
{
	if (isUndo) {
		mApproxR = mpEO->DisplacmentApprox();
		mDoSubdiv = mpEO->DoSubdivisionDisplacment();
		mSplitMeshR = mpEO->SplitMeshForDisplacement();
	}
	mpEO->DisplacmentApprox() = mApprox;
	mpEO->DoSubdivisionDisplacment() = mDoSubdiv;
	mpEO->SplitMeshForDisplacement() = mSplitMesh;
	if (mpEO->hApprox) mpEO->UpdateApproxUI ();
}

void
UIApproxRestore::Redo()
{
	mpEO->DisplacmentApprox() = mApproxR;
	mpEO->DoSubdivisionDisplacment() = mDoSubdivR;
	mpEO->SplitMeshForDisplacement() = mSplitMeshR;
	if (mpEO->hApprox) mpEO->UpdateApproxUI ();
}






#define MAX_F 1000.0f
INT_PTR CALLBACK AdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

INT_PTR CALLBACK
DispApproxDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EditTriObject *eo = (EditTriObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);


	switch (msg) {
	case WM_INITDIALOG: {
		eo = (EditTriObject*)lParam;
		eo->hApprox = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		psUSpin = SetupIntSpinner( hWnd, IDC_TESS_U_SPINNER, IDC_TESS_U, 1, 100,
									eo->DisplacmentApprox().u);
		psEdgeSpin = SetupFloatSpinner( hWnd, IDC_TESS_EDGE_SPINNER, IDC_TESS_EDGE, 0.0f, MAX_F,
									eo->DisplacmentApprox().edge);
		psDistSpin = SetupFloatSpinner( hWnd, IDC_TESS_DIST_SPINNER, IDC_TESS_DIST, 0.0f, MAX_F,
									eo->DisplacmentApprox().dist);
		psAngSpin =  SetupFloatSpinner( hWnd, IDC_TESS_ANG_SPINNER,  IDC_TESS_ANG, 0.0f, 180.0f,
									eo->DisplacmentApprox().ang);
		CheckDlgButton(hWnd, IDC_TESS_VIEW_DEP, eo->DisplacmentApprox().view);
		eo->UpdateApproxUI ();
		break; }

	case CC_SPINNER_BUTTONDOWN:
		theHold.Begin();
		theHold.Put(new UIApproxRestore(eo));
		break;


    case CC_SPINNER_CHANGE:
		if (!HIWORD(wParam)) {
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
		}
		switch ( LOWORD(wParam) ) {
		case IDC_TESS_U_SPINNER:
			eo->DisplacmentApprox().u = psUSpin->GetIVal();
			break;
		case IDC_TESS_EDGE_SPINNER:
			eo->DisplacmentApprox().edge = psEdgeSpin->GetFVal();
			break;
		case IDC_TESS_DIST_SPINNER:
			eo->DisplacmentApprox().dist = psDistSpin->GetFVal();
			break;
		case IDC_TESS_ANG_SPINNER:
			eo->DisplacmentApprox().ang = psAngSpin->GetFVal();
			break;
		}
  		if (!HIWORD(wParam)) {
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
		}
      break;

	case CC_SPINNER_BUTTONUP:
		if (HIWORD(wParam)) {
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
		} else {
			theHold.Cancel();
		}
		break;

    case WM_COMMAND:
		switch ( LOWORD(wParam) ) {

		case IDC_DO_SUBDIV:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DoSubdivisionDisplacment() = IsDlgButtonChecked(hWnd, IDC_DO_SUBDIV)?true:false;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_SPLITMESH:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->SplitMeshForDisplacement() = IsDlgButtonChecked(hWnd, IDC_SPLITMESH)?true:false;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_PRESET1:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->SetDisplacmentApproxToPreset(0);
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_PRESET2:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->SetDisplacmentApproxToPreset(1);
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_PRESET3:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->SetDisplacmentApproxToPreset(2);
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;

		case IDC_TESS_REGULAR:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DisplacmentApprox().type = TESS_REGULAR;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_TESS_SPATIAL:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DisplacmentApprox().type = TESS_SPATIAL;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_TESS_CURV:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DisplacmentApprox().type = TESS_CURVE;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;
		case IDC_TESS_LDA:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DisplacmentApprox().type = TESS_LDA;
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			eo->UpdateApproxUI ();
			break;

		case IDC_TESS_VIEW_DEP:
			theHold.Begin();
			theHold.Put(new UIApproxRestore(eo));
			eo->DisplacmentApprox().view = IsDlgButtonChecked(hWnd, IDC_TESS_VIEW_DEP);
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			break;
		case IDC_ADVANCED: {
			sParams.mStyle = eo->DisplacmentApprox().subdiv;
			sParams.mMin = eo->DisplacmentApprox().minSub;
			sParams.mMax = eo->DisplacmentApprox().maxSub;
			sParams.mTris = eo->DisplacmentApprox().maxTris;
			int retval = eo->ip ? DialogBoxParam( hInstance,
						MAKEINTRESOURCE(IDD_DISP_APPROX_ADV),
						eo->ip->GetMAXHWnd(), AdvParametersDialogProc, (LPARAM)eo) : FALSE;
			if (retval == 1) {
				BOOL confirm = FALSE;
				if ((sParams.mStyle == SUBDIV_DELAUNAY && sParams.mTris > 200000) ||
					(sParams.mStyle != SUBDIV_DELAUNAY && sParams.mMax > 5)) {
					// warning!
					TSTR title = GetString(IDS_ADV_DISP_APPROX_WARNING_TITLE),
						warning = GetString(IDS_ADV_DISP_APPROX_WARNING);
					if (eo->ip && (MessageBox(eo->ip->GetMAXHWnd(), warning, title,
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 ) == IDYES))
						confirm = TRUE;
 
				} else
					confirm = TRUE;
				if (confirm) {
					theHold.Begin();
					theHold.Put(new UIApproxRestore(eo));
					eo->DisplacmentApprox().subdiv = sParams.mStyle;
					eo->DisplacmentApprox().minSub = sParams.mMin;
					eo->DisplacmentApprox().maxSub = sParams.mMax;
					eo->DisplacmentApprox().maxTris = sParams.mTris;
					theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
				}
			}
			break; }
		}
        break;

		
	case WM_DESTROY:
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
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), false);
			break;
		case SUBDIV_TREE:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), false);
			break;
		case SUBDIV_DELAUNAY:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			psMinSpin->Disable();
			psMaxSpin->Disable();
			psMaxTrisSpin->Enable();
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), false);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), false);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), true);
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
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), false);
			break;
		case IDC_TREE:
			sParams.mStyle = SUBDIV_TREE;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			psMinSpin->Enable();
			psMaxSpin->Enable();
			psMaxTrisSpin->Disable();
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), true);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), false);
			break;
		case IDC_DELAUNAY:
			sParams.mStyle = SUBDIV_DELAUNAY;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			psMinSpin->Disable();
			psMaxSpin->Disable();
			psMaxTrisSpin->Enable();
			EnableWindow (GetDlgItem (hDlg, IDC_MIN_SUBDIV_LEV_TEXT), false);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_LEV_TEXT), false);
			EnableWindow (GetDlgItem (hDlg, IDC_MAX_SUBDIV_TRIS_TEXT), true);
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
