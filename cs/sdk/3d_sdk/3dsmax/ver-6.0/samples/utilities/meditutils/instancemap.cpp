/**********************************************************************
*<
FILE: instancemap.cpp

DESCRIPTION:	
-- Instance Duplicate Map
substitude duplicates scene material maps with instances

CREATED BY:		Alex Zadorozhny

HISTORY:		Created 6/17/03

*>	Copyright (c) 2003, All Rights Reserved.
**********************************************************************/


#include "meditutils.h"
#include <stdmat.h>
#include <hash_map>
#include <algorithm>

//Utility comparasion methods 
bool floatComp           (const float kf1, const float kf2);                                 // compare floats with cosnt threshlold
bool ParamBlockComp      (const IParamBlock* ppb1, const IParamBlock* ppb2);                 // compare paramblock parameters
bool UVGenComp           (const StdUVGen* pUVGen1, const StdUVGen* pUVGen2);                 // compare UVGen paramteres 
bool TexoutComp          (const TextureOutput* pTexout1, const TextureOutput* pTexout2);     // compare  Texture Output parameters
bool CurveComp           (const ICurve* pCurve1, const ICurve* pCurve2);                     // compare Curve parameters

//Instance Duplicate Maps
#define INSTANCEDUPLMAP_CLASS_ID	Class_ID(0x2a76b1c, 0xd605a5f)
class InstanceDuplMap : public UtilityObj {
public:
	class			FixUpSet;		// Utility class that finds duplicate maps
	class			FixUpDialog;	// Utility class for handling the dialog.

	HWND			hPanel;
	IUtil			*iu;
	Interface		*ip;

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);

	void DeleteThis() { }		

	InstanceDuplMap();
	~InstanceDuplMap();		

	// Find all duplicates maps
	// and fix them up. If prompt is true,
	// a dialog is displayed.
	bool FixUpAllMaps(BOOL prompt = true);        

	// Find all duplicates map
	void FindDuplicateMaps(Tab<Texmap*>& maps);

	// Find all copies of duplicate map
	void FindCopyMaps(Texmap* map, Tab<Texmap*>& maps);

	// Instance maps
	// If prompt is true, show a dialog.
	bool FixUpMaps(Tab<Texmap*>& maps, BOOL prompt = true); 
	

};


static InstanceDuplMap theInstanceDuplMap;

class InstanceDuplMapClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theInstanceDuplMap; }
	const TCHAR *	ClassName() { return GetString(IDS_INSTANCEDUPLMAP_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return INSTANCEDUPLMAP_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("InstanceDuplMap"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};
static InstanceDuplMapClassDesc InstanceDuplMapDesc;

ClassDesc2* GetInstanceDuplMapDesc() { return &InstanceDuplMapDesc; }




// Function Publishing 
class InstanceDuplMapFPInterface : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(InstanceDuplMapFPInterface)

	enum {
		kFindDuplMaps = 1,
		kFindCopyMaps = 2,
		kFixUpMaps = 3,
		kFixUpAllMaps = 4
	};

	BEGIN_FUNCTION_MAP

		VFN_1(kFindDuplMaps, theInstanceDuplMap.FindDuplicateMaps, TYPE_TEXMAP_TAB_BR)
		VFN_2(kFindCopyMaps, theInstanceDuplMap.FindCopyMaps, TYPE_TEXMAP, TYPE_TEXMAP_TAB_BR)
		FN_2(kFixUpMaps, TYPE_BOOL, theInstanceDuplMap.FixUpMaps, TYPE_TEXMAP_TAB_BR, TYPE_BOOL)
		FN_1(kFixUpAllMaps, TYPE_BOOL, theInstanceDuplMap.FixUpAllMaps, TYPE_BOOL)

	END_FUNCTION_MAP

	static InstanceDuplMapFPInterface mmInstanceDuplMapFPInterface;
};

#define INSTANCEDUPLMAP_FP_INTERFACE	Interface_ID(0x303741e1, 0x18d27e66)

InstanceDuplMapFPInterface InstanceDuplMapFPInterface::mmInstanceDuplMapFPInterface(
	INSTANCEDUPLMAP_FP_INTERFACE, _T("InstanceDuplMapInterface"), IDS_INSTANCEDUPLMAP_INTERFACE, &InstanceDuplMapDesc, FP_STATIC_METHODS,

	kFindDuplMaps, _T("findDuplicateMap"), 0, TYPE_VOID, 0, 1,
	_T("mapsFound"), 0, TYPE_TEXMAP_TAB_BR,
	f_inOut, FPP_OUT_PARAM,

	kFindCopyMaps, _T("findCopyMap"), 0, TYPE_VOID, 0, 2,
	_T("duplMap"), 0, TYPE_TEXMAP,
	_T("mapsFound"), 0, TYPE_TEXMAP_TAB_BR,
	f_inOut, FPP_OUT_PARAM,

	kFixUpMaps, _T("fixMap"), 0, TYPE_bool, 0, 2,
	_T("maps"), 0, TYPE_TEXMAP_TAB_BR,
	f_inOut, FPP_IN_PARAM,
	_T("prompt"), 0, TYPE_bool,
	f_keyArgDefault, true,

	kFixUpAllMaps, _T("fixMapAll"), 0, TYPE_BOOL, 0, 1,
	_T("prompt"), 0, TYPE_bool,                  
	f_keyArgDefault, true,

	end
	);



class InstanceDuplMapActions : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(InstanceDuplMapActions)

	enum {
		kFixUpAllMapsAction = 1,
	};

	BEGIN_FUNCTION_MAP
		FN_0(kFixUpAllMapsAction, TYPE_BOOL, theInstanceDuplMap.FixUpAllMaps)
	END_FUNCTION_MAP

	static InstanceDuplMapActions mmInstanceDuplMapActions;
};

#define INSTANCEDUPLMAP_ACTIONS		Interface_ID(0x26615cee, 0x26293586)

InstanceDuplMapActions InstanceDuplMapActions::mmInstanceDuplMapActions(
	INSTANCEDUPLMAP_ACTIONS, _T("InstanceDuplMapActions"), IDS_INSTANCEDUPLMAP_ACTIONS, &InstanceDuplMapDesc, FP_ACTIONS,
	kActionMaterialEditorContext,

	kFixUpAllMapsAction, _T("InstanceDuplMapAll"), IDS_INSTANCEDUPLMAP_ALL, 0,
	f_menuText,	IDS_INSTANCEDUPLMAP_MENU,
	end,

	end
	);




static BOOL CALLBACK InstanceDuplMapDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
case WM_INITDIALOG:
	theInstanceDuplMap.Init(hWnd);
	break;

case WM_DESTROY:
	theInstanceDuplMap.Destroy(hWnd);
	break;

case WM_COMMAND:
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	theInstanceDuplMap.FixUpAllMaps();
	break;

case WM_LBUTTONDOWN:
case WM_LBUTTONUP:
case WM_MOUSEMOVE:
	theInstanceDuplMap.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
	break;

default:
	return FALSE;
	}
	return TRUE;
}


InstanceDuplMap::InstanceDuplMap()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

InstanceDuplMap::~InstanceDuplMap()
{

}

void InstanceDuplMap::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL_INSTANCEDUPLMAP),
		InstanceDuplMapDlgProc,
		GetString(IDS_PARAMS),
		0);
}

void InstanceDuplMap::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void InstanceDuplMap::Init(HWND hWnd)
{
	hPanel=hWnd;
}

void InstanceDuplMap::Destroy(HWND hWnd)
{
	hPanel=NULL;
}



// This utility class is used to find all duplicates Bitmaps and make instances out of 
// one master. It doesn't have any UI.
class InstanceDuplMap::FixUpSet {
public:
	FixUpSet() : mIp(*GetCOREInterface()) {}

	// Find all duplicates bitmaps
	void FindDuplicateAll(Tab<BitmapTex*>& dupl_maps);
	// Find all copies of duplicate
	void FindCopyAll(BitmapTex* dupl_map, Tab<BitmapTex*>& copy_maps);

private:
	friend class InstanceDuplMap::FixUpDialog;

	// MapSet is the internal data structure we use
	// to hold the maps found.
	typedef std::hash_map<BitmapTex*, Mtl*> MapSet;


	// Find duplicates maps
	// The found maps are stored in mMaps.
	void findAll();					    // Find all scene maps      
	void findMaps(Mtl* mtl);		    // Find all scene maps mtl's material tree  
	void CollectDuplMaps(Mtl* mtl);		// Collect all duplicates in mtl's material tree
	
	// Duplicates maps 
	void outputDuplMaps(Tab<BitmapTex*>& maps);
	
	// Copies maps 
	void outputMaps(BitmapTex* map, Tab<BitmapTex*>& maps);

	// Make instance out of copies from master (maps[0])
	bool MakeInstance(Tab<BitmapTex*>& maps);                               

	//Replace scene map instances
	void ReplaceInstances(BitmapTex* map, BitmapTex* map_inst, Mtl* mtl);      

	// Instance all maps
	bool fixUp(); 

	// Add map to mMaps
	bool addMap(BitmapTex* map, Mtl* mtl);

	//compare BMTex parameters
	bool CompareMaps (BitmapTex* m1, BitmapTex* m2);   

 	Interface&		mIp;			                   // handy pointer to MAX interface
	MapSet		mMaps;		                           // hash map to hold all duplicates
    Tab<BitmapTex*> SceneMapContainer;                 // all scene maps
	
};


// This utility dialog is used to let the user see which Bitmaps 
// are to be instanced and exclude some that they want to remain unchanged
class InstanceDuplMap::FixUpDialog : public InstanceDuplMap::FixUpSet {
public:
	FixUpDialog() {}; 
	~FixUpDialog() {};
	
	// Instance duplicates maps. If prompt is true, the dialog is shown
	bool FixUpAll(bool prompt = true);
	bool FixUp(Tab<BitmapTex*>& maps, bool prompt = true);

private:
	enum errorflag {
		kallok = 1<<2,
		kanim = 1<<3,
		kpair = 1<<4
	};
	
	// Compare the names of two Bitmaps for sorting.
	// The dialog keeps a cache of type names so we don't have
	// to keep looking them up.
	class CompareMap {
	public:
		CompareMap(FixUpDialog* dlg) : mDlg(*dlg) {}

		bool operator()(
			BitmapTex* m1,
			BitmapTex* m2
			) const
		{
			return mDlg.lessThanMap(m1, m2);
		}

	private:
		FixUpDialog&	mDlg;
	};
	friend class CompareMap;

	
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
	
	INT_PTR proc(
		UINT		msg,
		WPARAM		w,
		LPARAM		l
		);

	// WM_COMMAND handler
	bool cmd(UINT ctlID, UINT notification, HWND ctl);

	// WM_INITDIALOG handler
	bool initDialog(HWND focus);

	// WM_WINDOWPOSCHANGING handler - resizes and moves control
	// when the dialog size is changed.
	bool resizeDialog(LPWINDOWPOS pos);
	void moveControl(int id, int dx, int dy);

	// Fill the list control with the duplicates maps name
	void fillDuplMapList();
	void appendMapToDuplList(MapSet::iterator p, int index);
	
	// Fill the list control with the copies of duplicate map 
	void fillCopyMapList(BitmapTex* map);
	void appendMapToList(MapSet::iterator p, int index);

	// Compare two map names 
	bool lessThanMap(
		BitmapTex* m1,
		BitmapTex* m2
		) const;


	HWND			mWnd;			// Dialog window handle
	HWND			mListCtrl;		// Copy List control window handle
	int				mBottom;		// Bottom of the list control, used by resizing
	UINT			mError;  	    // Error flag - used to turn text red
	int             mDuplIndex;     // Selection of ComboBox;
	WNDPROC			mWndProc;		// Sub classed window proc


};


bool InstanceDuplMap::FixUpAllMaps(BOOL prompt)
{

	// Create dialog and ask it to fix all maps
	FixUpDialog dialog;
	return dialog.FixUpAll(prompt != 0);
}

void InstanceDuplMap::FindDuplicateMaps(Tab<Texmap*>& dupl_texmaps)
{
	int i, count = 0;
	Tab<BitmapTex*> dupl_maps;
	FixUpSet maps;
	maps.FindDuplicateAll(dupl_maps);

	count = dupl_maps.Count();
	i = dupl_texmaps.Count();
	count += i;
	dupl_texmaps.SetCount(count);
	for (i=0; i<count; i++){
			dupl_texmaps[i] = (Texmap*) dupl_maps[i];
	}
}

void InstanceDuplMap::FindCopyMaps(Texmap* dupl_texmap, Tab<Texmap*>& copy_texmaps)
{
	int i, count = 0;
	Tab<BitmapTex*> copy_maps;
	FixUpSet maps;
	if (dupl_texmap) {
		if(dupl_texmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
			BitmapTex* bmt = (BitmapTex*) dupl_texmap;
			maps.FindCopyAll(bmt, copy_maps);

			count = copy_maps.Count();
			i = copy_texmaps.Count();
			count += i;
			copy_texmaps.SetCount(count);
			for (i=0; i<count; i++){
				copy_texmaps[i] = (Texmap*) copy_maps[i];
			}
		}
	}

}

bool InstanceDuplMap::FixUpMaps(Tab<Texmap*>& tex_maps, BOOL prompt)
{
	Tab<BitmapTex*> maps;
	int count = tex_maps.Count();
	for(int i=0; i<count; i++){
		BitmapTex* bmt = (BitmapTex*)tex_maps[i];
		maps.Append(1, &bmt);
	}
	// Create dialog and ask it to fix maps
	FixUpDialog dialog;
	return dialog.FixUp(maps, prompt != 0);
}



//--- InstanceDuplMap::FixUpSet ---------------------------------------------
void InstanceDuplMap::FixUpSet::FindDuplicateAll(Tab<BitmapTex*>& maps)
{
	// Find all maps and store in maps
	findAll();
	outputDuplMaps(maps);                    
} 

void InstanceDuplMap::FixUpSet::FindCopyAll(BitmapTex* map, Tab<BitmapTex*>& maps)
{
	// Find all maps and store in maps
	findAll();
	outputMaps(map, maps);                    
}


void InstanceDuplMap::FixUpSet::findAll()
{
	MtlBaseLib* mlib =  mIp.GetSceneMtls();
	int count = mlib->Count();

	// first round to collect all maps
	for (int i=0; i<count; i++) {
		Mtl* mtl = (Mtl*)(*mlib)[i];
		findMaps (mtl);
	}
    
	// second round to filter duplicates and insert them in hash table 
	for (int i=0; i<count; i++) {
		Mtl* mtl = (Mtl*)(*mlib)[i];
		CollectDuplMaps (mtl);
	}

}

void InstanceDuplMap::FixUpSet::findMaps(Mtl* mtl)
{
	if (mtl != NULL) {
		// check if mtl is actually in scene - work around for SceneMaterials not been updated
		INodeTab nodeTab;
		FindNodesProc dep(&nodeTab);
		mtl->EnumDependents(&dep);
		int node_count = nodeTab.Count();
		if (!node_count) return; 

		int map_count = mtl->NumSubTexmaps();
		for (int i=0; i < map_count; i++) {
			Texmap *tmap = mtl->GetSubTexmap(i);
			if (tmap) {
				if(tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					BitmapTex* bmt = (BitmapTex*) tmap;
					SceneMapContainer.Append (1, &bmt); 
				}
			}
		}
	
	    // Look through all sub-materials
		int mtl_count = mtl->NumSubMtls();
		for (int j=0; j < mtl_count; j++) {
			findMaps(mtl->GetSubMtl(j));
		}
	
	}
}

void InstanceDuplMap::FixUpSet::CollectDuplMaps(Mtl* mtl)
{
	if (mtl != NULL) {
		
		int map_count = mtl->NumSubTexmaps();
		int c_count = SceneMapContainer.Count();
		for (int i=0; i < map_count; i++) {
			Texmap *tmap = mtl->GetSubTexmap(i);
			if (tmap) {
				if(tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					BitmapTex* bmt = (BitmapTex*) tmap;
					for (int j=0; j<c_count; j++){
						if (bmt!= SceneMapContainer[j]){
							if (CompareMaps(bmt, SceneMapContainer[j]))
								addMap(bmt, mtl);
						}
					}
				}
			}
		}

		// Look through all sub-materials
		int mtl_count = mtl->NumSubMtls();
		for (int j=0; j < mtl_count; j++) {
			CollectDuplMaps(mtl->GetSubMtl(j));
		}

	}
}

void InstanceDuplMap::FixUpSet::ReplaceInstances(BitmapTex* map, BitmapTex* map_inst, Mtl* mtl)
{
	if (mtl != NULL) {
		
		int tex_display = map->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED);   //get active status
		
		// check if mtl is actually in scene - work around for SceneMaterials not been updated
		INodeTab nodeTab;
		FindNodesProc dep(&nodeTab);
		mtl->EnumDependents(&dep);
		int node_count = nodeTab.Count();
		if (!node_count) return; 

		int map_count = mtl->NumSubTexmaps();
		for (int i=0; i < map_count; i++) {
			Texmap *tmap = mtl->GetSubTexmap(i);
			if (tmap && tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
				BitmapTex* bmt = (BitmapTex*) tmap;
				if (map_inst == bmt) {
					mIp.FlushUndoBuffer();				
					bmt->DeactivateMapsInTree();
					mtl->SetSubTexmap(i, map);
					if(tex_display) mIp.ActivateTexture(map, mtl);
				}
			}
		}

		// Look through all sub-materials
		int mtl_count = mtl->NumSubMtls();
		for (int j=0; j < mtl_count; j++) {
			findMaps(mtl->GetSubMtl(j));
		}

	}
}



void InstanceDuplMap::FixUpSet::outputDuplMaps(Tab<BitmapTex*>& maps)
{
	int i, count = 0;

	MapSet::iterator p = mMaps.begin();
	for ( ; p != mMaps.end(); ++p) {
		if (p->first != NULL)
			++count;
	}

	i = maps.Count();
	count += i;
	maps.SetCount(count);
    
	if (count > 0) {
	
		// Output only single instance of each map path
		for (p = mMaps.begin(); p != mMaps.end(); ++p) {
			bool found = false;
			if (i!=0)	{
				for (int j=0; j<i; j++){
					if(_tcscmp(p->first->GetMapName(), maps[j]->GetMapName()) == 0
						&& CompareMaps(p->first, maps[j])){
						found = true;
						break;
					}
    			}
			}
			if (!found){
					maps[i] = p->first;
					++i;
				
	         }
		}

		maps.SetCount(i);
	}
	else
		maps.SetCount(0);

}


void InstanceDuplMap::FixUpSet::outputMaps(BitmapTex* map, Tab<BitmapTex*>& maps)
{
	int i, count = 0;

	MapSet::iterator p = mMaps.begin();
	for ( ; p != mMaps.end(); ++p) {
		if (p->first != NULL)
			++count;
	}

	i = maps.Count();
	count += i;
	maps.SetCount(count);

	for (p = mMaps.begin(); p != mMaps.end(); ++p) {
		if(_tcscmp(map->GetMapName(), p->first->GetMapName()) == 0
			&& CompareMaps(map, p->first)){
				maps[i] = p->first;
				++i;
        }
	}

	maps.SetCount(i);
}

bool InstanceDuplMap::FixUpSet::addMap(BitmapTex* map, Mtl* mtl)
{
	// Add the map and it's parent material
	mMaps.insert(MapSet::value_type(map, mtl)).first->second;
	return true;
}


bool InstanceDuplMap::FixUpSet::MakeInstance(Tab<BitmapTex*>& maps)
{
	MtlBaseLib* mlib =  mIp.GetSceneMtls();
	int mtl_count = mlib->Count();

	int count = maps.Count();
    BitmapTex* map = maps[0];          // make first in the list as master
	int tex_display = map->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED);   //get active status

	for (int i = 1; i<count; i++){
		// Loop through the maps. And fix them up
		MapSet::iterator p = mMaps.begin();
		while (p != mMaps.end()) {
			// We may removed the map from the list,
			// so increment the iterator now.
			MapSet::iterator m = p;
			++p;
		
			if(maps[i] == m->first){
				//get map and i-th position (channel)
				int pos = 0, txt_count = m->second->NumSubTexmaps();
				for(pos; txt_count; pos++){
					if(m->first == m->second->GetSubTexmap(pos))
						break;
				}
				mIp.FlushUndoBuffer();				
				m->first->DeactivateMapsInTree();
				m->second->SetSubTexmap(pos, map);
				if(tex_display) mIp.ActivateTexture(map, m->second);           //set active status
				
				// replace instances (if any) with master as well 
				for (int i=0; i<mtl_count; i++) {
					Mtl* mtl = (Mtl*)(*mlib)[i];
					ReplaceInstances(map, m->first, mtl);
				}
				
			    mMaps.erase(m);
		   			
			}
		}	
	}
    
	//check if only master left, delete if true
	int map_count = 0;                // number duplicates in hash table after erase
	MapSet::iterator master = mMaps.find(map);
	
	MapSet::iterator pp = mMaps.begin();
	for ( ; pp != mMaps.end(); ++pp) {
		if(_tcscmp(pp->first->GetMapName(), map->GetMapName()) == 0)
					++map_count; 
	}
	if (map_count == 1)
		mMaps.erase(master);                
	
	return true;
}

bool InstanceDuplMap::FixUpSet::fixUp()
{
	
	Tab<BitmapTex*> dupl_maps;
	outputDuplMaps(dupl_maps);
	int dupl_count = dupl_maps.Count();
	if (dupl_count < 1) return false;
	for (int i=0; i<dupl_count; i++){
		Tab<BitmapTex*> maps;
		outputMaps(dupl_maps[i], maps);
		MakeInstance( maps);
	}

	return mMaps.empty();	// Everything worked if the list is empty
	
}



//--- InstanceDuplMap::FixUpDialog ------------------------------------------

bool InstanceDuplMap::FixUpDialog::FixUpAll(bool prompt)
{
	// Find all maps
	findAll();
	 
	// Either show the dialog, which will fix the maps,
	// or fix them.
	if (prompt) {
		doModal();
		return mMaps.empty();
	}
	else {
		return fixUp();          
	}

}

bool InstanceDuplMap::FixUpDialog::FixUp(Tab<BitmapTex*>& maps, bool prompt)
{
	// Find all maps
	findAll();
	int count = maps.Count(); 
	//keep only submitted in the mMaps hash table
    MapSet::iterator p = mMaps.begin();
	while (p != mMaps.end()) {
		bool found = false;
		// We may removed the map from the list,
		// so increment the iterator now.
		MapSet::iterator m = p;
		++p;
		for (int i=0; i<count; i++){
			if(_tcsicmp(m->first->GetMapName(), maps[i]->GetMapName()) == 0) {
				found = true;
				break;
			}
		}
		if (!found)  mMaps.erase(m);
	}
	// Either show the dialog, which will fix the maps,
	// or fix them.
	if (prompt) {
		doModal();
		return mMaps.empty();
	}
	else {
		return fixUp();          
	}

}



void InstanceDuplMap::FixUpDialog::doModal()
{
	// Display the dialog box
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_FIXUP_INSTDUPLMAP),
		mIp.GetMAXHWnd(), dialogProc, reinterpret_cast<LPARAM>(this));
}

LRESULT CALLBACK InstanceDuplMap::FixUpDialog::windowProc(
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
		if (GetWindowLong(reinterpret_cast<HWND>(l), GWL_ID) == IDC_FIX_MESSAGE){
			if(p->mError == kanim || p->mError == kpair) {
				// Let the MAX code set the colors, then change the text to red.
				LRESULT r = CallWindowProc(p->mWndProc, hwnd, msg, w, l);
				SetTextColor(reinterpret_cast<HDC>(w), RGB(255, 0, 0));
				return r;
			}
		}
		break;
	}

	// Pass call to subclassed proc.
	return CallWindowProc(p->mWndProc, hwnd, msg, w, l);
}

// Dialog handler for the dialog
INT_PTR CALLBACK InstanceDuplMap::FixUpDialog::dialogProc(
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
		p->mError = kallok;         //initialize
		return p->initDialog(reinterpret_cast<HWND>(w));
	}

	FixUpDialog* p = reinterpret_cast<FixUpDialog*>(
		GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (p != NULL) {
		switch (msg) {
		case WM_COMMAND:
			return p->cmd(LOWORD(w), HIWORD(w), reinterpret_cast<HWND>(l));
		}
	return p->proc(msg, w, l);
	}

	return FALSE;
}


// Our dialog message processor
INT_PTR InstanceDuplMap::FixUpDialog::proc(
									  UINT		msg,
									  WPARAM		w,
									  LPARAM		l
									  )
{
	switch (msg) {
	case WM_WINDOWPOSCHANGING:
		// Resize when WM_WINDOWPOSCHANGING is received
		return resizeDialog(reinterpret_cast<LPWINDOWPOS>(l));
	case WM_CLOSE:
		return EndDialog(mWnd, 0);
	}

	return FALSE;
}

bool InstanceDuplMap::FixUpDialog::cmd(
									   UINT		ctlID,
									   UINT		notfication,
									   HWND		ctl									  
									   )
{
	int index, val;
	
	switch (ctlID) {
	case IDC_CLOSE:
		// Cancel the dialog
		EndDialog(mWnd, IDCANCEL);
	break;

    case IDC_DUPL_LIST:
		case CBN_SELENDOK: {
			index = SendMessage(GetDlgItem(mWnd,IDC_DUPL_LIST), CB_GETCURSEL, 0, 0);
			val = SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_GETITEMDATA, (WPARAM)index, 0);
			BitmapTex* bmt = (BitmapTex*)val;
			if (index != CB_ERR){
				bmt->IsAnimated()?mError = kanim:mError = kallok;  // check if duplicates animated
				fillCopyMapList(bmt);
				if (mError == kanim)
					SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_ANIM));
				else
					SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_INSTANCE));
			}
			
		break;                                                    
		}
		
	break;
	
	

	case IDC_PROC: {            
		Tab<BitmapTex*> maps;
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		int selItem = -1;
	
		while (( selItem = ListView_GetNextItem(mListCtrl, selItem, LVNI_SELECTED)) != -1)
		{
			lvi.iItem = selItem;
			ListView_GetItem(mListCtrl, &lvi);
			BitmapTex* map = reinterpret_cast<BitmapTex*>(lvi.lParam);
			maps.Append(1, &map);
		}
		if(maps.Count()>=2){       // must be a pair           
			mDuplIndex = SendMessage(GetDlgItem(mWnd,IDC_DUPL_LIST), CB_GETCURSEL, 0, 0);
			MakeInstance(maps);
			fillDuplMapList();
			index = SendMessage(GetDlgItem(mWnd,IDC_DUPL_LIST), CB_SETCURSEL, mDuplIndex, 0);
			if(index != CB_ERR) {
				val = SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_GETITEMDATA, (WPARAM)index, 0);
				BitmapTex* bmt = (BitmapTex*)val;
				fillCopyMapList(bmt);
						
			}
			else {
				ListView_DeleteAllItems(mListCtrl);
				SetWindowText(GetDlgItem(mWnd, IDC_STATIC_DUPL), _T(""));
				
			}
			mError = kallok;
			SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_FOUND));
		}
		else {
			mError = kpair;
			SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_PAIR));
		}
	break;
	}
	
	case IDC_PROC_ALL: {
		Tab<BitmapTex*> dupl_maps;
		int count;
		count = SendMessage(GetDlgItem(mWnd,IDC_DUPL_LIST), CB_GETCOUNT, 0, 0);
		for(index=0; index<count; index++){
			val = SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_GETITEMDATA, (WPARAM)index, 0);
			BitmapTex* bmt = (BitmapTex*)val;
			dupl_maps.Append(1, &bmt);
		}
		count = dupl_maps.Count();
		for (index=0; index<count; index++){
			Tab<BitmapTex*> maps;
			// Get the maps and sort them by name
			outputMaps(dupl_maps[index], maps);
			int count = maps.Count();
			if (count > 0) {
				BitmapTex** addr = maps.Addr(0);
				std::sort(addr, addr + count, CompareMap(this));
				MakeInstance(maps);
            }
			
		}

		EndDialog(mWnd, 0);
    break;	
	}
  
	}

	return false;
}

bool InstanceDuplMap::FixUpDialog::initDialog(HWND focus)
{
	// Subclass the dialog so we can show error messages in red
	mWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(mWnd, GWLP_WNDPROC));
	if (mWndProc != NULL) {
		SetWindowLongPtr(mWnd, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(windowProc));
	}
	
	SetWindowText(GetDlgItem(mWnd, IDC_STATIC_DUPL), _T(""));

	// Get the list control handle 
	mListCtrl = GetDlgItem(mWnd, IDC_MAP_LIST);
	
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 125;
	lvc.pszText = GetString(IDS_MAPNAME);
	lvc.iSubItem = 0;
	ListView_InsertColumn(mListCtrl, 0, &lvc);
	
	lvc.cx = 175;
	lvc.pszText =  GetString(IDS_PMTL);
	lvc.iSubItem = 1;
	ListView_InsertColumn(mListCtrl, 1, &lvc);
	ListView_SetExtendedListViewStyle(mListCtrl, LVS_EX_FULLROWSELECT);
	
	// Populate duplicates list control
    fillDuplMapList();
	mDuplIndex = 0;          // initialize selection
	int index, val;
	index = SendMessage(GetDlgItem(mWnd,IDC_DUPL_LIST), CB_SETCURSEL, mDuplIndex, 0);
	
	// Populate copies list control
	if(index != CB_ERR){
		val = SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_GETITEMDATA, (WPARAM)index, 0);
		BitmapTex* bmt = (BitmapTex*)val;
		bmt->IsAnimated()?mError = kanim:mError = kallok;  // check if duplicates animated
		fillCopyMapList(bmt);
		if (mError == kanim)
			SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_ANIM));
	}

	// Calculate the bottom of the list control, relative to
	// the bottom of the dialog.
	RECT ctrl, wdw;
	GetWindowRect(mWnd, &wdw);
	GetWindowRect(mListCtrl, &ctrl);
	mBottom = wdw.bottom - ctrl.bottom;
	
	
	if(mError != kanim){
		if (mMaps.size() != 0) 
			SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_MAP_FOUND));
		else
			SetDlgItemText(mWnd, IDC_FIX_MESSAGE, GetString(IDS_NO_MAP));
	}
	return true;
}


void InstanceDuplMap::FixUpDialog::fillDuplMapList()
{
	// Fill the combo box. Start by clearing any entries
	SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_RESETCONTENT, 0, 0);

	Tab<BitmapTex*> maps;
	
	// Get the duplicates maps and list just single name for each set
	outputDuplMaps(maps);
	int count = maps.Count();	

	if (count > 0) {                      
		BitmapTex** addr = maps.Addr(0);
		std::sort(addr, addr + count, CompareMap(this));
		
		for (int i = 0; i < count; ++i) {
			// Add the map to the list.
			appendMapToDuplList(mMaps.find(maps[i]), i);
		}
		
		// update selection
		if(mDuplIndex>count-1)
			mDuplIndex = count-1;
	}
	// get total count into Dialog 
	TSTR buf;
	//if (count != 0)
	buf.printf(_T("%d"), count);
	SetWindowText(GetDlgItem(mWnd, IDC_STATIC_TEXT), buf.data());
	
}


void InstanceDuplMap::FixUpDialog::fillCopyMapList(BitmapTex* map)
{
	// Fill the list box. Start by clearing any entries
	ListView_DeleteAllItems(mListCtrl);

	Tab<BitmapTex*> maps;

	// Get the maps and sort them by name 
	outputMaps(map, maps);
	int count = maps.Count();

	if (count > 0) {
		BitmapTex** addr = maps.Addr(0);
		std::sort(addr, addr + count, CompareMap(this));

		for (int i = 0; i < count; ++i) {
			// Add the map to the list.
			appendMapToList(mMaps.find(maps[i]), i);
		}
	}
	
	// get total count into Dialog 
	TSTR buf;
	buf.printf(_T("%d"), count);
	SetWindowText(GetDlgItem(mWnd, IDC_STATIC_DUPL), buf.data());
	
}

void InstanceDuplMap::FixUpDialog::appendMapToDuplList(MapSet::iterator p, int index)
{
	// Better be a valid map
	DbgAssert(p != mMaps.end() && p->first != NULL);
	if (p == mMaps.end() || p->first == NULL)
		return;

	// Get the name.
	TSTR entry_map = p->first->GetMapName();
	
	int ith = SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_ADDSTRING, 0, (LPARAM)entry_map.data());
	SendMessage(GetDlgItem(mWnd, IDC_DUPL_LIST), CB_SETITEMDATA, ith, (LPARAM)p->first);

}


void InstanceDuplMap::FixUpDialog::appendMapToList(MapSet::iterator p, int index)
{
	// Better be a valid map
	DbgAssert(p != mMaps.end() && p->first != NULL);
	if (p == mMaps.end() || p->first == NULL)
		return;
	
	// Get the name.
	TSTR entry_map = p->first->GetName();
	TSTR entry_mtl = p->second->GetFullName();
		
	LVITEM lvi;
	lvi.mask = LVIF_PARAM | LVIF_TEXT;
	lvi.iItem = index;

	// Map Name
	lvi.iSubItem = 0;
	lvi.lParam = reinterpret_cast<LPARAM>(p->first);
	lvi.pszText = entry_map.data();
	ListView_InsertItem(mListCtrl, &lvi);

   
	// Map Parent
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	lvi.pszText = entry_mtl.data();
	ListView_SetItem(mListCtrl, &lvi);

}

bool InstanceDuplMap::FixUpDialog::lessThanMap(
	BitmapTex* m1,
	BitmapTex* m2
	) const
{

	if (m1 == m2)
		return 0;
	if (m1 == NULL)
		return false;
	if (m2 == NULL)
		return true;

	// Compare the path name first
	int path = _tcsicmp(m1->GetMapName(), m2->GetMapName());
	if (path != 0)
		return _tcsicmp(m1->GetMapName(), m2->GetMapName()) < 0;
	else
		return _tcsicmp(m1->GetName(), m2->GetName()) < 0;
	

}

bool InstanceDuplMap::FixUpDialog::resizeDialog(LPWINDOWPOS pos)
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
		RECT client;
		GetWindowRect(mListCtrl, &client);
		SetWindowPos(mListCtrl, NULL, 0, 0,
			client.right - client.left + dx, 
			wdw.top + pos->cy - mBottom - client.top,
			SWP_NOMOVE | SWP_NOZORDER);

		// Now the size of the static message box and combo box. The height is
		// fixed, but the width can change.
		HWND msg = GetDlgItem(mWnd, IDC_FIX_MESSAGE);
		GetWindowRect(msg, &client);
		SetWindowPos(msg, NULL, 0, 0,
			client.right - client.left + dx,
			client.bottom - client.top,
			SWP_NOMOVE | SWP_NOZORDER);
				
		HWND combo = GetDlgItem(mWnd, IDC_DUPL_LIST);
		GetWindowRect(combo, &client);
		SetWindowPos(combo, NULL, 0, 0,
			client.right - client.left + dx, 
			client.bottom - client.top,
			SWP_NOMOVE | SWP_NOZORDER);

		// Move the buttons 
		moveControl(IDC_CLOSE, dx, dy);
		moveControl(IDC_PROC, dx, dy);
		moveControl(IDC_PROC_ALL, dx, dy);
		
		InvalidateRect(mWnd, NULL, true);	// Invalidate the control
		
	}
	

	return true;
}

void InstanceDuplMap::FixUpDialog::moveControl(int id, int dx, int dy)
{
	// Move the control by dx and dy.
	HWND hCtl = GetDlgItem(mWnd, id);
	RECT rect;

	GetWindowRect(hCtl, &rect);
	ScreenToClient(mWnd, reinterpret_cast<LPPOINT>(&rect));

	SetWindowPos(hCtl, NULL, rect.left + dx, rect.top + dy, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER);
}



bool InstanceDuplMap::FixUpSet::CompareMaps(BitmapTex* m1, BitmapTex* m2)
{
	
	// compare paramblock2 portion
	TimeValue t = mIp.GetTime();
	int pb1_count = m1->NumParamBlocks();
	for (int j=0; j<pb1_count; j++){
		IParamBlock2* pb1 = m1->GetParamBlock(j);
		IParamBlock2* pb2 = m2->GetParamBlock(j);
	
		int param_count = pb1->NumParams();    
	
		//check for map name first - if not the same no reason to check further
		for (int i=0; i<param_count; i++){
			ParamID pID = pb1->IndextoID(i);
			if(pb1->GetParameterType(pID) == TYPE_STRING){
				TCHAR* s1_name;
				TCHAR* s2_name;	
				pb1->GetValue(pID, t, s1_name, FOREVER);
				pb2->GetValue(pID, t, s2_name, FOREVER);
				if(_tcscmp(s1_name, s2_name))
					return false;
				// check the animation
				if (m1->IsAnimated() != m2->IsAnimated())
					return false;
			}
	
		}

		//paramblock2 cases
		for (int k=0; k<param_count; k++){
			ParamID pID = pb1->IndextoID(k);
			switch (pb1->GetParameterType(pID))
			{
			case TYPE_FLOAT:
				{
					float f1, f2;
					pb1->GetValue(pID, t, f1, FOREVER);
					pb2->GetValue(pID, t, f2, FOREVER);
					if(!floatComp(f1, f2)) return false;
					break;

				}
			case TYPE_INT:
				{
					int	i1, i2;
					pb1->GetValue(pID, t, i1, FOREVER);
					pb2->GetValue(pID, t, i2, FOREVER);
					if(i1 != i2)
						return false;
					break;
				}

			case TYPE_BOOL:
				{
					BOOL b1, b2;
					pb1->GetValue(pID, t, b1, FOREVER);
					pb2->GetValue(pID, t, b2, FOREVER);
					if(b1 != b2)
						return false;
					break;
				}
			case TYPE_TIMEVALUE:
				{
					TimeValue tv1, tv2;
					pb1->GetValue(pID, t, tv1, FOREVER);
					pb2->GetValue(pID, t, tv2, FOREVER);
					if(tv1 != tv2)
						return false;
					break;
				}
			case TYPE_REFTARG:
				{
					ReferenceTarget *targ1;
					ReferenceTarget *targ2;
					targ1= pb1->GetReferenceTarget(pID, t);
					targ2= pb2->GetReferenceTarget(pID, t);

					// StdUVGen parameters 
					if (targ1->SuperClassID()==UVGEN_CLASS_ID &&
						targ1->ClassID()==Class_ID(STDUV_CLASS_ID,0)){
						StdUVGen* uvgen1 = m1->GetUVGen();
						StdUVGen* uvgen2 = m2->GetUVGen();
						if(!UVGenComp(uvgen1, uvgen2)) return false;
					}   
					
					// Texture Output parameters 
					if (targ1->SuperClassID()==TEXOUTPUT_CLASS_ID &&
						targ1->ClassID()==Class_ID(STDTEXOUT_CLASS_ID,0)){
						TextureOutput* texout1 = m1->GetTexout();
						TextureOutput* texout2 = m2->GetTexout();
						if(!TexoutComp(texout1, texout2)) return false;
					}   	
						
					break;
				}
			}
		}

	}

	return true;
}


// compare floats with threshold factor
bool floatComp(const float kf1, const float kf2){
	float f1 = kf1;
	float f2 = kf2;
	float eps = 1E-6f;   //close enough, same as in Point2
	bool res = f1 == f2 || (2.0f*fabs(f1-f2)/(fabs(f1)+fabs(f2))) < eps;
	return res;
}                            

// compare paramblock parameters
bool ParamBlockComp(const IParamBlock* ppb1, const IParamBlock* ppb2){
	TimeValue t = GetCOREInterface()->GetTime();
	IParamBlock* pb_1 = (IParamBlock*) ppb1;
	IParamBlock* pb_2 = (IParamBlock*) ppb2; 
	
	int count = pb_1->NumParams();                           
	for (int i=0; i<count; i++){
		switch (pb_1->GetParameterType(i))
		{
		case TYPE_FLOAT:
			{
				float f1, f2;
				pb_1->GetValue(i, t, f1, FOREVER);
				pb_2->GetValue(i, t, f2, FOREVER);
				if (!floatComp(f1, f2)) return false;
				break;
			}
		case TYPE_BOOL:
		case TYPE_INT:
			{
				int	i1, i2;
				pb_1->GetValue(i, t, i1, FOREVER);
				pb_2->GetValue(i, t, i2, FOREVER);
				if(i1 != i2) return false;
				break;
			}
		}
	}
	return true;
}


// compare Standard UVGen rollout parameters
bool UVGenComp (const StdUVGen* pUVGen1, const StdUVGen* pUVGen2) {
		StdUVGen* uvgen1 = (StdUVGen*)pUVGen1;
		StdUVGen* uvgen2 = (StdUVGen*)pUVGen2;
		
		// compare paramblock portion
		IParamBlock* pb_1 = (IParamBlock*)uvgen1->SubAnim(0);    // reference to paramblock     
	    IParamBlock* pb_2 = (IParamBlock*)uvgen2->SubAnim(0);
			
		if(!ParamBlockComp(pb_1, pb_2)) return false;
	    
		// compare portion not covered by paramblock
	    if (uvgen1->GetSlotType() != uvgen2->GetSlotType()) return false;
	   
	    //handle mapping texture
	    int mapping1 = uvgen1->GetSlotType() == MAPSLOT_TEXTURE? 
		   uvgen1->GetUVWSource():uvgen1->GetCoordMapping(0);
	    int mapping2 = uvgen2->GetSlotType() == MAPSLOT_TEXTURE? 
		   uvgen2->GetUVWSource():uvgen2->GetCoordMapping(0);
	    if (mapping1 != mapping2) return false;
	   
	    if (uvgen1->GetMapChannel() != uvgen2->GetMapChannel()) return false;
	    if (uvgen1->GetAxis() != uvgen2->GetAxis()) return false;
	    if (uvgen1->GetFlag(U_MIRROR) != uvgen2->GetFlag(U_MIRROR)) return false;
	    if (uvgen1->GetFlag(V_MIRROR) != uvgen2->GetFlag(V_MIRROR)) return false;
	    if (uvgen1->GetFlag(U_WRAP) != uvgen2->GetFlag(U_WRAP)) return false;
	    if (uvgen1->GetFlag(V_WRAP) != uvgen2->GetFlag(V_WRAP)) return false;
	    if (uvgen1->GetHideMapBackFlag() != uvgen2->GetHideMapBackFlag()) return false;
	    if (uvgen1->GetFlag(UV_NOISE) != uvgen2->GetFlag(UV_NOISE)) return false;	
	    if (uvgen1->GetFlag(UV_NOISE_ANI) != uvgen2->GetFlag(UV_NOISE_ANI)) return false;

	return true;
}

// compare Curve parameters
bool CurveComp  (const ICurve* pCurve1, const ICurve* pCurve2){
	ICurve* cv_1 = (ICurve*)pCurve1;
	ICurve* cv_2 = (ICurve*)pCurve2;

	int numpts1 = cv_1->GetNumPts();
	int numpts2 = cv_2->GetNumPts();
	if (numpts1 != numpts2) return false;

	// compare points parameters
	for (int i=0; i<numpts1; i++){
		CurvePoint cp_1 = cv_1->GetPoint(GetCOREInterface()->GetTime(), i);
		CurvePoint cp_2 = cv_2->GetPoint(GetCOREInterface()->GetTime(), i);
		
		if(!cp_1.p.Equals(cp_2.p)) return false;
		if(!cp_1.in.Equals(cp_2.in)) return false;
		if(!cp_1.out.Equals(cp_2.out)) return false;
		if(cp_1.flags != cp_2.flags) return false;
	}

	return true;
}                     


// compare  Texture Output parameters
bool TexoutComp (const TextureOutput* pTexout1, const TextureOutput* pTexout2){
	TimeValue t = GetCOREInterface()->GetTime();
	
	//down cast to StdTexoutGen - the actual Output generator
	TextureOutput* texoutput1 = (TextureOutput*) pTexout1;
	TextureOutput* texoutput2 = (TextureOutput*) pTexout2;

    StdTexoutGen* texout1 = (StdTexoutGen*) texoutput1;
	StdTexoutGen* texout2 = (StdTexoutGen*) texoutput2;

	// compare paramblock portion
	IParamBlock* pb_1;
	IParamBlock* pb_2;
	         
	pb_1 = (IParamBlock*)texout1->SubAnim(0);   // reference to paramblock  
	pb_2 = (IParamBlock*)texout2->SubAnim(0);
	
	if(!ParamBlockComp(pb_1, pb_2)) return false;
	
	//compare portion not covered by paramblock
	if (texout1->GetInvert() != texout2->GetInvert()) return false;
	if (texout1->GetClamp() != texout2->GetClamp()) return false;
	if (texout1->GetAlphaFromRGB() != texout2->GetAlphaFromRGB()) return false;
	if (texout1->GetFlag(TEXOUT_COLOR_MAP) != texout2->GetFlag(TEXOUT_COLOR_MAP)) return false;
	
	// handle map ouptut 
	if(texout1->GetFlag(TEXOUT_COLOR_MAP) == 1){                // if map on, continue verification
    		BOOL map1 =  texout1->GetFlag(TEXOUT_COLOR_MAP_RGB);
			BOOL map2 =  texout2->GetFlag(TEXOUT_COLOR_MAP_RGB);
	    	if (map1 != map2) return false;
			
			if (map1 == 1) {           // RGB type
				ICurveCtl* ccRGB_1 = (ICurveCtl*)texout1->SubAnim(1); //reference to RGB curve control
				ICurveCtl* ccRGB_2 = (ICurveCtl*)texout2->SubAnim(1);
				
				for (int i=0; i<ccRGB_1->GetNumCurves(); i++) {                         
					ICurve* cRGB_1 = ccRGB_1->GetControlCurve(i);
					ICurve* cRGB_2 = ccRGB_2->GetControlCurve(i);
					if(!CurveComp(cRGB_1, cRGB_2)) return false;
				}
			}
			else{                      // Mono type
				ICurveCtl* ccMono_1 = (ICurveCtl*)texout1->SubAnim(2); //reference to Mono curve control
				ICurveCtl* ccMono_2 = (ICurveCtl*)texout2->SubAnim(2);

				ICurve* cMono_1 = ccMono_1->GetControlCurve(0);   //1 curve
				ICurve* cMono_2 = ccMono_2->GetControlCurve(0);
				if(!CurveComp(cMono_1, cMono_2)) return false;
			}
			
	}
	return true;
}
