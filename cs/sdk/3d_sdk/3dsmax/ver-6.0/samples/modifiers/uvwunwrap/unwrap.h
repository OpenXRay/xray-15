
#ifndef __UNWRAP__H
#define __UNWRAP__H

#include "mods.h"
#include "iparamm.h"
#include "meshadj.h"
#include "sctex.h"
#include "decomp.h"

#include "gport.h"
#include "bmmlib.h"
#include "macrorec.h"

#include "stdmat.h"
#include "MaxIcon.h"
#include "modsres.h"

#include "iunwrap.h"
#include "TVData.h"
#include "undo.h"
#include "mnmesh.h"
#include "polyobj.h"

#include "iMenuMan.h"

#include "iparamb2.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"

#ifndef WEBVERSION // orb 02-15-02 rename "uvwunwrap" to "unwrap texture"
#define UNWRAP_NAME		GetString(IDS_RB_UNWRAPMOD)
#else
#define UNWRAP_NAME		GetString(IDS_RB_UNWRAPMOD_PLASMA)
#endif // WEBVERSION

#define DEBUGMODE	1
#define ScriptPrint (the_listener->edit_stream->printf)

#define  TVVERTMODE		1
#define  TVEDGEMODE		2
#define  TVFACEMODE		3

class UnwrapMod;


#define			LINECOLORID			0x368408e0
#define			SELCOLORID			0x368408e1
#define			OPENEDGECOLORID		0x368408e2
#define			HANDLECOLORID		0x368408e3
#define			FREEFORMCOLORID		0x368408e4
#define			SHAREDCOLORID		0x368408e5
#define			BACKGROUNDCOLORID	0x368408e6
//new
#define			GRIDCOLORID			0x368408e7

#define CBS		(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL)



// Rightclick menu UI stuff

#define FACE4_CHUNK		0x0310
#define VEC4_CHUNK		0x0311

extern INT_PTR CALLBACK UnwrapFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//stitch dlg proc
extern INT_PTR CALLBACK UnwrapStitchFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//flatten dlg proc
extern INT_PTR CALLBACK UnwrapFlattenFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//unfold dlg proc
extern INT_PTR CALLBACK UnwrapUnfoldFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//unfold dlg proc
extern INT_PTR CALLBACK UnwrapNormalFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//unfold dlg proc
extern INT_PTR CALLBACK UnwrapPackFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//sketch dlg proc
extern INT_PTR CALLBACK UnwrapSketchFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//relax dlg proc
extern INT_PTR CALLBACK UnwrapRelaxFloaterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class SelectMode : public MouseCallBack {
	public:
		UnwrapMod *mod;
		BOOL region, toggle, subtract;
		IPoint2 om, lm;
		SelectMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		virtual int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)=0;
		virtual HCURSOR GetXFormCur()=0;
	};



class PaintSelectMode : public MouseCallBack {
	public:
		UnwrapMod *mod;
		BOOL  subtract,toggle;
		IPoint2 om, lm;
		PaintSelectMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};




class MoveMode : public SelectMode {
	public:				
		UnwrapMod *mod;
		MoveMode(UnwrapMod *m) : SelectMode(m) {mod = m;}
		int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};
class RotateMode : public SelectMode {
	public:				
		RotateMode(UnwrapMod *m) : SelectMode(m) {}
		int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};
class ScaleMode : public SelectMode {
	public:				
		ScaleMode(UnwrapMod *m) : SelectMode(m) {}
		int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};
class WeldMode : public SelectMode {
	public:				
		WeldMode(UnwrapMod *m) : SelectMode(m) {}
		int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};

class PanMode : public MouseCallBack {
	public:
		UnwrapMod *mod;
		IPoint2 om;
		float oxscroll, oyscroll;
		PanMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur(); 

	};
class ZoomMode : public MouseCallBack {
	public:
		UnwrapMod *mod;
		IPoint2 om;
		float ozoom;
		float oxscroll, oyscroll;
		ZoomMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};
class ZoomRegMode : public MouseCallBack {
	public:
		UnwrapMod *mod;
		IPoint2 om, lm;		
		ZoomRegMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};

class FreeFormMode : public SelectMode  {
	public:
		UnwrapMod *mod;
		IPoint2 om, lm;		
		BOOL dragging;
		FreeFormMode(UnwrapMod *m)  : SelectMode(m) {mod = m;dragging=FALSE;}
		int subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
	};

class SketchMode : public MouseCallBack {
	public:				
		UnwrapMod *mod;
		Tab<int> indexList;
		Tab<Point3*> pointList;
		Tab<Point3> tempPointList;
		Tab<IPoint2> ipointList;
		int lastPoint;
		IPoint2 prevPoint,currentPoint;	
		int pointCount;
		int drawPointCount;
		int oldMode;
		int oldSubObjectMode;
		SketchMode(UnwrapMod *m) {mod = m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		HCURSOR GetXFormCur();
//		int override(int mode) { return mode;}

		void GetIndexListFromSelection();
		void Apply(HWND hWnd, IPoint2 m);
		BOOL inPickLineMode;

	};


class RightMouseMode : public MouseCallBack {
	public:
		UnwrapMod *mod;		
		RightMouseMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
	};


class MiddleMouseMode : public MouseCallBack {
	public:
		UnwrapMod *mod;		
		IPoint2 om;
		float ozoom;
		float oxscroll, oyscroll;
		MiddleMouseMode(UnwrapMod *m) {mod=m;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
	};


//#define CID_FACEALIGNMAP 	0x4f298c7c
//#define CID_REGIONFIT 		0x4f298c7d





#define NumElements(array) (sizeof(array) / sizeof(array[0]))


//UNFOLD STUFF

//This is an edge class for the TV Face Data
//It is pretty simplistic but may grow as needed to support more advance tools
class BorderClass
{
	public:
	int innerFace,outerFace;	//Index into the face list of the left and right faces that touch this edge
								//This only support edges with 2 faces

	int edge;					//Index into the edge list for the mesh/patch/polyobj

	};


//Just a simple rect class to hold rects for sub clusters
class SubClusterClass
	{
public:
	float x,y;
	float w,h;

	SubClusterClass()
		{
		this->x = 0.0f;
		this->y = 0.0f;
		this->w = 0.0f;
		this->h = 0.0f;
		}

	SubClusterClass(float x, float y, float w, float h)
		{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		}
	};

//Class that contains data on a cluster.  A cluster is just a group of TV faces.  These do not have 
//to be contiguous 
class ClusterClass
	{
	public:
		Tab<int> faces;					//List of face indices that this cluster own into TVMaps.f
		Tab<BorderClass> borderData;	//List of border data for this cluster.  This is computed in Unwrap::BuildCluster
		Point3 normal;					//This is the geometric normal for this cluster
		Point3 offset;					//this is the min corner of the bounding box for the cluster
		float w,h;						//Width and Height of the cluster
		Box3 bounds;					//The bounding box for the cluster

		float newX, newY;				//Temp data to store offsets
		float surfaceArea;				//Surface area of the faces for the cluster

//this just though cluster building a bounding box for each face that is in this cluster
//UVW_ChannelClass &TVMaps needs to be passed in to get the vertex position data
		void BuildList(UVW_ChannelClass &TVMaps);

		Tab<Box3> boundsList;			//this is the result of the BuildList method.  
										//It is a list of bounding boxes for each face

//This determines whether the rect passed in intersects with any of the
//bounds in boundsList.  Basically it deteremines if a quad intersects
//any of the faces in this cluster based on bounding boxes
		BOOL DoesIntersect(float x, float y, float w, float h);

//this function finds a squre bounding box that fits in the clusters open space
		void	FindSquareX(int x, int y,									//the center of the square to start looking from
							int w, int h, BitArray &used,					//the bitmap and width and height of the bitmap
																			//used must be set to the right size and will be filled
							int &iretX, int &iretY, int &iretW, int &iretH); //the lower right of the square it is width and height that fits 

//this function finds a rect bounding box that fits in the clusters open space
		void	FindRectDiag(int x, int y,				//the starting point to start lookng from
							int w, int h,				//the width and height of the bitmap used grid
							int dirX, int dirY,			//the direction opf travel to progress through the map
														//needs to be 1 or -1
							BitArray &used,				// the map used list
							int &iretX, int &iretY, int &iretW, int &iretH);  //the bounding box return

//this function goes through the boundsList and finds all the open spaces
		int ComputeOpenSpaces(float spacing);

		Tab<SubClusterClass> openSpaces; // this is a list of bounding box open spaces in this cluster
										 // This is computed in ComputeOpenSpaces

	};

//Just another rect class but this one contains an area
class OpenAreaList
	{
public:
	float x,y;
	float w,h;
	float area;
	OpenAreaList() {}
	OpenAreaList(float x, float y, float w, float h)
		{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		area = w*h;
		}
	};

//COPYPASTE STUFF
//this is the copy and paste buffer class
//it contains the face and vertex data to pasted
class CopyPasteBuffer
	{
	public:
		~CopyPasteBuffer();
//			{
//			for (int i =0; i < faceData.Count(); i++)
//				delete faceData[i];
//			}

		BOOL CanPaste();
//			{
//			if (faceData.Count() == 0) return FALSE;
//			return TRUE;
//			}
		BOOL CanPasteInstance(UnwrapMod *mod);
//			{
//			if (faceData.Count() == 0) return FALSE;
//			if (this->mod != mod) return FALSE;
//			return TRUE;
//			}

		int iRotate;
		BitArray lastSel;
		Tab<Point3> tVertData;				//list of TV vertex data that is in the copy buffer

		Tab<UVW_TVFaceClass*> faceData;		//the list of face data for the copy buffer
		UnwrapMod *mod;						//pointer to the mod that did the last copy
											//we need this since we cannot instance paste across different modifiers
//FIX we neeed to put some support code in here to know when an lmd or mod goes away
		MeshTopoData *lmd;
		int copyType;						// 0 = one face
											// 1 = whole object
											// 2 = a sub selection
};




class UwrapAction;
class UnwrapActionCallback;


//class UnwrapFaceAlignMode;
//5.1.01
class UnwrapMod : public IUnwrapMod, TimeChangeCallback,IUnwrapMod2,IUnwrapMod3{	
	public:
		float falloffStr;
		BOOL forceUpdate;

		BOOL getFaceSelectionFromStack;
		int version;
		BOOL oldDataPresent;
		BOOL firstPass;
		BitArray vsel;
		BitArray esel,fsel;

		Tab<int> facehit;
		Control *tmControl;
		Control *offsetControl;
		Control *scaleControl;
		Control *rotateControl;


		Point3 gNormal,gCenter;
		float gXScale,gYScale, gZScale;


		DWORD flags;
		BOOL instanced;

		Point3 s;

		float zoom, aspect, xscroll, yscroll;
		Mtl *BaseMtl; 
		static int CurrentMap;
		Texmap *map[10];
		void AddMaterial(MtlBase *mtl);
		void LoadMaterials();
		void UpdateListBox();

		UBYTE *image;
		int iw, ih, uvw, move,scale;
		int rendW, rendH;
		int channel;
		int pixelSnap;
		int isBitmap;
		int bitmapWidth, bitmapHeight;
		BOOL useBitmapRes;
		int type;
		int objType;

		int zoomext;
		int lockSelected;
		int mirror;
		int hide;
		int freeze;
		int incSelected;
		int falloff;
		int falloffSpace;
		BOOL showMap;

		BOOL updateCache;

		static COLORREF lineColor, selColor;
		static COLORREF openEdgeColor;
		static COLORREF handleColor;
		static COLORREF freeFormColor;
		static COLORREF sharedColor;
		static COLORREF backgroundColor;
		COLORREF gridColor;


		static float weldThreshold;
		static BOOL update;
		static int showVerts;
		static int midPixelSnap;

	
		UVW_ChannelClass TVMaps;

//planar gizmo stuff
		Point3 gOffset;
		Point3 gScale;
		float gRotate;
		Matrix3 PlanarTM;
		int GizmoSelected;
		Tab<UVW_TVFaceClass*> gfaces;
		VertexLookUpListClass gverts;
		void ApplyGizmo();
		void ComputeSelectedFaceData();
	
//filter stuff
		int filterSelectedFaces;
		int matid;
		int alignDir;
		void BuildMatIDList();
		int IsFaceVisible(int i);
		int IsVertVisible(int i);
		Tab<int> filterMatID;
		BitArray vertMatIDList;
		void SetMatFilters();
	

		static HWND hParams, hWnd, hView;
		static HWND hSelRollup;
		static IObjParam  *ip;
		static UnwrapMod *editMod;
		static ICustToolbar *iTool;
		static ICustToolbar *iVertex;
		static ICustToolbar *iView;
		static ICustToolbar *iOption;
		static ICustToolbar *iFilter;
        static ICustButton *iApplyButton;

		HWND hTextures;
		HWND hMatIDs;

		static ICustButton *iMove, *iRot, *iScale,*iMirror, *iPan, *iZoom,*iFalloff ;
		static ICustButton *iZoomReg, *iZoomExt, *iUVW, *iProp, *iShowMap, *iUpdate;
		static ICustButton *iSnap,*iWeld,*iLockSelected,*iFilterSelected;
		static ICustButton *iHide,*iFreeze;
		static ICustButton *iIncSelected;
		static ICustButton *iDecSelected;
		static ICustButton *iFalloffSpace;

		static ICustButton *iWeldSelected, *iBreak;

		static ICustButton *iFreeForm;


		static ISpinnerControl *iU, *iV, *iW;
		static ISpinnerControl *iStr;
		static ISpinnerControl *iMapID;

		static ISpinnerControl *iPlanarThreshold;
		static ISpinnerControl *iMatID;
		static ISpinnerControl *iSG;


		static MouseManager mouseMan;
		static IOffScreenBuf *iBuf;

//watje tile
		static BOOL tileValid;
		static IOffScreenBuf *iTileBuf;


		static int mode;
		static int oldMode;
		static MoveMode *moveMode;
		static RotateMode *rotMode;
		static ScaleMode *scaleMode;
		static PanMode *panMode;
		static ZoomMode *zoomMode;
		static ZoomRegMode *zoomRegMode;
		static WeldMode *weldMode;
		static FreeFormMode *freeFormMode;
		static SketchMode *sketchMode;



		static RightMouseMode *rightMode;
		static MiddleMouseMode *middleMode;


		static SelectModBoxCMode *selectMode;

		static PaintSelectMode *paintSelectMode;

		int axis;
		int GetAxis();


		static BOOL viewValid, typeInsValid;

		Point2 center;
		int centeron;
		Matrix3 InverseTM;
		Point3 cdebug;


		UnwrapMod();
		~UnwrapMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s=UNWRAP_NAME;}
		virtual Class_ID ClassID() {return UNWRAP_CLASSID;}
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return UNWRAP_NAME; }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		BOOL AssignController(Animatable *control,int subAnim);

		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
		ChannelMask ChannelsChanged() {return TEXMAP_CHANNEL|PART_VERTCOLOR; }		
		Class_ID InputType() {return mapObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);		
		Interval GetValidity(TimeValue t);		

		int NumRefs() {
						int ct = 0;
						ct += TVMaps.cont.Count();
						return ct+11;
						}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int RemapRefOnLoad(int iref) ;

		// From BaseObject
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		Box3 BuildBoundVolume(Object *obj);


		void InitControl(TimeValue t);
		Matrix3 CompMatrix(TimeValue t,ModContext *mc, Matrix3 *ntm,BOOL applySize=TRUE, BOOL applyAxis=TRUE);
		void DoIcon(PolyLineProc& lp,BOOL sel);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		void ActivateSubobjSel(int level, XFormModes& modes);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		

		int NumSubs() {
						int ct = 0;
						ct += TVMaps.cont.Count();

						return ct;
						}

		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From TimeChangeCallback
		void TimeChanged(TimeValue t) {InvalidateView();}


		// Floater win methods
		void SetupDlg(HWND hWnd);
		void SizeDlg();
		void DestroyDlg();
		void PaintView();		
		void RegisterClasses();
		Point2 UVWToScreen(Point3 pt, float xzoom, float yzoom,int w,int h);
		void ComputeZooms(HWND hWnd, float &xzoom, float &yzoom,int &width,int &height);
		void SetMode(int m, BOOL updateMenuBar = TRUE);
		void InvalidateView();
		BOOL HitTest(Rect rect,Tab<int> &hits,BOOL selOnly,BOOL circleMode = FALSE);
		void Select(Tab<int> &hits,BOOL toggle,BOOL subtract,BOOL all);
		void ClearSelect();
		void HoldPoints();
		void HoldPointsAndFaces();
		void HoldSelection();
		void MovePoints(Point2 pt);
		void MoveGizmo(Point2 pt);
		void RotatePoints(HWND h, float ang);
		void RotateGizmo(HWND h, float ang);
		void ScalePoints(HWND h, float scale, int direction);
		void ScaleGizmo(HWND h, float scale, int direction);
		void MirrorPoints(HWND h, int direction,BOOL hold = TRUE);
		void MirrorGizmo(HWND h, int direction);
		void FlipPoints(int direction);
		void DetachEdgeVerts( BOOL hold = TRUE);
		void AlignMap();
		void PickMap();
		void SetupImage();
		void GetUVWIndices(int &i1, int &i2);
		void PropDialog();
		void PlugControllers();
		Point3 GetPoint(TimeValue t,int i);

		Tab<Point3> objectPointList;
		Point3 GetObjectPoint(TimeValue t,int i);
		void BuildObjectPoints();

		void ZoomExtents();
		void Reset();
		void SetupChannelButtons();
		void SetupTypeins();
		void InvalidateTypeins();
		void TypeInChanged(int which);
		void ChannelChanged(int which, float x);
		void SetVertexPosition(TimeValue t, int which, Point3 pos, BOOL hold = TRUE, BOOL update = TRUE);

		void SnapPoint( Point3 &p);
		void BreakSelected();
		void WeldSelected(BOOL hold = TRUE, BOOL notify = FALSE);
		BOOL WeldPoints(HWND h, IPoint2 m);

		void HideSelected();
		void UnHideAll();

		void FreezeSelected();
		void UnFreezeAll();

		void ZoomSelected();

		void DeleteSelected();

		void DeleteVertsFromFace(Tab<UVW_TVFaceClass*> f);
		void DeleteVertsFromFace(BitArray f);

		void UpdateFaceSelection(BitArray f);
		int IsSelected(int index);

		void ExpandSelection(int dir, BOOL rebuildCache = TRUE, BOOL hold = TRUE);

		void RebuildDistCache();
		void ComputeFalloff(float &u, int ftype);

		void LoadUVW(HWND hWnd);
		void SaveUVW(HWND hWnd);
		void TrackRBMenu(HWND hwnd, int x, int y);


		Tab<TSTR*> namedSel;		
		Tab<DWORD> ids;
		BOOL SupportsNamedSubSels() {return TRUE;}
		void ActivateSubSelSet(TSTR &setName);
		void NewSetFromCurSel(TSTR &setName);
		void RemoveSubSelSet(TSTR &setName);
		void SetupNamedSelDropDown();
		int NumNamedSelSets();
		TSTR GetNamedSelSetName(int i);
		void SetNamedSelSetName(int i,TSTR &newName);
		void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);

	// Local methods for handling named selection sets
		int FindSet(TSTR &setName);		
		DWORD AddSet(TSTR &setName);
		void RemoveSet(TSTR &setName);
		void ClearSetNames();

		void LocalDataChanged();
		IOResult LoadNamedSelChunk(ILoad *iload);
		IOResult SaveLocalData(ISave *isave, LocalModData *ld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
		void SetNumSelLabel();		

		void BuildInitialMapping(Mesh *msh);
		void BuildInitialMapping(MNMesh *msh);
		void BuildInitialMapping(PatchMesh *msh);

		void RemoveDeadVerts(PatchMesh *mesh,int CurrentChannel);
		int IsSelectedSetup();
		BitArray isSelected;
		BitArray wasSelected;

//watje 10-19-99 213458
		BOOL DependOnTopology(ModContext &mc) {return TRUE;}

		void SelectHandles(int dir);

		void AddPoint(Point3 p, int j, int k, BOOL sel = FALSE);
		void AddHandle(Point3 p, int fid, int vid, BOOL sel = FALSE);
		void AddInterior(Point3 p, int fid, int vid, BOOL sel = FALSE);


		BOOL InitializeMeshData(ObjectState *os, int CurrentChannel);
		BOOL InitializePatchData(ObjectState *os, int CurrentChannel);
		BOOL InitializeMNMeshData(ObjectState *os, int CurrentChannel);

		void CopySelectionMesh(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t);
		void CopySelectionMNMesh(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t);
		void CopySelectionPatch(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t);

		void GetFaceSelectionFromMesh(ObjectState *os, ModContext &mc, TimeValue t);
		void GetFaceSelectionFromPatch(ObjectState *os, ModContext &mc, TimeValue t);
		void GetFaceSelectionFromMNMesh(ObjectState *os, ModContext &mc, TimeValue t);


		void ApplyMNMeshMapping(ObjectState *os, int CurrentChannel, TimeValue t);
		void ApplyMeshMapping(ObjectState *os, int CurrentChannel, TimeValue t);
		void ApplyPatchMapping(ObjectState *os, int CurrentChannel, TimeValue t);

		//NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == UNWRAP_INTERFACE) 
				return (IUnwrapMod*)this; 
			else if (id == UNWRAP_INTERFACE2) 
				return (IUnwrapMod2*)this; 
			else if (id == UNWRAP_INTERFACE3) //5.1.05
				return (IUnwrapMod3*)this; 
			else 
				return Modifier::GetInterface(id);
				//return FPMixinInterface::IUnwrapMod::GetInterface(id);
			} 
		UnwrapActionCallback* pCallback;

		void	fnPlanarMap();
		void	fnSave();
		void	fnLoad();
		void	fnReset();
		void	fnEdit();
		void	fnSetMapChannel(int channel);
		int		fnGetMapChannel();

		void	fnSetProjectionType(int proj);
		int		fnGetProjectionType();

		void	fnSetVC(BOOL vc);
		BOOL	fnGetVC();

		void	fnMove();
		void	fnMoveH();
		void	fnMoveV();

		void	fnRotate();

		void	fnScale();
		void	fnScaleH();
		void	fnScaleV();

		void	fnMirrorH();
		void	fnMirrorV();

		void	fnExpandSelection();
		void	fnContractSelection();

		void	fnSetFalloffType(int falloff);
		int		fnGetFalloffType();
		void	fnSetFalloffSpace(int space);
		int		fnGetFalloffSpace();
		void	fnSetFalloffDist(float dist);
		float	fnGetFalloffDist();

		void	fnBreakSelected();
		void	fnWeld();
		void	fnWeldSelected();

		void	fnUpdatemap();
		void	fnDisplaymap(BOOL update);
		BOOL	fnIsMapDisplayed();

		void	fnSetUVSpace(int space);
		int		fnGetUVSpace();
		void	fnOptions();

		void	fnLock();
		void	fnHide();
		void	fnUnhide();

		void	fnFreeze();
		void	fnThaw();
		void	fnFilterSelected();

		void	fnPan();
		void	fnZoom();
		void	fnZoomRegion();
		void	fnFit();
		void	fnFitSelected();

		void	fnSnap();

		int		fnGetCurrentMap();
		void	fnSetCurrentMap(int map);
		int		fnNumberMaps();

		Point3  lColor,sColor;
		Point3*	fnGetLineColor();
		void	fnSetLineColor(Point3 color);

		Point3*	fnGetSelColor();
		void	fnSetSelColor(Point3 color);



		void	fnSetRenderWidth(int dist);
		int		fnGetRenderWidth();
		void	fnSetRenderHeight(int dist);
		int		fnGetRenderHeight();
		
		void	fnSetWeldThreshold(float dist);
		float	fnGetWeldThresold();

		void	fnSetUseBitmapRes(BOOL useBitmapRes);
		BOOL	fnGetUseBitmapRes();
		
		BOOL	fnGetConstantUpdate();
		void	fnSetConstantUpdate(BOOL constantUpdates);

		BOOL	fnGetShowSelectedVertices();
		void	fnSetShowSelectedVertices(BOOL show);

		BOOL	fnGetMidPixelSnape();
		void	fnSetMidPixelSnape(BOOL midPixel);

		int		fnGetMatID();
		void	fnSetMatID(int matid);
		int		fnNumberMatIDs();

		BitArray* fnGetSelectedVerts();
		void	fnSelectVerts(BitArray *sel);
		BOOL	fnIsVertexSelected(int index);

		BOOL	useCenter;
		Point3  tempCenter;
		Point3  tempVert;
		HWND	tempHwnd;
		int		tempDir;
		float	tempAmount;
		int		tempWhich;
		void	fnMoveSelectedVertices(Point3 offset);
		void	fnRotateSelectedVertices(float angle);
		void	fnRotateSelectedVertices(float angle, Point3 axis);
		void	fnScaleSelectedVertices(float scale,int dir);
		void	fnScaleSelectedVertices(float scale,int dir,Point3 axis);

		Point3* fnGetVertexPosition(TimeValue t, int index);
		int		fnNumberVertices();

		void	fnMoveX(float p);
		void	fnMoveY(float p);
		void	fnMoveZ(float p);

		BitArray* fnGetSelectedPolygons();
		void	fnSelectPolygons(BitArray *sel);
		BOOL	fnIsPolygonSelected(int index);
		int		fnNumberPolygons();

		void	fnDetachEdgeVerts();

		void	fnFlipH();
		void	fnFlipV();

		BOOL	lockAspect;
		float	mapScale;

		BOOL	fnGetLockAspect();
		void	fnSetLockAspect(BOOL a);

		float	fnGetMapScale();
		void	fnSetMapScale(float sc);

		WINDOWPLACEMENT windowPos;

		void	fnGetSelectionFromFace();
		void	fnForceUpdate(BOOL update);
		Box3	gizmoBounds;
		void	fnZoomToGizmo(BOOL all);

		void	fnSetVertexPosition(TimeValue t, int index, Point3 pos);
		void	fnMarkAsDead(int index);

		int		fnNumberPointsInFace(int index);
		int		fnGetVertexIndexFromFace(int index,int vertexIndex);
		int		fnGetHandleIndexFromFace(int index,int vertexIndex);
		int		fnGetInteriorIndexFromFace(int index,int vertexIndex);
		int		fnGetVertexGIndexFromFace(int index,int vertexIndex);
		int		fnGetHandleGIndexFromFace(int index,int vertexIndex);
		int		fnGetInteriorGIndexFromFace(int index,int vertexIndex);

		void	fnAddPoint(Point3 pos, int fIndex,int vIndex, BOOL sel);
		void	fnAddHandle(Point3 pos, int fIndex,int vIndex, BOOL sel);
		void	fnAddInterior(Point3 pos, int fIndex,int vIndex, BOOL sel);

		void	fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex);
		void	fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex);
		void	fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex);

		void	fnUpdateViews();

		void	fnGetFaceSelFromStack();
//UNFOLD STUFF
//cluster list is a tab of a cluster of faces
//it contains a list of indexs, the average normal for the  
		Tab<ClusterClass*> clusterList;

/*************************************************************************************

This function creates a bit array of all the faces that are within a normal and an angle threshold.

MeshTopoData *md  - pointer to the local data of Unwrap
BitArray &sel - where the selection of all faces within the threshold is stored
Point3 norm - the normal that the faces are checked against
float angle - the threshold of to apply against the norm
Tab<Point3> &normList - list of normals of all the faces.  If the tab is empty it will be filled in

**************************************************************************************/
		void	SelectFacesByNormals( MeshTopoData *md, BitArray &sel, Point3 norm, float angle,Tab<Point3> &normList);

/*************************************************************************************

This function creates a bit array of all contigous faces that are within a normal and an angle threshold.

MeshTopoData *md  - pointer to the local data of Unwrap
int seedFaceIndex, - this is the face that the function starts searching from
BitArray &sel - where the selection of all faces within the threshold is stored
Point3 norm - the normal of the seed face
float angle - the threshold of to apply against the norm
Tab<Point3> &normList - list of normals of all the faces.  If the tab is empty it will be filled in

**************************************************************************************/
		void	SelectFacesByGroup( MeshTopoData *md,BitArray &sel,int seedFaceIndex, Point3 norm, float angle, BOOL relative,Tab<Point3> &normList,
									Tab<BorderClass> &borderData,
									AdjEdgeList *edges=NULL);

		void	BuildNormals(MeshTopoData *md, Tab<Point3> &normList);
		void	FreeClusterList();
/***************************************************************************************
This builds clusters a cluster list based on angle threshold
returns true if it was successful, false if the user aborted it
****************************************************************************************/
		BOOL	BuildCluster( Tab<Point3> normalList, float threshold, BOOL connected, BOOL cleanUpStrayFaces);

/***************************************************************************************
This builds clusters a cluster list based on natural tv element cluster
returns true if it was successful, false if the user aborted it
****************************************************************************************/
		BOOL	BuildClusterFromTVVertexElement();
		void	NormalizeCluster(float spacing = 0.0f);

		void	PlanarMapNoScale(Point3 gNormal);

		void	fnSelectPolygonsUpdate(BitArray *sel, BOOL update);
		void	fnSelectFacesByNormal(Point3 normal, float angleThreshold, BOOL update);
		void	fnSelectClusterByNormal(float angleThreshold, int seedIndex, BOOL relative, BOOL update);


		int		normalMethod;
		float	normalSpacing;
		BOOL	normalNormalize;
		BOOL	normalRotate;
		BOOL	normalAlignWidth;
		void	fnNormalMap( Tab<Point3*> *normaList,  float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth);
		void	fnNormalMapNoParams();
		void	fnNormalMapDialog();
		void	SetNormalDialogPos();
		void	SaveNormalDialogPos();



		void	fnFlattenMap(float angleThreshold, Tab<Point3*> *normaList,  float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth);
		
		BOOL	unfoldNormalize;
		int		unfoldMethod;


		void	fnUnfoldSelectedPolygons(int unfoldMethod,BOOL normalize);
		void	fnUnfoldSelectedPolygonsDialog();
		void	fnUnfoldSelectedPolygonsNoParams();
		void	SetUnfoldDialogPos();
		void	SaveUnfoldDialogPos();

		BitArray hiddenPolygons;
		void	fnHideSelectedPolygons();
		void	fnUnhideAllPolygons();


		Point3 n;
		Point3* fnGetNormal(int faceIndex);
		Tab<int> seedFaces;
		void fnSetSeedFace();

		BOOL	LayoutClusters(float spacing, BOOL rotateCluster, BOOL alignWidth, BOOL combineClusters);
		BOOL	LayoutClusters2(float spacing, BOOL rotateCluster, BOOL combineClusters);
		float	PlaceClusters2(float area);

		void MeshUpdateGData(Mesh *mesh, BitArray faceSel);
		void PatchUpdateGData(PatchMesh *patch, BitArray faceSel);
		void PolyUpdateGData(MNMesh *mnMesh, BitArray faceSel);

		void AlignCluster(int baseCluster, int moveCluster, int innerFaceIndex, int outerFaceIndex,int edgeIndex);

		BOOL showVertexClusterList;
		Tab<int> vertexClusterList;
		Tab<int> vertexClusterListCounts;
		void	BuildVertexClusterList();
		void	fnShowVertexConnectionList();

		void	BuildUsedListFromSelection(BitArray &usedVerts);
		void	BuildUsedList(BitArray &usedList, int clusterIndex);
		void	BuildUsedList(BitArray &usedList, ClusterClass *joinCluster);

		void	JoinCluster(ClusterClass *baseCluster, ClusterClass *joinCluster);
		void	JoinCluster(ClusterClass *baseCluster, ClusterClass *joinCluster, float x, float y);

		void	FlipSingleCluster(int i,float spacing);

		void	FlipClusters(BOOL flipW,float spacing);
//returns the total area of the faces
		BOOL	RotateClusters(float &area);
		BOOL	CollapseClusters(float spacing);

//COPYPASTE
		void	fnCopy();
		void	fnPaste(BOOL rotate);
		void	fnPasteInstance();
		void	CleanUpDeadVertices();

		void	fnSetDebugLevel(int level);

		void	fnStitchVerts(BOOL bAlign, float fBias);

		void	fnSelectElement();

		
		BOOL bStitchAlign;
		float fStitchBias;
		void	fnStitchVertsNoParams();
		void	fnStitchVertsDialog();
		void	SetStitchDialogPos();
		void	SaveStitchDialogPos();



		void	SelectElement();

		float	flattenAngleThreshold;
		float	flattenSpacing;
		BOOL	flattenNormalize;
		BOOL	flattenRotate;
		BOOL	flattenCollapse;

		void	fnFlattenMapDialog();
		void	fnFlattenMapNoParams();
		void	SetFlattenDialogPos();
		void	SaveFlattenDialogPos();




		HWND stitchHWND;
		HWND flattenHWND;
		HWND unfoldHWND;
		HWND sketchHWND;
		HWND normalHWND;

		BOOL	fnGetTile();
		void	fnSetTile(BOOL tile);

		int		fnGetTileLimit();
		void	fnSetTileLimit(int lmit);

		float	fnGetTileContrast();
		void	fnSetTileContrast(float contrast);

//action Table methods
		void DeActivateActionTable();
		void ActivateActionTable();
		BOOL WtIsChecked(int id);
		BOOL WtIsEnabled(int id);
		BOOL WtExecute(int id);

		BOOL	fnGetShowMap();
		void	fnSetShowMap(BOOL smap);
		void	fnShowMap();

		void	InitScriptUI();
		void	LaunchScriptUI();
		void	EndScriptUI();
		void	MoveScriptUI();

		BOOL	fnGetLimitSoftSel();
		void	fnSetLimitSoftSel(BOOL limit);

		int		fnGetLimitSoftSelRange();
		void	fnSetLimitSoftSelRange(int range);


		float	fnGetVertexWeight(int index);
		void	fnSetVertexWeight(int index,float weight);

		BOOL	fnIsWeightModified(int index);
		void	fnModifyWeight(int index, BOOL modified);

		BOOL	fnGetGeomElemMode();
		void	fnSetGeomElemMode(BOOL elem);
		void	SelectGeomElement(MeshTopoData* tmd);


		BOOL	fnGetGeomPlanarMode();
		void	fnSetGeomPlanarMode(BOOL planar);


		void	SelectGeomFacesByAngle(MeshTopoData* tmd);
		float	fnGetGeomPlanarModeThreshold();
		void	fnSetGeomPlanarModeThreshold(float threshold);


		int		fnGetWindowX();
		int		fnGetWindowY();
		int		fnGetWindowW();
		int		fnGetWindowH();

		BOOL	fnGetBackFaceCull();
		void	fnSetBackFaceCull(BOOL backFaceCull);


		BOOL	fnGetOldSelMethod();
		void	fnSetOldSelMethod(BOOL oldSelMethod);

		void	fnSelectByMatID(int matID);

		void	fnSelectBySG(int sg);

		BOOL	fnGetTVElementMode();
		void	fnSetTVElementMode(BOOL mode);


		void	GeomExpandFaceSel(MeshTopoData* tmd);
		void	GeomContractFaceSel(MeshTopoData* tmd);
		void	fnGeomExpandFaceSel();
		void	fnGeomContractFaceSel();

		BOOL	fnGetAlwaysEdit();
		void	fnSetAlwaysEdit(BOOL always);

		BOOL	fnGetShowConnection();
		void	fnSetShowConnection(BOOL show);

		BOOL	fnGetFilteredSelected();
		void	fnSetFilteredSelected(BOOL filter);


		BOOL	fnGetSnap();
		void	fnSetSnap(BOOL snap);

		BOOL	fnGetLock();
		void	fnSetLock(BOOL snap);

		void	Pack(int method, float spacing, BOOL normalize, BOOL rotate, BOOL fillHoles);
		void	fnPack( int method, float spacing, BOOL normalize, BOOL rotate, BOOL fillHoles);
		void	fnPackNoParams();
		void	fnPackDialog();
		void	SetPackDialogPos();
		void	SavePackDialogPos();
		HWND	packHWND;
//Packing vars
		int packMethod;
		float packSpacing;
		BOOL packNormalize;
		BOOL packRotate;
		BOOL packFillHoles;

		int		fnGetTVSubMode();
		void	fnSetTVSubMode(int mode);

		BitArray* fnGetSelectedFaces();
		void	fnSelectFaces(BitArray *sel);
		BOOL	fnIsFaceSelected(int index);

		//Converts the face selection into a vertex selection set
		void    GetVertSelFromFace(BitArray &sel);
		//Converts the vertex selection into a face selection set
		//PartialSelect determines whether all the vertices of a face need to be selected for that face to be selected
		void    GetFaceSelFromVert(BitArray &sel,BOOL partialSelect);


		//these must be called in pairs
		//this transfer our current selection into vertex selection
		

		void	TransferSelectionStart();
		//this transfer our vertex selection into our curren selection
		void	TransferSelectionEnd(BOOL partial,BOOL recomputeSelection);

		inline BOOL LineIntersect(Point3 p1, Point3 p2, Point3 q1, Point3 q2);
		int PolyIntersect(Point3 p, int i1, int i2);

		int		fnGetFillMode();
		void	fnSetFillMode(int mode);

		void	fnMoveSelected(Point3 offset);
		void	fnRotateSelected(float angle);
		void	fnRotateSelected(float angle, Point3 axis);
		void	fnScaleSelected(float scale,int dir);
		void	fnScaleSelected(float scale,int dir,Point3 axis);
		
		BitArray* fnGetSelectedEdges();
		void	fnSelectEdges(BitArray *sel);
		BOOL	fnIsEdgeSelected(int index);

		//Converts the edge selection into a vertex selection set
		void    GetVertSelFromEdge(BitArray &sel);
		//Converts the vertex selection into a edge selection set
		//PartialSelect determines whether all the vertices of a edge need to be selected for that edge to be selected
		void    GetEdgeSelFromVert(BitArray &sel,BOOL partialSelect);


		BOOL	fnGetDisplayOpenEdges();
		void	fnSetDisplayOpenEdges(BOOL openEdgeDisplay);
		
		Point3*	fnGetOpenEdgeColor();
		void	fnSetOpenEdgeColor(Point3 color);
	
		void	RebuildEdges();
		

		BOOL	fnGetUVEdgeMode();
		void	fnSetUVEdgeMode(BOOL uvmode);

		void	SelectUVEdge();	
		void	SelectOpenEdge();
		
		void	GrowSelectOpenEdge();
		void	ShrinkSelectOpenEdge();

		void	GrowSelectUVEdge();
		void	ShrinkSelectUVEdge();


		BOOL	fnGetOpenEdgeMode();
		void	fnSetOpenEdgeMode(BOOL uvmode);

		void	fnUVEdgeSelect();
		void	fnOpenEdgeSelect();

		void	fnVertToEdgeSelect();
		void	fnVertToFaceSelect();

		void	fnEdgeToVertSelect();
		void	fnEdgeToFaceSelect();

		void	fnFaceToVertSelect();
		void	fnFaceToEdgeSelect();

		BOOL	fnGetDisplayHiddenEdges();
		void	fnSetDisplayHiddenEdges(BOOL hiddenEdgeDisplay);

		Point3*	fnGetHandleColor();
		void	fnSetHandleColor(Point3 color);

		BOOL	fnGetFreeFormMode();
		void	fnSetFreeFormMode(BOOL freeFormMode);

		Point3*	fnGetFreeFormColor();
		void	fnSetFreeFormColor(Point3 color);

		int freeFormSubMode;
		int scaleCorner;
		int scaleCornerOpposite;
		Point3  freeFormCorners[4];
		Point2  freeFormCornersScreenSpace[4];
		Point3  freeFormEdges[4];
		Point2  freeFormEdgesScreenSpace[4];
		
		Point3 selCenter;
		Point3	freeFormPivotOffset;
		Point2	freeFormPivotScreenSpace;
		Box3   freeFormBounds;
		BOOL   inRotation;
		Point3 origSelCenter;
		float currentRotationAngle;

		void ScalePointsXY(HWND h, float scaleX, float scaleY);
		void fnScaleSelectedXY(float scaleX,float scaleY,Point3 axis);

		void	fnSnapPivot(int pos);
		Point3*	fnGetPivotOffset();
		void	fnSetPivotOffset(Point3 color);
		Point3*	fnGetSelCenter();
		void	fnGetSelCenter(Point3 color);

		void	Sketch(Tab<int> *indexList, Tab<Point3*> *positionList, BOOL closed = FALSE);
		void	fnSketch(Tab<int> *indexList, Tab<Point3*> *positionList);
		void	fnSketchNoParams();
		void	fnSketchReverse();
		void	fnSketchSetFirst(int index);
		void	fnSketchDialog();
		void	SetSketchDialogPos();
		void	SaveSketchDialogPos();

		BOOL	restoreSketchSettings;
		int		restoreSketchSelMode;
		int		restoreSketchType;
		
		BOOL	restoreSketchInteractiveMode;
		BOOL	restoreSketchDisplayPoints;

		int		restoreSketchCursorSize;
		
		int		sketchSelMode;
		int		sketchType;
		BOOL	sketchInteractiveMode;
		BOOL	sketchDisplayPoints;
		int		sketchCursorSize;

		static HCURSOR selCur  ;
		static HCURSOR moveCur ;
		static HCURSOR moveXCur;
		static HCURSOR moveYCur;
		static HCURSOR rotCur ;
		static HCURSOR scaleCur;
		static HCURSOR scaleXCur;
		static HCURSOR scaleYCur;

		static HCURSOR zoomCur;
		static HCURSOR zoomRegionCur;
		static HCURSOR panCur ;
		static HCURSOR weldCur ;
		static HCURSOR weldCurHit ;

		static HCURSOR sketchCur ;
		static HCURSOR sketchPickCur ;
		static HCURSOR sketchPickHitCur ;

		HCURSOR circleCur[16];


		Tab<int> sketchBelongsToList;
		Tab<Point3> originalPos;
		void InitReverseSoftData();
		void ApplyReverseSoftData();


		int		fnGetHitSize();
		void	fnSetHitSize(int size);



		BOOL	fnGetResetPivotOnSel();
		void	fnSetResetPivotOnSel(BOOL reset);

		BOOL	fnGetPolyMode();
		void	fnSetPolyMode(BOOL pmode);
		void	fnPolySelect();
		void	ConvertFaceToEdgeSel();


		BOOL	fnGetAllowSelectionInsideGizmo();
		void	fnSetAllowSelectionInsideGizmo(BOOL select);
		void	RebuildFreeFormData();
	

		void	GetCfgFilename( TCHAR *filename );
		void	fnSetAsDefaults();
		void	fnLoadDefaults();


		void	fnSetSharedColor(Point3 color);
		Point3*	fnGetSharedColor();

		BOOL	fnGetShowShared();
		void	fnSetShowShared(BOOL share);

		inline  void DrawEdge(HDC hdc, int a,int b, int va,int vb, 
						 IPoint2 pa, IPoint2 pb, IPoint2 pva, IPoint2 pvb);


		void	fnShowIcon(int icon, BOOL show);


		BOOL	fnGetSyncSelectionMode();
		void	fnSetSyncSelectionMode(BOOL sync);

		void	SyncTVToGeomSelection(MeshTopoData *md);
		void	SyncGeomToTVSelection(MeshTopoData *md);

		void	fnSyncTVSelection();
		void	fnSyncGeomSelection();

		Point3*	fnGetBackgroundColor();
		void	fnSetBackgroundColor(Point3 color);

		void	fnUpdateMenuBar();

		BOOL	fnGetBrightCenterTile();
		void	fnSetBrightCenterTile(BOOL bright);

		BOOL	fnGetBlendToBack();
		void	fnSetBlendToBack(BOOL blend);


		BOOL	fnGetPaintMode();
		void	fnSetPaintMode(BOOL paint);


		int		fnGetPaintSize();
		void	fnSetPaintSize(int size);


		void	fnIncPaintSize();
		void	fnDecPaintSize();


		int		fnGetTickSize();
		void	fnSetTickSize(int size);

//new
		float	fnGetGridSize();
		void	fnSetGridSize(float size);

		BOOL	fnGetGridSnap();
		void	fnSetGridSnap(BOOL snap);
		BOOL	fnGetGridVisible();
		void	fnSetGridVisible(BOOL visible);

		Point3*	fnGetGridColor();
		void	fnSetGridColor(Point3 color);

		
		float	fnGetGridStr();
		void	fnSetGridStr(float str);

		BOOL	fnGetAutoMap();
		void	fnSetAutoMap(BOOL autoMap);


		float	fnGetFlattenAngle();
		void	fnSetFlattenAngle(float angle);

		float	fnGetFlattenSpacing();
		void	fnSetFlattenSpacing(float spacing);

		BOOL	fnGetFlattenNormalize();
		void	fnSetFlattenNormalize(BOOL normalize);

		BOOL	fnGetFlattenRotate();
		void	fnSetFlattenRotate(BOOL rotate);

		BOOL	fnGetFlattenFillHoles();
		void	fnSetFlattenFillHoles(BOOL fillHoles);

		BOOL	fnGetPreventFlattening();
		void	fnSetPreventFlattening(BOOL preventFlattening);

		BOOL	fnGetEnableSoftSelection();
		void	fnSetEnableSoftSelection(BOOL enable);


		BOOL	floaterWindowActive;
		BOOL	optionsDialogActive;

		BOOL	fnGetApplyToWholeObject();
		void	fnSetApplyToWholeObject(BOOL wholeObject);


		int		tWeldHit;

		void	fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update);
		void	fnRelax(int iteration, float str, BOOL lockEdges, BOOL matchArea);
		void	RelaxPatch(int iteration, float str, BOOL lockEdges);

		Matrix3 GetTMFromFace(int index, BOOL useTVFace);
		void	fnFit(int iteration, float str);

		BOOL bringUpPanel;
		BOOL gridSnap;

		//5.1.05
		BOOL	autoBackground;
		BOOL	fnGetAutoBackground();
		void	fnSetAutoBackground(BOOL autoBackground);

//NEW RELAX
		float	relaxAmount;
		int		relaxIteration;
		BOOL	relaxBoundary, relaxSaddle;
		float	fnGetRelaxAmount();
		void	fnSetRelaxAmount(float amount);

		int		fnGetRelaxIter();
		void	fnSetRelaxIter(int iter);

		BOOL	fnGetRelaxBoundary();
		void	fnSetRelaxBoundary(BOOL boundary);

		BOOL	fnGetRelaxSaddle();
		void	fnSetRelaxSaddle(BOOL saddle);


		void	fnRelax2();
		void	fnRelax2Dialog();
		HWND	relaxHWND;
		void	SetRelaxDialogPos();
		void	SaveRelaxDialogPos();

		WINDOWPLACEMENT relaxWindowPos;


		void	RelaxVerts2(float relax, int iter, BOOL boundary, BOOL saddle);


		private:

			BOOL modifierInstanced;

			BOOL applyToWholeObject;

			BOOL loadDefaults;

			BOOL enableSoftSelection;

			BOOL preventFlattening;

//new		
			float gridSize;
			BOOL  gridVisible;
			BOOL autoMap;
			int mouseHitVert;
			Point2 mouseHitPos;
			int tickSize;
			float gridStr;


			int paintSize;

			BOOL blendTileToBackGround;
			BOOL brightCenterTile;

			BOOL syncSelection;

			int viewSize;
			int filterSize;
			int optionSize;
			int vertSize;
			int toolSize;
			BitArray showIconList;

			BOOL showShared;

			BOOL allowSelectionInsideGizmo;

			BOOL polyMode;

			BOOL resetPivotOnSel;
			int hitSize;
			
			WINDOWPLACEMENT sketchWindowPos;
			

			BOOL displayHiddenEdges;

			BOOL openEdgeMode;

			BOOL uvEdgeMode;

			BOOL displayOpenEdges;
			TimeValue currentTime;

			int fillMode;

			BitArray originalSel;
			BitArray holdFSel;
			BitArray holdESel;

			int TVSubObjectMode;
			


			WINDOWPLACEMENT packWindowPos;


//This is a bool that if enabled will pop up the edit dialog when ever the rollup ui is put up
			BOOL alwaysEdit;

			BOOL tvElementMode;

			static BOOL executedStartUIScript;
//old selection method, where if you single clicked it was culled, dragged it was not
			BOOL oldSelMethod;
//backface cull
			BOOL ignoreBackFaceCull;
//planar threshold for select
			BOOL planarMode;
			float planarThreshold;
//elem mode stuff
			BOOL geomElemMode;

//edge limit data
			BOOL limitSoftSel;
			int limitSoftSelRange;


//tile data
			BOOL bTile;
			float fContrast;
			int iTileLimit;

//stitch parameters
			WINDOWPLACEMENT stitchWindowPos;
			WINDOWPLACEMENT flattenWindowPos;

			WINDOWPLACEMENT unfoldWindowPos;

			WINDOWPLACEMENT normalWindowPos;


			float gBArea;		//surface area of all the texture faces
			float gSArea;		//the area of the bounding box surrounding the faces
			float gEdgeHeight, gEdgeWidth;  
//			float gEdgeLenH,gEdgeLenW;
			int gDebugLevel;	//debug level	0 means no debug info will be displayed
								//				1 means that a minimal amount mostly to the listener window
								//				2 means that a fair amount to the listener window and some display debug info
								//				3 means that a alot of info to the listener window and the display 


			
			Tab<SubClusterClass> tempSubCluster;

			float fEdgeLenW;
			float fEdgeLenH;


			DWORD mMaxProcessID;
			void BailStart () 
				{
				GetAsyncKeyState (VK_ESCAPE);
				mMaxProcessID = GetCurrentProcessId();
				}

			BOOL CheckESC () 
				{
				if (!GetAsyncKeyState (VK_ESCAPE)) return false;
				DWORD processID=0;
				HWND hForeground = GetForegroundWindow ();
				if (hForeground) GetWindowThreadProcessId (hForeground, &processID);
				return (processID == mMaxProcessID) ? TRUE : FALSE;
				}


			inline BOOL Bail(Interface *ip = NULL, TCHAR *status = NULL, int skip=10)
				{
				static unsigned long lSkip = 0;
				lSkip++;
				if ((ip) && (status) && ((skip==0)||((lSkip%skip) == 0)))
					ip->ReplacePrompt(status);
				return CheckESC();
/*				

				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767)
					{
					HWND topWindow =GetTopWindow(NULL);
					if ((  topWindow != flattenHWND) && (hWnd!=NULL))
						return FALSE;

					while (GetAsyncKeyState (VK_ESCAPE))
						{
						}	
					return TRUE;
					}
				else return FALSE;
				*/
				}	

			int sourcePoints[2];
			int targetPoints[2];
			void	GetEdgeCluster(BitArray &cluster);


			static CopyPasteBuffer copyPasteBuffer;

			int subObjCount;

			MeshTopoData *GetModData();
			BitArray usedVertices;		
			BOOL IsInStack(INode *node);

			//5.1.02 adds new bitmap bg management
			enum { multi_params, };  		// pblock ID

			enum							// multi_params param IDs
				{	multi_mtls,
					multi_ons,
					multi_names,
					multi_ids, };

			Tab<Mtl*>  matPairList;
			BOOL popUpDialog;

};

class MeshTopoData : public LocalModData {
public:
	Mesh *mesh;
	PatchMesh *patch;
	MNMesh *mnMesh;

	BitArray faceSel;
	GenericNamedSelSetList fselSet;

	MeshTopoData(Mesh &mesh);
	MeshTopoData(MNMesh &mesh);
	MeshTopoData(PatchMesh &patch);
	MeshTopoData() { mesh=NULL; patch = NULL; mnMesh= NULL;}
	~MeshTopoData() { FreeCache(); }
	LocalModData *Clone();

	Mesh *GetMesh() {return mesh;}
	MNMesh *GetMNMesh() {return mnMesh;}
	PatchMesh *GetPatch() {return patch;}
	void SetCache(Mesh &mesh);
	void SetCache(MNMesh &mesh);
	void SetCache(PatchMesh &patch);
	void FreeCache();

	BitArray &GetFaceSel() { return faceSel; }
	void SetFaceSel(BitArray &set, UnwrapMod *imod, TimeValue t);

};



// action table
// Keyboard Shortcuts stuff
const ActionTableId kUnwrapActions = 0x7bd55e42;
const ActionContextId kUnwrapContext = 0x7bd55e43;
const int   kUnwrapMenuBar = 2292144;


class UnwrapActionCallback : public ActionCallback
	{
	public:
	BOOL ExecuteAction(int id)
		{
		if (pUnwrap)
			return pUnwrap->WtExecute(id);
		return FALSE;
		}
	void SetUnwrap(UnwrapMod *pUnwrap) {this->pUnwrap = pUnwrap;}
	private:
		UnwrapMod *pUnwrap;
	};

class UnwrapAction : public ActionItem
{
	public:

//ActionItem methods
		BOOL IsChecked()						{if (pUnwrap)
													return pUnwrap->WtIsChecked(id);
												else return FALSE;}
		void GetMenuText(TSTR& menuText)		{menuText.printf("%s",this->menuText);}
		void GetButtonText(TSTR& buttonText)	{buttonText.printf("%s",this->buttonText);	}
		void GetCategoryText(TSTR& catText)		{catText.printf("%s",this->catText);}
		void GetDescriptionText(TSTR& descText) {descText.printf("%s",this->descText);}
		BOOL ExecuteAction()					{if (pUnwrap)
													return pUnwrap->WtExecute(id);
												else return FALSE;
												}
		int GetId()								{
												return id;
												}
		BOOL IsItemVisible()					{if (pUnwrap)
													return TRUE;
												else return FALSE;}
		BOOL IsEnabled()					{if (pUnwrap)
													return pUnwrap->WtIsEnabled(id);
												else return FALSE;}
		MaxIcon* GetIcon() 					{return NULL;}
		void DeleteThis() {delete this;}

//WeightTableAction methods
		void Init(int id,TCHAR *mText, TCHAR *bText,TCHAR *cText, TCHAR *dText)
			{
			pUnwrap = NULL;
			this->id = id;
			menuText.printf("%s",mText);	
			buttonText.printf("%s",bText);	
			descText.printf("%s",dText);	
			catText.printf("%s",cText);	
			}
		void SetUnwrap(UnwrapMod *pUnwrap) {this->pUnwrap = pUnwrap;}
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
		UnwrapMod *pUnwrap;
		TSTR buttonText, menuText, descText, catText;

};

static int spActions[] = {

	ID_PLANAR_MAP,
    IDS_PW_PLANARLONG,
    IDS_PW_PLANARMAP,

	ID_SAVE,
	IDS_PW_SAVEOBJECT,
	IDS_PW_SAVEOBJECT,

	ID_LOAD,
	IDS_PW_LOADOBJECT,
	IDS_PW_LOADOBJECT,

	ID_RESET,
	IDS_RB_RESETUVWS,
	IDS_RB_RESETUVWS,

	ID_EDIT,
	IDS_PW_EDITUVW,
	IDS_PW_EDITUVW,

	ID_MOVE,
	IDS_PW_MOVELONG,
	IDS_PW_MOVE_UVW,

	ID_MOVEH,
	IDS_PW_MOVEH,
	IDS_PW_MOVEH,

	ID_MOVEV,
	IDS_PW_MOVEV,
	IDS_PW_MOVEV,

	ID_ROTATE,
	IDS_PW_ROTATELONG,
	IDS_PW_ROTATE_UVW,

	ID_SCALE,
	IDS_PW_SCALELONG,
	IDS_PW_SCALE_UVW,

	ID_SCALEH,
	IDS_PW_SCALEH,
	IDS_PW_SCALEH,

	ID_SCALEV,
	IDS_PW_SCALEV,
	IDS_PW_SCALEV,

	ID_MIRRORH,
	IDS_PW_MIRRORH,
	IDS_PW_MIRRORH,

	ID_MIRRORV,
	IDS_PW_MIRRORV,
	IDS_PW_MIRRORV,

	ID_INCSEL,
	IDS_PW_INCSELLONG,
	IDS_PW_INC_UVW,

	ID_DECSEL,
	IDS_PW_DECSELLONG,
	IDS_PW_DEC_UVW,

	ID_BREAK,
	IDS_PW_BREAK,
	IDS_TH_VERTBREAK,

	ID_WELD,
	IDS_PW_WELDLONG,
	IDS_PW_WELD,

	ID_WELD_SELECTED,
	IDS_PW_WELDSELECTEDLONG,
	IDS_PW_WELDSELECTED,

	ID_UPDATEMAP,
	IDS_RB_UPDATE,
	IDS_RB_UPDATE,

	ID_OPTIONS,
	IDS_RB_PROP,
	IDS_RB_PROP,

	ID_LOCK,
	IDS_PW_LOCKSELECTED,
	IDS_PW_LOCKSELECTED,

	ID_HIDE,
	IDS_PW_HIDE_SELECTED,
	IDS_PW_HIDE,

	ID_UNHIDE,
	IDS_PW_UNHIDEALL,
	IDS_PW_UNHIDEALL,


	ID_FREEZE,
	IDS_PW_FREEZE_SELECTED,
	IDS_PW_FREEZE,

	ID_UNFREEZE,
	IDS_PW_UNFREEZEALL,
	IDS_PW_UNFREEZE,

	ID_FILTERSELECTED,
	IDS_PW_FACEFILTER,
	IDS_PW_FACEFILTER,

	ID_PAN,
	IDS_RB_PAN,
	IDS_RB_PAN,

	ID_ZOOM,
	IDS_RB_ZOOM,
	IDS_RB_ZOOM,

	ID_ZOOMREGION,
	IDS_RB_ZOOMREG,
	IDS_RB_ZOOMREG,

	ID_FIT,
	IDS_RB_ZOOMEXT,
	IDS_RB_ZOOMEXT,

	ID_FITSELECTED,
	IDS_RB_ZOOMSELEXT,
	IDS_RB_ZOOMSELEXT,

	ID_SNAP,
	IDS_PW_SNAP,
	IDS_PW_SNAP,

	ID_DETACH,
	IDS_PW_DETACH,
	IDS_PW_DETACH,

	ID_FLIPH,
	IDS_PW_FLIPH,
	IDS_PW_FLIPH,

	ID_FLIPV,
	IDS_PW_FLIPV,
	IDS_PW_FLIPV,

	ID_FACETOVERTEX,
	IDS_PW_FACETOVERTEX,
	IDS_PW_FACETOVERTEX,

	ID_ZOOMTOGIZMO,
	IDS_PW_ZOOMTOGIZMO,
	IDS_PW_ZOOMTOGIZMO,

	ID_GETFACESELFROMSTACK,
	IDS_PW_GETFACESELFROMSTACK,
	IDS_PW_GETFACESELFROMSTACK,

	ID_SHOWMAP,
	IDS_RB_SHOWMAP,
	IDS_RB_SHOWMAP,

	ID_STITCH,
	IDS_PW_STITCH,
	IDS_PW_STITCH,

	ID_STITCHDIALOG,
	IDS_PW_STITCHDIALOG,
	IDS_PW_STITCHDIALOG,

	ID_FLATTENMAP,
	IDS_PW_FLATTENMAP,
	IDS_PW_FLATTENMAP,

	ID_FLATTENMAPDIALOG,
	IDS_PW_FLATTENMAPDIALOG,
	IDS_PW_FLATTENMAPDIALOG,

	ID_NORMALMAP,
	IDS_PW_NORMALMAP,
	IDS_PW_NORMALMAP,

	ID_NORMALMAPDIALOG,
	IDS_PW_NORMALMAPDIALOG,
	IDS_PW_NORMALMAPDIALOG,

	ID_UNFOLDMAP,
	IDS_PW_UNFOLDMAP,
	IDS_PW_UNFOLDMAP,

	ID_UNFOLDMAPDIALOG,
	IDS_PW_UNFOLDMAPDIALOG,
	IDS_PW_UNFOLDMAPDIALOG,


	ID_COPY,
	IDS_PW_COPY,
	IDS_PW_COPY,

	ID_PASTE,
	IDS_PW_PASTE,
	IDS_PW_PASTE,

	ID_PASTEINSTANCE,
	IDS_PW_PASTEINSTANCE,
	IDS_PW_PASTEINSTANCE,

	ID_LIMITSOFTSEL,
	IDS_PW_LIMITSOFTSEL,
	IDS_PW_LIMITSOFTSEL,

	ID_LIMITSOFTSEL1,
	IDS_PW_LIMITSOFTSEL1,
	IDS_PW_LIMITSOFTSEL1,

	ID_LIMITSOFTSEL2,
	IDS_PW_LIMITSOFTSEL2,
	IDS_PW_LIMITSOFTSEL2,

	ID_LIMITSOFTSEL3,
	IDS_PW_LIMITSOFTSEL3,
	IDS_PW_LIMITSOFTSEL3,

	ID_LIMITSOFTSEL4,
	IDS_PW_LIMITSOFTSEL4,
	IDS_PW_LIMITSOFTSEL4,

	ID_LIMITSOFTSEL5,
	IDS_PW_LIMITSOFTSEL5,
	IDS_PW_LIMITSOFTSEL5,

	ID_LIMITSOFTSEL6,
	IDS_PW_LIMITSOFTSEL6,
	IDS_PW_LIMITSOFTSEL6,

	ID_LIMITSOFTSEL7,
	IDS_PW_LIMITSOFTSEL7,
	IDS_PW_LIMITSOFTSEL7,

	ID_LIMITSOFTSEL8,
	IDS_PW_LIMITSOFTSEL8,
	IDS_PW_LIMITSOFTSEL8,

	ID_GEOMELEMMODE,
	IDS_PW_GEOMELEMMODE,
	IDS_PW_GEOMELEMMODE,

	ID_ELEMENTMODE,
	IDS_PW_ELEMENTMODE,
	IDS_PW_ELEMENTMODE,

	ID_IGNOREBACKFACE,
	IDS_PW_IGNOREBACKFACES,
	IDS_PW_IGNOREBACKFACES,

	ID_PLANARMODE,
	IDS_PW_PLANARMODE,
	IDS_PW_PLANARMODE,

	ID_GEOMEXPANDFACESEL,
	IDS_PW_GEOMEXPANDFACESEL,
	IDS_PW_GEOMEXPANDFACESEL,

	ID_GEOMCONTRACTFACESEL,
	IDS_PW_GEOMCONTRACTFACESEL,
	IDS_PW_GEOMCONTRACTFACESEL,

	ID_SHOWVERTCONNECT,
	IDS_PW_SHOWVERTCONNECT,
	IDS_PW_SHOWVERTCONNECT,

	ID_TV_VERTMODE,
	IDS_PW_TV_VERTMODE,
	IDS_PW_TV_VERTMODE,

	ID_TV_EDGEMODE,
	IDS_PW_TV_EDGEMODE,
	IDS_PW_TV_EDGEMODE,

	ID_TV_FACEMODE,
	IDS_PW_TV_FACEMODE,
	IDS_PW_TV_FACEMODE,

	ID_PACK,
	IDS_PW_PACK,
	IDS_PW_PACK,

	ID_PACKDIALOG,
	IDS_PW_PACKDIALOG,
	IDS_PW_PACKDIALOG,


	ID_UVEDGEMODE,
	IDS_PW_UVEDGEMODE,
	IDS_PW_UVEDGEMODE,

	ID_OPENEDGEMODE,
	IDS_PW_OPENEDGEMODE,
	IDS_PW_OPENEDGEMODE,

	ID_UVEDGESELECT,
	IDS_PW_UVEDGESELECT,
	IDS_PW_UVEDGESELECT,

	ID_OPENEDGESELECT,
	IDS_PW_OPENEDGESELECT,
	IDS_PW_OPENEDGESELECT,

	ID_VERTTOEDGESELECT,
	IDS_PW_VERTTOEDGESELECT,
	IDS_PW_VERTTOEDGESELECT,

	ID_VERTTOFACESELECT,
	IDS_PW_VERTTOFACESELECT,
	IDS_PW_VERTTOFACESELECT,


	ID_EDGETOVERTSELECT,
	IDS_PW_EDGETOVERTSELECT,
	IDS_PW_EDGETOVERTSELECT,

	ID_EDGETOFACESELECT,
	IDS_PW_EDGETOFACESELECT,
	IDS_PW_EDGETOFACESELECT,


	ID_FACETOVERTSELECT,
	IDS_PW_FACETOVERTSELECT,
	IDS_PW_FACETOVERTSELECT,

	ID_FACETOEDGESELECT,
	IDS_PW_FACETOEDGESELECT,
	IDS_PW_FACETOEDGESELECT,

	ID_DISPLAYHIDDENEDGES,
	IDS_PW_DISPLAYHIDDENEDGES,
	IDS_PW_DISPLAYHIDDENEDGES,

	ID_FREEFORMMODE,
	IDS_PW_FREEFORMMODE,
	IDS_PW_FREEFORMMODE,

	ID_SNAPCENTER,
	IDS_PW_SNAPCENTER,
	IDS_PW_SNAPCENTER,

	ID_SNAPLOWERLEFT,
	IDS_PW_SNAPLOWERLEFT,
	IDS_PW_SNAPLOWERLEFT,

	ID_SNAPLOWERRIGHT,
	IDS_PW_SNAPLOWERRIGHT,
	IDS_PW_SNAPLOWERRIGHT,

	ID_SNAPLOWERCENTER,
	IDS_PW_SNAPLOWERCENTER,
	IDS_PW_SNAPLOWERCENTER,

	ID_SNAPUPPERLEFT,
	IDS_PW_SNAPUPPERLEFT,
	IDS_PW_SNAPUPPERLEFT,

	ID_SNAPUPPERRIGHT,
	IDS_PW_SNAPUPPERRIGHT,
	IDS_PW_SNAPUPPERRIGHT,

	ID_SNAPUPPERCENTER,
	IDS_PW_SNAPUPPERCENTER,
	IDS_PW_SNAPUPPERCENTER,

	ID_SNAPRIGHTCENTER,
	IDS_PW_SNAPRIGHTCENTER,
	IDS_PW_SNAPRIGHTCENTER,

	ID_SNAPLEFTCENTER,
	IDS_PW_SNAPLEFTCENTER,
	IDS_PW_SNAPLEFTCENTER,

	ID_SKETCHREVERSE,
	IDS_PW_SKETCHREVERSE,
	IDS_PW_SKETCHREVERSE,

	ID_SKETCHDIALOG,
	IDS_PW_SKETCHDIALOG,
	IDS_PW_SKETCHDIALOG,


	ID_RESETPIVOTONSEL,
	IDS_PW_RESETPIVOTONSEL,
	IDS_PW_RESETPIVOTONSEL,

	ID_SKETCH,
	IDS_PW_SKETCH,
	IDS_PW_SKETCH,

/*	ID_SHOWHIDDENEDGES,
	IDS_PW_SHOWHIDDENEDGES,
	IDS_PW_SHOWHIDDENEDGES,
*/
	ID_POLYGONMODE,
	IDS_PW_POLYGONMODE,
	IDS_PW_POLYGONMODE,

	ID_POLYGONSELECT,
	IDS_PW_POLYGONSELECT,
	IDS_PW_POLYGONSELECT,

	ID_ALLOWSELECTIONINSIDEGIZMO,
	IDS_PW_ALLOWSELECTIONINSIDEGIZMO,
	IDS_PW_ALLOWSELECTIONINSIDEGIZMO,

	ID_SAVEASDEFAULT,
	IDS_PW_SAVEASDEFAULT,
	IDS_PW_SAVEASDEFAULT,

	ID_LOADDEFAULT,
	IDS_PW_LOADDEFAULT,
	IDS_PW_LOADDEFAULT,

	ID_SHOWSHARED,
	IDS_PW_SHOWSHARED,
	IDS_PW_SHOWSHARED,

	ID_ALWAYSEDIT,
	IDS_PW_ALWAYSEDIT,
	IDS_PW_ALWAYSEDIT,

	ID_SYNCSELMODE,
	IDS_PW_SYNCSELECTIONMODE,
	IDS_PW_SYNCSELECTIONMODE,

	ID_SYNCTVSELECTION,
	IDS_PW_SYNCTVSELECTION,
	IDS_PW_SYNCTVSELECTION,

	ID_SYNCGEOMSELECTION,
	IDS_PW_SYNCGEOMSELECTION,
	IDS_PW_SYNCGEOMSELECTION,

	ID_SHOWOPENEDGES,
	IDS_PW_SHOWOPENEDGES,
	IDS_PW_SHOWOPENEDGES,

	ID_BRIGHTCENTERTILE,
	IDS_PW_BRIGHTCENTERTILE,
	IDS_PW_BRIGHTCENTERTILE,

	ID_BLENDTOBACK,
	IDS_PW_BLENDTOBACK,
	IDS_PW_BLENDTOBACK,

	ID_PAINTSELECTMODE,
	IDS_PW_PAINTSELECT,
	IDS_PW_PAINTSELECT,
	
	ID_PAINTSELECTINC,
	IDS_PW_PAINTSELECTINC,
	IDS_PW_PAINTSELECTINC,

	ID_PAINTSELECTDEC,
	IDS_PW_PAINTSELECTDEC,
	IDS_PW_PAINTSELECTDEC,

	ID_GRIDSNAP,
	IDS_PW_GRIDSNAP,
	IDS_PW_GRIDSNAP,

	ID_GRIDVISIBLE,
	IDS_PW_GRIDVISIBLE,
	IDS_PW_GRIDVISIBLE,

	ID_PREVENTREFLATTENING,
	IDS_PW_PREVENTREFLATTENING,
	IDS_PW_PREVENTREFLATTENING,

	ID_RELAX,
	IDS_PW_RELAX,
	IDS_PW_RELAX,

	ID_RELAXDIALOG,
	IDS_PW_RELAXDIALOG,
	IDS_PW_RELAXDIALOG,


};







class UnwrapClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE); 
	const TCHAR *	ClassName() {return UNWRAP_NAME;}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return UNWRAP_CLASSID;}
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFSURFACE);}

	const TCHAR*	InternalName() { return _T("UVWUnwrap"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	//You only need to add the action stuff to one Class Desc
	int             NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i) { return GetActions(); }

	ActionTable* GetActions()
			{
		    TSTR name = GetString(IDS_RB_UNWRAPMOD);
		    ActionTable* pTab;
		    pTab = new ActionTable(kUnwrapActions, kUnwrapContext, name);        

			int numOps = NumElements(spActions)/3;
			UnwrapAction *wtActions = NULL;
			int ct = 0;
			for (int i =0; i < numOps; i++)
				{
				wtActions = new UnwrapAction();
				int id, ids1, ids2;
				id = spActions[ct++];
				ids1 = spActions[ct++];
				ids2 = spActions[ct++];

				wtActions->Init(id,GetString(ids1),GetString(ids2),
							    GetString(IDS_RB_UNWRAPMOD), GetString(IDS_RB_UNWRAPMOD)  );
				pTab->AppendOperation(wtActions);
				}

			GetCOREInterface()->GetActionManager()->RegisterActionContext(kUnwrapContext, name.data());
			return pTab;
			}	};






#endif // __UWNRAP__H
