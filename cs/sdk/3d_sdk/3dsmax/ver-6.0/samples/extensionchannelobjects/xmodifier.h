/**********************************************************************
 *<
	FILE: XModifier.h

	DESCRIPTION:	Sample modifier, that adds an extension channel to the pipeline

	CREATED BY:		Nikolai Sander

	HISTORY:		Created: 3/22/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __XMODIFIER__H
#define __XMODIFIER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "XTCObject.h"
#include "meshadj.h"



extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

class XModifier;

#define XMODIFIER_CLASS_ID	Class_ID(0x6ad61243, 0x36f6f68f)

#define XTCSAMPLE_CHAN_CHANGED PART_GEOM|PART_TOPO|PART_SELECT|PART_VERTCOLOR|TEXMAP_CHANNEL

class XTCSample : public XTCObject
{
	friend class XGSphereObject;
	friend class XModifier;

	float size;
	BOOL bSuspDisp;
	BOOL bFN_OnOff;
	BOOL bNF_OnOff;
	BOOL bFA_OnOff;

	BaseObject *bo;
public:
	XTCSample(BaseObject *BaseObj, float size, BOOL suspDisp, BOOL fn_OnOff, BOOL nf_OnOff, BOOL fa_OnOff);
	XTCSample(XTCSample *mFrom);
	~XTCSample();
	virtual Class_ID ExtensionID(){return XMODIFIER_CLASS_ID;}
	virtual XTCObject *Clone(){return new XTCSample(this);}
	virtual void DeleteThis(){delete this;}
	virtual int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj);
	virtual ChannelMask DependsOn(){return GEOM_CHANNEL|TOPO_CHANNEL;}
	virtual ChannelMask ChannelsChanged(){return XTCSAMPLE_CHAN_CHANGED;}

	virtual void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline);
	virtual void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline);
	
	virtual BOOL SuspendObjectDisplay();
	virtual void MaybeEnlargeViewportRect(GraphicsWindow *gw, Rect &rect);
	
protected:
	void DeleteFaces(TimeValue t,Object *obj);
	Mesh *GetMesh(Object *obj);
	virtual int  DisplayMesh(TimeValue t, INode* inode, ViewExp *vpt, int flags, Mesh *mesh);
#ifndef NO_PATCHES
	virtual int  DisplayPatch(TimeValue t, INode* inode, ViewExp *vpt, int flags, PatchObject *patch);
#endif
};
struct teststruct {
	int a;
	int b;
};

class XModifier : public Modifier {
	BOOL bFN_OnOff;
	BOOL bNF_OnOff;
	BOOL bFA_OnOff;

	BOOL bModDisabled;
	Tab<teststruct *> testtab;
	public:
		// Parameter block
		IParamBlock2	*pblock;	//ref 0
		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
		//TODO: Add the channels that the modifier needs to perform its modification
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL | TOPO_CHANNEL | EXTENSION_CHANNEL; }
		
		
		// We have to include the channels, that the extension object changes, so the 
		// PostChanChangedNotify will be called after the modifier added the extension objects
		// to the object flowing up the stack.

		ChannelMask ChannelsChanged() { return EXTENSION_CHANNEL|XTCSAMPLE_CHAN_CHANGED; }
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
		Class_ID ClassID() {return XMODIFIER_CLASS_ID;}		
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
		void DeleteThis() { delete this; }		

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node, Object *obj, IDerivedObject *derObj, int index);

		//Constructor/Destructor
		XModifier();
		~XModifier();		
};

class XModifierClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new XModifier();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return XMODIFIER_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("XModifier"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};



#endif // __XMODIFIER__H
