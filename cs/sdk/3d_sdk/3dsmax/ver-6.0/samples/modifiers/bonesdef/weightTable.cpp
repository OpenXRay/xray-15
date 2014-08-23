/********************************************************************************************************

  
THINGS TO DO

BUGS

	window update bug when flip flopping UI


	Docked windows not saving right

	macro record everything
  
	display locked
	edit locked
		

*********************************************************************************************/



#include "mods.h"

#include "bonesdef.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"


//proc to just loop through all dependant nodes
int WeightTableEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
		Nodes.Append(1, (INode **)&rmaker);  
		}
	return 0;
	}	






WeightTableWindow::~WeightTableWindow()
	{
	FreeData();
	GetCOREInterface()->UnRegisterViewWindow(this);
	mod = NULL;
	}
WeightTableWindow::WeightTableWindow()
	{

	pCallback = NULL;

	toolTipHwnd = NULL;

	hWnd = NULL;
	hWeightList = NULL;
	hNameList = NULL;
	hAttribList = NULL;

	hWeight = NULL;
//	affectedBonesOnly = FALSE;

//	affectSelectedOnly = FALSE;
//	updateOnMouseUp = FALSE;

	vertNameWidth = 120;
	buttonWidth = 80;
	numAttributes = 5;
//	flipFlopUI = FALSE;
	leftBorder = 10;
//	topBorder = 120;

	iWeight = NULL;

	selectionColor = RGB(255,0,0);
	textColor = RGB(0,0,0);
	backColor = RGB(15,15,15);


	textHeight = 14;

	firstRow = 0;
	firstColumn = 0;

	iWeightListBuf = NULL;
	iNameListBuf = NULL;
	iAttribListBuf = NULL;

	iWeightListLabelBuf = NULL;
	iNameListLabelBuf = NULL;
	iAttribListLabelBuf = NULL;
		
	iWeightListGlobalBuf = NULL;
//	iNameListGlobalBuf = NULL;
	iAttribListGlobalBuf = NULL;

//	showAttributes = TRUE;
//	showGlobals = TRUE;
//	reduceLabels = FALSE;

//	showExclusion = TRUE;
//	showLock  = TRUE;

	isDocked = FALSE;

	clearBack = FALSE;
	GetCOREInterface()->RegisterViewWindow(this);

	iGlobalWeight = NULL;
	iWeight = NULL;

	kWeightTableMenuBar = 2182123;
//put back after finish menu
	if (GetCOREInterface()->GetMenuManager()->RegisterMenuBarContext(kWeightTableMenuBar, _T("Skin Weight Table Menu Bar"))) 
		{

	// if not already registered, create the menu it needs

		IMenu* pMenu = GetCOREInterface()->GetMenuManager()->FindMenu(_T("Skin Weight Table Menu Bar"));
		if (pMenu)
			{
			GetCOREInterface()->GetMenuManager()->UnRegisterMenu(pMenu);
			pMenu=NULL;
			}

		if (!pMenu) 
			{
			pMenu = GetIMenu();
			pMenu->SetTitle("Skin Weight Table Menu Bar");
			GetCOREInterface()->GetMenuManager()->RegisterMenu(pMenu, 0);

			//add Edit Menu
			IMenu* pEditSubMenu = GetIMenu();
			pEditSubMenu->SetTitle("Edit");

			//add Sets menu
			IMenu* pSetsSubMenu = GetIMenu();
			pSetsSubMenu->SetTitle("Vertex Sets");
		
			//add Opitons menu
			IMenu* pOptionsSubMenu = GetIMenu();
			pOptionsSubMenu->SetTitle("Options");

			ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kWeightTableActions);
			
			if (!GetCOREInterface()->GetMenuManager()->RegisterMenu(pEditSubMenu, 0))
				DebugPrint("Skin Weight Table Edit Menu not registered\n");
			else if (actionTable)
				{
				for (int i =0; i < (actionSets); i++)
					{
					IMenuItem* pMenuItem1 = GetIMenuItem();
					WeightTableAction *action =  (WeightTableAction *) actionTable->GetAction(actionIDs[i]);
					if (action)
						{
						pMenuItem1->SetActionItem((ActionItem*)action);
						pEditSubMenu->AddItem(pMenuItem1);
						}
					}
         // Create a new menu item to hold the sub-menu
				IMenuItem* pSubMenuItem1 = GetIMenuItem();   //menu in menu bar...
				pSubMenuItem1->SetSubMenu(pEditSubMenu);
		// Add the sub-menu item to the main menu bar
				pMenu->AddItem(pSubMenuItem1);
				}

			if (!GetCOREInterface()->GetMenuManager()->RegisterMenu(pSetsSubMenu, 0))
				DebugPrint("Skin Weight Table Sets Menu not registered\n");
			else if (actionTable)
				{
				for (int i =actionSets; i < actionOption; i++)
					{
					IMenuItem* pMenuItem1 = GetIMenuItem();
					WeightTableAction *action =  (WeightTableAction *) actionTable->GetAction(actionIDs[i]);
					if (action)
						{
						pMenuItem1->SetActionItem((ActionItem*)action);
						pSetsSubMenu->AddItem(pMenuItem1);
						}
					}
				
         // Create a new menu item to hold the sub-menu
				IMenuItem* pSubMenuItem1 = GetIMenuItem();   //menu in menu bar...
				pSubMenuItem1->SetSubMenu(pSetsSubMenu);
		// Add the sub-menu item to the main menu bar
				pMenu->AddItem(pSubMenuItem1);
				}
			if (!GetCOREInterface()->GetMenuManager()->RegisterMenu(pOptionsSubMenu, 0))
				DebugPrint("Skin Weight Table Options Menu not registered\n");
			else if (actionTable)
				{
				for (int i = actionOption; i < actionOptionEnd; i++)
					{
					IMenuItem* pMenuItem1 = GetIMenuItem();
					WeightTableAction *action =  (WeightTableAction *) actionTable->GetAction(actionIDs[i]);
					if (action)
						{
						pMenuItem1->SetActionItem((ActionItem*)action);
						pOptionsSubMenu->AddItem(pMenuItem1);
						}
					}

				IMenuItem* pSubMenuItem1 = GetIMenuItem();   //menu in menu bar...
				pSubMenuItem1->SetSubMenu(pOptionsSubMenu);
		// Add the sub-menu item to the main menu bar
				pMenu->AddItem(pSubMenuItem1);

				}
			}
		assert(pMenu);
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kWeightTableMenuBar);
		pContext->SetMenu(pMenu);
		pContext->CreateWindowsMenu();
		}

	}


HWND WeightTableWindow::CreateViewWindow(HWND hParent, int x, int y, int w, int h)
	{
	if (hWnd == NULL)
		{
//create the window
		if (mod) mod->fnWeightTable();
		}
	ShowWindow(hWnd, SW_HIDE);
	GetWindowPlacement(hWnd, &lwPlacement);
	lwParent = SetParent(hWnd, hParent);
	lwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
	MoveWindow(hWnd, x, y, w, h, TRUE);
	ShowWindow(hWnd, SW_RESTORE);
	isDocked = TRUE;

	return hWnd;
	}

void WeightTableWindow::DestroyViewWindow(HWND hWnd)
{

	ShowWindow(hWnd, SW_HIDE);
	SetParent(hWnd, lwParent);
	lwPlacement.showCmd = SW_SHOW;
	SetWindowPlacement(hWnd, &lwPlacement);
	SetWindowLongPtr(hWnd, GWL_STYLE, lwStyle);
	ShowWindow(hWnd, SW_SHOWMINIMIZED);
	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(hWnd, SW_RESTORE);

	isDocked = FALSE;
}


BOOL WeightTableWindow::CanCreate()
	{
	if (mod == NULL) return FALSE;
	return TRUE;
	}
TCHAR* WeightTableWindow::GetName()
	{
	viewWindowName.printf(GetString(IDS_SKIN_WEIGHT_TABLE));
	if (modDataList.Count() == 1)
		{
		if (modDataList[0].node)
			viewWindowName.printf(GetString(IDS_SKIN_WEIGHT_TABLE_ONE),modDataList[0].node->GetName());
		}
	if (modDataList.Count() > 1)
		{
		int end = modDataList.Count()-1;
		if ((modDataList[0].node) && (modDataList[end].node))
			viewWindowName.printf(GetString(IDS_SKIN_WEIGHT_TABLE_MULTI),modDataList[0].node->GetName(),modDataList[end].node->GetName());
		}

	return viewWindowName.data(); 
	}



void WeightTableWindow::InitMod(BonesDefMod *mod)
	{
	firstRow = 0;
	this->mod = mod;

	BuildModDataList();
	}

void WeightTableWindow::ClearMod()
	{
	mod = NULL;
	}


void WeightTableWindow::BuildModDataList()
	{

	WeightTableEnumProc dep;  
	modDataList.ZeroCount();
	if (mod)
		{
		mod->EnumDependents(&dep);


		//copy all the local data and nodes in a table s we can reference it later
		for (int i =0 ; i < dep.Nodes.Count(); i++)
			{
			ModData temp;
			temp.node = dep.Nodes[i];
			temp.bmd = mod->GetBMD(dep.Nodes[i]);
			if (temp.bmd)
				{
				modDataList.Append(1,&temp);
				}
			}
		}
	}


void WeightTableWindow::CreateGDIData()
	{
	pTextPen   = CreatePen(PS_SOLID,1,textColor);
	pSelPen   = CreatePen(PS_SOLID,1,selectionColor);
	pBackPen   = CreatePen(PS_SOLID,1,backColor);


	hFixedFont = CreateFont(GetFontSize(),0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY, VARIABLE_PITCH  | FF_SWISS, _T(""));
	hFixedFontBold = CreateFont(GetFontSize(),0,0,0,700,0,0,0,0,0,0,PROOF_QUALITY, VARIABLE_PITCH  | FF_SWISS, _T(""));
	}	

void WeightTableWindow::RecomputeBones()
	{
	if ((mod))
		{
		if (hWnd)
			{
			UpdateWindowControls();
			BringDownEditField();
			ResizeWindowControls();
			UpdateUI();
			InvalidateViews();
			}
		}

	}

void WeightTableWindow::UpdateWindowControls()
	{
//init vertex filter drop list
	FillOutSets();

//fill out vertex list
	FillOutVertexList();

//fillout bone list
	ComputeActiveBones();
	ResizeWindowControls();

//init check boxes from last save
//build attrib list
//build weight list
//setup global checks

	}

void WeightTableWindow::InitWindowControl()
	{
//init vertex filter drop list
//	SetActiveSet(0);

	hWeight = CreateWindowEx(0,
							CUSTEDITWINDOWCLASS,
							_T(""),
							WS_CHILD | WS_TABSTOP | WS_GROUP,
							0, 0, 0, 0,    
							hWeightList, (HMENU) IDC_WEIGHT, hInstance, NULL);

//create the spinner window handle
	tempSpinner = CreateWindowEx(0,
							SPINNERWINDOWCLASS,
							_T(""),
							WS_CHILD,
							0, 0, 0, 0,    
							hWeightList,(HMENU) IDC_WEIGHT_SPIN, hInstance, NULL);
 
//create a spinner 
	ShowWindow( hWeight, SW_HIDE);
	ShowWindow( tempSpinner, SW_HIDE);

	hGlobalWeight = CreateWindowEx(0,
							CUSTEDITWINDOWCLASS,
							_T(""),
							WS_CHILD | WS_TABSTOP | WS_GROUP,
							0, 0, 0, 0,    
							hWeightListGlobal, (HMENU) IDC_WEIGHT, hInstance, NULL);

//create the spinner window handle
	hGlobalSpinner = CreateWindowEx(0,
							SPINNERWINDOWCLASS,
							_T(""),
							WS_CHILD,
							0, 0, 0, 0,    
							hWeightListGlobal,(HMENU) IDC_WEIGHT_SPIN, hInstance, NULL);
 
//create a spinner 
	ShowWindow( hGlobalWeight, SW_HIDE);
	ShowWindow( hGlobalSpinner, SW_HIDE);

	SendMessage(hGlobalWeight, WM_SETFONT, (WPARAM)hFixedFont, MAKELONG(0, 0));
	SendMessage(hWeight, WM_SETFONT, (WPARAM)hFixedFont, MAKELONG(0, 0));

	UpdateWindowControls();
	}

void WeightTableWindow::FillOutSets()
	{
//get window
	HWND hSets = GetDlgItem(hWnd,IDC_SETS);
//do standard sets
	SendMessage(hSets, CB_RESETCONTENT, 0, 0);

	SendMessage(hSets, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_ALLVERTS));
	SendMessage(hSets, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_SELECTEDVERTS));
	SendMessage(hSets, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_SELECTEDBONE));

	for (int i =0; i < customLists.Count(); i++)
		{
		SendMessage(hSets, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) customLists[i]->name.data());
		}

	SendMessage(hSets, CB_SETCURSEL , (WPARAM) GetActiveSet(), 0);
	}

void WeightTableWindow::FillOutVertexList()
	{

//get window

	BitArray verts;
	BitArray oldSelState;

	oldSelState.SetSize(vertexPtrList.Count());
	oldSelState.ClearAll();

	for (int i = 0; i < oldSelState.GetSize(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			oldSelState.Set(i);
		}




//do all
	vertexPtrList.ZeroCount();

	if (mod== NULL) return;

	if (GetActiveSet() == 0)
		{
		for (int i =0; i < modDataList.Count(); i++)
			{
			for (int j =0; j < modDataList[i].bmd->VertexData.Count(); j++)
				{
				TSTR vertName;
				vertName.printf("#%d Vertex (%s)",j,modDataList[i].node->GetName());
				VertexDataClass tempData;
				tempData.index = j;
				tempData.node = modDataList[i].node;
				tempData.bmd = modDataList[i].bmd;

				tempData.vertexData = modDataList[i].bmd->VertexData[j];
				vertexPtrList.Append(1,&tempData);
				}
			}
		}
//do selected
	else if (GetActiveSet() == 1)
		{
		for (int i =0; i < modDataList.Count(); i++)
			{
			for (int j =0; j < modDataList[i].bmd->VertexData.Count(); j++)
				{
				if (modDataList[i].bmd->selected[j])
					{
					TSTR vertName;
					vertName.printf("#%d Vertex (%s)",j,modDataList[i].node->GetName());
					VertexDataClass tempData;
					tempData.index = j;
					tempData.node = modDataList[i].node;
					tempData.bmd = modDataList[i].bmd;
					tempData.vertexData = modDataList[i].bmd->VertexData[j];
					vertexPtrList.Append(1,&tempData);
					}
				}
			}
		}
//do bone
	else if (GetActiveSet() == 2)
		{
		for (int i =0; i < modDataList.Count(); i++)
			{
			for (int j =0; j < modDataList[i].bmd->VertexData.Count(); j++)
				{
				for (int k =0; k < modDataList[i].bmd->VertexData[j]->d.Count(); k++)
					{
					if (modDataList[i].bmd->VertexData[j]->d[k].Bones == mod->ModeBoneIndex)
						{
						TSTR vertName;
						vertName.printf("#%d Vertex (%s)",j,modDataList[i].node->GetName());
						
						VertexDataClass tempData;
						tempData.index = j;
						tempData.node = modDataList[i].node;
						tempData.bmd = modDataList[i].bmd;
						tempData.vertexData = modDataList[i].bmd->VertexData[j];

						vertexPtrList.Append(1,&tempData);

						k = modDataList[i].bmd->VertexData[j]->d.Count();
						}
					}
				}
			}
		}
//look through created sets 
	else 
		{
		int customListIndex = GetActiveSet() -3;
		if ((customListIndex >=0) && (customListIndex < customLists.Count()))
			{
			CustomListClass *cl = customLists[customListIndex];
			for (int i =0; i<cl->data.Count(); i++)
				{
				for (int j =0; j<cl->data[i]->usedList.Count(); j++)
					{

					int index = cl->data[i]->usedList[j];
					VertexDataClass tempData;
					tempData.index = index;
					tempData.node = cl->data[i]->node;
//					tempData.name = vertName;

					
					int modIndex= -1;
					for (int k = 0; k < modDataList.Count(); k++)
						{
						if (cl->data[i]->node == modDataList[k].node)
							{
							modIndex = k;
							k = modDataList.Count();
							}
						}
					if (modIndex >=0)
						{
						tempData.vertexData = modDataList[modIndex].bmd->VertexData[index];
						tempData.bmd = modDataList[modIndex].bmd;
						vertexPtrList.Append(1,&tempData);
						}

					}

				}
			}
		}
	if (GetFlipFlopUI())
		{
		if (firstColumn>=vertexPtrList.Count())
			firstColumn = 0;
		}
	else
		{
		if (firstRow>=vertexPtrList.Count())
			firstRow = 0;
		}


	for (i = 0; i < oldSelState.GetSize(); i++)
		{

		if ((oldSelState[i]) && (i < vertexPtrList.Count()))
			vertexPtrList[i].Select(TRUE);
			
		}


	InvalidateViews();

	}


void WeightTableWindow::ComputeNumberRowsColumns()
	{
//compute the number of active columns
	WINDOWPLACEMENT winPos;
	//compute the number of buttons visible
	if (GetFlipFlopUI())
		{
		GetWindowPlacement(hNameList , &winPos);
		int width = (winPos.rcNormalPosition.right-winPos.rcNormalPosition.left);

		numberOfColumns = (int)floor((float)width/(float)vertNameWidth) + 1 ;
//now limit this by the actual number of bones in the list
		if (numberOfColumns > vertexPtrList.Count())
			numberOfColumns = vertexPtrList.Count();

		}
	else
		{
		GetWindowPlacement(hWeightList , &winPos);
		int width = (winPos.rcNormalPosition.right-winPos.rcNormalPosition.left);

		numberOfColumns = (int)floor((float)width/(float)buttonWidth) + 1;
//now limit this by the actual number of bones in the list
		if (numberOfColumns > activeBones.Count())
			numberOfColumns = activeBones.Count();
		}


//compute the number of active rows
	if (GetFlipFlopUI())
		{
		GetWindowPlacement(hWeightList , &winPos);
		int height = (winPos.rcNormalPosition.bottom-winPos.rcNormalPosition.top);
		numberOfRows = (int)floor((float)height/(float)textHeight) + 1 ;  // the minus 2 is because the first 2 rows are reserved
																		// the first row is the label row
																		//the second row is the global row
//now limit this by the actual number of vertices
		if (numberOfRows > activeBones.Count())
			numberOfRows = activeBones.Count();

		}
	else
		{
		GetWindowPlacement(hNameList , &winPos);
		int height = (winPos.rcNormalPosition.bottom-winPos.rcNormalPosition.top);
		numberOfRows = (int)floor((float)height/(float)textHeight) + 1 ;  
//now limit this by the actual number of vertices
		if (numberOfRows > vertexPtrList.Count())
			numberOfRows = vertexPtrList.Count();
		}
	}


void WeightTableWindow::ComputeActiveBones()
	{

	activeBones.ZeroCount();
	if (mod == NULL) return;

//bit arrray of all the active bone
	BitArray activeBoneBits;
	activeBoneBits.SetSize(mod->BoneData.Count());
	activeBoneBits.SetAll();
	int numberOfBones;

	if (GetAffectedBonesOnly())
		{
		
		activeBoneBits.ClearAll();
//loop through all the current vert
//tagging all bones that are currentle affected
		for (int i =0; i < vertexPtrList.Count(); i++)
			{
			VertexListClass *vd;
			vd = vertexPtrList[i].vertexData;
			for (int j = 0; j < vd->d.Count(); j++)
				{
				int boneId;
				boneId = vd->d[j].Bones;
				if ((boneId >=0) && (boneId < activeBoneBits.GetSize()))
					activeBoneBits.Set(boneId);
				}
			}
		numberOfBones = activeBoneBits.NumberSet();
		}
	else
		{
		numberOfBones = 0;
		for (int i=0; i < mod->BoneData.Count(); i++)
			{
//check if bone to be drawn
			if (mod->BoneData[i].Node != NULL) numberOfBones++;
			}

		}
//paint name across top
//start with first bone

	int ct = 0;
	for ( int i=0; i < mod->BoneData.Count(); i++)
		{
//check if bone to be drawn
		int index = i;
		if (activeBoneBits[i])
			{
			if (mod->BoneData[index].Node != NULL)
				{
				activeBones.Append(1,&index);
				}
			}


		}

	}

void WeightTableWindow::SetWindowState()
	{
	int x,y,width, height;
	if (mod)
		{
		mod->pblock_weighttable->GetValue(skin_wt_xpos,0,x,FOREVER);
		mod->pblock_weighttable->GetValue(skin_wt_ypos,0,y,FOREVER);
		mod->pblock_weighttable->GetValue(skin_wt_width,0,width,FOREVER);
		mod->pblock_weighttable->GetValue(skin_wt_height,0,height,FOREVER);
		
		HWND hwndOwner = GetDesktopWindow(); 
		RECT rcOwner; 
		GetWindowRect(hwndOwner, &rcOwner); 

		if (x >= rcOwner.right) x = 0;
		if (y >= rcOwner.bottom) y = 0;

		if (x < rcOwner.left) x = 0;
		if (y < rcOwner.top) y = 0;
		
		if (width > (rcOwner.right - rcOwner.left)) width = (rcOwner.right - rcOwner.left);
		if (height > (rcOwner.bottom - rcOwner.top)) width = (rcOwner.bottom - rcOwner.top);
		
		}

	MoveWindow(hWnd,x,y,width,height,SW_SHOW);

	}

void WeightTableWindow::SaveWindowState()
	{
	WINDOWPLACEMENT hwndWinPos;
	GetWindowPlacement(hWnd , &hwndWinPos);
	int width, height;
	width = hwndWinPos.rcNormalPosition.right - hwndWinPos.rcNormalPosition.left;
	height = hwndWinPos.rcNormalPosition.bottom - hwndWinPos.rcNormalPosition.top;
	if (mod)
		{
		mod->pblock_weighttable->SetValue(skin_wt_xpos,0,hwndWinPos.rcNormalPosition.left);
		mod->pblock_weighttable->SetValue(skin_wt_ypos,0,hwndWinPos.rcNormalPosition.top);
		mod->pblock_weighttable->SetValue(skin_wt_width,0,width);
		mod->pblock_weighttable->SetValue(skin_wt_height,0,height);
		}
	

	}

void WeightTableWindow::DockWindow(HWND child, HWND left, HWND top, int spacing, BOOL matchWidth, BOOL matchHeight, 
								   int width, int height )
	{
	WINDOWPLACEMENT leftWinPos, topWinPos, childWinPos;
	if (left)
		GetWindowPlacement(left , &leftWinPos);
	if (top)
		GetWindowPlacement(top , &topWinPos);

	GetWindowPlacement(child , &childWinPos);
	

	int x,y;
	if ((matchWidth == TRUE) && (top))
		{
		width = topWinPos.rcNormalPosition.right - topWinPos.rcNormalPosition.left;
		}
	else if ((matchWidth == FALSE) && (width == 0))
		{
		width = childWinPos.rcNormalPosition.right  - childWinPos.rcNormalPosition.left;
		}

	if ( (matchHeight == TRUE) && (left))
		{
		height = leftWinPos.rcNormalPosition.bottom - leftWinPos.rcNormalPosition.top;
		}
	else if ((matchHeight == FALSE) && ( height == 0))
		{
		height = childWinPos.rcNormalPosition.bottom  - childWinPos.rcNormalPosition.top;
		}

	if ((left) && (top))
		{
		x = leftWinPos.rcNormalPosition.left + (leftWinPos.rcNormalPosition.right- leftWinPos.rcNormalPosition.left) + spacing;
		y = leftWinPos.rcNormalPosition.top;
		}
	else if (!left)
		{
		x = topWinPos.rcNormalPosition.left;
		y = topWinPos.rcNormalPosition.bottom + spacing;
		}
	else if (left)
		{
		x = leftWinPos.rcNormalPosition.left + (leftWinPos.rcNormalPosition.right- leftWinPos.rcNormalPosition.left) + spacing;
		y = leftWinPos.rcNormalPosition.top;
		}


	MoveWindow(child,x,y,width,height, TRUE);
	}

void WeightTableWindow::ResizeWindowControls()
	{

//resize the bones buttong

	if (mod == NULL) return;

	WINDOWPLACEMENT mainWinPos;
//	GetWindowPlacement(hWnd , &mainWinPos);
	GetClientRect(hWnd , &mainWinPos.rcNormalPosition);
	int mainHeight = mainWinPos.rcNormalPosition.bottom - mainWinPos.rcNormalPosition.top;
	int mainWidth = mainWinPos.rcNormalPosition.right - mainWinPos.rcNormalPosition.left;



//	int spinnerX = 0;
//	int spinnerY = 0;

//sets the Vertex Set control pos		
	HWND hVertexSet = GetDlgItem(hWnd,IDC_VERTEX_STATIC);
	int vertexSetX = leftBorder;
	int vertexSetY = leftBorder/2;
	int vertexSetWidth = vertNameWidth-2;
	if (GetFlipFlopUI())
		vertexSetWidth = buttonWidth-2;
	int vertexSetHeight = GetTopBorder() - (leftBorder*2);

//	if (!GetFlipFlopUI())
		MoveWindow(hVertexSet,vertexSetX,vertexSetY,vertexSetWidth, vertexSetHeight, TRUE);

//sets the Create/Delete button pos
	HWND hCreate = GetDlgItem(hWnd,IDC_CREATE);
	int createWidth = vertNameWidth-13;
	if (GetFlipFlopUI())
		createWidth = buttonWidth-13;

	int createHeight = 20;
	int createX = leftBorder+5;
	int createY = vertexSetY+15;
//	if (!GetFlipFlopUI())
		MoveWindow(hCreate,createX,createY,createWidth,createHeight, TRUE);

	HWND hDelete = GetDlgItem(hWnd,IDC_DELETE);
	DockWindow(hDelete, NULL, hCreate, 4, TRUE, FALSE);




//sets the Copy/Paste button pos should be moved
	HWND hCopy = GetDlgItem(hWnd,IDC_COPY);

	int copyWidth = vertNameWidth+1;
	if (GetFlipFlopUI())
		copyWidth = buttonWidth+1;
	copyWidth /= 2;

	int copyHeight = 16;
	int copyX = leftBorder;
	int copyY = mainHeight-20;
	if (isDocked)
		copyY += 28;

	if (!GetShowMenu()) copyY += 14;

//	if (!flipFlopUI)
		MoveWindow(hCopy,copyX,copyY,copyWidth,copyHeight, TRUE);
		
	HWND hPaste = GetDlgItem(hWnd,IDC_PASTE);
	DockWindow(hPaste, hCopy, NULL, 0, FALSE, TRUE,copyWidth);




//set the Options control pos
	HWND hOptionStatic = GetDlgItem(hWnd,IDC_OPTION_STATIC);
 	int optionStaticX = vertexSetX+vertexSetWidth + (numAttributes*textHeight)+10;
	int optionStaticY = vertexSetY;
	int optionStaticWidth = 350;
	int optionStaticHeight = vertexSetHeight + 6;
	MoveWindow(hOptionStatic,optionStaticX,optionStaticY,optionStaticWidth, optionStaticHeight, TRUE);

	HWND hShowBones = GetDlgItem(hWnd,IDC_AFFECTEDBONES_CHECK);
	WINDOWPLACEMENT winBonePos;
	GetWindowPlacement(hShowBones , &winBonePos);
//do the check box option placements
	int showBonesX = optionStaticX+10;
	int showBonesY = optionStaticY+15;
	int showBonesWidth = 150;
	int showBonesHeight = 14;
	MoveWindow(hShowBones,showBonesX,showBonesY,showBonesWidth, showBonesHeight, TRUE);
		
	HWND hUpdateUI = GetDlgItem(hWnd,IDC_UPDATEONMOUSEUP_CHECK2);
	DockWindow(hUpdateUI, NULL, hShowBones, 2, TRUE, FALSE,0,14);

		
	HWND hFlipFlopUI = GetDlgItem(hWnd,IDC_FLIPFLOPUI_CHECK2);
	DockWindow(hFlipFlopUI, NULL, hUpdateUI, 2, TRUE, FALSE,0,14);

	HWND hPrecisionStatic = GetDlgItem(hWnd,IDC_PRECISION_STATIC);
	DockWindow(hPrecisionStatic, NULL,hFlipFlopUI, 4, FALSE, FALSE);

	HWND hPrecisionEdit = GetDlgItem(hWnd,IDC_PRECISION2);
	DockWindow(hPrecisionEdit, hPrecisionStatic,NULL, 2, FALSE, FALSE,60,16);

	HWND hPrecisionSpin = GetDlgItem(hWnd,IDC_PRECISION_SPIN2);
	DockWindow(hPrecisionSpin, hPrecisionEdit,NULL, 2, FALSE, FALSE,12,16);

	HWND hFontSizeStatic = GetDlgItem(hWnd,IDC_FONTSIZE_STATIC);
	DockWindow(hFontSizeStatic, NULL,hPrecisionStatic, 8, TRUE, FALSE);

	HWND hFontSizeEdit = GetDlgItem(hWnd,IDC_FONTSIZE2);
	DockWindow(hFontSizeEdit, NULL,hPrecisionEdit, 4, TRUE, FALSE);
	HWND hFontSizeSpin = GetDlgItem(hWnd,IDC_FONTSIZE_SPIN2);
	DockWindow(hFontSizeSpin, NULL,hPrecisionSpin, 4, TRUE, FALSE);


	HWND hShowAttribute = GetDlgItem(hWnd,IDC_ATTRIBUTE_CHECK2);
	DockWindow(hShowAttribute, hShowBones, NULL, 2, FALSE, TRUE,showBonesWidth,14);

	HWND hShowGlobal = GetDlgItem(hWnd,IDC_GLOBAL_CHECK2);
	DockWindow(hShowGlobal, NULL, hShowAttribute, 2, TRUE, FALSE,0,14);

	HWND hReduceLabels = GetDlgItem(hWnd,IDC_REDUCELABELS_CHECK2);
	DockWindow(hReduceLabels, NULL, hShowGlobal, 2, TRUE, FALSE,0,14);

	HWND hShowExclusion = GetDlgItem(hWnd,IDC_SHOWEXCLUSION_CHECK);
	DockWindow(hShowExclusion, NULL, hReduceLabels, 2, TRUE, FALSE,0,14);

	HWND hShowLock = GetDlgItem(hWnd,IDC_SHOWLOCK_CHECK);
	DockWindow(hShowLock, NULL, hShowExclusion, 2, TRUE, FALSE,0,14);

	BOOL showOptions,showSets, showCopyPaste;
	mod->pblock_weighttable->GetValue(skin_wt_showoptionui,0,showOptions, FOREVER);
	mod->pblock_weighttable->GetValue(skin_wt_showsetui,0,showSets, FOREVER);
	mod->pblock_weighttable->GetValue(skin_wt_showcopypasteui,0,showCopyPaste, FOREVER);

	if (!showCopyPaste)
		{
		ShowWindow(hCopy,SW_HIDE);
		ShowWindow(hPaste,SW_HIDE);
		}	
	else
		{
		ShowWindow(hCopy,SW_SHOW);
		ShowWindow(hPaste,SW_SHOW);
		}
	

	if (!showSets)
		{
		ShowWindow(hVertexSet,SW_HIDE);
		ShowWindow(hCreate,SW_HIDE);
		ShowWindow(hDelete,SW_HIDE);
		}	
	else
		{
		ShowWindow(hVertexSet,SW_SHOW);
		ShowWindow(hCreate,SW_SHOW);
		ShowWindow(hDelete,SW_SHOW);
		}	

	if (!showOptions)
		{
		ShowWindow(hShowLock,SW_HIDE);
		ShowWindow(hShowExclusion,SW_HIDE);
		ShowWindow(hReduceLabels,SW_HIDE);
		ShowWindow(hShowGlobal,SW_HIDE);
		ShowWindow(hShowAttribute,SW_HIDE);

		ShowWindow(hFontSizeSpin,SW_HIDE);
		ShowWindow(hFontSizeEdit,SW_HIDE);
		ShowWindow(hFontSizeStatic,SW_HIDE);

		ShowWindow(hPrecisionStatic,SW_HIDE);
		ShowWindow(hPrecisionEdit,SW_HIDE);
		ShowWindow(hPrecisionSpin,SW_HIDE);

		ShowWindow(hFlipFlopUI,SW_HIDE);
		ShowWindow(hUpdateUI,SW_HIDE);
		ShowWindow(hShowBones,SW_HIDE);

		ShowWindow(hOptionStatic,SW_HIDE);

		}	
	else
		{
		ShowWindow(hShowLock,SW_SHOW);
		ShowWindow(hShowExclusion,SW_SHOW);
		ShowWindow(hReduceLabels,SW_SHOW);
		ShowWindow(hShowGlobal,SW_SHOW);
		ShowWindow(hShowAttribute,SW_SHOW);

		ShowWindow(hFontSizeSpin,SW_SHOW);
		ShowWindow(hFontSizeEdit,SW_SHOW);
		ShowWindow(hFontSizeStatic,SW_SHOW);

		ShowWindow(hPrecisionStatic,SW_SHOW);
		ShowWindow(hPrecisionEdit,SW_SHOW);
		ShowWindow(hPrecisionSpin,SW_SHOW);

		ShowWindow(hFlipFlopUI,SW_SHOW);
		ShowWindow(hUpdateUI,SW_SHOW);
		ShowWindow(hShowBones,SW_SHOW);

		ShowWindow(hOptionStatic,SW_SHOW);

		}

	if (GetFlipFlopUI())
		{
//no vertex label on flop UI

		int nameListLabelX = leftBorder;
		int nameListLabelY = GetTopBorder()+5;
		int nameListLabelWidth = buttonWidth+1;
		int nameListLabelHeight = textHeight+1;
		MoveWindow(hNameListLabel,nameListLabelX,nameListLabelY-5,nameListLabelWidth, nameListLabelHeight, TRUE);
		int attribLabelHeight = numAttributes *textHeight;
		
		if (!GetShowAttributes())
			attribLabelHeight = -1;

		DockWindow(hAttribListLabel, NULL ,hNameListLabel , 2, TRUE, FALSE,0,attribLabelHeight);

		int attribBottom = 0;
		if (GetShowAttributes())
			attribBottom = (nameListLabelY + (numAttributes *textHeight)) ;
		else attribBottom = nameListLabelY  ;
		int weightHeight = mainHeight - attribBottom;
		weightHeight -= 60;
//		if (isDocked) weightHeight += 28;
//		if (!GetShowMenu()) weightHeight += 14;
//		weightHeight -= 12;
		DockWindow(hWeightListLabel, NULL ,hAttribListLabel , 2, TRUE, FALSE,0,weightHeight);


		HWND hGlobalDrop = GetDlgItem(hWnd,IDC_NAMELISTGLOBAL_DROP);
//move global drop window
		int nameListGlobalX = leftBorder+buttonWidth+2;
		int nameListGlobalY = GetTopBorder();
		int nameListGlobalWidth = vertNameWidth+1;
		if ((!GetShowGlobals()) || (GetJBMethod()))
			nameListGlobalWidth = 0;


		int nameListGlobalHeight = textHeight;
		MoveWindow(hGlobalDrop,nameListGlobalX,nameListGlobalY-4,nameListGlobalWidth, nameListGlobalHeight, TRUE);
		if (GetShowGlobals() && (!GetJBMethod()))
			ShowWindow(hGlobalDrop,SW_SHOW);
		else ShowWindow(hGlobalDrop,SW_HIDE);
//move global attribs
		DockWindow(hAttribListGlobal, NULL ,hGlobalDrop , 2, TRUE, FALSE,0,attribLabelHeight);
//move global weights
		DockWindow(hWeightListGlobal, NULL ,hAttribListGlobal , 2, TRUE, FALSE,0,weightHeight);


//move name list
		int nameListX = nameListGlobalX+nameListGlobalWidth+1;
		int nameListY = GetTopBorder();
		int nameListHeight = textHeight;
		int nameListWidth = mainWidth - (buttonWidth +nameListGlobalWidth+ (leftBorder*4));
		MoveWindow(hNameList,nameListX,nameListY,nameListWidth, nameListHeight, TRUE);
//move attirb
		DockWindow(hAttribList, NULL ,hNameList , 2, TRUE, FALSE,0,attribLabelHeight);
//move weightlist
		DockWindow(hWeightList, NULL ,hAttribList , 2, TRUE, FALSE,0,weightHeight);

//setup horz scroll pos
		HWND hScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR3);
		DockWindow(hScrollBar, NULL, hWeightList , 2, TRUE, FALSE);
		int range = vertexPtrList.Count()- numberOfColumns;
		SendMessage(hScrollBar, SBM_SETRANGE, 0, (LPARAM)range);

//setup vert scroll pos
		HWND hVScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR1);
		DockWindow(hVScrollBar, hWeightList, NULL , 2, FALSE, TRUE);
		range = activeBones.Count() -  numberOfRows;
		SendMessage(hVScrollBar, SBM_SETRANGE, 0, (LPARAM)range);

		HWND hStatus = GetDlgItem(hWnd,IDC_STATUS);
		DockWindow(hStatus, NULL, hScrollBar , 2, TRUE, FALSE);

		HWND hDropList = GetDlgItem(hWnd,IDC_SETS);
		DockWindow(hDropList,NULL, hWeightListLabel, 2, TRUE, FALSE);

		}
	else
		{
//move name label window
		int nameListLabelX = leftBorder;
		int nameListLabelY = GetTopBorder();
		int nameListLabelWidth = vertNameWidth+1;
		int nameListLabelHeight = textHeight+1;
		
		if ((!GetShowGlobals()) || (GetJBMethod()))
			MoveWindow(hNameListLabel,nameListLabelX,nameListLabelY,nameListLabelWidth, nameListLabelHeight, TRUE);
		else MoveWindow(hNameListLabel,nameListLabelX,nameListLabelY-5,nameListLabelWidth, nameListLabelHeight, TRUE);

//move attrib label
		int attribListLabelX = nameListLabelX+nameListLabelWidth+2;
		int labelHeight = 6;
		if (mod)
			mod->pblock_weighttable->GetValue(skin_wt_attriblabelheight,0,labelHeight,FOREVER);
		int attribListLabelHeight = textHeight *labelHeight +4;
		if (labelHeight == 1)
			attribListLabelHeight = textHeight *labelHeight+1;	

		int attribListLabelY = (nameListLabelY+nameListLabelHeight)- attribListLabelHeight;
		int attribListLabelWidth= numAttributes*textHeight+1;
		if (!GetShowAttributes())
			attribListLabelWidth= 0;
		MoveWindow(hAttribListLabel,attribListLabelX,attribListLabelY,attribListLabelWidth,attribListLabelHeight, TRUE);

//resize weight list label
		int weightListLabelX = attribListLabelX+attribListLabelWidth+2;
		int weightListLabelY = nameListLabelY;
		int weightListLabelHeight = nameListLabelHeight;
		int weightListLabelWidth = mainWidth - (weightListLabelX+(leftBorder*3));
		MoveWindow(hWeightListLabel,weightListLabelX,weightListLabelY,weightListLabelWidth,weightListLabelHeight, TRUE);



		HWND hGlobalDrop = GetDlgItem(hWnd,IDC_NAMELISTGLOBAL_DROP);

		int globalHeight = nameListLabelHeight;
		BOOL showGlobs = GetShowGlobals();
		BOOL jbMeth = GetJBMethod();
		if ((!showGlobs)  || (jbMeth))
			globalHeight = -1;

		DockWindow(hGlobalDrop, NULL, hNameListLabel , 2, TRUE, FALSE,0,globalHeight);
		if (GetShowGlobals() && (!GetJBMethod()))
			ShowWindow(hGlobalDrop,SW_SHOW);
		else ShowWindow(hGlobalDrop,SW_HIDE);

		WINDOWPLACEMENT winGlobalDropPos;
		GetWindowPlacement(hGlobalDrop , &winGlobalDropPos);
		int weightX = (winGlobalDropPos.rcNormalPosition.bottom - winGlobalDropPos.rcNormalPosition.top);
		int weightHeight = nameListLabelY +weightX+16;
		weightHeight = mainHeight - weightHeight;
		weightHeight -= 48;
	//	if (isDocked) weightHeight += 28;
	//	if (!GetShowMenu()) weightHeight += 14;
	//	weightHeight -= 12;

		if ((!GetShowGlobals())  || (GetJBMethod()))
			DockWindow(hNameList, NULL ,hGlobalDrop,  2, TRUE,FALSE,0,weightHeight);
		else DockWindow(hNameList, NULL ,hGlobalDrop,  2, TRUE,FALSE,0,weightHeight);

		
		DockWindow(hAttribListGlobal, NULL ,hAttribListLabel,  2, TRUE,FALSE,0,globalHeight);
		DockWindow(hAttribList,hNameList, hAttribListGlobal ,  2, TRUE,TRUE);

		DockWindow(hWeightListGlobal,hAttribListGlobal, hWeightListLabel ,  2, TRUE,TRUE);
		DockWindow(hWeightList,hAttribList, hWeightListGlobal ,  2, TRUE,TRUE);



//resize and move hscroll bar
		HWND hScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR3);
		DockWindow(hScrollBar, NULL, hWeightList , 2, TRUE, FALSE);
//adjust the range of the scroll bar
//get the number of buttons - number of bones
		int range = activeBones.Count() -  numberOfColumns;
		SendMessage(hScrollBar, SBM_SETRANGE, 0, (LPARAM)range);

//resize and move vscroll bar

		HWND hVScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR1);
		DockWindow(hVScrollBar, hWeightList, NULL , 2, FALSE, TRUE);
		range = vertexPtrList.Count()- numberOfRows;
//do standard sets
		SendMessage(hVScrollBar, SBM_SETRANGE, 0, (LPARAM)range);

		HWND hStatus = GetDlgItem(hWnd,IDC_STATUS);
		DockWindow(hStatus, NULL, hScrollBar , 2, TRUE, FALSE);

		HWND hDropList = GetDlgItem(hWnd,IDC_SETS);
		DockWindow(hDropList,NULL, hNameList, 2, TRUE, FALSE);

		
		}


	ComputeNumberRowsColumns();


	InvalidateViews();

	}

void WeightTableWindow::InvalidateViews()
	{
//if the UI gets to flashy we can break the invalidate into certain regions
	if (hWeightList) 
		{
		InvalidateRect(hWeightList,NULL,TRUE);
		}
	if (hNameList) 
		{
		InvalidateRect(hNameList,NULL,TRUE);
		}
	if (hAttribList) 
		{
		InvalidateRect(hAttribList,NULL,TRUE);
		}
	if (hWeightListGlobal) 
		{
		InvalidateRect(hWeightListGlobal,NULL,TRUE);
		}
//	if (hNameListGlobal) 
//		{
//		InvalidateRect(hNameListGlobal,NULL,TRUE);
//		}
	if (hAttribListGlobal) 
		{
		InvalidateRect(hAttribListGlobal,NULL,TRUE);
		}

	if (hWeightListLabel) 
		{
		InvalidateRect(hWeightListLabel,NULL,TRUE);
		}
	if (hNameListLabel) 
		{
		InvalidateRect(hNameListLabel,NULL,TRUE);
		}
	if (hAttribListLabel) 
		{
		InvalidateRect(hAttribListLabel,NULL,TRUE);
		}

	}


void WeightTableWindow::AddCustomList(TSTR name)
	{

	//hold our customlist
	if (!theHold.Holding())
		{
		theHold.Begin();
		theHold.Put(new CustomListRestore(this));
		theHold.Accept(GetString(IDS_PW_CUSTOMLIST));
		}

//create a new custom data
	CustomListClass *currentList = new CustomListClass();
	currentList->name.printf("%s",name);
//loop through all nodes

	
	BOOL used = FALSE;

	for (int h =0; h< modDataList.Count(); h++)
		{
//loop through the current list set
		INode *matchNode = modDataList[h].node;
		CustomListDataClass *cData = NULL;	
		for (int i =0; i< vertexPtrList.Count(); i++)
			{
//add the data if node matches
			if (vertexPtrList[i].node == matchNode)
				{
//check to make sure we created it, if not create one
				if (cData == NULL)
					{
					cData = new CustomListDataClass();
					cData->node = matchNode;
					used = TRUE;
					}	
//append it to the custom list 
				cData->usedList.Append(1,&vertexPtrList[i].index);
				}
			}
		if (cData)
			{
			currentList->data.Append(1,&cData);
			}

		}
//no data was added so delete and done
	if (!used)
		 delete currentList;
	else
//need to update the custom list drop list
		{
		//get window
		HWND hSets = GetDlgItem(hWnd,IDC_SETS);
//do standard sets
		SendMessage(hSets, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) name.data());
		customLists.Append(1,&currentList);
		}
	}

void WeightTableWindow::DeleteCustomList(int index)
	{
	//hold our customlist
	//hold our customlist
	if (!theHold.Holding())
		{
		theHold.Begin();
		theHold.Put(new CustomListRestore(this));
		theHold.Accept(GetString(IDS_PW_CUSTOMLIST));
		}

//this is the drop list index so we need to take 3 from it 
//since there are 3 static entries
	index -= 3;
//check to make sure it is within our array
	if ((index <0) || (index >= customLists.Count())) return;
//delete entry
	for (int i = 0; i < customLists[index]->data.Count(); i++)
		delete customLists[index]->data[i];
	delete customLists[index];
	customLists.Delete(index,1);

//refresh list
	UpdateWindowControls();
	}


void WeightTableWindow::ScrollRowUpOne()
	{
	HWND hwndScrollBar = GetDlgItem(hWnd, IDC_SCROLLBAR1);
	firstRow--;
	if (firstRow<0) firstRow = 0;
	int nPos = firstRow;
	SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
	BringDownEditField();
	InvalidateViews();
	}
void WeightTableWindow::ScrollRowDownOne()
	{

	HWND hwndScrollBar = GetDlgItem(hWnd, IDC_SCROLLBAR1);
	int lastRow;
	firstRow++;
	lastRow = vertexPtrList.Count() - numberOfRows;
	if (firstRow>lastRow) firstRow = lastRow ;
	int nPos = firstRow;
	SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
	BringDownEditField();
	InvalidateViews();
	}

void WeightTableWindow::ScrollColumnUpOne()
	{
	HWND hwndScrollBar = GetDlgItem(hWnd, IDC_SCROLLBAR3);
	firstColumn--;
	if (firstColumn<0) firstColumn = 0;
	int nPos = firstColumn;
	SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
	BringDownEditField();
	InvalidateViews();
	}
void WeightTableWindow::ScrollColumnDownOne()
	{
	HWND hwndScrollBar = GetDlgItem(hWnd, IDC_SCROLLBAR3);
	int lastColumn;
	firstColumn++;
	lastColumn = vertexPtrList.Count() - numberOfColumns;
	if (firstColumn>lastColumn) firstColumn = lastColumn ;
	int nPos = firstColumn;
	SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
	BringDownEditField();
	InvalidateViews();
	}


void WeightTableWindow::ClearAllSelections()
	{
	for (int i =0; i < vertexPtrList.Count(); i++)
		vertexPtrList[i].Select(FALSE);

	}

void WeightTableWindow::SetAllSelections()
	{
	for (int i =0; i < vertexPtrList.Count(); i++)
		vertexPtrList[i].Select(TRUE);
	}

void WeightTableWindow::InvertSelections()
	{
	for (int i =0; i < vertexPtrList.Count(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			vertexPtrList[i].Select(FALSE);
		else 	vertexPtrList[i].Select(TRUE);

		}

	}



void WeightTableWindow::SelectVerts(int x, int y)
	{
//get affected vert
	int sel =0;
	if (GetFlipFlopUI())
		{
		sel = x/vertNameWidth + firstColumn;
		}
	else
		{
		sel = y/textHeight + firstRow;
		}

//select it based on flag
	if ((sel >= 0) && (sel < vertexPtrList.Count()))
		{
		if (mouseButtonFlags == CTRL_KEY)
			{
			if (vertexPtrList[sel].IsSelected())
				vertexPtrList[sel].Select(FALSE);
			else vertexPtrList[sel].Select(TRUE);
			}
		else if (mouseButtonFlags == ALT_KEY)
			vertexPtrList[sel].Select(FALSE);
		else vertexPtrList[sel].Select(TRUE);

//redraw window
		}

	BringDownEditField();
	InvalidateViews();

	if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	


	}

void WeightTableWindow::SetSelection(BitArray &sel)
	{
	ClearAllSelections();

	int ct = vertexPtrList.Count();
	int selCt = sel.GetSize();
	for (int i = 0; i < selCt; i++)
		{
		if (sel[i])
			{
			if (i < ct)
				vertexPtrList[i].Select(TRUE);
			}
		}

	InvalidateViews();

	}

void WeightTableWindow::GetSelectionSet(BitArray &sel)
	{
	sel.SetSize( vertexPtrList.Count());
	sel.ClearAll();

	for (int i = 0; i < vertexPtrList.Count(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			{
			sel.Set(i);
			}
		}
	}

void WeightTableWindow::SelectVertsRange(int x, int y, int x1, int y1,BitArray &selArray)
	{

	if (vertexPtrList.Count() == 0) return;

	selArray.SetSize( vertexPtrList.Count());
	selArray.ClearAll();
//get affected vert
	int sel =0,sel1;
	if (GetFlipFlopUI())
		{
		sel = x/vertNameWidth + firstColumn;
		sel1 = x1/vertNameWidth + firstColumn;
		}
	else
		{
		sel = y/textHeight + firstRow;
		sel1 = y1/textHeight + firstRow;
		}

	if (sel1 < sel)
		{
		int temp = sel;
		sel = sel1;
		sel1 = temp;
		}
	if (sel < 0) 
		sel = 0;
	if (sel1 < 0) 
		sel1 = 0;
	if (sel >= vertexPtrList.Count()) 
		sel = vertexPtrList.Count()-1;
	if (sel1 >= vertexPtrList.Count()) 
		sel1 = vertexPtrList.Count()-1;
	for (int i = sel; i <= sel1; i++)
		{
		selArray.Set(i);
//		vertexPtrList[i].Select(TRUE);
		}

	BringDownEditField();
//	InvalidateViews();
	}


int WeightTableWindow::GetCurrentVert(int x,int y)
	{
	int currentVert = -1;
	if (GetFlipFlopUI())
		{

		currentVert = (x/vertNameWidth) + firstColumn;
		}
	else
		{
		currentVert = (y/textHeight) + firstRow;

		}
	return currentVert;
	}

int WeightTableWindow::GetCurrentBone(int x,int y)
	{
	int currentBone = -1;
	if (GetFlipFlopUI())
		currentBone = (y/textHeight) + firstRow;
	else currentBone = (x/buttonWidth) + firstColumn;
	if ((currentBone >=0) && (currentBone < activeBones.Count()))
		currentBone = activeBones[currentBone];
	return currentBone;
	}

int WeightTableWindow::GetCurrentAttrib(int x,int y)
	{
	int currentAttrib = -1;
	if (GetFlipFlopUI())
		{
		currentAttrib = (y/textHeight);
		}
	else 
		{
		currentAttrib = (x/textHeight);
		}

	return currentAttrib;
	}

BOOL WeightTableWindow::ToggleExclusion(int x, int y)
	{
	int cellX;
	int cellY;
	float fracX=0.0f,fracY=0.0f;
	if (GetFlipFlopUI())
		{
		fracX = (float)x/(float)vertNameWidth;
		fracX = (fracX - floor(fracX)) * vertNameWidth;
		fracX = vertNameWidth - fracX;

		fracY = (float)y/(float)textHeight;
		fracY = (fracY - floor(fracY)) * textHeight;

		cellX = x/vertNameWidth;
		cellY = y/textHeight;

		}
	else
		{
		fracX = (float)x/(float)buttonWidth;
		fracX = (fracX - floor(fracX)) * buttonWidth;
		fracX = buttonWidth - fracX;

		fracY = (float)y/(float)textHeight;
		fracY = (fracY - floor(fracY)) * textHeight;


		cellX = x/buttonWidth;
		cellY = y/textHeight;
		}
	

	if (!GetShowExclusion()) return FALSE;


	if ((fracY < 9) && (fracX))
		{
//put in exclusions here
		BOOL bFirst = TRUE;
		BOOL bExcluded = FALSE;
		if (mod)
			{
			theHold.Begin();

			int currentVert = GetCurrentVert(x,y);
			int	currentBone = GetCurrentBone(x,y);


			BOOL doSelected = FALSE;

			if (GetJBMethod())
				{
				if (vertexPtrList[currentVert].IsSelected())
					doSelected = TRUE;
				}
			if (doSelected)
				{
//get clicked on vert
				Tab<int> tempTable;

				int exCount = vertexPtrList[currentVert].bmd->exclusionList.Count();
				if  ( (currentBone <  exCount) && (vertexPtrList[currentVert].bmd->exclusionList[currentBone]) )
					bExcluded = vertexPtrList[currentVert].bmd->isExcluded(currentBone, vertexPtrList[currentVert].index);
				

				for (int k = 0; k < modDataList.Count(); k++ )
					{
					BoneModData *bmd = modDataList[k].bmd;

	
					tempTable.ZeroCount();

					for (int j = 0; j < vertexPtrList.Count(); j++ )
						{
						if (vertexPtrList[j].IsSelected())
							{
							if (bmd == vertexPtrList[j].bmd)
								{
								tempTable.Append(1,&vertexPtrList[j].index);
								}
							}
						}
					if (tempTable.Count() >0)
						{
						theHold.Put(new ExclusionListRestore(mod,vertexPtrList[currentVert].bmd,currentBone));  
					
						if (!bExcluded)
							vertexPtrList[currentVert].bmd->ExcludeVerts(currentBone,tempTable);
						else vertexPtrList[currentVert].bmd->IncludeVerts(currentBone,tempTable);
						}
					}

				

				}
			else
				{
				Tab<int> tempTable;
				tempTable.SetCount(1);
				tempTable[0] = vertexPtrList[currentVert].index;

				int exCount = vertexPtrList[currentVert].bmd->exclusionList.Count();
				if  ( (currentBone <  exCount) && (vertexPtrList[currentVert].bmd->exclusionList[currentBone]) )
					bExcluded = vertexPtrList[currentVert].bmd->isExcluded(currentBone, vertexPtrList[currentVert].index);
				
				theHold.Put(new ExclusionListRestore(mod,vertexPtrList[currentVert].bmd,currentBone));  
					
				if (!bExcluded)
					vertexPtrList[currentVert].bmd->ExcludeVerts(currentBone,tempTable);
				else vertexPtrList[currentVert].bmd->IncludeVerts(currentBone,tempTable);
				}
			InvalidateViews();

/*
			for (int k =0; k < modDataList.Count(); k++)
				{
				BoneModData *bmd = modDataList[k].bmd;
				Tab<int> tempTable;
				for ( int i = 0; i < vertexPtrList.Count(); i++ ) 
					{
					if (vertexPtrList[i].bmd == bmd)
						{
						if (vertexPtrList[i].IsSelected())
							{
							tempTable.Append(1,&vertexPtrList[i].index);
							if (bFirst)
								{
								bFirst = FALSE;
									
								int exCount = vertexPtrList[i].bmd->exclusionList.Count();

								if  ( (currentBone <  exCount) && (vertexPtrList[i].bmd->exclusionList[currentBone]) )
									bExcluded = vertexPtrList[i].bmd->isExcluded(currentBone, vertexPtrList[i].index);
								}
							}

						}
					}
				if (tempTable.Count() > 0)
					{
					theHold.Put(new ExclusionListRestore(mod,bmd,currentBone));  
					
					if (bExcluded)
						bmd->ExcludeVerts(currentBone,tempTable);
					else bmd->IncludeVerts(mod->ModeBoneIndex,tempTable);
					}

				}
*/

			theHold.Accept(GetString(IDS_PW_EXCLUSION));


			mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
			}

		return TRUE;
		}

	
	return FALSE;
	}

void WeightTableWindow::BringUpEditField(int x, int y)
	{
//check is field is up if not create it
//	hWeight = GetDlgItem(hWnd,IDC_WEIGHT);



	WINDOWPLACEMENT winWPos;
	GetWindowPlacement(hWeightList , &winWPos);


//move it into the cell

	int cellX;
	int cellY;
	if (GetFlipFlopUI())
		{
		cellX = x/vertNameWidth;
		cellY = y/textHeight;

		}
	else
		{
		cellX = x/buttonWidth;
		cellY = y/textHeight;
		}
	BOOL valid = TRUE;
	if (cellX < 0 ) valid = FALSE;
	if (cellY < 0 ) valid = FALSE;
	if (GetFlipFlopUI())
		{
		if ((cellY +firstRow) >= activeBones.Count()) valid = FALSE;
		if ((cellX +firstColumn)>= vertexPtrList.Count()) valid = FALSE;

		}
	else
		{
		if ((cellY +firstRow) >= vertexPtrList.Count()) valid = FALSE;
		if ((cellX +firstColumn)>= activeBones.Count()) valid = FALSE;

		}

	if ( (y > 0) && valid)
		{

		int xPos= 0;
		int yPos= 0;

		if (GetFlipFlopUI())
			{
			xPos = cellX * vertNameWidth;//+winWPos.rcNormalPosition.left;
			yPos = cellY * textHeight;//+winWPos.rcNormalPosition.top;

			MoveWindow(hWeight,xPos,yPos,vertNameWidth,textHeight, TRUE);

			}
		else
			{
			xPos = cellX * buttonWidth;//+winWPos.rcNormalPosition.left;
			yPos = cellY * textHeight;//+winWPos.rcNormalPosition.top;

			MoveWindow(hWeight,xPos,yPos,buttonWidth,textHeight, TRUE);
			}

//undhide it
		ShowWindow( hWeight, SW_SHOW);
		BringWindowToTop(hWeight);
		InvalidateRect(hWeight, NULL,TRUE);
  
		UpdateWindow( hWeight);

		if (iWeight == NULL)
			{
			iWeight = SetupFloatSpinner(hWeightList, IDC_WEIGHT_SPIN, IDC_WEIGHT, 0.0f, 1.0f, 0.0f, .01f);

			}

//set it value
		int currentVert = GetCurrentVert(x,y);
		int	currentBone = GetCurrentBone(x,y);

		iWeight->SetValue(0.0f, TRUE);

		if ((currentVert >= 0) && (currentBone >= 0))
			{
			for (int i = 0; i < vertexPtrList[currentVert].vertexData->d.Count(); i++)
				{
				if (vertexPtrList[currentVert].vertexData->d[i].Bones == currentBone)
					{
					float value;
					if (vertexPtrList[currentVert].vertexData->IsModified())
						value = vertexPtrList[currentVert].vertexData->d[i].Influences;
					else value = vertexPtrList[currentVert].vertexData->d[i].normalizedInfluences;
					iWeight->SetValue(value, TRUE);
					i = vertexPtrList[currentVert].vertexData->d.Count();
					}
				}
			}
		SetFocus(hWeight);


		}
	}


void WeightTableWindow::BringUpGlobalEditField(int x, int y)
	{
//check is field is up if not create it
//	hWeight = GetDlgItem(hWnd,IDC_WEIGHT);



	WINDOWPLACEMENT winWPos;
	GetWindowPlacement(hWeightListGlobal , &winWPos);


//move it into the cell

	int cellX;
	int cellY;
	if (GetFlipFlopUI())
		{
		cellX = x/vertNameWidth;
		cellY = y/textHeight;
		}
	else
		{
		cellX = x/buttonWidth;
		cellY = y/textHeight;
		}
	if (y > 0)
		{

		int xPos= 0;
		int yPos= 0;

		if (GetFlipFlopUI())
			{
			xPos = cellX * vertNameWidth;;
			yPos = cellY * textHeight;;

			MoveWindow(hGlobalWeight,xPos,yPos,vertNameWidth,textHeight, TRUE);

			}
		else
			{
			xPos = cellX * buttonWidth;//+winWPos.rcNormalPosition.left;
			yPos = cellY * textHeight;//+winWPos.rcNormalPosition.top;

			MoveWindow(hGlobalWeight,xPos,yPos,buttonWidth,textHeight, TRUE);
			}

//undhide it

		int	currentBone = GetCurrentBone(x,y);
		if (currentBone >= 0) 
			{
			ShowWindow( hGlobalWeight, SW_SHOW);
			BringWindowToTop(hGlobalWeight);
			InvalidateRect(hGlobalWeight, NULL,TRUE);
  
			UpdateWindow( hGlobalWeight);

			if (iGlobalWeight == NULL)
				{
				iGlobalWeight = SetupFloatSpinner(hWeightListGlobal, IDC_WEIGHT_SPIN, IDC_WEIGHT, 0.0f, 1.0f, 0.0f, .01f);
	
				}

//set it value


			iGlobalWeight->SetValue(0.0f, TRUE);
			}

		}
	}


void WeightTableWindow::BringDownEditField()
	{

	if (iWeight)
		{
		ShowWindow( hWeight, SW_HIDE);
		InvalidateRect(hWeight, NULL,TRUE);
		ReleaseISpinner(iWeight);
		iWeight = NULL;
		}
	if (iGlobalWeight)
		{
		ShowWindow( hGlobalWeight, SW_HIDE);
		InvalidateRect(hGlobalWeight, NULL,TRUE);
		ReleaseISpinner(iGlobalWeight);
		iGlobalWeight = NULL;
		}
	SetFocus(hWeightList);

	}

void WeightTableWindow::ClearAllWeights(int x,int y)
	{
	int currentBone  = GetCurrentBone(x,y);
	for (int i =0; i < vertexPtrList.Count(); i++)
		{
		int currentVert = i;
		if (GetAffectSelectedOnly())
			{
			if (vertexPtrList[i].IsSelected())
				SetWeight(currentVert,currentBone,0.0f,FALSE);
			}
		else SetWeight(currentVert,currentBone,0.0f,FALSE);
		}
	
	if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
	InvalidateViews();
	}
void WeightTableWindow::DeleteAllWeights(int x,int y)
	{
	int currentBone  = GetCurrentBone(x,y);
	for (int j =0; j < vertexPtrList.Count(); j++)
		{
		VertexListClass *vd = vertexPtrList[j].vertexData;
		for (int i =0; i < vd->d.Count();i++)
			{
			if (vd->d[i].Bones == currentBone)
				{
				if (GetAffectSelectedOnly())
					{
					if (vertexPtrList[j].IsSelected())
						{
						SetWeight(i,currentBone, 0.0f,FALSE);
						vd->d.Delete(i,1);
						vd->Modified(TRUE);
						i = vd->d.Count();

						}
					}
				else
					{
					SetWeight(i,currentBone,0.0f,FALSE);
					vd->d.Delete(i,1);
					vd->Modified(TRUE);
					i = vd->d.Count();
					}
				}
			}
		}
	if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	InvalidateViews();

	}
void WeightTableWindow::MaxAllWeights(int x,int y)
	{
	int currentBone = GetCurrentBone(x,y);
	for (int i =0; i < vertexPtrList.Count(); i++)
		{
		int currentVert = i;
		if (GetAffectSelectedOnly())
			{
			if (vertexPtrList[i].IsSelected())
				SetWeight(currentVert,currentBone,1.0f,FALSE);
			}
		else SetWeight(currentVert,currentBone,1.0f,FALSE);
		}
	if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
	InvalidateViews();
	}


void WeightTableWindow::AddOffsetToAll(int x, int y, float offset)
	{
	int currentBone = GetCurrentBone(x,y);
	for (int i =0; i < vertexPtrList.Count(); i++)
		{
		int currentVert = i;
		float value = initialValue[i] + offset;
		if (GetAffectSelectedOnly())
			{
			if (vertexPtrList[i].IsSelected())
				SetWeight(currentVert,currentBone, value,FALSE);
			}
		else SetWeight(currentVert,currentBone, value,FALSE);
		}
	if (!GetUpdateOnMouseUp())
		{
		if (mod) mod->weightTableWindow.UpdateSpinner();
		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
		}
	InvalidateViews();
	}


void WeightTableWindow::MaxWeight(int x, int y)
	{
	int currentBone = GetCurrentBone(x,y);
	int currentVert = GetCurrentVert(x,y);
	SetWeight(currentVert,currentBone,1.0f);
	InvalidateViews();
	}


void WeightTableWindow::ClearWeight(int x, int y)
	{
	int currentBone = GetCurrentBone(x,y);
	int currentVert = GetCurrentVert(x,y);
	SetWeight(currentVert,currentBone,0.0f);
	InvalidateViews();
	}
void WeightTableWindow::DeleteWeight(int x, int y)
	{
	int currentBone = GetCurrentBone(x,y);
	int currentVert = GetCurrentVert(x,y);
	
	VertexListClass *vd = vertexPtrList[currentVert].vertexData;
	for (int i =0; i < vd->d.Count();i++)
		{
		if (vd->d[i].Bones == currentBone)
			{
			SetWeight(currentVert,currentBone,0.0f);
			vd->d.Delete(i,1);
			vd->Modified(TRUE);
			i = vd->d.Count();
			if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			InvalidateViews();
			}
		}
	UpdateSpinner();
	}

void WeightTableWindow::StoreInitialWeights(int x,int y)
	{
	int currentBone = GetCurrentBone(x,y);
	initialValue.SetCount(vertexPtrList.Count());
	for (int j =0; j < initialValue.Count();j++)
		{
		initialValue[j] = 0.0f;
		VertexListClass *vd = vertexPtrList[j].vertexData;
		for (int i =0; i < vd->d.Count();i++)
			{

			if (vd->d[i].Bones == currentBone)
				{
				if (vd->IsModified())
					initialValue[j] = vd->d[i].Influences;	
				else initialValue[j] = vd->d[i].normalizedInfluences;	
				}
			}
		}
	}

float WeightTableWindow::GetWeightFromCell(int x, int y)
	{
	float v = 0.0f;

	int currentBone = GetCurrentBone(x,y);
	int currentVert = GetCurrentVert(x,y);

	if (currentVert >= vertexPtrList.Count()) return 0.0f;

	if (currentVert >= 0)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		for (int i =0; i < vd->d.Count();i++)
			{
			if (vd->d[i].Bones == currentBone)
				{
				if (vd->IsModified())
					v = vd->d[i].Influences;	
				else v = vd->d[i].normalizedInfluences;	
				}
			}
		}
	return v;
	}

BOOL WeightTableWindow::GetWeightFromEdit(float &v)
	{
	v = 0.0f;
	if (iWeight)
		{
		v = iWeight->GetFVal();
		return TRUE;
		}
	else return FALSE;
	}


BOOL WeightTableWindow::GetGlobalWeightFromEdit(float &v)
	{
	v = 0.0f;
	if (iGlobalWeight)
		{
		v = iGlobalWeight->GetFVal();
		return TRUE;
		}
	else return FALSE;
	}



void WeightTableWindow::ToggleAttribute(int x, int y, BOOL update )
	{
	
	int currentVert = GetCurrentVert(x,y);

	if (currentVert >= vertexPtrList.Count()) return;

	int currentAttrib = GetCurrentAttrib(x,y);
//selected
	if (currentAttrib == 0)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		if (vertexPtrList[currentVert].bmd->selected[vertexPtrList[currentVert].index])
			vertexPtrList[currentVert].bmd->selected.Set(vertexPtrList[currentVert].index,FALSE);
		else vertexPtrList[currentVert].bmd->selected.Set(vertexPtrList[currentVert].index,TRUE);
		}
//modified
	else if (currentAttrib == 1)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		if (vd->IsModified())
			vd->Modified (FALSE);
		else vd->Modified (TRUE);
		vertexPtrList[currentVert].bmd->reevaluate = TRUE;
		}
//normalized
	else if (currentAttrib == 2)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		if (vd->IsUnNormalized())
			vd->UnNormalized (FALSE);
		else vd->UnNormalized (TRUE);
		}
//rigid
	else if (currentAttrib == 3)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		if (vd->IsRigid())
			vd->Rigid (FALSE);
		else vd->Rigid(TRUE);
		vertexPtrList[currentVert].bmd->validVerts.Set(vertexPtrList[currentVert].index,FALSE);
		}
//rigid
	else if (currentAttrib == 4)
		{
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;
		if (vd->IsRigidHandle())
			vd->RigidHandle(FALSE);
		else vd->RigidHandle(TRUE);
		vertexPtrList[currentVert].bmd->validVerts.Set(vertexPtrList[currentVert].index,FALSE);
		}


	if (update)
		{
		UpdateSpinner();
		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
		}

	}

void WeightTableWindow::ToggleGlobalAttribute(int x, int y, BOOL update )
	{
	int currentVert = GetCurrentVert(x,y);
	int currentAttrib = GetCurrentAttrib(x,y);

	BuildGlobalAttribList();
	
	BOOL state = TRUE;
	if (globalAttribList[currentAttrib]== STATE_CHECKED)
		state = FALSE;
	else if (globalAttribList[currentAttrib]== STATE_UNCHECKED)
		state = TRUE;
	for (int i = 0; i < vertexPtrList.Count(); i++)
		{
		BOOL process = TRUE;
		if (GetAffectSelectedOnly())
			{
			if (vertexPtrList[i].IsSelected())
				process = TRUE;
			else process = FALSE;
			}
		if (process)
			{
//selected
			if (currentAttrib == 0)
				{
		
				VertexListClass *vd = vertexPtrList[i].vertexData;
				vertexPtrList[i].bmd->selected.Set(vertexPtrList[i].index,state);
				}
//modified
			else if (currentAttrib == 1)
				{
				VertexListClass *vd = vertexPtrList[i].vertexData;
				vd->Modified (state);
				vertexPtrList[i].bmd->reevaluate = TRUE;
				}
//normalized
			else if (currentAttrib == 2)
				{
				VertexListClass *vd = vertexPtrList[i].vertexData;
				vd->UnNormalized (!state);
//				vertexPtrList[i].bmd->reevaluate = TRUE;
				}
//rigid
			else if (currentAttrib == 3)
				{
				VertexListClass *vd = vertexPtrList[i].vertexData;
				vd->Rigid (state);
				vertexPtrList[i].bmd->validVerts.Set(vertexPtrList[i].index,FALSE);
				vertexPtrList[i].bmd->reevaluate = TRUE;
				}
//rigid Handle
			else if (currentAttrib == 4)
				{
				VertexListClass *vd = vertexPtrList[i].vertexData;
				vd->RigidHandle(state);
				vertexPtrList[i].bmd->validVerts.Set(vertexPtrList[i].index,FALSE);
				vertexPtrList[i].bmd->reevaluate = TRUE;
				}

			}
		}
	if (update)
		{
		UpdateSpinner();
		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
		}

	}


void WeightTableWindow::BuildGlobalAttribList()
	{
	globalAttribList.SetCount(numAttributes);
	for (int i = 0; i < numAttributes; i++)
		globalAttribList[i] = STATE_INDETERMIANT;

	if (vertexPtrList.Count()>0)
		{
//selected
		int startIndex = 0;
		BOOL selectedOnly = FALSE;
		if ((GetJBMethod()) || (GetAffectSelectedOnly()))
			{
			selectedOnly = TRUE;
			startIndex = -1;
			for (int i =0; i < vertexPtrList.Count(); i++)
				{
				if (vertexPtrList[i].IsSelected())
					{
					startIndex = i;
					i = vertexPtrList.Count(); 
					}

				}
			if (startIndex == -1 ) return;
			}

		int index = vertexPtrList[startIndex].index;
		int finalState = STATE_CHECKED;
		BOOL state = vertexPtrList[startIndex].bmd->selected[index];
		for (int i =(startIndex+1); i < vertexPtrList.Count(); i++)
			{
			if ((!selectedOnly) || vertexPtrList[i].IsSelected())
				{
				index = vertexPtrList[i].index;
				BOOL nextState = vertexPtrList[i].bmd->selected[index];
				if (nextState != state)
					finalState = STATE_INDETERMIANT;
				}
			}
		if (finalState != STATE_INDETERMIANT)
			{
			if (state)
				finalState = STATE_CHECKED;
			else finalState = STATE_UNCHECKED;
			}

		globalAttribList[0] = finalState;

//modified	
		index = vertexPtrList[startIndex].index;
		finalState = STATE_CHECKED;
		state = vertexPtrList[startIndex].vertexData->IsModified();
		for (i =(startIndex+1); i < vertexPtrList.Count(); i++)
			{
			if ((!selectedOnly) || vertexPtrList[i].IsSelected())
				{
				index = vertexPtrList[i].index;
				BOOL nextState = vertexPtrList[i].vertexData->IsModified();
				if (nextState != state)
					finalState = STATE_INDETERMIANT;
				}
			}
		if (finalState != STATE_INDETERMIANT)
			{
			if (state)
				finalState = STATE_CHECKED;
			else finalState = STATE_UNCHECKED;
			}
		globalAttribList[1] = finalState;


//normalized	
		index = vertexPtrList[startIndex].index;
		finalState = STATE_CHECKED;
		state = !vertexPtrList[startIndex].vertexData->IsUnNormalized();
		for (i =(startIndex+1); i < vertexPtrList.Count(); i++)
			{
			if ((!selectedOnly) || vertexPtrList[i].IsSelected())
				{
				index = vertexPtrList[i].index;
				BOOL nextState = !vertexPtrList[i].vertexData->IsUnNormalized();
				if (nextState != state)
					finalState = STATE_INDETERMIANT;
				}
			}
		if (finalState != STATE_INDETERMIANT)
			{
			if (state)
				finalState = STATE_CHECKED;
			else finalState = STATE_UNCHECKED;
			}
		globalAttribList[2] = finalState;

//rigid	
		index = vertexPtrList[startIndex].index;
		finalState = STATE_CHECKED;
		state = vertexPtrList[startIndex].vertexData->IsRigid();
		for (i =(startIndex+1); i < vertexPtrList.Count(); i++)
			{
			if ((!selectedOnly) || vertexPtrList[i].IsSelected())
				{
				index = vertexPtrList[i].index;
				BOOL nextState = vertexPtrList[i].vertexData->IsRigid();
				if (nextState != state)
					finalState = STATE_INDETERMIANT;
				}
			}
		if (finalState != STATE_INDETERMIANT)
			{
			if (state)
				finalState = STATE_CHECKED;
			else finalState = STATE_UNCHECKED;
			}
		globalAttribList[3] = finalState;



//rigid	handle
		index = vertexPtrList[startIndex].index;
		finalState = STATE_CHECKED;
		state = vertexPtrList[startIndex].vertexData->IsRigidHandle();
		for (i =(startIndex+1); i < vertexPtrList.Count(); i++)
			{
			if ((!selectedOnly) || vertexPtrList[i].IsSelected())
				{
				index = vertexPtrList[i].index;
				BOOL nextState = vertexPtrList[i].vertexData->IsRigidHandle();
				if (nextState != state)
					finalState = STATE_INDETERMIANT;
				}
			}
		if (finalState != STATE_INDETERMIANT)
			{
			if (state)
				finalState = STATE_CHECKED;
			else finalState = STATE_UNCHECKED;
			}
		globalAttribList[4] = finalState;
		}
		
	}	


void WeightTableWindow::UpdateDeleteButton()
	{
	HWND bHwnd = GetDlgItem(hWnd,IDC_DELETE);
	if (GetActiveSet() >2)
		EnableWindow(bHwnd ,TRUE);
	else EnableWindow(bHwnd ,FALSE);
	}


void WeightTableWindow::UpdatePasteButton()
	{
	HWND bHwnd = GetDlgItem(hWnd,IDC_PASTE);
	if (copyBuffer.Count() ==0)
		EnableWindow(bHwnd ,FALSE);
	else EnableWindow(bHwnd ,TRUE);
	}

void WeightTableWindow::SetCopyBuffer()
	{
//zero out old copy buffer
	FreeCopyBuffer();
//count the number of selected
	int numSelected = 0;
	for (int i =0; i < vertexPtrList.Count(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			numSelected++;
		}
	copyBuffer.SetCount(numSelected);

//copy data into new buffer
	int ct = 0;
	for (i =0; i < vertexPtrList.Count(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			{
			copyBuffer[ct] = new VertexListClass();
			VertexListClass *vd = vertexPtrList[i].vertexData;
//			copyBuffer[ct]->Selected (vd->IsSelected());
			copyBuffer[ct]->Modified (vd->IsModified());
			copyBuffer[ct]->UnNormalized (vd->IsUnNormalized());
			copyBuffer[ct]->Rigid (vd->IsRigid());
			copyBuffer[ct]->RigidHandle (vd->IsRigidHandle());
			copyBuffer[ct]->SelectedTemp (vertexPtrList[i].bmd->selected[vertexPtrList[i].index]);
			copyBuffer[ct]->d = vd->d;
			ct++;
			}
		}

	}
void WeightTableWindow::PasteCopyBuffer()
	{
	int ct = 0;

	theHold.Begin();
	for (int i =0; i < modDataList.Count(); i++)
		{
		BoneModData *bmd = modDataList[i].bmd;
		if (bmd)
			theHold.Put(new WeightRestore(mod,bmd));
		}
	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));

	for (i =0; i < vertexPtrList.Count(); i++)
		{
		if (vertexPtrList[i].IsSelected())
			{
			if (ct >= copyBuffer.Count()) ct = 0;
			if (ct < copyBuffer.Count())
				{
				VertexListClass *vd = vertexPtrList[i].vertexData;
				if (vd->IsModified() != copyBuffer[ct]->IsModified())
					vertexPtrList[i].bmd->reevaluate = TRUE;
//				vd->Selected (copyBuffer[ct]->IsSelected());
//				vd->Modified (copyBuffer[ct]->IsModified());
				vd->Modified (TRUE);
				vd->UnNormalized (copyBuffer[ct]->IsUnNormalized());
				vd->Rigid (copyBuffer[ct]->IsRigid());
				vd->RigidHandle (copyBuffer[ct]->IsRigidHandle());

				if (copyBuffer[ct]->IsSelectedTemp())
					vertexPtrList[i].bmd->selected.Set(vertexPtrList[i].index,TRUE);
				else vertexPtrList[i].bmd->selected.Set(vertexPtrList[i].index,FALSE);

				vd->d = copyBuffer[ct]->d;
				if (!copyBuffer[ct]->IsUnNormalized())
					vd->NormalizeWeights();
				ct++;
				}
			
			}
		}
	if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	

	}
void WeightTableWindow::FreeCopyBuffer()
	{
	for (int i =0; i < copyBuffer.Count(); i++)
		{
		delete copyBuffer[i];
		}
	copyBuffer.ZeroCount();
	copyBuffer.Resize(0);
	}


void  WeightTableWindow::HoldWeights()
	{
	for (int i = 0; i < modDataList.Count(); i++)
		theHold.Put(new WeightRestore(mod,modDataList[i].bmd));
	}

void  WeightTableWindow::HoldSelection()
	{
	for (int i = 0; i < modDataList.Count(); i++)
		theHold.Put(new SelectionRestore(mod,modDataList[i].bmd));
	}

void  WeightTableWindow::SelectBone(int x, int y)
	{
	if (mod == NULL) return;

	int index = GetCurrentBone(x,y);
	int currentBone = 0;

//	if ((index) >= activeBones.Count()) return;

	currentBone = index;

	
	for (int i = 0; i < modDataList.Count(); i++)
		{
		theHold.Begin();
		theHold.Put(new SelectionRestore(mod,modDataList[i].bmd));
		theHold.Accept(GetString(IDS_PW_SELECT));
		}

	if (mod->ip)
		{
		int fsel = mod->ConvertSelectedBoneToListID(currentBone);
		SendMessage(GetDlgItem(mod->hParam,IDC_LIST1),	LB_SETCURSEL ,fsel,0);
		}

	mod->ModeBoneIndex = currentBone;
	mod->Reevaluate(TRUE);
	if (mod->ip)
		mod->UpdatePropInterface();
	mod->updateP = TRUE;

	mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}

#define CUSTOMSET_NUMBER          0x810
#define CUSTOMSET_NAMEDATA			0x820
#define CUSTOMSET_ENTRYDATA		  0x830


IOResult WeightTableWindow::Save(ISave *isave)
	{
	ULONG nb;

//save custom sets
//save number of custom sets 
	int c = customLists.Count();
	
	if (c == 0) return IO_OK;
	
	isave->BeginChunk(CUSTOMSET_NUMBER);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();

	NameTab names;
//save the names in a nametab for ease of use
	for (int i = 0; i < c; i++)
		{	
		TSTR temp(customLists[i]->name);
//		TCHAR *n = customLists[i]->name.data();
//		isave->WriteWString(n);
		names.AddName(temp);
		}

	isave->BeginChunk(CUSTOMSET_NAMEDATA);
	names.Save(isave);
	isave->EndChunk();

//now loop through the lists and write out each list
	isave->BeginChunk(CUSTOMSET_ENTRYDATA);

	for ( i = 0; i < c; i++)
		{
		int ct = customLists[i]->data.Count();
//write number of entries
		isave->Write(&ct,sizeof(ct),&nb);


		for (int j = 0; j < ct; j++)
			{

//save node with a back pointer

			INode *node = customLists[i]->data[j]->node;
			ULONG id = isave->GetRefID(node);
			isave->Write(&id,sizeof(ULONG),&nb);

			int usedCount = customLists[i]->data[j]->usedList.Count();
			isave->Write(&usedCount,sizeof(int),&nb);

//save used list
			int *pUsedList = customLists[i]->data[j]->usedList.Addr(0);
			isave->Write(pUsedList,sizeof(int)*usedCount,&nb);

			}
		}
	isave->EndChunk();

	return IO_OK;

	}

IOResult WeightTableWindow::Load(ILoad *iload)
	{
	IOResult res = IO_OK;
	

	ULONG nb;


	while (IO_OK==(res=iload->OpenChunk())) 
		{
		int id = iload->CurChunkID();
		switch(id)  
			{
			case CUSTOMSET_NUMBER: 
				{
				int ct = 0;
				iload->Read(&ct,sizeof(ct), &nb);
				customLists.SetCount(ct);
				break;
				}
			case CUSTOMSET_NAMEDATA: 
				{
				NameTab names;
				names.Load(iload);
				int c = customLists.Count();
				for (int i = 0; i < c; i++)
					{
					customLists[i] = new CustomListClass();
					customLists[i]->name = names[i];
					}

				break;
				}
			case CUSTOMSET_ENTRYDATA: 
				{
				int c = customLists.Count();
				for (int i = 0; i < c; i++)
					{
					int ct = 0;
					iload->Read(&ct,sizeof(ct), &nb);
					customLists[i]->data.SetCount(ct);

					for (int j = 0; j < ct; j++)
						{
						customLists[i]->data[j] = new CustomListDataClass();
						ULONG id;
						iload->Read(&id,sizeof(ULONG), &nb);
						if (id!=0xffffffff)
							{
							iload->RecordBackpatch(id,(void**)&customLists[i]->data[j]->node);
							}
						int ct = 0;
						iload->Read(&ct,sizeof(ct), &nb);
						customLists[i]->data[j]->usedList.SetCount(ct);

						int *pUsed = customLists[i]->data[j]->usedList.Addr(0);
						iload->Read(pUsed,sizeof(int)*ct, &nb);

						}

		
					}
		

				}


			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}	

	return IO_OK;

/*
//	if (ct == 0) return IO_OK;

	int c = customLists.Count();

	for (int i = 0; i < c; i++)
		{
		TCHAR *name = new TCHAR[255];
//		TSTR name;

		res = iload->ReadWStringChunk(&name);
//		TSTR *newName = new TSTR(name);
//		namedSel[lastID] = newName;		
		TSTR temp(name);
		customLists[i] = new CustomListClass();
		customLists[i]->name = temp;
		delete [] name;
		}

	for ( i = 0; i < c; i++)
		{
		int ct = 0;
		iload->Read(&ct,sizeof(ct), &nb);
		customLists[i]->data.SetCount(ct);

		for (int j = 0; j < ct; j++)
			{
			customLists[i]->data[j] = new CustomListDataClass();
			ULONG id;
			iload->Read(&id,sizeof(ULONG), &nb);
			if (id!=0xffffffff)
				{
				iload->RecordBackpatch(id,(void**)&customLists[i]->data[j]->node);
				}
			int ct = 0;
			iload->Read(&ct,sizeof(ct), &nb);
			customLists[i]->data[j]->usedList.SetCount(ct);

			int *pUsed = customLists[i]->data[j]->usedList.Addr(0);
			iload->Read(pUsed,sizeof(int)*ct, &nb);

			}


		}
		

	return IO_OK;
*/
	}


