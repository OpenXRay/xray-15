/**********************************************************************
 *<
	FILE: setkeyUtil.cpp

	DESCRIPTION: set-key mode test utility

	CREATED BY: Rolf Berteig

	HISTORY: created 11/03/2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "istdplug.h"
#include "modstack.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "SetKeyMode.h"
#include "ICustAttribContainer.h"
#include "CustAttrib.h"

#define SETKEY_UTIL_CLASS_ID	0x1f937a48


class SetKeyUtil : public UtilityObj, public GlobalReferenceMaker, public SetKeyModeCallback {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;		
		BOOL modListValid;
		BOOL createPosKey, createRotKey, createScaleKey, entireHierarchy;
		int showModTracks;
		ICustButton *iSetKeyMode;

		SetKeyUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
				
		void SetSetKeyMode(BOOL onOff);
		void SetKeyCommit();
		void SetKeyRestore();
		BOOL CreateTransformKeysIfBuffersNotPresent(TimeValue t,INode *node, BOOL doHierarchy);

		void SelectionSetChanged(Interface *ip,IUtil *iu);		

		void UpdateModTrackList();
		void InvalidateModTrackList();
		void AlterModListSel(WORD cmd);
		void SetupButtonStates();

		RefResult NotifyRefChanged(
			Interval iv, RefTargetHandle hTarg,
			PartID& partID, RefMessage msg);

		// From SetKeyModeCallback
		void SetKey();
		void ShowUI();
		void SetKeyModeStateChanged();
	};
static SetKeyUtil theSetKeyUtil;

class SetKeyUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theSetKeyUtil;}
	const TCHAR *	ClassName() {return _T("Set Key");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(SETKEY_UTIL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static SetKeyUtilClassDesc setKeyUtilDesc;
ClassDesc* GetSetKeyUtilDesc() {return &setKeyUtilDesc;}


class UncommittedTrackRec {
	public:
		TSTR *name;
		Animatable *anim, *sanim;
		int subNum;
		UncommittedTrackRec(TSTR *n, Animatable *a, Animatable *s, int sn) {name = n; anim=a; sanim=s; subNum=sn;}
		~UncommittedTrackRec() {delete name;}
	};


static INT_PTR CALLBACK SetKeyUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theSetKeyUtil.Init(hWnd);			
			break;
		
		case WM_DESTROY:
			theSetKeyUtil.Destroy(hWnd);
			break;

		case WM_PAINT:
			theSetKeyUtil.UpdateModTrackList();
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theSetKeyUtil.iu->CloseUtility();
					break;

				case IDC_SETKEY_MODE:
					theSetKeyUtil.SetSetKeyMode(theSetKeyUtil.iSetKeyMode->IsChecked());					
					break;

				case IDC_SETKEY_COMMIT:
					theSetKeyUtil.SetKeyCommit();
					break;

				case IDC_SETKEY_RESTORE:
					theSetKeyUtil.SetKeyRestore();
					break;				

				case IDC_SELECT_ALL:
				case IDC_SELECT_NONE:
				case IDC_SELECT_INVERT:
					theSetKeyUtil.AlterModListSel(LOWORD(wParam));
					break;

				case IDC_ALWYAS_CREATE_POSKEY:
					theSetKeyUtil.createPosKey = IsDlgButtonChecked(hWnd, LOWORD(wParam));
					break;
				case IDC_ALWYAS_CREATE_ROTKEY:
					theSetKeyUtil.createRotKey = IsDlgButtonChecked(hWnd, LOWORD(wParam));
					break;
				case IDC_ALWYAS_CREATE_SCLKEY:
					theSetKeyUtil.createScaleKey = IsDlgButtonChecked(hWnd, LOWORD(wParam));
					break;

				case IDC_SHOWTRACKS_ALL:
				case IDC_SHOWTRACKS_SEL_OBJ:
				case IDC_SHOWTRACKS_SEL_HIERARCHY:
					theSetKeyUtil.showModTracks = LOWORD(wParam);
					theSetKeyUtil.SetupButtonStates();
					theSetKeyUtil.InvalidateModTrackList();
					break;

				case IDC_ENTIRE_HIERARCHY:
					theSetKeyUtil.entireHierarchy = IsDlgButtonChecked(hWnd, LOWORD(wParam));
					theSetKeyUtil.InvalidateModTrackList();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theSetKeyUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE; 
	}


SetKeyUtil::SetKeyUtil()
	{
	modListValid    = FALSE;
	createPosKey    = FALSE;
	createRotKey    = FALSE;
	createScaleKey  = FALSE;
	entireHierarchy = FALSE;
	showModTracks   = IDC_SHOWTRACKS_ALL;
	iSetKeyMode     = NULL;
	
	SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(GetCOREInterface());
	if (iSetKey) {
		iSetKey->RegisterSetKeyModeCallback(this);
		}
	}

void SetKeyUtil::BeginEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SETKEY_PANEL),
		SetKeyUtilDlgProc,
		_T("Set Key Utility"),
		0);
	RegisterGlobalReference(this);
	InvalidateModTrackList();
	}

void SetKeyUtil::EndEditParams(Interface *ip,IUtil *iu)
	{	
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	UnRegisterGlobalReference(this);
	}

void SetKeyUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;
	iSetKeyMode = GetICustButton(GetDlgItem(hPanel, IDC_SETKEY_MODE));
	iSetKeyMode->SetType(CBT_CHECK);
	iSetKeyMode->SetHighlightColor(RGB(59,133,63));
	SetupButtonStates();
	UpdateModTrackList();
	}

void SetKeyUtil::Destroy(HWND hWnd)
	{
	ReleaseICustButton(iSetKeyMode);
	iSetKeyMode = NULL;
	}


void SetKeyUtil::SetSetKeyMode(BOOL onOff)
	{
	//ip->ActivateSetKeyMode(onOff);
	SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(ip);
	if (iSetKey) {
		iSetKey->ActivateSetKeyMode(onOff);
		ip->RedrawViews(ip->GetTime());
		}
	}


BOOL SetKeyUtil::CreateTransformKeysIfBuffersNotPresent(TimeValue t,INode *node, BOOL doHierarchy)
	{
	Control *tmControl = node->GetTMController();
	Control *cont;
	BOOL res = FALSE;

	if (tmControl) {
		if (createPosKey && (cont=tmControl->GetPositionController())) {
			if (!cont->SetKeyBufferPresent()) {
				//cont->AddNewKey(t,ADDKEY_INTERP);
				cont->CreateLockKey(t,0);
				res = TRUE;
				}
			}
		if (createRotKey && (cont=tmControl->GetRotationController())) {
			if (!cont->SetKeyBufferPresent()) {
				//cont->AddNewKey(t,ADDKEY_INTERP);
				cont->CreateLockKey(t,1);
				res = TRUE;
				}
			}
		if (createScaleKey && (cont=tmControl->GetScaleController())) {
			if (!cont->SetKeyBufferPresent()) {
				//cont->AddNewKey(t,ADDKEY_INTERP);
				cont->CreateLockKey(t,2);
				res = TRUE;
				}
			}
		}
	if (doHierarchy) {
		for (int i=0; i<node->NumberOfChildren(); i++) {
			if (CreateTransformKeysIfBuffersNotPresent(t,node->GetChildNode(i), doHierarchy)) {
				res = TRUE;
				}
			}
		}
	return res;
	}

void SetKeyUtil::SetKeyCommit()
	{
	TimeValue t = ip->GetTime();

	int ct = SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETCOUNT, 0, 0);
	if (ct) theHold.Begin();
	BOOL anyDone = FALSE;

	// First, create keys without buffers	
	if ((showModTracks==IDC_SHOWTRACKS_SEL_OBJ || showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY) &&
		(createPosKey || createRotKey || createScaleKey)) {
		
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode *node = ip->GetSelNode(i);
			if (showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY && entireHierarchy) {
				// Find root of this subtree					
				while (INode *parNode = node->GetParentNode()) {						
					if (parNode->IsRootNode()) break;
					node = parNode;
					}
				}
			if (CreateTransformKeysIfBuffersNotPresent(
				t, node, (showModTracks==IDC_SHOWTRACKS_SEL_OBJ || showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY))) {
				anyDone = TRUE;
				}
			}
		}

	// Then commit other tracks
	for (int i=ct-1; i>=0; i--) {
		if (SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETSEL, i, 0)>0) {
			UncommittedTrackRec *rec = 
				(UncommittedTrackRec*)SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETITEMDATA, i, 0);
			
			// Commit the buffer
			rec->anim->SubAnimCommitSetKeyBuffer(t, rec->subNum);
			anyDone = TRUE;
			}		
		}
	if (ct) {
		if (anyDone) theHold.Accept(GetString(IDS_RB_SETKEY_COMMIT_UNDO));
		else theHold.Cancel();
		}
	
	ip->RedrawViews(ip->GetTime());
	}

void SetKeyUtil::SetKeyRestore()
	{
	int ct = SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETCOUNT, 0, 0);
	if (ct) theHold.Begin();
	BOOL anyDone = FALSE;
	for (int i=ct-1; i>=0; i--) {
		if (SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETSEL, i, 0)>0) {
			UncommittedTrackRec *rec = 
				(UncommittedTrackRec*)SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETITEMDATA, i, 0);
			
			rec->anim->SubAnimRevertSetKeyBuffer(rec->subNum);
			anyDone = TRUE;
			}		
		}
	if (ct) {
		if (anyDone) theHold.Accept(GetString(IDS_RB_SETKEY_REVERT_UNDO));
		else theHold.Cancel();
		}
	ip->RedrawViews(ip->GetTime());
	}

void SetKeyUtil::SetupButtonStates()
	{	
	iSetKeyMode->SetCheck(GetSetKeyMode());
	
	CheckDlgButton(hPanel, IDC_SHOWTRACKS_ALL, showModTracks==IDC_SHOWTRACKS_ALL);
	CheckDlgButton(hPanel, IDC_SHOWTRACKS_SEL_OBJ, showModTracks==IDC_SHOWTRACKS_SEL_OBJ);
	CheckDlgButton(hPanel, IDC_SHOWTRACKS_SEL_HIERARCHY, showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY);	
	CheckDlgButton(hPanel, IDC_ENTIRE_HIERARCHY, entireHierarchy);	
	EnableWindow(GetDlgItem(hPanel, IDC_ENTIRE_HIERARCHY), showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY);

	if (showModTracks==IDC_SHOWTRACKS_SEL_OBJ || showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY) {
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_POSKEY), TRUE);
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_ROTKEY), TRUE);
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_SCLKEY), TRUE);
		
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_POSKEY, createPosKey);
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_ROTKEY, createRotKey);
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_SCLKEY, createScaleKey);
	} else {
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_POSKEY), FALSE);
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_ROTKEY), FALSE);
		EnableWindow(GetDlgItem(hPanel, IDC_ALWYAS_CREATE_SCLKEY), FALSE);
		
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_POSKEY, FALSE);
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_ROTKEY, FALSE);
		CheckDlgButton(hPanel, IDC_ALWYAS_CREATE_SCLKEY, FALSE);
		}	
	}

void SetKeyUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	if (showModTracks==IDC_SHOWTRACKS_SEL_OBJ || showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY) {	
		InvalidateModTrackList();
		}
	}

static void RecursiveClearAFlag(Animatable *anim, DWORD flag)
	{
	if (!anim) return;
	int ns = anim->NumSubs();
	for (int i=0; i<ns; i++) {
		Animatable *sanim = anim->SubAnim(i);
		if (sanim) {
			sanim->ClearAFlag(flag);
			RecursiveClearAFlag(sanim, flag);
			}
		}

	if (anim->GetCustAttribContainer() && anim->GetCustAttribContainer()->NumSubs()) {
		ICustAttribContainer *pContainer = anim->GetCustAttribContainer();
		for (i = 0; i<pContainer->NumSubs(); i++) {
			Animatable *sanim = pContainer->SubAnim(i);
			if (sanim) {
				sanim->ClearAFlag(flag);
				RecursiveClearAFlag(sanim, flag);
				}
			}
		}

	if (anim->SuperClassID()==BASENODE_CLASS_ID) {
		INode *node = (INode*)anim;		
		node->ClearAFlag(flag);
		for (int i=0; i<node->NumberOfChildren(); i++) {
			RecursiveClearAFlag(node->GetChildNode(i), flag);			
			}
		}
	}


static void RecursiveFindUncommittedTracks(
		Tab<UncommittedTrackRec*> &ucTracksTab, Animatable *anim, TSTR *nodeName, BOOL doHierarchy)
	{
	if (!anim) return;
	int ns = anim->NumSubs();
	for (int i=0; i<ns; i++) {
		
		if (anim->SubAnimSetKeyBufferPresent(i)) {
			Animatable *sanim = anim->SubAnim(i);
			if (sanim && !sanim->TestAFlag(A_WORK1)) {
				TSTR *sn = new TSTR(anim->SubAnimName(i));
				if (nodeName) {
					*sn = *nodeName + TSTR(_T("\\")) + *sn;
					}
				
				UncommittedTrackRec *rec = new UncommittedTrackRec(sn, anim, sanim, i);
				ucTracksTab.Append(1, &rec);				
				sanim->SetAFlag(A_WORK1);
				}
			}
		
		RecursiveFindUncommittedTracks(ucTracksTab, anim->SubAnim(i), nodeName, doHierarchy);
		}

	if (anim->GetCustAttribContainer() && anim->GetCustAttribContainer()->NumSubs()) {
		ICustAttribContainer *pContainer = anim->GetCustAttribContainer();
		for (i = 0; i<pContainer->NumSubs(); i++) {
			Animatable *sanim = pContainer->SubAnim(i);
			if (sanim && !sanim->TestAFlag(A_WORK1)) {
				if (sanim->SetKeyBufferPresent()) {
					TCHAR *nameCA = pContainer->GetCustAttrib(i)->GetName();
					TSTR *sn = new TSTR(nameCA);
					if (nodeName) {
						*sn = *nodeName + TSTR(_T("\\")) + *sn;
						}
					
					UncommittedTrackRec *rec = new UncommittedTrackRec(sn, pContainer, sanim, i);
					ucTracksTab.Append(1, &rec);				
					sanim->SetAFlag(A_WORK1);
				} else {
					sanim->SetAFlag(A_WORK1);
					RecursiveFindUncommittedTracks(ucTracksTab, sanim, nodeName, doHierarchy);
					}
				}
			}
		}

	if (anim->SuperClassID()==BASENODE_CLASS_ID && doHierarchy) {
		INode *node = (INode*)anim;		

		for (int i=0; i<node->NumberOfChildren(); i++) {
			TSTR nm = node->GetChildNode(i)->GetName();
			RecursiveFindUncommittedTracks(ucTracksTab, node->GetChildNode(i), &nm, doHierarchy);
			}
		}
	}

void SetKeyUtil::InvalidateModTrackList()
	{
	if (modListValid) {
		modListValid = FALSE;
		InvalidateRect(hPanel, NULL, FALSE);
		}
	}

void SetKeyUtil::UpdateModTrackList()
	{
	if (!modListValid) {
		modListValid = TRUE;
		
		// Use a flag so we don't put instanced items in the list multiple times
		RecursiveClearAFlag(ip->GetScenePointer(), A_WORK1);
		
		// Build a table of uncommitted tracks
		Tab<UncommittedTrackRec*> ucTracksTab;
		if (showModTracks==IDC_SHOWTRACKS_SEL_OBJ ||
			showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY) {

			for (int i=0; i<ip->GetSelNodeCount(); i++) {
				INode *node = ip->GetSelNode(i);
				if (showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY && entireHierarchy) {
					// Find root of this subtree					
					while (INode *parNode = node->GetParentNode()) {						
						if (parNode->IsRootNode()) break;
						node = parNode;
						}
					}
				TSTR nm = node->GetName();
				RecursiveFindUncommittedTracks(ucTracksTab, node, &nm, showModTracks==IDC_SHOWTRACKS_SEL_HIERARCHY);
				}

		} else {			
			RecursiveFindUncommittedTracks(ucTracksTab, ip->GetRootNode(), NULL, TRUE);
			RecursiveFindUncommittedTracks(ucTracksTab, ip->GetScenePointer(), NULL, TRUE);
			}

		// Delete the UncommittedTrackRec instances attached to each item in the list
		int ct = SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETCOUNT, 0, 0);
		for (int i=0; i<ct; i++) {
			UncommittedTrackRec *rec = 
				(UncommittedTrackRec*)SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETITEMDATA, i, 0);
			delete rec;
			}

		// Clear the list box and rebuild it
		SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_RESETCONTENT, 0, 0);
		for ( i=0; i<ucTracksTab.Count(); i++) {
			int ix = SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_ADDSTRING, 0, 
				(LPARAM)(TCHAR*)(  *((ucTracksTab[i])->name)  )  );
			SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_SETSEL, TRUE, ix);
			SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_SETITEMDATA, ix, (LPARAM)ucTracksTab[i]);
			}		
		}
	}

void SetKeyUtil::AlterModListSel(WORD cmd)
	{
	int ct = SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETCOUNT, 0, 0);
	for (int i=0; i<ct; i++) {
		BOOL sel = FALSE;
		switch (cmd) {
			case IDC_SELECT_ALL:  sel = TRUE;  break;
			case IDC_SELECT_NONE: sel = FALSE; break;
			case IDC_SELECT_INVERT:
				sel = !(0<SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_GETSEL, i, 0));
				break;
			}
		SendDlgItemMessage(hPanel, IDC_MOD_TRACK_LIST, LB_SETSEL, sel, i);
		}
	}

RefResult SetKeyUtil::NotifyRefChanged(
		Interval iv, RefTargetHandle hTarg, PartID& partID, RefMessage msg)
	{
	if (modListValid) {
		switch (msg) {
			case REFMSG_CHANGE:
			case REFMSG_NODE_NAMECHANGE:
				InvalidateModTrackList();
				break;
			}
		}
	return REF_SUCCEED;
	}

void SetKeyUtil::SetKey()
	{
	if (hPanel) {
		// Our UI is up
		SetKeyCommit();
	} else {
		// Our UI is not up. Commit all
		SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(GetCOREInterface());
		if (iSetKey) {
			theHold.Begin();
			iSetKey->AllTracksCommitSetKeyBuffer();
			theHold.Accept(GetString(IDS_RB_SETKEY_COMMIT_UNDO));
			}
		}
	}

void SetKeyUtil::ShowUI()
	{

	}

void SetKeyUtil::SetKeyModeStateChanged()
	{
	if (iSetKeyMode) {
		iSetKeyMode->SetCheck(GetSetKeyMode());
		}
	}

