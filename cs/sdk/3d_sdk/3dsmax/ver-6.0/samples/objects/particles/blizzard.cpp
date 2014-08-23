/**********************************************************************
 *<									  

	FILE: Blizzard.CPP
	DESCRIPTION: SuperSpray and Blizzard main code

	CREATED BY: Audrey Peterson	

	HISTORY: created November 1996
	Instancing Geometry 1/97
	IPC 8/98

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include <io.h>
#include "SuprPrts.h"
#include "iparamm.h"
#include "interpik.h"
#include "texutil.h"
#include "stdmat.h"
#include "macrorec.h"

// russom - 10/11/01
#if !defined(NO_PARTICLES_BLIZZARD) && !defined(NO_PARTICLES_SUPERSPRAY)

#define EPSILON				0.0001f
#define MAX_PATH_LENGTH		257
#define MAX_STRING_LENGTH	256
#define PARTICLE_SEED		0x8d6a65bc

#define PBLK			0
#define CUSTNODE 		1

#define BASER 2

typedef struct 
{
	float Vsz,Ts,Ts0,LamTs,A,LamA,To;
	float M,Dis,Fo,Mltvar,pvar;
	int themtl,gennum,SpVar;
	TimeValue L,showframe,DL,persist;
	Point3	V,W,RV;
}	CSavePt;

typedef struct 
{
	float Size,VSz,VSpin,Phase,VPhase,Speed,VSpeed;
	float width,bstr,bstrvar,ToAmp,VToAmp,axisvar;
	float ToPhase,VToPhase,VToPeriod,Av180,Ah180,VAv,VAh,pvar;
	int axisentered;
	Point3 Ve,Axis;
	TimeValue Spin,ToPeriod,Life,Vl,persist;
	BirthPositionSpeed bps;
}	VelDir;

static Class_ID SUPRSPRAY_CLASS_ID(0x74f811e3, 0x21fb7b57);
static Class_ID BLIZZARD_CLASS_ID(0x5835054d, 0x564b40ed);

class CPickOperand;
class CommonParticle;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//        WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class CommonParticleDraw : public CustomParticleDisplay {
	public:
		float tumble,scale;
		BOOL firstpart;
		CommonParticle *obj;
		int disptype,ptype,bb,anifr,anioff;
		boxlst *bboxpt;
		TimeValue t;
		InDirInfo indir;

		CommonParticleDraw() {obj=NULL;}
		BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i);
	};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING - a copy of this class description is in maxscrpt\maxnode.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class CommonParticle : public SimpleParticle, IParamArray {
	public:
		CommonParticleDraw theSuprSprayDraw;
		static IParamMap *pmapParam;
		static IParamMap *pmapPGen;
		static IParamMap *pmapPType;
		static IParamMap *pmapPSpin;
		static IParamMap *pmapEmitV;
		static IParamMap *pmapSpawn;
		int stepSize,size;
		static custsettings;
		ULONG dflags;
		Mesh *cmesh,*dispmesh;
		INode *custnode,*cnode;
		TSTR custname;
		DWORD flags;
		BOOL cancelled,wasmulti;
		static BOOL creating;
		static CPickOperand pickCB;
		Mtl *origmtl;

		Point3 boxcenter;
		int CustMtls;
		TimeLst times;
		void GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype);
		void TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl);
		void GetMesh(TimeValue t,int subtree,int custmtl);
		void GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode);
		void GetallBB(INode *custnode,int subtree,TimeValue t);

		Tab<int> nmtls;
		void DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emit);
		void SetUpList();
		void AddToList(INode *newnode,int i,BOOL add);
		void DeleteFromList(int nnum,BOOL all);
		Tab<INode*> nlist;
		Tab<int> llist;
		int deftime;
		int maincount,rseed;
		int NumRefs() {return BASER+nlist.Count();}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
		void SetUpLifeList();
		void AddToLifeList(int newlife);
		void DeleteFromLifeList(int nnum);
		void ShowName();
		static AName *NameLst;
		static HWND hParams2,hptype,hgen,hparam,hrot,spawn;
		static ICustEdit *custCtrlEdit;
		void ResetSystem(TimeValue t,BOOL full=TRUE);

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
		CSavePt *sdata;

		// From BaseObject
		int IsInstanceDependent() {return 1;}
		Matrix3 TumbleMat(int index,float amount, float scale);
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
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt);
		TimeValue dispt;
	};

class SuprSprayParticle : public CommonParticle {
	public:
		SuprSprayParticle();
		~SuprSprayParticle();
		static IParamMap *pmapBubl;
		static HWND hbubl;
		CreateMouseCallBack* GetCreateMouseCallBack();

		TCHAR *GetObjectName();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void BuildEmitter(TimeValue t, Mesh& amesh);
		void BirthParticle(INode *node,TimeValue bt,int num,VelDir* ptvel,Matrix3 tmlast);
		BOOL ComputeParticleStart(TimeValue t0,int c);
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		// Animatable methods		
		Class_ID ClassID() {return SUPRSPRAY_CLASS_ID;} 
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From Simple Particle
		void UpdateParticles(TimeValue t,INode *node);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI();
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);
		Point3 ParticlePosition(TimeValue t,int i);
		Point3 ParticleVelocity(TimeValue t,int i);		
		void MovePart(int j,TimeValue dt,BOOL fullframe,int tpf);
	};
//--- ClassDescriptor and class vars ---------------------------------

class SuprSprayClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new SuprSprayParticle;}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return SUPRSPRAY_CLASS_ID;}
	const TCHAR* 	Category(); 
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static SuprSprayClassDesc SuprSprayDesc;
ClassDesc* GetSuprSprayDesc() {return &SuprSprayDesc;}

class CPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		CommonParticle *po;
		int dodist,repi;

		CPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};
//--- ClassDescriptor and class vars ---------------------------------
IParamMap *CommonParticle::pmapParam;
IParamMap *CommonParticle::pmapPGen;
IParamMap *CommonParticle::pmapPType;
IParamMap *CommonParticle::pmapPSpin;
IParamMap *CommonParticle::pmapEmitV;
IParamMap *CommonParticle::pmapSpawn;
IObjParam *CommonParticle::ip    = NULL;
BOOL CommonParticle::creating    = FALSE;
CPickOperand CommonParticle::pickCB;
ICustEdit *CommonParticle::custCtrlEdit=NULL;
int CommonParticle::custsettings=0;
AName *CommonParticle::NameLst=NULL;
HWND CommonParticle::hParams2;
HWND CommonParticle::hgen;
HWND CommonParticle::hptype;
HWND CommonParticle::hparam;
HWND CommonParticle::hrot;
HWND CommonParticle::spawn;

IParamMap *SuprSprayParticle::pmapBubl;
HWND SuprSprayParticle::hbubl;

class BlizzardParticle : public CommonParticle {
	public:
		BlizzardParticle();
		~BlizzardParticle();
		CreateMouseCallBack* GetCreateMouseCallBack();

		TCHAR *GetObjectName();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void BuildEmitter(TimeValue t, Mesh& amesh);
		void BirthParticle(INode *node,TimeValue bt,int num,VelDir* ptvel,Matrix3 tmlast);
		BOOL ComputeParticleStart(TimeValue t0,int c);
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		// Animatable methods		
		Class_ID ClassID() {return BLIZZARD_CLASS_ID;} 
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From Simple Particle
		void UpdateParticles(TimeValue t,INode *node);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI();
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);
		Point3 ParticlePosition(TimeValue t,int i);
		Point3 ParticleVelocity(TimeValue t,int i);		
	};
//--- ClassDescriptor and class vars ---------------------------------

class BlizzardClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new BlizzardParticle();}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return BLIZZARD_CLASS_ID;}
	const TCHAR* 	Category(); 
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static BlizzardClassDesc BlizzardDesc;
ClassDesc* GetBlizzardDesc() {return &BlizzardDesc;}

#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)

//--- Parameter map/block descriptors -------------------------------

#define PB_OFFAXIS				0
#define PB_AXISSPREAD			1
#define PB_OFFPLANE				2
#define PB_PLANESPREAD			3
#define PB_SPEED				4
#define PB_SPEEDVAR				5

#define PB_BIRTHMETHOD			6
#define PB_PBIRTHRATE			7
#define PB_PTOTALNUMBER			8
#define PB_EMITSTART			9
#define PB_EMITSTOP				10
#define PB_DISPUNTIL			11
#define PB_LIFE					12
#define PB_LIFEVAR				13
#define PB_SUBFRAMEMOVE			14
#define PB_SUBFRAMETIME			15
#define PB_SIZE					16
#define PB_SIZEVAR				17
#define PB_GROWTIME				18
#define PB_FADETIME				19
#define PB_RNDSEED				20
#define PB_EMITRWID				21
#define PB_EMITRHID				22

#define PB_PARTICLECLASS		23
#define PB_PARTICLETYPE			24
#define PB_METATENSION			25
#define PB_METATENSIONVAR		26
#define PB_METACOURSE			27
#define PB_METAAUTOCOARSE		28
#define PB_USESUBTREE			29
#define PB_ANIMATIONOFFSET		30
#define PB_OFFSETAMOUNT			31
#define PB_VIEWPORTSHOWS		32
#define PB_DISPLAYPORTION		33
#define PB_MAPPINGTYPE			34
#define PB_MAPPINGTIME			35
#define PB_MAPPINGDIST			36
						
#define PB_SPINTIME				37
#define PB_SPINTIMEVAR			38
#define PB_SPINPHASE			39
#define PB_SPINPHASEVAR			40
#define PB_SPINAXISTYPE			41
#define PB_SPINAXISX			42
#define PB_SPINAXISY			43
#define PB_SPINAXISZ			44
#define PB_SPINAXISVAR			45

#define PB_EMITVINFL			46
#define PB_EMITVMULT			47
#define PB_EMITVMULTVAR			48

#define PB_SPAWNTYPE			49
#define PB_SPAWNGENS			50
#define PB_SPAWNCOUNT			51
#define PB_SPAWNDIRCHAOS		52
#define PB_SPAWNSPEEDCHAOS		53
#define PB_SPAWNSPEEDSIGN		54
#define PB_SPAWNINHERITV		55
#define PB_SPAWNSCALECHAOS		56
#define PB_SPAWNSCALESIGN		57
#define PB_SPAWNLIFEVLUE		58
#define PB_SPAWNSPEEDFIXED		59
#define PB_SPAWNSCALEFIXED		60	

#define PB_BUBLAMP				61
#define PB_BUBLAMPVAR			62
#define PB_BUBLPER				63
#define PB_BUBLPERVAR			64
#define PB_BUBLPHAS				65
#define PB_BUBLPHASVAR			66

#define PB_STRETCH				67
#define PB_CUSTOMMTL			68
#define PB_METACOURSEV			69
#define PB_SUBFRAMEROT			70
#define PB_SPAWNPERCENT			71
#define PB_SPAWNMULTVAR			72
#define PB_SSNOTDRAFT			73
#define PB_SSSPAWNDIEAFTER		74
#define PB_SSSPAWNDIEAFTERVAR	75

#define PB_IPCOLLIDE_ON			76
#define PB_IPCOLLIDE_STEPS		77
#define PB_IPCOLLIDE_BOUNCE		78
#define PB_IPCOLLIDE_BOUNCEVAR	79

// render types
#define RENDMETA    8
#define RENDGEOM	9

// Parameters
#define ISSTD 0
#define METABALLS 1
#define INSTGEOM 2

static int countIDs[] = {IDC_SP_GENUSERATE,IDC_SP_GENUSETTL};

static int particleclassIDs[] = {IDC_SP_TYPESTD,IDC_SP_TYPEMET,IDC_SP_TYPEINSTANCE};

static int particletypeIDs[] = {IDC_SP_TYPETRI,IDC_SP_TYPECUB,IDC_SP_TYPESPC,IDC_SP_TYPEFAC,
								IDC_SP_TYPEPIX,IDC_SP_TYPETET,IDC_SP_TYPE6PNT,IDC_SP_TYPESPHERE};

static int viewportoptionIDs[] = {IDC_SP_VIEWDISPDOT,IDC_SP_VIEWDISPTIK,IDC_SP_VIEWDISPMESH,IDC_SP_VIEWDISPBOX};

static int mappingIDs[] = {IDC_SP_MAPTIME,IDC_SP_MAPDIST};

static int spindirectionIDs[] = {IDC_AP_PARTICLEDIRRND,IDC_AP_PARTICLEDIRTRAVL,IDC_AP_PARTICLEDIRUSER};
static int bspindirectionIDs[] = {IDC_AP_PARTICLEDIRRND,IDC_AP_PARTICLEDIRUSER};

#define DIRTRAVEL 1

static int animateoffsetIDs[] = {IDC_AP_NOANIOFF,IDC_AP_ANIOFFBIRTH,IDC_AP_ANIOFFRND};
static int custmtlIDs[] = {IDC_SP_MAPCUSTOMEMIT,IDC_SP_MAPCUSTOMINST};

// Dialog Unique to SuperSpray
static ParamUIDesc descParamSuprSpray[] = {

	// Particle Off Axis 
	ParamUIDesc(
		PB_OFFAXIS,
		EDITTYPE_FLOAT,
		IDC_SP_OFFAXIS,IDC_SP_OFFAXISSPIN,
		-180.0f,180.0f,
		1.0f,
		stdAngleDim),

	// Particle Off Axis Spread
	ParamUIDesc(
		PB_AXISSPREAD,
		EDITTYPE_FLOAT,
		IDC_SP_OFFAXISVAR,IDC_SP_OFFAXISVARSPIN,
		0.0f,180.0f,
		1.0f,
		stdAngleDim),

	// Particle Off Plane 
	ParamUIDesc(
		PB_OFFPLANE,
		EDITTYPE_FLOAT,
		IDC_SP_OFFPLANE,IDC_SP_OFFPLANESPIN,
		-180.0f,180.0f,
		1.0f,
		stdAngleDim),

	// Particle Off Plane Spread
	ParamUIDesc(
		PB_PLANESPREAD,
		EDITTYPE_FLOAT,
		IDC_SP_OFFPLANEVAR,IDC_SP_OFFPLANEVARSPIN,
		0.0f,180.0f,
		1.0f,
		stdAngleDim),

	// Emitter Width
	ParamUIDesc(
		PB_EMITRWID,
		EDITTYPE_UNIVERSE,
		IDC_SP_EMITWID,IDC_SP_EMITWIDSPIN,
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

#define PARAMSuprSpray_LENGTH 8

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
		IDC_SP_SPEEDVAR2,IDC_SP_SPEEDVARSPIN2,
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

#define PARAMPGEN_LENGTH 18

// Particle Type for SuprSpray
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
	ParamUIDesc(PB_SSNOTDRAFT,TYPE_SINGLECHEKBOX,IDC_SP_DRAFTMODE),

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

// SS Dialog for Particle Spin
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
	ParamUIDesc(PB_IPCOLLIDE_ON,TYPE_SINGLECHEKBOX,IDC_INTERP_BOUNCEON),

	// IPC Steps
	ParamUIDesc(
		PB_IPCOLLIDE_STEPS,
		EDITTYPE_INT,
		IDC_INTERP_NSTEPS,IDC_INTERP_NSTEPSSPIN,
		1.0f,1000.0f,
		1.0f),

	// IPC Bounce
	ParamUIDesc(
		PB_IPCOLLIDE_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCE,IDC_INTERP_BOUNCESPIN,
		0.0f,10000.0f,
		1.0f,
		stdPercentDim),

	// IPC Bounce
	ParamUIDesc(
		PB_IPCOLLIDE_BOUNCEVAR,
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
		PB_SSSPAWNDIEAFTER,
		EDITTYPE_TIME,
		IDC_AP_MAXSPAWNDIEAFTER,IDC_AP_MAXSPAWNDIEAFTERSPIN,
		0.0f,999999999.0f,
		10.0f),

	ParamUIDesc(
		PB_SSSPAWNDIEAFTERVAR,
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

	// Spawn Generations
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
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
};

static ParamBlockDescID spdescVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
};

static ParamBlockDescID spdescVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
	{ TYPE_INT, NULL, FALSE, 70 },  // Subframe rot
};

static ParamBlockDescID spdescVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
	{ TYPE_INT, NULL, FALSE, 70 },  // Subframe rot
	{ TYPE_INT, NULL, FALSE, 71 },  // Spawn Percent
	{ TYPE_FLOAT, NULL, FALSE, 72 },  // Spawn Mult var
};

static ParamBlockDescID spdescVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
	{ TYPE_INT, NULL, FALSE, 70 },  // Subframe rot
	{ TYPE_INT, NULL, FALSE, 71 },  // Spawn Percent
	{ TYPE_FLOAT, NULL, FALSE, 72 },  // Spawn Mult var
	{ TYPE_INT, NULL, FALSE, 73 },  // Surface Tracking
};

static ParamBlockDescID spdescVer5[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
	{ TYPE_INT, NULL, FALSE, 70 },  // Subframe rot
	{ TYPE_INT, NULL, FALSE, 71 },  // Spawn Percent
	{ TYPE_FLOAT, NULL, FALSE, 72 },  // Spawn Mult var
	{ TYPE_INT, NULL, FALSE, 73 },  // Surface Tracking
	{ TYPE_INT, NULL, TRUE, 74 },  // dies after X
	{ TYPE_FLOAT, NULL, TRUE, 75 },  // dies after X var
};

static ParamBlockDescID spdescVer6[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // off axis
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // axis apread
	{ TYPE_FLOAT, NULL, TRUE, 2 },	 // off plane
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // plane spread
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 }, // rnd seed
	{ TYPE_FLOAT, NULL, FALSE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
    { TYPE_INT, NULL, FALSE, 29 }, // use subtree
    { TYPE_INT, NULL, FALSE, 30 }, // animation offset
    { TYPE_INT, NULL, FALSE, 31 }, // offset amount

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choice
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_FLOAT, NULL, TRUE, 61 },  // bubble amp61
	{ TYPE_FLOAT, NULL, TRUE, 62 },  // bubble amp var
	{ TYPE_INT, NULL, TRUE, 63 },  // bubble period
	{ TYPE_FLOAT, NULL, TRUE, 64 },  // bubble period var
	{ TYPE_FLOAT, NULL, TRUE, 65 },  // bubble phase
	{ TYPE_FLOAT, NULL, TRUE, 66 },  // bubble phase var

	{ TYPE_INT, NULL, TRUE, 67 },  // Stretch
	{ TYPE_INT, NULL, FALSE, 68 },  // Custom Mtl
	{ TYPE_FLOAT, NULL, FALSE, 69 },  // ViewPort Courseness
	{ TYPE_INT, NULL, FALSE, 70 },  // Subframe rot
	{ TYPE_INT, NULL, FALSE, 71 },  // Spawn Percent
	{ TYPE_FLOAT, NULL, FALSE, 72 },  // Spawn Mult var
	{ TYPE_INT, NULL, FALSE, 73 },  // Surface Tracking
	{ TYPE_INT, NULL, TRUE, 74 },  // dies after X
	{ TYPE_FLOAT, NULL, TRUE, 75 },  // dies after X var

	{ TYPE_INT, NULL, FALSE, 76 },  // IPC Enable
	{ TYPE_INT, NULL, FALSE, 77 },  // IPC Steps
	{ TYPE_FLOAT, NULL, TRUE, 78 },  // IPC Bounce
	{ TYPE_FLOAT, NULL, TRUE, 79 },  // IPC Bounce Var
};

#define PBLOCK_LENGTH_SUPRSPRAY 80

static ParamVersionDesc spversions[] = {
	ParamVersionDesc(spdescVer0,69,0),
	ParamVersionDesc(spdescVer1,70,1),
	ParamVersionDesc(spdescVer2,71,2),
	ParamVersionDesc(spdescVer3,73,3),
	ParamVersionDesc(spdescVer4,74,4),
	ParamVersionDesc(spdescVer5,76,5),
	};
#define NUM_OLDVERSIONS	6

// Current version
#define CURRENT_VERSION	6

#define PB_TUMBLE					0
#define PB_TUMBLERATE				1
#define PB_EMITMAP					2
#define PB_EMITRLENGTH				3
#define PB_CUSTOMMTL2				61
#define PB_METACOURSEVB				62
#define PB_SUBFRAMEROT2				63
#define PB_SPAWNPERCENT2			64
#define PB_SPAWNMULTVAR2			65
#define PB_BLNOTDRAFT				66
#define PB_BLSPAWNDIEAFTER			67
#define PB_BLSPAWNDIEAFTERVAR		68
#define PB_BLIPCOLLIDE_ON			69
#define PB_BLIPCOLLIDE_STEPS		70
#define PB_BLIPCOLLIDE_BOUNCE		71
#define PB_BLIPCOLLIDE_BOUNCEVAR	72

static int bmappingIDs[] = {IDC_SP_MAPTIME,IDC_SP_MAPDIST,IDC_SP_MAPPLANAR};

// Spin Dialog for Blizzard
static ParamUIDesc descBParamPSpin[] = {

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
	ParamUIDesc(PB_SPINAXISTYPE,TYPE_RADIO,bspindirectionIDs,2),

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

	// IPC Enable
	ParamUIDesc(PB_BLIPCOLLIDE_ON,TYPE_SINGLECHEKBOX,IDC_INTERP_BOUNCEON),

	// IPC Steps
	ParamUIDesc(
		PB_BLIPCOLLIDE_STEPS,
		EDITTYPE_INT,
		IDC_INTERP_NSTEPS,IDC_INTERP_NSTEPSSPIN,
		1.0f,1000.0f,
		1.0f),

	// IPC Bounce
	ParamUIDesc(
		PB_BLIPCOLLIDE_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCE,IDC_INTERP_BOUNCESPIN,
		0.0f,10000.0f,
		1.0f,
		stdPercentDim),

	// IPC Bounce
	ParamUIDesc(
		PB_BLIPCOLLIDE_BOUNCEVAR,
		EDITTYPE_FLOAT,
		IDC_INTERP_BOUNCEVAR,IDC_INTERP_BOUNCEVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),
};

#define BPARAMPSPIN_LENGTH 13

static ParamUIDesc BdescParamPType[] = {

	// Particle Class
	ParamUIDesc(PB_PARTICLECLASS,TYPE_RADIO,particleclassIDs,3),

	// Particle Type
	ParamUIDesc(PB_PARTICLETYPE,TYPE_RADIO,particletypeIDs,8),

	// Metaball Tension
	ParamUIDesc(
		PB_METATENSION,
		EDITTYPE_UNIVERSE,
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
		PB_METACOURSEVB,
		EDITTYPE_UNIVERSE,
		IDC_SP_METCOURSEV,IDC_SP_METCOURSEVSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Metaball Auto Coarseness
	ParamUIDesc(PB_METAAUTOCOARSE,TYPE_SINGLECHEKBOX,IDC_SP_AUTOCOARSE),
     
	// Display type
	ParamUIDesc(PB_BLNOTDRAFT,TYPE_SINGLECHEKBOX,IDC_SP_DRAFTMODE),

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
	ParamUIDesc(PB_MAPPINGTYPE,TYPE_RADIO,bmappingIDs,3),

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
	ParamUIDesc(PB_CUSTOMMTL2,TYPE_RADIO,custmtlIDs,2),
};
#define BPARAMPTYPE_LENGTH 15

// Dialog Unique to Particle Array
static ParamUIDesc BdescParam[] = {

	// Emitter Width
	ParamUIDesc(
		PB_EMITRWID,
		EDITTYPE_UNIVERSE,
		IDC_SP_EMITWID,IDC_SP_EMITWIDSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Emitter Length
	ParamUIDesc(
		PB_EMITRLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_SP_EMITLEN,IDC_SP_EMITLENSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Hide Emitter
	ParamUIDesc(PB_EMITRHID,TYPE_SINGLECHEKBOX,IDC_SP_EMITHID),

	// Animation Offset Type
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
#define PARAMBLIZZARD_LENGTH 5

// Common Dialog for Particle Generation
static ParamUIDesc BdescParamPGen[] = {

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
		IDC_SP_BLIZSPEED,IDC_SP_BLIZSPEEDSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Particle Speed Var
	ParamUIDesc(
		PB_SPEEDVAR,
		EDITTYPE_FLOAT,
		IDC_SP_BLIZSPEEDVAR,IDC_SP_BLIZSPEEDVARSPIN,
		0.0f,100.0f,
		SPIN_AUTOSCALE,stdPercentDim),

	// Particle Tumble 
	ParamUIDesc(
		PB_TUMBLE,
		EDITTYPE_FLOAT,
		IDC_SP_BLIZTUMBL,IDC_SP_BLIZTUMBLSPIN,
		0.0f,1.0f,
		0.005f),

	// Particle Tumble Rate
	ParamUIDesc(
		PB_TUMBLERATE,
		EDITTYPE_FLOAT,
		IDC_SP_BLIZTUMBLRATE,IDC_SP_BLIZTUMBLRATESPIN,
		0.0f,999999999.0f,
		0.01f),

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
	ParamUIDesc(PB_SUBFRAMEROT2,TYPE_SINGLECHEKBOX,IDC_SP_GENSMPLROT),

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

#define BPARAMPGEN_LENGTH 20

static ParamUIDesc descBPSpawning[] = {
	
	// Spawing Effects Type
	ParamUIDesc(PB_SPAWNTYPE,TYPE_RADIO,SpawnTypeIDs,5),

	// Die after X
	ParamUIDesc(
		PB_BLSPAWNDIEAFTER,
		EDITTYPE_TIME,
		IDC_AP_MAXSPAWNDIEAFTER,IDC_AP_MAXSPAWNDIEAFTERSPIN,
		0.0f,999999999.0f,
		10.0f),

	ParamUIDesc(
		PB_BLSPAWNDIEAFTERVAR,
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

	// Spawn Generations
	ParamUIDesc(
		PB_SPAWNPERCENT2,
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
		PB_SPAWNMULTVAR2,
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
		0.1f),
};

#define PBSPAWNINGPARAMS_LENGTH 16

// Particle Type for Blizzard

static ParamBlockDescID BdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
};

static ParamBlockDescID BdescVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
};
static ParamBlockDescID BdescVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
	{ TYPE_INT, NULL, FALSE, 63},  // subframe rot
};
static ParamBlockDescID BdescVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
	{ TYPE_INT, NULL, FALSE, 63},  // subframe rot
	{ TYPE_INT, NULL, FALSE, 64 }, // Spawn PErcent
	{ TYPE_FLOAT, NULL, FALSE, 65},  // spawn mult var
};
static ParamBlockDescID BdescVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
	{ TYPE_INT, NULL, FALSE, 63},  // subframe rot
	{ TYPE_INT, NULL, FALSE, 64 }, // Spawn PErcent
	{ TYPE_FLOAT, NULL, FALSE, 65},  // spawn mult var
	{ TYPE_INT, NULL, TRUE, 66 }, // Draft
};

static ParamBlockDescID BdescVer5[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
	{ TYPE_INT, NULL, FALSE, 63},  // subframe rot
	{ TYPE_INT, NULL, FALSE, 64 }, // Spawn PErcent
	{ TYPE_FLOAT, NULL, FALSE, 65},  // spawn mult var
	{ TYPE_INT, NULL, TRUE, 66 }, // Draft
	{ TYPE_INT, NULL, TRUE, 67 }, // Dies after X
	{ TYPE_FLOAT, NULL, TRUE, 68 }, // Dies after X var
};

static ParamBlockDescID BdescVer6[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	 // tumble
	{ TYPE_FLOAT, NULL, TRUE, 1 },	 // tumblerate
	{ TYPE_FLOAT, NULL, FALSE, 2 },	 // emit map
	{ TYPE_FLOAT, NULL, TRUE, 3 },	 // emitter length
	{ TYPE_FLOAT, NULL, TRUE, 4 },	 // speed
	{ TYPE_FLOAT, NULL, TRUE, 5 },	 // speed var

	{ TYPE_INT, NULL, FALSE, 6 },	 // Birth method
	{ TYPE_INT, NULL, TRUE, 7 },	 // Birth rate
	{ TYPE_INT, NULL, FALSE, 8 },    // total number
	{ TYPE_INT, NULL, FALSE, 9 },	 // emit start
	{ TYPE_INT, NULL, FALSE, 10 },   // emit stop
	{ TYPE_INT, NULL, FALSE, 11 },   // display until
	{ TYPE_INT, NULL, TRUE, 12 },	 // life
	{ TYPE_INT, NULL, TRUE, 13 },	 // life var
	{ TYPE_INT, NULL, FALSE, 14 },	 // sub frame move
	{ TYPE_INT, NULL, FALSE, 15 },	 // sub frame time
	{ TYPE_FLOAT, NULL, TRUE, 16 },	 // size
	{ TYPE_FLOAT, NULL, TRUE, 17 },	 // size var
	{ TYPE_INT, NULL, FALSE, 18 },	 // grow time
	{ TYPE_INT, NULL, FALSE, 19 },    // fade time
	{ TYPE_INT, NULL, FALSE, 20 },    // rnd seed
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // emitter width
	{ TYPE_INT, NULL, FALSE, 22 },    // emitter hidden

	{ TYPE_INT, NULL, FALSE, 23 },  // particle class
	{ TYPE_INT, NULL, FALSE, 24 },  // particle type
	{ TYPE_FLOAT, NULL, TRUE, 25 },	// meta tension
	{ TYPE_FLOAT, NULL, TRUE, 26 },	// metatension var
	{ TYPE_FLOAT, NULL, FALSE, 27 }, // meta course
	{ TYPE_INT, NULL, FALSE, 28 }, // auto coarseness
	{ TYPE_INT, NULL, FALSE, 29 }, // use subtree
	{ TYPE_INT, NULL, FALSE, 30 }, // animation offset 
	{ TYPE_INT, NULL, FALSE, 31 }, // animation offset time

	{ TYPE_INT, NULL, FALSE, 32 },	  // viewport shows
	{ TYPE_FLOAT, NULL, FALSE, 33 }, // display portion
	{ TYPE_INT, NULL, FALSE, 34 },	  // mapping type
	{ TYPE_INT, NULL, TRUE, 35 },	  // mapping time
	{ TYPE_FLOAT, NULL, TRUE, 36 },	  // mapping dist

	{ TYPE_INT, NULL, TRUE, 37 },	// spin time
	{ TYPE_FLOAT, NULL, TRUE, 38 },  // spin time var
	{ TYPE_FLOAT, NULL, TRUE, 39 },  // spin phase
	{ TYPE_FLOAT, NULL, TRUE, 40 },  // spin phase var
	{ TYPE_INT, NULL, FALSE, 41 },  // spin axis choose
	{ TYPE_FLOAT, NULL, TRUE, 42 }, // spin axis x
	{ TYPE_FLOAT, NULL, TRUE, 43 }, // spin axis y
	{ TYPE_FLOAT, NULL, TRUE, 44 }, // spin axis z
	{ TYPE_FLOAT, NULL, TRUE, 45 }, // spin axis var

	{ TYPE_FLOAT, NULL, TRUE, 46 },  // emit influence
	{ TYPE_FLOAT, NULL, TRUE, 47 },  // emit multiplier
	{ TYPE_FLOAT, NULL, TRUE, 48 },  // emit mult var

	{ TYPE_INT, NULL, FALSE, 49 }, // spawn type
	{ TYPE_INT, NULL, FALSE, 50 }, // number of gens
	{ TYPE_INT, NULL, FALSE, 51 }, // number of spawns
	{ TYPE_FLOAT, NULL, TRUE, 52 }, // direction chaos
	{ TYPE_FLOAT, NULL, TRUE, 53 }, // speed chaos
	{ TYPE_INT, NULL, FALSE, 54 }, // speed chaos sign
	{ TYPE_INT, NULL, FALSE, 55 }, // inherit old particle velocity
	{ TYPE_FLOAT, NULL, TRUE, 56 }, // scale chaos 
	{ TYPE_INT, NULL, FALSE, 57 }, // scale chaos sign
	{ TYPE_INT, NULL, FALSE, 58 }, // lifespan entry field
	{ TYPE_INT, NULL, FALSE, 59 }, // constant spawn speed
	{ TYPE_INT, NULL, FALSE, 60 }, // constant spawn scale

	{ TYPE_INT, NULL, FALSE, 61 },  // custmlt
	{ TYPE_FLOAT, NULL, FALSE, 62 }, // meta course Viewport
	{ TYPE_INT, NULL, FALSE, 63},  // subframe rot
	{ TYPE_INT, NULL, FALSE, 64 }, // Spawn PErcent
	{ TYPE_FLOAT, NULL, FALSE, 65},  // spawn mult var
	{ TYPE_INT, NULL, TRUE, 66 }, // Draft
	{ TYPE_INT, NULL, TRUE, 67 }, // Dies after X
	{ TYPE_FLOAT, NULL, TRUE, 68 }, // Dies after X var

	{ TYPE_INT, NULL, FALSE, 69 }, // IPC Enable
	{ TYPE_INT, NULL, FALSE, 70 }, // IPC Steps
	{ TYPE_FLOAT, NULL, TRUE, 71 }, // IPC Bounce
	{ TYPE_FLOAT, NULL, TRUE, 72 }, // IPC Bounce Var
};

static ParamVersionDesc Bversions[] = {
	ParamVersionDesc(BdescVer0,62,0),
	ParamVersionDesc(BdescVer1,63,1),
	ParamVersionDesc(BdescVer2,64,2),
	ParamVersionDesc(BdescVer3,66,3),
	ParamVersionDesc(BdescVer4,67,4),
	ParamVersionDesc(BdescVer5,69,5),
	};
#define PBLOCK_LENGTH_BLIZZARD 73

static ParamVersionDesc curVersionBL(BdescVer6,PBLOCK_LENGTH_BLIZZARD,CURRENT_VERSION);
static ParamVersionDesc curVersionSp(spdescVer6,PBLOCK_LENGTH_SUPRSPRAY,CURRENT_VERSION);

//-- ParticleDlgProc ------------------------------------------------

class CreateSSBlizProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		CommonParticle *SSBlizObject;
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
		
		CreateSSBlizProc()
			{
			ignoreSelectionChange = FALSE;
			}
//		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};
class ComObjectListRestore : public RestoreObj {
	public:   		
		CommonParticle *po;
		Tab<INode*> unodes;
		Tab<INode*> rnodes;
		int lnum,lnum2;
		ComObjectListRestore(CommonParticle *p) 
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

class LifeListRestore : public RestoreObj {
	public:   		
		CommonParticle *po;
		Tab<int> utimes;
		Tab<int> rtimes;
		LifeListRestore(CommonParticle *p) 
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

class CreateCPartPickNode : public RestoreObj {
	public:   		
		CommonParticle *obj;
		TSTR name,name2;
		CreateCPartPickNode(CommonParticle *o, TSTR n,TSTR n1) {
			obj = o; name=TSTR(n);name2=TSTR(n1);
			}
		void Restore(int isUndo) {
			if (obj->custnode) 
			 obj->custname = name;
			else 
			  obj->custname=TSTR(_T(""));
			obj->ShowName();
			}
		void Redo() 
		{ obj->custname = name2;
		if (obj->hptype)
	{TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (_tcslen(name2)>0? obj->custname : TSTR(GetString(IDS_AP_NONE)));
	SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);}
			}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};
class CreateCPartDelNode : public RestoreObj {
	public:   		
		CommonParticle *obj;
		TSTR name;
		CreateCPartDelNode(CommonParticle *o, TSTR n) {
			obj = o; name=TSTR(n);
			}
		void Restore(int isUndo)
		{obj->custname = name;
		 obj->ShowName();
			}
		void Redo() 
			{  obj->custname=TSTR(_T(""));
		if (obj->hptype)
	{TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + TSTR(GetString(IDS_AP_NONE));
	SetWindowText(GetDlgItem(obj->hptype, IDC_AP_INSTANCESRCNAME), name);}
			}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};

#define CID_CREATESSBlizMODE	CID_USER +15

class CreateSSBlizMode : public CommandMode {		
	public:		
		CreateSSBlizProc proc;
		IObjParam *ip;
		CommonParticle *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,CommonParticle*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATESSBlizMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateSSBlizMode theCreateSSBlizMode;

RefResult CreateSSBlizProc::NotifyRefChanged(
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
		 	if ( SSBlizObject && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (SSBlizObject && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (SSBlizObject->creating) 
						{  theCreateSSBlizMode.JumpStart(SSBlizObject->ip,SSBlizObject);
							createInterface->SetCommandMode(&theCreateSSBlizMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				SSBlizObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				SSBlizObject  = NULL;				
				CloudNode    = NULL;
/*				if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);*/
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateSSBlizProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	SSBlizObject       = NULL;
	CreateNewObject();
	}
void CreateSSBlizProc::CreateNewObject()
{
	SuspendSetKeyMode();
	createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	SSBlizObject = (CommonParticle*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( SSBlizObject ) {
		SSBlizObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#ifdef _OSNAP
		SSBlizObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreateSSBlizProc::End()
{ if ( SSBlizObject ) 
	{ 
#ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	
	SSBlizObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			createInterface->GetMacroRecorder()->Cancel();
			theHold.Suspend(); 
			delete SSBlizObject;
			SSBlizObject = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();}
	}
}

void CreateSSBlizMode::JumpStart(IObjParam *i,CommonParticle *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int BlizzardClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateSSBlizMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateSSBlizMode);
	return TRUE;
	}

int BlizzardClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateSSBlizMode.End();
	i->RemoveMode(&theCreateSSBlizMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}
int SuprSprayClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateSSBlizMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateSSBlizMode);
	return TRUE;
	}

int SuprSprayClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateSSBlizMode.End();
	i->RemoveMode(&theCreateSSBlizMode);
	return TRUE;
	}

int CreateSSBlizProc::proc(HWND hwnd,int msg,int point,int flag,
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
					assert( SSBlizObject );					
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
						SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		SSBlizObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						// Get rid of the reference.
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}
						// new object
						CreateNewObject();   // creates SSBlizObject
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( SSBlizObject);
					attachedToNode = TRUE;
					assert( CloudNode );					
					createCB = SSBlizObject->GetCreateMouseCallBack();
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
						if (res==CREATE_STOP)
						{
#ifdef _OSNAP
                         SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
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
         SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
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
	  createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
	  if (res==CREATE_ABORT)
	      goto abort;
	  if (res==CREATE_STOP)
	  {
#ifdef _OSNAP
         SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	    theHold.Accept(GetString(IDS_DS_CREATE));	
	  }
		break;
	}
	abort:
		assert( SSBlizObject );
#ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		SSBlizObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the Cloudera and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);
		SSBlizObject=NULL;
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

void CheckInstButtons(IParamBlock *pblock,HWND hptype)
{ int isinst;
  pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
  if (isinst==INSTGEOM)
  { TurnButton(hptype,IDC_AP_OBJECTPICK,TRUE);
  }
}
void CheckLifeButtons(int stype,HWND spawn)
{ int rep;
  rep = SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_GETCURSEL,0,0);
  TurnButton(spawn,IDC_AP_LIFEQUEUEADD,(stype>1));
  TurnButton(spawn,IDC_AP_LIFEQUEUEREPL,(stype>1)&&(rep>-1));
  TurnButton(spawn,IDC_AP_LIFEQUEUEDEL,(stype>1)&&(rep>-1));
}

void CheckSpawnButtons(IParamBlock *pblock,HWND spawn,int repi)
{ int stype;
  pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  EnableWindow(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),(stype>1));
  TurnButton(spawn,IDC_AP_OBJECTQUEUEPICK,(stype>1));
  TurnButton(spawn,IDC_AP_OBJQUEUEREPLACE,(stype>1)&&(repi>-1));
  TurnButton(spawn,IDC_AP_OBJQUEUEDELETE,(stype>1)&&(repi>-1));
  CheckLifeButtons(stype,spawn);
}

void CheckPickButtons(IParamBlock *pblock,HWND hptype,HWND spawn,int repi)
{ CheckInstButtons(pblock,hptype);
  CheckSpawnButtons(pblock,spawn,repi);
}

class EmitterCreateCallback : public CreateMouseCallBack {
	public:
		SuprSprayParticle *po;
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int EmitterCreateCallback::proc(
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
		switch(point)  {
			case 0:
				// if hidden by category, re-display particles and objects
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				po->pblock->SetValue(PB_EMITRWID,0,0.01f);
				po->pmapParam->Invalidate();
				break;

			case 1: {
//				mat.IdentityMatrix();
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				po->pblock->SetValue(PB_EMITRWID,0,Length(p1-p0));
				po->pmapParam->Invalidate();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 || Length(p1-p0)<0.1f) {						
						return CREATE_ABORT;
					} else { CheckPickButtons(po->pblock,po->hptype,po->spawn,po->pickCB.repi);
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
	return 1;
	}

static EmitterCreateCallback emitterCallback;

CreateMouseCallBack* SuprSprayParticle::GetCreateMouseCallBack() 
	{
	emitterCallback.po = this;
	return &emitterCallback;
	}

class SSParticleGenDlgProc : public ParamMapUserDlgProc {
	public:
		CommonParticle *po;

		SSParticleGenDlgProc(CommonParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void SSParticleGenDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
  int birthmeth;
  po->pblock->GetValue(PB_BIRTHMETHOD,0,birthmeth,FOREVER);
  if (birthmeth)
  { SpinnerOff(po->hgen,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
    SpinnerOn(po->hgen,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
  }else 
  { SpinnerOn(po->hgen,IDC_SP_GENRATESPIN,IDC_SP_GENRATE);
    SpinnerOff(po->hgen,IDC_SP_GENTTLSPIN,IDC_SP_GENTTL);
  }
}

BOOL SSParticleGenDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{  int acourse;
       float size;
      switch (msg) 
	  { case WM_INITDIALOG: {
		    Update(t);
			break;
			}
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
					   BOOL bliz=(po->ClassID()==BLIZZARD_CLASS_ID);
					   if (bliz) po->pblock->SetValue(PB_METACOURSEVB,t,size/3.0f);
					   else po->pblock->SetValue(PB_METACOURSEV,t,size/3.0f);
					   po->pmapPType->Invalidate();
					 }
					break;
				   }
				case IDC_AP_NEWSEED:
					{ srand( (unsigned)time( NULL ) );
					  int newseed=rand() % 25001;
					  po->pblock->SetValue(PB_RNDSEED,0,newseed);
					  po->pmapPGen->Invalidate();
					}
			        return TRUE;
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
void CourseCheck(CommonParticle *po,HWND hWnd,TimeValue t)
{ int acourse;
  float size;
  po->pblock->GetValue(PB_METAAUTOCOARSE,t,acourse,FOREVER);
  BOOL bliz=(po->ClassID()==BLIZZARD_CLASS_ID);
  if (acourse) 
  { float mc1,mc2,mc,mcv;
	po->pblock->GetValue(PB_SIZE,t,size,FOREVER);mc=size/coursedivider;mcv=size/3.0f;
	po->pblock->GetValue(PB_METACOURSE,t,mc1,FOREVER);
	if (bliz) po->pblock->GetValue(PB_METACOURSEVB,t,mc2,FOREVER);
	else po->pblock->GetValue(PB_METACOURSEV,t,mc2,FOREVER);
	if ((mc1!=mc)||(mc2!=mcv))
	{ po->pblock->SetValue(PB_METACOURSE,t,mc);
	  if (bliz) po->pblock->SetValue(PB_METACOURSEVB,t,mcv);
	  else po->pblock->SetValue(PB_METACOURSEV,t,mcv);
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
void SetMapVals(IParamBlock *pblock,HWND hWnd,TimeValue t)
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
    if (maptype==2)
	{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
	  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
	}
    else if (maptype)
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

void MappingStuff(IParamBlock *pblock,HWND hWnd,TimeValue t,BOOL isbliz)
{ int maptype;
  pblock->GetValue((isbliz?PB_CUSTOMMTL2:PB_CUSTOMMTL),t,maptype,FOREVER);
  if (isbliz)
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPPLANAR),!maptype);
  if (maptype)
  { SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
    SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
    EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
  }
  else SetMapVals(pblock,hWnd,t);
}
void CheckStretchBox(HWND hWnd,CommonParticle *po)
{ int enon=0;
  if (IsStdMtl(po->cnode))
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),TRUE);
  } else EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
  SpinnerOn(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),TRUE);
  po->pmapPSpin->Invalidate();
}

void StretchStuff(int dir,int isphase,HWND hWnd,CommonParticle *po)
{ if (dir==0)
  { EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,FALSE,isphase);
  }
  else if (dir==1)
  {	CheckStretchBox(hWnd,po);
	SpinStuff(hWnd,FALSE,isphase);
  }  
  else if (dir==2)
  {	EnableWindow(GetDlgItem(hWnd,IDC_AP_PBLURON),FALSE);
	SpinnerOff(hWnd,IDC_AP_STRETCHSPIN,IDC_AP_STRETCH);
	EnableWindow(GetDlgItem(hWnd,IDC_AP_STRETCH_TXT),FALSE);
	SpinStuff(hWnd,TRUE,isphase);
  }
}

void AniFr(HWND hWnd,IParamBlock *pblock)
{ int anitype;
  pblock->GetValue(PB_ANIMATIONOFFSET,0,anitype,FOREVER);
  if (anitype>1)
	  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  else	SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),anitype>1);
}
void InstIn(CommonParticle *po,HWND hWnd, TimeValue t)
{ int ison;
  BOOL isbliz;
  MetaOff(hWnd);
  StdStuff(hWnd,FALSE);
  InstStuff(hWnd,TRUE,po->spawn,po->hparam);
  AniFr(hWnd,po->pblock);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,TRUE);
  isbliz=(po->ClassID()==BLIZZARD_CLASS_ID);
  po->pblock->GetValue(PB_SPINAXISTYPE,0,ison,FOREVER);
  if (isbliz) SpinStuff(po->hrot,ison,TRUE);
  else StretchStuff(ison,TRUE,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_AUTOCOARSE),FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_DRAFTMODE),FALSE);
  MappingStuff(po->pblock,hWnd,t,isbliz);
  int stype;po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
  ObjectMutQueOn(stype,po->spawn,po->pickCB.repi);
  if (!isbliz)
    EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
  po->pmapPSpin->Invalidate();
}

void MetaIn(CommonParticle *po,HWND hWnd,TimeValue t)
{ BOOL isbliz;
  isbliz=(po->ClassID()==BLIZZARD_CLASS_ID);
  int mon,pname=(isbliz?PB_CUSTOMMTL2:PB_CUSTOMMTL);
  po->pblock->GetValue(pname,t,mon,FOREVER);
  if (mon>0) po->pblock->SetValue(pname,t,0);
  SpinnerOn(hWnd,IDC_SP_METTENSSPIN,IDC_SP_METTENS);
  SpinnerOn(hWnd,IDC_SP_METTENSVARSPIN,IDC_SP_METTENSVAR);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENS_TXT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_TXT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_PCNT),TRUE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_AUTOCOARSE),TRUE);
  CourseCheck(po,hWnd,t);
  if (isbliz) SpinStuff(po->hrot,FALSE,FALSE);
  else StretchStuff(0,FALSE,po->hrot,po);
  SpinMainStuff(po->hrot,FALSE);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  int showtype;
  po->pblock->GetValue(PB_VIEWPORTSHOWS,0,showtype,FOREVER);
  if (showtype==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,0);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
  StdStuff(hWnd,FALSE);
  InstStuff(hWnd,FALSE,po->spawn,po->hparam);
  isbliz=(po->ClassID()==BLIZZARD_CLASS_ID);
  MappingStuff(po->pblock,hWnd,t,isbliz);
  ObjectMutQueOff(po->spawn);
  if (!isbliz) EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_DRAFTMODE),TRUE);
  int stype;
  po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
//  if (stype>1) po->pblock->SetValue(PB_SPAWNTYPE,0,0);  AllSpawnBad(po->spawn,0,FALSE);
  int ison; po->pblock->GetValue((isbliz?PB_BLIPCOLLIDE_ON:PB_IPCOLLIDE_ON),t,ison,FOREVER);
  if (ison) stype=0;
  SpawnStuff(po->spawn,stype);
  po->pmapPSpin->Invalidate();
}

void StdOn(CommonParticle *po,HWND hWnd,TimeValue t)
{ int ison,mon;
  BOOL isbliz;
  StdStuff(hWnd,TRUE);
  isbliz=(po->ClassID()==BLIZZARD_CLASS_ID);
  int pname=(isbliz?PB_CUSTOMMTL2:PB_CUSTOMMTL);
  po->pblock->GetValue(pname,t,mon,FOREVER);
  if (mon>0) po->pblock->SetValue(pname,t,0);
  po->pblock->GetValue(PB_SPINAXISTYPE,0,ison,FOREVER);
  if (isbliz) SpinStuff(po->hrot,ison,TRUE);
  else StretchStuff(ison,TRUE,po->hrot,po);
  SpinMainStuff(po->hrot,TRUE);
  int facing;
  po->pblock->GetValue(PB_PARTICLETYPE,t,facing,FOREVER);
/*
	if ((facing==RENDTYPE5)||(facing==RENDTYPE6))
	{		
		po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
		if (facing==2) 
			po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
		EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
	}
	else  
	*/
		EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,FALSE);
  int showtype;
  po->pblock->GetValue(PB_VIEWPORTSHOWS,0,showtype,FOREVER);
  if (showtype==3) po->pblock->SetValue(PB_VIEWPORTSHOWS,0,0);
  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPBOX),FALSE);
 // SpinnerOn(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
  MetaOff(hWnd);
  InstStuff(hWnd,FALSE,po->spawn,po->hparam);
  MappingStuff(po->pblock,hWnd,t,isbliz);
  ObjectMutQueOff(po->spawn);
  if (!isbliz)
    EnableWindow(GetDlgItem(po->hrot,IDC_AP_PARTICLEDIRTRAVL),TRUE);
  po->pmapPSpin->Invalidate();
}

class BLParticleDlgProc : public ParamMapUserDlgProc {
	public:
		BlizzardParticle *po;

		BLParticleDlgProc(BlizzardParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL BLParticleDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			break;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_VIEWDISPMESH:
				  {po->valid=FALSE;
				   int subtree,custmtl;
					po->pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					po->theSuprSprayDraw.t=t;
					po->theSuprSprayDraw.anioff=anioff;
					TimeValue aniend=GetAnimEnd();
					int anifr;
					anifr=aniend+GetTicksPerFrame();
					po->theSuprSprayDraw.anifr=anifr;
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
					po->theSuprSprayDraw.t=t;
					po->theSuprSprayDraw.anioff=anioff;
					TimeValue aniend=GetAnimEnd();
					int anifr;
					anifr=aniend+GetTicksPerFrame();
					po->theSuprSprayDraw.anifr=anifr;
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
class SSParticleDlgProc : public ParamMapUserDlgProc {
	public:
		SuprSprayParticle *po;

		SSParticleDlgProc(SuprSprayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL SSParticleDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			break;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_VIEWDISPMESH:
				  {po->valid=FALSE;
				   int subtree,custmtl;
					po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
					po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
					int anioff;
	  				po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					po->theSuprSprayDraw.t=t;
					po->theSuprSprayDraw.anioff=anioff;
					TimeValue aniend=GetAnimEnd();
					int anifr;
					anifr=aniend+GetTicksPerFrame();
					po->theSuprSprayDraw.anifr=anifr;
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
					po->theSuprSprayDraw.t=t;
					po->theSuprSprayDraw.anioff=anioff;
					TimeValue aniend=GetAnimEnd();
					int anifr;
					anifr=aniend+GetTicksPerFrame();
					po->theSuprSprayDraw.anifr=anifr;
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
void AddMtl(CommonParticle *po,TimeValue t,int custmtl)
{ if (po->cnode)
	{ int subtree,frag,submtl=0;
    po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
    po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	if ((po->custnode)&&(frag==INSTGEOM)&& custmtl) 
		po->AssignMtl(po->cnode,po->custnode,subtree,t);
	po->valid=FALSE;
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
 }


class SSParticleDisableDlgProc : public ParamMapUserDlgProc {
	public:
		SuprSprayParticle *po;
		ICustButton *iBut;

		SSParticleDisableDlgProc(SuprSprayParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void SSParticleDisableDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
  SetMapVals(po->pblock,po->hptype,t);
  float width;
  po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
  if (width<0.01f) iBut->Disable();
  po->ShowName();
  int chunky;
  po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
  if (chunky==METABALLS) MetaIn(po,po->hptype,t);
  else if (chunky==ISSTD) StdOn(po,po->hptype,t);
  else InstIn(po,po->hptype,t);
}

BOOL SSParticleDisableDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
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
			{ case IDC_AP_OBJECTPICK:
				   { po->flags=0;
					if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateSSBlizMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateSSBlizMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;	
					     po->pickCB.dodist=0;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
			case IDC_AP_UPDATEMTL:
				{ int custmtl;
				  po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
				  AddMtl(po,t,custmtl);
				  if ((custmtl) &&(po->cnode))
				  { EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMEMIT),FALSE);
				    po->origmtl=po->cnode->GetMtl();
				  }
				  break;
				}
			   case IDC_SP_TYPESTD:
				{ StdOn(po,hWnd,t);
				  po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreateSSBlizMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreateSSBlizMode);
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
			  case IDC_SP_TYPEFAC:
			  case IDC_SP_TYPEPIX:
				{ SetMapVals(po->pblock,hWnd,t);
//				  int facing;
//				  po->pblock->GetValue(PB_VIEWPORTSHOWS,t,facing,FOREVER);
//				  if (facing==2) po->pblock->SetValue(PB_VIEWPORTSHOWS,t,1);
//				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),FALSE);
				  po->pmapParam->Invalidate();
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				 break;
				}
			  case IDC_SP_TYPETRI:
			  case IDC_SP_TYPECUB:
			  case IDC_SP_TYPESPC:
			  case IDC_SP_TYPE6PNT:
			  case IDC_SP_TYPESPHERE:
				{ SetMapVals(po->pblock,hWnd,t);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				 break;
				}
			  case IDC_SP_TYPETET:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
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
			  case IDC_SP_TYPEMET:
				  { MetaIn(po,hWnd,t);
				    po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreateSSBlizMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreateSSBlizMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				  break;
				  }
			  case IDC_SP_MAPCUSTOMEMIT:
			  case IDC_SP_MAPCUSTOMINST:
				{ MappingStuff(po->pblock,hWnd,t,FALSE);
				  int dir;
				  po->pblock->GetValue(PB_SPINAXISTYPE,0,dir,FOREVER);
				  if (dir==1)
				  { int subtree,frag,custmtl=0;
				    po->pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
				    po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
				    po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
				    CheckStretchBox(po->hrot,po);
				  }
				  break;}
			  case IDC_SP_TYPEINSTANCE:
				  { int custmtl,vshow,anioff;
					po->pblock->GetValue(PB_VIEWPORTSHOWS,0,vshow,FOREVER);
				    po->pblock->SetValue(PB_CUSTOMMTL,0,1);
					InstIn(po,hWnd,t);
					if (vshow>1)
					{int subtree;
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 po->theSuprSprayDraw.t=t;
					 po->theSuprSprayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					 int anifr;
					 anifr=aniend+GetTicksPerFrame();
					 po->theSuprSprayDraw.anifr=anifr;
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
class BLParticleDisableDlgProc : public ParamMapUserDlgProc {
	public:
		BlizzardParticle *po;
		ICustButton *iBut;

		BLParticleDisableDlgProc(BlizzardParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void BLParticleDisableDlgProc::Update(TimeValue t)
{	if (!po->editOb) return;
	float width;
	po->pblock->GetValue(PB_EMITRWID,0,width,FOREVER);
	int chunky;
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
	if (chunky==METABALLS) MetaIn(po,po->hptype,t);
	else if (chunky==ISSTD) StdOn(po,po->hptype,t);
	else InstIn(po,po->hptype,t);
	if (width<0.01f)
	{ iBut->Disable();
	}
	po->ShowName();
}

BOOL BLParticleDisableDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
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
						{  theCreateSSBlizMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateSSBlizMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;	
					      po->pickCB.dodist = 0;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
			case IDC_AP_UPDATEMTL:
				{ int custmtl;
				  po->pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
				  AddMtl(po,t,custmtl);
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
			  case IDC_SP_TYPEFAC:
			  case IDC_SP_TYPEPIX:
				{ SetMapVals(po->pblock,hWnd,t);
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
				{ SetMapVals(po->pblock,hWnd,t);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),TRUE);
				 break;
				}
			  case IDC_SP_TYPETET:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				  EnableWindow(GetDlgItem(po->hparam,IDC_SP_VIEWDISPMESH),TRUE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPDIST),FALSE);
				  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPTIME),FALSE);
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
			  case IDC_SP_MAPPLANAR:
				{ SpinnerOff(hWnd,IDC_SP_MAPTIMEVALSPIN,IDC_SP_MAPTIMEVAL);
				  SpinnerOff(hWnd,IDC_SP_MAPDISTVALSPIN,IDC_SP_MAPDISTVAL);
				 break;
				}
			   case IDC_SP_TYPESTD:
				{ StdOn(po,hWnd,t);
				  po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreateSSBlizMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreateSSBlizMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				 break;
				}
			  case IDC_SP_TYPEMET:
				  { MetaIn(po,hWnd,t);
				    po->valid=FALSE;
				  if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				  { if (po->creating) 
					{  theCreateSSBlizMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreateSSBlizMode);
					} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
				  }
				  break;
				  }
			  case IDC_SP_MAPCUSTOMEMIT:
			  case IDC_SP_MAPCUSTOMINST:
				  MappingStuff(po->pblock,hWnd,t,TRUE);
				  break;
			  case IDC_SP_TYPEINSTANCE:
				  { int custmtl,vshow;
				     po->pblock->SetValue(PB_CUSTOMMTL2,0,1);
				    InstIn(po,hWnd,t);
					po->pblock->GetValue(PB_VIEWPORTSHOWS,0,vshow,FOREVER);
					if (vshow>1)
					{int subtree,anioff;
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					 po->theSuprSprayDraw.t=t;
					 po->theSuprSprayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					 int anifr;
					 anifr=aniend+GetTicksPerFrame();
					 po->theSuprSprayDraw.anifr=anifr;
					 po->GetTimes(po->times,t,anifr,anioff);
					 if (vshow==2)
					 { po->pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
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

void CommonParticle::GetFilename(TCHAR *filename)
{   _tcscpy(filename,ip->GetDir(APP_PLUGCFG_DIR));
  int len= _tcslen(filename);
  if (len)
  {  if (filename[len-1]!=_T('\\'))
		  _tcscat(filename,_T("\\"));
  }
  if (ClassID()==BLIZZARD_CLASS_ID)
    _tcscat(filename,GetString(IDS_AP_BLIZZARDCST));
  else  _tcscat(filename,GetString(IDS_AP_SSPRAYCST));
}

void CommonParticle::SetupTargetList()		
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
	}
	}
	onerr:if (i<custsettings) 
	{ custsettings=0;
	MessageBox (NULL,GetString(IDS_RB_BADFILE),
            "", MB_ICONINFORMATION);
	}
	if (fileok) fclose(f);
	UpdatePresetListBox(GetDlgItem(hParams2, IDC_SP_SETLIST), custsettings, NameLst);
	}
int CommonParticle::RemSettings(int overwrite,TCHAR *newname)
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
  {fclose(f);remove(filename);custsettings=0;
    delete[] NameLst;NameLst=NULL;
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
int CommonParticle::SaveSettings(int overwrite,TCHAR *newname)
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
  ParamBlockDescID *descVer=((ClassID()==BLIZZARD_CLASS_ID)?BdescVer6:spdescVer6);
  int plength=((ClassID()==BLIZZARD_CLASS_ID)?PBLOCK_LENGTH_BLIZZARD:PBLOCK_LENGTH_SUPRSPRAY);
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
int CommonParticle::GetSettings(int setnum,TCHAR *newname)
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
  ParamBlockDescID *descVer=((ClassID()==BLIZZARD_CLASS_ID)?BdescVer6:spdescVer6);
 int plength=((ClassID()==BLIZZARD_CLASS_ID)?PBLOCK_LENGTH_BLIZZARD:PBLOCK_LENGTH_SUPRSPRAY);
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
  CommonParticle *po = (CommonParticle*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
  if (!po && message!=WM_INITDIALOG) return FALSE;

  switch (message) {
		case WM_INITDIALOG: {
			po = (SuprSprayParticle*)lParam;
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
					  int stype;
					  po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
					  int chunky;
			          po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
					  if (chunky==METABALLS) MetaIn(po,po->hptype,po->ip->GetTime());
				      else 
					  {// AllSpawnBad(po->spawn,stype,TRUE);
						int ison; po->pblock->GetValue((po->ClassID()==BLIZZARD_CLASS_ID?PB_BLIPCOLLIDE_ON:PB_IPCOLLIDE_ON),0,ison,FOREVER);
						if (ison) stype=0;
						SpawnStuff(po->spawn,stype);
						if (chunky==ISSTD) StdOn(po,po->hptype,po->ip->GetTime());
					    else InstIn(po,po->hptype,po->ip->GetTime());
					  }
				 if (chunky==INSTGEOM)
				 { CheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi);
				   int onscreen;
				   po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
				   if (onscreen>1)
				   {int subtree,anioff;
					 po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  				 po->pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
					TimeValue t=po->ip->GetTime();
					 po->theSuprSprayDraw.t=t;
					 po->theSuprSprayDraw.anioff=anioff;
					 TimeValue aniend=GetAnimEnd();
					 int anifr,custmtl;
					 anifr=aniend+GetTicksPerFrame();
					 po->theSuprSprayDraw.anifr=anifr;
					 po->GetTimes(po->times,t,anifr,anioff);
					 po->pblock->GetValue((po->ClassID()==BLIZZARD_CLASS_ID?PB_CUSTOMMTL2:PB_CUSTOMMTL),0,custmtl,FOREVER);
					 if (onscreen==2)
						 po->GetMesh(po->ip->GetTime(),subtree,custmtl);
						else po->GetallBB(po->custnode,subtree,po->ip->GetTime());
				   }
				 }
				 else ObjectMutQueOff(po->spawn);
					  if (stype==EMIT)
					  {	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
						EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
					  }
					  po->pmapParam->Invalidate(); 
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

class ComParticleSpawnDlgProc : public ParamMapUserDlgProc {
	public:
		CommonParticle *po;
		ICustButton *iBut,*iButrep;

		ComParticleSpawnDlgProc(CommonParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void ComParticleSpawnDlgProc::Update(TimeValue t)
{ if (!po->editOb) return;
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
	int chunky,stype;
	po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
	po->pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
//	AllSpawnBad(po->spawn,stype,chunky!=METABALLS);
	int ison; po->pblock->GetValue((po->ClassID()==BLIZZARD_CLASS_ID?PB_BLIPCOLLIDE_ON:PB_IPCOLLIDE_ON),t,ison,FOREVER);
	if (ison) stype=0;
    SpawnStuff(po->spawn,stype);
	if (chunky==INSTGEOM)
		    CheckSpawnButtons(po->pblock,po->spawn,po->pickCB.repi);
	else 
	{ ObjectMutQueOff(po->spawn);
	  CheckLifeButtons(stype,po->spawn);
	}
	if (stype==EMIT) 
	{	SpinnerOff(po->spawn,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
		EnableWindow(GetDlgItem(po->spawn,IDC_AP_MAXSPAWNDIEAFTER_TXT),FALSE);
	}
}

BOOL ComParticleSpawnDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ int dtype=2,stype,rep;	
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
			{ case IDC_AP_NOSPAWN:stype=0;goto spawnradio;
			  case IDC_AP_COLLIDEDIE:
				  stype=1;goto spawnradio;
			  case IDC_AP_SPAWNTRAILS:stype=EMIT;goto spawnradio;
			  case IDC_AP_COLLIDESPAWN:
			  case IDC_AP_DEATHSPAWN: stype=2;
			spawnradio:	
				int chunky;
			po->pblock->GetValue(PB_PARTICLECLASS,0,chunky,FOREVER);
//				AllSpawnBad(po->spawn,stype,chunky!=METABALLS);
				int ison; po->pblock->GetValue((po->ClassID()==BLIZZARD_CLASS_ID?PB_BLIPCOLLIDE_ON:PB_IPCOLLIDE_ON),t,ison,FOREVER);
				if (ison) stype=0;
				SpawnStuff(po->spawn,stype);
				 if (chunky==INSTGEOM)
				    CheckSpawnButtons(po->pblock,hWnd,po->pickCB.repi);
				 else 
				 { ObjectMutQueOff(po->spawn);
				   CheckLifeButtons(stype,po->spawn);
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
			      dtype=1;goto dopick;
			   case IDC_AP_OBJQUEUEREPLACE:
				  dtype=2;
				  dopick:
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateSSBlizMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateSSBlizMode);
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
						TurnButton(hWnd,IDC_AP_OBJQUEUEREPLACE,0);
						TurnButton(hWnd,IDC_AP_OBJQUEUEDELETE,0);
					if ((po->nlist.Count()>0)&&(i>-1))
					{	theHold.Begin();
						po->DeleteFromList(i,0);
						theHold.Accept(GetString(IDS_AP_OBJDEL));
					}
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				break;
				}
				case IDC_AP_LIFEQUEUEDEL:
				{  int i = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),
							LB_GETCURSEL,0,0);
					int Lcnt=po->llist.Count();
					if ((Lcnt>0)&&(i>-1)&&(i<Lcnt))
					{	theHold.Begin();
						theHold.Put(new LifeListRestore(po));
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
					theHold.Put(new LifeListRestore(po));
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
					po->valid=FALSE;
					po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				  break;
				}
				case IDC_AP_LIFEQUEUEREPL:
				{ int i;
				 rep = SendMessage(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),
							LB_GETCURSEL,0,0);
				  if (rep>-1)
				  {	po->pblock->GetValue(PB_SPAWNLIFEVLUE,t,i,FOREVER);
					theHold.Begin();
					theHold.Put(new LifeListRestore(po));
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
		default:
			return FALSE;
	}
	return TRUE;
}
class BParticleSpinDlgProc : public ParamMapUserDlgProc {
	public:
		CommonParticle *po;

		BParticleSpinDlgProc(CommonParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void BParticleSpinDlgProc::Update(TimeValue t)
{   int axis;
	po->pblock->GetValue(PB_SPINAXISTYPE,t,axis,FOREVER);
	SpinStuff(po->hrot,axis==1,TRUE);
	int ison; po->pblock->GetValue(PB_BLIPCOLLIDE_ON,t,ison,FOREVER);
	int stype; po->pblock->GetValue(PB_SPAWNTYPE,t,stype,FOREVER);
	IPCControls(po->hrot,po->spawn,stype,ison);
}

BOOL BParticleSpinDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: 
		{ Update(t);
		  break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_AP_PARTICLEDIRRND:
			  { SpinStuff(hWnd,FALSE,TRUE);
			  }
			  break;
			  case IDC_AP_PARTICLEDIRUSER:
				   SpinStuff(hWnd,TRUE,TRUE);
				break;
			  case IDC_INTERP_BOUNCEON:
				{  int ison; po->pblock->GetValue(PB_BLIPCOLLIDE_ON,t,ison,FOREVER);
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
class SSParticleSpinDlgProc : public ParamMapUserDlgProc {
	public:
		CommonParticle *po;

		SSParticleSpinDlgProc(CommonParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void SSParticleSpinDlgProc::Update(TimeValue t)
{   int axis;
	po->pblock->GetValue(PB_SPINAXISTYPE,t,axis,FOREVER);
	StretchStuff(axis,TRUE,po->hrot,po);
	int ison; po->pblock->GetValue(PB_IPCOLLIDE_ON,t,ison,FOREVER);
	int stype; po->pblock->GetValue(PB_SPAWNTYPE,t,stype,FOREVER);
	IPCControls(po->hrot,po->spawn,stype,ison);
}

BOOL SSParticleSpinDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: 
		{ Update(t);
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_AP_PARTICLEDIRTRAVL:
			    StretchStuff(1,TRUE,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRRND:
				StretchStuff(0,TRUE,hWnd,po);
			  break;
			  case IDC_AP_PARTICLEDIRUSER:
				StretchStuff(2,TRUE,hWnd,po);
				break;
			  case IDC_INTERP_BOUNCEON:
				{  int ison; po->pblock->GetValue(PB_IPCOLLIDE_ON,t,ison,FOREVER);
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

void CommonParticle::ResetSystem(TimeValue t,BOOL full)
{	lc.lastmin=-1;lc.lastcollide=-1;
	rcounter=0;
	vcounter=0;
	if (full)
	{ tvalid = t;
	  valid  = TRUE;
	}
}

//--- SuprSprayParticle Methods--------------------------------------------

SuprSprayParticle::SuprSprayParticle()
{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
     int tpf=GetTicksPerFrame();

	MakeRefByID(FOREVER, PBLK, CreateParameterBlock(spdescVer6, PBLOCK_LENGTH_SUPRSPRAY, CURRENT_VERSION));
	pblock->SetValue(PB_SPEED,0,10.0f);
	pblock->SetValue(PB_SPEEDVAR,0,0.0f);
	pblock->SetValue(PB_OFFAXIS,0,0.0f);
	pblock->SetValue(PB_AXISSPREAD,0,0.0f);
	pblock->SetValue(PB_OFFPLANE,0,0.0f);
	pblock->SetValue(PB_PLANESPREAD,0,0.0f);
	pblock->SetValue(PB_PBIRTHRATE,0,10);
	pblock->SetValue(PB_PTOTALNUMBER,0,100);
	pblock->SetValue(PB_BIRTHMETHOD,0,0);
	pblock->SetValue(PB_DISPLAYPORTION,0,0.1f);
	pblock->SetValue(PB_EMITSTART,0,TimeValue(0));
	pblock->SetValue(PB_EMITSTOP,0,(TimeValue)30*FToTick);// correct constant?
	pblock->SetValue(PB_DISPUNTIL,0,100*FToTick);// correct constant?
	pblock->SetValue(PB_LIFE,0,30*FToTick);// correct constant?
	pblock->SetValue(PB_LIFEVAR,0,0.0f);
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
	pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	pblock->SetValue(PB_MAPPINGTYPE,0,0);
	pblock->SetValue(PB_MAPPINGTIME,0,30*FToTick);
	pblock->SetValue(PB_MAPPINGDIST,0,100.0f);

	pblock->SetValue(PB_SPINTIME,0,30*FToTick);
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
	pblock->SetValue(PB_SPAWNGENS,0,1);
	pblock->SetValue(PB_SPAWNCOUNT,0,1);
	pblock->SetValue(PB_METACOURSEV,0,1.0f);
	pblock->SetValue(PB_SPAWNPERCENT,0,100);
	pblock->SetValue(PB_SPAWNMULTVAR,0,0.0f);
	pblock->SetValue(PB_SSNOTDRAFT,0,0);
	pblock->SetValue(PB_SSSPAWNDIEAFTER,0,0);
	pblock->SetValue(PB_IPCOLLIDE_ON,0,0);
	pblock->SetValue(PB_IPCOLLIDE_STEPS,0,2);
	pblock->SetValue(PB_IPCOLLIDE_BOUNCE,0,1.0f);
	pblock->SetValue(PB_IPCOLLIDE_BOUNCEVAR,0,0.0f);
	sdata=NULL;
	cnode=NULL;
	custnode=NULL;
	custname=TSTR(_T(" "));
	ResetSystem(0,FALSE);
	size=43*isize+fsize*37;
	times.tl.SetCount(0);
	cmesh=NULL;
	dispmesh=NULL;
	dispt=-99999;
	theSuprSprayDraw.bboxpt=NULL;
	nmtls.ZeroCount();
	deftime=0;
	parts.points.ZeroCount();
	maincount=0;
	cancelled=FALSE;
	wasmulti=FALSE;
	nlist.ZeroCount();
	llist.ZeroCount();
	dflags=APRTS_ROLLUP_FLAGS;
	backpatch=TRUE;
	origmtl=NULL;
	ClearAFlag(A_NOTREND);
    stepSize=GetTicksPerFrame();
}

SuprSprayParticle::~SuprSprayParticle()
{	if (sdata) {delete[] sdata;sdata=NULL;}
//	SetFlag(dflags,STDMTL_ROLLUP1_OPEN,ip->IsRollupPanelOpen(hPanelBasic));
	DeleteAllRefsFromMe();
	pblock=NULL;
	parts.FreeAll();
	times.tl.SetCount(0);
	times.tl.Shrink();
	nmtls.ZeroCount();nmtls.Shrink();
	llist.ZeroCount();llist.Shrink();
	nlist.ZeroCount();nlist.Shrink();
	if (cmesh) delete[] cmesh;
	if (theSuprSprayDraw.bboxpt) delete[] theSuprSprayDraw.bboxpt;
	if (dispmesh) delete dispmesh;
}

Matrix3 CommonParticle::TumbleMat(int index,float amount, float scale)
	{
	Matrix3 mat;
	Quat q;
	float ang[3];

	srand(int(PARTICLE_SEED) * Perm(index) + int(PARTICLE_SEED));
	
	for (int i=0; i<3; i++) {
		ang[i] = (float(2*rand())/float(RAND_MAX) - 1.0f);
		if (amount>0.0f) {
			float off = 8725.0f*i;
			ang[i] += noise3((parts[index]+Point3(off,off,off))*scale)*amount;
			}
		ang[i] *= TWOPI;
		}
	
	EulerToQuat(ang,q);
	q.MakeMatrix(mat);
	return mat;
	}

TimeValue CommonParticle::ParticleLife(TimeValue t, int i)
{   int pcount=parts.Count();
	if (!(i<pcount)) return 0;
	return sdata[i].L;
}

Point3 BlizzardParticle::ParticlePosition(TimeValue t,int i)
{	int pcount=parts.points.Count();
	if (!(i<pcount)) return Zero;
	Point3 retpt=parts.points[i];
	return retpt;
}

Point3 SuprSprayParticle::ParticlePosition(TimeValue t,int i)
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
		int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum));
		if (mnum>=0) zoffset=cmesh[mnum].getBoundingBox().Center().z;
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

Point3 SuprSprayParticle::ParticleVelocity(TimeValue t,int i)
{	Point3 retvel=Zero;
	int pcount=parts.vels.Count();
	if (i<pcount)
		retvel=parts.vels[i];
	return retvel;
}

float SuprSprayParticle::ParticleSize(TimeValue t,int i)
{	float strlen=1.0f;
	float boxlen=1.0f;
	float dlgsize;
	int pcount=parts.radius.Count();
	if (!(i<pcount)) return 0.0f;
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
		int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum));
		if (mnum>=0) boxlen=cmesh[mnum].getBoundingBox().Width().z;
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

int SuprSprayParticle::ParticleCenter(TimeValue t,int i)
{	int ptype,isinst;
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
	if (isinst==INSTGEOM) return PARTCENTER_CENTER;
	if (ptype==RENDTET) return PARTCENTER_HEAD;
	return PARTCENTER_CENTER;	
}

Point3 BlizzardParticle::ParticleVelocity(TimeValue t,int i)
{	Point3 retvel=Zero;
	int pcount=parts.vels.Count();
	if (i<pcount)
		retvel=parts.vels[i];
	return retvel;
}

float BlizzardParticle::ParticleSize(TimeValue t,int i)
{	int isinst;
	int pcount=parts.radius.Count();
	if (!(i<pcount)) return 0.0f;
	float radius=1.0f,size=parts.radius[i];
	pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
	if ((isinst==INSTGEOM) &&(custnode))
	{	int anioff;
		TimeValue aniend=GetAnimEnd();
		int anifr=aniend+GetTicksPerFrame();
		pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
		TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
		int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum));
		if (mnum>=0) 
		radius=(cmesh[mnum].getBoundingBox().Width()).z;
	}
	return radius*size;	
}

int BlizzardParticle::ParticleCenter(TimeValue t,int i)
{	return PARTCENTER_CENTER;	
}

int CommonParticle::RenderBegin(TimeValue t, ULONG flags)
	{ 	SetAFlag(A_RENDER);
  		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		cancelled=FALSE;
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)1);
	return 0;
	}

int CommonParticle::RenderEnd(TimeValue t)
	{
		ClearAFlag(A_RENDER);
		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		int isinst;
		pblock->GetValue(PB_PARTICLECLASS,0,isinst,FOREVER);
		if ((isinst==INSTGEOM) &&(custnode))
			custnode->SetProperty(PROPID_FORCE_RENDER_MESH_COPY,(void *)0);
	return 0;
	}

void SuprSprayParticle::BeginEditParams(
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
			descParamSuprSpray,PARAMSuprSpray_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_SUPERSPRAY),
			GetString(IDS_RB_PARAMETERS),
			dflags&APRTS_ROLLUP1_OPEN?0:APPENDROLL_CLOSED);

		pmapPGen = CreateCPParamMap(
			descParamPGen,PARAMPGEN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_GEN_SS),
			GetString(IDS_RB_PGEN),
			dflags&APRTS_ROLLUP2_OPEN?0:APPENDROLL_CLOSED);
		
		pmapPType = CreateCPParamMap(
			descParamPType,PARAMPTYPE_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTTYPE_SS),
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

		}
	spawn=pmapSpawn->GetHWnd();
	hparam=pmapParam->GetHWnd();
	hgen=pmapPGen->GetHWnd();
	hptype=pmapPType->GetHWnd();
	hrot=pmapPSpin->GetHWnd();
	hbubl=pmapBubl->GetHWnd();
	if (pmapPType) pmapPType->SetUserDlgProc(new SSParticleDisableDlgProc(this));
	if (pmapPGen) pmapPGen->SetUserDlgProc(new SSParticleGenDlgProc(this));
	if (pmapParam) pmapParam->SetUserDlgProc(new SSParticleDlgProc(this));
	if (pmapSpawn) pmapSpawn->SetUserDlgProc(new ComParticleSpawnDlgProc(this));
	if (pmapPSpin) pmapPSpin->SetUserDlgProc(new SSParticleSpinDlgProc(this));
}	

void SuprSprayParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{TimeValue t0,t2;
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
	}else
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG)NULL);
	ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
	}

void CommonParticle::MapKeys(TimeMap *map,DWORD flags)
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
//	pblock->GetValue(PB_OFFSETAMOUNT,0,TempTime,FOREVER);
//	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_OFFSETAMOUNT,0,TempTime);
	if (ClassID()==SUPRSPRAY_CLASS_ID)
	{	pblock->GetValue(PB_BUBLPER,0,TempTime,FOREVER);
		TempTime=map->map(TempTime);
		pblock->SetValue(PB_BUBLPER,0,TempTime);
	} 
}  

Interval CommonParticle::GetValidity(TimeValue t)
	{
	// For now...
	return Interval(t,t);
	}

void SuprSprayParticle::BuildEmitter(TimeValue t, Mesh& amesh)
	{
	float width,hwidth,u;
	mvalid = FOREVER;
	pblock->GetValue(PB_EMITRWID,t,width,mvalid);
//	width  *= 0.5f;
	hwidth =width*0.5f;

	mesh.setNumVerts(23);
	mesh.setNumFaces(18);
	for (int i=0; i<12; i++) {
		u = float(i)/12.0f * TWOPI;
		mesh.setVert(i, Point3((float)cos(u) * width, (float)sin(u) * width, 0.0f));
	}
	for (i=0; i<12; i++) 
	{ int i1 = i+1;
	  if (i1==12) i1 = 0;
	  mesh.faces[i].setEdgeVisFlags(1,0,0);
	  mesh.faces[i].setSmGroup(0);
	  mesh.faces[i].setVerts(i,i1,12);
	}
	mesh.setVert(12, Point3(0.0f, 0.0f, 0.0f));
	mesh.setVert(13, Point3(width,0.0f, width));
	mesh.setVert(14, Point3(0.0f, 0.0f, width));
	mesh.setVert(15, Point3(-width, 0.0f, width));
	mesh.setVert(16, Point3( width, 0.0f,-width));
	mesh.setVert(17, Point3( 0.0f, 0.0f, -width));
	mesh.setVert(18, Point3(-width, 0.0f, -width));
	mesh.setVert(19, Point3(hwidth, 0.0f,hwidth));
	mesh.setVert(20, Point3(-hwidth, 0.0f,hwidth));
	mesh.setVert(21, Point3(0.0f,hwidth,hwidth));
	mesh.setVert(22, Point3(0.0f,-hwidth,hwidth));

	mesh.faces[12].setVerts(13,14,17);
	mesh.faces[12].setEdgeVisFlags(1,1,0);
	mesh.faces[12].setSmGroup(0);

	mesh.faces[13].setEdgeVisFlags(0,1,1);
	mesh.faces[13].setSmGroup(0);
	mesh.faces[13].setVerts(13,17,16);

	mesh.faces[14].setEdgeVisFlags(1,1,0);
	mesh.faces[14].setSmGroup(0);
	mesh.faces[14].setVerts(14,15,18);

	mesh.faces[15].setEdgeVisFlags(0,1,1);
	mesh.faces[15].setSmGroup(0);
	mesh.faces[15].setVerts(14,18,17);

	mesh.faces[16].setEdgeVisFlags(1,1,1);
	mesh.faces[16].setSmGroup(0);
	mesh.faces[16].setVerts(19,14,20);

	mesh.faces[17].setEdgeVisFlags(1,1,1);
	mesh.faces[17].setSmGroup(0);
	mesh.faces[17].setVerts(21,14,22);

	mesh.InvalidateGeomCache();
	}

int CommonParticle::CountLive()
{	int c=0;
	for (int i=0; i<parts.Count(); i++)
	  {if (parts.Alive(i)) c++;}
	return c;
}

void CommonParticle::RetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
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

void CommonParticle::DoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) DoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if((subtree)||(nxtnode->IsGroupMember())) RetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void CommonParticle::AssignMtl(INode *node,INode *topnode,int subtree,TimeValue t) 
{	Mtl *submtl;
	MultiMtl *newmat=NULL;
	Mtl *nmtl=NULL;
	TSTR newname;
	MtlBaseLib glib;
	int tabmax=256;
	newname=TSTR(_T("CMat"))+node->GetName();
	if (_tcslen(newname)>16) newname[16]='\0';
	int numsubs=0,numtabs=0,nCount=nlist.Count();
	nmtls.SetCount(nCount);
    mttab.SetCount(tabmax);
    submtl=custnode->GetMtl();
	INode *tmpnode=custnode;
	backpatch=FALSE;
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
void CommonParticle::CntRetrieveMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
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

void CommonParticle::CntDoGroupMtls(INode *node,int subtree,int *numsubs,int *numtabs,int *tabmax,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
   for (int i=0;i<nc;i++)
   { INode *nxtnode=node->GetChildNode(i);
	  if (nxtnode->IsGroupHead()) CntDoGroupMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) CntRetrieveMtls(nxtnode,subtree,numsubs,numtabs,tabmax,t);
  }
}

void CommonParticle::GetSubs(INode *node,INode *topnode,int subtree,TimeValue t) 
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

void CommonParticle::GetNextBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
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
	{ theSuprSprayDraw.bboxpt[tcount].bpts.Resize((*tabmax)+=256);}
    Matrix3 ctm = node->GetObjTMAfterWSM(t);
	if (node!=custnode)
  { theSuprSprayDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f)*ctm-boxcenter;
  }	
  else theSuprSprayDraw.bboxpt[tcount].bpts[*count].Suboffset=Point3(0.0f,0.0f,0.0f);
	ctm.NoTrans();
  Box3 bbox=triOb->GetMesh().getBoundingBox();
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[0]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[1]=Point3(bbox.pmax[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[2]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[3]=Point3(bbox.pmax[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[4]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmax[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[5]=Point3(bbox.pmin[0],bbox.pmax[1],bbox.pmin[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[6]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmin[2])*ctm;
  theSuprSprayDraw.bboxpt[tcount].bpts[*count].pts[7]=Point3(bbox.pmin[0],bbox.pmin[1],bbox.pmax[2])*ctm;
  (*count)++;
  if (triOb!=cobj) triOb->DeleteThis();
  }
}

void CommonParticle::DoGroupBB(INode *node,int subtree,int *count,int *tabmax,Point3 boxcenter,TimeValue t,int tcount,INode *onode)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) DoGroupBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	  else if ((subtree)||(nxtnode->IsGroupMember())) GetNextBB(nxtnode,subtree,count,tabmax,boxcenter,t,tcount,onode);
	}
}

void CommonParticle::GetallBB(INode *custnode,int subtree,TimeValue t)
{ int tabmax=256;
  int count=1,ocount=times.tl.Count();
  if (ocount>0) count=ocount;
  if (theSuprSprayDraw.bboxpt) delete[] theSuprSprayDraw.bboxpt;
  theSuprSprayDraw.bboxpt=NULL;
  INode *tmpnode;
  if (custnode!=NULL)
  { theSuprSprayDraw.bboxpt=new boxlst[count];
    int cgen;
    for (int tcount=0;tcount<count;tcount++)
    { TimeValue tofs=(ocount>0?times.tl[tcount].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[tcount].gennum-1:-1);
	  if ((cgen>-1)&&(cgen<nlist.Count()))
	  { if (!(tmpnode=nlist[cgen])) tmpnode=custnode;
	  } else tmpnode=custnode;
	  theSuprSprayDraw.bboxpt[tcount].bpts.SetCount(tabmax);
      theSuprSprayDraw.bboxpt[tcount].numboxes=0;
      Matrix3 ctm = tmpnode->GetObjTMAfterWSM(tofs);
      boxcenter=Zero*ctm;
	  if (tmpnode->IsGroupHead())
	    DoGroupBB(tmpnode,subtree,&(theSuprSprayDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  else
      GetNextBB(tmpnode,subtree,&(theSuprSprayDraw.bboxpt[tcount].numboxes),&tabmax,boxcenter,tofs,tcount,tmpnode);
	  theSuprSprayDraw.bboxpt[tcount].bpts.SetCount(theSuprSprayDraw.bboxpt[tcount].numboxes);
	 theSuprSprayDraw.bboxpt[tcount].bpts.Shrink();
    }
  }
}

void CommonParticle::CheckTree(INode *node,Matrix3 tspace,Mesh *cmesh,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
{ Object *pobj;	  
  TriObject *triOb;
  Point3 deftex=Point3(0.5f,0.5f,0.0f);
  TVFace Zerod;
  Zerod.t[0]=0;Zerod.t[1]=0;Zerod.t[2]=0;
  int nc,i,j,subv=0,subf=0,subtvnum=0,tface,tvert;

  if (subtree)
  { if ((nc=node->NumberOfChildren())>0)
	for (i=0;i<nc;i++)
	  CheckTree(node->GetChildNode(i),tspace,cmesh,numV,numF,tvnum,ismapped,t,subtree,custmtl);
  }
  if ((triOb=TriIsUseable(pobj = node->EvalWorldState(t).obj,t))!=NULL)
  {	Point3 Suboffset;
    Matrix3 ctm = node->GetObjTMAfterWSM(t);
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
/*	Matrix3 objoff(1);
	Point3 pos = node->GetObjOffsetPos();objoff.PreTranslate(pos);
	Quat quat = node->GetObjOffsetRot();PreRotateMatrix(objoff, quat);
	ScaleValue scaleValue = node->GetObjOffsetScale();ApplyScaling(objoff, scaleValue);*/
	for (j=*numV;j<tvert;j++)
	{ cmesh->verts[j]=(triOb->GetMesh().verts[k]*ctm);//+Suboffset;
	  k++;
	}

	// Multiple Channel Map Support (single line)
	CopyMultipleMapping(cmesh, triOb, numF, tface, deftex, Zerod, subf, mirror);

	*numV+=subv;
	*numF+=subf;
	*tvnum+=subtvnum;
	if (submtls>0)
	CustMtls+=submtls;
	else CustMtls++;
   if (triOb!=pobj) triOb->DeleteThis();
  }
}

void CommonParticle::TreeDoGroup(INode *node,Matrix3 tspace,Mesh *cmesh,int *numV,int *numF,int *tvnum,int *ismapped,TimeValue t,int subtree,int custmtl)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) TreeDoGroup(nxtnode,tspace,cmesh,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	  else if ((subtree)||(nxtnode->IsGroupMember())) CheckTree(nxtnode,tspace,cmesh,numV,numF,tvnum,ismapped,t,subtree,custmtl);
	}
}

void CommonParticle::GetMesh(TimeValue t,int subtree,int custmtl)
{	int tnums,numV,numF,tvnum,ismapped;
    INode *tmpnode;
	tnums=times.tl.Count();
	if (tnums==0) tnums=1;
	if (cmesh) delete[] cmesh;cmesh=NULL;
	if (custnode!=NULL)
	{ cmesh=new Mesh[tnums];
	  int cgen;
	 for (int i=0;i<tnums;i++)
	{ TimeValue tofs=(tnums>1?times.tl[i].tl:t);
	  cgen=(times.tl.Count()>0?times.tl[i].gennum-1:-1);
	  if ((cgen>-1)&&(cgen<nlist.Count()))
	  { if (!(tmpnode=nlist[cgen])) tmpnode=custnode;
	  } else tmpnode=custnode;
	  Matrix3 ptm = tmpnode->GetObjTMAfterWSM(tofs);
	  Matrix3 topspace=Inverse(ptm);
	  numV=numF=tvnum=CustMtls=ismapped=0;
	  if (tmpnode->IsGroupHead())
	   TreeDoGroup(tmpnode,topspace,&cmesh[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	  else
	   CheckTree(tmpnode,topspace,&cmesh[i],&numV,&numF,&tvnum,&ismapped,tofs,subtree,custmtl);
	}
	}
}
/*void ResizeAllParts(ParticleSys &parts,int c)
{ parts.points.Resize(c);parts.points.SetCount(c);
  parts.vels.Resize(c);parts.vels.SetCount(c);
  parts.ages.Resize(c);parts.ages.SetCount(c);
  parts.radius.Resize(c);parts.radius.SetCount(c);
  parts.tension.Resize(c);parts.tension.SetCount(c);
}*/

void CommonParticle::DoSpawn(int j,int spmult,SpawnVars spvars,TimeValue lvar,BOOL emit)
{ if (!emit) spmult--;
  int oldcount,newcount=((oldcount=parts.Count())+spmult); 
  srand(rseed);
  if (spmult)
  {	parts.SetCount(newcount,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	CSavePt *tmp=sdata;
	sdata=new CSavePt[newcount];
	if (tmp)
	{ for (int j=0;j<oldcount;j++) sdata[j]=tmp[j];
	  delete[] tmp;
	}
  }
  int Lcnt=llist.Count();
  int baselife=(Lcnt==0?deftime:(sdata[j].gennum<Lcnt?llist[sdata[j].gennum]:llist[Lcnt-1]))*GetTicksPerFrame();
  if (!emit)
  { sdata[j].gennum++;
	parts.ages[j]=0;}
  Point3 holdv=(emit?-parts.vels[j]:parts.vels[j]);
  for (int i=oldcount;i<newcount;i++)
  { parts.points[i]=parts.points[j];
    parts.vels[i]=holdv;
	parts.radius[i]=parts.radius[j];
	parts.tension[i]=parts.tension[j];
	parts.ages[i]=0;
	memcpy(&sdata[i],&sdata[j],sizeof(CSavePt));
	if (emit) sdata[i].gennum++;
    parts.vels[i]=DoSpawnVars(spvars,parts.vels[j],holdv,&parts.radius[i],&sdata[i].W);
	sdata[i].Vsz=parts.radius[i];
	sdata[i].L=baselife+(int)RND11()*lvar;
 	sdata[i].Mltvar=RND11();
	sdata[i].SpVar=RND0x(99);
	sdata[i].DL=-1;
 }
  if (!emit)
  { parts.vels[j]=DoSpawnVars(spvars,parts.vels[j],holdv,&parts.radius[j],&sdata[j].W);
    sdata[j].Vsz=parts.radius[j];
    sdata[j].L=baselife+(int)RND11()*lvar;
		sdata[j].Mltvar=RND11();
		sdata[j].SpVar=RND0x(99);
		sdata[j].DL=-1;
  }
  int tmprseed=rand();
  rseed=(tmprseed==rseed?rand():tmprseed);
}

#define VEL_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))
#define VAR_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))

BOOL SuprSprayParticle::ComputeParticleStart(TimeValue t0,int c)
	{
	int seed,anioff,tani;
	TimeValue anifr;
	if (c > gCountUpperLimit) c = gCountUpperLimit;
	pblock->GetValue(PB_RNDSEED,t0,seed,FOREVER);
    pblock->GetValue(PB_OFFSETAMOUNT,t0,anifr,FOREVER);
    pblock->GetValue(PB_ANIMATIONOFFSET,t0,anioff,FOREVER);
	srand(seed);					
	parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
 	int pcount=parts.Count();
    if (sdata){delete[] sdata;sdata=NULL;} if (pcount) sdata=new CSavePt[pcount];
	if ((pcount<c)||(c>0 && (!sdata)))
	{   parts.FreeAll();if (sdata) delete sdata;sdata=NULL;maincount=0;
		BOOL playing=GetCOREInterface()->IsAnimPlaying();
		if (playing) GetCOREInterface()->EndAnimPlayback();
	    TSTR name;name=(cnode ? cnode->GetName() : TSTR(GetString(IDS_AP_SUPRSPRAY)));
		TSTR buf; buf=TSTR(GetString(IDS_OFM_PART));
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			GetString(IDS_OFM_ERROR),_T("%s: \n\n%s\n"),buf,name);
	  return (0);
	}
	float tmp;
	int oneframe=GetTicksPerFrame();
	for (int i=0; i<parts.Count(); i++) {
		parts.ages[i] = -1;
		sdata[i].themtl=0;
  		sdata[i].L=RND0x(99);sdata[i].DL=-1;sdata[i].pvar=RND11();
		tmp=RND01();sdata[i].Fo=tmp;
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
		tmp=RND11();sdata[i].M=tmp;
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		parts.vels[i]=Point3(xtmp,ytmp,ztmp);
		sdata[i].To=RND11();
		sdata[i].Vsz=RND11();
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND11(); ytmp=RND11(); xtmp=RND11();
		sdata[i].W=Point3(xtmp,ytmp,ztmp);
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND11(); ytmp=RND11(); xtmp=RND11();
		sdata[i].RV=Point3(xtmp,ytmp,ztmp);
		tmp=RND11();sdata[i].Dis=tmp;
		parts.radius[i]=0.0f;
		parts.tension[i]=RND11();
		sdata[i].Mltvar=RND11();
		sdata[i].SpVar=RND0x(99);
		}
	tvalid = t0-1;
	valid  = TRUE;
	rseed=rand();
	return (1);
	}

void SuprSprayParticle::BirthParticle(INode *node,TimeValue bt,int num,VelDir* ptvel,Matrix3 tmlast)
{
	Matrix3 tm = node->GetObjTMBeforeWSM(bt);
	Matrix3 atm = node->GetObjTMAfterWSM(bt);
	tm.SetRow(3,atm.GetRow(3));
	Point3 vel;
	float Ie,Em,Vm;
	int RotSampleOn;
	srand(rseed);

	pblock->GetValue(PB_EMITVINFL,bt,Ie,FOREVER);
	pblock->GetValue(PB_EMITVMULT,bt,Em,FOREVER);
	pblock->GetValue(PB_EMITVMULTVAR,bt,Vm,FOREVER);
	pblock->GetValue(PB_SUBFRAMEROT,bt,RotSampleOn,FOREVER);

	int MotionOffset,EmitOffset;

	pblock->GetValue(PB_SUBFRAMEMOVE,bt,MotionOffset,FOREVER);
	pblock->GetValue(PB_SUBFRAMETIME,bt,EmitOffset,FOREVER);

	sdata[num].Ts0 = (1.0f + sdata[num].Ts0*ptvel->VSpin)/TWOPI;
	sdata[num].Ts = (float)ptvel->Spin*sdata[num].Ts0;
	parts.tension[num] = ptvel->bstr*(1.0f + parts.tension[num]*ptvel->bstrvar);
	sdata[num].persist = (TimeValue)(ptvel->persist*(1.0f + sdata[num].pvar*ptvel->pvar));
// ok, so I'm using L for M and .z for L.  They were unused float and ints
	sdata[num].M = (sdata[num].L<Ie?Em*(1 + sdata[num].M*Vm):0);  
	sdata[num].L = ptvel->Life + (int)(parts.vels[num].z*ptvel->Vl);
	sdata[num].Vsz *= ptvel->VSz;
	sdata[num].LamTs = ptvel->Phase*(1.0f + sdata[num].LamTs*ptvel->VPhase);

	sdata[num].A = ptvel->ToAmp*(1.0f + sdata[num].A*ptvel->VToAmp);
	sdata[num].LamA = ptvel->ToPhase*(1 + sdata[num].LamA*ptvel->VToPhase);
	sdata[num].To = ptvel->ToPeriod*(1 + sdata[num].To*ptvel->VToPeriod);

	if (ptvel->axisentered==2)
	{	sdata[num].W = Normalize(ptvel->Axis);
		if (ptvel->axisvar>0.0f)
			VectorVar(&sdata[num].W,ptvel->axisvar,180.0f);
	}
	else 
		sdata[num].W = Normalize(sdata[num].W);

	parts.ages[num] = 0;
	float Thetav,Thetah,SSv,SCv;
	Thetav = ptvel->Av180+ptvel->VAv*parts.vels[num].x;
	Thetah = ptvel->Ah180+ptvel->VAh*parts.vels[num].y;
	SSv = (float)sin(Thetav);
	SCv = (float)cos(Thetav);
	vel.y = (float)(SSv*sin(Thetah));
	vel.x = (float)(SSv*cos(Thetah));
	vel.z = SCv;

	Matrix3 OffRotTm;
	if (RotSampleOn) 
		MakeInterpRotXform(tmlast,tm,(1.0f - sdata[num].Fo),OffRotTm);
	else 
		OffRotTm = tm;

	vel = VectorTransform(OffRotTm,vel); //speed value and direction according to orientation

	vel = vel*ptvel->Speed*(1.0f + sdata[num].V.z*ptvel->VSpeed); // speed variation

	parts[num] = tm.GetRow(3);

	if (MotionOffset) 
		parts[num] = ptvel->bps.Position(1.0f - sdata[num].Fo);

	if (EmitOffset) 
		parts[num] += vel*(sdata[num].Fo);
//		parts[num] -= (sdata[num].Fo)*vel;

	if (sdata[num].M != 0.0f)
	{
		Point3 inhVel = ptvel->bps.Speed(1.0f - sdata[num].Fo);
		parts[num] += inhVel*sdata[num].M*sdata[num].Fo;
		vel += inhVel*sdata[num].M; // motion inheritance
	}

	vel /= (float)GetTicksPerFrame();

	parts.vels[num] = vel;
	sdata[num].V = parts[num];
	rseed = rand();
}

void SuprSprayParticle::MovePart(int j,TimeValue dt,BOOL fullframe,int tpf)
{	parts[j] = sdata[j].V+parts.vels[j] * (float)dt;
	// add transverse oscillation
	if (fullframe) sdata[j].V=parts[j];
	Point3 DotPt=(FGT0(parts.vels[j]*(float)tpf)?parts.vels[j]:sdata[j].W);
	DotPt=sdata[j].RV^DotPt;				// Cross Product 
	DotPt=Normalize(DotPt);	       
	float oscil=(sdata[j].To>0.0f?parts.ages[j]*TWOPI/sdata[j].To:0.0f);
	oscil=sdata[j].A*(float)sin(oscil+sdata[j].LamA);
	parts[j] +=DotPt*oscil;
}


void SuprSprayParticle::UpdateParticles(TimeValue t,INode *node)
{	TimeValue t0,dt,t2,grow,fade;
	int i, j, birth,counter,tpf=GetTicksPerFrame(),count=0,anioff;
	VelDir ptvel;
	int isrend=TestAFlag(A_RENDER),bmethod,onscreen,oneframe;
	TimeValue c;
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
	float FperT = GetFrameRate()/(float)TIME_TICKSPERSEC;
    pblock->GetValue(PB_ANIMATIONOFFSET,t,anioff,FOREVER);
	pblock->GetValue(PB_EMITSTART,t,t0,FOREVER);
	pblock->GetValue(PB_SIZE,t,parts.size,FOREVER);	
	pblock->GetValue(PB_BIRTHMETHOD,0,bmethod,FOREVER);
	pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	pblock->GetValue(PB_EMITSTOP,0,t2,FOREVER);
	if (bmethod)
		pblock->GetValue(PB_PTOTALNUMBER,0,c,FOREVER);
	int subtree,frag,custmtl=0;
    pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);

	// Before the start time, nothing is happening
	if (t < t0)
	{	parts.FreeAll();
		if (sdata)
		{	delete[] sdata;
			sdata=NULL;
		}
		ResetSystem(t);
		return;
	}
	int pkind;
	float dpercent;
	pblock->GetValue(PB_DISPLAYPORTION,0,dpercent,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,pkind,FOREVER);

	// Set the particle system to the initial condition
	if ((!valid || t<tvalid) || tvalid<t0) 
	{   int cincr;	
		if (!bmethod)
		{	c=0;
			for (TimeValue ti=t0;ti<=t2;ti+=oneframe)
			{	pblock->GetValue(PB_PBIRTHRATE,ti,cincr,FOREVER);
				if (cincr<0) cincr=0;
				c += cincr;
			}
		}
		if (!isrend) 
			c=(int)(dpercent*(float)c+FTOIEPS);
		if (!ComputeParticleStart(t0,c))
		{ ResetSystem(t);
		  return;
		}
		dispt=t-1;
		maincount=parts.Count();
		ResetSystem(t,FALSE);
	}
	int total;
	total=maincount;
	valid = TRUE;
	int stype,maxgens,spmultb;
	float spawnbvar;
    pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
    pblock->GetValue(PB_SPAWNGENS,0,maxgens,FOREVER);
    pblock->GetValue(PB_SPAWNCOUNT,0,spmultb,FOREVER);
    pblock->GetValue(PB_SPINAXISTYPE,0,ptvel.axisentered,FOREVER);
	SpawnVars spawnvars;
 	spawnvars.axisentered=ptvel.axisentered;
	TimeValue dis;
    pblock->GetValue(PB_DISPUNTIL,0,dis,FOREVER);

	if (t2<t0) 
		t2 = t0;
	TimeValue fstep=oneframe;

//	t2+=fstep;
	TimeValue createover;
	createover = t2-t0+oneframe;
	counter = (isrend?rcounter:vcounter);
	float frate,grate;
	pblock->GetValue(PB_GROWTIME,0,grow,FOREVER);
    pblock->GetValue(PB_FADETIME,0,fade,FOREVER);
	frate=(fade>0.0f?(1-M)/fade:0.0f);
	grate=(grow>0.0f?(1-M)/grow:0.0f);
	float basesize;
	BOOL fullframe;
	if (!isrend)
	{	int offby=t%oneframe;
		if (offby!=0) 
			t-=offby;
	}
	pblock->GetValue(PB_SPAWNSCALESIGN,0,spawnvars.scsign,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDSIGN,0,spawnvars.spsign,FOREVER);
    pblock->GetValue(PB_SPAWNINHERITV,0,spawnvars.invel,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDFIXED,0,spawnvars.spconst,FOREVER);
    pblock->GetValue(PB_SPAWNSCALEFIXED,0,spawnvars.scconst,FOREVER);
	int sper,spmult;
	float smper;BOOL first=(tvalid<t0);
	while ((tvalid < t)&&(tvalid<=dis))
	{	int born = 0;
	    count=0;
		if (first) 
			tvalid=t0;
		// Compute our step size
		if (tvalid%stepSize !=0) 
		{	dt = stepSize - abs(tvalid)%stepSize;
		} 
		else 
		{	dt = stepSize;
		}
		if (tvalid + dt > t) 
		{	dt = t-tvalid;
		}

  		// Increment time
		if (!first) 
			tvalid += dt;
		if (tvalid>dis)
		{	for (j=0; j<parts.Count(); j++)
			{	parts.ages[j] = -1;  
			}
			tvalid=t;
			continue;
		}
		// Compute the number of particles that should be born!
		birth=0;
		fullframe=(tvalid%tpf==0);
		if (fullframe)
		{	if (bmethod)
			{	int tdelta;
				if (tvalid>=t2) 
					birth=total-counter;
				else
				{	tdelta=int((float)total*(tvalid-t0+oneframe)/createover);
					birth=tdelta-counter;
				}
			}
			else if (tvalid<=t2)
			{	pblock->GetValue(PB_PBIRTHRATE,tvalid,total,FOREVER);
				if (!isrend) 
					total = (int)(dpercent*(float)total+FTOIEPS);
				birth=total;
				if (birth+counter>maincount) 
					birth=0;
			}
		}
	    pblock->GetValue(PB_SSSPAWNDIEAFTER,tvalid,ptvel.persist,FOREVER);
	    pblock->GetValue(PB_SSSPAWNDIEAFTERVAR,tvalid,ptvel.pvar,FOREVER);
	    pblock->GetValue(PB_SPEED,tvalid,ptvel.Speed,FOREVER);
    	pblock->GetValue(PB_SPEEDVAR,tvalid,ptvel.VSpeed,FOREVER);
		pblock->GetValue(PB_SIZE,tvalid,ptvel.Size,FOREVER);
		pblock->GetValue(PB_OFFAXIS,tvalid,ptvel.Av180,FOREVER);
		pblock->GetValue(PB_AXISSPREAD,tvalid,ptvel.VAv,FOREVER);
		pblock->GetValue(PB_OFFPLANE,tvalid,ptvel.Ah180,FOREVER);
		pblock->GetValue(PB_PLANESPREAD,tvalid,ptvel.VAh,FOREVER);
		pblock->GetValue(PB_LIFE,tvalid,ptvel.Life,FOREVER);
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
	    pblock->GetValue(PB_SPAWNDIRCHAOS,tvalid,spawnvars.dirchaos,FOREVER);
	    pblock->GetValue(PB_SPAWNSPEEDCHAOS,tvalid,spawnvars.spchaos,FOREVER);
		spawnvars.spchaos/=100.0f;
	    pblock->GetValue(PB_SPAWNSCALECHAOS,tvalid,spawnvars.scchaos,FOREVER);
		spawnvars.scchaos/=100.0f;
	    pblock->GetValue(PB_SPAWNPERCENT,tvalid,sper,FOREVER);	
		pblock->GetValue(PB_SPAWNMULTVAR,tvalid,smper,FOREVER);	
		spawnbvar=smper*spmultb;
  		pblock->GetValue(PB_SPINAXISX,tvalid,ptvel.Axis.x,FOREVER);
		pblock->GetValue(PB_SPINAXISY,tvalid,ptvel.Axis.y,FOREVER);
		pblock->GetValue(PB_SPINAXISZ,tvalid,ptvel.Axis.z,FOREVER);
		if (Length(ptvel.Axis)==0.0f) 
			ptvel.Axis.x=0.001f;
		pblock->GetValue(PB_SPINAXISVAR,tvalid,ptvel.axisvar,FOREVER);
		spawnvars.Axis=ptvel.Axis;
		spawnvars.axisvar=ptvel.axisvar;
		if (llist.Count()==0) 
			deftime=ptvel.Life/oneframe;
		basesize=M*ptvel.Size;
		// First increment age and kill off old particles
		for (j=0;j<parts.Count();j++)
		{	if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
			{	valid=FALSE;
				tvalid=t;
				cancelled=TRUE;
				parts.FreeAll();
				return;
			}
			if (!parts.Alive(j)) 
				continue;
			parts.ages[j] += dt;
			if (parts.ages[j] >= sdata[j].L) 
			{	if ((stype!=ONDEATH)||(sdata[j].gennum>=maxgens)||(sdata[j].SpVar>=sper)) 
					parts.ages[j] = -1;	
				else if (fullframe)
				{	spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
					else 
						parts.ages[j] = -1;
				}
			} 
			else if (sdata[j].DL>-1) 
			{	sdata[j].DL+=dt;
				if (sdata[j].DL>sdata[j].persist) 
					parts.ages[j]=-1;	
			}
			if (parts.ages[j]>-1)
			{	if (fullframe && ((stype==EMIT)&&(sdata[j].gennum==0)&&(sdata[j].SpVar<sper)))
				{	spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,TRUE);
				}
//			if (pkind==RENDTYPE6)
//		    parts.radius[j]=ptvel.Size; else 
				if ((stype<2)||(maxgens==0))
		 			parts.radius[j]=FigureOutSize(parts.ages[j],ptvel.Size,grow,fade,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
				else 
				{	if (sdata[j].gennum==0)
						parts.radius[j]=FigureOutSize(parts.ages[j],ptvel.Size,grow,0,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
					else if (sdata[j].gennum==maxgens)
						parts.radius[j]=FigureOutSize(parts.ages[j],sdata[j].Vsz,0,fade,sdata[j].L,grate,frate);
				}
			}
		}
		// Apply forces to modify velocity
//		if (fullframe)
		if (birth>0)
		{	Matrix3 tm,tmold;	
			float stepCoef = 1.0f;
			TimeValue checkone = tvalid - t0;

			if (checkone < 0) 
				checkone = -checkone;						// ***** I just added this line

			if (checkone == 0)
				tmold = node->GetObjTMBeforeWSM(tvalid - stepSize);
			else
				tmold = node->GetObjTMBeforeWSM(checkone<stepSize?t0:(tvalid - stepSize));

			tm = node->GetObjTMBeforeWSM(tvalid);
			ptvel.bps.Init(node, tvalid, stepSize);

		// Next, birth particles at the birth rate
			for (j=counter; j<maincount; j++) 
			{	if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
				{	valid=FALSE;
					tvalid=t;cancelled=TRUE;
					parts.FreeAll();
					return;
				}
				if (born>=birth) 
					break;

				BirthParticle(node,tvalid,j,&ptvel,tmold);
//				if ((pkind==RENDTYPE5)||(pkind==RENDTYPE6))
//					parts.radius[j]=ptvel.Size;	
//				else 
				parts.radius[j]=(grow>0?basesize:ptvel.Size)*(1.0f + sdata[j].Vsz);
				sdata[j].themtl = int((tvalid-t0)*FperT);
				sdata[j].showframe = (anioff==1?0:sdata[j].showframe);
				born++;
				counter++;
			}
		}
		int fc=fields.Count();

		if (fc>0)
			for (j=0; j<parts.Count(); j++) 
			{	Point3 force,tvel=Zero;
				for (i=0; i<fc; i++) 
				{	if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
					{	valid=FALSE;
						tvalid=t;
						cancelled=TRUE;
						parts.FreeAll();
						return;
					}
					if (!parts.Alive(j)) 
						continue;
					parts[j]=sdata[j].V;
					force = fields[i]->Force(tvalid,parts[j],parts.vels[j],j);
					float curdt = (float)dt;
					if ((parts.ages[j]==0)&&(sdata[j].gennum==0)) 
						curdt=tpf*sdata[j].Fo;
					tvel += 10.0f*force * curdt;
				}
				parts.vels[j]+=tvel;
			}

// IPC IPC IPC IPC ss

		int IPC,ipcsteps;float B,Vb;
  		pblock->GetValue(PB_IPCOLLIDE_ON,tvalid,IPC,FOREVER);
  		pblock->GetValue(PB_IPCOLLIDE_STEPS,tvalid,ipcsteps,FOREVER);
  		pblock->GetValue(PB_IPCOLLIDE_BOUNCE,tvalid,B,FOREVER);
  		pblock->GetValue(PB_IPCOLLIDE_BOUNCEVAR,tvalid,Vb,FOREVER);
		if (IPC)
		{	CollideParticle cp;
			int ddt=dt/ipcsteps,remtime=0,snum=0;
			TimeValue curt=tvalid;
			if (dt > 0)
			{	while (snum < ipcsteps)
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
					{ if ((j!=md.min)&&(j!=md.min2)) 
					  { if (fullframe) 
						  MovePart(j,md.mintime,fullframe,tpf);
						else 
						  parts[j]+=parts.vels[j]*(float)md.mintime;
					  }
					  else if (fullframe) sdata[j].V=parts[j];
					}
				  }
				}
			}
		}
		else
			for (j=0; j<parts.Count(); j++)
			{	if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) ) 
				{	valid=FALSE;
					tvalid=t;
					cancelled=TRUE;
					parts.FreeAll();
					return;
				}
				if ((!parts.Alive(j))||(parts.ages[j]==0)) 
					continue;
				count++;

			// Check for collisions
				BOOL collide = FALSE;
			//	float meaninglesstime;
				float collisionTime, remTime = (float)dt;
				BOOL maybeStuck = FALSE;
//				for (int i=0; i<cobjs.Count(); i++)
				for (int i=0; i<cc.MAX_COLLISIONS_PER_STEP; i++)
				{	
//					if (cobjs[i]->CheckCollision(tvalid,parts[j],parts.vels[j], (float)dt, j,&meaninglesstime,TRUE)) 
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
					{	if (sdata[j].persist==0) 
						{	parts.ages[j] = -1;
							count--;
						}
						else 
							sdata[j].DL=0;
					}
					else if (fullframe &&((stype==COLLIDE)&&(sdata[j].gennum<maxgens)&&(sdata[j].SpVar<sper)))
					{	int spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
						if (spmult!=0) 
							DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
					}
				}
			// If we didn't collide, then increment.
				if (!collide) 
					MovePart(j,dt,fullframe,tpf);
				else if (fullframe) 
					sdata[j].V=parts[j];
			}

// end of code that will change to support IPC

		if (first) 
			first=FALSE;
		for (j=0; j<parts.Count(); j++)
		{	sdata[j].Ts = (float)ptvel.Spin*sdata[j].Ts0;
			sdata[j].LamTs += (FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);
		}
	}	

/*	if ((frag==METABALLS)&&((!isrend)&&(onscreen==2)))
	{	float res,bstr,thres=0.6f;
		pblock->GetValue(PB_METATENSION,t,bstr,FOREVER);
		pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
		if (cmesh) delete[] cmesh;cmesh=new Mesh;
		metap.CreateMetas(parts,cmesh,thres,res,bstr);
	}
	else*/
	if ((frag==INSTGEOM)&&(onscreen>1)&&(custnode))
	{	TimeValue anist=GetAnimStart(),aniend=GetAnimEnd();
		theSuprSprayDraw.anifr=aniend+stepSize;
		theSuprSprayDraw.t=t;
		theSuprSprayDraw.anioff=anioff;
		if (count>0)
			GetTimes(times,t,theSuprSprayDraw.anifr,anioff);
		else 
			times.tl.ZeroCount();
		if (onscreen==2)
			GetMesh(t,subtree,custmtl);
		else 
			GetallBB(custnode,subtree,t);
	}  
	if (isrend) 
		rcounter=counter;
	else 
		vcounter=counter;
	if (tvalid<t) 
		tvalid=t;
	valid=TRUE;
//	ssert(tvalid==t);
}


void SuprSprayParticle::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	if (pmapPGen) pmapPGen->Invalidate();
	if (pmapPType) pmapPType->Invalidate();
	if (pmapPSpin) pmapPSpin->Invalidate();
	if (pmapEmitV) pmapEmitV->Invalidate();
	if (pmapBubl) pmapBubl->Invalidate();
	if (pmapSpawn) pmapSpawn->Invalidate();
	}

BOOL CommonParticle::EmitterVisible()
	{
	int hide;
	pblock->GetValue(PB_EMITRHID,0,hide,FOREVER);
	return !hide;
	}

ParamDimension *SuprSprayParticle::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SPEED:
		case PB_SIZE:
		case PB_METATENSION:
		case PB_METACOURSE:
		case PB_MAPPINGDIST:
		case PB_SPINAXISX:
		case PB_SPINAXISY:
		case PB_SPINAXISZ:
		case PB_EMITVMULT:
		case PB_BUBLAMP:
		case PB_EMITRWID:  			return stdWorldDim;

		case PB_RNDSEED:
		case PB_PBIRTHRATE:
		case PB_PTOTALNUMBER:
									return defaultDim;

		case PB_OFFAXIS:
		case PB_AXISSPREAD:
		case PB_OFFPLANE:
		case PB_PLANESPREAD:
		case PB_SPINPHASE:
		case PB_SPINAXISVAR:
		case PB_BUBLPHAS:			return stdAngleDim;

		case PB_DISPLAYPORTION:
		case PB_SIZEVAR:
		case PB_SPEEDVAR:
		case PB_SPAWNDIRCHAOS:
		case PB_METATENSIONVAR:
		case PB_SPINTIMEVAR:
		case PB_SPINPHASEVAR:
		case PB_EMITVMULTVAR:
		case PB_BUBLAMPVAR:
		case PB_BUBLPERVAR:
		case PB_BUBLPHASVAR:
		case PB_SPAWNMULTVAR:
		case PB_SSSPAWNDIEAFTERVAR:			
		case PB_IPCOLLIDE_BOUNCE:			
		case PB_IPCOLLIDE_BOUNCEVAR:			
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
		case PB_SSSPAWNDIEAFTER:
								return stdTimeDim;
		
		default: return defaultDim;
		}
	}

TSTR SuprSprayParticle::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_OFFAXIS:			return GetString(IDS_AP_OFFAXIS);
		case PB_AXISSPREAD:			return GetString(IDS_AP_AXISSPREAD);
		case PB_OFFPLANE:			return GetString(IDS_AP_OFFPLANE);
		case PB_PLANESPREAD:		return GetString(IDS_AP_PLANESPREAD);
		case PB_SPEED:				return GetString(IDS_RB_SPEED);
		case PB_SPEEDVAR:			return GetString(IDS_RB_SPEEDVAR);
		case PB_PBIRTHRATE:			return GetString(IDS_RB_PBIRTHRATE);
		case PB_PTOTALNUMBER:		return GetString(IDS_RB_PTOTALNUMBER);
		case PB_DISPLAYPORTION:		return GetString(IDS_RB_DISPLAYPORTION);
		case PB_EMITSTART:			return GetString(IDS_RB_EMITSTART);
		case PB_EMITSTOP:			return GetString(IDS_RB_EMITSTOP);
		case PB_DISPUNTIL:			return GetString(IDS_RB_DISPUNTIL);
		case PB_LIFE:				return GetString(IDS_RB_LIFE);
		case PB_LIFEVAR:			return GetString(IDS_RB_LIFEVAR);
		case PB_SIZE:				return GetString(IDS_RB_SIZE);
		case PB_SIZEVAR:			return GetString(IDS_RB_SIZEVAR);
		case PB_GROWTIME:			return GetString(IDS_RB_GROWTIME);
		case PB_FADETIME:			return GetString(IDS_RB_FADETIME);
		case PB_RNDSEED:			return GetString(IDS_RB_RNDSEED);
		case PB_METATENSION:		return GetString(IDS_RB_METATENSION);
		case PB_METATENSIONVAR:		return GetString(IDS_RB_METATENSIONVAR);
		case PB_METACOURSE:			return GetString(IDS_RB_METACOURSE);
		case PB_MAPPINGTIME:		return GetString(IDS_RB_MAPPINGTIME);
		case PB_MAPPINGDIST:		return GetString(IDS_RB_MAPPINGDIST);
		case PB_SPINTIME:			return GetString(IDS_RB_SPINTIME);
		case PB_SPINTIMEVAR:		return GetString(IDS_RB_SPINTIMEVAR);
		case PB_SPINPHASE:			return GetString(IDS_RB_SPINPHASE);
		case PB_SPINPHASEVAR:		return GetString(IDS_RB_SPINPHASEVAR);
		case PB_SPINAXISX:			return GetString(IDS_RB_SPINAXISX);
		case PB_SPINAXISY:			return GetString(IDS_RB_SPINAXISY);
		case PB_SPINAXISZ:			return GetString(IDS_RB_SPINAXISZ);
		case PB_SPINAXISVAR:		return GetString(IDS_RB_SPINAXISVAR);
		case PB_EMITVINFL:			return GetString(IDS_RB_EMITVINFL);
		case PB_EMITVMULT:			return GetString(IDS_RB_EMITVMULT);
		case PB_EMITVMULTVAR:		return GetString(IDS_RB_EMITVMULTVAR);
		case PB_BUBLAMP:			return GetString(IDS_RB_BUBLAMP);
		case PB_BUBLAMPVAR:			return GetString(IDS_RB_BUBLAMPVAR);
		case PB_BUBLPER:			return GetString(IDS_RB_BUBLPER);
		case PB_BUBLPERVAR:			return GetString(IDS_RB_BUBLPERVAR);
		case PB_BUBLPHAS:			return GetString(IDS_RB_BUBLPHAS);
		case PB_BUBLPHASVAR:		return GetString(IDS_RB_BUBLPHASVAR);
		case PB_EMITRWID:			return GetString(IDS_RB_EMITRWID);
		case PB_STRETCH:			return GetString(IDS_AP_STRETCH);
		case PB_OFFSETAMOUNT:		return GetString(IDS_AP_OFFSETAMT);
		case PB_SPAWNDIRCHAOS:		return GetString(IDS_AP_SPAWNDIRCHAOS);
		case PB_SPAWNSPEEDCHAOS:		return GetString(IDS_AP_SPAWNSPEEDCHAOS);
		case PB_SPAWNSCALECHAOS:		return GetString(IDS_AP_SPAWNSCALECHAOS);
		case PB_SPAWNMULTVAR:			return GetString(IDS_EP_SPAWNMULTVAR);
		case PB_SPAWNPERCENT:			return GetString(IDS_EP_SPAWNAFFECTS);
		case PB_SSSPAWNDIEAFTER:		return GetString(IDS_AP_SPAWNDIEAFTER);
		case PB_SSSPAWNDIEAFTERVAR:		return GetString(IDS_AP_SPAWNDIEAFTERVAR);
		case PB_IPCOLLIDE_ON:			return GetString(IDS_AP_IPCOLLIDE_ON);
		case PB_IPCOLLIDE_STEPS:		return GetString(IDS_AP_IPCOLLIDE_STEPS);
		case PB_IPCOLLIDE_BOUNCE:		return GetString(IDS_AP_IPCOLLIDE_BOUNCE);
		case PB_IPCOLLIDE_BOUNCEVAR:	return GetString(IDS_AP_IPCOLLIDE_BOUNCEVAR);
		default: 						return TSTR(_T(""));
		}
	}	


void CommonParticle::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{
	// particles may require update (bayboro|march.25.2002)
	if (!OKtoDisplay(t)) return;
	if (t!=tvalid) cancelled=FALSE;
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (doupdate) Update(t,inode);
	// end of particles may require update (bayboro|march.25.2002)

	cnode=inode;
	int type,ptype,disptype;
	pblock->GetValue(PB_VIEWPORTSHOWS,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,disptype,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,ptype,FOREVER);
//	if ((type>2)&&((ptype==RENDTYPE5)||(ptype==RENDTYPE6)||((disptype>0)&&(!custnode))  )) type=1;
	if ((type>2)&&(((disptype>0)&&(!custnode))  )) type=1;
	if ((type==3)&&((disptype!=INSTGEOM)||(!custnode))) type=1;
	if (type==2) 
	{ Box3 pbox;
		Matrix3 mat = inode->GetObjTMBeforeWSM(t);
		UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {continue;	}
			Point3 pt = parts.points[i];
			float r=parts.radius[i]*SQR2,strlen=1.0f;
			int axisentered,K;
			pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
			 if ((ptype!=RENDTYPE5)&&(ptype!=RENDTYPE6))
			 {if ((ClassID()!=BLIZZARD_CLASS_ID)&&(axisentered==DIRTRAVEL))
			{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			  strlen=GetLen(parts.vels[i],K);
			  r*=strlen;
			 }}
			if ((disptype==INSTGEOM)&&(custnode))
			{ int tlst=times.tl.Count();if (tlst==0) tlst=1;
			  pbox += pt;
			  for (int level=0;level<tlst;level++)
			  {	Box3 cbb=cmesh[level].getBoundingBox();
			    Point3 dist=cbb.pmax-cbb.pmin;
				float pdist=Largest(dist)*r;
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
		Matrix3 mat = inode->GetObjTMBeforeWSM(t);
		UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) 
		{	if (!parts.Alive(i)) {continue;	}
			Point3 pt;
			float radius=parts.radius[i]*SQR2;
			int axisentered,K;
			pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
			 if ((ptype!=RENDTYPE5)&&(ptype!=RENDTYPE6))
			 {if ((ClassID()!=BLIZZARD_CLASS_ID)&&(axisentered==DIRTRAVEL))
			{ pblock->GetValue(PB_STRETCH,t,K,FOREVER);
			  float strlen=GetLen(parts.vels[i],K);
			  radius*=strlen;
			 }}
			int n=0,found=0;
			TimeValue Ctime=(theSuprSprayDraw.anioff?GetCurTime(sdata[i].showframe,(theSuprSprayDraw.anioff>1?theSuprSprayDraw.t:parts.ages[i]),theSuprSprayDraw.anifr):theSuprSprayDraw.t);
			found=((n=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum)))>-1);
			for (int nb=0;nb<theSuprSprayDraw.bboxpt[n].numboxes;nb++)
			{ pt=theSuprSprayDraw.bboxpt[n].bpts[nb].Suboffset+parts[i];
			  float pdist=Largest(theSuprSprayDraw.bboxpt[n].bpts[nb].pts[0]-theSuprSprayDraw.bboxpt[n].bpts[nb].pts[6])*radius;
			  pbox += pt + Point3( pdist, 0.0f, 0.0f);
			  pbox += pt + Point3(-pdist, 0.0f, 0.0f);
			  pbox += pt + Point3( 0.0f, pdist, 0.0f);
			  pbox += pt + Point3( 0.0f,-pdist, 0.0f);
			  pbox += pt + Point3( 0.0f, 0.0f, pdist);
			  pbox += pt + Point3( 0.0f, 0.0f,-pdist);
			}
		  }
		if (!pbox.IsEmpty()) box += pbox;
	  }
	else {SimpleParticle::GetWorldBoundBox(t,inode,vpt,box);}
	}



int GetDrawType(CommonParticle *po, int &ptype,int &disptype)
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
      po->theSuprSprayDraw.anifr=aniend+oneframe;
	  if (po->ClassID()==BLIZZARD_CLASS_ID) po->theSuprSprayDraw.indir.inaxis=0;
	  else
	  { int axisentered;
		po->pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	    if (po->theSuprSprayDraw.indir.inaxis=(axisentered==DIRTRAVEL))
	    { po->pblock->GetValue(PB_STRETCH,po->theSuprSprayDraw.t,po->theSuprSprayDraw.indir.K,FOREVER);
		  po->theSuprSprayDraw.indir.oneframe=oneframe;
	    }
	  }
	}
	return type;
}

/*BOOL CommonParticleDraw::DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i)
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
  {int n=0,found=0;
    TimeValue Ctime=(anioff?GetCurTime(obj->sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	found=((n=TimeFound(obj->times,Ctime,(obj->sdata[i].gennum>obj->nCount?obj->nCount:obj->sdata[i].gennum)))>-1);
	float radius=parts.radius[i];
	float elapsedtime=(float)parts.ages[i];
    float Angle=(FloatEQ0(obj->sdata[i].Ts)?0.0f:elapsedtime/obj->sdata[i].Ts)+obj->sdata[i].LamTs;
	if (indir.inaxis)
	  indir.vel=parts.vels[i];
	Point3 nextpt;
	if (found) 
	{ BOOL dotumble=((obj->ClassID()==BLIZZARD_CLASS_ID)&&(tumble>0.0f));
	  Matrix3 mat; if (dotumble) mat= obj->TumbleMat(i,tumble,scale);
	if (bb)
    { Point3 pt[9];
	  for (int nb=0;nb<bboxpt[n].numboxes;nb++)
	  { for (int j=0;j<8;j++)
	    {if (indir.inaxis)
	    pt[j]=RotateAboutAxis(Angle,parts.points[i],radius*(bboxpt[n].bpts[nb].pts[j]+bboxpt[n].bpts[nb].Suboffset),obj->sdata[i].W,indir);
	    else
		{ nextpt=(radius*(bboxpt[n].bpts[nb].pts[j]+bboxpt[n].bpts[nb].Suboffset))+parts[i];
		  RotateOnePoint(&nextpt.x,&parts.points[i].x,&(obj->sdata[i].W.x),Angle);
		  if (dotumble) pt[j]=parts.points[i]+(nextpt-parts.points[i])*mat;
		  else pt[j]=nextpt;
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
		    if (dotumble) pt[pnum]=parts.points[i]+(nextpt-parts.points[i])*mat;
			else pt[pnum]=nextpt;
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
	if (nfaces>LOTSOFACES)
	{  hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	   chcur=TRUE;
	}
	if ((obj->ClassID()==BLIZZARD_CLASS_ID)&&(tumble>0.0f))
	{ Matrix3 mat = obj->TumbleMat(i,tumble,scale);
	  for (int j=0;j<numV;j++) 
		  pm->verts[j]=obj->parts.points[i]+(pm->verts[j]-obj->parts.points[i])*mat;
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
}	*/

BOOL CommonParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
{ HCURSOR hCur;
  BOOL chcur=FALSE;
  if (!((disptype==INSTGEOM)&&(bb))) return TRUE;
  Point3 pt[4];  
  if (obj->custnode)
  { int n=0,found=0;
    TimeValue Ctime=(anioff?GetCurTime(obj->sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
	found=((n=TimeFound(obj->times,Ctime,(obj->sdata[i].gennum>obj->nlist.Count()?obj->nlist.Count():obj->sdata[i].gennum)))>-1);
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

MarkerType CommonParticle::GetMarkerType() 
	{int ptype,disptype,type=GetDrawType(this,ptype,disptype);
	  switch (type) {
		case 0:
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;			
		case 1:
			parts.SetCustomDraw(NULL);
			return PLUS_SIGN_MRKR;
		case 2:	{			
			theSuprSprayDraw.obj = this;
			theSuprSprayDraw.firstpart=TRUE;
			theSuprSprayDraw.disptype=disptype;
			theSuprSprayDraw.ptype=ptype;
			parts.SetCustomDraw(&theSuprSprayDraw);			
			theSuprSprayDraw.bb=FALSE;
			return POINT_MRKR;
			}	 
		case 3:{theSuprSprayDraw.obj = this;
			theSuprSprayDraw.firstpart=TRUE;
			theSuprSprayDraw.disptype=disptype;
			theSuprSprayDraw.ptype=ptype;
			theSuprSprayDraw.bb=TRUE;
			parts.SetCustomDraw(&theSuprSprayDraw);			
			return POINT_MRKR;
		   }
		default:
			return PLUS_SIGN_MRKR;
		}
	}


void CacheSpin(float *holddata,CSavePt *sdata,int pcount,BOOL issave)
{ for (int i=0;i<pcount;i++)
	if (issave) holddata[i]=sdata[i].LamTs;
	else sdata[i].LamTs=holddata[i];
}

//--- SuprSpray particle -----------------------------------------------

RefTargetHandle SuprSprayParticle::Clone(RemapDir& remap) 
	{
	SuprSprayParticle* newob = new SuprSprayParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
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
	 newob->GetTimes(newob->times,t,anifr,anioff);
	 if (vshow==2)
	 { pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	   newob->GetMesh(t,subtree,custmtl);
	 }
	 else newob->GetallBB(custnode,subtree,t);
	}
	BaseClone(this, newob, remap);
	return newob;
	}

static float findmappos(float curpos)
{ float mappos;

  return(mappos=((mappos=curpos)<0?0:(mappos>1?1:mappos)));
}

Mesh *SuprSprayParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
{	float Mval, Uval, Vval,Wval,Angle,elapsedtime;
	int type, count,maptype,anifr;
	float width;
	Point3 Center;
	BOOL mirror=FALSE;
	TVFace defface;
	Tab<int> tVertsOffset;
	MultipleChannelMapMerger mcmm;
	Mesh *pm = new Mesh;
	if (cancelled) 
	{ZeroMesh(pm);mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;}
	TimeValue mt,aniend=GetAnimEnd();
	dispt=t;
    anifr=aniend+GetTicksPerFrame();
	int nummtls=0,curmtl=0,multi=0,pc=0,custmtl=0;
	pblock->GetValue(PB_PARTICLETYPE,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,pc,FOREVER);
	if (pc==METABALLS) type=RENDMETA;
	else if (pc==INSTGEOM)
	{if (custnode==NULL) {type=0;pc=ISSTD;} else type=RENDGEOM;
	pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
	}
	float FperT=GetFrameRate()/(float)TIME_TICKSPERSEC;
	pblock->GetValue(PB_EMITRWID,t,width,FOREVER);
	pblock->GetValue(PB_MAPPINGTYPE,t,maptype,FOREVER);
	if (maptype)
	 pblock->GetValue(PB_MAPPINGDIST,t,Mval,FOREVER);
	else 
	 pblock->GetValue(PB_MAPPINGTIME,0,mt,FOREVER);
	int isrend=!TestAFlag(A_NOTREND);

//my comment out below - ecp
//	if ((!isrend)&&((type==RENDTYPE5)||(type==RENDTYPE6))) 
//		type=RENDTYPE1;

	Matrix3 wcam, cam = ident;
	Point3 v, v0, v1, otherV = Zero, camV = Zero;

	if (isrend)
	{ 
		cam = Inverse(wcam = view.worldToView);
		otherV = cam.GetRow(2), camV = cam.GetRow(3);
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

	Matrix3 tm = inode->GetObjTMAfterWSM(t);
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
	Center = Zero* tm;
	float Thetah;
	Point3 r1;
	if (count==0)
	{ ZeroMesh(pm);	goto quit;}
	else
	{mirror=DotProd(tm.GetRow(0)^tm.GetRow(1),tm.GetRow(2))<0.0f;
	if (type==RENDTYPE6)
	{ 
		if (view.projType) 
			type = RENDTYPE5;
	  	else
		{ 
			Thetah = view.fov;
			r1 = cam.GetRow(1);
		}
	}

/*	{ if (cview==NULL) type=RENDTYPE5;
	   else
	  { Thetah=(cview!=NULL?((CameraObject *)cview->GetObjectRef())->GetFOV(t):0);
	    r1=cam.GetRow(1);
	  }
	}*/
	int gtvnum=0,anioff=0;
	if (pc<METABALLS) GetMeshInfo(type,count,pm,&numF,&numV);
	else if (type==RENDGEOM)
	{ int subtree,onscreen;
	  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
//	  if (onscreen!=2) { 
	  theSuprSprayDraw.t=t;
	  GetTimes(times,t,anifr,anioff);
	  GetMesh(t,subtree,custmtl);
//	  }
	  int custmtl,frag;
      pblock->GetValue(PB_CUSTOMMTL,0,custmtl,FOREVER);
      pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	  if ((custnode)&&(frag==INSTGEOM)&& custmtl) 
		GetSubs(inode,custnode,subtree,t);
	  TimeValue Ctime;
	  int mnum,tmptvs=0;
	  numV=0;numF=0;
	  BOOL alltex=TRUE;
	  for (int pcnt=0;pcnt<parts.Count();pcnt++)
	  { if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(pcnt)) continue;
		Ctime=(anioff?GetCurTime(sdata[pcnt].showframe,(anioff>1?t:parts.ages[pcnt]),anifr):t);
		mnum=TimeFound(times,Ctime,(sdata[pcnt].gennum>nlist.Count()?nlist.Count():sdata[pcnt].gennum));
		numV+=cmesh[mnum].getNumVerts();
		numF+=cmesh[mnum].getNumFaces();
	  }
	  pm->setNumFaces(numF);
	  pm->setNumVerts(numV);
	  int mcnt=0;
	  int numInstGeomWithMapping=0;
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
      pblock->GetValue(PB_SSNOTDRAFT,t,notdraft,FOREVER);
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
	InDirInfo indir; indir.oneframe=GetTicksPerFrame();
	int axisentered;indir.vel=Zero;indir.K=0;
    pblock->GetValue(PB_SPINAXISTYPE,0,axisentered,FOREVER);
	if (indir.inaxis=(axisentered==DIRTRAVEL))
	{  pblock->GetValue(PB_STRETCH,t,indir.K,FOREVER);
	}
  MtlID cm;
    for (i=0; i<parts.Count(); i++) 
	{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
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
		{ 	v  = (view.projType?otherV:Normalize(camV-parts[i]));
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
		   int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum));
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
			{cm=cmesh[mnum].faces[j].getMatID();
			 int mtlgen=times.tl[mnum].gennum-1,maxmtl=nmtls.Count();
			 if (mtlgen>=maxmtl) mtlgen=maxmtl-1;
			 if ((mtlgen>-1)&&((times.tl.Count()>0)&&(times.tl[mnum].gennum>0)))
				cm+=nmtls[mtlgen];
			 pm->faces[jf].setMatID(cm);
			}
			pm->faces[jf].setEdgeVisFlags(cface.getEdgeVis(0),cface.getEdgeVis(1),cface.getEdgeVis(2));
			jf++;
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
  if (mirror) SwitchVerts(pm);
quit:mesh.InvalidateGeomCache();
  if ((isrend)&&(midframe) )
  { if (pcount>0)
	{CacheData(&lastparts,&parts);
    CacheSpin(holddata,sdata,pcount,FALSE);
	delete[] holddata;}
    tvalid=t-offtime;
  }
	needDelete = TRUE;
	return pm;
}

const TCHAR *SuprSprayClassDesc::ClassName ()	{return GetString(IDS_AP_SUPRSPRAY);}
const TCHAR *SuprSprayClassDesc::Category ()	{return GetString(IDS_RB_PARTICLESYSTEMS);}
TCHAR *SuprSprayParticle::GetObjectName() {return GetString(IDS_AP_SUPRSPRAYGC);}

class BEmitterCreateCallback : public CreateMouseCallBack {
	public:
		BlizzardParticle *blizzard;
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int BEmitterCreateCallback::proc(
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
		switch(point)  {
			case 0:
				// if hidden by category, re-display particles and objects
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				blizzard->pblock->SetValue(PB_EMITRWID,0,0.01f);
				blizzard->pblock->SetValue(PB_EMITRLENGTH,0,0.01f);
				blizzard->pmapParam->Invalidate();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
				blizzard->pblock->SetValue(PB_EMITRWID,0,
					(float)fabs(p1.x-p0.x));
				blizzard->pblock->SetValue(PB_EMITRLENGTH,0,
					(float)fabs(p1.y-p0.y));
				blizzard->pmapParam->Invalidate();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 || Length(p1-p0)<0.1f) {						
						return CREATE_ABORT;
					} else { CheckPickButtons(blizzard->pblock,blizzard->hptype,blizzard->spawn,blizzard->pickCB.repi);
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
	return 1;
	}

static BEmitterCreateCallback BemitterCallback;

CreateMouseCallBack* BlizzardParticle::GetCreateMouseCallBack() 
	{
	BemitterCallback.blizzard = this;
	return &BemitterCallback;
	}
BlizzardParticle::BlizzardParticle()
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
     int tpf=GetTicksPerFrame();

	MakeRefByID(FOREVER, PBLK, CreateParameterBlock(BdescVer6, PBLOCK_LENGTH_BLIZZARD, CURRENT_VERSION));
	pblock->SetValue(PB_SPEED,0,10.0f);
	pblock->SetValue(PB_SPEEDVAR,0,0.0f);
	pblock->SetValue(PB_PBIRTHRATE,0,10);
	pblock->SetValue(PB_PTOTALNUMBER,0,100);
	pblock->SetValue(PB_BIRTHMETHOD,0,0);
	pblock->SetValue(PB_DISPLAYPORTION,0,0.1f);
	pblock->SetValue(PB_EMITSTART,0,TimeValue(0));
	pblock->SetValue(PB_EMITSTOP,0,(TimeValue)30*FToTick);// correct constant?
	pblock->SetValue(PB_DISPUNTIL,0,100*FToTick);// correct constant?
	pblock->SetValue(PB_LIFE,0,30*FToTick);// correct constant?
	pblock->SetValue(PB_LIFEVAR,0,0.0f);
	pblock->SetValue(PB_SUBFRAMEMOVE,0,1);
	pblock->SetValue(PB_SUBFRAMETIME,0,1);
	pblock->SetValue(PB_SUBFRAMEROT2,0,0);
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
	pblock->SetValue(PB_VIEWPORTSHOWS,0,1);
	pblock->SetValue(PB_MAPPINGTYPE,0,0);
	pblock->SetValue(PB_MAPPINGTIME,0,30*FToTick);
	pblock->SetValue(PB_MAPPINGDIST,0,100.0f);

	pblock->SetValue(PB_SPINTIME,0,30*FToTick);
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

	pblock->SetValue(PB_EMITRWID,0,0.0f);
	pblock->SetValue(PB_EMITRLENGTH,0,0.0f);

	pblock->SetValue(PB_EMITRHID,0,0);
	pblock->SetValue(PB_SPAWNGENS,0,1);
	pblock->SetValue(PB_SPAWNCOUNT,0,1);
	pblock->SetValue(PB_METACOURSEVB,0,1.0f);
	pblock->SetValue(PB_SPAWNPERCENT2,0,100);
	pblock->SetValue(PB_SPAWNMULTVAR2,0,0);

	pblock->SetValue(PB_BLNOTDRAFT,0,0);
	pblock->SetValue(PB_BLSPAWNDIEAFTER,0,0);

	pblock->SetValue(PB_BLIPCOLLIDE_ON,0,0);
	pblock->SetValue(PB_BLIPCOLLIDE_STEPS,0,2);
	pblock->SetValue(PB_BLIPCOLLIDE_BOUNCE,0,1.0f);
	pblock->SetValue(PB_BLIPCOLLIDE_BOUNCEVAR,0,0.0f);
	
	sdata=NULL;
	cnode=NULL;
	custnode=NULL;
	custname=TSTR(_T(" "));
	ResetSystem(0,FALSE);
	int plength=PBLOCK_LENGTH_BLIZZARD;
	size=0;
	for (int i=0;i<plength;i++)
	  size+=(BdescVer6[i].type==TYPE_INT?isize:fsize); 
//	size=40*isize+fsize*32;
	cmesh=NULL;
	dispmesh=NULL;
	dispt=-99999;
	theSuprSprayDraw.bboxpt=NULL;
	nmtls.ZeroCount();
	parts.points.ZeroCount();
	nlist.ZeroCount();
	llist.ZeroCount();
	maincount=0;
	deftime=0;
	cancelled=FALSE;
	dflags=APRTS_ROLLUP_FLAGS;
	backpatch=TRUE;
	origmtl=NULL;
	ClearAFlag(A_NOTREND);
    stepSize=GetTicksPerFrame();
}

BlizzardParticle::~BlizzardParticle()
{
	if (sdata) {delete[] sdata;sdata=NULL;}
	DeleteAllRefsFromMe();
	pblock=NULL;
	parts.FreeAll();
	times.tl.SetCount(0);
	times.tl.Shrink();
	nmtls.ZeroCount();nmtls.Shrink();
	llist.ZeroCount();llist.Shrink();
	nlist.ZeroCount();nlist.Shrink();
	if (cmesh) delete[] cmesh;
	if (theSuprSprayDraw.bboxpt) delete[] theSuprSprayDraw.bboxpt;
	if (dispmesh) delete dispmesh;
}

void BlizzardParticle::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{ 	SimpleParticle::BeginEditParams(ip,flags,prev);
	editOb = this;
	this->ip = ip;

	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else { creating = FALSE; }
	if (pmapParam && pmapPGen && pmapPType && pmapPSpin && pmapEmitV && pmapSpawn) 
	{	pmapParam->SetParamBlock(pblock);
		pmapPGen->SetParamBlock(pblock);
		pmapPType->SetParamBlock(pblock);
		pmapPSpin->SetParamBlock(pblock);
		pmapEmitV->SetParamBlock(pblock);
		pmapSpawn->SetParamBlock(pblock);
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG_PTR)this);
	} else 
	{ 	pmapParam = CreateCPParamMap(
			BdescParam,PARAMBLIZZARD_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_BLIZZARD),
			GetString(IDS_RB_PARAMETERS),
			dflags&APRTS_ROLLUP1_OPEN?0:APPENDROLL_CLOSED);

		pmapPGen = CreateCPParamMap(
			BdescParamPGen,BPARAMPGEN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_GEN_BL),
			GetString(IDS_RB_PGEN),
			dflags&APRTS_ROLLUP2_OPEN?0:APPENDROLL_CLOSED);
		
		pmapPType = CreateCPParamMap(
			BdescParamPType,BPARAMPTYPE_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_PARTTYPE_BL),
			GetString(IDS_RB_PTYPE),
			dflags&APRTS_ROLLUP3_OPEN?0:APPENDROLL_CLOSED);		
	
		pmapPSpin = CreateCPParamMap(
			descBParamPSpin,BPARAMPSPIN_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_ROT_BL),
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

		pmapSpawn = CreateCPParamMap(
			descBPSpawning,PBSPAWNINGPARAMS_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SUPRPRTS_SPAWNINGB),
			GetString(IDS_AP_PSPAWN),
			dflags&APRTS_ROLLUP6_OPEN?0:APPENDROLL_CLOSED);		
	
		hParams2 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SUPRPRTS_SAVE),
				CustomSettingParamDlgProc, 
				GetString(IDS_RB_LOADSAVE), 
				(LPARAM)this,dflags&APRTS_ROLLUP7_OPEN?0:APPENDROLL_CLOSED);		
		ip->RegisterDlgWnd(hParams2);

		}
	spawn=pmapSpawn->GetHWnd();
	hparam=pmapParam->GetHWnd();
	hgen=pmapPGen->GetHWnd();
	hptype=pmapPType->GetHWnd();
	hrot=pmapPSpin->GetHWnd();
	if (pmapParam) pmapParam->SetUserDlgProc(new BLParticleDlgProc(this));
	if (pmapPType) pmapPType->SetUserDlgProc(new BLParticleDisableDlgProc(this));
	if (pmapPGen) pmapPGen->SetUserDlgProc(new SSParticleGenDlgProc(this));
	if (pmapSpawn) pmapSpawn->SetUserDlgProc(new ComParticleSpawnDlgProc(this));
	if (pmapPSpin) pmapPSpin->SetUserDlgProc(new BParticleSpinDlgProc(this));
}	

void BlizzardParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{   TimeValue t0,t2;
	SetFlag(dflags,APRTS_ROLLUP1_OPEN,IsRollupPanelOpen(hparam));
	SetFlag(dflags,APRTS_ROLLUP2_OPEN,IsRollupPanelOpen(hgen));
	SetFlag(dflags,APRTS_ROLLUP3_OPEN,IsRollupPanelOpen(hptype));
	SetFlag(dflags,APRTS_ROLLUP4_OPEN,IsRollupPanelOpen(hrot));
	SetFlag(dflags,APRTS_ROLLUP5_OPEN,IsRollupPanelOpen(pmapEmitV->GetHWnd()));
	SetFlag(dflags,APRTS_ROLLUP6_OPEN,IsRollupPanelOpen(spawn));
	SetFlag(dflags,APRTS_ROLLUP7_OPEN,IsRollupPanelOpen(hParams2));
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
		DestroyCPParamMap(pmapSpawn);

		ip->UnRegisterDlgWnd(hParams2);
		ip->DeleteRollupPage(hParams2);
		hParams2 = NULL;

		pmapParam  = NULL;
		pmapPGen = NULL;
		pmapPType = NULL;
		pmapPSpin = NULL;
		pmapEmitV = NULL;
		pmapSpawn = NULL;
	}else
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG)NULL);
		ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
	}

void BlizzardParticle::BuildEmitter(TimeValue t, Mesh& amesh)
	{
	float width, height;
	mvalid = FOREVER;
	pblock->GetValue(PB_EMITRWID,t,width,mvalid);
	pblock->GetValue(PB_EMITRLENGTH,t,height,mvalid);
	width  *= 0.5f;
	height *= 0.5f;

	mesh.setNumVerts(7);
	mesh.setNumFaces(6);
	mesh.setVert(0, Point3(-width,-height, 0.0f));
	mesh.setVert(1, Point3( width,-height, 0.0f));
	mesh.setVert(2, Point3( width, height, 0.0f));
	mesh.setVert(3, Point3(-width, height, 0.0f));
	mesh.setVert(4, Point3(  0.0f,   0.0f, 0.0f));
	mesh.setVert(5, Point3(  0.0f,   0.0f, -(width+height)/2.0f));
	mesh.setVert(6, Point3(  0.0f,   0.0f, 0.0f));

	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[0].setVerts(0,1,3);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[1].setVerts(1,2,3);

	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[2].setVerts(4,5,6);

	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[3].setVerts(0,3,1);

	mesh.faces[4].setEdgeVisFlags(0,1,1);
	mesh.faces[4].setSmGroup(0);
	mesh.faces[4].setVerts(1,3,2);

	mesh.faces[5].setEdgeVisFlags(1,0,0);
	mesh.faces[5].setSmGroup(0);
	mesh.faces[5].setVerts(5,4,6);

	mesh.InvalidateGeomCache();
	}

BOOL BlizzardParticle::ComputeParticleStart(TimeValue t0,int c)
	{
	int seed,anifr,anioff,tani;
	if (c > gCountUpperLimit) c = gCountUpperLimit;
	pblock->GetValue(PB_RNDSEED,t0,seed,FOREVER);
    pblock->GetValue(PB_OFFSETAMOUNT,t0,anifr,FOREVER);
    pblock->GetValue(PB_ANIMATIONOFFSET,t0,anioff,FOREVER);
	srand(seed);					
	parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
	int pcount=parts.Count();
    if (sdata){delete[] sdata;sdata=NULL;} if (pcount) sdata=new CSavePt[pcount];
	if ((pcount<c)||(c>0 && (!sdata)))
	{   parts.FreeAll();if (sdata) delete sdata;sdata=NULL;	maincount=0;
		BOOL playing=GetCOREInterface()->IsAnimPlaying();
		if (playing) GetCOREInterface()->EndAnimPlayback();
	    TSTR name;name=(cnode ? cnode->GetName() : TSTR(GetString(IDS_AP_BLIZZARD)));
		TSTR buf; buf=TSTR(GetString(IDS_OFM_PART));
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			GetString(IDS_OFM_ERROR),_T("%s: \n\n%s\n"),buf,name);
	  return(0);
	}
	float tmp;
	int oneframe=GetTicksPerFrame();
	for (int i=0; i<parts.Count(); i++) {
		parts.ages[i] = -1;
		sdata[i].themtl=0;
		sdata[i].gennum=0;
  		sdata[i].L=RND0x(99);sdata[i].DL=-1;sdata[i].pvar=RND11();
		tani=RND0x(anifr/oneframe);
		sdata[i].showframe=(anioff==2?tani*oneframe:0);
		tmp=RND01();sdata[i].Fo=tmp;
		sdata[i].V.x=RND01();
		sdata[i].V.y=RND01();
		sdata[i].V.z=RND11();
		sdata[i].Ts0=RND11();
		sdata[i].Ts=0.0f;
		sdata[i].LamTs=RND11();
		tmp=RND11();sdata[i].M=tmp;
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		parts.vels[i]=Point3(xtmp,ytmp,ztmp);
		sdata[i].Vsz=RND11();
		// Martell 4/14/01: Fix for order of ops bug.
		ztmp=RND11(); ytmp=RND11(); xtmp=RND11();
		sdata[i].W=Point3(xtmp,ytmp,ztmp);
		tmp=RND11();sdata[i].Dis=tmp;
		parts.radius[i]=0.0f;
		parts.tension[i]=RND11();
		sdata[i].Mltvar=RND11();
		sdata[i].SpVar=RND0x(99);
		}
	tvalid = t0-1;
	valid  = TRUE;
	rseed=rand();
	return (1);
	}

void BlizzardParticle::BirthParticle(INode *node,TimeValue bt,int num,VelDir* ptvel,Matrix3 tmlast)
{	Matrix3 tm = node->GetObjTMBeforeWSM(bt);
	Matrix3 atm = node->GetObjTMAfterWSM(bt);
//	tm.SetRow(3,atm.GetRow(3));
	Point3 vel;
	float Ie,Em,Vm,width,height;
	int RotSampleOn;
	srand(rseed);

	pblock->GetValue(PB_EMITRWID,bt,width,FOREVER);
	pblock->GetValue(PB_EMITRLENGTH,bt,height,FOREVER);
	pblock->GetValue(PB_EMITVINFL,bt,Ie,FOREVER);
	pblock->GetValue(PB_EMITVMULT,bt,Em,FOREVER);
	pblock->GetValue(PB_EMITVMULTVAR,bt,Vm,FOREVER);
	pblock->GetValue(PB_SUBFRAMEROT2,bt,RotSampleOn,FOREVER);

	int MotionOffset,EmitOffset;

	pblock->GetValue(PB_SUBFRAMEMOVE,bt,MotionOffset,FOREVER);
	pblock->GetValue(PB_SUBFRAMETIME,bt,EmitOffset,FOREVER);

	sdata[num].Ts0 = (1.0f + sdata[num].Ts0*ptvel->VSpin)/TWOPI;
	sdata[num].Ts = (float)ptvel->Spin*sdata[num].Ts0;
	parts.tension[num] = ptvel->bstr*(1.0f + parts.tension[num]*ptvel->bstrvar);
	sdata[num].persist = (TimeValue)(ptvel->persist*(1.0f + sdata[num].pvar*ptvel->pvar));
// ok, so I'm using L for M and .z for L.  They were unused float and ints
	sdata[num].M = (sdata[num].L<Ie?Em*(1 + sdata[num].M*Vm):0);  
	sdata[num].L = ptvel->Life + (int)(parts.vels[num].z*ptvel->Vl);
	sdata[num].Vsz *= ptvel->VSz;
	sdata[num].LamTs = ptvel->Phase*(1.0f + sdata[num].LamTs*ptvel->VPhase);

	if (ptvel->axisentered==1)
	{	sdata[num].W = Normalize(ptvel->Axis);
		if (ptvel->axisvar>0.0f)
			VectorVar(&sdata[num].W,ptvel->axisvar,180.0f);
	}
	else 
		sdata[num].W = Normalize(sdata[num].W);

	parts.ages[num] = 0;
	vel = Point3(0.0f,0.0f,-ptvel->Speed*(1.0f + sdata[num].V.z*ptvel->VSpeed));

	Matrix3 OffRotTm;
	if (RotSampleOn) 
		MakeInterpRotXform(tmlast,tm,(1.0f - sdata[num].Fo),OffRotTm);
	else 
		OffRotTm=tm;

	vel = VectorTransform(OffRotTm,vel);

	parts.points[num].x = -width/2.0f + sdata[num].V.x * width;
	parts.points[num].y = -height/2.0f + sdata[num].V.y * height;
	parts.points[num].z = 0.0f;
	parts[num] = parts[num]*tm;

	if (MotionOffset) 
		parts[num] += ptvel->bps.Position(1.0f - sdata[num].Fo) - tm.GetTrans();

	if (EmitOffset) 
		parts[num] += vel*(sdata[num].Fo);

	if (sdata[num].M != 0.0f)	
	{
		Point3 inhVel = ptvel->bps.Speed(1.0f - sdata[num].Fo);
		parts[num] += inhVel*sdata[num].M*sdata[num].Fo;
		vel += inhVel*sdata[num].M; // motion inheritance	
	}

	vel /= (float)GetTicksPerFrame();

	parts.vels[num] = vel;
	sdata[num].V = parts[num];
	rseed = rand();
}

void CommonParticle::GetTimes(TimeLst &times,TimeValue t,int anifr,int ltype)
{ int m,n,found,tmax,tnums=0,tgen;
  TimeValue tframe;
  times.tl.SetCount(0);times.tl.Shrink();
  times.tl.Resize(100);tmax=100;times.tl.SetCount(tmax);
  for (m=0;m<parts.Count();m++)
  { if (!parts.Alive(m)) continue;
    if (ltype)
	{ if (ltype==1) tframe=sdata[m].showframe+parts.ages[m];
	  else tframe=sdata[m].showframe+t;
     // if ((tframe>=anifr)&&(anifr!=0)) tframe=tframe % anifr;
	} else tframe=t;
	found=n=0;
	tgen=(sdata[m].gennum>nlist.Count()?nlist.Count():sdata[m].gennum);
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

void BlizzardParticle::UpdateParticles(TimeValue t,INode *node)
{
	TimeValue t0,dt,t2,grow,fade;
	int i, j, c, birth,counter,tpf=GetTicksPerFrame(),count=0,anioff;
	VelDir ptvel;
	int isrend = TestAFlag(A_RENDER),bmethod,onscreen,oneframe;
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
	float FperT = GetFrameRate()/(float)TIME_TICKSPERSEC;
    pblock->GetValue(PB_ANIMATIONOFFSET,t,anioff,FOREVER);
	pblock->GetValue(PB_EMITSTART,t,t0,FOREVER);
	pblock->GetValue(PB_SIZE,t,parts.size,FOREVER);	
	pblock->GetValue(PB_BIRTHMETHOD,0,bmethod,FOREVER);
	pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	pblock->GetValue(PB_EMITSTOP,0,t2,FOREVER);
	pblock->GetValue(PB_TUMBLE,t,theSuprSprayDraw.tumble,FOREVER);
	pblock->GetValue(PB_TUMBLERATE,t,theSuprSprayDraw.scale,FOREVER);	
	int subtree,frag,custmtl=0;
    pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
	pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	theSuprSprayDraw.scale /= 50.0f;
	if (bmethod)
		pblock->GetValue(PB_PTOTALNUMBER,0,c,FOREVER);
	if (t < t0) 
	{
		// Before the start time, nothing is happening
		parts.FreeAll();
		if (sdata) 
		{
			delete[] sdata;
			sdata=NULL;
		}
		ResetSystem(t);
		return;
	}
	int pkind;
	float dpercent;
	pblock->GetValue(PB_DISPLAYPORTION,0,dpercent,FOREVER);
	pblock->GetValue(PB_PARTICLETYPE,0,pkind,FOREVER);

	if ((!valid || t<tvalid)|| tvalid<t0) 
	{	// Set the particle system to the initial condition
	    int cincr;
		if (!bmethod)
		{	c=0;
			for (TimeValue ti=t0;ti<=t2;ti+=oneframe)
			{	pblock->GetValue(PB_PBIRTHRATE,ti,cincr,FOREVER);
				if (cincr<0) cincr=0;
					c += cincr;
			}
		}
		if (!isrend) 
			c = (int)(dpercent*(float)c+FTOIEPS);
		if (!ComputeParticleStart(t0,c))
		{	ResetSystem(t);
			return;
		}
		dispt = t-1;
		maincount = parts.Count();
		ResetSystem(t,FALSE);
	}
	int total;
	total = maincount;
	valid = TRUE;
	int stype,maxgens,spmultb,sper;float spawnbvar;
    pblock->GetValue(PB_SPAWNTYPE,0,stype,FOREVER);
    pblock->GetValue(PB_SPAWNGENS,0,maxgens,FOREVER);
    pblock->GetValue(PB_SPAWNCOUNT,0,spmultb,FOREVER);
    pblock->GetValue(PB_SPINAXISTYPE,0,ptvel.axisentered,FOREVER);
	SpawnVars spawnvars;
	spawnvars.axisentered=ptvel.axisentered;
	TimeValue dis;
    pblock->GetValue(PB_DISPUNTIL,0,dis,FOREVER);

	if (t2<t0) 
		t2=t0;
//	TimeValue fstep=oneframe;

//	t2+=fstep;
	TimeValue createover;
	createover = t2-t0+oneframe;
	counter = (isrend?rcounter:vcounter);
	float frate,grate;
	pblock->GetValue(PB_GROWTIME,0,grow,FOREVER);
    pblock->GetValue(PB_FADETIME,0,fade,FOREVER);
	frate = (fade>0.0f?(1-M)/fade:0.0f);
	grate = (grow>0.0f?(1-M)/grow:0.0f);
	float basesize,smper;
	BOOL fullframe;
	if (!isrend)
	{	int offby = t%oneframe;
		if (offby!=0) 
			t -= offby;
	}
    pblock->GetValue(PB_SPAWNSCALESIGN,0,spawnvars.scsign,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDSIGN,0,spawnvars.spsign,FOREVER);
    pblock->GetValue(PB_SPAWNINHERITV,0,spawnvars.invel,FOREVER);
    pblock->GetValue(PB_SPAWNSPEEDFIXED,0,spawnvars.spconst,FOREVER);
    pblock->GetValue(PB_SPAWNSCALEFIXED,0,spawnvars.scconst,FOREVER);
	BOOL first=(tvalid<t0);
	while ((tvalid < t)&&(tvalid<=dis))
	{	int born = 0;		
		if (first) tvalid=t0;
			// Compute our step size
		if (tvalid%stepSize !=0) 
		{	
			dt = stepSize - abs(tvalid)%stepSize;
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
			tvalid +=dt;
		if (tvalid>dis)
		{	for (j=0; j<parts.Count(); j++)
			{	
				parts.ages[j] = -1;  
			}
			tvalid = t;
			continue;
		}
	    pblock->GetValue(PB_BLSPAWNDIEAFTER,tvalid,ptvel.persist,FOREVER);
	    pblock->GetValue(PB_BLSPAWNDIEAFTERVAR,tvalid,ptvel.pvar,FOREVER);
	    pblock->GetValue(PB_SPEED,tvalid,ptvel.Speed,FOREVER);
    	pblock->GetValue(PB_SPEEDVAR,tvalid,ptvel.VSpeed,FOREVER);
		pblock->GetValue(PB_SIZE,tvalid,ptvel.Size,FOREVER);
		pblock->GetValue(PB_LIFE,tvalid,ptvel.Life,FOREVER);
		pblock->GetValue(PB_LIFEVAR,tvalid,ptvel.Vl,FOREVER);
		pblock->GetValue(PB_SPINTIME,tvalid,ptvel.Spin,FOREVER);
		pblock->GetValue(PB_SPINTIMEVAR,tvalid,ptvel.VSpin,FOREVER);
		pblock->GetValue(PB_SPINPHASE,tvalid,ptvel.Phase,FOREVER);
		pblock->GetValue(PB_SPINPHASEVAR,tvalid,ptvel.VPhase,FOREVER);
		pblock->GetValue(PB_SIZEVAR,tvalid,ptvel.VSz,FOREVER);
		pblock->GetValue(PB_METATENSION,tvalid,ptvel.bstr,FOREVER);
		pblock->GetValue(PB_METATENSIONVAR,tvalid,ptvel.bstrvar,FOREVER);
		pblock->GetValue(PB_SPAWNDIRCHAOS,tvalid,spawnvars.dirchaos,FOREVER);
		pblock->GetValue(PB_SPAWNSPEEDCHAOS,tvalid,spawnvars.spchaos,FOREVER);
		spawnvars.spchaos/=100.0f;

		pblock->GetValue(PB_SPAWNPERCENT2,tvalid,sper,FOREVER);	
		pblock->GetValue(PB_SPAWNMULTVAR2,tvalid,smper,FOREVER);	
		spawnbvar=smper*(float)spmultb;

		pblock->GetValue(PB_SPAWNSCALECHAOS,tvalid,spawnvars.scchaos,FOREVER);
		pblock->GetValue(PB_SPINAXISX,tvalid,ptvel.Axis.x,FOREVER);
		pblock->GetValue(PB_SPINAXISY,tvalid,ptvel.Axis.y,FOREVER);
		pblock->GetValue(PB_SPINAXISZ,tvalid,ptvel.Axis.z,FOREVER);
		if (Length(ptvel.Axis)==0.0f) 
			ptvel.Axis.x = 0.001f;
		pblock->GetValue(PB_SPINAXISVAR,tvalid,ptvel.axisvar,FOREVER);
		spawnvars.scchaos /= 100.0f;
		spawnvars.Axis = ptvel.Axis;spawnvars.axisvar=ptvel.axisvar;
		if (llist.Count()==0) 
			deftime=ptvel.Life/oneframe;
		basesize = M*ptvel.Size;
		// Compute the number of particles that should be born 
		birth=0;
		fullframe = (tvalid%tpf==0);
		if (fullframe)
		{	if (bmethod)
			{	int tdelta;
				if (tvalid>=t2) 
					birth = total-counter;
				else
				{	
					tdelta = int((float)total*(tvalid-t0+oneframe)/createover);
					birth = tdelta-counter;
				}
			}
			else if (tvalid<=t2)
			{	pblock->GetValue(PB_PBIRTHRATE,tvalid,total,FOREVER);
				if (!isrend) 
					total = (int)(dpercent*(float)total+FTOIEPS);
				birth = total;
				if (birth+counter>maincount) 
					birth = 0;
			}
		}
		// First increment age and kill off old particles
		for (j=0;j<parts.Count();j++)
		{	if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
			{ 
				valid = FALSE;
				tvalid = t;
				cancelled = TRUE;
				parts.FreeAll();
				return;
			}
			if (!parts.Alive(j)) 
				continue;
			int spmult;
			parts.ages[j] += dt;
			if (parts.ages[j] >= sdata[j].L) 
			{	
				spmult = (int)(sdata[j].Mltvar*spawnbvar)+spmultb;
				if ((stype!=ONDEATH)||(sdata[j].gennum>=maxgens)||(sdata[j].SpVar>=sper)) 
					parts.ages[j] = -1;	
				else if (fullframe)
				{  
					spmult = (int)(sdata[j].Mltvar*(float)spawnbvar)+spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
					else 
						parts.ages[j]=-1;
				}
			} 
			else if (sdata[j].DL>-1) 
			{
				sdata[j].DL+=dt;
				if (sdata[j].DL>sdata[j].persist) 
					parts.ages[j]=-1;
			}
			if (parts.ages[j]>-1)
			{	if (fullframe && ((stype==EMIT)&&(sdata[j].gennum==0)&&(sdata[j].SpVar<sper)))
				{  
					spmult = (int)(sdata[j].Mltvar*spawnbvar)+spmultb;
					if (spmult!=0) 
						DoSpawn(j,spmult,spawnvars,ptvel.Vl,TRUE);
				}
//			if ((pkind==RENDTYPE5)||(pkind==RENDTYPE6))
//		     parts.radius[j]=ptvel.Size;  else 
				if ((stype<2)||(maxgens==0))
		 			parts.radius[j]=FigureOutSize(parts.ages[j],ptvel.Size,grow,fade,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
				else 
				{ 
					if (sdata[j].gennum==0)
						parts.radius[j] = FigureOutSize(parts.ages[j],ptvel.Size,grow,0,sdata[j].L,grate,frate)*(1+sdata[j].Vsz);
					else if (sdata[j].gennum==maxgens)
						parts.radius[j] = FigureOutSize(parts.ages[j],sdata[j].Vsz,0,fade,sdata[j].L,grate,frate);
				}
			}
		}
		if (birth>0)
		{	Matrix3 tm,tmold;	
			float stepCoef = 1.0f;
			TimeValue checkone = tvalid-t0;

			if (checkone < 0) 
				checkone = -checkone;						// ***** I just added this line

			if (checkone == 0)
				tmold = node->GetObjTMBeforeWSM(tvalid - stepSize);
			else
				tmold = node->GetObjTMBeforeWSM(checkone<stepSize?t0:(tvalid - stepSize));

			tm = node->GetObjTMBeforeWSM(tvalid);
			ptvel.bps.Init(node, tvalid, stepSize);

		// Next, birth particles at the birth rate
			for (j=counter; j<maincount; j++) 
			{
				if (born>=birth) break;

				BirthParticle(node,tvalid,j,&ptvel,tmold);
//				BirthParticle(node,tvalid,j,&ptvel,tm);

//				if ((pkind==RENDTYPE5)||(pkind==RENDTYPE6))
//					parts.radius[j]=ptvel.Size;	
//				else 
				parts.radius[j] = (grow>0?basesize:ptvel.Size)*(1.0f + sdata[j].Vsz);
				sdata[j].themtl = int((tvalid-t0)*FperT);
//				sdata[j].showframe=(anioff==1?tlast:tlast+sdata[j].showframe);
				sdata[j].showframe=(anioff==1?0:sdata[j].showframe);
				born++;
				counter++;
			}
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
			 force = fields[i]->Force(tvalid,parts[j],parts.vels[j],j);
			 float curdt=(float)dt;
			 if ((parts.ages[j]==0)&&(sdata[j].gennum==0)) curdt=tpf*sdata[j].Fo;
			 tvel += 10.0f*force * curdt;
			}
		    parts.vels[j]+=tvel;
		}
		count=0;
		int IPC,ipcsteps;float B,Vb;
  		pblock->GetValue(PB_BLIPCOLLIDE_ON,tvalid,IPC,FOREVER);
  		pblock->GetValue(PB_BLIPCOLLIDE_STEPS,tvalid,ipcsteps,FOREVER);
  		pblock->GetValue(PB_BLIPCOLLIDE_BOUNCE,tvalid,B,FOREVER);
  		pblock->GetValue(PB_BLIPCOLLIDE_BOUNCEVAR,tvalid,Vb,FOREVER);
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
							  parts[j]+=parts.vels[j]*(float)md.mintime;
						}
					}
				}
			// If we didn't collide, then increment.
		}
		else
		// Increment the positions
		for (j=0; j<parts.Count(); j++)
		{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
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
				// if (cobjs[i]->CheckCollision(tvalid,parts[j],parts.vels[j], (float)dt, j,&meaninglesstime,TRUE)) 
				if (cc.CheckCollision(tvalid, parts[j], parts.vels[j], remTime, j, &collisionTime, FALSE))
				{
					collide = TRUE;
					remTime -= collisionTime;
					if (remTime <= 0.0f) break; // time limit for the current integration step
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
					if (sdata[j].persist==0) {parts.ages[j] = -1;count--;}
					else sdata[j].DL=0;
				}
				else if (fullframe &&((stype==COLLIDE)&&(sdata[j].gennum<maxgens)&&(sdata[j].SpVar<sper)))
				{ 
					int spmult=(int)(sdata[j].Mltvar*spawnbvar)+spmultb;
					if (spmult!=0) DoSpawn(j,spmult,spawnvars,ptvel.Vl,FALSE);
				}
			}
			
			if (!collide) parts[j] += parts.vels[j] * (float)dt;
		}
		if (first) first=FALSE;
		for (j=0; j<parts.Count(); j++)
		{ if ((!parts.Alive(j))||(parts.ages[j]==0)) continue;
		  sdata[j].Ts = (float)ptvel.Spin*sdata[j].Ts0;
		  sdata[j].LamTs += (FloatEQ0(sdata[j].Ts)?0.0f:dt/sdata[j].Ts);
		}
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
      theSuprSprayDraw.anifr=aniend+stepSize;
	  theSuprSprayDraw.t=t;
	  theSuprSprayDraw.anioff=anioff;
	if (count>0)
	   GetTimes(times,t,theSuprSprayDraw.anifr,anioff);
	else times.tl.ZeroCount();
	if (onscreen==2)
	  GetMesh(t,subtree,custmtl);
	else  GetallBB(custnode,subtree,t);
	}  
	if (isrend) rcounter=counter;
	else
	{ vcounter=counter;
	}
	if (tvalid<t) tvalid=t;
	valid=TRUE;
//	assert(tvalid==t);
	}

Mesh *BlizzardParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
{	float Mval=1.0f, Uval, Vval,Wval,Angle,elapsedtime,tumble,scale;
	int type, count,maptype,anifr;
	float height;
	Point3 Center;
	TVFace defface;
	Tab<int> tVertsOffset;
	MultipleChannelMapMerger mcmm;
	BOOL mirror=FALSE;
	TimeValue mt,aniend=GetAnimEnd();
	Mesh *pm = new Mesh;		
	if (cancelled) 
	{ZeroMesh(pm);mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;}
	dispt=t;
    anifr=aniend+GetTicksPerFrame();
	int nummtls=0,curmtl=0,multi=0,pc=0,custmtl=0;
	pblock->GetValue(PB_PARTICLETYPE,0,type,FOREVER);
	pblock->GetValue(PB_PARTICLECLASS,0,pc,FOREVER);
	if (pc==METABALLS) type=RENDMETA;
	else if (pc==INSTGEOM)
	{if (custnode==NULL) {type=0;pc=ISSTD;} else type=RENDGEOM;
	 pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
	}
	float FperT=GetFrameRate()/(float)TIME_TICKSPERSEC;
	pblock->GetValue(PB_MAPPINGTYPE,t,maptype,FOREVER);
	if (maptype==2)
	{ pblock->GetValue(PB_EMITRWID,t,Mval,FOREVER);
      pblock->GetValue(PB_EMITRLENGTH,t,height,FOREVER);
	  if (height==0.0f) height=0.01f;
	}
    else if (maptype==1)
	 pblock->GetValue(PB_MAPPINGDIST,t,Mval,FOREVER);
	else 
	 pblock->GetValue(PB_MAPPINGTIME,0,mt,FOREVER);
    if (Mval==0.0f) Mval=0.01f;
	pblock->GetValue(PB_TUMBLE,t,tumble,FOREVER);
	pblock->GetValue(PB_TUMBLERATE,t,scale,FOREVER);
	Matrix3 wcam,cam=ident;
	Point3 v, v0,v1, otherV,camV;
	int isrend=!TestAFlag(A_NOTREND);

//my comment out below - ecp
//	if ((!isrend)&&((type==RENDTYPE5)||(type==RENDTYPE6))) 
//		type=RENDTYPE1;

	if (isrend)
	{ 
		cam = Inverse(wcam = view.worldToView);
		otherV = cam.GetRow(2),camV = cam.GetRow(3);
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

	Matrix3 tm = inode->GetObjTMAfterWSM(t);
	Matrix3 itm = Inverse(tm);
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
	Center = Zero * tm;
	float Thetah;
	Point3 r1;
	if (count==0)
	{ ZeroMesh(pm);goto quit;}
	else
	{ mirror=DotProd(tm.GetRow(0)^tm.GetRow(1),tm.GetRow(2))<0.0f;
	if (type==RENDTYPE6)
	{ 
		if (view.projType) 
			type=RENDTYPE5;
		else
		{ 
			Thetah=view.fov;
			r1=cam.GetRow(1);
		}
	}
	int i,gtvnum=0,anioff=0;
	if (pc<METABALLS) GetMeshInfo(type,count,pm,&numF,&numV);
	else if (type==RENDGEOM)
	{ int subtree,onscreen;
	  pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	  pblock->GetValue(PB_ANIMATIONOFFSET,0,anioff,FOREVER);
//	  if (onscreen!=2) {
	  theSuprSprayDraw.t=t;
	  GetTimes(times,t,anifr,anioff);
	  GetMesh(t,subtree,custmtl);
//	  }
	  if (backpatch)
	  { int custmtl,frag;
        pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
        pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	    if ((custnode)&&(frag==INSTGEOM)&& custmtl) 
		  GetSubs(inode,custnode,subtree,t);
	  }
	  TimeValue Ctime;
	  int mnum,tmptvs=0;
	  numV=0;numF=0;
	  BOOL alltex=TRUE;
	  for (int pcnt=0;pcnt<parts.Count();pcnt++)
	  { if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		 if (!parts.Alive(pcnt)) continue;
		Ctime=(anioff?GetCurTime(sdata[pcnt].showframe,(anioff>1?t:parts.ages[pcnt]),anifr):t);
		mnum=TimeFound(times,Ctime,(sdata[pcnt].gennum>nlist.Count()?nlist.Count():sdata[pcnt].gennum));
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
	if (count>0)
	{
	if (type==RENDMETA)
	{ float res,bstr,thres=0.6f;int notdraft;
      pblock->GetValue(PB_METATENSION,t,bstr,FOREVER);
      pblock->GetValue(PB_BLNOTDRAFT,t,notdraft,FOREVER);
	  notdraft = (notdraft?0:1);
	  if (isrend) pblock->GetValue(PB_METACOURSE,0,res,FOREVER);
	  else pblock->GetValue(PB_METACOURSEVB,0,res,FOREVER);
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
	InDirInfo indir;
	indir.vel=Zero;
	indir.inaxis=0;
	indir.K = 0;
	indir.oneframe = GetTicksPerFrame();
	MtlID cm;
    for (i=0; i<parts.Count(); i++) 
	{ if ((!isrend)&&(GetAsyncKeyState (VK_ESCAPE)) )
		{ ZeroMesh(pm);cancelled=TRUE;goto quit;}
		if (!parts.Alive(i)) continue;
	    if (multi) 
		{ curmtl=sdata[i].themtl;
	      if (curmtl>=nummtls) 
		    curmtl=curmtl % nummtls;
	    } else curmtl=i;
		float x;
	    elapsedtime=(float)parts.ages[i];
		if (maptype==2)
		{ Uval=(parts.points[i].x-(Center.x-Mval/2.0f))/Mval;
		  Vval=(parts.points[i].y-(Center.y-height/2.0f))/height;
		}
		else if (maptype) Vval=(x=Length(parts[i]-Center))/Mval;
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
		 {TimeValue Ctime=(anioff?GetCurTime(sdata[i].showframe,(anioff>1?t:parts.ages[i]),anifr):t);
		 int mnum=TimeFound(times,Ctime,(sdata[i].gennum>nlist.Count()?nlist.Count():sdata[i].gennum));
		   if (mnum<0) continue;
		   numF=cmesh[mnum].getNumFaces();
		   numV=cmesh[mnum].getNumVerts();
			if ((gtvnum>0)&&(custmtl))
			{ if (cmesh[mnum].getNumTVerts()>0)
			  { for (j=0;j<numF;j++)
				  pm->tvFace[j+face].setTVerts(cmesh[mnum].tvFace[j].t[0]+tVertsOffset[mnum],
												cmesh[mnum].tvFace[j].t[1]+tVertsOffset[mnum],
												cmesh[mnum].tvFace[j].t[2]+tVertsOffset[mnum]);
			  } else pm->tvFace[j+face]=defface;
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
			{cm=cmesh[mnum].faces[j].getMatID();
			 int mtlgen=times.tl[mnum].gennum-1,maxmtl=nmtls.Count();
			 if (mtlgen>=maxmtl) mtlgen=maxmtl-1;
			 if ((mtlgen>-1)&&((times.tl.Count()>0)&&(times.tl[mnum].gennum>0)))
				cm+=nmtls[mtlgen];
			 pm->faces[jf].setMatID(cm);
			}
			pm->faces[jf].setEdgeVisFlags(cface.getEdgeVis(0),cface.getEdgeVis(1),cface.getEdgeVis(2));
			jf++;
		  }
		  PlotCustom(parts.radius[i],i,vertexnum,pm,Angle,&sdata[i].W.x,&cmesh[mnum],&parts.points[i],numV,indir);
		 }
		if (((type<RENDTYPE5)||(type>RENDTYPE6))&&(tumble>0.0f))
	    { Matrix3 mat = TumbleMat(i,tumble,scale);
	     for (int j=vertexnum;j<vertexnum+numV;j++) 
		  pm->verts[j]=parts.points[i]+(pm->verts[j]-parts.points[i])*mat;}

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
  if ((isrend)&&(midframe) )
  { if (pcount>0)
	{CacheData(&lastparts,&parts);
    CacheSpin(holddata,sdata,pcount,FALSE);
	delete[] holddata;}
    tvalid=t-offtime;
  }
  if (mirror) SwitchVerts(pm);
	quit:	mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;
}


void BlizzardParticle::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	if (pmapPGen) pmapPGen->Invalidate();
	if (pmapPType) pmapPType->Invalidate();
	if (pmapPSpin) pmapPSpin->Invalidate();
	if (pmapEmitV) pmapEmitV->Invalidate();
	if (pmapSpawn) pmapSpawn->Invalidate();
	}

RefTargetHandle BlizzardParticle::Clone(RemapDir& remap) 
	{
	BlizzardParticle* newob = new BlizzardParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
	newob->mvalid.SetEmpty();	
	newob->tvalid = FALSE;
	newob->dflags=dflags;
	newob->nlist.SetCount(nlist.Count());
	newob->llist.SetCount(llist.Count());
	for (int ix=0;ix<nlist.Count();ix++) 
	{ newob->nlist[ix]=NULL;
	  newob->ReplaceReference(ix+BASER,nlist[ix]);
	}
	for (ix=0;ix<llist.Count();ix++)
	  newob->llist[ix]=llist[ix];
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
	 { pblock->GetValue(PB_CUSTOMMTL2,0,custmtl,FOREVER);
	   newob->GetMesh(t,subtree,custmtl);
	 }
	 else newob->GetallBB(custnode,subtree,t);
	}
	BaseClone(this, newob, remap);
	return newob;
	}

ParamDimension *BlizzardParticle::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SPEED:
		case PB_SIZE:
		case PB_METATENSION:
		case PB_METACOURSE:
		case PB_MAPPINGDIST:
		case PB_SPINAXISX:
		case PB_SPINAXISY:
		case PB_SPINAXISZ:
		case PB_EMITVMULT:
//		case PB_TUMBLE:
//		case PB_TUMBLERATE:
		case PB_EMITRWID:  		
		case PB_EMITRLENGTH:  		return stdWorldDim;
		
		case PB_RNDSEED:
		case PB_PBIRTHRATE:
		case PB_PTOTALNUMBER:
			return defaultDim;

		case PB_SPINPHASE:
		case PB_SPINAXISVAR:	return stdAngleDim;

		case PB_DISPLAYPORTION:
		case PB_SIZEVAR:
		case PB_SPEEDVAR:
		case PB_METATENSIONVAR:
		case PB_SPINTIMEVAR:
		case PB_SPINPHASEVAR:
		case PB_EMITVMULTVAR:	
		case PB_SPAWNMULTVAR2:
		case PB_BLSPAWNDIEAFTERVAR:		
		case PB_BLIPCOLLIDE_BOUNCE:		
		case PB_BLIPCOLLIDE_BOUNCEVAR:		
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
		case PB_BLSPAWNDIEAFTER:		
								return stdTimeDim;
		
		default: return defaultDim;
		}
	}

TSTR BlizzardParticle::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_TUMBLE:			return GetString(IDS_AP_TUMBLE);
		case PB_TUMBLERATE:		return GetString(IDS_AP_TUMBLERATE);
		case PB_EMITRLENGTH:	return GetString(IDS_AP_EMITRLENGTH);
		case PB_SPEED:			return GetString(IDS_RB_SPEED);
		case PB_SPEEDVAR:		return GetString(IDS_RB_SPEEDVAR);
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
		case PB_BUBLPERVAR:		return GetString(IDS_EP_SPAWNAFFECTS);
		case PB_BUBLPHAS:		return GetString(IDS_EP_SPAWNMULTVAR);
		case PB_BUBLPHASVAR:	return GetString(IDS_RB_BUBLPHASVAR);
		case PB_EMITRWID:		return GetString(IDS_RB_EMITRWID);
		case PB_OFFSETAMOUNT:	return GetString(IDS_AP_OFFSETAMT);
		case PB_SPAWNDIRCHAOS:	return GetString(IDS_AP_SPAWNDIRCHAOS);
		case PB_SPAWNSPEEDCHAOS:		return GetString(IDS_AP_SPAWNSPEEDCHAOS);
		case PB_SPAWNSCALECHAOS:		return GetString(IDS_AP_SPAWNSCALECHAOS);
		case PB_BLSPAWNDIEAFTER:		return GetString(IDS_AP_SPAWNDIEAFTER);
		case PB_BLSPAWNDIEAFTERVAR:		return GetString(IDS_AP_SPAWNDIEAFTERVAR);
		case PB_BLIPCOLLIDE_ON:			return GetString(IDS_AP_IPCOLLIDE_ON);
		case PB_BLIPCOLLIDE_STEPS:		return GetString(IDS_AP_IPCOLLIDE_STEPS);
		case PB_BLIPCOLLIDE_BOUNCE:		return GetString(IDS_AP_IPCOLLIDE_BOUNCE);
		case PB_BLIPCOLLIDE_BOUNCEVAR:	return GetString(IDS_AP_IPCOLLIDE_BOUNCEVAR);
			
		default: 				return TSTR(_T(""));
		}
	}	

BOOL CPickOperand::Filter(INode *node)
	{
	if ((node)&&(!node->IsGroupHead())) {
		ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
		if (!IsGEOM(os.obj))
		{	node = NULL;
			return FALSE;
		}
	}

	return node ? TRUE : FALSE;
}

BOOL CPickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if ((node)&&(!node->IsGroupHead())) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if (!IsGEOM(os.obj)) 
		{	node = NULL;
			return FALSE;
			}
	}

	return node ? TRUE : FALSE;
	}

void CommonParticle::ShowName()
{ TSTR name;
if (hptype)
{FormatName(name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE))));
 SetWindowText(GetDlgItem(hptype, IDC_AP_INSTANCESRCNAME), name);}
}

BOOL CPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);

	INodeTab nodes;
//	if ((dodist==1)||(dodist==2)) {nodes.SetCount(1);nodes[0]=node;}
	int subtree;
	  if (node->IsGroupMember()) 
	    while (node->IsGroupMember()) node=node->GetParentNode();
	  po->pblock->GetValue(PB_USESUBTREE,0,subtree,FOREVER);
	  nodes.SetCount(0);
	if (dodist==1)
	 {  theHold.Begin();
		ComObjectListRestore *padd;
		theHold.Put(padd=(new ComObjectListRestore(po)));
		po->AddToList(node,po->nlist.Count(),TRUE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
	else if (dodist==2)
	  { theHold.Begin();
		ComObjectListRestore *padd;
		theHold.Put(padd=(new ComObjectListRestore(po)));
	    po->AddToList(node,repi,FALSE);
		theHold.Accept(GetString(IDS_AP_OBJADD));
	  }
	else
	{
	theHold.Begin();
	theHold.Put(new CreateCPartPickNode(po,po->custname,node->GetName()));
	if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	theHold.Accept(GetString(IDS_AP_COMPICK));
	po->custname = TSTR(node->GetName());
	// Automatically check show result and do one update
	po->ShowName();	
}
	if (po->flags=(node->IsGroupHead()?1:0))
	 MakeGroupNodeList(node,&nodes,subtree,ip->GetTime());
	else MakeNodeList(node,&nodes,subtree,ip->GetTime());
	int frag,custmtl,onscreen;
	po->pblock->GetValue((po->ClassID()==BLIZZARD_CLASS_ID?PB_CUSTOMMTL2:PB_CUSTOMMTL),0,custmtl,FOREVER);
	po->pblock->GetValue(PB_PARTICLECLASS,0,frag,FOREVER);
	po->pblock->GetValue(PB_VIEWPORTSHOWS,0,onscreen,FOREVER);
	if ((frag==INSTGEOM)&&(onscreen>1))
	{  if (onscreen==2)
	   po->GetMesh(ip->GetTime(),subtree,custmtl);
	  else po->GetallBB(node,subtree,ip->GetTime());}
	po->pmapParam->Invalidate();
	ip->FlashNodes(&nodes);
	nodes.Resize(0);
	po->valid=FALSE;
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	if (po->creating) {
		theCreateSSBlizMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateSSBlizMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {	
		return TRUE;
		}
	}

void CPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	if (dodist)
	{iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==2?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
	  TurnButton(po->spawn,(dodist==1?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK),FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
		TurnButton(po->hptype,IDC_AP_OBJECTPICK,FALSE);
	}
	else
	{ iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	}
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
}

void CPickOperand::ExitMode(IObjParam *ip)
	{if (!po->ip) return;
	ICustButton *iBut;
	if (dodist)
	{	iBut=GetICustButton(GetDlgItem(po->spawn,(dodist==2?IDC_AP_OBJQUEUEREPLACE:IDC_AP_OBJECTQUEUEPICK)));
		CheckInstButtons(po->pblock,po->hptype);
	  TurnButton(po->spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
	  if (dodist==2)
	   TurnButton(po->spawn,IDC_AP_OBJECTQUEUEPICK,TRUE);
//	  TurnButton(po->spawn,IDC_AP_OBJQUEUEDELETE,TRUE);
	}
	else
	{iBut=GetICustButton(GetDlgItem(po->hptype,IDC_AP_OBJECTPICK));
	  CheckSpawnButtons(po->pblock,po->spawn,repi);
	}
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}
RefTargetHandle CommonParticle::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return (RefTargetHandle)nlist[i-BASER];
		}
	}

void CommonParticle::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		default: nlist[i-BASER]= (INode *)rtarg;return;
		}
	}

RefResult CommonParticle::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_TARGET_DELETED:	
			{ if (hTarget==custnode) 
				{if (theHold.Holding())
					theHold.Put(new CreateCPartDelNode(this,custnode->GetName()));
 				DeleteReference(CUSTNODE);
				custnode=NULL;
				custname=TSTR(_T(" "));cancelled=FALSE;
			}
			  BOOL notfound=TRUE;
			  for (int i=0;(i<nlist.Count())&&(notfound);i++)
				if (hTarget==nlist[i]) 
				{ DeleteFromList(i,TRUE);
				   notfound=FALSE;cancelled=FALSE;
				}
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ if (hTarget==custnode) 
			  { custname = TSTR(custnode->GetName());
			    ShowName();
				cancelled=FALSE;
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
/*		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
			if (editOb==this) InvalidateUI();
			break;*/
		default: SimpleParticle::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}
class CommonPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		CommonPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((CommonParticle*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			int mc;
			if (oldVer<6)
			{  if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
				{ ((CommonParticle*)targ)->pblock->SetValue(PB_BLIPCOLLIDE_ON,0,0);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_BLIPCOLLIDE_STEPS,0,2);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_BLIPCOLLIDE_BOUNCE,0,1.0f);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_BLIPCOLLIDE_BOUNCEVAR,0,0.0f);
				}
				else 
				{ ((CommonParticle*)targ)->pblock->SetValue(PB_IPCOLLIDE_ON,0,0);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_IPCOLLIDE_STEPS,0,2);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_IPCOLLIDE_BOUNCE,0,1.0f);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_IPCOLLIDE_BOUNCEVAR,0,0.0f);
				}
			}
			if (oldVer<5)
			{  if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
				{((CommonParticle*)targ)->pblock->SetValue(PB_BLSPAWNDIEAFTER,0,0);
				 ((CommonParticle*)targ)->pblock->SetValue(PB_BLSPAWNDIEAFTERVAR,0,0.0f);}
				else 
				{ ((CommonParticle*)targ)->pblock->SetValue(PB_SSSPAWNDIEAFTER,0,0);
				  ((CommonParticle*)targ)->pblock->SetValue(PB_SSSPAWNDIEAFTERVAR,0,0.0f);
				}
			}
			if (oldVer<4)
			{  if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
 				((CommonParticle*)targ)->pblock->SetValue(PB_BLNOTDRAFT,0,0);
				else ((CommonParticle*)targ)->pblock->SetValue(PB_SSNOTDRAFT,0,0);
			}
			if (oldVer<3)
			{  if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
 				((CommonParticle*)targ)->pblock->SetValue(PB_SPAWNPERCENT2,0,100);
				else ((CommonParticle*)targ)->pblock->SetValue(PB_SPAWNPERCENT,0,100);
			}
			if (oldVer<2)
			{  if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
 				((CommonParticle*)targ)->pblock->SetValue(PB_SUBFRAMEROT2,0,0);
				else ((CommonParticle*)targ)->pblock->SetValue(PB_SUBFRAMEROT,0,0);
			}
			if (oldVer<1) {	
				((CommonParticle*)targ)->pblock->GetValue(PB_METACOURSE,0,mc,FOREVER);
				 if (((CommonParticle*)targ)->ClassID()==BLIZZARD_CLASS_ID)
 				((CommonParticle*)targ)->pblock->SetValue(PB_METACOURSEVB,0,mc);
				else ((CommonParticle*)targ)->pblock->SetValue(PB_METACOURSEV,0,mc);
				}
			delete this;
			}
	};

#define COM_CUSTNAME_CHUNK	0x0100
#define COM_CUSTFLAGS_CHUNK	0x0101
#define COM_SPAWNC_CHUNK	0x0102
#define COM_LCNT_CHUNK		0x0103
#define COM_LIFE_CHUNK		0x0104

IOResult CommonParticle::Save(ISave *isave)
	{ 	ULONG nb;
	isave->BeginChunk(COM_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();

	isave->BeginChunk(COM_CUSTFLAGS_CHUNK);		
	isave->Write(&flags,sizeof(flags),&nb);
	isave->EndChunk();

	int nCount=nlist.Count();
	isave->BeginChunk(COM_SPAWNC_CHUNK);		
	isave->Write(&nCount,sizeof(nCount),&nb);
	isave->EndChunk();
	
	int Lcnt=llist.Count();
	isave->BeginChunk(COM_LCNT_CHUNK);		
	isave->Write(&Lcnt,sizeof(Lcnt),&nb);
	isave->EndChunk();

	isave->BeginChunk(COM_LIFE_CHUNK);
	for (int i=0;i<llist.Count();i++)
	{ isave->Write(&llist[i],sizeof(int),&nb);
	}
	isave->EndChunk();

	return IO_OK;
	}

IOResult CommonParticle::Load(ILoad *iload)
	{
	if (ClassID()==BLIZZARD_CLASS_ID) {
		iload->RegisterPostLoadCallback(
			new CommonPostLoadCallback(
				new ParamBlockPLCB(Bversions,NUM_OLDVERSIONS,&curVersionBL,this,0)));
	} else {
		iload->RegisterPostLoadCallback(
			new CommonPostLoadCallback(
				new ParamBlockPLCB(spversions,NUM_OLDVERSIONS,&curVersionSp,this,0)));
		}
	ULONG nb;
	IOResult res = IO_OK;
	int cnmtl=0,nCount;
	
	// Default names
	custname = TSTR(_T(" "));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case COM_CUSTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
				}
			case COM_CUSTFLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;
			case COM_SPAWNC_CHUNK:
				{	res=iload->Read(&nCount,sizeof(nCount),&nb);
					nlist.SetCount(nCount);
					for (int i=0; i<nCount; i++) nlist[i] = NULL;
				}
				break;
			case COM_LCNT_CHUNK:
				{	int Lcnt;
					res=iload->Read(&Lcnt,sizeof(Lcnt),&nb);
					llist.SetCount(Lcnt);
					for (int i=0; i<Lcnt; i++) llist[i] = NULL;
				}
				break;
			case COM_LIFE_CHUNK:
				{	for (int i=0;i<llist.Count();i++)
					res=iload->Read(&llist[i],sizeof(int),&nb);}
				break;
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}

const TCHAR *BlizzardClassDesc::ClassName ()	{return GetString(IDS_AP_BLIZZARD);}
const TCHAR *BlizzardClassDesc::Category ()	{return GetString(IDS_RB_PARTICLESYSTEMS);}
TCHAR *BlizzardParticle::GetObjectName() {return GetString(IDS_AP_BLIZZARDGC);}
void CommonParticle::SetUpList()
{ SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<nlist.Count(); i++) 
		SendMessage(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)(nlist[i]->GetName()));
}

void CommonParticle::AddToList(INode *newnode,int i,BOOL add)
{	if (add)
	{ nlist.Insert(i,1,&newnode);
	  MakeRefByID(FOREVER,BASER+i,newnode);
	}	  
	else ReplaceReference(i+BASER,newnode);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	SetUpList();
}

void CommonParticle::DeleteFromList(int nnum,BOOL all)
{ int nCount=nlist.Count();
  INode *cnode=nlist[nnum];
  DeleteReference(nnum+BASER);
  if (theHold.Holding() && !TestAFlag(A_HELD)) 
	  theHold.Put(new ComObjectListRestore(this));
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

void CommonParticle::SetUpLifeList()
{ TCHAR buffer[20];
  SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_RESETCONTENT,0,0);
  for (int i=0;i<llist.Count(); i++) 
  {	_itoa(llist[i], buffer, 10 );
	SendMessage(GetDlgItem(spawn,IDC_AP_LIFEQUEUE),LB_ADDSTRING,0,(LPARAM)(TCHAR*)buffer);
  }
} 

void CommonParticle::AddToLifeList(int newlife)
{	llist.Insert(llist.Count(),1,&newlife);
	SetUpLifeList();
}

void CommonParticle::DeleteFromLifeList(int nnum)
{ 	llist.Delete(nnum,1);
    if (ip) SetUpLifeList();
}
int CommonParticle::HitTest(
		TimeValue t, INode *inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt) 
{ 	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
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
	    dispmesh=GetRenderMesh(t,inode,nullView,needdel);}
		ClearAFlag(A_NOTREND);
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

int CommonParticle::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{   if (!OKtoDisplay(t)) return 0;
	if (t!=tvalid) cancelled=FALSE;
	if ((ip && origmtl) &&(origmtl!=inode->GetMtl()))
	{ EnableWindow(GetDlgItem(hptype,IDC_SP_MAPCUSTOMEMIT),TRUE);
	  origmtl=NULL;
	}
	BOOL doupdate=((!cancelled)&&((t!=tvalid)||!valid));
	if (!doupdate) doupdate=CheckMtlChange(inode->GetMtl(),wasmulti);
	if (doupdate) Update(t,inode);
	GraphicsWindow *gw = vpt->getGW();
	DWORD rlim  = gw->getRndLimits();

	// Draw emitter
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER) );	//removed BC on 4/29/99 DB
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
	{ theSuprSprayDraw.obj = this;
		theSuprSprayDraw.firstpart=TRUE;
		theSuprSprayDraw.disptype=disptype;
		theSuprSprayDraw.ptype=ptype;
		theSuprSprayDraw.bb=TRUE;
		parts.SetCustomDraw(&theSuprSprayDraw);			
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
	{ parts.SetCustomDraw(&theSuprSprayDraw);			
	  NullView nullView;
	  BOOL needdel;
      gw->setRndLimits(rlim);
	  if ((t!=dispt)||doupdate||!dispmesh)
		{
		if (dispmesh) delete dispmesh;
		SetAFlag(A_NOTREND);
		dispmesh=GetRenderMesh(t,inode,nullView,needdel);	
		ClearAFlag(A_NOTREND);
		}
	  Matrix3 mat = inode->GetObjTMAfterWSM(t);
	  gw->setTransform(mat);
	  if(dispmesh)	// dispmesh was NULL in bug 257748  DB 11/00
	    dispmesh->render(gw, inode->Mtls(),
		  (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL,
		  COMP_ALL, inode->NumMtls());
	}
	return(0);
}

#endif // NO_PARTICLE_BIZZARD NO_PARTICLE_SUPERSPRAY