/**********************************************************************
 *<
	FILE: delmod.cpp

	DESCRIPTION:  A delete spline modifier

	CREATED BY: Audrey Peterson

	HISTORY: 1/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"

#ifndef NO_MODIFIER_DELETE_SPLINE	// russom - 10/25/01

#define SDELETE_CLASS_ID		0x120f2c0c

class SDeleteMod : public Modifier {	
	public:		
		SDeleteMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s = GetString(IDS_AP_SDELETEMOD);}  
		virtual Class_ID ClassID() { return Class_ID(SDELETE_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_SDELETEMOD);}

		// From modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT;}
		Class_ID InputType() {return splineShapeClassID;}
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

class SDeleteClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SDeleteMod; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_SDELETEMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SDELETE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static SDeleteClassDesc SdeleteDesc;
ClassDesc* GetSDeleteModDesc() {return &SdeleteDesc;}


//--- Delete mod methods -------------------------------

SDeleteMod::SDeleteMod()
	{

	}

RefTargetHandle SDeleteMod::Clone(RemapDir& remap)
	{
	SDeleteMod *mod = new SDeleteMod;
	BaseClone(this, mod, remap);
	return mod;
	}

void SDeleteMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{ if (os->obj->CanConvertToType(splineShapeClassID))
  {		SplineShape *sshape = (SplineShape *)os->obj->ConvertToType(t,splineShapeClassID);
		int ss;
		switch (ss=sshape->shape.selLevel)
		{ case SHAPE_OBJECT: 
			{ for(int poly = sshape->shape.vertSel.polys - 1; poly >= 0; --poly) 
			  	 sshape->shape.vertSel[poly].SetAll(); 
			  sshape->shape.DeleteSelectedVerts(); 
			}
			break;
		  case SHAPE_VERTEX: sshape->shape.DeleteSelectedVerts();
						  break;
		  case SHAPE_SEGMENT: sshape->shape.DeleteSelectedSegs();
						 break;
		  case SHAPE_SPLINE:  sshape->shape.DeleteSelectedPolys();
						break;
		}
	}
}

#endif // NO_MODIFIER_DELETE_SPLINE
