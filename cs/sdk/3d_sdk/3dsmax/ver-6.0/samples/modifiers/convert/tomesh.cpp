/**********************************************************************
 *<
	FILE: ToMesh.cpp

	DESCRIPTION:  Convert to Mesh Modifier

	CREATED BY: Steve Anderson

	HISTORY: Created February 2000

 *>	Copyright (c) 2000 Autodesk, All Rights Reserved.
 **********************************************************************/

#include "Convert.h"
#include "MeshNormalSpec.h"

static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Face(4);

class ConvertToMesh : public Modifier {
public:
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static ConvertToMesh *editMod;

	ConvertToMesh();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_CONVERT_TO_MESH); }  
	virtual Class_ID ClassID() { return CONVERT_TO_MESH_ID; }		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_CONVERT_TO_MESH); }

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

	void Convert (PolyObject *obj, TimeValue t, Mesh & m, Interval & ivalid);
	void Convert (TriObject *obj, TimeValue t, Mesh & m, Interval & ivalid);
	void Convert (PatchObject *obj, TimeValue t, Mesh & m, Interval & ivalid);

	int UI2SelLevel(int selLevel);
	ISubObjType *GetSubObjType(int i);
	bool ChangesSelType();
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *ConvertToMesh::ip              = NULL;
ConvertToMesh *ConvertToMesh::editMod         = NULL;

class ConvertToMeshClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ConvertToMesh; }
	const TCHAR *	ClassName() { return GetString(IDS_CONVERT_TO_MESH); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return CONVERT_TO_MESH_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MOD_CATEGORY); }
	const TCHAR*	InternalName() { return _T("ConvertToMesh"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static ConvertToMeshClassDesc convertToMeshDesc;
ClassDesc* GetConvertToMeshDesc() {return &convertToMeshDesc;}

// Parameter block IDs:
// Blocks themselves:
enum { turn_params };
// Parameters in first block:
enum { turn_use_invis, turn_sel_type, turn_softsel, turn_sel_level };

static ParamBlockDesc2 turn_param_desc ( turn_params, _T("convertToMeshParams"),
									IDS_PARAMETERS, &convertToMeshDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI, REF_PBLOCK,
	//rollout description
	IDD_TO_MESH, IDS_PARAMETERS, 0, 0, NULL,

	// params
	turn_use_invis, _T("useInvisibleEdges"), TYPE_BOOL, P_RESET_DEFAULT|P_ANIMATABLE, IDS_USE_INVIS,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_INVIS,
		end,

	turn_sel_type, _T("selectionConversion"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_TYPE,
		p_default, 0, // Preserve selection
		p_ui, TYPE_RADIO, 3, IDC_SEL_PRESERVE, IDC_SEL_CLEAR, IDC_SEL_INVERT,
		end,

	turn_softsel, _T("useSoftSelection"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_SOFTSEL,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	turn_sel_level, _T("selectionLevel"), TYPE_INT, P_RESET_DEFAULT, IDS_SEL_LEVEL,
		p_default, 0, // Object level.
		p_ui, TYPE_RADIO, 5, IDC_SEL_PIPELINE, IDC_SEL_OBJ, IDC_SEL_VERT, IDC_SEL_EDGE, IDC_SEL_FACE,
		end,
	end
);

//--- Modifier methods -------------------------------

ConvertToMesh::ConvertToMesh() {
	pblock = NULL;
	GetConvertToMeshDesc()->MakeAutoParamBlocks(this);
}

RefTargetHandle ConvertToMesh::Clone(RemapDir& remap) {
	ConvertToMesh *mod = new ConvertToMesh();
	mod->ReplaceReference (0, pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

IParamBlock2 *ConvertToMesh::GetParamBlockByID (short id) {
	return (pblock->ID() == id) ? pblock : NULL; 
}

Interval ConvertToMesh::GetValidity (TimeValue t) {
	Interval ret = FOREVER;
	pblock->GetValidity (t, FOREVER);
	return ret;
}

void ConvertToMesh::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->CanConvertToType (triObjectClassID)) return;

	Interval ivalid=os->obj->ObjectValidity (t);
	TriObject *triObj;
	PolyObject *polyObj;
	bool fromPatch=FALSE;

	TriObject *tobj = CreateNewTriObject ();
	Mesh & m = tobj->GetMesh();

#ifndef NO_PATCHES  // orb 02-11-2002
	PatchObject *patchObj;

	if (os->obj->IsSubClassOf (patchObjectClassID)) {
		patchObj = (PatchObject *) os->obj;
		fromPatch = TRUE;
		Convert (patchObj, t, m, ivalid);
	} else 
#endif // NO_PATCHES
	{
		if (os->obj->IsSubClassOf (triObjectClassID)) {
			triObj = (TriObject *) os->obj;
			Convert (triObj, t, m, ivalid);
			tobj->mDisableDisplacement = triObj->mDisableDisplacement;
			tobj->mSplitMesh = triObj->mSplitMesh;
			tobj->mDispApprox = triObj->mDispApprox;
			tobj->mSubDivideDisplacement = triObj->mSubDivideDisplacement;
		} else {
			if (os->obj->IsSubClassOf (polyObjectClassID)) {
				polyObj = (PolyObject *) os->obj;
				Convert (polyObj, t, m, ivalid);
				tobj->mDisableDisplacement = polyObj->GetDisplacementDisable ();
				tobj->mSplitMesh = polyObj->GetDisplacementSplit();
				tobj->mDispApprox = polyObj->GetDisplacementParameters();
				tobj->mSubDivideDisplacement = polyObj->GetDisplacement ();
			} else {
				// Some other kind of object: default conversion.
				triObj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
				Convert (triObj, t, m, ivalid);
				tobj->mDisableDisplacement = triObj->mDisableDisplacement;
				tobj->mSplitMesh = triObj->mSplitMesh;
				tobj->mDispApprox = triObj->mDispApprox;
				tobj->mSubDivideDisplacement = triObj->mSubDivideDisplacement;
				if (triObj != os->obj) delete triObj;
			}
		}
	}

	// Indicate that we support the user-specified normals in this conversion.
	MeshNormalSpec *pNorm = m.GetSpecifiedNormals ();
	if (pNorm != NULL) pNorm->SetFlag (MESH_NORMAL_MODIFIER_SUPPORT);

	// Make all edges visible if desired.
	int use_invis;
	pblock->GetValue (turn_use_invis, t, use_invis, ivalid);
	if (!use_invis) {
		for (int i=0; i<m.numFaces; i++) {
			m.faces[i].flags |= EDGE_ALL;
		}
	}

	// Handle Selection Conversion
	int selConv, selLevel, use_ss;

	pblock->GetValue (turn_sel_type, t, selConv, ivalid);
	pblock->GetValue (turn_sel_level, t, selLevel, ivalid);
	pblock->GetValue (turn_softsel, t, use_ss, ivalid);
	if (selLevel && (UI2SelLevel (selLevel) != m.selLevel)) use_ss = false;

	switch (selConv) {
	case 0: // Preserve selection
		if (!use_ss) m.ClearVSelectionWeights ();
		break;

	case 1:	// Clear selection
		m.vertSel.ClearAll ();
		m.edgeSel.ClearAll ();
		m.faceSel.ClearAll ();
		m.ClearVSelectionWeights ();
		break;

	case 2: // Invert selection
		m.vertSel = ~m.vertSel;
		m.edgeSel = ~m.edgeSel;
		m.faceSel = ~m.faceSel;
		if (use_ss) {
			float *vsw = m.getVSelectionWeights ();
			if (vsw) {
				for (int i=0; i<m.numVerts; i++) {
					vsw[i] = 1.0f - vsw[i];
					if (vsw[i] < 1.0f) m.vertSel.Clear (i);
				}
			}
		} else {
			m.ClearVSelectionWeights ();
		}
		break;
	}

	// Set Subobject Level
	if(selLevel != 0) m.selLevel = UI2SelLevel(selLevel);

	// Set display flags based on sel level.
	m.dispFlags = 0;
	switch (m.selLevel) {
	case MESH_VERTEX:
		m.SetDispFlag (DISP_VERTTICKS|DISP_SELVERTS);
		break;
	case MESH_EDGE:
		m.SetDispFlag (DISP_SELEDGES);
		break;
	case MESH_FACE:
		m.SetDispFlag (DISP_SELPOLYS);
		break;
	}

	tobj->UpdateValidity (GEOM_CHAN_NUM, ivalid);
	tobj->UpdateValidity (TOPO_CHAN_NUM, ivalid);
	tobj->UpdateValidity (TEXMAP_CHAN_NUM, ivalid);
	tobj->UpdateValidity (VERT_COLOR_CHAN_NUM, ivalid);
	tobj->UpdateValidity (SELECT_CHAN_NUM, ivalid);
	tobj->UpdateValidity (SUBSEL_TYPE_CHAN_NUM, ivalid);
	tobj->UpdateValidity (DISP_ATTRIB_CHAN_NUM, ivalid);
	tobj->UpdateValidity (GFX_DATA_CHAN_NUM, ivalid);
	os->obj = (Object *) tobj;
}

void ConvertToMesh::Convert (PolyObject *obj, TimeValue t, 
							  Mesh & m, Interval & ivalid) {
	obj->mm.OutToTri(m);

	switch (obj->mm.selLevel) {
	case MNM_SL_VERTEX: m.selLevel = MESH_VERTEX; break;
	case MNM_SL_EDGE: m.selLevel = MESH_EDGE; break;
	case MNM_SL_FACE: m.selLevel = MESH_FACE; break;
	default: m.selLevel = MESH_OBJECT;
	}
}

void ConvertToMesh::Convert (TriObject *obj, TimeValue t,
							  Mesh & m, Interval & ivalid) {
	m = obj->mesh;
	m.selLevel = obj->mesh.selLevel;
}

void ConvertToMesh::Convert (PatchObject *obj, TimeValue t,
							  Mesh & m, Interval & ivalid) {
	int selConv, useSoftSel;
	pblock->GetValue (turn_sel_type, t, selConv, ivalid);
	pblock->GetValue (turn_softsel, t, useSoftSel, ivalid);

	DWORD flags=0;
	if (selConv != 1) {
		flags = CONVERT_KEEPSEL;
		if (useSoftSel) flags |= CONVERT_USESOFTSEL;
	}

	ConvertPatchToMesh (obj->patch, m, flags);

	switch (obj->patch.selLevel) {
	case PATCH_VERTEX: m.selLevel = MESH_VERTEX; break;
	case PATCH_EDGE: m.selLevel = MESH_EDGE; break; 
	case PATCH_PATCH: m.selLevel = MESH_FACE; break;
	default: m.selLevel = MESH_OBJECT; break;
	}
}

void ConvertToMesh::BeginEditParams (IObjParam  *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;	
	editMod  = this;

	// throw up all the appropriate auto-rollouts
	convertToMeshDesc.BeginEditParams(ip, this, flags, prev);

	// Necessary?
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void ConvertToMesh::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	convertToMeshDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	editMod  = NULL;
}

RefResult ConvertToMesh::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
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

int ConvertToMesh::UI2SelLevel(int selLevel)
{
	switch (selLevel) {
	case 1: return MESH_OBJECT; break;
	case 2: return MESH_VERTEX; break;
	case 3: return MESH_EDGE; break;
	case 4: return MESH_FACE; break;
	default: return 4; break;
	}
}

ISubObjType *ConvertToMesh::GetSubObjType(int i) {
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
				switch (selLevel) {
				case MESH_VERTEX: return &SOT_Vertex;
				case MESH_EDGE: return &SOT_Edge;
				case MESH_FACE: return &SOT_Face;
				default: return NULL;
				}
			}
		}
		break;
	case 0: return &SOT_Vertex;
	case 1: return &SOT_Edge;
	case 2: return &SOT_Face;
	}
	return NULL;
}

bool ConvertToMesh::ChangesSelType()
{
	int selLevel;
	Interval ivalid = FOREVER;
	
	pblock->GetValue (turn_sel_level, GetCOREInterface()->GetTime(), selLevel, ivalid);
	if(selLevel == 0) return false;
	else return true;
}
