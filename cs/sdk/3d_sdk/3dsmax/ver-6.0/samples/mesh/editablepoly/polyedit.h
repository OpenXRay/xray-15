/**********************************************************************
 *<
	FILE: PolyEdit.h

	DESCRIPTION:   Editable Polygon Mesh Object

	CREATED BY: Steve Anderson

	HISTORY: created March 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __POLYOBJED__
#define __POLYOBJED__

#include "iparamm2.h"
#include "sbmtlapi.h"
#include "istdplug.h"
#include "nsclip.h"
#include "iFnPub.h"
#include "iEPoly.h"
#include "stdmat.h"             

#define DEF_FALLOFF 20.0f


// Table converts editable poly selection levels to MNMesh ones.
static int meshSelLevel[] = { MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE,
								MNM_SL_EDGE, MNM_SL_FACE, MNM_SL_FACE };
// Table converts editable poly selection levels to display flags.
static DWORD epLevelToDispFlags[] = { 0, MNDISP_VERTTICKS|MNDISP_SELVERTS,
		MNDISP_SELEDGES, MNDISP_SELEDGES, MNDISP_SELFACES,
		MNDISP_SELFACES };

#define DEF_PICKBOX_SIZE	4

// ID's for toolbar
#define IDC_SELVERTEX	0x4015
#define IDC_SELEDGE	0x4016
#define IDC_SELBORDER	0x4017
#define IDC_SELFACE		0x4018
#define IDC_SELELEMENT	0x4019

// Parameter block 2 enumerations:
// One parameter block:
enum { ep_block };

// Several rollouts:
enum { ep_select, ep_softsel, ep_geom, ep_geom_vertex, ep_geom_edge,
	ep_geom_border, ep_geom_face, ep_geom_element, ep_subdivision,
	ep_displacement, ep_vertex, ep_face,
	ep_advanced_displacement, ep_settings };

class CreateVertCMode;
class CreateEdgeCMode;
class CreateFaceCMode;
class AttachPickMode;
class ShapePickMode;		
class DivideEdgeCMode;
class DivideFaceCMode;
class ExtrudeCMode;
class ExtrudeVECMode;		
class ChamferCMode;
class BevelCMode;
class InsetCMode;			
class OutlineCMode;
class CutCMode;				
class QuickSliceCMode;		
class WeldCMode;
class LiftFromEdgeCMode;	
class PickLiftEdgeCMode;	
class EditTriCMode;
class SingleRefMakerPolyNode;         
class SingleRefMakerPolyMtl;   


#define CID_CREATEVERT	CID_USER+0x1000
#define CID_CREATEEDGE  CID_USER+0x1001
#define CID_CREATEFACE  CID_USER+0x1002
#define CID_DIVIDEEDGE	CID_USER+0x1018
#define CID_DIVIDEFACE	CID_USER+0x1019
#define CID_EXTRUDE		CID_USER+0x1020
#define CID_EXTRUDE_VERTEX_OR_EDGE CID_USER+0x104A	
#define CID_POLYCHAMFER		CID_USER+0x1028
#define CID_BEVEL			CID_USER+0x102C
#define CID_INSET			CID_USER+0x102D	
#define CID_OUTLINE_FACE	CID_USER+0x102E
#define CID_CUT				CID_USER+0x1038	
#define CID_QUICKSLICE		CID_USER+0x1050	
#define CID_WELD			CID_USER+0x1040
#define CID_SLIDE_EDGES		CID_USER+0x1054	
#define CID_LIFT_FROM_EDGE	CID_USER+0x1058	
#define CID_PICK_LIFT_EDGE	CID_USER+0x105A	
#define CID_EDITTRI			CID_USER+0x1048

#define MAX_MATID	0xffff

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2

// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_EDGE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_EDGE, CLIP_FACE, CLIP_FACE };
static int hitLevel[] = {0, SUBHIT_MNVERTS,SUBHIT_MNEDGES,
					SUBHIT_MNEDGES|SUBHIT_OPENONLY, 
					SUBHIT_MNFACES, SUBHIT_MNFACES};

// Flags:
// Disp Result keeps track of "Show End Result" button for this Editable Mesh.
#define EPOLY_DISP_RESULT 0x0001
// In Render indicates if we're in the middle of a render.
#define EPOLY_IN_RENDER 0x0002
// Aborted indicates that the user escaped out of a MeshSmooth subdivision.
#define EPOLY_ABORTED 0x0004
// Force indicates that the user wishes to force a recomputation of the subdivision result.
#define EPOLY_FORCE 0x0008

// References:
#define EPOLY_PBLOCK 0
#define EPOLY_MASTER_CONTROL_REF  1
#define EPOLY_VERT_BASE_REF 2

class EPolyShortcutCB;
class TempMoveRestore;

class RefmsgKillCounter {
private:
	friend class KillRefmsg;
	LONG	counter;

public:
	RefmsgKillCounter() : counter(-1) {}

	bool DistributeRefmsg() { return counter < 0; }
};

class KillRefmsg {
private:
	LONG&	counter;

public:
	KillRefmsg(RefmsgKillCounter& c) : counter(c.counter) { ++counter; }
	~KillRefmsg() { --counter; }
};

class EditPolyObject : public PolyObject, public EventUser, public ISubMtlAPI,
			public EPoly, public AttachMatDlgUser, public IMeshSelect, 
			public IMeshSelectData {
friend class TransformPlaneRestore;
friend class EPolyBackspaceUser;
friend class EPolyActionCB;
friend class CollapseDeadVertsRestore;
private:

	// Preview data:
	bool mPreviewOn, mPreviewValid, mPreviewDrag, mPreviewSuspend;
	int mPreviewOperation;
	MNMesh mPreviewMesh;
	MNTempData *mpPreviewTemp;

	// Repeat-last data:
	int mLastOperation;

	// For hit testing other than the current SO level:
	DWORD mHitLevelOverride, mDispLevelOverride;
	bool mForceIgnoreBackfacing, mSuspendConstraints;

	// Internal methods to support preview:
	void ApplyMeshOp (MNMesh & mesh, MNTempData *temp, int operation);
	MNTempData *PreviewTempData ();
	void PreviewTempDataFree ();
	void UpdatePreviewMesh ();

	// Class vars
	static EditPolyObject *editObj;
	static IParamMap2 *pSelect, *pSoftsel, *pGeom, *pSubdivision, *pSurface, *pDisplacement;
	static IParamMap2 *pSubobjControls;
	static bool rsSelect, rsSoftsel, rsGeom, rsSubdivision, rsSurface, rsDisplacement;
	static bool rsSubobjControls;
	static bool inExtrude, inBevel, inChamfer, inInset;

	// handle for all "Settings" dialogs:

	static IParamMap2 *pOperationSettings;

	// Command modes
	static MoveModBoxCMode *moveMode;
	static RotateModBoxCMode *rotMode;
	static UScaleModBoxCMode *uscaleMode;
	static NUScaleModBoxCMode *nuscaleMode;
	static SquashModBoxCMode *squashMode;
	static SelectModBoxCMode *selectMode;

	static CreateVertCMode *createVertMode;
	static CreateEdgeCMode * createEdgeMode;
	static CreateFaceCMode* createFaceMode;
	static AttachPickMode* attachPickMode;
	static ShapePickMode* shapePickMode;
	static DivideEdgeCMode* divideEdgeMode;
	static DivideFaceCMode *divideFaceMode;
	static ExtrudeCMode *extrudeMode;
	static ExtrudeVECMode *extrudeVEMode;
	static ChamferCMode *chamferMode;
	static BevelCMode* bevelMode;
	static InsetCMode * insetMode;
	static OutlineCMode *outlineMode;
	static CutCMode *cutMode;
	static QuickSliceCMode *quickSliceMode;
	static WeldCMode *weldMode;
	static LiftFromEdgeCMode *liftFromEdgeMode;
	static PickLiftEdgeCMode *pickLiftEdgeMode;
	static EditTriCMode *editTriMode;

	static int attachMat;
	static BOOL condenseMat;
	static Quat sliceRot;
	static Point3 sliceCenter;
	static float sliceSize;
	static bool sliceMode;
	static EPolyBackspaceUser backspacer;


	static EPolyActionCB * mpEPolyActions;

	IParamBlock2 *pblock;

	RefmsgKillCounter	killRefmsg;

	MNTempData *tempData;

	static TempMoveRestore *tempMove;

	GenericNamedSelSetList selSet[3];
	MasterPointControl	*masterCont;		// Master track controller
	Tab<Control*> cont;
	int selLevel;
	Interval arValid, localGeomValid, displacementSettingsValid;
	DWORD ePolyFlags;
	BitArray vsel, esel, fsel;
	bool sliceInitialized;
	MNMesh subdivResult;
	Interval subdivValid;
	MNMeshSelectionConverter mSelConv;

public:
	static Interface *ip;
	SingleRefMakerPolyNode* noderef;        
	SingleRefMakerPolyMtl* mtlref;          


	EditPolyObject();
	~EditPolyObject();
	void ResetClassParams (BOOL fileReset);

	// Flag methods.
	void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) ePolyFlags |= fl; else ePolyFlags &= ~fl; }
	void ClearFlag (DWORD fl) { ePolyFlags &= (~fl); }
	bool GetFlag (DWORD fl) { return (ePolyFlags&fl) ? TRUE : FALSE; }

	MNMesh *GetMeshPtr () { return &mm; }
	MNTempData *TempData ();
	void InvalidateTempData (PartID parts=PART_TOPO|PART_GEOM);

	// Temp (drag) move methods:
	void DragMoveInit ();
	void DragMoveRestore ();
	void DragMove (Tab<Point3> & delta, EPoly *pEPoly, TimeValue t);
	void DragMap (int mapChannel, Tab<UVVert> & mapDelta, EPoly *pEPoly, TimeValue t);
	void DragMoveAccept (TimeValue t);
	void DragMoveClear ();
	void ApplyDelta (Tab<Point3> & delta, EPoly *pEPoly, TimeValue t);
	void ApplyMapDelta (int mapChannel, Tab<UVVert> & mapDelta, EPoly *pEPoly, TimeValue t);

	// Animatable methods
	void DeleteThis() {delete this;}
	Class_ID ClassID() {return EPOLYOBJ_CLASS_ID;}
	void GetClassName(TSTR& s) {s = GetString(IDS_EDITABLE_POLY);}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
	int NumSubs() { return 2; }		// only the pblock and the master point controller
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum) {return subNum;}
	BOOL SelectSubAnim(int subNum);

	// PBlock2 methods:
	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock (int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID (short id) { return (pblock->ID() == id) ? pblock : NULL; }

	// Reference methods
	int NumRefs() {return EPOLY_VERT_BASE_REF+cont.Count();}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	void CreateContArray();
	void SynchContArray(int newNV);
	void AllocContArray(int count);
	void ReplaceContArray(Tab<Control *> &nc);
	void DeletePointConts(BitArray &set);
	void PlugControllersSel(TimeValue t,BitArray &set);
	BOOL PlugControl(TimeValue t,int i);
	void SetPtCont(int i, Control *c);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	void RescaleWorldUnits (float f);

	void UpdateGeometry (TimeValue time);
	void UpdateSoftSelection (TimeValue time);
	void UpdateSubdivResult (TimeValue time);
	void UpdateEverything (TimeValue time);

	// BaseObject methods
	ObjectState Eval(TimeValue time);
	//Object versions:
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box);
	// Gizmo versions:
	bool ShowGizmoConditions ();
	bool ShowGizmo ();
	void UpdateDisplayFlags ();
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
	void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box, ModContext *mc);

	void GetLocalBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box);
	DWORD CurrentHitLevel (int *selByVert=NULL);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	void SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection (int selLevel);
	void SelectAll (int selLevel);
	void InvertSelection (int selLevel);
	void EpfnGrowSelection (int meshSelLevel);
	void EpfnShrinkSelection (int meshSelLevel);
	int EpfnConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll=false);
	void EpfnSelectBorder ();
	void EpfnSelectElement ();
	void EpfnSelectEdgeRing ();
	void EpfnSelectEdgeLoop ();
	void InvalidateDistanceCache ();
	void InvalidateSoftSelectionCache ();
	void ActivateSubobjSel (int level, XFormModes& modes );		
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ShowEndResultChanged (BOOL showEndResult);

	// Object methods		
	TCHAR *GetObjectName() { return GetString(IDS_EDITABLE_POLY);}
	BOOL IsSubClassOf(Class_ID classID);
	Object *CollapseObject(){return this;}
	Object *ConvertToType (TimeValue t, Class_ID obtype);
	int RenderBegin(TimeValue t, ULONG flags);
	int RenderEnd(TimeValue t);
	Mesh* GetRenderMesh(TimeValue t, INode *inode, View &view,  BOOL& needDelete);
	Interval ChannelValidity (TimeValue t, int nchan);

	// Named subobject selections:
	BOOL SupportsNamedSubSels() { return true; }
	void ActivateSubSelSet(TSTR &setName);
	void NewSetFromCurSel(TSTR &setName);
	void RemoveSubSelSet(TSTR &setName);
	void SetupNamedSelDropDown();
	void UpdateNamedSelDropDown ();
	int NumNamedSelSets();
	TSTR GetNamedSelSetName(int i);
	void SetNamedSelSetName(int i,TSTR &newName);
	void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
	void EpfnNamedSelectionCopy (TSTR setName);
	void NamedSelectionCopy (int index);
	void EpfnNamedSelectionPaste (bool useDlgToRename);
	BOOL GetUniqueSetName(TSTR &name, bool useDlg);
	int SelectNamedSet();
	void IncreaseNamedSetSize (int nsl, int oldsize, int increase);
	void DeleteNamedSetArray (int nsl, BitArray & del);
	BitArray *GetNamedSelSet (int nsl, int setIndex);
	void SetNamedSelSet (int nsl, int setIndex, BitArray *newSet);

	// Reference methods
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	// Local methods
	void ExitAllCommandModes (bool exSlice=true, bool exStandardModes=true);
	void ExitPickLiftEdgeMode ();
	GenericNamedSelSetList &GetSelSet(int namedSelLevel);
	int GetSubobjectLevel();
	void SetSubobjectLevel(int level);
	void UpdateUIToSelectionLevel (int oldSelLevel);

	// Operations -- in polyedops.cpp
	// Shift-cloning:
	void CloneSelSubComponents(TimeValue t);
	void AcceptCloneSelSubComponents(TimeValue t);
	// Transform stuff:
	void Transform (int sl, TimeValue t, Matrix3& partm, Matrix3 tmAxis, BOOL localOrigin, Matrix3 xfrm, int type);
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
	void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void TransformStart(TimeValue t);
	void TransformHoldingFinish (TimeValue t);
	void TransformFinish(TimeValue t);
	void TransformCancel(TimeValue t);

	// Useful many places
	void CollapseDeadVerts ();
	void CollapseDeadEdges ();
	void CollapseDeadFaces ();
	void CollapseDeadStructs ();

	// Select panel ops:
	bool EpfnHideMesh (MNMesh & mesh, int msl, DWORD flag=MN_SEL);
	bool EpfnHide (int msl, DWORD flag=MN_SEL);
	bool EpfnUnhideAllMesh (MNMesh & mesh, int msl);
	bool EpfnUnhideAll (int msl);

	// Topological & Geometric ops from the Edit Geometry panel:
	int EpfnCreateVertex(Point3 pt, bool pt_local=false, bool select=true);
	int EpfnCreateEdge (int v1, int v2, bool select=true);
	int EpfnCreateFace (int *v, int deg, bool select=true);
	int EpfnCreateFace2 (Tab<int> *vertices, bool select=true);
	bool EpfnCapHoles (int msl=MNM_SL_EDGE, DWORD targetFlags=MN_SEL);
	bool EpfnDelete (int msl, DWORD delFlag=MN_SEL, bool delIsoVerts=false);
	bool EpfnRemove (int msl, DWORD delFlag=MN_SEL);
	bool EpfnDeleteIsoVerts ();
	bool EpfnDeleteIsoMapVerts ();
	bool DeleteVerts (DWORD flag=MN_SEL);
	bool RemoveVerts (DWORD flag=MN_SEL);
	bool RemoveEdges (DWORD flag=MN_SEL);
	bool DeleteFaces (DWORD flag=MN_SEL, bool delIsoVerts=false);
	bool EpfnExtrudeOpenEdges (DWORD flag=MN_SEL);
	
	// from AttachMatDlgUser
	int GetAttachMat() { return attachMat; }
	void SetAttachMat(int value) { attachMat = value; }
	BOOL GetCondenseMat() { return condenseMat; }
	void SetCondenseMat(BOOL sw) { condenseMat = sw; }

	void EpfnAttach (INode *node, bool & canUndo, INode *myNode, TimeValue t);
	void EpfnAttach (INode *node, INode *myNode, TimeValue t);
	void EpfnMultiAttach (INodeTab &nodeTab, INode *myNode, TimeValue t);
	bool EpfnDetachToElement (int msl, DWORD flag, bool keepOriginal);
	bool EpfnDetachToObject (TSTR name, int msl, DWORD flag, bool keepOriginal, INode *myNode, TimeValue t);
	bool EpfnSplitEdges (DWORD flag=MN_SEL);
	bool EpfnBreakVerts (DWORD flag=MN_SEL);
	int EpfnDivideFace (int face, Tab<float> &bary, bool select=true);
	int EpfnDivideEdge (int edge, float prop, bool select=true);
	bool EpfnCollapse (int msl, DWORD flag);

	// Extrude/Bevel/Chamfer methods:

	// Extrude/etc by the amounts described in the parameter block:
	bool EpfnExtrudeMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag);
	bool EpfnBevelMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag);
	bool EpfnInsetMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag);
	bool EpfnChamferMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag);
	bool EpfnOutlineMesh (MNMesh & mesh, MNTempData *temp, DWORD flag);
	bool EpfnOutline (DWORD flag);

	void EpfnExtrudeFaces (float amount, DWORD flag, TimeValue t);
	void EpfnBevelFaces (float height, float outline, DWORD flag, TimeValue t);
	void EpfnChamferVertices (float amount, TimeValue t);
	void EpfnChamferEdges (float amount, TimeValue t);

	// Perform isolated topological change:
	bool DoExtrusion(int msl, DWORD flag, bool isBevel);
	bool DoExtrusionMesh(MNMesh & mesh, MNTempData *temp, int msl, DWORD flag, bool isBevel);
	bool DoChamfer (int msl, DWORD flag=MN_SEL);
	bool DoChamferMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag=MN_SEL);
	bool DoInset (int msl, DWORD flag=MN_SEL);
	bool DoInsetMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag=MN_SEL);

	// Begin for topo change, Drag to try various geometric changes, and End to wrap up & create Undo objects.
	void EpfnBeginExtrude (int msl, DWORD flag, TimeValue t);
	void EpfnEndExtrude (bool accept, TimeValue t);
	void EpfnDragExtrude (float amount, TimeValue t);
	void EpfnBeginBevel (int msl, DWORD flag, bool doExtrude, TimeValue t);
	void EpfnEndBevel (bool accept, TimeValue t);
	void EpfnDragBevel (float outline, float height, TimeValue t);
	void EpfnBeginChamfer (int msl, TimeValue t);
	void EpfnEndChamfer (bool accept, TimeValue t);
	void EpfnDragChamfer (float amount, TimeValue t);
	void EpfnBeginInset (int msl, DWORD flag, TimeValue t);
	void EpfnEndInset (bool accept, TimeValue t);
	void EpfnDragInset (float amount, TimeValue t);

	// Various operations:
	bool EpfnCreateShape (TSTR name, bool smooth, INode *myNode, DWORD edgeFlag=MN_SEL);
	bool EpfnMakePlanar (int msl, DWORD flag=MN_SEL, TimeValue t=0);
	bool EpfnMoveToPlane (Point3 planeNormal, float planeOffset, int msl, DWORD flag=MN_SEL, TimeValue t=0);
	bool EpfnAlignToGrid (int msl, DWORD flag=MN_SEL);
	bool EpfnAlignToView (int msl, DWORD flag=MN_SEL);
	bool EpfnMeshSmooth (int msl, DWORD flag=MN_SEL);
	bool EpfnMeshSmoothMesh (MNMesh & mesh, int msl, DWORD flag=MN_SEL);
	bool EpfnTessellate (int msl, DWORD flag=MN_SEL);
	bool EpfnTessellateMesh (MNMesh & mesh, int msl, DWORD flag=MN_SEL);
	bool EpfnSlice (Point3 planeNormal, Point3 planeCenter, bool flaggedFacesOnly=false, DWORD faceFlags=MN_SEL);
	bool EpfnSliceMesh (MNMesh & mesh, int msl, DWORD faceFlags=MN_SEL);
	bool EpfnInSlicePlaneMode () { return sliceMode; }
	int EpfnCutMesh (MNMesh & mesh);
	// Following three are obselete - use EpfnCutMesh with the pblock cut parameters.
	// (Retained for SDK compatibility.)
	int EpfnCutVertex (int startv, Point3 destination, Point3 projDir);
	int EpfnCutEdge (int e1, float prop1, int e2, float prop2, Point3 projDir);
	int EpfnCutFace (int f1, Point3 p1, Point3 p2, Point3 projDir);
	bool EpfnWeldVerts (int vert1, int vert2, Point3 destination);
	bool EpfnWeldEdges (int edge1, int edge2);
	bool EpfnWeldMeshVerts (MNMesh & mesh, DWORD flag);
	bool EpfnWeldFlaggedVerts (DWORD flag);
	bool EpfnWeldMeshEdges (MNMesh & mesh, DWORD flag);
	bool EpfnWeldFlaggedEdges (DWORD flag);
	bool EpfnConnectMeshEdges (MNMesh & mesh, DWORD edgeFlag);
	bool EpfnConnectEdges (DWORD edgeFlag);
	bool EpfnConnectMeshVertices (MNMesh & mesh, DWORD vertexFlag);
	bool EpfnConnectVertices (DWORD vertexFlag);
	bool EpfnExtrudeAlongSplineMesh (MNMesh & mesh, MNTempData *pTemp, DWORD faceFlag);
	bool EpfnExtrudeAlongSpline (DWORD faceFlag);
	bool EpfnLiftFromEdgeMesh (MNMesh & mesh, MNTempData *pTemp, DWORD faceFlag);
	bool EpfnLiftFromEdge (DWORD faceFlag);
	void EpfnForceSubdivision ();
	void EpfnSetDiagonal (int face, int corner1, int corner2);
	bool EpfnRetriangulate (DWORD flag=MN_SEL);
	bool EpfnFlipNormals (DWORD flag=MN_SEL);
	void EpfnSelectByMat (int index, bool clear, TimeValue t);
	void EpfnSelectBySmoothGroup (DWORD bits, BOOL clear, TimeValue t);
	void EpfnAutoSmooth (TimeValue t);
	void EpfnToggleShadedFaces ();

	// Slice plane accessors:
	void EpResetSlicePlane ();
	void EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float *planeSize=NULL);
	void EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float planeSize);
	bool EpGetSliceInitialized () { return sliceInitialized; }

	// Vertex/Edge Data accessors:
	float GetVertexDataValue (int channel, int *numSel, bool *uniform, DWORD vertFlags, TimeValue t);
	float GetEdgeDataValue (int channel, int *numSel, bool *uniform, DWORD edgeFlags, TimeValue t);
	void SetVertexDataValue (int channel, float w, DWORD vertFlags, TimeValue t);
	void SetEdgeDataValue (int channel, float w, DWORD edgeFlags, TimeValue t);
	void ResetVertexData (int channel);
	void ResetEdgeData (int channel);
	void BeginPerDataModify (int mnSelLevel, int channel);
	bool InPerDataModify ();
	void EndPerDataModify (bool success);
	void UpdatePerDataDisplay (TimeValue t, int mnSelLev, int channel);

	// EPoly color accessors:
	Color GetVertexColor (bool *uniform=NULL, int *num=NULL, int mapChannel=0, DWORD flag=MN_SEL, TimeValue=0);
	void SetVertexColor (Color clr, int mapChannel=0, DWORD flag=MN_SEL, TimeValue t=0);
	Color GetFaceColor (bool *uniform=NULL, int *num=NULL, int mapChannel=0, DWORD flag=MN_SEL, TimeValue t=0);
	void SetFaceColor (Color clr, int mapChannel=0, DWORD flag=MN_SEL, TimeValue t=0);
	void BeginVertexColorModify (int mapChannel=0);
	bool InVertexColorModify ();
	void EndVertexColorModify (bool success);

	// Support for above.
	void InitVertColors (int mapChannel=0);

	void EpfnSelectVertByColor (BOOL add, BOOL sub, int mapChannel=0, TimeValue t=0);

	// Face Property Accessors:
	int GetMatIndex (bool *determined);
	void SetMatIndex (int index, DWORD flag=MN_SEL);
	void GetSmoothingGroups (DWORD faceFlag, DWORD *anyFaces, DWORD *allFaces=NULL);
	// Note: needed "bool" version locally, but need to maintain support for EPoly interface.
	// "bool" version returns true if any faces were selected; false otherwise.
	void SetSmoothBits (DWORD bits, DWORD bitmask, DWORD flag) { LocalSetSmoothBits (bits, bitmask, flag); }
	bool LocalSetSmoothBits (DWORD bits, DWORD bitmask, DWORD flag);

	// Psuedo-command-mode, mixed in with real ones in polymodes.cpp:
	void EnterSliceMode ();
	void ExitSliceMode ();

	void GetSlicePlaneBoundingBox (Box3 & box, Matrix3 *tm=NULL);
	void DisplaySlicePlane (GraphicsWindow *gw);

	// UI code -- polyedui.cpp
	void InvalidateDialogElement (int elem);
	HWND GetDlgHandle(int dlgID);
	float GetPolyFaceThresh();
	void InvalidateSurfaceUI();
	void InvalidateSubdivisionUI();
	void InvalidateNumberSelected ();
	void UpdateCageCheckboxEnable ();
	void SetNumSelLabel();
	BOOL SplitSharedVertCol();

	// Copy displacement parameters from pblock to polyobject:
	void SetDisplacementParams ();
	void SetDisplacementParams (TimeValue t);
	// Copy displacement parameters from polyobject to pblock:
	void UpdateDisplacementParams ();
	// Engage a displacement approximation preset:
	void UseDisplacementPreset (int presetNumber);

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// IMeshSelect methods:
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// IMeshSelectData methods:
	BitArray GetVertSel() { mm.getVertexSel(vsel); return vsel; }
	BitArray GetFaceSel() { mm.getFaceSel(fsel); return fsel; }
	BitArray GetEdgeSel() { if (mm.GetFlag (MN_MESH_FILLED_IN)) mm.getEdgeSel(esel); return esel; }
	bool EpGetVerticesByFlag (BitArray & vset, DWORD flags, DWORD fmask=0x0) { return mm.getVerticesByFlag (vset, flags, fmask); }
	bool EpGetEdgesByFlag (BitArray & eset, DWORD flags, DWORD fmask=0x0) { return mm.getEdgesByFlag (eset, flags, fmask); }
	bool EpGetFacesByFlag (BitArray & fset, DWORD flags, DWORD fmask=0x0) { return mm.getFacesByFlag (fset, flags, fmask); }
	BitArray GetSel (int nsl);
	void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void EpSetVertexFlags (BitArray &vset, DWORD flags, DWORD fmask=0x0, bool undoable=true);
	void EpSetEdgeFlags (BitArray &eset, DWORD flags, DWORD fmask = 0x0, bool undoable=true);
	void EpSetFaceFlags (BitArray &fset, DWORD flags, DWORD fmask = 0x0, bool undoable=true);
	void SetSel (int nsl, BitArray & set, IMeshSelect *imod, TimeValue t);
	GenericNamedSelSetList & GetNamedVertSelList () { return selSet[NS_VERTEX]; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return selSet[NS_EDGE]; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return selSet[NS_FACE]; }

	BitArray *EpfnGetSelection (int msl);
	void EpfnSetSelection (int msl, BitArray *newSel);

	// Read-only access to mesh information for Maxscript.
	int EpfnGetNumVertices () { return mm.numv; }
	Point3 EpfnGetVertex (int vertIndex);
	int EpfnGetVertexFaceCount (int vertIndex);
	int EpfnGetVertexFace (int vertIndex, int whichFace);
	int EpfnGetVertexEdgeCount (int vertIndex);
	int EpfnGetVertexEdge (int vertIndex, int whichEdge);

	int EpfnGetNumEdges () { return mm.nume; }
	int EpfnGetEdgeVertex (int edgeIndex, int end);
	int EpfnGetEdgeFace (int edgeIndex, int side);

	int EpfnGetNumFaces() { return mm.numf; }
	int EpfnGetFaceDegree (int faceIndex);
	int EpfnGetFaceVertex (int faceIndex, int corner);
	int EpfnGetFaceEdge (int faceIndex, int side);
	int EpfnGetFaceMaterial (int faceIndex);
	DWORD EpfnGetFaceSmoothingGroup (int faceIndex);

	int EpfnGetNumMapChannels () { return mm.numm; }
	bool EpfnGetMapChannelActive (int mapChannel);
	int EpfnGetNumMapVertices (int mapChannel);
	UVVert EpfnGetMapVertex (int mapChannel, int vertIndex);
	int EpfnGetMapFaceVertex (int mapChannel, int faceIndex, int corner);

	// EPoly methods:
	void LocalDataChanged (DWORD parts);
	void RefreshScreen ();
	void EnterCommandMode (int mode);
	CommandMode *getCommandMode (int mode);
	IParamBlock2 *getParamBlock () { return pblock; }
	void EpActionToggleCommandMode (int mode);
	void EpActionEnterPickMode (int mode);
	void EpActionButtonOp (int opcode);
	void EpActionExitCommandModes () { ExitAllCommandModes (); }
	void MoveSelection(int msl, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void RotateSelection(int msl, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
	void ScaleSelection(int msl, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	bool Editing () { return (ip && (editObj==this)) ? TRUE : FALSE; }
	int GetEPolySelLevel() { return selLevel; }
	int GetMNSelLevel() { return meshSelLevel[selLevel]; }
	void SetEPolySelLevel (int sl) { if (ip) ip->SetSubObjectLevel (sl); else selLevel=sl; }
	int EpfnPropagateComponentFlags (int mnSlTo, DWORD flTo, int mnSlFrom, DWORD flFrom, bool ampersand=FALSE, bool set=TRUE, bool undoable=FALSE);

	// ISubMtlAPI methods:
	void*	GetInterface(ULONG id);
	MtlID	GetNextAvailMtlID(ModContext* mc);
	BOOL	HasFaceSelection(ModContext* mc);
	void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
	int		GetSelFaceUniqueMtlID(ModContext* mc);
	int		GetSelFaceAnyMtlID(ModContext* mc);
	int		GetMaxMtlID(ModContext* mc);

	// Access to EPoly interface:
	BaseInterface *GetInterface (Interface_ID id);

	// EventUser methods:
	void Notify() { EpActionButtonOp (epop_delete); /*delete key was pressed*/}

	// New preview interface:

	void EpPreviewClear ();
	void EpPreviewBegin (int opcode);
	void EpPreviewCancel ();
	void EpPreviewAccept ();
	void EpPreviewInvalidate ();
	MNMesh *EpPreviewMesh () { return &mPreviewMesh; }
	bool EpPreviewOn ();
	void EpPreviewSetDragging (bool drag) { mPreviewDrag = drag; }
	bool EpPreviewGetDragging () { return mPreviewDrag; }
	void EpPreviewSetSuspend (bool suspend) { mPreviewSuspend = suspend; }
	bool EpPreviewGetSuspend () { return mPreviewSuspend; }

	// Manage floating dialog: includes code to remember, use previous location.

	bool EpfnPopupDialog (int popupOperation);
	void EpfnClosePopupDialog ();
	void ClearOperationSettings () { pOperationSettings = NULL; }

	// Used in command modes to change which level we're hit testing on.
	void SetHitLevelOverride (DWORD hlo) { mHitLevelOverride = hlo; }
	void ClearHitLevelOverride () { mHitLevelOverride = 0x0; }
	DWORD GetHitLevelOverride () { return mHitLevelOverride; }

	void SetDisplayLevelOverride (DWORD dlo) { mDispLevelOverride = dlo; UpdateDisplayFlags (); }
	void ClearDisplayLevelOverride () { mDispLevelOverride = 0x0; UpdateDisplayFlags (); }
	DWORD GetDisplayLevelOverride () { return mDispLevelOverride; }

	// Also used by command modes where needed:
	void ForceIgnoreBackfacing (bool force) { mForceIgnoreBackfacing = force; }
	bool GetForceIgnoreBackfacing () { return mForceIgnoreBackfacing; }
	void SuspendContraints (bool suspend) { mSuspendConstraints = suspend; }
	bool GetSuspendConstraints () { return mSuspendConstraints; }
	Point3 *ConstrainDelta (MNMesh & mesh, TimeValue t, Tab<Point3> & delta);

	// New repeat-last methods:

	void EpfnRepeatLastOperation ();
	int EpGetLastOperation () { return mLastOperation; }
	void EpSetLastOperation (int op);

	// Override PolyObject method:
	BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);
};

// Backspace event user:
class EPolyBackspaceUser : public EventUser {
	EditPolyObject *mpEditPoly;
public:
	void Notify();
	void SetEPoly (EditPolyObject *e) { mpEditPoly=e; }
};

// --- Command Modes & Mouse Procs -------------------------------

// Virtual mouse procs: these are useful for multiple actual procs.
class PickEdgeMouseProc : public MouseCallBack {
protected:
	EPoly *mpEPoly;
	IObjParam *ip;

public:
	PickEdgeMouseProc(EditPolyObject* e, IObjParam *i) : mpEPoly(e), ip(i) { }
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt, float *prop, Point3 *snapPoint);
	virtual int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
	virtual bool EdgePick(int edge, float prop)=0;
};

class PickFaceMouseProc : public MouseCallBack {
protected:
	EPoly *mpEPoly;
	IObjParam *ip;
	int face;	// Face we picked
	Tab<float> bary;	// Barycentric coords - size of face degree, with up to 3 nonzero values summing to 1

public:
	PickFaceMouseProc(EditPolyObject* e, IObjParam *i) : mpEPoly(e), ip(i) { }
	HitRecord *HitTestFaces(IPoint2 &m, ViewExp *vpt);
	void ProjectHitToFace (IPoint2 &m, ViewExp *vpt, HitRecord *hr, Point3 *snapPoint);
	virtual int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void FacePick()=0;
};

// Used in both create edge and edit triangulation:
class ConnectVertsMouseProc : public MouseCallBack {
protected:
	EPoly *mpEPoly;
	IObjParam *ip;
	int v1, v2;
	Tab<int> neighbors;
	IPoint2 m1, lastm;

public:
	ConnectVertsMouseProc (EPoly *e, IObjParam *i) : mpEPoly(e), ip(i) { }
	HitRecord *HitTestVertices (IPoint2 & m, ViewExp *vpt);
	void DrawDiag (HWND hWnd, const IPoint2 & m);
	void SetV1 (int vv);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void VertConnect () {}
};

// Actual procs & command modes:

class CreateVertMouseProc : public MouseCallBack {
private:
	EPoly *mpEPoly;
	IObjParam *ip;
public:
	CreateVertMouseProc (EPoly* e, IObjParam *i) : mpEPoly(e), ip(i) { }
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	CreateVertMouseProc proc;
	EditPolyObject* mpEditPoly;

public:
	CreateVertCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATEVERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class CreateEdgeMouseProc : public ConnectVertsMouseProc {
public:
	CreateEdgeMouseProc (EditPolyObject *e, IObjParam *i) : ConnectVertsMouseProc (e,i) {}
	void VertConnect ();
};

class CreateEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	CreateEdgeMouseProc proc;
	EditPolyObject* mpEditPoly;

public:
	CreateEdgeCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATEEDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class CreateFaceMouseProc : public MouseCallBack {
friend class CreateFaceCMode;
	EPoly *mpEPoly;
	IObjParam *ip;
	Tab<int> vts;
	Tab<IPoint2> mpts;
	Matrix3 mWorld2obj;
	IPoint2 mMousePosition;
	int pt;

public:
	CreateFaceMouseProc(EditPolyObject* e, IObjParam *i) : mpEPoly(e), ip(i), pt(0) { }
	void DrawCreatingFace (HWND hWnd);
	BOOL HitTestVerts(IPoint2 m, ViewExp *vpt,int &v);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
	void Backspace ();
};

class CreateFaceCMode : public CommandMode {
	ChangeFGObject fgProc;
	EditPolyObject *mpEditPoly;
	CreateFaceMouseProc proc;

public:
	CreateFaceCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATEFACE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=999999; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();

	void Display (GraphicsWindow *gw);
	void Backspace () { proc.Backspace(); }
};

class AttachPickMode : public PickModeCallback, public PickNodeCallback {
	EditPolyObject* mpEditPoly;
	IObjParam *ip;

public:
	AttachPickMode() : mpEditPoly(NULL), ip(NULL) { }
	void SetPolyObject (EditPolyObject *e, IObjParam *i) { mpEditPoly=e; ip=i; }
	void ClearPolyObject () { mpEditPoly=NULL; ip=NULL; }
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);		

	BOOL Filter(INode *node);
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return true;}
	PickNodeCallback *GetFilter() {return this;}
};

class AttachHitByName : public HitByNameDlgCallback {
	EditPolyObject *mpEditPoly;
	bool inProc;

public:
	AttachHitByName (EditPolyObject *e) : mpEditPoly(e), inProc(false) { }
	TCHAR *dialogTitle() { return GetString(IDS_ATTACH_LIST); }
	TCHAR *buttonText() { return GetString(IDS_ATTACH); }
	int filter(INode *node);	
	void proc(INodeTab &nodeTab);	
};

class ShapePickMode : public PickModeCallback, public PickNodeCallback {
	EditPolyObject* mpEditPoly;
	IObjParam *ip;

public:
	ShapePickMode() : mpEditPoly(NULL), ip(NULL) { }
	void SetPolyObject (EditPolyObject *e, IObjParam *i) { mpEditPoly=e; ip=i; }
	void ClearPolyObject () { mpEditPoly=NULL; ip=NULL; }
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);		

	BOOL Filter(INode *node);
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	PickNodeCallback *GetFilter() {return this;}
};

class DivideEdgeProc : public PickEdgeMouseProc {
public:
	DivideEdgeProc(EditPolyObject* e, IObjParam *i) : PickEdgeMouseProc(e,i) {}
	bool EdgePick(int edge, float prop);
};

class DivideEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideEdgeProc proc;
	EditPolyObject* mpEditPoly;

public:
	DivideEdgeCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_DIVIDEEDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class DivideFaceProc : public PickFaceMouseProc {
public:
	DivideFaceProc(EditPolyObject* e, IObjParam *i) : PickFaceMouseProc(e,i) {}
	void FacePick();
};

class DivideFaceCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideFaceProc proc;
	EditPolyObject* mpEditPoly;

public:
	DivideFaceCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_DIVIDEFACE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class ExtrudeProc : public MouseCallBack {
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 om;

public:
	ExtrudeProc (EditPolyObject *e, IObjParam *i) : mpEditPoly(e), ip(i) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	ExtrudeSelectionProcessor(ExtrudeProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ExtrudeSelectionProcessor mouseProc;
	ExtrudeProc eproc;
	EditPolyObject* mpEditPoly;

public:
	ExtrudeCMode(EditPolyObject* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class ChamferMouseProc : public MouseCallBack {
private:
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 om;
public:
	ChamferMouseProc (EditPolyObject* e, IObjParam *i) : mpEditPoly(e), ip(i) { }
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ChamferSelectionProcessor : public GenModSelectionProcessor {
	EditPolyObject *mpEditPoly;
protected:
	HCURSOR GetTransformCursor();
public:
	ChamferSelectionProcessor(ChamferMouseProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i), mpEditPoly(e) { }
};

class ChamferCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ChamferSelectionProcessor mouseProc;
	ChamferMouseProc eproc;
	EditPolyObject* mpEditPoly;

public:
	ChamferCMode (EditPolyObject* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_POLYCHAMFER; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class BevelMouseProc : public MouseCallBack {
private:
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 m0, m1;
	bool m0set, m1set;
	float height;
public:
	BevelMouseProc(EditPolyObject* e, IObjParam *i) : mpEditPoly(e), ip(i), m0set(false), m1set(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	BevelSelectionProcessor(BevelMouseProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	BevelSelectionProcessor mouseProc;
	BevelMouseProc eproc;
	EditPolyObject* mpEditPoly;

public:
	BevelCMode(EditPolyObject* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class InsetMouseProc : public MouseCallBack {
private:
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 m0;
	bool m0set;
public:
	InsetMouseProc(EditPolyObject* e, IObjParam *i) : mpEditPoly(e), ip(i), m0set(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class InsetSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	InsetSelectionProcessor(InsetMouseProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class InsetCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	InsetSelectionProcessor mouseProc;
	InsetMouseProc eproc;
	EditPolyObject* mpEditPoly;

public:
	InsetCMode(EditPolyObject* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_INSET; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class OutlineMouseProc : public MouseCallBack {
private:
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 m0;
	bool m0set;
public:
	OutlineMouseProc(EditPolyObject* e, IObjParam *i) : mpEditPoly(e), ip(i), m0set(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class OutlineSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	OutlineSelectionProcessor(OutlineMouseProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class OutlineCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	OutlineSelectionProcessor mouseProc;
	OutlineMouseProc eproc;
	EditPolyObject* mpEditPoly;

public:
	OutlineCMode(EditPolyObject* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_OUTLINE_FACE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class ExtrudeVEMouseProc : public MouseCallBack {
private:
	EditPolyObject *mpEditPoly;
	Interface *ip;
	IPoint2 m0;
	bool m0set;

public:
	ExtrudeVEMouseProc(EditPolyObject* e, IObjParam *i) : mpEditPoly(e), ip(i), m0set(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class ExtrudeVESelectionProcessor : public GenModSelectionProcessor {
private:
	EPoly *mpEPoly;
protected:
	HCURSOR GetTransformCursor();
public:
	ExtrudeVESelectionProcessor(ExtrudeVEMouseProc *mc, EditPolyObject *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i), mpEPoly(e) { }
};

class ExtrudeVECMode : public CommandMode {
private:
	ChangeFGObject fgProc;	
	ExtrudeVESelectionProcessor mouseProc;
	ExtrudeVEMouseProc eproc;
	EditPolyObject *mpEditPoly;

public:
	ExtrudeVECMode(EditPolyObject *e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EXTRUDE_VERTEX_OR_EDGE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class CutProc : public MouseCallBack {
friend class CutCMode;
private:
	EPoly *mpEPoly;
	EditPolyObject *mpEditPoly;
	IObjParam *ip;
	int startLevel, startIndex;
	Matrix3 mObj2world;
	IPoint2 mMouse1, mMouse2;
	int mLastHitLevel, mLastHitIndex;
	Point3 mLastHitPoint, mLastHitDirection;

	// In the following, "completeAnalysis" means that we want to fill in the information
	// about exactly which component we hit, and what point in object space we hit.
	// (So we fill in mLastHitIndex and mLastHitPoint.)
	// (We also fill in the obj2world transform of our node here.)
	// If completeAnalysis is false, all we care about is knowing what SO level
	// we're mousing over (so we (always) fill in mLastHitLevel).
	HitRecord *HitTestVerts(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord *HitTestFaces(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord *HitTestAll (IPoint2 &m, ViewExp *vpt, int flags, bool completeAnalysis);
	void DrawCutter (HWND hwnd);

public:
	CutProc (EPoly* e, IObjParam *i) : mpEPoly(e), ip(i), startIndex(-1) { mpEditPoly = (EditPolyObject *)mpEPoly; }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class CutCMode : public CommandMode {
private:
	ChangeFGObject fgProc;	
	CutProc proc;
	EditPolyObject* mpEditPoly;

public:
	CutCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CUT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=999; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class QuickSliceProc : public MouseCallBack {
friend class QuickSliceCMode;
private:
	EPoly *mpEPoly;
	EditPolyObject *mpEditPoly;
	IObjParam *mpInterface;
	IPoint2 mMouse1, mMouse2;
	Point3 mLocal1, mLocal2;
	Point3 mZDir;
	Matrix3 mWorld2obj;
	bool mSlicing;

public:
	QuickSliceProc(EPoly *e, IObjParam *i) : mpEPoly(e), mpInterface(i), mSlicing(false) { mpEditPoly = (EditPolyObject *) mpEPoly; }
	void DrawSlicer (HWND hWnd);
	void ComputeSliceParams (bool center=false);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class QuickSliceCMode : public CommandMode {
private:
	ChangeFGObject fgProc;	
	QuickSliceProc proc;
	EditPolyObject* mpEditPoly;

public:
	QuickSliceCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_QUICKSLICE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=3; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
	bool Slicing () { return proc.mSlicing; }
};

class WeldMouseProc : public MouseCallBack {
	EditPolyObject *mpEditPoly;
	IObjParam *ip;
	int targ1;
	IPoint2 m1, oldm2;

public:
	WeldMouseProc(EditPolyObject *e, IObjParam *i) : mpEditPoly(e), ip(i), targ1(-1) { }
	void DrawWeldLine (HWND hWnd, IPoint2 &m);
	bool CanWeldEdges (int e1, int e2);
	bool CanWeldVertices (int v1, int v2);
	HitRecord *HitTest (IPoint2 &m, ViewExp *vpt);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	void Reset() { targ1=-1; }
};

class WeldCMode : public CommandMode {
	ChangeFGObject fgProc;
	WeldMouseProc mproc;
	EditPolyObject* mpEditPoly;

public:
	WeldCMode(EditPolyObject* mod, IObjParam *i) :
	  fgProc(mod), mproc(mod,i), mpEditPoly(mod) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_WELD; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &mproc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) {return oldMode->ChangeFGProc() != &fgProc;}
	void EnterMode();
	void ExitMode();
};

class LiftFromEdgeProc : public MouseCallBack {
	EPoly *mpEPoly;
	IObjParam *ip;
	IPoint2 firstClick;
	bool edgeFound;

public:
	LiftFromEdgeProc (EPoly *e, IObjParam *i) : mpEPoly(e), ip(i), edgeFound(false) { }
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
	void Reset () { edgeFound = false; }
};

class LiftFromEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;	
	LiftFromEdgeProc proc;
	EditPolyObject* mpEditPoly;

public:
	LiftFromEdgeCMode (EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_LIFT_FROM_EDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class PickLiftEdgeProc : public PickEdgeMouseProc {
public:
	PickLiftEdgeProc(EditPolyObject* e, IObjParam *i) : PickEdgeMouseProc(e,i) {}
	bool EdgePick(int edge, float prop);
};

class PickLiftEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	PickLiftEdgeProc proc;
	EditPolyObject* mpEditPoly;

public:
	PickLiftEdgeCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_PICK_LIFT_EDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class EditTriProc : public ConnectVertsMouseProc {
public:
	EditTriProc(EditPolyObject* e, IObjParam *i) : ConnectVertsMouseProc(e,i) { }
	void VertConnect ();
};

class EditTriCMode : public CommandMode {
	ChangeFGObject fgProc;
	EditTriProc proc;
	EditPolyObject* mpEditPoly;

public:
	EditTriCMode(EditPolyObject* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EDITTRI; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

// --- Restore objects ---------------------------------------------

// Not really a restore object, just used in some drag moves.
class TempMoveRestore {
friend EditPolyObject;
	Tab<Point3> init;
	Tab<UVVert> *mapInit;
	BitArray active;

public:
	TempMoveRestore (EditPolyObject *e);
	~TempMoveRestore ();
	void Restore (EditPolyObject *e);
};

class ComponentFlagRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int sl;
	Tab<DWORD> undo, redo;

public:
	ComponentFlagRestore (EditPolyObject *e, int selLevel);
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Component flag change restore")); }
	int Size () { return sizeof(void *) + sizeof(int) + 2*sizeof(Tab<DWORD>); }
};

class CueDragRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;

public:
	CueDragRestore (EditPolyObject *e) : mpEditPoly(e) { }
	void Restore(int isUndo) { mpEditPoly->DragMoveRestore(); }
	void Redo() { }
	TSTR Description() {return TSTR(_T("Cue drag move Restore"));}
	int Size() { return sizeof(void *); }
};

class PerDataRestore;

// Made to track "local" topological changes to an MNMesh.
// (No collapseDead's allowed between Before and After.)
// (Creations are ok.)
class TopoChangeRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	MNMesh start;
	Tab<int> vertID, edgeID, faceID;
	Tab<MNVert> vertsBefore, vertsAfter;
	Tab<int> *vfacBefore, *vfacAfter;
	Tab<int> *vedgBefore, *vedgAfter;
	Tab<MNEdge> edgesBefore, edgesAfter;
	Tab<MNFace> facesBefore, facesAfter;
	int numvBefore, numvAfter, numeBefore, numeAfter, numfBefore, numfAfter;
	Tab<int> numMvBefore, numMvAfter;
	Tab<int> *mapVertID, *mapFaceID;
	Tab<UVVert> *mapVertsBefore, *mapVertsAfter;
	Tab<MNMapFace> *mapFacesBefore, *mapFacesAfter;
	Tab<float> *vertexDataBefore, *vertexDataAfter;
	Tab<float> *edgeDataBefore, *edgeDataAfter;
	int vertexDataCount, edgeDataCount;

public:
	TopoChangeRestore (EditPolyObject *editObj);
	~TopoChangeRestore ();
	void Before () { start = mpEditPoly->mm; }
	void After ();
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("EPoly Topology Change Restore")); }
	int Size () { return sizeof(void *) + 9*sizeof (Tab<int>) + sizeof (MNMesh) + 4*sizeof(Tab<int> *) + 6*sizeof(int); }
};

class MapChangeRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int mMapChannel;
	Tab<UVVert> preVerts;
	Tab<MNMapFace> preFaces;
	Tab<int> vertID, faceID;
	Tab<UVVert> vbefore, vafter;
	Tab<MNMapFace> fbefore, fafter;
	int numvBefore, numvAfter;
	int numfBefore, numfAfter;
	DWORD mapFlagsBefore, mapFlagsAfter;
	bool mAfterCalled;

public:
	MapChangeRestore (EditPolyObject *editObj, int mapchan);
	~MapChangeRestore ();
	bool After ();
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Map change restore")); }
	int Size () { return sizeof(void *) + 4*sizeof(int) + 8*sizeof(Tab<int>) + 2*sizeof(DWORD); }
};

// MapVertRestore is used when only the values of the map vertices change.
// (Not their number, or any map faces.)
class MapVertRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int mMapChannel;
	// Used to store the original map vertices before After()
	Tab<UVVert> preVerts;
	// Used to store only the changed map vertices after After();
	Tab<int> vertID;
	Tab<UVVert> vbefore, vafter;
	bool mAfterCalled;

public:
	MapVertRestore (EditPolyObject *editObj, int mapchan);
	bool After ();
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Map vertex restore")); }
	int Size () { return sizeof(void *) + sizeof(int) + sizeof(Tab<int>) + 3*sizeof(Tab<UVVert>); }
	UVVert *GetUndoPoints () { return preVerts.Count() ? preVerts.Addr(0) : NULL; }
	int NumUndoPoints () { return preVerts.Count(); }
};

// Following intended only for operations that purely create, not those that modify existing topology.
class CreateOnlyRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int ovnum, oenum, ofnum;
	Tab<int> omvnum, omfnum;
	Tab<MNVert> nverts;
	Tab<int> *nvfac;
	Tab<int> *nvedg;
	Tab<MNEdge> nedges;
	Tab<MNFace> nfaces;
	Tab<MNMapFace> *nmfaces;
	Tab<UVVert> *nmverts;
	BitArray omapsUsed, nmapsUsed;
	bool afterCalled;

public:
	CreateOnlyRestore (EditPolyObject *mpEditPoly);
	~CreateOnlyRestore ();
	void After ();
	void Restore (BOOL isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Creation restore")); }
	int Size () { return sizeof(void *) + 3*sizeof(int) + 3*sizeof(Tab<MNVert>); }
};

class MeshVertRestore : public RestoreObj {
	Tab<Point3> undo, redo;
	EditPolyObject *mpEditPoly;

public:
	MeshVertRestore(EditPolyObject *mpEditPoly);
	void Restore(int isUndo);
	void Redo();
	void EndHold() { mpEditPoly->ClearAFlag(A_HELD); }
	TSTR Description() {return TSTR(_T("Mesh Geometry Change"));}
	Point3 *GetUndoPoints() { return undo.Count() ? undo.Addr(0) : NULL; }
};

class MtlIDRestore : public RestoreObj {	
	Tab<MtlID> undo, redo;
	EditPolyObject *mpEditPoly;		

public:
	MtlIDRestore (EditPolyObject *e);
	void After ();
	void Restore(int isUndo);
	void Redo();
	void EndHold() { mpEditPoly->ClearAFlag(A_HELD);}
	TSTR Description() {return TSTR(_T("Mesh Face Mat"));}
};

class SmGroupRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	Tab<DWORD> osmg, nsmg;

public:
	SmGroupRestore (EditPolyObject *e);
	void After ();
	void Restore (int isUndo);
	void Redo ();
	TSTR Description() {return TSTR(_T("MatID Restore"));}
};

class CollapseDeadVertsRestore : public RestoreObj {	
	Tab<MNVert> undo;
	Tab<int> *undovedg, *undovfac;
	Tab<int> positions;
	int startNum;
	EditPolyObject *mpEditPoly;		
	Tab<Control*> ucont, rcont;

public:
	CollapseDeadVertsRestore (EditPolyObject *e);
	void Restore(int isUndo);
	void Redo();
	void MyDebugPrint ();
	TSTR Description() {return TSTR(_T("Collapse Dead Verts Restore"));}
	int Size() { return sizeof(Tab<int>) + sizeof(Tab<MNVert>) + sizeof(void *) + 2*sizeof(Tab<Control*>); }
};

class CollapseDeadEdgesRestore : public RestoreObj {	
	Tab<MNEdge> undo;
	Tab<int> positions;
	EditPolyObject *mpEditPoly;		

public:
	CollapseDeadEdgesRestore (EditPolyObject *e);
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("CollapseDeadEdgesRestore"));}
	int Size() { return sizeof(Tab<int>) + sizeof(Tab<MNEdge>) + sizeof(void *); }
};

class CollapseDeadFacesRestore : public RestoreObj {	
	Tab<MNFace> undo;
	Tab<int> positions;
	Tab<MNMapFace *> undoMap;
	EditPolyObject *mpEditPoly;		

public:
	CollapseDeadFacesRestore (EditPolyObject *e);
	~CollapseDeadFacesRestore ();
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Collapse Dead Faces Restore"));}
	int Size() { return sizeof(Tab<int>) + sizeof(Tab<MNFace>) + sizeof(Tab<MNMapFace *>) + sizeof(void *); }
};

class PerDataRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int msl, channel, num;
	Tab<float> oldData;
	Tab<float> newData;

public:
	PerDataRestore (EditPolyObject *e, int mnSelLev, int chan);
	void After ();
	void Restore (int isUndo);
	void Redo ();
	int Size () { return sizeof(EditPolyObject *) + 3*sizeof(int) + 2*sizeof(Tab<float>); }
	TSTR Description() { return _T("PerData Change Restore"); }
};

class InitVertColorRestore : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int mapChannel;

public:
	InitVertColorRestore (EditPolyObject *e, int mapChan) : mpEditPoly(e), mapChannel(mapChan) { }
	void Restore (int isUndo) { mpEditPoly->mm.M(mapChannel)->SetFlag (MN_DEAD); }
	void Redo () { mpEditPoly->InitVertColors (mapChannel); }
	TSTR Description () { return TSTR(_T("Initializing vertex colors")); }
	int Size () { return sizeof(void *); }
};

class AppendSetRestore : public RestoreObj {
	BitArray set;
	TSTR name;
	GenericNamedSelSetList *setList;
	EditPolyObject *mpEditPoly;
			
public:
	AppendSetRestore(GenericNamedSelSetList *sl,EditPolyObject *e) : setList(sl), mpEditPoly(e) { }
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Append Set"));}
};

class EPDeleteSetRestore : public RestoreObj {
	BitArray set;
	TSTR name;
	int index;
	GenericNamedSelSetList *setList;
	EditPolyObject *mpEditPoly;
			
public:
	EPDeleteSetRestore(TSTR nm, GenericNamedSelSetList *sl,EditPolyObject *e) :
		setList(sl), mpEditPoly(e), name(nm) { set  = *(sl->GetSet(nm)); }
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Delete Set"));}
};

class EPSetNameRestore : public RestoreObj {
	TSTR undo, redo;
	int index;
	GenericNamedSelSetList *setList;
	EditPolyObject *mpEditPoly;

public:
	EPSetNameRestore(int i,GenericNamedSelSetList *sl,EditPolyObject *e) : index(i), setList(sl), mpEditPoly(e) {
		undo = *setList->names[index];
	}
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Set Name"));}
};

class NamedSetChangeRestore : public RestoreObj {
	BitArray mUndo, mRedo;
	int mLevel, mIndex;
	EditPolyObject *mpEditPoly;

public:
	NamedSetChangeRestore (int level, int index, EditPolyObject *e);
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Named Selection Set Change")); }
};

class TransformPlaneRestore : public RestoreObj {
	Point3 oldSliceCenter, newSliceCenter;
	Quat oldSliceRot, newSliceRot;
	float oldSliceSize, newSliceSize;
	EditPolyObject *mpEditPoly;

public:
	TransformPlaneRestore (EditPolyObject *e) : mpEditPoly(e),
		oldSliceCenter(e->sliceCenter), oldSliceRot(e->sliceRot), oldSliceSize(e->sliceSize) { }
	void Restore(int isUndo);
	void Redo();
	int Size () { return 2*(sizeof(Point3) + sizeof(Quat) + sizeof(float)) + sizeof (EditPolyObject *); }
	TSTR Description () { return TSTR (_T("Slice Plane transform")); }
};

// PolyEdUI.cpp functions:
bool GetCreateShapeName (Interface *ip, TSTR &name, bool &ccSmooth);
bool GetDetachObjectName (Interface *ip, TSTR &name, bool &elem, bool &clone);
BOOL GetCloneObjectName (Interface *ip, TSTR &name);
//bool GetSelectByMaterialParams (Interface *ip, MtlID &matId, bool &clear);       //az 042503 - oblselete, no dialog popup anymore
bool GetSelectByMaterialParams (EditPolyObject *ep, MtlID &matId, bool &clear);
bool GetSelectBySmoothParams (Interface *ip, DWORD usedBits, DWORD &smBits, bool &clear);

// triops.cpp
bool CreateCurveFromMeshEdges (MNMesh & mesh, INode *node, Interface *ip, TSTR & name, bool curved, DWORD flag);
BOOL CALLBACK DispApproxDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Can we put this here safely?
class EditablePolyObjectClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() {return GetString(IDS_EDITABLE_POLY); }
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return EPOLYOBJ_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EDITABLE_OBJECTS);}
	const TCHAR*	InternalName() { return _T("EditablePolyMesh"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
#ifndef RENDER_VER
	int NumActionTables() { return 1; }
#else
	int NumActionTables() { return 0; }
#endif
	ActionTable*  GetActionTable (int i);
};

// Action table class for Editable Poly:

const ActionTableId kEPolyActionID = 0x160d7c17;

class EPolyActionCB : public ActionCallback {
	EditPolyObject *mpObj;
public:
	EPolyActionCB (EditPolyObject *o) : mpObj(o) { }
	BOOL ExecuteAction(int id);
};

void ConvertPathToFrenets (Spline3D *pSpline, Matrix3 & relativeTransform,
						   Tab<Matrix3> & tFrenets, int numSegs, bool align, float rotation);

//az -  042303  MultiMtl sub/mtl name support
void GetMtlIDList(Mtl *mtl, NumList& mtlIDList);
void GetEPolyMtlIDList(EditPolyObject *ep, NumList& mtlIDList);
INode* GetNode (EditPolyObject *ep);
BOOL SetupMtlSubNameCombo (HWND hWnd, EditPolyObject *ep);
void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin);
void ValidateUINameCombo (HWND hWnd, EditPolyObject *ep);

#define  SRMM_CLASS_ID 0x4e5057f3
class SingleRefMakerPolyMtl: public SingleRefMaker {
public:
	HWND hwnd;
	EditPolyObject *ep;
	RefTargetHandle rtarget;
	SingleRefMakerPolyMtl() {hwnd = NULL; ep = NULL; rtarget = NULL; }
	~SingleRefMakerPolyMtl() { 
		theHold.Suspend();         
		DeleteAllRefsFromMe();
		theHold.Resume();
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
			switch(message) {
			case REFMSG_CHANGE:
				if(hwnd) ValidateUINameCombo(hwnd, ep);
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMM_CLASS_ID; }
};


#define  SRMN_CLASS_ID 0x4e6168f3
class SingleRefMakerPolyNode: public SingleRefMaker {
public:
	HWND hwnd;
	EditPolyObject *ep;
	RefTargetHandle rtarget;
	SingleRefMakerPolyNode() {hwnd = NULL; ep = NULL; rtarget = NULL; }
	~SingleRefMakerPolyNode() { 
		theHold.Suspend();         
		DeleteAllRefsFromMe(); 
		theHold.Resume();
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
			switch(message) {
			case REFMSG_NODE_MATERIAL_CHANGED:
				ep->mtlref->SetRef(GetNode(ep)->GetMtl());
				if(hwnd) ValidateUINameCombo(hwnd, ep);
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMN_CLASS_ID; }
};


#endif //__POLYOBJED__
