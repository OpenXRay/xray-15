/**********************************************************************
 *<
	FILE: PointCache.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "PointCache.h"
#include "modstack.h"
#include "macrorec.h"
#include "simpobj.h"

#define PBLOCK_REF	0



#define A_RENDER			A_PLUGIN1


class PointCacheBase : public Modifier {

	protected:
		// I have to store these off and save them since the modifier loads first then the enable state
	    BOOL pcacheEnabled;  // this is the state of the enable state of modifier at save time
		BOOL pcacheEnabledInRender; // this is the state of the enablerender state of modifier at save time

	public:

		BOOL isPCacheEnabled() { return pcacheEnabled;}
		BOOL isPCacheEnabledInRender() { return pcacheEnabledInRender;}


 		int errorMessage;

		BOOL tempDisable;
		BOOL beforeCache, afterCache;
//point cache version start at -100
//particle cache version start at -200
		TimeValue lastCount;
		int version;
		BOOL staticCount;
		int fileVersion;
		Tab<int> pointCountList;
		int maxCount;
		int ComputeStartOffset(int index);

		SimpleParticle *spobj;

		TSTR extension;
		TSTR extensionWildCard;
		TSTR loadName;
		// Parameter block
		IParamBlock2	*pblock;	//ref 0
		static IObjParam *ip;			//Access to the interface
		HWND hWnd;
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
		//TODO: Add the channels that the modifier needs to perform its modification
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}
		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		

		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);

		// Automatic texture support
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return POINTCACHE_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}
		
		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags);

		void DeleteThis() { delete this; }		
		//Constructor/Destructor
		PointCacheBase();
		~PointCacheBase();	
		
//point cache
		Point3 *cFrame;
		Point3 *nFrame;
		Point3 *firstFrame;
		int cTime,nTime;
		FILE *outFile;
		void RecordToFile(HWND hWnd);
		void SaveFile(HWND hWnd);
		void OpenFile(HWND hWnd);
		void SetupButtons(HWND hWnd);

		BOOL recordMode;
		BOOL relativeOffset;
		float strength;

		int numberPoints;

		int cacheSamples, cacheNumberPoints;
		int cacheNumberFrames;
		float per;
		TimeValue recordTime;
		void DisableModsBelow(BOOL enable);
		FILE* OpenFile(TSTR &name);

		TimeValue ComputeIndex(TimeValue t);

		void InvalidateUI();
};


class PointCache : public PointCacheBase, public IPointCache
	{
//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
	BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == POINTCACHE_INTERFACE) 
				return (IPointCache*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 

//published functions
	void	fnRecord();
	void	fnSetCache();
	void	fnEnableMods();
	void	fnDisableMods();

	};

class PointCacheWSM : public PointCacheBase, public IPointCacheWSM
	{
public:
	PointCacheWSM();
	~PointCacheWSM();	


	void InvalidateUI();
	RefTargetHandle Clone( RemapDir &remap );
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return GetString(IDS_POINTCACHE_NAMEBINDING); }

	SClass_ID SuperClassID() { return WSM_CLASS_ID; }
	Class_ID ClassID() { return POINTCACHEWSM_CLASS_ID; }

//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == POINTCACHEWSM_INTERFACE) 
				return (IPointCacheWSM*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 
//published functions
	void	fnRecord();
	void	fnSetCache();
	void	fnEnableMods();
	void	fnDisableMods();

	};

class ParticleData
	{
public:
	Point3 pos, vel;
	TimeValue age;
	};

class ParticleCache;

class ParticleCacheField : public CollisionObject {
	public:	
		ParticleCacheField()
			{
			}
		TimeValue initialT;
		ParticleCache *mod;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index, float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject() {return NULL;}
	};



class ParticleCache : public PointCacheBase, public IParticleCache
	{
public:

	BOOL fastCache;
	ParticleCache();
	~ParticleCache();	

	ParticleCacheField cacheField;

	void InvalidateUI();
	RefTargetHandle Clone( RemapDir &remap );
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
//	void RecordToFile(HWND hWnd);

	int RenderBegin(TimeValue t, ULONG flags);
	int RenderEnd(TimeValue t, ULONG flags);

	void ReloadCache(TimeValue t);

	ParticleData *cFrame;
	ParticleData *nFrame;
	ParticleData *firstFrame;


	SClass_ID SuperClassID() { return WSM_CLASS_ID; }
	Class_ID ClassID() { return PARTICLECACHE_CLASS_ID; }
		// From Animatable
	TCHAR *GetObjectName() { return GetString(IDS_PARTICLECACHE_NAMEBINDING); }


//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == PARTICLECACHE_INTERFACE) 
				return (IParticleCache*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 
//published functions
	void	fnRecord();
	void	fnSetCache();
	void	fnEnableCache();
	void	fnDisableMods();

	};

class PointCacheClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE); 

	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return POINTCACHE_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("PointCache"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};


class PointCacheWSMClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE); 

	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return POINTCACHEWSM_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("PointCacheWSM"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};


class ParticleCacheClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE); 

	const TCHAR *	ClassName() {return GetString(IDS_PARTICLECACHE);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return PARTICLECACHE_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("ParticleCache"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};


static PointCacheClassDesc PointCacheDesc;
ClassDesc2* GetPointCacheDesc() {return &PointCacheDesc;}

static PointCacheWSMClassDesc PointCacheWSMDesc;
ClassDesc2* GetPointCacheWSMDesc() {return &PointCacheWSMDesc;}


static ParticleCacheClassDesc ParticleCacheDesc;
ClassDesc2* GetParticleCacheDesc() {return &ParticleCacheDesc;}

static FPInterfaceDesc pointcache_interface(
    POINTCACHE_INTERFACE, _T("pointCache"), 0, &PointCacheDesc, FP_MIXIN,
		pointcache_record, _T("record"), 0, TYPE_VOID, 0, 0,
		pointcache_setcache, _T("setCache"), 0, TYPE_VOID, 0, 0,
		pointcache_enablemods, _T("enableMods"), 0, TYPE_VOID, 0, 0,

		pointcache_disablemods, _T("disableMods"), 0, TYPE_VOID, 0, 0,


      end
      );

static FPInterfaceDesc pointcachewsm_interface(
    POINTCACHEWSM_INTERFACE, _T("pointCacheWSM"), 0, &PointCacheWSMDesc, FP_MIXIN,
		pointcache_record, _T("record"), 0, TYPE_VOID, 0, 0,
		pointcache_setcache, _T("setCache"), 0, TYPE_VOID, 0, 0,
		pointcache_enablemods, _T("enableMods"), 0, TYPE_VOID, 0, 0,

		pointcache_disablemods, _T("disableMods"), 0, TYPE_VOID, 0, 0,


      end
      );

static FPInterfaceDesc particlecache_interface(
    PARTICLECACHE_INTERFACE, _T("particleCache"), 0, &ParticleCacheDesc, FP_MIXIN,
		pointcache_record, _T("record"), 0, TYPE_VOID, 0, 0,
		pointcache_setcache, _T("setCache"), 0, TYPE_VOID, 0, 0,
		pointcache_enablemods, _T("enableMods"), 0, TYPE_VOID, 0, 0,
		pointcache_disablemods, _T("disableMods"), 0, TYPE_VOID, 0, 0,

      end
      );



void *	PointCacheClassDesc::Create(BOOL loading) 
	{
	AddInterface(&pointcache_interface);
	return new PointCache();
	}

void *	PointCacheWSMClassDesc::Create(BOOL loading) 
	{
	AddInterface(&pointcachewsm_interface);
	return new PointCacheWSM();
	}

void *	ParticleCacheClassDesc::Create(BOOL loading) 
	{
	AddInterface(&particlecache_interface);
	return new ParticleCache();
	}


FPInterfaceDesc* IPointCache::GetDesc()
	{
	 return &pointcache_interface;
	}

FPInterfaceDesc* IPointCacheWSM::GetDesc()
	{
	 return &pointcachewsm_interface;
	}

FPInterfaceDesc* IParticleCache::GetDesc()
	{
	 return &particlecache_interface;
	}


class sMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}



class PointCachePBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		PointCache* p = (PointCache*)owner;

		switch (id)
		{
			case pb_disable_mods:
				p->DisableModsBelow(!v.i);
				break;
				

		}

	}
};

static PointCachePBAccessor pointCache_accessor;


static ParamBlockDesc2 pointcache_param_blk ( pointcache_params, _T("params"),  0, &PointCacheDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_POINTPANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_time, 			_T("time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_TIME,	IDC_TIME_SPIN, 1.0f, 
		end,
	pb_start_time, 			_T("start_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_START_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STIME,	IDC_STIME_SPIN, 1.0f, 
		end,	
	pb_end_time, 			_T("end_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_END_TIME, 
		p_default, 		100.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ETIME,	IDC_ETIME_SPIN, 1.0f, 
		end,
		
	pb_samples, 		_T("samples"), 		TYPE_INT, 	P_RESET_DEFAULT, 	IDS_SAMPLES, 
		p_default, 		1, 
		p_range, 		1,10, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SAMPLES,	IDC_SAMPLES_SPIN, 1.0f, 
		end,	

/*	pb_disable_mods, 	_T("disable_mods"), 	TYPE_BOOL, 		0,				IDS_DISABLE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DISABLE_STACK_CHECK, 
		p_accessor,		&pointCache_accessor,
		end, 
*/
	pb_cache_file, 	_T("cache_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,				IDS_CACHEFILE,
		end, 
/*	pb_record_file, 	_T("record_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,			IDS_RECORDFILE,
		end, 
*/
	pb_relative, 	_T("relativeOffset"), 	TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_RELATIVE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RELATIVE_CHECK, 
		p_enable_ctrls,	1, pb_strength,
		end, 
	pb_strength, 			_T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_STRENGTH, 
		p_default, 		1.0f, 
		p_range, 		-10.0f,10.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STR,	IDC_STR_SPIN, 0.1f, 
		end,	


	end
	);


static ParamBlockDesc2 pointcachewsm_param_blk ( pointcache_params, _T("params"),  0, &PointCacheWSMDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_POINTPANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_time, 			_T("time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_TIME,	IDC_TIME_SPIN, 1.0f, 
		end,
	pb_start_time, 			_T("start_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_START_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STIME,	IDC_STIME_SPIN, 1.0f, 
		end,	
	pb_end_time, 			_T("end_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_END_TIME, 
		p_default, 		100.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ETIME,	IDC_ETIME_SPIN, 1.0f, 
		end,
		
	pb_samples, 		_T("samples"), 		TYPE_INT, 	P_RESET_DEFAULT, 	IDS_SAMPLES, 
		p_default, 		1, 
		p_range, 		1,10, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SAMPLES,	IDC_SAMPLES_SPIN, 1.0f, 
		end,	

/*	pb_disable_mods, 	_T("disable_mods"), 	TYPE_BOOL, 		0,				IDS_DISABLE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DISABLE_STACK_CHECK, 
		p_accessor,		&pointCache_accessor,
		end, 
*/
	pb_cache_file, 	_T("cache_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,				IDS_CACHEFILE,
		end, 
/*	pb_record_file, 	_T("record_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,			IDS_RECORDFILE,
		end, 
*/
	pb_relative, 	_T("relativeOffset"), 	TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_RELATIVE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RELATIVE_CHECK, 
		p_enable_ctrls,	1, pb_strength,
		end, 
	pb_strength, 			_T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_PW_STRENGTH, 
		p_default, 		1.0f, 
		p_range, 		-10.0f,10.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STR,	IDC_STR_SPIN, 0.1f, 
		end,	


	end
	);


static ParamBlockDesc2 particlecache_param_blk ( pointcache_params, _T("params"),  0, &ParticleCacheDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PARTPANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_time, 			_T("time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_TIME,	IDC_TIME_SPIN, 1.0f, 
		end,
	pb_start_time, 			_T("start_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_START_TIME, 
		p_default, 		0.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STIME,	IDC_STIME_SPIN, 1.0f, 
		end,	
	pb_end_time, 			_T("end_time"), 		TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_END_TIME, 
		p_default, 		100.0f, 
		p_range, 		-9999999.0f,9999999.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ETIME,	IDC_ETIME_SPIN, 1.0f, 
		end,
		
	pb_samples, 		_T("samples"), 		TYPE_INT, 	P_RESET_DEFAULT, 	IDS_SAMPLES, 
		p_default, 		1, 
		p_range, 		1,10, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SAMPLES,	IDC_SAMPLES_SPIN, 1.0f, 
		end,	

/*	pb_disable_mods, 	_T("enable_cache"), 	TYPE_BOOL, 		0,				IDS_DISABLE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_ENABLE_CHECK, 
		p_accessor,		&pointCache_accessor,
		end, 
*/
	pb_cache_file, 	_T("cache_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,				IDS_CACHEFILE,
		end, 

/*	pb_record_file, 	_T("record_file"), 	TYPE_STRING, 		P_RESET_DEFAULT,			IDS_RECORDFILE,
		end, 
*/
/*	pb_relative, 	_T("relativeOffset"), 	TYPE_BOOL, 		0,				IDS_RELATIVE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RELATIVE_CHECK, 
		p_enable_ctrls,	1, pb_strength,
		end, 
	pb_strength, 			_T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_PW_STRENGTH, 
		p_default, 		1.0f, 
		p_range, 		-10.0f,10.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STR,	IDC_STR_SPIN, 0.1f, 
		end,	
*/
	pb_fastcache, 	_T("recomputeOnChange"), 	TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_FASTCACHE,
		p_default, 		FALSE, 
		end, 


	end
	);



IObjParam *PointCacheBase::ip			= NULL;


class PointCacheParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		PointCacheBase *mod;		
		PointCacheParamsMapDlgProc(PointCacheBase *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


BOOL PointCacheParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{
			mod->SetupButtons(hWnd);
			mod->hWnd = hWnd;

			 if (mod->errorMessage != 0)
				{

				if ((mod->errorMessage == 3) && (mod->outFile!= NULL))
					mod->errorMessage = 0;

				if (mod->errorMessage == 1)
					{
					TSTR name(GetString(IDS_PW_ERROR));
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					}
				else if (mod->errorMessage == 2)
					{
					TSTR name(GetString(IDS_PW_ERROR2));
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					}
				else if (mod->errorMessage == 3)
					{
					TSTR name(GetString(IDS_PW_ERROR3));
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					}
				

//				mod->errorMessage = 0;
				}
			if (mod->ClassID() == PARTICLECACHE_CLASS_ID)
				{
				ShowWindow(GetDlgItem(hWnd,IDC_RELATIVE_CHECK),SW_HIDE);
				ShowWindow(GetDlgItem(hWnd,IDC_STR),SW_HIDE);
				ShowWindow(GetDlgItem(hWnd,IDC_STR_SPIN),SW_HIDE);
				ShowWindow(GetDlgItem(hWnd,IDC_STRTEXT),SW_HIDE);
				}


			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

/*				case IDC_SAVE:
					{
					mod->SaveFile(hWnd);
					break;
					}
*/
				case IDC_LOAD:
					{

					mod->OpenFile(hWnd);
					mod->SetupButtons(hWnd);
					break;
					}
				case IDC_RECORD:
					{
					TCHAR *fname;
					Interval iv;
					mod->pblock->GetValue(pb_cache_file,0,fname,iv);
					if (!fname)
						{
						macroRecorder->FunctionCall(_T("$.modifiers[#Point_Cache].SetCache"), 0, 0);
						macroRecorder->FunctionCall(_T("$.modifiers[#Point_Cache].Record"), 0, 0);
						}
					else
						macroRecorder->FunctionCall(_T("$.modifiers[#Point_Cache].Record"), 0, 0);
					mod->RecordToFile(hWnd);
					mod->SetupButtons(hWnd);
					break;
					}
				case IDC_ENABLE:
					{
					mod->DisableModsBelow(TRUE);
					macroRecorder->FunctionCall(_T("$.modifiers[#Point_Cache].EnableMods"), 0, 0);

					break;
					}
				case IDC_DISABLE:
					{
					mod->DisableModsBelow(FALSE);
					macroRecorder->FunctionCall(_T("$.modifiers[#Point_Cache].DisableMods"), 0, 0);

					break;
					}


				}
			break;
			}



		}
	return FALSE;
	}



//--- PointCache -------------------------------------------------------
PointCacheBase::PointCacheBase()
{
pcacheEnabled = TRUE;  
pcacheEnabledInRender = TRUE; 


errorMessage = 0;
tempDisable = FALSE;
beforeCache = FALSE;
afterCache = FALSE;
lastCount = -1;
version = -100;
spobj = NULL;

extension.printf("pts");
extensionWildCard.printf("*.pts");
loadName.printf("%s",GetString(IDS_PW_CACHEFILES));
hWnd=NULL;
PointCacheDesc.MakeAutoParamBlocks(this);
cFrame = NULL;
nFrame = NULL;
firstFrame = NULL;
outFile  = NULL;
recordMode = FALSE;

Interval iv = GetCOREInterface()->GetAnimRange();
float startTime = iv.Start()/GetTicksPerFrame();
pblock->SetValue(pb_start_time,0,startTime);
float endTime = iv.End()/GetTicksPerFrame();
pblock->SetValue(pb_end_time,0,endTime);

}

PointCacheBase::~PointCacheBase()
{
if (cFrame) delete [] cFrame;
cFrame = NULL;
if (nFrame) delete [] nFrame;
nFrame = NULL;
if (firstFrame) delete [] firstFrame;
firstFrame = NULL;

if (outFile) fclose(outFile);
}


TimeValue PointCacheBase::ComputeIndex(TimeValue t)
{
float startTime;
pblock->GetValue(pb_time,0,startTime,FOREVER);

//load up cache data
int tps = GetTicksPerFrame();
float ft = (float)t/(float)tps;
ft = ft - startTime;
ft = ft * cacheSamples;
int it = (int) ft;
if (it >= cacheNumberFrames) it = cacheNumberFrames-1;
if (it < 0) it = 0;
return it;
}
Interval PointCacheBase::LocalValidity(TimeValue t)
{
	Interval iv(t,t);
	return iv;
/*
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
		return NEVER;
*/
}

RefTargetHandle PointCacheBase::Clone(RemapDir& remap)
{
	PointCacheBase* newmod = new PointCacheBase();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}

void PointCacheBase::SetupButtons(HWND hWnd)
{

if (hWnd == NULL) return; // LAM - 4/22/03
TCHAR *fname;
Interval iv;
/*
pblock->GetValue(pb_record_file,0,fname,iv);
ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_SAVE));
if (fname)
	{
	TSTR name(fname);
	TSTR path, file;
	SplitPathFile(name,&path, &file);
	iBut->SetText(file);
	}
ReleaseICustButton(iBut);
*/

pblock->GetValue(pb_cache_file,0,fname,iv);
//iBut = GetICustButton(GetDlgItem(hWnd,IDC_LOAD));
if (fname)
	{
	TSTR name(fname);
	TSTR path, file;
	SplitPathFile(name,&path, &file);
	HWND hTextWnd = GetDlgItem(hWnd,IDC_CACHENAME);
	SetWindowText(  hTextWnd,file);
//	iBut->SetText(file);

	}
else
	{
	TSTR file(GetString(IDS_NONE));
	HWND hTextWnd = GetDlgItem(hWnd,IDC_CACHENAME);
	SetWindowText(  hTextWnd,file);

	}
//ReleaseICustButton(iBut);
INode *node;
int nodeCount = 0;
INodeTab nodes;
if (!ip)
	{
	sMyEnumProc dep;              
	EnumDependents(&dep);
	node = dep.Nodes[0];
	nodeCount = dep.Nodes.Count();
	}
else
	{
	ModContextList mcList;
	ip->GetModContexts(mcList,nodes);
	assert(nodes.Count());
	node = nodes[0];
	nodeCount = nodes.Count();
		
	}

Object* obj = node->GetObjectRef();


BOOL enable;

if ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	enable = TRUE;
else enable = FALSE;

ICustButton *iEnableBut = GetICustButton(GetDlgItem(hWnd,IDC_ENABLE));
iEnableBut->Enable(enable);
ReleaseICustButton(iEnableBut);

iEnableBut = GetICustButton(GetDlgItem(hWnd,IDC_DISABLE));
iEnableBut->Enable(enable);
ReleaseICustButton(iEnableBut);
nodes.DisposeTemporary();

}

// LAM - 4/21/03 - set up to return name of file actually opened in 'name' for use by EnumFiles
FILE* PointCacheBase::OpenFile(TSTR &name)
{
	FILE* tempFile = NULL;
	tempFile = fopen(name,"rb");
	if (tempFile) 
		return tempFile;
	else
		{
//split name from path
//get mesh path
		TSTR path, file;
		SplitPathFile(name,&path, &file);
		TCHAR *meshPath = GetCOREInterface()->GetDir(APP_IMPORT_DIR);
//put them back togther
		name.printf("%s\\%s",meshPath,file);
		tempFile = fopen(name,"rb");
		}
	return tempFile;
}
void PointCacheBase::DisableModsBelow(BOOL enable)
{
	int				i;
	SClass_ID		sc;
	IDerivedObject* dobj;

// return the indexed modifier in the mod chain
	INode *node;
	INodeTab nodes;
	if (!ip)
		{
		sMyEnumProc dep;              
		EnumDependents(&dep);
		node = dep.Nodes[0];
		}
	else
		{
		ModContextList mcList;
		ip->GetModContexts(mcList,nodes);
		assert(nodes.Count());
		node = nodes[0];
		}
	BOOL found = TRUE;

// then osm stack
	Object* obj = node->GetObjectRef();
	int ct = 0;

	if ((dobj = node->GetWSMDerivedObject()) != NULL)
		{
		for (i = 0; i < dobj->NumModifiers(); i++)
			{
			Modifier *m = dobj->GetModifier(i);
			BOOL en = m->IsEnabled();
			BOOL env = m->IsEnabledInViews();
			if (!enable)
				{
				if (!found)
					{
					m->DisableMod();
					}
				}
			else 
				{
				if (!found)
					{
					m->EnableMod();
					}
				}
			if (this == dobj->GetModifier(i))
				found = FALSE;

			}
		}

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;

		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (i = 0; i < dobj->NumModifiers(); i++)
				{
				TSTR name;
				Modifier *m = dobj->GetModifier(i);
				m->GetClassName(name);


				BOOL en = m->IsEnabled();
				BOOL env = m->IsEnabledInViews();
				if (!enable)
					{
					if (!found)
						{
						m->DisableMod();
						}
					}
				else 
					{
					if (!found)
						{
						m->EnableMod();
						}
					}
				if (this == dobj->GetModifier(i))
					found = FALSE;


				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			sc = dobj->SuperClassID();
			}
		}


nodes.DisposeTemporary();

GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
NotifyDependents(FOREVER,PART_ALL,REFMSG_NUM_SUBOBJECTTYPES_CHANGED);


}



void PointCacheBase::RecordToFile(HWND hWnd)
{
TCHAR *fname;
Interval iv;
pblock->GetValue(pb_cache_file,0,fname,iv);
if (!fname)
	{
	SaveFile(hWnd);
	}
pblock->GetValue(pb_cache_file,0,fname,iv);
if (fname)
	{
//if out file close it
	if (outFile)
		{
		fclose(outFile);
		outFile = NULL;
		}
//create the outfile
	float s,e;
	int samples;
	int tps;
	Interval iv;
	//get the start time
	pblock->GetValue(pb_start_time,0,s,iv);
	//get the end time
	pblock->GetValue(pb_end_time,0,e,iv);
	//get the samples
	pblock->GetValue(pb_samples,0,samples,iv);
	tps = GetTicksPerFrame();

	s = s *tps;
	e = e*tps;
	int sampleRate = tps/samples;
		
	//loop through time
	if (s==e) e += tps;

    sMyEnumProc dep;              
	EnumDependents(&dep);
	if (spobj)
		{
		tempDisable = TRUE;
//		spobj->valid = FALSE;
		spobj->tvalid = s+tps*4;
		spobj->valid = FALSE;
		spobj->UpdateParticles(s+tps*3, dep.Nodes[0]);

		tempDisable = FALSE;
		}

	else if (GetCOREInterface()->GetTime() == s)
		{
		tempDisable = TRUE;
		if (dep.Nodes[0])
			dep.Nodes[0]->EvalWorldState(s+tps);
		tempDisable = FALSE;
		}
		
	outFile = fopen(fname,"wb");

	if (outFile == NULL)
		{
		TSTR error;
		error.printf("%s",GetString(IDS_PW_OPENERROR));
		MessageBox(GetCOREInterface()->GetMAXHWnd(),error,NULL,MB_OK|MB_ICONERROR);	
		return;
		}
	
	if (spobj)
		numberPoints = spobj->parts.Count();

	//write the header
	//write point count
//this is now the version I was stupid and did not put a version number in the file now I want to change it
//since the number of cache point is now dynamic I am gonna overload this slot and use negative version 
//numbers to signify the difference
	fwrite(&version,sizeof(version),1,outFile);
	cacheNumberPoints = numberPoints;
	//write samples perframe
	fwrite(&samples,sizeof(samples),1,outFile);

	cacheNumberFrames=0;
	if (e > s)
		{
		for (int i = s; i <= e; i+=tps)
			cacheNumberFrames++;
		}
	else 
		{
		for (int i = s; i >= e; i-=tps)
			cacheNumberFrames++;
		}
	cacheNumberFrames *= samples;

	fwrite(&cacheNumberFrames,sizeof(cacheNumberFrames),1,outFile);
	
	if (outFile)
		{
//set new range


		TSTR name;

		recordMode = TRUE;
		BOOL failed = FALSE;
		pointCountList.ZeroCount();
		if (e > s)
			{
			for (int i = s; i <= e; i+=tps)
				{
				TimeValue ct = i;
		//loop through samples
				for (int j = 0; j < samples; j++)
					{
					recordTime = ct;
					if (dep.Nodes[0])
						dep.Nodes[0]->EvalWorldState(ct);
					ct += sampleRate;
					}

				int percent = (i*100)/e;
				name.printf("Frame %d of %d",i/tps,(int)e/tps);
				SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767)
					{
					i = e+1;
					failed = TRUE;
					}
				}
			}
		else
			{
			for (int i = s; i >= e; i-=tps)
				{
				TimeValue ct = i;
		//loop through samples
				for (int j = 0; j < samples; j++)
					{
					recordTime = ct;
					if (dep.Nodes[0])
						dep.Nodes[0]->EvalWorldState(ct);
					ct -= sampleRate;
					}

				int percent = (i*100)/e;
				name.printf("Frame %d of %d",i/tps,(int)e/tps);
				SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767)
					{
					i = e-1;
					failed = TRUE;
					}
				}

			}

//now write the dynamic data size
		for (int i = 0; i < pointCountList.Count(); i++)
			{
			int ct = pointCountList[i];
			fwrite(&ct,sizeof(ct),1,outFile);
			}

		if (failed)
			{
			//need to write the correct number of frames back to the file if they cancel
			int ct = pointCountList.Count();
			int offset = sizeof(int) *2;
			fseek(outFile, offset, SEEK_SET);
			fwrite(&ct,sizeof(ct),1,outFile);
			}
		else
			{
			TCHAR *inName = NULL;
			pblock->GetValue(pb_cache_file,0,inName,FOREVER);
			if (inName == NULL)
				{
				pblock->SetValue(pb_cache_file,0,fname);
				ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_LOAD));
				TSTR name(fname);
				TSTR path, file;
				SplitPathFile(name,&path, &file);
				iBut->SetText(file);
				ReleaseICustButton(iBut);

				}
			}

		name.printf(" ");
		SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
		errorMessage = 0;
//close the outfile
		fclose(outFile);


		}

	outFile = NULL;
	recordMode = FALSE;
	}

}

void PointCacheBase::SaveFile(HWND hWnd)
{
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
//fl.Append( GetString(IDS_PW_CACHEFILES));
fl.Append( loadName);
//fl.Append( _T("*.pts"));		
fl.Append( _T(extensionWildCard));		
TSTR title = GetString(IDS_PW_SAVEOBJECT);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = hWnd;
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
//ofn.lpstrDefExt     = _T("pts");
ofn.lpstrDefExt     = _T(extension);
ofn.lpstrTitle      = title;

tryAgain:
if (GetSaveFileName(&ofn)) {
	if (DoesFileExist(fname)) {
		TSTR buf1;
		TSTR buf2 = GetString(IDS_PW_SAVEOBJECT);
		buf1.printf(GetString(IDS_PW_FILEEXISTS),fname);
		if (IDYES!=MessageBox(
			hWnd,
			buf1,buf2,MB_YESNO|MB_ICONQUESTION)) {
			goto tryAgain;
			}
		}
//save stuff here
	pblock->SetValue(pb_cache_file,0,fname);

//	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_SAVE));
//	iBut->SetType(CBT_CHECK);
//	iBut->SetHighlightColor(GREEN_WASH);
//	TSTR name(fname);
//	TSTR path, file;
//	SplitPathFile(name,&path, &file);
//	iBut->SetText(file);
//	ReleaseICustButton(iBut);

//	FILE *file = fopen(fname,_T("wb"));
	}


}

void PointCacheBase::OpenFile(HWND hWnd)
{
if (outFile)
	{	
	fclose(outFile);
	outFile = NULL;
	}

static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
//fl.Append( GetString(IDS_PW_CACHEFILES));
fl.Append( loadName);
//fl.Append( _T("*.pts"));		
fl.Append( _T(extensionWildCard));		
TSTR title = GetString(IDS_PW_LOADOBJECT);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = hWnd;
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
//ofn.lpstrDefExt     = _T("pts");
ofn.lpstrDefExt     = _T(extension);
ofn.lpstrTitle      = title;


if (GetOpenFileName(&ofn)) {
//load stuff here  stuff here
//	FILE *file = fopen(fname,_T("rb"));
	if 	(spobj)
		{
		spobj->valid = FALSE;
		}
	pblock->SetValue(pb_cache_file,0,fname);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	pblock->SetValue(pb_cache_file,0,fname);
	errorMessage = 0;
	TSTR name;
	name.printf(" ");
	SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);


//	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_LOAD));
//	iBut->SetType(CBT_CHECK);
//	iBut->SetHighlightColor(GREEN_WASH);
//	TSTR name(fname);
//	TSTR tpath, tfile;
//	SplitPathFile(name,&tpath, &tfile);
//	iBut->SetText(tfile);
//	ReleaseICustButton(iBut);


	}


}

class CacheDeformer : public Deformer {
	public:	
		PointCacheBase *mod;
		CacheDeformer(PointCacheBase *m ) {mod = m;}
		Point3 Map(int i, Point3 p)
			{
			if ((!mod->cFrame) || (!mod->nFrame) || (!mod->firstFrame))
				return p;
			if (mod->fileVersion == 0)
				{
				if (i<mod->cacheNumberPoints)
					{
					if (mod->relativeOffset)
						p += ((mod->cFrame[i] + (mod->nFrame[i]-mod->cFrame[i]) * mod->per)-mod->firstFrame[i])*mod->strength;
					else p = mod->cFrame[i] + (mod->nFrame[i]-mod->cFrame[i]) * mod->per;
					}
				}
			else
				{
				if (mod->pointCountList[mod->cTime] == mod->pointCountList[mod->nTime])
					{
					if ((i<mod->pointCountList[mod->cTime]) &&(i<mod->pointCountList[mod->nTime]))
						{
						if (mod->relativeOffset)
							{
							p += ((mod->cFrame[i] + (mod->nFrame[i]-mod->cFrame[i]) * mod->per)-mod->firstFrame[i])*mod->strength;
/*							Point3 start, end;
							start = mod->firstFrame[i];
							end = mod->cFrame[i] + (mod->nFrame[i]-mod->cFrame[i]) * mod->per; 
							p += (end-start)*mod->strength;
*/
							}
						else p = mod->cFrame[i] + (mod->nFrame[i]-mod->cFrame[i]) * mod->per;
						}
					}
				else
					{
					if (i<mod->pointCountList[mod->cTime])
						{
						p = mod->cFrame[i];
						}

					}
				}

			return p;
			}
	};

int PointCacheBase::ComputeStartOffset(int index)
{
	int ct = 0;
	for (int i =0; i < (index); i++)
		ct += pointCountList[i];
	return ct;

}
void PointCacheBase::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
//TODO: Add the code for actually modifying the object
	//set new range
    sMyEnumProc dep;              
	EnumDependents(&dep);

	if (dep.Nodes.Count()>1)
		{
		if ( (hWnd) && (errorMessage != 1))
			{
			TSTR name(GetString(IDS_PW_ERROR));
			SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
			errorMessage = 1;
			InvalidateUI();
			}

		}
	numberPoints = os->obj->NumPoints();
//check if in record mode
	if (recordMode)
		{
		if ((outFile) && (recordTime == t))
			{
//loop through points and append to file
			Point3 p;
			pointCountList.Append(1,&numberPoints);
			for (int i = 0; i < numberPoints; i++)
				{
				p = os->obj->GetPoint(i);
//				if (i<cacheNumberPoints)
				fwrite(&p,sizeof(p),1,outFile);
//DebugPrint("time %d  index %d   %f %f %f\n",t/160,i,p.x,p.y,p.z);
				}
			}
		recordTime++;
		}
	else if (!tempDisable)
		{
		TCHAR *fname;
		Interval iv;
		pblock->GetValue(pb_cache_file,0,fname,iv);
		float startTime;
		pblock->GetValue(pb_time,0,startTime,iv);

		pblock->GetValue(pb_relative,0,relativeOffset,iv);
		pblock->GetValue(pb_strength,t,strength,iv);


		if (fname)
			{
//check to see if outfile needs to be opened
			int tps = GetTicksPerFrame();
			if (!outFile)
				{
//				outFile = fopen(fname,"rb");
				TSTR name = fname;
				outFile = OpenFile(name);
//read numberPoints and samples and first frame
				if (outFile)
					{
					fread(&cacheNumberPoints,sizeof(cacheNumberPoints),1,outFile);
					if (cacheNumberPoints < 0)
						fileVersion = cacheNumberPoints;
					else fileVersion = 0;
					fread(&cacheSamples,sizeof(cacheSamples),1,outFile);
					fread(&cacheNumberFrames,sizeof(cacheNumberFrames),1,outFile);
					if (fileVersion < 0)
						{
//read in the frame sizes
						int offset = sizeof(int) *cacheNumberFrames;
						pointCountList.SetCount(cacheNumberFrames);
						fseek(outFile, -offset, SEEK_END);
						maxCount = -1;
						for (int i = 0; i < pointCountList.Count(); i++)
							{
							int ct;
							fread(&ct,sizeof(ct),1,outFile);
							pointCountList[i] = ct;
							if (ct > maxCount) maxCount = ct;
							cacheNumberPoints = maxCount;
							}
						}
					else maxCount = cacheNumberPoints;



//					if (pFrame) delete [] pFrame;
					if (cFrame) delete [] cFrame;
					cFrame = NULL;
					if (nFrame) delete [] nFrame;
					nFrame = NULL;
					if (firstFrame) delete [] firstFrame;
					firstFrame = NULL;
	
					cTime = -999999;
					nTime = -999999;
		
//					pFrame = new Point3[cacheNumberPoints];
					cFrame = new Point3[maxCount];
					nFrame = new Point3[maxCount];
					firstFrame = new Point3[maxCount];
					

					int offset = (sizeof(int) *3) ;
					fseek(outFile, offset, SEEK_SET);

					if (fileVersion == 0)
						fread(firstFrame,sizeof(Point3)*cacheNumberPoints,1,outFile);
					else fread(firstFrame,sizeof(Point3)*pointCountList[0],1,outFile);
					}
//fread first frame


				}
			if ( (outFile) && (cFrame) && (nFrame) && (firstFrame))
				{
//load up cache data

				float ft = (float)t/(float)tps;
				ft = ft - startTime;
				ft = ft * cacheSamples;
/*				int it = (int) ft;
				if (it >= cacheNumberFrames) it = cacheNumberFrames-1;
				if (it < 0) it = 0;
*/
				int it = ComputeIndex(t);
				int nit = it + 1;


				if (nit >= cacheNumberFrames) nit = cacheNumberFrames-1;
				if (nit < 0) nit = 0;

//				int frames = cacheNumberFrames/cacheSamples;

				int offset;
				if (fileVersion == 0)
					offset= it* cacheNumberPoints;
				else offset = ComputeStartOffset(it);
				
				float inc = 1.0f/(float)(cacheSamples);
				float gt = (it * inc)* cacheSamples;
				float ht = (it * inc + inc)* cacheSamples;
				per = (ft-gt)/(ht-gt);
				if (per < 0.0f) per = 0.0f;
				if (per > 1.0f) per = 1.0f;
				offset = (sizeof(int) *3) + (offset*sizeof(Point3));
				fseek(outFile, offset, SEEK_SET);
//	DebugPrint("time %d(%d)   frame %f  per %f it %d gt %f ht %f \n",t,t/tps,ft,per,it,gt,ht);

				if (it == nTime)
					{
					memcpy(cFrame,nFrame,sizeof(Point3)*cacheNumberPoints);
					if (fileVersion == 0)
						fseek(outFile, sizeof(Point3)*cacheNumberPoints, SEEK_CUR);
					else fseek(outFile, sizeof(Point3)*pointCountList[it], SEEK_CUR);
					if (nit != it)
						{
						if (fileVersion == 0)
							fread(nFrame,sizeof(Point3)*cacheNumberPoints,1,outFile);
						else fread(nFrame,sizeof(Point3)*pointCountList[nit],1,outFile);
						}
					else
						{
						memcpy(nFrame,cFrame,sizeof(Point3)*cacheNumberPoints);
						}
					}
				else	
				
					{
					if (fileVersion == 0)
						fread(cFrame,sizeof(Point3)*cacheNumberPoints,1,outFile);
					else fread(cFrame,sizeof(Point3)*pointCountList[it],1,outFile);
					if (nit != it)
						{
						if (fileVersion == 0)
							fread(nFrame,sizeof(Point3)*cacheNumberPoints,1,outFile);
						else fread(nFrame,sizeof(Point3)*pointCountList[nit],1,outFile);
						}
					else
						{
						memcpy(nFrame,cFrame,sizeof(Point3)*cacheNumberPoints);
						}
					}

				nTime = nit;
				cTime = it;

				if ( (hWnd) && 
					 (cTime  < pointCountList.Count()) &&
					 (os->obj->NumPoints() != pointCountList[cTime]) &&
					 (errorMessage != 2)

					 )
					 
					{
					TSTR name(GetString(IDS_PW_ERROR2));
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					errorMessage = 2;
					InvalidateUI();

					}

				CacheDeformer deformer(this);
				os->obj->Deform(&deformer, TRUE);

				if ( errorMessage == 3)
					{
					errorMessage = 0;
					TSTR name;
					name.printf(" ");
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					InvalidateUI();
					}

				}
			else
				{
				if ( (hWnd) && (errorMessage != 3))
					{
					TSTR name(GetString(IDS_PW_ERROR3));
					SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
					errorMessage = 3;
					InvalidateUI();

					}

				}
			}
		}
	Interval iv(t,t);
	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
}

void PointCacheBase::InvalidateUI()
{
pointcache_param_blk.InvalidateUI();
}

void PointCacheBase::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	PointCacheDesc.BeginEditParams(ip, this, flags, prev);

	pointcache_param_blk.SetUserDlgProc(new PointCacheParamsMapDlgProc(this));
	
}

void PointCacheBase::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PointCacheDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	hWnd=NULL;
}


//From ReferenceMaker 
RefResult PointCacheBase::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

//From Object
BOOL PointCacheBase::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void PointCacheBase::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

class ErrorPLCB : public PostLoadCallback {
	public:
		PointCacheBase *obj;
		ErrorPLCB(PointCacheBase *o) {obj=o;}
		void proc(ILoad *iload);
	};

void ErrorPLCB::proc(ILoad *iload)
	{
	TCHAR *fname;
	FILE *tempFile = NULL;
	Interval iv;

	if (obj->pblock && 
		 ( (obj->isPCacheEnabled() && obj->isPCacheEnabledInRender()) ||
		   (obj->isPCacheEnabled() && !obj->isPCacheEnabledInRender()) 
	     )   
	   )
		{
		obj->pblock->GetValue(pb_cache_file,0,fname,iv);
		if (fname)
			{
			TSTR name = fname;
			tempFile = obj->OpenFile(name);
			if (!tempFile)
				{
/*				LAM - 4/21/03 - per conv with PeterW, no need to detect/log missing file here. 
				This will happen through the call to EnumAuxFiles
				if (GetCOREInterface()->IsNetServer()) 
					{
					GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_PW_ERROR3),fname);
					} 
				else 
					{
					TSTR error;
					error.printf("%s %s",GetString(IDS_PW_ERROR3),fname);
					MessageBox(GetCOREInterface()->GetMAXHWnd(),error,NULL,MB_OK|MB_ICONERROR);	
					}
*/
				obj->errorMessage = 3;
					
				}
			if (tempFile) fclose(tempFile);
			}
		}	
	delete this;
	}

#define ENABLE_CHUNK		0x2120
#define ENABLEINRENDER_CHUNK		0x2130

IOResult PointCacheBase::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	iload->RegisterPostLoadCallback(new ErrorPLCB(this));	
	Modifier::Load(iload);

	//TODO: Add code to allow plugin to load its data
	IOResult res = IO_OK;
	ULONG nb;

	pcacheEnabled = TRUE;
	pcacheEnabledInRender = TRUE;

	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{
			case ENABLE_CHUNK:
				iload->Read(&pcacheEnabled,sizeof(pcacheEnabled), &nb);
				break;
			case ENABLEINRENDER_CHUNK:
				iload->Read(&pcacheEnabledInRender,sizeof(pcacheEnabledInRender), &nb);
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
		}


	return IO_OK;
}

IOResult PointCacheBase::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	Modifier::Save(isave);	

	ULONG nb;

	Modifier *m = (PointCacheBase *) this;
	pcacheEnabled = m->IsEnabled();
	pcacheEnabledInRender = m->IsEnabledInRender();

	isave->BeginChunk(ENABLE_CHUNK);
	isave->Write(&pcacheEnabled,sizeof(pcacheEnabled),&nb);
	isave->EndChunk();

	isave->BeginChunk(ENABLEINRENDER_CHUNK);
	isave->Write(&pcacheEnabledInRender,sizeof(pcacheEnabledInRender),&nb);
	isave->EndChunk();

	return IO_OK;
}



void PointCacheBase::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) 
	{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/11/03
	TCHAR *fname;
	FILE *tempFile = NULL;
	Interval iv;
	pblock->GetValue(pb_cache_file,0,fname,iv);
	if (fname) 
		{
		TSTR name = fname;
		tempFile = OpenFile(name); // LAM - 4/21/03 - need resolved file name
		if (!tempFile && (flags&FILE_ENUM_MISSING_ONLY))
			nameEnum.RecordName(fname);
		else if (tempFile && !(flags&FILE_ENUM_MISSING_ONLY)) 
			nameEnum.RecordName(name);

		if (tempFile) fclose(tempFile);

		}

	Modifier::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03
	
	}


void PointCache::fnRecord()
{
RecordToFile(hWnd);
SetupButtons(hWnd);
}
void PointCache::fnSetCache()
{
OpenFile(hWnd);
SetupButtons(hWnd);

}
void PointCache::fnEnableMods()
{
DisableModsBelow(TRUE);

}
void PointCache::fnDisableMods()
{
DisableModsBelow(FALSE);
}


//*************** WSM Components

PointCacheWSM::PointCacheWSM()
{

pcacheEnabled = TRUE;  
pcacheEnabledInRender = TRUE; 


hWnd=NULL;
PointCacheWSMDesc.MakeAutoParamBlocks(this);
cFrame = NULL;
nFrame = NULL;
firstFrame = NULL;

outFile  = NULL;
recordMode = FALSE;

Interval iv = GetCOREInterface()->GetAnimRange();
float startTime = iv.Start()/GetTicksPerFrame();
pblock->SetValue(pb_start_time,0,startTime);
float endTime = iv.End()/GetTicksPerFrame();
pblock->SetValue(pb_end_time,0,endTime);


}

PointCacheWSM::~PointCacheWSM()
{
if (cFrame) delete [] cFrame;
cFrame = NULL;
if (nFrame) delete [] nFrame;
nFrame = NULL;
if (firstFrame) delete [] firstFrame;
firstFrame = NULL;

if (outFile) fclose(outFile);
}

RefTargetHandle PointCacheWSM::Clone(RemapDir& remap)
{
	PointCacheWSM* newmod = new PointCacheWSM();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));

	BaseClone(this, newmod, remap);
	return(newmod);
}

void PointCacheWSM::InvalidateUI()
{
pointcachewsm_param_blk.InvalidateUI();
}

void PointCacheWSM::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	PointCacheWSMDesc.BeginEditParams(ip, this, flags, prev);

	pointcachewsm_param_blk.SetUserDlgProc(new PointCacheParamsMapDlgProc(this));
	
}

void PointCacheWSM::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PointCacheWSMDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	hWnd=NULL;
}



void PointCacheWSM::fnRecord()
{
RecordToFile(hWnd);
SetupButtons(hWnd);
}

void PointCacheWSM::fnSetCache()
{
OpenFile(hWnd);
SetupButtons(hWnd);
}

void PointCacheWSM::fnEnableMods()
{
DisableModsBelow(TRUE);
}

void PointCacheWSM::fnDisableMods()
{
DisableModsBelow(FALSE);
}



//*************** Particle Components

ParticleCache::ParticleCache()
{

pcacheEnabled = TRUE;  
pcacheEnabledInRender = TRUE; 


version = -200;
spobj = NULL;
hWnd=NULL;
ParticleCacheDesc.MakeAutoParamBlocks(this);
cFrame = NULL;
nFrame = NULL;
firstFrame = NULL;

outFile  = NULL;
recordMode = FALSE;
extension.printf("par");
extensionWildCard.printf("*.par");
loadName.printf("%s",GetString(IDS_PW_CACHEFILESPARTS));

}

ParticleCache::~ParticleCache()
{
if (cFrame) delete [] cFrame;
cFrame = NULL;
if (nFrame) delete [] nFrame;
nFrame = NULL;
if (firstFrame) delete [] firstFrame;
firstFrame = NULL;

if (outFile) fclose(outFile);
}


RefTargetHandle ParticleCache::Clone(RemapDir& remap)
{
	ParticleCache* newmod = new ParticleCache();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));

	BaseClone(this, newmod, remap);
	return(newmod);
}

void ParticleCache::InvalidateUI()
{
particlecache_param_blk.InvalidateUI();
}


void ParticleCache::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	ParticleCacheDesc.BeginEditParams(ip, this, flags, prev);

	particlecache_param_blk.SetUserDlgProc(new PointCacheParamsMapDlgProc(this));
	
}

void ParticleCache::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ParticleCacheDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	hWnd=NULL;
}



void ParticleCache::fnRecord()
{
RecordToFile(hWnd);
SetupButtons(hWnd);
}

void ParticleCache::fnSetCache()
{
OpenFile(hWnd);
SetupButtons(hWnd);
}



void ParticleCache::ReloadCache(TimeValue t)
{

if (!outFile) return;

float startTime;
pblock->GetValue(pb_time,0,startTime,FOREVER);

//load up cache data
int tps = GetTicksPerFrame();
float ft = (float)t/(float)tps;
ft = ft - startTime;
/*
t = ft * cacheSamples;
int it = (int) ft;
if (it >= cacheNumberFrames) it = cacheNumberFrames-1;
if (it < 0) it = 0;
*/
int it = ComputeIndex(t);
int nit = it + 1;



if (nit >= cacheNumberFrames) 
	{
    nit = cacheNumberFrames-1;
	}
if (nit < 0) 
	{
    nit = 0;
	}

//				int frames = cacheNumberFrames/cacheSamples;

int offset;
if (fileVersion == 0)
	offset = it* cacheNumberPoints;
else offset = ComputeStartOffset(it);
				
float inc = 1.0f/(float)(cacheSamples);
float gt = (it * inc)* cacheSamples;
float ht = (it * inc + inc)* cacheSamples;
per = (ft-gt)/(ht-gt);
if (per < 0.0f) per = 0.0f;
if (per > 1.0f) per = 1.0f;
offset = (sizeof(int) *3) + (offset*sizeof(ParticleData));
fseek(outFile, offset, SEEK_SET);
//DebugPrint("time %d(%d)   frame %f  per %f it %d gt %f ht %f \n",t,t/tps,ft,per,it,gt,ht);

if (it == nTime)
	{
	memcpy(cFrame,nFrame,sizeof(ParticleData)*cacheNumberPoints);
	if (fileVersion == 0)
		fseek(outFile, sizeof(ParticleData)*cacheNumberPoints, SEEK_CUR);
	else fseek(outFile, sizeof(ParticleData)*pointCountList[it], SEEK_CUR);
	if (nit != it)
		{
		if (fileVersion == 0)
			fread(nFrame,sizeof(ParticleData)*cacheNumberPoints,1,outFile);
		else fread(nFrame,sizeof(ParticleData)*pointCountList[nit],1,outFile);
		}
	else
		{
		memcpy(nFrame,cFrame,sizeof(ParticleData)*cacheNumberPoints);
		}
	}
else	
	{
	if (fileVersion == 0)
		fread(cFrame,sizeof(ParticleData)*cacheNumberPoints,1,outFile);
	else fread(cFrame,sizeof(ParticleData)*pointCountList[it],1,outFile);
	if (nit != it)
		{
		if (fileVersion == 0)
			fread(nFrame,sizeof(ParticleData)*cacheNumberPoints,1,outFile);
		else fread(nFrame,sizeof(ParticleData)*pointCountList[nit],1,outFile);
		}
	else
		{
		memcpy(nFrame,cFrame,sizeof(ParticleData)*cacheNumberPoints);
		}
	}

nTime = nit;
cTime = it;

if (spobj)
	{
	int ct = cacheNumberPoints;
	if (fileVersion < 0) ct = pointCountList[it];
	
//	lastCount = spobj->parts.Count();

	if (spobj->parts.ages.Count() != ct)
		{
		if ( (hWnd) && (errorMessage != 2))
			{
			TSTR name(GetString(IDS_PW_ERROR2));
			SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
			errorMessage = 2;
			InvalidateUI();
			}

		}
	
			
	for (int i =0; i < ct; i++)
		{
		if (i < spobj->parts.ages.Count())
			{
			spobj->parts.ages[i] = cFrame[i].age;
			}

		if ( (fileVersion == 0) || 
			 (pointCountList[cTime] == pointCountList[nTime]) )
			{
			if (i < spobj->parts.points.Count())
				{
				Point3 p = spobj->parts.points[i];
				if (relativeOffset)
					p = firstFrame[i].pos + (((cFrame[i].pos + (nFrame[i].pos-cFrame[i].pos) * per)-firstFrame[i].pos)*strength);
				else p = cFrame[i].pos + (nFrame[i].pos-cFrame[i].pos) * per;

				spobj->parts.points[i] = p;
				}


			if (i < spobj->parts.vels.Count())
				{
				Point3 v = spobj->parts.vels[i];
				if (relativeOffset)
					v = firstFrame[i].vel + (((cFrame[i].vel + (nFrame[i].vel-cFrame[i].vel) * per)-firstFrame[i].vel)*strength);
				else v = cFrame[i].vel + (nFrame[i].vel-cFrame[i].vel) * per;
				spobj->parts.vels[i] = v;
				}
			}
		else
			{
			if (i < spobj->parts.points.Count())
				{
				Point3 p = spobj->parts.points[i];
				
				p = cFrame[i].pos ;

				spobj->parts.points[i] = p;
				}


			if (i < spobj->parts.vels.Count())
				{
				Point3 v = spobj->parts.vels[i];
				v = cFrame[i].vel;
				spobj->parts.vels[i] = v;
				}

			}
			


		}
	}

}


void ParticleCache::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
//TODO: Add the code for actually modifying the object
	//set new range
    sMyEnumProc dep;              
	EnumDependents(&dep);

	if (dep.Nodes.Count()>1)
		{
		if ( (hWnd) && (errorMessage != 1))
			{
			TSTR name(GetString(IDS_PW_ERROR));
			SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
			errorMessage = 1;
			InvalidateUI();

			}

		}

	if (os->obj && os->obj->IsParticleSystem()) 
		{
		
		ParticleObject *obj = GetParticleInterface(os->obj);
		spobj = (SimpleParticle*)os->obj;
		numberPoints = spobj->parts.Count();
//DebugPrint("P count %d\n", numberPoints);
//check if in record mode
		if (recordMode)
			{
//			DisableModsBelow(TRUE);


			recordMode = FALSE;
			tempDisable=TRUE;
			spobj->UpdateParticles(t, node);

			numberPoints = spobj->parts.Count();
			tempDisable=FALSE;
			recordMode = TRUE;

			if ((outFile) && (recordTime == t))
				{
//loop through points and append to file
				Point3 p(0.0f,0.0f,0.0f);
				Point3 v(0.0f,0.0f,0.0f);
				TimeValue age=-1;

				pointCountList.Append(1,&numberPoints);
				for (int i = 0; i < numberPoints; i++)
					{
					if (i <  spobj->parts.points.Count())
						p = spobj->parts.points[i];
					if (i <  spobj->parts.vels.Count())
						v = spobj->parts.vels[i];
					if (i <  spobj->parts.ages.Count())
						age = spobj->parts.ages[i];
//					if (i<cacheNumberPoints)
						{
						fwrite(&p,sizeof(p),1,outFile);
						fwrite(&v,sizeof(v),1,outFile);
						fwrite(&age,sizeof(age),1,outFile);
						}
					}
				}
			recordTime++;
			}
		else  if (!tempDisable) 
			{

//			DisableModsBelow(FALSE);
			TCHAR *fname;
			Interval iv;

			pblock->GetValue(pb_cache_file,0,fname,iv);
			float startTime;
			pblock->GetValue(pb_time,0,startTime,iv);

//			pblock->GetValue(pb_relative,0,relativeOffset,iv);
//			pblock->GetValue(pb_strength,0,strength,iv);
			relativeOffset = FALSE;
			strength = 1.0f;

			pblock->GetValue(pb_fastcache,0,fastCache,iv);
			fastCache = !fastCache;

			if (fname)
				{
//check to see if outfile needs to be opened
				int tps = GetTicksPerFrame();
				if (!outFile)
					{
//				outFile = fopen(fname,"rb");
					TSTR name = fname;
					outFile = OpenFile(name);
//read numberPoints and samples and first frame
					if (outFile)
						{
						cacheNumberPoints = 0;
						cacheSamples = 0;
						cacheNumberFrames = 0;
						fread(&cacheNumberPoints,sizeof(cacheNumberPoints),1,outFile);
						fread(&cacheSamples,sizeof(cacheSamples),1,outFile);
						fread(&cacheNumberFrames,sizeof(cacheNumberFrames),1,outFile);
						if (cacheNumberPoints < 0)
							fileVersion = cacheNumberPoints;
						else fileVersion = 0;
						staticCount = TRUE;
						if (fileVersion < 0)
							{
//read in the frame sizes
							int offset = sizeof(int) *cacheNumberFrames;
							pointCountList.SetCount(cacheNumberFrames);
							fseek(outFile, -offset, SEEK_END);
							maxCount = -1;
							for (int i = 0; i < pointCountList.Count(); i++)
								{
								int ct;
								fread(&ct,sizeof(ct),1,outFile);
								pointCountList[i] = ct;
								if (ct > maxCount) maxCount = ct;
								cacheNumberPoints = maxCount;
								if (i!=0)
									{
									if (pointCountList[i]!=pointCountList[i-1])
										staticCount = FALSE;
									}
								}
							}
						else maxCount = cacheNumberPoints;


//					if (pFrame) delete [] pFrame;
						if (cFrame) delete [] cFrame;
						cFrame = NULL;
						if (nFrame) delete [] nFrame;
						nFrame = NULL;
						if (firstFrame) delete [] firstFrame;
						firstFrame = NULL;
	
						cTime = -999999;
						nTime = -999999;
		
//					pFrame = new Point3[cacheNumberPoints];
						cFrame = new ParticleData[maxCount];
						nFrame = new ParticleData[maxCount];
						firstFrame = new ParticleData[maxCount];

						if (fileVersion == 0)
							fread(firstFrame,sizeof(ParticleData)*cacheNumberPoints,1,outFile);
						else fread(firstFrame,sizeof(ParticleData)*pointCountList[0],1,outFile);
						}
//fread first frame


					}
				if (outFile)
					{
//DebugPrint("Time %d\n",t/160);
					int it = ComputeIndex(t);
					
//DebugPrint("%d PlayBack %d lst %d\n",t/160,pointCountList[it],lastCount);
					if (fastCache)
						{
						if ( (pointCountList[it]!=lastCount) || beforeCache || afterCache)
							{
//						spobj->valid = FALSE;
							int cacheT = ComputeIndex(t);
							beforeCache = FALSE;
							afterCache = FALSE;
							if (cacheT == 0)
								beforeCache = TRUE;
							if (cacheT >= cacheNumberFrames)
								afterCache = TRUE;
						
							}
						else if ((!TestAFlag(A_RENDER)) && (t == GetCOREInterface()->GetTime()))
							{
							int cacheT = ComputeIndex(t);
							beforeCache = FALSE;
							afterCache = FALSE;
							if (cacheT == 0)
								beforeCache = TRUE;
							if (cacheT >= cacheNumberFrames)
								afterCache = TRUE;


							if ( (staticCount) && (!beforeCache) && (!afterCache) )
								{
								if (spobj->tvalid != t)
									{
									spobj->tvalid = t -tps;
									}
								else spobj->valid = TRUE;
	
								}
							else if (  (!beforeCache) && (!afterCache) )
								{
								if (spobj->tvalid != t)
									{
									spobj->tvalid = t -tps;
									if (spobj->parts.ages.Count() >0)
										spobj->parts.ages[0] = 0;
									}
								else spobj->valid = TRUE;
								}

							}
						}
						
					lastCount = pointCountList[it];

		//			if ((!TestAFlag(A_RENDER)) && (t == GetCOREInterface()->GetTime()) && (fastCache))
					//	{
					//	if (pointCountList[it-1]>=0)
					//		spobj->parts.SetCount(pointCountList[it-1],PARTICLE_VELS|PARTICLE_AGES);
					//	ReloadCache(t-GetTicksPerFrame());
					//	}
					cacheField.mod  = this;
					if (fastCache)
						cacheField.initialT  = -999999;

					if (spobj->parts.ages.Count() != lastCount)
						{
						if ( (hWnd) && (errorMessage != 2))
							{
							TSTR name(GetString(IDS_PW_ERROR2));
							SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
							errorMessage = 2;
							InvalidateUI();
							}

						}
					
					obj->ApplyCollisionObject(&cacheField);

					}
				else
					{
					if ( (hWnd) && (errorMessage != 3))
						{
						TSTR name(GetString(IDS_PW_ERROR3));
						SetWindowText(GetDlgItem(hWnd,IDC_PERCENT),name);
						errorMessage = 3;
						InvalidateUI();

						}
					}
				}
			}
		Interval iv(t,t);
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
		}
}

int ParticleCache::RenderBegin(TimeValue t, ULONG flags)
	{
	SetAFlag(A_RENDER);
	return 0;

	}


int ParticleCache::RenderEnd(TimeValue t, ULONG flags)
	{
	ClearAFlag(A_RENDER);
	return 0;
	}



BOOL ParticleCacheField::CheckCollision(TimeValue t,Point3 &p, Point3 &v, float dt, int i, float *ct,BOOL UpdatePastCollide)

{

if (mod->fastCache )
	{
	if ((mod->TestAFlag(A_RENDER)) || (t == GetCOREInterface()->GetTime()) )
		{
		if (t != initialT)
			{
			mod->ReloadCache(t);
			initialT = t;
			}
		}

	}
else
	{

	if (t != initialT)
		{



		mod->ReloadCache(t);
		initialT = t;
		}

	}
if (UpdatePastCollide)
	{
	if (ct) (*ct) = 0;
	}
else
	{
	if (ct) (*ct) = dt;
	}

return TRUE;
	

}
