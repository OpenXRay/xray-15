/**********************************************************************
 *<
	FILE: tapehelp.cpp

	DESCRIPTION:  A measuring tape helper implementation

	CREATED BY: Don Brittain

	HISTORY: created 8 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
//#include "target.h"
#include "tapehelp.h"

#ifndef NO_HELPER_TAPE // russom 10/16/01

// Parameter block indices
#define PB_LENGTH	0

//------------------------------------------------------

class TapeHelpClassDesc:public ClassDesc 
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TapeHelpObject; }
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	const TCHAR *	ClassName() { return GetString(IDS_DB_TAPE_CLASS); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(TAPEHELP_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetTapeParams(); }
};

static TapeHelpClassDesc tapeHelpDesc;

ClassDesc* GetTapeHelpDesc() { return &tapeHelpDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for measuring tape class.
Mesh TapeHelpObject::mesh;
short TapeHelpObject::meshBuilt=0;
HWND TapeHelpObject::hTapeHelpParams = NULL;
IObjParam *TapeHelpObject::iObjParams;
ISpinnerControl *TapeHelpObject::lengthSpin = NULL;
float TapeHelpObject::dlgLength = 100.0f;
short TapeHelpObject::dlgSpecLen = FALSE;

void resetTapeParams() 
{
	TapeHelpObject::dlgLength = 100.0f;
	TapeHelpObject::dlgSpecLen = FALSE;
}

INT_PTR CALLBACK TapeHelpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	TapeHelpObject *th = (TapeHelpObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	
	if ( !th && message != WM_INITDIALOG ) return FALSE;

	switch ( message ) {
		case WM_INITDIALOG:
			th = (TapeHelpObject *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)th );
			SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );
			
			th->lengthSpin = GetISpinner(GetDlgItem(hDlg,IDC_LENSPINNER));
			th->lengthSpin->SetLimits( MIN_TAPE_LEN, MAX_TAPE_LEN, FALSE );
			// alexc - 03.06.09 - increments proportional to the spinner value
            th->lengthSpin->SetAutoScale();
			if(th->specLenState)
				th->lengthSpin->SetValue( th->GetLength(th->iObjParams->GetTime()), FALSE );
			else
				th->lengthSpin->SetValue( th->lastDist, FALSE );
			th->lengthSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LENGTH), EDITTYPE_UNIVERSE );
						
			CheckDlgButton( hDlg, IDC_SPEC_LEN, th->specLenState );
			EnableWindow(GetDlgItem(hDlg, IDC_LENGTH), th->specLenState);
			EnableWindow(GetDlgItem(hDlg, IDC_LENSPINNER), th->specLenState);

			return FALSE;			

		case WM_DESTROY:
			ReleaseISpinner( th->lengthSpin );
			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();
			switch ( LOWORD(wParam) ) {
				case IDC_LENSPINNER:
					th->SetLength( th->iObjParams->GetTime(), th->lengthSpin->GetFVal() );
					th->lengthSpin->SetKeyBrackets(th->pblock->KeyFrameAtTime(PB_LENGTH,th->iObjParams->GetTime()));
					th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
					break;
				}
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_END);
			return TRUE;

		case WM_MOUSEACTIVATE:
			th->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:			
			switch( LOWORD(wParam) ) {
				case IDC_SPEC_LEN:					
					th->SetSpecLen( IsDlgButtonChecked( hDlg, IDC_SPEC_LEN ) );
					EnableWindow(GetDlgItem(hDlg, IDC_LENGTH), th->specLenState);
					EnableWindow(GetDlgItem(hDlg, IDC_LENSPINNER), th->specLenState);					
					th->iObjParams->RedrawViews(th->iObjParams->GetTime());
					break;
				}
			return FALSE;

		default:
			return FALSE;
	}
}



void TapeHelpObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	iObjParams = ip;
	editting = TRUE;
	
	if ( !hTapeHelpParams ) {
		hTapeHelpParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_TAPEHELPER),
				TapeHelpParamDialogProc, 
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this );	

		ip->RegisterDlgWnd(hTapeHelpParams);

	} 
	else {
		SetWindowLongPtr( hTapeHelpParams, GWLP_USERDATA, (LONG_PTR)this );
		
		// Init the dialog to our values.
		SetSpecLen( IsDlgButtonChecked(hTapeHelpParams,IDC_SPEC_LEN) );
		if(specLenState)
			lengthSpin->SetValue( GetLength(ip->GetTime()), FALSE );
		else 
			lengthSpin->SetValue( lastDist, FALSE );
	}
}
		
void TapeHelpObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	editting = FALSE;

	if(specLenState)
		dlgLength = GetLength(ip->GetTime());
	dlgSpecLen = IsDlgButtonChecked(hTapeHelpParams, IDC_SPEC_LEN);

	if ( flags&END_EDIT_REMOVEUI ) {		
		ip->UnRegisterDlgWnd(hTapeHelpParams);
		ip->DeleteRollupPage(hTapeHelpParams);
		hTapeHelpParams = NULL;				
	} 
	else {
		SetWindowLongPtr( hTapeHelpParams, GWLP_USERDATA, 0 );
	}
	
	iObjParams = NULL;
}


void TapeHelpObject::BuildMesh()
{
	if(meshBuilt)
		return;
	int nverts = 5;
	int nfaces = 6;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	float d =  5.0f;
	float h = 10.0f;

	mesh.setVert(0, Point3(  -d, -d, 0.0f ));
	mesh.setVert(1, Point3(   d, -d, 0.0f ));
	mesh.setVert(2, Point3(   d,  d, 0.0f ));
	mesh.setVert(3, Point3(  -d,  d, 0.0f ));
	mesh.setVert(4, Point3(0.0f,0.0f, -h  ));
	mesh.faces[0].setVerts(0, 3, 1);
	mesh.faces[0].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setVerts(3, 2, 1);
	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setVerts(0, 1, 4);
	mesh.faces[2].setEdgeVisFlags(1,1,1);
	mesh.faces[3].setVerts(1, 2, 4);
	mesh.faces[3].setEdgeVisFlags(1,1,1);
	mesh.faces[4].setVerts(2, 3, 4);
	mesh.faces[4].setEdgeVisFlags(1,1,1);
	mesh.faces[5].setVerts(3, 0, 4);
	mesh.faces[5].setEdgeVisFlags(1,1,1);
#if 0
	// whoops- rotate 180 about x to get it facing the right way
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.RotateX(DegToRad(180.0));
	for (int i=0; i<nverts; i++)
		mesh.getVert(i) = mat*mesh.getVert(i);
#endif
	mesh.buildNormals();
	mesh.EnableEdgeList(1);
	meshBuilt = 1;
}

#define RadToDegDbl	(180.0 / 3.141592653589793)

static void DisplayPlaneAngle(HWND hDlg, int id, Point3 &dirPt, Point3 &planePt)
{
	float len, cosAng;
	TCHAR buf[32];

	if(len = Length(planePt)) {
		cosAng = DotProd(planePt, dirPt) / len;
		if(cosAng > 0.99999f)	// beyond float accuracy!
			SetDlgItemText(hDlg, id, _T("0"));
		else {
			_stprintf(buf, "%g", acos((double)cosAng) * RadToDegDbl);
			SetDlgItemText(hDlg, id, buf);
		}
	}
	else
		SetDlgItemText(hDlg, id, _T("90"));
}

void TapeHelpObject::UpdateUI(TimeValue t)
{
	if ( hTapeHelpParams &&	GetWindowLongPtr(hTapeHelpParams,GWLP_USERDATA)==(LONG_PTR)this ) {
		if(specLenState) {
			lengthSpin->SetValue( GetLength(t), FALSE );
			lengthSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_LENGTH,t));
		}
		else 
			lengthSpin->SetValue( lastDist, FALSE );
	}
	float dirLen = Length(dirPt);
	if(dirLen) {
		TCHAR buf[32];
		
		_stprintf(buf, "%g", acos((double)dirPt.x) * RadToDegDbl);
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_X_AXIS, buf);
		_stprintf(buf, "%g", acos((double)dirPt.y) * RadToDegDbl);
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_Y_AXIS, buf);
		_stprintf(buf, "%g", acos((double)dirPt.z) * RadToDegDbl);
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_Z_AXIS, buf);

		Point3 planePt = dirPt;
		planePt.z = 0.0f;
		DisplayPlaneAngle(hTapeHelpParams, IDC_TAPE_XY_PLANE, dirPt, planePt);
		planePt = dirPt;
		planePt.x = 0.0f;
		DisplayPlaneAngle(hTapeHelpParams, IDC_TAPE_YZ_PLANE, dirPt, planePt);
		planePt = dirPt;
		planePt.y = 0.0f;
		DisplayPlaneAngle(hTapeHelpParams, IDC_TAPE_ZX_PLANE, dirPt, planePt);
	}
	else {
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_X_AXIS, _T(""));
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_Y_AXIS, _T(""));
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_Z_AXIS, _T(""));
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_XY_PLANE, _T(""));
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_YZ_PLANE, _T(""));
		SetDlgItemText(hTapeHelpParams, IDC_TAPE_ZX_PLANE, _T(""));
	}
}


TapeHelpObject::TapeHelpObject() : HelperObject() 
{
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, TRUE } };

	// RB: Need to make a reference to the parameter block!
	MakeRefByID( FOREVER, 0, CreateParameterBlock( desc, 1 ));
	//pblock = CreateParameterBlock( desc, 1 );

	enable = 0;
	editting = 0;
	lastDist = 0.0f;
	SetLength( TimeValue(0), dlgLength );
	specLenState = dlgSpecLen;
	BuildMesh();
	dirPt.x = dirPt.y = dirPt.z = 0.0f;	
}

TapeHelpObject::~TapeHelpObject()
{
	DeleteAllRefsFromMe();
	pblock = NULL;
}


void TapeHelpObject::SetLength( TimeValue t, float len )
{
	pblock->SetValue( PB_LENGTH, t, len );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float TapeHelpObject::GetLength( TimeValue t, Interval& valid )
{
	float f;
	pblock->GetValue( PB_LENGTH, t, f, valid );
	return f;
}

void TapeHelpObject::SetSpecLen( int onOff) 
{
	specLenState = onOff;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

class TapeHelpObjCreateCallBack: public CreateMouseCallBack 
{
	TapeHelpObject *ob;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TapeHelpObject *obj) { ob = obj; }
	};

int TapeHelpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{
	ob->enable = 1;

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		#ifdef _3D_CREATE
			mat.SetTrans( vpt->SnapPoint(m,m,NULL,SNAP_IN_3D) );
		#else
			mat.SetTrans( vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE) );
		#endif
		if (point==1 && msg==MOUSE_POINT) 
			return 0;
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;

	return TRUE;
}

static TapeHelpObjCreateCallBack tapeHelpCreateCB;

CreateMouseCallBack* TapeHelpObject::GetCreateMouseCallBack() 
{
	tapeHelpCreateCB.SetObj(this);
	return(&tapeHelpCreateCB);
}

static int GetTargetPoint(TimeValue t, INode *inode, Point3& p) 
{
	Matrix3 tmat;
	if (inode->GetTargetTM(t,tmat)) {
		p = tmat.GetTrans();
		return 1;
	}
	return 0;
}

void TapeHelpObject::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm) 
{
	tm = inode->GetObjectTM(t);
	tm.NoScale();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void TapeHelpObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	box = mesh.getBoundingBox(tm);
}

void TapeHelpObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) 
{
	Matrix3 m = inode->GetObjectTM(t);
	Point3 pt;
	Point3 q[4];
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/(float)360.0;
	box = mesh.getBoundingBox();
	box.Scale(scaleFactor);

	float d;
	if (GetTargetPoint(t,inode,pt)){
		d = Length(m.GetTrans()-pt)/Length(inode->GetObjectTM(t).GetRow(2));
		box += Point3(float(0),float(0),-d);
	}
	if(GetSpecLen()) {
		GetLinePoints(t, q, GetLength(t) );
		box += q[0];
		box += q[1];
	}
}

void TapeHelpObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	int i, nv;
	Matrix3 tm;
	float dtarg;
	Point3 pt;
	Point3 q[2];
	GetMat(t,inode,vpt,tm);
	nv = mesh.getNumVerts();
	box.Init();
	for (i=0; i<nv; i++) 
		box += tm*mesh.getVert(i);
	if (GetTargetPoint(t,inode,pt)) {
		tm = inode->GetObjectTM(t);
		dtarg = Length(tm.GetTrans()-pt)/Length(tm.GetRow(2));
		box += tm*Point3(float(0),float(0),-dtarg);
	}
	if(GetSpecLen()) {
		GetLinePoints(t, q, GetLength(t) );
		box += tm * q[0];
		box += tm * q[1];
	}
}

void TapeHelpObject::GetLinePoints(TimeValue t, Point3* q, float len) 
{
	q[0] = Point3( 0.0f, 0.0f, 0.0f);				
	q[1] = Point3( 0.0f, 0.0f, -len);				
}

// From BaseObject
int TapeHelpObject::DrawLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) 
{
	Matrix3 tm = inode->GetObjectTM(t);
	gw->setTransform(tm);
	gw->clearHitCode();
	Point3 pt,v[3];
	if (GetTargetPoint(t,inode,pt)) {
		float den = Length(tm.GetRow(2));
		float dist = (den!=0)?Length(tm.GetTrans()-pt)/den : 0.0f;
		if(!inode->IsFrozen() && !inode->Dependent() && drawing)
			gw->setColor( LINE_COLOR, GetUIColor(COLOR_TAPE_OBJ));
		if (drawing) {
			if(specLenState) {
				GetLinePoints(t, v, GetLength(t) );
				if(drawing == -1)
					v[1] = 0.9f * v[1];
			}
			else {
				v[0] = Point3(0,0,0);
				if(drawing == -1)	// hit-testing!  Shorten the line so target can be picked
					v[1] = Point3(0.0f, 0.0f, -0.9f * dist);
				else
					v[1] = Point3(0.0f, 0.0f, -dist);
			}
			gw->polyline( 2, v, NULL, NULL, FALSE, NULL );	
		}
	}
	return gw->checkHitCode();
}

int TapeHelpObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
{
	HitRegion hitRegion;
	DWORD	savedLimits;
	int res;
	Matrix3 m;
	if (!enable) return  0;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();
	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	// if we get a hit on the mesh, we're done
	gw->clearHitCode();
	if (mesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT )) 
		return TRUE;
	// if not, check the target line, and set the pair flag if it's hit
	// this special case only works with point selection
	if(type != HITTYPE_POINT)
		return 0;
	// don't let line be active if only looking at selected stuff and target isn't selected
	if((flags & HIT_SELONLY) && (inode->GetTarget()) && !inode->GetTarget()->Selected() )
		return 0;
	gw->clearHitCode();
	if(res = DrawLine(t,inode,gw,-1))
		inode->SetTargetNodePair(1);
	gw->setRndLimits(savedLimits);
	return res;
}

void TapeHelpObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Matrix3 invPlane = Inverse(snap->plane);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;

		// Get the two endpoints
		Point3 pt,v[2];
		if (GetTargetPoint(t,inode,pt)) {
			float den = Length(tm.GetRow(2));
			float dist = (den!=0)?Length(tm.GetTrans()-pt)/den : 0.0f;
			if(specLenState)
				GetLinePoints(t, v, GetLength(t) );
			else {
				v[0] = Point3(0,0,0);
				v[1] = Point3(0.0f, 0.0f, -dist);
			}

			for(int i = 0; i < 2; ++i) {
				Point3 thePoint = v[i];

				// If constrained to the plane, make sure this point is in it!
				if(snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE) {
					Point3 test = thePoint * tm * invPlane;
					if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
						return;
				}

				gw->wTransPoint(&thePoint,&screen3);
				screen2.x = (float)screen3.x;
				screen2.y = (float)screen3.y;
				// Are we within the snap radius?
				int len = (int)Length(screen2 - fp);
				if(len <= snap->strength) {
					// Is this priority better than the best so far?
					if(snap->vertPriority < snap->priority) {
						snap->priority = snap->vertPriority;
						snap->bestWorld = thePoint * tm;
						snap->bestScreen = screen2;
						snap->bestDist = len;
					}
					else if(len < snap->bestDist) {
						snap->priority = snap->vertPriority;
						snap->bestWorld = thePoint * tm;
						snap->bestScreen = screen2;
						snap->bestDist = len;
					}
				}
			}
		}
	}
}

int TapeHelpObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();

	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())
		gw->setColor( LINE_COLOR, GetUIColor(COLOR_TAPE_OBJ));
	mesh.render( gw, mtl, NULL, COMP_ALL);	
	DrawLine(t,inode,gw,1);
	gw->setRndLimits(rlim);
	if(editting && !specLenState) {
		Point3 pt(0,0,0);
		Matrix3 tm = inode->GetObjectTM(t);
		GetTargetPoint(t,inode,pt);
		float den = Length(tm.GetRow(2));
		float dist = (den!=0)?Length(tm.GetTrans()-pt)/den : 0.0f;
		lengthSpin->SetValue( lastDist = dist, FALSE );
	}
	if(editting) {
		float len;
		m.NoTrans();
		dirPt = m * Point3(0,0,1);
		if(len = Length(dirPt))
			dirPt *= 1.0f/len;
		UpdateUI(iObjParams->GetTime());
	}
	return(0);
}


// From GeomObject
int TapeHelpObject::IntersectRay(TimeValue t, Ray& r, float& at) { return(0); }

//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult TapeHelpObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			if (iObjParams) UpdateUI(iObjParams->GetTime());
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case 0:
					gpd->dim = stdWorldDim;
					break;				
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case 0:
					gpn->name = TSTR(GetString(IDS_DB_TAPE_LENGTH));
					break;												
			}
			return REF_STOP; 
		}
	}
	return(REF_SUCCEED);
}

ObjectState TapeHelpObject::Eval(TimeValue time){
	return ObjectState(this);
	}

Interval TapeHelpObject::ObjectValidity(TimeValue time) {
	Interval ivalid;
	ivalid.SetInfinite();
	GetLength(time, ivalid);
	UpdateUI(time);
	return ivalid;	
	}

RefTargetHandle TapeHelpObject::Clone(RemapDir& remap) 
{
	TapeHelpObject* newob = new TapeHelpObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->specLenState = specLenState;
	newob->ivalid.SetEmpty();
	newob->enable = enable;
	BaseClone(this, newob, remap);
	return(newob);
}


class TapeHelpCreationManager : public MouseCallBack, ReferenceMaker 
{
	private:
		CreateMouseCallBack *createCB;	
		INode *tapeNode,*targNode;
		TapeHelpObject *tapeObject;
//		TargetObject *targObject;
		Object *targObject;
		int attachedToNode;
		int lastPutCount;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 pt0;
		int ignoreSelectionChange;

		void CreateNewObject();	

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)tapeNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { tapeNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		TapeHelpCreationManager()	{ ignoreSelectionChange = FALSE; }
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
};


#define CID_TAPEOBJCREATE	CID_USER + 5

class TapeHelpCreateMode : public CommandMode 
{
	TapeHelpCreationManager proc;
public:
	void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
	void End() { proc.End(); }

	int Class() { return CREATE_COMMAND; }
	int ID() { return CID_TAPEOBJCREATE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1000000; return &proc; }
	ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
	BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
	void EnterMode() { SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR))); }
	void ExitMode() { SetCursor(LoadCursor(NULL, IDC_ARROW)); }
	BOOL IsSticky() { return FALSE; }
};

static TapeHelpCreateMode theTapeHelpCreateMode;

//TapeHelpCreationManager::TapeHelpCreationManager( IObjCreate *ioc, ClassDesc *desc )
void TapeHelpCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB		= NULL;
	tapeNode		= NULL;
	targNode		= NULL;
	tapeObject		= NULL;
	targObject		= NULL;
	CreateNewObject();
}

//TapeHelpCreationManager::~TapeHelpCreationManager
void TapeHelpCreationManager::End()
{
	if ( tapeObject ) {
		tapeObject->ClearAFlag(A_OBJ_LONG_CREATE);

#ifndef NO_CREATE_TASK	// russom - 12/04/01
		tapeObject->EndEditParams( (IObjParam*)createInterface, 
                    	          END_EDIT_REMOVEUI, NULL);
#endif
		
		if ( !attachedToNode ) {
			// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...			
			theHold.Suspend(); 
			delete tapeObject;
			tapeObject = NULL;
			theHold.Resume();
			// RB 7/28/97: If something has been put on the undo stack since this object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
				}
		} 
		else if ( tapeNode ) {
			 // Get rid of the reference.
			DeleteReference(0);  // sets tapeNode = NULL
		}
	}	
}

RefResult TapeHelpCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
{
	switch (message) {
		case REFMSG_PRENOTIFY_PASTE:
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
			}
		 	if ( tapeObject && tapeNode==hTarget ) {
				// this will set tapeNode== NULL;
				DeleteReference(0);
				goto endEdit;
			}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if ( tapeObject && tapeNode==hTarget ) {
				endEdit:
				tapeObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				tapeObject  = NULL;				
				tapeNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
			}
			else if (targNode==hTarget) {
				targNode = NULL;
				targObject = NULL;
			}
			break;		
	}
	return REF_SUCCEED;
}


void TapeHelpCreationManager::CreateNewObject()
	{
	tapeObject = (TapeHelpObject*)cDesc->Create();
	lastPutCount = theHold.GetGlobalPutCount();

	// Start the edit params process
	if ( tapeObject ) {

		tapeObject->SetAFlag(A_OBJ_LONG_CREATE);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		tapeObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
		}	
	}

static BOOL needToss;
#define DUMSZ 20.0f
			
int TapeHelpCreationManager::proc( 
				HWND hwnd,
				int msg,
				int point,
				int flag,
				IPoint2 m )
{	
	int res;
	TSTR targName;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );
	DWORD hideflags;

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					pt0 = m;
					assert( tapeObject );					
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					// if helpers were hidden by category, re-display them
					hideflags = GetCOREInterface()->GetHideByCategoryFlags();
					if(hideflags & (HIDE_HELPERS))
					{
						hideflags = hideflags & ~(HIDE_HELPERS);
						GetCOREInterface()->SetHideByCategoryFlags(hideflags);
						hideflags = GetCOREInterface()->GetHideByCategoryFlags();
					}

					if ( attachedToNode ) {
				   		// send this one on its way
				   		tapeObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
						
						// Get rid of the reference.
						if (tapeNode)
							DeleteReference(0);

						// new object
						CreateNewObject();   // creates tapeObject
						}

					needToss = theHold.GetGlobalPutCount()!=lastPutCount;

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					tapeNode = createInterface->CreateObjectNode( tapeObject);
					attachedToNode = TRUE;
					assert( tapeNode );					
					createCB = tapeObject->GetCreateMouseCallBack();					
					createInterface->SelectNode( tapeNode );
					
					// Create target object and node
//					targObject = new TargetObject;
					targObject = (Object*) createInterface->CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(TARGET_CLASS_ID,0));
					targObject->SetAFlag(A_OBJ_LONG_CREATE);
					assert(targObject);
					targNode = createInterface->CreateObjectNode( targObject);
					assert(targNode);
					targName = tapeNode->GetName();
					targName += GetString(IDS_DB_DOT_TARGET);
					targNode->SetName(targName);

					// hook up camera to target using lookat controller.
					createInterface->BindToTarget(tapeNode,targNode);

					// Reference the new node so we'll get notifications.
					MakeRefByID( FOREVER, 0, tapeNode);

					// Position camera and target at first point then drag.
					mat.IdentityMatrix();
					//mat[3] = vpx->GetPointOnCP(m);
					#ifdef _3D_CREATE
						mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
					#else
						mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
					#endif
					createInterface->SetNodeTMRelConstPlane(tapeNode, mat);
					createInterface->SetNodeTMRelConstPlane(targNode, mat);
					tapeObject->Enable(1);

				   	ignoreSelectionChange = TRUE;
				   	createInterface->SelectNode( targNode,0);
				   	ignoreSelectionChange = FALSE;
					res = TRUE;
					break;
					
				case 1:
					if (Length(m-pt0)<2)
						goto abort;
					//mat[3] = vpx->GetPointOnCP(m);
					#ifdef _3D_CREATE
						mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
					#else
						mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
					#endif
					createInterface->SetNodeTMRelConstPlane(targNode, mat);
				   	ignoreSelectionChange = TRUE;
				   	createInterface->SelectNode( tapeNode);
				   	ignoreSelectionChange = FALSE;
					
					createInterface->RedrawViews(createInterface->GetTime());  

				    theHold.Accept(IDS_DS_CREATE);	 
					targObject->ClearAFlag(A_OBJ_LONG_CREATE);

					res = FALSE;	// We're done
					break;
				}			
			break;

		case MOUSE_MOVE:
			//mat[3] = vpx->GetPointOnCP(m);
			#ifdef _3D_CREATE
				mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
			#else
				mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			#endif
			createInterface->SetNodeTMRelConstPlane(targNode, mat);
			createInterface->RedrawViews(createInterface->GetTime());	   
			res = TRUE;
			break;

		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
			#ifdef _OSNAP
			//Snap Preview
				#ifdef _3D_CREATE
					vpx->SnapPreview(m,m,NULL, SNAP_IN_3D);
				#else
					vpx->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
				#endif
			#endif
			break;

	    case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;
		
		case MOUSE_ABORT:
			abort:
			assert( tapeObject );
			tapeObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
			theHold.Cancel();	 // deletes both the camera and target.
			// Toss the undo stack if param changes have been made
			if (needToss) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			tapeNode = NULL;			
			targNode = NULL;	 	
			createInterface->RedrawViews(createInterface->GetTime()); 
			CreateNewObject();	
			attachedToNode = FALSE;
			res = FALSE;						
		}
	
	done:
	createInterface->ReleaseViewport(vpx); 
	return res;
}

int TapeHelpClassDesc::BeginCreate(Interface *i)
{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	
	//iob->SetMouseProc( new TapeHelpCreationManager(iob,this), 1000000 );

	theTapeHelpCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theTapeHelpCreateMode );
	
	return TRUE;
}

int TapeHelpClassDesc::EndCreate(Interface *i)
{
	ResumeSetKeyMode();
	theTapeHelpCreateMode.End();
	i->RemoveMode( &theTapeHelpCreateMode );

	return TRUE;
}


#define TAPE_SPEC_LEN_CHUNK		0x2680

// IO
IOResult TapeHelpObject::Save(ISave *isave) 
{
	if (specLenState) {
		isave->BeginChunk(TAPE_SPEC_LEN_CHUNK);
		isave->EndChunk();
	}
	return IO_OK;
}

IOResult TapeHelpObject::Load(ILoad *iload) 
{
	IOResult res;
	enable = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case TAPE_SPEC_LEN_CHUNK:
				specLenState = 1;
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}


#endif // NO_HELPER_TAPE

