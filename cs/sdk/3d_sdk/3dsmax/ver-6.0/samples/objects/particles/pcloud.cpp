/**********************************************************************
 *<									  

	FILE: PCloud.CPP
	DESCRIPTION: Pcloud main code


	CREATED BY: Audrey Peterson	

	HISTORY: created February 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include <io.h>
#include "SuprPrts.h"
#include "iparamm.h"
#include "interpik.h"
#include "texutil.h"
#include "stdmat.h"
#include "macrorec.h"

// russom - 10/11/01
#ifndef NO_PARTICLES_PCLOUD

#define EPSILON 0.0001f
#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256
#define PARTICLE_SEED	0x8d6a65bc
#define PBLK		0
#define CUSTNODE 		1
#define DISTNODE 		2
#define MBASE	 		3
#define BASER			4
#define shownormname 6

typedef struct {
 float Ts,Ts0,LamTs,Mltvar;
 TimeValue L,showframe,DL,persist;
 Point3	V,W,RV;
 int themtl,gennum,SpVar;
 float Vsz,A,LamA,To,pvar;
} PCSavePt;
typedef struct{
} VelDir2;
typedef struct {
 float Size,VSz,VSpin,Phase,VPhase,Speed,VSpeed;
 float bstr,bstrvar,ToAmp,VToAmp;
 float ToPhase,VToPhase,VToPeriod,DirVar;
 int axisentered,direntered,sym;
 TimeValue Spin,ToPeriod,Life,Vl,persist;
float len,width,depth,axisvar,pvar;
 Point3 Ve,Dir,Axis;
 BirthPositionSpeed bps;
} VelDir;

static Class_ID PCLOUD_CLASS_ID(0x1c0f3d2f, 0x30310af9);

class PCPickOperand;
class PCPickNorm;
class PCloudParticle;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class PCloudParticleDraw : public CustomParticleDisplay {
	public:
		BOOL firstpart;
		PCloudParticle *obj;
		int disptype,ptype,bb,anifr,anioff;
		boxlst *bboxpt;
		TimeValue t;
		InDirInfo indir;

		PCloudParticleDraw() {obj=NULL;}
		BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i);
	};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class PCloudParticle : public SimpleParticle, IParamArray, RandGeneratorParticles {
	public:
		PCloudParticleDraw thePCloudDraw;
		PCloudParticle();
		~PCloudParticle();
		static IParamMap *pmapParam;
		static IParamMap *pmapPGen;
		static IParamMap *pmapPType;
		static IParamMap *pmapPSpin;
		static IParamMap *pmapEmitV;
		static IParamMap *pmapBubl;
		static IParamMap *pmapSpawn;
		static int createmeth;
		static HWND hbubl;
		int stepSize,size,maincount,fragflags;
		static custsettings;
		Mesh *cmesh,*dispmesh;
		Box3 *cmbb;
		TimeValue dispt;
		INode *custnode,*distnode,*cnode,*mbase;
		TSTR custname,distname,normname;
		DWORD flags;
		ULONG dflags;
		BOOL cancelled,wasmulti;
		static BOOL creating;
		static PCPickOperand pickCB;
		static PCPickNorm pickNorm;
		Mtl *origmtl;

		Point3 boxcenter;
		int CustMtls;
		Tab<int> nmtls;
		TimeLst times;
		void GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype);
		void TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void GetMesh(TimeValue t,int subtree,int custmtl);
		void GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void GetallBB(INode *custnode,int subtree,TimeValue t);

		void SetUpList();
		void AddToList(INode *newnode,int i,BOOL add);
		void DeleteFromList(int nnum,BOOL all);
		Tab<INode*> nlist;
		Tab<int> llist;
		int deftime;
		int NumRefs() {return BASER+nlist.Count();}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
		void SetUpLifeList();
		void AddToLifeList(int newlife);
		void DeleteFromLifeList(int nnum);
		void ShowName(int dist);
		static AName *NameLst;
		static HWND hexts,hParams2,hptype,hgen,hparam,hrot,spawn;
		static ICustEdit *custCtrlEdit;

		void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box);
		int CountLive();
		int rcounter,vcounter;
		oldipc lc;
		static IObjParam *ip;
		// Animatable methods		
		void DeleteThis() {delete this;}
		void MapKeys(TimeMap *map,DWORD flags);
		int RenderBegin(TimeValue t, ULONG flags);		
		int RenderEnd(TimeValue t);

		// From SimpleParticle
		Interval GetValidity(TimeValue t);		
		BOOL EmitterVisible();		
		MarkerType GetMarkerType();	
		PCSavePt *sdata;
//		unsigned int rseed; // don't need it anymore because of RandGenerator

		// From BaseObject
		int IsInstanceDependent() {return 1;}
		void GetFilename(TCHAR *filename);
		void SetupTargetList();
		int SaveSettings(int overwrite,TCHAR *newname);
		int GetSettings(int setnum,TCHAR *newname);
		int RemSettings(int setnum,TCHAR *newname);
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		void AssignMtl(INode *node,INode *topnode,int subtree,TimeValue t);
		void DoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void RetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void GetSubs(INode *node,INode *topnode,int subtree,TimeValue t);
		void CntDoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		void CntRetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t);
		BOOL backpatch;
		Mtllst mttab;
		TimeValue ParticleLife(TimeValue t, int i);
		Point3 ParticlePosition(TimeValue t,int i);
		Point3 ParticleVelocity(TimeValue t,int i);		
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);
		void DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emits);

		CreateMouseCallBack* GetCreateMouseCallBack();

		TCHAR *GetObjectName();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void BuildEmitter(TimeValue t, Mesh& amesh);
		void BirthParticle(INode *node,TimeValue bt,int num,VelDir *ptvel,Mesh* amesh,Point3* fnorms,Matrix3 disttm);
		BOOL ComputeParticleStart(TimeValue t0,int c);
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		// Animatable methods		
		Class_ID ClassID() {return PCLOUD_CLASS_ID;} 
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From Simple Particle
		void UpdateParticles(TimeValue t,INode *node);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI();
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt);
		void MovePart(int j,TimeValue dt,BOOL fullframe,int tpf);
		void ResetSystem(TimeValue t,BOOL full=TRUE);
	};
//--- ClassDescriptor and class vars ---------------------------------

class PCloudClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new PCloudParticle;}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return PCLOUD_CLASS_ID;}
	const TCHAR* 	Category(); 
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	void			ResetClassParams(BOOL fileReset);
	};

static PCloudClassDesc PCloudDesc;
ClassDesc* GetPCloudDesc() {return &PCloudDesc;}

void PCloudClassDesc::ResetClassParams(BOOL fileReset)
	{
	PCloudParticle::createmeth=0;
	}

class PCPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PCloudParticle *po;
		int dodist,repi;
		
		PCPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};
class PCPickNorm : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PCloudParticle *po;
		
		PCPickNorm() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};
class PCObjectListRestore : public RestoreObj {
	public:   		
		PCloudParticle *po;
		Tab<INode*> unodes;
		Tab<INode*> rnodes;
		int lnum,lnum2;
		PCObjectListRestore(PCloudParticle *p) 
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

class PCLifeListRestore : public RestoreObj {
	public:   		
		PCloudParticle *po;
		Tab<int> utimes;
		Tab<int> rtimes;
		PCLifeListRestore(PCloudParticle *p) 
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


class CreatePCPartDelNode : public RestoreObj {
	public:   		
		PCloudParticle *obj;
		TSTR name;
		int dist;
		CreatePCPartDelNode(PCloudParticle *o, TSTR n,int d) {
			obj = o; name=TSTR(n); dist=d;
			}
		void Restore(int isUndo)
		{if (dist==shownormname) obj->normname=name;
		else if (dist) obj->distname=name;
		else obj->custname = name;
		 obj->ShowName(dist);
			}
		void Redo() 
			{ if (dist==shownormname) obj->normname=TSTR(_T(""));
			  else if (dist) obj->distname=TSTR(_T(""));
			  else obj->custname = TSTR(_T(""));
		 TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + TSTR(GetString(IDS_AP_NONE));
		  if (dist==shownormname)
		  { if (obj->hgen) SetWindowText(GetDlgItem(obj->hgen, IDC_AP_PCLOUDOBJDIRNAME), name);}
		  if (dist)
		  { if (obj->hparam)SetWindowText(GetDlgItem(obj->hparam, IDC_AP_FILLPICKOBJECT), name);}
		  else
		  { if (obj->hptype) SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);}
		}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};
class CreatePCPartPickNode : public RestoreObj {
	public:   		
		PCloudParticle *obj;
		TSTR name,name2;int dist;
		CreatePCPartPickNode(PCloudParticle *o, TSTR n,TSTR n1,int d) {
			obj = o; name=TSTR(n);name2=TSTR(n1);dist=d;
			}
		void Restore(int isUndo) {
		   if (dist==shownormname)
		   { if (obj->normname) obj->normname=name;
		   else obj->normname=TSTR(_T(""));}
			else if (dist)
			{ if (obj->distnode) obj->distname = name;
			else  obj->distname=TSTR(_T(""));}
			else
			{ if (obj->custnode) obj->custname = name;
			else  obj->custname=TSTR(_T(""));}
			obj->ShowName(dist);
			}
		void Redo() 
		{ if (((dist)&&(obj->hptype))||((!dist)&&(obj->hparam)))
		{ TSTR name;
		  if (dist==shownormname) obj->normname=name2;
		  else if (dist) obj->distname=name2;
		  else  obj->custname = name2;
		  name=TSTR(GetString(IDS_AP_OBJECTSTR))+(_tcslen(name2)>0 ? name2 : TSTR(GetString(IDS_AP_NONE)));
		  if (dist==shownormname)
		  {if (obj->hgen) SetWindowText(GetDlgItem(obj->hgen, IDC_AP_PCLOUDOBJDIRNAME), name);}
		  else if (dist)
		  {  if (obj->hparam) SetWindowText(GetDlgItem(obj->hparam, IDC_AP_FILLPICKOBJECT), name);}
		  else
		    if (obj->hptype) SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);
		}
			}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};


//--- ClassDescriptor and class vars ---------------------------------
IParamMap *PCloudParticle::pmapParam;
IParamMap *PCloudParticle::pmapPGen;
IParamMap *PCloudParticle::pmapPType;
IParamMap *PCloudParticle::pmapPSpin;
IParamMap *PCloudParticle::pmapEmitV;
IParamMap *PCloudParticle::pmapBubl;
IParamMap *PCloudParticle::pmapSpawn;
IObjParam *PCloudParticle::ip    = NULL;
BOOL PCloudParticle::creating    = FALSE;
PCPickOperand PCloudParticle::pickCB;
PCPickNorm PCloudParticle::pickNorm;
ICustEdit *PCloudParticle::custCtrlEdit=NULL;
int PCloudParticle::custsettings=0;
AName *PCloudParticle::NameLst=NULL;
HWND PCloudParticle::hexts;
HWND PCloudParticle::hParams2;
HWND PCloudParticle::hgen;
HWND PCloudParticle::hptype;
HWND PCloudParticle::hparam;
HWND PCloudParticle::hrot;
HWND PCloudParticle::hbubl;
HWND PCloudParticle::spawn;
int PCloudParticle::createmeth=0;


#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)

//--- Parameter map/block descriptors -------------------------------

#define PB_CREATEIN			0
#define PB_SPEED			1
#define PB_SPEEDVAR			2
#define PB_SPEEDDIR			3
#define PB_DIRX				4
#define PB_DIRY				5
#define PB_DIRZ				6
#define PB_DIRVAR			7

#define PB_BIRTHMETHOD		8
#define PB_PBIRTHRATE		9
#define PB_PTOTALNUMBER		10
#define PB_EMITSTART		11
#define PB_EMITSTOP			12
#define PB_DISPUNTIL		13
#define PB_LIFE				14
#define PB_LIFEVAR			15
#define PB_CUSTOMMTL		16
#define PB_SIZE				17
#define PB_SIZEVAR			18
#define PB_GROWTIME			19
#define PB_FADETIME			20
#define PB_RNDSEED			21
#define PB_EMITRWID			22
#define PB_EMITRHEIGHT		23
#define PB_EMITRDEPTH		24

#define PB_EMITRHID			25

#define PB_PARTICLECLASS	26
#define PB_PARTICLETYPE		27
#define PB_METATENSION		28
#define PB_METATENSIONVAR	29
#define PB_METACOURSE		30
#define PB_METAAUTOCOARSE	31
#define PB_USESUBTREE		32
#define PB_ANIMATIONOFFSET  33
#define PB_OFFSETAMOUNT     34
#define PB_VIEWPORTSHOWS	35
#define PB_DISPLAYPORTION	36
#define PB_MAPPINGTYPE		37
#define PB_MAPPINGTIME		38
#define PB_MAPPINGDIST		39
						
#define PB_SPINTIME			40
#define PB_SPINTIMEVAR		41
#define PB_SPINPHASE		42
#define PB_SPINPHASEVAR		43
#define PB_SPINAXISTYPE		44
#define PB_SPINAXISX		45
#define PB_SPINAXISY		46
#define PB_SPINAXISZ		47
#define PB_SPINAXISVAR		48

#define PB_EMITVINFL		49
#define PB_EMITVMULT		50
#define PB_EMITVMULTVAR		51

#define PB_BUBLAMP			52
#define PB_BUBLAMPVAR		53
#define PB_BUBLPER			54
#define PB_BUBLPERVAR		55
#define PB_BUBLPHAS			56
#define PB_BUBLPHASVAR		57

#define PB_STRETCH			58

#define PB_SPAWNTYPE		59
#define PB_SPAWNGENS		60
#define PB_SPAWNCOUNT		61
#define PB_SPAWNDIRCHAOS	62
#define PB_SPAWNSPEEDCHAOS	63
#define PB_SPAWNSPEEDSIGN	64
#define PB_SPAWNINHERITV	65
#define PB_SPAWNSCALECHAOS	66
#define PB_SPAWNSCALESIGN	67
#define PB_SPAWNLIFEVLUE	68
#define PB_SPAWNSPEEDFIXED	69
#define PB_SPAWNSCALEFIXED	70
#define PB_METACOURSEV		71
#define PB_SPAWNPERCENT		72
#define PB_SPAWNMULTVAR		73
#define PB_PCNOTDRAFT		74
#define PB_SPAWNDIEAFTER	75
#define PB_SPAWNDIEAFTERVAR	76

#define PB_PCIPCOLLIDE_ON		77
#define PB_PCIPCOLLIDE_STEPS	78
#define PB_PCIPCOLLIDE_BOUNCE	79
#define PB_PCIPCOLLIDE_BOUNCEVAR 80


// render types
#define RENDMETA    8
#define RENDGEOM	9

//
//
// Parameters

#define ISSTD 0
#define METABALLS 1
#define INSTGEOM 2

static int countIDs[] = {IDC_SP_GENUSERATE,IDC_SP_GENUSETTL};
static int createIDs[] = {IDC_AP_FILLBOX,IDC_AP_FILLSPHERE,IDC_AP_FILLCYLINDER,IDC_AP_FILLOBJECT};

static int particleclassIDs[] = {IDC_SP_TYPESTD,IDC_SP_TYPEMET,IDC_SP_TYPEINSTANCE};

static int particletypeIDs[] = {IDC_SP_TYPETRI,IDC_SP_TYPECUB,IDC_SP_TYPESPC,IDC_SP_TYPEFAC,
								IDC_SP_TYPEPIX,IDC_SP_TYPETET,IDC_SP_TYPE6PNT,IDC_SP_TYPESPHERE};

static int viewportoptionIDs[] = {IDC_SP_VIEWDISPDOT,IDC_SP_VIEWDISPTIK,IDC_SP_VIEWDISPMESH,IDC_SP_VIEWDISPBOX};

static int mappingIDs[] = {IDC_SP_MAPTIME,IDC_SP_MAPDIST};

static int spindirectionIDs[] = {IDC_AP_PARTICLEDIRRND,IDC_AP_PARTICLEDIRTRAVL,IDC_AP_PARTICLEDIRUSER};
static int particleaxisIDs[] = {IDC_AP_PCLOUDDIRRND,IDC_AP_PCLOUDDIRENTER,IDC_AP_PCLOUDDIROBJ};

#define DIRTRAVEL 1

static int animateoffsetIDs[] = {IDC_AP_NOANIOFF,IDC_AP_ANIOFFBIRTH,IDC_AP_ANIOFFRND};
static int custmtlIDs[] = {IDC_SP_MAPCUSTOMEMIT,IDC_SP_MAPCUSTOMINST};

// Dialog Unique to Particle Array
static ParamUIDesc descParamPCloud[] = {

	// Distribution Method
	ParamUIDesc(PB_CREATEIN,TYPE_RADIO,createIDs,4),

	// Emitter Width
	ParamUIDesc(
		PB_EMITRWID,
		EDITTYPE_UNIVERSE,
		IDC_AP_EMITRADWID,IDC_AP_EMITRADWIDSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Emitter Height
	ParamUIDesc(
		PB_EMITRHEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_AP_EMITHGT,IDC_AP_EMITHGTSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Emitter Depth
	ParamUIDesc(
		PB_EMITRDEPTH,
		EDITTYPE_UNIVERSE,
		IDC_AP_EMITDEP,IDC_AP_EMITDEPSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Hide Emitter
	ParamUIDesc(PB_EMITRHID,TYPE_SINGLECHEKBOX,IDC_SP_EMITHID),

	// Viewport Shows
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

#define PARAMPCLOUD_LENGTH 7

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

	// Enter Axis Control
	ParamUIDesc(PB_SPEEDDIR,TYPE_RADIO,particleaxisIDs,3),

	// X-Axis
	ParamUIDesc(
		PB_DIRX,
		EDITTYPE_FLOAT,
		IDC_AP_MOVEDIRX,IDC_AP_MOVEDIRXSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Y-Axis
	ParamUIDesc(
		PB_DIRY,
		EDITTYPE_FLOAT,
		IDC_AP_MOVEDIRY,IDC_AP_MOVEDIRYSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Z-Axis
	ParamUIDesc(
		PB_DIRZ,
		EDITTYPE_FLOAT,
		IDC_AP_MOVEDIRZ,IDC_AP_MOVEDIRZSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Particle Move Var
	ParamUIDesc(
		PB_DIRVAR,
		EDITTYPE_FLOAT,
		IDC_AP_MOVEDIRVAR,IDC_AP_MOVEDIRVARSPIN,
		0.0f,100.0f,
		SPIN_AUTOSCALE,stdPercentDim),

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
		0.0f,100.0f,
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

#define PARAMPGEN_LENGTH 20

// Particle Type for PCloud
static ParamUIDesc descParamPType[] = {

	// Particle Class
	ParamUIDesc(PB_PARTICLECLASS,TYPE_RADIO,particleclassIDs,3),

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
	ParamUIDesc(PB_PCNOTDRAFT,TYPE_SINGLECHEKBOX,IDC_SP_DRAFTMODE),

	// Use Subtree Checkbox
	ParamUIDesc(PB_USESUBTREE,TYPE_SINGLECHEKBOX,IDC_AP_USESUBTREE),

	// Display type
	ParamUIDesc(PB_ANIMATIONOFFSET,TYPE_RADIO,animateoffsetIDs,3),

	// Animation Offset Amount
	ParamUIDesc(
		PB_OFFSETAMOUNT,
		EDITTYPE_TIME,
		IDC_AP_ANIRNDFR,IDC_AP_ANIRNDFRSPIN,
		1.0f,999999999.0f,
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

	// Use Custom Mtl
	ParamUIDesc(PB_CUSTOMMTL,TYPE_RADIO,custmtlIDs,2),
};
#define PARAMPTYPE_LENGTH 15

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
	ParamUIDesc(PB_PCIPCOLLIDE_ON,TYPE_SINGLECHEKBOX,IDC_INTERP_BOUNCEON),

	// IPC Steps
	ParamUIDesc(
		PB_PCIPCOLLIDE_STEPS,
		EDITTYPE_INT,
		IDC_INTERP_NSTEPS,IDC_INTERP_NSTEPSSPIN,
		1.0f,1000.0f,
		1.0f),

	// IPC Bounce
	ParamUIDesc(
		PB_PCIPCOLLIDE_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCE,IDC_INTERP_BOUNCESPIN,
		0.0f,10000.0f,
		1.0f,
		stdPercentDim),

	// IPC Bounce
	ParamUIDesc(
		PB_PCIPCOLLIDE_BOUNCEVAR,
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

	ParamUIDesc(
		PB_SPAWNDIEAFTER,
		EDITTYPE_TIME,
		IDC_AP_MAXSPAWNDIEAFTER,IDC_AP_MAXSPAWNDIEAFTERSPIN,
		0.0f,999999999.0f,
		10.0f),

	// Spawn Generations
	ParamUIDesc(
		PB_SPAWNDIEAFTERVAR,
		EDITTYPE_FLOAT,
		IDC_AP_MAXSPAWNDIEAFTERVAR,IDC_AP_MAXSPAWNDIEAFTERVARSPIN,
		0.0f,100.0f,
		0.01f,
		stdPercentDim),

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
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
};
static ParamBlockDescID spdescVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 71 }, // Meta courseness viewport
};
static ParamBlockDescID spdescVer2[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 71 }, // Meta courseness viewport
	{ TYPE_INT, NULL, FALSE, 72 }, // Spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 73 }, // Spawn mult var
};
static ParamBlockDescID spdescVer3[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 71 }, // Meta courseness viewport
	{ TYPE_INT, NULL, FALSE, 72 }, // Spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 73 }, // Spawn mult var
	{ TYPE_INT, NULL, FALSE, 74 }, // Not Draft
};
static ParamBlockDescID spdescVer4[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 71 }, // Meta courseness viewport
	{ TYPE_INT, NULL, FALSE, 72 }, // Spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 73 }, // Spawn mult var
	{ TYPE_INT, NULL, FALSE, 74 }, // Not Draft
	{ TYPE_INT, NULL, TRUE, 75 }, // die after
	{ TYPE_FLOAT, NULL, TRUE, 76 }, // die after var
};
static ParamBlockDescID spdescVer5[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	 // distribution method
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // speed var
	{ TYPE_INT, NULL, FALSE, 3 },	 // vel dir
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // x
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // y
	{ TYPE_FLOAT, NULL, TRUE, 6 },	 // z
	{ TYPE_FLOAT, NULL, TRUE, 7 },	 // var

	{ TYPE_INT, NULL, FALSE, 8 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 9 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 10},    // total number
	{ TYPE_INT, NULL, FALSE, 11 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 12 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 13 },   // display until
	{ TYPE_INT, NULL, TRUE, 14 },	 // life
	{ TYPE_INT, NULL, TRUE, 15 },	 // life var
	{ TYPE_INT, NULL, FALSE, 16 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 18 },	 // size var
	{ TYPE_INT, NULL, FALSE, 19 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 20 },    // fade time
	{ TYPE_INT, NULL, FALSE, 21 }, // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // emitter width
	{ TYPE_FLOAT, NULL, TRUE, 23 },  // emitter height
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // emitter depth
	{ TYPE_INT, NULL, FALSE, 25 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 26 },  // particle class
	{ TYPE_INT, NULL, FALSE, 27 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 28 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 29 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 30 }, // meta course
	{ TYPE_INT, NULL, FALSE, 31}, // auto coarseness
    { TYPE_INT, NULL, FALSE, 32 }, // use subtree
    { TYPE_INT, NULL, FALSE, 33 }, // animation offset
    { TYPE_INT, NULL, FALSE, 34 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 35 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 36 }, // display portion
	{ TYPE_INT, NULL, FALSE, 37 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 38 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 39 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 40 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 41 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 42 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 43 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 44 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 46 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 47 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 48 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 49 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 50 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 51 },  // emit mult var

	{ TYPE_FLOAT, NULL, TRUE, 52},  // bubble amp
	{ TYPE_FLOAT, NULL, TRUE, 53 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 54 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 55 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 56 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 57 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 58 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 59 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 60 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 61 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 62 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 63 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 64 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 65 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 66 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 67 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 68 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 69 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 70 }, // constant spawn scale
	{ TYPE_FLOAT, NULL, FALSE, 71 }, // Meta courseness viewport
	{ TYPE_INT, NULL, FALSE, 72 }, // Spawn percent
	{ TYPE_FLOAT, NULL, FALSE, 73 }, // Spawn mult var
	{ TYPE_INT, NULL, FALSE, 74 }, // Not Draft
	{ TYPE_INT, NULL, TRUE, 75 }, // die after
	{ TYPE_FLOAT, NULL, TRUE, 76 }, // die after var
	{ TYPE_INT, NULL, FALSE, 77 },  // IPC Enable
	{ TYPE_INT, NULL, FALSE, 78 },  // IPC Steps
	{ TYPE_FLOAT, NULL, TRUE, 79 },  // IPC Bounce
	{ TYPE_FLOAT, NULL, TRUE, 80 },  // IPC Bounce Var
};

#define PBLOCK_LENGTH_PCLOUD 81

static ParamVersionDesc pcversions[] = {
	ParamVersionDesc(spdescVer0,71,0),
	ParamVersionDesc(spdescVer1,72,1),
	ParamVersionDesc(spdescVer2,74,2),
	ParamVersionDesc(spdescVer3,75,3),
	ParamVersionDesc(spdescVer4,77,4),
	};
#define NUM_OLDVERSIONS	5

// Current version
#define CURRENT_VERSION	5
static ParamVersionDesc curVersionPC(spdescVer5,PBLOCK_LENGTH_PCLOUD,CURRENT_VERSION);

//-- ParticleDlgProc ------------------------------------------------

class CreatePCloudProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		PCloudParticle *CloudObject;
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
		
		CreatePCloudProc()
			{
			ignoreSelectionChange = FALSE;
			}
//		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEPCLOUDMODE	CID_USER +14

class CreatePCloudMode : public CommandMode {		
	public:		
		CreatePCloudProc proc;
		IObjParam *ip;
		PCloudParticle *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,PCloudParticle*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEPCLOUDMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreatePCloudMode theCreatePCloudMode;

RefResult CreatePCloudProc::NotifyRefChanged(
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
		 	if ( CloudObject && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (CloudObject && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (CloudObject->creating) 
						{  theCreatePCloudMode.JumpStart(CloudObject->ip,CloudObject);
							createInterface->SetCommandMode(&theCreatePCloudMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				CloudObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				CloudObject  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreatePCloudProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	CloudObject       = NULL;
	CreateNewObject();
	}
void CreatePCloudProc::CreateNewObject()
	{
	SuspendSetKeyMode();
  createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	lastPutCount  = theHold.GetGlobalPutCount();
	CloudObject = (PCloudParticle*)cDesc->Create();
	
	// Start the edit params process
	if ( CloudObject ) {
		CloudObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#ifdef _OSNAP
		CloudObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreatePCloudProc::End()
{ if ( CloudObject ) 
	{ 
#ifdef _OSNAP
		CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		CloudObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			createInterface->GetMacroRecorder()->Cancel();
			delete CloudObject;
			CloudObject = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();}
	}
}

void CreatePCloudMode::JumpStart(IObjParam *i,PCloudParticle *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int PCloudClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreatePCloudMode.Begin(iob,this);
	iob->PushCommandMode(&theCreatePCloudMode);
	return TRUE;
	}

int PCloudClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreatePCloudMode.End();
	i->RemoveMode(&theCreatePCloudMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreatePCloudProc::proc(HWND hwnd,int msg,int point,int flag,
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
					assert( CloudObject );					
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
						CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		CloudObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						// Get rid of the reference.
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}
						// new object
						CreateNewObject();   // creates CloudObject
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( CloudObject);
					attachedToNode = TRUE;
					assert( CloudNode );					
					createCB = CloudObject->GetCreateMouseCallBack();
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();
					mat.IdentityMatrix();
				default:				
					if (createCB) {						
						res = createCB->proc(vpx,msg,point,flag,m,mat);
//						macroRec->Disable();   // 10/00
						createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
//						macroRec->Enable();

						if (res==CREATE_ABORT)
							goto abort;
						if (res==CREATE_STOP){
#ifdef _OSNAP
                         CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
						    theHold.Accept(GetString(IDS_DS_CREATE));	 
						}
						createInterface->RedrawViews(createInterface->GetTime());   //DS
						}

					break;
					
				}			
			break;

		case MOUSE_MOVE:
			if (createCB) {				
				res = createCB->proc(vpx,msg,point,flag,m,mat);
//				macroRec->Disable();   // 10/00
				createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
//				macroRec->Enable();
				if (res==CREATE_ABORT) 
					goto abort;
				if (res==CREATE_STOP){
#ifdef _OSNAP
				CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
					theHold.Accept(GetString(IDS_DS_CREATE));	// TH
				}
				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_INTERACTIVE);		//DS		
				}
			break;

	case MOUSE_PROPCLICK:
		createInterface->SetStdCommandMode(CID_OBJMOVE);
		break;
		case MOUSE_ABORT: 
	if (createCB)
	{ res = createCB->proc(vpx,msg,point,flag,m,mat);
//		macroRec->Disable();   // 10/00
	  createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
//		macroRec->Enable();
	  if (res==CREATE_ABORT)
	      goto abort;
	  if (res==CREATE_STOP){
#ifdef _OSNAP
         CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	    theHold.Accept(GetString(IDS_DS_CREATE));	
	  }
	  createInterface->RedrawViews(createInterface->GetTime()); 
		break;
	}
	abort:
		assert( CloudObject );
#ifdef _OSNAP
		CloudObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		CloudObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
		// Toss the Undo stack if any parameters have changed.
		theHold.Cancel();	 // deletes both the Cloudera and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
			GetSystemSetting(SYSSET_CLEAR_UNDO);
		CloudObject=NULL;
		createInterface->RedrawViews(createInterface->GetTime());
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
		break;
	
		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
#ifdef _OSNAP  //PREVIEW SNAP
		 if (createCB) res =createCB->proc(vpx,msg,point,flag,m,mat);
#endif
		vpx->TrackImplicitGrid(m);
			break;
	}

	done:
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}

void EnterDir(HWND hWnd,BOOL ison,BOOL pickon=FALSE)
{ if (ison)
  { SpinnerOn(hWnd,IDC_AP_MOVEDIRXSPIN,IDC_AP_MOVEDIRX);
	SpinnerOn(hWnd,IDC_AP_MOVEDIRYSPIN,IDC_AP_MOVEDIRY);
	SpinnerOn(hWnd,IDC_AP_MOVEDIRZSPIN,IDC_AP_MOVEDIRZ);
  }
  else
  { SpinnerOff(hWnd,IDC_AP_MOVEDIRXSPIN,IDC_AP_MOVEDIRX);
	SpinnerOff(hWnd,IDC_AP_MOVEDIRYSPIN,IDC_AP_MOVEDIRY);
	SpinnerOff(hWnd,IDC_AP_MOVEDIRZSPIN,IDC_AP_MOVEDIRZ);
  }
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MOVEDIRX_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MOVEDIRY_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MOVEDIRZ_TXT),ison);
  if ((ison)||(pickon)) 
	SpinnerOn(hWnd,IDC_AP_MOVEDIRVARSPIN,IDC_AP_MOVEDIRVAR);
  else
	SpinnerOff(hWnd,IDC_AP_MOVEDIRVARSPIN,IDC_AP_MOVEDIRVAR);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MOVEDIRVAR_TXT),(ison)||(pickon));
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MOVEDIRVAR_PCNT),(ison)||(pickon));
  TurnButton(hWnd,IDC_AP_OBJECTDIRPICK,pickon);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_PCLOUDOBJDIRNAME),pickon);
}

void PCCheckInstButtons(IParamBlock *pblock,HWND hptype)
{ int isinst;
  pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
  float width;
  pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
  TurnButton(hptype,IDC_AP_OBJECTPICK,((isinst==INSTGEOM)&&(width>=0.01f)));
}
void PCCheckLifeButtons(int stype,HWND spawn)
{ int rep;
  rep = SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_GETCURSEL,0,0);
  TurnButton(spawn,IDC_AP_LIFEQUEUEADD,(stype>1));
  TurnButton(spawn,IDC_AP_LIFEQUEUEREPL,(stype>1)&&(rep>-1));
  TurnButton(spawn,IDC_AP_LIFEQUEUEDEL,(stype>1)&&(rep>-1));
}
void PCCheckSpawnButtons(IParamBlock *pblock,HWND spawn,int repi)
{ int stype;
  pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  EnableWindow(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),(stype>1));
  TurnButton(spawn,IDC_AP_OBJECTQUEUEPICK,(stype>1));
  TurnButton(spawn,IDC_AP_OBJQUEUEREPLACE,(stype>1)&&(repi>-1));
  TurnButton(spawn,IDC_AP_OBJQUEUEDELETE,(stype>1)&&(repi>-1));
  PCCheckLifeButtons(stype,spawn);
}

void PCCheckPickButtons(IParamBlock *pblock,HWND hptype,HWND spawn,int repi)
{ PCCheckInstButtons(pblock,hptype);
  PCCheckSpawnButtons(pblock,spawn,repi);
}

class PCloudEmitterCreateCallback : public CreateMouseCallBack {
	public:
		PCloudParticle *po;
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		BOOL square;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int PCloudEmitterCreateCallback::proc(
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
	Point3 d;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point)  {
			case 0:
				// if hidden by category, re-display particles and objects
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				po->pblock->SetValue(PB_EMITRWID,0,0.01f);
				po->pblock->SetValue(PB_EMITRHEIGHT,0,0.01f);
				po->pblock->SetValue(PB_EMITRDEPTH,0,0.01f);
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
				po->pmapParam->Invalidate();
				break;

			case 1: 
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				if ((po->createmeth==1)||(po->createmeth==2))
				{ d = p1-p0;}
				else
				{p1.z = p0.z +(float).01; 
				if((flags&MOUSE_CTRL) ||(po->createmeth==3))
				{ mat.SetTrans(p0);	} 
				else mat.SetTrans(float(.5)*(p0+p1));
				d = p1-p0;
				square = FALSE;
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
					}
				}
				po->pblock->SetValue(PB_EMITRWID,0,float(fabs(d.y)));
				po->pblock->SetValue(PB_EMITRHEIGHT,0,float(fabs(d.x)));

				if (msg==MOUSE_POINT)
				{ if (Length(sp1-sp0)<3 || Length(d)<0.1f) 
				   return CREATE_ABORT;	
				  else if ((po->createmeth==1)||(po->createmeth==3))
				  {	TurnButton(po->hptype,IDC_AP_OBJECTPICK,TRUE);
					TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,TRUE);
					TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,TRUE);
					if (po->pickCB.repi>-1)
					TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,TRUE);
					po->pblock->SetValue(PB_EMITRDEPTH,0,float(fabs(d.y)));
				    po->pmapParam->Invalidate();
					 return CREATE_STOP;
				  }
				}
				po->pblock->SetValue(PB_EMITRDEPTH,0,float(fabs(d.z)));
				po->pmapParam->Invalidate();
				break;
			case 2:
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
				if (po->createmeth==2)
				{ Point3 tmp=p0;tmp.z=float(.5)*(p0.z+p1.z);
				  mat.SetTrans(tmp);
				}
				else if (!square)
				{ mat.SetTrans(float(.5)*(p0+p1)); }

				d = p1-p0;
				if (square) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;					
					}

				po->pblock->SetValue(PB_EMITRWID,0,float(fabs(d.y)));
				po->pblock->SetValue(PB_EMITRHEIGHT,0,float(fabs(d.x)));
				po->pblock->SetValue(PB_EMITRDEPTH,0,float(fabs(d.z)));
				po->pmapParam->Invalidate();				
					
				if (msg==MOUSE_POINT) {	
					PCCheckPickButtons(po->pblock,po->hptype,po->spawn,po->pickCB.repi);
				TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,TRUE);
				return CREATE_STOP;	}
				break;

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	return 1;
	}

static PCloudEmitterCreateCallback pcloudemitterCallback;

CreateMouseCallBack* PCloudParticle::GetCreateMouseCallBack() 
	{
	pcloudemitterCallback.po = this;
	return &pcloudemitterCallback;
	}

class PCParticleGenDlgProc : public ParamMapUserDlgProc {
	public:
		PCloudParticle *po;
		ICustButton *iBut;

		PCParticleGenDlgProc(PCloudParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PCParticleGenDlgProc::Update(TimeValue t)
{	if (!po->editOb) return;
    int birthmeth;
	po->pblock->GetValue(PB_BIRTHMETHOD,0,birthmeth,FOREVER);
	if (birthmeth)
	{ SpinnerOff(po->hgen,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
	  SpinnerOn(po->hgen,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
	}else 
	{ SpinnerOn(po->hgen,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
	  SpinnerOff(po->hgen,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
	}
	int ison; 
	po->pblock->GetValue(PB_SPEEDDIR,0,ison,FOREVER);
	EnterDir(po->hgen,ison==1,ison==2);
	po->ShowName(shownormname);
}

BOOL PCParticleGenDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{  int acourse;
       float size;
      switch (msg) 
	  { case WM_INITDIALOG: {
			iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_OBJECTDIRPICK));
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
					   po->pmapPType->Invalidate();
					 }
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
				case IDC_AP_NEWSEED:
					{ srand( (unsigned)time( NULL ) );
					  int newseed=rand() % 25001;
					  po->pblock->SetValue(PB_RNDSEED,0,newseed);
					  po->pmapPGen->Invalidate();
					  break;
					}
				case IDC_AP_PCLOUDDIRRND:
				case IDC_AP_PCLOUDDIRENTER:
				case IDC_AP_PCLOUDDIROBJ:
				   { int ison; 
					po->pblock->GetValue(PB_SPEEDDIR,0,ison,FOREVER);
					EnterDir(hWnd,ison==1,ison==2);
			        return TRUE;
				   }
				case IDC_AP_OBJECTDIRPICK:
				    if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePCloudMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePCloudMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickNorm.po = po;						
						  po->ip->SetPickMode(&po->pickNorm);
						}
					break;
			}
	  }
	return FALSE;
	}

void PCCheckStretchBox(HWND hWnd,PCloudParticle *po)
{ if (IsStdMtl(po->cnode))
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),TRUE);
  } else EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
  SpinnerOn(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),TRUE);
  po->pmapPSpin->Invalidate();
}

void PCStretchStuff(int dir,int fragflags,HWND hWnd,PCloudParticle *po)
{ if (dir==0)
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,FALSE,fragflags!=METABALLS);
  }
  else if (dir==1)
  { PCCheckStretchBox(hWnd,po);
	SpinStuff(hWnd,FALSE,fragflags!=METABALLS);
  }  
  else if (dir==2)
  {	EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,TRUE,fragflags!=METABALLS);
  }
}

void CourseCheck(PCloudParticle *po,HWND hWnd,TimeValue t)
{ int acourse;
  float size;
  po->pblock->GetValue(PB_METAAUTOCOARSE,t,acourse,FOREVER);
  if (acourse) 
  { float mc1,mc2,mc,mcv;
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
void PCSetMapVals(IParamBlock *pblock,HWND hWnd,TimeValue t)
{ int type,maptype;
  pblock->GetValue(PB_PARTICLETYPE,t,type,FOREVER);
  if (type==RENDTET)
  { SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
	SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
 	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
	EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
 }
  else
  { pblock->GetValue(PB_MAPPINGTYPE,t,maptype,FOREVER);
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
void MappingStuff(PCloudParticle *po,HWND hWnd,TimeValue t)
{ int maptype;
  po->pblock->GetValue(PB_CUSTOMMTL,t,maptype,FOREVER);
 if (maptype)
  { SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
    SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
   }
  else PCSetMapVals(po->pblock,hWnd,t);
}

void PCAniFr(HWND hWnd,IParamBlock *pblock)
{ int anitype;
  pblock->GetValue(PB_ANIMATIONOFFSET,0,anitype,FOREVER);
  if (anitype>1)
	  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  else	SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),anitype>1);
}

void PCSetRateSpinner(IParamBlock *pblock,TimeValue t,HWND hWnd)
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

void InstOn(PCloudParticle *po,HWND hWnd,TimeValue t)
{ if ((po->fragflags==METABALLS)||(po->fragflags<0))
  { MetaOff(hWnd);
    SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  }
  if (po->fragflags<=0)
  { StdStuff(hWnd,FALSE);  }
  po->fragflags=INSTGEOM;
  InstStuff(hWnd,TRUE,po->hparam,po->spawn,FALSE);
  PCAniFr(hWnd,po->pblock);
  int sptype;
  TurnButton(hWnd,IDC_AP_OBJECTPICK,TRUE);
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),TRUE);
  po->pblock->GetValue(PB_SPINAXISTYPE,0,sptype,FOREVER);
  PCStretchStuff(sptype,INSTGEOM,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  MappingStuff(po,hWnd,t);
  PCSetRateSpinner(po->pblock,t,po->hgen);
  int stype;po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  ObjectMutQueOn(stype,po->spawn,po->pickCB.repi);
  po->pmapPSpin->Invalidate();
}

void MetaIn(PCloudParticle *po,HWND hWnd,TimeValue t)
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
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),FALSE);
  PCStretchStuff(0,METABALLS,po->hrot,po);
  SpinMainStuff(po->hrot,FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
  StdStuff(hWnd,FALSE);
  MappingStuff(po,hWnd,t);
  PCSetRateSpinner(po->pblock,t,po->hgen);
  if (lastflag<=0)
  { StdStuff(hWnd,FALSE); }
  if ((lastflag==INSTGEOM)||(lastflag<0))
  {  InstStuff(hWnd,FALSE,po->hparam,po->spawn,FALSE);
     int maptype,viewpt;
     po->pblock->GetValue(PB_CUSTOMMTL,0,maptype,FOREVER);
     if (maptype==2) po->pblock->SetValue(PB_CUSTOMMTL,0,0);
     po->pblock->GetValue(PB_VIEWPORTSHOWS,0,viewpt,FOREVER);
	 if (viewpt==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	 po->pmapParam->Invalidate();
  }
  ObjectMutQueOff(po->spawn);
  int stype;
  po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
//  if (stype>1) po->pblock->SetValue(PB_SPAWNTYPE,0,0);
//  AllSpawnBad(po->spawn,0,FALSE);
  int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,0,ison,FOREVER);
  if (ison) stype=0;
  SpawnStuff(po->spawn,stype);
  po->pmapPSpin->Invalidate();
}

void StdOn(PCloudParticle *po,HWND hWnd,TimeValue t)
{ if ((po->fragflags==METABALLS)||(po->fragflags<0))
  { MetaOff(hWnd);
    SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  }
  if ((po->fragflags==INSTGEOM)||(po->fragflags<0))
  {  InstStuff(hWnd,FALSE,po->hparam,po->spawn,FALSE);
     int maptype,viewpt;
     po->pblock->GetValue(PB_CUSTOMMTL,0,maptype,FOREVER);
     if (maptype==2) po->pblock->SetValue(PB_CUSTOMMTL,0,0);
     po->pblock->GetValue(PB_VIEWPORTSHOWS,0,viewpt,FOREVER);
	 if (viewpt==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	 po->pmapParam->Invalidate();
  }
  StdStuff(hWnd,TRUE);
  po->fragflags=0;
  EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
 EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
  int sptype;
  po->pblock->GetValue(PB_SPINAXISTYPE,0,sptype,FOREVER);
  PCStretchStuff(sptype,0,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  MappingStuff(po,hWnd,t);
  int facing;
  po->pblock->GetValue(PB_PARTICLETYPE,t,facing,FOREVER);
  /*
  if ((facing==RENDTYPE5)||(facing==RENDTYPE6))
  { po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
	if (facing==2) po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
   EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
  }
  else  */
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  ObjectMutQueOff(po->spawn);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
  SpinnerOff(hWnd,IDC_SP_METTENSSPIN,IDC_SP_METTENS);
}

class PCParticleSpinDlgProc : public ParamMapUserDlgProc {
	public:
		PCloudParticle *po;

		PCParticleSpinDlgProc(PCloudParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void PCParticleSpinDlgProc::Update(TimeValue t)
{   int axis;
	po->pblock->GetValue(PB_SPINAXISTYPE,t,axis,FOREVER);
	PCStretchStuff(axis,po->fragflags,po->hrot,po);
	int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,t,ison,FOREVER);
	int stype; po->pblock->GetValue(PB_SPAWNTYPE,t,stype,FOREVER);
	IPCControls(po->hrot,po->spawn,stype,ison);
}

BOOL PCParticleSpinDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: 
		{break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_AP_PARTICLEDIRTRAVL:
			    PCStretchStuff(1,po->fragflags,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRRND:
				PCStretchStuff(0,po->fragflags,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRUSER:
				PCStretchStuff(2,po->fragflags,hWnd,po);
				break;
			  case IDC_INTERP_BOUNCEON:
				{  int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,t,ison,FOREVER);
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

void AddMtl(PCloudParticle *po,TimeValue t)
{ if (po->cnode)
	{ int subtree,frag,custmtl=0,submtl=0;
    po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
    po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
    po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	if ((po->custnode)&&(frag==INSTGEOM)&& custmtl) 
		po->AssignMtl(po->cnode,po->custnode,subtree,t);
	po->valid=FALSE;
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
 }

class PCParticleDisableDlgProc : public ParamMapUserDlgProc {
	public:
		PCloudParticle *po;
		ICustButton *iBut;

		PCParticleDisableDlgProc(PCloudParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PCParticleDisableDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
	po->ShowName(0);
	int chunky,stype;
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
//	AllSpawnBad(po->spawn,stype,chunky!=METABALLS);
    int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,0,ison,FOREVER);
    if (ison) stype=0;
	SpawnStuff(po->spawn,stype);
	PCCheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi=0);
	if (chunky==METABALLS) MetaIn(po,po->hptype,t);
	else if (chunky==0) StdOn(po,po->hptype,t);
	else InstOn(po,po->hptype,t);
	PCCheckInstButtons(po->pblock,po->hptype);
}

BOOL PCParticleDisableDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{  switch (msg) {
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
			{  case IDC_AP_OBJECTPICK:
				   { po->flags=0;
					if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePCloudMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePCloudMode);
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
				  po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
				  if ((po->cnode)&&(custmtl))
				  { EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMEMIT),FALSE);
				    po->origmtl=po->cnode->GetMtl();
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
				{ po->pblock->SetValue(PB_CUSTOMMTL,0,0);
				  StdOn(po,hWnd,t);
				  po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreatePCloudMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreatePCloudMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				 break;
				}
			  case IDC_SP_TYPEFAC:
			  case IDC_SP_TYPEPIX:
				{ PCSetMapVals(po->pblock,hWnd,t);
//				  int facing;
//				  po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
//				  if (facing==2) po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
//				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
 				  po->pmapParam->Invalidate();
				 break;
				}
			  case IDC_SP_TYPETRI:
			  case IDC_SP_TYPECUB:
			  case IDC_SP_TYPESPC:
			  case IDC_SP_TYPE6PNT:
			  case IDC_SP_TYPESPHERE:
				{ PCSetMapVals(po->pblock,hWnd,t);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
 				 break;
				}
			  case IDC_SP_TYPETET:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
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
			  case IDC_SP_MAPCUSTOMINST:
				{ MappingStuff(po,hWnd,t);
				  int dir;
				  po->pblock->GetValue(PB_SPINAXISTYPE,0,dir,FOREVER);
				  if (dir==1)  PCCheckStretchBox(po->hrot,po);
				 break;
				}
			  case IDC_SP_TYPEMET:
				  {po->pblock->SetValue(PB_CUSTOMMTL,0,0);
					MetaIn(po,hWnd,t);
				    po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreatePCloudMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreatePCloudMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				  break;
				  }
			  case IDC_SP_TYPEINSTANCE:
				  { int custmtl,vshow,anioff;
				    po->pblock->SetValue(PB_CUSTOMMTL,0,1);
					InstOn(po,hWnd,t);
					po->pblock->GetValue(PB_VIEWPORTSHOWS,0,vshow,FOREVER);
					if (vshow>1)
					{int subtree;
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 TimeValue aniend=GetAnimEnd();
					 int anifr=aniend+GetTicksPerFrame();
					 po->GetTimes(po->times,t,anifr,anioff);
					 if (vshow==2)
					 { po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
				       po->GetMesh(t,subtree,custmtl);
					 }
					else po->GetallBB(po->custnode,subtree,t);
					}
				    po->valid=FALSE;
				  break;
				  }
			  case IDC_SP_AUTOCOARSE:
				{ int chunky;
				  po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
				  if (chunky==METABALLS)
					 CourseCheck(po,hWnd,t);
				  break;
				}
			}
			break;	
		default:
			return FALSE;
		}
	return TRUE;
	}
class PCParticleParmDlgProc : public ParamMapUserDlgProc {
	public:
		PCloudParticle *po;
		ICustButton *iBut;

		PCParticleParmDlgProc(PCloudParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PCParticleParmDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
 	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	if (width<0.01f) iBut->Disable(); else iBut->Enable();
	po->ShowName(1);
	po->pblock->GetValue(PB_CREATEIN,0,po->createmeth,FOREVER);
	if (po->createmeth>0)
	  SpinnerOff(po->hparam,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
	else SpinnerOn(po->hparam,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
	EnableWindow(GetDlgItem(po->hparam,IDC_AP_EMITHGT_TXT),po->createmeth==0);
	if ((po->createmeth==0)||(po->createmeth==2))
	  SpinnerOn(po->hparam,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
	else SpinnerOff(po->hparam,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
	EnableWindow(GetDlgItem(po->hparam,IDC_AP_EMITDEP_TXT),(po->createmeth==0)||(po->createmeth==2));
}
	
BOOL PCParticleParmDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_FILLPICKBUTTON));
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
			{   case IDC_AP_FILLPICKBUTTON:
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePCloudMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePCloudMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;
					      po->pickCB.dodist=1;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
				case IDC_AP_FILLBOX:
					po->createmeth=0;
					SpinnerOn(hWnd,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
					SpinnerOn(hWnd,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITHGT_TXT),TRUE);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITDEP_TXT),TRUE);
					break;
				case IDC_AP_FILLSPHERE:
					po->createmeth=1;
					SpinnerOff(hWnd,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
					SpinnerOff(hWnd,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITHGT_TXT),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITDEP_TXT),FALSE);
					break;
				case IDC_AP_FILLCYLINDER:
					po->createmeth=2;
					SpinnerOff(hWnd,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
					SpinnerOn(hWnd,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITHGT_TXT),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITDEP_TXT),TRUE);
					break;
				case IDC_AP_FILLOBJECT:
					po->createmeth=3;
					SpinnerOff(hWnd,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
					SpinnerOff(hWnd,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITHGT_TXT),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_EMITDEP_TXT),FALSE);
					break;
			  case IDC_SP_VIEWDISPMESH:
				  {po->valid=FALSE;
				   int subtree,custmtl;
					po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					TimeValue aniend=GetAnimEnd();
					int anifr=aniend+GetTicksPerFrame();
					po->GetTimes(po->times,t,anifr,anioff);
				    po->GetMesh(t,subtree,custmtl);
				   break;
				  }
			  case IDC_SP_VIEWDISPBOX:			  
				  { po->valid=FALSE;
					int subtree;
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					TimeValue aniend=GetAnimEnd();
					int anifr=aniend+GetTicksPerFrame();
					po->GetTimes(po->times,t,anifr,anioff);
					po->GetallBB(po->custnode,subtree,t);
				  break;
				  }
			}
			break;	
		default:
			return FALSE;
		}
	return TRUE;
	}
class PCParticleSpawnDlgProc : public ParamMapUserDlgProc {
	public:
		PCloudParticle *po;
		ICustButton *iBut,*iButrep;

		PCParticleSpawnDlgProc(PCloudParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void PCParticleSpawnDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
	po->SetUpList();
	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	if (width<0.01f) iBut->Disable();else iBut->Enable();
	po->SetUpLifeList();
	if (width<0.01f) iButrep->Disable();
	else
	{ po->pickCB.repi= SendMessage(GetDlgItem(po->spawn,IDC_AP_OBJECTQUEUE),
					LB_GETCURSEL,0,0);
	  if (po->pickCB.repi<0) iButrep->Disable();
	  else iButrep->Enable();
	}
	int stype,chunky;
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
//	AllSpawnBad(po->spawn,stype,chunky!=METABALLS);
    int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,0,ison,FOREVER);
    if (ison) stype=0;
	SpawnStuff(po->spawn,stype);
//	PCCheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi);
	if (chunky==INSTGEOM)
	  PCCheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi);
	 else 
	 { ObjectMutQueOff(po->spawn);
	   PCCheckLifeButtons(stype,po->spawn);
	 }
	if (stype==EMIT) 
	{	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
		EnableWindow(GetDlgItem(po->spawn,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
	}
}

BOOL PCParticleSpawnDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{  int dtype=2,stype,rep;	
	switch (msg) {
		case WM_INITDIALOG: 
		{ iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUEPICK));
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
			{   case IDC_AP_NOSPAWN:stype=0;goto spawnradio;
			  case IDC_AP_COLLIDEDIE:
				  stype=1;goto spawnradio;
			  case IDC_AP_SPAWNTRAILS:stype=EMIT;goto spawnradio;
			  case IDC_AP_COLLIDESPAWN:
			  case IDC_AP_DEATHSPAWN: stype=2;
			spawnradio:	
				int chunky;
			po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
//			AllSpawnBad(po->spawn,stype,chunky!=METABALLS);
				int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,0,ison,FOREVER);
				if (ison) stype=0;
				SpawnStuff(po->spawn,stype);		
				 if (chunky==INSTGEOM)
					PCCheckSpawnButtons(po->pblock,hWnd,po->pickCB.repi);
				 else 
				 {	 ObjectMutQueOff(po->spawn);
				    PCCheckLifeButtons(stype,po->spawn);
				 }
				 if (stype==EMIT) 
				 {	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
					EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
				 }
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
						{  theCreatePCloudMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePCloudMode);
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
						theHold.Accept(GetString(IDS_AP_OBJDEL));}
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
						theHold.Put(new PCLifeListRestore(po));
						po->DeleteFromLifeList(i);
						TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
						TurnButton(hWnd,IDC_AP_LIFEQUEUEDEL,0);
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
					theHold.Put(new PCLifeListRestore(po));
					po->AddToLifeList(i);
				    TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
					TurnButton(hWnd,IDC_AP_LIFEQUEUEDEL,0);
					theHold.Accept(GetString(IDS_AP_LIFEADD));
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					break;
				}
				case IDC_AP_LIFEQUEUE:
				{ rep = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),LB_GETCURSEL,0,0);
				  TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,rep>-1);
				  TurnButton(hWnd,IDC_AP_LIFEQUEUEDEL,rep>-1);
				  break;
				}
				case IDC_AP_LIFEQUEUEREPL:
				{ int i;
				rep = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),
							LB_GETCURSEL,0,0);
				  if (rep>-1)
				  {	po->pblock->GetValue(PB_SPAWNLIFEVLUE,t,i,FOREVER);
					theHold.Begin();
					theHold.Put(new PCLifeListRestore(po));
					po->llist[rep]=i;
					po->SetUpLifeList();
				    TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,0);
					TurnButton(hWnd,IDC_AP_LIFEQUEUEDEL,0);
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

void PCloudParticle::GetFilename(TCHAR *filename)
{   _tcscpy(filename,ip->GetDir(APP_PLUGCFG_DIR));
  int len= _tcslen(filename);
  if (len)
  {  if (filename[len-1]!=_T('\\'))
		  _tcscat(filename,_T("\\"));
  }
  _tcscat(filename,GetString(IDS_AP_PCLOUDCST));
}

void PCloudParticle::SetupTargetList()		
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
int PCloudParticle::RemSettings(int overwrite,TCHAR *newname)
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
int PCloudParticle::SaveSettings(int overwrite,TCHAR *newname)
{ TCHAR filename[MAX_PATH];
  FILE *f;
  int vers,newsets,Namelen=NLEN;

  if ((overwrite>-1)&&(MessageBox (NULL,GetString(IDS_AP_SETEXISTS),GetString(IDS_AP_WARNING), MB_ICONQUESTION | MB_YESNO ) == IDNO))
	 return 0;
  GetFilename(filename);
  vers=CURRENT_VERSION;
  newsets=custsettings+1;
  if ((f = _tfopen(filename,(custsettings==0?_T("wb"):_T("r+b")))) == NULL)
  { MessageBox(NULL,GetString(IDS_AP_WRITEPRO),"", MB_ICONINFORMATION);
	return 0;
  }
  // longest one
  ParamBlockDescID *descVer=spdescVer5;
  int plength=(PBLOCK_LENGTH_PCLOUD);
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
  for (i=0;i<plength;i++)
  {	if (descVer[i].type==TYPE_INT) 
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
int PCloudParticle::GetSettings(int setnum,TCHAR *newname)
{ TCHAR filename[MAX_PATH];
  FILE *f;
  GetFilename(filename);
  if ((f = _tfopen(filename,_T("rb"))) == NULL) return 0;
  { setnum=setnum*(size+NLEN)+HLEN;
    fseek(f,setnum,SEEK_SET); 
  }	 
  int ival,i;
  float fval;
  // longest one
  ParamBlockDescID *descVer=spdescVer5;
  int plength=(PBLOCK_LENGTH_PCLOUD);
  if (fread(newname,1,NLEN,f)!=NLEN) goto errend;
  for (i=0;i<plength;i++)
  {	if (descVer[i].type==TYPE_INT) 
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
  PCloudParticle *po = (PCloudParticle*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
  if (!po && message!=WM_INITDIALOG) return FALSE;

  switch (message) {
		case WM_INITDIALOG: {
			po = (PCloudParticle*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			po->custCtrlEdit = GetICustEdit(GetDlgItem(hWnd,IDC_SP_SETEDIT));
			po->custCtrlEdit->SetText(_T(""));
			po->hParams2 = hWnd;
			po->SetupTargetList();
			break;
			}
		case WM_COMMAND:
			{ switch(LOWORD(wParam)) {
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
					else if ((i>-1)&&(i<po->custsettings))
					{ po->GetSettings(i,newname);
					  int chunky;
			          po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
					  if (chunky==METABALLS) MetaIn(po,po->hptype,po->ip->GetTime());
				      else 
					  { int stype;
					    po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
//						AllSpawnBad(po->spawn,stype,TRUE);
						int ison; po->pblock->GetValue(PB_PCIPCOLLIDE_ON,0,ison,FOREVER);
						if (ison) stype=0;
						SpawnStuff(po->spawn,stype);
						if (chunky==0) StdOn(po,po->hptype,po->ip->GetTime());
					    else InstOn(po,po->hptype,po->ip->GetTime());
					    if (stype==EMIT)
						{	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
							EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
						}
					  }
					  PCCheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi=0);
				 if (chunky==INSTGEOM)
				 { int onscreen;
				   po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
				   if (onscreen>1)
				   { int subtree,custmtl=0,anioff;
					TimeValue t=po->ip->GetTime();
					 po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 TimeValue aniend=GetAnimEnd();
					 int anifr=aniend+GetTicksPerFrame();
					 po->GetTimes(po->times,t,anifr,anioff);
					 if (onscreen==2)
						 po->GetMesh(po->ip->GetTime(),subtree,custmtl);
						else po->GetallBB(po->custnode,subtree,po->ip->GetTime());
				   }
				 }
					  po->ip->RedrawViews(po->ip->GetTime()); 
					}
					else MessageBox (NULL,GetString(IDS_RB_BADNAME),
            "", MB_ICONINFORMATION);
					break;
				}
			break;
			}

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
		
		default:	return FALSE;		
		}
	return TRUE;
	}	


//--- PCloudParticle Methods--------------------------------------------
TimeValue PCloudParticle::ParticleLife(TimeValue t, int i)
{   int pcount=parts.Count();
	if (!(i<pcount)) return 0;
	return sdata[i].L;
}
void PCloudParticle::ResetSystem(TimeValue t,BOOL full)
{	lc.lastmin=-1;lc.lastcollide=-1;
	rcounter=0;
	vcounter=0;
	if (full)
	{ tvalid = t;
	  valid  = TRUE;
	}
}

Point3 PCloudParticle::ParticlePosition(TimeValue t,int i)
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

Point3 PCloudParticle::ParticleVelocity(TimeValue t,int i)
{	Point3 retvel=Zero;
	int pcount=parts.vels.Count();
	if (i<pcount)
		retvel=parts.vels[i];
	return retvel;
}

float PCloudParticle::ParticleSize(TimeValue t,int i)
{	float strlen=1.0f;
	float boxlen=1.0f;
	float dlgsize;
	int axisentered,K,isinst,ptype;
// get the size/scale from the dialog box processed for this particle...
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

int PCloudParticle::ParticleCenter(TimeValue t,int i)
{	int ptype,isinst;
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	if (isinst==INSTGEOM) return PARTCENTER_CENTER;
	if (ptype==RENDTET) return PARTCENTER_HEAD;
	return PARTCENTER_CENTER;	
}

int PCloudParticle::RenderBegin(TimeValue t, ULONG flags)
	{	SetAFlag(A_RENDER);
  		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		cancelled=FALSE;
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)1);
	return 0;
	}

int PCloudParticle::RenderEnd(TimeValue t)
	{
		ClearAFlag(A_RENDER);
		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)0);
/*		TimeValue aniend=GetAnimEnd();
	int subtree,onscreen,custmtl=0;
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	if ((frag==INSTGEOM)&&(onscreen>1)&&(custnode))
	{ pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
      pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	  thePCloudDraw.anioff=anioff;
      thePCloudDraw.anifr=aniend+GetTicksPerFrame();
	  thePCloudDraw.t=t;
	  int anioff;
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
	  if (anioff>0)
	   GetTimes(times,thePCloudDraw.t,thePCloudDraw.anifr,anioff);
	  else times.tl.ZeroCount();
	  if (onscreen==2)
	   GetMesh(t,subtree,custmtl);
	  else GetallBB(custnode,subtree,t);
	}*/
	return 0;
	}

void PCloudParticle::BeginEditParams(
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
	{ 		pmapParam = CreateCPParamMap(
			descParamPCloud,PARAMPCLOUD_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTICLECLOUD),
			GetString(IDS_RB_PARAMETERS),
			dflags&APRTS_ROLLUP1_OPEN?0:APPENDROLL_CLOSED);

		pmapPGen = CreateCPParamMap(
			descParamPGen,PARAMPGEN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_GEN_PC),
			GetString(IDS_RB_PGEN),
			dflags&APRTS_ROLLUP2_OPEN?0:APPENDROLL_CLOSED);
		
		pmapPType = CreateCPParamMap(
			descParamPType,PARAMPTYPE_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTTYPE_PC),
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
		ip->RegisterDlgWnd(hParams2);
		fragflags=-1;

		}
	spawn=pmapSpawn->GetHWnd();
	hparam=pmapParam->GetHWnd();
	hgen=pmapPGen->GetHWnd();
	hptype=pmapPType->GetHWnd();
	hrot=pmapPSpin->GetHWnd();
	hbubl=pmapBubl->GetHWnd();
	if (pmapParam) pmapParam->SetUserDlgProc(new PCParticleParmDlgProc(this));
	if (pmapPType) pmapPType->SetUserDlgProc(new PCParticleDisableDlgProc(this));
	if (pmapPGen) pmapPGen->SetUserDlgProc(new PCParticleGenDlgProc(this));
	if (pmapSpawn) pmapSpawn->SetUserDlgProc(new PCParticleSpawnDlgProc(this));
	if (pmapPSpin) pmapPSpin->SetUserDlgProc(new PCParticleSpinDlgProc(this));
}	

void PCloudParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{ TimeValue t0,t2;
	SetFlag(dflags,APRTS_ROLLUP1_OPEN,IsRollupPanelOpen(hparam));
	SetFlag(dflags,APRTS_ROLLUP2_OPEN,IsRollupPanelOpen(hgen));
	SetFlag(dflags,APRTS_ROLLUP3_OPEN,IsRollupPanelOpen(hptype));
	SetFlag(dflags,APRTS_ROLLUP4_OPEN,IsRollupPanelOpen(hrot));
	SetFlag(dflags,APRTS_ROLLUP5_OPEN,IsRollupPanelOpen(pmapEmitV->GetHWnd()));
	SetFlag(dflags,APRTS_ROLLUP6_OPEN,IsRollupPanelOpen(hbubl));
	SetFlag(dflags,APRTS_ROLLUP7_OPEN,IsRollupPanelOpen(spawn));
	SetFlag(dflags,APRTS_ROLLUP8_OPEN,IsRollupPanelOpen(hParams2));
	SimpleParticle::EndEditParams(ip,flags,next);

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
		hgen=NULL;
	}else
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG)NULL);
	ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
}

void PCloudParticle::MapKeys(TimeMap *map,DWORD flags)
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

Interval PCloudParticle::GetValidity(TimeValue t)
	{
	// For now...
	return Interval(t,t);
	}

void PCloudParticle::BuildEmitter(TimeValue t, Mesh& amesh)
	{
	float width,height,depth,wholewid;
	int sym,csymv=20,csymf=18,startv,startf;
	mvalid = FOREVER;
    pblock->GetValue(PB_CREATEIN,0,sym,FOREVER);
	pblock->GetValue(PB_EMITRWID,t,height,mvalid);
	pblock->GetValue(PB_EMITRHEIGHT,t,width,mvalid);
	pblock->GetValue(PB_EMITRDEPTH,t,depth,mvalid);
	wholewid=width;
	width  *= 0.5f;
	height  *= 0.5f;
	depth  *= 0.5f;

	if (sym==0)
	{ mesh.setNumVerts(9+csymv);startv=9;
	  mesh.setNumFaces(12+csymf);startf=12;
	  mesh.setVert(0, Point3(-width,-height, depth));
	  mesh.setVert(1, Point3( width,-height, depth));
	  mesh.setVert(2, Point3( width, height, depth));
	  mesh.setVert(3, Point3(-width, height, depth));
	  mesh.setVert(4, Point3(-width,-height, -depth));
	  mesh.setVert(5, Point3( width,-height, -depth));
	  mesh.setVert(6, Point3( width, height, -depth));
	  mesh.setVert(7, Point3(-width, height, -depth));
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
	}
	else if (sym==1)
	{ 
		height *= 2.0f;
		float u,cu,su;
	  #define NUM_SEGS 12
	  startf=3*NUM_SEGS;
	  startv=startf+1;
	  mesh.setNumVerts(startv+csymv);
	  mesh.setNumFaces(startf+csymf);

	  for (int i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			cu=(float)cos(u)*height;su=(float)sin(u)*height;
			mesh.setVert(i, Point3(cu, su, 0.0f));
			mesh.setVert(i+NUM_SEGS, Point3(0.0f, cu, su));
			mesh.setVert(i+2*NUM_SEGS, Point3(cu, 0.0f, su));
			}
	  mesh.setVert(3*NUM_SEGS, Point3(0.0f, 0.0f, 0.0f));
		
	  for (i=0; i<3*NUM_SEGS; i++) {
			int i1 = i+1;
			if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
			mesh.faces[i].setEdgeVisFlags(1,0,0);
			mesh.faces[i].setSmGroup(0);
			mesh.faces[i].setVerts(i,i1,3*NUM_SEGS);
			}
	}
	else if (sym==2)
	{ float u,cu,su;
	  #define NUM_SEGS 12
	  startv=(startf=2*NUM_SEGS);
	  mesh.setNumVerts(startv+csymv);
	  mesh.setNumFaces(startf+csymf);
      
	  for (int i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			cu=(float)cos(u)*height;su=(float)sin(u)*height;
			mesh.setVert(i, Point3(cu, su, depth));
			mesh.setVert(i+NUM_SEGS, Point3(cu,su, -depth));
			}
	  int il=NUM_SEGS;
	  i=0;	
	  for (int f=0; f<2*NUM_SEGS; f+=2) {
			int i1 = i+1,i2=il+1;
			if (i1%NUM_SEGS==0) {i1 -= NUM_SEGS;i2-=NUM_SEGS;}
			mesh.faces[f].setEdgeVisFlags(1,0,1);
			mesh.faces[f].setSmGroup(0);
			mesh.faces[f].setVerts(i,il,i1);
			mesh.faces[f+1].setEdgeVisFlags(1,1,0);
			mesh.faces[f+1].setSmGroup(0);
			mesh.faces[f+1].setVerts(il,i2,i1);
			il++;i++;
			}
	}
	else
	{ mesh.setNumVerts(35);
	  mesh.setNumFaces(28);
/*	  if (distnode)
	  { Object *cobj=distnode->EvalWorldState(t).obj;
		TriObject *triOb=TriIsUseable(cobj,t);
		if (triOb)
		{ Box3 bbox=triOb->mesh.getBoundingBox();
		  Point3 lens=(bbox.pmax-bbox.pmin)/2.0f;
		  width=lens.x;
	      height=lens.y;
	      depth=lens.z;
		}
	  }*/
	  width=2*height;depth=height;wholewid=width*2;
	  mesh.setVert(0, Point3(-width,-height, depth));
	  mesh.setVert(1, Point3( width,-height, depth));
	  mesh.setVert(2, Point3( width, height, depth));
	  mesh.setVert(3, Point3(-width, height, depth));
	  mesh.setVert(4, Point3(-width,-height, -depth));
	  mesh.setVert(5, Point3( width,-height, -depth));
	  mesh.setVert(6, Point3( width, height, -depth));
	  mesh.setVert(7, Point3(-width, height, -depth));
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

	  float h6=height*0.625f,h1=height*0.125f,h3=height*0.375f;
	  float w4=0.4f*wholewid,w3=0.3f*wholewid,w2=0.2f*wholewid,w1=0.1f*wholewid;

	  mesh.setVert(9, Point3(-w4,-h6, 0.0f));
	  mesh.setVert(10, Point3(-w3,-h6, 0.0f));
	  mesh.setVert(11, Point3(-w3,-h1, 0.0f));
	  mesh.setVert(12, Point3(-w2,-h1, 0.0f));
	  mesh.setVert(13, Point3(-w2,h1, 0.0f));
	  mesh.setVert(14, Point3(-w3,h1, 0.0f));
	  mesh.setVert(15, Point3(-w3,h3, 0.0f));
	  mesh.setVert(16, Point3(-w2,h3, 0.0f));
	  mesh.setVert(17, Point3(-w2,h6, 0.0f));
	  mesh.setVert(18, Point3(-w4,h6, 0.0f));
	  mesh.setVert(19, Point3(-w1,-h6, 0.0f));
	  mesh.setVert(20, Point3(0.0f,-h6, 0.0f));
	  mesh.setVert(21, Point3(0.0f,-h1, 0.0f));
	  mesh.setVert(22, Point3(-w1,-h1, 0.0f));
	  mesh.setVert(23, Point3(-w1,h1, 0.0f));
	  mesh.setVert(24, Point3(0.0f,h1, 0.0f));
	  mesh.setVert(25, Point3(0.0f,h3, 0.0f));
	  mesh.setVert(26, Point3(-w1,h3, 0.0f));
	  mesh.setVert(27, Point3(w1,-h6, 0.0f));
	  mesh.setVert(28, Point3(w2,-h6, 0.0f));
	  mesh.setVert(29, Point3(w2,h6, 0.0f));
	  mesh.setVert(30, Point3(w1,h6, 0.0f));
	  mesh.setVert(31, Point3(w3,-h6, 0.0f));
	  mesh.setVert(32, Point3(w4,-h6, 0.0f));
	  mesh.setVert(33, Point3(w4,h6, 0.0f));
	  mesh.setVert(34, Point3(w3,h6, 0.0f));

	  mesh.faces[12].setEdgeVisFlags(1,1,0);
	  mesh.faces[12].setSmGroup(0);
	  mesh.faces[12].setVerts(9,10,11);

	  mesh.faces[13].setEdgeVisFlags(1,1,0);
	  mesh.faces[13].setSmGroup(0);
	  mesh.faces[13].setVerts(11,12,13);

	  mesh.faces[14].setEdgeVisFlags(1,0,0);
	  mesh.faces[14].setSmGroup(0);
	  mesh.faces[14].setVerts(13,14,11);

	  mesh.faces[15].setEdgeVisFlags(1,0,0);
	  mesh.faces[15].setSmGroup(0);
	  mesh.faces[15].setVerts(14,15,18);

	  mesh.faces[16].setEdgeVisFlags(1,1,0);
	  mesh.faces[16].setSmGroup(0);
	  mesh.faces[16].setVerts(15,16,17);

	  mesh.faces[17].setEdgeVisFlags(1,0,0);
	  mesh.faces[17].setSmGroup(0);
	  mesh.faces[17].setVerts(17,18,15);

	  mesh.faces[18].setEdgeVisFlags(1,0,0);
	  mesh.faces[18].setSmGroup(0);
	  mesh.faces[18].setVerts(18,9,14);

	  mesh.faces[27].setEdgeVisFlags(0,0,0);
	  mesh.faces[27].setSmGroup(0);
	  mesh.faces[27].setVerts(9,11,14);

	  mesh.faces[19].setEdgeVisFlags(1,1,0);
	  mesh.faces[19].setSmGroup(0);
	  mesh.faces[19].setVerts(19,20,21);

	  mesh.faces[20].setEdgeVisFlags(1,1,0);
	  mesh.faces[20].setSmGroup(0);
	  mesh.faces[20].setVerts(21,22,19);

	  mesh.faces[21].setEdgeVisFlags(1,1,0);
	  mesh.faces[21].setSmGroup(0);
	  mesh.faces[21].setVerts(27,28,29);

	  mesh.faces[22].setEdgeVisFlags(1,1,0);
	  mesh.faces[22].setSmGroup(0);
	  mesh.faces[22].setVerts(29,30,27);

	  mesh.faces[23].setEdgeVisFlags(1,1,0);
	  mesh.faces[23].setSmGroup(0);
	  mesh.faces[23].setVerts(31,32,33);

	  mesh.faces[24].setEdgeVisFlags(1,1,0);
	  mesh.faces[24].setSmGroup(0);
	  mesh.faces[24].setVerts(33,34,31);

	  mesh.faces[25].setEdgeVisFlags(1,1,0);
	  mesh.faces[25].setSmGroup(0);
	  mesh.faces[25].setVerts(23,24,25);

	  mesh.faces[26].setEdgeVisFlags(1,1,0);
	  mesh.faces[26].setSmGroup(0);
	  mesh.faces[26].setVerts(25,26,23);
	}
	if (sym<3)
	{int s0=startv;
	 int f0=startf;
	  float r=height;
	  float r4=0.4f*r,r2=0.2f*r,r6=0.6f*r,r3=0.3f*r;

	  mesh.setVert(startv++, Point3(r6,r2, 0.0f));
	  mesh.setVert(startv++, Point3(r6,r4, 0.0f));
	  mesh.setVert(startv++, Point3(r4,r6, 0.0f));
	  mesh.setVert(startv++, Point3(-r4,r6, 0.0f));
	  mesh.setVert(startv++, Point3(-r6,r4, 0.0f));
	  mesh.setVert(startv++, Point3(-r6,-r4, 0.0f));
	  mesh.setVert(startv++, Point3(-r4,-r6, 0.0f));
	  mesh.setVert(startv++, Point3(r4,-r6, 0.0f));
	  mesh.setVert(startv++, Point3(r6,-r4, 0.0f));
	  mesh.setVert(startv++, Point3(r6,-r2, 0.0f));
	  mesh.setVert(startv++, Point3(r4,-r2, 0.0f));
	  mesh.setVert(startv++, Point3(r4,-r3, 0.0f));
	  mesh.setVert(startv++, Point3(r3,-r4, 0.0f));
	  mesh.setVert(startv++, Point3(-r3,-r4, 0.0f));
	  mesh.setVert(startv++, Point3(-r4,-r3, 0.0f));
	  mesh.setVert(startv++, Point3(-r4,r3, 0.0f));
	  mesh.setVert(startv++, Point3(-r3,r4, 0.0f));
	  mesh.setVert(startv++, Point3(r3,r4, 0.0f));
	  mesh.setVert(startv++, Point3(r4,r3, 0.0f));
	  mesh.setVert(startv++, Point3(r4,r2, 0.0f));

	  mesh.faces[startf].setEdgeVisFlags(1,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+19,s0,s0+1);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+1,s0+18,s0+19);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+18,s0+1,s0+2);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+2,s0+17,s0+18);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+17,s0+2,s0+3);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+3,s0+16,s0+17);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+16,s0+3,s0+4);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+4,s0+15,s0+16);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+15,s0+4,s0+5);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+5,s0+14,s0+15);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+14,s0+5,s0+6);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+6,s0+13,s0+14);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+13,s0+6,s0+7);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+7,s0+12,s0+13);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+12,s0+7,s0+8);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+8,s0+11,s0+12);

	  mesh.faces[startf].setEdgeVisFlags(0,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+11,s0+8,s0+9);

	  mesh.faces[startf].setEdgeVisFlags(1,1,0);
	  mesh.faces[startf].setSmGroup(0);
	  mesh.faces[startf++].setVerts(s0+9,s0+10,s0+11);

	}
	mesh.InvalidateGeomCache();
	}

int PCloudParticle::CountLive()
{	int c=0;
	for (int i=0; i<parts.Count(); i++)
	  {if (parts.Alive(i)) c++;}
	return c;
}

#define DUMSHINE .20f  //.25f
#define DUMSPEC .20f

void PCloudParticle::RetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
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

void PCloudParticle::DoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) DoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) RetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void PCloudParticle::AssignMtl(INode *node,INode *topnode,int subtree,TimeValue t) 
{	Mtl *submtl;
	MultiMtl *newmat=NULL;
	  Mtl *nmtl=NULL;
	TSTR newname;
	MtlBaseLib glib;
	int tabmax=256;
	newname=TSTR(_T("CMat"))+node->GetName();
	if (_tcslen(newname)>16) newname[16]='\0';
	int numsubs=0,numtabs=0;
	nmtls.SetCount(nlist.Count());
    mttab.SetCount(tabmax);
    submtl=custnode->GetMtl();
	INode *tmpnode=custnode;
	backpatch=FALSE;
	int nCount=nlist.Count();
	for (int mut=0;mut<=nCount;mut++)
	{ if (tmpnode->IsGroupHead())
	    DoGroupMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  else
	  RetrieveMtls(tmpnode,subtree,&numsubs,&numtabs,&tabmax,t);
	  if (mut!=nCount)
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
void PCloudParticle::CntRetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
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

void PCloudParticle::CntDoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) CntDoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) CntRetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void PCloudParticle::GetSubs(INode *node,INode *topnode,int subtree,TimeValue t) 
{	Mtl *submtl;
	int tabmax=256;
	int numsubs=0,numtabs=0;
	int mstart=0,mcnt=nlist.Count();INode *onode;
	onode=custnode;
	nmtls.SetCount(mcnt);
    submtl=onode->GetMtl();
	INode *tmpnode=onode;
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

void PCloudParticle::GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
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
	{ thePCloudDraw.bboxpt[tcount].bpts.Resize((*tabmax)+=256);}
    Matrix3 ctm = node->GetObjectTM(t); //change
	if (node!=custnode)
  { thePCloudDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f)*ctm-boxcenter;
  }	
  else thePCloudDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f);
	ctm.NoTrans();
	Box3 bbox=triOb->GetMesh().getBoundingBox();
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[0]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[1]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[2]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[3]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[4]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[5]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[6]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  thePCloudDraw.bboxpt[tcount].bpts[*count].pts[7]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  (*count)++;
	  if (triOb!=cobj) triOb->DeleteThis();
  }
}

void PCloudParticle::DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) DoGroupBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	  else if((subtree)||(nxtnode->IsGroupMember())) GetNextBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	}
}

void PCloudParticle::GetallBB(INode *custnode,int subtree,TimeValue t)
{ int tabmax=256;
  int count=1,ocount=times.tl.Count();
  if (ocount>0) count=ocount;
  if (thePCloudDraw.bboxpt) delete[] thePCloudDraw.bboxpt;
  thePCloudDraw.bboxpt=NULL;
  INode *tmpnode;
  if (custnode!=NULL)
  { thePCloudDraw.bboxpt=new boxlst[count];
    int cgen;
    for (int tcount=0;tcount<count;tcount++)
    { TimeValue tofs=(ocount>0?times.tl[tcount].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[tcount].gennum-1:-1);
	  if ((cgen>-1)&&(cgen<nlist.Count()))
	  { if (!(tmpnode=nlist[cgen])) tmpnode=custnode;
	  } else tmpnode=custnode;
	  thePCloudDraw.bboxpt[tcount].bpts.SetCount(tabmax);
      thePCloudDraw.bboxpt[tcount].numboxes=0;
      Matrix3 ctm = tmpnode->GetObjectTM(tofs); //change
      boxcenter=Zero*ctm;
	  if (tmpnode->IsGroupHead())
	    DoGroupBB(tmpnode,subtree,&(thePCloudDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  else
      GetNextBB(tmpnode,subtree,&(thePCloudDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  thePCloudDraw.bboxpt[tcount].bpts.SetCount(thePCloudDraw.bboxpt[tcount].numboxes);
	 thePCloudDraw.bboxpt[tcount].bpts.Shrink();
    }
  }
}

void PCloudParticle::CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
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
    Matrix3 ctm = node->GetObjectTM(t); //change
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
//	{ cmesh->verts[j]=triOb->GetMesh().verts[k]*ctm+Suboffset;
	{ cmesh->verts[j]=triOb->GetMesh().verts[k]*ctm;
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

void PCloudParticle::TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,Box3 *cmbb,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) TreeDoGroup(nxtnode,tspace,cmesh,cmbb,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	  else if ((subtree)||(nxtnode->IsGroupMember())) CheckTree(nxtnode,tspace,cmesh,cmbb,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	}
}

void PCloudParticle::GetMesh(TimeValue t,int subtree,int custmtl)
{	int tnums,numV,numF,tvnum,ismapped,msubtree=subtree;
    INode *tmpnode;
	tnums=times.tl.Count();
	if (tnums==0) tnums=1;
	if (cmesh) delete[] cmesh;cmesh=NULL;
	if (cmbb) delete[] cmbb;cmbb=NULL;
	if (custnode!=NULL)
	{ cmesh=new Mesh[tnums];
	  cmbb=new Box3[tnums];
//	  for (int cm=0;cm<tnums;cm++) cmbb[cm].SetEmpty();
	  int cgen;
	 for (int i=0;i<tnums;i++)
	{ TimeValue tofs=(tnums>1?times.tl[i].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[i].gennum-1:-1);
	  if ((cgen>-1)&&(cgen<nlist.Count()))
	  { if (!(tmpnode=nlist[cgen])) tmpnode=custnode;
	  } else tmpnode=custnode;
	  Matrix3 ptm = tmpnode->GetObjectTM(tofs); //change
	  ptm=Inverse(ptm);
	  numV=numF=tvnum=CustMtls=ismapped=0;
	  if (tmpnode->IsGroupHead())
	   TreeDoGroup(tmpnode,ptm,&cmesh[i],&cmbb[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	  else
	   CheckTree(tmpnode,ptm,&cmesh[i],&cmbb[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	}
	}
}

#define VEL_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))
#define VAR_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))

BOOL PCloudParticle::ComputeParticleStart(TimeValue t0,int c)
	{
	int seed,anioff,tani,sym;
	TimeValue anifr;

	if (c > gCountUpperLimit) c = gCountUpperLimit;
	pblock->GetValue(PB_RNDSEED,t0,seed,FOREVER);
    pblock->GetValue(PB_OFFSETAMOUNT,t0,anifr,FOREVER);
    pblock->GetValue(PB_ANIMATIONOFFSET,t0,anioff,FOREVER);
    pblock->GetValue(PB_CREATEIN,0,sym,FOREVER);
	srand(seed);					
	parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	int pcount=parts.Count();
    if (sdata){delete[] sdata;sdata=NULL;} if (pcount) sdata=new PCSavePt[pcount];
	if ((pcount<c)||(c>0 && (!sdata)))
	{   parts.FreeAll();if (sdata) delete sdata;sdata=NULL;	maincount=0;
		BOOL playing=GetCOREInterface()->IsAnimPlaying();
		if (playing) GetCOREInterface()->EndAnimPlayback();
	    TSTR name;name=(cnode ? cnode->GetName() : TSTR(GetString(IDS_AP_PCLOUD)));
		TSTR buf; buf=TSTR(GetString(IDS_OFM_PART));
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			GetString(IDS_OFM_ERROR),_T("%s: \n\n%s\n"),buf,name);
	  return (0);
	}
	int oneframe=GetTicksPerFrame();
	for (int i=0; i<parts.Count(); i++) {
		parts.ages[i] = -1;
		sdata[i].themtl=0;
  		sdata[i].L=RND0x(99);sdata[i].DL=-1;sdata[i].pvar=RND11();
		tani=RND0x(anifr/oneframe);
		sdata[i].showframe=(anioff==2?tani*oneframe:0);
		sdata[i].gennum=0;
		sdata[i].V.x=RND11();
		sdata[i].V.y=RND11();
		sdata[i].V.z=RND11();
		sdata[i].Ts0=RND11();
		sdata[i].Ts=0.0f;
		sdata[i].LamTs=RND11();
		sdata[i].A=RND11();
		sdata[i].LamA=RND11();
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		parts.vels[i]=Point3(xtmp,ytmp,ztmp);
		sdata[i].To=RND11();
		sdata[i].Vsz=RND11();
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND11(); ytmp=RND11(); xtmp=RND11();
		sdata[i].W=Point3(xtmp,ytmp,ztmp);
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND01(); ytmp=RND01(); xtmp=RND01();
		sdata[i].RV=Point3(xtmp,ytmp,ztmp);
//		sdata[i].RV=Point3(RND11(),RND11(),RND11());
		parts.radius[i]=0.0f;
		parts.tension[i]=RND11();
		sdata[i].Mltvar=RND11();
		sdata[i].SpVar=RND0x(99);
		// Martell 4/14/01: Fix for order of ops bug.
		//parts.points[i]=(sym<3?Point3((float)RNDSign(),(float)RNDSign(),(float)RNDSign()):Point3(RND01(),RND01(),RND01()));
		if ( sym<3 )
		{
			ztmp=(float)RNDSign(); ytmp=(float)RNDSign(); xtmp=(float)RNDSign();
			parts.points[i]=Point3( xtmp,ytmp,ztmp );
		}
		else
		{
			ztmp=RND01(); ytmp=RND01(); xtmp=RND01();
			parts.points[i]=Point3( xtmp,ytmp,ztmp );
		}
		}
	tvalid = t0-1;
	valid  = TRUE;
//	rseed=rand();
	return (1);
	}

 #define EPSILON	0.0001f

BOOL InsideMesh(Ray& ray, Mesh *amesh,Point3 *fnorms)
{	Face *face;	
	Point3 n,p, bry;
	float d, rn, a,at;
	BOOL notfirst = FALSE,inside=FALSE,savein=FALSE;
	int raycount=0;

   while ((!savein)&&(raycount<3))
   { notfirst=FALSE;
     if (raycount==0) ray.dir=Point3(1.0f,0.0f,0.0f);
	 else if (raycount==1) ray.dir=Point3(0.0f,1.0f,0.0f);
	 else ray.dir=Point3(0.0f,0.0f,1.0f);
	face=amesh->faces;
	for (int i=0; i<amesh->getNumFaces(); i++,face++)
	{	n = fnorms[i];
		// See if the ray intersects the plane (backfaced)
		rn = DotProd(ray.dir,n);
		if (fabs(rn) < EPSILON) continue; //is parallel, so bail
		// Use a point on the plane to find d
		d = DotProd(amesh->verts[face->v[0]],n);
		// Find the point on the ray that intersects the plane
		a = (d - DotProd(ray.p,n)) / rn;
		// The point on the ray and in the plane.
		p = ray.p + a*ray.dir;
		inside=(((a > 0.0f)&&(rn>0.0f))||((a < 0.0f)&&(rn<0.0f)));
		// Compute barycentric coords.
		bry = amesh->BaryCoords(i,p);
		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
		if (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) continue;
		if (fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) continue;
		// Must be closer than the closest at so far
		if (notfirst)
			if (fabs(a) > at) continue;

		// Hit!
		notfirst = TRUE;		
		at    = (float)fabs(a);
		savein=inside;
	  } raycount++;
   }
	return savein;

	}

#define TESTLIMIT 100

void PCloudParticle::BirthParticle(INode *node,TimeValue bt,int num,VelDir *ptvel,Mesh* amesh,Point3* fnorms,Matrix3 disttm)
{
//	Matrix3 atm = (amesh?distnode->GetObjTMBeforeWSM(bt):node->GetObjTMBeforeWSM(bt));
//	Matrix3 tm = (amesh?distnode->GetObjTMAfterWSM(bt):node->GetObjTMAfterWSM(bt));
//	tm.SetRow(3,atm.GetRow(3));
	Matrix3 tm =  (amesh?distnode->GetObjTMAfterWSM(bt):node->GetObjTMAfterWSM(bt)); //change

	Point3 vel;
	float Ie,Em,Vm;
	pblock->GetValue(PB_EMITVINFL,bt,Ie,FOREVER);
	pblock->GetValue(PB_EMITVMULT,bt,Em,FOREVER);
	pblock->GetValue(PB_EMITVMULTVAR,bt,Vm,FOREVER);
	sdata[num].Ts0 = (1.0f + sdata[num].Ts0*ptvel->VSpin)/TWOPI;
	sdata[num].Ts = (float)ptvel->Spin*sdata[num].Ts0;
	parts.tension[num] = ptvel->bstr*(1.0f + parts.tension[num]*ptvel->bstrvar);
// ok, so I'm using L for M and .z for L.  They were unused float and ints
//	srand(rseed);

	float inheremit,tmp = RND11();
	inheremit = (sdata[num].L<Ie?Em*(1 + tmp*Vm):0);  
	sdata[num].L = ptvel->Life + (int)(parts.vels[num].z*ptvel->Vl);
	sdata[num].Vsz *= ptvel->VSz;
	sdata[num].LamTs = ptvel->Phase*(1.0f + sdata[num].LamTs*ptvel->VPhase);
	sdata[num].A = ptvel->ToAmp*(1.0f + sdata[num].A*ptvel->VToAmp);
	sdata[num].LamA = ptvel->ToPhase*(1.0f + sdata[num].LamA*ptvel->VToPhase);
	sdata[num].To = ptvel->ToPeriod*(1 + sdata[num].To*ptvel->VToPeriod);
	sdata[num].persist = (TimeValue)(ptvel->persist*(1.0f + sdata[num].pvar*ptvel->pvar));

	if (ptvel->axisentered==2)
	{	sdata[num].W = Normalize(ptvel->Axis);
		if (ptvel->axisvar>0.0f) 
			VectorVar(&sdata[num].W,ptvel->axisvar,PI);
	}
	else
		sdata[num].W=Normalize(sdata[num].W);

	parts.ages[num] = 0;
	float theta = RND01(),
		  gamma = (ptvel->sym==2?RND11():RND01()),r=RND01();

	if (ptvel->sym==0) //box emitter emitter
	{	parts.points[num].x *= ptvel->width*r;
		parts.points[num].y *= ptvel->len*theta;
		parts.points[num].z *= ptvel->depth*gamma;
	}
	else if (ptvel->sym<3)
	{	float thetapi = theta*TWOPI;
		float lec = ptvel->len*r;
		if (ptvel->sym==2) //cylinder emitter
		{	parts.points[num].x = lec*(float)cos(thetapi);
			parts.points[num].y = lec*(float)sin(thetapi);
			parts.points[num].z *= gamma*ptvel->depth;
		} 
		else if (ptvel->sym==1) //spherical emitter
		{	float hgamma = gamma*HalfPI;
			lec *= 2.0f;
			parts.points[num].x = lec*(float)cos(thetapi)*(float)cos(hgamma);
			parts.points[num].y = lec*(float)sin(thetapi)*(float)cos(hgamma);
			parts.points[num].z *= lec*(float)sin(hgamma);
		}
	}
	else //custom emitter
	{	Ray ray;
		Box3 bbox = amesh->getBoundingBox();
		Point3 box = bbox.pmax - bbox.pmin;

//		if (box.x<EPSILON) {box.x=0.2f;bbox.pmin.x-=0.1f;}
//		if (box.y<EPSILON) {box.y=0.2f;bbox.pmin.y-=0.1f;}
//		if (box.z<EPSILON) {box.z=0.2f;bbox.pmin.z-=0.1f;}

		if ((box.x<EPSILON)||(box.y<EPSILON)||(box.z<EPSILON)) //emitter volume isn't
		{	
			// Martell 4/14/01: Fix for order of ops bug.
			float ztmp=RND01(); float ytmp=RND01(); float xtmp=RND01();
			ray.p = bbox.pmin + Point3(xtmp,ytmp,ztmp)*box;
		}
		else //assume we have a logically arranged mesh
		{	
			int testcount=0;
			ray.p = bbox.pmin + parts.points[num]*box;
			while ((!InsideMesh(ray,amesh,fnorms))&&(testcount<TESTLIMIT))
			{	testcount++;
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND01(); float ytmp=RND01(); float xtmp=RND01();
				ray.p = bbox.pmin + Point3(xtmp,ytmp,ztmp)*box;
			}
		}
		parts.points[num] = ray.p;
	}

	if ((ptvel->direntered==2)&&(mbase))
	{	Interval ivalid;
		Matrix3 ntm = mbase->GetObjectTM(bt,&ivalid);
		vel = ntm.GetRow(2);
		if (ptvel->DirVar>0.0f)
			VectorVar(&vel,ptvel->DirVar,PI);
	}
	else if (ptvel->direntered==1)
	{	vel = Normalize(ptvel->Dir);
		if (ptvel->DirVar>0.0f) 
			VectorVar(&vel,ptvel->DirVar,PI);
	}
	else
		vel = parts.vels[num];

	vel = VectorTransform(tm,vel);

	vel.x *= ptvel->Speed*(1.0f + sdata[num].V.x*ptvel->VSpeed);
	vel.y *= ptvel->Speed*(1.0f + sdata[num].V.y*ptvel->VSpeed);
	vel.z *= ptvel->Speed*(1.0f + sdata[num].V.z*ptvel->VSpeed);

//	parts[num] = (parts[num] * tm) + vel*(0.5f*sdata[num].RV.x);
	parts[num] = parts[num] * tm;

	parts[num] += ptvel->bps.Position(1.0f - sdata[num].RV.x) - tm.GetTrans();
	parts[num] += vel*(sdata[num].RV.x);

	if ((float)fabs(inheremit)>0.0f)
	{
		Point3 inhVel = ptvel->bps.Speed(1.0f - sdata[num].RV.x);
		parts[num] += inhVel*inheremit*sdata[num].RV.x;
		vel += inhVel*inheremit; // motion inheritance
	}

	vel /= (float)GetTicksPerFrame();

	parts.vels[num] = vel;  
	sdata[num].V = parts[num];
//	rseed=rand();
}

void PCloudParticle::DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emits)
{ if (!emits) spmult--;
  int oldcount,newcount=((oldcount=parts.Count())+spmult); 
//  srand(rseed);
  if (spmult)
  { parts.SetCount(newcount,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	PCSavePt *tmp=sdata;
	sdata=new PCSavePt[newcount];
	if (tmp)
	{ for (int j=0;j<oldcount;j++) sdata[j]=tmp[j];
	  delete[] tmp;
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
	memcpy(&sdata[i],&sdata[j],sizeof(PCSavePt));
	if (emits) sdata[i].gennum++;
    parts.vels[i]=DoSpawnVars(spvars,parts.vels[j],holdv,&parts.radius[i],&sdata[i].W);
	sdata[i].Vsz=parts.radius[i];
	sdata[i].L=baselife+(int)RND11()*lvar;
 	sdata[i].Mltvar=RND11();
	sdata[i].SpVar=RND0x(99);
	sdata[i].DL=-1;
 }
  if (!emits)
  {parts.vels[j]=DoSpawnVars(spvars,holdv,holdv,&parts.radius[j],&sdata[j].W);
  sdata[j].Vsz=parts.radius[j];
  sdata[j].L=baselife+(int)RND11()*lvar;
  	sdata[j].Mltvar=RND11();
	sdata[j].SpVar=RND0x(99);
	sdata[j].DL=-1;
}
/*  int tmprseed=rand();
  rseed=(tmprseed==rseed?rand():tmprseed);*/
//  rseed=rand();
}
void PCloudParticle::MovePart(int j,TimeValue dt,BOOL fullframe,int tpf)
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

void PCloudParticle::UpdateParticles(TimeValue t,INode *node)
	{
	TimeValue t0,dt,t2,grow,fade;
	int i, j, c, birth,counter,tpf=GetTicksPerFrame(),count=0,anioff;
	VelDir ptvel;
    Matrix3 tm,lasttm; 
	int isrend=TestAFlag(A_RENDER),bmethod,onscreen,oneframe;
	// variable for new collision scheme (Bayboro 2/5/01)
	CollisionCollection cc;

	// fix for "Abort by Escape key" problem (Bayboro 1/31/01)
	if (cancelled && (tvalid == t)) return;
	if (cancelled && valid) tvalid = TIME_NegInfinity;
	cancelled = FALSE;	

	// initialization for new collision scheme (Bayboro 2/5/01)
	cc.Init(cobjs);

	// The size of steps we take to integrate will be frame size steps.
	oneframe=GetTicksPerFrame();
	if (stepSize!=oneframe) 
	{	stepSize = oneframe;
		valid = FALSE;
	}
	float FperT=GetFrameRate()/(float)TIME_TICKSPERSEC;
    pblock->GetValue(PB_ANIMATIONOFFSET,t,anioff,FOREVER);
	pblock->GetValue(PB_EMITSTART,t,t0,FOREVER);
	pblock->GetValue(PB_SIZE,t,parts.size,FOREVER);	
	pblock->GetValue(PB_BIRTHMETHOD,0,bmethod,FOREVER);
	pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	pblock->GetValue(PB_EMITSTOP,0,t2,FOREVER);
//	t0-=oneframe;t2-=oneframe;
	if (bmethod)
	  pblock->GetValue(PB_PTOTALNUMBER,0,c,FOREVER);
	int subtree,frag,custmtl=0;
    pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
 	if (t < t0) {
		// Before the start time, nothing is happening
		parts.FreeAll();
		if (sdata) {delete[] sdata;sdata=NULL;}
		ResetSystem(t);
		return;
		}
	int pkind;
	float dpercent;
	pblock->GetValue(PB_DISPLAYPORTION,0,dpercent,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,pkind,FOREVER);

	if (!valid || t<tvalid || tvalid<t0) 
	{	// Set the particle system to the initial condition
	    TimeValue cincr;	
		dispt=-99999;
		if (!bmethod)
		{ c=0;
		  for (TimeValue ti=t0;ti<=t2;ti+=oneframe)
		  { pblock->GetValue(PB_PBIRTHRATE,ti,cincr,FOREVER);
		    if (cincr<0) cincr=0;
			c+=cincr;
		  }
		}
		if (!isrend) c=(int)(dpercent*(float)c+FTOIEPS);
		if (!ComputeParticleStart(t0,c))
		{ ResetSystem(t);
		  return;
		}
		dispt=t-1;
		maincount=parts.Count();
		ResetSystem(t,FALSE);
	}
	int total=maincount;
	valid = TRUE;
	int stype,maxgens,spmultb;float spawnbvar;
    pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
    pblock->GetValue(PB_SPAWNGENS,0,maxgens,FOREVER);
    pblock->GetValue(PB_SPAWNCOUNT,0,spmultb,FOREVER);
	pblock->GetValue(PB_SPINAXISTYPE,0,ptvel.axisentered,FOREVER);
    pblock->GetValue(PB_SPEEDDIR,0,ptvel.direntered,FOREVER);
    pblock->GetValue(PB_DIRX,0,ptvel.Dir.x,FOREVER);
    pblock->GetValue(PB_DIRY,0,ptvel.Dir.y,FOREVER);
    pblock->GetValue(PB_DIRZ,0,ptvel.Dir.z,FOREVER);
    pblock->GetValue(PB_DIRVAR,0,ptvel.DirVar,FOREVER);
	TimeValue dis;
    pblock->GetValue(PB_DISPUNTIL,0,dis,FOREVER);
	SpawnVars spawnvars;
	spawnvars.axisentered=ptvel.axisentered;

	if (t2<t0) t2=t0;
//	TimeValue fstep=oneframe;

	//t2+=fstep;
	TimeValue createover;
	createover=t2-t0+oneframe;
	counter=(isrend?rcounter:vcounter);
	float frate,grate;
	pblock->GetValue(PB_GROWTIME,0,grow,FOREVER);
    pblock->GetValue(PB_FADETIME,0,fade,FOREVER);
	frate=(fade>0.0f?(1-M)/fade:0.0f);
	grate=(grow>0.0f?(1-M)/grow:0.0f);
	float basesize;
	BOOL fullframe;
	if (!isrend)
	{ int offby=t%oneframe;
	  if (offby!=0) t-=offby;
	}
    pblock->GetValue(PB_SPAWNSCALESIGN,0,spawnvars.scsign,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDSIGN,0,spawnvars.spsign,FOREVER);
    pblock->GetValue(PB_SPAWNINHERITV,0,spawnvars.invel,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDFIXED,0,spawnvars.spconst,FOREVER);
    pblock->GetValue(PB_SPAWNSCALEFIXED,0,spawnvars.scconst,FOREVER);
    pblock->GetValue(PB_CREATEIN,0,ptvel.sym,FOREVER);
	if ((ptvel.sym==3)&&(!distnode)) ptvel.sym=0;
	int sper,spmult;float smper;BOOL first=(tvalid<t0);
	while ((tvalid < t)&&(tvalid<=dis))
	{	int born = 0;
	    TimeValue atvalid=abs(tvalid);
	    count=0;
		if (first) tvalid=t0;
			// Compute our step size
		 if (tvalid%stepSize !=0) 
		  { dt = stepSize - atvalid%stepSize;} 
		  else { dt = stepSize;	}
		  if (tvalid + dt > t) {dt = t-tvalid;}

  		  // Increment time
		if (!first) tvalid +=dt;
		if (tvalid>dis)
		{ for (j=0; j<parts.Count(); j++)
		  { parts.ages[j] = -1;  }
		  tvalid=t;
		  continue;
		}
	    pblock->GetValue(PB_SPAWNDIEAFTER,tvalid,ptvel.persist,FOREVER);
	    pblock->GetValue(PB_SPAWNDIEAFTERVAR,tvalid,ptvel.pvar,FOREVER);
	    pblock->GetValue(PB_SPEED,tvalid,ptvel.Speed,FOREVER);
    	pblock->GetValue(PB_SPEEDVAR,tvalid,ptvel.VSpeed,FOREVER);
		pblock->GetValue(PB_SIZE,tvalid,ptvel.Size,FOREVER);
		pblock->GetValue(PB_LIFE,tvalid,ptvel.Life,FOREVER);
		if (llist.Count()==0) deftime=ptvel.Life/oneframe;
		pblock->GetValue(PB_LIFEVAR,tvalid,ptvel.Vl,FOREVER);
		pblock->GetValue(PB_SPINTIME,tvalid,ptvel.Spin,FOREVER);
		pblock->GetValue(PB_SPINTIMEVAR,tvalid,ptvel.VSpin,FOREVER);
		pblock->GetValue(PB_SPINPHASE,tvalid,ptvel.Phase,FOREVER);
		pblock->GetValue(PB_SPINPHASEVAR,tvalid,ptvel.VPhase,FOREVER);
		pblock->GetValue(PB_SIZEVAR,tvalid,ptvel.VSz,FOREVER);
		pblock->GetValue(PB_METATENSION,tvalid,ptvel.bstr,FOREVER);
		pblock->GetValue(PB_METATENSIONVAR,tvalid,ptvel.bstrvar,FOREVER);
		pblock->GetValue(PB_BUBLAMP,tvalid,ptvel.ToAmp,FOREVER);
		pblock->GetValue(PB_BUBLAMPVAR,tvalid,ptvel.VToAmp,FOREVER);
		pblock->GetValue(PB_BUBLPHAS,tvalid,ptvel.ToPhase,FOREVER);
		pblock->GetValue(PB_BUBLPHASVAR,tvalid,ptvel.VToPhase,FOREVER);
		pblock->GetValue(PB_BUBLPER,tvalid,ptvel.ToPeriod,FOREVER);
		pblock->GetValue(PB_BUBLPERVAR,tvalid,ptvel.VToPeriod,FOREVER);
		pblock->GetValue(PB_EMITRHEIGHT,tvalid,ptvel.width,FOREVER);ptvel.width*=0.5f;
		pblock->GetValue(PB_EMITRWID,tvalid,ptvel.len,FOREVER);ptvel.len*=0.5f;
		pblock->GetValue(PB_EMITRDEPTH,tvalid,ptvel.depth,FOREVER);ptvel.depth*=0.5f;
		pblock->GetValue(PB_SPAWNDIRCHAOS,tvalid,spawnvars.dirchaos,FOREVER);
		pblock->GetValue(PB_SPAWNPERCENT,tvalid,sper,FOREVER);	
		pblock->GetValue(PB_SPAWNMULTVAR,tvalid,smper,FOREVER);	
		spawnbvar=smper*spmultb;
		pblock->GetValue(PB_SPAWNSPEEDCHAOS,tvalid,spawnvars.spchaos,FOREVER);
		spawnvars.spchaos/=100.0f;
		pblock->GetValue(PB_SPAWNSCALECHAOS,tvalid,spawnvars.scchaos,FOREVER);
		spawnvars.scchaos/=100.0f;
		pblock->GetValue(PB_SPINAXISX,tvalid,ptvel.Axis.x,FOREVER);
		pblock->GetValue(PB_SPINAXISY,tvalid,ptvel.Axis.y,FOREVER);
		pblock->GetValue(PB_SPINAXISZ,tvalid,ptvel.Axis.z,FOREVER);
		if (Length(ptvel.Axis)==0.0f) ptvel.Axis.x=0.001f;
		pblock->GetValue(PB_SPINAXISVAR,t,ptvel.axisvar,FOREVER);
		spawnvars.Axis=ptvel.Axis;spawnvars.axisvar=ptvel.axisvar;
		basesize=M*ptvel.Size;
		// Compute the number of particles that should be born LAST FRAME!
		birth=0;
		fullframe=(tvalid%tpf==0);
		if (fullframe)
		{ if (bmethod)
		  { int tdelta;
		    if (tvalid>=t2) birth=total-counter;
		    else
		    { tdelta=int((float)total*(tvalid-t0+oneframe)/createover);
		      birth=tdelta-counter;
		    }
	      }
		  else if (tvalid<=t2)
		  { pblock->GetValue(PB_PBIRTHRATE,tvalid,total,FOREVER);
		    if (!isrend) total=(int)(dpercent*(float)total+FTOIEPS);
		    birth=total;
			if (birth+counter>maincount) birth=0;
		  }
		}
		// First increment age and kill off old particles
		for (j=0;j<parts.Count();j++)
		{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
			{ valid=FALSE;tvalid=t;cancelled=TRUE;parts.FreeAll();return;}
		  if (!parts.Alive(j)) continue;
		  parts.ages[j] += dt;
		  if (parts.ages[j] >= sdata[j].L) 
		  {  if ((stype!=ONDEATH)||(sdata[j].gennum>=maxgens)||(sdata[j].SpVar>=sper)) parts.ages[j] = -1;	
		  else if (fullframe)
			 {spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
				if (spmult!=0) DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
				else parts.ages[j] = -1;
			 }
		  } else if (sdata[j].DL>-1) {sdata[j].DL+=dt;if (sdata[j].DL>sdata[j].persist) parts.ages[j]=-1;}
		  if (parts.ages[j]>-1)
		  { if (fullframe &&((stype==EMIT)&&(sdata[j].gennum==0)&&(sdata[j].SpVar<sper)))
		  {  spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
			  if (spmult!=0) DoSpawn(j,spmult,spawnvars,ptvel.Vl,TRUE);
		  }
//			if (pkind==RENDTYPE6)
//		     parts.radius[j]=ptvel.Size;		   else
			   if ((stype<2)||(maxgens==0))
		 	 parts.radius[j]=FigureOutSize(parts.ages[j],ptvel.Size,grow,fade,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
		   else 
		   { if (sdata[j].gennum==0)
			   parts.radius[j]=FigureOutSize(parts.ages[j],ptvel.Size,grow,0,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
		     else if (sdata[j].gennum==maxgens)
			   parts.radius[j]=FigureOutSize(parts.ages[j],sdata[j].Vsz,0,fade,sdata[j].L,grate,frate);
		   }
		  }
		}
		if (birth>0)
		{Matrix3 tm;	
		float stepCoef = 1.0f;
	    BOOL newmesh=(ptvel.sym==3)&&(distnode);
		Point3 *fnorms=NULL;TriObject *triOb=NULL;Matrix3 disttm=NULL;Object *pobj=NULL;
		TimeValue checkone=tvalid-t0,acheck=abs(checkone);
		if (newmesh!=NULL)
//		{	tm = distnode->GetObjTMAfterWSM((acheck<stepSize?t0:(tvalid-stepSize)));
//		{	tm = distnode->GetObjectTM((acheck<stepSize?t0:(tvalid-stepSize))); //change
//			stepCoef = acheck<stepSize ? float(tvalid-t0)/stepSize : 1.0;
		{	tm = distnode->GetObjectTM(tvalid-stepSize); //change
			stepCoef = 1.0;
		}
		else
//		{	tm = node->GetObjTMBeforeWSM((atvalid<stepSize?t0:(tvalid-stepSize))); //tune 
//		{	tm = node->GetObjTMBeforeWSM((acheck<stepSize?t0:(tvalid-stepSize)));
//			stepCoef = acheck<stepSize ? float(tvalid-t0)/stepSize : 1.0f;
		{	tm = node->GetObjTMBeforeWSM(tvalid-stepSize);
			stepCoef = 1.0f;
		}

//		if (newmesh!=NULL)	tm = distnode->GetObjTMAfterWSM(tvalid+stepSize);
		if (newmesh!=NULL)	tm = distnode->GetObjectTM(tvalid+stepSize); //change
		else tm = node->GetObjTMBeforeWSM(tvalid+stepSize);

		if (newmesh!=NULL)
//			tm = distnode->GetObjTMAfterWSM(tvalid);
			tm = distnode->GetObjectTM(tvalid); //change
		else 
			tm = node->GetObjTMBeforeWSM(tvalid);

		if (newmesh&&(born<birth)) 
		{ pobj = distnode->EvalWorldState(tvalid).obj;
		  triOb=TriIsUseable(pobj,tvalid);
		  if (triOb&&(triOb->GetMesh().getNumFaces()>0)) 
		  { int nf=triOb->GetMesh().getNumFaces();
			fnorms=new Point3[nf];
			Point3 v0, v1, v2;
			Face *face=triOb->GetMesh().faces;
			for (int fi=0; fi<nf; fi++) 
			{ v1 = triOb->GetMesh().verts[face[fi].v[1]];
			  fnorms[fi] = Normalize((v1-triOb->GetMesh().verts[face[fi].v[0]])^(triOb->GetMesh().verts[face[fi].v[2]]-v1));
			}
		  } 
		  else {
			ptvel.sym=0;newmesh=FALSE;
//		  	tm=node->GetObjTMBeforeWSM(acheck<stepSize?t0:(tvalid-stepSize));
		  	tm=node->GetObjTMBeforeWSM(tvalid-stepSize);
			tm=node->GetObjTMBeforeWSM(tvalid);
			}
		}

		ptvel.bps.Init((newmesh==NULL) ? node : distnode, tvalid, stepSize);

		// Next, birth particles at the birth rate
		for (j=counter; j<maincount; j++) {
			if (born>=birth) break;

			BirthParticle(node,tvalid,j,&ptvel,(newmesh?&triOb->GetMesh():NULL),fnorms,disttm);
//		    if ((pkind==RENDTYPE5)||(pkind==RENDTYPE6))
//			  parts.radius[j]=ptvel.Size;	else 
			parts.radius[j]=(grow>0?basesize:ptvel.Size)*(1+sdata[j].Vsz);
			sdata[j].themtl=int((tvalid-t0)*FperT);
			sdata[j].showframe=(anioff==1?0:sdata[j].showframe);
			born++;counter++;
			}
		if (triOb&&(triOb!=pobj)) triOb->DeleteThis();
		if (fnorms) delete[] fnorms;
		}
		// Apply forces to modify velocity
		int fc=fields.Count();
		if (fc>0)
		  for (j=0; j<parts.Count(); j++) 
		  { Point3 force,tvel=Zero;
		    for (i=0; i<fc; i++) 
			{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
			{ valid=FALSE;tvalid=t;cancelled=TRUE;parts.FreeAll();return;}
			if (!parts.Alive(j)) continue;
			 parts[j]=sdata[j].V;
			 force = fields[i]->Force(tvalid,parts[j],parts.vels[j],j);
			 float curdt=(float)dt;
			 if ((parts.ages[j]==0)&&(sdata[j].gennum==0)) curdt=tpf*(sdata[j].RV.x);
//			 if ((parts.ages[j]==0)&&(sdata[j].gennum==0)) curdt=tpf*((0.5f*sdata[j].RV.x)+0.5f);
			 tvel += 10.0f*force * curdt;
			}
		    parts.vels[j]+=tvel;
		}
		// Increment the positions
		count=0;
		// IPC IPC IPC IPC

		int IPC,ipcsteps;float B,Vb;
  		pblock->GetValue(PB_PCIPCOLLIDE_ON,tvalid,IPC,FOREVER);
  		pblock->GetValue(PB_PCIPCOLLIDE_STEPS,tvalid,ipcsteps,FOREVER);
  		pblock->GetValue(PB_PCIPCOLLIDE_BOUNCE,tvalid,B,FOREVER);
  		pblock->GetValue(PB_PCIPCOLLIDE_BOUNCEVAR,tvalid,Vb,FOREVER);
		if (IPC)
		{	CollideParticle cp;
			int ddt=dt/ipcsteps,remtime=0,snum=0;
			TimeValue curt=t;
			if (dt > 0)
				while (snum < ipcsteps)
				{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
				  {	valid=FALSE;
					tvalid=t;
					cancelled=TRUE;
					parts.FreeAll();
					return;
					}
					if (remtime==0) remtime=ddt;
					mindata md=cp.InterPartCollide(parts,cobjs,remtime,snum,B,Vb,curt,lc);
					for (j=0; j<parts.Count(); j++)
					{	if (parts.ages[j]>0) 
						{	if ((j!=md.min)&&(j!=md.min2)) 
							{	if (fullframe) 
									MovePart(j,md.mintime,fullframe,tpf);
								else 
									parts[j]+=parts.vels[j]*(float)md.mintime;
							}
							else if (fullframe) sdata[j].V=parts[j];
						}
					}
				}
			// If we didn't collide, then increment.
			for (j=0; j<parts.Count(); j++)
			{	sdata[j].Ts = (float)ptvel.Spin*sdata[j].Ts0;
				sdata[j].LamTs += (FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);
			}
		}
		else
		for (j=0; j<parts.Count(); j++)
		{  if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
			{ valid=FALSE;tvalid=t;cancelled=TRUE;parts.FreeAll();return;}
			if ((!parts.Alive(j))||(parts.ages[j]==0)) continue;
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
				{ if (sdata[j].persist==0) {parts.ages[j] = -1;count--;}else sdata[j].DL=0;}
				else if (fullframe &&((stype==COLLIDE)&&(sdata[j].gennum<maxgens)&&(sdata[j].SpVar<sper)))
				{ int spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
				  if (spmult!=0) DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
				}
			}
			sdata[j].Ts=(float)ptvel.Spin*sdata[j].Ts0;
			sdata[j].LamTs+=(FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);			
			// If we didn't collide, then increment.
			if (!collide) MovePart(j,dt,fullframe,tpf);
			else if (fullframe) sdata[j].V=parts[j];
		}
		if (first) first=FALSE;
	}
/*	if ((frag==METABALLS)&&(onscreen==2))
	{ float res,bstr,thres=0.6f;
      pblock->GetValue(PB_METATENSION,t,bstr,FOREVER);
      pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
	  if (cmesh) delete[] cmesh;cmesh=new Mesh;
      metap.CreateMetas(parts,cmesh,thres,res,bstr);
	}
	else */if ((frag==INSTGEOM)&&(onscreen>1)&&(custnode))
	{ TimeValue anist=GetAnimStart(),aniend=GetAnimEnd();
      thePCloudDraw.anifr=aniend+stepSize;
	  thePCloudDraw.t=t;
	  thePCloudDraw.anioff=anioff;
	  if (count>0)
	   GetTimes(times,t,thePCloudDraw.anifr,anioff);
	  else times.tl.ZeroCount();
	  if (onscreen==2)
	   GetMesh(t,subtree,custmtl);
	  else GetallBB(custnode,subtree,t);
	}  
	if (isrend) rcounter=counter;
	else
	{ vcounter=counter;
	}
	if (tvalid<t) tvalid=t;
	valid=TRUE;
//	assert(tvalid==t);
	}


void PCloudParticle::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	if (pmapPGen) pmapPGen->Invalidate();
	if (pmapPType) pmapPType->Invalidate();
	if (pmapPSpin) pmapPSpin->Invalidate();
	if (pmapEmitV) pmapEmitV->Invalidate();
	if (pmapBubl) pmapBubl->Invalidate();
	if (pmapSpawn) pmapSpawn->Invalidate();
	}

BOOL PCloudParticle::EmitterVisible()
	{
	int hide;
	pblock->GetValue(PB_EMITRHID,0,hide,FOREVER);
	return !hide;
	}

ParamDimension *PCloudParticle::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SPEED:
		case PB_PBIRTHRATE:
		case PB_PTOTALNUMBER:
		case PB_SIZE:
		case PB_RNDSEED:
		case PB_METATENSION:
		case PB_METACOURSE:
		case PB_MAPPINGDIST:
		case PB_SPINAXISX:
		case PB_SPINAXISY:
		case PB_SPINAXISZ:
		case PB_EMITVMULT:
		case PB_BUBLAMP:
		case PB_EMITRWID:
		case PB_EMITRHEIGHT:
		case PB_EMITRDEPTH:
									return stdWorldDim;

		case PB_SPINPHASE:
		case PB_SPINAXISVAR:
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
		case PB_SPAWNDIEAFTERVAR:
		case PB_PCIPCOLLIDE_BOUNCE:			
		case PB_PCIPCOLLIDE_BOUNCEVAR:			
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
		case PB_SPAWNDIEAFTER:
								return stdTimeDim;
		
		default: return defaultDim;
		}
	}

TSTR PCloudParticle::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_SPEED:			return GetString(IDS_RB_SPEED);
		case PB_SPEEDVAR:		return GetString(IDS_RB_SPEEDVAR);
		case PB_DIRX:			return GetString(IDS_AP_SPDIRX);
		case PB_DIRY:			return GetString(IDS_AP_SPDIRY);
		case PB_DIRZ:			return GetString(IDS_AP_SPDIRZ);
		case PB_DIRVAR:			return GetString(IDS_AP_SPDIRVAR);
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
		case PB_EMITRWID:		return GetString(IDS_AP_RADLEN);
		case PB_EMITRHEIGHT:	return GetString(IDS_RB_EMITRWID);
		case PB_EMITRDEPTH:		return GetString(IDS_AP_EMITRHEIGHT);
		case PB_OFFSETAMOUNT:	return GetString(IDS_AP_OFFSETAMT);
		case PB_SPAWNGENS:		return GetString(IDS_AP_SPAWNGENS);
		case PB_SPAWNCOUNT:		return GetString(IDS_AP_SPAWNCOUNT);
		case PB_SPAWNDIRCHAOS:	return GetString(IDS_AP_SPAWNDIRCHAOS);
		case PB_SPAWNSPEEDCHAOS:	return GetString(IDS_AP_SPAWNSPEEDCHAOS);
		case PB_SPAWNSCALECHAOS:	return GetString(IDS_AP_SPAWNSCALECHAOS);
		case PB_SPAWNLIFEVLUE:	return GetString(IDS_AP_SPAWNLIFEVLUE);
		case PB_STRETCH:	return GetString(IDS_AP_STRETCH);
		case PB_SPAWNDIEAFTER:	return GetString(IDS_AP_SPAWNDIEAFTER);
		case PB_SPAWNDIEAFTERVAR:	return GetString(IDS_AP_SPAWNDIEAFTERVAR);
		case PB_PCIPCOLLIDE_ON:			return GetString(IDS_AP_IPCOLLIDE_ON);
		case PB_PCIPCOLLIDE_STEPS:		return GetString(IDS_AP_IPCOLLIDE_STEPS);
		case PB_PCIPCOLLIDE_BOUNCE:		return GetString(IDS_AP_IPCOLLIDE_BOUNCE);
		case PB_PCIPCOLLIDE_BOUNCEVAR:	return GetString(IDS_AP_IPCOLLIDE_BOUNCEVAR);
		default: 				return TSTR(_T(""));
		}
	}	


void PCloudParticle::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{
	// particles may require update (bayboro|march.25.2002)
	if (!OKtoDisplay(t)) return;
	if (t!=tvalid) cancelled=FALSE;
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (doupdate) Update(t,inode);
	// end of particles may require update (bayboro|march.25.2002)

	cnode=inode;
	int type,ptype,disptype,createin;
	pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	pblock->GetValue(PB_CREATEIN,0,createin,FOREVER);
	if ((createin==3) && (!distnode)) type=0;
//	if ((type>2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype>0)&&(!custnode))  )) type=1;
	if ((type>2)&&(((disptype>0)&&(!custnode))  )) type=1;
	if ((type==3)&&((disptype!=INSTGEOM)||(!custnode))) type=1;
	if (type==2) 
	{ Box3 pbox;
		Matrix3 mat;
		mat = inode->GetObjTMBeforeWSM(t);
		UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box=box*mat;
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
	else if (type==3) 
	{ Box3 pbox;
		Matrix3 mat;
/*		if (createin)
		{ mat=distnode->GetObjTMAfterWSM(t);
		  box.pmin=box.pmax=Zero;
		} else */
		{ UpdateMesh(t);
		  mat = inode->GetObjTMBeforeWSM(t);
		  box  = mesh.getBoundingBox();
		}
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt;
			float radius=parts.radius[i];
			int axisentered,K;
			pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
			 if ((ptype!=RENDTYPE5)&&(ptype!=RENDTYPE6))
			 {if (axisentered==DIRTRAVEL)
			{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			  float strlen=GetLen(parts.vels[i],K);
			  radius*=strlen;
			 }}
			int n=0,found=0;
			TimeValue Ctime=(thePCloudDraw.anioff?GetCurTime(sdata[i].showframe,(thePCloudDraw.anioff>1?thePCloudDraw.t:parts.ages[i]),thePCloudDraw.anifr):thePCloudDraw.t);
			int nCount=nlist.Count();
		found=((n=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum)))>-1);
	  for (int nb=0;nb<thePCloudDraw.bboxpt[n].numboxes;nb++)
	  { for (int j=0;j<8;j++)
		{ pt=(radius*(thePCloudDraw.bboxpt[n].bpts[nb].pts[j]+thePCloudDraw.bboxpt[n].bpts[nb].Suboffset))+parts[i];
			  pbox += pt;
			}
		}
		}
		if (!pbox.IsEmpty()) box += pbox;
	  }
	else {SimpleParticle::GetWorldBoundBox(t,inode,vpt,box);}
	}

/*BOOL PCloudParticleDraw::DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i)
{ HCURSOR hCur;
  BOOL chcur=FALSE;
if (disptype==METABALLS)
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
	  if (GetAsyncKeyState (VK_ESCAPE)) return TRUE;
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
    Point3 pt[4],W;  
	int numF,numV;	
    GetMeshInfo(ptype,1,pm,&numF,&numV);
	if (indir.inaxis)
	    indir.vel=parts.vels[i];
    float elapsedtime=(float)parts.ages[i];
    float Angle=(FloatEQ0(obj->sdata[i].Ts)?0.0f:elapsedtime/obj->sdata[i].Ts)+obj->sdata[i].LamTs;
	W=obj->sdata[i].W;
	if (ptype==RENDTYPE1) PlotTriangle(parts.radius[i],0,0,pm,Angle,&W.x,0,&parts.points[i],indir);
	else if (ptype==RENDTYPE2) PlotCube8(parts.radius[i],0,0,pm,Angle,&W.x,0,&parts.points[i],indir);
	else if (ptype==RENDTYPE3) PlotSpecial(parts.radius[i],0,0,pm,Angle,&W.x,0,&parts.points[i],indir);
	else if (ptype==RENDTET) PlotTet(parts.radius[i],0,0,pm,Angle,W,0,&parts.points[i],indir);
	else if (ptype==REND6PT) Plot6PT(parts.radius[i],0,0,pm,Angle,&W.x,0,&parts.points[i],indir);
   int nfaces=pm->getNumFaces();
   for (int j=0;j<nfaces;j++)
    { pt[0]=pm->verts[pm->faces[j].v[0]];
      pt[1]=pm->verts[pm->faces[j].v[1]];
	  pt[2]=pm->verts[pm->faces[j].v[2]];
      gw->polyline(3,pt,NULL,NULL,TRUE,NULL);
    }
  if (pm) delete pm;
	  if (GetAsyncKeyState (VK_ESCAPE)) return TRUE;
}
return 0;
}*/	
BOOL PCloudParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
{ HCURSOR hCur;
  BOOL chcur=FALSE;
  Point3 pt[4];  
  if (!((disptype==INSTGEOM)&&(bb))) return TRUE;
  if (obj->custnode)
  { int n=0,found=0;
    TimeValue Ctime=(anioff?GetCurTime(obj->sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	int nCount=obj->nlist.Count();
	found=((n=TimeFound(obj->times,Ctime,(obj->sdata[i].gennum>nCount?nCount:obj->sdata[i].gennum)))>-1);
	float radius=parts.radius[i];
	float elapsedtime=(float)parts.ages[i];
    float Angle=obj->sdata[i].LamTs;
	if (indir.inaxis)
	  indir.vel=parts.vels[i];
	Point3 nextpt;
	if (found) 
	{ Point3 pt[9];
	  if (bboxpt[n].numboxes*8>LOTSOFACES)
	  { hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	    chcur=TRUE;
	  }
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
  }
  if (GetAsyncKeyState (VK_ESCAPE)) 
  { if (chcur) SetCursor(hCur);	return TRUE;}
return 0;
}

int GetDrawType(PCloudParticle *po, int &ptype,int &disptype)
{	int type;
	po->pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
	po->pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
	po->pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	TimeValue aniend;

//	if ((type>2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype>0)&&(!po->custnode))  )) type=1;
	if ((type>2)&&(((disptype>0)&&(!po->custnode))  )) type=1;
	if ((type==3)&&((disptype!=INSTGEOM)||(!po->custnode))) type=1;
	if (type>1)
	{ aniend=GetAnimEnd();
	  int oneframe=GetTicksPerFrame();
      po->thePCloudDraw.anifr=aniend+oneframe;
	   int axisentered;
		po->pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	    if (po->thePCloudDraw.indir.inaxis=(axisentered==DIRTRAVEL))
	    { po->pblock->GetValue(PB_STRETCH,0,po->thePCloudDraw.indir.K,FOREVER);
		  po->thePCloudDraw.indir.oneframe=oneframe;
	    }
	}
	return type;
}


MarkerType PCloudParticle::GetMarkerType() 
{int ptype,disptype,type=GetDrawType(this,ptype,disptype);
	switch (type) {
		case 0:
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;			
		case 1:
			parts.SetCustomDraw(NULL);
			return PLUS_SIGN_MRKR;
		case 2:	{			
			thePCloudDraw.obj = this;
			thePCloudDraw.firstpart=TRUE;
			thePCloudDraw.disptype=disptype;
			thePCloudDraw.ptype=ptype;
			parts.SetCustomDraw(&thePCloudDraw);			
			thePCloudDraw.bb=FALSE;
			return POINT_MRKR;
			}	 
		case 3:{thePCloudDraw.obj = this;
			thePCloudDraw.firstpart=TRUE;
			thePCloudDraw.disptype=disptype;
			thePCloudDraw.ptype=ptype;
			thePCloudDraw.bb=TRUE;
			parts.SetCustomDraw(&thePCloudDraw);			
			return POINT_MRKR;
		   }
		default:
			return PLUS_SIGN_MRKR;
		}
	}



//--- PCloud particle -----------------------------------------------

RefTargetHandle PCloudParticle::Clone(RemapDir& remap) 
	{
	PCloudParticle* newob = new PCloudParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	if (distnode) newob->ReplaceReference(DISTNODE,distnode);
	if (mbase) newob->ReplaceReference(MBASE,mbase);
	newob->nlist.SetCount(nlist.Count());
	newob->llist.SetCount(llist.Count());
	for (int ix=0;ix<nlist.Count();ix++) 
	{ newob->nlist[ix]=NULL;
	  newob->ReplaceReference(ix+BASER,nlist[ix]);
	}
	for (ix=0;ix<llist.Count();ix++) newob->llist[ix]=llist[ix];
	newob->createmeth=createmeth;
	newob->custname = custname;
	newob->distnode = distnode;
	newob->distname = distname;
	newob->normname = normname;
	newob->mvalid.SetEmpty();	
	newob->tvalid = FALSE;
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
	 newob->GetTimes(newob->times,t,anifr,anioff);
	 if (vshow==2)
	 { pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	   newob->GetMesh(t,subtree,custmtl);
	 }
	 else newob->GetallBB(newob->custnode,subtree,t);
	}
	BaseClone(this, newob, remap);
	return newob;
	}

void CacheSpin(float *holddata,PCSavePt *sdata,int pcount,BOOL issave)
{ for (int i=0;i<pcount;i++)
	if (issave) holddata[i]=sdata[i].LamTs;
	else sdata[i].LamTs=holddata[i];
}

static float findmappos(float curpos)
{ float mappos;

  return(mappos=((mappos=curpos)<0?0:(mappos>1?1:mappos)));
}

Mesh *PCloudParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
{	float Mval, Uval, Vval,Wval,Angle,elapsedtime;
	int type, count,maptype,anioff,anifr;
	float width;
	Point3 Center;
	TVFace defface;
	Tab<int> tVertsOffset;
	MultipleChannelMapMerger mcmm;
	BOOL mirror=FALSE;
	TimeValue mt,aniend=GetAnimEnd();
 	Mesh *pm = new Mesh;
	if (cancelled) {ZeroMesh(pm);mesh.InvalidateGeomCache();
	needDelete = TRUE;	return pm;}
    anifr=aniend+GetTicksPerFrame();
	dispt=t;
	int nummtls=0,curmtl=0,multi=0,pc=0,custmtl=0;
	pblock->GetValue(PB_PARTICLETYPE,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,pc,FOREVER);
	if (pc==METABALLS) type=RENDMETA;
	else if (pc==INSTGEOM)
	{if (custnode==NULL) {type=0;pc=ISSTD;} else type=RENDGEOM;
	pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	}
	float FperT=GetFrameRate()/(float)TIME_TICKSPERSEC;
	pblock->GetValue(PB_EMITRHEIGHT,t,width,FOREVER);
	pblock->GetValue(PB_MAPPINGTYPE,t,maptype,FOREVER);
	if (maptype)
	 pblock->GetValue(PB_MAPPINGDIST,t,Mval,FOREVER);
	else 
	 pblock->GetValue(PB_MAPPINGTIME,0,mt,FOREVER);
	int createin;
	pblock->GetValue(PB_CREATEIN,0,createin,FOREVER);
    BOOL weirdCenter=(createin==3)&&(distnode);

	int isrend=!TestAFlag(A_NOTREND);
//my comment out below - ecp
//	if ((!isrend)&&((type==RENDTYPE5)||(type==RENDTYPE6))) type=RENDTYPE1;
	Matrix3 wcam,cam=ident;
	Point3 v, v0,v1, otherV=Zero,camV=Zero;
	if (isrend)
	{ 
		cam= Inverse(wcam=view.worldToView);
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

//	Matrix3 tm = inode->GetObjTMAfterWSM(t);
	Matrix3 tm = inode->GetObjectTM(t); //change
	Matrix3 itm = Inverse(tm);
//	if (weirdCenter) tm=distnode->GetObjTMAfterWSM(t);
	if (weirdCenter) tm=distnode->GetObjectTM(t); //change
	int vertexnum=0, face=0,numV=0,numF=0,j=0,tvnum=0,ismapped=0;
	ParticleSys lastparts;
	TimeValue offtime=t%GetTicksPerFrame();
	BOOL midframe;
	midframe=offtime>0;
	float *holddata=NULL;int pcount=0;
	if (isrend)
	{ if (midframe) 
	{ Update(t-offtime,inode);
	  if ((pcount=parts.Count())>0)
	  {	CacheData(&parts,&lastparts);
		holddata=new float[pcount];
		CacheSpin(holddata,sdata,pcount,TRUE);
	  }
	}
		Update(t,inode);
	}
	count = CountLive();
	Center = Zero* tm;
	float Thetah;
	Point3 r1;
	if (count==0) {ZeroMesh(pm);goto quit;}
	else
	{mirror=DotProd(tm.GetRow(0)^tm.GetRow(1),tm.GetRow(2))<0.0f;
		if (type==RENDTYPE6)
	  { if (view.projType) type=RENDTYPE5;
	    else
	    { Thetah=view.fov;
	      r1=cam.GetRow(1);
	    }
	  }
	int gtvnum=0;
	if (pc<METABALLS) GetMeshInfo(type,count,pm,&numF,&numV);
	else if (type==RENDGEOM)
	{ int subtree,onscreen;
	  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
//	  if (onscreen!=2) { 
	  thePCloudDraw.t=t;
	  GetTimes(times,t,anifr,anioff);
	  GetMesh(t,subtree,custmtl);
//	  }
	  if (backpatch)
	  { int custmtl,frag;
        pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
        pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	    if ((custnode)&&(frag==INSTGEOM)&& custmtl) 
		  GetSubs(inode,custnode,subtree,t);}
	  TimeValue Ctime;
	  int mnum,tmptvs=0;
	  numV=0;numF=0;
	  BOOL alltex=TRUE;
	  for (int pcnt=0;pcnt<parts.Count();pcnt++)
	  { if (GetAsyncKeyState (VK_ESCAPE)) 
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
//	  pm->setNumTVerts(gtv=(custmtl?gtvnum:count*numV));
	  pm->setNumTVerts(gtv=gtvnum);
	  if ((custmtl)&&(gtvnum>0))
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
	  pm->setNumTVFaces(gtv>0?numF:0);

	  // Multiple Channel Map Support (single line)
	  if (custmtl) mcmm.setNumTVertsTVFaces(pm, cmesh, mcnt);
	}
	Uval=0.5f;Wval=0.5f;
	Point3 ipt;
	int i;
	if (count>0)
	{
	if (type==RENDMETA)
	{ float res,bstr,thres=0.6f;int notdraft;
      pblock->GetValue(PB_METATENSION,t,bstr,FOREVER);
      pblock->GetValue(PB_PCNOTDRAFT,t,notdraft,FOREVER);
	  notdraft = (notdraft?0:1);
	  if (isrend)
		pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
	  else pblock->GetValue(PB_METACOURSEV,0,res,FOREVER);
	  metap.CreateMetas(parts,pm,thres,res,bstr,notdraft);	
	  for (int j=0;j<pm->getNumVerts();j++)
         pm->verts[j] = itm * pm->verts[j];
 	}
	else
	{ Mtl *mtl;
	  mtl=inode->GetMtl();
	  if (mtl)
	  { Class_ID mc=Class_ID(MULTI_CLASS_ID,0);
	    Class_ID oc=mtl->ClassID();
	    if (multi=(oc==mc))
	    { nummtls=mtl->NumSubMtls();
		  if (nummtls==0) multi=0;
		}
	  }
	wasmulti=multi;
	InDirInfo indir;indir.vel=Zero;indir.K=0;indir.oneframe=GetTicksPerFrame();
	int axisentered;
    pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	if (indir.inaxis=(axisentered==DIRTRAVEL))
	{ 	  pblock->GetValue(PB_STRETCH,0,indir.K,FOREVER);
	}
	MtlID cm;
    for (i=0; i<parts.Count(); i++) 
	{ if (GetAsyncKeyState (VK_ESCAPE)) 
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(i)) continue;
	    if (multi) 
		{ curmtl=sdata[i].themtl;
	      if (curmtl>=nummtls) 
		    curmtl=curmtl % nummtls;
	    } else curmtl=i;
	  if (indir.inaxis)
	    indir.vel=parts.vels[i];
		float x;
	    elapsedtime=(float)parts.ages[i];
		if (maptype) Vval=(x=Length(parts[i]-Center))/Mval;
		else Vval=(float)elapsedtime/mt;
        Angle=sdata[i].LamTs;
	    if (type==RENDTYPE1) PlotTriangle(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTYPE2) PlotCube8(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTYPE3) PlotSpecial(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
	    else if (type==RENDTET) PlotTet(parts.radius[i],vertexnum,face,pm,Angle,sdata[i].W,curmtl,&parts.points[i],indir);
	    else if (type==REND6PT) Plot6PT(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
		else if (type==RENDSPHERE) PlotSphere(parts.radius[i],vertexnum,face,pm,Angle,&sdata[i].W.x,curmtl,&parts.points[i],indir);
		else if (type==RENDTYPE5)
	    {   v  = (view.projType?otherV:Normalize(camV-parts[i]));
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
		 else if (type==RENDGEOM)
		 { TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
		 int nCount=nlist.Count();
		 int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nCount?nCount:sdata[i].gennum));
		   if (mnum<0) continue;
		   numF=cmesh[mnum].getNumFaces();
		   numV=cmesh[mnum].getNumVerts();
			if ((gtvnum>0)&&(custmtl))
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
		  { Face cface=cmesh[mnum].faces[j];
			pm->faces[jf].flags=cface.flags;
		    pm->faces[jf].setSmGroup(cface.smGroup);
		    pm->faces[jf].setVerts(vertexnum+cface.v[0],vertexnum+cface.v[1],vertexnum+cface.v[2]);
		    if (!custmtl)
		      pm->faces[jf].setMatID((MtlID)curmtl); 
		    else
			{cm=cface.getMatID();
			 int mtlgen=times.tl[mnum].gennum-1,maxmtl=nmtls.Count();
			 if (mtlgen>=maxmtl) mtlgen=maxmtl-1;
			 if ((mtlgen>-1)&&((times.tl.Count()>0)&&(times.tl[mnum].gennum>0)))
				cm+=nmtls[mtlgen];
			 pm->faces[jf].setMatID(cm);
			}
			pm->faces[jf].setEdgeVisFlags(cface.getEdgeVis(0),cface.getEdgeVis(1),cface.getEdgeVis(2));
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
		if ((type!=RENDTET)&&(!custmtl))
		{ pm->tVerts[tvnum]=Point3(findmappos(Uval),findmappos(Vval),findmappos(Wval));
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
quit:mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;
}

const TCHAR *PCloudClassDesc::ClassName ()	{return GetString(IDS_AP_PCLOUD);}
const TCHAR *PCloudClassDesc::Category ()	{return GetString(IDS_RB_PARTICLESYSTEMS);}
TCHAR *PCloudParticle::GetObjectName() {return GetString(IDS_AP_PCLOUDGC);}


void PCloudParticle::GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype)
{ int m,n,found,tmax,tnums=0,tgen;
  TimeValue tframe;
  times.tl.SetCount(0);times.tl.Shrink();
  times.tl.Resize(100);tmax=100;times.tl.SetCount(tmax);
  for (m=0;m<parts.Count();m++)
  { if (!parts.Alive(m)) continue;
    if (ltype)
	{ if (ltype==1) tframe=sdata[m].showframe+parts.ages[m];
	  else tframe=sdata[m].showframe+t;
//      if ((tframe>=anifr)&&(anifr!=0)) tframe=tframe % anifr;
	} else tframe=t;
	found=n=0;
	int nCount=nlist.Count();
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

BOOL PCPickOperand::Filter(INode *node)
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

BOOL PCPickOperand::HitTest(
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

void PCloudParticle::ShowName(int dist)
{ TSTR name;
  if (dist==shownormname)
  { FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (mbase ? normname : TSTR(GetString(IDS_AP_NONE))));
    if (hgen) SetWindowText(GetDlgItem(hgen, IDC_AP_PCLOUDOBJDIRNAME), name);}
  else if (dist) 
  { FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (distnode ? distname : TSTR(GetString(IDS_AP_NONE))));
    if (hparam) SetWindowText(GetDlgItem(hparam, IDC_AP_FILLPICKOBJECT), name);}
  else
  { FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE))));
    if (hptype) SetWindowText(GetDlgItem(hptype, IDC_AP_INSTANCESRCNAME), name);
  }
}

BOOL PCPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);

	INodeTab nodes;
	int subtree;
//	if (dodist) {nodes.SetCount(1);nodes[0]=node;}
	 if (dodist==1)
	  {	theHold.Begin();
		theHold.Put(new CreatePCPartPickNode(po,po->distname,node->GetName(),1));
		if (po->distnode) po->ReplaceReference(DISTNODE,node,TRUE);
	    else po->MakeRefByID(FOREVER,DISTNODE,node);	
		theHold.Accept(GetString(IDS_AP_COMPICK));
	    po->distname = TSTR(node->GetName());
		po->createmeth=3;
		po->pblock->SetValue(PB_CREATEIN,0,po->createmeth);
		SpinnerOff(po->hparam,IDC_AP_EMITHGTSPIN,IDC_AP_EMITHGT);
		SpinnerOff(po->hparam,IDC_AP_EMITDEPSPIN,IDC_AP_EMITDEP);
		EnableWindow(GetDlgItem(po->hparam,IDC_AP_EMITHGT_TXT),FALSE);
		EnableWindow(GetDlgItem(po->hparam,IDC_AP_EMITDEP_TXT),FALSE);
	    po->ShowName(dodist);	
	  }
	  else
	  { po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  if (node->IsGroupMember()) 
	   while (node->IsGroupMember()) node=node->GetParentNode();
	  nodes.SetCount(0);
	  if (po->flags=(node->IsGroupHead()?1:0)) MakeGroupNodeList(node,&nodes,subtree,ip->GetTime());
	  else MakeNodeList(node,&nodes,subtree,ip->GetTime());
	  if (dodist==0)
	  {	theHold.Begin();
		theHold.Put(new CreatePCPartPickNode(po,po->custname,node->GetName(),0));
		if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	    else po->MakeRefByID(FOREVER,CUSTNODE,node);	
		theHold.Accept(GetString(IDS_AP_COMPICK));
	    po->custname = TSTR(node->GetName());
		po->ShowName(dodist);	
	  }
	  else if (dodist==2)
	 {  theHold.Begin();
		theHold.Put(new PCObjectListRestore(po));
		po->AddToList(node,po->nlist.Count(),TRUE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
	  else 
	  { theHold.Begin();
		theHold.Put(new PCObjectListRestore(po));
	    po->AddToList(node,repi,FALSE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
  	  // Automatically check show result and do one update
		int frag,custmtl,onscreen;
		po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
		po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
		po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
		if ((frag==INSTGEOM)&&(onscreen>1))
		  if (onscreen==2)
			po->GetMesh(ip->GetTime(),subtree,custmtl);
		  else po->GetallBB(node,subtree,ip->GetTime());
	} 
	ip->FlashNodes(&nodes);
	nodes.ZeroCount();nodes.Shrink();
	po->valid=FALSE;
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	if (po->creating) {
		theCreatePCloudMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreatePCloudMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void PCPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	if (dodist>=2)
	{ iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==3?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
	  TurnButton(po->spawn,(dodist==2?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK),FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
	  TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
	  TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,FALSE);
	}
    else if (dodist)
	{ iBut=GetICustButton(GetDlgItem(po->hparam,IDC_AP_FILLPICKBUTTON));
	  TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPESTD),FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEMET),FALSE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEINSTANCE),FALSE);
	}
	else
	{iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	}
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void PCPickOperand::ExitMode(IObjParam *ip)
	{if (!po->ip) return;
	ICustButton *iBut;
	if (dodist>=2)
	{ iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==3?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
	  PCCheckInstButtons(po->pblock,po->hptype);
	  TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,TRUE);
	  if (dodist==3)
	   TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,TRUE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	}
	else if (dodist) 
	{ iBut=GetICustButton(GetDlgItem(po->hparam,IDC_AP_FILLPICKBUTTON));
	  PCCheckInstButtons(po->pblock,po->hptype);
	  PCCheckSpawnButtons(po->pblock,po->spawn,repi);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPESTD),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEMET),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPECHUNKS),TRUE);
	  EnableWindow(GetDlgItem(po->hptype,IDC_SP_TYPEINSTANCE),TRUE);
	}
	else
	{ iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  TurnButton(po->hparam,IDC_AP_FILLPICKBUTTON,TRUE);
	  PCCheckSpawnButtons(po->pblock,po->spawn,repi);
	}
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}

BOOL PCPickNorm::Filter(INode *node)
	{
	return node ? TRUE : FALSE;
	}

BOOL PCPickNorm::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	return node ? TRUE : FALSE;
	}

BOOL PCPickNorm::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);
	INodeTab nodes;
	nodes.SetCount(1);nodes[0]=node;
	ip->FlashNodes(&nodes);
	theHold.Begin();
	theHold.Put(new CreatePCPartPickNode(po,po->normname,node->GetName(),shownormname));
	if (po->mbase) po->ReplaceReference(MBASE,node,TRUE);
	else po->MakeRefByID(FOREVER,MBASE,node);
	theHold.Accept(GetString(IDS_AP_COMPICK));
	po->normname = TSTR(node->GetName());
	po->ShowName(shownormname);
	po->valid=FALSE;
	
	if (po->creating) {
		theCreatePCloudMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreatePCloudMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void PCPickNorm::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(po->hgen,IDC_AP_OBJECTDIRPICK));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PCPickNorm::ExitMode(IObjParam *ip)
	{
	if (po->hgen)
	{	ICustButton *iBut = GetICustButton(GetDlgItem(po->hgen,IDC_AP_OBJECTDIRPICK));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}
	}

RefTargetHandle PCloudParticle::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case DISTNODE: return (RefTargetHandle)distnode;
		case CUSTNODE: return (RefTargetHandle)custnode;
		case MBASE: return (RefTargetHandle)mbase;
		default: return (RefTargetHandle)nlist[i-BASER];
		}
	}

void PCloudParticle::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case DISTNODE: distnode = (INode *)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		case MBASE: mbase = (INode *)rtarg; return;
		default: nlist[i-BASER]= (INode *)rtarg;return;
		}
	}

RefResult PCloudParticle::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_TARGET_DELETED:	
			{ if (hTarget==custnode) 
				{if (theHold.Holding()) theHold.Put(new CreatePCPartDelNode(this,custnode->GetName(),0));
				custnode=NULL;custname=TSTR(_T(" "));cancelled=FALSE;
				}
			  if (hTarget==distnode) {if (theHold.Holding()) theHold.Put(new CreatePCPartDelNode(this,distnode->GetName(),1));
				  distnode=NULL;distname=TSTR(_T(" "));cancelled=FALSE;
				createmeth=0;
				pblock->SetValue(PB_CREATEIN,0,createmeth);
			  }
			  if (hTarget==mbase) {if (theHold.Holding()) theHold.Put(new CreatePCPartDelNode(this,mbase->GetName(),shownormname));mbase=NULL;normname=TSTR(_T(" "));cancelled=FALSE;}
			  BOOL notfound=TRUE; 
			  for (int i=0;(i<nlist.Count())&&(notfound);i++)
				if (hTarget==nlist[i]) 
				{ DeleteFromList(i,TRUE);
				  notfound=FALSE;cancelled=FALSE;}
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ if (hTarget==custnode) 
			  { custname = TSTR(custnode->GetName());
			    ShowName(0);cancelled=FALSE;
				}
			  if (hTarget==distnode) 
			  { distname = TSTR(distnode->GetName());
			    ShowName(1);cancelled=FALSE;
				}
			  if (hTarget==mbase) 
			  { normname = TSTR(mbase->GetName());
			    ShowName(shownormname);cancelled=FALSE;
				}
			  BOOL notfound=TRUE;
			  for (int i=0;(i<nlist.Count())&&(notfound);i++)
				if (hTarget==nlist[i]) 
			      {notfound=FALSE;SetUpList();cancelled=FALSE;}
			  break;
			  			}
		case REFMSG_NODE_LINK:		
		case REFMSG_CHANGE:
			{ int pblst=0;
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
			break;
			}
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
			if (editOb==this) InvalidateUI();
//			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			break;
		default: SimpleParticle::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}

class PCloudPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		PCloudPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((PCloudParticle*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			int mc;
			if (oldVer<1) {	
				((PCloudParticle*)targ)->pblock->GetValue(PB_METACOURSE,0,mc,FOREVER);
				((PCloudParticle*)targ)->pblock->SetValue(PB_METACOURSEV,0,mc);
				}
			if (oldVer<2) {	
				((PCloudParticle*)targ)->pblock->SetValue(PB_SPAWNPERCENT,0,100);
				}
			if (oldVer<3) {	
				((PCloudParticle*)targ)->pblock->SetValue(PB_PCNOTDRAFT,0,0);
				}
			if (oldVer<4) {	
				((PCloudParticle*)targ)->pblock->SetValue(PB_SPAWNDIEAFTER,0,0);
				((PCloudParticle*)targ)->pblock->SetValue(PB_SPAWNDIEAFTERVAR,0,0.0f);
				}
			if (oldVer<5)
			{ ((PCloudParticle*)targ)->pblock->SetValue(PB_PCIPCOLLIDE_ON,0,0);
			  ((PCloudParticle*)targ)->pblock->SetValue(PB_PCIPCOLLIDE_STEPS,0,2);
			  ((PCloudParticle*)targ)->pblock->SetValue(PB_PCIPCOLLIDE_BOUNCE,0,1.0f);
			  ((PCloudParticle*)targ)->pblock->SetValue(PB_PCIPCOLLIDE_BOUNCEVAR,0,0.0f);
			}
			delete this;
			}
	};

#define PC_CUSTNAME_CHUNK	0x0100
#define PC_DISTNAME_CHUNK	0x0200
#define PC_CUSTFLAGS_CHUNK	0x0300
#define PC_SPAWNC_CHUNK		0x0400
#define PC_LCNT_CHUNK		0x0500
#define PC_LIFE_CHUNK		0x0600
#define PC_NORMNAME_CHUNK	0x0700

IOResult PCloudParticle::Save(ISave *isave)
	{ 	ULONG nb;

	isave->BeginChunk(PC_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();

	isave->BeginChunk(PC_DISTNAME_CHUNK);		
	isave->WriteWString(distname);
	isave->EndChunk();

	isave->BeginChunk(PC_CUSTFLAGS_CHUNK);		
	isave->Write(&flags,sizeof(flags),&nb);
	isave->EndChunk();

	int nCount=nlist.Count();
	isave->BeginChunk(PC_SPAWNC_CHUNK);		
	isave->Write(&nCount,sizeof(nCount),&nb);
	isave->EndChunk();

	int Lcnt=llist.Count();
	isave->BeginChunk(PC_LCNT_CHUNK);		
	isave->Write(&Lcnt,sizeof(Lcnt),&nb);
	isave->EndChunk();

	isave->BeginChunk(PC_LIFE_CHUNK);
	for (int i=0;i<Lcnt;i++)
	{ isave->Write(&llist[i],sizeof(int),&nb);
	}
	isave->EndChunk();

	isave->BeginChunk(PC_NORMNAME_CHUNK);		
	isave->WriteWString(normname);
	isave->EndChunk();

	return IO_OK;
	}

IOResult PCloudParticle::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	iload->RegisterPostLoadCallback(
			new PCloudPostLoadCallback(
				new ParamBlockPLCB(pcversions,NUM_OLDVERSIONS,&curVersionPC,this,0)));
	int cnmtl=0;
	// Default names
	custname = TSTR(_T(" "));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case PC_CUSTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
				}
			case PC_DISTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				distname = TSTR(buf);
				break;
				}
			case PC_CUSTFLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;
			case PC_SPAWNC_CHUNK:
				{  int nCount;
					res=iload->Read(&nCount,sizeof(nCount),&nb);
					nlist.SetCount(nCount);
					for (int i=0; i<nCount; i++) nlist[i] = NULL;
				}
				break;
			case PC_LCNT_CHUNK:
				{	int Lcnt;
					res=iload->Read(&Lcnt,sizeof(Lcnt),&nb);
					llist.SetCount(Lcnt);
					for (int i=0; i<Lcnt; i++) llist[i] = NULL;
				}
				break;
			case PC_LIFE_CHUNK:
				{	for (int i=0;i<llist.Count();i++)
					res=iload->Read(&llist[i],sizeof(int),&nb);}
				break;
			case PC_NORMNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				normname = TSTR(buf);
				break;
				}
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}

PCloudParticle::PCloudParticle()
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
     int tpf=GetTicksPerFrame();

	MakeRefByID(FOREVER, PBLK, CreateParameterBlock(spdescVer5, PBLOCK_LENGTH_PCLOUD, CURRENT_VERSION));
	pblock->SetValue(PB_SPEED,0,0.0f);
	pblock->SetValue(PB_SPEEDVAR,0,0.0f);
	pblock->SetValue(PB_PBIRTHRATE,0,10);
	pblock->SetValue(PB_PTOTALNUMBER,0,100);
	pblock->SetValue(PB_BIRTHMETHOD,0,0);
	pblock->SetValue(PB_DISPLAYPORTION,0,1.0f);
	pblock->SetValue(PB_EMITSTART,0,0);
	pblock->SetValue(PB_EMITSTOP,0,0);// correct constant?
	pblock->SetValue(PB_DISPUNTIL,0,100*FToTick);// correct constant?
	pblock->SetValue(PB_LIFE,0,101*FToTick);// correct constant?
	pblock->SetValue(PB_LIFEVAR,0,0.0f);
	pblock->SetValue(PB_SIZE,0,1.0f);
	pblock->SetValue(PB_SIZEVAR,0,0.0f);
	pblock->SetValue(PB_GROWTIME,0,0);
	pblock->SetValue(PB_FADETIME,0,0);
	pblock->SetValue(PB_RNDSEED,0,12345);

	pblock->SetValue(PB_PARTICLETYPE,0,0);
	pblock->SetValue(PB_METATENSION,0,1.0f);
	pblock->SetValue(PB_METATENSIONVAR,0,0.0f);
	pblock->SetValue(PB_METAAUTOCOARSE,0,1);
	pblock->SetValue(PB_METACOURSE,0,0.5f);
	pblock->SetValue(PB_METACOURSEV,0,1.0f);
	pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	pblock->SetValue(PB_MAPPINGTYPE,0,0);
	pblock->SetValue(PB_MAPPINGTIME,0,30*FToTick);
	pblock->SetValue(PB_MAPPINGDIST,0,100.0f);

	pblock->SetValue(PB_SPINTIME,0,10000*FToTick);
	pblock->SetValue(PB_SPINTIMEVAR,0,0.0f);
	pblock->SetValue(PB_SPINPHASE,0,0.0f);
	pblock->SetValue(PB_SPINPHASEVAR,0,0.0f);
	pblock->SetValue(PB_SPINAXISTYPE,0,0);
	pblock->SetValue(PB_SPINAXISX,0,1.0f);
	pblock->SetValue(PB_SPINAXISY,0,0.0f);
	pblock->SetValue(PB_SPINAXISZ,0,0.0f);
	pblock->SetValue(PB_SPINAXISVAR,0,0.0f);
	pblock->SetValue(PB_DIRX,0,1.0f);
	pblock->SetValue(PB_DIRY,0,0.0f);
	pblock->SetValue(PB_DIRZ,0,0.0f);

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
	pblock->SetValue(PB_SPAWNGENS,0,1);
	pblock->SetValue(PB_SPAWNPERCENT,0,100);
	pblock->SetValue(PB_SPAWNCOUNT,0,1);
	pblock->SetValue(PB_CREATEIN,0,0);
	createmeth=0;

	pblock->SetValue(PB_PCNOTDRAFT,0,0);
	pblock->SetValue(PB_SPAWNDIEAFTER,0,0);
	pblock->SetValue(PB_SPAWNDIEAFTERVAR,0,0.0f);
	pblock->SetValue(PB_PCIPCOLLIDE_ON,0,0);
	pblock->SetValue(PB_PCIPCOLLIDE_STEPS,0,2);
	pblock->SetValue(PB_PCIPCOLLIDE_BOUNCE,0,1.0f);
	pblock->SetValue(PB_PCIPCOLLIDE_BOUNCEVAR,0,0.0f);
	
	sdata=NULL;
	cnode=NULL;
	custnode=NULL;
	custname=TSTR(_T(" "));
	distnode=NULL;
	mbase=NULL;
	normname=TSTR(_T(" "));
	nlist.ZeroCount();
	llist.ZeroCount();
	distname=TSTR(_T(" "));
	ResetSystem(0,FALSE);
	size=42*isize+fsize*39;
	times.tl.SetCount(0);
	cmesh=NULL;
	cmbb=NULL;
	dispmesh=NULL;
	dispt=-99999;
	thePCloudDraw.bboxpt=NULL;
	nmtls.ZeroCount();
	parts.points.ZeroCount();
	cancelled=FALSE;
	wasmulti=FALSE;
	maincount=0;
	fragflags=-1;
	dflags=APRTS_ROLLUP_FLAGS;
	backpatch=TRUE;
	origmtl=NULL;
	ClearAFlag(A_NOTREND);
    stepSize=GetTicksPerFrame();
}

PCloudParticle::~PCloudParticle()
{
	if (sdata) {delete[] sdata;sdata=NULL;}
	DeleteAllRefsFromMe();
	pblock=NULL;
	parts.FreeAll();
	times.tl.Resize(0);
	nmtls.Resize(0);
	llist.ZeroCount();llist.Shrink();
	nlist.ZeroCount();nlist.Shrink();
	if (cmesh) delete[] cmesh;if (dispmesh) delete dispmesh;
	if (cmbb) delete[] cmbb;
	if (thePCloudDraw.bboxpt) delete[] thePCloudDraw.bboxpt;
}

void PCloudParticle::SetUpList()
{ SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<nlist.Count(); i++) 
		SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)(nlist[i]->GetName()));
}

void PCloudParticle::AddToList(INode *newnode,int i,BOOL add)
{	if (add)
	{ nlist.Insert(i,1,&newnode);
	  MakeRefByID(FOREVER,BASER+i,newnode);
	}	  
	else ReplaceReference(i+BASER,newnode);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	SetUpList();
}

void PCloudParticle::DeleteFromList(int nnum,BOOL all)
{ int nCount=nlist.Count();
  INode *cnode=nlist[nnum];
  DeleteReference(nnum+BASER);
  if (theHold.Holding() && !TestAFlag(A_HELD)) 
	  theHold.Put(new PCObjectListRestore(this));
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
void PCloudParticle::SetUpLifeList()
{ TCHAR buffer[20];
  SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<llist.Count(); i++) 
  {	_itoa(llist[i], buffer, 10 );
	SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_ADDSTRING,0,(LPARAM)(TCHAR*)buffer);
  }
} 

void PCloudParticle::AddToLifeList(int newlife)
{	llist.Insert(llist.Count(),1,&newlife);
	SetUpLifeList();
}

void PCloudParticle::DeleteFromLifeList(int nnum)
{ 	llist.Delete(nnum,1);
    if (ip) SetUpLifeList();
}
int PCloudParticle::HitTest(
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
		gw->setTransform(inode->GetObjTMBeforeWSM(t));
		res = dispmesh->select(gw, &particleMtl, &hitRegion, flags & HIT_ABORTONHIT);
		if (res) return TRUE;
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

int PCloudParticle::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{   if (!OKtoDisplay(t)) return 0;
	if (t!=tvalid) cancelled=FALSE;
	if ((ip && origmtl) &&(origmtl!=inode->GetMtl()))
	{ EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
	  origmtl=NULL;
	}
	GraphicsWindow *gw = vpt->getGW();
	DWORD rlim  = gw->getRndLimits();
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (!doupdate) doupdate=CheckMtlChange(inode->GetMtl(),wasmulti);
	if (doupdate) Update(t,inode);

	// Draw emitter
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER) );	// removed BC on 4/29/99
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_PARTICLE_EM));

	if (EmitterVisible()) {
		gw->setTransform(inode->GetObjTMBeforeWSM(t));	
		mesh.render(gw, &particleMtl, 
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);
		}
		
	  Material *mtl = gw->getMaterial();	
	  if (!inode->Selected() && !inode->IsFrozen())
		gw->setColor( LINE_COLOR, mtl->Kd[0], mtl->Kd[1], mtl->Kd[2]);
	int ptype,disptype,type=GetDrawType(this,ptype,disptype);
	if (type==3)
	{ thePCloudDraw.obj = this;
		thePCloudDraw.firstpart=TRUE;
		thePCloudDraw.disptype=disptype;
		thePCloudDraw.ptype=ptype;
		thePCloudDraw.bb=TRUE;
		parts.SetCustomDraw(&thePCloudDraw);			
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
	{ parts.SetCustomDraw(&thePCloudDraw);			
	  NullView nullView;
	  BOOL needdel;
      gw->setRndLimits(rlim);
	  if ((t!=dispt)||doupdate||!dispmesh)
		{
		if (dispmesh) delete dispmesh;
		SetAFlag(A_NOTREND);
		dispmesh=GetRenderMesh(t,inode,nullView,needdel);	ClearAFlag(A_NOTREND);
		}
//	  Matrix3 mat = inode->GetObjTMAfterWSM(t);
	  Matrix3 mat = inode->GetObjectTM(t); //change
	  gw->setTransform(mat);
	  dispmesh->render(gw, inode->Mtls(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL,
		COMP_ALL, inode->NumMtls());
	}
	return(0);
}

#endif NO_PARTICLES_PCLOUD
