/*===========================================================================*\
 | 
 |  FILE:	wM3_dlg.cpp
 |			Weighted Morpher for MAX R3
 |			UI Handler and management code for main dialogs
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 22-5-98
 | 
\*===========================================================================*/

#include "wM3.h"
#include "iColorMan.h"

//WNDPROC prevTargetProc = NULL;
//HMENU menu = NULL;	

HIMAGELIST gMorphUpDownImages = NULL;
void SortSelectedProgressiveTarget( MorphR3 * morphObj );


/*
static LRESULT CALLBACK TargetListBoxProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	
	switch (msg) {

		case WM_INITDIALOG:

		break;

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			menu = NULL;
		break;

		case WM_RBUTTONDOWN:
			menu = GetSubMenu(LoadMenu(hInstance,MAKEINTRESOURCE(IDR_CHANSUB)),1);
			POINT lpPt; GetCursorPos(&lpPt);
			TrackPopupMenu(menu, TPM_RIGHTBUTTON, lpPt.x, lpPt.y, 0, hWnd, NULL);	
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam) )
			{
				case ID_MENU_TARGET_DELETED:
					SendMessage( GetParent( hWnd ), WM_COMMAND,  MAKEWPARAM(IDC_DELETE2, 0), 0 );
				break;

				case ID_MENU_TARGET_MOVEUP:
					if(mp) mp->SwapTargets(-1);
				break;

				case ID_MENU_TARGET_MOVEDOWN:
					if(mp) mp->SwapTargets(1);
				break;

				case ID_MENU_PICK_TARGET:
					thePickMode.mp  = mp;					
					mp->ip->SetPickMode(&thePickMode);
				break;
			}

		break;
			
	}

	return CallWindowProc(prevTargetProc, hWnd, msg, wParam, lParam);
}
*/

/*===========================================================================*\
 |
 | Channel List Rollout
 |
\*===========================================================================*/

class getMulti : public HitByNameDlgCallback
{
public:
	MorphR3 *mp;

	getMulti(MorphR3 *mpi) {mp=mpi;}

	TCHAR		*dialogTitle()			{ return GetString(IDS_LOADMULTI);	}
	TCHAR		*buttonText() 			{ return GetString(IDS_MULTIBTN);	}
	BOOL		singleSelect()			{ return FALSE;						}	
	BOOL		useFilter()				{ return TRUE;						}	
	BOOL		useProc()				{ return TRUE;						}
	BOOL		doCustomHilite()		{ return TRUE;						}
	BOOL		doHilite(INode *node)	{ return FALSE;						}
	BOOL		showHiddenAndFrozen()	{ return TRUE;						}

	void proc(INodeTab &nodeTab)
	{
		int i;
		Tab<int>	freeC;
		freeC.ZeroCount();


		// find available channels
		for(i=0;i<100;i++)
		{
			int tI = i;
			if(!mp->chanBank[i].mModded) freeC.Append(1,&tI,0);
		}

		// If there are less channels than incoming targets, tell the user and quit out
		if(nodeTab.Count()>freeC.Count())
		{
			TSTR buf1(GetString(IDS_NOFREE));
			TSTR buf2(GetString(IDS_CANNOT));
			MessageBox(mp->hwChannelList,buf1,buf2,MB_ICONSTOP | MB_OK);
			goto leapOut;
		}

		// Do the assignment of nodes
		for(i=0;i<nodeTab.Count();i++)
		{
			UI_MAKEBUSY

			if (theHold.Holding()) 
				theHold.Put(new Restore_FullChannel(mp, i ));

			mp->ReplaceReference(101+freeC[i],nodeTab[i]);
			mp->chanBank[freeC[i]].buildFromNode(nodeTab[i]);
			mp->chanBank[freeC[i]].ReNormalize();

			UI_MAKEFREE
		}

		leapOut:
		i=0;
	}

	int filter(INode *node)
	{
		Interval valid; 
		
		ObjectState os = node->GetObjectRef()->Eval(mp->ip->GetTime());

		if( os.obj->IsDeformable() == FALSE ) return FALSE;

		// Check for same-num-of-verts-count
		if( os.obj->NumPoints()!=mp->MC_Local.Count) return FALSE;

		node->BeginDependencyTest();
		mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest()) {		
			return FALSE;
		} 

		mp->BeginDependencyTest();
		node->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (mp->EndDependencyTest()) {		
			return FALSE;
		} 
		else return TRUE;
	}
};

// menu ID commands for dynamic popup
#define ID_PICKFROMSCENE 5
#define ID_DELETECHANNEL 6
#define ID_RELOADCHANNEL 7

INT_PTR CALLBACK ChannelList_DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;
	int id = LOWORD(wParam);
	int notify = HIWORD(wParam);


	switch (msg) {
	
		// Paint the little color boxes next to the buttons (color indicators)
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd,&ps);
			int q;
			for( q=0;q<10;q++ )
			{
				// Get next CI
				HWND item = GetDlgItem(hWnd,(IDC_I1+q));
				Rect rect;
				GetClientRect(item,&rect);
				HDC hdc = GetDC(item);
				
				// choose the indicator colour
				COLORREF FilCol;
				FilCol = GetCustSysColor(COLOR_3DFACE);
				if(mp->chanBank[q+mp->chanNum].mModded==TRUE) FilCol = RGB(255,160,0);
				if(mp->chanBank[q+mp->chanNum].mActive==TRUE) FilCol = RGB(0,255,0);
				if(mp->chanBank[q+mp->chanNum].mActive==TRUE&&mp->chanBank[q+mp->chanNum].mConnection==FALSE) FilCol = RGB(10,10,255);
	
				if(mp->chanBank[q+mp->chanNum].mInvalid==TRUE||mp->chanBank[q+mp->chanNum].mActiveOverride==FALSE) {
					FilCol = RGB(90,90,90);
				}


				// Draw the button-y outline around the colour indicator
				Rect tR = rect;
				tR.right--;
				tR.bottom--;
				if( q == mp->chanSel ) Rect3D(hdc,tR,TRUE);
				else Rect3D(hdc,tR,FALSE);


				HPEN Pen = CreatePen( PS_SOLID , 1 , FilCol );
				SelectObject(hdc,Pen);
				LOGBRUSH BR;
				BR.lbStyle = BS_SOLID;
				BR.lbColor = FilCol;
				
				HBRUSH Brush = CreateBrushIndirect(&BR);
				SelectObject(hdc,Brush);


				Rectangle( hdc, 1,1,rect.w()-2,rect.h()-2);

				// russom 07/13/01
				// d/e 298789 Peter Watje fix
			    SelectObject( hdc, GetStockObject(BLACK_PEN) );  //need to select an default object in before deleting
				DeleteObject(Pen);
			    SelectObject( hdc, GetStockObject(NULL_BRUSH));  //need to select an default object in before deleting
				DeleteObject(Brush);

			ReleaseDC(item,hdc);
			}

			EndPaint(hWnd,&ps);
		break;}

		case WM_INITDIALOG:{

			// Update the class pointer
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mp->hwChannelList = hWnd;


			ICustButton *iTmp; ICustEdit *eTmp;
			int i,tIdx = 0;

			// Assign button configuration to the channel buttons
			for( i=IDC_P1;i<IDC_P10+1;i++){
				iTmp = GetICustButton(GetDlgItem(hWnd,i));
				iTmp->SetType(CBT_CHECK);
#ifndef GAME_VER	// russom - 08/20/01
				iTmp->SetHighlightColor(RGB(210,210,210));
#else
				iTmp->SetHighlightColor(GetCustSysColor( COLOR_BTNHIGHLIGHT )); 
#endif
				iTmp->SetRightClickNotify(TRUE);

				if(tIdx==mp->chanSel) iTmp->SetCheck(TRUE);
				else iTmp->SetCheck(FALSE);

				iTmp->SetText(mp->chanBank[mp->chanNum+tIdx].mName);
				ReleaseICustButton(iTmp);
				tIdx++;
			}
			iTmp = NULL;


			// Print the channel range
			char s[15];
			sprintf(s, "%d - %d", mp->chanNum+1,mp->chanNum+10);
			SetWindowText(GetDlgItem(hWnd,IDC_PAGE),s);

			int itmp; Interval valid = FOREVER;

			mp->pblock->GetValue(PB_CL_AUTOLOAD, 0, itmp, valid);
			SetCheckBox(hWnd,IDC_AUTOLOAD,itmp?TRUE:FALSE);

			// Initialize the spinners
			// Get the value incrementation index, evaluate it and set the scale
			// when building the spinners
			float tmp; 
			for( i=0; i<10; i++ )
			{
				mp->chanBank[mp->chanNum+i].cblock->GetValue(0, mp->ip->GetTime(), tmp, valid);
				mp->chanSpins[i] = SetupFloatSpinner(hWnd, IDC_FS1+i, IDC_FE1+i, -999.9f, 999.9f, (float)tmp);
				mp->chanSpins[i]->SetScale(mp->GetIncrements());

				// Hack the leading values for the spinners (asthetics!)
				eTmp = GetICustEdit(GetDlgItem(hWnd,IDC_FE1+i));
				eTmp->SetLeading(1);
				ReleaseICustEdit(eTmp);
				eTmp=NULL;

				// Make those colour indicators ownerdraw only
				LONG style = GetWindowLongPtr(GetDlgItem(hWnd,IDC_I1+i),GWL_STYLE);
				style &= ~(SS_BLACKFRAME);
				style |= SS_OWNERDRAW;
				SetWindowLongPtr(GetDlgItem(hWnd,IDC_I1+i),GWL_STYLE,style);
			}


			// Setup the scroll bar to move the list
			HWND vScr = GetDlgItem(hWnd,IDC_LISTSCROLL);

			SCROLLINFO	si;
			memset(&si,0,sizeof(si)); 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_POS; 
			si.nMin   = 0; 
			si.nMax   = 90; 
			si.nPos   = mp->chanNum; 
			SetScrollInfo((HWND)vScr, SB_CTL, &si, TRUE);    

			// do the channel limits & markers
			mp->Update_channelLimits();
			mp->Update_channelMarkers();

			break;
			}

		case WM_DESTROY:
			{
			for( int i=0; i<10; i++ ) ReleaseISpinner(mp->chanSpins[i]);
			break;
			}

		// Handle the scrolling
		case WM_VSCROLL:
				mp->VScroll(LOWORD(wParam),(short int)HIWORD(wParam));
			break;

		// Spinner change handling
		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();
			switch (LOWORD(wParam)) {

				// user changes a channel value
				case IDC_FS1: 
				case IDC_FS2: 
				case IDC_FS3: 
				case IDC_FS4: 
				case IDC_FS5: 
				case IDC_FS6: 
				case IDC_FS7: 
				case IDC_FS8: 
				case IDC_FS9: 
				case IDC_FS10: 
					int tIdx = LOWORD(wParam) - IDC_FS1;
					mp->chanBank[mp->chanNum+tIdx].cblock->SetValue( 0, mp->ip->GetTime(), mp->chanSpins[tIdx]->GetFVal() );

					mp->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
					mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_INTERACTIVE,NULL);
					break;
			}
			break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_BEGIN,NULL);
			break;	

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PC_MORPH));
			else theHold.Cancel();
			mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_END,NULL);
			mp->Update_channelValues();
			break;



		case WM_COMMAND:
			switch(HIWORD(wParam)){

				// User clicks on marker window - disable or enable
				// the max keyboard accelerators
				case CBN_SETFOCUS :
					DisableAccelerators();
					break;
				case CBN_KILLFOCUS :
					EnableAccelerators();
					break;

				// User is changing a marker's name, or adding a new one
				case CBN_EDITCHANGE:
					mp->markerSel = -1;

					HWND hwMarker	= GetDlgItem(hWnd,IDC_MARKERLIST);
					HWND hwSave		= GetDlgItem(hWnd,IDC_SAVEMARKER);
					HWND hwDel		= GetDlgItem(hWnd,IDC_DELMARKER);

					BOOL sEn = FALSE;
					BOOL dEn = FALSE;

					char tm[256];
					GetWindowText(hwMarker,tm,255);
					if(stricmp(tm,"")!=0)
					{	
						sEn=TRUE;
						dEn=FALSE;
					}

					EnableWindow(hwSave,sEn);
					EnableWindow(hwDel,dEn);

					break;

			}

			if (notify==CBN_SELENDOK){
				if(id==IDC_MARKERLIST){
					int mkSel = SendMessage(GetDlgItem(hWnd, IDC_MARKERLIST), CB_GETCURSEL, 0, 0);
					if(mkSel>=0){
						mp->chanSel = 0;
						mp->chanNum = (mp->markerIndex[mkSel]);
						mp->markerSel = mkSel;

						// Refresh everything!
						mp->Update_channelFULL();
						mp->Update_channelParams();
					}
				}
			}

			switch (LOWORD(wParam)) {

			// Channel menu handlers
			// *---------------------------------------------*
			case ID_PICKFROMSCENE:
				thePickMode.mp  = mp;					
				mp->ip->SetPickMode(&thePickMode);
				break;

			case ID_DELETECHANNEL:{
					if (!theHold.Holding()) theHold.Begin();
					theHold.Put(new Restore_FullChannel(mp, mp->CurrentChannelIndex() ));
					
					mp->DeleteChannel( mp->CurrentChannelIndex() );
					theHold.Accept(GetString(IDS_UNDOMCDEL));

					// Refresh system
					mp->Update_channelFULL();
					mp->Update_channelParams();	
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
					mp->ip->RedrawViews(mp->ip->GetTime());
				break;}


			case ID_RELOADCHANNEL:
			{
				morphChannel &bank = mp->CurrentChannel();
				
				UI_MAKEBUSY

				if(bank.mConnection)
				{
					bank.buildFromNode(bank.mConnection);
				}
				else if(bank.mActive)
				{
					bank.rebuildChannel();
				}
				
				for(int i=0; i<bank.mNumProgressiveTargs; i++) { 
					bank.mTargetCache[i].Init(bank.mTargetCache[i].mTargetINode); }

				mp->NotifyDependents(FOREVER,PART_OBJ,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());

				UI_MAKEFREE
				break;}

			// *---------------------------------------------*


			// Save a channel list marker
			case IDC_SAVEMARKER:
				{
					HWND hwMarker	= GetDlgItem(hWnd,IDC_MARKERLIST);
					char tm[256];
					GetWindowText(hwMarker,tm,255);

					mp->markerName.AddName(tm);

					int tmp;
					tmp = mp->chanNum;
					mp->markerIndex.Append(1,&tmp,0);

					mp->markerSel = mp->markerName.Count()-1;
					mp->Update_channelMarkers();
				}
				break;
			// Delete a channel list marker
			case IDC_DELMARKER:
				{
					if (!theHold.Holding()) 
					{
						theHold.Begin();
						theHold.Put(new Restore_Marker(mp));
						theHold.Accept(GetString(IDS_DEL_MARKER));
					}
					mp->markerName.Delete(mp->markerSel,1);
					mp->markerIndex.Delete(mp->markerSel,1);
					mp->Update_channelMarkers();
				}
				break;



			case IDC_LOADMULT:
				{
				getMulti ChooseMultiple((MorphR3*)mp);
				if (!theHold.Holding()) 
					theHold.Begin();

				mp->ip->DoHitByNameDialog(&ChooseMultiple);

				theHold.Accept(GetString(IDS_PC_MORPH));

				// Refresh system
				mp->Update_channelFULL();
				mp->Update_channelParams();	
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
				mp->ip->RedrawViews(mp->ip->GetTime());

				break;}


			case IDC_UPDATETARGS:
				{

				UI_MAKEBUSY

				theHold.Begin();

				for(int i=0;i<100;i++)
				{
					if( mp->chanBank[i].mConnection || mp->chanBank[i].mActive ) {
						theHold.Put(new Restore_FullChannel(mp, i, FALSE));
					}
					if(mp->chanBank[i].mConnection)
					{
						mp->chanBank[i].buildFromNode(mp->chanBank[i].mConnection);
						mp->Update_colorIndicators();
					}
					else if(mp->chanBank[i].mActive)
					{
						mp->chanBank[i].rebuildChannel();
					}
					for(int j=0; j<mp->chanBank[i].mNumProgressiveTargs; j++) { 
						mp->chanBank[i].mTargetCache[j].Init(mp->chanBank[i].mTargetCache[j].mTargetINode); }
				}

				theHold.Accept (GetString (IDS_UNDO_RELOAD));

				mp->NotifyDependents(FOREVER,PART_OBJ,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());

				UI_MAKEFREE

				break;}


			case IDC_AUTOLOAD:
				{
					mp->pblock->SetValue(PB_CL_AUTOLOAD, 0, GetCheckBox(hWnd,IDC_AUTOLOAD));
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				break;}


			case IDC_ZEROOTHERS:{
				int i;
				TimeValue t = mp->ip->GetTime();
				theHold.Begin();
					for(i=0;i<100;i++)
					{
						if(mp->chanBank[i].mModded&&mp->chanBank[i].mActiveOverride) mp->chanBank[i].cblock->SetValue(0,t,0.0f);
					}

				mp->Update_channelFULL();
				mp->Update_channelParams();	
				mp->NotifyDependents(FOREVER,PART_OBJ,REFMSG_CHANGE);
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
				mp->ip->RedrawViews(mp->ip->GetTime());
				theHold.Accept(GetString(IDS_PC_MORPH));
				break;}


			// user clicks on a channel button
			case IDC_P1:
			case IDC_P2:
			case IDC_P3:
			case IDC_P4:
			case IDC_P5:
			case IDC_P6:
			case IDC_P7:
			case IDC_P8:
			case IDC_P9:
			case IDC_P10:{

				mp->chanSel = LOWORD(wParam) - IDC_P1;

				// Just rebuild all the buttons now
				int tIdx = 0,i;
				ICustButton *iTmp;
				for( i=IDC_P1;i<IDC_P10+1;i++){
					iTmp = GetICustButton(GetDlgItem(hWnd,i));
					if(tIdx==mp->chanSel) iTmp->SetCheck(TRUE);
					else iTmp->SetCheck(FALSE);
					ReleaseICustButton(iTmp);
					tIdx++;
				}
				mp->Update_channelParams();
				mp->Update_colorIndicators();

				if(!thePickMode.isPicking)
				{
					ICustEdit *cName = GetICustEdit(GetDlgItem(mp->hwChannelParams,IDC_CHANNAME));
					cName->GiveFocus();
					ReleaseICustEdit(cName);
				}

				// User has right-clicked on channel buttton
				// Do a mini popup menu that lets them
				// put an object into a channel or delete it
				if (HIWORD(wParam)==BN_RIGHTCLICK) {

					HMENU hMCM = CreatePopupMenu();
					MENUITEMINFO minfo;
					memset(&minfo,sizeof(minfo),0);
					minfo.cbSize = sizeof(minfo);

					minfo.fMask = MIIM_TYPE|MIIM_ID|MIIM_STATE;
					minfo.fType = MFT_STRING;

					minfo.wID = ID_PICKFROMSCENE;
					minfo.dwTypeData = GetString(IDS_MENUITEM1_PS);
					minfo.fState = thePickMode.isPicking?MFS_CHECKED:MFS_UNCHECKED;
					InsertMenuItem(hMCM,1,TRUE,&minfo);

					// If channel == active, allow deletion
					if(mp->chanBank[mp->chanNum+mp->chanSel].mActive)
					{
						minfo.wID = ID_DELETECHANNEL;
						minfo.dwTypeData = GetString(IDS_MENUITEM2_DC);
						minfo.fState = MFS_UNCHECKED;
						InsertMenuItem(hMCM,2,TRUE,&minfo);
					}

					// If Node == alive, allow reload
					if(mp->chanBank[mp->chanNum+mp->chanSel].mConnection)
					{
						minfo.wID = ID_RELOADCHANNEL;
						minfo.dwTypeData = GetString(IDS_MENUITEM3_RT);
						minfo.fState = MFS_UNCHECKED;
						InsertMenuItem(hMCM,3,TRUE,&minfo);
					}


					// Show our popup
					POINT lpPt; GetCursorPos(&lpPt);
					TrackPopupMenu(hMCM, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
						lpPt.x, lpPt.y, 0, hWnd, NULL);	

					DestroyMenu(hMCM);
				}

			break;}

			}

			break;


		default:
			return FALSE;
	}
	return TRUE;
}

/*===========================================================================*\
 |
 | Channel Parameters Rollout
 |
\*===========================================================================*/


INT_PTR CALLBACK ChannelParams_DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MorphR3 *mp = (MorphR3*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;

	ICustEdit *cName = NULL;
	float xtmp,ntmp;

	switch (msg) {
		case WM_INITDIALOG:{

			HBITMAP hBitmap, hMask;
			if (!gMorphUpDownImages) {
				gMorphUpDownImages = ImageList_Create(16, 15, ILC_MASK, 5, 0);
				hBitmap          = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MORPHPARAMS_ICON));
				hMask            = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MORPHPARAMS_MASK));
				ImageList_Add(gMorphUpDownImages,hBitmap,hMask);
				DeleteObject(hBitmap);
				DeleteObject(hMask);
			}

			ICustToolbar *pUpDownToolbar;
			
			pUpDownToolbar = GetICustToolbar(GetDlgItem(hWnd, IDC_UP_DOWN));

            pUpDownToolbar->SetImage(gMorphUpDownImages);
            pUpDownToolbar->AddTool(ToolButtonItem(CTB_PUSHBUTTON,0,0,2,2,
                                                   16,15,24,23,
                                                   IDC_TARGET_UP ) );
            pUpDownToolbar->AddTool(ToolButtonItem(CTB_PUSHBUTTON,1,1,3,3,
                                                   16,15,24,23,
                                                   IDC_TARGET_DOWN ) );
            
            ReleaseICustToolbar(pUpDownToolbar);

			// Update the class pointer
			mp = (MorphR3*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mp->hwChannelParams = hWnd;

//			prevTargetProc = (WNDPROC)SetWindowLongPtr(GetDlgItem( hWnd, IDC_TARGETLIST ), GWLP_WNDPROC, (LONG_PTR)TargetListBoxProc);
			SetWindowLongPtr(GetDlgItem( hWnd, IDC_TARGETLIST ),GWL_USERDATA, (INT_PTR)mp);
			
			cName = GetICustEdit(GetDlgItem(hWnd,IDC_CHANNAME));
			cName->SetLeading(2);

			// load the current channel
			morphChannel *curTmp = &mp->chanBank[mp->chanNum+mp->chanSel];

			// Setup the min/max stuff
			mp->cSpinmin = SetupFloatSpinner(hWnd, IDC_MIN_SPIN, IDC_MIN_EDIT, -999.9f, 999.9f, curTmp->mSpinmin);
			mp->cSpinmin->SetScale(mp->GetIncrements());
			mp->cSpinmax = SetupFloatSpinner(hWnd, IDC_MAX_SPIN, IDC_MAX_EDIT, -999.9f, 999.9f, curTmp->mSpinmax);
			mp->cSpinmax->SetScale(mp->GetIncrements());

			mp->cSpinmin->SetLimits(-999.9f,curTmp->mSpinmax,FALSE);
			mp->cSpinmax->SetLimits(curTmp->mSpinmin,999.9f,FALSE);

			SetCheckBox(hWnd,IDC_LIMIT, curTmp->mUseLimit );

			// Use Channel button
			SetCheckBox(hWnd,IDC_USECHAN, curTmp->mActiveOverride );

			mp->cCurvature = SetupFloatSpinner(hWnd, IDC_CURVE_EDIT_SPIN, IDC_CURVE_EDIT, 
				0.0f, 1.0f, mp->chanBank[mp->CurrentChannelIndex()].mCurvature, 0.01f);

			mp->cTargetPercent = SetupFloatSpinner(hWnd, IDC_TARGET_PERCENT_SPIN, IDC_TARGET_PERCENT_EDIT, 
				0.0f, 100.0f, mp->GetCurrentTargetPercent(), 0.01f);


			// Use selection button
			ICustButton *iTmp;

			iTmp = GetICustButton(GetDlgItem(hWnd,IDC_USESEL));
			iTmp->SetType(CBT_CHECK);
			iTmp->SetHighlightColor(BLUE_WASH);
			ReleaseICustButton(iTmp);
				

			// channel assignment buttons
			iTmp = GetICustButton(GetDlgItem(hWnd,IDC_PICK));
			iTmp->SetType(CBT_CHECK);
			iTmp->SetHighlightColor(GREEN_WASH);
			iTmp = GetICustButton(GetDlgItem(hWnd,IDC_MAKE));
			iTmp->SetType(CBT_PUSH);
			ReleaseICustButton(iTmp);
			
			mp->Update_channelParams();

			break;}


		case WM_DESTROY:
			ReleaseICustEdit(cName);
			ReleaseISpinner(mp->cSpinmin); mp->cSpinmin = NULL;	
			ReleaseISpinner(mp->cSpinmax); mp->cSpinmax = NULL;
			ReleaseISpinner(mp->cCurvature); mp->cCurvature = NULL;
			ReleaseISpinner(mp->cTargetPercent); mp->cTargetPercent = NULL;
//			prevTargetProc = NULL;
		break;
		

		// Spinner change handling
		// Knock the channel into Modded if it isn't already
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_MIN_SPIN: 
					{
						if (!HIWORD(wParam)) {
							theHold.Begin();
							theHold.Put( new Restore_FullChannel(mp, mp->CurrentChannelIndex()) );
						}

						ntmp = mp->cSpinmin->GetFVal();
						mp->cSpinmax->SetLimits(ntmp,999.9f,FALSE);
						mp->chanBank[mp->chanNum+mp->chanSel].mSpinmin = ntmp;

						if (mp->chanBank[mp->chanNum+mp->chanSel].mModded == FALSE)
						{
							mp->chanBank[mp->chanNum+mp->chanSel].mModded = TRUE;
							mp->Update_colorIndicators();
						}

						if (!HIWORD(wParam))
							theHold.Accept(GetString(IDS_PC_MORPH));
					}
					break;
				case IDC_MAX_SPIN: 
					{
						if (!HIWORD(wParam)) {
							theHold.Begin();
							theHold.Put( new Restore_FullChannel(mp, mp->CurrentChannelIndex()) );
						}

						xtmp = mp->cSpinmax->GetFVal();
						mp->cSpinmin->SetLimits(-999.9f,xtmp,FALSE);
						mp->chanBank[mp->chanNum+mp->chanSel].mSpinmax = xtmp;

						if (mp->chanBank[mp->chanNum+mp->chanSel].mModded == FALSE)
						{
							mp->chanBank[mp->chanNum+mp->chanSel].mModded = TRUE;
							mp->Update_colorIndicators();
						}

						if (!HIWORD(wParam))
							theHold.Accept(GetString(IDS_PC_MORPH));

						}
					break;

				case IDC_CURVE_EDIT_SPIN:
					{
					if (!HIWORD(wParam)) {
						theHold.Begin();
						theHold.Put( new Restore_FullChannel(mp, mp->CurrentChannelIndex()) );
					}

					mp->chanBank[mp->CurrentChannelIndex()].mCurvature = mp->cCurvature->GetFVal();

					if (!HIWORD(wParam)) {
						theHold.Put( new Restore_Display( mp ) );
						theHold.Accept(GetString(IDS_PC_MORPH));
					}

 					mp->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
					mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_INTERACTIVE,NULL);
					}
				break;

				case IDC_TARGET_PERCENT_SPIN:
					{
					if (!HIWORD(wParam)) {
						theHold.Begin();
						theHold.Put( new Restore_FullChannel(mp, mp->CurrentChannelIndex()) );
					}

					mp->SetCurrentTargetPercent( mp->cTargetPercent->GetFVal() );

					if (!HIWORD(wParam)) {
						theHold.Put( new Restore_Display( mp ) );
						theHold.Accept(GetString(IDS_PC_MORPH));
					}

					mp->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
					SortSelectedProgressiveTarget( mp );
					mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_INTERACTIVE,NULL);
					}
				break;
			}
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PC_MORPH));
			break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			theHold.Put( new Restore_FullChannel(mp, mp->CurrentChannelIndex()) );
			break;	

		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PC_MORPH));
			else theHold.Cancel();

			// Update those limit changes
			mp->Update_channelLimits();				

			break;


		case WM_CUSTEDIT_ENTER:
			switch (LOWORD(wParam)) {
				case IDC_CHANNAME:{ 
					TCHAR strTmp[256];
					ICustEdit *icT = GetICustEdit(GetDlgItem(hWnd,IDC_CHANNAME));
					icT->GetText( strTmp, 255 );

					if(strcmp(strTmp,mp->chanBank[mp->chanNum+mp->chanSel].mName)!=0)
					{
						// Set channel name and channel 'dirty/changed' bit
						mp->chanBank[mp->chanNum+mp->chanSel].mName = TSTR(strTmp);
						mp->chanBank[mp->chanNum+mp->chanSel].mModded = TRUE;
					}
					icT->SetText( strTmp );

					ReleaseICustEdit(icT);
					mp->Update_channelNames();
					mp->Update_colorIndicators();
					break;}

				default:
					{
					if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PC_MORPH));
					else theHold.Cancel();

					// Update those limit changes
					mp->Update_channelLimits();
					break;
					}
				}
			break;


		case WM_COMMAND:{

			// Jump to channel
			int idCode = LOWORD(wParam);
			if(idCode>=5000&&idCode<=5200)
			{
				int id = idCode-5000;

				mp->chanNum = id;
				mp->Clamp_chanNum();

				int difSel = mp->chanSel + (id-(mp->chanNum + mp->chanSel));

				mp->chanSel = difSel;

				// Rebuild all the buttons
				int tIdx = 0,i;
				ICustButton *iTmp;
				for( i=IDC_P1;i<IDC_P10+1;i++){
					iTmp = GetICustButton(GetDlgItem(mp->hwChannelList,i));
					if(tIdx==mp->chanSel) iTmp->SetCheck(TRUE);
					else iTmp->SetCheck(FALSE);
					ReleaseICustButton(iTmp);
					tIdx++;
				}
				mp->Update_channelParams();
				mp->Update_colorIndicators();

				ICustEdit *cName = GetICustEdit(GetDlgItem(mp->hwChannelParams,IDC_CHANNAME));
				cName->GiveFocus();
				ReleaseICustEdit(cName);

				mp->Update_channelFULL();
				mp->Update_channelParams();
			}



			switch (LOWORD(wParam)) {

			case IDC_TARGETLIST:
				mp->CurrentChannel().iTargetListSelection = 
					SendDlgItemMessage( hWnd, IDC_TARGETLIST, LB_GETCURSEL, 0, 0);
				if(mp->cTargetPercent) mp->cTargetPercent->SetValue(mp->GetCurrentTargetPercent(), FALSE);
			break;

			case IDC_TARGET_DOWN:
				mp->SwapTargets(1);
			break;

			case IDC_TARGET_UP:
				mp->SwapTargets(-1);
			break;
				

			// Support the channel management dialog
			case ID_CSM_SWAP:
			case ID_CSM_MOVE:
				{
					mp->cOp = LOWORD(wParam)-ID_CSM_MOVE;
					mp->srcIdx = (mp->chanNum+mp->chanSel);
					int res = DialogBoxParam( 
					hInstance, 
					MAKEINTRESOURCE( IDD_CHANOP ),
					hWnd,
					(DLGPROC)ChannelOpDlgProc,
					(LPARAM)(MorphR3*)mp);

					mp->Update_channelFULL();
					mp->Update_channelParams();

			break;}



			case IDC_MAKE:{ 
					
						mp->hackUI = false;

						mp->recordTarget = mp->chanNum+mp->chanSel;
						mp->chanBank[mp->recordTarget].mActive = TRUE;
						mp->chanBank[mp->recordTarget].mModded = TRUE;
						mp->chanBank[mp->recordTarget].mInvalid = FALSE;

					int res = DialogBoxParam( hInstance,  MAKEINTRESOURCE( IDD_NAMECAP ),
						hWnd, (DLGPROC)NameDlgProc, (LPARAM)(MorphR3*)mp);

					if(mp-> hackUI)
					{
						UI_MAKEBUSY
						mp->recordModifications = TRUE;
											

						mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
						mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
						mp->ip->RedrawViews(mp->ip->GetTime());

						mp->Update_channelFULL();
						mp->Update_channelParams();
						UI_MAKEFREE
					}
					else
					{
						mp->recordModifications = FALSE;
						mp->chanBank[mp->recordTarget].mActive = FALSE;
						mp->chanBank[mp->recordTarget].mModded = FALSE;
						mp->chanBank[mp->recordTarget].mInvalid = TRUE;
					}

					mp->hackUI = false;

					
					break;
				}


			case IDC_CHANNUM:{

					HMENU hCi = CreatePopupMenu();
					MENUITEMINFO minfo;
					memset(&minfo,sizeof(minfo),0);
					minfo.cbSize = sizeof(minfo);

					minfo.fMask = MIIM_TYPE|MIIM_ID;
					minfo.fType = MFT_STRING;

					int mID = 0;
					char s[255];

					for(int x=0;x<100;x++)
					{
						if(mp->chanBank[x].mModded)
						{
							sprintf(s,"[%i]  %s",(x+1),mp->chanBank[x].mName);
							minfo.wID = 5000+x;
							minfo.dwTypeData = s;
							InsertMenuItem(hCi,mID,TRUE,&minfo);
							mID++;
						}
					}

					HMENU mainMenu = GetSubMenu(LoadMenu(hInstance,MAKEINTRESOURCE(IDR_CHANSUB)),0);

					AppendMenu(mainMenu, MF_BYCOMMAND | MF_POPUP, (UINT)hCi,GetString(IDS_ACTIVE));

					// Show our popup
					POINT lpPt; GetCursorPos(&lpPt);
					TrackPopupMenu(mainMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
						lpPt.x, lpPt.y, 0, hWnd, NULL);	

					break;}



			// Another monster hack. This time, i gotta somehow extract a list of points and 
			// apply those deltas to the current object, at the point in the stack just below
			// the current modifier. 
			//
			// This is what i do:
			//	[] Get the current INode from calling a GetSelNode. This relies on the UI
			//		only ever being open when the mod is selected, and therefore there will
			//		only ever be one object legally selected.
			//
			//	[] Get the derived object, and start traversing the WSM modifier list.
			//
			//	[] As i cycle through 'em, store the enabled state, and turn 'em off.
			//
			//	[] When i reach myself, STOP! I want to disable myself, of course.
			//
			//  [] Invalidate the stack, causing the node to rebuild. Then, take a snapshot
			//		of the state. (ie, up to the point in the stack just below the morpher)
			//
			//	[] We then have a new Object and therefore a buildable copy. 
			//
			//  [] Apply the current channel's mPoints array. Build a new node from this.
			//
			//	[] Recycle through the mod list, appling the saved enabled states to the mods.
			//
			//  [] clean up!
			//
			// And it seems to work! :o)
			//
			case IDC_EXTRACT:{
				UI_MAKEBUSY

				if(mp->ip->GetSelNodeCount()==1) 
				{
					INode *node = mp->ip->GetSelNode(0);

					int cIndex = mp->chanNum+mp->chanSel;
					TimeValue t = mp->ip->GetTime();

					Object *pObj = node->GetObjectRef();
					IDerivedObject *pDerObj = NULL;
					Modifier *pMod = NULL;

					if( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) 
					{
						pDerObj = (IDerivedObject *) pObj;

						// for saving enabled state of modifiers
						BOOL *onOff = new BOOL[pDerObj->NumModifiers()];
						
						for(int i = 0 ; i < pDerObj->NumModifiers() ; i++ )
						{
							pMod = pDerObj->GetModifier(i);	

							if(pMod->IsEnabled()>0) onOff[i]=TRUE;
							else onOff[i]=FALSE;

							pMod->DisableMod();

							if(pMod == mp )	goto DoneDisabling;
						}

						DoneDisabling:
						node->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
						node->InvalidateWS();

						ObjectState osp = node->EvalWorldState(t);

						Object *osMr = ((Object*)osp.obj->Clone())->ConvertToType(t,defObjectClassID);


						for(int z=0;z<mp->chanBank[cIndex].mNumPoints;z++)
						{
							osMr->SetPoint(z,mp->chanBank[cIndex].mPoints[z]);
							osMr->SetWeight(z,mp->chanBank[cIndex].mWeights[z]);
						}

						// nuke seven bells out of the validity intervals
						// this was the only way to get patches to properly update
						osMr->UpdateValidity(GEOM_CHAN_NUM,FOREVER);
						osMr->UpdateValidity(TOPO_CHAN_NUM,FOREVER);
						osMr->UpdateValidity(SELECT_CHAN_NUM,FOREVER);
						osMr->UpdateValidity(SUBSEL_TYPE_CHAN_NUM,FOREVER);
						osMr->UpdateValidity(DISP_ATTRIB_CHAN_NUM,FOREVER);
						osMr->UpdateValidity(TEXMAP_CHAN_NUM, FOREVER);
						osMr->UpdateValidity(MTL_CHAN_NUM, FOREVER);
						osMr->LockObject();

						INode *newnode = mp->ip->CreateObjectNode(osMr);

						for(i = 0 ; i < pDerObj->NumModifiers() ; i++ )
						{
							pMod = pDerObj->GetModifier(i);	
							if(onOff[i]) pMod->EnableMod();

							if(pMod == mp )	goto DoneRestoring;
						}

						DoneRestoring:

						// clean up
						delete [] onOff;

						// Set the new node's properties (TM, name, etc)
						TSTR nodename = mp->chanBank[cIndex].mName;
						//mp->ip->MakeNameUnique (nodename);
						newnode->SetName(nodename);

						SuspendAnimate();
						AnimateOff();
						newnode->SetNodeTM(t,node->GetNodeTM(t));
						ResumeAnimate();

						// Make sure everything is up to date
						newnode->InvalidateTM();
						newnode->InvalidateWS();
						newnode->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

						mp->ip->RedrawViews(t);

					}
					else
					{
						// there should be no way to push this button without having the morph modifier
						// applied to the object...
						MessageBox(hWnd,GetString(IDS_EXTRACT_MOD_ERROR),GetString(IDS_CLASS_NAME),MB_OK);
					}
				}	// node sel count check
				else
				{
					// this will only happen if the mod stack is 'pinned'
					MessageBox(hWnd,GetString(IDS_EXTRACT_GRAB_ERROR),GetString(IDS_CLASS_NAME),MB_OK);
				}
				
				UI_MAKEFREE
				break;}


			case IDC_DELETE:{

					if (!theHold.Holding()) theHold.Begin();
					theHold.Put(new Restore_FullChannel(mp, mp->CurrentChannelIndex()));

					mp->DeleteChannel( mp->CurrentChannelIndex() );

					theHold.Accept(GetString(IDS_UNDOMCDEL));

					// Refresh system
					mp->Update_channelFULL();
					mp->Update_channelParams();	
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
					mp->ip->RedrawViews(mp->ip->GetTime());
					break;}

			case IDC_DELETE2:
				mp->DeleteTarget();				
			break;

			case IDC_USESEL:{
				ICustButton *iTmp;
					iTmp = GetICustButton(GetDlgItem(hWnd,IDC_USESEL));
					mp->chanBank[mp->chanNum+mp->chanSel].mUseSel = iTmp->IsChecked();
				ReleaseICustButton(iTmp);
				mp->chanBank[mp->chanNum+mp->chanSel].mModded = TRUE;
				mp->Update_colorIndicators();
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());
				break;}


			case IDC_USECHAN:{
				mp->chanBank[mp->chanNum+mp->chanSel].mActiveOverride = GetCheckBox(hWnd,IDC_USECHAN);
				mp->Update_colorIndicators();
				mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());
				break;}


			case IDC_LIMIT:
				mp->chanBank[mp->chanNum+mp->chanSel].mUseLimit = GetCheckBox(hWnd,IDC_LIMIT);
				mp->Update_channelLimits();
				mp->chanBank[mp->chanNum+mp->chanSel].mModded = TRUE;
				mp->Update_colorIndicators();
				break;


			// Ask the channel to update itself
			case IDC_UPDATETARGET:
				{
				morphChannel &bank = mp->CurrentChannel();

				if (!theHold.Holding()) theHold.Begin();
				theHold.Put(new Restore_FullChannel(mp, mp->CurrentChannelIndex()));
				theHold.Accept(GetString (IDS_UNDO_RELOAD));

				UI_MAKEBUSY
				if(bank.iTargetListSelection==0) 
				{
					if(bank.mConnection)
					{
						bank.buildFromNode(bank.mConnection);
						mp->Update_colorIndicators();
					}
					else if(bank.mActive) bank.rebuildChannel();
				}
				else
				{
					int i = bank.iTargetListSelection - 1;
					bank.mTargetCache[i].Init(bank.mTargetCache[i].mTargetINode);
				}					

				mp->NotifyDependents(FOREVER,PART_OBJ,REFMSG_CHANGE);
				mp->ip->RedrawViews(mp->ip->GetTime());

				UI_MAKEFREE

				break;}
			


			case IDC_PICK:
				thePickMode.mp  = mp;					
				mp->ip->SetPickMode(&thePickMode);
				break;

			}
			break;
		}


		default:
			return FALSE;
	}
	return TRUE;
}






/*===========================================================================*\
 | RightClick Menu Stuff
\*===========================================================================*/

class MRCMenu : public RightClickMenu
{
private:
	Interface	*ip;
	MorphR3		*mp; 
	INode		*Sel;

public:
	BOOL Installed;

	MRCMenu(){Installed=FALSE;}
	~MRCMenu(){}

	void Bind(Interface *ipi, MorphR3 *cpi){
		ip=ipi; 
		mp=cpi;
	}

	void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m){

		Installed=TRUE;
		BOOL UsedObject = FALSE;

		char s[255];
		int indexer;
		Interval valid = FOREVER;
		float tmp;

		NameTab ListEm;
		ListEm.ZeroCount();

		Tab<int> indexTab;
		indexTab.ZeroCount();

		Sel = ip->PickNode(hWnd,m);

		if(Sel){

			for(int x=0;x<100;x++)
			{
				if(mp->chanBank[x].mConnection == Sel) 
				{
					mp->chanBank[x].cblock->GetValue(0, ip->GetTime(), tmp, valid);

					indexer = (x+1);

					sprintf(s,"[%i] %s  :  %.1f%%",indexer,mp->chanBank[x].mName,tmp);
					ListEm.AddName(s);
					indexTab.Append(1,&indexer,0);

					UsedObject = TRUE;
				}
			}

			if(UsedObject)
			{
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				manager->BeginSubMenu(GetString(IDS_MENUNAME));
				for(int x=0;x<ListEm.Count();x++)
				{
					manager->AddMenu(this, MF_STRING, indexTab[x], ListEm[x]);
				}
				manager->EndSubMenu();
			}
		}

	}

	void Selected(UINT id){	
		mp->chanNum = id-1;
		mp->Clamp_chanNum();

		int difSel = mp->chanSel + (id-(mp->chanNum + mp->chanSel)) - 1;

		mp->chanSel = difSel;

		// Rebuild all the buttons
		int tIdx = 0,i;
		ICustButton *iTmp;
		for( i=IDC_P1;i<IDC_P10+1;i++){
			iTmp = GetICustButton(GetDlgItem(mp->hwChannelList,i));
			if(tIdx==mp->chanSel) iTmp->SetCheck(TRUE);
			else iTmp->SetCheck(FALSE);
			ReleaseICustButton(iTmp);
			tIdx++;
		}
		mp->Update_channelParams();
		mp->Update_colorIndicators();

		ICustEdit *cName = GetICustEdit(GetDlgItem(mp->hwChannelParams,IDC_CHANNAME));
		cName->GiveFocus();
		ReleaseICustEdit(cName);

		mp->Update_channelFULL();
		mp->Update_channelParams();
	}

};

MRCMenu rcdd;


/*===========================================================================*\
 | Load and display the interfaces
\*===========================================================================*/

void MorphR3::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	hMaxWnd = ip->GetMAXHWnd();
	this->ip = ip;
	if(!tccI) { ip->RegisterTimeChangeCallback(this); tccI = TRUE; }

	hwLegend = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_LEGEND),
				Legend_DlgProc, 
				GetString(IDS_LEGEND_DLG), 
				(LPARAM)this,
				APPENDROLL_CLOSED);		
	ip->RegisterDlgWnd(hwLegend);
	SetWindowLongPtr(hwLegend,GWLP_USERDATA,(LONG_PTR)this);

	hwGlobalParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_GLOBAL_PARAMS),
				Globals_DlgProc, 
				GetString(IDS_GLOBAL_PARAMS), 
				(LPARAM)this,
				APPENDROLL_CLOSED);		
	ip->RegisterDlgWnd(hwGlobalParams);
	SetWindowLongPtr(hwGlobalParams,GWLP_USERDATA,(LONG_PTR)this);

	hwChannelList = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_CHANNEL_LIST),
				ChannelList_DlgProc, 
				GetString(IDS_CHANNEL_LIST), 
				(LPARAM)this);		
	ip->RegisterDlgWnd(hwChannelList);
	SetWindowLongPtr(hwChannelList,GWLP_USERDATA,(LONG_PTR)this);

	hwChannelParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_CHANNEL_PARAMS),
				ChannelParams_DlgProc, 
				GetString(IDS_CHANNEL_PARAMS), 
				(LPARAM)this);		
	ip->RegisterDlgWnd(hwChannelParams);
	SetWindowLongPtr(hwChannelParams,GWLP_USERDATA,(LONG_PTR)this);

	hwAdvanced = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_ADVANCED),
				Advanced_DlgProc, 
				GetString(IDS_ADVANCED), 
				(LPARAM)this,
				APPENDROLL_CLOSED);		
	ip->RegisterDlgWnd(hwAdvanced);
	SetWindowLongPtr(hwAdvanced,GWLP_USERDATA,(LONG_PTR)this);

	// Load our RCMenu Callback
	RightClickMenuManager *rcm = ip->GetRightClickMenuManager();
	rcdd.Bind(ip,this);
	rcm->Register(&rcdd);

	Update_channelValues();
}

/*===========================================================================*\
 | Remove all the pages
\*===========================================================================*/

void MorphR3::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ip->UnRegisterDlgWnd(hwLegend);
	ip->DeleteRollupPage(hwLegend);
	hwLegend = NULL;

	ip->UnRegisterDlgWnd(hwGlobalParams);
	ip->DeleteRollupPage(hwGlobalParams);
	hwGlobalParams = NULL;

	ip->UnRegisterDlgWnd(hwChannelList);
	ip->DeleteRollupPage(hwChannelList);
	hwChannelList = NULL;

	ip->UnRegisterDlgWnd(hwChannelParams);
	ip->DeleteRollupPage(hwChannelParams);
	hwChannelParams = NULL;

	ip->UnRegisterDlgWnd(hwAdvanced);
	ip->DeleteRollupPage(hwAdvanced);
	hwAdvanced = NULL;


	RightClickMenuManager *rcm = ip->GetRightClickMenuManager();
	rcm->Unregister(&rcdd);
}


// The following static method would be better as a method on the MorphR3 class. I have
// placed them here as we are not able to break SDK compatability this release. They should
// be moved during the next SDK-breaking release cycle. (tb) 
//
static float GetTargetPercentage( morphChannel * bank, int index ) 
{
		float targPerc = 0.0;
		if ( index == 0 ) {
			targPerc = bank->mTargetPercent;	
		}
		else {
			targPerc = bank->mTargetCache[index-1].mTargetPercent;	
		}

		return targPerc;
}

// The following static method would be better as a method on the MorphR3 class. I have
// placed them here as we are not able to break SDK compatability this release. They should
// be moved during the next SDK-breaking release cycle. (tb) 
//
static void SetTargetPercentage( morphChannel * bank, int index, float perc ) 
{
	if(index==0) {
		bank->mTargetPercent = perc;
	}
	else if(index>0 && index<=bank->mNumProgressiveTargs ) {
		bank->mTargetCache[index-1].mTargetPercent = perc;
	}

}

// The following static method would be better as a method on the MorphR3 class. I have
// placed them here as we are not able to break SDK compatability this release. They should
// be moved during the next SDK-breaking release cycle. (tb) 
//
static void SwapTargetPercentage( morphChannel * bank, int way ) 
{

	int from = bank->iTargetListSelection;
	int to = from + way;

	float fromPerc = GetTargetPercentage( bank, from );
	float toPerc =   GetTargetPercentage( bank, to );

	SetTargetPercentage( bank, from, toPerc );
	SetTargetPercentage( bank, to,   fromPerc );
}

// The following static method would be better as a method on the MorphR3 class. I have
// placed them here as we are not able to break SDK compatability this release. They should
// be moved during the next SDK-breaking release cycle. (tb) 
//
static void SortSelectedProgressiveTarget( MorphR3 * morphObj ) 
{

	// do swapping of progressive targets based on an edit of target percentage.
	// morphObj->SwapTargets will update the selected target index 
	// if it is changed
	//
	// make sure we don't overlap the next or prev target, if we do, change them to be = to this one
	//
	morphChannel &bank= morphObj->CurrentChannel();
	int num = bank.NumProgressiveTargets();

	int whereItIs = bank.iTargetListSelection;
	int whereItBelongs = 0;
	
	float selectedTargetPercent = morphObj->GetCurrentTargetPercent();
	for( int i = 0; i < bank.NumProgressiveTargets()+1; ++i ) {

		float testTargetPercent = GetTargetPercentage( &bank, i );

		if ( testTargetPercent < selectedTargetPercent ) {
			++whereItBelongs;
		}

	}

	if ( whereItBelongs > whereItIs ) {
		for( int index = whereItIs; index < whereItBelongs; ++index ) {
			SwapTargetPercentage( &bank, +1 );
			morphObj->SwapTargets( +1 );
		}
	}

	if ( whereItBelongs < whereItIs ) {
		for( int index = whereItIs; index > whereItBelongs; --index ) {
			SwapTargetPercentage( &bank, -1 );
			morphObj->SwapTargets( -1 );
		}
	}

}

