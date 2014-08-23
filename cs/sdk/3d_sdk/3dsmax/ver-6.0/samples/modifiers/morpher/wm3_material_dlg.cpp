/*===========================================================================*\
 | 
 |  FILE:	wM3_material_dlg.cpp
 |			Weighted Morpher for MAX R3
 |			Morph Material dialog code
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 23-12-98
 | 
\*===========================================================================*/


#include "wM3.h"



static INT_PTR CALLBACK PanelDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	M3MatDlg *dlg = (M3MatDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (M3MatDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;
		}
	if (dlg) return dlg->WndProc(hWnd,msg,wParam,lParam);
	else return FALSE;
	}


M3MatDlg::M3MatDlg(HWND hwMtlEdit, IMtlParams *imp, M3Mat *m)
	{
	dadMgr.Init(this);

	hwmedit = hwMtlEdit;
	ip      = imp;
	theMtl  = m;
	valid   = FALSE;

	for (int i=0; i<NSUBMTL; i++) iBut[i] = NULL;
	bBut = NULL;
	pickBut = NULL;
	m->listSel = 0;

	hPanel = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_MORPHMTL),
		PanelDlgProc,
		GetString(IDS_MTL_PARAM),
		(LPARAM)this);
	}


int M3MatDlg::FindSubMtlFromHWND(HWND hw){
	for (int i=0; i<NSUBMTL; i++) {
		if (hw == iBut[i]->GetHwnd()) return i+theMtl->listSel;
		}
	if(hw == bBut->GetHwnd()) return 100;
	return -1;
	}

void M3MatDlg::UpdateSubMtlNames() {
	TSTR nm;
	HWND label;
	char s[30];

	for (int i=0; i<NSUBMTL; i++) {
		Mtl *m = theMtl->GetSubMtl(i+theMtl->listSel);
		if (m) 	nm = m->GetFullName();
		else 	nm = GetString(IDS_DS_NONE);
		iBut[i]->SetText(nm.data());

		label = GetDlgItem(hPanel,IDC_MNAME1+i);
		sprintf(s,GetString(IDS_MTL_MAPNAMEDOTS),i+1+theMtl->listSel);
		SetWindowText(label,s);
		}

		Mtl *mb = theMtl->GetSubMtl(100);
		if (mb) 	nm = mb->GetFullName();
		else 	nm = GetString(IDS_DS_NONE);
		bBut->SetText(nm.data());

	}



void M3MatDlg::DragAndDrop(int ifrom, int ito) {
	theMtl->CopySubMtl(hPanel,ifrom, ito);
	theMtl->NotifyChanged();
	}


M3MatDlg::~M3MatDlg()
	{
// rjc 21 may 2003 - when the dialog goes away, clear the pick mode
	if (theModPickmode.isPicking)
		ip->EndPickMode();

	theMtl->matDlg = NULL;	
	SetWindowLongPtr(hPanel, GWLP_USERDATA, NULL);
	for (int i=0; i<NSUBMTL; i++) {
		ReleaseICustButton(iBut[i]);
		iBut[i] = NULL; 
		}

	ReleaseICustButton(bBut);
	ReleaseICustButton(pickBut);
	bBut = NULL; 
	pickBut = NULL;

	}


void M3MatDlg::SetThing(ReferenceTarget *m) {
	theMtl = (M3Mat*)m;
	theMtl->matDlg= this;
	ReloadDialog();
	}

static int subMtlId[10] = {
IDC_MORPH_MAT1, IDC_MORPH_MAT2,
IDC_MORPH_MAT3, IDC_MORPH_MAT4, 
IDC_MORPH_MAT5, IDC_MORPH_MAT6, 
IDC_MORPH_MAT7, IDC_MORPH_MAT8, 
IDC_MORPH_MAT9, IDC_MORPH_MAT10 
};


// Clamp the channel number (0-90)
void M3MatDlg::Clamp_listSel()
{
	if(theMtl->listSel<0) theMtl->listSel = 0;
	if(theMtl->listSel>=90) theMtl->listSel = 90;
}


// Handle the scroll bar
void M3MatDlg::VScroll(int code, short int cpos ) {
	switch (code) {
		case SB_LINEUP: 	theMtl->listSel--;		break;
		case SB_LINEDOWN:	theMtl->listSel++;		break;
		case SB_PAGEUP:		theMtl->listSel -= 10;	break;
		case SB_PAGEDOWN:	theMtl->listSel += 10;	break;
		
		case SB_THUMBPOSITION: 
		case SB_THUMBTRACK:
			theMtl->listSel = cpos;
			break;
		}

	// Check for out-of-bounds values
	Clamp_listSel();
	
	// Reposition the scrollbar
	HWND vScr = GetDlgItem(hPanel,IDC_LISTSCROLL);
	SetScrollPos((HWND)vScr, SB_CTL, theMtl->listSel, TRUE); 

	ReloadDialog();
	}



void M3MatDlg::UpdateMorphInfo(int upFlag)
{
	macroRecorder->Disable();

	int i;

	if(theMtl->morphp)
	{
		theMtl->morphp->morphmaterial = theMtl;
		SetWindowText(GetDlgItem(hPanel,IDC_MORPHNAME),theMtl->obName);
		EnableWindow(GetDlgItem(hPanel,IDC_REFRESH),TRUE);
		EnableWindow(GetDlgItem(hPanel,IDC_MARKERLIST),TRUE);


		HWND hwMarker	= GetDlgItem(hPanel,IDC_MARKERLIST);
		SendMessage(hwMarker,CB_RESETCONTENT,0,0);

		// Add the bookmark names to the dropdown
		for( i=0; i<theMtl->morphp->markerName.Count(); i++ )
		{
			SendMessage(hwMarker,CB_ADDSTRING,0,(LPARAM) (LPCTSTR) theMtl->morphp->markerName[i]);
		}

		// Set the current selection
		SendMessage(hwMarker,CB_SETCURSEL ,(WPARAM)-1,0);

		for( i=0;i<100;i++)
		{
			if(theMtl->mTex[i]) theMtl->mTex[i]->SetName(theMtl->morphp->chanBank[i].mName);
		}

		if(upFlag==UD_LINK)
		{
			for( i=0;i<100;i++)
			{
				Control *c = theMtl->morphp->chanBank[i].cblock->GetController(0);
				theMtl->pblockMat->SetController(i,c);
			}
		}

	}
	else
	{
		SetWindowText(GetDlgItem(hPanel,IDC_MORPHNAME),GetString(IDS_MTL_NOTARG));
		EnableWindow(GetDlgItem(hPanel,IDC_REFRESH),FALSE);
		EnableWindow(GetDlgItem(hPanel,IDC_MARKERLIST),FALSE);

		HWND hwMarker	= GetDlgItem(hPanel,IDC_MARKERLIST);
		SendMessage(hwMarker,CB_RESETCONTENT,0,0);
	}

	macroRecorder->Enable();
}


BOOL M3MatDlg::WndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	int id = LOWORD(wParam);
	int notify = HIWORD(wParam);


	switch (msg) {
		case WM_INITDIALOG:{
			for (int i=0; i<NSUBMTL; i++) {
				iBut[i] = GetICustButton(GetDlgItem(hWnd,subMtlId[i]));
				iBut[i]->SetDADMgr(&dadMgr);
				}

				bBut = GetICustButton(GetDlgItem(hWnd,IDC_MORPH_BASE));
				bBut->SetDADMgr(&dadMgr);

				pickBut = GetICustButton(GetDlgItem(hWnd,IDC_BIND));
				pickBut->SetHighlightColor(GREEN_WASH);
				pickBut->SetType(CBT_CHECK);

			for (i=0; i<10; i++) 
				SetCheckBox(hWnd, IDC_TEXON1+i, theMtl->mapOn[i+theMtl->listSel]);

			
			// Setup the scroll bar to move the list
			HWND vScr = GetDlgItem(hWnd,IDC_LISTSCROLL);

			SCROLLINFO	si;
			memset(&si,0,sizeof(si)); 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_POS; 
			si.nMin   = 0; 
			si.nMax   = 90; 
			si.nPos   = theMtl->listSel; 
			SetScrollInfo((HWND)vScr, SB_CTL, &si, TRUE);    
			
			break;}

		case WM_PAINT:
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;


		// Handle the scrolling
		case WM_VSCROLL:
				VScroll(LOWORD(wParam),(short int)HIWORD(wParam));
			break;



		case WM_COMMAND:
			
			if (notify==CBN_SELENDOK){
				if(id==IDC_MARKERLIST){
					int mkSel = SendMessage(GetDlgItem(hWnd, IDC_MARKERLIST), CB_GETCURSEL, 0, 0);
					if(mkSel>=0){
						theMtl->listSel = theMtl->morphp->markerIndex[mkSel];
						Clamp_listSel();

						for (int i=0; i<10; i++) 
							SetCheckBox(hWnd, IDC_TEXON1+i, theMtl->mapOn[i+theMtl->listSel]);

						UpdateSubMtlNames();
	
						HWND vScr = GetDlgItem(hWnd,IDC_LISTSCROLL);
						SetScrollPos((HWND)vScr, SB_CTL, theMtl->listSel, TRUE); 
					}
				}
			}

			switch (id) {
				case IDC_MORPH_MAT1:
				case IDC_MORPH_MAT2:
				case IDC_MORPH_MAT3:
				case IDC_MORPH_MAT4:
				case IDC_MORPH_MAT5:
				case IDC_MORPH_MAT6:
				case IDC_MORPH_MAT7:
				case IDC_MORPH_MAT8:
				case IDC_MORPH_MAT9:
				case IDC_MORPH_MAT10:
					PostMessage(hwmedit,WM_SUB_MTL_BUTTON, 
						(id-IDC_MORPH_MAT1)+theMtl->listSel ,(LPARAM)theMtl);
					break;


				case IDC_MORPH_BASE:
					PostMessage(hwmedit,WM_SUB_MTL_BUTTON, 
						100 ,(LPARAM)theMtl);
					break;


				case IDC_BIND:{
						theModPickmode.mp = theMtl;
						ip->SetPickMode(&theModPickmode);
					break;}


				case IDC_UPDATE1:
				case IDC_UPDATE2:
				case IDC_UPDATE3:
					{
						int idU = id-IDC_UPDATE1;
						theMtl->pblockMat->SetValue(100,ip->GetTime(),idU);
						theMtl->NotifyChanged();
					break;}


				case IDC_REFRESH:
						ReloadDialog();
					break;


				case IDC_TEXON1:
				case IDC_TEXON2:
				case IDC_TEXON3:
				case IDC_TEXON4:
				case IDC_TEXON5:
				case IDC_TEXON6:
				case IDC_TEXON7:
				case IDC_TEXON8:
				case IDC_TEXON9:
				case IDC_TEXON10:
					theMtl->mapOn[(id-IDC_TEXON1)+theMtl->listSel] = GetCheckBox(hWnd,id);
					Invalidate();
					theMtl->NotifyChanged();
		    		ip->MtlChanged();
					break;
				}
			break;
		
		

		default:
			return FALSE;
		}
	return TRUE;
	}

void M3MatDlg::Invalidate()
	{
	valid = FALSE;
	InvalidateRect(hPanel,NULL,FALSE);	
	}

		
void M3MatDlg::ReloadDialog()
	{
	UpdateMorphInfo(UD_NORM);

	for (int i=0; i<10; i++) 
		SetCheckBox(hPanel, IDC_TEXON1+i, theMtl->mapOn[i+theMtl->listSel]);

	UpdateSubMtlNames();

	SetCheckBox(hPanel,IDC_UPDATE1,FALSE);
	SetCheckBox(hPanel,IDC_UPDATE2,FALSE);
	SetCheckBox(hPanel,IDC_UPDATE3,FALSE);

	int q;
	if(theMtl->pblockMat) theMtl->pblockMat->GetValue(100,ip->GetTime(),q,FOREVER);

	if(q==0) SetCheckBox(hPanel,IDC_UPDATE1,TRUE);
	if(q==1) SetCheckBox(hPanel,IDC_UPDATE2,TRUE);
	if(q==2) SetCheckBox(hPanel,IDC_UPDATE3,TRUE);

	}
