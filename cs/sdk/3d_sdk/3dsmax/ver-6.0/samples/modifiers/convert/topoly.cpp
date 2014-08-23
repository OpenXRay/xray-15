/**********************************************************************
 *<
	FILE: ToPoly.cpp

	DESCRIPTION:  Convert to PolyMesh Modifier

	CREATED BY: Steve Anderson

	HISTORY: Created November 22, 1998

 *>	Copyright (c) 1998 Autodesk, All Rights Reserved.
 **********************************************************************/

#include "Convert.h"

static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Face(4);

class ConvertToPoly : public Modifier {
public:
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static ConvertToPoly *editMod;

	ConvertToPoly();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_CONVERT_TO_POLY); }  
	virtual Class_ID ClassID() { return CONVERT_TO_POLY_ID; }		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_CONVERT_TO_POLY); }

	// From modifier
	ChannelMask ChannelsUsed()  { return OBJ_CHANNELS; }
	ChannelMask ChannelsChanged() { return OBJ_CHANNELS; }
	Class_ID InputType() { return mapObjectClassID; }
	void ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t) { return GetValidity(t); }
	Interval GetValidity (TimeValue t);
	BOOL DependOnTopology (ModContext &mc) { return FALSE; }

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

	void Convert (PolyObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid);
	void Convert (TriObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid);
	void Convert (PatchObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid);

	ISubObjType *GetSubObjType(int i);
	int UI2SelLevel(int selLevel);
	bool ChangesSelType();
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *ConvertToPoly::ip              = NULL;
ConvertToPoly *ConvertToPoly::editMod         = NULL;

class ConvertToPolyClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ConvertToPoly; }
	const TCHAR *	ClassName() { return GetString(IDS_CONVERT_TO_POLY); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return CONVERT_TO_POLY_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MOD_CATEGORY); }
	const TCHAR*	InternalName() { return _T("ConvertToPoly"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static ConvertToPolyClassDesc convertToPolyDesc;
ClassDesc* GetConvertToPolyDesc() {return &convertToPolyDesc;}


// Parameter block IDs:
// Blocks themselves:
enum { turn_params };
// Parameters in first block:
enum { turn_keep_convex, turn_limit_size, turn_max_size,
turn_planar, turn_thresh, turn_sel_type, turn_softsel, turn_sel_level,
turn_eliminate_collinear };

static ParamBlockDesc2 turn_param_desc ( turn_params, _T("convertToPolyParams"),
									IDS_PARAMETERS, &convertToPolyDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI, REF_PBLOCK, // This is where the pblock refid goes?
	//rollout description
	IDD_TO_POLY, IDS_PARAMETERS, 0, 0, NULL,

	// params
	turn_keep_convex, _T("keepConvex"), TYPE_BOOL, P_RESET_DEFAULT|P_ANIMATABLE, IDS_KEEP_CONVEX,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_KEEP_CONVEX,
		end,

	turn_limit_size, _T("limitPolySize"), TYPE_BOOL, P_RESET_DEFAULT|P_ANIMATABLE, IDS_LIMIT_SIZE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIMIT_SIZE,
		p_enable_ctrls,	1, turn_max_size,
		end,

	turn_max_size, _T("maxPolySize"), TYPE_INT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_MAX_SIZE,
		p_default, 4,
		p_range, 3, 99999,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_MAX_SIZE, IDC_MAX_SIZESPIN, .5f,
		end,

	turn_planar, _T("requirePlanar"), TYPE_BOOL, P_RESET_DEFAULT|P_ANIMATABLE, IDS_PLANAR,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_PLANAR,
		p_enable_ctrls,	1, turn_thresh,
		end,

	turn_thresh, _T("planarThresh"), TYPE_ANGLE, P_RESET_DEFAULT|P_ANIMATABLE, IDS_THRESH,
		p_default, PI/12.f,	// 15 degrees.
		p_range, 0.0, 180.0,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_THRESH, IDC_THRESHSPIN, SPIN_AUTOSCALE,
		end,

	turn_eliminate_collinear, _T("removeMidEdgeVertices"), TYPE_BOOL, P_RESET_DEFAULT, IDS_REMOVE_MID_EDGE_VERTICES,
		p_default, true,
		p_ui, TYPE_SINGLECHEKBOX, IDC_ELIMINATE_COLLINEAR,
		end,

	turn_sel_type, _T("selectionConversion"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_TYPE,
		p_default, 0,	// Preserve selection
		p_ui, TYPE_RADIO, 3, IDC_SEL_PRESERVE, IDC_SEL_CLEAR, IDC_SEL_INVERT,
		end,

	turn_softsel, _T("useSoftSelection"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_SOFTSEL,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	turn_sel_level, _T("selectionLevel"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_LEVEL,
		p_default, 0,	// pipeline selection level
		p_ui, TYPE_RADIO, 5, IDC_SEL_PIPELINE, IDC_SEL_OBJ, IDC_SEL_VERT,
			IDC_SEL_EDGE, IDC_SEL_FACE,
		end,
	end
);

//--- Modifier methods -------------------------------

ConvertToPoly::ConvertToPoly() {
	pblock = NULL;
	GetConvertToPolyDesc()->MakeAutoParamBlocks(this);
}

RefTargetHandle ConvertToPoly::Clone(RemapDir& remap) {
	ConvertToPoly *mod = new ConvertToPoly();
	mod->ReplaceReference (0, pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

IParamBlock2 *ConvertToPoly::GetParamBlockByID (short id) {
	return (pblock->ID() == id) ? pblock : NULL; 
}

Interval ConvertToPoly::GetValidity (TimeValue t) {
	Interval ret = FOREVER;
	pblock->GetValidity (t, FOREVER);
	return ret;
}

void ConvertToPoly::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->CanConvertToType (triObjectClassID)) return;

	Interval ivalid=os->obj->ObjectValidity (t);
	TriObject *triObj;
	PolyObject *polyObj;

	PolyObject *pobj = new PolyObject ();

#ifndef NO_PATCHES  // orb 02-11-2002
	PatchObject *patchObj;

	if (os->obj->IsSubClassOf (patchObjectClassID)) {
		patchObj = (PatchObject *) os->obj;
		Convert (patchObj, t, pobj->mm, ivalid);
	} else 
#endif // NO_PATCHES
	{
		if (os->obj->IsSubClassOf (triObjectClassID)) {
			triObj = (TriObject *) os->obj;
			Convert (triObj, t, pobj->mm, ivalid);
			pobj->SetDisplacementDisable (triObj->mDisableDisplacement);
			pobj->SetDisplacementSplit (triObj->mSplitMesh);
			pobj->SetDisplacementParameters (triObj->mDispApprox);
			pobj->SetDisplacement (triObj->mSubDivideDisplacement);
		} else {
			if (os->obj->IsSubClassOf (polyObjectClassID)) {
				polyObj = (PolyObject *) os->obj;
				Convert (polyObj, t, pobj->mm, ivalid);
				pobj->SetDisplacementDisable (polyObj->GetDisplacementDisable ());
				pobj->SetDisplacementSplit (polyObj->GetDisplacementSplit());
				pobj->SetDisplacementParameters (polyObj->GetDisplacementParameters());
				pobj->SetDisplacement (polyObj->GetDisplacement ());
			} else {
				if (os->obj->CanConvertToType (polyObjectClassID)) {
					polyObj = (PolyObject *) os->obj->ConvertToType (t, polyObjectClassID);
					Convert (polyObj, t, pobj->mm, ivalid);
					if (polyObj != os->obj) delete polyObj;
				} else {
					triObj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
					Convert (triObj, t, pobj->mm, ivalid);
					if (triObj != os->obj) delete triObj;
				}
			}
		}
	}

	// Handle Selection Conversion
	int selConv, useSoftSel;
	int selLevel;
	pblock->GetValue (turn_sel_type, t, selConv, ivalid);
	pblock->GetValue (turn_softsel, t, useSoftSel, ivalid);
	pblock->GetValue (turn_sel_level, t, selLevel, ivalid);
	if (selLevel && (pobj->mm.selLevel != UI2SelLevel(selLevel))) useSoftSel = false;
	if (!useSoftSel) pobj->mm.freeVSelectionWeights ();
	int i;
	float *vsel;
	switch (selConv) {
	case 1:	// Clear selection
		pobj->mm.ClearVFlags (MN_SEL);
		pobj->mm.ClearEFlags (MN_SEL);
		pobj->mm.ClearFFlags (MN_SEL);
		pobj->mm.freeVSelectionWeights ();
		break;

	case 2: // Invert selection
		vsel = pobj->mm.getVSelectionWeights ();
		for (i=0; i<pobj->mm.numv; i++) {
			if (pobj->mm.v[i].GetFlag (MN_SEL)) pobj->mm.v[i].ClearFlag (MN_SEL);
			else pobj->mm.v[i].SetFlag (MN_SEL);
			if (vsel) {
				vsel[i] = 1.0f - vsel[i];
				if (vsel[i]<1) pobj->mm.v[i].ClearFlag (MN_SEL);
			}
		}
		for (i=0; i<pobj->mm.nume; i++) {
			if (pobj->mm.e[i].GetFlag (MN_SEL)) pobj->mm.e[i].ClearFlag (MN_SEL);
			else pobj->mm.e[i].SetFlag (MN_SEL);
		}
		for (i=0; i<pobj->mm.numf; i++) {
			if (pobj->mm.f[i].GetFlag (MN_SEL)) pobj->mm.f[i].ClearFlag (MN_SEL);
			else pobj->mm.f[i].SetFlag (MN_SEL);
		}
	}

	// Set Subobject Level if needed
	if(selLevel != 0)
		pobj->mm.selLevel = UI2SelLevel(selLevel);

	// Set display flags:
	pobj->mm.dispFlags = 0;
	switch (pobj->mm.selLevel) {
	case MNM_SL_VERTEX:
		pobj->mm.SetDispFlag (MNDISP_VERTTICKS|MNDISP_SELVERTS);
		break;
	case MNM_SL_EDGE:
		pobj->mm.SetDispFlag (MNDISP_SELEDGES);
		break;
	case MNM_SL_FACE:
		pobj->mm.SetDispFlag (MNDISP_SELFACES);
		break;
	}
	pobj->SetPartValidity (OBJ_CHANNELS, ivalid);
	if (os->obj != pobj) os->obj = (Object *) pobj;
}

void ConvertToPoly::Convert (PolyObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid) {
	int keepConvex;
	int limitSize;
	int maxdeg=0;
	int keepPlanar, elimCollin;
	float planarThresh;

	pblock->GetValue (turn_keep_convex, t, keepConvex, ivalid);
	pblock->GetValue (turn_limit_size, t, limitSize, ivalid);
	if (limitSize) pblock->GetValue (turn_max_size, t, maxdeg, ivalid);
	pblock->GetValue (turn_planar, t, keepPlanar, ivalid);
	if (keepPlanar) {
		pblock->GetValue (turn_thresh, t, planarThresh, ivalid);
		planarThresh = cosf (planarThresh);
	}
	pblock->GetValue (turn_eliminate_collinear, t, elimCollin, ivalid);

	mm = obj->mm;

	// Luna task 747
	// We cannot support specified normals in Convert to Poly at this time.
	mm.ClearSpecifiedNormals();

	if (!mm.GetFlag (MN_MESH_FILLED_IN)) mm.FillInMesh ();
	if (!mm.GetFlag (MN_MESH_NO_BAD_VERTS)) mm.EliminateBadVerts ();

	if (maxdeg) mm.RestrictPolySize (maxdeg);
	if (keepConvex) mm.MakeConvex ();
	if (maxdeg || keepConvex) mm.ClearEFlags (MN_EDGE_INVIS);
	if (keepPlanar) mm.MakePlanar (planarThresh);
	if (elimCollin) mm.EliminateCollinearVerts ();
	mm.selLevel = obj->mm.selLevel;
}

void ConvertToPoly::Convert (TriObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid) {
	int keepConvex;
	int limitSize;
	int maxdeg=0;
	int keepPlanar;
	float planarThresh;
	int elimCollin;

	pblock->GetValue (turn_keep_convex, t, keepConvex, ivalid);
	pblock->GetValue (turn_limit_size, t, limitSize, ivalid);
	if (limitSize) pblock->GetValue (turn_max_size, t, maxdeg, ivalid);
	pblock->GetValue (turn_planar, t, keepPlanar, ivalid);
	if (keepPlanar) {
		pblock->GetValue (turn_thresh, t, planarThresh, ivalid);
		planarThresh = cosf (planarThresh);
	}
	pblock->GetValue (turn_eliminate_collinear, t, elimCollin, ivalid);

	mm.AddTri (obj->mesh);
	mm.FillInMesh ();
	mm.EliminateBadVerts ();
	if (maxdeg != 3) {
		if (keepPlanar) mm.FenceNonPlanarEdges (planarThresh, TRUE);
		mm.MakePolyMesh (maxdeg, elimCollin);
		if (keepConvex) mm.MakeConvex ();
	}
	mm.ClearEFlags (MN_EDGE_INVIS);
	switch (obj->mesh.selLevel) {
	case MESH_VERTEX: mm.selLevel = MNM_SL_VERTEX; break;
	case MESH_EDGE: mm.selLevel = MNM_SL_EDGE; break;
	case MESH_FACE: mm.selLevel = MNM_SL_FACE; break;
	default: mm.selLevel = MNM_SL_OBJECT; break;
	}
}

void ConvertToPoly::Convert (PatchObject *obj, TimeValue t, MNMesh & mm, Interval & ivalid) {
	int selConv, useSoftSel;
	int keepConvex, limitSize;
	int maxdeg=0;
	int keepPlanar;
	float planarThresh;
	int elimCollin;

	pblock->GetValue (turn_sel_type, t, selConv, ivalid);
	pblock->GetValue (turn_softsel, t, useSoftSel, ivalid);
	pblock->GetValue (turn_keep_convex, t, keepConvex, ivalid);
	pblock->GetValue (turn_limit_size, t, limitSize, ivalid);
	if (limitSize) pblock->GetValue (turn_max_size, t, maxdeg, ivalid);
	pblock->GetValue (turn_planar, t, keepPlanar, ivalid);
	if (keepPlanar) {
		pblock->GetValue (turn_thresh, t, planarThresh, ivalid);
		planarThresh = cosf (planarThresh);
	}
	pblock->GetValue (turn_eliminate_collinear, t, elimCollin, ivalid);

	DWORD flags=0;
	if (selConv != 1) {
		flags = CONVERT_KEEPSEL;
		if (useSoftSel) flags |= CONVERT_USESOFTSEL;
	}

	ConvertPatchToPoly (obj->patch, mm, flags);
	if (maxdeg) mm.RestrictPolySize (maxdeg);
	if (keepConvex) mm.MakeConvex ();
	if (keepPlanar) mm.MakePlanar (planarThresh);
	if (elimCollin) mm.EliminateCollinearVerts ();

	switch (obj->patch.selLevel) {
	case PATCH_VERTEX: mm.selLevel = MNM_SL_VERTEX; break;
	case PATCH_EDGE: mm.selLevel = MNM_SL_EDGE; break;
	case PATCH_PATCH: mm.selLevel = MNM_SL_FACE; break;
	default: mm.selLevel = MNM_SL_OBJECT; break;
	}
}

void ConvertToPoly::BeginEditParams (IObjParam  *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;	
	editMod  = this;

	// throw up all the appropriate auto-rollouts
	convertToPolyDesc.BeginEditParams(ip, this, flags, prev);

	// Necessary?
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void ConvertToPoly::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	convertToPolyDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	editMod  = NULL;
}

RefResult ConvertToPoly::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
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

int ConvertToPoly::UI2SelLevel(int selLevel)
{
	switch (selLevel) {
	case 1: return MNM_SL_OBJECT; break;
	case 2: return MNM_SL_VERTEX; break;
	case 3: return MNM_SL_EDGE; break;
	case 4: return MNM_SL_FACE; break;
	default: return 4; break;
	}
}

ISubObjType *ConvertToPoly::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_VERTEX));
		SOT_Edge.SetName(GetString(IDS_EDGE));
		SOT_Face.SetName(GetString(IDS_FACE));
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
				return selLevel > 0 ? GetSubObjType(selLevel-1) : NULL;
			}
		}
		break;
	case 0: return &SOT_Vertex;
	case 1: return &SOT_Edge;
	case 2: return &SOT_Face;
	}
	return NULL;
}

bool ConvertToPoly::ChangesSelType()
{
	int selLevel;
	Interval ivalid = FOREVER;
	
	pblock->GetValue (turn_sel_level, GetCOREInterface()->GetTime(), selLevel, ivalid);
	if(selLevel == 0) return false;
	else return true;
}
