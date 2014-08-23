/*===========================================================================*\
	FILE: sunlight.cpp

	DESCRIPTION: Sunlight system plugin.

	HISTORY: Created Oct.15 by John Hutchinson
			Derived from the ringarray

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/
/*===========================================================================*\
 | Include Files:
\*===========================================================================*/

#include <max.h>
#include <iparamm2.h>
#include <modstack.h>
#include <INodeGIProperties.h>
#include <naturalLight.h>
#include <IAssembly.h>
#include <IAssemblyMgr.h>
#include <decomp.h>
#include <maxscrpt.h>
#include <Name.h>
#include <maxobj.h>
#include "natLight.h"
#include "suntypes.h"
#include "sunclass.h"
#include "sunlight.h"
#include <marketDefaults.h>
#include <shadgen.h>
#include <icustattribcontainer.h>

extern TCHAR* GetString(int);
extern HINSTANCE hInstance;

const SClass_ID kValidSuperClass = LIGHT_CLASS_ID;
const Class_ID kStandardSunClass = Class_ID(DIR_LIGHT_CLASS_ID, 0);

#define OVERSHOOT TRUE
//#define SUN_RGB Point3(0.88235294f, 0.88235294f, 0.88235294f)  // 225
#define SUN_RGB Point3(1.00f, 0.95f, 0.90f)  // 225

static bool clampColor(Point3& color)
{
	if (color.x < 0.0f)
		color.x = 0.0f;
	else if (color.x > 1.0f)
		color.x = 1.0f;

	if (color.y < 0.0f)
		color.y = 0.0f;
	else if (color.y > 1.0f)
		color.y = 1.0f;

	if (color.z < 0.0f)
		color.z = 0.0f;
	else if (color.z > 1.0f)
		color.z = 1.0f;
	return true;
}

static void GetSceneBBox(INode* node, TimeValue t, Box3& box, bool hideFrozen)
{
	if (node == NULL)
		return;

	ObjectState os = node->EvalWorldState(t);
	if (os.obj != NULL) {
		switch (os.obj->SuperClassID()) {
		default:
			if (!(node->IsNodeHidden()
					|| (hideFrozen && node->IsFrozen())
					|| !node->Renderable()
					|| os.obj == NULL
					|| !os.obj->IsRenderable())) {
				Box3 objBox;
				Matrix3 m(node->GetObjTMAfterWSM(t));
				os.obj->GetDeformBBox(t, objBox, &m, false);
				box += objBox;
			}
			break;
		case LIGHT_CLASS_ID:
			break;
		}
	}

	for (int i = node->NumberOfChildren(); --i >= 0; ) {
		GetSceneBBox(node->GetChildNode(i), t, box, hideFrozen);
	}
}

void SetUpSun(SClass_ID sid, const Class_ID& systemClassID, Interface* ip,
	GenLight* obj, const Point3& sunColor)
{
	MarketDefaults* def = GetMarketDefaults();

	obj->Enable(1);
	obj->SetShadow(def->GetInt(sid, systemClassID,
		_T("sunCastShadows"), 1) != 0);
	ShadowType* s = static_cast<ShadowType*>(def->CreateInstance(
		sid, systemClassID, _T("sunShadowGenerator"),
		SHADOW_TYPE_CLASS_ID, Class_ID(0, 0), MarketDefaults::CheckNULL));
	BOOL useGlobal = obj->GetUseGlobal();
	if (s != NULL) {
		obj->SetUseGlobal(0);
		obj->SetShadowGenerator(s);
	}
	obj->SetUseGlobal(def->GetInt(sid, systemClassID,
		_T("sunUseGlobalShadowSettings"), useGlobal) != 0);
	obj->SetOvershoot(def->GetInt(sid, systemClassID,
		_T("sunOvershoot"), OVERSHOOT) != 0);
	SuspendAnimate();
	obj->SetRGBColor(ip->GetTime(),
		def->GetRGBA(sid, systemClassID,
			_T("sunColor"), sunColor, clampColor));

	// Pretty ugly. The core code doesn't have a normal way to
	// determine whether frozen nodes are hidden and INode::IsNodeHidden
	// doesn't return true when the node is frozen and frozen
	// nodes are hidden. However, SetupRendParams does set the
	// RENDER_HIDE_FROZEN flag in RendParams::extraFlags if frozen
	// nodes are hidden. So we use this to help determine when nodes
	// are hidden.
	RendParams params;
	Box3 box;
	TimeValue t = ip->GetTime();

	GetCOREInterface()->SetupRendParams(params, NULL);
	GetSceneBBox(ip->GetRootNode(), t, box, (params.extraFlags & RENDER_HIDE_FROZEN) != 0);

	if (!box.IsEmpty()) {
		float len = ceilf(FLength(box.pmax - box.pmin) * 0.65f);

		if (len > obj->GetFallsize(t)) {
			obj->SetHotspot(t, len);
			obj->SetFallsize(t, len + 2);
		}
	}

	ResumeAnimate();
}

/*===========================================================================*\
 | NatLightAssembly Class Descriptor
\*===========================================================================*/

static bool checkStandin(Class_ID& id)
{
	return id.PartA() != STANDIN_CLASS_ID || id.PartB() != 0;
}

class NatLightAssemblyClassDesc:public ClassDesc2 {
	public:
	NatLightAssemblyClassDesc() { ResetClassParams(true); }
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new NatLightAssembly(); }
	const TCHAR *	ClassName() { return GetString(IDS_NAT_LIGHT_NAME); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() 
		{ return NATLIGHT_HELPER_CLASS_ID; }

	const TCHAR* InternalName() { return _T("DaylightAssemblyHead"); }
	HINSTANCE HInstance() { return hInstance; }

	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }

	void ResetClassParams(BOOL fileReset) {
		// IES Sun class ID - Defaults to directional light
		mSunID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
			NATLIGHT_HELPER_CLASS_ID, _T("sun"), kStandardSunClass,
			checkStandin);
		// IES Sky class ID - Defaults to Skylight
		mSkyID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
			NATLIGHT_HELPER_CLASS_ID, _T("sky"),
			Class_ID(0x7bf61478, 0x522e4705), checkStandin);
		mSunValid = true;
		mSkyValid = true;
	}

	Object* CreateSun(Interface* ip) {
		theHold.Suspend();
		Object* obj = static_cast<Object*>(mSunValid && CheckSun()
			? ip->CreateInstance(kValidSuperClass, mSunID) : NULL);
		theHold.Resume();
		return obj;
	}
	Object* CreateSky(Interface* ip) {
		theHold.Suspend();
		Object* obj = static_cast<Object*>(mSkyValid && CheckSky()
			? ip->CreateInstance(kValidSuperClass, mSkyID) : NULL);
		theHold.Resume();
		return obj;
	}

	private:

	bool IsPublicClass(const Class_ID& id) {
		if (id == Class_ID(0, 0))
			return true;

		ClassEntry* pE = GetCOREInterface()->GetDllDir().ClassDir()
			.FindClassEntry(kValidSuperClass, id);
		return pE != NULL && pE->IsPublic() != 0;
	}

	bool CheckSun() {
		if (IsPublicClass(mSunID))
			return true;
		mSunID = kStandardSunClass;
		mSunValid = true;
		return true;
	}

	bool CheckSky() {
		if (IsPublicClass(mSkyID))
			return true;
		mSunID = Class_ID(0,0);
		mSkyValid = false;
		return false;
	}

	Class_ID					mSunID;
	Class_ID					mSkyID;
	bool						mSunValid;
	bool						mSkyValid;
};


// A single instance of the class descriptor.
static NatLightAssemblyClassDesc natLightDesc;
// This returns a pointer to the instance.
ClassDesc* GetNatLightAssemblyDesc() { return &natLightDesc; }

/*===========================================================================*\
 | TraverseAssembly Class
\*===========================================================================*/

class TraverseAssembly {
public:
	enum ExcludeMembers {
		kNone = 0x00,
		kHead = 0x01,
		kOpen = 0x02,
		kClosed = 0x04,
		kOpenSubAssemblies = 0x08,
		kClosedSubAssemblies = 0x10,
		kOpenSubMembers = 0x20,
		kClosedSubMembers = 0x40
	};

	bool traverse(
		INode*	node,
		int		exclude = kNone
	);

protected:
	virtual bool __fastcall proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	) = 0;

private:
	bool __fastcall traverseChild(
		INode*		head,
		INode*		child,
		int			exclude,
		bool		subAssembly
	);
};

bool TraverseAssembly::traverse(
	INode*	node,
	int		exclude
)
{
	IAssembly* asmb;

	if (node == NULL || ((asmb = GetAssemblyInterface(node)) != NULL
			&& !asmb->IsAssemblyHead()))
		return false;

	if (!(exclude & kHead) && !proc(node, node, asmb))
		return false;

	int count = node->NumberOfChildren();
	for (int i = 0; i < count; ++i) {
		if (!traverseChild(node, node->GetChildNode(i), exclude, false))
			return false;
	}

	return true;
}

bool __fastcall TraverseAssembly::traverseChild(
	INode*		head,
	INode*		child,
	int			exclude,
	bool		subAssembly
)
{
	IAssembly* asmb;
	if (child == NULL || ((asmb = GetAssemblyInterface(child)) != NULL
			&& !asmb->IsAssemblyMember()))
		return true;

	if (asmb != NULL && asmb->IsAssemblyHead()) {
		static int excludeSubMask[2] = { kClosedSubAssemblies, kOpenSubAssemblies };
		if (exclude & excludeSubMask[asmb->IsAssemblyHeadOpen()])
			return true;
		if (!proc(head, child, asmb))
			return false;
		subAssembly = true;
		head = child;
	}
	else {
		static int selectType[2][2] = {
			{ kClosed, kOpen },
			{ kClosedSubMembers, kOpenSubMembers },
		};

		if (!(exclude & selectType[subAssembly][asmb != NULL && asmb->IsAssemblyMemberOpen()])
				&& !proc(head, child, asmb))
			return false;
	}

	if (asmb != NULL) {
		int count = child->NumberOfChildren();
		for (int i = 0; i < count; ++i) {
			if (!traverseChild(head, child->GetChildNode(i), exclude, subAssembly))
				return false;
		}
	}

	return true;
}

/*===========================================================================*\
 | AppendAssembly Class:
\*===========================================================================*/

class AppendAssembly : public TraverseAssembly {
public:
	AppendAssembly(INodeTab& nodes) : mNodes(nodes) {}

	bool __fastcall proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);

private:
	INodeTab&	mNodes;
};

bool __fastcall AppendAssembly::proc(
	INode*,
	INode*		node,
	IAssembly*
)
{
	mNodes.Append(1, &node);
	return true;
}

/*===========================================================================*\
 | Natural Light Assembly CreateMouseCallBack
\*===========================================================================*/

class NatLightAssembly::CreateCallback : public CreateMouseCallBack {
public:

	virtual int proc(
		ViewExp*	vpt,
		int			msg,
		int			point,
		int			flags,
		IPoint2		m,
		Matrix3&	mat
	);
};

int NatLightAssembly::CreateCallback::proc(
	ViewExp*	vpt,
	int			msg,
	int			point,
	int			flags,
	IPoint2		m,
	Matrix3&	mat
)
{
	// Process the mouse message sent...
	switch ( msg ) {
		case MOUSE_POINT:
			return CREATE_STOP;
	}

	return CREATE_CONTINUE;
}

/*===========================================================================*\
 | Natural Light Assembly Dialog Class:
\*===========================================================================*/

class NatLightAssembly::ParamDlgProc {
public:
	ParamDlgProc() : mpEditing(NULL),
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		mpMaster(NULL),
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		mpIp(NULL), mpEditSun(NULL), mpEditSky(NULL) {}

	void Begin(
		NatLightAssembly*	editing,
		IObjParam*			ip,
		ULONG				flags,
		Animatable*			prev
	);
	void End(
		NatLightAssembly*	editing,
		IObjParam*			ip,
		ULONG				flags,
		Animatable*			next
	);

	void Begin(
		NatLightAssembly*	natLight,
		Object*				obj
	);
	void End(
		NatLightAssembly*	natLight,
		Object*				obj
	);

	virtual void DeleteThis();
	virtual void Update(NatLightAssembly* natLight);

protected:
	bool isModifyPanel() const {
		return (mFlags & (BEGIN_EDIT_CREATE | BEGIN_EDIT_MOTION | BEGIN_EDIT_HIERARCHY
			| BEGIN_EDIT_IK | BEGIN_EDIT_LINKINFO)) == 0;
	}

#ifndef NO_DAYLIGHT_SELECTOR
	static CALLBACK DialogProc(
		HWND hwnd,
		UINT msg,
		WPARAM w,
		LPARAM l
	);
	virtual BOOL DlgProc(
		HWND		hWnd,
		UINT		msg,
		WPARAM		wParam,
		LPARAM		lParam
	);
#endif	// NO_DAYLIGHT_SELECTOR

private:
#ifndef NO_DAYLIGHT_SELECTOR
	BOOL InitDialog(HWND focus);
	void ReplaceNode(
		ParamID	param,
		HWND	combo
	);
#endif	// NO_DAYLIGHT_SELECTOR

	NatLightAssembly*		mpEditing;
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	SunMaster*				mpMaster;
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	IObjParam*				mpIp;
	Object*					mpEditSun;
	Object*					mpEditSky;
#ifndef NO_DAYLIGHT_SELECTOR
	HWND					mDlgWnd;
#endif	// NO_DAYLIGHT_SELECTOR
	ULONG					mFlags;
};


/*===========================================================================*\
 | Natural Light Assembly Param Accessor Class:
\*===========================================================================*/

class NatLightAssembly::Accessor : public PBAccessor {
protected:
	virtual void Get(
		PB2Value&		v,
		ReferenceMaker*	owner,
		ParamID			id,
		int				tabIndex,
		TimeValue		t,
		Interval&		valid
	);

	virtual void Set(
		PB2Value&		v,
		ReferenceMaker*	owner,
		ParamID			id,
		int				tabIndex,
		TimeValue		t
	);
};

/*===========================================================================*\
 | Natural Light Assembly Find Assembly Head Class
\*===========================================================================*/

class NatLightAssembly::FindAssemblyHead : protected DependentEnumProc
{
public:
	FindAssemblyHead(
		NatLightAssembly*	natLight
	) : mpNatLight(natLight) {}

	void FindHeadNodes() { mpNatLight->EnumDependents(this); }

protected:
	virtual int proc(ReferenceMaker *rmaker);
	virtual int proc(INode* head) = 0;

private:
	NatLightAssembly*	mpNatLight;
};

/*===========================================================================*\
 | Natural Light Assembly Are there any Sun and Sky Class
\*===========================================================================*/

class NatLightAssembly::AnySunAndSkyNodes : protected FindAssemblyHead,
											protected TraverseAssembly
{
public:
	AnySunAndSkyNodes(
		NatLightAssembly*	natLight,
		Object*				sun,
		Object*				sky
	) : FindAssemblyHead(natLight),
		mpSun(sun),
		mpSky(sky),
		mAnySuns(false),
		mAnySkys(false) {}

	void CheckNodes() { FindHeadNodes(); }
	bool AnySuns() const { return mAnySuns; }
	bool AnySkys() const { return mAnySkys; }

protected:
	virtual bool __fastcall proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);
	virtual int proc(INode* head);

private:
	Object*				mpSun;
	Object*				mpSky;
	bool				mAnySuns;
	bool				mAnySkys;
};

/*===========================================================================*\
 | Natural Light Assembly Replace Sun and Sky Class
\*===========================================================================*/

class NatLightAssembly::ReplaceSunAndSkyNodes : protected FindAssemblyHead,
												protected TraverseAssembly
{
public:
	ReplaceSunAndSkyNodes(
		NatLightAssembly*	natLight,
		Object*				old,
		bool				storeDirect
	) : FindAssemblyHead(natLight),
		mpOld(old),
		_storeDirect(storeDirect) {}

	void ReplaceNodes(
		TimeValue			t,
		IAssemblyMgr*		mgr,
		Interface*			ip,
		Object*				newObj,
		const TCHAR*		name
	);

protected:
	virtual bool __fastcall proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);
	virtual int proc(INode* head);

private:
	struct ObjEntry {
		ObjEntry(INode* head, INode* node) : mpHead(head), mpNode(node) {}
		INode*		mpHead;
		INode*		mpNode;
	};

	Object*				mpOld;
	Tab<ObjEntry>		mNodes;
	bool				_storeDirect;
};

/*===========================================================================*\
 | Natural Light Assembly Find Manual Setting Class
\*===========================================================================*/

class NatLightAssembly::FindManual : protected FindAssemblyHead
{
public:
	FindManual(
		NatLightAssembly*	natLight,
		TimeValue			t
	) : FindAssemblyHead(natLight),
		mT(t),
		mManualCount(0),
		mDateTimeCount(0) {}

	int FindManualSetting();

protected:
	virtual int proc(INode* head);

private:
	TimeValue	mT;
	ULONG		mManualCount;
	ULONG		mDateTimeCount;
};

/*===========================================================================*\
 | Natural Light Assembly Set Manual Setting Class
\*===========================================================================*/

class NatLightAssembly::SetManual : protected FindAssemblyHead
{
public:
	SetManual(
		NatLightAssembly*	natLight,
		TimeValue			t,
		bool				manual
	) : FindAssemblyHead(natLight),
		mT(t),
		mManual(manual) {}

	void SetManualSetting();

protected:
	virtual int proc(INode* head);

private:
	TimeValue	mT;
	bool		mManual;
};

/*===========================================================================*\
 | Natural Light Assembly Open and Close Assembly Class
\*===========================================================================*/

class NatLightAssembly::OpenAssembly : protected FindAssemblyHead
{
public:
	OpenAssembly(
		NatLightAssembly*	natLight
	) : FindAssemblyHead(natLight) {}

	void Open();

protected:
	virtual int proc(INode* head);

private:
	INodeTab		mTab;
};

class NatLightAssembly::CloseAssembly : protected FindAssemblyHead
{
public:
	CloseAssembly(
		NatLightAssembly*	natLight
	) : FindAssemblyHead(natLight) {}

	void Close();

protected:
	virtual int proc(INode* head);

private:
	INodeTab		mSel, mNotSel;
};

/*===========================================================================*\
 | Natural Light Assembly Sun Mesh Class
\*===========================================================================*/

class NatLightAssembly::SunMesh : public Mesh
{
public:
	SunMesh() : mBuilt(false) {}

	void BuildMesh() {
		if (!mBuilt) {
			CreateMesh(8.0f, 11, 3.0f);
			mBuilt = true;
		}
	}

private:
	void CreateMesh(
		float	radius,
		int		nbRays,
		float	heightFactor
	);

	bool	mBuilt;
};

NatLightAssembly::Accessor NatLightAssembly::sAccessor;
NatLightAssembly::ParamDlgProc NatLightAssembly::sEditParams;
NatLightAssembly::CreateCallback	NatLightAssembly::sCreateCallback;
NatLightAssembly::SunMesh NatLightAssembly::sSunMesh;

ParamBlockDesc2 NatLightAssembly::sMainParams(
	MAIN_PBLOCK, _T("NaturalLightParameters"), IDS_NATLIGHT_PARAMS,
		&natLightDesc, P_AUTO_CONSTRUCT,
	// Auto-construct params
	PBLOCK_REF,

	// Sun
	SUN_PARAM, _T("sun"), TYPE_REFTARG, 0, IDS_LIGHT_NAME,
		p_accessor, &sAccessor,
		end,

	// Sky
	SKY_PARAM, _T("sky"), TYPE_REFTARG, 0, IDS_SKY_NAME,
		p_accessor, &sAccessor,
		end,

	// Manual position
	MANUAL_PARAM, _T("manual"), TYPE_INT, P_TRANSIENT, IDS_MANUAL,
		p_default, TRUE,
		p_accessor, &sAccessor,
		end,

	end
);

/*===========================================================================*\
 | Natural Light Assembly Class:
\*===========================================================================*/

NatLightAssembly::NatLightAssembly()
	: mpParams(NULL),
	  mpSunObj(NULL),
	  mpSkyObj(NULL),
	  mpMult(NULL),
	  mpIntense(NULL),
	  mpSkyCond(NULL),
	  mExtDispFlags(0)
{
	natLightDesc.MakeAutoParamBlocks(this);
	assert(mpParams != NULL);
}

NatLightAssembly::~NatLightAssembly()
{
	DeleteAllRefs();
}

INode* NatLightAssembly::CreateAssembly(
	IObjCreate*			createInterface,
	NatLightAssembly*&	natObj
)
{
	theHold.Suspend();
	natObj = new NatLightAssembly();
	theHold.Resume();
	INode* natNode = createInterface->CreateObjectNode(natObj);
	assert(natNode != NULL);

	IAssembly* asmb = GetAssemblyInterface(natNode);
	if (asmb != NULL) {
		asmb->SetAssemblyHead(true);
	}

//	TSTR natName = GetString(IDS_DAY_CLASS);
//	createInterface->MakeNameUnique(natName);
//	natNode->SetName(natName);

	DbgAssert(natObj->ClassID() == NATLIGHT_HELPER_CLASS_ID);

	Object* sunObj = natLightDesc.CreateSun(createInterface);

	if (sunObj != NULL && sunObj->SuperClassID() == LIGHT_CLASS_ID) {
		SetUpSun(HELPER_CLASS_ID, NATLIGHT_HELPER_CLASS_ID, createInterface,
			static_cast<GenLight*>(sunObj), SUN_RGB);
	}

	natObj->mpParams->SetValue(SUN_PARAM, 0, sunObj);
	assert(natObj->mpSunObj == sunObj);

	Object* skyObj = natLightDesc.CreateSky(createInterface);

	natObj->mpParams->SetValue(SKY_PARAM, 0, skyObj);
	assert(natObj->mpSkyObj == skyObj);

	return natNode;
}

void NatLightAssembly::AppendAssemblyNodes(
	INode*			theAssembly,
	INodeTab&		nodes,
	SysNodeContext
)
{
	AppendAssembly(nodes).traverse(theAssembly, TraverseAssembly::kHead);
}

static inline bool IsValidSun(ReferenceTarget* obj)
{
	INaturalLightClass* nl;
	return obj != NULL && obj->SuperClassID() == kValidSuperClass
		&& (obj->ClassID() == kStandardSunClass
			|| ((nl = GetNaturalLightClass(obj)) != NULL
				&& nl->IsSun()));
}

static inline bool IsValidSky(ReferenceTarget* obj)
{
	INaturalLightClass* nl;
	return obj != NULL && obj->SuperClassID() == kValidSuperClass
		&& (nl = GetNaturalLightClass(obj)) != NULL
		&& nl->IsSky();
}

static inline bool IsValidSunOrSky(
	int					ref,
	ReferenceTarget*	obj
)
{
	// Make sure the objects are valid. Suns can be standard
	// or export the ISunLightInterface, or be None.
	// Skys can be None or export the ISkyLightInterface
	if (obj != NULL) {
		if (ref == NatLightAssembly::SUN_REF) {
			if (!IsValidSun(obj))
				return false;
		}
		else if (ref == NatLightAssembly::SKY_REF) {
			if (!IsValidSky(obj))
				return false;
		}
		else
			return false;
	}

	return true;
}

static void SetController(Object* obj, const TCHAR* name, Control* slave)
{
	if (obj != NULL && obj->SuperClassID() == LIGHT_CLASS_ID) {
		two_value_locals(prop, val);			// Keep two local variables

		try {
			// Get the name and value to set
			vl.prop = Name::intern(const_cast<TCHAR*>(name));
			vl.val = slave == NULL ? NULL : MAXControl::intern(slave, defaultDim);

			// Set the value.
			MAXWrapper::set_max_prop_controller(obj, vl.prop,
				static_cast<MAXControl*>(vl.val));
		} catch (MAXScriptException& ex) {
			(void)ex;
		}
		pop_value_locals();
	}
}

void NatLightAssembly::SetSunAndSky(
	TimeValue			t,
	int					ref,
	ParamID				param,
	const TCHAR*		name,
	ReferenceTarget*	obj
)
{
	Object* old = static_cast<Object*>(GetReference(ref));
	if (obj != old) {
		IAssemblyMgr* mgr = GetAssemblyMgr();
		if (IsValidSunOrSky(ref, obj)) {
			// If restoring or redoing, then everything will happen automatically
			if (!theHold.RestoreOrRedoing()) {
				// Param block 2 is calling us with the hold suspended
				// which is not good.
				ULONG resume = 0;
				while (theHold.IsSuspended()) {
					theHold.Resume();
					++resume;
				}

				Interface* ip = GetCOREInterface();
				bool wereInvalid = mpSunObj == NULL && mpSkyObj == NULL;

				ip->DisableSceneRedraw();
//				sEditParams.End(this, old);
				ReplaceReference(ref, obj);

				bool storeDirect = ref != SUN_REF && obj != NULL
					&& GetMarketDefaults()->GetInt(LIGHT_CLASS_ID, obj->ClassID(),
						_T("storeDirectIllumination"), 0) != 0;
				ReplaceSunAndSkyNodes replace(this, old, storeDirect);
				replace.ReplaceNodes(t, mgr, ip, static_cast<Object*>(obj), name);

				if (obj != NULL) {
					if (ref == SUN_REF) {
						ISunLight* sun = GetSunLightInterface(mpSunObj);
						if (sun == NULL || sun->IsIntensityInMAXUnits()) {
							if (mpMult != NULL)
								SetController(mpSunObj, _T("multiplier"), mpMult);
						} else {
							if (mpIntense != NULL)
								SetController(mpSunObj, _T("multiplier"), mpIntense);
						}
					} else {
						if (mpSkyCond != NULL)
							SetController(mpSkyObj, _T("sky_cover"), mpSkyCond);
					}
				}

//				sEditParams.Begin(this, static_cast<Object*>(obj));
				sEditParams.Update(this);
				bool areInvalid = mpSunObj == NULL && mpSkyObj == NULL;
				if (wereInvalid != areInvalid) {
					if (areInvalid) {
						OpenAssembly open(this);
						open.Open();
					} else {
						CloseAssembly close(this);
						close.Close();
					}
					NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				}

				ip->EnableSceneRedraw();
				ip->RedrawViews(ip->GetTime());

				while (resume > 0) {
					theHold.Suspend();
					--resume;
				}
			}
		}
		else {
			mpParams->SetValue(param, 0, old);
		}
	}
}

void NatLightAssembly::SetMultController(Control* c)
{
	ReplaceReference(MULT_REF, c);
	if (mpSunObj != NULL) {
		ISunLight* sun = GetSunLightInterface(mpSunObj);
		if (sun == NULL || sun->IsIntensityInMAXUnits())
			SetController(mpSunObj, _T("multiplier"), c);
	}
}

void NatLightAssembly::SetIntenseController(Control* c)
{
	ReplaceReference(INTENSE_REF, c);
	if (mpSunObj != NULL) {
		ISunLight* sun = GetSunLightInterface(mpSunObj);
		if (sun != NULL && !sun->IsIntensityInMAXUnits())
			SetController(mpSunObj, _T("multiplier"), c);
	}
}

void NatLightAssembly::SetSkyCondController(Control* c)
{
	ReplaceReference(SKY_COND_REF, c);
	if (mpSkyObj != NULL)
		SetController(mpSkyObj, _T("sky_cover"), c);
}

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
SunMaster* NatLightAssembly::FindMaster()
{
	static Control* NatLightAssembly::*controllers[] = {
		&NatLightAssembly::mpMult,
		&NatLightAssembly::mpIntense,
		&NatLightAssembly::mpSkyCond
	};
	int i;
	ReferenceTarget* master;
	
	for (i = 0; i < sizeof(controllers) / sizeof(controllers[0]); ++i) {
		Control* c = this->*controllers[i];
		if (c != NULL
				&& c->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
					DAYLIGHT_SLAVE_CONTROL_CID2))
				&& (master = c->GetReference(0)) != NULL
				&& master->IsSubClassOf(Class_ID(DAYLIGHT_CID1,DAYLIGHT_CID2)))
			return static_cast<SunMaster*>(master);
	}
	return NULL;
}
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

void NatLightAssembly::GetDeformBBox(
	TimeValue	t,
	Box3&		box,
	Matrix3*	tm,
	BOOL		useSel
)
{
	sSunMesh.BuildMesh();
	box = sSunMesh.getBoundingBox(tm);
}

void NatLightAssembly::InitNodeName(TSTR& s)
{
	s = GetString(IDS_DAY_CLASS);
}

// From Object

ObjectState NatLightAssembly::Eval(TimeValue t)
{
	return ObjectState(this);
}

// From BaseObject

static void RemoveScaling(Matrix3 &tm) {
	AffineParts ap;
	decomp_affine(tm, &ap);
	tm.IdentityMatrix();
	tm.SetRotate(ap.q);
	tm.SetTrans(ap.t);
}

static void GetMat(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Matrix3&	tm
) 
{
	tm = inode->GetObjectTM(t);
//	tm.NoScale();
	RemoveScaling(tm);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void NatLightAssembly::SetExtendedDisplay(int flags)
{
	mExtDispFlags = flags;
}

int NatLightAssembly::Display(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	int			flags
)
{
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL|(gw->getRndMode() & GW_Z_BUFFER));
	if (inode->Selected())
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())	{
		Color color(inode->GetWireColor());
		gw->setColor( LINE_COLOR, color );
	}
	sSunMesh.BuildMesh();
	sSunMesh.render( gw, gw->getMaterial(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);	
	gw->setRndLimits(rlim);
	return 0 ;
}

int NatLightAssembly::HitTest(
	TimeValue t,
	INode*		inode,
	int			type,
	int			crossing,
	int			flags,
	IPoint2*	p,
	ViewExp*	vpt
)
{
	HitRegion hitRegion;
	DWORD savedLimits;
	int res;
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();

	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits( ((savedLimits = gw->getRndLimits()) | GW_PICK) & ~(GW_ILLUM|GW_BACKCULL));
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	// if we get a hit on the mesh, we're done
	sSunMesh.BuildMesh();
	res = sSunMesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
	gw->setRndLimits(savedLimits);
	return res;
}

void NatLightAssembly::Snap(
	TimeValue	t,
	INode*		inode,
	SnapInfo*	snap,
	IPoint2*	p,
	ViewExp*	vpt
)
{
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;
		Point3 vert(0.0f,0.0f,0.0f);

		gw->wTransPoint(&vert,&screen3);

		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
			// Closer than the best of this priority?
			else if(len < snap->bestDist) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
		}
	}
}

void NatLightAssembly::GetWorldBoundBox(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Box3&		box
)
{
	int nv;
	Matrix3 tm;
	GetMat(t, inode,vpt,tm);
	Point3 loc = tm.GetTrans();
	sSunMesh.BuildMesh();
	nv = sSunMesh.getNumVerts();
	box.Init();
	if(!(mExtDispFlags & EXT_DISP_ZOOM_EXT)) 
		box.IncludePoints(sSunMesh.verts,nv,&tm);
	else
		box += loc;
}

void NatLightAssembly::GetLocalBoundBox(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Box3&		box
)
{
	Point3 loc = inode->GetObjectTM(t).GetTrans();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(loc) / 360.0f;
	sSunMesh.BuildMesh();
	box = sSunMesh.getBoundingBox();
	box.Scale(scaleFactor);
}

CreateMouseCallBack* NatLightAssembly::GetCreateMouseCallBack()
{
	return &sCreateCallback;
}

TCHAR* NatLightAssembly::GetObjectName()
{
	return GetString(IDS_NAT_LIGHT_NAME);
}


BOOL NatLightAssembly::HasUVW()
{
	return FALSE;
}

// From ReferenceTarget

RefTargetHandle NatLightAssembly::Clone(RemapDir &remap)
{
    NatLightAssembly* newl = new NatLightAssembly();
	if (mpSunObj != NULL)
		newl->ReplaceReference(SUN_REF,mpSunObj->Clone(remap));
	if (mpSkyObj != NULL)
		newl->ReplaceReference(SKY_REF,mpSkyObj->Clone(remap));
	return(newl);
}

// From ReferenceMaker

RefResult NatLightAssembly::NotifyRefChanged(
	Interval		changeInt,
	RefTargetHandle	hTarget,
	PartID&			partID,
	RefMessage		message
)
{
	switch (message) {
		case REFMSG_CHANGE: {
			if (hTarget == mpSunObj || hTarget == mpSkyObj) {
				sEditParams.Update(this);
			}
		} break;
	}
	return REF_SUCCEED;
}

int NatLightAssembly::NumRefs()
{
	return NUM_REFS;
}

RefTargetHandle NatLightAssembly::GetReference(int i)
{
	switch (i) {
	case PBLOCK_REF:
		return mpParams;
	case SUN_REF:
		return mpSunObj;
	case SKY_REF:
		return mpSkyObj;
	case MULT_REF:
		return mpMult;
	case INTENSE_REF:
		return mpIntense;
	case SKY_COND_REF:
		return mpSkyCond;
	}
	return NULL;
}

void NatLightAssembly::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
	case PBLOCK_REF:
		mpParams = static_cast<IParamBlock2*>(rtarg);
		break;
	case SUN_REF:
		if (mpSunObj != rtarg) {
			sEditParams.End(this, mpSunObj);
			mpSunObj = static_cast<LightObject*>(rtarg);
			sEditParams.Begin(this, mpSunObj);
			sEditParams.Update(this);
		}
		break;
	case SKY_REF:
		if (mpSkyObj != rtarg) {
			sEditParams.End(this, mpSkyObj);
			mpSkyObj = static_cast<LightObject*>(rtarg);
			sEditParams.Begin(this, mpSkyObj);
			sEditParams.Update(this);
		}
		break;
	case MULT_REF:
		mpMult = static_cast<Control*>(rtarg);
		break;
	case INTENSE_REF:
		mpIntense = static_cast<Control*>(rtarg);
		break;
	case SKY_COND_REF:
		mpSkyCond = static_cast<Control*>(rtarg);
		break;
	}
}

IOResult NatLightAssembly::Save(ISave *isave)
{
	return IO_OK;
}

IOResult NatLightAssembly::Load(ILoad *iload)
{
	return IO_OK;
}

// From Animatable

void NatLightAssembly::GetClassName(TSTR& s)
{
	s = GetString(IDS_NAT_LIGHT_NAME);
}

Class_ID NatLightAssembly::ClassID()
{
	return NATLIGHT_HELPER_CLASS_ID;
}

void NatLightAssembly::DeleteThis()
{
	delete this;
}

void NatLightAssembly::BeginEditParams(
	IObjParam*	ip,
	ULONG		flags,
	Animatable*	prev
)
{
	sEditParams.Begin(this, ip, flags, prev);
}

void NatLightAssembly::EndEditParams(
	IObjParam*	ip,
	ULONG		flags,
	Animatable*	next
)
{
	sEditParams.End(this, ip, flags, next);
}


int NatLightAssembly::NumSubs()
{
	return NUM_SUBS;
}

Animatable* NatLightAssembly::SubAnim(int i)
{
	return NULL;
}

TSTR NatLightAssembly::SubAnimName(int i)
{
	return TSTR();
}

BOOL NatLightAssembly::IsAnimated()
{
	return FALSE;
}

int NatLightAssembly::NumParamBlocks()
{
	return NUM_PBLOCK;
}

IParamBlock2* NatLightAssembly::GetParamBlock(int i)
{
	switch (i) {
	case 0:
		return mpParams;
	}

	return NULL;
}

IParamBlock2* NatLightAssembly::GetParamBlockByID(short id)
{
	switch (id) {
	case MAIN_PBLOCK:
		return mpParams;
	}

	return NULL;
}

void* NatLightAssembly::GetInterface(ULONG id)
{
	return NULL;
}

BaseInterface* NatLightAssembly::GetInterface(Interface_ID id)
{
	return NULL;
}

void NatLightAssembly::ReleaseInterface(Interface_ID id, void* i)
{
}

int NatLightAssembly::SetProperty(
	ULONG	id,
	void*	data
)
{
	return 0;
}

void* NatLightAssembly::GetProperty(ULONG id)
{
	return NULL;
}


/*===========================================================================*\
 | Natural Light Assembly Dialog Class:
\*===========================================================================*/

void NatLightAssembly::ParamDlgProc::Begin(
	NatLightAssembly*	editing,
	IObjParam*			ip,
	ULONG				flags,
	Animatable*			prev
)
{
	// If asked to edit an object we are already editing
	// then do nothing.
	if (mpEditing == editing)
		return;

	if (mpEditing != NULL) {
		End(mpEditing, mpEditing->mpSunObj);
		End(mpEditing, mpEditing->mpSkyObj);
	}

	mpEditing = editing;
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
#ifndef NO_CREATE_TASK
	mpMaster = SunMaster::IsCreateModeActive() ? NULL : mpEditing->FindMaster();
#else
	mpMaster = mpEditing->FindMaster();
#endif
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpIp = ip;
	mFlags = flags;

#ifndef NO_DAYLIGHT_SELECTOR
	if (mDlgWnd == NULL) {
		ip->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_NATLIGHTPARAM),
			DialogProc,
			GetString(IDS_NATLIGHT_PARAM_NAME),
			reinterpret_cast<LPARAM>(this),
			0,
			(ROLLUP_CAT_SYSTEM + ROLLUP_CAT_STANDARD) / 2);
		assert(mDlgWnd != NULL);
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		if (mpMaster != NULL)
			mpMaster->BeginEditParams(ip, flags, prev);
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

#ifndef NO_DAYLIGHT_SELECTOR
	} else {
		SetWindowLongPtr(mDlgWnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(this));
		Update(mpEditing);
	}
#endif	// NO_DAYLIGHT_SELECTOR

	Begin(mpEditing, editing->mpSunObj);
	Begin(mpEditing, editing->mpSkyObj);
}

void NatLightAssembly::ParamDlgProc::End(
	NatLightAssembly*	editing,
	IObjParam*			ip,
	ULONG				flags,
	Animatable*			next
)
{
	if (mpEditing != NULL) {
		End(mpEditing, mpEditing->mpSkyObj);
		End(mpEditing, mpEditing->mpSunObj);
	}

#ifndef NO_DAYLIGHT_SELECTOR
	if ((flags & END_EDIT_REMOVEUI) && mDlgWnd != NULL) {
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		if (mpMaster != NULL)
			mpMaster->EndEditParams(ip, flags, next);
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

#ifndef NO_DAYLIGHT_SELECTOR
		ip->UnRegisterDlgWnd(mDlgWnd);
		ip->DeleteRollupPage(mDlgWnd);
		assert(mDlgWnd == NULL);
	} else
		SetWindowLongPtr(mDlgWnd, GWLP_USERDATA, NULL);
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpMaster = NULL;
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpEditing = NULL;
	mpIp = NULL;
}

void NatLightAssembly::ParamDlgProc::Begin(
	NatLightAssembly*	natLight,
	Object*				obj
)
{
	if (mpEditing == natLight && obj != NULL
			&& (obj == mpEditing->mpSunObj || obj == mpEditing->mpSkyObj)) {
		End(natLight, obj);
		if (obj == mpEditing->mpSunObj)
			mpEditSun = obj;
		else
			mpEditSky = obj;
		obj->BeginEditParams(mpIp, mFlags, NULL);
		if (obj == mpEditSun && isModifyPanel() && obj->GetCustAttribContainer())
			obj->GetCustAttribContainer()->BeginEditParams(mpIp, mFlags, NULL);
	}
}

void NatLightAssembly::ParamDlgProc::End(
	NatLightAssembly*	natLight,
	Object*				obj
)
{
	if (mpEditing == natLight && obj != NULL
			&& (obj == mpEditSun || obj == mpEditSky)) {
		obj->EndEditParams(mpIp, END_EDIT_REMOVEUI, NULL);
		if (obj == mpEditSun && isModifyPanel() && obj->GetCustAttribContainer())
			obj->GetCustAttribContainer()->EndEditParams(mpIp, END_EDIT_REMOVEUI, NULL);
		if (obj == mpEditSun)
			mpEditSun = NULL;
		else
			mpEditSky = NULL;
	}
}

#ifndef NO_DAYLIGHT_SELECTOR
BOOL NatLightAssembly::ParamDlgProc::DialogProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	if (msg == WM_INITDIALOG) {
		SetWindowLongPtr(hwnd, GWLP_USERDATA, l);
		ParamDlgProc* p = reinterpret_cast<ParamDlgProc*>(l);
		p->mDlgWnd = hwnd;
		BOOL rVal = p->InitDialog(reinterpret_cast<HWND>(w));
		p->Update(p->mpEditing);
		return rVal;
	}

	ParamDlgProc* p = reinterpret_cast<ParamDlgProc*>(
		GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (p != NULL) {
		BOOL rVal = p->DlgProc(hwnd, msg, w, l);
		if (msg == WM_NCDESTROY)
			p->mDlgWnd = NULL;
		return rVal;
	}

	return false;
}

BOOL NatLightAssembly::ParamDlgProc::DlgProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case IDC_SUN_ACTIVE: {
					if (mpEditing->mpSunObj != NULL) {
						BOOL on = IsDlgButtonChecked(mDlgWnd, IDC_SUN_ACTIVE);
						if (on != mpEditing->mpSunObj->GetUseLight()) {
							theHold.Begin();
							mpEditing->mpSunObj->SetUseLight(on);
							theHold.Accept(GetString(IDS_UNDO_PARAM));
							Interface* ip = GetCOREInterface();
							ip->RedrawViews(ip->GetTime());
						}
					}
				} break;

				case IDC_SKY_ACTIVE: {
					if (mpEditing->mpSkyObj != NULL) {
						BOOL on = IsDlgButtonChecked(mDlgWnd, IDC_SKY_ACTIVE);
						if (on != mpEditing->mpSkyObj->GetUseLight()) {
							theHold.Begin();
							mpEditing->mpSkyObj->SetUseLight(on);
							theHold.Accept(GetString(IDS_UNDO_PARAM));
							Interface* ip = GetCOREInterface();
							ip->RedrawViews(ip->GetTime());
						}
					}
				} break;

				case IDC_MANUAL_POSITION: {
					Interface* ip = GetCOREInterface();
					TimeValue t = ip->GetTime();
					theHold.Begin();
					mpEditing->mpParams->SetValue(MANUAL_PARAM, t, 1);
					theHold.Accept(GetString(IDS_UNDO_PARAM));
					ip->RedrawViews(t);
				} break;

				case IDC_CONTROLLER_POS: {
					Interface* ip = GetCOREInterface();
					TimeValue t = ip->GetTime();
					theHold.Begin();
					mpEditing->mpParams->SetValue(MANUAL_PARAM, t, 0);
					ip->RedrawViews(t);
					theHold.Accept(GetString(IDS_UNDO_PARAM));
				} break;

				case IDC_SETUP_CONTROLLER:
					if (HIWORD(w) == BN_CLICKED)
						mpIp->ExecuteMAXCommand(MAXCOM_MOTION_MODE);
					break;

				case IDC_SUN_COMBO:
					if (HIWORD(w) == CBN_SELENDOK) {
						theHold.Begin();
						ReplaceNode(SUN_PARAM, GetDlgItem(mDlgWnd, IDC_SUN_COMBO));
						theHold.Accept(GetString(IDS_UNDO_PARAM));
					}
					break;

				case IDC_SKY_COMBO:
					if (HIWORD(w) == CBN_SELENDOK) {
						theHold.Begin();
						ReplaceNode(SKY_PARAM, GetDlgItem(mDlgWnd, IDC_SKY_COMBO));
						theHold.Accept(GetString(IDS_UNDO_PARAM));
					}
					break;
			}
			break;
	}

	return false;
}
#endif	// NO_DAYLIGHT_SELECTOR

void NatLightAssembly::ParamDlgProc::DeleteThis()
{
}

#ifndef NO_DAYLIGHT_SELECTOR
static int findEntry(
	HWND				combo,
	Object*				obj
)
{
	int ix;
	if (obj == NULL) {
		for (ix = SendMessage(combo, CB_GETCOUNT, 0, 0); --ix >= 0; ) {
			ClassDesc* pCd = reinterpret_cast<ClassDesc*>(
				SendMessage(combo, CB_GETITEMDATA, ix, 0));
			if (pCd == NULL) {
				break;
			}
		}
	}
	else {
		SClass_ID super = obj->SuperClassID();
		Class_ID id = obj->ClassID();
		for (ix = SendMessage(combo, CB_GETCOUNT, 0, 0); --ix >= 0; ) {
			ClassDesc* pCd = reinterpret_cast<ClassDesc*>(
				SendMessage(combo, CB_GETITEMDATA, ix, 0));
			if (pCd != NULL && pCd->SuperClassID() == super
					&& pCd->ClassID() == id) {
				break;
			}
		}
	}

	return ix;
}
#endif	// NO_DAYLIGHT_SELECTOR

void NatLightAssembly::ParamDlgProc::Update(NatLightAssembly* natLight)
{
#ifndef NO_DAYLIGHT_SELECTOR
	if (mpEditing == natLight && mDlgWnd != NULL) {
//		if (mpEditing->mpSunObj != NULL || mpEditing->mpSkyObj != NULL) {
//			AnySunAndSkyNodes check(mpEditing, mpEditing->mpSunObj, mpEditing->mpSkyObj);
//			check.CheckNodes();
//			if (!check.AnySuns() && mpEditing->mpSunObj != NULL)
//				mpEditing->DeleteReference(SUN_REF);
//			if (!check.AnySkys() && mpEditing->mpSkyObj != NULL)
//				mpEditing->DeleteReference(SKY_REF);
//		}

		bool lightSun = mpEditing->mpSunObj != NULL
			&& mpEditing->mpSunObj->SuperClassID() == kValidSuperClass;

		EnableWindow(GetDlgItem(mDlgWnd, IDC_SUN_ACTIVE), lightSun);
		CheckDlgButton(mDlgWnd, IDC_SUN_ACTIVE,
			lightSun && static_cast<LightObject*>(
				mpEditing->mpSunObj)->GetUseLight() != 0);

		bool lightSky = mpEditing->mpSkyObj != NULL
			&& mpEditing->mpSkyObj->SuperClassID() == kValidSuperClass;

		EnableWindow(GetDlgItem(mDlgWnd, IDC_SKY_ACTIVE), lightSky);
		CheckDlgButton(mDlgWnd, IDC_SKY_ACTIVE,
			lightSky && static_cast<LightObject*>(
				mpEditing->mpSkyObj)->GetUseLight() != 0);

		HWND sunCombo = GetDlgItem(mDlgWnd, IDC_SUN_COMBO);
		SendMessage(sunCombo, CB_SETCURSEL,
			findEntry(sunCombo, mpEditing->mpSunObj), 0);

		HWND skyCombo = GetDlgItem(mDlgWnd, IDC_SKY_COMBO);
		SendMessage(skyCombo, CB_SETCURSEL,
			findEntry(skyCombo, mpEditing->mpSkyObj), 0);

		TimeValue t = GetCOREInterface()->GetTime();
		CheckDlgButton(mDlgWnd, IDC_MANUAL_POSITION,
			mpEditing->mpParams->GetInt(MANUAL_PARAM, t) == 1);
		CheckDlgButton(mDlgWnd, IDC_CONTROLLER_POS,
			mpEditing->mpParams->GetInt(MANUAL_PARAM, t) == 0);
	}
#endif	// NO_DAYLIGHT_SELECTOR
}

#ifndef NO_DAYLIGHT_SELECTOR
static inline void addLightToCombo(
	HWND			combo,
	const TCHAR*	string,
	ClassDesc*		pCd
)
{
	// Add this one to the sun combo.
	int ix = SendMessage(combo, CB_ADDSTRING, 0,
		reinterpret_cast<LPARAM>(string));
	if (ix >= 0) {
		SendMessage(combo, CB_SETITEMDATA, ix,
			reinterpret_cast<LPARAM>(pCd));
	}
}

static void addLightToCombo(
	HWND						combo,
	ClassDesc*					pCd,
	const INaturalLightClass*	pNC,
	BOOL						(INaturalLightClass::*shouldAdd)() const
)
{
	if ((pNC->*shouldAdd)()) {
		addLightToCombo(combo, pCd->ClassName(), pCd);
	}
}

static void addLightsToComboBoxes(
	Interface*	pIp,
	HWND		sunCombo,
	HWND		skyCombo
)
{
	SendMessage(sunCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(skyCombo, CB_RESETCONTENT, 0, 0);

	// Look for lights that have the SUN_INTERFACE_ID or SKY_INTERFACE_ID
	// mixin interfaces. These will go into the sun and sky combo boxes.
	ClassDirectory& dir = pIp->GetDllDir().ClassDir();
	SubClassList& sc = *dir.GetClassList(kValidSuperClass);

	if (&sc != NULL) {
		for (int isub = sc.Count(ACC_ALL); --isub >= 0; ) {
			ClassDesc* pCd = sc[isub].FullCD();
			if (pCd != NULL) {
				if (pCd->ClassID() == kStandardSunClass) {
					// Direct lights are called Standard sunlight
					addLightToCombo(sunCombo, GetString(IDS_STANDARD_SUNLIGHT), pCd);
				}
				else {
					INaturalLightClass* pNC;
					
					pNC = GetNaturalLightClassInterface(pCd);
					if (pNC != NULL && (pNC->GetDesc()->flags & FP_STATIC_METHODS)) {
						addLightToCombo(sunCombo, pCd, pNC, &INaturalLightClass::IsSun);
						addLightToCombo(skyCombo, pCd, pNC, &INaturalLightClass::IsSky);
					}
				}
			}
		}
	}

	// Finally add "No sunlight" and "No skylight" at the top
	SendMessage(sunCombo, CB_INSERTSTRING, 0,
		reinterpret_cast<LPARAM>(LPCTSTR(GetString(IDS_NO_SUNLIGHT))));
	SendMessage(skyCombo, CB_INSERTSTRING, 0,
		reinterpret_cast<LPARAM>(LPCTSTR(GetString(IDS_NO_SKYLIGHT))));
}

BOOL NatLightAssembly::ParamDlgProc::InitDialog(HWND)
{
	addLightsToComboBoxes(mpIp, GetDlgItem(mDlgWnd, IDC_SUN_COMBO),
		GetDlgItem(mDlgWnd, IDC_SKY_COMBO));
	return true;
}

void NatLightAssembly::ParamDlgProc::ReplaceNode(
	ParamID	param,
	HWND	combo
)
{
	if (mpEditing == NULL)
		return;

	// Get selection from combo. If nothing is selected return.
	int sel = SendMessage(combo, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		return;

	// Get class description from combo. NULL means we don't
	// know how to build the object.
	RefTargetHandle obj = mpEditing->mpParams->GetReferenceTarget(param);
	ClassDesc* pCd = reinterpret_cast<ClassDesc*>(SendMessage(
		combo, CB_GETITEMDATA, sel, 0));

	if (pCd != NULL) {
		// If the object is the same. Don't do anything.
		if (obj == NULL || obj->SuperClassID() != pCd->SuperClassID()
				|| obj->ClassID() != pCd->ClassID()) {
			theHold.Suspend();
			Interface* ip = GetCOREInterface();
			obj = static_cast<RefTargetHandle>(
				ip->CreateInstance(pCd->SuperClassID(), pCd->ClassID()));
			theHold.Resume();
			assert(obj != NULL);
			if (obj->SuperClassID() == LIGHT_CLASS_ID) {
				static_cast<GenLight*>(obj)->Enable(1);
				if (param == SUN_PARAM) {
					SetUpSun(HELPER_CLASS_ID, NATLIGHT_HELPER_CLASS_ID,
						ip, static_cast<GenLight*>(obj), SUN_RGB);
				}
			}

		}
	} else
		obj = NULL;

	mpEditing->mpParams->SetValue(param, GetCOREInterface()->GetTime(), obj);
}
#endif	// NO_DAYLIGHT_SELECTOR

/*===========================================================================*\
 | Natural Light Assembly Param Accessor Class:
\*===========================================================================*/

void NatLightAssembly::Accessor::Get(
	PB2Value&		v,
	ReferenceMaker*	owner,
	ParamID			id,
	int				tabIndex,
	TimeValue		t,
	Interval&		valid
)
{
	NatLightAssembly* p = static_cast<NatLightAssembly*>(owner);

	switch(id) {
		case MANUAL_PARAM: {
			FindManual findManual(p, t);
			v.i = findManual.FindManualSetting();
		} break;
	}
}

void NatLightAssembly::Accessor::Set(
	PB2Value&		v,
	ReferenceMaker*	owner,
	ParamID			id,
	int				tabIndex,
	TimeValue		t
)
{
	NatLightAssembly* p = static_cast<NatLightAssembly*>(owner);

	switch(id) {
		case SUN_PARAM: {
			TSTR sunName = GetString(IDS_LIGHT_NAME);
			p->SetSunAndSky(t, SUN_REF, SUN_PARAM, sunName, v.r);
		} break;

		case SKY_PARAM: {
			TSTR skyName = GetString(IDS_SKY_NAME);
			p->SetSunAndSky(t, SKY_REF, SKY_PARAM, skyName, v.r);
		} break;

		case MANUAL_PARAM: {
			SetManual setManual(p, t, v.i > 0);
			setManual.SetManualSetting();
		} break;
	}
}

/*===========================================================================*\
 | Natural Light Assembly Find Assembly Head Class
\*===========================================================================*/

int NatLightAssembly::FindAssemblyHead::proc(ReferenceMaker *rmaker)
{
	if (rmaker->SuperClassID() == BASENODE_CLASS_ID) {
		INode* node = static_cast<INode*>(rmaker);
		Object* obj = node->GetObjectRef();
		if (obj != NULL) {
			obj = obj->FindBaseObject();
			if (obj == mpNatLight) {
				return proc(node);
			}
		}
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}

/*===========================================================================*\
 | Natural Light Assembly Are there any Sun and Sky Class
\*===========================================================================*/

bool __fastcall NatLightAssembly::AnySunAndSkyNodes::proc(
	INode*		head,
	INode*		node,
	IAssembly*
)
{
	Object* obj = node->GetObjectRef();
	if (obj != NULL) {
		obj = obj->FindBaseObject();
		if (obj == mpSun && mpSun != NULL) {
			mAnySuns = true;
		}
		if (obj == mpSky && mpSky != NULL) {
			mAnySkys = true;
		}
	}

	return true;
}

int NatLightAssembly::AnySunAndSkyNodes::proc(INode* node)
{
	traverse(node, kHead | kOpenSubAssemblies
		| kClosedSubAssemblies);
	if ((mAnySkys || mpSky == NULL) && (mAnySuns || mpSun == NULL))
		return DEP_ENUM_HALT;
	return DEP_ENUM_CONTINUE;
}


/*===========================================================================*\
 | Natural Light Assembly Replace Sun and Sky Class
\*===========================================================================*/

static void DeleteNode(
	Interface*	ip,
	INode*		newNode,
	INode*		node
)
{
	if (node != NULL) {
		int i, count = node->NumberOfChildren();

		for (i = 0; i < count; ++i) {
			INode* child = node->GetChildNode(i);
			child->Detach(0);
			newNode->AttachChild(child);
		}

		IAssembly* asmb = GetAssemblyInterface(node);
		if (asmb != NULL) {
			asmb->SetAssemblyMember(false);
			node->Detach(0);
			asmb->SetAssemblyMember(true);
		}
		ip->DeleteNode(node);
	}
}

void NatLightAssembly::ReplaceSunAndSkyNodes::ReplaceNodes(
	TimeValue			t,
	IAssemblyMgr*		mgr,
	Interface*			ip,
	Object*				newObj,
	const TCHAR*		baseName
)
{
	FindHeadNodes();

	int i;
	INode* lastHead = NULL;
	INode* node = NULL;

	for (i = mNodes.Count(); --i >= 0; ) {
		// Now we need to replace the object in this node.
		ObjEntry& entry = mNodes[i];

		if (lastHead == entry.mpHead || newObj == NULL) {
			DeleteNode(ip, node, entry.mpNode);
		}
		else if (entry.mpNode != NULL && mpOld == newObj) {
			node = entry.mpNode;
		}
		else {
			node = ip->CreateObjectNode(newObj);
			assert(node != NULL);
			SuspendAnimate();
			node->SetNodeTM(t, entry.mpHead->GetObjTMBeforeWSM(t));
			ResumeAnimate();
			entry.mpHead->AttachChild(node);

			if (mgr != NULL) {
				INodeTab nodes;
				nodes.Append(1, &node);
				mgr->Attach(&nodes, entry.mpHead);
			}

			INodeGIProperties* giHead = static_cast<INodeGIProperties*>(
				entry.mpHead->GetInterface(NODEGIPROPERTIES_INTERFACE));
			INodeGIProperties* giNode = static_cast<INodeGIProperties*>(
				node->GetInterface(NODEGIPROPERTIES_INTERFACE));
			if (giHead != NULL && giNode != NULL) {
                giNode->CopyGIPropertiesFrom(*giHead);
			}

			if (giNode != NULL) {
				bool storeDirect = _storeDirect;
				if (entry.mpNode != NULL) {
					INodeGIProperties* gi = static_cast<INodeGIProperties*>(
						entry.mpNode->GetInterface(NODEGIPROPERTIES_INTERFACE));
					storeDirect = gi->GIGetStoreIllumToMesh() != 0;
				}
				giNode->GISetStoreIllumToMesh(storeDirect);
			}

			if (newObj->SuperClassID() == LIGHT_CLASS_ID)
				ip->AddLightToScene(node);

			TSTR name;
			if (entry.mpNode != NULL) {
				name = entry.mpNode->GetName();
				DeleteNode(ip, node, entry.mpNode);
			}
			else {
				name = baseName;
				ip->MakeNameUnique(name);
			}

			node->SetName(name);
		}

		lastHead = entry.mpHead;
	}
}

int NatLightAssembly::ReplaceSunAndSkyNodes::proc(
	INode*		node
)
{
	int count = mNodes.Count();
	// This node has a natural light assembly attached.
	// Find the objects and replace the nodes.
	if (mpOld != NULL) {
		// Find the existing object in the assembly
		traverse(node, kHead | kOpenSubAssemblies
			| kClosedSubAssemblies);
	}

	if (count == mNodes.Count()) {
		// We don't have an existing object, so
		// we will just add an entry to create a new one.
		ObjEntry entry(node, NULL);
		mNodes.Append(1, &entry, 3);
	}

	return DEP_ENUM_SKIP;
}

bool __fastcall NatLightAssembly::ReplaceSunAndSkyNodes::proc(
	INode*		head,
	INode*		node,
	IAssembly*
)
{
	Object* obj = node->GetObjectRef();
	if (obj != NULL) {
		obj = obj->FindBaseObject();
		if (obj == mpOld) {
			ObjEntry entry(head, node);
			mNodes.Append(1, &entry, 3);
		}
	}

	return true;
}

/*===========================================================================*\
 | Natural Light Assembly Find Manual Setting Class
\*===========================================================================*/

int NatLightAssembly::FindManual::FindManualSetting()
{
	FindHeadNodes();
	return mManualCount == 0
		? (mDateTimeCount == 0 ? 2 : 0)
		: (mDateTimeCount == 0 ? 1 : 2);
}

int NatLightAssembly::FindManual::proc(INode* head)
{
	Control* tm = head->GetTMController();
	ReferenceTarget* master;
	if (tm != NULL
			&& tm->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
				DAYLIGHT_SLAVE_CONTROL_CID2))
			&& (master = tm->GetReference(0)) != NULL
			&& master->IsSubClassOf(Class_ID(DAYLIGHT_CID1,DAYLIGHT_CID2))
			&& !static_cast<SunMaster*>(master)->GetManual(mT)) {
		++mDateTimeCount;
	}
	else
		++mManualCount;
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Set Manual Setting Class
\*===========================================================================*/

void NatLightAssembly::SetManual::SetManualSetting()
{
	FindHeadNodes();
}

int NatLightAssembly::SetManual::proc(INode* head)
{
	Control* tm = head->GetTMController();
	ReferenceTarget* master;
	if (tm != NULL
			&& tm->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
				DAYLIGHT_SLAVE_CONTROL_CID2))
			&& (master = tm->GetReference(0)) != NULL
			&& master->IsSubClassOf(Class_ID(DAYLIGHT_CID1,DAYLIGHT_CID2))) {
		static_cast<SunMaster*>(master)->SetManual(mT, mManual);
	}
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Open Assembly Class
\*===========================================================================*/

void NatLightAssembly::OpenAssembly::Open()
{
	IAssemblyMgr* mgr = GetAssemblyMgr();
	if (mgr != NULL) {
		mTab.ZeroCount();
		FindHeadNodes();
		mgr->Open(&mTab, false);
	}
}

int NatLightAssembly::OpenAssembly::proc(INode* head)
{
	mTab.Append(1, &head, 3);
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Open and Close Assembly Class
\*===========================================================================*/

void NatLightAssembly::CloseAssembly::Close()
{
	IAssemblyMgr* mgr = GetAssemblyMgr();
	if (mgr != NULL) {
		mSel.ZeroCount();
		mNotSel.ZeroCount();
		FindHeadNodes();
		if (mSel.Count() > 0)
			mgr->Close(&mSel, true);
		if (mNotSel.Count() > 0)
			mgr->Close(&mNotSel, false);
	}
}

int NatLightAssembly::CloseAssembly::proc(INode* head)
{
	if (head->Selected())
		mSel.Append(1, &head, 3);
	else
		mNotSel.Append(1, &head, 3);
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Sun Mesh Class
\*===========================================================================*/

void NatLightAssembly::SunMesh::CreateMesh(
	float	radius,
	int		nbRays,
	float	heightFactor
)
{
	const float kPI = 3.14159265358979323846f;
	float seg = float(2 * kPI / nbRays);
	float halfSeg = seg / 2.0f;
	float tip = radius * (1.0f + heightFactor * 2.0f * (1.0f - cosf(seg)));
	int i;

	setNumVerts(2 * nbRays);
	for (i = 0; i < nbRays; ++i) {
		// First the point on the circle
		float angle = i * seg;
		setVert(i + i, Point3(radius * cosf(angle), radius * sinf(angle), 0.0f));
		// Now the tip of the ray
		angle += halfSeg;
		setVert(i + i + 1, Point3(tip * cosf(angle), tip * sinf(angle), 0.0f));
	}

	setNumFaces(2 * nbRays);
	for (i = 0; i < nbRays; ++i) {
		Face* f = faces + i;

		f->v[0] = i + i;
		f->v[1] = i + i + 1;
		f->v[2] = i + i + 2;
		f->smGroup = 0;
		f->flags = EDGE_ALL;

		f = faces + i + nbRays;
		f->v[0] = i + i + 2;
		f->v[1] = i + i + 1;
		f->v[2] = i + i;
		f->smGroup = 0;
		f->flags = EDGE_ALL;
	}

	faces[nbRays - 1].v[2] = 0;
	faces[nbRays + nbRays - 1].v[0] = 0;
}
