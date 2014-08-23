/**********************************************************************
 *<									  

	FILE: PArray.CPP
	DESCRIPTION: Particle Array Systems using Input Geometry

	CREATED BY: Audrey Peterson	

	HISTORY: created November 1996
	Geometry Instancing 1/1997

 *>	Copyright (c) 1996 for and assigned to Yost Group, All Rights Reserved.
 **********************************************************************/
#include <io.h>
#include "SuprPrts.h"
#include "iparamm.h"
#include "interpik.h"
#include "texutil.h"
#include "stdmat.h"
#include "macrorec.h"
//#include "metapart.h"

// russom - 10/11/01
#ifndef NO_PARTICLES_PARRAY

#define EPSILON 0.0001f
#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256
#define PBLK		0
#define CUSTNODE 		1
#define DISTNODE 		2

#define BASER 3

#define DIRTRAVEL 1

typedef Tab<Mtl *>Mtllst;
typedef struct {
  Tab<Face> f;
  Tab<Point3> v;
  Tab<UVVert> tv;
  Tab<TVFace> tvf;
} CustLst;
class PMesh
{  public:
	PMesh();
	~PMesh();
    int numVerts,numFaces,numtvFace;
	Point3 *verts;
	Face *faces;
	TVFace *tvFace;
	BOOL 	setNumVerts(int ct, BOOL keep=FALSE);
	int		getNumVerts(void) const	{ return numVerts; }
	BOOL 	setNumFaces(int ct, BOOL keep=FALSE);
	int		getNumFaces(void) const	{ return numFaces; }
	BOOL 	setNumtvFace(int ct, BOOL keep=FALSE);
	int		getNumtvFace(void) const { return numtvFace; }
};
PMesh::PMesh()
{  numVerts=0;
   verts=NULL;
   faces=NULL;
   tvFace=NULL;
   numFaces=0;
   numtvFace=0;
}
PMesh::~PMesh()
{  if (verts){delete[] verts;verts=NULL;}
   if (tvFace){
	 delete[] tvFace;
	 tvFace=NULL;}
   if (faces){delete[] faces;faces=NULL;}
}
BOOL PMesh::setNumVerts(int ct, BOOL keep)
{ Point3 *tmp;
int maxv=(ct>numVerts?ct:numVerts);
 if (keep)
	{tmp=verts;
     verts=new Point3[ct];
	 if ((tmp!=NULL)&&(verts!=NULL)) memcpy(verts,tmp,sizeof(Point3)*maxv);
	 if (tmp!=NULL) delete[] tmp;
    }
   else 
   {if (verts) delete[] verts;
    if (ct>0) verts=new Point3[ct];
	else verts=NULL;
   }
   numVerts=(verts==NULL?0:ct);
   return(verts!=NULL);
};
BOOL PMesh::setNumFaces(int ct, BOOL keep)
{ Face *tmp;
int maxv=(ct>numFaces?ct:numFaces);
 if (keep)
	{tmp=faces;
     faces=new Face[ct];
	 if ((tmp!=NULL)&&(faces!=NULL)) memcpy(faces,tmp,sizeof(Face)*maxv);
	 if (tmp!=NULL) delete[] tmp;
    }
   else 
   {if (faces) delete[] faces;
    if (ct>0) faces=new Face[ct];
	else faces=NULL;
   }
   numFaces=(faces==NULL?0:ct);
   return(faces!=NULL);
};
BOOL PMesh::setNumtvFace(int ct, BOOL keep)
{ TVFace *tmp;
int maxv=(ct>numtvFace?ct:numtvFace);
 if (keep)
	{tmp=tvFace;
     tvFace=new TVFace[ct];
	 if ((tmp!=NULL)&&(tvFace!=NULL)) memcpy(tvFace,tmp,sizeof(TVFace)*maxv);
	 if (tmp!=NULL) delete[] tmp;
    }
   else 
   {if (tvFace) delete[] tvFace;
    if (ct>0) tvFace=new TVFace[ct];
	else tvFace=NULL;
   }
   numtvFace=(tvFace==NULL?0:ct);
   return(tvFace!=NULL);
};

typedef struct{
 float Vsz,Ts,Ts0,LamTs,A,LamA,To;
 float M,Dis,Fo,Inf,Mltvar,pvar;
 int themtl,showframe,gennum,frommesh,SpVar;
 TimeValue L,DL,persist;
 MtlID pmtl;
 Point3 wbb[8];
 Point3	V,Ve,W,RV,tv,start;
} SavePt;

typedef struct {
 Point3 Axis;
 TimeValue persist;
 float axisvar,div,pvar; 
 int axisentered;
}VelDir2;
typedef struct {
 float Size,VSz,VSpin,Phase,VPhase;
 float bstr,bstrvar,Speed,VSpeed;
 float ToAmp,VToAmp;
 float ToPhase,VToPhase,VToPeriod;
 TimeValue Spin,ToPeriod,Life,Vl;
} VelDir;
static Class_ID PArray_CLASS_ID(0xe3c25b5, 0x109d1659);

class PickOperand;
class PArrayParticle;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//        WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class PArrayParticleDraw : public CustomParticleDisplay {
	public:
//		float size,VSz,
		BOOL firstpart;
		PArrayParticle *obj;
		int disptype,ptype,bb,anifr,anioff;
		boxlst *bboxpt;
		TimeValue t;
		InDirInfo indir;

		PArrayParticleDraw() {obj=NULL;}
		BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i);
	};

typedef struct{
float M,Vsz;
Point3 Ve,vel,pos;
Point3 wbb[8];
}CacheDataType;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//      WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class PArrayParticle : public SimpleParticle, IParamArray {
	public:
		PArrayParticleDraw thePArrayDraw;
		PArrayParticle();
		~PArrayParticle();
		static IParamMap *pmapParam;
		static IParamMap *pmapPGen;
		static IParamMap *pmapPType;
		static IParamMap *pmapPSpin;
		static IParamMap *pmapEmitV;
		static IParamMap *pmapBubl;
		static IParamMap *pmapSpawn;
		int stepSize,size,oldtvnum,lastrnd,emitrnd;
		static custsettings;
		BOOL cancelled,wasmulti,storernd;
		static BOOL creating;
		BOOL fromgeom;
		INode *distnode,*custnode,*cnode;
		CacheDataType *storefrag;
		TSTR distname,custname;
		ULONG dflags;
		int fragflags;
		BOOL doTVs;
		static AName *NameLst;
		static HWND hParams2,hptype,hgen,hparam,hrot,hbubl,spawn;
		static PickOperand pickCB;
		static ICustEdit *custCtrlEdit;
		Mtl *origmtl;

		// to fix #182223 & 270224, add these members
		Matrix3 lastTM;
		TimeValue lastTMTime;

		BOOL GenerateNotGeom(TimeValue t,TimeValue lastt,int c,int counter,INode *distnode,int type,Matrix3 tm,Matrix3 nxttm);
		void GetInfoFromObject(float thick,int *c,INode *distnode,TimeValue t,TimeValue lastt);
		void GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box); 
		void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box);
		void RendGeom(Mesh *pm,Matrix3 itm,int maxmtl,int maptype,BOOL eitmtl,float mval,PArrayParticleDraw thePArrayDraw,TVFace defface,BOOL notrend);

		Tab<int> nmtls;
		void DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emits,int &oldcnt);
		void SetUpList();
		void AddToList(INode *newnode,int i,BOOL add);
		void DeleteFromList(int nnum,BOOL all);
		Tab<INode*> nlist;
		Tab<int> llist;
		int deftime;
		int maincount;
		int NumRefs() {return BASER+nlist.Count();}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
		void SetUpLifeList();
		void AddToLifeList(int newlife);
		void DeleteFromLifeList(int nnum);
		void ShowName(int dist);
		int CountLive();
		int rcounter,vcounter;
		oldipc lc;
		static IObjParam *ip;
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		TimeValue ParticleLife(TimeValue t, int i);
		// Animatable methods		
		void DeleteThis() {delete this;}
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		void MapKeys(TimeMap *map,DWORD flags);
		int RenderBegin(TimeValue t, ULONG flags);		
		int RenderEnd(TimeValue t);
		
		// From SimpleParticle
		void BuildEmitter(TimeValue t, Mesh& amesh);
		Interval GetValidity(TimeValue t);		
		void InvalidateUI();
		BOOL EmitterVisible();		
		MarkerType GetMarkerType();	
		SavePt *sdata;
		Mesh *pmesh;
		// From BaseObject
		TCHAR *GetObjectName();
		void BirthParticle(INode *node,TimeValue bt,int index,VelDir2 *ptvel2,VelDir ptvel,Matrix3 tmlast);
		BOOL ComputeParticleStart(TimeValue t0,TimeValue lastt,INode *node,int c);
		int IsInstanceDependent() {return 1;}

		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

		// Animatable methods		
		Class_ID ClassID() {return PArray_CLASS_ID;} 

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From Simple Particle
		void UpdateParticles(TimeValue t,INode *node);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void GetFilename(TCHAR *filename);
		void SetupTargetList();
		int SaveSettings(int overwrite,TCHAR *newname);
		int GetSettings(int setnum,TCHAR *newname);
		int RemSettings(int setnum,TCHAR *newname);
		Point3 ParticlePosition(TimeValue t,int i);
		Point3 ParticleVelocity(TimeValue t,int i);		
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);

		Mesh *cmesh,*dispmesh;
		Box3 *cmbb;
		Point3 boxcenter;
		int CustMtls;
		TimeLst times;
		void GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype,int fragflags);
		void TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void GetMesh(TimeValue t,int subtree,int custmtl,int fragflags);
		void GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void GetallBB(INode *custnode,int subtree,TimeValue t,int fragflags);
		void AssignMtl(INode *node,INode *topnode,int subtree,TimeValue t,int fragflags);
		void DoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void RetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void GetSubs(INode *node,INode *topnode,int subtree,TimeValue t,int fragflags);
		void CntDoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void CntRetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		BOOL backpatch;
		Mtllst mttab;
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt);
		TimeValue dispt;
		void MovePart(int j,TimeValue dt,BOOL fullframe,int tpf);
		void ResetSystem(TimeValue t,BOOL full=TRUE);
	};

class PickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PArrayParticle *po;
		int dodist,repi;
		
		PickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};
//--- ClassDescriptor and class vars ---------------------------------

class PArrayClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new PArrayParticle;}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return PArray_CLASS_ID;}
	const TCHAR* 	Category(); 
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static PArrayClassDesc PArrayDesc;
ClassDesc* GetPArrayDesc() {return &PArrayDesc;}

//--- ClassDescriptor and class vars ---------------------------------
IParamMap *PArrayParticle::pmapParam;
IParamMap *PArrayParticle::pmapPGen;
IParamMap *PArrayParticle::pmapPType;
IParamMap *PArrayParticle::pmapPSpin;
IParamMap *PArrayParticle::pmapEmitV;
IParamMap *PArrayParticle::pmapBubl;
IParamMap *PArrayParticle::pmapSpawn;
IObjParam *PArrayParticle::ip    = NULL;
BOOL PArrayParticle::creating    = FALSE;
PickOperand PArrayParticle::pickCB;
ICustEdit *PArrayParticle::custCtrlEdit=NULL;
int PArrayParticle::custsettings=0;
AName *PArrayParticle::NameLst=NULL;
HWND PArrayParticle::hParams2;
HWND PArrayParticle::hgen;
HWND PArrayParticle::hptype;
HWND PArrayParticle::hparam;
HWND PArrayParticle::hrot;
HWND PArrayParticle::hbubl;
HWND PArrayParticle::spawn;

class PAObjectListRestore : public RestoreObj {
	public:   		
		PArrayParticle *po;
		Tab<INode*> unodes;
		Tab<INode*> rnodes;
		int lnum,lnum2;
		PAObjectListRestore(PArrayParticle *p) 
		{  po=p;
		   unodes=p->nlist;
		}
		void Restore(int isUndo)
		{	if (isUndo) {
				rnodes = po->nlist;
				}
			po->nlist = unodes;
			po->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		}
		void Redo() 
		{	po->nlist = rnodes;
			po->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		}
	};

class PALifeListRestore : public RestoreObj {
	public:   		
		PArrayParticle *po;
		Tab<int> utimes;
		Tab<int> rtimes;
		PALifeListRestore(PArrayParticle *p) 
		{ po=p;
		  utimes=p->llist;
		}
		void Restore(int isUndo)
		{ if (isUndo) { rtimes = po->llist;	}
		  po->llist=utimes;
		  if (po->ip) po->SetUpLifeList();
		}
		void Redo() 
		{ po->llist = rtimes;
		  if (po->ip) po->SetUpLifeList();
		}
	};


class CreatePAPartDelNode : public RestoreObj {
	public:   		
		PArrayParticle *obj;
		TSTR name;
		int dist;
		CreatePAPartDelNode(PArrayParticle *o, TSTR n,int d) {
			obj = o; name=TSTR(n); dist=d;
			}
		void Restore(int isUndo)
		{if (dist) obj->distname=name; else obj->custname = name;
		 obj->ShowName(dist);
			}
		void Redo() 
			{  if (dist) obj->distname==TSTR(_T(""));else obj->custname=TSTR(_T(""));
		if (((dist)&&(obj->hptype))||((!dist)&&(obj->hparam)))
		{ TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + TSTR(GetString(IDS_AP_NONE));
		  if (dist)
		    SetWindowText(GetDlgItem(obj->hparam, IDC_SP_CHUNKPICKOBJECT), name);
		  else
		    SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);
		}
		}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};
class CreatePAPartPickNode : public RestoreObj {
	public:   		
		PArrayParticle *obj;
		TSTR name,name2;int dist;
		CreatePAPartPickNode(PArrayParticle *o, TSTR n,TSTR n1,int d) {
			obj = o; name=TSTR(n);name2=TSTR(n1);dist=d;
			}
		void Restore(int isUndo) {
			if (dist)
			{ if (obj->distnode) obj->distname = name;
			else  obj->distname=TSTR(_T(""));}
			else
			{ if (obj->custnode) obj->custname = name;
			else  obj->custname=TSTR(_T(""));}
			obj->ShowName(dist);
			}
		void Redo() 
		{ 
		if (((dist)&&(obj->hptype))||((!dist)&&(obj->hparam)))
		{ TSTR name;
		  if (dist) obj->distname=name2;
		  else  obj->custname = name2;
		  name=TSTR(GetString(IDS_AP_OBJECTSTR))+(_tcslen(name2)>0 ? name2 : TSTR(GetString(IDS_AP_NONE)));
		  if (dist)
		    SetWindowText(GetDlgItem(obj->hparam, IDC_SP_CHUNKPICKOBJECT), name);
		  else
		    SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);
		}
			}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};


#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)


//--- Parameter map/block descriptors -------------------------------

#define PB_DISTRIBUTION		0
#define PB_EMITTERCOUNT		1
#define PB_SPEED			2
#define PB_SPEEDVAR			3
#define PB_ANGLEDIV			4

#define PB_BIRTHMETHOD		5
#define PB_PBIRTHRATE		6
#define PB_PTOTALNUMBER		7
#define PB_EMITSTART		8
#define PB_EMITSTOP			9
#define PB_DISPUNTIL		10
#define PB_LIFE				11
#define PB_LIFEVAR			12
#define PB_SUBFRAMEMOVE		13
#define PB_SUBFRAMETIME		14
#define PB_SIZE				15
#define PB_SIZEVAR			16
#define PB_GROWTIME			17
#define PB_FADETIME			18
#define PB_RNDSEED			19
#define PB_EMITRWID			20
#define PB_EMITRHID			21

#define PB_PARTICLECLASS	22
#define PB_PARTICLETYPE		23
#define PB_METATENSION		24
#define PB_METATENSIONVAR	25
#define PB_METACOURSE		26
#define PB_METAAUTOCOARSE	27
#define PB_FRAGTHICKNESS	28
#define PB_FRAGMETHOD		29
#define PB_FRAGCOUNT		30
#define PB_SMOOTHANG		31
#define	PB_USESUBTREE		32
#define PB_ANIMATIONOFFSET  33
#define PB_OFFSETAMOUNT     34
#define PB_VIEWPORTSHOWS	35
#define PB_DISPLAYPORTION	36
#define PB_MAPPINGTYPE		37
#define PB_MAPPINGTIME		38
#define PB_MAPPINGDIST		39
#define PB_CUSTOMMATERIAL	40
#define PB_EMAT				41
#define PB_BMAT				42
#define PB_FMAT				43

#define PB_SPINTIME			44
#define PB_SPINTIMEVAR		45
#define PB_SPINPHASE		46
#define PB_SPINPHASEVAR		47
#define PB_SPINAXISTYPE		48
#define PB_SPINAXISX		49
#define PB_SPINAXISY		50
#define PB_SPINAXISZ		51
#define PB_SPINAXISVAR		52

#define PB_EMITVINFL		53
#define PB_EMITVMULT		54
#define PB_EMITVMULTVAR		55

#define PB_BUBLAMP			56
#define PB_BUBLAMPVAR		57
#define PB_BUBLPER			58
#define PB_BUBLPERVAR		59
#define PB_BUBLPHAS			60
#define PB_BUBLPHASVAR		61

#define PB_STRETCH			62

#define PB_SPAWNTYPE		63
#define PB_SPAWNGENS		64
#define PB_SPAWNCOUNT		65
#define PB_SPAWNDIRCHAOS	66
#define PB_SPAWNSPEEDCHAOS	67
#define PB_SPAWNSPEEDSIGN	68
#define PB_SPAWNINHERITV	69
#define PB_SPAWNSCALECHAOS	70
#define PB_SPAWNSCALESIGN	71
#define PB_SPAWNLIFEVLUE	72
#define PB_SPAWNSPEEDFIXED	73
#define PB_SPAWNSCALEFIXED	74
#define PB_METACOURSEV			75
#define PB_SUBFRAMEROT			76
#define PB_SPAWNPERCENT			77
#define PB_SPAWNMULTVAR			78
#define PB_PANOTDRAFT			79
#define PB_USESELECTED				80
#define PB_PASPAWNDIEAFTER			81
#define PB_PASPAWNDIEAFTERVAR		82
	
#define PB_PAIPCOLLIDE_ON			83
#define PB_PAIPCOLLIDE_STEPS		84
#define PB_PAIPCOLLIDE_BOUNCE		85
#define PB_PAIPCOLLIDE_BOUNCEVAR	86

// render types
#define RENDMETA    8
#define RENDGEOM	9
#define RENDCGEOM	10

//
//
// Parameters

static int distributeIDs[] = {IDC_SP_PASURFACE,IDC_SP_PAEDGES,IDC_SP_PAVERTICES,IDC_SP_PAPOINTS,IDC_SP_PAFACEC};

#define UNIFORM 0
#define EDGES 1
#define VERTS 2
#define EMITS 3
#define FACEC 4

#define ISSTD 0
#define METABALLS 1
#define BYGEOM 2
#define INSTGEOM 3

static int countIDs[] = {IDC_SP_GENUSERATE,IDC_SP_GENUSETTL};

static int particleclassIDs[] = {IDC_SP_TYPESTD,IDC_SP_TYPEMET,IDC_SP_TYPECHUNKS,IDC_SP_TYPEINSTANCE};

static int particletypeIDs[] = {IDC_SP_TYPETRI,IDC_SP_TYPECUB,IDC_SP_TYPESPC,IDC_SP_TYPEFAC,
								IDC_SP_TYPEPIX,IDC_SP_TYPETET,IDC_SP_TYPE6PNT,IDC_SP_TYPESPHERE};

static int fragmenttypeIDs[] = {IDC_SP_CHUNKFACES,IDC_SP_CHUNKQTY,IDC_SP_CHUNKSMOOTH};

static int spindirectionIDs[] = {IDC_AP_PARTICLEDIRRND,IDC_AP_PARTICLEDIRTRAVL,IDC_AP_PARTICLEDIRUSER};

static int animateoffsetIDs[] = {IDC_AP_NOANIOFF,IDC_AP_ANIOFFBIRTH,IDC_AP_ANIOFFRND};

#define FRAGFACE 0
#define FRAGRND 1

static int viewportoptionIDs[] = {IDC_SP_VIEWDISPDOT,IDC_SP_VIEWDISPTIK,IDC_SP_VIEWDISPMESH,IDC_SP_VIEWDISPBOX};

static int mappingIDs[] = {IDC_SP_MAPTIME,IDC_SP_MAPDIST};

static int custmtlIDs[] = {IDC_SP_MAPCUSTOMEMIT,IDC_SP_MAPCUSTOMDIST,IDC_SP_MAPCUSTOMINST};

// Dialog Unique to Particle Array
static ParamUIDesc descParamPArray[] = {
	
	// Distribution Method
	ParamUIDesc(PB_DISTRIBUTION,TYPE_RADIO,distributeIDs,5),

	// Emitter Count
	ParamUIDesc(
		PB_EMITTERCOUNT,
		EDITTYPE_INT,
		IDC_SP_PAPOINTSEDIT,IDC_SP_PAPOINTSEDITSPIN,
		1.0f,65000.0f,
		1.0f),

	// Use Subobjects
	ParamUIDesc(PB_USESELECTED,TYPE_SINGLECHEKBOX,IDC_SP_USESUBOBJS),

	// Emitter Width
	ParamUIDesc(
		PB_EMITRWID,
		EDITTYPE_UNIVERSE,
		IDC_SP_EMITWID,IDC_SP_EMITWIDSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Hide Emitter
	ParamUIDesc(PB_EMITRHID,TYPE_SINGLECHEKBOX,IDC_SP_EMITHID),

	// Display type
	ParamUIDesc(PB_VIEWPORTSHOWS,TYPE_RADIO,viewportoptionIDs,4),

	// Particle Display Portion
	ParamUIDesc(
		PB_DISPLAYPORTION,
		EDITTYPE_FLOAT,
		IDC_SP_GENDISP,IDC_SP_GENDISPSPIN,
		0.0f,100.0f,
		0.1f,
		stdPercentDim),
};
#define PARAMPARRAY_LENGTH 7

// Common Dialog for Particle Generation
static ParamUIDesc descParamPGen[] = {

	// Distribution Method
	ParamUIDesc(PB_BIRTHMETHOD,TYPE_RADIO,countIDs,2),

	// Particle Birth Rate
	ParamUIDesc(
		PB_PBIRTHRATE,
		EDITTYPE_INT,
		IDC_SP_GENRATE,IDC_SP_GENRATESPIN,
		0.0f,65000.0f,
		1.0f),

	// Particle Total Count
	ParamUIDesc(
		PB_PTOTALNUMBER,
		EDITTYPE_INT,
		IDC_SP_GENTTL,IDC_SP_GENTTLSPIN,
		0.0f,65000.0f,
		1.0f),

	// Particle Speed
	ParamUIDesc(
		PB_SPEED,
		EDITTYPE_UNIVERSE,
		IDC_SP_SPEED,IDC_SP_SPEEDSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Particle Speed Var
	ParamUIDesc(
		PB_SPEEDVAR,
		EDITTYPE_FLOAT,
		IDC_SP_SPEEDVAR,IDC_SP_SPEEDVARSPIN,
		0.0f,100.0f,
		SPIN_AUTOSCALE,stdPercentDim),

	// Particle Angle Divergence
	ParamUIDesc(
		PB_ANGLEDIV,
		EDITTYPE_FLOAT,
		IDC_SP_DIVERG,IDC_SP_DIVERGSPIN,
		0.0f,90.0f,
		0.1f,stdAngleDim),

		// Emitter Start Time
	ParamUIDesc(
		PB_EMITSTART,
		EDITTYPE_TIME,
		IDC_SP_GENEMIT1,IDC_SP_GENEMIT1SPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Emitter Stop Time
	ParamUIDesc(
		PB_EMITSTOP,
		EDITTYPE_TIME,
		IDC_SP_GENEMIT2,IDC_SP_GENEMIT2SPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Particle Time Limit
	ParamUIDesc(
		PB_DISPUNTIL,
		EDITTYPE_TIME,
		IDC_SP_DISPUNTIL,IDC_SP_DISPUNTILSPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Particle Life
	ParamUIDesc(
		PB_LIFE,	
		EDITTYPE_TIME,
		IDC_SP_GENLIFE,IDC_SP_GENLIFESPIN,
		0.0f,999999999.0f,
		10.0f),

	// Particle Life Var
	ParamUIDesc(
		PB_LIFEVAR,
		EDITTYPE_TIME,
		IDC_SP_GENLIFEVAR,IDC_SP_GENLIFEVARSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Subframe Time and Motion Sampling
	ParamUIDesc(PB_SUBFRAMEMOVE,TYPE_SINGLECHEKBOX,IDC_SP_GENSMPLMOVE),
	ParamUIDesc(PB_SUBFRAMETIME,TYPE_SINGLECHEKBOX,IDC_SP_GENSMPLTIME),
	ParamUIDesc(PB_SUBFRAMEROT,TYPE_SINGLECHEKBOX,IDC_SP_GENSMPLROT),

	// Particle Size
	ParamUIDesc(
		PB_SIZE,
		EDITTYPE_UNIVERSE,
		IDC_SP_GENSIZE,IDC_SP_GENSIZESPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Particle Size Var
	ParamUIDesc(
		PB_SIZEVAR,
		EDITTYPE_FLOAT,
		IDC_SP_GENSIZEVAR,IDC_SP_GENSIZEVARSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE,stdPercentDim),

	// Particle Grow Time
	ParamUIDesc(
		PB_GROWTIME,
		EDITTYPE_TIME,
		IDC_SP_GENGRO,IDC_SP_GENGROSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Particle Fade Time
	ParamUIDesc(
		PB_FADETIME,
		EDITTYPE_TIME,
		IDC_SP_GENFAD,IDC_SP_GENFADSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Random Number Seed
	ParamUIDesc(
		PB_RNDSEED,
		EDITTYPE_INT,
		IDC_SP_GENSEED,IDC_SP_GENSEEDSPIN,
		0.0f,25000.0f,
		1.0f),
	};

#define PARAMPGEN_LENGTH 19

// Particle Type for PArray
static ParamUIDesc descParamPType[] = {

	// Particle Class
	ParamUIDesc(PB_PARTICLECLASS,TYPE_RADIO,particleclassIDs,4),

	// Particle Type
	ParamUIDesc(PB_PARTICLETYPE,TYPE_RADIO,particletypeIDs,8),

	// Metaball Tension
	ParamUIDesc(
		PB_METATENSION,
		EDITTYPE_FLOAT,
		IDC_SP_METTENS,IDC_SP_METTENSSPIN,
		0.1f,10.0f,
		SPIN_AUTOSCALE),

	// Metaball Tension Variation
	ParamUIDesc(
		PB_METATENSIONVAR,
		EDITTYPE_FLOAT,
		IDC_SP_METTENSVAR,IDC_SP_METTENSVARSPIN,
		0.0f,100.0f,
		SPIN_AUTOSCALE,stdPercentDim),

	// Metaball Courseness
	ParamUIDesc(
		PB_METACOURSE,
		EDITTYPE_UNIVERSE,
		IDC_SP_METCOURSE,IDC_SP_METCOURSESPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Metaball Courseness ViewPort
	ParamUIDesc(
		PB_METACOURSEV,
		EDITTYPE_UNIVERSE,
		IDC_SP_METCOURSEV,IDC_SP_METCOURSEVSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Metaball Auto Coarseness
	ParamUIDesc(PB_METAAUTOCOARSE,TYPE_SINGLECHEKBOX,IDC_SP_AUTOCOARSE),

	// Display type
	ParamUIDesc(PB_PANOTDRAFT,TYPE_SINGLECHEKBOX,IDC_SP_DRAFTMODE),

	// Fragment Thickness
	ParamUIDesc(
		PB_FRAGTHICKNESS,
		EDITTYPE_UNIVERSE,
		IDC_SP_CHUNKTHICK,IDC_SP_CHUNKTHICKSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Fragment Object Using
	ParamUIDesc(PB_FRAGMETHOD,TYPE_RADIO,fragmenttypeIDs,3),

	// Number of Chunks
	ParamUIDesc(
		PB_FRAGCOUNT,
		EDITTYPE_INT,
		IDC_SP_CHUNKQTYEDIT,IDC_SP_CHUNKQTYEDITSPIN,
		1.0f,65000.0f,
		1.0f),

	// Smoothing Angle
	ParamUIDesc(
		PB_SMOOTHANG,
		EDITTYPE_FLOAT,
		IDC_SP_CHUNKSMANGLE,IDC_SP_CHUNKSMANGLESPIN,
		0.0f,180.0f,
		1.0f,
		stdAngleDim),

 	// Use Subtree Checkbox
	ParamUIDesc(PB_USESUBTREE,TYPE_SINGLECHEKBOX,IDC_AP_USESUBTREE),

	// Display type
	ParamUIDesc(PB_ANIMATIONOFFSET,TYPE_RADIO,animateoffsetIDs,3),

	// Animation Offset Amount
	ParamUIDesc(
		PB_OFFSETAMOUNT,
		EDITTYPE_TIME,
		IDC_AP_ANIRNDFR,IDC_AP_ANIRNDFRSPIN,
		0.0f,999999999.0f,
		10.0f),

		// Mapping Across
	ParamUIDesc(PB_MAPPINGTYPE,TYPE_RADIO,mappingIDs,2),

	// Time Mapping Option
	ParamUIDesc(
		PB_MAPPINGTIME,
		EDITTYPE_TIME,
		IDC_SP_MAPTIMEVAL,IDC_SP_MAPTIMEVALSPIN,
		1.0f,999999999.0f,
		10.0f),

	// Distance Mapping Option
	ParamUIDesc(
		PB_MAPPINGDIST,
		EDITTYPE_UNIVERSE,
		IDC_SP_MAPDISTVAL,IDC_SP_MAPDISTVALSPIN,
		0.1f,999999999.0f,
		SPIN_AUTOSCALE),
	
	// Use Material From Custom
	ParamUIDesc(PB_CUSTOMMATERIAL,TYPE_RADIO,custmtlIDs,3),

	// Edge Material ID
	ParamUIDesc(
		PB_EMAT,
		EDITTYPE_INT,
		IDC_AP_EDGEMATID,IDC_AP_EDGEMATIDSPIN,
		0.0f,65000.0f,
		SPIN_AUTOSCALE),

	// Back Material ID
	ParamUIDesc(
		PB_BMAT,
		EDITTYPE_INT,
		IDC_AP_BACKMATID,IDC_AP_BACKMATIDSPIN,
		0.0f,65000.0f,
		SPIN_AUTOSCALE),

	// Front Material ID
	ParamUIDesc(
		PB_FMAT,
		EDITTYPE_INT,
		IDC_AP_MATID,IDC_AP_MATIDSPIN,
		0.0f,65000.0f,
		SPIN_AUTOSCALE),

};
#define PARAMPTYPE_LENGTH 22

// Common Dialog for Particle Spin
static ParamUIDesc descParamPSpin[] = {

	// Spin Time
	ParamUIDesc(
		PB_SPINTIME,
		EDITTYPE_TIME,
		IDC_SP_SPIN,IDC_SP_SPINSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Spin Time Var
	ParamUIDesc(
		PB_SPINTIMEVAR,
		EDITTYPE_FLOAT,
		IDC_SP_SPINVAR,IDC_SP_SPINVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

	// Spin Phase
	ParamUIDesc(
		PB_SPINPHASE,
		EDITTYPE_FLOAT,
		IDC_SP_SPINPHA,IDC_SP_SPINPHASPIN,
		-360.0f,360.0f,
		0.01f,
		stdAngleDim),

	// Spin Phase Variation
	ParamUIDesc(
		PB_SPINPHASEVAR,
		EDITTYPE_FLOAT,
		IDC_SP_SPINPHAVAR,IDC_SP_SPINPHAVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

	// Enter Axis Control
  	ParamUIDesc(PB_SPINAXISTYPE,TYPE_RADIO,spindirectionIDs,3),

	// X-Axis
	ParamUIDesc(
		PB_SPINAXISX,
		EDITTYPE_FLOAT,
		IDC_SP_SPINAXISX,IDC_SP_SPINAXISXSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Y-Axis
	ParamUIDesc(
		PB_SPINAXISY,
		EDITTYPE_FLOAT,
		IDC_SP_SPINAXISY,IDC_SP_SPINAXISYSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Z-Axis
	ParamUIDesc(
		PB_SPINAXISZ,
		EDITTYPE_FLOAT,
		IDC_SP_SPINAXISZ,IDC_SP_SPINAXISZSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Spin Direction Variation
	ParamUIDesc(
		PB_SPINAXISVAR,
		EDITTYPE_FLOAT,
		IDC_SP_SPINAXISVAR,IDC_SP_SPINAXISVARSPIN,
		-180.0f,180.0f,
		0.01f,
		stdAngleDim),

	// Stretch
	ParamUIDesc(
		PB_STRETCH,
		EDITTYPE_INT,
		IDC_AP_STRETCH,IDC_AP_STRETCHSPIN,
		-1000.0f,1000.0f,
		1.0f),

	// IPC Enable
	ParamUIDesc(PB_PAIPCOLLIDE_ON,TYPE_SINGLECHEKBOX,IDC_INTERP_BOUNCEON),

	// IPC Steps
	ParamUIDesc(
		PB_PAIPCOLLIDE_STEPS,
		EDITTYPE_INT,
		IDC_INTERP_NSTEPS,IDC_INTERP_NSTEPSSPIN,
		1.0f,1000.0f,
		1.0f),

	// IPC Bounce
	ParamUIDesc(
		PB_PAIPCOLLIDE_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCE,IDC_INTERP_BOUNCESPIN,
		0.0f,10000.0f,
		1.0f,
		stdPercentDim),

	// IPC Bounce
	ParamUIDesc(
		PB_PAIPCOLLIDE_BOUNCEVAR,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCEVAR,IDC_INTERP_BOUNCEVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),

	};
#define PARAMPSPIN_LENGTH 14

// Common Dialog for Secondary Motion Data
static ParamUIDesc descParamEmitV[] = {

	// Emitter Velocity Inheritance Influence Portion
	ParamUIDesc(
		PB_EMITVINFL,
		EDITTYPE_FLOAT,
		IDC_SP_EMVI,IDC_SP_EMVISPIN,
		0.0f,100.0f,
		0.01f),

	// Secondary Motion Multiplier
	ParamUIDesc(
		PB_EMITVMULT,
		EDITTYPE_FLOAT,
		IDC_SP_EMVIMULT,IDC_SP_EMVIMULTSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Multiplier Variation
	ParamUIDesc(
		PB_EMITVMULTVAR,
		EDITTYPE_FLOAT,
		IDC_SP_EMVIMULTVAR,IDC_SP_EMVIMULTVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),
	};
#define PARAMEMITV_LENGTH 3

// Common Dialog for Bubble Motion
static ParamUIDesc descParamBubl[] = {

	// Bubble Amplitude
	ParamUIDesc(
		PB_BUBLAMP,
		EDITTYPE_UNIVERSE,
		IDC_SP_BUBL_AMP,IDC_SP_BUBL_AMPSPIN,
		-.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Bubble Amp Var
	ParamUIDesc(
		PB_BUBLAMPVAR,
		EDITTYPE_FLOAT,
		IDC_SP_BUBL_AMPVAR,IDC_SP_BUBL_AMPVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

	// Bubble Period
	ParamUIDesc(
		PB_BUBLPER,
		EDITTYPE_TIME,
		IDC_SP_BUBL_PER,IDC_SP_BUBL_PERSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Bubble Period Var
	ParamUIDesc(
		PB_BUBLPERVAR,
		EDITTYPE_FLOAT,
		IDC_SP_BUBL_PERVAR,IDC_SP_BUBL_PERVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

	// Bubble Phase
	ParamUIDesc(
		PB_BUBLPHAS,
		EDITTYPE_FLOAT,
		IDC_SP_BUBL_PERPHA,IDC_SP_BUBL_PERPHASPIN,
		-360.0f,360.0f,
		0.01f,
		stdAngleDim),

	// Bubble Phase Var
	ParamUIDesc(
		PB_BUBLPHASVAR,
		EDITTYPE_FLOAT,
		IDC_SP_BUBL_PERPHAVAR,IDC_SP_BUBL_PERPHAVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),
};
#define PARAMBUBL_LENGTH 6

static int SpawnTypeIDs[] = {IDC_AP_NOSPAWN,IDC_AP_COLLIDEDIE,IDC_AP_COLLIDESPAWN,IDC_AP_DEATHSPAWN,IDC_AP_SPAWNTRAILS};
static int SpeedChaosIDs[] = {IDC_AP_SPEEDLESS,IDC_AP_SPEEDMORE,IDC_AP_SPEEDBOTH};
static int ScaleChaosIDs[] = {IDC_AP_SCALEDOWN,IDC_AP_SCALEUP,IDC_AP_SCALEBOTH};

// Dialog for Particle Spawning
static ParamUIDesc descPSpawning[] = {
	
	// Spawing Effects Type
	ParamUIDesc(PB_SPAWNTYPE,TYPE_RADIO,SpawnTypeIDs,5),

	// Die after X
	ParamUIDesc(
		PB_PASPAWNDIEAFTER,
		EDITTYPE_TIME,
		IDC_AP_MAXSPAWNDIEAFTER,IDC_AP_MAXSPAWNDIEAFTERSPIN,
		0.0f,999999999.0f,
		10.0f),

	ParamUIDesc(
		PB_PASPAWNDIEAFTERVAR,
		EDITTYPE_FLOAT,
		IDC_AP_MAXSPAWNDIEAFTERVAR,IDC_AP_MAXSPAWNDIEAFTERVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

	// Spawn Generations
	ParamUIDesc(
		PB_SPAWNGENS,
		EDITTYPE_INT,
		IDC_AP_MAXSPAWNGENS,IDC_AP_MAXSPAWNGENSSPIN,
		1.0f,65000.0f,
		1.0f),

	// Spawn Percent
	ParamUIDesc(
		PB_SPAWNPERCENT,
		EDITTYPE_INT,
		IDC_AP_PARENTPERCENT,IDC_AP_PARENTPERCENTSPIN,
		1.0f,100.0f,
		1.0f),

	// Spawn Spawncount
	ParamUIDesc(
		PB_SPAWNCOUNT,
		EDITTYPE_INT,
		IDC_AP_NUMBERVAR,IDC_AP_NUMBERVARSPIN,
		1.0f,65000.0f,
		1.0f),

	// Spawn Mult Percent
	ParamUIDesc(
		PB_SPAWNMULTVAR,
		EDITTYPE_FLOAT,
		IDC_AP_NUMBERVARVAR,IDC_AP_NUMBERVARVARSPIN,
		0.0f,100.0f,
		0.1f,stdPercentDim),

	// Spawn Direction Chaos
	ParamUIDesc(
		PB_SPAWNDIRCHAOS,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSANGLE,IDC_AP_CHAOSANGLESPIN,
		0.0f,100.0f,
		0.1f,stdPercentDim),

	// Spawn Speed Chaos
	ParamUIDesc(
		PB_SPAWNSPEEDCHAOS,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSSPEED,IDC_AP_CHAOSSPEEDSPIN,
		0.0f,100.0f,
		0.1f),

	// Spawing Speed Sign
	ParamUIDesc(PB_SPAWNSPEEDSIGN,TYPE_RADIO,SpeedChaosIDs,3),

	// Spawning Inherit Parent V
	ParamUIDesc(PB_SPAWNINHERITV,TYPE_SINGLECHEKBOX,IDC_AP_SPAWNSUMV),

	// Spawning Speed Fixed
	ParamUIDesc(PB_SPAWNSPEEDFIXED,TYPE_SINGLECHEKBOX,IDC_AP_SPAWNSPEEDFIXED),

	// Spawn Scale Chaos
	ParamUIDesc(
		PB_SPAWNSCALECHAOS,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSSCALE,IDC_AP_CHAOSSCALESPIN,
		0.0f,100.0f,
		0.1f),

	// Spawning Scale Sign
	ParamUIDesc(PB_SPAWNSCALESIGN,TYPE_RADIO,ScaleChaosIDs,3),

	// Spawning Speed Fixed
	ParamUIDesc(PB_SPAWNSCALEFIXED,TYPE_SINGLECHEKBOX,IDC_AP_SPAWNSCALEFIXED),

	// Spawn Lifespan Entry Field
	ParamUIDesc(
		PB_SPAWNLIFEVLUE,
		EDITTYPE_INT,
		IDC_AP_QUEUELIFESPAN,IDC_AP_QUEUELIFESPANSPIN,
		0.0f,65000.0f,
		1.0f),
};
#define PSPAWNINGPARAMS_LENGTH 16

static ParamBlockDescID spdescVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
};

static ParamBlockDescID spdescVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
};

static ParamBlockDescID spdescVer2[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
	{ TYPE_INT, NULL, FALSE, 76 }, // Subframe rotation checkbox
};
static ParamBlockDescID spdescVer3[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
	{ TYPE_INT, NULL, FALSE, 76 }, // Subframe rotation checkbox
	{ TYPE_INT, NULL, FALSE, 77 }, // spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 78 }, // spawn mult var
};
static ParamBlockDescID spdescVer4[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
	{ TYPE_INT, NULL, FALSE, 76 }, // Subframe rotation checkbox
	{ TYPE_INT, NULL, FALSE, 77 }, // spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 78 }, // spawn mult var
	{ TYPE_INT, NULL, FALSE, 79 }, // Not Draft
	{ TYPE_INT, NULL, TRUE, 80 }, // Use Selected
};
static ParamBlockDescID spdescVer5[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
	{ TYPE_INT, NULL, FALSE, 76 }, // Subframe rotation checkbox
	{ TYPE_INT, NULL, FALSE, 77 }, // spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 78 }, // spawn mult var
	{ TYPE_INT, NULL, FALSE, 79 }, // Not Draft
	{ TYPE_INT, NULL, TRUE, 80 }, // Use Selected
	{ TYPE_INT, NULL, TRUE, 81 }, // die after X
	{ TYPE_FLOAT, NULL, TRUE, 82 }, // die after X var
};
static ParamBlockDescID spdescVer6[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Distribution
	{ TYPE_INT, NULL, FALSE, 1 },	 // Emitter count
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // speed var
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // angle div

	{ TYPE_INT, NULL, FALSE, 5 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 6 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 7 },    // total number
	{ TYPE_INT, NULL, FALSE, 8 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 9 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 10 },   // display until
	{ TYPE_INT, NULL, TRUE, 11 },	 // life
	{ TYPE_INT, NULL, TRUE, 12 },	 // life var
	{ TYPE_INT, NULL, FALSE, 13 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 15 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size var
	{ TYPE_INT, NULL, FALSE, 17 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 18 },    // fade time
	{ TYPE_INT, NULL, FALSE, 19 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 20 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 21 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 22 },  // particle class
	{ TYPE_INT, NULL, FALSE, 23 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 24 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 26 }, // meta course
	{ TYPE_INT, NULL, FALSE, 27 }, // auto coarseness
	{ TYPE_FLOAT, NULL, FALSE, 28 },  // frag thickness
	{ TYPE_INT, NULL, FALSE, 29 },	  // frag method
	{ TYPE_INT, NULL, FALSE, 30 },	  // frag count
	{ TYPE_FLOAT, NULL, FALSE, 31 },  //smooth angle
	{ TYPE_INT, NULL, FALSE, 32 },  //Use subtree
	{ TYPE_INT, NULL, FALSE, 33 },  //animation offset method
	{ TYPE_INT, NULL, FALSE, 34 },  //animation offset amount
	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist
	{ TYPE_INT, NULL, FALSE, 40 },	  // custom material

	{ TYPE_INT, NULL, FALSE, 41 },  // side material
	{ TYPE_INT, NULL, FALSE, 42 },  // back material
	{ TYPE_INT, NULL, FALSE, 43 },  // front material

	{ TYPE_INT, NULL, TRUE, 44 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 45 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 46 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 48 },  // spin axis type
	{ TYPE_FLOAT, NULL, TRUE, 49 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 50 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 51 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 53 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 54 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 58 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 59 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 60 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble phase var
	{ TYPE_INT, NULL, TRUE, 62 },  // stretch

	{ TYPE_INT, NULL, FALSE, 63 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 64 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 65 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 67 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 68 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 69 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 70 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 71 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 72 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 73 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 74 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 75 }, // Meta course viewport
	{ TYPE_INT, NULL, FALSE, 76 }, // Subframe rotation checkbox
	{ TYPE_INT, NULL, FALSE, 77 }, // spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 78 }, // spawn mult var
	{ TYPE_INT, NULL, FALSE, 79 }, // Not Draft
	{ TYPE_INT, NULL, TRUE, 80 }, // Use Selected
	{ TYPE_INT, NULL, TRUE, 81 }, // die after X
	{ TYPE_FLOAT, NULL, TRUE, 82 }, // die after X var
	{ TYPE_INT, NULL, FALSE, 83 },  // IPC Enable
	{ TYPE_INT, NULL, FALSE, 84 },  // IPC Steps
	{ TYPE_FLOAT, NULL, TRUE, 85 },  // IPC Bounce
	{ TYPE_FLOAT, NULL, TRUE, 86 },  // IPC Bounce Var
};

static ParamVersionDesc paversions[] = {
	ParamVersionDesc(spdescVer0,75,0),
	ParamVersionDesc(spdescVer1,76,1),
	ParamVersionDesc(spdescVer2,77,2),
	ParamVersionDesc(spdescVer3,79,3),
	ParamVersionDesc(spdescVer4,81,4),
	ParamVersionDesc(spdescVer5,83,5),
	};
#define PBLOCK_LENGTH_PARRAY 87

#define NUM_OLDVERSIONS	6

// Current version
#define CURRENT_VERSION	6
static ParamVersionDesc curVersionPA(spdescVer6,PBLOCK_LENGTH_PARRAY,CURRENT_VERSION);

void PACheckInstButtons(IParamBlock *pblock,HWND hptype)
{ int isinst;
  pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
  if ((isinst==INSTGEOM)||(isinst==BYGEOM))
   TurnButton(hptype,IDC_AP_OBJECTPICK,TRUE);
}

void PACheckSpawnButtons(IParamBlock *pblock,HWND spawn,int repi)
{ int stype;
  pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  int ison; pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
  if (ison) stype=0;
  SpawnWithStype(stype,spawn,repi);
}

void PACheckPickButtons(IParamBlock *pblock,HWND hptype,HWND spawn,int repi)
{ PACheckInstButtons(pblock,hptype);
  PACheckSpawnButtons(pblock,spawn,repi);
}

//-- ParticleDlgProc ------------------------------------------------

class CreatePArrayProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		PArrayParticle *PArrayObject;
		int attachedToNode;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		BOOL square;

		int ignoreSelectionChange;

		int lastPutCount;
		void CreateNewObject();	

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)CloudNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { CloudNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
		BOOL SupportAutoGrid(){return TRUE;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		CreatePArrayProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEPArrayMODE	CID_USER +13

class CreatePArrayMode : public CommandMode {		
	public:		
		CreatePArrayProc proc;
		IObjParam *ip;
		PArrayParticle *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,PArrayParticle*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEPArrayMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreatePArrayMode theCreatePArrayMode;

RefResult CreatePArrayProc::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
				}
		 	if ( PArrayObject && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (PArrayObject && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (PArrayObject->creating) 
						{  theCreatePArrayMode.JumpStart(PArrayObject->ip,PArrayObject);
							createInterface->SetCommandMode(&theCreatePArrayMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				PArrayObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				PArrayObject  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreatePArrayProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{					   
	ip=(IObjParam*)ioc;
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	PArrayObject       = NULL;
	CreateNewObject();
	}
void CreatePArrayProc::CreateNewObject()
	{
	SuspendSetKeyMode();
  createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	PArrayObject = (PArrayParticle*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( PArrayObject ) {
		PArrayObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#ifdef _OSNAP
		PArrayObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreatePArrayProc::End()
{ if ( PArrayObject ) 
	{ 
#ifdef _OSNAP
		PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	
	PArrayObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete PArrayObject;
			PArrayObject = NULL;
			createInterface->GetMacroRecorder()->Cancel();
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();  }
	}
}

void CreatePArrayMode::JumpStart(IObjParam *i,PArrayParticle *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}


int PArrayClassDesc::BeginCreate(Interface *i)
	{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreatePArrayMode.Begin(iob,this);
	iob->PushCommandMode(&theCreatePArrayMode);
	return TRUE;
	}

int PArrayClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreatePArrayMode.End();
	i->RemoveMode(&theCreatePArrayMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreatePArrayProc::createmethod(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{	Point3 d;

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
				PArrayObject->pblock->SetValue(PB_EMITRWID,0,0.01f);
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
				PArrayObject->pmapParam->Invalidate();
				break;

			case 1: 
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1.z = p0.z +(float).01; 
				if(flags&MOUSE_CTRL) 
				{ mat.SetTrans(p0);	} 
				else mat.SetTrans(float(.5)*(p0+p1));
				d = p1-p0;
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
				PArrayObject->pblock->SetValue(PB_EMITRWID,0,float(fabs(d.x)));
				PArrayObject->pmapParam->Invalidate();

				if (msg==MOUSE_POINT)
				{ if (Length(sp1-sp0)<3 || Length(d)<0.1f)  return CREATE_ABORT;	
				  else
				  {	ICustButton *iBut = GetICustButton(GetDlgItem(PArrayObject->hparam,IDC_SP_CHUNKPICKBUTTON));
					iBut->Enable();
					ReleaseICustButton(iBut);
					PACheckPickButtons(PArrayObject->pblock,PArrayObject->hptype,PArrayObject->spawn,PArrayObject->pickCB.repi);
				   return CREATE_STOP;	}
				}
				break;
			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	return 1;
	}

int CreatePArrayProc::proc(HWND hwnd,int msg,int point,int flag,
				IPoint2 m )
{	int res=TRUE;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					assert( PArrayObject );					
					vpx->CommitImplicitGrid(m, flag );
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					if ( attachedToNode ) {
				   		// send this one on its way
#ifdef _OSNAP
						PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		PArrayObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						// Get rid of the reference.
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}
						// new object
						CreateNewObject();   // creates PArrayObject
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( PArrayObject);
					attachedToNode = TRUE;
					assert( CloudNode );					
					createCB = NULL;
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();

					mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));

//						macroRec->Disable();
					createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
//						macroRec->Enable();
				default:				
						res = createmethod(vpx,msg,point,flag,m,mat);

						if (res==CREATE_ABORT)
							goto abort;
						if (res==CREATE_STOP)
						{
#ifdef _OSNAP
                         PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
						 theHold.Accept(GetString(IDS_DS_CREATE));	 
						}
						createInterface->RedrawViews(createInterface->GetTime());   //DS

					break;
					
				}			
			break;

		case MOUSE_MOVE:
				mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));			
				res = createmethod(vpx,msg,point,flag,m,mat);
				if (res==CREATE_ABORT) 
					goto abort;
				if (res==CREATE_STOP)
				{
#ifdef _OSNAP
         PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
					theHold.Accept(GetString(IDS_DS_CREATE));	// TH
				}
				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_INTERACTIVE);		//DS		
			break;

		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
#ifdef _OSNAP  //PREVIEW SNAP
				res = createmethod(vpx,msg,point,flag,m,mat);
#endif
		vpx->TrackImplicitGrid(m);
			break;
	  case MOUSE_PROPCLICK:
		createInterface->SetStdCommandMode(CID_OBJMOVE);
		break;
	  case MOUSE_ABORT: 
		 abort:
		assert( PArrayObject );
#ifdef _OSNAP
		PArrayObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		PArrayObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the Cloudera and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
			GetSystemSetting(SYSSET_CLEAR_UNDO);
		PArrayObject=NULL;
		createInterface->RedrawViews(createInterface->GetTime());
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
	}

	done:
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}

// The EnterMode and ExitMode methods allow the plug-in to update it's UI
// to indicate the command mode is active.  In this case we update the 
// state of the pick button to indicate the mode is active or inactive.
// The button color GREEN_WASH is the standard for command modes.

void PArrayParticle::GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype,int fragflags)
{ int m,n,found,tmax,tnums=0,tgen;
  TimeValue tframe;  
  times.tl.SetCount(0);times.tl.Shrink();
  times.tl.Resize(100);tmax=100;times.tl.SetCount(tmax);
  int nCount=nlist.Count();
  for (m=0;m<parts.Count();m++)
  { if ((!parts.Alive(m))||((fragflags==BYGEOM)&&((sdata[m].gennum==0)||(nCount==0)))) continue;
    if (ltype)
	{ if (ltype==1) tframe=sdata[m].showframe+parts.ages[m];
	  else tframe=sdata[m].showframe+t;
  //    if ((tframe>=anifr)&&(anifr!=0)) tframe=tframe % anifr;
	} else tframe=t;
	found=n=0;
	tgen=(sdata[m].gennum>nCount?nCount:sdata[m].gennum);
	while ((n<tnums)&&(!found))
	{ found=((times.tl[n].gennum==tgen)&&(tframe==times.tl[n].tl));
	  n++;
	}
	if (!found) 
	{ if (tnums>=tmax) 
	  { times.tl.Resize(tmax+=100);times.tl.SetCount(tmax);}
	  times.tl[tnums].gennum=tgen;times.tl[tnums++].tl=tframe;
	}
  }
  times.tl.SetCount(tnums);
  times.tl.Shrink();
}

void PArrayParticle::RetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc,i;
  Mtl *submtl=node->GetMtl();
  if (subtree)
  { if ((nc=node->NumberOfChildren())>0)
	for (i=0;i<nc;i++)
	  RetrieveMtls(node->GetChildNode(i),subtree,numsubs,numtabs,tabmax,t);
  }
  if (IsGEOM(node->EvalWorldState(t).obj)!=NULL)
  { if (*numtabs>=*tabmax)
	{ mttab.Resize((*tabmax)+=256);}
	if (submtl!=NULL)
	{ mttab[(*numtabs)++]=submtl;
	  int subs;
	  if ((subs=submtl->NumSubMtls())>0)
	    (*numsubs)+=subs;
	  else (*numsubs)++;
	}
	else 
	{DWORD tc=node->GetWireColor();
	StdMat *m = NewDefaultStdMat();
	m->SetName(TSTR(_T("C"))+node->GetName());
	Color bcolor(tc);
	m->SetDiffuse(bcolor,0);
	mttab[(*numtabs)++]=(Mtl*)m;
	(*numsubs)++;
	}
  }
}

void PArrayParticle::DoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) DoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) RetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void PArrayParticle::AssignMtl(INode *node,INode *topnode,int subtree,TimeValue t,int fragflags) 
{	Mtl *submtl;
	MultiMtl *newmat=NULL;
	  Mtl *nmtl=NULL;
	TSTR newname;
	MtlBaseLib glib;
	int tabmax=256;
	newname=TSTR(_T("CMat"))+node->GetName();
	if (_tcslen(newname)>16) newname[16]='\0';
	int numsubs=0,numtabs=0;
	int mstart=0,mcnt=nlist.Count();INode *onode;
	backpatch=FALSE;
	if (fragflags==BYGEOM)
	{ onode=distnode;mstart++;}
	else onode=custnode;
	nmtls.SetCount(mcnt+1);
    mttab.SetCount(tabmax);
    submtl=onode->GetMtl();
	INode *tmpnode=onode;
	if (fragflags==BYGEOM) 
	{ RetrieveMtls(distnode,FALSE,&numsubs,&numtabs,&tabmax,t);
	  nmtls[0]=numsubs;
	  tmpnode=nlist[0];
	}
	for (int mut=mstart;mut<=mcnt;mut++)
	{ if (tmpnode->IsGroupHead())
	    DoGroupMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  else
	  RetrieveMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  if (mut!=mcnt)
	  {nmtls[mut]=numsubs;
	   tmpnode=nlist[mut];
	  }
	}
	mttab.Shrink();
	if (!((numtabs==1)&&(submtl!=NULL)))
	{ TSTR oldname=TSTR(_T(" "));
	  if (nmtl=node->GetMtl())
		  oldname=nmtl->GetName();
	  if (_tcscmp(oldname,newname)!=0) 
	  { newmat=NewDefaultMultiMtl();
	    newmat->SetName(newname); }
	  else newmat=(MultiMtl*)nmtl;
//	  nmtl->FindMtl(nmtl);
	  newmat->SetNumSubMtls(numsubs);
	  int k=0,nt=0,j;
	  for (int i=0;i<numtabs;i++)
	  {	if ((nt=mttab[i]->NumSubMtls())>0)
	     for (j=0;j<nt;j++) 
		 { newmat->SetSubMtl(k,mttab[i]->GetSubMtl(j));
	       k++;
	     }
	     else 
		 { newmat->SetSubMtl(k,mttab[i]);
		   k++;
		 }
	  }
	}
    mttab.SetCount(0); mttab.Shrink();
	node->SetMtl(newmat!=NULL?newmat:submtl);  
}
void PArrayParticle::CntRetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc,i;
  Mtl *submtl=node->GetMtl();
  if (subtree)
  { if ((nc=node->NumberOfChildren())>0)
	for (i=0;i<nc;i++)
	  CntRetrieveMtls(node->GetChildNode(i),subtree,numsubs,numtabs,tabmax,t);
  }
  if (IsGEOM(node->EvalWorldState(t).obj)!=NULL)
  { if (submtl!=NULL)
	{ int subs;
	  if ((subs=submtl->NumSubMtls())>0)
	    (*numsubs)+=subs;
	  else (*numsubs)++;
	}
	else 
	(*numsubs)++;
  }
}

void PArrayParticle::CntDoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) CntDoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) CntRetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void PArrayParticle::GetSubs(INode *node,INode *topnode,int subtree,TimeValue t,int fragflags) 
{	Mtl *submtl;
	int tabmax=256;
	int numsubs=0,numtabs=0;
	int mstart=0,mcnt=nlist.Count();INode *onode;
	if (fragflags==BYGEOM)
	{ onode=distnode;mstart++;}
	else onode=custnode;
	nmtls.SetCount(mcnt+1);
    submtl=onode->GetMtl();
	INode *tmpnode=onode;
	if (fragflags==BYGEOM) 
	{ CntRetrieveMtls(distnode,FALSE,&numsubs,&numtabs,&tabmax,t);
	  nmtls[0]=numsubs;
	  tmpnode=nlist[0];
	}
	for (int mut=mstart;mut<=mcnt;mut++)
	{ if (tmpnode->IsGroupHead())
	    CntDoGroupMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  else
	  CntRetrieveMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  if (mut!=mcnt)
	  {nmtls[mut]=numsubs;
	   tmpnode=nlist[mut];
	  }
	}
}
void MakeBBpts(Point3 *pts,Box3 bbox,Matrix3 ctm)
{ pts[0]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  pts[1]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  pts[2]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  pts[3]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  pts[4]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  pts[5]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  pts[6]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  pts[7]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmax[2])*ctm;
}

void PArrayParticle::GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
{int i,nc;
	if (subtree)
  { if ((nc=node->NumberOfChildren())>0)
	for (i=0;i<nc;i++)
	  GetNextBB(node->GetChildNode(i),subtree,count,tabmax,boxcenter,t,tcount,onode);
  }
  Object *cobj=node->EvalWorldState(t).obj;
  TriObject *triOb=TriIsUseable(cobj,t);
  if (triOb)
  {	 if (*count>=*tabmax)
	{ thePArrayDraw.bboxpt[tcount].bpts.Resize((*tabmax)+=256);}
    Matrix3 ctm = node->GetObjectTM(t);
	if (node!=onode)
  { thePArrayDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f)*ctm-boxcenter;
  }	
  else thePArrayDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f);
	ctm.NoTrans();
	Box3 bbox=triOb->GetMesh().getBoundingBox();
	MakeBBpts(thePArrayDraw.bboxpt[tcount].bpts[*count].pts,bbox,ctm);
/* thePArrayDraw.bboxpt[tcount].bpts[*count].pts[0]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmax[2])*ctm;
      thePArrayDraw.bboxpt[tcount].bpts[*count].pts[1]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[2]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[3]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[4]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[5]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[6]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  thePArrayDraw.bboxpt[tcount].bpts[*count].pts[7]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  */
  (*count)++;
  if (triOb!=cobj) triOb->DeleteThis();
  }
}

void PArrayParticle::DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) DoGroupBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	  else if ((subtree)||(nxtnode->IsGroupMember())) GetNextBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	}
}
void PArrayParticle::GetallBB(INode *custnode,int subtree,TimeValue t,int fragflags)
{ int tabmax=256;
  int count=1,ocount=times.tl.Count();
  if (ocount>0) count=ocount;
  if (thePArrayDraw.bboxpt) delete[] thePArrayDraw.bboxpt;
  thePArrayDraw.bboxpt=NULL;
  INode *tmpnode,*onode;
  int cofs=(fragflags==BYGEOM?0:1);
  if ((onode=(fragflags==BYGEOM?distnode:custnode))!=NULL)
  { thePArrayDraw.bboxpt=new boxlst[count];
    int cgen;
    for (int tcount=0;tcount<count;tcount++)
    { TimeValue tofs=(ocount>0?times.tl[tcount].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[tcount].gennum-1:-1);
	  if ((cgen>-1)&&(cgen<nlist.Count()))
	  { if (!(tmpnode=nlist[cgen])) tmpnode=onode;
	  } else tmpnode=onode;
	  thePArrayDraw.bboxpt[tcount].bpts.SetCount(tabmax);
      thePArrayDraw.bboxpt[tcount].numboxes=0;
      Matrix3 ctm = tmpnode->GetObjectTM(tofs);
      boxcenter=Zero*ctm;
	  if (tmpnode->IsGroupHead())
	    DoGroupBB(tmpnode,subtree,&(thePArrayDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  else
        GetNextBB(tmpnode,subtree,&(thePArrayDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  thePArrayDraw.bboxpt[tcount].bpts.Shrink();
	}
  }
}
void PArrayParticle::CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
{ Object *pobj;	  
  TriObject *triOb;
  Point3 deftex=Point3(0.5f,0.5f,0.0f);
  TVFace Zerod;
  Zerod.t[0]=0;Zerod.t[1]=0;Zerod.t[2]=0;
  int nc,i,j,subv=0,subf=0,subtvnum=0,tface,tvert;

  if (subtree)
  { if ((nc=node->NumberOfChildren())>0)
	for (i=0;i<nc;i++)
	  CheckTree(node->GetChildNode(i),tspace,cmesh,cmbb,numV,numF,tvnum,ismapped,t,subtree,custmtl);
  }
  if ((triOb=TriIsUseable(pobj = node->EvalWorldState(t).obj,t))!=NULL)
  {	Point3 Suboffset;
    Matrix3 ctm = node->GetObjectTM(t);
/*	if (node!=custnode)
     Suboffset=Point3(0.0f,0.0f,0.0f)*ctm-boxcenter;
	else
		Suboffset=Point3(0.0f,0.0f,0.0f);
	ctm.NoTrans();*/
	int submtls=0;
	Mtl* m=node->GetMtl();
	if (!((m!=NULL)&&(submtls=m->NumSubMtls()))) 
	  submtls=-1;
	int tottv;
	subv=triOb->GetMesh().getNumVerts();
    subf=triOb->GetMesh().getNumFaces();
    subtvnum=triOb->GetMesh().getNumTVerts();
 	(*cmbb)+=(triOb->GetMesh().getBoundingBox()*ctm);
    cmesh->setNumFaces(tface=(*numF+subf),(*numF>0?TRUE:FALSE));
    cmesh->setNumVerts(tvert=(*numV+subv),(*numV>0?TRUE:FALSE));
    cmesh->setNumTVerts(tottv=(*tvnum+subtvnum),(*tvnum>0?TRUE:FALSE));
    if ((subtvnum>0)||(*ismapped))
    { if ((!(*ismapped))&&(*numF>0))
	  { cmesh->setNumTVFaces(tface);
	    *tvnum=1;
		cmesh->setNumTVerts(tottv+1);
		cmesh->tVerts[0]=deftex;
		for (int k=0;k<*numF;k++)
		  memcpy(&(cmesh->tvFace[k]),&Zerod,sizeof(TVFace));
	   }
	  else 
	   cmesh->setNumTVFaces(tface,(*numF>0?TRUE:FALSE),*numF);
	  *ismapped=1;
	}
	if (subf>0) 
	{ memcpy(&(cmesh->faces[*numF]),triOb->GetMesh().faces,sizeof(Face)*subf);
	  if (subtvnum>0)
	    memcpy(&(cmesh->tvFace[*numF]),triOb->GetMesh().tvFace,sizeof(TVFace)*subf);
	}
	j=(*numF);
	BOOL mirror=DotProd(ctm.GetRow(0)^ctm.GetRow(1),ctm.GetRow(2))<0.0f;
	for (j=(*numF);j<tface;j++)
	{ cmesh->faces[j].v[0]+=*numV;
	  cmesh->faces[j].v[1]+=*numV;
	  cmesh->faces[j].v[2]+=*numV;
	  if (mirror) MirrorFace(&cmesh->faces[j]);
	  if (custmtl)
	  { if (submtls<0) cmesh->faces[j].setMatID(CustMtls);
	  else cmesh->faces[j].setMatID(cmesh->faces[j].getMatID()+CustMtls);
	  }
	  if (subtvnum>0)
	  { cmesh->tvFace[j].t[0]+=*tvnum;
	    cmesh->tvFace[j].t[1]+=*tvnum;
	    cmesh->tvFace[j].t[2]+=*tvnum;
		if (mirror) MirrorTVs(&cmesh->tvFace[j]);
	  }
	  else if (*ismapped)
	    memcpy(&(cmesh->tvFace[j]),&Zerod,sizeof(TVFace));
	}
	if (subtvnum>0) 
      memcpy(&(cmesh->tVerts[*tvnum]),triOb->GetMesh().tVerts,sizeof(UVVert)*subtvnum);
//	if (subv) memcpy(&(clst->v[*numV]),triOb->mesh.verts,sizeof(Point3)*subv);
	int k=0;
	ctm=ctm*tspace;
	for (j=*numV;j<tvert;j++)
	{ cmesh->verts[j]=triOb->GetMesh().verts[k]*ctm;//+Suboffset;
	  k++;
	}

// Multiple Map Support (Bayboro)
	CopyMultipleMapping(cmesh, triOb, numF, tface, deftex, Zerod, subf, mirror);
// end of Multiple Map Support

	*numV+=subv;
	*numF+=subf;
	*tvnum+=subtvnum;
	if (submtls>0)
	CustMtls+=submtls;
	else CustMtls++;
  if (triOb!=pobj) triOb->DeleteThis();
  }
}

void PArrayParticle::TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) TreeDoGroup(nxtnode,tspace,cmesh,cmbb,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	  else if((subtree)||(nxtnode->IsGroupMember())) CheckTree(nxtnode,tspace,cmesh,cmbb,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	}
}

void PArrayParticle::GetMesh(TimeValue t,int subtree,int custmtl,int fragflags)
{	int tnums,numV,numF,tvnum,ismapped;
    INode *tmpnode,*onode;
	tnums=times.tl.Count();
	if (tnums==0) tnums=1;
	if (cmesh) delete[] cmesh;cmesh=NULL;
	if (cmbb) delete[] cmbb;cmbb=NULL;
	onode=(fragflags==BYGEOM?distnode:custnode);
	if (onode!=NULL)
	{ cmesh=new Mesh[tnums];
	  cmbb=new Box3[tnums];
	  int cgen;
	 for (int i=0;i<tnums;i++)
	{ TimeValue tofs=(tnums>1?times.tl[i].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[i].gennum-1:-1);
	  BOOL badnode=TRUE;
	  if ((cgen>-1)&&(cgen<nlist.Count())) badnode=(!(tmpnode=nlist[cgen]));
	  if (badnode)
	  {	if (fragflags==BYGEOM) continue; else tmpnode=custnode;}
	  Matrix3 ptm = tmpnode->GetObjectTM(tofs);
	  ptm=Inverse(ptm);
	  numV=numF=tvnum=CustMtls=ismapped=0;
	  if (tmpnode->IsGroupHead())
	   TreeDoGroup(tmpnode,ptm,&cmesh[i],&cmbb[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	  else
	   CheckTree(tmpnode,ptm,&cmesh[i],&cmbb[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	}
	}
}
BOOL PickOperand::Filter(INode *node)
	{
	if ((node)&&(!node->IsGroupHead())) {
		ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
		if (!IsGEOM(os.obj)) {
			node = NULL;
			return FALSE;
			}
		}

	return node ? TRUE : FALSE;
	}

BOOL PickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if ((node)&&(!node->IsGroupHead())) {
		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if (!IsGEOM(os.obj)) {
			node = NULL;
			return FALSE;
			}
		}

	return node ? TRUE : FALSE;
	}

void PArrayParticle::ShowName(int dist)
{ TSTR name;
if (dist)
  {FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (distnode ? distname : TSTR(GetString(IDS_AP_NONE))));
   if (hparam) SetWindowText(GetDlgItem(hparam, IDC_SP_CHUNKPICKOBJECT), name);}
 else
 {FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE))));
   if (hptype) SetWindowText(GetDlgItem(hptype, IDC_AP_INSTANCESRCNAME), name);}
}

BOOL PickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);

	INodeTab nodes;
	int subtree,flags;
	 if (dodist==1)
	  { nodes.SetCount(1);nodes[0]=node;
		theHold.Begin();
		theHold.Put(new CreatePAPartPickNode(po,po->distname,node->GetName(),1));
		if (po->distnode) po->ReplaceReference(DISTNODE,node,TRUE);
	    else po->MakeRefByID(FOREVER,DISTNODE,node);	
		theHold.Accept(GetString(IDS_AP_COMPICK));
	    po->distname = TSTR(node->GetName());
	    po->ShowName(dodist);	
	  }
	  else
	  {	if (node->IsGroupMember()) 
		  while (node->IsGroupMember()) node=node->GetParentNode();
	    po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
		flags=(node->IsGroupHead()?1:0);
		if ((!subtree)&&(!flags)) {nodes.SetCount(1);nodes[0]=node;}
	    else
		{ nodes.SetCount(0);
		  if (flags) MakeGroupNodeList(node,&nodes,subtree,ip->GetTime());
		  else MakeNodeList(node,&nodes,subtree,ip->GetTime());
		}
		if (dodist==0)
		{ theHold.Begin();
		  theHold.Put(new CreatePAPartPickNode(po,po->custname,node->GetName(),0));
	      if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	      else po->MakeRefByID(FOREVER,CUSTNODE,node);	
		  po->custname = TSTR(node->GetName());
		  theHold.Accept(GetString(IDS_AP_COMPICK));
		  po->ShowName(dodist);	
		}
	  else if (dodist==2)
	 {  theHold.Begin();
		theHold.Put(new PAObjectListRestore(po));
		po->AddToList(node,po->nlist.Count(),TRUE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
	  else 
	  { theHold.Begin();
		theHold.Put(new PAObjectListRestore(po));
	    po->AddToList(node,repi,FALSE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
	// Automatically check show result and do one update
		int frag,custmtl,onscreen;
		po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
		po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
		po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
		if (((frag==INSTGEOM)||(frag==BYGEOM))&&(onscreen>1))
		 if (onscreen==2) po->GetMesh(ip->GetTime(),subtree,custmtl,frag);
		else po->GetallBB(node,subtree,ip->GetTime(),frag);
	  }
	po->valid=FALSE;
	ip->FlashNodes(&nodes);
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	 nodes.ZeroCount();nodes.Shrink();
	if (po->creating) {
		theCreatePArrayMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreatePArrayMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void PickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	if (dodist>=2)
	{ iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==3?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
	  TurnButton(po->spawn,(dodist==2?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK),FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
	  TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
	  TurnButton(po->hparam,IDC_SP_CHUNKPICKBUTTON,FALSE);
	}
    else if (dodist) 
	  {iBut=GetICustButton(GetDlgItem(po->hparam,IDC_SP_CHUNKPICKBUTTON));
	   TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPESTD),FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEMET),FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPECHUNKS),FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEINSTANCE),FALSE);
	  }
	else
	{ iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  TurnButton(po->hparam,IDC_SP_CHUNKPICKBUTTON,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	}
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void PickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	if (!po->ip) return;
	if (dodist>=2)
	{ iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==3?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
	  PACheckInstButtons(po->pblock,po->hptype);
	  TurnButton(po->hparam,IDC_SP_CHUNKPICKBUTTON,TRUE);
	  if (dodist==3)
	   TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,TRUE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	}
    else if (dodist)
	{ iBut=GetICustButton(GetDlgItem(po->hparam,IDC_SP_CHUNKPICKBUTTON));
	  PACheckInstButtons(po->pblock,po->hptype);
	  PACheckSpawnButtons(po->pblock,po->spawn,repi);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPESTD),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEMET),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPECHUNKS),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEINSTANCE),TRUE);
	}
	else
	{ iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  TurnButton(po->hparam,IDC_SP_CHUNKPICKBUTTON,TRUE);
	  PACheckSpawnButtons(po->pblock,po->spawn,repi);
	}
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
}

void PACheckStretchBox(HWND hWnd,PArrayParticle *po)
{ if (IsStdMtl(po->cnode))
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),TRUE);
  } else EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
  SpinnerOn(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),TRUE);
  po->pmapPSpin->Invalidate();
}

void PAStretchStuff(int dir,int fragflags,HWND hWnd,PArrayParticle *po)
{ if (dir==0)
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,FALSE,fragflags!=BYGEOM);
  }
  else if (dir==1)
  {	if (IsStdMtl(po->cnode))
    { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),TRUE);
    } else EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOn(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),TRUE);
	SpinStuff(hWnd,FALSE,fragflags!=BYGEOM);
  }  
  else if (dir==2)
  {	EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,TRUE,fragflags!=BYGEOM);
  }
}

void InitParams(PArrayParticle *po,HWND hWnd)
{ int dist;
  po->pblock->GetValue(PB_DISTRIBUTION,0,dist,FOREVER);
  ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDITSPIN));
  if (dist==3) spin2->Enable();
  else spin2->Disable();
  ReleaseISpinner(spin2);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDIT_TXT),dist==3);
}
class ParticleDlgProc : public ParamMapUserDlgProc {
	public:
		PArrayParticle *po;
		ICustButton *iBut;

		ParticleDlgProc(PArrayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void ParticleDlgProc::Update(TimeValue t)
{	if (!po->editOb) return;
	InitParams(po,po->hparam);
	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	if (width<0.01f) iBut->Disable();
	po->ShowName(1);
}

BOOL ParticleDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	switch (msg) {
		case WM_INITDIALOG: {
			iBut = GetICustButton(GetDlgItem(hWnd,IDC_SP_CHUNKPICKBUTTON));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_DESTROY:
			// Release all our Custom Controls
			ReleaseICustButton(iBut);
			return FALSE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_PASURFACE:
			  case IDC_SP_PAEDGES:
			  case IDC_SP_PAVERTICES:
			  case IDC_SP_PAFACEC:
				{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDITSPIN));
				  spin2->Disable();
				  ReleaseISpinner(spin2);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDIT_TXT),FALSE);
				 break;
				}
			  case IDC_SP_PAPOINTS:
				{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDITSPIN));
				  spin2->Enable();
				  ReleaseISpinner(spin2);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDIT_TXT),TRUE);
				 break;
				}
			 case IDC_SP_CHUNKPICKBUTTON:
				   { if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePArrayMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePArrayMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;
					      po->pickCB.dodist=1;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
			  case IDC_SP_VIEWDISPMESH:
				  {po->valid=FALSE;
				   int subtree,custmtl;
					po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	  po->thePArrayDraw.t=t;
	  po->thePArrayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					  int anifr=aniend+GetTicksPerFrame();
					po->thePArrayDraw.anifr=anifr;
					po->GetTimes(po->times,t,anifr,anioff,po->fragflags);
				    po->GetMesh(t,subtree,custmtl,po->fragflags);
				   break;
				  }
			  case IDC_SP_VIEWDISPBOX:			  
				  { po->valid=FALSE;
				   int subtree,custmtl;
					po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	  po->thePArrayDraw.t=t;
	  po->thePArrayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					    int anifr;
						anifr=aniend+GetTicksPerFrame();
	  po->thePArrayDraw.anifr=anifr;
						po->GetTimes(po->times,t,anifr,anioff,po->fragflags);
					po->GetallBB(po->custnode,subtree,t,po->fragflags);
				  break;
				  }
			}
			break;	
		default:
			return FALSE;
		}
	return TRUE;
	}
void SetRateSpinner(IParamBlock *pblock,TimeValue t,HWND hWnd)
{ int birthmeth;
  EnableWindow(GetDlgItem(hWnd,IDC_SP_GENUSERATE),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_GENUSETTL),TRUE);
  pblock->GetValue(PB_BIRTHMETHOD,0,birthmeth,FOREVER);
  if (birthmeth)
  { SpinnerOff(hWnd,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
    SpinnerOn(hWnd,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
  }else 
  { SpinnerOn(hWnd,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
    SpinnerOff(hWnd,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
  }
}

class ParticleGenDlgProc : public ParamMapUserDlgProc {
	public:
		PArrayParticle *po;

		ParticleGenDlgProc(PArrayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL ParticleGenDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{  int acourse;
       float size;
      switch (msg) 
	  {   case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_GENUSERATE:
				{ SpinnerOn(hWnd,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
				  SpinnerOff(hWnd,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
				 return TRUE;
				}
			  case IDC_SP_GENUSETTL:
				{ SpinnerOff(hWnd,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
				  SpinnerOn(hWnd,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
				 return TRUE;
				}
			}
			case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) 
			{ case IDC_SP_GENSIZESPIN:
				   { po->pblock->GetValue(PB_METAAUTOCOARSE,t,acourse,FOREVER);
					int mpart;
				    po->pblock->GetValue(PB_PARTICLECLASS,t,mpart,FOREVER);
				    if ((mpart==METABALLS)&&(acourse) )
					 { po->pblock->GetValue(PB_SIZE,t,size,FOREVER);
					   po->pblock->SetValue(PB_METACOURSE,t,size/coursedivider);
					   po->pblock->SetValue(PB_METACOURSEV,t,size/3.0f);
					   po->pmapParam->Invalidate();
					 }
			        return TRUE;
				   }
				case IDC_AP_NEWSEED:
					{ srand( (unsigned)time( NULL ) );
					  int newseed=rand() % 25001;
					  po->pblock->SetValue(PB_RNDSEED,0,newseed);
					  po->pmapPGen->Invalidate();
					  return TRUE;
					}
				case IDC_SP_GENEMIT1SPIN:
				case IDC_SP_GENEMIT2SPIN:
					{ int sstop,sstart;
					  po->pblock->GetValue(PB_EMITSTOP,t,sstop,FOREVER);
					  po->pblock->GetValue(PB_EMITSTART,t,sstart,FOREVER);
					  if (sstop<sstart) 
					     po->pblock->SetValue(PB_EMITSTOP,t,sstart);
					  return TRUE;
					}
			}
	  }
	return FALSE;
	}
void CourseCheck(PArrayParticle *po,HWND hWnd,TimeValue t)
{ int acourse;
  float size;
  po->pblock->GetValue(PB_METAAUTOCOARSE,t,acourse,FOREVER);
  if (acourse) 
  {float mc1,mc2,mc,mcv;
	po->pblock->GetValue(PB_SIZE,t,size,FOREVER);mc=size/coursedivider;mcv=size/3.0f;
	po->pblock->GetValue(PB_METACOURSE,t,mc1,FOREVER);
	po->pblock->GetValue(PB_METACOURSEV,t,mc2,FOREVER);
	if ((mc1!=mc)||(mc2!=mcv))
	{ po->pblock->SetValue(PB_METACOURSE,t,mc);
	  po->pblock->SetValue(PB_METACOURSEV,t,mcv);
	  po->pmapPType->Invalidate();
	}
	SpinnerOff(hWnd,IDC_SP_METCOURSESPIN,IDC_SP_METCOURSE);
 	SpinnerOff(hWnd,IDC_SP_METCOURSEVSPIN,IDC_SP_METCOURSEV);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_COARSENESS_TXT),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSE_TXT),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSEV_TXT),FALSE);
  }
  else 
  { SpinnerOn(hWnd,IDC_SP_METCOURSESPIN,IDC_SP_METCOURSE);
 	SpinnerOn(hWnd,IDC_SP_METCOURSEVSPIN,IDC_SP_METCOURSEV); 
    EnableWindow(GetDlgItem(hWnd,IDC_SP_COARSENESS_TXT),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSE_TXT),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSEV_TXT),TRUE); 
  }
};
void SetMapVals(PArrayParticle *po,HWND hWnd,TimeValue t)
{ int type,maptype;
  po->pblock->GetValue(PB_PARTICLETYPE,t,type,FOREVER);
  if ((po->fragflags!=BYGEOM) &&(type==RENDTET))
  { SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
	SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
 	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
  }
  else
  { po->pblock->GetValue(PB_MAPPINGTYPE,t,maptype,FOREVER);
    if (maptype)
	{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
	  SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
	}
	else
	{ SpinnerOn(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
	  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
	}
 	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
  }
}

void PForm(HWND hWnd,BOOL ison,HWND hbubl,HWND hgen,IParamBlock *pblock)
{   EnableWindow(GetDlgItem(hWnd,IDC_SP_PASURFACE),ison);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_PAEDGES),ison);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_PAVERTICES),ison);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTS),ison);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_PAFACEC),ison);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_USESUBOBJS),ison);	
	if (!ison)
	{ SpinnerOff(hWnd,IDC_SP_GENDISPSPIN,IDC_SP_GENDISP);
 	  SpinnerOff(hgen,IDC_SP_GENEMIT2SPIN,IDC_SP_GENEMIT2);
	  SpinnerOff(hgen,IDC_SP_GENSIZESPIN,IDC_SP_GENSIZE);
	  SpinnerOff(hgen,IDC_SP_GENSIZEVARSPIN,IDC_SP_GENSIZEVAR);
	  SpinnerOff(hbubl,IDC_SP_BUBL_PERPHASPIN,IDC_SP_BUBL_PERPHA);
	  SpinnerOff(hbubl,IDC_SP_BUBL_PERPHAVARSPIN,IDC_SP_BUBL_PERPHAVAR);
	  SpinnerOff(hWnd,IDC_SP_PAPOINTSEDITSPIN,IDC_SP_PAPOINTSEDIT);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDIT_TXT),FALSE);
	}
	else
	{ SpinnerOn(hWnd,IDC_SP_GENDISPSPIN,IDC_SP_GENDISP);
 	  SpinnerOn(hgen,IDC_SP_GENEMIT2SPIN,IDC_SP_GENEMIT2);
	  SpinnerOn(hgen,IDC_SP_GENSIZESPIN,IDC_SP_GENSIZE);
	  SpinnerOn(hgen,IDC_SP_GENSIZEVARSPIN,IDC_SP_GENSIZEVAR);
	  SpinnerOn(hgen,IDC_SP_GENGROSPIN,IDC_SP_GENGRO);
	  SpinnerOn(hgen,IDC_SP_GENFADSPIN,IDC_SP_GENFAD);
	  SpinnerOn(hbubl,IDC_SP_BUBL_PERPHASPIN,IDC_SP_BUBL_PERPHA);
	  SpinnerOn(hbubl,IDC_SP_BUBL_PERPHAVARSPIN,IDC_SP_BUBL_PERPHAVAR);
	  int dist;
	  pblock->GetValue(PB_DISTRIBUTION,0,dist,FOREVER);
	  if (dist==3) SpinnerOn(hWnd,IDC_SP_PAPOINTSEDITSPIN,IDC_SP_PAPOINTSEDIT);
	  else SpinnerOff(hWnd,IDC_SP_PAPOINTSEDITSPIN,IDC_SP_PAPOINTSEDIT);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_PAPOINTSEDIT_TXT),dist==3);
	}
}
void SpawnButtons(HWND hWnd,int repi,int stype,int chunky)
{// AllSpawnBad(hWnd,stype,chunky!=METABALLS);
  SpawnStuff(hWnd,stype);
  if (chunky>=BYGEOM)  SpawnWithStype(stype,hWnd,repi);
  else ObjectMutQueOff(hWnd);
}

void ChunkStuff(HWND hWnd,BOOL ison,HWND hgen,HWND spawn,int stype,int repi,int meth=0)
{ EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKFACES),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTY),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMOOTH),ison);
  EnableWindow(GetDlgItem(hgen,IDC_SP_GENSMPLMOVE),!ison);
  EnableWindow(GetDlgItem(hgen,IDC_SP_GENSMPLTIME),!ison);
  EnableWindow(GetDlgItem(hgen,IDC_SP_GENSMPLROT),!ison);
  EnableWindow(GetDlgItem(spawn,IDC_AP_COLLIDESPAWN),!ison);
  EnableWindow(GetDlgItem(spawn,IDC_AP_DEATHSPAWN),!ison);	
  if (ison)
  {	SpinnerOn(hWnd,IDC_SP_CHUNKTHICKSPIN,IDC_SP_CHUNKTHICK);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKTHICK_TXT),TRUE);	
	if (meth==1) 
	{ SpinnerOn(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),TRUE);	
	  SpinnerOff(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),FALSE);	
	}
	else if (meth==2) 
	{ SpinnerOff(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),FALSE);	
	  SpinnerOn(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),TRUE);	
	}
	else 
	{ SpinnerOff(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),FALSE);	
	  SpinnerOff(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
	  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),FALSE);	
	}
  }
  else
  {	SpinnerOff(hWnd,IDC_SP_CHUNKTHICKSPIN,IDC_SP_CHUNKTHICK);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKTHICK_TXT),FALSE);
	SpinnerOff(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),FALSE);	
	SpinnerOff(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),FALSE);	
  }
	SpinnerOff(hWnd,IDC_AP_MATIDSPIN,IDC_AP_MATID);
	SpinnerOff(hWnd,IDC_AP_EDGEMATIDSPIN,IDC_AP_EDGEMATID);
	SpinnerOff(hWnd,IDC_AP_BACKMATIDSPIN,IDC_AP_BACKMATID);
    EnableWindow(GetDlgItem(hWnd,IDC_AP_MATID_TXT),FALSE);	
    EnableWindow(GetDlgItem(hWnd,IDC_AP_EDGEMATID_TXT),FALSE);	
    EnableWindow(GetDlgItem(hWnd,IDC_AP_BACKMATID_TXT),FALSE);	
}

void MappingStuff(PArrayParticle *po,HWND hWnd,TimeValue t)
{ int maptype;
  po->pblock->GetValue(PB_CUSTOMMATERIAL,t,maptype,FOREVER);
 if (maptype)
  { SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
    SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
   }
  else SetMapVals(po,hWnd,t);
}
void GrowFade(HWND hgen,BOOL ison)
{ if (ison)
  { SpinnerOn(hgen,IDC_SP_GENGROSPIN,IDC_SP_GENGRO);
    SpinnerOn(hgen,IDC_SP_GENFADSPIN,IDC_SP_GENFAD);
  }
  else
  { SpinnerOff(hgen,IDC_SP_GENGROSPIN,IDC_SP_GENGRO);
    SpinnerOff(hgen,IDC_SP_GENFADSPIN,IDC_SP_GENFAD);
  }
}

void ChunkOn(PArrayParticle *po,HWND hWnd,BOOL ChunkIn,TimeValue t)
{ if ((po->fragflags==METABALLS)||(po->fragflags<0))
  { MetaOff(hWnd);
	SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  }
  if (po->fragflags<=0)
  { StdStuff(hWnd,FALSE);
  }
  if ((po->fragflags==INSTGEOM)||(po->fragflags<0))
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_INSTANCESRCNAME),FALSE);
	TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
	TurnButton(hWnd,IDC_AP_TREEPICK,FALSE);
/*     int viewpt;
     po->pblock->GetValue(PB_VIEWPORTSHOWS,0,viewpt,FOREVER);
//	 if (viewpt==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	 po->pmapParam->Invalidate();*/
   InstStuff(hWnd,FALSE,po->hparam,po->spawn,TRUE);
 }
  int meth;
  po->pblock->GetValue(PB_FRAGMETHOD,0,meth,FOREVER);
/*  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_USESUBTREE),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_NOANIOFF),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIOFFBIRTH),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIOFFRND),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMINST,ison);
  EnableWindow(GetDlgItem(hparam,IDC_SP_VIEWDISPBOX),ison);
  EnableWindow(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),ison);
	if (ison)
  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);  
else
  SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);  */
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),TRUE);
  ChunkStuff(hWnd,TRUE,po->hgen,po->spawn,0,po->pickCB.repi,meth);
  int stype;
  po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
//  AllSpawnBad(po->spawn,stype,TRUE);
  int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
  if (ison) stype=0;
  SpawnStuff(po->spawn,stype);
  po->fragflags=BYGEOM;
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMINST),stype==4);
  int maptype;
  po->pblock->GetValue(PB_CUSTOMMATERIAL,0,maptype,FOREVER);
  if ((maptype==2)&&(stype!=EMIT)) po->pblock->SetValue(PB_CUSTOMMATERIAL,0,0);
  GrowFade(po->hgen,stype==EMIT);
  PForm(po->hparam,FALSE,po->hbubl,po->hgen,po->pblock);
  EnableWindow(GetDlgItem(po->hgen,IDC_SP_GENUSERATE),FALSE);
  EnableWindow(GetDlgItem(po->hgen,IDC_SP_GENUSETTL),FALSE);
  SpinnerOff(po->hgen,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
  SpinnerOff(po->hgen,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),FALSE);
  TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
  int sptype;
  po->pblock->GetValue(PB_SPINAXISTYPE,0,sptype,FOREVER);
  if (sptype==1) po->pblock->SetValue(PB_SPINAXISTYPE,0,0);
  PAStretchStuff(sptype,BYGEOM,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  StdStuff(hWnd,FALSE);
  MappingStuff(po,hWnd,t);
  int cmtl;
  po->pblock->GetValue(PB_CUSTOMMATERIAL,0,cmtl,FOREVER);
  if (cmtl>0)
  { SpinnerOn(po->hptype,IDC_AP_MATIDSPIN,IDC_AP_MATID);
    SpinnerOn(po->hptype,IDC_AP_EDGEMATIDSPIN,IDC_AP_EDGEMATID);
    SpinnerOn(po->hptype,IDC_AP_BACKMATIDSPIN,IDC_AP_BACKMATID);
    EnableWindow(GetDlgItem(po->hptype,IDC_AP_MATID_TXT),TRUE);	
    EnableWindow(GetDlgItem(po->hptype,IDC_AP_EDGEMATID_TXT),TRUE);	
    EnableWindow(GetDlgItem(po->hptype,IDC_AP_BACKMATID_TXT),TRUE);	
  }
  int showtype;
  po->pblock->GetValue(PB_PARTICLETYPE,0,showtype,FOREVER);
  if (showtype==3) po->pblock->SetValue(PB_PARTICLETYPE,0,0);
  ObjectMutQueOn(stype,po->spawn,po->pickCB.repi);
  po->pmapParam->Invalidate();
  po->pmapPSpin->Invalidate();
}

void MetaOn(PArrayParticle *po,HWND hWnd,BOOL MetaIn,TimeValue t)
{ int lastflag=po->fragflags;
  po->fragflags=METABALLS;
  SpinnerOn(hWnd,IDC_SP_METTENSSPIN,IDC_SP_METTENS);
  SpinnerOn(hWnd,IDC_SP_METTENSVARSPIN,IDC_SP_METTENSVAR);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENS_TXT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_TXT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_PCNT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_AUTOCOARSE),TRUE);
  CourseCheck(po,hWnd,t);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_DRAFTMODE),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MANYBLOBS),TRUE);
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),FALSE);
  PAStretchStuff(0,BYGEOM,po->hrot,po);
  SpinMainStuff(po->hrot,FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  StdStuff(hWnd,FALSE);
  MappingStuff(po,hWnd,t);
  SetRateSpinner(po->pblock,t,po->hgen);
 int stype;
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
 if ((lastflag==BYGEOM)||(lastflag<0))
  {		ChunkStuff(hWnd,FALSE,po->hgen,po->spawn,stype,po->pickCB.repi);
	PForm(po->hparam,TRUE,po->hbubl,po->hgen,po->pblock);
  }
  if (lastflag<=0)
  { StdStuff(hWnd,FALSE);
  }
  if ((lastflag==INSTGEOM)||(lastflag<0))
  {  InstStuff(hWnd,FALSE,po->hparam,po->spawn,TRUE);
     int maptype,viewpt;
     po->pblock->GetValue(PB_CUSTOMMATERIAL,0,maptype,FOREVER);
     if (maptype==2) po->pblock->SetValue(PB_CUSTOMMATERIAL,0,0);
		po->pblock->GetValue(PB_VIEWPORTSHOWS,0,viewpt,FOREVER);
	 if (viewpt==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
  }
//  if (stype>1) po->pblock->SetValue(PB_SPAWNTYPE,0,0);
//  AllSpawnBad(po->spawn,0,FALSE);
  int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
  if (ison) stype=0;
  SpawnStuff(po->spawn,stype);
 TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
 EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
 ObjectMutQueOff(po->spawn);
 po->pmapParam->Invalidate();
 po->pmapPSpin->Invalidate();
}

void PAAniFr(HWND hWnd,IParamBlock *pblock)
{ int anitype;
  pblock->GetValue(PB_ANIMATIONOFFSET,0,anitype,FOREVER);
  if (anitype>1)
	  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  else	SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),anitype>1);
}

void InstOn(PArrayParticle *po,HWND hWnd,TimeValue t)
{ if ((po->fragflags==METABALLS)||(po->fragflags<0))
  { MetaOff(hWnd);
    SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  }
	int stype;po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  if ((po->fragflags==BYGEOM)||(po->fragflags<0))
  {	ChunkStuff(po->hptype,FALSE,po->hgen,po->spawn,stype,po->pickCB.repi);
	PForm(po->hparam,TRUE,po->hbubl,po->hgen,po->pblock);
  }
  if (po->fragflags<=0)
  { StdStuff(hWnd,FALSE);
  }
  po->fragflags=INSTGEOM;
  InstStuff(hWnd,TRUE,po->hparam,po->spawn,TRUE);
  TurnButton(po->hptype,IDC_AP_OBJECTPICK,TRUE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),TRUE);
  PAAniFr(hWnd,po->pblock);
  int sptype;
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
  po->pblock->GetValue(PB_SPINAXISTYPE,0,sptype,FOREVER);
  PAStretchStuff(sptype,INSTGEOM,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  MappingStuff(po,hWnd,t);
  SetRateSpinner(po->pblock,t,po->hgen);
  ObjectMutQueOn(stype,po->spawn,po->pickCB.repi);
  po->pmapParam->Invalidate();
  po->pmapPSpin->Invalidate();
}

void StdOn(PArrayParticle *po,HWND hWnd,TimeValue t)
{ if ((po->fragflags==METABALLS)||(po->fragflags<0))
  { MetaOff(hWnd);
    SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  }
  if ((po->fragflags==BYGEOM)||(po->fragflags<0))
  { int stype;po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
	ChunkStuff(po->hptype,FALSE,po->hgen,po->spawn,stype,po->pickCB.repi);
	PForm(po->hparam,TRUE,po->hbubl,po->hgen,po->pblock);
  }
  if ((po->fragflags==INSTGEOM)||(po->fragflags<0))
  {  InstStuff(hWnd,FALSE,po->hparam,po->spawn,TRUE);
     int maptype,viewpt;
     po->pblock->GetValue(PB_CUSTOMMATERIAL,0,maptype,FOREVER);
     if (maptype==2) po->pblock->SetValue(PB_CUSTOMMATERIAL,0,0);
	 po->pblock->GetValue(PB_VIEWPORTSHOWS,0,viewpt,FOREVER);
	 if (viewpt==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	 po->pmapParam->Invalidate();
  }
  StdStuff(hWnd,TRUE);
  TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
  po->fragflags=0;
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
  int sptype;
  po->pblock->GetValue(PB_SPINAXISTYPE,0,sptype,FOREVER);
  PAStretchStuff(sptype,0,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  MappingStuff(po,hWnd,t);
  SetRateSpinner(po->pblock,t,po->hgen);
  int facing;
  po->pblock->GetValue(PB_PARTICLETYPE,t,facing,FOREVER);
//	if ((facing==RENDTYPE5)||(facing==RENDTYPE6))
//	{	po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
//		if (facing==2) po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
//		EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
//	}
//	else  
	  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  ObjectMutQueOff(po->spawn);
  po->pmapParam->Invalidate();
  po->pmapPSpin->Invalidate();
}

class PAParticleSpinDlgProc : public ParamMapUserDlgProc {
	public:
		PArrayParticle *po;

		PAParticleSpinDlgProc(PArrayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PAParticleSpinDlgProc::Update(TimeValue t)
{   int axis;
	po->pblock->GetValue(PB_SPINAXISTYPE,t,axis,FOREVER);
	PAStretchStuff(axis,po->fragflags,po->hrot,po);
	int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,t,ison,FOREVER);
	int stype; po->pblock->GetValue(PB_SPAWNTYPE,t,stype,FOREVER);
	IPCControls(po->hrot,po->spawn,stype,ison);
}

BOOL PAParticleSpinDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: 
		{break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_AP_PARTICLEDIRTRAVL:
			    PAStretchStuff(1,po->fragflags,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRRND:
				PAStretchStuff(0,po->fragflags,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRUSER:
				PAStretchStuff(2,po->fragflags,hWnd,po);
				break;
			  case IDC_INTERP_BOUNCEON:
				{  int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,t,ison,FOREVER);
				   int stype; po->pblock->GetValue(PB_SPAWNTYPE,t,stype,FOREVER);
				   IPCControls(hWnd,po->spawn,stype,ison);
				}
				break;
			}
		default:
			return FALSE;
	}
	return TRUE;
}


class ParticleDisableDlgProc : public ParamMapUserDlgProc {
	public:
		PArrayParticle *po;
		ICustButton *iBut;

		ParticleDisableDlgProc(PArrayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void ParticleDisableDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
	int chunky;
	int stype;
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
    int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
    if (ison) stype=0;
	SpawnButtons(po->spawn,0,stype,chunky);
	if (chunky!=po->fragflags)
	{ if (chunky==BYGEOM) ChunkOn(po,po->hptype,TRUE,t);
	  else if (chunky==METABALLS) MetaOn(po,po->hptype,TRUE,t);
	  else if (chunky==0) StdOn(po,po->hptype,t);
	  else InstOn(po,po->hptype,t);
	}
	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	if (width<0.01f) 
	{ TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);}
	po->ShowName(0);
}
void AddMtl(PArrayParticle *po,TimeValue t)
{ if (po->cnode)
	{ int subtree,frag,custmtl=0,submtl=0;
    po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
    po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
    po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	if ((po->distnode)&&(custmtl==1)) 
	{ Mtl *submtl;
	  po->cnode->SetMtl(submtl=po->distnode->GetMtl());
	  if (submtl==NULL)
	     po->cnode->SetWireColor(po->distnode->GetWireColor());	
	}
	else if ((po->custnode)&&(frag==INSTGEOM)&& custmtl) 
		po->AssignMtl(po->cnode,po->custnode,subtree,t,frag);
	else if ((po->distnode)&&(frag==BYGEOM)&&(custmtl==2))
		po->AssignMtl(po->cnode,po->distnode,subtree,t,frag);
	po->valid=FALSE;
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
 }

BOOL ParticleDisableDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
      BOOL dofrag=FALSE;
	switch (msg) {
		case WM_INITDIALOG: {
			iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_OBJECTPICK));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			Update(t);
			break;
			}
		case WM_DESTROY:
			// Release all our Custom Controls
			ReleaseICustButton(iBut);
			return FALSE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{   case IDC_AP_OBJECTPICK:
				   { 
					if (po->ip->GetCommandMode()->ID() == CID_STDPICK)
					{ if (po->creating) 
						{  theCreatePArrayMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePArrayMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;						
					      po->pickCB.dodist=0;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
			case IDC_AP_UPDATEMTL:
				{ AddMtl(po,t);
				  int custmtl;
				  po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
				  if (po->cnode)
				  {
				  if (custmtl==2) 
				  { EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMEMIT),FALSE);
				    po->origmtl=po->cnode->GetMtl();
				  }
				  else if (custmtl==1) 
				  { EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMEMIT),FALSE);
				    po->origmtl=po->cnode->GetMtl();
				  }}
				  break;
				}
			  case IDC_SP_TYPECHUNKS:
				  { ChunkOn(po,hWnd,TRUE,t);
				    dofrag=TRUE;
				    goto doshows;
				  }
			  case IDC_SP_TYPEINSTANCE:
				  {	po->pblock->SetValue(PB_CUSTOMMATERIAL,0,2);
					InstOn(po,hWnd,t);
					dofrag=FALSE;
				doshows:
					int custmtl,vshow;
						po->pblock->GetValue(PB_VIEWPORTSHOWS,0,vshow,FOREVER);
					if (vshow>1)
					{int subtree,anioff;
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 po->thePArrayDraw.t=t;
					 po->thePArrayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					 int anifr;
					 anifr=aniend+GetTicksPerFrame();
					 po->thePArrayDraw.anifr=anifr;
					 po->GetTimes(po->times,t,anifr,anioff,po->fragflags);
					 if (vshow==2)
					 { po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
				       po->GetMesh(t,subtree,custmtl,po->fragflags);
					 }
					else po->GetallBB(po->custnode,subtree,t,po->fragflags);
					}
				    po->valid=FALSE;
					if ((dofrag)&&(po->ip->GetCommandMode()->ID() == CID_STDPICK))
					{ if (po->creating) 
						{  theCreatePArrayMode.JumpStart(po->ip,po);
							   po->ip->SetCommandMode(&theCreatePArrayMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					}
				  break;
				  }
			  case IDC_AP_NOANIOFF:
			  case IDC_AP_ANIOFFBIRTH:
				  	SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),FALSE);
				  break;
			  case IDC_AP_ANIOFFRND:
			  	  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
				  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),TRUE);
				  break;
			  case IDC_SP_TYPESTD:
				{ StdOn(po,hWnd,t);
				  po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreatePArrayMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreatePArrayMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				 break;
				}
			  case IDC_SP_TYPEFAC:
			  case IDC_SP_TYPEPIX:
				{ SetMapVals(po,hWnd,t);
//				  int facing;
//				  po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
//				  if (facing==2) po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
//				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				  po->pmapParam->Invalidate();
				 break;
				}
			  case IDC_SP_TYPETRI:
			  case IDC_SP_TYPECUB:
			  case IDC_SP_TYPESPC:
			  case IDC_SP_TYPE6PNT:
			  case IDC_SP_TYPESPHERE:
				{ SetMapVals(po,hWnd,t);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
 				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				 break;
				}
			  case IDC_SP_TYPETET:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
 				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				 break;
				}
			  case IDC_SP_MAPTIME:
				{ SpinnerOn(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				 break;
				}
			  case IDC_SP_MAPDIST:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				 break;
				}
			  case IDC_SP_MAPCUSTOMEMIT:
				 {SpinnerOff(hWnd,IDC_AP_MATIDSPIN,IDC_AP_MATID);	
				  SpinnerOff(hWnd,IDC_AP_EDGEMATIDSPIN,IDC_AP_EDGEMATID);	
				  SpinnerOff(hWnd,IDC_AP_BACKMATIDSPIN,IDC_AP_BACKMATID);	
				  EnableWindow(GetDlgItem(hWnd,IDC_AP_MATID_TXT),FALSE);	
				  EnableWindow(GetDlgItem(hWnd,IDC_AP_EDGEMATID_TXT),FALSE);	
				  EnableWindow(GetDlgItem(hWnd,IDC_AP_BACKMATID_TXT),FALSE);	
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  SetMapVals(po,hWnd,t);
				  int dir;
				  po->pblock->GetValue(PB_SPINAXISTYPE,0,dir,FOREVER);
				  if (dir==1)  PACheckStretchBox(po->hrot,po);
				}
			  case IDC_SP_MAPCUSTOMDIST:
			  case IDC_SP_MAPCUSTOMINST:
				 {	  int chunky;
				  po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
				  if (chunky==BYGEOM)
				  { SpinnerOn(hWnd,IDC_AP_MATIDSPIN,IDC_AP_MATID);	
				    SpinnerOn(hWnd,IDC_AP_EDGEMATIDSPIN,IDC_AP_EDGEMATID);	
					SpinnerOn(hWnd,IDC_AP_BACKMATIDSPIN,IDC_AP_BACKMATID);	
				    EnableWindow(GetDlgItem(hWnd,IDC_AP_MATID_TXT),TRUE);	
				    EnableWindow(GetDlgItem(hWnd,IDC_AP_EDGEMATID_TXT),TRUE);	
					EnableWindow(GetDlgItem(hWnd,IDC_AP_BACKMATID_TXT),TRUE);	
				  }
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
				  SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				  int dir;
				  po->pblock->GetValue(PB_SPINAXISTYPE,0,dir,FOREVER);
				  if (dir==1)  PACheckStretchBox(po->hrot,po);
				 break;
				}
			  case IDC_SP_TYPEMET:
				  { MetaOn(po,hWnd,TRUE,t);
 				    po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreatePArrayMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreatePArrayMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				  break;
				  }
			  case IDC_SP_CHUNKFACES:
				{  SpinnerOff(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
			       EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),FALSE);	
				   SpinnerOff(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
			       EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),FALSE);	
				 break;
				}
			  case IDC_SP_CHUNKQTY:
				{  SpinnerOn(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
			       EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),TRUE);	
				   SpinnerOff(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
			       EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),FALSE);	
				 break;
				}
			  case IDC_SP_CHUNKSMOOTH:
				{ SpinnerOff(hWnd,IDC_SP_CHUNKQTYEDITSPIN,IDC_SP_CHUNKQTYEDIT);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKQTYEDIT_TXT),FALSE);	
				  SpinnerOn(hWnd,IDC_SP_CHUNKSMANGLESPIN,IDC_SP_CHUNKSMANGLE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_CHUNKSMANGLE_TXT),TRUE);	
				  break;
				}
			  case IDC_SP_AUTOCOARSE:
				{ int chunky;
				  po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
				  if (chunky==METABALLS)
				  {	 CourseCheck(po,hWnd,t);
				  po->valid=FALSE;}
				  break;
				}
			}
			break;	
		default:
			return FALSE;
		}
	return TRUE;
	}
void PArrayParticle::GetFilename(TCHAR *filename)
{   _tcscpy(filename,ip->GetDir(APP_PLUGCFG_DIR));
  int len= _tcslen(filename);
  if (len)
  {  if (filename[len-1]!=_T('\\'))
		  _tcscat(filename,_T("\\"));
  }
  _tcscat(filename,TSTR(GetString(IDS_AP_PARRAYCST)));
}
void PArrayParticle::SetupTargetList()		
	{TCHAR filename[MAX_PATH];
     FILE *f;
	 int vers,i,Namelen,osize;

	custsettings=i=0; // LAM - 6/9/02 - initialize i so if fileok == false, will display error in onerr.
    GetFilename(filename);
	BOOL fileok=TRUE;
	if ((f = _tfopen(filename, _T("rb"))) == NULL) return;
	if((ReadInt(&custsettings,f))&&(ReadInt(&vers,f))&&(ReadInt(&osize,f))&&(ReadInt(&Namelen,f)))
	{if ( ((Namelen!=NLEN)&& (fileok=GenNewNameLen(size,custsettings,f,filename,CURRENT_VERSION)))||
		((vers!=CURRENT_VERSION)&&(fileok=GenNewSaveFile(osize,size,custsettings,f,filename,CURRENT_VERSION))))
		{ fileok=((ReadInt(&custsettings,f))&&(ReadInt(&vers,f))&&(ReadInt(&osize,f))&&(ReadInt(&Namelen,f)));
		}
	if (fileok&&(vers==CURRENT_VERSION))
	{ NameLst=new AName[custsettings]; 
	  for (i=0;i<custsettings;i++)
	  if (fread(NameLst[i],NLEN,1,f)==1)
	    fseek(f,size,SEEK_CUR);
	  else goto onerr;
	}}
	onerr:if (i<custsettings) 
	{ custsettings=0;
	MessageBox (NULL,GetString(IDS_RB_BADFILE),
            "", MB_ICONINFORMATION);
	}
	if (fileok) fclose(f);
	UpdatePresetListBox(GetDlgItem(hParams2, IDC_SP_SETLIST), custsettings, NameLst);
	}
int PArrayParticle::RemSettings(int overwrite,TCHAR *newname)
{ TCHAR filename[MAX_PATH];
  FILE *f;
  long startpt;
  int vers,newsets,baselen=size+NLEN;
  GetFilename(filename);
  vers=CURRENT_VERSION;
  newsets=custsettings-1;
  if ((f = _tfopen(filename,_T("r+b"))) == NULL)
  { MessageBox(NULL,GetString(IDS_AP_WRITEPRO),"", MB_ICONINFORMATION);
	return 0;
  }
  if (custsettings==1) 
  {fclose(f);remove(filename);custsettings=0;delete[] NameLst;NameLst=NULL;
    SendMessage(GetDlgItem(hParams2,IDC_SP_SETLIST),LB_RESETCONTENT,0,0);
   return (1);}
 if (!WriteInt(&newsets,f)) {fclose(f);return 0;}
  startpt=overwrite*(baselen)+HLEN;
  fseek(f,startpt,SEEK_SET); 
  int i;
  BYTE *buf;
  buf=new BYTE[baselen];
  assert(buf);
  long cpos=startpt;
  for (i=overwrite+1;i<custsettings;i++)
  {	fseek(f,cpos+baselen,SEEK_SET);
	if (fread(buf,baselen,1,f)!=1) {delete[] buf;fclose(f);return 0;}
	fseek(f,cpos,SEEK_SET);
	if (fwrite(buf,baselen,1,f)!=1) {delete[] buf;fclose(f);return 0;}
	cpos+=baselen;
  }
  delete[] buf;
  _chsize(_fileno(f),ftell(f));
  fclose(f);								  
  AName *Tmp=new AName[custsettings-1];
  int newi=0;
  for (i=0;i<custsettings;i++)
  { if (i!=overwrite) 
      _tcscpy(Tmp[newi++],NameLst[i]);}
  delete []NameLst;NameLst=Tmp;
  custsettings--;
  UpdatePresetListBox(GetDlgItem(hParams2, IDC_SP_SETLIST), custsettings, NameLst);
  return(1);
}
int PArrayParticle::SaveSettings(int overwrite,TCHAR *newname)
{ TCHAR filename[MAX_PATH];
  FILE *f;
  int vers,newsets,Namelen=NLEN;

  GetFilename(filename);
  vers=CURRENT_VERSION;
  newsets=custsettings+1;
  if ((f = _tfopen(filename,(custsettings==0?_T("wb"):_T("r+b")))) == NULL) 
  { MessageBox(NULL,GetString(IDS_AP_WRITEPRO),"", MB_ICONINFORMATION);
	return 0;
  }

  if (custsettings==0)
  { if (!(WriteInt(&newsets,f)&&WriteInt(&vers,f)&&WriteInt(&size,f)&&WriteInt(&Namelen,f))) goto errend;
  } 
  else 
  { if (overwrite>=0) 
    { overwrite=overwrite*(size+NLEN)+HLEN;
      fseek(f,overwrite,SEEK_SET); }
    else 
	{ if (!WriteInt(&newsets,f)) goto errend;
	  fseek(f,0,SEEK_END);
	}
  }	 
  int ival,i;
  float fval;
  if (fwrite(newname,1,NLEN,f)!=NLEN) goto errend;
  for (i=0;i<PBLOCK_LENGTH_PARRAY;i++)
  {	if (spdescVer6[i].type==TYPE_INT) 
    { pblock->GetValue(i,0,ival,FOREVER);
      if (fwrite(&ival,isize,1,f)!=1) goto errend;
	}
    else
    { pblock->GetValue(i,0,fval,FOREVER);
      if (fwrite(&fval,fsize,1,f)!=1) goto errend;
	}
  }
  fclose(f);
  if (overwrite<0) 
  { AName *Tmp=new AName[custsettings+1];
    memcpy(Tmp,NameLst,sizeof(AName)*custsettings);
	delete []NameLst;NameLst=Tmp;
    _tcscpy(NameLst[custsettings],newname);
	custsettings++;
	UpdatePresetListBox(GetDlgItem(hParams2, IDC_SP_SETLIST), custsettings, NameLst);
  }  
  return(1);
errend: fclose(f);return(0);
}

int PArrayParticle::GetSettings(int setnum,TCHAR *newname)
{ TCHAR filename[MAX_PATH];
  FILE *f;
  GetFilename(filename);
  if ((f = _tfopen(filename,_T("rb"))) == NULL) return 0;
  { setnum=setnum*(size+NLEN)+HLEN;
    fseek(f,setnum,SEEK_SET); 
  }	 
  int ival,i;
  float fval;
  if (fread(newname,1,NLEN,f)!=NLEN) goto errend;
  for (i=0;i<PBLOCK_LENGTH_PARRAY;i++)
  {	if (spdescVer6[i].type==TYPE_INT) 
    { if (fread(&ival,isize,1,f)!=1) goto errend;
	  pblock->SetValue(i,0,ival);
	}
    else
    { if (fread(&fval,fsize,1,f)!=1) goto errend;
//	  if (i!=PB_EMITRWID) 
		  pblock->SetValue(i,0,fval);
	}
  }
  fclose(f);
  InvalidateUI();
  return(1);
  errend: fclose(f);return(0);
}

static INT_PTR CALLBACK CustomSettingParamDlgProc( 
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ TCHAR newname[NLEN];
  int i,save=0;
  PArrayParticle *po = (PArrayParticle*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
  if (!po && message!=WM_INITDIALOG) return FALSE;

  switch (message) {
		case WM_INITDIALOG: {
			po = (PArrayParticle*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			po->custCtrlEdit = GetICustEdit(GetDlgItem(hWnd,IDC_SP_SETEDIT));
			po->custCtrlEdit->SetText(_T(""));
			po->hParams2 = hWnd;
			po->SetupTargetList();			
			break;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SP_SETLIST:  
					if (HIWORD(wParam)==LBN_DBLCLK)
					  goto doload;
					break;
				case IDC_SP_DELETE:  
					i = SendMessage(GetDlgItem(po->hParams2,IDC_SP_SETLIST),
							LB_GETCURSEL,0,0);
					   if ((i>-1)&&(po->custsettings>0))
					   { po->custCtrlEdit->GetText(newname,NLEN);
						 if (_tcscmp(newname,po->NameLst[i])==0)
						   po->custCtrlEdit->SetText(_T(""));
					     po->RemSettings(i,po->NameLst[i]);
					   }
					break;
				case IDC_SP_SAVE:  
				   save=1;
				case IDC_SP_LOAD:
					doload:
					if (!save)
					{ i = SendMessage(GetDlgItem(po->hParams2,IDC_SP_SETLIST),
							LB_GETCURSEL,0,0);
					   if ((i>-1)&&(po->custsettings>0))
					     po->custCtrlEdit->SetText(po->NameLst[i]);
					}
				    po->custCtrlEdit->GetText(newname,NLEN);
					if (save) 
					{int tstblk=0,tstlen=_tcslen(newname);
					  while ((tstblk<tstlen)&&(newname[tstblk]==' ')) tstblk++;
					  if (tstblk>=tstlen) 
						MessageBox (NULL,GetString(IDS_RB_NONAME),
            "", MB_ICONINFORMATION);
					  else 
					  { i=0;
					    while ((i<po->custsettings)&&(_tcscmp(newname,po->NameLst[i])))
					     i++;
						if (i>=po->custsettings) i=-1;
					    po->SaveSettings(i,newname);
					  }
					}
					else if ((i>-1)&&(i<po->custsettings) )
					{ po->GetSettings(i,newname);
					  InitParams(po,po->hparam);
					  po->valid=FALSE;
					  int chunky;
			          po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
					  int stype;
					  po->pblock->GetValue(PB_SPAWNTYPE,po->pickCB.repi=0,stype,FOREVER);
					  int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
					  if (ison) stype=0;
					  SpawnButtons(po->spawn,0,stype,chunky);
			          if (chunky==BYGEOM) ChunkOn(po,po->hptype,TRUE,po->ip->GetTime());
					  else if (chunky==METABALLS) MetaOn(po,po->hptype,TRUE,po->ip->GetTime());
				      else if (chunky==0) StdOn(po,po->hptype,po->ip->GetTime());
					  else InstOn(po,po->hptype,po->ip->GetTime());
					  if (stype==EMIT) 
					  {	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
  						EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
					  }
					int onscreen;
					po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
					if (((chunky==INSTGEOM)||(chunky==BYGEOM))&&(onscreen>1))
					{int custmtl;
					po->pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
					int subtree,anioff;
					TimeValue t=po->ip->GetTime();
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 po->thePArrayDraw.t=t;
					 po->thePArrayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					 int anifr;
					 anifr=aniend+GetTicksPerFrame();
					 po->thePArrayDraw.anifr=anifr;
					 po->GetTimes(po->times,t,anifr,anioff,po->fragflags);
					if (onscreen==2) po->GetMesh(po->ip->GetTime(),subtree,custmtl,chunky);
					else po->GetallBB(po->custnode,subtree,po->ip->GetTime(),chunky);
					}
					  po->ip->RedrawViews(po->ip->GetTime()); 
					}
					else MessageBox (NULL,GetString(IDS_RB_BADNAME),
            "", MB_ICONINFORMATION);
				}
			break;

		case WM_DESTROY:
			// Release all our Custom Controls
			ReleaseICustEdit(po->custCtrlEdit);
			if (po->NameLst) {delete []po->NameLst;po->NameLst=NULL;}
			return FALSE;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			po->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;
		
		default:
			return FALSE;
		}
	return TRUE;
	}	

class PAParticleSpawnDlgProc : public ParamMapUserDlgProc {
	public:
		PArrayParticle *po;

		ICustButton *iBut,*iButrep;
		PAParticleSpawnDlgProc(PArrayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PAParticleSpawnDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
	int stype,rep;
	po->SetUpList();
	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	if (width<0.01f) iBut->Disable();
	po->SetUpLifeList();
	po->pickCB.repi= SendMessage(GetDlgItem(po->spawn,IDC_AP_OBJECTQUEUE),
					LB_GETCURSEL,0,0);
	if (width<0.01f) iButrep->Disable();
	else
	{ if (po->pickCB.repi<0) iButrep->Disable(); else iButrep->Enable();
	}
	rep = SendMessage(GetDlgItem(po->spawn,IDC_AP_LIFEQUEUE),LB_GETCURSEL,0,0);
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
	int chunky;
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
    int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
    if (ison) stype=0;
	SpawnButtons(po->spawn,po->pickCB.repi,stype,chunky);
	TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,0);
	TurnButton(po->spawn,IDC_AP_LIFEQUEUEREPL,0);
}

BOOL PAParticleSpawnDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{  int dtype=2,stype,rep;	
	switch (msg) {
		case WM_INITDIALOG: 
		{ 	iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUEPICK));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			iButrep = GetICustButton(GetDlgItem(hWnd,IDC_AP_OBJQUEUEREPLACE));
			iButrep->SetType(CBT_CHECK);
			iButrep->SetHighlightColor(GREEN_WASH);
			Update(t);
			break;
		}
		case WM_DESTROY:
			// Release all our Custom Controls
			ReleaseICustButton(iBut);
			ReleaseICustButton(iButrep);
			return FALSE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_AP_NOSPAWN:stype=0;goto spawnradio;
			  case IDC_AP_COLLIDEDIE:
				  stype=1;goto spawnradio;
			  case IDC_AP_SPAWNTRAILS:stype=EMIT;goto spawnradio;
			  case IDC_AP_COLLIDESPAWN:
			  case IDC_AP_DEATHSPAWN: stype=2;
			spawnradio:	
				int chunky;
				po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
				int ison; po->pblock->GetValue(PB_PAIPCOLLIDE_ON,0,ison,FOREVER);
				if (ison) stype=0;
				SpawnButtons(hWnd,po->pickCB.repi,stype,chunky);
				 if (chunky==BYGEOM)
				 {	EnableWindow(GetDlgItem(po->hptype,IDC_SP_MAPCUSTOMINST),stype==EMIT);
				    int maptype;
					po->pblock->GetValue(PB_CUSTOMMATERIAL,0,maptype,FOREVER);
 					if ((maptype==2)&&(stype!=EMIT)) po->pblock->SetValue(PB_CUSTOMMATERIAL,0,0);
					GrowFade(po->hgen,(stype==EMIT));
				 } else GrowFade(po->hgen,TRUE);
			   break;
			  case IDC_AP_OBJECTQUEUE:
				{ po->pickCB.repi= SendMessage(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUE),
							LB_GETCURSEL,0,0);
			    TurnButton(hWnd,IDC_AP_OBJQUEUEREPLACE,po->pickCB.repi>-1);
			    TurnButton(hWnd,IDC_AP_OBJQUEUEDELETE,po->pickCB.repi>-1);
				break;
				}
			   case IDC_AP_OBJECTQUEUEPICK:
			      dtype=2;goto dopick;
				case IDC_AP_OBJQUEUEREPLACE:
				  dtype=3;
				  dopick:
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePArrayMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePArrayMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;
					      po->pickCB.dodist=dtype;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
				case IDC_AP_OBJQUEUEDELETE:
				{  int i = SendMessage(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUE),
							LB_GETCURSEL,0,0);
					if ((po->nlist.Count()>0)&&(i>-1))
					{	theHold.Begin();
						po->DeleteFromList(i,0);
						theHold.Accept(GetString(IDS_AP_OBJDEL));
					}
						TurnButton(hWnd,IDC_AP_OBJQUEUEREPLACE,0);
						TurnButton(hWnd,IDC_AP_OBJQUEUEDELETE,0);
					break;
				}
				case IDC_AP_LIFEQUEUEDEL:
				{  int i = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),
							LB_GETCURSEL,0,0);
					int Lcnt=po->llist.Count();
					if ((Lcnt>0)&&(i>-1)&&(i<Lcnt))
					{	theHold.Begin();
						theHold.Put(new PALifeListRestore(po));
						po->DeleteFromLifeList(i);
						TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
						theHold.Accept(GetString(IDS_AP_LIFEDEL));
					}
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					break;
				}
				case IDC_AP_LIFEQUEUEADD:
				{   int i;
					po->pblock->GetValue(PB_SPAWNLIFEVLUE,t,i,FOREVER);
					theHold.Begin();
					theHold.Put(new PALifeListRestore(po));
					po->AddToLifeList(i);
				    TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
					theHold.Accept(GetString(IDS_AP_LIFEADD));
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					break;
				}
				case IDC_AP_LIFEQUEUE:
				{ rep = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),LB_GETCURSEL,0,0);
				  TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,rep>-1);
				  break;
				}
				case IDC_AP_LIFEQUEUEREPL:
				{ int i;
				   rep = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),
							LB_GETCURSEL,0,0);
				  if (rep>-1)
				  {	po->pblock->GetValue(PB_SPAWNLIFEVLUE,t,i,FOREVER);
					theHold.Begin();
					theHold.Put(new PALifeListRestore(po));
					po->llist[rep]=i;
					po->SetUpLifeList();
					TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
					theHold.Accept(GetString(IDS_AP_LIFEREP));
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				  }
					break;
				}
				
			}
			break;	
		default:
			return FALSE;
		}
	return TRUE;
}


//--- PArrayParticle Methods--------------------------------------------
void PArrayParticle::ResetSystem(TimeValue t,BOOL full)
{	lc.lastmin=-1;lc.lastcollide=-1;
	rcounter=0;
	vcounter=0;
	if (full)
	{ tvalid = t;
	  valid  = TRUE;
	}
}

PArrayParticle::PArrayParticle()
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
     int tpf=GetTicksPerFrame();

	MakeRefByID(FOREVER, 0, CreateParameterBlock(spdescVer6, PBLOCK_LENGTH_PARRAY, CURRENT_VERSION));
	pblock->SetValue(PB_DISTRIBUTION,0,0);
	pblock->SetValue(PB_EMITTERCOUNT,0,20);
	pblock->SetValue(PB_SPEED,0,10.0f);
	pblock->SetValue(PB_SPEEDVAR,0,0.0f);
	pblock->SetValue(PB_ANGLEDIV,0,DegToRad(10));

	pblock->SetValue(PB_PBIRTHRATE,0,10);
	pblock->SetValue(PB_PTOTALNUMBER,0,100);
	pblock->SetValue(PB_BIRTHMETHOD,0,0);
	pblock->SetValue(PB_DISPLAYPORTION,0,0.1f);
	pblock->SetValue(PB_EMITSTART,0,TimeValue(0));
	pblock->SetValue(PB_EMITSTOP,0,(TimeValue)30*FToTick);// correct constant?
	pblock->SetValue(PB_DISPUNTIL,0,100*FToTick);// correct constant?
	pblock->SetValue(PB_LIFE,0,30*FToTick);// correct constant?
	pblock->SetValue(PB_LIFEVAR,0,0);
	pblock->SetValue(PB_SUBFRAMEMOVE,0,1);
	pblock->SetValue(PB_SUBFRAMETIME,0,1);
	pblock->SetValue(PB_SUBFRAMEROT,0,0);
	pblock->SetValue(PB_SIZE,0,1.0f);
	pblock->SetValue(PB_SIZEVAR,0,0.0f);
	pblock->SetValue(PB_GROWTIME,0,10*FToTick);
	pblock->SetValue(PB_FADETIME,0,10*FToTick);
	pblock->SetValue(PB_RNDSEED,0,12345);

	pblock->SetValue(PB_PARTICLETYPE,0,0);
	pblock->SetValue(PB_METATENSION,0,1.0f);
	pblock->SetValue(PB_METATENSIONVAR,0,0.0f);
	pblock->SetValue(PB_METAAUTOCOARSE,0,1);
	pblock->SetValue(PB_METACOURSE,0,0.5f);
	pblock->SetValue(PB_METACOURSEV,0,1.0f);
	pblock->SetValue(PB_FRAGTHICKNESS,0,1.0f);
	pblock->SetValue(PB_FRAGMETHOD,0,0);
	pblock->SetValue(PB_FRAGCOUNT,0,100);
	pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	pblock->SetValue(PB_MAPPINGTYPE,0,0);
	pblock->SetValue(PB_MAPPINGTIME,0,30*FToTick);
	pblock->SetValue(PB_MAPPINGDIST,0,100.0f);
	pblock->SetValue(PB_CUSTOMMATERIAL,0,0);

	pblock->SetValue(PB_SPINTIME,0,0);
	pblock->SetValue(PB_SPINTIMEVAR,0,0.0f);
	pblock->SetValue(PB_SPINPHASE,0,0.0f);
	pblock->SetValue(PB_SPINPHASEVAR,0,0.0f);
	pblock->SetValue(PB_SPINAXISTYPE,0,0);
	pblock->SetValue(PB_SPINAXISX,0,1.0f);
	pblock->SetValue(PB_SPINAXISY,0,0.0f);
	pblock->SetValue(PB_SPINAXISZ,0,0.0f);
	pblock->SetValue(PB_SPINAXISVAR,0,0.0f);

	pblock->SetValue(PB_EMITVINFL,0,100.0f);
	pblock->SetValue(PB_EMITVMULT,0,1.0f);
	pblock->SetValue(PB_EMITVMULTVAR,0,0.0f);

	pblock->SetValue(PB_BUBLAMP,0,0.0f);
	pblock->SetValue(PB_BUBLAMPVAR,0,0.0f);
	pblock->SetValue(PB_BUBLPER,0,100000*FToTick);
	pblock->SetValue(PB_BUBLPERVAR,0,0.0f);
	pblock->SetValue(PB_BUBLPHAS,0,0.0f);
	pblock->SetValue(PB_BUBLPHASVAR,0,0.0f);

	pblock->SetValue(PB_EMITRWID,0,0.0f);
	pblock->SetValue(PB_EMITRHID,0,0);
	pblock->SetValue(PB_EMAT,0,2);
	pblock->SetValue(PB_BMAT,0,3);
	pblock->SetValue(PB_SPAWNGENS,0,1);
	pblock->SetValue(PB_SPAWNCOUNT,0,1);
	pblock->SetValue(PB_SPAWNPERCENT,0,100);
	pblock->SetValue(PB_PANOTDRAFT,0,0);
	pblock->SetValue(PB_USESELECTED,0,0);
	pblock->SetValue(PB_PASPAWNDIEAFTER,0,0);
	pblock->SetValue(PB_PASPAWNDIEAFTERVAR,0,0.0f);
	pblock->SetValue(PB_PAIPCOLLIDE_ON,0,0);
	pblock->SetValue(PB_PAIPCOLLIDE_STEPS,0,2);
	pblock->SetValue(PB_PAIPCOLLIDE_BOUNCE,0,1.0f);
	pblock->SetValue(PB_PAIPCOLLIDE_BOUNCEVAR,0,0.0f);
	sdata=NULL;
	pmesh=NULL;
	distnode=NULL;
	custnode=NULL;
	custname=TSTR(_T(" "));
	distname=TSTR(_T(" "));
	cnode=NULL;
	storefrag=NULL;
	ResetSystem(0,FALSE);
	doTVs=FALSE;
	storernd=12345;
	size=51*isize+fsize*36;
	times.tl.SetCount(0);
	cmesh=NULL;
	cmbb=NULL;
	dispmesh=NULL;
	cancelled=FALSE;
	wasmulti=FALSE;
	dispt=-99999;
	if (storefrag) delete[] storefrag;
	thePArrayDraw.bboxpt=NULL;
	llist.ZeroCount();
	nlist.ZeroCount();
	nmtls.ZeroCount();
	parts.points.ZeroCount();
	maincount=0;
	fragflags=-1;
	dflags=APRTS_ROLLUP_FLAGS;
	backpatch=TRUE;
	origmtl=NULL;
	ClearAFlag(A_NOTREND);
    stepSize=GetTicksPerFrame();
}

PArrayParticle::~PArrayParticle()
{
	if (sdata) {delete[] sdata;sdata=NULL;}
	if (pmesh) {delete[] pmesh;pmesh=NULL;}
	DeleteAllRefsFromMe();
	pblock=NULL;
	parts.FreeAll();
	times.tl.SetCount(0);
	times.tl.Shrink();
	nmtls.ZeroCount();nmtls.Shrink();
	llist.ZeroCount();llist.Shrink();
	nlist.ZeroCount();nlist.Shrink();
	if (cmesh) delete[] cmesh;
	if (cmbb) delete[] cmbb;
	if (thePArrayDraw.bboxpt) delete[] thePArrayDraw.bboxpt;
	if (dispmesh) delete dispmesh;
	lastTMTime = TIME_NegInfinity;
}
class PArrayPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		PArrayPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((PArrayParticle*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			int mc;
			if (oldVer<1) {	
				((PArrayParticle*)targ)->pblock->GetValue(PB_METACOURSE,0,mc,FOREVER);
				((PArrayParticle*)targ)->pblock->SetValue(PB_METACOURSEV,0,mc);
				}
			if (oldVer<2) {	
				((PArrayParticle*)targ)->pblock->SetValue(PB_SUBFRAMEROT,0,0);
				}
			if (oldVer<3) {	
				((PArrayParticle*)targ)->pblock->SetValue(PB_SPAWNPERCENT,0,100);
				}
			if (oldVer<4) {	
				((PArrayParticle*)targ)->pblock->SetValue(PB_PANOTDRAFT,0,0);
				((PArrayParticle*)targ)->pblock->SetValue(PB_USESELECTED,0,0);
				}
			if (oldVer<5) {	
				((PArrayParticle*)targ)->pblock->SetValue(PB_PASPAWNDIEAFTER,0,0);
				((PArrayParticle*)targ)->pblock->SetValue(PB_PASPAWNDIEAFTERVAR,0,0.0f);
				}
			if (oldVer<6)
			{ ((PArrayParticle*)targ)->pblock->SetValue(PB_PAIPCOLLIDE_ON,0,0);
			  ((PArrayParticle*)targ)->pblock->SetValue(PB_PAIPCOLLIDE_STEPS,0,2);
			  ((PArrayParticle*)targ)->pblock->SetValue(PB_PAIPCOLLIDE_BOUNCE,0,1.0f);
			  ((PArrayParticle*)targ)->pblock->SetValue(PB_PAIPCOLLIDE_BOUNCEVAR,0,0.0f);
			}
			delete this;
			}
	};

#define PARRAY_DISTNAME_CHUNK	0x0100
#define PARRAY_CUSTNAME_CHUNK	0x0200
#define PARRAY_CUSTFLAGS_CHUNK	0x0300
#define PARRAY_SPAWNC_CHUNK		0x0400
#define PARRAY_LCNT_CHUNK		0x0500
#define PARRAY_LIFE_CHUNK		0x0600

IOResult PArrayParticle::Save(ISave *isave)
	{ULONG nb;

	isave->BeginChunk(PARRAY_DISTNAME_CHUNK);		
	isave->WriteWString(distname);
	isave->EndChunk();
	isave->BeginChunk(PARRAY_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();

	int nCount=nlist.Count();
	isave->BeginChunk(PARRAY_SPAWNC_CHUNK);		
	isave->Write(&nCount,sizeof(nCount),&nb);
	isave->EndChunk();

	int Lcnt=llist.Count();
	isave->BeginChunk(PARRAY_LCNT_CHUNK);		
	isave->Write(&Lcnt,sizeof(Lcnt),&nb);
	isave->EndChunk();

	isave->BeginChunk(PARRAY_LIFE_CHUNK);
	for (int i=0;i<Lcnt;i++)
	{ isave->Write(&llist[i],sizeof(int),&nb);
	}
	isave->EndChunk();

	return IO_OK;
	}

IOResult PArrayParticle::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	DWORD flags;
	// Default names
	distname = TSTR(_T(" "));
	int cnmtl=0;
	iload->RegisterPostLoadCallback(
			new PArrayPostLoadCallback(
				new ParamBlockPLCB(paversions,NUM_OLDVERSIONS,&curVersionPA,this,0)));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case PARRAY_DISTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				distname = TSTR(buf);
				break;
				}
			case PARRAY_CUSTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
				}
			case PARRAY_CUSTFLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;
			case PARRAY_SPAWNC_CHUNK:
				{ int nCount=nlist.Count();
					res=iload->Read(&nCount,sizeof(nCount),&nb);
					nlist.SetCount(nCount);
					for (int i=0; i<nCount; i++) nlist[i] = NULL;
				}
				break;
			case PARRAY_LCNT_CHUNK:
				{	int Lcnt;
					res=iload->Read(&Lcnt,sizeof(Lcnt),&nb);
					llist.SetCount(Lcnt);
					for (int i=0; i<Lcnt; i++) llist[i] = NULL;
				}
				break;
			case PARRAY_LIFE_CHUNK:
				{	for (int i=0;i<llist.Count();i++)
					res=iload->Read(&llist[i],sizeof(int),&nb);}
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}

int PArrayParticle::RenderBegin(TimeValue t, ULONG flags)
	{ 	SetAFlag(A_RENDER);
  		ParticleInvalid();		
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)1);
		if (distnode)
			distnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)1);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		cancelled=FALSE;
		return 0;
	}

int PArrayParticle::RenderEnd(TimeValue t)
	{
		ClearAFlag(A_RENDER);
		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)0);
		if (distnode)
			distnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)0);
	return 0;
	}

void PArrayParticle::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{ 
	SimpleParticle::BeginEditParams(ip,flags,prev);
	editOb = this;
	this->ip = ip;
	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else { creating = FALSE; }

	if (pmapParam && pmapPGen && pmapPType && pmapPSpin && pmapEmitV && pmapBubl && pmapSpawn) 
	{	pmapParam->SetParamBlock(pblock);
		pmapPGen->SetParamBlock(pblock);
		pmapPType->SetParamBlock(pblock);
		pmapPSpin->SetParamBlock(pblock);
		pmapEmitV->SetParamBlock(pblock);
		pmapBubl->SetParamBlock(pblock);
		pmapSpawn->SetParamBlock(pblock);
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG_PTR)this);
	} else 
	{ 	pmapParam = CreateCPParamMap(
			descParamPArray,PARAMPARRAY_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTICLEARRAY),
			GetString(IDS_RB_PARAMETERS),
			dflags&APRTS_ROLLUP1_OPEN?0:APPENDROLL_CLOSED);

		pmapPGen = CreateCPParamMap(
			descParamPGen,PARAMPGEN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_GEN_PA),
			GetString(IDS_RB_PGEN),
			dflags&APRTS_ROLLUP2_OPEN?0:APPENDROLL_CLOSED);
		
		pmapPType = CreateCPParamMap(
			descParamPType,PARAMPTYPE_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTTYPE_PA),
			GetString(IDS_RB_PTYPE),
			dflags&APRTS_ROLLUP3_OPEN?0:APPENDROLL_CLOSED);		
	
		pmapPSpin = CreateCPParamMap(
			descParamPSpin,PARAMPSPIN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_ROT),
			GetString(IDS_RB_SPIN),
			dflags&APRTS_ROLLUP4_OPEN?0:APPENDROLL_CLOSED);		
	
		pmapEmitV = CreateCPParamMap(
			descParamEmitV,PARAMEMITV_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_EMITV),
			GetString(IDS_RB_EMITV),
			dflags&APRTS_ROLLUP5_OPEN?0:APPENDROLL_CLOSED);		
	
		pmapBubl = CreateCPParamMap(
			descParamBubl,PARAMBUBL_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_BUBL),
			GetString(IDS_RB_BUBL),
			dflags&APRTS_ROLLUP6_OPEN?0:APPENDROLL_CLOSED);		
		pmapSpawn = CreateCPParamMap(
			descPSpawning,PSPAWNINGPARAMS_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_SPAWNING),
			GetString(IDS_AP_PSPAWN),
			dflags&APRTS_ROLLUP7_OPEN?0:APPENDROLL_CLOSED);		
	
		hParams2 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SUPRPRTS_SAVE),
				CustomSettingParamDlgProc, 
				GetString(IDS_RB_LOADSAVE), 
				(LPARAM)this,dflags&APRTS_ROLLUP8_OPEN?0:APPENDROLL_CLOSED);	
		
		fragflags=-1;
		ip->RegisterDlgWnd(hParams2);

		}
	spawn=pmapSpawn->GetHWnd();
	hparam=pmapParam->GetHWnd();
	hgen=pmapPGen->GetHWnd();
	hptype=pmapPType->GetHWnd();
	hrot=pmapPSpin->GetHWnd();
	hbubl=pmapBubl->GetHWnd();
	if (pmapParam)
		pmapParam->SetUserDlgProc(new ParticleDlgProc(this));
	if (pmapPType) pmapPType->SetUserDlgProc(new ParticleDisableDlgProc(this));
	if (pmapPGen) pmapPGen->SetUserDlgProc(new ParticleGenDlgProc(this));
	if (pmapSpawn) pmapSpawn->SetUserDlgProc(new PAParticleSpawnDlgProc(this));
	if (pmapPSpin) pmapPSpin->SetUserDlgProc(new PAParticleSpinDlgProc(this));
}	

void PArrayParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{	SimpleParticle::EndEditParams(ip,flags,next);
	TimeValue t0,t2;
	SetFlag(dflags,APRTS_ROLLUP1_OPEN,IsRollupPanelOpen(hparam));
	SetFlag(dflags,APRTS_ROLLUP2_OPEN,IsRollupPanelOpen(hgen));
	SetFlag(dflags,APRTS_ROLLUP3_OPEN,IsRollupPanelOpen(hptype));
	SetFlag(dflags,APRTS_ROLLUP4_OPEN,IsRollupPanelOpen(hrot));
	SetFlag(dflags,APRTS_ROLLUP5_OPEN,IsRollupPanelOpen(pmapEmitV->GetHWnd()));
	SetFlag(dflags,APRTS_ROLLUP6_OPEN,IsRollupPanelOpen(hbubl));
	SetFlag(dflags,APRTS_ROLLUP7_OPEN,IsRollupPanelOpen(spawn));
	SetFlag(dflags,APRTS_ROLLUP8_OPEN,IsRollupPanelOpen(hParams2));
	editOb = NULL;

	if (flags&END_EDIT_REMOVEUI) {
		pblock->GetValue(PB_EMITSTART,0,t0,FOREVER);
		pblock->GetValue(PB_EMITSTOP,0,t2,FOREVER);
		if (t2<t0) pblock->SetValue(PB_EMITSTOP,0,t0);
		DestroyCPParamMap(pmapParam);
		DestroyCPParamMap(pmapPGen);
		DestroyCPParamMap(pmapPType);
		DestroyCPParamMap(pmapPSpin);
		DestroyCPParamMap(pmapEmitV);
		DestroyCPParamMap(pmapBubl);
		DestroyCPParamMap(pmapSpawn);

		ip->UnRegisterDlgWnd(hParams2);
		ip->DeleteRollupPage(hParams2);
		hParams2 = NULL;

		pmapParam  = NULL;
		pmapPGen = NULL;
		pmapPType = NULL;
		pmapPSpin = NULL;
		pmapEmitV = NULL;
		pmapBubl = NULL;
		pmapSpawn = NULL;
	}else
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG)NULL);
	ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
	}

void PArrayParticle::MapKeys(TimeMap *map,DWORD flags)
{	Animatable::MapKeys(map,flags);
	TimeValue TempTime;
// remap values
	pblock->GetValue(PB_EMITSTART,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_EMITSTART,0,TempTime);
	pblock->GetValue(PB_EMITSTOP,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_EMITSTOP,0,TempTime);
	pblock->GetValue(PB_DISPUNTIL,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_DISPUNTIL,0,TempTime);
// scaled values
	pblock->GetValue(PB_LIFE,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_LIFE,0,TempTime);
	pblock->GetValue(PB_LIFEVAR,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_LIFEVAR,0,TempTime);
	pblock->GetValue(PB_GROWTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_GROWTIME,0,TempTime);
	pblock->GetValue(PB_FADETIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_FADETIME,0,TempTime);
	pblock->GetValue(PB_MAPPINGTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_MAPPINGTIME,0,TempTime);
	pblock->GetValue(PB_SPINTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_SPINTIME,0,TempTime);
	pblock->GetValue(PB_BUBLPER,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_BUBLPER,0,TempTime);
//	pblock->GetValue(PB_OFFSETAMOUNT,0,TempTime,FOREVER);
//	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_OFFSETAMOUNT,0,TempTime);
}  

Interval PArrayParticle::GetValidity(TimeValue t)
	{
	// For now...
	return Interval(t,t);
	}

TimeValue PArrayParticle::ParticleLife(TimeValue t, int i)
{   int pcount=parts.Count();
	if (!(i<pcount)) return 0;
	return sdata[i].L;
}

Point3 PArrayParticle::ParticlePosition(TimeValue t,int i)
{	int pcount=parts.points.Count();
	if (!(i<pcount)) return Zero;
	Point3 FinalP=parts.points[i];
// figure out particle class
	int isinst;
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
// if we have custom geometry, find the center of the bounding box and velocity normal
	if ((isinst==INSTGEOM)&&(custnode))
	{	int anioff;
		float dlgsize;
		dlgsize=parts.radius[i];
		float zoffset=0.0f;
		Point3 OffsetV=Zero;;
		TimeValue aniend=GetAnimEnd();
		int anifr=aniend+GetTicksPerFrame();
		pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
		TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
		int nCount=nlist.Count();
		int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum));
		if (mnum>=0) zoffset=cmbb[mnum].Center().z;
// if we're using DoT/MBlur, account for scaling due to stretch
		int axisentered;
		pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
		if (axisentered==DIRTRAVEL)
		{	int K;
			pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			float strlen=GetLen(parts.vels[i],K);
			zoffset*=strlen;
		}
		float lenV=Length(parts.vels[i]);
		if (lenV>EPSILON) OffsetV=parts.vels[i]/lenV;
		FinalP+=zoffset*dlgsize*OffsetV;
	}
	return FinalP;
}

Point3 PArrayParticle::ParticleVelocity(TimeValue t,int i)
{	Point3 retvel=Zero;
	int pcount=parts.vels.Count();
	if (i<pcount)
		retvel=parts.vels[i];
	else
		return retvel;
	return retvel;
}

float PArrayParticle::ParticleSize(TimeValue t,int i)
{	float strlen=1.0f;
	float boxlen=1.0f;
	float dlgsize;
	int axisentered,K,isinst,ptype;
// get the size/scale from the dialog box processed for this particle...
	int pcount=parts.radius.Count();
	if (!(i<pcount)) return 0.0f;
	dlgsize=parts.radius[i];
// figure out particle type and class
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
// if there are custom particles, find bounding box params...
	if ((isinst==INSTGEOM)&&(custnode))
	{	int anioff;
		TimeValue aniend=GetAnimEnd();
		int anifr=aniend+GetTicksPerFrame();
		pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
		TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
		int nCount=nlist.Count();
		int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum));
		if (mnum>=0) boxlen=cmbb[mnum].Width().z;
	}
// if we're using MBlur and DoT then account for scaling along DoT
	pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	if (axisentered==DIRTRAVEL)
	{	pblock->GetValue(PB_STRETCH,t,K,FOREVER);
		strlen=GetLen(parts.vels[i],K);
	}
	float templen=boxlen*strlen;
	if ((isinst!=PB_PARTICLECLASS)&&(ptype==RENDTET)) templen*=1.3f;
	return templen*dlgsize;	
}

int PArrayParticle::ParticleCenter(TimeValue t,int i)
{	int ptype,isinst;
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	if (isinst==INSTGEOM) return PARTCENTER_CENTER;
	if (ptype==RENDTET) return PARTCENTER_HEAD;
	return PARTCENTER_CENTER;	
}

void PArrayParticle::BuildEmitter(TimeValue t, Mesh& amesh)
	{
	float width;
	mvalid = FOREVER;
	pblock->GetValue(PB_EMITRWID,t,width,mvalid);
	width  *= 0.5f;

	mesh.setNumVerts(21); //9+12
	mesh.setNumFaces(24); //12+12
	mesh.setVert(0, Point3(-width,-width, width));
	mesh.setVert(1, Point3( width,-width, width));
	mesh.setVert(2, Point3( width, width, width));
	mesh.setVert(3, Point3(-width, width, width));
	mesh.setVert(4, Point3(-width,-width, -width));
	mesh.setVert(5, Point3( width,-width, -width));
	mesh.setVert(6, Point3( width, width, -width));
	mesh.setVert(7, Point3(-width, width, -width));
	mesh.setVert(8, Point3(0,0,0));

	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[0].setVerts(0,1,3);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[1].setVerts(1,2,3);

	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[2].setVerts(1,0,4);

	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[3].setVerts(5,1,4);

	mesh.faces[4].setEdgeVisFlags(0,1,1);
	mesh.faces[4].setSmGroup(0);
	mesh.faces[4].setVerts(5,2,1);

	mesh.faces[5].setEdgeVisFlags(1,1,0);
	mesh.faces[5].setSmGroup(0);
	mesh.faces[5].setVerts(5,6,2);

	mesh.faces[6].setEdgeVisFlags(1,0,1);
	mesh.faces[6].setSmGroup(0);
	mesh.faces[6].setVerts(2,6,3);

	mesh.faces[7].setEdgeVisFlags(1,1,0);
	mesh.faces[7].setSmGroup(0);
	mesh.faces[7].setVerts(6,7,3);

	mesh.faces[8].setEdgeVisFlags(1,1,0);
	mesh.faces[8].setSmGroup(0);
	mesh.faces[8].setVerts(0,3,7);

	mesh.faces[9].setEdgeVisFlags(1,0,1);
	mesh.faces[9].setSmGroup(0);
	mesh.faces[9].setVerts(4,0,7);

	mesh.faces[10].setEdgeVisFlags(0,1,1);
	mesh.faces[10].setSmGroup(0);
	mesh.faces[10].setVerts(6,4,7);

	mesh.faces[11].setEdgeVisFlags(1,1,0);
	mesh.faces[11].setSmGroup(0);
	mesh.faces[11].setVerts(6,5,4);
	float r6=0.6f*width,a=0.2f*width,a73=a*0.73f,a5=a*0.5f,a7=a*1.0f;
	int nv=9,nf=12,bnv;
	Point3 basept[3]={Point3(r6,0.0f,-r6),Point3(0.0f,0.0f,0.5f*width),Point3(-r6,0.0f,-r6)};
	for (int i=0;i<3;i++)
	{ bnv=nv;
	  mesh.setVert(nv++, Point3(0.0f,a,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(-a73,-a5,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(a73,-a5,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(0.0f,0.0f,a7)+basept[i]);
	  mesh.faces[nf].setVerts(bnv,bnv+1,bnv+3);
	  mesh.faces[nf].setEdgeVisFlags(1,1,1);
	  mesh.faces[nf++].setSmGroup(0);
	  mesh.faces[nf].setVerts(bnv+1,bnv+2,bnv+3);
	  mesh.faces[nf].setEdgeVisFlags(1,1,1);
	  mesh.faces[nf++].setSmGroup(0);
	  mesh.faces[nf].setVerts(bnv+2,bnv,bnv+3);
	  mesh.faces[nf].setEdgeVisFlags(1,1,1);
	  mesh.faces[nf++].setSmGroup(0);
	  mesh.faces[nf].setVerts(bnv+2,bnv+1,bnv);
	  mesh.faces[nf].setEdgeVisFlags(1,1,1);
	  mesh.faces[nf++].setSmGroup(0);
	}
	mesh.InvalidateGeomCache();
	}

int PArrayParticle::CountLive()
{	int c=0;
	for (int i=0; i<parts.Count(); i++)
	  {if (parts.Alive(i)) c++;}
	return c;
}



// ******************************************************
int amatch(DWORD *ptr,DWORD nxt)
{ return((ptr[0]==nxt)||(ptr[1]==nxt)||(ptr[2]==nxt));
}
Point3 GetNormalizedNormal(ulong *curface,Point3 *v)
{return(Normalize((v[curface[0]]-v[curface[1]])^(v[curface[0]]-v[curface[2]])));
}

Point3 AveNormal(Point3 newnorm,Point3 oldnorm)
{ Point3 aveN;

  aveN=(newnorm+oldnorm)/2.0f;
  if (Length(aveN)==0.0f)
  { 
	  // Martell 4/14/01: Fix for order of ops bug.
	  float z=RND11(); float y=RND11(); float x=RND11();
	  aveN=Point3(x,y,z);
  }
  return(Normalize(aveN));
}

void SetCore(int faces,int Groups,int *toplist,int *nextlist)
{ int i,nface;
  int newnum;

  nface=faces-1;
  for (i=0;i<Groups;i++)
  { newnum=RND0x(nface);
    while (nextlist[newnum]>-2)
     if (++newnum>=faces) newnum=0;
    toplist[i]=newnum;nextlist[newnum]=-1;
  }
}

int isnext(int faces,int *j,int *nextlist)
{ while ((*j<faces)&&(nextlist[*j]>-2)) 
   (*j)++;
  return(*j<faces);
}
BOOL checkvoverlap(int seedface,int *nextlist,DWORD *nextptr,int *matches,Face *f)
{ BOOL notfound[3];
 notfound[0]=TRUE;notfound[1]=TRUE;notfound[2]=TRUE;
 do
 {if ((notfound[0])&&(amatch(&f[seedface].v[0],nextptr[0])))
  { notfound[0]=FALSE;(*matches)++;}
  if ((notfound[1])&&(amatch(&f[seedface].v[0],nextptr[1])))
  { notfound[1]=FALSE;(*matches)++;}
  if ((notfound[2])&&(amatch(&f[seedface].v[0],nextptr[2])))
  { notfound[2]=FALSE;(*matches)++;}
 }while ((*matches<3)&&((seedface=(nextlist[seedface]))>-1));
  return(*matches>1);
};

typedef struct
{ int fnum,sides;
} EdgeFace;
typedef Tab<EdgeFace> TEdge;
typedef Tab<Edge> OTEdge;
int EdgeFound(TEdge gl,int num)
{ BOOL found=FALSE;int cnt=gl.Count(),i=0;
  while ((!found)&&(i<cnt))
    if (!(found=(gl[i].fnum==num))) i++;
	return (found?i:-1);
}
void FillInEdges(TEdge &gl,OTEdge &el,DWORD tl,int *nxtlst)
{ int e=0,num,f;
  while (e<el.Count())
  { if ((el[e].f[f=0]==tl)||(el[e].f[f=1]==tl))
	{ f=(f?0:1);
	  num=EdgeFound(gl,el[e].f[f]);
      if (num>-1) gl[num].sides++;
	  else if (nxtlst[el[e].f[f]]==-2)
	  { EdgeFace *tmp=new EdgeFace;
		tmp->fnum=el[e].f[f];tmp->sides=1;
		gl.Append(1,tmp,0);
	  }
	  el.Delete(e,1);
	}
    else e++;
  }
}

int FillInGroups(int faces,int basegroup,int *toplist,int* nextlist,Face *f,Point3 *v,OTEdge el)
{ int count,i,j,lastnum,fullcount;
  int topptr[3];
  BOOL *full;
  int Groups=basegroup;

  if (Groups>faces) Groups=faces;
  SetCore(faces,Groups,toplist,nextlist);
  full=new BOOL[faces];
  assert(full!=NULL);
  for (i=0;i<faces;i++) full[i]=FALSE;
  fullcount=0;count=Groups; 
  TEdge *glst;glst=new TEdge[Groups];
  for (int gx=0;gx<Groups;gx++) 
  { glst[gx].SetCount(0);
    FillInEdges(glst[gx],el,toplist[gx],nextlist);
	if (glst[gx].Count()==0) {full[gx]=TRUE;fullcount++;}
  }
  while ((fullcount<Groups)&&(count<faces))
  { i=0;
    while ((i<Groups)&&(count<faces))
    { if (!full[i])
	{	topptr[0]=f[toplist[i]].v[0];topptr[1]=f[toplist[i]].v[1];
	    topptr[2]=f[toplist[i]].v[2];
        j=0;
		Point3 seedcenter=(v[topptr[0]]+v[topptr[1]]+v[topptr[2]])/3.0f;
		int maxs=0;	float Mindist;int ThreeFace=-1; 
		for (int x=0;x<glst[i].Count();x++)
		{ if (nextlist[j=glst[i][x].fnum]>-2) glst[i].Delete(x,1);
		  else if (glst[i][x].sides>=maxs)
		  { float dist=Length(seedcenter-(v[f[j].v[0]]+v[f[j].v[1]]+v[f[j].v[2]])/3.0f);
			if ((glst[i][x].sides>maxs)||(Mindist>dist))
		    { ThreeFace=glst[i][x].fnum; Mindist=dist;}
		    maxs=glst[i][x].sides;
		  }
		}
		if (ThreeFace>-1)
		{ lastnum=toplist[i];while (nextlist[lastnum]>-1) lastnum=nextlist[lastnum];
		  int minj=ThreeFace;
		  nextlist[minj]=-1;nextlist[lastnum]=minj;count++;
		  FillInEdges(glst[i],el,ThreeFace,nextlist);
		  glst[i].Delete(ThreeFace,1);
		  if (glst[i].Count()==0) {full[i]=TRUE;fullcount++;}
		}
		else {full[i]=TRUE;fullcount++;}
      }
      i++;
    }
  }
  if (count<faces) 
  { i=0;
    while ((count<faces)&&(i<faces))
    { if (nextlist[i]<-1)
      {nextlist[i]=-1;toplist[Groups++]=i;count++;}
	  i++;
    }
  }
  if (full) delete[] full;
  if (glst) delete[] glst;
  return(Groups);
}

typedef struct{
  DWORD vfrom,vto,face;
  Point3 normal;
}fEdge;
typedef struct{
  DWORD vfrom,vto,fnum;
  int f0,f1;
}Edgelst;

void getext(float *min,float *max,float *p0,float *p1,float *p2)
{ int i;

  for (i=0;i<3;i++)
  { min[i]=(p0[i]<p1[i]?(p2[i]<p0[i]?p2[i]:p0[i]):(p2[i]<p1[i]?p2[i]:p1[i]));
    max[i]=(p0[i]>p1[i]?(p2[i]>p0[i]?p2[i]:p0[i]):(p2[i]>p1[i]?p2[i]:p1[i]));
  }
}
void FindGroupCenters(int Groups,int *toplist,int *nextlist,ParticleSys *parts,Face *f,Point3 *v)
{ int j;
  float mins[3],maxs[3],facemin[3],facemax[3];

  for (int i=0;i<Groups;i++)
  { j=toplist[i];
    maxs[0]=mins[0]=v[f[j].v[0]].x;
    maxs[1]=mins[1]=v[f[j].v[0]].y;
    maxs[2]=mins[2]=v[f[j].v[0]].z;
    do
    { getext(facemin,facemax,&v[f[j].v[0]].x,&v[f[j].v[1]].x,&v[f[j].v[2]].x);
      if (facemin[0]<mins[0]) mins[0]=facemin[0];
      else if (facemax[0]>maxs[0]) maxs[0]=facemax[0];
      if (facemin[1]<mins[1]) mins[1]=facemin[1];
      else if (facemax[1]>maxs[1]) maxs[1]=facemax[1];
      if (facemin[2]<mins[2]) mins[2]=facemin[2];
      else if (facemax[2]>maxs[2]) maxs[2]=facemax[2];
    } while ((j=nextlist[j])>-1);
	parts->points[i]=Point3((mins[0]+maxs[0])/2.0f,(mins[1]+maxs[1])/2.0f,(mins[2]+maxs[2])/2.0f);
 }
}

void FindAllNormals(int faces,Point3 *norm,Point3 *v,Face *f)
{ int i;
  for (i=0;i<faces;i++)
    norm[i]=GetNormalizedNormal(&f[i].v[0],v);
}

/*void GetXYZ(int num, *ptr)
{ memcpy(ptr,&(fdata[num].v[0]),3*sizeof(ushort));
}  */

BOOL CheckForMatch(Point3 *norm,int one,int two,float NAngle)
{ float inangle,newangle;

   inangle=DotProd(norm[one],norm[two]);
   newangle=(float)acos((inangle>1?1:(inangle<-1?-1:inangle)));
   return((newangle>=-NAngle)&&(newangle<=NAngle));
}

BOOL ismatch(DWORD *ptr,int *nxt,int one,int two,Point3 *norm,float NAngle)
{BOOL found;
 int matches;
  matches=0;if (amatch(ptr,nxt[0])) matches++;if (amatch(ptr,nxt[1])) matches++;
  if ((matches<2)&&(amatch(ptr,nxt[2]))) matches++;
  found=(matches>1)&&(CheckForMatch(norm,one,two,NAngle));
  return(found);
}

BOOL PtInCommon(int topnum,int curnum,int *toplist,int *nextlist,int *nextptr,int *endnum,Point3 *norm,float NAngle,Face *f,Point3 *v)
{ BOOL found,done;
  DWORD ptr[3];

  done=found=0;
  topnum=toplist[topnum];
  ptr[0]=f[topnum].v[0];ptr[1]=f[topnum].v[1];ptr[2]=f[topnum].v[2];
  while (done==FALSE)
  { found=ismatch(ptr,nextptr,topnum,curnum,norm,NAngle);
    if ((found==0)&&(nextlist[topnum]>-1)) 
	{ topnum=nextlist[topnum];
      ptr[0]=f[topnum].v[0];ptr[1]=f[topnum].v[1];ptr[2]=f[topnum].v[2];
	}
    else done=1;
  }
  if (found)
  { while (nextlist[topnum]>-1) topnum=nextlist[topnum]; }
  *endnum=topnum;
  return(found);
}

typedef struct
{ DWORD v[3];
  DWORD fnum;
}ftov;
typedef struct
{ DWORD v0,v1;
}dupedge;
void AddVertsToGroup(DWORD *vlst,int *vcount,DWORD *curface,Point3 norm,Point3 *vnorm,ftov *fnew,int *vsused)
{ BOOL found;
  int j;
  for (int i=0;i<3;i++)
  { j=0;found=FALSE;
    while ((!found)&&(j<*vcount))
	{found=(vlst[j]==curface[i]);
	  if (!found) j++;
	}
	if (!found) 
	{vlst[(*vcount)++]=curface[i]; 
	 if (vnorm) {vnorm[j]=norm;vsused[j]=1;}}
	else if (vnorm) {vnorm[j]+=norm;vsused[j]++;}
	fnew->v[i]=j;
  }
}
BOOL SharedEdge(ftov f1,int ffrom,int fto,ftov f2,DWORD v0,DWORD v1)
{ return(((f1.v[ffrom]==f2.v[v0])&&(f1.v[fto]==f2.v[v1]))||
		 ((f1.v[fto]==f2.v[v0])&&(f1.v[ffrom]==f2.v[v1])));
}

BOOL IsDup(DWORD v0a,DWORD v1a,DWORD v0b,DWORD v1b)
{ return(((v0a==v0b)&&(v1a==v1b))||((v0a==v1b)&&(v1a==v0b)));
}
int CheckForGroupEdges(int count,ftov *flst,Edgelst *edges)
{ BOOL found=FALSE;
  int innercount,ecount=0,fto=0;
  dupedge *de=new dupedge[count*3];int dc=0;
  for (int fcount=0;fcount<count;fcount++)
    for (int fedge=0;fedge<3;fedge++)
	{ found=FALSE;innercount=fcount+1;fto=(fedge<2?fedge+1:0);
	  int cdc=0;BOOL isd=FALSE;
	  while ((!found)&&(cdc<dc)) 
	  {	found=(isd=IsDup(de[cdc].v0,de[cdc].v1,flst[fcount].v[fedge],flst[fcount].v[fto]));
	    cdc++;
	  }
      while ((!found)&&(innercount<count))
	  {found=(SharedEdge(flst[fcount],fedge,fto,flst[innercount],0,1)||SharedEdge(flst[fcount],fedge,fto,flst[innercount],1,2)||
	         SharedEdge(flst[fcount],fedge,fto,flst[innercount],2,0));
	   innercount++;
	  }
	  if (!found)
	  { edges[ecount].vfrom=flst[fcount].v[fedge];
	    edges[ecount].fnum=fcount;
		edges[ecount].f0=fedge;edges[ecount].f1=fto;
	    edges[ecount++].vto=flst[fcount].v[fto];
	  } else if (!isd) {de[dc].v0=flst[fcount].v[fedge];de[dc].v1=flst[fcount].v[fto];dc++;}
    }
	if (de) delete[] de;
  return(ecount);
}

int FillInSmoothing(int faces,float NAngle,Point3* norm,int *toplist,int* nextlist,Face *f,Point3 *v)
{ int topptr[3],nextptr[3],Groups;
  int count,j,lastnum;

  Groups=count=0;
  while (count<faces)
  { j=0;
    isnext(faces,&j,nextlist);
	toplist[Groups]=j;nextlist[j]=-1;count++;j++;
	topptr[0]=f[j].v[0];topptr[1]=f[j].v[1];topptr[2]=f[j].v[2];
	BOOL newfound=TRUE;
	while (newfound==TRUE)
	{ newfound=FALSE;
    while ((count<faces)&&(isnext(faces,&j,nextlist)))
    { nextptr[0]=f[j].v[0];nextptr[1]=f[j].v[1];nextptr[2]=f[j].v[2];
      if (PtInCommon(Groups,j,toplist,nextlist,nextptr,&lastnum,norm,NAngle,f,v)>0)
       {nextlist[j]=-1;nextlist[lastnum]=j;count++;newfound=TRUE;}
      j++;
    } j=0;
	}
    Groups++;
  }
  return(Groups);
}
typedef struct
{ Matrix3 oldtm,newtm;
  Matrix3 curtm;
}Matlist;


void MakeIntoMesh(int Groups,int faces,Point3* norm,int *toplist,int *nextlist,float thickness,Mesh *mesh,Mesh *pmesh,ParticleSys *parts,SavePt *s,Point3 *lastv,Face *lastf,Matlist m,int emat,int bmat,int fmat,BOOL doTVs)
{DWORD *vlst;
 int *vsused;
 BOOL thick=(thickness>0.0f);
 Point3 *vnorm;
 int count,i,Fnum,vcount=0,ecount=0;
  Point3 *oldcenters=new Point3[faces],lastpt;
  vlst=new DWORD[3*faces];
  FindGroupCenters(Groups,toplist,nextlist,parts,lastf,lastv);
  for (i=0;i<Groups;i++)
  {	oldcenters[i]=parts->points[i]; }
  FindGroupCenters(Groups,toplist,nextlist,parts,mesh->faces,mesh->verts);
  ftov *flst;
  vnorm=(thick?new Point3[3*faces]:NULL);
  vsused=(thick?new int[3*faces]:NULL);
  flst=new ftov[faces];
  Edgelst* edges=NULL;
  for (int j=0;j<Groups;j++)
	{ 
//		if (s[j].M! = 0.0f)
//		{ 
			lastpt = oldcenters[j];
			s[j].Ve = parts->points[j]*(m.newtm)-lastpt*(m.oldtm);
//		} 
//		else 
//			s[j].Ve = Zero;
    Fnum=toplist[j]; count=0;ecount=0;vcount=0;
	do
    { AddVertsToGroup(vlst,&vcount,&(mesh->faces[Fnum].v[0]),norm[Fnum],vnorm,&flst[count],vsused);
	  flst[count].fnum=Fnum;
	  count++;
    } while ((count<faces)&&((Fnum=nextlist[Fnum])>-1));
    parts->vels[j]=norm[toplist[j]];
	if (thick) 
      parts->points[j]-=parts->vels[j]*(thickness*(1+s[j].Vsz))/2.0f;
	pmesh[j].setNumVerts(thick?vcount*2:vcount);
	for (i=0;i<vcount;i++)
	{ pmesh[j].verts[i]=(mesh->verts[vlst[i]]-parts->points[j])*(m.curtm);
	  if (thick)
	  {Point3 aveN=vnorm[i]/(float)vsused[i];
	   if (Length(aveN)==0.0f)
	   {
			// Martell 4/14/01: Fix for order of ops bug.
			float z=RND11(); float y=RND11(); float x=RND11();
			aveN=Point3(x,y,z);
	   }
        pmesh[j].verts[i+vcount]=pmesh[j].verts[i]-thickness*aveN*(m.curtm);
	  }
	}
	int fc=2*count;
	if (thick)
	{ edges=(thick?new Edgelst[3*count]:NULL);
	  ecount=CheckForGroupEdges(count,flst,edges);
	  pmesh[j].setNumFaces(2*(count+ecount));
	  if (doTVs)
	   pmesh[j].setNumTVFaces(pmesh[j].numFaces);
	  else pmesh[j].setNumTVFaces(0);
	  for (i=0;i<count;i++)
	  { int icount=i+count;
		pmesh[j].faces[i]=mesh->faces[flst[i].fnum];
		if (fmat>0) pmesh[j].faces[i].setMatID(fmat-1);
		pmesh[j].faces[i].setVerts(flst[i].v[0],flst[i].v[1],flst[i].v[2]);
		pmesh[j].faces[icount]=pmesh[j].faces[i];
		pmesh[j].faces[icount].setVerts(flst[i].v[0]+vcount,flst[i].v[2]+vcount,flst[i].v[1]+vcount);
		if (bmat>0) pmesh[j].faces[icount].setMatID(bmat-1);
		else pmesh[j].faces[icount].setMatID(pmesh[j].faces[i].getMatID());
		if (doTVs)
		{ pmesh[j].tvFace[icount]=(pmesh[j].tvFace[i]=mesh->tvFace[flst[i].fnum]);
		}
	  }	
	  for (i=0;i<ecount;i++)
	  {	MtlID ematerial;
		pmesh[j].faces[fc+1]=(pmesh[j].faces[fc]=pmesh[j].faces[edges[i].fnum+count]);
	    pmesh[j].faces[fc].setVerts(edges[i].vfrom,edges[i].vfrom+vcount,edges[i].vto+vcount);
		pmesh[j].faces[fc].setSmGroup(0);
	    pmesh[j].faces[fc+1].setVerts(edges[i].vfrom,edges[i].vto+vcount,edges[i].vto);
		pmesh[j].faces[fc+1].setEdgeVisFlags(0,1,1);
		pmesh[j].faces[fc+1].setSmGroup(0);
		if (emat>0)	ematerial=emat-1; else ematerial=pmesh[j].faces[fc].getMatID();
		pmesh[j].faces[fc].setMatID(ematerial);
		pmesh[j].faces[fc+1].setMatID(ematerial);
		if (doTVs)
		{ DWORD tv0=pmesh[j].tvFace[edges[i].fnum].t[edges[i].f0],tv1=pmesh[j].tvFace[edges[i].fnum].t[edges[i].f1];
		 pmesh[j].tvFace[fc].setTVerts(tv0,tv0,tv1);
		 pmesh[j].tvFace[fc+1].setTVerts(tv0,tv1,tv1);
		}
		fc+=2;
	  }
	}
	else
	{ pmesh[j].setNumFaces(count);
	  if (doTVs)
	   pmesh[j].setNumTVFaces(pmesh[j].numFaces);
	  else pmesh[j].setNumTVFaces(0);
	  for (i=0;i<count;i++)
	  { pmesh[j].faces[i]=mesh->faces[flst[i].fnum];
	    if (fmat>0) pmesh[j].faces[i].setMatID(fmat-1);
		pmesh[j].faces[i].setVerts(flst[i].v[0],flst[i].v[1],flst[i].v[2]);
		if (doTVs)
		 pmesh[j].tvFace[i]=(pmesh[j].tvFace[i]=mesh->tvFace[flst[i].fnum]);
	  }
	}
    if (edges) delete[] edges;
  }
  if (flst) delete[] flst;
  if (vlst) delete[] vlst;
  if (vnorm) delete[] vnorm;
  if (vsused) delete[] vsused;
  if (oldcenters) delete[] oldcenters;
}

void SelectEmitterPer(int emitters,int pcount,int *lst)
{ int i,j,rounds,counter=0,emits=emitters-1;

  rounds=(int)floor((float)pcount/emitters);
  for (j=0;j<emitters;j++)
  {   counter+=(lst[j]=rounds);
  }
  for (i=counter;i<pcount;i++)
    { j=RND0x(emits);
      lst[j]++;
    }
}

void SpreadOutParts(float *arealst,float TotalArea,int maxfaces,int block,int total,int *lst)
{int i,newcount;

  newcount=0;
  for (i=0;i<block;i++)
  {	newcount+=(lst[i]=(int)floor(total*(arealst[i]/TotalArea)));
  }
  for (i=newcount;i<total;i++)
   lst[RND0x(maxfaces)]+=1;
}

float GetFaceArea(DWORD *curface,Point3 *v)
{ Point3 V1,V2;

  V1=v[curface[1]]-v[curface[0]];
  V2=v[curface[2]]-v[curface[0]];
/* Point3 D,N;
  float h,b;
  N=V1^V2;
	D=Normalize(N^V1);
  h=(float)fabs(DotProd(D,v[curface[2]])-DotProd(D,v[curface[0]]));
  b=Length(V1);
  return(0.5f*b*h);	 */
  return(Length(V1^V2)*0.5f);
}

void FillByFixedPts(int infaces,int emits,int c,int vertices, int lastrnd, Mesh *mesh,ParticleSys *parts,SavePt *s,Point3 *lastv,Face *lastf,Matlist m,BOOL doTVs,int mf,BOOL doall)
{ int i,*elst,*plst,ecount=0;
  float *arealst,TotalArea,*rndx,*rndy;
  Point3 Norm;

  arealst=new float[infaces];
  elst=new int[infaces];
  plst=new int[emits];
  rndx=new float[emits];
  rndy=new float[emits];
  assert(arealst && elst && plst);
  TotalArea=0.0f;int selface=0;
  for (i=0;i<infaces;i++)
  { if (doall || (mesh->faceSel[i])) 
	{ TotalArea+=(arealst[selface]=GetFaceArea(&mesh->faces[i].v[0],mesh->verts));
      selface++;
	}
  }
  //sort fixed emitters
  SpreadOutParts(arealst,TotalArea,selface-1,selface,emits,elst);
  for (i=0;i<emits;i++)
  { rndx[i]=RND01();rndy[i]=RND01();}
  //set parts to emitters
  srand(lastrnd);
  SelectEmitterPer(emits,c,plst);
  Point3 V0,V1,V2,V01,V02,lastpt,lV0,lV01,lV02;
  float rx,ry;int count=0;
  for (i=0;i<infaces;i++)
    if (doall || (mesh->faceSel[i])) 
  { V01=(V1=mesh->verts[mesh->faces[i].v[1]])-(V0=mesh->verts[mesh->faces[i].v[0]]);
    V02=(V2=mesh->verts[mesh->faces[i].v[2]])-V0;
	int li=(i>mf?mf:i);
	lV01=(lastv[lastf[li].v[1]])-(lV0=lastv[lastf[li].v[0]]);
    lV02=(lastv[lastf[li].v[2]])-lV0;
    Norm=GetNormalizedNormal(&mesh->faces[i].v[0],mesh->verts);
    for (int j=0;j<elst[count];j++)
    { if (plst[ecount]>0)
	  { parts->vels[vertices]=Norm;
        rx=rndx[ecount];ry=rndy[ecount];
        if (rx+ry>1) {rx=1-rx;ry=1-ry;}
	    parts->points[vertices]=V0+V01*rx+V02*ry;
//		if (s[vertices].M!=0.0f)
//		{	
			lastpt = lV0+lV01*rx+lV02*ry;
			s[vertices].Ve = parts->points[vertices]*(m.newtm)-lastpt*(m.oldtm);
//		} 
//		else 
//			s[vertices].Ve = Zero;
	    if (doTVs)
	    { Point3 tv0=mesh->tVerts[mesh->tvFace[i].t[0]];
	      Point3 tv1=mesh->tVerts[mesh->tvFace[i].t[1]];
	      Point3 tv2=mesh->tVerts[mesh->tvFace[i].t[2]];
	      s[vertices].tv=Point3((1-ry)*(rx*tv1.x+tv0.x-rx*tv0.x)+ry*tv2.x,(1-ry)*(rx*tv1.y+tv0.y-rx*tv0.y)+ry*tv2.y,0.0f);
		  s[vertices].pmtl=mesh->faces[i].getMatID();
	    }
	    int localv=vertices+1;
		for (int k=1;k<plst[ecount];k++)
		{ parts->points[localv]=parts->points[vertices];
		  parts->vels[localv]=parts->vels[vertices];
		  s[localv].Ve = s[vertices].Ve;
		  if (doTVs) 
		  { s[localv].tv=s[vertices].tv;
			s[localv].pmtl=s[vertices].pmtl;
		  }
		  localv++;
		}
	    vertices=localv;
	  }
	  ecount++;
	}
	count++;
  }
  delete[] arealst;
  delete[] plst;
  delete[] elst;
  delete[] rndx;
  delete[] rndy;
  lastrnd=rand();
} 

void FillInUniform(int infaces,int c,int vertices,Mesh *mesh,ParticleSys *parts,SavePt *s,Point3 *lastv,Face *lastf,Matlist m,BOOL doTVs,int mf,BOOL doall)
{ int i,*lst;
  float *arealst,TotalArea,rx,ry;
  Point3 Norm;

  arealst=new float[infaces];
  lst=new int[infaces];
  assert(arealst && lst);
  TotalArea=0.0f;int fsel=0;
  for (i=0;i<infaces;i++)
  { if (doall || (mesh->faceSel[i])) 
	{ TotalArea+=(arealst[fsel]=GetFaceArea(&mesh->faces[i].v[0],mesh->verts));
	  fsel++;
	}
  }
  SpreadOutParts(arealst,TotalArea,fsel-1,fsel,c,lst);
  Point3 V0,V1,V2,V01,V02,lastpt,lV0,lV01,lV02;
  int count=0;
  for (i=0;i<infaces;i++)
    if (doall || mesh->faceSel[i])
	{ if (lst[count]>0)
		{ V01=(V1=mesh->verts[mesh->faces[i].v[1]])-(V0=mesh->verts[mesh->faces[i].v[0]]);
	  V02=(V2=mesh->verts[mesh->faces[i].v[2]])-V0;
	  int li=(i>mf?mf:i);
	  lV01=(lastv[lastf[li].v[1]])-(lV0=lastv[lastf[li].v[0]]);
      lV02=(lastv[lastf[li].v[2]])-lV0;
      Norm=GetNormalizedNormal(&mesh->faces[i].v[0],mesh->verts);
      for (int j=0;j<lst[count];j++)
	  { parts->vels[vertices]=Norm;
        rx=RND01();ry=RND01();
        if (rx+ry>1) {rx=1-rx;ry=1-ry;}
	    parts->points[vertices]=V0+V01*rx+V02*ry;
//	    if (s[vertices].M!=0.0f)
//		{ 
			lastpt = lV0 + lV01*rx + lV02*ry;
			s[vertices].Ve = parts->points[vertices]*(m.newtm)-lastpt*(m.oldtm);
//		} 
//		else 
//			s[vertices].Ve = Zero;
	    s[vertices].pmtl=mesh->faces[i].getMatID();
	    if (doTVs)
		{ Point3 tv0=mesh->tVerts[mesh->tvFace[i].t[0]];
		  Point3 tv1=mesh->tVerts[mesh->tvFace[i].t[1]];
		  Point3 tv2=mesh->tVerts[mesh->tvFace[i].t[2]];
	      s[vertices].tv=Point3((1-ry)*(rx*tv1.x+tv0.x-rx*tv0.x)+ry*tv2.x,(1-ry)*(rx*tv1.y+tv0.y-rx*tv0.y)+ry*tv2.y,0.0f);
		}
	    vertices++;
	  }
	}
	count++;
	}
  delete[] arealst;
  delete[] lst;
}

float Checkfordup(DWORD *curface,int P0,int P1,float *arealst,fEdge *edgelst,int *edges,int face,Point3 *vlst,Face *f)
{ int i,found;
  float area;
  Point3 norm;

  found=i=0;area=0.0f;
  while ((!found)&&(i<*edges))
   { found=((edgelst[i].vto==curface[P0])&&(edgelst[i].vfrom==curface[P1]))||
            ((edgelst[i].vfrom==curface[P0])&&(edgelst[i].vto==curface[P1]));
     if (!found) i++;
   }
   if (found)
   { norm=GetNormalizedNormal(curface,vlst);
     edgelst[i].normal=AveNormal(norm,edgelst[i].normal);
     if (f[face].getMatID()!=f[edgelst[i].face].getMatID())
     { if (GetFaceArea(curface,vlst)>GetFaceArea(&f[edgelst[i].face].v[0],vlst))
         edgelst[i].face=face;
     }
   }
   else
   {area=(arealst[i]=Length(vlst[curface[P0]]-vlst[curface[P1]]));
    edgelst[i].vfrom=curface[P0];edgelst[i].vto=curface[P1];
    edgelst[i].normal=GetNormalizedNormal(curface,vlst);
    edgelst[i].face=face;
    (*edges)++;
   }
 return(area);
}

float GetEdgeArea(Mesh *mesh,float *arealst,fEdge *edgelst,int *edges,int face,BOOL byedge)
{ float area;
  DWORD *curface=&(mesh->faces[face]).v[0],flags=mesh->faces[face].flags;
  area=0.0f;int fnum=face*3;
  if ( ((flags & EDGE_A)>0)&&((!byedge)||(mesh->edgeSel[fnum])))
    area+=Checkfordup(curface,0,1,arealst,edgelst,edges,face,mesh->verts,mesh->faces);
  if ( ((flags & EDGE_B)>0)&&((!byedge)||(mesh->edgeSel[fnum+1])))
    area+=Checkfordup(curface,1,2,arealst,edgelst,edges,face,mesh->verts,mesh->faces);
  if ( ((flags & EDGE_C)>0)&&((!byedge)||(mesh->edgeSel[fnum+2])))
    area+=Checkfordup(curface,2,0,arealst,edgelst,edges,face,mesh->verts,mesh->faces);
  return(area);
}

void FillByEdges(int infaces,int c,int vertices,Mesh *mesh,ParticleSys *parts,SavePt *s,Point3 *lastv,Matlist m,BOOL doTVs,int mv,BOOL doall)
{ int i,edges,numedges=3*infaces,*lst;
  float *arealst,TotalArea,r;
  fEdge *edgelst;
  Point3 lastpt;

  edges=0;
  assert((arealst=new float[numedges])!=NULL);
  assert((edgelst=new fEdge[numedges])!=NULL);
  lst=new int[numedges];assert(lst);
  TotalArea=0.0f;int fsel=0;
  BOOL byedge=(!doall)&&(mesh->edgeSel.NumberSet()>0);
  for (i=0;i<infaces;i++)
  {	if (doall || ((byedge)||(mesh->faceSel[i])) )
	{ TotalArea+=GetEdgeArea(mesh,arealst,edgelst,&edges,i,byedge);
	  fsel++;
	}
  }
  SpreadOutParts(arealst,TotalArea,edges-1,edges,c,lst);
  for (i=0;i<edges;i++)
  if (lst[i]>0)
  { Point3 pt=mesh->verts[edgelst[i].vfrom],pdist=mesh->verts[edgelst[i].vto]-pt;
  Point3 lpt=lastv[(edgelst[i].vfrom>(DWORD)mv?mv:edgelst[i].vfrom)],lpdist=lastv[(edgelst[i].vto>(DWORD)mv?mv:edgelst[i].vto)]-lpt;
	for (int j=0;j<lst[i];j++)
	{ r=RND01();
      parts->points[vertices]=pt+pdist*r;
//	  if (s[vertices].M!=0.0f)
//	  { 
			lastpt = lpt + lpdist*r;
			s[vertices].Ve = parts->points[vertices]*(m.newtm)-lastpt*(m.oldtm);
//	  } 
//	  else 
//		  s[vertices].Ve = Zero;
      parts->vels[vertices]=edgelst[i].normal;
	if (doTVs) 
	{ DWORD *vlst;vlst=&mesh->faces[edgelst[i].face].v[0];
	  int st0=(vlst[0]==edgelst[i].vfrom?0:(vlst[1]==edgelst[i].vfrom?1:2));
	  int st1=(vlst[0]==edgelst[i].vto?0:(vlst[1]==edgelst[i].vto?1:2));
	  Point3 tv0=mesh->tVerts[mesh->tvFace[edgelst[i].face].t[st0]];
	  Point3 tv1=mesh->tVerts[mesh->tvFace[edgelst[i].face].t[st1]];
	  s[vertices].tv=Point3(tv0.x+(tv1.x-tv0.x)*r,tv0.y+(tv1.y-tv0.y)*r,0.0f);
	}
	  s[vertices].pmtl=mesh->faces[edgelst[i].face].getMatID();
      vertices++;
    }
  }
  delete[] arealst;
  delete[] edgelst;
}

BOOL VertexinWhichFace(int v,Face *flst,int infaces,int *aface)
{ BOOL found;

  found=0;
  while ((*aface<infaces)&&(!found))
  { found=amatch(flst[*aface].v,v);
    if (!found) (*aface)++;
  }
  return(found);
}

void FillInFaces(int faces,int vertices,Mesh *mesh,int c,ParticleSys *parts,SavePt *s,Point3 *lastv,Face *lastf,Matlist m,BOOL doTVs,int mf,BOOL doall)
{ int i,j,*lst,localf;
  Point3 lastpt;int fsel=0;
  fsel=(doall?faces:mesh->faceSel.NumberSet());
  lst=new int[fsel];
  SelectEmitterPer(fsel,c,lst);
  int count=0;
  for (i=0;i<faces;i++)
  { int li=(i>mf?mf:i);
    if (doall ||(mesh->faceSel[i]))
	{ if (lst[count]>0)
	  { parts->points[vertices]=(mesh->verts[mesh->faces[i].v[0]]+mesh->verts[mesh->faces[i].v[1]]+mesh->verts[mesh->faces[i].v[2]])/3.0f;
		parts->vels[vertices]=GetNormalizedNormal(&mesh->faces[i].v[0],mesh->verts);
//		if (s[vertices].M!=0.0f)
//		{ 
			lastpt=(lastv[lastf[li].v[0]]+lastv[lastf[li].v[1]]+lastv[lastf[li].v[2]])/3.0f;
			s[vertices].Ve = parts->points[vertices]*(m.newtm)-lastpt*(m.oldtm);
//		} else s[vertices].Ve = Zero;
		if (doTVs)
		{ s[vertices].tv=(mesh->tVerts[mesh->tvFace[i].t[0]]+mesh->tVerts[mesh->tvFace[i].t[1]]+mesh->tVerts[mesh->tvFace[i].t[2]])/3.0f;
		}
		s[vertices].pmtl=mesh->faces[i].getMatID();
		localf=vertices+1;
		for (j=1;j<lst[count];j++)
		{ parts->points[localf]=parts->points[vertices];
		  parts->vels[localf]=parts->vels[vertices];
		  s[localf].Ve = s[vertices].Ve;
		  if (doTVs) 
		  { s[localf].tv=s[vertices].tv;
		  }
		  s[localf].pmtl=s[vertices].pmtl;
		  localf++;
		}
		vertices=localf;
		}
	  count++;
	}
  }
  delete[] lst;
}

void FillInVertex(int inverts,int infaces,int vertices,Mesh *mesh,int c,ParticleSys *parts,SavePt *s,Point3 *lastv,Matlist m,BOOL doTVs,int mv,BOOL doall)
{ int i,j,face,ncounter,firstface,*lst,localv;
  Point3 newNorm,zero=Point3(0.0f,0.0f,0.0f),lastpt;

  int vsel=0;
  vsel=(doall?inverts:mesh->vertSel.NumberSet());
  lst=new int[vsel];
  SelectEmitterPer(vsel,c,lst);
  int count=0;
  for (i=0;i<inverts;i++)
   if (doall ||mesh->vertSel[i])
   { if (lst[count]>0)
	{ parts->points[vertices]=mesh->verts[i];
//	  if (s[vertices].M!=0.0f)
//	  { 
			lastpt=lastv[(i>mv?mv:i)];
			s[vertices].Ve = parts->points[vertices]*(m.newtm)-lastpt*(m.oldtm);
//	  }else s[vertices].Ve = Zero;
      face=firstface=ncounter=0;
	  newNorm=zero;
      while (VertexinWhichFace(i,mesh->faces,infaces,&face))
	  { newNorm+=GetNormalizedNormal(&mesh->faces[face].v[0],mesh->verts);
	    if (ncounter==0) firstface=face;
	    ncounter++;face++;
	  }
	// texture comes from first vertex of face
	  if (doTVs) 
	  { s[vertices].tv=mesh->tVerts[mesh->tvFace[firstface].t[0]];
	  }
	  s[vertices].pmtl=mesh->faces[firstface].getMatID();
	  // Martell 4/14/01: Fix for order of ops bug.
	  if ( ncounter>0 )
		  parts->vels[vertices]=Normalize( newNorm/(float)ncounter );
	  else
	  {
		  float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		  parts->vels[vertices]=Normalize( Point3(xtmp,ytmp,ztmp) );
	  }
	  localv=vertices+1;
	  for (j=1;j<lst[count];j++)
	  { parts->points[localv]=mesh->verts[i];
	    parts->vels[localv]=parts->vels[vertices];
	    s[localv].Ve = s[vertices].Ve;
	    if (doTVs) 
		{ s[localv].tv=s[vertices].tv;
		}
	    s[localv].pmtl=s[vertices].pmtl;
	    localv++;
	  }
	  vertices=localv;
	}
	count++;
  }
  delete[] lst;
}

void BlowUpFaces(int faces,Mesh *mesh,Mesh *pmesh,ParticleSys *parts,float thickness,SavePt *s,Matlist m,Point3 *lastv,Face *lastf,int emat,int bmat,int fmat,BOOL doTVs)
{ int i,j;
  Point3 lastpt;

  BOOL thick=(thickness>0.0f);
  for (i=0;i<faces;i++)
  { parts->points[i]=(mesh->verts[mesh->faces[i].v[0]]+mesh->verts[mesh->faces[i].v[1]]+mesh->verts[mesh->faces[i].v[2]])/3.0f;
//	if (s[i].M!=0.0f)
//	{ 
		lastpt=(lastv[lastf[i].v[0]]+lastv[lastf[i].v[1]]+lastv[lastf[i].v[2]])/3.0f;
		s[i].Ve = parts->points[i]*(m.newtm)-lastpt*(m.oldtm);
//	}else s[i].Ve = Zero;
    parts->vels[i]=GetNormalizedNormal(&mesh->faces[i].v[0],mesh->verts);
    if (thick) 
      parts->points[i]-=parts->vels[i]*(thickness*(1+s[i].Vsz))/2.0f;
	pmesh[i].setNumVerts(thick?6:3);
	pmesh[i].setNumFaces(thick?8:1);
	pmesh[i].faces[0]=mesh->faces[i];
	pmesh[i].faces[0].setVerts(0,1,2);
	if (fmat>0) pmesh[i].faces[0].setMatID(fmat-1);
	pmesh[i].verts[0]=(mesh->verts[mesh->faces[i].v[0]]-parts->points[i])*(m.curtm);
	pmesh[i].verts[1]=(mesh->verts[mesh->faces[i].v[1]]-parts->points[i])*(m.curtm);
	pmesh[i].verts[2]=(mesh->verts[mesh->faces[i].v[2]]-parts->points[i])*(m.curtm);
	if (thick)
	{ Point3 tdir=thickness*parts->vels[i];
	  pmesh[i].verts[3]=pmesh[i].verts[0]-tdir*(m.curtm);
	  pmesh[i].verts[4]=pmesh[i].verts[1]-tdir*(m.curtm);
	  pmesh[i].verts[5]=pmesh[i].verts[2]-tdir*(m.curtm);
	  pmesh[i].faces[1]=pmesh[i].faces[0];
	  if (emat>0) pmesh[i].faces[1].setMatID(emat-1);
	  pmesh[i].faces[1].setSmGroup(0);
	  for (j=2;j<8;j++)
	   pmesh[i].faces[j]=pmesh[i].faces[1];
	  pmesh[i].faces[1].setVerts(0,3,4);
	  pmesh[i].faces[1].setEdgeVisFlags(1,1,0);
	  pmesh[i].faces[2].setVerts(0,4,1);
	  pmesh[i].faces[2].setEdgeVisFlags(0,1,1);
	  pmesh[i].faces[3].setVerts(1,4,5);
	  pmesh[i].faces[3].setEdgeVisFlags(1,1,0);
	  pmesh[i].faces[4].setVerts(1,5,2);
	  pmesh[i].faces[4].setEdgeVisFlags(0,1,1);
	  pmesh[i].faces[5].setVerts(2,5,3);
	  pmesh[i].faces[5].setEdgeVisFlags(1,1,0);
	  pmesh[i].faces[6].setVerts(2,3,0);
	  pmesh[i].faces[6].setEdgeVisFlags(0,1,1);
	  pmesh[i].faces[7].setVerts(3,5,4);
	  if (bmat>0) pmesh[i].faces[7].setMatID(bmat-1);
	  else pmesh[i].faces[7].setMatID(mesh->faces[i].getMatID());
	} else pmesh[i].faces[0].setEdgeVisFlags(1,1,1);
    if (doTVs)
	{ int tv0,tv1,tv2;
	  pmesh[i].setNumTVFaces(thick?8:1);
	  pmesh[i].tvFace[0].setTVerts(tv0=mesh->tvFace[i].t[0],tv1=mesh->tvFace[i].t[1],tv2=mesh->tvFace[i].t[2]);
	  if (thick)
	  {	pmesh[i].tvFace[1].setTVerts(tv0,tv0,tv1);
		pmesh[i].tvFace[2].setTVerts(tv0,tv1,tv1);
		pmesh[i].tvFace[3].setTVerts(tv1,tv1,tv2);
		pmesh[i].tvFace[4].setTVerts(tv1,tv2,tv2);
		pmesh[i].tvFace[5].setTVerts(tv2,tv2,tv0);
		pmesh[i].tvFace[6].setTVerts(tv2,tv0,tv0);
		pmesh[i].tvFace[7].setTVerts(tv0,tv2,tv1);
	  }
	}
	else { pmesh[i].setNumTVFaces(0);}
  }
}
void AddToMesh(INode *node,Mesh **mesh,TimeValue t,Matrix3 mtm,Interval &tmpValid,int subtree=0)
{ /*int nc;
  if (subtree)
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	  AddToMesh(node->GetChildNode(j),ntab,subtree,t);*/
  TriObject *triOb=NULL;
  Object *pobj=NULL;
  if ((!node->IsGroupHead())&&((triOb=TriIsUseable(pobj = node->EvalWorldState(t).obj,t))!=NULL))
  { if (*mesh) 
	{ Matrix3 tm2=node->GetObjectTM(t,&tmpValid);
      Mesh *newmesh=new Mesh;
	  CombineMeshes((*newmesh),(**mesh), triOb->GetMesh(), NULL, &tm2,-1);
	  (*mesh)->DeleteThis();
	  (*mesh)=newmesh;
	}
    else
	{ (*mesh)=new Mesh;(*mesh)->DeepCopy(&triOb->GetMesh(),GEOM_CHANNEL | TOPO_CHANNEL | TEXMAP_CHANNEL |	MTL_CHANNEL | DISP_ATTRIB_CHANNEL | TM_CHANNEL);
	  Matrix3 tm2=node->GetObjectTM(t,&tmpValid);
	  for (int i=0;i<triOb->GetMesh().getNumVerts();i++)
		(*mesh)->verts[i]=(*mesh)->verts[i]*tm2;
	}
  }
  if ((triOb) &&(triOb!=pobj)) triOb->DeleteThis();
}

void GetGroupMeshPtr(INode *node,Mesh **mesh,TimeValue t,Matrix3 curtm,Interval &tmpValid,int subtree=0)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) GetGroupMeshPtr(nxtnode,mesh,t,curtm,tmpValid,subtree);
	  else if ((subtree)||(nxtnode->IsGroupMember())) AddToMesh(nxtnode,mesh,t,curtm,tmpValid,subtree);
	}
}

void PArrayParticle::GetInfoFromObject(float thick,int *c,INode *distnode,TimeValue t,TimeValue lastt)
{ Object *pobj=NULL,*oldpobj=NULL;	  
  TriObject *triOb=NULL,*oldtriOb=NULL;
  Matlist m;
  int oldv,oldf,pnum=0;
  float NAngle,Vsz;
  Interval tmpValid=FOREVER;
  Matrix3 oldtm=distnode->GetObjTMAfterWSM(lastt,&tmpValid);
  tmpValid=FOREVER; 
  Matrix3 curtm=distnode->GetObjTMAfterWSM(t,&tmpValid);
  m.newtm=curtm;
  m.oldtm=oldtm;
  m.curtm=curtm;
  m.curtm.NoTrans();
  Mesh *oldmesh=NULL,*curmesh=NULL;
  BOOL tempmesh=FALSE;
  if (distnode->IsGroupHead())
  { tempmesh=TRUE;
	GetGroupMeshPtr(distnode,&oldmesh,lastt,oldtm,tmpValid);
	if (oldmesh) 
	{ Matrix3 itm=Inverse(oldtm);
	  for (int j=0;j<oldmesh->getNumVerts();j++) oldmesh->verts[j]=oldmesh->verts[j]*itm;
	}
  }
  else
  { if ((oldtriOb=TriIsUseable(oldpobj=distnode->EvalWorldState(t).obj,t))==NULL) return;
    oldmesh=&oldtriOb->GetMesh();
  }
  int ov=oldmesh->getNumVerts(),of=oldmesh->getNumFaces();
  Point3 *oldvs=new Point3[ov];
  Face *oldvf=new Face[of];
  for (int cnt=0;cnt<ov;cnt++)
  	oldvs[cnt]=oldmesh->verts[cnt];
  for (cnt=0;cnt<of;cnt++)
  	oldvf[cnt]=oldmesh->faces[cnt];	
  if (distnode->IsGroupHead())
  { tempmesh=TRUE;
	GetGroupMeshPtr(distnode,&curmesh,t,curtm,tmpValid);
	if (curmesh) 
	{ Matrix3 itm=Inverse(curtm);
	  for (int j=0;j<curmesh->getNumVerts();j++) curmesh->verts[j]=curmesh->verts[j]*itm;
	}
  }
  else
  { if ((triOb=TriIsUseable(pobj=distnode->EvalWorldState(t).obj,t))==NULL)
	{ if (oldtriOb!=oldpobj) oldtriOb->DeleteThis(); return;}
    curmesh=&triOb->GetMesh();
  }
  oldv=curmesh->getNumVerts();
  oldf=curmesh->getNumFaces();
  oldtvnum=curmesh->getNumTVerts();	
  doTVs=doTVs && (oldtvnum>0);
  int frag,emat,bmat,fmat;
  float Ie=0.0f,Em=0.0f,Vm=0.0f;
	pblock->GetValue(PB_EMAT,0,emat,FOREVER);
	pblock->GetValue(PB_BMAT,0,bmat,FOREVER);
	pblock->GetValue(PB_FMAT,0,fmat,FOREVER);
  pblock->GetValue(PB_EMITVINFL,t,Ie,FOREVER);
  pblock->GetValue(PB_EMITVMULT,t,Em,FOREVER);
  pblock->GetValue(PB_EMITVMULTVAR,t,Vm,FOREVER);
	pblock->GetValue(PB_FRAGMETHOD,0,frag,FOREVER); 
	pblock->GetValue(PB_SIZEVAR,t,Vsz,FOREVER);
	if (frag==FRAGFACE)
	{ *c=oldf;
      if (sdata){delete[] sdata;sdata=NULL;} 
	  sdata=new SavePt[oldf];
	  assert(sdata);
	  if (thick>0.0f)
	  for (int i=0;i<oldf;i++)
	  {	sdata[i].Vsz=RNDSign()*RND01()*Vsz;
		sdata[i].M=(RND0x(99)<Ie?Em*(1+RND11()*Vm):0); }
	  parts.SetCount(oldf,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	  if (pmesh) { delete[] pmesh; pmesh=NULL; }
	  if (oldf) pmesh=new Mesh[oldf];
	  else pmesh = new Mesh[1];
	  BlowUpFaces(oldf,&(*curmesh),pmesh,&parts,thick,&sdata[0],m,oldvs,oldvf,emat,bmat,fmat,doTVs);
    }  //FRAGRND
	else 
	{ int *toplist,*nextlist;
      toplist=new int[oldf];
      nextlist=new int[oldf];
      assert((toplist!=NULL)&&(nextlist!=NULL));
      pblock->GetValue(PB_SMOOTHANG,0,NAngle,FOREVER); 
	  Point3 *norm;
	  norm=new Point3[oldf];
	  assert(norm!=NULL);
	  FindAllNormals(oldf,norm,curmesh->verts,curmesh->faces);
	  for (int i=0;i<oldf;i++) nextlist[i]=-2;
	  if (frag==FRAGRND)
	  { int fcnt;
		pblock->GetValue(PB_FRAGCOUNT,0,fcnt,FOREVER); 
		int numedges;
		Edge* el = curmesh->MakeEdgeList(&numedges,0);
		Tab<Edge> te;te.SetCount(numedges);
		for (int tx=0;tx<numedges;tx++)
		{ te[tx].v[0]=el[tx].v[0];te[tx].v[1]=el[tx].v[1];
		  te[tx].f[0]=el[tx].f[0];te[tx].f[1]=el[tx].f[1];
		} delete[] el;
		*c=FillInGroups(oldf,fcnt,toplist,nextlist,curmesh->faces,curmesh->verts,te); 
		te.Resize(0);
	  }
	  else
	  {	*c=FillInSmoothing(oldf,NAngle,norm,toplist,nextlist,curmesh->faces,curmesh->verts);
	  }
	  if (pmesh) { delete[] pmesh; pmesh=NULL; }
	  if (*c) pmesh=new Mesh[*c];
	  else pmesh = new Mesh[1];
	  parts.SetCount(*c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
      if (sdata){delete[] sdata;sdata=NULL;} sdata=new SavePt[parts.Count()];
	  if (thick>0.0f) 
	  for (int i=0;i<*c;i++)
	  {	sdata[i].Vsz=RNDSign()*RND01()*Vsz;
	    sdata[i].M=(RND0x(99)<Ie?Em*(1+RND11()*Vm):0);}
	  MakeIntoMesh(*c,oldf,norm,toplist,nextlist,thick,&(*curmesh),pmesh,&parts,&sdata[0],oldvs,oldvf,m,emat,bmat,fmat,doTVs);
	  if (norm) delete[] norm;
	  if (toplist) delete[] toplist;
	  if (nextlist) delete[] nextlist;
	}
	  if (pmesh)
	  { pmesh[0].setNumTVerts(oldtvnum);
		for (int tmp=0;tmp<oldtvnum;tmp++)
		{ pmesh[0].tVerts[tmp]=curmesh->tVerts[tmp];}
		curtm.NoTrans();
		Box3 tbox;
		for (int tc=0;tc<*c;tc++)
		{ tbox=pmesh[tc].getBoundingBox();
		  MakeBBpts(sdata[tc].wbb,tbox,curtm);
		}
	  }
	  if (tempmesh) 
	  { oldmesh->DeleteThis();curmesh->DeleteThis();}
	  if (oldvs) delete[] oldvs; 
	  if (oldvf) delete[] oldvf;
	  if (oldtriOb!=oldpobj) oldtriOb->DeleteThis();
	  if (triOb!=pobj) triOb->DeleteThis();
	  if (storefrag) delete[] storefrag;
   storefrag=new CacheDataType[*c];
  for (int ti=0;ti<*c;ti++)
  { storefrag[ti].M=sdata[ti].M;
	storefrag[ti].Ve = sdata[ti].Ve;
	storefrag[ti].Vsz=sdata[ti].Vsz;
	for (int wi=0;wi<8;wi++)
	  storefrag[ti].wbb[wi]=sdata[ti].wbb[wi];
	storefrag[ti].vel=parts.vels[ti];
	storefrag[ti].pos=parts.points[ti];
  }
	storernd=rand();
};
BOOL PArrayParticle::GenerateNotGeom(TimeValue t,TimeValue lastt,int c,int counter,INode *distnode,int type,Matrix3 tm,Matrix3 lasttm)
{	Object *pobj,*oldpobj;	  
	TriObject *triOb=NULL,*oldtriOb=NULL;
	Matlist m;
	int oldv,oldf,pnum=0,cnt;
	Point3 *oldvs=NULL;
	Face *oldvf=NULL;
	BOOL isok=FALSE;
//	srand(emitrnd);
	if ((oldtriOb=TriIsUseable(oldpobj=distnode->EvalWorldState(lastt).obj,lastt))==NULL) 
	  return isok;
	srand(emitrnd);
	int ov=oldtriOb->GetMesh().getNumVerts(),of=oldtriOb->GetMesh().getNumFaces();
	if ((ov==0)||(of==0)) 
		goto bombout;
	oldvs=new Point3[ov];
	oldvf=new Face[of];
	for (cnt=0;cnt<ov;cnt++)
  		oldvs[cnt]=oldtriOb->GetMesh().verts[cnt];
	for (cnt=0;cnt<of;cnt++)
  		oldvf[cnt]=oldtriOb->GetMesh().faces[cnt];	
	if ((triOb=TriIsUseable(pobj=distnode->EvalWorldState(t).obj,t))==NULL)
		goto bombout;
	oldv=triOb->GetMesh().getNumVerts();
	oldf=triOb->GetMesh().getNumFaces();
	if ((oldv==0)||(oldf==0)) goto bombout;
	BOOL doall;  pblock->GetValue(PB_USESELECTED,t,doall,FOREVER);
	doall=!doall;
	if (!doall)
	{	if (! (((type==VERTS)&&(triOb->GetMesh().vertSel.NumberSet()>0))||((type==EDGES)&&(triOb->GetMesh().edgeSel.NumberSet()>0))  )   )
		{	if (triOb->GetMesh().faceSel.NumberSet()==0) goto bombout;
		if (type==VERTS)
			for (int i=0;i<oldf;i++)
			{	if (triOb->GetMesh().faceSel[i])
				for (int j=0;j<3;j++) triOb->GetMesh().vertSel.Set(triOb->GetMesh().faces[i].v[j]);
			}
		}
	}
  oldtvnum=triOb->GetMesh().getNumTVerts();	
  doTVs=doTVs && (oldtvnum>0);
  m.oldtm=lasttm;
  m.newtm=distnode->GetObjTMAfterWSM(t);
  float Em,Vm,Ie;
  pblock->GetValue(PB_EMITVINFL,t,Ie,FOREVER);
  pblock->GetValue(PB_EMITVMULT,t,Em,FOREVER);
  pblock->GetValue(PB_EMITVMULTVAR,t,Vm,FOREVER);
  for (cnt=counter;cnt<counter+c;cnt++)
	sdata[cnt].M=(sdata[cnt].L<Ie?Em*(1+sdata[cnt].M*Vm):0);
  if (type==FACEC) FillInFaces(oldf,counter,&(triOb->GetMesh()),c,&parts,&sdata[0],oldvs,oldvf,m,doTVs,of-1,doall);
  else if (type==EDGES) FillByEdges(oldf,c,counter,&(triOb->GetMesh()),&parts,&sdata[0],oldvs,m,doTVs,ov-1,doall);
  else if (type==UNIFORM) FillInUniform(oldf,c,counter,&(triOb->GetMesh()),&parts,&sdata[0],oldvs,oldvf,m,doTVs,of-1,doall);
  else if (type==VERTS) FillInVertex(oldv,oldf,counter,&(triOb->GetMesh()),c,&parts,&sdata[0],oldvs,m,doTVs,ov-1,doall);
  else 
  { int emits,seed;
	pblock->GetValue(PB_EMITTERCOUNT,0,emits,FOREVER); 
	pblock->GetValue(PB_RNDSEED,0,seed,FOREVER); 
	srand(seed);
	FillByFixedPts(oldf,emits,c,counter,emitrnd,&(triOb->GetMesh()),&parts,&sdata[0],oldvs,oldvf,m,doTVs,of-1,doall);
  }
  emitrnd=rand();
  isok=TRUE;
bombout:
  if (oldvs) delete[] oldvs; 
  if (oldvf) delete[] oldvf;
  if ((oldtriOb)&&(oldtriOb!=oldpobj)) oldtriOb->DeleteThis();
  if ((triOb)&&(triOb!=pobj)) triOb->DeleteThis();
  return isok;
}

BOOL PArrayParticle::ComputeParticleStart(TimeValue t0,TimeValue lastt,INode *node,int c)
	{
	int seed,type,anioff,tani;
	TimeValue anifr;
	Point3 vel;
	float thick;
	if (c > gCountUpperLimit) c = gCountUpperLimit;
    pblock->GetValue(PB_DISTRIBUTION,0,type,FOREVER);
    pblock->GetValue(PB_FRAGTHICKNESS,0,thick,FOREVER);
	if (!fromgeom) thick=0.0f;
	pblock->GetValue(PB_RNDSEED,t0,seed,FOREVER);
    pblock->GetValue(PB_OFFSETAMOUNT,t0,anifr,FOREVER);
    pblock->GetValue(PB_ANIMATIONOFFSET,t0,anioff,FOREVER);
	srand(seed);					
	int oneframe=GetTicksPerFrame(),frag;
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	if (!fromgeom)
	{ if (pmesh) { delete[] pmesh; pmesh=NULL; }
	  if (c) pmesh=new Mesh[c];
	   else pmesh=new Mesh[1];
	  parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	  int pcount=parts.Count();
      if (sdata){delete[] sdata;sdata=NULL;} if (pcount) sdata=new SavePt[pcount];
	if ((pcount<c)||(c>0 && (!sdata)))
	{   parts.FreeAll();if (sdata) delete sdata;sdata=NULL;	maincount=0;
		BOOL playing=GetCOREInterface()->IsAnimPlaying();
		if (playing) GetCOREInterface()->EndAnimPlayback();
	    TSTR name;name=(cnode ? cnode->GetName() : TSTR(GetString(IDS_RB_PARRAY)));
		TSTR buf; buf=TSTR(GetString(IDS_OFM_PART));
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			GetString(IDS_OFM_ERROR),_T("%s: \n\n%s\n"),buf,name);
	    return (0);
	  }
	} 		
	else if (!pmesh) 
	{ GetInfoFromObject(thick,&c,distnode,t0,t0-lastt);
		srand(storernd);
	}
	else
	{ c=maincount;
	  parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
      if (sdata){delete[] sdata;sdata=NULL;} sdata=new SavePt[parts.Count()];
	  for (int ti=0;ti<maincount;ti++)
	  { sdata[ti].M=storefrag[ti].M;
		sdata[ti].Ve = storefrag[ti].Ve;
		sdata[ti].Vsz=storefrag[ti].Vsz;
		for (int wi=0;wi<8;wi++)
		  sdata[ti].wbb[wi]=storefrag[ti].wbb[wi];
		parts.vels[ti]=storefrag[ti].vel;
		parts.points[ti]=storefrag[ti].pos;
	  }
		srand(storernd);
	}
	float tmp;Point3 tmppt;
	for (int i=0; i<parts.Count(); i++) {
		parts.ages[i] = -1;
		sdata[i].themtl=0;
		sdata[i].gennum=0;
  		sdata[i].L=RND0x(99);sdata[i].DL=-1;sdata[i].pvar=RND11();
		tmp=RND01();sdata[i].Fo=(fromgeom?0:tmp);
		tani=RND0x(anifr/oneframe);
		sdata[i].showframe=(anioff==2?tani*oneframe:0);
		sdata[i].V.x=RND11();
		sdata[i].V.y=RND11();
		sdata[i].V.z=RND11();
		sdata[i].Ts=0.0f;
		sdata[i].Ts0=RND11();
		sdata[i].LamTs=RND11();
		sdata[i].A=RND11();
		sdata[i].LamA=RND11();
		sdata[i].Inf=RND11();
		sdata[i].frommesh=0;
//		tmppt=Point3(RND11(),RND11(),RND11());
		if (!fromgeom)
		{ sdata[i].M=RND11();
		  sdata[i].Ve = Point3(0.0f,0.0f,0.0f);
//		  if (frag==INSTGEOM) parts.vels[i]=tmppt;
		}
		sdata[i].To=RND11();
		tmp=RND11();if (thick==0.0f) sdata[i].Vsz=tmp;
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		sdata[i].W=Point3(xtmp,ytmp,ztmp);
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND11(); ytmp=RND11(); xtmp=RND11();
		sdata[i].RV=Point3(xtmp,ytmp,ztmp);
		tmp=RND11();sdata[i].Dis=tmp;
		parts.radius[i]=thick;
		parts.tension[i]=RND11();
		sdata[i].Mltvar=RND11();
		sdata[i].SpVar=RND0x(99);
		}
	tvalid = t0-1;
	valid  = TRUE;
	lastrnd=rand();
	emitrnd=lastrnd;
	return (1);
	}

#define VEL_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))
#define VAR_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))

void PArrayParticle::BirthParticle(INode *node,TimeValue bt,int num,VelDir2 *ptvel2,VelDir ptvel,Matrix3 tmlast)
{
	Point3 pos, vel;
//	Matrix3 btm = node->GetObjTMBeforeWSM(bt);
	Matrix3 tm = node->GetObjTMAfterWSM(bt);
//	Matrix3 tm = node->GetObjectTM(bt);
//	tm.SetRow(3,atm.GetRow(3));

	int MotionOffset,EmitOffset;
	pblock->GetValue(PB_SUBFRAMEMOVE,bt,MotionOffset,FOREVER);
	pblock->GetValue(PB_SUBFRAMETIME,bt,EmitOffset,FOREVER);

	int RotSampleOn;
	pblock->GetValue(PB_SUBFRAMEROT,bt,RotSampleOn,FOREVER);

	parts.tension[num] = ptvel.bstr*(1.0f + parts.tension[num]*ptvel.bstrvar);
	sdata[num].L = ptvel.Life+(int)(sdata[num].Inf*ptvel.Vl);
	sdata[num].Vsz *= ptvel.VSz;
	sdata[num].Ts0 = (1.0f + sdata[num].Ts0*ptvel.VSpin)/TWOPI;
	sdata[num].Ts = (float)ptvel.Spin*sdata[num].Ts0;
	sdata[num].persist = (TimeValue)(ptvel2->persist*(1.0f+sdata[num].pvar*ptvel2->pvar));

	if (fromgeom) 
		sdata[num].LamTs = 0.0f;
	else
		sdata[num].LamTs = ptvel.Phase*(1.0f + sdata[num].LamTs*ptvel.VPhase);

	sdata[num].A = ptvel.ToAmp*(1.0f + sdata[num].A*ptvel.VToAmp);
	sdata[num].LamA = ptvel.ToPhase*(1.0f + sdata[num].LamA*ptvel.VToPhase);
	sdata[num].To = ptvel.ToPeriod*(1 + sdata[num].To*ptvel.VToPeriod);

	if (ptvel2->axisentered==2)
	{	sdata[num].W = Normalize(ptvel2->Axis);
		if (ptvel2->axisvar>0.0f)
			VectorVar(&sdata[num].W,ptvel2->axisvar,PI);
	}
	else 
		sdata[num].W = Normalize(sdata[num].W);

	parts.ages[num] = 0;

	Matrix3 OffRotTm;
	if (RotSampleOn) 
		MakeInterpRotXform(tmlast,tm,(1.0f - sdata[num].Fo),OffRotTm);
	else 
	{
		OffRotTm=tm;
		OffRotTm.NoScale();
	}

	vel = VectorTransform(OffRotTm,parts.vels[num]);

//	vel=VectorTransform(tm,parts.vels[num]);

	if (ptvel2->div>0.0f) 
		vel = CalcSpread(ptvel2->div,vel); // diverergence!

	vel.x*=ptvel.Speed*(1+sdata[num].V.x*ptvel.VSpeed);
	vel.y*=ptvel.Speed*(1+sdata[num].V.y*ptvel.VSpeed);
	vel.z*=ptvel.Speed*(1+sdata[num].V.z*ptvel.VSpeed);

	pos = parts.points[num]*tm;

	if (MotionOffset) 
		pos -= (sdata[num].Ve)*(sdata[num].Fo);
//		pos += (sdata[num].Ve)*(1.0f-sdata[num].Fo);

	if (EmitOffset) 
		pos += (sdata[num].Fo)*vel;
//		pos += (sdata[num].Fo)*vel;

	if (sdata[num].M != 0.0f)
	{
		pos += sdata[num].Ve*(sdata[num].M)*(sdata[num].Fo); //new
		vel += sdata[num].Ve*sdata[num].M;
	}

	parts[num] = pos; 

	vel /= (float)GetTicksPerFrame();

	parts.vels[num] = vel;
	sdata[num].start = (sdata[num].V = parts[num]);
}

void PArrayParticle::DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emits,int &oldcount)
{ if (!emits) spmult--;
  int newcount=(oldcount+spmult); 
  srand(lastrnd);
  if ((!emits)&&(spmult))
  { parts.SetCount(newcount,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	SavePt *tmp=sdata;
	sdata=new SavePt[newcount];
	if (tmp) {memcpy(sdata,tmp,sizeof(SavePt)*oldcount); delete[] tmp;
	}
  }
  int Lcnt=llist.Count();
  int baselife=(Lcnt==0?deftime:(sdata[j].gennum<Lcnt?llist[sdata[j].gennum]:llist[Lcnt-1]))*GetTicksPerFrame();
  if (!emits)
  { sdata[j].gennum++;
	parts.ages[j]=0;}
  Point3 holdv=(emits?-parts.vels[j]:parts.vels[j]);
  for (int i=oldcount;i<newcount;i++)
  { parts.points[i]=parts.points[j];
    parts.vels[i]=holdv;
	parts.radius[i]=parts.radius[j];
	parts.tension[i]=parts.tension[j];
	parts.ages[i]=0;
	memcpy(&sdata[i],&sdata[j],sizeof(SavePt));
 	if (emits) sdata[i].gennum++;
 	if (sdata[j].gennum==1) sdata[i].frommesh=j;
    parts.vels[i]=DoSpawnVars(spvars,parts.vels[j],holdv,&parts.radius[i],&sdata[i].W);
	sdata[i].L=baselife+(int)RND11()*lvar;
	sdata[i].Mltvar=RND11();
	sdata[i].SpVar=RND0x(99);
	sdata[i].DL=-1;
	sdata[i].Vsz=parts.radius[i];
//	parts.radius[i]=basesize*sdata[i].Vsz;
 }
  if (!emits)
  { parts.vels[j]=DoSpawnVars(spvars,holdv,holdv,&parts.radius[j],&sdata[j].W);
 	if (sdata[j].gennum==1) sdata[j].frommesh=j;
    sdata[j].L=baselife+(int)RND11()*lvar;
	sdata[j].Mltvar=RND11();
	sdata[j].SpVar=RND0x(99);
	sdata[j].DL=-1;
//	parts.radius[j]=basesize*sdata[j].Vsz;
  }
  lastrnd=rand();
  oldcount=newcount;
}

void PArrayParticle::MovePart(int j,TimeValue dt,BOOL fullframe,int tpf)
{ 	float curdt=(float)dt;
	parts[j] = sdata[j].V+parts.vels[j] * (float)dt;
	// add transverse oscillation
	if (fullframe) sdata[j].V=parts[j];
	Point3 DotPt=(FGT0(parts.vels[j]*(float)tpf)?parts.vels[j]:sdata[j].W);
	DotPt=sdata[j].RV^DotPt;				// Cross Product 
	DotPt=Normalize(DotPt);	       
	float oscil=(sdata[j].To>0.0f?parts.ages[j]*TWOPI/sdata[j].To:0.0f);
	oscil=sdata[j].A*(float)sin(oscil+sdata[j].LamA);
	parts[j] +=DotPt*oscil;
}

void PArrayParticle::UpdateParticles(TimeValue t,INode *node)
{
	TimeValue t0,dt,t2,grow,fade;
	int i, j, c, birth,counter,tpf=GetTicksPerFrame(),anioff;
	float FperT;
	VelDir ptvel;
	VelDir2 ptvel2;
    Matrix3 tm,lasttm;
//	Matrix3 nxttm; 
	int isrend = TestAFlag(A_RENDER),bmethod,onscreen,oneframe;
	BOOL custmtl;
	// variable for new collision scheme (Bayboro 2/5/01)
	CollisionCollection cc;

	// fix for "Abort by Escape key" problem (Bayboro 1/31/01)
	if (cancelled && (tvalid == t)) return;
	if (cancelled && valid) tvalid = TIME_NegInfinity;
	cancelled = FALSE;	

	// initialization for new collision scheme (Bayboro 2/5/01)
	cc.Init(cobjs);

// The size of steps we take to integrate will be frame size steps.

	oneframe = GetTicksPerFrame();
    if (stepSize!=oneframe) 
	{	stepSize = oneframe;
		valid = FALSE;
	}
	FperT = GetFrameRate()/(float)TIME_TICKSPERSEC;

    pblock->GetValue(PB_ANIMATIONOFFSET,t,anioff,FOREVER);
	pblock->GetValue(PB_EMITSTART,t,t0,FOREVER);
	pblock->GetValue(PB_SIZE,t,parts.size,FOREVER);	
	pblock->GetValue(PB_BIRTHMETHOD,0,bmethod,FOREVER);
	pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
	int subtree,frag;
	pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	pblock->GetValue(PB_EMITSTOP,0,t2,FOREVER);
	if (bmethod)
		pblock->GetValue(PB_PTOTALNUMBER,0,c,FOREVER);
	if ((t < t0)||(!distnode)) 
	{
		// Before the start time, nothing is happening
		parts.FreeAll();
		if (sdata) 
		{
			delete[] sdata;
			sdata=NULL;
		}
		if ((!distnode)||(!valid))
		{ 
			if (pmesh) 
			{
				delete[] pmesh; 
				pmesh=NULL;
			}
		}
		ResetSystem(t);
		doTVs = FALSE;
		return;
	}
	int pkind,vkind;
	float dpercent;
	pblock->GetValue(PB_DISPLAYPORTION,0,dpercent,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,pkind,FOREVER);
    pblock->GetValue(PB_DISTRIBUTION,0,vkind,FOREVER);
	fromgeom = (frag==BYGEOM);

	if (!valid || t<tvalid || tvalid<t0) 
	{	
		// Set the particle system to the initial condition
	    TimeValue cincr;	
    	doTVs = custmtl;
		if ((!fromgeom)&&(!bmethod))
		{ 
			c=0;
			for (TimeValue ti=t0;ti<=t2;ti+=oneframe)
			{ 
				pblock->GetValue(PB_PBIRTHRATE,ti,cincr,FOREVER);
				if (cincr<0) cincr=0;
					c += cincr;
			}
		}
		if (!isrend) 
			c = (int)(dpercent*(float)c + FTOIEPS);
		if ((pmesh)&&(!valid))
		{ 
			delete[] pmesh; 
			pmesh=NULL;
		}
		if (!ComputeParticleStart(t0,stepSize,distnode,c))
		{ 
			if (pmesh) 
			{
				delete[] pmesh; 
				pmesh=NULL;
			}
			ResetSystem(t);
			doTVs = FALSE;
			return;
		}
		dispt = t - 1;
		maincount = parts.Count();
		ResetSystem(t,FALSE);
//		if (origmtl==NULL) origmtl=node->GetMtl();
		lastTMTime = TIME_NegInfinity;
	}
	int total;
	total = maincount;
	valid = TRUE;
	int stype,maxgens,spmultb;
	float spawnbvar;
    pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
    pblock->GetValue(PB_SPAWNGENS,0,maxgens,FOREVER);
    pblock->GetValue(PB_SPAWNCOUNT,0,spmultb,FOREVER);
	valid = TRUE;
    pblock->GetValue(PB_GROWTIME,0,grow,FOREVER);
    pblock->GetValue(PB_FADETIME,0,fade,FOREVER);
    pblock->GetValue(PB_SPINAXISTYPE,0,ptvel2.axisentered,FOREVER);

	SpawnVars spawnvars;
	spawnvars.axisentered = ptvel2.axisentered;

	TimeValue dis;
    pblock->GetValue(PB_DISPUNTIL,0,dis,FOREVER);
//	if (Length(ptvel2.Dir)==0.0f) ptvel2.Dir.x=0.001f;

	if (t2<t0) 
		t2 = t0;
//	TimeValue fstep=oneframe;t2+=fstep;
	TimeValue createover;
	createover = t2 - t0 + oneframe;
	counter = (isrend?rcounter:vcounter);
	float frate,grate;
	if (frag==BYGEOM) 
	{
		fade=0;
		grow=0;
	}
	frate = (fade>0.0f?(1-M)/fade:0.0f);
	grate = (grow>0.0f?(1-M)/grow:0.0f);
	BOOL fullframe;
	float basesize,startsize;
	if (!isrend)
	{ 
		int offby = t%oneframe;
		if (offby!=0) 
			t -= offby;
	}
    pblock->GetValue(PB_SPAWNSCALESIGN,0,spawnvars.scsign,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDSIGN,0,spawnvars.spsign,FOREVER);
    pblock->GetValue(PB_SPAWNINHERITV,0,spawnvars.invel,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDFIXED,0,spawnvars.spconst,FOREVER);
    pblock->GetValue(PB_SPAWNSCALEFIXED,0,spawnvars.scconst,FOREVER);
	int count,sper,spmult;
	float smper;BOOL first=(tvalid<t0);
	while ((tvalid < t)&&(tvalid<=dis))
	{	
		int born = 0;		
		if (first) 
			tvalid = t0;
		TimeValue atvalid = abs(tvalid);
			// Compute our step size
		if (tvalid%stepSize !=0) 
		{ 
			dt = stepSize - atvalid%stepSize;
		} 
		else 
		{ 
			dt = stepSize;	
		}
		if (tvalid + dt > t) 
		{
			dt = t-tvalid;
		}

  		// Increment time
		if (!first) 
			tvalid += dt;
		if (tvalid>dis)
		{ 
			for (j=0; j<parts.Count(); j++)
			{ 
				parts.ages[j] = -1;  
			}
			tvalid=t;
			continue;
		}
	    pblock->GetValue(PB_PASPAWNDIEAFTER,tvalid,ptvel2.persist,FOREVER);
	    pblock->GetValue(PB_PASPAWNDIEAFTERVAR,tvalid,ptvel2.pvar,FOREVER);
		pblock->GetValue(PB_SIZE,tvalid,ptvel.Size,FOREVER);
		pblock->GetValue(PB_SIZEVAR,tvalid,ptvel.VSz,FOREVER);
	    pblock->GetValue(PB_SPEED,tvalid,ptvel.Speed,FOREVER);
	    pblock->GetValue(PB_SPEEDVAR,tvalid,ptvel.VSpeed,FOREVER);
        pblock->GetValue(PB_ANGLEDIV,tvalid,ptvel2.div,FOREVER);
        pblock->GetValue(PB_METATENSION,tvalid,ptvel.bstr,FOREVER);
	    pblock->GetValue(PB_METATENSIONVAR,tvalid,ptvel.bstrvar,FOREVER);
		pblock->GetValue(PB_LIFE,tvalid,ptvel.Life,FOREVER);
		pblock->GetValue(PB_LIFEVAR,tvalid,ptvel.Vl,FOREVER);
	    pblock->GetValue(PB_SPINTIME,tvalid,ptvel.Spin,FOREVER);
		pblock->GetValue(PB_SPINTIMEVAR,tvalid,ptvel.VSpin,FOREVER);
		pblock->GetValue(PB_SPINPHASE,tvalid,ptvel.Phase,FOREVER);
		pblock->GetValue(PB_SPINPHASEVAR,tvalid,ptvel.VPhase,FOREVER);
		pblock->GetValue(PB_SPINAXISX,tvalid,ptvel2.Axis.x,FOREVER);
		pblock->GetValue(PB_SPINAXISY,tvalid,ptvel2.Axis.y,FOREVER);
		pblock->GetValue(PB_SPINAXISZ,tvalid,ptvel2.Axis.z,FOREVER);
		if (Length(ptvel2.Axis)==0.0f) 
			ptvel2.Axis.x = 0.001f;
		pblock->GetValue(PB_SPINAXISVAR,tvalid,ptvel2.axisvar,FOREVER);
		pblock->GetValue(PB_BUBLAMP,tvalid,ptvel.ToAmp,FOREVER);
		pblock->GetValue(PB_BUBLAMPVAR,tvalid,ptvel.VToAmp,FOREVER);
		pblock->GetValue(PB_BUBLPHAS,tvalid,ptvel.ToPhase,FOREVER);
		pblock->GetValue(PB_BUBLPHASVAR,tvalid,ptvel.VToPhase,FOREVER);
		pblock->GetValue(PB_BUBLPER,tvalid,ptvel.ToPeriod,FOREVER);
		pblock->GetValue(PB_BUBLPERVAR,tvalid,ptvel.VToPeriod,FOREVER);
		pblock->GetValue(PB_SPAWNPERCENT,tvalid,sper,FOREVER);	
		pblock->GetValue(PB_SPAWNMULTVAR,tvalid,smper,FOREVER);
		spawnbvar = smper*spmultb;
		pblock->GetValue(PB_SPAWNDIRCHAOS,tvalid,spawnvars.dirchaos,FOREVER);
		pblock->GetValue(PB_SPAWNSPEEDCHAOS,tvalid,spawnvars.spchaos,FOREVER);
		spawnvars.spchaos /= 100.0f;
		pblock->GetValue(PB_SPAWNSCALECHAOS,tvalid,spawnvars.scchaos,FOREVER);
		spawnvars.scchaos = spawnvars.scchaos/100.0f;
		spawnvars.Axis = ptvel2.Axis;
		spawnvars.axisvar = ptvel2.axisvar;
		if (llist.Count()==0) 
			deftime = ptvel.Life/oneframe;
		if (frag==BYGEOM) 
			ptvel.Size = 1.0f;
		basesize = M*ptvel.Size;
		startsize = ((grow==0)?ptvel.Size:basesize);
//		startsize=ptvel.Size;
		// Compute the number of particles that should be born!
		birth = 0;
		fullframe = (tvalid%tpf==0);
		if (fullframe)
		{ 
			if (fromgeom)
			{ 
				if (tvalid>=t0) 
					birth = total - counter;	
			}
			else
			{ 
				if (bmethod)
				{ 
					int tdelta;
					if (tvalid>=t2) 
						birth = total - counter;
					else
					{ 
						tdelta = int((float)total*(tvalid - t0 + oneframe)/createover);
						birth=tdelta-counter;
					}
				}
				else if (tvalid<=t2)
				{ 
					pblock->GetValue(PB_PBIRTHRATE,tvalid,total,FOREVER);
					if (!isrend) 
						total = (int)(dpercent*(float)total + FTOIEPS);
					birth = total;
					if (birth+counter>maincount) 
						birth = maincount - counter;
				}	  
			} 
		}
		int pc = parts.Count(),newsp = 0, oldcnt = pc;
		if ((fullframe)&&(stype==EMIT))
		{ 
			for (j=0;j<maincount;j++)
			{ 
				if ((!parts.Alive(j))||(parts.ages[j]+dt >= sdata[j].L)) 
					continue;
				if ((sdata[j].gennum==0)&&(sdata[j].SpVar<sper))
					newsp += (int)(sdata[j].Mltvar*spawnbvar) + spmultb;
			}
			if (newsp>0)
			{ 
				int newcount = pc + newsp;
				parts.SetCount(newcount,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
				SavePt *tmp = sdata;
				sdata = new SavePt[newcount];
				if (tmp) 
				{
					memcpy(sdata,tmp,sizeof(SavePt)*pc); delete[] tmp;
				}
			}
		}
		// First increment age and kill off old particles
		for (j=0;j<pc;j++)
		{	
			if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
			{ 
				valid = FALSE;
				tvalid = t;
				cancelled = TRUE;
				parts.FreeAll();
				return;
			}
			if (!parts.Alive(j)) 
				continue;
			parts.ages[j] += dt;
//			startsize=ptvel.Size;
			if (parts.ages[j] >= sdata[j].L) 
			{  
				if ((stype!=ONDEATH)||(sdata[j].gennum>=maxgens)||(sdata[j].SpVar>=sper)) 
					parts.ages[j] = -1;	
				else if (fullframe)
				{
					spmult = (int)(sdata[j].Mltvar*spawnbvar) + spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE,oldcnt);
					else 
						parts.ages[j] = -1;
				}
			} 
			else if (sdata[j].DL>-1)
			{ 
				sdata[j].DL += dt;
				if (sdata[j].DL>sdata[j].persist) 
					parts.ages[j] = -1;
			}
			if (parts.ages[j]>-1)
			{ 
				if (fullframe &&((stype==EMIT)&&(sdata[j].gennum==0)&&(sdata[j].SpVar<sper)))
				{  
					spmult = (int)(sdata[j].Mltvar*spawnbvar) + spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,TRUE,oldcnt);
				}
				if ((stype<2)||(maxgens==0))
		 			parts.radius[j] = FigureOutSize(parts.ages[j],ptvel.Size,grow,fade,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
				else if (!fromgeom) 
				{ 
					if (sdata[j].gennum==0)
						parts.radius[j] = FigureOutSize(parts.ages[j],ptvel.Size,grow,0,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
					else if (sdata[j].gennum==maxgens)
						parts.radius[j] = FigureOutSize(parts.ages[j],sdata[j].Vsz,0,fade,sdata[j].L,grate,frate);
				}
			}
		}
		if (birth>0)
		{ 
			Matrix3 tm;	
			TimeValue lasttime = (atvalid<stepSize?0:(tvalid-stepSize));
			if (lasttime == lastTMTime) lasttm = lastTM;
			else lasttm = distnode->GetObjTMAfterWSM(lasttime);
			tm = distnode->GetObjectTM(tvalid);
//			nxttm = distnode->GetObjectTM(tvalid + stepSize);
			if (!fromgeom) 
				if (!GenerateNotGeom(tvalid,lasttime,birth,counter,distnode,vkind,tm,lasttm))
					birth = 0;

			for (j=counter; j<maincount; j++) 
			{
				if (born>=birth) 
					break;
				BirthParticle(distnode,tvalid,j,&ptvel2,ptvel,lasttm);
				parts.radius[j] = startsize*(1.0f + sdata[j].Vsz);
				sdata[j].themtl = int((tvalid-t0)*FperT);
				born++;
				counter++;
			}
		}

		if (distnode)
		{
			lastTM = distnode->GetObjTMAfterWSM(tvalid);
			lastTMTime = tvalid;
		}

		// Apply forces to modify velocity
		int fc = fields.Count();
		if (fc>0)
			for (j=0; j<parts.Count(); j++) 
			{ 
				Point3 force,tvel=Zero;
				for (i=0; i<fc; i++) 
				{ 
					if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
					{ 
						valid = FALSE;
						tvalid = t;
						cancelled = TRUE;
						parts.FreeAll();
						return;
					}
				if (!parts.Alive(j)) 
					continue;
				parts[j] = sdata[j].V;
				force = fields[i]->Force(tvalid,parts[j],parts.vels[j],j);
				float curdt = (float)dt;
				if ((parts.ages[j]==0)&&(sdata[j].gennum==0)) 
					curdt = tpf*sdata[j].Fo;
				tvel += 10.0f*force * curdt;
			}
		    parts.vels[j] += tvel;
		}
		count = 0;
		int IPC,ipcsteps;
		float B,Vb;
  		pblock->GetValue(PB_PAIPCOLLIDE_ON,tvalid,IPC,FOREVER);
  		pblock->GetValue(PB_PAIPCOLLIDE_STEPS,tvalid,ipcsteps,FOREVER);
  		pblock->GetValue(PB_PAIPCOLLIDE_BOUNCE,tvalid,B,FOREVER);
  		pblock->GetValue(PB_PAIPCOLLIDE_BOUNCEVAR,tvalid,Vb,FOREVER);
		if (IPC)
		{	
			CollideParticle cp;
			int ddt = dt/ipcsteps,remtime=0,snum=0;
			TimeValue curt = t;
			if (dt > 0)
				while (snum < ipcsteps)
				{	
					if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
					{	
						valid = FALSE;
						tvalid = t;
						cancelled = TRUE;
						parts.FreeAll();
						return;
					}
					if (remtime==0) 
						remtime=ddt;
						mindata md = cp.InterPartCollide(parts,cobjs,remtime,snum,B,Vb,curt,lc);
						for (j=0; j<parts.Count(); j++)
						{	
							if (parts.ages[j]>0) 
							{	
								if ((j!=md.min)&&(j!=md.min2)) 
								{	
									if (fullframe) 
										MovePart(j,md.mintime,fullframe,tpf);
									else 
										parts[j] += parts.vels[j]*(float)md.mintime;
								}
								else if (fullframe) 
									sdata[j].V = parts[j];
							}
						}
					}
			// If we didn't collide, then increment.
			for (j=0; j<parts.Count(); j++)
			{	
				sdata[j].Ts = (float)ptvel.Spin*sdata[j].Ts0;
				sdata[j].LamTs += (FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);
			}
		}
		else
		for (j=0; j<parts.Count(); j++)
		{	
			if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
			{ 
				valid = FALSE;
				tvalid = t;
				cancelled = TRUE;
				parts.FreeAll();
				return;
			}
			if ((!parts.Alive(j))||(parts.ages[j]==0)) 
				continue;
			count++;
			// Check for collisions
			BOOL collide = FALSE;
			// float meaninglesstime;
			float collisionTime, remTime = (float)dt;
			BOOL maybeStuck = FALSE;
			// for (int i=0; i<cobjs.Count(); i++) 
			for (int i=0; i<cc.MAX_COLLISIONS_PER_STEP; i++)
			{
				//if (cobjs[i]->CheckCollision(
				//		tvalid,parts[j],parts.vels[j], (float)dt, j,&meaninglesstime,TRUE))
				if (cc.CheckCollision(tvalid, parts[j], parts.vels[j], remTime, j, &collisionTime, FALSE))
				{
					collide = TRUE;
					remTime -= collisionTime;
					if (remTime <= 0.0f) break; // time limit for the current inegration step
				}
				else break;
				if (i==cc.MAX_COLLISIONS_PER_STEP-1) maybeStuck = TRUE;
			}
			if (collide)
			{
				if (!maybeStuck) // if particle stuck we can't risk to propagate particle movement for the current frame
					parts[j] += parts.vels[j] * remTime;
				if (stype==1)
				{ 
					if (sdata[j].persist==0) 
					{
						parts.ages[j] = -1;
						count--;
					}
					else 
						sdata[j].DL=0;
				}
				else if (fullframe &&((stype==COLLIDE)&&(sdata[j].gennum<maxgens)&&(sdata[j].SpVar<sper)))
				{ 
					int spmult = (int)(sdata[j].Mltvar*spawnbvar) + spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE,oldcnt);
				}
			}

			sdata[j].Ts = (float)ptvel.Spin*sdata[j].Ts0;
			sdata[j].LamTs += (FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);			
			
			// If we didn't collide, then increment.
			if (!collide) 
				MovePart(j,dt,fullframe,tpf);
			else if (fullframe) 
				sdata[j].V=parts[j];
		}
		// Next, birth particles at the birth rate
		if (first) 
			first=FALSE;
	}
/*	if ((frag==METABALLS)&&((!isrend)&&(onscreen==2)))
	{	float res,bstr,thres=0.6f;
		pblock->GetValue(PB_METATENSION,tvalid,bstr,FOREVER);
		pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
		metap.CreateMetas(parts,metamesh,thres,res,bstr);
	}
	else */
	if (((frag==BYGEOM && distnode)||((frag==INSTGEOM)&&custnode))&&(onscreen>1))
	{ 
		TimeValue anist = GetAnimStart(),aniend=GetAnimEnd();
		thePArrayDraw.anifr = aniend+stepSize;
		thePArrayDraw.t = t;
		thePArrayDraw.anioff = anioff;
		if (count>0)
			GetTimes(times,t,thePArrayDraw.anifr,anioff,frag);
		else 
			times.tl.ZeroCount();
		if (onscreen==2)
			GetMesh(t,subtree,custmtl,frag);
		else 
			GetallBB(custnode,subtree,t,frag);
	}  
	if (isrend) 
		rcounter=counter;
	else
	{ 
		vcounter=counter;
	}
	if (tvalid<t) 
		tvalid=t;
	valid=TRUE;
//	assert(tvalid==t);
}


void PArrayParticle::InvalidateUI()
{
	if (pmapParam) pmapParam->Invalidate();
	if (pmapPGen) pmapPGen->Invalidate();
	if (pmapPType) pmapPType->Invalidate();
	if (pmapPSpin) pmapPSpin->Invalidate();
	if (pmapEmitV) pmapEmitV->Invalidate();
	if (pmapBubl) pmapBubl->Invalidate();
	if (pmapSpawn) pmapSpawn->Invalidate();
}

BOOL PArrayParticle::EmitterVisible()
	{
	int hide;
	pblock->GetValue(PB_EMITRHID,0,hide,FOREVER);
	return !hide;
	}

ParamDimension *PArrayParticle::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_EMITTERCOUNT:
		case PB_SPEED:
		case PB_PBIRTHRATE:
		case PB_PTOTALNUMBER:
		case PB_SIZE:
		case PB_RNDSEED:
		case PB_METATENSION:
		case PB_METACOURSE:
		case PB_FRAGTHICKNESS:
		case PB_FRAGCOUNT:
		case PB_MAPPINGDIST:
		case PB_SPINAXISX:
		case PB_SPINAXISY:
		case PB_SPINAXISZ:
		case PB_EMITVMULT:
		case PB_BUBLAMP:
		case PB_EMITRWID:  			return stdWorldDim;

		case PB_ANGLEDIV:
		case PB_SPINPHASE:
		case PB_SPINAXISVAR:
		case PB_SMOOTHANG:
		case PB_BUBLPHAS:			return stdAngleDim;

		case PB_DISPLAYPORTION:
		case PB_SPAWNDIRCHAOS:
		case PB_SIZEVAR:
		case PB_SPEEDVAR:
		case PB_METATENSIONVAR:
		case PB_SPINTIMEVAR:
		case PB_SPINPHASEVAR:
		case PB_EMITVMULTVAR:
		case PB_BUBLAMPVAR:
		case PB_BUBLPERVAR:
		case PB_BUBLPHASVAR:		
		case PB_SPAWNMULTVAR:
		case PB_PASPAWNDIEAFTERVAR:
		case PB_PAIPCOLLIDE_BOUNCE:			
		case PB_PAIPCOLLIDE_BOUNCEVAR:			
								return stdPercentDim;

		case PB_EMITSTART:
		case PB_EMITSTOP:
		case PB_DISPUNTIL:
		case PB_LIFE:
		case PB_LIFEVAR:
		case PB_GROWTIME:
		case PB_FADETIME:
		case PB_MAPPINGTIME:
		case PB_SPINTIME:
		case PB_BUBLPER:
		case PB_OFFSETAMOUNT:
		case PB_PASPAWNDIEAFTER:
								return stdTimeDim;
		
		default: return defaultDim;
		}
	}

TSTR PArrayParticle::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_DISTRIBUTION:	return GetString(IDS_RB_DISTRIBUTION);		
		case PB_EMITTERCOUNT:	return GetString(IDS_RB_EMITTERCOUNT);
		case PB_SPEED:			return GetString(IDS_RB_SPEED);
		case PB_SPEEDVAR:		return GetString(IDS_RB_SPEEDVAR);
		case PB_ANGLEDIV:		return GetString(IDS_RB_ANGLEDIV);
		case PB_PBIRTHRATE:		return GetString(IDS_RB_PBIRTHRATE);
		case PB_PTOTALNUMBER:	return GetString(IDS_RB_PTOTALNUMBER);
		case PB_DISPLAYPORTION:	return GetString(IDS_RB_DISPLAYPORTION);
		case PB_EMITSTART:		return GetString(IDS_RB_EMITSTART);
		case PB_EMITSTOP:		return GetString(IDS_RB_EMITSTOP);
		case PB_DISPUNTIL:		return GetString(IDS_RB_DISPUNTIL);
		case PB_LIFE:			return GetString(IDS_RB_LIFE);
		case PB_LIFEVAR:		return GetString(IDS_RB_LIFEVAR);
		case PB_SIZE:			return GetString(IDS_RB_SIZE);
		case PB_SIZEVAR:		return GetString(IDS_RB_SIZEVAR);
		case PB_GROWTIME:		return GetString(IDS_RB_GROWTIME);
		case PB_FADETIME:		return GetString(IDS_RB_FADETIME);
		case PB_RNDSEED:		return GetString(IDS_RB_RNDSEED);
		case PB_METATENSION:	return GetString(IDS_RB_METATENSION);
		case PB_METATENSIONVAR:	return GetString(IDS_RB_METATENSIONVAR);
		case PB_METACOURSE:		return GetString(IDS_RB_METACOURSE);
		case PB_FRAGTHICKNESS:	return GetString(IDS_RB_FRAGTHICKNESS);
		case PB_FRAGCOUNT:		return GetString(IDS_RB_FRAGCOUNT);
		case PB_MAPPINGTIME:	return GetString(IDS_RB_MAPPINGTIME);
		case PB_MAPPINGDIST:	return GetString(IDS_RB_MAPPINGDIST);
		case PB_SPINTIME:		return GetString(IDS_RB_SPINTIME);
		case PB_SPINTIMEVAR:	return GetString(IDS_RB_SPINTIMEVAR);
		case PB_SPINPHASE:		return GetString(IDS_RB_SPINPHASE);
		case PB_SPINPHASEVAR:	return GetString(IDS_RB_SPINPHASEVAR);
		case PB_SPINAXISX:		return GetString(IDS_RB_SPINAXISX);
		case PB_SPINAXISY:		return GetString(IDS_RB_SPINAXISY);
		case PB_SPINAXISZ:		return GetString(IDS_RB_SPINAXISZ);
		case PB_SPINAXISVAR:	return GetString(IDS_RB_SPINAXISVAR);
		case PB_EMITVINFL:		return GetString(IDS_RB_EMITVINFL);
		case PB_EMITVMULT:		return GetString(IDS_RB_EMITVMULT);
		case PB_EMITVMULTVAR:	return GetString(IDS_RB_EMITVMULTVAR);
		case PB_BUBLAMP:		return GetString(IDS_RB_BUBLAMP);
		case PB_BUBLAMPVAR:		return GetString(IDS_RB_BUBLAMPVAR);
		case PB_BUBLPER:		return GetString(IDS_RB_BUBLPER);
		case PB_BUBLPERVAR:		return GetString(IDS_RB_BUBLPERVAR);
		case PB_BUBLPHAS:		return GetString(IDS_RB_BUBLPHAS);
		case PB_BUBLPHASVAR:	return GetString(IDS_RB_BUBLPHASVAR);
		case PB_EMITRWID:		return GetString(IDS_RB_EMITRWID);
		case PB_STRETCH:		return GetString(IDS_AP_STRETCH);
		case PB_OFFSETAMOUNT:	return GetString(IDS_AP_OFFSETAMT);
		case PB_SPAWNGENS:		return GetString(IDS_AP_SPAWNGENS);
		case PB_SPAWNCOUNT:		return GetString(IDS_AP_SPAWNCOUNT);
		case PB_SPAWNDIRCHAOS:	return GetString(IDS_AP_SPAWNDIRCHAOS);
		case PB_SPAWNSPEEDCHAOS:	return GetString(IDS_AP_SPAWNSPEEDCHAOS);
		case PB_SPAWNSCALECHAOS:	return GetString(IDS_AP_SPAWNSCALECHAOS);
		case PB_SPAWNLIFEVLUE:	return GetString(IDS_AP_SPAWNLIFEVLUE);
		case PB_SPAWNPERCENT:	return GetString(IDS_EP_SPAWNAFFECTS);
		case PB_SPAWNMULTVAR:	return GetString(IDS_EP_SPAWNMULTVAR);
		case PB_USESELECTED:	return GetString(IDS_AP_USESELECTED);
		case PB_PASPAWNDIEAFTER:	return GetString(IDS_AP_SPAWNDIEAFTER);
		case PB_PASPAWNDIEAFTERVAR:	return GetString(IDS_AP_SPAWNDIEAFTERVAR);
		case PB_PAIPCOLLIDE_ON:			return GetString(IDS_AP_IPCOLLIDE_ON);
		case PB_PAIPCOLLIDE_STEPS:		return GetString(IDS_AP_IPCOLLIDE_STEPS);
		case PB_PAIPCOLLIDE_BOUNCE:		return GetString(IDS_AP_IPCOLLIDE_BOUNCE);
		case PB_PAIPCOLLIDE_BOUNCEVAR:	return GetString(IDS_AP_IPCOLLIDE_BOUNCEVAR);
		default: 				return TSTR(_T(""));
		}
	}	
void PArrayParticle::GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) 
	{
	Matrix3 mat = inode->GetObjectTM(t);
	Box3 pbox;
	cnode=inode;
	int type,ptype,disptype;
	pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
//	if ((type==2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype==INSTGEOM)&&(!custnode))||((disptype==BYGEOM)&&(!distnode))))
//		type=1;
	if ((type==2)&&(((disptype==INSTGEOM)&&(!custnode))||((disptype==BYGEOM)&&(!distnode))))
		type=1;
	if ((type==3)&&((disptype==INSTGEOM)||(disptype==BYGEOM)))
		{if (disptype==INSTGEOM?!custnode:!distnode) type=1;}
	else if (type==3) type=1;
	if (disptype!=METABALLS) UpdateMesh(t);
	box  = mesh.getBoundingBox();
	pbox = parts.BoundBox();
	if (type==2) 
	{ if (disptype==BYGEOM)
	  {	for (int i=0; i<parts.points.Count(); i++)
		{ if (!parts.Alive(i)) {continue;	}
		  int frommesh=(sdata[i].gennum>0?sdata[i].frommesh:i);
	      int vpts=pmesh[frommesh].getNumVerts();
		  float elapsedtime=(float)parts.ages[i];
		  float Angle=sdata[i].LamTs;
		  Point3 pt = parts.points[i],nextpt;
		  pbox += pt;
		  for (int j=0;j<vpts;j++)
		  {nextpt=parts.radius[i]*pmesh[frommesh].verts[j]+parts.points[i];
		   RotateOnePoint(&nextpt.x,&parts.points[i].x,&(sdata[i].W.x),Angle);
		   pbox +=nextpt;
	   	  }
		}
	  }
	else 
	  {	for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt = parts.points[i];
			float r=parts.radius[i];
			int axisentered,K;
			pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
			if (axisentered==DIRTRAVEL)
			{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			  float len=GetLen(parts.vels[i],K);
			  r*=len;
			}
			if ((disptype==INSTGEOM)&&(custnode))
			{ int tlst=times.tl.Count();if (tlst==0) tlst=1;
			  pbox += pt;
			  for (int level=0;level<tlst;level++)
			  {	Box3 cbb=cmbb[level];  
			    Point3 dist=cbb.pmax-cbb.pmin;
				float pdist=Largest(dist)*r*SQR2;
			  pbox += pt + Point3( pdist, 0.0f, 0.0f);
			  pbox += pt + Point3(-pdist, 0.0f, 0.0f);
			  pbox += pt + Point3( 0.0f, pdist, 0.0f);
			  pbox += pt + Point3( 0.0f,-pdist, 0.0f);
			  pbox += pt + Point3( 0.0f, 0.0f, pdist);
			  pbox += pt + Point3( 0.0f, 0.0f,-pdist);
			  }
			}
			else
			{ if (ptype==RENDTET) r*=8.0f;
			  pbox += pt;
			  pbox += pt + Point3( r, 0.0f, 0.0f);
			  pbox += pt + Point3(-r, 0.0f, 0.0f);
			  pbox += pt + Point3( 0.0f, r, 0.0f);
			  pbox += pt + Point3( 0.0f,-r, 0.0f);
			  pbox += pt + Point3( 0.0f, 0.0f, r);
			  pbox += pt + Point3( 0.0f, 0.0f,-r);
			}
		}
	  }
	}
	else if (type==3) 
	{	for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt;
			float radius=parts.radius[i];
				int axisentered,K;
			    pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
				if (axisentered==DIRTRAVEL)
				{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
				  float len=GetLen(parts.vels[i],K);
				  radius*=len;
				}
			int n=0,found=0;;
		int nCount=nlist.Count();
		if ((disptype==INSTGEOM)||((sdata[i].gennum>0)&&(nCount>0)))
		{ TimeValue Ctime=(thePArrayDraw.anioff?GetCurTime(sdata[i].showframe,(thePArrayDraw.anioff>1?thePArrayDraw.t:parts.ages[i]),thePArrayDraw.anifr):thePArrayDraw.t);
		  found=((n=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum)))>-1);
		}
		if (!found) 
		{ if (sdata[i].gennum==0) radius=1.0f; 
		  for (int j=0;j<8;j++)
			{ pt=radius*sdata[i].wbb[j]+parts[i];
			  pbox += pt;
			}
		}
		else
		{ for (int nb=0;nb<thePArrayDraw.bboxpt[n].numboxes;nb++)
		    for (int j=0;j<8;j++)
			{ pt=(radius*(thePArrayDraw.bboxpt[n].bpts[nb].pts[j]+thePArrayDraw.bboxpt[n].bpts[nb].Suboffset))+parts[i];
			  pbox += pt;
			}
		}
		}
	  }
	if (!pbox.IsEmpty()) box += pbox * Inverse(mat);
}

void PArrayParticle::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
{   
	// particles may require update (bayboro|march.25.2002)
	if (!OKtoDisplay(t)) return;
	if (t!=tvalid) cancelled=FALSE;
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (doupdate) Update(t,inode);
	// end ofparticles may require update (bayboro|march.25.2002)

	cnode=inode;
	int type,ptype,disptype;
	pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
//	if ((type==2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype==INSTGEOM)&&(!custnode))||((disptype==BYGEOM)&&(!distnode))))
//		type=1;
	if ((type==2)&&(((disptype==INSTGEOM)&&(!custnode))||((disptype==BYGEOM)&&(!distnode))))
		type=1;
	if ((type==3)&&((disptype==INSTGEOM)||(disptype==BYGEOM)))
		{if (disptype==INSTGEOM?!custnode:!distnode) type=1;}
	else if (type==3) type=1;
	if (type==2) 
	{ if (disptype==BYGEOM)
	  {	Box3 pbox;
		Matrix3 mat =inode->GetObjTMBeforeWSM(t);	
		UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++)
		{ if (!parts.Alive(i)) {continue;	}
		  int frommesh=(sdata[i].gennum>0?sdata[i].frommesh:i);
	      int vpts=pmesh[frommesh].getNumVerts();
		  float elapsedtime=(float)parts.ages[i];
		  float Angle=sdata[i].LamTs;
		  Point3 pt = parts.points[i],nextpt;
		  pbox += pt;
		  for (int j=0;j<vpts;j++)
		  {nextpt=parts.radius[i]*pmesh[frommesh].verts[j]+parts.points[i];
		   RotateOnePoint(&nextpt.x,&parts.points[i].x,&(sdata[i].W.x),Angle);
		   pbox +=nextpt;
	   	  }
		}
		if (!pbox.IsEmpty()) box += pbox;
	  }
	else 
	  { Box3 pbox;
		Matrix3 mat = inode->GetObjTMBeforeWSM(t);
		if (disptype!=METABALLS) UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt = parts.points[i];
			float r=parts.radius[i];
			int axisentered,K;
			pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
			if (axisentered==DIRTRAVEL)
			{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			  float len=GetLen(parts.vels[i],K);
			  r*=len;
			}
			if ((disptype==INSTGEOM)&&(custnode))
			{ int tlst=times.tl.Count();if (tlst==0) tlst=1;
			  pbox += pt;
			  for (int level=0;level<tlst;level++)
			  {	Box3 cbb=cmbb[level];  
			    Point3 dist=cbb.pmax-cbb.pmin;
				float pdist=Largest(dist)*r*SQR2;
			  pbox += pt + Point3( pdist, 0.0f, 0.0f);
			  pbox += pt + Point3(-pdist, 0.0f, 0.0f);
			  pbox += pt + Point3( 0.0f, pdist, 0.0f);
			  pbox += pt + Point3( 0.0f,-pdist, 0.0f);
			  pbox += pt + Point3( 0.0f, 0.0f, pdist);
			  pbox += pt + Point3( 0.0f, 0.0f,-pdist);
			  }
			}
			else
			{ if (ptype==RENDTET) r*=8.0f;
			  pbox += pt;
			  pbox += pt + Point3( r, 0.0f, 0.0f);
			  pbox += pt + Point3(-r, 0.0f, 0.0f);
			  pbox += pt + Point3( 0.0f, r, 0.0f);
			  pbox += pt + Point3( 0.0f,-r, 0.0f);
			  pbox += pt + Point3( 0.0f, 0.0f, r);
			  pbox += pt + Point3( 0.0f, 0.0f,-r);
			}
		}
		if (!pbox.IsEmpty()) box += pbox;
	  }
	}
	else if (type==3) 
	{ Box3 pbox;
		Matrix3 mat = inode->GetObjTMBeforeWSM(t);
		if (disptype!=METABALLS) UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt;
			float radius=parts.radius[i];
				int axisentered,K;
			    pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
				if (axisentered==DIRTRAVEL)
				{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
				  float len=GetLen(parts.vels[i],K);
				  radius*=len;
				}
			int n=0,found=0;;
		int nCount=nlist.Count();
		if ((disptype==INSTGEOM)||((sdata[i].gennum>0)&&(nCount>0)))
		{ TimeValue Ctime=(thePArrayDraw.anioff?GetCurTime(sdata[i].showframe,(thePArrayDraw.anioff>1?thePArrayDraw.t:parts.ages[i]),thePArrayDraw.anifr):thePArrayDraw.t);
		  found=((n=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum)))>-1);
		}
		if (!found) 
		{ if (sdata[i].gennum==0) radius=1.0f; 
		  for (int j=0;j<8;j++)
			{ pt=radius*sdata[i].wbb[j]+parts[i];
			  pbox += pt;
			}
		}
		else
		{ for (int nb=0;nb<thePArrayDraw.bboxpt[n].numboxes;nb++)
		    for (int j=0;j<8;j++)
			{ pt=(radius*(thePArrayDraw.bboxpt[n].bpts[nb].pts[j]+thePArrayDraw.bboxpt[n].bpts[nb].Suboffset))+parts[i];
			  pbox += pt;
			}
		}
		}
		if (!pbox.IsEmpty()) box += pbox;
	  }
 else {SimpleParticle::GetWorldBoundBox(t,inode,vpt,box);}
	}

int GetDrawType(PArrayParticle *po, int &ptype,int &disptype)
{ int type;
  po->pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
  po->pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
  po->pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
  TimeValue aniend;
//  if ((type==2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype==INSTGEOM)&&(!po->custnode))||((disptype==BYGEOM)&&(!po->distnode))))
//		type=1;
  if ((type==2)&&(((disptype==INSTGEOM)&&(!po->custnode))||((disptype==BYGEOM)&&(!po->distnode))))
		type=1;
  if ((type==3)&&((disptype==INSTGEOM)||(disptype=BYGEOM)))
  {if (disptype==INSTGEOM?!po->custnode:!po->distnode) type=1;}
  else if (type==3) type=1;
  if (type>1)
  { aniend=GetAnimEnd();
	int oneframe=GetTicksPerFrame();
    po->thePArrayDraw.anifr=aniend+oneframe;
	int axisentered;
	po->pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	if (po->thePArrayDraw.indir.inaxis=(axisentered==1))
	{ po->pblock->GetValue(PB_STRETCH,0,po->thePArrayDraw.indir.K,FOREVER);
	  po->thePArrayDraw.indir.oneframe=oneframe;
	}
  }
  return type;
}


/*BOOL PArrayParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
{ HCURSOR hCur;
  BOOL chcur=FALSE;
  if (disptype==BYGEOM)
 { Point3 pt[4],tmp[2];
   BOOL bail=FALSE;
  float elapsedtime=(float)parts.ages[i];
  float Angle=(FloatEQ0(obj->sdata[i].Ts)?0.0f:elapsedtime/obj->sdata[i].Ts)+obj->sdata[i].LamTs;
  int vpts=obj->pmesh[i].getNumVerts(),nf=obj->pmesh[i].getNumFaces();
  Point3 *rlst=new Point3[vpts],nextpt;
  if (nf>LOTSOFACES)
  {  hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
     chcur=TRUE;
  }
  for (int j=0;j<vpts;j++)
  {	nextpt=obj->pmesh[i].verts[j]+parts.points[i];
	RotateOnePoint(&nextpt.x,&parts.points[i].x,&(obj->sdata[i].W.x),Angle);
    rlst[j]=nextpt;
	if (GetAsyncKeyState (VK_ESCAPE)) {bail=TRUE;goto done;}
  }
  for (j=0;j<nf;j++)
  {	pt[0]=rlst[obj->pmesh[i].faces[j].v[0]];
	pt[1]=rlst[obj->pmesh[i].faces[j].v[1]];
	pt[2]=rlst[obj->pmesh[i].faces[j].v[2]];
  if ((obj->pmesh[i].faces[j].flags & EDGE_A)>0)
    { tmp[0]=pt[0];tmp[1]=pt[1];gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);}
  if ((obj->pmesh[i].faces[j].flags & EDGE_B)>0)
    { tmp[0]=pt[1];tmp[1]=pt[2];gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);}
  if ((obj->pmesh[i].faces[j].flags & EDGE_C)>0)
    { tmp[0]=pt[2];tmp[1]=pt[0];gw->polyline(2,tmp,NULL,NULL,FALSE,NULL);}
  if (GetAsyncKeyState (VK_ESCAPE)) {bail= TRUE;goto done;}
  }
done:
  if (rlst) delete[] rlst;
  if (chcur) SetCursor(hCur);
  return bail;
}
else if (disptype==METABALLS)
{ if (firstpart)
  { Point3 pt[4]; 
    int nfaces=obj->metamesh->getNumFaces();
    if (nfaces>LOTSOFACES)
	{ hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	  chcur=TRUE;
	}
    for (int j=0;j<nfaces;j++)
    { pt[0]=obj->metamesh->verts[obj->metamesh->faces[j].v[0]];
      pt[1]=obj->metamesh->verts[obj->metamesh->faces[j].v[1]];
	  pt[2]=obj->metamesh->verts[obj->metamesh->faces[j].v[2]];
      gw->polyline(3,pt,NULL,NULL,TRUE,NULL);
	  if (GetAsyncKeyState (VK_ESCAPE)) 
	  { if (chcur) SetCursor(hCur);	return TRUE;}
    }
    firstpart=FALSE;
  }
}
else if (disptype==INSTGEOM)
{ Point3 pt[4];  
 if (obj->custnode)
  { int n=0,found=0;
    TimeValue Ctime=(anioff?GetCurTime(obj->sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	found=((n=TimeFound(obj->times,Ctime,(obj->sdata[i].gennum>obj->nCount?obj->nCount:obj->sdata[i].gennum)))>-1);
	float radius=parts.radius[i];
	float elapsedtime=(float)parts.ages[i];
    float Angle=(FloatEQ0(obj->sdata[i].Ts)?0.0f:elapsedtime/obj->sdata[i].Ts)+obj->sdata[i].LamTs;
	if (indir.inaxis)
	  indir.vel=parts.vels[i];
	Point3 nextpt;
	if (found) 
	{
	if (bb)
    { Point3 pt[9];
	  for (int nb=0;nb<bboxpt[n].numboxes;nb++)
	  { for (int j=0;j<8;j++)
	    {if (indir.inaxis)
	       pt[j]=RotateAboutAxis(Angle,parts.points[i],radius*(bboxpt[n].bpts[nb].pts[j]+bboxpt[n].bpts[nb].Suboffset),obj->sdata[i].W,indir);
	     else
		 { nextpt=(radius*(bboxpt[n].bpts[nb].pts[j]+bboxpt[n].bpts[nb].Suboffset))+parts[i];
		   RotateOnePoint(&nextpt.x,&parts.points[i].x,&(obj->sdata[i].W.x),Angle);
		   pt[j]=nextpt;
		 }
	    }
	    gw->polyline(4,pt,NULL,NULL,TRUE,NULL);
		Point3 tmppt[5]; for (int k=0;k<4;k++) tmppt[k]=pt[4+k];
		gw->polyline(4,tmppt,NULL,NULL,TRUE,NULL);
		tmppt[0]=pt[0];tmppt[1]=pt[4];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[1];tmppt[1]=pt[5];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[2];tmppt[1]=pt[6];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[3];tmppt[1]=pt[7];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		if (GetAsyncKeyState (VK_ESCAPE))  { return TRUE;}
	  }
	}
    else
    { Mesh *mp;
	  mp=&obj->cmesh[n];
	  int nfaces=mp->getNumFaces();
	  if (nfaces>LOTSOFACES)
	  {  hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
		 chcur=TRUE;
	  }
		for (int j=0;j<nfaces;j++)
		{ for (int pnum=0;pnum<3;pnum++)
		{if (indir.inaxis)
	    pt[pnum]=RotateAboutAxis(Angle,parts.points[i],radius*(mp->verts[mp->faces[j].v[pnum]]),obj->sdata[i].W,indir);
		else
		  { nextpt=(radius*(mp->verts[mp->faces[j].v[pnum]]))+parts[i];
		    RotateOnePoint(&nextpt.x,&parts.points[i].x,&(obj->sdata[i].W.x),Angle);
		    pt[pnum]=nextpt;
		  }}
		  gw->polyline(3,pt,NULL,NULL,TRUE,NULL);
		  if (GetAsyncKeyState (VK_ESCAPE))  
		  { if (chcur) SetCursor(hCur);	return TRUE;}
		}
	}
	}
  }
}
else
{  Mesh *pm = new Mesh;
    Point3 pt[4];  
	int numF,numV;	
    GetMeshInfo(ptype,1,pm,&numF,&numV);
    float elapsedtime=(float)obj->parts.ages[i];
    float Angle=(FloatEQ0(obj->sdata[i].Ts)?0.0f:elapsedtime/obj->sdata[i].Ts)+obj->sdata[i].LamTs;
	if (indir.inaxis) indir.vel=parts.vels[i];
	if (ptype==RENDTYPE1) PlotTriangle(obj->parts.radius[i],0,0,pm,Angle,&obj->sdata[i].W.x,0,&obj->parts.points[i],indir);
	else if (ptype==RENDTYPE2) PlotCube8(obj->parts.radius[i],0,0,pm,Angle,&obj->sdata[i].W.x,0,&obj->parts.points[i],indir);
	else if (ptype==RENDTYPE3) PlotSpecial(obj->parts.radius[i],0,0,pm,Angle,&obj->sdata[i].W.x,0,&obj->parts.points[i],indir);
	else if (ptype==RENDTET) PlotTet(obj->parts.radius[i],0,0,pm,Angle,obj->sdata[i].W,0,&obj->parts.points[i],indir);
	else if (ptype==REND6PT) Plot6PT(obj->parts.radius[i],0,0,pm,Angle,&obj->sdata[i].W.x,0,&obj->parts.points[i],indir);
	int nfaces=pm->getNumFaces();
	if (nfaces>LOTSOFACES)
	{  hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	   chcur=TRUE;
	}
   for (int j=0;j<nfaces;j++)
    { pt[0]=pm->verts[pm->faces[j].v[0]];
      pt[1]=pm->verts[pm->faces[j].v[1]];
	  pt[2]=pm->verts[pm->faces[j].v[2]];
      gw->polyline(3,pt,NULL,NULL,TRUE,NULL);
    }
  if (pm) delete pm;
  if (GetAsyncKeyState (VK_ESCAPE)) 
  { if (chcur) SetCursor(hCur);	return TRUE;}
}
return 0;
}*/	

BOOL PArrayParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
{ HCURSOR hCur;
  BOOL chcur=FALSE;
  if (!(((disptype==INSTGEOM)||(disptype==BYGEOM))&&(bb))) return TRUE;
  Point3 pt[4];
  INode *onode=(disptype==BYGEOM?obj->distnode:obj->custnode);
  if (onode)
  { int n=0,found=0;
    int nCount=obj->nlist.Count();
    if ((disptype==INSTGEOM)||((obj->sdata[i].gennum>0)&&(nCount>0)))
    { TimeValue Ctime=(anioff?GetCurTime(obj->sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	  found=((n=TimeFound(obj->times,Ctime,(obj->sdata[i].gennum>nCount?nCount:obj->sdata[i].gennum)))>-1);
	}
	float radius=parts.radius[i];
	float elapsedtime=(float)parts.ages[i];
    float Angle=obj->sdata[i].LamTs;
	if (indir.inaxis)
	  indir.vel=parts.vels[i];
	Point3 nextpt;BOOL usepmesh=FALSE;
	int numbox=(usepmesh=((!found)&&(disptype==BYGEOM))?1:bboxpt[n].numboxes);
	if ((disptype==BYGEOM)&&(obj->sdata[i].gennum==0)) radius=1.0f;
	if (found ||(disptype==BYGEOM)) 
	{ Point3 pt[9];
	  if (numbox*8>LOTSOFACES)
	  { hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	    chcur=TRUE;
	  }
	  for (int nb=0;nb<numbox;nb++)
	  { for (int j=0;j<8;j++)
	  { Point3 boxpt=radius*(found?bboxpt[n].bpts[nb].pts[j]+bboxpt[n].bpts[nb].Suboffset:obj->sdata[i].wbb[j]);
		  if (indir.inaxis)
	       pt[j]=RotateAboutAxis(Angle,parts.points[i],boxpt,obj->sdata[i].W,indir);
	     else
		 { nextpt=boxpt+parts[i];
		   RotateOnePoint(&nextpt.x,&parts.points[i].x,&(obj->sdata[i].W.x),Angle);
		   pt[j]=nextpt;
		 }
	    }
	    gw->polyline(4,pt,NULL,NULL,TRUE,NULL);
		Point3 tmppt[5]; for (int k=0;k<4;k++) tmppt[k]=pt[4+k];
		gw->polyline(4,tmppt,NULL,NULL,TRUE,NULL);
		tmppt[0]=pt[0];tmppt[1]=pt[4];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[1];tmppt[1]=pt[5];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[2];tmppt[1]=pt[6];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		tmppt[0]=pt[3];tmppt[1]=pt[7];
		gw->polyline(2,tmppt,NULL,NULL,FALSE,NULL);
		if (GetAsyncKeyState (VK_ESCAPE))  { return TRUE;}
	  }
	}
  }
  if (GetAsyncKeyState (VK_ESCAPE)) 
  { if (chcur) SetCursor(hCur);	return TRUE;}
return 0;
}

MarkerType PArrayParticle::GetMarkerType() 
{ int ptype,disptype,type=GetDrawType(this,ptype,disptype);
	switch (type) {
		case 0:
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;			
		case 1:
			parts.SetCustomDraw(NULL);
			return PLUS_SIGN_MRKR;
		case 2:	{			
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;
			}	 
		case 3:{thePArrayDraw.obj = this;
			thePArrayDraw.firstpart=TRUE;
			thePArrayDraw.disptype=disptype;
			thePArrayDraw.ptype=ptype;
			thePArrayDraw.bb=TRUE;
			parts.SetCustomDraw(&thePArrayDraw);			
			return POINT_MRKR;
		   }
		default:
			return PLUS_SIGN_MRKR;
		}
	}



//--- PArray particle -----------------------------------------------

RefTargetHandle PArrayParticle::Clone(RemapDir& remap) 
	{
	PArrayParticle* newob = new PArrayParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	if (distnode) newob->ReplaceReference(DISTNODE,distnode);
	newob->custname = custname;
	newob->distname = distname;
	newob->mvalid.SetEmpty();	
	newob->tvalid = FALSE;
	newob->nlist.SetCount(nlist.Count());
	newob->llist.SetCount(llist.Count());
	for (int ix=0;ix<nlist.Count();ix++) 
	{ newob->nlist[ix]=NULL;
	  newob->ReplaceReference(ix+BASER,nlist[ix]);
	}
	for (ix=0;ix<llist.Count();ix++) newob->llist[ix]=llist[ix];
	newob->dflags=dflags;
	newob->dispmesh=NULL;
	newob->times.tl.SetCount(0);
	newob->nmtls.ZeroCount();
	newob->cancelled=FALSE;
	newob->wasmulti=FALSE;
	newob->stepSize=stepSize;
	int vshow;
	pblock->GetValue(PB_VIEWPORTSHOWS,0,vshow,FOREVER);
	if (vshow>1)
	{int subtree,anioff,custmtl;
	 pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	 pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	 TimeValue aniend=GetAnimEnd();
	 int anifr=aniend+GetTicksPerFrame();
	 TimeValue t=GetCOREInterface()->GetTime();
	 newob->GetTimes(newob->times,t,anifr,anioff,fragflags);
	 if (vshow==2)
	 { pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
	   newob->GetMesh(t,subtree,custmtl,fragflags);
	 }
	 else newob->GetallBB(custnode,subtree,t,fragflags);
	}
	BaseClone(this, newob, remap);
	return newob;
	}

static float findmappos(float curpos)
{ float mappos;

  return(mappos=((mappos=curpos)<0?0:(mappos>1?1:mappos)));
}


void PArrayParticle::RendGeom(Mesh *pm,Matrix3 itm,int maxmtl,int maptype,int emitmtl,float mval,PArrayParticleDraw thePArrayDraw,TVFace defface,BOOL notrend)
{ float elapsedtime,Angle,Uval=0.5f,Wval=0.5f,Vval=0.0f;
  Point3 nextpt;
  int vertexnum=0,tvcnt=0,fn,face=0,dtvs=pm->getNumTVerts(),ctvs=0;
  if (emitmtl==2) ctvs=cmesh[0].getNumTVerts();
  TimeValue t=thePArrayDraw.t;int anifr=thePArrayDraw.anifr,anioff=thePArrayDraw.anioff;
  MtlID mid=0;
  for (int i=0; i<parts.Count(); i++) 
  { if ((notrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
		{ ZeroMesh(pm);cancelled=TRUE;return;}
	if (!parts.Alive(i)) continue;
    elapsedtime=(float)parts.ages[i];
	TVFace parttv(tvcnt,tvcnt,tvcnt);
	int mnum=-1;TimeValue Ctime;int nCount=nlist.Count();
	if ((sdata[i].gennum>0)&&(nCount>0))
	{ Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	  mnum=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum));
	} 
	if (emitmtl==0)
	{ if (maxmtl) 
		{ mid=sdata[i].themtl;
	      if (mid>=maxmtl) 
		    mid =mid % maxmtl;
	    } else mid=i;
		if (maptype) Vval=Length(parts[i]-sdata[i].start)/mval;
		else Vval=(float)elapsedtime/mval;
		pm->tVerts[tvcnt]=Point3(findmappos(Uval),findmappos(Vval),findmappos(Wval));
		tvcnt++;
	}
	Angle=sdata[i].LamTs;
	int frommesh=(sdata[i].gennum>0?sdata[i].frommesh:i);
	int maxf,numv;
	if (mnum<0) {maxf=pmesh[frommesh].getNumFaces();numv=pmesh[frommesh].getNumVerts();}
	else {maxf=cmesh[mnum].getNumFaces();numv=cmesh[mnum].getNumVerts();}
	for (fn=0;fn<maxf;fn++)
	{ Face f=(mnum<0?pmesh[frommesh].faces[fn]:cmesh[mnum].faces[fn]);
	pm->faces[face].setSmGroup(f.getSmGroup());
	if (emitmtl==0) {pm->tvFace[face]=parttv;pm->faces[face].setMatID(mid);}
	else 
	{ if (dtvs) pm->tvFace[face]=(emitmtl==1?pmesh[frommesh].tvFace[mnum<0?fn:0]:(mnum<0?(ctvs?defface:pmesh[frommesh].tvFace[fn]):(ctvs?cmesh[mnum].tvFace[fn]:defface)  ));
	MtlID mid;
	  if (mnum<0) mid=pmesh[frommesh].faces[fn].getMatID();
	  else 
	  { if (emitmtl==1) mid=pmesh[frommesh].faces[0].getMatID();
		else 
		{ mid=cmesh[mnum].faces[fn].getMatID();
			 int mtlgen=times.tl[mnum].gennum-1,maxmtl=nmtls.Count();
			 if (mtlgen>=maxmtl) mtlgen=maxmtl-1;
			 if ((mtlgen>-1)&&((times.tl.Count()>0)&&(times.tl[mnum].gennum>0)))
			  mid+=nmtls[mtlgen];
		}
	  }
	  if (maxmtl>0)
	  { if (mid<((emitmtl<2)||(sdata[i].gennum>0)?maxmtl:nmtls[0]))
		  pm->faces[face].setMatID(mid);
		 else pm->faces[face].setMatID(0);
	  }else pm->faces[face].setMatID(i);
	}
	 pm->faces[face].setEdgeVisFlags(f.getEdgeVis(0),f.getEdgeVis(1),f.getEdgeVis(2));
	 pm->faces[face++].setVerts(f.v[0]+vertexnum,f.v[1]+vertexnum,f.v[2]+vertexnum);
	}
	float radius=(sdata[i].gennum>0?parts.radius[i]:1.0f);
    for (int j=0;j<numv;j++)
    { nextpt=parts.radius[i]*(mnum<0?pmesh[frommesh].verts[j]:cmesh[mnum].verts[j])+parts[i];
      RotateOnePoint(&nextpt.x,&parts.points[i].x,&sdata[i].W.x,Angle);
      pm->verts[vertexnum++]=itm*nextpt;
	}
  }
}
void CacheSpin(float *holddata,SavePt *sdata,int pcount,BOOL issave)
{ for (int i=0;i<pcount;i++)
	if (issave) holddata[i]=sdata[i].LamTs;
	else sdata[i].LamTs=holddata[i];
}

Mesh *PArrayParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
{	float Mval, Uval, Vval,Wval,Angle,elapsedtime;
	int type, count,maptype,anifr;
	float width;
	TVFace defface;
	Tab<int> tVertsOffset;
	MultipleChannelMapMerger mcmm;
	BOOL mirror=FALSE;
	Mesh *pm = new Mesh;
	if (cancelled) 
	{ZeroMesh(pm);mesh.InvalidateGeomCache();
	needDelete = TRUE;return pm;}
	TimeValue mt,aniend=GetAnimEnd();
	int isrend=!TestAFlag(A_NOTREND);
	if (!isrend) dispt=t;
    anifr=aniend+GetTicksPerFrame();
	int nummtls=0,curmtl=0,multi=0,custmtl,pc=0;
	pblock->GetValue(PB_PARTICLETYPE,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,pc,FOREVER);
	if (pc==BYGEOM) type=(distnode?RENDGEOM:0);
	else if (pc==METABALLS) type=RENDMETA;
	else if (pc==INSTGEOM) type=(custnode?RENDCGEOM:0);
	if (type==0) pc=ISSTD;
	float FperT=GetFrameRate()/(float)TIME_TICKSPERSEC;
	pblock->GetValue(PB_EMITRWID,t,width,FOREVER);
	pblock->GetValue(PB_MAPPINGTYPE,0,maptype,FOREVER);
	pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
	if (maptype)
	 pblock->GetValue(PB_MAPPINGDIST,t,Mval,FOREVER);
	else 
	 pblock->GetValue(PB_MAPPINGTIME,t,mt,FOREVER);

//my comment out below - ecp
//	if ((!isrend)&&((type==RENDTYPE5)||(type==RENDTYPE6))) 
//		type=RENDTYPE1;
	Matrix3 wcam,cam = ident;
	Point3 v, v0,v1, otherV = Zero, camV = Zero;
	if (isrend)
	{ 
		cam = Inverse(wcam=view.worldToView);
		otherV=cam.GetRow(2),camV = cam.GetRow(3);
	}
	else
	{
		// this is new active viewport stuff here
		ViewExp *VE = GetCOREInterface()->GetActiveViewport();
		VE->GetAffineTM(wcam);
		GetCOREInterface()->ReleaseViewport(VE);
		cam = Inverse(wcam);
		otherV = cam.GetRow(2), camV = cam.GetRow(3);
	}


	Matrix3 tm = inode->GetObjectTM(t);
	Matrix3 itm = Inverse(tm);
	int vertexnum=0, face=0,numV=0,numF=0,j=0,tvnum=0,ismapped=0;

	ParticleSys lastparts;
	TimeValue offtime=t%GetTicksPerFrame();
	BOOL midframe;
	midframe=offtime>0;
	float *holddata=NULL;int pcount=0;
	if (isrend)
	{if (midframe) 
	{ Update(t-offtime,inode);
	  if ((pcount=parts.Count())>0)
	  { CacheData(&parts,&lastparts);
	    holddata=new float[pcount];
        CacheSpin(holddata,sdata,pcount,TRUE);
	  }
	}
	 Update(t,inode);
	}
	count = CountLive();
	Point3 r1;
	float Thetah;
	if (count==0) ZeroMesh(pm);
	else 
	{mirror=DotProd(tm.GetRow(0)^tm.GetRow(1),tm.GetRow(2))<0.0f;
	 Mtl *mtl;
	mtl=inode->GetMtl();
	BOOL txtfound=FALSE;
    if (mtl)
	{ Class_ID mc=Class_ID(MULTI_CLASS_ID,0);
	  Class_ID oc=mtl->ClassID();
	  if (multi=(oc==mc))
	  { nummtls=mtl->NumSubMtls();if (nummtls==0) multi=0;}
    }
	wasmulti=multi;
	if (type==RENDTYPE6)
	  { if (view.projType) type=RENDTYPE5;
	    else
	    { Thetah=view.fov;
	      r1=cam.GetRow(1);
	    }
	  }
	int gtvnum=0,anioff=0;
	if (pc==0) GetMeshInfo(type,count,pm,&numF,&numV);
	else if (type==RENDGEOM)
	{/* int nv=0,nf=0,ntv=0;
      for (int tmp=0; tmp<parts.Count(); tmp++) 
	  { if (GetAsyncKeyState (VK_ESCAPE)) 
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(tmp)) continue;
	    nv+=pmesh[tmp].getNumVerts();
		nf+=pmesh[tmp].getNumFaces();
	  }
	  pm->setNumVerts(nv);
	  pm->setNumFaces(nf);
	  BOOL dotxt=((doTVs)||(!custmtl));
	  if ((count>0)&&(dotxt))
	  { if (doTVs) pm->setNumTVerts(oldtvnum);
	    else pm->setNumTVerts(count);
	  }
	  else pm->setNumTVerts(0);
	  if ((count>0)&&(dotxt)) 
	  {  pm->setNumTVFaces(nf);
	     if (doTVs)
		 { for (tmp=0;tmp<oldtvnum;tmp++)
		   { pm->tVerts[tmp]=pmesh[0].tVerts[tmp];}
		 }
	  }
	  else pm->setNumTVFaces(0);*/
	int subtree,onscreen;
	  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	  thePArrayDraw.t=t;
	  GetTimes(times,t,anifr,anioff,pc);
	  GetMesh(t,subtree,custmtl,pc);
	  TimeValue Ctime;
	  int mnum,tmptvs=0;
	  numV=0;numF=0;
	  BOOL alltex=TRUE;int distemit=0,dtvs=pmesh[0].getNumTVerts();
	  for (int pcnt=0;pcnt<parts.Count();pcnt++)
	  { if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(pcnt)) continue;mnum=-1;
		int nCount=nlist.Count();
		if ((sdata[pcnt].gennum>0)&&(nCount>0))
		{ Ctime=(anioff?GetCurTime(sdata[pcnt].showframe,(anioff>1?t:parts.ages[pcnt]),anifr):t);
		  mnum=TimeFound(times,Ctime,(sdata[pcnt].gennum>nCount?nCount:sdata[pcnt].gennum));
		} 
		if (mnum<0)
		{ int frommesh=(sdata[pcnt].gennum>0?sdata[pcnt].frommesh:pcnt);
		  numV+=pmesh[frommesh].getNumVerts();
		  numF+=pmesh[frommesh].getNumFaces();
		  distemit++;
		}
		else
		{ numV+=cmesh[mnum].getNumVerts();
		  numF+=cmesh[mnum].getNumFaces();
		}
	  }
	  pm->setNumFaces(numF);
	  pm->setNumVerts(numV);
	  int mcnt=0;
      if (!custmtl) gtvnum=count;
	  else if (custmtl==1) gtvnum=dtvs;
	  else
	  {	mcnt=times.tl.Count();if (mcnt==0) mcnt=1;
	    if (distemit) 
		{ if (dtvs==0) alltex=FALSE;
		  else gtvnum+=dtvs;
		}
		for (int mc=0;mc<mcnt;mc++)
		{ tmptvs=cmesh[mc].getNumTVerts();
		  if (tmptvs==0) alltex=FALSE;
		  else gtvnum+=tmptvs;
		}
        if ((!alltex)&&(gtvnum>0))
		{ defface.setTVerts(gtvnum,gtvnum,gtvnum);gtvnum++;}
	  	tVertsOffset.SetCount(mcnt);
		tVertsOffset[0]=0;
	  }
	  int gtv,tvs=0;
	  pm->setNumTVerts(gtv=gtvnum);
	  if ((custmtl)&&(gtvnum>0))	     
		for (int imtv=0;imtv<dtvs;imtv++) pm->tVerts[tvs++]=pmesh[0].tVerts[imtv];
	  if ((custmtl==2)&&(gtvnum>0))
	  { int mtvs=0,imtv;
		 for (int mc=0;mc<mcnt;mc++)
	     { if (mc) tVertsOffset[mc]=tVertsOffset[mc-1]+mtvs;
		   mtvs=cmesh[mc].getNumTVerts();
		   if (mtvs>0)
		   { for (imtv=0;imtv<mtvs;imtv++)
			   pm->tVerts[tvs++]=cmesh[mc].tVerts[imtv];
		   } 
		 }
		 if (!alltex) pm->tVerts[tvs]=deftex;
	  }
	  pm->setNumTVFaces(gtv>0?numF:0);

	  // Multiple Channel Map Support (single line)
	  if (custmtl) mcmm.setNumTVertsTVFaces(pm, cmesh, custmtl, mcnt);
	}
	else if (type==RENDCGEOM)
	{int subtree,onscreen;
	  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	  thePArrayDraw.t=t;
	  GetTimes(times,t,anifr,anioff,pc);
	  GetMesh(t,subtree,custmtl,pc);
	  TimeValue Ctime;
	  int mnum,tmptvs=0;
	  numV=0;numF=0;
	  BOOL alltex=TRUE;
	  for (int pcnt=0;pcnt<parts.Count();pcnt++)
	  { if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(pcnt)) continue;
		Ctime=(anioff?GetCurTime(sdata[pcnt].showframe,(anioff>1?t:parts.ages[pcnt]),anifr):t);
		int nCount=nlist.Count();
		mnum=TimeFound(times,Ctime,(sdata[pcnt].gennum>nCount?nCount:sdata[pcnt].gennum));
		numV+=cmesh[mnum].getNumVerts();
		numF+=cmesh[mnum].getNumFaces();
	  }
	  pm->setNumFaces(numF);
	  pm->setNumVerts(numV);
	  int mcnt=0;
      if (!custmtl) gtvnum=count;
	  else if (custmtl==1) gtvnum=((doTVs)?count:0);
	  else
	  {	mcnt=times.tl.Count();if (mcnt==0) mcnt=1;
		for (int mc=0;mc<mcnt;mc++)
		{ tmptvs=cmesh[mc].getNumTVerts();
		  if (tmptvs==0) alltex=FALSE;
		  else gtvnum+=tmptvs;
		}
        if ((!alltex)&&(gtvnum>0))
		{ defface.setTVerts(gtvnum,gtvnum,gtvnum);gtvnum++;}
	  	tVertsOffset.SetCount(mcnt);
		tVertsOffset[0]=0;
	  }
	  int gtv;
	  pm->setNumTVerts(gtv=gtvnum);
	  if ((custmtl==2)&&(gtvnum>0))
	  { int tvs=0,mtvs=0,imtv;
		 for (int mc=0;mc<mcnt;mc++)
	     { if (mc) tVertsOffset[mc]=tVertsOffset[mc-1]+mtvs;
		   mtvs=cmesh[mc].getNumTVerts();
		   if (mtvs>0)
		   { for (imtv=0;imtv<mtvs;imtv++)
			   pm->tVerts[tvs++]=cmesh[mc].tVerts[imtv];
		   } 
		 }
		 if (!alltex) pm->tVerts[tvs]=deftex;
	  }
	  else if (custmtl==1) 
	  { pm->setNumTVerts(gtv=((doTVs)&&(count>0)?count:0));
	  }
	  pm->setNumTVFaces(gtv>0?numF:0);

	  // Multiple Channel Map Support (single line)
	  if (custmtl) mcmm.setNumTVertsTVFaces(pm, cmesh, mcnt);
	}
	Uval=0.5f;Wval=0.5f;
	Point3 ipt;
	int i;
	if (count>0)
	{	InDirInfo indir;
	int axisentered;
    pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	indir.oneframe=GetTicksPerFrame();indir.vel=Zero;indir.K=0;
	if (indir.inaxis=(axisentered==1))
	{ 
	  pblock->GetValue(PB_STRETCH,0,indir.K,FOREVER);
	}
	if (type==RENDGEOM) 
	{ if (backpatch)
		{int subtree,frag,custmtl=0,submtl=0;
		pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
		pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
		pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
		if ((custnode)&&(frag==INSTGEOM)&& custmtl) 
		  GetSubs(inode,custnode,subtree,t,frag);
		else if ((distnode)&&(frag==BYGEOM)&&(custmtl==2))
			GetSubs(inode,distnode,subtree,t,frag);}
		RendGeom(pm,itm,nummtls,maptype,custmtl,(maptype?Mval:(float)mt),thePArrayDraw,defface,!isrend);
	}
	else 
	if (type==RENDMETA)
	{ float res,bstr,thres=0.6f;int notdraft;
      pblock->GetValue(PB_METATENSION,0,bstr,FOREVER);
      pblock->GetValue(PB_PANOTDRAFT,t,notdraft,FOREVER);
	  notdraft = (notdraft?0:1);
	  if (isrend)
		pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
	  else pblock->GetValue(PB_METACOURSEV,0,res,FOREVER);
	  metap.CreateMetas(parts,pm,thres,res,bstr,notdraft);	
	  for (int j=0;j<pm->getNumVerts();j++)
         pm->verts[j] = itm * pm->verts[j];
 	}
	else
	{   MtlID cm;
    for (i=0; i<parts.Count(); i++) 
	{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
	  if (!parts.Alive(i)) continue;
	  if (indir.inaxis)
	    indir.vel=parts.vels[i];
	    if (multi) 
		{ curmtl=((custmtl==1)?sdata[i].pmtl:sdata[i].themtl);
	      if (curmtl>=nummtls) 
		    curmtl=curmtl % nummtls;
	    } else curmtl=i;
		float x;
	    elapsedtime=(float)parts.ages[i];
		if (maptype) Vval=(x=Length(parts[i]-sdata[i].start))/Mval;
		else Vval=(float)elapsedtime/mt;
        Angle=sdata[i].LamTs;
	    if (type==RENDTYPE1) PlotTriangle(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTYPE2) PlotCube8(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTYPE3) PlotSpecial(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTET) PlotTet(parts.radius[i],vertexnum,face,pm,Angle,sdata[i].W,curmtl,&parts.points[i],indir);
	    else if (type==REND6PT) Plot6PT(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
		else if (type==RENDSPHERE) PlotSphere(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
		else if (type==RENDTYPE5)
	    {	v  = (view.projType?otherV:Normalize(camV-parts[i]));
			v0 = Normalize(v111^v) * parts.radius[i];
		    v1 = Normalize(v0^v) * parts.radius[i];
		    pm->verts[vertexnum] = (parts[i]+v0+v1);
		    pm->verts[vertexnum+1] = (parts[i]-v0+v1);
		    pm->verts[vertexnum+2] = (parts[i]-v0-v1);
		    pm->verts[vertexnum+3] = (parts[i]+v0-v1);
		    for (int l=0;l<4;l++)
		      RotateOnePoint(pm->verts[vertexnum+l],&parts.points[i].x,&v.x,Angle);
		    AddFace(vertexnum+3,vertexnum+2,vertexnum+1,face,pm,curmtl);
		    AddFace(vertexnum+1,vertexnum,vertexnum+3,face+1,pm,curmtl);
		 }
		 else if (type==RENDTYPE6)
		 { 	Point3 a,b,Rv;
			float Theta,R=Length(Rv=(parts[i]-camV)),angle=parts.radius[i]*R*Thetah/view.screenW;
			a=angle*Normalize(r1^Rv);
			b=angle*Normalize(a^Rv);
			pm->verts[vertexnum]=parts[i];
			for (int l=0;l<numV;l++)
			{ Theta=(l-1)*PIOver5;
			  pm->verts[vertexnum+l]=parts[i]+(float)cos(Theta)*a+(float)sin(Theta)*b;
			}
			for (l=0;l<numF-1;l++)
			  AddFace(vertexnum,vertexnum+1+l,vertexnum+2+l,face+l,pm,curmtl);
		    AddFace(vertexnum,vertexnum+1+l,vertexnum+1,face+l,pm,curmtl);
		 }
		 else if (type==RENDCGEOM)
		 {  if (backpatch)
			{ int subtree,frag,custmtl=0,submtl=0;
			  pblock->GetValue(PB_CUSTOMMATERIAL,0,custmtl,FOREVER);
			  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
			  pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
			  if ((custnode)&&(frag==INSTGEOM)&& custmtl) 
				GetSubs(inode,custnode,subtree,t,frag);
			  else if ((distnode)&&(frag==BYGEOM)&&(custmtl==2))
			  GetSubs(inode,distnode,subtree,t,frag);
			}
			TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
			int nCount=nlist.Count();
		 int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum));
		   if (mnum<0) continue;
		   numF=cmesh[mnum].getNumFaces();
		   numV=cmesh[mnum].getNumVerts();
			if ((gtvnum>0)&&(custmtl==2))
			{ for (j=0;j<numF;j++)
				 if (cmesh[mnum].getNumTVerts()>0) 
					 pm->tvFace[j+face].setTVerts(cmesh[mnum].tvFace[j].t[0]+tVertsOffset[mnum],
													cmesh[mnum].tvFace[j].t[1]+tVertsOffset[mnum],
													cmesh[mnum].tvFace[j].t[2]+tVertsOffset[mnum]);
				 else pm->tvFace[j+face]=defface;
			}
			// Multiple Channel Map Support (single line)
			if (custmtl) mcmm.setTVFaces(pm, cmesh, face, mnum);

			int jf=face;
		  for (j=0;j<numF;j++)
		  { pm->faces[jf].flags=cmesh[mnum].faces[j].flags;
		    pm->faces[jf].setSmGroup(cmesh[mnum].faces[j].smGroup);
		    pm->faces[jf].setVerts(vertexnum+cmesh[mnum].faces[j].v[0],vertexnum+cmesh[mnum].faces[j].v[1],vertexnum+cmesh[mnum].faces[j].v[2]);
		    if (!custmtl)
		      pm->faces[jf].setMatID((MtlID)curmtl); 
		    else if (custmtl==1)
			 pm->faces[jf].setMatID(sdata[i].pmtl);
			else
			{cm=cmesh[mnum].faces[j].getMatID();
			 int mtlgen=times.tl[mnum].gennum-1,maxmtl=nmtls.Count();
			 if (mtlgen>=maxmtl) mtlgen=maxmtl-1;
			 if ((mtlgen>-1)&&((times.tl.Count()>0)&&(times.tl[mnum].gennum>0)))
				cm+=nmtls[mtlgen];
			 pm->faces[jf].setMatID(cm);
			}
			jf++;
//		  pm->faces[face].setEdgeVisFlags(f[j].flags&EDGE_A,f[j].flags&EDGE_Bf[j].flags&EDGE_A);
		  }
		  PlotCustom(parts.radius[i],i,vertexnum,pm,Angle,&sdata[i].W.x,&cmesh[mnum],&parts.points[i],numV,indir);
		 }
	  // Convert to object coords
        for (j=0;j<numV;j++)
        { pm->verts[vertexnum] = itm * pm->verts[vertexnum];
          vertexnum++;
        }
		if ((type!=RENDTET)&&((type!=RENDCGEOM)||(custmtl==0)||((custmtl==1)&&doTVs)) )
		{ if (custmtl)
		    pm->tVerts[tvnum]=sdata[i].tv;
		  else pm->tVerts[tvnum]=Point3(findmappos(Uval),findmappos(Vval),findmappos(Wval));
	      for (j=0;j<numF;j++)
          { pm->tvFace[face++].setTVerts(tvnum,tvnum,tvnum); }	
	      tvnum++;
		} else face+=numF;
	  }
	}
   }
  }
  if ((isrend)&&(midframe))
  { if (pcount>0)
	{ CacheData(&lastparts,&parts);
      CacheSpin(holddata,sdata,pcount,FALSE);
	  delete[] holddata;}
    tvalid=t-offtime;
  }
  if (mirror) SwitchVerts(pm);
quit:  mesh.InvalidateGeomCache();
  needDelete = TRUE;
  return pm;
}

RefTargetHandle PArrayParticle::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case DISTNODE: return (RefTargetHandle)distnode;
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return (RefTargetHandle)nlist[i-BASER];
		}
	}

void PArrayParticle::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case DISTNODE: distnode = (INode *)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		default: nlist[i-BASER]= (INode *)rtarg;return;
		}
	}

RefResult PArrayParticle::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_TARGET_DELETED:	
			{ if (hTarget==distnode)
			 {  if (theHold.Holding())
			     theHold.Put(new CreatePAPartDelNode(this,distnode->GetName(),1));
 				DeleteReference(DISTNODE);
			    distnode=NULL;cancelled=FALSE;
			 }
			 if (hTarget==custnode)
			 { if (theHold.Holding()) theHold.Put(new CreatePAPartDelNode(this,custnode->GetName(),0)); 
 				DeleteReference(CUSTNODE);
			   custnode=NULL;cancelled=FALSE;
			 }
			 BOOL notfound=TRUE;
			  for (int i=0;(i<nlist.Count())&&(notfound);i++)
				if (hTarget==nlist[i]) 
				{ DeleteFromList(i,TRUE);
				 notfound=FALSE;cancelled=FALSE;}
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ if (hTarget==distnode) 
			  { distname = TSTR(distnode->GetName());
			    ShowName(1);cancelled=FALSE;
				}
			if (hTarget==custnode) 
			  { custname = TSTR(custnode->GetName());
			    ShowName(0);cancelled=FALSE;
				}
			  BOOL notfound=TRUE;
			  for (int i=0;(i<nlist.Count())&&(notfound);i++)
				if (hTarget==nlist[i]) 
			      {notfound=FALSE;SetUpList();cancelled=FALSE;}
			  break;
			}		
		case REFMSG_NODE_LINK:		
		case REFMSG_CHANGE:
			{int pblst=0;
			  if (pblock && (pblock==hTarget))
			  { pblst=pblock->LastNotifyParamNum();
			    if (pblst==PB_METACOURSE)
			  	  return REF_STOP;
				if (pblst==PB_SPAWNLIFEVLUE)
					return REF_STOP;
			  }
			  MeshInvalid();
			  ParticleInvalid();
			  if (editOb==this) InvalidateUI();
			  cancelled=FALSE;
			}
			break;
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
			if (editOb==this) InvalidateUI();
			break;
		default: SimpleParticle::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}

const TCHAR *PArrayClassDesc::ClassName ()	{return GetString(IDS_RB_PARRAY);}
const TCHAR *PArrayClassDesc::Category ()	{return GetString(IDS_RB_PARTICLESYSTEMS);}
TCHAR *PArrayParticle::GetObjectName() {return GetString(IDS_RB_PARRAYGC);}

void PArrayParticle::SetUpList()
{ SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<nlist.Count(); i++) 
		SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)(nlist[i]->GetName()));
}

void PArrayParticle::AddToList(INode *newnode,int i,BOOL add)
{	if (add)
	{ nlist.Insert(i,1,&newnode);
	  MakeRefByID(FOREVER,BASER+i,newnode);
	}	  
	else ReplaceReference(i+BASER,newnode);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	SetUpList();
}

void PArrayParticle::DeleteFromList(int nnum,BOOL all)
{ int nCount=nlist.Count();
  INode *cnode=nlist[nnum];
  DeleteReference(nnum+BASER);
  if (theHold.Holding() && !TestAFlag(A_HELD)) 
	  theHold.Put(new PAObjectListRestore(this));
  nlist.Delete(nnum,1);
  if (all) 
  { for (int di=nnum;di<nlist.Count();di++)
     if (nlist[di]==cnode)
	 { DeleteReference(di+BASER);
	   nlist.Delete(di,1);
	 }
  }
  NotifyDependents(FOREVER,0,REFMSG_CHANGE);
  if (ip) SetUpList();
  valid=FALSE;
}
void PArrayParticle::SetUpLifeList()
{ TCHAR buffer[20];
  SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<llist.Count(); i++) 
  {	_itoa(llist[i], buffer, 10 );
	SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_ADDSTRING,0,(LPARAM)(TCHAR*)buffer);
  }
} 

void PArrayParticle::AddToLifeList(int newlife)
{	llist.Insert(llist.Count(),1,&newlife);
	SetUpLifeList();
}

void PArrayParticle::DeleteFromLifeList(int nnum)
{ 	llist.Delete(nnum,1);
    if (ip) SetUpLifeList();
}
int PArrayParticle::HitTest(
		TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt) 
	{	
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (doupdate) Update(t,inode);
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	DWORD rlim  = gw->getRndLimits();
	int res;

	gw->setTransform(ident);
	int ptype,disptype,dtype=GetDrawType(this,ptype,disptype);
	if ((dtype<2)||(dtype==3))
	{  if (parts.HitTest(gw,&hitRegion,flags&HIT_ABORTONHIT,GetMarkerType()))
		{return TRUE;}
	}
	else
	{ if ((t!=dispt)||doupdate||!dispmesh)
	  {	NullView nullView;
		BOOL needdel;
		if (dispmesh) delete dispmesh;
		SetAFlag(A_NOTREND);
	    dispmesh=GetRenderMesh(t,inode,nullView,needdel);
		ClearAFlag(A_NOTREND);
		}
		gw->setRndLimits(rlim);
		gw->setTransform(inode->GetObjectTM(t));
		res = dispmesh->select(gw, &particleMtl, &hitRegion, flags & HIT_ABORTONHIT);
		if (res) return TRUE;
	}
	
	if (EmitterVisible()) {
		gw->setRndLimits((rlim|GW_PICK|GW_WIREFRAME) 
			& ~(GW_ILLUM|GW_BACKCULL|GW_FLAT|GW_SPECULAR));
		gw->setTransform(inode->GetObjectTM(t));
		res = mesh.select(gw, &particleMtl, &hitRegion, flags & HIT_ABORTONHIT);

		gw->setRndLimits(rlim);
	} else {
		res = 0;
		}
	return res;
	}

int PArrayParticle::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{   if (!OKtoDisplay(t)) return 0;
	if (t!=tvalid) cancelled=FALSE;
	if ((ip && origmtl) &&(origmtl!=inode->GetMtl()))
	{ EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
	  origmtl=NULL;
	}
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (!doupdate) doupdate=CheckMtlChange(inode->GetMtl(),wasmulti);
	if (doupdate)
	 Update(t,inode);

	GraphicsWindow *gw = vpt->getGW();
	DWORD rlim  = gw->getRndLimits();
	// Draw emitter
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER) );	// removed BC on 4/29/99 DB
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_PARTICLE_EM));
    RECT *r=&vpt->GetDammageRect();
	if (EmitterVisible()) {
		gw->setTransform(inode->GetObjectTM(t));	
		r=&vpt->GetDammageRect();
		mesh.render(gw, &particleMtl, 
			(flags&USE_DAMAGE_RECT) ? r : NULL, COMP_ALL);
		}
		
	  Material *mtl = gw->getMaterial();	
	  if (!inode->Selected() && !inode->IsFrozen())
		gw->setColor( LINE_COLOR, mtl->Kd[0], mtl->Kd[1], mtl->Kd[2]);
	int ptype,disptype,type=GetDrawType(this,ptype,disptype);
	if (type==3)
	{ thePArrayDraw.obj = this;
		thePArrayDraw.firstpart=TRUE;
		thePArrayDraw.disptype=disptype;
		thePArrayDraw.ptype=ptype;
		thePArrayDraw.bb=TRUE;
		parts.SetCustomDraw(&thePArrayDraw);			
	}
	if ((type<2)||(type==3)) // Draw particles
	{ if (type<2) parts.SetCustomDraw(NULL);
	  MarkerType mt=(type==0?POINT_MRKR:PLUS_SIGN_MRKR);
	  gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL| (rlim&(GW_Z_BUFFER|GW_BOX_MODE)) );
	  gw->setTransform(ident);
	  parts.Render(gw,mt);
	  gw->setRndLimits(rlim);
	}
	else
	{ parts.SetCustomDraw(NULL);			
	  NullView nullView;
	  BOOL needdel;
	  if ((t!=dispt)||doupdate||!dispmesh)
		{
		if (dispmesh) delete dispmesh;
		SetAFlag(A_NOTREND);
		dispmesh=GetRenderMesh(t,inode,nullView,needdel);
		ClearAFlag(A_NOTREND);
		}
	  Matrix3 mat = inode->GetObjectTM(t);
	  gw->setRndLimits(rlim);
	  gw->setTransform(mat);
	  r=&vpt->GetDammageRect();
	  dispmesh->render( gw, inode->Mtls(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
		COMP_ALL | ((flags&DISP_SHOWSUBOBJECT)?COMP_OBJSELECTED:0),
		inode->NumMtls());
	   gw->setRndLimits(rlim);
	}
	return(0);
}

#endif // NO_PARTICLES_PARRAY