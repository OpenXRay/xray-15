/**********************************************************************
 *<
	FILE: surfctrl.cpp

	DESCRIPTION: A controller that moves an object along a surface

	CREATED BY: Rolf Berteig

	HISTORY: created 2/13/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"

#ifndef NO_CONTROLLER_SURFACE

#include "units.h"
#include "interpik.h"
#include "istdplug.h"

#define SURF_CONTROL_CNAME		GetString(IDS_RB_SURF)

#define U_REF		0
#define V_REF		1
#define SURF_REF	2

class SurfPosition;

class PickSurfMode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		SurfPosition *cont;
		
		PickSurfMode(SurfPosition *c) {cont=c;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);		
		PickNodeCallback *GetFilter() {return this;}
	};

class SurfPosition : public ISurfPosition, public TimeChangeCallback {
	public:
		Control *uCont, *vCont;
		INode *surf;
		int align;
		BOOL flip;
		BOOL heldHere;

		Point3 curval;
		Quat curRot;
		Interval ivalid;

		static HWND hWnd;
		static IObjParam *ip;
		static ICustButton *iPick;		
		static PickSurfMode *pickMode;
		static ISpinnerControl *iU, *iV;
		static SurfPosition *editCont;

		SurfPosition();
		~SurfPosition();

		// Animatable methods
		Class_ID ClassID() {return SURF_CONTROL_CLASSID;}  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SURF_CONTROL_CNAME;}
		void DeleteThis() {delete this;}
		int IsKeyable() {return 0;}
		int NumSubs()  {return 2;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum);
		ParamDimension* GetParamDimension(int i) {return stdPercentDim;}
		BOOL AssignController(Animatable *control,int subAnim);
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 
		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);

		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
		bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

		// Reference methods
		int NumRefs() {return 3;};
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		RefTargetHandle Clone(RemapDir& remap);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Control methods
		void Copy(Control *from);		
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method) {}
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);
		void EnumIKParams(IKEnumCallback &callback);
		BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
		float IncIKParam(TimeValue t,int index,float delta);
		void ClearIKParam(Interval iv,int index);

		// TimeChangeCallback methods
		void TimeChanged(TimeValue t);

		// Local methods
		void SetSurface(INode *node);
		void SetupDialog(HWND hWnd);
		void DestroyDialog();
		void UpdateDialog();
		void GetUV(TimeValue t,float &u, float &v,Interval *iv=NULL);
		Point3 DU(TimeValue t,float u, float v);
		Point3 DV(TimeValue t,float u, float v);
		Point3 Norm(TimeValue t,float u, float v);
		Point3 Pos(TimeValue t,float u, float v,Interval *iv=NULL);

		int GetAlign() {return align;}
		void SetAlign(int a);
		BOOL GetFlip() {return flip;}
		void SetFlip(BOOL f);
	};

class JointParamsSurf : public JointParams {
	public:			 	
		JointParamsSurf() : JointParams((DWORD)JNT_POS,2,1.0f) {}
		void SpinnerChange(InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL interactive);
	};

//--- ClassDesc and class vars -------------------------------------

class SurfClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) { return new SurfPosition();}
	const TCHAR *	ClassName() {return SURF_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return SURF_CONTROL_CLASSID;}
	const TCHAR* 	Category() {return _T("");}
	};
static SurfClassDesc surfCD;
ClassDesc* GetSurfCtrlDesc() {return &surfCD;}


HWND             SurfPosition::hWnd     = NULL;
IObjParam       *SurfPosition::ip       = NULL;
ICustButton     *SurfPosition::iPick    = NULL;
ISpinnerControl *SurfPosition::iU       = NULL;
ISpinnerControl *SurfPosition::iV       = NULL;
PickSurfMode    *SurfPosition::pickMode = NULL;
SurfPosition    *SurfPosition::editCont = NULL;

static INT_PTR CALLBACK SurfParamDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--- SurfPosition methods ------------------------------------------

SurfPosition::SurfPosition()
	{
	uCont = vCont = NULL;
	surf = NULL;
	ReplaceReference(U_REF,NewDefaultFloatController());
	ReplaceReference(V_REF,NewDefaultFloatController());
	align = IDC_ALIGN_NONE;
	flip  = FALSE;
	ivalid.SetEmpty();	
	heldHere = false;

	}

SurfPosition::~SurfPosition()
	{
	DeleteAllRefsFromMe();
	}

Animatable* SurfPosition::SubAnim(int i)
	{
	switch (i) {
		case 0: return uCont; break;
		case 1: return vCont; break;
		};
	return NULL;
	}

TSTR SurfPosition::SubAnimName(int i)
	{
	switch (i) {
		case 0: return GetString(IDS_RB_U); break;
		case 1: return GetString(IDS_RB_V); break;
		}
	return _T("");
	}

int SurfPosition::SubNumToRefNum(int subNum)
	{
	switch (subNum) {
		case 0: return 0;
		case 1: return 1;
		default: return -1;
		}
	}

BOOL SurfPosition::AssignController(
		Animatable *control,int subAnim)
	{
	ReplaceReference(subAnim,(ReferenceTarget*)control);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return TRUE;
	}

void SurfPosition::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editCont = this;
	if (flags&BEGIN_EDIT_HIERARCHY) {		
		if (align!=IDC_ALIGN_NONE) return;

		JointParamsSurf *jp = (JointParamsSurf*)GetProperty(PROPID_JOINTPARAMS);
		InterpCtrlUI *ui;	

		if (!jp) {
			jp = new JointParamsSurf();
			SetProperty(PROPID_JOINTPARAMS,jp);
			}

		if (prev &&
			prev->ClassID()==ClassID() && 
		    (ui = (InterpCtrlUI*)prev->GetProperty(PROPID_INTERPUI))) {
			JointParams *prevjp = (JointParams*)prev->GetProperty(PROPID_JOINTPARAMS);
			prevjp->EndDialog(ui);
			ui->cont = this;
			ui->ip   = ip;
			prev->SetProperty(PROPID_INTERPUI,NULL);
			JointDlgData *jd = (JointDlgData*)GetWindowLongPtr(ui->hParams,GWLP_USERDATA);
			jd->jp = jp;
			jp->InitDialog(ui);
		} else {
			ui = new InterpCtrlUI(NULL,ip,this);
			DWORD f=0;
			if (!jp || !jp->RollupOpen()) f = APPENDROLL_CLOSED;	

			ui->hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SURFJOINTPARAMS),
				JointParamDlgProc,
				GetString(IDS_RB_SURFJOINTPARAMS), 
				(LPARAM)new JointDlgData(ui,jp),f);	
			}
	
		SetProperty(PROPID_INTERPUI,ui);
	} else {
		pickMode = new PickSurfMode(this);
		hWnd     = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SURFPARAMS),
			SurfParamDlgProc,
			GetString(IDS_RB_SURFPARAMS), 
			(LPARAM)this );				
		ip->RegisterTimeChangeCallback(this);
		}
	}

void SurfPosition::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	editCont = NULL;
	this->ip = NULL;
	if (next && next->ClassID()==ClassID() && !hWnd) return;

	if (hWnd) {
		ip->UnRegisterTimeChangeCallback(this);
		ip->ClearPickMode();
		delete pickMode;
		pickMode = NULL;		
		ip->DeleteRollupPage(hWnd);
		hWnd = NULL;
	} else {
		int index = aprops.FindProperty(PROPID_INTERPUI);
		if (index>=0) {
			InterpCtrlUI *ui = (InterpCtrlUI*)aprops[index];
			if (ui->hParams) {
				ip->UnRegisterDlgWnd(ui->hParams);
				ip->DeleteRollupPage(ui->hParams);			
				}
			index = aprops.FindProperty(PROPID_INTERPUI);
			if (index>=0) {
				delete aprops[index];
				aprops.Delete(index,1);
				}
			}	
		}
	}

RefTargetHandle SurfPosition::GetReference(int i)
	{
	switch (i) {
		case U_REF: return uCont;
		case V_REF: return vCont;
		case SURF_REF: return surf;
		}
	return NULL;
	}

void SurfPosition::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case U_REF: uCont = (Control*)rtarg; break;
		case V_REF: vCont = (Control*)rtarg; break;
		case SURF_REF: surf = (INode*)rtarg; break;
		}
	}

RefResult SurfPosition::NotifyRefChanged(
		Interval iv, RefTargetHandle hTarg, 
		PartID &partID, RefMessage msg)
	{
	switch (msg) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hWnd && editCont == this) UpdateDialog();
			break;

		case REFMSG_TARGET_DELETED:
			if (hTarg == surf) {
				surf = NULL;
				if (hWnd && editCont == this) {
					SetWindowText(GetDlgItem(hWnd,IDC_SURFNAME),
						GetString(IDS_RB_NONE));
					}
				}			
			break;

		case REFMSG_OBJECT_CACHE_DUMPED:
			
			return REF_STOP;

			break;
		}
	return REF_SUCCEED;
	}

void SurfPosition::SetAlign(int a)
	{
	align = a;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	if (ip) UpdateDialog();
	}

void SurfPosition::SetFlip(BOOL f)
	{
	flip = f;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	if (ip) UpdateDialog();
	}

RefTargetHandle SurfPosition::Clone(RemapDir& remap)
	{
	SurfPosition *cont = new SurfPosition;
	cont->ReplaceReference(U_REF,remap.CloneRef(uCont));
	cont->ReplaceReference(V_REF,remap.CloneRef(vCont));
	cont->ReplaceReference(SURF_REF,surf);
	cont->align = align;
	cont->flip  = flip;
	BaseClone(this, cont, remap);
	return cont;
	}

#define ALIGN_CHUNK				0x0100
#define FLIP_CHUNK				0x0110
#define JOINTPARAMSURF_CHUNK	0x0200

IOResult SurfPosition::Save(ISave *isave)
	{
	JointParamsSurf *jp = (JointParamsSurf*)GetProperty(PROPID_JOINTPARAMS);
	ULONG nb;

	if (jp) {
		isave->BeginChunk(JOINTPARAMSURF_CHUNK);
		jp->Save(isave);
		isave->EndChunk();
		}

	isave->BeginChunk(ALIGN_CHUNK);
	isave->Write(&align,sizeof(align),&nb);
	isave->EndChunk();

	if (flip) {
		isave->BeginChunk(FLIP_CHUNK);	
		isave->EndChunk();
		}
	return IO_OK;
	}

IOResult SurfPosition::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case ALIGN_CHUNK:
				res=iload->Read(&align,sizeof(align),&nb);
				break;

			case JOINTPARAMSURF_CHUNK: {
				JointParamsSurf *jp = new JointParamsSurf;
				jp->Load(iload);
				SetProperty(PROPID_JOINTPARAMS,jp);
				break;
				}

			case FLIP_CHUNK:
				flip = TRUE;
				break;
			}		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}

void SurfPosition::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
		SurfPosition *cont = (SurfPosition*)from;
		ReplaceReference(U_REF,cont->uCont->Clone());
		ReplaceReference(V_REF,cont->vCont->Clone());
		ReplaceReference(SURF_REF,cont->surf);
		align = cont->align;
		flip  = cont->flip;
		}
	}

void SurfPosition::GetUV(
		TimeValue t,float &u, float &v,Interval *iv)
	{	
	Interval valid;
	if (!iv) iv = &valid;	
	uCont->GetValue(t,&u,*iv);
	vCont->GetValue(t,&v,*iv);
	}

Point3 SurfPosition::DU(TimeValue t,float u, float v)
	{
	// CAL-08/14/02: don't cross the uv wrapping boundary. this is a hack.
	// TODO: It'll be better to have an API function for the surface to return its tangent.
	if (floor(u+0.01f) != floor(u))
		return (Pos(t,u,v)-Pos(t,u-0.01f,v))/0.01f;
	else
		return (Pos(t,u+0.01f,v)-Pos(t,u,v))/0.01f;
	}

Point3 SurfPosition::DV(TimeValue t,float u, float v)
	{
	// CAL-08/14/02: don't cross the uv wrapping boundary. this is a hack.
	// TODO: It'll be better to have an API function for the surface to return its tangent.
	if (floor(v+0.01f) != floor(v))
		return (Pos(t,u,v)-Pos(t,u,v-0.01f))/0.01f;
	else
		return (Pos(t,u,v+0.01f)-Pos(t,u,v))/0.01f;
	}

Point3 SurfPosition::Norm(TimeValue t,float u, float v)
	{
	Point3 du = DU(t,u,v);
	Point3 dv = DV(t,u,v);
	return Normalize(du^dv);
	}

Point3 SurfPosition::Pos(
		TimeValue t,float u, float v,Interval *iv)
	{	
	Interval valid;
	if (!iv) iv = &valid;
	if (surf) {
		// CAL-05/30/02: should eval at time t
		ObjectState os = surf->EvalWorldState(t);
		// CAL-05/30/02: need to set validity interval explicitly, cause GetSurfacePoint() doesn't
		//		set the interval argument.
		*iv &= os.obj->ChannelValidity(t,GEOM_CHAN_NUM);
		*iv &= os.obj->ChannelValidity(t,TOPO_CHAN_NUM);
		return os.obj->GetSurfacePoint(t,u,v,*iv) *
			surf->GetObjTMBeforeWSM(t,iv);
	} else {
		return Point3(0,0,0);
		}
	}

void SurfPosition::GetValue(
		TimeValue t, void *val, Interval &valid, 
		GetSetMethod method)
	{
	if (!ivalid.InInterval(t)) {
		float u, v;
		ivalid = FOREVER;
		GetUV(t,u,v,&ivalid);
		curval = Pos(t,u,v,&ivalid);
		if (align!=IDC_ALIGN_NONE) {
			Matrix3 tm(1);
			Point3 norm = Norm(t,u,v);
			tm.SetRow(2,flip?-norm:norm);
			if (align==IDC_ALIGN_U) 
				 tm.SetRow(0,Normalize(DU(t,u,v)));
			else tm.SetRow(0,Normalize(DV(t,u,v)));
			tm.SetRow(1,Normalize(tm.GetRow(2)^tm.GetRow(0)));
			curRot = Quat(tm);
			}
		}

	if (method==CTRL_RELATIVE) {
		Matrix3 *mat = (Matrix3*)val;
		if (align!=IDC_ALIGN_NONE) {
			curRot.MakeMatrix(*mat);
			}
		mat->SetTrans(curval);		
	} else {
		*((Point3*)val) = curval;
		}
	valid &= ivalid;
	}

void SurfPosition::CommitValue(TimeValue t)
	{
	uCont->CommitValue(t);
	vCont->CommitValue(t);
	}

void SurfPosition::RestoreValue(TimeValue t)
	{
	uCont->RestoreValue(t);
	vCont->RestoreValue(t);
	}

void SurfPosition::TimeChanged(TimeValue t)
	{
	UpdateDialog();
	}

void SurfPosition::SetSurface(INode *node)
	{
	ivalid.SetEmpty();
	ReplaceReference(SURF_REF,node);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	if (ip) UpdateDialog();
	}


//--- PickSurfMode methods -----------------------------------------

BOOL PickSurfMode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,
		IPoint2 m,int flags)
	{
	INode *node = cont->ip->PickNode(hWnd,m, this);	//AG: added "this" argument such that the Filter below is used
	return node?TRUE:FALSE;
	}

BOOL PickSurfMode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		cont->SetSurface(node);
		ip->RedrawViews(ip->GetTime());
		}
	return TRUE;
	}

void PickSurfMode::EnterMode(IObjParam *ip)
	{
	cont->iPick->SetCheck(TRUE);
	}

void PickSurfMode::ExitMode(IObjParam *ip)
	{
	cont->iPick->SetCheck(FALSE);
	}

BOOL PickSurfMode::Filter(INode *node)
	{
	if (node) {
		//AG: added code for looptest
		if (node->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED)
			return FALSE;

		ObjectState os = node->EvalWorldState(0);
		if (os.obj->IsParamSurface()) return TRUE;
		}
	return FALSE;
	}


//--- UI -------------------------------------------------------------

static INT_PTR CALLBACK SurfParamDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	SurfPosition *co = (SurfPosition*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG:

			co = (SurfPosition*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)co);
			co->SetupDialog(hWnd);
//			SetFocus(GetCOREInterface()->GetMAXHWnd());
			break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;


		case WM_CUSTEDIT_ENTER:
			if (co->heldHere) {
				theHold.Accept(GetString(IDS_RB_CHANGEUV));
				co->heldHere = false;
			}
			theHold.Accept(GetString(IDS_RB_CHANGEUV));
			co->ip->RedrawViews(co->ip->GetTime());
			break;

		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam)) {
				theHold.Accept(GetString(IDS_RB_CHANGEUV));
			} else {
				theHold.Cancel();
				}
			co->ip->RedrawViews(co->ip->GetTime());
			break;


		case CC_SPINNER_CHANGE: {
			if (!theHold.Holding()) 
			{
				co->heldHere = true;
				theHold.Begin();
			}
			float f;
			TimeValue t = co->ip->GetTime(); // mjm - 3.1.99
			switch (LOWORD(wParam)) {
				case IDC_SURF_USPIN:
					f = co->iU->GetFVal() / 100.0f; // RB 4/19/99: Display as percent
					co->uCont->SetValue(t, &f);
					co->iU->SetKeyBrackets( co->uCont->IsKeyAtTime(t, 0) ); // mjm - 3.1.99
					break;
				case IDC_SURF_VSPIN:
					f = co->iV->GetFVal()/ 100.0f; // RB 4/19/99: Display as percent
					co->vCont->SetValue(t, &f);
					co->iV->SetKeyBrackets( co->vCont->IsKeyAtTime(t, 0) ); // mjm - 3.1.99
					break;
				}
			co->ip->RedrawViews(co->ip->GetTime());
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_ALIGN_FLIP:
					co->flip = IsDlgButtonChecked(hWnd,IDC_ALIGN_FLIP);
					co->ivalid.SetEmpty();
					co->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					co->ip->RedrawViews(co->ip->GetTime());
					break;

				case IDC_ALIGN_NONE:
				case IDC_ALIGN_U:
				case IDC_ALIGN_V:
					co->align = LOWORD(wParam);
					co->ivalid.SetEmpty();
					co->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					co->ip->RedrawViews(co->ip->GetTime());
					co->UpdateDialog();
					break;

				case IDC_PICKSURF:
					co->ip->SetPickMode(co->pickMode);
					break;
				}
			break;

		case WM_LBUTTONDOWN:case WM_LBUTTONUP:	case WM_MOUSEMOVE:
			co->ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			return FALSE;

		case WM_DESTROY:
			co->DestroyDialog();
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void SurfPosition::SetupDialog(HWND hWnd)
	{
	this->hWnd = hWnd;

	iPick = GetICustButton(GetDlgItem(hWnd,IDC_PICKSURF));
	iPick->SetType(CBT_CHECK);
	iPick->SetHighlightColor(GREEN_WASH);

	iU = GetISpinner(GetDlgItem(hWnd,IDC_SURF_USPIN));			
	iU->SetLimits(-999999999.0f,999999999.0f,FALSE);			
	iU->SetAutoScale();
	iU->LinkToEdit(GetDlgItem(hWnd,IDC_SURF_U),EDITTYPE_FLOAT);
	
	iV = GetISpinner(GetDlgItem(hWnd,IDC_SURF_VSPIN));
	iV->SetLimits(-999999999.0f,999999999.0f,FALSE);			
	iV->SetAutoScale();
	iV->LinkToEdit(GetDlgItem(hWnd,IDC_SURF_V),EDITTYPE_FLOAT);
	
	UpdateDialog();
	}

void SurfPosition::UpdateDialog()
	{	
	if (!hWnd) return;

	TimeValue t = ip->GetTime();
	float f;
	uCont->GetValue(t,&f,FOREVER);
	iU->SetValue(f * 100.0f,FALSE); // RB 4/19/99: Display as percent
	iU->SetKeyBrackets( uCont->IsKeyAtTime(t, 0) ); // mjm - 3.1.99
	vCont->GetValue(t,&f,FOREVER);
	iV->SetValue(f * 100.0f,FALSE);
	iV->SetKeyBrackets( vCont->IsKeyAtTime(t, 0) ); // mjm - 3.1.99

	// RB 3/7/99: Auto radio buttons were supposed to uncheck other buttons... I thought.
	//CheckDlgButton(hWnd,align,TRUE);
	CheckDlgButton(hWnd, IDC_ALIGN_NONE, align==IDC_ALIGN_NONE);	
	CheckDlgButton(hWnd, IDC_ALIGN_U, align==IDC_ALIGN_U);
	CheckDlgButton(hWnd, IDC_ALIGN_V, align==IDC_ALIGN_V);

	CheckDlgButton(hWnd,IDC_ALIGN_FLIP,flip);
	if (align!=IDC_ALIGN_NONE)
		 EnableWindow(GetDlgItem(hWnd,IDC_ALIGN_FLIP),TRUE);
	else EnableWindow(GetDlgItem(hWnd,IDC_ALIGN_FLIP),FALSE);

	if (surf) {
		SetWindowText(GetDlgItem(hWnd,IDC_SURFNAME),
			surf->GetName());
	} else {
		SetWindowText(GetDlgItem(hWnd,IDC_SURFNAME),
			GetString(IDS_RB_NONE));
		}
	}

void SurfPosition::DestroyDialog()
	{
	ReleaseISpinner(iU); iU = NULL;
	ReleaseISpinner(iV); iV = NULL;
	ReleaseICustButton(iPick); iPick = NULL;
	hWnd = NULL;
	}

TSTR SurfPosition::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool SurfPosition::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool SurfPosition::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	if( gNodeTarget->GetAnim() == surf ) {
		SetSurface( NULL );
		return true;
	}

	return false;
}

bool SurfPosition::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID )
		return true;

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool SurfPosition::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID ) {
		SetSurface( (INode*)pChildAnim );
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		return true;
	}
	return Control::SvLinkChild(gom, gNodeThis, gNodeChild);
}

SvGraphNodeReference SurfPosition::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		if( surf )
			gom->AddRelationship( nodeRef.gNode, surf, 0, RELTYPE_CONSTRAINT );
	}

	return nodeRef;
}

//--- IK -----------------------------------------------------------

int SurfPosition::SetProperty(ULONG id, void *data)
	{
	if (id==PROPID_JOINTPARAMS) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {
			JointParamsSurf *jp = (JointParamsSurf*)GetProperty(id);
			if (jp) {
				*jp = *((JointParamsSurf*)data);
				delete (JointParamsSurf*)data;
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}					
			}
		return 1;
	} else
	if (id==PROPID_INTERPUI) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {
			InterpCtrlUI *ui = (InterpCtrlUI*)GetProperty(id);
			if (ui) {
				*ui = *((InterpCtrlUI*)data);
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}					
			}
		return 1;
	} else {
		return Animatable::SetProperty(id,data);
		}
	}

void* SurfPosition::GetProperty(ULONG id)
	{
	if (id==PROPID_INTERPUI || id==PROPID_JOINTPARAMS) {
		int index = aprops.FindProperty(id);
		if (index>=0) {
			return aprops[index];
		} else {
			return NULL;
			}
	} else {
		return Animatable::GetProperty(id);
		}
	}

void JointParamsSurf::SpinnerChange(
		InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,
		BOOL interactive)
	{
	float val;
	int set = 0;

	switch (id) {
		case IDC_XFROMSPIN:
			val = min[0] = spin->GetFVal()/scale; 
			set = 1;
			break;
		case IDC_XTOSPIN:
			val = max[0] = spin->GetFVal()/scale;
			set = 1;
			break;
		
		case IDC_XDAMPINGSPIN:
			damping[0] = spin->GetFVal(); break;

		case IDC_YFROMSPIN:
			val = min[1] = spin->GetFVal()/scale; 
			set = 2;
			break;
		case IDC_YTOSPIN:
			val = max[1] = spin->GetFVal()/scale;
			set = 2;
			break;
		
		case IDC_YDAMPINGSPIN:
			damping[1] = spin->GetFVal(); break;
		}
	
	if (set && interactive) {
		SurfPosition *c = (SurfPosition*)ui->cont;
 		if (set==1) 
			 c->uCont->SetValue(
				ui->ip->GetTime(),&val,TRUE,CTRL_ABSOLUTE);
		else c->vCont->SetValue(
				ui->ip->GetTime(),&val,TRUE,CTRL_ABSOLUTE);
		ui->ip->RedrawViews(ui->ip->GetTime(),REDRAW_INTERACTIVE);
		}
	}

void SurfPosition::EnumIKParams(IKEnumCallback &callback)
	{
	JointParamsSurf *jp = (JointParamsSurf*)GetProperty(PROPID_JOINTPARAMS);
	if (jp && align==IDC_ALIGN_NONE) {
		if (jp->Active(0)) callback.proc(this,0);
		if (jp->Active(1)) callback.proc(this,1);
		}
	}

BOOL SurfPosition::CompDeriv(
		TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
	{
	JointParamsSurf *jp = (JointParamsSurf*)GetProperty(PROPID_JOINTPARAMS);
	if (!jp || align!=IDC_ALIGN_NONE) return FALSE;

	float u, v;		
	GetUV(t,u,v);
	Point3 zero(0,0,0);

	if (jp->Active(0)) {
		Point3 d = DU(t,u,v);
		for (int j=0; j<derivs.NumEndEffectors(); j++) {
			if (flags&POSITION_DERIV) {
				derivs.DP(d,j);
				}
			if (flags&ROTATION_DERIV) {
				derivs.DR(zero,j);
				}
			}
		derivs.NextDOF();
		}
	if (jp->Active(1)) {
		Point3 d = DV(t,u,v);
		for (int j=0; j<derivs.NumEndEffectors(); j++) {
			if (flags&POSITION_DERIV) {
				derivs.DP(d,j);
				}
			if (flags&ROTATION_DERIV) {
				derivs.DR(zero,j);
				}
			}
		derivs.NextDOF();
		}
	
	ptm.SetTrans(Pos(t,u,v));
	return TRUE;
	}

float SurfPosition::IncIKParam(TimeValue t,int index,float delta)
	{
	JointParamsSurf *jp = (JointParamsSurf*)GetProperty(PROPID_JOINTPARAMS);
	if (fabs(delta) > 0.01f) {
		if (delta<0) delta = -0.01f;
		else delta = 0.01f;
		}
	if (jp) {
		float u, v;
		if (index==0) {
			if (jp->Limited(0)) {			
				uCont->GetValue(t,&u,FOREVER);			
				}
			delta = jp->ConstrainInc(0,u,delta);
			uCont->SetValue(t,&delta,FALSE,CTRL_RELATIVE);
		} else {
			if (jp->Limited(1)) {			
				vCont->GetValue(t,&v,FOREVER);			
				}
			delta = jp->ConstrainInc(0,v,delta);
			vCont->SetValue(t,&delta,FALSE,CTRL_RELATIVE);
			}
		}	
	return delta;
	}

void SurfPosition::ClearIKParam(Interval iv,int index)
	{
	uCont->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE);
	vCont->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE);
	}

#endif // NO_CONTROLLER_SURFACE

