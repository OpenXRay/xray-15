/**********************************************************************
 *<
	FILE: ffd.cpp

	DESCRIPTION: A FFD Modifier

	CREATED BY: Rolf Berteig, 3.0 additions by Ravi Karra 

	HISTORY: 7/22/96

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
#include "ffdui.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "macrorec.h"
#include "MaxIcon.h"
#include "resource.h"
#include "setkeymode.h"

// Compute the linear address of a control point from a
// set of 3D indices. Assumes a 4x4x4 grid.
#define GRIDINDEX44(i,j,k) (((i)<<4) + ((j)<<2) + (k))
#define GRIDINDEX33(i,j,k) (((i)*9) + ((j)*3) + (k))
#define GRIDINDEX22(i,j,k) (((i)<<2) + ((j)<<1) + (k))

Point3 LatticeSize(Box3 box);
void MakeBoxThick(Box3 &box);

static GenSubObjType SOT_ContPoints(27);
static GenSubObjType SOT_Lattice(28);
static GenSubObjType SOT_Volume(29);


//--- FFD Modifier -------------------------------------------------

class FFDMod : public IFFDMod<Modifier> {	
	public:
		MasterPointControl *masterCont;		// Master track controller		
		// An addition transformation from "Mod Space".
		// Let's the user move/rotate/scale the source box
		Control	*tmControl;
		
		// Parameter block to store parameters
		IParamBlock2 *pblock;		
		
		// This BitArray will be set to a length of 64. 1 bit for
		// each point indicating its selection state.
		BitArray sel;
		
		// A cache of the input object's bounding box.
		Box3 lbox;
		int	 selLevel;
		int loadRefVersion;

		FFDRightMenu<FFDMod >	*ffdMenu;			// Right-click menu handler		
		FFDActionCB<FFDMod >	*ffdActionCB;		// Actions handler 		
		

		// Class variables -- these are only used by one instance
		// of this class at a time while it is being edited in
		// the command panel.
		static IObjParam			*ip;		
		static MoveModBoxCMode		*moveMode;
		static RotateModBoxCMode	*rotMode;
		static UScaleModBoxCMode	*uscaleMode;
		static NUScaleModBoxCMode	*nuscaleMode;
		static SquashModBoxCMode	*squashMode;
		static SelectModBoxCMode	*selectMode;

		FFDMod();
		~FFDMod();
		
		// From Animatable
		void DeleteThis() {delete this;}		
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);				
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		BOOL AssignController(Animatable *control,int subAnim);		
		int SubNumToRefNum(int subNum);
		BOOL SelectSubAnim(int subNum);
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		// From BaseObject/Object
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		void ActivateSubobjSel(int level, XFormModes& modes);
		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
		void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void TransformStart(TimeValue t) {if (ip) ip->LockAxisTripods(TRUE);}
		void TransformFinish(TimeValue t) {if (ip) ip->LockAxisTripods(FALSE);}
		void TransformCancel(TimeValue t) {if (ip) ip->LockAxisTripods(FALSE);}

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// From Modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		ChannelMask ChannelsChanged() {return PART_GEOM;}
		Class_ID InputType() {return defObjectClassID;}		
		Interval LocalValidity(TimeValue t);
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From ReferenceTarget/Maker
		int NumRefs() {return 67;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int NumSubs() {return 3;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		int		 RemapRefOnLoad(int iref);
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
						
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		int DrawControlPoints(TimeValue t,ViewExp *vpt,GraphicsWindow *gw,Box3 box,BOOL ht=FALSE,INode *inode=NULL,ModContext *mc=NULL,int flags=0);		
		Matrix3 CompTM(TimeValue t,INode *inode,ModContext *mc);
		void PlugControllers(TimeValue t, BOOL all=FALSE);
		
		
		virtual Point3& getPt(int i)=0;
		virtual Point3& getOffset(int i)=0;
		virtual Point3 *GetPtPtr()=0;
		virtual Point3 *GetOffsetPtr()=0;
		virtual int GridWidth()=0;
		virtual int GridIndex(int i,int j,int k)=0;
		virtual Point3 GetControlPoint(TimeValue t, int i, int src=FALSE, BOOL initVol=FALSE)=0;
		virtual	ClassDesc2* GetClassDesc()=0;

		// From IFFDMod
		Point3 GetPt(int i) { return getPt(i); }
		void SetPt(int i, Point3 p) { getPt(i) = p; }
		void Reset();
		void Conform();
		void AnimateAll();
		void SetGridDim(IPoint3 d) { Reset(); }
		};


class FFDMod44 : public FFDMod {
	public:
		Control *ptCont[64];		
		Point3  pt[64];
		Point3	offsets[64];

		FFDMod44();
		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_FFD44);}
		Class_ID ClassID() {return FFD44_CLASS_ID;}
		TCHAR *GetObjectName() {return GetString(IDS_RB_FFD44);}				

		int NumPts() {return 64;}
		int	NumPtConts() { return NumPts(); }
		IPoint3	GetGridDim() { return IPoint3(4,4,4); }
		Control* GetPtCont(int i) {return ptCont[i];}
		void SetPtCont(int i,Control *c);
		Point3& getPt(int i) {return pt[i];}
		Point3& getOffset(int i)  {return offsets[i];}
		Point3 * GetPtPtr() {return pt;}
		Point3 * GetOffsetPtr() {return offsets;}
		int GridWidth() {return 4;}
		int GridIndex(int i,int j,int k) {return GRIDINDEX44(i,j,k);}		
		Point3 GetControlPoint(TimeValue t, int i, int src=FALSE, BOOL initVol=FALSE);
		ClassDesc2* GetClassDesc();
	};

class FFDMod33 : public FFDMod {
	public:
		Control *ptCont[27];		
		Point3  pt[27];
		Point3	offsets[27];

		FFDMod33();

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_FFD33);}
		Class_ID ClassID() {return FFD33_CLASS_ID;}
		TCHAR *GetObjectName() {return GetString(IDS_RB_FFD33);}		

		int NumPts() {return 27;}
		int	NumPtConts() { return NumPts(); }
		IPoint3	GetGridDim() { return IPoint3(3,3,3); }
		Control* GetPtCont(int i) {return ptCont[i];}
		void SetPtCont(int i,Control *c);
		Point3& getPt(int i) {return pt[i];}
		Point3& getOffset(int i)  {return offsets[i];}
		Point3 * GetPtPtr() {return pt;}
		Point3 * GetOffsetPtr() {return offsets;}
		int GridWidth() {return 3;}
		int GridIndex(int i,int j,int k) {return GRIDINDEX33(i,j,k);}
		Point3 GetControlPoint(TimeValue t, int i, int src=FALSE, BOOL initVol=FALSE);
		ClassDesc2* GetClassDesc();
	};

class FFDMod22 : public FFDMod {
	public:
		Control *ptCont[8];		
		Point3  pt[8];
		Point3	offsets[8];

		FFDMod22();

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_FFD22);}
		Class_ID ClassID() {return FFD22_CLASS_ID;}
		TCHAR *GetObjectName() {return GetString(IDS_RB_FFD22);}

		int NumPts() {return 8;}
		int	NumPtConts() { return NumPts(); }
		IPoint3	GetGridDim() { return IPoint3(2,2,2); }
		Control* GetPtCont(int i) {return ptCont[i];}
		void SetPtCont(int i,Control *c);
		Point3& getPt(int i) {return pt[i];}
		Point3& getOffset(int i)  {return offsets[i];}
		Point3 * GetPtPtr() {return pt;}
		Point3 * GetOffsetPtr() {return offsets;}
		int GridWidth() {return 2;}
		int GridIndex(int i,int j,int k) {return GRIDINDEX22(i,j,k);}
		Point3 GetControlPoint(TimeValue t, int i, int src=FALSE, BOOL initVol=FALSE);
		ClassDesc2* GetClassDesc();
	};

//--- Class Descriptor and Class Vars. ------------------------------------------

IObjParam			*FFDMod::ip = NULL;
MoveModBoxCMode		*FFDMod::moveMode = NULL;
RotateModBoxCMode	*FFDMod::rotMode = NULL;
UScaleModBoxCMode	*FFDMod::uscaleMode = NULL;
NUScaleModBoxCMode	*FFDMod::nuscaleMode = NULL;
SquashModBoxCMode	*FFDMod::squashMode = NULL;
SelectModBoxCMode	*FFDMod::selectMode = NULL;

// The FFD Class Descriptor
class FFDClassDesc44: public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new FFDMod44;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_FFD44);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return FFD44_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	const TCHAR*	InternalName() { return _T("FFD4x4x4"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	};
class FFDClassDesc33: public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new FFDMod33;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_FFD33);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return FFD33_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	const TCHAR*	InternalName() { return _T("FFD3x3x3"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	};
class FFDClassDesc22: public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new FFDMod22;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_FFD22);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return FFD22_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	const TCHAR*	InternalName() { return _T("FFD2x2x2"); }
	HINSTANCE		HInstance()	{ return hInstance; }
	};

// We'll just declare one instace of the descriptor.
static FFDClassDesc44 ffdDesc44;
static FFDClassDesc33 ffdDesc33;
static FFDClassDesc22 ffdDesc22;
ClassDesc* GetFFDDesc44() {return &ffdDesc44;}
ClassDesc* GetFFDDesc33() {return &ffdDesc33;}
ClassDesc* GetFFDDesc22() {return &ffdDesc22;}
ClassDesc2* FFDMod44::GetClassDesc() { return &ffdDesc44; }
ClassDesc2* FFDMod33::GetClassDesc() { return &ffdDesc33; }
ClassDesc2* FFDMod22::GetClassDesc() { return &ffdDesc22; }

//--- RestoreObjects for undo/redo --------------------------------

// A restore object to save the selection state.
class SelRestore : public RestoreObj {
	public:		
		FFDMod *mod;
		BitArray undo,redo;
		SelRestore(FFDMod *m) {mod=m;undo=mod->sel;}
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
class MoveRestore : public RestoreObj {
	public:		
		FFDMod *mod;
		Point3 undo[64], redo[64], undoOffs[64], redoOffs[64];
		MoveRestore(FFDMod *m) {
			mod = m;
			for (int i=0; i<mod->NumPts(); i++) {
				undo[i] = mod->getPt(i);
				undoOffs[i] = mod->getOffset(i);
				}
			}
		void Restore(int isUndo) {
			// if we're undoing, save a redo state
			if (isUndo) {
				for (int i=0; i<mod->NumPts(); i++) {
					redo[i] = mod->getPt(i);
					redoOffs[i] = mod->getOffset(i);
					}
				}
			for (int i=0; i<mod->NumPts(); i++) {
				mod->getPt(i) = undo[i];
				mod->getOffset(i) = undoOffs[i];
				}
			mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
			}
		void Redo() {
			for (int i=0; i<mod->NumPts(); i++) {
				mod->getPt(i) = redo[i];
				mod->getOffset(i) = redoOffs[i];
				}
			mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
			}
		void EndHold() {
			mod->ClearAFlag(A_HELD);
			}
	};

//--- Parameter map/block descriptors -------------------------------
enum { ffd_params };
enum { ffd_disp_lattice, ffd_disp_source, ffd_deform, ffd_inpoints, ffd_outpoints, ffd_offset};


static ParamBlockDesc2 ffd44_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdDesc44, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_FFDPARAMS, IDS_RB_PARAMETERS, 0, 0, NULL,
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
		p_ui, 			TYPE_RADIO, 	2,		IDC_FFD_INVOLUME,				IDC_FFD_DEFORMALL,  
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

static ParamBlockDesc2 ffd33_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdDesc33, P_AUTO_CONSTRUCT + P_USE_PARAMS, PBLOCK_REF,
	// use params from existing descriptor
	&ffd44_param_blk
	);

static ParamBlockDesc2 ffd22_param_blk ( ffd_params, _T("ffdparameters"),  0, &ffdDesc22, P_AUTO_CONSTRUCT + P_USE_PARAMS, PBLOCK_REF,
	// use params from existing descriptor
	&ffd44_param_blk
	);

// These are the parameters stored in our parameter block
#define ffd_disp_lattice	0	// Is lattice display turned on
#define ffd_disp_source	1 	// Is display source turned on
#define ffd_deform		2	// Is the deform only in volume option selected

//
//
// Parameters

// The parameter block. Note that none of these parameters are animatable.
static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 } };

// Arrays of old versions
static ParamVersionDesc versionsMod[] = {
	ParamVersionDesc(descVer0,3,0),
};

#define NUM_OLDVERSIONS	1

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


// This callback will let us do additional processing
// for our UI. The parameter map will handle the parameters,
// we just need to handle the about box.

class FFDDlgProc : public ParamMap2UserDlgProc {
	public:
		FFDMod	*ffd;
		int		dlgID;
		FFDDlgProc() {dlgID=IDD_FFD_ABOUT44;}
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};

// This class doesn't have any state, so we'll just declare one
// static instance.
static FFDDlgProc theFFDProc;


// This will get called everytime the rollup page's window proc
// is called.
// The only message we're interested in is the about button.
BOOL FFDDlgProc::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
#if defined( DESIGN_VER )
		case WM_INITDIALOG:
			ShowWindow(GetDlgItem(hWnd, IDC_FFD_ANIMATEALL), SW_HIDE);
		break;
#endif // DESIGN_VER
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_FFD_RESET:
					macroRecorder->FunctionCall(_T("resetLattice"), 1, 0, mr_reftarg, ffd, mr_funcall);
					theHold.Begin();
					ffd->Reset();
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
						MAKEINTRESOURCE(dlgID),
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


//--- FFD Deformer --------------------------------------------------
//
// The deformer will actually modify the points of the input object.
// Note that the Map() function must be thread safe. In this
// case it is not a problem since the only variables modified by
// the Map() function are on the stack.
 
class FFDDeformer : public Deformer {
	public:
		// Lattice points
		Point3 pt[64];
		
		// These transformations will take a point from object
		// space into lattice space and back.
		Matrix3 tm, itm;
		
		// If TRUE, only deform points in the source volume.
		int inVol;

		FFDDeformer(FFDMod *m,TimeValue t,ModContext *mc);		
	};

class FFDDeformer44 : public FFDDeformer {
	public:
		FFDDeformer44(FFDMod *m,TimeValue t,ModContext *mc) :
			FFDDeformer(m,t,mc) {}
		Point3 Map(int ii, Point3 p);
	};
class FFDDeformer33 : public FFDDeformer {
	public:
		FFDDeformer33(FFDMod *m,TimeValue t,ModContext *mc) :
			FFDDeformer(m,t,mc) {}
		Point3 Map(int ii, Point3 p);
	};
class FFDDeformer22 : public FFDDeformer {
	public:
		FFDDeformer22(FFDMod *m,TimeValue t,ModContext *mc) :
			FFDDeformer(m,t,mc) {}
		Point3 Map(int ii, Point3 p);
	};


FFDDeformer::FFDDeformer(FFDMod *mod,TimeValue t,ModContext *mc)
	{	
	// Copy the lattice into our buffer.
	for (int i=0; i<mod->NumPts(); i++) {
		pt[i] = mod->GetControlPoint(t,i) - mod->getOffset(i);
		}

	// Get the state of the deform all option
	inVol = !mod->pblock->GetInt(ffd_deform,t);	

	// Evaluate the TM controller
	Matrix3 ctm(1);
	mod->tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
	
	// Get the ModContext TM (if there is one)
	tm  = mc->tm ? *mc->tm : Matrix3(1);
	
	// Apply our TM to the MC TM
	tm *= Inverse(ctm);
	
	// The origin of the TM is the lower left corner of the
	// box, not the center.
	tm.SetTrans(tm.GetTrans()-mc->box->Min());
	
	// Compute scale factors to normalize lattice space
	// to the extents of the box.
	Point3 s = LatticeSize(*mc->box);	

	for (i=0; i<3; i++) {
		if (s[i]==0.0f) s[i] = 1.0f;
		else s[i] = 1.0f/s[i];
		}
	tm.Scale(s,TRUE);

	// Compute the inverse.
	itm = Inverse(tm);
	}

// Cubic basis function.
// Note that this could be optimized by precomputing u^2, u^3, s^2, s^3
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
inline float BPoly3(int i, float u)
	{
	float s = 1.0f-u;
	switch (i) {
		case 0: return s*s;
		case 1: return 2.0f*u*s;
		case 2: return u*u;
		default: return 0.0f;
		}
	}
inline float BPoly2(int i, float u)
	{	
	switch (i) {
		case 0: return 1.0f-u;
		case 1: return u;
		default: return 0.0f;
		}
	}

#define EPSILON	0.001f

// This is the function that computes the deformed points.
Point3 FFDDeformer44::Map(int ii, Point3 p)
	{
	Point3 q(0,0,0), pp;
	
	// Transform into lattice space
	pp = p*tm;

	// maybe skip the point if it is outside the source volume.
	if (inVol) {
		for (int i=0; i<3; i++) {
			if (pp[i]<-EPSILON || pp[i]>1.0f+EPSILON) return p;
			}
		}

	// Compute the deformed point as a weighted average of all
	// 64 control points.
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			for (int k=0; k<4; k++) {
				q += pt[GRIDINDEX44(i,j,k)]*
					BPoly4(i,pp.x)*
					BPoly4(j,pp.y)*
					BPoly4(k,pp.z);
				}
			}
		}
	
	// Transform out of lattice space back into object space.
	return q*itm;
	}

Point3 FFDDeformer33::Map(int ii, Point3 p)
	{
	Point3 q(0,0,0), pp;
	
	// Transform into lattice space
	pp = p*tm;

	// maybe skip the point if it is outside the source volume.
	if (inVol) {
		for (int i=0; i<3; i++) {
			if (pp[i]<-EPSILON || pp[i]>1.0f+EPSILON) return p;
			}
		}

	// Compute the deformed point as a weighted average of all
	// 64 control points.
	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			for (int k=0; k<3; k++) {
				q += pt[GRIDINDEX33(i,j,k)]*
					BPoly3(i,pp.x)*
					BPoly3(j,pp.y)*
					BPoly3(k,pp.z);
				}
			}
		}
	
	// Transform out of lattice space back into object space.
	return q*itm;
	}

Point3 FFDDeformer22::Map(int ii, Point3 p)
	{
	Point3 q(0,0,0), pp;
	
	// Transform into lattice space
	pp = p*tm;

	// maybe skip the point if it is outside the source volume.
	if (inVol) {
		for (int i=0; i<3; i++) {
			if (pp[i]<-EPSILON || pp[i]>1.0f+EPSILON) return p;
			}
		}

	// Compute the deformed point as a weighted average of all
	// 64 control points.
	for (int i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			for (int k=0; k<2; k++) {
				q += pt[GRIDINDEX22(i,j,k)]*
					BPoly2(i,pp.x)*
					BPoly2(j,pp.y)*
					BPoly2(k,pp.z);
				}
			}
		}
	
	// Transform out of lattice space back into object space.
	return q*itm;
	}

//--- FFD Modifier Methods -----------------------------------------

FFDMod::FFDMod()
	{
	// Create a new matrix controller to controller the lattice TM
	tmControl = NULL;
	MakeRefByID(FOREVER,TM_REF,NewDefaultMatrix3Controller()); 
	
	// Create the ffd master control
	masterCont = NULL;		
	MakeRefByID(FOREVER, MASTER_REF, NewDefaultMasterPointController());	
	}

FFDMod44::FFDMod44()
	{
	ffd44_param_blk.dlg_template = IDD_FFDPARAMS;
	ffd44_param_blk.title = IDS_RB_PARAMETERS;
	GetClassDesc()->MakeAutoParamBlocks(this);
	// Init all the control point controllers to NULL;
	for (int i=0; i<64; i++) {
		ptCont[i] = NULL;		
		}

	// Init the lattice grid
	for (i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			for (int k=0; k<4; k++) {
				pt[GRIDINDEX44(i,j,k)] = Point3(
					float(i)/3.0f,
					float(j)/3.0f,
					float(k)/3.0f);
				offsets[GRIDINDEX44(i,j,k)] = Point3(0.0, 0.0, 0.0);
				}
			}
		}
	
	// Set the selection set size
	sel.SetSize(64);
	}

FFDMod33::FFDMod33()
	{
	ffd33_param_blk.dlg_template = IDD_FFDPARAMS;
	ffd33_param_blk.title = IDS_RB_PARAMETERS;
	GetClassDesc()->MakeAutoParamBlocks(this);
	// Init all the control point controllers to NULL;
	for (int i=0; i<27; i++) {
		ptCont[i] = NULL;		
		}

	// Init the lattice grid
	for (i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			for (int k=0; k<3; k++) {
				pt[GRIDINDEX33(i,j,k)] = Point3(
					float(i)/2.0f,
					float(j)/2.0f,
					float(k)/2.0f);
				offsets[GRIDINDEX33(i,j,k)] = Point3(0.0, 0.0, 0.0);
				}
			}
		}
	
	// Set the selection set size
	sel.SetSize(27);
	}

FFDMod22::FFDMod22()
	{
	ffd22_param_blk.dlg_template = IDD_FFDPARAMS;
	ffd22_param_blk.title = IDS_RB_PARAMETERS;
	GetClassDesc()->MakeAutoParamBlocks(this);
	// Init all the control point controllers to NULL;
	for (int i=0; i<8; i++) {
		ptCont[i] = NULL;		
		}

	// Init the lattice grid
	for (i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			for (int k=0; k<2; k++) {
				pt[GRIDINDEX22(i,j,k)] = Point3(
					float(i),
					float(j),
					float(k));
				offsets[GRIDINDEX22(i,j,k)] = Point3(0.0, 0.0, 0.0);
				}
			}
		}
	
	// Set the selection set size
	sel.SetSize(8);
	}

FFDMod::~FFDMod()
	{
	}

RefTargetHandle FFDMod::GetReference(int i)
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

void FFDMod::SetReference(int i, RefTargetHandle rtarg)
	{
	if (i==0) 
		tmControl = (Control*)rtarg;
	else if (i==1) {
		pblock = (IParamBlock2*)rtarg;
	} else if (i==2) {
		masterCont = (MasterPointControl*)rtarg;
		if (masterCont)
			masterCont->SetNumSubControllers(64);
	} else if (i<NumPts()+3) {
		SetPtCont(i-3,(Control*)rtarg);
	} else 
		assert(0);
	}

Animatable* FFDMod::SubAnim(int i)
	{
	return GetReference(i);
	}

TSTR FFDMod::SubAnimName(int i)
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

int FFDMod::SubNumToRefNum(int subNum)
	{
	if (subNum==1) return -1;
	else return subNum;
	}

BOOL FFDMod::AssignController(Animatable *control,int subAnim)
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

void FFDMod::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;

	// Activate the right-click menu callback
	ffdMenu = new FFDRightMenu<FFDMod >(this);
	ip->GetRightClickMenuManager()->Register(ffdMenu);
	
	// Set up keyboard actions    
	ffdActionCB = new FFDActionCB<FFDMod >(this);
    ip->GetActionManager()->ActivateActionTable(ffdActionCB, kFFDActions);
	
	// Creates some modes
	moveMode       = new MoveModBoxCMode(this,ip);
	rotMode        = new RotateModBoxCMode(this,ip);
	uscaleMode     = new UScaleModBoxCMode(this,ip);
	nuscaleMode    = new NUScaleModBoxCMode(this,ip);
	squashMode     = new SquashModBoxCMode(this,ip);
	selectMode     = new SelectModBoxCMode(this,ip);

	// Add our sub object type
	// TSTR type1(GetString(IDS_RB_CONTPOINTS));	
	// TSTR type2(GetString(IDS_RB_LATTICE));	
	// TSTR type3(GetString(IDS_RK_SETVOLUME));
	// const TCHAR *ptype[] = {type1,type2, type3};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 3);

	// Notify the system that we have an apparatus to display
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	// Create a parameter map to handle UI
	theFFDProc.ffd = this;
	switch (NumPts()) {		
		case 64: theFFDProc.dlgID = IDD_FFD_ABOUT44; break;
		case 27: theFFDProc.dlgID = IDD_FFD_ABOUT33; break;
		case  8: theFFDProc.dlgID = IDD_FFD_ABOUT22; break;
		}
	ParamBlockDesc2* pbd = GetClassDesc()->GetParamBlockDesc(0);
	pbd->flags |= P_AUTO_UI;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
	pbd->SetUserDlgProc(&theFFDProc);

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

	}

void FFDMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{	
	ip->GetRightClickMenuManager()->Unregister(ffdMenu);
	delete ffdMenu;
	ip->GetActionManager()->DeactivateActionTable(ffdActionCB, kFFDActions);
	delete ffdActionCB;

	GetClassDesc()->EndEditParams(ip, this, flags, next);
	
	// Turn off aparatus display
	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	
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
	}

#define MIN_THICK	0.001f

void MakeBoxThick(Box3 &box)
	{
	if (box.IsEmpty()) box.MakeCube(Point3(0,0,0),1.0f);
	for (int i=0; i<3; i++) {
		if (fabs(box.pmax[i]-box.pmin[i])<MIN_THICK) {
			box.pmax[i] = box.pmin[i] + MIN_THICK;
			}
		}
	}

void FFDMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	// Compute our validity interval
	Interval valid = LocalValidity(t);
	if (valid.Empty()) valid.SetInstant(t);
	
	// Cache the input box
	lbox = *mc.box;
	MakeBoxThick(lbox);

	if (NumPts()==64) {	
		FFDDeformer44 deformer(this,t,&mc);	
		os->obj->Deform(&deformer, TRUE);
	} else if (NumPts()==27) {
		FFDDeformer33 deformer(this,t,&mc);	
		os->obj->Deform(&deformer, TRUE);
	} else {
		FFDDeformer22 deformer(this,t,&mc);	
		os->obj->Deform(&deformer, TRUE);
		}
	
	// This will intersect our validity with the object's
	// validity.
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);
	}

Interval FFDMod::LocalValidity(TimeValue t)
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
		for (int i=0; i<NumPts(); i++) {			
			if (GetPtCont(i)) GetPtCont(i)->GetValue(t,&p,valid,CTRL_ABSOLUTE);
			}
		return valid;
		}
	}

RefTargetHandle FFDMod44::Clone(RemapDir& remap)
	{
	FFDMod44 *mod = new FFDMod44;
	 
	mod->ReplaceReference(TM_REF,remap.CloneRef(tmControl));
	mod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	
	for (int i=0; i<64; i++) {
		if (ptCont[i]) {
			mod->ReplaceReference(i+3,remap.CloneRef(ptCont[i]));
			}
		mod->pt[i] = pt[i];
		}

	mod->sel  = sel;
	mod->lbox = lbox;

	BaseClone(this, mod, remap);
	return mod;
	}

RefTargetHandle FFDMod33::Clone(RemapDir& remap)
	{
	FFDMod33 *mod = new FFDMod33;
	 
	mod->ReplaceReference(TM_REF,remap.CloneRef(tmControl));
	mod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	
	for (int i=0; i<27; i++) {
		if (ptCont[i]) {
			mod->ReplaceReference(i+3,remap.CloneRef(ptCont[i]));
			}
		mod->pt[i] = pt[i];
		}

	mod->sel  = sel;
	mod->lbox = lbox;

	BaseClone(this, mod, remap);
	return mod;
	}

RefTargetHandle FFDMod22::Clone(RemapDir& remap)
	{
	FFDMod22 *mod = new FFDMod22;
	 
	mod->ReplaceReference(TM_REF,remap.CloneRef(tmControl));
	mod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	
	for (int i=0; i<8; i++) {
		if (ptCont[i]) {
			mod->ReplaceReference(i+3,remap.CloneRef(ptCont[i]));
			}
		mod->pt[i] = pt[i];
		}

	mod->sel  = sel;
	mod->lbox = lbox;

	BaseClone(this, mod, remap);
	return mod;
	}


int  FFDMod::RemapRefOnLoad(int iref) {
	if(loadRefVersion == ES_REF_VER_0 && iref > PBLOCK_REF)
		return iref+1;
	return iref;
}
	
#define FFD_SEL_CHUNKID			0x0100
#define FFD_PT_CHUNKID			0x0200
#define FFD_SELLEVEL_CHUNKID	0x0300
#define FFD_OFFSETS_CHUNKID		0x0400

// The following chunk tells which reference version we're dealing with
#define REF_VERSION_CHUNK	0x2000

//deform involume setting has changed from 2.5 to 3.0, this will fix that
class InVolFixPLCB : public PostLoadCallback 
{
public:
	FFDMod *ffd;

	InVolFixPLCB(FFDMod* f) { ffd = f; }
	void proc(ILoad *iload)
	{
		if (ffd->loadRefVersion == ES_REF_VER_0)
			ffd->pblock->SetValue(ffd_deform, 0, !ffd->pblock->GetInt(ffd_deform,0));
		delete this;
	}
};

IOResult FFDMod::Load(ILoad *iload) 
	{
	Modifier::Load(iload);	
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(versionsMod, NUM_OLDVERSIONS, GetClassDesc()->GetParamBlockDesc(0), this, PBLOCK_REF));
	iload->RegisterPostLoadCallback(new InVolFixPLCB(this));
	ULONG nb;
	IOResult res;
	loadRefVersion = ES_REF_VER_0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case REF_VERSION_CHUNK:
				res = iload->Read(&loadRefVersion,sizeof(int), &nb);
				break;
			case FFD_SEL_CHUNKID:
				sel.Load(iload);
				break;
			case FFD_PT_CHUNKID:
				res = iload->Read(GetPtPtr(),NumPts()*sizeof(Point3), &nb);
				break;
			case FFD_OFFSETS_CHUNKID:
				res = iload->Read(GetOffsetPtr(),NumPts()*sizeof(Point3), &nb);
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

IOResult FFDMod::Save(ISave *isave) 	
	{
	Modifier::Save(isave);
	ULONG nb;
	int refVer = ES_REF_VER_1;
	
	isave->BeginChunk(REF_VERSION_CHUNK);
	isave->Write(&refVer,sizeof(int), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(FFD_SEL_CHUNKID);
	sel.Save(isave);
	isave->EndChunk();
	
	isave->BeginChunk(FFD_PT_CHUNKID);
	isave->Write(GetPtPtr(),NumPts()*sizeof(Point3), &nb);
	isave->EndChunk();

	isave->BeginChunk(FFD_OFFSETS_CHUNKID);
	isave->Write(GetOffsetPtr(),NumPts()*sizeof(Point3), &nb);
	isave->EndChunk();

	isave->BeginChunk(FFD_SELLEVEL_CHUNKID);
	isave->Write(&selLevel,sizeof(selLevel),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}

RefResult FFDMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	return REF_SUCCEED;
	}

int FFDMod::Display(
		TimeValue t, 
		INode* inode, 
		ViewExp *vpt, 
		int flags, 
		ModContext *mc)
	{
	GraphicsWindow *gw = vpt->getGW();
	
	// Compute a the transformation out of lattice space into world space
	// Then plug this matrix into the GW.	
	Matrix3 tm = CompTM(t,inode,mc);
	gw->setTransform(tm);
	
	// Draw...
	if (ip->GetSubObjectLevel()==SEL_LATTICE) {
		//gw->setColor(LINE_COLOR, (float)1, (float)1, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}
	DrawControlPoints(t,vpt,gw,*mc->box);

	return 0;
	}


int FFDMod::HitTest(
		TimeValue t, 
		INode* inode, 
		int type, 
		int crossing, 
		int flags, 
		IPoint2 *p, 
		ViewExp *vpt, 
		ModContext* mc)
	{
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

	int res = DrawControlPoints(t,vpt,gw,*mc->box,TRUE,inode,mc,flags);

	// Restore the GW's render limits
	gw->setRndLimits(savedLimits);

	return res;
	}


void FFDMod::GetWorldBoundBox(
		TimeValue t,
		INode* inode, 
		ViewExp *vpt, 
		Box3& box, 
		ModContext *mc)
	{	
	Box3 mcbox = *mc->box;
	MakeBoxThick(mcbox);
	Point3 s = LatticeSize(mcbox);
	Point3 p;
	GraphicsWindow *gw = vpt->getGW();
	//if (mc && mc->box->IsEmpty()) return;

	// Compute a the transformation out of lattice space into world space	
	Matrix3 tm = CompTM(t,inode,mc);
	
	// Expand the box to include all control points
	for (int i=0; i<NumPts(); i++) {
		p    = GetControlPoint(t,i)*s + mcbox.Min();
		box += p * tm;
		}
	}

int FFDMod::DrawControlPoints(
		TimeValue t,
		ViewExp *vpt,
		GraphicsWindow *gw,
		Box3 box,
		BOOL ht,
		INode *inode,
		ModContext *mc,
		int flags)
	{	
	MakeBoxThick(box);
	Point3 s = LatticeSize(box);
	Point3 p, pp[3];
	int res=0, dispLat, dispSrc;
	int level = ip ? ip->GetSubObjectLevel() : SEL_LATTICE;
	pblock->GetValue(ffd_disp_lattice,0,dispLat,FOREVER);
	pblock->GetValue(ffd_disp_source,0,dispSrc,FOREVER);

	// Draw the control points
	for (int i=0; i<NumPts(); i++) {
		// Maybe skip sel or unsel points
		if (ht && flags&HIT_SELONLY   && !sel[i]) continue;
		if (ht && flags&HIT_UNSELONLY &&  sel[i]) continue;

		if (!ht)
		{
			// Set the line color based on selection
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
			gw->clearHitCode();
			vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL);
			res = 1;
			if (flags&HIT_ABORTONHIT) return res;
			if (level==SEL_LATTICE) return 1;
			}
		}

	// Don't hit test the lattice
	if (ht && level!=SEL_LATTICE) return res;
	if (!dispLat) return res;

	// Set the GW line color
	if (level!=SEL_LATTICE) {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}

	// Draw the lattice
	for (i=0; i<GridWidth(); i++) {
		for (int j=0; j<GridWidth(); j++) {
			for (int k=0; k<GridWidth(); k++) {
				pp[0] = GetControlPoint(t,GridIndex(i,j,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
				if (i<GridWidth()-1) {
					pp[1] = GetControlPoint(t,GridIndex(i+1,j,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				if (j<GridWidth()-1) {
					pp[1] = GetControlPoint(t,GridIndex(i,j+1,k),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				if (k<GridWidth()-1) {
					pp[1] = GetControlPoint(t,GridIndex(i,j,k+1),dispSrc,level==SEL_SETVOLUME)*s + box.Min();
					gw->polyline(2,pp,NULL,NULL,FALSE,NULL);
					}
				}
			}
		}
	
	if (ht && gw->checkHitCode()) {
		gw->clearHitCode();
		vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL);
		return 1;
		}
	return 0;
	}

void FFDMod::AnimateAll()
{
	PlugControllers(0, TRUE);	
}

void FFDMod::Conform()
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

	if (theHold.Holding()) theHold.Put(new MoveRestore(this));	
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
	//center = Point3(0.5,0.5,0.5);
	s = LatticeSize(box);		
	
	// Shrink wrap the lattice to fit the shape of the object
	for (int i=0; i<NumPts(); i++) {				
		
		BOOL hit = FALSE; 	
		Point3 lp = GetPt(i);
		
		// Shoot ray from lattice point to center
		ry.p = box.Min() + lp*s;
		ry.dir =  center - ry.p;
		for (int j=0; j < 3 && !hit; j++, ry.p.z += 0.01f)		
			if (ffdmesh->IntersectRay(ry, at, norm))
				out = hit = TRUE;
		if (!hit || at>1.0f) {			
			// Shoot ray from center to lattice point
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
		getPt(i) = ipt;
		getOffset(i) = ipt-GetControlPoint(t,i,TRUE);
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
}


void FFDMod::Reset()
	{
	int total = NumPts();
	MoveRestore *rest = NULL;
	if (theHold.Holding()) rest = new MoveRestore(this);
//	for (int i=0; i<NumPtConts(); i++) DeleteReference(i+3);

// solved a multilple reference delete problem	
	for (int i=0; i<NumPtConts(); i++) 
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

	masterCont->SetNumSubControllers(total);
	sel.SetSize(total);
	for (i=0; i<total; i++) {
		SetPtCont(i, NULL);
		getPt(i) = GetControlPoint(0,i,TRUE);
		getOffset(i) = Point3(0,0,0);
		}	
	if (rest) theHold.Put(rest);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE);
	NotifyDependents(FOREVER,(PartID) this,REFMSG_BRANCHED_HISTORY_CHANGED);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);  // make sure the TV is upto date
	}

void FFDMod44::SetPtCont(int i,Control *c)
	{
	assert(i>=0 && i<NumPtConts());	
	ptCont[i]=c;
	if (masterCont) masterCont->SetSubController(i,c);
	}

void FFDMod33::SetPtCont(int i,Control *c)
	{
	assert(i>=0 && i<NumPtConts());
	ptCont[i]=c;
	if (masterCont) masterCont->SetSubController(i,c);
	}

void FFDMod22::SetPtCont(int i,Control *c)
	{
	assert(i>=0 && i<NumPtConts());
	ptCont[i]=c;
	if (masterCont) masterCont->SetSubController(i,c);
	}

Point3 FFDMod44::GetControlPoint(TimeValue t, int i, int src, BOOL initVol)
	{
	if (initVol) return pt[i];
	if (src) {
		// Return the unmodified control point
		int ii, jj, kk;
		ii = (i>>4)&3;
		jj = (i>>2)&3;
		kk = i&3;
		return Point3(
			float(ii)/3.0f,
			float(jj)/3.0f,
			float(kk)/3.0f);
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

Point3 FFDMod33::GetControlPoint(TimeValue t, int i, int src, BOOL initVol)
	{
	if (initVol) return pt[i];
	if (src) {
		// Return the unmodified control point
		int ii, jj, kk;
		ii = i/9;
		jj = (i-(ii*9))/3;
		kk = i-(ii*9)-(jj*3);
		return Point3(
			float(ii)/2.0f,
			float(jj)/2.0f,
			float(kk)/2.0f);
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

Point3 FFDMod22::GetControlPoint(TimeValue t, int i, int src, BOOL initVol)
	{
	if (initVol) return pt[i];
	if (src) {
		// Return the unmodified control point
		int ii, jj, kk;
		ii = (i>>2)&1;
		jj = (i>>1)&1;
		kk = i&1;
		return Point3(
			float(ii),
			float(jj),
			float(kk));
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

BOOL FFDMod::SelectSubAnim(int subNum)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore(this));
	
	BOOL add = GetKeyState(VK_CONTROL)<0;
	BOOL sub = GetKeyState(VK_MENU)<0;

	if (!add && !sub) sel.ClearAll();
	if (sub)
		 sel.Clear(subNum-3);
	else sel.Set(subNum-3);

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	return TRUE;
	}

void FFDMod::SelectSubComponent(
		HitRecord *hitRec, 
		BOOL selected, 
		BOOL all, 
		BOOL invert)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) return;

	if (theHold.Holding()) theHold.Put(new SelRestore(this));
	while (hitRec) {
		BOOL state = selected;
		if (invert) state = !sel[hitRec->hitInfo];
		if (state) sel.Set(hitRec->hitInfo);
		else       sel.Clear(hitRec->hitInfo);
		if (!all) break;
		hitRec = hitRec->Next();
		}	
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDMod::ClearSelection(int selLevel)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) return;

	if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel.ClearAll();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDMod::SelectAll(int selLevel)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel.SetAll();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDMod::InvertSelection(int selLevel)
	{
	if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel = ~sel;
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void FFDMod::ActivateSubobjSel(int level, XFormModes& modes)
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
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
	}

void FFDMod::PlugControllers(TimeValue t,BOOL all)
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
				GetPtCont(i)->SetValue(0,&getPt(i),TRUE,CTRL_ABSOLUTE);
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

Point3 LatticeSize(Box3 box)
	{
	Point3 size = box.Max()-box.Min();
	if (size.x==0.0f) size.x = MIN_THICK;
	if (size.y==0.0f) size.y = MIN_THICK;
	if (size.z==0.0f) size.z = MIN_THICK;
	return size;
	}

void FFDMod::Move(
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis, 
		Point3& val, 
		BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	} else {		
		PlugControllers(t);

		// Compute a matrix to move points
		Matrix3 ctm(1);
		tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
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
			theHold.Put(new MoveRestore(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&getPt(i),FOREVER,CTRL_ABSOLUTE);
					getPt(i) = (tm*getPt(i))*itm;
					GetPtCont(i)->SetValue(t,&getPt(i),TRUE,CTRL_ABSOLUTE);
				} else {
					Point3 p = GetPt(i);
					getPt(i) = (tm*p)*itm;
					// Calculate offsets if setting the initial state of the lattice
					if(level==SEL_SETVOLUME) getOffset(i) += GetPt(i)-p;
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}

void FFDMod::Rotate(
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis, 
		Quat& val, 
		BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {
		PlugControllers(t);

		Matrix3 ctm(1);
		tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
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
			theHold.Put(new MoveRestore(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&getPt(i),FOREVER,CTRL_ABSOLUTE);
					getPt(i) = (tm*getPt(i))*itm;
					GetPtCont(i)->SetValue(t,&getPt(i),TRUE,CTRL_ABSOLUTE);
				} else {
					Point3 p = GetPt(i);
					getPt(i) = (tm*p)*itm;
					// Calculate offsets if setting the initial state of the lattice
					if(level==SEL_SETVOLUME) getOffset(i) += GetPt(i)-p;
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}

void FFDMod::Scale(
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis, 
		Point3& val, 
		BOOL localOrigin)
	{
	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {
		PlugControllers(t);

		Matrix3 ctm(1);
		tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
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
			theHold.Put(new MoveRestore(this));
			SetAFlag(A_HELD);
			}
		
		// Move the control points
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				if (GetPtCont(i)) {
					GetPtCont(i)->GetValue(t,&getPt(i),FOREVER,CTRL_ABSOLUTE);
					getPt(i) = (tm*getPt(i))*itm;
					GetPtCont(i)->SetValue(t,&getPt(i),TRUE,CTRL_ABSOLUTE);
				} else {
					Point3 p = GetPt(i);
					getPt(i) = (tm*p)*itm;
					// Calculate offsets if setting the initial state of the lattice
					if(level==SEL_SETVOLUME) getOffset(i) += GetPt(i)-p;
					}
				}
			}
		}
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	}


void FFDMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,
		TimeValue t,
		INode *node,
		ModContext *mc)
	{
	Box3 box = *mc->box;
	MakeBoxThick(box);
	// Compute a the transformation out of lattice space into world space	
	Point3 s = box.Max() - box.Min();
	Matrix3 tm = CompTM(t,node,mc);	

	int level = ip->GetSubObjectLevel();
	if (level==SEL_LATTICE) {
		cb->Center(tm.GetTrans(),0);
	} else {
		Point3 cent(0,0,0);
		int ct=0;
		for (int i=0; i<NumPts(); i++) {
			if (sel[i]) {
				cent += (GetControlPoint(t,i)*s + box.Min()) * tm;
				ct++;
				}
			}
		if (ct) {
			cent /= float(ct);
			cb->Center(cent,0);
			}
		}
	}

void FFDMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,
		TimeValue t,
		INode *node,
		ModContext *mc)
	{
	// Compute a the transformation out of lattice space into world space
	Matrix3 tm = CompTM(t,node,mc);
	cb->TM(tm,0);
	}

Matrix3 FFDMod::CompTM(TimeValue t,INode *inode,ModContext *mc)
	{
	// Compute a the transformation out of lattice space into world space	
	Matrix3 ntm(1);
	if (inode) 
	{
#ifdef DESIGN_VER
		ntm = inode->GetObjTMBeforeWSM(GetCOREInterface()->GetTime());
#else
		ntm = inode->GetObjTMBeforeWSM(t);
#endif // DESIGN_VER
	}
	Matrix3 ctm(1);
	if (mc && mc->tm) {
		ntm = Inverse(*(mc->tm)) * ntm;
		}
	tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);
	return ctm * ntm;
	}

int FFDMod::NumSubObjTypes() 
{ 
	return 3;
}

ISubObjType *FFDMod::GetSubObjType(int i) 
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
	case 0:
		return &SOT_ContPoints;
	case 1:
		return &SOT_Lattice;
	case 2:
		return &SOT_Volume;
	}
	return NULL;
}

