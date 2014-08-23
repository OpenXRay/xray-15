/**********************************************************************
 *<
	FILE: resettm.cpp

	DESCRIPTION: A reset xform utility

	CREATED BY: Rolf Berteig

	HISTORY: created June 28, 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "utilapi.h"
#include "istdplug.h"
#include "modstack.h"
#include "simpmod.h"

#ifndef NO_UTILITY_RESETXFORM	// russom - 12/04/01

class ResetXForm : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		
		ResetXForm();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void SelectionSetChanged(Interface *ip,IUtil *iu);
		void DeleteThis() {}		
		void ResetSel();

	};
static ResetXForm theResetXForm;

class ResetXFormClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theResetXForm;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_RESETXFORM_CLASS);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(RESET_XFORM_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static ResetXFormClassDesc resetXFormDesc;
ClassDesc* GetResetXFormDesc() {return &resetXFormDesc;}


static INT_PTR CALLBACK ResetXFormDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theResetXForm.hPanel = hWnd;
			theResetXForm.
				SelectionSetChanged(theResetXForm.ip,theResetXForm.iu);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theResetXForm.iu->CloseUtility();
					break;

				case IDC_RESETTM_SELECTED:
					theResetXForm.ResetSel();
					break;
				}
			break;
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theResetXForm.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	


ResetXForm::ResetXForm()
	{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;	
	}

void ResetXForm::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_RESETXFORM_PANEL),
		ResetXFormDlgProc,
		GetString(IDS_RB_RESETXFORM),
		0);
	}
	
void ResetXForm::EndEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void ResetXForm::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	if (ip->GetSelNodeCount()) {
		BOOL res = FALSE;
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode *node = ip->GetSelNode(i);
			if (!node->IsGroupMember() && !node->IsGroupHead()) {
				res = TRUE;
				break;
				}
			}
		EnableWindow(GetDlgItem(hPanel,IDC_RESETTM_SELECTED),res);
	} else {
		EnableWindow(GetDlgItem(hPanel,IDC_RESETTM_SELECTED),FALSE);
		}
	}

static BOOL SelectedAncestor(INode *node)
	{
	if (!node->GetParentNode()) return FALSE;
	if (node->GetParentNode()->Selected()) return TRUE;
	else return SelectedAncestor(node->GetParentNode());
	}

void ResetXForm::ResetSel()
	{
	//theHold.Begin();

	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		INode *node = ip->GetSelNode(i);
		if (node->IsGroupMember() || node->IsGroupHead()) continue;
		if (SelectedAncestor(node)) continue;

		Matrix3 ntm, ptm, rtm(1), piv(1), tm;
		
		// Get Parent and Node TMs
		ntm = node->GetNodeTM(ip->GetTime());
		ptm = node->GetParentTM(ip->GetTime());
		
		// Compute the relative TM
		ntm = ntm * Inverse(ptm);
		
		// The reset TM only inherits position
		rtm.SetTrans(ntm.GetTrans());
		
		// Set the node TM to the reset TM		
		tm = rtm*ptm;
		node->SetNodeTM(ip->GetTime(), tm);

		// Compute the pivot TM
		piv.SetTrans(node->GetObjOffsetPos());
		PreRotateMatrix(piv,node->GetObjOffsetRot());
		ApplyScaling(piv,node->GetObjOffsetScale());
		
		// Reset the offset to 0
		node->SetObjOffsetPos(Point3(0,0,0));
		node->SetObjOffsetRot(IdentQuat());
		node->SetObjOffsetScale(ScaleValue(Point3(1,1,1)));

		// Take the position out of the matrix since we don't reset position
		ntm.NoTrans();

		// Apply the offset to the TM
		ntm = piv * ntm;

		// Apply a derived object to the node's object
		Object *obj = node->GetObjectRef();
		IDerivedObject *dobj = CreateDerivedObject(obj);
		
		// Create an XForm mod
		SimpleMod *mod = (SimpleMod*)ip->CreateInstance(
			OSM_CLASS_ID,
			Class_ID(CLUSTOSM_CLASS_ID,0));

		// Apply the transformation to the mod.
		SetXFormPacket pckt(ntm);
		mod->tmControl->SetValue(ip->GetTime(),&pckt);

		// Add the bend modifier to the derived object.
		dobj->SetAFlag(A_LOCK_TARGET); // RB 3/11/99: When the macro recorder is on the derived object will get deleted unless it is locked.
		dobj->AddModifier(mod);
		dobj->ClearAFlag(A_LOCK_TARGET);

		// Replace the node's object
		node->SetObjectRef(dobj);
		}
	
	//theHold.Accept(GetString(IDS_RB_RESETXFORM));
	GetSystemSetting(SYSSET_CLEAR_UNDO);
	ip->RedrawViews(ip->GetTime());
	SetSaveRequiredFlag(TRUE);
	}

#endif // NO_UTILITY_RESETXFORM
