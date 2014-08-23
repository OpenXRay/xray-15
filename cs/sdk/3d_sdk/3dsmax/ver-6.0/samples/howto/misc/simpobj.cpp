/**********************************************************************
 *<
	FILE: SimpObj.cpp

	DESCRIPTION:  A base class for procedural objects that fit into
	              a standard form.

	CREATED BY: Rolf Berteig

	HISTORY: created 10/10/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "core.h"
#include "control.h"
#include "paramblk.h"
#include "coremain.h"
#include "custcont.h"
#include "triobj.h"
#include "IParamM.h"
#include "mouseman.h"
#include "objmode.h"
#include "SimpObj.h"
#include "simpmod.h"
#include "patchobj.h"

//-- SimpleObject ---------------------------------------------------------

SimpleObject *SimpleObject::editOb = NULL;

SimpleObject::SimpleObject()
	{
	ivalid.SetEmpty();
	mesh.EnableEdgeList(1);
	SetAFlag(A_OBJ_CREATING);
	suspendSnap = FALSE;
	}

SimpleObject::~SimpleObject()
	{
	DeleteAllRefsFromMe();
	}

IParamArray *SimpleObject::GetParamBlock()
	{
	return pblock;
	}

int SimpleObject::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

void SimpleObject::UpdateMesh(TimeValue t) 
	{
	if (!ivalid.InInterval(t) ) {
		BuildMesh(t);
		}
	}

void SimpleObject::FreeCaches() 
	{
	ivalid.SetEmpty();
	mesh.FreeAll();
	} 

void SimpleObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
	{
	UpdateMesh(t);
	box = mesh.getBoundingBox(tm);
	}

void SimpleObject::GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) 
	{
	UpdateMesh(t);
	box = mesh.getBoundingBox();
	}

void SimpleObject::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box )
	{
	Matrix3 mat = inode->GetObjectTM(t);
	UpdateMesh(t);
	box = mesh.getBoundingBox();
	box = box * mat;
	}

void SimpleObject::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	editOb = this;
	}

void SimpleObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	editOb = NULL;
	ClearAFlag(A_OBJ_CREATING);
	}

// From BaseObject
int SimpleObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
	{
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	UpdateMesh(t);
	gw->setTransform(inode->GetObjectTM(t));
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	return mesh.select(gw, inode->Mtls(), &hitRegion, flags & HIT_ABORTONHIT, inode->NumMtls());
	}

void SimpleObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
	{
	if(suspendSnap)	// No snap to ourself while creating!
		return;

	Matrix3 tm = inode->GetObjectTM(t);
	GraphicsWindow *gw = vpt->getGW();
	UpdateMesh(t);
	gw->setTransform(tm);
	mesh.snap(gw, snap, p, tm);
	}

int SimpleObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
	{
	if (!OKtoDisplay(t)) return 0;
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 mat = inode->GetObjectTM(t);	 
	UpdateMesh(t);		
	gw->setTransform(mat);
	mesh.render(gw, inode->Mtls(), 
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, inode->NumMtls());
	return(0);
	}


// From GeomObject
int SimpleObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{	
	UpdateMesh(t);
	return mesh.IntersectRay(ray,at,norm);	
	}


RefResult SimpleObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			MeshInvalid();
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

ObjectState SimpleObject::Eval(TimeValue time) 
	{
	return ObjectState(this);
	}

Interval SimpleObject::ObjectValidity(TimeValue time) 
	{
	UpdateMesh(time);	
	return ivalid;	
	}

int SimpleObject::CanConvertToType(Class_ID obtype) 
	{
	if (obtype==defObjectClassID||obtype==mapObjectClassID||obtype==triObjectClassID||obtype==patchObjectClassID) {
		return 1;
		}	
	return Object::CanConvertToType(obtype);
	}

Object* SimpleObject::ConvertToType(TimeValue t, Class_ID obtype) 
	{
	if (obtype==defObjectClassID||obtype==triObjectClassID||obtype==mapObjectClassID) {
		TriObject *triob;
		UpdateMesh(t);
		triob = CreateNewTriObject();
		triob->GetMesh() = mesh;
		triob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
		triob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));		
		return triob;
		}
	else
	if (obtype == patchObjectClassID) {
		UpdateMesh(t);
		PatchObject *patchob = new PatchObject();
		patchob->patch = mesh;		// Handy Mesh->PatchMesh conversion
		patchob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
		patchob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));		
		return patchob;
		}
	return Object::ConvertToType(t,obtype);
	}

Mesh* SimpleObject::GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) {
	needDelete = FALSE;
	UpdateMesh(t);
	return &mesh;
	}

TSTR SimpleObject::SubAnimName(int i) 
	{ 
	return TSTR(GetResString(IDS_RB_PARAMETERS));
	}


BOOL 
SimpleObject::PolygonCount(TimeValue t, int& numFaces, int& numVerts) 
{
    UpdateMesh(t);
    numFaces = mesh.getNumFaces();
    numVerts = mesh.getNumVerts();
    return TRUE;
}

//-- SimpleWSMObject ---------------------------------------------------------

SimpleWSMObject *SimpleWSMObject::editOb = NULL;

SimpleWSMObject::SimpleWSMObject()
	{
	ivalid.SetEmpty();
	mesh.EnableEdgeList(1);
	SetAFlag(A_OBJ_CREATING);
	}

SimpleWSMObject::~SimpleWSMObject()
	{
	DeleteAllRefsFromMe();
	}

IParamArray *SimpleWSMObject::GetParamBlock()
	{
	return pblock;
	}

int SimpleWSMObject::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

void SimpleWSMObject::UpdateMesh(TimeValue t) 
	{
	if (!ivalid.InInterval(t) ) {
		BuildMesh(t);
		}
	}

void SimpleWSMObject::FreeCaches() 
	{
	ivalid.SetEmpty();
	mesh.FreeAll();
	} 

void SimpleWSMObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
	{
	UpdateMesh(t);
	box = mesh.getBoundingBox(tm);
	}

void SimpleWSMObject::GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) 
	{
	UpdateMesh(t);
	box = mesh.getBoundingBox();
	}

void SimpleWSMObject::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box )
	{
	Matrix3 mat = inode->GetObjectTM(t);
	UpdateMesh(t);
	box = mesh.getBoundingBox();
	box = box * mat;
	}

void SimpleWSMObject::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	editOb = this;
	}

void SimpleWSMObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	editOb = NULL;
	ClearAFlag(A_OBJ_CREATING);
	}

// From BaseObject
int SimpleWSMObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
	{
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();
		
	UpdateMesh(t);
	gw->setTransform(inode->GetObjectTM(t));
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	DWORD rlim  = gw->getRndLimits();	
	int res;

	gw->setRndLimits((rlim|GW_PICK|GW_WIREFRAME) 
		& ~(GW_ILLUM|GW_BACKCULL|GW_FLAT|GW_SPECULAR));

	res = mesh.select(gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
	
	gw->setRndLimits(rlim);
	return res;
	}

void SimpleWSMObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
	{
	if(TestAFlag(A_OBJ_CREATING))	// No snap to ourself while creating!
		return;

	Matrix3 tm = inode->GetObjectTM(t);
	GraphicsWindow *gw = vpt->getGW();
	UpdateMesh(t);
	gw->setTransform(tm);
	mesh.snap(gw, snap, p, tm);
	}


class WSMMtl: public Material {
	public:
	WSMMtl();
	};
static WSMMtl wsmMtl;

#define WSM_R	float(1.0)
#define WSM_G	float(1.0)
#define WSM_B	float(0.0)

WSMMtl::WSMMtl():Material() {
	Kd[0] = WSM_R;
	Kd[1] = WSM_G;
	Kd[2] = WSM_B;
	Ks[0] = WSM_R;
	Ks[1] = WSM_G;
	Ks[2] = WSM_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME;
	selfIllum = (float)1.0;
	}

int SimpleWSMObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
	{
	if (!OKtoDisplay(t)) return 0;
	GraphicsWindow *gw = vpt->getGW();	
	Matrix3 mat = inode->GetObjectTM(t);	 
	UpdateMesh(t);		
	gw->setTransform(mat);

	//mesh.render(gw, inode->Mtls(), 
	DWORD rlim  = gw->getRndLimits();
	
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/(rlim&GW_Z_BUFFER?GW_Z_BUFFER:0));
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		//gw->setColor( LINE_COLOR, wsmMtl.Kd[0], wsmMtl.Kd[1], wsmMtl.Kd[2]);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SPACE_WARPS));

	mesh.render(gw, &wsmMtl, 
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);
	
	gw->setRndLimits(rlim);
	return(0);
	}


RefResult SimpleWSMObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			MeshInvalid();
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

ObjectState SimpleWSMObject::Eval(TimeValue time) 
	{
	return ObjectState(this);
	}

Interval SimpleWSMObject::ObjectValidity(TimeValue time) 
	{
	UpdateMesh(time);	
	return ivalid;	
	}

TSTR SimpleWSMObject::SubAnimName(int i) 
	{ 
	return TSTR(GetResString(IDS_RB_PARAMETERS));
	}


//-- SimpleParticle ---------------------------------------------------------

SimpleParticle *SimpleParticle::editOb = NULL;
IObjParam *SimpleParticle::ip          = NULL;

SimpleParticle::SimpleParticle()
	{	
	pblock = NULL;
	valid  = FALSE;
	mvalid.SetEmpty();
	tvalid=mvalid.Start();
	}

SimpleParticle::~SimpleParticle()
	{
	DeleteAllRefsFromMe();
	}

IParamArray *SimpleParticle::GetParamBlock()
	{
	return pblock;
	}

int SimpleParticle::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

TimeValue SimpleParticle::ParticleAge(TimeValue t, int i)
	{
	if (i<parts.ages.Count()) return parts.ages[i];
	else return -1;
	}

void SimpleParticle::SetParticlePosition(TimeValue t, int i, Point3 pos)
	{
	if (i<parts.points.Count()) parts.points[i]=pos;
	}

void SimpleParticle::SetParticleVelocity(TimeValue t, int i, Point3 vel)
	{
	if (i<parts.vels.Count()) parts.vels[i]=vel;
	}

void SimpleParticle::SetParticleAge(TimeValue t, int i, TimeValue age)
	{
	if (i<parts.ages.Count()) parts.ages[i]=age;
	}

void SimpleParticle::Update(TimeValue t,INode *node)
	{
	if (node) {
		if (tvalid!=t || !valid) UpdateParticles(t,node);
		}
	UpdateMesh(t);
	}

void SimpleParticle::UpdateMesh(TimeValue t) 
	{
	if (!mvalid.InInterval(t) ) {
		BuildEmitter(t,mesh);
		}
	}
	
void SimpleParticle::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
	{
	Update(t);
	box = mesh.getBoundingBox(tm);
	}

void SimpleParticle::GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) 
	{
	Matrix3 mat = inode->GetObjTMBeforeWSM(t);
	Box3 pbox;
	Update(t,inode);
	box  = mesh.getBoundingBox();
	pbox = parts.BoundBox();
	if (!pbox.IsEmpty()) box += pbox * Inverse(mat);
	}

void SimpleParticle::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box )
	{
	Box3 pbox;
	Matrix3 mat = inode->GetObjTMBeforeWSM(t);	
	UpdateMesh(t);
	box  = mesh.getBoundingBox();
	box  = box * mat;
	pbox = parts.BoundBox();	
	if (!pbox.IsEmpty()) box += pbox;	
	}

void SimpleParticle::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editOb   = this;
	}

void SimpleParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editOb   = NULL;	
	}


static Matrix3 ident(1);

class ParticleMtl: public Material {
	public:
	ParticleMtl();
	};
static ParticleMtl particleMtl;

#define PARTICLE_R	float(1.0)
#define PARTICLE_G	float(1.0)
#define PARTICLE_B	float(0.0)

ParticleMtl::ParticleMtl():Material() {
	Kd[0] = PARTICLE_R;
	Kd[1] = PARTICLE_G;
	Kd[2] = PARTICLE_B;
	Ks[0] = PARTICLE_R;
	Ks[1] = PARTICLE_G;
	Ks[2] = PARTICLE_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME;
	selfIllum = (float)1.0;
	}


int SimpleParticle::HitTest(
		TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt) 
	{	
	Update(t,inode);
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	DWORD rlim  = gw->getRndLimits();
	int res;

	gw->setTransform(ident);
	if (parts.HitTest(gw,&hitRegion,flags&HIT_ABORTONHIT,GetMarkerType())) {
		return TRUE;
		}
	
	if (EmitterVisible()) {
		gw->setRndLimits((rlim|GW_PICK|GW_WIREFRAME) 
			& ~(GW_ILLUM|GW_BACKCULL|GW_FLAT|GW_SPECULAR));
		gw->setTransform(inode->GetObjTMBeforeWSM(t));
		res = mesh.select(gw, &particleMtl, &hitRegion, flags & HIT_ABORTONHIT);

		gw->setRndLimits(rlim);
	} else {
		res = 0;
		}
	return res;
	}


int SimpleParticle::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
	{
	if (!OKtoDisplay(t)) return 0;
	Update(t,inode);
	GraphicsWindow *gw = vpt->getGW();
	DWORD rlim  = gw->getRndLimits();	
	
	// Draw emitter
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL| (rlim&GW_Z_BUFFER) );
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		//gw->setColor( LINE_COLOR, particleMtl.Kd[0], particleMtl.Kd[1], particleMtl.Kd[2]);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_PARTICLE_EM));

	if (EmitterVisible()) {
		gw->setTransform(inode->GetObjTMBeforeWSM(t));	
		mesh.render(gw, &particleMtl, 
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);
		}
			
	// Draw particles
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL| (rlim&(GW_Z_BUFFER|GW_BOX_MODE)) );
	Material *mtl = gw->getMaterial();	
	if (!inode->Selected() && !inode->IsFrozen())
		gw->setColor( LINE_COLOR, mtl->Kd[0], mtl->Kd[1], mtl->Kd[2]);

	gw->setTransform(ident);
	parts.Render(gw,GetMarkerType());

	gw->setRndLimits(rlim);
	return(0);
	}


RefResult SimpleParticle::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			MeshInvalid();
			ParticleInvalid();
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

void* SimpleParticle::GetInterface(ULONG id)
	{
	if (id==I_PARTICLEOBJ) {
		return this;
	} else {
		return ParticleObject::GetInterface(id);
		}
	}

ObjectState SimpleParticle::Eval(TimeValue time) 
	{
	// Clear the force fields
	fields.Resize(0);
	cobjs.Resize(0);
	return ObjectState(this);
	}

Object *SimpleParticle::MakeShallowCopy(ChannelMask channels)
	{	
	return this;
	}

void SimpleParticle::ApplyForceField(ForceField *ff)
	{
	fields.Append(1,&ff);
	}

BOOL SimpleParticle::ApplyCollisionObject(CollisionObject *co)
	{
	cobjs.Append(1,&co);
	return TRUE;
	}

Interval SimpleParticle::ObjectValidity(TimeValue time) 
	{
	Interval valid = GetValidity(time);
	UpdateMesh(time);
	return mvalid & valid;
	}

int SimpleParticle::CanConvertToType(Class_ID obtype) 
	{
	if (obtype==defObjectClassID) {
		return 1;
		}
	return Object::CanConvertToType(obtype);
	}

Object* SimpleParticle::ConvertToType(TimeValue t, Class_ID obtype) 
	{
	if (obtype==defObjectClassID) {
		return this;
	} else {
		return Object::ConvertToType(t,obtype);
		}
	}

TSTR SimpleParticle::SubAnimName(int i) 
	{ 
	return TSTR(GetResString(IDS_RB_PARAMETERS));
	}

//---SimpleOSMToWSMObject---------------------------------------------

IParamMap *SimpleOSMToWSMObject::pmapParam = NULL;

ParamBlockDescID descObjVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 }};

ParamBlockDescID descObjVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 }};

#define PBLOCK_LENGTH 4

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descObjVer0,3,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descObjVer1,PBLOCK_LENGTH,CURRENT_VERSION);


static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_OSMTOWSM_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_OSMTOWSM_LENGTH,IDC_OSMTOWSM_LENGTHSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),

	// Width
	ParamUIDesc(
		PB_OSMTOWSM_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_OSMTOWSM_WIDTH,IDC_OSMTOWSM_WIDTHSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_OSMTOWSM_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_OSMTOWSM_HEIGHT,IDC_OSMTOWSM_HEIGHTSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),

	// Decay
	ParamUIDesc(
		PB_OSMTOWSM_DECAY,
		EDITTYPE_FLOAT,
		IDC_OSMTOWSM_DECAY,IDC_OSMTOWSM_DECAYSPIN,
		0.0f,float(1.0E30),
		0.0001f),
	};
#define PARAMDESC_LENGH 4


SimpleOSMToWSMObject::SimpleOSMToWSMObject()
	{
	mod    = NULL;
	pblock = NULL;
	ivalid.SetEmpty();	
	}

SimpleOSMToWSMObject::SimpleOSMToWSMObject(SimpleMod *m)
	{
	mod    = NULL;
	pblock = NULL;
	ivalid.SetEmpty();
	MakeRefByID(FOREVER,0,m);
	MakeRefByID(FOREVER, 1, 
		CreateParameterBlock(descObjVer1, PBLOCK_LENGTH, 1));
	}

SimpleOSMToWSMObject* SimpleOSMToWSMObject::SimpleOSMToWSMClone(
		SimpleOSMToWSMObject *from,RemapDir& remap)
	{
	ReplaceReference(1,(ReferenceTarget*)remap.CloneRef(from->pblock));
	return this;
	}

IOResult SimpleOSMToWSMObject::Load(ILoad *iload)
	{	
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,1));
	return IO_OK;
	}

#define MESH_SEGMENTS	10

static void MakeLine(Face *f, int a,  int b)
	{
	f->setVerts(a,b,0);
	f->setSmGroup(0);
	f->setEdgeVisFlags(1,0,0);
	}

void SimpleOSMToWSMObject::BuildMesh(TimeValue t)
	{
	float l, w, h;
	ivalid = FOREVER;
	pblock->GetValue(PB_OSMTOWSM_LENGTH,t,l,ivalid);
	pblock->GetValue(PB_OSMTOWSM_WIDTH,t,w,ivalid);
	pblock->GetValue(PB_OSMTOWSM_HEIGHT,t,h,ivalid);	
	w = w/2.0f;
	l = l/2.0f;

	float zmin, zmax;
	int axis;
	BOOL limit = mod->GetModLimits(t,zmin,zmax,axis);
	if (limit) {
		mesh.setNumVerts(20*(MESH_SEGMENTS+1)+1);
		mesh.setNumFaces(20*MESH_SEGMENTS);
	} else {
		mesh.setNumVerts(12*(MESH_SEGMENTS+1)+1);
		mesh.setNumFaces(12*MESH_SEGMENTS);
		}
	
	int vi=0, fi=0;
	mesh.setVert(vi++, Point3(0.0f,0.0f,h/2.0f));

	Point3 data1[] = {
		Point3(-w, -l, 0.0f),
		Point3( w, -l, 0.0f),
		Point3( w,  l, 0.0f),
		Point3(-w,  l, 0.0f),
		Point3(-w, -l,    h),
		Point3( w, -l,    h),
		Point3( w,  l,    h),
		Point3(-w,  l,    h)};
	
	for (int j=0; j<4; j++) {		
		for (int i=0; i<=MESH_SEGMENTS; i++) {
			float u = float(i)/float(MESH_SEGMENTS);
			mesh.setVert(vi++, (1.0f-u)*data1[j] + u*data1[j+4]);
			if (i) MakeLine(&mesh.faces[fi++], vi-2, vi-1);
			}		
		}

	Point3 data2[] = {
		Point3(-w, -l, 0.0f),			
		Point3(-w,  l, 0.0f),
		Point3(-w,  l,    h),
		Point3(-w, -l,    h),
		Point3( w, -l, 0.0f),			
		Point3( w,  l, 0.0f),
		Point3( w,  l,    h),
		Point3( w, -l,    h)};

	for (j=0; j<4; j++) {		
		for (int i=0; i<=MESH_SEGMENTS; i++) {
			float u = float(i)/float(MESH_SEGMENTS);
			mesh.setVert(vi++, (1.0f-u)*data2[j] + u*data2[j+4]);
			if (i) MakeLine(&mesh.faces[fi++], vi-2, vi-1);
			}		
		}

	Point3 data3[] = {
		Point3(-w, -l, 0.0f),			
		Point3( w, -l, 0.0f),
		Point3( w, -l,    h),
		Point3(-w, -l,    h),
		Point3(-w,  l, 0.0f),			
		Point3( w,  l, 0.0f),
		Point3( w,  l,    h),
		Point3(-w,  l,    h)};

	for (j=0; j<4; j++) {		
		for (int i=0; i<=MESH_SEGMENTS; i++) {
			float u = float(i)/float(MESH_SEGMENTS);
			mesh.setVert(vi++, (1.0f-u)*data3[j] + u*data3[j+4]);
			if (i) MakeLine(&mesh.faces[fi++], vi-2, vi-1);
			}		
		}	

	if (limit) {
		Point3 data3[] = {
			Point3(-w, -l, 0.0f),
			Point3( w, -l, 0.0f),
			Point3( w,  l, 0.0f),
			Point3(-w,  l, 0.0f)};
		Point3 data1[] = {
			Point3(-l, 0.0f, 0.0f),
			Point3( l, 0.0f, 0.0f),
			Point3( l,    h, 0.0f),
			Point3(-l,    h, 0.0f)};
		Point3 data2[] = {
			Point3(0.0f ,-w, 0.0f),
			Point3(0.0f,  w, 0.0f),
			Point3(   h,  w, 0.0f),
			Point3(   h, -w, 0.0f)};
		Point3 *data[] = {data1,data2,data3};

		// Min limit
		for (j=0; j<4; j++) {
			for (int i=0; i<=MESH_SEGMENTS; i++) {
				float u = float(i)/float(MESH_SEGMENTS);
				Point3 p1, p2;
				p1[(axis+1)%3] = data[axis][j].x;
				p1[(axis+2)%3] = data[axis][j].y;
				p1[axis]       = zmin;
				p2[(axis+1)%3] = data[axis][(j+1)%4].x;
				p2[(axis+2)%3] = data[axis][(j+1)%4].y;
				p2[axis]       = zmin;				

				mesh.setVert(vi++, (1.0f-u)*p1 + u*p2);
				if (i) MakeLine(&mesh.faces[fi++], vi-2, vi-1);
				}		
			}
		// Max limit
		for (j=0; j<4; j++) {
			for (int i=0; i<=MESH_SEGMENTS; i++) {
				float u = float(i)/float(MESH_SEGMENTS);
				Point3 p1, p2;
				p1[(axis+1)%3] = data[axis][j].x;
				p1[(axis+2)%3] = data[axis][j].y;
				p1[axis]       = zmax;
				p2[(axis+1)%3] = data[axis][(j+1)%4].x;
				p2[(axis+2)%3] = data[axis][(j+1)%4].y;
				p2[axis]       = zmax;				

				mesh.setVert(vi++, (1.0f-u)*p1 + u*p2);
				if (i) MakeLine(&mesh.faces[fi++], vi-2, vi-1);
				}		
			}	
		}

	Matrix3 tm(1);
	ModContext mc;
	mc.tm = NULL;
	mc.box = new Box3;
	mc.box->pmin = Point3(-w,-l,0.0f);
	mc.box->pmax = Point3( w, l,h);
	Deformer &def = GetDecayDeformer(t,mod->GetDeformer(t,mc,tm,tm),Point3(0,0,0),ivalid);
	for (int i=0; i<mesh.getNumVerts(); i++) {
		mesh.verts[i] = def.Map(i, mesh.verts[i]);
		}
	ivalid &= mod->GetValidity(t);
	mesh.InvalidateGeomCache();
	}

	class DecayDeformer : public Deformer {
	public:
		Deformer *mdef;
		Point3 origin;
		float decay;
		DecayDeformer() {mdef=NULL;}
		DecayDeformer(Deformer *md,Point3 o,float d) {origin=o;mdef=md;decay=d;}
		Point3 Map(int i, Point3 p) {
			if (decay>0.0f) {
				float dist = Length(p-origin);
				Point3 pd = mdef->Map(i,p);
				float u = (float)exp(-dist*decay);
				return pd*u + p*(1.0f-u);
			} else {
				return mdef->Map(i,p);
				}
			}
	};
static DecayDeformer theDecayDef;

Deformer &SimpleOSMToWSMObject::GetDecayDeformer(
		TimeValue t,Deformer &mdef,Point3 origin,Interval &iv)
	{
	float d;
	pblock->GetValue(PB_OSMTOWSM_DECAY,t,d,iv);
	theDecayDef = DecayDeformer(&mdef,origin,d);
	return theDecayDef;
	}

Modifier *SimpleOSMToWSMObject::CreateWSMMod(INode *node)
	{
	return new SimpleOSMToWSMMod(node);
	}

RefTargetHandle SimpleOSMToWSMObject::GetReference(int i) 
	{
	switch (i) {
		case 0: return mod;
		case 1: return pblock;
		}
	return NULL;
	}

void SimpleOSMToWSMObject::SetReference(int i, RefTargetHandle rtarg) 
	{
	switch (i) {
		case 0: mod = (SimpleMod*)rtarg; break;
		case 1: pblock = (IParamBlock*)rtarg; break;
		}
	}

Animatable* SimpleOSMToWSMObject::SubAnim(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return mod->pblock;
		default: return NULL;
		}
	}

TSTR SimpleOSMToWSMObject::SubAnimName(int i)
	{
	return GetResString(IDS_RB_PARAMETERS);
	}

TSTR SimpleOSMToWSMObject::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {	
		case PB_OSMTOWSM_LENGTH: return GetResString(IDS_RB_LENGTH);
		case PB_OSMTOWSM_WIDTH:  return GetResString(IDS_RB_WIDTH);
		case PB_OSMTOWSM_HEIGHT: return GetResString(IDS_RB_HEIGHT);
		case PB_OSMTOWSM_DECAY:  return GetResString(IDS_RB_DECAY);
		default: return GetResString(IDS_RB_PARAMETERS);
		}
	}

class SimpleOSMToWSMObjectCreateCallBack: public CreateMouseCallBack {
	SimpleOSMToWSMObject *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;	
	public:
		int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(SimpleOSMToWSMObject *obj) {ob = obj;}
	};

#define BOTTOMPIV

int SimpleOSMToWSMObjectCreateCallBack::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat) 
	{
	Point3 d;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				ob->pblock->SetValue(PB_OSMTOWSM_WIDTH,0,0.0f);
				ob->pblock->SetValue(PB_OSMTOWSM_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_OSMTOWSM_HEIGHT,0,0.0f);
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
#ifdef BOTTOMPIV
				{
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);
				}
#endif
				break;
			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1.z = p0.z +(float).01; 
				
				mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV
				{
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);					
				}
#endif
				d = p1-p0;
								
				ob->pblock->SetValue(PB_OSMTOWSM_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_OSMTOWSM_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_OSMTOWSM_HEIGHT,0,float(fabs(d.z)));
				ob->pmapParam->Invalidate();										

				if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
					}
				break;
			case 2:
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
				
				mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV
				mat.SetTrans(2,p0.z); // set the Z component of translation
#endif					
				d = p1-p0;
				
				ob->pblock->SetValue(PB_OSMTOWSM_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_OSMTOWSM_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_OSMTOWSM_HEIGHT,0,float(d.z));
				ob->pmapParam->Invalidate();				
					
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

static SimpleOSMToWSMObjectCreateCallBack createCB;


CreateMouseCallBack* SimpleOSMToWSMObject::GetCreateMouseCallBack()
	{
	createCB.SetObj(this);
	return &createCB;
	}

void SimpleOSMToWSMObject::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	if (pmapParam) {
		pmapParam->SetParamBlock(pblock);
	} else {
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			getResMgr().getHInst(RES_ID_RB),
			MAKEINTRESOURCE(IDD_OSMTOWSMPARAMS),
			GetResString(IDS_RB_GIZMOPARAMS),
			0);
		}
	mod->BeginEditParams(ip,flags,prev);
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(NULL,0);
	}

void SimpleOSMToWSMObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	SimpleWSMObject::EndEditParams(ip,flags,next);
	mod->EndEditParams(ip,flags,next);
	if (flags&END_EDIT_REMOVEUI) {
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		}
	}

void SimpleOSMToWSMObject::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}
