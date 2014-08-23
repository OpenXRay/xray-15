/**********************************************************************
 *<
	FILE: vweld.cpp

	DESCRIPTION: Vertex Weld Modifier (for Meshes, PolyMeshes, Patches.)

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig.

	HISTORY: created 9/1/2001

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"

const Class_ID kVERTEX_WELD_CLASS_ID(0x709029e0, 0x2cfa07bd);
const unsigned int kVW_PBLOCK_REF(0);

class VWeldMod : public Modifier {	
	IParamBlock2 *mp_pblock;

public:
	VWeldMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_VERTEX_WELD_MOD);}  
	virtual Class_ID ClassID () { return kVERTEX_WELD_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_VERTEX_WELD_MOD); }

	void BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier
	// Since we're changing topology, all these other channels are affected as well:
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR; }
	ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

	// ParamBlock2 access:
	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	// Reference Management:
	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// Animatable management:
	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return mp_pblock;}
	TSTR SubAnimName(int i) { return _T(""); }

#ifndef NO_PATCHES
	// Local methods
	void ConvertPatchSelection (PatchMesh & mesh);
	void ModifyPatchObject(TimeValue t, ModContext &mc, PatchObject *pobj);
#endif

	void SetPolyFlags (MNMesh & mesh, DWORD flag);
	bool WeldShortPolyEdges (MNMesh & mesh, float thresh, DWORD flag);
	void ModifyPolyObject(TimeValue t, ModContext &mc, PolyObject *pobj);

	void ConvertTriSelection (Mesh & mesh, BitArray & targetVerts);
	void ModifyTriObject(TimeValue t, ModContext &mc, TriObject *tobj);
};

//--- ClassDescriptor and class vars ---------------------------------

class VertexWeldClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new VWeldMod; }
	const TCHAR *	ClassName() { return GetString(IDS_VERTEX_WELD_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kVERTEX_WELD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("VertexWeld"); }
	HINSTANCE HInstance() { return hInstance; }
};

static VertexWeldClassDesc vweldDesc;
extern ClassDesc* GetVertexWeldModDesc() {return &vweldDesc;}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kVertexWeldParams };

// And enumerate the parameters within that block:
enum { kVwThreshold };

// Parameters
static ParamBlockDesc2 vertex_weld_param_blk (kVertexWeldParams, _T("Vertex Weld Parameters"), 0, &vweldDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kVW_PBLOCK_REF,
	// rollout
	IDD_VW_PARAM, IDS_PARAMETERS, 0, 0, NULL,

	// Parameters
	kVwThreshold, _T("threshold"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_VW_THRESHOLD,
		p_default, 0.1f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_VW_THRESH, IDC_VW_THRESHSPIN, SPIN_AUTOSCALE,
		end,

	end
);

//--- VWeldMod methods -------------------------------


VWeldMod::VWeldMod() {
	mp_pblock = NULL;
	vweldDesc.MakeAutoParamBlocks(this);
}

void VWeldMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	vweldDesc.BeginEditParams(ip,this,flags,prev);
}

void VWeldMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	vweldDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle VWeldMod::Clone(RemapDir& remap) {
	VWeldMod *mod = new VWeldMod();
	mod->ReplaceReference(kVW_PBLOCK_REF,mp_pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

void VWeldMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (os->obj->IsSubClassOf(triObjectClassID))
		ModifyTriObject (t, mc, (TriObject *)os->obj);
	else {
		if (os->obj->IsSubClassOf (polyObjectClassID))
			ModifyPolyObject (t, mc, (PolyObject *)os->obj);
		else {
#ifndef NO_PATCHES
			if (os->obj->IsSubClassOf (patchObjectClassID))
				ModifyPatchObject (t, mc, (PatchObject *) os->obj);
			else 
#endif
			{
				if (os->obj->CanConvertToType (triObjectClassID)) {
					TriObject *tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
					ModifyTriObject (t, mc, tobj);
					os->obj = (Object *) tobj;
				}
			}
		}
	}
}
#ifndef NO_PATCHES

void VWeldMod::ConvertPatchSelection (PatchMesh & mesh) {
	int i;

	switch (mesh.selLevel) {
	case PATCH_OBJECT:
		mesh.vertSel.SetAll ();
		break;
	case PATCH_VERTEX:
		// Don't need to do anything.
		break;
	case PATCH_EDGE:
		mesh.vertSel.ClearAll ();
		for (i=0; i<mesh.getNumEdges(); i++) {		
			if (!mesh.edgeSel[i]) continue;
			mesh.vertSel.Set(mesh.edges[i].v1,TRUE);
			mesh.vertSel.Set(mesh.edges[i].v2,TRUE);
		}
		break;
	case PATCH_PATCH:
		mesh.vertSel.ClearAll ();
		for (i=0; i<mesh.getNumPatches(); i++) {
			if (!mesh.patchSel[i]) continue;
			for (int j=0; j<mesh.patches[i].type; j++) mesh.vertSel.Set (mesh.patches[i].v[j]);
		}
		break;
	}
}

void VWeldMod::ModifyPatchObject (TimeValue t, ModContext &mc, PatchObject *pobj) {
	PatchMesh &mesh = pobj->GetPatchMesh (t);
	Interval iv = FOREVER;
	float thresh;

	mp_pblock->GetValue (kVwThreshold, t, thresh, iv);
	if (thresh<0.0f) thresh=0.0f;

	// Convert existing selection (at whatever level) to vertex selection:
	// NOTE that if there is an incoming vertex selection, but we're at a different selection level,
	// we lose the vertex selection in this process.  Unavoidable until PatchMesh::Weld starts
	// accepting a BitArray targetVerts argument.
	ConvertPatchSelection (mesh);

	// Weld the vertices
	BOOL found = mesh.Weld (thresh);

	pobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	pobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	pobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

#endif //NO_PATCHES

void VWeldMod::SetPolyFlags (MNMesh & mesh, DWORD flag) {
	// Convert existing selection (at whatever level) to vertex selection:
	mesh.ClearVFlags (flag);
	if (mesh.selLevel == MNM_SL_OBJECT) {
		for (int i=0; i<mesh.numv; i++) mesh.v[i].SetFlag (flag);
	} else {
		mesh.PropegateComponentFlags (MNM_SL_VERTEX, flag, mesh.selLevel, MN_SEL);
	}
}

// (This code was copied from EditPolyObj::EpfnCollapse.)
bool VWeldMod::WeldShortPolyEdges (MNMesh & mesh, float thresh, DWORD flag) {
	// In order to collapse vertices, we turn them into edge selections,
	// where the edges are shorter than the weld threshold.
	bool canWeld = false;
	mesh.ClearEFlags (flag);
	float threshSq = thresh*thresh;
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag (flag)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag (flag)) continue;
		if (LengthSquared (mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2)) > threshSq) continue;
		mesh.e[i].SetFlag (flag);
		canWeld = true;
	}
	if (!canWeld) return false;

	return MNMeshCollapseEdges (mesh, MN_USER);
}

void VWeldMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj) {
	MNMesh &mesh = pobj->GetMesh();

	// Luna task 747
	// We cannot support specified normals in Vertex Weld at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	float thresh;

	mp_pblock->GetValue (kVwThreshold, t, thresh, iv);
	if (thresh<0.0f) thresh=0.0f;
	SetPolyFlags (mesh, MN_USER);

	// Weld the suitable border vertices:
	bool haveWelded = false;
	if (mesh.WeldBorderVerts (thresh, MN_USER)) {
		mesh.CollapseDeadStructs ();
		haveWelded = true;
	}

	// Weld vertices that share short edges:
	if (WeldShortPolyEdges (mesh, thresh, MN_USER)) haveWelded = true;

	if (haveWelded) {
		mesh.InvalidateTopoCache ();
		mesh.FillInMesh ();
	}

	pobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	pobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	pobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

void VWeldMod::ConvertTriSelection (Mesh & mesh, BitArray & targetVerts) {
	targetVerts.SetSize (mesh.numVerts);
	targetVerts.ClearAll ();

	int i, j;
	switch (mesh.selLevel) {
	case MESH_OBJECT:
		targetVerts.SetAll ();
		break;
	case MESH_VERTEX:
		targetVerts = mesh.vertSel;
		break;
	case MESH_EDGE:
		for (i=0; i<mesh.numFaces; i++) {
			for (j=0; j<3; j++) {
				if (!mesh.edgeSel[i*3+j]) continue;
				targetVerts.Set (mesh.faces[i].v[j]);
				targetVerts.Set (mesh.faces[i].v[(j+1)%3]);
			}
		}
		break;
	case MESH_FACE:
		for (i=0; i<mesh.numFaces; i++) {
			if (!mesh.faceSel[i]) continue;
			for (j=0; j<3; j++) targetVerts.Set (mesh.faces[i].v[j]);
		}
		break;
	}
}

void VWeldMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	
	float threshold;
	mp_pblock->GetValue (kVwThreshold, t, threshold, iv);
	if (threshold<0.0f) threshold=0.0f;

	// Convert existing selection (at whatever level) to vertex selection:
	BitArray targetVerts;
	ConvertTriSelection (mesh, targetVerts);

	// Weld the vertices
	MeshDelta tmd(mesh);
	BOOL found = tmd.WeldByThreshold (mesh, targetVerts, threshold);
	tmd.Apply (mesh);

	tobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	tobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	tobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	tobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	tobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

Interval VWeldMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	Interval iv = FOREVER;
	float v;
	mp_pblock->GetValue(kVwThreshold,t,v,iv);
	return iv;
}

RefTargetHandle VWeldMod::GetReference(int i) {
	switch (i) {
	case kVW_PBLOCK_REF: return mp_pblock;
	default: return NULL;
	}
}

void VWeldMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kVW_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	}
}
