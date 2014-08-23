/**********************************************************************
 *<
	FILE: afregion.cpp

	DESCRIPTION:  Affect region modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/16/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#ifndef NO_MODIFIER_AFFECTREGION // JP Morel - June 28th 2002

#include "iparamm.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"
#include "MeshDLib.h"

#define PBLOCK_REF	0
#define POINT1_REF	1
#define POINT2_REF	2

class AFRMod : public Modifier {	
	public:
		IParamBlock *pblock;
		Control *p1, *p2;
		BYTE sel[2];

		static IObjParam *ip;
		static IParamMap *pmapParam;
		static AFRMod *editMod;
		static MoveModBoxCMode *moveMode;
		
		AFRMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_AFRMOD);}  
		virtual Class_ID ClassID() { return Class_ID(AFFECTREGION_CLASS_ID,0);}		
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_AFRMOD);}
		int SubNumToRefNum(int subNum);
		BOOL AssignController(Animatable *control,int subAnim);

		// From modifier
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		ChannelMask ChannelsChanged() {return PART_GEOM;}
		Class_ID InputType() {return defObjectClassID;}
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
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		int NumRefs() {return 3;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int NumSubs() {return 3;}
		Animatable* SubAnim(int i) {return GetReference(i);}
		TSTR SubAnimName(int i);

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);
	};


class AFRDeformer : public Deformer {
	public:		
		float falloff, pinch, bubble;
		Point3 p1, p2;
		Tab<Point3> *normals;
		AFRDeformer(
			ModContext &mc, float f, float p, float b,
			Point3 pt1, Point3 pt2, Tab<Point3> *n=NULL);
		Point3 Map(int i, Point3 p);
	};


//--- ClassDescriptor and class vars ---------------------------------

IParamMap       *AFRMod::pmapParam = NULL;
IObjParam       *AFRMod::ip        = NULL;
AFRMod          *AFRMod::editMod   = NULL;
MoveModBoxCMode *AFRMod::moveMode  = NULL;
		

class AFRClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new AFRMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_AFRMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(AFFECTREGION_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static AFRClassDesc afrDesc;
extern ClassDesc* GetAFRModDesc() {return &afrDesc;}

static GenSubObjType SOT_Points(13);

//--- Parameter map/block descriptors -------------------------------

#define PB_FALLOFF		0
#define PB_BACKFACE		1
#define PB_PINCH		2
#define PB_BUBBLE		3

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Falloff
	ParamUIDesc(
		PB_FALLOFF,
		EDITTYPE_FLOAT,
		IDC_AFR_FALLOFF,IDC_AFR_FALLOFFSPIN,
		0.0f,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Backface
	ParamUIDesc(PB_BACKFACE,TYPE_SINGLECHEKBOX,IDC_AFR_BACKFACE),

	// Pinch
	ParamUIDesc(
		PB_PINCH,
		EDITTYPE_FLOAT,
		IDC_AFR_PINCH,IDC_AFR_PINCHSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Bubble
	ParamUIDesc(
		PB_BUBBLE,
		EDITTYPE_FLOAT,
		IDC_AFR_BUBBLE,IDC_AFR_BUBBLESPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	};
#define PARAMDESC_LENGTH	4

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  0 },		// Falloff	
	{ TYPE_INT,   NULL, FALSE, 1 },		// Backface
	{ TYPE_FLOAT, NULL, TRUE,  2 },		// Pinch
	{ TYPE_FLOAT, NULL, TRUE,  3 },		// Bubble
	};
#define PBLOCK_LENGTH	4

#define CURRENT_VERSION	0



//--- Affect region mod methods -------------------------------

AFRMod::AFRMod() 
	{
	MakeRefByID(
		FOREVER, PBLOCK_REF, 
		CreateParameterBlock(
			descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	p1 = p2 = NULL;
	MakeRefByID(FOREVER,POINT1_REF,NewDefaultPoint3Controller()); 
	MakeRefByID(FOREVER,POINT2_REF,NewDefaultPoint3Controller());
	sel[0] = sel[1] = 0;
	pblock->SetValue(PB_FALLOFF,0,20.0f);
	Point3 pt(0,0,25);
	p2->SetValue(0,&pt);
	}

void AFRMod::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editMod  = this;

	// Add our sub object type
	// TSTR type1(GetString(IDS_RB_AFRPOINTS));
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
		MAKEINTRESOURCE(IDD_AFRPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	}

void AFRMod::EndEditParams(
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

RefTargetHandle AFRMod::Clone(RemapDir& remap)
	{
	AFRMod *mod = new AFRMod();
	mod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	mod->ReplaceReference(POINT1_REF,p1->Clone(remap));
	mod->ReplaceReference(POINT2_REF,p2->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
	}


void AFRMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	Interval iv = FOREVER;
	float f, p, b;
	int backface;
	Point3 pt1, pt2;
	pblock->GetValue(PB_FALLOFF,t,f,iv);
	pblock->GetValue(PB_PINCH,t,p,iv);
	pblock->GetValue(PB_BUBBLE,t,b,iv);
	pblock->GetValue(PB_BACKFACE,t,backface,iv);
	p1->GetValue(t,&pt1,iv,CTRL_ABSOLUTE);
	p2->GetValue(t,&pt2,iv,CTRL_ABSOLUTE);
	if (f==0.0) {
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
		return;
	}
	Tab<Point3> normals;
	if (backface) {
		// Need to get vertex normals.
		if (os->obj->IsSubClassOf(triObjectClassID)) {
			TriObject *tobj = (TriObject*)os->obj;
			AverageVertexNormals (tobj->GetMesh(), normals);
		} else if (os->obj->IsSubClassOf (polyObjectClassID)) {
			PolyObject *pobj = (PolyObject *) os->obj;
			MNMesh &mesh = pobj->GetMesh();
			normals.SetCount (mesh.numv);
			Point3 *vn = normals.Addr(0);
			for (int i=0; i<mesh.numv; i++) {
				if (mesh.v[i].GetFlag (MN_DEAD)) vn[i]=Point3(0,0,0);
				else vn[i] = mesh.GetVertexNormal (i);
			}
#ifndef NO_PATCHES
		} else if (os->obj->IsSubClassOf (patchObjectClassID)) {
			PatchObject *pobj = (PatchObject *) os->obj;
			normals.SetCount (pobj->NumPoints ());
			Point3 *vn = normals.Addr(0);
			for (int i=0; i<pobj->NumPoints(); i++) vn[i] = pobj->VertexNormal (i);
#endif
		}
	}
	if (normals.Count()) {
		AFRDeformer deformer(mc,f,p,b,pt1,pt2,&normals);
		os->obj->Deform(&deformer, TRUE);
	} else {
		AFRDeformer deformer(mc,f,p,b,pt1,pt2);
		os->obj->Deform(&deformer, TRUE);
	}	
	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
	}

Interval AFRMod::LocalValidity(TimeValue t)
	{
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval iv = FOREVER;
	float v;
	Point3 pt;
	pblock->GetValue(PB_FALLOFF,t,v,iv);
	pblock->GetValue(PB_PINCH,t,v,iv);
	pblock->GetValue(PB_BUBBLE,t,v,iv);
	p1->GetValue(t,&pt,iv,CTRL_ABSOLUTE);
	p2->GetValue(t,&pt,iv,CTRL_ABSOLUTE);
	return iv;
	}


Matrix3 AFRMod::CompMatrix(TimeValue t,INode *inode,ModContext *mc)
	{
	Interval iv;
	Matrix3 tm(1);	
	if (mc && mc->tm) tm = Inverse(*(mc->tm));
	if (inode) 
	{
#ifdef DESIGN_VER
		tm = tm * inode->GetObjTMBeforeWSM(GetCOREInterface()->GetTime(),&iv);
#else
		tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
#endif // DESIGN_VER
	}
	return tm;
	}

int AFRMod::HitTest(
		TimeValue t, INode* inode, 
		int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt, ModContext* mc)
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

	// Hit test start point
	if ((flags&HIT_SELONLY   &&  sel[0]) ||
		(flags&HIT_UNSELONLY && !sel[0]) ||
		!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) {
	
		gw->clearHitCode();
		p1->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
		gw->marker(&pt,HOLLOW_BOX_MRKR);
		if (gw->checkHitCode()) {
			vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
			res = 1;
			}
		}

	// Hit test end point
	if ((flags&HIT_SELONLY   &&  sel[1]) ||
		(flags&HIT_UNSELONLY && !sel[1]) ||
		!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) {
	
		gw->clearHitCode();
		p2->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
		gw->marker(&pt,HOLLOW_BOX_MRKR);
		if (gw->checkHitCode()) {
			vpt->LogHit(inode, mc, gw->getHitDistance(), 1, NULL); 
			res = 1;
			}
		}

	gw->setRndLimits(savedLimits);
	return res;
	}

int AFRMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc)
	{
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt[4];
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	
	// Draw start point
	if (ip && ip->GetSubObjectLevel() == 1) {
		if (sel[0]) 
			 gw->setColor(LINE_COLOR, (float)1.0, (float)0.0, (float)0.0);			 
		else //gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
			 gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	p1->GetValue(t,&pt[0],FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt[0],HOLLOW_BOX_MRKR);

	// Draw end point
	if (ip && ip->GetSubObjectLevel() == 1) {
		if (sel[1]) 
			 gw->setColor(LINE_COLOR, (float)1.0, (float)0.0, (float)0.0);
		else //gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
			   gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	p2->GetValue(t,&pt[1],FOREVER,CTRL_ABSOLUTE);
	gw->marker(&pt[1],HOLLOW_BOX_MRKR);

	// Draw a line inbetween
	if (ip && ip->GetSubObjectLevel() == 1) {
		//gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor(LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
	
	// Draw arrowhead
	Matrix3 vtm;
	vpt->GetAffineTM(vtm);
	Point3 vz = vtm.GetRow(2) * Inverse(tm);
	Point3 dp = (pt[1]-pt[0]);
	Point3 v = Normalize(vz^dp) * Length(dp) * 0.2f;
	pt[0] = pt[1] - (0.2f*dp) + v;
	pt[2] = pt[1] - (0.2f*dp) - v;
	gw->polyline(3, pt, NULL, NULL, 0, NULL);
	v = Normalize(v^dp) * Length(dp) * 0.2f;
	pt[0] = pt[1] - (0.2f*dp) + v;
	pt[2] = pt[1] - (0.2f*dp) - v;
	gw->polyline(3, pt, NULL, NULL, 0, NULL);

	gw->setRndLimits(savedLimits);
	return 0;
	}

void AFRMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,inode,mc);
	Point3 pt1, pt2;
	box.Init();
	p1->GetValue(t,&pt1,FOREVER,CTRL_ABSOLUTE);
	box += pt1 * tm;
	p2->GetValue(t,&pt2,FOREVER,CTRL_ABSOLUTE);
	box += pt2 * tm;
	
	Matrix3 vtm;
	vpt->GetAffineTM(vtm);
	Point3 vz = vtm.GetRow(2) * Inverse(tm);
	Point3 dp = (pt2-pt1);
	Point3 v = Normalize(vz^dp) * Length(dp) * 0.2f;
	box += (pt2 - (0.2f*dp) + v) * tm;
	box += (pt2 - (0.2f*dp) - v) * tm;
	v = Normalize(v^dp) * Length(dp) * 0.2f;
	box += (pt2 - (0.2f*dp) + v) * tm;
	box += (pt2 - (0.2f*dp) - v) * tm;	
	}

void AFRMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
	if (sel[0]) {
		p1->SetValue(
			t,VectorTransform(tmAxis*Inverse(partm),val),
			TRUE,CTRL_RELATIVE);
		}
	if (sel[1]) {
		p2->SetValue(
			t,VectorTransform(tmAxis*Inverse(partm),val),
			TRUE,CTRL_RELATIVE);
		}
	}

void AFRMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 pt(0,0,0), p;
	int c=0;
	if (sel[0]) {
		p1->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
		pt += p;
		c++;
		}
	if (sel[1]) {
		p2->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
		pt += p;
		c++;
		}
	if (c) pt /= float(c);
	tm.PreTranslate(pt);
	cb->Center(tm.GetTrans(),0);
	}

void AFRMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
	}


void AFRMod::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));
	while (hitRec) {
		assert(hitRec->hitInfo<=1);
		BOOL state = selected;
		if (invert) state = !sel[hitRec->hitInfo];
		if (state) sel[hitRec->hitInfo] = 1;
		else       sel[hitRec->hitInfo] = 0;
		if (!all) break;
		hitRec = hitRec->Next();
		}	
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void AFRMod::ClearSelection(int selLevel)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel[0] = sel[1] = 0;
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void AFRMod::SelectAll(int selLevel)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel[0] = sel[1] = 1;
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void AFRMod::InvertSelection(int selLevel)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));
	sel[0] = !sel[0];
	sel[1] = !sel[1];
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void AFRMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	switch (level) {
		case 1: // Points
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}

BOOL AFRMod::AssignController(Animatable *control,int subAnim)
	{
	ReplaceReference(subAnim,(Control*)control);
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
	}

int AFRMod::SubNumToRefNum(int subNum)
	{
	switch (subNum) {
		case POINT1_REF: return POINT1_REF;
		case POINT2_REF: return POINT2_REF;		
		}
	return -1;
	}

RefTargetHandle AFRMod::GetReference(int i)
	{
	switch (i) {
		case PBLOCK_REF: return pblock;
		case POINT1_REF: return p1;
		case POINT2_REF: return p2;
		default: return NULL;
		}
	}

void AFRMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case PBLOCK_REF: pblock = (IParamBlock*)rtarg; break;
		case POINT1_REF: p1     = (Control*)rtarg; break;
		case POINT2_REF: p2     = (Control*)rtarg; break;
		}
	}

TSTR AFRMod::SubAnimName(int i)
	{
	switch (i) {
		case POINT1_REF: return GetString(IDS_RB_STARTPOINT); break;
		case POINT2_REF: return GetString(IDS_RB_ENDPOINT); break;
		default: return _T(""); break;
		}
	}

RefResult AFRMod::NotifyRefChanged(
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
				case PB_FALLOFF: gpd->dim = stdWorldDim; break;
				case PB_PINCH:   
				case PB_BUBBLE:	 gpd->dim = stdNormalizedDim; break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;			
			switch (gpn->index) {
				case PB_FALLOFF: gpn->name = GetString(IDS_AFR_FALLOFF); break;
				case PB_PINCH:   gpn->name = GetString(IDS_AFR_PINCH); break;
				case PB_BUBBLE:	 gpn->name = GetString(IDS_AFR_BUBBLE); break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}

int AFRMod::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *AFRMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Points.SetName(GetString(IDS_RB_AFRPOINTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Points;
	}
	return NULL;
}


AFRDeformer::AFRDeformer(
		ModContext &mc, float f, float p, float b,
		Point3 pt1, Point3 pt2, Tab<Point3> *n)
	{
	Matrix3 tm, itm;
	if (mc.tm) tm = *mc.tm;
	else tm.IdentityMatrix();
	itm     = Inverse(tm);	
	falloff = f;
	pinch   = p;
	bubble  = b;
	pt1 = pt1 * itm;
	pt2 = pt2 * itm;
	p1      = pt1;
	p2      = pt2-p1;
	normals = n;
	}

Point3 AFRDeformer::Map(int i, Point3 p)
	{
	float dist = Length(p-p1), d;
	if (dist>falloff) return p;
	if (normals) {		
		d = DotProd(p2,(*normals)[i]);
		if (d<0.0) return p;
		}
	float u = ((falloff - dist)/falloff);	
	float u2 = u*u, s = 1.0f-u;	
	return p + p2 * ((3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2);	
	}

#endif // NO_MODIFIER_AFFECTREGION
