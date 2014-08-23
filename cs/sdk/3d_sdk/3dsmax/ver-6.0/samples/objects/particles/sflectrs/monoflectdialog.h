#ifndef __MONOFLECTDLG__H
#define __MONOFLECTDLG__H

#include "max.h"
#include "sflectrs.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "Simpobj.h"
#include "icollision.h"
#include "sflectr.h"
// #include "pod.h" // Bayboro 9/18/01
#include "macrorec.h"

#define	MONOFLECTOR_CLASSID		Class_ID(0x5eea6cb7, 0x29027b3e)
#define	BASICFLECTOR_CLASSID	Class_ID(0x5e5259be, 0x47263c57)
#define	BASICFLECTORMOD_CLASSID	Class_ID(0x149a4a28, 0xffc17f5)

TCHAR *GetString(int id);

#define		DONTCARE	2

class dlglist
{
	public:
		int cnt;
		int *namelst;
		dlglist(int newcnt,int *list);
};

#define numpblocks 2

enum 
{	monoflectorobj_params, 
}; 

//list of subanimatables - other lists
enum 
{	pbType_subani,
	pbComplex_subani,
	monoflecdlg
}; 

enum 
{
	PB_TYPE,
	monoflect_colliderp,
	monoflect_colliders,
	monoflect_colliderm
};

enum 
{
	PB_COMPLEX
};

enum 
{
	PB_WIDTH,
	PB_LENGTH,
	PB_QUALITY,
	PB_MESHNODE,
	PB_HIDEICON
};

enum 
{
	PB_REFLECTS,
	PB_BOUNCE,
	PB_BVAR,
	PB_CHAOS,
	PB_FRICTION,
	PB_INHERVEL,
	PB_TIMEON,
	PB_TIMEOFF,
	PB_REFRACTS,
	PB_PASSVEL,
	PB_PASSVELVAR,
	PB_DISTORTION,
	PB_DISTORTIONVAR,
	PB_DIFFUSION,
	PB_DIFFUSIONVAR,
	PB_COLAFFECTS,
	PB_COLPASSVEL,
	PB_COLPASSVELVAR
};


class MonoFlector;

class ShadowParamDlg 
{
public:
	virtual void DeleteThis()=0;
};

class MonoFlectorParam: public ShadowParamDlg 
{
public:
	MonoFlector *theType;
	Interface *ip;
	int type;
	// Constructor
	MonoFlectorParam(MonoFlector *shad, Interface *iface, int num, int type,HWND cwnd);
	~MonoFlectorParam();

	void DeleteThis() { delete this; }
};
typedef MonoFlectorParam* MonoFlectorParamPtr;
typedef IParamMap2 *IParamMap2Ptr;

class BasicFlectorType: public ReferenceTarget
{	public:
	static MonoFlectorParamPtr theParam[numpblocks];
	static IParamMap2Ptr pmap[numpblocks];

	BOOL SupportStdMapInterface() { return FALSE; }
	BasicFlectorType() 
	{ }
	~BasicFlectorType()
	{ for (int i=0;i<numpblocks;i++) if (theParam[i]) theParam[i]->theType = NULL;
	  DeleteAllRefsFromMe();
	}
};

class BasicFlectorMod;

class FlectorPickOperand;

class BasicFlectorObj : public DynamModObject // ,ITestInterface // Bayboro 9/18/01
{
	public:	
		static IObjParam *ip;
		BasicFlectorObj();
		~BasicFlectorObj();
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
		void MapKeys(TimeMap *map,DWORD flags);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_AP_MONONAME); }
		static IParamMap2Ptr pmap[numpblocks];
		static FlectorPickOperand pickCB;

		int lastrnd;
		TimeValue t;
		INode *custnode;
		TSTR custname;

		BOOL SupportsDynamics();
		Mesh *dmesh;
		int nv,nf;
		VNormal *vnorms;
		Point3 *fnorms;
		Matrix3 tm,ptm,invtm,tmNoTrans,invtmNoTrans;
		Interval tmValid,mValid;
		Point3 dvel;

		// Automatic UVW Generation
		static BOOL creating;
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL cancelled;
		// Plugin identification
		void DeleteThis() {delete this;}
		int CanConvertToType(Class_ID obtype);
		Class_ID ClassID() { return BASICFLECTOR_CLASSID; } 
		MonoFlector *st;
		Modifier *CreateWSMMod(INode *node);

		FlectForces ForceData(TimeValue t);
		FlectForces ffdata;	
		TimeValue ctime;

// Direct paramblock access
		IParamBlock2 *pbComplex;
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int	NumParamBlocks() { return numpblocks; }
		IParamBlock2* GetParamBlock(int i);
		IParamBlock2* GetParamBlockByID(BlockID id);
		int NumSubs(){return numpblocks+1;}
		int NumRefs(){return numpblocks+1;}
		RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message );
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		// Create the object itself
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		void IntoPickMode();

//		int NPTestInterface(TimeValue t,BOOL UpdatePastCollide,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		CollisionObject *GetCollisionObject(INode *node);

		int ReturnThreeStateValue;
		CollisionObject *gf;
		void SetUpModifier(TimeValue t,INode *node);
		BasicFlectorMod *mf;
};

class BasicFlectorField : public CollisionObject 
{
	public:		
		BasicFlectorField()
		{
//	 		colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
//	 		cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
//	 		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
			srand(897443);
			for (int i =0;i < 500; i++)
			{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
			}		
		}

		float randomFloat[500];
		CollisionPlane *colp;
		CollisionSphere *cols;
		CollisionMesh *colm;

		void DeleteThis() 
			{	 
			if (colp) colp->DeleteThis();
			colp=NULL;
			delete this;

			if (cols) cols->DeleteThis();
			cols=NULL;
			delete this;

			if (colm) colm->DeleteThis();
			colm=NULL;
			delete this;

			}



		float width, height;
		int quality;
		float chaos,bounce,bvar,vinher,friction;
		float refvol,refvar,decel,decelvar;

		BasicFlectorObj *obj;
		INode *node;
		int badmesh;
		Point3 totalforce,applyat;
		int totalnumber;
		TimeValue curtime;

		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
};

class BasicFlectorMod : public SimpleWSMMod 
{
	public:				
		BasicFlectorField deflect;

		BasicFlectorMod() {deflect.curtime=NoAni;}
		BasicFlectorMod(INode *node,BasicFlectorObj *obj);	
		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_EP_BASICDEFLECTORMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return BASICFLECTORMOD_CLASSID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_BASICDEFLECTORBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

class BasicFlectorDlgProc : public ParamMap2UserDlgProc 
{
	public:
		BasicFlectorObj *sso;
		MonoFlector *st;
		HWND sdlgs;
		BasicFlectorDlgProc() {}

		void DeleteThis() { }
		void SetUpList(HWND cwnd,HWND hWnd,dlglist ilist);
		void SetParamBlock(IParamBlock2 *pb) 
		{
			sso = (BasicFlectorObj*)pb->GetOwner();
		}
};
extern TriObject *TriIsUseable(Object *pobj,TimeValue t);

extern ClassDesc* GetMonoFlectorDesc();

// Paramblock2 name
enum 
{	flectplane, 
	flectsphere,
	flectmesh,
}; 

enum 
{	flectsimple, 
	flectadvanced,
	flectdynamics,
}; 

enum 
{ 	flectplanepbd,
	flectspherepbd,
	flectmeshpbd,
};

enum 
{	flectsimplepbd,
	flectcomplexpbd,
	flectdynamicpbd,
};

//manages subrollups
class MonoFlector: public BasicFlectorType 
{
	public:
		IParamBlock2 *pblock2,*pbComplex;
		MonoFlector();
		MonoFlector(BasicFlectorObj *bobj);
		dlglist GetNameList(int which);
		BasicFlectorObj *bfobj;
		static HWND hParams; 
		void InvalidateUI();

		// Create the system's UI
		ShadowParamDlg *CreateMonoFlectorParamDlg(Interface *ip,int num,int type,HWND cwnd=NULL) 
		{ 
			theParam[type] = new MonoFlectorParam(this, ip,num,type, cwnd); return theParam[type];
		}
		
		SClass_ID SuperClassID() { return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() { return MONOFLECTOR_CLASSID;}

		int NumSubs() {return numpblocks;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		IParamBlock2* GetParamBlock(int i);
		IParamBlock2* GetParamBlockByID(BlockID id);
		int NumRefs() {return numpblocks;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		void DeleteThis() { delete this; }
		void ShowName(INode *oldn=NULL);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
};

#endif