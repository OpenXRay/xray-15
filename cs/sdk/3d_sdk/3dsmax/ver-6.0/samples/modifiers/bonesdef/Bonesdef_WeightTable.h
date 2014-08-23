																																																/*****************************************************************

  WEIGHTTABLE.H
  Created by: Peter Watje

  This header contains classes used to manage the weight table



******************************************************************/

#ifndef __WEIGHTTABLE__H
#define __WEIGHTTABLE__H


class BonesDefMod;
class BoneModData;
class VertexListClass;

//#define DEBUGMODE 
#define ScriptPrint (the_listener->edit_stream->printf)

//set of spinners that are dynamically created
#define IDC_WEIGHT				0x100
#define IDC_WEIGHT_SPIN			0x101

#define		ALT_KEY		1
#define		CTRL_KEY	2

#define		STATE_CHECKED		1
#define		STATE_UNCHECKED		2
#define		STATE_INDETERMIANT	4



//5.1.01
#define		TEXT_LEFT_JUSTIFIED		0
#define		TEXT_CENTER_JUSTIFIED	1
#define		TEXT_RIGHT_JUSTIFIED	2



/*******************************************************************
class ModData contains a local mod pointer and the node that owns the local mod
pointer.  This is used to the weigth table can have a list of all the local data
and nodes that point to a particular instance of skin
********************************************************************/
class ModData
{
public:
		BoneModData *bmd;
		INode *node;
};


/*****************************************************************
List data for each node
node - node that owns the vertices
usedList - the list of indices that are in the list
******************************************************************/

#define FLAG_SELECTED	1

class CustomListDataClass
{
public:
	INode* node;
	TSTR name;
	Tab<int> usedList;

};

/*****************************************************************
Data entry for a custom list
name - the name of the custom list that will appear in the drop list
data - the data for that list contain which vertices it owns, 
	   one data member per number of nodes that is contributing vertices
******************************************************************/
class CustomListClass
{
public:
	TSTR name;
	Tab<CustomListDataClass*> data;
};

/******************************************************************
This is the struct that contains the info about each vertex 
in the current vertex list
index -  is the index of the vertex into the vertex list of the localmod data
node - is which node owns that vertex
vertexData - is a pointer back to the original vertex weight data in the localmod data
******************************************************************/


class VertexDataClass
{
public:
	int flags;
	int index;

	INode *node;
	BoneModData *bmd;
	VertexListClass* vertexData;

//	TSTR name;
	void GetName(TSTR &vertName)
		{
		vertName.printf("#%d Vertex (%s)",index,node->GetName());

		}

	VertexDataClass()
		{
		flags = 0;
		index = 0;
		}
//	Tab<int> usedList;

	void Select(BOOL sel)
		{
		if (sel)
			flags |= FLAG_SELECTED;
		else flags &= ~FLAG_SELECTED;
		
		}
	BOOL IsSelected()
		{
		return flags&FLAG_SELECTED;
		}

};

/*******************************************************************
class WeightTableEnumProc is a just a class used to walk the dependance
tree gathering all nodes that are dependant on skin
********************************************************************/
class WeightTableEnumProc : public DependentEnumProc 
	{
      public :
      int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};


/*******************************************************************
class WeightTableWindow is the class that manages the weight table
********************************************************************/

class WeightTableAction;
class WeightTableActionCallback;

class WeightTableWindow : public ViewWindow 
{

friend class CustomListRestore;

public:
//ViewWindow methods
		TCHAR *GetName();
		HWND CreateViewWindow(HWND hParent, int x, int y, int w, int h);
		void DestroyViewWindow(HWND hWnd) ;
		BOOL CanCreate();
		int NumberCanCreate() {return 1;}


//size of the text font used 
		int textHeight;
		int vertNameWidth, buttonWidth;

		
	


//various window pointer handles
		HWND toolTipHwnd;
		
		HWND hWnd;			//the windows handle of the weight table floater
		HWND hWeightList;	//the windows handle of the weight list custom control
		HWND hNameList;		//the windows handle of the name list custom control
		HWND hAttribList;	//the windows handle of the name list custom control
		HWND hWeightListGlobal;	//the windows handle of the weight list custom control
		HWND hAttribListGlobal;	//the windows handle of the name list custom control
		HWND hWeightListLabel;	//the windows handle of the weight list custom control
		HWND hNameListLabel;		//the windows handle of the name list custom control
		HWND hAttribListLabel;	//the windows handle of the name list custom control

		HWND hWeight;		//handle to a temp edit field
	

		int firstRow;		//index of first visible active row into vertexPtrList
		int firstColumn;	//index of first visible active column into activeBones
		int numberOfColumns;	
		int numberOfRows;

		//the list of all active bones
		Tab<int> activeBones;
		
		

		//which vertex filter is active
		//	0 = is all vertices
		//  1 = selected vertices only
		//  2 = vertices affected by the current bone
		//  > 3 custom vertex lists
//		int activeSet;
		int GetActiveSet();
		void SetActiveSet(int set);

		//the list of selected vertices
		Tab<int> activeVertices;



		WeightTableWindow();
		~WeightTableWindow();

		//initializes the mod pointer and fills out the modDataList
		//needs to be called once when windows created
		void InitMod(BonesDefMod *mod);
		void BuildModDataList();
		void ClearMod();



		//intializes the whole window
		//needs to be called once when the window is created
		void InitWindowControl();

		//this is called when a bone list is changed,ie a bone is removed or added
		void RecomputeBones();

		//fills out the vertex filter dropdown list, should be called after a new set is created
		void FillOutSets();
		//fills out the vertex list, should be called after a bones, vertex selection change in the base skin mod
		void FillOutVertexList();

		//Fillsout active list of bones which are bones that could possible be displayed
		//this needs to be called any time the bones list is changed 
		void ComputeActiveBones();


		//should be called when the window changes but does not resize, ei a slider first vertex change
		void UpdateWindowControls();

		// a helper function that places a window up against the edge of another 
		void DockWindow( HWND child, HWND left, HWND top, int spacing = 1, BOOL matchWidth = TRUE, BOOL matchHeight = TRUE, int width = 0, int height = 0 );
		//should be called when the window changes by resizing
		void ResizeWindowControls();
		//Saves the windows position state
		void SaveWindowState();
		//Moves the window to the last saved state
		void SetWindowState();

		//should be called when one of the none Custom Controls get updated for instance from code or maxscript or an action item
		void UpdateUI();

		//recomputes the number of rows and columns
		void ComputeFirstBoneAndVertexData();

		//should be called when the window is resized so all the off screen buffers get resized
		void ResizeOffScreenBuffers();
		//this initializes and builds all my offscreen buffers
		void BuildOffScreenBuffers();

		//Paints the vertex weight window
		void PaintNameList();
		void PaintAttribList();
		void PaintWeightList();

		void PaintAttribListGlobal();
		void PaintWeightListGlobal();

		void PaintNameListLabel();
		void PaintAttribListLabel();
		void PaintWeightListLabel();


		void InvalidateViews();


		//Addss new custome vertex set based on the current ste using the name supplied
		void AddCustomList(TSTR name);
		//Deletes the current custom vertex set
		void DeleteCustomList(int index);

		//this frees any temporary list that are created when the ui is up
		//and should be called when the floater goes away
		void CreateGDIData();
		void FreeTempData()
			{
			modDataList.ZeroCount();
			modDataList.Resize(0);

			vertexPtrList.ZeroCount();
			vertexPtrList.Resize(0);

			initialValue.ZeroCount();
			vertexPtrList.Resize(0);

			DestroyIOffScreenBuf(iWeightListBuf); 
			iWeightListBuf   = NULL;
			DestroyIOffScreenBuf(iNameListBuf); 
			iNameListBuf   = NULL;
			DestroyIOffScreenBuf(iAttribListBuf); 
			iAttribListBuf   = NULL;


			DestroyIOffScreenBuf(iWeightListGlobalBuf); 
			iWeightListGlobalBuf   = NULL;
			DestroyIOffScreenBuf(iAttribListGlobalBuf); 
			iAttribListGlobalBuf   = NULL;

			DestroyIOffScreenBuf(iWeightListLabelBuf); 
			iWeightListLabelBuf   = NULL;
			DestroyIOffScreenBuf(iNameListLabelBuf); 
			iNameListLabelBuf   = NULL;
			DestroyIOffScreenBuf(iAttribListLabelBuf); 
			iAttribListLabelBuf   = NULL;


			if (iWeight)
				ReleaseISpinner(iWeight);
			iWeight = NULL;

			FreeCopyBuffer();

			DeleteObject(pTextPen);
			DeleteObject(pSelPen);
			DeleteObject(pBackPen);

			DeleteObject(hFixedFont);
			DeleteObject(hFixedFontBold);

			}

		//frees any new'd data
		void FreeData()
			{
			for (int i = 0; i < customLists.Count();i++)
				{
				for (int j = 0; j < customLists[i]->data.Count();j++)
					delete customLists[i]->data[j];
				customLists[i]->data.ZeroCount();
				CustomListClass *cList = customLists[i];
				delete cList;
				}
			};

		//this is a list of pointers back into the vertex data
		//this is not neccessarily all the vertices
		//the filters control which pointers get put in
		Tab<VertexDataClass> vertexPtrList;

		int mouseButtonFlags;

		void ClearAllSelections();
		void SetAllSelections();
		void InvertSelections();

		void SetSelection(BitArray &sel);

		void SelectVerts(int x, int y);
		void GetSelectionSet(BitArray &sel);
		void SelectVertsRange(int x, int y, int x1, int y1,BitArray &sel);

		int GetCurrentVert(int x,  int y);
		int GetCurrentBone(int x, int y);
		int GetCurrentAttrib(int x, int y);

		BOOL ToggleExclusion(int x, int y);

		void BringUpEditField(int x, int y);
		void BringUpGlobalEditField(int x, int y);
		void BringDownEditField();
		void SetWeight(int currentVert, int currentBone, float value, BOOL update = TRUE);
		void SetAllWeights( int currentBone, float value, BOOL update = TRUE);
		BOOL GetWeightFromEdit(float &v);
		BOOL GetGlobalWeightFromEdit(float &v);
		void ClearWeight(int x, int y);
		void DeleteWeight(int x, int y);
		void MaxWeight(int x, int y);

		void ScrollRowUpOne();
		void ScrollRowDownOne();

		void ScrollColumnUpOne();
		void ScrollColumnDownOne();


		float GetWeightFromCell(int x, int y);

		void ClearAllWeights(int x,int y);
		void DeleteAllWeights(int x,int y);
		void MaxAllWeights(int x,int y);


		void StoreInitialWeights(int x,int y);
		void AddOffsetToAll(int x,int y, float offset);

		void ToggleAttribute(int x, int y,BOOL update = TRUE);
		void BuildGlobalAttribList();

		void ToggleGlobalAttribute(int x,int y,BOOL update = TRUE);

		void SelectBone(int x, int y);

		void SetCopyBuffer();
		void PasteCopyBuffer();
		void FreeCopyBuffer();

		void UpdatePasteButton();
		void UpdateDeleteButton();


		void HoldWeights();
		void HoldSelection();


		void ToggleAffectSelected()
			{
			if (GetAffectSelectedOnly())
				SetAffectSelectedOnly(FALSE);
			else SetAffectSelectedOnly(TRUE);
			}	
		void SetFontSize()
			{
			HDC hdc = iWeightListBuf->GetDC();
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
//get sme intial sizes
			SIZE strSize;
			TSTR hString("Hg");
			GetTextExtentPoint32(hdc,  (LPCTSTR) hString,    hString.Length(),  (LPSIZE) &strSize ); 
			textHeight = strSize.cy+2;
			if (textHeight == 2) textHeight = GetFontSize();
			SelectObject(hdc, hOldFont);
			}

		void ResetFont()
			{
			DeleteObject(hFixedFont);
			hFixedFont = CreateFont(GetFontSize(),0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY, VARIABLE_PITCH  | FF_SWISS, _T(""));
			DeleteObject(hFixedFontBold);
			hFixedFontBold = CreateFont(GetFontSize(),0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY, VARIABLE_PITCH  | FF_SWISS, _T(""));
			SendMessage(hGlobalWeight, WM_SETFONT, (WPARAM)hFixedFont, MAKELONG(0, 0));
			SendMessage(hWeight, WM_SETFONT, (WPARAM)hFixedFont, MAKELONG(0, 0));
			clearBack = TRUE;
			}
		void ResetScrollBars()
			{
			firstRow = 0;
			firstColumn = 0;
			HWND hwndScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR1);
			SetScrollPos( hwndScrollBar,SB_CTL, 0, TRUE);
			hwndScrollBar = GetDlgItem(hWnd,IDC_SCROLLBAR3);
			SetScrollPos( hwndScrollBar,SB_CTL, 0, TRUE);
			}	





		void ComputeNumberRowsColumns();

		float GetPrecision();
		void  SetPrecision(float val);

		int GetFontSize();
		void  SetFontSize(int val);


		
		BOOL GetAffectSelectedOnly();
		void SetAffectSelectedOnly(BOOL val);

		BOOL GetUpdateOnMouseUp();
		void SetUpdateOnMouseUp(BOOL val);

		BOOL GetAffectedBonesOnly();
		void SetAffectedBonesOnly(BOOL val);

		BOOL GetFlipFlopUI();
		void SetFlipFlopUI(BOOL val);

		BOOL GetShowAttributes();
		void SetShowAttributes(BOOL val);

		BOOL GetShowGlobals();
		void SetShowGlobals(BOOL val);

		BOOL GetReduceLabels();
		void SetReduceLabels(BOOL val);

		BOOL GetShowExclusion();
		void SetShowExclusion(BOOL val);

		BOOL GetShowLock();
		void SetShowLock(BOOL val);

		int GetTopBorder();
		void  SetTopBorder(int val);

//		BOOL IsLeftRightDragMode();

		BOOL GetJBMethod();
		void SetJBMethod(BOOL val);

		BOOL GetShowMenu();
		void SetShowMenu(BOOL val);


		BOOL GetShowSetUI();
		void SetShowSetUI(BOOL val);

		BOOL GetShowOptionsUI();
		void SetShowOptionsUI(BOOL val);

		BOOL GetShowCopyPasteUI();
		void SetShowCopyPasteUI(BOOL val);

		BOOL GetDragLeftMode();
		void SetDragLeftMode(BOOL val);

		BOOL GetDebugMode();
		void SetDebugMode(BOOL val);

		BOOL GetShowMarker();
		void SetShowMarker(BOOL val);

//5.1.01
		BOOL GetRightJustify();
		void SetRightJustify(BOOL val);

		int kWeightTableMenuBar;

//action Table methods
		void DeActivateActionTable();
		void ActivateActionTable();
		BOOL WtIsChecked(int id);
		BOOL WtIsEnabled(int id);
		BOOL WtExecute(int id);

		BOOL isDocked;

		int CustomListCount() { return customLists.Count(); }

		int currentStatusID;
		void PrintStatus(int mode);


		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		

		void UpdateSpinner();

private:



		BOOL clearBack;
	

	//spacing vars
	
		int leftBorder;		//is the left border space
//		int topBorder;		//is the top border space for the custom control windows

	
	//Pointer the bones modifier that created the window
		BonesDefMod *mod;
		
		//a table of all the local data/Nodes that are used by mod
		Tab<ModData> modDataList;

		//thisi is a list of all our custom vertex sets that show in the drop down
		//below the 3 base filters
		Tab<CustomListClass*> customLists;

		ISpinnerControl *iWeight;

		ISpinnerControl *iGlobalWeight;
		HWND hGlobalWeight;
		HWND hGlobalSpinner;

		int numAttributes;

		Tab<VertexListClass *> copyBuffer;


//new custom controls
		

		Tab<float> initialValue;
		Tab<int> globalAttribList;
		
				
		//This builds data on the bone bone list and should be called everytime the window is resized

		COLORREF selectionColor;
		COLORREF textColor;
		COLORREF backColor;

		

		HPEN pTextPen;
		HPEN pSelPen;
		HPEN pBackPen;

		HFONT hFixedFont;
		HFONT hFixedFontBold;

		HWND tempSpinner;


//off screen buffers for paint windows
		IOffScreenBuf *iWeightListBuf;
		IOffScreenBuf *iNameListBuf;
		IOffScreenBuf *iAttribListBuf;

		IOffScreenBuf *iWeightListLabelBuf;
		IOffScreenBuf *iNameListLabelBuf;
		IOffScreenBuf *iAttribListLabelBuf;
		
		IOffScreenBuf *iWeightListGlobalBuf;
		IOffScreenBuf *iAttribListGlobalBuf;

//5.01.01
		void PaintCellName(HDC hdc, int x, int y,int width, BOOL sel, int justify, TSTR name);

		void PaintCellWeight(HDC hdc, int x, int y,int width, BOOL sel, BOOL centered, 
							 float value ,BOOL indeterminant, BOOL locked, BOOL excluded);

		void PaintCellNameVertically(HDC hdc, int x, int y, int height,  BOOL sel, TSTR name);
		void PaintCellAttribute(HDC hdc, int x, int y,BOOL sel, int state);


		LONG lwStyle;
		HWND lwParent;
		WINDOWPLACEMENT lwPlacement;
		TSTR viewWindowName;

		void BlackBorder(HDC hdc, HWND hWnd);


		WeightTableActionCallback* pCallback;


		

};

const ActionTableId kWeightTableActions = 0x7ff73cb1;
const ActionContextId kWeightTableContext = 0x7ff73cb1;

class WeightTableActionCallback : public ActionCallback
	{
	public:
	BOOL ExecuteAction(int id)
		{
		if (wTable)
			return wTable->WtExecute(id);
		return FALSE;
		}
	void SetWeightTable(WeightTableWindow *wTable) {this->wTable = wTable;}
	private:
		WeightTableWindow *wTable;
	};

class WeightTableAction : public ActionItem
{
	public:

//ActionItem methods
		BOOL IsChecked()						{if (wTable)
													return wTable->WtIsChecked(id);
												else return FALSE;}
		void GetMenuText(TSTR& menuText)		{menuText.printf("%s",this->menuText);}
		void GetButtonText(TSTR& buttonText)	{buttonText.printf("%s",this->buttonText);	}
		void GetCategoryText(TSTR& catText)		{catText.printf("%s",this->catText);}
		void GetDescriptionText(TSTR& descText) {descText.printf("%s",this->descText);}
		BOOL ExecuteAction()					{if (wTable)
													return wTable->WtExecute(id);
												else return FALSE;
												}
		int GetId()								{return id;}
		BOOL IsItemVisible()					{if (wTable)
													return TRUE;
												else return FALSE;}
		BOOL IsEnabled()					{if (wTable)
													return wTable->WtIsEnabled(id);
												else return FALSE;}
		MaxIcon* GetIcon() 					{return NULL;}
		void DeleteThis() {delete this;}

//WeightTableAction methods
		void Init(int id,TCHAR *mText, TCHAR *bText,TCHAR *cText, TCHAR *dText)
			{
			wTable = NULL;
			this->id = id;
			menuText.printf("%s",mText);	
			buttonText.printf("%s",bText);	
			descText.printf("%s",dText);	
			catText.printf("%s",cText);	
			}
		void SetWeightTable(WeightTableWindow *wTable) {this->wTable = wTable;}
		void SetID(int id) {this->id = id;}
		void SetNames(TCHAR *mText, TCHAR *bText,TCHAR *cText, TCHAR *dText )
			{
			menuText.printf("%s",mText);	
			buttonText.printf("%s",bText);	
			descText.printf("%s",dText);	
			catText.printf("%s",cText);	
			}



	private:
		int id;
		WeightTableWindow *wTable;
		TSTR buttonText, menuText, descText, catText;

};


class CustomListRestore : public RestoreObj {
	public:

		WeightTableWindow *wt;

		Tab<CustomListClass*> ucustomLists;
		Tab<CustomListClass*> rcustomLists;

		CustomListRestore(WeightTableWindow *wt) 
			{
			this->wt = wt;
			//copy pointer to weight table and copy the customlist data to our undo
			int ct = wt->customLists.Count();
			ucustomLists.SetCount(ct);
			for ( int i = 0; i < ct; i++)
				{
				ucustomLists[i] = NULL;
				if (wt->customLists[i])
					{
					ucustomLists[i] = new CustomListClass();
					ucustomLists[i]->name = wt->customLists[i]->name;
					int dataCount = wt->customLists[i]->data.Count();
					ucustomLists[i]->data.SetCount(dataCount);
					for (int j = 0; j < dataCount; j++)
						{
						ucustomLists[i]->data[j] = new CustomListDataClass();
						ucustomLists[i]->data[j]->node = wt->customLists[i]->data[j]->node;
						ucustomLists[i]->data[j]->name = wt->customLists[i]->data[j]->name;
						ucustomLists[i]->data[j]->usedList = wt->customLists[i]->data[j]->usedList;
						}
					}
				}
			}   		
		~CustomListRestore()
			{
			//free our data
			for (int i = 0; i < ucustomLists.Count();i++)
				{
				for (int j = 0; j < ucustomLists[i]->data.Count();j++)
					delete ucustomLists[i]->data[j];
				ucustomLists[i]->data.ZeroCount();
				CustomListClass *cList = ucustomLists[i];
				delete cList;
				}
			for (i = 0; i < rcustomLists.Count();i++)
				{
				for (int j = 0; j < rcustomLists[i]->data.Count();j++)
					delete rcustomLists[i]->data[j];
				rcustomLists[i]->data.ZeroCount();
				CustomListClass *cList = rcustomLists[i];
				delete cList;
				}

			}
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				//copy the customlist data to our redo
				int ct = wt->customLists.Count();
				rcustomLists.SetCount(ct);
				for ( int i = 0; i < ct; i++)
					{
					rcustomLists[i] = NULL;
					if (wt->customLists[i])
						{
						rcustomLists[i] = new CustomListClass();
						rcustomLists[i]->name = wt->customLists[i]->name;
						int dataCount = wt->customLists[i]->data.Count();
						rcustomLists[i]->data.SetCount(dataCount);
						for (int j = 0; j < dataCount; j++)
							{
							rcustomLists[i]->data[j] = new CustomListDataClass();
							rcustomLists[i]->data[j]->node = wt->customLists[i]->data[j]->node;
							rcustomLists[i]->data[j]->name = wt->customLists[i]->data[j]->name;
							rcustomLists[i]->data[j]->usedList = wt->customLists[i]->data[j]->usedList;
							}
						}
					}
				}
			//copy our udata to the custom list
			for (int i = 0; i < wt->customLists.Count();i++)
				{
				for (int j = 0; j < wt->customLists[i]->data.Count();j++)
					delete wt->customLists[i]->data[j];
				wt->customLists[i]->data.ZeroCount();
				CustomListClass *cList = wt->customLists[i];
				delete cList;
				}

			int ct = ucustomLists.Count();
			wt->customLists.SetCount(ct);
			for (i = 0; i < ct; i++)
				{
				wt->customLists[i] = NULL;
				if (ucustomLists[i])
					{
					wt->customLists[i] = new CustomListClass();
					wt->customLists[i]->name = ucustomLists[i]->name;
					int dataCount = ucustomLists[i]->data.Count();
					wt->customLists[i]->data.SetCount(dataCount);
					for (int j = 0; j < dataCount; j++)
						{
						wt->customLists[i]->data[j] = new CustomListDataClass();
						wt->customLists[i]->data[j]->node = ucustomLists[i]->data[j]->node;
						wt->customLists[i]->data[j]->name = ucustomLists[i]->data[j]->name;
						wt->customLists[i]->data[j]->usedList = ucustomLists[i]->data[j]->usedList;
						}
					}
				}
			//update the weight table
			wt->UpdateWindowControls();
			}
		void Redo()
			{
			//copy our rdata to the custom list
			//update the weight table
			//copy our udata to the custom list
			for (int i = 0; i < wt->customLists.Count();i++)
				{
				for (int j = 0; j < wt->customLists[i]->data.Count();j++)
					delete wt->customLists[i]->data[j];
				wt->customLists[i]->data.ZeroCount();
				CustomListClass *cList = wt->customLists[i];
				delete cList;
				}

			int ct = rcustomLists.Count();
			wt->customLists.SetCount(ct);
			for (i = 0; i < ct; i++)
				{
				wt->customLists[i] = NULL;
				if (rcustomLists[i])
					{
					wt->customLists[i] = new CustomListClass();
					wt->customLists[i]->name = rcustomLists[i]->name;
					int dataCount = rcustomLists[i]->data.Count();
					wt->customLists[i]->data.SetCount(dataCount);
					for (int j = 0; j < dataCount; j++)
						{
						wt->customLists[i]->data[j] = new CustomListDataClass();
						wt->customLists[i]->data[j]->node = rcustomLists[i]->data[j]->node;
						wt->customLists[i]->data[j]->name = rcustomLists[i]->data[j]->name;
						wt->customLists[i]->data[j]->usedList = rcustomLists[i]->data[j]->usedList;
						}
					}
				}
			//update the weight table
			wt->UpdateWindowControls();

			}		
		void EndHold() 
			{ 
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }
	};


static int actionCount = 25;
static int actionEdit = 0;
static int actionSets = 5;
static int actionOption = 7;
static int actionOptionEnd = 15;

static int actionIDs[] = {
							IDC_COPY,
							IDC_PASTE,
							IDC_SELECTALL,
							IDC_SELECTNONE,
							IDC_SELECTINVERT,

							IDC_CREATE,
							IDC_DELETE,


							IDC_AFFECTEDBONES_CHECK,
							IDC_UPDATEONMOUSEUP_CHECK2,
							IDC_FLIPFLOPUI_CHECK2,
							IDC_ATTRIBUTE_CHECK2,
							IDC_GLOBAL_CHECK2,
							IDC_REDUCELABELS_CHECK2,
							IDC_SHOWEXCLUSION_CHECK,
							IDC_SHOWLOCK_CHECK,

							IDC_NAMELISTGLOBAL_DROP,

							IDC_JBUIMETHOD,
							IDC_SHOWCOPYPASTEUI,
							IDC_SHOWSETUI,
							IDC_SHOWOPTIONSUI,
							IDC_SHOWMENU,
							IDC_DRAGMODE,
							IDC_SHOWMARKER,
							IDC_DEBUGMODE,
							IDC_RIGHTJUSTIFY    //5.1.01


};

static int actionNames[] = {
							IDS_PW_COPY,
							IDS_PW_PASTE,
							IDS_PW_SELECTALL ,
							IDS_PW_SELECTNONE,
							IDS_PW_SELECTINVERT,

							IDS_PW_CREATE,
							IDS_PW_DELETE,

							IDS_PW_SHOWAFFECTEDBONES,
							IDS_PW_UPDATEONMOUSEUP  ,
							IDS_PW_FLIPUI,
							IDS_PW_SHOWATTRIBUTES,
							IDS_PW_SHOWGLOBAL,
							IDS_PW_SHORTENLABELS,
							IDS_PW_SHOWEXLUSIONS,
							IDS_PW_SHOWLOCKS,

							IDS_PW_AFFECTALLVERTS,

							IDS_PW_JBUIMETHOD,
							IDS_PW_SHOWCOPYPASTEUI,
							IDS_PW_SHOWSETUI,
							IDS_PW_SHOWOPTIONSUI,
							IDS_PW_SHOWMENU,
							IDS_PW_DRAGMODE,
							IDS_PW_SHOWMARKER,
							IDS_PW_DEBUGMODE,
							IDS_PW_RIGHTJUSTIFY			//5.1.01

};


#define		STATUS_RANDOM				0
#define		STATUS_DRAGMODELR			1
#define		STATUS_DRAGMODETB			2
#define		STATUS_DRAGMODEALT			3
#define		STATUS_DRAGMODECTRL			4
#define		STATUS_LEFTCLICK			5
#define		STATUS_RIGHTCLICK			6
#define		STATUS_RIGHTCLICKSHIFT		7
#define		STATUS_RIGHTCLICKCTRL		8
#define		STATUS_MIDDLEMOUSEBUTTON	9
#define		STATUS_MIDDLEMOUSESCROLL	10


static int statusIDs[] = {
					IDS_STATUS_DRAGMODELR,
					IDS_STATUS_DRAGMODETB,
					IDS_STATUS_DRAGMODEALT,
					IDS_STATUS_DRAGMODECTRL,
					IDS_STATUS_LEFTCLICK,
					IDS_STATUS_RIGHTCLICK,
					IDS_STATUS_RIGHTCLICKSHIFT,
					IDS_STATUS_RIGHTCLICKCTRL,
					IDS_STATUS_MIDDLEMOUSEBUTTON,
					IDS_STATUS_MIDDLEMOUSESCROLL,
					};



#endif