/**********************************************************************
 *<
	FILE: gravity.cpp

	DESCRIPTION:  Gravity World Space Modifier

	CREATED BY: Rolf Berteig

	Separated out from Wind, add range hoops by Eric Peterson

	HISTORY: 10-30-95

	Modified: 9/18/01 Bayboro: separate mods from EDP dependency

 *>	Copyright (c) 1994, All Rights Reserved.

  PB2 conversion '00 ECP
 **********************************************************************/
#include "mods.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
// #include "pod.h" // Bayboro 9/18/01

#define PBLOCK_REF_NO 0

#define FORCE_PLANAR	0
#define FORCE_SPHERICAL	1

#define		DONTCARE	2

class GravityMtl: public Material 
{
	public:
	GravityMtl();
};

static GravityMtl swMtl;

#define GRAVITY_R	float(.7)
#define GRAVITY_G	float(0)
#define GRAVITY_B	float(0)

const Point3 HOOPCOLOR(0.0f,0.0f,0.0f);

GravityMtl::GravityMtl():Material() 
{	Kd[0] = GRAVITY_R;
	Kd[1] = GRAVITY_G;
	Kd[2] = GRAVITY_B;
	Ks[0] = GRAVITY_R;
	Ks[1] = GRAVITY_G;
	Ks[2] = GRAVITY_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME|GW_BACKCULL;
	selfIllum = (float)1.0;
}

class ForceObject : public SimpleWSMObject2 
{	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		ForceObject() {}
		BOOL SupportsDynamics() {return TRUE;}

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);

		virtual int DialogID()=0;
//		virtual ParamUIDesc *UIDesc()=0;
//		virtual int UIDescLength()=0;
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
};

class GravityMod;

IObjParam *ForceObject::ip        = NULL;
// IParamMap *ForceObject::pmapParam = NULL;
HWND       ForceObject::hSot      = NULL;

class GravityObject : public ForceObject //,IOperatorInterface // Bayboro 9/18/01
{	
	public:									
		GravityObject();		
		~GravityObject();		

		// From Animatable		
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return Class_ID(GRAVITYOBJECT_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_GRAVITY);}
						
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		ForceField *GetForceField(INode *node);

		// From SimpleWSMObject				
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);

		int DialogID() {return IDD_GRAVITYPARAM;}
//		ParamUIDesc *UIDesc();
//		int UIDescLength();

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

		// from ref
		IOResult Load(ILoad *iload);

//		int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		ForceField *ff;
		void SetUpModifier(TimeValue t,INode *node);
		GravityMod *mf;
};

//--- ClassDescriptor and class vars ---------------------------------

class GravityClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new GravityObject;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_GRAVITY_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(GRAVITYOBJECT_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_FORCES);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("Gravity"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

static GravityClassDesc gravityDesc;
ClassDesc* GetGravityObjDesc() {return &gravityDesc;}


//--- GravityMod -----------------------------------------------------

class GravityMod;

class GravityField : public ForceField 
{
	public:
		GravityObject *obj;
		INode *node;
		Matrix3 tm;
		Interval tmValid;
		Point3 force;
		Interval fValid;
		int type;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index);
		void DeleteThis() {delete this;} // RB 5/12/99
};

class GravityMod : public SimpleWSMMod 
{
	public:				
		GravityField force;

		GravityMod() {}
		GravityMod(INode *node,GravityObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_GRAVITYMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(GRAVITYMOD_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_GRAVITYBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
};

//--- ClassDescriptor and class vars ---------------------------------

class GravityModClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) {return new GravityMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_GRAVITYMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(GRAVITYMOD_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
};

static GravityModClassDesc gravityModDesc;
ClassDesc* GetGravityModDesc() {return &gravityModDesc;}

//--- GravityObject Parameter map/block descriptors ------------------

/*
#define PB_STRENGTH		0
#define PB_DECAY		1
#define PB_TYPE			2
#define PB_DISPLENGTH	3
#define PB_HOOPSON		4
*/

// block IDs
enum { gravity_params, };

// geo_param param IDs
enum { PB_STRENGTH, PB_DECAY, PB_TYPE, PB_DISPLENGTH, PB_HOOPSON };
//
//
// Parameters

static ParamBlockDesc2 gravity_param_blk ( gravity_params, _T("GravityParameters"),  0, &gravityDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_GRAVITYPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	PB_STRENGTH,  _T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_STRENGTH2, 
		p_default, 		1.0f,	
		p_ms_default,	1.0f,
		p_range, 		-9999999.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_GRAV_STRENGTH,IDC_GRAV_STRENGTHSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_DECAY,	  _T("decay"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_DECAY, 
		p_default, 		0.0f,	
		p_ms_default,	0.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_GRAV_DECAY,IDC_GRAV_DECAYSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_TYPE,	  _T("gravitytype"),	TYPE_RADIOBTN_INDEX,	0,		IDS_ECP_GRAVITYTYPE,
		p_default,			0,
		p_range,			0, 1, 
		p_ui,				TYPE_RADIO,		2,	IDC_FORCE_PLANAR, IDC_FORCE_SPHERICAL,
		end,

	PB_DISPLENGTH,  _T("iconsize"),		TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_ECP_GRAVITYICONSIZE, 
		p_default, 		10.0f,	
		p_ms_default,	10.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISPLENGTH,IDC_DISPLENGTHSPIN, SPIN_AUTOSCALE, 
		end, 

	PB_HOOPSON,   _T("hoopson"),		TYPE_BOOL, 0, IDS_ECP_HOOPSON,
		p_default, 		TRUE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_EP_HOOPSON, 
		end,

	end
	);


/*
static int typeIDs[] = {IDC_FORCE_PLANAR,IDC_FORCE_SPHERICAL};

static ParamUIDesc descParamGrav[] = {
	// Strength
	ParamUIDesc(
		PB_STRENGTH,
		EDITTYPE_FLOAT,
		IDC_GRAV_STRENGTH,IDC_GRAV_STRENGTHSPIN,
		-9999999.0f, 9999999.0f,
		0.01f),

	// Decay
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_FLOAT,
		IDC_GRAV_DECAY,IDC_GRAV_DECAYSPIN,
		0.0f, 9999999.0f,
		0.001f),

	// Force type
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),

	// Display length
	ParamUIDesc(
		PB_DISPLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_DISPLENGTH,IDC_DISPLENGTHSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Use  Range Hoops Checkbox
	ParamUIDesc(PB_HOOPSON,TYPE_SINGLECHEKBOX,IDC_EP_HOOPSON),

	};
*/

#define GRAVPARAMDESC_LENGTH	5

// parameter block for version zero
ParamBlockDescID descGravVer0[] = 
{
	{ TYPE_FLOAT, NULL, TRUE,  0 },
	{ TYPE_FLOAT, NULL, TRUE,  1 },
	{ TYPE_INT,   NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, FALSE, 3 },
};

//parameter block for version 1
ParamBlockDescID descGravVer1[] = 
{
	{ TYPE_FLOAT, NULL, TRUE,  0 },
	{ TYPE_FLOAT, NULL, TRUE,  1 },
	{ TYPE_INT,   NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, FALSE, 3 },
	{ TYPE_INT,   NULL, FALSE, 4 },
};

#define GRAVPBLOCK_LENGTH	5

//#define CURRENT_GRAVVERSION	1
//#define NUM_OLDVERSIONS		1
#define NUM_OLDVERSIONS		2

static ParamVersionDesc gravversions[] = 
{
	ParamVersionDesc(descGravVer0,4,0),
	ParamVersionDesc(descGravVer1,5,1),
};

//static ParamVersionDesc gravcurVersion(descGravVer1,GRAVPBLOCK_LENGTH,CURRENT_GRAVVERSION);

//--- ForceObject Methods ---------------------------------------------

void ForceObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_WINDGRAV_SOT),
			DefaultSOTProc,
			GetString(IDS_RB_SOT), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	gravityDesc.BeginEditParams(ip, this, flags, prev);
/*
	if (pmapParam) 
	{		
		// Left over from last SinWave ceated
		pmapParam->SetParamBlock(pblock);
	} 
	else 
	{		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_WINDGRAV_SOT),
			DefaultSOTProc,
			GetString(IDS_RB_SOT), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			UIDesc(),UIDescLength(),
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(DialogID()),
			GetString(IDS_RB_PARAMETERS),
			0);
	}
	*/
}

void ForceObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	gravityDesc.EndEditParams(ip, this, flags, next);

	if (flags&END_EDIT_REMOVEUI ) 
	{		
		if (hSot)
//		DestroyCPParamMap(pmapParam);
		{	ip->DeleteRollupPage(hSot);
			hSot = NULL;
		}
//		pmapParam = NULL;		
	}	
}

class ModDrawLineProc:public PolyLineProc 
{
	GraphicsWindow *gw;
	public:
		ModDrawLineProc() { gw = NULL; }
		ModDrawLineProc(GraphicsWindow *g) { gw = g; }
		int proc(Point3 *p, int n) { gw->polyline(n, p, NULL, NULL, 0, NULL); return 0; }
		int Closed(Point3 *p, int n) { gw->polyline(n, p, NULL, NULL, TRUE, NULL); return 0; }
		void SetLineColor(float r, float g, float b) {gw->setColor(LINE_COLOR,r,g,b);}
		void SetLineColor(Point3 c) {gw->setColor(LINE_COLOR,c);}
		void Marker(Point3 *p,MarkerType type) {gw->marker(p,type);}
};

static void DrawPlaneRangeHoops(float range,float len3, ModDrawLineProc& lp)
{	
	Point3 pt[5];
	lp.SetLineColor(GetUIColor(COLOR_END_RANGE));		
	pt[0] = Point3(len3, len3, range); 
	pt[1] = Point3(-len3, len3, range); 
	pt[2] = Point3(-len3,-len3, range); 
	pt[3] = Point3(len3,-len3, range); 
 	lp.Closed(pt,4);
	pt[0].z =-range; 
	pt[1].z =-range;
	pt[2].z =-range;
	pt[3].z =-range;
 	lp.Closed(pt,4);
}

#define NUM_SEGS	16

static void DrawFalloffSphere(float range, ModDrawLineProc& lp)
{	
	float u;
	Point3 pt[3],pty[3],ptz[3],first,firsty;
	
	lp.SetLineColor(GetUIColor(COLOR_END_RANGE));	
	pt[0]=(first= Point3(range,0.0f,0.0f));
	pty[0] =(firsty=Point3(0.0f,range,0.0f));
	ptz[0] = pt[0];
	for (int i=0; i<NUM_SEGS; i++)
	{	u = float(i)/float(NUM_SEGS) * TWOPI;
		float crange=(float)cos(u)*range,srange=(float)sin(u)*range;
		pt[1]=Point3(crange, srange, 0.0f);
		lp.proc(pt,2); pt[0]=pt[1];
		pty[1]=Point3(0.0f, crange, srange);
		lp.proc(pty,2); pty[0]=pty[1];
		ptz[1]=Point3(crange, 0.0f, srange);
		lp.proc(ptz,2); ptz[0]=ptz[1];
	}
	pt[1]=first;lp.proc(pt,2); 
	pty[1]=firsty;lp.proc(pty,2); 
	ptz[1]=first;lp.proc(ptz,2); 
}

int ForceObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = &swMtl;
	Matrix3 mat = inode->GetObjectTM(t);
 	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );//removed BC 2/16/99 DB

	gw->setTransform(mat);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SPACE_WARPS));
	mesh.render(gw, mtl, NULL, COMP_ALL);
	int hoopson;float decay;
	pblock2->GetValue(PB_HOOPSON,t,hoopson,FOREVER);
	pblock2->GetValue(PB_DECAY,t,decay,FOREVER);
	if (hoopson && (decay > 0.0f))
	{	int type;pblock2->GetValue(PB_TYPE,t,type,FOREVER);
		float range;
		range=(decay > 0.0f?0.6931472f / decay:0.0f);
		ModDrawLineProc lp(gw);
		if (type==FORCE_PLANAR)
		{	float length;
			pblock2->GetValue(PB_DISPLENGTH,t,length,FOREVER);
			DrawPlaneRangeHoops(range,3.0f*length,lp);
		}
		else 
			DrawFalloffSphere(range,lp);
	}
/*	if (hParam && GetWindowLongPtr(hParam,GWLP_USERDATA)==(LONG_PTR)this &&
		GetFalloffOn(t)) 
	{
		DrawLineProc lp(gw);
		DrawFalloffSphere(GetFalloff(t),lp);
	}
*/
	gw->setRndLimits(rlim);
	return(0);
}

void ForceObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{   
	Box3 meshBox;
    Matrix3 mat = inode->GetObjectTM(t);
	box.Init();
	int hoopson;
	pblock2->GetValue(PB_HOOPSON,t,hoopson,FOREVER);
	if (hoopson)
	{	float decay; pblock2->GetValue(PB_DECAY,t,decay,FOREVER);
		if (decay>0.0f)
		{	float range,xy; range=2.0f*(decay > 0.0f?0.6931472f / decay:0.0f);
			int type;pblock2->GetValue(PB_TYPE,t,type,ivalid);
			if (type==FORCE_PLANAR)	
			{	pblock2->GetValue(PB_DISPLENGTH,t,xy,FOREVER);
				xy*=3.0f;
			}	
			else 
				xy=range;
			Box3 rangeBox(Point3(-xy,-xy,-range),Point3(xy,xy,range)); 
			for(int i = 0; i < 8; i++)	
				box += mat * rangeBox[i];
		}
	}
	GetLocalBoundBox(t,inode,vpt,meshBox);	
	for(int i = 0; i < 8; i++)
		box += mat * meshBox[i];
}

void ForceObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float length;
	int type;	
	pblock2->GetValue(PB_DISPLENGTH,t,length,ivalid);
	pblock2->GetValue(PB_TYPE,t,type,ivalid);

	int norvs,norfs,extraverts,extrafaces;
	float decay,range;
	pblock2->GetValue(PB_DECAY,t,decay,ivalid);

	if (decay > 0.0f)
		range = 0.6931472f / decay;
	else
		range = 0.0f;

	if (type==FORCE_PLANAR)
	{	extrafaces = 0;
		extraverts = 0;
		mesh.setNumVerts((norvs=15) + extraverts);
		mesh.setNumFaces((norfs=5) + extrafaces);
		mesh.setVert(0, Point3(-length,-length, 0.0f));
		mesh.setVert(1, Point3( length,-length, 0.0f));
		mesh.setVert(2, Point3( length, length, 0.0f));
		mesh.setVert(3, Point3(-length, length, 0.0f));
		mesh.setVert(4, Point3(   0.0f,   0.0f, 0.0f));
		mesh.setVert(5, Point3(   0.0f,   0.0f, 3*length));
		mesh.setVert(6, Point3(   0.0f,   0.0f, 0.0f));
		length *= 0.3f;
		mesh.setVert(7, Point3(-length,-length, 7.0f*length));
		mesh.setVert(8, Point3( length,-length, 7.0f*length));
		mesh.setVert(9, Point3( length, length, 7.0f*length));
		mesh.setVert(10, Point3(-length, length, 7.0f*length));

		mesh.setVert(11, Point3(-length,-length, range));
		mesh.setVert(12, Point3( length,-length, range));
		mesh.setVert(13, Point3( length, length, range));
		mesh.setVert(14, Point3(-length, length, range));

		mesh.faces[0].setEdgeVisFlags(1,0,1);
		mesh.faces[0].setSmGroup(0);
		mesh.faces[0].setVerts(0,1,3);

		mesh.faces[1].setEdgeVisFlags(1,1,0);
		mesh.faces[1].setSmGroup(0);
		mesh.faces[1].setVerts(1,2,3);

		mesh.faces[2].setEdgeVisFlags(1,1,0);
		mesh.faces[2].setSmGroup(0);
		mesh.faces[2].setVerts(4,5,6);

		mesh.faces[3].setEdgeVisFlags(0,1,1);
		mesh.faces[3].setSmGroup(0);
		mesh.faces[3].setVerts(7,9,5);

		mesh.faces[4].setEdgeVisFlags(0,1,1);
		mesh.faces[4].setSmGroup(0);
		mesh.faces[4].setVerts(10,8,5);
	}
	else
	{	extrafaces = 0;
		extraverts = 0;
		float u;

		mesh.setNumVerts((norvs = 3*NUM_SEGS + 1) + extraverts);
		mesh.setNumFaces((norfs = 3*NUM_SEGS) + extrafaces);

		for (int i=0; i<NUM_SEGS; i++)
		{	u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i           , Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
			mesh.setVert(i + NUM_SEGS, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
			mesh.setVert(i+2*NUM_SEGS, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
		}
		mesh.setVert(3*NUM_SEGS, Point3(0.0f, 0.0f, 0.0f));
		
		int i1,lastvertex = 3*NUM_SEGS;
		for (i=0; i<lastvertex; i++)
		{	i1 = i + 1;
			if (i1%NUM_SEGS == 0) 
				i1 -= NUM_SEGS;
			mesh.faces[i].setEdgeVisFlags(1,0,0);
			mesh.faces[i].setSmGroup(0);
			mesh.faces[i].setVerts(i,i1,lastvertex);
		}
	}
	mesh.InvalidateGeomCache();
}

class ForceObjCreateCallback : public CreateMouseCallBack 
{
	public:
		ForceObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int ForceObjCreateCallback::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) 
	{
		switch(point) 
		{
			case 0:								
				sp0 = m;
				p0  = vpt->GetPointOnCP(m);
				mat.SetTrans(p0);
				break;
			case 1:
				if (ob->ClassID()==Class_ID(GRAVITYOBJECT_CLASS_ID,0)) 
				{
					mat.IdentityMatrix();
					mat.RotateX(PI);
					mat.SetTrans(p0);
				}
				p1  = vpt->GetPointOnCP(m);
				ob->pblock2->SetValue(PB_DISPLENGTH,0,Length(p1-p0)/2.0f);
//				ob->pmapParam->Invalidate();
				gravity_param_blk.InvalidateUI();
				if (msg==MOUSE_POINT) 
				{
					if (Length(m-sp0)<3) 
						return CREATE_ABORT;
					else 
						return CREATE_STOP;
				}
				break;
			}
		} 
		else 
		{
		if (msg == MOUSE_ABORT) 
		{
			return CREATE_ABORT;
		}
		else
		if (msg == MOUSE_FREEMOVE) 
		{
			vpt->SnapPreview(m,m);
		}
	}
	
	return TRUE;
}

static ForceObjCreateCallback forceCreateCB;

CreateMouseCallBack* ForceObject::GetCreateMouseCallBack()
{
	forceCreateCB.ob = this;
	return &forceCreateCB;
}

void ForceObject::InvalidateUI() 
{
//	if (pmapParam) pmapParam->Invalidate();
	gravity_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

//--- GravityObject methods ---------------------------------------

GravityObject::GravityObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(descGravVer1, GRAVPBLOCK_LENGTH, CURRENT_GRAVVERSION));
	pblock2 = NULL;
	gravityDesc.MakeAutoParamBlocks(this);
	assert(pblock2);	
	ff = NULL;
	mf = NULL;
/*
	pblock->SetValue(PB_STRENGTH,0,1.0f);
	pblock->SetValue(PB_HOOPSON,0,0);
*/
}

Modifier *GravityObject::CreateWSMMod(INode *node)
{
	return new GravityMod(node,this);
}

ForceField *GravityObject::GetForceField(INode *node)
{
	GravityField *gf = new GravityField;	
	gf->obj  = this;
	gf->node = node;
	gf->tmValid.SetEmpty();
	gf->fValid.SetEmpty();
	gf->obj->pblock2->GetValue(PB_TYPE,0,gf->type,FOREVER);
	return gf;
}

RefTargetHandle GravityObject::Clone(RemapDir& remap) 
{
	GravityObject* newob = new GravityObject();	
	newob->ReplaceReference(0,pblock2->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
}


ParamDimension *GravityObject::GetParameterDim(int pbIndex) 
{
	switch (pbIndex) 
	{		
		case 0:
		default: return defaultDim;
	}
}

/*
TSTR GravityObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_STRENGTH: 	return GetString(IDS_RB_STRENGTH2);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		default: 			return TSTR(_T(""));
		}
	}

ParamUIDesc *GravityObject::UIDesc()
{
	return descParamGrav;
}

int GravityObject::UIDescLength()
{
	return GRAVPARAMDESC_LENGTH;
}
*/

//--- GravityMod methods -----------------------------------------


GravityMod::GravityMod(INode *node,GravityObject *obj)
{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
}

Interval GravityMod::GetValidity(TimeValue t) 
{
	if (nodeRef) 
	{
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		((GravityObject*)GetWSMObject(t))->pblock2->GetValue(PB_STRENGTH,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} 
	else 
	{
		return FOREVER;
	}
}

class GravityDeformer : public Deformer 
{
	public:		
		Point3 Map(int i, Point3 p) {return p;}
};

static GravityDeformer gdeformer;

Deformer& GravityMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	return gdeformer;
}

RefTargetHandle GravityMod::Clone(RemapDir& remap) 
{
	GravityMod *newob = new GravityMod(nodeRef,(GravityObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}

void GravityMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) 
	{
		force.obj  = (GravityObject*)GetWSMObject(t);
		force.node = nodeRef;
		force.tmValid.SetEmpty();
		force.fValid.SetEmpty();
		if (force.obj != NULL)
			force.obj->pblock2->GetValue(PB_TYPE,t,force.type,FOREVER);
		
		obj->ApplyForceField(&force);
	}
}

// This is an adjustment to forces to make the independent of time scale.
// They were previously dependent on the old 1200 ticks per second constant.
// Note that the constants are being squared because we are dealing with acceleration not velocity.
static float forceScaleFactor = float(1200*1200)/float(TIME_TICKSPERSEC*TIME_TICKSPERSEC);

Point3 GravityField::Force(
		TimeValue t,const Point3 &pos, const Point3 &vel,int index)
	{	
	float strength, decay;
	obj->pblock2->GetValue(PB_DECAY,t,decay,fValid);
	if (decay <0.0f) decay = 0.0f;

	if (!fValid.InInterval(t) || type==FORCE_SPHERICAL || decay!=0.0f) {
		fValid = FOREVER;		
		if (!tmValid.InInterval(t)) {
			tmValid = FOREVER;
			tm = node->GetObjectTM(t,&tmValid);
			}
		fValid &= tmValid;
		obj->pblock2->GetValue(PB_STRENGTH,t,strength,fValid);
		
		if (type==FORCE_PLANAR) {
			force = Normalize(tm.GetRow(2));
			if (decay!=0.0f) {
				float dist = (float)fabs(DotProd(force,pos-tm.GetTrans()));
				strength *= (float)exp(-decay*dist);
				}			
			force *= strength * 0.0001f * forceScaleFactor;
		} else {
			float dist;
			force = tm.GetTrans()-pos;
			dist  = Length(force);
			if (dist!=0.0f) force /= dist;
			if (decay!=0.0f) {				
				strength *= (float)exp(-decay*dist);
				}			
			force *= strength * 0.0001f * forceScaleFactor;
			}
		}	
	return force;
	}

class GravityPostLoadCallback : public PostLoadCallback 
{
	public:
		ParamBlock2PLCB *cb;
		GravityPostLoadCallback(ParamBlock2PLCB *c) {cb=c;}
		void proc(ILoad *iload) 
		{
			DWORD oldVer = ((GravityObject*)(cb->targ))->pblock2->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) 
			{	
 				((GravityObject*)targ)->pblock2->SetValue(PB_HOOPSON,0,0);
			}
			delete this;
		}
};

IOResult GravityObject::Load(ILoad *iload) 
{	
//	iload->RegisterPostLoadCallback(new GravityPostLoadCallback(new ParamBlock2PLCB(gravversions,NUM_OLDVERSIONS,&gravity_param_blk,this,PBLOCK_REF_NO)));
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(gravversions, NUM_OLDVERSIONS, &gravity_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
}

/*  // Bayboro 9/18/01
void* GravityObject::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTOPERATOR: return (IOperatorInterface*)this;
	}
	return Object::GetInterface(id);
}

#define NORMALOP	-1

int GravityObject::NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)
{	
	if (!mf)
		mf = (GravityMod *)CreateWSMMod(node);
	SetUpModifier(t,node);
	Point3 findforce = mf->force.Force(t,part->position,part->velocity,index);
	part->velocity += 10.0f*findforce * dt;
	return (NORMALOP);
}
*/  // Bayboro 9/18/01

GravityObject::~GravityObject()
{	
//	if (mf)
//		delete mf;
	if (ff)
		delete ff;	
	DeleteAllRefsFromMe();
}

void GravityObject::SetUpModifier(TimeValue t,INode *node)
{
	mf->force.obj  = (GravityObject*)(mf->GetWSMObject(t));
	mf->force.node = mf->nodeRef;
	mf->force.tmValid.SetEmpty();
	mf->force.fValid.SetEmpty();
	pblock2->GetValue(PB_TYPE,t,mf->force.type,FOREVER);
}

