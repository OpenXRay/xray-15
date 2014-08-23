/**********************************************************************
 *<
	FILE: target.cpp

	DESCRIPTION:  A Target object implementation

	CREATED BY: Dan Silva

	HISTORY: created 13 September 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"
#include "camera.h"
#include "target.h"

//------------------------------------------------------

class TargetObjectClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new TargetObject; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_TARGET); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID( TARGET_CLASS_ID, 0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES);  }
	};

static TargetObjectClassDesc targetObjDesc;

ClassDesc* GetTargetObjDesc() { return &targetObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *TargetObject::iObjParams;
Mesh TargetObject::mesh;		
int TargetObject::meshBuilt=0;


BOOL CALLBACK TargetParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	return 0;
	}


void TargetObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)	{}
		
void TargetObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )	{}

class TargetObjectCreateCallBack: public CreateMouseCallBack {
	TargetObject *ob;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TargetObject *obj) { ob = obj; }
	};

int TargetObjectCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 c;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				c = vpt->GetPointOnCP(m);
				mat.SetTrans(c);
				return CREATE_STOP;
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;

	return TRUE;
	}

static TargetObjectCreateCallBack boxCreateCB;

CreateMouseCallBack* TargetObject::GetCreateMouseCallBack() {
	boxCreateCB.SetObj(this);
	return(&boxCreateCB);
	}


static void MakeQuad(Face *f, int a, int b , int c , int d, int sg) {
	f[0].setVerts( a, b, c);
	f[0].setSmGroup(sg);
	f[0].setEdgeVisFlags(1,1,0);
	f[1].setVerts( c, d, a);
	f[1].setSmGroup(sg);
	f[1].setEdgeVisFlags(1,1,0);
	}

#define sz float(4.0)
void TargetObject::BuildMesh()
	{
	int nverts = 8;
	int nfaces = 12;
	Point3 va(-sz,-sz,-sz);
	Point3 vb( sz, sz, sz);
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	mesh.setVert(0, Point3( va.x, va.y, va.z));
	mesh.setVert(1, Point3( vb.x, va.y, va.z));
	mesh.setVert(2, Point3( va.x, vb.y, va.z));
	mesh.setVert(3, Point3( vb.x, vb.y, va.z));
	mesh.setVert(4, Point3( va.x, va.y, vb.z));
	mesh.setVert(5, Point3( vb.x, va.y, vb.z));
	mesh.setVert(6, Point3( va.x, vb.y, vb.z));
	mesh.setVert(7, Point3( vb.x, vb.y, vb.z));

	MakeQuad(&(mesh.faces[ 0]), 0,2,3,1,  1);
	MakeQuad(&(mesh.faces[ 2]), 2,0,4,6,  2);
	MakeQuad(&(mesh.faces[ 4]), 3,2,6,7,  4);
	MakeQuad(&(mesh.faces[ 6]), 1,3,7,5,  8);
	MakeQuad(&(mesh.faces[ 8]), 0,1,5,4, 16);
	MakeQuad(&(mesh.faces[10]), 4,5,7,6, 32);
	mesh.buildNormals();
	mesh.EnableEdgeList(1);
	}

TargetObject::TargetObject() : GeomObject() {
	if (!meshBuilt) {
		BuildMesh();
		meshBuilt = 1;
		}
	}

void TargetObject::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm) {
	tm = inode->GetObjectTM(t);
	tm.NoScale();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	if (scaleFactor!=(float)1.0)
		tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
	}

void TargetObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
	{
	box = mesh.getBoundingBox(tm);
	}

void TargetObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ){
	Matrix3 m = inode->GetObjectTM(t);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/(float)360.0;
	box = mesh.getBoundingBox();
	box.Scale(scaleFactor);
	}

void TargetObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
	int i,nv;
	Matrix3 m;
	GetMat(t,inode,vpt,m);
	nv = mesh.getNumVerts();
	box.Init();
	for (i=0; i<nv; i++) 
		box += m*mesh.getVert(i);
	}


// From BaseObject
int TargetObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {
	HitRegion hitRegion;
	DWORD savedLimits;
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	if(mesh.select( gw, gw->getMaterial(), &hitRegion, flags & HIT_ABORTONHIT ))
		return TRUE;
	gw->setRndLimits( savedLimits );
	return FALSE;

#if 0
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();
	gw->fWinMarker(&pt, HOLLOW_BOX_MRKR);
	return gw->checkHitCode();
#endif

	}

void TargetObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
		gw->setTransform(tm);

		Matrix3 invPlane = Inverse(snap->plane);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;

		Point3 thePoint(0,0,0);
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
			else
			if(len < snap->bestDist) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			}
		}
	}

int TargetObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent() && inode->GetLookatNode()) {
		const ObjectState& os = inode->GetLookatNode()->EvalWorldState(t);
		Object* ob = os.obj;

		// 6/25/01 3:32pm --MQM-- 
		// set color to wire-frame color, 
		// instead of COLOR_LIGHT_OBJ or COLOR_CAMERA_OBJ
		if ( (ob!=NULL) && ( (ob->SuperClassID()==LIGHT_CLASS_ID) ||
							 (ob->SuperClassID()==CAMERA_CLASS_ID) ) )
		{													
			Color color(inode->GetWireColor());
			gw->setColor( LINE_COLOR, color );
		}
		else
			gw->setColor( LINE_COLOR, GetUIColor(COLOR_CAMERA_OBJ)); // default target color, just use camera targ color
	}

	mesh.render( gw, gw->getMaterial(), NULL, COMP_ALL);	

//	gw->fWinMarker(&pt,HOLLOW_BOX_MRKR);
	return(0);
	}

// From Object
ObjectHandle TargetObject::ApplyTransform(Matrix3& matrix){
	return(ObjectHandle(this));
	}

// From GeomObject
int TargetObject::IntersectRay(TimeValue t, Ray& r, float& at) {
	return(0); 
	}

ObjectHandle TargetObject::CreateTriObjRep(TimeValue t) {
	return(NULL);
	}

//
// Reference Managment:
//


// This is only called if the object MAKES references to other things.
RefResult TargetObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
    {
#if 1
		switch ( message )
		{
		case REFMSG_NODE_WIRECOLOR_CHANGED:
			Beep( 1000, 500 );
		}
#endif
	return(REF_SUCCEED);
	}

ObjectState TargetObject::Eval(TimeValue time) {
	return ObjectState(this);
	}

RefTargetHandle TargetObject::Clone(RemapDir& remap) {
	TargetObject* newob = new TargetObject();
	BaseClone(this, newob, remap);
	return(newob);
	}


// IO
IOResult TargetObject::Save(ISave *isave) {
	return IO_OK;
	}

IOResult  TargetObject::Load(ILoad *iload) {
	return IO_OK;
	}
