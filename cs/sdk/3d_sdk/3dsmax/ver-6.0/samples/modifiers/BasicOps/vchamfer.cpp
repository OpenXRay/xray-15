/**********************************************************************
 *<
	FILE: vchamfer.cpp

	DESCRIPTION: Vertex Chamfer Modifier (for Meshes, PolyMeshes.)

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig.

	HISTORY: created 8/31/2001

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"

const Class_ID kVERTEX_CHAMFER_CLASS_ID(0x18266757,0x6ee80dfb);
const unsigned short kVCH_PBLOCK_REF(0);

class VChamferMod : public Modifier {	
	IParamBlock2 *mp_pblock;

public:
	VChamferMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_VERTEX_CHAMFER_MOD);}  
	virtual Class_ID ClassID () { return kVERTEX_CHAMFER_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_VERTEX_CHAMFER_MOD); }

	void BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier
	// Since we're changing topology, all these other channels are affected as well:
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR; }
	ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void ModifyTriObject(TimeValue t, ModContext &mc, TriObject *tobj);
	void ModifyPolyObject(TimeValue t, ModContext &mc, PolyObject *pobj);
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
};

//--- ClassDescriptor and class vars ---------------------------------

class VertexChamferClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new VChamferMod; }
	const TCHAR *	ClassName() { return GetString(IDS_VERTEX_CHAMFER_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kVERTEX_CHAMFER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("VertexChamfer"); }
	HINSTANCE HInstance() { return hInstance; }
};

static VertexChamferClassDesc vchamferDesc;
extern ClassDesc* GetVertexChamferModDesc() {return &vchamferDesc;}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kVChamferParams };

// And enumerate the parameters within that block:
enum { kVChAmount };

// Parameters
static ParamBlockDesc2 vertex_chamfer_param_blk (kVChamferParams, _T("Vertex Chamfer Parameters"), 0, &vchamferDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kVCH_PBLOCK_REF,
	// rollout
	IDD_VCH_PARAM, IDS_PARAMETERS, 0, 0, NULL,

	// Parameters
	kVChAmount, _T("amount"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_VCH_AMOUNT,
		p_default, 0.0f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_VCH_AMOUNT, IDC_VCH_AMOUNTSPIN, SPIN_AUTOSCALE,
		end,

	end
);

//--- VChamferMod methods -------------------------------


VChamferMod::VChamferMod() {
	mp_pblock = NULL;
	vchamferDesc.MakeAutoParamBlocks(this);
}

void VChamferMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	vchamferDesc.BeginEditParams(ip,this,flags,prev);
}

void VChamferMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	vchamferDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle VChamferMod::Clone(RemapDir& remap) {
	VChamferMod *mod = new VChamferMod();
	mod->ReplaceReference(kVCH_PBLOCK_REF,mp_pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

void VChamferMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (os->obj->IsSubClassOf(triObjectClassID))
		ModifyTriObject (t, mc, (TriObject *)os->obj);
	else {
		if (os->obj->IsSubClassOf (polyObjectClassID))
			ModifyPolyObject (t, mc, (PolyObject *)os->obj);
		else {
			if (os->obj->CanConvertToType (triObjectClassID)) {
				TriObject *tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
				ModifyTriObject (t, mc, tobj);
				os->obj = (Object *) tobj;
			}
		}
	}
}

void VChamferMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj) {
	MNMesh &mesh = pobj->GetMesh();

	// Luna task 747
	// We cannot support specified normals in Vertex Chamfer at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	float amount;
	int i, mp;

	mp_pblock->GetValue (kVChAmount, t, amount, iv);

	// Convert existing selection (at whatever level) to vertex selection:
	DWORD targetFlag = MN_SEL;
	if (mesh.selLevel != MNM_SL_VERTEX) {
		targetFlag = MN_WHATEVER;
		mesh.ClearVFlags (MN_WHATEVER);
		if (mesh.selLevel == MNM_SL_OBJECT) {
			for (i=0; i<mesh.numv; i++) if (!mesh.v[i].GetFlag (MN_DEAD)) mesh.v[i].SetFlag (MN_WHATEVER);
		} else {
			mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_WHATEVER, mesh.selLevel, MN_SEL);
		}
	}

	MNTempData temp(&mesh);
	MNChamferData *mcd = temp.ChamferData();
	bool ret = FALSE;
	ret = mesh.ChamferVertices (targetFlag, mcd);

	Tab<UVVert> mapDelta;
	for (mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) {
		if (mesh.M(mp)->GetFlag (MN_DEAD)) continue;
		mcd->GetMapDelta (mesh, mp, amount, mapDelta);
		UVVert *mv = mesh.M(mp)->v;
		for (i=0; i<mesh.M(mp)->numv; i++) mv[i] += mapDelta[i];
	}

	Tab<Point3> delta;
	mcd->GetDelta (amount, delta);
	for (i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		mesh.v[i].p += delta[i];
	}

	mesh.InvalidateTopoCache ();
	mesh.FillInMesh ();
	pobj->UpdateValidity(GEOM_CHAN_NUM,iv);
}

void VChamferMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	float amount;
	int i, j;
	
	mp_pblock->GetValue (kVChAmount, t, amount, iv);

	// Convert existing selection (at whatever level) to vertex selection:
	BitArray targetVerts;
	targetVerts.SetSize (mesh.numVerts);
	targetVerts.ClearAll ();

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

	// Chamfer the vertices -- this just does the topological operation.
	MeshDelta tmd;
	tmd.InitToMesh (mesh);
	MeshTempData temp;
	temp.SetMesh (&mesh);
	MeshChamferData *mcd = temp.ChamferData();
	AdjEdgeList *ae = temp.AdjEList();
	tmd.ChamferVertices (mesh, targetVerts, *mcd, ae);
	tmd.Apply (mesh);

	// Reset the meshdelta, temp data to deal with the post-chamfered topology:
	tmd.InitToMesh (mesh);
	temp.Invalidate (PART_TOPO);	// Generates a new edge list, but preserves chamfer data
	temp.SetMesh (&mesh);
	tmd.ChamferMove (mesh, *temp.ChamferData(), amount, temp.AdjEList());
	tmd.Apply (mesh);

	tobj->UpdateValidity(GEOM_CHAN_NUM,iv);		
}

Interval VChamferMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	Interval iv = FOREVER;
	float v;
	mp_pblock->GetValue(kVChAmount,t,v,iv);
	return iv;
}

RefTargetHandle VChamferMod::GetReference(int i) {
	switch (i) {
	case kVCH_PBLOCK_REF: return mp_pblock;
	default: return NULL;
	}
}

void VChamferMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kVCH_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	}
}

