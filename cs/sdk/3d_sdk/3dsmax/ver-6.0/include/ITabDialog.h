/**********************************************************************
 *<
	FILE: ITabDialog.h

	DESCRIPTION: Interface for tabbed dialogs

	CREATED BY:	Cleve Ard

	HISTORY: Created 01 October 2002
		24 March 2003 - Updated for R6. This update allows the
		current renderer to control which tabs are present in the
		render dialog.

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __ITABDIALOG_H_
#define __ITABDIALOG_H_

class ITabbedDialog;
class ITabPage;

class ITabRegister;

class ITabDialogProc;
class ITabPageProc;
class ITabDialogFilter;

class IRollupWindow;

class ITabDialogPluginTab;

/***********************************************************************
 *
 * ITabDialogManager
 *
 * This interface is used to create tabbed dialogs and to register
 * tabs with dialogs that are created by the system or other plugins
 *
 **********************************************************************/
class ITabDialogManager : public FPStaticInterface {
public:

	// Create a tabbed dialog. Parent is the parent window of the dialog,
	// Instance is the instance where the dialog tempate is stored, and dlg
	// is the name of the dialog template resource. If instance is NULL,
	// then dlg is a pointer to the dialog template in memory. MainProc is
	// the object that is used to handle messages for the dialog. Mult is
	// true if the tab control should be multiline and is only applicable
	// when tabID is -1. TabID is the ID of a tab control in the dialog
	// template, that is to be used. If tabID is -1, then a tab control
	// is created. HelpID is used to bring up context sensitive help
	// when the F1 key is pressed.
	virtual ITabbedDialog* CreateTabbedDialog(
		HWND			parent,
		HINSTANCE		instance,
		LPCTSTR			dlg,
		ITabDialogProc*	mainProc,
		bool			multiline,
		DWORD			helpID=0,
		int				tabID = -1,
		const Class_ID&	dialogID = Class_ID(0,0)
	) = 0;
	
	// Register/UnRegister a tab for a dialog, DialogID is the id of the dialog
	// this tab is to be used with. Tab is the address of an object that is used
	// to add the tab when the dialog is created. Tab must remain valid while
	// it is registered.
	virtual bool RegisterTab(const Class_ID& dialogID, ITabDialogPluginTab* tab) = 0;
	virtual void UnRegisterTab(const Class_ID& dialogID, ITabDialogPluginTab* tab) = 0;

	// Set/Get the current plugin tab filter for a dialog. This filter is
	// used to determine which tabs will be present when the dialog is created.
	virtual void SetTabFilter(const Class_ID& dialogID, ITabDialogFilter* filter) = 0;
	virtual ITabDialogFilter* GetTabFilter(const Class_ID& dialogID) = 0;

	// Get the ITabbedDialog for the dialog id. Returns NULL if dialog not
	// present. Assumes that only one dialog is present for a dialog id at a time.
	// Note: NOTIFY_TABBED_DIALOG_CREATED and NOTIFY_TABBED_DIALOG_DELETED sent when 
	// a tabbed dialog is created or deleted. callparam is point to dialogID
	virtual ITabbedDialog* GetTabbedDialog(const Class_ID& dialogID) = 0;

	// The return value can be a combination of TAB_DIALOG_REMOVE_TAB and
	// TAB_DIALOG_ADD_TAB
	static int AcceptTab(
		ITabDialogFilter*		filter,
		ITabDialogPluginTab*	tab
	);
};

// These are some internally defined dialog ids
// Preferences dialog
#define TAB_DIALOG_PREFERENCES_ID	Class_ID(0x16d416fb, 0x1cdd4da5)
// Object Properties dialog
#define TAB_DIALOG_PROPERTIES_ID	Class_ID(0x777f709b, 0x44404554)
// Configure Path dilaog
#define TAB_DIALOG_PATH_ID			Class_ID(0x2c863840, 0x67b1202f)
// Render dialogs
#define TAB_DIALOG_RENDER_ID		Class_ID(0x648636a0, 0xe52086b)
#define TAB_DIALOG_VIDEO_POST_ID	Class_ID(0x41be0fad, 0x160c099c)
// Environment Dialog
#define TAB_DIALOG_ENVIRONMENT_ID	Class_ID(0x447f2d77, 0x781268a7)

// These are the results of accepting a tab for a dialog
enum {
	TAB_DIALOG_REMOVE_TAB = 1,
	TAB_DIALOG_ADD_TAB = 2
};

#define TAB_DIALOG_MANAGER_INTERFACE_ID Interface_ID(0x46465ead, 0x6e3cbb)

// This inline will return the ITabDialogManager.
inline ITabDialogManager* GetTabDialogManager()
{
	return static_cast<ITabDialogManager*>(
		GetCOREInterface(TAB_DIALOG_MANAGER_INTERFACE_ID));
}

/***********************************************************************
 *
 * ITabDialogPluginTab
 *
 * This interface is used to add plugin tabs to a tabbed dialog.
 *
 **********************************************************************/
class ITabDialogPluginTab : public BaseInterface {
public:
	// Add your tab(s) to the dialog. This will only be called if
	// both this object and the dialog agree that the tab should
	// be added. Dialog is the address of the dialog.
	virtual void AddTabToDialog(ITabbedDialog* dialog) = 0;

	// Accept this tab in this dialog. Dialog is the address of the
	// dialog. Return true if the tab should be added. Return false
	// if the tab should not be added or should be removed if it
	// was added previously. Filter is the dialog filter.
	virtual int AcceptDialog(
		ITabDialogFilter*	filter
	) { return TAB_DIALOG_ADD_TAB; }

	// These return various information that is likely to be interesting
	// when deciding whether or not to use this tab.

	// If this tab is the UI for a ReferenceMaker, return it here.
	virtual ReferenceMaker* GetReferenceMaker() { return NULL; }

	// Otherwise, if this tab is the UI for an Animatable, return it here
	virtual Animatable* GetAnimatable() { return GetReferenceMaker(); }

	// Otherwise, If this tab has some known class ID and super class ID
	virtual Class_ID GetClassID() {
		Animatable* a = GetAnimatable();
		return a ? a->ClassID() : Class_ID(0, 0);
	}
	virtual SClass_ID GetSuperClassID() {
		Animatable* a = GetAnimatable();
		return a ? a->SuperClassID() : 0;
	}

	// Otherwise, you will need to define a special interface
	virtual BaseInterface* GetInterface(const Interface_ID& id) { return NULL; }
};

/***********************************************************************
 *
 * TabDialogFilter
 *
 * This interface is used to filter plugin tabs in a tabbed dialog.
 *
 **********************************************************************/
class ITabDialogFilter : public BaseInterface {
public:
	// Return true if the tabs added by the ITabDialogPluginTab tab
	// are acceptable for this dialog. Dialog is the dialog being
	// filtered.
	virtual int AcceptTab(
		ITabDialogPluginTab*	tab
	) { return TAB_DIALOG_ADD_TAB; }

	// Launch the dialog for this filter, open the dialog at a specific page
	virtual int LaunchDialog(const Class_ID& page = Class_ID(0,0)) { return IDOK; }

	// These return various information that is likely to be interesting
	// when deciding whether or not to add a tab.

	// If this dialog is the UI for a ReferenceMaker, return it here.
	virtual ReferenceMaker* GetReferenceMaker() { return NULL; }

	// Otherwise, if this dialog is the UI for an Animatable, return it here
	virtual Animatable* GetAnimatable() { return GetReferenceMaker(); }

	// Otherwise, If this dialog has some known class ID and super class ID
	virtual Class_ID GetClassID() {
		Animatable* a = GetAnimatable();
		return a ? a->ClassID() : Class_ID(0, 0);
	}
	virtual SClass_ID GetSuperClassID() {
		Animatable* a = GetAnimatable();
		return a ? a->SuperClassID() : 0;
	}

	// Otherwise, you will need to define a special interface
	virtual BaseInterface* GetInterface(const Interface_ID& id) { return NULL; }
};

/***********************************************************************
 *
 * TabDialogMessages
 *
 * Messages used by the tab dialog class
 *
 **********************************************************************/
enum TabDialogMessages {
	TABDLG_SWITCH_FROM	= WM_USER + 1,	// Switching from this page
	TABDLG_SWITCH_TO	= WM_USER + 2,	// Switching to this page
	TABDLG_COMMIT		= WM_USER + 3,	// Commit the page's working values for real
	TABDLG_PRECOMMIT	= WM_USER + 4,	// Check your values, but don't close the dialog;
										// if your page needs to reject an OK, set the
										// bool* in LPARAM to false.
	TABDLG_CANCEL		= WM_USER + 5,	// User is cancelling out of the dialog
	TABDLG_CLOSE		= WM_USER + 6,	// User is closing a modeless dialog
	TABDLG_INVALIDATE	= WM_USER + 7,	// The underlying data has changed
	TABDLGNOTIFY_GETINITINFO	= WM_USER + 8,	// WE need to get init, see struct TABDLG_NMHDR
	TABDLG_RESIZE_DIALOG= WM_USER + 9	// The tab is too small for the content
};

// The address of this struct passed in LPARAM when
// you get a WM_NOTIFY, with TABDLGNOTIFY_GETINITINFO
// as the notify code. This notification is sent to
// the tab dialog before each page is created. hdr.idFrom
// is the index of the page about to be created. The
// value stored in dwInitParam is passed to the InitPage
// method as lparam.
struct TABDLG_NMHDR 
{
	NMHDR	hdr;//the common denominator
				//we use the 
	LPARAM dwInitParam; //this out param contains the ptr to initialization data for the page.
};

/***********************************************************************
 *
 * ITabbedDialog
 *
 * This interface is used to manage a tabbed dialog. You can add pages
 * and either display a modal or modeless dialog.
 * You can add a rollout page, that can contain MAX rollouts
 *
 **********************************************************************/

#define TABBED_DIALOG_INTERFACE_ID Interface_ID(0x4128621b, 0x744d5789)

class ITabbedDialog {
public:
	enum {
		kSystemPage = 20,
		kNormalPage = 50,
		kMaxPage = 100
	};
	
	static ITabbedDialog* GetPointer(HWND dialog) {
		return reinterpret_cast<ITabbedDialog*>(GetWindowLongPtr(
			dialog, GWLP_USERDATA));
	}

	// Delete yourself
	virtual void DeleteThis() = 0;

	// Return the dialogID for this dialog
	virtual Class_ID GetDialogID() const = 0;

	// Add a page to the dialog. Text is the tab text. Instance and
	// tmplt are the instance and resource for the dialog template
	// for the page. If instance is NULL, the tmplt is a pointer to
	// the dialog template in memory. Proc is the message handler for
	// the page. Image is an image for the tab in the image list set
	// by SetImageList. Order is the order that the tab will appear
	// in the tabs. Two pages with the same order are ordered
	// by the order they are added. HelpID is ID used for context
	// sensitive help.
	virtual ITabPage* AddPage(
		LPCTSTR					text,
		HINSTANCE				instance,
		LPCTSTR					tmplt,
		ITabPageProc*			proc,
		const Class_ID&			pageID,
		ITabDialogPluginTab*	plugin = NULL,
		DWORD					helpID = 0,
		int						order = kNormalPage,
		int						image = -1
	) = 0;
	
	// Add a rollout page to the dialog. Text is the tab text. Proc is
	// the message handler for the page. The margins give additional
	// space between the rollout control and the client are of the tab.
	// Image is an image for the tab in the image list set by SetImageList.
	// Order is the order that the tab will appear in the tabs. Two
	// pages with the same order are ordered by the order they are added.
	// HelpID is ID used for context sensitive help.
	virtual ITabPage* AddRollout(
		LPCTSTR					text,
		ITabPageProc*			proc,
		const Class_ID&			pageID,
		ITabDialogPluginTab*	plugin = NULL,
		int						controlID = -1,
		int						width = 0,
		int						bottomMargin = 0,
		DWORD					helpID = 0,
		int						order = kNormalPage,
		int						image = -1
	) = 0;

	// Add and remove registered tabs to the dialog.
	virtual void SyncRegisteredTabs() = 0;

	// Get the filter for this dialog
	virtual ITabDialogFilter* GetTabFilter() const = 0;

	// Get/Set the dialog proc. The previous
	// value is returned when setting the proc
	virtual ITabDialogProc* GetProc() const = 0;
	virtual ITabDialogProc* SetProc(ITabDialogProc* newProc) = 0;
	
	// Display and process modal dialog
	virtual INT_PTR DoModal(const Class_ID& page) = 0;
	
	// Display and process modeless dialog
	virtual bool DoFloater(const Class_ID& page) = 0;

	// Switch to the given page.
	virtual void Switch(const Class_ID& page) = 0;

	// Invalidate all pages in the dialog.
	virtual void Invalidate() = 0;
		
	// Set the image list for the tab control
	virtual void SetImageList(HIMAGELIST list) = 0;
		
	// Get the index of the current page
	virtual int CurrentPage() const = 0;

	// Set the margins for the tab in the dialog. This is
	// only applicable if the tabCtrl is not part of the
	// dialog template. If SetTabbedRect is called with
	// a nonempty rectangle, it is used, otherwise the
	// margins are used.
	virtual void SetMargins(int left, int top, int right, int bottom) = 0;
	virtual void SetTabbedRect(const RECT& rect) = 0;
		
	// Convert indices to pages
	virtual int GetNbPages() const = 0;
	virtual ITabPage* GetPage(int index) const = 0;
	virtual ITabPage* GetPage(const Class_ID& id) const = 0;

	// Get the window for the tabbed dialog
	virtual HWND GetHWND() const = 0;

	// Used for modal dialogs with IDOK and IDCANCEL
	virtual bool OkToCommit() = 0;
	virtual bool CommitPages() = 0;

	// Used for modeless dialogs; note that if the user hits the X in the upper right
	// corner, a cancel message will be sent.
	virtual void CloseDialog() = 0;
	virtual void ClosePages() = 0;

	// Used for both.
	virtual void CancelDialog() = 0;
	virtual void CancelPages() =0;

	// Get the window for the tab control
	virtual HWND GetTabHWND() const = 0;

protected:
	// Don't allow it to be deleted except through DeleteThis
	~ITabbedDialog() {}
};

/***********************************************************************
 *
 * TabDialogPointer
 *
 * This smart pointer is used to simplify scope management
 *
 **********************************************************************/
class TabDialogPointer {
public:
	TabDialogPointer() : mpDlg(NULL) { }
	explicit TabDialogPointer(ITabbedDialog* dlg) : mpDlg(dlg) { }
	explicit TabDialogPointer(TabDialogPointer& src) : mpDlg(src.mpDlg) { src.mpDlg = NULL; }
	TabDialogPointer(
		HWND			parent,
		HINSTANCE		instance,
		LPCTSTR			dlg,
		ITabDialogProc*	mainProc,
		bool			multiline,
		DWORD			helpID = 0,
		int				tabID = -1,
		const Class_ID&	dialogID = Class_ID(0,0)
	) : mpDlg(NULL)
	{
		ITabDialogManager* i = GetTabDialogManager();
		if (i != NULL) {
			mpDlg = i->CreateTabbedDialog(parent, instance, dlg, mainProc,
				multiline, helpID, tabID, dialogID);
		}
	}

	~TabDialogPointer() { free(); mpDlg = NULL; }

	TabDialogPointer& operator=(ITabbedDialog* dlg) { free(); mpDlg = dlg; return *this; }
	TabDialogPointer& operator=(TabDialogPointer& src) { free(); mpDlg = src.mpDlg; src.mpDlg = NULL; return *this; }

	operator ITabbedDialog*() { return mpDlg; }
	ITabbedDialog& operator*() { return *mpDlg; }
	ITabbedDialog* get() { return mpDlg; }
	ITabbedDialog* release() { ITabbedDialog* dlg = mpDlg; mpDlg = NULL; return dlg; }
	ITabbedDialog* operator->() { return mpDlg; }
	ITabbedDialog** operator&() { return &mpDlg; }

	bool Create(
		HWND			parent,
		HINSTANCE		instance,
		LPCTSTR			dlg,
		ITabDialogProc*	mainProc,
		bool			multiline,
		int				tabID = -1,
		DWORD			helpID = 0,
		const Class_ID&	dialogID = Class_ID(0,0)
	)
	{
		ITabDialogManager* i = GetTabDialogManager();
		if (i != NULL) {
			ITabbedDialog* retVal = i->CreateTabbedDialog(parent, instance, dlg, mainProc,
				multiline, helpID, tabID, dialogID);
			if (retVal != NULL) {
				free();
				mpDlg = retVal;
				return true;
			}
		}
		return false;
	}

private:
	void free() { if (mpDlg != NULL) mpDlg->DeleteThis(); }

	ITabbedDialog*		mpDlg;
};

/***********************************************************************
 *
 * ITabPage
 *
 * This interface is used to manage a page in a tabbed dialog.
 *
 **********************************************************************/
class ITabPage : public BaseInterface {
public:
	// Get the window for this page
	virtual HWND GetHWND() const = 0;

	// Get the pageID for this page
	virtual Class_ID GetPageID() const = 0;
	
	// Get the help ID for the page
	virtual int GetHelpID() const = 0;
	
	// Get the tab dialog containing this page
	virtual ITabbedDialog* GetTabDialog() const = 0;
	
	// Get/Set the tab dialog page proc. The previous
	// value is returned when setting the proc
	virtual ITabPageProc* GetProc() const = 0;
	virtual ITabPageProc* SetProc(ITabPageProc* newProc) = 0;
	
	// Get the Plugin that added this page
	virtual ITabDialogPluginTab* GetPlugin() const = 0;
	
	// Convert pages to indices
	virtual int GetIndex() const = 0;

	// Refresh your UI.
	virtual void Invalidate() = 0;

	// Get the IRollupWindow interface for the rollout
	// This interface should not be released, because
	// it is release by this object when the rollout
	// is destroyed. NULL is returned if the page is
	// not a rollout.
	virtual IRollupWindow* GetRollout() = 0;
};


/***********************************************************************
 *
 * ITabDialogProc
 *
 * This interface is implemented by the code that wants to use the
 * tab dialog. The implementer can choose whether to respond to
 * the methods or to messages
 *
 **********************************************************************/
class ITabDialogProc : public BaseInterface {
public:
	// Construct
	ITabDialogProc() : mpDlg(NULL) {}
	
	// Set the dialog proc
	void SetTabDialog(ITabbedDialog* dlg) { mpDlg = dlg; }
	ITabbedDialog* GetTabDialog() const { return mpDlg; }

	// Delete this dialog proc. This method does nothing, because most
	// dialog procs are not scoped by their dialog, but you can override
	// it if you want your dialog proc deleted when the dialog is deleted.
	virtual void DeleteThis() {}

	// Initialize the dialog
	INT_PTR InitDialog(HWND focus) {
		return dialogProc(WM_INITDIALOG, reinterpret_cast<WPARAM>(focus), 0);
	}
	
	// Invalidate the dialog
	void Invalidate() {
		dialogProc(TABDLG_INVALIDATE, 0, 0);
	}
		
	// Used for modal dialogs with IDOK and IDCANCEL
	bool OkToCommit() {
		bool commit = true;
		dialogProc(TABDLG_PRECOMMIT, 0, reinterpret_cast<LPARAM>(&commit));
		return commit;
	}
	void Commit() {
		dialogProc(TABDLG_COMMIT, 0, 0);
	}

	// The dialog is being closed. This is called after
	// all of the pages have been committed.
	void Close() {
		dialogProc(TABDLG_CLOSE, 0, 0);
	}

	// The dialog is being canceled
	void Cancel() {
		dialogProc(TABDLG_CANCEL, 0, 0);
	}

	// Resize the dialog. Delta is the required
	// change in size. This method is only called
	// if the size needs to be increased.
	// Return true if the dialog was resized.
	// If the dialog was resize, the dialog manager
	// will resize the tab control and the content.
	// The dialog proc may change the delta values
	// to correspond the the amount the dialog was resized.
	bool ResizeDialog(SIZE* delta) {
		return dialogProc(TABDLG_RESIZE_DIALOG, 0,
			reinterpret_cast<LPARAM>(delta)) != 0;

	};

	// The general message proc. This proc is like a
	// dialog proc, not a window proc
	virtual INT_PTR dialogProc(
		UINT	msg,
		WPARAM	wParam,
		LPARAM	lParam
	) { return FALSE; }

	// Get extension interfaces, if there are any
	virtual BaseInterface* GetInterface(const Interface_ID& id) { return NULL; }
	
private:
	ITabbedDialog*		mpDlg;
};


/***********************************************************************
 *
 * ITabPageProc
 *
 * This interface is used to filter plugin tabs in a tabbed dialog.
 *
 **********************************************************************/
class ITabPageProc {
public:
	// Construct
	ITabPageProc() : mpPage(NULL) {}
	
	// Set the dialog proc
	void SetTabPage(ITabPage* page) { mpPage = page; }
	ITabPage* GetTabPage() const { return mpPage; }

	// Delete this page proc. This method does nothing, because most
	// page procs are not scoped by their page, but you can override
	// it if you want your page proc deleted when the page is deleted.
	virtual void DeleteThis() {}

	// Initialize the dialog
	INT_PTR InitPage(HWND focus, LPARAM lparam) {
		return dialogProc(WM_INITDIALOG, reinterpret_cast<WPARAM>(focus), lparam);
	}
	
	// Invalidate the dialog
	void Invalidate() {
		dialogProc(TABDLG_INVALIDATE, 0, 0);
	}
		
	// Used for modal dialogs with IDOK and IDCANCEL
	bool OkToCommit() {
		bool commit = true;
		dialogProc(TABDLG_PRECOMMIT, 0, reinterpret_cast<LPARAM>(&commit));
		return commit;
	}
	void Commit() {
		dialogProc(TABDLG_COMMIT, 0, 0);
	}

	// The dialog is being closed. This is called after
	// all of the pages have been committed.
	void Close() {
		dialogProc(TABDLG_CLOSE, 0, 0);
	}

	// The dialog is being canceled
	void Cancel() {
		dialogProc(TABDLG_CANCEL, 0, 0);
	}

	// Switch from this page
	void SwitchFrom() {
		dialogProc(TABDLG_SWITCH_FROM, 0, 0);
	}
	
	// Switch to this page
	void SwitchTo() {
		dialogProc(TABDLG_SWITCH_TO, 0, 0);
	}

	// The general message proc. This proc is like a
	// dialog proc, not a window proc
	virtual INT_PTR dialogProc(
		UINT	msg,
		WPARAM	wParam,
		LPARAM	lParam
	) { return FALSE; }
	
private:
	ITabPage*		mpPage;
};


/***********************************************************************
 *
 * ITabDialogObject
 *
 * This interface is used to allow plugins to interact with a tabbed dialog.
 * Objects can expose this interface when they want to participate
 * in tab selection in a tabbed dialog.
 *
 * The dialog defines how objects can participate:
 *
 * The Render dialog allows the renderer to participate in the dialog tabs.
 * 
 * If the renderer doesn't expose this interface, a single separate
 * tab is created for the renderer's paramters, and the renderer is
 * allowed to create rollouts in this tab.
 *
 * If the renderer does expose this interface, its filter is assigned
 * to the dialog, to allow it to reject tabs that are not applicable
 * to its user interface, and it is informed of different actions
 * that are taking place.
 *
 **********************************************************************/
class ITabDialogObject : public BaseInterface {
public:
	// Notify the object that the dialog is activated 
	// or deactivated in a tabbed dialog
	virtual void Activate(ITabbedDialog* dialog) { }
	virtual void Deactivate(ITabbedDialog* dialog) { }

	// Add your tab(s) to the dialog. This will only be called if
	// both this object and the dialog agree that the tab should
	// be added. Dialog is the address of the dialog.
	virtual void AddTabToDialog(ITabbedDialog* dialog, ITabDialogPluginTab* tab) { }

	// Return true if the tabs added by the ITabDialogPluginTab tab
	// are acceptable for this dialog. Dialog is the dialog being
	// filtered.
	virtual int AcceptTab(
		ITabDialogPluginTab*	tab
	) { return TAB_DIALOG_ADD_TAB; }
};

#define TAB_DIALOG_OBJECT_INTERFACE_ID	Interface_ID(0x313c6db9, 0x42890bb3)

inline ITabDialogObject* GetTabDialogObject(InterfaceServer* obj)
{
	return static_cast<ITabDialogObject*>(obj->GetInterface(
		TAB_DIALOG_OBJECT_INTERFACE_ID));
}


inline int ITabDialogManager::AcceptTab(
	ITabDialogFilter*		filter,
	ITabDialogPluginTab*	tab
)
{
	int dlgResult = filter == NULL ? TAB_DIALOG_ADD_TAB : filter->AcceptTab(tab);
	if (dlgResult & TAB_DIALOG_ADD_TAB) {
		dlgResult &= ~TAB_DIALOG_ADD_TAB;
		dlgResult |= tab->AcceptDialog(filter);
	}
	return dlgResult;
}

#endif