/**********************************************************************
 *<
	FILE: editspl.h

	DESCRIPTION:  Edit BezierShape OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 25 April 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __EDITSPLINE_H__
#define __EDITSPLINE_H__

#define Alert(x) MessageBox(GetActiveWindow(),x,_T("Alert"),MB_OK);

#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "sbmtlapi.h"
#include "stdmat.h"

#define MAX_MATID	0xffff

#define EDITSPL_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO)

// These are values for selLevel.
#define ES_OBJECT	0
#define ES_VERTEX	1
#define ES_SEGMENT	2
#define ES_SPLINE	3

// Named selection set levels:
#define ES_NS_VERTEX 0
#define ES_NS_SEGMENT 1
#define ES_NS_SPLINE 2
// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { ES_NS_VERTEX, ES_NS_VERTEX, ES_NS_SEGMENT, ES_NS_SPLINE };

#define CID_OUTLINE		CID_USER + 201	// Spline only
#define CID_SEGBREAK	CID_USER + 202	// Segment only
#define CID_SEGREFINE	CID_USER + 203	// Spline/Vertex
#define CID_VERTCONNECT	CID_USER + 204	// Vertex only
#define CID_VERTINSERT	CID_USER + 205	// All levels
#define CID_BOOLEAN		CID_USER + 206	// Spline only
#define CID_CREATELINE	CID_USER + 207	// All levels
#define CID_CROSSINSERT	CID_USER + 208	// Spline only
#define CID_FILLET		CID_USER + 209
#define CID_CHAMFER		CID_USER + 210
#define CID_TRIM		CID_USER + 211
#define CID_EXTEND		CID_USER + 212

//watje
#define CID_SPLINEBIND	CID_USER + 213
#define CID_REFINECONNECT	CID_USER + 214

// CAL-02/26/03: Add Cross Section. (FID #827)
#define CID_CROSSSECTION	CID_USER + 215
// CAL-03/03/03: copy/paste tangent. (FID #827)
#define CID_COPYTANGENT		CID_USER + 216
#define CID_PASTETANGENT	CID_USER + 217

#define UNDEFINED	0xffffffff

// Edit Spline Flags
#define ES_DISP_RESULT 0x0100
// Use additional vertex selection (marked as SPLINEKNOT_ADD_SEL) in EditSplineMod::XFormVerts()
#define ES_ADDED_SELECT 0x0200

class VertInsertRecord;
class PickSplineAttach;
class SingleRefMakerSplineMNode;  
class SingleRefMakerSplineMMtl;   

class EditSplineMod : public Modifier, ISplineOps, ISplineSelect, ISubMtlAPI, AttachMatDlgUser {
	friend INT_PTR CALLBACK SplineSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend INT_PTR CALLBACK SplineOpsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend INT_PTR CALLBACK SplineSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend INT_PTR CALLBACK SelectByDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

	friend class ESTempData;
	friend class EditSplineData;
	friend class XFormProc;
	friend class OutlineCMode;
	friend class FilletCMode;
	friend class ESChamferCMode;
	friend class SegBreakCMode;
	friend class SegRefineCMode;
	friend class CrossInsertCMode;
	friend class VertConnectCMode;
	friend class VertInsertCMode;
	friend class CreateLineCMode;
	friend class CrossSectionCMode;
	friend class BooleanCMode;
	friend class TrimCMode;
	friend class ExtendCMode;
	friend class CopyTangentCMode;
	friend class PasteTangentCMode;
	friend class OutlineMouseProc;
	friend class FilletMouseProc;
	friend class ESChamferMouseProc;
	friend class SegBreakMouseProc;
	friend class SegRefineMouseProc;
	friend class CrossInsertMouseProc;
	friend class VertConnectMouseProc;
	friend class VertInsertMouseProc;
	friend class CreateLineMouseProc;
	friend class BooleanMouseProc;
	friend class TrimMouseProc;
	friend class ShapeRestore;
	friend class ESRightMenu;
	friend class ESMBackspaceUser;
	friend class ESIBackspaceUser;
	friend class PickSplineAttach;
	friend class AttachHitByName;
//watje
	friend class BindMouseProc;
	friend class BindCMode;
	friend class RefineConnectMouseProc;
	friend class RefineConnectCMode;


	public:
		static IObjParam *ip;		
		static EditSplineMod *editMod;
		static HWND hSelectPanel, hOpsPanel, hSurfPanel, hSelectBy, hSoftSelPanel;
		static BOOL rsSel, rsSoftSel, rsOps, rsSurf;	// rollup states (FALSE = rolled up)
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;
		static OutlineCMode *outlineMode;
		static FilletCMode *filletMode;
		static ESChamferCMode *chamferMode;
		static SegBreakCMode *segBreakMode;
		static SegRefineCMode *segRefineMode;
		static CrossInsertCMode *crossInsertMode;
		static VertConnectCMode *vertConnectMode;
		static VertInsertCMode *vertInsertMode;
		static CreateLineCMode *createLineMode;
		static CrossSectionCMode *crossSectionMode;
		static BooleanCMode *booleanMode;
		static TrimCMode *trimMode;
		static ExtendCMode *extendMode;
		static CopyTangentCMode *copyTangentMode;
		static PasteTangentCMode *pasteTangentMode;
		static ISpinnerControl *outlineSpin;
		static ISpinnerControl *filletSpin;
		static ISpinnerControl *chamferSpin;
		static ISpinnerControl *weldSpin;
		static ISpinnerControl *crossSpin;
		static ISpinnerControl *divSpin;
		static ISpinnerControl *matSpin;
		static ISpinnerControl *matSpinSel;            
		SingleRefMakerSplineMNode* noderef;                 
		SingleRefMakerSplineMMtl* mtlref;                   


		// endpoint autoweld spinner
		static ISpinnerControl *pEndPointAutoConnectWeldSpinner;

		// CAL-05/23/03: Threshold for extending existing splines when Shift-Move Copy. (FID #827)
		static ISpinnerControl *pConnectCopyThreshSpinner;

		//2-1-99 watje
		static ISpinnerControl *selectAreaSpin;

		static ICustButton *iUnion;
		static ICustButton *iSubtraction;
		static ICustButton *iIntersection;
		static ICustButton *iMirrorHorizontal;
		static ICustButton *iMirrorVertical;
		static ICustButton *iMirrorBoth;

		static BOOL inOutline;
		static BOOL inFillet;
		static BOOL inChamfer;
		static BOOL inSegBreak;
		static int boolType;
		static int mirrorType;
		static PickSplineAttach pickCB;
		static BOOL segUIValid;
		static int attachMat;
		static BOOL condenseMat;

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message ) { return REF_SUCCEED; }
		
		BOOL showVertNumbers;
		BOOL SVNSelectedOnly;

		int selLevel;

		// Vertex insertion information
		BezierShape *insertShape;
		Spline3D *insertSpline;
		int insertPoly;
		int insertVert;
		INode *insertNode;
		Matrix3 insertTM;	// Transform for the insert node
		EditSplineData *insertShapeData;

		// Create line data
		BezierShape *createShape;
		INode *createNode;
		Matrix3 createTM;	// Transform for the create node
		EditSplineData *createShapeData;

		// Boolean info
		BezierShape* boolShape;
		int boolPoly1;

		// Remembered info
		BezierShape *rememberedShape;	// NULL if using all selected verts
		int rememberedPoly;
		int rememberedIndex;
		int rememberedData;
	
		// Fillet and chamfer upper limit
		float FCLimit;

		// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
		BOOL connectCopy;
		float connectCopyThreshold;

		// CAL-02/24/03: Add Cross Section mode. (FID #827)
		int newKnotType;

		// CAL-03/03/03: copy/paste tangent info. (FID #827)
		BOOL copyTanLength;
		BOOL tangentCopied;
		Point3 copiedTangent;

		// Named selections
		Tab<TSTR*> namedSel[3];
		int FindSet(TSTR &setName,int level);
		void AddSet(TSTR &setName,int level);
		void RemoveSet(TSTR &setName,int level);
		void RemoveAllSets();
		void ClearSetNames();

		DWORD esFlags;
		void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) esFlags |= fl; else esFlags &= ~fl; }
		void ClearFlag (DWORD fl) { esFlags &= (~fl); }
		bool GetFlag (DWORD fl) { return (esFlags&fl) ? TRUE : FALSE; }

		EditSplineMod();
		~EditSplineMod();

		Interval LocalValidity(TimeValue t);
		ChannelMask ChannelsUsed()  { return EDITSPL_CHANNELS; }
		ChannelMask ChannelsChanged() { return EDITSPL_CHANNELS; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
		Class_ID InputType() { return Class_ID(SPLINESHAPE_CLASS_ID,0); }
		
		int CompMatrix(TimeValue t, ModContext& mc, Matrix3& tm, Interval& valid);
		
		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(_T("EditSplineMod")); }
		Class_ID ClassID() { return Class_ID(EDITSPLINE_CLASS_ID,0);}
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

		// Generic xform procedure.
		void XFormVerts( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis );

		// The following is called before the first Move(), Rotate() or Scale() call
		void TransformStart(TimeValue t);

		// The following is called after the user has completed the Move, Rotate or Scale operation and
		// the undo object has been accepted.
		void TransformFinish(TimeValue t);		

		// The following is called when the transform operation is cancelled by a right-click and the undo
		// has been cancelled.
		void TransformCancel(TimeValue t);		

		// Specialized xform for bezier handles
		void XFormHandles( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis, int object );

		// Affine transform methods		
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		// Named selection support
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

		void BeginOutlineMove(TimeValue t);
		void BeginFilletMove(TimeValue t);
		void BeginChamferMove(TimeValue t);
		void OutlineMove( TimeValue t, float amount );
		void FilletMove( TimeValue t, float amount );
		void ChamferMove( TimeValue t, float amount );
		void EndMoveModes(TimeValue t, BOOL accept=TRUE);	// End all Moves (below)
		void EndOutlineMove(TimeValue t,BOOL accept=TRUE);
		void EndFilletMove(TimeValue t,BOOL accept=TRUE);
		void EndChamferMove(TimeValue t,BOOL accept=TRUE);

		void StartOutlineMode();
		void StartFilletMode();
		void StartChamferMode();
		void DoOutline();
		void DoFillet();
		void DoChamfer();
		void StartSegBreakMode();
		void DoSegBreak(ViewExp *vpt, BezierShape *shape, int poly, int seg, IPoint2 p);
		void StartSegRefineMode();
		void DoSegRefine(ViewExp *vpt, BezierShape *shape, int poly, int seg, IPoint2 p);
		void StartCrossInsertMode();
		void DoCrossInsert(ViewExp *vpt, BezierShape *shape, int poly1, int seg1, int poly2, int seg2, IPoint2 p);
		void DoSegDivide(int divisions);
		void DoPolyReverse();
		void DoPolyExplode();
		void DoExplodeToObjects();
		void StartVertConnectMode();
		void DoVertConnect(ViewExp *vpt, BezierShape *shape, int poly1, int vert1, int poly2, int vert2);
		void StartVertInsertMode(int controlID);
		int StartVertInsert(ViewExp *vpt, BezierShape *shape, int poly, int seg, int vert, EditSplineMod **mod);
		void EndVertInsert();
		void StartCreateLineMode();
		BOOL StartCreateLine(BezierShape **shape);
		void EndCreateLine(BOOL altered=TRUE);
		void StartCrossSectionMode();
		BOOL StartCrossSection(EditSplineData **shapeData);
		void EndCrossSection(BOOL acceptUndo);
		BOOL BooleanStartUp();
		void StartBooleanMode();
		void DoBoolean(int poly2);
		void StartTrimMode();
		void StartExtendMode();
		void HandleTrimExtend(ViewExp *vpt, ShapeHitData *hit, IPoint2 &m, int operation);
		void MultiAttachObject(INodeTab &nodeTab);

		// from AttachMatDlgUser
		int GetAttachMat() { return attachMat; }
		void SetAttachMat(int value) { attachMat = value; }
		BOOL GetCondenseMat() { return condenseMat; }
		void SetCondenseMat(BOOL sw) { condenseMat = sw; }

		int DoAttach(INode *node, bool & canUndo);
		void DoCrossSection(EditSplineData *shapeData, Tab<int> &splineIndices);
		void DoVertBreak();
		void DoVertWeld();
		void DoMakeFirst();
		void DoVertDelete();
		void DoSegDelete();
		void DoSegDetach(int sameShape, int copy, int reorient);
		void DoPolyClose();
		void DoPolyDetach(int copy, int reorient);
		void DoPolyMirror(int type, int copy, BOOL aboutPivot);
		void DoPolyDelete();
		void SetBoolOperation(int type) { boolType = type; }
		void SetMirrorOperation(int type) { mirrorType = type; }
		int GetBoolOperation() { return boolType; }
		int GetMirrorOperation() { return mirrorType; }
		int GetBoolCursorID();
		int GetBoolMirrString(int type);
		void SetBooleanButton();
		void SetMirrorButton();
		void ChangeSelVerts(int type);
		void ChangeRememberedVert(int type);
		int RememberVertThere(HWND hWnd, IPoint2 m);
		void SetRememberedVertType(int type);
		void ChangeSelSegs(int type);
		void ChangeRememberedSeg(int type);
		int RememberSegThere(HWND hWnd, IPoint2 m);
		void SetRememberedSegType(int type);
		void ChangeSelPolys(int type);
		void ChangeRememberedPoly(int type);
		int RememberPolyThere(HWND hWnd, IPoint2 m);
		void SetRememberedPolyType(int type);
		
		int GetPointIndex(const Tab<Point3> &vertList, const Point3 &point) const;
		
		void RefreshSelType();
		void UpdateSelectDisplay();
		void SetSelDlgEnables();
		void SetOpsDlgEnables();
		void SetSurfDlgEnables();
		void SetSoftSelDlgEnables( HWND hSoftSel = NULL );
		void SelectionChanged();
		void InvalidateSurfaceUI();

		// Get the commonality of material index for the selection (-1 indicates no commonality)
		int GetSelMatIndex();
		void SetSelMatIndex(int index);
		void SelectByMat(int index,BOOL clear);

		BOOL SingleObjectMod();		// returns TRUE if just one input object
		BezierShape *SingleObjectShape(INode **node = NULL); // returns the shape for the one input object

		void ClearShapeDataFlag(ModContextList& mcList,DWORD f);
		void DeleteShapeDataTempData();		
		void CreateShapeDataTempData();

		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {}

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
		TCHAR *GetObjectName() { return GetString(IDS_TH_EDITSPLINE); }
		int GetSubobjectLevel();
		void SetSubobjectLevel(int level);
		void ActivateSubobjSel(int level, XFormModes& modes );
		int NeedUseSubselButton() { return 0; }
		void SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert );
		void ClearSelection(int selLevel);		
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		void MaybeSelectSingleSpline(BOOL makeUndo = FALSE);
		BOOL AnyPolysSelected();
		void SetFCLimit();

		void RescaleWorldUnits(float f);

//watje
		
		BOOL showVerts;
		BOOL smoothRefineConnect;
		BOOL closedRefineConnect;
		BOOL bindFirst, bindLast;

//2-1-99 watje
		BOOL rConnect;
		BOOL useAreaSelect;
		float areaSelect;

		int startSegRC, startSegSplineRC;
		int startSplineRC;
		int endSegRC, endSegSplineRC;
		int endSplineRC;
		Tab<Point3> pointList;

//2-21-99 watje 
		SplineKnot knotPoint1, knotPoint2;

		static BindCMode *bindMode;
		static RefineConnectCMode *refineConnectMode;
		void StartBindMode();
		void DoBind(int poly1, int vert1, int poly2, int vert2);
		void DoUnBind();
		void DoHide();
		void DoUnhide();
		void DoCycleVerts();

		void StartRefineConnectMode();
		void EndRefineConnectMode();
		void DoRefineConnect(ViewExp *vpt, BezierShape *shape, int poly, int seg, IPoint2 p);

//2-1-99 watje
		void DoVertFuse();

		// CAL-03/03/03: copy/paste tangent. (FID #827)
		void StartCopyTangentMode();
		void StartPasteTangentMode();
		void StartPasteTangent(EditSplineData *shapeData);
		void EndPasteTangent(EditSplineData *shapeData);

		BOOL CopyTangent(BezierShape *shape, int poly, int vert);
		BOOL PasteTangent(BezierShape *shape, int poly, int vert);

		// spline select and operations interfaces, JBW 2/1/99
		void StartCommandMode(splineCommandMode mode);
		void ButtonOp(splineButtonOp opcode);
// LAM: added 9/3/00
	// UI controls access
		void GetUIParam (splineUIParam uiCode, int & ret);
		void SetUIParam (splineUIParam uiCode, int val);
		void GetUIParam (splineUIParam uiCode, float & ret);
		void SetUIParam (splineUIParam uiCode, float val);
		bool Editing () { return (ip && (editMod==this)) ? TRUE : FALSE; }

		DWORD GetSelLevel();
		void SetSelLevel(DWORD level);
		void LocalDataChanged();

		// 4/11/00 TH
		void SelectBySegment();
		void SelectBySpline();

		// ISubMtlAPI methods:
		MtlID	GetNextAvailMtlID(ModContext* mc);
		BOOL	HasFaceSelection(ModContext* mc);
		void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
		int		GetSelFaceUniqueMtlID(ModContext* mc);
		int		GetSelFaceAnyMtlID(ModContext* mc);
		int		GetMaxMtlID(ModContext* mc);


		// soft selection support
		// should these be	private?:
        float mFalloff, mPinch, mBubble;
		int   mEdgeDist, mUseEdgeDists, mUseSoftSelections;

		int  UseEdgeDists();
		void SetUseEdgeDists( int useSoftSelections );
		int  UseSoftSelections();
		void SetUseSoftSelections( int useSoftSelections );
		void InvalidateVertexWeights();

		void ApplySoftSelectionToSpline( BezierShape * pSpline );
		void UpdateVertexDists();
		void UpdateEdgeDists( );
		void UpdateVertexWeights();

		// NS: New SubObjTypes API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);
	};

class PickSplineAttach : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		EditSplineMod *es;
		
		PickSplineAttach() {es=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		HCURSOR GetHitCursor(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
	};

// Table to convert selLevel values to shape selLevel flags.
const int shapeLevel[] = {SHAPE_OBJECT,SHAPE_VERTEX,SHAPE_SEGMENT,SHAPE_SPLINE};

// Get display flags based on selLevel.
const DWORD shapeLevelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,DISP_VERTTICKS|DISP_SELSEGMENTS,DISP_VERTTICKS|DISP_SELPOLYS};

// For hit testing...
const int shapeHitLevel[] = {0,SUBHIT_SHAPE_VERTS,SUBHIT_SHAPE_SEGMENTS,SUBHIT_SHAPE_POLYS};

class EditSplineClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE ) { return new EditSplineMod; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_EDITSPLINE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(EDITSPLINE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFEDIT);}
	void			ResetClassParams(BOOL fileReset);
	};

typedef Tab<Point3> Point3Tab;
typedef Tab<int> SegTab;

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

// General-purpose shape point table -- Maintains point table for each of n polygons
class ShapePointTab {
	public:
		int polys;
		int *pUsed;	// Tells whether polygon is affected
		Point3Tab *ptab;
		IntTab *ktab;
		IntTab *ltab;
		ShapePointTab();
		~ShapePointTab();
		void Empty();
		void Zero();
		void MakeCompatible(BezierShape& shape, BOOL clear=TRUE);
		ShapePointTab& operator=(ShapePointTab& from);
		BOOL IsCompatible(BezierShape &shape);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class ShapeVertexDelta {
	public:
		ShapePointTab dtab;

		void SetSize(BezierShape& shape, BOOL load=TRUE);
		void Empty() { dtab.Empty(); }
		void Zero() { dtab.Zero(); }
		void SetPoint(int poly, int i, const Point3& p) { dtab.pUsed[poly] = 1; dtab.ptab[poly][i] = p; }
		void SetKType(int poly, int i, int k) { dtab.pUsed[poly] = 1; dtab.ktab[poly][i] = k; }
		void SetLType(int poly, int i, int l) { dtab.pUsed[poly] = 1; dtab.ltab[poly][i] = l; }
		void Move(int poly, int i, const Point3& p) { dtab.pUsed[poly] = 1; dtab.ptab[poly][i] += p; }
		void Apply(BezierShape& shape);
		void ClearUsed(int poly) { dtab.pUsed[poly] = 0; }
		void SetUsed(int poly) { dtab.pUsed[poly] = 1; }
		int IsUsed(int poly) { return dtab.pUsed[poly] ? 1 : 0; }
		ShapeVertexDelta& operator=(ShapeVertexDelta& from) { dtab = from.dtab; return *this; }
		void ApplyHandlesAndZero(BezierShape &shape, int handlePoly, int handleVert);
		BOOL IsCompatible(BezierShape &shape) { return dtab.IsCompatible(shape); }
		void RescaleWorldUnits(float f) { dtab.RescaleWorldUnits(f); }
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class AdjEdgeList;
class ESTempData;

/*-------------------------------------------------------------------*/

// Class for recording shape changes -- This is used to reconstruct an object from the original whenever
// the modifier is re-entered or whenever the system needs to reconstruct an object's cache.  This may be
// slow if a lot of changes have been recorded, but it's about the only way to properly reconstruct an object
// because the output of one operation becomes the input of the next.

// These are used as follows:
// When a user makes a modification to an object, a StartChangeGroup call needs to be made to the EditSplineData
// object.  Then a change record needs to be added for each sub-operation that makes up the modification.  These
// records are owned by the EditSplineData object, but they should also be referenced by the undo object for that
// operation.  If an undo is done, ownership of the modification record transfers to the undo/redo object and the
// record is REMOVED (NOT DELETED) from the EditSplineData object.  This keeps the record around for a redo operation
// but removes it from the list of records for the modifier.  If the undo is redone, ownership transfers back to
// the modifier, when it is re-added to the modification record list.

// Note that this class contains load and save methods, necessary because the modifier needs to be able to save
// and load them.  When you subclass off of this, be sure your load and save methods call the base class's first!

class ShapeRestore;

class ModRecord {
	public:
		virtual ~ModRecord();
		virtual BOOL Redo(BezierShape *shape,int reRecord)=0;
		virtual IOResult Load(ILoad *iload)=0;
	};

typedef ModRecord* PModRecord;
typedef Tab<PModRecord> ModRecordTab;

/*-------------------------------------------------------------------*/

// Here are the types of modification records we use!

#define CLEARVERTSELRECORD_CHUNK	0x2000
#define SETVERTSELRECORD_CHUNK		0x2001
#define INVERTVERTSELRECORD_CHUNK	0x2002
#define CLEARSEGSELRECORD_CHUNK		0x2010
#define SETSEGSELRECORD_CHUNK		0x2011
#define INVERTSEGSELRECORD_CHUNK	0x2012
#define CLEARPOLYSELRECORD_CHUNK	0x2020
#define SETPOLYSELRECORD_CHUNK		0x2021
#define INVERTPOLYSELRECORD_CHUNK	0x2022
#define VERTSELRECORD_CHUNK			0x2030
#define SEGSELRECORD_CHUNK			0x2040
#define POLYSELRECORD_CHUNK			0x2050
#define POLYCLOSERECORD_CHUNK		0x2060
#define POLYREVERSERECORD_CHUNK		0x2068
#define OUTLINERECORD_CHUNK			0x2070
#define POLYDETACHRECORD_CHUNK		0x2080
#define POLYDELETERECORD_CHUNK		0x2090
#define VERTMOVERECORD_CHUNK		0x20A0
#define SEGDELETERECORD_CHUNK		0x20B0
#define SEGDETACHRECORD_CHUNK		0x20C0
#define POLYFIRSTRECORD_CHUNK		0x20C8
#define SEGBREAKRECORD_CHUNK		0x20D0
#define SEGREFINERECORD_CHUNK		0x20E0
#define VERTBREAKRECORD_CHUNK		0x20F0
#define VERTCONNECTRECORD_CHUNK		0x2100
#define VERTINSERTRECORD_CHUNK		0x2110
#define VERTWELDRECORD_CHUNK		0x2120
#define BOOLEANRECORD_CHUNK			0x2130
#define ATTACHRECORD_CHUNK			0x2140
#define VERTCHANGERECORD_CHUNK		0x2150
#define SEGCHANGERECORD_CHUNK		0x2160
#define POLYCHANGERECORD_CHUNK		0x2165
#define VERTDELETERECORD_CHUNK		0x2170
#define CREATELINERECORD_CHUNK		0x2180
#define POLYMIRRORRECORD_CHUNK		0x2190
#define POLYENDATTACHRECORD_CHUNK	0x21A0
#define POLYCOPYRECORD_CHUNK		0x21B0
#define SEGCOPYRECORD_CHUNK			0x21C0
										 
class ClearVertSelRecord : public ModRecord {
	public:
		ShapeVSel sel;	// Old state
		ClearVertSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetVertSelRecord : public ModRecord {
	public:
		ShapeVSel sel;	// Old state
		SetVertSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertVertSelRecord : public ModRecord {
	public:
		InvertVertSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearSegSelRecord : public ModRecord {
	public:
		ShapeSSel sel;	// Old state
		ClearSegSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetSegSelRecord : public ModRecord {
	public:
		ShapeSSel sel;	// Old state
		SetSegSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertSegSelRecord : public ModRecord {
	public:
		InvertSegSelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPolySelRecord : public ModRecord {
	public:
		ShapePSel sel;	// Old state
		ClearPolySelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPolySelRecord : public ModRecord {
	public:
		ShapePSel sel;	// Old state
		SetPolySelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPolySelRecord : public ModRecord {
	public:
		InvertPolySelRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class VertSelRecord : public ModRecord {
	public:
		ShapeVSel oldSel;	// Old state
		ShapeVSel newSel;	// New state
		VertSelRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SegSelRecord : public ModRecord {
	public:
		ShapeSSel oldSel;	// Old state
		ShapeSSel newSel;	// New state
		SegSelRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PolySelRecord : public ModRecord {
	public:
		ShapePSel oldSel;	// Old state
		ShapePSel newSel;	// New state
		PolySelRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PolyCloseRecord : public ModRecord {
	public:
		int poly;
		PolyCloseRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PolyReverseRecord : public ModRecord {
	public:
		int poly;
		PolyReverseRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PolyMirrorRecord : public ModRecord {
	public:
		int poly;
		int type;
		BOOL copy;
		PolyMirrorRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PolyEndAttachRecord : public ModRecord {
	public:
		int poly1;				// First poly
		int vert1;				// Vertex on first poly
		int poly2;				// Second poly (may be same as first)
		int vert2;				// Vertex on second poly
		Spline3D oldSpline1;	// How the first spline looked before connect
		Spline3D oldSpline2;	// How the first spline looked before connect
		BitArray oldVSel1;		// Vertex selections before op
		BitArray oldVSel2;		// Vertex selections before op
		BitArray oldSSel1;		// Segment selections before op
		BitArray oldSSel2;		// Segment selections before op
		int selected2;			// TRUE if spline #2 was selected before op
		PolyEndAttachRecord() : ModRecord() { }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline1.Transform(&stm);
			oldSpline2.Transform(&stm);
			}
	};

class OutlineRecord : public ModRecord {
	public:
		BOOL newType;			// Present in MAX 2.0 and up
		int poly;
		int centered;
		float size;
		int oldSplineCount;		// The number of splines present before it was outlined
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		Spline3D oldSpline;		// The original spline
		OutlineRecord() : ModRecord() { newType = TRUE; }
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class PolyDetachRecord : public ModRecord {
	public:
		int poly;
		int copy;
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		Spline3D spline;
		PolyDetachRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			spline.Transform(&stm);
			}
	};

class PolyDeleteRecord : public ModRecord {
	public:
		int poly;
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		Spline3D spline;
		PolyDeleteRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			spline.Transform(&stm);
			}
	};

class VertMoveRecord : public ModRecord {
	public:
		ShapeVertexDelta delta;	// Position changes for each vertex (Wasteful!  Change later?)
		VertMoveRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { delta.RescaleWorldUnits(f); }
	};

class SegDeleteRecord : public ModRecord {
	public:
		int poly;
		Spline3D oldSpline;		// How the spline looked before the delete
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		int deleted;			// TRUE if segment delete results in this polygon being deleted
		int oldSplineCount;		// The number of splines present before it was broken up
		SegDeleteRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class SegDetachRecord : public ModRecord {
	public:
		int poly;
		int copy;				// TRUE if copying segments
		Spline3D oldSpline;		// How the spline looked before the delete
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		int deleted;			// TRUE if segment delete results in this polygon being deleted
		int oldSplineCount;		// The number of splines present before it was broken up
		SegDetachRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class PolyFirstRecord : public ModRecord {
	public:
		int poly;
		int vertex;				// The new first vertex
		Spline3D oldSpline;		// How the spline looked before the new first vertex
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		PolyFirstRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class SegBreakRecord : public ModRecord {
	public:
		int poly;				// The polygon being refined
		int segment;			// The segment being refined
		float param;			// The point on the segment (0-1) where the new point is inserted
		Spline3D oldSpline;		// The spline before we refined it
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		int oldSplineCount;		// The number of splines present before it was broken up
		SegBreakRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class SegRefineRecord : public ModRecord {
	public:
		int poly;				// The polygon being refined
		int segment;			// The segment being refined
		float param;			// The point on the segment (0-1) where the new point is inserted
		Spline3D oldSpline;		// The spline before we refined it
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		SegRefineRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class VertBreakRecord : public ModRecord {
	public:
		int poly;
		Spline3D oldSpline;		// How the spline looked before the break
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		int oldSplineCount;		// The number of splines present before it was broken up
		VertBreakRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class VertConnectRecord : public ModRecord {
	public:
		int poly1;				// First poly
		int vert1;				// Vertex on first poly
		int poly2;				// Second poly (may be same as first)
		int vert2;				// Vertex on second poly
		Spline3D oldSpline1;	// How the first spline looked before connect
		Spline3D oldSpline2;	// How the first spline looked before connect
		BitArray oldVSel1;		// Vertex selections before op
		BitArray oldVSel2;		// Vertex selections before op
		BitArray oldSSel1;		// Segment selections before op
		BitArray oldSSel2;		// Segment selections before op
		int selected2;			// TRUE if spline #2 was selected before op
		VertConnectRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline1.Transform(&stm);
			oldSpline2.Transform(&stm);
			}
	};

class VertInsertRecord : public ModRecord {
	public:
		int poly;				// Poly	being inserted
		Spline3D oldSpline;		// How the spline looked before insert
		Spline3D newSpline;		// How the spline looked after insert
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		BitArray newVSel;		// Vertex selections after op
		BitArray newSSel;		// Segment selections after op
		VertInsertRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			newSpline.Transform(&stm);
			}
	};

class VertWeldRecord : public ModRecord {
	public:
		int poly;
		float thresh;			// Weld threshold
		Spline3D oldSpline;		// How the spline looked before the break
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		BOOL deleted;			// TRUE if spline was deleted as a result of the weld
		VertWeldRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

// The boolean operations
#define BOOL_UNION 0
#define BOOL_SUBTRACTION 1
#define BOOL_INTERSECTION 2

// The mirror operations
#define MIRROR_HORIZONTAL 3
#define MIRROR_VERTICAL 4
#define MIRROR_BOTH 5

// Flags used for boolean polygons
#define POLYBOOL (1 << 0)
#define POLYOUTSIDE (1 << 1)
#define POLYINSIDE (1 << 2)

class BooleanRecord : public ModRecord {
	public:
		int poly1;				// Poly 1 of boolean pair
		int poly2;				// Poly 2 of boolean pair
		int operation;			// Boolean operation (see above)
		int oldSplineCount;		// The number of splines present before it was broken up
		Spline3D oldSpline1;	// How spline 1 looked before boolean
		BitArray oldVSel1;		// Vertex selections before op
		BitArray oldSSel1;		// Segment selections before op
		Spline3D oldSpline2;	// How spline 2 looked before boolean
		BitArray oldVSel2;		// Vertex selections before op
		BitArray oldSSel2;		// Segment selections before op
		BooleanRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline1.Transform(&stm);
			oldSpline2.Transform(&stm);
			}
	};
 
class AttachRecord : public ModRecord {
	public:
		BezierShape attShape;			// The shape we're attaching
		int oldSplineCount;		// The number of splines present before attaching
		AttachRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			attShape.Transform(stm);
			}
	};

class VertChangeRecord : public ModRecord {
	public:
		Spline3D oldSpline;		// How the spline looked before the change
		int poly;
		int vertex;
		int type;
		VertChangeRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class SegChangeRecord : public ModRecord {
	public:
		Spline3D oldSpline;		// How the spline looked before the change
		int poly;
		int segment;
		int type;
		SegChangeRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class PolyChangeRecord : public ModRecord {
	public:
		Spline3D oldSpline;		// How the spline looked before the change
		int poly;
		int type;
		PolyChangeRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class VertDeleteRecord : public ModRecord {
	public:
		int poly;
		Spline3D oldSpline;		// How the spline looked before the delete
		BitArray oldVSel;		// Vertex selections before op
		BitArray oldSSel;		// Segment selections before op
		int selected;			// TRUE if spline was selected before op
		int deleted;			// TRUE if segment delete results in this polygon being deleted
		int oldSplineCount;		// The number of splines present before it was broken up
		VertDeleteRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			oldSpline.Transform(&stm);
			}
	};

class CreateLineRecord : public ModRecord {
	public:
		Spline3D spline;		// The spline we created that will be added
		CreateLineRecord() : ModRecord() {}
		CreateLineRecord(Spline3D *s);
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
		void RescaleWorldUnits(float f) { 
			Matrix3 stm = ScaleMatrix(Point3(f, f, f));
			spline.Transform(&stm);
			}
	};

class PolyCopyRecord : public ModRecord {
	public:
		int poly;
		BOOL selOrig;
		BOOL selCopy;
		PolyCopyRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SegCopyRecord : public ModRecord {
	public:
		int oldSplineCount;
		ShapeSSel oldSSel;	// Old segment selection sets
		BOOL selCopy;
		SegCopyRecord() : ModRecord() {}
		BOOL Redo(BezierShape *shape,int reRecord);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// ESPolyVert Flags:
#define ESP_KNOTTYPE_ALTERED (1<<0)
#define ESP_LINETYPE_ALTERED (1<<1)

// Vertex Mapping class -- Gives mapping from poly/vert in original shape to
// poly/vert in modified shape

class ESPolyVert {
	public:
		int poly;
		int vert;
		Point3 delta;		// The delta we've applied
		DWORD flags;		// See above
		ESPolyVert() { poly = vert = 0; flags = 0; delta = Point3(0,0,0); }
		ESPolyVert(int p, int v, Point3 &d) { poly = p; vert = v; flags = 0; delta = d; }
	};

class ESPolysPV {
	public:
		int verts;
		ESPolyVert *vertMap;
		ESPolysPV() { vertMap = NULL; }
		ESPolysPV(ESPolysPV &from) { vertMap = NULL; *this=from; }
		~ESPolysPV();
		void Build(Spline3D &spline, int poly);
		void Reset();	// Empty all mappings
		ESPolysPV& operator=(ESPolysPV &from);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class ESVertMapper {
	public:
		int polys;
		ESPolysPV *polyMap;
		ESVertMapper() { polys = 0; polyMap = NULL; }
		~ESVertMapper();
		// Set up remap data structures.
		void Build(BezierShape &shape);
		// Apply our stored delta info to the specified shape
		void ApplyDeltas(BezierShape &inShape, BezierShape &outShape);
		// Recompute the deltas we have stored
		// This is done after the modifier's user interaction changes the shape
		void RecomputeDeltas(BOOL checkTopology, BezierShape &shape, BezierShape &oldShape);
		// Record the topology tags in the specified shape
		void RecordTopologyTags(BezierShape &shape);
		ESVertMapper& operator=(ESVertMapper &from);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// EditSplineData flags
#define ESD_BEENDONE			(1<<0)
#define ESD_UPDATING_CACHE		(1<<1)
#define ESD_HASDATA				(1<<2)

// This is the data that each mod app will have.
class EditSplineData : public LocalModData {
	public:
		BOOL handleFlag;
		int handlePoly, handleVert;

		DWORD flags;

		// This records the changes to the incoming object.
		// (OLD SYSTEM, used only when loading old files)
		ModRecordTab changes;

		// Vertex delta object
		ShapeVertexDelta vdelta;

		// RB: Named selection set lists
		NamedVertSelSetList vselSet;
		NamedSegSelSetList sselSet;
		NamedPolySelSetList pselSet;		

		// While an object is being edited, this exists.
		ESTempData *tempData;

		// The knot mapping for the edited shape
		ESVertMapper mapper; 

		// The final edited shape
		BezierShape finalShape;
		
		// "Held" data -- The object we're modifying.  Held for later reference by PreUpdateChanges
		// so that UpdateChanges can detect deltas, not saved
		BezierShape *oldShape;

		EditSplineData();
		EditSplineData(EditSplineData& esc);
		~EditSplineData();
		
		// Applies modifications to a triOb
		void Apply(TimeValue t,SplineShape *splShape,int selLevel,BOOL showVertNumbers,BOOL SVNSelectedOnly);

		// Invalidates any caches affected by the change.
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		// If this is the first edit, then the delta arrays will be allocated
		void BeginEdit(TimeValue t);

		LocalModData *Clone() { return new EditSplineData(*this); }
		
		void SetFlag(DWORD f,BOOL on) 
			{ 
			if ( on ) {
				flags|=f;
			} else {
				flags&=~f; 
				}
			}
		DWORD GetFlag(DWORD f) { return flags&f; }

		ESTempData *TempData(EditSplineMod *mod);

		// Change recording functions:
		void ClearHandleFlag() { handleFlag = FALSE; }
		void SetHandleFlag(int poly, int vert) { handlePoly = poly; handleVert = vert; handleFlag = TRUE; }
		BOOL DoingHandles() { return handleFlag; }
		void ApplyHandlesAndZero(BezierShape &shape) { vdelta.ApplyHandlesAndZero(shape, handlePoly, handleVert); }
		void RescaleWorldUnits(float f);
		
		// MAXr4: New recording system
		void PreUpdateChanges(BezierShape *shape, BOOL checkTopology=TRUE);	// Call before modifying (replaces old RecordTopologyTags call)
		//bindTrackingOptions  0 =  no update
		//bindTrackingOptions  1 =  update based on geometry
		//bindTrackingOptions  2 =  update based on selection
		void UpdateChanges(BezierShape *shape, BOOL checkTopology=TRUE, int bindTrackingOptions = 1);	// Also updates named selection sets

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

// My generic restore class

class ShapeRestore : public RestoreObj {
	public:
		BezierShape oldShape, newShape;
		BOOL gotRedo;
		TimeValue t;
		EditSplineData *esd;
		EditSplineMod *mod;
//2-22-99 watje need to add points for refine connect
		Tab<Point3> undoPointList;
		Tab<Point3> redoPointList;

		ShapeRestore(EditSplineData* md, EditSplineMod* mod, BezierShape *shape);

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {mod->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("Generic shape restore")); }
	};

class MEdge {
	public:
		DWORD f[2];
		DWORD v[2];
	};

class AdjEdgeList {
	public:
		DWORDTab *list;
		Tab<MEdge> edges;
		int nverts;

		AdjEdgeList( BezierShape& ashape );
		~AdjEdgeList() { if (list) delete [] list; }
		AdjEdgeList& operator=(AdjEdgeList& a) {assert(0);return *this;}
		void AddEdge( DWORD fi, DWORD v1, DWORD v2 );
		DWORDTab& operator[](int i) { return list[i]; }
	};

class OutlineTransformer : public Transformer {
 	public:
 		CoreExport Point3 Eval(ViewExp *vpt);
		OutlineTransformer(IObjParam *i) : Transformer(i) {}
 		};	

class OutlineMouseProc : public MouseCallBack {
	private:
		OutlineTransformer outlineTrans;
		EditSplineMod *es;
		IObjParam *ip;
		Point3 p0, p1;
		IPoint2 sp0;
	public:
		OutlineMouseProc(EditSplineMod* mod, IObjParam *i)
			: outlineTrans(i) {es=mod;ip=i;}
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class OutlineSelectionProcessor : public GenModSelectionProcessor {
	protected:
		HCURSOR GetTransformCursor();
		
	public:
		OutlineSelectionProcessor(OutlineMouseProc *mc, Modifier *m, IObjParam *i) 
			: GenModSelectionProcessor(mc,m,i) {}
	};


class OutlineCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		OutlineSelectionProcessor mouseProc;
		OutlineMouseProc eproc;
		EditSplineMod* es;

	public:
		OutlineCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_OUTLINE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class FilletTransformer : public Transformer {
 	public:
 		CoreExport Point3 Eval(ViewExp *vpt);
		FilletTransformer(IObjParam *i) : Transformer(i) {}
 		};	

class FilletMouseProc : public MouseCallBack {
	private:
		FilletTransformer filletTrans;
		EditSplineMod *es;
		IObjParam *ip;
		Point3 p0, p1;
		IPoint2 sp0;

	public:
		FilletMouseProc(EditSplineMod* mod, IObjParam *i)
			: filletTrans(i) {es=mod;ip=i;}
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class FilletSelectionProcessor : public GenModSelectionProcessor {
	protected:
		HCURSOR GetTransformCursor();
		
	public:
		FilletSelectionProcessor(FilletMouseProc *mc, Modifier *m, IObjParam *i) 
			: GenModSelectionProcessor(mc,m,i) {}
	};


class FilletCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		FilletSelectionProcessor mouseProc;
		FilletMouseProc eproc;
		EditSplineMod* es;

	public:
		FilletCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_OUTLINE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class ESChamferTransformer : public Transformer {
 	public:
 		CoreExport Point3 Eval(ViewExp *vpt);
		ESChamferTransformer(IObjParam *i) : Transformer(i) {}
 		};	

class ESChamferMouseProc : public MouseCallBack {
	private:
		ESChamferTransformer chamferTrans;
		EditSplineMod *es;
		IObjParam *ip;
		Point3 p0, p1;
		IPoint2 sp0;

	public:
		ESChamferMouseProc(EditSplineMod* mod, IObjParam *i)
			: chamferTrans(i) {es=mod;ip=i;}
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class ESChamferSelectionProcessor : public GenModSelectionProcessor {
	protected:
		HCURSOR GetTransformCursor();
		
	public:
		ESChamferSelectionProcessor(ESChamferMouseProc *mc, Modifier *m, IObjParam *i) 
			: GenModSelectionProcessor(mc,m,i) {}
	};


class ESChamferCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		ESChamferSelectionProcessor mouseProc;
		ESChamferMouseProc eproc;
		EditSplineMod* es;

	public:
		ESChamferCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), mouseProc(&eproc,mod,i), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_OUTLINE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class SegBreakTransformer : public Transformer {
 	public:
 		CoreExport Point3 Eval(ViewExp *vpt);
		SegBreakTransformer(IObjParam *i) : Transformer(i) {}
 		};	

class SegBreakMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		SegBreakMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
//		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class SegBreakCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		SegBreakMouseProc eproc;
		EditSplineMod* es;

	public:
		SegBreakCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_SEGBREAK; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

#define REFINE_VERT 0
#define REFINE_SEG 1

class SegRefineMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;
		int type; // See above
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		SegRefineMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		void SetType(int type) { this->type = type; }
	};

class SegRefineCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		SegRefineMouseProc eproc;
		EditSplineMod* es;
		int type; // See above

	public:
		SegRefineCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_SEGREFINE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
		void SetType(int type) { this->type = type; eproc.SetType(type); }
	};

/*-------------------------------------------------------------------*/

class CrossInsertMouseProc : public MouseCallBack {
	private:
		EditSplineMod* es;
		IObjParam *ip;
		IPoint2 om;
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		CrossInsertMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class CrossInsertCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		CrossInsertMouseProc eproc;
		EditSplineMod* es;

	public:
		CrossInsertCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es = mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_CROSSINSERT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class VertConnectMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
		BOOL HitAnEndpoint(ViewExp *vpt, IPoint2 *p, BezierShape *shape, int poly, int vert,
			BezierShape **shapeOut, int *polyOut, int *vertOut);
	public:
		VertConnectMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
//		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class VertConnectCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		VertConnectMouseProc eproc;
		EditSplineMod* es;

	public:
		VertConnectCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_VERTCONNECT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class VertInsertMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int hitType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
		BOOL InsertWhere(ViewExp *vpt, IPoint2 *p, BezierShape **shapeOut, int *polyOut,int *segOut, int *vertOut);
	public:
		VertInsertMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class VertInsertCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		VertInsertMouseProc eproc;
		EditSplineMod* es;
		int control;	// ID of the resource button
	public:
		VertInsertCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod; control= -1;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_VERTINSERT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=999999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
		void SetControl(int id) { control = id; }
	};

/*-------------------------------------------------------------------*/

class CreateLineMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;

	protected:
	public:
		CreateLineMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int hitType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
		BOOL InsertWhere(ViewExp *vpt, IPoint2 *p, BezierShape **shapeOut, int *polyOut,int *segOut, int *vertOut);
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class CreateLineCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		CreateLineMouseProc eproc;
		EditSplineMod* es;

	public:
		CreateLineCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_CREATELINE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=999999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/
// CAL-02/26/03: Add Cross Section mode. (FID #827)

class CrossSectionMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;
		
		static bool mCreating;
		static bool mCrossingDrawn;
		static EditSplineData *mShapeData;
		static int mPolys;
		static Tab<int> mSelectedSplines;
		static Matrix3 mObjToWorldTM;
		static IPoint2 mMouse;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		void DrawCrossing(HWND hWnd);

	public:
		CrossSectionMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class CrossSectionCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		CrossSectionMouseProc eproc;
		EditSplineMod* es;

	public:
		CrossSectionCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_CROSSSECTION; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=999999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class BooleanMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
	public:
		BooleanMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class BooleanCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		BooleanMouseProc eproc;
		EditSplineMod* es;

	public:
		BooleanCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_BOOLEAN; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=9999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class TrimMouseProc : public MouseCallBack {
	private:
		EditSplineMod* es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
	public:
		TrimMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class TrimCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		TrimMouseProc eproc;
		EditSplineMod* es;

	public:
		TrimCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_TRIM; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=9999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class ExtendMouseProc : public MouseCallBack {
	private:
		EditSplineMod* es;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
	public:
		ExtendMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		int override(int mode) { return CLICK_DOWN_POINT; }
	};

class ExtendCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		ExtendMouseProc eproc;
		EditSplineMod* es;

	public:
		ExtendCMode(EditSplineMod* mod, IObjParam *i) :
			fgProc(mod), eproc(mod,i) {es=mod;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EXTEND; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=9999; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

//watje
/*-------------------------------------------------------------------*/

class BindMouseProc : public MouseCallBack {
	private:
		EditSplineMod *ss;
		IObjParam *ip;
		IPoint2 om;

	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL HitTestSeg( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		
		BOOL HitAnEndpoint(ViewExp *vpt, IPoint2 *p, BezierShape *shape, int poly, int vert,
			BezierShape **shapeOut, int *polyOut, int *vertOut);
		BOOL HitASegment(ViewExp *vpt, IPoint2 *p, BezierShape *shape, int poly, int vert,
			BezierShape **shapeOut, int *polyOut, int *vertOut);

	public:
		BindMouseProc(EditSplineMod* spl, IObjParam *i) { ss=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class BindCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		BindMouseProc eproc;
		EditSplineMod* ss;

	public:
		BindCMode(EditSplineMod* spl, IObjParam *i) :
			fgProc(spl), eproc(spl,i) {ss=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_VERTCONNECT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};



class RefineConnectMouseProc : public MouseCallBack {
	private:
		EditSplineMod *ss;
		IObjParam *ip;
		IPoint2 om;
		int type; // See above
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		RefineConnectMouseProc(EditSplineMod* spl, IObjParam *i) { ss=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		void SetType(int type) { this->type = type; }
	};

class RefineConnectCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		RefineConnectMouseProc eproc;
		EditSplineMod* ss;
		int type; // See above

	public:
		RefineConnectCMode(EditSplineMod* spl, IObjParam *i) :
			fgProc(spl), eproc(spl,i) {ss=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_REFINECONNECT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
		void SetType(int type) { this->type = type; eproc.SetType(type); }
	};

/*-------------------------------------------------------------------*/
// CAL-03/03/03: copy/paste tangent modes. (FID #827)

class CopyTangentMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		CopyTangentMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class CopyTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		CopyTangentMouseProc eproc;
		EditSplineMod* es;
		IObjParam *ip;

	public:
		CopyTangentCMode(EditSplineMod* mod, IObjParam *i) :
		  fgProc(mod), eproc(mod,i) { es=mod; ip=i; }

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_COPYTANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

class PasteTangentMouseProc : public MouseCallBack {
	private:
		EditSplineMod *es;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		PasteTangentMouseProc(EditSplineMod* mod, IObjParam *i) { es=mod; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class PasteTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		PasteTangentMouseProc eproc;
		EditSplineMod* es;
		IObjParam *ip;

	public:
		PasteTangentCMode(EditSplineMod* mod, IObjParam *i) :
		  fgProc(mod), eproc(mod,i) { es=mod; ip=i; }

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_PASTETANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

/*-------------------------------------------------------------------*/

class ESTempData {
	private:
		BezierShape		*shape;
		Interval 		shapeValid;
		
		EditSplineMod 	*mod;
		EditSplineData 	*shapeData;

	public:		
		
		~ESTempData();
		ESTempData(EditSplineMod *m,EditSplineData *md);
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		BezierShape		*GetShape(TimeValue t);
		
		BOOL ShapeCached(TimeValue t);
		void UpdateCache(SplineShape *splShape);
	};


// Spline hit override functions

extern void SetSplineHitOverride(int value);
extern void ClearSplineHitOverride();

extern BOOL ValidBooleanPolygon(IObjParam *ip, Spline3D *spline);

//az -  042803  MultiMtl sub/mtl name support
INode* GetNode (EditSplineMod *ep);
void GetMtlIDListSpline(Mtl *mtl, NumList& mtlIDList);
void GetESplineMtlIDList(EditSplineMod *es, NumList& mtlIDList);
BOOL SetupMtlSubNameCombo (HWND hWnd, EditSplineMod *es);
void UpdateNameComboSpline (HWND hWnd, ISpinnerControl *spin);
void ValidateUINameCombo (HWND hWnd, EditSplineMod *es);


#define  SRMMS_CLASS_ID 0x681a5018
class SingleRefMakerSplineMMtl: public SingleRefMaker {
public:
	HWND hwnd;
	EditSplineMod *es;
	RefTargetHandle rtarget;
	SingleRefMakerSplineMMtl() {hwnd = NULL; es = NULL; rtarget = NULL; }
	~SingleRefMakerSplineMMtl() { 
		theHold.Suspend();         
		DeleteAllRefsFromMe(); 
		theHold.Resume(); 
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
			switch(message) {
			case REFMSG_CHANGE:
				if(hwnd) ValidateUINameCombo(hwnd, es);
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMMS_CLASS_ID; }
};

#define  SRMNS_CLASS_ID 0x3ddb759a
class SingleRefMakerSplineMNode: public SingleRefMaker {
public:
	HWND hwnd;
	EditSplineMod *es;
	RefTargetHandle rtarget;
	SingleRefMakerSplineMNode() {hwnd = NULL; es = NULL; rtarget = NULL; }
	~SingleRefMakerSplineMNode() { 
		theHold.Suspend();         
		DeleteAllRefsFromMe(); 
		theHold.Resume();
	}
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) { 
		 	INode* singleNode = GetNode(es);
		    switch(message) {
			case REFMSG_NODE_MATERIAL_CHANGED:
				if (singleNode) {
					es->mtlref->SetRef(singleNode->GetMtl());
					if(hwnd) ValidateUINameCombo(hwnd, es);
				}
				break;
			}
			return REF_SUCCEED;			
	}
	SClass_ID  SuperClassID() { return SRMNS_CLASS_ID; }
	
};



#endif // __EDITSPLINE_H__

