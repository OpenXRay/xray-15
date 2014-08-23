/**********************************************************************
 *<
	FILE: prothelp.cpp

	DESCRIPTION:  Angle measuring helper implementation

	CREATED BY: Don Brittain

	HISTORY: created 12 June 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
#include "prothelp.h"

#ifndef NO_HELPER_PROTRACTOR // russom 10/16/01

//------------------------------------------------------

static BOOL creating = FALSE;

class ProtHelpClassDesc : public ClassDesc 
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ProtHelpObject; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_PROT_CLASS); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(PROTHELP_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	int 			BeginCreate(Interface *i)	{ 
						SuspendSetKeyMode();
						creating = TRUE;
						return ClassDesc::BeginCreate(i); }
	int 			EndCreate(Interface *i)	{ 
						ResumeSetKeyMode();
						creating = FALSE; 
						return ClassDesc::EndCreate(i); }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetProtParams(); }
};

static ProtHelpClassDesc ProtHelpDesc;

ClassDesc* GetProtHelpDesc() { return &ProtHelpDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for measuring tape class.
Mesh ProtHelpObject::mesh;
short ProtHelpObject::meshBuilt=0;
HWND ProtHelpObject::hProtHelpParams = NULL;
IObjParam *ProtHelpObject::iObjParams;

void resetProtParams() 
{
}

// Handles the work of actually picking the target.
class PickProtTarget : public PickModeCallback, PickNodeCallback 
{
public:
	int which;		// 0 or 1
	ProtHelpObject *ph;
	int doingPick;
	CommandMode *cm;
	HWND hDlg;
	PickProtTarget () {}

	BOOL HitTest (IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);
	BOOL Pick (IObjParam *ip, ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);

	// Allow right-clicking out of mode.
	BOOL RightClick (IObjParam *ip,ViewExp *vpt) { return TRUE; }
	
	// Is supposed to return a PickNodeCallback-derived class: we qualify!
	PickNodeCallback *GetFilter() {return this;}
	
	// PickNodeCallback methods:
	BOOL Filter(INode *node);
};

BOOL PickProtTarget::Filter(INode *node) 
{
	return node==NULL ? FALSE : TRUE;
}

BOOL PickProtTarget::HitTest (IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags) 
{	
	INode *node = ip->PickNode(hWnd, m, this);
	return Filter (node);
}

BOOL PickProtTarget::Pick (IObjParam *ip, ViewExp *vpt) 
{
	INode *node = vpt->GetClosestHit();
	assert(node);
	int res = TRUE;

	ph->ReplaceReference(which,node);
	/*
	if(node->MakeReference(FOREVER,ph) != REF_SUCCEED)
		ph->refNode[which] = NULL;
	else
		ph->refNode[which] = node;
		*/
	ph->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ph->Invalidate();
	SetDlgItemText(hDlg, which ? IDC_PROT_TARGET2_NAME : IDC_PROT_TARGET1_NAME, 
			ph->refNode[which] ? node->GetName() : _T(""));

	if(creating) {
		ph->BeginEditParams(ip, BEGIN_EDIT_CREATE, NULL);
		ip->SetCommandMode(cm);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	}
	return TRUE;
}

void PickProtTarget::EnterMode(IObjParam *ip) 
{
	ICustButton *iBut = GetICustButton(GetDlgItem(hDlg, which ? 
					IDC_PROT_PICK_TARGET2 : IDC_PROT_PICK_TARGET1));
	if (iBut) {
		iBut->SetCheck(TRUE);
		ReleaseICustButton(iBut);
	}
	doingPick = 1;
}

void PickProtTarget::ExitMode(IObjParam *ip) 
{
	ICustButton *iBut = GetICustButton(GetDlgItem(hDlg, which ? 
					IDC_PROT_PICK_TARGET2 : IDC_PROT_PICK_TARGET1));
	if (iBut) {
		iBut->SetCheck(FALSE);
		ReleaseICustButton(iBut);
	}
	doingPick = 0;
}

static PickProtTarget pickCB;

INT_PTR CALLBACK ProtHelpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	ICustButton *iBut;
	ProtHelpObject *ph = (ProtHelpObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	
	if ( !ph && message != WM_INITDIALOG ) return FALSE;

	switch ( message ) {
		case WM_INITDIALOG:
			ph = (ProtHelpObject *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)ph );
			SetDlgFont( hDlg, ph->iObjParams->GetAppHFont() );
			if( iBut = GetICustButton(GetDlgItem(hDlg, IDC_PROT_PICK_TARGET1))) {
				iBut->SetType (CBT_CHECK);
				iBut->SetHighlightColor (GREEN_WASH);
				ReleaseICustButton(iBut);
			}
			if( iBut = GetICustButton(GetDlgItem(hDlg, IDC_PROT_PICK_TARGET2))) {
				iBut->SetType (CBT_CHECK);
				iBut->SetHighlightColor (GREEN_WASH);
				ReleaseICustButton(iBut);
			}
			return FALSE;			

		case WM_DESTROY:
			return FALSE;

		case WM_MOUSEACTIVATE:
			ph->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			ph->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:			
			switch( LOWORD(wParam) ) {
			case IDC_PROT_PICK_TARGET1:
			case IDC_PROT_PICK_TARGET2:
				if(ph->created == FALSE) {
					if(iBut = GetICustButton(GetDlgItem(hDlg, IDC_PROT_PICK_TARGET1))) {
						iBut->SetCheck(FALSE);
						ReleaseICustButton(iBut);
					}
					if(iBut = GetICustButton(GetDlgItem(hDlg, IDC_PROT_PICK_TARGET2))) {
						iBut->SetCheck(FALSE);
						ReleaseICustButton(iBut);
					}
					break;
				}
				pickCB.ph = ph;
				pickCB.hDlg = hDlg;
				if(pickCB.doingPick) 
					ph->iObjParams->SetCommandMode(pickCB.cm);
				else
					pickCB.cm = ph->iObjParams->GetCommandMode();
				pickCB.which = ((LOWORD(wParam) - IDC_PROT_PICK_TARGET1) > 0);
				ph->iObjParams->SetPickMode (&pickCB);
				ph->UpdateUI(ph->iObjParams->GetTime());
				ph->iObjParams->RedrawViews (ph->iObjParams->GetTime());
				break;
			}
			return FALSE;

		default:
			return FALSE;
	}
}



void ProtHelpObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	iObjParams = ip;
	editting = TRUE;
	
	if ( !hProtHelpParams ) {
		hProtHelpParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PROTHELPER),
				ProtHelpParamDialogProc, 
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this );	
		ip->RegisterDlgWnd(hProtHelpParams);
	} 
	else {
		SetWindowLongPtr( hProtHelpParams, GWLP_USERDATA, (LONG_PTR)this );
	}
	UpdateUI(ip->GetTime());
}
		
void ProtHelpObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	editting = FALSE;

	if ( flags&END_EDIT_REMOVEUI ) {		
		ip->UnRegisterDlgWnd(hProtHelpParams);
		ip->DeleteRollupPage(hProtHelpParams);
		hProtHelpParams = NULL;				
	} 
	else {
		SetWindowLongPtr( hProtHelpParams, GWLP_USERDATA, 0 );
	}
	iObjParams = NULL;
}


void ProtHelpObject::BuildMesh()
{
	if(meshBuilt)
		return;
	int nverts = 7;
	int nfaces = 8;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	float d = 5.0f;
	float ds = 0.866f * d;
	float h = 2.0f * 0.866f * 5.0f;

	mesh.setVert(0, Point3(0.0f, 0.0f, 0.0f ));
	mesh.setVert(1, Point3(  -d,  -ds, -h ));
	mesh.setVert(2, Point3(   d,  -ds, -h ));
	mesh.setVert(3, Point3(0.0f,   ds, -h ));
	mesh.setVert(4, Point3(  -d,  -ds,  h ));
	mesh.setVert(5, Point3(   d,  -ds,  h ));
	mesh.setVert(6, Point3(0.0f,   ds,  h ));
	mesh.faces[0].setVerts(0, 1, 2);
	mesh.faces[0].setEdgeVisFlags(1,1,1);
	mesh.faces[1].setVerts(0, 2, 3);
	mesh.faces[1].setEdgeVisFlags(1,1,1);
	mesh.faces[2].setVerts(0, 3, 1);
	mesh.faces[2].setEdgeVisFlags(1,1,1);
	mesh.faces[3].setVerts(3, 2, 1);
	mesh.faces[3].setEdgeVisFlags(1,1,1);
	mesh.faces[4].setVerts(0, 5, 4);
	mesh.faces[4].setEdgeVisFlags(1,1,1);
	mesh.faces[5].setVerts(0, 6, 5);
	mesh.faces[5].setEdgeVisFlags(1,1,1);
	mesh.faces[6].setVerts(0, 4, 6);
	mesh.faces[6].setEdgeVisFlags(1,1,1);
	mesh.faces[7].setVerts(4, 5, 6);
	mesh.faces[7].setEdgeVisFlags(1,1,1);
#if 1
	// whoops- rotate 90 about x to get it facing the right way
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.RotateX(DegToRad(90.0));
	for (int i=0; i<nverts; i++)
		mesh.getVert(i) = mat*mesh.getVert(i);
#endif
//	mesh.buildNormals();
	meshBuilt = 1;
}

void ProtHelpObject::UpdateUI(TimeValue t)
{
	if ( hProtHelpParams &&	GetWindowLongPtr(hProtHelpParams,GWLP_USERDATA)==(LONG_PTR)this ) {
		TCHAR buf[256];

		if(refNode[0]) {
			GetDlgItemText(hProtHelpParams, IDC_PROT_TARGET1_NAME, buf, sizeof(buf));
			if(_tcscmp(buf, refNode[0]->GetName()))
				SetDlgItemText(hProtHelpParams, IDC_PROT_TARGET1_NAME, refNode[0]->GetName());
		}
		else
			SetDlgItemText(hProtHelpParams, IDC_PROT_TARGET1_NAME, _T(""));

		if(refNode[1]) {
			GetDlgItemText(hProtHelpParams, IDC_PROT_TARGET2_NAME, buf, sizeof(buf));
			if(_tcscmp(buf, refNode[1]->GetName()))
				SetDlgItemText(hProtHelpParams, IDC_PROT_TARGET2_NAME, refNode[1]->GetName());
		}
		else
			SetDlgItemText(hProtHelpParams, IDC_PROT_TARGET2_NAME, _T(""));

		if(refNode[0] && refNode[1])
			_stprintf(buf, _T("%g"), lastAngle);
		else
			buf[0] = _T('\0');
		SetDlgItemText(hProtHelpParams, IDC_PROT_ANGLE, buf);
	}
}


ProtHelpObject::ProtHelpObject() : HelperObject() 
{
	editting = 0;
	created = 0;
	refNode[0] = refNode[1] = NULL;
	suspendSnap = FALSE;
	ivalid.SetEmpty();
	BuildMesh();
}

ProtHelpObject::~ProtHelpObject()
{
	DeleteAllRefsFromMe();
}


int ProtHelpObject::NumRefs()
{
	return 2;
}

RefTargetHandle ProtHelpObject::GetReference(int i)
{
	return refNode[i];
}

void ProtHelpObject::SetReference(int i, RefTargetHandle rtarg)
{
	refNode[i] = (INode *)rtarg;
}

class ProtHelpObjCreateCallBack: public CreateMouseCallBack 
{
	ProtHelpObject *ph;
public:
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
	void SetObj(ProtHelpObject *obj) { ph = obj; }
};

int ProtHelpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{	
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0:
			ph->suspendSnap = TRUE;
			mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			break;
		case 1:
			mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			if (msg==MOUSE_POINT) {
				ph->suspendSnap = FALSE;
				return 0;
			}
			break;			
		}
	} else if (msg == MOUSE_ABORT) {		
		return CREATE_ABORT;
	}
	return 1;
}

static ProtHelpObjCreateCallBack protHelpCreateCB;

CreateMouseCallBack* ProtHelpObject::GetCreateMouseCallBack() 
{
	protHelpCreateCB.SetObj(this);
	return &protHelpCreateCB;
}

void ProtHelpObject::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm) 
{
	tm = inode->GetObjectTM(t);
	tm.NoScale();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

BOOL ProtHelpObject::GetTargetPoint(int which, TimeValue t, Point3 *pt)
{
	if(refNode[which]) {
		Matrix3 tm = refNode[which]->GetObjectTM(t);
		*pt = tm.GetTrans();
		return TRUE;
	}
	return FALSE;
}

void ProtHelpObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	box = mesh.getBoundingBox(tm);
}

void ProtHelpObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) 
{
	Matrix3 m = inode->GetObjectTM(t);
	Point3 pt;
	Point3 q[4];
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/360.0f;
	box = mesh.getBoundingBox();
	box.Scale(scaleFactor);

	if (GetTargetPoint(0, t, &pt))
		box += Inverse(m) * pt;
	if (GetTargetPoint(1, t, &pt))
		box += Inverse(m) * pt;
}

void ProtHelpObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	int i, nv;
	Matrix3 tm;
	Point3 pt;
	GetMat(t,inode,vpt,tm);
	nv = mesh.getNumVerts();
	box.Init();
	for (i=0; i<nv; i++) 
		box += tm*mesh.getVert(i);

	if (GetTargetPoint(0, t, &pt))
		box += pt;
	if (GetTargetPoint(1, t, &pt))
		box += pt;
}

int ProtHelpObject::DrawLines(TimeValue t, INode *inode, GraphicsWindow *gw, int drawing )
{
	Point3 pt[3];
	Matrix3 tm(TRUE);
	gw->setTransform(tm);
	gw->clearHitCode();
	tm = inode->GetObjectTM(t);
	pt[0] = tm.GetTrans();
	if(refNode[1]) {
		pt[1] = refNode[1]->GetObjectTM(t).GetTrans();
		gw->polyline(2, pt, NULL, NULL, FALSE, NULL);
	}
	if(refNode[0]) {
		pt[1] = refNode[0]->GetObjectTM(t).GetTrans();
		gw->setColor(LINE_COLOR, GetUIColor(COLOR_TARGET_LINE));
		gw->polyline(2, pt, NULL, NULL, FALSE, NULL);
	}
	if(gw->checkHitCode())
		pt[0].x = 1.0f;
	return gw->checkHitCode();
}

// From BaseObject

int ProtHelpObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
{
	HitRegion hitRegion;
	DWORD	savedLimits;
	int res;
	Matrix3 m;
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
	gw->clearHitCode();
	res = DrawLines(t, inode, gw, 1);
	gw->setRndLimits(savedLimits);
	return res;
}

void ProtHelpObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	if(suspendSnap)
		return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);

	Matrix3 invPlane = Inverse(snap->plane);

	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Point2 fp = Point2((float)p->x, (float)p->y);
		Point2 screen2;
		IPoint3 pt3;

		Point3 thePoint(0,0,0);
		// If constrained to the plane, make sure this point is in it!
		if(snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE) {
			Point3 test = thePoint * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				return;
		}
		gw->wTransPoint(&thePoint,&pt3);
		screen2.x = (float)pt3.x;
		screen2.y = (float)pt3.y;

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

int ProtHelpObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();

	created = TRUE;
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())
		gw->setColor( LINE_COLOR, GetUIColor(COLOR_TAPE_OBJ));
	mesh.render( gw, mtl, NULL, COMP_ALL);
	
	// calc angle
	lastAngle = 0.0;
	#define RadToDegDbl	(180.0 / 3.141592653589793)
	if(refNode[0] && refNode[1]) {
		Point3 origin = m.GetTrans();
		Point3 vec1 = refNode[0]->GetObjectTM(t).GetTrans() - origin;
		Point3 vec2 = refNode[1]->GetObjectTM(t).GetTrans() - origin;
		float len1 = Length(vec1);
		float len2 = Length(vec2);
		if(len1 > 0.00001f && len2 > 0.00001f) {
			double cosAng = (double)DotProd(vec1, vec2) / (double)(len1 * len2);
			if(fabs(cosAng) <= 0.999999)	// beyond float accuracy!
				lastAngle = acos(cosAng) * RadToDegDbl;
			else
				lastAngle = 180.0;
		}
	}
#if 0
	Point3 pt(0,0,0);
	TCHAR buf[32];
	_stprintf(buf, "%g", lastAngle);
	gw->setColor(TEXT_COLOR, GetUIColor(COLOR_TAPE_OBJ));
	gw->text(&pt, buf);
#endif
	DrawLines(t, inode, gw, 1);
	UpdateUI(t);
	return(0);
}


// From GeomObject
int ProtHelpObject::IntersectRay(TimeValue t, Ray& r, float& at) { return(0); }

//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult ProtHelpObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
{
	switch (message) {
	case REFMSG_CHANGE:
		if (iObjParams)
			UpdateUI(iObjParams->GetTime());
		ivalid.SetEmpty();
		break;
	case REFMSG_TARGET_DELETED:
		if (hTarget == refNode[0])
			refNode[0] = NULL;
		else if (hTarget == refNode[1])
			refNode[1] = NULL;
		if (iObjParams)
			UpdateUI(iObjParams->GetTime());
		break;
	}
	return REF_SUCCEED;
}

ObjectState ProtHelpObject::Eval(TimeValue time)
{
	return ObjectState(this);
}

void ProtHelpObject::Invalidate()
{
	ivalid.SetEmpty();
}

Interval ProtHelpObject::ObjectValidity(TimeValue t) 
{
	if (ivalid.Empty())
		ivalid = Interval(t,t);
	else {
		ivalid.SetInfinite();
		if(refNode[0])
			refNode[0]->GetNodeTM(t, &ivalid);
		if(refNode[1])
			refNode[1]->GetNodeTM(t, &ivalid);
		UpdateUI(t);
	}
	return ivalid;	
}

RefTargetHandle ProtHelpObject::Clone(RemapDir& remap) 
{
	ProtHelpObject* newob = new ProtHelpObject();
	newob->refNode[0] = refNode[0];
	newob->refNode[1] = refNode[1];
	BaseClone(this, newob, remap);
	return(newob);
}



// IO
IOResult ProtHelpObject::Save(ISave *isave) 
{
	return IO_OK;
}

IOResult ProtHelpObject::Load(ILoad *iload) 
{
#if 0
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
#endif
	return IO_OK;
}


#endif // NO_HELPER_PROTRACTOR
