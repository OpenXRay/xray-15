// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
//	 FILE: dlgProcs.cpp
//
//	 DESCRIPTION: dialog procedures - class definitions
//
//	 CREATED BY: michael malone (mjm)
//
//	 HISTORY: created November 4, 1998
//
//	 Copyright (c) 1998, All Rights Reserved
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// precompiled header
#include "pch.h"

// local includes
#include "pch.h"
#include "dllMain.h"
#include "dlgProcs.h"
#include "blurMgr.h"

void MasterDlgProc::checkIn(int id, HWND hDlg)
{
	mHWnds[id] = hDlg;
	numCheckedIn++;
	mAllDlgsCreated = (numCheckedIn == numIDs-1) ? true : false; // idSelCurveCtrl not included
}

void MasterDlgProc::placeChildren()
{
	// compute locations for child dialogs
	Rect rc;
	GetWindowRect(mHWnds[idMaster], &rc);
	int originX = rc.left;
	int originY = rc.top;

	// find display area of tab control
	HWND hTabCtrl = GetDlgItem(mHWnds[idMaster], IDC_TAB);
	GetWindowRect(hTabCtrl, &rc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &rc);

	// convert to client coordinates
	rc.left -= originX; rc.right  -= originX;
	rc.top  -= originY; rc.bottom -= originY;

	// move child dialogs into place
	MoveWindow(mHWnds[idBlurData], rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
	MoveWindow(mHWnds[idSelData],  rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
}

void MasterDlgProc::notifyChildrenCreated(BlurMgr *blurMgr)
{
	assert(mAllDlgsCreated);
	mp_blurMgr = blurMgr;

	// setup tab control
	HWND hTabCtrl = GetDlgItem(mHWnds[idMaster], IDC_TAB);
	TC_ITEM tci;
	tci.mask = TCIF_TEXT;
	tci.iImage = -1;
	tci.pszText = GetString(IDS_BLUR_TYPE);
	TabCtrl_InsertItem(hTabCtrl, 0, &tci);
	tci.pszText = GetString(IDS_SEL_TYPES);
	TabCtrl_InsertItem(hTabCtrl, 1, &tci);

	// position child dialogs
	placeChildren();

	// show appropriate window
	switch( TabCtrl_GetCurSel( GetDlgItem(mHWnds[idMaster], IDC_TAB) ) )
	{
		case 0:
			ShowWindow(mHWnds[idSelData],  SW_HIDE);
			ShowWindow(mHWnds[idBlurData], SW_SHOW);
			break;
		case 1:
			ShowWindow(mHWnds[idBlurData], SW_HIDE);
			ShowWindow(mHWnds[idSelData],  SW_SHOW);
			break;
	}
	UpdateCurveControl();
}

void MasterDlgProc::UpdateCurveControl()
{
	assert(mp_blurMgr);
	ICurveCtl *pCCtl = mp_blurMgr->getCCtrl();
	assert(pCCtl);
	pCCtl->SetMessageSink(mHWnds[idSelData]);
	pCCtl->SetCustomParentWnd(GetDlgItem(mHWnds[idSelData], IDC_FALLOFF_CURVE));
	pCCtl->SetActive(TRUE);
}

void MasterDlgProc::ShowCurveControl(BOOL show)
{
	assert(mp_blurMgr);
	if ( mp_blurMgr->getCCtrl() )
		mp_blurMgr->getCCtrl()->SetActive(show);
}

void MasterDlgProc::SetThing(ReferenceTarget *m)
{
	// deactivate current curve control
	ShowCurveControl(FALSE);

	// parammap is being recycled - need to manually set the new blur's child paramblocks into the child parammaps
	mp_blurMgr = (BlurMgr *)m;
	mp_blurMgr->pmBlurData->SetParamBlock(mp_blurMgr->pbBlurData);
	mp_blurMgr->pmSelData->SetParamBlock(mp_blurMgr->pbSelData);

	// update the control of the new blur instance
	UpdateCurveControl();
}

void MasterDlgProc::DeleteThis()
{
	numCheckedIn = 0;
	mAllDlgsCreated = false;
	mHWnds[0] = mHWnds[1] = mHWnds[2] = NULL;
	mp_blurMgr = NULL;
}

BOOL MasterDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);

	switch(msg)
	{
		case WM_INITDIALOG:
			checkIn(idMaster, hWnd);
			return FALSE;

		case WM_NOTIFY:
		{
			NMHDR* pnmhdr = (LPNMHDR)lParam;
			HWND hTabCtrl = GetDlgItem(hWnd, IDC_TAB);

			switch(pnmhdr->code)
			{
				case TCN_SELCHANGING:
					switch(TabCtrl_GetCurSel(hTabCtrl))
					{
					case 0:
						hideWnd(idBlurData);
						break;
					case 1:
						hideWnd(idSelData);
//						ShowCurveControl(FALSE);
						break;
					}
					break;

				case TCN_SELCHANGE:
					switch(TabCtrl_GetCurSel(hTabCtrl))
					{
					case 0:
						showWnd(idBlurData);
						break;
					case 1:
						showWnd(idSelData);
//						ShowCurveControl(TRUE);
						break;
					}
					break;

				default:
					return FALSE;
			}
			break;
		}

		default:
			return FALSE;
	}
	return TRUE;
}


// ---------------
// BlurTypeDlgProc
// ---------------
void BlurDataDlgProc::enableControls(TimeValue t, IParamMap2* map, HWND hWnd)
{
	int blurType;
	BOOL state;

	map->GetParamBlock()->GetValue(prmBlurType, t, blurType, FOREVER);
	switch (blurType)
	{
		case idBlurUnif:
		case idBlurDir:
			EnableWindow(GetDlgItem(hWnd, IDB_BRADIAL_CLEAR_NODE), FALSE);
			break;
		case idBlurRadial:
			state = ( IsDlgButtonChecked(hWnd, IDC_BRADIAL_USE_NODE) != BST_CHECKED );
			map->Enable( prmRadialXOrig, state );
			map->Enable( prmRadialYOrig, state );
			map->Enable( prmRadialNode, !state );
			EnableWindow(GetDlgItem(hWnd, IDB_BRADIAL_CLEAR_NODE), !state);
			break;
	}
}

BOOL BlurDataDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			mpMasterDlgProc->checkIn(idBlurData, hWnd);
			map->SetTooltip( prmRadialNode, TRUE, GetString(IDS_BRADIAL_NODE_PROMPT) );
			break;

		case WM_SHOWWINDOW:
			enableControls(t, map, hWnd);
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDR_BLUR_UNIF:
				case IDR_BLUR_DIR:
					enableControls(t, map, hWnd);
					break;

				case IDR_BLUR_RADIAL:
				case IDC_BRADIAL_USE_OBJECT:
					enableControls(t, map, hWnd);
					break;

				case IDB_BRADIAL_CLEAR_NODE:
					if ( HIWORD(wParam) == BN_CLICKED )
						map->GetParamBlock()->SetValue(prmRadialNode, 0, (INode *)NULL); // not animatable -- use time = 0
					break;

				default:
					return FALSE;
			}
			break;

		case WM_DESTROY:
			GetCOREInterface()->UnRegisterDlgWnd(hWnd); // this should be handled automatically, but currently is not.
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


// --------------
// SelTypeDlgProc
// --------------
#define NUMCHANNELS 5

BOOL SelDataDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ICurve *pCurve;
	static int oldNumCurvePts = 0;
	int newNumCurvePts, ptIndex;
	CurvePoint cPt;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			mpMasterDlgProc->checkIn(idSelData, hWnd);

			// initialize strings for combo boxes
			_tcscpy( mChannelStr[0], GetString(IDS_RED) );
			_tcscpy( mChannelStr[1], GetString(IDS_GREEN));
			_tcscpy( mChannelStr[2], GetString(IDS_BLUE));
			_tcscpy( mChannelStr[3], GetString(IDS_ALPHA));
			_tcscpy( mChannelStr[4], GetString(IDS_LUMINANCE));

			// insert strings into listbox
			HWND hwndMap = GetDlgItem(hWnd, IDCB_SMASK_CHANNEL);  
			for (int index=0; index<NUMCHANNELS; index++)
				SendMessage( hwndMap, CB_ADDSTRING, 0, (LPARAM)mChannelStr[index] );
			break;
		}

		case WM_SHOWWINDOW:
		{
			int curIndex;
			map->GetParamBlock()->GetValue(prmMaskChannel, t, curIndex, FOREVER);
			SendMessage(GetDlgItem(hWnd, IDCB_SMASK_CHANNEL), CB_SETCURSEL, (WPARAM)curIndex, 0);

			// comboboxes not supported by parammap2 so manually set enable/disable for this control
			BOOL checked = ( SendMessage(GetDlgItem(hWnd, IDC_SMASK_ACTIVE), BM_GETCHECK, 0, 0) == BST_CHECKED ) ? TRUE : FALSE;
			EnableWindow(GetDlgItem(hWnd, IDCB_SMASK_CHANNEL), checked);

			BitArray ba = mpMasterDlgProc->getBlurMgr()->getCCtrl()->GetDisplayMode();
			CheckDlgButton(hWnd, IDC_SEL_BRT_CURVE,   (ba[0]) ? BST_CHECKED : BST_UNCHECKED );
			CheckDlgButton(hWnd, IDC_SEL_BLEND_CURVE, (ba[1]) ? BST_CHECKED : BST_UNCHECKED );
			break;
		}

		case WM_CC_SEL_CURVEPT:
			// a curve point was selected or deselected
			// lParam = ptr to the ICurve
			// LOWORD(wParam) = new # of selected points
			pCurve = (ICurve *)lParam;
			newNumCurvePts = LOWORD(wParam);
			DebugPrint("For curve at address: %p, ", pCurve);
			DebugPrint("a curve point was %s.\n", (newNumCurvePts > oldNumCurvePts) ? "selected" : "deselected");
			DebugPrint("There are now %d points selected.\n\n", newNumCurvePts);
			oldNumCurvePts = newNumCurvePts;
			break;

		case WM_CC_CHANGE_CURVEPT:
			// a curve point has changed
			// lParam = ptr to the ICurve
			// LOWORD(wParam) = zero based index of the changed point
			pCurve = (ICurve *)lParam;
			ptIndex = LOWORD(wParam);
			DebugPrint("For curve at address: %p, curve point %d has changed.\n", pCurve, ptIndex);
			cPt = pCurve->GetPoint(t, ptIndex);
			DebugPrint("The point's new value is: [%g, %g]\n\n", cPt.p.x, cPt.p.y);
			break;

		case WM_CC_CHANGE_CURVETANGENT:
			// a curve point's in or out tangent has changed
			// lParam = ptr to the ICurve
			// LOWORD(wParam) = zero based index of the changed point
			// HIWORD(wParam) = TRUE for InTangent or FALSE for OutTangent
			pCurve = (ICurve *)lParam;
			ptIndex = LOWORD(wParam);
			DebugPrint("For curve at address: %p, curve tangent %d has changed.\n", pCurve, ptIndex);
			cPt = pCurve->GetPoint(t, ptIndex);
			DebugPrint("The point's new in-tangent is: [%g, %g]\n", cPt.in.x, cPt.in.y);
			DebugPrint("The point's new out-tangent is: [%g, %g]\n", cPt.out.x, cPt.out.y);
			DebugPrint("The point's tangent flags are:\n");
			DebugPrint("  CURVEP_BEZIER   - %d\n", (cPt.flags && CURVEP_BEZIER)   ? 1 : 0);
			DebugPrint("  CURVEP_CORNER   - %d\n", (cPt.flags && CURVEP_CORNER)   ? 1 : 0);
			DebugPrint("  CURVEP_LOCKED_Y - %d\n", (cPt.flags && CURVEP_LOCKED_Y) ? 1 : 0);
			DebugPrint("  CURVEP_LOCKED_X - %d\n", (cPt.flags && CURVEP_LOCKED_X) ? 1 : 0);
			DebugPrint("  CURVEP_SELECTED - %d\n", (cPt.flags && CURVEP_SELECTED) ? 1 : 0);
			DebugPrint("  CURVEP_ENDPOINT - %d\n", (cPt.flags && CURVEP_ENDPOINT) ? 1 : 0);
			break;

		case WM_CC_DEL_CURVEPT:
			// a curve point was deleted
			// lParam = ptr to the ICurve
			// LOWORD(wParam) = zero based index of the deleted point
			pCurve = (ICurve *)lParam;
			ptIndex = LOWORD(wParam);
			DebugPrint("For curve at address: %p, curve point %d was deleted.\n", pCurve, ptIndex);
			break;

		case WM_COMMAND:
			if ( (LOWORD(wParam) == IDC_SMASK_ACTIVE) && (HIWORD(wParam) == BN_CLICKED) )
			{
				BOOL checked = ( SendMessage(GetDlgItem(hWnd, IDC_SMASK_ACTIVE), BM_GETCHECK, 0, 0) == BST_CHECKED ) ? TRUE : FALSE;
				EnableWindow(GetDlgItem(hWnd, IDCB_SMASK_CHANNEL), checked);
				break;
			}

			else if ( (LOWORD(wParam) == IDCB_SMASK_CHANNEL) && (HIWORD(wParam) == CBN_SELCHANGE) )
			{
				HWND hListBox = (HWND)lParam;
				int prevIndex(-1), index( SendMessage(hListBox, CB_GETCURSEL, 0, 0) );
				map->GetParamBlock()->GetValue(prmMaskChannel, GetCOREInterface()->GetTime(), prevIndex, FOREVER);
				if (index != prevIndex)
					map->GetParamBlock()->SetValue(prmMaskChannel, GetCOREInterface()->GetTime(), index);
				break;
			}

			else if ( (LOWORD(wParam) == IDC_SEL_BRT_CURVE) && (HIWORD(wParam) == BN_CLICKED) )
			{
				BOOL checked = ( SendMessage(GetDlgItem(hWnd, IDC_SEL_BRT_CURVE), BM_GETCHECK, 0, 0) == BST_CHECKED ) ? TRUE : FALSE;
				BitArray ba = mpMasterDlgProc->getBlurMgr()->getCCtrl()->GetDisplayMode();
				(checked) ? ba.Set(0) : ba.Clear(0);
				mpMasterDlgProc->getBlurMgr()->getCCtrl()->SetDisplayMode(ba);
				break;
			}

			else if ( (LOWORD(wParam) == IDC_SEL_BLEND_CURVE) && (HIWORD(wParam) == BN_CLICKED) )
			{
				BOOL checked = ( SendMessage(GetDlgItem(hWnd, IDC_SEL_BLEND_CURVE), BM_GETCHECK, 0, 0) == BST_CHECKED ) ? TRUE : FALSE;
				BitArray ba = mpMasterDlgProc->getBlurMgr()->getCCtrl()->GetDisplayMode();
				(checked) ? ba.Set(1) : ba.Clear(1);
				mpMasterDlgProc->getBlurMgr()->getCCtrl()->SetDisplayMode(ba);
				break;
			}

			else
				return FALSE;

		case WM_DESTROY:
			GetCOREInterface()->UnRegisterDlgWnd(hWnd); // this should be handled automatically, but currently is not.
			break;

		default:
			return FALSE;
	}
	return TRUE;
}
