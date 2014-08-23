/*===========================================================================*\
 |  File: CUITest.h
 |	Header file for the CUITest Utility. Check CUITest.cpp for notes and information.
 |
\*===========================================================================*/


#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include "ActionTable.h" // This is for the keyboard shortcut APIs
#include "imacroscript.h" // This is to use methods of MacroDir and MacroEntry


TCHAR *GetString(int id);
extern ClassDesc* GetCUITestDesc();


/*===========================================================================*\
 | Misc defines, etc.
\*===========================================================================*/

#define MY_CATEGORY _T("How To")
#define MY_CLASSNAME _T("CUITester")
#define MY_CLASS_ID Class_ID(0x227d9df3, 0x999895ed)
#define MY_LIBDESCRIPTION _T("CUI Tester")



/*===========================================================================*\
 | Definitions for the toolbar items themselves
\*===========================================================================*/

#define TBITEM(type, pIcon, cmd) \
		ToolButtonItem(type,pIcon,GetCUIFrameMgr()->GetImageSize(),GetCUIFrameMgr()->GetImageSize(),GetCUIFrameMgr()->GetButtonWidth(),GetCUIFrameMgr()->GetButtonHeight(),cmd,0)
#define TBMACRO(md) \
		ToolMacroItem(0, GetCUIFrameMgr()->GetButtonHeight(), md)

// Here a large value is used so it won't conflict with the IDs used
// by MAX.  If the ProcessMessage() method of the handler returns FALSE
// MAX will use the default CUI toolbar processing and this could invoke
// a MAX command unintentionally.  Note that this is only the case if
// it returns FALSE.
#define ID_TB_0 10000
#define ID_TB_1 10001
#define ID_TB_2 10002



/*===========================================================================*\
 | This is the main plug-in class
\*===========================================================================*/
class CUITest : public UtilityObj {
  public:
    ICUIFrame* GetICUIFrame();

	CUITest() { iu = NULL; ip = NULL; hToolbarWnd = NULL; };
	~CUITest() { };

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void DeleteThis() {}

private:
	IUtil *iu;
	Interface *ip;

	HWND hToolbarWnd; // CUIFrameWindow handle
	
};

static CUITest theCUITest;
