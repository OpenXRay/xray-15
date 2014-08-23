/*===========================================================================*\
 | 
 |  FILE:	MorphPick.cpp
 |			MorpherView Utility - demonstrates use of MorpherAPI access
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 4-4-99
 | 
\*===========================================================================*/

#include "MorpherView.h"



/*===========================================================================*\
 |
 | The mini-mod stack window handler, for picking a morph modifier
 |
\*===========================================================================*/


BOOL CALLBACK BindProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	MorphViewUtil *mvup = (MorphViewUtil*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mvup && msg!=WM_INITDIALOG) return FALSE;

	int id = LOWORD(wParam);
	int notify = HIWORD(wParam);


	switch (msg) {
		case WM_INITDIALOG:{
			mvup = (MorphViewUtil*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG)mvup);

			HWND modList = GetDlgItem(hWnd,IDC_MODLIST);

			SendMessage(modList,LB_RESETCONTENT,0,0);

			POINT lpPt; GetCursorPos(&lpPt);
			SetWindowPos(hWnd, NULL, lpPt.x, lpPt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			Object *pObj = mvup->Wnode->GetObjectRef();
			IDerivedObject *pDerObj = NULL;
			Modifier *pMod = NULL;

			if( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) 
			{
				pDerObj = (IDerivedObject *) pObj;

				for(int i = 0 ; i < pDerObj->NumModifiers() ; i++ )
				{
					pMod = pDerObj->GetModifier(i);	
					SendMessage(modList,LB_ADDSTRING,0,(LPARAM) (LPCTSTR) pMod->GetName());
				}
			}

			SendMessage(modList,LB_SETCURSEL ,(WPARAM)-1,0);

			break;}


		case WM_COMMAND:

			if (notify==LBN_SELCHANGE){
				if(id==IDC_MODLIST){
					int mkSel = SendMessage(GetDlgItem(hWnd, IDC_MODLIST), LB_GETCURSEL, 0, 0);
					if(mkSel>=0){

						Object *pObj = mvup->Wnode->GetObjectRef();
						IDerivedObject *pDerObj = NULL;
						Modifier *pMod = NULL;

						if( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) 
						{
							pDerObj = (IDerivedObject *) pObj;
							pMod = pDerObj->GetModifier(mkSel);	
							if(pMod->ClassID() == MR3_CLASS_ID) EnableWindow(GetDlgItem(hWnd,IDOK),TRUE);
							else EnableWindow(GetDlgItem(hWnd,IDOK),FALSE);
						}


					}
				}
			}

			switch (id) {
				case IDOK:
				{
					int mkSel = SendMessage(GetDlgItem(hWnd, IDC_MODLIST), LB_GETCURSEL, 0, 0);
					if(mkSel>=0){

						Object *pObj = mvup->Wnode->GetObjectRef();
						IDerivedObject *pDerObj = NULL;
						Modifier *pMod = NULL;

						if( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) 
						{
							pDerObj = (IDerivedObject *) pObj;
							pMod = pDerObj->GetModifier(mkSel);	

							mvup->mp = (MorphR3*)pMod;
							SetWindowText(GetDlgItem(mvup->hPanel,IDC_OBJNAME),mvup->Wnode->GetName());
							InvalidateRect(mvup->hPanel,NULL,TRUE);
							mvup->LoadMorpherInfo(mvup->hPanel);
						}


					}
				}
				case IDCANCEL:
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
 | Pickmode support
 |
\*===========================================================================*/


BOOL  GetMorphMod_MorpherView
::Filter(INode *node)
{
	return TRUE;
}


BOOL  GetMorphMod_MorpherView::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
}

BOOL  GetMorphMod_MorpherView::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();

	if (node) {

		mvup->Wnode = node;

		int res = DialogBoxParam( 
			hInstance, 
			MAKEINTRESOURCE( IDD_GETMORPH ),
			GetDlgItem(mvup->hPanel,IDC_GET),
			(DLGPROC)BindProc,
			(LPARAM)(MorphViewUtil*)mvup);

	}
	
	return TRUE;
}


void  GetMorphMod_MorpherView::EnterMode(IObjParam *ip)
{
	isPicking=TRUE;
	if (mvup->pickBut) mvup->pickBut->SetCheck(TRUE);
}

void  GetMorphMod_MorpherView::ExitMode(IObjParam *ip)
{
	isPicking=FALSE;
	if (mvup->pickBut) mvup->pickBut->SetCheck(FALSE);
}






/*===========================================================================*\
 |
 | The pickmode for choosing nodes out of the scene to use as targets
 |
\*===========================================================================*/


BOOL  GetMorphTarget_MorpherView::Filter(INode *node)
{
	Interval valid; 
	
	ObjectState os = node->GetObjectRef()->Eval(mvup->ip->GetTime());

	if( os.obj->IsDeformable() == FALSE ) return FALSE;

	// Check for same-num-of-verts-count
	if( os.obj->NumPoints()!=mp->MC_Local.Count) return FALSE;

	node->BeginDependencyTest();
	mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	} else {
		return TRUE;
		
		}
}


BOOL  GetMorphTarget_MorpherView::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	
	if (mvup->ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
}

BOOL  GetMorphTarget_MorpherView::Pick(IObjParam *ip,ViewExp *vpt)
	{
	
	INode *node = vpt->GetClosestHit();
	if (node) {
		// Make the node reference, and then ask the channel to load itself

		UI_MAKEBUSY

		mp->ReplaceReference(101+idx,node);
		mp->chanBank[idx].buildFromNode(node);
		mvup->LoadMorpherInfo(mvup->hPanel);

		UI_MAKEFREE
	}
	
	return TRUE;
}


void  GetMorphTarget_MorpherView::EnterMode(IObjParam *ip)
{
	isPicking=TRUE;
	if (mvup->bnode) mvup->bnode->SetCheck(TRUE);
}

void  GetMorphTarget_MorpherView::ExitMode(IObjParam *ip)
{
	isPicking=FALSE;
	if (mvup->bnode) mvup->bnode->SetCheck(FALSE);
}