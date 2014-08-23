/*===========================================================================*\
 | 
 |  FILE:	wM3_subdlg.cpp
 |			Weighted Morpher for MAX R3
 |			UI Handler and management code for minor dialogs
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 24-11-98
 | 
\*===========================================================================*/

#include "wM3.h"
#include "iColorMan.h"


/*===========================================================================*\
 |
 | Name Captured object page
 |
\*===========================================================================*/


INT_PTR CALLBACK NameDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;


	switch (msg) {
		case WM_INITDIALOG:{
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)mp);

			HWND maxWnd = mp->ip->GetMAXHWnd();
			CenterWindow(hWnd,maxWnd);
			EnableWindow(maxWnd,FALSE);

			mp->newname = GetICustEdit(GetDlgItem(hWnd,IDC_NEWNAME));
			mp->newname->SetText(GetString(IDS_CAPTURED));
			mp->newname->SetLeading(3);			

			SetFocus(GetDlgItem(hWnd,IDOK));

			break;
			}

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			ReleaseICustEdit(mp->newname);
			mp->newname = NULL;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					mp->hackUI = true;
					
					TCHAR nametoUse[256];
					mp->newname->GetText(nametoUse,255);

					mp->chanBank[mp->recordTarget].mName = nametoUse;

					EnableWindow(mp->ip->GetMAXHWnd(),TRUE);
					EndDialog(hWnd,1);
					
				break;

				case IDCANCEL:
					mp->hackUI = false;
					EndDialog(hWnd,1);
				break;
			}

			break;

		default:
			return FALSE;
	}
	return TRUE;
}



/*===========================================================================*\
 |
 | Legend page showing colour indicators
 |
\*===========================================================================*/


INT_PTR CALLBACK Legend_DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd,&ps);

			int q;
			for( q=0;q<5;q++ )
			{
				// Get next CI
				HWND item = GetDlgItem(hWnd,(IDC_IC1+q));
				Rect rect;
				GetClientRect(item,&rect);
				HDC hdc = GetDC(item);
				
				// choose the indicator colour
				COLORREF FilCol;
				FilCol = GetCustSysColor(COLOR_3DFACE);
				if(q==1) FilCol = RGB(255,160,0);
				if(q==2) FilCol = RGB(0,255,0);
				if(q==3) FilCol = RGB(10,10,255);
				if(q==4) FilCol = RGB(90,90,90);


				// Draw the button-y outline around the colour indicator
				Rect tR = rect;
				tR.right--;
				tR.bottom--;
				Rect3D(hdc,tR,FALSE);

				HPEN Pen = CreatePen( PS_SOLID , 1 , FilCol );
				SelectObject(hdc,Pen);
				LOGBRUSH BR;
				BR.lbStyle = BS_SOLID;
				BR.lbColor = FilCol;
				
				HBRUSH Brush = CreateBrushIndirect(&BR);
				SelectObject(hdc,Brush);


				Rectangle( hdc, 1,1,rect.w()-2,rect.h()-2);

				DeleteObject(Pen);
				DeleteObject(Brush);

			ReleaseDC(item,hdc);
			}
			
			EndPaint(hWnd,&ps);
		break;}


		case WM_INITDIALOG:{

			// Update the class pointer
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mp->hwLegend = hWnd;

			for(int i=0;i<5;i++)
			{
				LONG style = GetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE);
				style &= ~(SS_BLACKFRAME);
				style |= SS_OWNERDRAW;
				SetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE,style);
			}

			break;}


		default:
			return FALSE;
	}
	return TRUE;
}

/*===========================================================================*\
 |
 | Globals Page Rollout
 |
\*===========================================================================*/


INT_PTR CALLBACK Globals_DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;
	Interval valid=FOREVER;
	int tmp; float ntmp;

	switch (msg) {

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd,&ps);

			int q;
			for( q=0;q<4;q++ )
			{
				// Get next CI
				HWND item = GetDlgItem(hWnd,(IDC_IC1+q));
				Rect rect;
				GetClientRect(item,&rect);
				HDC hdc = GetDC(item);
				
				// choose the indicator colour
				COLORREF FilCol;
				FilCol = GetCustSysColor(COLOR_3DFACE);
				if(q==1) FilCol = RGB(255,160,0);
				if(q==2) FilCol = RGB(0,255,0);
				if(q==3) FilCol = RGB(10,10,255);


				// Draw the button-y outline around the colour indicator
				Rect tR = rect;
				tR.right--;
				tR.bottom--;
				Rect3D(hdc,tR,FALSE);

				HPEN Pen = CreatePen( PS_SOLID , 1 , FilCol );
				SelectObject(hdc,Pen);
				LOGBRUSH BR;
				BR.lbStyle = BS_SOLID;
				BR.lbColor = FilCol;
				
				HBRUSH Brush = CreateBrushIndirect(&BR);
				SelectObject(hdc,Brush);


				Rectangle( hdc, 1,1,rect.w()-2,rect.h()-2);

				DeleteObject(Pen);
				DeleteObject(Brush);

			ReleaseDC(item,hdc);
			}
			
			EndPaint(hWnd,&ps);

		break;}


		case WM_INITDIALOG:{

			// Update the class pointer
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mp->hwGlobalParams = hWnd;

			// Load the values from the pblockMat

			// MAX/MIN spinners
			float smin,smax;
			mp->pblock->GetValue(PB_OV_SPINMIN, mp->ip->GetTime(), smin, valid);
			mp->pblock->GetValue(PB_OV_SPINMAX, mp->ip->GetTime(), smax, valid);

			mp->glSpinmin = SetupFloatSpinner(hWnd, IDC_MIN_SPIN, IDC_MIN_EDIT, -999.9f, smax, smin);
			mp->glSpinmin->SetScale(mp->GetIncrements());

			mp->glSpinmax = SetupFloatSpinner(hWnd, IDC_MAX_SPIN, IDC_MAX_EDIT, smin, 999.9f, smax);
			mp->glSpinmax->SetScale(mp->GetIncrements());

			// USELIMITS checkbox
			mp->pblock->GetValue(PB_OV_USELIMITS, mp->ip->GetTime(), tmp, valid);
			SetCheckBox(hWnd,IDC_LIMIT, tmp );

			// USESEL button
			mp->pblock->GetValue(PB_OV_USESEL, mp->ip->GetTime(), tmp, valid);
			ICustButton *iTmp;
				iTmp = GetICustButton(GetDlgItem(hWnd,IDC_USESEL));
				iTmp->SetType(CBT_CHECK);
				iTmp->SetHighlightColor(BLUE_WASH);
				
				if(tmp) iTmp->SetCheck(TRUE);
				else iTmp->SetCheck(FALSE);

			ReleaseICustButton(iTmp);


			for(int i=0;i<4;i++)
			{
				LONG style = GetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE);
				style &= ~(SS_BLACKFRAME);
				style |= SS_OWNERDRAW;
				SetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE,style);
			}

			break;}


		case WM_DESTROY:
			ReleaseISpinner(mp->glSpinmin);
			ReleaseISpinner(mp->glSpinmax);
			break;



		// Spinner change handling
		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();
			switch (LOWORD(wParam)) {
				case IDC_MIN_SPIN: 
					{
						if (!HIWORD(wParam)) {
							theHold.Begin();
							theHold.Put( new Restore_Display( mp ) );
						}

						ntmp = mp->glSpinmin->GetFVal();
						mp->glSpinmax->SetLimits(ntmp,999.9f,FALSE);
						mp->pblock->SetValue( PB_OV_SPINMIN, 0, ntmp );

						if (!HIWORD(wParam)) {
							theHold.Put( new Restore_Display( mp ) );
							theHold.Accept(GetString(IDS_PC_MORPH));
						}
					}


					break;
				case IDC_MAX_SPIN: 
					if (!HIWORD(wParam)) {
						theHold.Begin();
						theHold.Put( new Restore_Display(mp) );
					}

					ntmp = mp->glSpinmax->GetFVal();
					mp->glSpinmin->SetLimits(-999.9f,ntmp,FALSE);
					mp->pblock->SetValue( PB_OV_SPINMAX, 0, ntmp );
					
					if (!HIWORD(wParam)) {
						theHold.Put( new Restore_Display( mp ) );
						theHold.Accept(GetString(IDS_PC_MORPH));
					}
				

					break;

			}
			break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;	

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PC_MORPH));
			else theHold.Cancel();

			// Update those limit changes
			mp->Update_channelLimits();
			break;



		case WM_COMMAND:
			switch (LOWORD(wParam)) {

			case IDC_USESEL:
				if (!theHold.Holding()) { 
					theHold.Begin();
					theHold.Put( new Restore_Display( mp ) );
				}

					ICustButton *iTmp;
						iTmp = GetICustButton(GetDlgItem(hWnd,IDC_USESEL));
						mp->pblock->SetValue(PB_OV_USESEL, 0, iTmp->IsChecked());
					ReleaseICustButton(iTmp);

				theHold.Put( new Restore_Display( mp ) );
				theHold.Accept(GetString(IDS_PC_MORPH));
				mp->Update_channelParams();
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());
				break;

			case IDC_LIMIT:
				if (!theHold.Holding()) { 
					theHold.Begin();
					theHold.Put( new Restore_Display( mp ) );
				}
					mp->pblock->SetValue(PB_OV_USELIMITS, 0, GetCheckBox(hWnd,IDC_LIMIT));
			
				theHold.Put( new Restore_Display( mp ) );
				theHold.Accept(GetString(IDS_PC_MORPH));
					mp->Update_channelLimits();
					mp->Update_channelParams();
				break;


			case IDC_ALL_ACTIVATE:
				{
					for(int i=0;i<100;i++) mp->chanBank[i].mActiveOverride = TRUE;
				//	mp->Update_colorIndicators();
					mp->Update_channelFULL();
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					mp->ip->RedrawViews(mp->ip->GetTime());
					break;}

			case IDC_ALL_DEACTIVATE:
				{
					for(int i=0;i<100;i++) mp->chanBank[i].mActiveOverride = FALSE;
				//	mp->Update_colorIndicators();
					mp->Update_channelFULL();
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					mp->ip->RedrawViews(mp->ip->GetTime());
					break;}


/*
			case IDC_LOAD:{
				int res = DialogBoxParam( 
					hInstance, 
					MAKEINTRESOURCE( IDD_MC_IMPORT ),
					hWnd,
					(DLGPROC)IMPORT_DlgProc,
					(LPARAM)(MorphR3*)mp);
				break;}


			case IDC_SAVE:{
				int res = DialogBoxParam( 
					hInstance, 
					MAKEINTRESOURCE( IDD_MC_EXPORT ),
					hWnd,
					(DLGPROC)EXPORT_DlgProc,
					(LPARAM)(MorphR3*)mp);
				break;}
*/


#ifndef NO_MTL_MORPHER	// russom - 03/18/02
			case IDC_MTLASSIGN:{
				if(mp->ip->GetSelNodeCount()==1 ) 
				{
					INode *node = mp->ip->GetSelNode(0);
					M3Mat *mmtl = (M3Mat*)CreateInstance(MATERIAL_CLASS_ID,M3MatClassID);
					// Instead of assigning the pointer, we need to replace the reference (tb)
					// mmtl->morphp = (MorphR3*)mp; 
					//
					mmtl->ReplaceReference(102,mp);

					mp->morphmaterial = mmtl;
					mmtl->obName = node->GetName();
					if(mp->CheckMaterialDependency() ) return TRUE;
					// Wire up our pblockMat to mtl
					for(int i=0;i<100;i++)
					{
						Control *c = mp->chanBank[i].cblock->GetController(0);
						mmtl->pblockMat->SetController(i,c);
					}

					node->SetMtl(mmtl);

					mmtl->BeginDependencyTest();
					mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
					if (mmtl->EndDependencyTest()) 
					{		
						//UNDO it all
						mmtl->morphp = NULL; mp->morphmaterial = NULL;
						mmtl->obName = "";
						for(int i=0;i<100;i++)
						{
							mmtl->pblockMat->SetController(i, (Control *)NULL );
						}
					}


					mp->ip->RedrawViews(mp->ip->GetTime());
				}
				else
				{
					MessageBox(hWnd,GetString(IDS_MTL_NOOBJ),GetString(IDS_CLASS_NAME),MB_OK);
				}

				break;}
#endif // NO_MTL_MORPHER

			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



/*===========================================================================*\
 |
 | Advanced Settings Rollout
 |
\*===========================================================================*/


INT_PTR CALLBACK Advanced_DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;
	Interval valid=FOREVER;
	int tmp;

	switch (msg) {
		case WM_INITDIALOG:{

			// Update the class pointer
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mp->hwAdvanced = hWnd;

			// VALUEINC radio button
			mp->pblock->GetValue(PB_AD_VALUEINC, mp->ip->GetTime(), tmp, valid);

			ICustButton *iTmp;
			int bIdx = 0;
			for( int i=IDC_V1; i<IDC_V3+1; i++)
			{
				iTmp = GetICustButton(GetDlgItem(hWnd,i));
				iTmp->SetType(CBT_CHECK);
				iTmp->SetHighlightColor(BLUE_WASH);
				
				if(bIdx==tmp) iTmp->SetCheck(TRUE);
				else iTmp->SetCheck(FALSE);

			ReleaseICustButton(iTmp);
			bIdx++;
			}
			mp->DisplayMemoryUsage();

			break;
			}

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_V1:{
					ICustButton *iTmp;
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V1));
					iTmp->SetCheck(TRUE);

					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V2));
					iTmp->SetCheck(FALSE);
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V3));
					iTmp->SetCheck(FALSE);
					ReleaseICustButton(iTmp);
					mp->pblock->SetValue(PB_AD_VALUEINC, 0, 0);

					// Now update the UIs
					mp->Update_SpinnerIncrements();

					break;}
			case IDC_V2:{
					ICustButton *iTmp;
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V2));
					iTmp->SetCheck(TRUE);

					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V1));
					iTmp->SetCheck(FALSE);
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V3));
					iTmp->SetCheck(FALSE);
					ReleaseICustButton(iTmp);
					mp->pblock->SetValue(PB_AD_VALUEINC, 0, 1);

					// Now update the UIs
					mp->Update_SpinnerIncrements();

					break;}
			case IDC_V3:{
					ICustButton *iTmp;
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V3));
					iTmp->SetCheck(TRUE);

					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V2));
					iTmp->SetCheck(FALSE);
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_V1));
					iTmp->SetCheck(FALSE);
					ReleaseICustButton(iTmp);
					mp->pblock->SetValue(PB_AD_VALUEINC, 0, 2);

					// Now update the UIs
					mp->Update_SpinnerIncrements();

					break;}


			// Compress all fringe channels into one solid block
			case IDC_COMPACT:{
					UI_MAKEBUSY

					Tab<int>	activeInd;
					activeInd.ZeroCount();

					// Using Restore_FullChannel for undo/redo now, no need to save the moved indices.
					// Tab<int>	movedsrc;		movedsrc.ZeroCount();
					// Tab<int>	movedtarg;		movedtarg.ZeroCount();

					int i,flag,numMoved=0;

					// Gather a list of all modified channels
					for(i=0;i<100;i++)
					{
						int n = i;
						if(mp->chanBank[i].mModded) activeInd.Append(1,&n,0);
					}

					
				theHold.SuperBegin();
				if (!theHold.Holding()) theHold.Begin();
					// For each modified channel, search the 100 channel entries
					// If it is empty, and lies below the source channel's index
					// Then copy. If not, leave it there.
					for(int x=0;x<activeInd.Count();x++)
					{
						for(i=0;i<100;i++)
						{
							if(!mp->chanBank[i].mModded) 
							{
								int targIdx = i;
								int srcIdx = activeInd[x];

								if(targIdx<srcIdx)
								{
									// movedsrc.Append(1,&srcIdx,0); movedtarg.Append(1,&targIdx,0);
									theHold.Put( new Restore_Display( mp ) );
									theHold.Put(new Restore_FullChannel(mp, targIdx, FALSE));
									theHold.Put(new Restore_FullChannel(mp, srcIdx, FALSE));
									mp->ChannelOp(i,activeInd[x],OP_MOVE);
									theHold.Put( new Restore_Display( mp ) );
									numMoved++;
									goto jumpStart;
								}
							}
						}
						jumpStart:
						flag = 0;
					}

					if (!theHold.Holding()) theHold.Begin();
					//theHold.Put(new Restore_CompactChannel(mp, movedtarg, movedsrc) );
					theHold.Accept(GetString (IDS_UNDO_COMPACT));
					theHold.SuperAccept (GetString (IDS_UNDO_COMPACT));

					mp->Update_channelFULL();
					mp->Update_channelParams();

					char s[50];
					sprintf(s,GetString(IDS_CHANNELS_MOVED),numMoved);
					SetWindowText(GetDlgItem(hWnd,IDC_COMPACTSTAT),s);

					UI_MAKEFREE
					break;}
			}

			break;

		default:
			return FALSE;
	}
	return TRUE;
}





INT_PTR CALLBACK ChannelOpDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:{
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)mp);

			HWND maxWnd = mp->ip->GetMAXHWnd();
			CenterWindow(hWnd,maxWnd);
			EnableWindow(maxWnd,FALSE);
			

			// Load active channels
			HWND clist = GetDlgItem(hWnd,IDC_CLIST);
			SendMessage(clist,LB_RESETCONTENT,0,0);

			for(int i=0;i<100;i++)
			{
				char tmp[255];
				int Cind = i+1;

				sprintf(tmp, "%i : %s", Cind, mp->chanBank[i].mName );
				SendMessage(clist,LB_ADDSTRING,0,(LPARAM)tmp);
			}


			// Set the 'perform operation' button to appropriate text
			HWND opWnd = GetDlgItem(hWnd,IDC_PERFORMOP);
			if(mp->cOp == OP_MOVE) SetWindowText(opWnd,GetString(IDS_OPMOVE));
			if(mp->cOp == OP_SWAP) SetWindowText(opWnd,GetString(IDS_OPSWAP));

			break;
			}

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:

			// Status of operation
			if ( HIWORD(wParam) == LBN_SELCHANGE ) {
				HWND clist = GetDlgItem(hWnd,IDC_CLIST);
				HWND statWnd = GetDlgItem(hWnd,IDC_STAT);
				int targIdx = SendMessage(clist,LB_GETCURSEL,0,0);

				SetWindowText(statWnd,GetString(IDS_OPOKAY));
				EnableWindow(GetDlgItem(hWnd,IDC_PERFORMOP),TRUE);
				if(mp->cOp == OP_MOVE)
				{
					if(mp->chanBank[targIdx].mModded) SetWindowText(statWnd,GetString(IDS_OPWARN));
				}
				if(targIdx == mp->srcIdx) 
				{
					SetWindowText(statWnd,GetString(IDS_OPCANT));
					EnableWindow(GetDlgItem(hWnd,IDC_PERFORMOP),FALSE);
				}
			}

			switch (LOWORD(wParam)) {
				case IDC_CANCEL:
					EnableWindow(mp->ip->GetMAXHWnd(),TRUE);
					EndDialog(hWnd,1);
					break;

				case IDC_PERFORMOP:
					int targIdx = SendMessage(GetDlgItem(hWnd,IDC_CLIST),LB_GETCURSEL,0,0);
					theHold.SuperBegin();
					if(!theHold.Holding()) theHold.Begin();
					theHold.Put( new Restore_Display( mp ) );
					theHold.Put(new Restore_FullChannel(mp, targIdx, FALSE));
					theHold.Put(new Restore_FullChannel(mp, mp->srcIdx, FALSE));
					mp->ChannelOp(targIdx,mp->srcIdx,mp->cOp);
					theHold.Put( new Restore_Display( mp ) );
					//theHold.Put(new Restore_FullChannel_Redo(mp, mp->srcIdx, FALSE));

					EnableWindow(mp->ip->GetMAXHWnd(),TRUE);
					EndDialog(hWnd,1);
					if(mp->cOp==OP_MOVE) {
						theHold.Accept(GetString (IDS_OPMOVE));
						theHold.SuperAccept(GetString (IDS_OPMOVE));
					}
					else {
						theHold.Accept(GetString (IDS_OPSWAP));
						theHold.SuperAccept(GetString (IDS_OPSWAP));
					}
					break;
			}

			break;

		default:
			return FALSE;
	}
	return TRUE;
}

