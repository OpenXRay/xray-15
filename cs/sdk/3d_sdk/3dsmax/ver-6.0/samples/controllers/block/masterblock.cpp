/**********************************************************************
 *<
	FILE: noizctrl.cpp

	DESCRIPTION: A simple noise controller

	CREATED BY: Rolf Berteig

	HISTORY: created 26 August 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "block.h"
#include "units.h"
#include "masterblock.h"
#include "istdplug.h"
//#include "treevw.h"


//#include "iparamm2.h"

static MasterBlockClassDesc masterBlockCD;
ClassDesc* GetMasterBlockDesc() {return &masterBlockCD;}
//static Tab<NoiseCtrlWindow> noiseCtrlWindows;

static INT_PTR CALLBACK MasterBlockDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddBlockDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AttachBlockDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MasterBlockPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK KeyPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*

enum { masterblock_params };
// path_params param IDs
enum { masterblock_slaves };

// per instance path controller block
static ParamBlockDesc2 masterblock_paramblk (masterblock_params, _T("MasterParameters"),  0, &masterBlockCD, P_AUTO_CONSTRUCT + P_AUTO_UI, MASTER_PBLOCK_REF, 
	//rollout
	0, 0, 0, 0, NULL,
	// params
	masterblock_slaves,	_T("Slaves"),TYPE_FLOAT_TAB,10, P_SUBANIM, 	IDS_PW_SLAVE, 
		end, 
	end
	);

*/
//-----------------------------------------------------------------------------

/*
static void RegisterNoiseCtrlWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	NoiseCtrlWindow rec(hWnd,hParent,cont);
	noiseCtrlWindows.Append(1,&rec);
	}

static void UnRegisterNoiseCtrlWindow(HWND hWnd)
	{	
	for (int i=0; i<noiseCtrlWindows.Count(); i++) {
		if (hWnd==noiseCtrlWindows[i].hWnd) {
			noiseCtrlWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenNoiseCtrlWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<noiseCtrlWindows.Count(); i++) {
		if (hParent == noiseCtrlWindows[i].hParent &&
			cont    == noiseCtrlWindows[i].cont) {
			return noiseCtrlWindows[i].hWnd;
			}
		}
	return NULL;
	}

*/

MasterBlockDlg::MasterBlockDlg(
		MasterBlockControl *cont,
		ParamDimensionBase *dim,
		TCHAR *pname,
		IObjParam *ip,
		HWND hParent)
	{
//	InitNoiseGraph(hInstance);
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;
	cont->iop=ip;
//	elems = cont->Elems();

//	theHold.Suspend();
//	MakeRefByID(FOREVER,0,cont);
//	theHold.Resume();

//	hWnd = CreateDialogParam(
	int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_MASTERPARAMS),
		        hParent,MasterBlockDlgProc,(LPARAM)this);	
	cont->iop=NULL;

//	TSTR title = TSTR(GetString(IDS_RB_NOISECONTROLTITLE)) + TSTR(pname);
//	SetWindowText(hWnd,title);
//	ip->RegisterTimeChangeCallback(this);
	}

MasterBlockDlg::~MasterBlockDlg()
	{
//	UnRegisterNoiseCtrlWindow(hWnd);
//	ip->UnRegisterTimeChangeCallback(this);

//	theHold.Suspend();
//	DeleteAllRefsFromMe();
//	theHold.Resume();

	}

void MasterBlockDlg::Invalidate()
	{
//	valid = FALSE;
//	InvalidateRect(hWnd,NULL,FALSE);	
//	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,FALSE);
	}

void MasterBlockDlg::Update()
	{
	if (!valid && hWnd) {
/*
		float strength[MAX_ELEMS];
		cont->GetStrength(ip->GetTime(),strength);

		for (int i=0; i<elems; i++) {			
//			iStrength[i]->SetValue(dim->Convert(strength[i]),FALSE);
//			CheckDlgButton(hWnd,limID[i],cont->lim[i]);
			}
		if (cont->fractal) {
		} else {
			}
*/
		valid = TRUE;
		}
	}

void MasterBlockDlg::SetupUI(HWND hWnd)
	{
	this->hWnd = hWnd;

/*	iRampOut = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_RAMPOUTSPIN));
	iRampOut->SetLimits(0,TIME_PosInfinity,FALSE);
	iRampOut->SetScale(10.0f);
	iRampOut->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_RAMPOUT),EDITTYPE_TIME);	
*/
	
//	SetWindowLongPtr(GetDlgItem(hWnd,IDC_NOISE_GRAPH),GWLP_USERDATA,(LONG)cont);
	SetupList();


	valid = FALSE;
	Update();
	}

void MasterBlockDlg::SetupList()
	{
	SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_RESETCONTENT,0,0);
	for (int i=0; i<cont->Blocks.Count(); i++) {
		TSTR name = cont->SubAnimName(i+1);
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
		}

	int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);

	if (sel!=LB_ERR) {
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,(WPARAM)sel,0);
		SetButtonStates();
	} else {
		if (cont->Blocks.Count() > 0)
			{
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCURSEL,(WPARAM)cont->Blocks.Count()-1,0);
			SetButtonStates();
			}
		else SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCURSEL,(WPARAM)-1,0);
		}


	}

void MasterBlockDlg::SetButtonStates()
	{
	int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
		LB_GETCURSEL,0,0);
	ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));

	if (sel!=LB_ERR) {
		iName->Enable();
		if (cont->names[sel]) iName->SetText(*cont->names[sel]);
		else iName->SetText(_T(""));
	} else {
		iName->Disable();
		}
	ReleaseICustEdit(iName);


	if (sel!=LB_ERR) {
		COLORREF c = cont->Blocks[sel]->color.toRGB();
		IColorSwatch *cs = GetIColorSwatch(GetDlgItem(hWnd,IDC_COLOR),
				c, _T("Block Color"));
		cs->SetColor(c);
		ReleaseIColorSwatch(cs);
		}


	}

void MasterBlockDlg::EnableButtons()
{
int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
		LB_GETCURSEL,0,0);
if (sel>=0) {
	EnableWindow(GetDlgItem(hWnd,IDC_SAVE),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_REPLACE),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_DELETE),TRUE);
	}
else {
	EnableWindow(GetDlgItem(hWnd,IDC_SAVE),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_REPLACE),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_DELETE),FALSE);
	}
}

void MasterBlockDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_LIST_NAME: {
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel>=0) {
				TCHAR buf[256];
				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
				iName->GetText(buf,256);
				if (!cont->names[sel]) cont->names[sel] = new TSTR;
				*cont->names[sel] = buf;

				cont->NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);

				SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_RESETCONTENT,0,0);
				for (int i=0; i<cont->Blocks.Count(); i++) {
					TSTR name = cont->SubAnimName(i+1);
					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
					}
				SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_SETCURSEL,(WPARAM)sel,0);



//				SetupList();
				ReleaseICustEdit(iName);
//				EnableWindow(GetDlgItem(hWnd,IDC_SAVE),TRUE);
//				EnableWindow(GetDlgItem(hWnd,IDC_REPLACE),TRUE);
//				EnableWindow(GetDlgItem(hWnd,IDC_DELETE),TRUE);

				}
			break;
			}
		case IDC_LIST1:
			if (notify==LBN_SELCHANGE) {
				SetButtonStates();				
				}

			break;
		case IDC_ADD:
			theHold.Begin();
			theHold.Put(new MasterBlockAdd(cont));
			theHold.Suspend();
			cont->AddBlock(hCtrl);
			SetupList();
			EnableButtons();
			theHold.Resume();
			theHold.Accept(GetString(IDS_PW_ADDBLOCK));
			GetSystemSetting(SYSSET_CLEAR_UNDO);
			SetSaveRequiredFlag(TRUE);
			break;
		case IDC_ADD_SELECTED:
			theHold.Begin();
			theHold.Suspend();
			cont->AddSelected(hCtrl);
			SetupList();
			EnableButtons();
			theHold.Resume();
			theHold.Accept(GetString(IDS_PW_ADDBLOCK));
			GetSystemSetting(SYSSET_CLEAR_UNDO);
			SetSaveRequiredFlag(TRUE);
			break;

		case IDC_REPLACE:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			cont->ReplaceBlock(hCtrl,sel);
			SetupList();
			EnableButtons();
			GetSystemSetting(SYSSET_CLEAR_UNDO);
			SetSaveRequiredFlag(TRUE);
			break;
			}
		case IDC_DELETE:
			{
//			theHold.Begin();
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			cont->DeleteBlock(sel);
			SetupList();
			EnableButtons();
			GetSystemSetting(SYSSET_CLEAR_UNDO);
			break;
			}
		case IDC_SAVE:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			cont->SaveBlock(sel);
			SetupList();
			break;
			}
		case IDC_LOAD:
			{
			cont->LoadBlock();
			SetupList();
			break;
			}
		case IDOK:
//			DestroyWindow(hWnd);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			EndDialog(hWnd,1);
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);

//			DestroyWindow(hWnd);
			break;


		}

	}


void MasterBlockDlg::Change(BOOL redraw)
	{
//	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,TRUE);
//	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
//	UpdateWindow(GetDlgItem(hWnd,IDC_NOISE_GRAPH));
//	UpdateWindow(GetParent(hWnd));	
//	if (redraw) ip->RedrawViews(ip->GetTime());
	}


class CheckForNonMasterBlockDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonMasterBlockDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(MASTERBLOCKDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void MasterBlockDlg::MaybeCloseWindow()
	{
	CheckForNonMasterBlockDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}


/*
RefResult MasterBlockDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			Invalidate();			
			break;
		
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}


*/





static INT_PTR CALLBACK MasterBlockDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	MasterBlockDlg *dlg = (MasterBlockDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			dlg = (MasterBlockDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			dlg->SetupUI(hWnd);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCURSEL,0,0);
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel==-1) 
				{
				EnableWindow(GetDlgItem(hWnd,IDC_SAVE),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_REPLACE),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_DELETE),FALSE);
				}

			if (dlg->cont->Blocks.Count()>0)
				{
				COLORREF c = dlg->cont->Blocks[0]->color.toRGB();
				IColorSwatch *cs = GetIColorSwatch(GetDlgItem(hWnd,IDC_COLOR),
					c, _T("Block Color"));
				cs->SetColor(c);
				ReleaseIColorSwatch(cs);

				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));

				if (dlg->cont->names[0]) iName->SetText(*dlg->cont->names[0]);
				else iName->SetText(_T(""));
				ReleaseICustEdit(iName);

	
				}


			break;

			}
		case WM_COMMAND:
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;

		case WM_PAINT:
			dlg->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:						
			delete dlg;
			break;
	
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE: {
			int i = LOWORD(wParam);
			IColorSwatch *cs = (IColorSwatch*)lParam;
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel != -1)
				{
				if (HIWORD(wParam)) theHold.Begin();
				dlg->cont->Blocks[sel]->color = Color(cs->GetColor());
				if (HIWORD(wParam)) {
					theHold.Accept(GetString(IDS_DS_PARAMCHG));
					}
				}
			break;
		}

		
		default:
			return FALSE;
		}
	return TRUE;
	}




void MasterBlockControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags)
	{
	
	BOOL found = FALSE;
	trackHWND = hParent;
	int where;
	for (int i = 0; i < BlockKeys.Count(); i++)
		{
		if (BlockKeys[i].start < BlockKeys[i].end)
			{
			if ( ((t >= BlockKeys[i].start) && (t <= BlockKeys[i].end)) &&
				 ((BlockKeys[i].startSelected) || ( BlockKeys[i].endSelected))
				)
				{
				found = TRUE;
				where = i;
				}
			}
		else
			{
			if ( ( (t >= BlockKeys[i].end) && (t <= BlockKeys[i].start) ) &&
				 ((BlockKeys[i].startSelected) || ( BlockKeys[i].endSelected)) )
				{
				found = TRUE;
				where = i;
				}
			}

		}
	if (found)
		{
		propStart = BlockKeys[where].start;
		propEnd = BlockKeys[where].end;
		propRelative = BlockKeys[where].relative;

		int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_KEYPROP),
				hParent, KeyPropDlgProc, (LPARAM)this);
		if (OK)
			{
//change block type			
			BlockKeys[where].start = propStart * GetTicksPerFrame() ;
			BlockKeys[where].end = propEnd * GetTicksPerFrame() ;
			BlockKeys[where].relative = propRelative;
			UpdateControl(BlockKeys[where].block_id);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		}
	else{ 
//add right click menu here
//add only if valid time range
		if (EDITTRACK_MOUSE&flags)
			{
			HMENU hMenu = CreatePopupMenu();
			for (int i = 0; i < Blocks.Count();i++)
				AppendMenu(  hMenu,      // handle to menu to be changed
						 MF_ENABLED | MF_STRING,      // menu-item flags
						i+2,  // menu-item identifier or handle to drop-down menu or submenu
						*names[i] // menu-item content
						);
			if (Blocks.Count() > 0)
				AppendMenu(  hMenu,      // handle to menu to be changed
						 MF_SEPARATOR,      // menu-item flags
						0,  // menu-item identifier or handle to drop-down menu or submenu
						NULL // menu-item content
						);
			AppendMenu(  hMenu,      // handle to menu to be changed
					 MF_ENABLED | MF_STRING,      // menu-item flags
					1,  // menu-item identifier or handle to drop-down menu or submenu
					GetString(IDS_PW_PROPERTIES) // menu-item content
					);
			POINT lpPoint;   
			GetCursorPos( &lpPoint );  // address of structure for cursor position); 

			int id = TrackPopupMenuEx(hMenu, 
				TPM_TOPALIGN | TPM_VCENTERALIGN | TPM_RIGHTBUTTON  | TPM_RETURNCMD , 
				lpPoint.x, lpPoint.y,  hParent, NULL);
			DestroyMenu(hMenu);		

			if (id == 1) 
				{
				MasterBlockDlg *dlg = new MasterBlockDlg(this,dim,pname,ip,hParent);
//				delete dlg;
				}
				else
				{
				if ((id >=2) && (id < (Blocks.Count()+2)) )
					AddKey(t, id-2);
				}
			}
		else
			{
			MasterBlockDlg *dlg = new MasterBlockDlg(this,dim,pname,ip,hParent);
			}



//add new key block

		}
	}


//------------------------------------------------------------

MasterBlockControl::MasterBlockControl() 
	{	
//	for (int i=0; i<MAX_ELEMS; i++) {
		//strength[i] = 50.0f;
//		lim[i] = FALSE;
//		}
//	fractal   = TRUE;
//	roughness = 0.0f;
//	frequency = 0.5f;	
//	seed      = 0;	
//	rampin = rampout = 0;
//	`SetStart(0);
	range.SetEmpty();
//	range    = Interval(GetAnimStart(),GetAnimEnd());

//	cont=NULL;
//	ReplaceReference(Control::NumRefs(),NewDefaultFloatController());
//	float v = 50.0f;
//	cont->SetValue(0,&v);
	blendControl = NULL;
	ReplaceReference(0,NewDefaultFloatController());
	float v = 1.0f;

	SuspendAnimate();
	AnimateOn();
	blendControl->SetValue(0,&v);
	ResumeAnimate();
	isCurveSelected = FALSE;
	rangeUnlocked = FALSE;

//	masterBlockCD.MakeAutoParamBlocks(this);


	} 

int MasterBlockControl::NumSubs() 
	{
	return Blocks.Count()+1;
	}

Animatable* MasterBlockControl::SubAnim(int i) 
	{
	if (i==0)
		return blendControl;
	else if ((i-1) < Blocks.Count())
		return Blocks[i-1];
	else
		{
//		DebugPrint("Subanims out of range call\n");
		return NULL;
		}
	}

TSTR MasterBlockControl::SubAnimName(int i) 
	{
		TSTR name;
		if (i==0)
			return GetString(IDS_PW_BLEND); 
		else if ((i-1) < names.Count())
			{
			if (names[i-1] && names[i-1]->length()) 
				{
				name = *names[i-1];
				}
			} 
		else if (Blocks[i-1]) 
			{
			Blocks[i-1]->GetClassName(name);
			} 
/*
	if (i<Control::NumSubs()) 
		return Control::SubAnimName(i);*/
	return name;
	}


BOOL MasterBlockControl::AssignController(Animatable *control,int subAnim) 
	{
	if (subAnim == 0)
		{
		ReplaceReference(0,(Control*)control);
		return TRUE;
		}
	else return FALSE;
	}


int MasterBlockControl::NumRefs() 
	{
	return Blocks.Count() + 1;
	}

RefTargetHandle MasterBlockControl::GetReference(int i) 
	{
	if (i == 0)
		return blendControl;
	else if ((i-1) < Blocks.Count())
		return Blocks[i-1];
	else 
		{
//		DebugPrint("get reference error occurred\n");
		return NULL;
		}
	}

void MasterBlockControl::SetReference(int i, RefTargetHandle rtarg) 
	{
	if (i==0) blendControl = (Control *) rtarg;
	else if ((i-1) < Blocks.Count())
		Blocks[i-1] = (BlockControl *) rtarg;
	else DebugPrint("set reference error occurred\n");

//	pblock2 = (IParamBlock2*)rtarg;
	}


RefResult MasterBlockControl::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
//			Invalidate();			
//			DebugPrint("Ref changed should have slave get updated\n");
			break;
		
		}
	return REF_SUCCEED;
	}


void MasterBlockControl::UpdateControl(int index)
{


Blocks[index]->RebuildTempControl();
for (int i = 0; i < BlockKeys.Count();i++)
	{
	if (BlockKeys[i].block_id == index)
		Blocks[index]->AddKeyToTempControl(BlockKeys[i].start,  BlockKeys[i].end-BlockKeys[i].start,BlockKeys[i].relative);
	}
Blocks[index]->NotifySlaves();


}


void MasterBlockControl::HoldRange()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
	}
void MasterBlockControl::HoldTrack()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new MasterBlockRest(this));
		}
	}



void MasterBlockControl::EditTimeRange(Interval range,DWORD flags)
	{
//DebugPrint("EditTimeRaneg\n");
	if ((flags&EDITRANGE_LINKTOKEYS)) {
		rangeUnlocked = FALSE; 
		}
	else 
		{
		HoldRange();
		this->range = range;
		rangeUnlocked = TRUE; 
		}

	for (int i = 0; i < Blocks.Count();i++)
		{
		UpdateControl(i);
		}

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
/*	
	if (!(flags&EDITRANGE_LINKTOKEYS)) {
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

		}
*/
	}

void MasterBlockControl::SelectCurve(BOOL sel)
{
isCurveSelected = sel;
if (!sel)
	{
//	DebugPrint("Not Selected Curve\n");

	for (int i=0; i < BlockKeys.Count(); i++)
		{
		BlockKeys[i].startSelected = FALSE;
		BlockKeys[i].endSelected = FALSE;
		}
	}
//else DebugPrint("Selected Curve\n");
}
BOOL MasterBlockControl::IsCurveSelected()
{
return isCurveSelected;
}


void MasterBlockControl::UpdateRange()
{

if (!rangeUnlocked)
	{
		range.SetEmpty();
		if (BlockKeys.Count() > 0)
			{
			if (BlockKeys[0].start < BlockKeys[0].end)
				{
				range.SetStart(BlockKeys[0].start);
				range.SetEnd(BlockKeys[0].end);
				}
			else
				{
				range.SetEnd(BlockKeys[0].start);
				range.SetStart(BlockKeys[0].end);
				}
			}
		for (int i=1; i < BlockKeys.Count(); i++)
			{
			if (BlockKeys[i].start < range.Start()) range.SetStart(BlockKeys[i].start);
			if (BlockKeys[i].end < range.Start()) range.SetStart(BlockKeys[i].end);
			if (BlockKeys[i].start > range.End()) range.SetEnd(BlockKeys[i].start);
			if (BlockKeys[i].end > range.End()) range.SetEnd(BlockKeys[i].end);
			}
	}
}

void MasterBlockControl::MapKeys(TimeMap *map,DWORD flags)
	{

	HoldTrack();
//	DebugPrint("MapKEys\n");
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());

		range.Set(t0,t1);
//	DebugPrint("Range %d %d new range %d %d\n",t0,t1,range.Start(),range.End());
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}

	int n = BlockKeys.Count();
	BOOL changed = FALSE;
	if (!n) return;
	BitArray changedBlock;

	changedBlock.SetSize(Blocks.Count());
	changedBlock.ClearAll();

	if (flags&TRACK_DOALL) {
		for (int i=0; i<n; i++) {
			BlockKeys[i].start = map->map(BlockKeys[i].start);
			BlockKeys[i].end = map->map(BlockKeys[i].end);
			changedBlock.Set(BlockKeys[i].block_id);
			changed = TRUE;
			}
	} else 
	if (flags&TRACK_DOSEL) {
		BOOL slide = flags&TRACK_SLIDEUNSEL;
		TimeValue delta = 0, prev;
		int start, end, inc;
		if (flags&TRACK_RIGHTTOLEFT) {
			start = n-1;
			end = -1;
			inc = -1;
		} else {
			start = 0;
			end = n;
			inc = 1;
			} 
		for (int i = start; i!=end; i+=inc) {
			if (BlockKeys[i].startSelected) {                   
				prev = BlockKeys[i].start;
				BlockKeys[i].start = map->map(BlockKeys[i].start);
				delta = BlockKeys[i].start - prev;
				changedBlock.Set(BlockKeys[i].block_id);

				changed = TRUE;
			} else if (slide) {
				BlockKeys[i].start += delta;
				changedBlock.Set(BlockKeys[i].block_id);

				}
			if (BlockKeys[i].endSelected) {                   
				prev = BlockKeys[i].end;
				BlockKeys[i].end = map->map(BlockKeys[i].end);
				delta = BlockKeys[i].end - prev;
				changedBlock.Set(BlockKeys[i].block_id);

				changed = TRUE;
			} else if (slide) {
				BlockKeys[i].end += delta;
				changedBlock.Set(BlockKeys[i].block_id);

				}
			}
		}


/*	if (flags&TRACK_MAPRANGE && keys.TestFlag(RANGE_UNLOCKED)) {
		TimeValue t0 = map->map(keys.range.Start());
		TimeValue t1 = map->map(keys.range.End());
		keys.range.Set(t0,t1);
		}
*/
	UpdateRange();

	if (changed) {
//		keys.Invalidate();
//		Invalidate();
		for (int i = 0; i < changedBlock.GetSize();i++)
			{
			if (changedBlock[i])
				UpdateControl(i);
			}

		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}


	}

void MasterBlockControl::FlagKey(TrackHitRecord hit)
	{
	int n = BlockKeys.Count();
	for ( int i = 0; i < n; i++ ) {
		BlockKeys[i].startFlag  = 0;
		BlockKeys[i].endFlag  = 0;
		}
	int index  = hit.hit;

	int whichBlock = index/2;
	int whichEnd = index%2;
//	assert(hit.hit>=0&&hit.hit<(DWORD)n);
	if (whichEnd ==0)
		BlockKeys[whichBlock].startFlag = KEY_FLAGGED;
	else if (whichEnd ==1)
		BlockKeys[whichBlock].endFlag = KEY_FLAGGED;

//DebugPrint("Flag key %d %d \n",whichBlock,whichEnd);

	}

int MasterBlockControl::GetFlagKeyIndex()
	{
	int n = BlockKeys.Count()/2;
//DebugPrint("Get Flag key \n");
	for ( int i = 0; i < n; i++ ) {
		if (BlockKeys[i].startFlag == KEY_FLAGGED)
			{
			return i*2;
			}
		if (BlockKeys[i].endFlag == KEY_FLAGGED)
			{
			return i*2+1;
			}
		}

	return -1;
	}


int MasterBlockControl::NumSelKeys()
{
int ct = 0;
for (int i = 0; i < BlockKeys.Count(); i++)
	{
	if (BlockKeys[i].startSelected)  ct++;
	if (BlockKeys[i].endSelected)  ct++;
	}
//DebugPrint("Num sel keys %d\n",ct);
return ct;
}
int MasterBlockControl::NumKeys()
{
//DebugPrint("Num keys %d\n",BlockKeys.Count());
return BlockKeys.Count()*2;
}

TimeValue MasterBlockControl::GetKeyTime(int index) 
{
//DebugPrint("Get key time  %d\n",index);
	int whichBlock = index/2;
	int whichEnd = index%2;
	if (whichEnd == 0)
		return BlockKeys[whichBlock].start;
	else return BlockKeys[whichBlock].end;
}

void MasterBlockControl::CloneSelectedKeys(BOOL offset)
	{
//DebugPrint("Clone Me Offset %d\n",offset);
//loop through block keys
	int ct = BlockKeys.Count();
	for (int i = 0; i < ct; i++)
		{
		if ( ( BlockKeys[i].startSelected) && (BlockKeys[i].endSelected))
			{
			BlockKeysClass b;
			b = BlockKeys[i];
			BlockKeys.Append(1,&b,1);

			}
		BlockKeys[i].startSelected = FALSE;
		BlockKeys[i].endSelected = FALSE;

		}

//	if (!conts.Count()) return;
//	assert(active>=0);
//	conts[active]->CloneSelectedKeys(offset);
	}

void MasterBlockControl::DeleteKeys(DWORD flags)
	{
//DebugPrint("Delete Me \n");

	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new MasterBlockDeleteKey(this));
		SetAFlag(A_HELD);
		}
	theHold.Accept(GetString(IDS_PW_MB_DELETEKEY));
	//HoldRange();

	BitArray UpdateList;
	UpdateList.SetSize(Blocks.Count());
	UpdateList.ClearAll();
	for (int i = 0; i < BlockKeys.Count(); i++)
		{
//loop through blocks
		if ( ( BlockKeys[i].startSelected) || (BlockKeys[i].endSelected))
			{
//find blocks that are selected and delete them
			UpdateList.Set(BlockKeys[i].block_id);
			BlockKeys.Delete(i,1);
			i--;
			}	
		}
	UpdateRange();
	for (i = 0; i < UpdateList.GetSize(); i++)
		{
		if (UpdateList[i])
			{
			UpdateControl(i);
			}
		}

	}


BOOL MasterBlockControl::IsKeySelected(int index)
	{
//	if (!conts.Count()) return FALSE;
//	assert(active>=0);
//	return conts[active]->IsKeySelected(index);
//DebugPrint("Is key selected \n");
	int whichBlock = index /2;
	int whichEnd = index %2;
	
	if (index<BlockKeys.Count())
		{
		if (whichEnd == 0)
			return BlockKeys[index].startSelected;
		else return BlockKeys[index].endSelected;
		}
	return FALSE;
	}

void MasterBlockControl::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
	{
//DebugPrint("Copy Me %d to %d\n",src,dst);

//	if (!conts.Count()) return;
//	assert(active>=0);
//	conts[active]->CopyKeysFromTime(src,dst,flags);
	}
/*
void ListControl::DeleteKeyAtTime(TimeValue t)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->DeleteKeyAtTime(t);
	}
*/

BOOL MasterBlockControl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
//DebugPrint("Is Key at time %d \n",t);
//	if (!conts.Count()) return FALSE;
//	assert(active>=0);
//	return conts[active]->IsKeyAtTime(t,flags);
	for (int i = 0; i < BlockKeys.Count();i++)
		{
		if (BlockKeys[i].start = t) return TRUE;
		}
	return FALSE;
	}
/*
BOOL MasterBlockControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]->GetNextKeyTime(t,flags,nt);
	}
*/
int MasterBlockControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
	{
//DebugPrint("Get Key times \n");
//	if (!conts.Count()) return 0;
//	assert(active>=0);
//	return conts[active]->GetKeyTimes(times,range,flags);
	int ct =  0;
	for (int i = 0; i < BlockKeys.Count();i++)
		{
		if (range.InInterval(BlockKeys[i].start))
			{
			times.Append(1,&BlockKeys[i].start,1);
			}
		else if (BlockKeys[i].start<range.Start()) 
			ct++;
		}
	return ct;
	}

int MasterBlockControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
	{
//DebugPrint("Is Key Sel state  \n");

//	if (!conts.Count()) return 0;
//	assert(active>=0);
//	return conts[active]->GetKeySelState(sel,range,flags);
	int ct =  0;
	for (int i = 0; i < BlockKeys.Count();i++)
		{
		if (range.InInterval(BlockKeys[i].start))
			{
			if (BlockKeys[i*2].startSelected)
				sel.Set(i*2);
			else sel.Clear(i*2);
			}
		else if (BlockKeys[i].start<range.Start()) 
			ct++;
		if (range.InInterval(BlockKeys[i].end))
			{
			if (BlockKeys[i*2+1].endSelected)
				sel.Set(i*2+1);
			else sel.Clear(i*2+1);
			}
		else if (BlockKeys[i].end<range.Start()) 
			ct++;

		
		}
	return ct;
	}


void MasterBlockControl::AddKey(TimeValue t, int whichBlock)
	{
//DebugPrint("Add key here\n");
//pop up a menu listing all types

	
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new MasterBlockAddKey(this));
		SetAFlag(A_HELD);
		}
	else
		{
		theHold.Begin();
		theHold.Put(new MasterBlockAddKey(this));
		SetAFlag(A_HELD);

		}
	theHold.Accept(	GetString(IDS_PW_MB_ADDKEY));

//	theHold.Cancel();
//	PopCommandMode();
	AddDialogSelect = whichBlock;

	if (AddDialogSelect != -1)
		{
		//HoldTrack();
//add a new block track here at this time
		BlockKeysClass block;
		block.startSelected = FALSE;
		block.endSelected = FALSE;
		block.relative = TRUE;
		block.start = t;
		TimeValue l = Blocks[AddDialogSelect]->end - Blocks[AddDialogSelect]->start +1;
//		block.end = t+ Blocks[AddDialogSelect]->l-1;
		block.end = t+ l-1;
		block.block_id = AddDialogSelect;
		BlockKeys.Append(1,&block,1);

		UpdateRange();

		UpdateControl(AddDialogSelect);
//		UpdateControl(AddDialogSelect);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		
//		Blocks[AddDialogSelect]->NotifySlaves();
		}

		
	}


void MasterBlockControl::AddNewKey(TimeValue t,DWORD flags)
	{
	}
void MasterBlockControl::SelectKeys(TrackHitTab& sel, DWORD flags)
{
if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
	theHold.Put(new MasterBlockRest(this));
	SetAFlag(A_HELD);
	}
else
	{
	theHold.Begin();
	theHold.Put(new MasterBlockRest(this));
	SetAFlag(A_HELD);
	}



theHold.Accept(	GetString(IDS_PW_MB_SELECTKEY));




if (flags&SELKEYS_CLEARCURVE) isCurveSelected = FALSE;
if ((flags&SELKEYS_CLEARKEYS) || (flags&SELKEYS_CLEARCURVE))
	{
	
	for (int i = 0; i < BlockKeys.Count(); i++)
		{
		if (sel.Count() == 0)
			{
			BlockKeys[i].startSelected = FALSE;
			BlockKeys[i].endSelected = FALSE;
			}
		else
			{
			if (sel[0].flags !=2)
				{
				BlockKeys[i].startSelected = FALSE;
				BlockKeys[i].endSelected = FALSE;

				}	
			}
		}
	}

BOOL fcurve = flags&SELKEYS_FCURVE;
if (flags&SELKEYS_SELECT) 
	{	
	if (fcurve) 
		{
		// If the curve isn't yet selected, eat the input and just
		// select the curve.
		if (!isCurveSelected)
			{				
			isCurveSelected = TRUE;
			return;
			}
		}
	}


for ( int i = 0; i < sel.Count(); i++)
	{
	int whichBlock = sel[i].hit /2;
	int whichEnd = sel[i].hit %2;
	if (sel[i].flags ==2)
		{
		BlockKeys[whichBlock].relative = !BlockKeys[whichBlock].relative;
		UpdateControl(BlockKeys[whichBlock].block_id);
		BlockKeys[whichBlock].endSelected = endRestoreState;
		BlockKeys[whichBlock].startSelected = startRestoreState;

		}
	else if (sel[i].flags ==1)
		{
		if (flags&SELKEYS_DESELECT)
			BlockKeys[whichBlock].startSelected = FALSE;
		if (flags&SELKEYS_SELECT)                      
			BlockKeys[whichBlock].startSelected = TRUE;
		if (flags&SELKEYS_DESELECT)
			BlockKeys[whichBlock].endSelected = FALSE;
		if (flags&SELKEYS_SELECT)                      
			BlockKeys[whichBlock].endSelected = TRUE;
		}
	else if (whichEnd ==0)
		{
		if (flags&SELKEYS_DESELECT)
			BlockKeys[whichBlock].startSelected = FALSE;
		if (flags&SELKEYS_SELECT)                      
			BlockKeys[whichBlock].startSelected = TRUE;

		}
	else if (whichEnd ==1)
		{
		if (flags&SELKEYS_DESELECT)
			BlockKeys[whichBlock].endSelected = FALSE;
		if (flags&SELKEYS_SELECT)                      
			BlockKeys[whichBlock].endSelected = TRUE;
		}

	}

NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

}
void MasterBlockControl::SelectKeyByIndex(int i,BOOL sel)
{
	HoldTrack();
	int whichBlock = i /2;
	int whichEnd = i %2;

	if (sel) 
		{
		if (whichEnd == 0)
			BlockKeys[whichBlock].startSelected = TRUE;
		else BlockKeys[whichBlock].endSelected = TRUE;
		}
	else
		{
		if (whichEnd == 0)
			BlockKeys[whichBlock].startSelected = FALSE;
		else BlockKeys[whichBlock].endSelected = FALSE;
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

}

void MasterBlockControl::MoveKeys(ParamDimensionBase *dim,float delta,DWORD flags)
{
}

void MasterBlockControl::SetSelKeyCoords(TimeValue t, float val,DWORD flags)
{
//DebugPrint("Move key %d\n",t);
for (int i = 0; i < BlockKeys.Count(); i++)
	{
	if (BlockKeys[i].startSelected)
		{
//		TimeValue l = BlockKeys[i].end - BlockKeys[i].start;
		BlockKeys[i].start = t;
//		BlockKeys[i].end = t+l;

		}
	if (BlockKeys[i].endSelected)
		{
//		TimeValue l = BlockKeys[i].end - BlockKeys[i].start;
		BlockKeys[i].end = t;
//		BlockKeys[i].end = t+l;

		}
	}

NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

}


int MasterBlockControl::GetSelKeyCoords(TimeValue &t, float &val,	DWORD flags)
{
int ct = 0;
TimeValue at;
BOOL tfound = FALSE;

//DebugPrint("Get Sel key Coord\n");

if (relativeHit)
	return KEYS_MULTISELECTED;

for (int i = 0; i < BlockKeys.Count(); i++)
	{
	if (flags&KEYCOORDS_TIMEONLY)
		{
		if (BlockKeys[i].startSelected)
			{
			if (tfound)
				{
				if (BlockKeys[i].start!=at) 
					{
//DebugPrint("Get Sel key multi\n");
					return KEYS_MULTISELECTED;
					}

		
				}
			else 
				{			
				tfound = TRUE;
				at = BlockKeys[i].start;
				}
			}
		if (BlockKeys[i].endSelected)
			{
			if (tfound)
				{
				if (BlockKeys[i].end!=at) 
					{
//DebugPrint("Get Sel key multi\n");
					return KEYS_MULTISELECTED;
					}
				}
			else
				{			
				tfound = TRUE;
				at = BlockKeys[i].end;
				}
			}


		}
	else
		{
//DebugPrint("Get Sel key none\n");
		return KEYS_NONESELECTED;
		}



	}
if (tfound) 
	{
	t = at;
//DebugPrint("Get Sel key %d\n",t);
	return KEYS_COMMONTIME;
	} 
else {
//DebugPrint("Get Sel key none\n");

	return KEYS_NONESELECTED;
	}

}

int MasterBlockControl::HitTestTrack(TrackHitTab& hits,Rect& rcHit,Rect& rcTrack,float zoom,int scroll,DWORD flags)

{

	int left  = ScreenToTime(rcTrack.left,zoom,scroll) - 4;
	int right = ScreenToTime(rcTrack.right,zoom,scroll) + 4;
	int n = BlockKeys.Count();
	int y = (rcTrack.top+rcTrack.bottom)/2;	
	int y2 = (y + rcTrack.bottom)/2;


	int hitCount = 0;
	int setRelative = -1;
	relativeHit = FALSE;


//check individual keys first
	for (int i = (n-1); i >= 0; i-- ) 
//	for (int i = 0; i < n; i++ ) 
		{


if  ((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected))
{

			
		int sx = 0;
		int ex = 0;
		BOOL flip = FALSE;
		if (BlockKeys[i].start <= BlockKeys[i].end)
			{
			sx = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			ex = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			}
		else
			{
			ex = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			sx = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			flip = TRUE;
			}
		int mid = (sx+ex)/2;
		IPoint2 ml(mid-8,y),mr(mid+8,rcTrack.bottom);
		IPoint2 rl(rcHit.left,rcHit.top),rr(rcHit.right,rcHit.bottom);

		Box2 box(ml,mr);
		if ((sx>=rcHit.left)&&  (sx<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;

				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}

			}
		if ((ex>=rcHit.left)&&  (ex<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;



				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			}
//check if relative/absolute hit

		if (box.Contains(rl) && box.Contains(rr))
			{

				setRelative = i;
				TrackHitRecord rec(setRelative*2,2);
				hits.Append(1,&rec);
				relativeHit = TRUE;
				startRestoreState = BlockKeys[i].startSelected;
				endRestoreState = BlockKeys[i].endSelected;
				return TRACK_DONE;


			}

		else if ((rcHit.left>sx)&&  (rcHit.right<ex))
			{
			if ( (flags&HITTRACK_SELONLY) && 
				 ((!BlockKeys[i].endSelected) && (!BlockKeys[i].startSelected))) continue;
			if ( (flags&HITTRACK_UNSELONLY) && 
				 ((BlockKeys[i].endSelected) && (BlockKeys[i].startSelected))) continue;

			TrackHitRecord rec(i*2,1);
			hits.Append(1,&rec);
			hitCount++;
			if (flags&HITTRACK_ABORTONHIT) 
				return TRACK_DONE;
			}
}					
		}

//check individual keys first
	for (i = (n-1); i >= 0; i-- ) 
//	for (int i = 0; i < n; i++ ) 
		{



			
		int sx = 0;
		int ex = 0;
		BOOL flip = FALSE;
		if (BlockKeys[i].start <= BlockKeys[i].end)
			{
			sx = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			ex = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			}
		else
			{
			ex = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			sx = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			flip = TRUE;
			}
		int mid = (sx+ex)/2;
		IPoint2 ml(mid-8,y),mr(mid+8,rcTrack.bottom);
		IPoint2 rl(rcHit.left,rcHit.top),rr(rcHit.right,rcHit.bottom);

		Box2 box(ml,mr);
		if ((sx>=rcHit.left)&&  (sx<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;

				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}

			}
		if ((ex>=rcHit.left)&&  (ex<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;



				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			}
//check if relative/absolute hit

		if (box.Contains(rl) && box.Contains(rr))
			{

				setRelative = i;
				TrackHitRecord rec(setRelative*2,2);
				hits.Append(1,&rec);
				relativeHit = TRUE;
				startRestoreState = BlockKeys[i].startSelected;
				endRestoreState = BlockKeys[i].endSelected;
				return TRACK_DONE;


			}

		else if ((rcHit.left>sx)&&  (rcHit.right<ex))
			{
			if ( (flags&HITTRACK_SELONLY) && 
				 ((!BlockKeys[i].endSelected) && (!BlockKeys[i].startSelected))) continue;
			if ( (flags&HITTRACK_UNSELONLY) && 
				 ((BlockKeys[i].endSelected) && (BlockKeys[i].startSelected))) continue;

			TrackHitRecord rec(i*2,1);
			hits.Append(1,&rec);
			hitCount++;
			if (flags&HITTRACK_ABORTONHIT) 
				return TRACK_DONE;
			}
					
		}

//now check blocks
/*
	for (i = (n-1); i >= 0; i-- ) 
		{



			
		int sx = 0;
		int ex = 0;
		BOOL flip = FALSE;
		if (BlockKeys[i].start <= BlockKeys[i].end)
			{
			sx = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			ex = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			}
		else
			{
			ex = TimeToScreen(BlockKeys[i].start,zoom,scroll);
			sx = TimeToScreen(BlockKeys[i].end,zoom,scroll);
			flip = TRUE;
			}
		int mid = (sx+ex)/2;
		IPoint2 ml(mid-8,y),mr(mid+8,rcTrack.bottom);
		IPoint2 rl(rcHit.left,rcHit.top),rr(rcHit.right,rcHit.bottom);

		Box2 box(ml,mr);

		if ((sx>=rcHit.left)&&  (sx<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;

				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}

			}
		if ((ex>=rcHit.left)&&  (ex<=rcHit.right))
			   
			{
			if (!flip)
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].endSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].endSelected) )) continue;



				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			else
				{
				if ( (flags&HITTRACK_SELONLY) && 
					 ((!BlockKeys[i].startSelected) )) continue;
				if ( (flags&HITTRACK_UNSELONLY) && 
					 ((BlockKeys[i].startSelected) )) continue;

				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				if (flags&HITTRACK_ABORTONHIT) 
					return TRACK_DONE;
				}
			}

//check if relative/absolute hit

		if (box.Contains(rl) && box.Contains(rr))
			{

				setRelative = i;
				TrackHitRecord rec(setRelative*2,2);
				hits.Append(1,&rec);
				relativeHit = TRUE;
				startRestoreState = BlockKeys[i].startSelected;
				endRestoreState = BlockKeys[i].endSelected;
				return TRACK_DONE;


			}

		else if ((rcHit.left>sx)&&  (rcHit.right<ex))
			{
			if ( (flags&HITTRACK_SELONLY) && 
				 ((!BlockKeys[i].endSelected) && (!BlockKeys[i].startSelected))) continue;
			if ( (flags&HITTRACK_UNSELONLY) && 
				 ((BlockKeys[i].endSelected) && (BlockKeys[i].startSelected))) continue;

			TrackHitRecord rec(i*2,1);
			hits.Append(1,&rec);
			hitCount++;
			if (flags&HITTRACK_ABORTONHIT) 
				return TRACK_DONE;
			}
					
		}
*/
	return TRACK_DONE;

}

#define HSIZE 4
#define VSIZE 5
inline void PaintKey(HDC hdc, int x, int y) {
	Ellipse(hdc,x-HSIZE,y-VSIZE,x+HSIZE,y+VSIZE);
	}


int MasterBlockControl::PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags)
{

	HPEN penIn = CreatePen(PS_SOLID,0,RGB(0,0,200));
	HPEN Redpen = CreatePen(PS_SOLID,0,RGB(200,0,0));
	HPEN RedLightpen = CreatePen(PS_SOLID,0,RGB(210,187,187));
	HPEN oldPen;

	int xl = rcPaint.left;
	int xr = rcPaint.right;
	int bot = rcPaint.bottom;
	int top = rcPaint.top;
	HGDIOBJ oldFt;  
	int		oldBkMode;


	// Kludge to paint the waveform if min == max

	oldPen = (HPEN)SelectObject(hdc,Redpen);
	oldFt = SelectObject( hdc, GetCOREInterface()->GetAppHFont());
	oldBkMode = GetBkMode( hdc); 
	SetBkMode( hdc, TRANSPARENT); 
	HBRUSH selBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH unselBrush = (HBRUSH)GetStockObject(GRAY_BRUSH); 

	for (int i = 0; i < BlockKeys.Count();i++)
		{
		if (((!BlockKeys[i].startSelected) && (!BlockKeys[i].endSelected)))
			{
			int sx,ex;
			sx = BlockKeys[i].start;
			ex = BlockKeys[i].end;
			sx = TimeToScreen(sx,zoom,scroll);
			ex = TimeToScreen(ex,zoom,scroll);
			RECT r;
			r.left = sx-HSIZE-2;
			r.top = top;
			r.right = ex+HSIZE+2;
			r.bottom = bot;

			COLORREF c;
		

			if ((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected))
				{	
				Color cc = Blocks[BlockKeys[i].block_id]->color;
				cc.r += (1.0f-cc.r) *.75f;
				cc.g += (1.0f-cc.g) *.75f;
				cc.b += (1.0f-cc.b) *.75f;
				c = cc.toRGB();
				}
			else
				{
				c = Blocks[BlockKeys[i].block_id]->color.toRGB();
				}

			HBRUSH sBrush = CreateSolidBrush(c);
			SelectObject(hdc,sBrush);
			Rectangle(hdc,sx-HSIZE-2,top,ex+HSIZE+2,bot-1);	
			DeleteObject(sBrush);

			int subid = BlockKeys[i].block_id;

			TSTR name = SubAnimName(subid+1);
			WhiteRect3D(hdc,r,FALSE);
			RECT tr;
			tr.left = sx-HSIZE-2;
			tr.top = top+1;
			tr.right = ex+HSIZE+2;
			tr.bottom = bot-4;

			DrawText( hdc, (TCHAR*)name, names[subid]->length(),&tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			TSTR abs ="A\n";
			TSTR rel ="R\n";
			if (BlockKeys[i].relative)
				DrawText( hdc, (TCHAR*)rel, 1,&tr, DT_CENTER  | DT_BOTTOM | DT_SINGLELINE );
			else DrawText( hdc, (TCHAR*)abs, 1,&tr, DT_CENTER  |  DT_BOTTOM | DT_SINGLELINE );

			if  (BlockKeys[i].startSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,sx,bot-VSIZE-2);

			if  (BlockKeys[i].endSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,ex,bot-VSIZE-2);

			}
		}

	for (i = 0; i < BlockKeys.Count();i++)
		{
		if (((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected)))
			{
			int sx,ex;
			sx = BlockKeys[i].start;
			ex = BlockKeys[i].end;
			sx = TimeToScreen(sx,zoom,scroll);
			ex = TimeToScreen(ex,zoom,scroll);
			RECT r;
			r.left = sx-HSIZE-2;
			r.top = top;
			r.right = ex+HSIZE+2;
			r.bottom = bot;

			COLORREF c;
		

			if ((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected))
				{	
				Color cc = Blocks[BlockKeys[i].block_id]->color;
				cc.r += (1.0f-cc.r) *.75f;
				cc.g += (1.0f-cc.g) *.75f;
				cc.b += (1.0f-cc.b) *.75f;
				c = cc.toRGB();
				}
			else
				{
				c = Blocks[BlockKeys[i].block_id]->color.toRGB();
				}

			HBRUSH sBrush = CreateSolidBrush(c);
			SelectObject(hdc,sBrush);
			Rectangle(hdc,sx-HSIZE-2,top,ex+HSIZE+2,bot-1);	
			DeleteObject(sBrush);

			int subid = BlockKeys[i].block_id;

			TSTR name = SubAnimName(subid+1);
			WhiteRect3D(hdc,r,FALSE);
			RECT tr;
			tr.left = sx-HSIZE-2;
			tr.top = top+1;
			tr.right = ex+HSIZE+2;
			tr.bottom = bot-4;

			DrawText( hdc, (TCHAR*)name, names[subid]->length(),&tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			TSTR abs ="A\n";
			TSTR rel ="R\n";
			if (BlockKeys[i].relative)
				DrawText( hdc, (TCHAR*)rel, 1,&tr, DT_CENTER  | DT_BOTTOM | DT_SINGLELINE );
			else DrawText( hdc, (TCHAR*)abs, 1,&tr, DT_CENTER  |  DT_BOTTOM | DT_SINGLELINE );

			if  (BlockKeys[i].startSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,sx,bot-VSIZE-2);

			if  (BlockKeys[i].endSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,ex,bot-VSIZE-2);

			}
		}

	for (i = 0; i < BlockKeys.Count();i++)
		{
		if (((BlockKeys[i].startSelected) && (BlockKeys[i].endSelected)))
			{
			int sx,ex;
			sx = BlockKeys[i].start;
			ex = BlockKeys[i].end;
			sx = TimeToScreen(sx,zoom,scroll);
			ex = TimeToScreen(ex,zoom,scroll);
			RECT r;
			r.left = sx-HSIZE-2;
			r.top = top;
			r.right = ex+HSIZE+2;
			r.bottom = bot;

			COLORREF c;
		

			if ((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected))
				{	
				Color cc = Blocks[BlockKeys[i].block_id]->color;
				cc.r += (1.0f-cc.r) *.75f;
				cc.g += (1.0f-cc.g) *.75f;
				cc.b += (1.0f-cc.b) *.75f;
				c = cc.toRGB();
				}
			else
				{
				c = Blocks[BlockKeys[i].block_id]->color.toRGB();
				}

			HBRUSH sBrush = CreateSolidBrush(c);
			SelectObject(hdc,sBrush);
			Rectangle(hdc,sx-HSIZE-2,top,ex+HSIZE+2,bot-1);	
			DeleteObject(sBrush);

			int subid = BlockKeys[i].block_id;

			TSTR name = SubAnimName(subid+1);
			WhiteRect3D(hdc,r,FALSE);
			RECT tr;
			tr.left = sx-HSIZE-2;
			tr.top = top+1;
			tr.right = ex+HSIZE+2;
			tr.bottom = bot-4;

			DrawText( hdc, (TCHAR*)name, names[subid]->length(),&tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			TSTR abs ="A\n";
			TSTR rel ="R\n";
			if (BlockKeys[i].relative)
				DrawText( hdc, (TCHAR*)rel, 1,&tr, DT_CENTER  | DT_BOTTOM | DT_SINGLELINE );
			else DrawText( hdc, (TCHAR*)abs, 1,&tr, DT_CENTER  |  DT_BOTTOM | DT_SINGLELINE );

			if  (BlockKeys[i].startSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,sx,bot-VSIZE-2);

			if  (BlockKeys[i].endSelected) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}

			PaintKey(hdc,ex,bot-VSIZE-2);

			}
		}



	SelectObject(hdc,oldPen);
	SelectObject( hdc, oldFt); 		  
	SetBkMode(hdc,oldBkMode);

	DeleteObject(penIn);
	DeleteObject(Redpen);
	DeleteObject(RedLightpen);
	DeleteObject(selBrush);
	DeleteObject(unselBrush);
	
	return TRACK_DONE;

}





int MasterBlockControl::PaintFCurves(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags )
	{


	HPEN penIn = CreatePen(PS_SOLID,0,RGB(0,0,200));
	HPEN Redpen = CreatePen(PS_SOLID,0,RGB(200,0,0));
	HPEN RedLightpen = CreatePen(PS_SOLID,0,RGB(210,187,187));
	HPEN oldPen;

	int mid = (rcPaint.bottom-30);

	int xl = rcPaint.left;
	int xr = rcPaint.right;
//	int bot = rcPaint.bottom;
//	int top = rcPaint.top;
	int bot = mid+20;
	int top = mid-20;
	HGDIOBJ oldFt;  
	int		oldBkMode;


	// Kludge to paint the waveform if min == max

	oldPen = (HPEN)SelectObject(hdc,Redpen);
	oldFt = SelectObject( hdc, GetCOREInterface()->GetAppHFont());
	oldBkMode = GetBkMode( hdc); 
	SetBkMode( hdc, TRANSPARENT); 
	HBRUSH selBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH unselBrush = (HBRUSH)GetStockObject(GRAY_BRUSH); 

	for (int i = 0; i < BlockKeys.Count();i++)
		{
		int sx,ex;
		sx = BlockKeys[i].start;
		ex = BlockKeys[i].end;
		sx = TimeToScreen(sx,tzoom,tscroll);
		ex = TimeToScreen(ex,tzoom,tscroll);
		RECT r;
		r.left = sx-HSIZE-2;
		r.top = top;
		r.right = ex+HSIZE+2;
		r.bottom = bot;

		COLORREF c;
		

		if (((BlockKeys[i].startSelected) || (BlockKeys[i].endSelected)) && (isCurveSelected))
			{	
//			SelectObject(hdc,GetStockObject(LTGRAY_BRUSH  ));
			Color cc = Blocks[BlockKeys[i].block_id]->color;
			cc.r += (1.0f-cc.r) *.75f;
			cc.g += (1.0f-cc.g) *.75f;
			cc.b += (1.0f-cc.b) *.75f;
			c = cc.toRGB();
			}
		else
			{
			c = Blocks[BlockKeys[i].block_id]->color.toRGB();
			}
			//SelectObject(hdc,GetStockObject(DKGRAY_BRUSH ));

		HBRUSH sBrush = CreateSolidBrush(c);
//		HPEN Colorpen = CreatePen(PS_SOLID,0,c);
		SelectObject(hdc,sBrush);
		Rectangle(hdc,sx-HSIZE-2,top,ex+HSIZE+2,bot-1);	
		DeleteObject(sBrush);

		int subid = BlockKeys[i].block_id;

		TSTR name = SubAnimName(subid+1);
		WhiteRect3D(hdc,r,FALSE);
		RECT tr;
		tr.left = sx-HSIZE-2;
		tr.top = top+1;
		tr.right = ex+HSIZE+2;
		tr.bottom = bot-4;

		DrawText( hdc, (TCHAR*)name, names[subid]->length(),&tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
		TSTR abs ="A\n";
		TSTR rel ="R\n";
		if (BlockKeys[i].relative)
			DrawText( hdc, (TCHAR*)rel, 1,&tr, DT_CENTER  | DT_BOTTOM | DT_SINGLELINE );
		else DrawText( hdc, (TCHAR*)abs, 1,&tr, DT_CENTER  |  DT_BOTTOM | DT_SINGLELINE );

		if  (BlockKeys[i].startSelected) {
			SelectObject(hdc,selBrush);
		} else {
			SelectObject(hdc,unselBrush);
			}

		if (isCurveSelected)
			PaintKey(hdc,sx,bot-VSIZE-2);

		if  (BlockKeys[i].endSelected) {
			SelectObject(hdc,selBrush);
		} else {
			SelectObject(hdc,unselBrush);
			}

		if (isCurveSelected)
			PaintKey(hdc,ex,bot-VSIZE-2);

		}


	SelectObject(hdc,oldPen);
	SelectObject( hdc, oldFt); 		  
	SetBkMode(hdc,oldBkMode);

	DeleteObject(penIn);
	DeleteObject(Redpen);
	DeleteObject(RedLightpen);
	DeleteObject(selBrush);
	DeleteObject(unselBrush);
	
	return TRACK_DONE;

	return 0;
	}

int MasterBlockControl::HitTestFCurves(ParamDimensionBase *dim,TrackHitTab& hits, 
									   Rect& rcHit, Rect& rcTrack,
									   float zoom, int scroll,
									   float vzoom,int vscroll, DWORD flags)
{

	int left  = ScreenToTime(rcTrack.left,zoom,scroll) - 4;
	int right = ScreenToTime(rcTrack.right,zoom,scroll) + 4;
	int n = BlockKeys.Count();


	if (flags&HITTRACK_SELONLY && (!isCurveSelected)) {
		return HITCURVE_NONE;
		}		

//	int y = (rcTrack.top+rcTrack.bottom)/2;	
//	int y2 = (y + rcTrack.bottom)/2;
	int y = rcTrack.bottom-30;	
	int y2 = (y + rcTrack.bottom)/2;

//DebugPrint("Flags selonly %d\n",flags&HITTRACK_SELONLY);
//DebugPrint("Flags unselonly %d\n",flags&HITTRACK_UNSELONLY);
//DebugPrint("Flags abort on hit %d\n",flags&HITTRACK_ABORTONHIT);


	int hitCount = 0;
	int setRelative = -1;

	for (int i = 0; i < n; i++ ) 
		{

		if ( (flags&HITTRACK_SELONLY) && 
			 ((!BlockKeys[i].startSelected) && (!BlockKeys[i].endSelected))) continue;
		if ( (flags&HITTRACK_UNSELONLY) && 
			 ((BlockKeys[i].startSelected) && (BlockKeys[i].endSelected))) continue;

			
	
		int sx = TimeToScreen(BlockKeys[i].start,zoom,scroll);
		int ex = TimeToScreen(BlockKeys[i].end,zoom,scroll);
		int mid = (sx+ex)/2;
		if ((sx>=rcHit.left) &&  (sx<=rcHit.right) && (rcHit.top > (y-30)))
			   
			{

//			if ( ((flags&HITTRACK_SELONLY) && (BlockKeys[i].startSelected)) ||
//			     ((flags&HITTRACK_UNSELONLY) && (!BlockKeys[i].startSelected)) )
				{
				TrackHitRecord rec(i*2,0);
				hits.Append(1,&rec);
				hitCount++;
				}

//			if (flags&HITTRACK_ABORTONHIT) 
//				return TRACK_DONE;
			}
		if ((ex>=rcHit.left)&&  (ex<=rcHit.right) && (rcHit.top > (y-30)))
			   
			{
//			if ( ((flags&HITTRACK_SELONLY) && (BlockKeys[i].endSelected)) ||
//			     ((flags&HITTRACK_UNSELONLY) && (!BlockKeys[i].endSelected)) )
				{
				TrackHitRecord rec(i*2+1,0);
				hits.Append(1,&rec);
				hitCount++;
				}

//			if (flags&HITTRACK_ABORTONHIT) return TRACK_DONE;
			}

//check if relative/absolute hit
		if (rcHit.Contains(IPoint2(mid,y2)))
			{
			setRelative = i;
			}

		else if ((rcHit.left>sx)&&  (rcHit.right<ex) && (rcHit.top > (y-30)))
			{
//			if ( ((flags&HITTRACK_SELONLY) && ((BlockKeys[i].endSelected) ||(BlockKeys[i].startSelected))) ||
//			     ((flags&HITTRACK_UNSELONLY) && ((!BlockKeys[i].endSelected) || (!BlockKeys[i].startSelected))) )
				{
				TrackHitRecord rec(i*2,1);
				hits.Append(1,&rec);
				hitCount++;
				}
//			if (flags&HITTRACK_ABORTONHIT) 
//				return TRACK_DONE;
			}

					
		}

	if ((hitCount ==0) && (setRelative != -1))
		{
		TrackHitRecord rec(setRelative*2,2);
		hits.Append(1,&rec);
		hitCount++;
		}

	int result = HITCURVE_NONE;
	if (hitCount >0)
		result = HITCURVE_KEY;
	if (((!isCurveSelected) && result==HITCURVE_KEY)  || ((!isCurveSelected) && (rcHit.top > (y-20))) )
		{
		result = HITCURVE_WHOLE;
		}

if (result != HITCURVE_NONE)
{
/*DebugPrint("Flags HITTRACK_SELONLY %d HITTRACK_UNSELONLY %d HITTRACK_ABORT %d \n",
			 flags&HITTRACK_SELONLY,
			 flags&HITTRACK_UNSELONLY,
			 flags&HITTRACK_ABORTONHIT
			 );
DebugPrint("Result HITCURVE_WHOLE %d HITCURVE_KEY %d\n",
			 result&HITCURVE_WHOLE,
			 result&HITCURVE_KEY
			 );
*/

}
//DebugPrint("Result %d\n",result);
	return result;

/*
	if (hitCount >0)
		{
		DebugPrint("Hit keys\n");
		if (isCurveSelected)
			return HITCURVE_KEY;
		else
			{ 
			if (flags&HITTRACK_SELONLY)
				return HITCURVE_NONE;
			else return HITCURVE_WHOLE;
			}
		}
	else if (rcHit.top > (y-20))
		{
		DebugPrint("Hit whole\n");
		if (flags&HITTRACK_SELONLY)
			return HITCURVE_NONE;
		else return HITCURVE_WHOLE;
		}
	else
		{
		DebugPrint("Hit none\n");
		return HITCURVE_NONE;
		}
*/
}


#define BLOCKKEYSCOUNT_CHUNK	0x01010
#define BLOCKKEYS_CHUNK			0x01020
#define NAMECOUNT_CHUNK			0x01030
#define NAME_CHUNK				0x01040
#define NONAME_CHUNK			0x01050
#define LOCK_CHUNK				0x01060
#define RANGE_CHUNK				0x01070



IOResult MasterBlockControl::Save(ISave *isave)
	{		
	ULONG nb;	
//names
//block keys
	int count = BlockKeys.Count();
	isave->BeginChunk(BLOCKKEYSCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	isave->BeginChunk(BLOCKKEYS_CHUNK);
	for (int i=0; i<count; i++) 
		{
		isave->Write(&BlockKeys[i],sizeof(BlockKeysClass),&nb);
		}
	isave->EndChunk();

	count = Blocks.Count();
	isave->BeginChunk(NAMECOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	for (i=0; i<count; i++) {
		if (names[i]) {
			isave->BeginChunk(NAME_CHUNK);
			isave->WriteWString(*names[i]);
			isave->EndChunk();
		} else {
			isave->BeginChunk(NONAME_CHUNK);
			isave->EndChunk();
			}
		}
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&rangeUnlocked,sizeof(BOOL),&nb);			
	isave->EndChunk();

	isave->BeginChunk(RANGE_CHUNK);
	isave->Write(&range,sizeof(range),&nb);			
	isave->EndChunk();

	return IO_OK;
	}

class MasterBlockPostLoadCallback:public  PostLoadCallback
{
public:
	MasterBlockControl      *s;
	MasterBlockPostLoadCallback(MasterBlockControl *r) {s=r;}
	void proc(ILoad *iload);
};

void MasterBlockPostLoadCallback::proc(ILoad *iload)
{

	if (s)
		{
		for (int i = 0; i < s->Blocks.Count();i++)
			{
			s->UpdateControl(i);
			}

		}

	delete this;

}
 


IOResult MasterBlockControl::Load(ILoad *iload)
	{
	int ID =  0;
	ULONG nb;
	IOResult res = IO_OK;
	int ix = 0;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		ID = iload->CurChunkID();
		if (ID ==BLOCKKEYSCOUNT_CHUNK)
			{
			int ct;
			iload->Read(&ct, sizeof(ct), &nb);
			BlockKeys.SetCount(ct);
			}
		else if (ID ==LOCK_CHUNK)
			{
			iload->Read(&rangeUnlocked, sizeof(BOOL), &nb);
			}

		else if (ID == BLOCKKEYS_CHUNK)
			{
			for (int i = 0; i < BlockKeys.Count(); i++)
				{
				BlockKeysClass b;
				iload->Read(&b, sizeof(BlockKeysClass), &nb);
				BlockKeys[i] = b;
				}
			}
		else if (ID == NAMECOUNT_CHUNK)
			{
			int ct;
			iload->Read(&ct, sizeof(ct), &nb);
			names.SetCount(ct);
			Blocks.SetCount(ct);
			for (int i=0; i<ct; i++) 
				{
				names[i] = NULL;
				Blocks[i] = NULL;
				}

			}
		else if (ID == NAME_CHUNK)
			{
			TCHAR *buf;
			iload->ReadWStringChunk(&buf);
			names[ix++] = new TSTR(buf);
			}
		else if (ID == NONAME_CHUNK)
			{
			ix++;
			}
		else if (ID == RANGE_CHUNK)
			{
			res=iload->Read(&range,sizeof(range),&nb);
			}


		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
//rebuild all tempcontrols	
	MasterBlockPostLoadCallback* mplcb = new MasterBlockPostLoadCallback(this);
	iload->RegisterPostLoadCallback(mplcb);

	return IO_OK;
	}


//--------------------------------------------------------------------

int MasterBlockControl::LoadBlock()

{
//need to pop up a standard load dialog
Interface *ip = GetCOREInterface();
HWND hWnd = ip->GetMAXHWnd();

static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_BLKFILES));
fl.Append( _T("*.blk"));		
TSTR title = GetString(IDS_PW_LOADBLOCK);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = hWnd;
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
ofn.lpstrDefExt     = _T("blk");
ofn.lpstrTitle      = title;


if (GetOpenFileName(&ofn)) {
//load stuff here  stuff here
//merge file
//	theHold.Begin();
	theHold.Suspend();

	ip->MergeFromFile(fname, TRUE,TRUE, FALSE,MERGE_DUPS_DELOLD);

//get selected block
	INode *node = ip->GetSelNode(0);
	if (node != NULL)
		{
		ObjectState os = node->EvalWorldState(0);

		ControlContainerObject *cobj;// = new ControlContainerObject();
		ControlContainerObject *tcobj;

		tcobj = (ControlContainerObject *) os.obj;
		cobj = (ControlContainerObject *) tcobj->Clone();
		
		propContainer = cobj;

		ip->DeleteNode(node);
		theHold.Resume();

//		Color c;
//		TimeValue s,e;
		propContainer->pblock2->GetValue(container_color,0,propColor,FOREVER);
		propContainer->pblock2->GetValue(container_start,0,propStart,FOREVER);
		propContainer->pblock2->GetValue(container_end,0,propEnd,FOREVER);
		TCHAR *c;

		propContainer->pblock2->GetValue(container_blockname,0,c,FOREVER);
		propBlockName = c;

//create a new block
//copy over the subs
//copy over the names
//pop up a attach dialog so we can associate it with correct controller
		DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ATTACH_CONTROLS_DIALOG),
				hWnd, AttachBlockDlgProc, (LPARAM)this);
		cobj->DeleteThis();
//		theHold.Suspend();
		}
	else
		theHold.Resume();


//	theHold.Cancel();
	return 1;
	}
return 0;

}
//--------------------------------------------------------------------
int MasterBlockControl::DeleteBlock(int whichBlock)

{
//names.Delete(1,whichBlock);
if ((whichBlock < 0) || (whichBlock >= Blocks.Count())) return 0;
//notify all back pointer that there block is about to be deleted
for (int i = 0;i <Blocks[whichBlock]->backPointers.Count();i++)
	{
//lock at the bockid table
//	int subCount = Blocks[whichBlock]->backPointers[i]->blockID.Count();
	if (Blocks[whichBlock]->backPointers[i])
		{
		for (int j=0; j< Blocks[whichBlock]->backPointers[i]->blockID.Count(); j++)
			{
			if (Blocks[whichBlock]->backPointers[i]->blockID[j] == whichBlock)
				{
				Blocks[whichBlock]->backPointers[i]->blockID.Delete(j,1);
				Blocks[whichBlock]->backPointers[i]->subID.Delete(j,1);
				j--;
				}
			else if (Blocks[whichBlock]->backPointers[i]->blockID[j] > whichBlock)
				{
				Blocks[whichBlock]->backPointers[i]->blockID[j] -= 1;
				}
			}
		}

	}

for (i = 0;i <Blocks[whichBlock]->externalBackPointers.Count();i++)
	{
//lock at the bockid table
//	int subCount = Blocks[whichBlock]->backPointers[i]->blockID.Count();
	if (Blocks[whichBlock]->externalBackPointers[i])
		{
		for (int j=0; j< Blocks[whichBlock]->externalBackPointers[i]->blockID.Count(); j++)
			{
			if (Blocks[whichBlock]->externalBackPointers[i]->blockID[j] == whichBlock)
				{
				Blocks[whichBlock]->externalBackPointers[i]->blockID.Delete(j,1);
				Blocks[whichBlock]->externalBackPointers[i]->subID.Delete(j,1);
				j--;
				}
			else if (Blocks[whichBlock]->externalBackPointers[i]->blockID[j] > whichBlock)
				{
				Blocks[whichBlock]->externalBackPointers[i]->blockID[j] -= 1;
				}
			}
		}

	}


//loop through all block keys and reduce the indices by one
DeleteReference(whichBlock+1);
//Blocks[whichBlock]->DeleteThis();
//Blocks[whichBlock]= NULL;
Blocks.Delete(whichBlock,1);
names.Delete(whichBlock,1);

for (i = 0; i < BlockKeys.Count(); i++)
	{
	if (BlockKeys[i].block_id == whichBlock)
		{
		BlockKeys.Delete(i,1);
		i--;
		}
	else if (BlockKeys[i].block_id > whichBlock)
		BlockKeys[i].block_id -=1;

	}
NotifyDependents(FOREVER,0,REFMSG_CHANGE);
NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
return 1;
}

int MasterBlockControl::SaveBlock(int whichBlock)

{
//build a container node
ControlContainerObject *cobj = new ControlContainerObject();

	// Create a new object through the CreateInstance() API

Interface *ip = GetCOREInterface();
/*Object *obj = (Object*)ip->CreateInstance(
	GEOMOBJECT_CLASS_ID,
	Class_ID(CYLINDER_CLASS_ID,0));
assert(obj);
*/

//ntrolContainerObject *cobj = (ControlContainerObject*) obj;

//copy data in color start end time
cobj->pblock2->SetValue(container_color,0,Blocks[whichBlock]->color);
cobj->pblock2->SetValue(container_start,0,Blocks[whichBlock]->start);
cobj->pblock2->SetValue(container_end,0,Blocks[whichBlock]->end);

TCHAR *n = *names[whichBlock];
cobj->pblock2->SetValue(container_blockname,0,n);

// Get a hold of the parameter block
//copy names selection in
//cobj->pblock2->SetValue(container_name,0,names[i]);
for (int i = 0; i < Blocks[whichBlock]->controls.Count(); i++)
	{
//copy the sub anims in
//	cobj->pblock2->SetValue(container_refs,0,Blocks[whichBlock]->controls[i]->Clone(),1);
	ReferenceTarget *c = (ReferenceTarget *) Blocks[whichBlock]->controls[i]->Clone();
	cobj->pblock2->Append(container_refs,1,&c);
	TCHAR *s = *Blocks[whichBlock]->names[i];
	cobj->pblock2->Append(container_names,1,&s);
//	cobj->pblock2->SetValue(container_names,0,s);
	}
for (i = 0; i < Blocks[whichBlock]->controls.Count(); i++)
	{
	TCHAR *c;
	cobj->pblock2->GetValue(container_names,0,c,FOREVER,i);
//	DebugPrint("save str %s\n",c);
	}
// Create a node in the scene that references the derived object

INode *node = ip->CreateObjectNode(cobj);
	
// Name the node and make the name unique.
TSTR name(_T("47df4589"));
//ip->MakeNameUnique(name);
node->SetName(name);

//save selection unselect all select it
theHold.Suspend();
//call save selected
ip->SelectNode(node,1);
//ip->FileSaveSelected();

HWND hWnd = ip->GetMAXHWnd();
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_BLKFILES));
fl.Append( _T("*.blk"));		
TSTR title = GetString(IDS_PW_SAVEBLOCK);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = hWnd;
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
ofn.lpstrDefExt     = _T("blk");
ofn.lpstrTitle      = title;

tryAgain:
if (GetSaveFileName(&ofn)) {
	if (DoesFileExist(fname)) {
		TSTR buf1;
		TSTR buf2 = GetString(IDS_PW_SAVEBLOCK);
		buf1.printf(GetString(IDS_PW_FILEEXISTS),fname);
		if (IDYES!=MessageBox(
			hWnd,
			buf1,buf2,MB_YESNO|MB_ICONQUESTION)) {
			goto tryAgain;
			}
		}
	ip->FileSaveSelected(fname);
//reapply old selection
//delete container node
	ip->DeleteNode(node,FALSE);
	theHold.Resume();
//	theHold.Cancel();
	return 1;

	}




//reapply old selection
//delete container node
ip->DeleteNode(node,FALSE);
theHold.Resume();
//theHold.Cancel();
return 0;

}


int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			name = Nodes[0]->GetName();
			return 1;
			}
	TSTR s;
	rmaker->GetClassName(s);
	TSTR b("blank");
	if (name == b)
		{
		if ( (rmaker->SuperClassID() == MATERIAL_CLASS_ID) || 
			 (rmaker->SuperClassID() == TEXMAP_CLASS_ID) || 
			 (rmaker->SuperClassID() == ATMOSPHERIC_CLASS_ID) )
			{
			 rmaker->GetClassName(name);
			 return 1;
			}

		}
//	DebugPrint("dep %s %d\n",s,rmaker->SuperClassID());
//MATERIAL_CLASS_ID
//TEXMAP_CLASS_ID
//ATMOSPHERIC_CLASS_ID
     return 0;              
	}

int MasterBlockControl::AddBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum, NameList &names)

{
MyEnumProc dep;             
TSTR b("");
dep.name =  b;
anim->EnumDependents(&dep);
//TSTR nodeName = TSTR( dep.Nodes[0]->GetName());
TSTR nodeName = dep.name;
TSTR np = TSTR(client->SubAnimName(subNum));
TSTR Slash("/");
nodeName += Slash;
nodeName += np;
TSTR *st = new TSTR(nodeName);
names.Append(1,&st,1);
return 1;
}

TSTR* MasterBlockControl::GetBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum)

{

MyEnumProc dep;              
TSTR b("");
dep.name =  b;
anim->EnumDependents(&dep);
//TSTR nodeName = TSTR( dep.Nodes[0]->GetName());
TSTR nodeName = dep.name;
TSTR np = TSTR(client->SubAnimName(subNum));
TSTR Slash("/");
nodeName += Slash;
nodeName += np;
TSTR *st = new TSTR(nodeName);
return st;
}



Control* MasterBlockControl::BuildListControl(TrackViewPick res, BOOL &createdList)
{
Control *list=NULL;
createdList = FALSE;
if ((res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) && (res.client->ClassID() != Class_ID(FLOATLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_FLOAT_CLASS_ID,
	Class_ID(FLOATLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) && (res.client->ClassID() == Class_ID(FLOATLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) && (res.client->ClassID() != Class_ID(POSLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_POSITION_CLASS_ID,
	Class_ID(POSLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) && (res.client->ClassID() == Class_ID(POSLIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID) && (res.client->ClassID() != Class_ID(POINT3LIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_POINT3_CLASS_ID,
	Class_ID(POINT3LIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID) && (res.client->ClassID() == Class_ID(POINT3LIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}

else if ((res.anim->SuperClassID() == CTRL_POINT4_CLASS_ID) && (res.client->ClassID() != Class_ID(POINT4LIST_CONTROL_CLASS_ID,0)))
{
	list = (Control*)GetCOREInterface()->CreateInstance(
		CTRL_POINT4_CLASS_ID,
		Class_ID(POINT4LIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
}
else if ((res.anim->SuperClassID() == CTRL_POINT4_CLASS_ID) && (res.client->ClassID() == Class_ID(POINT4LIST_CONTROL_CLASS_ID,0)))
{
	list =  (Control *)res.client;
}

else if ((res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) && (res.client->ClassID() != Class_ID(ROTLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_ROTATION_CLASS_ID,
	Class_ID(ROTLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) && (res.client->ClassID() == Class_ID(ROTLIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) && (res.client->ClassID() != Class_ID(SCALELIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_SCALE_CLASS_ID,
	Class_ID(SCALELIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) && (res.client->ClassID() == Class_ID(SCALELIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
return list;
}

Control* MasterBlockControl::BuildSlave(TrackViewPick res,Control* list, BOOL createdList)
{
int count = list->NumSubs()-2;
Control *slave = NULL;
for (int i = 0; i < count; i++)
	{
	if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEFLOAT_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEPOS_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEPOINT3_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEROTATION_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVESCALE_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	}
BOOL isRotation = FALSE;
BOOL isFloat = FALSE;
if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) 
	{
	slave = (Control*)new SlaveFloatControl;
	isFloat = TRUE;
	}
#ifndef NO_CONTROLLER_SLAVE_POSITION
else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) 
	slave = (Control*)new SlavePosControl;
#endif
else if (res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID) 
	slave = (Control*)new SlavePoint3Control;
#ifndef NO_CONTROLLER_SLAVE_ROTATION
else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) 
	{
	slave = (Control*)new SlaveRotationControl;
	isRotation = TRUE;
	}
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) 
	slave = (Control*)new SlaveScaleControl;
#endif
if (createdList)
	{
//	if (isFloat)
	if (0)
		{
		list->AssignController(slave,count);
		list->AssignController(res.anim->Clone(),count+1);
		}	
	else
		{
		if (GetCOREInterface()->GetCommandPanelTaskMode() == TASK_MODE_MOTION)
			res.anim->EndEditParams(iop,END_EDIT_REMOVEUI,NULL);

		list->AssignController(res.anim->Clone(),count);
		list->AssignController(slave,count+1);
		}	
	}
else
	{
	list->AssignController(slave,count);
	}
return slave;
}




//watje 4-24-99
int MasterBlockControl::AppendBlockNoSlave(BlockControl *b,int i, TSTR *name, Control *bdata)
{

int where = -1;

b->ReplaceReference(i,(Control *) bdata->Clone());

int bc;
if (where == -1)
	bc = Blocks.Count()-1;
else bc = where;





b->tempControls[i] = (Control *) bdata->Clone();

b->backPointers[i] = NULL;


b->names.Append(1,&name,1);


//copy relvant keys
Interval iv(b->start,b->end);
TrackClipObject *cpy = b->controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
b->controls[i]->DeleteKeys(TRACK_DOALL);
iv.Set(0,b->end-b->start);
b->controls[i]->PasteTrack(cpy, iv, TIME_INCLEFT|TIME_INCRIGHT);


if (b->SuperClassID() == CTRL_FLOAT_CLASS_ID)
	{
	float f = 0.0f;
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	}
else if (b->SuperClassID() == CTRL_POSITION_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	}
else if (b->SuperClassID() == CTRL_POINT3_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	}
else if (b->SuperClassID() == CTRL_ROTATION_CLASS_ID)
	{
	Quat f;
	f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);

	}
else if (b->SuperClassID() == CTRL_SCALE_CLASS_ID)
	{
//	Matrix3 f(1);
//		f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);

	Quat f;
	f.Identity();
	Point3 p(1.0f,1.0f,1.0f);
	ScaleValue s(p,f); 
	b->tempControls[i]->SetValue(0,&s);

	}




return 1;
}

void NukeEaseAndMult(Control *c)
{
	int ct = c->NumEaseCurves();
	for (int i=0; i<ct;i++)		
		 c->DeleteEaseCurve(0);
	ct = c->NumMultCurves();
	for (i=0; i<ct;i++)		
		 c->DeleteMultCurve(0);
}


int MasterBlockControl::AppendBlock(BlockControl *b,int i, TrackViewPick res, int where)
{
Control *list;
BOOL createdList = FALSE;
//check for list control if not add
list = BuildListControl(res,createdList);
//check if list has a slave control
Control *slaveControl;
slaveControl = BuildSlave(res,list,createdList);

int bc;
if (where == -1)
	bc = Blocks.Count()-1;
else bc = where;

b->ReplaceReference(i,res.anim->Clone());



b->tempControls[i] = (Control *) res.anim->Clone();
b->backPointers[i] = (SlaveControl *) slaveControl;



AddBlockName(res.anim,res.client,res.subNum,b->names);
//add slaves controls to the selected tracks and put the original as a sub anim of the slaves
//set slave	to have reference to master
slaveControl->ReplaceReference(1,this);
//copy selected track into slave sub
slaveControl->ReplaceReference(0,(Control*)res.anim->Clone());

//copy relvant keys
Interval iv(b->start,b->end);
TrackClipObject *cpy = b->controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
//nuke all keys 
b->controls[i]->DeleteKeys(TRACK_DOALL);
//paste back relevant keys
iv.Set(0,b->end-b->start);
b->controls[i]->PasteTrack(cpy, iv, TIME_INCLEFT|TIME_INCRIGHT);


if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID)
	{
	float f = 0.0f;
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlaveFloatControl *slave = (SlaveFloatControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#ifndef NO_CONTROLLER_SLAVE_POSITION
else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlavePosControl *slave = (SlavePosControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);

//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#endif
else if (res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlavePoint3Control *slave = (SlavePoint3Control *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#ifndef NO_CONTROLLER_SLAVE_ROTATION
else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID)
	{
	Quat f;
	f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlaveRotationControl *slave = (SlaveRotationControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);

	}
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID)
	{
//	Matrix3 f(1);
//		f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);

	Quat f;
	f.Identity();
	Point3 p(1.0f,1.0f,1.0f);
	ScaleValue s(p,f); 
	b->tempControls[i]->SetValue(0,&s);

	SlaveScaleControl *slave = (SlaveScaleControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#endif
if (createdList)
	res.client->AssignController(list,res.subNum);
//res.client->AssignController(slave,res.subNum);

return 1;
}


int MasterBlockControl::AppendBlock(BlockControl *b,int i, TrackViewPick res, Control *bdata,int where)
{
Control *list;
BOOL createdList = FALSE;
//check for list control if not add
list = BuildListControl(res,createdList);
//check if list has a slave control
Control *slaveControl;
slaveControl = BuildSlave(res,list,createdList);

//b->ReplaceReference(i,res.anim->Clone());
//b->tempControls[i] = (Control *) res.anim->Clone();
b->ReplaceReference(i,(Control *) bdata->Clone());

int bc;
if (where == -1)
	bc = Blocks.Count()-1;
else bc = where;





b->tempControls[i] = (Control *) bdata->Clone();

b->backPointers[i] = (SlaveControl *) slaveControl;



AddBlockName(res.anim,res.client,res.subNum,b->names);
//add slaves controls to the selected tracks and put the original as a sub anim of the slaves
//set slave	to have reference to master
slaveControl->ReplaceReference(1,this);
//copy selected track into slave sub
slaveControl->ReplaceReference(0,(Control*)res.anim->Clone());


//copy relvant keys
Interval iv(b->start,b->end);
TrackClipObject *cpy = b->controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
b->controls[i]->DeleteKeys(TRACK_DOALL);
iv.Set(0,b->end-b->start);
b->controls[i]->PasteTrack(cpy, iv, TIME_INCLEFT|TIME_INCRIGHT);


if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID)
	{
	float f = 0.0f;
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlaveFloatControl *slave = (SlaveFloatControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#ifndef NO_CONTROLLER_SLAVE_POSITION
else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlavePosControl *slave = (SlavePosControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#endif
else if (res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID)
	{
	Point3 f(0.0f,0.0f,0.0f);
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlavePoint3Control *slave = (SlavePoint3Control *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#ifndef NO_CONTROLLER_SLAVE_ROTATION
else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID)
	{
	Quat f;
	f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);
	b->tempControls[i]->SetValue(0,&f);
	SlaveRotationControl *slave = (SlaveRotationControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);

	}
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID)
	{
//	Matrix3 f(1);
//		f.Identity();
	b->tempControls[i]->DeleteKeys(TRACK_DOALL);

	Quat f;
	f.Identity();
	Point3 p(1.0f,1.0f,1.0f);
	ScaleValue s(p,f); 

	b->tempControls[i]->SetValue(0,&s);

	SlaveScaleControl *slave = (SlaveScaleControl *) slaveControl;

	slave->scratchControl = (Control *) res.anim->Clone();
	NukeEaseAndMult((Control *) slave->scratchControl);
	slave->scratchControl->DeleteKeys(TRACK_DOALL) ;
	slave->scratchControl->SetValue(0,&f);
//now replace track with slave
//	int bc = Blocks.Count()-1;
	slave->blockID.Append(1,&bc,1);
	slave->subID.Append(1,&i,1);
	}
#endif
if (createdList)
	res.client->AssignController(list,res.subNum);
return 1;
}


MasterBlockControl::AddBlock(HWND hWnd)
{


Tab<TrackViewPick> res;
MasterBlockTrackViewFilter filter;


if (GetCOREInterface()->TrackViewPickMultiDlg(hWnd, &res,&filter,PICKMULTI_FLAG_ANIMATED | PICKMULTI_FLAG_VISTRACKS  ))
	{
//pop up time space and name dialog
//	DebugPrint("Got track conunt %d\n",res.Count());
//	TSTR nameClip;
	TSTR *ptr = new TSTR(GetString(IDS_PW_BLOCK));
	names.Append(1,&ptr,1);

//add new block name
	propNamePos = names.Count()-1;
	if (DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_BLOCKPROP),
			hWnd, MasterBlockPropDlgProc, (LPARAM)this))
		{

		BlockControl *b = new BlockControl;
		Blocks.Append(1,&b,1);
		propStart *= GetTicksPerFrame();
		propEnd *= GetTicksPerFrame();
		b->start = propStart;
		b->end = propEnd;
		b->color = propColor;
		b->l = b->end-b->start+1;

//copy sub controls into blocks
		b->controls.SetCount(res.Count());
		b->tempControls.SetCount(res.Count());
		b->backPointers.SetCount(res.Count());
//	b->controls.SetCount(1);
//	b->tempControls.SetCount(1);
		for (int i = 0; i < res.Count(); i++)
			{
			b->backPointers[i] = NULL;
			b->tempControls[i] = NULL;
			}
		b->suspendNotifies = TRUE;
		for (i = 0; i < res.Count(); i++)
//	for (int i = 0; i < 1; i++)
			{
			b->controls[i] = NULL;
//strip off note tracks
			if (res[i].anim->HasNoteTracks())
				{
				int noteCount = res[i].anim->NumNoteTracks();
				for (int n = noteCount-1; n >= 0; n--)
					{
					NoteTrack *nt = res[i].anim->GetNoteTrack(n);
					res[i].anim->DeleteNoteTrack(nt);
					}
				}
//need to make copy
//need to check is controller really attched or is there no keys and just constant
			if (res[i].anim != NULL)
				{
//check if this controler already has a slave attached don't create another one just update its blockid and subid table
//need check what type of control it is 
				AppendBlock(b,i,res[i]);
				}
			}
		b->suspendNotifies = FALSE;
		b->end = b->end-b->start;
		b->l = b->end-b->start+1;
		b->start = 0;

		ReplaceReference(Blocks.Count()-1+1,b);
		b->NotifySlaves();

		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	else
		{
		names.Delete(names.Count()-1,1);
		}

	
	}
return 0;

}


int MasterBlockControl::AddSelected(HWND hWnd)
{

Tab<TrackViewPick> res;
//MasterBlockTrackViewFilter filter;

res.ZeroCount();
#define ID_TV_GETSELECTED	680

SendMessage(trackHWND,WM_COMMAND,ID_TV_GETSELECTED,(LPARAM)&res);

//nuke invalid types
for (int rc = 0; rc < res.Count(); rc++)
	{
	if (!(res[rc].anim->CanCopyTrack(FOREVER,0)  && res[rc].anim->IsAnimated() && res[rc].anim->CanCopyAnim()))
		{
		res.Delete(rc,1);
		rc--;
		}
	}
//if (GetCOREInterface()->TrackViewPickMultiDlg(hWnd, &res,&filter,3 ))
if (res.Count()>0)
	{
//pop up time space and name dialog
//	DebugPrint("Got track conunt %d\n",res.Count());
//	TSTR nameClip;
	TSTR *ptr = new TSTR(GetString(IDS_PW_BLOCK));
	names.Append(1,&ptr,1);

//add new block name
	propNamePos = names.Count()-1;
	if (DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_BLOCKPROP),
			hWnd, MasterBlockPropDlgProc, (LPARAM)this))
		{

		BlockControl *b = new BlockControl;
		Blocks.Append(1,&b,1);
		propStart *= GetTicksPerFrame();
		propEnd *= GetTicksPerFrame();
		b->start = propStart;
		b->end = propEnd;
		b->color = propColor;
		b->l = b->end-b->start+1;

//copy sub controls into blocks
		b->controls.SetCount(res.Count());
		b->tempControls.SetCount(res.Count());
		b->backPointers.SetCount(res.Count());
//	b->controls.SetCount(1);
//	b->tempControls.SetCount(1);
//DebugPrint("Resized control to  %d \n",b->controls.Count());
		for (int i = 0; i < res.Count(); i++)
			{
			b->backPointers[i] = NULL;
			b->tempControls[i] = NULL;
			}

		b->suspendNotifies = TRUE;
		for (i = 0; i < res.Count(); i++)
//	for (int i = 0; i < 1; i++)
			{
//DebugPrint("Addingf ref %d controls\n",i);
			b->controls[i] = NULL;

//need to make copy
//need to check is controller really attched or is there no keys and just constant
			if (res[i].anim != NULL)
				{

//strip off note tracks
				if (res[i].anim->HasNoteTracks())
					{
					int noteCount = res[i].anim->NumNoteTracks();
					for (int n = noteCount-1; n >= 0; n--)
						{
						NoteTrack *nt = res[i].anim->GetNoteTrack(n);
						res[i].anim->DeleteNoteTrack(nt);
						}
					}

//check if this controler already has a slave attached don't create another one just update its blockid and subid table
//need check what type of control it is 
				AppendBlock(b,i,res[i]);
				}
			}
		b->suspendNotifies = FALSE;
		b->end = b->end-b->start;
		b->l = b->end-b->start+1;
		b->start = 0;

		ReplaceReference(Blocks.Count()-1+1,b);
		b->NotifySlaves();

		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	else
		{
		names.Delete(names.Count()-1,1);
		}

	
	}
return 0;

}


int MasterBlockControl::ReplaceBlock(HWND hWnd, int whichBlock)
{


Tab<TrackViewPick> res;
MasterBlockTrackViewFilter filter;

if (GetCOREInterface()->TrackViewPickMultiDlg(hWnd, &res,&filter,PICKMULTI_FLAG_ANIMATED | PICKMULTI_FLAG_VISTRACKS  ))
	{
//pop up time space and name dialog
//	DebugPrint("Got track conunt %d\n",res.Count());

//add new block name
	propNamePos = whichBlock;

	if (DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_BLOCKPROP),
			hWnd, MasterBlockPropDlgProc, (LPARAM)this))
		{


//loop through back pointers removing all references to this current block
		for (int i = 0;i <Blocks[whichBlock]->backPointers.Count();i++)
			{
//lock at the bockid table
//	int subCount = Blocks[whichBlock]->backPointers[i]->blockID.Count();
			for (int j=0; j< Blocks[whichBlock]->backPointers[i]->blockID.Count(); j++)
				{
				if (Blocks[whichBlock]->backPointers[i]->blockID[j] == whichBlock)
					{
					Blocks[whichBlock]->backPointers[i]->blockID.Delete(j,1);
					Blocks[whichBlock]->backPointers[i]->subID.Delete(j,1);
					j--;
					}
				}
			Blocks[whichBlock]->backPointers[i]->DeleteReference(1);

			}


		for (i = 0;i <Blocks[whichBlock]->externalBackPointers.Count();i++)
			{
//lock at the bockid table
//	int subCount = Blocks[whichBlock]->backPointers[i]->blockID.Count();
			for (int j=0; j< Blocks[whichBlock]->externalBackPointers[i]->blockID.Count(); j++)			{
				if (Blocks[whichBlock]->externalBackPointers[i]->blockID[j] == whichBlock)
					{
					Blocks[whichBlock]->externalBackPointers[i]->blockID.Delete(j,1);
					Blocks[whichBlock]->externalBackPointers[i]->subID.Delete(j,1);
					j--;
					}
				}
			Blocks[whichBlock]->externalBackPointers[i]->DeleteReference(1);

			}


		BlockControl *b = new BlockControl;
//	DeleteReference(whichBlock+1);
		



		propStart *= GetTicksPerFrame();
		propEnd *= GetTicksPerFrame();
		b->start = propStart;
		b->end = propEnd;
		b->color = propColor;
		b->l = b->end-b->start+1;

//copy sub controls into blocks
		b->controls.SetCount(res.Count());
		b->tempControls.SetCount(res.Count());
		b->backPointers.SetCount(res.Count());
//	b->controls.SetCount(1);
//	b->tempControls.SetCount(1);
//DebugPrint("Resized control to  %d \n",b->controls.Count());

		for (i = 0; i < res.Count(); i++)
			{
			b->backPointers[i] = NULL;
			b->tempControls[i] = NULL;
			}
		b->suspendNotifies = TRUE;
		for (i = 0; i < res.Count(); i++)
//	for (int i = 0; i < 1; i++)
			{
//DebugPrint("Addingf ref %d controls\n",i);
			b->controls[i] = NULL;

//need to make copy
//need to check is controller really attched or is there no keys and just constant
			if (res[i].anim != NULL)
				{
//check if this controler already has a slave attached don't create another one just update its blockid and subid table
//need check what type of control it is 
				AppendBlock(b,i,res[i],whichBlock);
				}
			}
		b->end = b->end-b->start;
		b->l = b->end-b->start+1;
		b->start = 0;

		b->suspendNotifies = FALSE;

		ReplaceReference(whichBlock+1,b);
		b->NotifySlaves();

		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

		}
	}
return 0;

}

void MasterBlockControl::AttachAddToList(int where,  TrackViewPick res)
{

if (res.anim)
	{
	if (where < (propTargetList.Count()-1))
		{
		propTargetList[where] = res;
		propNames[where] = GetBlockName(res.anim,res.client, res.subNum);
		}
	else
		{
		int st = propTargetList.Count()-1;
		for (int i = st; i <= where; i++)
			{
			TrackViewPick c;
			c.anim = NULL;
			propTargetList.Append(1,&c,1);
			TSTR *nullName = new TSTR("NULL");
			propNames.Append(1,&nullName,1);
			}
		propTargetList[where] = res;
		int k = res.subNum;
//		DebugPrint("SubName %d %s\n",k,res.client->SubAnimName(k));
		TSTR str;
		TSTR str2;
		res.client->GetClassName(str);
		res.anim->GetClassName(str2);
//		DebugPrint("animClass %s %s\n",str);
		Animatable *r = res.client->SubAnim(k);


		propNames[where] = GetBlockName(res.anim,res.client, res.subNum);
		}

//	AddBlockName(res[i].anim,res[i].client,res[i].subNum,propNames);
//	SendMessage(hListWnd,
//			LB_ADDSTRING,0,(LPARAM)(TCHAR*)*propNames[propNames.Count()-1]);
	}

}

int IsSubString(TSTR sub, TSTR main)
{
int l = sub.Length();
int l2 = main.Length();
for (int i = 0; i < (l2-1+1); i++)
	{
	TSTR match = main.Substr(i,l);
	if (match == sub)
		return 1;
	}
return 0;

}
int MasterBlockControl::RecurseSubs(TSTR matchString, Class_ID pid, Animatable* anim, TrackViewPick& r)
{
if (anim)
	{
	for (int j = 0; j < anim->NumSubs();j++)
		{
//wwatje 4-30-99
		Animatable *sub = anim->SubAnim(j);
		if (sub)
			{
			TSTR subAnim(anim->SubAnimName(j));
//			if ((matchString == subAnim) && (pid == sub->ClassID()))
			if ( IsSubString(matchString, subAnim) && (pid == sub->ClassID()))
				{
				r.client = (ReferenceTarget *)anim;
				if (r.client && r.client->SuperClassID()==PARAMETER_BLOCK_CLASS_ID) 
					{
					IParamBlock *iparam = (IParamBlock*)r.client;
					r.anim = (ReferenceTarget *) iparam->GetController(iparam->AnimNumToParamNum(j));
					r.subNum = j;//;
					if (!r.anim)
						{
						r.anim = (ReferenceTarget*)GetDefaultController(iparam->GetAnimParamControlType(j))->Create();
						iparam->SetController(iparam->AnimNumToParamNum(j), (Control *) r.anim, TRUE);					
	
						}
					return 1;
					}
				else
					{
					r.anim = (ReferenceTarget *) anim->SubAnim(j);
					r.subNum = j;
					return 1;
					}
				}
			}
		

		}
	
	for ( j = 0; j < anim->NumSubs();j++)
		{
		if (RecurseSubs(matchString, pid, anim->SubAnim(j), r)) return 1;
		}
	}
return 0;


}

void MasterBlockControl::MatchNode(Tab<BOOL> selSet, HWND hParent)
{
//popup node selection box
Interface *ip = GetCOREInterface();
MasterMatchNodeViewFilter filter;
TrackViewPick res;
if (ip->TrackViewPickDlg(hParent, &res, NULL))
	{
//	TSTR nodeName = res.client->SubAnimName(res.subNum);	
//	int l = nodeName.Length();
//loop through sel set removing any that don't match the first
	for (int i=0; i < selSet.Count(); i++)
		{
		if (selSet[i])
			{
			//find 
			
			char *name;
			propContainer->pblock2->GetValue(container_names,0,name,FOREVER,i);
			TSTR pName(name);
			int ct = pName.first('/');
			TSTR propSubAnim;
			int l = pName.Length();
			if (ct == -1)
				propSubAnim = pName.Substr(0,l);
			else 
				{
				ct++;
				propSubAnim = pName.Substr(ct,l-ct);
				}
			TrackViewPick r;
			r.anim = NULL;
			r.client = NULL;

			ReferenceTarget *c;
			propContainer->pblock2->GetValue(container_refs,0,c,FOREVER,i);
			Class_ID propID = c->ClassID();

//			for (int j = 0; j < res.anim->NumSubs();j++)
//				{
				if (RecurseSubs(propSubAnim, propID, res.anim,r))
					{
					if (r.anim)
						AttachAddToList(i,  r);
//					j = res.anim->NumSubs();
					}
/*
				TSTR subAnim = res.anim->SubAnimName(j);
				if (propSubAnim == subAnim)
					{
					TrackViewPick r;
					r.client = res.anim;
					r.anim = (ReferenceTarget *) res.anim->SubAnim(j);
					r.subNum = j;
					AttachAddToList(i, r);
					j = res.anim->NumSubs();
					}
*/
//				}
			}
		}
	int ct = 0;
	ct = propContainer->pblock2->Count(container_refs);
	for (i=ct; i < propTargetList.Count(); i++)
		{
		if (propTargetList[i].anim == NULL)
			{
			propTargetList.Delete(i,1);
			propNames.Delete(i,1);
			i--;
			}
		}
	}

}
int MasterBlockControl::AttachAddMoveUp(int i)
{
if (i <= 0) return 0;
TSTR *Name;
TrackViewPick ref;
Name = propNames[i-1];
ref = propTargetList[i-1];
propNames[i-1] = propNames[i];
propTargetList[i-1] = propTargetList[i];
propNames[i] = Name;
propTargetList[i] = ref;
return 1;

}

int MasterBlockControl::AttachAddMoveDown(int i)
{
if (i >= (propTargetList.Count()-1)) return 0;
TSTR *Name;
TrackViewPick ref;
Name = propNames[i+1];
ref = propTargetList[i+1];
propNames[i+1] = propNames[i];
propTargetList[i+1] = propTargetList[i];
propNames[i] = Name;
propTargetList[i] = ref;
return 1;

}

int MasterBlockControl::AttachAdd(HWND hListWnd)

{
Tab<TrackViewPick> res;
HWND hWnd = GetCOREInterface()->GetMAXHWnd();
MasterBlockTrackViewFilterAdd filter;

if (GetCOREInterface()->TrackViewPickMultiDlg(hWnd, &res,&filter, PICKMULTI_FLAG_VISTRACKS  ))
	{
	for (int i = 0; i < res.Count();i++)
		{
		BOOL found = FALSE;
		for (int j = 0; j < propTargetList.Count();j++)
			{
			if (res[i].anim == propTargetList[j].anim)
				{
				found = TRUE;
				j = propTargetList.Count();
				}
			}
//add to list make sure they are not dulicates
		if (!found)
			{
//add names list
//			Control *c = (Control*)res[i].anim; 
			propTargetList.Append(1,&res[i],1);
			AddBlockName(res[i].anim,res[i].client,res[i].subNum,propNames);
			SendMessage(hListWnd,
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)*propNames[propNames.Count()-1]);
			
			}
		}
	}
return 1;
}


int MasterBlockControl::AttachAddNullAt(HWND hListWnd, int where)

{
//add names list
//Control *c = NULL; 
TrackViewPick c;
c.anim = NULL;
propTargetList.Insert(where,1,&c);
TSTR *nullName = new TSTR("NULL");
propNames.Insert(where,1,&nullName);
SendMessage(hListWnd,
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)*propNames[where]);
			
return 1;
}


int MasterBlockControl::BuildNewBlock()

{
//add names list
//TrackViewPick res;
BlockControl *b = new BlockControl;
Blocks.Append(1,&b,1);

TSTR *ptr = new TSTR(propBlockName);
names.Append(1,&ptr,1);

//add new block name
b->start = propStart;
b->end = propEnd;
b->color = propColor;
b->l = b->end-b->start+1;
b->suspendNotifies = TRUE;

//copy sub controls into blocks
int selCount = 0;
for (int i = 0; i < propTargetList.Count(); i++)
	{
//	if (propTargetList[i].anim != NULL)
		selCount++;
	}

int ccount = propContainer->pblock2->Count(container_refs);
//if (selCount> ccount) selCount = ccount;
selCount = ccount;
b->controls.SetCount(selCount);
b->tempControls.SetCount(selCount);
b->backPointers.SetCount(selCount);
int ct = 0;
//for (i = 0; i < propTargetList.Count(); i++)
for (i = 0; i < ccount; i++)
	{
	ReferenceTarget *c;
	propContainer->pblock2->GetValue(container_refs,0,c,FOREVER,i);
//need to make copy
//need to check is controller really attched or is there no keys and just constant
	if ((i<propTargetList.Count()) && (propTargetList[i].anim != NULL)&&
		(propTargetList[i].anim->SuperClassID() == c->SuperClassID())
		)		{
		b->controls[ct] = NULL;
//check if this controler already has a slave attached don't create another one just update its blockid and subid table
//need check what type of control it is 
		AppendBlock(b,ct,propTargetList[i],(Control *)c);
		ct++;
		}
	else 
		{
		b->controls[ct] = NULL;
//check if this controler already has a slave attached don't create another one just update its blockid and subid table
//need check what type of control it is 
		ReferenceTarget *c;
		propContainer->pblock2->GetValue(container_refs,0,c,FOREVER,i);
		char *name;
		propContainer->pblock2->GetValue(container_names,0,name,FOREVER,i);
		TSTR *pName = new TSTR(name);
		AppendBlockNoSlave(b,ct,pName,(Control *)c);
		ct++;
		}

	}
b->end = b->end-b->start;
b->l = b->end-b->start+1;
b->start = 0;

ReplaceReference(Blocks.Count()-1+1,b);
b->suspendNotifies = FALSE;
b->NotifySlaves();
propTargetList.ZeroCount();
NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

return 1;
}

int MasterBlockControl::AttachDeleteAt(HWND hListWnd, int where)

{
//add names list
propTargetList.Delete(where,1);
propNames.Delete(where,1);
SendMessage(hListWnd,
			LB_DELETESTRING,(WPARAM) where,0);
if (propTargetList.Count() !=0)
	{
	if (where >= propTargetList.Count())
		where = propTargetList.Count()-1;
	SendMessage(hListWnd,
			LB_SETCURSEL,where,0);

	}
			
return 1;
}



RefTargetHandle MasterBlockControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
//	MasterBlockControl *cont = new MasterBlockControl;
//	*cont = *this;
	// Clone the strength controller
//	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
//	CloneControl(cont,remap);
	BaseClone(this, NULL, remap);
	return NULL;
	}


void MasterBlockControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
//DebugPrint("Error occured this getValue should never be called on a block control\n");
	}

void MasterBlockControl::GetValue2(
		TimeValue t, void *val, Interval &valid, int whichBlock, int whichSub, GetSetMethod method)
	{

	if (Blocks[whichBlock]->tempControls[whichSub] == NULL)
		{
		UpdateControl(whichBlock);
		}	
	else Blocks[whichBlock]->tempControls[whichSub]->GetValue(t,val,valid, method);
	}


void MasterBlockControl::GetValue3(Control *sub,
		TimeValue t, void *val, Interval &valid, Tab<int> whichBlock, Tab<int> whichSub, Interval localIV, GetSetMethod method)
	{

/*		if ((whichBlock.Count() == 0) || (whichSub.Count() == 0)) return;
	
		for (int j = 0; j < whichBlock.Count(); j++)
			{
			for (int i = 0; i < BlockKeys.Count();i++)
				{
				if (BlockKeys[i].block_id == whichBlock[j])
					Blocks[whichBlock[j]]->AddKeyToSub(sub, whichSub[j], BlockKeys[i].start,  BlockKeys[i].end-BlockKeys[i].start,BlockKeys[i].relative);
				}
			}

//add multiplier curve
		if (sub->CanApplyEaseMultCurves())
			{
			for (int ct = 0; ct < sub->NumMultCurves(); ct++)
				sub->DeleteMultCurve(ct);
			if (sub->NumMultCurves() == 0)
				sub->AppendMultCurve(blendControl);
			}
*/
		if (sub) sub->GetValue(t,val,valid, method);
	}


void MasterBlockControl::Update(Control *sub,
		Tab<int> whichBlock, Tab<int> whichSub)
	{

		if ((whichBlock.Count() == 0) || (whichSub.Count() == 0)) return;
	
		for (int j = 0; j < whichBlock.Count(); j++)
			{
			for (int i = 0; i < BlockKeys.Count();i++)
				{
				if (BlockKeys[i].block_id == whichBlock[j])
					{
					Blocks[whichBlock[j]]->AddKeyToSub(sub, whichSub[j], BlockKeys[i].start,  BlockKeys[i].end-BlockKeys[i].start,range,BlockKeys[i].relative);
					}
//					Blocks[whichBlock[j]]->AddKeyToSub(sub, whichSub[j], BlockKeys[i].start,  BlockKeys[i].end-BlockKeys[i].start,BlockKeys[i].relative);
				}
			}

//add multiplier curve
		if (sub->CanApplyEaseMultCurves())
			{
//			for (int ct = 0; ct < sub->NumMultCurves(); ct++)
//				sub->DeleteMultCurve(ct);
			if (sub->NumMultCurves() == 0)
				sub->AppendMultCurve(blendControl);
			}



//		sub->GetValue(t,val,valid, method);
	}

void MasterBlockControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{

//this should never get called
//DebugPrint("Error occured SetValue should never be called on a block control\n");
/*
	if (method==CTRL_ABSOLUTE) {
		float v = *((float*)val);
		float before = 0.0f, after = 0.0f;
		Interval valid;
		sub->GetValue(t,&before,valid,CTRL_RELATIVE);
		sub->GetValue(t,&after,valid,CTRL_RELATIVE);
		v = -before + v + -after;
		sub->SetValue(t,&v,commit,method);
	} else {
		sub->SetValue(t,val,commit,method);
		}
*/
	}

/*
void MasterBlockControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength;
	GetStrength(t,&strength);

	// This controller is always changing.
	valid.SetInstant(t);
	
//	*((float*)val) = NoiseAtTime(t,seed,0) * strength;
	}

void MasterBlockControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	float val0, val1, val2, res;
	switch (type) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
				}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
				}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
	valid.Set(t,t);
	*((float*)val) = res;
	}
*/


BOOL CALLBACK AddBlockDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MasterBlockControl *mb = (MasterBlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
		mb = (MasterBlockControl*)lParam;

		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//load up list box 
		for (int i=0; i<mb->Blocks.Count(); i++) 
			{
			TSTR name = mb->SubAnimName(i+1);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
			}


		break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			mb->AddDialogSelect = sel;
			EndDialog(hWnd,1);
			break;
			}
		case IDCANCEL:
			mb->AddDialogSelect = -1;
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}




INT_PTR CALLBACK MasterBlockPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MasterBlockControl *mb = (MasterBlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	ISpinnerControl *spin;
	static TSTR zero = FormatUniverseValue(0.0f);
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG:
		{
		mb = (MasterBlockControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		Interval range = GetCOREInterface()->GetAnimRange();
		
		spin = GetISpinner(GetDlgItem(hWnd,IDC_STARTSPIN));
		spin->SetLimits(-999999.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_START), EDITTYPE_INT);
		spin->SetValue(range.Start()/GetTicksPerFrame(),FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_ENDSPIN));
		spin->SetLimits(-999999.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_END), EDITTYPE_INT);
		spin->SetValue(range.End()/GetTicksPerFrame(),FALSE);
		ReleaseISpinner(spin);

		mb->propStart =  range.Start()/GetTicksPerFrame();
		mb->propEnd = range.End()/GetTicksPerFrame();

	//copy name into field
		ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
		iName->SetText(*mb->names[mb->names.Count()-1]);
//		iName->GetText(buf,256);
		ReleaseICustEdit(iName);

		IColorSwatch *cs = GetIColorSwatch(GetDlgItem(hWnd,IDC_COLOR),
				RGB(88,88,188), _T("Block Color"));
		cs->SetColor(RGB(88,88,188));
		mb->propColor = Color(cs->GetColor());
		ReleaseIColorSwatch(cs);


		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		
	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_STARTSPIN: mb->propStart = spin->GetIVal(); break;
		case IDC_ENDSPIN: mb->propEnd = spin->GetIVal(); break;
		}
		break;

	case CC_COLOR_BUTTONDOWN:
		theHold.Begin();
		break;
	case CC_COLOR_BUTTONUP:
		if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
		else theHold.Cancel();
		break;
	case CC_COLOR_CHANGE: {
		if (HIWORD(wParam)) theHold.Begin();
		int i = LOWORD(wParam);
		IColorSwatch *cs = (IColorSwatch*)lParam;
		mb->propColor = Color(cs->GetColor());
		if (HIWORD(wParam)) {
			theHold.Accept(GetString(IDS_DS_PARAMCHG));
			}
		break;
		}


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			TCHAR buf[256];
			ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
			iName->GetText(buf,256);
//			*mb->names[mb->names.Count()-1] = buf;
			*mb->names[mb->propNamePos] = buf;
			ReleaseICustEdit(iName);
			EndDialog(hWnd,1);
			break;
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


INT_PTR CALLBACK KeyPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MasterBlockControl *mb = (MasterBlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	ISpinnerControl *spin;
	static TSTR zero = FormatUniverseValue(0.0f);
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG:
		{
		mb = (MasterBlockControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

		if (mb->propRelative)
			CheckDlgButton(  hWnd,IDC_RELATIVE,TRUE);
		else CheckDlgButton(  hWnd,IDC_RELATIVE,FALSE);
 




		Interval range = GetCOREInterface()->GetAnimRange();

		
		spin = GetISpinner(GetDlgItem(hWnd,IDC_STARTSPIN));
		spin->SetLimits(-999999.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_START), EDITTYPE_INT);
		spin->SetValue(mb->propStart/GetTicksPerFrame(),FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_ENDSPIN));
		spin->SetLimits(-999999.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_END), EDITTYPE_INT);
		spin->SetValue(mb->propEnd/GetTicksPerFrame(),FALSE);
		ReleaseISpinner(spin);

		mb->propStart =  mb->propStart/GetTicksPerFrame();
		mb->propEnd = mb->propEnd/GetTicksPerFrame();

/*
	//copy name into field
		ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
		iName->SetText(*mb->names[mb->names.Count()-1]);
//		iName->GetText(buf,256);
		ReleaseICustEdit(iName);

		IColorSwatch *cs = GetIColorSwatch(GetDlgItem(hWnd,IDC_COLOR),
				RGB(128,128,128), _T("Block Color"));
		cs->SetColor(RGB(128,128,128));
		mb->propColor = cs->GetColor();
		ReleaseIColorSwatch(cs);
*/

		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		
	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_STARTSPIN: mb->propStart = spin->GetIVal(); break;
		case IDC_ENDSPIN: mb->propEnd = spin->GetIVal(); break;
		}
		break;



	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			mb->propRelative = IsDlgButtonChecked(hWnd,IDC_RELATIVE);
			EndDialog(hWnd,1);
			mb->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

			break;
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK AttachBlockDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MasterBlockControl *mb = (MasterBlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
		mb = (MasterBlockControl*)lParam;

		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//load up list box 
		int scount = mb->propContainer->pblock2->Count(container_names);
		for (int i=0; i<mb->propContainer->pblock2->Count(container_refs);i++)
			{
			char *name;
			mb->propContainer->pblock2->GetValue(container_names,0,name,FOREVER,i);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
			}


/*		for (i=0; i<mb->propTargetList.Count();i++)
			{
			char *name;
			name = *mb->propNames[i];
//			mb->propContainer->pblock2->GetValue(container_names,0,name,FOREVER,i);
			SendMessage(GetDlgItem(hWnd,IDC_LIST2),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
			}
*/

		mb->propTargetList.ZeroCount();
		mb->propNames.ZeroCount();
		break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD_CONTROL:
			{
			//bring up muti selector track picker
			mb->AttachAdd(GetDlgItem(hWnd,IDC_LIST2));
			//update list 
			//save off list of controls and names to current
			break;
			}
		case IDC_MATCH_CONTROL:
			{
			Tab<BOOL> selSet;
			selSet.SetCount(mb->propContainer->pblock2->Count(container_names));
			for (int i = 0;i<mb->propContainer->pblock2->Count(container_names);i++)
				{
				selSet[i] = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_GETSEL,i,0);
				}
			mb->MatchNode(selSet, hWnd);

			int ct = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_GETCOUNT,0,0);
			for (i=0; i<ct;i++)
				{
				SendMessage(GetDlgItem(hWnd,IDC_LIST2),
							LB_DELETESTRING,0,0);
				}

			for (i=0; i<mb->propTargetList.Count();i++)
				{
				char *name;
				name = *mb->propNames[i];
				SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
				}

			break;
			}
		case IDC_UP:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_GETCURSEL,0,0);
			if (sel >=0)
				if (mb->AttachAddMoveUp(sel))
					{
					for (int i=0; i<mb->propNames.Count();i++)
						SendMessage(GetDlgItem(hWnd,IDC_LIST2),
							LB_DELETESTRING,0,0);
					for (i=0; i<mb->propNames.Count();i++)
						{
						char *name;
						name = *mb->propNames[i];
						SendMessage(GetDlgItem(hWnd,IDC_LIST2),
							LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
						}
					sel--;
					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_SETCURSEL,sel,0);
					}

			break;
			}
		case IDC_DOWN:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_GETCURSEL,0,0);
			if (sel >=0)
				if (mb->AttachAddMoveDown(sel))
					{
					for (int i=0; i<mb->propNames.Count();i++)
						SendMessage(GetDlgItem(hWnd,IDC_LIST2),
							LB_DELETESTRING,0,0);
					for (i=0; i<mb->propNames.Count();i++)
						{
						char *name;
						name = *mb->propNames[i];
						SendMessage(GetDlgItem(hWnd,IDC_LIST2),
							LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
						}
					sel++;
					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_SETCURSEL,sel,0);
					}

			break;
			}
		case IDC_ADDNULL_CONTROL:
			{
			//bring up muti selector track picker
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_GETCURSEL,0,0);
			if (sel < 0)
			sel = 0;

			mb->AttachAddNullAt(GetDlgItem(hWnd,IDC_LIST2),sel);
			//update list 
			//save off list of controls and names to current
			break;
			}

		case IDC_DELETE_CONTROL:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
					LB_GETCURSEL,0,0);
			if (sel >=0)
				mb->AttachDeleteAt(GetDlgItem(hWnd,IDC_LIST2),sel);
			//get selecetd list
			//delete selected tracks
			break;
			}
		case IDOK:
			{
			mb->BuildNewBlock();
			EndDialog(hWnd,1);
			break;
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

#define CUBICMORPHCONT_CLASS_ID 0x09923023


BOOL MasterBlockTrackViewFilter :: proc(Animatable *anim, Animatable *client,int subNum)

{
//make sure the parent is not a slave or 
if ( anim->CanCopyTrack(FOREVER,0) /*&& ((Control*)anim)->IsKeyable()*/ && anim->IsAnimated() && anim->CanCopyAnim()
	 && (client->ClassID() !=BLOCK_CONTROL_CLASS_ID)
	&& (anim->ClassID() != Class_ID(CUBICMORPHCONT_CLASS_ID,0))
	&& (anim->ClassID() !=Class_ID(MASTERPOINTCONT_CLASS_ID,0))
	&& (anim->SuperClassID()!=NOTETRACK_CLASS_ID)

//	|| ( (anim->ClassID() == SLAVEFLOAT_CONTROL_CLASS_ID) || 
//		 (anim->ClassID() == SLAVEPOS_CONTROL_CLASS_ID) ||
//		 (anim->ClassID() == SLAVEROTATION_CONTROL_CLASS_ID) 
//	   )
	)
	return TRUE;
return FALSE;
}

BOOL MasterBlockTrackViewFilterAdd :: proc(Animatable *anim, Animatable *client,int subNum)

{
//make sure the parent is not a slave or 
if ( anim->CanCopyTrack(FOREVER,0) && anim->CanCopyAnim() &&
	 (client->ClassID() !=BLOCK_CONTROL_CLASS_ID)
	&& (anim->ClassID() != Class_ID(CUBICMORPHCONT_CLASS_ID,0))
	&& (anim->ClassID() !=Class_ID(MASTERPOINTCONT_CLASS_ID,0))
	&& (anim->SuperClassID()!=NOTETRACK_CLASS_ID)
	 
//	|| ( (anim->ClassID() == SLAVEFLOAT_CONTROL_CLASS_ID) || 
//		 (anim->ClassID() == SLAVEPOS_CONTROL_CLASS_ID) ||
//		 (anim->ClassID() == SLAVEROTATION_CONTROL_CLASS_ID) 
//	   )
	)
	return TRUE;
return FALSE;
}


BOOL MasterMatchNodeViewFilter :: proc(Animatable *anim, Animatable *client,int subNum)

{
//make sure the parent is not a slave or 
if ( ( anim->SuperClassID()==BASENODE_CLASS_ID) &&
	 (client->ClassID() !=BLOCK_CONTROL_CLASS_ID)
	&& (anim->ClassID() != Class_ID(CUBICMORPHCONT_CLASS_ID,0))
	&& (anim->ClassID() !=Class_ID(MASTERPOINTCONT_CLASS_ID,0))
	&& (anim->SuperClassID()!=NOTETRACK_CLASS_ID)

//	|| ( (anim->ClassID() == SLAVEFLOAT_CONTROL_CLASS_ID) || 
//		 (anim->ClassID() == SLAVEPOS_CONTROL_CLASS_ID) ||
//		 (anim->ClassID() == SLAVEROTATION_CONTROL_CLASS_ID) 
//	   )
	)
	return TRUE;
return FALSE;
}
