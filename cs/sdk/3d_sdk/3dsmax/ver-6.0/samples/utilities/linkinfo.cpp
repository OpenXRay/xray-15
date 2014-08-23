/**********************************************************************
 *<
	FILE: linkinfo.cpp

	DESCRIPTION:  A utility for editing link info across objects

	CREATED BY: Rolf Berteig

	HISTORY: created 02/14/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"

#ifndef NO_UTILITY_LINKINFO	// russom - 10/16/01

#define LINKINFO_CLASS_ID		Class_ID(0x0017d5e1,0x7730aa61)
#define LINKINFO_CNAME			GetString(IDS_RB_LINKINFO)

class LinkInfoUtil : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;		
		HWND hWnd;

		LinkInfoUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);
		
		void SetStates(HWND hWnd);
		void SetBit(int bit, BOOL onOff);
	};
static LinkInfoUtil theLinkInfoUtil;

class LinkInfoUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theLinkInfoUtil;}
	const TCHAR *	ClassName() {return LINKINFO_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return LINKINFO_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static LinkInfoUtilClassDesc linkInfoUtilDesc;
ClassDesc* GetLinkInfoUtilDesc() {return &linkInfoUtilDesc;}

//-----------------------------------------------------------------------

static int ctrlIDs[] = {
	IDC_INHERIT_XTRANS,IDC_INHERIT_YTRANS,IDC_INHERIT_ZTRANS,
	IDC_INHERIT_XROT,IDC_INHERIT_YROT,IDC_INHERIT_ZROT,
	IDC_INHERIT_XSCALE,IDC_INHERIT_YSCALE,IDC_INHERIT_ZSCALE
	};

static int GetCtrlBit(int id)
	{
	for (int i=0; i<9; i++) {
		if (ctrlIDs[i]==id) return i;
		}
	return -1;
	}

static INT_PTR CALLBACK LinkInfoUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theLinkInfoUtil.SetStates(hWnd);
			break;

		case WM_DESTROY:			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_INHERIT_XTRANS:
				case IDC_INHERIT_YTRANS:
				case IDC_INHERIT_ZTRANS:
				case IDC_INHERIT_XROT:
				case IDC_INHERIT_YROT:
				case IDC_INHERIT_ZROT:
				case IDC_INHERIT_XSCALE:
				case IDC_INHERIT_YSCALE:
				case IDC_INHERIT_ZSCALE: {
					int bit = GetCtrlBit(LOWORD(wParam));
					assert(bit>=0);
					theLinkInfoUtil.SetBit(bit,
						IsDlgButtonChecked(hWnd,LOWORD(wParam)));
					break;
					}

				case IDOK:
					theLinkInfoUtil.iu->CloseUtility();
					break;							
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theLinkInfoUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	

//-----------------------------------------------------------------------

LinkInfoUtil::LinkInfoUtil()
	{
	iu   = NULL;
	ip   = NULL;
	hWnd = NULL;
	}

void LinkInfoUtil::BeginEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = iu;
	this->ip = ip;
	hWnd = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PRSLINKINFO),
		LinkInfoUtilDlgProc,
		GetString(IDS_RB_LINKINFO),
		0);
	}

void LinkInfoUtil::EndEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hWnd);	
	}

void LinkInfoUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	SetStates(hWnd);
	}

void LinkInfoUtil::SetStates(HWND hWnd)
	{
	DWORD cf=0, valid=0xffffffff, init=0;

	if (!ip->GetSelNodeCount()) {
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XTRANS),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YTRANS),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZTRANS),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XROT),  FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YROT),  FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZROT),  FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XSCALE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YSCALE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZSCALE),FALSE);
		return;
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XTRANS),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YTRANS),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZTRANS),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XROT),  TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YROT),  TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZROT),  TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_XSCALE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_YSCALE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_INHERIT_ZSCALE),TRUE);
		}

	for (DWORD i=0; i<(DWORD)ip->GetSelNodeCount(); i++) {
		DWORD flags = ip->GetSelNode(i)->GetTMController()->
			GetInheritanceFlags();
		for (DWORD j=0; j<9; j++) {
			if (init&(1<<j)) {
				if ((cf&(1<<j)) != (flags&(1<<j))) {
					valid &= ~(1<<j);
					}
			} else {
				cf   |= flags & (1<<j);
				init |= (1<<j);
				}
			}
		}

	for (i=0; i<9; i++) {
		if (valid & (1<<i)) {
			MakeButton2State(GetDlgItem(hWnd,ctrlIDs[i]));
			CheckDlgButton(hWnd,ctrlIDs[i],cf&(1<<i)?FALSE:TRUE);
		} else {
			MakeButton3State(GetDlgItem(hWnd,ctrlIDs[i]));
			CheckDlgButton(hWnd,ctrlIDs[i],BST_INDETERMINATE);
			}		 
		}
	}

void LinkInfoUtil::SetBit(int bit, BOOL onOff)
	{
	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		DWORD flags = ~ip->GetSelNode(i)->GetTMController()->
			GetInheritanceFlags();

		if (onOff) 
			 flags |= (1<<bit);
		else flags &= ~(1<<bit);

		ip->GetSelNode(i)->GetTMController()->
			SetInheritanceFlags(flags,TRUE);
		ip->GetSelNode(i)->GetTMController()->
			NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		}
	SetStates(hWnd);
	ip->RedrawViews(ip->GetTime());
	}

#endif // NO_UTILITY_LINKINFO	
