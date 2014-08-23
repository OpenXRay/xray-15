/**********************************************************************
 *<
	FILE: sphered.cpp

	DESCRIPTION: A simple spherical deflector object for particles

	CREATED BY: Audrey Peterson

	HISTORY: 19November 96

    PB2 ECP 3/14/00

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "suprprts.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "icollision.h"
#include "macrorec.h"

static Class_ID SPHEREDEF_CLASS_ID(0x6cbd289d, 0x3fef6656);
static Class_ID SPHEREDEFMOD_CLASS_ID(0x5cdf4181, 0x4c5b42f9);

class SphereDefObject : public SimpleWSMObject2 {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		int lastrnd;
		TimeValue t;
					
		SphereDefObject();
		~SphereDefObject();

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		Class_ID ClassID() {return SPHEREDEF_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_SPHDEFLECTOR);}
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);		
		
		IOResult Load(ILoad *iload);
		CollisionObject *GetCollisionObject(INode *node);
		void SetRandSeed(int seed);

		// From WSMObject
		Modifier *CreateWSMMod(INode *node);		

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *SphereDefObject::ip        = NULL;
//IParamMap *SphereDefObject::pmapParam = NULL;
HWND       SphereDefObject::hSot      = NULL;

class SphereDeflectorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new SphereDefObject;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_SPHDEFLECTOR_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return SPHEREDEF_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("SDeflector"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static SphereDeflectorClassDesc SpheredeflectDesc;
ClassDesc* GetSphereDefDesc() {return &SpheredeflectDesc;}

//--- DeflectMod -----------------------------------------------------

class SphereDeflectorField : public CollisionObject {
	public:		
//watje
		SphereDeflectorField()
			{
			srand(7896764);
			for (int i =0;i < 500; i++)
				{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
				}
			cols=NULL;
			}

		void DeleteThis() 
			{	 
			if (cols) cols->DeleteThis();
			cols=NULL;
			delete this;

			}

		~SphereDeflectorField()
		{
			if (cols) cols->DeleteThis();
			cols=NULL;
		}

//watje moved these here since they don't need to be accessed for every particle
		float radius,chaos,bounce,bvar,vinher,friction;
//using a table of rnadmom floats now
		float randomFloat[500];

		SphereDefObject *obj;
		INode *node;
		Matrix3 tm, invtm,tp;
		Interval tmValid;
		Point3 Vc,Vcp;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
		void SetRandSeed(int seed) { if (obj != NULL) obj->lastrnd = seed; }
		CollisionSphere *cols;
	};

class SphereDeflectMod : public SimpleWSMMod {
	public:				
		SphereDeflectorField deflect;

		SphereDeflectMod() {}
		SphereDeflectMod(INode *node,SphereDefObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_SPHDEFLECTMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return SPHEREDEFMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_SPHDEFBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class SphereDeflectorModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new SphereDeflectMod;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_SPHDEFLECTMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return SPHEREDEFMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static SphereDeflectorModClassDesc SpheredeflectModDesc;
ClassDesc* GetSphereDefModDesc() {return &SpheredeflectModDesc;}

enum 
{	sdeflectrobj_params, 
}; 

enum 
{	sdeflectrobj_bounce, 
	sdeflectrobj_bouncevar,
	sdeflectrobj_chaos,
	sdeflectrobj_radius,
	sdeflectrobj_velocity,
	sdeflectrobj_friction,
//watje ref to hold the collision engine
	sdeflectrobj_collider
};

#define PBLOCK_REF_NO	0

static ParamBlockDesc2 sdeflector_param_blk 
(	sdeflectrobj_params, 
	_T("SDeflectorParameters"),  
	0, 
	&SpheredeflectDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 
	PBLOCK_REF_NO, 

	//rollout
	IDD_SW_SPHEREDEFL, IDS_RB_PARAMETERS, 0, 0, NULL, 

	// params
	sdeflectrobj_bounce,	_T("bounce"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_BOUNCE,
		p_default,			1.0f,
		p_range,			0.0f,65535.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_UDEFL_MULT2,				IDC_UDEFL_MULTSPIN2, 0.01f,
		end,

	sdeflectrobj_bouncevar, _T("bouncevar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_BVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_UDEFL_MULTVAR,			IDC_UDEFL_MULTVARSPIN, 1.0f,
		end,

	sdeflectrobj_chaos,		_T("chaos"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_CHAOS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_UDEFL_CHAOS,			IDC_UDEFL_CHAOSSPIN,	1.0f,
		end,

	sdeflectrobj_radius,	_T("radius"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_ICONSIZED,
		p_default,			0.0f,
		p_ms_default,		10.0f,
		p_range,			0.0f,9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_SP_UDEFL_ICONSIZE,		IDC_SP_UDEFL_ICONSIZESPIN,	SPIN_AUTOSCALE,
		end,

	sdeflectrobj_velocity,	_T("inheritVelocity"),	TYPE_FLOAT,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_VELOCITY,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SP_UDEFL_VELINH,		IDC_SP_UDEFL_VELINHSPIN,	0.01f,
		end,

	sdeflectrobj_friction,	_T("friction"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_FRICTION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SDEFLECT_FRICTION,		IDC_SDEFLECT_FRICTIONSPIN, 1.0f,
		end,
//watje ref to hold the collision engine
	sdeflectrobj_collider,  _T(""),	TYPE_REFTARG, 	0, 	0, 
		end, 

		end
);

/*
//--- SphereDefObject Parameter map/block descriptors ------------------

#define PB_BOUNCE	 0
#define PB_BVAR		 1
#define PB_CHAOS	 2
#define PB_RADIUS	 3
#define PB_VELOCITY	 4

static ParamUIDesc descParam[] = {
	// Bounce
	ParamUIDesc(
		PB_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_UDEFL_MULT,IDC_UDEFL_MULTSPIN,
		0.0f, 9999999.0f,
		0.01f),
	
	// Bounce Var
	ParamUIDesc(
		PB_BVAR,
		EDITTYPE_FLOAT,
		IDC_UDEFL_MULTVAR,IDC_UDEFL_MULTVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),
	
	// Chaos
	ParamUIDesc(
		PB_CHAOS,
		EDITTYPE_FLOAT,
		IDC_UDEFL_CHAOS,IDC_UDEFL_CHAOSSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),
		
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_SP_UDEFL_ICONSIZE,IDC_SP_UDEFL_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Velocity Inheritance
	ParamUIDesc(
		PB_VELOCITY,
		EDITTYPE_FLOAT,
		IDC_SP_UDEFL_VELINH,IDC_SP_UDEFL_VELINHSPIN,
		0.0f,1000.0f,SPIN_AUTOSCALE),

	};
*/

//#define PARAMDESC_LENGTH	6

ParamBlockDescID SphDefdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	//Bounce
	{ TYPE_FLOAT, NULL, FALSE, 1 },	//Bounce Var
	{ TYPE_FLOAT, NULL, TRUE, 2 },	//Chaos
	{ TYPE_FLOAT, NULL, FALSE, 3 },	//Radius
	{ TYPE_FLOAT, NULL, TRUE, 4 },	//Velocity Inheritance
	{ TYPE_FLOAT, NULL, TRUE, 5 }	//Friction
};

#define PBLOCK_LENGTH	6

#define CURRENT_VERSION	0
#define NUM_OLDVERSIONS	1

// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc versions[] = 
{
	ParamVersionDesc (SphDefdescVer0, 5, 0),
};

//--- Deflect object methods -----------------------------------------

SphereDefObject::SphereDefObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(SphDefdescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
//	assert(pblock);	

	pblock2 = NULL;
	SpheredeflectDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

//	pblock->SetValue(PB_BOUNCE,0,1.0f);
//	pblock->SetValue(PB_BVAR,0,0.0f);
//	pblock->SetValue(PB_CHAOS,0,0.0f);
//	pblock->SetValue(PB_VELOCITY,0,0.0f);

	pblock2->SetValue(sdeflectrobj_bounce,0,1.0f);

	pblock2->SetValue(sdeflectrobj_bouncevar,0,0.0f);
	pblock2->SetValue(sdeflectrobj_chaos,0,0.0f);
//	pblock2->SetValue(sdeflectrobj_velocity,0,0.0f);
	pblock2->SetValue(sdeflectrobj_friction,0,0.0f);
	pblock2->SetValue(sdeflectrobj_radius,0,0.0f);

	srand(lastrnd=12345);
	t=99999;
	macroRec->Disable();
//watje create a new ref to our collision engine
	CollisionSphere *cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
	if (cols)
	{
		pblock2->SetValue(sdeflectrobj_collider,0,(ReferenceTarget*)cols);
	}
	macroRec->Enable();

}

SphereDefObject::~SphereDefObject()
{
	DeleteAllRefsFromMe();

}

Modifier *SphereDefObject::CreateWSMMod(INode *node)
	{
	return new SphereDeflectMod(node,this);
	}

RefTargetHandle SphereDefObject::Clone(RemapDir& remap) 
	{
	SphereDefObject* newob = new SphereDefObject();	
//	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ReplaceReference(0,pblock2->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
	}

void SphereDefObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_LEGACY),
			DefaultSOTProc,
			GetString(IDS_RB_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	SpheredeflectDesc.BeginEditParams(ip, this, flags, prev);

/*	if (pmapParam) {		
		// Left over
		pmapParam->SetParamBlock(pblock);
	} else {		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC),
			DefaultSOTProc,
			GetString(IDS_RB_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SW_SPHEREDEFL),
			GetString(IDS_RB_PARAMETERS),
			0);
		}
*/
}

void SphereDefObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;
	
	SpheredeflectDesc.EndEditParams(ip, this, flags, next);

	if (flags&END_EDIT_REMOVEUI ) 
	{		
//		DestroyCPParamMap(pmapParam);
//		pmapParam = NULL;		
		if (hSot)
		{
			ip->DeleteRollupPage(hSot);
			hSot = NULL;
		}
	}	
}

void SphereDefObject::BuildMesh(TimeValue t)
	{
	ivalid = FOREVER;
	float length;
//	pblock->GetValue(PB_RADIUS,t,length,ivalid);
	pblock2->GetValue(sdeflectrobj_radius,t,length,ivalid);
	float u;
	#define NUM_SEGS	16
	length/=2.0f;

	mesh.setNumVerts(3*NUM_SEGS+1);
	mesh.setNumFaces(3*NUM_SEGS);

	for (int i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i, Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
			}
	for (i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i+NUM_SEGS, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
			}
	for (i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i+2*NUM_SEGS, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
			}		
	mesh.setVert(3*NUM_SEGS, Point3(0.0f, 0.0f, 0.0f));
		
	for (i=0; i<3*NUM_SEGS; i++) {
			int i1 = i+1;
			if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
			mesh.faces[i].setEdgeVisFlags(1,0,0);
			mesh.faces[i].setSmGroup(0);
			mesh.faces[i].setVerts(i,i1,3*NUM_SEGS);
			}
	mesh.InvalidateGeomCache();
	}


class SphereDeflectObjCreateCallback : public CreateMouseCallBack {
	public:
		SphereDefObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int SphereDeflectObjCreateCallback::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{

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
		switch(point) {
			case 0:
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
//				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pblock2->SetValue(sdeflectrobj_radius,0,0.01f);
//				ob->pmapParam->Invalidate();
				sdeflector_param_blk.InvalidateUI();
				break;

			case 1: {
				sp1 = m;
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
//				ob->pblock->SetValue(PB_RADIUS,0,2.0f*Length(p1-p0));
				ob->pblock2->SetValue(sdeflectrobj_radius,0,2.0f*Length(p1-p0));
//				ob->pmapParam->Invalidate();
				sdeflector_param_blk.InvalidateUI();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3) {						
						return CREATE_ABORT;
					} else {
						return CREATE_STOP;
						}
					}
				break;
				}

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}	
	return TRUE;
	}
static SphereDeflectObjCreateCallback SpheredeflectCreateCB;

CreateMouseCallBack* SphereDefObject::GetCreateMouseCallBack()
	{
	SpheredeflectCreateCB.ob = this;
	return &SpheredeflectCreateCB;
	}

void SphereDefObject::InvalidateUI() 
{
	sdeflector_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
//	if (pmapParam) pmapParam->Invalidate();
}

ParamDimension *SphereDefObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case sdeflectrobj_bouncevar:
		case sdeflectrobj_chaos:
		case sdeflectrobj_friction:
				 return stdPercentDim;
		case sdeflectrobj_radius:
				return stdWorldDim;
		default: return defaultDim;
		}
	}

/*
TSTR SphereDefObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_BOUNCE: 	return GetString(IDS_RB_BOUNCE);
		case PB_BVAR:		return GetString(IDS_RB_BVAR);
		case PB_CHAOS:		return GetString(IDS_RB_CHAOS);
		case PB_RADIUS:		return GetString(IDS_AP_ICONSIZE);
		case PB_VELOCITY:	return GetString(IDS_AP_VELOCITY);
		default: 			return TSTR(_T(""));
		}
	}
*/


//--- DeflectMod methods -----------------------------------------------

SphereDeflectMod::SphereDeflectMod(INode *node,SphereDefObject *obj)
	{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
	}

Interval SphereDeflectMod::GetValidity(TimeValue t) 
	{
	if (obRef && nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		SphereDefObject *obj = (SphereDefObject*)GetWSMObject(t);
//		obj->pblock->GetValue(PB_BOUNCE,t,f,valid);
//		obj->pblock->GetValue(PB_CHAOS,t,f,valid);
//		obj->pblock->GetValue(PB_RADIUS,t,f,valid);
		obj->pblock2->GetValue(sdeflectrobj_bounce,t,f,valid);
		obj->pblock2->GetValue(sdeflectrobj_chaos,t,f,valid);
		obj->pblock2->GetValue(sdeflectrobj_radius,t,f,valid);
		obj->pblock2->GetValue(sdeflectrobj_friction,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}

class SphereDeflectDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static SphereDeflectDeformer Sphereddeformer;

Deformer& SphereDeflectMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return Sphereddeformer;
	}

RefTargetHandle SphereDeflectMod::Clone(RemapDir& remap) 
	{
	SphereDeflectMod *newob = new SphereDeflectMod(nodeRef,(SphereDefObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}


void SphereDeflectMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (SphereDefObject*)GetWSMObject(t);
		deflect.node = nodeRef;
		deflect.tmValid.SetEmpty();	
		if (t<=deflect.obj->t) deflect.obj->lastrnd=12345;
		deflect.obj->t=t;
		obj->ApplyCollisionObject(&deflect);
		}
	}

Object *SphereDeflectorField::GetSWObject()
{	return obj;
}

BOOL SphereDeflectorField::CheckCollision(
		TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide)
{	Point3 zero=Zero;

//watje need to get a pointer to out collider engine
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(sdeflectrobj_collider,t,rt,tmValid);
//	cols = (CollisionSphere *) rt;
// bayboro: the proper way to get a pointer to a our collider engine
// is to create CollisionSphere for each SphereDeflectorField object
	if (cols==NULL)	
		cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);

	if (!tmValid.InInterval(t)) 
	{
		tmValid = FOREVER;
		tm = node->GetObjectTM(t,&tmValid);

//watje moved the get values so they would not get called on every particle
		obj->pblock2->GetValue(sdeflectrobj_bounce,t,bounce,FOREVER);
		if (bounce<0.001f) 
			bounce+=0.001f;
		obj->pblock2->GetValue(sdeflectrobj_bouncevar,t,bvar,FOREVER);
		obj->pblock2->GetValue(sdeflectrobj_chaos,t,chaos,FOREVER);
		obj->pblock2->GetValue(sdeflectrobj_friction,t,friction,FOREVER);
		obj->pblock2->GetValue(sdeflectrobj_radius,t,radius,FOREVER);
		obj->pblock2->GetValue(sdeflectrobj_velocity,t,vinher,FOREVER);
		radius*=0.5f;
		if (friction < 0.0f) friction = 0.0f;
		if (friction > 1.0f) friction = 1.0f;
		if (bounce < 0.0f) bounce = 0.0f;

//watje load up the engine
		if (cols) 
		{	//macroRecorder->Disable();
			// AnimateOff/On sequence to avoid "pseudo" animated radius for collision engine (Bayboro 6/15/01)
			int animateOn = Animating();
			if (animateOn) AnimateOff();
			Control* cont = obj->pblock2->GetController(sdeflectrobj_radius);
			if (cont) {
				 cols->pblock->SetController(collisionsphere_radius, 0, cont, FALSE);
				// need scale factor of 0.5 for radius
				cols->pblock->SetValue(collisionsphere_scaleFactor, 0, 0.5f);
			}
			else cols->SetRadius(t, radius);
			cols->SetNode(t,node);
			if (animateOn) AnimateOn();
// this lets it set up some pre initialization data
			cols->PreFrame(t,(TimeValue) dt);
			//macroRecorder->Enable();
		}
//watje if the engine does not exist load up old way
		else
		{
			invtm = Inverse(tm);
			Interval tmpValid=FOREVER;
			tp=node->GetObjectTM(t,&tmpValid);
			Vc=zero;
			Vcp=zero*tp*invtm;
		}
	}
//watje
	if (cols)
	{
		Point3 hitpoint,bnorm,frict,inheritedVel;
		float at;
//just make the call to the engine to get the hit point and relevant data
		inheritedVel.x = vinher; // to pass information about inherited Velocity (for values < 1.0f)
		BOOL hit = cols->CheckCollision(t,pos,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

		if (!hit)
		{	obj->lastrnd=rand();
			return FALSE;
		}

	// Remove the part of dt we used to get to the collision point
		float holddt = dt;
		dt -= at;
		
	// Reflect the velocity about the XY plane and attenuate with the bounce factor
	// and add in inherited motion
		float rvariation = 1.0f;
		float rchaos = 1.0f;
		if (bvar != 0.0f)
			{
			rvariation =1.0f-( bvar * randomFloat[index%500]);
			}
		if (chaos != 0.0f)
			{
			rchaos =1.0f-( chaos * randomFloat[index%500]);
			}

		// Bayboro: New vector calculation scheme
		float normLen = bounce*rvariation*Length(bnorm);
		float fricLen = Length(frict);

		vel = bnorm*(bounce*rvariation) + frict*FricCoef(friction, normLen, fricLen, at);

		if (rchaos < 0.9999f) // randomize vector direction
		{	int indexOffset = index + 33 + abs(t - (int)ceil(dt));
			Point3 axis(randomFloat[(indexOffset)%500],randomFloat[(indexOffset+1)%500],randomFloat[(indexOffset+2)%500]);
			axis = axis - Point3(0.5f,0.5f,0.5f);
			float lsq = LengthSquared(bnorm);
			axis -= bnorm*DotProd(axis,bnorm)/lsq;
			float angle = PI*(1.0f-rchaos)*randomFloat[(index+57)%500];
			Quat quat = QFromAngAxis(angle, axis);
			Matrix3 tm;
			quat.MakeMatrix(tm);
			vel = vel*tm;
			// vector should not go beyond surface
			float dotProd = DotProd(vel,bnorm);
			if ( dotProd < 0.0f)
				vel -= 2*bnorm*dotProd/lsq;
		}

		vel += vinher*inheritedVel;

//		AddInheritVelocity(bnorm, frict, vel, inheritedVel,vinher, normLen, fricLen);
		// Bayboro: end of the new scheme

		// Bayboro: old scheme
//		vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) + (inheritedVel * vinher);
		
		pos = hitpoint;

		if (UpdatePastCollide)
			{	pos += vel * dt;  //uses up the rest of the time with the new velocity
				if (ct) (*ct) = holddt;
			}
			else
			{	if (ct) (*ct) = at;
			}

			obj->lastrnd=rand();

			return TRUE;
		}
//watje collision engine does not exist fall back
	else
		{
		Point3 p, vr;
		srand(obj->lastrnd);
		p=pos*invtm; 
		vr=VectorTransform(invtm,vel);
		Point3 Vdt;
		Vdt=(Vcp-Vc)/dt;
		Point3 Vrel=vr-Vdt;
		//if (!FGT0(Vrel))return(0);
		float rsquare=radius*radius;
		float rplus=radius,rminus=radius;
		Point3 Vreln=Normalize(Vrel);
		if (LengthSquared(p-Vc)>=(rminus*rminus)) //outside
		{	Point3 P1;
			P1=p+dt*Vrel; //second particle position
			if (!(LengthSquared(P1-Vc)<(rplus*rplus)))
			{	float Dist,Dist1;
				Dist=DotProd(Vreln,(Vc-p));
				if (Dist<0.0f) 
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Dist1=DotProd(-Vreln,(Vc-P1));
				if (Dist1<0.0f)	
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Point3 P10=P1-p,Pc=Vc-p;
				float gamma=(float)acos(DotProd(P10,Pc)/(Length(P10)*Length(Pc)));
				float Dist2=Length(Pc)*(float)cos(HalfPI-gamma);
				if (Dist2>radius) 
				{	obj->lastrnd=rand();
					return FALSE;
				}
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt)) 
			{	obj->lastrnd=rand();
				return FALSE;
			}
			Point3 XI,r,n;
			XI=p+omega*vr;
			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(-Vreln,r);
			float theta=(float)acos(q1);
			if (theta>=HalfPI) theta-=PI;
			float v[4];
			if (theta<FLOAT_EPSILON) vr=-vr;
			else 
			{	n=Normalize((-Vreln)^r);
				vr=-vr;
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&zero.x,&n.x,2*theta);
				memcpy(vr,v,row3size);
			}
			vr=vr*bounce*(1-bvar*RND01());
			if (!FloatEQ0(chaos))
			{	theta=(HalfPI-theta)*chaos*RND01();
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(), ytmp=RND11(), xtmp=RND11();
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&zero.x,&c.x,theta);
				memcpy(vr,v,row3size);
			}
			if ((vinher>0.0f)&&(t>0))
			{	vr = vr+DotProd(Vdt*vinher,r)*r;
			}	
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
			pos=pos*tm;
			vel=VectorTransform(tm,vr);
			obj->lastrnd=rand();
			return TRUE;
		}
		else //inside
		{	Point3 P1;
			P1=p+dt*Vrel;
			if (LengthSquared(P1-Vc)<(rplus*rplus)) 
			{	obj->lastrnd=rand();
				return FALSE;
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt)) 
			{	obj->lastrnd=rand();
				return FALSE;
			}
			Point3 XI,r,n;
			XI=p+omega*vr;
			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(Vreln,r);
			float theta=(float)acos(q1);
			float v[4];
			if (theta<FLOAT_EPSILON) vr=-vr;
			else 
			{	n=Normalize(Vreln^r);
				vr=-vr;
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&zero.x,&n.x,2*theta);
				memcpy(vr,v,row3size);
			}
			vr=vr*bounce*(1-bvar*RND01());
			if (!FloatEQ0(chaos))
			{	theta=(HalfPI-theta)*chaos*RND01();
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(), ytmp=RND11(), xtmp=RND11(); //Barberi 4/19/01 added fload to y,x declares
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&zero.x,&c.x,theta);
				memcpy(vr,v,row3size);
			}
			if ((vinher>0.0f)&&(t>0)){vr=vr+DotProd(Vdt*vinher,r)*r;}
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
			pos=pos*tm;
			vel=VectorTransform(tm,vr);
			obj->lastrnd=rand();
			return TRUE;
			}
	}
} 

//watje
//need a post load call back to create an spherical collision engine for our pblock
class SphereDefLoad : public PostLoadCallback {
	public:
		SphereDefObject *n;
		SphereDefLoad(SphereDefObject *ns) {n = ns;}
		void proc(ILoad *iload) {  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(sdeflectrobj_collider,0,rt,iv);
			if (rt == NULL)
				{

				CollisionSphere *cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
				if (cols)
					n->pblock2->SetValue(sdeflectrobj_collider,0,(ReferenceTarget*)cols);
				}

			// Bayboro
			// Parameters Variation, Chaos, Friction and Inherit Vel are readjusted to be
			// consistent with corresponding parameters in Omni- and DynaFlectors.
			if ((HIWORD(iload->GetFileSaveVersion()) < 4000) && 
			    (LOWORD(iload->GetFileSaveVersion()) < 48)) // MAX v.3.9, build 48
			{
				Control *cont, *mult;
				float pcnt = 0.01f;

				int defl_chan[3] = {sdeflectrobj_bouncevar, sdeflectrobj_chaos, sdeflectrobj_friction};
				// do nothing with Variation, Chaos & Friction since it's converted from Shiva automatically
				if (HIWORD(iload->GetFileSaveVersion()) >= 3900)
					for( int i=0; i<3; i++)
					{	cont = n->pblock2->GetController(defl_chan[i]);
						if (cont == NULL)
							n->pblock2->SetValue(defl_chan[i], 0, 0.01f*n->pblock2->GetFloat(defl_chan[i], 0));
						else
						{	if (cont->CanApplyEaseMultCurves())
							{	mult = NewDefaultFloatController();
								mult->SetValue(0, &pcnt);
								cont->AppendMultCurve(mult);
							}
							else
								n->pblock2->SetValue(defl_chan[i], 0, 0.01f*n->pblock2->GetFloat(defl_chan[i], 0));
						}
					}

				cont = n->pblock2->GetController(sdeflectrobj_velocity);
				if (cont != NULL) // user's done some manipulations with the parameters; let's rescale
				{	if (cont->CanApplyEaseMultCurves())
					{	mult = NewDefaultFloatController();
						mult->SetValue(0, &pcnt);
						cont->AppendMultCurve(mult);
					}
					else
						n->pblock2->SetValue(sdeflectrobj_velocity, 0, 0.01f*n->pblock2->GetFloat(sdeflectrobj_velocity, 0));
				}
				else
				{	if ((n->pblock2->GetFloat(sdeflectrobj_velocity,0) == 1.0f) &&
						(HIWORD(iload->GetFileSaveVersion()) >= 3900))
					{	; // do nothing since user probably haven't adjusted the parameter and left it as default value
						// therefore it should be left as 1.0 multiplier
					}
					else	// user's done some manipulations with the parameters; let's rescale
						n->pblock2->SetValue(sdeflectrobj_velocity, 0, 0.01f*n->pblock2->GetFloat(sdeflectrobj_velocity, 0));
				}
			}

			delete this; 
			} 
	};


IOResult SphereDefObject::Load(ILoad *iload)
{
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &sdeflector_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
//watje
	iload->RegisterPostLoadCallback(new SphereDefLoad(this));
	
	return IO_OK;
}

CollisionObject *SphereDefObject::GetCollisionObject(INode *node)
{
	SphereDeflectorField *gf = new SphereDeflectorField;	
	gf->obj  = this;
	gf->node = node;
	gf->tmValid.SetEmpty();
	return gf;
}

