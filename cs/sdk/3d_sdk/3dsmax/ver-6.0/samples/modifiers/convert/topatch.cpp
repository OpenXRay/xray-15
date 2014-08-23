/**********************************************************************
 *<
	FILE: ToPatch.cpp

	DESCRIPTION:  Convert to Patch Modifier

	CREATED BY: Steve Anderson

	HISTORY: Created February 2000

 *>	Copyright (c) 2000 Autodesk, All Rights Reserved.
 **********************************************************************/
#include "buildver.h"
#ifndef NO_MODIFIER_CONVERTTOPATCH // orb 02-11-2002

#include "Convert.h"

static GenSubObjType SOT_Vertex(6);
static GenSubObjType SOT_Edge(7);
static GenSubObjType SOT_Patch(8);

class ConvertToPatch : public Modifier {
public:
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static ConvertToPatch *editMod;

	ConvertToPatch();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_CONVERT_TO_PATCH); }  
	virtual Class_ID ClassID() { return CONVERT_TO_PATCH_ID; }		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_CONVERT_TO_PATCH); }

	// From modifier
	ChannelMask ChannelsUsed()  { return OBJ_CHANNELS; }
	ChannelMask ChannelsChanged() { return OBJ_CHANNELS; }
	Class_ID InputType() { return mapObjectClassID; }
	void ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t) { return GetValidity(t); }
	Interval GetValidity (TimeValue t);
	BOOL DependOnTopology(ModContext &mc) { return FALSE; }

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		

	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock (int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID (short id);

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }
	void SetReference(int i, RefTargetHandle rtarg) { pblock = (IParamBlock2 *) rtarg; }

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) {return GetString (IDS_PARAMETERS);}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message);

	void Convert (PolyObject *obj, TimeValue t, PatchMesh & pm, Interval & ivalid);
	void Convert (TriObject *obj, TimeValue t, PatchMesh & pm, Interval & ivalid);
	void Convert (PatchObject *obj, TimeValue t, PatchMesh & pm, Interval & ivalid);

	ISubObjType *GetSubObjType(int i);
	int UI2SelLevel(int selLevel);
	bool ChangesSelType();
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *ConvertToPatch::ip              = NULL;
ConvertToPatch *ConvertToPatch::editMod         = NULL;

class ConvertToPatchClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ConvertToPatch; }
	const TCHAR *	ClassName() { return GetString(IDS_CONVERT_TO_PATCH); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return CONVERT_TO_PATCH_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MOD_CATEGORY); }
	const TCHAR*	InternalName() { return _T("ConvertToPatch"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static ConvertToPatchClassDesc convertToPatchDesc;
ClassDesc* GetConvertToPatchDesc() {return &convertToPatchDesc;}

// Parameter block IDs:
// Blocks themselves:
enum { turn_params };
// Parameters in first block:
enum { turn_quads, turn_sel_type, turn_softsel, turn_sel_level };

static ParamBlockDesc2 turn_param_desc ( turn_params, _T("convertToPatchParams"),
									IDS_PARAMETERS, &convertToPatchDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI, REF_PBLOCK,
	//rollout description
	IDD_TO_PATCH, IDS_PARAMETERS, 0, 0, NULL,

	// params
	turn_quads, _T("quadsToQuads"), TYPE_BOOL, P_RESET_DEFAULT|P_ANIMATABLE, IDS_PATCH_QUADS,
		p_default, TRUE,	// Turn this to TRUE when supported.
		p_ui, TYPE_SINGLECHEKBOX, IDC_QUADS,
		end,

	turn_sel_type, _T("selectionConversion"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_TYPE,
		p_default, 0,	// Preserve Selection
		p_ui, TYPE_RADIO, 3, IDC_SEL_PRESERVE, IDC_SEL_CLEAR, IDC_SEL_INVERT,
		end,

	turn_softsel, _T("useSoftSelection"), TYPE_INT, P_RESET_DEFAULT, IDS_USE_SOFTSEL,
		p_default, true,	// Do engage automatically.
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	turn_sel_level, _T("selectionLevel"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_LEVEL,
		p_default, 0,	// Preserve selection level
		p_ui, TYPE_RADIO, 5, IDC_SEL_PIPELINE, IDC_SEL_OBJ, 
			IDC_SEL_VERT, IDC_SEL_EDGE, IDC_SEL_PATCH,
		end,
	end
);

//--- Modifier methods -------------------------------

ConvertToPatch::ConvertToPatch() {
	pblock = NULL;
	GetConvertToPatchDesc()->MakeAutoParamBlocks(this);
}

RefTargetHandle ConvertToPatch::Clone(RemapDir& remap) {
	ConvertToPatch *mod = new ConvertToPatch();
	mod->ReplaceReference (0, pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

IParamBlock2 *ConvertToPatch::GetParamBlockByID (short id) {
	return (pblock->ID() == id) ? pblock : NULL; 
}

Interval ConvertToPatch::GetValidity (TimeValue t) {
	Interval ret = FOREVER;
	pblock->GetValidity (t, FOREVER);
	return ret;
}

void ConvertToPatch::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->CanConvertToType (triObjectClassID)) return;

	Interval ivalid=os->obj->ObjectValidity (t);
	TriObject *triObj;
	PatchObject *patchObj;
	PolyObject *polyObj;

	PatchObject *pobj = new PatchObject;
	PatchMesh & pm = pobj->patch;

	if (os->obj->IsSubClassOf (patchObjectClassID)) {
		patchObj = (PatchObject *) os->obj;
		Convert (patchObj, t, pm, ivalid);
	} else {
		if (os->obj->IsSubClassOf (triObjectClassID)) {
			triObj = (TriObject *) os->obj;
			Convert (triObj, t, pm, ivalid);
		} else {
			if (os->obj->IsSubClassOf (polyObjectClassID)) {
				polyObj = (PolyObject *) os->obj;
				Convert (polyObj, t, pm, ivalid);
			} else {
				// Some other kind of object: See if it can convert to patch, then fall back on Mesh.
				if (os->obj->CanConvertToType (patchObjectClassID)) {
					patchObj = (PatchObject *) os->obj->ConvertToType (t, patchObjectClassID);
					Convert (patchObj, t, pm, ivalid);
					if (patchObj != os->obj) delete patchObj;
				} else {
					triObj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
					Convert (triObj, t, pm, ivalid);
					if (triObj != os->obj) delete triObj;
				}
			}
		}
	}

	// Handle Selection Conversion
	int selConv, selLevel, use_ss;

	pblock->GetValue (turn_sel_type, t, selConv, ivalid);
	pblock->GetValue (turn_sel_level, t, selLevel, ivalid);
	pblock->GetValue (turn_softsel, t, use_ss, ivalid);

	switch (selConv) {
	case 0: // Preserve selection
		if (!use_ss) pm.SetUseSoftSelections (false);
		break;

	case 1:	// Clear selection
		pm.vertSel.ClearAll ();
		pm.edgeSel.ClearAll ();
		pm.patchSel.ClearAll ();
		pm.SetUseSoftSelections (false);
		break;

	case 2: // Invert selection
		pm.vertSel = ~pm.vertSel;
		pm.edgeSel = ~pm.edgeSel;
		pm.patchSel = ~pm.patchSel;
		if (use_ss) {
			float *vsw = pm.GetVSelectionWeights();
			int numVSW = pm.numVecs + pm.numVerts;
			if (vsw) {
				for (int i=0; i<numVSW; i++) vsw[i] = 1.0f - vsw[i];
				for (i=0; i<pm.numVerts; i++) {
					if (vsw[i] < 1.0f) pm.vertSel.Clear (i);
				}
			}
		} else {
			pm.SetUseSoftSelections (false);
		}
		break;
	}

	// Set Subobject Level if needed
	if(selLevel != 0) {
		int newSelLevel = UI2SelLevel (selLevel);
		if (newSelLevel != pm.selLevel) {
			// Lose the soft selection info
			pm.SetUseSoftSelections (false);
			pm.selLevel = newSelLevel;
		}
	}

	// Set display flags:
	switch (pm.selLevel) {
	case PATCH_OBJECT:
		pm.dispFlags = 0;
		break;
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case PATCH_HANDLE:
		pm.dispFlags = DISP_VERTTICKS|DISP_SELHANDLES;
		break;
	case PATCH_VERTEX:
		pm.dispFlags = DISP_VERTTICKS|DISP_SELVERTS;
		break;
	case PATCH_EDGE:
		pm.dispFlags = DISP_SELEDGES;
		break;
	case PATCH_PATCH:
		pm.dispFlags = DISP_SELPATCHES;
		break;
	}

	pobj->UpdateValidity (GEOM_CHAN_NUM, ivalid);
	pobj->UpdateValidity (TOPO_CHAN_NUM, ivalid);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, ivalid);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, ivalid);
	pobj->UpdateValidity (SELECT_CHAN_NUM, ivalid);
	pobj->UpdateValidity (SUBSEL_TYPE_CHAN_NUM, ivalid);
	pobj->UpdateValidity (DISP_ATTRIB_CHAN_NUM, ivalid);
	pobj->UpdateValidity (GFX_DATA_CHAN_NUM, ivalid);
	os->obj = (Object *) pobj;
}

void ConvertToPatch::Convert (PolyObject *obj, TimeValue t, 
							  PatchMesh & pm, Interval & ivalid) {
	int quads, selConvert, softSel;
	DWORD flags=0;
	pblock->GetValue (turn_quads, t, quads, ivalid);
	pblock->GetValue (turn_sel_type, t, selConvert, ivalid);

	if (selConvert != 1) {
		flags |= CONVERT_KEEPSEL;
		pblock->GetValue (turn_softsel, t, softSel, ivalid);
		if (softSel) flags |= CONVERT_USESOFTSEL;
	}
	if (quads) flags |= CONVERT_PATCH_USEQUADS;

	ConvertPolyToPatch (obj->mm, pm, flags);

	switch (obj->mm.selLevel) {
	case MNM_SL_OBJECT: pm.selLevel = PATCH_OBJECT; break;
	case MNM_SL_FACE: pm.selLevel = PATCH_PATCH; break;
	case MNM_SL_EDGE: pm.selLevel = PATCH_EDGE; break;
	case MNM_SL_VERTEX: pm.selLevel = PATCH_VERTEX; break;
	}
}

void ConvertToPatch::Convert (TriObject *obj, TimeValue t,
							  PatchMesh & pm, Interval & ivalid) {
	int quads, selConvert, softSel;
	DWORD flags=0;
	pblock->GetValue (turn_quads, t, quads, ivalid);
	pblock->GetValue (turn_sel_type, t, selConvert, ivalid);

	if (selConvert != 1) {
		flags |= CONVERT_KEEPSEL;
		pblock->GetValue (turn_softsel, t, softSel, ivalid);
		if (softSel) flags |= CONVERT_USESOFTSEL;
	}
	if (quads) flags |= CONVERT_PATCH_USEQUADS;

	ConvertMeshToPatch (obj->mesh, pm, flags);

	switch (obj->mesh.selLevel) {
	case MESH_OBJECT: pm.selLevel = PATCH_OBJECT; break;
	case MESH_FACE: pm.selLevel = PATCH_PATCH; break;
	case MESH_EDGE: pm.selLevel = PATCH_EDGE; break;
	case MESH_VERTEX: pm.selLevel = PATCH_VERTEX; break;
	}
}

void ConvertToPatch::Convert (PatchObject *obj, TimeValue t,
							  PatchMesh & pm, Interval & ivalid) {
	pm = obj->patch;
}

void ConvertToPatch::BeginEditParams (IObjParam  *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;
	editMod  = this;

	// throw up all the appropriate auto-rollouts
	convertToPatchDesc.BeginEditParams(ip, this, flags, prev);

	// Necessary?
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void ConvertToPatch::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	convertToPatchDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	editMod  = NULL;
}

RefResult ConvertToPatch::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
										   PartID& partID, RefMessage message) {
	switch (message) {
	case REFMSG_CHANGE:
		if (editMod!=this) break;
		// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
		// will contain ID to update, else it will be -1 => inval whole rollout
		if (pblock->LastNotifyParamID() == turn_sel_level) {
			// Notify stack that subobject info has changed:
			NotifyDependents(changeInt, partID, message);
			NotifyDependents(FOREVER, 0, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
			return REF_STOP;
		}
		turn_param_desc.InvalidateUI(pblock->LastNotifyParamID());
		break;
	}
	return REF_SUCCEED;
}

int ConvertToPatch::UI2SelLevel (int selLevel) {
	switch (selLevel) {
	case 1: return PATCH_OBJECT;
	case 2: return PATCH_VERTEX;
	case 3: return PATCH_EDGE;
	case 4: return PATCH_PATCH;
	}
	// have to have a default...
	return PATCH_VERTEX;
}

ISubObjType *ConvertToPatch::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_VERTEX));
		SOT_Edge.SetName(GetString(IDS_EDGE));
		SOT_Patch.SetName(GetString(IDS_PATCH));
	}

	switch(i) {
	case -1:
		{
			int selLevel;
			Interval ivalid = FOREVER;

			pblock->GetValue (turn_sel_level, GetCOREInterface()->GetTime(), selLevel, ivalid);
			
			if(selLevel == 0) return NULL;
			else {
				selLevel = UI2SelLevel(selLevel);
				switch (selLevel) {
				case PATCH_VERTEX: return &SOT_Vertex;
				case PATCH_EDGE: return &SOT_Edge;
				case PATCH_PATCH: return &SOT_Patch;
				default: return NULL;
				}
			}
		}
		break;
	case 0: return &SOT_Vertex;
	case 1: return &SOT_Edge;
	case 2: return &SOT_Patch;
	}
	return NULL;
}

bool ConvertToPatch::ChangesSelType() {
	int selLevel;
	Interval ivalid = FOREVER;
	
	pblock->GetValue (turn_sel_level, GetCOREInterface()->GetTime(), selLevel, ivalid);
	if(selLevel == 0) return false;
	else return true;
}

#endif // NO_MODIFIER_CONVERTTOPATCH
