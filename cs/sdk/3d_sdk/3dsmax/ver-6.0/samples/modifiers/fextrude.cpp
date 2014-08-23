/**********************************************************************
 *<
	FILE: fextrude.cpp

	DESCRIPTION: Face Extrude Modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/25/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"
#include "meshadj.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"

#ifndef NO_MODIFIER_FACE_EXTRUDE // JP Morel - July 23th 2002


#define PBLOCK_REF	0
#define POINT_REF	1

class FExtrudeMod : public Modifier {	
	public:
		IParamBlock *pblock;
		Control *base;		

		static IObjParam *ip;
		static IParamMap *pmapParam;
		static FExtrudeMod *editMod;
		static MoveModBoxCMode *moveMode;
		
		FExtrudeMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_FACEEXTRUDEMOD);}  
		virtual Class_ID ClassID() { return Class_ID(SUB_EXTRUDE_CLASS_ID,0);}
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_FACEEXTRUDEMOD);}
		BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum);

		// From modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR;}
		Class_ID InputType() {return triObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
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

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap       *FExtrudeMod::pmapParam = NULL;
IObjParam       *FExtrudeMod::ip        = NULL;
FExtrudeMod     *FExtrudeMod::editMod   = NULL;
MoveModBoxCMode *FExtrudeMod::moveMode  = NULL;
		

class FaceExtrudeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FExtrudeMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_FACEEXTRUDEMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SUB_EXTRUDE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static FaceExtrudeClassDesc fextrudeDesc;
extern ClassDesc* GetFaceExtrudeModDesc() {return &fextrudeDesc;}

static GenSubObjType SOT_Center(16);

//--- Parameter map/block descriptors -------------------------------

#define PB_AMOUNT	0
#define PB_SCALE	1
#define PB_CENTER	2

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Amount
	ParamUIDesc(
		PB_AMOUNT,
		EDITTYPE_FLOAT,
		IDC_EXT_AMOUNT,IDC_EXT_AMOUNTSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Scale
	ParamUIDesc(
		PB_SCALE,
		EDITTYPE_FLOAT,
		IDC_EXT_SCALE,IDC_EXT_SCALESPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Backface
	ParamUIDesc(PB_CENTER,TYPE_SINGLECHEKBOX,IDC_EXT_CENTER),
	};
#define PARAMDESC_LENGTH	3

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  0 },		// Amount
	{ TYPE_FLOAT, NULL, TRUE,  1 },		// Scale
	{ TYPE_INT,   NULL, FALSE, 2 },		// Center
	};
#define PBLOCK_LENGTH	3

#define CURRENT_VERSION	0



//--- Face extude mod methods -------------------------------


FExtrudeMod::FExtrudeMod()
	{
	base   = NULL;
	pblock = NULL;
	MakeRefByID(
		FOREVER, PBLOCK_REF, 
		CreateParameterBlock(
			descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	MakeRefByID(FOREVER,POINT_REF,NewDefaultPoint3Controller()); 
	pblock->SetValue(PB_SCALE,0,100.0f);
	} 

void FExtrudeMod::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editMod  = this;

	// Add our sub object type
	// TSTR type1(GetString(IDS_RB_EXTRUDECENT));
	// const TCHAR *ptype[] = {type1};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 1);

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_FACEEXTRUDEPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	}

void FExtrudeMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);	
	if (moveMode) delete moveMode;
	moveMode = NULL;	

	DestroyCPParamMap(pmapParam);
	}

RefTargetHandle FExtrudeMod::Clone(RemapDir& remap)
	{
	FExtrudeMod *mod = new FExtrudeMod();
	mod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	mod->ReplaceReference(POINT_REF,base->Clone(remap));	
	BaseClone(this, mod, remap);
	return mod;
	}

void FExtrudeMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		Mesh &mesh = tobj->GetMesh();
		Interval iv = FOREVER;
		float a, s;
		Point3 pt, center;
		int c;
		
		pblock->GetValue(PB_AMOUNT,t,a,iv);
		pblock->GetValue(PB_SCALE,t,s,iv);
		pblock->GetValue(PB_CENTER,t,c,iv);
		base->GetValue(t,&pt,iv,CTRL_ABSOLUTE);		

		// Extrude the faces -- this just creates the new faces
		mesh.ExtrudeFaces();

		// Build normals of selected faces only		
		Tab<Point3> normals;
		if (!c) {
			normals.SetCount(mesh.getNumVerts());
			for (int i=0; i<mesh.getNumVerts(); i++) {
				normals[i] = Point3(0,0,0);
				}
			for (i=0; i<mesh.getNumFaces(); i++) {
				if (mesh.faceSel[i]) {
					Point3 norm = 
						(mesh.verts[mesh.faces[i].v[1]]-mesh.verts[mesh.faces[i].v[0]]) ^
						(mesh.verts[mesh.faces[i].v[2]]-mesh.verts[mesh.faces[i].v[1]]);
					for (int j=0; j<3; j++) {				
						normals[mesh.faces[i].v[j]] += norm;
						}
					}
				}			
			for (i=0; i<mesh.getNumVerts(); i++) {
				normals[i] = Normalize(normals[i]);
				}
		} else {
			// Compute the center point			
			base->GetValue(t,&center,iv,CTRL_ABSOLUTE);			
			}

		// Mark vertices used by selected faces
		BitArray sel;
		sel.SetSize(mesh.getNumVerts());
		for (int i=0; i<mesh.getNumFaces(); i++) {
			if (mesh.faceSel[i]) {
				for (int j=0; j<3; j++) sel.Set(mesh.faces[i].v[j],TRUE);
				}
			}

		// Move selected verts
		for (i=0; i<mesh.getNumVerts(); i++) {
			if (sel[i]) {
				if (!c) {
					mesh.verts[i] += normals[i]*a;
				} else {
					Point3 vect = Normalize((mesh.verts[i] * (*mc.tm))
						- center);
					mesh.verts[i] += vect*a;
					}
				}
			}
		
		// Scale verts
		if (s!=100.0f) {
			s /= 100.0f;

			AdjEdgeList ae(mesh);
			AdjFaceList af(mesh,ae);
			FaceClusterList clust(mesh.faceSel,af);
			
			// Make sure each vertex is only scaled once.
			BitArray done;
			done.SetSize(mesh.getNumVerts());

			// scale each cluster independently
			for (int i=0; (DWORD)i<clust.count; i++) {
				// First determine cluster center
				Point3 cent(0,0,0);
				int ct=0;
				for (int j=0; j<mesh.getNumFaces(); j++) {
					if (clust[j]==(DWORD)i) {
						for (int k=0; k<3; k++) {
							cent += mesh.verts[mesh.faces[j].v[k]];
							ct++;
							}
						}
					}
				if (ct) cent /= float(ct);

				// Now scale the cluster about its center
				for (j=0; j<mesh.getNumFaces(); j++) {
					if (clust[j]==(DWORD)i) {
						for (int k=0; k<3; k++) {
							int index = mesh.faces[j].v[k]; 
							if (done[index]) continue;
							done.Set(index);
							mesh.verts[index] = 
								(mesh.verts[index]-cent)*s + cent;							
							}
						}
					}
				}
			}
		
		mesh.InvalidateTopologyCache ();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);		
		}
	}

Interval FExtrudeMod::LocalValidity(TimeValue t)
	{
	// aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval iv = FOREVER;
	float v;
	Point3 pt;
	pblock->GetValue(PB_AMOUNT,t,v,iv);
	pblock->GetValue(PB_SCALE,t,v,iv);
	base->GetValue(t,&pt,iv,CTRL_ABSOLUTE);
	return iv;
	}

int FExtrudeMod::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{	
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
	base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt,HOLLOW_BOX_MRKR);
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		res = 1;
		}			

	gw->setRndLimits(savedLimits);
	return res;
	}

int FExtrudeMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc)
	{
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	
	// Draw start point
	if (ip && ip->GetSubObjectLevel() == 1) {
		//gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt,HOLLOW_BOX_MRKR);
	
	gw->setRndLimits(savedLimits);
	return 0;
	}

void FExtrudeMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,inode,mc);
	Point3 pt;
	box.Init();
	base->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
	box += pt * tm;		
	}

void FExtrudeMod::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
	base->SetValue(
		t,VectorTransform(tmAxis*Inverse(partm),val),
		TRUE,CTRL_RELATIVE);
	}

void FExtrudeMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 p;
	base->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
	tm.PreTranslate(p);
	cb->Center(tm.GetTrans(),0);
	}

void FExtrudeMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
	}

void FExtrudeMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	switch (level) {
		case 1: // Points
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}

BOOL FExtrudeMod::AssignController(Animatable *control,int subAnim)
	{
	if (subAnim==POINT_REF) {
		ReplaceReference(POINT_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);		
		return TRUE;
	} else {
		return FALSE;
		}
	}

int FExtrudeMod::SubNumToRefNum(int subNum)
	{
	if (subNum==POINT_REF) return subNum;
	else return -1;
	}

RefTargetHandle FExtrudeMod::GetReference(int i)
	{
	switch (i) {
		case PBLOCK_REF: return pblock;
		case POINT_REF:  return base;
		default: return NULL;
		}
	}

void FExtrudeMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case PBLOCK_REF: pblock = (IParamBlock*)rtarg; break;
		case POINT_REF : base   = (Control*)rtarg; break;
		}
	}

TSTR FExtrudeMod::SubAnimName(int i)
	{
	switch (i) {		
		case POINT_REF:  return GetString(IDS_RB_EXTRUDECENT);
		default:         return _T("");
		}
	}

RefResult FExtrudeMod::NotifyRefChanged( 
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this && pmapParam) pmapParam->Invalidate();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_AMOUNT:  gpd->dim = stdWorldDim;   break;
				case PB_SCALE:   gpd->dim = defaultDim; break;				
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;			
			switch (gpn->index) {
				case PB_AMOUNT:  gpn->name = GetString(IDS_EXT_AMOUNT); break;
				case PB_SCALE:   gpn->name = GetString(IDS_EXT_SCALE);  break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}

Matrix3 FExtrudeMod::CompMatrix(
		TimeValue t,INode *inode,ModContext *mc)
	{
	Interval iv;
	Matrix3 tm(1);	
	if (mc && mc->tm) tm = Inverse(*(mc->tm));
	if (inode) {
#ifdef DESIGN_VER
		tm = tm * inode->GetObjTMBeforeWSM(GetCOREInterface()->GetTime(),&iv);
#else
		tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
#endif // DESIGN_VER
		}
	return tm;
	}

int FExtrudeMod::NumSubObjTypes() 
	{ 
	return 1;
	}

ISubObjType *FExtrudeMod::GetSubObjType(int i) 
	{	
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Center.SetName(GetString(IDS_RB_EXTRUDECENT));
		}

	switch(i) {
		case 0:
			return &SOT_Center;
		}
	return NULL;
	}

#endif // NO_MODIFIER_FACE_EXTRUDE
