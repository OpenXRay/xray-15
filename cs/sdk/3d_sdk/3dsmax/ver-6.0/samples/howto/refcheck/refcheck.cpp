/*===========================================================================*\
 |    File: RefCheck.cpp
 |
 | Purpose: To allow the user to select an item and review the items which
 |          reference it, those that it references, and the messages
 |          generated.
 |
 | History: Mark Meier, Began 06/07/97.
 |          MM, 4/98, Incorporated some of the improvements made by StewS.
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "MAX.H"
#include "resource.h"
#include "utilapi.h"

/*===========================================================================*\
 | Misc defines, typedefs, enums, etc.
\*===========================================================================*/
#define MY_CLASS_ID Class_ID(0x7a0d3c9b, 0x4e841ffd)
#define MY_CLASSNAME _T("Reference Watcher")
#define MY_CATEGORY _T("How To")
#define MY_LIBDESCRIPTION _T("Reference Watcher by Mark Meier")
#define IP GetCOREInterface()

HINSTANCE hInstance;
void FormatObjectDisplay(int i, ReferenceMaker* rm, bool showaddress, TSTR& buf);

/*===========================================================================*\
 | Class RefWatch
\*===========================================================================*/
class RefWatch : public UtilityObj {
	public:
		HWND hMain;
		// This is the reference target we are investigating
		ReferenceTarget *watchMe;
		IUtil* iu;

		// --- Inherited Virtual Methods From UtilityObj ---
		void BeginEditParams(Interface *ip, IUtil *iu);
		void EndEditParams(Interface *ip, IUtil *iu);
		void DeleteThis() {
			// Do not delete the object here if you want the 
			// dialog to remain open when you switch to another
			// panel from the Utility panel
		}

		// --- Methods From RefWatch ---
		void DestroyWindow();
		RefWatch() {hMain = NULL; watchMe = NULL; iu = NULL;}
		TCHAR *GetName();
};
// This is the static instance of the Utility plug-in.
static RefWatch refWatch;

/*===========================================================================*\
 | Class RefCheckDepEnumProc
\*===========================================================================*/
class RefCheckDepEnumProc : public DependentEnumProc {
	HWND hWnd;
	ReferenceTarget* rtarg;
	int *count;
	bool showaddress;
public:
	RefCheckDepEnumProc(HWND hW, ReferenceTarget* rt, int *c, bool addr)
		: hWnd(hW), rtarg(rt), count(c), showaddress(addr)
		{}
	int proc(ReferenceMaker *rmaker);
};

RefCheckDepEnumProc::proc(ReferenceMaker *rmaker) {
	if (rmaker != rtarg)
	{
		TSTR buf;
		FormatObjectDisplay(*count, rmaker, showaddress, buf);
		SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)buf.data());
		*count += 1;
	}
	return 0;
}

/*===========================================================================*\
 | Class RefCheckFindDepEnumProc
\*===========================================================================*/
class RefCheckFindDepEnumProc : public DependentEnumProc {
public:
	int index;
	int count;
	ReferenceTarget* rtarg;
	ReferenceMaker* rmaker;
	RefCheckFindDepEnumProc(int i, ReferenceTarget* rt)
		: index(i), count(0), rtarg(rt), rmaker(NULL)
		{}
	int proc(ReferenceMaker *rm);
};

RefCheckFindDepEnumProc::proc(ReferenceMaker *rm) {
	if (count == index)
	{
		rmaker = rm;
		return 1;
	}
	if (rm != rtarg) ++count;
	return 0;
}

/*===========================================================================*\
 | Class NotifyMgr -- this class was developed by Christer Janson.
\*===========================================================================*/
class NotifyMgr : public ReferenceTarget {
	private:
		int nNumRefs;
		RefResult (*pNotifyFunc)(Interval changeInt,
			RefTargetHandle hTarget, PartID& partID,
				RefMessage message, DWORD pData);
		RefTargetHandle ref0;
		DWORD pPrivateData;
	public:
		// --- Inherited Virtual Methods From Animatable ---
		void GetClassName(TSTR &s) { s = _T("(This Reference Watcher)"); }

		// --- Inherited Virtual Methods From ReferenceMaker ---
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
			PartID& partID,RefMessage message);
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// --- Methods From NotifyMgr ---
		NotifyMgr::NotifyMgr();
		NotifyMgr::~NotifyMgr();
		BOOL CreateReference(RefTargetHandle hTarget);
		BOOL RemoveReference();

		void SetNotifyFunc(RefResult (*func)(Interval changeInt,
		RefTargetHandle hTarget, PartID& partID,RefMessage message, 
			DWORD pData), DWORD pData);
		void ResetNotifyFunc();
};
NotifyMgr notifyObject;

// Constructor
NotifyMgr::NotifyMgr() {
	nNumRefs = 1;
	pNotifyFunc = NULL;
	ref0 = NULL;
}

// Destructor. Remove any references we have made.
NotifyMgr::~NotifyMgr() {
	DeleteAllRefsFromMe();
}

// Notification function.
// When we get a callback we check to see if we have a registered notify
// function. If we do, we forward the notification to the notify function,
RefResult NotifyMgr::NotifyRefChanged(Interval changeInt,
	RefTargetHandle hTarget, PartID& partID,RefMessage message) {
	RefResult res = REF_SUCCEED;

	if (pNotifyFunc) {
		res = pNotifyFunc(changeInt, hTarget, partID, message, pPrivateData);
	}
	return res;
}

// Number of references we have made. Currently we allow one single
// reference only.
int NotifyMgr::NumRefs() {
	return nNumRefs;
}

// Return the n'th reference
RefTargetHandle NotifyMgr::GetReference(int i) {
	switch (i) {
	case 0: return ref0; break;
	}
	return NULL;
}

// Set the n'th reference.
void NotifyMgr::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case 0: ref0 = rtarg; break;
	}
}

// Exported function that let someone make a reference from
// outside of this class.
BOOL NotifyMgr::CreateReference(RefTargetHandle hTarget) {
   	//theHold.Begin();
	MakeRefByID(FOREVER, 0, hTarget);
    //theHold.Accept(_T("Create Reference"));	 
	return TRUE;
}

// Exported function that let someone remove a reference.
BOOL NotifyMgr::RemoveReference() {
	if (ref0) {
		DeleteReference(0);
		ref0 = NULL;
	}
	return TRUE;
}

// Exported function to set the function that will be used as a
// notification callback.
void NotifyMgr::SetNotifyFunc(RefResult (*func)(Interval changeInt,
	RefTargetHandle hTarget, PartID& partID,RefMessage message, 
		DWORD pData), DWORD pData) {
	pPrivateData = pData;
	pNotifyFunc = func;
}

// Exported function to clear the notification callback.
void NotifyMgr::ResetNotifyFunc() {
	RemoveReference();
	pNotifyFunc = NULL;
}

void DisplayRefInfo(RefWatch *rc, HWND hWnd, Tab<TSTR *> *pMessages);
Tab<TSTR *> *GetRefName(RefMessage message, PartID& partID, BOOL bShowpartIDs = TRUE);

/*===========================================================================*\
 | ProcessRefMessage
\*===========================================================================*/
// This is the heart of the reference tracker.  It gets called by the 
// NotifyMgr when it recieves a message via NotifyRefChanged().  It passes
// all the parameter here (plus the additional pData).
RefResult ProcessRefMessage(Interval changeInt, RefTargetHandle hTarget,
	PartID& partID, RefMessage message, DWORD pData) {

	RefWatch* rc = (RefWatch *)pData;
	if (message == REFMSG_TARGET_DELETED) {
		// The item we monitor has been deleted -- we're done...
		DestroyWindow(rc->hMain);
	}
	else {
		LRESULT checkstate = SendMessage(GetDlgItem(rc->hMain, IDC_PARTID),BM_GETCHECK,0,0);
		DisplayRefInfo(rc, rc->hMain, GetRefName(message, partID, (BST_CHECKED==checkstate)));
	}
	return REF_SUCCEED;
}

/*===========================================================================*\
 | Dialog Procs and related functions
\*===========================================================================*/
void FormatObjectDisplay(int i, ReferenceMaker* rm, bool showaddress, TSTR& buf) {
	TSTR str;
	if (rm) {
		rm->GetClassName(str);
		buf.printf(_T("%d: %s"), i, str.data());
		if (str == TSTR("Animatable")) {
			str = buf;
			if (rm->ClassID() == Class_ID(0,0))
				buf.printf(_T("%s (0,0)"), str.data());
			else
				buf.printf(_T("%s (0x%08x, 0x%08x)"), str.data(),
					rm->ClassID().PartA(), rm->ClassID().PartB());
		}
		else if (str == TSTR("Node")) {
			INode* node = (INode*)rm;
			str = buf;
			buf.printf(_T("%s \"%s\""), str.data(), node->GetName());
		}
		if (showaddress) {
			str = buf;
			buf.printf(_T("%s at 0x%08x"), str.data(), rm);
		}
	}
	else
		buf.printf(_T("%d: NULL"), i);
}

void DisplayRefInfo(RefWatch *rc, HWND hWnd, Tab<TSTR *> *pMessages) {
	TSTR buf, str;
	HWND hIRefList = GetDlgItem(hWnd, IDC_IREF_LIST);
	HWND hRefMeList = GetDlgItem(hWnd, IDC_REFME_LIST);
	bool showaddress = IsDlgButtonChecked(hWnd, IDC_ADDRESS) ? true : false;
	bool fulltree = IsDlgButtonChecked(hWnd, IDC_FULLTREE) ? true : false;

	// Add this message to the ref message list
	for (int i = 0 ; i < pMessages->Count() ; i++) {
		SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_ADDSTRING, 0, (LPARAM)(*pMessages)[i]->data());
	}

	// Display the references of this item
	SendMessage(hIRefList, LB_RESETCONTENT, 0, 0L);
	int numRefs = rc->watchMe->NumRefs();
	for (i = 0; i < numRefs; i++) {
		ReferenceTarget *rt = rc->watchMe->GetReference(i);
		FormatObjectDisplay(i, rt, showaddress, buf);
		SendMessage(hIRefList, LB_ADDSTRING, 0, (LPARAM)buf.data());
	}

	// Display the items which reference this target
	int count = 0;
	SendMessage(hRefMeList, LB_RESETCONTENT, 0, 0L);
	if (fulltree) {
		// Enumerate all the dependents
		RefCheckDepEnumProc rfep(hRefMeList, rc->watchMe, &count, showaddress);
		rc->watchMe->EnumDependents(&rfep);
	}
	else {
		// We only want first-level dependents.
		RefList& rl = rc->watchMe->GetRefList();
		RefListItem* ptr = rl.first;
		while (ptr) {
			if (ptr->maker != NULL) {
				FormatObjectDisplay(count, ptr->maker, showaddress, buf);
				SendMessage(hRefMeList, LB_ADDSTRING, 0, (LPARAM)buf.data());
			}
			ptr = ptr->next;
			++count;
		}
	}
}

Tab<TSTR *> *GetRefName(RefMessage message,PartID& partID,BOOL bShowpartIDs) {
	static TSTR *pMsgStr, MsgStr;
	static Tab <TSTR *> pMessages;
	pMsgStr = &MsgStr;
	pMessages.ZeroCount();
	switch (message) {
		case 0x00000010:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new TSTR(_T("   REFMSG_LOOPTEST"));
			break;
		case 0x00000020:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR(_T("   REFMSG_TARGET_DELETED"));		
			break;
		case 0x00000021:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR(_T("   REFMSG_MODAPP_DELETING"));		
			break;
		case 0x00000030:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("   REFMSG_EVAL"));			
			break;
		case 0x00000040:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("   REFMSG_RESET_ORIGIN"));
			break;
		case 0x00000050:
			{
				pMessages.SetCount(pMessages.Count() +1);
				pMessages[pMessages.Count() -1] = new  TSTR(_T("REFMSG_CHANGE  "));
				if(bShowpartIDs)
				{
					if(partID&PART_TOPO)	
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_TOPO"));
					}
					if(partID&PART_GEOM)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_GEOM"));
					}
					if(partID&PART_TEXMAP)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_TEXMAP"));
					}
					if(partID&PART_MTL)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_MTL"));
					}
					if(partID&PART_SELECT)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_SELECT"));
					}
					if(partID&PART_SUBSEL_TYPE)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_SUBSEL_TYPE"));
					}
					if(partID&PART_DISPLAY)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_DISPLAY"));
					}
					if(partID&PART_VERTCOLOR)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_VERTCOLOR"));
					}
					if(partID&PART_GFX_DATA)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_GFX_DATA"));
					}			
					if(partID&PART_TM_CHAN)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_TM_CHAN"));
					}
					if(partID&PART_MTL_CHAN)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_MTL_CHAN"));
					}				
					if(partID&PART_OBJECT_TYPE)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_OBJECT_TYPE"));
					}
					if(partID&PART_HIDESTATE)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_HIDESTATE"));
					}	
				}
			}
		case 0x00000070:
			{
			
				pMessages.SetCount(pMessages.Count() +1);
				pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_FLAGDEPENDENTS"));
				if(bShowpartIDs)
				{
					if(partID&PART_PUT_IN_FG)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_PUT_IN_FG"));
					}				
					if(partID&PART_SHOW_DEPENDENCIES)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new TSTR(_T("   PART_SHOW_DEPENDENCIES"));
					}
					if(partID&PART_HIDESTATE)
					{
						pMessages.SetCount(pMessages.Count() +1);
						pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_SHOWDEP_ON"));
					}	
				}
			}
			break;
		case 0x00000080:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TARGET_SELECTIONCHANGE"));
			break;
		case 0x00000090:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_BEGIN_EDIT"));				
			break;
		case 0x000000A0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_END_EDIT"));
			break;
		case 0x000000B0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_DISABLE"));				
			break;
		case 0x000000C0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_ENABLE"));				
			break;
		case 0x000000D0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TURNON"));				
			break;
		case 0x000000E0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TURNOFF"));	
			break;
		case 0x000000F0:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_LOOKAT_TARGET_DELETED"));
			break;
		case 0x000000F1:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_INVALIDATE_IF_BG"));		
			break;
		case 0x000000F2:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MOD_DISPLAY_ON"));		
			break;
		case 0x000000F3:
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MOD_DISPLAY_OFF"));
			pMessages.SetCount(pMessages.Count() +1);
			break;
		case 0x000000F4:
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MOD_EVAL"));
			pMessages.SetCount(pMessages.Count() +1);
			break;
		case 0x000000F5:
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_IS_OK_TO_CHANGE_TOPOLOGY"));
			pMessages.SetCount(pMessages.Count() +1);
			break;
		case 0x000000F6:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODE_LINK"));		
			break;
		case 0x000000F7:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODE_NAMECHANGE"));			
			break;
		case 0x000000F8:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_OBREF_CHANGE"));		
			break;
		case 0x000000F9:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MODIFIER_ADDED"));		
			break;
		case 0x000000FA:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_CONTROLREF_CHANGE"));		
			break;
		case 0x000000FB:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_GET_PARAM_NAME"));			
			break;
		case 0x000000FC:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_GET_PARAM_DIM"));		
			break;
		case 0x000000FD:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_GET_CONTROL_DIM"));
			break;
		case 0x000000FE:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TM_CHANGE"));			
			break;
		case 0x000000FF:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_RANGE_CHANGE"));		
			break;
		case 0x00000100:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_LINEHEIGHT_CHANGE"));		
			break;
		case 0x00000101:
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_BECOMING_ANIMATED"));
			pMessages.SetCount(pMessages.Count() +1);
			break;
		case 0x00000102:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SUBANIM_STRUCTURE_CHANGED"));
			break;
		case 0x00000103:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_REF_DELETED"));
			break;
		case 0x00000104:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_REF_ADDED"));
			break;
		case 0x00000105:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_BRANCHED_HISTORY_CHANGED"));
			break;
		case 0x00000106:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODEINSELSET_CHANGED"));
			break;
		case 0x00000107:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TEST_DEPENDENCY"));
			break;
		case 0x00000108:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_WANT_SHOWPARAMLEVEL"));
			break;
		case 0x00000109:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_BEFORE_PASTE"));	
			break;
		case 0x0000010A:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NOTIFY_PASTE"));	
			break;
		case 0x0000010B:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_UV_SYM_CHANGE")); 
			break;
		case 0x0000010C:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_GET_NODE_NAME"));		
			break;
		case 0x0000010D:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SEL_NODES_DELETED"));		
			break;
		case 0x0000010E:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_PRENOTIFY_PASTE"));
			break;
		case 0x0000010F:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SHAPE_START_CHANGE"));	
			break;
		case 0x00000110:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SHAPE_END_CHANGE"));	
			break;
		case 0x00000111:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_TEXMAP_REMOVED"));
			break;
		case 0x00000112:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS"));
			break;
		case 0x00000120:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_CONTAINED_SHAPE_POS_CHANGE"));
			break;
		case 0x00000121:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_CONTAINED_SHAPE_SEL_CHANGE"));
			break;
		case 0x00000122:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_CONTAINED_SHAPE_GENERAL_CHANGE"));
			break;
		case 0x00000130:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SELECT_BRANCH"));
			break;
		case 0x00000140:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MOUSE_CYCLE_STARTED"));
			break;
		case 0x00000150:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_MOUSE_CYCLE_COMPLETED"));
			break;

		case 0x00000161:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_CHECK_FOR_INVALID_BIND"));
			break;

		case 0x00000162:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_OBJECT_CACHE_DUMPED"));
			break;

		case 0x00000170:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_SFX_CHANGE"));
			break;

		case 0x00000180:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_OBJXREF_UPDATEMAT"));
			break;

		case 0x00000190:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_OBJXREF_GETNODES"));
			break;

		case 0x00000200:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_OBJECT_REPLACED"));
			break;

		case 0x00000210:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODE_WIRECOLOR_CHANGED"));
			break;

		case 0x00000240:
		{
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODE_RENDERING_PROP_CHANGED"));
			if (bShowpartIDs)
			{
				if (partID&PART_REND_PROP_RENDERABLE)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_RENDERABLE"));
				}				
				if (partID&PART_REND_PROP_CAST_SHADOW)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_CAST_SHADOW"));
				}				
				if (partID&PART_REND_PROP_RCV_SHADOW)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_RCV_SHADOW"));
				}				
				if (partID&PART_REND_PROP_RENDER_OCCLUDED)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_RENDER_OCCLUDED"));
				}				
				if (partID&PART_REND_PROP_VISIBILITY)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_VISIBILITY"));
				}				
				if (partID&PART_REND_PROP_INHERIT_VIS)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_INHERIT_VIS"));
				}				
				if (partID&PART_REND_PROP_PRIMARY_INVISIBILITY)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_PRIMARY_INVISIBILITY"));
				}				
				if (partID&PART_REND_PROP_SECONDARY_INVISIBILITY)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_REND_PROP_SECONDARY_INVISIBILITY"));
				}				
			}
		} break;

		case 0x00000241:
		{
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T("REFMSG_NODE_DISPLAY_PROP_CHANGED"));
			if (bShowpartIDs)
			{
				if (partID&PART_DISP_PROP_IS_HIDDEN)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_DISP_PROP_IS_HIDDEN"));
				}				
				if (partID&PART_DISP_PROP_IS_FROZEN)
				{
					pMessages.SetCount(pMessages.Count() +1);
					pMessages[pMessages.Count() -1] = new  TSTR(_T("   PART_DISP_PROP_IS_FROZEN"));
				}				
			}
		} break;

		default:
			pMessages.SetCount(pMessages.Count() +1);
			pMessages[pMessages.Count() -1] = new  TSTR( _T(""));
			break;
	};
	MsgStr = TSTR( _T(""));
	return &pMessages;
}

static bool OkToDisplay(HWND hWnd, ReferenceMaker* rmaker)
{
	// Don't let them click into "this" or into and class with a ClassID of 0,0.
	if (rmaker == &notifyObject)
		return false;
	else if (rmaker->ClassID() == Class_ID(0,0)  &&  rmaker->SuperClassID() == 0) {
		TSTR name;
		rmaker->GetClassName(name);
		// If they see a real class name, give them the option.
		if (name != TSTR(_T("Animatable"))) {
			int ret = MessageBox(hWnd, 
				"RefWalker will probably fail if you try to examine this object."
				"\nAre you sure you want to do this?",
				"RefWatcher",
				MB_YESNO | MB_ICONEXCLAMATION);
			if (ret == IDYES)
				return true;
		}
		return false;
	}
	return true;
}

INT_PTR CALLBACK MainDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	TSTR buf;
	RefWatch *rc = (RefWatch *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!rc && msg != WM_INITDIALOG ) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			rc = (RefWatch *)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)rc);
			CenterWindow(hWnd, IP->GetMAXHWnd());
			SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), _T(""));
			SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), _T(""));
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NAME), _T(""));
			SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
			return TRUE;
		case WM_CLOSE:
			DestroyWindow(rc->hMain);
			break;
		case WM_DESTROY:
			rc->DestroyWindow();
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLEAR:
					SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
					break;
				case IDC_TVPICK: {
					TSTR str;
					HWND hIRefList = GetDlgItem(hWnd, IDC_IREF_LIST);
					HWND hRefMeList = GetDlgItem(hWnd, IDC_REFME_LIST);
					TrackViewPick tvp;
					tvp.anim = tvp.client = NULL;
					BOOL ok = IP->TrackViewPickDlg(hWnd, &tvp);
					SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), _T(""));
					if (ok) {
						if (tvp.anim && tvp.client) {
							rc->watchMe = tvp.anim;
							buf.printf(_T("Item Name: %s"), rc->GetName());
							SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NAME), buf.data());
							notifyObject.RemoveReference();
							notifyObject.CreateReference(rc->watchMe);
							tvp.anim->GetClassName(str);
							buf.printf(_T("This Target ClassName: %s"), str);
							SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), buf.data());

							tvp.client->GetClassName(str);
							buf.printf(_T("Client ClassName: %s"), str);
							SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), buf.data());
							Tab<TSTR *> ptStr;
							TSTR *pTstr, Str("");
							pTstr = &Str;
							ptStr.Append(1, &pTstr);
							DisplayRefInfo(rc, hWnd, &ptStr);
							SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
						}
						else 
							ok = FALSE;
					}
					if (!ok) {
						rc->watchMe = NULL;
						notifyObject.RemoveReference();
						SendMessage(hIRefList, LB_RESETCONTENT, 0, 0L);
						SendMessage(hRefMeList, LB_RESETCONTENT, 0, 0L);
						SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
						SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), _T(""));
						SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), _T(""));
						SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NAME), _T(""));
					}
					break;
				};
				return TRUE;

				case IDC_REFME_LIST:
				case IDC_IREF_LIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						TSTR str;
						HWND hIRefList = GetDlgItem(hWnd, IDC_IREF_LIST);
						HWND hRefMeList = GetDlgItem(hWnd, IDC_REFME_LIST);
						BOOL ok = TRUE;

						// Get list item.
						HWND listbox = (HWND)lParam;
						int i = SendMessage(listbox, LB_GETCURSEL, 0, 0);

						// Change the currently watched object.
						if (LOWORD(wParam) == IDC_IREF_LIST)
							rc->watchMe = rc->watchMe->GetReference(i);
						else
						{
							bool fulltree = IsDlgButtonChecked(hWnd, IDC_FULLTREE) ? true : false;
							if (fulltree)
							{
								// Enum dependents.
								RefCheckFindDepEnumProc rfep(i, rc->watchMe);
								rc->watchMe->EnumDependents(&rfep);
								if (rfep.rmaker != NULL)
								{
									if (!OkToDisplay(hWnd, rfep.rmaker))
										return TRUE;
									rc->watchMe = (ReferenceTarget*)rfep.rmaker;
								}
								else
									ok = FALSE;
							}
							else
							{
								// We only displayed first-level dependents.
								int count = 0;
								RefList& rl = rc->watchMe->GetRefList();
								RefListItem* ptr = rl.first;
								while (ptr)
								{
									if (count == i)
									{
										if (!OkToDisplay(hWnd, ptr->maker))
											return TRUE;
										rc->watchMe = (ReferenceTarget*)ptr->maker;
										break;
									}
									if (count > i) return TRUE;// not sure how this could happen...
									ptr = ptr->next;
									++count;
								}
							}
						}

						SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), _T(""));
						SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), _T(""));
						if (ok) {
							buf.printf(_T("Item Name: %s"), rc->GetName());
							SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NAME), buf.data());
							notifyObject.RemoveReference();
							notifyObject.CreateReference(rc->watchMe);
							rc->watchMe->GetClassName(str);
							buf.printf(_T("This Target ClassName: %s"), str);
							SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), buf.data());

							buf.printf(_T("Client ClassName: %s"), "(Unable to determine)");
							SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), buf.data());
							Tab<TSTR *> ptStr;
							TSTR *pTstr, Str("");
							pTstr = &Str;
							ptStr.Append(1, &pTstr);
							DisplayRefInfo(rc, hWnd, &ptStr);
							SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
						}
						else {
							rc->watchMe = NULL;
							notifyObject.RemoveReference();
							SendMessage(hIRefList, LB_RESETCONTENT, 0, 0L);
							SendMessage(hRefMeList, LB_RESETCONTENT, 0, 0L);
							SendMessage(GetDlgItem(hWnd, IDC_MSG_LIST), LB_RESETCONTENT, 0, 0L);
							SetWindowText(GetDlgItem(hWnd, IDC_CLASSNAME), _T(""));
							SetWindowText(GetDlgItem(hWnd, IDC_CLIENTNAME), _T(""));
							SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NAME), _T(""));
						}
					}
					break;

			};
			break;

		default:
			return FALSE;
	};
	return TRUE;
}

// --- Inherited Virtual Methods From UtilityObj ---
void RefWatch::BeginEditParams(Interface *ip, IUtil *iutil) {
	if (!hMain) {
		notifyObject.SetNotifyFunc(ProcessRefMessage, (DWORD_PTR)this);
		hMain = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN),
			IP->GetMAXHWnd(), MainDlgProc, (LPARAM)this);
		CheckDlgButton(hMain, IDC_FULLTREE, BST_CHECKED); 
	}
	iu = iutil;
}
	
void RefWatch::EndEditParams(Interface *ip, IUtil *iu) {
	// We are not closing the dialog here, because we want it
	// to survive switches from the Utility Panel to some other panel
	// in order to continue reporting reference messages
}
	
void RefWatch::DestroyWindow()
{
	hMain = NULL;
	notifyObject.ResetNotifyFunc();
	iu = NULL;
}
	
TCHAR *RefWatch::GetName() {
	SClass_ID sID = watchMe->SuperClassID();
	if ((sID == MATERIAL_CLASS_ID) || (sID == TEXMAP_CLASS_ID))
		return ((MtlBase *)watchMe)->GetName();
	else if (sID == BASENODE_CLASS_ID)
		return ((INode *)watchMe)->GetName();
	else if (sID == ATMOSPHERIC_CLASS_ID)
		return ((Atmospheric *)watchMe)->GetName();
	else if ((sID == OSM_CLASS_ID) || (sID == WSM_CLASS_ID) ||
		(sID == GEOMOBJECT_CLASS_ID) || (sID == WSM_OBJECT_CLASS_ID))
		return ((BaseObject *)watchMe)->GetObjectName();
	return _T("(Unknown)");
}

/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/
class RefCheckCDesc : public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &refWatch;}
	const TCHAR *	ClassName() {return MY_CLASSNAME;}
	SClass_ID		SuperClassID() {return SClass_ID(UTILITY_CLASS_ID);}
	Class_ID		ClassID() {return MY_CLASS_ID;}
	const TCHAR * 	Category() {return MY_CATEGORY;}
};
static RefCheckCDesc refCheckCDesc;

/*===========================================================================*\
 | DLL/Lib Functions
\*===========================================================================*/
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {	
	hInstance = hinstDLL;
	if (! controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	return(TRUE);
}

__declspec(dllexport) const TCHAR *LibDescription() {
	return MY_LIBDESCRIPTION;
}

__declspec(dllexport) int LibNumberClasses() { 
	return 1; 
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i) { 
	return &refCheckCDesc; 
}

__declspec(dllexport) ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}

