/**********************************************************************
 *<
	FILE: wind.cpp

	DESCRIPTION:  Wind World Space Modifier

	CREATED BY: Rolf Berteig (as part of gravity.CPP)
	Split out, range added, by Eric Peterson

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

#define		DONTCARE	2

#define FORCE_PLANAR	0
#define FORCE_SPHERICAL	1

class WindMtl: public Material 
{
	public:
	WindMtl();
};

static WindMtl swMtl;
const Point3 WHOOPCOLOR(0.0f,0.0f,0.0f);

#define WIND_R	float(.7)
#define WIND_G	float(0)
#define WIND_B	float(0)

WindMtl::WindMtl():Material() 
{	Kd[0] = WIND_R;
	Kd[1] = WIND_G;
	Kd[2] = WIND_B;
	Ks[0] = WIND_R;
	Ks[1] = WIND_G;
	Ks[2] = WIND_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME|GW_BACKCULL;
	selfIllum = (float)1.0;
}

class WindMod;

class WindObject : public SimpleWSMObject2 // ,IOperatorInterface // Bayboro 9/18/01
{	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;

		WindObject();		
		~WindObject();		
		TCHAR *GetObjectName() {return GetString(IDS_RB_WIND);}
					
		BOOL SupportsDynamics() {return TRUE;}

		// From Animatable		
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return Class_ID(WINDOBJECT_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();	
		
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		ForceField *GetForceField(INode *node);
		
		// From SimpleWSMObject		
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);
		void InvalidateUI();		
		void BuildMesh(TimeValue t);

		int DialogID() {return IDD_WINDPARAM;}
//		ParamUIDesc *UIDesc();
//		int UIDescLength();

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

		// from ref
		IOResult Load(ILoad *iload);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );

//		int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		ForceField *ff;
		void SetUpModifier(TimeValue t,INode *node);
		WindMod *mf;
};

IObjParam *WindObject::ip        = NULL;
//IParamMap *WindObject::pmapParam = NULL;
HWND       WindObject::hSot      = NULL;


//--- ClassDescriptor and class vars ---------------------------------

class WindClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new WindObject;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_WIND_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(WINDOBJECT_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_FORCES);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("Wind"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

static WindClassDesc windDesc;
ClassDesc* GetWindObjDesc() {return &windDesc;}

//--- WindMod -----------------------------------------------------

class WindMod;

class WindField : public ForceField 
{
	public:
		WindObject *obj;
		INode *node;
		Matrix3 tm;
		Interval tmValid;
		Point3 force;
		Interval fValid;
		int type;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index);
		void DeleteThis() {delete this;} // RB 5/12/99
};

class WindMod : public SimpleWSMMod 
{
	public:				
		WindField force;

		WindMod() {}
		WindMod(INode *node,WindObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_WINDMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(WINDMOD_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_WINDBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
};

//--- ClassDescriptor and class vars ---------------------------------

class WindModClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) {return new WindMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_WINDMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(WINDMOD_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
};

static WindModClassDesc windModDesc;
ClassDesc* GetWindModDesc() {return &windModDesc;}

/*
#define PB_STRENGTH		0
#define PB_DECAY		1
#define PB_TYPE			2
#define PB_DISPLENGTH	3
#define PB_TURBULENCE	4
#define PB_FREQUENCY	5
#define PB_SCALE		6
#define PB_HOOPSON		7
*/

// block IDs
enum { wind_params, };

// geo_param param IDs
enum { PB_STRENGTH,PB_DECAY,PB_TYPE,PB_DISPLENGTH,PB_TURBULENCE,PB_FREQUENCY,PB_SCALE,PB_HOOPSON };
//
//
// Parameters

static ParamBlockDesc2 wind_param_blk ( wind_params, _T("WindParameters"),  0, &windDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_WINDPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,

	// params
	PB_STRENGTH,  _T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_STRENGTH2, 
		p_default, 		1.0f,	
		p_ms_default,	1.0f,
		p_range, 		-9999999.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_WIND_STRENGTH,IDC_WIND_STRENGTHSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_DECAY,	  _T("decay"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_DECAY, 
		p_default, 		0.0f,	
		p_ms_default,	1.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_WIND_DECAY,IDC_WIND_DECAYSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_TYPE,	  _T("windtype"),		TYPE_RADIOBTN_INDEX,	0,		IDS_ECP_WINDTYPE,
		p_default,			0,
		p_range,			0, 1, 
		p_ui,				TYPE_RADIO,		2,	IDC_FORCE_PLANAR, IDC_FORCE_SPHERICAL,
		end,

	PB_TURBULENCE,  _T("turbulence"), 	TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_TURBULENCE, 
		p_default, 		0.0f,	
		p_ms_default,	1.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_WIND_TURB,IDC_WIND_TURBSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_FREQUENCY,  _T("frequency"), 	TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_FREQUENCY, 
		p_default, 		0.0f,	
		p_ms_default,	1.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_WIND_FREQ,IDC_WIND_FREQSPIN,SPIN_AUTOSCALE, 
		end, 

	PB_SCALE,	  _T("scale"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_SCALE, 
		p_default, 		1.0f,	
		p_ms_default,	1.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER,EDITTYPE_FLOAT,IDC_WIND_SCALE,IDC_WIND_SCALESPIN,SPIN_AUTOSCALE, 
		end, 

	PB_DISPLENGTH,  _T("iconsize"),		TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_ECP_GRAVITYICONSIZE, 
		p_default, 		10.0f,	
		p_ms_default,	10.0f,
		p_range, 		0.0f, 9999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISPLENGTH,IDC_DISPLENGTHSPIN, SPIN_AUTOSCALE, 
		end, 

	PB_HOOPSON,   _T("hoopson"),		TYPE_BOOL, 0, IDS_ECP_HOOPSON,
		p_default, 		FALSE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_EP_HOOPSON, 
		end,

	end
	);

/*
static int typeIDs[] = {IDC_FORCE_PLANAR,IDC_FORCE_SPHERICAL};

static ParamUIDesc descParamWind[] = {
	// Strength
	ParamUIDesc(
		PB_STRENGTH,
		EDITTYPE_FLOAT,
		IDC_WIND_STRENGTH,IDC_WIND_STRENGTHSPIN,
		-9999999.0f, 9999999.0f,
		0.01f),

	// Decay
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_FLOAT,
		IDC_WIND_DECAY,IDC_WIND_DECAYSPIN,
		0.0f, 9999999.0f,
		0.001f),

	// Force type
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),

	// Turbulence
	ParamUIDesc(
		PB_TURBULENCE,
		EDITTYPE_FLOAT,
		IDC_WIND_TURB,IDC_WIND_TURBSPIN,
		0.0f, 9999999.0f,
		0.01f),

	// Frequency
	ParamUIDesc(
		PB_FREQUENCY,
		EDITTYPE_FLOAT,
		IDC_WIND_FREQ,IDC_WIND_FREQSPIN,
		0.0f, 9999999.0f,
		0.01f),

	// Scale
	ParamUIDesc(
		PB_SCALE,
		EDITTYPE_FLOAT,
		IDC_WIND_SCALE,IDC_WIND_SCALESPIN,
		0.0f, 9999999.0f,
		0.01f),

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

#define WINDPARAMDESC_LENGTH	8

ParamBlockDescID descWindVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, FALSE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
};

ParamBlockDescID descWindVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, FALSE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
};

#define WINDPBLOCK_LENGTH		8

//#define CURRENT_WINDVERSION		1
#define NUM_OLDVERSIONS			2

static ParamVersionDesc windversions[] = 
{
	ParamVersionDesc(descWindVer0,7,0),
	ParamVersionDesc(descWindVer1,8,1),
};

//static ParamVersionDesc windcurVersion(descWindVer1,WINDPBLOCK_LENGTH,CURRENT_WINDVERSION);

//--- WindObject Methods ---------------------------------------------

void WindObject::BeginEditParams(
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

	windDesc.BeginEditParams(ip, this, flags, prev);

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

void WindObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	windDesc.EndEditParams(ip, this, flags, next);

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
{	Point3 pt[5];
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
{	float u;
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

int WindObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
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
	gw->setRndLimits(rlim);
	return(0);
}

void WindObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
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

void WindObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float length;
	int type;	
	pblock2->GetValue(PB_DISPLENGTH,t,length,ivalid);
	pblock2->GetValue(PB_TYPE,t,type,ivalid);
	int norvs,norfs,extraverts,extrafaces;
	float decay,range;
	pblock2->GetValue(PB_DECAY,t,decay,ivalid);
	range=decay;
	int hoopson;
	pblock2->GetValue(PB_HOOPSON,t,hoopson,FOREVER);
	range=(decay > 0.0f?0.6931472f / decay:0.0f);
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

class WindObjCreateCallback : public CreateMouseCallBack {
	public:
		WindObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int WindObjCreateCallback::proc(
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
				wind_param_blk.InvalidateUI();
				if (msg==MOUSE_POINT) 
				{
					if (Length(m-sp0)<3) return CREATE_ABORT;
					else return CREATE_STOP;
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

static WindObjCreateCallback forceCreateCB;

CreateMouseCallBack* WindObject::GetCreateMouseCallBack()
{
	forceCreateCB.ob = this;
	return &forceCreateCB;
}

void WindObject::InvalidateUI() 
{
//	if (pmapParam) pmapParam->Invalidate();
	wind_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

//--- WindObject methods ---------------------------------------

WindObject::WindObject()
	{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(descWindVer1, WINDPBLOCK_LENGTH, CURRENT_WINDVERSION));
	pblock2 = NULL;
	windDesc.MakeAutoParamBlocks(this);
	assert(pblock2);	
	ff = NULL;
	mf = NULL;
/*
	pblock2->SetValue(PB_STRENGTH,0,1.0f);
	pblock2->SetValue(PB_SCALE,0,1.0f);
	pblock2->SetValue(PB_HOOPSON,0,0);
*/
}

Modifier *WindObject::CreateWSMMod(INode *node)
{
	return new WindMod(node,this);
}

ForceField *WindObject::GetForceField(INode *node)
{
	WindField *wf = new WindField;	
	wf->obj  = this;
	wf->node = node;
	wf->tmValid.SetEmpty();
	wf->fValid.SetEmpty();
	wf->obj->pblock2->GetValue(PB_TYPE,0,wf->type,FOREVER);
	return wf;
}

RefTargetHandle WindObject::Clone(RemapDir& remap) 
{
	WindObject* newob = new WindObject();
	newob->ReplaceReference(0,pblock2->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
}

ParamDimension *WindObject::GetParameterDim(int pbIndex) 
{
	switch (pbIndex) 
	{		
		case 0:
		default: return defaultDim;
	}
}

/*
TSTR WindObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {		
		case PB_STRENGTH: 	return GetString(IDS_RB_STRENGTH2);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		case PB_TURBULENCE:	return GetString(IDS_RB_TURBULENCE);
		case PB_FREQUENCY:	return GetString(IDS_RB_FREQUENCY);
		case PB_SCALE:		return GetString(IDS_RB_SCALE);
		default: 			return TSTR(_T(""));
		}
	}

ParamUIDesc *WindObject::UIDesc()
	{
	return descParamWind;
	}

int WindObject::UIDescLength()
	{
	return WINDPARAMDESC_LENGTH;
	}
*/

//--- WindMod methods -----------------------------------------


WindMod::WindMod(INode *node,WindObject *obj)
	{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
	}

Interval WindMod::GetValidity(TimeValue t) 
{
	if (nodeRef) 
	{
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		((WindObject*)GetWSMObject(t))->pblock2->GetValue(PB_STRENGTH,t,f,valid);		
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} 
	else 
	{
		return FOREVER;
	}
}

class WindDeformer : public Deformer 
{
	public:		
		Point3 Map(int i, Point3 p) {return p;}
};

static WindDeformer wdeformer;

Deformer& WindMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	return wdeformer;
}

RefTargetHandle WindMod::Clone(RemapDir& remap) 
{
	WindMod *newob = new WindMod(nodeRef,(WindObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}

void WindMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) 
	{
		force.obj  = (WindObject*)GetWSMObject(t);
		force.node = nodeRef;
		force.tmValid.SetEmpty();
		force.fValid.SetEmpty();
		if (force.obj != NULL)
			force.obj->pblock2->GetValue(PB_TYPE,t,force.type,FOREVER);
		obj->ApplyForceField(&force);
	}
}

static float RTurbulence(Point3 p,float freq)
{
	return noise3(p*freq);
}

static float forceScaleFactor = float(1200*1200)/float(TIME_TICKSPERSEC*TIME_TICKSPERSEC);

Point3 WindField::Force(
		TimeValue t,const Point3 &pos, const Point3 &vel,int index)
	{	
	float strength, decay, turb;
	obj->pblock2->GetValue(PB_DECAY,t,decay,fValid);
	obj->pblock2->GetValue(PB_TURBULENCE,t,turb,FOREVER);
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
			force = pos-tm.GetTrans();
			dist  = Length(force);
			if (dist!=0.0f) force /= dist;
			if (decay!=0.0f) {				
				strength *= (float)exp(-decay*dist);
				}			
			force *= strength * 0.0001f * forceScaleFactor;
			}
		}	
	if (turb!=0.0f) {
		float freq, scale;
		Point3 tf, pt = pos-tm.GetTrans(), p;
		obj->pblock2->GetValue(PB_FREQUENCY,t,freq,FOREVER);
		obj->pblock2->GetValue(PB_SCALE,t,scale,FOREVER);
		freq *= 0.01f;
		turb *= 0.0001f * forceScaleFactor;

		p    = pt;
		p.x  = freq * float(t);
		tf.x = RTurbulence(p,scale);
		p    = pt;
		p.y  = freq * float(t);
		tf.y = RTurbulence(p,scale);
		p    = pt;
		p.z  = freq * float(t);
		tf.z = RTurbulence(p,scale);

		return force + (turb*tf);
	} else {
		return force;
		}
	}

class WindPostLoadCallback : public PostLoadCallback 
{
	public:
		ParamBlock2PLCB *cb;
		WindPostLoadCallback(ParamBlock2PLCB *c) {cb=c;}
		void proc(ILoad *iload) 
		{
			DWORD oldVer = ((WindObject*)(cb->targ))->pblock2->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) 
			{	
 				((WindObject*)targ)->pblock2->SetValue(PB_HOOPSON,0,0);
			}
			delete this;
		}
};

IOResult WindObject::Load(ILoad *iload) 
{	iload->RegisterPostLoadCallback(new ParamBlock2PLCB(windversions,NUM_OLDVERSIONS,&wind_param_blk,this,PBLOCK_REF_NO));
	return IO_OK;
}

/*  // Bayboro 9/18/01
void* WindObject::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTOPERATOR: return (IOperatorInterface*)this;
	}
	return Object::GetInterface(id);
}

#define NORMALOP	-1

int WindObject::NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)
{	
	if (!mf)
		mf = (WindMod *)CreateWSMMod(node);
	SetUpModifier(t,node);
	Point3 findforce = mf->force.Force(t,part->position,part->velocity,index);
	part->velocity += 10.0f*findforce * dt;
	return (NORMALOP);
}
*/  // Bayboro 9/18/01

WindObject::~WindObject()
{	
	if (mf)
		delete mf;	
	if (ff)
		delete ff;	
	DeleteAllRefsFromMe();
}

void WindObject::SetUpModifier(TimeValue t,INode *node)
{
	mf->force.obj  = (WindObject*)(mf->GetWSMObject(t));
	mf->force.node = mf->nodeRef;
	mf->force.tmValid.SetEmpty();
	mf->force.fValid.SetEmpty();
	pblock2->GetValue(PB_TYPE,t,mf->force.type,FOREVER);
}

