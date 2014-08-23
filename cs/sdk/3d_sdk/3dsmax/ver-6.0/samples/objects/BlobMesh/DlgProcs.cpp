
#include "BlobMesh.h"
#include "IParticleObjectExt.h"

static PickControlNode thePickMode;



void	BlobMesh::fnAddNode(INode *node)
{
	if (node == NULL) return;

	for (int i = 0; i < pblock2->Count(pb_nodelist); i++)
	{
		if (node == pblock2->GetINode(pb_nodelist, 0, i))
			return;
	}

	node->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return;
		} 



	theHold.Begin();
	pblock2->Append(pb_nodelist,1,&node);
	theHold.Accept(GetString(IDS_ADD));

	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void	BlobMesh::fnRemoveNode(INode *node)
{

	int index = -1;

	if (node == NULL) return;

	for (int i = 0; i < pblock2->Count(pb_nodelist); i++)
		{
		INode *pnode = NULL;
		pblock2->GetValue(pb_nodelist,0,pnode,FOREVER,i);
		if (pnode == node)
			{
			index = i;
			i = pblock2->Count(pb_nodelist);
			}
		}

	if (index != -1)
		{
		theHold.Begin();
		pblock2->Delete(pb_nodelist,index,1);
		theHold.Accept(GetString(IDS_REMOVE_BLOB));

		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
}


void	BlobMesh::fnAddPFNode(INode *node)
{
	if (node == NULL) return;

	for (int i = 0; i < pblock2->Count(pb_pfeventlist); i++)
	{
		if (node == pblock2->GetINode(pb_pfeventlist, 0, i))
			return;
	}

	node->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return;
		} 



	theHold.Begin();
	pblock2->Append(pb_pfeventlist,1,&node);
	theHold.Accept(GetString(IDS_ADD));

	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void	BlobMesh::fnRemovePFNode(INode *node)
{

	int index = -1;

	if (node == NULL) return;

	for (int i = 0; i < pblock2->Count(pb_pfeventlist); i++)
		{
		INode *pnode = NULL;
		pblock2->GetValue(pb_pfeventlist,0,pnode,FOREVER,i);
		if (pnode == node)
			{
			index = i;
			i = pblock2->Count(pb_pfeventlist);
			}
		}

	if (index != -1)
		{
		theHold.Begin();
		pblock2->Delete(pb_pfeventlist,index,1);
		theHold.Accept(GetString(IDS_REMOVE_BLOB));

		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
}


void	BlobMesh::fnPickMode()
{
	thePickMode.hWnd = paramsHWND;
	PickNodesMode();

	ICustButton *iBut = GetICustButton(GetDlgItem(paramsHWND, IDC_PICK));
	if (iBut)
		{
		iBut->SetType(CBT_CHECK);
		iBut->SetCheck( inPickMode);
		ReleaseICustButton(iBut);
		}
}

void	BlobMesh::fnAddMode()
{
	GetCOREInterface()->DoHitByNameDialog(new DumpHitDialog(this));
}
void	BlobMesh::fnAddPFMode()
{
	PickPFEvents(pfparamsHWND);
}

void	BlobMesh::RemoveSelectedNode()
{
	int index = 0;
	index = SendMessage(GetDlgItem(paramsHWND, IDC_LIST1),LB_GETCURSEL ,1,0);
	if (index == -1) return;
	int pblockIndex = 0;
	int ct = -1;
	for (int i = 0; i < pblock2->Count(pb_nodelist);i++)
		{
		INode *pnode = NULL;

		pblock2->GetValue(pb_nodelist,0,pnode,FOREVER,i);

		if (pnode != NULL)
			{
			ct++;
			if (ct == index)
				{
				pblockIndex = i;
				i = pblock2->Count(pb_nodelist);
				}
			}

		}

	if ((pblockIndex < 0) || (pblockIndex >=pblock2->Count(pb_nodelist)))
		return;

	INode * node = NULL;
	pblock2->GetValue(pb_nodelist,0,node,FOREVER,pblockIndex);

	if (node)
		{
//		fnRemoveNode(node);
		macroRecorder->FunctionCall(_T("$.blobMeshOps.RemoveBlob"), 1, 0, 
											mr_reftarg,node);

		}
		

}

void BlobMesh::PickNodesMode()
{
if (inPickMode)
	{
	inPickMode = FALSE;
	ip->ClearPickMode();
	}

else if (ip && (!inPickMode))
	{
	inPickMode = TRUE;
	thePickMode.obj  = this;					
	ip->SetPickMode(&thePickMode);
	}
}

void
BlobMesh::DisableButtons(HWND hWnd)
{
	if (GetCOREInterface()->GetCommandPanelTaskMode() == TASK_MODE_CREATE)
		{
		ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_ADD));
		if (iBut)
			{
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);
			}

		iBut = NULL;
		iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		if (iBut)
			{
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);
			}
			
		iBut = NULL;
		iBut = GetICustButton(GetDlgItem(hWnd, IDC_REMOVE));
		if (iBut)
			{
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);
			}
			

		}
	else
		{
		ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_ADD));
		if (iBut)
			{
			iBut->Enable(TRUE);
			iBut->SetHighlightColor(GREEN_WASH);
			
			ReleaseICustButton(iBut);
			}

		iBut = NULL;
		iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		if (iBut)
			{
			iBut->Enable(TRUE);
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			}

		}
}


BOOL BlobMeshDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			obj->pfparamsHWND = hWnd;	
			if (GetCOREInterface()->GetCommandPanelTaskMode() == TASK_MODE_CREATE)
			{
				EnableWindow(GetDlgItem(hWnd,IDC_ADDPF_EVENT_BUTTON),FALSE);
			}
					
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_ADDPF_EVENT_BUTTON:
					obj->PickPFEvents(hWnd);
					break;

				}
			break;
		}
	return FALSE;
	}




BOOL BlobMeshParamsDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			obj->paramsHWND = hWnd;
		
			obj->DisableButtons(hWnd);			
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_PICK:
					{
					thePickMode.hWnd = hWnd;
					obj->PickNodesMode();

					ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
					if (iBut)
						{
						iBut->SetType(CBT_CHECK);
						iBut->SetCheck( obj->inPickMode);
						ReleaseICustButton(iBut);
						}

					break;
					}

				case IDC_ADD:
					{
					obj->ip->DoHitByNameDialog(new DumpHitDialog(obj));
					break;
					}
//				case IDC_REMOVE:
//					{
//					obj->RemoveSelectedNode();
//					}

				}
			break;
		}
	return FALSE;
	}


BOOL PickControlNode::Filter(INode *node)
	{

	for (int i = 0; i < obj->pblock2->Count(pb_nodelist); i++)
	{
		if (node == obj->pblock2->GetINode(pb_nodelist, 0, i))
			return FALSE;
	}

	node->BeginDependencyTest();
	obj->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 

	return TRUE;
}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) 
		{
		theHold.Begin();
		obj->pblock2->Append(pb_nodelist,1,&node);
		theHold.Accept(GetString(IDS_ADD));
		obj->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		obj->ip->RedrawViews(obj->ip->GetTime());

		macroRecorder->FunctionCall(_T("$.blobMeshOps.AddBlob"), 1, 0, 
											mr_reftarg,node);

		}
	return FALSE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{


	}
HCURSOR PickControlNode::GetDefCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ARROW)); 
        }
	return hCur;
	}

HCURSOR PickControlNode::GetHitCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CROSS)); 
        }
	return hCur;
	}


void PickControlNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
	if (iBut)
		{
		iBut->SetCheck( FALSE);
		ReleaseICustButton(iBut);
		}

	obj->inPickMode = FALSE;	
	}



void DumpHitDialog::proc(INodeTab &nodeTab)

{


int nodeCount = nodeTab.Count(); 

if (nodeCount == 0) return;

theHold.Begin();

for (int i=0;i<nodeTab.Count();i++)
	{

	eo->pblock2->Append(pb_nodelist,1,&nodeTab[i]);
	macroRecorder->FunctionCall(_T("$.blobMeshOps.AddBlob"), 1, 0, 
											mr_reftarg,nodeTab[i]);

	}

theHold.Accept(GetString(IDS_ADD));
eo->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
eo->ip->RedrawViews(eo->ip->GetTime());

}


int DumpHitDialog::filter(INode *node)

{

	for (int i = 0; i < eo->pblock2->Count(pb_nodelist); i++)
	{
		if (node == eo->pblock2->GetINode(pb_nodelist, 0, i))
			return FALSE;
	}

	node->BeginDependencyTest();
	eo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 

	return TRUE;
}



