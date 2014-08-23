/**********************************************************************
 *<
	FILE: XModifier.h

	DESCRIPTION:	Sample object, that adds an extension channel to the pipeline. Derived from GSphere

	CREATED BY:		Nikolai Sander

	HISTORY:		Created: 3/22/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __XGSPHERE__H
#define __XGSPHERE__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "meshadj.h"
#include "Simpobj.h"

#define XGSPHERE_CLASS_ID Class_ID(0x68d1123, 0x154b6707)
#define PBLOCK_REF_NO	 0
#define PBLOCK_REF	1

// block IDs
enum { geo_creation_type, geo_type_in, geo_params, x_params };

// x param IDs
enum { pb_fn_spin,pb_nf_spin,pb_fa_spin,pb_suspdisp,pb_fn_onoff,pb_nf_onoff,pb_fa_onoff,  };


ClassDesc2* GetXGSphereDesc();

// JBW: IParamArray has gone since the class variable UI paramters are stored in static ParamBlocks
//      all corresponding class vars have gone, including the ParamMaps since they are replaced 
//      by the new descriptors
class XGSphereObject : public SimpleObject2
{
		IParamBlock2 *pblock2_x;
		bool bDisableXObjs;
	public:	
		// Class vars
		static IObjParam *ip;
		static BOOL typeinCreate;

// JBW: minimal constructor, call MakeAutoParamBlocks() on my ClassDesc to
//      have all the declared per-instance P_AUTO_CONSTRUCT blocks made, initialized and
//      wired in.
		XGSphereObject() { GetXGSphereDesc()->MakeAutoParamBlocks(this); bDisableXObjs = false;}
		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_XGEOSPHERE); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist);

		// From GeomObject
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return XGSPHERE_CLASS_ID; } 
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 2; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { if(i == 0) return pblock2; else return pblock2_x; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { if(pblock2->ID() == id) return pblock2; else if(pblock2_x->ID() == id) return pblock2_x; else return NULL; } // return id'd ParamBlock

// JBW: the Load() post-load callback insertion has gone since versioning is 
//		handled automatically by virtue of permanent parameter IDs.  These IDs
//		are defined in enums and are never retired so that old versions can be
//		automatically re-mapped to new ones
//
//      Note that this is only true in new plug-ins; old plug-ins need to 
//		continue to support version re-mapping as before for version up until
//		converting to the new descriptors
		IOResult Load(ILoad *iload);
		
// JBW: all the IParamArray methods are gone since we don't need them for the class variables

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
// JBW: the GetParamName() and GetParamDim() function have gone	as this all 
//      is available in the descriptors. REFMSG_GET_PARAM_NAME, etc. become unnecessary as well
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int NumRefs() { return 2;}
		
		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node, Object *obj, IDerivedObject *derObj, int index);

	protected:
		void CopyParamsToMod(XModifier *pmod);
};

//--- ClassDescriptor and class vars ---------------------------------

// The class descriptor for gsphere
class GSphereClassDesc: public ClassDesc2 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new XGSphereObject; }
	const TCHAR *	ClassName() { return GetString(IDS_XGEOSPHERE_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return XGSPHERE_CLASS_ID;}
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
// JBW:  the ResetClassParams() has gone since this is automatic now
//       using the default values in the descriptors

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("GeoSphere"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

#endif  // __XGSPHERE__H