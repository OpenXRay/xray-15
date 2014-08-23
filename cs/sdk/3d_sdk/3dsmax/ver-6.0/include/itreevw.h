/**********************************************************************
 *<
	FILE: itreevw.h

	DESCRIPTION: Tree View Interface

	CREATED BY:	Rolf Berteig

	HISTORY: Created 17 April 1995
			 Moved into public SDK, JBW 5.25.00
			 Extended by Adam Felt for R5

 *>	Copyright (c) 1994-2002, All Rights Reserved.
 **********************************************************************/


#ifndef __ITREEVW__
#define __ITREEVW__

#include "iFnPub.h"

#define WM_TV_SELCHANGED			WM_USER + 0x03001
#define WM_TV_MEDIT_TV_DESTROYED	WM_USER + 0x03b49

// Sent to a track view window to force it to do a horizontal zoom extents
#define WM_TV_DOHZOOMEXTENTS		WM_USER + 0xb9a1

// Style flag options for Interface::CreateTreeViewChild
#define TVSTYLE_MAXIMIZEBUT			(1<<0) // Has a maximize button for TV in a viewport.
#define TVSTYLE_INVIEWPORT			(1<<1)
//Setting the TVSTYLE_NAMEABLE flag will give the user maxscript access to your treeview through the trackviews.current interface.   
//Set this flag if you want the user to be able to modify your trackview settings.
#define TVSTYLE_NAMEABLE			(1<<2)
#define TVSTYLE_INMOTIONPAN			(1<<3)
#define TVSTYLE_SHOW_NONANIMATABLE	(1<<4)
// this flag specifies the treeview child window contains a menu bar and toolbars. (R5 and later only)
// if this style has been set you can use ITreeViewUI::GetHWnd() to get the handle to the window housing the CUI elements.
// this automatically sets the TVSTYLE_NAMEABLE flag since UI macroScripts may depend in the window being scriptable
#define TVSTYLE_SHOW_CUI			(1<<5)

// Docking options for ITrackViewArray::OpenTrackViewWindow
#define TV_FLOAT		0	// float window.  can't dock on top (the default)
#define TV_DOCK_TOP		1	// dock on top
#define TV_DOCK_BOTTOM	2	// dock on bottom.  can't dock on top
#define TV_CAN_DOCK_TOP	3	// floating but able to dock on top

// Major modes
#define MODE_EDITKEYS		0
#define MODE_EDITTIME		1
#define MODE_EDITRANGES		2
#define MODE_POSRANGES		3
#define MODE_EDITFCURVE		4

// Operations on keys can be performed on one of the following set types
#define EFFECT_ALL_SEL_KEYS				0
#define EFFECT_SEL_KEYS_IN_SEL_TRACKS	1
#define EFFECT_ALL_KEYS_IN_SEL_TRACKS	2
#define EFFECT_ALL_KEYS					3


class TrackViewFilter;
class TrackViewPick;

typedef Animatable* AnimatablePtr;

//This is an interface into many of the UI layout functions of trackview.
//You can get an instance of this class by calling ITreeView::GetInterface(TREEVIEW_UI_INTERFACE)
//***********************************************************************************************
#define TREEVIEW_UI_INTERFACE Interface_ID(0x1bcd78ef, 0x21990819)
class ITreeViewUI : public FPMixinInterface
{
public:
	virtual HWND GetHWnd()=0;

	virtual TSTR GetMenuBar()=0;
	virtual void SetMenuBar(TSTR name)=0;

	virtual TSTR GetControllerQuadMenu()=0;
	virtual void SetControllerQuadMenu(TSTR name)=0;
	virtual TSTR GetKeyQuadMenu()=0;
	virtual void SetKeyQuadMenu(TSTR name)=0;
	
	virtual int ToolbarCount()=0;
	virtual void AddToolbar()=0;
	virtual void DeleteToolbar()=0;
	virtual bool AddToolbar(TCHAR *name)=0;
	virtual bool DeleteToolbar(int index)=0;
	virtual bool DeleteToolbar(TCHAR *name)=0;
	virtual TCHAR* GetToolbarName(int index)=0;
	virtual void ShowToolbar(TCHAR *name)=0;
	virtual void HideToolbar(TCHAR *name)=0;
	virtual bool IsToolbarVisible(TCHAR *name)=0;
	virtual void ShowAllToolbars()=0;
	virtual void HideAllToolbars()=0;

	virtual void ShowMenuBar(bool visible)=0;
	virtual bool IsMenuBarVisible()=0;

	virtual void ShowScrollBars(bool visible)=0;
	virtual bool IsScrollBarsVisible()=0;

	virtual void ShowTrackWindow(bool visible)=0;
	virtual bool IsTrackWindowVisible()=0;

	virtual void ShowKeyWindow(bool visible)=0;
	virtual bool IsKeyWindowVisible()=0;

	virtual void ShowTimeRuler(bool visible)=0;
	virtual bool IsTimeRulerVisible()=0;

	virtual void ShowKeyPropertyIcon(bool visible)=0;
	virtual bool IsKeyPropertyIconVisible()=0;

	virtual void ShowIconsByClass(bool byClass)=0;
	virtual bool ShowIconsByClass()=0;

	virtual void SaveUILayout()=0;
	virtual void SaveUILayout(TCHAR* name)=0;
	virtual void LoadUILayout(TCHAR* name)=0;

	virtual int LayoutCount()=0;
	virtual TSTR GetLayoutName(int index)=0;
	virtual TSTR GetLayoutName()=0; //the current layout

	enum { tv_getHWnd, tv_getMenuBar, tv_setMenuBar, tv_getControllerQuadMenu, tv_setControllerQuadMenu,
			tv_getKeyQuadMenu, tv_setKeyQuadMenu,
			tv_getMenuBarVisible, tv_setMenuBarVisible, tv_getScrollBarsVisible, tv_setScrollBarsVisible,
			tv_getTrackWindowVisible, tv_setTrackWindowVisible, tv_getKeyWindowVisible, tv_setKeyWindowVisible,
			tv_getTimeRulerVisible, tv_setTimeRulerVisible, tv_showAllToolbars, tv_hideAllToolbars,
			tv_showToolbar, tv_hideToolbar, tv_getToolbarName, tv_deleteToolbar, tv_addToolbar, tv_toolbarCount,
			tv_saveUILayout, tv_saveCurrentUILayout, tv_loadUILayout, tv_layoutCount, tv_getLayoutName, tv_layoutName,
			tv_isToolbarVisible, tv_getKeyPropertyVisible, tv_setKeyPropertyVisible,
			tv_getIconsByClass, tv_setIconsByClass,
		};
	
	BEGIN_FUNCTION_MAP
		RO_PROP_FN(tv_getHWnd, GetHWnd, TYPE_HWND);
		RO_PROP_FN(tv_layoutName, GetLayoutName, TYPE_TSTR_BV);
		PROP_FNS(tv_getMenuBar, GetMenuBar, tv_setMenuBar, SetMenuBar, TYPE_TSTR_BV);
		PROP_FNS(tv_getControllerQuadMenu, GetControllerQuadMenu, tv_setControllerQuadMenu, SetControllerQuadMenu, TYPE_TSTR_BV);
		PROP_FNS(tv_getKeyQuadMenu, GetKeyQuadMenu, tv_setKeyQuadMenu, SetKeyQuadMenu, TYPE_TSTR_BV);
		PROP_FNS(tv_getMenuBarVisible, IsMenuBarVisible, tv_setMenuBarVisible, ShowMenuBar, TYPE_bool);
		PROP_FNS(tv_getScrollBarsVisible, IsScrollBarsVisible, tv_setScrollBarsVisible, ShowScrollBars, TYPE_bool);
		PROP_FNS(tv_getTrackWindowVisible, IsTrackWindowVisible, tv_setTrackWindowVisible, ShowTrackWindow, TYPE_bool);
		PROP_FNS(tv_getKeyWindowVisible, IsKeyWindowVisible, tv_setKeyWindowVisible, ShowKeyWindow, TYPE_bool);
		PROP_FNS(tv_getTimeRulerVisible, IsTimeRulerVisible, tv_setTimeRulerVisible, ShowTimeRuler, TYPE_bool);
		PROP_FNS(tv_getKeyPropertyVisible, IsKeyPropertyIconVisible, tv_setKeyPropertyVisible, ShowKeyPropertyIcon, TYPE_bool);
		PROP_FNS(tv_getIconsByClass, ShowIconsByClass, tv_setIconsByClass, ShowIconsByClass, TYPE_bool);
		VFN_0(tv_showAllToolbars, ShowAllToolbars);
		VFN_0(tv_hideAllToolbars, HideAllToolbars);
		VFN_1(tv_showToolbar, ShowToolbar, TYPE_STRING);
		VFN_1(tv_hideToolbar, HideToolbar, TYPE_STRING);
		FN_1(tv_getToolbarName, TYPE_STRING, GetToolbarName, TYPE_INDEX);
		FN_1(tv_deleteToolbar, TYPE_bool, fp_DeleteToolbar, TYPE_FPVALUE);
		FN_1(tv_addToolbar, TYPE_bool, AddToolbar, TYPE_STRING);
		FN_0(tv_toolbarCount, TYPE_INT, ToolbarCount);
		FN_1(tv_isToolbarVisible, TYPE_bool, IsToolbarVisible, TYPE_STRING);
		VFN_0(tv_saveCurrentUILayout, SaveUILayout);
		VFN_1(tv_saveUILayout, SaveUILayout, TYPE_STRING);
		VFN_1(tv_loadUILayout, LoadUILayout, TYPE_STRING);
		FN_0(tv_layoutCount, TYPE_INT, LayoutCount);
		FN_1(tv_getLayoutName, TYPE_TSTR_BV, GetLayoutName, TYPE_INDEX);
	END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc() { return GetDescByID(TREEVIEW_UI_INTERFACE); }
		Interface_ID	GetID() { return TREEVIEW_UI_INTERFACE; }

private:
	virtual bool fp_DeleteToolbar(FPValue* val)=0;

};

#define TREEVIEW_OPS_INTERFACE Interface_ID(0x60fb7eef, 0x1f6d6dd3)
//These are the operations you can do on any open trackview
//Added by AF (09/12/00)
//*********************************************************
class ITreeViewOps : public FPMixinInterface {
	public:
		virtual ~ITreeViewOps() {}

		virtual int GetNumTracks()=0;
		virtual int NumSelTracks()=0;
 		virtual void GetSelTrack(int i,AnimatablePtr &anim,AnimatablePtr &client,int &subNum)=0;
		virtual ReferenceTarget* GetAnim(int index)=0;
		virtual ReferenceTarget* GetClient(int index)=0;

		virtual BOOL CanAssignController()=0;
		virtual void DoAssignController(BOOL clearMot=TRUE)=0;
		virtual void ShowControllerType(BOOL show)=0;	

		virtual TCHAR *GetTVName()=0;
		// added for scripter access, JBW - 11/11/98
		virtual void SetTVName(TCHAR *)=0;
		virtual void CloseTreeView()=0;

		virtual void SetFilter(DWORD mask,int which, BOOL redraw=TRUE)=0;
		virtual void ClearFilter(DWORD mask,int which, BOOL redraw=TRUE)=0; 
		virtual DWORD TestFilter(DWORD mask,int which)=0; 
		
		// added for param wiring, JBW - 5.26.00
		virtual void ZoomOn(Animatable* owner, int subnum)=0;
		virtual void ZoomSelected()=0;
		virtual void ExpandTracks()=0;

		//added for completeness by AF (09/12/00) 
		virtual int GetIndex(Animatable *anim)=0;
		virtual void SelectTrackByIndex(int index, BOOL clearSelection=TRUE)=0;
		virtual void SelectTrack(Animatable* anim, BOOL clearSelection=TRUE)=0;
		virtual BOOL AssignControllerToSelected(Animatable* ctrl)=0;

		//added by AF (09/25/00) for MAXScript exposure
		virtual void SetEditMode(int mode)=0;
		virtual int GetEditMode()=0;

		//added by AF (09/25/00) for more MAXScript exposure
		//These differ from "active" because the trackview 
		//doesn't have to be selected for it to be the currently used trackview
		virtual BOOL IsCurrent()=0;
		virtual void SetCurrent()=0;

		enum {  tv_getName, tv_setName, tv_close, tv_numSelTracks, tv_getNumTracks, tv_getSelTrack, 
				tv_canAssignController, tv_doAssignController, tv_assignController, tv_showControllerTypes, 
				tv_expandTracks, tv_zoomSelected, tv_zoomOnTrack,
				tv_getAnim, tv_getClient, tv_getSelAnim, tv_getSelClient, tv_getSelAnimSubNum, 
				tv_getIndex, tv_selectTrackByIndex, tv_selectTrack,
				tv_setFilter, tv_clearFilter, 
				tv_setEditMode, tv_getEditMode, tv_setEditModeProp, tv_getEditModeProp,
				tv_setCurrent, tv_getCurrent, tv_getUIInterface, tv_getModifySubTree, tv_setModifySubTree, 
				tv_getModifyChildren, tv_setModifyChildren,
				tv_launchUtilityDialog, tv_launchUtility, tv_getUtilityCount, tv_getUtilityName, tv_closeUtility,
				tv_getInteractiveUpdate, tv_setInteractiveUpdate, tv_getSyncTime, tv_setSyncTime,
				tv_setTangentType, tv_setInTangentType, tv_setOutTangentType,
				tv_getFreezeSelKeys, tv_setFreezeSelKeys, 
				tv_getFreezeNonSelCurves, tv_setFreezeNonSelCurves, tv_getShowNonSelCurves, tv_setShowNonSelCurves,
				tv_getShowTangents, tv_setShowTangents, tv_getShowFrozenKeys, tv_setShowFrozenKeys,
				tv_getEffectSelectedObjectsOnly, tv_setEffectSelectedObjectsOnly, 
				tv_getAutoExpandChildren, tv_setAutoExpandChildren, tv_getAutoExpandTransforms, tv_setAutoExpandTransforms,
				tv_getAutoExpandObjects, tv_setAutoExpandObjects, tv_getAutoExpandModifiers, tv_setAutoExpandModifiers, 
				tv_getAutoExpandMaterials, tv_setAutoExpandMaterials, tv_getAutoExpandXYZ, tv_setAutoExpandXYZ,
				tv_getAutoSelectAnimated, tv_setAutoSelectAnimated, tv_getAutoSelectPosition, tv_setAutoSelectPosition,
				tv_getAutoSelectRotation, tv_setAutoSelectRotation, tv_getAutoSelectScale, tv_setAutoSelectScale,
				tv_getAutoSelectXYZ, tv_setAutoSelectXYZ, tv_getManualNavigation, tv_setManualNavigation,
				tv_getAutoZoomToRoot, tv_setAutoZoomToRoot, tv_getAutoZoomToSelected, tv_setAutoZoomToSelected,
				tv_getAutoZoomToEdited, tv_setAutoZoomToEdited, tv_getUseSoftSelect, tv_setUseSoftSelect, 
				tv_getSoftSelectRange, tv_setSoftSelectRange, tv_getSoftSelectFalloff, tv_setSoftSelectFalloff,
				tv_getRootTrack, tv_setRootTrack, tv_restoreRootTrack, tv_getScaleValuesOrigin, tv_setScaleValuesOrigin,
				tv_updateList,
				//symbolic enums
				tv_enumEffectTracks, tv_enumKeyTangentType, tv_editModeTypes, tv_enumTangentDisplay,
			};

		BEGIN_FUNCTION_MAP
			FN_0(tv_getName, TYPE_STRING, GetTVName);
			VFN_1(tv_setName, SetTVName, TYPE_STRING);
			VFN_0(tv_close, CloseTreeView);
			FN_0(tv_getNumTracks, TYPE_INT, GetNumTracks);
			FN_0(tv_numSelTracks, TYPE_INT, NumSelTracks);
			
			FN_0(tv_canAssignController, TYPE_BOOL, CanAssignController);
			VFN_0(tv_doAssignController, DoAssignController);
			FN_1(tv_assignController, TYPE_BOOL, AssignControllerToSelected, TYPE_REFTARG);
			VFN_1(tv_showControllerTypes, ShowControllerType, TYPE_BOOL);
			
			VFN_0(tv_expandTracks, ExpandTracks);
			VFN_0(tv_zoomSelected, ZoomSelected);
			VFN_2(tv_zoomOnTrack, ZoomOn, TYPE_REFTARG, TYPE_INT);

			FN_1(tv_getAnim, TYPE_REFTARG, GetAnim, TYPE_INDEX);
			FN_1(tv_getClient, TYPE_REFTARG, GetClient, TYPE_INDEX);

			FN_1(tv_getSelAnim, TYPE_REFTARG, fpGetSelectedAnimatable, TYPE_INDEX);
			FN_1(tv_getSelClient, TYPE_REFTARG, fpGetSelectedClient, TYPE_INDEX);
			FN_1(tv_getSelAnimSubNum, TYPE_INDEX, fpGetSelectedAnimSubNum, TYPE_INDEX);

			FN_1(tv_getIndex, TYPE_INDEX, GetIndex, TYPE_REFTARG);
			VFN_2(tv_selectTrackByIndex, SelectTrackByIndex, TYPE_INDEX, TYPE_BOOL);
			VFN_2(tv_selectTrack, SelectTrack, TYPE_REFTARG, TYPE_BOOL);

			FN_VA(tv_setFilter, TYPE_BOOL, fpSetFilter);
			FN_VA(tv_clearFilter, TYPE_BOOL, fpClearFilter);

			VFN_1(tv_setEditMode, SetEditMode, TYPE_ENUM);
			FN_0(tv_getEditMode, TYPE_ENUM, GetEditMode);
			PROP_FNS(tv_getEditModeProp, GetEditMode, tv_setEditModeProp, SetEditMode, TYPE_ENUM);
			FN_0(tv_getCurrent, TYPE_BOOL, IsCurrent);
			VFN_0(tv_setCurrent, SetCurrent);
			// UI interface property
			RO_PROP_FN(tv_getUIInterface, fpGetUIInterface, TYPE_INTERFACE);
			// dope sheet mode property
			PROP_FNS(tv_getModifySubTree, ModifySubTree, tv_setModifySubTree, ModifySubTree, TYPE_BOOL);
			PROP_FNS(tv_getModifyChildren, ModifyChildren, tv_setModifyChildren, ModifyChildren, TYPE_BOOL);
			//trackview utility methods
			VFN_0(tv_launchUtilityDialog, LaunchUtilityDialog);
			VFN_1(tv_launchUtility, LaunchUtility, TYPE_TSTR_BV);
			VFN_1(tv_closeUtility, CloseUtility, TYPE_TSTR_BV);
			FN_0(tv_getUtilityCount, TYPE_INT, GetUtilityCount);
			FN_1(tv_getUtilityName, TYPE_TSTR_BV, GetUtilityName, TYPE_INDEX);
			// key tangent type methods
			VFN_2(tv_setTangentType, SetTangentType, TYPE_ENUM, TYPE_ENUM);
			VFN_2(tv_setInTangentType, SetInTangentType, TYPE_ENUM, TYPE_ENUM);
			VFN_2(tv_setOutTangentType, SetOutTangentType, TYPE_ENUM, TYPE_ENUM);
			// button state properties
			PROP_FNS(tv_getInteractiveUpdate, InteractiveUpdate, tv_setInteractiveUpdate, InteractiveUpdate, TYPE_BOOL);
			PROP_FNS(tv_getSyncTime, SyncTime, tv_setSyncTime, SyncTime, TYPE_BOOL);
			PROP_FNS(tv_getFreezeSelKeys, FreezeSelKeys, tv_setFreezeSelKeys, FreezeSelKeys, TYPE_BOOL);
			PROP_FNS(tv_getFreezeNonSelCurves, FreezeNonSelCurves, tv_setFreezeNonSelCurves, FreezeNonSelCurves, TYPE_BOOL);
			PROP_FNS(tv_getShowNonSelCurves, ShowNonSelCurves, tv_setShowNonSelCurves, ShowNonSelCurves, TYPE_BOOL);
			PROP_FNS(tv_getShowTangents, ShowTangents, tv_setShowTangents, ShowTangents, TYPE_ENUM);
			PROP_FNS(tv_getShowFrozenKeys, ShowFrozenKeys, tv_setShowFrozenKeys, ShowFrozenKeys, TYPE_BOOL);
			// soft selection properties
			PROP_FNS(tv_getUseSoftSelect, UseSoftSelect, tv_setUseSoftSelect, UseSoftSelect, TYPE_BOOL);
			PROP_FNS(tv_getSoftSelectRange, SoftSelectRange, tv_setSoftSelectRange, SoftSelectRange, TYPE_TIMEVALUE);
			PROP_FNS(tv_getSoftSelectFalloff, SoftSelectFalloff, tv_setSoftSelectFalloff, SoftSelectFalloff, TYPE_FLOAT);
			// workflow properties
			PROP_FNS(tv_getEffectSelectedObjectsOnly, EffectSelectedObjectsOnly, tv_setEffectSelectedObjectsOnly, EffectSelectedObjectsOnly, TYPE_BOOL);
			PROP_FNS(tv_getManualNavigation, ManualNavigation, tv_setManualNavigation, ManualNavigation, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandChildren, AutoExpandChildren, tv_setAutoExpandChildren, AutoExpandChildren, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandTransforms, AutoExpandTransforms, tv_setAutoExpandTransforms, AutoExpandTransforms, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandObjects, AutoExpandObjects, tv_setAutoExpandObjects, AutoExpandObjects, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandModifiers, AutoExpandModifiers, tv_setAutoExpandModifiers, AutoExpandModifiers, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandMaterials, AutoExpandMaterials, tv_setAutoExpandMaterials, AutoExpandMaterials, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandXYZ, AutoExpandXYZ, tv_setAutoExpandXYZ, AutoExpandXYZ, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectAnimated, AutoSelectAnimated, tv_setAutoSelectAnimated, AutoSelectAnimated, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectPosition, AutoSelectPosition, tv_setAutoSelectPosition, AutoSelectPosition, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectRotation, AutoSelectRotation, tv_setAutoSelectRotation, AutoSelectRotation, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectScale, AutoSelectScale, tv_setAutoSelectScale, AutoSelectScale, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectXYZ, AutoSelectXYZ, tv_setAutoSelectXYZ, AutoSelectXYZ, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToRoot, AutoZoomToRoot, tv_setAutoZoomToRoot, AutoZoomToRoot, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToSelected, AutoZoomToSelected, tv_setAutoZoomToSelected, AutoZoomToSelected, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToEdited, AutoZoomToEdited, tv_setAutoZoomToEdited, AutoZoomToEdited, TYPE_BOOL);
			//root node methods
			PROP_FNS(tv_getRootTrack, GetRootTrack, tv_setRootTrack, SetRootTrack, TYPE_REFTARG);
			VFN_0(tv_restoreRootTrack, RestoreDefaultRootTrack);
			// scale values mode -- scale origin
			PROP_FNS(tv_getScaleValuesOrigin, ScaleValuesOrigin, tv_setScaleValuesOrigin, ScaleValuesOrigin, TYPE_FLOAT);
			VFN_0(tv_updateList, UpdateList);

		END_FUNCTION_MAP
		
			FPInterfaceDesc* GetDesc() { return GetDescByID(TREEVIEW_OPS_INTERFACE); }
			Interface_ID	GetID() { return TREEVIEW_OPS_INTERFACE; }

	private:
		//these methods are created to massage data into a format the function publishing system can interpret
		//these functions just call other public functions above
		//Added by AF (09/12/00)
		virtual Animatable* fpGetSelectedAnimatable(int index)=0;
		virtual Animatable* fpGetSelectedClient(int index)=0;
		virtual int fpGetSelectedAnimSubNum(int index)=0;
		virtual BOOL fpSetFilter(FPParams* val)=0;
		virtual BOOL fpClearFilter(FPParams* val)=0;

		// these are here so we don't break plugin compatibility
		// these can be called using FPInterface::Invoke()

		// Dope Sheet Mode effect children
		virtual BOOL ModifySubTree()=0;
		virtual void ModifySubTree(BOOL onOff)=0;
		virtual BOOL ModifyChildren()=0;
		virtual void ModifyChildren(BOOL onOff)=0;

		// Track View Utilities
		virtual void LaunchUtility(TSTR name)=0;
		virtual void LaunchUtilityDialog()=0;
		virtual void CloseUtility(TSTR name)=0;
		virtual int GetUtilityCount()=0;
		virtual TSTR GetUtilityName(int index)=0;

		// Update the viewport interactively, or on mouse up
		virtual BOOL InteractiveUpdate()=0;
		virtual void InteractiveUpdate(BOOL update)=0;

		// Sync Time with the mouse while clicking and dragging selections, keys, etc.
		virtual BOOL SyncTime()=0;
		virtual void SyncTime(BOOL sync)=0;

		// Button States
		virtual BOOL FreezeSelKeys()=0;
		virtual void FreezeSelKeys(BOOL onOff)=0;
		virtual BOOL FreezeNonSelCurves()=0;
		virtual void FreezeNonSelCurves(BOOL onOff)=0;
		virtual BOOL ShowNonSelCurves()=0;
		virtual void ShowNonSelCurves(BOOL onOff)=0;

		virtual int ShowTangents()=0;
		virtual void ShowTangents(int type)=0;
		virtual BOOL ShowFrozenKeys()=0;
		virtual void ShowFrozenKeys(BOOL onOff)=0;
		
		// Soft selection
		virtual void UseSoftSelect(BOOL use)=0;
		virtual BOOL UseSoftSelect()=0;
		virtual void SoftSelectRange(TimeValue range)=0;
		virtual TimeValue SoftSelectRange()=0;
		virtual void SoftSelectFalloff(float falloff)=0;
		virtual float SoftSelectFalloff()=0;

		// Adjust the key tangent types
		virtual void SetTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;
		virtual void SetInTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;
		virtual void SetOutTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;

		ITreeViewUI* fpGetUIInterface() { return (ITreeViewUI*)GetInterface(TREEVIEW_UI_INTERFACE); }

		// Workflow settings
		virtual BOOL EffectSelectedObjectsOnly()=0;
		virtual void EffectSelectedObjectsOnly(BOOL effect)=0;

		virtual BOOL ManualNavigation()=0;
		virtual void ManualNavigation(BOOL manual)=0;

		virtual BOOL AutoExpandChildren()=0;
		virtual void AutoExpandChildren(BOOL expand)=0;
		virtual BOOL AutoExpandTransforms()=0;
		virtual void AutoExpandTransforms(BOOL expand)=0;
		virtual BOOL AutoExpandObjects()=0;
		virtual void AutoExpandObjects(BOOL expand)=0;
		virtual BOOL AutoExpandModifiers()=0;
		virtual void AutoExpandModifiers(BOOL expand)=0;
		virtual BOOL AutoExpandMaterials()=0;
		virtual void AutoExpandMaterials(BOOL expand)=0;
		virtual BOOL AutoExpandXYZ()=0;
		virtual void AutoExpandXYZ(BOOL expand)=0;
			
		virtual BOOL AutoSelectAnimated()=0;
		virtual void AutoSelectAnimated(BOOL select)=0;
		virtual BOOL AutoSelectPosition()=0;
		virtual void AutoSelectPosition(BOOL select)=0;
		virtual BOOL AutoSelectRotation()=0;
		virtual void AutoSelectRotation(BOOL select)=0;
		virtual BOOL AutoSelectScale()=0;
		virtual void AutoSelectScale(BOOL select)=0;
		virtual BOOL AutoSelectXYZ()=0;
		virtual void AutoSelectXYZ(BOOL select)=0;

		virtual BOOL AutoZoomToRoot()=0;
		virtual void AutoZoomToRoot(BOOL zoom)=0;
		virtual BOOL AutoZoomToSelected()=0;
		virtual void AutoZoomToSelected(BOOL zoom)=0;
		virtual BOOL AutoZoomToEdited()=0;
		virtual void AutoZoomToEdited(BOOL zoom)=0;

		virtual ReferenceTarget* GetRootTrack()=0;
		virtual void SetRootTrack(ReferenceTarget* root)=0;
		virtual void RestoreDefaultRootTrack()=0;

		virtual float ScaleValuesOrigin()=0;
		virtual void ScaleValuesOrigin(float origin)=0;
		
		// force an update of the list
		virtual void UpdateList()=0;
};


class ITreeView : public IObject, public ITreeViewOps{
	public:
		
		virtual ~ITreeView() {}
		virtual void SetPos(int x, int y, int w, int h)=0;
		virtual void Show()=0;
		virtual void Hide()=0;
		virtual BOOL IsVisible()=0;
		virtual BOOL InViewPort()=0; 

		virtual void SetTreeRoot(ReferenceTarget *root,ReferenceTarget *client=NULL,int subNum=0)=0;
		virtual void SetLabelOnly(BOOL only)=0;

		virtual void SetMultiSel(BOOL on)=0;
		virtual void SetSelFilter(TrackViewFilter *f=NULL)=0;
		virtual void SetActive(BOOL active)=0;
		virtual BOOL IsActive()=0;
		virtual HWND GetHWnd()=0;
		virtual int GetTrackViewParent(int index)=0; // returns -1 if no parent is found

		virtual void Flush()=0;
		virtual void UnFlush()=0;
		virtual void SetMatBrowse()=0;
		virtual DWORD GetTVID()=0;

		//from IObject
		virtual TCHAR* GetIObjectName(){return _T("ITrackView");}
		virtual int NumInterfaces() { return IObject::NumInterfaces() + 1; }
		virtual BaseInterface* GetInterfaceAt(int index) {
							if (index == 0)
								return (ITreeViewOps*)this; 
							return IObject::GetInterfaceAt(index-1);
						}

		virtual BaseInterface* GetInterface(Interface_ID id) 
				{ 
					if (id == TREEVIEW_OPS_INTERFACE) 
						return (BaseInterface*)this; 
					else { 
						return IObject::GetInterface(id);
		 			}
		 			return NULL;
				} 

	};


//Added by AF (09/07/00)
//A CORE interface to get the trackview windows
//Use GetCOREInterface(ITRACKVIEWS) to get a pointer to this interface
//**************************************************
#define ITRACKVIEWS Interface_ID(0x531c5f2c, 0x6fdf29cf)

class ITrackViewArray : public FPStaticInterface
	{
	public:	
		int GetNumAvailableTrackViews();
		ITreeViewOps* GetTrackView(int index);
		ITreeViewOps* GetTrackView(TSTR name);
		Tab<ITreeViewOps*> GetAvaliableTrackViews();
		ITreeViewOps* GetLastActiveTrackView();

		BOOL IsTrackViewOpen(TSTR name);
		BOOL IsTrackViewOpen(int index);
		BOOL OpenTrackViewWindow(TSTR name, TSTR layoutName = _T(""), Point2 pos = Point2(-1.0f,-1.0f), 
								int width = -1, int height = -1, int dock = TV_FLOAT);
		BOOL OpenTrackViewWindow(int index);
		BOOL OpenLastActiveTrackViewWindow();

		BOOL CloseTrackView(TSTR name);
		BOOL CloseTrackView(int index);
		void DeleteTrackView(TSTR name);
		void DeleteTrackView(int index);

		TCHAR* GetTrackViewName(int index);
		TCHAR* GetLastUsedTrackViewName();

		BOOL IsTrackViewCurrent(int index);
		BOOL IsTrackViewCurrent(TSTR name);
		BOOL SetTrackViewCurrent(int index);
		BOOL SetTrackViewCurrent(TSTR name);

		BOOL TrackViewZoomSelected(TSTR tvName);
		BOOL TrackViewZoomOn(TSTR tvName, Animatable* anim, int subNum);
	
		enum{ getTrackView, getAvaliableTrackViews, getNumAvailableTrackViews,
			  openTrackView, closeTrackView, getTrackViewName, trackViewZoomSelected, trackViewZoomOn,
			  setFilter, clearFilter, pickTrackDlg, isOpen, openLastTrackView, currentTrackViewProp,
			  lastUsedTrackViewNameProp, deleteTrackView, isTrackViewCurrent, setTrackViewCurrent,

			  dockTypeEnum,
			};

		DECLARE_DESCRIPTOR(ITrackViewArray);

		BEGIN_FUNCTION_MAP
		FN_1(getTrackView, TYPE_INTERFACE, fpGetTrackView, TYPE_FPVALUE); 
		FN_0(getAvaliableTrackViews, TYPE_INTERFACE_TAB_BV, GetAvaliableTrackViews);
		FN_0(getNumAvailableTrackViews, TYPE_INT, GetNumAvailableTrackViews);

		FN_6(openTrackView, TYPE_BOOL, fpOpenTrackViewWindow, TYPE_FPVALUE, TYPE_TSTR_BV, TYPE_POINT2_BV, TYPE_INT, TYPE_INT, TYPE_ENUM);
		FN_1(closeTrackView, TYPE_BOOL, fpCloseTrackView, TYPE_FPVALUE);
		VFN_1(deleteTrackView, fpDeleteTrackView, TYPE_FPVALUE);

		FN_1(getTrackViewName, TYPE_STRING, GetTrackViewName, TYPE_INDEX);
		FN_1(trackViewZoomSelected, TYPE_BOOL, TrackViewZoomSelected, TYPE_TSTR);
		FN_3(trackViewZoomOn, TYPE_BOOL, TrackViewZoomOn, TYPE_TSTR, TYPE_REFTARG, TYPE_INDEX);
		FN_VA(setFilter, TYPE_BOOL, fpSetTrackViewFilter);
		FN_VA(clearFilter, TYPE_BOOL, fpClearTrackViewFilter);
		FN_VA(pickTrackDlg, TYPE_FPVALUE_BV, fpDoPickTrackDlg);
		FN_1(isOpen, TYPE_BOOL, fpIsTrackViewOpen, TYPE_FPVALUE);
		FN_0(openLastTrackView, TYPE_BOOL, OpenLastActiveTrackViewWindow);		
		RO_PROP_FN(currentTrackViewProp, GetLastActiveTrackView, TYPE_INTERFACE);
		RO_PROP_FN(lastUsedTrackViewNameProp, GetLastUsedTrackViewName, TYPE_STRING);
		FN_1(isTrackViewCurrent, TYPE_BOOL, fpIsTrackViewCurrent, TYPE_FPVALUE);
		FN_1(setTrackViewCurrent, TYPE_BOOL, fpSetTrackViewCurrent, TYPE_FPVALUE);
        END_FUNCTION_MAP

	private:
		// these functions are wrapper functions to massage maxscript specific values into standard values
		// These methods just call one of the corresponding public methods
		BOOL fpSetTrackViewFilter(FPParams* val);
		BOOL fpClearTrackViewFilter(FPParams* val);
		FPValue fpDoPickTrackDlg(FPParams* val);
		BOOL fpIsTrackViewOpen(FPValue* val);
		BOOL fpCloseTrackView(FPValue* val);
		void fpDeleteTrackView(FPValue* val);
		BOOL fpIsTrackViewCurrent(FPValue* val);
		BOOL fpSetTrackViewCurrent(FPValue* val);
		ITreeViewOps* fpGetTrackView(FPValue* val);
		BOOL fpOpenTrackViewWindow(FPValue* val,TSTR layoutName, Point2 pos, int width, int height, int dock);

	};


#define TRACK_SELSET_MGR_INTERFACE Interface_ID(0x18f36a84, 0x1f572eb7)
class TrackSelectionSetMgr : public FPStaticInterface
	{
public:
	virtual BOOL	Create(TSTR name, Tab<ReferenceTarget*> tracks, Tab<TCHAR*> trackNames)=0;
	virtual BOOL	Delete(TSTR name)=0;
	virtual int		Count()=0;
	virtual TSTR	GetName(int index)=0;
	virtual void	SetName(int index, TSTR name)=0;
	virtual TSTR	GetCurrent(ITreeView* tv)=0;
	virtual void	GetTracks(TSTR name, Tab<ReferenceTarget*> &tracks, Tab<TCHAR*> &trackNames)=0;
	virtual BOOL	Add(TSTR name, Tab<ReferenceTarget*> tracks, Tab<TCHAR*> trackNames)=0;
	virtual BOOL	Remove(TSTR name, Tab<ReferenceTarget*> tracks)=0;

	enum { kCreate, kDelete, kCount, kGetName, kSetName, 
			kGetCurrent, kGetTracks, kAdd, kRemove,
		};
	};

TrackSelectionSetMgr* GetTrackSelectionSetMgr();

// Defines for the "open" argument to Interface::CreateTreeViewChild
// *****************************************************************
// "Special" windows are TreeViews whose data can not customized and is not saved.
#define OPENTV_SPECIAL	-2
// "Custom" windows are TreeViews whose data can be customized but is not saved.
// R5.1 and later only. A TreeView will not be created if using this flag in a previous release.
#define OPENTV_CUSTOM	-3  
// These arguments should not be used by third party developers.  
// They are used to create the saved trackviews that appear in the Graph Editors Menu.
#define OPENTV_LAST		-1
#define OPENTV_NEW		0
// *****************************************************************

// Sent by a tree view window to its parent when the user right clicks
// on dead area of the toolbar.
// Mouse points are relative to the client area of the tree view window
//
// LOWORD(wParam) = mouse x
// HIWORD(wParam) = mouse y
// lparam         = tree view window handle
#define WM_TV_TOOLBAR_RIGHTCLICK	WM_USER + 0x8ac1

// Sent by a tree view window when the user double
// clicks on a track label.
// wParam = 0
// lParam = HWND of track view window
#define WM_TV_LABEL_DOUBLE_CLICK	WM_USER + 0x8ac2

class TrackViewActionCallback: public ActionCallback {
public:
    BOOL ExecuteAction(int id);
    void SetHWnd(HWND hWnd) { mhWnd = hWnd; }

    HWND mhWnd;
};

//-----------------------------------------------------------------
//
// Button IDs for the track view
	
#define ID_TV_TOOLBAR			200    // the toolbar itself (not valid in R5 and later)
//#define ID_TV_DELETE			210
#define ID_TV_DELETETIME		215
#define ID_TV_MOVE				220
#define ID_TV_SCALE				230
#define ID_TV_SCALETIME			250
//#define ID_TV_FUNCTION_CURVE	240
#define ID_TV_SNAPKEYS			260
#define ID_TV_ALIGNKEYS			270
#define ID_TV_ADD				280
//#define ID_TV_EDITKEYS			290
//#define ID_TV_EDITTIME			300
//#define ID_TV_EDITRANGE			310
//#define ID_TV_POSITIONRANGE		320
#define ID_TV_FILTERS			330
#define ID_TV_INSERT			340
#define ID_TV_CUT				350
#define ID_TV_COPY				360
#define ID_TV_PASTE				370
#define ID_TV_SLIDE				380
#define ID_TV_SELECT			390
#define ID_TV_REVERSE			400
#define ID_TV_LEFTEND			410
#define ID_TV_RIGHTEND			420
#define ID_TV_SUBTREE			430
#define ID_TV_ASSIGNCONTROL		440
#define ID_TV_MAKEUNIQUE		450
#define ID_TV_CHOOSEORT			460
#define ID_TV_SHOWTANGENTS		470
#define ID_TV_SHOWALLTANGENTS	475
#define ID_TV_SCALEVALUES		480
#define ID_TV_FREEZESEL			490
#define ID_TV_SHOWKEYSONFROZEN	495
#define ID_TV_TEMPLATE			500	//Same as ID_TV_FREEZENONSELCURVES
#define ID_TV_FREEZENONSELCURVES 500
#define ID_TV_HIDENONSELCURVES	505
#define ID_TV_LOCKTAN			510
#define ID_TV_PROPERTIES		520
#define ID_TV_NEWEASE			530
#define ID_TV_DELEASE			540
#define ID_TV_TOGGLEEASE		550
#define ID_TV_CHOOSE_EASE_ORT	560
#define ID_TV_CHOOSE_MULT_ORT	570
#define ID_TV_ADDNOTE			580
#define ID_TV_DELETENOTE		590
#define ID_TV_RECOUPLERANGE		600
#define ID_TV_COPYTRACK			610
#define ID_TV_PASTETRACK		620
#define ID_TV_REDUCEKEYS		630
#define ID_TV_ADDVIS			640
#define ID_TV_DELVIS			650
#define ID_TV_TVNAME			660
#define ID_TV_TVUTIL			670
//watje
#define ID_TV_GETSELECTED		680
#define ID_TV_DELETECONTROL		690

// Status tool button IDs
#define ID_TV_STATUS			1000
#define ID_TV_ZOOMREGION		1020
#define ID_TV_PAN				1030
#define ID_TV_VFITTOWINDOW		1040
#define ID_TV_HFITTOWINDOW		1050
#define ID_TV_SHOWSTATS			1060
#define ID_TV_TIMETYPEIN		1070
#define ID_TV_VALUETYPEIN		1080
#define ID_TV_ZOOM				1090
#define ID_TV_MAXIMIZE			1100
#define ID_TV_SELWILDCARD		1110
#define ID_TV_ZOOMSEL			1120

// From accelerators
#define ID_TV_K_SNAP			2000
//#define ID_TV_K_LOCKKEYS		2010
#define ID_TV_K_MOVEKEYS		2020
#define ID_TV_K_MOVEVERT		2030
#define ID_TV_K_MOVEHORZ		2040
#define ID_TV_K_SELTIME			2050
#define ID_TV_K_SUBTREE			2060
#define ID_TV_K_LEFTEND			2070
#define ID_TV_K_RIGHTEND		2080
#define ID_TV_K_TEMPLATE		2090
#define ID_TV_K_SHOWTAN			2100
#define ID_TV_K_LOCKTAN			2110
#define ID_TV_K_APPLYEASE		2120
#define ID_TV_K_APPLYMULT		2130
#define ID_TV_K_ACCESSTNAME		2140
#define ID_TV_K_ACCESSSELNAME	2150
#define ID_TV_K_ACCESSTIME		2160
#define ID_TV_K_ACCESSVAL		2170
#define ID_TV_K_ZOOMHORZ		2180
#define ID_TV_K_ZOOMHORZKEYS	2190
#define ID_TV_K_ZOOM			2200
#define ID_TV_K_ZOOMTIME		2210
#define ID_TV_K_ZOOMVALUE		2220
//#define ID_TV_K_NUDGELEFT		2230
//#define ID_TV_K_NUDGERIGHT		2240
//#define ID_TV_K_MOVEUP			2250
//#define ID_TV_K_MOVEDOWN		2260
//#define ID_TV_K_TOGGLENODE		2270
//#define ID_TV_K_TOGGLEANIM		2280
#define ID_TV_K_SHOWSTAT		2290
#define ID_TV_K_MOVECHILDUP		2300
#define ID_TV_K_MOVECHILDDOWN	2310

// Button IDs for the tangent type buttons
#define ID_TV_TANGENT_FLAT		2320
#define ID_TV_TANGENT_CUSTOM	2330
#define ID_TV_TANGENT_FAST		2340
#define ID_TV_TANGENT_SLOW		2350
#define ID_TV_TANGENT_STEP		2360
#define ID_TV_TANGENT_LINEAR	2370
#define ID_TV_TANGENT_SMOOTH	2380

//Button ID for the DrawCurves Mode button
#define ID_TV_DRAWCURVES		2390

// ID for the keyable property toggle action item (R5.1 and later only)
#define ID_TV_TOGGLE_KEYABLE	2400

// trackview filter name to mask map
#define FILTER_SELOBJECTS		(1<<0)
#define FILTER_SELCHANNELS		(1<<1)
#define FILTER_ANIMCHANNELS		(1<<2)

#define FILTER_WORLDMODS		(1<<3)
#define FILTER_OBJECTMODS		(1<<4)
#define FILTER_TRANSFORM		(1<<5)
#define FILTER_BASEPARAMS		(1<<6)

#define FILTER_POSX				(1<<7)
#define FILTER_POSY				(1<<8)
#define FILTER_POSZ				(1<<9)
#define FILTER_POSW				(1<<10)
#define FILTER_ROTX				(1<<11)
#define FILTER_ROTY				(1<<12)
#define FILTER_ROTZ				(1<<13)
#define FILTER_SCALEX			(1<<14)
#define FILTER_SCALEY			(1<<15)
#define FILTER_SCALEZ			(1<<16)
#define FILTER_RED				(1<<17)
#define FILTER_GREEN			(1<<18)
#define FILTER_BLUE				(1<<19)
#define FILTER_ALPHA			(1<<20)

#define FILTER_CONTTYPES		(1<<21)
#define FILTER_NOTETRACKS		(1<<22)
#define FILTER_SOUND			(1<<23)
#define FILTER_MATMAPS			(1<<24)
#define FILTER_MATPARAMS		(1<<25)
#define FILTER_VISTRACKS		(1<<26)


// More filter bits. These are stored in the 2nd DWORD.
#define FILTER_GEOM				(1<<0)
#define FILTER_SHAPES			(1<<1)
#define FILTER_LIGHTS			(1<<2)
#define FILTER_CAMERAS			(1<<3)
#define FILTER_HELPERS			(1<<4)
#define FILTER_WARPS			(1<<5)
#define FILTER_VISIBLE_OBJS		(1<<6)
#define FILTER_POSITION			(1<<7)
#define FILTER_ROTATION			(1<<8)
#define FILTER_SCALE			(1<<9)
#define FILTER_CONTX			(1<<10)
#define FILTER_CONTY			(1<<11)
#define FILTER_CONTZ			(1<<12)
#define FILTER_CONTW			(1<<13)
#define FILTER_STATICVALS		(1<<14)
#define FILTER_HIERARCHY		(1<<15)
#define FILTER_NODES			(1<<16)
#define FILTER_BONES			(1<<17)
#define FILTER_KEYABLE			(1<<18)

#define DEFAULT_TREEVIEW_FILTER0	(FILTER_WORLDMODS|FILTER_OBJECTMODS|FILTER_TRANSFORM|FILTER_BASEPARAMS| \
	FILTER_POSX|FILTER_POSY|FILTER_POSZ|FILTER_POSW|FILTER_ROTX|FILTER_ROTY|FILTER_ROTZ| \
	FILTER_SCALEX|FILTER_SCALEY|FILTER_SCALEZ|FILTER_RED|FILTER_GREEN|FILTER_BLUE|FILTER_ALPHA| \
	FILTER_NOTETRACKS|FILTER_MATMAPS|FILTER_MATPARAMS|FILTER_VISTRACKS|FILTER_SOUND)

#define DEFAULT_TREEVIEW_FILTER1	(FILTER_POSITION|FILTER_ROTATION|FILTER_SCALE| \
	FILTER_CONTX|FILTER_CONTY|FILTER_CONTZ|FILTER_CONTW|FILTER_VISIBLE_OBJS|FILTER_STATICVALS| \
	FILTER_HIERARCHY|FILTER_NODES)

// key tangent display setting
#define DISPLAY_TANGENTS_NONE		1
#define DISPLAY_TANGENTS_SELECTED	2
#define DISPLAY_TANGENTS_ALL		3

#endif

