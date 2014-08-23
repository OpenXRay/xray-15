

#ifndef __LAG__H
#define __LAG__H


#include "mods.h"
#include "iparamm2.h"
#include "tvnode.h"
#include "modstack.h"
#include "notify.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "macrorec.h"

#include "ilag.h"




#define PBLOCK_REF	0
#define POINT1_REF	1

#define A_RENDER			A_PLUGIN1



#define CID_CREATEPAINT CID_USER + 204


#define SPRINGMODEL

//defines to mark chunks so I can save the weights and the nail pieces

#define POS_WEIGHT_COUNT_CHUNK	100
#define POS_WEIGHT_CHUNK		150
#define NAIL_CHUNK				200
#define GRAVITY_COUNT_CHUNK		250

#define VERTLIMIT	2000
#define PATCHLIMIT	500


#define LAGCONTAINERMASTER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad494)
#define LAGCONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad483)
#define LAGNODE_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad484)



class LagNodeNotify;


class LagMod;

class ExtraSpringDataClass
{
public:
	float strength, sway;
};

class LagValidatorClass : public PBValidator
{
public:
LagMod *mod;


BOOL Validate(PB2Value &v) 
	{
	INode *node = (INode*) v.r;

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) return FALSE;

//	const ObjectState& os = node->EvalWorldState(0);
//	Object* ob = os.obj;
	Object* ob = node->GetObjectRef();;
	ob=ob->FindBaseObject();
	if (ob!=NULL) 
			{	
			int id=(ob?ob->SuperClassID():SClass_ID(0));
			if (id==WSM_OBJECT_CLASS_ID)
				{
				WSMObject *obref=(WSMObject*)ob;
				ForceField *ff = obref->GetForceField(node);
				if (ff)
					{
					ff->DeleteThis();
					return TRUE;
					}
				else return FALSE;

//				BOOL ShouldBeHere=obref->SupportsDynamics();

//				return ShouldBeHere;
				}
			else return FALSE;
				  
			}
	return FALSE;

	};
private:
};


class LagColliderValidatorClass : public PBValidator
{

private:

public:
LagMod *mod;

BOOL Validate(PB2Value &v) 
	{
	INode *node = (INode*) v.r;

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) return FALSE;

//	const ObjectState& os = node->EvalWorldState(0);
//	Object* ob = os.obj;
	Object* ob = node->GetObjectRef();;
	ob=ob->FindBaseObject();
	if (ob!=NULL) 
			{	
			int id=(ob?ob->SuperClassID():SClass_ID(0));
			if (id==WSM_OBJECT_CLASS_ID)
				{
				WSMObject *obref=(WSMObject*)ob;
				CollisionObject *col = obref->GetCollisionObject(node);
				if (col)
					{
					col->DeleteThis();
					return TRUE;
					}
				else return FALSE;
				}
			else return FALSE;
				  
			}
	return FALSE;

	};
};


class CreatePaintMode;

class LagMod :  public Modifier, public  ILagMod {	
	public:

		int numSprings, maxSprings;
		float suggested;
		int forceCount, collisionCount;
		BitArray selectedList;

		int flexVersion; 
		BOOL spinnerDown;

		BOOL affectAll;
		BOOL disableUI;
		BOOL enableAdvanceSprings;
		float stretch, stiffness;

		BOOL lazyEval;
		BOOL holdLength;
		float holdLengthPercent;

		ExtraSpringDataClass extraStrAndSway[12];
		float edgeLength, maxEdgeLength, minEdgeLength;
		void ComputeEdgeLength(LagModData *lmd,Object *obj);
		void FilterPass(LagModData *lmd,int numberPasses, float stretchPercentage,float per);

		BOOL inPaint;
		BOOL painting;

		BOOL subSelection;
		BOOL useChase, useCenter;

		int addMode;
		float holdRadius;
		float tholdRadius;

		IParamBlock2 *pblock2;

		ITrackViewNode *container;

		Control *p1;
		static HWND hParams;
		static HWND hPaint;
		static HWND hAdvance;
		static HWND hAdvanceSprings;
		static HWND hCreateSimple;

		LagNodeNotify *notify;


// data cache
		LagValidatorClass validator;
		LagColliderValidatorClass colValidator;


		BYTE sel[1];    //seclection for center

		Tab<SpringClass> oldSpringList;
		Tab<BYTE> oldesel;  //selection for edges vertices


		float weightDelta;
		int ApplyDelta() ;
	
		Point3 lagCenter;


		static IObjParam *ip;
		static IParamMap *pmapParam;
		static LagMod *editMod;
		static MoveModBoxCMode *moveMode;
		ISpinnerControl *iVWeight;

		int solver;

//Paint controlls
        float PaintStrength;
        float Radius;
        float Feather;
        int ProjectThrough;
		int absoluteWeight;
        static CreatePaintMode *PaintMode;
		void StartPaintMode();
        ICustButton* iPaintButton;
		BOOL updating;
		void ResetSprings();

		LagMod();
		~LagMod();
		Point3 GetForce(TimeValue t, Point3 p, Point3 v, Matrix3 tm);
		int GetCollision(TimeValue t, Point3 &p, Point3 &v, TimeValue &dt);


		Tab<ForceField*> ff;
		Tab<CollisionObject*> colliderObjects;

		Class_ID cid;
		void AddForceField();
		void AddCollider();

		TimeValue ReferenceFrame;
		float falloff, strength, dampening;
		
		
										
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock


		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_LAGMOD);}  
		virtual Class_ID ClassID() { return Class_ID(LAZYID);}		
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_LAGMOD);}
		BOOL AssignController(Animatable *control, int subAnim); //AF (5/03/01)

		// From modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE|PART_TOPO;}
//		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO;}
		ChannelMask ChannelsChanged() {return PART_GEOM;}
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		Interval GetValidity(TimeValue t);


		void DrawCrossSectionNoMarkers(Point3 a, Point3 align, float length, GraphicsWindow *gw);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);		
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void ActivateSubobjSel(int level, XFormModes& modes);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		int NumRefs() {return 2 ;
//			+ GravityNode.Count();
						}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int NumSubs() {return 2;}
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);


		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

		Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);


		int RenderBegin(TimeValue t, ULONG flags);		
		int RenderEnd(TimeValue t);
		BOOL nukeCache;
		void HoldWeights(LagModData *lmd);

		BOOL updateWeights;
		void ComputeCenterFalloff(LagModData *lmd, TimeValue t, ObjectState *os);

		void EnableModsAbove(LagModData *lmd, BOOL enable);
		Tab<BOOL> enableStates;
		Tab<BOOL> enableViewStates;
		BOOL aboutToRender;

		Tab<LagModData*> lmdData;

		void AddSprings(LagModData *lmd, Object *obj);
		void RemoveSprings(LagModData *lmd);
		void CreateSimpleSoft(LagModData *lmd, Object *obj);

		int CheckCache(TimeValue t);

		void Solve(int level, LagModData *lmd, TimeValue i, TimeValue t, 
				 int nv, ObjectState *os, int samples);

		void Evaluate(LagModData *lmd, TimeValue i, TimeValue t, 
				 int nv, ObjectState *os, int samples, float per);

		INode* GetNodeFromModContext(ModContext *mc, int &which);


//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == LAG_INTERFACE) 
				return (ILagMod*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 

		void	fnPaint();
		void	fnSetReference();
		void	fnReset();

		void	fnAddForce(INode *node);
		void	fnRemoveForce(int whichNode);
		int		fnNumberVertices();

		void	fnSelectVertices(BitArray *selList, BOOL updateViews);
		BitArray *fnGetSelectedVertices();

		float	fnGetVertexWeight(int index);
		void	fnSetVertexWeight(Tab<int> *indexList, Tab<float> *values);


		void	fnSetEdgeList(BitArray *selList, BOOL updateViews);
		BitArray *fnGetEdgeList();

		void	fnAddSingleSpringFromSelection(int flag,BOOL addDupes);
		void	AddSingleSpringFromSelection(LagModData *lmd, int flag,BOOL addDupes);

		void	fnAddSpring(int a, int b, int flag,BOOL addDupes);
		void	AddSpring(LagModData *lmd, int a, int b, int flag,BOOL addDupes);


/*		void	fnDeleteSpring(int a, int b);
		void	fnDeleteSpring(int index);
		void	DeleteSpring(LagModData *lmd, int a, int b);
		void	DeleteSpring(LagModData *lmd, int index);

		void	fnSetSpringFlag(int index, int flag);
		void	SetSpringFlag(LagModData *lmd,int index, int flag);
		int		fnGetSpringFlag(int index);
		int		GetSpringFlag(LagModData *lmd,int index);
*/
		void	fnRemoveAllSprings();
		void	RemoveAllSprings(LagModData *lmd);

		BOOL	IsDupe(LagModData *lmd,int a, int b);

		void	fnAddSpringButton();
		void	fnRemoveSpringButton();
		void	fnOptionButton();
		void	fnSimpleSoftButton();

		void	fnRemoveSpring(int a);
		void	RemoveSpring(LagModData *lmd,int a);
		void	fnRemoveSpring(int a,int b);
		void	RemoveSpring(LagModData *lmd,int a,int b);
		void	fnRemoveSpringByIndex(int index);
		void	RemoveSpringByIndex(LagModData *lmd,int index);

		int		fnNumberSprings();
		int		NumberSprings(LagModData *lmd);

		int		fnGetSpringGroup(int index);
		int		GetSpringGroup(LagModData *lmd,int index);
		void	fnSetSpringGroup(int index, int group);
		void	SetSpringGroup(LagModData *lmd,int index, int group);

		float	fnGetSpringLength(int index);
		float	GetSpringLength(LagModData *lmd,int index);
		void	fnSetSpringLength(int index,float dist);
		void	SetSpringLength(LagModData *lmd,int index,float dist);

		int		fnGetIndex(int a, int b);
		int		GetIndex(LagModData *lmd,int a, int b);

		BitArray *occuppiedList;




	};

//--- ClassDescriptor and class vars ---------------------------------


		



class LagNodeNotify  : public TVNodeNotify 
{
public:
LagMod *s;
LagNodeNotify(LagMod *smod)
	{
	s = smod;
	}

RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
	{
	if (message == REFMSG_CHANGE) 
		{
		if (partID & PART_TM)
			{
			if (s->ip)
				s->pblock2->SetValue(lag_referenceframe,0,s->ReferenceFrame/GetTicksPerFrame());
			}
		}
	return REF_SUCCEED ;
	}
};



class LagDeformer : public Deformer {
	public:		

		LagModData *lmd;
		LagMod *mod;
		LagDeformer(LagMod *m, LagModData *l) {mod = m; lmd = l;}
		Point3 Map(int i, Point3 p);
	};

class LastFrameLagDeformer : public Deformer {
	public:		

		LagModData *lmd;
		LagMod *mod;
		LastFrameLagDeformer(LagMod *m, LagModData *l) {mod = m; lmd = l;}
		Point3 Map(int i, Point3 p);
	};


//class LagClassDesc:public ClassDesc {
class LagClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE);// { return new LagMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_LAGMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(LAZYID); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Flex"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};


class LagModContextEnumProc:public ModContextEnumProc
{
public:
	Tab<ModContext*> modList;
	BOOL proc(ModContext *mc)
		{
		modList.Append(1,&mc);
		return TRUE;
		}


};

class LagModEnumProc : public ModContextEnumProc {
public:
	LagMod *lm;
	LagModEnumProc(LagMod *l)
		{
		lm = l;
		lm->lmdData.ZeroCount();
		}
private:
	BOOL proc (ModContext *mc);
};


static LagClassDesc lagDesc;



#endif // __ILAG__H
