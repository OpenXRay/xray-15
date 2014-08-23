
/**********************************************************************
 *<
	FILE: editpat.cpp

	DESCRIPTION:  Edit Patch OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 23 June, 1995

	IMPORTANT USAGE NOTE:

		When you do an operation in edit patch which will change the topology, the form
		of the code should look like this code, taken from the vertex deletion:

		-----

			ip->GetModContexts(mcList,nodes);
			ClearPatchDataFlag(mcList,EPD_BEENDONE);

			theHold.Begin();
		-->	RecordTopologyTags();
			for ( int i = 0; i < mcList.Count(); i++ ) {
				int altered = 0;
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;
				if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

				// If the mesh isn't yet cache, this will cause it to get cached.
				PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
				if(!patch)
					continue;

				// If this is the first edit, then the delta arrays will be allocated
				patchData->BeginEdit(t);

				// If any bits are set in the selection set, let's DO IT!!
				if(patch->vertSel.NumberSet()) {
					altered = holdNeeded = 1;
		-->			patchData->PreUpdateChanges(patch);
					if ( theHold.Holding() )
						theHold.Put(new PatchRestore(patchData,this,patch,"DoVertDelete"));
					// Call the vertex delete function
					DeleteSelVerts(patch);
		-->			patchData->UpdateChanges(patch);
					patchData->TempData(this)->Invalidate(PART_TOPO);
					}
				patchData->SetFlag(EPD_BEENDONE,TRUE);
				}
			
			if(holdNeeded) {
		-->		ResolveTopoChanges();
				theHold.Accept(GetString(IDS_TH_VERTDELETE));
				}
			else {
				ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
				theHold.End();
				}
			
			nodes.DisposeTemporary();
			ClearPatchDataFlag(mcList,EPD_BEENDONE);

		-----

		The key elements in the "changed topology" case are the calls noted by arrows.
		These record special tags inside the object so that after the topology is changed
		by the modifier code, the UpdateChanges code can make a new mapping from the old
		object topology to the new.

		If the operation doesn't change the topology, then the RecordTopologyTags and
		ResolveTopoChanges calls aren't needed and the PreUpdateChanges/UpdateChanges calls
		become:

			patchData->PreUpdateChanges(patch, FALSE);
			patchData->UpdateChanges(patch, FALSE);

		This tells UpdateChanges not to bother remapping the topology.

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "dllmain.h"

#include "editpat.h"
#include "decomp.h"
#include "MaxIcon.h"
#include "resource.h"
#include "resourceOverride.h"
#include "splshape.h"

#ifndef NO_PATCHES

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx

#define DBG_NAMEDSELSx

// Special message to refresh rollup dialog values that are grabbed from the object
// being edited on the first ::ModifyObject call
#define REFRESH_EP_VALUES (WM_USER + 1)
#define REFRESH_EP_GENSURF (WM_USER + 2)		// CAL-05/01/03: refresh spline surface group (FID #1914)

// Uncomment this for vert mapper debugging
//#define VMAP_DEBUG 1

// Forward references
static float AffectRegionFunct( float dist, float falloff, float pinch, float bubble);
INT_PTR CALLBACK PatchSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PatchOpsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PatchObjSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PatchSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PatchVertSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK SoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

// forward declarations
//
// HACK!!!!
//
// These should be member functions / member variables, but I'm putting
// this fix in after the SDK freeze for Magma.  After Magma is shipped, these
// should become members.
//
static void hackInvalidateSelUI(HWND hSelPanel);
static void hackInvalidateOpsUI(HWND hOpsPanel);
static void hackInvalidateSurfUI(HWND hOpsPanel);
static BOOL s_bHackSelUIValid = FALSE;
static BOOL s_bHackOpsUIValid = FALSE;
static BOOL s_bHackSurfUIValid = FALSE;

// draws a dotted line
static void PatchXORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 );

// A handy zero point
static Point3 zeroPoint(0,0,0);

// Our temporary prompts last 2 seconds:
#define PROMPT_TIME 2000

// in mods.cpp
extern HINSTANCE hInstance;

// Select by material parameters
static int sbmParams[2]     = {1,1};

// Select by smooth parameters
static DWORD sbsParams[3]   = {1,1,0};

// Vertex Color Stuff...
static int selDeltaR		= 10;
static int selDeltaG		= 10;
static int selDeltaB		= 10;
static Point3 selByColor    = Point3(1,1,1);
#define SEL_BY_COLOR 0
#define SEL_BY_ILLUM 1
static int selBy			= SEL_BY_COLOR;

HWND				EditPatchMod::hSelectPanel    = NULL;
HWND				EditPatchMod::hOpsPanel       = NULL;
HWND				EditPatchMod::hSurfPanel      = NULL;
HWND				EditPatchMod::hSoftSelPanel   = NULL;
BOOL				EditPatchMod::rsSel           = TRUE;
BOOL				EditPatchMod::rsOps           = TRUE;
BOOL				EditPatchMod::rsSurf          = TRUE;
BOOL				EditPatchMod::rsSoftSel       = FALSE;
IObjParam*          EditPatchMod::ip              = NULL;
EditPatchMod*       EditPatchMod::editMod         = NULL;
MoveModBoxCMode*    EditPatchMod::moveMode        = NULL;
RotateModBoxCMode*  EditPatchMod::rotMode 	      = NULL;
UScaleModBoxCMode*  EditPatchMod::uscaleMode      = NULL;
NUScaleModBoxCMode* EditPatchMod::nuscaleMode     = NULL;
SquashModBoxCMode *	EditPatchMod::squashMode      = NULL;
SelectModBoxCMode*  EditPatchMod::selectMode      = NULL;
ISpinnerControl*	EditPatchMod::weldSpin        = NULL;
ISpinnerControl*	EditPatchMod::vertWeldSpin    = NULL;
ISpinnerControl*	EditPatchMod::genThreshSpin   = NULL;
ISpinnerControl*	EditPatchMod::stepsSpin       = NULL;

#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
ISpinnerControl*	EditPatchMod::stepsRenderSpin       = NULL;
#endif // NO_OUTPUTRENDERER
// 7/20/00 TH -- Relax controls
ISpinnerControl*	EditPatchMod::relaxSpin		= NULL;
ISpinnerControl*	EditPatchMod::relaxIterSpin	= NULL;

BOOL				EditPatchMod::settingViewportTess = FALSE;
BOOL				EditPatchMod::settingDisp     = FALSE;
ISpinnerControl*	EditPatchMod::uSpin           = NULL;
ISpinnerControl*	EditPatchMod::vSpin           = NULL;
ISpinnerControl*	EditPatchMod::edgeSpin        = NULL;
ISpinnerControl*	EditPatchMod::distSpin        = NULL;
ISpinnerControl*	EditPatchMod::angSpin         = NULL;
ISpinnerControl*	EditPatchMod::mergeSpin       = NULL;
ISpinnerControl*	EditPatchMod::matSpin         = NULL;
ISpinnerControl*	EditPatchMod::matSpinSel      = NULL;
BOOL				EditPatchMod::patchUIValid    = TRUE;
PickPatchAttach		EditPatchMod::pickCB;
int					EditPatchMod::condenseMat     = true;
int					EditPatchMod::attachMat       = ATTACHMAT_IDTOMAT;
int					EditPatchMod::pickBoxSize	= DEF_PICKBOX_SIZE;
int					EditPatchMod::weldBoxSize	= DEF_PICKBOX_SIZE;

static float weldThreshold = 0.1f;

// Checkbox items for rollup pages
static int lockedHandles = 0;
static BOOL byVertex = FALSE;
static BOOL inPatchCreate = FALSE;
static int patchDetachCopy = 0;
static int patchDetachReorient = 0;
static int attachReorient = 0;
static BOOL filterVerts = TRUE;
static BOOL filterVecs = TRUE;
static BOOL ignoreBackfacing = FALSE;

static int MaybeIgnoreBackfacing() {
	return ignoreBackfacing ? SUBHIT_PATCH_IGNORE_BACKFACING : 0;
	}

EPM_BindCMode*			EditPatchMod::bindMode   = NULL;
EPM_ExtrudeCMode*		EditPatchMod::extrudeMode   = NULL;
EPM_BevelCMode*			EditPatchMod::bevelMode   = NULL;
EPM_NormalFlipCMode*	EditPatchMod::normalFlipMode   = NULL;

// create command mode
EPM_CreateVertCMode*	EditPatchMod::createVertMode = NULL;
EPM_CreatePatchCMode*	EditPatchMod::createPatchMode = NULL;

EPM_VertWeldCMode*		EditPatchMod::vertWeldMode = NULL;

// CAL-06/02/03: add copy/paste tangent modes. (FID #827)
EPM_CopyTangentCMode*	EditPatchMod::copyTangentMode = NULL;
EPM_PasteTangentCMode*	EditPatchMod::pasteTangentMode = NULL;

static GenSubObjType SOT_Handle(39);
static GenSubObjType SOT_Vertex(6);
static GenSubObjType SOT_Edge(7);
static GenSubObjType SOT_Patch(8);
static GenSubObjType SOT_Element(5);

static void SetVertFilter() {
	patchHitLevel[EP_VERTEX] = (filterVerts ? SUBHIT_PATCH_VERTS : 0) | (filterVecs ? SUBHIT_PATCH_VECS : 0);
	}

// Handy temporary integer table

class TempIntTab {
	public:
		int count;
		int *tab;
		TempIntTab(int ct) { tab = new int [ct]; count=ct; }
		~TempIntTab() { delete [] tab; }
		int &operator[](int x) { assert(x>=0 && x<count); return tab[x]; }
		operator int*() { return tab; }
	};

// Handy IntTab copier

static void CopyIntTab(IntTab &to, const IntTab &from) {
	to.Delete(0, to.Count());
	int pcount = from.Count();
	to.Resize(pcount);
	for(int i = 0; i < pcount; ++i)
		to.Append(1, &from[i]);
	}

// This is a special override value which allows us to hit-test on
// any sub-part of a patch

int patchHitOverride = 0;	// If zero, no override is done

void SetPatchHitOverride(int value) {
	patchHitOverride = value;
	}

void ClearPatchHitOverride() {
	patchHitOverride = 0;
	}

/*-------------------------------------------------------------------*/

static HIMAGELIST hFaceImages = NULL;
static void LoadImages() {
	if (hFaceImages) return;

	HBITMAP hBitmap, hMask;
	hFaceImages = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
	hBitmap     = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PATCHSELTYPES));
	hMask       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PATCHSELMASK));
	ImageList_Add(hFaceImages,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

class EPImageListDestroyer {
	~EPImageListDestroyer() {
		if(hFaceImages)
			ImageList_Destroy(hFaceImages);
		}
	};

/*-------------------------------------------------------------------*/

class PatchDeleteUser : public EventUser {
	private:
		EditPatchMod *ep;
	public:
		void Notify() { ep->DoDeleteSelected(); }
		void SetMod(EditPatchMod *ep) { this->ep = ep; }
	};

static PatchDeleteUser pDel;

// Support for dynamic quad menus
class PatchDynamicMenuCallback: public DynamicMenuCallback {
public:
    void MenuItemSelected(int itemId);
    void SetMod(EditPatchMod *ep) { this->ep = ep; }
    BOOL GetLinearMapped() { return linearMapped; }
    void SetLinearMapped(BOOL lm) { linearMapped = lm; }

private:
    EditPatchMod *ep;
    BOOL linearMapped;
};

void
PatchDynamicMenuCallback::MenuItemSelected(int id)
{
	switch(ep->GetSubobjectType()) {
		case EP_VERTEX:
			ep->SetRememberedVertType((int)id);
			break;
		case EP_PATCH:
			if (id==3)
				{
				if (linearMapped)
					ep->ChangeMappingTypeLinear(FALSE);
				else ep->ChangeMappingTypeLinear(TRUE);
				}
			else ep->SetRememberedPatchType((int)id);

			break;
		}
}

static PatchDynamicMenuCallback sMenuCallback;

class PatchActionCallback: public ActionCallback {
public:
    IMenu* GetDynamicMenu(int id, HWND hwnd, IPoint2& m);
    void SetMod(EditPatchMod *ep) { this->ep = ep; }

private:
    EditPatchMod *ep;
    BOOL linearMapped;
};

IMenu* 
PatchActionCallback::GetDynamicMenu(int id, HWND hWnd, IPoint2& m)
{
    sMenuCallback.SetMod(ep);
    DynamicMenu menu(&sMenuCallback);

	switch(ep->GetSubobjectType()) {
		case EP_VERTEX:
			if(ep->RememberVertThere(hWnd, m)) {
				int oldType = -1;
				int flags0, flags1, flags2;
				flags0 = flags1 = flags2 = 0;
				switch(ep->rememberedData) {
					case PVERT_COPLANAR:
						flags1 |= DynamicMenu::kChecked;
						break;
					case 0:
						flags2 |= DynamicMenu::kChecked;
						break;
					}
				// CAL-04/28/03: add reset tangent. (FID #827)
				menu.AddItem(flags0, PVERT_RESET, GetString(IDS_TH_RESET_TANGENTS));
				menu.AddItem(flags1, PVERT_COPLANAR, GetString(IDS_TH_COPLANAR));
				menu.AddItem(flags2, 0, GetString(IDS_TH_CORNER));
				menu.AddItem( DynamicMenu::kSeparator, 0, NULL);
				}
			break;
		case EP_PATCH:
			if(ep->RememberPatchThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = 0;
				switch(ep->rememberedData) {
					case PATCH_AUTO:
						flags1 |= DynamicMenu::kChecked;
						break;
					case 0:
						flags2 |= DynamicMenu::kChecked;
						break;
					}
				menu.AddItem(flags1, PATCH_AUTO, GetString(IDS_TH_AUTOINTERIOR));
				menu.AddItem(flags2, 0, GetString(IDS_TH_MANUALINTERIOR));
				}
//watje new patch mapping
			int flags3;
			flags3 = 0;
			linearMapped = ep->CheckMappingTypeLinear();
            sMenuCallback.SetLinearMapped(linearMapped);
			if (linearMapped)
				flags3 |= DynamicMenu::kChecked;
			menu.AddItem(flags3, 3, GetString(IDS_PW_LINEARMAPPING));
            menu.AddItem( DynamicMenu::kSeparator, 0, NULL);

			break;
		}

    return menu.GetMenu();
}

static PatchActionCallback sActionCallback;

class PatchRightMenu : public RightClickMenu {
	private:
		EditPatchMod *ep;
//watje new patch mapping
		BOOL linearMapped;

	public:
		void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
		void Selected(UINT id);
		void SetMod(EditPatchMod *ep) { this->ep = ep; }
	};

void PatchRightMenu::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) {
	switch(ep->GetSubobjectType()) {
		case EP_VERTEX:
			if(ep->RememberVertThere(hWnd, m)) {
				int oldType = -1;
				int flags0, flags1, flags2;
				flags0 = flags1 = flags2 = MF_STRING;
				switch(ep->rememberedData) {
					case PVERT_COPLANAR:
						flags1 |= MF_CHECKED;
						break;
					case 0:
						flags2 |= MF_CHECKED;
						break;
					}
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				// CAL-04/28/03: add reset tangent. (FID #827)
				manager->AddMenu(this, flags0, PVERT_RESET, GetString(IDS_TH_RESET_TANGENTS));
				manager->AddMenu(this, flags1, PVERT_COPLANAR, GetString(IDS_TH_COPLANAR));
				manager->AddMenu(this, flags2, 0, GetString(IDS_TH_CORNER));
				}
			break;
		case EP_PATCH:
			if(ep->RememberPatchThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = MF_STRING;
				switch(ep->rememberedData) {
					case PATCH_AUTO:
						flags1 |= MF_CHECKED;
						break;
					case 0:
						flags2 |= MF_CHECKED;
						break;
					}
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				manager->AddMenu(this, flags1, PATCH_AUTO, GetString(IDS_TH_AUTOINTERIOR));
				manager->AddMenu(this, flags2, 0, GetString(IDS_TH_MANUALINTERIOR));
				}
//watje new patch mapping
			int flags3;
			flags3 = MF_STRING;
			linearMapped = ep->CheckMappingTypeLinear();
			if (linearMapped)
				flags3 |= MF_CHECKED;
			manager->AddMenu(this, flags3, 3, GetString(IDS_PW_LINEARMAPPING));

			break;
		}
	}

void PatchRightMenu::Selected(UINT id) {
	switch(ep->GetSubobjectType()) {
		case EP_VERTEX:
			ep->SetRememberedVertType((int)id);
			break;
		case EP_PATCH:
			if (id==3)
				{
				if (linearMapped)
					ep->ChangeMappingTypeLinear(FALSE);
				else ep->ChangeMappingTypeLinear(TRUE);
				}
			else ep->SetRememberedPatchType((int)id);

			break;
		}
	}

PatchRightMenu pMenu;

/*-------------------------------------------------------------------*/

static
BOOL IsCompatible(BitArray &a, BitArray &b) {
	return (a.GetSize() == b.GetSize()) ? TRUE : FALSE;
	}


//--- Named Selection Set Methods ------------------------------------

// Used by EditPatchMod destructor to free pointers
void EditPatchMod::ClearSetNames()
	{
	for (int i=0; i<EP_NS_LEVELS; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
			}
		}
	}

int EditPatchMod::FindSet(TSTR &setName,int level)
	{	
	assert(level>0 && level<6);
	level = namedSetLevel[level];
	for (int i=0; i<namedSel[level].Count(); i++) {
		if (setName == *namedSel[level][i]) {
			return i;			
			}
		}
	return -1;
	}

void EditPatchMod::AddSet(TSTR &setName,int level)
	{
	assert(level>0 && level<6);
	level = namedSetLevel[level];
	TSTR *name = new TSTR(setName);
	namedSel[level].Append(1,&name);
	}

void EditPatchMod::RemoveSet(TSTR &setName,int level)
	{
	MaybeFixupNamedSels();
	int i = FindSet(setName,level);
	if (i>=0) {
		assert(level>0 && level<6);
		level = namedSetLevel[level];
		delete namedSel[level][i];
		namedSel[level].Delete(i,1);
		}
	}

static void AssignSetMatchSize(BitArray &dst, BitArray &src)
	{
	int size = dst.GetSize();
	dst = src;
	if (dst.GetSize() != size) {
		dst.SetSize(size,TRUE);
		}
	}

void EditPatchMod::ActivateSubSelSet(TSTR &setName)
	{
	MaybeFixupNamedSels();
	ModContextList mcList;
	INodeTab nodes;
	int index = FindSet(setName,GetSubobjectType());
	if (index<0 || !ip) return;	

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;
		patchData->BeginEdit(ip->GetTime());
		// If that set exists in this context, deal with it
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		BitArray *set = sel.GetSet(setName);
		if(set) {
			patchData->PreUpdateChanges(patch, FALSE);
			if (theHold.Holding())
				theHold.Put(new PatchSelRestore(patchData,this,patch));
			BitArray *psel = GetLevelSelectionSet(patch);	// Get the appropriate selection set
			AssignSetMatchSize(*psel, *set);				
			PatchSelChanged();
			patchData->UpdateChanges(patch, FALSE);
			}

		if (patchData->tempData)
			patchData->TempData(this)->Invalidate(PART_SELECT);
		}
	
	theHold.Accept(GetString(IDS_DS_SELECT));
	nodes.DisposeTemporary();	
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

// Named Selection restore for PatchObject

class EP_NSRestore : public RestoreObj {
	public:
		EditPatchMod *thePatch;
		EditPatchData *patchData;

		Tab<TSTR*> oldNamedSel[EP_NS_LEVELS], newNamedSel[EP_NS_LEVELS];
		GenericNamedSelSetList oldHselSet, newHselSet;		// CAL-06/10/03: (FID #1914)
		GenericNamedSelSetList oldVselSet, newVselSet;
		GenericNamedSelSetList oldEselSet, newEselSet;
		GenericNamedSelSetList oldPselSet, newPselSet;
		BOOL setNew;

		EP_NSRestore(EditPatchMod *po, EditPatchData *pd) {
			thePatch = po;			
			patchData = pd;
			for(int i = 0; i < EP_NS_LEVELS; ++i)
				oldNamedSel[i] = po->namedSel[i];
			oldHselSet = pd->hselSet;		// CAL-06/10/03: (FID #1914)
			oldVselSet = pd->vselSet;
			oldEselSet = pd->eselSet;
			oldPselSet = pd->pselSet;
			setNew = FALSE;
			}
		void Restore(int isUndo) {
			if(isUndo && !setNew) {
				for(int i = 0; i < EP_NS_LEVELS; ++i)
					newNamedSel[i] = thePatch->namedSel[i];
				newHselSet = patchData->hselSet;	// CAL-06/10/03: (FID #1914)
				newVselSet = patchData->vselSet;
				newEselSet = patchData->eselSet;
				newPselSet = patchData->pselSet;
				setNew = TRUE;
				}
			for(int i = 0; i < EP_NS_LEVELS; ++i)
				thePatch->namedSel[i] = oldNamedSel[i];
			patchData->hselSet = oldHselSet;		// CAL-06/10/03: (FID #1914)
			patchData->vselSet = oldVselSet;
			patchData->eselSet = oldEselSet;
			patchData->pselSet = oldPselSet;
			if (thePatch->ip)
				thePatch->ip->NamedSelSetListChanged();
			}
		void Redo() {
			for(int i = 0; i < EP_NS_LEVELS; ++i)
				thePatch->namedSel[i] = newNamedSel[i];
			patchData->hselSet = newHselSet;		// CAL-06/10/03: (FID #1914)
			patchData->vselSet = newVselSet;
			patchData->eselSet = newEselSet;
			patchData->pselSet = newPselSet;
			if (thePatch->ip)
				thePatch->ip->NamedSelSetListChanged();
			}
		int Size() { return 1; }
		void EndHold() { thePatch->ClearAFlag(A_HELD); }
		TSTR Description() { return TSTR(_T("EditPatch NS restore")); }
	};

void EditPatchMod::NewSetFromCurSel(TSTR &setName)
	{
	MaybeFixupNamedSels();

	ModContextList mcList;
	INodeTab nodes;	
	if (!ip) return;
	
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;
		if (theHold.Holding())
			theHold.Put(new EP_NSRestore(this, patchData));
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		BitArray *exist = sel.GetSet(setName);	
		switch (GetSubobjectType()) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE:	
				if (exist) {
					*exist = patch->vecSel;
				} else {
					patchData->hselSet.AppendSet(patch->vecSel, 0, setName);
					}
				break;

			case EP_VERTEX:	
				if (exist) {
					*exist = patch->vertSel;
				} else {
					patchData->vselSet.AppendSet(patch->vertSel, 0, setName);
					}
				break;

			case EP_PATCH:
				if (exist) {
					*exist = patch->patchSel;
				} else {
					patchData->pselSet.AppendSet(patch->patchSel, 0, setName);
					}
				break;

			case EP_EDGE:
				if (exist) {
					*exist = patch->edgeSel;
				} else {
					patchData->eselSet.AppendSet(patch->edgeSel, 0, setName);
					}
				break;
			}
		}	
	
	int index = FindSet(setName,GetSubobjectType());
	if (index<0)
		AddSet(setName,GetSubobjectType());		
	nodes.DisposeTemporary();
	}

void EditPatchMod::RemoveSubSelSet(TSTR &setName)
	{
	MaybeFixupNamedSels();

	ModContextList mcList;
	INodeTab nodes;

	if (!ip) return;	
	
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;		
		patchData->BeginEdit(ip->GetTime());
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		sel.RemoveSet(setName);
		}
	// Remove the modifier's entry
	RemoveSet(setName,GetSubobjectType());
	ip->ClearCurNamedSelSet();
	SetupNamedSelDropDown();
	nodes.DisposeTemporary();
	}

static void MaybeDisplayNamedSelSetName(IObjParam *ip, EditPatchMod *ep) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);
	int sublevel = namedSetLevel[ep->GetSubobjectType()];
	int dataSet;
	for(int set = 0; set < ep->namedSel[sublevel].Count(); ++set) {
		ep->ClearPatchDataFlag(mcList,EPD_BEENDONE);
		BOOL gotMatch = FALSE;
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if ( !patchData ) continue;
			if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
			PatchMesh *patch = patchData->TempData(ep)->GetPatch(t);
			if(!patch) continue;
			// See if this patch has the named selection set
			switch(ep->GetSubobjectType()) {
				// CAL-06/10/03: add handle sub-object mode. (FID #1914)
				case EP_HANDLE: 
					for(dataSet = 0; dataSet < patchData->hselSet.Count(); ++dataSet) {
						if(*(patchData->hselSet.names[dataSet]) == *(ep->namedSel)[sublevel][set]) {
							if(!(*patchData->hselSet.sets[set] == patch->vecSel))
								goto next_set;
							gotMatch = TRUE;
							break;
							}
						}
					break;
				case EP_VERTEX: 
					for(dataSet = 0; dataSet < patchData->vselSet.Count(); ++dataSet) {
						if(*(patchData->vselSet.names[dataSet]) == *(ep->namedSel)[sublevel][set]) {
							if(!(*patchData->vselSet.sets[set] == patch->vertSel))
								goto next_set;
							gotMatch = TRUE;
							break;
							}
						}
					break;
				case EP_EDGE:
					for(dataSet = 0; dataSet < patchData->eselSet.Count(); ++dataSet) {
						if(*(patchData->eselSet.names[dataSet]) == *(ep->namedSel)[sublevel][set]) {
							if(!(*patchData->eselSet.sets[set] == patch->edgeSel))
								goto next_set;
							gotMatch = TRUE;
							break;
							}
						}
					break;
				case EP_PATCH:
					for(dataSet = 0; dataSet < patchData->pselSet.Count(); ++dataSet) {
						if(*(patchData->pselSet.names[dataSet]) == *(ep->namedSel)[sublevel][set]) {
							if(!(*patchData->pselSet.sets[set] == patch->patchSel))
								goto next_set;
							gotMatch = TRUE;
							break;
							}
						}
					break;
				}
			patchData->SetFlag(EPD_BEENDONE,TRUE);
			}
		// If we reach here, we might have a set that matches
		if(gotMatch) {
			ip->SetCurNamedSelSet(*(ep->namedSel)[sublevel][set]);
			goto namedSelUpdated;
			}
next_set:;
		}
	// No set matches, clear the named selection
	ip->ClearCurNamedSelSet();
			

namedSelUpdated:
	nodes.DisposeTemporary();
	ep->ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}
	
void EditPatchMod::SetupNamedSelDropDown()
	{
	// Setup named selection sets	
	if (GetSubobjectType() == EP_OBJECT)
		return;
	ip->ClearSubObjectNamedSelSets();
	int sublevel = namedSetLevel[GetSubobjectType()];
	for (int i=0; i<namedSel[sublevel].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[sublevel][i]);
	MaybeDisplayNamedSelSetName(ip, this);
	}

int EditPatchMod::NumNamedSelSets() {
	if(GetSubobjectLevel() == PO_OBJECT)
		return 0;
	return namedSel[namedSetLevel[GetSubobjectType()]].Count();
	}

TSTR EditPatchMod::GetNamedSelSetName(int i) {
	return *namedSel[namedSetLevel[GetSubobjectType()]][i];
	}

class EPSelSetNameRestore : public RestoreObj {
	public:
		TSTR undo, redo;
		TSTR *target;
		EditPatchMod *mod;
		EPSelSetNameRestore(EditPatchMod *m, TSTR *t, TSTR &newName) {
			mod = m;
			undo = *t;
			target = t;
			}
		void Restore(int isUndo) {			
			if(isUndo)
				redo = *target;
			*target = undo;
			if (mod->ip)
				mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			*target = redo;
			if (mod->ip)
				mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Sel Set Name"));}
	};

void EditPatchMod::SetNamedSelSetName(int index, TSTR &newName) {
	if(!ip) return;
	MaybeFixupNamedSels();

	// First do the master name list
	int sublevel = namedSetLevel[GetSubobjectType()];
	if (theHold.Holding())
		theHold.Put(new EPSelSetNameRestore(this, namedSel[sublevel][index], newName));

	// Save the old name so we can change those in the EditPatchData
	TSTR oldName = *namedSel[sublevel][index];
	*namedSel[sublevel][index] = newName;

	ModContextList mcList;
	INodeTab nodes;
	
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (patchData)
			patchData->GetSelSet(this).RenameSet(oldName, newName);
		}
	nodes.DisposeTemporary();
	}

void EditPatchMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	MaybeFixupNamedSels();

	// First do it in the master name list
	AddSet(newName,GetSubobjectType());		
// TO DO: Undo?
	ModContextList mcList;
	INodeTab nodes;	

	if (!ip) return;
	
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData ) continue;		
		GenericNamedSelSetList &set = patchData->GetSelSet(this);
		BitArray bits = *set.GetSetByIndex(sets[0]);
		for (i=1; i<sets.Count(); i++) {
			BitArray *bit2 = set.GetSetByIndex(sets[i]);
			switch (op) {
				case NEWSET_MERGE:
					bits |= *bit2;
					break;

				case NEWSET_INTERSECTION:
					bits &= *bit2;
					break;

				case NEWSET_SUBTRACT:
					bits &= ~(*bit2);
					break;
				}
			}
		set.AppendSet(bits,0,newName);
		}	
	
	nodes.DisposeTemporary();
	}

/*-------------------------------------------------------------------*/

// CAL-07/17/03: Patch Object bool parameter restore object. (Defect #500974)
class EPBoolParamRestore : public RestoreObj {
public:
	enum BoolParamID { kShowInterior, kUsePatchNorm };

private:
	EditPatchMod *mpEditPatch;
	BOOL mUndoVal, mRedoVal;
	BoolParamID mParamID;

public:
	EPBoolParamRestore(EditPatchMod *mod, BoolParamID paramID) : mpEditPatch(mod), mParamID(paramID) {
		switch (mParamID) {
			case kShowInterior:
				mUndoVal = mpEditPatch->GetShowInterior();
				break;
			case kUsePatchNorm:
				mUndoVal = mpEditPatch->GetUsePatchNormals();
				break;
		}
	}

	void Restore(int isUndo) {
		switch (mParamID) {
			case kShowInterior:
				if (isUndo)
					mRedoVal = mpEditPatch->GetShowInterior();
				mpEditPatch->SetShowInterior(mUndoVal);
				if (mpEditPatch->hOpsPanel) CheckDlgButton(mpEditPatch->hOpsPanel, IDC_SHOW_INTERIOR_FACES, mUndoVal);
				break;
			case kUsePatchNorm:
				if (isUndo)
					mRedoVal = mpEditPatch->GetUsePatchNormals();
				mpEditPatch->SetUsePatchNormals(mUndoVal);
				if (mpEditPatch->hOpsPanel) CheckDlgButton(mpEditPatch->hOpsPanel, IDC_TRUE_PATCH_NORMALS, mUndoVal);
				break;
		}
	}

	void Redo() {
		switch (mParamID) {
			case kShowInterior:
				mpEditPatch->SetShowInterior(mRedoVal);
				if (mpEditPatch->hOpsPanel) CheckDlgButton(mpEditPatch->hOpsPanel, IDC_SHOW_INTERIOR_FACES, mRedoVal);
				break;
			case kUsePatchNorm:
				mpEditPatch->SetUsePatchNormals(mRedoVal);
				if (mpEditPatch->hOpsPanel) CheckDlgButton(mpEditPatch->hOpsPanel, IDC_TRUE_PATCH_NORMALS, mRedoVal);
				break;
		}
	}

	int Size() { return sizeof (void *) + 2*sizeof(BOOL) + sizeof(BoolParamID); }

	TSTR Description() { return TSTR(_T("Edit Patch Modifier Bool Parameter Restore")); }
};

/*-------------------------------------------------------------------*/

#define GRAPHSTEPS 20
static void DrawCurve (HWND hWnd,HDC hdc) {

	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFF_SPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCH_SPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLE_SPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	rect.left   += 3;
	rect.right  -= 3;
	rect.top    += 20;
	rect.bottom -= 20;
	
	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx(hdc,(rect.left+rect.right)/2,orect.top,NULL);
	LineTo(hdc,(rect.left+rect.right)/2,orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	
	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float dist = falloff * float(abs(i-GRAPHSTEPS/2))/float(GRAPHSTEPS/2);		
		float y = AffectRegionFunct( dist, falloff, pinch, bubble );
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
}


INT_PTR CALLBACK SoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditPatchMod *pEpm = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
			// WIN64 Cleanup: Shuler
	if ( !pEpm && message != WM_INITDIALOG ) return FALSE;

	ISpinnerControl *spin;
	Rect rect;

	switch ( message ) {
		case WM_INITDIALOG: {
		 	pEpm = (EditPatchMod *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (INT_PTR)pEpm );		 	

			CheckDlgButton( hDlg, IDC_SOFT_SELECTION, pEpm->UseSoftSelections() );
			CheckDlgButton( hDlg, IDC_EDGE_DIST, pEpm->UseEdgeDists() );
			CheckDlgButton( hDlg, IDC_AFFECT_BACKFACING, pEpm->mAffectBackface );


			SetupIntSpinner   ( hDlg, IDC_EDGE_SPIN, IDC_EDGE, 1, 999, pEpm->mEdgeDist );
			SetupUniverseSpinner ( hDlg, IDC_FALLOFF_SPIN, IDC_FALLOFF, 0.0, 9999999.0, pEpm->mFalloff );
			SetupUniverseSpinner ( hDlg, IDC_PINCH_SPIN, IDC_PINCH, -10.0, 10.0, pEpm->mPinch );
			SetupUniverseSpinner ( hDlg, IDC_BUBBLE_SPIN, IDC_BUBBLE, -10.0, 10.0, pEpm->mBubble );
		 	pEpm->SetSoftSelDlgEnables( hDlg );
			return TRUE;
			}

		case WM_DESTROY:
			return FALSE;

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;

			switch ( LOWORD(wParam) ) {

				case IDC_EDGE_SPIN: 
					pEpm->mEdgeDist = spin->GetIVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateEdgeDists();
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_FALLOFF_SPIN: 
					pEpm->mFalloff = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_PINCH_SPIN: 
					pEpm->mPinch = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_BUBBLE_SPIN: 
					pEpm->mBubble = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				}
			break;
		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				// WTF? do these things ever get hit?
				//
				case IDC_BUBBLE_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_PINCH_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_FALLOFF_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				}
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hDlg,&ps);
				DrawCurve(hDlg,hdc);
				EndPaint(hDlg,&ps);
			}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			pEpm->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {	
				// Normals
				case IDC_SOFT_SELECTION:
					{
					// use method called SetUseSoftSelections() and do all of this work there!
					//
					int useSoftSelections = IsDlgButtonChecked(hDlg, IDC_SOFT_SELECTION);
					pEpm->SetUseSoftSelections( useSoftSelections );
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
		 			pEpm->SetSoftSelDlgEnables( hDlg );
					}
					break;
				case IDC_EDGE_DIST:
					{
					int useEdgeDist = IsDlgButtonChecked(hDlg, IDC_EDGE_DIST);
					pEpm->SetUseEdgeDists( useEdgeDist );
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				case IDC_AFFECT_BACKFACING:
					pEpm->mAffectBackface = IsDlgButtonChecked(hDlg, IDC_AFFECT_BACKFACING);					
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				// CAL-05/06/03: shaded face toggle. (FID #1914)
				case IDC_SHADED_FACE_TOGGLE:
					pEpm->ToggleShadedFaces();
					break;
				}
			break;
		}
	
	return FALSE;
	}

// Named selection set copy/paste methods follow...

static INT_PTR CALLBACK PickSetNameDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static TSTR *name;

	switch (msg) {
		case WM_INITDIALOG: {
			name = (TSTR*)lParam;
			ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
			edit->SetText(*name);
			ReleaseICustEdit(edit);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
					TCHAR buf[256];
					edit->GetText(buf,256);
					*name = TSTR(buf);
					ReleaseICustEdit(edit);
					EndDialog(hWnd,1);
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		};
	return TRUE;
	}

BOOL EditPatchMod::GetUniqueSetName(TSTR &name) {
	while (1) {		
		if(FindSet(name, GetSubobjectType()) < 0)
			break;

		if (!DialogBoxParam(
			hInstance, 
			MAKEINTRESOURCE(IDD_PASTE_NAMEDSET),
			ip->GetMAXHWnd(), 
			PickSetNameDlgProc,
			(LPARAM)&name)) return FALSE;		
		}
	return TRUE;
	}

static INT_PTR CALLBACK PickSetDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{	
	switch (msg) {
		case WM_INITDIALOG:	{
			Tab<TSTR*> &names = *((Tab<TSTR*>*)lParam);
			for (int i=0; i<names.Count(); i++) {
				int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
					(LPARAM)(TCHAR*)*names[i]);
				SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
				}
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_NS_LIST:
					if (HIWORD(wParam)!=LBN_DBLCLK) break;
					// fall through
				case IDOK: {
					int sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
					if (sel!=LB_ERR) {
						int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
						EndDialog(hWnd,res);
						break;
						}
					// fall through
					}

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		};
	return TRUE;
	}

int EditPatchMod::SelectNamedSet() {
	Tab<TSTR*> names = namedSel[namedSetLevel[GetSubobjectType()]];
	return DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), 
		PickSetDlgProc,
		(LPARAM)&names);
	}

void EditPatchMod::NSCopy() {
	MaybeFixupNamedSels();
	if (GetSubobjectType() == EP_OBJECT) return;
	int index = SelectNamedSet();
	if(index < 0) return;
	if(!ip) return;
	// Get the name for that index
	int nsl = namedSetLevel[GetSubobjectType()];
	TSTR setName = *namedSel[nsl][index];
	PatchNamedSelClip *clip = new PatchNamedSelClip(setName);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;

		GenericNamedSelSetList &setList = patchData->GetSelSet(this);
		BitArray *set = setList.GetSet(setName);
		if(set) {
			BitArray *bits = new BitArray(*set);
			clip->sets.Append(1,&bits);
			}
		}
	SetPatchNamedSelClip(clip, namedClipLevel[GetSubobjectType()]);

	// Enable the paste button
	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_PASTE));
	but->Enable();
	ReleaseICustButton(but);
	}

void EditPatchMod::NSPaste() {
	MaybeFixupNamedSels();
	if (GetSubobjectType() == EP_OBJECT) return;
	int nsl = namedSetLevel[GetSubobjectType()];
	PatchNamedSelClip *clip = GetPatchNamedSelClip(namedClipLevel[GetSubobjectType()]);
	if (!clip) return;
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;
	if(!ip) return;

	ModContextList mcList;
	INodeTab nodes;

	AddSet(name, GetSubobjectType());

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;

		GenericNamedSelSetList & setList = patchData->GetSelSet(this);

		if (i>=clip->sets.Count()) {
			BitArray bits;
			setList.AppendSet(bits, 0, name);
			}
		else
			setList.AppendSet(*clip->sets[i], 0, name);
		}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	SetupNamedSelDropDown();
	}

// Old MAX files (pre-r3) have EditPatchData named selections without names assigned.  This
// assigns them their proper names for r3 and later code.  If no fixup is required, this does nothing.
void EditPatchMod::MaybeFixupNamedSels() {
	int i;
	if(!ip) return;

	// Go thru the modifier contexts, and stuff the named selection names into the EditPatchData
	ModContextList mcList;
	INodeTab nodes;
	
	ip->GetModContexts(mcList,nodes);

#ifdef DBG_NAMEDSELS
	DebugPrint("Context/named sels:\n");
	for (i=0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;		

		// Go thru each subobject level
		for(int j = 0; j < EP_NS_LEVELS; ++j) {
			GenericNamedSelSetList &pdSel = patchData->GetSelSet(j);
			for(int k = 0; k < pdSel.Count(); ++k)
				DebugPrint("Context %d, level %d, set %d: [%s]\n", i, j, k, *pdSel.names[k]);
			}
		}	
#endif //DBG_NAMEDSELS

	if(!namedSelNeedsFixup) {
#ifdef DBG_NAMEDSELS
		DebugPrint("!!! NO FIXUP REQUIRED !!!\n");
#endif //DBG_NAMEDSELS
		return;
		}

#ifdef DBG_NAMEDSELS
	DebugPrint("*** Fixing up named sels ***\n");
#endif //DBG_NAMEDSELS

	for (i=0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;		

		// Go thru each subobject level
		for(int j = 0; j < EP_NS_LEVELS; ++j) {
			Tab<TSTR*> &mSel = namedSel[j];
			GenericNamedSelSetList &pdSel = patchData->GetSelSet(j);
			// Some old files may have improper counts in the EditPatchData.  Limit the counter
			int mc = mSel.Count();
			int pdc = pdSel.Count();
			int limit = (mc < pdc) ? mc : pdc;
#ifdef DBG_NAMEDSELS
			if(mc != pdc)
				DebugPrint("****** mSel.Count=%d, pdSel.Count=%d ******\n", mc, pdc);
#endif //DBG_NAMEDSELS
			for(int k = 0; k < limit; ++k)
				*pdSel.names[k] = *mSel[k];
			}
		}	
	
	nodes.DisposeTemporary();
	namedSelNeedsFixup = FALSE;
	}

void EditPatchMod::RemoveAllSets()
	{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip) return;	
	
	ip->GetModContexts(mcList,nodes);

	for (int i=0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;		
		
		int j;
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		for (j=patchData->hselSet.Count()-1; j>=0; j--) {
			patchData->hselSet.DeleteSet(j);
			}
		for (j=patchData->vselSet.Count()-1; j>=0; j--) {
			patchData->vselSet.DeleteSet(j);
			}
		for (j=patchData->pselSet.Count()-1; j>=0; j--) {
			patchData->pselSet.DeleteSet(j);
			}
		for (j=patchData->eselSet.Count()-1; j>=0; j--) {
			patchData->eselSet.DeleteSet(j);
			}		
		}	
	
	for (int j=0; j<EP_NS_LEVELS; j++) {
		for (int i=0; i<namedSel[j].Count(); i++) {
			delete namedSel[j][i];		
			}
		namedSel[j].Resize(0);
		}

	ip->ClearCurNamedSelSet();
	ip->ClearSubObjectNamedSelSets();
	nodes.DisposeTemporary();
	}


/*-------------------------------------------------------------------*/

// Deletes any vertices tagged, also any patches tagged.  Automatically deletes the vectors that
// are deleted as a result of the patch deletion and sweeps any vertices floating in space.

static void DeletePatchParts(PatchMesh *patch, BitArray &delVerts, BitArray &delPatches, BitArray *delVecs=NULL) {
	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();
	int vecs = patch->getNumVecs();
	int dest;

	// We treat vectors specially in order to clean up after welds.  First, we tag 'em all,
	// then untag only those on unselected patches so that any dangling vectors will be deleted.
	BitArray delVectors(vecs);
	delVectors.SetAll();

	// Untag vectors that are on nondeleted patches
	for(int i = 0; i < patches; ++i) {
		if(!delPatches[i]) {
			Patch& p = patch->patches[i];
			for(int j = 0; j < (p.type * 2); ++j) {
				delVectors.Clear(p.vec[j]);
				}
			for(j = 0; j < p.type; ++j)
				delVectors.Clear(p.interior[j]);
			}
		}
	// If they want other vectors deleted, add them
	if(delVecs)
		delVectors |= *delVecs;

	// Make a table of vertices that are still in use -- Used to
	// delete those vertices which are floating, unused, in space.
	BitArray usedVerts(verts);
	usedVerts.ClearAll();
	for(i = 0; i < patches; ++i) {
		if(!delPatches[i]) {
			Patch& p = patch->patches[i];
			for(int j = 0; j < p.type; ++j) {
				usedVerts.Set(p.v[j]);
				}
			}
		}
	for(i = 0; i < verts; ++i) {
		if(!usedVerts[i])
			delVerts.Set(i);
		}

	// If we have texture vertices, handle them, too
	for(int chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
		int tverts = patch->getNumMapVerts(chan);
		if(tverts && patch->mapPatches(chan)) {
			BitArray delTVerts(tverts);
			delTVerts.SetAll();
			for(i = 0; i < patches; ++i) {
				if(!delPatches[i]) {
					Patch& p = patch->patches[i];
					TVPatch& tp = patch->mapPatches(chan)[i];
					for(int j = 0; j < p.type; ++j)
						{
						delTVerts.Clear(tp.tv[j]);
//watje 255081 to handle new TV handles and interiors 
						if (patch->ArePatchesCurvedMapped(i))
							{
							if (tp.handles[j*2]!=-1)
								delTVerts.Clear(tp.handles[j*2]);
							if (tp.handles[j*2+1]!=-1)
								delTVerts.Clear(tp.handles[j*2+1]);
							if (!(p.flags&PATCH_AUTO))
								{
								if (tp.interiors[j]!=-1)
									delTVerts.Clear(tp.interiors[j]);
								}
							}
						}
					}
				}
			// Got the list of tverts to delete -- now delete 'em
			// Build a table of redirected texture vertex indices
			int newTVerts = tverts - delTVerts.NumberSet();
			IntTab tVertIndex;
			tVertIndex.SetCount(tverts);
			PatchTVert *newTVertArray = new PatchTVert[newTVerts];
			dest = 0;
			for(i = 0; i < tverts; ++i) {
				if(!delTVerts[i]) {
					newTVertArray[dest] = patch->mapVerts(chan)[i];
					tVertIndex[i] = dest++;
				}
			}
			patch->setNumMapVerts (chan, newTVerts);
			if (newTVerts) memcpy (patch->mapVerts(chan), newTVertArray, newTVerts*sizeof (PatchTVert));
			delete[] newTVertArray;

			// Now, copy the untagged texture patches to a new array
			// While you're at it, redirect the vertex indices
			int newTVPatches = patches - delPatches.NumberSet();
			TVPatch *newArray = new TVPatch[newTVPatches];
			dest = 0;
			for(i = 0; i < patches; ++i) {
				if(!delPatches[i]) {
					Patch& p = patch->patches[i];
					TVPatch& tp = newArray[dest++];
					tp = patch->mapPatches(chan)[i];
					for(int j = 0; j < p.type; ++j)
						{
						tp.tv[j] = tVertIndex[tp.tv[j]];
//watje 255081 to handle new TV handles and interiors
						if (patch->ArePatchesCurvedMapped(i))
							{
							if (tp.handles[j*2]!=-1)
								tp.handles[j*2] = tVertIndex[tp.handles[j*2]];
							if (tp.handles[j*2+1]!=-1)
								tp.handles[j*2+1] = tVertIndex[tp.handles[j*2+1]];
							if (!(p.flags&PATCH_AUTO))
								{
								if (tp.interiors[j]!=-1)
									tp.interiors[j] = tVertIndex[tp.interiors[j]];
								}
							}

						}
					}
				}
			patch->setNumMapPatches (chan, newTVPatches);
			if (newTVPatches) memcpy (patch->mapPatches(chan), newArray, newTVPatches*sizeof(TVPatch));
			delete [] newArray;
			}
		}

	// Build a table of redirected vector indices
	IntTab vecIndex;
	vecIndex.SetCount(vecs);
	int newVectors = vecs - delVectors.NumberSet();
	PatchVec *newVecArray = new PatchVec[newVectors];
	BitArray newVecSel(newVectors);
	newVecSel.ClearAll();
	dest = 0;
	for(i = 0; i < vecs; ++i) {
		if(!delVectors[i]) {
			newVecArray[dest] = patch->vecs[i];
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			newVecSel.Set(dest, patch->vecSel[i]);
			vecIndex[i] = dest++;
			}
		else
			vecIndex[i] = -1;
		}
	delete[] patch->vecs;
	patch->vecs = newVecArray;
	patch->numVecs = newVectors;
	patch->vecSel = newVecSel;

	// Build a table of redirected vertex indices
	int newVerts = verts - delVerts.NumberSet();
	IntTab vertIndex;
	vertIndex.SetCount(verts);
	PatchVert *newVertArray = new PatchVert[newVerts];
	BitArray newVertSel(newVerts);
	newVertSel.ClearAll();
	dest = 0;
	for(i = 0; i < verts; ++i) {
		if(!delVerts[i]) {
			newVertArray[dest] = patch->verts[i];
			newVertSel.Set(dest, patch->vertSel[i]);
			// redirect & adjust attached vector list
			PatchVert& v = newVertArray[dest];
			for(int j = 0; j < v.vectors.Count(); ++j) {
				v.vectors[j] = vecIndex[v.vectors[j]];
				if(v.vectors[j] < 0) {
					v.vectors.Delete(j, 1);
					j--;	// realign index
					}
				}
			vertIndex[i] = dest++;
			}
		}
	delete[] patch->verts;
	patch->verts = newVertArray;
	patch->numVerts = newVerts;
	patch->vertSel = newVertSel;

	// Now, copy the untagged patches to a new array
	// While you're at it, redirect the vertex and vector indices
	int newPatches = patches - delPatches.NumberSet();
	Patch *newArray = new Patch[newPatches];
	BitArray newPatchSel(newPatches);
	newPatchSel.ClearAll();
	dest = 0;
	for(i = 0; i < patches; ++i) {
		if(!delPatches[i]) {
			newArray[dest] = patch->patches[i];
			Patch& p = newArray[dest];
			for(int j = 0; j < p.type; ++j)
				p.v[j] = vertIndex[p.v[j]];
			for(j = 0; j < (p.type * 2); ++j)
				p.vec[j] = vecIndex[p.vec[j]];
			for(j = 0; j < p.type; ++j)
				p.interior[j] = vecIndex[p.interior[j]];
			newPatchSel.Set(dest++, patch->patchSel[i]);
			}
		}
	delete[] patch->patches;
	patch->patches = newArray;;
	patch->numPatches = newPatches;
	patch->patchSel.SetSize(newPatches,TRUE);
	patch->patchSel = newPatchSel;
	patch->buildLinkages();
	}

/*-------------------------------------------------------------------*/

// This function checks the current command mode and resets it to CID_OBJMOVE if
// it's one of our command modes

static
void CancelEditPatchModes(IObjParam *ip) {
	switch(ip->GetCommandMode()->ID()) {
		case CID_STDPICK:
			ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		}
	}

// This gets rid of two-step modes, like booleans.  This is necessary because
// the first step, which activates the mode button, validates the selection set.
// If the selection set changes, the mode must be turned off because the new
// selection set may not be valid for the mode.
static
void Cancel2StepPatchModes(IObjParam *ip) {
//	switch(ip->GetCommandMode()->ID()) {
//		case CID_BOOLEAN:
//			ip->SetStdCommandMode( CID_OBJMOVE );
//			break;
//		}
	}

/*-------------------------------------------------------------------*/

static
TSTR detachName;

static
BOOL CALLBACK DetachDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	TCHAR tempName[256];
	switch(message) {
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_DETACH_NAME, detachName);
			SetFocus(GetDlgItem(hDlg, IDC_DETACH_NAME));
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hDlg, IDC_DETACH_NAME, tempName, 255);
					detachName = TSTR(tempName);
					EndDialog(hDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}

static
int GetDetachOptions(IObjParam *ip, TSTR& newName) {
	detachName = newName;
	ip->MakeNameUnique(detachName);	
	if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_DETACH), ip->GetMAXHWnd(), (DLGPROC)DetachDialogProc)==1) {
		newName = detachName;
		return 1;
		}
	return 0;
	}

/*-------------------------------------------------------------------*/

static EditPatchClassDesc editPatchDesc;
extern ClassDesc* GetEditPatchModDesc() { return &editPatchDesc; }

void EditPatchClassDesc::ResetClassParams(BOOL fileReset)
	{
	sbmParams[0]   = 1;
	sbmParams[1]   = 1;
	EditPatchMod::condenseMat = true;
	EditPatchMod::attachMat = ATTACHMAT_IDTOMAT;
	lockedHandles = 0;
	ignoreBackfacing = FALSE;
	byVertex = FALSE;
	inPatchCreate = FALSE;
	EditPatchMod::pickBoxSize	= DEF_PICKBOX_SIZE;
	EditPatchMod::weldBoxSize	= DEF_PICKBOX_SIZE;
	}

/*-------------------------------------------------------------------*/

PatchPointTab::PatchPointTab() {
	}

PatchPointTab::~PatchPointTab() {
	}

void PatchPointTab::Empty() {
	ptab.Delete(0, ptab.Count());
	vtab.Delete(0, vtab.Count());
	pttab.Delete(0, pttab.Count());
	}

void PatchPointTab::Zero() {
//DebugPrint("Zeroing\n");
	int points = ptab.Count();
	int vectors = vtab.Count();
	Point3 zero(0, 0, 0);

	for(int i = 0; i < points; ++i) {
		ptab[i] = zero;
		pttab[i] = 0;
		}
	for(i = 0; i < vectors; ++i)
		vtab[i] = zero;
	}

void PatchPointTab::MakeCompatible(PatchMesh& patch,int clear) {
	int izero = 0;
	if(clear) {
		ptab.Delete(0, ptab.Count());
		pttab.Delete(0, pttab.Count());
		vtab.Delete(0, vtab.Count());
		}
	// First, the verts
	int size = patch.numVerts;
	if(ptab.Count() > size) {
		int diff = ptab.Count() - size;
		ptab.Delete( ptab.Count() - diff, diff );
		pttab.Delete( pttab.Count() - diff, diff );
		}
	if(ptab.Count() < size) {
		int diff = size - ptab.Count();
		ptab.Resize( size );
		pttab.Resize( size );
		for( int j = 0; j < diff; j++ ) {
			ptab.Append(1,&zeroPoint);
			pttab.Append(1,&izero);
			}
		}
	// Now, the vectors
	size = patch.numVecs;
	if(vtab.Count() > size) {
		int diff = vtab.Count() - size;
		vtab.Delete( vtab.Count() - diff, diff );
		}
	if(vtab.Count() < size) {
		int diff = size - vtab.Count();
		vtab.Resize( size );
		for( int j = 0; j < diff; j++ )
			vtab.Append(1,&zeroPoint);
		}
	}

PatchPointTab& PatchPointTab::operator=(PatchPointTab& from) {
	ptab = from.ptab;
	vtab = from.vtab;
	pttab = from.pttab;
	return *this;
	}

BOOL PatchPointTab::IsCompatible(PatchMesh &patch) {
	if(ptab.Count() != patch.numVerts)
		return FALSE;
	if(pttab.Count() != patch.numVerts)
		return FALSE;
	if(vtab.Count() != patch.numVecs)
		return FALSE;
	return TRUE;
	}

void PatchPointTab::RescaleWorldUnits(float f) {
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	int points = ptab.Count();
	int vectors = vtab.Count();

	for(int i = 0; i < points; ++i)
		ptab[i] = ptab[i] * stm;
	for(i = 0; i < vectors; ++i)
		vtab[i] = vtab[i] * stm;
	}

#define PPT_VERT_CHUNK		0x1000
#define PPT_VEC_CHUNK		0x1010
#define PPT_VERTTYPE_CHUNK	0x1020

IOResult PatchPointTab::Save(ISave *isave) {	
	int i;
	ULONG nb;
	isave->BeginChunk(PPT_VERT_CHUNK);
	int count = ptab.Count();
	isave->Write(&count,sizeof(int),&nb);
	for(i = 0; i < count; ++i)
		isave->Write(&ptab[i],sizeof(Point3),&nb);
	isave->EndChunk();
	isave->BeginChunk(PPT_VERTTYPE_CHUNK);
	count = pttab.Count();
	isave->Write(&count,sizeof(int),&nb);
	for(i = 0; i < count; ++i)
		isave->Write(&pttab[i],sizeof(int),&nb);
	isave->EndChunk();
	isave->BeginChunk(PPT_VEC_CHUNK);
	count = vtab.Count();
	isave->Write(&count,sizeof(int),&nb);
	for(i = 0; i < count; ++i)
		isave->Write(&vtab[i],sizeof(Point3),&nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult PatchPointTab::Load(ILoad *iload) {	
	int i, count;
	Point3 workpt;
	int workint;
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PPT_VERT_CHUNK:
				ptab.Delete(0,ptab.Count());
				iload->Read(&count,sizeof(int),&nb);
				for(i = 0; i < count; ++i) {
					iload->Read(&workpt,sizeof(Point3),&nb);
					ptab.Append(1,&workpt);
					}
				break;
			case PPT_VERTTYPE_CHUNK:
				pttab.Delete(0,pttab.Count());
				iload->Read(&count,sizeof(int),&nb);
				for(i = 0; i < count; ++i) {
					iload->Read(&workint,sizeof(int),&nb);
					pttab.Append(1,&workint);
					}
				break;
			case PPT_VEC_CHUNK:
				vtab.Delete(0,vtab.Count());
				iload->Read(&count,sizeof(int),&nb);
				for(i = 0; i < count; ++i) {
					iload->Read(&workpt,sizeof(Point3),&nb);
					vtab.Append(1,&workpt);
					}
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

void PatchVertexDelta::SetSize(PatchMesh& patch, BOOL load)
	{
	dtab.MakeCompatible(patch, FALSE);
	
	// Load it if necessary
	if(load) {
		int verts = patch.numVerts;
		int vecs = patch.numVecs;
		for(int i = 0; i < verts; ++i) {
			dtab.ptab[i] = patch.verts[i].p;
			dtab.pttab[i] = patch.verts[i].flags & PVERT_COPLANAR;
			}
		for(i = 0; i < vecs; ++i)
			dtab.vtab[i] = patch.vecs[i].p;
		}
	}

void PatchVertexDelta::Apply(PatchMesh &patch)
	{
//DebugPrint(_T("PVD:Applying\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);

	// Apply the deltas
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	for(int i = 0; i < verts; ++i) {
		patch.verts[i].p += dtab.ptab[i];
		patch.verts[i].flags ^= dtab.pttab[i];
		}
	for(i = 0; i < vecs; ++i) {
		patch.vecs[i].p += dtab.vtab[i];
		}
	patch.computeInteriors();
	}

void PatchVertexDelta::UnApply(PatchMesh &patch)
	{
//DebugPrint(_T("PVD:UnApplying\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);

	// Apply the deltas
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	for(int i = 0; i < verts; ++i) {
		patch.verts[i].p -= dtab.ptab[i];
		patch.verts[i].flags ^= dtab.pttab[i];
		}
	for(i = 0; i < vecs; ++i) {
		patch.vecs[i].p -= dtab.vtab[i];
		}
	patch.computeInteriors();
	}

// This function applies the current changes to slave handles and their knots, and zeroes everything else
void PatchVertexDelta::ApplyHandlesAndZero(PatchMesh &patch, int handleVert) {
//DebugPrint(_T("PVD:ApplyAndZero\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);

	Point3 zeroPt(0.0f, 0.0f, 0.0f);

	// Apply the deltas	to just the slave handles
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	Point3Tab& delta = dtab.vtab;
	IntTab& kdelta = dtab.pttab;
	for(int i = 0; i < vecs; ++i) {
		if(!(delta[i] == zeroPt)) {
			if((handleVert < 0) ? !patch.vecSel[i] : (i != handleVert))
				patch.vecs[i].p += delta[i];
			else
				delta[i] = zeroPt;
			}
		}

	for(i = 0; i < verts; ++i) {
		if(kdelta[i])
			patch.verts[i].flags ^= kdelta[i];
		}
	}


#define PVD_POINTTAB_CHUNK		0x1000

IOResult PatchVertexDelta::Save(ISave *isave) {
	isave->BeginChunk(PVD_POINTTAB_CHUNK);
	dtab.Save(isave);
	isave->	EndChunk();
	return IO_OK;
	}

IOResult PatchVertexDelta::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PVD_POINTTAB_CHUNK:
				res = dtab.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

EPMapper::~EPMapper() {
	if(vertMap) {
		delete [] vertMap;
		vertMap = NULL;
		}
	if(vecMap) {
		delete [] vecMap;
		vecMap = NULL;
		}
	if(patchMap) {
		delete [] patchMap;
		patchMap = NULL;
		}
	if(colorMap) {
		delete [] colorMap;
		colorMap = NULL;
		}
	if(illumMap) {
		delete [] illumMap;
		illumMap = NULL;
		}
	if(alphaMap) {
		delete [] alphaMap;
		alphaMap = NULL;
		}
	}

void EPMapper::Build(PatchMesh &patch) {
	// Init vertex storage
	verts = patch.numVerts;
	if(vertMap)
		delete [] vertMap;
	vertMap = new EPMapVert[verts];
	for(int i = 0; i < verts; ++i)
		vertMap[i] = EPMapVert(i, zeroPoint);
	// Init vector storage
	vecs = patch.numVecs;
	if(vecMap)
		delete [] vecMap;
	vecMap = new EPMapVert[vecs];
	for(i = 0; i < vecs; ++i)
		vecMap[i] = EPMapVert(i, zeroPoint);
	// Init patch storage
	patches = patch.numPatches;
	if(patchMap)
		delete [] patchMap;
	patchMap = new EPMapPatch[patches];
	for(i = 0; i < patches; ++i)
		patchMap[i] = EPMapPatch(i);
	// Init color storage
	colors = patch.getNumMapVerts(0);
	if(colorMap)
		delete [] colorMap;
	colorMap = new EPMapUVVert[colors];
	for(i = 0; i < colors; ++i)
		colorMap[i] = EPMapUVVert(i);
	// Init illum storage
	illums = patch.getNumMapVerts(MAP_SHADING);
	if(illumMap)
		delete [] illumMap;
	illumMap = new EPMapUVVert[illums];
	for(i = 0; i < illums; ++i)
		illumMap[i] = EPMapUVVert(i);
	// Init alpha storage
	alphas = patch.getNumMapVerts(MAP_ALPHA);
	if(alphaMap)
		delete [] alphaMap;
	alphaMap = new EPMapUVVert[alphas];
	for(i = 0; i < alphas; ++i)
		alphaMap[i] = EPMapUVVert(i);
	}

void EPMapper::RecordTopologyTags(PatchMesh &patch) {
	// First, stuff all -1's into aux fields
	for(int i = 0; i < patch.numVerts; ++i)
		patch.verts[i].aux1 = 0xffffffff;
	for(i = 0; i < patch.numVecs; ++i)
		patch.vecs[i].aux1 = 0xffffffff;
	for(i = 0; i < patch.numPatches; ++i)
		patch.patches[i].aux1 = 0xffffffff;
	// Colors
	for(i = 0; i < patch.getNumMapVerts(0); ++i)
		patch.mapVerts(0)[i].aux1 = 0xffffffff;
	// Illums
	for(i = 0; i < patch.getNumMapVerts(MAP_SHADING); ++i)
		patch.mapVerts(MAP_SHADING)[i].aux1 = 0xffffffff;
	// Alphas
	for(i = 0; i < patch.getNumMapVerts(MAP_ALPHA); ++i)
		patch.mapVerts(MAP_ALPHA)[i].aux1 = 0xffffffff;
	for(i = 0; i < verts; ++i) {
		// If it's still mapped, record it!
		if(vertMap[i].vert >= 0)
			patch.verts[vertMap[i].vert].aux1 = i;
		}
	for(i = 0; i < vecs; ++i) {
		// If it's still mapped, record it!
		if(vecMap[i].vert >= 0)
			patch.vecs[vecMap[i].vert].aux1 = i;
		}
	for(i = 0; i < patches; ++i) {
		// If it's still mapped, record it!
		if(patchMap[i].patch >= 0)
			patch.patches[patchMap[i].patch].aux1 = i;
		}
	for(i = 0; i < colors; ++i) {
		// If it's still mapped, record it!
		if(colorMap[i].vert >= 0)
			patch.mapVerts(0)[colorMap[i].vert].aux1 = i;
		}
	for(i = 0; i < illums; ++i) {
		// If it's still mapped, record it!
		if(illumMap[i].vert >= 0)
			patch.mapVerts(MAP_SHADING)[illumMap[i].vert].aux1 = i;
		}
	for(i = 0; i < alphas; ++i) {
		// If it's still mapped, record it!
		if(alphaMap[i].vert >= 0)
			patch.mapVerts(MAP_ALPHA)[alphaMap[i].vert].aux1 = i;
		}
	}

// Patch part comparison functions
static BOOL PatchVertChanged(PatchVert &o, PatchVert &n) {
	if(o.p != n.p)
		return TRUE;
	if(o.flags != n.flags)
		return TRUE;
	return FALSE;
	}

static BOOL PatchTVertChanged(PatchTVert &o, PatchTVert &n) {
	if(o.p != n.p)
		return TRUE;
	return FALSE;
	}

static BOOL PatchVecChanged(PatchVec &o, PatchVec &n) {
	if(o.p != n.p)
		return TRUE;
	return FALSE;
	}

static BOOL PatchPatchChanged(Patch &o, Patch &n) {
	if(o.smGroup != n.smGroup)
		return TRUE;
	if(o.flags != n.flags)
		return TRUE;
	return FALSE;
	}

void EPMapper::RecomputeDeltas(BOOL checkTopology, PatchMesh &patch, PatchMesh &oldPatch) {
	// Hang on to the original indexes
	TempIntTab prevVert(verts);
	TempIntTab prevVec(vecs);
	TempIntTab prevPatch(patches);
	TempIntTab prevColor(colors);
	TempIntTab prevIllum(illums);
	TempIntTab prevAlpha(alphas);
	for(int i = 0; i < verts; ++i)
		prevVert[i] = vertMap[i].vert;
	for(i = 0; i < vecs; ++i)
		prevVec[i] = vecMap[i].vert;
	for(i = 0; i < patches; ++i)
		prevPatch[i] = patchMap[i].patch;
	for(i = 0; i < colors; ++i)
		prevColor[i] = colorMap[i].vert;
	for(i = 0; i < illums; ++i)
		prevIllum[i] = illumMap[i].vert;
	for(i = 0; i < alphas; ++i)
		prevAlpha[i] = alphaMap[i].vert;
	// If topology may have changed, fix up the mapping!
	if(checkTopology) {
		// Flush existing mapping
		for(int i = 0; i < verts; ++i)
			vertMap[i].vert = -1;
		for(i = 0; i < vecs; ++i)
			vecMap[i].vert = -1;
		for(i = 0; i < patches; ++i)
			patchMap[i].patch = -1;
		for(i = 0; i < colors; ++i)
			colorMap[i].vert = -1;
		for(i = 0; i < illums; ++i)
			illumMap[i].vert = -1;
		for(i = 0; i < alphas; ++i)
			alphaMap[i].vert = -1;
		// Build the new mapping
		int wverts = patch.numVerts;
		for(int wvert = 0; wvert < wverts; ++wvert) {
			int aux = patch.verts[wvert].aux1;
			if(aux != 0xffffffff) {
				if(aux >=0 && aux < verts)
					vertMap[aux].vert = wvert;
				}
			}
		int wvecs = patch.numVecs;
		for(int wvec = 0; wvec < wvecs; ++wvec) {
			int aux = patch.vecs[wvec].aux1;
			if(aux != 0xffffffff) {
				if(aux >= 0 && aux < vecs)
					vecMap[aux].vert = wvec;
				}
			}
		int wpatches = patch.numPatches;
		for(int wpatch = 0; wpatch < wpatches; ++wpatch) {
			int aux = patch.patches[wpatch].aux1;
			if(aux != 0xffffffff) {
				if(aux >= 0 && aux < patches)
					patchMap[aux].patch = wpatch;
				}
			}
		int wcolors = patch.getNumMapVerts(0);
		for(int wcolor = 0; wcolor < wcolors; ++wcolor) {
			int aux = patch.mapVerts(0)[wcolor].aux1;
			if(aux != 0xffffffff) {
				if(aux >= 0 && aux < colors)
					colorMap[aux].vert = wcolor;
				}
			}
		int willums = patch.getNumMapVerts(MAP_SHADING);
		for(int willum = 0; willum < willums; ++willum) {
			int aux = patch.mapVerts(MAP_SHADING)[willum].aux1;
			if(aux != 0xffffffff) {
				if(aux >= 0 && aux < illums)
					illumMap[aux].vert = willum;
				}
			}
		int walphas = patch.getNumMapVerts(MAP_ALPHA);
		for(int walpha = 0; walpha < walphas; ++walpha) {
			int aux = patch.mapVerts(MAP_ALPHA)[walpha].aux1;
			if(aux != 0xffffffff) {
				if(aux >= 0 && aux < alphas)
					alphaMap[aux].vert = walpha;
				}
			}
		}
	// Now compute the vertex deltas...
	for(i = 0; i < verts; ++i) {
		EPMapVert &map = vertMap[i];
		int prev = prevVert[i];
		if(map.vert >= 0) {
			PatchVert &v = patch.verts[map.vert];
			assert(prev >= 0);
			PatchVert &vo = oldPatch.verts[prev];
#ifdef VMAP_DEBUG
			Point3 oldDelta = map.delta;
#endif
			map.delta += (v.p - vo.p);
			if(PatchVertChanged(vo, v))
				map.flags |= EPMAP_ALTERED;
#ifdef VMAP_DEBUG
			if(map.delta != oldDelta)
				DebugPrint("Vert %d delta changed from %.2f %.2f %.2f to %.2 %.2f %.2f\n",i,oldDelta.x,oldDelta.y,oldDelta.z,map.delta.x,map.delta.y,map.delta.z);
#endif
			}
		}
	// And compute the vector deltas!
	for(i = 0; i < vecs; ++i) {
		EPMapVert &map = vecMap[i];
		int prev = prevVec[i];
		if(map.vert >= 0) {
			PatchVec &v = patch.vecs[map.vert];
			assert(prev >= 0);
			PatchVec &vo = oldPatch.vecs[prev];
#ifdef VMAP_DEBUG
			Point3 oldDelta = map.delta;
#endif
			map.delta += (v.p - vo.p);
			if(PatchVecChanged(vo, v))
				map.flags |= EPMAP_ALTERED;
#ifdef VMAP_DEBUG
			if(map.delta != oldDelta)
				DebugPrint("Vec %d delta changed from %.2f %.2f %.2f to %.2 %.2f %.2f\n",i,oldDelta.x,oldDelta.y,oldDelta.z,map.delta.x,map.delta.y,map.delta.z);
#endif
			}
		}
	// Now check patches for changes...
	for(i = 0; i < patches; ++i) {
		EPMapPatch &map = patchMap[i];
		int prev = prevPatch[i];
		if(map.patch >= 0) {
			Patch &p = patch.patches[map.patch];
			assert(prev >= 0);
			Patch &po = oldPatch.patches[prev];
			if(PatchPatchChanged(po, p))
				map.flags |= EPMAP_ALTERED;
			}
		}
	// Now see if the color was altered...
	for(i = 0; i < colors; ++i) {
		EPMapUVVert &map = colorMap[i];
		int prev = prevColor[i];
		if(map.vert >= 0) {
			PatchTVert &v = patch.mapVerts(0)[map.vert];
			assert(prev >= 0);
			PatchTVert &vo = oldPatch.mapVerts(0)[prev];
			if(PatchTVertChanged(vo, v))
				map.flags |= EPMAP_ALTERED;
			}
		}
	// Now see if the illum was altered...
	for(i = 0; i < illums; ++i) {
		EPMapUVVert &map = illumMap[i];
		int prev = prevIllum[i];
		if(map.vert >= 0) {
			PatchTVert &v = patch.mapVerts(MAP_SHADING)[map.vert];
			assert(prev >= 0);
			PatchTVert &vo = oldPatch.mapVerts(MAP_SHADING)[prev];
			if(PatchTVertChanged(vo, v))
				map.flags |= EPMAP_ALTERED;
			}
		}
	// Now see if the alpha was altered...
	for(i = 0; i < alphas; ++i) {
		EPMapUVVert &map = alphaMap[i];
		int prev = prevAlpha[i];
		if(map.vert >= 0) {
			PatchTVert &v = patch.mapVerts(MAP_ALPHA)[map.vert];
			assert(prev >= 0);
			PatchTVert &vo = oldPatch.mapVerts(MAP_ALPHA)[prev];
			if(PatchTVertChanged(vo, v))
				map.flags |= EPMAP_ALTERED;
			}
		}
	}

void EPMapper::ApplyDeltas(PatchMesh &inPatch, PatchMesh &outPatch) {
	// Now apply to output
	for(int i = 0; i < verts; ++i) {
		EPMapVert &pv = vertMap[i];
		if(pv.vert >= 0) {
			//assert(pv.vert >= 0 && pv.vert < outPatch.numVerts);
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.numVerts))
				{
				outPatch.setNumVerts(pv.vert+1,TRUE); 
				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			PatchVert &vo = outPatch.verts[pv.vert];
			if(i >= inPatch.numVerts) 
				vo.p = pv.delta;
			else {
				PatchVert &vi = inPatch.verts[i];
				if(pv.flags & EPMAP_ALTERED)
					vo.p = vi.p + pv.delta;
				else {
					vo.p = vi.p;
					vo.flags = vi.flags;
					}
				}
			}
		}
	for(i = 0; i < vecs; ++i) {
		EPMapVert &pv = vecMap[i];
		if(pv.vert >= 0) {
			//assert(pv.vert >= 0 && pv.vert < outPatch.numVecs);
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.numVecs))
				{
				outPatch.setNumVecs(pv.vert+1,TRUE); 

				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			PatchVec &vo = outPatch.vecs[pv.vert];
			if(i >= inPatch.numVecs) 
				vo.p = pv.delta;
			else {
				PatchVec &vi = inPatch.vecs[i];
				if(pv.flags & EPMAP_ALTERED)
					vo.p = vi.p + pv.delta;
				else {
					vo.p = vi.p;
					}
				}
			}
		}
	for(i = 0; i < patches; ++i) {
		EPMapPatch &pp = patchMap[i];
		if(pp.patch >= 0) {
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pp.patch >= 0 && pp.patch < outPatch.numPatches))
				{
				outPatch.setNumPatches(pp.patch+1,TRUE); 

				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			Patch &po = outPatch.patches[pp.patch];
			if(i >= inPatch.numPatches) {
				// Nothing to do
				}
			else {
				Patch &pi = inPatch.patches[i];
				if(pp.flags & EPMAP_ALTERED) {
					// Nothing to do
					}
				else {
					po.smGroup = pi.smGroup;
					po.flags = pi.flags;
					}
				}
			}
		}

	// Here's the only weird part of this.  The Edit Patch modifier doesn't screw around
	// with any of the mapping channels except the vertex color, illumination and alpha,
	// so we're just gonna apply deltas to those.  The rest of the incoming mapping channels
	// just get copied vertatim.

	for(i = 0; i < colors; ++i) {
		EPMapUVVert &pv = colorMap[i];
		if(pv.vert >= 0) {
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.getNumMapVerts(0)))
				{
				outPatch.setNumMapVerts(pv.vert+1,TRUE); 

				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			PatchTVert &vo = outPatch.mapVerts(0)[pv.vert];
			if(i >= inPatch.getNumMapVerts(0)) {
				// Nothing to do
				}
			else {
				PatchTVert &vi = inPatch.mapVerts(0)[i];
				if(pv.flags & EPMAP_ALTERED) {
					// Nothing to do
					}
				else {
					vo.p = vi.p;
					}
				}
			}
		}
	for(i = 0; i < illums; ++i) {
		EPMapUVVert &pv = illumMap[i];
		if(pv.vert >= 0) {
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.getNumMapVerts(MAP_SHADING)))
				{
				outPatch.setNumMapVerts(pv.vert+1,TRUE); 

				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			PatchTVert &vo = outPatch.mapVerts(MAP_SHADING)[pv.vert];
			if(i >= inPatch.getNumMapVerts(MAP_SHADING)) {
				// Nothing to do
				}
			else {
				PatchTVert &vi = inPatch.mapVerts(MAP_SHADING)[i];
				if(pv.flags & EPMAP_ALTERED) {
					// Nothing to do
					}
				else {
					vo.p = vi.p;
					}
				}
			}
		}
	for(i = 0; i < alphas; ++i) {
		EPMapUVVert &pv = alphaMap[i];
		if(pv.vert >= 0) {
			//watje 4-27-99 instead just throwing an assert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.getNumMapVerts(MAP_ALPHA)))
				{
				outPatch.setNumMapVerts(pv.vert+1,TRUE); 

				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
				warning = GetString(IDS_PW_SURFACEERROR);

				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK|MB_APPLMODAL );
				}

			PatchTVert &vo = outPatch.mapVerts(MAP_ALPHA)[pv.vert];
			if(i >= inPatch.getNumMapVerts(MAP_ALPHA)) {
				// Nothing to do
				}
			else {
				PatchTVert &vi = inPatch.mapVerts(MAP_ALPHA)[i];
				if(pv.flags & EPMAP_ALTERED) {
					// Nothing to do
					}
				else {
					vo.p = vi.p;
					}
				}
			}
		}


	int inMaps = inPatch.getNumMaps();
	int outMaps = outPatch.getNumMaps();
	if(inMaps > outMaps)
		outMaps = inMaps;
	outPatch.setNumMaps (outMaps, TRUE);	// Prepare output mapping channels

	// Future work: Work out deltas for channels -1 thru 0 (Alpha, Illum, Color)
	// For now, we're just keeping the ones in our edited patch (this freezes those
	// channels, preventing changes from below in the edit stack)

	// Copy the tverts and init the texture patches for channels 1 and up
	for(int mp=1; mp<inPatch.numTVerts.Count(); ++mp) {
		outPatch.setNumMapVerts (mp, inPatch.getNumMapVerts(mp));
		if (outPatch.mapVerts(mp) && inPatch.mapVerts(mp) && outPatch.getNumMapVerts(mp)) 
			memcpy (outPatch.mapVerts(mp), inPatch.mapVerts(mp), outPatch.getNumMapVerts(mp)*sizeof (PatchTVert));

		if (inPatch.mapPatches(mp)) {
			int outPatches = outPatch.numPatches;
			outPatch.setNumMapPatches (mp, outPatches);
			int inPatches = inPatch.numPatches;
			if(inPatches >= outPatches)
				memcpy (outPatch.mapPatches(mp), inPatch.mapPatches(mp), outPatches*sizeof (TVPatch));
			else {
				// Copy what there is in the original
				memcpy (outPatch.mapPatches(mp), inPatch.mapPatches(mp), inPatches*sizeof (TVPatch));
				// Fill in the rest with dummies
				TVPatch dummy;
				for(i = inPatches; i < outPatches; ++i) {
					outPatch.mapPatches(mp)[i] = dummy;
					}
				}
			// Now process remapping
			for(i = 0; i < patches; ++i) {
				EPMapPatch &pp = patchMap[i];
				if(pp.patch >= 0) {
					TVPatch &po = outPatch.mapPatches(mp)[pp.patch];
					if(i >= inPatch.numPatches) {
						// Nothing to do
						}
					else {
						TVPatch &pi = inPatch.mapPatches(mp)[i];
						po = pi;
						}
					}
				}
			}
		else
			outPatch.setNumMapPatches (mp, 0);
		}

	}

EPMapper& EPMapper::operator=(EPMapper &from) {
	if(vertMap)
		delete [] vertMap;
	verts = from.verts;
	vertMap = new EPMapVert[verts];
	for(int i = 0; i < verts; ++i)
		vertMap[i] = from.vertMap[i];
	if(vecMap)
		delete [] vecMap;
	vecs = from.vecs;
	vecMap = new EPMapVert[vecs];
	for(i = 0; i < vecs; ++i)
		vecMap[i] = from.vecMap[i];
	if(patchMap)
		delete [] patchMap;
	patches = from.patches;
	patchMap = new EPMapPatch[patches];
	for(i = 0; i < patches; ++i)
		patchMap[i] = from.patchMap[i];
	if(colorMap)
		delete [] colorMap;
	colors = from.colors;
	colorMap = new EPMapUVVert[colors];
	for(i = 0; i < colors; ++i)
		colorMap[i] = from.colorMap[i];
	if(illumMap)
		delete [] illumMap;
	illums = from.illums;
	illumMap = new EPMapUVVert[illums];
	for(i = 0; i < illums; ++i)
		illumMap[i] = from.illumMap[i];
	if(alphaMap)
		delete [] alphaMap;
	alphas = from.alphas;
	alphaMap = new EPMapUVVert[alphas];
	for(i = 0; i < alphas; ++i)
		alphaMap[i] = from.alphaMap[i];
	return *this;
	}

void EPMapper::RescaleWorldUnits(float f) {
	for(int i = 0; i < verts; ++i) {
		vertMap[i].delta *= f;
		}
	for(i = 0; i < vecs; ++i) {
		vecMap[i].delta *= f;
		}
	}

#define EPVM_DATA_CHUNK 0x1000
#define EPVM_DATA_CHUNK_R4_PRELIM 0x1010
#define EPVM_DATA_CHUNK_R4 0x1020

IOResult EPMapper::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(EPVM_DATA_CHUNK_R4);
	isave->Write(&verts,sizeof(int),&nb);
	isave->Write(vertMap,sizeof(EPMapVert) * verts, &nb);
	isave->Write(&vecs,sizeof(int),&nb);
	isave->Write(vecMap,sizeof(EPMapVert) * vecs, &nb);
	isave->Write(&patches,sizeof(int),&nb);
	isave->Write(patchMap,sizeof(EPMapPatch) * patches, &nb);
	isave->Write(&colors,sizeof(int),&nb);
	isave->Write(colorMap,sizeof(EPMapUVVert) * colors, &nb);
	isave->Write(&illums,sizeof(int),&nb);
	isave->Write(illumMap,sizeof(EPMapUVVert) * illums, &nb);
	isave->Write(&alphas,sizeof(int),&nb);
	isave->Write(alphaMap,sizeof(EPMapUVVert) * alphas, &nb);
	isave->EndChunk();
	return IO_OK;
	}

class R3EPMapVert {
	public:
		BOOL originalStored;
		int vert;
		Point3 original;
		Point3 delta;		// The delta we've applied
	};

IOResult EPMapper::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	int index = 0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case EPVM_DATA_CHUNK: {
				res = iload->Read(&verts,sizeof(int),&nb);
				if(vertMap)
					delete [] vertMap;
				vertMap = new EPMapVert[verts];
				for(int i = 0; i < verts; ++i) {
					R3EPMapVert theVert;
					res = iload->Read(&theVert,sizeof(R3EPMapVert),&nb);
					vertMap[i].vert = theVert.vert;
					vertMap[i].delta = theVert.delta;
					vertMap[i].flags = EPMAP_ALTERED;
					}
				res = iload->Read(&vecs,sizeof(int),&nb);
				if(vecMap)
					delete [] vecMap;
				vecMap = new EPMapVert[vecs];
				for(i = 0; i < vecs; ++i) {
					R3EPMapVert theVec;
					res = iload->Read(&theVec,sizeof(R3EPMapVert),&nb);
					vecMap[i].vert = theVec.vert;
					vecMap[i].delta = theVec.delta;
					vecMap[i].flags = EPMAP_ALTERED;
					}
				if(patchMap)
					delete [] patchMap;
				patches = 0;			// Old file, no such data!
				patchMap = new EPMapPatch[0];
				iload->SetObsolete();
				}
				break;
			case EPVM_DATA_CHUNK_R4_PRELIM:
				res = iload->Read(&verts,sizeof(int),&nb);
				if(vertMap)
					delete [] vertMap;
				vertMap = new EPMapVert[verts];
				res = iload->Read(vertMap,sizeof(EPMapVert) * verts,&nb);
				res = iload->Read(&vecs,sizeof(int),&nb);
				if(vecMap)
					delete [] vecMap;
				vecMap = new EPMapVert[vecs];
				res = iload->Read(vecMap,sizeof(EPMapVert) * vecs,&nb);
				res = iload->Read(&patches,sizeof(int),&nb);
				if(patchMap)
					delete [] patchMap;
				patchMap = new EPMapPatch[patches];
				res = iload->Read(patchMap,sizeof(EPMapPatch) * patches,&nb);
				break;
			case EPVM_DATA_CHUNK_R4:
				res = iload->Read(&verts,sizeof(int),&nb);
				if(vertMap)
					delete [] vertMap;
				vertMap = new EPMapVert[verts];
				res = iload->Read(vertMap,sizeof(EPMapVert) * verts,&nb);
				res = iload->Read(&vecs,sizeof(int),&nb);
				if(vecMap)
					delete [] vecMap;
				vecMap = new EPMapVert[vecs];
				res = iload->Read(vecMap,sizeof(EPMapVert) * vecs,&nb);
				res = iload->Read(&patches,sizeof(int),&nb);
				if(patchMap)
					delete [] patchMap;
				patchMap = new EPMapPatch[patches];
				res = iload->Read(patchMap,sizeof(EPMapPatch) * patches,&nb);
				res = iload->Read(&colors,sizeof(int),&nb);
				if(colorMap)
					delete [] colorMap;
				colorMap = new EPMapUVVert[colors];
				res = iload->Read(colorMap,sizeof(EPMapUVVert) * colors,&nb);
				res = iload->Read(&illums,sizeof(int),&nb);
				if(illumMap)
					delete [] illumMap;
				illumMap = new EPMapUVVert[illums];
				res = iload->Read(illumMap,sizeof(EPMapUVVert) * illums,&nb);
				res = iload->Read(&alphas,sizeof(int),&nb);
				if(alphaMap)
					delete [] alphaMap;
				alphaMap = new EPMapUVVert[alphas];
				res = iload->Read(alphaMap,sizeof(EPMapUVVert) * alphas,&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

EditPatchData::EditPatchData(EditPatchMod *mod)
	{
	r3Fix = FALSE;	// Flag to fixup r3 deltas to r4 standards

	// CAL-05/01/03: support spline surface generation (FID #1914)
	splineInput = FALSE;

	meshSteps = mod->meshSteps;
#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
	meshStepsRender = mod->meshStepsRender;
#endif
	showInterior = mod->showInterior;
	usePatchNormals = mod->usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

//	meshAdaptive = mod->meshAdaptive;	// Future use (Not used now)
	viewTess = mod->viewTess;
	prodTess = mod->prodTess;
	dispTess = mod->dispTess;
	mViewTessNormals = mod->mViewTessNormals;
	mProdTessNormals = mod->mProdTessNormals;
	mViewTessWeld = mod->mViewTessWeld;
	mProdTessWeld = mod->mProdTessWeld;
	displayLattice = mod->displayLattice;
	displaySurface = mod->displaySurface;
	flags = 0;
	tempData = NULL;
	oldPatch = NULL;
	}

EditPatchData::EditPatchData(EditPatchData& emc)
	{
	// CAL-05/01/03: support spline surface generation (FID #1914)
	splineInput = emc.splineInput;

	meshSteps = emc.meshSteps;
#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
	meshStepsRender = emc.meshStepsRender;
#endif
	showInterior = emc.showInterior;
	usePatchNormals = emc.usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

//	meshAdaptive = emc.meshAdaptive;	// Future use (Not used now)
	viewTess = emc.viewTess;
	prodTess = emc.prodTess;
	dispTess = emc.dispTess;
	mViewTessNormals = emc.mViewTessNormals;
	mProdTessNormals = emc.mProdTessNormals;
	mViewTessWeld = emc.mViewTessWeld;
	mProdTessWeld = emc.mProdTessWeld;
	displayLattice = emc.displayLattice;
	displaySurface = emc.displaySurface;
	flags = emc.flags;
	tempData = NULL;
	mapper = emc.mapper;
	finalPatch = emc.finalPatch;
	oldPatch = NULL;
	eselSet = emc.eselSet;
	pselSet = emc.pselSet;
	vselSet = emc.vselSet;
	hselSet = emc.hselSet;	// CAL-06/10/03: (FID #1914)
	}

EditPatchData::~EditPatchData() {
	if(oldPatch) {
		delete oldPatch;
		oldPatch = NULL;
		}
	}

void EditPatchData::Apply(EditPatchMod *mod,TimeValue t,PatchObject *patchOb,int level)
	{
	// Either just copy it from the existing cache or rebuild from previous level!
	if ( !GetFlag(EPD_UPDATING_CACHE) && tempData && tempData->PatchCached(t) ) {
 //212967 watje need to update our hooks just before copying
		tempData->GetPatch(t)->UpdateHooks();
		patchOb->patch.DeepCopy (tempData->GetPatch(t), EDITPAT_CHANNELS );		
		patchOb->PointsWereChanged();
		patchOb->patch.InvalidateGeomCache();
		}	
	else if ( GetFlag(EPD_HASDATA) ) {
		// For old files, which contain exhaustive data to reconstruct the editing process
		// of patches, we'll have data in the 'changes' table.  If it's there, go ahead and
		// replay the edits, then store the alterations in our new delta format and discard
		// the change table!
		int count = changes.Count();
		if(count) {
//DebugPrint("*** Applying old style (%d) ***\n", count);
			// Store the topology for future reference
			mapper.Build(patchOb->patch);
			finalPatch = patchOb->patch;
			for(int i = 0; i < count; ++i) {
				PModRecord *rec = changes[i];
				// Record the topo flags
				PreUpdateChanges(&patchOb->patch);
				BOOL result = rec->Redo(&patchOb->patch,0);
				UpdateChanges(&patchOb->patch);
				// If we hit one that didn't play back OK, we need to flush the remainder
				if(!result) {
					for(int j = i; j < count; ++j)
						delete changes[j];
					changes.Delete(i, count - i);
					break;
					}
				}
			// Nuke the changes table
			count = changes.Count();
			for(int k = 0; k < count; ++k)
				delete changes[k];
			changes.Delete(0, count);
			changes.Shrink();
			count = 0;
			}
		else
			{
			// Apply deltas to incoming shape, placing into finalPatch
			mod->ApplySoftSelectionToPatch( &finalPatch );
			// Old way was to just stuff in finalPatch.
//			patchOb->patch = finalPatch;
//212967 watje need to update our hooks just before copying
			finalPatch.UpdateHooks();

			// New way is to use some smarts and apply just the things we changed
			PatchMesh thePatch(finalPatch);	// Init with our stored patch
			mapper.ApplyDeltas(patchOb->patch, thePatch);
			patchOb->patch = thePatch;			// Stuff it in!
			}
		patchOb->PointsWereChanged();
		// Kind of a waste when there's no animation...		
		patchOb->UpdateValidity(GEOM_CHAN_NUM,FOREVER);
		patchOb->UpdateValidity(TOPO_CHAN_NUM,FOREVER);
		patchOb->UpdateValidity(SELECT_CHAN_NUM,FOREVER);
		patchOb->UpdateValidity(SUBSEL_TYPE_CHAN_NUM,FOREVER);
		patchOb->UpdateValidity(DISP_ATTRIB_CHAN_NUM,FOREVER);		
		}
	else {	// No data yet -- Store initial required data
//DebugPrint("<<<Storing Initial Data>>>\n");
		mapper.Build(patchOb->patch);
		finalPatch = patchOb->patch;
		}

	// Hand it its mesh interpolation info
	patchOb->SetMeshSteps(meshSteps);
#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
	patchOb->SetMeshStepsRender(meshStepsRender);
#endif // NO_OUTPUTRENDERER
	patchOb->SetShowInterior(showInterior);
	// CAL-05/15/03: use true patch normals. (FID #1760)
	patchOb->SetUsePatchNormals(usePatchNormals);

//	patchOb->SetAdaptive(meshAdaptive);	// Future use (Not used now)
	patchOb->SetViewTess(viewTess);
	patchOb->SetProdTess(prodTess);
	patchOb->SetDispTess(dispTess);
	patchOb->SetViewTessNormals(mViewTessNormals);
	patchOb->SetProdTessNormals(mProdTessNormals);
	patchOb->SetViewTessWeld(mViewTessWeld);
	patchOb->SetProdTessWeld(mProdTessWeld);

	// put in the relax stuff...  TH 7/26/00
	patchOb->SetRelax(mod->relax);
	patchOb->SetRelaxViewports(mod->relaxViewports);
	patchOb->SetRelaxValue(mod->relaxValue);
	patchOb->SetRelaxIter(mod->relaxIter);
	patchOb->SetRelaxBoundary(mod->relaxBoundary);
	patchOb->SetRelaxSaddle(mod->relaxSaddle);

	patchOb->showMesh = displaySurface;
	patchOb->SetShowLattice(displayLattice);

	// CAL-06/02/03: enable the display of Bezier handles (FID #827)
	BOOL dispBezier = patchOb->patch.dispFlags & DISP_BEZHANDLES;
	patchOb->patch.dispFlags = 0;	// TH 3/3/99
	switch ( level ) {
		case EP_PATCH:
			patchOb->patch.SetDispFlag(DISP_SELPATCHES);
			if (inPatchCreate ||
				(byVertex && mod->GetSubobjectLevel() == EP_PATCH) )
				patchOb->patch.SetDispFlag(DISP_VERTTICKS);
			break;
		case EP_EDGE:
			patchOb->patch.SetDispFlag(DISP_SELEDGES);
			if(byVertex)
				patchOb->patch.SetDispFlag(DISP_VERTTICKS);
			break;
		case EP_VERTEX:
			patchOb->patch.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS|DISP_VERTS);
			break;
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			patchOb->patch.SetDispFlag(DISP_VERTTICKS|DISP_SELHANDLES);
			if(byVertex)
				patchOb->patch.SetDispFlag(DISP_VERTTICKS);
			break;
		}
	// CAL-06/02/03: enable the display of Bezier handles (FID #827)
	if (dispBezier) patchOb->patch.dispFlags |= DISP_BEZHANDLES;

	patchOb->patch.selLevel = patchLevel[level];
	
	if ( GetFlag(EPD_UPDATING_CACHE) ) {
		assert(tempData);
		tempData->UpdateCache(patchOb);
		SetFlag(EPD_UPDATING_CACHE,FALSE);
		}		
	}

void EditPatchData::Invalidate(PartID part,BOOL patchValid)
	{
	if ( tempData ) {
		tempData->Invalidate(part,patchValid);
		}
	}

void EditPatchData::BeginEdit(TimeValue t)
	{
	assert(tempData);
	if ( !GetFlag(EPD_HASDATA) )
		SetFlag(EPD_HASDATA,TRUE);
	}

EPTempData *EditPatchData::TempData(EditPatchMod *mod)
	{
	if ( !tempData ) {
		assert(mod->ip);
		tempData = new EPTempData(mod,this);
		}
	return tempData;
	}

void EditPatchData::RescaleWorldUnits(float f) {
	// Scale the deltas inside the vertex map
	mapper.RescaleWorldUnits(f);
	// Now rescale stuff inside our data structures
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	finalPatch.Transform(stm);
	}

GenericNamedSelSetList &EditPatchData::GetSelSet(EditPatchMod *mod) {
	switch(mod->GetSubobjectType()) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			return hselSet;
		case EP_VERTEX:
			return vselSet;
		case EP_EDGE:
			return eselSet;
		case EP_PATCH:
		default:
			return pselSet;
		}
	}

GenericNamedSelSetList &EditPatchData::GetSelSet(int level) {
	switch(level) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_NS_HANDLE:
			return hselSet;
		case EP_NS_VERTEX:
			return vselSet;
		case EP_NS_EDGE:
			return eselSet;
		case EP_NS_PATCH:
		default:
			return pselSet;
		}
	}

// --------------------------------------------------------------------------------------

class EPVertMapRestore : public RestoreObj {
public:
	BOOL gotRedo;
	EPMapper undo;
	EPMapper redo;
	EditPatchData *epd;
	
	EPVertMapRestore(EditPatchData *d) {
		undo = d->mapper;
		epd = d;
		gotRedo = FALSE;
		}

	void Restore(int isUndo) {
		if(!gotRedo) {
			gotRedo = TRUE;
			redo = epd->mapper;
			}
		epd->mapper = undo;
		}

	void Redo() {
		epd->mapper = redo;
		}

	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("EPVertMapRestore")); }
};

// --------------------------------------------------------------------------------------

class FinalPatchRestore : public RestoreObj {
public:
	BOOL gotRedo;
	PatchMesh undo;
	PatchMesh redo;
	PatchMesh *patch;
	
	FinalPatchRestore(PatchMesh *s) {
		undo = *s;
		patch = s;
		gotRedo = FALSE;
		}

	void Restore(int isUndo) {
		if(!gotRedo) {
			gotRedo = TRUE;
			redo = *patch;
			}
		*patch = undo;
		}

	void Redo() {
		*patch = redo;
		}

	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("FinalPatchRestore")); }
};

// --------------------------------------------------------------------------------------
// CAL-06/20/03: support spline surface generation (FID #1914)
// restore object to keep the flags on PatchData for undo/redo.
class PatchDataFlagsRestore : public RestoreObj {
	DWORD filters;
	DWORD undoFlags, redoFlags;
	EditPatchData *patchData;
	
public:
	PatchDataFlagsRestore(EditPatchData *epd, DWORD fl) : patchData(epd), filters(fl)
	{
		undoFlags = patchData->flags & filters;
	}

	void Restore(int isUndo)
	{
		if(isUndo) {
			redoFlags = patchData->flags & filters;
		}
		patchData->flags = (patchData->flags & (~filters)) | undoFlags;
	}

	void Redo()
	{
		patchData->flags = (patchData->flags & (~filters)) | redoFlags;
	}

	int Size() { return 3 * sizeof(DWORD) + sizeof(void*); }
	
	TSTR Description() { return TSTR(_T("PatchDataFlagsRestore")); }
};

// --------------------------------------------------------------------------------------
// CAL-06/20/03: support spline surface generation (FID #1914)
// restore object to invalidate PatchData for undo/redo.
class PatchDataInvalidateRestore : public RestoreObj {
	EditPatchData *patchData;
	
public:
	PatchDataInvalidateRestore(EditPatchData *epd) : patchData(epd) { }

	void Restore(int isUndo)
	{
		patchData->Invalidate(PART_ALL, FALSE);
	}

	void Redo()
	{
		patchData->Invalidate(PART_ALL, FALSE);
	}

	int Size() { return sizeof(void*); }
	
	TSTR Description() { return TSTR(_T("PatchDataInvalidateRestore")); }
};

// --------------------------------------------------------------------------------------

void EditPatchData::PreUpdateChanges(PatchMesh *patch, BOOL checkTopology) {
//DebugPrint("Preupdating changes\n");
	// Store a temporary copy so we can compare after modifying
	if(!oldPatch)
		oldPatch = new PatchMesh(*patch);
	else
		*oldPatch = *patch;
	// Now put in our tags
	mapper.RecordTopologyTags(*patch);
	}

void EditPatchData::UpdateChanges(PatchMesh *patch, BOOL checkTopology, BOOL held) {
	assert(oldPatch);
	if (!held) {
		if(theHold.Holding()) {
			theHold.Put(new EPVertMapRestore(this));
			theHold.Put(new FinalPatchRestore(&finalPatch));
		}
	}
	// Update mapper's XYZ deltas
	mapper.RecomputeDeltas(checkTopology, *patch, *oldPatch);
	// Store the final shape
	finalPatch = *patch;
	}

#define EPD_GENERAL_CHUNK		0x1000	// Obsolete as of 11/12/98 (r3)
#define CHANGE_CHUNK			0x1010 	// Obsolete as of 11/12/98 (r3)
#define EPD_R3_GENERAL_CHUNK	0x1015
#define MESH_ATTRIB_CHUNK		0x1020
#define DISP_PARTS_CHUNK		0x1030
#define VTESS_ATTRIB_CHUNK		0x1070
#define PTESS_ATTRIB_CHUNK		0x1080
#define DTESS_ATTRIB_CHUNK		0x1090
#define NORMAL_TESS_ATTRIB_CHUNK	0x1110
#define WELD_TESS_ATTRIB_CHUNK	0x1120
#define VERTMAP_CHUNK			0x1130
#define FINALPATCH_CHUNK		0x1140
#define RENDERSTEPS_CHUNK		0x1150
#define SHOWINTERIOR_CHUNK		0x1160
// Delta chunks
#define VERT_DELTA_CHUNK		0x1170
#define VEC_DELTA_CHUNK			0x1180
#define EDGE_DELTA_CHUNK		0x1190
#define PATCH_DELTA_CHUNK		0x11A0
#define EPD_MESH_COUNTS			0x11B0
// CAL-05/01/03: support spline surface generation (FID #1914)
#define SPLINEINPUT_CHUNK		0x11C0
// CAL-05/15/03: use true patch normals. (FID #1760)
#define USEPATCHNORM_CHUNK		0x11D0

// Named sel set chunks
#define VSELSET_CHUNK		0x1040
#define ESELSET_CHUNK		0x1050
#define PSELSET_CHUNK		0x1060
#define HSELSET_CHUNK		0x1065

IOResult EditPatchData::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(EPD_R3_GENERAL_CHUNK);
	isave->Write(&flags,sizeof(DWORD),&nb);
	isave->EndChunk();
	isave->BeginChunk(MESH_ATTRIB_CHUNK);
	isave->Write(&meshSteps,sizeof(int),&nb);
// Future use (Not used now)
	BOOL fakeAdaptive = FALSE;	
	isave->Write(&fakeAdaptive,sizeof(BOOL),&nb);
//	isave->Write(&meshAdaptive,sizeof(BOOL),&nb);	// Future use (Not used now)
	isave->EndChunk();

	// CAL-05/01/03: support spline surface generation (FID #1914)
	isave->BeginChunk(SPLINEINPUT_CHUNK);
	isave->Write(&splineInput,sizeof(BOOL),&nb);
	isave->EndChunk();

#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
	isave->BeginChunk(RENDERSTEPS_CHUNK);
	if ( (meshStepsRender < 0) || (meshStepsRender > 100))
		{
		meshStepsRender = 5;
		DbgAssert(0);
		}
	isave->Write(&meshStepsRender,sizeof(int),&nb);
	isave->EndChunk();
#endif // NO_OUTPUTRENDERER
	isave->BeginChunk(SHOWINTERIOR_CHUNK);
	isave->Write(&showInterior,sizeof(BOOL),&nb);
	isave->EndChunk();
	// CAL-05/15/03: use true patch normals. (FID #1760)
	isave->BeginChunk(USEPATCHNORM_CHUNK);
	isave->Write(&usePatchNormals, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(VTESS_ATTRIB_CHUNK);
	viewTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(PTESS_ATTRIB_CHUNK);
	prodTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(DTESS_ATTRIB_CHUNK);
	dispTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(DISP_PARTS_CHUNK);
	isave->Write(&displaySurface,sizeof(BOOL),&nb);
	isave->Write(&displayLattice,sizeof(BOOL),&nb);
	isave->EndChunk();

	isave->BeginChunk(NORMAL_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessNormals,sizeof(BOOL),&nb);
	isave->Write(&mProdTessNormals,sizeof(BOOL),&nb);
	isave->EndChunk();

	isave->BeginChunk(WELD_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessWeld,sizeof(BOOL),&nb);
	isave->Write(&mProdTessWeld,sizeof(BOOL),&nb);
	isave->EndChunk();

	
	// Save named sel sets
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	if (hselSet.Count()) {
		isave->BeginChunk(HSELSET_CHUNK);
		hselSet.Save(isave);
		isave->EndChunk();
		}
	if (vselSet.Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		vselSet.Save(isave);
		isave->EndChunk();
		}
	if (eselSet.Count()) {
		isave->BeginChunk(ESELSET_CHUNK);
		eselSet.Save(isave);
		isave->EndChunk();
		}
	if (pselSet.Count()) {
		isave->BeginChunk(PSELSET_CHUNK);
		pselSet.Save(isave);
		isave->EndChunk();
		}

	isave->BeginChunk(VERTMAP_CHUNK);
	mapper.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(FINALPATCH_CHUNK);
	finalPatch.setNumMaps(1);	// Strip off all texture info except color, illum, alpha
	finalPatch.Save(isave);
	isave->EndChunk();
	return IO_OK;
	}

IOResult EditPatchData::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	PModRecord *theChange;
	r3Fix = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			// The following code is here to load pre-release 3 files.
			case EPD_GENERAL_CHUNK:
				iload->SetObsolete();
				iload->Read(&flags,sizeof(DWORD),&nb);
				break;
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case CLEARVECSELRECORD_CHUNK:
				theChange = new ClearPVecSelRecord;
				goto load_change;
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case SETVECSELRECORD_CHUNK:
				theChange = new SetPVecSelRecord;
				goto load_change;
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case INVERTVECSELRECORD_CHUNK:
				theChange = new InvertPVecSelRecord;
				goto load_change;
			case CLEARVERTSELRECORD_CHUNK:
				theChange = new ClearPVertSelRecord;
				goto load_change;
			case SETVERTSELRECORD_CHUNK:
				theChange = new SetPVertSelRecord;
				goto load_change;
			case INVERTVERTSELRECORD_CHUNK:
				theChange = new InvertPVertSelRecord;
				goto load_change;
			case CLEAREDGESELRECORD_CHUNK:
				theChange = new ClearPEdgeSelRecord;
				goto load_change;
			case SETEDGESELRECORD_CHUNK:
				theChange = new SetPEdgeSelRecord;
				goto load_change;
			case INVERTEDGESELRECORD_CHUNK:
				theChange = new InvertPEdgeSelRecord;
				goto load_change;
			case CLEARPATCHSELRECORD_CHUNK:
				theChange = new ClearPatchSelRecord;
				goto load_change;
			case SETPATCHSELRECORD_CHUNK:
				theChange = new SetPatchSelRecord;
				goto load_change;
			case INVERTPATCHSELRECORD_CHUNK:
				theChange = new InvertPatchSelRecord;
				goto load_change;
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case VECSELRECORD_CHUNK:
				theChange = new PVecSelRecord;
				goto load_change;
			case VERTSELRECORD_CHUNK:
				theChange = new PVertSelRecord;
				goto load_change;
			case EDGESELRECORD_CHUNK:
				theChange = new PEdgeSelRecord;
				goto load_change;
			case PATCHSELRECORD_CHUNK:
				theChange = new PatchSelRecord;
				goto load_change;
			case PATCHDELETERECORD_CHUNK:
				theChange = new PatchDeleteRecord;
				goto load_change;
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case VECMOVERECORD_CHUNK:
				theChange = new PVecMoveRecord;
				goto load_change;
			case VERTMOVERECORD_CHUNK:
				theChange = new PVertMoveRecord;
				goto load_change;
			case PATCHCHANGERECORD_CHUNK:
				theChange = new PatchChangeRecord;
				goto load_change;
			case VERTCHANGERECORD_CHUNK:
				theChange = new PVertChangeRecord;
				goto load_change;
			case PATCHADDRECORD_CHUNK:
				theChange = new PatchAddRecord;
				goto load_change;
			case EDGESUBDIVIDERECORD_CHUNK:
				theChange = new EdgeSubdivideRecord;
				goto load_change;
			case PATCHSUBDIVIDERECORD_CHUNK:
				theChange = new PatchSubdivideRecord;
				goto load_change;
			case PATTACHRECORD_CHUNK:
				theChange = new PAttachRecord;
				goto load_change;
			case PATCHDETACHRECORD_CHUNK:
				theChange = new PatchDetachRecord;
				goto load_change;
			case PATCHMTLRECORD_CHUNK:
				theChange = new PatchMtlRecord;
				goto load_change;
			case VERTWELDRECORD_CHUNK:
				theChange = new PVertWeldRecord;
				goto load_change;
			case VERTDELETERECORD_CHUNK:
				theChange = new PVertDeleteRecord;
				// Intentional fall-thru!
				load_change:
				changes.Append(1,&theChange);
				changes[changes.Count()-1]->Load(iload);
				break;
			//
			// The following code is used for post-release 3 files
			//
			case EPD_R3_GENERAL_CHUNK:
				res = iload->Read(&flags,sizeof(DWORD),&nb);
				break;
			case VERTMAP_CHUNK:
				res = mapper.Load(iload);
				break;
			case FINALPATCH_CHUNK:
				res = finalPatch.Load(iload);
				break;
			//
			// The following code is common to all versions' files
			//
			case MESH_ATTRIB_CHUNK:
				iload->Read(&meshSteps,sizeof(int),&nb);
				res = iload->Read(&meshAdaptive,sizeof(BOOL),&nb);	// Future use (Not used now)
				break;
			// CAL-05/01/03: support spline surface generation (FID #1914)
			case SPLINEINPUT_CHUNK:
				iload->Read(&splineInput,sizeof(BOOL),&nb);
				break;
#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
			case RENDERSTEPS_CHUNK:
				iload->Read(&meshStepsRender,sizeof(int),&nb);
				if ( (meshStepsRender < 0) || (meshStepsRender > 100))
					{
					meshStepsRender = 5;
					DbgAssert(0);
					}
				break;
#endif // NO_OUTPUTRENDERER
			case SHOWINTERIOR_CHUNK:
				iload->Read(&showInterior,sizeof(BOOL),&nb);
				break;
			// CAL-05/15/03: use true patch normals. (FID #1760)
			case USEPATCHNORM_CHUNK:
				iload->Read(&usePatchNormals, sizeof(BOOL), &nb);
				break;

			case VTESS_ATTRIB_CHUNK:
				viewTess.Load(iload);
				break;
			case PTESS_ATTRIB_CHUNK:
				prodTess.Load(iload);
				break;
			case DTESS_ATTRIB_CHUNK:
				dispTess.Load(iload);
				break;
			case NORMAL_TESS_ATTRIB_CHUNK:
				iload->Read(&mViewTessNormals,sizeof(BOOL),&nb);
				res = iload->Read(&mProdTessNormals,sizeof(BOOL),&nb);
				break;
			case WELD_TESS_ATTRIB_CHUNK:
				iload->Read(&mViewTessWeld,sizeof(BOOL),&nb);
				res = iload->Read(&mProdTessWeld,sizeof(BOOL),&nb);
				break;
			case DISP_PARTS_CHUNK:
				iload->Read(&displaySurface,sizeof(BOOL),&nb);
				res = iload->Read(&displayLattice,sizeof(BOOL),&nb);
				break;
			// Load named selection sets
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case HSELSET_CHUNK:
				res = hselSet.Load(iload);
				break;
			case VSELSET_CHUNK:
				res = vselSet.Load(iload);
				break;
			case PSELSET_CHUNK:
				res = pselSet.Load(iload);
				break;
			case ESELSET_CHUNK:
				res = eselSet.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

void EditPatchData::DumpData() {
	}

/*-------------------------------------------------------------------*/		
// CAL-06/10/03: add handle sub-object mode. (FID #1914)

BOOL ClearPVecSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->vecSel;
	patch->vecSel.ClearAll();
	return TRUE;
	}

#define CHSR_SEL_CHUNK 0x1000

IOResult ClearPVecSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CHSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		
// CAL-06/10/03: add handle sub-object mode. (FID #1914)

BOOL SetPVecSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->vecSel;
	patch->vecSel.SetAll();
	return TRUE;
	}

#define SHSR_SEL_CHUNK 0x1000

IOResult SetPVecSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SHSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		
// CAL-06/10/03: add handle sub-object mode. (FID #1914)

BOOL InvertPVecSelRecord::Redo(PatchMesh *patch,int reRecord) {
	patch->vecSel = ~patch->vecSel;
	return TRUE;
	}

IOResult InvertPVecSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			default:
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearPVertSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->vertSel;
	patch->vertSel.ClearAll();
	return TRUE;
	}

#define CVSR_SEL_CHUNK 0x1000

IOResult ClearPVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CVSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetPVertSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->vertSel;
	patch->vertSel.SetAll();
	return TRUE;
	}

#define SVSR_SEL_CHUNK 0x1000

IOResult SetPVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SVSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertPVertSelRecord::Redo(PatchMesh *patch,int reRecord) {
	patch->vertSel = ~patch->vertSel;
	return TRUE;
	}

IOResult InvertPVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			default:
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearPEdgeSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->edgeSel;
	patch->edgeSel.ClearAll();
	return TRUE;
	}

#define CESR_SEL_CHUNK 0x1000

IOResult ClearPEdgeSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CESR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetPEdgeSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->edgeSel;
	patch->edgeSel.SetAll();
	return TRUE;
	}

#define SESR_SEL_CHUNK 0x1000

IOResult SetPEdgeSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SESR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertPEdgeSelRecord::Redo(PatchMesh *patch,int reRecord) {
	patch->edgeSel = ~patch->edgeSel;
	return TRUE;
	}

IOResult InvertPEdgeSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			default:
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearPatchSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->patchSel;
	patch->patchSel.ClearAll();
	return TRUE;
	}

#define CPSR_SEL_CHUNK 0x1000

IOResult ClearPatchSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CPSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetPatchSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		sel = patch->patchSel;
	patch->patchSel.SetAll();
	return TRUE;
	}

#define SPSR_SEL_CHUNK 0x1000

IOResult SetPatchSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SPSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertPatchSelRecord::Redo(PatchMesh *patch,int reRecord) {
	patch->patchSel = ~patch->patchSel;
	return TRUE;
	}

IOResult InvertPatchSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			default:
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		
// CAL-06/10/03: add handle sub-object mode. (FID #1914)

BOOL PVecSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!IsCompatible(patch->vecSel, newSel))
		return FALSE;
	patch->vecSel = newSel;
	return TRUE;
	}

#define HSR_OLDSEL_CHUNK 0x1000
#define HSR_NEWSEL_CHUNK 0x1010

IOResult PVecSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case HSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case HSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PVertSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!IsCompatible(patch->vertSel, newSel))
		return FALSE;
	patch->vertSel = newSel;
	return TRUE;
	}

#define VSR_OLDSEL_CHUNK 0x1000
#define VSR_NEWSEL_CHUNK 0x1010

IOResult PVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case VSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PEdgeSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!IsCompatible(patch->edgeSel, newSel))
		return FALSE;
	patch->edgeSel = newSel;
	return TRUE;
	}

#define ESR_OLDSEL_CHUNK 0x1000
#define ESR_NEWSEL_CHUNK 0x1010

IOResult PEdgeSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ESR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case ESR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PatchSelRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!IsCompatible(patch->patchSel, newSel))
		return FALSE;
	patch->patchSel = newSel;
	return TRUE;
	}

#define PSR_OLDSEL_CHUNK 0x1000
#define PSR_NEWSEL_CHUNK 0x1010

IOResult PatchSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case PSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void DeleteSelPatches(PatchMesh *patch) {
	if(!patch->patchSel.NumberSet())
		return;		// Nothing to do!

	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();

	// Tag the patches that are selected
	BitArray delPatches(patches);
	delPatches = patch->patchSel;

	BitArray delVerts(verts);
	delVerts.ClearAll();

	DeletePatchParts(patch, delVerts, delPatches);
	patch->computeInteriors();
	}

BOOL PatchDeleteRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	DeleteSelPatches(patch);
	return TRUE;
	}

#define PDELR_PATCH_CHUNK		0x1060

IOResult PatchDeleteRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			case PDELR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		
// CAL-06/10/03: add handle sub-object mode. (FID #1914)

BOOL PVecMoveRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!delta.IsCompatible(*patch))
		return FALSE;
	delta.Apply(*patch);
	return TRUE;
	}

#define HMR_DELTA_CHUNK		0x1000

IOResult PVecMoveRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case HMR_DELTA_CHUNK:
				res = delta.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PVertMoveRecord::Redo(PatchMesh *patch,int reRecord) {
	if(!delta.IsCompatible(*patch))
		return FALSE;
	delta.Apply(*patch);
	return TRUE;
	}

#define VMR_DELTA_CHUNK		0x1000

IOResult PVertMoveRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VMR_DELTA_CHUNK:
				res = delta.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void DeleteSelVerts(PatchMesh *patch) {
	if(!patch->vertSel.NumberSet())
		return;		// Nothing to do!

	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();

	// Tag the patches that use selected vertices
	BitArray delPatches(patches);
	delPatches.ClearAll();
	for(int i = 0; i < patches; ++i) {
		Patch& p = patch->patches[i];
		for(int j = 0; j < p.type; ++j) {
			if(patch->vertSel[p.v[j]]) {
				delPatches.Set(i);
				goto next_patch;
				}
			}
		next_patch:;
		}

	BitArray delVerts(verts);
	delVerts = patch->vertSel;
	DeletePatchParts(patch, delVerts, delPatches);
	patch->computeInteriors();
	}

BOOL PVertDeleteRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	DeleteSelVerts(patch);
	return TRUE;
	}

#define VDELR_PATCH_CHUNK		0x1060

IOResult PVertDeleteRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
//		switch(iload->CurChunkID())  {
//			case VDELR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
//			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PVertChangeRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	patch->ChangeVertType(index, type);
	return TRUE;
	}

#define VCHG_GENERAL_CHUNK		0x1001
#define VCHG_PATCH_CHUNK		0x1010

IOResult PVertChangeRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VCHG_GENERAL_CHUNK:
				res = iload->Read(&index,sizeof(int),&nb);
				res = iload->Read(&type,sizeof(int),&nb);
				break;
//			case VCHG_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PAttachRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatchCount = patch->numPatches;
	patch->Attach(&attPatch, mtlOffset);
	return TRUE;
	}

#define ATTR_GENERAL_CHUNK		0x1001
#define ATTR_ATTPATCH_CHUNK		0x1010
#define ATTR_MTLOFFSET_CHUNK	0x1020

IOResult PAttachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ATTR_GENERAL_CHUNK:
				res = iload->Read(&oldPatchCount,sizeof(int),&nb);
				break;
			case ATTR_ATTPATCH_CHUNK:
				res = attPatch.Load(iload);
				break;
			case ATTR_MTLOFFSET_CHUNK:
				res = iload->Read(&mtlOffset,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PatchDetachRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord && !copy)
		oldPatch = *patch;
	if(!copy) {
		BitArray vdel(patch->numVerts);
		vdel.ClearAll();
		BitArray pdel = patch->patchSel;
		DeletePatchParts(patch, vdel, pdel);
		}
	return TRUE;
	}

#define PDETR_GENERAL_CHUNK		0x1000
#define PDETR_PATCH_CHUNK		0x1030

IOResult PatchDetachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PDETR_GENERAL_CHUNK:
				res = iload->Read(&copy,sizeof(int),&nb);
				break;
//			case PDETR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PatchMtlRecord::Redo(PatchMesh *patch,int reRecord) {
	for(int i = 0; i < patch->numPatches; ++i) {
		if(patch->patchSel[i])
			patch->patches[i].setMatID(index);
		}
	return TRUE;
	}

#define PMTLR_GENERAL_CHUNK		0x1000
#define PMTLR_INDEX_CHUNK		0x1020

IOResult PatchMtlRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PMTLR_INDEX_CHUNK:
				res = iload->Read(&index,sizeof(MtlID),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void ChangePatchType(PatchMesh *patch, int index, int type) {
	patch->ChangePatchInterior(index, type);
/*
	// If positive vertex number, do it to just one vertex
	if(index >= 0) {
		patch->patches[index].flags = type;
		patch->computeInteriors();
		return;
		}

	// Otherwise, do it to all selected vertices!
	int patches = patch->numPatches;
	BitArray &psel = patch->patchSel;
	for(int i = 0; i < patches; ++i) {
		if(psel[i])
			patch->patches[i].flags = type;
		}

	patch->computeInteriors();
*/
	}

//watje
static void FixUpVerts(PatchMesh *patch) {
	int patches = patch->numPatches;
	for(int i = 0; i < patches; i++) {

		if (!(patch->patches[i].IsHidden()))
			{
			int ct = 4;
			if (patch->patches[i].type==PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
				{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(FALSE);
				}

			}
		}

	}

//watje unhide all
static void UnHidePatches(PatchMesh *patch) {
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	for(int i = 0; i < patches; i++) {
		if (patch->patches[i].IsHidden()) {
			patch->patches[i].SetHidden(FALSE);
			for (int j=0; j<patch->patches[i].type; j++) {
				patch->verts[patch->patches[i].v[j]].SetHidden (FALSE);
				}
			}
		}
	/* sca 3/10/01: Shouldn't be unhiding all vertices, just those on hidden patches.
	int verts = patch->numVerts;
	for(i = 0; i < verts; i++) {
		if (patch->verts[i].IsHidden())
			patch->verts[i].SetHidden(FALSE);
		}
		*/
	}

//watje hide patch
static void HidePatches(PatchMesh *patch) {
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	BitArray &psel = patch->patchSel;
	for(int i = 0; i < patches; i++) {
		if(psel[i])
			{
			patch->patches[i].SetHidden(TRUE);
//hide all 
			int ct = 4;
			if (patch->patches[i].type==PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
				{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
				}
			}
		}
	FixUpVerts(patch);
	}
//watje hide patches by verts
static void HideVerts(PatchMesh *patch) {
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	BitArray &vsel = patch->vertSel;
	for(int i = 0; i < patches; i++) {
		int ct = 4;
		if (patch->patches[i].type==PATCH_TRI)
			ct = 3;
		for (int k = 0; k < ct; k++)
			{
			int a = patch->patches[i].v[k];

			if(vsel[a])
				{
				patch->patches[i].SetHidden(TRUE);
				}
			}
		}
	for(i = 0; i < patches; i++) {
		if(patch->patches[i].IsHidden())
			{
//hide all 
			int ct = 4;
			if (patch->patches[i].type==PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
				{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
				}
			}
		}

	FixUpVerts(patch);
	}
//watje hide patches by verts
static void HideEdges(PatchMesh *patch) {
	// If positive vertex number, do it to just one vertex
	int edges = patch->numEdges;
	BitArray &esel = patch->edgeSel;
	for(int i = 0; i < edges; i++) {
		if (esel[i])
			{
//			int a = patch->edges[i].patch1;
//			int b = patch->edges[i].patch2;
//			if (a>0)
//				patch->patches[a].SetHidden(TRUE);
//			if (b>0)
//				patch->patches[b].SetHidden(TRUE);
// TH 3/24/00
			for(int j = 0; j < patch->edges[i].patches.Count(); ++j)
				patch->patches[patch->edges[i].patches[j]].SetHidden(TRUE);
			}
		}
	int patches = patch->numPatches;
	for(i = 0; i < patches; i++) {
		if(patch->patches[i].IsHidden())
			{
//hide all 
			int ct = 4;
			if (patch->patches[i].type==PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
				{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
				}
			}
		}
	FixUpVerts(patch);
	}



BOOL PatchChangeRecord::Redo(PatchMesh *patch,int reRecord) {
	if(index >= 0 && index >= patch->numPatches)
		return FALSE;
	if(reRecord)
		oldPatch = *patch;
	ChangePatchType(patch, index, type);
	return TRUE;
	}

#define PCHG_GENERAL_CHUNK		0x1001
#define PCHG_PATCH_CHUNK		0x1010

IOResult PatchChangeRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PCHG_GENERAL_CHUNK:
				res = iload->Read(&index,sizeof(int),&nb);
				res = iload->Read(&type,sizeof(int),&nb);
				break;
//			case PCHG_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void AddPatches(int type, PatchMesh *patch, BOOL postWeld) {
	if(!patch->edgeSel.NumberSet())
		return;		// Nothing to do!

	int lastVert = patch->getNumVerts();
	int edges = patch->getNumEdges();

	// Add a patch of the desired type to each selected edge that doesn't have two patches atatched!
	for(int i = 0; i < edges; ++i) {
		if(patch->edgeSel[i]) {
			PatchEdge &edge = patch->edges[i];
			if(edge.patches.Count() < 2) {
				int verts = patch->getNumVerts();
				int vecs = patch->getNumVecs();
				int patches = patch->getNumPatches();
				patch->setNumPatches(patches+1, TRUE);			// Add a patch
				patch->patches[patches].SetType(type);			// Make it the type we want
				patch->setNumVerts(verts + type - 2, TRUE);		// Add the appropriate number of verts
				patch->setNumVecs(vecs + (type-1) * 2 + type, TRUE);	// And the appropriate vector count
				Point3 p1 = patch->verts[edge.v1].p;
				Point3 p2 = patch->verts[edge.v2].p;
				Point3 v12 = patch->vecs[edge.vec12].p;
				Point3 v21 = patch->vecs[edge.vec21].p;
				Point3 edgeCenter = (p1 + p2) / 2.0f;
				// Load up the patch with the correct vert/vector indices
				Patch &spatch = patch->patches[edge.patches[0]];
				Patch &dpatch = patch->patches[patches];
				switch(type) {
					case PATCH_TRI:
						dpatch.setVerts(edge.v2, edge.v1, verts);
						dpatch.setVecs(edge.vec21, edge.vec12, vecs, vecs+1, vecs+2, vecs+3);
						dpatch.setInteriors(vecs+4, vecs+5, vecs+6);
						switch(spatch.type) {
							case PATCH_TRI: {		// Tri from Tri
								// Find the opposite vertex in the source triangle
								int opposite, o2a, o1a;
								if(spatch.edge[0] == i) {
									opposite = 2;
									o1a = 5;
									o2a = 2;
									}
								else
								if(spatch.edge[1] == i) {
									opposite = 0;
									o1a = 1;
									o2a = 4;
									}
								else {
									opposite = 1;
									o1a = 3;
									o2a = 0;
									}
								// Compute the new vert position
								Point3 oppVec = edgeCenter - patch->verts[spatch.v[opposite]].p;
								float oppLen = Length(oppVec);
								if(oppLen == 0.0f) {
									oppVec = Point3(0,0,1);
									oppLen = 1.0f;
									}
								Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
								Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
								Point3 n1a, n2a;
								if(Length(v1a) > 0.0f)
									n1a = Normalize(v1a);
								else
									n1a = Normalize(oppVec);
								if(Length(v2a) > 0.0f)
									n2a = Normalize(v2a);
								else
									n2a = Normalize(oppVec);
								
								// Build a composite vector based on the two edge vectors
								Point3 compVec = Normalize((n1a + n2a) / 2.0f);
								
								// Create the new vertex
								Point3 newPos = edgeCenter - compVec * oppLen;
								patch->verts[verts].p = newPos;

								// Compute the vectors
								patch->vecs[vecs].p = p1 - v1a;
								patch->vecs[vecs+1].p = newPos - (newPos - p1) / 3.0f;
								patch->vecs[vecs+2].p = newPos - (newPos - p2) / 3.0f;
								patch->vecs[vecs+3].p = p2 - v2a;
								}
								break;
							case PATCH_QUAD: {	// Tri from Quad
								// Find the opposite edge verts in the source quad
								int opposite1, opposite2, o1a, o2a;
								if(spatch.edge[0] == i) {
									opposite1 = 2;
									opposite2 = 3;
									o1a = 7;
									o2a = 2;
									}
								else
								if(spatch.edge[1] == i) {
									opposite1 = 3;
									opposite2 = 0;
									o1a = 1;
									o2a = 4;
									}
								else
								if(spatch.edge[2] == i) {
									opposite1 = 0;
									opposite2 = 1;
									o1a = 3;
									o2a = 6;
									}
								else {
									opposite1 = 1;
									opposite2 = 2;
									o1a = 5;
									o2a = 0;
									}
								// Compute the new vert position
								Point3 otherCenter = (patch->verts[spatch.v[opposite1]].p + patch->verts[spatch.v[opposite2]].p) / 2.0f;
								Point3 oppVec = edgeCenter - otherCenter;
								float oppLen = Length(oppVec);
								if(oppLen == 0.0f) {
									oppVec = Point3(0,0,1);
									oppLen = 1.0f;
									}
								Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
								Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
								Point3 n1a, n2a;
								if(Length(v1a) > 0.0f)
									n1a = Normalize(v1a);
								else
									n1a = Normalize(oppVec);
								if(Length(v2a) > 0.0f)
									n2a = Normalize(v2a);
								else
									n2a = Normalize(oppVec);
								
								// Build a composite vector based on the two edge vectors
								Point3 compVec = Normalize((n1a + n2a) / 2.0f);
								
								// Create the new vertex
								Point3 newPos = edgeCenter - compVec * oppLen;
								patch->verts[verts].p = newPos;

								// Compute the vectors
								patch->vecs[vecs].p = p1 - v1a;
								patch->vecs[vecs+1].p = newPos - (newPos - p1) / 3.0f;
								patch->vecs[vecs+2].p = newPos - (newPos - p2) / 3.0f;
								patch->vecs[vecs+3].p = p2 - v2a;
								}
								break;
							}
						break;
					case PATCH_QUAD:
						dpatch.setVerts(edge.v2, edge.v1, verts, verts+1);
						dpatch.setVecs(edge.vec21, edge.vec12, vecs, vecs+1, vecs+2, vecs+3, vecs+4, vecs+5);
						dpatch.setInteriors(vecs+6, vecs+7, vecs+8, vecs+9);
						switch(spatch.type) {
							case PATCH_TRI: {		// Quad from Tri
								// Find the opposite vertex in the source triangle
								int opposite, o2a, o1a;
								if(spatch.edge[0] == i) {
									opposite = 2;
									o1a = 5;
									o2a = 2;
									}
								else
								if(spatch.edge[1] == i) {
									opposite = 0;
									o1a = 1;
									o2a = 4;
									}
								else {
									opposite = 1;
									o1a = 3;
									o2a = 0;
									}

								Point3 oppVec = edgeCenter - patch->verts[spatch.v[opposite]].p;
								float oppLen = Length(oppVec);
								if(oppLen == 0.0f) {
									oppVec = Point3(0,0,1);
									oppLen = 1.0f;
									}
								Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
								Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
								Point3 n1a, n2a;
								if(Length(v1a) > 0.0f)
									n1a = Normalize(v1a);
								else
									n1a = Normalize(oppVec);
								if(Length(v2a) > 0.0f)
									n2a = Normalize(v2a);
								else
									n2a = Normalize(oppVec);

								// Compute the new vert positions
								Point3 newPos1 = p1 - n1a * oppLen;
								Point3 newPos2 = p2 - n2a * oppLen;
								patch->verts[verts].p = newPos1;
								patch->verts[verts+1].p = newPos2;
								// Compute the vectors
								patch->vecs[vecs].p = p1 - v1a;
								patch->vecs[vecs+1].p = newPos1 - (newPos1 - p1) / 3.0f;
								patch->vecs[vecs+2].p = newPos1 + (v12 - p1);
								patch->vecs[vecs+3].p = newPos2 + (v21 - p2);
								patch->vecs[vecs+4].p = newPos2 + (p2 - newPos2) / 3.0f;
								patch->vecs[vecs+5].p = p2 - v2a;
								}
								break;
							case PATCH_QUAD: {	// Quad from Quad
								// Find the opposite edge verts in the source quad
								int opposite1, opposite2, o1a, o2a;
								if(spatch.edge[0] == i) {
									opposite1 = 2;
									opposite2 = 3;
									o1a = 7;
									o2a = 2;
									}
								else
								if(spatch.edge[1] == i) {
									opposite1 = 3;
									opposite2 = 0;
									o1a = 1;
									o2a = 4;
									}
								else
								if(spatch.edge[2] == i) {
									opposite1 = 0;
									opposite2 = 1;
									o1a = 3;
									o2a = 6;
									}
								else {
									opposite1 = 1;
									opposite2 = 2;
									o1a = 5;
									o2a = 0;
									}

								Point3 otherCenter = (patch->verts[spatch.v[opposite1]].p + patch->verts[spatch.v[opposite2]].p) / 2.0f;
								Point3 oppVec = edgeCenter - otherCenter;
								float oppLen = Length(oppVec);
								if(oppLen == 0.0f) {
									oppVec = Point3(0,0,1);
									oppLen = 1.0f;
									}
								Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
								Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
								Point3 n1a, n2a;
								if(Length(v1a) > 0.0f)
									n1a = Normalize(v1a);
								else
									n1a = Normalize(oppVec);
								if(Length(v2a) > 0.0f)
									n2a = Normalize(v2a);
								else
									n2a = Normalize(oppVec);

								// Compute the new vert position
								Point3 newPos1 = p1 - n1a * oppLen;
								Point3 newPos2 = p2 - n2a * oppLen;
								patch->verts[verts].p = newPos1;
								patch->verts[verts+1].p = newPos2;

								// Compute the vectors
								patch->vecs[vecs].p = p1 - v1a;
								patch->vecs[vecs+1].p = newPos1 - (newPos1 - p1) / 3.0f;
								patch->vecs[vecs+2].p = newPos1 + (v12 - p1);
								patch->vecs[vecs+3].p = newPos2 + (v21 - p2);
								patch->vecs[vecs+4].p = newPos2 + (p2 - newPos2) / 3.0f;
								patch->vecs[vecs+5].p = p2 - v2a;
								}
								break;
							}
						break;
					}
				}
			}
		}
	patch->computeInteriors();
	patch->buildLinkages();
	// This step welds all new identical verts
	if(postWeld && (patch->getNumVerts() != lastVert))
		patch->Weld(0.0f, TRUE, lastVert);
	}

BOOL PatchAddRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	AddPatches(type, patch, postWeld);
	return TRUE;
	}

#define PADDR_TYPE_CHUNK		0x1000
#define PADDR_PATCH_CHUNK		0x1010
#define PADDR_POSTWELD_CHUNK	0x1020

IOResult PatchAddRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	postWeld = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PADDR_TYPE_CHUNK:
				res = iload->Read(&type,sizeof(int),&nb);
				break;
//			case PADDR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			// If the following chunk is present, it's a MAX 2.0 file and a post-addition
			// weld is to be performed
			case PADDR_POSTWELD_CHUNK:
				postWeld = TRUE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

class NewEdge {
	public:
		int oldEdge;
		int v1;
		int vec12;
		int vec21;
		int v2;
		int vec23;
		int vec32;
		int v3;
		NewEdge() { oldEdge = v1 = v2 = v3 = vec12 = vec21 = vec23 = vec32 = -1; }
	};

class PatchDivInfo {
	public:
		BOOL div02;
		BOOL div13;
		PatchDivInfo() { div02 = div13 = FALSE; }
	};

// Compute midpoint division for patch vectors -- Provide patchmesh, patch number, 4 bez points
// returns 2 new vectors

static Point3 InterpCenter(PatchMesh *patch, int index, int e1, int i1, int i2, int e2, Point3 *v1=NULL, Point3 *v2=NULL, Point3 *v3=NULL, Point3 *v4=NULL) {
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 e1i1 = (v[p.vec[e1]].p + v[p.interior[i1]].p) / 2.0f;
	Point3 i1i2 = (v[p.interior[i1]].p + v[p.interior[i2]].p) / 2.0f;
	Point3 i2e2 = (v[p.interior[i2]].p + v[p.vec[e2]].p) / 2.0f;
	Point3 a = (e1i1 + i1i2) / 2.0f;
	Point3 b = (i1i2 + i2e2) / 2.0f;
	if(v1) *v1 = e1i1;
	if(v2) *v2 = a;
	if(v3) *v3 = b;
	if(v4) *v4 = i2e2;
	return (a + b) / 2.0f;
	}

static Point3 InterpCenter(PatchMesh *patch, int index, int e1, int i1, int e2, Point3 *v1=NULL, Point3 *v2=NULL) {
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 a = (p.aux[e1] + v[p.interior[i1]].p) / 2.0f;
	Point3 b = (v[p.interior[i1]].p + p.aux[e2]) / 2.0f;
	if(v1) *v1 = a;
	if(v2) *v2 = b;
	return (a + b) / 2.0f;
	}

static Point3 InterpCenter(Point3 e1, Point3 i1, Point3 i2, Point3 e2, Point3 *v1=NULL, Point3 *v2=NULL, Point3 *v3=NULL, Point3 *v4=NULL ) {
	Point3 e1i1 = (e1 + i1) / 2.0f;
	Point3 i1i2 = (i1 + i2) / 2.0f;
	Point3 i2e2 = (i2 + e2) / 2.0f;
	Point3 a = (e1i1 + i1i2) / 2.0f;
	Point3 b = (i1i2 + i2e2) / 2.0f;
	if(v1) *v1 = e1i1;
	if(v2) *v2 = a;
	if(v3) *v3 = b;
	if(v4) *v4 = i2e2;
	return (a + b) / 2.0f;
	}

static Point3 InterpCenter(Point3 e1, Point3 i1, Point3 e2, Point3 *v1=NULL, Point3 *v2=NULL) {
	Point3 a = (e1 + i1) / 2.0f;
	Point3 b = (i1 + e2) / 2.0f;
	if(v1) *v1 = a;
	if(v2) *v2 = b;
	return (a + b) / 2.0f;
	}

static Point3 InterpEdge(PatchMesh *patch, int index, float pct, int c1, int e1, int e2, int c2, Point3 *v1=NULL, Point3 *v2=NULL, Point3 *v3=NULL, Point3 *v4=NULL) {
	PatchVert *vert = patch->verts;
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 pv1 = vert[p.v[c1]].p;
	Point3 pv2 = vert[p.v[c2]].p;
	Point3 pe1 = v[p.vec[e1]].p;
	Point3 pe2 = v[p.vec[e2]].p;
	Point3 v1e1 = pv1 + (pe1 - pv1) * pct;
	Point3 e1e2 = pe1 + (pe2 - pe1) * pct;
	Point3 e2v2 = pe2 + (pv2 - pe2) * pct;
	Point3 a = v1e1 + (e1e2 - v1e1) * pct;
	Point3 b = e1e2 + (e2v2 - e1e2) * pct;
	if(v1) *v1 = v1e1;
	if(v2) *v2 = a;
	if(v3) *v3 = b;
	if(v4) *v4 = e2v2;
	return a + (b - a) * pct;
	}

static Point3 InterpPoint(PatchMesh *patch, int index, float pct, int e1, int i1, int i2, int e2, Point3 *v1=NULL, Point3 *v2=NULL, Point3 *v3=NULL, Point3 *v4=NULL) {
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 pe1 = v[p.vec[e1]].p;
	Point3 pe2 = v[p.vec[e2]].p;
	Point3 pi1 = v[p.interior[i1]].p;
	Point3 pi2 = v[p.interior[i2]].p;
	Point3 e1i1 = pe1 + (pi1 - pe1) * pct;
	Point3 i1i2 = pi1 + (pi2 - pi1) * pct;
	Point3 i2e2 = pi2 + (pe2 - pi2) * pct;
	Point3 a = e1i1 + (i1i2 - e1i1) * pct;
	Point3 b = i1i2 + (i2e2 - i1i2) * pct;
	if(v1) *v1 = e1i1;
	if(v2) *v2 = a;
	if(v3) *v3 = b;
	if(v4) *v4 = i2e2;
	return a + (b - a) * pct;
	}

static Point3 InterpPoint(float pct, Point3 e1, Point3 i1, Point3 i2, Point3 e2, Point3 *v1=NULL, Point3 *v2=NULL, Point3 *v3=NULL, Point3 *v4=NULL ) {
	Point3 e1i1 = e1 + (i1 - e1) * pct;
	Point3 i1i2 = i1 + (i2 - i1) * pct;
	Point3 i2e2 = i2 + (e2 - i2) * pct;
	Point3 a = e1i1 + (i1i2 - e1i1) * pct;
	Point3 b = i1i2 + (i2e2 - i1i2) * pct;
	if(v1) *v1 = e1i1;
	if(v2) *v2 = a;
	if(v3) *v3 = b;
	if(v4) *v4 = i2e2;
	return a + (b - a) * pct;
	}

static Point3 InterpLinear(Point3 a, Point3 b, float interp) {
	return a + (a - b) * interp;
	}

static Point3 InterpDegree2(Point3 a, Point3 b, Point3 c, float interp) {
	Point3 ab = a + (b - a) * interp;
	Point3 bc = b + (c - b) * interp;
	return ab + (bc - ab) * interp;
	}

static Point3 InterpDegree3(Point3 a, Point3 b, Point3 c, Point3 d, float interp) {
	Point3 ab = a + (b - a) * interp;
	Point3 bc = b + (c - b) * interp;
	Point3 cd = c + (d - c) * interp;
	Point3 abbc = ab + (bc - ab) * interp;
	Point3 bccd = bc + (cd - bc) * interp;
	return abbc + (bccd - abbc) * interp;
	}

// Handy fractional constants
#define _1_16 0.0625f
#define _1_8 0.125f
#define _3_16 0.1875f
#define _1_4 0.25f

static Point3 GetOuterInside(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f) {
	return a * _1_8 + b * _1_8 + c * _1_4 + d * _1_8 + e * _1_4 + f * _1_8;
	}

static Point3 GetNewEdgeVec(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h) {
	return a * _1_16 + b * _1_16 + c * _3_16 + d * _3_16 + e * _1_16 + f * _1_16 + g * _3_16 + h * _3_16;
	}

static Point3 GetCentralInterior(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h, Point3 i, Point3 j) {
	return a * _1_16 + b * _1_8 + c * _1_16 + d * _1_16 + e * _1_16 + f * _1_16 + g * _1_16 + h * _3_16 + i * _3_16 + j * _1_8;
	}

static Point3 GetNewEdgeCenter(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h, Point3 i) {
	return a * _1_16 + b * _1_8 + c * _1_16 + d * _1_8 + e * _1_16 + f * _1_16 + g * _1_4 + h * _1_8 + i * _1_8;
	}

static Point3 GetOuterOutside(Point3 a, Point3 b, Point3 c, Point3 d) {
	return a * _1_4 + b * _1_4 + c * _1_4 + d * _1_4;
	}

static void FindNewTriEdge(PatchMesh *patch, Patch &p, int vert, Point3 &e1, Point3 &e2, Point3 &e3) {
	int a = vert;
	int b = vert * 3;
	int c = b + 1;
	int d = (b + 8) % 9;
	int e = (b + 7) % 9;
	int f = (b + 4) % 9;
	int g = vert;
	int h = (g + 1) % 3;
	int i = (g + 2) % 3;
	int j = (b + 6) % 9;
	int k = (b + 5) % 9;
	int l = b + 2;
	int m = (b + 3) % 9;
	Point3 pa = patch->verts[p.v[a]].p;
	Point3 pb = p.aux[b];
	Point3 pc = p.aux[c];
	Point3 pd = p.aux[d];
	Point3 pe = p.aux[e];
	Point3 pf = p.aux[f];
	Point3 pg = patch->vecs[p.interior[g]].p;
	Point3 ph = patch->vecs[p.interior[h]].p;
	Point3 pi = patch->vecs[p.interior[i]].p;
	Point3 pj = p.aux[j];
	Point3 pk = p.aux[k];
	Point3 pl = p.aux[l];
	Point3 pm = p.aux[m];
	e1 = GetNewEdgeVec(pa,pb,pd,pe,pj,pk,pg,pi);
	e2 = GetNewEdgeCenter(pa,pb,pc,pd,pe,pf,pg,ph,pi);
	e3 = GetNewEdgeVec(pa,pd,pb,pc,pl,pm,pg,ph);
	}

static void FindNewOuterTriInteriors(PatchMesh *patch, Patch &p, int vert, Point3 &i1, Point3 &i2, Point3 &i3) {
	int a = vert;
	int b = vert * 3;
	int c = (b + 8) % 9;
	int d = (b + 7) % 9;
	int e = vert;
	int f = (e + 2) % 3;
	int g = b + 1;
	int h = (e + 1) % 3;
	Point3 pa = patch->verts[p.v[a]].p;
	Point3 pb = p.aux[b];
	Point3 pc = p.aux[c];
	Point3 pd = p.aux[d];
	Point3 pe = patch->vecs[p.interior[e]].p;
	Point3 pf = patch->vecs[p.interior[f]].p;
	Point3 pg = p.aux[g];
	Point3 ph = patch->vecs[p.interior[h]].p;
	i1 = GetOuterOutside(pa,pb,pc,pe);
	i2 = GetOuterInside(pa,pc,pb,pg,pe,ph);
	i3 = GetOuterInside(pa,pb,pc,pd,pe,pf);
	}

static void FindNewInnerTriInteriors(PatchMesh *patch, Patch &p, Point3 &i1, Point3 &i2, Point3 &i3) {
	Point3 pa = p.aux[0];
	Point3 pb = p.aux[1];
	Point3 pc = p.aux[2];
	Point3 pd = p.aux[3];
	Point3 pe = p.aux[4];
	Point3 pf = p.aux[5];
	Point3 pg = p.aux[6];
	Point3 ph = p.aux[7];
	Point3 pi = p.aux[8];
	Point3 pj = patch->vecs[p.interior[0]].p;
	Point3 pk = patch->vecs[p.interior[1]].p;
	Point3 pl = patch->vecs[p.interior[2]].p;
	i1 = GetCentralInterior(pa,pb,pc,pi,ph,pe,pd,pj,pk,pl);
	i2 = GetCentralInterior(pd,pe,pf,pc,pb,ph,pg,pk,pl,pj);
	i3 = GetCentralInterior(pg,ph,pi,pf,pe,pb,pa,pl,pj,pk);
	}

// This is a first shot at a degree reducer which turns a degree-4 curve into a degree-3 curve,
// it probably won't give very good results unless the curve was converted from degree 3 to degree 4
// returns just the vector points
static void CubicFromQuartic(Point3 q1, Point3 q2, Point3 q3, Point3 q4, Point3 q5, Point3 &c2, Point3 &c3) {
	c2 = q1 + (q2 - q1) * 1.33333f;
	c3 = q5 + (q4 - q5) * 1.33333f;
	}

#define SUBDIV_EDGES 0
#define SUBDIV_PATCHES 1

static void SubdividePatch(int type, BOOL propagate, PatchMesh *patch) {
	int i;

	int verts = patch->getNumVerts();
	int vecs = patch->getNumVecs();
	int edges = patch->getNumEdges();
	int patches = patch->getNumPatches();

	// Make an edge flags array to note which edges must be processed
	BitArray eDiv(edges);
	// Make a patch flags array to note which patches must be processed
	BitArray pDiv(patches);
	// Make an edge flags array to note which edges have been done
	BitArray eDone(edges);
	eDone.ClearAll();
	// Make a patch flags array to note which patches have been done
	BitArray pDone(patches);
	pDone.ClearAll();

	switch(type) {
		case SUBDIV_EDGES:
			if(!patch->edgeSel.NumberSet())
				return;		// Nothing to do!
			eDiv = patch->edgeSel;
			pDiv.ClearAll();
			break;
		case SUBDIV_PATCHES:
			if(!patch->patchSel.NumberSet())
				return;		// Nothing to do!
			eDiv.ClearAll();
			pDiv = patch->patchSel;
			for(i = 0; i < patches; ++i) {
				if(pDiv[i]) {
					Patch &p = patch->patches[i];
					// Mark all edges for division
					eDiv.Set(p.edge[0]);
					eDiv.Set(p.edge[1]);
					eDiv.Set(p.edge[2]);
					if(p.type == PATCH_QUAD)
						eDiv.Set(p.edge[3]);
					}
				}
			// If not propagating, mark the edges as done
			if(!propagate)
				eDone = eDiv;
			break;
		}

	BOOL more = TRUE;
	while(more) {
		BOOL altered = FALSE;
		for(i = 0; i < edges; ++i) {
			if(eDiv[i] && !eDone[i]) {
				PatchEdge &e = patch->edges[i];
				for(int j = 0; j < e.patches.Count(); ++j)
					pDiv.Set(e.patches[j]);
				eDone.Set(i);
				altered = TRUE;
				}
			}
		if(altered && propagate) {
			for(i = 0; i < patches; ++i) {
				if(pDiv[i] && !pDone[i]) {
					Patch &p = patch->patches[i];
					if(p.type == PATCH_TRI) {	// Triangle -- tag all edges for division
						eDiv.Set(p.edge[0]);
						eDiv.Set(p.edge[1]);
						eDiv.Set(p.edge[2]);
						}
					else {		// Quad -- Tag edges opposite tagged edges
						if(eDiv[p.edge[0]])
							eDiv.Set(p.edge[2]);
						if(eDiv[p.edge[1]])
							eDiv.Set(p.edge[3]);
						if(eDiv[p.edge[2]])
							eDiv.Set(p.edge[0]);
						if(eDiv[p.edge[3]])
							eDiv.Set(p.edge[1]);
						}
					pDone.Set(i);
					}
				}
			}
		else
			more = FALSE;
		}

	// Keep a count of the new interior vectors
	int newInteriors = 0;

	// Also keep a count of the new vertices inside double-divided quads
	int newCenters = 0;

	// And a count of new texture vertices
	Tab<int> newTVerts;
	newTVerts.SetCount (patch->getNumMaps() + NUM_HIDDENMAPS);
	for(int chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan)
		newTVerts[chan+NUM_HIDDENMAPS] = 0;

	// And a count of new patches
	int newPatches = 0;

	int divPatches = pDiv.NumberSet();
	PatchDivInfo *pInfo = new PatchDivInfo [divPatches];
	int pDivIx;

	// Tag the edges that are on tagged patches but aren't tagged (only happens in propagate=0)
	// And set up a table with useful division info
	for(i = 0, pDivIx = 0; i < patches; ++i) {
		if(pDiv[i]) {
			PatchDivInfo &pi = pInfo[pDivIx];
			Patch &p = patch->patches[i];
			if(p.type == PATCH_TRI) {	// Triangle -- tag all edges for division
				eDiv.Set(p.edge[0]);
				eDiv.Set(p.edge[1]);
				eDiv.Set(p.edge[2]);
				newInteriors += (6 + 12);
				newPatches += 4;
				for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) 
					{
					int nchan = NUM_HIDDENMAPS + chan;
					if(patch->mapPatches(chan))
						{
						newTVerts[nchan] += 3;
						if (patch->ArePatchesCurvedMapped(i))
							{
							newTVerts[nchan] += 18;
							if (!(p.flags&PATCH_AUTO))
								newTVerts[nchan] += 12;
							}
						}
					}
				}
			else {		// Quad -- Tag edges opposite tagged edges
				int divs = 0;
				pi.div02 = pi.div13 = FALSE;
				if(eDiv[p.edge[0]]) {
					eDiv.Set(p.edge[2]);
					divs++;
					pi.div02 = TRUE;
					}
				else
				if(eDiv[p.edge[2]]) {
					eDiv.Set(p.edge[0]);
					divs++;
					pi.div02 = TRUE;
					}
				if(eDiv[p.edge[1]]) {
					eDiv.Set(p.edge[3]);
					divs++;
					pi.div13 = TRUE;
					}
				else
				if(eDiv[p.edge[3]]) {
					eDiv.Set(p.edge[1]);
					divs++;
					pi.div13 = TRUE;
					}
				newPatches += (divs==1) ? 2 : 4;
				newInteriors += (divs==1) ? (2 + 8) : (8 + 16);
				for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
					int nchan = chan + NUM_HIDDENMAPS;
					if(patch->mapPatches(chan)) {
						if(divs == 2)
							{
							newTVerts[nchan] += 5;
							if (patch->ArePatchesCurvedMapped(i))
								{
								newTVerts[nchan] += 24;
								if (!(p.flags&PATCH_AUTO))
									newTVerts[nchan] += 16;
								}
							}
						else
							{
							newTVerts[nchan] += 2;
							if (patch->ArePatchesCurvedMapped(i))
								{
								newTVerts[nchan] += 14;
								if (!(p.flags&PATCH_AUTO))
									newTVerts[nchan] += 8;
								}
							}	
						}
					}
				if(divs==2)
					newCenters++;
				}
			pDivIx++;
			}
		}

	// Figure out how many new verts and vecs we'll need...
	int divEdges = eDiv.NumberSet();
	int newVerts = divEdges + newCenters;		// 1 new vert per edge
	int newVecs = divEdges * 4 + newInteriors;	// 4 new vectors per edge + new interior verts

	int vert = verts;
	Tab<int> tvert;
	tvert.SetCount (patch->getNumMaps() + NUM_HIDDENMAPS);
	Tab<int> tverts;
	tverts.SetCount (patch->getNumMaps() + NUM_HIDDENMAPS);
	Tab<int> tpat;
	tpat.SetCount (patch->getNumMaps() + NUM_HIDDENMAPS);
	for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
		int nchan = chan + NUM_HIDDENMAPS;
		tverts[nchan] = tvert[nchan] = patch->getNumMapVerts (chan);
		tpat[nchan] = patches;
	}
	int vec = vecs;
	int pat = patches;

	// Add the new vertices
	patch->setNumVerts(verts + newVerts, TRUE);

	// Add the new texture vertices
	for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
		int nchan = chan + NUM_HIDDENMAPS;
		patch->setNumMapVerts (chan, tverts[nchan] + newTVerts[nchan], TRUE);
	}

	// Add the new vectors
	patch->setNumVecs(vecs + newVecs, TRUE);

	// Add the new patches
	patch->setNumPatches(patches + newPatches, TRUE);

	// Create a new edge map
	NewEdge *eMap = new NewEdge [edges];
	for(i = 0; i < edges; ++i) {
		if(eDiv[i]) {
			PatchEdge &edge = patch->edges[i];
			NewEdge &map = eMap[i];
			map.oldEdge = i;
			map.v1 = edge.v1;
			map.vec12 = vec++;
			map.vec21 = vec++;
			map.v2 = vert++;
			map.vec23 = vec++;
			map.vec32 = vec++;
			map.v3 = edge.v2;
			
			// Compute the new edge vertex and vectors
			Point3 v00 = patch->verts[edge.v1].p;
			Point3 v10 = patch->vecs[edge.vec12].p;
			Point3 v20 = patch->vecs[edge.vec21].p;
			Point3 v30 = patch->verts[edge.v2].p;
			Point3 v01 = (v10 + v00) / 2.0f;
			Point3 v21 = (v30 + v20) / 2.0f;
			Point3 v11 = (v20 + v10) / 2.0f;
			Point3 v02 = (v11 + v01) / 2.0f;
			Point3 v12 = (v21 + v11) / 2.0f;
			Point3 v03 = (v12 + v02) / 2.0f;

			patch->verts[map.v2].p = v03;
			patch->vecs[map.vec12].p = v01;
			patch->vecs[map.vec21].p = v02;
			patch->vecs[map.vec23].p = v12;
			patch->vecs[map.vec32].p = v21;
			}
		}

#ifdef DUMPING
// Dump edge map
DebugPrint("Edge map:\n");
for(i = 0; i < edges; ++i) {
	NewEdge &e = eMap[i];
	DebugPrint("Old edge: %d  New edge: %d (%d %d) %d (%d %d) %d\n",e.oldEdge,e.v1,e.vec12,e.vec21,e.v2,e.vec23,e.vec32,e.v3);
	}
#endif

	// Now go and subdivide them!

	for(i = 0, pDivIx = 0; i < patches; ++i) {
		if(pDiv[i]) {
			PatchDivInfo &pi = pInfo[pDivIx];
			Patch &p = patch->patches[i];
			if(p.type == PATCH_TRI) {
				// Need to create four new patches
				int newev1 = vec++;	// edge 0 -> edge 1
				int newev2 = vec++;	// edge 1 -> edge 0
				int newev3 = vec++;	// edge 1 -> edge 2
				int newev4 = vec++;	// edge 2 -> edge 1
				int newev5 = vec++;	// edge 2 -> edge 0
				int newev6 = vec++;	// edge 0 -> edge 2

				// Get pointers to new edges
				NewEdge &e0 = eMap[p.edge[0]];
				NewEdge &e1 = eMap[p.edge[1]];
				NewEdge &e2 = eMap[p.edge[2]];

				// See if edges need to be flopped
				BOOL flop0 = (e0.v1 == p.v[0]) ? FALSE : TRUE;
				BOOL flop1 = (e1.v1 == p.v[1]) ? FALSE : TRUE;
				BOOL flop2 = (e2.v1 == p.v[2]) ? FALSE : TRUE;

				// Create the four new patches
				Patch &p1 = patch->patches[pat++];
				Patch &p2 = patch->patches[pat++];
				Patch &p3 = patch->patches[pat++];
				Patch &p4 = patch->patches[pat++];

				p1.SetType(PATCH_TRI);
				p1.v[0] = e0.v2;
				p1.v[1] = flop1 ? e1.v3 : e1.v1;
				p1.v[2] = e1.v2;
				p1.vec[0] = flop0 ? e0.vec21 : e0.vec23;
				p1.vec[1] = flop0 ? e0.vec12 : e0.vec32;
				p1.vec[2] = flop1 ? e1.vec32 : e1.vec12;
				p1.vec[3] = flop1 ? e1.vec23 : e1.vec21;
				p1.vec[4] = newev2;
				p1.vec[5] = newev1;
				p1.interior[0] = vec++;
				p1.interior[1] = vec++;
				p1.interior[2] = vec++;
				p1.smGroup = p.smGroup;
				p1.flags = p.flags;

				p2.SetType(PATCH_TRI);
				p2.v[0] = e1.v2;
				p2.v[1] = flop2 ? e2.v3 : e2.v1;
				p2.v[2] = e2.v2;
				p2.vec[0] = flop1 ? e1.vec21 : e1.vec23;
				p2.vec[1] = flop1 ? e1.vec12 : e1.vec32;
				p2.vec[2] = flop2 ? e2.vec32 : e2.vec12;
				p2.vec[3] = flop2 ? e2.vec23 : e2.vec21;
				p2.vec[4] = newev4;
				p2.vec[5] = newev3;
				p2.interior[0] = vec++;
				p2.interior[1] = vec++;
				p2.interior[2] = vec++;
				p2.smGroup = p.smGroup;
				p2.flags = p.flags;

				p3.SetType(PATCH_TRI);
				p3.v[0] = e0.v2;
				p3.v[1] = e1.v2;
				p3.v[2] = e2.v2;
				p3.vec[0] = newev1;
				p3.vec[1] = newev2;
				p3.vec[2] = newev3;
				p3.vec[3] = newev4;
				p3.vec[4] = newev5;
				p3.vec[5] = newev6;
				p3.interior[0] = vec++;
				p3.interior[1] = vec++;
				p3.interior[2] = vec++;
				p3.smGroup = p.smGroup;
				p3.flags = p.flags;

				p4.SetType(PATCH_TRI);
				p4.v[0] = flop0 ? e0.v3 : e0.v1;
				p4.v[1] = e0.v2;
				p4.v[2] = e2.v2;
				p4.vec[0] = flop0 ? e0.vec32 : e0.vec12;
				p4.vec[1] = flop0 ? e0.vec23 : e0.vec21;
				p4.vec[2] = newev6;
				p4.vec[3] = newev5;
				p4.vec[4] = flop2 ? e2.vec21 : e2.vec23;
				p4.vec[5] = flop2 ? e2.vec12 : e2.vec32;
				p4.interior[0] = vec++;
				p4.interior[1] = vec++;
				p4.interior[2] = vec++;
				p4.smGroup = p.smGroup;
				p4.flags = p.flags;

				// If this patch is textured, create three new texture verts for it
				for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
					int nchan = chan + NUM_HIDDENMAPS;
					if(patch->mapPatches(chan)) {
						int tva = tvert[nchan]++;
						int tvb = tvert[nchan]++;
						int tvc = tvert[nchan]++;
						TVPatch &tp = patch->mapPatches(chan)[i];
						TVPatch &tp1 = patch->mapPatches(chan)[tpat[nchan]++];
						TVPatch &tp2 = patch->mapPatches(chan)[tpat[nchan]++];
						TVPatch &tp3 = patch->mapPatches(chan)[tpat[nchan]++];
						TVPatch &tp4 = patch->mapPatches(chan)[tpat[nchan]++];
//need to compute handles if neccessary
						Point3 tvi[3];
						for (int vi = 0; vi <3; vi++)
							tvi[vi] = patch->mapVerts(chan)[tp.tv[vi]].p;

						if (patch->ArePatchesCurvedMapped(i))
							{

							if ( (tp.handles[0] == -1) || (tp.handles[1] == -1) || (tp.handles[2] == -1) ||
								 (tp.handles[3] == -1) || (tp.handles[4] == -1) || (tp.handles[5] == -1) )
								{
								for (int k = 0; k < 6; k++)
									{
									tp1.handles[k] = -1;
									tp2.handles[k] = -1;
									tp3.handles[k] = -1;
									tp4.handles[k] = -1;
									}
	
								}
							else
								{
								PatchMesh msh;
								msh.setNumPatches(1);
								msh.setNumVerts(3);
								if (p.flags&PATCH_AUTO)
									{
									msh.setNumVecs(9);
									for (int k = 0; k < 6; k++)
										{
										int a = tp.handles[k];
										msh.setVec(k,patch->mapVerts(chan)[a].p);
										}	
									}
								else 
									{
									msh.setNumVecs(9);
									for (int k = 0; k < 6; k++)
										{
										int a = tp.handles[k];
										msh.setVec(k,patch->mapVerts(chan)[a].p);
										}	
									for (k = 0; k < 3; k++)
										{
										int a = tp.interiors[k];
										msh.setVec(k+6,patch->mapVerts(chan)[a].p);
										}	
									}

								msh.setVert(0,tvi[0]);
								msh.setVert(1,tvi[1]);
								msh.setVert(2,tvi[2]);


								msh.MakeTriPatch(0, 0, 0,1,
											 1, 2,3, 
											 2, 4, 5,
											 6, 7, 8,
											 0);
								msh.computeInteriors();
								msh.buildLinkages();

								msh.patchSel.Set(0,TRUE);
								msh.Subdivide(SUBDIV_PATCHES,FALSE);

								
								int a = msh.patches[0].vec[0];
								int b = msh.patches[0].vec[1];

								int c = msh.patches[0].vec[2];
								int d = msh.patches[0].vec[3];

								int e = msh.patches[0].vec[4];
								int f = msh.patches[0].vec[5];

//patch a
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
								tp1.handles[0] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
								tp1.handles[1] = tvert[nchan]++;

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
								tp1.handles[2] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
								tp1.handles[3] = tvert[nchan]++;

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
								tp1.handles[4] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
								tp1.handles[5] = tvert[nchan]++;

//patch b	
								a = msh.patches[1].vec[0];
								b = msh.patches[1].vec[1];

								c = msh.patches[1].vec[2];
								d = msh.patches[1].vec[3];

								e = msh.patches[1].vec[4];
								f = msh.patches[1].vec[5];



								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
								tp2.handles[0] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
								tp2.handles[1] = tvert[nchan]++;

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
								tp2.handles[2] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
								tp2.handles[3] = tvert[nchan]++;

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
								tp2.handles[4] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
								tp2.handles[5] = tvert[nchan]++;
	

//patch c
								a = msh.patches[2].vec[0];
								b = msh.patches[2].vec[1];
	
								c = msh.patches[2].vec[2];
								d = msh.patches[2].vec[3];
	
								e = msh.patches[2].vec[4];
								f = msh.patches[2].vec[5];
	


								tp3.handles[0] = tp1.handles[5];
								tp3.handles[1] = tp1.handles[4];

								tp3.handles[2] = tp2.handles[5];
								tp3.handles[3] = tp2.handles[4];

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
								tp3.handles[4] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
								tp3.handles[5] = tvert[nchan]++;

//patch d
	
								a = msh.patches[3].vec[0];
								b = msh.patches[3].vec[1];

								c = msh.patches[3].vec[2];
								d = msh.patches[3].vec[3];

								e = msh.patches[3].vec[4];
								f = msh.patches[3].vec[5];

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
								tp4.handles[0] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
								tp4.handles[1] = tvert[nchan]++;


								tp4.handles[2] = tp3.handles[5];
								tp4.handles[3] = tp3.handles[4];

								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
								tp4.handles[4] = tvert[nchan]++;
								patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
								tp4.handles[5] = tvert[nchan]++;
	

		
								if (!(p.flags&PATCH_AUTO))
									{
									a = msh.patches[0].interior[0];
									b = msh.patches[0].interior[1];
									c = msh.patches[0].interior[2];
		
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp1.interiors[0] = tvert[nchan]++;
	
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp1.interiors[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp1.interiors[2] = tvert[nchan]++;


									a = msh.patches[1].interior[0];
									b = msh.patches[1].interior[1];
									c = msh.patches[1].interior[2];
	
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp2.interiors[0] = tvert[nchan]++;
	
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp2.interiors[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp2.interiors[2] = tvert[nchan]++;

									a = msh.patches[2].interior[0];
									b = msh.patches[2].interior[1];
									c = msh.patches[2].interior[2];

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp3.interiors[0] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp3.interiors[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp3.interiors[2] = tvert[nchan]++;


									a = msh.patches[3].interior[0];
									b = msh.patches[3].interior[1];
									c = msh.patches[3].interior[2];
								
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;

									tp4.interiors[0] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp4.interiors[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp4.interiors[2] = tvert[nchan]++;
									}
								}
	

							}

						tp1.tv[0] = tva;
						tp1.tv[1] = tp.tv[1];
						tp1.tv[2] = tvb;
						tp2.tv[0] = tvb;
						tp2.tv[1] = tp.tv[2];
						tp2.tv[2] = tvc;
						tp3.tv[0] = tva;
						tp3.tv[1] = tvb;
						tp3.tv[2] = tvc;
						tp4.tv[0] = tp.tv[0];
						tp4.tv[1] = tva;
						tp4.tv[2] = tvc;
						patch->mapVerts(chan)[tva].p = (patch->mapVerts(chan)[tp.tv[0]].p + patch->mapVerts(chan)[tp.tv[1]].p) / 2.0f;
						patch->mapVerts(chan)[tvb].p = (patch->mapVerts(chan)[tp.tv[1]].p + patch->mapVerts(chan)[tp.tv[2]].p) / 2.0f;
						patch->mapVerts(chan)[tvc].p = (patch->mapVerts(chan)[tp.tv[2]].p + patch->mapVerts(chan)[tp.tv[0]].p) / 2.0f;
						}
					}

				// Now we'll compute the vectors for the three new edges being created inside this patch
				// These come back as degree 4's, and we need to reduce them to degree 3 for use in our
				// edges -- This is a bit risky because we aren't guaranteed a perfect fit.
				Point3 i1, i2, i3, i4, i5, i6, i7, i8, i9;
				FindNewTriEdge(patch, p, 0, i1, i2, i3);
				FindNewTriEdge(patch, p, 1, i4, i5, i6);
				FindNewTriEdge(patch, p, 2, i7, i8, i9);
				Point3 v1, v2, v3, v4, v5, v6;
				CubicFromQuartic(patch->verts[e2.v2].p, i1, i2, i3, patch->verts[e0.v2].p, v1, v2);
				CubicFromQuartic(patch->verts[e0.v2].p, i4, i5, i6, patch->verts[e1.v2].p, v3, v4);
				CubicFromQuartic(patch->verts[e1.v2].p, i7, i8, i9, patch->verts[e2.v2].p, v5, v6);
				patch->vecs[newev1].p = v3;
				patch->vecs[newev2].p = v4;
				patch->vecs[newev3].p = v5;
				patch->vecs[newev4].p = v6;
				patch->vecs[newev5].p = v1;
				patch->vecs[newev6].p = v2;
				// Now compute the interior vectors for the new patches if the one we're dividing isn't automatic
				// Must compute vectors for this patch's divided edges
				if(!(p.flags & PATCH_AUTO)) {
					p1.flags &= ~PATCH_AUTO;
					p2.flags &= ~PATCH_AUTO;
					p3.flags &= ~PATCH_AUTO;
					p4.flags &= ~PATCH_AUTO;

					FindNewOuterTriInteriors(patch, p, 1, patch->vecs[p1.interior[1]].p, patch->vecs[p1.interior[2]].p, patch->vecs[p1.interior[0]].p);
					FindNewOuterTriInteriors(patch, p, 2, patch->vecs[p2.interior[1]].p, patch->vecs[p2.interior[2]].p, patch->vecs[p2.interior[0]].p);
					FindNewInnerTriInteriors(patch, p, patch->vecs[p3.interior[0]].p, patch->vecs[p3.interior[1]].p, patch->vecs[p3.interior[2]].p);
					FindNewOuterTriInteriors(patch, p, 0, patch->vecs[p4.interior[0]].p, patch->vecs[p4.interior[1]].p, patch->vecs[p4.interior[2]].p);
					}
				}
			else {		// Quad patch
				// Check division flags to see how many patches we'll need
				if(pi.div02 && pi.div13) {		// Divide both ways
					// Need a new central vertex
					Point3 newc = p.interp(patch, 0.5f, 0.5f);
					patch->verts[vert].p = newc;
					int center = vert++;

					// Need to create four new patches
					int newev1 = vec++;	// edge 0 -> center
					int newev2 = vec++;	// center -> edge 0
					int newev3 = vec++;	// edge 1 -> center
					int newev4 = vec++;	// center -> edge 1
					int newev5 = vec++;	// edge 2 -> center
					int newev6 = vec++;	// center -> edge 2
					int newev7 = vec++;	// edge 3 -> center
					int newev8 = vec++;	// center -> edge 3

					// Get pointers to new edges
					NewEdge &e0 = eMap[p.edge[0]];
					NewEdge &e1 = eMap[p.edge[1]];
					NewEdge &e2 = eMap[p.edge[2]];
					NewEdge &e3 = eMap[p.edge[3]];

					// See if edges need to be flopped
					BOOL flop0 = (e0.v1 == p.v[0]) ? FALSE : TRUE;
					BOOL flop1 = (e1.v1 == p.v[1]) ? FALSE : TRUE;
					BOOL flop2 = (e2.v1 == p.v[2]) ? FALSE : TRUE;
					BOOL flop3 = (e3.v1 == p.v[3]) ? FALSE : TRUE;

					// Compute the new vectors for the dividing line
					Point3 w1,w2,w3,w4;
					w1 = InterpCenter(patch, i, 7, 0, 1, 2);
					w2 = InterpCenter(patch, i, 6, 3, 2, 3);
					w3 = InterpCenter(patch, i, 1, 1, 2, 4);
					w4 = InterpCenter(patch, i, 0, 0, 3, 5);
					Point3 new0 = patch->verts[e0.v2].p;
					Point3 new1 = patch->verts[e1.v2].p;
					Point3 new2 = patch->verts[e2.v2].p;
					Point3 new3 = patch->verts[e3.v2].p;
					InterpCenter(new0, w1, w2, new2, &patch->vecs[newev1].p, &patch->vecs[newev2].p, &patch->vecs[newev6].p, &patch->vecs[newev5].p);
					InterpCenter(new1, w3, w4, new3, &patch->vecs[newev3].p, &patch->vecs[newev4].p, &patch->vecs[newev8].p, &patch->vecs[newev7].p);

					// Create the four new patches
					Patch &p1 = patch->patches[pat++];
					Patch &p2 = patch->patches[pat++];
					Patch &p3 = patch->patches[pat++];
					Patch &p4 = patch->patches[pat++];

					p1.SetType(PATCH_QUAD);
					p1.v[0] = p.v[0];
					p1.v[1] = e0.v2;
					p1.v[2] = center;
					p1.v[3] = e3.v2;
					p1.vec[0] = flop0 ? e0.vec32 : e0.vec12;
					p1.vec[1] = flop0 ? e0.vec23 : e0.vec21;
					p1.vec[2] = newev1;
					p1.vec[3] = newev2;
					p1.vec[4] = newev8;
					p1.vec[5] = newev7;
					p1.vec[6] = flop3 ? e3.vec21 : e3.vec23;
					p1.vec[7] = flop3 ? e3.vec12 : e3.vec32;
					p1.interior[0] = vec++;
					p1.interior[1] = vec++;
					p1.interior[2] = vec++;
					p1.interior[3] = vec++;
					p1.smGroup = p.smGroup;
					p1.flags = p.flags;

					p2.SetType(PATCH_QUAD);
					p2.v[0] = p.v[1];
					p2.v[1] = e1.v2;
					p2.v[2] = center;
					p2.v[3] = e0.v2;
					p2.vec[0] = flop1 ? e1.vec32 : e1.vec12;
					p2.vec[1] = flop1 ? e1.vec23 : e1.vec21;
					p2.vec[2] = newev3;
					p2.vec[3] = newev4;
					p2.vec[4] = newev2;
					p2.vec[5] = newev1;
					p2.vec[6] = flop0 ? e0.vec21 : e0.vec23;
					p2.vec[7] = flop0 ? e0.vec12 : e0.vec32;
					p2.interior[0] = vec++;
					p2.interior[1] = vec++;
					p2.interior[2] = vec++;
					p2.interior[3] = vec++;
					p2.smGroup = p.smGroup;
					p2.flags = p.flags;

					p3.SetType(PATCH_QUAD);
					p3.v[0] = p.v[2];
					p3.v[1] = e2.v2;
					p3.v[2] = center;
					p3.v[3] = e1.v2;
					p3.vec[0] = flop2 ? e2.vec32 : e2.vec12;
					p3.vec[1] = flop2 ? e2.vec23 : e2.vec21;
					p3.vec[2] = newev5;
					p3.vec[3] = newev6;
					p3.vec[4] = newev4;
					p3.vec[5] = newev3;
					p3.vec[6] = flop1 ? e1.vec21 : e1.vec23;
					p3.vec[7] = flop1 ? e1.vec12 : e1.vec32;
					p3.interior[0] = vec++;
					p3.interior[1] = vec++;
					p3.interior[2] = vec++;
					p3.interior[3] = vec++;
					p3.smGroup = p.smGroup;
					p3.flags = p.flags;

					p4.SetType(PATCH_QUAD);
					p4.v[0] = p.v[3];
					p4.v[1] = e3.v2;
					p4.v[2] = center;
					p4.v[3] = e2.v2;
					p4.vec[0] = flop3 ? e3.vec32 : e3.vec12;
					p4.vec[1] = flop3 ? e3.vec23 : e3.vec21;
					p4.vec[2] = newev7;
					p4.vec[3] = newev8;
					p4.vec[4] = newev6;
					p4.vec[5] = newev5;
					p4.vec[6] = flop2 ? e2.vec21 : e2.vec23;
					p4.vec[7] = flop2 ? e2.vec12 : e2.vec32;
					p4.interior[0] = vec++;
					p4.interior[1] = vec++;
					p4.interior[2] = vec++;
					p4.interior[3] = vec++;
					p4.smGroup = p.smGroup;
					p4.flags = p.flags;

					// If this patch is textured, create five new texture verts for it
					for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
						int nchan = NUM_HIDDENMAPS + chan;
						if(patch->mapPatches(chan)) {
							int tva = tvert[nchan]++;
							int tvb = tvert[nchan]++;
							int tvc = tvert[nchan]++;
							int tvd = tvert[nchan]++;
							int tve = tvert[nchan]++;

							TVPatch &tp = patch->mapPatches(chan)[i];
							TVPatch &tp1 = patch->mapPatches(chan)[tpat[nchan]++];
							TVPatch &tp2 = patch->mapPatches(chan)[tpat[nchan]++];
							TVPatch &tp3 = patch->mapPatches(chan)[tpat[nchan]++];
							TVPatch &tp4 = patch->mapPatches(chan)[tpat[nchan]++];

							Point3 tvi[4];	
							for (int vi = 0; vi <4; vi++)
								tvi[vi] = patch->mapVerts(chan)[tp.tv[vi]].p;
//need to compute handles if neccessary
							if (patch->ArePatchesCurvedMapped(i))
								{
								if ( (tp.handles[0] == -1) || (tp.handles[1] == -1) || (tp.handles[2] == -1)||
									 (tp.handles[3] == -1) || (tp.handles[4] == -1) || (tp.handles[5] == -1) ||
									 (tp.handles[6] == -1) || (tp.handles[7] == -1)
									 )
									{
									for (int k = 0; k < 8; k++)
										{
										tp1.handles[k] = -1;
										tp2.handles[k] = -1;
										tp3.handles[k] = -1;
										tp4.handles[k] = -1;
										}

									}
								else
									{
	
									PatchMesh msh;
									msh.setNumPatches(1);
									msh.setNumVerts(4);
									if (p.flags&PATCH_AUTO)
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										}
									else 
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										for (k = 0; k < 4; k++)
											{
											int a = tp.interiors[k];
											msh.setVec(k+8,patch->mapVerts(chan)[a].p);
											}	
										}

									msh.setVert(0,tvi[0]);
									msh.setVert(1,tvi[1]);
									msh.setVert(2,tvi[2]);
									msh.setVert(3,tvi[3]);

									msh.MakeQuadPatch(0, 0, 0,1,
													 1, 2,3, 
													 2, 4, 5,
													 3, 6, 7, 
													 8, 9, 10,
													11, 0);


									msh.computeInteriors();
									msh.buildLinkages();

									msh.patchSel.Set(0,TRUE);


									msh.Subdivide(SUBDIV_PATCHES,FALSE);

									int vecStart = tvert[nchan];
							
									int a = msh.patches[0].vec[0];
									int b = msh.patches[0].vec[1];

									int c = msh.patches[0].vec[2];
									int d = msh.patches[0].vec[3];

									int e = msh.patches[0].vec[4];
									int f = msh.patches[0].vec[5];

									int g = msh.patches[0].vec[6];
									int h = msh.patches[0].vec[7];
//patch a
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp1.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp1.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp1.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp1.handles[3] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp1.handles[4] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp1.handles[5] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp1.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp1.handles[7] = tvert[nchan]++;
//patch b
									a = msh.patches[1].vec[0];
									b = msh.patches[1].vec[1];

									c = msh.patches[1].vec[2];
									d = msh.patches[1].vec[3];

									e = msh.patches[1].vec[4];
									f = msh.patches[1].vec[5];

									g = msh.patches[1].vec[6];
									h = msh.patches[1].vec[7];


									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp2.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp2.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp2.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp2.handles[3] = tvert[nchan]++;

									tp2.handles[4] = tp1.handles[3];
									tp2.handles[5] = tp1.handles[2];

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp2.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp2.handles[7] = tvert[nchan]++;

//patch c
									a = msh.patches[2].vec[0];
									b = msh.patches[2].vec[1];

									c = msh.patches[2].vec[2];
									d = msh.patches[2].vec[3];

									e = msh.patches[2].vec[4];
									f = msh.patches[2].vec[5];

									g = msh.patches[2].vec[6];
									h = msh.patches[2].vec[7];


									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp3.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp3.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp3.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp3.handles[3] = tvert[nchan]++;

//							patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp3.handles[4] = tp2.handles[3];
//							patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp3.handles[5] = tp2.handles[2];

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp3.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp3.handles[7] = tvert[nchan]++;

//patch d


									a = msh.patches[3].vec[0];
									b = msh.patches[3].vec[1];
	
									c = msh.patches[3].vec[2];
									d = msh.patches[3].vec[3];

									e = msh.patches[3].vec[4];
									f = msh.patches[3].vec[5];

									g = msh.patches[3].vec[6];
									h = msh.patches[3].vec[7];
	
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp4.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp4.handles[1] = tvert[nchan]++;

									tp4.handles[2] = tp1.handles[5];
									tp4.handles[3] = tp1.handles[4];

									tp4.handles[4] = tp3.handles[3];
									tp4.handles[5] = tp3.handles[2];
	
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp4.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp4.handles[7] = tvert[nchan]++;


									if (!(p.flags&PATCH_AUTO))
										{
										a = msh.patches[0].interior[0];
										b = msh.patches[0].interior[1];
										c = msh.patches[0].interior[2];
										d = msh.patches[0].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp1.interiors[0] = tvert[nchan]++;
	
										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp1.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp1.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp1.interiors[3] = tvert[nchan]++;


										a = msh.patches[1].interior[0];
										b = msh.patches[1].interior[1];
										c = msh.patches[1].interior[2];
										d = msh.patches[1].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp2.interiors[0] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp2.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp2.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp2.interiors[3] = tvert[nchan]++;

										a = msh.patches[2].interior[0];
										b = msh.patches[2].interior[1];
										c = msh.patches[2].interior[2];
										d = msh.patches[2].interior[3];
	
										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp3.interiors[0] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp3.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp3.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp3.interiors[3] = tvert[nchan]++;
	

										a = msh.patches[3].interior[0];
										b = msh.patches[3].interior[1];
										c = msh.patches[3].interior[2];
										d = msh.patches[3].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp4.interiors[0] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp4.interiors[1] = tvert[nchan]++;
	
										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp4.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp4.interiors[3] = tvert[nchan]++;
										}
									}
								}



							tp1.tv[0] = tp.tv[0];
							tp1.tv[1] = tva;
							tp1.tv[2] = tve;
							tp1.tv[3] = tvd;
							tp2.tv[0] = tp.tv[1];
							tp2.tv[1] = tvb;
							tp2.tv[2] = tve;
							tp2.tv[3] = tva;
							tp3.tv[0] = tp.tv[2];
							tp3.tv[1] = tvc;
							tp3.tv[2] = tve;
							tp3.tv[3] = tvb;
							tp4.tv[0] = tp.tv[3];
							tp4.tv[1] = tvd;
							tp4.tv[2] = tve;
							tp4.tv[3] = tvc;
							PatchTVert *ptv = patch->mapVerts(chan);
							ptv[tva].p = (ptv[tp.tv[0]].p + ptv[tp.tv[1]].p) / 2.0f;
							ptv[tvb].p = (ptv[tp.tv[1]].p + ptv[tp.tv[2]].p) / 2.0f;
							ptv[tvc].p = (ptv[tp.tv[2]].p + ptv[tp.tv[3]].p) / 2.0f;
							ptv[tvd].p = (ptv[tp.tv[3]].p + ptv[tp.tv[0]].p) / 2.0f;
							ptv[tve].p = (ptv[tp.tv[0]].p + ptv[tp.tv[1]].p + ptv[tp.tv[2]].p + ptv[tp.tv[3]].p) / 4.0f;
							}
						}

					// If it's not an auto patch, compute the new interior points
					if(!(p.flags & PATCH_AUTO)) {
						p1.flags &= ~PATCH_AUTO;
						p2.flags &= ~PATCH_AUTO;
						p3.flags &= ~PATCH_AUTO;
						p4.flags &= ~PATCH_AUTO;

						Point3 a,b,c,d;
						Point3 a1,b1,c1,d1;
						Point3 a2,b2,c2,d2;
						Point3 a3,b3,c3,d3;
						Point3 a4,b4,c4,d4;
						InterpEdge(patch, i, 0.5f, 0, 0, 1, 1, &a1, &b1, &c1, &d1);
						InterpCenter(patch, i, 7, 0, 1, 2, &a2, &b2, &c2, &d2);
						InterpCenter(patch, i, 6, 3, 2, 3, &a3, &b3, &c3, &d3);
						InterpEdge(patch, i, 0.5f, 3, 5, 4, 2, &a4, &b4, &c4, &d4);

						InterpCenter(a1, a2, a3, a4, &a, &b, &c, &d);
						patch->vecs[p1.interior[0]].p = a;
						patch->vecs[p1.interior[3]].p = b;
						patch->vecs[p4.interior[1]].p = c;
						patch->vecs[p4.interior[0]].p = d;
						InterpCenter(b1, b2, b3, b4, &a, &b, &c, &d);
						patch->vecs[p1.interior[1]].p = a;
						patch->vecs[p1.interior[2]].p = b;
						patch->vecs[p4.interior[2]].p = c;
						patch->vecs[p4.interior[3]].p = d;
						InterpCenter(c1, c2, c3, c4, &a, &b, &c, &d);
						patch->vecs[p2.interior[3]].p = a;
						patch->vecs[p2.interior[2]].p = b;
						patch->vecs[p3.interior[2]].p = c;
						patch->vecs[p3.interior[1]].p = d;
						InterpCenter(d1, d2, d3, d4, &a, &b, &c, &d);
						patch->vecs[p2.interior[0]].p = a;
						patch->vecs[p2.interior[1]].p = b;
						patch->vecs[p3.interior[3]].p = c;
						patch->vecs[p3.interior[0]].p = d;
						}
					}
				else
				if(pi.div02) {					// Divide edges 0 & 2
					// Need to create two new patches
					// Compute new edge vectors between new edge verts
					int newev1 = vec++;	// edge 0 -> edge 2
					int newev2 = vec++;	// edge 2 -> edge 0

					// Get pointers to new edges
					NewEdge &e0 = eMap[p.edge[0]];
					NewEdge &e2 = eMap[p.edge[2]];

					// See if edges need to be flopped
					BOOL flop0 = (e0.v1 == p.v[0]) ? FALSE : TRUE;
					BOOL flop2 = (e2.v1 == p.v[2]) ? FALSE : TRUE;

					// Compute the new vectors for the dividing line
					
					patch->vecs[newev1].p = InterpCenter(patch, i, 7, 0, 1, 2);
					patch->vecs[newev2].p = InterpCenter(patch, i, 6, 3, 2, 3);

					// Create the two new patches
					Patch &p1 = patch->patches[pat++];
					Patch &p2 = patch->patches[pat++];

					p1.SetType(PATCH_QUAD);
					p1.v[0] = flop0 ? e0.v3 : e0.v1;
					p1.v[1] = e0.v2;
					p1.v[2] = e2.v2;
					p1.v[3] = flop2 ? e2.v1 : e2.v3;
					p1.vec[0] = flop0 ? e0.vec32 : e0.vec12;
					p1.vec[1] = flop0 ? e0.vec23 : e0.vec21;
					p1.vec[2] = newev1;
					p1.vec[3] = newev2;
					p1.vec[4] = flop2 ? e2.vec21 : e2.vec23;
					p1.vec[5] = flop2 ? e2.vec12 : e2.vec32;
					p1.vec[6] = p.vec[6];
					p1.vec[7] = p.vec[7];
					p1.interior[0] = vec++;
					p1.interior[1] = vec++;
					p1.interior[2] = vec++;
					p1.interior[3] = vec++;

					p2.SetType(PATCH_QUAD);
					p2.v[0] = e0.v2;
					p2.v[1] = flop0 ? e0.v1 : e0.v3;
					p2.v[2] = flop2 ? e2.v3 : e2.v1;
					p2.v[3] = e2.v2;
					p2.vec[0] = flop0 ? e0.vec21 : e0.vec23;
					p2.vec[1] = flop0 ? e0.vec12 : e0.vec32;
					p2.vec[2] = p.vec[2];
					p2.vec[3] = p.vec[3];
					p2.vec[4] = flop2 ? e2.vec32 : e2.vec12;
					p2.vec[5] = flop2 ? e2.vec23 : e2.vec21;
					p2.vec[6] = newev2;
					p2.vec[7] = newev1;
					p2.interior[0] = vec++;
					p2.interior[1] = vec++;
					p2.interior[2] = vec++;
					p2.interior[3] = vec++;

					// If this patch is textured, create two new texture verts for it
					for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
						int nchan = chan + NUM_HIDDENMAPS;
						if(patch->mapPatches(chan)) {
							int tva = tvert[nchan]++;
							int tvb = tvert[nchan]++;
							TVPatch &tp = patch->mapPatches(chan)[i];
							TVPatch &tp1 = patch->mapPatches(chan)[tpat[nchan]++];
							TVPatch &tp2 = patch->mapPatches(chan)[tpat[nchan]++];
							Point3 tvi[4];
							for (int vi = 0; vi <4; vi++)
							tvi[vi] = patch->mapVerts(chan)[tp.tv[vi]].p;
//need to compute handles if neccessary
							if (patch->ArePatchesCurvedMapped(i))
								{
								if ( (tp.handles[0] == -1) || (tp.handles[1] == -1) || (tp.handles[2] == -1)||
									 (tp.handles[3] == -1) || (tp.handles[4] == -1) || (tp.handles[5] == -1) ||
									 (tp.handles[6] == -1) || (tp.handles[7] == -1)
									 )
									{
									for (int k = 0; k < 8; k++)
										{
										tp1.handles[k] = -1;
										tp2.handles[k] = -1;
										}

									}
								else
									{

									PatchMesh msh;
									msh.setNumPatches(1);
									msh.setNumVerts(4);
									if (p.flags&PATCH_AUTO)
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										}
									else 
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										for (k = 0; k < 4; k++)
											{
											int a = tp.interiors[k];
											msh.setVec(k+8,patch->mapVerts(chan)[a].p);
											}	
										}

									msh.setVert(0,tvi[0]);
									msh.setVert(1,tvi[1]);
									msh.setVert(2,tvi[2]);
									msh.setVert(3,tvi[3]);

									msh.MakeQuadPatch(0, 0, 0,1,
													 1, 2,3, 
													 2, 4, 5,
													 3, 6, 7, 
													 8, 9, 10,
													11, 0);


									msh.computeInteriors();
									msh.buildLinkages();

									msh.edgeSel.Set(0,TRUE);


									msh.Subdivide(SUBDIV_EDGES,FALSE);

									int vecStart = tvert[nchan];
							
									int a = msh.patches[0].vec[0];
									int b = msh.patches[0].vec[1];

									int c = msh.patches[0].vec[2];
									int d = msh.patches[0].vec[3];

									int e = msh.patches[0].vec[4];
									int f = msh.patches[0].vec[5];

									int g = msh.patches[0].vec[6];
									int h = msh.patches[0].vec[7];
//patch a
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp1.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp1.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp1.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp1.handles[3] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp1.handles[4] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp1.handles[5] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp1.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp1.handles[7] = tvert[nchan]++;
//patch b
									a = msh.patches[1].vec[0];
									b = msh.patches[1].vec[1];

									c = msh.patches[1].vec[2];
									d = msh.patches[1].vec[3];

									e = msh.patches[1].vec[4];
									f = msh.patches[1].vec[5];

									g = msh.patches[1].vec[6];
									h = msh.patches[1].vec[7];


									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp2.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp2.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp2.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp2.handles[3] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp2.handles[4] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp2.handles[5] = tvert[nchan]++;

									tp2.handles[6] = tp1.handles[3];
									tp2.handles[7] = tp1.handles[2];

									if (!(p.flags&PATCH_AUTO))
										{
										a = msh.patches[0].interior[0];
										b = msh.patches[0].interior[1];
										c = msh.patches[0].interior[2];
										d = msh.patches[0].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp1.interiors[0] = tvert[nchan]++;
		
										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp1.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp1.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp1.interiors[3] = tvert[nchan]++;


										a = msh.patches[1].interior[0];
										b = msh.patches[1].interior[1];
										c = msh.patches[1].interior[2];
										d = msh.patches[1].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp2.interiors[0] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp2.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp2.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp2.interiors[3] = tvert[nchan]++;

										}
									}
								}


							tp1.tv[0] = tp.tv[0];
							tp1.tv[1] = tva;
							tp1.tv[2] = tvb;
							tp1.tv[3] = tp.tv[3];
							tp2.tv[0] = tva;
							tp2.tv[1] = tp.tv[1];
							tp2.tv[2] = tp.tv[2];
							tp2.tv[3] = tvb;
							patch->mapVerts(chan)[tva].p = (patch->mapVerts(chan)[tp.tv[0]].p + patch->mapVerts(chan)[tp.tv[1]].p) / 2.0f;
							patch->mapVerts(chan)[tvb].p = (patch->mapVerts(chan)[tp.tv[2]].p + patch->mapVerts(chan)[tp.tv[3]].p) / 2.0f;
							}
						}

					// If it's not an auto patch, compute the new interior points
					if(!(p.flags & PATCH_AUTO)) {
						p1.flags &= ~PATCH_AUTO;
						p2.flags &= ~PATCH_AUTO;

						Point3 a,b,c,d;
						InterpCenter(patch, i, 7, 0, 1, 2, &a, &b, &c, &d);
						patch->vecs[p1.interior[0]].p = a;
						patch->vecs[p1.interior[1]].p = b;
						patch->vecs[p2.interior[0]].p = c;
						patch->vecs[p2.interior[1]].p = d;
						InterpCenter(patch, i, 6, 3, 2, 3, &a, &b, &c, &d);
						patch->vecs[p1.interior[3]].p = a;
						patch->vecs[p1.interior[2]].p = b;
						patch->vecs[p2.interior[3]].p = c;
						patch->vecs[p2.interior[2]].p = d;
						}
					}
				else {							// Divide edges 1 & 3
					// Need to create two new patches
					// Compute new edge vectors between new edge verts
					int newev1 = vec++;	// edge 1 -> edge 3
					int newev2 = vec++;	// edge 3 -> edge 1

					// Get pointers to new edges
					NewEdge &e1 = eMap[p.edge[1]];
					NewEdge &e3 = eMap[p.edge[3]];

					// See if edges need to be flopped
					BOOL flop1 = (e1.v1 == p.v[1]) ? FALSE : TRUE;
					BOOL flop3 = (e3.v1 == p.v[3]) ? FALSE : TRUE;

					// Compute the new vectors for the dividing line
					patch->vecs[newev1].p = InterpCenter(patch, i, 1, 1, 2, 4);
					patch->vecs[newev2].p = InterpCenter(patch, i, 0, 0, 3, 5);

					// Create the two new patches
					Patch &p1 = patch->patches[pat++];
					Patch &p2 = patch->patches[pat++];

					p1.SetType(PATCH_QUAD);
					p1.v[0] = p.v[1];
					p1.v[1] = e1.v2;
					p1.v[2] = e3.v2;
					p1.v[3] = p.v[0];
					p1.vec[0] = flop1 ? e1.vec32 : e1.vec12;
					p1.vec[1] = flop1 ? e1.vec23 : e1.vec21;
					p1.vec[2] = newev1;
					p1.vec[3] = newev2;
					p1.vec[4] = flop3 ? e3.vec21 : e3.vec23;
					p1.vec[5] = flop3 ? e3.vec12 : e3.vec32;
					p1.vec[6] = p.vec[0];
					p1.vec[7] = p.vec[1];
					p1.interior[0] = vec++;
					p1.interior[1] = vec++;
					p1.interior[2] = vec++;
					p1.interior[3] = vec++;

					p2.SetType(PATCH_QUAD);
					p2.v[0] = e1.v2;
					p2.v[1] = p.v[2];
					p2.v[2] = p.v[3];
					p2.v[3] = e3.v2;
					p2.vec[0] = flop1 ? e1.vec21 : e1.vec23;
					p2.vec[1] = flop1 ? e1.vec12 : e1.vec32;
					p2.vec[2] = p.vec[4];
					p2.vec[3] = p.vec[5];
					p2.vec[4] = flop3 ? e3.vec32 : e3.vec12;
					p2.vec[5] = flop3 ? e3.vec23 : e3.vec21;
					p2.vec[6] = newev2;
					p2.vec[7] = newev1;
					p2.interior[0] = vec++;
					p2.interior[1] = vec++;
					p2.interior[2] = vec++;
					p2.interior[3] = vec++;

					// If this patch is textured, create two new texture verts for it
					for(chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
						int nchan = chan + NUM_HIDDENMAPS;
						if(patch->mapPatches(chan)) {
							int tva = tvert[nchan]++;
							int tvb = tvert[nchan]++;
							TVPatch &tp = patch->mapPatches(chan)[i];
							TVPatch &tp1 = patch->mapPatches(chan)[tpat[nchan]++];
							TVPatch &tp2 = patch->mapPatches(chan)[tpat[nchan]++];

							Point3 tvi[4];
							for (int vi = 0; vi <4; vi++)
								tvi[vi] = patch->mapVerts(chan)[tp.tv[vi]].p;
//need to compute handles if neccessary
							if (patch->ArePatchesCurvedMapped(i))
								{
								if ( (tp.handles[0] == -1) || (tp.handles[1] == -1) || (tp.handles[2] == -1)||
									 (tp.handles[3] == -1) || (tp.handles[4] == -1) || (tp.handles[5] == -1) ||
									 (tp.handles[6] == -1) || (tp.handles[7] == -1)
									 )
									{
									for (int k = 0; k < 8; k++)
										{
										tp1.handles[k] = -1;
										tp2.handles[k] = -1;
										}

									}
								else
									{

									PatchMesh msh;
									msh.setNumPatches(1);
									msh.setNumVerts(4);
									if (p.flags&PATCH_AUTO)
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										}
									else 
										{
										msh.setNumVecs(12);
										for (int k = 0; k < 8; k++)
											{
											int a = tp.handles[k];
											msh.setVec(k,patch->mapVerts(chan)[a].p);
											}	
										for (k = 0; k < 4; k++)
											{
											int a = tp.interiors[k];
											msh.setVec(k+8,patch->mapVerts(chan)[a].p);
											}	
										}

									msh.setVert(0,tvi[0]);
									msh.setVert(1,tvi[1]);
									msh.setVert(2,tvi[2]);
									msh.setVert(3,tvi[3]);

									msh.MakeQuadPatch(0, 0, 0,1,
													 1, 2,3, 
													 2, 4, 5,
													 3, 6, 7, 
													 8, 9, 10,
													11, 0);


									msh.computeInteriors();
									msh.buildLinkages();

									msh.edgeSel.Set(1,TRUE);


									msh.Subdivide(SUBDIV_EDGES,FALSE);

									int vecStart = tvert[nchan];
							
									int a = msh.patches[0].vec[0];
									int b = msh.patches[0].vec[1];

									int c = msh.patches[0].vec[2];
									int d = msh.patches[0].vec[3];

									int e = msh.patches[0].vec[4];
									int f = msh.patches[0].vec[5];

									int g = msh.patches[0].vec[6];
									int h = msh.patches[0].vec[7];
//patch a
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp1.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp1.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp1.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp1.handles[3] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp1.handles[4] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp1.handles[5] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[g].p;
									tp1.handles[6] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[h].p;
									tp1.handles[7] = tvert[nchan]++;
//patch b
									a = msh.patches[1].vec[0];
									b = msh.patches[1].vec[1];

									c = msh.patches[1].vec[2];
									d = msh.patches[1].vec[3];

									e = msh.patches[1].vec[4];
									f = msh.patches[1].vec[5];

									g = msh.patches[1].vec[6];
									h = msh.patches[1].vec[7];


									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
									tp2.handles[0] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
									tp2.handles[1] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
									tp2.handles[2] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
									tp2.handles[3] = tvert[nchan]++;

									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[e].p;
									tp2.handles[4] = tvert[nchan]++;
									patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[f].p;
									tp2.handles[5] = tvert[nchan]++;

									tp2.handles[6] = tp1.handles[3];
									tp2.handles[7] = tp1.handles[2];

									if (!(p.flags&PATCH_AUTO))
										{
										a = msh.patches[0].interior[0];
										b = msh.patches[0].interior[1];
										c = msh.patches[0].interior[2];
										d = msh.patches[0].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp1.interiors[0] = tvert[nchan]++;
	
										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp1.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp1.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp1.interiors[3] = tvert[nchan]++;


										a = msh.patches[1].interior[0];
										b = msh.patches[1].interior[1];
										c = msh.patches[1].interior[2];
										d = msh.patches[1].interior[3];

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[a].p;
										tp2.interiors[0] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[b].p;
										tp2.interiors[1] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[c].p;
										tp2.interiors[2] = tvert[nchan]++;

										patch->mapVerts(chan)[tvert[nchan]].p = msh.vecs[d].p;
										tp2.interiors[3] = tvert[nchan]++;
	
										}
									}
								}


							tp1.tv[0] = tp.tv[1];
							tp1.tv[1] = tva;
							tp1.tv[2] = tvb;
							tp1.tv[3] = tp.tv[0];
							tp2.tv[0] = tva;
							tp2.tv[1] = tp.tv[2];
							tp2.tv[2] = tp.tv[3];
							tp2.tv[3] = tvb;
							patch->mapVerts(chan)[tva].p = (patch->mapVerts(chan)[tp.tv[1]].p + patch->mapVerts(chan)[tp.tv[2]].p) / 2.0f;
							patch->mapVerts(chan)[tvb].p = (patch->mapVerts(chan)[tp.tv[0]].p + patch->mapVerts(chan)[tp.tv[3]].p) / 2.0f;
							}
						}

					// If it's not an auto patch, compute the new interior points
					if(!(p.flags & PATCH_AUTO)) {
						p1.flags &= ~PATCH_AUTO;
						p2.flags &= ~PATCH_AUTO;

						Point3 a,b,c,d;
						InterpCenter(patch, i, 1, 1, 2, 4, &a, &b, &c, &d);
						patch->vecs[p1.interior[0]].p = a;
						patch->vecs[p1.interior[1]].p = b;
						patch->vecs[p2.interior[0]].p = c;
						patch->vecs[p2.interior[1]].p = d;
						InterpCenter(patch, i, 0, 0, 3, 5, &a, &b, &c, &d);
						patch->vecs[p1.interior[3]].p = a;
						patch->vecs[p1.interior[2]].p = b;
						patch->vecs[p2.interior[3]].p = c;
						patch->vecs[p2.interior[2]].p = d;
						}
					}	
				}
			pDivIx++;
			}
		}

	delete [] pInfo;
	delete [] eMap;

	// Now call the DeletePatchParts function to clean it all up
	BitArray dumVerts(patch->getNumVerts());
	dumVerts.ClearAll();
	BitArray dumPatches(patch->getNumPatches());
	dumPatches.ClearAll();
	// Mark the subdivided patches as deleted
	for(i = 0; i < patches; ++i)
		dumPatches.Set(i, pDiv[i]);

#ifdef DUMPING
DebugPrint("Before:\n");
patch->Dump();
#endif

	DeletePatchParts(patch, dumVerts, dumPatches);

#ifdef DUMPING
DebugPrint("After:\n");
patch->Dump();
#endif

	for(i = patch->numPatches - newPatches; i < patch->numPatches; ++i)
		patch->patchSel.Set(i);
	patch->computeInteriors();
	patch->buildLinkages();
	}

BOOL EdgeSubdivideRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	SubdividePatch(SUBDIV_EDGES, propagate, patch);
	return TRUE;
	}

#define ESUBR_PROPAGATE_CHUNK		0x1000
#define ESUBR_PATCH_CHUNK			0x1010

IOResult EdgeSubdivideRecord::Load(ILoad *iload) {
	IOResult res;
	propagate = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ESUBR_PROPAGATE_CHUNK:
				propagate = TRUE;
				break;
//			case ESUBR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PatchSubdivideRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	SubdividePatch(SUBDIV_PATCHES, propagate, patch);
	return TRUE;
	}

#define PSUBR_PROPAGATE_CHUNK		0x1000
#define PSUBR_PATCH_CHUNK			0x1010

IOResult PatchSubdivideRecord::Load(ILoad *iload) {
	IOResult res;
	propagate = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PSUBR_PROPAGATE_CHUNK:
				propagate = TRUE;
				break;
//			case PSUBR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

BOOL PVertWeldRecord::Redo(PatchMesh *patch,int reRecord) {
	if(reRecord)
		oldPatch = *patch;
	patch->Weld(thresh);
	return TRUE;
	}

#define WELDR_THRESH_CHUNK			0x1010
#define WELDR_PATCH_CHUNK			0x1000

IOResult PVertWeldRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	propagate = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case WELDR_THRESH_CHUNK:
				res = iload->Read(&thresh,sizeof(float),&nb);
				break;
//			case WELDR_PATCH_CHUNK:
//				res = oldPatch.Load(iload);
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

PatchRestore::PatchRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch, TCHAR *id)
	{
	gotRedo = FALSE;
	epd = pd;
	this->mod = mod;
	oldPatch = *patch;
	t = mod->ip->GetTime();
	where = TSTR(id);
	}

void PatchRestore::Restore(int isUndo)
	{
	if ( epd->tempData && epd->TempData(mod)->PatchCached(t) ) {
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t);
		if(patch) {
			if(isUndo && !gotRedo) {
				newPatch = *patch;
				gotRedo = TRUE;
				}
			}
		DWORD level = patch->selLevel;	// Grab this...
		DWORD dispFlags = patch->dispFlags;	// Grab this...
		*patch = oldPatch;
		patch->selLevel = level;	// ...and put it back in
		patch->dispFlags = dispFlags;	// ...and put it back in
		patch->InvalidateGeomCache();
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( epd->tempData ) {
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
	if(mod->ip)
		Cancel2StepPatchModes(mod->ip);
	mod->InvalidateSurfaceUI();
	mod->SelectionChanged();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

void PatchRestore::Redo()
	{
	if ( epd->tempData && epd->TempData(mod)->PatchCached(t) ) {
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t);
		if(patch) {
			DWORD level = patch->selLevel;	// Grab this...
			DWORD dispFlags = patch->dispFlags;	// Grab this...
			*patch = newPatch;
			patch->selLevel = level;	// ...and put it back in
			patch->dispFlags = dispFlags;	// ...and put it back in
			patch->InvalidateGeomCache();
			}
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( epd->tempData ) {
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT,FALSE);
		}
	if(mod->ip)
		Cancel2StepPatchModes(mod->ip);
	mod->InvalidateSurfaceUI();
	mod->SelectionChanged();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

/*-------------------------------------------------------------------*/

PatchSelRestore::PatchSelRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch)
	{
	gotRedo = FALSE;
	epd = pd;
	this->mod = mod;
	oldHSel = patch->vecSel;		// CAL-06/10/03: (FID #1914)
	oldVSel = patch->vertSel;
	oldESel = patch->edgeSel;
	oldPSel = patch->patchSel;
	t = mod->ip->GetTime();
	}

void PatchSelRestore::Restore(int isUndo)
	{
	if ( epd->tempData && epd->TempData(mod)->PatchCached(t) ) {
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t);
		if(patch) {
			if(isUndo && !gotRedo) {
				newHSel = patch->vecSel;		// CAL-06/10/03: (FID #1914)
				newVSel = patch->vertSel;
				newESel = patch->edgeSel;
				newPSel = patch->patchSel;
				gotRedo = TRUE;
				}
			}
		patch->vecSel = oldHSel;		// CAL-06/10/03: (FID #1914)
		patch->vertSel = oldVSel;
		patch->edgeSel = oldESel;
		patch->patchSel = oldPSel;
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( epd->tempData ) {
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
	if(mod->ip)
		Cancel2StepPatchModes(mod->ip);
	mod->InvalidateSurfaceUI();
//	mod->PatchSelChanged();
//	mod->UpdateSelectDisplay();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

void PatchSelRestore::Redo()
	{
	if ( epd->tempData && epd->TempData(mod)->PatchCached(t) ) {
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t);
		if(patch) {
			patch->vecSel = newHSel;		// CAL-06/10/03: (FID #1914)
			patch->vertSel = newVSel;
			patch->edgeSel = newESel;
			patch->patchSel = newPSel;
			}
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( epd->tempData ) {
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT,FALSE);
		}
	if(mod->ip)
		Cancel2StepPatchModes(mod->ip);
	mod->InvalidateSurfaceUI();
//	mod->PatchSelChanged();
//	mod->UpdateSelectDisplay();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

/*-------------------------------------------------------------------*/

BOOL PickPatchAttach::Filter(INode *node)
	{
	ModContextList mcList;		
	INodeTab nodes;
	if (node) {
		// Make sure the node does not depend on us
		node->BeginDependencyTest();
		ep->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest()) return FALSE;

		ObjectState os = node->GetObjectRef()->Eval(ep->ip->GetTime());
		GeomObject *object = (GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		ep->ip->GetModContexts(mcList,nodes);
		int numNodes = nodes.Count();
		for(int i = 0; i < numNodes; ++i) {
			if(nodes[i] == node) {
				nodes.DisposeTemporary();
				return FALSE;
				}
			}
		if(object->CanConvertToType(patchObjectClassID)) {
			nodes.DisposeTemporary();
			return TRUE;
			}
		}
	nodes.DisposeTemporary();
	return FALSE;
	}

BOOL PickPatchAttach::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	ModContextList mcList;		
	INodeTab nodes;
	
	if (node) {
		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		GeomObject *object = (GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		ep->ip->GetModContexts(mcList,nodes);
		int numNodes = nodes.Count();
		for(int i = 0; i < numNodes; ++i) {
			if(nodes[i] == node) {
				nodes.DisposeTemporary();
				return FALSE;
				}
			}
		if(object->CanConvertToType(patchObjectClassID)) {
			nodes.DisposeTemporary();
			return TRUE;
			}
		}

	nodes.DisposeTemporary();
	return FALSE;
	}

BOOL PickPatchAttach::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);
	GeomObject *object = (GeomObject *)node->GetObjectRef()->Eval(ip->GetTime()).obj;
	if(object->CanConvertToType(patchObjectClassID)) {
		PatchObject *attPatch = (PatchObject *)object->ConvertToType(ip->GetTime(),patchObjectClassID);
		if(attPatch) {
			PatchMesh patch = attPatch->patch;
			ModContextList mcList;
			INodeTab nodes;
			ip->GetModContexts(mcList,nodes);
			BOOL res = TRUE;
			if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl()))
				res = DoAttachMatOptionDialog(ep->ip, ep);
			if(res) {
				bool canUndo = TRUE;
				ep->DoAttach(node, &patch, canUndo);
				if (!canUndo)
					GetSystemSetting (SYSSET_CLEAR_UNDO);
				}
			nodes.DisposeTemporary();
			// Discard the copy it made, if it isn't the same as the object itself
			if(attPatch != (PatchObject *)object)
				delete attPatch;
			}
		}
	return FALSE;
	}


void PickPatchAttach::EnterMode(IObjParam *ip)
	{
	if ( ep->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(ep->hOpsPanel,IDC_ATTACH));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void PickPatchAttach::ExitMode(IObjParam *ip)
	{
	if ( ep->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(ep->hOpsPanel,IDC_ATTACH));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

HCURSOR PickPatchAttach::GetHitCursor(IObjParam *ip) {
	return LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ATTACHCUR));
	}

int EditPatchMod::DoAttach(INode *node, PatchMesh *attPatch, bool & canUndo) {
	ModContextList mcList;	
	INodeTab nodes;	

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);

	EditPatchData *patchData = (EditPatchData*)mcList[0]->localData;
	if ( !patchData ) {
		nodes.DisposeTemporary();
		return 0;
		}

	// If the mesh isn't yet cached, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) {
		nodes.DisposeTemporary();
		return 0;
		}
	patchData->BeginEdit(ip->GetTime());
	RecordTopologyTags();

	theHold.Begin();
	patchData->PreUpdateChanges(patch);

	// Transform the shape for attachment:
	// If reorienting, just translate to align pivots
	// Otherwise, transform to match our transform
	Matrix3 attMat(1);
	if(attachReorient) {
		Matrix3 thisTM = nodes[0]->GetNodeTM(ip->GetTime());
		Matrix3 thisOTMBWSM = nodes[0]->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 thisPivTM = thisTM * Inverse(thisOTMBWSM);
		Matrix3 otherTM = node->GetNodeTM(ip->GetTime());
		Matrix3 otherOTMBWSM = node->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 otherPivTM = otherTM * Inverse(otherOTMBWSM);
		Point3 otherObjOffset = node->GetObjOffsetPos();
		attMat = Inverse(otherPivTM) * thisPivTM;
		}
	else {
		attMat = node->GetObjectTM(ip->GetTime()) *
			Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
		}
		
	// RB 3-17-96 : Check for mirroring
	AffineParts parts;
	decomp_affine(attMat,&parts);
	if (parts.f<0.0f) {
		int v[8], ct, ct2, j;
		int tvInteriors[4], tvHandles[8];

		Point3 p[9];
//watje 10-21-99  212991 since there is topochange need to record and resolve topo 
//		even though it is a temporary object  to update any bind
		attPatch->RecordTopologyTags();

		for (int i=0; i<attPatch->numPatches; i++) {
			
			// Re-order vertices
			ct = attPatch->patches[i].type==PATCH_QUAD ? 4 : 3;
			for (j=0; j<ct; j++) {
				v[j] = attPatch->patches[i].v[j];
				}
			for (j=0; j<ct; j++) {
				attPatch->patches[i].v[j] = v[ct-j-1];
				}

			// Re-order vecs
			ct  = attPatch->patches[i].type==PATCH_QUAD ? 8 : 6;
			ct2 = attPatch->patches[i].type==PATCH_QUAD ? 5 : 3;
			for (j=0; j<ct; j++) {
				v[j] = attPatch->patches[i].vec[j];
				}
			for (j=0; j<ct; j++,ct2--) {
				if (ct2<0) ct2 = ct-1;
				attPatch->patches[i].vec[j] = v[ct2];
				}

			// Re-order enteriors
			if (attPatch->patches[i].type==PATCH_QUAD) {
				ct = 4;
				for (j=0; j<ct; j++) {
					v[j] = attPatch->patches[i].interior[j];
					}
				for (j=0; j<ct; j++) {
					attPatch->patches[i].interior[j] = v[ct-j-1];
					}
				}

			// Re-order aux
			if (attPatch->patches[i].type==PATCH_TRI) {
				ct = 9;
				for (j=0; j<ct; j++) {
					p[j] = attPatch->patches[i].aux[j];
					}
				for (j=0; j<ct; j++) {
					attPatch->patches[i].aux[j] = p[ct-j-1];
					}
				}

			// Re-order TV faces if present
			for(int chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) {
				if (chan < attPatch->getNumMaps() && attPatch->mapPatches(chan)) {
					ct = attPatch->patches[i].type==PATCH_QUAD ? 4 : 3;
					for (j=0; j<ct; j++) {
						v[j] = attPatch->mapPatches(chan)[i].tv[j];
						tvInteriors[j] = attPatch->mapPatches(chan)[i].interiors[j];
						int a;
						int b;
						a = j*2-1;
						b = j*2;
						if (a<0) a = ct*2-1;
						tvHandles[j*2] = attPatch->mapPatches(chan)[i].handles[a];
					 	tvHandles[j*2+1] = attPatch->mapPatches(chan)[i].handles[b];

						}
					for (j=0; j<ct; j++) {
						attPatch->mapPatches(chan)[i].tv[j] = v[ct-j-1];
						attPatch->mapPatches(chan)[i].interiors[j] = tvInteriors[(ct)-(j)-1];
						int index = ct-j-1;
						int a;
						int b;
						a = j*2-1;
						b = j*2;
						if (a<0) a = ct*2-1;
						attPatch->mapPatches(chan)[i].handles[b] = tvHandles[index*2];
						attPatch->mapPatches(chan)[i].handles[a] = tvHandles[index*2+1];

						}
					}

				}
			}
//watje 10-21-99  212991 since there is topochange need to record and resolve topo 
//		even though it is a temporary object  to update any bind
		attPatch->buildLinkages();
		attPatch->HookFixTopology();
		}

	for(int i = 0; i < attPatch->numVerts; ++i)
		attPatch->verts[i].p = attPatch->verts[i].p * attMat;
	for(i = 0; i < attPatch->numVecs; ++i)
		attPatch->vecs[i].p = attPatch->vecs[i].p * attMat;
	attPatch->computeInteriors();

	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = nodes[0]->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 && (m1 != m2)) {
		if (attachMat==ATTACHMAT_IDTOMAT) {
			int ct=1;
			if (m1->IsMultiMtl())
				ct = m1->NumSubMtls();
			for(int i = 0; i < patch->numPatches; ++i) {
				int mtid = patch->getPatchMtlIndex(i);
				if(mtid >= ct)
					patch->setPatchMtlIndex(i, mtid % ct);
				}
			FitPatchIDsToMaterial (*attPatch, m2);
			if (condenseMat) condenseMe = TRUE;
			}
		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		theHold.Suspend ();
		if (attachMat==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToPatchIDs (*patch, m1);
			m2 = FitMaterialToPatchIDs (*attPatch, m2);
			}

		Mtl *multi = CombineMaterials (m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		theHold.Resume ();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		patch->selLevel = PATCH_OBJECT;
		nodes[0]->SetMtl(multi);
		patch->selLevel = oldSL;
		m1 = multi;
		// canUndo = FALSE;	// DS: 11/15/00 - Undo should work now.
		}
	if (!m1 && m2) {
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		patch->selLevel = PATCH_OBJECT;
		nodes[0]->SetMtl(m2);
		patch->selLevel = oldSL;
		m1 = m2;
		}

	// Start a restore object...
	if ( theHold.Holding() )
		theHold.Put(new PatchRestore(patchData,this,patch,"DoAttach"));

	// Do the attach
	patch->Attach(attPatch, mat2Offset);
	patchData->UpdateChanges(patch);
	patchData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);

	// Get rid of the original node
	ip->DeleteNode(node);

	ResolveTopoChanges();
	theHold.Accept(GetString(IDS_TH_ATTACH));

	if (m1 && condenseMe) {
		// Following clears undo stack.
		patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		m1 = CondenseMatAssignments (*patch, m1);
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO|PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	return 1;
	}

/*-------------------------------------------------------------------*/

EditPatchMod::EditPatchMod()
	{
	selLevel = EP_OBJECT;
	displayLattice = TRUE;
	displaySurface = TRUE;
	propagate = FALSE;
	meshSteps = 5;
	//3-18-99 to suport render steps and removal of the mental tesselator
	meshStepsRender = 5;
	showInterior = FALSE;	// CAL-05/02/03: change the default to FALSE.
	usePatchNormals = FALSE;	// CAL-05/15/03: use true patch normals. (FID #1760)

	// CAL-05/01/03: support spline surface generation (FID #1914)
	generateSurface = TRUE;
	genSurfWeldThreshold = EP_DEF_GENSURF_THRESH;
	genSurfFlipNormals = FALSE;
	genSurfRmInterPatches = TRUE;
	genSurfUseOnlySelSegs = FALSE;

	// CAL-06/02/03: copy/paste tangent. (FID #827)
	copyTanLength = FALSE;
	tangentCopied = FALSE;

	mFalloff =  20.0;
	mBubble = 0.0;
	mPinch = 0.0;
	mUseSoftSelections = 0;
	mUseEdgeDists = 0;
	mEdgeDist = 1;
	mAffectBackface = 1;

	namedSelNeedsFixup = FALSE;
//	meshAdaptive = FALSE;	// Future use (Not used now)
	epFlags = 0;
	objectChanged = TRUE;
	// Relax inits -- TH 7/26/00
	relax = DEF_EP_RELAX;
	relaxViewports = DEF_EP_RELAX_VIEWPORTS;
	relaxValue = DEF_EP_RELAX_VALUE;
	relaxIter = DEF_EP_RELAX_ITER;
	relaxBoundary = DEF_EP_RELAX_BOUNDARY;
	relaxSaddle = DEF_EP_RELAX_SADDLE;
	}

EditPatchMod::~EditPatchMod()
	{
	ClearSetNames();
	}

Interval EditPatchMod::LocalValidity(TimeValue t)
	{
	// Force a cache if being edited.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  			   
	return FOREVER;
	}

RefTargetHandle EditPatchMod::Clone(RemapDir& remap) {
	EditPatchMod* newmod = new EditPatchMod();	
	newmod->selLevel = selLevel;
	newmod->displaySurface = displaySurface;
	newmod->displayLattice = displayLattice;
	newmod->meshSteps = meshSteps;
#ifndef NO_OUTPUTRENDERER
	//3-18-99 to suport render steps and removal of the mental tesselator
	newmod->meshStepsRender = meshStepsRender;
#endif // NO_OUTPUTRENDERER
	newmod->showInterior = showInterior;
	newmod->usePatchNormals = usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

	// CAL-05/01/03: support spline surface generation (FID #1914)
	newmod->generateSurface		  = generateSurface;
	newmod->genSurfWeldThreshold  = genSurfWeldThreshold;
	newmod->genSurfFlipNormals    = genSurfFlipNormals;
	newmod->genSurfRmInterPatches = genSurfRmInterPatches;
	newmod->genSurfUseOnlySelSegs = genSurfUseOnlySelSegs;

//	newmod->meshAdaptive = meshAdaptive;	// Future use (Not used now)
	newmod->viewTess = viewTess;
	newmod->prodTess = prodTess;
	newmod->dispTess = dispTess;
	newmod->mViewTessNormals = mViewTessNormals;
	newmod->mProdTessNormals = mProdTessNormals;
	newmod->mViewTessWeld = mViewTessWeld;
	newmod->mProdTessWeld = mProdTessWeld;
	newmod->propagate = propagate;
	newmod->objectChanged = objectChanged;
	newmod->relax = relax;
	newmod->relaxViewports = relaxViewports;
	newmod->relaxValue = relaxValue;
	newmod->relaxIter = relaxIter;
	newmod->relaxBoundary = relaxBoundary;
	newmod->relaxSaddle = relaxSaddle;
	for (int i=0; i<EP_NS_LEVELS; i++)
	{
		newmod->namedSel[i].SetCount (namedSel[i].Count());
		for (int j=0; j<namedSel[i].Count(); j++) newmod->namedSel[i][j] = new TSTR (*namedSel[i][j]);
	}
	BaseClone(this, newmod, remap);
	return(newmod);
	}

void EditPatchMod::ClearPatchDataFlag(ModContextList& mcList,DWORD f)
	{
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		patchData->SetFlag(f,FALSE);
		}
	}

void EditPatchMod::XFormHandles(XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis,	int object, int handleIndex)
	{	
	ModContextList mcList;		
	INodeTab nodes;
	Matrix3 mat,imat,theMatrix;
	Interval valid;
	int numAxis;
	Point3 oldpt,newpt,oldin,oldout,rel;
	BOOL shiftPressed = FALSE;
	static BOOL wasBroken;
	Point3 theKnot;
	Point3 oldVector;
	Point3 newVector;
	float oldLen;
	float newLen;
//DebugPrint("XFormHandles\n");
	shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	numAxis = ip->GetNumAxis();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	bool bUseSel = (object < 0) && (handleIndex < 0);		// use selection in vecSel if handleIndex < 0
	for ( int obj = (bUseSel) ? 0 : object; obj < mcList.Count(); obj = (bUseSel) ? obj+1 : mcList.Count() ) {
		EditPatchData *patchData = (EditPatchData*)mcList[obj]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
			
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// Create a change record for this object and store a pointer to its delta info in this EditPatchData
		if(!TestAFlag(A_HELD)) {
			patchData->vdelta.SetSize(*patch,FALSE);
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"XFormHandles"));
			patchData->vdelta.Zero();		// Reset all deltas
			patchData->ClearHandleFlag();
			wasBroken = FALSE;
			}
		else {
			if(wasBroken && !shiftPressed)
				wasBroken = FALSE;
			if(patchData->DoingHandles())
				patchData->ApplyHandlesAndZero(*patch);		// Reapply the slave handle deltas
			else
				patchData->vdelta.Zero();
			}

		patchData->SetHandleFlag(handleIndex);

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		int vecCount = patch->getNumVecs();
		for (int vec = (bUseSel) ? 0 : handleIndex; vec < vecCount; vec = (bUseSel) ? vec+1 : vecCount) {
			if (bUseSel && !patch->vecSel[vec]) continue;

			int primaryKnot = patch->vecs[vec].vert;
			Point3Tab &pDeltas = patchData->vdelta.dtab.vtab;

			tmAxis = ip->GetTransformAxis(nodes[obj],primaryKnot);
			mat    = nodes[obj]->GetObjectTM(t,&valid) * Inverse(tmAxis);
			imat   = Inverse(mat);
			xproc->SetMat(mat);
						
			// XForm the cache vertices
			oldpt = patch->vecs[vec].p;
			newpt = xproc->proc(oldpt,mat,imat);

			// Update the vector being moved
			patch->vecs[vec].p = newpt;

			// Move the delta's vertices.
			patchData->vdelta.SetVec(vec,newpt - oldpt);

			if(primaryKnot >= 0) {
				PatchVert &vert = patch->verts[primaryKnot];
				theKnot = vert.p;
				// vectors could be transformed multiple times if we don't break it. break it when necessary.
				bool bMultiXForms = false;
				if (bUseSel && (lockedHandles || (!shiftPressed && (vert.flags&PVERT_COPLANAR && vert.vectors.Count() > 2)))) {
					for (int vv = 0; vv < vert.vectors.Count(); vv++)
						if (vert.vectors[vv] != vec && patch->vecSel[vert.vectors[vv]]) break;
					if (vv < vert.vectors.Count()) bMultiXForms = true;
					}
				// If locked handles, turn the movement into a transformation matrix
				// and transform all the handles attached to the owner vertex
				if(lockedHandles && !bMultiXForms) {
					if(!wasBroken && shiftPressed)
						wasBroken = TRUE;
					goto locked_handles;
					}
				else {
					if(shiftPressed || bMultiXForms) {
						wasBroken = TRUE;
						vert.flags &= ~PVERT_COPLANAR;
						// Need to record this for undo!
						patchData->vdelta.SetVertType(primaryKnot,PVERT_COPLANAR);
						}
					// If a coplanar knot, do the other vectors!
					// If at the same point as knot, do nothing!
					if((vert.flags & PVERT_COPLANAR) && (vert.vectors.Count() > 2) && !(newpt == theKnot)) {
locked_handles:
						oldVector = oldpt - theKnot;
						newVector = newpt - theKnot;
						oldLen = Length(oldVector);
						newLen = Length(newVector);
						Point3 oldNorm = Normalize(oldVector);
						Point3 newNorm = Normalize(newVector);
						theMatrix.IdentityMatrix();
						Point3 axis;
						float angle = 0.0f;
						int owner = patch->vecs[vec].vert;
						if(owner >= 0) {
							PatchVert &vert = patch->verts[owner];
							int vectors = vert.vectors.Count();
							// Watch out for cases where the vectors are exactly opposite -- This
							// results in an invalid axis for transformation!
							// In this case, we look for a vector to one of the other handles that
							// will give us a useful vector for the rotational axis
							if(newNorm == -oldNorm) {
								for(int v = 0; v < vectors; ++v) {
									int theVec = vert.vectors[v];
									// Ignore the vector being moved!
									if(theVec != vec) {
										Point3 testVec = patch->vecs[theVec].p - pDeltas[theVec] - theKnot;
										if(testVec != zeroPoint) {
											Point3 testNorm = Normalize(testVec);
											if(!(testNorm == newNorm) && !(testNorm == oldNorm)) {
												// Cross product gives us the normal of the rotational axis
												axis = Normalize(testNorm ^ newNorm);
												// The angle is 180 degrees
												angle = PI;
												goto build_matrix;
												}
											}
										}
									}
								}
							else {
								// Get a matrix that will transform the old point to the new one
								// Cross product gives us the normal of the rotational axis
								axis = Normalize(oldNorm ^ newNorm);
								// Dot product gives us the angle
								float dot = DotProd(oldNorm, newNorm);
								if(dot >= -1.0f && dot <= 1.0f)
									angle = (float)-acos(dot);
								}
build_matrix:
							if(angle != 0.0f) {
								// Now let's build a matrix that'll do this for us!
								Quat quat = QFromAngAxis(angle, axis);
								quat.MakeMatrix(theMatrix);
								if(lockedHandles) {
									// If need to break the vector, 
									if(shiftPressed && vert.flags & PVERT_COPLANAR) {
										vert.flags &= ~PVERT_COPLANAR;
										patchData->vdelta.SetVertType(primaryKnot,PVERT_COPLANAR);
										}
									}
								}
							// Process all other handles through the matrix
							for(int v = 0; v < vectors; ++v) {
								int theVec = vert.vectors[v];
								// Ignore the vector being moved!
								if(theVec != vec) {
									Point3 oldpt2 = patch->vecs[theVec].p - pDeltas[theVec];
									Point3 newpt2 = (oldpt2 - theKnot) * theMatrix + theKnot;
									patch->vecs[theVec].p = newpt2;
									// Move the delta's vertices.
									patchData->vdelta.SetVec(theVec,newpt2 - oldpt2);
									}
								}
							}
						}
					}
				}
			}

		// Really only need to do this if neighbor knots are non-bezier
		patch->computeInteriors();

		patchData->UpdateChanges(patch, FALSE, TestAFlag(A_HELD));					
		patchData->TempData(this)->Invalidate(PART_GEOM);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

void EditPatchMod::XFormVerts( 
		XFormProc *xproc, 
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis  ) 
	{	
	ModContextList mcList;		
	INodeTab nodes;
	Matrix3 mat,imat;	
	Interval valid;
	int numAxis;
	Point3 oldpt,newpt,rel,delta;
	int shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
	static BOOL wasBroken;
	static BOOL handleEdit = FALSE;
	static int handleObject;
	static int handleIndex;

	if ( !ip ) return;

	BOOL isLocal = (ip->GetRefCoordSys()==COORDS_LOCAL);

	ip->GetModContexts(mcList,nodes);
	numAxis = ip->GetNumAxis();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	if(!TestAFlag(A_HELD)) {
		handleEdit = FALSE;
//DebugPrint("Handle edit cleared\n");
		// Check all patches to see if they are altering a bezier vector handle...
		if(GetSubobjectType() == EP_VERTEX) {
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;
				if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
		
				// If the mesh isn't yet cache, this will cause it to get cached.
				PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
				if(!patch) continue;

				if(!ip->SelectionFrozen() && patch->bezVecVert >= 0) {
					// Editing a bezier handle -- Go do it!
					handleEdit = TRUE;
					handleObject = i;
					handleIndex = patch->bezVecVert;
					break;
					}
	 			patchData->SetFlag(EPD_BEENDONE,TRUE);
				}
			}
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		else if(GetSubobjectType() == EP_HANDLE) {
			handleEdit = TRUE;
			handleObject = -1;
			handleIndex = -1;
			}
		}
	
	// If editing the handles, cut to the chase!
	if(handleEdit) {
		XFormHandles(xproc, t, partm, tmAxis, handleObject, handleIndex);
		nodes.DisposeTemporary();
		return;
		}

	// Not doing handles, just plain ol' verts
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	// Clear these out again
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// Create a change record for this object and store a pointer to its delta info in this EditPatchData
		if(!TestAFlag(A_HELD)) {
			patchData->vdelta.SetSize(*patch,FALSE);
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() ) {
				theHold.Put(new PatchRestore(patchData,this,patch,"XFormVerts"));
				}
			patchData->vdelta.Zero();		// Reset all deltas
			patchData->ClearHandleFlag();
			wasBroken = FALSE;
			}
		else {
			if(wasBroken)
				shiftPressed = TRUE;
			if(patchData->DoingHandles())
				patchData->ApplyHandlesAndZero(*patch);		// Reapply the slave handle deltas
			else
				patchData->vdelta.Zero();
			}

		// Compute the transforms
		if (numAxis==NUMAXIS_INDIVIDUAL) {
			switch(GetSubobjectType()) {
				case EP_EDGE:
				case EP_PATCH:
				case EP_VERTEX: {
					// Selected vertices - either directly or indirectly through selected faces or edges.
					BitArray sel = patch->VertexTempSel();
					int vertexCount = patch->numVerts;

					for( int vert = 0; vert < vertexCount; vert++ ) {
						float vertexWeight;
						if ( sel[vert] ) 
							vertexWeight = 1.0;
						else 
							vertexWeight = patch->VertexWeight( vert );

						if ( fabs(vertexWeight) > 0.0001 ) {
							if ( !shiftPressed || (shiftPressed && sel[vert]) ) {
								if (isLocal)
									tmAxis = ip->GetTransformAxis(nodes[i],vert);
								mat    = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);
					
								// XForm the cache vertices
								oldpt = patch->verts[vert].p;
								newpt = xproc->proc(oldpt,mat,imat,vertexWeight);
								patch->verts[vert].p = newpt;
								delta = newpt - oldpt;

								// Move the delta's vertices.
								patchData->vdelta.MoveVert(vert,delta);
							}

						}
					}

					// Also affect the vectors
					//
					int vectorCount = patch->getNumVecs();
					for(int vec = 0; vec < vectorCount; ++vec) {
						float vectorWeight = 0.0;
						PatchVec vecObj = patch->vecs[vec];

						bool hardSelected = false;
						if ( (vecObj.vert >= 0) && sel[vecObj.vert] ) { // the vertex associated with this vector is "hard" selected
							hardSelected = true;
						}

						if ( hardSelected ) { // the vertex associated with this vector is "hard" selected
							vectorWeight = 1.0;
						}
						else {
							vectorWeight = patch->VertexWeight(	vertexCount + vec ); // use weight of the vector tip
						}
						
						if ( fabs(vectorWeight) > 0.0001 ) {
							if ( !shiftPressed || (shiftPressed && hardSelected) ) {
								//patch->verts[vert].flags &= ~PVERT_COPLANAR;
								if (isLocal)
									tmAxis = ip->GetTransformAxis(nodes[0],vec);
								mat    = nodes[0]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);
									
								oldpt = vecObj.p;
								newpt = xproc->proc(oldpt, mat, imat, vectorWeight);

								patch->vecs[vec].p = newpt;
								delta = newpt - oldpt;

								// Move the delta's vertices.
								patchData->vdelta.MoveVec(vec,delta);
							}
						}
					}

					patch->ApplyConstraints();
					patch->computeInteriors();	// Kind of broad-spectrum -- only need to recompute affected patches
					}
					break;
#if 0
				case EP_EDGE:
				case EP_PATCH: {
					// Selected vertices - either directly or indirectly through selected faces or edges.
					BitArray sel = patch->VertexTempSel();
					int verts = patch->numVerts;
					for( int vert = 0; vert < verts; vert++ ) {
						if ( sel[vert] ) {
							if (isLocal)
								tmAxis = ip->GetTransformAxis(nodes[i],vert);
							mat    = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
							imat   = Inverse(mat);
							xproc->SetMat(mat);
			
							// XForm the cache vertices
							oldpt = patch->verts[vert].p;
							newpt = xproc->proc(oldpt,mat,imat);
							patch->verts[vert].p = newpt;
							delta = newpt - oldpt;

							// Move the delta's vertices.
							patchData->vdelta.MoveVert(vert,delta);

							// Also affect its vectors
							int vecs = patch->verts[vert].vectors.Count();
							for(int vec = 0; vec < vecs; ++vec) {
								int index = patch->verts[vert].vectors[vec];
								// XForm the cache vertices
								oldpt = patch->vecs[index].p;
								newpt = xproc->proc(oldpt,mat,imat);
								patch->vecs[index].p = newpt;
								delta = newpt - oldpt;

								// Move the delta's vertices.
								patchData->vdelta.MoveVec(index,delta);
							}
						}
					patch->computeInteriors();
					}
					break;
#endif
				}			
			}
		else {
			mat = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
			imat = Inverse(mat);
			xproc->SetMat(mat);

			// Selected vertices - either directly or indirectly through selected faces or edges.
			BitArray sel = patch->VertexTempSel();
					int vertexCount = patch->numVerts;

					for( int vert = 0; vert < vertexCount; vert++ ) {
						float vertexWeight;
						if ( sel[vert] ) 
							vertexWeight = 1.0;
						else 
							vertexWeight = patch->VertexWeight( vert );

						if ( fabs(vertexWeight) > 0.0001 ) {
							if ( !shiftPressed || (shiftPressed && sel[vert]) ) {
								if (isLocal)
									tmAxis = ip->GetTransformAxis(nodes[i],vert);
								mat    = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);
					
								// XForm the cache vertices
								oldpt = patch->verts[vert].p;
								newpt = xproc->proc(oldpt,mat,imat,vertexWeight);
								patch->verts[vert].p = newpt;
								delta = newpt - oldpt;

								// Move the delta's vertices.
								patchData->vdelta.MoveVert(vert,delta);
							}

						}
					}

					// Also affect the vectors
					//
					int vectorCount = patch->getNumVecs();
					for(int vec = 0; vec < vectorCount; ++vec) {
						float vectorWeight = 0.0;
						PatchVec vecObj = patch->vecs[vec];

						bool hardSelected = false;
						if ( (vecObj.vert >= 0) && sel[vecObj.vert] ) { // the vertex associated with this vector is "hard" selected
							hardSelected = true;
						}

						if ( hardSelected ) { // the vertex associated with this vector is "hard" selected
							vectorWeight = 1.0;
						}
						else {
							vectorWeight = patch->VertexWeight(	vertexCount + vec ); // use weight of the vector tip
						}
						
						if ( fabs(vectorWeight) > 0.0001 ) {
							if ( !shiftPressed || (shiftPressed && hardSelected) ) {
								//patch->verts[vert].flags &= ~PVERT_COPLANAR;
								if (isLocal)
									tmAxis = ip->GetTransformAxis(nodes[0],vec);
								mat    = nodes[0]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);
									
								oldpt = vecObj.p;
								newpt = xproc->proc(oldpt, mat, imat, vectorWeight);

								patch->vecs[vec].p = newpt;
								delta = newpt - oldpt;

								// Move the delta's vertices.
								patchData->vdelta.MoveVec(vec,delta);
							}
						}
					}

					patch->ApplyConstraints();
					patch->computeInteriors();	// Kind of broad-spectrum -- only need to recompute affected patches
			}
		// If there are any automatic edges, process them!
		if(patchData->autoEdges.Count()) {
			for(int i = 0; i < patchData->autoEdges.Count(); ++i) {
				PatchEdge &e = patch->edges[patchData->autoEdges[i]];
				Point3 v1p = patch->verts[e.v1].p;
				Point3 v2p = patch->verts[e.v2].p;
				// Move the edge's vectors automatically
				int index = e.vec12;
				oldpt = patch->vecs[index].p;
				newpt = v1p + ((v2p - v1p) / 3.0f);
				patch->vecs[index].p = newpt;
				delta = newpt - oldpt;
				// Move the delta's vertices.
				patchData->vdelta.MoveVec(index,delta);
				index = e.vec21;
				oldpt = patch->vecs[index].p;
				newpt = v2p + ((v1p - v2p) / 3.0f);
				patch->vecs[index].p = newpt;
				delta = newpt - oldpt;
				// Move the delta's vertices.
				patchData->vdelta.MoveVec(index,delta);
				}
			}
		patchData->UpdateChanges(patch, FALSE, TestAFlag(A_HELD));					
		patchData->TempData(this)->Invalidate(PART_GEOM);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}



void EditPatchMod::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin )
	{
	MoveXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

void EditPatchMod::Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin )
	{
	RotateXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

void EditPatchMod::Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin )
	{
	ScaleXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

void EditPatchMod::TransformStart(TimeValue t)
	{
	if(!ip) return;
	if (ip) ip->LockAxisTripods(TRUE);
	// Flush auto edge flags
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
// 212967 watje this is not needed the MaybeClone Parts add a record topology if the shift key is pressed
//	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		patchData->autoEdges.Delete(0, patchData->autoEdges.Count());
		}
	nodes.DisposeTemporary();
	MaybeClonePatchParts();
	}

void EditPatchMod::TransformFinish(TimeValue t)
	{
	if (ip) ip->LockAxisTripods(FALSE);
	UpdateSelectDisplay();

	if ( !ip ) return;	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());

		if(!patch)
			continue;
		// Flush automatic edge list
		patchData->autoEdges.Delete(0, patchData->autoEdges.Count());
//		patchData->UpdateChanges(patch, FALSE);
		}

	if ( UseSoftSelections() ) {
		UpdateVertexDists();
		UpdateEdgeDists();
		UpdateVertexWeights();
	}
	nodes.DisposeTemporary();
	}

void EditPatchMod::TransformCancel(TimeValue t)
	{
	if (ip) ip->LockAxisTripods(FALSE);
	}

void EditPatchMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{
	PatchObject *patchOb = NULL;
	BOOL splineInput = os->obj->IsSubClassOf (splineShapeClassID);
	BOOL useSplineSurface = GetGenerateSurface();

	// CAL-05/01/03: convert spline shape to patch surface (FID #1914)
	if (splineInput && useSplineSurface) {
		// convert input spline shape into patch object using Surface modifier
		SplineShape *splShape = (SplineShape*) os->obj;
		DbgAssert(splShape);
		patchOb = new PatchObject;
		DbgAssert(patchOb);
		BuildPatchFromShape(t, splShape, patchOb->patch);
	} else {
		// convert to patch object
		// DbgAssert (os->obj->CanConvertToType (patchObjectClassID));
		if (!os->obj->CanConvertToType (patchObjectClassID)) return;
		patchOb = (PatchObject*) os->obj->ConvertToType(t, patchObjectClassID);
		DbgAssert(patchOb);
	}

	if (patchOb != os->obj) {
		// Make sure "patchOb" has the right validity intervals:
		patchOb->SetChannelValidity (TOPO_CHAN_NUM, os->obj->ChannelValidity (t, TOPO_CHAN_NUM));
		patchOb->SetChannelValidity (GEOM_CHAN_NUM, os->obj->ChannelValidity (t, GEOM_CHAN_NUM));
		patchOb->SetChannelValidity (TEXMAP_CHAN_NUM, os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM));
		patchOb->SetChannelValidity (SELECT_CHAN_NUM, os->obj->ChannelValidity (t, SELECT_CHAN_NUM));
		patchOb->SetChannelValidity (SUBSEL_TYPE_CHAN_NUM, os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM));
		patchOb->SetChannelValidity (DISP_ATTRIB_CHAN_NUM, os->obj->ChannelValidity (t, DISP_ATTRIB_CHAN_NUM));
		patchOb->SetChannelValidity (VERT_COLOR_CHAN_NUM, os->obj->ChannelValidity (t, VERT_COLOR_CHAN_NUM));

		// Our output will definitely be a PatchObject, so replace the cache with "patchOb".
		os->obj = patchOb;
	}

//Alert(_T("in ModifyObject"));
	assert( os->obj->ClassID() == Class_ID(PATCHOBJ_CLASS_ID,0) );
//Alert(_T("ModifyObject class ID is OK"));
	
	// PatchObject *patchOb = (PatchObject *)os->obj;
	EditPatchData *patchData;

	if ( !mc.localData ) {
		mc.localData = new EditPatchData(this);
		patchData = (EditPatchData*)mc.localData;
		meshSteps = patchData->meshSteps = patchOb->GetMeshSteps();
#ifndef NO_OUTPUTRENDERER
		//3-18-99 to suport render steps and removal of the mental tesselator
		meshStepsRender = patchData->meshStepsRender = patchOb->GetMeshStepsRender();
#endif // NO_OUTPUTRENDERER
		showInterior = patchData->showInterior = patchOb->GetShowInterior();
		// CAL-05/15/03: use true patch normals. (FID #1760)
		usePatchNormals = patchData->usePatchNormals = patchOb->GetUsePatchNormals();
		relax = patchOb->GetRelax();
		relaxViewports = patchOb->GetRelaxViewports();
		relaxValue = patchOb->GetRelaxValue();
		relaxIter = patchOb->GetRelaxIter();
		relaxBoundary = patchOb->GetRelaxBoundary();
		relaxSaddle = patchOb->GetRelaxSaddle();
//		meshAdaptive = patchData->meshAdaptive = patchOb->GetAdaptive();	// Future use (Not used now)
		viewTess = patchData->viewTess = patchOb->GetViewTess();
		prodTess = patchData->prodTess = patchOb->GetProdTess();
		dispTess = patchData->dispTess = patchOb->GetDispTess();
		mViewTessNormals = patchData->mViewTessNormals = patchOb->GetViewTessNormals();
		mProdTessNormals = patchData->mProdTessNormals = patchOb->GetProdTessNormals();
		mViewTessWeld = patchData->mViewTessWeld = patchOb->GetViewTessWeld();
		mProdTessWeld = patchData->mProdTessWeld = patchOb->GetProdTessWeld();
		displayLattice = patchData->displayLattice = patchOb->ShowLattice();
		displaySurface = patchData->displaySurface = patchOb->showMesh;
		// If the rollups are there, refresh them so they display the updated info
		if (hOpsPanel)
			SendMessage(hOpsPanel, REFRESH_EP_VALUES, 0, 0);
		if (hSurfPanel)
			SendMessage(hSurfPanel, REFRESH_EP_VALUES, 0, 0);
	} else {
		patchData = (EditPatchData*)mc.localData;
		}

	PatchMesh &pmesh = patchOb->patch;	
	pmesh.InvalidateVertexWeights();
	assert(pmesh.numVecs == pmesh.vecSel.GetSize());	// CAL-06/10/03: (FID #1914)
	assert(pmesh.numVerts == pmesh.vertSel.GetSize());
	assert(pmesh.getNumEdges() == pmesh.edgeSel.GetSize());
	assert(pmesh.numPatches == pmesh.patchSel.GetSize());

#if 0
	UpdateSoftSelections( pmesh );

	if ( UseSoftSelections() ) {
		UpdateVertexDists();
		UpdateEdgeDists();
		UpdateVertexWeights();
	}
#endif

	// CAL-05/01/03: convert spline shape to patch surface (FID #1914)
	if (patchData->splineInput != splineInput) {
		patchData->splineInput = splineInput;
		if (hOpsPanel) SendMessage(hOpsPanel, REFRESH_EP_GENSURF, 0, 0);
	}

	patchData->Apply(this,t,patchOb,GetSubobjectType());

	// CAL-10/01/03: when using true patch normals, add an XTCOBject to wipe out specified normals when necessary. (Defect #525089)
	if (GetUsePatchNormals()) {
		// Finally, add this XTC object so the normals will be cleared if a modifier
		// changes the geometry or topology without updating the normals.
		// (Note: This object may already exist, in which case we don't need to add it.)
		for (int i=0; i<patchOb->NumXTCObjects(); i++)
		{
			if (patchOb->GetXTCObject(i)->ExtensionID() == kTriObjNormalXTCID) break;
		}
		if (i>=patchOb->NumXTCObjects())
			patchOb->AddXTCObject (new TriObjectNormalXTC());
		}
	}

void EditPatchMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
	{
	if ( mc->localData ) {
		EditPatchData *patchData = (EditPatchData*)mc->localData;
		if ( patchData ) {
			// The FALSE parameter indicates the the mesh cache itself is
			// invalid in addition to any other caches that depend on the
			// mesh cache.
			patchData->Invalidate(partID,FALSE);
			}
		}
	}

// Select a subcomponent within our object(s).  WARNING! Because the HitRecord list can
// indicate any of the objects contained within the group of patches being edited, we need
// to watch for control breaks in the patchData pointer within the HitRecord!

void EditPatchMod::SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert )
	{
	// Don't do anything if at vertex level with verts turned off
	if(GetSubobjectType() == EP_VERTEX && !filterVerts)
		return;

	if ( !ip ) return; 
	TimeValue t = ip->GetTime();

	ip->ClearCurNamedSelSet();

	// Keep processing hit records as long as we have them!
	while(hitRec) {	
		EditPatchData *patchData = (EditPatchData*)hitRec->modContext->localData;
	
		if ( !patchData )
			return;

		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			return;

		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() ) 
			theHold.Put(new PatchRestore(patchData,this,patch,"SelectSubComponent"));

		switch ( GetSubobjectLevel() ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vec_done;
							int index = ((PatchHitData *)(hitRec->hitData))->index;
							int type = ((PatchHitData *)(hitRec->hitData))->type;
							if(type == PATCH_HIT_VECTOR || type == PATCH_HIT_INTERIOR) {
								if(patch->vecSel[index])
									patch->vecSel.Clear(index);
								else
									patch->vecSel.Set(index);
								}
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vec_done;
							PatchHitData *hit = (PatchHitData *)(hitRec->hitData);
							if(hit->type == PATCH_HIT_VECTOR || hit->type == PATCH_HIT_INTERIOR)
								patch->vecSel.Set(hit->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vec_done;
							PatchHitData *hit = (PatchHitData *)(hitRec->hitData);
							if(hit->type == PATCH_HIT_VECTOR || hit->type == PATCH_HIT_INTERIOR)
								patch->vecSel.Clear(hit->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int index = ((PatchHitData *)(hitRec->hitData))->index;
					int type = ((PatchHitData *)(hitRec->hitData))->type;
					if(type == PATCH_HIT_VERTEX && byVertex) {
						PatchVert &v = patch->verts[index];
						for(int i = 0; i < v.vectors.Count(); ++i) {
							index = v.vectors[i];
							if( invert ) {
								if(patch->vecSel[index])
									patch->vecSel.Clear(index);
								else
									patch->vecSel.Set(index);
								}
							else
							if ( selected ) {
								patch->vecSel.Set(index);
								}
							else {
								patch->vecSel.Clear(index);
								}
							}
						hitRec = NULL;	// Reset it so we can exit
						}
					else
					if(type == PATCH_HIT_VECTOR || type == PATCH_HIT_INTERIOR) {
						if( invert ) {
							if(patch->vecSel[index])
								patch->vecSel.Clear(index);
							else
								patch->vecSel.Set(index);
							}
						else
						if ( selected )
							patch->vecSel.Set(index);
						else
							patch->vecSel.Clear(index);
						hitRec = NULL;	// Reset it so we can exit
						}
					else
						hitRec = hitRec->Next();
					}
				vec_done:
				break;
				}
			case EP_VERTEX: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vert_done;
							int index = ((PatchHitData *)(hitRec->hitData))->index;
							if(((PatchHitData *)(hitRec->hitData))->type == PATCH_HIT_VERTEX) {
								if(patch->vertSel[index])
									patch->vertSel.Clear(index);
								else
									patch->vertSel.Set(index);
								}
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vert_done;
							PatchHitData *hit = (PatchHitData *)(hitRec->hitData);
							if(hit->type == PATCH_HIT_VERTEX)
								patch->vertSel.Set(hit->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto vert_done;
							PatchHitData *hit = (PatchHitData *)(hitRec->hitData);
							if(hit->type == PATCH_HIT_VERTEX)
								patch->vertSel.Clear(hit->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int index = ((PatchHitData *)(hitRec->hitData))->index;
					if(((PatchHitData *)(hitRec->hitData))->type == PATCH_HIT_VERTEX) {
						if( invert ) {
							if(patch->vertSel[index])
								patch->vertSel.Clear(index);
							else
								patch->vertSel.Set(index);
							}
						else
						if ( selected )
							patch->vertSel.Set(index);
						else
							patch->vertSel.Clear(index);
						}
					hitRec = NULL;	// Reset it so we can exit	
					}
				vert_done:
				break;
				}
			case EP_EDGE: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto edge_done;
							int index = ((PatchHitData *)(hitRec->hitData))->index;
							if(patch->edgeSel[index])
								patch->edgeSel.Clear(index);
							else
								patch->edgeSel.Set(index);
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto edge_done;
							patch->edgeSel.Set(((PatchHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto edge_done;
							patch->edgeSel.Clear(((PatchHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int index = ((PatchHitData *)(hitRec->hitData))->index;
					int type = ((PatchHitData *)(hitRec->hitData))->type;
					if(type == PATCH_HIT_VERTEX && byVertex) {
						PatchVert &v = patch->verts[index];
						for(int i = 0; i < v.edges.Count(); ++i) {
							index = v.edges[i];
							if( invert ) {
								if(patch->edgeSel[index])
									patch->edgeSel.Clear(index);
								else
									patch->edgeSel.Set(index);
								}
							else
							if ( selected ) {
								patch->edgeSel.Set(index);
								}
							else {
								patch->edgeSel.Clear(index);
								}
							}
						hitRec = NULL;	// Reset it so we can exit
						}
					else if(type == PATCH_HIT_EDGE) {
						if( invert ) {
							if(patch->edgeSel[index])
								patch->edgeSel.Clear(index);
							else
								patch->edgeSel.Set(index);
							}
						else
						if ( selected ) {
							patch->edgeSel.Set(index);
							}
						else {
							patch->edgeSel.Clear(index);
							}
						hitRec = NULL;	// Reset it so we can exit
						}
					else
						hitRec = hitRec->Next();
					}
				edge_done:
				break;
				}
			case EP_PATCH: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto patch_done;
							int index = ((PatchHitData *)(hitRec->hitData))->index;
							if(patch->patchSel[index])
								patch->patchSel.Clear(index);
							else
								patch->patchSel.Set(index);
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto patch_done;
							patch->patchSel.Set(((PatchHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(patchData != (EditPatchData*)hitRec->modContext->localData)
								goto patch_done;
							patch->patchSel.Clear(((PatchHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int index = ((PatchHitData *)(hitRec->hitData))->index;
					int type = ((PatchHitData *)(hitRec->hitData))->type;
					if(type == PATCH_HIT_VERTEX && byVertex) {
						PatchVert &v = patch->verts[index];
						for(int i = 0; i < v.patches.Count(); ++i) {
							index = v.patches[i];
							if( invert ) {
								if(patch->patchSel[index])
									patch->patchSel.Clear(index);
								else
									patch->patchSel.Set(index);
								}
							else
							if ( selected ) {
								patch->patchSel.Set(index);
								}
							else {
								patch->patchSel.Clear(index);
								}
							}
						hitRec = NULL;	// Reset it so we can exit
						}
					else if(type == PATCH_HIT_PATCH) {
						if( invert ) {
							if(patch->patchSel[index])
								patch->patchSel.Clear(index);
							else
								patch->patchSel.Set(index);
							}
						else
						if ( selected ) {
							patch->patchSel.Set(index);
							}
						else {
							patch->patchSel.Clear(index);
							}
						hitRec = NULL;	// Reset it so we can exit
						}
					else
						hitRec = hitRec->Next();
					}
				patch_done:
				break;
				}
			case EP_ELEMENT: {
				if ( all ) {				
					BitArray mask(patch->patchSel.GetSize());
					mask.ClearAll();
					// Go thru all hit records, if face isn't part of mask, go get its
					// element and add it to the mask.
					while(hitRec) {
						// If the object changes, we're done!
						if(patchData != (EditPatchData*)hitRec->modContext->localData)
							goto element_done;
						int index = ((PatchHitData *)(hitRec->hitData))->index;
						if(!mask[index])
							mask |= patch->GetElement(index);
						hitRec = hitRec->Next();
						}
					element_done:
					if( invert ) {
						patch->patchSel ^= mask;
						}
					else
					if ( selected ) {
						patch->patchSel |= mask;
						}
					else {
						patch->patchSel &= ~mask;
						}
					}
				else {
					int index = ((PatchHitData *)(hitRec->hitData))->index;
					BitArray element = patch->GetElement(((PatchHitData *)(hitRec->hitData))->index);
					if( invert ) {
						patch->patchSel ^= element;
						}
					else
					if ( selected ) {
						patch->patchSel |= element;
						}
					else {
						patch->patchSel &= ~element;
						}
					hitRec = NULL;	// Reset it so we can exit	
					}
				break;
				}

			case EP_OBJECT:
			default:
				return;
			}
		patchData->UpdateChanges(patch, FALSE);
		if ( patchData->tempData ) {
			patchData->tempData->Invalidate(PART_SELECT);
			}
		PatchSelChanged();
		}

	UpdateSelectDisplay();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditPatchMod::ClearSelection(int level) 
	{
	// Don't do anything if at vertex level with verts turned off
	if(level == EP_VERTEX && !filterVerts)
		return;
	if(level == EP_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	ip->ClearCurNamedSelSet();
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patchData->BeginEdit(ip->GetTime());
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() ) {
			theHold.Put(new PatchRestore(patchData,this,patch,"ClearSelection"));
			}

		switch ( level ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
				patch->vecSel.ClearAll();
				break;
				}
			case EP_VERTEX: {
				patch->vertSel.ClearAll();
				break;
				}
			case EP_EDGE: {
				patch->edgeSel.ClearAll();
				break;
				}
			case EP_PATCH:
			case EP_ELEMENT: {
				patch->patchSel.ClearAll();
				break;
				}
			}
		patchData->UpdateChanges(patch, FALSE);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_SELECT);
			}
		PatchSelChanged();
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}


void UnselectHiddenPatches(int level, PatchMesh *patch)
{
switch ( level ) {
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case EP_HANDLE: 
		{
		for (int i = 0; i < patch->numVecs; i++)
			{
			int v = patch->getVec(i).vert;
			if (v >= 0 && patch->getVert(v).IsHidden())
				patch->vecSel.Set(i,FALSE);
			}
		break;
		}
	case EP_VERTEX: 
		{
		for (int i = 0; i < patch->numVerts; i++)
			{
			if (patch->getVert(i).IsHidden())
				patch->vertSel.Set(i,FALSE);
			}
		break;
		}
	case EP_EDGE: 
		{
		for (int i = 0; i < patch->numEdges; i++)
			{
			int a,b;
			a = patch->edges[i].v1;
			b = patch->edges[i].v2;
			if (patch->getVert(a).IsHidden() && patch->getVert(b).IsHidden())
				patch->edgeSel.Set(i,FALSE);
			}
		break;
		}
	case EP_PATCH: 
	case EP_ELEMENT:
		{
		for (int i = 0; i < patch->numPatches; i++)
			{
			if (patch->patches[i].IsHidden())
				patch->patchSel.Set(i,FALSE);
			}
		break;
		}

	}


}

void EditPatchMod::SelectAll(int level) 
	{
	// Don't do anything if at vertex level with verts turned off
	if(level == EP_VERTEX && !filterVerts)
		return;
	if(level == EP_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);
	ip->ClearCurNamedSelSet();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patchData->BeginEdit(ip->GetTime());
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() ) {
			theHold.Put(new PatchRestore(patchData,this,patch,"SelectAll"));
			}

		switch ( level ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
				patch->vecSel.SetAll();
				break;
				}
			case EP_VERTEX: {
				patch->vertSel.SetAll();
				break;
				}
			case EP_EDGE: {
				patch->edgeSel.SetAll();
				break;
				}
			case EP_PATCH:
			case EP_ELEMENT: {
				patch->patchSel.SetAll();
				break;
				}
			}
		UnselectHiddenPatches(level, patch);
		patchData->UpdateChanges(patch, FALSE);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_SELECT);
			}
		PatchSelChanged();
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditPatchMod::InvertSelection(int level) 
	{
	// Don't do anything if at vertex level with verts turned off
	if(level == EP_VERTEX && !filterVerts)
		return;
	if(level == EP_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);
	ip->ClearCurNamedSelSet();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patchData->BeginEdit(ip->GetTime());
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"InvertSelection"));

		switch ( level ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
				patch->vecSel = ~patch->vecSel;
				break;
				}
			case EP_VERTEX: {
				patch->vertSel = ~patch->vertSel;
				break;
				}
			case EP_EDGE: {
				patch->edgeSel = ~patch->edgeSel;
				break;
				}
			case EP_PATCH:
			case EP_ELEMENT: {
				patch->patchSel = ~patch->patchSel;
				break;
				}
			}
		UnselectHiddenPatches(level, patch);
		patchData->UpdateChanges(patch, FALSE);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_SELECT);
			}
		PatchSelChanged();
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditPatchMod::SetDisplaySurface(BOOL sw) {
	sw = TRUE;
	displaySurface = sw;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		patchData->displaySurface = sw;

		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetDisplayLattice(BOOL sw) {
	displayLattice = sw;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		patchData->displayLattice = sw;

		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;
		if(sw)
			patch->SetDispFlag(DISP_LATTICE);
		else
			patch->ClearDispFlag(DISP_LATTICE);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetPropagate(BOOL sw) {
	propagate = sw;
	}

void EditPatchMod::SetMeshSteps(int steps) {
	meshSteps = steps;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetMeshSteps(steps);
		patchData->meshSteps = steps;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
void EditPatchMod::SetMeshStepsRender(int steps) {
	meshStepsRender = steps;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetMeshStepsRender(steps);
		patchData->meshStepsRender = steps;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}
#endif // NO_OUTPUTRENDERER

BOOL EditPatchMod::Relaxing() {
	return (relax && relaxValue != 0.0f && relaxIter != 0) ? TRUE : FALSE;
	}

void EditPatchMod::SetRelax(BOOL v, BOOL redraw) {
	relax = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelax(relax);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetRelaxViewports(BOOL v, BOOL redraw) {
	relaxViewports = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelaxViewports(relaxViewports);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetRelaxValue(float v, BOOL redraw) {
	relaxValue = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelaxValue(relaxValue);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetRelaxIter(int v, BOOL redraw) {
	relaxIter = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelaxIter(relaxIter);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetRelaxBoundary(BOOL v, BOOL redraw) {
	relaxBoundary = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelaxBoundary(relaxBoundary);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetRelaxSaddle(BOOL v, BOOL redraw) {
	relaxSaddle = v;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetRelaxSaddle(relaxSaddle);
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_ALL);
			}
		}
	nodes.DisposeTemporary();
	if(redraw) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	}

void EditPatchMod::SetShowInterior(BOOL si)
{
	if (theHold.Holding()) theHold.Put(new EPBoolParamRestore(this, EPBoolParamRestore::kShowInterior));

	showInterior = si;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetShowInterior(si);
		patchData->showInterior = si;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// CAL-05/15/03: use true patch normals. (FID #1760)
void EditPatchMod::SetUsePatchNormals(BOOL usePatchNorm)
{
	if (theHold.Holding()) theHold.Put(new EPBoolParamRestore(this, EPBoolParamRestore::kUsePatchNorm));

	usePatchNormals = usePatchNorm;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;
	
	ip->GetModContexts(mcList, nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetUsePatchNormals(usePatchNorm);
		patchData->usePatchNormals = usePatchNorm;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}


// CAL-06/20/03: support spline surface generation (FID #1914)
// restore object to keep the surface generation parameters for undo/redo.
class EPGenSurfParamsRestore : public RestoreObj {
	BOOL undoGen, redoGen;
	float undoThresh, redoThresh;
	BOOL undoFlipNm, redoFlipNm;
	BOOL undoRmInt, redoRmInt;
	BOOL undoUseSel, redoUseSel;
	EditPatchMod *patchMod;

public:
	EPGenSurfParamsRestore(EditPatchMod *mod) : patchMod(mod)
	{
		undoGen = patchMod->generateSurface;
		undoThresh = patchMod->genSurfWeldThreshold;
		undoFlipNm = patchMod->genSurfFlipNormals;
		undoRmInt = patchMod->genSurfRmInterPatches;
		undoUseSel = patchMod->genSurfUseOnlySelSegs;
	}

	void Restore(int isUndo)
	{
		if(isUndo) {
			redoGen = patchMod->generateSurface;
			redoThresh = patchMod->genSurfWeldThreshold;
			redoFlipNm = patchMod->genSurfFlipNormals;
			redoRmInt = patchMod->genSurfRmInterPatches;
			redoUseSel = patchMod->genSurfUseOnlySelSegs;
		}
		patchMod->generateSurface = undoGen;
		patchMod->genSurfWeldThreshold = undoThresh;
		patchMod->genSurfFlipNormals = undoFlipNm;
		patchMod->genSurfRmInterPatches = undoRmInt;
		patchMod->genSurfUseOnlySelSegs = undoUseSel;

		patchMod->UpdateGenSurfGroupControls();
		patchMod->SetGenSurfGroupEnables();

		patchMod->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	}

	void Redo()
	{
		patchMod->generateSurface = redoGen;
		patchMod->genSurfWeldThreshold = redoThresh;
		patchMod->genSurfFlipNormals = redoFlipNm;
		patchMod->genSurfRmInterPatches = redoRmInt;
		patchMod->genSurfUseOnlySelSegs = redoUseSel;
		
		patchMod->UpdateGenSurfGroupControls();
		patchMod->SetGenSurfGroupEnables();

		patchMod->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	}

	int Size() { return 8 * sizeof(BOOL) + 2 * sizeof(float) + 2 * sizeof(void*); }

	TSTR Description() { return TSTR(_T("EPGenSurfParamsRestore")); }
};


// CAL-05/01/03: support spline surface generation (FID #1914)
// Invalidate the PatchData of surface generated from splines
void EditPatchMod::InvalidateGenSurfacePatchData()
{
	if ( !ip ) return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList, nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData || !patchData->splineInput ) continue;

		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch) continue;

		if(theHold.Holding()) {
			theHold.Put(new PatchDataInvalidateRestore(patchData));
			theHold.Put(new EPVertMapRestore(patchData));
			theHold.Put(new FinalPatchRestore(&(patchData->finalPatch)));
			theHold.Put(new PatchDataFlagsRestore(patchData, EPD_HASDATA));
		}
		if ( patchData->tempData )
			patchData->TempData(this)->Invalidate(PART_ALL, FALSE);
		patchData->ClearFlag(EPD_HASDATA);
	}

	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetGenerateSurface(BOOL genSurf)
{
	theHold.Begin();
	if (theHold.Holding()) theHold.Put(new EPGenSurfParamsRestore(this));
	
	generateSurface = genSurf;
	InvalidateGenSurfacePatchData();

	if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
}

void EditPatchMod::SetGenSurfWeldThreshold(float thresh)
{
	theHold.Begin();
	if (theHold.Holding()) theHold.Put(new EPGenSurfParamsRestore(this));

	genSurfWeldThreshold = thresh;
	if (generateSurface) InvalidateGenSurfacePatchData();

	if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
}

void EditPatchMod::SetGenSurfFlipNormals(BOOL flipNorm)
{
	theHold.Begin();
	if (theHold.Holding()) theHold.Put(new EPGenSurfParamsRestore(this));

	genSurfFlipNormals = flipNorm;
	if (generateSurface) InvalidateGenSurfacePatchData();

	if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
}

void EditPatchMod::SetGenSurfRmInterPatches(BOOL rmInter)
{
	theHold.Begin();
	if (theHold.Holding()) theHold.Put(new EPGenSurfParamsRestore(this));

	genSurfRmInterPatches = rmInter;
	if (generateSurface) InvalidateGenSurfacePatchData();

	if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
}

void EditPatchMod::SetGenSurfUseOnlySelSegs(BOOL useSel)
{
	theHold.Begin();
	if (theHold.Holding()) theHold.Put(new EPGenSurfParamsRestore(this));

	genSurfUseOnlySelSegs = useSel;
	if (generateSurface) InvalidateGenSurfacePatchData();

	if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
}


// CAL-05/01/03: support spline surface generation (FID #1914)
//				 This function is copied from the Surface modifier.

// Quad patch layout
//
//   A---> ad ----- da <---D
//   |                     |
//   |                     |
//   v                     v
//   ab    i1       i4     dc
//
//   |                     |
//   |                     |
// 
//   ba    i2       i3     cd
//   ^					   ^
//   |                     |
//   |                     |
//   B---> bc ----- cb <---C
//
// vertices ( a b c d ) are in counter clockwise order when viewed from 
// outside the surface

void EditPatchMod::BuildPatchFromShape(TimeValue t, SplineShape *splShape, PatchMesh &pmesh)
{
	DbgAssert(splShape);		if (!splShape) return;
	ShapeObject *shape = (ShapeObject *)splShape;

	// If the shape can convert itself to a BezierShape, have it do so!
	BezierShape bShape;
	if(shape->CanMakeBezier())
		shape->MakeBezier(t, bShape);
	else {
		PolyShape pShape;
		shape->MakePolyShape(t, pShape);
		bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
		}

	PatchData PatchStuff;
	float amount = GetGenSurfWeldThreshold();
	BOOL selsegs = GetGenSurfUseOnlySelSegs();
	
	amount = amount * amount;
	LimitValue(amount, 0.0f, 1000000.0f);
	
	// Our shapes are now organized for patch-making -- Let's do the sides!
	int polys = bShape.splineCount;
	int poly;
	int levelVerts = 0, levelVecs = 0, levelPatches = 0, nverts = 0, nvecs = 0, npatches = 0;
	int levelTVerts = 0, ntverts = 0, ntpatches = 0;
	int i,j;
	BOOL anyClosed = FALSE;
	Tab<BindSplines> bindSplineList;

	//add any hook points first
	for (int bct = 0; bct < bShape.bindList.Count(); bct++)
		{
		BindSplines bsp;
		Spline3D *pointSpline;
		Spline3D *segSpline;
		if ( (bShape.bindList[bct].pointSplineIndex <  bShape.SplineCount()) &&
			 (bShape.bindList[bct].segSplineIndex <  bShape.SplineCount()) &&
			 (!bShape.splines[bShape.bindList[bct].pointSplineIndex]->Closed())
			)
			{
			pointSpline = bShape.splines[bShape.bindList[bct].pointSplineIndex];
			segSpline = bShape.splines[bShape.bindList[bct].segSplineIndex];

			//build a spline based on that edge
			Spline3D work;
			Point3 zero(0,0,0);
			work.AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, zero, zero, zero));
			work.AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, zero, zero, zero));

			SplineKnot ka,kc;
			ka = segSpline->GetKnot(bShape.bindList[bct].seg);
			//do we need to check if this spline is closed and use the start vert
			if ( segSpline->Closed() && 
				 (bShape.bindList[bct].seg ==  (segSpline->KnotCount()-1))
				)
				kc = segSpline->GetKnot(0);
			else kc = segSpline->GetKnot(bShape.bindList[bct].seg+1);

			work.SetKnot(0, ka);
			work.SetKnot(1, kc);
			//refine from the spline refine
			// Get the knot points
			Point3 v00 = work.GetKnotPoint(0);
			Point3 v30 = work.GetKnotPoint(1);

			Point3 point,subPoint;
			if (bShape.bindList[bct].isEnd)
				{
				point =pointSpline->GetKnotPoint(pointSpline->KnotCount()-1);
				subPoint =pointSpline->GetKnotPoint(pointSpline->KnotCount()-2);
				}
			else
				{
				point =pointSpline->GetKnotPoint(0);
				subPoint =pointSpline->GetKnotPoint(1);
				}

			Point3 v10 = work.GetOutVec(0);
			Point3 v20 = work.GetInVec(1);
			Point3 v01 = v00 + (v10 - v00) * 0.5f;
			Point3 v21 = v20 + (v30 - v20) * 0.5f;
			Point3 v11 = v10 + (v20 - v10) * 0.5f;
			Point3 v02 = v01 + (v11 - v01) * 0.5f;
			Point3 v12 = v11 + (v21 - v11) * 0.5f;
			Point3 v03 = v02 + (v12 - v02) * 0.5f;

			Point3 av,bv,cv,dv;
			av = v01;
			bv = v02;
			cv = v12;
			dv = v21;

			SplineKnot af(KTYPE_BEZIER, LTYPE_CURVE, v00, v00, av);
			SplineKnot bf(KTYPE_BEZIER, LTYPE_CURVE, point, bv, cv);
			SplineKnot cf(KTYPE_BEZIER, LTYPE_CURVE, v30, dv, v30);

			bsp.a = af;
			bsp.b = bf;
			bsp.c = cf;
			bsp.subHook = subPoint;

			float adist ,bdist;

			adist = LengthSquared(bsp.a.Knot()-bsp.b.Knot());
			bdist = LengthSquared(bsp.c.Knot()-bsp.b.Knot());
			if ( (adist > amount) && (bdist > amount) )
				bindSplineList.Append(1,&bsp,1);

			}
		}

	//add verts
	for (poly = 0; poly < polys; ++poly) {
		//number of sleected segements
		int nselsegs = 0;
		if (poly < bShape.segSel.polys)
			nselsegs = bShape.segSel.sel[poly].NumberSet();
		//check if !selsegs || nselsegs > 0
		if ((!selsegs) || (nselsegs>0))
			{
			Spline3D *spline = bShape.splines[poly];
			if (spline->KnotCount() > 1)
				{
				if(spline->Closed())
					anyClosed = TRUE;
				else anyClosed = FALSE;

				//add spline stuff here
				for (i = 0;i<spline->KnotCount();i++)
					//add closed spline
					PatchStuff.AddVert(spline->GetKnotPoint(i), amount);
				}
			}
		}

	//add in hook poit edges
	for (i = 0; i < bindSplineList.Count(); i++)
		{
		//do I really need this one since it should already be in outr vert list?
		PatchStuff.AddVert(bindSplineList[i].a.Knot(), amount,TRUE);
		PatchStuff.AddVert(bindSplineList[i].b.Knot(), amount,TRUE,TRUE);
		//do I really need this one since it should already be in outr vert list?
		PatchStuff.AddVert(bindSplineList[i].c.Knot(), amount,TRUE);
		}

	//add connections
	//add verts
	for(poly = 0; poly < polys; ++poly) {
		int nselsegs = 0;
		if (poly < bShape.segSel.polys)
			nselsegs = bShape.segSel.sel[poly].NumberSet();
		//check if !selsegs || nselsegs > 0
		if ((!selsegs) || (nselsegs>0))
			{
			Spline3D *spline = bShape.splines[poly];
			if (spline->KnotCount() >1)
				{
				if(spline->Closed())
					anyClosed = TRUE;
				else anyClosed = FALSE;
				//add spline stuff here
				for (int i = 0;i<spline->KnotCount();i++)
					{
					//add closed spline
					int st,en;
					st = i;
					en = i-1;
					if (en < 0) en = 0;
					
					if ((!selsegs) || (bShape.segSel.sel[poly][st]) || (bShape.segSel.sel[poly][en]))
						{
						if (anyClosed) 
							{
							if (i==0)
								PatchStuff.AddVecs2(spline->GetKnotPoint(i),spline->GetOutVec(i),spline->GetInVec(i),
								   spline->GetKnotPoint(i+1),spline->GetKnotPoint(spline->KnotCount()-1),
								   amount);
							else if (i==(spline->KnotCount()-1))
								PatchStuff.AddVecs2(spline->GetKnotPoint(i),spline->GetOutVec(i),spline->GetInVec(i),
								   spline->GetKnotPoint(0),spline->GetKnotPoint(i-1),
								   amount);
							else
								PatchStuff.AddVecs2(spline->GetKnotPoint(i),spline->GetOutVec(i),spline->GetInVec(i),
								   spline->GetKnotPoint(i+1),spline->GetKnotPoint(i-1),
								   amount);				
							}
						else
							//add open spline
							{
							if (i==0)
								PatchStuff.AddVecs1Out(spline->GetKnotPoint(i),spline->GetOutVec(i),
								   spline->GetKnotPoint(i+1),
								   amount);
							else if (i==(spline->KnotCount()-1))
								PatchStuff.AddVecs1In(spline->GetKnotPoint(i),spline->GetInVec(i),
								   spline->GetKnotPoint(i-1),
								   amount);
							else
								PatchStuff.AddVecs2(spline->GetKnotPoint(i),spline->GetOutVec(i),spline->GetInVec(i),
								   spline->GetKnotPoint(i+1),spline->GetKnotPoint(i-1),
								   amount);
							}
						}
					}
				}
			}
		}

	//add in hook point connections
	for (i = 0; i < bindSplineList.Count(); i++)
		{
		SplineKnot a = bindSplineList[i].a;
		SplineKnot b = bindSplineList[i].b;
		SplineKnot c = bindSplineList[i].c;
		PatchStuff.AddVecs1Out(a.Knot(),a.OutVec(),
								b.Knot(),
							   amount);
		PatchStuff.AddVecs2(b.Knot(),b.OutVec(),b.InVec(),
						   c.Knot(),a.Knot(),
						   amount);				
		PatchStuff.AddVecs1In(c.Knot(),c.InVec(),
						   b.Knot(),
						   amount);
		}

	BitArray hookVertsBA,subHookVertsBA;
	hookVertsBA.SetSize(PatchStuff.VertList.Count());
	hookVertsBA.ClearAll();

 	for (i = 0; i < PatchStuff.hookVerts.Count(); i++)
		{
		hookVertsBA.Set(PatchStuff.hookVerts[i]);
		}

	//build face data
	if (!PatchStuff.CreateFaceData())
		{
		for (i=0;i<PatchStuff.VertData.Count();i++)
			{
			PatchStuff.VertData[i]->OutVerts.ZeroCount();
			PatchStuff.VertData[i]->InVerts.ZeroCount();
			PatchStuff.VertData[i]->OutVecs.ZeroCount();
			PatchStuff.VertData[i]->InVecs.ZeroCount();
			if (PatchStuff.VertData[i] != NULL)
				delete (PatchStuff.VertData[i]);
			PatchStuff.VertData[i] = NULL;
			}
	
		PatchStuff.hookVerts.ZeroCount();
		PatchStuff.hookPoints.ZeroCount();
		PatchStuff.VecList.ZeroCount();
		PatchStuff.VecCount.ZeroCount();
		PatchStuff.VertList.ZeroCount();
		PatchStuff.VertData.ZeroCount();
		PatchStuff.FaceData.ZeroCount();
		PatchStuff.Verts33.ZeroCount();

		pmesh.setNumTVerts(0);
		pmesh.setNumTVPatches(0);
		pmesh.setNumVerts(0);
		pmesh.setNumVecs(0);

		return;
		}

	//build remove interior face data
	int rcaps=0;
	int itemp = GetGenSurfFlipNormals();

 	PatchStuff.RemoveInteriorFaces();
	PatchStuff.UnifyNormals(itemp);

	//nuke degenerate patches created by the hookverts
	PatchStuff.NukeDegenerateHookPatches(hookVertsBA);

	itemp = GetGenSurfRmInterPatches();
	PatchStuff.DeleteInteriorFaces(itemp,rcaps);
	PatchStuff.RecombineVectors();

	if (PatchStuff.FaceData.Count() ==0)
		{
		pmesh.setNumTVerts(0);
		pmesh.setNumTVPatches(0);
		pmesh.setNumVerts(0);
		pmesh.setNumVecs(0);

		pmesh.buildLinkages();
		pmesh.computeInteriors();

		// CAL-05/01/03: this will be taking care of later by ModifyObject()
		/*
		itemp = GetMeshSteps();
		if (itemp < 0 ) itemp = 0;

		pmesh.SetMeshSteps(itemp);
#ifndef NO_OUTPUTRENDERER
		pmesh.SetMeshStepsRender(itemp);
#endif // NO_OUTPUTRENDERER
		*/

		for (i=0;i<PatchStuff.VertData.Count();i++)
			{
			PatchStuff.VertData[i]->OutVerts.ZeroCount();
			PatchStuff.VertData[i]->InVerts.ZeroCount();
			PatchStuff.VertData[i]->OutVecs.ZeroCount();
			PatchStuff.VertData[i]->InVecs.ZeroCount();
			if (PatchStuff.VertData[i] != NULL)
				delete (PatchStuff.VertData[i]);
			PatchStuff.VertData[i] = NULL;
			}
		PatchStuff.hookVerts.ZeroCount();
		PatchStuff.hookPoints.ZeroCount();
		PatchStuff.VecList.ZeroCount();
		PatchStuff.VecCount.ZeroCount();
		PatchStuff.VertList.ZeroCount();
		PatchStuff.VertData.ZeroCount();
		PatchStuff.FaceData.ZeroCount();
		PatchStuff.Verts33.ZeroCount();

		return;
		}

	pmesh.setNumVerts(PatchStuff.VertList.Count());

	int Num3Faces=0,Num4Faces=0;
	
	for (i=0;i<PatchStuff.FaceData.Count();i++)
		{
		if (PatchStuff.FaceData[i].face3)
			Num3Faces++;
		else
			Num4Faces++;
		}

	pmesh.setNumVecs(PatchStuff.VecList.Count()+(3*Num3Faces)+(4*Num4Faces));
	pmesh.setNumPatches(PatchStuff.FaceData.Count());
	pmesh.setNumTVerts(0);
	pmesh.setNumTVPatches(0);

	int vec = 0;

	for (i=0;i<PatchStuff.VertList.Count();i++)
		{
		pmesh.verts[i].flags = 0;
		pmesh.setVert(i, PatchStuff.VertList[i]);
		}

	for (i=0;i<PatchStuff.VecList.Count();i++)
		{
		pmesh.setVec(i, PatchStuff.VecList[i]);
		}

	int invec = PatchStuff.VecList.Count();

	for (j=0;j<PatchStuff.FaceData.Count();j++)
		{
		 if (PatchStuff.FaceData[j].face3)
			{
			pmesh.MakeTriPatch(j, 
				    PatchStuff.FaceData[j].a,PatchStuff.FaceData[j].ab,PatchStuff.FaceData[j].ba,
				    PatchStuff.FaceData[j].b,PatchStuff.FaceData[j].bc,PatchStuff.FaceData[j].cb,
				    PatchStuff.FaceData[j].c,PatchStuff.FaceData[j].ca,PatchStuff.FaceData[j].ac,
				    invec, invec+1, invec+2, 
				    1);
			invec += 3;
			}
		else 
			//Makea a quad patch
			{
			pmesh.MakeQuadPatch(j, 
					  PatchStuff.FaceData[j].a,PatchStuff.FaceData[j].ab,PatchStuff.FaceData[j].ba,
					  PatchStuff.FaceData[j].b,PatchStuff.FaceData[j].bc,PatchStuff.FaceData[j].cb,
				      PatchStuff.FaceData[j].c,PatchStuff.FaceData[j].cd,PatchStuff.FaceData[j].dc,
				      PatchStuff.FaceData[j].d,PatchStuff.FaceData[j].da,PatchStuff.FaceData[j].ad,
				      invec, invec+1, invec+2, invec+3,
			          1);
			invec += 4;
			}
		}

	pmesh.buildLinkages();
	pmesh.computeInteriors();

	//need to do this after the linkage since it used the edge list stuff
	if (amount <= 0.01f) amount = 0.01f;
	for (i = 0; i < PatchStuff.hookPoints.Count(); i++)
		{
	//need to also define the edge which is to be attached to
		BOOL found = FALSE;
		Point3 a,b;
		a = bindSplineList[i].a.Knot();
		b = bindSplineList[i].c.Knot();
		int segID;
		int ecount = pmesh.getNumEdges();
		for (int j = 0; j < pmesh.getNumEdges(); j++)
			{
			int aid,bid;
			aid = pmesh.edges[j].v1; 
			bid = pmesh.edges[j].v2; 
			Point3 at, bt;
			at = pmesh.getVert(aid).p;
			bt = pmesh.getVert(bid).p;
			if ( ((LengthSquared(at-a)<amount) && (LengthSquared(bt-b)<amount)) ||
				 ((LengthSquared(bt-a)<amount) && (LengthSquared(at-b)<amount))
			   )
				{	
				found = TRUE;
				segID = j;
				j = pmesh.getNumEdges();
				}
			}
		if (found)
			{
			pmesh.AddHook(PatchStuff.hookPoints[i],segID);
			}
		}

	// CAL-05/01/03: this will be taking care of later by ModifyObject()
	/*
	itemp = GetMeshSteps();
	if (itemp < 0 ) itemp = 0;

	pmesh.SetMeshSteps(itemp);
#ifndef NO_OUTPUTRENDERER
	pmesh.SetMeshStepsRender(itemp);
#endif // NO_OUTPUTRENDERER
	*/

	for (i=0;i<PatchStuff.VertData.Count();i++)
		{
		PatchStuff.VertData[i]->OutVerts.ZeroCount();
		PatchStuff.VertData[i]->InVerts.ZeroCount();
		PatchStuff.VertData[i]->OutVecs.ZeroCount();
		PatchStuff.VertData[i]->InVecs.ZeroCount();
		if (PatchStuff.VertData[i] != NULL)
			delete (PatchStuff.VertData[i]);
		PatchStuff.VertData[i] = NULL;
		}
	
	PatchStuff.hookVerts.ZeroCount();
	PatchStuff.hookPoints.ZeroCount();
	PatchStuff.VecList.ZeroCount();
	PatchStuff.VecCount.ZeroCount();
	PatchStuff.VertList.ZeroCount();
	PatchStuff.VertData.ZeroCount();
	PatchStuff.FaceData.ZeroCount();
	PatchStuff.Verts33.ZeroCount();
}


// CAL-05/01/03: support spline surface generation (FID #1914)
//				 This following functions are copied from the Surface modifier.

void PatchData::AddVert(Point3 vert ,float threshold, BOOL isHook, BOOL isHookPoint)
{
	//add vert
	//go through all verts looking for a match
	int found = -1;
	for (int i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(vert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 found = VertData[i]->Index;
			 i = VertData.Count();
			}
		}

	//if no match add vert and vecs
	PatchVerts *Hold;
	Hold = new PatchVerts;
	Point3 HoldPoint;
	if (found == -1)
		{
		//add vert to list
		Hold->Index = VertList.Count();
		VertData.Append(1,&Hold,1);

		HoldPoint = vert;
		VertList.Append(1,&HoldPoint,1);
		int id = VertList.Count()-1;
		if (isHook)
			hookVerts.Append(1,&id,1);
		if (isHookPoint)
			hookPoints.Append(1,&id,1);
		}
	//else just add vec
	else
		{
		Hold->Index = found;
		if (isHook)
			hookVerts.Append(1,&found,1);
		if (isHookPoint)
			hookPoints.Append(1,&found,1);

		VertData.Append(1,&Hold,1);
		}
}

void PatchData::AddVecs2(Point3 vert,Point3 outvec,Point3 invec,Point3 outvert,Point3 invert,float threshold)
{
	int found = -1;
	int foundin = -1;
	int foundout = -1;
	int outv=-1,inv=-1;

	//find matching vert
	for (int i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(vert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 found = i;
			 i = VertData.Count();
			}
		}

	//find matching out vert
	for (i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(outvert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 outv = i;
			 i = VertData.Count();
			}
		}

	//Add Out Vec to veclist
	Point3 HoldPoint;
	int ct = 0;
	//keep a loop back vert
	if (found != outv)
		{
		if (foundout == -1)
			{
			foundout = VecList.Count();
			VecList.Append(1,&outvec,1);
			VecCount.Append(1,&ct,1);
			VertData[found]->OutVecs.Append(1,&foundout,1);
			VertData[found]->OutVerts.Append(1,&outv,1);
			}
			//else just update Vertdata
			else
			{
			VertData[found]->OutVecs.Append(1,&foundout,1);
			VertData[found]->OutVerts.Append(1,&outv,1);
			}
		}

	//find matching in vert
	for (i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(invert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 inv = i;
			 i = VertData.Count();
			}
		}

	if (found != inv)
		{
		//add in vec
		if (foundin == -1)
			{
			foundin = VecList.Count();
			VecList.Append(1,&invec,1);
			VecCount.Append(1,&ct,1);
			VertData[found]->InVecs.Append(1,&foundin,1);
			VertData[found]->InVerts.Append(1,&inv,1);
			}
			//else just update Vertdata
			else 
			{
			VertData[found]->InVecs.Append(1,&foundin,1);
			VertData[found]->InVerts.Append(1,&inv,1);
			}
		}
}

void PatchData::AddVecs1Out(Point3 vert,Point3 outvec,Point3 outvert,float threshold)
{
	int found = -1;
	int foundin = -1;
	int foundout = -1;
	int outv=-1,inv=-1;

	//find matching vert
	for (int i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(vert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 found = i;
			 i = VertData.Count();
			}
		}

	//find matching out vert
	for (i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(outvert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 outv = i;
			 i = VertData.Count();
			}
		}

	//Add Out Vec to veclist
	Point3 HoldPoint;
	int ct = 0;
	if (found!=outv)
		{
		if (foundout == -1)
			{
			foundout = VecList.Count();
			VecList.Append(1,&outvec,1);
			VecCount.Append(1,&ct,1);
			VertData[found]->OutVecs.Append(1,&foundout,1);
			VertData[found]->OutVerts.Append(1,&outv,1);
			}
			//else just update Vertdata
			else
			{
			VertData[found]->OutVecs.Append(1,&foundout,1);
			VertData[found]->OutVerts.Append(1,&outv,1);
			}
		}
}

void PatchData::AddVecs1In(Point3 vert,Point3 invec,Point3 invert,float threshold)
{
	int found = -1;
	int foundin = -1;
	int foundout = -1;
	int outv=-1,inv=-1;

	//find matching vert
	for (int i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(vert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 found = i;
			 i = VertData.Count();
			}
		}

	//find matching in vert
	for (i=0;i<VertData.Count();i++)
		{
		if (LengthSquared(invert-VertList[VertData[i]->Index]) <= threshold)
			 {
			 inv = i;
			 i = VertData.Count();
			}
		}

	//add in vec
	int ct = 0;
	if (found != inv)
		{
		if (foundin == -1)
			{
			foundin = VecList.Count();
			VecList.Append(1,&invec,1);
			VecCount.Append(1,&ct,1);

			VertData[found]->InVecs.Append(1,&foundin,1);
			VertData[found]->InVerts.Append(1,&inv,1);
			}
	//else just update Vertdata
			else 
			{
			VertData[found]->InVecs.Append(1,&foundin,1);
			VertData[found]->InVerts.Append(1,&inv,1);
			}
		}
}

int PatchData::FindVec(int SourceVert, int DestVert)
{
	int i;
	int tvec,vec=-1;
	int temp;
	//find ab vector
	for (i=0;i<VertData[SourceVert]->OutVerts.Count();i++)
		{
		temp = VertData[SourceVert]->OutVerts[i];
		if (DestVert==temp) 
			{
			tvec = VertData[SourceVert]->OutVecs[i];
			vec = tvec;
			continue;
			}
		}
	if (vec==-1)
		{
		for (i=0;i<VertData[SourceVert]->InVerts.Count();i++)
			{
			temp = VertData[SourceVert]->InVerts[i];
			if (DestVert==temp) 
				{
				tvec = VertData[SourceVert]->InVecs[i];
				vec = tvec;
				continue;
				}
			}
		}
	if (vec!=-1)
		{
		VecCount[vec]++;
		return vec;
		}
	else
		//add new vec and vecount
	{
		int ct = 1;
		Point3 t;
		t = VecList[tvec];
		VecCount.Append(1,&ct,1);
		VecList.Append(1,&t,1);
		return (VecList.Count()-1);
	}
}

void PatchData::AddFace3(int va, int vb, int vc)
{
	int found = -1;
	PatchFaces Hold;

	int ab=-1,ba=-1;
	int bc=-1,cb=-1;
	int ca=-1,ac=-1;
	int a,b,c;

	a = VertData[va]->Index;
	b = VertData[vb]->Index;
	c = VertData[vc]->Index;

	for (int i=0;i<FaceData.Count();i++)
		{
		if ( ((FaceData[i].a == a) && (FaceData[i].b == b) && (FaceData[i].c == c)) ||
			((FaceData[i].a == b) && (FaceData[i].b == c) && (FaceData[i].c == a)) ||
			((FaceData[i].a == c) && (FaceData[i].b == a) && (FaceData[i].c == b)) ||
			((FaceData[i].a == c) && (FaceData[i].b == b) && (FaceData[i].c == a)) ||
			((FaceData[i].a == a) && (FaceData[i].b == c) && (FaceData[i].c == b)) ||
			((FaceData[i].a == b) && (FaceData[i].b == a) && (FaceData[i].c == c)) 
			)
			{
			found = i;
			}
		}

	if (found==-1)
		{
		int flip = 0;
		//put code in here to build vectors and track vector count
		Hold.face3 = 1;
		Hold.a = a;
		Hold.b = b;
		Hold.c = c;

		Hold.ab = FindVec(va,vb);
		Hold.ba = FindVec(vb,va);

		Hold.bc = FindVec(vb,vc);
		Hold.cb = FindVec(vc,vb);

		Hold.ca = FindVec(vc,va);
		Hold.ac = FindVec(va,vc);

		Hold.va = va;
		Hold.vb = vb;
		Hold.vc = vc;
		Hold.tagged = 0;

		FaceData.Append(1,&Hold,1);
		}
}

//fix this procedure to make 4 faces
void PatchData::AddFace4(int va, int vb, int vc, int vd)
{
	int found = -1,j;
	PatchFaces Hold;
	int FList[5];
	int v0,v1,v2,v3;
	int fa,fb,fc,fd;
	int f1,f2,f3,f4;

	int ab=-1,ba=-1;
	int bc=-1,cb=-1;
	int cd=-1,dc=-1;
	int da=-1,ad=-1;
	int a,b,c,d;

	a = VertData[va]->Index;
	b = VertData[vb]->Index;
	c = VertData[vc]->Index;
	d = VertData[vd]->Index;

	FList[0] = a;
	FList[1] = b;
	FList[2] = c;
	FList[3] = d;

	for (int i=0;i<FaceData.Count();i++)
		{
		fa = FaceData[i].a;
		fb = FaceData[i].b;
		fc = FaceData[i].c;
		fd = FaceData[i].d;
		for (j=0;j<4;j++)
			{
			v0 = j%4;
			v1 = (j+1)%4;
			v2 = (j+2)%4;
			v3 = (j+3)%4;
			 
			f1 = FList[v0];
			f2 = FList[v1];
			f3 = FList[v2];
			f4 = FList[v3];

			//check against all 4 faces
			if (!FaceData[i].face3)
				{
				if ( 
					( (f1 == fa) && (f2 == fb) && (f3 == fc) && (f4 == fd) ) ||
					( (f1 == fd) && (f2 == fc) && (f3 == fb) && (f4 == fa) ) 
					)
					{
					found = i;
					i = FaceData.Count();
					j=4;
					}
				}
			//check against 3 faces
			else
				{
				if ( 
					( (f1 == fa) && (f2 == fb) && (f3 == fc) ) ||
					( (f1 == fb) && (f2 == fc) && (f3 == fa) ) ||
					( (f1 == fc) && (f2 == fa) && (f3 == fb) ) ||
					( (f1 == fc) && (f2 == fb) && (f3 == fa) ) ||
					( (f1 == fa) && (f2 == fc) && (f3 == fb) ) ||
					( (f1 == fb) && (f2 == fa) && (f3 == fc) ) 
					)
					{
					found = i;
					i = FaceData.Count();
					j=4;
					}
				}
			}
		}

	if (found==-1)
		{
		int flip = 0;

		Hold.face3 = 0;
		Hold.a = a;
		Hold.b = b;
		Hold.c = c;
		Hold.d = d;

		Hold.ab = FindVec(va,vb);
		Hold.ba = FindVec(vb,va);

		Hold.bc = FindVec(vb,vc);
		Hold.cb = FindVec(vc,vb);

		Hold.cd = FindVec(vc,vd);
		Hold.dc = FindVec(vd,vc);

		Hold.da = FindVec(vd,va);
		Hold.ad = FindVec(va,vd);

		Hold.va = va;
		Hold.vb = vb;
		Hold.vc = vc;
		Hold.vd = vd;
		Hold.tagged = 0;

		FaceData.Append(1,&Hold,1);
		}
}

int PatchData::RecurseFaces3(int FirstVert,int CurrentVertex,int *List, int Count)
{
	int i,NextVert;

	Count++;
	if (Count == 4)		
		{
		if (FirstVert == CurrentVertex)
			{
			//add face to list
			AddFace3(List[0],List[1],List[2]);
			}
		return TRUE;
		}
	else
		{
		for (i=0;i<VertData[CurrentVertex]->OutVerts.Count();i++)
			{
			 NextVert = VertData[CurrentVertex]->OutVerts[i];
			 List[Count] =  NextVert;
			 RecurseFaces3(FirstVert,NextVert, List, Count);
			}
	 
		for (i=0;i<VertData[CurrentVertex]->InVerts.Count();i++)
			{
			 NextVert = VertData[CurrentVertex]->InVerts[i];
			 if (List[Count-1] != NextVert)
				{
				 List[Count] =  NextVert;
				 RecurseFaces3(FirstVert,NextVert, List, Count);
				}
			}
		}
	return FALSE;
}

int PatchData::RecurseFaces4(int FirstVert,int CurrentVertex,int *List, int Count)
{
	int i,NextVert,k,check;

	Count++;

	if (Count == 5)		
		{
		if (FirstVert == CurrentVertex)
			{
			//add face to list
			AddFace4(List[0],List[1],List[2],List[3]);
			}
		return TRUE;
		}
	else
		{
		for (i=0;i<VertData[CurrentVertex]->OutVerts.Count();i++)
			{
			 NextVert = VertData[CurrentVertex]->OutVerts[i];
			 check = 1;
			 for (k=1;k<(Count);k++)
				{
				 if (List[k] == NextVert)
					 check = 0;
			    }
			 if (check)
				{

				 List[Count] =  NextVert;
				 RecurseFaces4(FirstVert,NextVert, List, Count);
				}
			}
	 
		for (i=0;i<VertData[CurrentVertex]->InVerts.Count();i++)
			{
			 NextVert = VertData[CurrentVertex]->InVerts[i];
			 check = 1;
			 for (k=1;k<(Count);k++)
				{
				 if (List[k] == NextVert)
					 check = 0;
			    }
			 if (check)
				{

				 List[Count] =  NextVert;
				 RecurseFaces4(FirstVert,NextVert, List, Count);
				}
			}
		}
	return FALSE;
}

BOOL PatchData::CreateFaceData()
{
	 int count,FaceList[7];

	//get 3 face data
	 for (int i=0; i<VertData.Count();i++)
		{
		 count = 0;
		 FaceList[0] = i;
		 RecurseFaces3(i,i,FaceList, count);
		 if (GetAsyncKeyState (VK_ESCAPE)) return FALSE;
		}

	//get 4 face data
	 for (i=0; i<VertData.Count();i++)
		{
		 count = 0;
		 FaceList[0] = i;
		 RecurseFaces4(i,i,FaceList, count);
		 if (GetAsyncKeyState (VK_ESCAPE)) return FALSE;
		}

	return TRUE;
}

void PatchData::FlipFace(int facea)
{
	if (FaceData[facea].face3)
		{
		//flip direction
		int temp,temp2;

		//flip verts 
		temp = FaceData[facea].a;
		FaceData[facea].a = FaceData[facea].c;
		FaceData[facea].c = temp;

		temp = FaceData[facea].va;
		FaceData[facea].va = FaceData[facea].vc;
		FaceData[facea].vc = temp;

		//flip vecs 
		temp = FaceData[facea].ab;
		temp2 = FaceData[facea].ba;
		FaceData[facea].ab = FaceData[facea].cb;
		FaceData[facea].ba = FaceData[facea].bc;

		FaceData[facea].bc = temp2;
		FaceData[facea].cb = temp;

		temp = FaceData[facea].ca;
		FaceData[facea].ca = FaceData[facea].ac;
		FaceData[facea].ac = temp;
		}
	else
		{
		//flip direction
		int temp,temp2,temp3;

		//flip verts 
		temp = FaceData[facea].a;
		temp2 = FaceData[facea].b;
		FaceData[facea].a = FaceData[facea].d;
		FaceData[facea].b = FaceData[facea].c;
		FaceData[facea].c = temp2;
		FaceData[facea].d = temp;

		temp = FaceData[facea].va;
		temp2 = FaceData[facea].vb;
		FaceData[facea].va = FaceData[facea].vd;
		FaceData[facea].vb = FaceData[facea].vc;
		FaceData[facea].vc = temp2;
		FaceData[facea].vd = temp;
							 
		//flip vecs 
		temp = FaceData[facea].ab;
		temp2 = FaceData[facea].ba;
		FaceData[facea].ab = FaceData[facea].dc;
		FaceData[facea].ba = FaceData[facea].cd;

		temp3 = FaceData[facea].bc;
		FaceData[facea].bc = FaceData[facea].cb;
		FaceData[facea].cb = temp3;

		FaceData[facea].cd = temp2;
		FaceData[facea].dc = temp;

		temp = FaceData[facea].da;
		FaceData[facea].da = FaceData[facea].ad;
		FaceData[facea].ad = temp;
		}
}

void PatchData::CheckFace(int face,int a, int b)
{
	//find faces that share a,b and not equal to face
	int i;

	for (i=0;i<FaceData.Count();i++)
		{
	//	if ((i != face) && (!FaceData[i].tagged) && (FaceData[i].interior != 2)
	//		 &&(!((FaceData[i].interior == 1) && (R)))   )
		if ((i != face) && (!FaceData[i].tagged))
	//		&& (FaceData[i].interior != 2)
	//		 &&(!((FaceData[i].interior == 1) && (R)))   )
			{
			if (FaceData[i].face3)
				{
				if 	(
						((a == FaceData[i].a) && (b == FaceData[i].b)) ||
						((a == FaceData[i].b) && (b == FaceData[i].c)) ||
						((a == FaceData[i].c) && (b == FaceData[i].a)) 
					)
					//matched found and need to flip
					//flip direction
					{
					FlipFace(i);
					FaceData[i].tagged = 1;
					if (
						(FaceData[i].interior != 2)  && (FaceData[i].interior != 1)
						)
						{
						if ((b!=FaceData[i].a) &&(a!=FaceData[i].b))
							CheckFace(i,FaceData[i].a,FaceData[i].b);

						if ((b!=FaceData[i].b) &&(a!=FaceData[i].c))
							CheckFace(i,FaceData[i].b,FaceData[i].c);

						if ((b!=FaceData[i].c) &&(a!=FaceData[i].a))
							CheckFace(i,FaceData[i].c,FaceData[i].a);
						}
					}
				else if 	(
						((b == FaceData[i].a) && (a == FaceData[i].b)) ||
						((b == FaceData[i].b) && (a == FaceData[i].c)) ||
						((b == FaceData[i].c) && (a == FaceData[i].a)) 
						)
					//matched found and don't need to flip
					{
					FaceData[i].tagged = 1;
					if (
						(FaceData[i].interior != 2)  && (FaceData[i].interior != 1)
						)
						{
						if ((b!=FaceData[i].a) &&(a!=FaceData[i].b))
							CheckFace(i,FaceData[i].a,FaceData[i].b);

						if ((b!=FaceData[i].b) &&(a!=FaceData[i].c))
							CheckFace(i,FaceData[i].b,FaceData[i].c);

						if ((b!=FaceData[i].c) &&(a!=FaceData[i].a))
							CheckFace(i,FaceData[i].c,FaceData[i].a);
						}
					}
				}
			else
				{
				if 	(
						((a == FaceData[i].a) && (b == FaceData[i].b)) ||
						((a == FaceData[i].b) && (b == FaceData[i].c)) ||
						((a == FaceData[i].c) && (b == FaceData[i].d)) ||
						((a == FaceData[i].d) && (b == FaceData[i].a)) 
					)
					//matched found and need to flip
					//flip direction
					{
					FlipFace(i);
					FaceData[i].tagged = 1;
					if (
						(FaceData[i].interior != 2)  && (FaceData[i].interior != 1)
						)
						{
						if ((b!=FaceData[i].a) &&(a!=FaceData[i].b))
							CheckFace(i,FaceData[i].a,FaceData[i].b);
				
						if ((b!=FaceData[i].b) &&(a!=FaceData[i].c))
							CheckFace(i,FaceData[i].b,FaceData[i].c);

						if ((b!=FaceData[i].c) &&(a!=FaceData[i].d))
							CheckFace(i,FaceData[i].c,FaceData[i].d);

						if ((b!=FaceData[i].d) &&(a!=FaceData[i].a))
							CheckFace(i,FaceData[i].d,FaceData[i].a);
						}
					}
				else if 	(
						((b == FaceData[i].a) && (a == FaceData[i].b)) ||
						((b == FaceData[i].b) && (a == FaceData[i].c)) ||
						((b == FaceData[i].c) && (a == FaceData[i].d)) ||
						((b == FaceData[i].d) && (a == FaceData[i].a)) 
					)
					//matched found and don't need to flip
					{
					FaceData[i].tagged = 1;
					if (
						(FaceData[i].interior != 2)  && (FaceData[i].interior != 1)
						)
						{
						if ((b!=FaceData[i].a) &&(a!=FaceData[i].b))
							CheckFace(i,FaceData[i].a,FaceData[i].b);

						if ((b!=FaceData[i].b) &&(a!=FaceData[i].c))
							CheckFace(i,FaceData[i].b,FaceData[i].c);

						if ((b!=FaceData[i].c) &&(a!=FaceData[i].d))
							CheckFace(i,FaceData[i].c,FaceData[i].d);

						if ((b!=FaceData[i].d) &&(a!=FaceData[i].a))
							CheckFace(i,FaceData[i].d,FaceData[i].a);
						}
					}
				}
			}
		}
}

void PatchData::UnifyNormals(int flip)
{
	int i;

	for (i=0;i<FaceData.Count();i++)
		{								   
		FaceData[i].tagged = 0;

		}

	if (FaceData.Count() >0)
		FaceData[0].tagged = 1;

	for (i=0;i<FaceData.Count();i++)
		{
		if (((!FaceData[i].tagged) || (i==0)) && (FaceData[i].interior!=2))
			{
			CheckFace(i,FaceData[i].a,FaceData[i].b);
			CheckFace(i,FaceData[i].b,FaceData[i].c);
			if (FaceData[i].face3)
				{
				CheckFace(i,FaceData[i].c,FaceData[i].a);
				}
				else
				{
				CheckFace(i,FaceData[i].c,FaceData[i].d);
				CheckFace(i,FaceData[i].d,FaceData[i].a);
				}
			}
		}
	if (flip)
		{
		for (i=0;i<FaceData.Count();i++)
			FlipFace(i);
		}
}

void PatchData::CheckData(int face)
{
    int i;
	int NumberConnectedFaces[4];
	int a,b,c,d;
	a = FaceData[face].a;
	b = FaceData[face].b;
	c = FaceData[face].c;
	if (!FaceData[face].face3)
		d = FaceData[face].d;
	NumberConnectedFaces[0]=0;
	NumberConnectedFaces[1]=0;
	NumberConnectedFaces[2]=0;
	NumberConnectedFaces[3]=0;
	for (i=0;i<FaceData.Count();i++)
	{
	if (i!=face)
		{
		if ( (FaceData[i].face3) && (FaceData[face].face3) )
			{
			if (
				//check ab edge
				((a == FaceData[i].a) && (b == FaceData[i].b)) ||
				((a == FaceData[i].b) && (b == FaceData[i].c)) ||
				((a == FaceData[i].c) && (b == FaceData[i].a)) ||
				((b == FaceData[i].a) && (a == FaceData[i].b)) ||
				((b == FaceData[i].b) && (a == FaceData[i].c)) ||
				((b == FaceData[i].c) && (a == FaceData[i].a)) 
				)
					NumberConnectedFaces[0]++;
				//check bc edge
			if (
				((b == FaceData[i].a) && (c == FaceData[i].b)) ||
				((b == FaceData[i].b) && (c == FaceData[i].c)) ||
				((b == FaceData[i].c) && (c == FaceData[i].a)) ||
				((c == FaceData[i].a) && (b == FaceData[i].b)) ||
				((c == FaceData[i].b) && (b == FaceData[i].c)) ||
				((c == FaceData[i].c) && (b == FaceData[i].a)) 
				)
				NumberConnectedFaces[1]++;

				//check ca edge
			if (
				((c == FaceData[i].a) && (a == FaceData[i].b)) ||
				((c == FaceData[i].b) && (a == FaceData[i].c)) ||
				((c == FaceData[i].c) && (a == FaceData[i].a)) ||
				((a == FaceData[i].a) && (c == FaceData[i].b)) ||
				((a == FaceData[i].b) && (c == FaceData[i].c)) ||
				((a == FaceData[i].c) && (c == FaceData[i].a)) 
				)
				NumberConnectedFaces[2]++;
			}
		else if ( (!FaceData[i].face3) && (!FaceData[face].face3) )
			{
			if (
			//check ab
				((a == FaceData[i].a) && (b == FaceData[i].b)) ||
				((a == FaceData[i].b) && (b == FaceData[i].c)) ||
				((a == FaceData[i].c) && (b == FaceData[i].d)) ||
				((a == FaceData[i].d) && (b == FaceData[i].a)) ||
				((b == FaceData[i].a) && (a == FaceData[i].b)) ||
				((b == FaceData[i].b) && (a == FaceData[i].c)) ||
				((b == FaceData[i].c) && (a == FaceData[i].d)) ||
				((b == FaceData[i].d) && (a == FaceData[i].a)) 
				)
				NumberConnectedFaces[0]++;
			//check bc
			if (
				((b == FaceData[i].a) && (c == FaceData[i].b)) ||
				((b == FaceData[i].b) && (c == FaceData[i].c)) ||
				((b == FaceData[i].c) && (c == FaceData[i].d)) ||
				((b == FaceData[i].d) && (c == FaceData[i].a)) ||
				((c == FaceData[i].a) && (b == FaceData[i].b)) ||
				((c == FaceData[i].b) && (b == FaceData[i].c)) ||
				((c == FaceData[i].c) && (b == FaceData[i].d)) ||
				((c == FaceData[i].d) && (b == FaceData[i].a)) 
				)
				NumberConnectedFaces[1]++;
			//check cd
			if (
				((c == FaceData[i].a) && (d == FaceData[i].b)) ||
				((c == FaceData[i].b) && (d == FaceData[i].c)) ||
				((c == FaceData[i].c) && (d == FaceData[i].d)) ||
				((c == FaceData[i].d) && (d == FaceData[i].a)) ||
				((d == FaceData[i].a) && (c == FaceData[i].b)) ||
				((d == FaceData[i].b) && (c == FaceData[i].c)) ||
				((d == FaceData[i].c) && (c == FaceData[i].d)) ||
				((d == FaceData[i].d) && (c == FaceData[i].a)) 
				)
				NumberConnectedFaces[2]++;
			//check da
			if (
				((d == FaceData[i].a) && (a == FaceData[i].b)) ||
				((d == FaceData[i].b) && (a == FaceData[i].c)) ||
				((d == FaceData[i].c) && (a == FaceData[i].d)) ||
				((d == FaceData[i].d) && (a == FaceData[i].a)) ||
				((a == FaceData[i].a) && (d == FaceData[i].b)) ||
				((a == FaceData[i].b) && (d == FaceData[i].c)) ||
				((a == FaceData[i].c) && (d == FaceData[i].d)) ||
				((a == FaceData[i].d) && (d == FaceData[i].a)) 
				)
				NumberConnectedFaces[3]++;
			}
		else if ( (!FaceData[i].face3) && (FaceData[face].face3) )
			{
			if (
			//check ab
				((a == FaceData[i].a) && (b == FaceData[i].b)) ||
				((a == FaceData[i].b) && (b == FaceData[i].c)) ||
				((a == FaceData[i].c) && (b == FaceData[i].d)) ||
				((a == FaceData[i].d) && (b == FaceData[i].a)) ||
				((b == FaceData[i].a) && (a == FaceData[i].b)) ||
				((b == FaceData[i].b) && (a == FaceData[i].c)) ||
				((b == FaceData[i].c) && (a == FaceData[i].d)) ||
				((b == FaceData[i].d) && (a == FaceData[i].a)) )
				NumberConnectedFaces[0]++;
			//check bc
			if (
				((b == FaceData[i].a) && (c == FaceData[i].b)) ||
				((b == FaceData[i].b) && (c == FaceData[i].c)) ||
				((b == FaceData[i].c) && (c == FaceData[i].d)) ||
				((b == FaceData[i].d) && (c == FaceData[i].a)) ||
				((c == FaceData[i].a) && (b == FaceData[i].b)) ||
				((c == FaceData[i].b) && (b == FaceData[i].c)) ||
				((c == FaceData[i].c) && (b == FaceData[i].d)) ||
				((c == FaceData[i].d) && (b == FaceData[i].a)) 
				)
				NumberConnectedFaces[1]++;
			//check ca
			if (
				((c == FaceData[i].a) && (a == FaceData[i].b)) ||
				((c == FaceData[i].b) && (a == FaceData[i].c)) ||
				((c == FaceData[i].c) && (a == FaceData[i].d)) ||
				((c == FaceData[i].d) && (a == FaceData[i].a)) ||
				((a == FaceData[i].a) && (c == FaceData[i].b)) ||
				((a == FaceData[i].b) && (c == FaceData[i].c)) ||
				((a == FaceData[i].c) && (c == FaceData[i].d)) ||
				((a == FaceData[i].d) && (c == FaceData[i].a)) 
				)
				NumberConnectedFaces[2]++;
			}
		else if ( (FaceData[i].face3) && (!FaceData[face].face3) )
			{
			if (
			//check ab edge
				((a == FaceData[i].a) && (b == FaceData[i].b)) ||
				((a == FaceData[i].b) && (b == FaceData[i].c)) ||
				((a == FaceData[i].c) && (b == FaceData[i].a)) ||
				((b == FaceData[i].a) && (a == FaceData[i].b)) ||
				((b == FaceData[i].b) && (a == FaceData[i].c)) ||
				((b == FaceData[i].c) && (a == FaceData[i].a)) 
				)
				NumberConnectedFaces[0]++;
			//check bc edge
			if (
				((b == FaceData[i].a) && (c == FaceData[i].b)) ||
				((b == FaceData[i].b) && (c == FaceData[i].c)) ||
				((b == FaceData[i].c) && (c == FaceData[i].a)) ||
				((c == FaceData[i].a) && (b == FaceData[i].b)) ||
				((c == FaceData[i].b) && (b == FaceData[i].c)) ||
				((c == FaceData[i].c) && (b == FaceData[i].a)) 
				)
				NumberConnectedFaces[1]++;
			//check cd edge
			if (
				((c == FaceData[i].a) && (d == FaceData[i].b)) ||
				((c == FaceData[i].b) && (d == FaceData[i].c)) ||
				((c == FaceData[i].c) && (d == FaceData[i].a)) ||
				((d == FaceData[i].a) && (c == FaceData[i].b)) ||
				((d == FaceData[i].b) && (c == FaceData[i].c)) ||
				((d == FaceData[i].c) && (c == FaceData[i].a)) 
				)
				NumberConnectedFaces[2]++;
			//check da edge
			if (
				((d == FaceData[i].a) && (a == FaceData[i].b)) ||
				((d == FaceData[i].b) && (a == FaceData[i].c)) ||
				((d == FaceData[i].c) && (a == FaceData[i].a)) ||
				((a == FaceData[i].a) && (d == FaceData[i].b)) ||
				((a == FaceData[i].b) && (d == FaceData[i].c)) ||
				((a == FaceData[i].c) && (d == FaceData[i].a)) 
				)
				NumberConnectedFaces[3]++;
			}
		}
	}

	if (FaceData[face].face3)	
		{
		if ( (NumberConnectedFaces[0]>=2) &&(NumberConnectedFaces[1]>=2) &&(NumberConnectedFaces[2]>=2) )
			FaceData[face].interior = 2;
		}
	else
		{
		if ( (NumberConnectedFaces[0]>=2) &&(NumberConnectedFaces[1]>=2) &&(NumberConnectedFaces[2]>=2)&&(NumberConnectedFaces[3]>=2) )
			FaceData[face].interior = 2;
		}
}

void PatchData::MoveVectors(int source,int dest)
{
	int i;

	for (i=0;i<FaceData.Count();i++)
		{

		if (FaceData[i].ab==source)
			{
			FaceData[i].ab=dest;
			i = FaceData.Count();
			}
		else if (FaceData[i].ba==source)
			{
			FaceData[i].ba=dest;
			i = FaceData.Count();
			}
		else if (FaceData[i].bc==source)
			{
			FaceData[i].bc=dest;
			i = FaceData.Count();
			}
		else if (FaceData[i].cb==source)
			{
			FaceData[i].cb=dest;
			i = FaceData.Count();
			}
		else if (FaceData[i].face3)
			{
			if (FaceData[i].ca==source)
				{
				FaceData[i].ca=dest;
				i = FaceData.Count();
				}
			else if (FaceData[i].ac==source)
				{
				FaceData[i].ac=dest;
				i = FaceData.Count();
				}
			}
		else
			{
			if (FaceData[i].cd==source)
				{
				FaceData[i].cd=dest;
				i = FaceData.Count();
				}
			else if (FaceData[i].dc==source)
				{
				FaceData[i].dc=dest;
				i = FaceData.Count();
				}
			else if (FaceData[i].da==source)
				{
				FaceData[i].da=dest;
				i = FaceData.Count();
				}
			else if (FaceData[i].ad==source)
				{
				FaceData[i].ad=dest;
				i = FaceData.Count();
				}
			}
		}
}

void PatchData::RecombineVectors()
{
	int i,j,VecCt = 0,ct =0,source;
	Point3 Vec;

	for (i=0;i<VecList.Count();i++)
		{
		if (VecCount[i]>2)
			{
			Vec = VecList[i];
			VecCt = VecCount[i]-2;
			source = i;
			ct = 1;

			VecCount[i]=2;

			VecList.Append(1,&Vec,1);
			VecCount.Append(1,&ct,1);
			MoveVectors(source,VecList.Count()-1);

			for (j=1;j<VecCt;j++)
				{
				if (VecCount[VecCount.Count()-1]<2)
					{
					VecCount[VecCount.Count()-1]++;
					MoveVectors(source,VecList.Count()-1);
					ct++;
					}
					else
					{
					ct = 1;
					VecList.Append(1,&Vec,1);
					VecCount.Append(1,&ct,1);
					MoveVectors(source,VecList.Count()-1);
					}
				}
			}	
		}
}

void PatchData::RemoveInteriorFaces()
{
	int i,count;//,j,count;
	count = 0;

	Verts33.ZeroCount();

	for (i=0;i<FaceData.Count();i++)
		{					
	//2 = interior
	//3 = cap
		FaceData[i].interior = 0;
		FaceData[i].tagged = 0;
		Verts33.Append(1,&count,1);
		}

	for (i=0;i<FaceData.Count();i++)
		{
		if (FaceData[i].face3)
			{
			count = VertData[FaceData[i].va]->InVerts.Count() + 
				VertData[FaceData[i].va]->OutVerts.Count() + 

				VertData[FaceData[i].vb]->InVerts.Count() + 
				VertData[FaceData[i].vb]->OutVecs.Count() + 

				VertData[FaceData[i].vc]->InVerts.Count() + 
				VertData[FaceData[i].vc]->OutVerts.Count()  ;
			if ((count)>= 11)
				CheckData(i);
			}
		else
			{
			count = VertData[FaceData[i].va]->InVerts.Count() + 
    				VertData[FaceData[i].va]->OutVerts.Count() + 

				VertData[FaceData[i].vb]->InVerts.Count() + 
				VertData[FaceData[i].vb]->OutVerts.Count() + 

				VertData[FaceData[i].vc]->InVerts.Count() + 
				VertData[FaceData[i].vc]->OutVerts.Count() +  

				VertData[FaceData[i].vd]->InVerts.Count() + 
				VertData[FaceData[i].vd]->OutVerts.Count()  ;

			if ((count) >= 13)
				CheckData(i);
			}
		}
	Verts33.ZeroCount();
}

void PatchData::DeleteInteriorFaces(int rinterior, int rcaps)
{
	int i,j;//,count;

	int NumTagged=0;

	//remove interior faces
	if (rinterior)
		{
		for (i=0;i<FaceData.Count();i++)
			{
			if (FaceData[i].interior==2) NumTagged++;
			}

		for (j=0;j<NumTagged;j++)
			{
			for (i=0;i<FaceData.Count();i++)
				{
				if (FaceData[i].interior==2) 
					{
					//reduce vec count
					VecCount[FaceData[i].ab]--;
					VecCount[FaceData[i].ba]--;
					VecCount[FaceData[i].bc]--;
					VecCount[FaceData[i].cb]--;
					if (FaceData[i].face3)
						{
						VecCount[FaceData[i].ca]--;
						VecCount[FaceData[i].ac]--;
						}
						else
						{
						VecCount[FaceData[i].cd]--;
						VecCount[FaceData[i].dc]--;
						VecCount[FaceData[i].da]--;
						VecCount[FaceData[i].ad]--;
						}
					FaceData.Delete(i,1);
					i = FaceData.Count()+1;
					}
				}

			}
		}
}

void PatchData::NukeDegenerateHookPatches(BitArray hooksBA)
{
	int i;
	for (i=0;i<FaceData.Count();i++)
		{
		//is a degenerate hook patch
		if (FaceData[i].face3)
			{

			int verts[4];
			int pcount = 3;
			verts[0] = FaceData[i].a;
			verts[1] = FaceData[i].b;
			verts[2] = FaceData[i].c;

			//check does thpatch have hook edge
			int hookCount = 0; 
			for (int j = 0; j < pcount; j++)
				{
				if (hooksBA[verts[j]]) hookCount++;
				}
			if (hookCount >= 3) 
				{
				FaceData.Delete(i,1);
				i--;
				}
			}
		}

	for (i=0;i<FaceData.Count();i++)
		{
		//is a degenerate hook patch
		int verts[4];
		int ct =3;
		verts[0] = FaceData[i].a;
		verts[1] = FaceData[i].b;
		verts[2] = FaceData[i].c;
		if (!FaceData[i].face3)
			{
			verts[3] = FaceData[i].d;
			ct = 4;
			}
		int ht = 0;
		for (int j = 0; j <(ct-1) ;j++)
			{
			for (int k = j+1; k <ct ;k++)
				{
				if (verts[j] == verts[k]) ht++;
				}

			}

		if (ht>0)
			{
			FaceData.Delete(i,1);
			i--;
			}
		}
}


/*
// Future use (Not used now)
void EditPatchMod::SetMeshAdaptive(BOOL sw) {
	meshAdaptive = sw;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		patch->SetAdaptive(sw);
		patchData->meshAdaptive = sw;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}
*/



void EditPatchMod::SetViewTess(TessApprox &tess) {
	viewTess = tess;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetViewTess(tess);
		patchData->viewTess = tess;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetProdTess(TessApprox &tess) {
	prodTess = tess;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetProdTess(tess);
		patchData->prodTess = tess;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetDispTess(TessApprox &tess) {
	dispTess = tess;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetDispTess(tess);
		patchData->dispTess = tess;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetViewTessNormals(BOOL use) {
	mViewTessNormals = use;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetViewTessNormals(use);
		patchData->mViewTessNormals = use;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetProdTessNormals(BOOL use) {
	mProdTessNormals = use;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetProdTessNormals(use);
		patchData->mProdTessNormals = use;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetViewTessWeld(BOOL weld) {
	mViewTessWeld = weld;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetViewTessWeld(weld);
		patchData->mViewTessWeld = weld;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::SetProdTessWeld(BOOL weld) {
	mProdTessWeld = weld;
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;

		patch->SetProdTessWeld(weld);
		patchData->mProdTessWeld = weld;
		if ( patchData->tempData ) {
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
			}
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


void EditPatchMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	ModContextList mcList;
	INodeTab nodes;
	int old = selLevel;

	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);

	selLevel = level;
//3-10-99 watje
	if (level != EP_PATCH && level != EP_ELEMENT)
		{
		if (ip->GetCommandMode()==bevelMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==extrudeMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==normalFlipMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==createPatchMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (inBevel)
			{
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_OUTLINESPINNER));
			if (spin) {
				HWND hWnd = spin->GetHwnd();
				SendMessage(hWnd,WM_LBUTTONUP,0,0);
				ReleaseISpinner(spin);
				}

			}
		if (inExtrude)
			{
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_EXTRUDESPINNER));
			if (spin) {
				HWND hWnd = spin->GetHwnd();
				SendMessage(hWnd,WM_LBUTTONUP,0,0);
				ReleaseISpinner(spin);
				}
			}
		}	
	if (level != EP_VERTEX)
		{
		if (ip->GetCommandMode()==bindMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==createVertMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==vertWeldMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		}
	if (level != EP_VERTEX && level != EP_HANDLE)
		{
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		if (ip->GetCommandMode()==copyTangentMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode()==pasteTangentMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		}


	switch ( level ) {
		case EP_OBJECT:
			// Not imp.
			break;

		case EP_PATCH:
		case EP_ELEMENT:
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;

		case EP_EDGE:
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;

		case EP_VERTEX:
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;
		}

	if ( selLevel != old ) {
		SetSubobjectLevel(level);

		// Modify the caches to reflect the new sel level.
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if ( !patchData ) continue;		
		
			if ( patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()) ) {
				
				PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
				if(patch) {
					if(selLevel == EP_VERTEX)
						patch->dispFlags = DISP_VERTS;
					else
						patch->dispFlags = 0;
					if(displayLattice)
						patch->SetDispFlag(DISP_LATTICE);
					patch->SetDispFlag(patchLevelDispFlags[selLevel]);
					patch->selLevel = patchLevel[selLevel];
					}
				}
			}		

		NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY,REFMSG_CHANGE);
		ip->PipeSelLevelChanged();
		// Update selection UI display, named sel
		SelectionChanged();
		}
	
	nodes.DisposeTemporary();
	}


int EditPatchMod::SubObjectIndex(HitRecord *hitRec)
	{	
	EditPatchData *patchData = (EditPatchData*)hitRec->modContext->localData;
	if ( !patchData ) return 0;
	if ( !ip ) return 0;
	TimeValue t = ip->GetTime();
	PatchHitData *hit = (PatchHitData *)(hitRec->hitData);
	switch ( GetSubobjectType() ) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE: {
			int hitIndex = hit->index;
			return hitIndex;
			}
		case EP_VERTEX: {
			if(hit->type != PATCH_HIT_VERTEX)
				return 0;
			int hitIndex = hit->index;
			return hitIndex;
			}
		case EP_EDGE: {
			int hitIndex = hit->index;
			return hitIndex;
			}
		case EP_PATCH: {
			int hitIndex = hit->index;
			return hitIndex;
			}
		default:
			return 0;
		}
	}

void EditPatchMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	if ( mc->localData ) {
		EditPatchData *patchData = (EditPatchData*)mc->localData;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		// Watch out -- The system can call us even if we didn't get a valid patch object
		if(!patch)
			return;

		switch ( GetSubobjectType() ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
 				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				Box3 box;
				BitArray &sel = patch->vecSel;
				int count = sel.GetSize();
				for ( int i = 0; i < count; i++ ) {
					if ( sel[i] )
						box += patch->vecs[i].p;
					}
				tm.SetTrans(otm * box.Center());
				cb->TM(tm, 0);
				break;
				}
			case EP_VERTEX: {
				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				BitArray sel = patch->VertexTempSel();
				int count = sel.GetSize();
				for(int i = 0; i < count; ++i) {
					if(sel[i]) {
						tm.SetTrans(patch->verts[i].p * otm);
						cb->TM(tm, i);
						}
					}
				break;
				}
			case EP_EDGE:
			case EP_PATCH: {
 				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				Box3 box;
				BitArray sel = patch->VertexTempSel();
				int count = sel.GetSize();
				for ( int i = 0; i < count; i++ ) {
					if ( sel[i] )
						box += patch->verts[i].p;
					}
				tm.SetTrans(otm * box.Center());
				cb->TM(tm, 0);
				break;
				}
			}
		}
	}

void EditPatchMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 tm = node->GetObjectTM(t,&valid);	
	
	assert(ip);
	if ( mc->localData ) {	
		EditPatchData *patchData = (EditPatchData*)mc->localData;		
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		// Watch out -- The system can call us even if we didn't get a valid patch object
		if(!patch)
			return;

		switch ( GetSubobjectType() ) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case EP_HANDLE: {
				Point3 cent(0,0,0);
				int ct=0;
				BitArray &sel = patch->vecSel;
				int vecs = patch->numVecs;
				for ( int i = 0; i < vecs; i++ ) {
					if ( sel[i] ) {
						cent += patch->vecs[i].p;
						ct++;
						}
					}
				if (ct) {
					cent /= float(ct);
					cb->Center(cent*tm,0);
					}
				break;
				}
			case EP_VERTEX:
			case EP_EDGE:
			case EP_PATCH: {
				// CAL-06/23/03: set only one center to be at the average of the selections.
				Point3 cent(0,0,0);
				int ct=0;
				BitArray sel = patch->VertexTempSel();
				int verts = patch->numVerts;
				for ( int i = 0; i < verts; i++ ) {
					if ( sel[i] ) {
						cent += patch->verts[i].p;
						ct++;
						}
					}
				if (ct) {
					cent /= float(ct);
					cb->Center(cent*tm,0);
					}
				break;
				}
			default:
				cb->Center(tm.GetTrans(), 0);
				break;
			}		
		}
	}

BOOL EditPatchMod::DependOnTopology(ModContext &mc)
	{
	EditPatchData *patchData = (EditPatchData*)mc.localData;
	if (patchData) {
		if (patchData->GetFlag(EPD_HASDATA)) {
			return TRUE;
			}
		}
	return FALSE;
	}

void EditPatchMod::DeletePatchDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;				
		if ( patchData->tempData ) {
			delete patchData->tempData;
			}
		patchData->tempData = NULL;
		}
	nodes.DisposeTemporary();
	}


void EditPatchMod::CreatePatchDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;				
		if ( !patchData->tempData ) {
			patchData->tempData = new EPTempData(this,patchData);
			}		
		}
	nodes.DisposeTemporary();
	}

//--------------------------------------------------------------

int EditPatchMod::RememberPatchThere(HWND hWnd, IPoint2 m) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	// Initialize so there isn't any remembered patch
	rememberedPatch = NULL;

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	// See if we're over a patch
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubPatchHitList hitList;

	int result = 0;
	int patchType = -1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		patch->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_PATCH_PATCHES | MaybeIgnoreBackfacing()/* | HIT_ABORTONHIT*/, hitList );
		PatchSubHitRec *hit = hitList.First();
		if(hit) {
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while(hit) {
				if(patch->patchSel[hit->index]) {
					// If in element mode, verify that all the other patches in this element are also selected
					if(selLevel==EP_ELEMENT) {
						BitArray el = patch->GetElement(hit->index);
						if(!(patch->patchSel == el))
							goto next_hit;	// Not all selected; bypass!
						}
					if(patch->SelPatchesSameType()) {
						rememberedPatch = NULL;
						rememberedData = patch->patches[hit->index].flags & (~PATCH_INTERIOR_MASK);
						goto onselect;		// CAL-05/29/03: find the patch type of current selection
						}
					// Selected patches not all the same type!
					rememberedPatch = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
					}
next_hit:
				hit = hit->Next();
				}
			if(ip->SelectionFrozen())
				goto onselect;		// CAL-05/29/03: find the patch type of current selection
			hit = hitList.First();
			theHold.Begin();
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchSelRestore(patchData,this,patch));
			// If in element mode, select all the element's patches
			if(selLevel==EP_ELEMENT) {
				patch->patchSel = patch->GetElement(hit->index);
				rememberedPatch = NULL;
				if(patch->SelPatchesSameType())
					rememberedData = patch->patches[hit->index].flags & (~PATCH_INTERIOR_MASK);
				else
					rememberedData = -1;	// Not all the same!
				}	
			else {
				// Not element mode, select just this patch
				patch->patchSel.ClearAll();
				patch->patchSel.Set(hit->index);
				rememberedPatch = patch;
				rememberedIndex = hit->index;
				rememberedData = patch->patches[rememberedIndex].flags & (~PATCH_INTERIOR_MASK);
				}
			patchData->UpdateChanges(patch, FALSE);
			theHold.Accept(GetString(IDS_DS_SELECT));
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			PatchSelChanged();
			goto finish;
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}

onselect:
	// CAL-05/29/03: No hit on patch. Find the patch type of the current selection. (FID #830)
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	for ( i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		// find the first selected patch
		for(int index = 0; index < patch->numPatches; index++)
			if (patch->patchSel[index]) break;
		// find the patch type of the selected patches
		if (index < patch->numPatches) {	// some patches are selected
			result = 1;
			rememberedPatch = NULL;
			if(patch->SelPatchesSameType()) {
				rememberedData = patch->patches[index].flags & (~PATCH_INTERIOR_MASK);
				if (patchType < 0) {
					patchType = rememberedData;
				} else if (patchType != rememberedData) {
					rememberedData = -1;
					goto finish;
				}
			} else {
				rememberedData = -1;	// Not all the same!
				goto finish;
			}
		}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}
	
finish:
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	return result;
	}

void EditPatchMod::ChangeRememberedPatch(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		if(patch == rememberedPatch) {
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);

			theHold.Begin();
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"ChangeRememberedPatch"));
			// Call the patch type change function
			ChangePatchType(patch, rememberedIndex, type);
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
			ClearPatchDataFlag(mcList,EPD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
 			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}

void EditPatchMod::ChangeSelPatches(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"ChangeSelPatches"));
			// Call the vertex type change function
			ChangePatchType(patch, -1, type);
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


void EditPatchMod::SetRememberedPatchType(int type) {
	if(rememberedPatch)
		ChangeRememberedPatch(type);
	else
		ChangeSelPatches(type);
	}

//--------------------------------------------------------------

int EditPatchMod::RememberVertThere(HWND hWnd, IPoint2 m) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	// Initialize so there isn't any remembered patch
	rememberedPatch = NULL;

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	// See if we're over a vertex
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubPatchHitList hitList;

	int result = 0;
	int knotType = -1;
	
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		patch->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_PATCH_VERTS | MaybeIgnoreBackfacing()/* | HIT_ABORTONHIT*/, hitList );
		PatchSubHitRec *hit = hitList.First();
		if(hit) {
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while(hit) {
				if(patch->vertSel[hit->index]) {
					if(patch->SelVertsSameType()) {
						rememberedPatch = NULL;
						rememberedData = patch->verts[hit->index].flags & (~PVERT_TYPE_MASK);
						goto onselect;		// CAL-04/23/03: find the vertex type of current selection
						}
					// Selected verts not all the same type!
					rememberedPatch = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
					}
				hit = hit->Next();
				}
			if(ip->SelectionFrozen())
				goto onselect;		// CAL-04/23/03: find the vertex type of current selection
			// Select just this vertex
			hit = hitList.First();
			theHold.Begin();
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchSelRestore(patchData,this,patch));
			patch->vertSel.ClearAll();
			patch->vertSel.Set(hit->index);
			patchData->UpdateChanges(patch, FALSE);
			theHold.Accept(GetString(IDS_DS_SELECT));
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			PatchSelChanged();

			rememberedPatch = patch;
			rememberedIndex = hit->index;
			rememberedData = patch->verts[rememberedIndex].flags & (~PVERT_TYPE_MASK);
			goto finish;
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
onselect:
	// CAL-04/23/03: No hit on vertex or handle. Find the vertex type of the current selection. (FID #830)
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	for ( i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		// find the first selected vertex
		for(int index = 0; index < patch->numVerts; index++)
			if (patch->vertSel[index]) break;
		// find the vertex type of the selected vertices
		if (index < patch->numVerts) {	// some vertices are selected
			result = 1;
			rememberedPatch = NULL;
			if(patch->SelVertsSameType()) {
				rememberedData = patch->verts[index].flags & (~PVERT_TYPE_MASK);
				if (knotType < 0) {
					knotType = rememberedData;
				} else if (knotType != rememberedData) {
					rememberedData = -1;
					goto finish;
				}
			} else {
				rememberedData = -1;	// Not all the same!
				goto finish;
			}
		}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}

finish:
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	return result;
	}

void EditPatchMod::ChangeRememberedVert(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		if(patch == rememberedPatch) {
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);

			theHold.Begin();
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"ChangeRememberedVert"));
			// Call the vertex type change function
			patch->ChangeVertType(rememberedIndex, type);
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_VERTCHANGE));
			ClearPatchDataFlag(mcList,EPD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
 			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}

void EditPatchMod::ChangeSelVerts(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->vertSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"ChangeSelVerts"));
			// Call the vertex type change function
			patch->ChangeVertType(-1, type);
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


void EditPatchMod::SetRememberedVertType(int type) {
	if(rememberedPatch)
		ChangeRememberedVert(type);
	else
		ChangeSelVerts(type);
	}

//--------------------------------------------------------------
int EditPatchMod::HitTest(TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
	{
	Interval valid;
	int savedLimits,res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,pickBoxSize,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if ( mc->localData ) {		
		EditPatchData *patchData = (EditPatchData*)mc->localData;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			return FALSE;

		SubPatchHitList hitList;
		PatchSubHitRec *rec;
		BOOL anyHits = FALSE;
		int level = GetSubobjectLevel();
		// If we're in "by vertex" mode, handle it
		if(type == HITTYPE_POINT && !patchHitOverride && (level == EP_HANDLE || level == EP_EDGE || level == EP_PATCH) && byVertex) {
			res = patch->SubObjectHitTest( gw, gw->getMaterial(), &hr,
				flags|/*patchHitLevel[EP_VERTEX]*/SUBHIT_PATCH_VERTS| MaybeIgnoreBackfacing(), hitList );	// CAL-06/16/03: hit test only on verts but not vecs. (Defect #469111)
			
			rec = hitList.First();
			if(rec) {
				vpt->LogHit(inode,mc,rec->dist,123456,new PatchHitData(rec->patch, rec->index, rec->type));
				anyHits = TRUE;
				}
			}
		if(!anyHits) {
			res = patch->SubObjectHitTest( gw, gw->getMaterial(), &hr,
				flags|((patchHitOverride) ? patchHitLevel[patchHitOverride] : patchHitLevel[selLevel]) | MaybeIgnoreBackfacing(), hitList );
	
			rec = hitList.First();
			while( rec ) {
				vpt->LogHit(inode,mc,rec->dist,123456,new PatchHitData(rec->patch, rec->index, rec->type));
				rec = rec->Next();
				}
			}
		}

	gw->setRndLimits(savedLimits);	
	return res;
	}

int EditPatchMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {	
	if (!ip || editMod != this || selLevel==EP_OBJECT) return 0;
	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	int savedLimits = gw->getRndLimits();
	gw->setRndLimits ((savedLimits & ~GW_ILLUM) | GW_ALL_EDGES);
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	gw->setTransform(tm);

	if (ip->GetShowEndResult() && mc->localData) {
		tm = inode->GetObjectTM(t);
		gw->setTransform(tm);
		// We need to draw a "gizmo" version of the patch:
		EditPatchData *patchData = (EditPatchData*)mc->localData;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		patch->renderGizmo(gw);
		}
	gw->setRndLimits(savedLimits);
	return 0;	
	}

void EditPatchMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip) return;
	box.Init();
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	if (ip->GetShowEndResult() && mc->localData && selLevel!=EP_OBJECT) {
		// We need to draw a "gizmo" version of the patch:
		Matrix3 tm = inode->GetObjectTM(t);
		EditPatchData *patchData = (EditPatchData*)mc->localData;
		if (patchData) {
			PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
			box = patch->getBoundingBox();
			box = box * tm;
			}
		}
	}

void EditPatchMod::ShowEndResultChanged (BOOL showEndResult) {
	if (!ip || editMod != this) return;
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

//---------------------------------------------------------------------
// UI stuff

void EditPatchMod::RecordTopologyTags() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		patch->RecordTopologyTags();
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}

class ChangeNamedSetRestore : public RestoreObj {
	public:
		BitArray oldset,newset;
		int index;
		GenericNamedSelSetList *setList;

		ChangeNamedSetRestore(GenericNamedSelSetList *sl,int ix,BitArray *o) {
			setList = sl; index = ix; oldset = *o;
			}   		
		void Restore(int isUndo) {
			newset = *(setList->sets[index]);
			*(setList->sets[index]) = oldset;
			}
		void Redo() {
			*(setList->sets[index]) = newset;
			}
				
		TSTR Description() {return TSTR(_T("Change Named Sel Set"));}
	};

// Selection set, misc fixup utility function
// This depends on PatchMesh::RecordTopologyTags being called prior to the topo changes
void EditPatchMod::ResolveTopoChanges() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		// vector selections
		for(int set = 0; set < patchData->hselSet.Count(); ++set) {
			BitArray *oldHS = &patchData->hselSet[set];
			BitArray newHS;
			newHS.SetSize(patch->numVecs);
			for(int vec = 0; vec < patch->numVecs; ++vec) {
				// Get the vector's previous location, then copy that selection into the new set
				int tag = patch->vecs[vec].aux1;
				if(tag >= 0)
					newHS.Set(vec, (*oldHS)[tag]);
				else
					newHS.Clear(vec);
				}
			if(theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->hselSet, set, oldHS));
			patchData->hselSet[set] = newHS;
			}
		// First, the vertex selections
		for(set = 0; set < patchData->vselSet.Count(); ++set) {
			BitArray *oldVS = &patchData->vselSet[set];
			BitArray newVS;
			newVS.SetSize(patch->numVerts);
			for(int vert = 0; vert < patch->numVerts; ++vert) {
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->verts[vert].aux1;
				if(tag >= 0)
					newVS.Set(vert, (*oldVS)[tag]);
				else
					newVS.Clear(vert);
				}
			if(theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->vselSet, set, oldVS));
			patchData->vselSet[set] = newVS;
			}
		// Now the edge selections
		for(set = 0; set < patchData->eselSet.Count(); ++set) {
			BitArray *oldES = &patchData->eselSet[set];
			BitArray newES;
			newES.SetSize(patch->numEdges);
			for(int edge = 0; edge < patch->numEdges; ++edge) {
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->edges[edge].aux1;
				if(tag >= 0)
					newES.Set(edge, (*oldES)[tag]);
				else
					newES.Clear(edge);
				}
			if(theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->eselSet, set, oldES));
			patchData->eselSet[set] = newES;
			}
		// Now the patch selections
		for(set = 0; set < patchData->pselSet.Count(); ++set) {
			BitArray *oldPS = &patchData->pselSet[set];
			BitArray newPS;
			newPS.SetSize(patch->numPatches);
			for(int p = 0; p < patch->numPatches; ++p) {
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->patches[p].aux1;
				if(tag >= 0)
					newPS.Set(p, (*oldPS)[tag]);
				else
					newPS.Clear(p);
				}
			if(theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->pselSet, set, oldPS));
			patchData->pselSet[set] = newPS;
			}

//watje 4-16-99
		patch->HookFixTopology();
//watje 8-23-99 fiexs collapse crash
		patchData->finalPatch.hooks = patch->hooks;

		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}

class EPModContextEnumProc : public ModContextEnumProc {
	float f;
	public:
		EPModContextEnumProc(float f) { this->f = f; }
		BOOL proc(ModContext *mc);  // Return FALSE to stop, TRUE to continue.
	};

BOOL EPModContextEnumProc::proc(ModContext *mc) {
	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( patchData )		
		patchData->RescaleWorldUnits(f);
	return TRUE;
	}

// World scaling
void EditPatchMod::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	
	// rescale all our references
	for (int i=0; i<NumRefs(); i++) {
		ReferenceMaker *srm = GetReference(i);
		if (srm) 
			srm->RescaleWorldUnits(f);
		}
	
	// Now rescale stuff inside our data structures
	EPModContextEnumProc proc(f);
	EnumModContexts(&proc);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

int EditPatchMod::GetSubobjectLevel()
	{
	return selLevel;
	}

int EditPatchMod::GetSubobjectType()
	{
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	if (selLevel==EP_HANDLE) return EP_HANDLE;
	if (selLevel==EP_VERTEX) return EP_VERTEX;
	if (selLevel==EP_EDGE) return EP_EDGE;
	if (selLevel==EP_PATCH || selLevel==EP_ELEMENT) return EP_PATCH;
	return EP_OBJECT;
	}

void EditPatchMod::SetSubobjectLevel(int level)
	{
	selLevel = level;

	UpdateVertexDists();
	UpdateEdgeDists();
	UpdateVertexWeights();

	if(hSelectPanel)
		RefreshSelType();
	// Setup named selection sets	
	if (ip)
		SetupNamedSelDropDown();
	}

static int butIDs[] = { 0, EP_VERTEX, EP_EDGE, EP_PATCH, EP_ELEMENT, EP_HANDLE };

void EditPatchMod::RefreshSelType () {
	if(!hSelectPanel)
		return;

	int isSoftSelOpen = FALSE;
	if(hOpsPanel) {
		// Set up or remove the surface properties rollup if needed
		if(hSurfPanel) {
			rsSurf = IsRollupPanelOpen (hSurfPanel);
			ip->DeleteRollupPage(hSurfPanel);
			hSurfPanel = NULL;
		
		}
		switch ( GetSubobjectType() ) {
		case EP_OBJECT:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF_OBJ),
				PatchObjSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
			break;

		case EP_PATCH:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF),
				PatchSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
				mtlref->hwnd = hSurfPanel;      
				noderef->hwnd = hSurfPanel;        
			break;

		case EP_EDGE:
			hSurfPanel = NULL;
			break;

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			hSurfPanel = NULL;
			break;

		case EP_VERTEX:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF_VERT),
				PatchVertSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
			break;

		default:
			hSurfPanel = NULL;
			break;
		}

		SetSurfDlgEnables();
	}

	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hSelectPanel,IDC_SELTYPE));
	ICustButton *but;
	for (int i=1; i<EP_LEVELS; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck (GetSubobjectLevel()==i);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar(iToolbar);
	SetSelDlgEnables();
	SetSoftSelDlgEnables();
	SetOpsDlgEnables();
	UpdateSelectDisplay();
}

void EditPatchMod::SelectionChanged() {
	if (hSelectPanel) {
		UpdateSelectDisplay();
		InvalidateRect(hSelectPanel,NULL,FALSE);
		}
	// Now see if the selection set matches one of the named selections!
	if(ip && (GetSubobjectType() != EP_OBJECT))
		MaybeDisplayNamedSelSetName(ip, this);
	}

void EditPatchMod::InvalidateSurfaceUI() {
	if(hSurfPanel && (GetSubobjectType() == EP_PATCH || GetSubobjectType() == EP_VERTEX)) {
		InvalidateRect (hSurfPanel, NULL, FALSE);
		patchUIValid = FALSE;
		}
	}

BitArray *EditPatchMod::GetLevelSelectionSet(PatchMesh *patch) {
	switch(GetSubobjectType()) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			return &patch->vecSel;

		case EP_VERTEX:
			return &patch->vertSel;

		case EP_PATCH:
			return &patch->patchSel;

		case EP_EDGE:
			return &patch->edgeSel;
		}
	assert(0);
	return NULL;
	}

void EditPatchMod::UpdateSelectDisplay() {	
	TSTR buf;
	int num, j;

	if (!hSelectPanel) return;

	ModContextList mcList;
	INodeTab nodes;
	if ( !ip )
		return;
	ip->GetModContexts(mcList,nodes);

	switch (GetSubobjectType()) {
		case EP_OBJECT:
			buf.printf (GetString (IDS_TH_OBJECT_SEL));
			break;

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE: {
			num = 0;
			PatchMesh *thePatch = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;		
			
				if ( patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()) ) {
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
					if(!patch) continue;
					int thisNum = patch->vecSel.NumberSet();
					if(thisNum) {
						num += thisNum;
						thePatch = patch;
						}
					}
				}
			if (num==1) {
				for (j=0; j<thePatch->vecSel.GetSize(); j++)
					if (thePatch->vecSel[j]) break;
				buf.printf (GetString(IDS_TH_NUMHANDLESEL), j+1);
				}
			else
				buf.printf (GetString(IDS_TH_NUMHANDLESELP), num);
			}
			break;

		case EP_VERTEX: {
			num = 0;
			PatchMesh *thePatch = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;		
			
				if ( patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()) ) {
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
					if(!patch) continue;
					int thisNum = patch->vertSel.NumberSet();
					if(thisNum) {
						num += thisNum;
						thePatch = patch;
						}
					}
				}
			if (num==1) {
				for (j=0; j<thePatch->vertSel.GetSize(); j++)
					if (thePatch->vertSel[j]) break;
				buf.printf (GetString(IDS_TH_NUMVERTSEL), j+1);
				}
			else
				buf.printf (GetString(IDS_TH_NUMVERTSELP), num);
			}
			break;

		case EP_PATCH: {
			num = 0;
			PatchMesh *thePatch = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;		
			
				if ( patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()) ) {
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
					if(!patch) continue;
					int thisNum = patch->patchSel.NumberSet();
					if(thisNum) {
						num += thisNum;
						thePatch = patch;
						}
					}
				}
			if (num==1) {
				for (j=0; j<thePatch->patchSel.GetSize(); j++)
					if (thePatch->patchSel[j]) break;
				buf.printf (GetString(IDS_TH_NUMPATCHSEL), j+1);
				}
			else
				buf.printf(GetString(IDS_TH_NUMPATCHSELP),num);
			}
			break;

		case EP_EDGE: {
			num = 0;
			PatchMesh *thePatch = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
				if ( !patchData ) continue;		
			
				if ( patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()) ) {
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
					if(!patch) continue;
					int thisNum = patch->edgeSel.NumberSet();
					if(thisNum) {
						num += thisNum;
						thePatch = patch;
						}
					}
				}
			if (num==1) {
				for (j=0; j<thePatch->edgeSel.GetSize(); j++)
					if (thePatch->edgeSel[j]) break;
				buf.printf (GetString(IDS_TH_NUMEDGESEL), j+1);
				}
			else
				buf.printf(GetString(IDS_TH_NUMEDGESELP),num);
			}
			break;
		}

	nodes.DisposeTemporary();
	SetDlgItemText(hSelectPanel, IDC_NUMSEL_LABEL, buf);
	}

static BOOL oldShowEnd;

void EditPatchMod::BeginEditParams( IObjParam *ip, ULONG flags, Animatable *prev )
	{
	this->ip = ip;
	editMod = this;

	patchUIValid = FALSE;
	CreatePatchDataTempData();

	hSelectPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SELECT),
		PatchSelectDlgProc, GetString(IDS_TH_SELECTION), (LPARAM)this, rsSel ? 0 : APPENDROLL_CLOSED);
	hSoftSelPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_SOFT_SELECTION),
		SoftSelectDlgProc, GetString (IDS_SOFT_SELECTION), (LPARAM) this, rsSoftSel ? 0 : APPENDROLL_CLOSED);
	hOpsPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_OPS),
		PatchOpsDlgProc, GetString (IDS_TH_GEOMETRY), (LPARAM) this, rsOps ? 0 : APPENDROLL_CLOSED);

	switch ( GetSubobjectType() ) {
		case EP_OBJECT:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF_OBJ),
				PatchObjSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
			break;

		case EP_PATCH:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF),
				PatchSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
			break;

		case EP_EDGE:
			hSurfPanel = NULL;
			break;

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			hSurfPanel = NULL;
			break;

		case EP_VERTEX:
			hSurfPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF_VERT),
				PatchVertSurfDlgProc, GetString (IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
			break;

		default:
			hSurfPanel = NULL;
			break;
		}

	// Create sub object editing modes.
	moveMode        = new MoveModBoxCMode(this,ip);
	rotMode         = new RotateModBoxCMode(this,ip);
	uscaleMode      = new UScaleModBoxCMode(this,ip);
	nuscaleMode     = new NUScaleModBoxCMode(this,ip);
	squashMode      = new SquashModBoxCMode(this,ip);
	selectMode      = new SelectModBoxCMode(this,ip);
	extrudeMode    = new EPM_ExtrudeCMode(this,ip);
	bevelMode    = new EPM_BevelCMode(this,ip);
	bindMode    = new EPM_BindCMode(this,ip);
	normalFlipMode  = new EPM_NormalFlipCMode(this,ip);
	createVertMode	= new EPM_CreateVertCMode(this,ip);
	createPatchMode = new EPM_CreatePatchCMode(this,ip);
	vertWeldMode	= new EPM_VertWeldCMode(this,ip);
	// CAL-06/02/03: copy/paste tangent. (FID #827)
	copyTangentMode  = new EPM_CopyTangentCMode(this,ip);
	pasteTangentMode  = new EPM_PasteTangentCMode(this,ip);

	// Create reference for MultiMtl name support         
	noderef = new SingleRefMakerPatchMNode;         //set ref to node
	INode* objNode = GetNode(this);
	if (objNode) {
		noderef->ep = this;
		noderef->SetRef(objNode);                 
	}

	mtlref = new SingleRefMakerPatchMMtl;       //set ref for mtl
	mtlref->ep = this;
	if (objNode) {
		Mtl* nodeMtl = objNode->GetMtl();
		mtlref->SetRef(nodeMtl);                        
	}

	// Add our sub object type
	// TSTR type1( GetString(IDS_TH_VERTEX) );
	// TSTR type2( GetString(IDS_TH_EDGE) );
	// TSTR type3( GetString(IDS_TH_PATCH) );
	// TSTR type4( GetString(IDS_TH_ELEMENT) );
	// const TCHAR *ptype[] = { type1, type2, type3, type4 };
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes( ptype, 4 );

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);
	
	// Setup named selection sets	
	SetupNamedSelDropDown();

	// Update selection UI display
	SelectionChanged();

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult();
	ip->SetShowEndResult (GetFlag (EP_DISP_RESULT));

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
	}
		
void EditPatchMod::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next )
	{
	if (hSelectPanel) {
		rsSel = IsRollupPanelOpen (hSelectPanel);
		ip->DeleteRollupPage(hSelectPanel);
		hSelectPanel = NULL;
		}
	if (hOpsPanel) {
		rsOps = IsRollupPanelOpen (hOpsPanel);
		ip->DeleteRollupPage(hOpsPanel);
		hOpsPanel = NULL;
		}
	if (hSurfPanel) {
		rsSurf = IsRollupPanelOpen (hSurfPanel);
		ip->DeleteRollupPage(hSurfPanel);
		hSurfPanel = NULL;
		}

	if (hSoftSelPanel) {
		rsSoftSel = IsRollupPanelOpen (hSoftSelPanel);
		ip->DeleteRollupPage(hSoftSelPanel);
		hSoftSelPanel = NULL;
		}

	CancelEditPatchModes(ip);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	
	if (noderef) delete noderef;                   
	noderef = NULL;                         
	if (mtlref) delete mtlref;             
	mtlref = NULL;                         

	DeletePatchDataTempData();
	this->ip = NULL;
	editMod = NULL;

//	if ( ip->GetCommandMode()->ID() == CID_EP_EXTRUDE ) ip->SetStdCommandMode( CID_OBJMOVE );
//	if ( ip->GetCommandMode()->ID() == CID_EP_BEVEL ) ip->SetStdCommandMode( CID_OBJMOVE );

	
	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);
	ip->DeleteMode(selectMode);
	ip->DeleteMode(extrudeMode);
	ip->DeleteMode(bevelMode);
	ip->DeleteMode(bindMode);
	ip->DeleteMode(normalFlipMode);
	ip->DeleteMode(createVertMode);
	ip->DeleteMode(createPatchMode);
	ip->DeleteMode(vertWeldMode);
	// CAL-06/02/03: copy/paste tangent. (FID #827)
	ip->DeleteMode(copyTangentMode);
	ip->DeleteMode(pasteTangentMode);

	if ( moveMode ) delete moveMode;
	moveMode = NULL;
	if ( rotMode ) delete rotMode;
	rotMode = NULL;
	if ( uscaleMode ) delete uscaleMode;
	uscaleMode = NULL;
	if ( nuscaleMode ) delete nuscaleMode;
	nuscaleMode = NULL;
	if ( squashMode ) delete squashMode;
	squashMode = NULL;
	if ( selectMode ) delete selectMode;
	selectMode = NULL;
	if( extrudeMode ) delete extrudeMode;
	extrudeMode = NULL;
	if( bevelMode ) delete bevelMode;
	bevelMode = NULL;
	if( bindMode ) delete bindMode;
	bindMode = NULL;
	if( normalFlipMode ) delete normalFlipMode;
	normalFlipMode = NULL;
	if( createVertMode ) delete createVertMode;
	createVertMode = NULL;
	if( createPatchMode ) delete createPatchMode;
	createPatchMode = NULL;
	if( vertWeldMode ) delete vertWeldMode;
	vertWeldMode = NULL;
	// CAL-06/02/03: copy/paste tangent. (FID #827)
	if ( copyTangentMode ) delete copyTangentMode;
	copyTangentMode = NULL;
	if ( pasteTangentMode ) delete pasteTangentMode;
	pasteTangentMode = NULL;
	
	// Reset show end result
	SetFlag (EP_DISP_RESULT, ip->GetShowEndResult());
	ip->SetShowEndResult(oldShowEnd);
	}

void EditPatchMod::DoDeleteSelected() {
	switch(GetSubobjectType()) {
		case EP_VERTEX:
			DoVertDelete();
			break;
		case EP_EDGE:
			DoEdgeDelete();
			break;
		case EP_PATCH:
			DoPatchDelete();
			break;
		}
	}

// Vertex Delete modifier method
void EditPatchMod::DoVertDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;

		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->vertSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoVertDelete"));
			// Call the vertex delete function
			DeleteSelVerts(patch);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTDELETE));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
		theHold.End();
		}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Edger Delete modifier method
void EditPatchMod::DoEdgeDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;

		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->edgeSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoEdgeDelete"));
			int edges = patch->getNumEdges();
			int patches = patch->getNumPatches();
			int verts = patch->getNumVerts();

			// Tag the patches that are attached to selected edges
			BitArray delPatches(patches);
			delPatches.ClearAll();

			for(int i = 0; i < edges; ++i) {
				if(patch->edgeSel[i]) {
					for(int j = 0; j < patch->edges[i].patches.Count(); ++j)
						delPatches.Set(patch->edges[i].patches[j]);
					}
				}

			BitArray delVerts(verts);
			delVerts.ClearAll();

			DeletePatchParts(patch, delVerts, delPatches);
			patch->computeInteriors();

			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_EDGEDELETE));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOEDGESSEL),PROMPT_TIME);
		theHold.End();
		}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::DoPatchAdd(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;

		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->edgeSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoPatchAdd"));
			// Call the patch add function
			AddPatches(type, patch, TRUE);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHADD));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDEDGESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::DoPatchDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoPatchDelete"));
			// Call the patch delete function
			DeleteSelPatches(patch);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHDELETE));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

class PatchLoop {
	public:
		int vertex;
		IntTab patches;
		class PatchLoop *next;
		PatchLoop() { vertex = -1; }
		PatchLoop& operator=(PatchLoop &pl) { vertex=pl.vertex; patches=pl.patches; }
	};

class PatchLoopTab {
	public:
		PatchLoop *head;
		int count;
		PatchLoopTab() { head = NULL; count = 0; }
		~PatchLoopTab() {
			PatchLoop *del = head;
			while(del) {
				PatchLoop *temp = del->next;
				delete del;
				del = temp;
				}
			}
		void Add(PatchLoop *item) {
			item->next = head;
			head = item;
			count++;
			}
		PatchLoop *operator[](int i) {
			PatchLoop *ret = head;
			while(i--)
				ret = ret->next;
			return ret;
			}
	};

static void CheckBreakLoop(PatchMesh *patch, PatchVert &v, int j, BitArray &pChecked, PatchLoop *pl) {
//static int recurse = 0;
//DebugPrint("+++ Recursing to level %d +++\n",++recurse);
	int addPatch = v.patches[j];
	Patch &p = patch->patches[addPatch];
	pl->patches.Append(1, &addPatch);
//DebugPrint("Added patch %d\n",addPatch);
	pChecked.Set(j);
	// Now find adjacent patches from our list, across non-selected edges
	for(int e = 0; e < p.type; ++e) {
//DebugPrint("Checking edge %d\n",e);
		if(!patch->edgeSel[p.edge[e]]) {
			PatchEdge &pe = patch->edges[p.edge[e]];
			for(int ep = 0; ep < pe.patches.Count(); ++ep) {
				int thePatch = pe.patches[ep];
				if(thePatch != addPatch) {
//DebugPrint("Checking patch %d\n",thePatch);
					for(int jj = 0; jj < v.patches.Count(); ++jj) {
						if(!pChecked[jj] && v.patches[jj] == thePatch)
							CheckBreakLoop(patch, v, jj, pChecked, pl);
						}
					}
				}
			}
//		else
//			DebugPrint("Edge %d selected\n",e);
		}
//DebugPrint("--- Recursing to level %d ---\n",--recurse);
	}

void EditPatchMod::DoBreak() {
	int level = GetSubobjectLevel();
	if(level != EP_VERTEX && level != EP_EDGE)
		return;
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL nothingToDo = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		patchData->PreUpdateChanges(patch);
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		int verts = patch->getNumVerts();
		int vecs = patch->getNumVecs();
		BitArray vertBreaks(verts);
		BitArray vecBreaks(vecs);
		vertBreaks.ClearAll();
		int numNewVerts = 0;
		int numNewVecs = 0;
		PatchLoopTab plt;
		switch(level) {
			case EP_VERTEX: {
				if(patch->vertSel.NumberSet()) {
					// Only mark those verts that are used by more than one patch
					for(int i = 0; i < verts; ++i) {
						if(patch->vertSel[i]) {
							if(patch->verts[i].patches.Count() > 1)
								vertBreaks.Set(i);
							}
						}
					}
				else 
					continue;
				// Mark the vectors involved
				for(int i = 0; i < verts; ++i) {
					if(vertBreaks[i]) {
						PatchVert &v = patch->verts[i];
						for(int j = 0; j < v.vectors.Count(); ++j) {
							PatchVec &vec = patch->vecs[v.vectors[j]];
							int newVecs = vec.patches.Count() - 1;
							if(newVecs) {
								vecBreaks.Set(v.vectors[j]);
								numNewVecs += newVecs;
								}
							}
						// Count up new verts needed
						numNewVerts += patch->verts[i].patches.Count() - 1;
						}
					}
				}
				break;
			case EP_EDGE:
				if(patch->edgeSel.NumberSet()) {
					int edges = patch->getNumEdges();
					// Only mark those vecs that are used by more than one patch
					for(int jk = 0; jk < edges; ++jk) {
						if(patch->edgeSel[jk]) {
							PatchEdge &e = patch->edges[jk];
							if(e.patches.Count() > 1) {
								vertBreaks.Set(e.v1);
								vecBreaks.Set(e.vec12);
								vecBreaks.Set(e.vec21);
								vertBreaks.Set(e.v2);
								numNewVecs += ((e.patches.Count() - 1) * 2);	// CAL-09/04/03: was patch->edges[i].patches.Count() (Defect #518909)
								}
							}
						}
					for(jk = 0; jk < verts; ++jk) {
						if(vertBreaks[jk]) {
//DebugPrint("*** Starting test for vertex %d ***\n",jk);
							PatchVert &v = patch->verts[jk];
							BitArray pChecked(v.patches.Count());
							pChecked.ClearAll();
							BOOL gotLoop = FALSE;
							for(int j = 0; j < v.patches.Count(); ++j) {
								if(!pChecked[j]) {
									PatchLoop *pl = new PatchLoop;
									pl->vertex = jk;
//DebugPrint("New loop started\n");
									CheckBreakLoop(patch, v, j, pChecked, pl);
									if(gotLoop) {
//DebugPrint("Got loop, Adding to plt\n");
										plt.Add(pl);
										numNewVerts++;
										}
									else {
//DebugPrint("No loop yet, discarding first one\n");
										delete pl;
										gotLoop = TRUE;
										}
									}
								}
							}
						}
					}
				else 
					continue;
				break;
			}
		if(!vertBreaks.NumberSet() && !vecBreaks.NumberSet()) {
			nothingToDo = TRUE;
			continue;
			}
		altered = holdNeeded = 1;
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"DoBreak"));
		// Add verts and vecs to the object
		patch->setNumVerts(verts + numNewVerts, TRUE);
		patch->setNumVecs(vecs + numNewVecs, TRUE);
		// Duplicate the verts and redirect the patches using them
		int vertIx = verts;
		if(level == EP_VERTEX) {
			// CAL-09/04/03: declare i as local (use "int i = 0"), don't mess with the outter i variable.
			for(int i = 0; i < verts; ++i) {
				if(vertBreaks[i]) {
					PatchVert &v = patch->verts[i];
					// Leave the first one alone, but clone the rest!
					for(int j = 1; j < v.patches.Count(); ++j) {
						// Clone the vertex
						patch->verts[vertIx] = v;
						// Zap its aux index!
						patch->verts[vertIx].aux1 = -1;
						// Now replace it in the patch
						Patch &p = patch->patches[v.patches[j]];
						for(int k = 0; k < p.type; ++k) {
							if(p.v[k] == i)
								p.v[k] = vertIx;
							}
						// Select the new vertex!
						if(level == EP_VERTEX)
							patch->vertSel.Set(vertIx);
						vertIx++;
						}
					}
				}
			}
		else {	// Edge code
			assert(plt.count == numNewVerts);
			// Go thru the patch loop table and create a new vertex for each eentry, assigning it to the
			// appropriate vertex in the appropriate patch
			for(int i = 0; i < plt.count; ++i) {
				PatchLoop *pl = plt[i];
				// Clone the vertex
				patch->verts[vertIx] = patch->verts[pl->vertex];
				// Zap its aux index!
				patch->verts[vertIx].aux1 = -1;
				// Go thru the patches and assign them
				for(int j = 0; j < pl->patches.Count(); ++j) {
					Patch &p = patch->patches[pl->patches[j]];
					for(int vx = 0; vx < p.type; ++vx) {
						if(p.v[vx] == pl->vertex)
							p.v[vx] = vertIx;
						}
					}
				vertIx++;
				}
			}
		// Duplicate the vectors and redirect the patches using them
		int vecIx = vecs;
		// CAL-09/04/03: declare i as local (use "int i = 0"), don't mess with the outter i variable.
		{
		for(int i = 0; i < vecs; ++i) {
			if(vecBreaks[i]) {
				PatchVec &v = patch->vecs[i];
				// Leave the first one alone, but clone the rest!
				for(int j = 1; j < v.patches.Count(); ++j) {
					// Clone the vector
					patch->vecs[vecIx] = v;
					// Zap its aux index!
					patch->vecs[vecIx].aux1 = -1;
					// Now replace it in the patch
					Patch &p = patch->patches[v.patches[j]];
					for(int k = 0; k < (p.type * 2); ++k) {
						if(p.vec[k] == i)
							p.vec[k] = vecIx;
						}
					// CAL-06/10/03: add handle sub-object mode. (FID #1914)
					patch->vecSel.Set(vecIx, patch->vecSel[i]);
					vecIx++;
					}
				}
			}
		}
		patch->InvalidateMesh();
		patch->buildLinkages();
		patch->computeInteriors();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		holdNeeded = TRUE;
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(level == PO_VERTEX ? IDS_TH_BREAK_VERTEX : IDS_TH_BREAK_EDGE));
		}
	else {
		if(nothingToDo)
			ip->DisplayTempPrompt(GetString(IDS_TH_NO_VALID_GEOMETRY_SELECTED),PROMPT_TIME);
		else
			ip->DisplayTempPrompt(GetString(level == PO_VERTEX ? IDS_TH_NOVERTSSEL : IDS_TH_NOEDGESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	PatchSelChanged();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

//watje 12-10-98
void EditPatchMod::DoUnHide() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
//		if(patch->patchSel.NumberSet()) {
		if(1) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			UnHidePatches(patch);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

static
TSTR shapeName;

static
INT_PTR CALLBACK CreateShapeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
			// WIN64 Cleanup: Shuler
	TCHAR tempName[256];
	switch(message) {
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_SHAPE_NAME, shapeName);
			SetFocus(GetDlgItem(hDlg, IDC_SHAPE_NAME));
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hDlg, IDC_SHAPE_NAME, tempName, 255);
					shapeName = TSTR(tempName);
					EndDialog(hDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}

static
int GetCreateShapeOptions(IObjParam *ip, TSTR& newName) {
	shapeName = newName;
	ip->MakeNameUnique(shapeName);	
	if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_CREATE_SHAPE), ip->GetMAXHWnd(), (DLGPROC)CreateShapeDialogProc)==1) {
		newName = shapeName;
		return 1;
		}
	return 0;
	}

void EditPatchMod::DoCreateShape() {
	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_SHAPE));
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();

	// Create a spline object
	SplineShape *splShape = new SplineShape;

	int multipleObjects = (mcList.Count() > 1) ? 1 : 0;
	int steps = 5;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int j;
		int altered = 0;
		Matrix3 tm(1);
		BitArray eSel;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;
		steps = patchData->meshSteps;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		
		if(!dialoged) {
			dialoged = 1;
			if(!GetCreateShapeOptions(ip, newName))
				goto bail_out;
			}

		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		holdNeeded = TRUE;
		eSel = patch->edgeSel;
		if(eSel.NumberSet() == 0)
			eSel.SetAll();
		if(multipleObjects)
			tm = nodes[i]->GetObjectTM(t);
		for(j = 0; j < patch->numEdges; ++j) {
			if(eSel[j]) {
				PatchEdge &e = patch->edges[j];
				Spline3D *newSpline = splShape->shape.NewSpline();
				SplineKnot k1(KTYPE_BEZIER, LTYPE_CURVE, patch->verts[e.v1].p * tm, patch->verts[e.v1].p * tm, patch->vecs[e.vec12].p * tm);
				SplineKnot k2(KTYPE_BEZIER, LTYPE_CURVE, patch->verts[e.v2].p * tm, patch->vecs[e.vec21].p * tm, patch->verts[e.v2].p * tm);
				newSpline->AddKnot(k1);
				newSpline->AddKnot(k2);
				}
			}

		bail_out:
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		splShape->shape.UpdateSels();	// Make sure it readies the selection set info
		splShape->shape.UpdateBindList();
		splShape->shape.InvalidateGeomCache();
		splShape->steps = steps;
		splShape->shape.steps = steps;
		INode *newNode = ip->CreateObjectNode(splShape);
		newNode->SetMtl(nodes[0]->GetMtl());
		newNode->SetName(newName.data());
		if(!multipleObjects) {	// Single input object?
			Matrix3 tm = nodes[0]->GetObjectTM(t);
			newNode->SetNodeTM(t, tm);	// Use this object's TM.
			}
		else {
			Matrix3 matrix;
			matrix.IdentityMatrix();
			newNode->SetNodeTM(t, matrix);	// Use identity TM
			}
		newNode->FlagForeground(t);		// WORKAROUND!
		theHold.Accept(GetString(IDS_TH_CREATE_SHAPE));
		}
	else {
		delete splShape;	// Didn't need it after all!
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	ip->RedrawViews(t,REDRAW_NORMAL);
	}

void EditPatchMod::DoHide(int type) {
	switch(type) {
		case EP_VERTEX:
			DoVertHide();
			break;
		case EP_EDGE:
			DoEdgeHide();
			break;
		case EP_PATCH:
			DoPatchHide();
			break;
		}
	}

void EditPatchMod::DoPatchHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			HidePatches(patch);
			patch->patchSel.ClearAll();
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

	}

void EditPatchMod::DoVertHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->vertSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			HideVerts(patch);
			patch->vertSel.ClearAll();
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

	}

void EditPatchMod::DoEdgeHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->edgeSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			HideEdges(patch);
			patch->edgeSel.ClearAll();
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

	}


void EditPatchMod::DoAddHook(PatchMesh *pMesh, int vert, int seg) {


	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);


	if (mcList.Count() != 1) return;

	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
//	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if ((!patch) || (patch != pMesh))
			continue;		
//		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
//		if(patch->vertSel.NumberSet()) {

		altered = holdNeeded = TRUE;
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
		patch->AddHook(vert,seg);
//		patch->UpdateHooks();
//			InvalidateMesh();

		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
//			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
//		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

/*	// If any bits are set in the selection set, let's DO IT!!
	if ( !ip ) return;
	theHold.Begin();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if ( theHold.Holding() )
		theHold.Put(new PatchObjectRestore(this,rec));
		// Call the patch type change function

	patch.AddHook();
	patch.InvalidateGeomCache();
	InvalidateMesh();
	theHold.Accept(GetResString(IDS_TH_PATCHCHANGE));

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/
	}

void EditPatchMod::DoRemoveHook() 
{

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->vertSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			patch->RemoveHook();
//			patch->InvalidateGeomCache();
//			InvalidateMesh();
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);

//watje 10-14-99 bug 212886 some spurious topo flags need to be cleared
			patch->hookTopoMarkers.ZeroCount();
			patch->hookTopoMarkersA.ZeroCount();
			patch->hookTopoMarkersB.ZeroCount();

			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


void EditPatchMod::DoExtrude()
{
	int type = GetSubobjectType();
	BOOL cloneEdges = FALSE;
	if(type == EP_EDGE)
		cloneEdges = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if((type == EP_PATCH && patch->patchSel.NumberSet())|| (type == EP_EDGE && patch->edgeSel.NumberSet())) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the extrusion start function
			patch->CreateExtrusion(type == EP_PATCH ? PATCH_PATCH : PATCH_EDGE, cloneEdges);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}


	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

/*
	theHold.Begin();
	patch.RecordTopologyTags();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if ( theHold.Holding() )
		theHold.Put(new PatchObjectRestore(this,rec));

	patch.CreateExtrusion();
	
	ResolveTopoChanges();
	theHold.Accept(GetResString(IDS_TH_PATCHADD));

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/
}


void EditPatchMod::BeginExtrude(TimeValue t) {	
	if (inExtrude) return;
	inExtrude = TRUE;
	theHold.SuperBegin();
	if(GetSubobjectType() == EP_PATCH)
		MaybeClonePatchParts();
	DoExtrude();
//	PlugControllersSel(t,sel);
	theHold.Begin();
}

void EditPatchMod::EndExtrude (TimeValue t, BOOL accept) {		
	if (!ip) return;

	if (!inExtrude) return;

	ISpinnerControl *spin;
	inExtrude = FALSE;
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
		}
//	TempData()->freeBevelInfo();
	// Select the new extruded edges
	ModContextList mcList;	
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData,this,patch));
		patch->edgeSel.ClearAll();
		for(int i = 0; i < patch->newEdges.Count(); ++i)
			patch->edgeSel.Set(patch->newEdges[i]);
		ip->ClearCurNamedSelSet();
		patchData->UpdateChanges(patch, FALSE);
		}
	PatchSelChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
	nodes.DisposeTemporary();

	theHold.Accept(GetString(IDS_RB_EXTRUDE));
	if (accept)
		theHold.SuperAccept(GetString(IDS_RB_EXTRUDE));
	else
		theHold.SuperCancel();
}

void EditPatchMod::Extrude( TimeValue t, float amount, BOOL useLocalNorms ) {
	if (!inExtrude) return;
	int type = GetSubobjectType();

	ModContextList mcList;		
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

//	theHold.Begin();
	RecordTopologyTags();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if((type == EP_PATCH && patch->patchSel.NumberSet())|| (type == EP_EDGE && patch->edgeSel.NumberSet())) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex normal move function
			patch->MoveNormal(amount,useLocalNorms,type == EP_PATCH ? PATCH_PATCH : PATCH_EDGE);

			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}


	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

/*	theHold.Restore();
	patch.MoveNormal(amount,useLocalNorms);

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/


}


static void SwapInt(int &a, int &b) {
	int temp = a;
	a = b;
	b = temp;
	}

static BitArray flippedPatches;
static BitArray coplanarVerts;
static BitArray visitedPatch;

static void EstablishFlipArrays(PatchMesh *patch) {
	flippedPatches.SetSize(patch->numPatches);
	flippedPatches.ClearAll();
	coplanarVerts.SetSize(patch->numVerts);
	coplanarVerts.ClearAll();
	visitedPatch.SetSize(patch->numPatches);
	visitedPatch.ClearAll();
	}

static void KillFlipArrays() {
	flippedPatches.SetSize(0);
	coplanarVerts.SetSize(0);
	visitedPatch.SetSize(0);
	}

static void FlipPatchNormal(PatchMesh *patch, int index) {
	Patch &p = patch->patches[index];
	flippedPatches.Set(index);
	switch(p.type) {
		case PATCH_TRI:
			SwapInt(p.v[1], p.v[2]);
			SwapInt(p.vec[0], p.vec[5]);
			SwapInt(p.vec[1], p.vec[4]);
			SwapInt(p.vec[2], p.vec[3]);
			SwapInt(p.interior[1], p.interior[2]);
			SwapInt(p.edge[0], p.edge[2]);
			break;
		case PATCH_QUAD:
			SwapInt(p.v[1], p.v[3]);
			SwapInt(p.vec[0], p.vec[7]);
			SwapInt(p.vec[1], p.vec[6]);
			SwapInt(p.vec[2], p.vec[5]);
			SwapInt(p.vec[3], p.vec[4]);
			SwapInt(p.interior[1], p.interior[3]);
			SwapInt(p.edge[0], p.edge[3]);
			SwapInt(p.edge[1], p.edge[2]);
			break;
		}
	// Note any coplanar vertices
	for(int i = 0; i < p.type; ++i) {
		if(patch->verts[p.v[i]].flags & PVERT_COPLANAR)
			coplanarVerts.Set(p.v[i]);
		}
	}		

static void FixupBorderCoplanars(PatchMesh *patch) {
	for(int i = 0; i < patch->numVerts; ++i) {
		if(coplanarVerts[i]) {
			PatchVert &v = patch->verts[i];
			// If any patches weren't flipped, this vertex is a border one and needs to have
			// its coplanar flag turned off
			for(int j = 0; j < v.patches.Count(); ++j) {
				if(!flippedPatches[v.patches[j]]) {
					v.flags &= ~PVERT_COPLANAR;
					break;
					}
				}
			}
		}
	}

void EditPatchMod::DoFlipNormals(PatchMesh *pmesh, int patchIndex, BOOL element) {
	BOOL holdNeeded = FALSE;
	ModContextList mcList;	
	INodeTab nodes;
	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	
	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;
		if(pmesh && patch != pmesh)
			continue;
		// If any bits are set in the selection set, let's DO IT!!
		if(patchIndex < 0 && !patch->patchSel.NumberSet())
			continue;
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"NormalFlip"));
		holdNeeded = TRUE;
		// Set up the bitarrays we need
		EstablishFlipArrays(patch);
		// If element, select the element using the supplied patch index!
		if(element) {
			patch->patchSel = patch->GetElement(patchIndex);
			patchIndex = -1;	// This makes the code operate on the element
			}
		// Reverse normals on the patches
		if(patchIndex < 0) {
			for(int i = 0; i < patch->numPatches; ++i) {
				if(patch->patchSel[i])
					FlipPatchNormal(patch, i);
				}
			}
		else
			FlipPatchNormal(patch, patchIndex);
		FixupBorderCoplanars(patch);
		// Flush the flip arrays
		KillFlipArrays();
		patch->InvalidateGeomCache();
		patch->buildLinkages();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_FLIP_NORMALS));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void UnifyPatches(PatchMesh *patch, int pix);

static void UnifyEdges(PatchMesh *patch, int edgeIndex, int v1, int v2) {
	PatchEdge &e = patch->edges[edgeIndex];
	int v12, v21;
	// Establish the "polarity" of this edge and set up some comparison values
	if(v1 == e.v1 && v2 == e.v2) {
		v12 = e.vec12;
		v21 = e.vec21;
		}
	else
	if(v1 == e.v2 && v2 == e.v1) {
		v12 = e.vec21;
		v21 = e.vec12;
		}
	else
		assert(0);	// Something's wrong, edge endpoints don't match
	for(int i = 0; i < e.patches.Count(); ++i) {
		int thePatch = e.patches[i];
		if(patch->patchSel[thePatch] && !visitedPatch[thePatch]) {
			Patch &p = patch->patches[thePatch];
			// See if this one needs to be flipped
			for(int j = 0; j < p.type; ++j) {
				if(p.v[j] == v1 && p.vec[j*2] == v12 && p.vec[j*2+1] == v21 && p.v[(j+1)%p.type] == v2) {
					FlipPatchNormal(patch, thePatch);
					break;
					}
				}
			UnifyPatches(patch, thePatch);
			}
		}
	}

static void UnifyPatches(PatchMesh *patch, int pix) {
	// Mark it as visited
	visitedPatch.Set(pix);
	Patch &p = patch->patches[pix];
	// Now visit all adjoining patches and process them!
	for(int i = 0; i < p.type; ++i) {
		UnifyEdges(patch, p.edge[i], p.v[i], p.v[(i+1)%p.type]);
		}
	}

void EditPatchMod::DoUnifyNormals() {
	BOOL holdNeeded = FALSE;
	ModContextList mcList;	
	INodeTab nodes;
	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	
	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"NormalFlip"));
		holdNeeded = TRUE;
		// Set up the bitarrays we need
		EstablishFlipArrays(patch);
		// Find selected patch that hasn't been visited, and process it
		for(int i = 0; i < patch->numPatches; ++i) {
			if(patch->patchSel[i] && !visitedPatch[i])
				UnifyPatches(patch, i);
			}
		FixupBorderCoplanars(patch);
		// Flush the flip arrays
		KillFlipArrays();
		patch->InvalidateGeomCache();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_UNIFY_NORMALS));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::DoBevel()
{

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {

			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			patch->CreateBevel();
//			patch->CreateExtrusion();
//			patch->InvalidateGeomCache();
//			InvalidateMesh();

			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}


	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

	/*
	theHold.Begin();
	patch.RecordTopologyTags();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if ( theHold.Holding() )
		theHold.Put(new PatchObjectRestore(this,rec));

	patch.CreateBevel();
	
	ResolveTopoChanges();
	theHold.Accept(GetResString(IDS_TH_PATCHADD));

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/
}


void EditPatchMod::BeginBevel(TimeValue t) {	
	if (inBevel) return;
	inBevel = TRUE;
	theHold.SuperBegin();
	DoBevel();
//	PlugControllersSel(t,sel);
	theHold.Begin();
}

void EditPatchMod::EndBevel (TimeValue t, BOOL accept) {		
	if (!ip) return;
	if (!inBevel) return;
	inBevel = FALSE;
//	TempData()->freeBevelInfo();
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
		}


	theHold.Accept(GetString(IDS_EM_BEVEL));
	if (accept) theHold.SuperAccept(GetString(IDS_EM_BEVEL));
	else theHold.SuperCancel();

}



void EditPatchMod::Bevel( TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd ) {
	if (!inBevel) return;


	ModContextList mcList;		
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

//	theHold.Begin();

	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {

			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch));
			// Call the vertex type change function
			patch->Bevel(amount,smoothStart, smoothEnd);
//			patch->MoveNormal(amount,useLocalNorms);
//			patch->InvalidateGeomCache();
//			InvalidateMesh();

			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}


	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
/*
	theHold.Restore();
	patch.Bevel(amount,smoothStart, smoothEnd);

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/
}


// CAL-04/23/03: patch smooth
void EditPatchMod::DoPatchSmooth(int type)
{
	if ( !ip ) return;

	ModContextList mcList;
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	BOOL altered = FALSE;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;

		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch));

		// call smooth function
		BOOL res = FALSE;
		switch (type)
		{
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			res = patch->PatchSmoothVector();
			break;
		case EP_VERTEX:
			res = patch->PatchSmoothVertex();
			break;
		case EP_EDGE:
			res = patch->PatchSmoothEdge();
			break;
		case EP_PATCH:
			res = patch->PatchSmoothPatch();
			break;
		default:	// EP_OBJECT
			res = patch->PatchSmoothVertex(false);
			break;
		}

		if (res) {
			altered = TRUE;
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}

	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	nodes.DisposeTemporary();

	if (altered) {
		theHold.Accept(GetString(IDS_EP_PATCH_SMOOTH));
		NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	} else {
		theHold.End();
	}
}


// Method used to add a newly created Vertex
void EditPatchMod::CreateVertex(Point3 pt, int& newIndex) {
	BOOL holdNeeded = FALSE;
	ModContextList mcList;	
	INodeTab nodes;
	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	
	theHold.Begin();
	RecordTopologyTags();

	// act on only the first object
//	for (int i = 0; i < mcList.Count(); i++) {
	EditPatchData *patchData = NULL;
	PatchMesh *patch = NULL;
	if (mcList.Count()) patchData = (EditPatchData*)mcList[0]->localData;
	if (patchData) {
		if (!patchData->GetFlag(EPD_BEENDONE)) {
			patchData->BeginEdit(ip->GetTime());
			patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		}
	}
	if(patch) {
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"CreateVertex"));
		holdNeeded = TRUE;

		// Add the new vertex
		pt = pt * Inverse(nodes[0]->GetObjectTM(ip->GetTime()));  // is nodes[i] correct???????
		newIndex = patch->getNumVerts();
		patch->setNumVerts(newIndex+1,TRUE);
		patch->setVert(newIndex,pt);


		patch->InvalidateGeomCache();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_CREATEVERTEX));
	}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
	}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
}


// Method used to add a newly created TriPatch from vertex indices
void EditPatchMod::CreatePatch(int vertIndxA,int vertIndxB,int vertIndxC) {
	BOOL holdNeeded = FALSE;
	ModContextList mcList;	
	INodeTab nodes;
	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	
	theHold.Begin();
	RecordTopologyTags();

//	for (int i = 0; i < mcList.Count(); i++) {
	// act on only the first object
	EditPatchData *patchData = NULL;
	PatchMesh *patch = NULL;
	if (mcList.Count()) patchData = (EditPatchData*)mcList[0]->localData;
	if (patchData) {
		if (!patchData->GetFlag(EPD_BEENDONE)) {
			patchData->BeginEdit(ip->GetTime());
			patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		}
	}
	if(patch) {
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"CreatePatch"));
		holdNeeded = TRUE;

		// allocate the new patch
		int numPatches;
		numPatches = patch->getNumPatches();
		patch->setNumPatches(numPatches+1,TRUE);

		Point3 vertA = patch->getVert(vertIndxA).p;
		Point3 vertB = patch->getVert(vertIndxB).p;
		Point3 vertC = patch->getVert(vertIndxC).p;

		// current number of control vectors
		int numVecs;
		numVecs = patch->getNumVecs();

		// allocate the new interiors
		int interior1 = numVecs, 
			interior2 = numVecs+1,
			interior3 = numVecs+2;
		numVecs += 3; // need 3 new ones for the interiors

		// check if any existing edges were selected
		Tab<int> foundEdgesAB = patch->GetEdge(vertIndxA,vertIndxB);
		Tab<int> foundEdgesBC = patch->GetEdge(vertIndxB,vertIndxC);
		Tab<int> foundEdgesCA = patch->GetEdge(vertIndxC,vertIndxA);

		// allocate the edge vectors when none are selected
		int vecAB, vecBA, vecBC, vecCB, vecCA, vecAC;
		if (!foundEdgesAB.Count()) {
			vecAB = numVecs; 
			vecBA = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		if (!foundEdgesBC.Count()) {
			vecBC = numVecs; 
			vecCB = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		if (!foundEdgesCA.Count()) {
			vecCA = numVecs; 
			vecAC = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		patch->setNumVecs(numVecs,TRUE);

		// set or build the edge vectors
		if (foundEdgesAB.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeAB = patch->edges[foundEdgesAB[0]];
			if(edgeAB.v1 == vertIndxA ) {
				vecAB = edgeAB.vec12;
				vecBA = edgeAB.vec21;
			}
			else {
				vecAB = edgeAB.vec21;
				vecBA = edgeAB.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecAB,  vertA + (vertB - vertA) / 3.0f); // vab
			patch->setVec(vecBA,  vertB + (vertA - vertB) / 3.0f); // vba
		}

		if (foundEdgesBC.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeBC = patch->edges[foundEdgesBC[0]];
			if(edgeBC.v1 == vertIndxB ) {
				vecBC = edgeBC.vec12;
				vecCB = edgeBC.vec21;
			}
			else {
				vecBC = edgeBC.vec21;
				vecCB = edgeBC.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecBC,  vertB + (vertC - vertB) / 3.0f); // vbc
			patch->setVec(vecCB,  vertC + (vertB - vertC) / 3.0f); // vcb
		}

		if (foundEdgesCA.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeCA = patch->edges[foundEdgesCA[0]];
			if(edgeCA.v1 == vertIndxC ) {
				vecCA = edgeCA.vec12;
				vecAC = edgeCA.vec21;
			}
			else {
				vecCA = edgeCA.vec21;
				vecAC = edgeCA.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecCA,  vertC + (vertA - vertC) / 3.0f); // vca
			patch->setVec(vecAC,  vertA + (vertC - vertA) / 3.0f); // vac
		}

		// select a smoothing group that does not match a neighboring patch
		// find all neighboring smoothing groups
		DWORD neighborSmGroups = 0;
		Tab<int> neighboringPatches = patch->GetPatches(vertIndxA);
		for (int i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		neighboringPatches = patch->GetPatches(vertIndxB);
		for (i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		neighboringPatches = patch->GetPatches(vertIndxC);
		for (i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		// select the first unused smoothing group from the list
		DWORD smthGrp = 1;
		for (i=0; i<32; ++i) {
			if ( !(smthGrp & neighborSmGroups)) break;
			smthGrp *= 2;
		}

		// Add the new patch
		patch->MakeTriPatch(
			numPatches, 
			vertIndxA, vecAB, vecBA,  
			vertIndxB, vecBC, vecCB,  
			vertIndxC, vecCA, vecAC,  
			interior1, interior2, interior3,  
			smthGrp);          

		// make the new patch selected
		patch->patchSel.Set(numPatches);

		// calculate the interior points
		patch->computeInteriors();
		// update all of the topology
		patch->buildLinkages();


		// TIDY UP

		patch->InvalidateGeomCache();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_CREATEPATCH));
	}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
	}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	PatchSelChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
}

// Method used to add a newly created QuadPatch from vetex indices
void EditPatchMod::CreatePatch(int vertIndxA,int vertIndxB,int vertIndxC,int vertIndxD) {
	BOOL holdNeeded = FALSE;
	ModContextList mcList;	
	INodeTab nodes;
	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	
	theHold.Begin();
	RecordTopologyTags();

//	for (int i = 0; i < mcList.Count(); i++) {
	// act on only the first object
	EditPatchData *patchData = NULL;
	PatchMesh *patch = NULL;
	if (mcList.Count()) patchData = (EditPatchData*)mcList[0]->localData;
	if (patchData) {
		if (!patchData->GetFlag(EPD_BEENDONE)) {
			patchData->BeginEdit(ip->GetTime());
			patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		}
	}
	if(patch) {
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"CreatePatch"));
		holdNeeded = TRUE;


		// allocate the new patch
		int numPatches;
		numPatches = patch->getNumPatches();
		patch->setNumPatches(numPatches+1,TRUE);

		Point3 vertA = patch->getVert(vertIndxA).p;
		Point3 vertB = patch->getVert(vertIndxB).p;
		Point3 vertC = patch->getVert(vertIndxC).p;
		Point3 vertD = patch->getVert(vertIndxD).p;

		// current number of control vectors
		int numVecs;
		numVecs = patch->getNumVecs();

		// allocate the new interiors
		int interior1 = numVecs, 
			interior2 = numVecs+1,
			interior3 = numVecs+2,
			interior4 = numVecs+3;
		numVecs += 4; // need 4 new ones for the interiors

		// check if any existing edges were selected
		Tab<int> foundEdgesAB = patch->GetEdge(vertIndxA,vertIndxB);
		Tab<int> foundEdgesBC = patch->GetEdge(vertIndxB,vertIndxC);
		Tab<int> foundEdgesCD = patch->GetEdge(vertIndxC,vertIndxD);
		Tab<int> foundEdgesDA = patch->GetEdge(vertIndxD,vertIndxA);

		// allocate the edge vectors when none are selected
		int vecAB, vecBA, vecBC, vecCB, vecCD, vecDC, vecDA, vecAD;
		if (!foundEdgesAB.Count()) {
			vecAB = numVecs; 
			vecBA = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		if (!foundEdgesBC.Count()) {
			vecBC = numVecs; 
			vecCB = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		if (!foundEdgesCD.Count()) {
			vecCD = numVecs; 
			vecDC = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		if (!foundEdgesDA.Count()) {
			vecDA = numVecs; 
			vecAD = numVecs+1;
			numVecs += 2; // need 2 new ones for this edge
		}
		patch->setNumVecs(numVecs,TRUE);

		// set or build the edge vectors
		if (foundEdgesAB.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeAB = patch->edges[foundEdgesAB[0]];
			if(edgeAB.v1 == vertIndxA ) {
				vecAB = edgeAB.vec12;
				vecBA = edgeAB.vec21;
			}
			else {
				vecAB = edgeAB.vec21;
				vecBA = edgeAB.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecAB,  vertA + (vertB - vertA) / 3.0f); // vab
			patch->setVec(vecBA,  vertB + (vertA - vertB) / 3.0f); // vba
		}

		if (foundEdgesBC.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeBC = patch->edges[foundEdgesBC[0]];
			if(edgeBC.v1 == vertIndxB ) {
				vecBC = edgeBC.vec12;
				vecCB = edgeBC.vec21;
			}
			else {
				vecBC = edgeBC.vec21;
				vecCB = edgeBC.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecBC,  vertB + (vertC - vertB) / 3.0f); // vbc
			patch->setVec(vecCB,  vertC + (vertB - vertC) / 3.0f); // vcb
		}

		if (foundEdgesCD.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeCD = patch->edges[foundEdgesCD[0]];
			if(edgeCD.v1 == vertIndxC ) {
				vecCD = edgeCD.vec12;
				vecDC = edgeCD.vec21;
			}
			else {
				vecCD = edgeCD.vec21;
				vecDC = edgeCD.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecCD,  vertC + (vertD - vertC) / 3.0f); // vcd
			patch->setVec(vecDC,  vertD + (vertC - vertD) / 3.0f); // vdc
		}

		if (foundEdgesDA.Count()) { 
			// use the 1st selected edge vectors
			PatchEdge& edgeDA = patch->edges[foundEdgesDA[0]];
			if(edgeDA.v1 == vertIndxD ) {
				vecDA = edgeDA.vec12;
				vecAD = edgeDA.vec21;
			}
			else {
				vecDA = edgeDA.vec21;
				vecAD = edgeDA.vec12;
			}
		}
		else { 
			// build default vectors
			patch->setVec(vecDA,  vertD + (vertA - vertD) / 3.0f); // vda
			patch->setVec(vecAD,  vertA + (vertD - vertA) / 3.0f); // vad
		}

		// select a smoothing group that does not match a neighboring patch
		// find all neighboring smoothing groups
		DWORD neighborSmGroups = 0;
		Tab<int> neighboringPatches = patch->GetPatches(vertIndxA);
		for (int i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		neighboringPatches = patch->GetPatches(vertIndxB);
		for (i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		neighboringPatches = patch->GetPatches(vertIndxC);
		for (i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		neighboringPatches = patch->GetPatches(vertIndxD);
		for (i=0; i<neighboringPatches.Count(); ++i)
			neighborSmGroups |= patch->patches[neighboringPatches[i]].smGroup;

		// select the first unused smoothing group from the list
		DWORD smthGrp = 1;
		for (i=0; i<32; ++i) {
			if ( !(smthGrp & neighborSmGroups)) break;
			smthGrp *= 2;
		}

		// Add the new patch
		patch->MakeQuadPatch(
			numPatches, 
			vertIndxA, vecAB, vecBA,  
			vertIndxB, vecBC, vecCB,  
			vertIndxC, vecCD, vecDC,  
			vertIndxD, vecDA, vecAD, 
			interior1, interior2, interior3, interior4, 
			smthGrp);          

		// make the new patch selected
		patch->patchSel.Set(numPatches);

		// calculate the interior points
		patch->computeInteriors();
		// update all of the topology
		patch->buildLinkages();


		// TIDY UP

		patch->InvalidateGeomCache();
		patchData->UpdateChanges(patch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_CREATEPATCH));
	}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
	}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	PatchSelChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

}



void EditPatchMod::DoSubdivide(int type) {
	switch(type) {
		case EP_EDGE:
			DoEdgeSubdivide();
			break;
		case EP_PATCH:
			DoPatchSubdivide();
			break;
		}
	}

void EditPatchMod::DoEdgeSubdivide() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->edgeSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoEdgeSubdivide"));
			// Call the patch add function
			SubdividePatch(SUBDIV_EDGES, propagate, patch);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_EDGESUBDIVIDE));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDEDGESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::DoPatchSubdivide() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
			altered = holdNeeded = 1;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoPatchSubdivide"));
			// Call the patch add function
			SubdividePatch(SUBDIV_PATCHES, propagate, patch);
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHSUBDIVIDE));
		PatchSelChanged();
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditPatchMod::DoVertWeld() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	int vertsWelded = 0;
	BOOL hadSel = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->vertSel.NumberSet() > 1) {
			hadSel = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoVertWeld"));
			// Call the patch weld function
			int oldVertCount = patch->getNumVerts();
			if(patch->Weld(weldThreshold)) {
				vertsWelded += (oldVertCount - patch->getNumVerts());

				altered = holdNeeded = TRUE;
				patchData->UpdateChanges(patch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
				}
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTWELD));
		PatchSelChanged();
		TSTR prompt;
		prompt.printf(GetString(IDS_N_VERTICES_WELDED), vertsWelded);
		ip->DisplayTempPrompt(prompt,PROMPT_TIME);
		}
	else {
		if(!hadSel)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
		else
			ip->DisplayTempPrompt(GetString(IDS_TH_NOWELDPERFORMED),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


void EditPatchMod::DoVertWeld(int fromVert, int toVert, ModContext* pTheModContext) {
	BOOL holdNeeded = FALSE;
	if ( !ip ) return;
	theHold.Begin();
	RecordTopologyTags();
	EditPatchData *patchData = NULL;
	PatchMesh *patch = NULL;
	if (pTheModContext/*mcList.Count()*/) patchData = (EditPatchData*)/*mcList[0]*/pTheModContext->localData;
	if (patchData) {
		if (!patchData->GetFlag(EPD_BEENDONE)) {
			patchData->BeginEdit(ip->GetTime());
			patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		}
	}
	if(patch) {
		patchData->PreUpdateChanges(patch);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch,"VertWeld"));

		// weld vertices
		if (patch->Weld(fromVert, toVert)) {
			holdNeeded = TRUE;

			patch->InvalidateGeomCache();
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
	}
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_VERTWELD));
		PatchSelChanged();
	}
	else {
		ip->DisplayTempPrompt(GetString(IDS_COULDNOTWELDVERTICES),PROMPT_TIME); 
		theHold.End();
	}
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

}

void EditPatchMod::DoEdgeWeld() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	int edgesWelded = 0;
	BOOL hadSel = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->edgeSel.NumberSet() > 1) {
			hadSel = TRUE;
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"DoVertWeld"));
			// Call the patch weld function
			int oldEdgeCount = patch->getNumEdges();
			if(patch->WeldEdges()) {
				edgesWelded += (oldEdgeCount - patch->getNumEdges());

				altered = holdNeeded = TRUE;
				patchData->UpdateChanges(patch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
				}
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_EDGEWELD));
		PatchSelChanged();
		TSTR prompt;
		prompt.printf(GetString(IDS_N_EDGES_WELDED), edgesWelded);
		ip->DisplayTempPrompt(prompt,PROMPT_TIME);
		}
	else {
		if(!hadSel)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOEDGESSEL),PROMPT_TIME);
		else
			ip->DisplayTempPrompt(GetString(IDS_TH_NOEDGEWELDPERFORMED),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


static void MakeDummyMapPatches(int channel, PatchMesh *patch) {
	patch->setNumMapVerts (channel, 1);
	patch->mapVerts(channel)[0] = UVVert(0,0,0);
	patch->setNumMapPatches (channel, patch->numPatches);
	TVPatch *tp = patch->mapPatches(channel);
	for(int i = 0; i < patch->numPatches; ++i) tp[i].Init();	// Sets all indices to zero
	}



// Detach all selected patches
void EditPatchMod::DoPatchDetach(int copy, int reorient) {
	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_PATCH));
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();

	// Create a patch mesh object
	PatchObject *patchOb = new PatchObject;
	PatchMesh &pmesh = patchOb->patch;
	int verts = 0;
	int vecs = 0;
	int patches = 0;

	int multipleObjects = (mcList.Count() > 1) ? 1 : 0;

//watje 10-4-99  184681 
	Tab<HookPoint> whooks;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any patches selected, we'll need to process this one
		if(patch->patchSel.NumberSet()) {
			if(!dialoged) {
				dialoged = 1;
				if(!GetDetachOptions(ip, newName))
					goto bail_out;
				}
			// Save the unmodified info.
			patchData->PreUpdateChanges(patch);
			if ( theHold.Holding() ) {
				theHold.Put(new PatchRestore(patchData,this,patch,"DoPatchDetach"));
				}
			PatchMesh wpatch = *patch;
//watje 10-4-99 184681
			wpatch.hooks = patch->hooks;
			wpatch.hookTopoMarkers = patch->hookTopoMarkers;
			wpatch.hookTopoMarkersA = patch->hookTopoMarkersA;
			wpatch.hookTopoMarkersB = patch->hookTopoMarkersB;

			BitArray vdel(wpatch.numVerts);
			vdel.ClearAll();
			BitArray pdel = wpatch.patchSel;	// Get inverse selection set
			// If not copying, delete the patches from this object
			if(!copy)
				DeletePatchParts(patch, vdel, pdel);
			pdel = ~wpatch.patchSel;	// Get inverse selection set
			if(pdel.NumberSet()) {
				vdel.ClearAll();
				DeletePatchParts(&wpatch, vdel, pdel);
				}
//watje 10-4-99 184681
			wpatch.HookFixTopology();
//watje 10-4-99 184681
			int oldEdges = pmesh.numEdges;


			// We've deleted everything that wasn't selected -- Now add this to the patch object accumulator
			int oldVerts = pmesh.numVerts;
			int oldVecs = pmesh.numVecs;
			int oldPatches = pmesh.numPatches;

//watje 10-4-99 184681
			for (int hi = 0; hi < wpatch.hooks.Count(); hi++)
				{
				HookPoint tempHook = wpatch.hooks[hi];
				tempHook.upperPoint += oldVerts;
				tempHook.lowerPoint += oldVerts;
				tempHook.hookPoint += oldVerts;
				tempHook.upperVec += oldVecs;
				tempHook.lowerVec += oldVecs;
				tempHook.upperHookVec += oldVecs;
				tempHook.lowerHookVec += oldVecs;
				tempHook.upperPatch += oldPatches; 
				tempHook.lowerPatch += oldPatches;
				tempHook.hookPatch += oldPatches;
				tempHook.hookEdge += oldEdges;
				tempHook.upperEdge += oldEdges;
				tempHook.lowerEdge += oldEdges;

				whooks.Append(1,&tempHook,1);

				}

			int newVerts = oldVerts + wpatch.numVerts;
			int newVecs = oldVecs + wpatch.numVecs;
			int newPatches = oldPatches + wpatch.numPatches;
			pmesh.setNumVerts(newVerts, TRUE);
			pmesh.setNumVecs(newVecs, TRUE);
			pmesh.setNumPatches(newPatches, TRUE);
			altered = holdNeeded = 1;
			Matrix3 tm(1);
			if(multipleObjects && !reorient)
				tm = nodes[i]->GetObjectTM(t);
			for(int jk = 0, i2 = oldVerts; jk < wpatch.numVerts; ++jk, ++i2) {
				pmesh.verts[i2] = wpatch.verts[jk];
				pmesh.verts[i2].p = pmesh.verts[i2].p * tm;
				}
			for(jk = 0, i2 = oldVecs; jk < wpatch.numVecs; ++jk, ++i2) {
				pmesh.vecs[i2] = wpatch.vecs[jk];
				pmesh.vecs[i2].p = pmesh.vecs[i2].p * tm;
				}
			for(jk = 0, i2 = oldPatches; jk < wpatch.numPatches; ++jk, ++i2) {
				Patch &p = wpatch.patches[jk];
				Patch &p2 = pmesh.patches[i2];
				p2 = p;
				for(int j = 0; j < p2.type; ++j) {	// Adjust vertices and interior vectors
					p2.v[j] += oldVerts;
					p2.interior[j] += oldVecs;
					}
				for(j = 0; j < (p2.type * 2); ++j)	// Adjust edge vectors
					p2.vec[j] += oldVecs;
				}
			// Now copy over mapping information
			int dmaps = pmesh.getNumMaps();
			int smaps = wpatch.getNumMaps();
			int maxMaps = dmaps > smaps ? dmaps : smaps;
			if(maxMaps != dmaps)
				pmesh.setNumMaps (maxMaps, TRUE);
			if(maxMaps != smaps)
				wpatch.setNumMaps(maxMaps, TRUE);
			// Then make sure any active maps are active in both:
			for(int chan = -NUM_HIDDENMAPS; chan < maxMaps; ++chan) {
				if(pmesh.mapPatches(chan) || wpatch.mapPatches(chan)) {
					if(!pmesh.mapPatches(chan)) MakeDummyMapPatches(chan, &pmesh);
					if(!wpatch.mapPatches(chan)) MakeDummyMapPatches(chan, &wpatch);
					}
				}
			for(chan = -NUM_HIDDENMAPS; chan < pmesh.getNumMaps(); ++chan) {
				if(chan < wpatch.getNumMaps()) {
					int oldTVerts = pmesh.getNumMapVerts(chan);
					int newTVerts = oldTVerts + wpatch.getNumMapVerts(chan);
					pmesh.setNumMapVerts (chan, newTVerts, TRUE);
					for(i = 0, i2 = oldTVerts; i < wpatch.getNumMapVerts(chan); ++i, ++i2)
						pmesh.mapVerts(chan)[i2] = wpatch.mapVerts(chan)[i];
					if(pmesh.mapPatches(chan)) {
						for(i = 0, i2 = oldPatches; i < wpatch.numPatches; ++i, ++i2) {
							Patch &p = wpatch.patches[i];
							TVPatch &tp = wpatch.mapPatches(chan)[i];
							TVPatch &tp2 = pmesh.mapPatches(chan)[i2];
							tp2 = tp;
							for(int j = 0; j < p.type; ++j)	// Adjust vertices
								{
								tp2.tv[j] += oldTVerts;
//watje 255081 to handle new TV handles and interiors 
								if (wpatch.ArePatchesCurvedMapped(i))
									{
									tp2.handles[j*2] += oldTVerts;
									tp2.handles[j*2+1] += oldTVerts;
									if (!(p.flags&PATCH_AUTO))
										tp2.interiors[j] += oldTVerts;
									}
								}
							}
						}
					}
				}
			patchData->UpdateChanges(patch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
//watje 10-4-99 184681
			pmesh.buildLinkages();
			}

		bail_out:
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		pmesh.computeInteriors();
		pmesh.buildLinkages();
//watje 10-4-99 184681
		pmesh.hooks = whooks;

		INode *newNode = ip->CreateObjectNode(patchOb);
		newNode->CopyProperties(nodes[0]);
		newNode->SetMtl(nodes[0]->GetMtl());
		newNode->SetName(newName.data());
		patchOb->patch.InvalidateGeomCache();
		if(!multipleObjects) {	// Single input object?
			if(!reorient) {
				Matrix3 tm = nodes[0]->GetObjectTM(t);
				newNode->SetNodeTM(t, tm);	// Use this object's TM.
				}
			}
		else {
			if(!reorient) {
				Matrix3 matrix;
				matrix.IdentityMatrix();
				newNode->SetNodeTM(t, matrix);	// Use identity TM
				}
			}
		newNode->FlagForeground(t);		// WORKAROUND!
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_DETACHPATCH));
		}
	else {
		delete patchOb;	// Didn't need it after all!
		if(!dialoged)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t,REDRAW_NORMAL);
	}

#if 0
// CCJ 8.15.00 - Removed this code when separating the project out of
// mods.dlm because this function is never called.
void EditPatchMod::SetTessUI(HWND hDlg, TessApprox *tess)
{
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), FALSE);
	CheckDlgButton( hDlg, IDC_TESS_SET, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_REGULAR, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_PARAM, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_SPATIAL, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_CURV, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_LDA, FALSE);

	ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_HIDE);

//watje 12-10-98
	if (tess->showInteriorFaces)
		CheckDlgButton( hDlg, IDC_SHOW_INTERIOR_FACES, TRUE);
	else CheckDlgButton( hDlg, IDC_SHOW_INTERIOR_FACES, FALSE);

	switch (tess->type) {
	case TESS_SET:
		CheckDlgButton( hDlg, IDC_TESS_SET, TRUE);
		mergeSpin->Disable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
		break;

	case TESS_REGULAR:
		CheckDlgButton( hDlg, IDC_TESS_REGULAR, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), TRUE);

		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), SW_HIDE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_PARAM:
		CheckDlgButton( hDlg, IDC_TESS_PARAM, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), TRUE);

		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_SPATIAL:
		CheckDlgButton( hDlg, IDC_TESS_SPATIAL, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_CURVE:
		CheckDlgButton( hDlg, IDC_TESS_CURV, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_LDA:
		CheckDlgButton( hDlg, IDC_TESS_LDA, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;
	}


	if (settingViewportTess) {
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_SET), SW_SHOW);

		if (tess->type != TESS_SET) {
			ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_SHOW);
			EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), !GetViewTessWeld());
			EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), tess->merge > 0.0f);
		}
	} else {
		if (settingDisp) {
			ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_SHOW);
		} else {
			if (tess->type != TESS_SET) {
				ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_SHOW);
				EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), !GetProdTessWeld());
				EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), tess->merge > 0.0f);
				CheckDlgButton( hDlg, IDC_MESH, TRUE);
			}
			ShowWindow(GetDlgItem(hDlg, IDC_TESS_SET), SW_SHOW);
		}
	}

	// now set all the settings
	uSpin->SetValue(tess->u, FALSE);
	vSpin->SetValue(tess->v, FALSE);
	edgeSpin->SetValue(tess->edge, FALSE);
	distSpin->SetValue(tess->dist, FALSE);
	angSpin->SetValue(tess->ang, FALSE);
	mergeSpin->SetValue(tess->merge, FALSE);
	CheckDlgButton( hDlg, IDC_TESS_VIEW_DEP, tess->view);
	if (settingViewportTess) {
		CheckDlgButton( hDlg, IDC_TESS_VIEW, TRUE);
		CheckDlgButton( hDlg, IDC_TESS_RENDERER, FALSE);
		CheckDlgButton( hDlg, IDC_TESS_NORMALS, GetViewTessNormals());
		CheckDlgButton( hDlg, IDC_WELDTESS, GetViewTessWeld());
	} else {
		CheckDlgButton( hDlg, IDC_TESS_VIEW, FALSE);
		CheckDlgButton( hDlg, IDC_TESS_RENDERER, TRUE);
		CheckDlgButton( hDlg, IDC_TESS_NORMALS, GetProdTessNormals());
		CheckDlgButton( hDlg, IDC_WELDTESS, GetProdTessWeld());
	}
	CheckDlgButton( hDlg, IDC_DISP, settingDisp);
}

#endif

int EditPatchMod::GetSelMatIndex()
	{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	int mat=-1;

	if (!ip) return -1;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
		if(!patch)
			continue;
		for (int j=0; j<patch->getNumPatches(); j++) {
			if (patch->patchSel[j]) {
				if (first) {
					first = FALSE;
					mat   = (int)patch->getPatchMtlIndex(j);					
				} else {
					if ((int)patch->getPatchMtlIndex(j) != mat) {
						return -1;
						}
					}
				}
			}
		}
	
	nodes.DisposeTemporary();
	return mat;
	}

void EditPatchMod::SetSelMatIndex(int index)
	{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		// Start a restore object...
		patchData->PreUpdateChanges(patch, FALSE);
		if (theHold.Holding()) {
			theHold.Put(new PatchRestore(patchData,this,patch,"SetSelMatIndex"));
			}

		for (int j=0; j<patch->getNumPatches(); j++) {			
			if (patch->patchSel[j]) {
				altered = holdNeeded = TRUE;
				patch->setPatchMtlIndex(j,(MtlID)index);			
				}
			}
		
		if(altered) {
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);		
		}	
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_PATCHMTLCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

void EditPatchMod::SelectByMat(int index,BOOL clear)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		// Start a restore object...
		patchData->PreUpdateChanges(patch, FALSE);
		if (theHold.Holding()) {
			theHold.Put(new PatchSelRestore(patchData,this,patch));
			}
		
		if (clear)
			patch->patchSel.ClearAll();

		for (int j=0; j<patch->getNumPatches(); j++) {			
			if (patch->getPatchMtlIndex(j)==index)
				patch->patchSel.Set(j);
			}
		
		patchData->UpdateChanges(patch, FALSE);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

DWORD EditPatchMod::GetSelSmoothBits(DWORD &invalid)
	{
	BOOL first = 1;
	DWORD bits = 0;
	invalid = 0;
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip) return 0;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		for (int j=0; j<patch->getNumPatches(); j++) {
			if (patch->patchSel[j]) {
				if (first) {
					first = FALSE;
					bits  = patch->patches[j].smGroup;					
				} else {
					if (patch->patches[j].smGroup != bits) {
						invalid |= patch->patches[j].smGroup^bits;
						}
					}
				}
			}

		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		
	nodes.DisposeTemporary();
	return bits;
	}

DWORD EditPatchMod::GetUsedSmoothBits()
	{	
	DWORD bits = 0;
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip) return 0;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		for (int j=0; j<patch->getNumPatches(); j++) {
			bits |= patch->patches[j].smGroup;
			}		

		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		
	nodes.DisposeTemporary();
	return bits;
	}

void EditPatchMod::SelectBySmoothGroup(DWORD bits,BOOL clear)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		// Start a restore object...
		patchData->PreUpdateChanges(patch, FALSE);
		if (theHold.Holding()) {
			theHold.Put(new PatchSelRestore(patchData,this,patch));
			}
		
		if (clear)
			patch->patchSel.ClearAll();			
		for (int j=0; j<patch->getNumPatches(); j++) {			
			if (patch->patches[j].smGroup & bits) {
				patch->patchSel.Set(j);			
				}
			}
		
		patchData->UpdateChanges(patch, FALSE);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SELECTBYSMOOTH));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

void EditPatchMod::SetSelSmoothBits(DWORD bits,DWORD which)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		// Start a restore object...
		patchData->PreUpdateChanges(patch, FALSE);
		if (theHold.Holding()) {
			theHold.Put(new PatchSelRestore(patchData,this,patch));
			}
		
		for (int j=0; j<patch->getNumPatches(); j++) {			
			if (patch->patchSel[j]) {
				patch->patches[j].smGroup &= ~which;
				patch->patches[j].smGroup |= bits&which;			
				}
			}
		
		patchData->UpdateChanges(patch, FALSE);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SETSMOOTHGROUP));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	InvalidateSurfaceUI();
	ip->RedrawViews(ip->GetTime());
	}

void EditPatchMod::PatchSelChanged() {
	SelectionChanged();
	if (hSurfPanel && (GetSubobjectType() == EP_PATCH || GetSubobjectType() == EP_VERTEX))
		InvalidateSurfaceUI();

	if ( UseSoftSelections() ) {
		UpdateVertexDists();
		UpdateEdgeDists();
		UpdateVertexWeights();
	}
	}

#ifdef USING_ADVANCED_APPROX
class AdvParams {
public:
	TessSubdivStyle mStyle;
	int mMin, mMax;
	int mTris;
};

static AdvParams sParams;

INT_PTR CALLBACK AdvParametersDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ); 
#endif //USING_ADVANCED_APPROX

// hack... see comment at top of file
//
void hackInvalidateOpsUI(HWND hOpsPanel)
{
	if(hOpsPanel && !s_bHackOpsUIValid) 
	{
		InvalidateRect (hOpsPanel, NULL, FALSE);
		s_bHackOpsUIValid = TRUE;
	}
}

// hack... see comment at top of file
//
void hackInvalidateSelUI(HWND hSelPanel)
{
	if(hSelPanel && !s_bHackSelUIValid) 
	{
		InvalidateRect (hSelPanel, NULL, FALSE);
		s_bHackSelUIValid = TRUE;
	}
}
// hack... see comment at top of file
//
void hackInvalidateSurfUI(HWND hSurfPanel)
{
	if(hSurfPanel && !s_bHackSurfUIValid) 
	{
		InvalidateRect (hSurfPanel, NULL, FALSE);
		s_bHackSurfUIValid = TRUE;
	}
}

// CAL-06/20/03: support spline surface generation (FID #1914)
// update UI controls based on the member data.
void EditPatchMod::UpdateGenSurfGroupControls()
{
	if (!hOpsPanel) return;

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_SURF_THRESHOLD_SPINNER));
	spin->SetValue(GetGenSurfWeldThreshold(), FALSE);
	ReleaseISpinner(spin);

	CheckDlgButton(hOpsPanel, IDC_EP_GENERATE_SURFACE, GetGenerateSurface());
	CheckDlgButton(hOpsPanel, IDC_EP_FLIP_NORMALS, GetGenSurfFlipNormals());
	CheckDlgButton(hOpsPanel, IDC_EP_RM_INTERIOR_PATCHES, GetGenSurfRmInterPatches());
	CheckDlgButton(hOpsPanel, IDC_EP_USE_SEL_SEGS, GetGenSurfUseOnlySelSegs());
}

// CAL-05/01/03: support spline surface generation (FID #1914)
void EditPatchMod::SetGenSurfGroupEnables()
{
	assert(ip);
	if(!hOpsPanel || !ip) return;
	
	BOOL splInput = FALSE;		// patch surface is generated from Splines
	BOOL genSurf = GetGenerateSurface();

	// check each local data and see if any one of them are generate from Spline
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( patchData && patchData->splineInput ) {
			splInput = TRUE;
			break;
		}
	}
	nodes.DisposeTemporary();

	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_SPLINE_SURFACE_LABEL), splInput);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_GENERATE_SURFACE), splInput);

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_SURF_THRESHOLD_SPINNER));
	spin->Enable(splInput && genSurf);
	ReleaseISpinner(spin);

	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_SURF_THRESHOLD_LABEL), splInput && genSurf);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_FLIP_NORMALS), splInput && genSurf);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_RM_INTERIOR_PATCHES), splInput && genSurf);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_USE_SEL_SEGS), splInput && genSurf);
}

void EditPatchMod::SetSelDlgEnables() {
	if(!hSelectPanel)
		return;

	BOOL hType  = (GetSubobjectType() == EP_HANDLE) ? TRUE : FALSE;
	BOOL vType  = (GetSubobjectType() == EP_VERTEX) ? TRUE : FALSE;
	BOOL eType  = (GetSubobjectType() == EP_EDGE) ? TRUE : FALSE;
	BOOL pLevel = (GetSubobjectLevel() == EP_PATCH) ? TRUE : FALSE;
	BOOL oType  = (GetSubobjectType() == EP_OBJECT) ? TRUE : FALSE;

	// can only copy in SO mode
	//
	ICustButton *copyButton = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_COPY));
	copyButton->Disable();
	copyButton->Enable(!oType);
	ReleaseICustButton(copyButton);

	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_PASTE));
	but->Disable();
	switch(GetSubobjectType()) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			if (GetPatchNamedSelClip(CLIP_P_HANDLE))
				but->Enable();
			break;
		case EP_VERTEX:
			if (GetPatchNamedSelClip(CLIP_P_VERT))
				but->Enable();
			break;
		case EP_EDGE:
			if (GetPatchNamedSelClip(CLIP_P_EDGE))
				but->Enable();
			break;
		case EP_PATCH:
			if (GetPatchNamedSelClip(CLIP_P_PATCH))
				but->Enable();
			break;
		}
	ReleaseICustButton(but);

	// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
	but = GetICustButton(GetDlgItem(hSelectPanel,IDC_EP_SELECTION_SHRINK));
	but->Enable(!oType && !hType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hSelectPanel,IDC_EP_SELECTION_GROW));
	but->Enable(!oType && !hType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hSelectPanel,IDC_EP_EDGE_RING_SEL));
	but->Enable(eType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hSelectPanel,IDC_EP_EDGE_LOOP_SEL));
	but->Enable(eType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hSelectPanel,IDC_SELECT_OPEN_EDGES));
	but->Enable(eType);
	ReleaseICustButton(but);
	EnableWindow (GetDlgItem (hSelectPanel, IDC_BY_VERTEX), hType || eType || pLevel);
	EnableWindow (GetDlgItem (hSelectPanel, IDC_IGNORE_BACKFACING), !oType);

	EnableWindow (GetDlgItem (hSelectPanel, IDC_EDPATCH_SELECT_NAMEDSEL_LABEL), !oType);
	EnableWindow (GetDlgItem (hSelectPanel, IDC_EDPATCH_SELECT_FILTER_LABEL), vType );
	EnableWindow (GetDlgItem (hSelectPanel, IDC_FILTVERTS), vType && (filterVecs ? true : false));
	EnableWindow (GetDlgItem (hSelectPanel, IDC_FILTVECS), vType && (filterVerts ? true : false));
	EnableWindow (GetDlgItem (hSelectPanel, IDC_LOCK_HANDLES), vType || hType );
	
	// invalidate UI, labels have changed
	//
	s_bHackSelUIValid = FALSE;

	}

void EditPatchMod::SetOpsDlgEnables() {
	if(!hOpsPanel)
		return;
	
	assert(ip);
	
	// Disconnect right-click and delete mechanisms
	ip->GetRightClickMenuManager()->Unregister(&pMenu);
    ip->GetActionManager()->DeactivateActionTable(&sActionCallback, kPatchActions);
	ip->UnRegisterDeleteUser(&pDel);

	BOOL oType = (GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL hType = (GetSubobjectLevel() == EP_HANDLE) ? TRUE : FALSE;
	BOOL vType = (GetSubobjectLevel() == EP_VERTEX) ? TRUE : FALSE;
	BOOL eType = (GetSubobjectLevel() == EP_EDGE) ? TRUE : FALSE;
	BOOL pType = (GetSubobjectType() == EP_PATCH) ? TRUE : FALSE;
	BOOL vhType = (vType || hType) ? TRUE : FALSE;
	BOOL vpType = (vType || pType) ? TRUE : FALSE;
	BOOL veType = (vType || eType) ? TRUE : FALSE;
	BOOL epType = (eType || pType) ? TRUE : FALSE;
	BOOL vepType = (vType || eType || pType) ? TRUE : FALSE;
	BOOL singleEdges = SingleEdgesOnly();
	BOOL extrudeOK = (pType || eType) ? TRUE : FALSE;

	BOOL elmType = (GetSubobjectLevel() == PO_ELEMENT) ? TRUE : FALSE;
	BOOL peeType = (elmType || eType || pType) ? TRUE : FALSE;
	BOOL pelmType = (elmType || pType) ? TRUE : FALSE;

	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_SUBDIVISION_LABEL), !oType && !hType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_WELD_LABEL),        veType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_VERTWELDHITRAD),                veType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_PIXELS_LABEL),      veType);

	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_EXBEV_LABEL),       peeType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_EXTRUSION_LABEL),   peeType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_OUTLINING_LABEL),   pelmType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_NORMAL_LABEL),      peeType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EM_EXTYPE_A),      peeType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EM_EXTYPE_B),      peeType);

	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_BEVELSMOOTHING_LABEL), pelmType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_BEVSTART_LABEL),    pelmType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_BEVEND_LABEL),      pelmType);
	// EnableWindow (GetDlgItem (hOpsPanel, IDC_EDPATCH_OPS_MISC_LABEL),        eType);

	// invalidate UI, labels have changed
	//
	s_bHackOpsUIValid = FALSE;
	ICustButton *but;

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_BIND));
	but->Enable (vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_UNBIND));
	but->Enable (vType);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_SUBDIVIDE));
	but->Enable (epType);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_PROPAGATE), epType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ADDTRI));
	but->Enable (eType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ADDQUAD));
	but->Enable (eType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_WELD));
	but->Enable (veType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_DETACH));
	but->Enable (pType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_BREAK));
	but->Enable (veType);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_DETACHREORIENT), pType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_DETACHCOPY), pType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_PATCH_DELETE));
	but->Enable (vepType);
	ReleaseICustButton (but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_THRESHSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_EP_CREATE));
	but->Enable (vpType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_VERTWELD));
	but->Enable (vType);
	ReleaseICustButton (but);

	// CAL-06/02/03: copy/paste tangent. (FID #827)
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_TANGENT_STATIC), vhType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_EP_COPY_TANGENT));
	but->Enable (vhType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_EP_PASTE_TANGENT));
	but->Enable (vhType && tangentCopied);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EP_COPY_TAN_LENGTH), vhType && tangentCopied);

//3-1-99 watje
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_HIDE));
	but->Enable (vepType);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_CREATE_SHAPE));
	but->Enable (eType);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_EP_EXTRUDE));
	but->Enable (extrudeOK);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_EP_BEVEL));
	but->Enable (pType);
	ReleaseICustButton (but);

	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_EXTRUDESPINNER));
	spin->Enable(extrudeOK);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_EP_OUTLINESPINNER));
	spin->Enable(pType);
	ReleaseISpinner(spin);

	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH4),pType);
	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH5),pType);
	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH6),pType);

	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH),pType);
	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH2),pType);
	EnableWindow(GetDlgItem(hOpsPanel,IDC_EP_SM_SMOOTH3),pType);

	EnableWindow(GetDlgItem(hOpsPanel,IDC_SHOW_INTERIOR_FACES), relax ? FALSE : TRUE);

	// Enable/disable right-click and delete mechanisms
	if(!oType) {			
		pMenu.SetMod(this);
		ip->GetRightClickMenuManager()->Register(&pMenu);
        sActionCallback.SetMod(this);
        ip->GetActionManager()->ActivateActionTable(&sActionCallback, kPatchActions);
		pDel.SetMod(this);
		ip->RegisterDeleteUser(&pDel);
		}
	}

void EditPatchMod::SetSurfDlgEnables() {
	if(!hSurfPanel)
		return;
	
	assert(ip);
	
	BOOL oType = (GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL pType = (GetSubobjectType() == EP_PATCH) ? TRUE : FALSE;

	if(oType)
		return;
	if(!pType)
		return;

	ICustButton *but;
	ISpinnerControl *spin;
	but = GetICustButton (GetDlgItem (hSurfPanel, IDC_SELECT_BYID));
	but->Enable (pType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hSurfPanel,IDC_MAT_IDSPIN));
	spin->Enable(pType);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hSurfPanel,IDC_MAT_IDSPIN_SEL));   
	spin->Enable(pType);
	ReleaseISpinner(spin);
	EnableWindow(GetDlgItem(hSurfPanel,IDC_CLEARSELECTION), TRUE);        
	for(int i = 0; i < 32; ++i) {
		but = GetICustButton (GetDlgItem (hSurfPanel, IDC_SMOOTH_GRP1+i));
		but->Enable (pType);
		ReleaseICustButton (but);
		}
	but = GetICustButton (GetDlgItem (hSurfPanel, IDC_SELECTBYSMOOTH));
	but->Enable (pType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hSurfPanel, IDC_SMOOTH_CLEAR));
	but->Enable (pType);
	ReleaseICustButton (but);
	}

/*-------------------------------------------------------------------*/

INT_PTR CALLBACK PatchSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditPatchMod *ep = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	ICustToolbar *iToolbar;
	if ( !ep && message != WM_INITDIALOG ) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG: {
		 	ep = (EditPatchMod *)lParam;
		 	ep->hSelectPanel = hDlg;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)ep );		 	
			// Set up the editing level selector
			LoadImages();
			iToolbar = GetICustToolbar(GetDlgItem(hDlg,IDC_SELTYPE));
			iToolbar->SetImage(hFaceImages);
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,EP_VERTEX));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,EP_HANDLE));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,EP_EDGE));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,EP_PATCH));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,EP_ELEMENT));
			ReleaseICustToolbar(iToolbar);
			ep->RefreshSelType();
			CheckDlgButton( hDlg, IDC_DISPLATTICE, ep->displayLattice);
//			CheckDlgButton( hDlg, IDC_DISPSURFACE, ep->displaySurface);
			CheckDlgButton( hDlg, IDC_FILTVERTS, filterVerts);
			CheckDlgButton( hDlg, IDC_FILTVECS, filterVecs);
			CheckDlgButton( hDlg, IDC_LOCK_HANDLES, lockedHandles);
			CheckDlgButton( hDlg, IDC_IGNORE_BACKFACING, ignoreBackfacing);
			CheckDlgButton( hDlg, IDC_BY_VERTEX, byVertex);
			ep->SetSelDlgEnables();
		 	return TRUE;
			}

		case WM_DESTROY:
			// Don't leave in one of our modes!
			ep->ip->ClearPickMode();
			CancelEditPatchModes(ep->ip);
			return FALSE;
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND: {
			BOOL needRedraw = FALSE;
			switch ( LOWORD(wParam) ) {
				// CAL-06/10/03: add handle sub-object mode. (FID #1914)
				case EP_HANDLE:
					if (ep->GetSubobjectLevel() == EP_HANDLE)
						ep->ip->SetSubObjectLevel (PO_OBJECT);
					else ep->ip->SetSubObjectLevel (EP_HANDLE);
					needRedraw = TRUE;
					break;

				case EP_VERTEX:
					if (ep->GetSubobjectLevel() == EP_VERTEX)
						ep->ip->SetSubObjectLevel (PO_OBJECT);
					else ep->ip->SetSubObjectLevel (EP_VERTEX);
					needRedraw = TRUE;
					break;

				case EP_EDGE:
					if (ep->GetSubobjectLevel() == EP_EDGE)
						ep->ip->SetSubObjectLevel (PO_OBJECT);
					else ep->ip->SetSubObjectLevel (EP_EDGE);
					needRedraw = TRUE;
					break;

				case EP_PATCH:
					if (ep->GetSubobjectLevel() == EP_PATCH)
						ep->ip->SetSubObjectLevel (PO_OBJECT);
					else ep->ip->SetSubObjectLevel (EP_PATCH);
					needRedraw = TRUE;
					break;

				case EP_ELEMENT:
					if (ep->GetSubobjectLevel() == EP_ELEMENT)
						ep->ip->SetSubObjectLevel (PO_OBJECT);
					else ep->ip->SetSubObjectLevel (EP_ELEMENT);
					needRedraw = TRUE;
					break;

				case IDC_DISPLATTICE:
					ep->SetDisplayLattice(IsDlgButtonChecked(hDlg, IDC_DISPLATTICE));
					needRedraw = TRUE;
					break;
				case IDC_DISPSURFACE:
					ep->SetDisplaySurface(IsDlgButtonChecked(hDlg, IDC_DISPSURFACE));
					needRedraw = TRUE;
					break;
				case IDC_FILTVERTS:
					filterVerts = IsDlgButtonChecked(hDlg, IDC_FILTVERTS);
					EnableWindow(GetDlgItem(hDlg,IDC_FILTVECS), filterVerts ? TRUE : FALSE);
					SetVertFilter();
					break;
				case IDC_FILTVECS:
					filterVecs = IsDlgButtonChecked(hDlg, IDC_FILTVECS);
					EnableWindow(GetDlgItem(hDlg,IDC_FILTVERTS), filterVecs ? TRUE : FALSE);
					SetVertFilter();
					break;
				case IDC_LOCK_HANDLES:
					lockedHandles = IsDlgButtonChecked( hDlg, IDC_LOCK_HANDLES);
					break;
				case IDC_IGNORE_BACKFACING:
					ignoreBackfacing = IsDlgButtonChecked( hDlg, IDC_IGNORE_BACKFACING);
					break;
				case IDC_BY_VERTEX:
					byVertex = IsDlgButtonChecked( hDlg, IDC_BY_VERTEX);
					needRedraw = TRUE;
					break;
				// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
				case IDC_EP_SELECTION_SHRINK:
					ep->ShrinkSelection(ep->GetSubobjectType());
					break;
				case IDC_EP_SELECTION_GROW:
					ep->GrowSelection(ep->GetSubobjectType());
					break;
				case IDC_EP_EDGE_RING_SEL:
					ep->SelectEdgeRing();
					break;
				case IDC_EP_EDGE_LOOP_SEL:
					ep->SelectEdgeLoop();
					break;
				case IDC_SELECT_OPEN_EDGES:
					ep->SelectOpenEdges();
					break;
				case IDC_NS_COPY:
					ep->NSCopy();
					break;
				case IDC_NS_PASTE:
					ep->NSPaste();
					break;
				}
			if(needRedraw) {
				ep->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ep->ip->RedrawViews(ep->ip->GetTime(),REDRAW_NORMAL);
				}
			}
			break;
		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == TTN_NEEDTEXT) {
				LPTOOLTIPTEXT lpttt;
				lpttt = (LPTOOLTIPTEXT)lParam;				
				switch (lpttt->hdr.idFrom) {
				// CAL-06/10/03: add handle sub-object mode. (FID #1914)
				case EP_HANDLE:
					lpttt->lpszText = GetString (IDS_TH_HANDLE);
					break;
				case EP_VERTEX:
					lpttt->lpszText = GetString (IDS_TH_VERTEX);
					break;
				case EP_EDGE:
					lpttt->lpszText = GetString (IDS_TH_EDGE);
					break;
				case EP_PATCH:
					lpttt->lpszText = GetString(IDS_TH_PATCH);
					break;
				case EP_ELEMENT:
					lpttt->lpszText = GetString(IDS_TH_ELEMENT);
					break;
				}
			}
			break;
			default:
				hackInvalidateSelUI(ep->hSelectPanel);
			return FALSE;


		}
	
	return FALSE;
	}

static void SetSmoothButtonState (HWND hWnd,DWORD bits,DWORD invalid,DWORD unused=0) {
	for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
		if ( (unused&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			ShowWindow(GetDlgItem(hWnd,i),SW_HIDE);
			continue;
		}

		if ( (invalid&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			SetWindowText(GetDlgItem(hWnd,i),NULL);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,FALSE);
		} else {
			TSTR buf;
			buf.printf(_T("%d"),i-IDC_SMOOTH_GRP1+1);
			SetWindowText(GetDlgItem(hWnd,i),buf);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,
				(bits&(1<<(i-IDC_SMOOTH_GRP1)))?TRUE:FALSE);
		}
		InvalidateRect(GetDlgItem(hWnd,i),NULL,TRUE);
	}
}

static INT_PTR CALLBACK SelectBySmoothDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static DWORD *param;
	switch (msg) {
	case WM_INITDIALOG:
		param = (DWORD*)lParam;
		int i;
		for (i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
		SetSmoothButtonState(hWnd,param[0],0,param[2]);
		CheckDlgButton(hWnd,IDC_CLEARSELECTION,param[1]);
		CenterWindow(hWnd,GetParent(hWnd));
		break;

	case WM_COMMAND: 
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));				
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;				
			if (iBut->IsChecked()) {
				param[0] |= 1<<shift;
			} else {
				param[0] &= ~(1<<shift);
			}				
			ReleaseICustButton(iBut);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDOK:
			param[1] = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION);					
			EndDialog(hWnd,1);					
			break;					

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;			

	default:
		return FALSE;
	}
	return TRUE;
}


INT_PTR CALLBACK PatchOpsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditPatchMod *ep = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !ep && message != WM_INITDIALOG ) return FALSE;

	
	ISpinnerControl *spin;
	ICustButton *ebut;

	switch ( message ) {
		case WM_INITDIALOG: {

		 	ep = (EditPatchMod *)lParam;
		 	ep->hOpsPanel = hDlg;
			for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
				SendMessage(GetDlgItem(hDlg,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)ep );		 	
			ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_ATTACH));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			CheckDlgButton( hDlg, IDC_ATTACHREORIENT, attachReorient);
			CheckDlgButton( hDlg, IDC_DETACHCOPY, patchDetachCopy);
			CheckDlgButton( hDlg, IDC_DETACHREORIENT, patchDetachReorient);
			CheckDlgButton( hDlg, IDC_PROPAGATE, ep->GetPropagate());

			// CAL-05/01/03: support spline surface generation (FID #1914)
			ep->genThreshSpin = GetISpinner(GetDlgItem(hDlg, IDC_EP_SURF_THRESHOLD_SPINNER));
			ep->genThreshSpin->SetLimits(EP_MIN_GENSURF_THRESH, EP_MAX_GENSURF_THRESH, FALSE);
			ep->genThreshSpin->LinkToEdit(GetDlgItem(hDlg, IDC_EP_SURF_THRESHOLD), EDITTYPE_POS_UNIVERSE);
			ep->genThreshSpin->SetValue(ep->GetGenSurfWeldThreshold(), FALSE);
			CheckDlgButton(hDlg, IDC_EP_GENERATE_SURFACE, ep->GetGenerateSurface());
			CheckDlgButton(hDlg, IDC_EP_FLIP_NORMALS, ep->GetGenSurfFlipNormals());
			CheckDlgButton(hDlg, IDC_EP_RM_INTERIOR_PATCHES, ep->GetGenSurfRmInterPatches());
			CheckDlgButton(hDlg, IDC_EP_USE_SEL_SEGS, ep->GetGenSurfUseOnlySelSegs());

		 	ep->stepsSpin = GetISpinner(GetDlgItem(hDlg,IDC_STEPSSPINNER));
			ep->stepsSpin->SetLimits( 0, 100, FALSE );
			ep->stepsSpin->LinkToEdit( GetDlgItem(hDlg,IDC_STEPS), EDITTYPE_POS_INT );
			ep->stepsSpin->SetValue(ep->GetMeshSteps(),FALSE);

#ifndef NO_OUTPUTRENDERER
			//3-18-99 to suport render steps and removal of the mental tesselator
		 	ep->stepsRenderSpin = GetISpinner(GetDlgItem(hDlg,IDC_STEPSRENDERSPINNER));
			ep->stepsRenderSpin->SetLimits( 0, 100, FALSE );
			ep->stepsRenderSpin->LinkToEdit( GetDlgItem(hDlg,IDC_STEPS_RENDER), EDITTYPE_POS_INT );
			ep->stepsRenderSpin->SetValue(ep->GetMeshStepsRender(),FALSE);
#endif // NO_OUTPUTRENDERER
			CheckDlgButton( hDlg, IDC_SHOW_INTERIOR_FACES, ep->GetShowInterior());
			// CAL-05/15/03: use true patch normals. (FID #1760)
			CheckDlgButton( hDlg, IDC_TRUE_PATCH_NORMALS, ep->GetUsePatchNormals());

		 	ep->weldSpin = GetISpinner(GetDlgItem(hDlg,IDC_THRESHSPINNER));
			ep->weldSpin->SetLimits( 0, 999999, FALSE );
			ep->weldSpin->LinkToEdit( GetDlgItem(hDlg,IDC_WELDTHRESH), EDITTYPE_UNIVERSE );
			ep->weldSpin->SetValue(weldThreshold,FALSE);

		 	ep->vertWeldSpin = GetISpinner(GetDlgItem(hDlg,IDC_VERTHITRADSPINNER));
			ep->vertWeldSpin->SetLimits( 1, 1000, FALSE );
			ep->vertWeldSpin->LinkToEdit( GetDlgItem(hDlg,IDC_VERTWELDHITRAD), EDITTYPE_POS_INT );
			ep->vertWeldSpin->SetValue(ep->weldBoxSize,FALSE);

			CheckDlgButton( hDlg, IDC_EM_EXTYPE_B, TRUE);
			CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH, TRUE);
			CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH4, TRUE);

			ep->inExtrude  = FALSE;
			ep->inBevel  = FALSE;

		// Set up spinners
			spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_EXTRUDESPINNER));
			spin->SetLimits(-9999999, 9999999, FALSE);
			spin->LinkToEdit (GetDlgItem (hDlg,IDC_EP_EXTRUDEAMOUNT), EDITTYPE_UNIVERSE);
			ReleaseISpinner (spin);

			spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_OUTLINESPINNER));
			spin->SetLimits(-9999999, 9999999, FALSE);
			spin->LinkToEdit (GetDlgItem (hDlg,IDC_EP_OUTLINEAMOUNT), EDITTYPE_UNIVERSE);
			ReleaseISpinner (spin);


			ebut = GetICustButton(GetDlgItem(hDlg,IDC_EP_EXTRUDE));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg,IDC_EP_BEVEL));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg,IDC_EP_CREATE));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg,IDC_VERTWELD));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg,IDC_BIND));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			// CAL-06/02/03: copy/paste tangent. (FID #827)
			ebut = GetICustButton(GetDlgItem(hDlg,IDC_EP_COPY_TANGENT));
			ebut->SetHighlightColor(GREEN_WASH);
			ebut->SetType(CBT_CHECK);
			ReleaseICustButton(ebut);
			ebut = GetICustButton(GetDlgItem(hDlg,IDC_EP_PASTE_TANGENT));
			ebut->SetHighlightColor(GREEN_WASH);
			ebut->SetType(CBT_CHECK);
			ReleaseICustButton(ebut);
			CheckDlgButton(hDlg, IDC_EP_COPY_TAN_LENGTH, ep->copyTanLength);

			ep->matSpin = SetupIntSpinner(hDlg,IDC_MAT_IDSPIN,IDC_MAT_ID,1,MAX_MATID,0);
		 	ep->SetOpsDlgEnables();
			ep->SetGenSurfGroupEnables();	// CAL-05/01/03:
			return TRUE;
			}

		case REFRESH_EP_VALUES:
			ep->stepsSpin->SetValue(ep->GetMeshSteps(),FALSE);
#ifndef NO_OUTPUTRENDERER
			ep->stepsRenderSpin->SetValue(ep->GetMeshStepsRender(),FALSE);
#endif // NO_OUTPUTRENDERER
			CheckDlgButton( hDlg, IDC_SHOW_INTERIOR_FACES, ep->GetShowInterior());
			// CAL-05/15/03: use true patch normals. (FID #1760)
			CheckDlgButton( hDlg, IDC_TRUE_PATCH_NORMALS, ep->GetUsePatchNormals());
			return TRUE;

		case REFRESH_EP_GENSURF:
			// CAL-05/01/03: support spline surface generation (FID #1914)
			ep->SetGenSurfGroupEnables();
			break;

		case WM_DESTROY:
			if( ep->weldSpin ) {
				ReleaseISpinner(ep->weldSpin);
				ep->weldSpin = NULL;
				}
			if( ep->vertWeldSpin ) {
				ReleaseISpinner(ep->vertWeldSpin);
				ep->vertWeldSpin = NULL;
				}
			if( ep->stepsSpin ) {
				ReleaseISpinner(ep->stepsSpin);
				ep->stepsSpin = NULL;
				}
#ifndef NO_OUTPUTRENDERER
//3-18-99 to suport render steps and removal of the mental tesselator
			if( ep->stepsRenderSpin ) {
				ReleaseISpinner(ep->stepsRenderSpin);
				ep->stepsRenderSpin = NULL;
				}
#endif // NO_OUTPUTRENDERER

			// Don't leave in one of our modes!
			ep->ip->ClearPickMode();
			CancelEditPatchModes(ep->ip);
			ep->ip->UnRegisterDeleteUser(&pDel);
			ep->ip->GetRightClickMenuManager()->Unregister(&pMenu);
            ep->ip->GetActionManager()->DeactivateActionTable(&sActionCallback, kPatchActions);
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				// CAL-05/01/03: support spline surface generation (FID #1914)
				case IDC_EP_SURF_THRESHOLD_SPINNER:
					ep->SetGenSurfWeldThreshold(ep->genThreshSpin->GetFVal());
					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					ep->ip->RedrawViews(ep->ip->GetTime(),REDRAW_NORMAL);
					break;

				case IDC_STEPSSPINNER:
					ep->SetMeshSteps(ep->stepsSpin->GetIVal());
					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					ep->ip->RedrawViews(ep->ip->GetTime(),REDRAW_NORMAL);
					break;
#ifndef NO_OUTPUTRENDERER
				case IDC_STEPSRENDERSPINNER:
					ep->SetMeshStepsRender(ep->stepsRenderSpin->GetIVal());
					break;
#endif // NO_OUTPUTRENDERER

				case IDC_THRESHSPINNER:
					weldThreshold = ep->weldSpin->GetFVal();
					break;
				case IDC_VERTHITRADSPINNER:
					ep->weldBoxSize = ep->vertWeldSpin->GetIVal();
					break;
				case IDC_EP_EXTRUDESPINNER:
					{
					// NOTE: In future, we should use WM_CUSTEDIT_ENTER
					// to complete the keyboard entry, not this enterKey variable.
					// (See Editable Poly for example.)
					bool enterKey;
					enterKey = FALSE;
					if (!HIWORD(wParam) && !ep->inExtrude) {
						enterKey = TRUE;
						ep->BeginExtrude(ep->ip->GetTime());
						}
					BOOL ln = IsDlgButtonChecked(hDlg,IDC_EM_EXTYPE_B);
					spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_EXTRUDESPINNER));

					ep->Extrude (ep->ip->GetTime(),spin->GetFVal(),ln);
					if (enterKey) {
						ep->EndExtrude (ep->ip->GetTime(),TRUE);
						spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_EXTRUDESPINNER));
						if (spin) {
							spin->SetValue(0,FALSE);
							ReleaseISpinner(spin);
							}

						ep->ip->RedrawViews (ep->ip->GetTime(), REDRAW_END);
						} else {
						ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_INTERACTIVE);
						}
					break;
					}
				case IDC_EP_OUTLINESPINNER:
					{
					// NOTE: In future, we should use WM_CUSTEDIT_ENTER
					// to complete the keyboard entry, not this enterKey variable.
					// (See Editable Poly for example.)
					bool enterKey;
					enterKey = FALSE;
					if (!HIWORD(wParam) && !ep->inBevel) {
						enterKey = TRUE;
						ep->BeginBevel (ep->ip->GetTime ());
						}
					int sm =0;
					int sm2 = 0;
					if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH)) sm = 0;					
					else if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH2)) sm = 1;					
					else if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH3)) sm = 2;					

					if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH4)) sm2 = 0;					
					else if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH5)) sm2 = 1;					
					else if (IsDlgButtonChecked(hDlg,IDC_EP_SM_SMOOTH6)) sm2 = 2;					

					spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_OUTLINESPINNER));
					ep->Bevel (ep->ip->GetTime (), spin->GetFVal (),sm,sm2);
					if (enterKey) {
						ep->EndBevel (ep->ip->GetTime (), TRUE);
						spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_OUTLINESPINNER));
						if (spin) {
							spin->SetValue(0,FALSE);
							ReleaseISpinner(spin);
							}

						ep->ip->RedrawViews (ep->ip->GetTime(), REDRAW_END);
						} else {
						ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_INTERACTIVE);
						}
					break;
					}

				}
			break;
		case CC_SPINNER_BUTTONDOWN:
			switch (LOWORD(wParam)) {
			case IDC_EP_EXTRUDESPINNER:
				ep->BeginExtrude (ep->ip->GetTime());
				break;
			case IDC_EP_OUTLINESPINNER:
				ep->BeginBevel (ep->ip->GetTime ());
				break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_EP_EXTRUDESPINNER:
					ep->EndExtrude (ep->ip->GetTime(), HIWORD(wParam));
					spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_EXTRUDESPINNER));
					if (spin) {
						spin->SetValue(0,FALSE);
						ReleaseISpinner(spin);
						}

					ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_END);
					break;
				case IDC_EP_OUTLINESPINNER:
					ep->EndBevel (ep->ip->GetTime(), HIWORD(wParam));
					spin = GetISpinner(GetDlgItem(hDlg,IDC_EP_OUTLINESPINNER));
					if (spin) {
						spin->SetValue(0,FALSE);
						ReleaseISpinner(spin);
						}

					ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_END);
					break;


				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {
				// CAL-05/01/03: support spline surface generation (FID #1914)
				case IDC_EP_GENERATE_SURFACE:
					ep->SetGenerateSurface(IsDlgButtonChecked(hDlg, IDC_EP_GENERATE_SURFACE));
					ep->SetGenSurfGroupEnables();
					break;
				case IDC_EP_FLIP_NORMALS:
					ep->SetGenSurfFlipNormals(IsDlgButtonChecked(hDlg, IDC_EP_FLIP_NORMALS));
					break;
				case IDC_EP_RM_INTERIOR_PATCHES:
					ep->SetGenSurfRmInterPatches(IsDlgButtonChecked(hDlg, IDC_EP_RM_INTERIOR_PATCHES));
					break;
				case IDC_EP_USE_SEL_SEGS:
					ep->SetGenSurfUseOnlySelSegs(IsDlgButtonChecked(hDlg, IDC_EP_USE_SEL_SEGS));
					break;

				// Subdivision
//watje 3-18-99
				case IDC_SHOW_INTERIOR_FACES:
					theHold.Begin();
					ep->SetShowInterior(IsDlgButtonChecked(hDlg, IDC_SHOW_INTERIOR_FACES));
					if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
//					ep->InvalidateMesh();
//					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
//					ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_END);
						break;
				// CAL-05/15/03: use true patch normals. (FID #1760)
				case IDC_TRUE_PATCH_NORMALS:
					theHold.Begin();
					ep->SetUsePatchNormals(IsDlgButtonChecked(hDlg, IDC_TRUE_PATCH_NORMALS));
					if (theHold.Holding()) theHold.Accept(GetString(IDS_PARAM_CHANGE));
					break;

//watje 12-10-98
				case IDC_HIDE:
					ep->DoHide(ep->GetSubobjectType());
					break;
				case IDC_UNHIDE:
					ep->DoUnHide();
					break;
				case IDC_CREATE_SHAPE:
					ep->DoCreateShape();
					break;
				case IDC_BIND:
//			ep->DoAddHook();
					if (ep->ip->GetCommandMode()==ep->bindMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->bindMode);
					break;

					break;
				case IDC_UNBIND:
					ep->DoRemoveHook();
					break;
//extrude and bevel stuff
//watje 12-10-98
				case IDC_EP_SM_SMOOTH:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH2, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH3, FALSE);
					break;
				case IDC_EP_SM_SMOOTH2:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH3, FALSE);
					break;
				case IDC_EP_SM_SMOOTH3:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH2, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH, FALSE);
					break;

				case IDC_EP_SM_SMOOTH4:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH5, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH6, FALSE);
					break;
				case IDC_EP_SM_SMOOTH5:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH4, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH6, FALSE);
					break;
				case IDC_EP_SM_SMOOTH6:
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH4, FALSE);
					CheckDlgButton( hDlg, IDC_EP_SM_SMOOTH5, FALSE);
					break;


				case IDC_EP_EXTRUDE:
					if (ep->ip->GetCommandMode()==ep->extrudeMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->extrudeMode);
					break;
				case IDC_EP_BEVEL:
					if (ep->ip->GetCommandMode()==ep->bevelMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->bevelMode);
					break;

				// CAL-04/23/03: Patch Smooth
				case IDC_EP_PATCH_SMOOTH:
					ep->DoPatchSmooth(ep->GetSubobjectType());
					break;

				case IDC_EP_CREATE:
					switch (ep->GetSubobjectLevel()) {
					case EP_OBJECT:
					case EP_EDGE:
					case EP_HANDLE:
						break;
					case EP_VERTEX:
						if (ep->ip->GetCommandMode()==ep->createVertMode)
							ep->ip->SetStdCommandMode(CID_OBJMOVE);
						else ep->ip->SetCommandMode(ep->createVertMode);
						break;
					case EP_PATCH:
					case EP_ELEMENT:
						if (ep->ip->GetCommandMode()==ep->createPatchMode)
							ep->ip->SetStdCommandMode(CID_OBJMOVE);
						else ep->ip->SetCommandMode(ep->createPatchMode);
						break;
					default:
						break;
					}
					break;

				case IDC_VERTWELD:
					if (ep->ip->GetCommandMode()==ep->vertWeldMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->vertWeldMode);
					break;

				// CAL-06/02/03: copy/paste tangent. (FID #827)
				case IDC_EP_COPY_TANGENT:
					ep->StartCopyTangentMode();
					break;
				case IDC_EP_PASTE_TANGENT:
					ep->StartPasteTangentMode();
					break;
				case IDC_EP_COPY_TAN_LENGTH:
					ep->copyTanLength = IsDlgButtonChecked(hDlg, IDC_EP_COPY_TAN_LENGTH);
					break;

				case IDC_SUBDIVIDE:
					ep->DoSubdivide(ep->GetSubobjectType());
					break;
				case IDC_PROPAGATE:
					ep->SetPropagate(IsDlgButtonChecked(hDlg, IDC_PROPAGATE));
					break;
				// Topology
				case IDC_ADDTRI:
					if(ep->GetSubobjectLevel() == PO_EDGE)
						ep->DoPatchAdd(PATCH_TRI);
					break;
				case IDC_ADDQUAD:
					if(ep->GetSubobjectLevel() == PO_EDGE)
						ep->DoPatchAdd(PATCH_QUAD);
					break;
				case IDC_WELD:
					if(ep->GetSubobjectLevel() == PO_EDGE)
						ep->DoEdgeWeld();
					else
						ep->DoVertWeld();
					break;
				case IDC_DETACH:
					ep->DoPatchDetach(patchDetachCopy, patchDetachReorient);
					break;
				case IDC_DETACHCOPY:
					patchDetachCopy = IsDlgButtonChecked( hDlg, IDC_DETACHCOPY);
					break;
				case IDC_DETACHREORIENT:
					patchDetachReorient = IsDlgButtonChecked( hDlg, IDC_DETACHREORIENT);
					break;
				case IDC_ATTACH: {
					ModContextList mcList;
					INodeTab nodes;
					// If the mode is on, turn it off and bail
					if (ep->ip->GetCommandMode()->ID() == CID_STDPICK) {
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
						return FALSE;
						}
					// Want to turn on the mode.  Make sure we're valid first
					ep->ip->GetModContexts(mcList,nodes);
					ep->pickCB.ep = ep;
					ep->ip->SetPickMode(&ep->pickCB);
					nodes.DisposeTemporary();
					break;
					}
				case IDC_ATTACHREORIENT:
					attachReorient = IsDlgButtonChecked( hDlg, IDC_ATTACHREORIENT);
					break;
				case IDC_PATCH_DELETE:
					ep->DoDeleteSelected();
					break;
				case IDC_BREAK:
					ep->DoBreak();
					break;
				}
			break;
			default:
				hackInvalidateOpsUI(ep->hOpsPanel);
			return FALSE;

		}
	
	return FALSE;
	}

INT_PTR CALLBACK PatchSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditPatchMod *ep = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !ep && message != WM_INITDIALOG ) return FALSE;
	
	ISpinnerControl *spin;
	IColorSwatch *iCol;
	float fval;
	bool differs;
	static Color clr(0,0,0);

	switch ( message ) {
		case WM_INITDIALOG: {

		 	ep = (EditPatchMod *)lParam;
		 	ep->hSurfPanel = hDlg;
			for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
				SendMessage(GetDlgItem(hDlg,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)ep );		 	
			ep->matSpin = SetupIntSpinner(hDlg,IDC_MAT_IDSPIN,IDC_MAT_ID,1,MAX_MATID,0);
			ep->matSpinSel = SetupIntSpinner (hDlg, IDC_MAT_IDSPIN_SEL, IDC_MAT_ID_SEL, 1, MAX_MATID, 0);  
			CheckDlgButton(hDlg, IDC_CLEARSELECTION, 1);                                 
			SetupMtlSubNameCombo (hDlg, ep);           

			iCol = GetIColorSwatch(GetDlgItem(hDlg,IDC_VERT_COLOR),
				ep->GetPatchColor(), GetString(IDS_RB_VERTEXCOLOR));
			ReleaseIColorSwatch(iCol);
			iCol = GetIColorSwatch(GetDlgItem(hDlg,IDC_VERT_ILLUM),
				ep->GetPatchColor(MAP_SHADING), GetString(IDS_EM_VERTEXILLUM));
			ReleaseIColorSwatch(iCol);
			fval = ep->GetPatchColor(MAP_ALPHA, &differs).r * 100.0f;
			spin = SetupFloatSpinner (hDlg, IDC_VERT_ALPHA_SPIN, IDC_VERT_ALPHA,
				0.0f, 100.0f, fval, .1f);
			spin->SetIndeterminate (differs);
			ReleaseISpinner (spin);

		 	ep->SetSurfDlgEnables();
			ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_NORMAL_FLIPMODE));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			return TRUE;
			}

		case WM_DESTROY:
			if( ep->matSpin ) {
				ReleaseISpinner(ep->matSpin);
				ep->matSpin = NULL;
				}
			if( ep->matSpinSel ) {
				ReleaseISpinner(ep->matSpinSel);
				ep->matSpinSel = NULL;
				}
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;
			switch ( LOWORD(wParam) ) {
				case IDC_MAT_IDSPIN: 
					if(HIWORD(wParam))
						break;		// No interactive action
					ep->SetSelMatIndex(ep->matSpin->GetIVal()-1);
					break;

				case IDC_VERT_ALPHA_SPIN:
					if (!theHold.Holding()) theHold.Begin ();
					clr.r = clr.g = clr.b = spin->GetFVal()/100.0f;
					ep->SetPatchColor (clr, MAP_ALPHA);
					break;
				}
			break;

		case CC_SPINNER_BUTTONDOWN:
			switch (LOWORD (wParam)) {
			case IDC_VERT_ALPHA_SPIN:
				theHold.Begin ();
				break;
			}
			break;

		case WM_CUSTEDIT_ENTER:
			switch( LOWORD(wParam) ) {
				case IDC_VERT_ALPHA:
					theHold.Accept (GetString(IDS_EM_CHANGE_ALPHA));
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_MAT_IDSPIN:
					ep->SetSelMatIndex(ep->matSpin->GetIVal()-1);
					ep->ip->RedrawViews(ep->ip->GetTime(),REDRAW_END);
					break;

				case IDC_VERT_ALPHA_SPIN:
					if (HIWORD(wParam)) theHold.Accept (GetString(IDS_EM_CHANGE_ALPHA));
					else theHold.Cancel();
					break;
				}
			break;

		case WM_PAINT:
			if (!ep->patchUIValid) {
				// Material index
				int mat = ep->GetSelMatIndex();
				if (mat == -1) {
					ep->matSpin->SetIndeterminate(TRUE);
					ep->matSpinSel->SetIndeterminate(TRUE);    
				} else {
					ep->matSpin->SetIndeterminate(FALSE);
					ep->matSpin->SetValue(mat+1,FALSE);
					ep->matSpinSel->SetIndeterminate(FALSE);       
					ep->matSpinSel->SetValue(mat+1,FALSE);         
					}
				if (GetDlgItem (hDlg, IDC_MTLID_NAMES_COMBO)) {    
					ValidateUINameCombo(hDlg, ep);                   
				}          
				// Smoothing groups
				DWORD invalid, bits;
				bits = ep->GetSelSmoothBits(invalid);
				SetSmoothButtonState(hDlg,bits,invalid);
   				// Vertex color
				iCol = GetIColorSwatch (GetDlgItem(hDlg,IDC_VERT_COLOR),
					ep->GetPatchColor(), GetString(IDS_RB_VERTEXCOLOR));
				ReleaseIColorSwatch(iCol);

				iCol = GetIColorSwatch (GetDlgItem(hDlg,IDC_VERT_ILLUM),
					ep->GetPatchColor(MAP_SHADING), GetString(IDS_EM_VERTEXILLUM));
				ReleaseIColorSwatch(iCol);

				fval = ep->GetPatchColor(MAP_ALPHA, &differs).r * 100.0f;
				spin = GetISpinner (GetDlgItem (hDlg, IDC_VERT_ALPHA_SPIN));
				spin->SetIndeterminate (differs);
				spin->SetValue (fval, FALSE);
				ReleaseISpinner (spin);

				ep->patchUIValid = TRUE;
				}
			return FALSE;

		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;

		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept (GetString(IDS_RB_SETVERTCOLOR));
			else theHold.Cancel();
			break;

		case CC_COLOR_CHANGE:
			iCol = (IColorSwatch*)lParam;
			switch (LOWORD(wParam)) {
			case IDC_VERT_COLOR:
				ep->SetPatchColor (Color(iCol->GetColor()));
				break;
			case IDC_VERT_ILLUM:
				ep->SetPatchColor (Color(iCol->GetColor()), MAP_SHADING);
				break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
				LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
				ICustButton *iBut = GetICustButton(GetDlgItem(hDlg,LOWORD(wParam)));
				int bit = iBut->IsChecked() ? 1 : 0;
				int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
				ep->SetSelSmoothBits(bit<<shift,1<<shift);
				ReleaseICustButton(iBut);
				break;
			}
			switch ( LOWORD(wParam) ) {				
				// Normals
				case IDC_NORMAL_FLIP:
					ep->DoFlipNormals();
					break;
				case IDC_NORMAL_UNIFY:
					ep->DoUnifyNormals();
					break;
				case IDC_NORMAL_FLIPMODE:
					if (ep->ip->GetCommandMode()==ep->normalFlipMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else
						ep->ip->SetCommandMode(ep->normalFlipMode);
					break;
				// Material
				case IDC_SELECT_BYID:										
					ep->SelectByMat(ep->matSpinSel->GetIVal()-1/*index*/, IsDlgButtonChecked(hDlg,IDC_CLEARSELECTION)/*clear*/);
					break;
				case IDC_MTLID_NAMES_COMBO:          
					switch(HIWORD(wParam)){
					case CBN_SELENDOK:
						int index, val;
						index = SendMessage(GetDlgItem(hDlg,IDC_MTLID_NAMES_COMBO), CB_GETCURSEL, 0, 0);
						val = SendMessage(GetDlgItem(hDlg, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
						if (index != CB_ERR){
								ep->SelectByMat(val/*index*/, IsDlgButtonChecked(hDlg,IDC_CLEARSELECTION)/*clear*/);
						}
					break;                                                    
				}
				break;
				
				 // Smoothing groups
				case IDC_SELECTBYSMOOTH: {										
					sbsParams[2] = ~ep->GetUsedSmoothBits();
					if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EM_SELECTBYSMOOTH),
								ep->ip->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)sbsParams)) {
						ep->SelectBySmoothGroup(sbsParams[0],(BOOL)sbsParams[1]);
					}
					break;
					}
				case IDC_SMOOTH_CLEAR:
					ep->SetSelSmoothBits(0,0xffffffff);
					break;
				}
			break;
			default:
				hackInvalidateSurfUI(ep->hSurfPanel);
			return FALSE;

		}
	
	return FALSE;
	}

static void CheckSelectBy(HWND hDlg) {
	CheckDlgButton( hDlg, IDC_BY_COLOR, (selBy==SEL_BY_COLOR) ? TRUE : FALSE);
	CheckDlgButton( hDlg, IDC_BY_ILLUM, (selBy==SEL_BY_ILLUM) ? TRUE : FALSE);
	}

INT_PTR CALLBACK PatchVertSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditPatchMod *ep = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
			// WIN64 Cleanup: Shuler
	if ( !ep && message != WM_INITDIALOG ) return FALSE;

	ISpinnerControl *spin;
	IColorSwatch *iCol;
	COLORREF rgb;
	float fval;
	bool differs;
	static Color clr(0,0,0);

	switch ( message ) {
		case WM_INITDIALOG: {
		 	ep = (EditPatchMod *)lParam;
		 	ep->hSurfPanel = hDlg;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (INT_PTR)ep );		 	
					// WIN64 Cleanup: Shuler
			CheckSelectBy(hDlg);
			SetupIntSpinner (hDlg, IDC_VERT_SELRSPIN, IDC_VERT_SELR, 0, 255, selDeltaR);
			SetupIntSpinner (hDlg, IDC_VERT_SELGSPIN, IDC_VERT_SELG, 0, 255, selDeltaG);
			SetupIntSpinner (hDlg, IDC_VERT_SELBSPIN, IDC_VERT_SELB, 0, 255, selDeltaB);

			// Select-by-color swatch.
			rgb = RGB(int(selByColor.x*255.0),int(selByColor.y*255.0),int(selByColor.z*255.0));
			iCol = GetIColorSwatch (GetDlgItem(hDlg,IDC_VERT_SELCOLOR), rgb,
				GetString((selBy==SEL_BY_COLOR) ?IDS_RB_SELBYCOLOR : IDS_SELBYILLUM));
			ReleaseIColorSwatch(iCol);

			iCol = GetIColorSwatch(GetDlgItem(hDlg,IDC_VERT_COLOR),
				ep->GetVertColor(), GetString(IDS_RB_VERTEXCOLOR));
			ReleaseIColorSwatch(iCol);
			iCol = GetIColorSwatch(GetDlgItem(hDlg,IDC_VERT_ILLUM),
				ep->GetVertColor(MAP_SHADING), GetString(IDS_EM_VERTEXILLUM));
			ReleaseIColorSwatch(iCol);

			fval = ep->GetVertColor(MAP_ALPHA, &differs).r * 100.0f;
			spin = SetupFloatSpinner (hDlg, IDC_VERT_ALPHA_SPIN,
				IDC_VERT_ALPHA, 0.0f, 100.0f, fval, .1f);
			spin->SetIndeterminate (differs);
			ReleaseISpinner (spin);

			ep->SetSurfDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			switch (LOWORD (wParam)) {
			case IDC_VERT_ALPHA_SPIN:
				theHold.Begin ();
				break;
			}
			break;

		case WM_CUSTEDIT_ENTER:
			switch( LOWORD(wParam) ) {
				case IDC_VERT_ALPHA:
					theHold.Accept (GetString(IDS_EM_CHANGE_ALPHA));
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_VERT_ALPHA_SPIN:
					if (HIWORD(wParam)) theHold.Accept (GetString(IDS_EM_CHANGE_ALPHA));
					else theHold.Cancel();
					break;
				}
			break;

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;

			switch ( LOWORD(wParam) ) {
				case IDC_VERT_SELRSPIN: selDeltaR = spin->GetIVal(); break;
				case IDC_VERT_SELGSPIN: selDeltaG = spin->GetIVal(); break;
				case IDC_VERT_SELBSPIN: selDeltaB = spin->GetIVal(); break;
				case IDC_VERT_ALPHA_SPIN:
					if (!theHold.Holding()) theHold.Begin ();
					clr.r = clr.g = clr.b = spin->GetFVal()/100.0f;
					ep->SetVertColor (clr, MAP_ALPHA);
					break;

				}
			break;

		case WM_PAINT:
			if (!ep->patchUIValid) {
				// Vertex color
				iCol = GetIColorSwatch (GetDlgItem(hDlg,IDC_VERT_COLOR),
					ep->GetVertColor(), GetString(IDS_RB_VERTEXCOLOR));
				ReleaseIColorSwatch(iCol);

				iCol = GetIColorSwatch(GetDlgItem(hDlg,IDC_VERT_ILLUM),
					ep->GetVertColor(MAP_SHADING), GetString(IDS_EM_VERTEXILLUM));
				ReleaseIColorSwatch(iCol);

				fval = ep->GetVertColor(MAP_ALPHA, &differs).r * 100.0f;
				spin = GetISpinner (GetDlgItem (hDlg, IDC_VERT_ALPHA_SPIN));
				spin->SetIndeterminate (differs);
				spin->SetValue (fval, FALSE);
				ReleaseISpinner (spin);

				// Update name of select-by-color swatch.
				rgb = RGB(int(selByColor.x*255.0),int(selByColor.y*255.0),int(selByColor.z*255.0));
				iCol = GetIColorSwatch (GetDlgItem(hDlg,IDC_VERT_SELCOLOR), rgb,
					GetString((selBy==SEL_BY_COLOR) ?IDS_RB_SELBYCOLOR : IDS_SELBYILLUM));
				ReleaseIColorSwatch(iCol);

				ep->patchUIValid = TRUE;
				}
			return FALSE;

		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;

		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept (GetString(IDS_RB_SETVERTCOLOR));
			else theHold.Cancel();
			break;

		case CC_COLOR_CHANGE:
			iCol = (IColorSwatch*)lParam;
			switch (LOWORD(wParam)) {
			case IDC_VERT_COLOR:
				ep->SetVertColor (Color(iCol->GetColor()));
				break;
			case IDC_VERT_ILLUM:
				ep->SetVertColor (Color(iCol->GetColor()), MAP_SHADING);
				break;
			case IDC_VERT_SELCOLOR:
				COLORREF rgb = iCol->GetColor();
				selByColor.x = float(GetRValue(rgb))/255.0f;
				selByColor.y = float(GetGValue(rgb))/255.0f;
				selByColor.z = float(GetBValue(rgb))/255.0f;
				break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {	
				case IDC_VERT_SELBYCOLOR:
					BOOL add, sub;
					add = GetKeyState(VK_CONTROL)<0;
					sub = GetKeyState(VK_MENU)<0;
					if(selBy == SEL_BY_COLOR)
						ep->SelectVertByColor (selByColor,selDeltaR,selDeltaG,selDeltaB,add,sub);
					else
						ep->SelectVertByColor (selByColor,selDeltaR,selDeltaG,selDeltaB,add,sub, MAP_SHADING);
					break;
				case IDC_BY_COLOR:
					if(selBy != SEL_BY_COLOR) {
						selBy = SEL_BY_COLOR;
						CheckSelectBy(hDlg);
						ep->patchUIValid = false;
						InvalidateRect (hDlg, NULL, false);
						}
					break;
				case IDC_BY_ILLUM:
					if(selBy != SEL_BY_ILLUM) {
						selBy = SEL_BY_ILLUM;
						CheckSelectBy(hDlg);
						ep->patchUIValid = false;
						InvalidateRect (hDlg, NULL, false);
						}
					break;
				}
			break;
		}
	
	return FALSE;
	}

static void MaybeDisableRelaxControls(EditPatchMod *ep,HWND hDlg) {
	BOOL enable = ep->relax;
	if(enable) {
		ep->SetShowInterior(TRUE);	// Force this on
		if(ep->hOpsPanel) {
			CheckDlgButton( ep->hOpsPanel, IDC_SHOW_INTERIOR_FACES, TRUE);
			EnableWindow(GetDlgItem(ep->hOpsPanel,IDC_SHOW_INTERIOR_FACES), FALSE);
			}
		}
	else {
		if(ep->hOpsPanel)
			EnableWindow(GetDlgItem(ep->hOpsPanel,IDC_SHOW_INTERIOR_FACES), TRUE);
		}
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_RELAXSPIN));
	spin->Enable(enable);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_ITERSPIN));
	spin->Enable(enable);
	ReleaseISpinner(spin);
	
#ifndef NO_OUTPUTRENDERER
	EnableWindow(GetDlgItem(hDlg,IDC_RELAX_VIEWPORTS),enable);
#else
	EnableWindow(GetDlgItem(hDlg,IDC_RELAX_VIEWPORTS),TRUE);
#endif

	
	
	EnableWindow(GetDlgItem(hDlg,IDC_BOUNDARY),enable);
	EnableWindow(GetDlgItem(hDlg,IDC_SADDLE),enable);

	EnableWindow(GetDlgItem(hDlg,IDC_EDPATCH_SURF_OBJ_RELAXVALUE_LABEL),enable);
	EnableWindow(GetDlgItem(hDlg,IDC_EDPATCH_SURF_OBJ_ITERATIONS_LABEL),enable);
		
	// invalidate UI, labels have changed
	//
	s_bHackSurfUIValid = FALSE;
	}

INT_PTR CALLBACK PatchObjSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	ISpinnerControl *spin;
	EditPatchMod *ep = (EditPatchMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !ep && message != WM_INITDIALOG ) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG: {

		 	ep = (EditPatchMod *)lParam;
		 	ep->hSurfPanel = hDlg;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)ep );		 	
			ep->relaxSpin = SetupFloatSpinner( hDlg, IDC_RELAXSPIN, IDC_RELAX, 0.0f, 1.0f, ep->relaxValue);
			ep->relaxIterSpin = SetupIntSpinner( hDlg, IDC_ITERSPIN, IDC_ITER, 0, 999999999, ep->relaxIter);
			CheckDlgButton( hDlg, IDC_DO_RELAX, ep->relax );
			CheckDlgButton( hDlg, IDC_EDPATCH_SURF_OBJ_RELAXVALUE_LABEL, ep->relax );
			CheckDlgButton( hDlg, IDC_EDPATCH_SURF_OBJ_ITERATIONS_LABEL, ep->relax );

#ifndef NO_OUTPUTRENDERER
			CheckDlgButton( hDlg, IDC_RELAX_VIEWPORTS, ep->relaxViewports );
#else
			CheckDlgButton( hDlg, IDC_RELAX_VIEWPORTS, ep->relax );
#endif
			CheckDlgButton( hDlg, IDC_BOUNDARY, ep->relaxBoundary);
			CheckDlgButton( hDlg, IDC_SADDLE, ep->relaxSaddle);
			MaybeDisableRelaxControls(ep,hDlg);
			return TRUE;
			}

		case REFRESH_EP_VALUES:
			ep->relaxSpin->SetValue(ep->relaxValue,FALSE);
			ep->relaxIterSpin->SetValue(ep->relaxIter,FALSE);
			CheckDlgButton( hDlg, IDC_DO_RELAX, ep->relax );
#ifndef NO_OUTPUTRENDERER
			CheckDlgButton( hDlg, IDC_RELAX_VIEWPORTS, ep->relaxViewports );
#else
			CheckDlgButton( hDlg, IDC_RELAX_VIEWPORTS, ep->relax );
#endif
			CheckDlgButton( hDlg, IDC_BOUNDARY, ep->relaxBoundary);
			CheckDlgButton( hDlg, IDC_SADDLE, ep->relaxSaddle);
			MaybeDisableRelaxControls(ep,hDlg);
			return TRUE;

		case WM_DESTROY:
			if( ep->relaxSpin ) {
				ReleaseISpinner(ep->relaxSpin);
				ep->relaxSpin = NULL;
				}
			if( ep->relaxIterSpin ) {
				ReleaseISpinner(ep->relaxIterSpin);
				ep->relaxIterSpin = NULL;
				}
			return FALSE;
		
		case WM_CUSTEDIT_ENTER:
			switch( LOWORD(wParam) ) {
				case IDC_RELAX:
					ep->SetRelaxValue(ep->relaxSpin->GetFVal(), TRUE);
					break;
				case IDC_ITER:
					ep->SetRelaxIter(ep->relaxIterSpin->GetIVal(), TRUE);
					break;
				}
			break;

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;

			switch ( LOWORD(wParam) ) {
				case IDC_RELAXSPIN:
					ep->SetRelaxValue(ep->relaxSpin->GetFVal(), TRUE);
					break;
				case IDC_ITERSPIN:
					ep->SetRelaxIter(ep->relaxIterSpin->GetIVal(), TRUE);
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {	
				case IDC_DO_RELAX:
					ep->SetRelax(IsDlgButtonChecked(hDlg, IDC_DO_RELAX), TRUE);
					MaybeDisableRelaxControls(ep,hDlg);
					break;
				case IDC_RELAX_VIEWPORTS:
#ifdef NO_OUTPUTRENDERER
					ep->SetRelax(IsDlgButtonChecked(hDlg, IDC_RELAX_VIEWPORTS), TRUE);
					MaybeDisableRelaxControls(ep,hDlg);
#endif
					ep->SetRelaxViewports(IsDlgButtonChecked(hDlg, IDC_RELAX_VIEWPORTS), TRUE);
					break;
				case IDC_BOUNDARY:
					ep->SetRelaxBoundary(IsDlgButtonChecked(hDlg, IDC_BOUNDARY), ep->Relaxing());
					break;
				case IDC_SADDLE:
					ep->SetRelaxSaddle(IsDlgButtonChecked(hDlg, IDC_SADDLE), ep->Relaxing());
					break;
				}
			break;
		}
	
	return FALSE;
	}

#ifdef USING_ADVANCED_APPROX
// Advanced TessApprox settings...

static ISpinnerControl* psMinSpin = NULL;
static ISpinnerControl* psMaxSpin = NULL;
static ISpinnerControl* psMaxTrisSpin = NULL;
// this max matches the MI max.
#define MAX_SUBDIV 7


static BOOL initing = FALSE; // this is a hack but CenterWindow causes bad commands

INT_PTR CALLBACK
AdvParametersDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg) {
    case WM_INITDIALOG: {
		initing = TRUE;
        CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
		initing = FALSE;
		psMinSpin = SetupIntSpinner( hDlg, IDC_TESS_MIN_REC_SPINNER, IDC_TESS_MIN_REC, 0, sParams.mMax, sParams.mMin);
		psMaxSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_REC_SPINNER, IDC_TESS_MAX_REC, sParams.mMin, MAX_SUBDIV, sParams.mMax);
		psMaxTrisSpin = SetupIntSpinner( hDlg, IDC_TESS_MAX_TRIS_SPINNER, IDC_TESS_MAX_TRIS, 0, 2000000, sParams.mTris);
		switch (sParams.mStyle) {
		case SUBDIV_GRID:
			CheckDlgButton( hDlg, IDC_GRID, TRUE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_TREE:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_DELAUNAY:
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			break;
		}
		break; }

    case WM_COMMAND:
		if (initing) return FALSE;
		switch ( LOWORD(wParam) ) {
		case IDOK:
			EndDialog(hDlg, 1);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDC_GRID:
			sParams.mStyle = SUBDIV_GRID;
			CheckDlgButton( hDlg, IDC_GRID, TRUE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case IDC_TREE:
			sParams.mStyle = SUBDIV_TREE;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, TRUE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, FALSE);
			break;
		case IDC_DELAUNAY:
			sParams.mStyle = SUBDIV_DELAUNAY;
			CheckDlgButton( hDlg, IDC_GRID, FALSE);
			CheckDlgButton( hDlg, IDC_TREE, FALSE);
			CheckDlgButton( hDlg, IDC_DELAUNAY, TRUE);
			break;
		}
		break;

    case CC_SPINNER_CHANGE:
		switch ( LOWORD(wParam) ) {
		case IDC_TESS_MIN_REC_SPINNER:
			sParams.mMin = psMinSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_REC_SPINNER:
			sParams.mMax = psMaxSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_TRIS_SPINNER:
			sParams.mTris = psMaxTrisSpin->GetIVal();
			break;
		}
		break;

	case WM_DESTROY:
		if( psMinSpin ) {
			ReleaseISpinner(psMinSpin);
			psMinSpin = NULL;
		}
		if( psMaxSpin ) {
			ReleaseISpinner(psMaxSpin);
			psMaxSpin = NULL;
		}
		if( psMaxTrisSpin ) {
			ReleaseISpinner(psMaxTrisSpin);
			psMaxTrisSpin = NULL;
		}
		break;
	}

	return FALSE;
}
#endif //USING_ADVANCED_APPROX

#define OLD_SEL_LEVEL_CHUNK 0x1000	// Original backwards ordering
#define SEL_LEVEL_CHUNK 0x1001
#define FLAGS_CHUNK 0x1005
#define DISP_LATTICE_CHUNK 0x1010
#define DISP_SURFACE_CHUNK 0x1020
#define DISP_VERTS_CHUNK 0x1030
#define EPM_MESH_ATTRIB_CHUNK	0x1040
#define EPM_VTESS_ATTRIB_CHUNK	0x1090
#define EPM_PTESS_ATTRIB_CHUNK	0x10a0
#define EPM_DTESS_ATTRIB_CHUNK	0x10b0
#define EPM_NORMAL_TESS_ATTRIB_CHUNK	0x10c0
#define EPM_WELD_TESS_ATTRIB_CHUNK	0x10d0
#define EPM_RENDERSTEPS_CHUNK		0x10e0
#define EPM_SHOWINTERIOR_CHUNK		0x10f0
// The following chunk is written on r3 and later files
// If not present, named selection data structures need fixup
#define EPM_SEL_NAMES_OK		0x1100	
#define EPM_RELAX_CHUNK			0x1110
#define EPM_GEN_SURFACE_CHUNK	0x1120
// CAL-05/15/03: use true patch normals. (FID #1760)
#define EPM_USEPATCHNORM_CHUNK	0x1130
// CAL-06/02/03: copy/paste tangent. (FID #827)
#define EPM_COPY_TANGENT_CHUNK	0x1140

// Names of named selection sets
#define NAMEDVSEL_NAMES_CHUNK	0x1050
#define NAMEDESEL_NAMES_CHUNK	0x1060
#define NAMEDPSEL_NAMES_CHUNK	0x1070
#define NAMEDSEL_STRING_CHUNK	0x1080

#define EPM_SOFT_SEL_CHUNK		0x2010

static int namedSelID[] = {
	NAMEDVSEL_NAMES_CHUNK,
	NAMEDESEL_NAMES_CHUNK,
	NAMEDPSEL_NAMES_CHUNK};


IOResult EditPatchMod::Save(ISave *isave) {
	Modifier::Save(isave);
	Interval valid;
	ULONG nb;
	// In r3 and later, if the named sel names are OK, write this chunk
	if(!namedSelNeedsFixup) {
		isave->BeginChunk(EPM_SEL_NAMES_OK);
		isave->EndChunk();
		}
	isave->BeginChunk (FLAGS_CHUNK);
	isave->Write (&epFlags, sizeof(DWORD), &nb);
	isave->EndChunk();
	isave->BeginChunk(SEL_LEVEL_CHUNK);
	isave->Write(&selLevel,sizeof(int),&nb);
	isave->	EndChunk();
	isave->BeginChunk(DISP_LATTICE_CHUNK);
	isave->Write(&displayLattice,sizeof(BOOL),&nb);
	isave->	EndChunk();
	isave->BeginChunk(DISP_SURFACE_CHUNK);
	isave->Write(&displaySurface,sizeof(BOOL),&nb);
	isave->	EndChunk();
	isave->BeginChunk(EPM_MESH_ATTRIB_CHUNK);
	isave->Write(&meshSteps,sizeof(int),&nb);
// Future use (Not used now)
	BOOL fakeAdaptive = FALSE;
	isave->Write(&fakeAdaptive,sizeof(BOOL),&nb);
//	isave->Write(&meshAdaptive,sizeof(BOOL),&nb);	// Future use (Not used now)
	isave->	EndChunk();



//3-18-99 to suport render steps and removal of the mental tesselator
#ifndef NO_OUTPUTRENDERER
	isave->BeginChunk(EPM_RENDERSTEPS_CHUNK);
	if ( (meshStepsRender < 0) || (meshStepsRender > 100))
		{
		meshStepsRender = 5;
		DbgAssert(0);
		}
	isave->Write(&meshStepsRender,sizeof(int),&nb);
	isave->	EndChunk();
#endif
	isave->BeginChunk(EPM_SHOWINTERIOR_CHUNK);
	isave->Write(&showInterior,sizeof(BOOL),&nb);
	isave->	EndChunk();
	// CAL-05/15/03: use true patch normals. (FID #1760)
	isave->BeginChunk(EPM_USEPATCHNORM_CHUNK);
	isave->Write(&usePatchNormals, sizeof(BOOL), &nb);
	isave->EndChunk();

	// CAL-05/01/03: support spline surface generation (FID #1914)
	isave->BeginChunk(EPM_GEN_SURFACE_CHUNK);
	isave->Write(&generateSurface,sizeof(BOOL),&nb);
	isave->Write(&genSurfWeldThreshold,sizeof(float),&nb);
	isave->Write(&genSurfFlipNormals,sizeof(BOOL),&nb);
	isave->Write(&genSurfRmInterPatches,sizeof(BOOL),&nb);
	isave->Write(&genSurfUseOnlySelSegs,sizeof(BOOL),&nb);
	isave->	EndChunk();

	// CAL-06/02/03: copy/paste tangent. (FID #827)
	isave->BeginChunk(EPM_COPY_TANGENT_CHUNK);
	isave->Write(&copyTanLength, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(EPM_RELAX_CHUNK);
	isave->Write(&relax,sizeof(BOOL), &nb);
	isave->Write(&relaxViewports,sizeof(BOOL), &nb);
	isave->Write(&relaxValue,sizeof(float), &nb);
	isave->Write(&relaxIter,sizeof(int), &nb);
	isave->Write(&relaxBoundary,sizeof(BOOL), &nb);
	isave->Write(&relaxSaddle,sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(EPM_VTESS_ATTRIB_CHUNK);
	viewTess.Save(isave);
	isave->	EndChunk();
	isave->BeginChunk(EPM_PTESS_ATTRIB_CHUNK);
	prodTess.Save(isave);
	isave->	EndChunk();
	isave->BeginChunk(EPM_DTESS_ATTRIB_CHUNK);
	dispTess.Save(isave);
	isave->	EndChunk();

	isave->BeginChunk(EPM_NORMAL_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessNormals,sizeof(BOOL),&nb);
	isave->Write(&mProdTessNormals,sizeof(BOOL),&nb);
	isave->	EndChunk();
	isave->BeginChunk(EPM_WELD_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessWeld,sizeof(BOOL),&nb);
	isave->Write(&mProdTessWeld,sizeof(BOOL),&nb);
	isave->	EndChunk();
	
	// Save names of named selection sets
	for (int j=0; j<EP_NS_LEVELS; j++) {
		if (namedSel[j].Count()) {
			isave->BeginChunk(namedSelID[j]);			
			for (int i=0; i<namedSel[j].Count(); i++) {
				isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
				isave->WriteWString(*namedSel[j][i]);
				isave->EndChunk();
				}
			isave->EndChunk();
			}
		}

	isave->BeginChunk(EPM_SOFT_SEL_CHUNK);
		isave->Write(&mEdgeDist,sizeof(int),&nb);
		isave->Write(&mUseEdgeDists,sizeof(int),&nb);
		isave->Write(&mAffectBackface,sizeof(int),&nb);
		isave->Write(&mUseSoftSelections,sizeof(int),&nb);
		isave->Write(&mFalloff,sizeof(float),&nb);
		isave->Write(&mPinch,sizeof(float),&nb);
		isave->Write(&mBubble,sizeof(float),&nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult EditPatchMod::LoadNamedSelChunk(ILoad *iload,int level)
	{	
	IOResult res;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case NAMEDSEL_STRING_CHUNK: {
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				// Set the name in the modifier
				AddSet(TSTR(name),level+1);
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

IOResult EditPatchMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	namedSelNeedsFixup = TRUE;	// Pre-r3 default
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case EPM_SEL_NAMES_OK:
				namedSelNeedsFixup = FALSE;
				break;
			case NAMEDVSEL_NAMES_CHUNK: {				
				res = LoadNamedSelChunk(iload,0);
				break;
				}
			case NAMEDESEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,1);
				break;
				}
			case NAMEDPSEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,2);
				break;
				}

			case FLAGS_CHUNK:
				res = iload->Read(&epFlags,sizeof(DWORD),&nb);
				break;
			case OLD_SEL_LEVEL_CHUNK:	// Correct backwards ordering
				{
				short sl;
				res = iload->Read(&sl,sizeof(short),&nb);
				selLevel = sl;
				switch(selLevel) {
					case 1:
						selLevel = EP_PATCH;
						break;
					case 3:
						selLevel = EP_VERTEX;
						break;
					}
				}
				break;
			case SEL_LEVEL_CHUNK:
				res = iload->Read(&selLevel,sizeof(int),&nb);
				break;
			case DISP_LATTICE_CHUNK:
				res = iload->Read(&displayLattice,sizeof(BOOL),&nb);
				break;
			case DISP_SURFACE_CHUNK:
				res = iload->Read(&displaySurface,sizeof(BOOL),&nb);
				break;
			case DISP_VERTS_CHUNK:
				iload->SetObsolete();
				break;
			case EPM_MESH_ATTRIB_CHUNK:
				res = iload->Read(&meshSteps,sizeof(int),&nb);
				res = iload->Read(&meshAdaptive,sizeof(BOOL),&nb);
				break;
//3-18-99 to suport render steps and removal of the mental tesselator
#ifndef NO_OUTPUTRENDERER
			case EPM_RENDERSTEPS_CHUNK:
				res = iload->Read(&meshStepsRender,sizeof(int),&nb);
				if ( (meshStepsRender < 0) || (meshStepsRender > 100))
					{
					meshStepsRender = 5;
					DbgAssert(0);
					}
				break;
#endif
			case EPM_SHOWINTERIOR_CHUNK:
				res = iload->Read(&showInterior,sizeof(BOOL),&nb);
				break;
			// CAL-05/15/03: use true patch normals. (FID #1760)
			case EPM_USEPATCHNORM_CHUNK:
				res = iload->Read(&usePatchNormals, sizeof(BOOL), &nb);
				break;
			// CAL-05/01/03: support spline surface generation (FID #1914)
			case EPM_GEN_SURFACE_CHUNK:
				res = iload->Read(&generateSurface,sizeof(BOOL),&nb);
				res = iload->Read(&genSurfWeldThreshold,sizeof(float),&nb);
				res = iload->Read(&genSurfFlipNormals,sizeof(BOOL),&nb);
				res = iload->Read(&genSurfRmInterPatches,sizeof(BOOL),&nb);
				res = iload->Read(&genSurfUseOnlySelSegs,sizeof(BOOL),&nb);
				break;
			// CAL-06/02/03: copy/paste tangent. (FID #827)
			case EPM_COPY_TANGENT_CHUNK:
				res = iload->Read(&copyTanLength, sizeof(BOOL), &nb);
				break;
			case EPM_RELAX_CHUNK:
				iload->Read(&relax,sizeof(BOOL), &nb);
				iload->Read(&relaxViewports,sizeof(BOOL), &nb);
				iload->Read(&relaxValue,sizeof(float), &nb);
				if(relaxValue < 0.0f)
					relaxValue = 0.0f;
				iload->Read(&relaxIter,sizeof(int), &nb);
				iload->Read(&relaxBoundary,sizeof(BOOL), &nb);
				res = iload->Read(&relaxSaddle,sizeof(BOOL), &nb);
				break;
			case EPM_VTESS_ATTRIB_CHUNK:
				viewTess.Load(iload);
				break;
			case EPM_PTESS_ATTRIB_CHUNK:
				prodTess.Load(iload);
				break;
			case EPM_DTESS_ATTRIB_CHUNK:
				dispTess.Load(iload);
				break;
			case EPM_NORMAL_TESS_ATTRIB_CHUNK:
				res = iload->Read(&mViewTessNormals,sizeof(BOOL),&nb);
				res = iload->Read(&mProdTessNormals,sizeof(BOOL),&nb);
				break;
			case EPM_WELD_TESS_ATTRIB_CHUNK:
				res = iload->Read(&mViewTessWeld,sizeof(BOOL),&nb);
				res = iload->Read(&mProdTessWeld,sizeof(BOOL),&nb);
				break;
			case EPM_SOFT_SEL_CHUNK:
				{
				iload->Read(&mEdgeDist,sizeof(int),&nb);
				iload->Read(&mUseEdgeDists,sizeof(int),&nb);
				iload->Read(&mAffectBackface,sizeof(int),&nb);
				iload->Read(&mUseSoftSelections,sizeof(int),&nb);
				iload->Read(&mFalloff,sizeof(float),&nb);
				iload->Read(&mPinch,sizeof(float),&nb);
				iload->Read(&mBubble,sizeof(float),&nb);
				}
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


#define EDITPATCHDATA_CHUNK 0x1000

IOResult EditPatchMod::SaveLocalData(ISave *isave, LocalModData *ld) {
	EditPatchData *ep = (EditPatchData *)ld;

	isave->BeginChunk(EDITPATCHDATA_CHUNK);
	ep->Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

IOResult EditPatchMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	IOResult res;
	EditPatchData *ep;
	if (*pld==NULL) {
		*pld =(LocalModData *) new EditPatchData(this);
		}
	ep = (EditPatchData *)*pld;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case EDITPATCHDATA_CHUNK:
				res = ep->Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

// ------------------------------------------------------

int EPM_ExtrudeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;
	BOOL ln;
	IPoint2 m2;
	float amount;
	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		if (!point) {
			po->BeginExtrude(ip->GetTime());		
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			po->EndExtrude(ip->GetTime(),TRUE);

		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		// sca 1999.02.24: Find m's projection in om's vertical axis:
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView(m2,float(-200));
		amount = Length (p1-p0);
		if (m.y > om.y) amount *= -1.0f;

		ln = IsDlgButtonChecked(po->hOpsPanel,IDC_EM_EXTYPE_B);
		po->Extrude (ip->GetTime(), amount, ln);

		spin = GetISpinner(GetDlgItem(po->hOpsPanel,IDC_EP_EXTRUDESPINNER));
		if (spin) {
			spin->SetValue(amount, FALSE);	// sca - use signed value here too.
			ReleaseISpinner(spin);
		}
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		po->EndExtrude(ip->GetTime(),FALSE);			
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EPM_ExtrudeSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDECUR));
	return hCur; 
}

void EPM_ExtrudeCMode::EnterMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EPM_ExtrudeCMode::ExitMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(po->hOpsPanel,IDC_EP_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0.0f,FALSE);
		ReleaseISpinner(spin);
		}

}

// ------------------------------------------------------

BOOL EPM_NormalFlipMouseProc::HitTest( 
		ViewExp *vpt, IPoint2 *p, int type, int flags, int subType  )
	{
	vpt->ClearSubObjHitList();
	SetPatchHitOverride(subType);

	ip->SubObHitTest(ip->GetTime(),type,ip->GetCrossing(),flags,p,vpt);
	ClearPatchHitOverride();
	if ( vpt->NumSubObjHits() ) {
		return TRUE;
	} else {
		return FALSE;
		}			
	}

BOOL EPM_NormalFlipMouseProc::HitAPatch(ViewExp *vpt, IPoint2 *p, int *pix) {
	int first = 1;
	
	if(HitTest(vpt, p, HITTYPE_POINT, 0, PO_PATCH) ) {
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while(rec) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			pMesh = hit->patch;
			if (hit->type == PATCH_HIT_PATCH)
				{
				if(first || rec->distance < best) 
					{
					first = 0;
					best = rec->distance;
					bestRec = rec;
					}
				}
			rec = rec->Next();
			}
		if(!first) {
			PatchHitData *hit = ((PatchHitData *)bestRec->hitData);
			*pix = hit->index;
			return TRUE;
			}
		}
	return FALSE;
	}

int EPM_NormalFlipMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;
	int pix;

	switch (msg) {
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			if(HitAPatch(vpt, &m,  &pix))
				ep->DoFlipNormals(pMesh, pix, (ep->GetSubobjectLevel() == EP_ELEMENT) ? TRUE : FALSE);
			break;
		case MOUSE_FREEMOVE:
			if(HitAPatch(vpt, &m,  &pix))
				{
				SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				}
			else {
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
			break;
		}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

void EPM_NormalFlipCMode::EnterMode() {
	if (!ep->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(ep->hSurfPanel,IDC_NORMAL_FLIPMODE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EPM_NormalFlipCMode::ExitMode() {
	if (!ep->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(ep->hSurfPanel,IDC_NORMAL_FLIPMODE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}


// ------------------------------------------------------

int EPM_BevelMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;
	int ln,ln2;
	IPoint2 m2;
	float amount;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		if (point==0) {
			po->BeginExtrude(ip->GetTime());		
			om = m;
			} 
		else if (point==1) {
			po->EndExtrude(ip->GetTime(),TRUE);
			po->BeginBevel(ip->GetTime());		
			om = m;
			} 
		else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			po->EndBevel(ip->GetTime(),TRUE);
		}
		break;

	case MOUSE_MOVE:
		if (point == 1)
			{
			p0 = vpt->MapScreenToView(om,float(-200));
			// sca 1999.02.24: find worldspace point with om's x value and m's y value
			m2.x = om.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView(m2, float(-200));
			amount = Length (p1-p0);
			ln = IsDlgButtonChecked(po->hOpsPanel,IDC_EM_EXTYPE_B);					
			if (om.y < m.y) amount *= -1.0f;
			po->Extrude (ip->GetTime(), amount, ln);

			spin = GetISpinner(GetDlgItem(po->hOpsPanel,IDC_EP_EXTRUDESPINNER));
			if (spin) {
				spin->SetValue (amount, FALSE);
				ReleaseISpinner(spin);
				}
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			}
		else if (point == 2)
			{
			p0 = vpt->MapScreenToView(om,float(-200));
			// sca 1999.02.24: find worldspace point with om's x value and m's y value
			m2.x = om.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView(m2, float(-200));
			if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH)) ln = 0;					
			else if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH2)) ln = 1;					
			else if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH3)) ln = 2;					

			if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH4)) ln2 = 0;					
			else if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH5)) ln2 = 1;					
			else if (IsDlgButtonChecked(po->hOpsPanel,IDC_EP_SM_SMOOTH6)) ln2 = 2;					

			amount = Length(p1-p0);
			if (om.y < m.y) amount *= -1.0f;
			po->Bevel (ip->GetTime(), amount, ln, ln2);

			spin = GetISpinner(GetDlgItem(po->hOpsPanel,IDC_EP_OUTLINESPINNER));
			if (spin) {
				spin->SetValue(amount,FALSE);
				ReleaseISpinner(spin);
				}
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			}
		break;

	case MOUSE_ABORT:
		if (point==1)
			po->EndExtrude(ip->GetTime(),FALSE);			
		else if (point>1)
			po->EndBevel(ip->GetTime(),FALSE);			
			

		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EPM_BevelSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_BEVEL));
	return hCur; 
}

void EPM_BevelCMode::EnterMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EPM_BevelCMode::ExitMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(po->hOpsPanel,IDC_EP_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0.0f,FALSE);
		ReleaseISpinner(spin);
		}

}


// Create interfaces
//------------------------------------------
void EPM_CreateVertCMode::EnterMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EPM_CreateVertCMode::ExitMode() {
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}


int EPM_CreateVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 screenPoint) {

	ViewExp *vpt = mpIP->GetViewport (hwnd);
	Matrix3 ctm;
	Point3 pt;
	IPoint2 screenPointSnapped;
	int newIndex;

	switch (msg) {
	case MOUSE_ABORT:
	case MOUSE_PROPCLICK:
		mpIP->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		mpIP->SetActiveViewport(hwnd);
		vpt->GetConstructionTM(ctm);
		pt = vpt->SnapPoint (screenPoint, screenPointSnapped, &ctm);
		pt = pt * ctm;
		po->CreateVertex(pt, newIndex);

		break;

	case MOUSE_FREEMOVE:
		SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ES_CROSS_HAIR)));
		vpt->SnapPreview(screenPoint, screenPoint, NULL, SNAP_FORCE_3D_RESULT);
		break;
	}

	if (vpt) mpIP->ReleaseViewport(vpt);
	return TRUE;
}


void EPM_CreatePatchCMode::EnterMode() 
{
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	inPatchCreate = TRUE;
	po->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	po->ip->RedrawViews(po->ip->GetTime(),REDRAW_NORMAL);

}

void EPM_CreatePatchCMode::ExitMode() 
{
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	inPatchCreate = FALSE;
	po->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	po->ip->RedrawViews(po->ip->GetTime(),REDRAW_NORMAL);
}

EPM_CreatePatchMouseProc::EPM_CreatePatchMouseProc(EditPatchMod* mod, IObjParam *i)
{
	po=mod;
	mpIP=i;
}

BOOL EPM_CreatePatchMouseProc::HitTest( 
		ViewExp *vpt, IPoint2 *p, int type, int flags, int subType  )  
{
	vpt->ClearSubObjHitList();
	SetPatchHitOverride(subType);

	mpIP->SubObHitTest(mpIP->GetTime(),type,mpIP->GetCrossing(),flags,p,vpt);
	ClearPatchHitOverride();
	if ( vpt->NumSubObjHits() ) {
		return TRUE;
	} else {
		return FALSE;
	}			
}

BOOL EPM_CreatePatchMouseProc::HitAVert(ViewExp *vpt, IPoint2 *p, 
									 int& vert, ModContext*& pTheModContext) 
{																 
	int first = 1;
	
	if(HitTest(vpt, p, HITTYPE_POINT, 0,1) ) {
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while(rec) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
			if (hit->type == PATCH_HIT_VERTEX) {
				// If there's an exclusion shape, the vert & poly can't be the same!
				if(first || rec->distance < best) {
					first = 0;
					best = rec->distance;
					bestRec = rec;
				}
			}
			rec = rec->Next();
		}
		if(!first) {
			PatchHitData *hit = ((PatchHitData *)bestRec->hitData);
			vert = hit->index;
			pTheModContext = bestRec->modContext;
			return TRUE;
		}
	}
	return FALSE;
}


int EPM_CreatePatchMouseProc::proc(HWND hwnd, int msg, int point, int flags, IPoint2 screenPoint )
{
	ViewExp *vpt = mpIP->GetViewport (hwnd);
	Matrix3 ctm;
	Point3 pt;
	IPoint2 screenPointSnapped;
	int vertIndex;
	ModContextList mcList;
	INodeTab nodes;
	bool pickedPt = true;
	mpIP->GetModContexts(mcList,nodes);
	ModContext* pFirstModContext = mcList[0];
	nodes.DisposeTemporary();
	ModContext* pTheModContext;

	// I had trouble getting XOR draws to behave nicely in all situations
	// I finally broke it down to separate lines and plugged the problems
	// to make it work.  This can be cleaned up when the time permits.

	switch (msg) {
	case MOUSE_ABORT:
		if (point==4) {
			// build the tri patch
			po->CreatePatch(verts[0],verts[1],verts[2]);
		}
		else {
			mpIP->SetStdCommandMode(CID_OBJMOVE);
		}
		break;
	case MOUSE_PROPCLICK:
		mpIP->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		if (point == 1) break; // skip the first up click

		mpIP->SetActiveViewport(hwnd);
		vpt->GetConstructionTM(ctm);

		// test for a hit on existing vertices and get the index for the selcted vertex
		if (!HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) ||
			pTheModContext!=pFirstModContext) { 
			// create a new vertex if no existing vertex was hit
			pt = vpt->SnapPoint (screenPoint, screenPointSnapped, &ctm); 
			pt = pt * ctm;
			po->CreateVertex(pt,  vertIndex);
			pickedPt = false;
		}

		switch(point) {
		case 0:
			verts[0]=vertIndex;

			// Draw new dotted line
			startPoint = anchor = lastPoint = screenPoint;
			PatchXORDottedLine(hwnd, anchor, screenPoint);
			break;
		case 1:
			// ignore the upclick
			break;
		case 2:
			if (vertIndex==verts[0]){
				// reset since this is one way to end the command
				mpIP->SetStdCommandMode(CID_OBJMOVE);
			}
			else {
				verts[1]=vertIndex;
			}

			// draw the dotted line
			if (!pickedPt) PatchXORDottedLine(hwnd, anchor, screenPoint);
			anchor = lastPoint = screenPoint;
			PatchXORDottedLine(hwnd, anchor, screenPoint);
			PatchXORDottedLine(hwnd, startPoint, screenPoint);
			break;
		case 3:
			if (vertIndex==verts[0] || vertIndex==verts[1]){
				// reset since this is one way to end the command
				mpIP->SetStdCommandMode(CID_OBJMOVE);
			}
			else {
				verts[2]=vertIndex;
			}

			// draw the dotted lines
			if (!pickedPt) {
				PatchXORDottedLine(hwnd, startPoint, anchor);
				PatchXORDottedLine(hwnd, anchor, screenPoint);
			}
			anchor = lastPoint = screenPoint;
			PatchXORDottedLine(hwnd, anchor, screenPoint);
			if (!pickedPt) PatchXORDottedLine(hwnd, startPoint, screenPoint);
			break;
		case 4:
			if (vertIndex==verts[0] || 
				vertIndex==verts[1] ||
				vertIndex==verts[2]){
				// create the tri patch
				po->CreatePatch(verts[0],verts[1],verts[2]);
			}
			else {
				verts[3]=vertIndex;
				// create the quad patch
				po->CreatePatch(verts[0],verts[1],verts[2],verts[3]);

			}
			break;
		default:
			assert(0);
		}
		break;

	case MOUSE_MOVE:
		vpt->SnapPreview(screenPoint, screenPoint, NULL, SNAP_FORCE_3D_RESULT);
		mpIP->RedrawViews(mpIP->GetTime(),REDRAW_NORMAL);
		if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) &&
			pTheModContext==pFirstModContext) { 
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
		}
		else {
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ES_CROSS_HAIR)));
		}
		// Erase old dotted line
		PatchXORDottedLine(hwnd, anchor, lastPoint);
		// Draw new dotted line
		PatchXORDottedLine(hwnd, anchor, screenPoint);
		if (point>2) {
			// Erase old dotted line
			PatchXORDottedLine(hwnd, startPoint, lastPoint);
			// Draw new dotted line
			PatchXORDottedLine(hwnd, startPoint, screenPoint);
		}
		lastPoint = screenPoint;
		break;
	case MOUSE_FREEMOVE:
		vpt->SnapPreview(screenPoint, screenPoint, NULL, SNAP_FORCE_3D_RESULT);
		if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) &&
			pTheModContext==pFirstModContext) { 
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
		}
		else {
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ES_CROSS_HAIR)));
		}
		break;
	}

	if (vpt) mpIP->ReleaseViewport(vpt);
	return TRUE;
}


//-----------------------------------------------------------------------
// VERTEX WELD

void EPM_VertWeldCMode::EnterMode() 
{
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_VERTWELD));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EPM_VertWeldCMode::ExitMode() 
{
	if (!po->hOpsPanel) return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_VERTWELD));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}


BOOL EPM_VertWeldMouseProc::HitTest( 
		ViewExp *vpt, IPoint2 *p, int type, int flags, int subType  ) 
{
	vpt->ClearSubObjHitList();
	SetPatchHitOverride(subType);

	// use the users pick box size for this operation
	int savedSize = po->pickBoxSize;
	po->pickBoxSize = po->weldBoxSize;
	mpIP->SubObHitTest(mpIP->GetTime(),type,mpIP->GetCrossing(),flags,p,vpt);
	po->pickBoxSize = savedSize;

	ClearPatchHitOverride();
	if ( vpt->NumSubObjHits() ) {
		return TRUE;
	} else {
		return FALSE;
	}			
}

BOOL EPM_VertWeldMouseProc::HitAVert(ViewExp *vpt, IPoint2 *p, 
									 int& vert, ModContext*& pTheModContext) 
{									
	int first = 1;
	
	if(HitTest(vpt, p, HITTYPE_POINT, 0,1) ) {
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while(rec) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
			if (hit->type == PATCH_HIT_VERTEX) {
				// If there's an exclusion shape, the vert & poly can't be the same!
				if(first || rec->distance < best) {
					first = 0;
					best = rec->distance;
					bestRec = rec;
				}
			}
			rec = rec->Next();
		}
		if(!first) {
			PatchHitData *hit = ((PatchHitData *)bestRec->hitData);
			vert = hit->index;
			pTheModContext = bestRec->modContext;
			return TRUE;
		}
	}
	return FALSE;
}


int EPM_VertWeldMouseProc::proc(HWND hwnd, int msg, int point, int flags, IPoint2 screenPoint )
{
	ViewExp *vpt = mpIP->GetViewport (hwnd);
	int vertIndex;
	int result = TRUE;
	ModContext* pTheModContext;

	switch (msg) {
	case MOUSE_ABORT:
		// Erase old dotted line
		PatchXORDottedLine(hwnd, anchor, lastPoint);
	case MOUSE_PROPCLICK:
		mpIP->SetStdCommandMode(CID_OBJMOVE);
		mpFromModContext = NULL;
		break;

	case MOUSE_POINT:
		mpIP->SetActiveViewport(hwnd);

		switch(point) {
		case 0:
			if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext)) {
				fromVert = vertIndex;
				mpFromModContext = pTheModContext;
				// Draw new dotted line
				anchor = lastPoint = screenPoint;
				PatchXORDottedLine(hwnd, anchor, screenPoint);
			}
			else {
				result = FALSE;
			}
			break;
		case 1:
			PatchXORDottedLine(hwnd, anchor, lastPoint);
			if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) &&
				pTheModContext==mpFromModContext) {
				toVert =vertIndex;
				// Erase old dotted line
				po->DoVertWeld(fromVert,toVert, pTheModContext);
			}
			else {
				result = FALSE;
			}
			mpFromModContext = NULL;
			break;
		default:
			assert(0);
		}
		break;

	case MOUSE_MOVE:
		// Erase old dotted line
		PatchXORDottedLine(hwnd, anchor, lastPoint);
		// Draw new dotted line
		PatchXORDottedLine(hwnd, anchor, screenPoint);
		lastPoint = screenPoint;
		if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) &&
			pTheModContext==mpFromModContext) { 
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
		}
		else {
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		break;
	case MOUSE_FREEMOVE:
		if (HitAVert(vpt, &screenPoint,  vertIndex, pTheModContext) ){
			SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
		}
		else {
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		break;
	}

	if (vpt) mpIP->ReleaseViewport(vpt);
	return result;
}


/*-------------------------------------------------------------------*/
// CAL-06/02/03: copy tangent. (FID #827)

void EPM_CopyTangentCMode::EnterMode()
	{
	if ( po->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList, nodes);
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if (!patchData) continue;
			if (patchData->GetFlag(EPD_BEENDONE)) continue;

			PatchMesh *patch = patchData->TempData(po)->GetPatch(t);
			if(!patch) continue;

			patch->dispFlags |= DISP_BEZHANDLES;
			patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_COPY_TANGENT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);

		po->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EPM_CopyTangentCMode::ExitMode()
	{
	if ( po->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList, nodes);
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if (!patchData) continue;
			if (patchData->GetFlag(EPD_BEENDONE)) continue;

			PatchMesh *patch = patchData->TempData(po)->GetPatch(t);
			if(!patch) continue;

			patch->dispFlags &= ~DISP_BEZHANDLES;
			patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_COPY_TANGENT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		po->SetOpsDlgEnables();

		po->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EditPatchMod::StartCopyTangentMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == copyTangentMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(copyTangentMode);
	}

BOOL EditPatchMod::CopyTangent(PatchMesh *patch, int vec)
	{
	if (!patch) return FALSE;

	DbgAssert(vec < patch->getNumVecs());
	if (vec >= patch->getNumVecs()) return FALSE;

	PatchVec &pVec = patch->getVec(vec);
	if (pVec.vert < 0) return FALSE;

	copiedTangent = pVec.p - patch->getVert(pVec.vert).p;
	tangentCopied = TRUE;
	return TRUE;
	}

HCURSOR EPM_CopyTangentMouseProc::GetTransformCursor()
	{ 
	static HCURSOR hCur = NULL;

	if ( !hCur )
		hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_COPY_TANGENT_CUR));

	return hCur;
	}

HitRecord* EPM_CopyTangentMouseProc::HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags )
	{
	HitRecord *bestRec = NULL;

	vpt->ClearSubObjHitList();
	SetPatchHitOverride(PO_HANDLE);
	ip->SubObHitTest(ip->GetTime(),type,ip->GetCrossing(),flags,p,vpt);
	ClearPatchHitOverride();
	
	if ( vpt->NumSubObjHits() ) {
		HitLog &hits = vpt->GetSubObjHitList();
		DWORD best;
		for (HitRecord *rec = hits.First(); rec != NULL; rec = rec->Next()) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			if(hit->type != PATCH_HIT_VECTOR) continue;
			if(bestRec == NULL || rec->distance < best) {
				best = rec->distance;
				bestRec = rec;
				}
			}
		}
	return bestRec;
	}

int EPM_CopyTangentMouseProc::proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m )
	{
	ViewExp *vpt = NULL;
	HitRecord *rec = NULL;

	switch ( msg ) {
		case MOUSE_ABORT:
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			vpt = ip->GetViewport(hwnd);
			if((rec = HitTest(vpt,&m,HITTYPE_POINT,0)) != NULL) {
				PatchHitData *hit = ((PatchHitData *)rec->hitData);
				EditPatchData *patchData = (EditPatchData*)rec->modContext->localData;
				if ( patchData ) {
					PatchMesh *patch = patchData->TempData(po)->GetPatch(ip->GetTime());
					if (po->CopyTangent(patch, hit->index))
						ip->SetStdCommandMode(CID_OBJMOVE);
					}
				}
			ip->ReleaseViewport(vpt);
			break;
		
		case MOUSE_MOVE:
		case MOUSE_FREEMOVE:
			vpt = ip->GetViewport(hwnd);
			if ( HitTest(vpt,&m,HITTYPE_POINT,HIT_ABORTONHIT) )
				SetCursor(GetTransformCursor());
			else
				SetCursor(LoadCursor(NULL,IDC_ARROW));
			ip->ReleaseViewport(vpt);
			break;
		}

	return FALSE;
	}


/*-------------------------------------------------------------------*/
// CAL-06/02/03: paste tangent. (FID #827)

void EPM_PasteTangentCMode::EnterMode()
	{
	if ( po->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList, nodes);
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if (!patchData) continue;
			if (patchData->GetFlag(EPD_BEENDONE)) continue;

			PatchMesh *patch = patchData->TempData(po)->GetPatch(t);
			if(!patch) continue;

			patch->dispFlags |= DISP_BEZHANDLES;
			patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_PASTE_TANGENT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);

		po->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EPM_PasteTangentCMode::ExitMode()
	{
	if ( po->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList, nodes);
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if (!patchData) continue;
			if (patchData->GetFlag(EPD_BEENDONE)) continue;

			PatchMesh *patch = patchData->TempData(po)->GetPatch(t);
			if(!patch) continue;

			patch->dispFlags &= ~DISP_BEZHANDLES;
			patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		po->ClearPatchDataFlag(mcList,EPD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel,IDC_EP_PASTE_TANGENT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);

		po->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EditPatchMod::StartPasteTangentMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == pasteTangentMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(pasteTangentMode);
	}

void EditPatchMod::StartPasteTangent(EditPatchData *patchData)
{
	if (!ip || !patchData) return;

	TimeValue t = ip->GetTime();
	PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
	if(!patch) return;

	theHold.Begin();
	patchData->BeginEdit(t);

	if(!TestAFlag(A_HELD)) {
		patchData->vdelta.SetSize(*patch,FALSE);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchRestore(patchData,this,patch));
		patchData->vdelta.Zero();		// Reset all deltas
		patchData->ClearHandleFlag();
		SetAFlag(A_HELD);
	}
}

void EditPatchMod::EndPasteTangent(EditPatchData *patchData)
{
	if (!ip || !patchData) return;

	TimeValue t = ip->GetTime();
	PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
	if(!patch) return;

	patchData->UpdateChanges(patch);
	patchData->TempData(this)->Invalidate(PART_GEOM);

	theHold.Accept(GetString(IDS_TH_PASTE_TANGENTS));
	
	ip->RedrawViews(t, REDRAW_NORMAL);
}

BOOL EditPatchMod::PasteTangent(PatchMesh *patch, int vec)
{
	if (!patch) return FALSE;

	DbgAssert(vec < patch->getNumVecs());
	if (vec >= patch->getNumVecs()) return FALSE;

	BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
	TimeValue t = ip->GetTime();
	PatchVec &phVec = patch->getVec(vec);
	if (phVec.vert < 0) return FALSE;

	PatchVert &phVert = patch->getVert(phVec.vert);
	Point3 pastedTangent = copiedTangent;
	Point3 knotPt = phVert.p;
	Point3 vecPt1 = phVec.p;
	Point3 oldVec1 = vecPt1 - knotPt;
	float oldLen1 = Length(oldVec1);
	if (copyTanLength == FALSE) {
		float tanLen = Length(pastedTangent);
		if (tanLen != 0.0f)
			pastedTangent *= (oldLen1 / tanLen);
	}

	patch->setVec(vec, knotPt + pastedTangent);

	// turn to corner type if shift key is pressed
	if (shiftPressed)
		phVert.flags &= ~PVERT_COPLANAR;
	
	// If locked handles or a coplanar knot, do the other vectors!
	// Turn the movement into a transformation matrix and transform all the handles attached to the owner vertex
	if (lockedHandles || ((phVert.flags & PVERT_COPLANAR) && (phVert.vectors.Count() > 2))) {
		Point3 oldVec = Normalize(oldVec1);
		Point3 newVec = Normalize(pastedTangent);
		Matrix3 rotMat(1);
		int vectors = phVert.vectors.Count();

		// Get a matrix that will transform the old point to the new one
		// Cross product gives us the normal of the rotational axis
		Point3 axis = Normalize(oldVec ^ newVec);
		// Dot product gives us the angle
		float dot = DotProd(oldVec, newVec);
		float angle = (dot >= -1.0f) ? ((dot <= 1.0f) ? (float)-acos(dot) : 0.0f) : PI;

		// Watch out for cases where the vectors are exactly the opposite --
		// This results in an invalid axis for transformation!
		// In this case, we look for a vector to one of the other handles that
		// will give us a useful vector for the rotational axis
		if (newVec == -oldVec) {
			for (int v = 0; v < vectors; v++) {
				int vi = phVert.vectors[v];
				if (vi != vec) {
					Point3 testVec = patch->vecs[vi].p - knotPt;
					if (testVec != Point3::Origin) {
						testVec = Normalize(testVec);
						if ((testVec != newVec) && (testVec != oldVec)) {
							// Cross product gives us the normal of the rotational axis
							axis = Normalize(testVec ^ newVec);
							// The angle is 180 degrees
							angle = PI;
							break;
						}
					}
				}
			}
		}
		
		// Now let's build a matrix that'll do this for us!
		if (angle != 0.0f) {
			Quat quat = QFromAngAxis(angle, axis);
			quat.MakeMatrix(rotMat);
		}

		// Process all other handles through the matrix
		for (int v = 0; v < vectors; v++) {
			int vi = phVert.vectors[v];
			if (vi != vec) {
				patch->setVec(vi, (patch->vecs[vi].p - knotPt) * rotMat + knotPt);
			}
		}
	}

	// Really only need to do this if neighbor knots are non-bezier
	patch->computeInteriors();
	patch->InvalidateGeomCache();

	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	return TRUE;
}

HCURSOR EPM_PasteTangentMouseProc::GetTransformCursor()
	{ 
	static HCURSOR hCur = NULL;

	if ( !hCur )
		hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_PASTE_TANGENT_CUR));

	return hCur;
	}

HitRecord* EPM_PasteTangentMouseProc::HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags )
	{
	HitRecord *bestRec = NULL;

	vpt->ClearSubObjHitList();
	SetPatchHitOverride(PO_HANDLE);
	ip->SubObHitTest(ip->GetTime(),type,ip->GetCrossing(),flags,p,vpt);
	ClearPatchHitOverride();
	
	if ( vpt->NumSubObjHits() ) {
		HitLog &hits = vpt->GetSubObjHitList();
		DWORD best;
		for (HitRecord *rec = hits.First(); rec != NULL; rec = rec->Next()) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			if(hit->type != PATCH_HIT_VECTOR) continue;
			if(bestRec == NULL || rec->distance < best) {
				best = rec->distance;
				bestRec = rec;
				}
			}
		}
	return bestRec;
	}

int EPM_PasteTangentMouseProc::proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m )
	{
	ViewExp *vpt = NULL;
	HitRecord *rec = NULL;

	switch ( msg ) {
		case MOUSE_ABORT:
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			vpt = ip->GetViewport(hwnd);
			if((rec = HitTest(vpt,&m,HITTYPE_POINT,0)) != NULL) {
				PatchHitData *hit = ((PatchHitData *)rec->hitData);
				EditPatchData *patchData = (EditPatchData*)rec->modContext->localData;
				if ( patchData ) {
					po->StartPasteTangent(patchData);
					PatchMesh *patch = patchData->TempData(po)->GetPatch(ip->GetTime());
					po->PasteTangent(patch, hit->index);
					po->EndPasteTangent(patchData);
					}
				}
			ip->ReleaseViewport(vpt);
			break;
		
		case MOUSE_MOVE:
		case MOUSE_FREEMOVE:
			vpt = ip->GetViewport(hwnd);
			if ( HitTest(vpt,&m,HITTYPE_POINT,HIT_ABORTONHIT) )
				SetCursor(GetTransformCursor());
			else
				SetCursor(LoadCursor(NULL,IDC_ARROW));
			ip->ReleaseViewport(vpt);
			break;
		}

	return FALSE;
	}


// --------------------------------------------------------------------
// IPatchSelect and IPatchOps interfaces   (JBW 2/2/99)

void* EditPatchMod::GetInterface(ULONG id) 
{
	switch (id)
	{
		case I_PATCHSELECT: return (IPatchSelect*)this;
		case I_PATCHSELECTDATA: return (IPatchSelectData*)this;
		case I_PATCHOPS: return (IPatchOps*)this;
		case I_SUBMTLAPI: return (ISubMtlAPI*)this;
	}
	return Modifier::GetInterface(id);
}

void EditPatchMod::StartCommandMode(patchCommandMode mode)
{
	switch (mode)
	{
		case PcmAttach:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ATTACH, 0);
			break;
		case PcmExtrude:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_EDGE && GetSubobjectType() != PO_HANDLE) // LAM - added SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_EXTRUDE, 0);
			break;
		case PcmBevel:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE) // LAM - added SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_BEVEL, 0);
			break;
		case PcmBind:
			if (hOpsPanel != NULL && GetSubobjectType() == PO_VERTEX) // LAM - added SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_BIND, 0);
			break;
// LAM: added following 9/2/00
		case PcmCreate:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == PO_VERTEX || (GetSubobjectLevel() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_CREATE, 0);
			break;
		case PcmWeldTarget:
			if (hOpsPanel != NULL && GetSubobjectType() == PO_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_VERTWELD, 0);
			break;
		case PcmFlipNormal:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_NORMAL_FLIPMODE, 0);
			break;
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		case PcmCopyTangent:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == PO_VERTEX || GetSubobjectLevel() == PO_HANDLE))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_COPY_TANGENT, 0);
			break;
		case PcmPasteTangent:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == PO_VERTEX || GetSubobjectLevel() == PO_HANDLE) && tangentCopied)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_PASTE_TANGENT, 0);
			break;
	}
}

void EditPatchMod::ButtonOp(patchButtonOp opcode)
{
	switch (opcode)
	{
		case PopUnbind:
			if (hOpsPanel != NULL && GetSubobjectType() == PO_VERTEX) // LAM - added SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_UNBIND, 0);
			break;
		case PopHide:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_VERTEX && GetSubobjectType() != PO_HANDLE) // LAM - added SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_HIDE, 0);
			break;
		case PopUnhideAll:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_UNHIDE, 0);
			break;
		case PopWeld:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == PO_VERTEX || GetSubobjectLevel() == PO_EDGE))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_WELD, 0);
			break;
		case PopDelete:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_VERTEX && GetSubobjectType() != PO_HANDLE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_PATCH_DELETE, 0);
			break;
		case PopSubdivide:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_EDGE && GetSubobjectType() != PO_HANDLE) // && GetSubobjectType() <= PO_PATCH) // LAM - modified SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_SUBDIVIDE, 0);
			break;
		case PopAddTri:
			if (hOpsPanel != NULL && GetSubobjectLevel() == PO_EDGE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ADDTRI, 0);
			break;
		case PopAddQuad:
			if (hOpsPanel != NULL && GetSubobjectLevel() == PO_EDGE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ADDQUAD, 0);
			break;
		case PopDetach:
			if (hOpsPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE) // LAM - modified SO test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_DETACH, 0);
			break;
		// CAL-04/23/03: add patch smooth
		case PopPatchSmooth:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_EP_PATCH_SMOOTH, 0);
			break;
		// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
		case PopSelectionShrink:
			if (hSelectPanel != NULL && GetSubobjectType() != PO_OBJECT && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSelectPanel, WM_COMMAND, IDC_EP_SELECTION_SHRINK, 0);
			break;
		case PopSelectionGrow:
			if (hSelectPanel != NULL && GetSubobjectType() != PO_OBJECT && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSelectPanel, WM_COMMAND, IDC_EP_SELECTION_GROW, 0);
			break;
		case PopEdgeRingSel:
			if (hSelectPanel != NULL && GetSubobjectType() == PO_EDGE)
				PostMessage(hSelectPanel, WM_COMMAND, IDC_EP_EDGE_RING_SEL, 0);
			break;
		case PopEdgeLoopSel:
			if (hSelectPanel != NULL && GetSubobjectType() == PO_EDGE)
				PostMessage(hSelectPanel, WM_COMMAND, IDC_EP_EDGE_LOOP_SEL, 0);
			break;
// LAM: added following 9/2/00
		case PopSelectOpenEdges:
			if (hSelectPanel != NULL && GetSubobjectType() == PO_EDGE)
				PostMessage(hSelectPanel, WM_COMMAND, IDC_SELECT_OPEN_EDGES, 0);
			break;
		case PopBreak:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == PO_VERTEX || GetSubobjectLevel() == PO_EDGE))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_BREAK, 0);
			break;
		case PopCreateShapeFromEdges:
			if (hOpsPanel != NULL && GetSubobjectType() == PO_EDGE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_CREATE_SHAPE, 0);
			break;
		case PopFlipNormal:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_NORMAL_FLIP, 0);
			break;
		case PopUnifyNormal:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_NORMAL_UNIFY, 0);
			break;
		case PopSelectByID:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_SELECT_BYID, 0);
			break;
		case PopSelectBySG:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_SELECTBYSMOOTH, 0);
			break;
		case PopClearAllSG:
			if (hSurfPanel != NULL && GetSubobjectType() >= PO_PATCH && GetSubobjectType() != PO_HANDLE)
				PostMessage(hSurfPanel, WM_COMMAND, IDC_SMOOTH_CLEAR, 0);
			break;
		// CAL-05/06/03: shaded face toggle. (FID #1914)
		case PopShadedFaceToggle:
			if (hSoftSelPanel != NULL && GetSubobjectType() != PO_OBJECT && GetSubobjectType() != PO_HANDLE && UseSoftSelections())
				PostMessage(hSoftSelPanel, WM_COMMAND, IDC_SHADED_FACE_TOGGLE, 0);
			break;
	}
}

// LAM: added following 9/3/00 - just stubs for now....
//---------------------------------------------------------
//  UI-related methods - examples of how to do it....

void EditPatchMod::GetUIParam (patchUIParam uiCode, float & ret) {
	if (!ip) return;
	if (!Editing()) return;

//	switch (uiCode) {
//	case MuiPolyThresh:
//		ret = planarFaceThresh;
//		break;
//	case MuiFalloff:
//		ret = falloff;
//		break;
//	}
}

void EditPatchMod::SetUIParam (patchUIParam uiCode, float val) {
	if (!ip) return;
	if (!Editing()) return;
//	ISpinnerControl *spin;

//	switch (uiCode) {
//	case MuiPolyThresh:
//		planarFaceThresh = val;
//		if (hSel) {
//			spin = GetISpinner (GetDlgItem (hSel, IDC_PLANARSPINNER));
//			spin->SetValue (val, FALSE);
//			ReleaseISpinner (spin);
//		}
//		break;
//	case MuiFalloff:
//		falloff = val;
//		InvalidateAffectRegion ();
//		if (hAR) {
//			spin = GetISpinner (GetDlgItem (hAR, IDC_FALLOFFSPIN));
//			spin->SetValue (val, FALSE);
//			ReleaseISpinner (spin);
//		}
//		break;
//	}
}

void EditPatchMod::GetUIParam (patchUIParam uiCode, int & ret) {
	if (!ip) return;
	if (!Editing()) return;

//	switch (uiCode) {
//	case MuiSelByVert:
//		ret = selByVert;
//		break;
//	case MuiIgBack:
//		ret = ignoreBackfaces;
//		break;
//	}
}

void EditPatchMod::SetUIParam (patchUIParam uiCode, int val) {
	if (!ip) return;
	if (!Editing()) return;
//	ISpinnerControl *spin;

//	switch (uiCode) {
//	case MuiSelByVert:
//		selByVert = val ? TRUE : FALSE;
//		if (hSel) CheckDlgButton (hSel, IDC_SEL_BYVERT, selByVert);
//		break;
//	case MuiSoftSel:
//		affectRegion = val ? TRUE : FALSE;
//		if (hAR) {
//			CheckDlgButton (hAR, IDC_AFFECT_REGION, affectRegion);
//			SetARDlgEnables ();
//		}
//		break;
//	case MuiSSUseEDist:
//		useEdgeDist = val ? TRUE : FALSE;
//		if (hAR) {
//			CheckDlgButton (hAR, IDC_E_DIST, useEdgeDist);
//			spin = GetISpinner (GetDlgItem (hAR, IDC_E_ITER_SPIN));
//			spin->Enable (useEdgeDist);
//			ReleaseISpinner (spin);
//		}
//		break;
//	case MuiExtrudeType:
//		extType = val;
//		if (hGeom) {
//			if (extType == MESH_EXTRUDE_CLUSTER) {
//				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_A);
//			} else {
//				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_B);
//			}
//		}
//		break;
//	}
}

DWORD EditPatchMod::GetSelLevel()
{
	return GetSubobjectLevel();
}

void EditPatchMod::SetSelLevel(DWORD level)
{	SetSubobjectLevel(level);
}

void EditPatchMod::LocalDataChanged()
{
}

Color EditPatchMod::GetVertColor(int mp, bool *differs) {
	ModContextList mcList;	
	INodeTab nodes;
	static Color white(1,1,1), black(0,0,0);

	Color col=white;
	BOOL init=FALSE;
	if (differs) *differs = false;

	if (!ip) return white;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
//		patchData->BeginEdit(ip->GetTime());
		if(!patch) continue;
		if (patch->vertSel.NumberSet() == 0) continue;

		TVPatch *cp = patch->mapPatches(mp);
		PatchTVert *cv = patch->mapVerts(mp);
		if (!cp || !cv) {
			if (init) {
				if (col == white) continue;
				if (differs) *differs = true;
				nodes.DisposeTemporary ();
				return black;
			} else {
				init = true;
				col = white;
				}
			continue;
			}

		for (int i=0; i<patch->getNumPatches(); i++) {
			int *tt = cp[i].tv;
			Patch &p = patch->patches[i];
			for (int j=0; j<p.type; j++) {
				if (!patch->vertSel[p.v[j]]) continue;
				if (!init) {
					col = cv[tt[j]].p;
					init = TRUE;
					}
				else {
					Color ac = cv[tt[j]].p;
					if (ac!=col) {
						if (differs) *differs = true;
						nodes.DisposeTemporary();
						return black;
						}
					}
				}
			}
		}
	nodes.DisposeTemporary();
	return col;
	}

void EditPatchMod::SetVertColor(Color clr, int mp) {
	ModContextList mcList;	
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch) continue;

		if (patch->vertSel.NumberSet()) {
			// Start a restore object...
			if (theHold.Holding()) {
				theHold.Put(new PatchRestore(patchData,this,patch,"SetVertColor"));
				}
			// Make sure we have map channel
			patch->setMapSupport (mp, TRUE);
			TVPatch *cp = patch->mapPatches(mp);
			PatchTVert *cv = patch->mapVerts(mp);
			if(!cp || !cv) {
				assert(0);		// Should never happen
				nodes.DisposeTemporary();
				return;
				}
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"SetVertColor"));
			for (int i=0; i<patch->getNumPatches(); i++) {
				int *tt = cp[i].tv;
				Patch &p = patch->patches[i];
				for (int j=0; j<p.type; j++) {
					if (patch->vertSel[p.v[j]])
						cv[tt[j]] = clr;
					}
				}
			patch->InvalidateGeomCache();
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

void EditPatchMod::SelectVertByColor(VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp) {
	ModContextList mcList;
	INodeTab nodes;

	float dr = float(deltaR)/255.0f;
	float dg = float(deltaG)/255.0f;
	float db = float(deltaB)/255.0f;

	theHold.Begin();
	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch) continue;
		TVPatch *cp = patch->mapPatches(mp);
		PatchTVert *cv = patch->mapVerts(mp);

		BitArray nvs;
		if (add || sub) {
			nvs = patch->vertSel;
			nvs.SetSize (patch->numVerts, TRUE);
		} else {
			nvs.SetSize (patch->numVerts);
			nvs.ClearAll();
		}

		Point3 col(1,1,1);
		for (int i=0; i<patch->getNumPatches(); i++) {
			Patch &p = patch->patches[i];
			for (int j=0; j<p.type; j++) {
				if (cv && cp) col = cv[cp[i].tv[j]];
				if ((float)fabs(col.x-clr.x) > dr) continue;
				if ((float)fabs(col.y-clr.y) > dg) continue;
				if ((float)fabs(col.z-clr.z) > db) continue;
				if (sub) nvs.Clear(patch->patches[i].v[j]);
				else nvs.Set(patch->patches[i].v[j]);
				}
			}
		ip->ClearCurNamedSelSet();
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData,this,patch));
		patch->vertSel = nvs;
		patchData->UpdateChanges(patch, FALSE);
		}
	PatchSelChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	theHold.Accept(GetString(IDS_RB_SELBYCOLOR));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
	nodes.DisposeTemporary();
	}

Color EditPatchMod::GetPatchColor (int mp, bool *differs) {
	ModContextList mcList;	
	INodeTab nodes;
	static Color white(1,1,1), black(0,0,0);

	Color col=white;
	BOOL init=FALSE;
	if (differs) *differs = false;

	if (!ip) return white;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
//		patchData->BeginEdit(ip->GetTime());
		if(!patch) continue;
		TVPatch *cp = patch->mapPatches(mp);
		PatchTVert *cv = patch->mapVerts(mp);
		if (!cp || !cv) {
			if (!patch->patchSel.NumberSet()) continue;
			if (init) {
				if (col != white) {
					if (differs) *differs = true;
					nodes.DisposeTemporary ();
					return black;
				}
			} else {
				init = TRUE;
				col = white;
			}
			continue;
			}

		for (int i=0; i<patch->getNumPatches(); i++) {
			if(!patch->patchSel[i]) continue;
			int *tt = cp[i].tv;
			for (int j=0; j<patch->patches[i].type; j++) {
				if (!init) {
					col = cv[tt[j]].p;
					init = TRUE;
					}
				else {
					Color ac = cv[tt[j]].p;
					if (ac!=col) {
						if (differs) *differs = true;
						nodes.DisposeTemporary();
						return black;
						}
					}
				}
			}
		}
	nodes.DisposeTemporary();
	return col;
	}

void EditPatchMod::SetPatchColor(Color clr, int mp) {
	ModContextList mcList;	
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);	

	for (int i = 0; i < mcList.Count(); i++) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if (!patchData) continue;
		if (patchData->GetFlag(EPD_BEENDONE)) continue;
		patchData->BeginEdit(ip->GetTime());
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());		
		if(!patch)
			continue;

		// Make sure we support map channel
		patch->setMapSupport(mp, TRUE);
		if (patch->patchSel.NumberSet()) {
			// Start a restore object...
			patchData->PreUpdateChanges(patch, FALSE);
			if (theHold.Holding()) {
				theHold.Put(new PatchRestore(patchData,this,patch,"SetPatchColor"));
				}
			// Use the special UVW mapping option built into the PatchMesh class
			patch->ApplyUVWMap(0, 1.0f, 1.0f, 1.0f, 0, 0, 0, 0, Matrix3(FALSE), mp);

			TVPatch *cp = patch->mapPatches(mp);
			PatchTVert *cv = patch->mapVerts(mp);
			if(!cp || !cv) {
				assert(0);		// Should never happen
				return;
				}
			for (int i=0; i<patch->getNumPatches(); i++) {
				if(!patch->patchSel[i]) continue;
				int *tt = cp[i].tv;
				for (int j=0; j<patch->patches[i].type; j++)
					cv[tt[j]] = clr;
				}
			patch->InvalidateGeomCache();
			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

MtlID EditPatchMod::GetNextAvailMtlID(ModContext* mc) {
	if(!mc)
		return 1;
	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return 1;
	
	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1) {
		int i;
 		
		MtlID min, max;
		BOOL first = TRUE;

		for(int p = 0; p < patch->numPatches; ++p) {
			MtlID thisID = patch->getPatchMtlIndex(p);
			if(first) {
				min = max = thisID;
				first = FALSE;
				}
			else
			if(thisID < min)
				min = thisID;
			else
			if(thisID > max)
				max = thisID;
			}
		// If room below, return it
		if(min > 0)
			return min - 1;
		// Build a bit array to find any gaps		
		BitArray b;
		int bits = max - min + 1;
		b.SetSize(bits);
		b.ClearAll();
		for(p = 0; p < patch->numPatches; ++p)
			b.Set(patch->getPatchMtlIndex(p) - min);
		for(i = 0; i < bits; ++i) {
			if(!b[i])
				return (MtlID)(i + min);
			}
		// No gaps!  If room above, return it
		if(max < 65535)
			return max + 1;
		}
	return (MtlID)mtlID;
	}

BOOL EditPatchMod::HasFaceSelection(ModContext* mc) {
	// Are we the edited object?
	if (ip == NULL)  return FALSE;

	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return FALSE;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return FALSE;

	// Is Patch selection active?
	if (GetSubobjectType() == EP_PATCH && patch->patchSel.NumberSet()) return TRUE;
	
	return FALSE;
	}

void EditPatchMod::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) {
	int altered = 0;
	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return;
	
	// If this is the first edit, then the delta arrays will be allocated
	patchData->BeginEdit(ip->GetTime());

	patchData->PreUpdateChanges(patch, FALSE);
	if ( theHold.Holding() )
		theHold.Put(new PatchRestore(patchData,this,patch));

	for(int p = 0; p < patch->numPatches; ++p) {
		if(patch->patchSel[p]) {
			altered = TRUE;
			patch->setPatchMtlIndex(p, id);
			}
		}

	if(altered)	{
		patchData->UpdateChanges(patch, FALSE);
		InvalidateSurfaceUI();
		}

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

int	EditPatchMod::GetSelFaceUniqueMtlID(ModContext* mc) {
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);
	if (mtlID == -1) return mtlID;

	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return 1;

	for(int p = 0; p < patch->numPatches; ++p) {
		if(patch->patchSel[p])
			continue;
		if(patch->getPatchMtlIndex(p) != mtlID)
			continue;
		mtlID = -1;
		}
	return mtlID;
	}

int	EditPatchMod::GetSelFaceAnyMtlID(ModContext* mc) {
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;

	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return 1;

	for(int p = 0; p < patch->numPatches; ++p) {
		if(!patch->patchSel[p])
			continue;
		if (bGotFirst) {
			if (mtlID != patch->getPatchMtlIndex(p)) {
				mtlID = -1;
				break;
				}
			}
		else {
			mtlID = patch->getPatchMtlIndex(p);
			bGotFirst = TRUE;
			}
		}
	return mtlID;
	}

int	EditPatchMod::GetMaxMtlID(ModContext* mc) {
	MtlID mtlID = 0;

	EditPatchData *patchData = (EditPatchData*)mc->localData;
	if ( !patchData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
	if(!patch) return 1;

	for(int p = 0; p < patch->numPatches; ++p)
		mtlID = max(mtlID, patch->getPatchMtlIndex(p));

	return mtlID;
	}

/*-------------------------------------------------------------------*/

void EPM_BindCMode::EnterMode()
	{
	if ( pobj->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_BIND));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void EPM_BindCMode::ExitMode()
	{
	if ( pobj->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_BIND));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}
/*
void EditPatchMod::DoAddHook(int vert1, int seg1) {

	// If any bits are set in the selection set, let's DO IT!!
	if ( !ip ) return;
	theHold.Begin();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if ( theHold.Holding() )
		theHold.Put(new PatchObjectRestore(this,rec));
		// Call the patch type change function

	patch.AddHook(vert1,seg1);
	patch.computeInteriors();
	patch.InvalidateGeomCache();
	InvalidateMesh();
	theHold.Accept(GetResString(IDS_TH_PATCHCHANGE));

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

	}
*./

/*-------------------------------------------------------------------*/

HCURSOR EPM_BindMouseProc::GetTransformCursor() 
	{ 
	static HCURSOR hCur = NULL;

	if ( !hCur ) {
		hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_SEGREFINECUR)); 
		}

	return hCur; 
	}

BOOL EPM_BindMouseProc::HitTest( 
		ViewExp *vpt, IPoint2 *p, int type, int flags, int subType  )
	{
	vpt->ClearSubObjHitList();
	SetPatchHitOverride(subType);

	ip->SubObHitTest(ip->GetTime(),type,ip->GetCrossing(),flags,p,vpt);
	ClearPatchHitOverride();
	if ( vpt->NumSubObjHits() ) {
		return TRUE;
	} else {
		return FALSE;
		}			
	}

BOOL EPM_BindMouseProc::HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert) {
	int first = 1;
	
	if(HitTest(vpt, p, HITTYPE_POINT, 0,1) ) {
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while(rec) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
//			if( patch == hit->patch) {
			pMesh = hit->patch;
			if( 1) {
				if (hit->type == PATCH_HIT_VERTEX)
					{

				// If there's an exclusion shape, the vert & poly can't be the same!
					if(first || rec->distance < best) 
						{
						first = 0;
						best = rec->distance;
						bestRec = rec;
						}
					}
				}
			rec = rec->Next();
			}
		if(!first) {
			PatchHitData *hit = ((PatchHitData *)bestRec->hitData);
			*vert = hit->index;
			return TRUE;
			}
		}
	return FALSE;
	}


BOOL EPM_BindMouseProc::HitASegment(ViewExp *vpt, IPoint2 *p, int *seg) {
	int first = 1;
	
	if(HitTest(vpt, p, HITTYPE_POINT, 0,2) ) {
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while(rec) {
			PatchHitData *hit = ((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
			if( pMesh == hit->patch) {
				if (hit->type == PATCH_HIT_EDGE)
					{

				// If there's an exclusion shape, the vert & poly can't be the same!
					if(first || rec->distance < best) 
						{
						first = 0;
						best = rec->distance;
						bestRec = rec;
						}
					}
				}
			rec = rec->Next();
			}
		if(!first) {
			PatchHitData *hit = ((PatchHitData *)bestRec->hitData);
			*seg = hit->index;
			return TRUE;
			}
		}
	return FALSE;
	}


static void PatchXORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, ComputeViewportXORDrawColor() ) );
	MoveToEx( hdc, p0.x, p0.y, NULL );
	LineTo( hdc, p1.x, p1.y );		
	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}


int EPM_BindMouseProc::proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m )
	{
	ViewExp *vpt = ip->GetViewport(hwnd);	
	int res = TRUE;
	static PatchMesh *shape1 = NULL;
	static int poly1, vert1, seg1;
	static IPoint2 anchor, lastPoint;

	switch ( msg ) {
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			switch(point) {
				case 0:
					{
					if(HitAKnot(vpt, &m,  &vert1))
						{
						res = TRUE;
						anchor = lastPoint = m;
						PatchXORDottedLine(hwnd, anchor, m);	// Draw it!
//get valid seg list
						knotList.SetSize(pMesh->numVerts);
						knotList.ClearAll();
						for (int i = 0; i < pMesh->numEdges; i++)
							{
							if (pMesh->edges[i].v1 == vert1) 
								{
								knotList.Set(pMesh->edges[i].v2);

								}
							if (pMesh->edges[i].v2 == vert1) 
								{
								knotList.Set(pMesh->edges[i].v1);
								}
							}
						}
					else res = FALSE;

					break;
					}
				case 1:
					PatchXORDottedLine(hwnd, anchor, lastPoint);	// Erase it!
//					if(HitAnEndpoint(vpt, &m, shape1, poly1, vert1, NULL, &poly2, &vert2))
//						ss->DoVertConnect(vpt, shape1, poly1, vert1, poly2, vert2); 
					if(HitASegment(vpt, &m,  &seg1))
						{
//if a valid segemtn change cursor
						int a = pMesh->edges[seg1].v1;
						int b = pMesh->edges[seg1].v2;
						if (knotList[a] && knotList[b])
							pobj->DoAddHook(pMesh,vert1,seg1);

						}
					res = FALSE;
					break;
				default:
					assert(0);
				}
			break;

		case MOUSE_MOVE:
			// Erase old dotted line
			PatchXORDottedLine(hwnd, anchor, lastPoint);
			// Draw new dotted line
			PatchXORDottedLine(hwnd, anchor, m);
			lastPoint = m;
			if(HitASegment(vpt, &m,  &seg1))
				{
//if a valid segemtn change cursor
				int a = pMesh->edges[seg1].v1;
				int b = pMesh->edges[seg1].v2;
				if (knotList[a] && knotList[b])
					SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				else SetCursor(LoadCursor(NULL,IDC_ARROW));

				}
			else {
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				}

			break;
					
		case MOUSE_FREEMOVE:
			if(HitAKnot(vpt, &m,  &vert1))
				{
				SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				}
			else {
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
/*
			if ( HitTest(vpt,&m,HITTYPE_POINT,HIT_ABORTONHIT,1) ) {
				HitLog &hits = vpt->GetSubObjHitList();
				HitRecord *rec = hits.First();
				if (rec )
					{
					SetCursor(LoadCursor(hInstance,MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
					}
				}
			SetCursor(LoadCursor(NULL,IDC_ARROW));
*/
			break;
		
		case MOUSE_ABORT:
			// Erase old dotted line
			PatchXORDottedLine(hwnd, anchor, lastPoint);
			break;			
		}

	if ( vpt ) ip->ReleaseViewport(vpt);
	return res;
	}

/*-------------------------------------------------------------------*/



//watje new patch mapping

void EditPatchMod::ChangeMappingTypeLinear(BOOL linear) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
			altered = holdNeeded = TRUE;
			patchData->PreUpdateChanges(patch, FALSE);
			if ( theHold.Holding() )
				theHold.Put(new PatchRestore(patchData,this,patch,"ChangeSelPatches"));
	// Call the map change function
				if (linear)
					patch->ChangePatchToLinearMapping(-1);
				else patch->ChangePatchToCurvedMapping(-1);

			patchData->UpdateChanges(patch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


BOOL EditPatchMod::CheckMappingTypeLinear() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	BOOL linear=TRUE;

	if ( !ip ) return FALSE;

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		// If any bits are set in the selection set, let's DO IT!!
		if(patch->patchSel.NumberSet()) {
	// Call the map change function
			BOOL cLinear = patch->ArePatchesLinearMapped(-1);
			if (cLinear == FALSE)
				linear = FALSE;

			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}
	

	nodes.DisposeTemporary();
	return linear;
	}

BOOL EditPatchMod::SingleEdgesOnly() {
	if ( !ip ) return FALSE;

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;		
		if(!patch->SingleEdgesOnly()) {
			nodes.DisposeTemporary();
			return FALSE;
			}
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}

	nodes.DisposeTemporary();
	return TRUE;
	}

void EditPatchMod::MaybeClonePatchParts() {
	if(!ip)
		return;
	int type = GetSubobjectType();
	if(type != EP_PATCH && type != EP_EDGE)
		return;
	BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
	if(shiftPressed) {
		ModContextList mcList;		
		INodeTab nodes;
		BOOL holdNeeded = FALSE;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList,nodes);
		ClearPatchDataFlag(mcList,EPD_BEENDONE);

		theHold.Begin();
		RecordTopologyTags();
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
			if ( !patchData ) continue;
			if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

			// If the mesh isn't yet cache, this will cause it to get cached.
			PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
			if(!patch)
				continue;
			
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);

			if(type == EP_PATCH && patch->patchSel.NumberSet()) {
				holdNeeded = TRUE;
				patchData->PreUpdateChanges(patch);
				if ( theHold.Holding() )
					theHold.Put(new PatchRestore(patchData,this,patch,"MaybeClone"));
				patch->ClonePatchParts();
				patchData->UpdateChanges(patch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
				}
			else
			if(type == EP_EDGE && patch->edgeSel.NumberSet()) {
				holdNeeded = TRUE;
				patchData->PreUpdateChanges(patch);
				if ( theHold.Holding() )
					theHold.Put(new PatchRestore(patchData,this,patch,"MaybeClone"));
				patch->CreateExtrusion(PATCH_EDGE, FALSE);
				// Select the new edges
				patch->edgeSel.ClearAll();
				for(int i = 0; i < patch->newEdges.Count(); ++i)
					patch->edgeSel.Set(patch->newEdges[i]);
				BitArray autos(patch->numEdges);
				autos.ClearAll();
				// Now mark the edges that are part of the extrusion but not selected -- These need
				// to be automatically computed to give the vectors some length
				for(int jk = 0; jk < patch->newEdges.Count(); ++jk) {
					PatchEdge &e = patch->edges[patch->newEdges[jk]];
					// This edge had better have only one patch attached!
					assert(e.patches.Count() == 1);
					Patch &p = patch->patches[e.patches[0]];
					for(int j = 0; j < p.type; ++j) {
						if(p.edge[j] == patch->newEdges[jk]) {
							int prevEdge = p.edge[(j + p.type - 1) % p.type];
							int nextEdge = p.edge[(j + 1) % p.type];
							if(!autos[prevEdge]) {
								autos.Set(prevEdge);
								patchData->autoEdges.Append(1, &prevEdge);
								}
							if(!autos[nextEdge]) {
								autos.Set(nextEdge);
								patchData->autoEdges.Append(1, &nextEdge);
								}
							}
						}
					}
				patchData->UpdateChanges(patch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
				}

			patchData->SetFlag(EPD_BEENDONE,TRUE);
			}

		if(holdNeeded) {
			ResolveTopoChanges();
			theHold.Accept(GetString(IDS_TH_COPY_PATCH));
			}
		else {
			theHold.End();
			}

		nodes.DisposeTemporary();
		ClearPatchDataFlag(mcList,EPD_BEENDONE);
		}
	}

int EditPatchMod::NumSubObjTypes() 
{ 
	return 5;
}

ISubObjType *EditPatchMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		SOT_Handle.SetName(GetString(IDS_TH_HANDLE));
		SOT_Vertex.SetName(GetString(IDS_TH_VERTEX));
		SOT_Edge.SetName(GetString(IDS_TH_EDGE));
		SOT_Patch.SetName(GetString(IDS_TH_PATCH));
		SOT_Element.SetName(GetString(IDS_TH_ELEMENT));
	}

	switch(i)
	{
	case -1:
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
		break;
	case 0:
		return &SOT_Vertex;
	case 1:
		return &SOT_Edge;
	case 2:
		return &SOT_Patch;
	case 3:
		return &SOT_Element;
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case 4:
		return &SOT_Handle;
	}

	return NULL;
}


// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
void EditPatchMod::ShrinkSelection(int type)
{
	if ( !ip || type == EP_OBJECT ) return;

	ModContextList mcList;
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	ip->ClearCurNamedSelSet();

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData, this, patch));

		switch (type)
		{
		case EP_VERTEX:
			patch->ShrinkSelection(PATCH_VERTEX);
			break;
		case EP_EDGE:
			patch->ShrinkSelection(PATCH_EDGE);
			break;
		case EP_PATCH:
			patch->ShrinkSelection(PATCH_PATCH);
			break;
		}

		patchData->UpdateChanges(patch, FALSE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		holdNeeded = TRUE;
	}

	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	nodes.DisposeTemporary();

	if(holdNeeded) {
		theHold.Accept(GetString(IDS_DS_SELECT));
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		PatchSelChanged();
	} else {
		theHold.End();
	}
}

void EditPatchMod::GrowSelection(int type)
{
	if ( !ip || type == EP_OBJECT ) return;

	ModContextList mcList;
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	ip->ClearCurNamedSelSet();

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData, this, patch));

		switch (type)
		{
		case EP_VERTEX:
			patch->GrowSelection(PATCH_VERTEX);
			break;
		case EP_EDGE:
			patch->GrowSelection(PATCH_EDGE);
			break;
		case EP_PATCH:
			patch->GrowSelection(PATCH_PATCH);
			break;
		}

		patchData->UpdateChanges(patch, FALSE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		holdNeeded = TRUE;
	}

	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	nodes.DisposeTemporary();

	if(holdNeeded) {
		theHold.Accept(GetString(IDS_DS_SELECT));
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		PatchSelChanged();
	} else {
		theHold.End();
	}
}

void EditPatchMod::SelectEdgeRing()
{
	if ( !ip ) return;

	ModContextList mcList;
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	ip->ClearCurNamedSelSet();

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData, this, patch));
		patch->SelectEdgeRing(patch->edgeSel);
		patchData->UpdateChanges(patch, FALSE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		holdNeeded = TRUE;
	}

	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	nodes.DisposeTemporary();

	if(holdNeeded) {
		theHold.Accept(GetString(IDS_DS_SELECT));
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		PatchSelChanged();
	} else {
		theHold.End();
	}
}

void EditPatchMod::SelectEdgeLoop()
{
	if ( !ip ) return;

	ModContextList mcList;
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	ip->ClearCurNamedSelSet();

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData, this, patch));
		patch->SelectEdgeLoop(patch->edgeSel);
		patchData->UpdateChanges(patch, FALSE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		holdNeeded = TRUE;
	}

	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	nodes.DisposeTemporary();

	if(holdNeeded) {
		theHold.Accept(GetString(IDS_DS_SELECT));
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		PatchSelChanged();
	} else {
		theHold.End();
	}
}


void EditPatchMod::SelectOpenEdges()
	{
	if ( !ip ) return; 
	ModContextList mcList;		
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList,nodes);
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	ip->ClearCurNamedSelSet();

	theHold.Begin();
	RecordTopologyTags();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData *patchData = (EditPatchData*)mcList[i]->localData;
		if ( !patchData ) continue;
		if ( patchData->GetFlag(EPD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		if(!patch)
			continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		patchData->PreUpdateChanges(patch, FALSE);
		if ( theHold.Holding() )
			theHold.Put(new PatchSelRestore(patchData,this,patch));
		holdNeeded = TRUE;
		patch->edgeSel.ClearAll();
		for(int i = 0; i < patch->numEdges; ++i) {
			PatchEdge &e = patch->edges[i];
			if(e.patches.Count() < 2)
				patch->edgeSel.Set(i);
			}

		patchData->UpdateChanges(patch, FALSE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		patchData->SetFlag(EPD_BEENDONE,TRUE);
		}

	if(holdNeeded) {
		theHold.Accept(GetString(IDS_DS_SELECT));
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		PatchSelChanged();
		}
	else {
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList,EPD_BEENDONE);
	}



static float AffectRegionFunct( float dist, float falloff, float pinch, float bubble) 
{
	float u = ((falloff - dist)/falloff);
	float u2 = u*u;
	float s = 1.0f-u;	

	return (3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2;
}

void EditPatchMod::ApplySoftSelectionToPatch( PatchMesh * pPatch )
{
	bool calcVertexWeights = false;

	if ( patchLevel[selLevel] != pPatch->VertexWeightSelectLevel() ) {
		pPatch->selLevel = patchLevel[selLevel];
		calcVertexWeights = true;	
	}

	if ( mFalloff != pPatch->Falloff() ) {
		pPatch->SetFalloff( mFalloff );
		calcVertexWeights = true;	
	}

	if ( mBubble != pPatch->Bubble() ) {
		pPatch->SetFalloff( mFalloff );
		calcVertexWeights = true;	
	}

	if ( mPinch != pPatch->Pinch() ) {
		pPatch->SetPinch( mPinch );
		calcVertexWeights = true;	
	}

	if ( mUseEdgeDists != pPatch->UseEdgeDists() ) {
		pPatch->SetUseEdgeDists( mUseEdgeDists );
		calcVertexWeights = true;	
	}

	if ( mEdgeDist != pPatch->EdgeDist() ) {
		pPatch->SetEdgeDist( mEdgeDist );
		calcVertexWeights = true;	
	}

	if ( mAffectBackface != pPatch->AffectBackface() ) {
		pPatch->SetAffectBackface( mAffectBackface );
		calcVertexWeights = true;	
	}

	if ( mUseSoftSelections != pPatch->UseSoftSelections() ) {
		pPatch->SetUseSoftSelections( mUseSoftSelections );
		// calcVertexWeights = true; /* not necessary since SetUseSoftSelections(..) will calc the vertex weights for us */	
	}

	if ( calcVertexWeights ) {
		pPatch->UpdateVertexDists();
		pPatch->UpdateEdgeDists();
		pPatch->UpdateVertexWeights();
	}
}

void EditPatchMod::SetSoftSelDlgEnables( HWND hSoftSel ) {
	
	if ( !hSoftSel && !hSoftSelPanel )  // nothing to set
		return;

	if ( !hSoftSel && hSoftSelPanel ) { // user omitted hSoftSel param, use hSoftSelPanel
		hSoftSel = hSoftSelPanel;
	}

	switch ( GetSubobjectLevel() ) {
	case EP_VERTEX:
	case EP_EDGE:
	case EP_PATCH:
	case EP_ELEMENT:
		{
			int useSoftSelections = UseSoftSelections();

			EnableWindow( GetDlgItem( hSoftSel, IDC_SOFT_SELECTION ), TRUE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_DIST ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_AFFECT_BACKFACING ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE ),    useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_SPIN ),    useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_LABEL ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_SPIN ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_LABEL ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_SPIN ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE ),  useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_LABEL ),  useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_SPIN ),  useSoftSelections );
			// CAL-05/06/03: shaded face toggle. (FID #1914)
			ICustButton *but = GetICustButton (GetDlgItem (hSoftSel, IDC_SHADED_FACE_TOGGLE));
			but->Enable (useSoftSelections);
			ReleaseICustButton (but);
		}
		break;

	default:
		{
			EnableWindow( GetDlgItem( hSoftSel, IDC_SOFT_SELECTION ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_DIST ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_AFFECT_BACKFACING ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE ),    FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_SPIN ),    FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_LABEL ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_SPIN ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_LABEL ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_SPIN ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE ),  FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_LABEL ),  FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_SPIN ),  FALSE );
			// CAL-05/06/03: shaded face toggle. (FID #1914)
			ICustButton *but = GetICustButton (GetDlgItem (hSoftSel, IDC_SHADED_FACE_TOGGLE));
			but->Enable (FALSE);
			ReleaseICustButton (but);
		}
		break;

	}
}

#if 0
void UpdateSoftSelections( PatchMesh & pmesh );

	if ( UseSoftSelections() ) {
		pmesh.
		UpdateVertexDists();
		UpdateEdgeDists();
		UpdateVertexWeights();
	}
#endif

void EditPatchMod::SetUseSoftSelections( int useSoftSelections )
{
	if ( mUseSoftSelections == useSoftSelections ) return;

	mUseSoftSelections = useSoftSelections;
	if ( useSoftSelections ) {
		UpdateVertexDists();
		UpdateEdgeDists( );
		UpdateVertexWeights();
	}
	else {
		InvalidateVertexWeights();
	}
}

int EditPatchMod::UseSoftSelections()
{
	return mUseSoftSelections;
}

void EditPatchMod::SetUseEdgeDists( int useEdgeDist )
{
	if ( useEdgeDist == mUseEdgeDists ) return;

	mUseEdgeDists = useEdgeDist;
	if ( useEdgeDist ) {
		UpdateVertexDists();
		UpdateEdgeDists( );
		UpdateVertexWeights();
	}
	else {
		UpdateVertexWeights();
	}
}

int EditPatchMod::UseEdgeDists()
{
	return mUseEdgeDists;
}

void EditPatchMod::InvalidateVertexWeights() {
	mVertexDists.SetCount( 0 );
	mVertexDists.Shrink();
	mVertexEdgeDists.SetCount( 0 );
	mVertexEdgeDists.Shrink();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts( mcList, nodes );

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData * pPatchData = (EditPatchData*)mcList[i]->localData;
		if ( !pPatchData ) 
			continue;	
		
		PatchMesh *pPatch = pPatchData->TempData(this)->GetPatch(ip->GetTime());
		if(!pPatch)
			continue;

		pPatch->InvalidateVertexWeights();
	}
	nodes.DisposeTemporary();
}


// CAL-05/06/03: toggle shaded faces display for soft selection. (FID #1914)
// NOTE: this class is copied from PolyEdOps.cpp
class ToggleShadedRestore : public RestoreObj {
	INode *mpNode;
	bool mOldShowVertCol, mOldShadedVertCol, mNewShowVertCol;
	int mOldVertexColorType;

public:
	ToggleShadedRestore (INode *pNode, bool newShow);
	void Restore(int isUndo);
	void Redo();
	int Size() { return sizeof (void *) + 3*sizeof(bool) + sizeof(int); }
	TSTR Description() { return TSTR(_T("ToggleShadedRestore")); }
};

ToggleShadedRestore::ToggleShadedRestore (INode *pNode, bool newShow) : mpNode(pNode), mNewShowVertCol(newShow) {
	mOldShowVertCol = mpNode->GetCVertMode() ? true : false;
	mOldShadedVertCol = mpNode->GetShadeCVerts() ? true : false;
	mOldVertexColorType = mpNode->GetVertexColorType ();
}

void ToggleShadedRestore::Restore (int isUndo) {
	mpNode->SetCVertMode (mOldShowVertCol);
	mpNode->SetShadeCVerts (mOldShadedVertCol);
	mpNode->SetVertexColorType (mOldVertexColorType);
	mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void ToggleShadedRestore::Redo () {
	mpNode->SetCVertMode (mNewShowVertCol);
	mpNode->SetShadeCVerts (true);
	mpNode->SetVertexColorType (nvct_soft_select);
	mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}


// CAL-05/06/03: toggle shaded faces display for soft selection. (FID #1914)
// NOTE: this function is copied from EditPolyObject::EpfnToggleShadedFaces().
void EditPatchMod::ToggleShadedFaces()
{
	if (!ip) return;

	theHold.Begin();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int oldShow, oldShaded, oldType;
		oldShow = nodes[i]->GetCVertMode ();
		oldShaded = nodes[i]->GetShadeCVerts ();
		oldType = nodes[i]->GetVertexColorType ();

		// If we have anything other than our perfect (true, true, nvct_soft_select) combo, set to that combo.
		// Otherwise, turn off shading.
		if (oldShow && oldShaded && (oldType == nvct_soft_select)) {
			if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[i], false));
			nodes[i]->SetCVertMode (false);
		} else {
			if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[i], true));
			nodes[i]->SetCVertMode (true);
			nodes[i]->SetShadeCVerts (true);
			nodes[i]->SetVertexColorType (nvct_soft_select);
		}
		nodes[i]->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	nodes.DisposeTemporary();	// Would this cause a problem when restore? Would nodes[i] become invalid when restore?
	theHold.Accept(GetString(IDS_TOGGLE_SHADED_FACES));

	// macroRecorder->FunctionCall(_T("$.EditPatchMod.ToggleShadedFaces"), 0, 0);
	// macroRecorder->EmitScript ();

	// KLUGE: above notify dependents call only seems to set a limited refresh region.  Result looks bad.
	// So we do this too:
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
}


Point3 EditPatchMod::VertexNormal( PatchMesh * pPatch, int vIndex ) 
{

	PatchVert * pVert = pPatch->getVertPtr( vIndex );

	int vecCount = pVert->vectors.Count();
	if ( vecCount < 2 ) { assert(0); return Point3(0.0, 0.0, 1.0); }

	int vecIndex = pVert->vectors[0];
	PatchVec * pVec0 = pPatch->getVecPtr( vecIndex );

	vecIndex = pVert->vectors[1];
	PatchVec * pVec1 = pPatch->getVecPtr( vecIndex );

	Point3 deltaVec = Normalize( pVec0->p - pVert->p )^Normalize( pVec1->p - pVert->p );
	return deltaVec;
}

void EditPatchMod::UpdateVertexWeights () {

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseSoftSelections ) {

		// discover total # of verts in all contexts
		//
		int totalVertexCount = 0;
		int totalVectorCount = 0;
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData * pPatchData = (EditPatchData*)mcList[i]->localData;
			if ( !pPatchData ) 
				continue;	
			
			PatchMesh * pPatch = pPatchData->TempData(this)->GetPatch(ip->GetTime());
			if(!pPatch)
				continue;

			pPatch->selLevel = patchLevel[selLevel];
			pPatch->SetFalloff( mFalloff );
			pPatch->SetBubble( mBubble );
			pPatch->SetPinch( mPinch );
			pPatch->SetUseEdgeDists( mUseEdgeDists );
			pPatch->SetEdgeDist( mEdgeDist );
			pPatch->SetAffectBackface( mAffectBackface );
			pPatch->SetUseSoftSelections( mUseSoftSelections );

			pPatch->UpdateVertexWeights();
		}

	}

}

void EditPatchMod::UpdateVertexDists( ) 
{
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	
	// discover total # of verts in all contexts
	//
	int totalVertexCount = 0;
	int totalVectorCount = 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditPatchData * pPatchData = (EditPatchData*)mcList[i]->localData;
		if ( !pPatchData ) 
			continue;	
		
		PatchMesh * pPatch = pPatchData->TempData(this)->GetPatch(ip->GetTime());
		if(!pPatch)
			continue;

		pPatch->selLevel = patchLevel[selLevel];
		pPatch->UpdateVertexDists();
	}

}

void EditPatchMod::UpdateEdgeDists( ) 
{

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseEdgeDists ) {
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPatchData * pPatchData = (EditPatchData*)mcList[i]->localData;
			if ( !pPatchData ) 
				continue;	
			
			PatchMesh * pPatch = pPatchData->TempData(this)->GetPatch(ip->GetTime());
			if(!pPatch)
				continue;

			pPatch->selLevel = patchLevel[selLevel];
			pPatch->UpdateEdgeDists();
		}
	}

}

#endif // NO_PATCHES

Point3 RotateXForm::proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt  ) 
{
	float ang;
	Point3 axis;

    AngAxisFromQ( mQuat, &ang, axis);
	ang *= wt;

	Quat quat = QFromAngAxis(ang, axis);
	
	quat.MakeMatrix(rot);
	trot = mat * rot;
	return (trot*p)*imat; 
}

Point3 ScaleXForm::proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt ) 
{

	// I think that Matrix3 could use GetScale() / SetScale() methods
	//
	Point3 r0 = scale.GetRow( 0 );
	Point3 r1 = scale.GetRow( 1 );
	Point3 r2 = scale.GetRow( 2 );

	float s0 = r0.x;
	float s1 = r1.y;
	float s2 = r2.z;

	float t0 = 1.0 - ((1.0 - s0) * wt );
	float t1 = 1.0 - ((1.0 - s1) * wt );
	float t2 = 1.0 - ((1.0 - s2) * wt );

	r0.x = t0;
	r1.y = t1;
	r2.z = t2;

	Matrix3 tmpMat = scale;

	tmpMat.SetRow( 0, r0 );
	tmpMat.SetRow( 1, r1 );
	tmpMat.SetRow( 2, r2 );

	tscale = mat*tmpMat;

	return (p*tscale)*imat;
}

RefResult EditPatchMod::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			DebugPrint("Ref %p changed!\n",hTarget);
			objectChanged = TRUE;
			break;
		}
	return REF_SUCCEED;
	}


//az -  042803  MultiMtl sub/mtl name support
INode* GetNode (EditPatchMod *ep){
	ModContextList mcList;
	INodeTab nodes;
	ep->ip->GetModContexts (mcList, nodes);
	INode* objnode = nodes.Count() == 1 ? nodes[0]->GetActualINode(): NULL;
	nodes.DisposeTemporary();
	return objnode;
}

void GetMtlIDList(Mtl *mtl, NumList& mtlIDList)
{
	if (mtl != NULL && mtl->IsMultiMtl()) {
		int subs = mtl->NumSubMtls();
		if (subs <= 0)
			subs = 1;
		for (int i=0; i<subs;i++){
			if(mtl->GetSubMtl(i))
				mtlIDList.Add(i, TRUE);  

		}
	}
}

void GetEPatchMtlIDList(EditPatchMod *ep, NumList& mtlIDList)
{
	ModContextList mcList;
	INodeTab nodes;
	if (ep) {
		ep->ip->GetModContexts (mcList, nodes);
		if (nodes.Count() == 1) {
			EditPatchData *patchData = (EditPatchData*)mcList[0]->localData;
			PatchMesh *patch = patchData->TempData(ep)->GetPatch(ep->ip->GetTime());
			int c_num = patch->getNumPatches();           
			for (int i=0; i<c_num; i++) {
				int mid = patch->getPatchMtlIndex(i); 
				if (mid != -1)
					mtlIDList.Add(mid, TRUE);
			}
			nodes.DisposeTemporary();
		}
	}
}

BOOL SetupMtlSubNameCombo (HWND hWnd, EditPatchMod *ep) {
	INode* singleNode;
	Mtl *nodeMtl;
	
	singleNode = GetNode(ep);
	if(singleNode)
		nodeMtl = singleNode->GetMtl();
	if(singleNode == NULL || nodeMtl == NULL || !nodeMtl->IsMultiMtl()) {    //no UI for multi, cloned nodes, and not MultiMtl 
		SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), false);
		return false;
	}
	NumList mtlIDList;
	NumList mtlIDMeshList;
	GetMtlIDList(nodeMtl, mtlIDList);
	GetEPatchMtlIDList(ep, mtlIDMeshList);
	MultiMtl *nodeMulti = (MultiMtl*) nodeMtl;
	EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), true);
	SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);

	for (int i=0; i<mtlIDList.Count(); i++){
		TSTR idname, buf;
		if(mtlIDMeshList.Find(mtlIDList[i]) != -1) {
			nodeMulti->GetSubMtlName(mtlIDList[i], idname); 
			if (idname.isNull())
				idname = GetString(IDS_MTL_NONAME);                                 //az: 042303  - FIGS
			buf.printf(_T("%s - ( %d )"), idname.data(), mtlIDList[i]+1);
			int ith = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_ADDSTRING, 0, (LPARAM)buf.data());
			SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_SETITEMDATA, ith, (LPARAM)mtlIDList[i]);
		}
	}
	return true;
}

void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin) {
	int cbcount, sval, cbval;
	sval = spin->GetIVal() - 1;          
	if (!spin->IsIndeterminate()){
		cbcount = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETCOUNT, 0, 0);
		if (cbcount > 0){
			for (int index=0; index<cbcount; index++){
				cbval = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
				if (sval == cbval) {
					SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)index, 0);
					return;
				}
			}
		}
	}
	SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)-1, 0);	
}


void ValidateUINameCombo (HWND hWnd, EditPatchMod *ep) {
	SetupMtlSubNameCombo (hWnd, ep);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	if (spin)
		UpdateNameCombo (hWnd, spin);
	ReleaseISpinner(spin);
}

