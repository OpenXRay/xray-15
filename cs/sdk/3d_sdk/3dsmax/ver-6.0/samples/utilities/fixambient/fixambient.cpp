/**********************************************************************
 *<
	FILE: fixambient.cpp

	DESCRIPTION:	Fix Ambient is a utility that will lock the ambient
					and diffuse colors and textures.

	CREATED BY:		Cleve Ard

	HISTORY:		Created 5/1/03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "fixambient.h"
#include "objectParams.h"
#include <stdmat.h>
#include <helpsys.h>
#include <hash_map>
#include <algorithm>

#define FIXAMBIENT_CLASS_ID	Class_ID(0x2ba09dc5, 0x2f7ff84e)









static Class_ID kStdMatClassID(DMTL_CLASS_ID, 0);	// Class ID for the standard material

// These strings are used to get various material values
// from a material using MAX Script.
static TCHAR kAmbientColor[] = _T("ambientColor");
static TCHAR kAmbientTexture[] = _T("ambientMap");
static TCHAR kAmbientTexEnabled[] = _T("ambientMapEnable");
static TCHAR kAmbientTexAmount[] = _T("ambientMapAmount");
static TCHAR kDiffuseColor[] = _T("diffuseColor");
static TCHAR kDiffuseTexture[] = _T("diffuseMap");
static TCHAR kDiffuseTexEnabled[] = _T("diffuseMapEnable");
static TCHAR kDiffuseTexAmount[] = _T("diffuseMapAmount");
static TCHAR kADLock[] = _T("adLock");
static TCHAR kADTextureLock[] = _T("adTextureLock");

class FixAmbient : public UtilityObj {
public:

	class			FixUpSet;		// Utility class that finds materials and fixes them
	class			FixUpDialog;	// Utility class for handling the dialog.

	HWND			hPanel;			// Panel window handle
	IUtil			*iu;			// Utility interface pointer
	Interface		*ip;			// MAX interface pointer


	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	

	void DeleteThis() { }		
	//Constructor/Destructor

	FixAmbient();
	~FixAmbient();		

	// Find materials with different ambient and diffuse
	// colors and fix them up. If prompt is true,
	// a dialog is displayed.
	bool FixUpAllMaterials(BOOL prompt = true);
	bool FixUpSelectedMaterials(BOOL prompt = true);

	// Find materials with different ambient and diffuse
	// color. The found materials are returned in mtls.
	// Objects is the objects whose materials are to be
	// searched.
	void FindAllMaterials(Tab<Mtl*>& mtls);
	void FindSelectedMaterials(Tab<Mtl*>& mtls);
	void FindMaterials(Tab<INode*>& objects, Tab<Mtl*>& mtls);
	void FindMaterials(Tab<Mtl*>& inMtls, Tab<Mtl*>& mtls);

	// Force the materials in mtls to have the same
	// ambient and diffuse channel, if they are different.
	// If prompt is true, show a dialog.
	bool FixUpMaterials(Tab<Mtl*>& mtls, BOOL prompt = true);

	// Determine whether the fixup selected action should
	// be enabled.
	bool FixUpSelectedIsEnabled();

	// These are handy utility routines from getting the
	// values we want from the material. They all use
	// MAX Script.
	static bool		getAmbClr(Mtl* m, Color& color)
		{ return getMAXScriptValue(m, kAmbientColor, 0, color); }
	static Control*	getAmbClrCtrl(Mtl* m, ParamDimension*& dim)
		{ return getMAXScriptController(m, kAmbientColor, dim); }
	static bool		getDifClr(Mtl* m, Color& color)
		{ return getMAXScriptValue(m, kDiffuseColor, 0, color); }
	static Control*	getDifClrCtrl(Mtl* m, ParamDimension*& dim)
		{ return getMAXScriptController(m, kDiffuseColor, dim); }

	static bool		getAmbTex(Mtl* m, Texmap*& tex)
		{ return getMAXScriptValue(m, kAmbientTexture, 0, tex); }
	static bool		getDifTex(Mtl* m, Texmap*& tex)
		{ return getMAXScriptValue(m, kDiffuseTexture, 0, tex); }

	static bool		getAmbEnb(Mtl* m, bool& enabled)
		{ return getMAXScriptValue(m, kAmbientTexEnabled, 0, enabled); }
	static bool		getDifEnb(Mtl* m, bool& enabled)
		{ return getMAXScriptValue(m, kDiffuseTexEnabled, 0, enabled); }

	static bool		getADLock(Mtl* m, bool& locked)
		{ return getMAXScriptValue(m, kADLock, 0, locked); }
	static bool		getADTexLock(Mtl* m, bool& locked)
		{ return getMAXScriptValue(m, kADTextureLock, 0, locked); }
	static bool		setADLock(Mtl* m, bool locked)
		{ return setMAXScriptValue(m, kADLock, 0, locked); }
	static bool		setADTexLock(Mtl* m, bool locked)
		{ return setMAXScriptValue(m, kADTextureLock, 0, locked); }
};

// This utility class is used to find the materials we want to
// change and to change them. It doesn't have any UI.
class FixAmbient::FixUpSet {
public:
	FixUpSet() : mIp(*GetCOREInterface()) {}

	// Find materials with different ambient and diffuse
	// color. The found materials are returned in mtls.
	// Objects is the objects whose materials are to be
	// searched.
	void FindAll(Tab<Mtl*>& mtls);
	void FindSelected(Tab<Mtl*>& mtls);
	void Find(Tab<INode*>& objects, Tab<Mtl*>& mtls);
	void Find(Tab<Mtl*>& inMtls, Tab<Mtl*>& mtls);
	
private:
	friend class FixAmbient::FixUpDialog;

	// MaterialSet is the internal data structure we use
	// to hold the materials we have found. The bool
	// is used to tell whether it is to be fixed or not.
	typedef std::hash_map<Mtl*, bool> MaterialSet;

	// Find materials with different ambient and diffuse
	// color. The found materials are stored in mMaterials.
	// Objects is the objects whose materials are to be
	// searched.
	void findAll();					// Find all scene materials
	void findAll(INode* node);		// Find materials used by node and its children
	void findSelected();			// Find materials used by selection set
	void find(Tab<INode*>& objects);// Find materials used by nodes in objects
	void find(Tab<Mtl*>& mtls);		// Find materials used by materials in mtls
	void findMtls(INode* node);		// Find materials used by node
	void findMtls(Mtl* mtl);		// Find all materials in mtl's material tree

	// Copy the materials from mMaterials to mtls
	void outputMtls(Tab<Mtl*>& mtls);

	// Copy the materials from mtls to mMaterials, checking
	// whether they have different ambient and diffuse colors.
	// Marked sets the initial state of whether the material
	// is to be fixed.
	void inputMtls(Tab<Mtl*>& mtls, bool marked = true);

	// Fix the marked materials in mMaterials.
	bool fixUp();

	// Add mtl to mMaterials, checking whether it has different
	// ambient and diffuse materials.
	bool addMtl(Mtl* mtl, bool marked = true);

	// Lock mtls ambient and diffuse color and texture.
	bool fixUpMtl(Mtl* mtl);

	Interface&		mIp;			// handy pointer to MAX interface
	MaterialSet		mMaterials;		// materials with different ambient and diffuse
};


// This utility dialog is used to let the user see which materials
// are to be changed and exclude some that they want to remain different
class FixAmbient::FixUpDialog : public FixAmbient::FixUpSet {
public:
	FixUpDialog() : mClassDir(mIp.GetDllDir().ClassDir()), mError(0), mWndProc(NULL) {}

	// Fix the materials. If prompt is true, the dialog is shown
	// Mtls allows an arbitrary set of materials to be changed.
	bool FixUpAll(bool prompt = true);
	bool FixUpSelected(bool prompt = true);
	bool FixUp(Tab<Mtl*>& mtls, bool prompt = true);

private:
	enum ErrorFlags {
		kErrorAmbientOnlyLight = 1,
		kErrorAmbientInEnvironment = 2,
		kErrorFixup = 4
	};

	// Compare the names and types of two materials for sorting.
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

	// This is a hash_compare class used to hash and compare class ids.
	// It is needed by std::hash_map. We use it for caching material types.
	class CompareID {
	public:
		enum {
			bucket_size = 4,
			min_buckets = 8
		};

		size_t operator()(const Class_ID& key) const
		{
			// Poor const usage in Class_ID
			return const_cast<Class_ID*>(&key)->PartA()
				^ const_cast<Class_ID*>(&key)->PartB();
		}
		bool operator()(
			const Class_ID& k1,
			const Class_ID& k2
		) const
		{
			return k1 < k2;
		}
	};

	// The material type cache.
	typedef std::hash_map<Class_ID, ClassEntry*, CompareID> IDMap;

	// Display the dialog in a modal fashion.
	void doModal();

	// The dialog proc.
	static INT_PTR CALLBACK dialogProc(
		HWND		hwnd,
		UINT		msg,
		WPARAM		w,
		LPARAM		l
	);

	// We subclass the dialog's window proc so we can change the
	// color of error messages.
	static LRESULT CALLBACK windowProc(
		HWND		hwnd,
		UINT		msg,
		WPARAM		w,
		LPARAM		l
	);

	// Dialog message handler
	INT_PTR proc(
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

	// WM_WINDOWPOSCHANGING handler - resizes and moves control
	// when the dialog size is changed.
	bool resizeDialog(LPWINDOWPOS pos);
	void moveControl(int id, int dx, int dy);

	// Special handler for space bar to switch the mark for
	// the entire selection set
	void spaceBar();

	// Fill the list control with the materials
	void fillMtlList();
	void appendMaterialToList(MaterialSet::iterator p);
	void getMtlString(Mtl* m, TSTR& entry);

	// Set and save the initial window position for the dialog
	void positionWindow();
	void getConfigFileName();
	void saveWindowPos();

	// Compare two material names and types.
	bool lessThanMtl(
		Mtl* m1,
		Mtl* m2
	) const;

	// Get the class type string
	TCHAR* getString(const Class_ID& id) const;

	static TSTR		mConfig;		// Name of configuration file

	HWND			mWnd;			// Dialog window handle
	HWND			mListCtrl;		// List control window handle
	mutable IDMap	mIDMap;			// Cache for materials types
	ClassDirectory&	mClassDir;		// Class directory
	int				mBottom;		// Bottom of the list control, used by resizing
	UINT			mError;			// Error flag - used to turn text red
	WNDPROC			mWndProc;		// Sub classed window proc
};

// The single instance of the utility
static FixAmbient theFixAmbient;

class FixAmbientClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theFixAmbient; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return FIXAMBIENT_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("FixAmbient"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static FixAmbientClassDesc FixAmbientDesc;
ClassDesc2* GetFixAmbientDesc() { return &FixAmbientDesc; }


class FixAmbientScriptInterface : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(FixAmbientScriptInterface)

	enum {
		kFixUpAllMaterials = 1,
		kFixUpSelectedMaterials = 2,
		kFindAllMaterials = 3,
		kFindSelectedMaterials = 4,
		kFindMaterials1 = 5,
		kFindMaterials2 = 6,
		kFixUpMaterials = 7
	};

	BEGIN_FUNCTION_MAP
		
		FN_1(kFixUpAllMaterials, TYPE_BOOL, theFixAmbient.FixUpAllMaterials, TYPE_BOOL)
		FN_1(kFixUpSelectedMaterials, TYPE_BOOL, theFixAmbient.FixUpSelectedMaterials, TYPE_BOOL)
		VFN_1(kFindAllMaterials, theFixAmbient.FindAllMaterials, TYPE_MTL_TAB_BR)
		VFN_1(kFindSelectedMaterials, theFixAmbient.FindSelectedMaterials, TYPE_MTL_TAB_BR)
		VFN_2(kFindMaterials1, theFixAmbient.FindMaterials, TYPE_INODE_TAB_BR, TYPE_MTL_TAB_BR)
		VFN_2(kFindMaterials2, theFixAmbient.FindMaterials, TYPE_MTL_TAB_BR, TYPE_MTL_TAB_BR)
		FN_2(kFixUpMaterials, TYPE_BOOL, theFixAmbient.FixUpMaterials, TYPE_MTL_TAB_BR, TYPE_BOOL)

	END_FUNCTION_MAP

	static FixAmbientScriptInterface fixAmbientScriptInterface;
};

#define FIX_AMBIENT_SCRIPT_INTERFACE	Interface_ID(0x56a83b9f, 0x705045f0)

FixAmbientScriptInterface FixAmbientScriptInterface::fixAmbientScriptInterface(
	FIX_AMBIENT_SCRIPT_INTERFACE, _T("fixAmbientColors"), IDS_FIX_AMBIENT_COLORS, &FixAmbientDesc, FP_STATIC_METHODS,

	kFixUpAllMaterials, _T("fixAll"), IDS_FIXUP_ALL, TYPE_BOOL, 0, 1,
		_T("prompt"), IDS_PROMPT, TYPE_BOOL,
			f_keyArgDefault, TRUE,
	kFixUpSelectedMaterials, _T("fixSelected"), IDS_FIXUP_SELECTED, TYPE_BOOL, 0, 1,
		_T("prompt"), IDS_PROMPT, TYPE_BOOL,
			f_keyArgDefault, TRUE,
	kFindAllMaterials, _T("findAll"), IDS_FIND_ALL, TYPE_VOID, 0, 1,
		_T("mtlsFound"), IDS_MTLSFOUND, TYPE_MTL_TAB_BR,
			f_inOut, FPP_OUT_PARAM,
	kFindSelectedMaterials, _T("findSelected"), IDS_FIND_SELECTED, TYPE_VOID, 0, 1,
		_T("mtlsFound"), IDS_MTLSFOUND, TYPE_MTL_TAB_BR,
			f_inOut, FPP_OUT_PARAM,
	kFindMaterials1, _T("find"), IDS_FIND, TYPE_VOID, 0, 2,
		_T("objects"), IDS_OBJECTS, TYPE_INODE_TAB_BR,
			f_inOut, FPP_IN_PARAM,
		_T("mtlsFound"), IDS_MTLSFOUND, TYPE_MTL_TAB_BR,
			f_inOut, FPP_OUT_PARAM,
	kFindMaterials2, _T("findMaterials"), IDS_FIND_MTLS, TYPE_VOID, 0, 2,
		_T("mtls"), IDS_OBJECTS, TYPE_MTL_TAB_BR,
			f_inOut, FPP_IN_PARAM,
		_T("mtlsFound"), IDS_MTLSFOUND, TYPE_MTL_TAB_BR,
			f_inOut, FPP_OUT_PARAM,
	kFixUpMaterials, _T("fix"), IDS_FIXUP_MTLS, TYPE_BOOL, 0, 2,
		_T("mtls"), IDS_MTLS, TYPE_MTL_TAB_BR,
			f_inOut, FPP_IN_PARAM,
		_T("prompt"), IDS_PROMPT, TYPE_BOOL,
			f_keyArgDefault, TRUE,

	end
);

class FixAmbientScriptActions : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(FixAmbientScriptActions)

	enum {
		kFixUpAllMaterialsAction = 1,
		kFixUpSelectedMaterialsAction = 2,
		kFixUpSelectedIsEnabled = 3
	};

	BEGIN_FUNCTION_MAP
		
		FN_0(kFixUpAllMaterialsAction, TYPE_BOOL, theFixAmbient.FixUpAllMaterials)
		FN_0(kFixUpSelectedMaterialsAction, TYPE_BOOL, theFixAmbient.FixUpSelectedMaterials)
		FN_0(kFixUpSelectedIsEnabled, TYPE_BOOL, theFixAmbient.FixUpSelectedIsEnabled)

	END_FUNCTION_MAP

	static FixAmbientScriptActions fixAmbientScriptActions;
};

#define FIX_AMBIENT_ACTIONS		Interface_ID(0x58935559, 0x51367380)

FixAmbientScriptActions FixAmbientScriptActions::fixAmbientScriptActions(
	FIX_AMBIENT_ACTIONS, _T("FixAmbientActions"), IDS_FIX_AMBIENT_ACTIONS, &FixAmbientDesc, FP_ACTIONS,
		kActionMainUIContext,

	kFixUpAllMaterialsAction, _T("fixAmbientAll"), IDS_FIX_AMBIENT_ALL, 0,
		end,
	kFixUpSelectedMaterialsAction, _T("fixAmbientSelected"), IDS_FIX_AMBIENT_SELECTED, 0,
		f_isEnabled, kFixUpSelectedIsEnabled,
		end,
//	kFixUpSelectedIsEnabled, _T("fixAmbiendSelectedIsEnabled"), IDS_FIXUP_SELECTED_IS_ENABLED, 0,

	end
);
	
// This is the handler for the panel dialog
static BOOL CALLBACK FixAmbientDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theFixAmbient.Init(hWnd);
			break;

		case WM_DESTROY:
			theFixAmbient.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_FIND_ALL:
				theFixAmbient.FixUpAllMaterials();
				break;

			case IDC_FIND_SELECTED:
				theFixAmbient.FixUpSelectedMaterials();
				break;

			case IDC_DO_HELP:
				DoHelp(HELP_CONTEXT, idh_fix_ambient);
				return FALSE;
			}
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theFixAmbient.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

//--- FixAmbient -------------------------------------------------------
FixAmbient::FixAmbient()
	: iu(NULL), ip(NULL), hPanel(NULL)
{
}

FixAmbient::~FixAmbient()
{

}

void FixAmbient::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		FixAmbientDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void FixAmbient::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void FixAmbient::Init(HWND hWnd)
{
	hPanel = hWnd;

	SetWindowContextHelpId(hWnd, idh_fix_ambient);
}

void FixAmbient::Destroy(HWND hWnd)
{
	hPanel = NULL;
}

bool FixAmbient::FixUpAllMaterials(BOOL prompt)
{
	// Create dialog and ask it to fix all materials
	FixUpDialog dialog;
	return dialog.FixUpAll(prompt != 0);
}

bool FixAmbient::FixUpSelectedMaterials(BOOL prompt)
{
	// Create dialog and ask it to fix selected materials
	FixUpDialog dialog;
	return dialog.FixUpSelected(prompt != 0);
}

void FixAmbient::FindAllMaterials(Tab<Mtl*>& mtls)
{
	// Create utiltiy and ask it to find all materials
	FixUpSet materials;
	materials.FindAll(mtls);
}

void FixAmbient::FindSelectedMaterials(Tab<Mtl*>& mtls)
{
	// Create utiltiy and ask it to find selected materials
	FixUpSet materials;
	materials.FindSelected(mtls);
}

void FixAmbient::FindMaterials(Tab<INode*>& objects, Tab<Mtl*>& mtls)
{
	// Create utiltiy and ask it to find materials used by objects
	FixUpSet materials;
	materials.Find(objects, mtls);
}

void FixAmbient::FindMaterials(Tab<Mtl*>& inMtls, Tab<Mtl*>& mtls)
{
	// Create utiltiy and ask it to find materials
	FixUpSet materials;
	materials.Find(inMtls, mtls);
}

bool FixAmbient::FixUpMaterials(Tab<Mtl*>& mtls, BOOL prompt)
{
	// Create dialog and ask it to fix materials in mtls
	FixUpDialog dialog;
	return dialog.FixUp(mtls, prompt != 0);
}

bool FixAmbient::FixUpSelectedIsEnabled()
{
	// Fixup selected is enabled when something is selected.
	return GetCOREInterface()->GetSelNodeCount() != 0;
}




//--- FixAmbient::FixUpSet ---------------------------------------------
void FixAmbient::FixUpSet::FindAll(Tab<Mtl*>& mtls)
{
	// Find all materials and store in mtls
	findAll();
	outputMtls(mtls);
}

void FixAmbient::FixUpSet::FindSelected(Tab<Mtl*>& mtls)
{
	// Find selected materials and store in mtls
	findSelected();
	outputMtls(mtls);
}

void FixAmbient::FixUpSet::Find(Tab<INode*>& objects, Tab<Mtl*>& mtls)
{
	// Find materials used by objects and store in mtls
	find(objects);
	outputMtls(mtls);
}

void FixAmbient::FixUpSet::Find(Tab<Mtl*>& inMtls, Tab<Mtl*>& mtls)
{
	// Find materials used by objects and store in mtls
	find(inMtls);
	outputMtls(mtls);
}

void FixAmbient::FixUpSet::findAll()
{
	// Find all materials starting with the root node.
	// We don't worry about materials assigned to XREF
	// sceen nodes, because we can't change them anyway.
	findAll(mIp.GetRootNode());
}

void FixAmbient::FixUpSet::findAll(INode* node)
{
	if (node != NULL) {
		// Find any materials used by node.
		findMtls(node);

		// Recursively find materials for children
		int i, count = node->NumberOfChildren();
		for (i = 0; i < count; ++i)
			findAll(node->GetChildNode(i));
	}
}

void FixAmbient::FixUpSet::findMtls(INode* node)
{
	if (node != NULL) {
		// Find materials for this node
		findMtls(node->GetMtl());
	}
}

void FixAmbient::FixUpSet::findMtls(Mtl* mtl)
{
	if (mtl != NULL) {
		// Add material to mMaterials, which will check
		// whether the ambient and diffuse colors are
		// different.
		addMtl(mtl);

		// Look through all sub-materials
		int i, count = mtl->NumSubMtls();
		for (i = 0; i < count; ++i) {
			findMtls(mtl->GetSubMtl(i));
		}
	}
}

void FixAmbient::FixUpSet::findSelected()
{
	// Find materials used by nodes in selection set
	int i, count = mIp.GetSelNodeCount();
	for (i = 0; i < count; ++i)
		findMtls(mIp.GetSelNode(i));
}

void FixAmbient::FixUpSet::find(Tab<INode*>& objects)
{
	// Find materials used by nodes in objects.
	int i, count = objects.Count();
	for (i = 0; i < count; ++i)
		findMtls(objects[i]);
}

void FixAmbient::FixUpSet::find(Tab<Mtl*>& inMtls)
{
	// Find materials used by nodes in objects.
	int i, count = inMtls.Count();
	for (i = 0; i < count; ++i)
		findMtls(inMtls[i]);
}

void FixAmbient::FixUpSet::outputMtls(Tab<Mtl*>& mtls)
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

	// Shouldn't be necessary, but why not, make sure
	// the final set is the correct size.
	DbgAssert(i == count);
	mtls.SetCount(i);
}

void FixAmbient::FixUpSet::inputMtls(Tab<Mtl*>& mtls, bool marked)
{
	int i, count = mtls.Count();
	for (i = 0; i < count; ++i) {
		// addMtl checks for different ambient and diffuse materials
		addMtl(mtls[i], marked);
	}
}

bool FixAmbient::FixUpSet::addMtl(Mtl* m, bool marked)
{
	if (m == NULL || m->ClassID() != kStdMatClassID)
		return false;

	bool same = true;
	bool locked = false;

	// First check the color
	if (!getADLock(m, locked) || !locked) {
		// Diffuse and Ambient colors are not locked
		// Let's see if they are different.
		ParamDimension* dim;
		Control* ctrl = getAmbClrCtrl(m, dim);
		if (ctrl != getDifClrCtrl(m, dim))
			same = false;			// Different controllers means they are different
		else if (ctrl == NULL) {
			Color ambClr, difClr;
			if (!getAmbClr(m, ambClr) || !getDifClr(m, difClr)
					|| ambClr != difClr)
				same = false;		// Different colors means they are different
		}
	}

	// Now check the texture, but only if the colors are the same
	locked = false;
	if (same && (!getADTexLock(m, locked) || !locked)) {
		bool ambEnb, difEnb;
		Texmap* ambTex, *difTex;

		// Get the ambient parameters. If ambient texture isn't enabled,
		// don't get the texture. If we can't get the enable or texture,
		// we add this to the list.
		if (!getAmbEnb(m, ambEnb) || (ambEnb && !getAmbTex(m, ambTex))) {
			same = false;
		}
		// Get the diffuse parameters. If diffuse texture isn't enabled,
		// don't get the texture. If we can't get the enable or texture,
		// we add this to the list.
		else if (!getDifEnb(m, difEnb) || (difEnb && !getDifTex(m, difTex))) {
			same = false;
		}
		// Compare the ambient and diffuse values. We will add the material
		// when either channel has an enabled texture. We don't compare the
		// ambient and diffuse texture amount, because we don't have a good
		// mechanism for getting the controller if there is one.
		else if ((ambEnb && ambTex != NULL) || (difEnb && difTex != NULL)) {
			same = false;
		}
	}

	// Ambient and diffuse are the same so don't add it
	if (same)
		return false;

	// Add the material. If it was already in the set, mark it if it marked is true.
	mMaterials.insert(MaterialSet::value_type(m, marked)).first->second |= marked;
	return true;
}

bool FixAmbient::FixUpSet::fixUp()
{
	// We are about to fix the materials in mMaterials

	// Loop through the materials. And fix them up
	MaterialSet::iterator p = mMaterials.begin();
	while (p != mMaterials.end()) {
		// We may removed the material from the list,
		// so increment the iterator now.
		MaterialSet::iterator m = p;
		++p;

		// Fixup the materials in the list. Only leave the material
		// in the list, if fixUpMtl couldn't fix it.
		if (m->first == NULL || !m->second || fixUpMtl(m->first))
			mMaterials.erase(m);
	}

	return mMaterials.empty();	// Everything worked if the list is empty
}

bool FixAmbient::FixUpSet::fixUpMtl(Mtl* m)
{
	if (m == NULL)
		return false;

	// Fix the material by locking the ambient color and texture.
	return setADLock(m, true) | setADTexLock(m, true);
}



//--- FixAmbient::FixUpDialog ------------------------------------------
TSTR	FixAmbient::FixUpDialog::mConfig;

bool FixAmbient::FixUpDialog::FixUpAll(bool prompt)
{
	// Find all materials
	findAll();

	// Either show the dialog, which will fix the materials,
	// or fix them.
	if (prompt) {
		doModal();
	}
	else {
		fixUp();
	}

	return mMaterials.empty();
}

bool FixAmbient::FixUpDialog::FixUpSelected(bool prompt)
{
	// find materials used by selection set
	findSelected();

	// Either show the dialog, which will fix the materials,
	// or fix them.
	if (prompt) {
		doModal();
	}
	else {
		fixUp();
	}

	return mMaterials.empty();
}

bool FixAmbient::FixUpDialog::FixUp(Tab<Mtl*>& mtls, bool prompt)
{
	// find materials from mtls
	inputMtls(mtls);

	// Either show the dialog, which will fix the materials,
	// or fix them.
	if (prompt) {
		doModal();
	}
	else {
		fixUp();
	}

	return mMaterials.empty();
}

void FixAmbient::FixUpDialog::doModal()
{
	// Display the dialog box
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_FIXUP_MTLS),
		mIp.GetMAXHWnd(), dialogProc, reinterpret_cast<LPARAM>(this));
}

LRESULT CALLBACK FixAmbient::FixUpDialog::windowProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	// We sublass the window proc of the dialog because the system
	// color code in max doesn't let WM_CTRLCOLORSTATIC filter
	// down to the dialog. We use this to turn error text red.
	FixUpDialog* p = reinterpret_cast<FixUpDialog*>(
		GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (p == NULL)
		return DefWindowProc(hwnd, msg, w, l);

	switch (msg) {
	case WM_CTLCOLORSTATIC:
		// If we have an error and this static is the message box
		// turn it red.
		if (GetWindowLong(reinterpret_cast<HWND>(l), GWL_ID) == IDC_FIX_MESSAGE
				&& p->mError != 0) {
			// Let the MAX code set the colors, then change the text to red.
			LRESULT r = CallWindowProc(p->mWndProc, hwnd, msg, w, l);
			SetTextColor(reinterpret_cast<HDC>(w), RGB(255, 0, 0));
			return r;
		}
		break;
	}

	// Pass call to subclassed proc.
	return CallWindowProc(p->mWndProc, hwnd, msg, w, l);
}

// Dialog handler for the dialog
INT_PTR CALLBACK FixAmbient::FixUpDialog::dialogProc(
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
		SetWindowLongPtr(hwnd, GWLP_USERDATA, l);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL,
			GetClassLong(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));
		p->mWnd = hwnd;
		return p->initDialog(reinterpret_cast<HWND>(w));
	}

	FixUpDialog* p = reinterpret_cast<FixUpDialog*>(
		GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (p != NULL) {
		switch (msg) {
		case WM_COMMAND:
			return p->cmd(LOWORD(w), HIWORD(w), reinterpret_cast<HWND>(l));
		case WM_NOTIFY:
			return p->notify(w, reinterpret_cast<NMHDR*>(l));
		}
		return p->proc(msg, w, l);
	}
	return FALSE;
}

// Our dialog message processor
INT_PTR FixAmbient::FixUpDialog::proc(
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	switch (msg) {
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO p = reinterpret_cast<LPMINMAXINFO>(l);
			RECT combined, client, dlg, msg;
			GetWindowRect(GetDlgItem(mWnd, IDOK), &dlg);
			GetWindowRect(GetDlgItem(mWnd, IDCANCEL), &client);
			GetWindowRect(GetDlgItem(mWnd, IDC_FIX_MESSAGE), &msg);
			UnionRect(&combined, &dlg, &client);
			GetClientRect(mWnd, &client);
			ClientToScreen(mWnd, reinterpret_cast<LPPOINT>(&client));
			ClientToScreen(mWnd, reinterpret_cast<LPPOINT>(&client) + 1);
			GetWindowRect(mWnd, &dlg);

			int temp = 2 * (client.right - combined.right)		// Space on left and right are the same
				+ combined.right - combined.left				// The width of the buttons.
				+ client.left - dlg.left						// Left non-client region
				+ dlg.right - client.right;						// Right non-client regin
			if (temp > p->ptMinTrackSize.x)
				p->ptMinTrackSize.x = temp;

			temp = 2 * (client.bottom - combined.bottom)		// Space on top and bottom are the same
				+ combined.bottom - combined.top				// The width of the buttons.
				+ client.top - dlg.top							// Top non-client region
				+ dlg.bottom - client.bottom					// Bottom non-client regin
				+ msg.bottom - client.top;						// Space for the message at the top
			if (temp > p->ptMinTrackSize.y)
				p->ptMinTrackSize.y = temp;
		}
		return 1;
	
	case WM_WINDOWPOSCHANGING:
		// Resize when WM_WINDOWPOSCHANGING is received
		return resizeDialog(reinterpret_cast<LPWINDOWPOS>(l));
	case WM_DESTROY:
		// Save the window position when we are destroyed.
		saveWindowPos();
		break;
	case WM_SYSCOMMAND:
		if ((w & 0xfff0) == SC_CONTEXTHELP) {
			DoHelp(HELP_CONTEXT, idh_fix_ambient);
			return FALSE;
		}
	}

	return FALSE;
}


bool FixAmbient::FixUpDialog::cmd(
	UINT		ctlID,
	UINT		notfication,
	HWND		ctl
)
{
	switch (ctlID) {
	case IDCANCEL:
		// Cancel the dialog
		EndDialog(mWnd, IDCANCEL);
		break;
		
	case IDOK:
		// Fix the materials and end the dialog if it is OK.

		// Make this undoable
		theHold.Begin();

		if (fixUp())
			EndDialog(mWnd, IDOK);
		else {
			// Bad material. Change the text and set the error flag.
			mError |= kErrorFixup;
			SetWindowText(GetDlgItem(mWnd, IDC_FIX_MESSAGE), GetString(IDS_FIXUP_ERROR));
			fillMtlList();	// Show the materials that didn't work
		}

		// Accept the undo
		theHold.Accept(GetString(IDS_FIX_AMBIENT));
		break;
	}

	return false;
}

bool FixAmbient::FixUpDialog::notify(
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

void FixAmbient::FixUpDialog::spaceBar()
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

// These message ids are used to display information about the use
// of the ambient colors to the user.
static const int msgs[4] = {
	IDS_ALL_OK, IDS_AMBIENT_LIGHT, IDS_AMBIENT_VALUE, IDS_AMBIENT_VALUE_AND_LIGHT
};

// Return whether one of the objects in node and its children is a light
// that is on and as ambientOnly checked.
static bool findAmbientOnlyLight(INode* node, TimeValue t)
{
	if (node == NULL)
		return false;

	ObjectState os = node->EvalWorldState(t);
	if (os.obj != NULL && os.obj->SuperClassID() == LIGHT_CLASS_ID) {
		// Ok We have a light. Now to figure out whether it is ambient-only
		bool amb;
		if (getMAXScriptValue(os.obj, _T("ambientOnly"), t, amb) && amb
				&& (!getMAXScriptValue(os.obj, _T("on"), t, amb) || amb))
			return true;	// Got one, we can stop looking
	}

	int i, count = node->NumberOfChildren();
	for (i = 0; i < count; ++i) {
		if (findAmbientOnlyLight(node->GetChildNode(i), t))
			return true;	// Got one, we can stop looking
	}

	return false;		// Keep looking.
}

bool FixAmbient::FixUpDialog::initDialog(HWND focus)
{
	// Subclass the dialog so we can show error messages in red
	mWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(mWnd, GWLP_WNDPROC));
	if (mWndProc != NULL) {
		SetWindowLongPtr(mWnd, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(windowProc));
	}

	SetWindowContextHelpId(mWnd, idh_fix_ambient);

	// Get the list control handle and turn on check boxes
	mListCtrl = GetDlgItem(mWnd, IDC_MTL_LIST);
	ListView_SetExtendedListViewStyle(mListCtrl,
		ListView_GetExtendedListViewStyle(mListCtrl) | LVS_EX_CHECKBOXES);

	// Add the materials to the list control
	fillMtlList();

	// Calculate the bottom of the list control, relative to
	// the bottom of the dialog.
	RECT ctrl, wdw;
	GetWindowRect(mWnd, &wdw);
	GetWindowRect(mListCtrl, &ctrl);
	mBottom = wdw.bottom - ctrl.bottom;

	if (mMaterials.size() != 0) {
		// Look through all objects, including XREFs for ambient only lights
		BOOL xrefs = mIp.GetIncludeXRefsInHierarchy();
		mIp.SetIncludeXRefsInHierarchy(true);

		mError = findAmbientOnlyLight(mIp.GetRootNode(), mIp.GetTime())
			? kErrorAmbientOnlyLight : 0;

		mIp.SetIncludeXRefsInHierarchy(xrefs);
	
		// Look at the ambient value in the environment
		Interval valid(FOREVER);
		const Interval forever(FOREVER);
		Point3 amb = mIp.GetAmbient(0, valid);
		if (amb.x != 0 || amb.y != 0 || amb.z != 0)
			mError |= kErrorAmbientInEnvironment;	// not-zero, this is an error
		else if (!(valid == forever)) {
			// The value is 0 but the validity isn't forever. There is a problem
			// somewhere that causes a validity of (0,0) at time 0 sometimes.
			// This is an effort to get around this. We try again one past
			// the validity interval unless it is the end of time, then we use
			// one before.
			TimeValue next = forever.End() != valid.End() ? valid.End() + 1 : valid.Start() - 1;
			valid = FOREVER;
			amb = mIp.GetAmbient(next, valid);
			if (amb.x != 0 || amb.y != 0 || amb.z != 0 || !(valid == forever))
				mError |= kErrorAmbientInEnvironment;	// Either not-zero, or animated, error
		}

		// Display the appropriate error message
		SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(
			msgs[mError & (kErrorAmbientOnlyLight | kErrorAmbientInEnvironment)]));
	}

	// Position the window at saved location
	positionWindow();
	return true;
}

void FixAmbient::FixUpDialog::fillMtlList()
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

void FixAmbient::FixUpDialog::appendMaterialToList(MaterialSet::iterator p)
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

bool FixAmbient::FixUpDialog::lessThanMtl(
	Mtl* m1,
	Mtl* m2
) const
{
	// Compare two material name and types. First weed out
	// NULL materials.
	if (m1 == m2)
		return 0;
	if (m1 == NULL)
		return false;
	if (m2 == NULL)
		return true;

	// Compare the name, if they are different, compare the type
	int r = _tcsicmp(m1->GetName().data(), m2->GetName().data());
	if (r != 0)
		return r < 0;

	Class_ID i1 = m1->ClassID();
	Class_ID i2 = m2->ClassID();

	// Don't need to look up the string if the classes are the same.
	if (i1 == i2)
		return false;

	// Compare the material type strings
	return _tcsicmp(getString(i1), getString(i2)) < 0;
}

TCHAR* FixAmbient::FixUpDialog::getString(const Class_ID& id) const
{
	// Find the class in the type name cache.
	IDMap::iterator p = mIDMap.find(id);

	if (p == mIDMap.end()) {
		// Name not found. Add it to the cache.
		p = mIDMap.insert(IDMap::value_type(id,
			mClassDir.FindClassEntry(MATERIAL_CLASS_ID, id))).first;
	}

	// Return the name. If p->second is NULL this class is not present.
	return p->second == NULL ? _T("") : p->second->ClassName().data();
}

void FixAmbient::FixUpDialog::getMtlString(Mtl* m, TSTR& entry)
{
	// Formate the name material-name (material-type)
	entry.Resize(0);
	if (m == NULL)
		return;
	entry = m->GetName();
	entry += " (";
	entry += getString(m->ClassID());
	entry += ")";
}

bool FixAmbient::FixUpDialog::resizeDialog(LPWINDOWPOS pos)
{
	// Resize the dialog. If the size isn't changing return.
	if (pos->flags & SWP_NOSIZE)
		return true;

	RECT wdw;

	// Calculate the change in width and height
	GetWindowRect(mWnd, &wdw);
	int dx = pos->cx - (wdw.right - wdw.left);
	int dy = pos->cy - (wdw.bottom - wdw.top);

	if (dx != 0 || dy != 0) {
		// Something changed. First we will calculate the
		// size of the list control. We need to be a little careful
		// because windows store negative widths and heights
		// as zero. The height can go negative, so we use the
		// distance from the bottom to calculate the control height.
		RECT client;
		GetWindowRect(mListCtrl, &client);
		SetWindowPos(mListCtrl, NULL, 0, 0,
			client.right - client.left + dx, 
			wdw.top + pos->cy - mBottom - client.top,
			SWP_NOMOVE | SWP_NOZORDER);

		// Now the size of the static message box. The height is
		// fixed, but the width can change.
		HWND msg = GetDlgItem(mWnd, IDC_FIX_MESSAGE);
		GetWindowRect(msg, &client);
		SetWindowPos(msg, NULL, 0, 0,
			client.right - client.left + dx,
			client.bottom - client.top,
			SWP_NOMOVE | SWP_NOZORDER);
		InvalidateRect(msg, NULL, true);	// Invalidate the control

		// Move the cancel and ok buttons to the lower right corner
		moveControl(IDCANCEL, dx, dy);
		moveControl(IDOK, dx, dy);
	}

	return true;
}

void FixAmbient::FixUpDialog::moveControl(int id, int dx, int dy)
{
	// Move the control by dx and dy.
	HWND hCtl = GetDlgItem(mWnd, id);
	RECT rect;

	InvalidateRect(hCtl, NULL, false);
	GetWindowRect(hCtl, &rect);
	ScreenToClient(mWnd, reinterpret_cast<LPPOINT>(&rect));

	SetWindowPos(hCtl, NULL, rect.left + dx, rect.top + dy, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER);
}

// These are used to get the last window position from
// the plugin configuration file.
static const TCHAR kWindowPosition[] = _T("WindowPosition");
static const TCHAR kStored[] = _T("Stored");
static const TCHAR kLeft[] = _T("Left");
static const TCHAR kTop[] = _T("Top");
static const TCHAR kWidth[] = _T("Width");
static const TCHAR kHeight[] = _T("Height");

void FixAmbient::FixUpDialog::positionWindow()
{
	// Position the window to the last place it was at.
	getConfigFileName();
	TCHAR* fn = mConfig.data();

	// Get the current position and size of the dialog
	RECT dlgRect;
	GetWindowRect(mWnd, &dlgRect);
	dlgRect.right -= dlgRect.left;
	dlgRect.bottom -= dlgRect.top;

	// Get the display rect to make sure we don't move
	// the dialog off of the screen.
	RECT displayRect;
	GetWindowRect(GetDesktopWindow(), &displayRect);

	// If we have stored the window position previously, get it.
	if (GetPrivateProfileInt(kWindowPosition, kStored, 0, fn)) {
		dlgRect.left = GetPrivateProfileInt(kWindowPosition, kLeft, dlgRect.left, fn);
		dlgRect.top = GetPrivateProfileInt(kWindowPosition, kTop, dlgRect.top, fn);
		dlgRect.right = GetPrivateProfileInt(kWindowPosition, kWidth, dlgRect.right, fn);
		dlgRect.bottom = GetPrivateProfileInt(kWindowPosition, kHeight, dlgRect.bottom, fn);
	}
	else {
		HWND owner = GetWindow(mWnd, GW_OWNER);
		RECT ownerRect;
		if (!IsWindow(owner)) {
			ownerRect = displayRect;
		}
		else {
			GetWindowRect(owner, &ownerRect);
		}

		// Center dialog over window
		dlgRect.left = ((ownerRect.right - ownerRect.left) - dlgRect.right) / 2;
		dlgRect.top = ((ownerRect.bottom - ownerRect.top) - dlgRect.bottom) / 2;
	}

	// Make sure the dialog is on the display.
	if (dlgRect.left > displayRect.right - dlgRect.right)
		dlgRect.left = displayRect.right - dlgRect.right;
	if (dlgRect.left < 0)
		dlgRect.left = 0;

	if (dlgRect.top > displayRect.bottom - dlgRect.bottom)
		dlgRect.top = displayRect.bottom - dlgRect.bottom;
	if (dlgRect.top < 0)
		dlgRect.top = 0;

	// Position the window.
	MoveWindow(mWnd, dlgRect.left, dlgRect.top, dlgRect.right, dlgRect.bottom, true);
}

void FixAmbient::FixUpDialog::getConfigFileName()
{
	// If the configuration file name hasn't been calculated, do it now.
	if (mConfig.isNull()) {
		mConfig = mIp.GetDir(APP_PLUGCFG_DIR);
		mConfig += _T("\\FixAmbient.ini");
	}
}

void FixAmbient::FixUpDialog::saveWindowPos()
{
	// Save the window position in the configuration file.
	getConfigFileName();
	TCHAR* fn = mConfig.data();

	RECT dlgRect;
	GetWindowRect(mWnd, &dlgRect);
	dlgRect.right -= dlgRect.left;
	dlgRect.bottom -= dlgRect.top;

	TCHAR buf[64];
	WritePrivateProfileString(kWindowPosition, kStored, "1", fn);
	_stprintf(buf, _T("%d"), dlgRect.left);
	WritePrivateProfileString(kWindowPosition, kLeft, buf, fn);
	_stprintf(buf, _T("%d"), dlgRect.top);
	WritePrivateProfileString(kWindowPosition, kTop, buf, fn);
	_stprintf(buf, _T("%d"), dlgRect.right);
	WritePrivateProfileString(kWindowPosition, kWidth, buf, fn);
	_stprintf(buf, _T("%d"), dlgRect.bottom);
	WritePrivateProfileString(kWindowPosition, kHeight, buf, fn);
}
