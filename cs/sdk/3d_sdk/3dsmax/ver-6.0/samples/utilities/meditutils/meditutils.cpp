/**********************************************************************
 *<
	FILE: meditutils.cpp


	DESCRIPTION:	
	-- Multi Material Clean
	clean unused sub materials in scene multi-materials


	CREATED BY:		Alex Zadorozhny

	HISTORY:		Created 6/17/03

	*>	Copyright (c) 2003, All Rights Reserved.
	**********************************************************************/

#include "meditutils.h"
#include <stdmat.h>
#include "splshape.h"
#include <hash_map>
#include <algorithm>

// Clean MultiMaterial
#define MMCLEAN_CLASS_ID	Class_ID(0x66d5bbcb, 0x1896b0fc)
static Class_ID kMultiMatClassID(MULTI_CLASS_ID, 0);	// Class ID for the multi material

class MMClean : public UtilityObj {
	public:
		class			FixUpSet;		// Utility class that finds materials and Clean them
		class			FixUpDialog;	// Utility class for handling the dialog.

		
		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void DeleteThis() { }		

		MMClean();
		~MMClean();	

		// Find multi-materials with unused sub-materials
		// and fix them up. If prompt is true,
		// a dialog is displayed.
		bool FixUpAllMaterials(BOOL prompt = true);        
	
		// Find multi-materials with unused sub-materials.
		// The found materials are returned in mtls.
		void FindAllMaterials(Tab<Mtl*>& mtls);

		// Clean unused sub-materials
		// If prompt is true, show a dialog.
		bool FixUpMaterials(Tab<Mtl*>& mtls, BOOL prompt = true);      

};

// This utility class is used to find and clean the multi-materials 
// that have unused sub-materials. It doesn't have any UI.
class MMClean::FixUpSet {
public:
	FixUpSet() : mIp(*GetCOREInterface()) {}

	// Find multi-materials with unused sub-materials
	// The found materials are returned in mtls.
	void FindAll(Tab<Mtl*>& mtls);

private:
	friend class MMClean::FixUpDialog;

	// MaterialSet is the internal data structure used
	// to hold the materials found. The bool
	// is used to tell whether it is to be fixed or not.
	typedef std::hash_map<Mtl*, bool> MaterialSet;

	// Find multi-materials with unused sub-materials
	// The found materials are stored in mMaterials.
	void findAll();					// Find all scene materials      
	void findMtls(Mtl* mtl);		// Find all materials in mtl's material tree     
	int findNotUsedMtlID(Mtl* mtl, NumList& mtlIDNotUsed);  //Find unused Mtl ID's    

	// Copy the materials from mMaterials to mtls
	void outputMtls(Tab<Mtl*>& mtls);

	// Fix the marked materials in mMaterials.
	bool fixUp();                               

	// Add mtl to mMaterials
	bool addMtl(Mtl* mtl, bool marked = true);

	// Delete unused sub-materials.
	bool fixUpMtl(Mtl* mtl);                 

	Interface&		mIp;			// handy pointer to MAX interface
	MaterialSet		mMaterials;		// multi-materials with unused sub-materials
};


// This utility dialog is used to let the user see which multi-materials
// are to be changed and exclude some that they want to remain unchanged
class MMClean::FixUpDialog : public MMClean::FixUpSet {
public:
	FixUpDialog() {}
	~FixUpDialog() {}
	
	// Clean the materials. If prompt is true, the dialog is shown
	// Mtls allows an arbitrary set of materials to be changed.
	bool FixUpAll(bool prompt = true);
	bool FixUp(Tab<Mtl*>& mtls, bool prompt = true);

private:
	// Compare the names of two materials for sorting.
	// The dialog keeps a cache of type names so we don't have
	// to keep looking them up.
	class CompareMtl {
	public:
		CompareMtl(FixUpDialog* dlg) : mDlg(*dlg) {}

		bool operator()(
			Mtl* m1,
			Mtl* m2
			) const
		{
			return mDlg.lessThanMtl(m1, m2);
		}

	private:
		FixUpDialog&	mDlg;
	};
	friend class CompareMtl;

	// Display the dialog in a modal fashion.
	void doModal();

	// The dialog proc.
	static INT_PTR CALLBACK dialogProc(
		HWND		hwnd,
		UINT		msg,
		WPARAM		w,
		LPARAM		l
		);

	// WM_COMMAND handler
	bool cmd(UINT ctlID, UINT notification, HWND ctl);

	// WM_NOTIFY handler
	bool notify(UINT ctlID, NMHDR* hdr);

	// WM_INITDIALOG handler
	bool initDialog(HWND focus);

	// Special handler for space bar to switch the mark for
	// the entire selection set
	void spaceBar();

	// Fill the list control with the materials
	void fillMtlList();
	void appendMaterialToList(MaterialSet::iterator p);
	void getMtlString(Mtl* m, TSTR& entry);

	
	// Compare two material names 
	bool lessThanMtl(
		Mtl* m1,
		Mtl* m2
		) const;

	HWND			mWnd;			// Dialog window handle
	HWND			mListCtrl;		// List control window handle

};



static MMClean theMMClean;

class MMCleanClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theMMClean; }
	const TCHAR *	ClassName() { return GetString(IDS_MMCLEAN_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return MMCLEAN_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("MMClean"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};
static MMCleanClassDesc MMCleanDesc;
ClassDesc2* GetMMCleanDesc() { return &MMCleanDesc; }


// Function Publishing 
class MMCleanFPInterface : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(MMCleanFPInterface)

	enum {
		kFindAllMaterials = 1,
		kFixUpMaterials = 2,
		kFixUpAllMaterials = 3
	};

	BEGIN_FUNCTION_MAP

		VFN_1(kFindAllMaterials, theMMClean.FindAllMaterials, TYPE_MTL_TAB_BR)
		FN_2(kFixUpMaterials, TYPE_BOOL, theMMClean.FixUpMaterials, TYPE_MTL_TAB_BR, TYPE_BOOL)
		FN_1(kFixUpAllMaterials, TYPE_BOOL, theMMClean.FixUpAllMaterials, TYPE_BOOL)

	END_FUNCTION_MAP

	static MMCleanFPInterface mmCleanFPInterface;
};

#define MMCLEAN_FP_INTERFACE	Interface_ID(0x130335d6, 0xe7a7529)

MMCleanFPInterface MMCleanFPInterface::mmCleanFPInterface(
	MMCLEAN_FP_INTERFACE, _T("MMCleanInterface"), IDS_MMCLEAN_INTERFACE, &MMCleanDesc, FP_STATIC_METHODS,

	kFindAllMaterials, _T("findAll"), 0, TYPE_VOID, 0, 1,
	_T("mtlsFound"), 0, TYPE_MTL_TAB_BR,
	f_inOut, FPP_OUT_PARAM,
		
	kFixUpMaterials, _T("fix"), 0, TYPE_bool, 0, 2,
	_T("mtls"), 0, TYPE_MTL_TAB_BR,
	f_inOut, FPP_IN_PARAM,
	_T("prompt"), 0, TYPE_bool,
	f_keyArgDefault, true,

	kFixUpAllMaterials, _T("fixAll"), 0, TYPE_BOOL, 0, 1,
	_T("prompt"), 0, TYPE_bool,                  
	f_keyArgDefault, true,

	end
	);

class MMCleanActions : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(MMCleanActions)

	enum {
		kFixUpAllMaterialsAction = 1,
	};

	BEGIN_FUNCTION_MAP
		FN_0(kFixUpAllMaterialsAction, TYPE_BOOL, theMMClean.FixUpAllMaterials)
	END_FUNCTION_MAP

	static MMCleanActions mmCleanActions;
};

#define MMCLEAN_ACTIONS		Interface_ID(0x329367d7, 0x35753c0d)

MMCleanActions MMCleanActions::mmCleanActions(
	MMCLEAN_ACTIONS, _T("MMCleanActions"), IDS_MMCLEAN_ACTIONS, &MMCleanDesc, FP_ACTIONS,
	kActionMaterialEditorContext,

	kFixUpAllMaterialsAction, _T("CleanMaterailsAll"), IDS_MMCLEAN_ALL, 0,
	f_menuText,	IDS_MMCLEAN_MENU,
	end,
	
	end
	);



// utility rollout Dialog Proc
static BOOL CALLBACK MMCleanDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theMMClean.Init(hWnd);
			break;

		case WM_DESTROY:
			theMMClean.Destroy(hWnd);
			break;

		case WM_COMMAND:
			SetCursor(LoadCursor(NULL,IDC_WAIT));
			theMMClean.FixUpAllMaterials();
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theMMClean.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


MMClean::MMClean()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

MMClean::~MMClean()
{

}

void MMClean::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL_MMCLEAN),
		MMCleanDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void MMClean::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void MMClean::Init(HWND hWnd)
{
	hPanel = hWnd;
}

void MMClean::Destroy(HWND hWnd)
{
	hPanel = NULL;
}


bool MMClean::FixUpAllMaterials(BOOL prompt)
{
	
	// Create dialog and ask it to fix all materials
	FixUpDialog dialog;
	return dialog.FixUpAll(prompt != 0);
}

void MMClean::FindAllMaterials(Tab<Mtl*>& mtls)
{
	// Create utiltiy and ask it to find all materials
	FixUpSet materials;
	materials.FindAll(mtls);
}

bool MMClean::FixUpMaterials(Tab<Mtl*>& mtls, BOOL prompt)
{
	// Create dialog and ask it to fix materials in mtls
	FixUpDialog dialog;
	return dialog.FixUp(mtls, prompt != 0);
}

//--- MMClean::FixUpSet ---------------------------------------------
void MMClean::FixUpSet::FindAll(Tab<Mtl*>& mtls)
{
	// Find all materials and store in mtls
	findAll();
	outputMtls(mtls);
} 

void MMClean::FixUpSet::findAll()
{
	// Find all scene multi-materials
	MtlBaseLib* mlib =  mIp.GetSceneMtls();
	int count = mlib->Count();

	for (int i=0; i<count; i++) {
		Mtl* mtl = (Mtl*)(*mlib)[i];
		findMtls (mtl);
	}
}

void MMClean::FixUpSet::findMtls(Mtl* mtl)
{
	if (mtl != NULL) {
		NumList notused;
		// add multi-material with unused sub-material
		if (findNotUsedMtlID(mtl, notused)){
			if (notused.Count() > 0)
				addMtl(mtl);
		}
			
		// Look through all sub-materials
		int i, count = mtl->NumSubMtls();
		for (i = 0; i < count; ++i) {
			findMtls(mtl->GetSubMtl(i));
		}
	}
}

int MMClean::FixUpSet::findNotUsedMtlID(Mtl* mtl, NumList& mtlIDNotUsed)
{
	NumList mtlID, meshID;
	// get material ID's
	int subs = mtl->NumSubMtls();
	if (subs <= 0) subs = 1;
	for (int i=0; i<subs;i++) {
		if(mtl->GetSubMtl(i))
			mtlID.Add(i, TRUE);
	}
	int mtlID_count = mtlID.Count();

	// get mesh mtl ID's used by dependents               
	INodeTab nodeTab;
	FindNodesProc dep(&nodeTab);
	mtl->EnumDependents(&dep);
	int node_count = nodeTab.Count();
	if (!node_count) return FALSE;              
	
	// get mesh snapshot and indetify unused mtlID's       
	for (int i=0; i<node_count; i++) {
		Mesh* resMesh;
		const ObjectState& os = nodeTab[i]->EvalWorldState(mIp.GetTime());
		Object* ob = os.obj;	
		if (!ob->CanConvertToType(triObjectClassID)) return FALSE;
		if (ob->CanConvertToType(splineShapeClassID)) { // EditableSpline case
			SplineShape *splshp;
			splshp = (SplineShape *)ob->ConvertToType(mIp.GetTime(), splineShapeClassID);
			BezierShape shape = splshp->shape;
			int polys = shape.splineCount;
			for(int poly = 0; poly < polys; poly++) {
				Spline3D *spline = shape.splines[poly];
				for(int seg = 0; seg < spline->Segments(); seg++) {
					int splmid = spline->GetMatID(seg);
						if (splmid != -1)
							meshID.Add(splmid, TRUE);
				}
			}
		}
		TriObject *tri = (TriObject *)ob->ConvertToType(mIp.GetTime(), triObjectClassID);
		if (!tri) return FALSE;
		resMesh = new Mesh (tri->GetMesh());
		int numFaces = resMesh->numFaces;
		for (int i=0; i<numFaces; i++) {
			int mid = resMesh->faces[i].getMatID();
			meshID.Add(mid, TRUE);
		}
		delete resMesh;
	}
	
	for (int i=0; i<mtlID_count; i++) {
		if (meshID.Find(mtlID[i]) == -1) 
			mtlIDNotUsed.Add(mtlID[i], TRUE);
	}
	return TRUE;
	
}

void MMClean::FixUpSet::outputMtls(Tab<Mtl*>& mtls)
{
	int i, count = 0;

	// First we count the number of materials that
	// we are copying, for efficiency.
	MaterialSet::iterator p = mMaterials.begin();
	for ( ; p != mMaterials.end(); ++p) {
		if (p->first != NULL)
			++count;
	}

	// Append the materials to mtls
	i = mtls.Count();
	count += i;
	mtls.SetCount(count);

	// No copy the materials
	for (p = mMaterials.begin(); p != mMaterials.end(); ++p) {
		if (p->first != NULL) {
			DbgAssert(i < count);
			if (i < count)
				mtls[i] = p->first;
			++i;
		}
	}

	// make sure the final set is the correct size.
	mtls.SetCount(i);
}


bool MMClean::FixUpSet::addMtl(Mtl* m, bool marked)
{
	if (m == NULL || m->ClassID() != kMultiMatClassID)
		return false;
	// Add the material. If it was already in the set, mark it if it marked is true.
	mMaterials.insert(MaterialSet::value_type(m, marked)).first->second |= marked;
	return true;
}


bool MMClean::FixUpSet::fixUp()
{
	// Loop through the materials. And fix them up
	MaterialSet::iterator p = mMaterials.begin();
	while (p != mMaterials.end()) {
		// We may removed the material from the list,
		// so increment the iterator now.
		MaterialSet::iterator m = p;
		++p;

		// Fixup the materials in the list. 
		if (m->first == NULL || !m->second || fixUpMtl(m->first))
			mMaterials.erase(m);
	}

	return mMaterials.empty();	// Everything worked if the list is empty
}

bool MMClean::FixUpSet::fixUpMtl(Mtl* m)
{
	if (m == NULL) return false;

	// Fix the multi-material by deleting unused submtl
	NumList notused;
	findNotUsedMtlID(m , notused);
	int count = notused.Count();
	if (count == 0) return false;

	MultiMtl* multi = (MultiMtl*) m;
		
	for (int i=0; i<count; i++){
		multi->RemoveMtl(notused[i]);
	}
	return true;

}

//--- MMClean::FixUpDialog ------------------------------------------

bool MMClean::FixUpDialog::FixUpAll(bool prompt)
{
	// Find all materials
	findAll();

	// Either show the dialog, which will fix the materials,
	// or fix them.
	if (prompt) {
		doModal();
		return mMaterials.empty();
	}
	else {
		return fixUp();       
	}

	
}

bool MMClean::FixUpDialog::FixUp(Tab<Mtl*>& mtls, bool prompt)
{
	// find materials from mtls
	findAll();
	
	//keep only submitted in the mMaps hash table
	int count = mtls.Count(); 
	MaterialSet::iterator p = mMaterials.begin();
	while (p != mMaterials.end()) {
		bool found = false;
		// We may removed the material from the list,
		// so increment the iterator now.
		MaterialSet::iterator m = p;
		++p;
		for (int i=0; i<count; i++){
			if(m->first == mtls[i]) {
				found = true;
				break;
			}
		}
		if (!found)  mMaterials.erase(m);
	}

	// Either show the dialog, which will fix the materials,
	// or fix them.
	if (prompt) {
		doModal();
		return mMaterials.empty();
	}
	else {
		return fixUp();           
	}

	
}

void MMClean::FixUpDialog::doModal()
{
	// Display the dialog box
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_FIXUP_MMCLEAN),
		mIp.GetMAXHWnd(), dialogProc, reinterpret_cast<LPARAM>(this));
}


// Dialog handler for the dialog
INT_PTR CALLBACK MMClean::FixUpDialog::dialogProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
	)
{
	// Handle WM_INITIDIALOG special, so we can establish the
	// link between the window and the FixUpDialog object.
	if (msg == WM_INITDIALOG) {
		FixUpDialog* p = reinterpret_cast<FixUpDialog*>(l);
		CenterWindow(hwnd,GetParent(hwnd));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, l);
		p->mWnd = hwnd;
		return p->initDialog(reinterpret_cast<HWND>(w));
	}

	FixUpDialog* p = reinterpret_cast<FixUpDialog*>(
		GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (p != NULL) {
		switch (msg) {
		case WM_CLOSE:
			return EndDialog(p->mWnd, 0);		
		case WM_COMMAND:
			return p->cmd(LOWORD(w), HIWORD(w), reinterpret_cast<HWND>(l));
		case WM_NOTIFY:
			return p->notify(w, reinterpret_cast<NMHDR*>(l));
		
		}
	}
	return FALSE;
}


bool MMClean::FixUpDialog::cmd(
								  UINT		ctlID,
								  UINT		notfication,
								  HWND		ctl
								  )
{
	switch (ctlID) {
	case IDC_CANCEL:
		// Cancel the dialog
		EndDialog(mWnd, IDCANCEL);
		break;

	case IDC_OK:
		// Fix the materials and end the dialog if it is OK.
		// Make this undoable
		theHold.Begin();

		if (fixUp())
			EndDialog(mWnd, IDOK);
	
		// Accept the undo
		theHold.Accept(GetString(IDS_FIX_MMCLEAN));
		break;
	}

	return false;
}

bool MMClean::FixUpDialog::notify(
									 UINT		ctlID,
									 NMHDR*		nmhdr
									 )
{
	switch (ctlID) {
	case IDC_MTL_LIST:
		switch (nmhdr->code) {
	case LVN_KEYDOWN:
		// When the list control gets a spacebar it toggles the
		// check state of the item with focus. We do extra processing
		// to toggle the entire selection set.
		if (reinterpret_cast<NMLVKEYDOWN*>(nmhdr)->wVKey == VK_SPACE) {
			spaceBar();
		}
		break;
	case LVN_ITEMCHANGED: {
		// When an item is changed we sync its check state in mMaterials.
		NMLISTVIEW*p = reinterpret_cast<NMLISTVIEW*>(nmhdr);
		if ((p->uChanged & LVIF_STATE)
			&& ((p->uNewState ^ p->uOldState) & LVIS_STATEIMAGEMASK)) {
				// Check state changed
				if (p->iItem < 0) {
					// This means that all of the items changed. Copy all of
					// the item's CheckState value.
					int i, count = ListView_GetItemCount(mListCtrl);
					LVITEM item;
					item.iSubItem = 0;
					item.mask = LVIF_PARAM;
					for (i = 0; i < count; ++i) {
						item.iImage = i;
						item.lParam = 0;
						ListView_GetItem(mListCtrl, &item);
						MaterialSet::iterator m = mMaterials.find(
							reinterpret_cast<Mtl*>(p->lParam));
						if (m != mMaterials.end())
							m->second = ListView_GetCheckState(mListCtrl, p->iItem) != 0;
					}
				}
				else {
					// Copy a single item check state value.
					MaterialSet::iterator m = mMaterials.find(
						reinterpret_cast<Mtl*>(p->lParam));
					if (m != mMaterials.end())
						m->second = ListView_GetCheckState(mListCtrl, p->iItem) != 0;
				}
			}
						  }
		}
		break;
	}

	return false;
}

void MMClean::FixUpDialog::spaceBar()
{
	// Toggle all items in selection set
	bool checked = false;
	int i, count = ListView_GetItemCount(mListCtrl);

	// First find the current state. If any item is check
	// we will set all of the items to unchecked.
	for (i = 0; !checked && i < count; ++i) {
		if (ListView_GetItemState(mListCtrl, i, LVIS_SELECTED)) {
			bool c = ListView_GetCheckState(mListCtrl, i) != 0;
			checked |= c;
		}
	}

	// toggle state
	checked = !checked;

	// Set the new state. If the item has focus we set the inverse
	// of the new state, because the list control code will toggle it later.
	for (i = 0; i < count; ++i) {
		UINT state = ListView_GetItemState(mListCtrl, i, LVIS_SELECTED | LVIS_FOCUSED);
		if (state) {
			bool c = checked;
			if (state & LVIS_FOCUSED)
				c = !c;
			ListView_SetCheckState(mListCtrl, i, c);
		}
	}

	// Redraw everything.
	ListView_RedrawItems(mListCtrl, 0, count);
}

bool MMClean::FixUpDialog::initDialog(HWND focus)
{
	FixUpDialog* p = reinterpret_cast<FixUpDialog*>(
		GetWindowLongPtr(focus, GWLP_USERDATA));

	// Get the list control handle and turn on check boxes
	mListCtrl = GetDlgItem(mWnd, IDC_MTL_LIST);
	ListView_SetExtendedListViewStyle(mListCtrl,
		ListView_GetExtendedListViewStyle(mListCtrl) | LVS_EX_CHECKBOXES);

	// Add the materials to the list control
	fillMtlList();

	if (mMaterials.size() != 0) 
		SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_ALL_OK));
	else
		SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_NO_MTL));

	return true;
}

void MMClean::FixUpDialog::fillMtlList()
{
	// Fill the list box. Start by clearing any entries
	ListView_DeleteAllItems(mListCtrl);

	Tab<Mtl*> mtls;

	// Get the materials and sort them by name and type
	outputMtls(mtls);
	int count = mtls.Count();

	if (count > 0) {
		Mtl** addr = mtls.Addr(0);
		std::sort(addr, addr + count, CompareMtl(this));

		int i;
		for (i = 0; i < count; ++i) {
			// Add the material to the list.
			appendMaterialToList(mMaterials.find(mtls[i]));
		}
	}
}

void MMClean::FixUpDialog::appendMaterialToList(MaterialSet::iterator p)
{
	// Better be a valid material
	DbgAssert(p != mMaterials.end() && p->first != NULL);
	if (p == mMaterials.end() || p->first == NULL)
		return;

	// Get the name.
	TSTR entry;
	getMtlString(p->first, entry);

	// Add it to the list.
	LVITEM item;
	item.iItem = INT_MAX;
	item.iSubItem = 0;
	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.lParam = reinterpret_cast<LPARAM>(p->first);
	item.pszText = entry.data();

	// Save the checked state, because inserting the item will reset it.
	// When we handle the LVN_ITEMCHANGED notitification
	bool checked = p->second;
	int index = ListView_InsertItem(mListCtrl, &item);

	// Set the check state.
	ListView_SetCheckState(mListCtrl, index, checked);
}

bool MMClean::FixUpDialog::lessThanMtl(
	Mtl* m1,
	Mtl* m2
	) const
{
	// Compare two material name and types. 
	if (m1 == m2)
		return 0;
	if (m1 == NULL)
		return false;
	if (m2 == NULL)
		return true;

	return _tcsicmp(m1->GetName(), m2->GetName()) < 0;
}


void MMClean::FixUpDialog::getMtlString(Mtl* m, TSTR& entry)
{
	// Formate the name material-name (material-type)
	entry.Resize(0);
	if (m == NULL)
		return;
	entry = m->GetFullName();
}











