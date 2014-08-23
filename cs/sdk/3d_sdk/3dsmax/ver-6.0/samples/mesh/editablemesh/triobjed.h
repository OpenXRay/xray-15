/**********************************************************************
 *<
	FILE: triobjed.h

	DESCRIPTION:   Editable Triangle Mesh Object

	CREATED BY: Rolf Berteig

	HISTORY: created 4 March 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#ifndef __TRIOBJED__
#define __TRIOBJED__

#include "sbmtlapi.h"
#include "MeshDLib.h"
#include "istdplug.h"
#include "nsclip.h"
#include "ActionTable.h"
#include  "stdmat.h"          

#define DEF_FALLOFF 20.0f

// Available selection levels
#define SL_OBJECT EM_SL_OBJECT  //0
#define SL_VERTEX EM_SL_VERTEX	//1
#define SL_EDGE EM_SL_EDGE	//2
#define SL_FACE EM_SL_FACE	//3
#define SL_POLY EM_SL_POLYGON	//4
#define SL_ELEMENT EM_SL_ELEMENT	//5

// Converts editable mesh selection levels to mesh ones.
static DWORD meshSelLevel[] = { MESH_OBJECT, MESH_VERTEX, MESH_EDGE, MESH_FACE, MESH_FACE, MESH_FACE };

#define DEF_PICKBOX_SIZE	4

// ID's for toolbar
#define IDC_SELVERTEX	0x4015
#define IDC_SELEDGE	0x4016
#define IDC_SELFACE	0x4017
#define IDC_SELPOLY		0x4018
#define IDC_SELELEMENT	0x4019

// Alignment types:
#define ALIGN_CONST	0
#define ALIGN_VIEW	1

class WeldVertCMode;
class CreateVertCMode;
class CreateFaceCMode;
class DivideEdgeCMode;
class TurnEdgeCMode;
class AttachPickMode;
class ExtrudeCMode;
class BevelCMode;
class ChamferCMode;
class FlipNormCMode;
class CutEdgeCMode;
class DivideFaceCMode;
class SingleRefMakerMeshNode;        
class SingleRefMakerMeshMtl;         

#define CID_EXTRUDE		CID_USER + 972
#define CID_CREATEVERT	CID_USER + 973
#define CID_OBJATTACH  CID_USER + 974
#define CID_BUILDFACE	CID_USER + 975
#define CID_DIVIDEEDGE	CID_USER + 976
#define CID_TURNEDGE	CID_USER + 977
#define CID_WELDVERT	CID_USER + 978
#define CID_DIVIDEFACE CID_USER + 979
#define CID_CUTEDGE    CID_USER + 980
#define CID_FLIPNORM	CID_USER + 981
#define CID_BEVEL    CID_USER + 982
#define CID_MCHAMFER CID_USER + 983	// "Mesh Chamfer" -- CID_CHAMFER was taken.

#define MAX_MATID	0xffff

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2
// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_FACE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_FACE, CLIP_FACE, CLIP_FACE };

// Flags:
// (Unlike Edit Mesh, all etFlags are permanent - no temporary ones - however we still start at 0x0100.)
// Disp Result keeps track of "Show End Result" button for this Editable Mesh.
#define ET_DISP_RESULT 0x0100

// References:
#define ET_MASTER_CONTROL_REF  0
#define ET_VERT_BASE_REF 1

class EMeshActionCB;
class TempMoveRestore;

class EditTriObject : public TriObject, public EventUser, public ISubMtlAPI, public IMeshSelect,
								public IMeshSelectData, public MeshDeltaUser, public MeshDeltaUserData,
								public AttachMatDlgUser {
	// Load reference version
	int loadRefVersion;
public:
	// Class vars
	// Window handles & an interface:
	static HWND hSel, hAR, hGeom, hSurf, hApprox;
	static Interface *ip;
	static EditTriObject *editObj;

	// Command modes
	static MoveModBoxCMode *moveMode;
	static RotateModBoxCMode *rotMode;
	static UScaleModBoxCMode *uscaleMode;
	static NUScaleModBoxCMode *nuscaleMode;
	static SquashModBoxCMode *squashMode;
	static SelectModBoxCMode *selectMode;
	static WeldVertCMode *weldVertMode;
	static CreateVertCMode *createVertMode;
	static CreateFaceCMode* createFaceMode;
	static TurnEdgeCMode* turnEdgeMode;
	static DivideEdgeCMode* divideEdgeMode;
	static DivideFaceCMode *divideFaceMode;
	static AttachPickMode* attachPickMode;
	static ExtrudeCMode* extrudeMode;
	static BevelCMode* bevelMode;
	static ChamferCMode *chamferMode;
	static FlipNormCMode* flipMode;
	static CutEdgeCMode * cutEdgeMode;
	SingleRefMakerMeshNode* noderef;   
	SingleRefMakerMeshMtl* mtlref;   

	static float normScale;
	static bool showFNormals, showVNormals;
	static BOOL selByVert, ignoreBackfaces;
	static BOOL inBuildFace, inCutEdge;
	static BOOL faceUIValid;
	static BOOL inExtrude, inBevel, inChamfer;
	static int extType;
	static BOOL ignoreVisEdge;
	static BOOL rsSel, rsAR, rsGeom, rsSurf, rsApprox;	// Rollup States (FALSE=Rolled-up.)
	static int pickBoxSize;
	static int weldBoxSize;
	static int attachMat;
	static BOOL condenseMat;
	static bool sliceMode, sliceSplit, cutRefine;
	static Quat sliceRot;
	static Point3 sliceCenter;
	static float sliceSize;

	// Cache for computing coord. systems
	// methods in tridata.cpp
	MeshTempData *tempData;
	MeshTempData *TempData ();
	void InvalidateTempData (PartID parts=PART_TOPO|PART_GEOM);

	static TempMoveRestore *tempMove;

	GenericNamedSelSetList selSet[3];
	MasterPointControl	*masterCont;		// Master track controller
	Tab<Control*> cont;
	int selLevel;
	float falloff, pinch, bubble;
	int affectRegion, arIgBack, useEdgeDist, edgeIts;
	Interval arValid;
	DWORD etFlags;

	EditTriObject();
	~EditTriObject();	// CCJ 3/9/99

	// Flag methods.
	void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) etFlags |= fl; else etFlags &= ~fl; }
	void ClearFlag (DWORD fl) { etFlags &= (~fl); }
	bool GetFlag (DWORD fl) { return (etFlags&fl) ? TRUE : FALSE; }

	// Temp (drag) move methods:
	void ClearSpecifiedNormals ();
	void DragMoveInit (bool doMaps=false);
	void DragMoveRestore ();
	void DragMove (MeshDelta & md, MeshDeltaUser *mdu);
	void DragMoveAccept (TimeValue t);
	void DragMoveClear ();

	// Animatable methods
	void DeleteThis() {delete this;}
	Class_ID ClassID() {return Class_ID(EDITTRIOBJ_CLASS_ID,0);}
	void GetClassName(TSTR& s) {s = GetString(IDS_SCA_BASE_MESH);}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
	int NumSubs() { return 1; }		// only the master.
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum) {return subNum;}
	BOOL SelectSubAnim(int subNum);

	// Reference methods
	int RemapRefOnLoad(int iref);
	int NumRefs() {return 1+cont.Count();}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	void CreateContArray();
	void SynchContArray(int newNV);
	void AllocContArray(int count);
	void ReplaceContArray(Tab<Control *> &nc);
	//BOOL BypassTreeView();
	void DeletePointConts(BitArray &set);
	BOOL PlugControl(TimeValue t,int i);
	void SetPtCont(int i, Control *c);
	void SetPointAnim(TimeValue t, int i, Point3 pt);
	BOOL CloneVertCont(int from, int to);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// BaseObject methods
	ObjectState Eval(TimeValue time);
	
	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	//Object versions:
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box);
	// Gizmo versions:
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
	void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box, ModContext *mc);

	void GetLocalBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	void SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection (int selLevel);
	void SelectAll (int selLevel);
	void InvertSelection (int selLevel);
	void InvalidateDistances ();
	void InvalidateAffectRegion ();
	void ActivateSubobjSel (int level, XFormModes& modes );		
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);		
	void ShowEndResultChanged (BOOL showEndResult);

	// Object methods		
	TCHAR *GetObjectName() { return GetString(IDS_SCA_BASE_MESH);}
	BOOL IsSubClassOf(Class_ID classID);

	// Named subobject selections:
	BOOL SupportsNamedSubSels() {return TRUE;}
	void ActivateSubSelSet(TSTR &setName);
	void NewSetFromCurSel(TSTR &setName);
	void RemoveSubSelSet(TSTR &setName);
	void SetupNamedSelDropDown();
	void UpdateNamedSelDropDown ();
	int NumNamedSelSets();
	TSTR GetNamedSelSetName(int i);
	void SetNamedSelSetName(int i,TSTR &newName);
	void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
	void NSCopy ();
	void NSPaste ();
	BOOL GetUniqueSetName(TSTR &name);
	int SelectNamedSet();
	void IncreaseNamedSetSize (int nsl, int oldsize, int increase);
	void DeleteNamedSetArray (int nsl, BitArray & del);

	// Reference methods
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	// Local methods
	void ExitAllCommandModes (bool exSlice=TRUE, bool exStandardModes=true);
	GenericNamedSelSetList &GetSelSet();
	int GetSubobjectLevel();
	void SetSubobjectLevel(int level);
	void RefreshSelType ();
	Object *CollapseObject ();

	// Operations -- in triops.cpp
	// Shift-cloning:
	void CloneSelSubComponents(TimeValue t);
	void AcceptCloneSelSubComponents(TimeValue t);
	// Transform stuff:
	void Transform(TimeValue t, Matrix3& partm, Matrix3 tmAxis, BOOL localOrigin, Matrix3 xfrm, int type);
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
	void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void TransformStart(TimeValue t);
	void TransformHoldingFinish (TimeValue t);
	void TransformFinish(TimeValue t);
	void TransformCancel(TimeValue t);

	// Selection panel operations:
	void HideSelectedVerts();
	void UnhideAllVerts();
	void HideSelectedFaces();
	void UnhideAllFaces();

	// Topological & Geometric ops from the Edit Geometry panel:
	DWORD CreateVertex(Point3 pt);
	bool CreateFace(int *v, int deg);
	void DeleteSelected();
	
	// from AttachMatDlgUser
	int GetAttachMat() { return attachMat; }
	void SetAttachMat(int value) { attachMat = value; }
	BOOL GetCondenseMat() { return condenseMat; }
	void SetCondenseMat(BOOL sw) { condenseMat = sw; }

	void Attach (INode *node, bool & canUndo);
	void MultiAttach (INodeTab &nodeTab);
	void Detach (TSTR &name,BOOL doFaces,BOOL del=TRUE,BOOL elem=FALSE);
	void BreakVerts();
	void DoExtrusion();
	void BeginExtrude (TimeValue t);
	void EndExtrude (TimeValue t,BOOL accept);
	void Extrude (TimeValue t, float amount);
	void BeginBevel (TimeValue t, BOOL doExtrude=FALSE);
	void EndBevel (TimeValue t,BOOL accept);
	void Bevel (TimeValue t, float outline, float height=0);
	void DoChamfer();
	void BeginChamfer (TimeValue t);
	void EndChamfer (TimeValue t,BOOL accept);
	void Chamfer (TimeValue t, float amount);
	void AlignTo (int alignType);
	void MakePlanar ();
	void Collapse ();
	void Tessellate (float tens,BOOL edge);
	void Explode (float thresh, BOOL objs, TSTR &name);
	void Slice ();
	BOOL WeldVerts(float thresh);
	void WeldVerts(Point3 pt);
	void RemoveIsoVerts();
	void SelectOpenEdges();

	// Vertex Surface operations:
	float GetWeight (TimeValue t, int *numSel=NULL);
	void SetWeight (TimeValue t, float w);
	void ResetWeights (TimeValue t);
	Color GetVertColor (int mp=0);
	void SetVertColor (Color clr, int mp=0);
	void SelectVertByColor (VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp=0);
	float GetAlpha (int mp=MAP_ALPHA, int *num=NULL, bool *differs=NULL);
	void SetAlpha (float alpha, int mp=MAP_ALPHA);

	// Edge Surface operations:
	void SetEdgeVis (BOOL vis);
	void AutoEdge (float thresh, int type);

	// Face Surface operations:
	void ShowNormals (DWORD vpFlags=REDRAW_NORMAL);
	void FlipNormals ();
	void UnifyNormals ();
	DWORD GetMatIndex ();
	void SetMatIndex (DWORD index);
	void SelectByMat (DWORD index,BOOL clear);
	DWORD GetUsedSmoothBits();
	DWORD GetSelSmoothBits(DWORD &some);
	void SetSelSmoothBits (DWORD bits, DWORD mask);		
	void SelectBySmoothGroup (DWORD bits, BOOL clear);
	void AutoSmooth(float thresh);
	Color GetFaceColor (int mp=0);
	void SetFaceColor (Color clr, int mp=0);

	// Psuedo-command-mode, mixed in with real ones in triops.cpp:
	void EnterSliceMode ();
	void ExitSliceMode ();

	// UI code -- triedui.cpp
	void UpdateSurfType ();
	void UpdateSurfaceSpinner (TimeValue t, HWND hWnd, int idSpin);
	void SetSelDlgEnables(), SetGeomDlgEnables(), SetARDlgEnables();
	float GetPolyFaceThresh();
	void InvalidateSurfaceUI();
	void InvalidateNumberSelected ();
	void SetNumSelLabel();
	BOOL SplitSharedVertCol();

	// UI code in approxui.cpp:
	void UpdateApproxUI ();

	// IMeshSelect methods:
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// IMeshSelectData methods:
	BitArray GetVertSel() { return GetMesh().vertSel; }
	BitArray GetFaceSel() { return GetMesh().faceSel; }
	BitArray GetEdgeSel() { return GetMesh().edgeSel; }
	BitArray GetSel (int nsl);
	void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetSel (int nsl, BitArray & set, IMeshSelect *imod, TimeValue t);
	GenericNamedSelSetList & GetNamedVertSelList () { return selSet[NS_VERTEX]; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return selSet[NS_EDGE]; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return selSet[NS_FACE]; }

	// MeshDeltaUser methods:
	void LocalDataChanged (DWORD parts);
	void ToggleCommandMode(meshCommandMode mode);
	void ButtonOp(meshButtonOp opcode);
	void GetUIParam (meshUIParam uiCode, int & ret);
	void SetUIParam (meshUIParam uiCode, int val);
	void GetUIParam (meshUIParam uiCode, float & ret);
	void SetUIParam (meshUIParam uiCode, float val);
	void ExitCommandModes () { ExitAllCommandModes (); }
	bool Editing () { return (ip && (editObj==this)) ? TRUE : FALSE; }
	DWORD GetEMeshSelLevel () { return selLevel; }
	void SetEMeshSelLevel (DWORD sl) { if (ip) ip->SetSubObjectLevel (sl); else selLevel = sl; }
	void PlugControllersSel(TimeValue t,BitArray &set);

	// MeshDeltaUserData methods:
	void ApplyMeshDelta (MeshDelta & md, MeshDeltaUser *mdu, TimeValue t);
	void MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
	void ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void ExtrudeSelection(int level, BitArray* sel, float amount, float bevel, BOOL groupNormal, Point3* direction);

	// ISubMtlAPI methods:
	void*	GetInterface(ULONG id);
	MtlID	GetNextAvailMtlID(ModContext* mc);
	BOOL	HasFaceSelection(ModContext* mc);
	void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
	int		GetSelFaceUniqueMtlID(ModContext* mc);
	int		GetSelFaceAnyMtlID(ModContext* mc);
	int		GetMaxMtlID(ModContext* mc);

	// EventUser methods:
	void Notify() {DeleteSelected();/*delete key was pressed*/}

private:
	// starts color picker for editing vert colors on the given map channel
	void EditVertColor(int mp, int ctrlID, int strID );

};

// Accelerator table callback for Editable or Edit Mesh.
class EMeshActionCB : public ActionCallback {
public:
	MeshDeltaUser *em;
	EMeshActionCB (MeshDeltaUser *emm) { em=emm; }
	BOOL ExecuteAction(int id);
};

// --- Command Modes & Mouse Procs -------------------------------

// Virtual mouse procs:
class PickEdgeMouseProc : public MouseCallBack {
public:
	EditTriObject *et;
	IObjParam *ip;

	PickEdgeMouseProc(EditTriObject* e, IObjParam *i) {et=e;ip=i;}
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt, float *prop, Point3 *snapPoint);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
	virtual void EdgePick(DWORD edge, float prop)=0;
};

class PickFaceMouseProc : public MouseCallBack {
public:
	EditTriObject *et;
	IObjParam *ip;

	PickFaceMouseProc(EditTriObject* e, IObjParam *i) {et=e;ip=i;}
	HitRecord *HitTestFaces(IPoint2 &m, ViewExp *vpt, float *bary, Point3 *snapPoint);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
	virtual void FacePick(DWORD face, float *bary)=0;
};

// Actual procs & command modes:

class CreateVertMouseProc : public MouseCallBack {
private:		
	EditTriObject *et;
	IObjParam *ip;		
public:
	CreateVertMouseProc(EditTriObject* mod, IObjParam *i) {et=mod;ip=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	CreateVertMouseProc proc;
	EditTriObject* et;

public:
	CreateVertCMode(EditTriObject* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {et=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATEVERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class CreateFaceMouseProc : public MouseCallBack {
public:
	EditTriObject *et;
	IObjParam *ip;
	Tab<int> vts;
	IPoint2 mlast, mfirst, oldm;
	int pt;

	CreateFaceMouseProc(EditTriObject* e, IObjParam *i);
	void DrawEstablishedFace (GraphicsWindow *gw);
	void DrawCreatingFace (HWND hWnd, const IPoint2 & m);
	BOOL HitTestVerts(IPoint2 m, ViewExp *vpt,int &v);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class CreateFaceCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	EditTriObject *et;
	CreateFaceMouseProc proc;

	CreateFaceCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i) {et=e;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_BUILDFACE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=999999; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class AttachPickMode : public PickModeCallback, public PickNodeCallback {
public:
	EditTriObject* eo;
	IObjParam *ip;

	AttachPickMode () { eo=NULL; ip=NULL; }
	AttachPickMode(EditTriObject* o, IObjParam *i) { eo=o; ip=i; }
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);

	BOOL Filter(INode *node);
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	PickNodeCallback *GetFilter() {return this;}
};

class AttachHitByName : public HitByNameDlgCallback {
public:
	EditTriObject *eo;
	bool inProc;

	AttachHitByName (EditTriObject *e) {eo=e; inProc=FALSE;}
	TCHAR *dialogTitle() { return GetString(IDS_ATTACH_LIST); }
	TCHAR *buttonText() { return GetString(IDS_RB_ATTACH); }
	int filter(INode *node);	
	void proc(INodeTab &nodeTab);	
};

class DivideEdgeProc : public PickEdgeMouseProc {
public:
	DivideEdgeProc(EditTriObject* e, IObjParam *i) : PickEdgeMouseProc(e,i) {}
	void EdgePick(DWORD edge, float prop);
};

class DivideEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideEdgeProc proc;
	EditTriObject* et;

public:
	DivideEdgeCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i) {et=e;}
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
	DivideFaceProc(EditTriObject* e, IObjParam *i) : PickFaceMouseProc(e,i) {}
	void FacePick(DWORD face, float *bary);
};

class DivideFaceCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideFaceProc proc;
	EditTriObject* et;

public:
	DivideFaceCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i) {et=e;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_DIVIDEFACE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class TurnEdgeProc : public PickEdgeMouseProc {
public:
	TurnEdgeProc(EditTriObject* e, IObjParam *i) : PickEdgeMouseProc(e,i) {}
	void EdgePick(DWORD edge, float prop);
};

class TurnEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	TurnEdgeProc proc;
	EditTriObject* et;

public:
	TurnEdgeCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i) {et=e;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_TURNEDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class ExtrudeMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditTriObject *eo;
	Interface *ip;
	IPoint2 om;
public:
	ExtrudeMouseProc(EditTriObject* o, IObjParam *i) : moveTrans(i) {eo=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	ExtrudeSelectionProcessor(ExtrudeMouseProc *mc, EditTriObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,o,i) {}
};

class ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ExtrudeSelectionProcessor mouseProc;
	ExtrudeMouseProc eproc;
	EditTriObject* eo;

public:
	ExtrudeCMode(EditTriObject* o, IObjParam *i) :
		fgProc(o), mouseProc(&eproc,o,i), eproc(o,i) {eo=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class BevelMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditTriObject *eo;
	Interface *ip;
	IPoint2 m0, m1;
	bool m0set, m1set;
	float height;
public:
	BevelMouseProc(EditTriObject* o, IObjParam *i) : moveTrans(i) {eo=o;ip=i; m0set=FALSE; m1set=FALSE; }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	BevelSelectionProcessor(BevelMouseProc *mc, EditTriObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,o,i) {}
};

class BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	BevelSelectionProcessor mouseProc;
	BevelMouseProc eproc;
	EditTriObject* eo;

public:
	BevelCMode(EditTriObject* o, IObjParam *i) :
		fgProc(o), mouseProc(&eproc,o,i), eproc(o,i) {eo=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class ChamferMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditTriObject *eo;
	Interface *ip;
	IPoint2 om;
public:
	ChamferMouseProc (EditTriObject* o, IObjParam *i) : moveTrans(i) {eo=o;ip=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ChamferSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditTriObject *eto;
	ChamferSelectionProcessor(ChamferMouseProc *mc, EditTriObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,o,i) {eto=o;}
};

class ChamferCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ChamferSelectionProcessor mouseProc;
	ChamferMouseProc eproc;
	EditTriObject* eo;

public:
	ChamferCMode (EditTriObject* o, IObjParam *i) :
		fgProc(o), mouseProc(&eproc,o,i), eproc(o,i) {eo=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_MCHAMFER; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class CutEdgeProc : public MouseCallBack {
public:
	EditTriObject *et;
	IObjParam *ip;
	DWORD e1;
	bool e1set;
	float prop1;
	IPoint2 m1, oldm2;

	CutEdgeProc(EditTriObject* e, IObjParam *i) { et=e; ip=i; e1set = FALSE;}
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
	void DrawCutter (HWND hWnd,IPoint2 &m);
};

class CutEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;	
	CutEdgeProc proc;
	EditTriObject* et;

public:
	CutEdgeCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i) {et=e;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CUTEDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=20; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
	void AbandonCut ();
};

class WeldVertMouseProc : public MoveModBox {
private:		
	EditTriObject *et;
	IObjParam *ip;
	int targetVert;
public:
	WeldVertMouseProc(EditTriObject* e, IObjParam *i) : MoveModBox(e,i) { et=e; ip=i; }
	BOOL HitTestVerts(IPoint2 &m, ViewExp *vpt,int &v);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	void PostTransformHolding ();
	int UndoStringID() { return IDS_RB_WELDVERTS; }
};

class WeldVertSelectionProcessor : public SubModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();

public:
	WeldVertSelectionProcessor(WeldVertMouseProc *mc, Object *o, IObjParam *i) 
		: SubModSelectionProcessor(mc,o,i) { SetSupportTransformGizmo (TRUE); }
};

class WeldVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	WeldVertSelectionProcessor mouseProc;
	WeldVertMouseProc eproc;
	EditTriObject* et;

public:
	WeldVertCMode(EditTriObject* mod, IObjParam *i) :
		fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {et=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_WELDVERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &mouseProc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) {return oldMode->ChangeFGProc() != &fgProc;}
	void EnterMode();
	void ExitMode();
};

class FlipNormProc : public PickFaceMouseProc {
public:
	FlipNormProc (EditTriObject* e, IObjParam *i) : PickFaceMouseProc(e,i) {}
	void FacePick (DWORD face, float *bary);
};

class FlipNormCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	FlipNormProc proc;
	EditTriObject* et;

public:
	FlipNormCMode(EditTriObject* e, IObjParam *i) : fgProc(e), proc(e,i	) {et=e;}
	int Class() {return MODIFY_COMMAND;}
	int ID() {return CID_FLIPNORM;}
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

// --- Restore objects ---------------------------------------------

// Not really a restore object, just used in some drag moves.
class TempMoveRestore {
public:
	Tab<Point3> init;
	BitArray active;
	Tab<Tab<UVVert> *> maps;

	TempMoveRestore (EditTriObject *em, bool useMaps=false);
	void Restore (EditTriObject *em);
	DWORD ChannelsChanged ();
};

class CueLocalRestore : public RestoreObj {
public:
	EditTriObject *editObj;

	CueLocalRestore (EditTriObject *eo) { editObj = eo; }
	void Restore(int isUndo) { editObj->DragMoveRestore(); }
	void Redo() { }
	TSTR Description() {return TSTR(_T("Cue internal Restore"));}
	int Size() { return sizeof(int) + sizeof(void *); }
};

class MeshSelRestore : public RestoreObj {	
public:		   	
	BitArray undo, redo;
	EditTriObject *et;
	int selLevel;

	MeshSelRestore(EditTriObject *et);
	MeshSelRestore (EditTriObject *et, int selLev);
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Mesh Sel"));}
};

class MeshVertRestore : public RestoreObj {
public:		   	
	Tab<Point3> undo, redo;
	BitArray uvdSupport, rvdSupport;
	PerData *uvData, *rvData;

	EditTriObject *et;

	MeshVertRestore(EditTriObject *et);
	~MeshVertRestore () { if (uvData) delete [] uvData; if (rvData) delete [] rvData; }
	void Restore(int isUndo);
	void Redo();
	void EndHold() {et->ClearAFlag(A_HELD);}
	TSTR Description() {return TSTR(_T("Mesh Geometry Change"));}
};

class MeshMapVertRestore : public RestoreObj {
public:		   	
	Tab<UVVert> undo, redo;
	int mapChannel;

	EditTriObject *et;

	MeshMapVertRestore(EditTriObject *et, int mapChan);
	bool After ();
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Mesh Map Vertex Change"));}
};

class MeshTopoRestore : public RestoreObj {
public:
	EditTriObject *et;
	Mesh umesh, rmesh;
	DWORD channels;
	BOOL undone;
	Tab<Control*> ucont, rcont;

	MeshTopoRestore(EditTriObject *et, DWORD chan);
	bool After ();
	void Restore(int isUndo);
	void Redo();
	void EndHold() {et->ClearAFlag(A_HELD);}
	TSTR Description() {return TSTR(_T("Mesh Topo"));}
};

class MeshVertHideRestore : public RestoreObj {	
public:		   	
	BitArray undo, redo;
	EditTriObject *et;		

	MeshVertHideRestore(EditTriObject *et);
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Mesh Vert Hide"));}
};

class MeshFaceHideRestore : public RestoreObj {	
public:
	BitArray undo, redo;
	EditTriObject *et;		

	MeshFaceHideRestore(EditTriObject *et);
	void Restore(int isUndo);
	void Redo();		
	TSTR Description() {return TSTR(_T("Mesh Face Hide"));}
};

class MeshFaceMatRestore : public RestoreObj {	
public:		   	
	Tab<MtlID> undo, redo;
	EditTriObject *et;		

	MeshFaceMatRestore(EditTriObject *et);
	void Restore(int isUndo);
	void Redo();
	void EndHold() {et->ClearAFlag(A_HELD);}
	TSTR Description() {return TSTR(_T("Mesh Face Mat"));}
};

class FaceIndexRec {
public:
	DWORD v[3], flags;
};
class UVFaceIndexRec {
public:
	DWORD v[3];
};


class AppendSetRestore : public RestoreObj {
public:
	BitArray set;
	TSTR name;
	GenericNamedSelSetList *setList;
	EditTriObject *et;

	AppendSetRestore(GenericNamedSelSetList *sl,EditTriObject *e) {
		setList = sl; et = e;
	}
	void Restore(int isUndo) {
		set  = *setList->sets[setList->Count()-1];
		name = *setList->names[setList->Count()-1];
		setList->DeleteSet(setList->Count()-1);
		if (et->ip) {
			et->ip->NamedSelSetListChanged();
			et->UpdateNamedSelDropDown ();
		}
	}
	void Redo() {
		setList->AppendSet(set, 0, name);
		if (et->ip) {
			et->ip->NamedSelSetListChanged();
			et->UpdateNamedSelDropDown ();
		}
	}
			
	TSTR Description() {return TSTR(_T("Append Set"));}
};

class DeleteSetRestore : public RestoreObj {
public:
	BitArray set;
	TSTR name;
	int index;
	GenericNamedSelSetList *setList;
	EditTriObject *et;

	DeleteSetRestore(TSTR nm, GenericNamedSelSetList *sl,EditTriObject *e) {
		setList = sl;
		et = e;
		set  = *(sl->GetSet(nm));
		name = nm;
	}
	void Restore(int isUndo) {
		setList->AppendSet (set, 0, name);
		if (et->ip) {
			et->ip->NamedSelSetListChanged();
			et->UpdateNamedSelDropDown ();
		}
	}
	void Redo() {
		setList->RemoveSet (name);
		if (et->ip) {
			et->ip->NamedSelSetListChanged();
			et->UpdateNamedSelDropDown ();
		}
	}
			
	TSTR Description() {return TSTR(_T("Delete Set"));}
};

class SetNameRestore : public RestoreObj {
public:
	TSTR undo, redo;
	int index;
	GenericNamedSelSetList *setList;
	EditTriObject *et;
	SetNameRestore(int i,GenericNamedSelSetList *sl,EditTriObject *e) {
		index = i; setList = sl; et = e;
		undo = *setList->names[index];
	}

	void Restore(int isUndo) {			
		redo = *setList->names[index];
		*setList->names[index] = undo;
		if (et->ip) et->ip->NamedSelSetListChanged();
	}
	void Redo() {
		*setList->names[index] = redo;
		if (et->ip) et->ip->NamedSelSetListChanged();
	}
			
	TSTR Description() {return TSTR(_T("Set Name"));}
};

class TransformPlaneRestore : public RestoreObj {
public:
	Point3 oldSliceCenter, newSliceCenter;
	Quat oldSliceRot, newSliceRot;
	float oldSliceSize, newSliceSize;
	EditTriObject *eo;
	TransformPlaneRestore (EditTriObject *eto) {
		eo = eto;
		oldSliceCenter = eo->sliceCenter;
		oldSliceRot = eo->sliceRot;
		oldSliceSize = eo->sliceSize;
	}
	void Restore (int isUndo) {
		newSliceCenter = eo->sliceCenter;
		newSliceRot = eo->sliceRot;
		newSliceSize = eo->sliceSize;
		eo->sliceCenter = oldSliceCenter;
		eo->sliceRot = oldSliceRot;
		eo->sliceSize = oldSliceSize;
		eo->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}
	void Redo () {
		oldSliceCenter = eo->sliceCenter;
		oldSliceRot = eo->sliceRot;
		oldSliceSize = eo->sliceSize;
		eo->sliceCenter = newSliceCenter;
		eo->sliceRot = newSliceRot;
		eo->sliceSize = newSliceSize;
		eo->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}
	int Size () {
		return 2*(sizeof(Point3) + sizeof(Quat) + sizeof(float))
			+ sizeof (EditTriObject *);
	}
	TSTR Description () { return TSTR (_T("Slice Plane transform")); }
};

// TriEdUI.cpp function:
BOOL GetCloneObjectName (Interface *ip, TSTR &name);
void ResetEditableMeshUI ();
ActionTable *GetEMeshActions ();

// triops.cpp
void ExplodeToObjects (Mesh *mesh, float thresh, INode *node, TSTR &name,
					   Interface *ip, MeshDelta *tmd, AdjFaceList *af, BOOL selOnly);
BOOL CreateCurveFromMeshEdges (Mesh & mesh, INode *node, Interface *ip, AdjEdgeList *ae,
							   TSTR & name, BOOL curved, BOOL ignoreHiddenEdges);

INT_PTR CALLBACK DispApproxDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



//az -  042303  MultiMtl sub/mtl name support
void GetMtlIDList(Mtl *mtl, NumList& mtlIDList);
void GetMeshMtlIDList(EditTriObject *eo, NumList& mtlIDList);
INode* GetNode (EditTriObject *eo);
BOOL SetupMtlSubNameCombo (HWND hWnd, EditTriObject *eo);
void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin);
void ValidateUINameCombo (HWND hWnd, EditTriObject *eo);


#define  SRMM_CLASS_ID 0xA8564321
class SingleRefMakerMeshMtl: public SingleRefMaker {
public:
	HWND hwnd;
	EditTriObject *eo;
	RefTargetHandle rtarget;
	SingleRefMakerMeshMtl() {hwnd = NULL; eo = NULL; rtarget = NULL; }
	~SingleRefMakerMeshMtl() { 
		theHold.Suspend();        
		DeleteAllRefsFromMe(); 
		theHold.Resume();          
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
			switch(message) {
			case REFMSG_CHANGE:
				if(hwnd) ValidateUINameCombo(hwnd, eo);
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMM_CLASS_ID; }
};


#define  SRMN_CLASS_ID 0xA8564432
class SingleRefMakerMeshNode: public SingleRefMaker {
public:
	HWND hwnd;
	EditTriObject *eo;
	RefTargetHandle rtarget;
	SingleRefMakerMeshNode() {hwnd = NULL; eo = NULL; rtarget = NULL; }
	~SingleRefMakerMeshNode() { 
		theHold.Suspend();         
		DeleteAllRefsFromMe();
		theHold.Resume();
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
			switch(message) {
			case REFMSG_NODE_MATERIAL_CHANGED:
				eo->mtlref->SetRef(GetNode(eo)->GetMtl());
				if(hwnd) ValidateUINameCombo(hwnd, eo);
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMN_CLASS_ID; }
};




#endif //__TRIOBJED__
