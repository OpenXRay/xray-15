/**********************************************************************
 *<
	FILE: Deflect.cpp

	DESCRIPTION: A simple planar deflector

	CREATED BY: Rolf Berteig
			

	HISTORY: Peter Watje 3-15-00

 *>	Copyright (c) 200, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "ICollision.h"
#include "macrorec.h"

#define PBLOCK_REF_NO 0

#define USE_OLD_COLLISION 0




class DeflectObject : public SimpleWSMObject2 {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		DeflectObject();
		~DeflectObject();

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		Class_ID ClassID() {return Class_ID(DEFLECTOBJECT_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_DEFLECTOR);}
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);		
		
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);	
		
		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

		IOResult Load(ILoad *iload);
//watje
		CollisionObject *GetCollisionObject(INode *node);

	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *DeflectObject::ip        = NULL;
//IParamMap *DeflectObject::pmapParam = NULL;
HWND       DeflectObject::hSot      = NULL;

class DeflectorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new DeflectObject;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_DEFLECTOR_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(DEFLECTOBJECT_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("Deflector"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static DeflectorClassDesc deflectDesc;
ClassDesc* GetDeflectObjDesc() {return &deflectDesc;}

//--- DeflectMod -----------------------------------------------------

class DeflectorField : public CollisionObject {
	public:	
		DeflectorField()
		{
			srand(1264873);
			for (int i =0;i < 500; i++)
				{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
				}
			colp=NULL;
		}

		void DeleteThis() 
			{	 
			if (colp) colp->DeleteThis();
			colp=NULL;
			delete this;

			}

		~DeflectorField()
		{
			if (colp) colp->DeleteThis();
			colp=NULL;
		}

		DeflectObject *obj;
		INode *node;
		Matrix3 tm, invtm;
		Interval tmValid;		
		CollisionPlane *colp;

		float randomFloat[500];

		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index, float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();

//hmm moved these so they are only gotten at one tick in time
		float width, height, at, bounce;
		float friction, chaos, variation, inherit;
		int quality;

	};
class DeflectMod : public SimpleWSMMod {
	public:				
		DeflectorField deflect;

		DeflectMod() {}
		DeflectMod(INode *node,DeflectObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_DEFLECTMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(DEFLECTMOD_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_DEFLECTORBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);


	};

//--- ClassDescriptor and class vars ---------------------------------

class DeflectorModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new DeflectMod;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_DEFLECTMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(DEFLECTMOD_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static DeflectorModClassDesc deflectModDesc;
ClassDesc* GetDeflectModDesc() {return &deflectModDesc;}


// block IDs
enum { deflect_params, };

// geo_param param IDs
enum { deflect_bounce, deflect_width, deflect_height, deflect_variation, deflect_chaos,deflect_friction, 
	   deflect_inherit_vel,deflect_quality,deflect_collider};

// JBW: here are the two static block descriptors.  This form of 
//      descriptor declaration uses a static NParamBlockDesc instance whose constructor
//      uses a varargs technique to walk through all the param definitions.
//      It has the advantage of supporting optional and variable type definitions, 
//      but may generate a tad more code than a simple struct template.  I'd
//      be interested in opinions about this.

//      I'll briefly describe the first definition so you can figure the others.  Note
//      that in certain places where strings are expected, you supply a string resource ID rather than
//      a string at it does the lookup for you as needed.
//
//		line 1: block ID, internal name, local (subanim) name, flags
//																 AUTO_UI here means the rollout will
//																 be automatically created (see BeginEditParams for details)
//      line 2: since AUTO_UI was set, this line gives: 
//				dialog resource ID, rollout title, flag test, appendRollout flags
//		line 3: required info for a parameter:
//				ID, internal name, type, flags, local (subanim) name
//		lines 4-6: optional parameter declaration info.  each line starts with a tag saying what
//              kind of spec it is, in this case default value, value range, and UI info as would
//              normally be in a ParamUIDesc less range & dimension
//	    the param lines are repeated as needed for the number of parameters defined.


// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 deflect_param_blk ( deflect_params, _T("DeflectorParameters"),  0, &deflectDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_DEFLECTORPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	deflect_bounce,  _T("bounce"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_BOUNCE, 
		p_default, 		1.0,	
		p_ms_default,	1.0,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DEFLECT_BOUNCE,IDC_DEFLECT_BOUNCESPIN, 0.01f, 
		end, 

	deflect_width,  _T("width"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_WIDTH, 
		p_default, 		0.0,	
		p_ms_default,	10.0,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DEFLECT_WIDTH,IDC_DEFLECT_WIDTHSPIN, SPIN_AUTOSCALE, 
		end, 

	deflect_height,  _T("length"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_LENGTH, 
		p_default, 		0.0,	
		p_ms_default,	10.0,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DEFLECT_HEIGHT,IDC_DEFLECT_HEIGHTSPIN, SPIN_AUTOSCALE, 
		end, 

	deflect_variation,  _T("variation"),	TYPE_PCNT_FRAC, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_VARIATION, 
		p_default, 		0.0,	
		p_ms_default,	0.0,
		p_range, 		0.0f, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SPHDEFLECT_BOUNCEVAR,IDC_SPHDEFLECT_BOUNCEVARSPIN, 1.0f, 
		end, 

	deflect_chaos,  _T("chaos"),	TYPE_PCNT_FRAC, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_CHAOS, 
		p_default, 		0.0,	
		p_ms_default,	0.0,
		p_range, 		0.0f, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DEFLECT_CHAOS,IDC_DEFLECT_CHAOSSPIN, 1.0f, 
		end, 

	deflect_friction,  _T("friction"),	TYPE_PCNT_FRAC, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_FRICTION, 
		p_default, 		0.0,	
		p_ms_default,	0.0,
		p_range, 		0.0f, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DEFLECT_FRICTION,IDC_DEFLECT_FRICTIONSPIN, 1.0f, 
		end, 

	deflect_inherit_vel,  _T("inheritVelocity"),	TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_INHERITVEL, 
		p_default, 		1.0,	
		p_ms_default,	1.0,
		p_range, 		0.0f, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DEFLECT_INHERIT,IDC_DEFLECT_INHERITSPIN, 0.01f, 
		end, 

//	deflect_quality,  _T("quality"),	TYPE_INT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_QUALITY, 
//	deflector quality parameter is no longer used (Bayboro 4/18/01)
	deflect_quality,  _T(""),	TYPE_INT, 0, 0, 
		p_default, 		20,	
		p_range, 		0, 100,
//		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_DEFLECT_QUALITY,IDC_DEFLECT_QUALITYSPIN, SPIN_AUTOSCALE, 
		end, 
//this is a reference to the planar collision engine
	deflect_collider,  _T(""),	TYPE_REFTARG, 	0, 	0, 
		end, 



	end
	);


//--- DeflectObject Parameter map/block descriptors ------------------
#define PARAMDESC_LENGTH	3

ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, deflect_bounce },
	{ TYPE_FLOAT, NULL, TRUE, deflect_width },
	{ TYPE_FLOAT, NULL, TRUE, deflect_height }};

#define PBLOCK_LENGTH	3

//#define CURRENT_VERSION	0
#define NUM_OLDVERSIONS	1

	// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc (descVer0, 3, 0),
};


//--- Deflect object methods -----------------------------------------

DeflectObject::DeflectObject()
	{
	pblock2 = NULL;
	deflectDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

	pblock2->SetValue(deflect_bounce,0,1.0f);

//create an instance of our planar collision engine
//and stuff it into our param block
	macroRecorder->Disable();
	CollisionPlane *colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
	if (colp)
		{
		pblock2->SetValue(deflect_collider,0,(ReferenceTarget*)colp);
		colp->SetWidth(pblock2->GetController(deflect_width));
		colp->SetHeight(pblock2->GetController(deflect_height));
		colp->SetQuality(pblock2->GetController(deflect_quality));
		}
	macroRecorder->Enable();

	}

Modifier *DeflectObject::CreateWSMMod(INode *node)
	{
	return new DeflectMod(node,this);
	}

CollisionObject *DeflectObject::GetCollisionObject(INode *node)
	{
	DeflectorField *gf = new DeflectorField;	
	gf->obj  = this;
	gf->node = node;
	gf->tmValid.SetEmpty();
	return gf;
	}


RefTargetHandle DeflectObject::Clone(RemapDir& remap) 
	{
	DeflectObject* newob = new DeflectObject();	
	newob->ReplaceReference(0,pblock2->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
	}

void DeflectObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_WINDRAIN_SOT),
			DefaultSOTProc,
			GetString(IDS_RB_SOT), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	deflectDesc.BeginEditParams(ip, this, flags, prev);

	}

void DeflectObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	deflectDesc.EndEditParams(ip, this, flags, next);

	if (flags&END_EDIT_REMOVEUI ) 
	{		
		if (hSot)
		{	ip->DeleteRollupPage(hSot);
			hSot = NULL;
		}
	}	

}


void DeflectObject::BuildMesh(TimeValue t)
	{
	float width, height;
	ivalid = FOREVER;
	pblock2->GetValue(deflect_width,t,width,ivalid);
	pblock2->GetValue(deflect_height,t,height,ivalid);
	width  *= 0.5f;
	height *= 0.5f;

	mesh.setNumVerts(4);
	mesh.setNumFaces(2);
	mesh.setVert(0, Point3(-width,-height, 0.0f));
	mesh.setVert(1, Point3( width,-height, 0.0f));
	mesh.setVert(2, Point3( width, height, 0.0f));
	mesh.setVert(3, Point3(-width, height, 0.0f));
	
	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(1);
	mesh.faces[0].setVerts(0,1,3);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(1);
	mesh.faces[1].setVerts(1,2,3);	
	mesh.InvalidateGeomCache();	
	}


class DeflectObjCreateCallback : public CreateMouseCallBack {
	public:
		DeflectObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int DeflectObjCreateCallback::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				p0  = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				mat.SetTrans(p0);
				ob->pblock2->SetValue(deflect_width,0,0.01f);
				ob->pblock2->SetValue(deflect_height,0,0.01f);
				deflect_param_blk.InvalidateUI();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
				ob->pblock2->SetValue(deflect_width,0,(float)fabs(p1.x-p0.x));
				ob->pblock2->SetValue(deflect_height,0,(float)fabs(p1.y-p0.y));
				deflect_param_blk.InvalidateUI();
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

static DeflectObjCreateCallback deflectCreateCB;

CreateMouseCallBack* DeflectObject::GetCreateMouseCallBack()
	{
	deflectCreateCB.ob = this;
	return &deflectCreateCB;
	}

void DeflectObject::InvalidateUI() 
	{
	deflect_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
	}

ParamDimension *DeflectObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case deflect_variation:
		case deflect_chaos:
		case deflect_friction:
				 return stdPercentDim;
		case deflect_width:
		case deflect_height:
				return stdWorldDim;
		default: return defaultDim;
		}
	}

//need a post load call back to create a planar collision engine for our pblock
class DeflectObjectLoad : public PostLoadCallback {
	public:
		DeflectObject *n;
		DeflectObjectLoad(DeflectObject *ns) {n = ns;}
		void proc(ILoad *iload) {  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(deflect_collider,0,rt,iv);
			if (rt == NULL)
				{
				macroRecorder->Disable();
				CollisionPlane *colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
				if (colp)
					n->pblock2->SetValue(deflect_collider,0,(ReferenceTarget*)colp);
				macroRecorder->Enable();
				}

			// Bayboro
			// Parameters Variation, Chaos, Friction and Inherit Vel are readjusted to be
			// consistent with corresponding parameters in Omni- and DynaFlectors.
			if (HIWORD(iload->GetFileSaveVersion() < 3900)) // MAX before Magma=v.3.900
			{ ; // do nothing since Variation, Chaos, Friction and Inherit Vel aren't present in Shiva and before
			}
			else if ((HIWORD(iload->GetFileSaveVersion()) < 4000) && 
					 (LOWORD(iload->GetFileSaveVersion()) < 48)) // MAX v.3.9, build 48
			{
				Control *cont, *mult;
				float pcnt = 0.01f;
	
				int defl_chan[3] = {deflect_variation, deflect_chaos, deflect_friction};
				for( int i=0; i<3; i++)
				{	cont = n->pblock2->GetController(defl_chan[i]);
					if (cont == NULL)
						n->pblock2->SetValue(defl_chan[i], 0, 0.01f*n->pblock2->GetFloat(defl_chan[i], 0));
					else
					{	mult = NewDefaultFloatController();
						mult->SetValue(0, &pcnt);
						cont->AppendMultCurve(mult);
					}
				}

				cont = n->pblock2->GetController(deflect_inherit_vel);
				if (cont != NULL) // user's done some manipulations with the parameters; let's rescale
				{	mult = NewDefaultFloatController();
					mult->SetValue(0, &pcnt);
					cont->AppendMultCurve(mult);
				}
				else
				{	if (n->pblock2->GetFloat(deflect_inherit_vel,0) == 1.0f)
					{	; // do nothing since user probably haven't adjusted the parameter and left it as default value
						// therefore it should be left as 1.0 multiplier
					}
					else	// user's done some manipulations with the parameters; let's rescale
						n->pblock2->SetValue(deflect_inherit_vel, 0, 0.01f*n->pblock2->GetFloat(deflect_inherit_vel, 0));
				}
			}

			delete this; 
			} 
	};

// JBW:  for loading old ParamBlock versions, register an Ed. 2 param block converter post-load callback
//       This one takes the ParamBlockDescID version array as before, but now takes the 
//       ParamBlockDesc2 for the block to describe the current version and will convert the loaded ParamBlock
//       into a corresponding ParamBlock2
IOResult
DeflectObject::Load(ILoad *iload) 
{	
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &deflect_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	iload->RegisterPostLoadCallback(new DeflectObjectLoad(this));

	return IO_OK;
}

//--- DeflectMod methods -----------------------------------------------

DeflectMod::DeflectMod(INode *node,DeflectObject *obj)
	{	
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
	}

Interval DeflectMod::GetValidity(TimeValue t) 
	{
	if (obRef && nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		DeflectObject *obj = (DeflectObject*)GetWSMObject(t);
		obj->pblock2->GetValue(deflect_bounce,t,f,valid);
		obj->pblock2->GetValue(deflect_width,t,f,valid);
		obj->pblock2->GetValue(deflect_height,t,f,valid);

		obj->pblock2->GetValue(deflect_variation,t,f,valid);
		obj->pblock2->GetValue(deflect_chaos,t,f,valid);
		obj->pblock2->GetValue(deflect_friction,t,f,valid);
		obj->pblock2->GetValue(deflect_inherit_vel,t,f,valid);
		int i;
		obj->pblock2->GetValue(deflect_quality,t,i,valid);


		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}

class DeflectDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static DeflectDeformer ddeformer;

Deformer& DeflectMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return ddeformer;
	}

RefTargetHandle DeflectMod::Clone(RemapDir& remap) 
	{
	DeflectMod *newob = new DeflectMod(nodeRef,(DeflectObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}


void DeflectMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (DeflectObject*)GetWSMObject(t);
		deflect.node = nodeRef;
		deflect.tmValid.SetEmpty();		
		obj->ApplyCollisionObject(&deflect);
		}
	}

Object *DeflectorField::GetSWObject()
{ return obj;
}


//watje
//can we make this inline code since this is going to get called a lot
//how much of a memory foot print would this make if we did(it is not that big of a function)

DeflectObject::~DeflectObject()
{	
}
const float angleCoefSmall=0.035f;
const float angleCoefBig=2.0f*angleCoefSmall;

float FricCoef(float friction, float normLen, float fricLen, float contactDur)
{
	float fricCoef;
	if (friction == 0.0f) fricCoef = 1.0f;
	else if (friction == 1.0f) fricCoef = 0.0f;
	else if (normLen >= angleCoefBig*fricLen) // angle of reflection is more than 4 degrees: not a "slide"
		fricCoef = 1.0f - friction;
	else if (normLen < angleCoefSmall*fricLen) // angle of reflection is less than 2 degrees: it's a "slide"
		fricCoef = (float)exp(contactDur*log(1-friction)/GetTicksPerFrame());
	else {
		float mix = (normLen/fricLen)/angleCoefSmall - 1.0f;
		fricCoef = mix*(1.0f-friction) 
					+ (1.0f-mix)*(float)exp(contactDur*log(1-friction)/GetTicksPerFrame());
	}
	return fricCoef;
}

void AddInheritVelocity(Point3 &bounceVec, Point3 &frictionVec, Point3 &partVec,
						Point3 &inheritVec, float vinher, float normLen, float fricLen)
{
	if (vinher == 0.0f) return; 
	if (normLen >= angleCoefBig*fricLen) // angle of reflection is more than 4 degrees: not a "slide"
	{
		partVec += vinher*inheritVec;
		return;
	}
	if (DotProd(inheritVec, inheritVec) == 0.0f) return;
	if (DotProd(bounceVec, bounceVec) == 0.0f) // no bounce component
	{
		partVec += vinher*inheritVec;
		return;
	}
	Point3 bounceAxe = FNormalize(bounceVec);
	float partZ = DotProd(bounceAxe, partVec);
	Point3 partAxe = partVec - partZ*bounceAxe;
	if (DotProd(partAxe, partAxe) < 0.000001f) 
	{ // particle velocity coinsides to bounceVec: add inherited Vel
		partVec += vinher*inheritVec;
		return;
	}
	partAxe = FNormalize(partAxe);
	float partX = DotProd(partAxe, partVec);
	Point3 ortAxe = bounceAxe^partAxe;
	float partY = DotProd(ortAxe, partVec);
	Point3 inhVec = vinher*inheritVec;
	float inhX = DotProd(partAxe, inhVec);
	float inhY = DotProd(ortAxe, inhVec);
	float inhZ = DotProd(bounceAxe, inhVec);
	if (inhX > partX) partX = inhX;
	else if (inhX < 0.0f) partX += inhX;
	Point3 addInherVel = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec - partVec;

	if (normLen < angleCoefSmall*fricLen) // angle of reflection is less than 2 degrees: it's a "slide"
	{
		partVec = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec;
	}
	else
	{
		float mix = (normLen/fricLen)/angleCoefSmall - 1.0f;
		Point3 addInherVel = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec - partVec;
		partVec += mix*inhVec + (1.0f-mix)*addInherVel;
	}
}

BOOL DeflectorField::CheckCollision(
		TimeValue t,Point3 &pos, Point3 &vel, float dt, int index, float *ct,BOOL UpdatePastCollide)
{	
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(deflect_collider,t,rt,tmValid);
//	colp = (CollisionPlane *) rt;
	if (colp==NULL)
	{
		colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
		if (colp)
		{
			colp->SetWidth(obj->pblock2->GetController(deflect_width));
			colp->SetHeight(obj->pblock2->GetController(deflect_height));
			colp->SetQuality(obj->pblock2->GetController(deflect_quality));
		}
	}
	
	if (!tmValid.InInterval(t)) 
	{	
		tmValid = FOREVER;
		tm    = node->GetObjectTM(t,&tmValid);
		invtm = Inverse(tm);

		obj->pblock2->GetValue(deflect_width,t,width,tmValid);
		obj->pblock2->GetValue(deflect_height,t,height,tmValid);
		obj->pblock2->GetValue(deflect_quality,t,quality,tmValid);
//this loads up our collision planar engine with the data it needs
//note we must check to see if it exists since the engine is a plugin and can be removed
		if (colp)
		{	macroRecorder->Disable();
			colp->SetWidth(t,width);
			colp->SetHeight(t,height);
			colp->SetQuality(t,quality);
			colp->SetNode(t,node);
			// this lets it set up some pre initialization data
			colp->PreFrame(t,(TimeValue) dt);
			macroRecorder->Enable();
		}

		obj->pblock2->GetValue(deflect_bounce,t,bounce,tmValid);
		obj->pblock2->GetValue(deflect_friction,t,friction,tmValid);
		obj->pblock2->GetValue(deflect_chaos,t,chaos,tmValid);
		obj->pblock2->GetValue(deflect_variation,t,variation,tmValid);
		obj->pblock2->GetValue(deflect_inherit_vel,t,inherit,tmValid);
		if (friction < 0.0f) friction = 0.0f;
		if (friction > 1.0f) friction = 1.0f;
		if (bounce < 0.0f) bounce = 0.0f;

		width  *= 0.5f;
		height *= 0.5f;
	}
	
	Point3 p, v, ph;


//if no engine fall back to the old method
	if ((USE_OLD_COLLISION) || (colp==NULL))
		{
	// Transform the point and velocity into our space

		p = pos * invtm; 
		v = VectorTransform(invtm,vel);

	// Compute the point of intersection
		if (fabs(p.z)<0.001) 
		{	v.z = 0.0f;
			at  = 0.0f;		
		} 
		else 
		{	if (v.z==0.0f) 
				return FALSE;
			at = -p.z/v.z;
			if (at < 0.0f || at > dt) 
				return FALSE;
		}
		ph = p + at*v;

	// See if the point is within our range
		if (ph.x<-width || ph.x>width || ph.y<-height || ph.y>height) 
			return FALSE;

	// Remove the part of dt we used to get to the collision point
		float holddt = dt;
		dt -= at;

	// Reflect the velocity about the XY plane and attenuate with the bounce factor
		v.z = -v.z;
		if (bounce!=1.0f) 
			v = v*bounce;

		if (UpdatePastCollide)
		{	ph += v * dt;  //uses up the rest of the time with the new velocity
			if (ct) (*ct) = holddt;
		}
		else
		{	if (ct) (*ct) = at;
		}

	// Tranform back into world space.
		pos = ph * tm;
		vel = VectorTransform(tm,v);
		return TRUE;
		}
	else
//new collision method here
//this is a iterative method that recurses through the dt til it finds a hit point within a threshold
		{
		Point3 hitpoint,bnorm,frict,inheritedVel;
//just make the call to the engine to get the hit point and relevant data
		inheritedVel.x = inherit; // to pass information about inherited Velocity (for values < 1.0f)
		BOOL hit = colp->CheckCollision(t,pos,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

		if (!hit) return FALSE;

	// Remove the part of dt we used to get to the collision point
		float holddt = dt;
		dt -= at;
		
	// Reflect the velocity about the XY plane and attenuate with the bounce factor
	// and add in inherited motion
		float rvariation = 1.0f;
		float rchaos = 1.0f;
		if (variation != 0.0f)
			{
			rvariation =1.0f-( variation * randomFloat[index%500]);
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
		{	int indexOffset = index + 33 + abs(t);
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

		vel += inherit*inheritedVel; 

//		AddInheritVelocity(bnorm, frict, vel, inheritedVel,inherit, normLen, fricLen);
		// Bayboro: end of the new scheme

		// Bayboro: old scheme
//		vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) + (inheritedVel * inherit);
		
		pos = hitpoint;

		if (UpdatePastCollide)
			{	pos += vel * dt;  //uses up the rest of the time with the new velocity
				if (ct) (*ct) = holddt;
			}
			else
			{	if (ct) (*ct) = at;
			}

			return TRUE;
		}

return TRUE;	

}
