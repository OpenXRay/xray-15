/**********************************************************************
 *<
	FILE: delmod.cpp

	DESCRIPTION:  A deletion modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/23/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"

#ifndef NO_MODIFIER_DELETE_MESH		// russom - 10/11/01

class DeleteMod : public Modifier {	
	public:		
		DeleteMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_DELETEMOD);}  
		virtual Class_ID ClassID() { return Class_ID(DELETE_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_DELETEMOD);}

		// From modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR|TEXMAP_CHANNEL;}
		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_VERTCOLOR|TEXMAP_CHANNEL;}
		Class_ID InputType() {return triObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t) {return FOREVER;}

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		
		int NumRefs() {return 0;}
		RefTargetHandle GetReference(int i) {return NULL;}
		void SetReference(int i, RefTargetHandle rtarg) {}

		int NumSubs() {return 0;}
		Animatable* SubAnim(int i) {return NULL;}
		TSTR SubAnimName(int i) {return _T("");}

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message) {return REF_SUCCEED;}
	};


//--- ClassDescriptor and class vars ---------------------------------

class DeleteClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new DeleteMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_DELETEMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(DELETE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static DeleteClassDesc deleteDesc;
ClassDesc* GetDeleteModDesc() {return &deleteDesc;}


//--- Delete mod methods -------------------------------

DeleteMod::DeleteMod()
	{

	}

RefTargetHandle DeleteMod::Clone(RemapDir& remap)
	{
	DeleteMod *mod = new DeleteMod();
	BaseClone(this, mod, remap);
	return mod;
	}

void DeleteMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		tobj->GetMesh().DeleteSelected();
		tobj->GetMesh().InvalidateTopologyCache ();
		tobj->GetMesh().InvalidateGeomCache ();
		}
	}

#endif // NO_MODIFIER_DELETE_MESH
