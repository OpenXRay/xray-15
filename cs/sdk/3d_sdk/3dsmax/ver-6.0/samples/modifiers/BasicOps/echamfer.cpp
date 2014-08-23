/**********************************************************************
 *<
	FILE: echamfer.cpp

	DESCRIPTION: Edge Chamfer Modifier (for Meshes, PolyMeshes.)

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig.

	HISTORY: created 8/31/2001

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"

const Class_ID kEDGE_CHAMFER_CLASS_ID(0x4b543af5, 0x57076d64);
const unsigned short kECH_PBLOCK_REF(0);

class EChamferMod : public Modifier {	
	IParamBlock2 *m_pblock;

public:
	EChamferMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_EDGE_CHAMFER_MOD);}  
	virtual Class_ID ClassID () { return kEDGE_CHAMFER_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_EDGE_CHAMFER_MOD); }

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
	IParamBlock2* GetParamBlock(int i) { return m_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (m_pblock->ID() == id) ? m_pblock : NULL; }

	// Reference Management:
	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// Animatable management:
	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return m_pblock;}
	TSTR SubAnimName(int i) { return _T(""); }
};

//--- ClassDescriptor and class vars ---------------------------------

class EdgeChamferClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new EChamferMod; }
	const TCHAR *	ClassName() { return GetString(IDS_EDGE_CHAMFER_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kEDGE_CHAMFER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("EdgeChamfer"); }
	HINSTANCE HInstance() { return hInstance; }
};

static EdgeChamferClassDesc echamferDesc;
extern ClassDesc* GetEdgeChamferModDesc() {return &echamferDesc;}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kEChamferParams };

// And enumerate the parameters within that block:
enum { kEchAmount };

// Parameters
static ParamBlockDesc2 edge_chamfer_param_blk (kEChamferParams, _T("Edge Chamfer Parameters"), 0, &echamferDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kECH_PBLOCK_REF,
	// rollout
	IDD_ECH_PARAM, IDS_PARAMETERS, 0, 0, NULL,

	// Parameters
	kEchAmount, _T("amount"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_ECH_AMOUNT,
		p_default, 0.0f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_ECH_AMOUNT, IDC_ECH_AMOUNTSPIN, SPIN_AUTOSCALE,
		end,

	end
);

//--- EChamferMod methods -------------------------------


EChamferMod::EChamferMod() {
	m_pblock = NULL;
	echamferDesc.MakeAutoParamBlocks(this);
}

void EChamferMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	echamferDesc.BeginEditParams(ip,this,flags,prev);
}

void EChamferMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	echamferDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle EChamferMod::Clone(RemapDir& remap) {
	EChamferMod *mod = new EChamferMod();
	mod->ReplaceReference(kECH_PBLOCK_REF,m_pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

void EChamferMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
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

void EChamferMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj) {
	MNMesh &mesh = pobj->GetMesh();

	// Luna task 747
	// We cannot support specified normals in Edge Chamfer at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	float amount;
	int i, mp;

	m_pblock->GetValue (kEchAmount, t, amount, iv);

	// Convert existing selection (at whatever level) to edge selection:
	DWORD targetFlag = MN_SEL;
	if (mesh.selLevel != MNM_SL_EDGE) {
		targetFlag = MN_WHATEVER;
		mesh.ClearEFlags (MN_WHATEVER);
		if (mesh.selLevel == MNM_SL_OBJECT) {
			for (i=0; i<mesh.nume; i++) if (!mesh.e[i].GetFlag (MN_DEAD)) mesh.e[i].SetFlag (MN_WHATEVER);
		} else {
			mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_WHATEVER, mesh.selLevel, MN_SEL);
		}
	}

	MNTempData temp(&mesh);
	MNChamferData *mcd = temp.ChamferData();
	bool ret = FALSE;
	ret = mesh.ChamferEdges (targetFlag, mcd);

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

void EChamferMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	float amount;
	int i, j;
	
	m_pblock->GetValue (kEchAmount, t, amount, iv);

	// Convert existing selection (at whatever level) to edge selection:
	BitArray targetEdges;
	targetEdges.SetSize (mesh.numFaces*3);
	targetEdges.ClearAll ();

	switch (mesh.selLevel) {
	case MESH_OBJECT:
		targetEdges.SetAll ();
		break;
	case MESH_VERTEX:
		for (i=0; i<mesh.numFaces; i++) {
			for (j=0; j<3; j++) {
				if (!mesh.vertSel[mesh.faces[i].v[j]]) continue;
				// Don't select invisible edges:
				if (mesh.faces[i].getEdgeVis(j)) targetEdges.Set (i*3+j);
				if (mesh.faces[i].getEdgeVis((j+2)%3)) targetEdges.Set (i*3+(j+2)%3);
			}
		}
		break;
	case MESH_EDGE:
		targetEdges = mesh.edgeSel;
		break;
	case MESH_FACE:
		for (i=0; i<mesh.numFaces; i++) {
			if (!mesh.faceSel[i]) continue;
			for (j=0; j<3; j++) {
				// Don't select invisible edges:
				if (mesh.faces[i].getEdgeVis(j)) targetEdges.Set (i*3+j);
			}
		}
		break;
	}

	// Chamfer the edges -- this just does the topological operation.
	MeshDelta tmd;
	tmd.InitToMesh (mesh);
	MeshTempData temp;
	temp.SetMesh (&mesh);
	MeshChamferData *mcd = temp.ChamferData();
	AdjEdgeList *ae = temp.AdjEList();
	tmd.ChamferEdges (mesh, targetEdges, *mcd, ae);
	tmd.Apply (mesh);

	// Reset the meshdelta, temp data to deal with the post-chamfered topology:
	tmd.InitToMesh (mesh);
	temp.Invalidate (PART_TOPO);	// Generates a new edge list, but preserves chamfer data
	temp.SetMesh (&mesh);
	tmd.ChamferMove (mesh, *temp.ChamferData(), amount, temp.AdjEList());
	tmd.Apply (mesh);

	tobj->UpdateValidity(GEOM_CHAN_NUM,iv);		
}

Interval EChamferMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	Interval iv = FOREVER;
	float v;
	m_pblock->GetValue(kEchAmount,t,v,iv);
	return iv;
}

RefTargetHandle EChamferMod::GetReference(int i) {
	switch (i) {
	case kECH_PBLOCK_REF: return m_pblock;
	default: return NULL;
	}
}

void EChamferMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kECH_PBLOCK_REF: m_pblock = (IParamBlock2*)rtarg; break;
	}
}

