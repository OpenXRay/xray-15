/**********************************************************************
 *<
	FILE: fextrude.cpp

	DESCRIPTION: Face Extrude Modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/25/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"

//const Class_ID kFACE_EXTRUDE_CLASS_ID(SUB_EXTRUDE_CLASS_ID,0);
const Class_ID kFACE_EXTRUDE_CLASS_ID(0x70884612,0x12059e7);
const unsigned int kFEX_PBLOCK_REF(0);
const unsigned int kFEX_POINT_REF(1);

class FExtrudeMod : public Modifier {	
	IParamBlock2 *mp_pblock;
	Control *mp_base;		

	static IObjParam *mp_ip;
	static MoveModBoxCMode *mp_moveMode;

public:
	FExtrudeMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_FACE_EXTRUDE_MOD);}  
	virtual Class_ID ClassID() { return kFACE_EXTRUDE_CLASS_ID;}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_FACE_EXTRUDE_MOD);}
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum);

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
	ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR;}
	Class_ID InputType() {return defObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void ModifyTriObject(TimeValue t, ModContext &mc, TriObject *tobj);
	void ModifyPolyObject(TimeValue t, ModContext &mc, PolyObject *pobj);
	void ModifyPatchObject(TimeValue t, ModContext &mc, PatchObject *pobj);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);		
	void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);		
	
	//NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	int NumRefs() {return 2;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	int NumSubs() {return 2;}
	Animatable* SubAnim(int i) {return GetReference(i);}
	TSTR SubAnimName(int i);

	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);

	IOResult FExtrudeMod::Load (ILoad *iload);
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam       *FExtrudeMod::mp_ip        = NULL;
MoveModBoxCMode *FExtrudeMod::mp_moveMode  = NULL;

class FaceExtrudeClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FExtrudeMod; }
	const TCHAR *	ClassName() { return GetString(IDS_FACE_EXTRUDE_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kFACE_EXTRUDE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("FaceExtrude2"); }
	HINSTANCE HInstance() { return hInstance; }
};

static FaceExtrudeClassDesc fextrudeDesc;
extern ClassDesc* GetFaceExtrudeModDesc() {return &fextrudeDesc;}

static GenSubObjType SOT_Center(16);

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kFaceExtrudeParams };

// And enumerate the parameters within that block:
enum { kFexAmount, kFexScale, kFexType };

// Old ParamBlockDescID used with old ParamBlock-1:
static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  kFexAmount },
	{ TYPE_FLOAT, NULL, TRUE,  kFexScale },
	{ TYPE_INT,   NULL, FALSE, kFexType } // type - note old values were either 0 (for local normal) or 1 (for "from center").
};
#define PBLOCK_LENGTH 3
#define NUM_OLDVERSIONS 1

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc (descVer0, 3, 0)
};

// Parameters
static ParamBlockDesc2 face_extrude_param_blk (kFaceExtrudeParams, _T("Face Extrude Parameters"), 0, &fextrudeDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kFEX_PBLOCK_REF,
	// rollout
	IDD_FEX_PARAM, IDS_PARAMETERS, 0, 0, NULL,

	// Parameters
	kFexAmount, _T("FaceExtrudeAmount"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_FEX_AMOUNT,
		p_default, 0.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_ui, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_FEX_AMOUNT, IDC_FEX_AMOUNTSPIN, SPIN_AUTOSCALE,
		end,

	kFexScale, _T("FaceExtrudeScale"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_FEX_SCALE,
		p_default, 1.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_dim, stdPercentDim,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FEX_SCALE, IDC_FEX_SCALESPIN, .1f,
		end,
	
	kFexType, _T("FaceExtrudeType"), TYPE_INT, P_RESET_DEFAULT, IDS_FEX_TYPE,
		p_default, 0,
		p_range, 0, 3,
		p_ui, TYPE_RADIO, 3, IDC_FEX_TYPE_GROUP, IDC_FEX_TYPE_LOCAL, IDC_FEX_TYPE_CENTER,
		p_vals, 0,1,2,
		end,

	end
);

//--- Face extude mod methods -------------------------------


FExtrudeMod::FExtrudeMod() {
	mp_base   = NULL;
	mp_pblock = NULL;
	// Handled now by ClassDesc2:
	//MakeRefByID(
		//FOREVER, kFEX_PBLOCK_REF, 
		//CreateParameterBlock(
			//descVer0, PBLOCK_LENGTH, CURRENT_VERSION));

	MakeRefByID (FOREVER, kFEX_POINT_REF, NewDefaultPoint3Controller());
	fextrudeDesc.MakeAutoParamBlocks(this);
}

void FExtrudeMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mp_ip = ip;

	// Create sub object editing modes.
	mp_moveMode    = new MoveModBoxCMode(this,ip);
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	fextrudeDesc.BeginEditParams(ip,this,flags,prev);
}

void FExtrudeMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	mp_ip = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(mp_moveMode);	
	if (mp_moveMode) delete mp_moveMode;
	mp_moveMode = NULL;	

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	fextrudeDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle FExtrudeMod::Clone(RemapDir& remap) {
	FExtrudeMod *mod = new FExtrudeMod();
	mod->ReplaceReference(kFEX_PBLOCK_REF,mp_pblock->Clone(remap));
	mod->ReplaceReference(kFEX_POINT_REF,mp_base->Clone(remap));	
	BaseClone(this, mod, remap);
	return mod;
}

void FExtrudeMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
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

void FExtrudeMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj) {
	MNMesh &mesh = pobj->GetMesh();

	// Luna task 747
	// We cannot support specified normals in Face Extrude at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	float amount, scale;
	Point3 center;
	int type, i, j;

	mp_pblock->GetValue (kFexAmount, t, amount, iv);
	mp_pblock->GetValue (kFexScale, t, scale, iv);
	mp_pblock->GetValue (kFexType, t, type, iv);

	MNTempData temp(&mesh);

	bool abort=false;
	DWORD flag=MN_SEL;	// Later can be used to "inherit" selection from other SO levels.

	switch (type) {
	case 0:	// Group normals
		abort = !mesh.ExtrudeFaceClusters (*(temp.FaceClusters(flag)));
		if (!abort) {
			temp.Invalidate (PART_TOPO);	// Forces reevaluation of face clusters.
			mesh.GetExtrudeDirection (temp.ChamferData(), temp.FaceClusters(flag),
				temp.ClusterNormals (MNM_SL_FACE, flag)->Addr(0));
		}
		break;

	case 1:	// Local normals
		abort = !mesh.ExtrudeFaceClusters (*(temp.FaceClusters(flag)));
		if (!abort) {
			temp.Invalidate (PART_TOPO);	// Forces reevaluation of face clusters.
			mesh.GetExtrudeDirection (temp.ChamferData(), flag);
		}
		break;

	case 2:	// from center:
		mp_base->GetValue(t,&center,iv,CTRL_ABSOLUTE);
		abort = !mesh.ExtrudeFaceClusters (*(temp.FaceClusters(flag)));
		if (!abort) {
			temp.Invalidate (PART_GEOM);
			MNChamferData *mcd = temp.ChamferData();

			mcd->InitToMesh (mesh);
			mcd->ClearLimits ();

			int j, mp;
			// Make clear that there are no map values:
			for (mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) mcd->MDir(mp).SetCount(0);

			// Each vertex on flagged faces moves according to the direction from the center:
			for (i=0; i<mesh.numv; i++) {
				mcd->vdir[i] = Point3(0,0,0);
				if (mesh.v[i].GetFlag (MN_DEAD)) continue;
				if (!mesh.vfac[i].Count()) continue;
				bool hasflagged = false;
				for (j=0; j<mesh.vfac[i].Count(); j++) {
					MNFace & mf = mesh.f[mesh.vfac[i][j]];
					if (!mf.GetFlag (MN_SEL)) continue;
					hasflagged = true;
				}
				if (!hasflagged) continue;
				mcd->vdir[i] = Normalize((mesh.P(i)*(*mc.tm))-center);
			}
		}
		break;
	}
	if (abort) return;

	// Move verts
	for (i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		mesh.v[i].p += temp.ChamferData()->vdir[i]*amount;
	}

	// Scale verts
	if (scale!=1.0f) {
		temp.freeAll ();
		temp.SetMesh (&mesh);
		MNFaceClusters *fc = temp.FaceClusters();
		Tab<Point3> *centers = temp.ClusterCenters (MNM_SL_FACE);

		// Make sure each vertex is only scaled once.
		BitArray done;
		done.SetSize(mesh.numv);

		// scale each cluster independently
		for (i=0; i<fc->count; i++) {
			Point3 cent = (centers->Addr(0))[i];
			// Scale the cluster about its center
			for (j=0; j<mesh.numf; j++) {
				if (fc->clust[j]==i) {
					for (int k=0; k<mesh.f[j].deg; k++) {
						int index = mesh.f[j].vtx[k]; 
						if (done[index]) continue;
						done.Set(index);
						mesh.v[index].p = (mesh.v[index].p-cent)*scale + cent;							
					}
				}
			}
		}
	}

	mesh.InvalidateTopoCache ();
	mesh.FillInMesh ();
	pobj->UpdateValidity(GEOM_CHAN_NUM,iv);
}

void FExtrudeMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	float amount, scale;
	Point3 center;
	int i, j, type;
	
	mp_pblock->GetValue (kFexAmount, t, amount, iv);
	mp_pblock->GetValue (kFexScale, t, scale, iv);
	mp_pblock->GetValue (kFexType, t, type, iv);

	// Extrude the faces -- this just does the topological operation.
	MeshDelta tmd(mesh);
	tmd.ExtrudeFaces (mesh, mesh.faceSel);
	tmd.Apply(mesh);

	// Mark vertices used by selected faces
	BitArray sel;
	sel.SetSize(mesh.getNumVerts());
	for (i=0; i<mesh.getNumFaces(); i++) {
		if (mesh.faceSel[i]) {
			for (int j=0; j<3; j++) sel.Set(mesh.faces[i].v[j],TRUE);
		}
	}

	MeshTempData temp(&mesh);
	Point3 *vertexDir;
	Tab<Point3> centerBasedBuffer;
	if (type<2) {
		int extrusionType = type ? MESH_EXTRUDE_LOCAL : MESH_EXTRUDE_CLUSTER;
		vertexDir = temp.FaceExtDir(extrusionType)->Addr(0);
	} else {
		// We need to move all vertices away from the "center".
		// Compute the center point
		Point3 center;
		mp_base->GetValue(t,&center,iv,CTRL_ABSOLUTE);

		// Create array of vertex directions 
		centerBasedBuffer.SetCount (mesh.numVerts);
		vertexDir = centerBasedBuffer.Addr(0);
		for (i=0; i<mesh.numVerts; i++) {
			if (!sel[i]) continue;
			vertexDir[i] = Normalize((mesh.verts[i] * (*mc.tm)) - center);
		}
	}

	// Actually do the move:
	for (i=0; i<mesh.numVerts; i++) {
		if (!sel[i]) continue;
		mesh.verts[i] += amount * vertexDir[i];
	}

	// Scale verts
	if (scale!=1.0f) {
		temp.freeAll ();
		temp.SetMesh (&mesh);
		FaceClusterList *fc = temp.FaceClusters();
		Tab<Point3> *centers = temp.ClusterCenters (MESH_FACE);

		// Make sure each vertex is only scaled once.
		BitArray done;
		done.SetSize(mesh.getNumVerts());

		// scale each cluster independently
		for (i=0; (DWORD)i<fc->count; i++) {
			Point3 cent = (centers->Addr(0))[i];
			// Scale the cluster about its center
			for (j=0; j<mesh.getNumFaces(); j++) {
				if (fc->clust[j]==(DWORD)i) {
					for (int k=0; k<3; k++) {
						int index = mesh.faces[j].v[k]; 
						if (done[index]) continue;
						done.Set(index);
						mesh.verts[index] = (mesh.verts[index]-cent)*scale + cent;							
					}
				}
			}
		}
	}

	mesh.InvalidateTopologyCache ();
	tobj->UpdateValidity(GEOM_CHAN_NUM,iv);		
}

void FExtrudeMod::ModifyPatchObject (TimeValue t, ModContext &mc, PatchObject *pobj) {
	Interval iv = FOREVER;
	float amount, scale;
	Point3 center;
	int type;

	mp_pblock->GetValue (kFexAmount, t, amount, iv);
	mp_pblock->GetValue (kFexScale, t, scale, iv);
	mp_pblock->GetValue (kFexType, t, type, iv);

	// Don't know how to tease extrusion code out of EPatch  - sca.

	pobj->UpdateValidity(GEOM_CHAN_NUM,iv);		
}

Interval FExtrudeMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	Interval iv = FOREVER;
	float v;
	Point3 pt;
	int type;
	mp_pblock->GetValue(kFexAmount,t,v,iv);
	mp_pblock->GetValue(kFexScale,t,v,iv);
	mp_pblock->GetValue (kFexType, t, type, iv);
	if (type==2) mp_base->GetValue(t,&pt,iv,CTRL_ABSOLUTE);
	return iv;
}

int FExtrudeMod::HitTest (TimeValue t, INode* inode, int type,
						  int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {	
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
	mp_base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt,HOLLOW_BOX_MRKR);
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		res = 1;
	}			

	gw->setRndLimits(savedLimits);
	return res;
}

int FExtrudeMod::Display (TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc) {
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	
	// Draw start point
	if (mp_ip && mp_ip->GetSubObjectLevel() == 1) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
	}	
	mp_base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt,HOLLOW_BOX_MRKR);
	
	gw->setRndLimits(savedLimits);
	return 0;
}

void FExtrudeMod::GetWorldBoundBox (TimeValue t, INode* inode,
									ViewExp *vpt, Box3& box, ModContext *mc) {
	Matrix3 tm = CompMatrix(t,inode,mc);
	Point3 pt;
	box.Init();
	mp_base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	box += pt * tm;		
}

void FExtrudeMod::Move (TimeValue t, Matrix3& partm,
						Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	mp_base->SetValue (t,VectorTransform(tmAxis*Inverse(partm),val),
		TRUE,CTRL_RELATIVE);
}

void FExtrudeMod::GetSubObjectCenters (SubObjAxisCallback *cb,
									   TimeValue t,	INode *node,ModContext *mc)	{
	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 p;
	mp_base->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
	tm.PreTranslate(p);
	cb->Center(tm.GetTrans(),0);
}

void FExtrudeMod::GetSubObjectTMs (SubObjAxisCallback *cb,
								   TimeValue t, INode *node,ModContext *mc) {
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
}

void FExtrudeMod::ActivateSubobjSel(int level, XFormModes& modes) {
	switch (level) {
	case 1: // Points
		modes = XFormModes(mp_moveMode,NULL,NULL,NULL,NULL,NULL);
		break;		
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}

BOOL FExtrudeMod::AssignController(Animatable *control,int subAnim) {
	if (subAnim==kFEX_POINT_REF) {
		ReplaceReference(kFEX_POINT_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);		
		return TRUE;
	} else {
		return FALSE;
	}
}

int FExtrudeMod::SubNumToRefNum(int subNum) {
	if (subNum==kFEX_POINT_REF) return subNum;
	else return -1;
}

RefTargetHandle FExtrudeMod::GetReference(int i) {
	switch (i) {
	case kFEX_PBLOCK_REF: return mp_pblock;
	case kFEX_POINT_REF:  return mp_base;
	default: return NULL;
	}
}

void FExtrudeMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kFEX_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	case kFEX_POINT_REF : mp_base   = (Control*)rtarg; break;
	}
}

TSTR FExtrudeMod::SubAnimName(int i) {
	switch (i) {
	case kFEX_POINT_REF:  return GetString(IDS_FEX_CENTER);
	default:         return _T("");
	}
}

Matrix3 FExtrudeMod::CompMatrix (TimeValue t,INode *inode,ModContext *mc) {
	Interval iv;
	Matrix3 tm(1);	
	if (mc && mc->tm) tm = Inverse(*(mc->tm));
	if (inode) tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
	return tm;
}

int FExtrudeMod::NumSubObjTypes() { 
	return 1;
}

ISubObjType *FExtrudeMod::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Center.SetName(GetString(IDS_FEX_CENTER));
	}

	switch(i) {
	case 0:
		return &SOT_Center;
	}

	return NULL;
}


// Old version loading:

// Face Extrude used to use regular param blocks, and now uses pb2's.
// But one parameter, "type", used to be a simple checkbox.
// Now it's a three-way radio, with the old off/on positions occupying spaces 2 and 3.
// So for backward compatibility we need to add one to this value after loading.

// STEVE: Note that we can't test this out until we actually replace the old Face Extrude.

class FEVersionFix : public PostLoadCallback {
public:
	FExtrudeMod *mod;

	FEVersionFix (FExtrudeMod *m) : mod(m) { }
	int Priority() { return 0; }	// necessary to make this run before the PLCB.
	void proc(ILoad *iload);
};

void FEVersionFix::proc (ILoad *iload) {
	int type;
	// See if we're dealing with an old parameter block:
	ReferenceTarget* ref = mod->GetReference (kFEX_PBLOCK_REF);
	if (ref != NULL && ref->ClassID() != Class_ID(PARAMETER_BLOCK2_CLASS_ID, 0)) {
		// Must be an old param block.
		IParamBlock *mp_pblock = (IParamBlock*) ref;
		mp_pblock->GetValue (kFexType, TimeValue(0), type, FOREVER);
		mp_pblock->SetValue (kFexType, TimeValue(0), type+1);
	}
	delete this;
}

IOResult FExtrudeMod::Load (ILoad *iload) {
	Modifier::Load(iload);

	// This callback increments the old "type" parameter to match the new numbering.
	iload->RegisterPostLoadCallback (new FEVersionFix(this));

	// this callback allows old Paramblocks to be loaded in and automatically converted to 
	// the ParamBlock2 mechanism - NH
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB (versions, NUM_OLDVERSIONS,
		&face_extrude_param_blk, this, kFEX_PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

	return IO_OK;
}

