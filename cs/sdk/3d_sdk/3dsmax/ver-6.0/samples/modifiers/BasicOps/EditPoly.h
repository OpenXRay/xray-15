/**********************************************************************
 *<
	FILE: PolyOperation.h

	DESCRIPTION: Edit Poly operation classes

	CREATED BY: Steve Anderson

	HISTORY: created March 2002

 *>	Copyright (c) 2002 Discreet, All Rights Reserved.
 **********************************************************************/

const Class_ID kEDIT_MOD_CLASS_ID(0x5ac632fc, 0xde875a9);

// Possible Selection Levels:
enum EditPolyModSelLevels { kSelLevObject, kSelLevVertex, kSelLevEdge,
	kSelLevBorder, kSelLevFace, kSelLevElement };

// Relate those selection levels to MNMesh selection levels:
const int meshSelLevel[] = { MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE, MNM_SL_EDGE,
	MNM_SL_FACE, MNM_SL_FACE };

// Get display flags based on selLevel.
const DWORD levelDispFlags[] = {0, MNDISP_VERTTICKS|MNDISP_SELVERTS,
		MNDISP_SELEDGES, MNDISP_SELEDGES, MNDISP_SELFACES, MNDISP_SELFACES};

// For hit testing...
const int hitLevel[] = { 0, SUBHIT_MNVERTS, SUBHIT_MNEDGES,
				SUBHIT_MNEDGES|SUBHIT_OPENONLY,
				SUBHIT_MNFACES, SUBHIT_MNFACES };

class PolyOperation;

// Reference ID's for EPolyMod
enum EditPolyRefID {EDIT_PBLOCK_REF, EDIT_NODE_REF};

class EditPolyMod : public Modifier, public IMeshSelect {
friend class SelectRestore;
friend class EditPolySelectProc;
friend class EditPolyData;
friend class ShapePickMode;
friend class EditPolyOperationProc;

	// Our references:
	// We always need a Parameter Block:
	IParamBlock2 *mpParams;
	// We sometimes need a node:
	INode *mpRefNode;

	// Selection level - this local data is saved/loaded.
	// (STEVE: Idea - put in parameter block?)
	DWORD mSelLevel;

	// Static list of available operations:
	static Tab<PolyOperation *> mOpList;

	// Interface pointer:
	static IObjParam *mpInterface;
	// Static pointer to the modifier instance that's currently being edited (if any):
	static EditPolyMod *mpCurrentEditMod;
	// Selection mode for making our SO selections
	static SelectModBoxCMode *mpSelectMode;
	static bool mUpdateCachePosted;

	// UI for the operation-specific dialog proc:
	static IParamMap2 *mpDialogOperation, *mpDialogType, *mpDialogSelect;

	// Pick mode for getting spline for extrude along spline:
	static ShapePickMode *mpShapePicker;

public:
	EditPolyMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_EDIT_POLY_MOD);}  
	virtual Class_ID ClassID () { return kEDIT_MOD_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_EDIT_POLY_MOD); }
	void* GetInterface(ULONG id) { if (id == I_MESHSELECT) return (IMeshSelect *)this; else return Modifier::GetInterface(id); }

	void BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier
	// We can change or depend on pretty much anything.
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
	ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_SUBSEL_TYPE|PART_VERTCOLOR; }
	Class_ID InputType() { return polyObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity (TimeValue t);
	//BOOL DependOnTopology(ModContext &mc);	// basically true if we're using explicit selections.

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

	// ParamBlock2 access:
	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mpParams; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mpParams->ID() == id) ? mpParams : NULL; }

	// Reference Management:
	int NumRefs() {return 2;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message);

	// Animatable management:
	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return mpParams;}
	TSTR SubAnimName(int i) { return _T(""); }

	IParamBlock2 *getParamBlock () { return mpParams; }

	// Subobject API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// Selection stuff from BaseObject:
	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);

	// From IMeshSelect
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// Selection related stuff:
	void UpdateSelLevelDisplay ();
	void SetEnableStates();
	void SetNumSelLabel();		
	void UpdateCache(TimeValue t);
	void AcquirePipeSelection ();
	bool IsVertexSelected (EditPolyData *pData, int vertexID);
	bool IsEdgeSelected (EditPolyData *pData, int edgeID);
	bool IsFaceSelected (EditPolyData *pData, int faceID);
	void GetVertexTempSelection (EditPolyData *pData, BitArray & vertexTempSel);

	void NotifyInputChanged (Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	void UpdateOpChoice ();
	void UpdateOperationDialog ();
	void UpdateOperationDialogParams ();

	void CommitToOperation ();
	void ClearSpline ();

	int GetNumOperations () { return mOpList.Count (); }
	int GetPolyOperationIndex (TimeValue t);
	PolyOperation *GetPolyOperation (TimeValue t, Interval & valid);
	PolyOperation *GetPolyOperationByIndex (int i) { return mOpList[i]; }
	PolyOperation *GetPolyOperationByID (int id);
private:
	void InitializeOperationList ();
	void ClearOperationList ();
	
	void ConvertPolySelection (MNMesh & mesh, int msl);
};

class EditPolyData : public LocalModData, public IMeshSelectData {
friend class SelectRestore;
friend class EditPolyMod;

	BitArray mVertSel, mEdgeSel, mFaceSel;
	MNMesh *mpMeshCopy;	// Simple copy of the mesh before modification
	BitArray *mpNewSelection;
	bool mHeld, mCommit;
	PolyOperation *mpOpList;

public:
	// LocalModData methods
	void* GetInterface(ULONG id) { if (id == I_MESHSELECTDATA) return(IMeshSelectData*)this; else return LocalModData::GetInterface(id); }

	EditPolyData (MNMesh &mesh) : mpMeshCopy(NULL), mHeld(false), mCommit(false),
		mpNewSelection(NULL), mpOpList(NULL) { SetCache (mesh); }
	EditPolyData () : mpMeshCopy(NULL), mHeld(false), mCommit(false),
		mpNewSelection(NULL), mpOpList(NULL) { }
	~EditPolyData() { DeleteAllOperations (); FreeCache(); FreeNewSelection (); }

	LocalModData *Clone();
	MNMesh *GetMesh() {return mpMeshCopy;}
	void SetCache(MNMesh &mesh);
	void FreeCache();
	void SynchBitArrays();

	void ConvertSelection (int epSelLevelFrom, int epSelLevelTo);

	// From IMeshSelectData:
	BitArray GetVertSel() { return mVertSel; }
	BitArray GetFaceSel() { return mFaceSel; }
	BitArray GetEdgeSel() { return mEdgeSel; }

	void SetVertSel (BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetFaceSel (BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetEdgeSel (BitArray &set, IMeshSelect *imod, TimeValue t);

	// Apparently have to do something for these methods...
	GenericNamedSelSetList & GetNamedVertSelList ();
	GenericNamedSelSetList & GetNamedEdgeSelList ();
	GenericNamedSelSetList & GetNamedFaceSelList ();

	// Accessor:
	void SetHeld () { mHeld = true; }
	void ClearHeld () { mHeld = false; }
	bool GetHeld () { return mHeld; }

	void SetCommit (bool commit) { mCommit = commit; }
	bool GetCommit () { return mCommit; }

	// New selection methods:
	void SetupNewSelection (int meshSelLevel);
	BitArray *GetNewSelection () { return mpNewSelection; }
	bool ApplyNewSelection (EditPolyMod *pMod, bool keepOld=false, bool invert=false, bool select=true);
	void ConvertNewSelectionToBorder ();
	void ConvertNewSelectionToElement ();
	void FreeNewSelection () { if (mpNewSelection) { delete mpNewSelection; mpNewSelection=NULL; } }

	// Operation stack methods:
	// PushOperation: Pushes an op onto the end of the stack (the last to be applied).
	void PushOperation (PolyOperation *pOp);
	// PopOperation: Pops an op off the end of the stack (the last one to be applied).
	PolyOperation *PopOperation ();
	void DeleteAllOperations ();
	void ApplyAllOperations (MNMesh & mesh);
};

class SelectRestore : public RestoreObj {
	BitArray mUndoSel, mRedoSel;
	EditPolyMod *mpMod;
	EditPolyData *mpData;
	int mLevel;

public:
	SelectRestore (EditPolyMod *pMod, EditPolyData *pData);
	SelectRestore (EditPolyMod *pMod, EditPolyData *pData, int level);
	void After ();
	void Restore(int isUndo);
	void Redo();
	int Size() { return 2*sizeof(void *) + 2*sizeof(BitArray) + sizeof(int); }
	void EndHold() { mpData->ClearHeld(); }
	TSTR Description() { return TSTR(_T("SelectRestore")); }
};

class AddOperationRestoreObj : public RestoreObj
{
private:
	EditPolyData *mpData;
	PolyOperation *mpOp;
public:
	AddOperationRestoreObj (EditPolyData *pData, PolyOperation *pOp)
		: mpData(pData), mpOp(pOp) { }
	void Restore (int isUndo) { mpData->PopOperation (); }
	void Redo () { mpData->PushOperation (mpOp); }
	int Size () { return 8; }
	TSTR Description() { return TSTR(_T("AddOperation")); }
};

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kEditPolyParams };

// And enumerate the parameters within that block:
enum {
	// The type of Edit Poly:
	kEditPolyType,

	// Selection parameters:
	kEPExplicitSelection, kEPSelByVertex, kEPIgnoreBackfacing,

	// Individual operation parameters:
	kEPWeldThreshold, kEPChamferAmount, kEPExtrudeHeight,
	kEPExtrudeType, kEPExtrudeWidth, kEPBevelHeight,
	kEPBevelOutline, kEPExtrudeSplineSegments, kEPExtrudeSplineTaper,
	kEPExtrudeSplineTaperCurve, kEPExtrudeSplineTwist, kEPExtrudeSplineRotation,
	kEPExtrudeSplineAlign, kEPExtrudeSpline,
};

// Then enumerate all the rollouts we may need:
enum { kEditPolyChoice, kEditPolySelect, kEditPolySettings };

// Finally, do the operations themselves.

enum opIDs { kOpNull, kOpWeldVertex, kOpExtrudeFace, kOpExtrudeVertex, kOpExtrudeEdge,
	kOpChamferVertex, kOpChamferEdge, kOpBevelFace, kOpBreakVertex,
	kOpExtrudeAlongSpline, kOpRetriangulate };

class PolyOperation 
{
protected:
	BitArray mSelection;
	PolyOperation *mpNext;
public:
	PolyOperation () : mpNext(NULL) { }

	void RecordSelection (MNMesh & mesh);
	void RestoreSelection (MNMesh & mesh);
	PolyOperation *Next () { return mpNext; }
	void SetNext (PolyOperation *pNext) { mpNext = pNext; }
	void CopyBasics (PolyOperation *pCopyTo);
	IOResult SaveBasics (ISave *isave);
	IOResult LoadBasics (ILoad *iload);

	virtual int OpID () { return kOpNull; }
	virtual TCHAR *Name () { return GetString (IDS_EP_CHOOSE_OP); }
	virtual int DialogID () { return 0; }
	virtual int MeshSelectionLevel () { return MNM_SL_OBJECT; }

	// These Parameter methods make it easy to manage the subset of parameters
	// that are significant to this operation.  Parameters supported by these methods
	// are automatically copied, saved, and loaded in CopyBasics, SaveBasics, and LoadBasics,
	// and are filled in with their correct values in the default implementation of GetValues.
	// They only support float and int parameters presently.
	// Simple PolyOperations can use these to avoid having to implement Save, Load, and GetValues.
	virtual void GetParameters (Tab<int> & paramList) { }
	virtual void *Parameter (int i) { return NULL; }
	virtual int ParameterSize (int i) { return sizeof(float); }	// note - also = sizeof(int).

	virtual void GetValues (IParamBlock2 *pBlock, TimeValue t, Interval & ivalid);
	virtual void Do (MNMesh & mesh) {}
	virtual PolyOperation *Clone () {
		PolyOperation *ret = new PolyOperation();
		CopyBasics (ret);
		return ret;
	}
	virtual IOResult Save (ISave *isave);
	virtual IOResult Load (ILoad *iload);
	virtual void DeleteThis () { delete this; }
};

// Most individual PolyOps are defined in PolyOps.cpp.
// This one, we have here, so we can access it from EditPolyMod::NotifyRefChanged.
class PolyOpExtrudeAlongSpline : public PolyOperation
{
private:
	Spline3D *mpSpline;
	Matrix3 mSplineXfm;
	Interval mSplineValidity;
	int mSegments, mAlign;
	float mTaper, mTaperCurve, mTwist, mRotation;

public:
	PolyOpExtrudeAlongSpline () : mpSpline(NULL), mSplineXfm(true), mSegments(10),
		mAlign(true), mTaper(0.0f), mTaperCurve(0.0f), mTwist(0.0f), mRotation(0.0f) { }
	void ClearSpline ();

	// Implement PolyOperation methods:
	int OpID () { return kOpExtrudeAlongSpline; }
	TCHAR * Name () { return GetString (IDS_EP_EXTRUDE_ALONG_SPLINE); }
	int DialogID () { return IDD_EP_EXTRUDE_ALONG_SPLINE; }
	int MeshSelectionLevel() { return MNM_SL_FACE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (6);
		paramList[0] = kEPExtrudeSplineSegments;
		paramList[1] = kEPExtrudeSplineTaper;
		paramList[2] = kEPExtrudeSplineTaperCurve;
		paramList[3] = kEPExtrudeSplineTwist;
		paramList[4] = kEPExtrudeSplineRotation;
		paramList[5] = kEPExtrudeSplineAlign;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPExtrudeSplineSegments) return &mSegments;
		if (paramID == kEPExtrudeSplineTaper) return &mTaper;
		if (paramID == kEPExtrudeSplineTaperCurve) return &mTaperCurve;
		if (paramID == kEPExtrudeSplineTwist) return &mTwist;
		if (paramID == kEPExtrudeSplineRotation) return &mRotation;
		if (paramID == kEPExtrudeSplineAlign) return &mAlign;
		return NULL;
	}
	void GetValues (IParamBlock2 *pBlock, TimeValue t, Interval & ivalid);
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpExtrudeAlongSpline *ret = new PolyOpExtrudeAlongSpline();
		CopyBasics (ret);
		if (mpSpline != NULL) ret->mpSpline = new Spline3D(*mpSpline);
		ret->mSplineValidity = FOREVER;
		ret->mSplineXfm = mSplineXfm;
		return ret;
	}
	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);
	void DeleteThis () { ClearSpline (); delete this; }
};
