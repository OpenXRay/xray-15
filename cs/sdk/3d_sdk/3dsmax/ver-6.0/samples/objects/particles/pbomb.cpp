/**********************************************************************
 *<
	FILE: pbomb.cpp

	DESCRIPTION: Particle Bomb

	CREATED BY: Audrey Peterson

	HISTORY: 12/11/96

	Modified: 9/18/01 Bayboro: removed dependency to EDP

 *>	Copyright (c) 1996, All Rights Reserved.  For Yost Group Inc.
 **********************************************************************/
#include "SuprPrts.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
// #include "pod.h" // Bayboro 9/18/01

class BombMtl: public Material {
	public:
	BombMtl();
	};
static BombMtl swMtl;

#define BOMB_R	float(1)
#define BOMB_G	float(1)
#define BOMB_B	float(0)
const Point3 HOOPCOLOR(1.0f,1.0f,0.0f);

BombMtl::BombMtl():Material() 
{	Kd[0] = BOMB_R;
	Kd[1] = BOMB_G;
	Kd[2] = BOMB_B;
	Ks[0] = BOMB_R;
	Ks[1] = BOMB_G;
	Ks[2] = BOMB_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME;
	selfIllum = (float)1.0;
}

static Class_ID PBOMB_CLASS_ID(0x4c200df3, 0x1a347a77);
static Class_ID PBOMBMOD_CLASS_ID(0xc0609ea, 0x1300b3d);

class PBombMod;

class PBombObject : public SimpleWSMObject // ,IOperatorInterface  // Bayboro 9/18/01
{	
	public:									
		PBombObject();		
		~PBombObject();		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		ForceField *GetForceField(INode *node);
		BOOL SupportsDynamics() {return TRUE;}
		IOResult Load(ILoad *iload);

		// From Animatable		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void MapKeys(TimeMap *map,DWORD flags);
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		// From Animatable		
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return PBOMB_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_PBOMB);}
						
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		
		// From SimpleWSMObject				
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void BuildMesh(TimeValue t);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);

		int DialogID() {return IDD_SW_PARTICLEBOMB;}
		ParamUIDesc *UIDesc();
		int UIDescLength();
		TSTR UIStrName();
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );

//		int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		ForceField *ff;
		void SetUpModifier(TimeValue t,INode *node);
		PBombMod *mf;
	};

IObjParam *PBombObject::ip        = NULL;
IParamMap *PBombObject::pmapParam = NULL;
HWND       PBombObject::hSot      = NULL;

class PBombClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new PBombObject;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_PBOMB_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return PBOMB_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_FORCES);}
	};

static PBombClassDesc PBombDesc;
ClassDesc* GetPBombObjDesc() {return &PBombDesc;}

class PBombMod;

class PBombField : public ForceField {
	public:
		PBombObject *obj;
		TimeValue dtsq,dt;
		INode *node;
		int count;
		Matrix3 tm,invtm;
		Interval tmValid;
		Point3 force;
		Interval fValid;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel, int index);
	};

class PBombMod : public SimpleWSMMod {
	public:				
		PBombField force;
		int seed;

		PBombMod() {}
		PBombMod(INode *node,PBombObject *obj);		
		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_RB_PBOMBMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return PBOMBMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_PBOMBBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};
class PBombModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) {return new PBombMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_PBOMBMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return PBOMBMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static PBombModClassDesc PBombModDesc;
ClassDesc* GetPBombModDesc() {return &PBombModDesc;}

class BombModData : public LocalModData {
public:
	int seed;
	LocalModData *Clone ();
};

LocalModData *BombModData::Clone () {
	BombModData *clone;
	clone = new BombModData ();
	clone->seed=seed;
	return(clone);
}

//--- BombObject Parameter map/block descriptors ------------------
#define PB_SYMMETRY			0
#define PB_CHAOS			1
#define PB_STARTTIME		2
#define PB_LASTSFOR			3
#define PB_DELTA_V			4 
#define PB_DECAY			5
#define PB_DECAYTYPE		6
#define PB_ICONSIZE			7
#define PB_RANGEON			8

static int symIDs[] = {IDC_SP_BLASTSPHR,IDC_SP_BLASTCYL,IDC_SP_BLASTPLAN};
static int decayIDs[] = {IDC_SP_DECAYOFF,IDC_SP_DECAYLIN,IDC_SP_DECAYEXP};
#define SPHERE 0
#define PLANAR 2
#define CYLIND 1

static ParamUIDesc descParamBomb[] = {

	// Blast Symmetry
	ParamUIDesc(PB_SYMMETRY,TYPE_RADIO,symIDs,3),

	// Direction Chaos
	ParamUIDesc(
		PB_CHAOS,
		EDITTYPE_FLOAT,
		IDC_SP_BLASTCHAOS,IDC_SP_BLASTCHAOSSPIN,
		0.0f, 100.0f,
		0.01f,
		stdPercentDim),
	
	// Start Time for Impulse
	ParamUIDesc(
		PB_STARTTIME,
		EDITTYPE_TIME,
		IDC_SP_BLASTSTRT,IDC_SP_BLASTSTRTSPIN,
		-999999999.0f,999999999.0f,
		10.0f),
	
	// Lasts For Time
	ParamUIDesc(
		PB_LASTSFOR,
		EDITTYPE_TIME,
		IDC_SP_BLASTSTOP,IDC_SP_BLASTSTOPSPIN,
		0.0f,999999999.0f,
		10.0f),
	
	// DeltaV
	ParamUIDesc(
		PB_DELTA_V,
		EDITTYPE_FLOAT,
		IDC_SP_BLASTDV,IDC_SP_BLASTDVSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),
	
	// Decay Range
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_UNIVERSE,
		IDC_SP_BLASTDECAY,IDC_SP_BLASTDECAYSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Decay Type
	ParamUIDesc(PB_DECAYTYPE,TYPE_RADIO,decayIDs,3),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_SP_BLAST_ICONSIZE,IDC_SP_BLAST_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Enable Range Indicator
	ParamUIDesc(PB_RANGEON,TYPE_SINGLECHEKBOX,IDC_PBOMB_RANGEON),
	};

#define BOMBPARAMDESC_LENGTH	9

ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//PB_SYMMETRY
	{ TYPE_FLOAT, NULL, FALSE, 1 },	//PB_CHAOS
	{ TYPE_INT, NULL, FALSE, 2 },	//PB_STARTTIME
	{ TYPE_INT, NULL, FALSE, 3 },	//PB_LASTSFOR
	{ TYPE_FLOAT, NULL, TRUE, 4 },	//PB_DELTA_V
	{ TYPE_FLOAT, NULL, TRUE, 5 },	// PB_DECAY
	{ TYPE_INT, NULL, FALSE, 6 },	// PB_DECAYTYPE
	{ TYPE_FLOAT, NULL, FALSE, 7 }};	// PB_ICONSIZE

ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//PB_SYMMETRY
	{ TYPE_FLOAT, NULL, FALSE, 1 },	//PB_CHAOS
	{ TYPE_INT, NULL, FALSE, 2 },	//PB_STARTTIME
	{ TYPE_INT, NULL, FALSE, 3 },	//PB_LASTSFOR
	{ TYPE_FLOAT, NULL, TRUE, 4 },	//PB_DELTA_V
	{ TYPE_FLOAT, NULL, TRUE, 5 },	// PB_DECAY
	{ TYPE_INT, NULL, FALSE, 6 },	// PB_DECAYTYPE
	{ TYPE_FLOAT, NULL, FALSE, 7 },	// PB_ICONSIZE
	{ TYPE_INT, NULL, FALSE, 8 }	// Range Indicator
};	

#define PBLOCK_LENGTH	9

static ParamVersionDesc pbombversions[] = {
	ParamVersionDesc(descVer0,8,0),
	ParamVersionDesc(descVer1,9,1),
	};

#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION	1
static ParamVersionDesc curVersionPBomb(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- Deflect object methods -----------------------------------------

class PBombPostLoadCallback : public PostLoadCallback 
{	public:
		ParamBlockPLCB *cb;
		PBombPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) 
		{	DWORD oldVer = ((PBombObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) 
				((PBombObject*)targ)->pblock->SetValue(PB_RANGEON,0,0);
			delete this;
		}
};

IOResult PBombObject::Load(ILoad *iload)
{	IOResult res = IO_OK;
	iload->RegisterPostLoadCallback(
			new PBombPostLoadCallback(
				new ParamBlockPLCB(pbombversions,NUM_OLDVERSIONS,&curVersionPBomb,this,0)));
	return IO_OK;
}

PBombObject::PBombObject()
{
	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);	
	ff = NULL;
	mf = NULL;

	pblock->SetValue(PB_SYMMETRY,0,0);
	pblock->SetValue(PB_CHAOS,0,0.10f);
	pblock->SetValue(PB_STARTTIME,0,FToTick*30);
	pblock->SetValue(PB_LASTSFOR,0,FToTick);
	pblock->SetValue(PB_DELTA_V,0,1.0f);
	pblock->SetValue(PB_DECAY,0,1000.0f);
	pblock->SetValue(PB_DECAYTYPE,0,1);
	srand(12345);
}

class PBombDlgProc : public ParamMapUserDlgProc {
	public:
		PBombObject *po;

		PBombDlgProc(PBombObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void PBombDlgProc::Update(TimeValue t)
{ int decay;
  po->pblock->GetValue(PB_DECAYTYPE,0,decay,FOREVER);
  HWND hWnd=po->pmapParam->GetHWnd();
  if (decay==0)
	 SpinnerOff(hWnd,IDC_SP_BLASTDECAYSPIN,IDC_SP_BLASTDECAY);
  else
	 SpinnerOn(hWnd,IDC_SP_BLASTDECAYSPIN,IDC_SP_BLASTDECAY);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_BLASTDECAY_TXT), decay);
  EnableWindow(GetDlgItem(hWnd,IDC_PBOMB_RANGEON), decay);
}

BOOL PBombDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{ switch (msg) 
	  {case WM_INITDIALOG: 
		{ Update(t);
		 break;
		}
	   case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_DECAYOFF:
				{ SpinnerOff(hWnd,IDC_SP_BLASTDECAYSPIN,IDC_SP_BLASTDECAY);
				 return TRUE;
				}
			  case IDC_SP_DECAYLIN:
			  case IDC_SP_DECAYEXP:
				{ SpinnerOn(hWnd,IDC_SP_BLASTDECAYSPIN,IDC_SP_BLASTDECAY);
				  return TRUE;
				}
			}
	  }
	return FALSE;
	}

void PBombObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapParam) {		
		// Left over
		pmapParam->SetParamBlock(pblock);
	} else {		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_BOTH),
			DefaultSOTProc,
			GetString(IDS_RB_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descParamBomb,BOMBPARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SW_PARTICLEBOMB),
			GetString(IDS_RB_PARAMETERS),
			0);
		}
		if (pmapParam) pmapParam->SetUserDlgProc(new PBombDlgProc(this));
	}

void PBombObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {		
		DestroyCPParamMap(pmapParam);
		ip->DeleteRollupPage(hSot);
		pmapParam = NULL;		
		}	
	}

ForceField *PBombObject::GetForceField(INode *node)
{
	PBombField *pb = new PBombField;	
	pb->obj  = this;
	pb->node = node;
	pb->tmValid.SetEmpty();
	pb->fValid.SetEmpty();
	pb->dt=GetTicksPerFrame();
	pb->dtsq=pb->dt*pb->dt;
	return pb;
}

void PBombObject::MapKeys(TimeMap *map,DWORD flags)
{	TimeValue TempTime;
	pblock->GetValue(PB_STARTTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_STARTTIME,0,TempTime);
	pblock->GetValue(PB_LASTSFOR,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_LASTSFOR,0,TempTime);
}

void PBombObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float length;
	pblock->GetValue(PB_ICONSIZE,t,length,ivalid);
	float u,zval;
	#define NUM_SEGS 12
	#define NUM_SEGS2 24
	int btype, norvs, norfs, rangeverts=0, rangefaces=0;
	pblock->GetValue(PB_SYMMETRY,0,btype,FOREVER);
//	int dorange,hoops;
//	pblock->GetValue(PB_RANGEON,0,hoops,FOREVER);
//	pblock->GetValue(PB_DECAYTYPE,0,dorange,FOREVER);
//	if (dorange && hoops){ rangeverts=73;rangefaces=72;}
	if (btype<2)
	{ length/=2.0f;
	  if (btype==0)
	  { mesh.setNumVerts((norvs=58)+rangeverts);
	    mesh.setNumFaces((norfs=58)+rangefaces);
		int fbase=22,vbase=21,newv;
	    for (int i=0; i<NUM_SEGS; i++)
	    { u = float(i)/float(NUM_SEGS) * TWOPI;
		  mesh.setVert(i+vbase, Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
	    }
		newv=NUM_SEGS+vbase;
	    for (i=0; i<NUM_SEGS; i++) 
	    { u = float(i)/float(NUM_SEGS) * TWOPI;
		  mesh.setVert(i+newv, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
	    } newv+=NUM_SEGS;
	    for (i=0; i<NUM_SEGS; i++)
	    { u = float(i)/float(NUM_SEGS) * TWOPI;
		  mesh.setVert(i+newv, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
	    }		
		newv+=NUM_SEGS;
	    mesh.setVert(newv, Point3(0.0f, 0.0f, 0.0f));
		int vi=vbase;
	    for (i=fbase; i<newv+1; i++) 
	    { int i1 = vi+1;
	      if ((i1-vbase)%NUM_SEGS==0) i1 -= NUM_SEGS;
	      mesh.faces[i].setEdgeVisFlags(1,0,0);
	      mesh.faces[i].setSmGroup(0);
	      mesh.faces[i].setVerts(vi,i1,newv);
		  vi++;
	    }
		zval=0.85f*length;
	  }
	  else 
	  { mesh.setNumVerts((norvs=21)+rangeverts);
	    mesh.setNumFaces((norfs=22)+rangefaces);
		zval=-0.85f*length;	
	  }
	  float hlen=0.4f*length;
	  mesh.setVert(0,Point3(0.0f,0.0f,zval));
	  for (int i=1;i<9;i++)
	  { u= TWOPI*float(i-1)/8.0f;
	    mesh.setVert(i, Point3((float)cos(u) * hlen,(float)sin(u) * hlen, zval));
	  }
	  zval=1.25f*length;
	  mesh.setVert(9,Point3(0.0f,0.0f,zval));
	  for (i=10;i<18;i++)
	  { u= TWOPI*float(i-10)/8.0f;
	    mesh.setVert(i, Point3((float)cos(u) * hlen,(float)sin(u) * hlen, zval));
	  }
	  int fcount=1;
	  for (i=0;i<8;i++)
	  { mesh.faces[i].setEdgeVisFlags(0,1,0);
	    mesh.faces[i].setSmGroup(0);
	    mesh.faces[i].setVerts(0,fcount,fcount+1);
	    fcount++;
	  }
	  mesh.faces[7].setVerts(0,8,1);
	  fcount=10;
	  for (i=8;i<16;i++)
	  { mesh.faces[i].setEdgeVisFlags(0,1,0);
	    mesh.faces[i].setSmGroup(0);
	    mesh.faces[i].setVerts(9,fcount,fcount+1);
	    fcount++;
	  }
	  mesh.faces[15].setVerts(9,17,10);
	  for (i=16;i<22;i++)
	  { mesh.faces[i].setSmGroup(0);
        mesh.faces[i].setEdgeVisFlags(0,1,0);
	  }
	  mesh.faces[16].setVerts(1,10,14);
      mesh.faces[16].setEdgeVisFlags(1,0,0);
	  mesh.faces[17].setVerts(1,14,5);
	  mesh.faces[18].setVerts(3,12,16);
      mesh.faces[18].setEdgeVisFlags(1,0,0);
	  mesh.faces[19].setVerts(3,16,7);
	  mesh.setVert(18,Point3(0.0f,0.0f,1.5f*length));
	  mesh.setVert(19,Point3(0.25f*length,0.0f,1.75f*length));
	  mesh.setVert(20,Point3(0.75f*length,0.0f,2.0f*length));
      mesh.faces[20].setVerts(9,19,18);
      mesh.faces[20].setEdgeVisFlags(0,1,1);
	  mesh.faces[21].setVerts(9,20,19);
	}
	else
	{ mesh.setNumVerts((norvs=45)+rangeverts);
	  mesh.setNumFaces((norfs=22)+rangefaces);
	  mesh.setVert(0,Point3(length,length,0.0f));
	  mesh.setVert(1,Point3(-length,length,0.0f));
	  mesh.setVert(2,Point3(-length,-length,0.0f));
	  mesh.setVert(3,Point3(length,-length,0.0f));
	  mesh.faces[0].setVerts(0,1,2);
      mesh.faces[0].setEdgeVisFlags(1,1,0);
	  mesh.faces[0].setSmGroup(0);
	  mesh.faces[1].setVerts(0,2,3);
      mesh.faces[1].setEdgeVisFlags(0,1,1);
	  mesh.faces[1].setSmGroup(0);
	  int i,vnum;
	  float r=0.1f*length;
	  Point3 basept=Point3(0.3f*length,0.3f*length,0.5f*length);
	  mesh.setVert(4,basept);
	  mesh.setVert(5,basept+Point3(0.0f,-r,-r));
	  mesh.setVert(6,basept+Point3(0.0f,r,-r));
	  mesh.setVert(7,basept+Point3(r,0.0f,-r));
	  mesh.setVert(8,basept+Point3(-r,0.0f,-r));
	  vnum=9;
	  for (i=4;i<9;i++)
	  { mesh.setVert(vnum,Point3(mesh.verts[i].x,mesh.verts[i].y,-mesh.verts[i].z));
		mesh.setVert(vnum+5,Point3(-mesh.verts[i].x,mesh.verts[i].y,mesh.verts[i].z));
		mesh.setVert(vnum+10,Point3(-mesh.verts[i].x,mesh.verts[i].y,-mesh.verts[i].z));
		mesh.setVert(vnum+15,Point3(mesh.verts[i].x,-mesh.verts[i].y,mesh.verts[i].z));
		mesh.setVert(vnum+20,Point3(mesh.verts[i].x,-mesh.verts[i].y,-mesh.verts[i].z));
		mesh.setVert(vnum+25,Point3(-mesh.verts[i].x,-mesh.verts[i].y,mesh.verts[i].z));
		mesh.setVert(vnum+30,Point3(-mesh.verts[i].x,-mesh.verts[i].y,-mesh.verts[i].z));
		vnum++;
	  }
	  mesh.setVert(44,Point3(0.0f,0.0f,0.0f));
	  int fnum=2;
	  vnum=4;
	  for (i=1;i<9;i++)
	  {	mesh.faces[fnum++].setVerts(vnum,vnum+1,vnum+2);
	    mesh.faces[fnum++].setVerts(vnum,vnum+3,vnum+4);
		vnum+=5;
	  }
	  for (i=2;i<18;i++)
	  { mesh.faces[i].setSmGroup(0);
        mesh.faces[i].setEdgeVisFlags(1,1,1);
	  }
	  for (i=18;i<22;i++)
	  { mesh.faces[i].setSmGroup(0);
        mesh.faces[i].setEdgeVisFlags(0,0,1);
	  }
	  mesh.faces[18].setVerts(4,44,9);
	  mesh.faces[19].setVerts(14,44,19);
	  mesh.faces[20].setVerts(24,44,29);
	  mesh.faces[21].setVerts(34,44,39);
	}
/*	if (dorange && hoops)
	{   int newv;
		pblock->GetValue(PB_DECAY,t,length,ivalid);
		for (int i=0; i<NUM_SEGS2; i++)
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+norvs, Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
	    }
		newv=NUM_SEGS2+norvs;
	    for (i=0; i<NUM_SEGS2; i++) 
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+newv, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
	    } newv+=NUM_SEGS2;
	    for (i=0; i<NUM_SEGS2; i++)
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+newv, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
	    }	
		newv+=NUM_SEGS2;
	    mesh.setVert(newv, Point3(0.0f, 0.0f, 0.0f));
		int vi=norvs;
	    for (i=norfs; i<norfs+rangefaces; i++) 
	    { int i1 = vi+1;
	      if ((i1-norvs)%NUM_SEGS2==0) i1 -= NUM_SEGS2;
	      mesh.faces[i].setEdgeVisFlags(1,0,0);
	      mesh.faces[i].setSmGroup(0);
	      mesh.faces[i].setVerts(vi,i1,newv);
		  vi++;
		}
	}*/

	mesh.InvalidateGeomCache();
}

class PBombObjCreateCallback : public CreateMouseCallBack {
	public:
		PBombObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int PBombObjCreateCallback::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{ vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:								
				// if hidden by category, re-display particles and objects
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0  = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				break;
			case 1:
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				float x=Length(p1-p0);
				ob->pblock->SetValue(PB_ICONSIZE,0,x);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3) return CREATE_ABORT;
					else return CREATE_STOP;
					}
				break;
			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	
	return TRUE;
	}

static PBombObjCreateCallback pbombCreateCB;

CreateMouseCallBack* PBombObject::GetCreateMouseCallBack()
	{
	pbombCreateCB.ob = this;
	return &pbombCreateCB;
	}

void PBombObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

Modifier *PBombObject::CreateWSMMod(INode *node)
	{
	return new PBombMod(node,this);
	}

RefTargetHandle PBombObject::Clone(RemapDir& remap) 
	{
	PBombObject* newob = new PBombObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
	}

ParamDimension *PBombObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case PB_CHAOS:	 return stdPercentDim;
		case PB_ICONSIZE:
		case PB_DECAY:
						return stdWorldDim;
		case PB_STARTTIME:
		case PB_LASTSFOR:
						return stdTimeDim;
		default: return defaultDim;
		}
	}
TSTR PBombObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_SYMMETRY: 	return GetString(IDS_RB_SYMMETRY);
		case PB_CHAOS:		return GetString(IDS_RB_CHAOS);
		case PB_STARTTIME:	return GetString(IDS_RB_STARTTIME);
		case PB_LASTSFOR:	return GetString(IDS_RB_LASTSFOR);
		case PB_DELTA_V:	return GetString(IDS_AP_STRENGTH);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		case PB_DECAYTYPE:	return GetString(IDS_RB_DECAYTYPE);
		case PB_ICONSIZE:	return GetString(IDS_RB_ICONSIZE);
		default: 			return TSTR(_T(""));
		}
	}

ParamUIDesc *PBombObject::UIDesc()
	{
	return descParamBomb;
	}

int PBombObject::UIDescLength()
	{
	return BOMBPARAMDESC_LENGTH;
	}

TSTR PBombObject::UIStrName()
	{
	   return GetString(IDS_RB_BOMBPARAM);
	}


PBombMod::PBombMod(INode *node,PBombObject *obj)
	{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	seed=12345;
	pblock = NULL;
	obRef = NULL;
	}

Interval PBombMod::GetValidity(TimeValue t) 
	{
	if (nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		((PBombObject*)GetWSMObject(t))->pblock->GetValue(PB_DELTA_V,t,f,valid);
		((PBombObject*)GetWSMObject(t))->pblock->GetValue(PB_DECAY,t,f,valid);
		((PBombObject*)GetWSMObject(t))->pblock->GetValue(PB_ICONSIZE,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}

class PBombDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static PBombDeformer gdeformer;

Deformer& PBombMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return gdeformer;
	}

RefTargetHandle PBombMod::Clone(RemapDir& remap) 
	{
	PBombMod *newob = new PBombMod(nodeRef,(PBombObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

void PBombMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
/*		if (!mc.localdata)
		{ mc.localdata=seed; seed+=5;}	  */
		force.obj  = (PBombObject*)GetWSMObject(t);
		force.node = nodeRef;
		force.tmValid.SetEmpty();
		force.fValid.SetEmpty();
		force.dt=GetTicksPerFrame();
		force.dtsq=force.dt*force.dt;
		obj->ApplyForceField(&force);
		}
	}
class BombDrawLineProc:public PolyLineProc {
	GraphicsWindow *gw;
	public:
		BombDrawLineProc() { gw = NULL; }
		BombDrawLineProc(GraphicsWindow *g) { gw = g; }
		int proc(Point3 *p, int n) { gw->polyline(n, p, NULL, NULL, 0, NULL); return 0; }
		int Closed(Point3 *p, int n) { gw->polyline(n, p, NULL, NULL, TRUE, NULL); return 0; }
		void SetLineColor(float r, float g, float b) {gw->setColor(LINE_COLOR,r,g,b);}
		void SetLineColor(Point3 c) {gw->setColor(LINE_COLOR,c);}
		void Marker(Point3 *p,MarkerType type) {gw->marker(p,type);}
	};

static void DrawFalloffSphere(float range, BombDrawLineProc& lp)
{	float u;
	Point3 pt[3],pty[3],ptz[3],first,firsty;
	int nsegs=16;
	
	lp.SetLineColor(GetUIColor(COLOR_END_RANGE));	
	pt[0]=(first= Point3(range,0.0f,0.0f));
	pty[0] =(firsty=Point3(0.0f,range,0.0f));
	ptz[0] = pt[0];
	for (int i=0; i<nsegs; i++)
	{	u = float(i)/float(nsegs) * TWOPI;
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
void PBombObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{	Box3 meshBox;
	Matrix3 mat = inode->GetObjectTM(t);
	box.Init();
	int hoopson,dorange;
	pblock->GetValue(PB_RANGEON,t,hoopson,FOREVER);
	pblock->GetValue(PB_DECAYTYPE,0,dorange,FOREVER);
	if ((hoopson)&&(dorange))
	{ float decay; pblock->GetValue(PB_DECAY,t,decay,FOREVER);
	  if (decay>0.0f)
	  {	float range; range=2.0f*decay;
	    Box3 rangeBox(Point3(-range,-range,-range),Point3(range,range,range)); 
		for(int i = 0; i < 8; i++)	box += mat * rangeBox[i];
	  }
	}
	GetLocalBoundBox(t,inode,vpt,meshBox);	
	for(int i = 0; i < 8; i++)
		box += mat * meshBox[i];
}

int PBombObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = &swMtl;
	Matrix3 mat = inode->GetObjectTM(t);
//	UpdateMesh(t);		
 	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );//removed BC 2/16/99 DB
	gw->setTransform(mat);

	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SPACE_WARPS));
	mesh.render(gw, mtl, NULL, COMP_ALL);
	int dorange,hoopson;
	pblock->GetValue(PB_RANGEON,0,hoopson,FOREVER);
	pblock->GetValue(PB_DECAYTYPE,0,dorange,FOREVER);
	float length;
	if (hoopson && dorange)
	{ pblock->GetValue(PB_DECAY,t,length,FOREVER);
	  float range;
	  range=length;
	  BombDrawLineProc lp(gw);
	  DrawFalloffSphere(range,lp);
	}
	gw->setRndLimits(rlim);
	return(0);
}

Point3 PBombField::Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index)
{	float d,chaos,dv;
	Point3 dlta,xb,yb,zb,center,expv;
	int decaytype,symm;
	Point3 zero=Zero;
		fValid = FOREVER;		
		if (!tmValid.InInterval(t)) 
		{	tmValid = FOREVER;
			tm = node->GetObjectTM(t,&tmValid);
			invtm = Inverse(tm);
		}
		xb=tm.GetRow(0);
		yb=tm.GetRow(1);
		zb=tm.GetRow(2);
		center=tm.GetTrans();
		fValid &= tmValid;
		TimeValue t0,t2,lastsfor;
		obj->pblock->GetValue(PB_STARTTIME,t,t0,fValid);
		obj->pblock->GetValue(PB_LASTSFOR,t,lastsfor,fValid);
		t2=t0+lastsfor;
		dlta=Zero;
		if ((t>=t0)&&(t<=t2))
		{ float L=Length(dlta=pos-center);
		  obj->pblock->GetValue(PB_DECAY,t,d,fValid);
		  obj->pblock->GetValue(PB_DECAYTYPE,t,decaytype,fValid);
		  if ((decaytype==0)||(L<=d))
		  { obj->pblock->GetValue(PB_DELTA_V,t,dv,fValid);
		    obj->pblock->GetValue(PB_CHAOS,t,chaos,fValid);
		    obj->pblock->GetValue(PB_SYMMETRY,t,symm,fValid);
		    Point3 r;
		    if (symm==SPHERE)
		      expv=(r=dlta/L);
	        else if (symm==PLANAR)
		    { L=DotProd(dlta,zb);
			  expv=(L<0.0f?L=-L,-zb:zb);
		    }
		    else
		    { Point3 E;
		      E=DotProd(dlta,xb)*xb+DotProd(dlta,yb)*yb;
		      L=Length(E);
		      expv=E/L;
		    }
		    dlta=(dv*expv)/(float)dtsq;
			if (decaytype==1)
			 dlta*=(d-L)/d;
			else if (decaytype==2)
			 dlta*=(1/(float)exp(L/d));
			if ((!FloatEQ0(chaos))&&(lastsfor==0.0f))
			{ float theta;
			  theta=HalfPI*chaos*RND01();
			  // Martell 4/14/01: Fix for order of ops bug.
			  float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
			  Point3 d=Point3(xtmp,ytmp,ztmp);
			  Point3 c=Normalize(dlta^d);
			  RotateOnePoint(&dlta.x,&zero.x,&c.x,theta);
			}
		  }	else dlta=Zero; 
		}
	return dlta;
	}

/*  // Bayboro 9/18/01
void* PBombObject::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTOPERATOR: return (IOperatorInterface*)this;
	}
	return Object::GetInterface(id);
}

#define DONTCARE	2
#define NORMALOP	-1

int PBombObject::NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)
{	
	if (!mf)
		mf = (PBombMod *)CreateWSMMod(node);
	SetUpModifier(t,node);
	Point3 findforce = mf->force.Force(t,part->position,part->velocity,index);
	part->velocity += 10.0f*findforce * dt;
	return (NORMALOP);
}
*/  // Bayboro 9/18/01

PBombObject::~PBombObject()
{	
	DeleteAllRefsFromMe();
	if (ff)
		delete ff;	
	if (mf)
		delete mf;	
}

void PBombObject::SetUpModifier(TimeValue t,INode *node)
{
	mf->force.obj  = (PBombObject*)(mf->GetWSMObject(t));
	mf->force.node = mf->nodeRef;
	mf->force.tmValid.SetEmpty();
	mf->force.fValid.SetEmpty();
	mf->force.dt = GetTicksPerFrame();
	mf->force.dtsq = mf->force.dt * mf->force.dt;
}
