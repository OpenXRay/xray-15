/**********************************************************************
 *<
	FILE: symmetry.cpp

	DESCRIPTION: Symmetry Modifier (for Meshes, PolyMeshes.)

	CREATED BY: Steve Anderson, based on Vertex Weld, Mirror, and Slice modifiers.

	HISTORY: created 9/20/2001

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"

const Class_ID kSYMMETRY_CLASS_ID(0x2d7e702b, 0xbe41a6e);
const unsigned int kSYM_PBLOCK_REF = 0;
const unsigned int kSYM_MIRROR_REF = 1;

class SymmetryMod : public Modifier {
	IParamBlock2 *mp_pblock;
	Control *mp_mirror;
	static IObjParam *mp_ip;
	static MoveModBoxCMode    *mp_moveMode;
	static RotateModBoxCMode  *mp_rotMode;
	static UScaleModBoxCMode *mp_scaleMode;
	static NUScaleModBoxCMode *mp_nuScaleMode;
	static SquashModBoxCMode	*mp_squashMode;

public:
	SymmetryMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_SYMMETRY_MOD);}
	virtual Class_ID ClassID () { return kSYMMETRY_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() { return GetString(IDS_SYMMETRY_MOD); }
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum);

	void BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier
	// Since we're changing topology, all these other channels are affected as well:
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR; }
	ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	Matrix3 CompMatrix (TimeValue t, INode *inode, ModContext *mc, Interval *validity=NULL);
	void DrawGizmo (float size,PolyLineProc& lp);

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);		
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
	void Scale (TimeValue t, Matrix3 & partm, Matrix3 & tmAxis, Point3 & val, BOOL localOrigin);
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);

	// Subobject API
	int NumSubObjTypes () { return 1; }
	ISubObjType *GetSubObjType (int i);

	// ParamBlock2 access:
	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	// Reference Management:
	int NumRefs() {return 2;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message);

	// Animatable management:
	int NumSubs() {return 2;}
	Animatable* SubAnim(int i) {return (Animatable*)GetReference(i); }
	TSTR SubAnimName(int i);

	// Local methods:
	void SliceTriObject (Mesh & mesh, Point3 & N, float offset);
	void MirrorTriObject (Mesh & mesh, int axis, Matrix3 & tm, Matrix3 & itm);
	void WeldTriObject (Mesh & mesh, Point3 & N, float offset, float threshold);
	void ModifyTriObject(TimeValue t, ModContext &mc, TriObject *tobj, INode *inode);

	void SlicePolyObject (MNMesh & mesh, Point3 & N, float offset);
	void MirrorPolyObject (MNMesh & mesh, int axis, Matrix3 & tm, Matrix3 & itm);
	void WeldPolyObject (MNMesh & mesh, Point3 & N, float offset, float threshold);
	void RemovePolySpurs (MNMesh & mesh);
	void ModifyPolyObject(TimeValue t, ModContext &mc, PolyObject *pobj, INode *inode);
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam       *SymmetryMod::mp_ip        = NULL;
MoveModBoxCMode    *SymmetryMod::mp_moveMode    = NULL;
RotateModBoxCMode  *SymmetryMod::mp_rotMode 	   = NULL;
UScaleModBoxCMode *SymmetryMod::mp_scaleMode = NULL;
NUScaleModBoxCMode *SymmetryMod::mp_nuScaleMode = NULL;
SquashModBoxCMode *SymmetryMod::mp_squashMode = NULL;

class SymmetryClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SymmetryMod; }
	const TCHAR *	ClassName() { return GetString(IDS_SYMMETRY_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kSYMMETRY_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("Symmetry"); }
	HINSTANCE HInstance() { return hInstance; }
};

static SymmetryClassDesc symDesc;
extern ClassDesc* GetSymmetryModDesc() {return &symDesc;}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kSymmetryParams };

// And enumerate the parameters within that block:
enum { kSymAxis, kSymSlice, kSymWeld, kSymThreshold, kSymFlip };

// Parameters
static ParamBlockDesc2 symmetry_param_blk (kSymmetryParams, _T("Symmetry Parameters"), 0, &symDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kSYM_PBLOCK_REF,
	// rollout
	IDD_SYM_PARAM, IDS_PARAMETERS, 0, 0, NULL,

	// Parameters
	kSymAxis, _T("axis"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_AXIS,
		p_default, 0,
		p_ui, TYPE_RADIO, 3, IDC_SYM_X, IDC_SYM_Y, IDC_SYM_Z,
		end,

	kSymFlip, _T("flip"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SYM_FLIP,
		p_default, 0,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SYM_FLIP,
		end,

	kSymSlice, _T("slice"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_SLICE,
		p_default, 1,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SYM_SLICE,
		end,

	kSymWeld, _T("weld"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_WELD,
		p_default, 1,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SYM_WELD,
		p_enable_ctrls, 1, kSymThreshold,
		end,

	kSymThreshold, _T("threshold"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_SYM_THRESHOLD,
		p_default, 0.1f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_SYM_THRESH, IDC_SYM_THRESHSPIN, SPIN_AUTOSCALE,
		end,

	end
);

//--- SymmetryMod methods -------------------------------


SymmetryMod::SymmetryMod() {
	mp_pblock = NULL;
	mp_mirror = NULL;
	MakeRefByID (FOREVER, kSYM_MIRROR_REF, NewDefaultMatrix3Controller()); 
	symDesc.MakeAutoParamBlocks(this);
}

void SymmetryMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mp_ip = ip;

	// Create sub object editing modes.
	mp_moveMode    = new MoveModBoxCMode(this,ip);
	mp_rotMode     = new RotateModBoxCMode(this,ip);
	mp_scaleMode = new UScaleModBoxCMode (this, ip);
	mp_nuScaleMode = new NUScaleModBoxCMode (this, ip);
	mp_squashMode = new SquashModBoxCMode (this, ip);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	symDesc.BeginEditParams(ip,this,flags,prev);
}

void SymmetryMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	mp_ip = NULL;

	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// Eliminate our command modes.
	ip->DeleteMode(mp_moveMode);
	ip->DeleteMode(mp_rotMode);
	ip->DeleteMode (mp_scaleMode);
	ip->DeleteMode (mp_nuScaleMode);
	ip->DeleteMode (mp_squashMode);
	if ( mp_moveMode ) delete mp_moveMode;
	mp_moveMode = NULL;
	if ( mp_rotMode ) delete mp_rotMode;
	mp_rotMode = NULL;
	if ( mp_scaleMode ) delete mp_scaleMode;
	mp_scaleMode = NULL;
	if ( mp_nuScaleMode ) delete mp_nuScaleMode;
	mp_nuScaleMode = NULL;
	if ( mp_squashMode ) delete mp_squashMode;
	mp_squashMode = NULL;

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	symDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle SymmetryMod::Clone(RemapDir& remap) {
	SymmetryMod *mod = new SymmetryMod();
	mod->ReplaceReference(kSYM_PBLOCK_REF,mp_pblock->Clone(remap));
	mod->ReplaceReference(kSYM_MIRROR_REF,remap.CloneRef(mp_mirror));
	BaseClone(this, mod, remap);
	return mod;
}

void SymmetryMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *inode) {
	if (os->obj->IsSubClassOf(triObjectClassID))
		ModifyTriObject (t, mc, (TriObject *)os->obj, inode);
	else {
		if (os->obj->IsSubClassOf (polyObjectClassID))
			ModifyPolyObject (t, mc, (PolyObject *)os->obj, inode);
		else {
			if (os->obj->CanConvertToType (triObjectClassID)) {
				TriObject *tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
				ModifyTriObject (t, mc, tobj, inode);
				os->obj = (Object *) tobj;
			}
		}
	}
}

void SymmetryMod::SlicePolyObject (MNMesh & mesh, Point3 & N, float offset) {
	// Steve Anderson 9/14/2002
	// Using the new "MN_MESH_TEMP_1" flag to override Slice selection behavior,
	// which is undesirable here.
	mesh.SetFlag (MN_MESH_TEMP_1);
	// Slice off everything below the plane:
	mesh.Slice (N, offset, MNEPS, false, true);
	mesh.ClearFlag (MN_MESH_TEMP_1);

	// Make sure we have a valid edge list:
	if (!mesh.GetFlag (MN_MESH_FILLED_IN)) mesh.FillInMesh();

	// Mark the vertices on the plane boundary:
	mesh.ClearVFlags (MN_USER);
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		float dist = DotProd (N, mesh.P(i)) - offset;
		if (fabsf(dist) > MNEPS) continue;
		mesh.v[i].SetFlag (MN_USER);
	}

	// Strip out faces on the mirror plane:  (These aren't always removed by slice.)
	// Find the faces that use only mirror-plane vertices:
	mesh.ClearFFlags (MN_USER);
	mesh.PropegateComponentFlags (MNM_SL_FACE, MN_USER,
		MNM_SL_VERTEX, MN_USER, true);
	mesh.DeleteFlaggedFaces (MN_USER);

	// Clear out dead components:
	mesh.CollapseDeadStructs ();
}

void SymmetryMod::MirrorPolyObject (MNMesh & mesh, int axis, Matrix3 & tm, Matrix3 & itm) {
	// Create scaling matrix for mirroring on selected axis:
	Point3 scale(1,1,1);
	scale[axis] = -1.0f;
	itm.Scale(scale,TRUE);

	// Make the mirror copy of the entire mesh:
	int oldnumv = mesh.numv;
	for (int i=0; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_USER);
	mesh.CloneFaces (MN_USER, true);

	// Transform the vertices to their mirror images:
	for (i=oldnumv; i<mesh.numv; i++) mesh.v[i].p = (itm*mesh.v[i].p)*tm;

	// Flip over faces, edges:
	mesh.FlipElementNormals (MN_USER);	// flag should now be set only on clones.

	DbgAssert (mesh.CheckAllData ());
}

void SymmetryMod::WeldPolyObject (MNMesh & mesh, Point3 & N, float offset, float threshold) {
	// Mark the vertices within the welding threshold of the plane:
	mesh.ClearVFlags (MN_USER);
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		float dist = DotProd (N, mesh.P(i)) - offset;
		if (fabsf(dist) > threshold) continue;
		mesh.v[i].SetFlag (MN_USER);
	}

	// Do the welding:
	if (mesh.WeldBorderVerts (threshold, MN_USER)) {
		// If result was true, we have some MN_DEAD components:
		mesh.CollapseDeadStructs ();
	}
}

void SymmetryMod::RemovePolySpurs (MNMesh & mesh) {
	// Make sure we don't have any "spurs".
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		if (mesh.vedg[i].Count() != 1) continue;
		int vid = i;
		while ((!mesh.v[vid].GetFlag (MN_DEAD)) && (mesh.vedg[vid].Count() == 1)) {
			int edge = mesh.vedg[vid][0];
			int otherEnd = mesh.e[edge].OtherVert (vid);
			mesh.RemoveSpur (edge);
			// The other end might be a tip of a spur now:
			vid = otherEnd;
			if (vid == i) {	// shouldn't happen - extra check to prevent loops.
				DbgAssert (0);
				break;
			}
		}
	}
}

void SymmetryMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj, INode *inode) {
	MNMesh &mesh = pobj->GetMesh();

	// Luna task 747
	// We cannot support specified normals in Symmetry at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	int axis, slice, weld, flip;
	float thresh;

	mp_pblock->GetValue (kSymAxis, t, axis, iv);
	mp_pblock->GetValue (kSymFlip, t, flip, iv);
	mp_pblock->GetValue (kSymSlice, t, slice, iv);
	mp_pblock->GetValue (kSymWeld, t, weld, iv);
	mp_pblock->GetValue (kSymThreshold, t, thresh, iv);
	if (thresh<0) thresh=0;

	// Get transform from mp_mirror controller:
	Matrix3 tm  = CompMatrix (t, NULL, &mc, &iv);
	Matrix3 itm = Inverse (tm);

	// Get DotProd(N,x)=off plane definition from transform
	Point3 Axis(0,0,0);
	Axis[axis] = flip ? -1.0f : 1.0f;
	Point3 or = tm.GetTrans();
	Point3 N = Normalize(tm*Axis - or);
	float off = DotProd (N, or);

	if (slice) SlicePolyObject (mesh, N, off);
	MirrorPolyObject (mesh, axis, tm, itm);
	if (weld) {
		WeldPolyObject (mesh, N, off, thresh);
		RemovePolySpurs (mesh);
	}

	pobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	pobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	pobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

void SymmetryMod::SliceTriObject (Mesh & mesh, Point3 & N, float offset) {
	// Steve Anderson 9/14/2002
	// Using the new "MESH_TEMP_1" flag to override Slice selection behavior,
	// which is undesirable here.
	mesh.SetFlag (MESH_TEMP_1);
	MeshDelta slicemd;
	slicemd.Slice (mesh, N, offset, false, true);
	slicemd.Apply (mesh);
	mesh.ClearFlag (MESH_TEMP_1);

	// We need to strip out faces on the mirror plane itself.
	// (These aren't always removed by slice.)

	// Mark vertices at the plane boundary:
	BitArray targetVerts;
	targetVerts.SetSize (mesh.numVerts);
	targetVerts.ClearAll ();
	for (int i=0; i<mesh.numVerts; i++) {
		float dist = DotProd (N, mesh.verts[i]) - offset;
		if (fabsf(dist) > MNEPS) continue;
		targetVerts.Set (i);
	}
	BitArray delFaces, delVerts;
	delFaces.SetSize (mesh.numFaces);
	for (i=0; i<mesh.numFaces; i++) {
		for (int j=0; j<3; j++) {
			if (!targetVerts[mesh.faces[i].v[j]]) break;
		}
		if (j<3) continue;
		// Face needs to be deleted.
		delFaces.Set (i);
	}
	mesh.DeleteFaceSet (delFaces, &delVerts);
	mesh.DeleteVertSet (delVerts);
}

void SymmetryMod::MirrorTriObject (Mesh & mesh, int axis, Matrix3 & tm, Matrix3 & itm) {
	// Create scaling matrix for mirroring on selected axis:
	Point3 scale(1,1,1);
	scale[axis] = -1.0f;
	itm.Scale(scale,TRUE);

	// Hang on to a copy of the incoming face selection:
	BitArray inputFaceSel = mesh.faceSel;

	// Make the mirror copy of the entire mesh:
	int oldnumv = mesh.numVerts;
	int oldnumf = mesh.numFaces;
	BitArray fset;
	fset.SetSize (oldnumf);
	fset.SetAll ();
	mesh.CloneFaces (fset);	// Clears selection on originals, sets it on new faces.

	// Transform the cloned vertices to their mirror images:
	for (int i=oldnumv; i<mesh.numVerts; i++) {
		mesh.verts[i] = (mesh.verts[i]*itm)*tm;
	}

	// Restore selection of input faces:
	for (i=0; i<oldnumf; i++) mesh.faceSel.Set (i, inputFaceSel[i]);
	// Flip over new faces and select to match input:
	for (i=oldnumf; i<mesh.numFaces; i++) {
		mesh.FlipNormal (i);
		mesh.faceSel.Set (i, inputFaceSel[i-oldnumf]);
	}
}

void SymmetryMod::WeldTriObject (Mesh & mesh, Point3 & N, float offset, float threshold) {
	// Find vertices in target zone of mirror plane:
	BitArray targetVerts;
	targetVerts.SetSize (mesh.numVerts, true);
	targetVerts.ClearAll ();
	for (int i=0; i<mesh.numVerts; i++) {
		float dist = DotProd (N, mesh.verts[i]) - offset;
		if (fabsf(dist) > threshold) continue;
		targetVerts.Set (i);
	}

	// Weld the suitable border vertices:
	MeshDelta tmd(mesh);
	BOOL found = tmd.WeldByThreshold (mesh, targetVerts, threshold);
	tmd.Apply (mesh);
}

void SymmetryMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj, INode *inode) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	int axis, slice, weld, flip;
	float threshold;

	mp_pblock->GetValue (kSymAxis, t, axis, iv);
	mp_pblock->GetValue (kSymFlip, t, flip, iv);
	mp_pblock->GetValue (kSymSlice, t, slice, iv);
	mp_pblock->GetValue (kSymWeld, t, weld, iv);
	mp_pblock->GetValue (kSymThreshold, t, threshold, iv);
	if (threshold<0) threshold=0;

	// Get transform from mirror controller:
	Matrix3 tm  = CompMatrix (t, NULL, &mc, &iv);
	Matrix3 itm = Inverse (tm);

	// Get DotProd(N,x)=offset plane definition from transform
	Point3 Axis(0,0,0);
	Axis[axis] = flip ? -1.0f : 1.0f;
	Point3 origin = tm.GetTrans();
	Point3 N = Normalize(tm*Axis - origin);
	float offset = DotProd (N, origin);

	// Slice off everything below the plane.
	if (slice) SliceTriObject (mesh, N, offset);
	MirrorTriObject (mesh, axis, tm, itm);
	if (weld) WeldTriObject (mesh, N, offset, threshold);

	tobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	tobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	tobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	tobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	tobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

Interval SymmetryMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval iv = FOREVER;
	int foo;
	mp_pblock->GetValue(kSymAxis, t, foo, iv);
	mp_pblock->GetValue (kSymFlip, t, foo, iv);
	mp_pblock->GetValue(kSymWeld, t, foo, iv);
	mp_pblock->GetValue(kSymSlice, t, foo, iv);
	float thresh;
	mp_pblock->GetValue (kSymThreshold, t, thresh, iv);
	Matrix3 mat(1);		
	mp_mirror->GetValue(t,&mat,iv,CTRL_RELATIVE);
	return iv;
}

Matrix3 SymmetryMod::CompMatrix (TimeValue t, INode *inode, ModContext *mc, Interval *validity) {
	Interval localInterval(FOREVER);
	if (!validity) validity = &localInterval;
	Matrix3 tm(1);
	mp_mirror->GetValue(t,&tm,*validity,CTRL_RELATIVE);
	if (mc && mc->tm) tm = tm * Inverse(*(mc->tm));
	if (inode) tm = tm * inode->GetObjTMBeforeWSM(t,validity);
	return tm;
}

#define AXIS_SIZE		2.0f
#define SCREEN_SCALE	0.1f

static void SetupAxisPoints(Point3 &v, Point3 &vp, float size,Point3 *pts) {
	pts[0] = (vp)*size;
	pts[2] = (-vp)*size;
	pts[1] = (v^vp)*size;
	pts[3] = (v^(-vp))*size;
	pts[4] = pts[0];

	pts[5] = -v*size*AXIS_SIZE;
	pts[6] = v*size*AXIS_SIZE;
	
	Point3 v2 = v*0.9f*size*AXIS_SIZE;

	pts[7] = v2+pts[0]*0.1f;
	pts[8] = pts[6];
	pts[9] = v2+pts[2]*0.1f;

	pts[10] = v2+pts[1]*0.1f;
	pts[11] = pts[6];
	pts[12] = v2+pts[3]*0.1f;
	
	pts[13] = -v2+pts[0]*0.1f;
	pts[14] = -pts[6];
	pts[15] = -v2+pts[2]*0.1f;

	pts[16] = -v2+pts[1]*0.1f;
	pts[17] = -pts[6];
	pts[18] = -v2+pts[3]*0.1f;
}

void SymmetryMod::DrawGizmo(float size,PolyLineProc& lp) {
	Point3 v0(0,0,0), pv0(0,0,0), v1(0,0,0), pv1(0,0,0);
	int ct=0;
	int axis;
	mp_pblock->GetValue (kSymAxis, 0, axis, FOREVER);

	switch (axis) {
	case 0:
	case 1:
	case 2:
		v0[axis]        = -1.0f;
		pv0[(axis+1)%3]	= 1.0f;
		pv0[(axis+2)%3]	= 1.0f;
		ct=1;
		break;
	
	case 3:
	case 4:
	case 5:
		v0[(axis)%3]    = -1.0f;
		pv0[(axis+1)%3]	= 1.0f;
		pv0[(axis+2)%3]	= 1.0f;
		v1[(axis+1)%3]  = -1.0f;
		pv1[(axis+2)%3]	= 1.0f;
		pv1[(axis+3)%3]	= 1.0f;
		ct=2;
		break;
	}

	Point3 pts[20];
	SetupAxisPoints(v0, pv0, size, pts);
	lp.proc(pts,5);
	lp.proc(pts+5,2);
	lp.proc(pts+7,3);
	lp.proc(pts+10,3);
	lp.proc(pts+13,3);
	lp.proc(pts+16,3);

	if (ct==2) {
		SetupAxisPoints(v1, pv1, size, pts);
		lp.proc(pts,5);
		lp.proc(pts+5,2);
		lp.proc(pts+7,3);
		lp.proc(pts+10,3);
		lp.proc(pts+13,3);
		lp.proc(pts+16,3);
	}
}

int SymmetryMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;
	Matrix3 tm = CompMatrix(t,inode,mc);

	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
	gw->setTransform(tm);
	gw->clearHitCode();
	DrawLineProc lp(gw);
	DrawGizmo(
		vpt->GetScreenScaleFactor(tm.GetTrans())*SCREEN_SCALE,lp);
	gw->setRndLimits(savedLimits);
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
}

int SymmetryMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc) {
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt[4];
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	if (mp_ip && mp_ip->GetSubObjectLevel() == 1) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
	}
	
	DrawLineProc lp(gw);
	DrawGizmo (vpt->GetScreenScaleFactor(tm.GetTrans())*SCREEN_SCALE,lp);

	gw->setRndLimits(savedLimits);
	return 0;
}

void SymmetryMod::GetWorldBoundBox (TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {	
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = CompMatrix(t,inode,mc);
	BoxLineProc bproc(&tm);
	DrawGizmo (vpt->GetScreenScaleFactor(tm.GetTrans())*SCREEN_SCALE, bproc);
	box = bproc.Box();
}

void SymmetryMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	SetXFormPacket pckt(val,partm,tmAxis);
	mp_mirror->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
}

void SymmetryMod::Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) {
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	mp_mirror->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
}

void SymmetryMod::Scale (TimeValue t, Matrix3 & partm, Matrix3 & tmAxis, Point3 & val, BOOL localOrigin) {
	SetXFormPacket pckt (val, localOrigin, partm, tmAxis);
	mp_mirror->SetValue (t, &pckt, true, CTRL_RELATIVE);
}

void SymmetryMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t, INode *node,ModContext *mc) {
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->Center(tm.GetTrans(),0);
}

void SymmetryMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t, INode *node,ModContext *mc) {
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
}

void SymmetryMod::ActivateSubobjSel(int level, XFormModes& modes) {
	switch (level) {
	case 1: // Mirror center
		modes = XFormModes (mp_moveMode, mp_rotMode, mp_scaleMode, mp_nuScaleMode, mp_squashMode, NULL);
		break;
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}

BOOL SymmetryMod::AssignController(Animatable *control,int subAnim) {
	if (subAnim==kSYM_MIRROR_REF) {
		ReplaceReference(kSYM_MIRROR_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);		
		return TRUE;
	} else {
		return FALSE;
	}
}

int SymmetryMod::SubNumToRefNum(int subNum) {
	if (subNum==kSYM_MIRROR_REF) return subNum;
	else return -1;
}

RefTargetHandle SymmetryMod::GetReference(int i) {
	switch (i) {
	case kSYM_PBLOCK_REF: return mp_pblock;
	case kSYM_MIRROR_REF: return mp_mirror;
	default: return NULL;
	}
}

void SymmetryMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kSYM_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	case kSYM_MIRROR_REF: mp_mirror = (Control*)rtarg; break;
	}
}

RefResult SymmetryMod::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
						   PartID& partID, RefMessage message) {
	int pid, weld;
	IParamMap2 *pPMap;
	HWND hDialog, hThreshLabel;

	switch (message) {
	case REFMSG_CHANGE:
		if (hTarget != mp_pblock) break;
		pid = mp_pblock->LastNotifyParamID ();
		if (pid != kSymWeld) break;

		pPMap = mp_pblock->GetMap(kSymmetryParams);
		if (!pPMap) break;
		hDialog = pPMap->GetHWnd();
		if (!hDialog) break;

		mp_pblock->GetValue (kSymWeld, TimeValue(0), weld, FOREVER);
		hThreshLabel = GetDlgItem (hDialog, IDC_SYM_THRESH_LABEL);
		if (hThreshLabel) EnableWindow (hThreshLabel, weld);
		break;
	}
	return REF_SUCCEED;
}

TSTR SymmetryMod::SubAnimName(int i) {
	switch (i) {
	case kSYM_PBLOCK_REF: return GetString (IDS_PARAMETERS);
	case kSYM_MIRROR_REF: return GetString (IDS_SYM_MIRROR);
	}
	return _T("");
}

static GenSubObjType SOT_Center(18);

ISubObjType *SymmetryMod::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Center.SetName(GetString(IDS_SYM_MIRROR));
	}

	switch(i) {
	case 0: return &SOT_Center;
	}

	return NULL;
}
