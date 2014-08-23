/**********************************************************************
 *<
	FILE: refobj.cpp

	DESCRIPTION: A utility that inserts a derived object 

	CREATED BY: Rolf Berteig

	HISTORY: created 1/27/97

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"

#define REFOBJ_CLASS_ID		Class_ID(0xbb300184,0xee6d2a10)
#define REFOBJ_CNAME		GetString(IDS_RB_REFOBJ)

class RefObjUtil : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;		
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);		

		void DoRefObj();
	};
static RefObjUtil theRefObjUtil;

class RefObjUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theRefObjUtil;}
	const TCHAR *	ClassName() {return REFOBJ_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return REFOBJ_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static RefObjUtilClassDesc refObjUtilDesc;
ClassDesc* GetRefObjUtilDesc() {return &refObjUtilDesc;}


static INT_PTR CALLBACK RefObjUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theRefObjUtil.Init(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theRefObjUtil.iu->CloseUtility();
					break;

				case IDC_REF_OBJECT:
					theRefObjUtil.DoRefObj();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theRefObjUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	

void RefObjUtil::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_REFERENCE_PANEL),
		RefObjUtilDlgProc,
		GetString(IDS_RB_REFOBJ),
		0);
	}
	
void RefObjUtil::EndEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void RefObjUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;
	SelectionSetChanged(ip,iu);	
	}

void RefObjUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{	
	if (ip->GetSelNodeCount()==1) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,ip->GetSelNode(0)->GetName());
	} else if (ip->GetSelNodeCount()) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_MULTISEL));
	} else {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_NONESEL));
		}
	if (ip->GetSelNodeCount()) {
		EnableWindow(GetDlgItem(hPanel,IDC_REF_OBJECT),TRUE);
	} else {
		EnableWindow(GetDlgItem(hPanel,IDC_REF_OBJECT),FALSE);
		}
	}

void RefObjUtil::DoRefObj()
	{
	theHold.Begin();
	INodeTab flash;
	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		INode *node = ip->GetSelNode(i);
		flash.Append(1,&node,10);
		node->SetObjectRef(
			MakeObjectDerivedObject(node->GetObjectRef()));
		}
	theHold.Accept(GetString(IDS_RB_REFOBJ));
	// Flash nodes
	ip->FlashNodes(&flash);	
	ip->RedrawViews(ip->GetTime());
	}
