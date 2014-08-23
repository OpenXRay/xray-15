/*===========================================================================*\
 | 
 |  FILE:	wM3_material_pick.cpp
 |			Weighted Morpher for MAX R3
 |			Pickmode for the morph material
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 23-12-98
 | 
\*===========================================================================*/

#include "wM3.h"

/*===========================================================================*\
 |
 | The mini-mod stack window handler, for picking a morph modifier
 |
\*===========================================================================*/


INT_PTR CALLBACK BindProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	M3Mat *mp = (M3Mat*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mp && msg!=WM_INITDIALOG) return FALSE;

	int id = LOWORD(wParam);
	int notify = HIWORD(wParam);


	switch (msg) {
		case WM_INITDIALOG:{
			mp = (M3Mat*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)mp);

			HWND modList = GetDlgItem(hWnd,IDC_MODLIST);

			SendMessage(modList,LB_RESETCONTENT,0,0);

			POINT lpPt; GetCursorPos(&lpPt);
			SetWindowPos(hWnd, NULL, lpPt.x, lpPt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			Object *pObj = mp->Wnode->GetObjectRef();
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

						Object *pObj = mp->Wnode->GetObjectRef();
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

						Object *pObj = mp->Wnode->GetObjectRef();
						IDerivedObject *pDerObj = NULL;
						Modifier *pMod = NULL;

						if( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) 
						{
							pDerObj = (IDerivedObject *) pObj;
							pMod = pDerObj->GetModifier(mkSel);	
							
							MorphR3 *mod = (MorphR3*)pMod;
							if(mod->CheckMaterialDependency() ) {
								EndDialog(hWnd,1);return TRUE;
							}

							// Make sure the node does not depend on us
							mod->BeginDependencyTest();
							mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
							if (mod->EndDependencyTest()) { 
								// display cyclic warning
								//
								if (GetCOREInterface()->GetQuietMode()) {
									TSTR cyclic;
									cyclic = GetString(IDS_CANNOT_BIND);
									GetCOREInterface()->Log()->LogEntry(SYSLOG_WARN,NO_DIALOG,GetString(IDS_CLASS_NAME),cyclic);
								}
								else
								{
									TSTR cyclic;
									cyclic = GetString(IDS_CANNOT_BIND);
									MessageBox(hWnd,cyclic,GetString(IDS_CLASS_NAME),MB_OK);
								}
								EndDialog(hWnd,1);
								return TRUE; 
							}

							if(mod) mod->morphmaterial = mp;

							mp->ReplaceReference(102, mod);
							mp->obName = mp->Wnode->GetName();

							mp->morphp = mod;
							if (mp->matDlg ){
								mp->matDlg->UpdateMorphInfo(UD_LINK);
								mp->matDlg->ReloadDialog();
							}
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


BOOL  GetMorphMod::Filter(INode *node)
{
	Interval valid; 
	if(!mp || !mp->matDlg) return FALSE;
	
	ObjectState os = node->GetObjectRef()->Eval(mp->matDlg->ip->GetTime());

	if( os.obj->IsDeformable() == FALSE ) return FALSE;

	return TRUE;
}


BOOL  GetMorphMod::Pick(INode *node)
	{
	if (node &&  mp->matDlg) {

		mp->Wnode = node;

		int res = DialogBoxParam( 
			hInstance, 
			MAKEINTRESOURCE( IDD_BINDMORPH ),
			GetDlgItem(mp->matDlg->hPanel,IDC_BIND),
			(DLGPROC)BindProc,
			(LPARAM)(M3Mat*)mp
			);

	}
	
	return TRUE;
}


void  GetMorphMod::EnterMode()
{
	isPicking=TRUE;
	if (mp->matDlg && mp->matDlg->pickBut) mp->matDlg->pickBut->SetCheck(TRUE);
}

void  GetMorphMod::ExitMode()
{
	isPicking=FALSE;
	if (mp->matDlg && mp->matDlg->pickBut) mp->matDlg->pickBut->SetCheck(FALSE);
}