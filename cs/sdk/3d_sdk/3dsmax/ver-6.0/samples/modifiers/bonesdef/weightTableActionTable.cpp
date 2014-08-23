

#include "mods.h"

#include "bonesdef.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"



extern INT_PTR CALLBACK AddCustomListDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) ;

void WeightTableWindow::ActivateActionTable()
	{

	if (pCallback) DeActivateActionTable();

	pCallback = new WeightTableActionCallback();
	pCallback->SetWeightTable(this);

	if (GetCOREInterface()->GetActionManager()->ActivateActionTable(pCallback, kWeightTableActions) )
		{

		ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kWeightTableActions);
		if (actionTable)
			{
			for (int i =0; i < actionCount; i++)
				{
				WeightTableAction *action =  (WeightTableAction *) actionTable->GetAction(actionIDs[i]);
				if (action)
					{
					action->SetWeightTable(this);
					}

				}

			}
		}
	}

void WeightTableWindow::DeActivateActionTable()
	{
	ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kWeightTableActions);
	if (actionTable)
		{
		for (int i =0; i < actionCount; i++)
			{
			WeightTableAction *action =  (WeightTableAction *) actionTable->GetAction(actionIDs[i]);
			if (action)
				{
				action->SetWeightTable(NULL);
				}
			}
		}
	GetCOREInterface()->GetActionManager()->DeactivateActionTable(pCallback, kWeightTableActions); 
	delete pCallback;
	pCallback = NULL;
	}
BOOL WeightTableWindow::WtIsEnabled(int id)
	{
	switch (id)
		{
		case IDC_PASTE  :
			{
			if (copyBuffer.Count() == 0) return FALSE;
			else return TRUE;
			break;
			}
		}
	return TRUE;
	}


BOOL WeightTableWindow::WtIsChecked(int id)
	{
	switch (id)
		{
		//5.1.01
		case IDC_RIGHTJUSTIFY: 
			{
			return  GetRightJustify();
			break;
			}

		case IDC_NAMELISTGLOBAL_DROP: 
			{
			if (GetAffectSelectedOnly())
				return  FALSE;
			else return  TRUE;
			break;
			}
		case IDC_AFFECTEDBONES_CHECK :
			{
			return GetAffectedBonesOnly();
			break;
			}
		case IDC_UPDATEONMOUSEUP_CHECK2 :
			{
			return GetUpdateOnMouseUp();
			break;
			}
		case IDC_FLIPFLOPUI_CHECK2 :
			{
			return GetFlipFlopUI();
			break;
			}
		case IDC_ATTRIBUTE_CHECK2 :
			{
			return GetShowAttributes();
			break;
			}
		case IDC_GLOBAL_CHECK2 :
			{
			return GetShowGlobals();
			break;
			}
		case IDC_REDUCELABELS_CHECK2 :
			{
			return GetReduceLabels();
			break;
			}
		case IDC_SHOWEXCLUSION_CHECK :
			{
			return GetShowExclusion();
			break;
			}

		case IDC_SHOWLOCK_CHECK :
			{
			return GetShowLock();
			break;
			}

		case IDC_JBUIMETHOD :
			{
			return GetJBMethod();
			break;
			}
		case IDC_SHOWSETUI :
			{
			return GetShowSetUI();
			break;
			}

		case IDC_SHOWOPTIONSUI :
			{
			return GetShowOptionsUI();
			break;
			}
		case IDC_SHOWCOPYPASTEUI :
			{
			return GetShowCopyPasteUI();
			break;
			}

		case IDC_DRAGMODE :
			{
			return GetDragLeftMode();
			break;
			}
		case IDC_DEBUGMODE :
			{
			return GetDebugMode();
			break;
			}
		case IDC_SHOWMARKER :
			{
			return GetShowMarker();
			break;
			}
							


		}
	return FALSE;
	}




BOOL WeightTableWindow::WtExecute(int id)
	{

	BOOL iret = FALSE;
	switch (id)
		{
//edit
		case IDC_COPY :
			{
			SetCopyBuffer();
			iret = TRUE;
			UpdatePasteButton();
			break;
			}
		case IDC_PASTE :
			{
			PasteCopyBuffer();
			InvalidateViews();
			iret = TRUE;
			break;
			}
		case IDC_SELECTALL:
			{
			SetAllSelections();
			InvalidateViews();
			iret = TRUE;
			break;
			}
		case IDC_SELECTNONE:
			{
			ClearAllSelections();
			InvalidateViews();
			iret = TRUE;
			break;
			}
		case IDC_SELECTINVERT:
			{
			InvertSelections();
			InvalidateViews();
			iret = TRUE;
			break;
			}


//sets
		case IDC_CREATE :
			{
			DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_WEIGHTTABLE_CUSTOMLISTNAME),hWnd,
						AddCustomListDlgProc, (LPARAM)this);
			iret = TRUE;
			break;
			}
		case IDC_DELETE :
			{
			if (GetActiveSet() >2)
				DeleteCustomList(GetActiveSet());
			iret = TRUE;
//			PasteCopyBuffer();
//			InvalidateViews();

			break;
			}

//options
		case IDC_AFFECTEDBONES_CHECK :
			{
			if (GetAffectedBonesOnly())
				SetAffectedBonesOnly(FALSE);
			else SetAffectedBonesOnly(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_UPDATEONMOUSEUP_CHECK2 :
			{
			if (GetUpdateOnMouseUp())
				SetUpdateOnMouseUp(FALSE);
			else SetUpdateOnMouseUp(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_FLIPFLOPUI_CHECK2 :
			{
			if (GetFlipFlopUI())
				SetFlipFlopUI(FALSE);
			else SetFlipFlopUI(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_ATTRIBUTE_CHECK2 :
			{
			if (GetShowAttributes())
				SetShowAttributes(FALSE);
			else SetShowAttributes(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_GLOBAL_CHECK2 :
			{
			if (GetShowGlobals())
				SetShowGlobals(FALSE);
			else SetShowGlobals(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_REDUCELABELS_CHECK2 :
			{
			if (GetReduceLabels())
				SetReduceLabels(FALSE);
			else SetReduceLabels(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWEXCLUSION_CHECK :
			{
			if (GetShowExclusion())
				SetShowExclusion(FALSE);
			else SetShowExclusion(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWLOCK_CHECK :
			{
			if (GetShowLock())
				SetShowLock(FALSE);
			else SetShowLock(TRUE);
			iret = TRUE;
			break;
			}

		case IDC_NAMELISTGLOBAL_DROP :
			{
			if (GetAffectSelectedOnly())
				SetAffectSelectedOnly(FALSE);
			else SetAffectSelectedOnly(TRUE);
			iret = TRUE;
			break;
			}

		case IDC_JBUIMETHOD :
			{
			if (GetJBMethod())
				SetJBMethod(FALSE);
			else SetJBMethod(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWMENU :
			{
			if (GetShowMenu())
				SetShowMenu(FALSE);
			else SetShowMenu(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWSETUI :
			{
			if (GetShowSetUI())
				SetShowSetUI(FALSE);
			else SetShowSetUI(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWOPTIONSUI :
			{
			if (GetShowOptionsUI())
				SetShowOptionsUI(FALSE);
			else SetShowOptionsUI(TRUE);
			iret = TRUE;
			break;
			}
		case IDC_SHOWCOPYPASTEUI :
			{
			if (GetShowCopyPasteUI())
				SetShowCopyPasteUI(FALSE);
			else SetShowCopyPasteUI(TRUE);
			iret = TRUE;
			break;
			}

		case IDC_DRAGMODE :
			{
			if (GetDragLeftMode())
				SetDragLeftMode(FALSE);
			else SetDragLeftMode(TRUE);
			iret = TRUE;
			break;
			}

		case IDC_DEBUGMODE :
			{
			if (GetDebugMode())
				SetDebugMode(FALSE);
			else SetDebugMode(TRUE);
			iret = TRUE;
			break;
			}


		case IDC_SHOWMARKER :
			{
			if (GetShowMarker())
				SetShowMarker(FALSE);
			else SetShowMarker(TRUE);
			iret = TRUE;
			break;
			}

//5.1.01
		case IDC_RIGHTJUSTIFY: 
			{
			if (GetRightJustify())
				SetRightJustify(FALSE);
			else SetRightJustify(TRUE);
			break;
			}


		}
	
	IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kWeightTableMenuBar);
	if (pContext)
		pContext->UpdateWindowsMenu();

	return iret;
	}
