/**********************************************************************
 *<
	FILE: editmesh.h

	DESCRIPTION:  Edit Mesh OSM

	CREATED BY: Dan Silva & Rolf Berteig

	HISTORY: created 18 March, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __EDITMESH_H__
#define __EDITMESH_H__

#ifdef _DEBUG
//#define EM_DEBUG
#endif

#include "namesel.h"
#include "nsclip.h"
#include "sbmtlapi.h"
#include "istdplug.h"
#include "ActionTable.h"

// Available selection levels
#define SL_OBJECT EM_SL_OBJECT	//0
#define SL_VERTEX EM_SL_VERTEX	//1
#define SL_EDGE EM_SL_EDGE	//2
#define SL_FACE EM_SL_FACE	//3
#define SL_POLY EM_SL_POLYGON	//4
#define SL_ELEMENT EM_SL_ELEMENT	//5

#define DEF_PICKBOX_SIZE	4

// Alignment types:
#define ALIGN_CONST 0
#define ALIGN_VIEW 1

#define EDITMESH_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL|PART_VERTCOLOR)

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
class XFormProc;
class EditMeshData;

#define CID_EXTRUDE		CID_USER + 972
#define CID_CREATEVERT	CID_USER + 973
#define CID_OBJATTACH	CID_USER + 974
#define CID_BUILDFACE	CID_USER + 975
#define CID_DIVIDEEDGE	CID_USER + 976
#define CID_TURNEDGE	CID_USER + 977
#define CID_WELDVERT	CID_USER + 978
#define CID_DIVIDEFACE CID_USER + 979
#define CID_CUTEDGE CID_USER + 980
#define CID_FLIPNORM CID_USER + 981
#define CID_BEVEL    CID_USER + 982
#define CID_MCHAMFER CID_USER + 983	// "Mesh Chamfer" -- CID_CHAMFER was taken.

#define MAX_MATID	0xffff

// Edit Mesh Flags
// Temp flags:
#define EM_TEMPFLAGS 0xff
#define EM_SWITCH_SUBOBJ_VERSIONS 0x01
#define EM_EDITING 0x02
// "Keeper" flags:
#define EM_KEEPFLAGS (~0xff)
#define EM_DISP_RESULT 0x0100

class SingleRefMakerMeshMNode;
class SingleRefMakerMeshMMtl;

class EditMeshMod : public Modifier, public IMeshSelect, public MeshDeltaUser,
							 public ISubMtlAPI, public AttachMatDlgUser {
public:
	// Window handles & an interface:
	static HWND hGeom, hSurf, hSel, hAR;
	static IObjParam *ip;		

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
	static DivideFaceCMode* divideFaceMode;
	static AttachPickMode* attachPickMode;
	static ExtrudeCMode *extrudeMode;
	static BevelCMode* bevelMode;
	static ChamferCMode *chamferMode;
	static FlipNormCMode *flipMode;
	static CutEdgeCMode *cutEdgeMode;

	static float normScale;
	static bool showVNormals, showFNormals;
	static BOOL selByVert;
	static BOOL inBuildFace, inCutEdge;
	static BOOL faceUIValid;
	static BOOL inExtrude, inBevel, inChamfer;
	static int extType;
	static BOOL ignoreBackfaces, ignoreVisEdge;
	static BOOL rsSel, rsAR, rsGeom, rsSurf;	// Rollup States (FALSE=Rolled-up.)
	static int pickBoxSize;
	static int weldBoxSize;
	static int attachMat;
	static BOOL condenseMat;
	static bool sliceMode, sliceSplit, cutRefine;
	static Quat sliceRot;
	static Point3 sliceCenter;
	static float sliceSize;

	// additonal references
	SingleRefMakerMeshMNode* noderef;                  
	SingleRefMakerMeshMMtl* mtlref; 

	// Named selection set info:
	Tab<TSTR*> namedSel[3];
	Tab<DWORD> ids[3];
	int affectRegion, arIgBack, useEdgeDist, edgeIts;
	float falloff, pinch, bubble;
	int selLevel;

	DWORD emFlags;
	void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) emFlags |= fl; else emFlags &= ~fl; }
	void ClearFlag (DWORD fl) { emFlags &= (~fl); }
	bool GetFlag (DWORD fl) { return (emFlags&fl) ? TRUE : FALSE; }

	EditMeshMod();
	~EditMeshMod();

	Interval LocalValidity(TimeValue t);
	ChannelMask ChannelsUsed()  { return EDITMESH_CHANNELS; }
	ChannelMask ChannelsChanged() { return EDITMESH_CHANNELS; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
	Class_ID InputType() { return triObjectClassID; }
	
	int CompMatrix(TimeValue t, ModContext& mc, Matrix3& tm, Interval& valid);
	
	// From Animatable
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return Class_ID(EDITMESH_CLASS_ID,0);}
	void GetClassName(TSTR& s) { s= GetString(IDS_RB_EDITMESHMOD); }
	void RescaleWorldUnits(float f);

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);

	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	int SubObjectIndex(HitRecord *hitRec);

	void ShowEndResultChanged (BOOL showEndResult);
	
	// Named selection set handling:
	// local methods:
	void UpdateSetNames ();	// Reconciles names with EditMeshData.
	void ClearSetNames();
	int FindSet(TSTR &setName,int level);
	DWORD AddSet(TSTR &setName,int level);
	void RemoveSet(TSTR &setName,int level);
	// from BaseObject:
	BOOL SupportsNamedSubSels () {return TRUE;}
	void ActivateSubSelSet (TSTR &setName);
	void NewSetFromCurSel (TSTR &setName);
	void RemoveSubSelSet (TSTR &setName);
	// also from BaseObject, for the named selection editing dialog:
	void SetupNamedSelDropDown ();
	int NumNamedSelSets ();
	TSTR GetNamedSelSetName (int i);
	void SetNamedSelSetName (int i, TSTR & newName);
	void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
	// More local methods, relating to copying and pasting of named selections:
	void NSCopy();
	void NSPaste();
	BOOL GetUniqueSetName(TSTR &name);
	int SelectNamedSet();

	BOOL DependOnTopology(ModContext &mc);

	// Operations -- in editmops.cpp
	// Shift-cloning:
	void CloneSelSubComponents(TimeValue t);
	void AcceptCloneSelSubComponents(TimeValue t);
	// Transform stuff:
	void Transform (TimeValue t, Matrix3& partm, Matrix3 tmAxis, BOOL localOrigin, Matrix3 xfrm, int type);
	void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
	void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
	void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
	void TransformStart(TimeValue t);
	void TransformHoldingFinish(TimeValue t);
	void TransformFinish(TimeValue t);
	void TransformCancel(TimeValue t);

	void DragMoveInit (TimeValue t, bool doMaps=false);
	void DragMoveRestore ();
	void DragMove (MeshDelta & md, MeshDeltaUserData *mdu);
	void DragMoveAccept ();

	// Selection panel operations:
	void HideSelectedVerts();
	void UnhideAllVerts();
	void HideSelectedFaces();
	void UnhideAllFaces();	

	// Topological & Geometric ops from the Edit Geometry panel
	DWORD CreateVertex (Point3 pt, EditMeshData *meshData=NULL, INode *nref=NULL);
	bool CreateFace (EditMeshData *meshData, int *v, int deg);
	void DeleteSelected ();

	// from AttachMatDlgUser
	int GetAttachMat() { return attachMat; }
	void SetAttachMat(int value) { attachMat = value; }
	BOOL GetCondenseMat() { return condenseMat; }
	void SetCondenseMat(BOOL sw) { condenseMat = sw; }

	void Attach (INode *node, bool & canUndo);
	void MultiAttach (INodeTab &nodeTab);
	void Detach (TSTR &name,BOOL doFaces,BOOL del=TRUE, BOOL elem=FALSE);
	void BreakVerts ();	// happens when we click "Divide" on a vert.
	void DoExtrusion();
	void BeginExtrude(TimeValue t);
	void Extrude( TimeValue t, float amount );
	void EndExtrude(TimeValue t,BOOL accept=TRUE);
	void BeginBevel (TimeValue t, BOOL doExtrude=FALSE);
	void EndBevel (TimeValue t,BOOL accept=TRUE);
	void Bevel (TimeValue t, float outline, float height=0);
	void DoChamfer (TimeValue t);
	void BeginChamfer (TimeValue t);
	void EndChamfer (TimeValue t,BOOL accept);
	void Chamfer (TimeValue t, float amount);
	void AlignTo (int alignType);
	void MakePlanar ();
	void Collapse ();
	void Tessellate (float tens,BOOL edge);
	void Explode (float thresh,BOOL objs,TSTR &name);
	void Slice ();
	BOOL WeldVerts (float thresh);
	void WeldVerts (Point3 weldPoint);
	void RemoveIsoVerts ();
	void SelectOpenEdges ();

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
	void ShowNormals (DWORD vpflags=REDRAW_NORMAL);
	void FlipNormals ();
	void UnifyNormals ();
	DWORD GetMatIndex ();
	void SetMatIndex (DWORD index);
	void SelectByMat (DWORD index,BOOL clear);
	DWORD GetUsedSmoothBits ();
	DWORD GetSelSmoothBits (DWORD &invalid);
	void SetSelSmoothBits (DWORD bits,DWORD mask); // Only bits that are 1 in 'mask' are changed to the value given by 'bits'
	void SelectBySmoothGroup (DWORD bits,BOOL clear);		
	void AutoSmooth (float thresh);
	Color GetFaceColor (int mp=0);
	void SetFaceColor (Color clr, int mp=0);

	// Psuedo-command-mode, mixed in with real ones in editmops.cpp:
	void EnterSliceMode ();
	void ExitSliceMode ();

	// UI code -- edmui.cpp
	void UpdateSurfType ();
	void UpdateSurfaceSpinner (TimeValue t, HWND hWnd, int idSpin);
	void RefreshSelType ();
	void SetSelDlgEnables (), SetGeomDlgEnables (), SetARDlgEnables ();
	void SetSurfDlgEnables ();
	void InvalidateSurfaceUI();
	void InvalidateNumberSelected ();
	void SetNumSelLabel ();
	void ExitAllCommandModes (bool exSlice=TRUE, bool exStandardModes=true);
	float GetPolyFaceThresh ();

	void ClearMeshDataFlag(ModContextList& mcList,DWORD f);
	void DeleteMeshDataTempData();		
	void CreateMeshDataTempData();

	int NumRefs() { return 0; }
	RefTargetHandle GetReference(int i) { return NULL; }
	void SetReference(int i, RefTargetHandle rtarg) {}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message ) { return REF_SUCCEED; }

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
	IOResult LoadNamedSelChunk(ILoad *iload,int level);

	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
	void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_RB_EDITMESH); }
	void ActivateSubobjSel(int level, XFormModes& modes );
	int NeedUseSubselButton() { return 0; }
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void UpdateNamedSelDropDown ();

	void InvalidateDistances ();
	void InvalidateAffectRegion ();

	// IMeshSelect methods:
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// MeshDeltaUser methods:
	void LocalDataChanged (DWORD parts);
	void GetUIParam (meshUIParam uiCode, float & ret);
	void SetUIParam (meshUIParam uiCode, float val);
	void GetUIParam (meshUIParam uiCode, int & ret);
	void SetUIParam (meshUIParam uiCode, int val);
	void ToggleCommandMode (meshCommandMode mode);
	void ButtonOp (meshButtonOp opcode);
	void ExitCommandModes () { ExitAllCommandModes (); }

	bool Editing () { return (ip && GetFlag (EM_EDITING)) ? TRUE : FALSE; }
	DWORD GetEMeshSelLevel () { return selLevel; }
	void SetEMeshSelLevel (DWORD sl) { if (ip) ip->SetSubObjectLevel (sl); else selLevel = sl; }

	// ISubMtlAPI methods:
	void*	GetInterface(ULONG id);
	MtlID	GetNextAvailMtlID(ModContext* mc);
	BOOL	HasFaceSelection(ModContext* mc);
	void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
	int		GetSelFaceUniqueMtlID(ModContext* mc);
	int		GetSelFaceAnyMtlID(ModContext* mc);
	int		GetMaxMtlID(ModContext* mc);

	bool HasActiveSelection ();

	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);
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

// Tables based on Edit Mesh's selLevel:
// Mesh selection level:
const int meshLevel[] = {MESH_OBJECT,MESH_VERTEX,MESH_EDGE,MESH_FACE,MESH_FACE,MESH_FACE};
// Display flags:
const DWORD levelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,DISP_SELEDGES,DISP_SELFACES,DISP_SELPOLYS,DISP_SELPOLYS};
// Hit testing...
const int hitLevel[] = { 0, SUBHIT_VERTS, SUBHIT_EDGES, SUBHIT_FACES, SUBHIT_FACES, SUBHIT_FACES };

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2
// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_FACE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_FACE, CLIP_FACE, CLIP_FACE };

class EditMeshClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return GetSystemSetting(SYSSET_ENABLE_EDITMESHMOD); }
	void *			Create(BOOL loading = FALSE ) { return new EditMeshMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_EDITMESH_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(EDITMESH_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFEDIT);}
	void			ResetClassParams(BOOL fileReset);
};

void ResetEditMeshUI();

class XFormProc {
public:
	virtual Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat)=0;
	virtual void SetMat( Matrix3& mat ) {}
};

class MoveXForm : public XFormProc {
private:
	Point3 delta, tdelta;		
public:
	Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) { return p + tdelta; }
	void SetMat( Matrix3& mat ) { tdelta = VectorTransform(Inverse(mat),delta); }
	MoveXForm(Point3 d) { delta = d; }
};

class RotateXForm : public XFormProc {
private:
	Matrix3 rot, trot;
public:
	Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) { return (trot*p)*imat; }
	void SetMat( Matrix3& mat ) { trot = mat * rot; }
	RotateXForm(Quat q) { q.MakeMatrix(rot); }
};

class ScaleXForm : public XFormProc {
private:
	Matrix3 scale, tscale;
public:
	Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) { return (p*tscale)*imat; }
	void SetMat( Matrix3& mat ) { tscale = mat*scale; }
	ScaleXForm(Point3 s) { scale = ScaleMatrix(s); }
};

// EditmeshData flags
#define EMD_BEENDONE			(1<<0)
#define EMD_UPDATING_CACHE		(1<<1)
#define EMD_HASDATA				(1<<2)

// Types of welds
#define WELD_COLLAPSE	1
#define WELD_THRESHOLD	2
#define WELD_TOVERT		3

// Not really a restore object, just used in some drag moves.
class TempMoveRestore {
public:
	Tab<Point3> init;
	Tab<Tab<UVVert> *> maps;

	TempMoveRestore (Mesh *msh, bool doMaps=false);
	void Restore (Mesh *msh);
	DWORD ChannelsChanged ();
};

// This is the data that each mod app will have.
class EditMeshData : public LocalModData, public IMeshSelectData, public MeshDeltaUserData {
public:
	DWORD flags;

	// This is the change record on the incoming object.
	MeshDelta  mdelta;

	// This represents temporary drag movements:
	MeshDelta tempMove;
	TempMoveRestore *tmr;

	// Lists of named selection sets
	GenericNamedSelSetList selSet[3];

	// While an object is being edited, these exist:
	EditMeshMod *mod;
	Mesh *mesh;
	bool updateMD;
	DWORD lastInVNum, lastInFNum;
	Tab<DWORD> lastInMVNum;
	MeshTempData *tempData;
	Interval mValid, topoValid, geomValid;
	BOOL lockInvalidate;

	EditMeshData();
	EditMeshData(EditMeshData& emc);
	~EditMeshData();

	// Applies modifications to a triOb
	void Apply(TimeValue t,TriObject *triOb, EditMeshMod *mod);

	// If this is the first edit, then the delta arrays will be allocated.  Returns cached mesh.
	void SetModifier (EditMeshMod *md) { mod = md; }
	void BeginEdit (TimeValue t);
	LocalModData *Clone() { return new EditMeshData(*this); }
	
	void SetFlag(DWORD f,BOOL on) { if ( on ) flags|=f; else flags&=~f; }
	DWORD GetFlag(DWORD f) { return flags&f; }

	// Temp data related methods in edmdata.cpp:
	Mesh *GetMesh (TimeValue t);
	MeshTempData *TempData (TimeValue t);
	void Invalidate (PartID part,BOOL meshValid=TRUE);
	BOOL MeshCached(TimeValue t);
	void UpdateCache(TimeValue t, TriObject *triOb);

	// LocalModData
	void* GetInterface(ULONG id) { if (id == I_MESHSELECTDATA) return(IMeshSelectData*)this; else return LocalModData::GetInterface(id); }

	// IMeshSelectData methods:
	BitArray GetVertSel() { return mdelta.vsel; }
	BitArray GetFaceSel() { return mdelta.fsel; }
	BitArray GetEdgeSel() { return mdelta.esel; }
	BitArray GetSel (int nsl);
	void AddVertHide(BitArray set, IMeshSelect *mod, TimeValue t);	// (Not an IMeshSelectData method)
	void ClearVertHide(IMeshSelect *mod, TimeValue t);	// (Not an IMeshSelectData method)
	void SetVertSel(BitArray &set, IMeshSelect *mod, TimeValue t);
	void SetFaceSel(BitArray &set, IMeshSelect *mod, TimeValue t);
	void SetEdgeSel(BitArray &set, IMeshSelect *mod, TimeValue t);
	void SetSel (int nsl, BitArray &set, IMeshSelect *mod, TimeValue t);
	GenericNamedSelSetList & GetNamedVertSelList () { return selSet[NS_VERTEX]; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return selSet[NS_EDGE]; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return selSet[NS_FACE]; }

	// Other named selection set methods:
	void ChangeNamedSetSize (int nsl, int oldsize, int increase);
	void DeleteNamedSetArray (int nsl, BitArray & del);

	// From MeshDeltaUserData
	void ApplyMeshDelta (MeshDelta & md, MeshDeltaUser *mdu, TimeValue t);
	MeshDelta *GetCurrentMDState () { return &mdelta; }
	void MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
	void ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void ExtrudeSelection(int level, BitArray* sel, float amount, float bevel, BOOL groupNormal, Point3* direction);
};

// --- Command Modes & Mouse Procs -------------------------------

// Virtual mouse procs:
class PickEdgeMouseProc : public MouseCallBack {
public:
	EditMeshMod *em;
	IObjParam *ip;

	PickEdgeMouseProc(EditMeshMod* mod, IObjParam *i) { em=mod; ip=i; }
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt, float *prop, Point3 *snapPoint);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void EdgePick(EditMeshData *meshData,DWORD edge, float prop)=0;
};

class PickFaceMouseProc : public MouseCallBack {
public:
	EditMeshMod *em;
	IObjParam *ip;

	PickFaceMouseProc(EditMeshMod* e, IObjParam *i) {em=e;ip=i;}
	HitRecord *HitTestFaces(IPoint2 &m, ViewExp *vpt, float *bary, Point3 *snapPoint);
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
	virtual void FacePick(EditMeshData *meshData, DWORD face, float *bary)=0;
};

// Actual procs & command modes:

class CreateVertMouseProc : public MouseCallBack {
private:		
	EditMeshMod *em;
	IObjParam *ip;
public:
	CreateVertMouseProc(EditMeshMod* mod, IObjParam *i) {em=mod;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	CreateVertMouseProc proc;
	EditMeshMod* em;

public:
	CreateVertCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
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
	EditMeshMod *em;
	IObjParam *ip;
	EditMeshData *meshData;
	INode *nref;
	Tab<int> vts;
	IPoint2 mlast, mfirst, oldm;
	int pt;

	CreateFaceMouseProc(EditMeshMod* mod, IObjParam *i);
	void DrawCreatingFace (HWND hWnd, const IPoint2 & m);
	void DrawEstablishedFace (GraphicsWindow *gw);
	BOOL HitTestVerts(IPoint2 m, ViewExp *vpt,int &v);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class CreateFaceCMode : public CommandMode {
public:
	ChangeFGObject fgProc;		
	CreateFaceMouseProc proc;
	EditMeshMod* em;

	CreateFaceCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
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
	EditMeshMod* em;
	IObjParam *ip;

	AttachPickMode() : em(NULL), ip(NULL) { }
	AttachPickMode(EditMeshMod* mod, IObjParam *i) : em(mod), ip(i) { }
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);				
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);		
	
	BOOL Filter(INode *node);
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	PickNodeCallback *GetFilter() {return this;}
};

class MAttachHitByName : public HitByNameDlgCallback {
public:
	EditMeshMod *em;
	bool inProc;

	MAttachHitByName (EditMeshMod *e) {em=e; inProc=FALSE;}
	TCHAR *dialogTitle() { return GetString(IDS_EM_ATTACH_LIST); }
	TCHAR *buttonText() { return GetString(IDS_TH_ATTACH); }
	int filter(INode *node);	
	void proc(INodeTab &nodeTab);	
};

class DivideEdgeProc : public PickEdgeMouseProc {
public:
	DivideEdgeProc(EditMeshMod* mod, IObjParam *i) : PickEdgeMouseProc(mod,i) {}
	void EdgePick(EditMeshData *meshData,DWORD edge, float prop);
};

class DivideEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideEdgeProc proc;
	EditMeshMod* em;

public:
	DivideEdgeCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
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
	DivideFaceProc(EditMeshMod* e, IObjParam *i) : PickFaceMouseProc(e,i) {}
	void FacePick(EditMeshData *meshData, DWORD edge, float *bary);
};

class DivideFaceCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	DivideFaceProc proc;
	EditMeshMod* em;

public:
	DivideFaceCMode(EditMeshMod* e, IObjParam *i) : fgProc(e), proc(e,i) {em=e;}
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
	TurnEdgeProc(EditMeshMod* mod, IObjParam *i) : PickEdgeMouseProc(mod,i) {}
	void EdgePick(EditMeshData *meshData,DWORD edge, float prop);
};

class TurnEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	TurnEdgeProc proc;
	EditMeshMod* em;

public:
	TurnEdgeCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
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
	EditMeshMod *em;
	IObjParam *ip;
	IPoint2 om;
public:
	ExtrudeMouseProc(EditMeshMod* mod, IObjParam *i) : moveTrans(i) {em=mod;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	ExtrudeSelectionProcessor(ExtrudeMouseProc *mc, Modifier *m, IObjParam *i) 
		: GenModSelectionProcessor(mc,m,i) {}
};

class ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ExtrudeSelectionProcessor mouseProc;
	ExtrudeMouseProc eproc;
	EditMeshMod* em;

public:
	ExtrudeCMode(EditMeshMod* mod, IObjParam *i) :
		fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {em=mod;}
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
	EditMeshMod *em;
	Interface *ip;
	IPoint2 m0, m1;
	bool m0set, m1set;
	float height;
public:
	BevelMouseProc(EditMeshMod* m, IObjParam *i) : moveTrans(i) {em=m;ip=i; m0set=FALSE; m1set=FALSE; }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	BevelSelectionProcessor(BevelMouseProc *mc, Modifier *m, IObjParam *i) 
		: GenModSelectionProcessor(mc,m,i) {}
};

class BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	BevelSelectionProcessor mouseProc;
	BevelMouseProc eproc;
	EditMeshMod* em;
public:
	BevelCMode(EditMeshMod* m, IObjParam *i) :
		fgProc(m), mouseProc(&eproc,m,i), eproc(m,i) {em=m;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=999; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class ChamferMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditMeshMod* em;
	Interface *ip;
	IPoint2 om;
public:
	ChamferMouseProc (EditMeshMod* m, IObjParam *i) : moveTrans(i) {em=m;ip=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class ChamferSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditMeshMod* em;
	ChamferSelectionProcessor(ChamferMouseProc *mc, EditMeshMod *m, IObjParam *i) 
		: GenModSelectionProcessor(mc,m,i) {em=m;}
};

class ChamferCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ChamferSelectionProcessor mouseProc;
	ChamferMouseProc eproc;
	EditMeshMod* em;

public:
	ChamferCMode (EditMeshMod* m, IObjParam *i) :
		fgProc(m), mouseProc(&eproc,m,i), eproc(m,i) {em=m;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_MCHAMFER; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class CutEdgeProc : public MouseCallBack {	// SCA -- derived from DivideEdgeProc.
public:
	EditMeshMod *em;
	IObjParam *ip;
	DWORD e1;
	bool e1set;
	float prop1;
	IPoint2 m1, oldm2;
	ModContext *mc;

	CutEdgeProc(EditMeshMod* e, IObjParam *i) { em=e; ip=i; e1set = FALSE; mc=NULL;}
	HitRecord *HitTestEdges(IPoint2 &m, ViewExp *vpt);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
	void DrawCutter (HWND hWnd,IPoint2 &m);
};

class CutEdgeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	CutEdgeProc proc;
	EditMeshMod* em;

public:
	CutEdgeCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CUTEDGE; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=999; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
	void AbandonCut ();
};

class WeldVertMouseProc : public MoveModBox {
private:		
	EditMeshMod *em;
	IObjParam *ip;
	int targetVert;
	EditMeshData *emd;
public:
	WeldVertMouseProc(EditMeshMod* mod, IObjParam *i) : MoveModBox(mod,i) {em=mod;ip=i;}		
	BOOL HitTestVerts(IPoint2 &m, ViewExp *vpt,int &v);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
	void PostTransformHolding ();
	int UndoStringID() { return IDS_RB_WELDVERTS; }
};

class WeldVertSelectionProcessor : public SubModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
	
public:
	WeldVertSelectionProcessor(WeldVertMouseProc *mc, Modifier *m, IObjParam *i) 
		: SubModSelectionProcessor(mc,m,i) { SetSupportTransformGizmo (TRUE); }
};

class WeldVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	WeldVertSelectionProcessor mouseProc;
	WeldVertMouseProc eproc;
	EditMeshMod* em;

public:
	WeldVertCMode(EditMeshMod* mod, IObjParam *i) :
		fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {em=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_WELDVERT; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

/*
class WeldToCMode : public MoveModBoxCMode {
public:
	EditMeshMod* em;

	WeldToCMode (EditMeshMod *mod, IObjParam *i) : MoveModBoxCMode (mod,i) { em=mod; }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_WELDVERT; }
	void PostTransformHolding ();
	void EnterMode();
	void ExitMode();
};
*/

class FlipNormProc : public PickFaceMouseProc {
public:
	FlipNormProc (EditMeshMod *mod, IObjParam *i) : PickFaceMouseProc (mod,i) {}
	void FacePick (EditMeshData *meshData, DWORD face, float *bary);
};

class FlipNormCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	FlipNormProc proc;
	EditMeshMod* em;

public:
	FlipNormCMode(EditMeshMod* mod, IObjParam *i) : fgProc(mod), proc(mod,i) {em=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_FLIPNORM; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class EditMeshDeleteEvent : public EventUser {
public:
	EditMeshMod *em;

	void Notify() {if (em) em->DeleteSelected();}
	void SetEditMeshMod(EditMeshMod *e) {em=e;}
};
extern EditMeshDeleteEvent delEvent;

// --- Restore Objects -----------------------------------------

class CueLocalRestore : public RestoreObj {
public:
	EditMeshMod *emesh;

	CueLocalRestore (EditMeshMod *em) { emesh = em; }
	void Restore(int isUndo) { emesh->DragMoveRestore(); }
	void Redo() { }
	TSTR Description() {return TSTR(_T("Cue internal Restore"));}
	int Size() { return sizeof(int) + sizeof(void *); }
};

class MeshSelectRestore : public RestoreObj {
public:
	BitArray undo, redo;
	EditMeshData *meshData;
	EditMeshMod *mod;
	DWORD selLevel;
	TimeValue t;

	MeshSelectRestore (EditMeshData * md, EditMeshMod * mm);
	MeshSelectRestore (EditMeshData * md, EditMeshMod * mm, DWORD selLev);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 2*sizeof(void *) + 2*sizeof(BitArray) + sizeof(DWORD) + sizeof(TimeValue); }
	TSTR Description() { return TSTR(_T("Edit Mesh Selection")); }
};

class VertexEditRestore : public RestoreObj {
public:
	Tab<VertMove> oMove, nMove;
	//Tab<Point3> oCreate, nCreate;
	Tab<VertMove> oClone, nClone;
	EditMeshData *meshData;
	EditMeshMod	 *mod;

	~VertexEditRestore() { }
	VertexEditRestore(EditMeshData* md, EditMeshMod* mod);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 2*sizeof(void *) + 4*sizeof(Tab<Point3>); }
			
	TSTR Description() { return TSTR(_T("Move Vertices")); }
};

class MapVertexEditRestore : public RestoreObj {
public:
	int mapChannel;
	Tab<UVVertSet> oSet, nSet;
	Tab<UVVert> oCreate, nCreate;
	EditMeshData *meshData;
	EditMeshMod	 *mod;

	MapVertexEditRestore(EditMeshData* md, EditMeshMod* mod, int mapChan);
	void After ();
	void Restore (int isUndo);
	void Redo();
	int Size() { return 2*sizeof(void *) + 4*sizeof(Tab<Point3>); }

	TSTR Description() { return TSTR(_T("Move Map Vertices")); }
};

// This one is memory-heavy, but can cope with anything.
class MeshEditRestore : public RestoreObj {
public:
	MeshDelta	omdelta, nmdelta;
	EditMeshData *meshData;
	EditMeshMod * mod;
	DWORD changeFlags;
	Tab<DWORD> mapChanges;
	bool updateMD;

	MeshEditRestore (EditMeshData* md, EditMeshMod *mod, MeshDelta & changer);
	MeshEditRestore (EditMeshData* md, EditMeshMod *mod, DWORD cflags);

	void Restore(int isUndo);
	void Redo();
	int Size () { return 2*sizeof(MeshDelta) + 2*sizeof (void *) + sizeof(DWORD); }
	TSTR Description() { return TSTR(_T("Mesh Topological Edit")); }
};

class VertexHideRestore : public RestoreObj {
public:
	BitArray hide;
	BitArray rhide;
	EditMeshData *meshData;
	EditMeshMod	 *mod;
	TimeValue t;

	~VertexHideRestore() {};
	VertexHideRestore(EditMeshData* md, EditMeshMod* mod);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }

	TSTR Description() { return TSTR(_T("Hide Vertices")); }
};

class FaceChangeRestore : public RestoreObj {
public:
	Tab<FaceChange> attribs;
	Tab<FaceChange> rattribs;

	EditMeshData *meshData;
	EditMeshMod	 *mod;		

	~FaceChangeRestore() {};
	FaceChangeRestore(EditMeshData* md, EditMeshMod* mod);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 2*sizeof(Tab<FaceChange>); }

	TSTR Description() { return TSTR(_T("Face attributes")); }
};

class TransformPlaneRestore : public RestoreObj {
public:
	Point3 oldSliceCenter, newSliceCenter;
	Quat oldSliceRot, newSliceRot;
	float oldSliceSize, newSliceSize;
	EditMeshMod *em;
	TransformPlaneRestore (EditMeshMod *emm);
	void Restore (int isUndo);
	void Redo ();
	int Size () {
		return 2*(sizeof(Point3) + sizeof(Quat) + sizeof(float))
			+ sizeof (EditMeshMod *);
	}
	TSTR Description () { return TSTR (_T("Slice Plane move")); }
};


#define  SRMEMMN_CLASS_ID 0x14e86d4e
class SingleRefMakerMeshMNode : public SingleRefMaker{
public:
	HWND hwnd;
	EditMeshMod *em;
	SingleRefMakerMeshMNode() {hwnd = NULL; em = NULL;}
	~SingleRefMakerMeshMNode() { 
		theHold.Suspend();  
		DeleteAllRefsFromMe();
		theHold.Resume();  
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
	SClass_ID  SuperClassID() { return SRMEMMN_CLASS_ID; }
};

#define  SRMEMMM_CLASS_ID 0xa180fb9
class SingleRefMakerMeshMMtl : public SingleRefMaker{
public:	
	HWND hwnd;
	EditMeshMod *em;
	SingleRefMakerMeshMMtl() {hwnd = NULL; em = NULL;}
	~SingleRefMakerMeshMMtl() { 
		theHold.Suspend();  
		DeleteAllRefsFromMe();
		theHold.Resume();  
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
	SClass_ID  SuperClassID() { return SRMEMMM_CLASS_ID; }
};


// In EdMUI.cpp:
BOOL GetCloneObjectName (Interface *ip, TSTR &name);

// In EditMops.cpp:
void ExplodeToObjects (Mesh *mesh, float thresh, INode *node, TSTR &name,
					   IObjParam *ip, MeshDelta *md, AdjFaceList *af, BOOL selFaces);
BOOL CreateCurveFromMeshEdges (Mesh & mesh, INode *onode, Interface *ip, AdjEdgeList *ae,
							   TSTR & name, BOOL curved, BOOL ignoreHiddenEdges);
#endif

