#ifndef _NATLIGHT_H_
#define _NATLIGHT_H_

/*===========================================================================*\
	FILE: natLight.h

	DESCRIPTION: Natural Light Assembly Class.

	HISTORY: Created July 27, 2001 by Cleve Ard

	Copyright (c) 2001, All Rights Reserved.
 \*==========================================================================*/

class SunMaster;

ClassDesc* GetNatLightAssemblyDesc();

 /*===========================================================================*\
 | Natural Light Assembly Class:
\*===========================================================================*/

#define NATLIGHT_HELPER_CLASS_ID Class_ID(0x4a1e6deb, 0x31c77d57)
 
class NatLightAssembly : public HelperObject {
public:
	enum {
		PBLOCK_REF = 0,
		SUN_REF = 1,
		SKY_REF = 2,
		MULT_REF = 3,
		INTENSE_REF = 4,
		SKY_COND_REF = 5,
		NUM_REFS = 6
	};

	enum {
		NUM_SUBS = 0
	};

	enum {
		MAIN_PBLOCK = 0,
		NUM_PBLOCK = 1
	};

	enum {
		SUN_PARAM = 0,
		SKY_PARAM = 1,
		MANUAL_PARAM = 2
	};

	NatLightAssembly();
	~NatLightAssembly();

	static INode* NatLightAssembly::CreateAssembly(
		IObjCreate*			createInterface,
		NatLightAssembly*&	natObj
	);

	static void AppendAssemblyNodes(
		INode*			theAssembly,
		INodeTab&		nodes,
		SysNodeContext
	);

	void SetSunAndSky(
		TimeValue			t,
		int					ref,
		ParamID				param,
		const TCHAR*		name,
		ReferenceTarget*	obj
	);

	Object* GetSun() { return mpSunObj; }
	Object* GetSky() { return mpSkyObj; }

	void SetMultController(Control* c);
	void SetIntenseController(Control* c);
	void SetSkyCondController(Control* c);
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	SunMaster* FindMaster();
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

	// From HelperObject

	virtual void InitNodeName(TSTR& s);

	// From Object

	virtual void GetDeformBBox(
		TimeValue	t,
		Box3&		box,
		Matrix3*	tm=NULL,
		BOOL		useSel=FALSE
	);
	virtual ObjectState Eval(TimeValue t);

	// From BaseObject

	virtual void SetExtendedDisplay(int flags);
	virtual int Display(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vpt,
		int			flags
	);
	virtual int HitTest(
		TimeValue t,
		INode*		inode,
		int			type,
		int			crossing,
		int			flags,
		IPoint2*	p,
		ViewExp*	vpt
	);
	virtual void Snap(
		TimeValue	t,
		INode*		inode,
		SnapInfo*	snap,
		IPoint2*	p,
		ViewExp*	vpt
	);
	virtual void GetWorldBoundBox(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vp,
		Box3&		box
	);
	virtual void GetLocalBoundBox(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vp,
		Box3&		box
	);
	virtual BOOL HasViewDependentBoundingBox() { return true; }
	virtual CreateMouseCallBack* GetCreateMouseCallBack();
	virtual TCHAR *GetObjectName();
	virtual BOOL HasUVW();

	// From ReferneceTarget

	virtual RefTargetHandle Clone(RemapDir &remap = NoRemap());

	// From ReferenceMaker

	virtual RefResult NotifyRefChanged(
		Interval		changeInt,
		RefTargetHandle	hTarget,
		PartID&			partID,
		RefMessage		message
	);
	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);

	virtual IOResult Save(ISave *isave);
	virtual IOResult Load(ILoad *iload);

	// From Animatable

	virtual void GetClassName(TSTR& s);
	virtual Class_ID ClassID();
	virtual void DeleteThis();

	virtual void BeginEditParams(
		IObjParam*	ip,
		ULONG		flags,
		Animatable*	prev=NULL
	);
	virtual void EndEditParams(
		IObjParam*	ip,
		ULONG		flags,
		Animatable*	next=NULL
	);

	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);
	virtual BOOL IsAnimated();

	virtual int NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(short id);

	virtual void* GetInterface(ULONG id);
	virtual BaseInterface* GetInterface(Interface_ID id);
	virtual void ReleaseInterface(Interface_ID id, void* i);
	virtual int SetProperty(
		ULONG	id,
		void*	data
	);
	virtual void* GetProperty(ULONG id);

private:
	class CreateCallback;
	class ParamDlgProc;
	friend class ParamDlgProc;
	class Accessor;
	class FindAssemblyHead;
	class ReplaceSunAndSkyNodes;
	class AnySunAndSkyNodes;
	class FindManual;
	class SetManual;
	class SunMesh;
	class OpenAssembly;
	class CloseAssembly;

	static Accessor			sAccessor;
	static ParamDlgProc		sEditParams;
	static CreateCallback	sCreateCallback;
	static ParamBlockDesc2	sMainParams;
	static SunMesh			sSunMesh;

	IParamBlock2*			mpParams;
	LightObject*			mpSunObj;
	LightObject*			mpSkyObj;
	Control*				mpMult;
	Control*				mpIntense;
	Control*				mpSkyCond;
	int						mExtDispFlags;
};

void SetUpSun(SClass_ID sid, const Class_ID& systemClassID, Interface* ip, 
	GenLight* obj, const Point3& sunColor);

#endif //_NATLIGHT_H_
