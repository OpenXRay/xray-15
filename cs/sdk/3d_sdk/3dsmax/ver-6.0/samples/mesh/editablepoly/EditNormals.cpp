/**********************************************************************
 *<
	FILE: EditNormals.cpp

	DESCRIPTION:  An Edit Normals modifier for PolyMeshes

	CREATED BY: Steve Anderson

	HISTORY: Created January 21, 2002

 *>	Copyright (c) 2002 Autodesk, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"

#ifndef NO_MODIFIER_EDIT_NORMAL

#include "iparamm2.h"
#include "namesel.h"
#include "nsclip.h"
#include "istdplug.h"
#include "iColorMan.h"
#include "MaxIcon.h"
#include "MeshDLib.h"	// for MatrixFromNormal method.
#include "MeshNormalSpec.h"
#include "iEditNormals.h"
#include "macrorec.h"

#ifdef _DEBUG
//#define EN_RESTORE_DEBUG
#endif

// Move to bitarray.h someday?  Definitely be useful...
void DebugPrintLiveBits (BitArray & bitArray) {
	int bit, numLive=0;
	for (bit=0; bit<bitArray.GetSize(); bit++) {
		if (!bitArray[bit]) continue;
		DebugPrint ("%2d ", bit);
		numLive++;
		if (numLive%8==7) DebugPrint ("\n");
	}
	if (numLive%8) DebugPrint ("\n");
}

#define EDIT_NORMALS_CLASS_ID Class_ID(0x4aa52ae3, 0x35ca1cde)

static GenSubObjType SOT_Normal(23);	// Looks like a normal to me.

// EditNormalsMod References:
#define REF_PBLOCK 0

class EditNormalsModData;
class EditNormalsActionCB;
class ENAverageNormalsCMode;

class EditNormalsMod : public Modifier, public IEditNormalsMod {
private:
	int mSelLevel;
	Tab<TSTR*> mNamedSelNames;
	Tab<DWORD> mNamedSelIDs;
	IParamBlock2 *mpParamBlock;
	bool mShowEndResult;
	Point3 *mpCopiedNormal;

	// CAL-03/21/03: For hit testing other than the current SO level:
	int mHitLevelOverride;

	static IObjParam *mpInterface;

	// Standard command modes supported:
	static SelectModBoxCMode *mpSelectMode;
	static MoveModBoxCMode *mpMoveMode;
	static RotateModBoxCMode *mpRotateMode;

	static bool mOldShowEnd;
	static EditNormalsActionCB *mpEditNormalsActions;

	static bool		mUnifyBreakToAvg;		// CAL-04/09/03: Unify/Break to the Average
	static bool		mUseAvgThreshold;		// CAL-04/09/03: use average threshold

	static ENAverageNormalsCMode *mpAverageNormalsCMode;	// CAL-03/21/03: average normals command mode
	static float	mAverageThreshold;		// CAL-03/21/03: average selection threshold
	static int		mAverageBoxSize;		// CAL-03/21/03: average target pick box size
	static int		mPickBoxSize;			// CAL-03/21/03: pick box size for hit test

public:
	EditNormalsMod();
	~EditNormalsMod() { if (mpCopiedNormal) delete mpCopiedNormal; }

	// Accessors:
	IObjParam *GetUserInterface () { return mpInterface; }
	int EnfnGetSelLevel ();
	void EnfnSetSelLevel (int selLevel);
	bool HasCopiedNormal () { return mpCopiedNormal ? true : false; }
	bool GetUnifyBreakToAvg () const { return mUnifyBreakToAvg; }		// CAL-04/09/03: Unify/Break to the average
	void SetUnifyBreakToAvg (bool toAverage) { mUnifyBreakToAvg = toAverage; }
	bool GetUseAvgThreshold () const { return mUseAvgThreshold; }		// CAL-04/09/03: use threshold in average
	void SetUseAvgThreshold (bool useAvgThresh) { mUseAvgThreshold = useAvgThresh; }
	float GetAverageThreshold () const { return mAverageThreshold; }	// CAL-03/21/03: average selection threshold
	void SetAverageThreshold (float threshold) { mAverageThreshold = threshold; }
	int GetAverageBoxSize () const { return mAverageBoxSize; }			// CAL-03/21/03: target average pick box size
	void SetAverageBoxSize (int boxSize) { mAverageBoxSize = boxSize; }
	void SetPickBoxSize (int boxSize) { mPickBoxSize = boxSize; }

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_EDIT_NORMALS_MOD);}  
	virtual Class_ID ClassID() { return EDIT_NORMALS_CLASS_ID;}		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_EDIT_NORMALS_MOD);}
	void* GetInterface(ULONG id) { return Modifier::GetInterface(id); }

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_TOPO|PART_GEOM;}	// Channels used by normals
	ChannelMask ChannelsChanged() {return PART_TOPO|PART_GEOM|PART_SUBSEL_TYPE;}	// Channels normals are stored in - and we set to object level.
	Class_ID InputType() {return defObjectClassID;}
	void ModifyPolyObject (TimeValue t, EditNormalsModData *pData, PolyObject *pobj);
	void ModifyTriObject (TimeValue t, EditNormalsModData *pData, TriObject *tobj);
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity (TimeValue t) { return FOREVER; }
	BOOL DependOnTopology(ModContext &mc) {return TRUE;}

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	void ShowEndResultChanged (BOOL showEndResult) { NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE); }

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
	void UpdateSetNames ();	// Reconciles names with EditNormalsModData.
	void ClearSetNames();
	void UpdateNamedSelDropDown ();

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult LoadNamedSelChunk(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock (int i) { return mpParamBlock; }
	IParamBlock2 *GetParamBlockByID (short id) { return (mpParamBlock->ID() == id) ? mpParamBlock : NULL; }

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return mpParamBlock; }
	void SetReference(int i, RefTargetHandle rtarg) { mpParamBlock = (IParamBlock2 *) rtarg; }

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) {return GetString (IDS_PARAMETERS);}

	// This is necessary for object->TriObject conversions,
	// in order to prevent the XTCObject we attach to the new TriObject from being deleted.
	void CopyAdditionalChannels (Object *fromObj, Object *toObj) { toObj->CopyAdditionalChannels (fromObj, false); }

	// Transform stuff:
	bool EnfnMove (Point3 & offset, TimeValue t);
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	bool EnfnRotate (Quat & rotation, TimeValue t);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
	void TransformStart(TimeValue t);
	void TransformHoldingStart(TimeValue t);
	void TransformHoldingFinish (TimeValue t);
	void TransformFinish(TimeValue t);
	void TransformCancel(TimeValue t);

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

	void UpdateDialog ();
	void InvalidateDialogElement (int elem);
	void LocalDataChanged (DWORD partIDs);

	// CAL-03/21/03: Used in command modes to change which level we're hit testing on.
	void SetHitLevelOverride (int hlo) { mHitLevelOverride = hlo; }
	void ClearHitLevelOverride () { mHitLevelOverride = 0; }
	int GetHitLevelOverride () { return mHitLevelOverride; }

	// Our actual operations:
	// These are not in the interface, but are called by the UI:
	bool BreakNormals (bool toAverage=false);
	bool UnifyNormals (bool toAverage=false);
	bool ResetNormals ();
	bool SpecifyNormals ();
	bool MakeNormalsExplicit ();
	bool CopyNormal ();
	bool PasteNormal ();
	bool AverageNormals (bool useThresh=false, float threshold=0.0f);		// CAL-03/21/03: average selected normals
	bool AverageGlobalNormals (bool useThresh=false, float threshold=0.0f);	// CAL-03/21/03: average normals among objects
	bool AverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2);
	void ToggleAverageNormalsCMode ();				// CAL-03/21/03:

	// This is used to force normal recomputation, for example if the user checks the
	// "Use Legacy R4 Vertex Normals" checkbox in Preferences / General
	void InvalidateComputedNormals ();

	// These implement IEditNormals methods
	// Each of the following is called on the LocalModData of the node indicated.
	// If the node is NULL, the first node using this modifier is used.
	bool EnfnBreakNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false);
	bool EnfnUnifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false);
	bool EnfnResetNormals (BitArray *normalSelection=NULL, INode *pNode=NULL);
	bool EnfnSpecifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL);
	bool EnfnMakeNormalsExplicit (BitArray *normalSelection=NULL, INode *pNode=NULL);
	bool EnfnCopyNormal (int normalID, INode *pNode=NULL);
	bool EnfnPasteNormal (BitArray *normalSelection=NULL, INode *pNode=NULL);
	bool EnfnAverageNormals (bool useThresh=false, float threshold=0.0f, BitArray *normalSelection=NULL, INode *pNode=NULL);	// CAL-03/21/03:
	bool EnfnAverageGlobalNormals (bool useThresh=false, float threshold=0.0f);												// CAL-03/21/03:
	bool EnfnAverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2);							// CAL-03/21/03:

	// Selection access
	BitArray *EnfnGetSelection (INode *pNode=NULL);
	bool EnfnSetSelection (BitArray & selection, INode *pNode=NULL);
	bool EnfnSelect (BitArray & selection, bool invert=false, bool select=true, INode *pNode=NULL);
	void EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection, INode *pNode=NULL);
	void EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection, INode *pNode=NULL);
	void EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection, INode *pNode=NULL);

	// Normal access
	int EnfnGetNumNormals (INode *pNode=NULL);
	Point3 *EnfnGetNormal (int normalID, INode *pNode=NULL, TimeValue t=0);
	void EnfnSetNormal (int normalID, Point3 &direction, INode *pNode=NULL, TimeValue t=0);
	bool EnfnGetNormalExplicit (int normID, INode *pNode=NULL);
	void EnfnSetNormalExplicit (int normID, bool specified=true, INode *pNode=NULL);

	// Normal face access
	int EnfnGetNumFaces (INode *pNode=NULL);
	int EnfnGetFaceDegree (int face, INode *pNode=NULL);
	int EnfnGetNormalID (int face, int corner, INode *pNode=NULL);
	void EnfnSetNormalID (int face, int corner, int normalID, INode *pNode=NULL);
	bool EnfnGetFaceNormalSpecified (int face, int corner, INode *pNode=NULL);
	void EnfnSetFaceNormalSpecified (int face, int corner, bool specified=true, INode *pNode=NULL);

	// Access to Vertices
	int EnfnGetNumVertices (INode *pNode=NULL);
	int EnfnGetVertexID (int face, int corner, INode *pNode=NULL);
	Point3 EnfnGetVertex (int vertexID, INode *pNode=NULL, TimeValue t=0);

	// Access to edges
	int EnfnGetNumEdges (INode *pNode=NULL);
	int EnfnGetEdgeID (int faceIndex, int sideIndex, INode *pNode=NULL);
	int EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex, INode *pNode=NULL);
	int EnfnGetEdgeVertex (int edgeIndex, int end, INode *pNode=NULL);
	int EnfnGetEdgeFace (int edgeIndex, int side, INode *pNode=NULL);
	int EnfnGetEdgeNormal (int edgeIndex, int end, int side, INode *pNode=NULL);

	// Cache invalidators
	void EnfnRecomputeNormals(INode *pNode=NULL);
	void EnfnRebuildNormals(INode *pNode=NULL);

	MNMesh *EnfnGetMesh (INode *pNode=NULL, TimeValue t=0);
	MNNormalSpec *EnfnGetNormals (INode *pNode=NULL, TimeValue t=0);

	BaseInterface *GetInterface (Interface_ID id);
};

// CAL-04/08/03: Average at all subobject levels (normal/vertex/edge/face). currently work at normal only.
// #define EN_AVERAGE_AT_ALL_SUBOBJECT_LEVELS

// CAL-03/21/03: average normals threshold values
#define EN_AVERAGE_DEF_THRESHOLD 0.1f
#define EN_AVERAGE_MIN_THRESHOLD 0.0f
#define EN_AVERAGE_MAX_THRESHOLD 9999999.0f
#define EN_DEF_PICKBOX_SIZE 4
#define EN_MIN_PICKBOX_SIZE 1
#define EN_MAX_PICKBOX_SIZE 1000

// Static class member initialization
bool EditNormalsMod::mOldShowEnd = NULL;
EditNormalsActionCB *EditNormalsMod::mpEditNormalsActions = NULL;
IObjParam *EditNormalsMod::mpInterface = NULL;
SelectModBoxCMode *EditNormalsMod::mpSelectMode = NULL;
MoveModBoxCMode *EditNormalsMod::mpMoveMode = NULL;
RotateModBoxCMode *EditNormalsMod::mpRotateMode = NULL;
bool  EditNormalsMod::mUnifyBreakToAvg = FALSE;		// CAL-04/09/03: Unify/Break to the Average
bool  EditNormalsMod::mUseAvgThreshold = FALSE;		// CAL-04/09/03: use threshold in average
ENAverageNormalsCMode *EditNormalsMod::mpAverageNormalsCMode = NULL;	// CAL-03/21/03: average normals command mode
float EditNormalsMod::mAverageThreshold = EN_AVERAGE_DEF_THRESHOLD;		// CAL-03/21/03: average selection threshold
int	  EditNormalsMod::mAverageBoxSize = EN_DEF_PICKBOX_SIZE;			// CAL-03/21/03: target average pick box size
int	  EditNormalsMod::mPickBoxSize = EN_DEF_PICKBOX_SIZE;				// CAL-03/21/03: pick box size for hit test

// class EditNormalsModData flags:
#define ENMD_NS_HELD 0x01
#define ENMD_NT_HELD 0x02
#define ENMD_NE_HELD 0x04
#define ENMD_TRIMESH 0x08

class EditNormalsModData : public LocalModData, public FlagUser {
private:
	// Actual specified normal data:
	MNNormalSpec *mpLocalPolyNormals;
	MeshNormalSpec *mpLocalTriNormals;
	Interval mGeomValid, mTopoValid;

	// List of named selection sets
	GenericNamedSelSetList mNamedSet;

	// Pointer to local copy of mesh last used with this LocalModData:
	// (Heavy memorywise, but necessary for display, hit-testing, algorithms.)
	// (We use CopyBasics, so it doesn't contain map, normal, vdata, etc info.)
	MNMesh *mpPolyMesh;
	Mesh *mpTriMesh;

	// Temporary BitArray to store new selection info as it's being built:
	BitArray *mpNewSelection;

public:
	// LocalModData method
	void* GetInterface(ULONG id) { return LocalModData::GetInterface(id); }

	EditNormalsModData () : mpLocalPolyNormals(NULL), mpLocalTriNormals(NULL), mpPolyMesh(NULL),
		mpTriMesh(NULL), mpNewSelection(NULL) { }
	~EditNormalsModData();

	LocalModData *Clone();

	// Data accessors:
	void AllocPolyMeshCache () { if (!mpPolyMesh) mpPolyMesh = new MNMesh; }
	MNMesh *GetPolyMeshCache () { return mpPolyMesh; }
	void ClearPolyMeshCache() { if (mpPolyMesh) delete mpPolyMesh; mpPolyMesh = NULL; }

	void AllocTriMeshCache () { if (!mpTriMesh) mpTriMesh = new Mesh; }
	Mesh *GetTriMeshCache () { return mpTriMesh; }
	void ClearTriMeshCache() { if (mpTriMesh) delete mpTriMesh; mpTriMesh = NULL; }

	void InvalidateGeomCache () { mGeomValid.SetEmpty (); }
	void InvalidateTopoCache () { mTopoValid.SetEmpty (); }
	bool GetGeomValid (TimeValue t) { return mGeomValid.InInterval(t) ? true : false; }
	bool GetTopoValid (TimeValue t) { return mTopoValid.InInterval(t) ? true : false; }
	void SetGeomValid (Interval & iValid) { mGeomValid = iValid; }
	void SetTopoValid (Interval & iValid) { mTopoValid = iValid; }

	void AllocatePolyNormals() { if (!mpLocalPolyNormals) mpLocalPolyNormals = new MNNormalSpec; }
	MNNormalSpec *GetPolyNormals () { return mpLocalPolyNormals; }
	void ClearPolyNormals () { delete mpLocalPolyNormals; mpLocalPolyNormals = NULL; }
	void AllocateTriNormals() { if (!mpLocalTriNormals) mpLocalTriNormals = new MeshNormalSpec; }
	MeshNormalSpec *GetTriNormals () { return mpLocalTriNormals; }
	void ClearTriNormals () { delete mpLocalTriNormals; mpLocalTriNormals = NULL; }
	GenericNamedSelSetList & GetNamedNormalSelList () { return mNamedSet; }

	// New selection methods:
	void SetupNewSelection ();
	BitArray *GetNewSelection () { return mpNewSelection; }
	bool ApplyNewSelection (bool keepOld=false, bool invert=false, bool select=true);

	// ModData support for IEditNormalsMod methods:

	// Selection accessors
	BitArray *EnfnGetSelection ();
	bool EnfnSetSelection (BitArray & selection);
	bool EnfnSelect (BitArray & selection, bool invert=false, bool select=true);
	void EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection);
	void EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection);
	void EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection);

	// Accessors for the mesh and normals:
	// Normals can be used by multiple faces, even at different vertices.
	// So we require access to both face and normal information.
	int EnfnGetNumNormals ();
	int EnfnGetNumFaces ();

	// Direct access to the normals themselves:
	Point3 *EnfnGetNormal (int normalID, TimeValue t);
	void EnfnSetNormal (int normalID, Point3 &direction, TimeValue t);

	// Control whether a given normal is built from smoothing groups or set to an explicit value
	bool EnfnGetNormalExplicit (int normID);
	// (Also sets the normal to a specified ID for all faces using this normal.)
	void EnfnSetNormalExplicit (int normID, bool value);

	// Access to the normals that are assigned to each face:
	int EnfnGetFaceDegree (int face);
	int EnfnGetNormalID (int face, int corner);
	void EnfnSetNormalID (int face, int corner, int normalID);

	// Access to vertices - often important for proper normal handling to have access to the vertex the normal is based on.
	int EnfnGetNumVertices ();
	int EnfnGetVertexID (int face, int corner);
	Point3 EnfnGetVertex (int vertexID, TimeValue t);

	// Access to Edges
	int EnfnGetNumEdges ();
	int EnfnGetEdgeID (int faceIndex, int sideIndex);
	int EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex);
	int EnfnGetEdgeVertex (int edgeIndex, int end);
	int EnfnGetEdgeFace (int edgeIndex, int side);
	int EnfnGetEdgeNormal (int edgeIndex, int end, int side);

	// Control whether a corner of a face uses an explicit normal ID, or builds normals based on smoothing groups.
	bool EnfnGetFaceNormalSpecified (int face, int corner);
	void EnfnSetFaceNormalSpecified (int face, int corner, bool specified);

	// Direct accessors to MNMesh and MNNormalSpec classes - unpublished for now.
	MNMesh *EnfnGetMesh (TimeValue t) { return mpPolyMesh; }
	MNNormalSpec *EnfnGetNormals (TimeValue t) { return mpLocalPolyNormals; }

	Mesh *EnfnGetTriMesh (TimeValue t) { return mpTriMesh; }
	MeshNormalSpec *EnfnGetTriNormals (TimeValue t) { return mpLocalTriNormals; }

	// Override FlagUser::SetFlag.
	void SetFlag (DWORD fl, bool val=true);
	BitArray & GetSelection ();

	// CAL-03/21/03: Support functions for AverageGlobalNormals
	bool FindNearbyVertices(const Point3 &refPoint, float sqrThreshDist, const BitArray &inSelection, BitArray &outSelection);
	bool SumSelectedNormalsOnVertices(const BitArray &vertSelection, Point3 &outNormal);
	bool SetSelectedNormalsOnVertices(const BitArray &vertSelection, const Point3 &inNormal);
};

//----- Our Restore Objects ----------------//

class NormalSelectNotify : public RestoreObj {
	EditNormalsMod *mpMod;

public:
	NormalSelectNotify (EditNormalsMod *m) : mpMod(m) { }
	void Restore (int isUndo);
	void Redo ();
	int Size() { return sizeof (void *); }
	TSTR Description() { return TSTR(_T("NormalSelectNotify")); }
};

void NormalSelectNotify::Restore (int isUndo) {
	mpMod->LocalDataChanged (PART_SELECT);
}

void NormalSelectNotify::Redo () {
	mpMod->LocalDataChanged (PART_SELECT);
}

class NormalSelectRestore : public RestoreObj {
	BitArray mOldSel, mNewSel;
	EditNormalsModData *mpData;

public:
	NormalSelectRestore (EditNormalsModData *pData, BitArray *pNewSel=NULL);
	void Restore (int isUndo);
	void Redo();
	int Size() { return 2*sizeof(BitArray) + sizeof(void *); }
	void EndHold() {mpData->ClearFlag(ENMD_NS_HELD);}
	TSTR Description() { return TSTR(_T("NormalSelectRestore")); }
};

NormalSelectRestore::NormalSelectRestore (EditNormalsModData *pData, BitArray *pNewSel) : mpData(pData) {
	mpData->SetFlag(ENMD_NS_HELD);
	if (mpData->GetFlag (ENMD_TRIMESH)) mOldSel = mpData->GetSelection();
	if (pNewSel) mNewSel = *pNewSel;
}

void NormalSelectRestore::Restore (int isUndo) {
	if (isUndo && (mNewSel.NumberSet() == 0)) {
		if (mpData->GetFlag (ENMD_TRIMESH)) mNewSel = mpData->GetSelection();
	}

	mpData->GetSelection() = mOldSel;
}

void NormalSelectRestore::Redo() {
	mpData->GetSelection() = mNewSel;
}

class NormalTransformRestore : public RestoreObj {
	Tab<int> mtNormalLookup;
	Tab<Point3> mtOldNormals, mtNewNormals;
	EditNormalsMod *mpMod;
	EditNormalsModData *mpData;

public:
	NormalTransformRestore (EditNormalsMod *m, EditNormalsModData *pData);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 3*sizeof(Tab<int>) + 2*sizeof(void *); }
	void EndHold() {mpData->ClearFlag(ENMD_NT_HELD);}
	TSTR Description() { return TSTR(_T("NormalTransformRestore")); }
};

NormalTransformRestore::NormalTransformRestore (EditNormalsMod *m, EditNormalsModData *pData) : mpMod(m), mpData(pData) {
	mpData->SetFlag(ENMD_NT_HELD);

	// Form lookup table so we only store the selected normals:
	int i, numSelected=0;
	mtNormalLookup.SetCount (mpData->GetSelection().GetSize());
	for (i=0; i<mtNormalLookup.Count(); i++) {
		if (mpData->GetSelection()[i]) {
			mtNormalLookup[numSelected] = i;
			numSelected++;
		}
	}

	mtNormalLookup.SetCount (numSelected);
	mtOldNormals.SetCount (numSelected);
	if (!numSelected) return;

	// Record the selected normals:
	if (mpData->GetFlag (ENMD_TRIMESH)) {
		for (i=0; i<mtNormalLookup.Count(); i++) {
			mtOldNormals[i] = mpData->GetTriNormals()->Normal(mtNormalLookup[i]);
		}
	} else {
		for (i=0; i<mtNormalLookup.Count(); i++) {
			mtOldNormals[i] = mpData->GetPolyNormals()->Normal(mtNormalLookup[i]);
		}
	}
}

void NormalTransformRestore::Restore (int isUndo) {
	if (mtOldNormals.Count() == 0) return;
	int i;
	if (isUndo && (mtNewNormals.Count()==0)) {
		// Record the new normals:
		mtNewNormals.SetCount(mtOldNormals.Count());
		if (mpData->GetFlag (ENMD_TRIMESH)) {
			for (i=0; i<mtNormalLookup.Count(); i++) {
				mtNewNormals[i] = mpData->GetTriNormals()->Normal(mtNormalLookup[i]);
			}
		} else {
			for (i=0; i<mtNormalLookup.Count(); i++) {
				mtNewNormals[i] = mpData->GetPolyNormals()->Normal(mtNormalLookup[i]);
			}
		}
	}

	// Restore the old normals:
	if (mpData->GetFlag (ENMD_TRIMESH)) {
		for (i=0; i<mtNormalLookup.Count(); i++) {
			mpData->GetTriNormals()->Normal(mtNormalLookup[i]) = mtOldNormals[i];
		}
	} else {
		for (i=0; i<mtNormalLookup.Count(); i++) {
			mpData->GetPolyNormals()->Normal(mtNormalLookup[i]) = mtOldNormals[i];
		}
	}

	mpMod->LocalDataChanged (PART_GEOM);
}

void NormalTransformRestore::Redo () {
	if (mtNewNormals.Count() == 0) return;

	// Restore the new normals:
	if (mpData->GetFlag (ENMD_TRIMESH)) {
		for (int i=0; i<mtNormalLookup.Count(); i++) {
			mpData->GetTriNormals()->Normal(mtNormalLookup[i]) = mtNewNormals[i];
		}
	} else {
		for (int i=0; i<mtNormalLookup.Count(); i++) {
			mpData->GetPolyNormals()->Normal(mtNormalLookup[i]) = mtNewNormals[i];
		}
	}

	mpMod->LocalDataChanged (PART_GEOM);
}

class AddNormalSetRestore : public RestoreObj {
	BitArray mSet;
	DWORD mID;
	TSTR mName;
	EditNormalsModData *mpData;

public:
	AddNormalSetRestore (EditNormalsModData *pData, DWORD i, TSTR &n) : 
	  mpData(pData), mID(i), mName(n) { }
	void Restore(int isUndo);
	void Redo();
	int Size() { return sizeof(BitArray) + sizeof(DWORD) + sizeof(TSTR) + sizeof(void *); }
	TSTR Description() {return TSTR(_T("AddNamedNormalSelectionRestore"));}
};

void AddNormalSetRestore::Restore (int isUndo) {
	mSet = *(mpData->GetNamedNormalSelList().GetSet(mID));
	mpData->GetNamedNormalSelList().RemoveSet (mID);
}

void AddNormalSetRestore::Redo () {
	mpData->GetNamedNormalSelList().InsertSet (mSet, mID, mName);
}

class AddNormalSetNameRestore : public RestoreObj {
	TSTR mName;
	DWORD mID;
	int mIndex;	// location in list
	EditNormalsMod *mpMod;
	Tab<TSTR*> *mpSets;
	Tab<DWORD> *mpIDs;

public:
	AddNormalSetNameRestore (EditNormalsMod *m, DWORD id, Tab<TSTR*> *s,Tab<DWORD> *i) :
	  mpMod(m), mpSets(s), mpIDs(i), mID(id) { }
	void Restore(int isUndo);
	void Redo();				
	TSTR Description() {return TSTR(_T("AddNormalSetName"));}
};

void AddNormalSetNameRestore::Restore(int isUndo) {
	int sct = mpSets->Count();
	for (mIndex=0; mIndex<sct; mIndex++) if ((*mpIDs)[mIndex] == mID) break;
	if (mIndex >= sct) return;

	mName = *(*mpSets)[mIndex];
	delete (*mpSets)[mIndex];
	mpSets->Delete (mIndex, 1);
	mpIDs->Delete (mIndex, 1);
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

void AddNormalSetNameRestore::Redo() {
	TSTR *nm = new TSTR(mName);
	if (mIndex < mpSets->Count()) {
		mpSets->Insert (mIndex, 1, &nm);
		mpIDs->Insert (mIndex, 1, &mID);
	} else {
		mpSets->Append (1, &nm);
		mpIDs->Append (1, &mID);
	}
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

class DeleteNormalSetRestore : public RestoreObj {
	BitArray mSet;
	DWORD mID;
	TSTR mName;
	EditNormalsModData *mpData;

public:
	DeleteNormalSetRestore (EditNormalsModData *pData, DWORD id, TSTR & name);
	void Restore(int isUndo) { mpData->GetNamedNormalSelList().InsertSet(mSet, mID, mName); }
	void Redo() { mpData->GetNamedNormalSelList().RemoveSet(mID); }
	int Size () { return sizeof(BitArray)+sizeof(TSTR)+sizeof(DWORD)+sizeof(void *); }
	TSTR Description() {return TSTR(_T("DeleteNormalSet"));}
};

DeleteNormalSetRestore::DeleteNormalSetRestore (EditNormalsModData *pData, DWORD id, TSTR & name) :
		mpData(pData), mID(id), mName(name) {
	BitArray *ptr = mpData->GetNamedNormalSelList().GetSet(id);
	if (ptr) mSet = *ptr;
}

class DeleteNormalSetNameRestore : public RestoreObj {
	TSTR mName;
	DWORD mID;
	EditNormalsMod *mpMod;
	Tab<TSTR*> *mpSets;
	Tab<DWORD> *mpIDs;

public:
	DeleteNormalSetNameRestore (Tab<TSTR*> *s, EditNormalsMod *e, Tab<DWORD> *i, DWORD id);
	void Restore(int isUndo);
	void Redo();
	int Size() { return sizeof(TSTR) + sizeof(DWORD) + 3*sizeof(void *); }
	TSTR Description() {return TSTR(_T("DeleteNormalSetName"));}
};

DeleteNormalSetNameRestore::DeleteNormalSetNameRestore (Tab<TSTR*> *s,EditNormalsMod *e,Tab<DWORD> *i, DWORD id) :
		mpSets(s), mpIDs(i), mpMod(e), mID(id) {
	for (int j=0; j<mpSets->Count(); j++) {
		if ((*mpIDs)[j]==mID) break;
	}
	if (j<mpSets->Count()) mName = *(*mpSets)[j];
}

void DeleteNormalSetNameRestore::Restore(int isUndo) {
	TSTR *nm = new TSTR(mName);
	mpSets->Append(1,&nm);
	mpIDs->Append(1,&mID);
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

void DeleteNormalSetNameRestore::Redo() {
	for (int j=0; j<mpSets->Count(); j++) {
		if ((*mpIDs)[j]==mID) break;
	}
	if (j<mpSets->Count()) {
		mpSets->Delete(j,1);
		mpIDs->Delete(j,1);
	}
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

class NormalSetNameRestore : public RestoreObj {
	TSTR mUndo, mRedo;
	DWORD mID;
	Tab<TSTR*> *mpSets;
	Tab<DWORD> *mpIDs;
	EditNormalsMod *mpMod;

public:
	NormalSetNameRestore(Tab<TSTR*> *s,EditNormalsMod *e,Tab<DWORD> *i,DWORD id);
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Set Name"));}
};


NormalSetNameRestore::NormalSetNameRestore (Tab<TSTR*> *s, EditNormalsMod *m, Tab<DWORD> *ids, DWORD id) :
		mID(id), mpIDs(ids), mpSets(s), mpMod(m) {
	for (int j=0; j<mpSets->Count(); j++) {
		if ((*mpIDs)[j]==mID) break;
	}
	if (j<mpSets->Count()) mUndo = *(*mpSets)[j];
}

void NormalSetNameRestore::Restore(int isUndo) {
	for (int j=0; j<mpSets->Count(); j++) {
		if ((*mpIDs)[j]==mID) break;
	}
	if (j<mpSets->Count()) {
		mRedo = *(*mpSets)[j];
		*(*mpSets)[j] = mUndo;
	}
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

void NormalSetNameRestore::Redo() {
	for (int j=0; j<mpSets->Count(); j++) {
		if ((*mpIDs)[j]==mID) break;
	}
	if (j<mpSets->Count()) *(*mpSets)[j] = mRedo;
	if (mpMod->GetUserInterface()) mpMod->GetUserInterface()->NamedSelSetListChanged();
}

#ifdef _DEBUG
//#define NORMAL_EDIT_RESTORE_DEBUG
#endif

class NormalEditRestore : public RestoreObj {
	MNNormalSpec *mpPolyBefore;
	MNNormalSpec *mpPolyAfter;
	MeshNormalSpec *mpTriBefore;
	MeshNormalSpec *mpTriAfter;
	EditNormalsMod *mpMod;
	EditNormalsModData *mpData;

public:
	NormalEditRestore (EditNormalsMod *pMod, EditNormalsModData *pData);
	void After ();
	void Restore (int isUndo);
	void Redo ();
	int Size() { return 4*sizeof(void *); }
	void EndHold() {mpData->ClearFlag(ENMD_NE_HELD);}
	TSTR Description() { return TSTR(_T("NormalEditRestore")); }
};

NormalEditRestore::NormalEditRestore (EditNormalsMod *pMod, EditNormalsModData *pData)
		: mpMod(pMod), mpData(pData), mpPolyBefore(NULL), mpPolyAfter(NULL),
		mpTriBefore(NULL), mpTriAfter(NULL) {
	pData->SetFlag (ENMD_NE_HELD);
	if (pData->GetFlag (ENMD_TRIMESH)) {
		mpTriBefore = new MeshNormalSpec;
		*mpTriBefore = *(pData->GetTriNormals());
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpTriBefore->MyDebugPrint (true);
#endif
	} else {
		mpPolyBefore = new MNNormalSpec;
		*mpPolyBefore = *(pData->GetPolyNormals());
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpPolyBefore->MNDebugPrint (true);
#endif
	}
}

void NormalEditRestore::After() {
	if (mpTriBefore) {
		if (mpTriAfter) return;
		mpTriAfter = new MeshNormalSpec;
		*mpTriAfter = *(mpData->GetTriNormals());
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpTriAfter->MyDebugPrint (true);
#endif
	} else {
		if (mpPolyAfter) return;
		mpPolyAfter = new MNNormalSpec;
		*mpPolyAfter = *(mpData->GetPolyNormals());
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpPolyAfter->MNDebugPrint (true);
#endif
	}
}

void NormalEditRestore::Restore (int isUndo) {
	if (mpTriBefore) {
		if (isUndo && !mpTriAfter) After ();
		*(mpData->GetTriNormals()) = *mpTriBefore;
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpData->GetTriNormals()->MNDebugPrint (true);
#endif
	} else {
		if (isUndo && !mpPolyAfter) After ();
		*(mpData->GetPolyNormals()) = *mpPolyBefore;
#ifdef NORMAL_EDIT_RESTORE_DEBUG
		mpData->GetPolyNormals()->MNDebugPrint (true);
#endif
	}
	mpMod->LocalDataChanged (PART_TOPO|PART_GEOM);
}

void NormalEditRestore::Redo () {
	if (mpTriBefore) {
		if (!mpTriAfter) return;
		*(mpData->GetTriNormals()) = *mpTriAfter;
	} else {
		if (!mpPolyAfter) return;
		*(mpData->GetPolyNormals()) = *mpPolyAfter;
	}
	mpMod->LocalDataChanged (PART_TOPO|PART_GEOM);
}

// Class Descriptor:
class EditNormalsClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return GetString(IDS_EDIT_NORMALS_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return EDIT_NORMALS_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MOD_CATEGORY);}
	const TCHAR*	InternalName() { return _T("EditNormals"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() { return hInstance; }
	int NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i);
};

static EditNormalsClassDesc theEditNormalsDesc;
ClassDesc2* GetEditNormalsDesc() {return &theEditNormalsDesc;}

// We need a dialog proc to handle buttons, etc.
class EditNormalProc : public ParamMap2UserDlgProc {
	EditNormalsMod *mpMod;
public:
	EditNormalProc () : mpMod(NULL) { }

	// Accessors
	void SetMod (EditNormalsMod *m) { mpMod=m; }

	void SetEnables (HWND hWnd, int numSelected);
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static EditNormalProc theEditNormalProc;

void EditNormalProc::SetEnables (HWND hParams, int numSelected) {
	if (!mpMod) return;

	bool subObj = (mpMod->EnfnGetSelLevel() == EN_SL_OBJECT) ? false : true;
	bool useThr = mpMod->GetUseAvgThreshold();
	int selectBy;
	mpMod->GetParamBlock(0)->GetValue (en_select_by, TimeValue(0), selectBy, FOREVER);

	EnableWindow (GetDlgItem (hParams, IDC_EN_SELECT_BY_BOX), subObj);
	EnableWindow (GetDlgItem (hParams, IDC_EN_SELBY_NORMAL), subObj);
	EnableWindow (GetDlgItem (hParams, IDC_EN_SELBY_VERTEX), subObj);
	EnableWindow (GetDlgItem (hParams, IDC_EN_SELBY_EDGE), subObj);
	EnableWindow (GetDlgItem (hParams, IDC_EN_SELBY_FACE), subObj);

	EnableWindow (GetDlgItem (hParams, IDC_EN_IGNORE_BACKFACING), subObj);

	ICustButton *but;
	but = GetICustButton (GetDlgItem (hParams, IDC_EN_COPY));
	but->Enable (subObj && (numSelected==1));
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_EN_PASTE));
	but->Enable (subObj && numSelected && mpMod->HasCopiedNormal());
	ReleaseICustButton (but);

	// CAL-03/21/03: average normals
	ICustEdit *edit;
	ISpinnerControl *spin;

	edit = GetICustEdit (GetDlgItem (hParams, IDC_EN_AVERAGE_THRESHOLD));
	edit->Enable (useThr);
	ReleaseICustEdit (edit);
	spin = GetISpinner (GetDlgItem (hParams, IDC_EN_AVERAGE_THRESHOLD_SPIN));
	spin->Enable (useThr);
	ReleaseISpinner (spin);
}

BOOL EditNormalProc::DlgProc (TimeValue t, IParamMap2 *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mpMod) return FALSE;
	ICustButton *but;
	ISpinnerControl *spin;

	switch (msg) {
	case WM_INITDIALOG:
		// CAL-03/21/03: average normals
		but = GetICustButton(GetDlgItem(hWnd, IDC_EN_TARGET_AVERAGE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		CheckDlgButton(hWnd, IDC_EN_TO_AVERAGE, mpMod->GetUnifyBreakToAvg());
		CheckDlgButton(hWnd, IDC_EN_USE_THRESHOLD, mpMod->GetUseAvgThreshold());

		spin = GetISpinner(GetDlgItem(hWnd, IDC_EN_AVERAGE_THRESHOLD_SPIN));
		spin->SetLimits(EN_AVERAGE_MIN_THRESHOLD, EN_AVERAGE_MAX_THRESHOLD, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd, IDC_EN_AVERAGE_THRESHOLD), EDITTYPE_POS_UNIVERSE);
		spin->SetValue(mpMod->GetAverageThreshold(), FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_EN_TAR_AVERAGE_BOX_SIZE_SPIN));
		spin->SetLimits(EN_MIN_PICKBOX_SIZE, EN_MAX_PICKBOX_SIZE, FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(GetDlgItem(hWnd, IDC_EN_TAR_AVERAGE_BOX_SIZE), EDITTYPE_INT);
		spin->SetValue(mpMod->GetAverageBoxSize(), FALSE);
		ReleaseISpinner(spin);
		break;
		
	case CC_SPINNER_CHANGE:
		// CAL-03/21/03: average normals
		spin = (ISpinnerControl*)lParam;

		switch (LOWORD(wParam)) {
		case IDC_EN_AVERAGE_THRESHOLD_SPIN:
			mpMod->SetAverageThreshold (spin->GetFVal());
			break;

		case IDC_EN_TAR_AVERAGE_BOX_SIZE_SPIN:
			mpMod->SetAverageBoxSize (spin->GetIVal());
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EN_BREAK:
			mpMod->BreakNormals (mpMod->GetUnifyBreakToAvg());
			break;

		case IDC_EN_UNIFY:
			mpMod->UnifyNormals (mpMod->GetUnifyBreakToAvg());
			break;

		case IDC_EN_TO_AVERAGE:			// CAL-04/09/03: unify/break to the average
			mpMod->SetUnifyBreakToAvg((IsDlgButtonChecked(hWnd, IDC_EN_TO_AVERAGE)) ? true : false);
			break;

		case IDC_EN_SPECIFY:
			mpMod->SpecifyNormals ();
			break;

		case IDC_EN_MAKE_EXPLICIT:
			mpMod->MakeNormalsExplicit ();
			break;

		case IDC_EN_RESET:
			mpMod->ResetNormals ();
			break;

		case IDC_EN_COPY:
			mpMod->CopyNormal ();
			break;

		case IDC_EN_PASTE:
			mpMod->PasteNormal ();
			break;

		case IDC_EN_AVERAGE:			// CAL-03/21/03: average normals
			mpMod->AverageNormals (mpMod->GetUseAvgThreshold(), mpMod->GetAverageThreshold());
			break;

		case IDC_EN_TARGET_AVERAGE:		// CAL-03/21/03: average normals command mode
			mpMod->ToggleAverageNormalsCMode ();
			break;

		case IDC_EN_USE_THRESHOLD:		// CAL-04/09/03: use threshold in averaging selected normals
			mpMod->SetUseAvgThreshold((IsDlgButtonChecked(hWnd, IDC_EN_USE_THRESHOLD)) ? true : false);
			
			ICustEdit *edit;
			ISpinnerControl *spin;
			bool useThr = mpMod->GetUseAvgThreshold();
			edit = GetICustEdit (GetDlgItem (hWnd, IDC_EN_AVERAGE_THRESHOLD));
			edit->Enable (useThr);
			ReleaseICustEdit (edit);
			spin = GetISpinner (GetDlgItem (hWnd, IDC_EN_AVERAGE_THRESHOLD_SPIN));
			spin->Enable (useThr);
			ReleaseISpinner (spin);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

// CAL-03/21/03: average normals mouse process
class ENAverageNormalsMouseProc : public MouseCallBack {
private:
	EditNormalsMod *mpMod;
	IObjParam *ip;
	INode *mpNode;		// target INode
	int mTarget;		// target selection id
	int mTargetBase;	// used in normal selection mode only
	IPoint2 mMouse;		// current mouse position
	bool mLineDrawn;	// average line has been drawn

protected:
	HitRecord* HitTest (ViewExp *vpt, IPoint2 &m);
	bool GetTargetCenter(Point3 &center);
	bool SetTargetBase();
	void DrawAverageLine (HWND hWnd);

public:
	ENAverageNormalsMouseProc (EditNormalsMod* mod, IObjParam *i) :
	  mpMod(mod), ip(i), mpNode(NULL), mLineDrawn(false) { }
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

HitRecord *ENAverageNormalsMouseProc::HitTest (ViewExp *vpt, IPoint2 &m) {
	vpt->ClearSubObjHitList();
	mpMod->SetPickBoxSize(mpMod->GetAverageBoxSize());
#ifdef EN_AVERAGE_AT_ALL_SUBOBJECT_LEVELS
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
#else
	mpMod->SetHitLevelOverride(EN_SL_NORMAL);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpMod->ClearHitLevelOverride();
#endif
	mpMod->SetPickBoxSize(EN_DEF_PICKBOX_SIZE);
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (mpNode && mpNode == hr->nodeRef && mTarget == hr->hitInfo) return NULL;

	return hr;
}

bool ENAverageNormalsMouseProc::GetTargetCenter(Point3 &center)
{
	if (!ip || !mpMod || !mpNode || mTarget < 0) return false;
	
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++) if (nodes[i] == mpNode) break;
	nodes.DisposeTemporary();
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	int fid, vid;
	float dispLength;
	Point3 *normVect = NULL;
#ifdef EN_AVERAGE_AT_ALL_SUBOBJECT_LEVELS
	int hitLev = mpMod->EnfnGetSelLevel();
#else
	int hitLev = EN_SL_NORMAL;
#endif
	if (pData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pData->GetTriMeshCache();
		if (!pMesh) return false;

		switch (hitLev) {
		case EN_SL_OBJECT:
			return false;
		case EN_SL_NORMAL:
			if (mTargetBase < 0 || mTargetBase >= pMesh->numVerts) return false;
			normVect = pData->EnfnGetNormal(mTarget, 0);
			if (normVect == NULL) return false;
			dispLength = mpMod->GetParamBlock(0)->GetFloat (en_display_length);
			center = pMesh->verts[mTargetBase] + (*normVect * dispLength);
			break;
		case EN_SL_VERTEX:
			if (mTarget >= pMesh->numVerts) return false;
			center = pMesh->verts[mTarget];
			break;
		case EN_SL_EDGE:
			if (mTarget >= 3 * pMesh->numFaces) return false;
			fid = mTarget/3;
			vid = mTarget%3;
			center = (pMesh->verts[pMesh->faces[fid].v[vid]] + pMesh->verts[pMesh->faces[fid].v[(vid+1)%3]]) * 0.5f;
			break;
		case EN_SL_FACE:
			if (mTarget >= pMesh->numFaces) return false;
			center = Point3::Origin;
			for (vid = 0; vid < 3; vid++)
				center += pMesh->verts[pMesh->faces[mTarget].v[vid]];
			center /= 3.0f;
			break;
		}
	} else {
		MNMesh *pMesh = pData->GetPolyMeshCache();
		if (!pMesh) return false;
		
		switch (hitLev) {
		case EN_SL_OBJECT:
			return false;
		case EN_SL_NORMAL:
			if (mTargetBase < 0 || mTargetBase >= pMesh->numv) return false;
			normVect = pData->EnfnGetNormal(mTarget, 0);
			if (normVect == NULL) return false;
			dispLength = mpMod->GetParamBlock(0)->GetFloat (en_display_length);
			center = pMesh->P(mTargetBase) + (*normVect * dispLength);
			break;
		case EN_SL_VERTEX:
			if (mTarget >= pMesh->numv) return false;
			center = pMesh->P(mTarget);
			break;
		case EN_SL_EDGE:
			if (mTarget >= pMesh->nume) return false;
			center = (pMesh->P(pMesh->E(mTarget)->v1) + pMesh->P(pMesh->E(mTarget)->v2)) * 0.5f;
			break;
		case EN_SL_FACE:
			if (mTarget >= pMesh->numf) return false;
			center = Point3::Origin;
			for (vid = 0; vid < pMesh->F(mTarget)->deg; vid++)
				center += pMesh->P(pMesh->F(mTarget)->vtx[vid]);
			center /= (float)pMesh->F(mTarget)->deg;
			break;
		}
	}
	return true;
}

bool ENAverageNormalsMouseProc::SetTargetBase()
{
	if (!ip || !mpMod || !mpNode || mTarget < 0) return false;

#ifdef EN_AVERAGE_AT_ALL_SUBOBJECT_LEVELS
	if (mpMod->EnfnGetSelLevel() != EN_SL_NORMAL) return false;		// no need to set target base
#endif

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++) if (nodes[i] == mpNode) break;
	nodes.DisposeTemporary();
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (pData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pData->GetTriMeshCache();
		if (!pMesh) return false;
		MeshNormalSpec *pNorm = pData->GetTriNormals();
		if (!pNorm) return false;

		// find the first matched normal ID and set target base to the corresponding vertex ID
		int max = pNorm->GetNumFaces();
		if (pMesh->numFaces<max) max = pMesh->numFaces;
		for (int i=0; i<max; i++) {
			for (int j=0; j<3; j++) {
				if (pNorm->Face(i).GetNormalID(j) == mTarget) {
					mTargetBase = pMesh->faces[i].v[j];
					return true;
				}
			}
		}
	} else {
		MNMesh *pMesh = pData->GetPolyMeshCache();
		if (!pMesh) return false;
		MNNormalSpec *pNorm = pData->GetPolyNormals();
		if (!pNorm) return false;

		// find the first matched normal ID and set target base to the corresponding vertex ID
		int max = pNorm->GetNumFaces();
		if (pMesh->numf<max) max = pMesh->numf;
		for (int i=0; i<max; i++) {
			if (pMesh->f[i].GetFlag (MN_DEAD)) continue;
			int maxDeg = pNorm->Face(i).GetDegree();
			if (pMesh->f[i].deg < maxDeg) maxDeg = pMesh->f[i].deg;
			for (int j=0; j<maxDeg; j++) {
				if (pNorm->Face(i).GetNormalID(j) == mTarget) {
					mTargetBase = pMesh->f[i].vtx[j];
					return true;
				}
			}
		}
	}
	return false;
}

void ENAverageNormalsMouseProc::DrawAverageLine(HWND hWnd)
{
	if (!ip || !mpNode) return;

	Point3 p;
	IPoint3 sp;
	if (!GetTargetCenter(p)) return;
	
	Matrix3 objToWorldTM = mpNode->GetObjectTM(ip->GetTime());
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	ip->ReleaseViewport(vpt);
	gw->setTransform(objToWorldTM);

	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));
	gw->wTransPoint(&p, &sp);
	MoveToEx(hdc, sp.x, sp.y, NULL);
	LineTo(hdc, mMouse.x, mMouse.y);
	DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

int ENAverageNormalsMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m)
{
	ViewExp *vpt = NULL;
	HitRecord *hr = NULL;
	int res = TRUE;

	switch (msg) {
	case MOUSE_ABORT:
		if (mpNode) {
			if (mLineDrawn) DrawAverageLine(hwnd);		// erase the previous line
			mpNode = NULL;
		}
		res = FALSE;
		break;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		res = FALSE;
		break;

	case MOUSE_DBLCLICK:
		res = FALSE;
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		if ((hr = HitTest (vpt, m)) != NULL) {
			if (mpNode == NULL) {		// start the averaging process
				mpNode = hr->nodeRef;
				mTarget = hr->hitInfo;
				SetTargetBase();
				mMouse = m;
				mLineDrawn = false;
			} else {					// finish the averaging process
				if (mLineDrawn) DrawAverageLine(hwnd);		// erase the previous line
				theHold.Begin();
				if (mpMod->EnfnAverageTwoNormals (mpNode, mTarget, hr->nodeRef, hr->hitInfo)) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.AverageTwo"), 4, 0,
						mr_reftarg, mpNode, mr_int, mTarget+1, mr_reftarg, hr->nodeRef, mr_int, hr->hitInfo+1);
					macroRecorder->EmitScript();
					theHold.Accept (GetString (IDS_EN_AVERAGE));
				} else
					theHold.Cancel();
				mpNode = NULL;
				ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
			}
		}
		if (mpNode == NULL) res = FALSE;
		break;

	case MOUSE_MOVE:
		if (mpNode) {
			if (mLineDrawn) DrawAverageLine(hwnd);			// erase the previous line
			mMouse = m;
			DrawAverageLine(hwnd);
			mLineDrawn = true;
		}
		// continue to the next case to set cursor.

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		if (HitTest (vpt, m))
			SetCursor (ip->GetSysCursor(SYSCUR_SELECT));
		else
			SetCursor (LoadCursor (NULL, IDC_ARROW));
		break;
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return res;
}

// CAL-03/21/03: average normals command mode
#define CID_EN_AVERAGENORMALS	CID_USER+0x1100

class ENAverageNormalsCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	ENAverageNormalsMouseProc mproc;
	EditNormalsMod *mpMod;

public:
	ENAverageNormalsCMode (EditNormalsMod* mod, IObjParam *i) :
	  fgProc(mod), mproc(mod, i), mpMod(mod) { }
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EN_AVERAGENORMALS; }
	MouseCallBack *MouseProc (int *numPoints) { *numPoints=999999; return &mproc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG (CommandMode *oldMode) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

void ENAverageNormalsCMode::EnterMode() {
	if (!mpMod) return;
	IParamBlock2 *pBlock = mpMod->GetParamBlock(0);
	if (!pBlock) return;
	IParamMap2 *pMap = pBlock->GetMap();
	if (!pMap) return;
	HWND hParams = pMap->GetHWnd();
	if (!hParams) return;
	
	ICustButton *but = GetICustButton (GetDlgItem(hParams, IDC_EN_TARGET_AVERAGE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ENAverageNormalsCMode::ExitMode() {
	if (!mpMod) return;
	IParamBlock2 *pBlock = mpMod->GetParamBlock(0);
	if (!pBlock) return;
	IParamMap2 *pMap = pBlock->GetMap();
	if (!pMap) return;
	HWND hParams = pMap->GetHWnd();
	if (!hParams) return;
	
	ICustButton *but = GetICustButton (GetDlgItem(hParams, IDC_EN_TARGET_AVERAGE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// Parameter block IDs:
// Blocks themselves:
enum { en_pblock };
// Parameter maps:
enum { en_map_main };

// Parameters in block defined in iEditNormals.h

static ParamBlockDesc2 edit_normals_desc ( en_pblock,
									_T("editNormals"),
									IDS_PARAMETERS, &theEditNormalsDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI,
									REF_PBLOCK,
	// Rollout description:
	IDD_EDIT_NORMALS, IDS_PARAMETERS, 0, 0, NULL, 

	// params
	en_select_by, _T("selectBy"), TYPE_INT, 0, IDS_EN_SELECT_BY,
		p_default, 0,
		p_ui, TYPE_RADIO, 4, IDC_EN_SELBY_NORMAL, IDC_EN_SELBY_VERTEX,
			IDC_EN_SELBY_EDGE, IDC_EN_SELBY_FACE,
		end,

	en_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, 0, IDS_EN_IGNORE_BACKFACING,
		p_default, false,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EN_IGNORE_BACKFACING,
		end,

	en_show_handles, _T("showHandles"), TYPE_BOOL, 0, IDS_EN_SHOW_HANDLES,
		p_default, false,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EN_SHOW_HANDLES,
		end,

	en_display_length, _T("displayLength"), TYPE_WORLD, 0, IDS_EN_DISPLAY_LENGTH,
		p_default, 10.0f,
		p_range, 0.0f, 9999999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_EN_LENGTH, IDC_EN_LENGTH_SPIN, SPIN_AUTOSCALE,
		end,

	end
);

// Function Publishing Interface description:
static FPInterfaceDesc editNormalsModInterfaceDesc (EDIT_NORMALS_MOD_INTERFACE,
													_T("EditNormalsMod"), IDS_EN_EDIT_NORMALS_MODIFIER,
													&theEditNormalsDesc, FP_MIXIN,
	enfn_move, _T("Move"), 0, TYPE_bool, 0, 1,
		_T("offset"), 0, TYPE_POINT3_BR,
	enfn_rotate, _T("Rotate"), 0, TYPE_bool, 0, 1,
		_T("rotation"), 0, TYPE_QUAT_BR,

	enfn_break, _T("Break"), 0, TYPE_bool, 0, 3,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
		_T("toAverage"), 0, TYPE_bool, f_keyArgDefault, false,
	enfn_unify, _T("Unify"), 0, TYPE_bool, 0, 3,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
		_T("toAverage"), 0, TYPE_bool, f_keyArgDefault, false,
	enfn_reset, _T("Reset"), 0, TYPE_bool, 0, 2,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_specify, _T("Specify"), 0, TYPE_bool, 0, 2,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_make_explicit, _T("MakeExplicit"), 0, TYPE_bool, 0, 2,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_copy, _T("Copy"), 0, TYPE_bool, 0, 2,
		_T("normalID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_paste, _T("Paste"), 0, TYPE_bool, 0, 2,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_average, _T("Average"), 0, TYPE_bool, 0, 4,				// CAL-03/21/03:
		_T("useThresh"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("threshold"), 0, TYPE_FLOAT, f_keyArgDefault, 0.0f,
		_T("selection"), 0, TYPE_BITARRAY, f_keyArgDefault, NULL,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_average_global, _T("AverageGlobal"), 0, TYPE_bool, 0, 2,	// CAL-03/21/03:
		_T("useThresh"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("threshold"), 0, TYPE_FLOAT, f_keyArgDefault, 0.0f,
	enfn_average_two, _T("AverageTwo"), 0, TYPE_bool, 0, 4,			// CAL-03/21/03:
		_T("node1"), 0, TYPE_INODE,
		_T("normalID1"), 0, TYPE_INDEX,
		_T("node2"), 0, TYPE_INODE,
		_T("normalID2"), 0, TYPE_INDEX,

	enfn_get_selection, _T("GetSelection"), 0, TYPE_BITARRAY, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_set_selection, _T("SetSelection"), 0, TYPE_bool, 0, 2,
		_T("newSelection"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_select, _T("Select"), 0, TYPE_bool, 0, 4,
		_T("newSelection"), 0, TYPE_BITARRAY_BR,
		_T("invert"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_convert_vertex_selection, _T("ConvertVertexSelection"), 0, TYPE_VOID, 0, 3,
		_T("vertexSelection"), 0, TYPE_BITARRAY_BR,
		_T("normalSelection"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_convert_edge_selection, _T("ConvertEdgeSelection"), 0, TYPE_VOID, 0, 3,
		_T("edgeSelection"), 0, TYPE_BITARRAY_BR,
		_T("normalSelection"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_convert_face_selection, _T("ConvertFaceSelection"), 0, TYPE_VOID, 0, 3,
		_T("faceSelection"), 0, TYPE_BITARRAY_BR,
		_T("normalSelection"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

		// Access to the normals themselves
	enfn_get_num_normals, _T("GetNumNormals"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_normal, _T("GetNormal"), 0, TYPE_POINT3, 0, 2,
		_T("normalIndex"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_set_normal, _T("SetNormal"), 0, TYPE_VOID, 0, 3,
		_T("normalIndex"), 0, TYPE_INDEX,
		_T("normalValue"), 0, TYPE_POINT3_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_normal_explicit, _T("GetNormalExplicit"), 0, TYPE_bool, 0, 2,
		_T("normalIndex"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_set_normal_explicit, _T("SetNormalExplicit"), 0, TYPE_VOID, 0, 3,
		_T("normalIndex"), 0, TYPE_INDEX,
		_T("explicit"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

		// Access to the normal faces
	enfn_get_num_faces, _T("GetNumFaces"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_face_degree, _T("GetFaceDegree"), 0, TYPE_INT, 0, 2,
		_T("face"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_normal_id, _T("GetNormalID"), 0, TYPE_INDEX, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_set_normal_id, _T("SetNormalID"), 0, TYPE_VOID, 0, 4,
		_T("face"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("normalID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_face_normal_specified, _T("GetFaceNormalSpecified"), 0, TYPE_bool, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_set_face_normal_specified, _T("SetFaceNormalSpecified"), 0, TYPE_VOID, 0, 4,
		_T("face"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("specified"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	enfn_get_num_vertices, _T("GetNumVertices"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_vertex_id, _T("GetVertexID"), 0, TYPE_INDEX, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_vertex, _T("GetVertex"), 0, TYPE_POINT3, 0, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	enfn_get_num_edges, _T("GetNumEdges"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_edge_id, _T("GetEdgeID"), 0, TYPE_INDEX, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_face_edge_side, _T("GetFaceEdgeSide"), 0, TYPE_INDEX, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("edge"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_edge_vertex, _T("GetEdgeVertex"), 0, TYPE_INDEX, 0, 3,
		_T("edge"), 0, TYPE_INDEX,
		_T("end"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_edge_face, _T("GetEdgeFace"), 0, TYPE_INDEX, 0, 3,
		_T("edge"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_get_edge_normal, _T("GetEdgeNormal"), 0, TYPE_INDEX, 0, 4,
		_T("edge"), 0, TYPE_INDEX,
		_T("end"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	enfn_rebuild_normals, _T("RebuildNormals"), 0, TYPE_VOID, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	enfn_recompute_normals, _T("RecomputeNormals"), 0, TYPE_VOID, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	properties,
	enfn_get_sel_level, enfn_set_sel_level, _T("SelLevel"), IDS_EN_SEL_LEVEL,
		TYPE_ENUM, enprop_sel_level,

	enums,
		enprop_sel_level, 5,
			"Object", EN_SL_OBJECT,
			"Normal", EN_SL_NORMAL,
			"Vertex", EN_SL_VERTEX,
			"Edge", EN_SL_EDGE,
			"Face", EN_SL_FACE,

	end
);

FPInterfaceDesc *IEditNormalsMod::GetDesc () {
	return &editNormalsModInterfaceDesc;
}

void *EditNormalsClassDesc::Create (BOOL loading) {
	AddInterface (&editNormalsModInterfaceDesc);
	return new EditNormalsMod;
}

//--- Accelerator table for Edit Normals -------------------

const ActionTableId kEditNormalsActionID = 0x2993eaf;

class EditNormalsActionCB : public ActionCallback {
	EditNormalsMod *mpMod;
public:
	EditNormalsActionCB (EditNormalsMod *m) : mpMod(m) { }
	BOOL ExecuteAction(int id);
};

BOOL EditNormalsActionCB::ExecuteAction (int id) {
	switch (id) {
	case ID_EN_OBJECT_LEVEL:
		mpMod->EnfnSetSelLevel(EN_SL_OBJECT);
		return true;

	case ID_EN_NORMAL_LEVEL:
		mpMod->EnfnSetSelLevel(EN_SL_NORMAL);
		return true;

	case ID_EN_VERTEX_LEVEL:
		mpMod->EnfnSetSelLevel(EN_SL_VERTEX);
		return true;

	case ID_EN_EDGE_LEVEL:
		mpMod->EnfnSetSelLevel(EN_SL_EDGE);
		return true;

	case ID_EN_FACE_LEVEL:
		mpMod->EnfnSetSelLevel(EN_SL_FACE);
		return true;

	case ID_EN_BREAK:
		if (!mpMod->BreakNormals (mpMod->GetUnifyBreakToAvg())) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_UNIFY:
		if (!mpMod->UnifyNormals (mpMod->GetUnifyBreakToAvg())) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_RESET:
		if (!mpMod->ResetNormals ()) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_SPECIFY:
		if (!mpMod->SpecifyNormals ()) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_MAKE_EXPLICIT:
		if (!mpMod->MakeNormalsExplicit ()) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_COPY:
		if (!mpMod->CopyNormal()) return false;
		return true;

	case ID_EN_PASTE:
		if (!mpMod->PasteNormal()) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_AVERAGE:			// CAL-03/21/03:
		// TODO: set shortcut for ID_EN_AVERAGE
		if (!mpMod->AverageNormals (mpMod->GetUseAvgThreshold(), mpMod->GetAverageThreshold())) return false;
		GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime());
		return true;

	case ID_EN_AVERAGE_MODE:		// CAL-03/21/03:
		// TODO: set shortcut for ID_EN_AVERAGE_MODE
		if (mpMod->EnfnGetSelLevel() == EN_SL_OBJECT) break;
		mpMod->ToggleAverageNormalsCMode ();
		return true;
	}
	return false;
}

const int kNumEditNormalsActions = 14;

static ActionDescription editNormalsActions[] = {
	ID_EN_OBJECT_LEVEL,
		IDS_OBJECT_LEVEL,
		IDS_OBJECT_LEVEL,
		IDS_EN_EDIT_NORMALS,

	ID_EN_NORMAL_LEVEL,
		IDS_EN_NORMAL_LEVEL,
		IDS_EN_NORMAL_LEVEL,
		IDS_EN_EDIT_NORMALS,

	ID_EN_VERTEX_LEVEL,
		IDS_VERTEX_LEVEL,
		IDS_VERTEX_LEVEL,
		IDS_EN_EDIT_NORMALS,

	ID_EN_EDGE_LEVEL,
		IDS_EDGE_LEVEL,
		IDS_EDGE_LEVEL,
		IDS_EN_EDIT_NORMALS,

	ID_EN_FACE_LEVEL,
		IDS_FACE_LEVEL,
		IDS_FACE_LEVEL,
		IDS_EN_EDIT_NORMALS,

	ID_EN_BREAK,
		IDS_EN_BREAK,
		IDS_EN_BREAK,
		IDS_EN_EDIT_NORMALS,

	ID_EN_UNIFY,
		IDS_EN_UNIFY,
		IDS_EN_UNIFY,
		IDS_EN_EDIT_NORMALS,

	ID_EN_RESET,
		IDS_EN_RESET,
		IDS_EN_RESET,
		IDS_EN_EDIT_NORMALS,

	ID_EN_SPECIFY,
		IDS_EN_SPECIFY,
		IDS_EN_SPECIFY,
		IDS_EN_EDIT_NORMALS,

	ID_EN_MAKE_EXPLICIT,
		IDS_EN_MAKE_EXPLICIT,
		IDS_EN_MAKE_EXPLICIT,
		IDS_EN_EDIT_NORMALS,

	ID_EN_COPY,
		IDS_EN_COPY,
		IDS_EN_COPY,
		IDS_EN_EDIT_NORMALS,

	ID_EN_PASTE,
		IDS_EN_PASTE,
		IDS_EN_PASTE,
		IDS_EN_EDIT_NORMALS,

	ID_EN_AVERAGE,			// CAL-03/21/03:
		IDS_EN_AVERAGE,
		IDS_EN_AVERAGE,
		IDS_EN_EDIT_NORMALS,

	ID_EN_AVERAGE_MODE,		// CAL-03/21/03:
		IDS_EN_AVERAGE_MODE,
		IDS_EN_AVERAGE_MODE,
		IDS_EN_EDIT_NORMALS,
};

ActionTable* GetEditNormalsActions() {
	TSTR name = GetString (IDS_EN_EDIT_NORMALS);
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_ACCEL_EDIT_NORMALS));
	ActionTable* pTab;
	pTab = new ActionTable (kEditNormalsActionID, kEditNormalsActionID,
		name, hAccel, kNumEditNormalsActions, editNormalsActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kEditNormalsActionID, name.data());

	return pTab;
}

ActionTable* EditNormalsClassDesc::GetActionTable(int i) {
	return GetEditNormalsActions ();
}

//--- EditNormalsMod methods -------------------------------

EditNormalsMod::EditNormalsMod() : mSelLevel(EN_SL_NORMAL), mpParamBlock(NULL), mShowEndResult(false), mpCopiedNormal(NULL), mHitLevelOverride(0) {
	theEditNormalsDesc.MakeAutoParamBlocks(this);
}

int EditNormalsMod::EnfnGetSelLevel () {
	if (mSelLevel == EN_SL_OBJECT) return mSelLevel;
	int selBy;
	mpParamBlock->GetValue (en_select_by, TimeValue(0), selBy, FOREVER);
	switch (selBy) {
	case 1: return EN_SL_VERTEX;
	case 2: return EN_SL_EDGE;
	case 3: return EN_SL_FACE;
	}
	return EN_SL_NORMAL;
}

void EditNormalsMod::EnfnSetSelLevel (int selLevel) {
	int desiredLevel = selLevel ? EN_SL_NORMAL : EN_SL_OBJECT;
	if (selLevel) {
		int selBy = selLevel - 1;
		mpParamBlock->SetValue (en_select_by, TimeValue(0), selBy);
	}
	if (mSelLevel != desiredLevel) {
		if (mpInterface) mpInterface->SetSubObjectLevel (desiredLevel);
		else mSelLevel=desiredLevel;
	}
}

RefTargetHandle EditNormalsMod::Clone(RemapDir& remap) {
	EditNormalsMod *mod = new EditNormalsMod();
	mod->mSelLevel = mSelLevel;
	if (mpCopiedNormal) {
		// NOTE: safe because initialized to NULL in EditNormalsMod() - otherwise we'd check for existing allocation.
		mod->mpCopiedNormal = new Point3(*mpCopiedNormal);
	}
	mod->ReplaceReference (0, mpParamBlock->Clone(remap));

	BaseClone(this, mod, remap);
	return mod;
}

void EditNormalsMod::ModifyPolyObject (TimeValue t, EditNormalsModData *pData, PolyObject *pobj) {
	if (pobj->mm.numv == 0) return;
	if (pobj->mm.numf == 0) return;

	if (!pData->GetPolyNormals()) {
		// Make sure we're in Poly mode:
		pData->SetFlag (ENMD_TRIMESH, false);

		// First time called - need to set up our local normals!
		pData->AllocatePolyNormals ();

		// Copy from normals in pipeline, if present:
		if (pobj->GetMesh().GetSpecifiedNormals())
			pData->GetPolyNormals()->CopySpecified (*(pobj->GetMesh().GetSpecifiedNormals()));

		// Note that normals need rebuilding:
		pData->InvalidateTopoCache();
	}

	if (!pData->GetTopoValid (t)) {
		pData->GetPolyNormals()->ClearFlag (MNNORMAL_NORMALS_BUILT);

		// Fill in unspecified normals:
		pData->GetPolyNormals()->SetParent (&(pobj->GetMesh()));
		pData->GetPolyNormals()->CheckNormals ();
		pData->GetPolyNormals()->SetParent (NULL);	// the pipeline mesh should NOT be remembered as our local normal cache's "parent".

		pData->SetTopoValid (pobj->ChannelValidity(t, TOPO_CHAN_NUM));
		pData->SetGeomValid (pobj->ChannelValidity(t, GEOM_CHAN_NUM));

		// Dialog may need updating (num selected, etc.)
		UpdateDialog ();
	}

	if (!pData->GetGeomValid (t)) {
		DbgAssert (pData->GetPolyNormals()->GetFlag (MNNORMAL_NORMALS_BUILT));
		pData->GetPolyNormals()->ClearFlag (MNNORMAL_NORMALS_COMPUTED);

		// Fill in unspecified normals:
		pData->GetPolyNormals()->SetParent (&(pobj->GetMesh()));
		pData->GetPolyNormals()->CheckNormals ();
		pData->GetPolyNormals()->SetParent (NULL);	// the pipeline mesh should NOT be remembered as our local normal cache's "parent".

		pData->SetGeomValid (pobj->ChannelValidity(t, GEOM_CHAN_NUM));
	}

	DbgAssert (pData->GetPolyNormals()->GetFlag (MNNORMAL_NORMALS_BUILT));
	DbgAssert (pData->GetPolyNormals()->GetFlag (MNNORMAL_NORMALS_COMPUTED));

	// Copy our normals over to pipeline mesh:
	pobj->GetMesh().SpecifyNormals ();
	if (!pobj->GetMesh().GetSpecifiedNormals()) {
		DbgAssert (0);
		return;
	}
	*(pobj->GetMesh().GetSpecifiedNormals()) = *(pData->GetPolyNormals());
	pobj->GetMesh().GetSpecifiedNormals()->SetParent(&(pobj->GetMesh()));
	pobj->GetMesh().selLevel = MNM_SL_OBJECT;
	pobj->GetMesh().ClearDispFlag (0x8f);

	// And store map-free, normal-free copy of resultant mesh for display, hit-testing:
	pData->AllocPolyMeshCache();
	pData->GetPolyMeshCache()->CopyBasics (pobj->GetMesh(), true);

	// Finally, add this XTC object so the normals will be cleared if a modifier
	// changes the geometry or topology without updating the normals.
	// (Note: This object may already exist, in which case we don't need to add it.)
	for (int i=0; i<pobj->NumXTCObjects(); i++)
	{
		if (pobj->GetXTCObject(i)->ExtensionID() == kTriObjNormalXTCID) break;
	}
	if (i>=pobj->NumXTCObjects())
		pobj->AddXTCObject (new TriObjectNormalXTC());
}

void EditNormalsMod::ModifyTriObject (TimeValue t, EditNormalsModData *pData, TriObject *tobj) {
	if (tobj->mesh.numVerts == 0) return;
	if (tobj->mesh.numFaces == 0) return;

	if (!pData->GetTriNormals()) {
		// Make sure we're in Tri mode:
		pData->SetFlag (ENMD_TRIMESH, true);

		// First time called - need to set up our local normals!
		pData->AllocateTriNormals ();

		// Copy from normals in pipeline, if present:
		MeshNormalSpec *pPipeNormals = (MeshNormalSpec *) tobj->GetMesh().GetInterface (MESH_NORMAL_SPEC_INTERFACE);
		if (pPipeNormals) pData->GetTriNormals()->CopySpecified (*(pPipeNormals));

		// Note that normals need rebuilding:
		pData->InvalidateTopoCache();
	}

	if (!pData->GetTopoValid (t)) {
		pData->GetTriNormals()->ClearFlag (MESH_NORMAL_NORMALS_BUILT);

		// Fill in unspecified normals:
		pData->GetTriNormals()->SetParent (&(tobj->GetMesh()));
		pData->GetTriNormals()->CheckNormals ();
		pData->GetTriNormals()->SetParent (NULL);	// the pipeline mesh should NOT be remembered as our local normal cache's "parent".

		pData->SetTopoValid (tobj->ChannelValidity(t, TOPO_CHAN_NUM));
		pData->SetGeomValid (tobj->ChannelValidity(t, GEOM_CHAN_NUM));

		// Dialog may need updating (num selected, etc.)
		UpdateDialog ();
	}

	if (!pData->GetGeomValid (t)) {
		DbgAssert (pData->GetTriNormals()->GetFlag (MESH_NORMAL_NORMALS_BUILT));
		pData->GetTriNormals()->ClearFlag (MESH_NORMAL_NORMALS_COMPUTED);

		// Fill in unspecified normals:
		pData->GetTriNormals()->SetParent (&(tobj->GetMesh()));
		pData->GetTriNormals()->CheckNormals ();
		pData->GetTriNormals()->SetParent (NULL);	// the pipeline mesh should NOT be remembered as our local normal cache's "parent".

		pData->SetGeomValid (tobj->ChannelValidity(t, GEOM_CHAN_NUM));
	}

	DbgAssert (pData->GetTriNormals()->GetFlag (MESH_NORMAL_NORMALS_BUILT));
	DbgAssert (pData->GetTriNormals()->GetFlag (MESH_NORMAL_NORMALS_COMPUTED));

	// Copy our normals over to pipeline mesh:
	MeshNormalSpec *pPipeNormals = (MeshNormalSpec *) tobj->GetMesh().GetInterface (MESH_NORMAL_SPEC_INTERFACE);
	if (pPipeNormals) {
		*(pPipeNormals) = *(pData->GetTriNormals());
		pPipeNormals->SetParent(&(tobj->GetMesh()));
	}

	// Make the pipeline object object-level.
	tobj->GetMesh().selLevel = MESH_OBJECT;
	tobj->GetMesh().ClearDispFlag (0x8f);

	// And store map-free, normal-free copy of resultant mesh for display, hit-testing:
	pData->AllocTriMeshCache();
	pData->GetTriMeshCache()->CopyBasics(tobj->GetMesh());

	// Finally, add this XTC object so the normals will be cleared if a modifier
	// changes the geometry or topology without updating the normals.
	// (Note: This object may already exist, in which case we don't need to add it.)
	for (int i=0; i<tobj->NumXTCObjects(); i++)
	{
		if (tobj->GetXTCObject(i)->ExtensionID() == kTriObjNormalXTCID) break;
	}
	if (i>=tobj->NumXTCObjects())
		tobj->AddXTCObject (new TriObjectNormalXTC());

	// And clarify that _this_ modifier supports the edited normals:
	if (pPipeNormals) pPipeNormals->SetFlag(MESH_NORMAL_MODIFIER_SUPPORT);
}

void EditNormalsMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	EditNormalsModData *pData  = (EditNormalsModData*) mc.localData;
	if (!pData) {
		mc.localData = pData = new EditNormalsModData;
	}

	PolyObject *pobj=NULL;
	TriObject *tobj=NULL;
	bool needsDelete(false);
	if (os->obj->IsSubClassOf(polyObjectClassID)) pobj = (PolyObject*)os->obj;
	else {
		if (os->obj->IsSubClassOf(triObjectClassID)) tobj = (TriObject*)os->obj;
		else {
			if (os->obj->CanConvertToType (triObjectClassID)) {
				tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
				if (tobj != os->obj) needsDelete = true;
			}
		}
	}

	if (pobj) ModifyPolyObject (t, pData, pobj);
	if (tobj) ModifyTriObject (t, pData, tobj);
	if (needsDelete) {
		os->obj = tobj;
		os->obj->UnlockObject ();
	}
}

Interval EditNormalsMod::LocalValidity(TimeValue t) 
{ 
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return FOREVER; // No local animatables - for now.
}	

class ENVertexNormalsCallback : public VertexNormalsCallback {
	EditNormalsMod *mpMod;
public:
	ENVertexNormalsCallback () : mpMod(NULL) { }
	void SetUseFaceAngles (bool value);
	void SetModifier (EditNormalsMod *pMod) { mpMod = pMod; }
};

static ENVertexNormalsCallback theENVertexNormalsCallback;

void ENVertexNormalsCallback::SetUseFaceAngles (bool value) {
	if (!mpMod) return;
	mpMod->InvalidateComputedNormals ();
}

void EditNormalsMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mpInterface = ip;

	UpdateSetNames ();

	// Use our classdesc2 to put up our parammap2 maps:
	theEditNormalsDesc.BeginEditParams(ip, this, flags, prev);

	// Add our dialog proc:
	theEditNormalProc.SetMod (this);
	theEditNormalsDesc.SetUserDlgProc (&edit_normals_desc, en_map_main, &theEditNormalProc);

	// Create our SO modes:
	mpSelectMode = new SelectModBoxCMode (this, ip);
	mpMoveMode = new MoveModBoxCMode (this, ip);
	mpRotateMode = new RotateModBoxCMode (this, ip);

	// CAL-03/21/03: Create average normals command mode
	mpAverageNormalsCMode = new ENAverageNormalsCMode(this, ip);

	// Update our UI:
	UpdateDialog ();

	// Add our accelerator table (keyboard shortcuts)
	mpEditNormalsActions = new EditNormalsActionCB(this);
	ip->GetActionManager()->ActivateActionTable (mpEditNormalsActions, kEditNormalsActionID);

	// Notify pipeline we're editing this modifier.
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	// Turn on SO mode if appropriate:
	if (mSelLevel) ip->SetSubObjectLevel(mSelLevel);

	// Register a callback so we know if the user turns "Legacy R4 Vertex Normals" on or off:
	theENVertexNormalsCallback.SetModifier (this);
	GetVertexNormalsControl()->RegisterCallback (&theENVertexNormalsCallback);

	// Set show end result.
	// NOTE that this generates a ModifyObject call
	mOldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	if (mOldShowEnd != mShowEndResult) ip->SetShowEndResult (mShowEndResult);
}

void EditNormalsMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	// Let classdesc2 handle the parammap2 maps:
	theEditNormalsDesc.EndEditParams (ip, this, flags, next);
	theEditNormalProc.SetMod (NULL);

	// Deactivate our keyboard shortcuts
	if (mpEditNormalsActions) {
		ip->GetActionManager()->DeactivateActionTable (mpEditNormalsActions, kEditNormalsActionID);
		delete mpEditNormalsActions;
		mpEditNormalsActions = NULL;
	}

	// Unregister our callback:
	theENVertexNormalsCallback.SetModifier (NULL);
	GetVertexNormalsControl()->UnregisterCallback (&theENVertexNormalsCallback);

	// CAL-03/21/03: Remove average normals command mode
	if (mpAverageNormalsCMode) {
		ip->DeleteMode(mpAverageNormalsCMode);
		delete mpAverageNormalsCMode;
		mpAverageNormalsCMode = NULL;
	}

	// Remove our SO modes:
	if (mpSelectMode) {
		ip->DeleteMode(mpSelectMode);
		delete mpSelectMode;
		mpSelectMode = NULL;
	}
	if (mpMoveMode) {
		ip->DeleteMode(mpMoveMode);
		delete mpMoveMode;
		mpMoveMode = NULL;
	}
	if (mpRotateMode) {
		ip->DeleteMode(mpRotateMode);
		delete mpRotateMode;
		mpRotateMode = NULL;
	}

	mpInterface = NULL;

	// Notify pipeline we're all done here.
	TimeValue t = ip->GetTime();
	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// Reset show end result (NOTE That this generates a ModifyObject call!)
	mShowEndResult = ip->GetShowEndResult() ? true : false;
	if (mShowEndResult != mOldShowEnd) ip->SetShowEndResult(mOldShowEnd);
}

int EditNormalsMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	if (!CheckNodeSelection (mpInterface, inode)) return false;
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;

	int ignoreBackfacing;
	mpParamBlock->GetValue (en_ignore_backfacing, t, ignoreBackfacing, FOREVER);

	// Setup GW
	MakeHitRegion(hr,type, crossing,mPickBoxSize,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

	// Note that currently, there's no way for normals to respect this setting.
	if (ignoreBackfacing) gw->setRndLimits (gw->getRndLimits() | GW_BACKCULL);
	else gw->setRndLimits (gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();

	SubObjHitList hitList;

	// Make sure we're using the correct display length:
	float dispLength;
	int showHandles;
	mpParamBlock->GetValue (en_display_length, t, dispLength, FOREVER);
	mpParamBlock->GetValue (en_show_handles, t, showHandles, FOREVER);

	// Verify we have a cached mesh to hit-test against:
	if (!mc->localData) return 0;
	EditNormalsModData *pData = (EditNormalsModData *) mc->localData;
	MNMesh *pMesh = pData->GetPolyMeshCache();
	MNNormalSpec *pNorm = pData->GetPolyNormals();
	int hitLev = (mHitLevelOverride != 0) ? mHitLevelOverride : EnfnGetSelLevel();
	if (pMesh && pNorm) {
		switch (hitLev) {
		case EN_SL_OBJECT:
			res = 0;	// shouldn't happen, anyway.
			break;
		case EN_SL_NORMAL:
			pNorm->SetDisplayLength(dispLength);
			pNorm->SetParent (pMesh);
			pNorm->SetFlag (MNNORMAL_DISPLAY_HANDLES, showHandles?true:false);
			pNorm->HitTest (gw, &hr, flags, hitList);
			pNorm->SetParent (NULL);
			break;
		case EN_SL_VERTEX:
			res = pMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_MNVERTS, hitList);
			break;
		case EN_SL_EDGE:
			res = pMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_MNEDGES, hitList);
			break;
		case EN_SL_FACE:
			res = pMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_MNFACES, hitList);
			break;
		}
	} else {
		Mesh *pTriMesh = pData->GetTriMeshCache();
		MeshNormalSpec *pTriNorm = pData->GetTriNormals();
		if (!pTriMesh || !pTriNorm) return 0;

		switch (hitLev) {
		case EN_SL_OBJECT:
			res = 0;	// shouldn't happen, anyway.
			break;
		case EN_SL_NORMAL:
			pTriNorm->SetDisplayLength(dispLength);
			pTriNorm->SetParent (pTriMesh);
			pTriNorm->SetFlag (MESH_NORMAL_DISPLAY_HANDLES, showHandles?true:false);
			pTriNorm->HitTest (gw, &hr, flags, hitList);
			pTriNorm->SetParent (NULL);
			break;
		case EN_SL_VERTEX:
			res = pTriMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_VERTS, hitList);
			break;
		case EN_SL_EDGE:
			res = pTriMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_EDGES, hitList);
			break;
		case EN_SL_FACE:
			res = pTriMesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|SUBHIT_FACES, hitList);
			break;
		}
	}

	MeshSubHitRec *rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int EditNormalsMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!CheckNodeSelection (mpInterface, inode)) return 0;
	// Verify we have a cached mesh to display:
	if (!mc->localData) return 0;
	EditNormalsModData *pData = (EditNormalsModData *) mc->localData;

	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	// Make sure we're using the correct display length:
	float dispLength;
	int showHandles;
	mpParamBlock->GetValue (en_display_length, t, dispLength, FOREVER);
	mpParamBlock->GetValue (en_show_handles, t, showHandles, FOREVER);

	if (pData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pData->GetTriMeshCache();
		if (!pMesh) return 0;

		// Draw Normals here:
		if (pData->GetTriNormals()) {
			pData->GetTriNormals()->SetDisplayLength(dispLength);
			pData->GetTriNormals()->SetParent (pMesh);
			pData->GetTriNormals()->SetFlag (MESH_NORMAL_DISPLAY_HANDLES, showHandles?true:false);
			pData->GetTriNormals()->Display (gw, mSelLevel ? true : false);
			pData->GetTriNormals()->SetParent (NULL);
		}

		if (mpInterface && mpInterface->GetShowEndResult ()) {
			// We need to draw a "gizmo" version of the mesh:
			Point3 colSel=GetSubSelColor();
			Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
			Point3 colGiz=GetUIColor(COLOR_GIZMOS);
			Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
			gw->setColor (LINE_COLOR, colGiz);

			Point3 rp[3];
			int i;
			int es[3];
			int drawSelLevel = EnfnGetSelLevel();
			for (i=0; i<pMesh->numFaces; i++) {
				if (pMesh->faces[i].Hidden()) continue;
				for (int j=0; j<3; j++) {
					if (pMesh->faces[i].getEdgeVis (j)) {
						es[j] = GW_EDGE_VIS;
					} else {
						if (drawSelLevel < EN_SL_EDGE) continue;
						es[j] = GW_EDGE_INVIS;
					}
					rp[0] = pMesh->verts[pMesh->faces[i].v[j]];
					rp[1] = pMesh->verts[pMesh->faces[i].v[(j+1)%3]];
					gw->polyline (2, rp, NULL, NULL, FALSE, es);
				}
			}
			if (drawSelLevel == EN_SL_VERTEX) {
				for (i=0; i<pMesh->numVerts; i++) {
					if (pMesh->vertHide[i]) continue;
					gw->setColor (LINE_COLOR, colTicks);
					if(getUseVertexDots()) gw->marker (&(pMesh->verts[i]), VERTEX_DOT_MARKER(getVertexDotType()));
					else gw->marker (&(pMesh->verts[i]), PLUS_SIGN_MRKR);
				}
			}
		}
	} else {
		MNMesh *pMesh = pData->GetPolyMeshCache();
		if (!pMesh) return 0;

		// Draw Normals here:
		if (pData->GetPolyNormals()) {
			pData->GetPolyNormals()->SetDisplayLength(dispLength);
			pData->GetPolyNormals()->SetParent (pMesh);
			pData->GetPolyNormals()->SetFlag (MNNORMAL_DISPLAY_HANDLES, showHandles?true:false);
			pData->GetPolyNormals()->Display (gw, mSelLevel ? true : false);
			pData->GetPolyNormals()->SetParent (NULL);
		}

		if (mpInterface && mpInterface->GetShowEndResult ()) {
			// We need to draw a "gizmo" version of the mesh:
			Point3 colSel=GetSubSelColor();
			Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
			Point3 colGiz=GetUIColor(COLOR_GIZMOS);
			Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
			gw->setColor (LINE_COLOR, colGiz);

			Point3 rp[3];
			int i;
			int es[3];
			int drawSelLevel = EnfnGetSelLevel();
			for (i=0; i<pMesh->nume; i++) {
				if (pMesh->e[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
				if (!pMesh->e[i].GetFlag (MN_EDGE_INVIS)) {
					es[0] = GW_EDGE_VIS;
				} else {
					if (drawSelLevel < EN_SL_EDGE) continue;
					es[0] = GW_EDGE_INVIS;
				}
				rp[0] = pMesh->v[pMesh->e[i].v1].p;
				rp[1] = pMesh->v[pMesh->e[i].v2].p;
				gw->polyline (2, rp, NULL, NULL, FALSE, es);
			}
			if (drawSelLevel == EN_SL_VERTEX) {
				for (i=0; i<pMesh->numv; i++) {
					if (pMesh->v[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
					gw->setColor (LINE_COLOR, colTicks);
					if(getUseVertexDots()) gw->marker (&(pMesh->P(i)), VERTEX_DOT_MARKER(getVertexDotType()));
					else gw->marker (&(pMesh->P(i)), PLUS_SIGN_MRKR);
				}
			}
		}
	}

	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditNormalsMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!CheckNodeSelection (mpInterface, inode)) return;
	if (!mc->localData) return;
	EditNormalsModData *pData = (EditNormalsModData *) mc->localData;
	Matrix3 tm = inode->GetObjectTM(t);

	// Make sure we're using the correct display length:
	float dispLength;
	int showHandles;
	mpParamBlock->GetValue (en_display_length, t, dispLength, FOREVER);
	mpParamBlock->GetValue (en_show_handles, t, showHandles, FOREVER);

	if (pData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pData->GetTriMeshCache ();
		if (!pMesh) return;
		if (pData->GetTriNormals()) {
			pData->GetTriNormals()->SetParent (pMesh);
			pData->GetTriNormals()->SetDisplayLength(dispLength);
			pData->GetTriNormals()->SetFlag (MESH_NORMAL_DISPLAY_HANDLES, showHandles?true:false);
			box = pData->GetTriNormals()->GetBoundingBox (&tm);
			pData->GetTriNormals()->SetParent (NULL);
		} else {
			box.Init();
		}
		if (mpInterface && mpInterface->GetShowEndResult()) box += pMesh->getBoundingBox (&tm);
	} else {
		MNMesh *pMesh = pData->GetPolyMeshCache();
		if (!pMesh) return;
		if (pData->GetPolyNormals()) {
			pData->GetPolyNormals()->SetParent (pMesh);
			pData->GetPolyNormals()->SetDisplayLength(dispLength);
			pData->GetPolyNormals()->SetFlag (MNNORMAL_DISPLAY_HANDLES, showHandles?true:false);
			box = pData->GetPolyNormals()->GetBoundingBox (&tm);
			pData->GetPolyNormals()->SetParent (NULL);
		} else {
			box.Init();
		}
		if (mpInterface && mpInterface->GetShowEndResult()) box += pMesh->getBoundingBox (&tm);
	}
}

// Finds a single center for the center of the normal BBox.
void EditNormalsMod::GetSubObjectCenters (SubObjAxisCallback *pCB,TimeValue t,INode *pNode,ModContext *pMC) {
	if (!pMC->localData) return;
	if (mSelLevel == EN_SL_OBJECT) return;	// shouldn't happen.

	EditNormalsModData *pModData = (EditNormalsModData *) pMC->localData;
	Matrix3 tm = pNode->GetObjectTM(t);

	Box3 box;
	box.Init();
	if (pModData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pModData->GetTriMeshCache();
		if (!pMesh) return;
		MeshNormalSpec *pNorm = pModData->GetTriNormals();
		if (!pNorm) return;
		if (!pNorm->GetSelection().NumberSet()) return;

		// Set up the bounding box of the _bases_ of the selected normals.
		int max = pNorm->GetNumFaces();
		if (pMesh->numFaces<max) max = pMesh->numFaces;
		for (int i=0; i<max; i++) {
			for (int j=0; j<3; j++) {
				if (pNorm->Face(i).GetNormalID(j) < 0) continue;
				if (!pNorm->GetSelection()[pNorm->Face(i).GetNormalID(j)]) continue;
				box += tm*pMesh->verts[pMesh->faces[i].v[j]];
			}
		}
	} else {
		MNMesh *pMesh = pModData->GetPolyMeshCache();
		if (!pMesh) return;
		MNNormalSpec *pNorm = pModData->GetPolyNormals();
		if (!pNorm) return;
		if (!pNorm->GetSelection().NumberSet()) return;

		// Set up the bounding box of the _bases_ of the selected normals.
		int max = pNorm->GetNumFaces();
		if (pMesh->numf<max) max = pMesh->numf;
		for (int i=0; i<max; i++) {
			if (pMesh->f[i].GetFlag (MN_DEAD)) continue;
			int maxDeg = pNorm->Face(i).GetDegree();
			if (pMesh->f[i].deg < maxDeg) maxDeg = pMesh->f[i].deg;
			for (int j=0; j<maxDeg; j++) {
				if (pNorm->Face(i).GetNormalID(j) < 0) continue;
				if (!pNorm->GetSelection()[pNorm->Face(i).GetNormalID(j)]) continue;
				box += tm*pMesh->P(pMesh->f[i].vtx[j]);
			}
		}
	}
	if (box.IsEmpty()) return;
	Point3 center = (box.pmin + box.pmax) * .5f;
	pCB->Center (center, 0);
}

// This is used only for "local" coordinate space (I believe).
// Users might want to rotate or move in this space, so...
void EditNormalsMod::GetSubObjectTMs (SubObjAxisCallback *pCB,TimeValue t,INode *pNode,ModContext *pMC) {
	if (!pMC->localData) return;
	if (mSelLevel == EN_SL_OBJECT) return;	// shouldn't happen.
	EditNormalsModData *pModData = (EditNormalsModData *) pMC->localData;
	Matrix3 otm = pNode->GetObjectTM(t);

	if (pModData->GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = pModData->GetTriMeshCache();
		if (!pMesh) return;
		MeshNormalSpec *pNorm = pModData->GetTriNormals();
		if (!pNorm) return;
		if (!pNorm->GetSelection().NumberSet()) return;

		for (int i=0; i<pMesh->numFaces; i++) {
			for (int j=0; j<3; j++) {
				int normID = pNorm->Face(i).GetNormalID(j);
				if (normID<0) continue;
				if (!pNorm->GetSelection()[normID]) continue;

				Matrix3 tm;
				Point3 n = VectorTransform (otm, pNorm->Normal(normID));
				float lenSq = LengthSquared(n);
				if (lenSq && (lenSq!=1.f)) n /= Sqrt(lenSq);
				MatrixFromNormal (n, tm);
				tm.SetTrans (pMesh->verts[pMesh->faces[i].v[j]] * otm);
				pCB->TM(tm,i);
			}
		}
	} else {
		MNMesh *pMesh = pModData->GetPolyMeshCache();
		if (!pMesh) return;
		MNNormalSpec *pNorm = pModData->GetPolyNormals();
		if (!pNorm) return;
		if (!pNorm->GetSelection().NumberSet()) return;

		for (int i=0; i<pMesh->numf; i++) {
			if (pMesh->f[i].GetFlag(MN_DEAD)) continue;

			for (int j=0; j<pMesh->f[i].deg; j++) {
				int normID = pNorm->Face(i).GetNormalID(j);
				if (normID<0) continue;
				if (!pNorm->GetSelection()[normID]) continue;

				Matrix3 tm;
				Point3 n = VectorTransform (otm, pNorm->Normal(normID));
				float lenSq = LengthSquared(n);
				if (lenSq && (lenSq!=1.f)) n /= Sqrt(lenSq);
				MatrixFromNormal (n, tm);
				tm.SetTrans (pMesh->P(pMesh->f[i].vtx[j]) * otm);
				pCB->TM(tm,i);
			}
		}
	}
}

void EditNormalsMod::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	mSelLevel = level;

	// Fill in modes with our sub-object modes
	if (level!=EN_SL_OBJECT) {
		modes = XFormModes (mpMoveMode, mpRotateMode, NULL, NULL, NULL, mpSelectMode);
	}

	// Update UI
	UpdateDialog ();

	// Setup named selection sets
	SetupNamedSelDropDown();

	// Doesn't actually affect the SELECT or SUBSEL_TYPE channels, just the display.
	NotifyDependents(FOREVER, DISP_ATTRIB_CHANNEL, REFMSG_CHANGE);
	if (mpInterface) mpInterface->PipeSelLevelChanged();
	NotifyDependents(FOREVER, DISP_ATTRIB_CHANNEL, REFMSG_CHANGE);
}

void EditNormalsMod::SelectSubComponent (HitRecord *pFirstHit, BOOL selected, BOOL all, BOOL invert) {
	EditNormalsModData *pData = NULL;

	bool multiNodes = true;
	if (mpInterface) {
		mpInterface->ClearCurNamedSelSet();

		// Find out if this modifier is instanced across multiple nodes (for best macroRecorder output)
		ModContextList list;
		INodeTab nodes;
		mpInterface->GetModContexts(list,nodes);
		multiNodes = (list.Count()>1);
		nodes.DisposeTemporary();
	}

	HitRecord *pHitRec;

	// Assemble various selection level hits into bitarray of hit normals:
	switch (EnfnGetSelLevel()) {
	case EN_SL_NORMAL:
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditNormalsModData *) pHitRec->modContext->localData;
			if (!pData) continue;
			if (!pData->GetNewSelection()) pData->SetupNewSelection();
			if (!pData->GetNewSelection()) {
				DbgAssert(0);
				continue;
			}

			pData->GetNewSelection()->Set (pHitRec->hitInfo, true);
			if (!all) break;
		}
		break;

	case EN_SL_VERTEX:
		// Store info from face hits in the localmoddata's "new selection":
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditNormalsModData *) pHitRec->modContext->localData;
			if (!pData) break;	// shouldn't happen
			if (!pData->GetNewSelection()) pData->SetupNewSelection();
			DbgAssert (pData->GetNewSelection());

			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache ();
				if (!pMesh || !pMesh->faces) continue;
				// STEVE: We need a vertex-face cache for this purpose.  It should be created
				// whenever the TriMeshCache is stored.

				for (int i=0; i<pMesh->numFaces; i++) {
					Face &face = pMesh->faces[i];
					for (int j=0; j<3; j++) {
						if (face.v[j] != pHitRec->hitInfo) continue;
						int normID = pData->GetTriNormals()->Face(i).GetNormalID(j);
						if (normID>-1) pData->GetNewSelection()->Set (normID);
					}
				}
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh || !pMesh->vfac) continue;

				for (int i=0; i<pMesh->vfac[pHitRec->hitInfo].Count(); i++) {
					int face = pMesh->vfac[pHitRec->hitInfo][i];
					for (int corner=0; corner<pMesh->f[face].deg; corner++) {
						if (pMesh->f[face].vtx[corner] != pHitRec->hitInfo) continue;
						int normID = pData->GetPolyNormals()->Face(face).GetNormalID(corner);
						if (normID>-1) pData->GetNewSelection()->Set (normID);
					}
				}
			}

			if (!all) break;
		}
		break;

	case EN_SL_EDGE:
		// Store info from face hits in the localmoddata's "new selection":
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditNormalsModData *) pHitRec->modContext->localData;
			if (!pData) break;	// shouldn't happen
			if (!pData->GetNewSelection()) pData->SetupNewSelection();
			DbgAssert (pData->GetNewSelection());

			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache ();
				if (!pMesh || !pMesh->faces) continue;
				// STEVE: Need edge adjacency information to select normals on both sides of edge.
				int edge = pHitRec->hitInfo;
				int face = edge/3;
				edge = edge%3;
				int normID = pData->GetTriNormals()->Face(face).GetNormalID (edge);
				if (normID > -1) pData->GetNewSelection()->Set (normID);
				normID = pData->GetTriNormals()->Face(face).GetNormalID ((edge+1)%3);
				if (normID > -1) pData->GetNewSelection()->Set (normID);
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh || !pMesh->e) continue;

				int edge = pHitRec->hitInfo;
				for (int side=0; side<2; side++) {
					int face = side ? pMesh->e[edge].f2 : pMesh->e[edge].f1;
					int edgeIndex = pMesh->f[face].EdgeIndex (edge, pMesh->e[edge][side]);
					int normID = pData->GetPolyNormals()->Face(face).GetNormalID(edgeIndex);
					if (normID>-1) pData->GetNewSelection()->Set (normID);
					normID = pData->GetPolyNormals()->Face(face).GetNormalID
						((edgeIndex+1)%pData->GetPolyNormals()->Face(face).GetDegree());
					if (normID>-1) pData->GetNewSelection()->Set (normID);
				}
			}

			if (!all) break;
		}
		break;

	case EN_SL_FACE:
		// Store info from face hits in the localmoddata's "new selection":
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditNormalsModData *) pHitRec->modContext->localData;
			if (!pData) break;	// shouldn't happen
			if (!pData->GetNewSelection()) pData->SetupNewSelection();
			DbgAssert (pData->GetNewSelection());

			if (pData->GetFlag (ENMD_TRIMESH)) {
				MeshNormalFace & normFace = pData->GetTriNormals()->Face (pHitRec->hitInfo);
				for (int corner=0; corner<3; corner++) {
					int normID = normFace.GetNormalID (corner);
					if (normID > -1) pData->GetNewSelection()->Set (normID);
				}
			} else {
				MNNormalFace & normFace = pData->GetPolyNormals()->Face(pHitRec->hitInfo);
				for (int corner=0; corner<normFace.GetDegree(); corner++) {
					int normID = normFace.GetNormalID(corner);
					if (normID>-1) pData->GetNewSelection()->Set (normID);
				}
			}
			if (!all) break;
		}
		break;
	}

	// Apply the "new selections" to all the local mod datas:
	// (Must be after assembling all hits so we don't over-invert normals.)
	bool changeOccurred = false;
	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
		pData = (EditNormalsModData *) pHitRec->modContext->localData;
		if (!pData->GetNewSelection()) continue;
		if (pData->GetNewSelection()->NumberSet() == 0) continue;

		// Spit out the quickest script possible for this action.
		if (multiNodes) {
			if (invert) {
				// Node and Invert values are non-default
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
					1, 2, mr_bitarray, pData->GetNewSelection(), _T("invert"), mr_bool, true,
					_T("node"), mr_reftarg, pHitRec->nodeRef);
			} else {
				if (selected) {
					// Node value is non-default
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
						1, 1, mr_bitarray, pData->GetNewSelection(), _T("node"), mr_reftarg, pHitRec->nodeRef);
				} else {
					// Node and Selected values are non-default
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
						1, 2, mr_bitarray, pData->GetNewSelection(), _T("select"), mr_bool, false,
						_T("node"), mr_reftarg, pHitRec->nodeRef);
				}
			}
		} else {
			if (invert) {
				// Invert value is non-default
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
					1, 1, mr_bitarray, pData->GetNewSelection(), _T("invert"), mr_bool, true);
			} else {
				if (selected) {
					// all values are default
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
						1, 0, mr_bitarray, pData->GetNewSelection());
				} else {
					// Selected value is non-default
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Select"),
						1, 1, mr_bitarray, pData->GetNewSelection(), _T("select"), mr_bool, false);
				}
			}
		}
		macroRecorder->EmitScript ();

		if (pData->ApplyNewSelection(true, invert?true:false, selected?true:false)) {
			changeOccurred = true;
		}
		if (!all) break;
	}

	if (changeOccurred) {
		if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
		LocalDataChanged (PART_SELECT);
	}
}

void EditNormalsMod::ClearSelection (int selLevel) {
	if (selLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection ();
		pData->GetNewSelection()->ClearAll ();
		if (pData->ApplyNewSelection()) {
			if (list.Count() > 1) {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 1, mr_bitarray, &(pData->GetSelection()), _T("node"), mr_reftarg, nodes[i]);
			} else {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 0, mr_bitarray, &(pData->GetSelection()));
			}
			macroRecorder->EmitScript ();
			changeOccurred = true;
		}
	}
	nodes.DisposeTemporary();

	if (changeOccurred) {
		if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
		LocalDataChanged (PART_SELECT);
	}
}

void EditNormalsMod::SelectAll(int selLevel) {
	if (selLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection ();
		pData->GetNewSelection()->SetAll ();
		if (pData->ApplyNewSelection()) {
			if (list.Count() > 1) {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 1, mr_bitarray, &(pData->GetSelection()), _T("node"), mr_reftarg, nodes[i]);
			} else {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 0, mr_bitarray, &(pData->GetSelection()));
			}
			macroRecorder->EmitScript ();
			changeOccurred = true;
		}
	}

	nodes.DisposeTemporary();

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
	LocalDataChanged (PART_SELECT);
}

void EditNormalsMod::InvertSelection(int selLevel) {
	if (selLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection ();
		*(pData->GetNewSelection()) = ~(pData->GetSelection());
		if (pData->ApplyNewSelection()) {
			if (list.Count() > 1) {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 1, mr_bitarray, &(pData->GetSelection()), _T("node"), mr_reftarg, nodes[i]);
			} else {
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
					1, 0, mr_bitarray, &(pData->GetSelection()));
			}
			macroRecorder->EmitScript ();
			changeOccurred = true;
		}
	}

	nodes.DisposeTemporary();

	if (changeOccurred) {
		if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
		LocalDataChanged (PART_SELECT);
	}
}

void EditNormalsMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *pMC) {
	if (!pMC->localData) return;
	if (!(partID & (PART_TOPO|PART_GEOM))) return;

	EditNormalsModData *pData = (EditNormalsModData *) pMC->localData;
	pData->InvalidateGeomCache ();
	if (partID & PART_TOPO) pData->InvalidateTopoCache ();
}

void EditNormalsMod::ActivateSubSelSet(TSTR &setName) {
	if (!mpInterface) return;

	int index = FindSet (setName);	
	if (index<0 || !mpInterface) return;

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	bool changeOccurred = false;

	theHold.Begin ();
	theHold.Put (new NormalSelectNotify(this));

	for (int i = 0; i < mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;

		BitArray *pNamedSet = pData->GetNamedNormalSelList().GetSet(mNamedSelIDs[index]);
		if (pNamedSet) {
			// Verify set has correct size for current MNNormalSpec
			if (pData->GetFlag (ENMD_TRIMESH)) {
				if (pNamedSet->GetSize() != pData->GetTriNormals()->GetNumNormals()) {
					pNamedSet->SetSize (pData->GetTriNormals()->GetNumNormals(), true);
				}
			} else {
				if (pNamedSet->GetSize() != pData->GetPolyNormals()->GetNumNormals()) {
					pNamedSet->SetSize(pData->GetPolyNormals()->GetNumNormals(),TRUE);
				}
			}
			if (pData->EnfnSelect (*pNamedSet)) {
				if (mcList.Count() > 1) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
						1, 1, mr_bitarray, pNamedSet, _T("node"), mr_reftarg, nodes[i]);
				} else {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection"),
						1, 0, mr_bitarray, pNamedSet);
				}
				macroRecorder->EmitScript();
				changeOccurred = true;
			}
		}
	}
	
	nodes.DisposeTemporary();

	if (changeOccurred) {
		theHold.Put (new NormalSelectNotify(this));
		LocalDataChanged (PART_SELECT);
		theHold.Accept (GetString (IDS_SELECT));
		mpInterface->RedrawViews(mpInterface->GetTime());
	} else {
		theHold.Cancel ();
	}
}

void EditNormalsMod::NewSetFromCurSel(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int index = FindSet(setName);
	if (index<0) {
		id = AddSet(setName);
		if (theHold.Holding()) theHold.Put (new AddNormalSetNameRestore (this, id, &mNamedSelNames, &mNamedSelIDs));
	} else id = mNamedSelIDs[index];

	mpInterface->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;
		
		BitArray *set = pData->GetNamedNormalSelList().GetSet(id);
		if ((index>=0) && set) {
			*set = pData->GetSelection();
		} else {
			pData->GetNamedNormalSelList().InsertSet (pData->GetSelection(), id, setName);
			if (theHold.Holding()) theHold.Put (new AddNormalSetRestore (pData, id, setName));
		}
	}	
	nodes.DisposeTemporary();
}

void EditNormalsMod::RemoveSubSelSet(TSTR &setName) {
	int index = FindSet (setName);
	if (index<0 || !mpInterface) return;		

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	DWORD id = mNamedSelIDs[index];

	for (int i = 0; i < mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;		

		if (theHold.Holding()) theHold.Put(new DeleteNormalSetRestore(pData,id, setName));
		pData->GetNamedNormalSelList().RemoveSet(id);
	}
	
	if (theHold.Holding()) theHold.Put(new DeleteNormalSetNameRestore(&mNamedSelNames,this,&(mNamedSelIDs),id));
	RemoveSet (setName);
	mpInterface->ClearCurNamedSelSet();
	nodes.DisposeTemporary();
}

void EditNormalsMod::SetupNamedSelDropDown() {
	if (mSelLevel == EN_SL_OBJECT) return;
	mpInterface->ClearSubObjectNamedSelSets();
	for (int i=0; i<mNamedSelNames.Count(); i++)
		mpInterface->AppendSubObjectNamedSelSet(*mNamedSelNames[i]);
	UpdateNamedSelDropDown ();
}

int EditNormalsMod::NumNamedSelSets() {
	return mNamedSelNames.Count();
}

TSTR EditNormalsMod::GetNamedSelSetName(int i) {
	return *mNamedSelNames[i];
}

void EditNormalsMod::SetNamedSelSetName(int i,TSTR &newName) {
	if (theHold.Holding()) theHold.Put(new NormalSetNameRestore(&mNamedSelNames,this,&mNamedSelIDs,mNamedSelIDs[i]));
	*mNamedSelNames[i] = newName;
}

void EditNormalsMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	DWORD id = AddSet(newName);
	if (theHold.Holding())
		theHold.Put(new AddNormalSetNameRestore(this, id, &mNamedSelNames, &mNamedSelIDs));

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;
	
		BitArray bits;
		GenericNamedSelSetList & namedSelList = pData->GetNamedNormalSelList();
		bits = namedSelList[sets[0]];

		for (int i=1; i<sets.Count(); i++) {
			switch (op) {
			case NEWSET_MERGE:
				bits |= namedSelList[sets[i]];
				break;

			case NEWSET_INTERSECTION:
				bits &= namedSelList[sets[i]];
				break;

			case NEWSET_SUBTRACT:
				bits &= ~(namedSelList[sets[i]]);
				break;
			}
		}
		namedSelList.InsertSet (bits, id, newName);
		if (theHold.Holding()) theHold.Put(new AddNormalSetRestore(pData, id, newName));
	}
}

int EditNormalsMod::FindSet(TSTR &setName) {
	for (int i=0; i<mNamedSelNames.Count(); i++) {
		if (setName == *mNamedSelNames[i]) return i;
	}
	return -1;
}

DWORD EditNormalsMod::AddSet(TSTR &setName) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	int nsCount = mNamedSelNames.Count();

	// Find an empty id to assign to this set.
	BOOL found = FALSE;
	while (!found) {
		found = TRUE;
		for (int i=0; i<mNamedSelIDs.Count(); i++) {
			if (mNamedSelIDs[i]!=id) continue;
			id++;
			found = FALSE;
			break;
		}
	}

	// Find location in alphabetized list:
	for (int pos=0; pos<nsCount; pos++) if (setName < *(mNamedSelNames[pos])) break;
	if (pos == nsCount) {
		mNamedSelNames.Append(1,&name);
		mNamedSelIDs.Append(1,&id);
	} else {
		mNamedSelNames.Insert (pos, 1, &name);
		mNamedSelIDs.Insert (pos, 1, &id);
	}

	return id;
}

void EditNormalsMod::RemoveSet(TSTR &setName) {
	int i = FindSet(setName);
	if (i<0) return;
	delete mNamedSelNames[i];
	mNamedSelNames.Delete(i,1);
	mNamedSelIDs.Delete(i,1);
}

void EditNormalsMod::UpdateSetNames () {
	if (!mpInterface) return;

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	for (int i=0; i<mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if ( !pData ) continue;

		// Make sure the namedSel array is in alpha order.
		// (Crude bubble sort since we expect that it will be.)
		int j, k, kmax = mNamedSelNames.Count();
		for (k=1; k<kmax; k++) {
			if (*(mNamedSelNames[k-1]) < *(mNamedSelNames[k])) continue;
			for (j=0; j<k-1; j++) {
				if (*(mNamedSelNames[j]) > *(mNamedSelNames[k])) break;
			}
			// j now represents the point at which k should be inserted.
			TSTR *hold = mNamedSelNames[k];
			DWORD dhold = mNamedSelIDs[k];
			int j2;
			for (j2=k; j2>j; j2--) {
				mNamedSelNames[j2] = mNamedSelNames[j2-1];
				mNamedSelIDs[j2] = mNamedSelIDs[j2-1];
			}
			mNamedSelNames[j] = hold;
			mNamedSelIDs[j] = dhold;
		}

		GenericNamedSelSetList & gnsl = pData->GetNamedNormalSelList();
		// Check for old, unnamed or misnamed sets with ids.
		for (k=0; k<gnsl.Count(); k++) {
			for (j=0; j<mNamedSelIDs.Count(); j++) if (mNamedSelIDs[j] == gnsl.ids[k]) break;
			if (j == mNamedSelIDs.Count()) continue;
			if (gnsl.names[k] && !(*(gnsl.names[k]) == *(mNamedSelNames[j]))) {
				delete gnsl.names[k];
				gnsl.names[k] = NULL;
			}
			if (gnsl.names[k]) continue;
			gnsl.names[k] = new TSTR(*(mNamedSelNames[j]));
		}
		gnsl.Alphabetize ();

		// Now check lists against each other, adding any missing elements.
		for (j=0; j<gnsl.Count(); j++) {
			if (*(gnsl.names[j]) == *(mNamedSelNames[j])) continue;
			if (j>= mNamedSelNames.Count()) {
				TSTR *nname = new TSTR(*gnsl.names[j]);
				DWORD nid = gnsl.ids[j];
				mNamedSelNames.Append (1, &nname);
				mNamedSelIDs.Append (1, &nid);
				continue;
			}
			if (*(gnsl.names[j]) > *(mNamedSelNames[j])) {
				BitArray baTemp;
				gnsl.InsertSet (j, baTemp, mNamedSelIDs[j], *(mNamedSelNames[j]));
				continue;
			}
			// Otherwise:
			TSTR *nname = new TSTR(*gnsl.names[j]);
			DWORD nid = gnsl.ids[j];
			mNamedSelNames.Insert (j, 1, &nname);
			mNamedSelIDs.Insert (j, 1, &nid);
		}
		for (; j<mNamedSelNames.Count(); j++) {
			BitArray baTemp;
			gnsl.AppendSet (baTemp, mNamedSelIDs[j], *(mNamedSelNames[j]));
		}
	}

	nodes.DisposeTemporary();
}

void EditNormalsMod::ClearSetNames() {
	for (int j=0; j<mNamedSelNames.Count(); j++) {
		delete mNamedSelNames[j];
		mNamedSelNames[j] = NULL;
	}
}

void EditNormalsMod::UpdateNamedSelDropDown () {
	if (!mpInterface) return;
	if (mSelLevel == EN_SL_OBJECT) {
		mpInterface->ClearCurNamedSelSet ();
		return;
	}

	if (mNamedSelNames.Count() == 0) return;	// no sets to match.

	// See if this selection matches a named set
	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts (mcList, nodes);
	BitArray nselmatch;
	nselmatch.SetSize (mNamedSelNames.Count());
	nselmatch.SetAll ();
	int nd, i, foundone=FALSE;
	for (nd=0; nd<mcList.Count(); nd++) {
		EditNormalsModData *pData = (EditNormalsModData *) mcList[nd]->localData;
		if (!pData) continue;
		foundone = TRUE;

		for (i=0; i<nselmatch.GetSize(); i++) {
			if (!nselmatch[i]) continue;
			if (!(*(pData->GetNamedNormalSelList().sets[i]) == pData->GetSelection())) nselmatch.Clear(i);
		}

		if (nselmatch.NumberSet () == 0) break;
	}
	nodes.DisposeTemporary();

	if (foundone && nselmatch.NumberSet ()) {
		for (i=0; i<nselmatch.GetSize(); i++) if (nselmatch[i]) break;
		mpInterface->SetCurNamedSelSet (*(mNamedSelNames[i]));
	} else mpInterface->ClearCurNamedSelSet ();
}

int EditNormalsMod::NumSubObjTypes() {
	return 1;
}

ISubObjType *EditNormalsMod::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Normal.SetName (GetString(IDS_EN_NORMAL));
	}

	switch(i)
	{
	case -1:
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
		break;
	case 0:
		return &SOT_Normal;
	}
	return NULL;
}

// Chunks for saving/loading EditNormalsMod:
const USHORT kModChunkSelLevel = 0x0100;
const USHORT kModChunkNamedSel = 0x0140;
const USHORT kNamedSelChunkString = 0x0144;
const USHORT kNamedSelChunkID = 0x0148;

IOResult EditNormalsMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(kModChunkSelLevel);
	res = isave->Write(&mSelLevel, sizeof(mSelLevel), &nb);
	isave->EndChunk();

	if (mNamedSelNames.Count()) {
		isave->BeginChunk (kModChunkNamedSel);
		for (int i=0; i<mNamedSelNames.Count(); i++) {
			isave->BeginChunk(kNamedSelChunkString);
			isave->WriteWString(*mNamedSelNames[i]);
			isave->EndChunk();

			isave->BeginChunk(kNamedSelChunkID);
			isave->Write(mNamedSelIDs.Addr(i),sizeof(DWORD),&nb);
			isave->EndChunk();
		}
		isave->EndChunk();
	}

	return res;
}

IOResult EditNormalsMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;

	Modifier::Load(iload);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kModChunkSelLevel:
			iload->Read(&mSelLevel, sizeof(mSelLevel), &nb);
			break;

		case kModChunkNamedSel:
			res = LoadNamedSelChunk(iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

IOResult EditNormalsMod::LoadNamedSelChunk(ILoad *iload) {
	IOResult res;
	DWORD ix=0;
	ULONG nb;
	TCHAR *name;
	TSTR *newName;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kNamedSelChunkString:
			res = iload->ReadWStringChunk(&name);
			newName = new TSTR(name);
			mNamedSelNames.Append(1,&newName);				
			mNamedSelIDs.Append(1,&ix);
			ix++;
			break;

		case kNamedSelChunkID:
			iload->Read(&mNamedSelIDs[mNamedSelIDs.Count()-1],sizeof(DWORD), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

// Chunks for saving/loading local data:
const USHORT kDataChunkNamedSet = 0x0200;
const USHORT kDataChunkPolyNormals = 0x0240;
const USHORT kDataChunkTriNormals = 0x0250;
const USHORT kDataChunkFlags = 0x0260;

IOResult EditNormalsMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	EditNormalsModData *pData = (EditNormalsModData*)ld;

	if (pData->GetNamedNormalSelList().Count()) {
		isave->BeginChunk(kDataChunkNamedSet);
		pData->GetNamedNormalSelList().Save(isave);
		isave->EndChunk();
	}

	if (pData->GetPolyNormals()) {
		isave->BeginChunk (kDataChunkPolyNormals);
		pData->GetPolyNormals()->Save (isave);
		isave->EndChunk();
	}

	if (pData->GetTriNormals()) {
		isave->BeginChunk (kDataChunkTriNormals);
		pData->GetTriNormals()->Save (isave);
		isave->EndChunk ();
	}

	DWORD flags = pData->ExportFlags ();
	if (flags != 0x0)
	{
		DWORD nb;
		isave->BeginChunk(kDataChunkFlags);
		isave->Write(&flags, sizeof(DWORD), &nb);
		isave->EndChunk();
	}

	return IO_OK;
}

IOResult EditNormalsMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	EditNormalsModData *pData = new EditNormalsModData;
	*pld = pData;

	DWORD flags, nb;
	IOResult res;	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kDataChunkNamedSet:
			res = pData->GetNamedNormalSelList().Load (iload);
			break;
		case kDataChunkPolyNormals:
			pData->AllocatePolyNormals ();
			res = pData->GetPolyNormals()->Load(iload);
			break;
		case kDataChunkTriNormals:
			pData->AllocateTriNormals();
			res = pData->GetTriNormals()->Load (iload);
			break;
		case kDataChunkFlags:
			res = iload->Read(&flags, sizeof(DWORD), &nb);
			if (res == IO_OK) pData->ImportFlags(flags);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

bool EditNormalsMod::EnfnMove (Point3 & offset, TimeValue t) {
	// Make sure we're using the correct display length (important for proper move processing)
	float dispLength;
	mpParamBlock->GetValue (en_display_length, t, dispLength, FOREVER);

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	bool ret = false;
	for (int i=0; i<mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if ( !pData ) continue;
		if (theHold.Holding() && !pData->GetFlag(ENMD_NT_HELD)) {
			theHold.Put(new NormalTransformRestore(this, pData));
		}
		if (pData->GetFlag (ENMD_TRIMESH)) {
			pData->GetTriNormals()->SetDisplayLength(dispLength);
			if (pData->GetTriNormals()->Translate (offset, true)) ret = true;
		} else {
			pData->GetPolyNormals()->SetDisplayLength(dispLength);
			if (pData->GetPolyNormals()->Translate (offset, true)) ret = true;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged (PART_GEOM);
	return ret;
}

void EditNormalsMod::Move (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	if (mSelLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	Matrix3 tm  = partm * Inverse(tmAxis);
	Matrix3 itm = Inverse(tm);
	Point3 myTranslation = (tm * val) * itm;

	EnfnMove (myTranslation, t);
	macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Move"), 
			1, 0, mr_point3, &myTranslation);
}

bool EditNormalsMod::EnfnRotate (Quat & rotation, TimeValue t) {
	Matrix3 rotMatrix;
	rotation.MakeMatrix (rotMatrix);

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	bool ret = false;
	for (int i=0; i<mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if ( !pData ) continue;
		if (theHold.Holding() && !pData->GetFlag(ENMD_NT_HELD))
			theHold.Put(new NormalTransformRestore(this, pData));
		if (pData->GetFlag(ENMD_TRIMESH)) {
			if (pData->GetTriNormals()->Transform (rotMatrix, true)) ret = true;
		} else {
			if (pData->GetPolyNormals()->Transform (rotMatrix, true)) ret = true;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged (PART_GEOM);
	return ret;
}

void EditNormalsMod::Rotate (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) {
	Matrix3 tm  = partm * Inverse(tmAxis);
	Matrix3 itm = Inverse(tm);
	val = TransformQuat (tm, val);
	val = TransformQuat (itm, val);

	if (mSelLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	EnfnRotate (val, t);
	macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Rotate"), 
			1, 0, mr_quat, &val);
}

void EditNormalsMod::TransformStart(TimeValue t) {
	if (mpInterface) mpInterface->LockAxisTripods(TRUE);
}

// When we're beginning a transform, we need to specify the relevant normals,
// as well as make backup copies of them.
void EditNormalsMod::TransformHoldingStart (TimeValue t) {
	if (mSelLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	for (int i=0; i<mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;
		if (!pData->GetFlag(ENMD_NT_HELD))
			theHold.Put (new NormalTransformRestore (this, pData));
	}
	nodes.DisposeTemporary();
}

void EditNormalsMod::TransformHoldingFinish (TimeValue t) {
	if (mSelLevel == EN_SL_OBJECT) return;
	if (!mpInterface) return;
	macroRecorder->EmitScript();	// to send out the Move or Rotate before doing the Specify.
	MakeNormalsExplicit ();
}

void EditNormalsMod::TransformFinish(TimeValue t) {
	mpInterface->LockAxisTripods(false);
}

void EditNormalsMod::TransformCancel(TimeValue t) {
	mpInterface->LockAxisTripods(false);
}

RefResult EditNormalsMod::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget, 
   		PartID& partID, RefMessage message) {
	if ((message == REFMSG_CHANGE) && (hTarget == mpParamBlock)) {
		// if this was caused by a NotifyDependents from mpParamBlock, LastNotifyParamID()
		// will contain ID to update, else it will be -1 => inval whole rollout
		int pid = mpParamBlock->LastNotifyParamID();
		InvalidateDialogElement (pid);
	}
	return(REF_SUCCEED);
}

void EditNormalsMod::UpdateDialog() {	
	if (!mpInterface) return;

	TSTR buf;
	int num = 0, which;
	if (!theEditNormalsDesc.NumParamMaps ()) return;
	IParamMap2 *pMap = theEditNormalsDesc.GetParamMap (en_map_main);
	if (!pMap) return;
	HWND hParams = pMap->GetHWnd();
	if (!hParams) return;

	if (mSelLevel == EN_SL_OBJECT) {
		SetDlgItemText (hParams, IDC_EN_NUMBER_SEL, GetString (IDS_OBJECT_SEL));
		theEditNormalProc.SetEnables (hParams, 0);
		return;
	}

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)mcList[i]->localData;
		if (!pData) continue;

		int numSetHere = pData->GetSelection().NumberSet();
		num += numSetHere;
		if ((numSetHere == 1) && (num == 1)) {
			for (which=0; which<pData->GetSelection().GetSize(); which++) {
				if (pData->GetSelection()[which]) break;
			}
		}
	}

	if (num==1) buf.printf (GetString(IDS_EN_WHICH_NORMAL_SELECTED), which+1);
	else buf.printf(GetString(IDS_EN_NUM_NORMALS_SELECTED),num);

	SetDlgItemText(hParams,IDC_EN_NUMBER_SEL,buf);

	theEditNormalProc.SetEnables (hParams, num);
}

void EditNormalsMod::InvalidateDialogElement (int elem) {
	if (!mpParamBlock) return;
	if (!theEditNormalsDesc.NumParamMaps ()) return;
	IParamMap2 *pMap = theEditNormalsDesc.GetParamMap (en_map_main);
	if (pMap) pMap->Invalidate (elem);
	if (pMap && pMap->GetHWnd() && (elem == en_select_by)) UpdateDialog ();
}

void EditNormalsMod::LocalDataChanged (DWORD partID) {
	DWORD partsChanged = PART_DISPLAY;
	partsChanged |= partID & (PART_TOPO|PART_GEOM);
	NotifyDependents(FOREVER, partsChanged, REFMSG_CHANGE);
	if (mpInterface && (partID & (PART_SELECT|PART_TOPO))) {
		UpdateDialog ();
		UpdateNamedSelDropDown ();
	}
}

bool EditNormalsMod::BreakNormals (bool toAverage) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	theHold.Begin ();
	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;
		if (pData->GetFlag (ENMD_NE_HELD)) continue;	// already did this one...

		theHold.Put (new NormalEditRestore (this, pData));

		bool localRet;
		if (pData->GetFlag (ENMD_TRIMESH)) localRet = pData->GetTriNormals()->BreakNormals (mSelLevel, NULL, toAverage);
		else localRet = pData->GetPolyNormals()->BreakNormals(mSelLevel, NULL, toAverage);
		if (localRet) {
			pData->InvalidateTopoCache();
			ret = true;

			if (list.Count() > 1) {
				// Macrorecord each node separately:
				if (toAverage) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Break"), 0, 2,
						_T("node"), mr_reftarg, nodes[i], _T("toAverage"), mr_bool, toAverage);
				} else {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Break"), 0, 1,
						_T("node"), mr_reftarg, nodes[i]);
				}
			} else {
				// Macrorecord with no node specified:
				if (toAverage) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Break"), 0, 1,
						_T("toAverage"), mr_bool, toAverage);
				} else {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Break"), 0, 0);
				}
			}
			macroRecorder->EmitScript();
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		LocalDataChanged (PART_TOPO|PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		theHold.Accept (GetString (IDS_EN_BREAK));
	} else {
		theHold.Cancel ();
	}

	return ret;
}

bool EditNormalsMod::UnifyNormals (bool toAverage) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	theHold.Begin ();
	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;
		if (pData->GetFlag (ENMD_NE_HELD)) continue;	// already did this one...

		if (pData->GetFlag (ENMD_TRIMESH)) {
			if (!pData->GetTriMeshCache()) {
				DbgAssert (0);
				continue;	// Shouldn't happen..
			}
			theHold.Put (new NormalEditRestore (this, pData));

			// Can't unify normals without access to a mesh.
			pData->GetTriNormals()->SetParent(pData->GetTriMeshCache());

			if (pData->GetTriNormals()->UnifyNormals(mSelLevel, NULL, toAverage)) {
				pData->InvalidateTopoCache();
				ret = true;

				if (list.Count() > 1) {
					// Macrorecord each node separately:
					if (toAverage) {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 2,
							_T("node"), mr_reftarg, nodes[i], _T("toAverage"), mr_bool, toAverage);
					} else {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 1,
							_T("node"), mr_reftarg, nodes[i]);
					}
				} else {
					// Macrorecord with no node specified:
					if (toAverage) {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 1,
							_T("toAverage"), mr_bool, toAverage);
					} else {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 0);
					}
				}
				macroRecorder->EmitScript();
			}

			// Can't leave it connected, either:
			pData->GetTriNormals()->SetParent(NULL);
		} else {
			if (!pData->GetPolyMeshCache()) {
				DbgAssert (0);
				continue;	// Shouldn't happen..
			}
			theHold.Put (new NormalEditRestore (this, pData));

			// Can't unify normals without access to a mesh.
			pData->GetPolyNormals()->SetParent(pData->GetPolyMeshCache());

			if (pData->GetPolyNormals()->UnifyNormals(mSelLevel, NULL, toAverage)) {
				pData->InvalidateTopoCache();
				ret = true;

				if (list.Count() > 1) {
					// Macrorecord each node separately:
					if (toAverage) {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 2,
							_T("node"), mr_reftarg, nodes[i], _T("toAverage"), mr_bool, toAverage);
					} else {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 1,
							_T("node"), mr_reftarg, nodes[i]);
					}
				} else {
					// Macrorecord with no node specified:
					if (toAverage) {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 1,
							_T("toAverage"), mr_bool, toAverage);
					} else {
						macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Unify"), 0, 0);
					}
				}
				macroRecorder->EmitScript();
			}

			// Can't leave it connected, either:
			pData->GetPolyNormals()->SetParent(NULL);
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		LocalDataChanged (PART_TOPO|PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		theHold.Accept (GetString (IDS_EN_UNIFY));
	} else {
		theHold.Cancel ();
	}

	return ret;
}

bool EditNormalsMod::ResetNormals () {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	theHold.Begin ();
	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;
		if (pData->GetFlag (ENMD_NE_HELD)) continue;	// already did this one...

		theHold.Put (new NormalEditRestore (this, pData));
		bool localRet;
		if (pData->GetFlag (ENMD_TRIMESH)) localRet = pData->GetTriNormals()->ResetNormals (mSelLevel);
		else localRet = pData->GetPolyNormals()->ResetNormals(mSelLevel);
		if (localRet) {
			pData->InvalidateTopoCache();
			ret = true;

			if (list.Count() > 1) {
				// Macrorecord each node separately:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Reset"), 0, 1,
					_T("node"), mr_reftarg, nodes[i]);
			} else {
				// Macrorecord with no node specified:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Reset"), 0, 0);
			}
			macroRecorder->EmitScript();
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		LocalDataChanged (PART_TOPO|PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		theHold.Accept (GetString (IDS_EN_RESET));
	} else {
		theHold.Cancel ();
	}

	return ret;
}

bool EditNormalsMod::SpecifyNormals () {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}

	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		if (pData->GetFlag(ENMD_NE_HELD)) continue;	// already did this one...

		theHold.Put (new NormalEditRestore (this, pData));

		bool localChange;
		if (pData->GetFlag (ENMD_TRIMESH)) {
			localChange = pData->GetTriNormals()->SpecifyNormals (mSelLevel);
			if (pData->GetTriNormals()->MakeNormalsExplicit (mSelLevel, NULL, false)) {
				pData->InvalidateGeomCache();
				localChange = true;
			}
		} else {
			localChange = pData->GetPolyNormals()->SpecifyNormals (mSelLevel);
			if (pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, NULL, false)) {
				pData->InvalidateGeomCache();
				localChange = true;
			}
		}

		if (localChange) {
			ret = true;
			if (list.Count() > 1) {
				// Macrorecord each node separately:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Specify"), 0, 1,
					_T("node"), mr_reftarg, nodes[i]);
			} else {
				// Macrorecord with no node specified:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Specify"), 0, 0);
			}
			macroRecorder->EmitScript();
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		LocalDataChanged (PART_TOPO|PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		if (localHold) theHold.Accept (GetString (IDS_EN_SPECIFY));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

bool EditNormalsMod::MakeNormalsExplicit () {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		if (pData->GetFlag(ENMD_NE_HELD)) continue;	// already did this one...

		NormalEditRestore *pEditRestore = new NormalEditRestore (this, pData);

		bool localRet;
		if (pData->GetFlag (ENMD_TRIMESH)) localRet = pData->GetTriNormals()->MakeNormalsExplicit(mSelLevel, NULL, true);
		else localRet = pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, NULL, true);
		if (localRet) {
			theHold.Put (pEditRestore);
			ret = true;

			if (list.Count() > 1) {
				// Macrorecord each node separately:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.MakeExplicit"), 0, 1,
					_T("node"), mr_reftarg, nodes[i]);
			} else {
				// Macrorecord with no node specified:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.MakeExplicit"), 0, 0);
			}
			macroRecorder->EmitScript();
		} else {
			pEditRestore->EndHold();
			delete pEditRestore;
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		if (localHold) theHold.Accept (GetString (IDS_EN_MAKE_EXPLICIT));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

// CopyNormal is different from other operations -
// nothing to undo, for instance, so no theHold calls.
bool EditNormalsMod::CopyNormal () {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	// Verify that only one normal is selected, across all contexts:
	int whichContext = -1;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if (!pData) continue;
		int numSet = pData->GetSelection().NumberSet();
		if (numSet > 1) return false;
		if (numSet == 0) continue;
		// Ok, we've got a context with a single normal selected.  It'd better be the only one:
		if (whichContext>-1) return false;
		whichContext = i;
	}

	EditNormalsModData *pData = (EditNormalsModData*)list[whichContext]->localData;
	if (!pData) return false;

	// Find our normal:
	for (int whichNormal=0; whichNormal<pData->EnfnGetNumNormals(); whichNormal++) {
		if (pData->GetSelection()[whichNormal]) break;
	}
	DbgAssert (whichNormal<pData->EnfnGetNumNormals());

	if (list.Count()>1) {
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Copy"), 1, 1,
			mr_int, whichNormal+1, _T("node"), mr_reftarg, nodes[whichContext]);
	} else {
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Copy"), 1, 0,
			mr_int, whichNormal+1);
	}
	macroRecorder->EmitScript();

	// Store the normal:
	if (!mpCopiedNormal) mpCopiedNormal = new Point3;
	if (pData->GetFlag (ENMD_TRIMESH)) *mpCopiedNormal = pData->GetTriNormals()->Normal(whichNormal);
	else *mpCopiedNormal = pData->GetPolyNormals()->Normal(whichNormal);

	nodes.DisposeTemporary();

	// Enable paste button
	UpdateDialog ();

	return true;
}

bool EditNormalsMod::PasteNormal () {
	if (!mpInterface) return false;
	if (!mpCopiedNormal) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}
	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		if (pData->GetFlag(ENMD_NE_HELD)) continue;	// already did this one...

		theHold.Put (new NormalEditRestore (this, pData));

		bool localChange = false;
		if (pData->GetFlag (ENMD_TRIMESH)) {
			if (pData->GetTriNormals()->MakeNormalsExplicit (mSelLevel, NULL, true)) localChange = true;
			for (int j=0; j<pData->GetTriNormals()->GetNumNormals(); j++) {
				if (!pData->GetSelection()[j]) continue;
				pData->GetTriNormals()->Normal(j) = *mpCopiedNormal;
				localChange = true;
			}
		} else {
			if (pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, NULL, true)) localChange = true;
			for (int j=0; j<pData->GetPolyNormals()->GetNumNormals(); j++) {
				if (!pData->GetSelection()[j]) continue;
				pData->GetPolyNormals()->Normal(j) = *mpCopiedNormal;
				localChange = true;
			}
		}
		if (localChange) {
			ret = true;

			if (list.Count() > 1) {
				// Macrorecord each node separately:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Paste"), 0, 1,
					_T("node"), mr_reftarg, nodes[i]);
			} else {
				// Macrorecord with no node specified:
				macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Paste"), 0, 0);
			}
			macroRecorder->EmitScript();
		}
	}

	nodes.DisposeTemporary();
	if (ret) {
		LocalDataChanged (PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		if (localHold) theHold.Accept (GetString (IDS_EN_PASTE));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

// CAL-03/21/03: average selected normals
bool EditNormalsMod::AverageNormals (bool useThresh, float threshold) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}
	theHold.Put (new NormalSelectNotify (this));

	bool ret = false;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		if (pData->GetFlag(ENMD_NE_HELD)) continue;	// already did this one...

		theHold.Put (new NormalEditRestore (this, pData));

		bool localChange = false;
		if (pData->GetFlag (ENMD_TRIMESH)) {
			if (!pData->GetTriMeshCache()) {
				DbgAssert (0);
				continue;	// Shouldn't happen..
			}

			// Can't unify normals without access to a mesh.
			pData->GetTriNormals()->SetParent(pData->GetTriMeshCache());

			if (pData->GetTriNormals()->AverageNormals(useThresh, threshold, mSelLevel)) {
				pData->InvalidateTopoCache();
				localChange = true;
			}

			// Can't leave it connected, either:
			pData->GetTriNormals()->SetParent(NULL);
		} else {
			if (!pData->GetPolyMeshCache()) {
				DbgAssert (0);
				continue;	// Shouldn't happen..
			}

			// Can't unify normals without access to a mesh.
			pData->GetPolyNormals()->SetParent(pData->GetPolyMeshCache());

			if (pData->GetPolyNormals()->AverageNormals(useThresh, threshold, mSelLevel)) {
				pData->InvalidateTopoCache();
				localChange = true;
			}

			// Can't leave it connected, either:
			pData->GetPolyNormals()->SetParent(NULL);
		}
		if (localChange) {
			ret = true;

			if (list.Count() > 1) {
				// Macrorecord each node separately:
				if (useThresh) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Average"), 0, 3,
						_T("useThresh"), mr_bool, useThresh, _T("threshold"), mr_float, threshold, _T("node"), mr_reftarg, nodes[i]);
				} else {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Average"), 0, 1,
						_T("node"), mr_reftarg, nodes[i]);
				}
			} else {
				// Macrorecord with no node specified:
				if (useThresh) {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Average"), 0, 2,
						_T("useThresh"), mr_bool, useThresh, _T("threshold"), mr_float, threshold);
				} else {
					macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.Average"), 0, 0);
				}
			}
			macroRecorder->EmitScript();
		}
	}

	nodes.DisposeTemporary();

	// Average normals between objects
	AverageGlobalNormals(useThresh, threshold);
	
	if (ret) {
		LocalDataChanged (PART_GEOM);
		theHold.Put (new NormalSelectNotify (this));
		if (localHold) theHold.Accept (GetString (IDS_EN_AVERAGE));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

// CAL-03/21/03: Average selected normals between objects
bool EditNormalsMod::AverageGlobalNormals (bool useThresh, float threshold) {
	if (!mpInterface) return false;

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}

	bool ret = EnfnAverageGlobalNormals(useThresh, threshold);

	if (ret) {
		if (useThresh) {
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.AverageGlobal"), 0, 2,
				_T("useThresh"), mr_bool, useThresh, _T("threshold"), mr_float, threshold);
		} else {
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.AverageGlobal"), 0, 0);
		}
		macroRecorder->EmitScript();
	}

	if (ret) {
		theHold.Put (new NormalSelectNotify (this));
		if (localHold) theHold.Accept (GetString (IDS_EN_AVERAGE));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

// CAL-03/21/03: Average two normals from two objects
bool EditNormalsMod::AverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2) {
	if (!mpInterface) return false;
	if (!pNode1 || !pNode2 || ((pNode1 == pNode2) && (normID1 == normID2))) return false;

	bool localHold = false;
	if (!theHold.Holding()) {
		theHold.Begin ();
		localHold = true;
	}

	bool ret = EnfnAverageTwoNormals (pNode1, normID1, pNode2, normID2);

	if (ret) {
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Normals].EditNormalsMod.AverageTwo"), 4, 0,
			mr_reftarg, pNode1, mr_int, normID1+1, mr_reftarg, pNode2, mr_int, normID2+1);
		macroRecorder->EmitScript();
	}

	if (ret) {
		theHold.Put (new NormalSelectNotify (this));
		if (localHold) theHold.Accept (GetString (IDS_EN_AVERAGE));
	} else {
		if (localHold) theHold.Cancel ();
	}

	return ret;
}

// CAL-03/21/03: toggle average normals command mode
void EditNormalsMod::ToggleAverageNormalsCMode () {
	if ( !mpInterface ) return;

	if (mpInterface->GetCommandMode() == mpAverageNormalsCMode)
		mpInterface->PopCommandMode();
	else
		mpInterface->PushCommandMode(mpAverageNormalsCMode);
}

void EditNormalsMod::InvalidateComputedNormals () {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		pData->InvalidateGeomCache ();
		if (pData->GetPolyNormals()) pData->GetPolyNormals()->ClearFlag (MNNORMAL_NORMALS_COMPUTED);
		if (pData->GetTriNormals()) pData->GetTriNormals()->ClearFlag (MESH_NORMAL_NORMALS_COMPUTED);
	}
	nodes.DisposeTemporary();

	LocalDataChanged (PART_GEOM);
}

bool EditNormalsMod::EnfnBreakNormals (BitArray *normalSelection, INode *pNode, bool toAverage) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret;
	if (pData->GetFlag (ENMD_TRIMESH)) ret = pData->GetTriNormals()->BreakNormals (mSelLevel, normalSelection, toAverage);
	else ret = pData->GetPolyNormals()->BreakNormals(mSelLevel, normalSelection, toAverage);
	if (ret) pData->InvalidateTopoCache();

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_TOPO|PART_GEOM);
	return ret;
}

// STEVE: Left off here.

bool EditNormalsMod::EnfnUnifyNormals (BitArray *normalSelection, INode *pNode, bool toAverage) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		// Can't unify normals without access to a mesh.
		pData->GetTriNormals()->SetParent(pData->GetTriMeshCache());

		ret = pData->GetTriNormals()->UnifyNormals(mSelLevel, normalSelection, toAverage);
		if (ret) pData->InvalidateTopoCache();

		// Can't leave it connected, either:
		pData->GetTriNormals()->SetParent(NULL);
	} else {
		// Can't unify normals without access to a mesh.
		pData->GetPolyNormals()->SetParent(pData->GetPolyMeshCache());

		ret = pData->GetPolyNormals()->UnifyNormals(mSelLevel, normalSelection, toAverage);
		if (ret) pData->InvalidateTopoCache();

		// Can't leave it connected, either:
		pData->GetPolyNormals()->SetParent(NULL);
	}

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_TOPO|PART_GEOM);
	return ret;
}

bool EditNormalsMod::EnfnResetNormals (BitArray *normalSelection, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		ret = pData->GetTriNormals()->ResetNormals (mSelLevel, normalSelection);
	} else {
		ret = pData->GetPolyNormals()->ResetNormals (mSelLevel, normalSelection);
	}
	if (ret) pData->InvalidateTopoCache();

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_TOPO|PART_GEOM);
	return ret;
}

bool EditNormalsMod::EnfnSpecifyNormals (BitArray *normalSelection, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret = pData->GetPolyNormals()->SpecifyNormals (mSelLevel, normalSelection);
	// Also clear the explicitness of these normals:
	if (pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, normalSelection, false)) {
		ret = true;
		pData->InvalidateGeomCache();
	}

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_TOPO|PART_GEOM);
	return ret;
}

bool EditNormalsMod::EnfnMakeNormalsExplicit (BitArray *normalSelection, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		ret = pData->GetTriNormals()->MakeNormalsExplicit (mSelLevel, normalSelection, true);
	} else {
		ret = pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, normalSelection, true);
	}
	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_DISPLAY);
	return ret;
}

bool EditNormalsMod::EnfnCopyNormal (int normalID, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;
	nodes.DisposeTemporary();

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		if (!pData->GetTriNormals()) return false;
		if (pData->GetTriNormals()->GetNumNormals() <= normalID) return false;

		// Store the normal:
		if (!mpCopiedNormal) mpCopiedNormal = new Point3;
		*mpCopiedNormal = pData->GetTriNormals()->Normal(normalID);
	} else {
		if (!pData->GetPolyNormals()) return false;
		if (pData->GetPolyNormals()->GetNumNormals() <= normalID) return false;

		// Store the normal:
		if (!mpCopiedNormal) mpCopiedNormal = new Point3;
		*mpCopiedNormal = pData->GetPolyNormals()->Normal(normalID);
	}

	return true;
}

bool EditNormalsMod::EnfnPasteNormal (BitArray *normalSelection, INode *pNode) {
	if (!mpInterface) return false;
	if (!mpCopiedNormal) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		ret = pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, normalSelection, true);
		for (int j=0; j<pData->GetPolyNormals()->GetNumNormals(); j++) {
			if (!pData->GetSelection()[j]) continue;
			pData->GetPolyNormals()->Normal(j) = *mpCopiedNormal;
			ret = true;
		}
	} else {
		ret = pData->GetPolyNormals()->MakeNormalsExplicit (mSelLevel, normalSelection, true);
		for (int j=0; j<pData->GetPolyNormals()->GetNumNormals(); j++) {
			if (!pData->GetSelection()[j]) continue;
			pData->GetPolyNormals()->Normal(j) = *mpCopiedNormal;
			ret = true;
		}
	}
	LocalDataChanged (PART_GEOM);
	nodes.DisposeTemporary();
	return ret;
}

// CAL-03/21/03: average selected normals
bool EditNormalsMod::EnfnAverageNormals (bool useThresh, float threshold, BitArray *normalSelection, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if ( !pData ) return false;

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pData));
		theHold.Put (new NormalSelectNotify (this));
	}

	bool ret = false;
	if (pData->GetFlag (ENMD_TRIMESH)) {
		// Can't unify normals without access to a mesh.
		pData->GetTriNormals()->SetParent(pData->GetTriMeshCache());

		ret = pData->GetTriNormals()->AverageNormals(useThresh, threshold, true, normalSelection);
		if (ret) pData->InvalidateTopoCache();

		// Can't leave it connected, either:
		pData->GetTriNormals()->SetParent(NULL);
	} else {
		// Can't unify normals without access to a mesh.
		pData->GetPolyNormals()->SetParent(pData->GetPolyMeshCache());

		ret = pData->GetPolyNormals()->AverageNormals(useThresh, threshold, true, normalSelection);
		if (ret) pData->InvalidateTopoCache();

		// Can't leave it connected, either:
		pData->GetPolyNormals()->SetParent(NULL);
	}

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_GEOM);

	return ret;
}

// CAL-03/21/03: Average selected normals between objects
bool EditNormalsMod::EnfnAverageGlobalNormals (bool useThresh, float threshold) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	if (list.Count() < 2) {
		nodes.DisposeTemporary();
		return false;
	}

	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
	
		for (int i=0; i<list.Count(); i++) {
			EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
			if ( !pData ) continue;
			if (pData->GetFlag(ENMD_NE_HELD)) continue;	// already did this one...

			theHold.Put (new NormalEditRestore (this, pData));
		}
	}

	TimeValue t = mpInterface->GetTime();
	bool ret = false;
	if (useThresh) {
		// identify the vertex candidates for averaging
		Tab<BitArray> vertexSelection;
		Tab<BitArray> vertexNearby;
		for (int i=0; i<list.Count(); i++) {
			// NOTE: Tab<> doesn't initialize the BitArray. So, append it one by one.
			BitArray emptyBitArray;
			vertexSelection.Append(1, &emptyBitArray);
			vertexNearby.Append(1, &emptyBitArray);
			
			EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
			if ( !pData ) continue;

			BitArray &vertSel = vertexSelection[i];
			BitArray &vertNear = vertexNearby[i];
			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache();
				if (!pMesh) continue;
				MeshNormalSpec *pNorm = pData->GetTriNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				vertSel.SetSize(pMesh->numVerts);
				vertSel.ClearAll();
				vertNear.SetSize(pMesh->numVerts);
				int maxf = pNorm->GetNumFaces();
				if (pMesh->numFaces<maxf) maxf = pMesh->numFaces;
				for (int f=0; f<maxf; f++) {
					for (int j=0; j<3; j++) {
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID<0) continue;
						if (pNorm->GetSelection()[normID]) vertSel.Set(pMesh->faces[f].v[j]);
					}
				}
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh) continue;
				MNNormalSpec *pNorm = pData->GetPolyNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				vertSel.SetSize(pMesh->numv);
				vertSel.ClearAll();
				vertNear.SetSize(pMesh->numv);
				int maxf = pNorm->GetNumFaces();
				if (pMesh->numf<maxf) maxf = pMesh->numf;
				for (int f=0; f<maxf; f++) {
					if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
					int maxDeg = pNorm->Face(f).GetDegree();
					if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
					for (int j=0; j<maxDeg; j++) {
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID<0) continue;
						if (pNorm->GetSelection()[normID]) vertSel.Set(pMesh->f[f].vtx[j]);
					}
				}
			}
		}
		
		// average normals on different objects
		float sqrThreshold = threshold * threshold;
		for (i=0; i<list.Count(); i++) {
			EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
			if ( !pData ) continue;

			BitArray &vertSel = vertexSelection[i];
			if (!vertSel.NumberSet()) continue;
			
			Matrix3 objToWorldTM = nodes[i]->GetObjectTM(t);
			Matrix3 worldToObjTM = Inverse(objToWorldTM);

			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache();
				if (!pMesh) continue;
				MeshNormalSpec *pNorm = pData->GetTriNormals();
				if (!pNorm) continue;

				// sum all normals for each selected vertex on this object
				Tab<Point3> sumNormal;
				sumNormal.SetCount(pMesh->numVerts);
				for (int v=0; v<pMesh->numVerts; v++)
					if (vertSel[v]) sumNormal[v] = Point3::Origin;
				BitArray visited;
				visited.SetSize(pNorm->GetNumNormals());
				visited.ClearAll();
				int maxf = pNorm->GetNumFaces();
				if (pMesh->numFaces<maxf) maxf = pMesh->numFaces;
				for (int f=0; f<maxf; f++) {
					for (int j=0; j<3; j++) {
						int vertID = pMesh->faces[f].v[j];
						if (!vertSel[vertID]) continue;
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID] || visited[normID]) continue;
						sumNormal[vertID] += pNorm->Normal(normID);
						visited.Set(normID);
					}
				}
				for (v=0; v<pMesh->numVerts; v++)
					if (vertSel[v]) sumNormal[v] = objToWorldTM.VectorTransform(sumNormal[v]);

				// sum all nearby normals on other objects
				for (v=0; v<pMesh->numVerts; v++) {
					if (!vertSel[v]) continue;
					Point3 p = objToWorldTM.PointTransform(pMesh->verts[v]);
					Point3 pnt, norm;
					for (int j=i+1; j<list.Count(); j++) {
						EditNormalsModData *pDataOther = (EditNormalsModData*)list[j]->localData;
						if ( !pDataOther ) continue;
						BitArray &vertSelOther = vertexSelection[j];
						if (!vertSelOther.NumberSet()) continue;
						BitArray &vertNear = vertexNearby[j];
						pnt = Inverse(nodes[j]->GetObjectTM(t)).PointTransform(p);
						if (!pDataOther->FindNearbyVertices(pnt, sqrThreshold, vertSelOther, vertNear)) continue;
						if (!pDataOther->SumSelectedNormalsOnVertices(vertNear, norm)) continue;
						sumNormal[v] += nodes[j]->GetObjectTM(t).VectorTransform(norm);
						vertSelOther ^= vertNear;			// remove nearby vertices from vert selection
					}
					sumNormal[v] = Normalize(sumNormal[v]);	// average normal in world space
					for (j=i+1; j<list.Count(); j++) {
						EditNormalsModData *pDataOther = (EditNormalsModData*)list[j]->localData;
						if ( !pDataOther ) continue;
						BitArray &vertNear = vertexNearby[j];
						if (!vertNear.NumberSet()) continue;
						norm = Normalize(Inverse(nodes[j]->GetObjectTM(t)).VectorTransform(sumNormal[v]));
						if (pDataOther->SetSelectedNormalsOnVertices(vertNear, norm)) ret = true;
					}
					sumNormal[v] = Normalize(worldToObjTM.VectorTransform(sumNormal[v]));
				}
				for (f=0; f<maxf; f++) {
					for (int j=0; j<3; j++) {
						int vertID = pMesh->faces[f].v[j];
						if (!vertSel[vertID]) continue;
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
						if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
						if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
						pNorm->Normal(normID) = sumNormal[vertID];
						ret = true;
					}
				}
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh) continue;
				MNNormalSpec *pNorm = pData->GetPolyNormals();
				if (!pNorm) continue;

				// sum all normals for each selected vertex on this object
				Tab<Point3> sumNormal;
				sumNormal.SetCount(pMesh->numv);
				for (int v=0; v<pMesh->numv; v++)
					if (vertSel[v]) sumNormal[v] = Point3::Origin;
				BitArray visited;
				visited.SetSize(pNorm->GetNumNormals());
				visited.ClearAll();
				int maxf = pNorm->GetNumFaces();
				if (pMesh->numf<maxf) maxf = pMesh->numf;
				for (int f=0; f<maxf; f++) {
					if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
					int maxDeg = pNorm->Face(f).GetDegree();
					if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
					for (int j=0; j<maxDeg; j++) {
						int vertID = pMesh->f[f].vtx[j];
						if (!vertSel[vertID]) continue;
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID] || visited[normID]) continue;
						sumNormal[vertID] += pNorm->Normal(normID);
						visited.Set(normID);
					}
				}
				for (v=0; v<pMesh->numv; v++)
					if (vertSel[v]) sumNormal[v] = objToWorldTM.VectorTransform(sumNormal[v]);

				// sum all nearby normals on other objects
				for (v=0; v<pMesh->numv; v++) {
					if (!vertSel[v]) continue;
					Point3 p = objToWorldTM.PointTransform(pMesh->P(v));
					Point3 pnt, norm;
					for (int j=i+1; j<list.Count(); j++) {
						EditNormalsModData *pDataOther = (EditNormalsModData*)list[j]->localData;
						if ( !pDataOther ) continue;
						BitArray &vertSelOther = vertexSelection[j];
						if (!vertSelOther.NumberSet()) continue;
						BitArray &vertNear = vertexNearby[j];
						pnt = Inverse(nodes[j]->GetObjectTM(t)).PointTransform(p);
						if (!pDataOther->FindNearbyVertices(pnt, sqrThreshold, vertSelOther, vertNear)) continue;
						if (!pDataOther->SumSelectedNormalsOnVertices(vertNear, norm)) continue;
						sumNormal[v] += nodes[j]->GetObjectTM(t).VectorTransform(norm);
						vertSelOther ^= vertNear;			// remove nearby vertices from vert selection
					}
					sumNormal[v] = Normalize(sumNormal[v]);	// average normal in world space
					for (j=i+1; j<list.Count(); j++) {
						EditNormalsModData *pDataOther = (EditNormalsModData*)list[j]->localData;
						if ( !pDataOther ) continue;
						BitArray &vertNear = vertexNearby[j];
						if (!vertNear.NumberSet()) continue;
						norm = Normalize(Inverse(nodes[j]->GetObjectTM(t)).VectorTransform(sumNormal[v]));
						if (pDataOther->SetSelectedNormalsOnVertices(vertNear, norm)) ret = true;
					}
					sumNormal[v] = Normalize(worldToObjTM.VectorTransform(sumNormal[v]));
				}
				for (f=0; f<maxf; f++) {
					if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
					int maxDeg = pNorm->Face(f).GetDegree();
					if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
					for (int j=0; j<maxDeg; j++) {
						int vertID = pMesh->f[f].vtx[j];
						if (!vertSel[vertID]) continue;
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
						if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
						if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
						pNorm->Normal(normID) = sumNormal[vertID];
						ret = true;
					}
				}
			}
		}

		// NOTE: Tab<> doesn't free the BitArray.
		for (i=0; i<list.Count(); i++) {
			vertexSelection[i].SetSize(0);
			vertexNearby[i].SetSize(0);
		}
	} else {
		// Average all selected normals
		Point3 sumNorm = Point3::Origin;
		for (int i=0; i<list.Count(); i++) {
			EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
			if ( !pData ) continue;

			// Add up local summation
			Point3 sumLocal = Point3::Origin;
			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache();
				if (!pMesh) continue;
				MeshNormalSpec *pNorm = pData->GetTriNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				for (int j=0; j<pNorm->GetNumNormals(); j++)
					if (pNorm->GetSelection()[j]) sumLocal += pNorm->Normal(j);
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh) continue;
				MNNormalSpec *pNorm = pData->GetPolyNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				for (int j=0; j<pNorm->GetNumNormals(); j++)
					if (pNorm->GetSelection()[j]) sumLocal += pNorm->Normal(j);
			}

			// Add to world summation
			sumNorm += nodes[i]->GetObjectTM(t).VectorTransform(sumLocal);
		}

		sumNorm = Normalize(sumNorm);	// average normal in world space

		for (i=0; i<list.Count(); i++) {
			EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
			if ( !pData ) continue;

			// Set average to each object
			Point3 avgLocal = Normalize(Inverse(nodes[i]->GetObjectTM(t)).VectorTransform(sumNorm));
			if (pData->GetFlag (ENMD_TRIMESH)) {
				Mesh *pMesh = pData->GetTriMeshCache();
				if (!pMesh) continue;
				MeshNormalSpec *pNorm = pData->GetTriNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				int maxf = pNorm->GetNumFaces();
				if (pMesh->numFaces<maxf) maxf = pMesh->numFaces;
				for (int f=0; f<maxf; f++) {
					for (int j=0; j<3; j++) {
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
						if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
						if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
						pNorm->Normal(normID) = avgLocal;
						ret = true;
					}
				}
			} else {
				MNMesh *pMesh = pData->GetPolyMeshCache();
				if (!pMesh) continue;
				MNNormalSpec *pNorm = pData->GetPolyNormals();
				if (!pNorm) continue;
				if (!pNorm->GetSelection().NumberSet()) continue;

				int maxf = pNorm->GetNumFaces();
				if (pMesh->numf<maxf) maxf = pMesh->numf;
				for (int f=0; f<maxf; f++) {
					if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
					int maxDeg = pNorm->Face(f).GetDegree();
					if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
					for (int j=0; j<maxDeg; j++) {
						int normID = pNorm->Face(f).GetNormalID(j);
						if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
						if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
						if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
						pNorm->Normal(normID) = avgLocal;
						ret = true;
					}
				}
			}
		}
	}

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_GEOM);

	return ret;
}

// CAL-03/21/03: Average two normals from two different objects
bool EditNormalsMod::EnfnAverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2) {
	if (!mpInterface) return false;
	if (!pNode1 || !pNode2 || ((pNode1 == pNode2) && (normID1 == normID2))) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	EditNormalsModData *pModData1 = NULL, *pModData2 = NULL;
	for (int i=0; i<list.Count(); i++) {
		EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
		if ( !pData ) continue;
		if (!pModData1 && nodes[i] == pNode1) pModData1 = pData;
		if (!pModData2 && nodes[i] == pNode2) pModData2 = pData;
	}
	if (!pModData1 || !pModData2) {
		nodes.DisposeTemporary();
		return false;
	}

	TimeValue t = mpInterface->GetTime();
	Point3 *pNorm1, *pNorm2;
	pNorm1 = pModData1->EnfnGetNormal(normID1, t);
	pNorm2 = pModData2->EnfnGetNormal(normID2, t);
	if (!pNorm1 || !pNorm2) {
		nodes.DisposeTemporary();
		return false;
	}

	bool ret = false;
	if (theHold.Holding()) {
		theHold.Put (new NormalSelectNotify (this));
		theHold.Put (new NormalEditRestore (this, pModData1));
		if (pModData2 != pModData1)
			theHold.Put (new NormalEditRestore (this, pModData2));
	}

	Point3 normal1, normal2, avgNorm;
	if (pNode1 == pNode2) {
		normal1 = normal2 = Normalize (*pNorm1 + *pNorm2);
	} else {
		normal1 = pNode1->GetObjectTM(t).VectorTransform(*pNorm1);
		normal2 = pNode2->GetObjectTM(t).VectorTransform(*pNorm2);
		avgNorm = Normalize (normal1 + normal2);
		normal1 = Normalize (Inverse(pNode1->GetObjectTM(t)).VectorTransform(avgNorm));
		normal2 = Normalize (Inverse(pNode2->GetObjectTM(t)).VectorTransform(avgNorm));
	}

	pModData1->EnfnSetNormalExplicit(normID1, true);
	pModData1->EnfnSetNormal(normID1, normal1, t);
	pModData2->EnfnSetNormalExplicit(normID2, true);
	pModData2->EnfnSetNormal(normID2, normal2, t);
	ret = true;

	nodes.DisposeTemporary();
	if (ret) LocalDataChanged (PART_GEOM);

	return ret;
}

BitArray *EditNormalsMod::EnfnGetSelection (INode *pNode) {
	if (!mpInterface) return NULL;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return NULL;

	return pData->EnfnGetSelection();
}

bool EditNormalsMod::EnfnSetSelection (BitArray & selection, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	if (!pData->EnfnSetSelection(selection)) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
	LocalDataChanged (PART_SELECT);
	return true;
}

bool EditNormalsMod::EnfnSelect (BitArray & selection, bool invert, bool select, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));

	if (!pData->EnfnSelect(selection, invert, select)) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectNotify(this));
	LocalDataChanged (PART_SELECT);
	return true;
}

void EditNormalsMod::EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection, INode *pNode) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return;

	pData->EnfnConvertVertexSelection(vertexSelection, normalSelection);
}

void EditNormalsMod::EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection, INode *pNode) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return;

	pData->EnfnConvertEdgeSelection(edgeSelection, normalSelection);
}

void EditNormalsMod::EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection, INode *pNode) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return;

	pData->EnfnConvertFaceSelection(faceSelection, normalSelection);
}

int EditNormalsMod::EnfnGetNumNormals (INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetNumNormals();
}

Point3 *EditNormalsMod::EnfnGetNormal (int normalID, INode *pNode, TimeValue t) {
	if (!mpInterface) return NULL;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return NULL;

	return pData->EnfnGetNormal(normalID, t);
}

void EditNormalsMod::EnfnSetNormal (int normalID, Point3 &direction, INode *pNode, TimeValue t) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->EnfnSetNormal(normalID, direction, t);
	LocalDataChanged (PART_TOPO|PART_GEOM);
}

bool EditNormalsMod::EnfnGetNormalExplicit (int normalIndex, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return false;

	return pData->EnfnGetNormalExplicit(normalIndex);
}

void EditNormalsMod::EnfnSetNormalExplicit (int normalIndex, bool value, INode *pNode) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->EnfnSetNormalExplicit (normalIndex, value);
	LocalDataChanged (PART_GEOM|PART_TOPO);
}

int EditNormalsMod::EnfnGetNumFaces (INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetNumFaces();
}

int EditNormalsMod::EnfnGetFaceDegree (int face, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetFaceDegree(face);
}

int EditNormalsMod::EnfnGetNormalID (int face, int corner, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetNormalID(face, corner);
}

void EditNormalsMod::EnfnSetNormalID (int face, int corner, int normalID, INode *pNode) {
	if (!mpInterface) return;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->EnfnSetNormalID(face, corner, normalID);
	LocalDataChanged (PART_TOPO);
}

int EditNormalsMod::EnfnGetNumVertices (INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetNumVertices();
}

int EditNormalsMod::EnfnGetVertexID (int face, int corner, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetVertexID(face, corner);
}

Point3 EditNormalsMod::EnfnGetVertex (int vertexID, INode *pNode, TimeValue t) {
	if (!mpInterface) return Point3(0,0,0);

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return Point3(0,0,0);

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return Point3(0,0,0);

	return pData->EnfnGetVertex(vertexID, t);
}

int EditNormalsMod::EnfnGetNumEdges (INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetNumEdges ();
}

int EditNormalsMod::EnfnGetEdgeID (int faceIndex, int sideIndex, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetEdgeID (faceIndex, sideIndex);
}

int EditNormalsMod::EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetFaceEdgeSide (faceIndex, edgeIndex);
}

int EditNormalsMod::EnfnGetEdgeVertex (int edgeIndex, int end, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetEdgeVertex (edgeIndex, end);
}

int EditNormalsMod::EnfnGetEdgeFace (int edgeIndex, int side, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetEdgeFace (edgeIndex, side);
}

int EditNormalsMod::EnfnGetEdgeNormal (int edgeIndex, int end, int side, INode *pNode) {
	if (!mpInterface) return 0;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return 0;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return 0;

	return pData->EnfnGetEdgeNormal (edgeIndex, end, side);
}

bool EditNormalsMod::EnfnGetFaceNormalSpecified (int face, int corner, INode *pNode) {
	if (!mpInterface) return false;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return false;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return false;

	return pData->EnfnGetFaceNormalSpecified(face, corner);
}

void EditNormalsMod::EnfnSetFaceNormalSpecified (int face, int corner, bool specified, INode *pNode) {
	if (!mpInterface) return ;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->EnfnSetFaceNormalSpecified(face, corner, specified);
	LocalDataChanged (PART_TOPO);
}

void EditNormalsMod::EnfnRecomputeNormals(INode *pNode) {
	if (!mpInterface) return ;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->InvalidateGeomCache ();
	LocalDataChanged (PART_GEOM);
}

void EditNormalsMod::EnfnRebuildNormals(INode *pNode) {
	if (!mpInterface) return ;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return ;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return ;

	pData->InvalidateTopoCache ();
	LocalDataChanged (PART_TOPO|PART_GEOM);
}

MNMesh *EditNormalsMod::EnfnGetMesh (INode *pNode, TimeValue t) {
	if (!mpInterface) return NULL;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	int i;
	if (pNode) {
		for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	} else i=0;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return NULL;

	return pData->EnfnGetMesh(t);
}

MNNormalSpec *EditNormalsMod::EnfnGetNormals (INode *pNode, TimeValue t) {
	if (!mpInterface) return NULL;

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);
	for (int i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
	if (i==list.Count()) return NULL;

	EditNormalsModData *pData = (EditNormalsModData*)list[i]->localData;
	if (!pData) return NULL;

	return pData->EnfnGetNormals(t);
}

BaseInterface *EditNormalsMod::GetInterface (Interface_ID id) {
	if (id == EDIT_NORMALS_MOD_INTERFACE) return (IEditNormalsMod *)this;
	return FPMixinInterface::GetInterface(id);
}



// EditNormalsModData -----------------------------------------------------

EditNormalsModData::~EditNormalsModData () {
	if (mpLocalPolyNormals) delete mpLocalPolyNormals;
	if (mpPolyMesh) delete mpPolyMesh;
	if (mpLocalTriNormals) delete mpLocalTriNormals;
	if (mpTriMesh) delete mpTriMesh;
	if (mpNewSelection) delete mpNewSelection;
}

LocalModData *EditNormalsModData::Clone() {
	EditNormalsModData *pData = new EditNormalsModData;
	if (mpLocalPolyNormals) {
		pData->AllocatePolyNormals();
		*(pData->mpLocalPolyNormals) = *mpLocalPolyNormals;
	}
	if (mpLocalTriNormals) {
		pData->AllocateTriNormals ();
		*(pData->mpLocalTriNormals) = *mpLocalTriNormals;
	}
	pData->CopyFlags (this);
	return pData;
}

void EditNormalsModData::SetupNewSelection () {
	if (!mpNewSelection) mpNewSelection = new BitArray;
	if (GetFlag (ENMD_TRIMESH)) {
		mpNewSelection->SetSize(mpLocalTriNormals->GetSelection().GetSize());
	} else {
		mpNewSelection->SetSize(mpLocalPolyNormals->GetSelection().GetSize());
	}
}

bool EditNormalsModData::ApplyNewSelection (bool keepOld, bool invert, bool select) {
	if (!mpNewSelection) return false;

	bool ret;
	if (keepOld) ret = EnfnSelect (*mpNewSelection, invert, select);
	else ret = EnfnSetSelection (*mpNewSelection);

	delete mpNewSelection;
	mpNewSelection = NULL;
	return ret;
}

BitArray *EditNormalsModData::EnfnGetSelection () {
	if (mpLocalPolyNormals) return &(mpLocalPolyNormals->GetSelection());
	if (mpLocalTriNormals) return &(mpLocalTriNormals->GetSelection());
	return NULL;
}

bool EditNormalsModData::EnfnSetSelection (BitArray & selection) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return false;
	} else {
		if (!mpLocalPolyNormals) return false;
	}
	int properSize = GetSelection().GetSize();
	if (!properSize) return false;
	BitArray properSelection = selection;
	properSelection.SetSize (properSize);

	// Don't do anything if new selection matches old:
	if (properSelection == GetSelection()) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectRestore (this, &properSelection));
	GetSelection() = properSelection;
	return true;
}

bool EditNormalsModData::EnfnSelect (BitArray & selection, bool invert, bool select) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return false;
	} else {
		if (!mpLocalPolyNormals) return false;
	}
	int properSize = GetSelection().GetSize();
	if (!properSize) return false;
	BitArray properSelection = selection;
	properSelection.SetSize (properSize);

	if (invert) {
		// Bits in result should be set if set in exactly one of current, incoming selections
		properSelection ^= GetSelection();
	} else {
		if (select) {
			// Result set if set in either of current, incoming:
			properSelection |= GetSelection();
		} else {
			// Result set if in current, and _not_ in incoming:
			properSelection = ~properSelection;
			properSelection &= GetSelection();
		}
	}

	// Don't do anything if new selection matches old:
	if (properSelection == GetSelection()) return false;

	if (theHold.Holding()) theHold.Put (new NormalSelectRestore (this, &properSelection));
	GetSelection() = properSelection;
	return true;
}

void EditNormalsModData::EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
		if (!mpTriMesh) return;
		if (!mpTriMesh->faces) return;
	} else {
		if (!mpLocalPolyNormals) return;
		if (!mpPolyMesh) return;
		if (!mpPolyMesh->f) return;
	}

	normalSelection.SetSize (GetSelection().GetSize());
	normalSelection.ClearAll ();

	if (GetFlag (ENMD_TRIMESH)) {
		// Iterate through faces, marking which normals are included in the vertex selection:
		int maxFace = mpLocalTriNormals->GetNumFaces();
		if (mpTriMesh->numFaces < maxFace) maxFace = mpTriMesh->numFaces;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			MeshNormalFace & face = mpLocalTriNormals->Face(i);
			for (int j=0; j<3; j++) {
				int vertexID = mpTriMesh->faces[i].v[j];
				if (!vertexSelection[vertexID]) continue;

				int normalID = face.GetNormalID(j);
				if (normalID < 0) continue;
				if (normalID >= normalSelection.GetSize()) continue;
				normalSelection.Set (normalID);
			}
		}
	} else {
		// Iterate through faces, marking which normals are included in the vertex selection:
		int maxFace = mpLocalPolyNormals->GetNumFaces();
		if (mpPolyMesh->numf < maxFace) maxFace = mpPolyMesh->numf;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			if (mpPolyMesh->f[i].GetFlag(MN_DEAD)) continue;
			MNNormalFace & face = mpLocalPolyNormals->Face(i);
			int maxDeg = face.GetDegree();
			if (mpPolyMesh->f[i].deg < maxDeg) maxDeg = mpPolyMesh->f[i].deg;

			for (int j=0; j<maxDeg; j++) {
				int vertexID = mpPolyMesh->f[i].vtx[j];
				if (!vertexSelection[vertexID]) continue;

				int normalID = face.GetNormalID(j);
				if (normalID < 0) continue;
				if (normalID >= normalSelection.GetSize()) continue;
				normalSelection.Set (normalID);
			}
		}
	}
}

void EditNormalsModData::EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
		if (!mpTriMesh) return;
		if (!mpTriMesh->faces) return;
	} else {
		if (!mpLocalPolyNormals) return;
		if (!mpPolyMesh) return;
		if (!mpPolyMesh->f) return;
		if (!mpPolyMesh->GetFlag (MN_MESH_FILLED_IN)) return;
		if (!mpPolyMesh->e) return;
	}

	normalSelection.SetSize(mpLocalPolyNormals->GetSelection().GetSize());
	normalSelection.ClearAll();
	if (!edgeSelection.NumberSet()) return;

	if (GetFlag (ENMD_TRIMESH)) {
		// Iterate through faces, marking which normals are included in the edge selection:
		int maxFace = mpLocalTriNormals->GetNumFaces();
		if (mpTriMesh->numFaces < maxFace) maxFace = mpTriMesh->numFaces;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			MeshNormalFace & face = mpLocalTriNormals->Face(i);
			for (int j=0; j<3; j++) {
				int edgeID = i*3+j;
				if (!edgeSelection[edgeID]) continue;
				for (int end=0; end<2; end++) {
					int normalID = face.GetNormalID((j+end)%3);
					if (normalID < 0) continue;
					if (normalID >= normalSelection.GetSize()) continue;
					normalSelection.Set (normalID);
				}
			}
		}
	} else {
		// Iterate through faces, marking which normals are included in the edge selection:
		int maxFace = mpLocalPolyNormals->GetNumFaces();
		if (mpPolyMesh->numf < maxFace) maxFace = mpPolyMesh->numf;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			if (mpPolyMesh->f[i].GetFlag(MN_DEAD)) continue;
			MNNormalFace & face = mpLocalPolyNormals->Face(i);
			int maxDeg = face.GetDegree();
			if (mpPolyMesh->f[i].deg < maxDeg) maxDeg = mpPolyMesh->f[i].deg;

			for (int j=0; j<maxDeg; j++) {
				int edgeID = mpPolyMesh->f[i].edg[j];
				if (!edgeSelection[edgeID]) continue;

				for (int end=0; end<2; end++) {
					int normalID = face.GetNormalID((j+end)%face.GetDegree());
					if (normalID < 0) continue;
					if (normalID >= normalSelection.GetSize()) continue;
					normalSelection.Set (normalID);
				}
			}
		}
	}
}

void EditNormalsModData::EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
	} else {
		if (!mpLocalPolyNormals) return;
	}

	normalSelection.SetSize(mpLocalPolyNormals->GetSelection().GetSize());
	normalSelection.ClearAll();

	if (GetFlag (ENMD_TRIMESH)) {
		// Iterate through faces, marking which normals are included in the face selection:
		int maxFace = faceSelection.GetSize();
		if (mpLocalTriNormals->GetNumFaces() < maxFace) maxFace = mpLocalTriNormals->GetNumFaces();
		if (mpTriMesh && (mpTriMesh->numFaces < maxFace)) maxFace = mpTriMesh->numFaces;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			if (!faceSelection[i]) continue;
			MeshNormalFace & face = mpLocalTriNormals->Face(i);
			for (int j=0; j<3; j++) {
				int normalID = face.GetNormalID(j);
				if (normalID < 0) continue;
				if (normalID >= normalSelection.GetSize()) continue;
				normalSelection.Set (normalID);
			}
		}
	} else {
		// Iterate through faces, marking which normals are included in the face selection:
		int maxFace = faceSelection.GetSize();
		if (mpLocalPolyNormals->GetNumFaces() < maxFace) maxFace = mpLocalPolyNormals->GetNumFaces();
		if (mpPolyMesh && (mpPolyMesh->numf < maxFace)) maxFace = mpPolyMesh->numf;
		if (!maxFace) return;

		for (int i=0; i<maxFace; i++) {
			if (!faceSelection[i]) continue;
			if (mpPolyMesh && mpPolyMesh->f && mpPolyMesh->f[i].GetFlag(MN_DEAD)) continue;
			MNNormalFace & face = mpLocalPolyNormals->Face(i);
			for (int j=0; j<face.GetDegree(); j++) {
				int normalID = face.GetNormalID(j);
				if (normalID < 0) continue;
				if (normalID >= normalSelection.GetSize()) continue;
				normalSelection.Set (normalID);
			}
		}
	}
}

int EditNormalsModData::EnfnGetNumNormals () {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return 0;
		return mpLocalTriNormals->GetNumNormals();
	} else {
		if (!mpLocalPolyNormals) return 0;
		return mpLocalPolyNormals->GetNumNormals();
	}
}

Point3 *EditNormalsModData::EnfnGetNormal (int normalID, TimeValue t) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return NULL;
		if ((normalID<0) || (normalID>=mpLocalTriNormals->GetNumNormals())) return NULL;
		return &(mpLocalTriNormals->Normal(normalID));
	} else {
		if (!mpLocalPolyNormals) return NULL;
		if ((normalID<0) || (normalID>=mpLocalPolyNormals->GetNumNormals())) return NULL;
		return &(mpLocalPolyNormals->Normal(normalID));
	}
}

void EditNormalsModData::EnfnSetNormal (int normalID, Point3 &direction, TimeValue t) {
	float lenSq = LengthSquared (direction);
	Point3 ourDir = (lenSq && (lenSq != 1.0f)) ? direction/Sqrt(lenSq) : direction;
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
		if ((normalID<0) || (normalID>=mpLocalTriNormals->GetNumNormals())) return;
		mpLocalTriNormals->Normal(normalID) = ourDir;
	} else {
		if (!mpLocalPolyNormals) return;
		if ((normalID<0) || (normalID>=mpLocalPolyNormals->GetNumNormals())) return;
		mpLocalPolyNormals->Normal(normalID) = ourDir;
	}
}

bool EditNormalsModData::EnfnGetNormalExplicit (int normalID) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return false;
		if ((normalID<0) || (normalID>=mpLocalTriNormals->GetNumNormals())) return false;
		return mpLocalTriNormals->GetNormalExplicit (normalID);
	} else {
		if (!mpLocalPolyNormals) return false;
		if ((normalID<0) || (normalID>=mpLocalPolyNormals->GetNumNormals())) return false;
		return mpLocalPolyNormals->GetNormalExplicit (normalID);
	}
}

void EditNormalsModData::EnfnSetNormalExplicit (int normalID, bool value) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return ;
		if ((normalID<0) || (normalID>=mpLocalTriNormals->GetNumNormals())) return ;
		mpLocalTriNormals->SetNormalExplicit (normalID, value);

		if (value) {
			// Set specified bits on faces using this normal:
			for (int i=0; i<mpLocalTriNormals->GetNumFaces(); i++) {
				MeshNormalFace & face = mpLocalTriNormals->Face(i);
				for (int j=0; j<3; j++) {
					if (face.GetNormalID(j) == normalID) face.SetSpecified(j, true);
				}
			}
		}
	} else {
		if (!mpLocalPolyNormals) return ;
		if ((normalID<0) || (normalID>=mpLocalPolyNormals->GetNumNormals())) return ;
		mpLocalPolyNormals->SetNormalExplicit (normalID, value);

		if (value) {
			// Set specified bits on faces using this normal:
			for (int i=0; i<mpLocalPolyNormals->GetNumFaces(); i++) {
				MNNormalFace & face = mpLocalPolyNormals->Face(i);
				for (int j=0; j<face.GetDegree(); j++) {
					if (face.GetNormalID(j) == normalID) face.SetSpecified(j, true);
				}
			}
		}
	}

	if (!value) {
		// Normal should be recomputed.
		InvalidateGeomCache ();
	}
}

int EditNormalsModData::EnfnGetNumFaces () {
	if (GetFlag (ENMD_TRIMESH)) {
		return mpLocalTriNormals ? mpLocalTriNormals->GetNumFaces() : 0;
	} else {
		if (!mpLocalPolyNormals) return 0;
		return mpLocalPolyNormals->GetNumFaces();
	}
}

int EditNormalsModData::EnfnGetFaceDegree (int face) {
	if (GetFlag (ENMD_TRIMESH)) return 3;
	if (!mpLocalPolyNormals) return 0;
	if ((face<0) || (face>=mpLocalPolyNormals->GetNumFaces())) return 0;
	return mpLocalPolyNormals->Face(face).GetDegree();
}

int EditNormalsModData::EnfnGetNormalID (int face, int corner) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return false;
		if ((face<0) || (face>=mpLocalTriNormals->GetNumFaces())) return false;
		return mpLocalTriNormals->Face(face).GetNormalID (corner);
	} else {
		if (!mpLocalPolyNormals) return false;
		if ((face<0) || (face>=mpLocalPolyNormals->GetNumFaces())) return false;
		return mpLocalPolyNormals->Face(face).GetNormalID (corner);
	}
}

void EditNormalsModData::EnfnSetNormalID (int face, int corner, int normalID) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
		if ((face<0) || (face>=mpLocalTriNormals->GetNumFaces())) return;
		mpLocalTriNormals->Face(face).SetNormalID (corner, normalID);
	} else {
		if (!mpLocalPolyNormals) return;
		if ((face<0) || (face>=mpLocalPolyNormals->GetNumFaces())) return;
		mpLocalPolyNormals->Face(face).SetNormalID (corner, normalID);
	}

	// Normals don't need to be rebuilt, but they should be recomputed.
	InvalidateGeomCache ();
}

bool EditNormalsModData::EnfnGetFaceNormalSpecified (int face, int corner) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return false;
		if ((face<0) || (face>=mpLocalTriNormals->GetNumFaces())) return false;
		return mpLocalTriNormals->Face(face).GetSpecified(corner);
	} else {
		if (!mpLocalPolyNormals) return false;
		if ((face<0) || (face>=mpLocalPolyNormals->GetNumFaces())) return false;
		return mpLocalPolyNormals->Face(face).GetSpecified(corner);
	}
}

void EditNormalsModData::EnfnSetFaceNormalSpecified (int face, int corner, bool specified) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpLocalTriNormals) return;
		if ((face<0) || (face>=mpLocalTriNormals->GetNumFaces())) return;
		mpLocalTriNormals->Face(face).SetSpecified (corner, specified);
	} else {
		if (!mpLocalPolyNormals) return;
		if ((face<0) || (face>=mpLocalPolyNormals->GetNumFaces())) return;
		mpLocalPolyNormals->Face(face).SetSpecified (corner, specified);
	}
	// Normals should be rebuilt.
	InvalidateTopoCache ();
}

int EditNormalsModData::EnfnGetNumVertices () {
	if (GetFlag (ENMD_TRIMESH)) return mpTriMesh ? mpTriMesh->numVerts : 0;
	if (!mpPolyMesh) return 0;
	return mpPolyMesh->numv;
}

int EditNormalsModData::EnfnGetVertexID (int face, int corner) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpTriMesh) return 0;
		if ((face<0) || (face>=mpTriMesh->numFaces)) return 0;
		if (!mpTriMesh->faces) return 0;
		if ((corner<0) || (corner>=3)) return 0;
		return mpTriMesh->faces[face].v[corner];
	} else {
		if (!mpPolyMesh) return 0;
		if ((face<0) || (face>=mpPolyMesh->numf)) return 0;
		if (!mpPolyMesh->f) return 0;
		MNFace *pFace = mpPolyMesh->F(face);
		if ((corner<0) || (corner>=pFace->deg)) return 0;
		return pFace->vtx[corner];
	}
}

Point3 EditNormalsModData::EnfnGetVertex (int vertexID, TimeValue t) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpTriMesh) return Point3(0,0,0);
		if (vertexID<0) return Point3(0,0,0);
		if (vertexID>=mpTriMesh->numVerts) return Point3(0,0,0);
		return mpTriMesh->verts[vertexID];
	} else {
		if (!mpPolyMesh) return Point3(0,0,0);
		if (vertexID<0) return Point3(0,0,0);
		if (vertexID>=mpPolyMesh->numv) return Point3(0,0,0);
		return mpPolyMesh->P(vertexID);
	}
}

int EditNormalsModData::EnfnGetNumEdges () {
	if (GetFlag (ENMD_TRIMESH)) {
		// STEVE: Use adjacent edge list?
		if (!mpTriMesh) return 0;
		return mpTriMesh->numFaces*3;
	} else {
		if (!mpPolyMesh) return 0;
		if (!mpPolyMesh->GetFlag(MN_MESH_FILLED_IN)) return 0;
		if (!mpPolyMesh->e) return 0;
		return mpPolyMesh->nume;
	}
}

int EditNormalsModData::EnfnGetEdgeID (int faceIndex, int side) {
	// STEVE: use adjacent edge list?
	if (GetFlag (ENMD_TRIMESH)) return faceIndex*3+side;

	if (!mpPolyMesh) return -1;
	if (!mpPolyMesh->GetFlag (MN_MESH_FILLED_IN)) return -1;
	if (!mpPolyMesh->e) return -1;
	if (!mpPolyMesh->f) return -1;
	if ((faceIndex<0) || (faceIndex>=mpPolyMesh->numf)) return -1;
	MNFace *pFace = mpPolyMesh->F(faceIndex);
	if ((side<0) || (side>=pFace->deg)) return -1;

	return pFace->edg[side];
}

int EditNormalsModData::EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex) {
	if (GetFlag (ENMD_TRIMESH)) return 0;
	if (!mpPolyMesh) return -1;
	if (!mpPolyMesh->f) return -1;
	if ((faceIndex<0) || (faceIndex>=mpPolyMesh->numf)) return -1;
	MNFace *pFace = mpPolyMesh->F(faceIndex);
	if (pFace->GetFlag(MN_DEAD)) return -1;
	return pFace->EdgeIndex (edgeIndex);
}

int EditNormalsModData::EnfnGetEdgeVertex (int edgeIndex, int end) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpTriMesh) return -1;
		if (!mpTriMesh->faces) return -1;
		if ((edgeIndex<0) || (edgeIndex>=mpTriMesh->numFaces*3)) return -1;
		if ((end<0) || (end>1)) return -1;

		return mpTriMesh->faces[edgeIndex/3].v[(edgeIndex%3+end)%3];
	} else {
		if (!mpPolyMesh) return -1;
		if (!mpPolyMesh->GetFlag (MN_MESH_FILLED_IN)) return -1;
		if (!mpPolyMesh->e) return -1;
		if ((edgeIndex<0) || (edgeIndex>=mpPolyMesh->nume)) return -1;
		if ((end<0) || (end>1)) return -1;

		return mpPolyMesh->e[edgeIndex][end];
	}
}

int EditNormalsModData::EnfnGetEdgeFace (int edgeIndex, int side) {
	if (GetFlag (ENMD_TRIMESH)) return edgeIndex/3;
	if (!mpPolyMesh) return -1;
	if (!mpPolyMesh->GetFlag (MN_MESH_FILLED_IN)) return -1;
	if (!mpPolyMesh->e) return -1;
	if ((edgeIndex<0) || (edgeIndex>=mpPolyMesh->nume)) return -1;
	if ((side<0) || (side>1)) return -1;

	return side ? mpPolyMesh->e[edgeIndex].f2 : mpPolyMesh->e[edgeIndex].f1;
}

int EditNormalsModData::EnfnGetEdgeNormal (int edgeIndex, int end, int side) {
	if (GetFlag (ENMD_TRIMESH)) {
		if (!mpTriMesh) return -1;
		if (!mpTriMesh->faces) return -1;
		if ((edgeIndex<0) || (edgeIndex>=mpTriMesh->numFaces*3)) return -1;
		if ((end<0) || (end>1)) return -1;
		if ((side<0) || (side>1)) return -1;

		int face = edgeIndex/3;
		int faceSide = edgeIndex%3;

		if (!mpLocalTriNormals) return -1;
		if (face>=mpLocalTriNormals->GetNumFaces()) return -1;
		MeshNormalFace & nFace = mpLocalTriNormals->Face(face);
		return nFace.GetNormalID(faceSide);
	} else {
		if (!mpPolyMesh) return -1;
		if (!mpPolyMesh->GetFlag (MN_MESH_FILLED_IN)) return -1;
		if (!mpPolyMesh->e) return -1;
		if ((edgeIndex<0) || (edgeIndex>=mpPolyMesh->nume)) return -1;
		if ((end<0) || (end>1)) return -1;
		if ((side<0) || (side>1)) return -1;

		int face = side ? mpPolyMesh->e[edgeIndex].f2 : mpPolyMesh->e[edgeIndex].f1;
		if (face<0) return -1;

		int faceSide = mpPolyMesh->f[face].EdgeIndex (edgeIndex, mpPolyMesh->e[edgeIndex][side]);
		if (faceSide<0) return -1;

		if (!mpLocalPolyNormals) return -1;
		if (face>=mpLocalPolyNormals->GetNumFaces()) return -1;
		MNNormalFace & nFace = mpLocalPolyNormals->Face(face);
		if (faceSide >= nFace.GetDegree()) return -1;
		if (end==side) return nFace.GetNormalID(faceSide);
		else return nFace.GetNormalID((faceSide+1)%nFace.GetDegree());
	}
}

void EditNormalsModData::SetFlag (DWORD fl, bool val) {
	if (fl == ENMD_TRIMESH) {
		if (GetFlag (fl) == val) return;
		if (val) {
			ClearPolyMeshCache ();
			ClearPolyNormals ();
		} else {
			ClearTriMeshCache ();
			ClearTriNormals ();
		}
	}
	FlagUser::SetFlag (fl, val);
}

BitArray & EditNormalsModData::GetSelection () {
	if (GetFlag (ENMD_TRIMESH)) {
		return mpLocalTriNormals->GetSelection();
	} else {
		return mpLocalPolyNormals->GetSelection();
	}
}

// CAL-03/21/03: Support functions for AverageGlobalNormals
bool EditNormalsModData::FindNearbyVertices(const Point3 &refPoint, float sqrThreshDist, const BitArray &inSelection, BitArray &outSelection)
{
	outSelection.SetSize(inSelection.GetSize());
	outSelection.ClearAll();
	if (GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = GetTriMeshCache();
		if (!pMesh) return false;

		for (int i=0; i<pMesh->numVerts; i++) {
			if (!inSelection[i]) continue;
			if (LengthSquared(refPoint - pMesh->verts[i]) < sqrThreshDist) outSelection.Set(i);
		}
	} else {
		MNMesh *pMesh = GetPolyMeshCache();
		if (!pMesh) return false;

		for (int i=0; i<pMesh->numv; i++) {
			if (!inSelection[i]) continue;
			if (LengthSquared(refPoint - pMesh->P(i)) < sqrThreshDist) outSelection.Set(i);
		}
	}
	return true;
}

bool EditNormalsModData::SumSelectedNormalsOnVertices(const BitArray &vertSelection, Point3 &outNormal)
{
	bool ret = false;
	outNormal = Point3::Origin;

	if (GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = GetTriMeshCache();
		if (!pMesh) return false;
		MeshNormalSpec *pNorm = GetTriNormals();
		if (!pNorm) return false;

		BitArray visited;
		visited.SetSize(pNorm->GetNumNormals());
		visited.ClearAll();

		int maxf = pNorm->GetNumFaces();
		if (pMesh->numFaces<maxf) maxf = pMesh->numFaces;
		for (int f=0; f<maxf; f++) {
			for (int j=0; j<3; j++) {
				int vertID = pMesh->faces[f].v[j];
				if (!vertSelection[vertID]) continue;
				int normID = pNorm->Face(f).GetNormalID(j);
				if (normID < 0 || !pNorm->GetSelection()[normID] || visited[normID]) continue;
				outNormal += pNorm->Normal(normID);
				visited.Set(normID);
				ret = true;
			}
		}
	} else {
		MNMesh *pMesh = GetPolyMeshCache();
		if (!pMesh) return false;
		MNNormalSpec *pNorm = GetPolyNormals();
		if (!pNorm) return false;
		
		BitArray visited;
		visited.SetSize(pNorm->GetNumNormals());
		visited.ClearAll();

		int maxf = pNorm->GetNumFaces();
		if (pMesh->numf<maxf) maxf = pMesh->numf;
		for (int f=0; f<maxf; f++) {
			if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
			int maxDeg = pNorm->Face(f).GetDegree();
			if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
			for (int j=0; j<maxDeg; j++) {
				int vertID = pMesh->f[f].vtx[j];
				if (!vertSelection[vertID]) continue;
				int normID = pNorm->Face(f).GetNormalID(j);
				if (normID < 0 || !pNorm->GetSelection()[normID] || visited[normID]) continue;
				outNormal += pNorm->Normal(normID);
				visited.Set(normID);
				ret = true;
			}
		}
	}

	return ret;
}

bool EditNormalsModData::SetSelectedNormalsOnVertices(const BitArray &vertSelection, const Point3 &inNormal)
{
	bool ret = false;

	if (GetFlag (ENMD_TRIMESH)) {
		Mesh *pMesh = GetTriMeshCache();
		if (!pMesh) return false;
		MeshNormalSpec *pNorm = GetTriNormals();
		if (!pNorm) return false;

		int maxf = pNorm->GetNumFaces();
		if (pMesh->numFaces<maxf) maxf = pMesh->numFaces;
		for (int f=0; f<maxf; f++) {
			for (int j=0; j<3; j++) {
				int vertID = pMesh->faces[f].v[j];
				if (!vertSelection[vertID]) continue;
				int normID = pNorm->Face(f).GetNormalID(j);
				if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
				if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
				if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
				pNorm->Normal(normID) = inNormal;
				ret = true;
			}
		}
	} else {
		MNMesh *pMesh = GetPolyMeshCache();
		if (!pMesh) return false;
		MNNormalSpec *pNorm = GetPolyNormals();
		if (!pNorm) return false;
		
		int maxf = pNorm->GetNumFaces();
		if (pMesh->numf<maxf) maxf = pMesh->numf;
		for (int f=0; f<maxf; f++) {
			if (pMesh->f[f].GetFlag (MN_DEAD)) continue;
			int maxDeg = pNorm->Face(f).GetDegree();
			if (pMesh->f[f].deg < maxDeg) maxDeg = pMesh->f[f].deg;
			for (int j=0; j<maxDeg; j++) {
				int vertID = pMesh->f[f].vtx[j];
				if (!vertSelection[vertID]) continue;
				int normID = pNorm->Face(f).GetNormalID(j);
				if (normID < 0 || !pNorm->GetSelection()[normID]) continue;
				if (!pNorm->Face(f).GetSpecified(j)) pNorm->Face(f).SetSpecified(j);
				if (!pNorm->GetNormalExplicit(normID)) pNorm->SetNormalExplicit(normID, true);
				pNorm->Normal(normID) = inNormal;
				ret = true;
			}
		}
	}

	return ret;
}

#endif

