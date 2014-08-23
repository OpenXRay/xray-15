
/**********************************************************************
 *<
	FILE: editpat.h

	DESCRIPTION:  Edit Patch OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 23 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/


#ifndef __EDITPATCH_H__
#define __EDITPATCH_H__

#include "namesel.h"
#include "nsclip.h"
#include "sbmtlapi.h"
#include "stdmat.h"

#define Alert(x) MessageBox(GetActiveWindow(),x,_T("Alert"),MB_OK);

#define EDITPAT_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL)

// set the default pick box size
#define DEF_PICKBOX_SIZE	4

// These are values for selLevel.
#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_ELEMENT	4
#define EP_HANDLE	5
#define EP_LEVELS	6

// Named selection set levels:
#define EP_NS_VERTEX 0
#define EP_NS_EDGE 1
#define EP_NS_PATCH 2
#define EP_NS_HANDLE 3
#define EP_NS_LEVELS 4		// number of selection levels
// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { EP_NS_VERTEX, EP_NS_VERTEX, EP_NS_EDGE, EP_NS_PATCH, EP_NS_PATCH, EP_NS_HANDLE };
static int namedClipLevel[] = { CLIP_P_VERT, CLIP_P_VERT, CLIP_P_EDGE, CLIP_P_PATCH, CLIP_P_PATCH, CLIP_P_HANDLE };

#define MAX_MATID	0xffff

#define UNDEFINED	0xffffffff

#define CID_EPM_BIND	CID_USER + 203
#define CID_EPM_EXTRUDE	CID_USER + 204
#define CID_EPM_BEVEL	CID_USER + 205
#define CID_EPM_NORMAL_FLIP	CID_USER + 206
// CAL-06/02/03: copy/paste tangent. (FID #827)
#define CID_COPY_TANGENT CID_USER + 210
#define CID_PASTE_TANGENT CID_USER + 211

// Relax defaults
#define DEF_EP_RELAX FALSE
#define DEF_EP_RELAX_VIEWPORTS	TRUE
#define DEF_EP_RELAX_VALUE	0.0f
#define DEF_EP_RELAX_ITER	1
#define DEF_EP_RELAX_BOUNDARY	TRUE
#define DEF_EP_RELAX_SADDLE	FALSE

// CAL-05/01/03: support spline surface generation (FID #1914)
#define EP_DEF_GENSURF_THRESH	1.0f
#define EP_MIN_GENSURF_THRESH	0.0f
#define EP_MAX_GENSURF_THRESH	float(1.0e15)


class EditPatchMod;

class EPM_BindMouseProc : public MouseCallBack {
	private:
		EditPatchMod *pobj;
		IObjParam *ip;
		IPoint2 om;
		BitArray knotList;
		PatchMesh *pMesh;
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert);
		BOOL HitASegment(ViewExp *vpt, IPoint2 *p, int *Seg);

		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		EPM_BindMouseProc(EditPatchMod* spl, IObjParam *i) { pobj=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};



class EPM_BindCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EPM_BindMouseProc eproc;
		EditPatchMod* pobj;
//		int type; // See above

	public:
		EPM_BindCMode(EditPatchMod* spl, IObjParam *i) :
			fgProc((ReferenceTarget*)spl), eproc(spl,i) {pobj=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EP_BIND; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
//		void SetType(int type) { this->type = type; eproc.SetType(type); }
	};

class EPM_ExtrudeMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditPatchMod *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EPM_ExtrudeMouseProc(EditPatchMod* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EPM_ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EPM_ExtrudeSelectionProcessor(EPM_ExtrudeMouseProc *mc, EditPatchMod *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EPM_ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EPM_ExtrudeSelectionProcessor mouseProc;
	EPM_ExtrudeMouseProc eproc;
	EditPatchMod* po;

public:
	EPM_ExtrudeCMode(EditPatchMod* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EPM_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};


class EPM_NormalFlipMouseProc : public MouseCallBack {
private:
	EditPatchMod *ep;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
	PatchMesh *pMesh;
public:
	EPM_NormalFlipMouseProc(EditPatchMod* mod, IObjParam *i) {ep=mod;ip=i;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAPatch(ViewExp *vpt, IPoint2 *p, int *pix);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EPM_NormalFlipCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EPM_NormalFlipMouseProc eproc;
	EditPatchMod* ep;

public:
	EPM_NormalFlipCMode(EditPatchMod* mod, IObjParam *i) :
		fgProc((ReferenceTarget *)mod), eproc(mod,i) {ep=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_NORMAL_FLIP; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class EPM_BevelMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditPatchMod *po;
	Interface *ip;
	IPoint2 om;
	
public:
	EPM_BevelMouseProc(EditPatchMod* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EPM_BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EPM_BevelSelectionProcessor(EPM_BevelMouseProc *mc, EditPatchMod *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EPM_BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EPM_BevelSelectionProcessor mouseProc;
	EPM_BevelMouseProc eproc;
	EditPatchMod* po;

public:
	EPM_BevelCMode(EditPatchMod* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EPM_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};



class EPM_CreateVertMouseProc : public MouseCallBack {
private:		
	EditPatchMod *po;
	IObjParam *mpIP;		
public:
	EPM_CreateVertMouseProc(EditPatchMod* mod, IObjParam *i) {po=mod;mpIP=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EPM_CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	EPM_CreateVertMouseProc proc;
	EditPatchMod *po;

public:
	EPM_CreateVertCMode(EditPatchMod* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_VERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class EPM_CreatePatchMouseProc : public MouseCallBack {
public:
	EditPatchMod *po;
	IObjParam *mpIP;		
	int verts[4];
	IPoint2 anchor, lastPoint, startPoint;

	EPM_CreatePatchMouseProc(EditPatchMod* mod, IObjParam *i);
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert, ModContext*& pTheModContext);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EPM_CreatePatchCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	EditPatchMod *po;
	EPM_CreatePatchMouseProc proc;

	EPM_CreatePatchCMode(EditPatchMod* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_PATCH; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=5; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};



class EPM_VertWeldMouseProc : public MouseCallBack {
public:
	EditPatchMod *po;
	IObjParam *mpIP;		
	int fromVert, toVert;
	IPoint2 anchor, lastPoint;
	ModContext* mpFromModContext;

	EPM_VertWeldMouseProc(EditPatchMod* mod, IObjParam *i){
		po=mod;mpIP=i;mpFromModContext=NULL;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert, ModContext*& pTheModContext);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EPM_VertWeldCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	EditPatchMod *po;
	EPM_VertWeldMouseProc proc;

	EPM_VertWeldCMode(EditPatchMod* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_VERT_WELD; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};


/*-------------------------------------------------------------------*/
// CAL-06/02/03: copy/paste tangent modes. (FID #827)

class EPM_CopyTangentMouseProc : public MouseCallBack {
	private:
		EditPatchMod *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EPM_CopyTangentMouseProc(EditPatchMod* mod, IObjParam *i) { po=mod; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EPM_CopyTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EPM_CopyTangentMouseProc eproc;
		EditPatchMod* po;
		IObjParam *ip;

	public:
		EPM_CopyTangentCMode(EditPatchMod* mod, IObjParam *i) :
			fgProc((ReferenceTarget*)mod), eproc(mod,i) {po=mod; ip=i;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_COPY_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

class EPM_PasteTangentMouseProc : public MouseCallBack {
	private:
		EditPatchMod *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EPM_PasteTangentMouseProc(EditPatchMod* mod, IObjParam *i) { po=mod; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EPM_PasteTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EPM_PasteTangentMouseProc eproc;
		EditPatchMod* po;
		IObjParam *ip;

	public:
		EPM_PasteTangentCMode(EditPatchMod* mod, IObjParam *i) :
			fgProc((ReferenceTarget*)mod), eproc(mod,i) {po=mod; ip=i;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_PASTE_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};


/*-------------------------------------------------------------------*/


// Edit Patch Flags
#define EP_DISP_RESULT 0x0100

class VertInsertRecord;
class PickPatchAttach;
class SingleRefMakerPatchMNode;   
class SingleRefMakerPatchMMtl;    

class EditPatchMod : public Modifier, IPatchOps, IPatchSelect, ISubMtlAPI, AttachMatDlgUser {
	friend class EPTempData;
	friend class EditPatchData;
	friend class XFormProc;
	friend class PatchRestore;
	friend class PVertexRightMenu;
	friend class PatchRightMenu;
	friend class PickPatchAttach;

	public:
		static HWND hSelectPanel, hOpsPanel, hSurfPanel, hSoftSelPanel;
		static BOOL rsSel, rsOps, rsSurf, rsSoftSel;	// rollup states (FALSE = rolled up)
		static IObjParam *ip;		
		static EditPatchMod *editMod;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;
		static ISpinnerControl *weldSpin;
		static ISpinnerControl *vertWeldSpin;
		static ISpinnerControl *genThreshSpin;	// CAL-05/01/03: support spline surface generation (FID #1914)
		static ISpinnerControl *stepsSpin;
#ifndef NO_OUTPUTRENDERER
		//3-18-99 to suport render steps and removal of the mental tesselator
		static ISpinnerControl *stepsRenderSpin;
#endif // NO_OUTPUTRENDERER
// 7/20/00 TH -- Relax controls
		static ISpinnerControl *relaxSpin;
		static ISpinnerControl *relaxIterSpin;
		static PickPatchAttach pickCB;
		static BOOL patchUIValid;

//watje command mode for the extrude and beevl		
		static EPM_ExtrudeCMode *extrudeMode;
		static EPM_NormalFlipCMode *normalFlipMode;
		static EPM_BevelCMode *bevelMode;
		static EPM_BindCMode *bindMode;
		static EPM_CreateVertCMode* createVertMode;
		static EPM_CreatePatchCMode* createPatchMode;
		static EPM_VertWeldCMode *vertWeldMode;
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		static EPM_CopyTangentCMode *copyTangentMode;
		static EPM_PasteTangentCMode *pasteTangentMode;

		// for the tessellation controls
		static BOOL settingViewportTess;  // are we doing viewport or renderer
		static BOOL settingDisp;          // if we're doign renderer is it mesh or displacmenent
		static ISpinnerControl *uSpin;
		static ISpinnerControl *vSpin;
		static ISpinnerControl *edgeSpin;
		static ISpinnerControl *angSpin;
		static ISpinnerControl *distSpin;
		static ISpinnerControl *mergeSpin;
		static ISpinnerControl *matSpin;
		static ISpinnerControl *matSpinSel;            
		SingleRefMakerPatchMNode* noderef;                  
		SingleRefMakerPatchMMtl* mtlref;                   
		static int attachMat;
		static BOOL condenseMat;
		// selection box size 
		static int pickBoxSize;
		// selection box size for target weld 
		static int weldBoxSize;

		// These fields track the changes in the incoming object
		BOOL objectChanged;

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );
		
		int selLevel;

		// RB:named sel sets
		BOOL namedSelNeedsFixup;	// TRUE for pre-r3 files
		Tab<TSTR*> namedSel[EP_NS_LEVELS];
		int FindSet(TSTR &setName,int level);
		void AddSet(TSTR &setName,int level);
		void RemoveSet(TSTR &setName,int level);
		void RemoveAllSets();
		void ClearSetNames();

		// Remembered info
		PatchMesh *rememberedPatch;	// NULL if using all selected patches
		int rememberedIndex;
		int rememberedData;

		BOOL displaySurface;
		BOOL displayLattice;
		int meshSteps;
//3-18-99 to suport render steps and removal of the mental tesselator
		int meshStepsRender;
		BOOL showInterior;
		BOOL usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

		// CAL-05/01/03: support spline surface generation (FID #1914)
		BOOL generateSurface;
		float genSurfWeldThreshold;
		BOOL genSurfFlipNormals;
		BOOL genSurfRmInterPatches;
		BOOL genSurfUseOnlySelSegs;

		// CAL-06/02/03: copy/paste tangent info. (FID #827)
		BOOL copyTanLength;
		BOOL tangentCopied;
		Point3 copiedTangent;

		// relax options
		BOOL relax;
		BOOL relaxViewports;
		float relaxValue;
		int relaxIter;
		BOOL relaxBoundary;
		BOOL relaxSaddle;

		BOOL meshAdaptive;	// Future use (Not used now)
		TessApprox viewTess; // for GAP tessellation
		TessApprox prodTess;
		TessApprox dispTess;
		BOOL mViewTessNormals;	// use normals from the tesselator
		BOOL mProdTessNormals;	// use normals from the tesselator
		BOOL mViewTessWeld;	// Weld the mesh after tessellation
		BOOL mProdTessWeld;	// Weld the mesh after tessellation
		BOOL propagate;

		BOOL inExtrude;
		BOOL inBevel;

		DWORD epFlags;
		void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) epFlags |= fl; else epFlags &= ~fl; }
		void ClearFlag (DWORD fl) { epFlags &= (~fl); }
		bool GetFlag (DWORD fl) { return (epFlags&fl) ? TRUE : FALSE; }

		EditPatchMod();
		~EditPatchMod();

		Interval LocalValidity(TimeValue t);
		ChannelMask ChannelsUsed()  { return EDITPAT_CHANNELS; }
		ChannelMask ChannelsChanged() 	{ return EDITPAT_CHANNELS; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
		Class_ID InputType() { return defObjectClassID; }
		
		int CompMatrix(TimeValue t, ModContext& mc, Matrix3& tm, Interval& valid);
		
		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(_T("EditPatchMod")); }
		Class_ID ClassID() { return Class_ID(EDITPATCH_CLASS_ID,0);}
		void* GetInterface(ULONG id);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		void ShowEndResultChanged (BOOL showEndResult);

		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		int SubObjectIndex(HitRecord *hitRec);

 		BOOL DependOnTopology(ModContext &mc);

		// This is necessary for object->PatchObject conversions,
		// in order to prevent the XTCObject we attach to the new PatchObject from being deleted. (Defect #525089)
		void CopyAdditionalChannels (Object *fromObj, Object *toObj) { toObj->CopyAdditionalChannels (fromObj, false); }

		// Generic xform procedure.
		void XFormVerts( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis );

		// Specialized xform for bezier handles
		void XFormHandles( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis, int object, int handleIndex );

		// Affine transform methods		
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		// The following is called before the first Move(), Rotate() or Scale() call
		void TransformStart(TimeValue t);

		// The following is called after the user has completed the Move, Rotate or Scale operation and
		// the undo object has been accepted.
		void TransformFinish(TimeValue t);		

		// The following is called when the transform operation is cancelled by a right-click and the undo
		// has been cancelled.
		void TransformCancel(TimeValue t);		

		BOOL SupportsNamedSubSels() {return TRUE;}
		void ActivateSubSelSet(TSTR &setName);
		void NewSetFromCurSel(TSTR &setName);
		void RemoveSubSelSet(TSTR &setName);
		void SetupNamedSelDropDown();
		int NumNamedSelSets();
		TSTR GetNamedSelSetName(int i);
		void SetNamedSelSetName(int i,TSTR &newName);
		void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
		BOOL GetUniqueSetName(TSTR &name);
		int SelectNamedSet();
		void NSCopy();
		void NSPaste();
		void MaybeFixupNamedSels();

//watje 12-10-98
		void DoHide(int type); 
		void DoUnHide(); 
		void DoPatchHide(); 
		void DoVertHide(); 
		void DoEdgeHide(); 

		void DoAddHook(PatchMesh *pMesh, int vert1, int seg1) ;
		void DoRemoveHook(); 

//watje bevel and extrusion stuff
		void DoExtrude() ;
		void BeginExtrude(TimeValue t); 	
		void EndExtrude (TimeValue t, BOOL accept=TRUE);		
		void Extrude( TimeValue t, float amount, BOOL useLocalNorm );
		// Supplying a patchIndex of -1 flips normals of selected patches
		// Setting 'element' to TRUE and supplying a patch index flips normals of element associated with that patch
		void DoFlipNormals(PatchMesh *pmesh=NULL, int patchIndex = -1, BOOL element=FALSE);
		void DoUnifyNormals();
		
		void DoBevel() ;
		void BeginBevel(TimeValue t); 	
		void EndBevel (TimeValue t, BOOL accept=TRUE);		
		void Bevel( TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd );

		// CAL-04/23/03: patch smooth
		void DoPatchSmooth(int type);

		// methods for creating new vertices and patches
		void CreateVertex(Point3 pt, int& newIndex);
		void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3);
		void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3,int vertIndx4);

		void DoDeleteSelected();
		void DoVertDelete();
		void DoEdgeDelete();
		void DoPatchDelete();
		void DoBreak();
		void DoPatchAdd(int type);
		void DoSubdivide(int type);
		void DoEdgeSubdivide();
		void DoPatchSubdivide();
		void DoVertWeld();
		void DoVertWeld(int fromVert, int toVert, ModContext* pTheModContext);
		void DoEdgeWeld();
		void DoPatchDetach(int copy, int reorient);

		// CAL-06/02/03: copy/paste tangent. (FID #827)
		void StartCopyTangentMode();
		void StartPasteTangentMode();
		void StartPasteTangent(EditPatchData *patchData);
		void EndPasteTangent(EditPatchData *patchData);

		BOOL CopyTangent(PatchMesh *patch, int vec);
		BOOL PasteTangent(PatchMesh *patch, int vec);

		void ClearPatchDataFlag(ModContextList& mcList,DWORD f);
		void DeletePatchDataTempData();		
		void CreatePatchDataTempData();

		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {}

		void ChangeRememberedPatch(int type);
		void ChangeSelPatches(int type);
		int RememberPatchThere(HWND hWnd, IPoint2 m);
		void SetRememberedPatchType(int type);
		void ChangeRememberedVert(int type);
		void ChangeSelVerts(int type);
		int RememberVertThere(HWND hWnd, IPoint2 m);
		void SetRememberedVertType(int type);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		IOResult SaveLocalData(ISave *isave, LocalModData *ld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
		IOResult LoadNamedSelChunk(ILoad *iload,int level);

		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next );
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_TH_EDITPATCH); }
		void ActivateSubobjSel(int level, XFormModes& modes );
		int NeedUseSubselButton() { return 0; }
		void SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert );
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		
		void SetDisplaySurface(BOOL sw);
		void SetDisplayLattice(BOOL sw);
		void SetPropagate(BOOL sw);		
		BOOL GetPropagate() {return propagate;}		
		void SetMeshSteps(int count);
		int GetMeshSteps() { return meshSteps; }
#ifndef NO_OUTPUTRENDERER
		//3-18-99 to suport render steps and removal of the mental tesselator
		void SetMeshStepsRender(int count);
		int GetMeshStepsRender() { return meshStepsRender; }
#endif // NO_OUTPUTRENDERER
		void SetShowInterior(BOOL si);
		BOOL GetShowInterior() { return showInterior; }
		// CAL-05/15/03: use true patch normals. (FID #1760)
		void SetUsePatchNormals(BOOL usePatchNorm);
		BOOL GetUsePatchNormals() { return usePatchNormals; }

		// CAL-05/01/03: support spline surface generation (FID #1914)
		void InvalidateGenSurfacePatchData();
		void SetGenerateSurface(BOOL genSurf);
		BOOL GetGenerateSurface() const { return generateSurface; }
		void SetGenSurfWeldThreshold(float thresh);
		float GetGenSurfWeldThreshold() const { return genSurfWeldThreshold; }
		void SetGenSurfFlipNormals(BOOL flipNorm);
		BOOL GetGenSurfFlipNormals() const { return genSurfFlipNormals; }
		void SetGenSurfRmInterPatches(BOOL rmInter);
		BOOL GetGenSurfRmInterPatches() const { return genSurfRmInterPatches; }
		void SetGenSurfUseOnlySelSegs(BOOL useSel);
		BOOL GetGenSurfUseOnlySelSegs() const { return genSurfUseOnlySelSegs; }
		void BuildPatchFromShape(TimeValue t, SplineShape *splShape, PatchMesh &pmesh);

		BOOL Relaxing();
		void SetRelax(BOOL v, BOOL redraw);
		void SetRelaxViewports(BOOL v, BOOL redraw);
		void SetRelaxValue(float v, BOOL redraw);
		void SetRelaxIter(int v, BOOL redraw);
		void SetRelaxBoundary(BOOL v, BOOL redraw);
		void SetRelaxSaddle(BOOL v, BOOL redraw);

// Future use (Not used now)
//		void SetMeshAdaptive(BOOL sw);
		void SetViewTess(TessApprox &tess);
		TessApprox GetViewTess() { return viewTess; }
		void SetProdTess(TessApprox &tess);
		TessApprox GetProdTess() { return prodTess; }
		void SetDispTess(TessApprox &tess);
		TessApprox GetDispTess() { return dispTess; }
		void SetTessUI(HWND hDlg, TessApprox *tess);
		BOOL GetViewTessNormals() { return mViewTessNormals; }
		void SetViewTessNormals(BOOL use);
		BOOL GetProdTessNormals() { return mProdTessNormals; }
		void SetProdTessNormals(BOOL use);
		BOOL GetViewTessWeld() { return mViewTessWeld; }
		void SetViewTessWeld(BOOL weld);
		BOOL GetProdTessWeld() { return mProdTessWeld; }
		void SetProdTessWeld(BOOL weld);

		// Get the commonality of material index for the selection (-1 indicates no commonality)
		int GetSelMatIndex();
		void SetSelMatIndex(int index);
		void SelectByMat(int index,BOOL clear);

		// Smoothing
		DWORD GetSelSmoothBits(DWORD &invalid);
		DWORD GetUsedSmoothBits();
		void SelectBySmoothGroup(DWORD bits,BOOL clear);
		void SetSelSmoothBits(DWORD bits,DWORD which);

		void PatchSelChanged();

		// from AttachMatDlgUser
		int GetAttachMat() { return attachMat; }
		void SetAttachMat(int value) { attachMat = value; }
		BOOL GetCondenseMat() { return condenseMat; }
		void SetCondenseMat(BOOL sw) { condenseMat = sw; }

		int DoAttach(INode *node, PatchMesh *attPatch, bool & canUndo);

		// Store current topology in the PatchObject
		void RecordTopologyTags();

		// Re-match named selection sets, etc. with changed topology (Call RecordTopologyTags
		// before making the changes to the shape, then call this)
		void ResolveTopoChanges();

		void RescaleWorldUnits(float f);

		int GetSubobjectLevel();
		int GetSubobjectType();	// returns EP_PATCH for EP_ELEMENT level
		void SetSubobjectLevel(int level);
		void RefreshSelType();
		void UpdateSelectDisplay();
		void UpdateGenSurfGroupControls();	// CAL-06/20/03:
		void SetGenSurfGroupEnables();		// CAL-05/01/03:
		void SetSelDlgEnables();
		void SetOpsDlgEnables();
		void SetSurfDlgEnables();
		void SetSoftSelDlgEnables( HWND hSoftSel = NULL );
		void SelectionChanged();
		void InvalidateSurfaceUI();
		BitArray *GetLevelSelectionSet(PatchMesh *patch);

		// patch select and operations interfaces, JBW 2/2/99
		void StartCommandMode(patchCommandMode mode);
		void ButtonOp(patchButtonOp opcode);

// LAM: added 9/3/00
	// UI controls access
		void GetUIParam (patchUIParam uiCode, int & ret);
		void SetUIParam (patchUIParam uiCode, int val);
		void GetUIParam (patchUIParam uiCode, float & ret);
		void SetUIParam (patchUIParam uiCode, float val);
		bool Editing () { return (ip && (editMod==this)) ? TRUE : FALSE; }

		DWORD GetSelLevel();
		void SetSelLevel(DWORD level);
		void LocalDataChanged();
	
		// Vertex Color Support...
		Color GetVertColor (int mp=0, bool *differs=NULL);
		void SetVertColor(Color clr, int mp=0);
		void SelectVertByColor(VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp=0);
		// and patch-level vertex color support
		Color GetPatchColor(int mp=0, bool *differs=NULL);
		void SetPatchColor(Color clr, int mp=0);

		// ISubMtlAPI methods:
		MtlID	GetNextAvailMtlID(ModContext* mc);
		BOOL	HasFaceSelection(ModContext* mc);
		void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
		int		GetSelFaceUniqueMtlID(ModContext* mc);
		int		GetSelFaceAnyMtlID(ModContext* mc);
		int		GetMaxMtlID(ModContext* mc);

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

//watje new patch mapping
		void ChangeMappingTypeLinear(BOOL linear);
		BOOL CheckMappingTypeLinear();
		BOOL SingleEdgesOnly();
		void MaybeClonePatchParts();

		// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
		void ShrinkSelection(int type);
		void GrowSelection(int type);
		void SelectEdgeRing();
		void SelectEdgeLoop();

		void SelectOpenEdges();
		void DoCreateShape();

// soft selection support
// should these be	private?:
		Tab<int>   mVertexEdgeDists;
		Tab<float> mVertexDists;

        float mFalloff, mPinch, mBubble;
		int   mEdgeDist, mUseEdgeDists, mAffectBackface, mUseSoftSelections;

		int  UseEdgeDists();
		void SetUseEdgeDists( int useSoftSelections );
		int  UseSoftSelections();
		void SetUseSoftSelections( int useSoftSelections );
		void InvalidateVertexWeights();
		// CAL-05/06/03: toggle shaded faces display for soft selection. (FID #1914)
		void ToggleShadedFaces();

		void ApplySoftSelectionToPatch( PatchMesh * pPatch );
		void UpdateVertexDists();
		void UpdateEdgeDists( );
		void UpdateVertexWeights();

		Point3 VertexNormal( PatchMesh * pPatch, int vIndex ); 

	};

class PickPatchAttach : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		EditPatchMod *ep;
		
		PickPatchAttach() {ep=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		HCURSOR GetHitCursor(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
	};

// Table to convert selLevel values to patch selLevel flags.
const int patchLevel[] = {PATCH_OBJECT,PATCH_VERTEX,PATCH_EDGE,PATCH_PATCH,PATCH_PATCH,PATCH_HANDLE};

// Get display flags based on selLevel.
const DWORD patchLevelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,DISP_SELEDGES,DISP_SELPATCHES,DISP_SELPATCHES,DISP_VERTTICKS|DISP_SELHANDLES};

// For hit testing...
static int patchHitLevel[] = {0,SUBHIT_PATCH_VERTS | SUBHIT_PATCH_VECS,SUBHIT_PATCH_EDGES,SUBHIT_PATCH_PATCHES,SUBHIT_PATCH_PATCHES,SUBHIT_PATCH_VECS};

class EditPatchClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE ) { return new EditPatchMod; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_EDITPATCH_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(EDITPATCH_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFEDIT);}
	void			ResetClassParams(BOOL fileReset);
	};

typedef Tab<Point3> Point3Tab;

class XFormProc {
	public:
		virtual Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt = 1.0 )=0;
		virtual void SetMat( Matrix3& mat ) {}
	};

class MoveXForm : public XFormProc {
	private:
		Point3 delta, tdelta;		
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt = 1.0 ) 
			{ return p + (tdelta * wt); }
		void SetMat( Matrix3& mat ) 
			{ tdelta = VectorTransform(Inverse(mat),delta); }
		MoveXForm(Point3 d) { delta = d; }
	};

class RotateXForm : public XFormProc {
	private:
		Matrix3 rot, trot;
		Matrix3 mMat;
		Quat    mQuat;
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt  );
		void SetMat( Matrix3& mat ) 
			{ mMat = mat; }
		RotateXForm(Quat q) { mQuat = q; }
	};

class ScaleXForm : public XFormProc {
	private:
		Matrix3 mMat, scale, tscale;
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt = 1.0 ); 
		void SetMat( Matrix3& mat ) 
			{ mMat = mat; }
		ScaleXForm(Point3 s) { scale = ScaleMatrix(s); }
	};

typedef Tab<int> IntTab;

// General-purpose patch point table -- Maintains point table for each of n polygons
class PatchPointTab {
	public:
		Point3Tab ptab;	// Patch mesh points
		Point3Tab vtab;	// Patch mesh vectors
		IntTab pttab;	// Patch point types
		PatchPointTab();
		~PatchPointTab();
		void Empty();
		void Zero();
		void MakeCompatible(PatchMesh& patch, BOOL clear=TRUE);
		PatchPointTab& operator=(PatchPointTab& from);
		BOOL IsCompatible(PatchMesh &patch);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class PatchVertexDelta {
	public:
		PatchPointTab dtab;

		void SetSize(PatchMesh &patch, BOOL load=TRUE);
		void Empty() { dtab.Empty(); }
		void Zero() { dtab.Zero(); }
		void SetVert(int i, const Point3& p) { dtab.ptab[i] = p; }
		void SetVertType(int i, int k) { dtab.pttab[i] = k; }
		void SetVec(int i, const Point3& p) { dtab.vtab[i] = p; }
		void MoveVert(int i, const Point3& p) { dtab.ptab[i] += p; }
		void MoveVec(int i, const Point3& p) { dtab.vtab[i] += p; }
		void Apply(PatchMesh& patch);
		void UnApply(PatchMesh& patch);
		PatchVertexDelta& operator=(PatchVertexDelta& from) { dtab = from.dtab; return *this; }
		void ApplyHandlesAndZero(PatchMesh &patch, int handleVert);
		BOOL IsCompatible(PatchMesh &patch) { return dtab.IsCompatible(patch); }
		void RescaleWorldUnits(float f) { dtab.RescaleWorldUnits(f); }
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class AdjEdgeList;
class EPTempData;

/*-------------------------------------------------------------------*/

// Class for recording changes -- This is used to reconstruct an object from the original whenever
// the modifier is re-entered or whenever the system needs to reconstruct an object's cache.  This may be
// slow if a lot of changes have been recorded, but it's about the only way to properly reconstruct an object
// because the output of one operation becomes the input of the next.

// These are used as follows:
// When a user makes a modification to an object, a StartChangeGroup call needs to be made to the EditPatchData
// object.  Then a change record needs to be added for each sub-operation that makes up the modification.  These
// records are owned by the EditPatchData object, but they should also be referenced by the undo object for that
// operation.  If an undo is done, ownership of the modification record transfers to the undo/redo object and the
// record is REMOVED (NOT DELETED) from the EditPatchData object.  This keeps the record around for a redo operation
// but removes it from the list of records for the modifier.  If the undo is redone, ownership transfers back to
// the modifier, when it is re-added to the modification record list.

// Note that this class contains load and save methods, necessary because the modifier needs to be able to save
// and load them.  When you subclass off of this, be sure your load and save methods call the base class's first!

class PatchRestore;

class PModRecord {
	public:
		virtual BOOL Redo(PatchMesh *patch,int reRecord)=0;
		virtual IOResult Load(ILoad *iload)=0;
	};

typedef PModRecord* PPModRecord;
typedef Tab<PPModRecord> ModRecordTab;

/*-------------------------------------------------------------------*/

// Here are the types of modification records we use!

#define CLEARVERTSELRECORD_CHUNK	0x2000
#define SETVERTSELRECORD_CHUNK		0x2001
#define INVERTVERTSELRECORD_CHUNK	0x2002
#define CLEAREDGESELRECORD_CHUNK	0x2005
#define SETEDGESELRECORD_CHUNK		0x2006
#define INVERTEDGESELRECORD_CHUNK	0x2007
#define CLEARPATCHSELRECORD_CHUNK	0x2010
#define SETPATCHSELRECORD_CHUNK		0x2011
#define INVERTPATCHSELRECORD_CHUNK	0x2012
#define CLEARVECSELRECORD_CHUNK		0x2015		// CAL-06/10/03: (FID #1914)
#define SETVECSELRECORD_CHUNK		0x2016		// CAL-06/10/03: (FID #1914)
#define INVERTVECSELRECORD_CHUNK	0x2017		// CAL-06/10/03: (FID #1914)
#define VERTSELRECORD_CHUNK			0x2020
#define EDGESELRECORD_CHUNK			0x2025
#define PATCHSELRECORD_CHUNK		0x2030
#define VECSELRECORD_CHUNK			0x2035		// CAL-06/10/03: (FID #1914)
#define VERTMOVERECORD_CHUNK		0x2040
#define VECMOVERECORD_CHUNK			0x2045		// CAL-06/10/03: (FID #1914)
#define PATCHDELETERECORD_CHUNK		0x2050
#define VERTDELETERECORD_CHUNK		0x2060
#define PATCHCHANGERECORD_CHUNK		0x2070
#define VERTCHANGERECORD_CHUNK		0x2080
#define PATCHADDRECORD_CHUNK		0x2090
#define EDGESUBDIVIDERECORD_CHUNK	0x20A0
#define PATCHSUBDIVIDERECORD_CHUNK	0x20B0
#define VERTWELDRECORD_CHUNK		0x20C0
#define PATTACHRECORD_CHUNK			0x20D0
#define PATCHDETACHRECORD_CHUNK		0x20E0
#define PATCHMTLRECORD_CHUNK		0x20F0
								
// CAL-06/10/03: add handle sub-object mode. (FID #1914)
class ClearPVecSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
class SetPVecSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
class InvertPVecSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPVertSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPVertSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPVertSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPEdgeSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPEdgeSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPEdgeSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPatchSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPatchSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPatchSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
class PVecSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PEdgeSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
class PVecMoveRecord : public PModRecord {
	public:
		PatchVertexDelta delta;	// Position changes for each vertex (Wasteful!  Change later?)
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertMoveRecord : public PModRecord {
	public:
		PatchVertexDelta delta;	// Position changes for each vertex (Wasteful!  Change later?)
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchDeleteRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the spline looked before the delete
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertDeleteRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchChangeRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch mesh looked before the change
		int index;
		int type;
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertChangeRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch mesh looked before the change
		int index;
		int type;
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchAddRecord : public PModRecord {
	public:
		BOOL postWeld;			// Present in MAX 2.0 and up
		int type;				// 3 or 4 sides!
		PatchMesh oldPatch;		// How the patch looked before the addition
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class EdgeSubdivideRecord : public PModRecord {
	public:
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchSubdivideRecord : public PModRecord {
	public:
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertWeldRecord : public PModRecord {
	public:
		float thresh;			// Weld threshold
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PAttachRecord : public PModRecord {
	public:
		PatchMesh attPatch;			// The patch we're attaching
		int oldPatchCount;		// The number of splines present before attaching
		int mtlOffset;
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchDetachRecord : public PModRecord {
	public:
		int copy;
		PatchMesh oldPatch;
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

typedef Tab<MtlID> MtlIDTab;

class PatchMtlRecord : public PModRecord {
	public:
		MtlIDTab materials;		// Materials from selected patches
		MtlID index;				// New material index assigned
		BOOL Redo(PatchMesh *patch,int reRecord);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// EPMapxxx Flags:
#define EPMAP_ALTERED (1<<0)

// Vertex/vector Mapping class -- Gives mapping from vert in original patch to
// vert in modified patch along with position delta

class EPMapVert {
	public:
		int vert;
		Point3 delta;		// The delta we've applied
		DWORD flags;		// See above
		EPMapVert() { vert = 0; flags = 0; delta = Point3(0,0,0); }
		EPMapVert(int v, Point3 &d) { vert = v; flags = 0; delta = d; }
	};

class EPMapPatch {
	public:
		int patch;
		DWORD flags;		// See above
		EPMapPatch() { patch = 0; flags = 0; }
		EPMapPatch(int p) { patch = p; flags = 0; }
	};

class EPMapUVVert {
	public:
		int vert;
		DWORD flags;		// See above
		EPMapUVVert() { vert = 0; flags = 0; }
		EPMapUVVert(int v) { vert = v; flags = 0; }
	};

class EPMapper {
	public:
		int verts;
		EPMapVert *vertMap;
		int vecs;
		EPMapVert *vecMap;
		int patches;
		EPMapPatch *patchMap;
		int colors;
		EPMapUVVert *colorMap;
		int illums;
		EPMapUVVert *illumMap;
		int alphas;
		EPMapUVVert *alphaMap;
		EPMapper() {
			verts = vecs = patches = colors = illums = alphas = 0;
			vertMap = vecMap = NULL;
			patchMap = NULL;
			colorMap = illumMap = alphaMap = NULL;
			}
		~EPMapper();
		// Set up remap data structures.
		void Build(PatchMesh &patch);
		// Apply our stored delta info to the specified patch
		void ApplyDeltas(PatchMesh &inPatch, PatchMesh &outPatch);
		// Recompute the deltas we have stored
		// This is done after the modifier's user interaction changes the shape
		void RecomputeDeltas(BOOL checkTopology, PatchMesh &patch, PatchMesh &oldPatch);
		// Record the topology tags in the specified shape
		void RecordTopologyTags(PatchMesh &patch);
		EPMapper& operator=(EPMapper &from);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// EditPatchData flags
#define EPD_BEENDONE			(1<<0)
#define EPD_UPDATING_CACHE		(1<<1)
#define EPD_HASDATA				(1<<2)
#define EMD_HELD				(1<<3) // equivalent to A_HELD

// This is the data that each mod app will have.
class EditPatchData : public LocalModData {
	public:
		BOOL r3Fix;
		BOOL handleFlag;
		int handleVert;

		// CAL-05/01/03: support spline surface generation (FID #1914)
		BOOL splineInput;

		// Stuff we need to have for the patch's mesh conversion -- These are
		// Here because they're kind of a global change -- not undoable.
		int meshSteps;
//3-18-99 to suport render steps and removal of the mental tesselator
		int meshStepsRender;
		BOOL showInterior;
		BOOL usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

		BOOL meshAdaptive;	// Future use (Not used now)
		TessApprox viewTess;
		TessApprox prodTess;
		TessApprox dispTess;
		BOOL mViewTessNormals;	// use normals from the tesselator
		BOOL mProdTessNormals;	// use normals from the tesselator
		BOOL mViewTessWeld;	// Weld the mesh after tessellation
		BOOL mProdTessWeld;	// Weld the mesh after tessellation
		BOOL displaySurface;
		BOOL displayLattice;

		DWORD flags;

		// This records the changes to the incoming object.
		ModRecordTab changes;

		// A pointer to the change record's vertex delta object
		PatchVertexDelta vdelta;

		// RB: Named selection set lists
		GenericNamedSelSetList hselSet;  // Vector	// CAL-06/10/03: (FID #1914)
		GenericNamedSelSetList vselSet;  // Vertex
		GenericNamedSelSetList eselSet;  // Edge
		GenericNamedSelSetList pselSet;  // Patch

		// While an object is being edited, this exists.
		EPTempData *tempData;

		// The mapping for the edited patch (links original geometry indices to 
		// edited geometry indices)
		EPMapper mapper; 

		// The final edited patch
		PatchMesh finalPatch;

		// Automatic edge array
		IntTab autoEdges;

		// "Held" data -- The object we're modifying.  Held for later reference by PreUpdateChanges
		// so that UpdateChanges can detect deltas, not saved
		PatchMesh *oldPatch;

		EditPatchData(EditPatchMod *mod);
		EditPatchData(EditPatchData& emc);
		~EditPatchData();
		
		// Applies modifications to a patchObject
		void Apply(EditPatchMod *mod,TimeValue t,PatchObject *patchOb,int selLevel);

		// Invalidates any caches affected by the change.
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		// If this is the first edit, then the delta arrays will be allocated
		void BeginEdit(TimeValue t);

		LocalModData *Clone() { return new EditPatchData(*this); }
		
		void SetFlag(DWORD fl, BOOL val=TRUE) { if (val) flags |= fl; else flags &= ~fl; }
		void ClearFlag(DWORD fl) { flags &= (~fl); }
		DWORD GetFlag(DWORD fl) { return (flags&fl); }

		EPTempData *TempData(EditPatchMod *mod);

		// Change recording functions:
		void ClearHandleFlag() { handleFlag = FALSE; }
		void SetHandleFlag(int vert) { handleVert = vert; handleFlag = TRUE; }
		BOOL DoingHandles() { return handleFlag; }
		void ApplyHandlesAndZero(PatchMesh &patch) { vdelta.ApplyHandlesAndZero(patch, handleVert); }
		void RescaleWorldUnits(float f);

		// MAXr4: New recording system
		void PreUpdateChanges(PatchMesh *patch, BOOL checkTopology=TRUE);	// Call before modifying (replaces old RecordTopologyTags call)
		void UpdateChanges(PatchMesh *patch, BOOL checkTopology=TRUE, BOOL held=FALSE);	// Call after modifying

		// Named selection set access
		GenericNamedSelSetList &GetSelSet(EditPatchMod *mod);	// Get the one for the current subobject selection level
		GenericNamedSelSetList &GetSelSet(int level);	// Get the one for the specified subobject selection level

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Debugging
		void DumpData();
	};

// My generic restore class

class PatchRestore : public RestoreObj {
	public:
		PatchMesh oldPatch, newPatch;
		BOOL gotRedo;
		TimeValue t;
		EditPatchData *epd;
		EditPatchMod *mod;
		TSTR where;
		
		PatchRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch, TCHAR *id=_T(""));

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {mod->ClearAFlag(A_HELD);}
		TSTR Description() {
			TSTR string;
			string.printf(_T("Generic patch restore [%s]"),where);
			return string;
			}
	};

// Patch selection restore class

class PatchSelRestore : public RestoreObj {
	public:
		BitArray oldHSel, newHSel;		// CAL-06/10/03: (FID #1914)
		BitArray oldVSel, newVSel;
		BitArray oldESel, newESel;
		BitArray oldPSel, newPSel;
		BOOL gotRedo;
		TimeValue t;
		EditPatchData *epd;
		EditPatchMod *mod;

		PatchSelRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch);

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {mod->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("Patch Select restore")); }
	};

/*-------------------------------------------------------------------*/
// CAL-05/01/03: support spline surface generation (FID #1914)
//				 The following 4 classes are copied from the Surface modifier.

class BindSplines
{
public:
	SplineKnot a,b,c;
	Point3 subHook;
};

class PatchVerts {
public:
	int used,Index;
	Tab<int> OutVerts;
	Tab<int> InVerts;
	Tab<int> OutVecs;
	Tab<int> InVecs;
	//neeed to add knot type

	PatchVerts() {
				OutVerts.ZeroCount();
				InVerts.ZeroCount();
				OutVecs.ZeroCount();
				InVecs.ZeroCount();
				used = 0;
				};
	};

class PatchFaces {
public:
	int face3;
	int  a,b,c,d;
	int  va,vb,vc,vd;
	int  ab,ba,
		 bc,cb,
		 ca,ac,
		 cd,dc,da,ad;
	char tagged;
	char interior;
	};

class PatchData
	{
public:
	Tab<int> hookVerts;
	Tab<int> hookPoints;
	Tab<Point3>	VecList;
	Tab<int>    VecCount;
	Tab<Point3>	VertList;

	Tab<PatchVerts*> VertData;
	Tab<PatchFaces> FaceData;

	Tab<int> Verts33;

	PatchData(){VertData.ZeroCount();FaceData.ZeroCount();VecList.ZeroCount();VertList.ZeroCount();VecCount.ZeroCount();};
	
	void AddVert(Point3 vert,float threshold, BOOL isHook = FALSE, BOOL isHookPoint=FALSE);

	void AddVecs2(Point3 vert,Point3 outvec,Point3 invec,
			Point3 outvert,Point3 invert,float threshold);
	void AddVecs1Out(Point3 vert,Point3 outvec,
			Point3 outvert,float threshold);
	void AddVecs1In(Point3 vert,Point3 invec,
			Point3 invert,float threshold);

	int FindVec(int SourceFace, int Destface);

	void AddFace3(int a, int b, int c);
	void AddFace4(int a, int b, int c, int d);

	int RecurseFaces3(int FirstVert, int CurrentVertex,int *List, int Count);
	int RecurseFaces4(int FirstVert, int CurrentVertex,int *List, int Count);

	//void AddToFaceList(int face, int vert);

	BOOL CreateFaceData();

	void FlipFace(int face);
	void CheckFace(int face,int a, int b);
	//int CheckFace2(int i,int a, int b);

	void UnifyNormals(int flip);

	void CheckData(int face);
	//void CheckData2(int face);
	void RemoveInteriorFaces();
	void DeleteInteriorFaces(int rinterior, int rcaps);
	void RecombineVectors();
	void MoveVectors(int a,int b);

	//int ValidateFace(int source,int dest);
    //int ValidateFace2(int source, int destvert);
	//int VecVertCount(int face);

	void NukeDegenerateHookPatches(BitArray hooksBA);
	};

/*-------------------------------------------------------------------*/

class EPTempData {
	private:
		PatchMesh		*patch;
		Interval 		patchValid;
		
		EditPatchMod 	*mod;
		EditPatchData 	*patchData;

	public:		
		
		~EPTempData();
		EPTempData(EditPatchMod *m,EditPatchData *md);
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		PatchMesh		*GetPatch(TimeValue t);
		
		BOOL PatchCached(TimeValue t);
		void UpdateCache(PatchObject *patchOb);
		EditPatchMod	*GetMod() { return mod; }
	};


// Patch hit override functions

extern void SetPatchHitOverride(int value);
extern void ClearPatchHitOverride();


//az -  042803  MultiMtl sub/mtl name support
INode* GetNode (EditPatchMod *ep);
void GetMtlIDList(Mtl *mtl, NumList& mtlIDList);
void GetEPatchMtlIDList(EditPatchMod *ep, NumList& mtlIDList);
BOOL SetupMtlSubNameCombo (HWND hWnd, EditPatchMod *ep);
void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin);
void ValidateUINameCombo (HWND hWnd, EditPatchMod *ep);


#define  SRMM_CLASS_ID 0x2e5b62c9
class SingleRefMakerPatchMMtl: public SingleRefMaker {
public:
	HWND hwnd;
	EditPatchMod *ep;
	RefTargetHandle rtarget;
	SingleRefMakerPatchMMtl() {hwnd = NULL; ep = NULL; rtarget = NULL; }
	~SingleRefMakerPatchMMtl() { 
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


#define  SRMN_CLASS_ID 0x70c06cf7
class SingleRefMakerPatchMNode: public SingleRefMaker {
public:
	HWND hwnd;
	EditPatchMod *ep;
	RefTargetHandle rtarget;
	SingleRefMakerPatchMNode() {hwnd = NULL; ep = NULL; rtarget = NULL; }
	~SingleRefMakerPatchMNode() { 
		theHold.Suspend();  
		DeleteAllRefsFromMe();
		theHold.Resume();  
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
		 	INode* singleNode = GetNode(ep);
		    switch(message) {
			case REFMSG_NODE_MATERIAL_CHANGED:
				if (singleNode) {
					ep->mtlref->SetRef(singleNode->GetMtl());
					if(hwnd) ValidateUINameCombo(hwnd, ep);
				}
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMN_CLASS_ID; }
};

#endif // __EDITPATCH_H__
