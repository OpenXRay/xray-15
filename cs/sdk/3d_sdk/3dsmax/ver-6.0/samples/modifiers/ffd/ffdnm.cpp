/**********************************************************************
 *<
	FILE: ffdnm.cpp

	DESCRIPTION: A NxM FFD Modifier

	CREATED BY: Rolf Berteig, 3.0 additions by Ravi Karra 

	HISTORY: 01/16/97

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/

////////////////////////////////////////////////////////////////////
//
// Free Form Deformation Patent #4,821,214 licensed 
// from Viewpoint DataLabs Int'l, Inc., Orem, UT
// www.viewpoint.com
// 
////////////////////////////////////////////////////////////////////

#include "ffdmod.h"
#include "istdplug.h"
#include "simpmod.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "macrorec.h"
#include "MaxIcon.h"
#include "resource.h"
#include "setkeymode.h"

#define INC_CONSTRAINTS
#include "ffdui.h"
#undef INC_CONSTRAINTS

#define FFD_SELECT_CLASS_ID			Class_ID(0x305f8a34,0x449eec81)
#define GENFFDOBJECT_CLASS_ID		Class_ID(0x763f2e8a,0xaa7b25e3)

static Point3 InterpSpline(float u,Point3 knot[4],float m1, float m2);
static void ComputeTCBMults(float tens, float cont, float &m1, float &m2);
INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

static GenSubObjType SOT_ContPoints(27);
static GenSubObjType SOT_Lattice(28);
static GenSubObjType SOT_Volume(29);

// in ffd.cpp
extern Point3 LatticeSize(Box3 box);
extern void MakeBoxThick(Box3 &box);

template <class T>
class FFDNM : public IFFDMod<T> {
	public:
		MasterPointControl	*masterCont;		// Master track controller
		IParamBlock2		*pblock;
		Control				*tmControl;
		BitArray			sel;
		Tab<Control*>		ptCont;
		Tab<Point3>			pt;
		Box3				lbox;
		int					dim[3];		
		int					selLevel;
		int					loadRefVersion;
		Tab<Point3>			offsets;				// Lattice offsets points
		FFDRightMenu<FFDNM<T> >		*ffdMenu;		// Right-click menu handler		
		FFDActionCB<FFDNM<T> >	*ffdActionCB;	// Actions handler 			
		BOOL beenDeformed;

		FFDNM();
		static FFDNM				*editFFD;
		static IObjParam			*ip;
		static MoveModBoxCMode		*moveMode;
		static RotateModBoxCMode	*rotMode;
		static UScaleModBoxCMode	*uscaleMode;
		static NUScaleModBoxCMode	*nuscaleMode;
		static SquashModBoxCMode	*squashMode;
		static SelectModBoxCMode	*selectMode;
		static BOOL allX, allY, allZ;
		static HWND	hSot;

		// From Animatable
		void	DeleteThis() { delete this; }		
		void	BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void	EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
		BOOL	AssignController(Animatable *control,int subAnim);		
		int		SubNumToRefNum(int subNum);
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


		// From ReferenceTarget/Maker
		int		NumRefs() { return ptCont.Count()+3; }
		RefTargetHandle GetReference(int i);
		void	SetReference(int i, RefTargetHandle rtarg);
		int		NumSubs() { return 3;}
		Animatable* SubAnim(int i) { return GetReference(i); }
		TSTR	SubAnimName(int i);
		BOOL	SelectSubAnim(int subNum);
		int		RemapRefOnLoad(int iref);
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);						
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID, RefMessage message) { return REF_SUCCEED; }

		// From BaseObject/Object
		int		Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		void	GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		int		HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		void	SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void	ClearSelection(int selLevel);
		void	SelectAll(int selLevel);
		void	InvertSelection(int selLevel);
		void	ActivateSubobjSel(int level, XFormModes& modes);
		void	Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void	Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
		void	Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void	GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void	GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void	TransformStart(TimeValue t) {  if (ip) ip->LockAxisTripods(TRUE);  }
		void	TransformFinish(TimeValue t) { if (ip) ip->LockAxisTripods(FALSE); }
		void	TransformCancel(TimeValue t) { if (ip) ip->LockAxisTripods(FALSE); }		
		TCHAR	*GetObjectName();
		DWORD	GetSubselState() { return selLevel; } 
		void	SetSubSelState(DWORD s) { selLevel=(int)s; }

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		int		DrawControlPoints(TimeValue t,ViewExp *vpt,GraphicsWindow *gw,Box3 box,BOOL ht=FALSE,INode *inode=NULL,ModContext *mc=NULL,int flags=0, int *hitCount=NULL);
		void	PlugControllers(TimeValue t, BOOL all=FALSE);
		Matrix3 CompTM(TimeValue t,INode *inode,ModContext *mc);

		// RK: Methods from IFFDMod
		int		NumPts() { return pt.Count(); }
		int		NumPtConts() { return ptCont.Count(); }
		Control* GetPtCont(int i) { return ptCont[i]; }
		void	SetPtCont(int i,Control *c);
		Point3	GetPt(int i);
		void	SetPt(int i, Point3 p) { pt[i] = p; }
		void	SetGridDim(IPoint3 d) { int dm[] = {d.x,d.y,d.z}; SetGridDim(dm); }
		IPoint3	GetGridDim() { return IPoint3(dim[0],dim[1],dim[2]); }
		Point3	GetControlPoint(TimeValue t, int i, int src=FALSE, BOOL initVol=FALSE);
		void	AnimateAll();		
		void	SelectPt(int i, BOOL sel, BOOL clearAll=FALSE);
		void	Conform();
		
		Point3	GetPtOR(int i, int j, int k);
		int		GridDim(int which);
		void	SetGridDim(int dm[3]);		
		int		GridIndex(int i, int j, int k);
		void	ExpandSelection(int ix, BOOL on);
		void	BaseClone(FFDNM<T> *src,RemapDir &remap,BOOL noRef=FALSE);

		virtual Point3 InverseLattice(Point3 p)=0;
		virtual Point3 GetSourcePoint(int i)=0;		
		virtual int GetNameID()=0;
		virtual int GetDlgID()=0;
		virtual	ClassDesc2* GetClassDesc()=0;		
		virtual BOOL WrapX() { return FALSE; }
		virtual int MinXDim() { return 2; }
		virtual int GetLengthNameID() { return IDS_RB_LENGTH; }
		virtual int GetWidthNameID() { return IDS_RB_WIDTH; }
		virtual void UpdateBox(TimeValue t) { }
		virtual int GetSetDimDlgID() { return IDD_FFD_SETDIM; }
	};

template <class T>
class FFDNMDeformer : public Deformer {
	public:		
		Matrix3		tm, itm;
		int			inVol, dim[3];
		float		falloff, m1, m2;
		FFDNM<T>	*ffd;
		Point3		***pts;
		BOOL		wrapX;

		FFDNMDeformer() { ffd=NULL;pts=NULL;dim[0]=dim[1]=dim[2]=0;wrapX=FALSE; }
		FFDNMDeformer(FFDNM<T> *ffd,TimeValue t,Matrix3 *mtm, Box3 &box, BOOL ws=FALSE);		
		Point3	Map(int ii, Point3 p);
		void	FreePointCache() {
			if (pts) { 				
				for (int i=0; i<dim[0]+(wrapX?0:2); i++) {
					for (int j=0; j<dim[1]+2; j++) {
						delete[] pts[i][j];						
						}
					delete[] pts[i];
					}
				delete[] pts;			
				pts = NULL;
				}
			}
	};


template <class T>
class FFDNMSquare : public FFDNM<T> { 	
	public:		
		Point3		InverseLattice(Point3 p) { return p; }
		Point3		GetSourcePoint(int i);
		int			GetNameID() { return IDS_RB_FFDRECT; }
	};

template <class T>
class FFDNMCyl : public FFDNM<T> { 	
	public:
		Point3		InverseLattice(Point3 p);
		Point3		GetSourcePoint(int i);
		int			GetNameID() { return IDS_RB_FFDCYL; }
		int			GetSetDimDlgID() { return IDD_FFD_SETDIMCYL; }
		BOOL		WrapX() { return TRUE; }
		int			MinXDim() { return 6; }
	};

template <class T>
class FFDNMOSMod : public T {
	public:
		FFDNMOSMod();		
		
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE; }
		ChannelMask ChannelsChanged() { return PART_GEOM; }
		Class_ID	InputType() { return defObjectClassID; }

		void		GetClassName(TSTR& s) { s = GetObjectName(); }
		int			GetDlgID() { return IDD_FFDNMOSPARAMS; }		
		ClassDesc2*	GetClassDesc()	{ return NULL; }		
	};

class FFDNMSquareOSMod : public FFDNMOSMod<FFDNMSquare<Modifier> > { 	
	public:		
					FFDNMSquareOSMod();
		Class_ID	ClassID() { return FFDNMOSSQUARE_CLASS_ID; }	
		Interval	LocalValidity(TimeValue t);
		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);		
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		IOResult	Load(ILoad *iload);
		ClassDesc2*	GetClassDesc();
	};

class FFDNMCylOSMod : public FFDNMOSMod<FFDNMCyl<Modifier> >  {
	public:
					FFDNMCylOSMod();
		Class_ID	ClassID() { return FFDNMOSCYL_CLASS_ID; }
		Interval	LocalValidity(TimeValue t);
		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		IOResult	Load(ILoad *iload);
		ClassDesc2*	GetClassDesc();
	};

template <class T>
class FFDNMWSObj : public T {
	public:
		void		InitNodeName(TSTR& s) { s = GetString(IDS_RB_FFDGEN); }
		//void		GetClassName(TSTR& s) { s = GetObjectName(); }
		int			IsRenderable() { return FALSE; }		
	};

class FFDNMSquareWSObj : public FFDNMWSObj<FFDNMSquare<WSMObject> > {
	public:
		Interval	geomValid, selValid;
		BOOL		selLevelValid;

					FFDNMSquareWSObj(BOOL noRef=FALSE);
		Class_ID	ClassID() { return FFDNMWSSQUARE_CLASS_ID; }
		void		GetClassName(TSTR& s) { s = GetString(IDS_RB_FFDRECT_C); }
		Modifier	*CreateWSMMod(INode *node);
		CreateMouseCallBack* GetCreateMouseCallBack();
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		IOResult	Load(ILoad *iload);
		BOOL		IsSubClassOf(Class_ID classID);

		Interval ObjectValidity(TimeValue t);
		void		GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box);
		void		GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box);
		void		GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel);
		int			Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int			HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);

		Interval	ChannelValidity(TimeValue t, int nchan);
		void		SetChannelValidity(int i, Interval v);
		void		InvalidateChannels(ChannelMask channels);
		int			CanConvertToType(Class_ID obtype);
		Object		*ConvertToType(TimeValue t, Class_ID obtype);		
		Object		*MakeShallowCopy(ChannelMask channels);
		void		ShallowCopy(Object* fromOb, ChannelMask channels);		
		void		Deform(Deformer *defProc, int useSel);
		int			IsDeformable() { return 1; }  
		int			NumPoints() { return pt.Count(); }
		Point3		GetPoint(int i) { 
									Point3 s = LatticeSize(lbox);
									Point3 p = pt[i]*s + lbox.Min(); 
									return p; 
									}
		void		SetPoint(int i, const Point3& p) { 
									Point3 s = LatticeSize(lbox);
									pt[i] = (p-lbox.Min())/s;
//									pt[i]=p; 
									}
		ObjectState Eval(TimeValue t);

		void		UpdateBox(TimeValue t);
		int			GetDlgID() { return IDD_FFDNMWSPARAMS; }		
		ClassDesc2*	GetClassDesc();		

	};

class FFDNMCylWSObj : public FFDNMWSObj<FFDNMCyl<WSMObject> > {
	public:		
		Interval	geomValid, selValid;
		BOOL		selLevelValid;

					FFDNMCylWSObj(BOOL noRef=FALSE);
		Class_ID	ClassID() { return FFDNMWSCYL_CLASS_ID; }
		void		GetClassName(TSTR& s) { s = GetString(IDS_RB_FFDCYL_C); }
		Modifier	*CreateWSMMod(INode *node);
		CreateMouseCallBack* GetCreateMouseCallBack();

		Interval	ObjectValidity(TimeValue t);
		void		GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box);
		void		GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box);
		void		GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel);
		int			Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int			HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());		
		IOResult	Load(ILoad *iload);
		BOOL		IsSubClassOf(Class_ID classID);

		Interval	ChannelValidity(TimeValue t, int nchan);
		void		SetChannelValidity(int i, Interval v);
		void		InvalidateChannels(ChannelMask channels);
		int			CanConvertToType(Class_ID obtype);
		Object		*ConvertToType(TimeValue t, Class_ID obtype);		
		Object		*MakeShallowCopy(ChannelMask channels);
		void		ShallowCopy(Object* fromOb, ChannelMask channels);		
		void		Deform(Deformer *defProc, int useSel);
		int			IsDeformable() { return 1; }  
		int			NumPoints() { return pt.Count(); }
		Point3		GetPoint(int i) { 
									Point3 s = LatticeSize(lbox);
									Point3 p = pt[i]*s + lbox.Min(); 
									return p; 
									}
		void		SetPoint(int i, const Point3& p) { 
//									pt[i]=p; 
									Point3 s = LatticeSize(lbox);
									pt[i] = (p-lbox.Min())/s;
									}
		ObjectState Eval(TimeValue t);

		void		UpdateBox(TimeValue t);		
		int			GetDlgID() { return IDD_FFDNMWSCYLPARAMS; }
		ClassDesc2*	GetClassDesc();
		int			GetLengthNameID() { return IDS_RB_HEIGHT; }
		int			GetWidthNameID() { return IDS_RB_RADIUS; }
	};


class FFDNMWSMod : public SimpleWSMMod {
	public:
					FFDNMWSMod();
					FFDNMWSMod(INode *node);

		// From Animatable
		void		GetClassName(TSTR& s) { s= GetString(IDS_RB_FFDNMWSMOD); }
		SClass_ID	SuperClassID() { return WSM_CLASS_ID; }		
		Class_ID	ClassID() { return FFDNMWSSQUARE_MOD_CLASS_ID; }
		void		DeleteThis() { delete this; }		
		RefTargetHandle Clone(RemapDir &remap = NoRemap());

		//RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR		*GetObjectName() { return GetString(IDS_RB_FFDNMWSMOD); }
		
		// From SimpleWSMMod		
		Deformer&	GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
		Interval	GetValidity(TimeValue t);

		RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		ClassDesc2*	GetClassDesc();
	};


//--- Class Descriptor and Class Vars. ------------------------------------------

template <class T> FFDNM<T>             *FFDNM<T>::editFFD = NULL;
template <class T> IObjParam			*FFDNM<T>::ip = NULL;
template <class T> MoveModBoxCMode		*FFDNM<T>::moveMode = NULL;
template <class T> RotateModBoxCMode	*FFDNM<T>::rotMode = NULL;
template <class T> UScaleModBoxCMode	*FFDNM<T>::uscaleMode = NULL;
template <class T> NUScaleModBoxCMode	*FFDNM<T>::nuscaleMode = NULL;
template <class T> SquashModBoxCMode	*FFDNM<T>::squashMode = NULL;
template <class T> SelectModBoxCMode	*FFDNM<T>::selectMode = NULL;
template <class T> BOOL                  FFDNM<T>::allX = 0;
template <class T> BOOL                  FFDNM<T>::allY = 0;
template <class T> BOOL                  FFDNM<T>::allZ = 0;
template <class T> HWND                  FFDNM<T>::hSot = NULL;

template <class T> void resetClassParams()
{
	FFDNM<T>::allX = 0;
	FFDNM<T>::allY = 0;
	FFDNM<T>::allZ = 0;
}

class FFDNMSquareOSModClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FFDNMSquareOSMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDRECT); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return FFDNMOSSQUARE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS); }
	const TCHAR*	InternalName() { return _T("FFDBox"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	int             NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i) { return GetActions(); }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetClassParams<Modifier>(); } // mjm - 2.3.99
	};
static FFDNMSquareOSModClassDesc ffdNMSquareOSDesc;
ClassDesc* GetFFDNMSquareOSDesc() { return &ffdNMSquareOSDesc; }
ClassDesc2*	FFDNMSquareOSMod::GetClassDesc() { return &ffdNMSquareOSDesc; }

class FFDNMSquareWSObjClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FFDNMSquareWSObj; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDRECT_C); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return FFDNMWSSQUARE_CLASS_ID; }
	const TCHAR* 	Category() { return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF); }
	const TCHAR*	InternalName() { return _T("SpaceFFDBox"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetClassParams<WSMObject>(); } // mjm - 2.3.99
	};
static FFDNMSquareWSObjClassDesc ffdNMSquareWSDesc;
ClassDesc* GetFFDNMSquareWSDesc() { return &ffdNMSquareWSDesc; }
ClassDesc2*	FFDNMSquareWSObj::GetClassDesc() { return &ffdNMSquareWSDesc; }

class FFDNMSquareWSModClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new FFDNMWSMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDNMWSMOD); }
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() { return FFDNMWSSQUARE_MOD_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }	
	HINSTANCE		HInstance()	{ return hInstance; }
	};
static FFDNMSquareWSModClassDesc ffdNMSquareWSModDesc;
ClassDesc* GetFFDNMSquareWSModDesc() { return &ffdNMSquareWSModDesc; }
ClassDesc2*	FFDNMWSMod::GetClassDesc() { return &ffdNMSquareWSModDesc; }

class FFDNMCylOSModClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FFDNMCylOSMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDCYL); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return FFDNMOSCYL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS); }
	const TCHAR*	InternalName() { return _T("FFDCyl"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetClassParams<Modifier>(); } // mjm - 2.3.99
	};
static FFDNMCylOSModClassDesc ffdNMCylOSDesc;
ClassDesc* GetFFDNMCylOSDesc() { return &ffdNMCylOSDesc; }
ClassDesc2*	FFDNMCylOSMod::GetClassDesc() { return &ffdNMCylOSDesc; }

class FFDNMCylWSObjClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FFDNMCylWSObj; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDCYL_C); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return FFDNMWSCYL_CLASS_ID; }
	const TCHAR* 	Category() { return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF); }
	const TCHAR*	InternalName() { return _T("SpaceFFDCyl"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetClassParams<WSMObject>(); } // mjm - 2.3.99
	};
static FFDNMCylWSObjClassDesc ffdNMCylWSDesc;
ClassDesc* GetFFDNMCylWSDesc() { return &ffdNMCylWSDesc; }
ClassDesc2*	FFDNMCylWSObj::GetClassDesc() { return &ffdNMCylWSDesc; }


//--- RestoreObjects for undo/redo --------------------------------

#define WM_RESETDIMTEXT	(WM_USER+0x0382)


// A restore object to restore changes to the dimension
template <class T>
class DimRestore : public RestoreObj {
	public:
		int			undoDim[3], redoDim[3];
		BitArray	undoSel, redoSel;
		Tab<Control*> undoCont, redoCont;
		Tab<Point3> undoPt, redoPt, undoOffs, redoOffs;
		FFDNM<T>	*ffd;
		DimRestore(FFDNM<T> *f) {
			ffd = f;
			for (int i=0; i<3; i++) undoDim[i] = ffd->dim[i];
			undoCont = ffd->ptCont;
			undoPt = ffd->pt;
			undoOffs = ffd->offsets;
			undoSel = ffd->sel;
			}
		void Restore(int isUndo) {
			for (int i=0; i<3; i++) redoDim[i] = ffd->dim[i];
			redoCont = ffd->ptCont;
			redoPt = ffd->pt;			
			redoSel = ffd->sel;
			redoOffs = ffd->offsets;
			for (i=0; i<3; i++) ffd->dim[i] = undoDim[i];			
			ffd->ptCont = undoCont;					
			ffd->masterCont->SetNumSubControllers(undoCont.Count());
			ffd->pt = undoPt;
			ffd->offsets = undoOffs;
			ffd->sel = undoSel;
			ffd->NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			ffd->NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE);
			ffd->NotifyDependents(FOREVER,(PartID) ffd,REFMSG_BRANCHED_HISTORY_CHANGED);
			if ((ffd->pblock) && ffd->pblock->GetMap()) PostMessage(ffd->pblock->GetMap()->GetHWnd(),WM_RESETDIMTEXT,0,0);
			}
		void Redo() {
			for (int i=0; i<3; i++) ffd->dim[i] = redoDim[i];
			ffd->ptCont = redoCont;
			ffd->pt = redoPt;
			ffd->offsets = redoOffs;
			ffd->sel = redoSel;			
			ffd->NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			ffd->NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE);
			ffd->NotifyDependents(FOREVER,(PartID) ffd,REFMSG_BRANCHED_HISTORY_CHANGED);
			if ((ffd->pblock) && ffd->pblock->GetMap()) PostMessage(ffd->pblock->GetMap()->GetHWnd(),WM_RESETDIMTEXT,0,0);
			}

	};

// A restore object to save the selection state.
template <class T>
class SelRestore : public RestoreObj {
	public:		
		FFDNM<T>	*mod;
		BitArray	undo,redo;
		SelRestore(FFDNM<T> *m) { mod=m;undo=mod->sel; }
		void Restore(int isUndo) {
			// if we're undoing, save a redo state
			if (isUndo) redo = mod->sel;
			mod->sel = undo;
			mod->NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		void Redo() {
			mod->sel = redo;
			mod->NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
	};

// A restore object to save the position of control points.
template <class T>
class MoveRestore : public RestoreObj {
	public:		
		FFDNM<T> *mod;
		Tab<Point3> undo, redo, undoOffs, redoOffs;
		MoveRestore(FFDNM<T> *m) {
			mod = m;
			undo.SetCount(mod->NumPts());
			undoOffs.SetCount(mod->NumPts());
			for (int i=0; i<mod->NumPts(); i++) {
				undo[i] = mod->GetPt(i);
				undoOffs[i] = mod->offsets[i];
				}
			}
		void Restore(int isUndo) {
			// if we're undoing, save a redo state
			if (isUndo) {
				redo.SetCount(mod->NumPts());
				redoOffs.SetCount(mod->NumPts());
				for (int i=0; i<mod->NumPts(); i++) {
					redo[i] = mod->GetPt(i);
					redoOffs[i] = mod->offsets[i];
					}
				}
			for (int i=0; i<mod->NumPts(); i++) {
				mod->pt[i] = undo[i];
				mod->offsets[i] = undoOffs[i];
				}
			mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
			}
		void Redo() {
			for (int i=0; i<mod->NumPts(); i++) {
				mod->pt[i] = redo[i];
				mod->offsets[i] = redoOffs[i];
				}
			mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
			}
		void EndHold() {
			mod->ClearAFlag(A_HELD);
			}
	};


//--- Parameter map/block descriptors -------------------------------

enum { ffd_params };
enum { ffd_disp_lattice, ffd_disp_source, ffd_deform, ffd_falloff, 
		ffd_tens, ffd_cont, ffd_length, ffd_width, ffd_height, 
		ffd_inpoints, ffd_outpoints, ffd_offset};


static ParamBlockDesc2 ffdos_box_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdNMSquareOSDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_FFDNMOSPARAMS, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	ffd_disp_lattice, 	_T("dispLattice"),		TYPE_BOOL, 		0,				IDS_RK_SHOWLATTICE,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWLATTICE, 
		end, 
	ffd_disp_source, 	_T("dispSource"),		TYPE_BOOL, 		0,				IDS_RK_SHOWSOURCE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWSOURCE, 
		end, 
	ffd_deform, 		_T("deformType"),		TYPE_INT, 		0,				0,
		p_default, 		0, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 	2,		IDC_FFD_INVOLUME, IDC_FFD_DEFORMALL,  
		end, 
	ffd_falloff, 		_T("falloff"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_FALLOFF, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_FALLOFF,IDC_FFD_FALLOFFSPIN, SPIN_AUTOSCALE, 
		end, 
	ffd_tens, 			_T("tension"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_TENSION, 
		p_default, 		25.0f, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_TENS,	IDC_FFD_TENSSPIN, 0.1f, 
		end, 
	ffd_cont, 			_T("continuity"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_CONT, 
		p_default, 		25.0, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_CONT,	IDC_FFD_CONTSPIN, 0.1f, 
		end, 
	ffd_inpoints, 		_T("inPoints"),			TYPE_BOOL, 		0,				IDS_RK_INPOINTS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_INPOINTS, 
		end,
	ffd_outpoints, 		_T("outPoints"),		TYPE_BOOL, 		0,				IDS_RK_OUTPOINTS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_OUTPOINTS, 
		end,		
	ffd_offset, 		_T("offset"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_OFFSET, 
		p_default, 		0.05f, 
		p_range, 		-0.2f, 0.2f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_OFFSET,	IDC_FFD_OFFSETSPIN, 0.01f, 
		end,
	end
	);

static ParamBlockDesc2 ffdos_cyl_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdNMCylOSDesc, P_AUTO_CONSTRUCT + P_USE_PARAMS, PBLOCK_REF,
	// use params from existing descriptor
  	&ffdos_box_param_blk
	);
 
static ParamBlockDesc2 ffdws_box_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdNMSquareWSDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_FFDNMWSPARAMS, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	ffd_disp_lattice, 	_T("dispLattice"),		TYPE_BOOL, 		0,				IDS_RK_SHOWLATTICE,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWLATTICE, 
		end, 
	ffd_disp_source, 	_T("dispSource"),		TYPE_BOOL, 		0,				IDS_RK_SHOWSOURCE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWSOURCE, 
		end, 
	ffd_deform, 		_T("deformType"),		TYPE_INT, 		0,				0,
		p_default, 		0, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 	2,		IDC_FFD_INVOLUME,IDC_FFD_DEFORMALL,  
		end, 
	ffd_falloff, 		_T("falloff"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_FALLOFF, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_FALLOFF,IDC_FFD_FALLOFFSPIN, SPIN_AUTOSCALE, 
		end, 
	ffd_tens, 			_T("tension"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_TENSION, 
		p_default, 		25.0f, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_TENS,	IDC_FFD_TENSSPIN, 0.1f, 
		end, 
	ffd_cont, 			_T("continuity"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_CONT, 
		p_default, 		25.0, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_CONT,	IDC_FFD_CONTSPIN, 0.1f, 
		end, 
	ffd_length, 		_T("length"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_LENGTH, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_LENGTH,IDC_FFD_LENGTHSPIN, SPIN_AUTOSCALE, 
		end,	
	ffd_width, 			_T("width"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_WIDTH, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_WIDTH,IDC_FFD_WIDTHSPIN, SPIN_AUTOSCALE, 
		end,			
	ffd_height, 		_T("height"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_HEIGHT, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_HEIGHT, IDC_FFD_HEIGHTSPIN, SPIN_AUTOSCALE, 
		end,
	end
	);

static ParamBlockDesc2 ffdws_cyl_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdNMCylWSDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_FFDNMWSCYLPARAMS, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	ffd_disp_lattice, 	_T("dispLattice"),		TYPE_BOOL, 		0,				IDS_RK_SHOWLATTICE,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWLATTICE, 
		end, 
	ffd_disp_source, 	_T("dispSource"),		TYPE_BOOL, 		0,				IDS_RK_SHOWSOURCE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FFD_SHOWSOURCE, 
		end, 
	ffd_deform, 		_T("deformType"),		TYPE_INT, 		0,				0,
		p_default, 		0, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 	2,		IDC_FFD_INVOLUME,IDC_FFD_DEFORMALL,  
		end, 
	ffd_falloff, 		_T("falloff"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_FALLOFF, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_FALLOFF,IDC_FFD_FALLOFFSPIN, SPIN_AUTOSCALE, 
		end, 
	ffd_tens, 			_T("tension"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_TENSION, 
		p_default, 		25.0f, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_TENS,	IDC_FFD_TENSSPIN, 0.1f, 
		end, 
	ffd_cont, 			_T("continuity"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RK_CONT, 
		p_default, 		25.0, 
		p_range, 		0.0f, 50.0f, 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_FLOAT, IDC_FFD_CONT,	IDC_FFD_CONTSPIN, 0.1f, 
		end, 
	//radius
	ffd_length, 		_T("radius"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RB_RADIUS, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_LENGTH,IDC_FFD_LENGTHSPIN, SPIN_AUTOSCALE, 
		end,	
	// height
	ffd_width, 			_T("height"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RB_HEIGHT, 
		p_default, 		0.0, 
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER,			EDITTYPE_UNIVERSE, IDC_FFD_WIDTH,IDC_FFD_WIDTHSPIN, SPIN_AUTOSCALE, 
		end,			
	end
	);

//
//
// Parameters

#define PARAMDESC_LENGH_MOD		6
#define PARAMDESC_LENGH_OBJ		9
#define PARAMDESC_LENGH_OBJCYL	8

// The parameter block. Note that none of these parameters are animatable.
static ParamBlockDescID descVerModV0[] = {
	{ TYPE_INT,   NULL, FALSE, ffd_disp_lattice },
	{ TYPE_INT,   NULL, FALSE, ffd_disp_source },
	{ TYPE_INT,   NULL, FALSE, ffd_deform },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_falloff }};

static ParamBlockDescID descVerModV1[] = {
	{ TYPE_INT,   NULL, FALSE, ffd_disp_lattice },
	{ TYPE_INT,   NULL, FALSE, ffd_disp_source },
	{ TYPE_INT,   NULL, FALSE, ffd_deform },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_falloff },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_tens },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_cont }};
#define PBLOCK_LENGTH_MOD	6

static ParamBlockDescID descVerObjV0[] = {
	{ TYPE_INT,   NULL, FALSE, ffd_disp_lattice },
	{ TYPE_INT,   NULL, FALSE, ffd_disp_source },
	{ TYPE_INT,   NULL, FALSE, ffd_deform },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_falloff },	
	{ TYPE_FLOAT, NULL, TRUE,  ffd_length },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_width },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_height }};

static ParamBlockDescID descVerObjV1[] = {
	{ TYPE_INT,   NULL, FALSE, ffd_disp_lattice },
	{ TYPE_INT,   NULL, FALSE, ffd_disp_source },
	{ TYPE_INT,   NULL, FALSE, ffd_deform },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_falloff },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_tens },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_cont },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_length },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_width },
	{ TYPE_FLOAT, NULL, TRUE,  ffd_height }};


// Arrays of old versions
static ParamVersionDesc versionsMod[] = {
	ParamVersionDesc(descVerModV0,4,0),
	ParamVersionDesc(descVerModV1,6,1),
	};

static ParamVersionDesc versionsObjSquare[] = {
	ParamVersionDesc(descVerObjV0,7,0),
	ParamVersionDesc(descVerObjV1,9,1)
	};

static ParamVersionDesc versionsObjCyl[] = {
	ParamVersionDesc(descVerObjV0,6,0),
	ParamVersionDesc(descVerObjV1,8,1)
	};

#define NUM_OLDVERSIONS	2



//--- FFDDlgProc ---------------------------------------------
//
// A dialog proc for the about box
//


// NOTE:
// The following statement (which appears in the about box of this
// modifier) ABSOLUTELY MUST APPEAR in any modifier based on this
// code or that uses the FFD technology.
//
////////////////////////////////////////////////////////////////////
//
// Free Form Deformation Patent #4,821,214 licensed 
// from Viewpoint DataLabs Int'l, Inc., Orem, UT
// www.viewpoint.com
// 
////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK AboutDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			CenterWindow(hWnd, GetParent(hWnd));
			break;
		case WM_CLOSE:
			EndDialog(hWnd,1);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hWnd,1);
					break;
				}
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

static INT_PTR CALLBACK SetDimDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int *dim;

	switch (msg) {
		case WM_INITDIALOG: {
			dim = (int*)lParam;
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM0SPIN));
			spin->SetLimits(dim[3],1000, FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_FFD_DIM0), EDITTYPE_INT);
			spin->SetValue(dim[0],FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM1SPIN));
			spin->SetLimits(2,1000, FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_FFD_DIM1), EDITTYPE_INT);
			spin->SetValue(dim[1],FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM2SPIN));
			spin->SetLimits(2,1000, FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_FFD_DIM2), EDITTYPE_INT);
			spin->SetValue(dim[2],FALSE);
			ReleaseISpinner(spin);
			
			CenterWindow(hWnd, GetParent(hWnd));
			break;
			}		

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ISpinnerControl *spin;
					spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM0SPIN));
					dim[0] = spin->GetIVal();
					ReleaseISpinner(spin);

					spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM1SPIN));
					dim[1] = spin->GetIVal();
					ReleaseISpinner(spin);

					spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_DIM2SPIN));
					dim[2] = spin->GetIVal();
					ReleaseISpinner(spin);

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

template <class T>
class FFDDlgProc : public ParamMap2UserDlgProc {
	public:		
		FFDNM<T>	*ffd;
		HWND		hWnd;
		FFDDlgProc(FFDNM<T> *f) { ffd=f; }
		BOOL		DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void		DeleteThis() { delete this; }		
		void		InitDlg(HWND hWnd);
		void		SetDimText();
		void		SetFalloffState();
		void		SetButtonStates();
		void		Update(TimeValue t) { SetFalloffState(); }
	};

template <class T>
void FFDDlgProc<T>::InitDlg(HWND hWnd)
	{
	this->hWnd = hWnd;
	SetDimText();
	SetFalloffState();
	SetButtonStates();	
#if defined( DESIGN_VER )
	ShowWindow(GetDlgItem(hWnd, IDC_FFD_ANIMATEALL), SW_HIDE);
#endif // DESIGN_VER
	}

template <class T>
void FFDDlgProc<T>::SetDimText()
	{
	TSTR buf;
	buf.printf(_T("%dx%dx%d"), ffd->dim[1], ffd->dim[0], ffd->dim[2]);
	SetDlgItemText(hWnd,IDC_FFD_DIMTEXT,buf);
	}

template <class T>
void FFDDlgProc<T>::SetFalloffState()
	{
	int def;
	ffd->pblock->GetValue(ffd_deform,0,def,FOREVER);
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FFD_FALLOFFSPIN));
	if (!def) {
		spin->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_FFD_FALLOFFLABEL),FALSE);
	} else {
		spin->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_FFD_FALLOFFLABEL),TRUE);
		}
	ReleaseISpinner(spin);
	}

template <class T>
void FFDDlgProc<T>::SetButtonStates()
	{
	ICustButton *but;
	but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLX));
	but->SetType(CBT_CHECK);
	but->SetCheck(ffd->allX);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLY));
	but->SetType(CBT_CHECK);
	but->SetCheck(ffd->allY);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLZ));
	but->SetType(CBT_CHECK);
	but->SetCheck(ffd->allZ);
	ReleaseICustButton(but);
	}

template <class T>
BOOL FFDDlgProc<T>::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			InitDlg(hWnd);
			break;

		case WM_RESETDIMTEXT:
			SetDimText();
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_FFD_ALLX: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLX));
					ffd->allX = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}
				case IDC_FFD_ALLY: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLY));
					ffd->allY = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}
				case IDC_FFD_ALLZ: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLZ));
					ffd->allZ = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}

				case IDC_FFD_SETDIM: {
					int dim[4] = { ffd->dim[0],ffd->dim[1],ffd->dim[2],ffd->MinXDim()};
					if (DialogBoxParam(
						hInstance,
						MAKEINTRESOURCE(ffd->GetSetDimDlgID()),
						hWnd,
						SetDimDlgProc,
						(LPARAM)dim)) {
						Point3 d = Point3(dim[0],dim[1],dim[2]);
						macroRecorder->FunctionCall(_T("setDimensions"), 2, 0, mr_reftarg, ffd, mr_point3, &d);
						theHold.Begin();						
						ffd->SetGridDim(dim);
						theHold.Accept(GetString(IDS_RB_SETFFDDIM));
						SetDimText();
						}
					break;
					}

				case IDC_FFD_DEFORMALL:
				case IDC_FFD_INVOLUME:
					SetFalloffState();
					break;

				case IDC_FFD_RESET:
					macroRecorder->FunctionCall(_T("resetLattice"), 1, 0, mr_reftarg, ffd, mr_funcall);
					theHold.Begin();
					ffd->SetGridDim(ffd->dim);
					//ffd->sel=0;
					theHold.Accept(GetString(IDS_RK_RESETLATTICE));						
					break;

				case IDC_FFD_ANIMATEALL:
					macroRecorder->FunctionCall(_T("animateAll"), 1, 0, mr_reftarg, ffd, mr_funcall);
					theHold.Begin();
					ffd->AnimateAll();
					theHold.Accept(GetString(IDS_RK_ANIMATEALL));					
					break;
				
				case IDC_FFD_CONFORM: {
					HCURSOR hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
					macroRecorder->FunctionCall(_T("conformToShape"), 1, 0, mr_reftarg, ffd, mr_funcall);
					theHold.Begin();
					ffd->Conform();
					theHold.Accept(GetString(IDS_RK_CONFORM));
					SetCursor(hCur);
					}
					break;

				case IDC_FFD_ABOUT:
					// Put up the about box
					DialogBox(
						hInstance,
						MAKEINTRESOURCE(IDD_FFD_ABOUTNM),
						hWnd,
						AboutDlgProc);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}


//--- FFDNM  Methods ----------------------------------------------


template <class T>
FFDNM<T>::FFDNM()
	{
	selLevel = 0;
	dim[0] = dim[1] = dim[2] = 0;
	tmControl = NULL;
	pblock = NULL;
	masterCont = NULL;		
	beenDeformed = FALSE;
	}

template <class T>
void FFDNM<T>::BaseClone(FFDNM<T> *src,RemapDir &remap,BOOL noRef)
	{
	if (noRef) {
		src->tmControl = tmControl;
		src->pblock    = pblock;
		src->masterCont = masterCont;
	} else {
		if (tmControl) src->ReplaceReference(0,remap.CloneRef(tmControl));
		src->ReplaceReference(1,remap.CloneRef(pblock));
		src->ReplaceReference(2,remap.CloneRef(masterCont));
		}
	src->ptCont.SetCount(NumPts());	
	src->masterCont->SetNumSubControllers(NumPts());
	src->pt       = pt;
	src->offsets  = offsets;
	src->sel      = sel;
	src->selLevel = selLevel;
	src->dim[0]   = dim[0];
	src->dim[1]   = dim[1];
	src->dim[2]   = dim[2];
	src->lbox     = lbox;	
	src->beenDeformed = beenDeformed;
	for (int i=0; i<NumPts(); i++) {
		src->ptCont[i] = NULL;
		if (ptCont[i] && !noRef) {
			src->ReplaceReference(i+3,remap.CloneRef(ptCont[i]));
			}
		}
	}

template <class T>
void FFDNM<T>::PlugControllers(TimeValue t, BOOL all)
	{
	BOOL notify=FALSE;
	// Plug-in controllers for selected points without controllers
	// if we're animating

	BOOL Key = FALSE;
	if (all || AreWeKeying(t)) Key=TRUE;
	else {
		SetKeyModeInterface *ski = GetSetKeyModeInterface(GetCOREInterface());
		if( ski && ski->TestSKFlag(SETKEY_SETTING_KEYS) ) {
			Key=TRUE; all=FALSE;
		}
	}

	if (Key) { 	// RK: Adding option to animate all
		SuspendSetKeyMode();
		SuspendAnimate();
		AnimateOff(); 

		for (int i=0; i<NumPts(); i++) { 		
			if ((all || sel[i]) && !GetPtCont(i)) {
				ReplaceReference(3+i,NewDefaultPoint3Controller());				
				theHold.Suspend();				
				GetPtCont(i)->SetValue(0,&pt[i],TRUE,CTRL_ABSOLUTE);								
				theHold.Resume();
				masterCont->SetSubController(i,GetPtCont(i));
				notify = TRUE;
				}			
			}
		ResumeAnimate(); 
		ResumeSetKeyMode();
	}
	if (notify) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	}

static Point3 Reflect(Point3 axis, Point3 vect)
	{
   	axis = Normalize(axis);
	Point3 perp = Normalize(axis^vect) ^ axis;
	return (DotProd(vect,axis)*axis) - (DotProd(vect,perp)*perp);
	}
 
template <class T>
Point3 FFDNM<T>::GetPtOR(int i, int j, int k)
	{
	int ii=i, jj=j, kk=k;
	if (i<0) i = 0; if (i>dim[0]-1) i = dim[0]-1;
	if (j<0) j = 0; if (j>dim[1]-1) j = dim[1]-1;
	if (k<0) k = 0; if (k>dim[2]-1) k = dim[2]-1;
	int gi = GridIndex(i,j,k);
	Point3 p = GetPt(gi)-offsets[gi];
	if (ii!=i || jj!=j || kk!=k) {
		if (TRUE) {
			Point3 pp=p, pp1, pp2;
			if (ii<0) {
				if (WrapX()) {
					p = GetPtOR(dim[0]+ii,jj,kk);
				} else {
					pp  = GetPtOR(i  ,jj,kk);
					pp1 = GetPtOR(i+1,jj,kk);
					if (dim[0]>2) {
						pp2 = GetPtOR(i+2,jj,kk);
						p += Reflect(pp-pp1,pp1-pp2);
					} else {
						p += pp-pp1;
						}
					}
				}
			if (ii>dim[0]-1) {
				if (WrapX()) {
					p = GetPtOR(ii-dim[0],jj,kk);
				} else {
					pp  = GetPtOR(i  ,jj,kk);
					pp1 = GetPtOR(i-1,jj,kk);
					if (dim[0]>2) {
						pp2 = GetPtOR(i-2,jj,kk);
						p += Reflect(pp-pp1,pp1-pp2);
					} else {
						p += pp-pp1;
						}
					}
				}

			if (jj<0) {
				pp  = GetPtOR(ii,j  ,kk);
				pp1 = GetPtOR(ii,j+1,kk);
				if (dim[1]>2) {
					pp2 = GetPtOR(ii,j+2,kk);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
			if (jj>dim[1]-1) {
				pp  = GetPtOR(ii,j  ,kk);
				pp1 = GetPtOR(ii,j-1,kk);
				if (dim[1]>2) {
					pp2 = GetPtOR(ii,j-2,kk);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}

			if (kk<0) {
				pp  = GetPtOR(ii,jj,k  );
				pp1 = GetPtOR(ii,jj,k+1);
				if (dim[2]>2) {
					pp2 = GetPtOR(ii,jj,k+2);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
			if (kk>dim[2]-1) {
				pp  = GetPtOR(ii,jj,k  );
				pp1 = GetPtOR(ii,jj,k-1);
				if (dim[2]>2) {
					pp2 = GetPtOR(ii,jj,k-2);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
		} else { 			
			float x = 1.0f/float(dim[0]-1);
			float y = 1.0f/float(dim[1]-1);
			float z = 1.0f/float(dim[2]-1);
			if (ii<0) {
				if (WrapX()) p = GetPtOR(dim[0]+ii,jj,kk);
				else p.x -= x; 
				}
			if (ii>dim[0]-1) {
				if (WrapX()) p = GetPtOR(ii-dim[0],jj,kk);
				else p.x += x;
				}
			if (jj<0) p.y -= y; if (jj>dim[1]-1) p.y += y;
			if (kk<0) p.z -= z; if (kk>dim[2]-1) p.z += z;
			}
		}
	return p;
	}

template <class T>
void FFDNM<T>::SetPtCont(int i,Control *c)
	{
	ptCont[i]=c;
	if (masterCont) 
		masterCont->SetSubController(i,c);
	}

template <class T>
Point3 FFDNM<T>::GetPt(int i)
	{
	return pt[i];
	}

template <class T>
Point3 FFDNM<T>::GetControlPoint(TimeValue t, int i, int src, BOOL initVol)
	{	
	if (initVol) return pt[i];
	if (src) {
		return GetSourcePoint(i);
	} else {
		if (ptCont[i]) {
			// The point is animated, get it from the controller
			Point3 p;
			ptCont[i]->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
			return p;
		} else {
			// the point is not animated.
			return pt[i];
			}
		}
	}

template <class T>
void FFDNM<T>::AnimateAll()
{
	PlugControllers(0, TRUE);	
}

template <class T>
void FFDNM<T>::Conform()
{
	int inPts, outPts, out=FALSE;
	Ray ry;
	Point3 norm, ipt, center, s;
	float at, off;	
	sMyEnumProc dep;              
	EnumDependents(&dep);
	TimeValue t = GetCOREInterface()->GetTime();
	
	if (SuperClassID()==WSM_OBJECT_CLASS_ID || (dep.Nodes.Count() != 1) ||
			!dep.Nodes[0]->EvalWorldState(t).obj->IsSubClassOf(triObjectClassID))
		return;
	
	inPts = pblock->GetInt(ffd_inpoints);
	outPts = pblock->GetInt(ffd_outpoints);
	off = pblock->GetFloat(ffd_offset);

	if (theHold.Holding()) theHold.Put(new MoveRestore<T>(this));	
	if (!inPts && !outPts) return;

	Box3 box;		
	ObjectState os = dep.Nodes[0]->EvalWorldState(t);
	os.obj->GetDeformBBox(t, box,NULL);
	Mesh *ffdmesh = &((TriObject*)os.obj)->GetMesh();
	if (!ffdmesh) return;		
	
	// Calculate the center of the mesh
	Point3 sum = Point3(0,0,0);	
	for (int v=0; v < ffdmesh->numVerts; v++)
		sum += ffdmesh->getVert(v);
	center = sum/(float)ffdmesh->numVerts;		
	s = LatticeSize(box);
	
	// Shrink wrap the lattice to fit the shape of the object
	for (int i=0; i<NumPts(); i++) {				
		
		BOOL hit = FALSE; 	
		Point3 lp = GetPt(i);				
		
		// Try three times by varying ry.p, to avoid de-generated faces
		// LAM - 3/13/03 - changing ry.p.y in the increment, and then calculating new value for ry.p in loop - bogus.
		// fixed and made scale independent by tweaking lp.y instead
		for (int j=0; j < 3 && !hit; j++, lp.y += 0.0001f) {
			ry.p = box.Min() + lp*s;		
			// Shoot ray from lattice point to center
			ry.dir =  center - ry.p;			
			if (ffdmesh->IntersectRay(ry, at, norm))
				out = hit = TRUE;
			}
		
		if (!hit || at>1.0f) {			
			// Shoot ray from center to lattice point
			lp = GetPt(i);
			ry.p = center;
			ry.dir =  box.Min() + lp*s - ry.p;
			if (ffdmesh->IntersectRay(ry, at, norm))
				out = !(hit = TRUE);
			}
		if (!hit || (out ? !outPts : !inPts))
			continue;
		
		// Compute the point of intersection 
		ipt = (ry.p + ry.dir * (at-off) - box.Min())/s; 		
		
		// Move the lattice point to the point of intersection
		pt[i] = ipt;
		offsets[i] = ipt-GetSourcePoint(i);
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
}

template <class T>
int FFDNM<T>::GridDim(int which)
	{
	return dim[which];
	}

template <class T>
int FFDNM<T>::GridIndex(int i, int j, int k)
	{
	int ix = k*dim[0]*dim[1] + j*dim[0] + i;
	assert(ix>=0 && ix<NumPts());
	return ix;
	}

template <class T>
void FFDNM<T>::SetGridDim(int dm[3])
	{
	if (dm[0]<MinXDim()) dm[0] = MinXDim();
	DimRestore<T> *rest = NULL;
	if (theHold.Holding()) rest = new DimRestore<T>(this);
	for (int i=0; i<3; i++) dim[i] = dm[i];
//	for (i=0; i<ptCont.Count(); i++) DeleteReference(i+3);	

// solved a reference delete problem
	for (i=0; i<ptCont.Count(); i++) 
	{

		ReferenceTarget* rtarg = GetReference(i+3);
		if (rtarg) {
			rtarg->SetAFlag(A_LOCK_TARGET);
			}
		DeleteReference(i+3);
		if (rtarg) {
			rtarg->ClearAFlag(A_LOCK_TARGET);
			rtarg->MaybeAutoDelete();
			}

	}


	int total = dim[0]*dim[1]*dim[2];
	ptCont.SetCount(total);
	masterCont->SetNumSubControllers(total);
	pt.SetCount(total);	
	offsets.SetCount(total);
	sel.SetSize(total);
	for (i=0; i<total; i++) {
		ptCont[i] = NULL;
		pt[i] = GetSourcePoint(i);
		offsets[i] = Point3(0,0,0);
		}	
	if (rest) theHold.Put(rest);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE);
	NotifyDependents(FOREVER,(PartID) this,REFMSG_BRANCHED_HISTORY_CHANGED);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	}

template <class T>
int FFDNM<T>::DrawControlPoints(
		TimeValue t,ViewExp *vpt,GraphicsWindow *gw,
		Box3 box,BOOL ht,INode *inode,
		ModContext *mc,int flags, int *hitCount)
	{
	MakeBoxThick(box);
	Point3 s = LatticeSize(box);
	Point3 p, pp[3];
	int res=0, dispLat, dispSrc;
	int level = ip ?  ip->GetSubObjectLevel() : SEL_LATTICE;		
	if (!ht) level = selLevel;
	// RK: 04/23/99 commented as it is showing sel vertices in green when sub-object 
	//     level is set to 0 through MaxScript 
	//if (!ht && !(flags&DISP_SHOWSUBOBJECT)) level = 0;

	pblock->GetValue(ffd_disp_lattice,0,dispLat,FOREVER);
	pblock->GetValue(ffd_disp_source,0,dispSrc,FOREVER);

	// Draw the control points
	for (int i=0; i<NumPts(); i++) {
		// Maybe skip sel or unsel points
		if (ht && flags&HIT_SELONLY   && !sel[i]) continue;
		if (ht && flags&HIT_UNSELONLY &&  sel[i]) continue;

		// Set the line color based on selection
		if (!ht)
		{
			if (level==SEL_POINTS) {
				if ( sel[i]) //gw->setColor(LINE_COLOR, (float)1, (float)1, (float)0.0);
					gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
				if (!sel[i]) //gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
					gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
				}			
			else if (level==SEL_SETVOLUME) {
				if ( sel[i]) gw->setColor(LINE_COLOR, (float)1, (float)0, (float)0.0);				
				if (!sel[i]) gw->setColor(LINE_COLOR, (float)0, (float)1, (float)0.0);				
				}
		}
		// Draw the point
		p = GetControlPoint(t,i,dispSrc,level==SEL_SETVOLUME)*s + box.Min();
		gw->marker(&p,HOLLOW_BOX_MRKR);		
		
		// If we're hit testing then check for a hit and log it
		if (ht && gw->checkHitCode()) {
			if (hitCount) *hitCount+=1;
			gw->clearHitCode();
			if (ip && level) vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL);
			res = 1;
			if (flags&HIT_ABORTONHIT) return res;
//			if (level==SEL_LATTICE) return 1;
			}
		}

	if (0 && !ht) {
		gw->setColor(LINE_COLOR, (float)0, (float)0, (float)1);
		for (int i=-1; i<dim[0]+1; i++) {
			for (int j=-1; j<dim[1]+1; j++) {
				for (int k=-1; k<dim[2]+1; k++) {
					if (i<0||i>=dim[0]||j<0||j>=dim[1]||k<0||k>=dim[2]) {
						p = GetPtOR(i,j,k)*s + box.Min();
						gw->marker(&p,HOLLOW_BOX_MRKR);		
						}
					}
				}
			}
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}		

	// Don't hit test the lattice
	if (ht && level!=SEL_LATTICE && level) return res;
	if (!dispLat) return res;

	// Set the GW line color
	if (level!=SEL_LATTICE ) {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}

	// Draw the lattice
	if (inode->IsFrozen()) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_FREEZE));
		}
	for (i=0; i<GridDim(0); i++) {
		for (int j=0; j<GridDim(1); j++) {
			for (int k=0; k<GridDim(2); k++) {
				pp[0] = GetControlPoint(t,GridIndex(i,j,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
				if (i<GridDim(0)-1 || WrapX()) {
					int ii = i+1;
					if (ii>GridDim(0)-1) ii = 0;					
					pp[1] = GetControlPoint(t,GridIndex(ii,j,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				if (j<GridDim(1)-1) {
					pp[1] = GetControlPoint(t,GridIndex(i,j+1,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				if (k<GridDim(2)-1) {
					pp[1] = GetControlPoint(t,GridIndex(i,j,k+1),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				}
			}
		}
	
	if (ht && gw->checkHitCode()) {
		gw->clearHitCode();
		if (ip && level) vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL);
		return 1;
		}
	return 0;
	}

template <class T>
int FFDNM<T>::SubNumToRefNum(int subNum)
	{
	if (subNum==1) return -1;
	else return subNum;
	}

template <class T>
BOOL FFDNM<T>::AssignController(Animatable *control,int subAnim)
	{
	ReplaceReference(subAnim,(RefTargetHandle)control);	
	if (subAnim==MASTER_REF) {
		int n = NumPtConts();
		masterCont->SetNumSubControllers(n);
		for (int i=0; i<n; i++)
			if (GetPtCont(i)) masterCont->SetSubController(i,GetPtCont(i));
		}
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return TRUE;
	}

template <class T>
RefTargetHandle FFDNM<T>::GetReference(int i)
	{
	if (i==0) 
		return tmControl;
	else if (i==1) {
		return pblock;
	} else if (i==2) {
		return masterCont;
	} else if (i<NumPts()+3) { 		
		return GetPtCont(i-3);
	} else 
		return NULL;
	}

template <class T>
void  FFDNM<T>::SetReference(int i, RefTargetHandle rtarg)
	{
	if (i==0) 
		tmControl = (Control*)rtarg;
	else if (i==1) {
		pblock = (IParamBlock2*)rtarg;
	} else if (i==2) {
		masterCont = (MasterPointControl*)rtarg;
		if (masterCont)
			masterCont->SetNumSubControllers(NumPtConts());
	} else if (i<NumPts()+3) {
		SetPtCont(i-3,(Control*)rtarg);
	} else 
		assert(0);
	}

template <class T>
TSTR  FFDNM<T>::SubAnimName(int i)
	{
	if (i==0) {
		return GetString(IDS_RB_LATTICETM);
	} else if (i==1) {
		return GetString(IDS_RB_PARAMETERS);
	} else if (i==2) {
		return GetString(IDS_RK_MASTER);
	} else if (i<NumPts()+3) { 		
		TSTR str;
		str.printf(GetString(IDS_RB_CONTROLPOINTN),i-2);
		return str;
	} else 
		return TSTR();
	}

template <class T>
int  FFDNM<T>::RemapRefOnLoad(int iref) {
	if(loadRefVersion == ES_REF_VER_0 && iref > PBLOCK_REF)
		return iref+1;
	return iref;
}

#define FFD_SEL_CHUNKID			0x0100
#define FFD_PT_CHUNKID			0x0200
#define FFD_DIM_CHUNKID			0x0300
#define FFD_SELLEVEL_CHUNKID	0x0400
#define FFD_OFFSETS_CHUNKID		0x0500
// The following chunk tells which reference version we're dealing with
#define REF_VERSION_CHUNK		0x2000

//deform involume setting has changed from 2.5 to 3.0, this will fix that
template <class T>
class InVolFixPLCB : public PostLoadCallback 
{
public:
	FFDNM<T> *ffd;

	InVolFixPLCB(FFDNM<T>* f) { ffd = f; }
	void proc(ILoad *iload)
	{
		if (ffd->loadRefVersion == ES_REF_VER_0)
			ffd->pblock->SetValue(ffd_deform, 0, !ffd->pblock->GetInt(ffd_deform,0));
		delete this;
	}
};

template <class T>
IOResult  FFDNM<T>::Load(ILoad *iload)
	{
	T::Load(iload);	
	iload->RegisterPostLoadCallback(new InVolFixPLCB<T>(this));
	ULONG nb;
	IOResult res;
	loadRefVersion = ES_REF_VER_0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case REF_VERSION_CHUNK:
				res = iload->Read(&loadRefVersion,sizeof(int), &nb);
				break;
			case FFD_DIM_CHUNKID: {
				int dm[3];
				iload->Read(dm,sizeof(int)*3, &nb);
				SetGridDim(dm);
				break;
				}
			case FFD_SEL_CHUNKID:
				sel.Load(iload);
				break;
			case FFD_PT_CHUNKID:
				res = iload->Read(&pt[0],NumPts()*sizeof(Point3), &nb);
				break;
			case FFD_OFFSETS_CHUNKID:
				res = iload->Read(&offsets[0],NumPts()*sizeof(Point3), &nb);
				break;
			case FFD_SELLEVEL_CHUNKID:
				iload->Read(&selLevel,sizeof(selLevel), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

template <class T>
IOResult  FFDNM<T>::Save(ISave *isave)
	{
	T::Save(isave);
	ULONG nb;

	int refVer = ES_REF_VER_1;
	
	isave->BeginChunk(REF_VERSION_CHUNK);
	isave->Write(&refVer,sizeof(int), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(FFD_DIM_CHUNKID);
	isave->Write(&dim[0],sizeof(int)*3,&nb);
	isave->EndChunk();
	
	isave->BeginChunk(FFD_SEL_CHUNKID);
	sel.Save(isave);
	isave->EndChunk();
	
	isave->BeginChunk(FFD_PT_CHUNKID);
	isave->Write(&pt[0],NumPts()*sizeof(Point3), &nb);
	isave->EndChunk();

	isave->BeginChunk(FFD_OFFSETS_CHUNKID);
	isave->Write(&offsets[0],NumPts()*sizeof(Point3), &nb);
	isave->EndChunk();

	isave->BeginChunk(FFD_SELLEVEL_CHUNKID);
	isave->Write(&selLevel,sizeof(selLevel),&nb);
	isave->EndChunk();

	return IO_OK;
	}

template <class T>
int  FFDNM<T>::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flags, ModContext *mc)
	{
	if (mc && mc->box) lbox = *mc->box;
	MakeBoxThick(lbox);
	GraphicsWindow *gw = vpt->getGW();
	
	// Compute a the transformation out of lattice space into world space
	// Then plug this matrix into the GW.	
	Matrix3 tm = CompTM(t,inode,mc);
	gw->setTransform(tm);
	
	// Draw...
	if (inode->Selected() && SuperClassID()!=OSM_CLASS_ID &&
		(!(flags&DISP_SHOWSUBOBJECT) || selLevel==0)) {
		gw->setColor( LINE_COLOR, GetSelColor());
	} else {
		if (ip && ip->GetSubObjectLevel()==SEL_LATTICE) {
			//gw->setColor(LINE_COLOR, (float)1, (float)1, (float)0.0);
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
		} else {
			if (inode->IsFrozen()) {
				gw->setColor(LINE_COLOR,GetUIColor(COLOR_FREEZE));
			} else {
				//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
				gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
				}
			}
		}	
	DrawControlPoints(t,vpt,gw,lbox,FALSE,inode,mc,flags);

	return 0;
	}

template <class T>
void  FFDNM<T>::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{
	if (mc && mc->box) lbox = *mc->box;	
	MakeBoxThick(lbox);
	Point3 s = LatticeSize(lbox);
	Point3 p;
	GraphicsWindow *gw = vpt->getGW();	
	if (lbox.IsEmpty()) return;
	
	// Compute a the transformation out of lattice space into world space	
	Matrix3 tm = CompTM(t,inode,mc);
	
	// Expand the box to include all control points
	for (int i=0; i<NumPts(); i++) {
		p    = GetControlPoint(t,i)*s + lbox.Min();
		box += p * tm;
		}
	}

template <class T>
int  FFDNM<T>::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{
	if (mc && mc->box) lbox = *mc->box;
	MakeBoxThick(lbox);
	GraphicsWindow *gw = vpt->getGW();
	
	// Set the GW into pick mode.
	int savedLimits;
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);

	// Compute a the transformation out of lattice space into world space
	// Then plug this matrix into the GW.
	Matrix3 tm = CompTM(t,inode,mc);	
	gw->setTransform(tm);

	int res;
	//RK:12/14/00, fixed to respect selection crossing setting
	if ((ip && ip->GetSubObjectLevel() != SEL_LATTICE) || type == HITTYPE_SOLID || type == HITTYPE_POINT) 
		{
		res = DrawControlPoints(t,vpt,gw,lbox,TRUE,inode,mc,flags) ;
		}
	else 
		{
		int hitCount = 0;
		DrawControlPoints(t,vpt,gw,lbox,TRUE,inode,mc,0,&hitCount) ;
		res = 0;

		if (hr.crossing)
			{
		 	if (hitCount) res = 1;
			}
		else
			{
			int ct = NumPts();
			if (hitCount == NumPts()) res = 1;
			}
		}

	// Restore the GW's render limits
	gw->setRndLimits(savedLimits);

	return res;
	}

template <class T>
void FFDNM<T>::SelectPt(int i, BOOL s, BOOL clearAll)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));	
	if (clearAll) sel.ClearAll();
	else if (!s) sel.Clear(i);
	else sel.Set(i);
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

template <class T>
BOOL FFDNM<T>::SelectSubAnim(int subNum)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));
	
	BOOL add = GetKeyState(VK_CONTROL)<0;
	BOOL sub = GetKeyState(VK_MENU)<0;

	if (!add && !sub) sel.ClearAll();
	if (sub)
		 sel.Clear(subNum-3);
	else sel.Set(subNum-3);

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	return TRUE;
	}


template <class T>
void  FFDNM<T>::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, 
		BOOL all, BOOL invert)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) return;

	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));
	while (hitRec) {
		BOOL state = selected;
		if (invert) state = !sel[hitRec->hitInfo];
		if (allX || allY || allZ) {
			ExpandSelection(hitRec->hitInfo,state);
		} else {
			if (state) sel.Set(hitRec->hitInfo);
			else       sel.Clear(hitRec->hitInfo);
			}
		if (!all) break;
		hitRec = hitRec->Next();
		}	
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::ClearSelection(int selLevel)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) return;

	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));
	sel.ClearAll();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::SelectAll(int selLevel)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));
	sel.SetAll();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::InvertSelection(int selLevel)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore<T>(this));
	sel = ~sel;
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

template <class T>
void FFDNM<T>::ExpandSelection(int ix, BOOL on)
	{
	int j, k, i;	
	if (dim[0]==0 || dim[1]==0) return;	

	k   = ix/(dim[0]*dim[1]);
	ix -= k*(dim[0]*dim[1]);
	j   = ix/dim[0];
	ix -= j*dim[0];
	i   = ix;

	if (allX) {
		for (int ix=0; ix<dim[0]; ix++) {
			if (allY) {
				for (int jx=0; jx<dim[1]; jx++) {
					int index = GridIndex(ix,jx,k);			
					sel.Set(index,on);
					}
				}
			if (allZ) {
				for (int kx=0; kx<dim[2]; kx++) {
					int index = GridIndex(ix,j,kx);			
					sel.Set(index,on);
					}
				}
			int index = GridIndex(ix,j,k);			
			sel.Set(index,on);
			}
		}
	if (allY) {
		for (int jx=0; jx<dim[1]; jx++) {
			if (allX) {
				for (int ix=0; ix<dim[0]; ix++) {
					int index = GridIndex(ix,jx,k);			
					sel.Set(index,on);
					}
				}
			if (allZ) {
				for (int kx=0; kx<dim[2]; kx++) {
					int index = GridIndex(ix,j,kx);			
					sel.Set(index,on);
					}
				}
			int index = GridIndex(i,jx,k);
			sel.Set(index,on);
			}
		}
	if (allZ) {
		for (int kx=0; kx<dim[2]; kx++) {
			if (allX) {
				for (int ix=0; ix<dim[0]; ix++) {
					int index = GridIndex(ix,j,kx);			
					sel.Set(index,on);
					}
				}
			if (allY) {
				for (int jx=0; jx<dim[1]; jx++) {
					int index = GridIndex(i,jx,kx);			
					sel.Set(index,on);
					}
				}
			int index = GridIndex(i,j,kx);
			sel.Set(index,on);
			}
		}
	}

template <class T>
void  FFDNM<T>::ActivateSubobjSel(int level, XFormModes& modes)
	{
	selLevel = level;
	if (level==SEL_OBJECT) {
		// Make sure these modes aren't still on the command stack.
		ip->DeleteMode(moveMode);
		ip->DeleteMode(rotMode);
		ip->DeleteMode(uscaleMode);
		ip->DeleteMode(nuscaleMode);
		ip->DeleteMode(squashMode);
		ip->DeleteMode(selectMode);		
	} else {
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
		}		
	NotifyDependents(FOREVER, SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	} else { 		
		PlugControllers(t);

		// Compute a matrix to move points
		Matrix3 ctm(1);
		if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
		Matrix3 tm  = ctm * partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		tm *= TransMatrix(val);
		
		// Compute scale (and inverse scale) to and from lattice space
		// and apply it to the matrices	
		Matrix3 stm = ScaleMatrix(LatticeSize(lbox));
		stm.SetTrans(lbox.Min());
		tm  = stm * tm;
		itm = itm * Inverse(stm);

		// Hold for undo
		if (theHold.Holding() && !TestAFlag(A_HELD)) {
			theHold.Put(new MoveRestore<T>(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&pt[i],FOREVER,CTRL_ABSOLUTE);
					pt[i] = (tm*GetPt(i))*itm;
					GetPtCont(i)->SetValue(t,&pt[i],TRUE,CTRL_ABSOLUTE);
				} else {					
					Point3 p = GetPt(i);
					pt[i] = (tm*p)*itm;					
					// Calculate offsets if setting the initial state of the lattice
					if(selLevel==SEL_SETVOLUME) offsets[i] += GetPt(i)-p;//GetPt(i) - GetSourcePoint(i);
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::Rotate(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {
		PlugControllers(t);

		Matrix3 ctm(1);
		if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
		Matrix3 tm  = ctm * partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		Matrix3 mat;
		val.MakeMatrix(mat);
		tm *= mat;

		// Compute scale (and inverse scale) to and from lattice space
		// and apply it to the matrices	
		Matrix3 stm = ScaleMatrix(LatticeSize(lbox));
		stm.SetTrans(lbox.Min());
		tm  = stm * tm;
		itm = itm * Inverse(stm);

		// Hold for undo
		if (theHold.Holding() && !TestAFlag(A_HELD)) {
			theHold.Put(new MoveRestore<T>(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&pt[i],FOREVER,CTRL_ABSOLUTE);
					pt[i] = (tm*GetPt(i))*itm;
					GetPtCont(i)->SetValue(t,&pt[i],TRUE,CTRL_ABSOLUTE);
				} else {
					Point3 p =  GetPt(i);
					pt[i] = (tm*p)*itm;
					// Calculate offsets if setting the initial state of the lattice
					if(selLevel==SEL_SETVOLUME) offsets[i] += GetPt(i)-p;
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::Scale(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {
		PlugControllers(t);

		Matrix3 ctm(1);
		if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
		Matrix3 tm  = ctm * partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		tm *= ScaleMatrix(val);;
		
		// Compute scale (and inverse scale) to and from lattice space
		// and apply it to the matrices	
		Matrix3 stm = ScaleMatrix(LatticeSize(lbox));
		stm.SetTrans(lbox.Min());
		tm  = stm * tm;
		itm = itm * Inverse(stm);

		// Hold for undo
		if (theHold.Holding() && !TestAFlag(A_HELD)) {
			theHold.Put(new MoveRestore<T>(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&pt[i],FOREVER,CTRL_ABSOLUTE);
					pt[i] = (tm*GetPt(i))*itm;
					GetPtCont(i)->SetValue(t,&pt[i],TRUE,CTRL_ABSOLUTE);
				} else {
					Point3 p = GetPt(i);
					pt[i] = (tm*p)*itm;
					// Calculate offsets if setting the initial state of the lattice
					if(selLevel==SEL_SETVOLUME) offsets[i] += GetPt(i)-p;
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}

template <class T>
void  FFDNM<T>::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	// Compute a the transformation out of lattice space into world space	
	if (mc && mc->box) lbox = *mc->box;
	MakeBoxThick(lbox);
	Point3 s = LatticeSize(lbox);
	Matrix3 tm = CompTM(t,node,mc);	

	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		cb->Center(tm.GetTrans(),0);
	} else {
		Point3 cent(0,0,0);
		int ct=0;
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				cent += (GetControlPoint(t,i)*s + lbox.Min()) * tm;
				ct++;
				}
			}
		if (ct) { 
			cent /= float(ct);
			cb->Center(cent,0);
			}
		}
	}

template <class T>
void  FFDNM<T>::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	// Compute a the transformation out of lattice space into world space
	Matrix3 tm = CompTM(t,node,mc);
	cb->TM(tm,0);
	}

template <class T>
TCHAR *FFDNM<T>::GetObjectName()
	{
	static TSTR buf;
	buf.printf("%s %dx%dx%d",GetString(GetNameID()),
		GridDim(1), GridDim(0), GridDim(2));
	return buf;
	}

template <class T>
Matrix3 FFDNM<T>::CompTM(TimeValue t,INode *inode,ModContext *mc)
	{
	// Compute a the transformation out of lattice space into world space	
	//Matrix3 ntm = inode->GetObjTMBeforeWSM(t);
	// RB 2/22/99: When a WS mod is applied to an FFD WS obj, the points are 
	// already transformed into world space so we want to make sure we don't
	// transform twice. GetObjectTM() correctly returns the identity when the
	// object has already been transformed.
	Matrix3 ntm(1);
	if (inode) 
	{
#ifdef DESIGN_VER
		ntm = inode->GetObjectTM(GetCOREInterface()->GetTime()); 
#else
		ntm = inode->GetObjectTM(t); 
#endif // DESIGN_VER
	}
	Matrix3 ctm(1);
	if (mc && mc->tm) {
		ntm = Inverse(*(mc->tm)) * ntm;
		}
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
	return ctm * ntm;
	}

template <class T>
void FFDNM<T>::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editFFD = this;		
	
	// Activate the right-click menu callback
	ffdMenu = new FFDRightMenu<FFDNM<T> >(this);
	ip->GetRightClickMenuManager()->Register(ffdMenu);
	
	// Set up keyboard actions    
	ffdActionCB = new FFDActionCB<FFDNM<T> >(this);
    ip->GetActionManager()->ActivateActionTable(ffdActionCB, kFFDActions);

	// Creates some modes
	moveMode       = new MoveModBoxCMode(this,ip);
	rotMode        = new RotateModBoxCMode(this,ip);
	uscaleMode     = new UScaleModBoxCMode(this,ip);
	nuscaleMode    = new NUScaleModBoxCMode(this,ip);
	squashMode     = new SquashModBoxCMode(this,ip);
	selectMode     = new SelectModBoxCMode(this,ip);

	// Add our sub object type
	if (SuperClassID()==OSM_CLASS_ID) {
		// TSTR type1(GetString(IDS_RB_CONTPOINTS));	
		// TSTR type2(GetString(IDS_RB_LATTICE));	
		// TSTR type3(GetString(IDS_RK_SETVOLUME));
		// const TCHAR *ptype[] = { type1,type2, type3};
		// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
		// 	ip->RegisterSubObjectTypes(ptype, 3);

		// Notify the system that we have an apparatus to display
		TimeValue t = ip->GetTime();
		NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
		NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
		SetAFlag(A_MOD_BEING_EDITED);
	} //else {
		// TSTR type1(GetString(IDS_RB_CONTPOINTS));
		// const TCHAR *ptype[] = { type1};
		// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
		// 	ip->RegisterSubObjectTypes(ptype, 1);
		//}

	if (SuperClassID()==WSM_OBJECT_CLASS_ID) {
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_FFD_SOT),
			DefaultSOTProc,
			GetString(IDS_RB_SOT), 
			(LPARAM)ip,APPENDROLL_CLOSED);
		}

	ParamBlockDesc2 *pbd = GetClassDesc()->GetParamBlockDesc(0);
	pbd->flags |= P_AUTO_UI;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
	pbd->SetUserDlgProc(new FFDDlgProc<T>(this));	
	
	if ((SuperClassID() != WSM_OBJECT_CLASS_ID)) {
		// Disable "conform" controls if multiple objects or non-triobjects are selected
		sMyEnumProc dep;              
		EnumDependents(&dep);
		
		if ((dep.Nodes.Count() != 1) ||
			!dep.Nodes[0]->EvalWorldState(ip->GetTime()).obj->IsSubClassOf(triObjectClassID)) {
				EnableWindow(GetDlgItem(pblock->GetMap()->GetHWnd(),IDC_FFD_CONFORM), FALSE);
				EnableWindow(GetDlgItem(pblock->GetMap()->GetHWnd(),IDC_FFD_INPOINTS), FALSE);
				EnableWindow(GetDlgItem(pblock->GetMap()->GetHWnd(),IDC_FFD_OUTPOINTS), FALSE);
				EnableWindow(GetDlgItem(pblock->GetMap()->GetHWnd(),IDC_FFD_OFFSETLABEL),FALSE);
				ISpinnerControl *spin = GetISpinner(GetDlgItem(pblock->GetMap()->GetHWnd(),IDC_FFD_OFFSETSPIN));				
				spin->Disable();			
				ReleaseISpinner(spin);
			}
	}
	// Restore the selection level
	ip->SetSubObjectLevel(selLevel);

	// Disable show end result.
	ip->EnableShowEndResult(FALSE);
	}

template <class T>
void FFDNM<T>::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	ip->GetRightClickMenuManager()->Unregister(ffdMenu);
	delete ffdMenu;
	ip->GetActionManager()->DeactivateActionTable(ffdActionCB, kFFDActions);
	delete ffdActionCB;

	if (hSot) ip->DeleteRollupPage(hSot);
	hSot = NULL;

	// Turn off aparatus display
	if (SuperClassID()==OSM_CLASS_ID) {
		TimeValue t = ip->GetTime();
		ClearAFlag(A_MOD_BEING_EDITED);
		NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
		NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
		}
	
	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);
	ip->DeleteMode(selectMode);
	if (moveMode) delete moveMode;
	moveMode = NULL;
	if (rotMode) delete rotMode;
	rotMode = NULL;
	if (uscaleMode) delete uscaleMode;
	uscaleMode = NULL;
	if (nuscaleMode) delete nuscaleMode;
	nuscaleMode = NULL;
	if (squashMode) delete squashMode;
	squashMode = NULL;
	if (selectMode) delete selectMode;
	selectMode = NULL;
	
	this->ip = NULL;
	editFFD  = NULL;

	GetClassDesc()->EndEditParams(ip, this, flags, next);	

	// Enable show end result
	ip->EnableShowEndResult(TRUE);
	}

template <class T>
int FFDNM<T>::NumSubObjTypes() 
{ 
	if(SuperClassID()==OSM_CLASS_ID)
		return 3;
	else
		return 1;
}

template <class T>
ISubObjType *FFDNM<T>::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_ContPoints.SetName(GetString(IDS_RB_CONTPOINTS));
		SOT_Lattice.SetName(GetString(IDS_RB_LATTICE));
		SOT_Volume.SetName(GetString(IDS_RK_SETVOLUME));
	}

	switch(i)
	{
	case -1:
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
		break;
	case 0:
		return &SOT_ContPoints;
	case 1:
		return &SOT_Lattice;
	case 2:
		return &SOT_Volume;
	}
	return NULL;
}



//--- FFDNMDeformer -------------------------------------------------


template <class T>
FFDNMDeformer<T>::FFDNMDeformer(
		FFDNM<T> *ffd,TimeValue t,Matrix3 *mtm, Box3 &box, BOOL ws)
	{
	pts = NULL;
	this->ffd = ffd;
	if (!ffd) return;

	inVol = !ffd->pblock->GetInt(ffd_deform,t);
	ffd->pblock->GetValue(ffd_falloff,t,falloff,FOREVER);	
	dim[0] = ffd->dim[0]; dim[1]=ffd->dim[1]; dim[2]=ffd->dim[2];
	wrapX = ffd->WrapX();
	
	// Evaluate any controllers
	for (int i=0; i<ffd->NumPts(); i++) {
		if (ffd->ptCont[i]) {
			ffd->ptCont[i]->GetValue(t,&ffd->pt[i],FOREVER,CTRL_ABSOLUTE);
			}
		}
	
	// Build a cache of all points
	pts = new Point3**[ffd->dim[0]+(ffd->WrapX()?0:2)];
	for (i=0; i<ffd->dim[0]+(ffd->WrapX()?0:2); i++) {
		pts[i] = new Point3*[ffd->dim[1]+2];
		for (int j=0; j<ffd->dim[1]+2; j++) {
			pts[i][j] = new Point3[ffd->dim[2]+2];
			for (int k=0; k<ffd->dim[2]+2; k++) {				
				pts[i][j][k] = ffd->GetPtOR(i-(ffd->WrapX()?0:1),j-1,k-1);
				}
			}
		}

	// Evaluate the TM controller
	Matrix3 ctm(1);
	if (ffd->tmControl)
		ffd->tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
	
	// Start with the ModContext TM or node TM (if there is one)
	tm  = mtm ? *mtm : Matrix3(1);
	
	// Apply our TM to the MC TM
	tm *= Inverse(ctm);
	
	// The origin of the TM is the lower left corner of the
	// box, not the center.
	tm.SetTrans(tm.GetTrans()-box.Min());
	
	// Compute scale factors to normalize lattice space
	// to the extents of the box.
	Point3 s = LatticeSize(box);
	for (i=0; i<3; i++) {
		if (s[i]==0.0f) s[i] = 1.0f;
		else s[i] = 1.0f/s[i];
		}
	tm.Scale(s,TRUE);

	// Compute the inverse.
	itm = Inverse(tm);

	// If the lattice has been transformed into WS we need to untransform the result.
	if (ffd->beenDeformed && mtm && ws) {
		itm = itm * (*mtm);
		}

	// Compute the TCB multipliers	
	float tens = 0.0f;
	float cont = 0.0f;
	ffd->pblock->GetValue(ffd_tens,t,tens,FOREVER);
	ffd->pblock->GetValue(ffd_cont,t,cont,FOREVER);
	tens = (tens-25.0f)/25.0f;
	cont = (cont-25.0f)/25.0f;
	ComputeTCBMults(tens,cont,m1,m2);
	}

inline float BPoly4(int i, float u)
	{
	float s = 1.0f-u;
	switch (i) {
		case 0: return s*s*s;
		case 1: return 3.0f*u*s*s;
		case 2: return 3.0f*u*u*s;
		case 3: return u*u*u;
		default: return 0.0f;
		}
	}

template <class T>
Point3 FFDNMDeformer<T>::Map(int ii, Point3 p)
	{
	if (!ffd) return p;

	Point3 q(0,0,0), pp;
	
	// Transform into lattice space
	pp = p*tm;

	// Allow derived classes to do additional inverse transforms
	pp = ffd->InverseLattice(pp);	

	// Compute distance for falloff
	float dist = 0.0f;
	if (inVol) { 		
		if (ffd->WrapX()) {
			if (pp.y<-0.001f || pp.y>1.001f ||
				pp.z<-0.001f || pp.z>1.001f) {
				return p;
				}
		} else {
			if (pp.x<-0.001f || pp.x>1.001f ||
				pp.y<-0.001f || pp.y>1.001f ||
				pp.z<-0.001f || pp.z>1.001f) {
				return p;
				}
			}
	} else
	if (falloff>0.0f) {
		float closest = 0.0f;
		BOOL found = FALSE;
		if (!ffd->WrapX()) {
			if (pp.x<0.0f && -pp.x>closest) { closest=-pp.x; found=TRUE; }
			if (pp.x>1.0f && (pp.x-1.0f)>closest) { closest=pp.x-1.0f; found=TRUE; }
			}
		if (pp.y<0.0f && -pp.y>closest) { closest=-pp.y; found=TRUE; }
		if (pp.y>1.0f && (pp.y-1.0f)>closest) { closest=pp.y-1.0f; found=TRUE; }
		if (pp.z<0.0f && -pp.z>closest) { closest=-pp.z; found=TRUE; }		
		if (pp.z>1.0f && (pp.z-1.0f)>closest) { closest=pp.z-1.0f; found=TRUE; }
		if (found) {
			dist = closest;
			if (dist>falloff) return p;
			}
		}

	// Find the cell we're in	
	pp.x = pp.x*float(ffd->dim[0]-(ffd->WrapX()?0:1));
	pp.y = pp.y*float(ffd->dim[1]-1);
	pp.z = pp.z*float(ffd->dim[2]-1);
	int i = int(pp.x);
	int j = int(pp.y);
	int k = int(pp.z);
	int io=i, jo=j, ko=k;
	if (!ffd->WrapX()) {
		if (i<0) i = 0; if (i>ffd->dim[0]-2) i = ffd->dim[0]-2;
		}
	if (j<0) j = 0; if (j>ffd->dim[1]-2) j = ffd->dim[1]-2;
	if (k<0) k = 0; if (k>ffd->dim[2]-2) k = ffd->dim[2]-2;	

	// Make pp relative to our cell
	pp.x -= float(i);
	pp.y -= float(j);
	pp.z -= float(k);

	// We are going to consider all surrounding cells. Make i,j,k
	// refer to the corner of a 3x3x3 cell array with the current cell at the center.
	i -= 1; j -= 1; k -= 1;

#if 0	
	for (int ix=0; ix<4; ix++) {
		for (int jx=0; jx<4; jx++) {
			Point3 knots[4];
			for (int kx=0; kx<4; kx++) { 				
				int ii;
				if (ffd->WrapX()) {
					ii = i+ix;
					if (ii<0) ii = dim[0]+ii;
					if (ii>dim[0]-1) ii = ii-dim[0];
				} else {
					ii = i+ix+1;
					}
				q += (pts[ii][j+jx+1][k+kx+1]+Point3(0.5,0.5,0.5))*
					BPoly4(ii,pp.x)*
					BPoly4(j+jx+1,pp.y)*
					BPoly4(k+kx+1,pp.z);
				}			
			}
		}
	return q*itm;
#else 	
	
	// Interpolate the 64 control points using Z to get 16 points
	Point3 pt[4][4];
	for (int ix=0; ix<4; ix++) {
		for (int jx=0; jx<4; jx++) {
			Point3 knots[4];
			for (int kx=0; kx<4; kx++) { 				
				int ii;
				if (ffd->WrapX()) {
					ii = i+ix;
					if (ii<0) ii = dim[0]+ii;
					if (ii>dim[0]-1) ii = ii-dim[0];
				} else {
					ii = i+ix+1;
					}
				knots[kx] = pts[ii][j+jx+1][k+kx+1];					
				}
			pt[ix][jx] = InterpSpline(pp.z,knots,m1,m2);
			}
		}

	// Now interpolate 16 points to get a single 4 point spline
	Point3 knots[4];
	for (ix=0; ix<4; ix++) {
		knots[ix] = InterpSpline(pp.y,pt[ix],m1,m2);
		}

	// Finally we get the point
	q = InterpSpline(pp.x,knots,m1,m2) * itm;

	if (falloff>0.0f && dist>0.0f) {
		float u=dist/falloff;
		u = (u*u*(3-2*u));
		q = u*p + (1.0f-u)*q;
		}	 
	return q;
#endif
	}

//--- FFDNMSquare ----------------------------------------------------

template <class T>
Point3 FFDNMSquare<T>::GetSourcePoint(int i)
	{
	int j, k;
	
	if (dim[0]==0 || dim[1]==0) return Point3(0,0,0);

	k = i/(dim[0]*dim[1]);
	i -= k*(dim[0]*dim[1]);
	j = i/dim[0];
	i -= j*dim[0];

	return Point3(
		float(i)/float(dim[0]-1),
		float(j)/float(dim[1]-1),
		float(k)/float(dim[2]-1));
	}


//--- FFDNMCyl ----------------------------------------------------

// Radius of sqrt(.5) for a cylinder that bounds a 1x1 box
#define CYL_RAD	0.7071067812f

template <class T>
Point3 FFDNMCyl<T>::GetSourcePoint(int i)
	{
	int j, k;
	
	if (dim[0]==0 || dim[1]==0) return Point3(0,0,0);

	k = i/(dim[0]*dim[1]);
	i -= k*(dim[0]*dim[1]);
	j = i/dim[0];
	i -= j*dim[0];

	float u = float(i)/float(dim[0]);
	float v = float(j)/float(dim[1]-1);
	
	return Point3(
		float(cos(u*TWOPI)*CYL_RAD*v+0.5),
		float(sin(u*TWOPI)*CYL_RAD*v+0.5),
		float(k)/float(dim[2]-1));
	}

template <class T>
Point3 FFDNMCyl<T>::InverseLattice(Point3 p)
	{
	float x = p.x-0.5f;
	float y = p.y-0.5f;
	float l = (float)sqrt(x*x + y*y);
	if (l==0.0f) {
		p.x = 0.0f;
	} else {
		p.x = float(atan2(y,x));
		if (p.x<0.0f) p.x += TWOPI;
		p.x /= TWOPI;		
		}
	p.y = l/CYL_RAD;	
	return p;
	}


//--- FFDNMSquareOSMod ----------------------------------------------------


template <class T>
FFDNMOSMod<T>::FFDNMOSMod()
	{	
	tmControl = NULL;
	MakeRefByID(FOREVER,TM_REF,NewDefaultMatrix3Controller()); 	
	pblock = NULL;	

	// Create the ffd master control
	masterCont = NULL;		
	MakeRefByID(FOREVER, MASTER_REF, NewDefaultMasterPointController());	
	int dm[3] = { 4,4,4};
	SetGridDim(dm);
	}

FFDNMSquareOSMod::FFDNMSquareOSMod()
	{ 	
	ffdos_box_param_blk.dlg_template = IDD_FFDNMOSPARAMS;
	ffdos_box_param_blk.title = IDS_RB_PARAMETERS;

	GetClassDesc()->MakeAutoParamBlocks(this);
	}

FFDNMCylOSMod::FFDNMCylOSMod() 
	{ 
	ffdos_cyl_param_blk.dlg_template = IDD_FFDNMOSPARAMS;
	ffdos_cyl_param_blk.title = IDS_RB_PARAMETERS;
	GetClassDesc()->MakeAutoParamBlocks(this);
	}

Interval FFDNMSquareOSMod::LocalValidity(TimeValue t)
	{ 	
	// If we're being edited, then returning NEVER for our
	// validity will ensure that the output of the previous
	// modifier is cached.
	if (TestAFlag(A_MOD_BEING_EDITED))
		 return NEVER;  
	else {
		// Our validity depends on whether any of our controllers
		// are animated.
		Interval valid = FOREVER;
		Matrix3 ctm(1);
		Point3 p;
		tmControl->GetValue(t,&ctm,valid,CTRL_RELATIVE);
		float f;
		pblock->GetValue(ffd_falloff,t,f,valid);
		pblock->GetValue(ffd_tens,t,f,valid);
		pblock->GetValue(ffd_cont,t,f,valid);
		for (int i=0; i<NumPts(); i++) { 			
			if (GetPtCont(i)) GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
			}
		return valid;
		}
	}
Interval FFDNMCylOSMod::LocalValidity(TimeValue t)
	{ 	
	// If we're being edited, then returning NEVER for our
	// validity will ensure that the output of the previous
	// modifier is cached.
	if (TestAFlag(A_MOD_BEING_EDITED))
		 return NEVER;  
	else {
		// Our validity depends on whether any of our controllers
		// are animated.
		Interval valid = FOREVER;
		Matrix3 ctm(1);
		Point3 p;
		tmControl->GetValue(t,&ctm,valid,CTRL_RELATIVE);
		float f;		
		pblock->GetValue(ffd_falloff,t,f,valid);
		pblock->GetValue(ffd_tens,t,f,valid);
		pblock->GetValue(ffd_cont,t,f,valid);
		for (int i=0; i<NumPts(); i++) { 			
			if (GetPtCont(i)) GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
			}
		return valid;
		}
	}


void FFDNMSquareOSMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{	
	// Compute our validity interval
	Interval valid = LocalValidity(t);
	if (valid.Empty()) valid.SetInstant(t);

	// Cache the input box
	lbox = *mc.box;
	MakeBoxThick(lbox);
	
	// Apply the deformer	
	FFDNMDeformer<Modifier> deformer(this,t,mc.tm,lbox);
	os->obj->Deform(&deformer, TRUE);	
	deformer.FreePointCache();

	// This will intersect our validity with the object's
	// validity.
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
	}
void FFDNMCylOSMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	// Compute our validity interval
	Interval valid = LocalValidity(t);
	if (valid.Empty()) valid.SetInstant(t);

	// Cache the input box
	lbox = *mc.box;
	MakeBoxThick(lbox);

	// Apply the deformer
	FFDNMDeformer<Modifier> deformer(this,t,mc.tm,lbox);	
	os->obj->Deform(&deformer, TRUE);		
	deformer.FreePointCache();	

	// This will intersect our validity with the object's
	// validity.
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);
	}

RefTargetHandle FFDNMSquareOSMod::Clone(RemapDir &remap)
	{
	FFDNMSquareOSMod *mod = new FFDNMSquareOSMod;
	BaseClone(mod,remap);
	BaseObject::BaseClone(this, mod, remap);
	return mod;
	}
RefTargetHandle FFDNMCylOSMod::Clone(RemapDir &remap)
	{
	FFDNMCylOSMod *mod = new FFDNMCylOSMod;
	BaseClone(mod,remap);
	BaseObject::BaseClone(this, mod, remap);
	return mod;
	}

IOResult FFDNMSquareOSMod::Load(ILoad *iload)
	{
	FFDNM<Modifier>::Load(iload);	
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(versionsMod, NUM_OLDVERSIONS, &ffdos_box_param_blk, this, PBLOCK_REF));
	return IO_OK;
	}
IOResult FFDNMCylOSMod::Load(ILoad *iload)
	{
	FFDNM<Modifier>::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(versionsMod, NUM_OLDVERSIONS, &ffdos_cyl_param_blk, this, PBLOCK_REF));
	return IO_OK;
	}


//--- FFDNMSquareWSObj --------------------------------------------------

FFDNMSquareWSObj::FFDNMSquareWSObj(BOOL noRef)
	{ 	
	geomValid = selValid = FOREVER;
	selLevelValid = TRUE;
	if (!noRef) {		
		pblock = NULL;				
		GetClassDesc()->MakeAutoParamBlocks(this);		
		masterCont = NULL;		
		MakeRefByID(FOREVER, MASTER_REF, NewDefaultMasterPointController());	
		int dm[3] = { 4,4,4};
		SetGridDim(dm);
	} else {
		pblock = NULL;		
		}
	}
FFDNMCylWSObj::FFDNMCylWSObj(BOOL noRef)
	{
	geomValid = selValid = FOREVER;
	selLevelValid = TRUE;
	if (!noRef) {		
		pblock = NULL;				
		GetClassDesc()->MakeAutoParamBlocks(this);
		masterCont = NULL;		
		MakeRefByID(FOREVER, MASTER_REF, NewDefaultMasterPointController());	
		int dm[3] = { 8,4,4};
		SetGridDim(dm);
	} else {
		pblock = NULL;
		}
	}


BOOL FFDNMSquareWSObj::IsSubClassOf(Class_ID classID)
	{
	return ((classID==ClassID()) || (classID==GENFFDOBJECT_CLASS_ID));
	}
BOOL FFDNMCylWSObj::IsSubClassOf(Class_ID classID)
	{
	return ((classID==ClassID()) || (classID==GENFFDOBJECT_CLASS_ID));
	}

int FFDNMSquareWSObj::CanConvertToType(Class_ID obtype) 
	{
	return IsSubClassOf(obtype) || obtype==defObjectClassID;
	}
int FFDNMCylWSObj::CanConvertToType(Class_ID obtype) 
	{
	return IsSubClassOf(obtype) || obtype==defObjectClassID;
	}

RefTargetHandle FFDNMSquareWSObj::Clone(RemapDir &remap)
	{
	FFDNMSquareWSObj *obj = new FFDNMSquareWSObj;
	BaseClone(obj,remap);
	BaseObject::BaseClone(this, obj, remap);
	return obj;
	}
RefTargetHandle FFDNMCylWSObj::Clone(RemapDir &remap)
	{
	FFDNMCylWSObj *obj = new FFDNMCylWSObj;
	BaseClone(obj,remap);
	BaseObject::BaseClone(this, obj, remap);
	return obj;
	}


IOResult FFDNMSquareWSObj::Load(ILoad *iload)
	{
	FFDNM<WSMObject>::Load(iload);
	iload->RegisterPostLoadCallback(		
		new ParamBlock2PLCB(versionsObjSquare, NUM_OLDVERSIONS, &ffdws_box_param_blk, this, PBLOCK_REF));
	return IO_OK;
	}
IOResult FFDNMCylWSObj::Load(ILoad *iload)
	{
	FFDNM<WSMObject>::Load(iload);
	iload->RegisterPostLoadCallback(		
		new ParamBlock2PLCB(versionsObjCyl, NUM_OLDVERSIONS, &ffdws_cyl_param_blk, this, PBLOCK_REF));
	return IO_OK;
	}

void FFDNMSquareWSObj::GetWorldBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
	{
	UpdateBox(t);
	FFDNM<WSMObject>::GetWorldBoundBox(t,inode,vpt,box,NULL);
	}
void FFDNMCylWSObj::GetWorldBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
	{
	UpdateBox(t);
	FFDNM<WSMObject>::GetWorldBoundBox(t,inode,vpt,box,NULL);
	}


void FFDNMSquareWSObj::GetLocalBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
	{
	UpdateBox(t);
	box = lbox;
	
	if (beenDeformed) {
		Box3 abox; abox.Init();
		Point3 s = LatticeSize(lbox);
		for (int i=0; i<pt.Count(); i++) {			
			abox += pt[i]*s + lbox.Min();
			}
		box = abox;
		}
	}
void FFDNMCylWSObj::GetLocalBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
	{
	UpdateBox(t);	
	box = lbox;	

	if (beenDeformed) {
		Box3 abox; abox.Init();
		Point3 s = LatticeSize(lbox);
		for (int i=0; i<pt.Count(); i++) {			
			abox += pt[i]*s + lbox.Min();
			}
		box = abox;
		}
	}

void FFDNMSquareWSObj::GetDeformBBox(
		TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
	{
	Matrix3 ttm(1);
	if (!tm) tm = &ttm;
	UpdateBox(t);
	if (selLevel!=SEL_POINTS && selLevel!=SEL_SETVOLUME) useSel = FALSE;
	if (useSel || tm) {
		Point3 s = LatticeSize(lbox);
		box.Init();
		for (int i=0; i<pt.Count(); i++) {
			if (sel[i] || !useSel) {
				box += (pt[i]*s + lbox.Min())* *tm;
				}
			}
	} else {
		box = lbox;
		}
	}
void FFDNMCylWSObj::GetDeformBBox(
		TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
	{
	Matrix3 ttm(1);
	if (!tm) tm = &ttm;
	UpdateBox(t);
	if (selLevel!=SEL_POINTS && selLevel!=SEL_SETVOLUME) useSel = FALSE;
	if (useSel || tm) {
		Point3 s = LatticeSize(lbox);
		box.Init();
		for (int i=0; i<pt.Count(); i++) {
			if (sel[i] || !useSel) {
				box += (pt[i]*s + lbox.Min())* *tm;
				}
			}
	} else {
		box = lbox;
		}
	}

int FFDNMSquareWSObj::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{
	UpdateBox(t);
	return FFDNM<WSMObject>::Display(t,inode,vpt,flags,NULL);		
	}
int FFDNMCylWSObj::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{
	UpdateBox(t);
	return FFDNM<WSMObject>::Display(t,inode,vpt,flags,NULL);		
	}

int FFDNMSquareWSObj::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt)
	{
	return FFDNM<WSMObject>::HitTest(t,inode,type,crossing,flags,p,vpt,NULL);
	}
int FFDNMCylWSObj::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt)
	{
	return FFDNM<WSMObject>::HitTest(t,inode,type,crossing,flags,p,vpt,NULL);
	}

Interval FFDNMSquareWSObj::ObjectValidity(TimeValue t)
	{
	Interval valid = FOREVER;	
	float f;
	Point3 p;	
	valid &= geomValid;
	valid &= selValid;
	valid &= Object::ObjectValidity(t);

	for (int i=0; i<NumPts(); i++) { 			
		if (GetPtCont(i)) GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
		}
	pblock->GetValue(ffd_width,t,f,valid);
	pblock->GetValue(ffd_length,t,f,valid);
	pblock->GetValue(ffd_height,t,f,valid);
	return valid;
	}
Interval FFDNMCylWSObj::ObjectValidity(TimeValue t)
	{
	Interval valid = FOREVER;	
	float f;
	Point3 p;	
	valid &= geomValid;
	valid &= selValid;
	valid &= Object::ObjectValidity(t);

	for (int i=0; i<NumPts(); i++) { 			
		if (GetPtCont(i)) GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
		}
	pblock->GetValue(ffd_width,t,f,valid);
	pblock->GetValue(ffd_length,t,f,valid);	
	return valid;
	}


static void RectifyBox(Box3 &box)
	{
	for (int i=0; i<3; i++) {
		if (box.pmin[i]>box.pmax[i]) {
			float tmp   = box.pmin[i];
			box.pmin[i] = box.pmax[i];
			box.pmax[i] = tmp;
			}
		}
	}

void FFDNMSquareWSObj::UpdateBox(TimeValue t)
	{	
	float l, w, h;	
	pblock->GetValue(ffd_width,t,w,FOREVER);
	pblock->GetValue(ffd_length,t,l,FOREVER);
	pblock->GetValue(ffd_height,t,h,FOREVER);

	lbox.pmin = Point3(-w/2.0f,-l/2.0f,-h/2.0f);
	lbox.pmax = Point3( w/2.0f, l/2.0f, h/2.0f);	

	RectifyBox(lbox);
	}
void FFDNMCylWSObj::UpdateBox(TimeValue t)
	{
	float r,h;		
	pblock->GetValue(ffd_width,t,r,FOREVER);
	pblock->GetValue(ffd_length,t,h,FOREVER);	

	lbox.pmin = Point3(-r,-r,-h/2.0f);
	lbox.pmax = Point3( r, r, h/2.0f);

	RectifyBox(lbox);
	}

Modifier *FFDNMSquareWSObj::CreateWSMMod(INode *node)
	{
	return new FFDNMWSMod(node);
	}
Modifier *FFDNMCylWSObj::CreateWSMMod(INode *node)
	{
	return new FFDNMWSMod(node);
	}

ObjectState FFDNMSquareWSObj::Eval(TimeValue t) 
	{
	UpdateBox(t);
	selValid = geomValid = FOREVER;
	selLevelValid = TRUE;
	for (int i=0; i<ptCont.Count(); i++) {
		if (ptCont[i]) ptCont[i]->GetValue(t,&pt[i],geomValid,CTRL_ABSOLUTE);
		}
	return ObjectState(this);
	}
ObjectState FFDNMCylWSObj::Eval(TimeValue t) 
	{
	UpdateBox(t);
	selValid = geomValid = FOREVER;
	selLevelValid = TRUE;
	for (int i=0; i<ptCont.Count(); i++) {
		if (ptCont[i]) ptCont[i]->GetValue(t,&pt[i],geomValid,CTRL_ABSOLUTE);
		}
	return ObjectState(this);
	}

Interval FFDNMSquareWSObj::ChannelValidity(TimeValue t, int nchan)
	{
	
	if(IsBaseClassOwnedChannel(nchan))
		return Object::ChannelValidity(t,nchan);

	switch(nchan) {
		case GEOM_CHAN_NUM: return geomValid;		
		case SELECT_CHAN_NUM: return selValid;
		case SUBSEL_TYPE_CHAN_NUM: return selLevelValid?FOREVER:NEVER;
		default: return FOREVER;
		}
	}
Interval FFDNMCylWSObj::ChannelValidity(TimeValue t, int nchan)
	{
	
	if(IsBaseClassOwnedChannel(nchan))
		return Object::ChannelValidity(t,nchan);

	switch(nchan) {
		case GEOM_CHAN_NUM: return geomValid;		
		case SELECT_CHAN_NUM: return selValid;
		case SUBSEL_TYPE_CHAN_NUM: return selLevelValid?FOREVER:NEVER;
		default: return FOREVER;
		}
	}

void FFDNMSquareWSObj::SetChannelValidity(int i, Interval v)
	{
	
	Object::SetChannelValidity(i,v);
	
	switch(i) {
		case GEOM_CHAN_NUM:  geomValid = v; break;		
		case SELECT_CHAN_NUM: selValid = v; break;
		case SUBSEL_TYPE_CHAN_NUM:
			if (v.InInterval(0)) selLevelValid = TRUE;
			else selLevelValid = FALSE;
			break;
		}
	}
void FFDNMCylWSObj::SetChannelValidity(int i, Interval v)
	{
	
	Object::SetChannelValidity(i,v);

	switch(i) {
		case GEOM_CHAN_NUM:  geomValid = v; break;		
		case SELECT_CHAN_NUM: selValid = v; break;
		case SUBSEL_TYPE_CHAN_NUM:
			if (v.InInterval(0)) selLevelValid = TRUE;
			else selLevelValid = FALSE;
			break;
		}
	}

void FFDNMSquareWSObj::InvalidateChannels(ChannelMask channels)
	{
	
	Object::InvalidateChannels(channels);

	if (channels&GEOM_CHANNEL) geomValid.SetEmpty();
	if (channels&SELECT_CHANNEL) selValid.SetEmpty();
	if (channels&SUBSEL_TYPE_CHANNEL) selLevelValid = FALSE;
	}
void FFDNMCylWSObj::InvalidateChannels(ChannelMask channels)
	{
	Object::InvalidateChannels(channels);
	if (channels&GEOM_CHANNEL) geomValid.SetEmpty();
	if (channels&SELECT_CHANNEL) selValid.SetEmpty();
	if (channels&SUBSEL_TYPE_CHANNEL) selLevelValid = FALSE;
	}

Object* FFDNMSquareWSObj::ConvertToType(TimeValue t, Class_ID obtype)
	{
	if (IsSubClassOf(obtype) || obtype==defObjectClassID) return this;
	return NULL;
	}
Object* FFDNMCylWSObj::ConvertToType(TimeValue t, Class_ID obtype)
	{
	if (IsSubClassOf(obtype) || obtype==defObjectClassID) return this;
	return NULL;
	}

Object *FFDNMSquareWSObj::MakeShallowCopy(ChannelMask channels)
	{
	FFDNMSquareWSObj *obj = new FFDNMSquareWSObj(TRUE);
	BaseClone(obj,NoRemap(),TRUE);
	obj->Object::ShallowCopy(this,channels);

	obj->selValid  = selValid;
	obj->geomValid = geomValid;
	obj->selLevelValid = selLevelValid;
	return obj;
	}
Object *FFDNMCylWSObj::MakeShallowCopy(ChannelMask channels)
	{
	FFDNMCylWSObj *obj = new FFDNMCylWSObj(TRUE);
	BaseClone(obj,NoRemap(),TRUE);
	obj->Object::ShallowCopy(this,channels);
	
	obj->selValid  = selValid;
	obj->geomValid = geomValid;
	obj->selLevelValid = selLevelValid;
	return obj;
	}

void FFDNMSquareWSObj::ShallowCopy(Object* fromOb, ChannelMask channels)
	{
	assert(fromOb->ClassID()==ClassID());
	FFDNMSquareWSObj *ob = (FFDNMSquareWSObj*)fromOb;
	
	Object::ShallowCopy(fromOb,channels);
	
	if (channels&GEOM_CHANNEL)   {
		pt = ob->pt;
		offsets = ob->offsets;
		geomValid = ob->geomValid;
		}
	if (channels&SELECT_CHANNEL) {
		sel = ob->sel;
		selValid = ob->selValid;
		}
	if (channels&SUBSEL_TYPE_CHANNEL) { 	
		selLevel = ob->selLevel;
		selLevelValid = ob->selLevelValid;
		}
	}
void FFDNMCylWSObj::ShallowCopy(Object* fromOb, ChannelMask channels)
	{
	assert(fromOb->ClassID()==ClassID());
	FFDNMCylWSObj *ob = (FFDNMCylWSObj*)fromOb;
	Object::ShallowCopy(fromOb,channels);
	
	if (channels&GEOM_CHANNEL) {
		pt = ob->pt;
		offsets = ob->offsets;
		geomValid = ob->geomValid;
		}
	if (channels&SELECT_CHANNEL) {
		sel = ob->sel;
		selValid = ob->selValid;
		}
	if (channels&SUBSEL_TYPE_CHANNEL) { 	
		selLevel = ob->selLevel;
		selLevelValid = ob->selLevelValid;
		}
	}

void FFDNMSquareWSObj::Deform(Deformer *defProc, int useSel)
	{
	if (selLevel!=SEL_POINTS) useSel = FALSE;
	Point3 s = LatticeSize(lbox);
	for (int i=0; i<pt.Count(); i++) {
		if (sel[i] || !useSel) {
			Point3 p = pt[i]*s + lbox.Min(); 
			p = defProc->Map(i,p);
			pt[i] = (p-lbox.Min())/s;
			}
		}
	beenDeformed = TRUE;
	}
void FFDNMCylWSObj::Deform(Deformer *defProc, int useSel)
	{
	if (selLevel!=SEL_POINTS) useSel = FALSE;
	Point3 s = LatticeSize(lbox);
	for (int i=0; i<pt.Count(); i++) {
		if (sel[i] || !useSel) {
			Point3 p = pt[i]*s + lbox.Min();
			p = defProc->Map(i,p);
			pt[i] = (p-lbox.Min())/s;
			}
		}
	beenDeformed = TRUE;
	}


class FFDObjSquareCreateCallBack: public CreateMouseCallBack { 	
	FFDNMSquareWSObj *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;	
	public:
		int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);		
		void SetObj(FFDNMSquareWSObj *obj) { ob = obj; }
	};
class FFDObjCylCreateCallBack: public CreateMouseCallBack { 	
	FFDNMCylWSObj *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;	
	public:
		int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);		
		void SetObj(FFDNMCylWSObj *obj) { ob = obj; }
	};

int FFDObjSquareCreateCallBack::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat) 
	{
	Point3 d, xyz;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				ob->pblock->SetValue(ffd_width,0,0.0f);
				ob->pblock->SetValue(ffd_length,0,0.0f);
				ob->pblock->SetValue(ffd_height,0,0.0f);
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1 = p0 + Point3(.01,.01,.01);				
				xyz = float(.5)*(p0+p1);
				mat.SetTrans(xyz);
				break;
			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1.z = p0.z +(float).01; 
				xyz = float(.5)*(p0+p1);
				mat.SetTrans(xyz);
				d = p1-p0;								
				ob->pblock->SetValue(ffd_width,0,float(fabs(d.x)));
				ob->pblock->SetValue(ffd_length,0,float(fabs(d.y)));
				ob->pblock->SetValue(ffd_height,0,float(fabs(d.z)));
				ob->pblock->GetMap()->Invalidate();										

				if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
					}
				break;
			case 2:
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
				xyz = float(.5)*(p0+p1);
				mat.SetTrans(xyz);
				d = p1-p0;				
				ob->pblock->SetValue(ffd_width,0,float(fabs(d.x)));
				ob->pblock->SetValue(ffd_length,0,float(fabs(d.y)));
				ob->pblock->SetValue(ffd_height,0,float(fabs(d.z)));
				ob->pblock->GetMap()->Invalidate();				
					
				if (msg==MOUSE_POINT) {
					return CREATE_STOP;
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) { 		
		return CREATE_ABORT;
		}
	else
	if (msg == MOUSE_FREEMOVE) {
		vpt->SnapPreview(m,m);
		}

	return TRUE;
	}

int FFDObjCylCreateCallBack::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat) 
	{
	Point3 d, xyz;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				ob->pblock->SetValue(ffd_width,0,0.0f);
				ob->pblock->SetValue(ffd_length,0,0.0f);				
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);				
				mat.SetTrans(p0);
				break;
			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1.z = p0.z +(float).01; 				
				mat.SetTrans(p0);
				d = p1-p0;								
				ob->pblock->SetValue(ffd_width,0,Length(d));
				ob->pblock->SetValue(ffd_length,0,float(fabs(d.z)));				
				ob->pblock->GetMap()->Invalidate();										

				if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
					}
				break;
			case 2:
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));				
				xyz  = p0;
				xyz.z = 0.5f*(p0.z+p1.z);
				mat.SetTrans(xyz);
				d = p1-p0;				
				ob->pblock->SetValue(ffd_length,0,float(fabs(d.z)));				
				ob->pblock->GetMap()->Invalidate();				
					
				if (msg==MOUSE_POINT) {
					return CREATE_STOP;
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) { 		
		return CREATE_ABORT;
		}
	else
	if (msg == MOUSE_FREEMOVE) {
		vpt->SnapPreview(m,m);
		}

	return TRUE;
	}

CreateMouseCallBack* FFDNMSquareWSObj::GetCreateMouseCallBack()
	{
	static FFDObjSquareCreateCallBack createCB;
	createCB.SetObj(this);
	return &createCB;
	}
CreateMouseCallBack* FFDNMCylWSObj::GetCreateMouseCallBack()
	{
	static FFDObjCylCreateCallBack createCB;
	createCB.SetObj(this);
	return &createCB;
	}


//--- FFDNMSquareWSMod ------------------------------------------------

FFDNMWSMod::FFDNMWSMod()
	{
	nodeRef = NULL;
	pblock = NULL;	
	obRef = NULL;
	}

FFDNMWSMod::FFDNMWSMod(INode *node)
	{
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;	
	obRef = NULL;
	}

RefResult FFDNMWSMod::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
	{
	if (message==REFMSG_OBJECT_CACHE_DUMPED) return REF_STOP;
	else return SimpleWSMMod::NotifyRefChanged(changeInt,hTarget,partID,message);
	}

RefTargetHandle FFDNMWSMod::Clone(RemapDir &remap)
	{
	FFDNMWSMod *mod = new FFDNMWSMod;
	mod->ReplaceReference(SIMPWSMMOD_NODEREF,nodeRef);
	BaseClone(this, mod, remap);
	return mod;
	}

static FFDNMDeformer<WSMObject> deformer;

Deformer& FFDNMWSMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{ 	
	FFDNM<WSMObject> *obj = (FFDNM<WSMObject>*)GetWSMObject(t);
	if (obj) obj->UpdateBox(t);
	
	// We want the TM even if a WS mod is applied to the FFD object.
	// We also want to compare the before and after TMs so we can determine if the lattice points
	// have already been transformed into WS. If they have then we need to include an untransform
	// so the verts deformed by the FFD don't get an extra transform.
	//Matrix3 tm = Inverse(nodeRef->GetObjectTM(t));	
	Matrix3 tm  = Inverse(nodeRef->GetObjTMBeforeWSM(t)); 
	Matrix3 tm2 = Inverse(nodeRef->GetObjTMAfterWSM(t)); 

	deformer.FreePointCache();
	deformer = FFDNMDeformer<WSMObject>(obj,t,&tm,obj->lbox, !(tm==tm2));	
	return deformer;
	}

Interval FFDNMWSMod::GetValidity(TimeValue t)
	{
	Interval valid = FOREVER;
	if (nodeRef) { 		
		FFDNM<WSMObject> *obj = (FFDNM<WSMObject>*)GetWSMObject(t);
		nodeRef->GetObjectTM(t,&valid);
		if (obj) {
			float f;
			valid &= obj->ChannelValidity(t,GEOM_CHAN_NUM);
			valid &= obj->ChannelValidity(t,SELECT_CHAN_NUM);
			obj->pblock->GetValue(ffd_falloff,t,f,valid);
			obj->pblock->GetValue(ffd_tens,t,f,valid);
			obj->pblock->GetValue(ffd_cont,t,f,valid);
			obj->pblock->GetValue(ffd_length,t,f,valid);
			obj->pblock->GetValue(ffd_width,t,f,valid);
			if (obj->ClassID()==FFDNMWSSQUARE_CLASS_ID) {
				obj->pblock->GetValue(ffd_height,t,f,valid);
				}
			Point3 p;
			for (int i=0; i<obj->NumPts(); i++) { 			
				if (obj->GetPtCont(i)) obj->GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
				}
			}
		}
	return valid;
	}


//--------------------------------------------------------------------
// Catmull-Rom spline from Texture and Modeling A Procedural Approach

#define CR00	-0.5f
#define CR01	 1.5f
#define CR02	-1.5f
#define CR03	 0.5f
#define CR10	 1.0f
#define CR11	-2.5f
#define CR12	 2.0f
#define CR13	-0.5f
#define CR20	-0.5f
#define CR21	 0.0f
#define CR22	 0.5f
#define CR23     0.0f
#define CR30     0.0f
#define CR31	 1.0f
#define CR32     0.0f
#define CR33     0.0f

static Point3 InterpSplineCatmullRom(float u,Point3 knot[4])
	{
	Point3 c0, c1, c2, c3;
	c3 = (CR00 * knot[0]) + (CR01 * knot[1])
	   + (CR02 * knot[2]) + (CR03 * knot[3]);
	c2 = (CR10 * knot[0]) + (CR11 * knot[1])
	   + (CR12 * knot[2]) + (CR13 * knot[3]);
	c1 = (CR20 * knot[0]) + (CR21 * knot[1])
	   + (CR22 * knot[2]) + (CR23 * knot[3]);
	c0 = (CR30 * knot[0]) + (CR31 * knot[1])
	   + (CR32 * knot[2]) + (CR33 * knot[3]);

	return ((c3*u + c2)*u + c1)*u + c0;
	}


//------------------------------------------------------------------- 
// TCB ... except we're not going to use bias

static void ComputeHermiteBasis(float u, float *v) 
	{
#if 1
	float u2,u3,a;
	
	u2 = u*u;
	u3 = u2*u;
	a  = 2.0f*u3 - 3.0f*u2;
	v[0] = 1.0f + a;
	v[1] = -a;
	v[2] = u - 2.0f*u2 + u3;
	v[3] = -u2 + u3;
#else
	float s = 1.0f-u;
	v[0] = s*s*s;
	v[1] = 3.0f*u*s*s;
	v[2] = 3.0f*u*u*s;
	v[3] = u*u*u;
#endif
	}

static void ComputeTCBMults(float tens, float cont, float &m1, float &m2)
	{ 	
	float tm,cm,cp;	
	tm = 0.5f*(1.0f - tens);
	cm = 1.0f - cont;       
	cp = 2.0f - cm;
	m1 = tm*cm;
	m2 = tm*cp;
	}

static Point3 InterpSpline(float u,Point3 knot[4],float m1, float m2)
	{
	float v[4];
	Point3 p;
	ComputeHermiteBasis(u,v);
#if 1
	float c[4];
	c[0] = -v[2]*m2;
	c[1] = v[0] + v[2]*(m2-m1) - v[3]*m1;
	c[2] = v[1] + v[2]*m1 + v[3]*(m1-m2);
	c[3] = v[3]*m2;
	p =  knot[0]*c[0] + knot[1]*c[1] + knot[2]*c[2] + knot[3]*c[3];
#else	
	Point3 R1, R4;
	R1 = 3.0f*(knot[1] - knot[0]);
	R4 = 3.0f*(knot[3] - knot[2]); 
	p = knot[0]*v[0] + knot[3]*v[1] + R1*v[2] + R4*v[3];
#endif

#if 0
	DebugPrint ("u:%.2f [%.2f, %.2f, %.2f] #([%.2f, %.2f, %.2f], [%.2f, %.2f, %.2f], [%.2f, %.2f, %.2f], [%.2f, %.2f, %.1f])\n",
			 u, p.x, p.y, p.x, knot[0].x, knot[0].y, knot[0].z, knot[1].x, knot[1].y, knot[1].z, 
				knot[2].x, knot[2].y, knot[2].z, knot[3].x, knot[3].y, knot[3].z);
#endif	
	return p;
	}


//-----------------------------------------------------------------------
//
// FFD select modifier
//
//

class FFDSelData;

class FFDSelMod : public Modifier { 	
	public:				
		DWORD selLevel;
		
		static IObjParam *ip;		
		static FFDSelMod *editMod;		
		static HWND hParams;
		static SelectModBoxCMode *selectMode;
		static BOOL allX, allY, allZ;

		FFDSelMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s = GetString(IDS_RB_FFDSELMOD); }  
		virtual Class_ID ClassID() { return FFD_SELECT_CLASS_ID; }		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_FFDSELMOD); }

		// From modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE; }
		ChannelMask ChannelsChanged() { return PART_SELECT|PART_SUBSEL_TYPE; }
		Class_ID InputType() { return GENFFDOBJECT_CLASS_ID; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t) { return FOREVER; }

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		void ActivateSubobjSel(int level, XFormModes& modes);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);		
		IOResult SaveLocalData(ISave *isave, LocalModData *ld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) { }

		int NumSubs() { return 0; }
		Animatable* SubAnim(int i) { return NULL; }
		TSTR SubAnimName(int i) { return _T(""); }

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message) { return REF_SUCCEED; }

		void SetButtonStates();
		void ExpandSelection(int ix, BOOL on,FFDSelData *d);

	};

class FFDSelData : public LocalModData {
	public:		
		BitArray sel;
		Box3 lbox;

		BOOL held;
		Tab<Point3> pt;
		int dim[3];

		FFDSelData(FFDNM<WSMObject> *ffd);
		FFDSelData() { held=0;lbox.Init(); }
		LocalModData *Clone();		
		void SetCache(FFDNM<WSMObject> *ffd);
		void FreeCache();
		int GridIndex(int i, int j, int k);
	};

class FFDSelRestore : public RestoreObj {
	public:
		BitArray usel, rsel;		
		FFDSelMod *mod;
		FFDSelData *d;
		int level;

		FFDSelRestore(FFDSelMod *m, FFDSelData *d);
		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() { d->held=FALSE; }
		TSTR Description() { return TSTR(_T("SelectRestore")); }
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam			*FFDSelMod::ip         = NULL;
FFDSelMod			*FFDSelMod::editMod    = NULL;
HWND				 FFDSelMod::hParams    = NULL;
SelectModBoxCMode	*FFDSelMod::selectMode = SEL_OBJECT;
BOOL                 FFDSelMod::allX       = FALSE;
BOOL                 FFDSelMod::allY       = FALSE;
BOOL                 FFDSelMod::allZ       = FALSE;

class FFDSelClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FFDSelMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FFDSELMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return FFD_SELECT_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS); }
	};

static FFDSelClassDesc ffdSelDesc;
ClassDesc* GetFFDSelModDesc() { return &ffdSelDesc; }

static INT_PTR CALLBACK FFDSelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--- FFDSelMod methods -----------------------------------------------------

FFDSelMod::FFDSelMod()
	{
	selLevel = SEL_POINTS;
	}

RefTargetHandle FFDSelMod::Clone(RemapDir& remap)
	{
	FFDSelMod *newMod = new FFDSelMod();
	newMod->selLevel = selLevel;
	BaseClone(this, newMod, remap);
	return newMod;
	}

void FFDSelMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	if (os->obj->IsSubClassOf(GENFFDOBJECT_CLASS_ID)) {
		FFDNM<WSMObject> *ffd = (FFDNM<WSMObject>*)os->obj;
		FFDSelData *d  = (FFDSelData*)mc.localData;
		if (!d) {
			mc.localData = d = new FFDSelData(ffd);
			}
		if (editMod==this) {
			if (d->pt.Count()!=ffd->pt.Count()) d->SetCache(ffd);
			}
		d->sel.SetSize(ffd->sel.GetSize(),TRUE);
		ffd->sel = d->sel;
		ffd->selLevel = selLevel;
		}
	}

void FFDSelMod::NotifyInputChanged(
		Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
	{
	if (mc->localData && ip) {
		((FFDSelData*)mc->localData)->FreeCache();
		TimeValue t = ip->GetTime();
		NotifyDependents(Interval(t,t), 
			PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE,
		    REFMSG_MOD_EVAL);
		}
	}

void FFDSelMod::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;	
	editMod  = this;
	selectMode = new SelectModBoxCMode(this,ip);

	// Add our sub object type
	// TSTR type1(GetString(IDS_RB_CONTPOINTS));
	// const TCHAR *ptype[] = { type1};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 1);	

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	// Disable show end result.
	ip->EnableShowEndResult(FALSE);	

	hParams = ip->AddRollupPage( 
		hInstance, 
		MAKEINTRESOURCE(IDD_FFDSELMODPARAMS),
		FFDSelProc,
		GetString(IDS_RB_FFDSELMOD),
		(LPARAM)this);	
	}

void FFDSelMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	for (int i=0; i<list.Count(); i++) {
		((FFDSelData*)list[i]->localData)->FreeCache();
		}
	nodes.DisposeTemporary();

	if (hParams) ip->DeleteRollupPage(hParams);
	hParams = NULL;	

	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	// Enable show end result
	ip->EnableShowEndResult(TRUE);

	this->ip = NULL;
	editMod  = NULL;
	hParams  = NULL;
	}

int FFDSelMod::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{ 		
	FFDSelData *d = (FFDSelData *)mc->localData;
	if (!d) return 0;

	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	// Setup GW
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
	gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();	

	Point3 s = LatticeSize(d->lbox);
	for (int i=0; i<d->pt.Count(); i++) {
		// Maybe skip sel or unsel points
		if (flags&HIT_SELONLY   && !d->sel[i]) continue;
		if (flags&HIT_UNSELONLY &&  d->sel[i]) continue;
		
		// Draw the point
		Point3 p = d->pt[i]*s + d->lbox.Min();
		gw->marker(&p,HOLLOW_BOX_MRKR);		
		
		// Check for a hit and log it
		if (gw->checkHitCode()) { 			
			gw->clearHitCode();
			vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL);
			res = 1;
			if (flags&HIT_ABORTONHIT) return res;			
			}
		}

	gw->setRndLimits(savedLimits);	
	return res;
	}

void FFDSelMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	selLevel = level;
	if (level!=SEL_OBJECT) {
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
		}
	
	NotifyDependents(FOREVER, SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);	
	}

void FFDSelMod::ExpandSelection(int ix, BOOL on,FFDSelData *d)
	{
	int j, k, i;	
	if (d->dim[0]==0 || d->dim[1]==0) return;	

	k   = ix/(d->dim[0]*d->dim[1]);
	ix -= k*(d->dim[0]*d->dim[1]);
	j   = ix/d->dim[0];
	ix -= j*d->dim[0];
	i   = ix;

	if (allX) {
		for (int ix=0; ix<d->dim[0]; ix++) {
			if (allY) {
				for (int jx=0; jx<d->dim[1]; jx++) {
					int index = d->GridIndex(ix,jx,k);			
					d->sel.Set(index,on);
					}
				}
			if (allZ) {
				for (int kx=0; kx<d->dim[2]; kx++) {
					int index = d->GridIndex(ix,j,kx);			
					d->sel.Set(index,on);
					}
				}
			int index = d->GridIndex(ix,j,k);			
			d->sel.Set(index,on);
			}
		}
	if (allY) {
		for (int jx=0; jx<d->dim[1]; jx++) {
			if (allX) {
				for (int ix=0; ix<d->dim[0]; ix++) {
					int index = d->GridIndex(ix,jx,k);			
					d->sel.Set(index,on);
					}
				}
			if (allZ) {
				for (int kx=0; kx<d->dim[2]; kx++) {
					int index = d->GridIndex(ix,j,kx);			
					d->sel.Set(index,on);
					}
				}
			int index = d->GridIndex(i,jx,k);
			d->sel.Set(index,on);
			}
		}
	if (allZ) {
		for (int kx=0; kx<d->dim[2]; kx++) {
			if (allX) {
				for (int ix=0; ix<d->dim[0]; ix++) {
					int index = d->GridIndex(ix,j,kx);			
					d->sel.Set(index,on);
					}
				}
			if (allY) {
				for (int jx=0; jx<d->dim[1]; jx++) {
					int index = d->GridIndex(i,jx,kx);			
					d->sel.Set(index,on);
					}
				}
			int index = d->GridIndex(i,j,kx);
			d->sel.Set(index,on);
			}
		}
	}

void FFDSelMod::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	FFDSelData *d;
	while (hitRec) {
		d = (FFDSelData*)hitRec->modContext->localData;
		if (theHold.Holding() && !d->held) theHold.Put(new FFDSelRestore(this,d));
		BOOL state = selected;
		if (invert) state = !d->sel[hitRec->hitInfo];
		if (allX || allY || allZ) {
			ExpandSelection(hitRec->hitInfo,state,d);
		} else {
			if (state) d->sel.Set(hitRec->hitInfo);
			else       d->sel.Clear(hitRec->hitInfo);
			}
		if (!all) break;
		hitRec = hitRec->Next();
		}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDSelMod::ClearSelection(int selLevel)
	{
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	FFDSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (FFDSelData*)list[i]->localData;
		if (theHold.Holding() && !d->held) theHold.Put(new FFDSelRestore(this,d));
		d->sel.ClearAll();		
		}	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDSelMod::SelectAll(int selLevel)
	{
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	FFDSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (FFDSelData*)list[i]->localData;
		if (theHold.Holding() && !d->held) theHold.Put(new FFDSelRestore(this,d));
		d->sel.SetAll();		
		}	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDSelMod::InvertSelection(int selLevel)
	{
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	FFDSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (FFDSelData*)list[i]->localData;
		if (theHold.Holding() && !d->held) theHold.Put(new FFDSelRestore(this,d));
		d->sel = ~d->sel;
		}	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

#define SELLEVEL_CHUNKID		0x0100
#define SEL_CHUNKID				0x0200

IOResult FFDSelMod::Save(ISave *isave)
	{
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk(SELLEVEL_CHUNKID);
	res = isave->Write(&selLevel, sizeof(selLevel), &nb);
	isave->EndChunk();
	return res;
	}

IOResult FFDSelMod::Load(ILoad *iload)
	{
	IOResult res;
	ULONG nb;
	Modifier::Load(iload);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SELLEVEL_CHUNKID:
				iload->Read(&selLevel, sizeof(selLevel), &nb);
				break;			
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

IOResult FFDSelMod::SaveLocalData(ISave *isave, LocalModData *ld)
	{
	FFDSelData *d = (FFDSelData*)ld;

	isave->BeginChunk(SEL_CHUNKID);
	d->sel.Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

IOResult FFDSelMod::LoadLocalData(ILoad *iload, LocalModData **pld)
	{
	FFDSelData *d = new FFDSelData;
	*pld = d;
	IOResult res;	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SEL_CHUNKID:
				d->sel.Load(iload);
				break;			
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


void FFDSelMod::SetButtonStates()
	{
	ICustButton *but;
	but = GetICustButton(GetDlgItem(hParams,IDC_FFD_ALLX));
	but->SetType(CBT_CHECK);
	but->SetCheck(allX);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hParams,IDC_FFD_ALLY));
	but->SetType(CBT_CHECK);
	but->SetCheck(allY);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hParams,IDC_FFD_ALLZ));
	but->SetType(CBT_CHECK);
	but->SetCheck(allZ);
	ReleaseICustButton(but);
	}


int FFDSelMod::NumSubObjTypes() 
{ 
	return 1;
}


ISubObjType *FFDSelMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_ContPoints.SetName(GetString(IDS_RB_CONTPOINTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_ContPoints;
	}
	return NULL;
}


static INT_PTR CALLBACK FFDSelProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	FFDSelMod *mod = (FFDSelMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG: {
			mod = (FFDSelMod*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mod->hParams = hWnd;
			mod->SetButtonStates();
			break;
			}
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_FFD_ALLX: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLX));
					mod->allX = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}
				case IDC_FFD_ALLY: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLY));
					mod->allY = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}
				case IDC_FFD_ALLZ: {
					ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_FFD_ALLZ));
					mod->allZ = but->IsChecked();
					ReleaseICustButton(but);
					break;
					}				
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			mod->ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			break;
		default: return FALSE;
		}
	return TRUE;
	}


//--- FFDSelData ------------------------------------------------------

FFDSelData::FFDSelData(FFDNM<WSMObject> *ffd)
	{
	held = 0;
	lbox = ffd->lbox;
	sel  = ffd->sel;
	}

LocalModData *FFDSelData::Clone()
	{
	FFDSelData *d = new FFDSelData();
	d->sel = sel;
	return d;
	}

void FFDSelData::SetCache(FFDNM<WSMObject> *ffd)
	{ 	
	pt   = ffd->pt;	
	lbox = ffd->lbox;
	for (int i=0; i<3; i++) dim[i] = ffd->dim[i];
	}

void FFDSelData::FreeCache()
	{ 	
	pt.Resize(0);
	}

int FFDSelData::GridIndex(int i, int j, int k)
	{
	int ix = k*dim[0]*dim[1] + j*dim[0] + i;
	assert(ix>=0 && pt.Count());
	return ix;
	}

//--- FFDSelRestore -----------------------------------------------------

FFDSelRestore::FFDSelRestore(FFDSelMod *m, FFDSelData *d)
	{
	mod     = m;
	level   = mod->selLevel;
	this->d = d;
	d->held = TRUE;
	usel    = d->sel;
	}

void FFDSelRestore::Restore(int isUndo)
	{
	if (isUndo) {
		rsel = d->sel;
		}
	d->sel = usel;
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDSelRestore::Redo()
	{
	d->sel = rsel;
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}
