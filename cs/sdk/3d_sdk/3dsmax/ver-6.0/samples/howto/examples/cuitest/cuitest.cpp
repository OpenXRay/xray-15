/*===========================================================================*\
 |    File: CUITest.cpp
 |
 | Purpose: To test the CUI Toolbar APIs.  This program adds a toolbar
 |          to MAX when BeginEditParams() is called and hides it when 
 |          EndEditParams() is called.
 |
 |          Important Notes:
 |          o This file requires four image files be placed in the MAX EXE\UI\ICONS 
 |            directory.  These are:
 |             CUITest_16a.bmp
 |             CUITest_16i.bmp
 |             CUITest_24a.bmp
 |             CUITest_24i.bmp
 |
 |          o It also requires CUITest.mcr to be put into EXE\UI\MACROSCRIPTS as well.
 |
 |			NOTE: The above files can be found in the CONTENT directory, off the
 |					root directory of this project.
 |
 |
 | History: Mark Meier, 03/05/99, Began.
 |          MM, 03/14/99, Last Change.
 |			Harry Denholm, 03/28/99, Prep'd for SDK inclusion
 |			Neil Hazzard	07/12/00 R4
 |          Scott Morrison  05/18/01 Removed usage of cached ICUIFrame
 |
\*===========================================================================*/

// Please see the following header file for class information
#include "CUITest.h"
#include "MaxIcon.h"


/*===========================================================================*\
 | Class Descriptor for the CUITest plugin
\*===========================================================================*/

class CUITestClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theCUITest;}
	const TCHAR *	ClassName() {return MY_CLASSNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return MY_CLASS_ID;}
	const TCHAR* 	Category() {return MY_CATEGORY;}
};
static CUITestClassDesc CUITestDesc;
ClassDesc* GetCUITestDesc() {return &CUITestDesc;}



/*===========================================================================*\
 | This class is the custom message handler installed by the method
 | ICUIFrame::InstallMsgHandler(tbMsgHandler).  It has one method,
 | ProcessMessage() which, uh, processes the messages.
\*===========================================================================*/

class TBMsgHandler : public CUIFrameMsgHandler {
	CUITest *ct;
  public:
	TBMsgHandler(CUITest *ctst)	{ this->ct = ctst; }
	int ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
};	
static TBMsgHandler *tbMsgHandler;


int TBMsgHandler::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TB_0: // Dock or Float
		{
            ICUIFrame* pFrame = theCUITest.GetICUIFrame();
            // Don't do anything of it's on the tab panel
            if (pFrame && !pFrame->GetTabbedToolbar()) {
                if (pFrame->IsFloating()) {
                    GetCUIFrameMgr()->DockCUIWindow(GetParent(pFrame->GetContentHandle()), CUI_LEFT_DOCK, NULL);
                    GetCUIFrameMgr()->RecalcLayout(TRUE);
                }
                else {
                    GetCUIFrameMgr()->FloatCUIWindow(GetParent(pFrame->GetContentHandle()), NULL);
                    GetCUIFrameMgr()->RecalcLayout(TRUE);
                }
            }
			return TRUE;
		}
		case ID_TB_1: // This is the check button ... ignore it.
			return TRUE;
		case ID_TB_2:
			MessageBox(NULL, _T("Pressed 2"), _T("ID_TB_2"), MB_OK);
			return TRUE;
		default: // ID not recognized -- use default CUI processing
			return FALSE;
		}
	}
	return FALSE;
}




/*===========================================================================*\
 | The Begin/EndEditParams calls, which create and destroy the toolbar
\*===========================================================================*/

void CUITest::BeginEditParams(Interface *ip,IUtil *iu) {
	this->iu = iu;
	this->ip = ip;

    ICUIFrame* iFrame = GetICUIFrame();
	if (iFrame) {
		// We have the toolbar already, just show it...(EndEditParams() hid it)
		iFrame->Hide(FALSE);
		// If the frame is floating there is no reason to 
		// recalc the layout, but if it's docked we need to do so
		if (!iFrame->IsFloating()) {
			GetCUIFrameMgr()->RecalcLayout(TRUE);
		}
	}
	else {
		// Create a simple toolbar
		// -- First create the frame
		HWND hParent = ip->GetMAXHWnd();
		HWND hWnd = CreateCUIFrameWindow(hParent, _T("CUI Test Toolbar"), 0, 0, 250, 100);
		// iFrame->SetName();
		ICUIFrame* iFrame = ::GetICUIFrame(hWnd);
		iFrame->SetContentType(CUI_TOOLBAR);
		iFrame->SetPosType(CUI_HORIZ_DOCK | CUI_VERT_DOCK | CUI_FLOATABLE | CUI_SM_HANDLES);

		// -- Now create the toolbar window
		HWND hToolbar = CreateWindow(
				CUSTTOOLBARWINDOWCLASS,
				NULL,
				WS_CHILD | WS_VISIBLE,
				0, 0, 250, 100,
				hWnd,
				NULL,
				hInstance,
				NULL);
        hToolbarWnd = hToolbar;

		// -- Now link the toolbar to the CUI frame
		ICustToolbar *iToolBar = GetICustToolbar(hToolbar);
		iToolBar->LinkToCUIFrame(hWnd, NULL);
		iToolBar->SetBottomBorder(FALSE);
		iToolBar->SetTopBorder(FALSE);

		// Install the message handler to process the controls we'll add...
		tbMsgHandler = new TBMsgHandler(this);
		iFrame->InstallMsgHandler(tbMsgHandler);

        // Get the 0th icon from the CUITest icon file.
        // Note that the index is 1-based.
        MaxBmpFileIcon* pIcon = new MaxBmpFileIcon(_T("CUITest"), 0+1);
		// -- Toss in a few controls of various sorts...

		// Add a push button
		// This one docks and undocks the toolbar if clicked...
		iToolBar->AddTool(TBITEM(CTB_PUSHBUTTON, pIcon, ID_TB_0));

		// Add a separator
		iToolBar->AddTool(ToolSeparatorItem(8));

		// Add a check button
		// This one doesn't do anything except change to GREEN when on...
        pIcon = new MaxBmpFileIcon(_T("CUITest"), 1+1);
		iToolBar->AddTool(TBITEM(CTB_CHECKBUTTON, pIcon, ID_TB_1));
		ICustButton* iBtn;
		(iBtn = iToolBar->GetICustButton(ID_TB_1))->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(iBtn);

		// Add a separator
		iToolBar->AddTool(ToolSeparatorItem(8));

		// Add a keyboard macro command.  In this case the Time Configuration dialog
		// is presented as an example.  In order to get the proper command ID to use
		// we must search through the names in the MAX UI action table and find
		// a match.  The list of names can be reviewed in the MAX UI when setting up
		// a keyboard shortcut.
		TSTR findName = _T("Time Configuration");
        ActionTable *pTable = ip->GetActionManager()->FindTable(kActionMainUI);
        assert(pTable);
        for(int i = 0; i < pTable->Count(); i++) {
            ActionItem* pAction = (*pTable)[i];
            TSTR desc;
            pAction->GetDescriptionText(desc);
			if (_tcscmp(findName.data(), desc.data()) == 0) {
                int cmdID = pAction->GetId();
				MacroButtonData md1(kActionMainUI, cmdID, _T("KBD Cmd"), _T("Key Macro Tooltip"));
				iToolBar->AddTool(TBMACRO(&md1));
				break;
			}
		}

		// Add a separator
		iToolBar->AddTool(ToolSeparatorItem(8));

		// Add a macro script button to the toolbar.  This is a custom macro script
		// which is loaded, then located, then added.
		TSTR ui = ip->GetDir(APP_UI_DIR);
		TSTR path = ui + _T("\\CUITEST.MCR");
		GetMacroScriptDir().LoadMacroScripts(path.data(), FALSE);
		// Find it based on the category and name defined in the CUITest.mcr file.
		MacroEntry *me = GetMacroScriptDir().FindMacro(_T("CUITestCategory"), _T("CUITestName"));
		if (me) {
			MacroID mID = me->GetID();
			MacroButtonData md2(mID, _T("Macro Scr"), _T("Macro Script Tooltip"));
			iToolBar->AddTool(TBMACRO(&md2));
		}

		// This button is processed by our message handler (ID_TB_2)
        pIcon = new MaxBmpFileIcon(_T("CUITest"), 2+1);
		iToolBar->AddTool(TBITEM(CTB_PUSHBUTTON, pIcon, ID_TB_2));
		
		// -- Set the initial floating position
		SIZE sz; RECT rect;
		iToolBar->GetFloatingCUIFrameSize(&sz);
		rect.top = 200; rect.left = 200;
		rect.right = rect.left+sz.cx; rect.bottom = rect.top+sz.cy;
		GetCUIFrameMgr()->FloatCUIWindow(hWnd, &rect);
		MoveWindow(hWnd, rect.left, rect.right, sz.cx, sz.cy, TRUE);

		// We are done, release the toolbar and frame handles
		ReleaseICustToolbar(iToolBar);
        iToolBar = NULL;
		ReleaseICUIFrame(iFrame);
	}
}
	
void CUITest::EndEditParams(Interface *ip,IUtil *iu) {
	this->iu = NULL;
	this->ip = NULL;
	// Hide the toolbar so the user can't execute anything 
	// when we're not active.
    ICUIFrame* iFrame = GetICUIFrame();
    if (iFrame) {
        iFrame->Hide(TRUE);
        // If the toolbar was docked we need to recalc the layout
        // so the toolbar will be removed.
        if (!iFrame->IsFloating()) {
            GetCUIFrameMgr()->RecalcLayout(TRUE);
        }
    }
	ReleaseICUIFrame(iFrame);
}

ICUIFrame*
CUITest::GetICUIFrame()
{
    HWND hFrameWnd = GetParent(hToolbarWnd);
    return  ::GetICUIFrame(hFrameWnd);
}
