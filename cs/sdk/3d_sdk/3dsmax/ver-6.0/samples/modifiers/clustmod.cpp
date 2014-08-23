/**********************************************************************
 *<
	FILE: clustmod.cpp   

	DESCRIPTION:  Vertex cluster animating modifier - XForm

	CREATED BY: Rolf Berteig

	HISTORY: created 24 August, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "buildver.h"

#ifndef NO_MODIFIER_XFORM // JP Morel - June 28th 2002

#include "simpmod.h"


class ClustMod : public SimpleMod {	
	private:		
		int selLevel;

	public:												
		ClustMod();
				
		void DeleteThis() {delete this;}
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_XFORM_CLASS); } 
		Class_ID ClassID() { return Class_ID(CLUSTOSM_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap);
		TCHAR *GetObjectName() { return GetString(IDS_RB_XFORM); }
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);	
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);		

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void ActivateSubobjSel(int level, XFormModes& modes);
	};


class ClustDeformer: public Deformer {
	public:
		Matrix3 tm;
			
		ClustDeformer();
		ClustDeformer(Matrix3& modmat);		
		
		Point3 Map(int i, Point3 p); 
	};


class ClustClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new ClustMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_XFORM_CLASS); }
	SClass_ID		SuperClassID() {return OSM_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(CLUSTOSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	};
static ClustClassDesc clustDesc;
extern ClassDesc* GetClustModDesc() { return &clustDesc; }

/*--------------------------------------------------------------------*/



RefTargetHandle ClustMod::Clone(RemapDir& remap) 
	{
	ClustMod* newmod = new ClustMod();			
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}

ClustDeformer::ClustDeformer() 
	{ 	
	tm.IdentityMatrix();	
	}

Point3 ClustDeformer::Map(int i, Point3 p)
	{	
	p = p * tm;	
	return p;
	}

ClustDeformer::ClustDeformer(Matrix3& modmat) 
	{	
	tm = modmat;		
	} 

ClustMod::ClustMod()
	{
	// We'll use A_WORK4 to indicate that this mod was just created.
	// The first time we enter BeginEditParams() we'll go directly into
	// sub object selection 
	//SetAFlag(A_WORK4);
	selLevel = 1;
	}

Deformer& ClustMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{	
	static ClustDeformer deformer;
	assert(0);
	return deformer;
	}

void ClustMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
	//if (TestAFlag(A_WORK4)) ip->SetSubObjectLevel(1);
	//ClearAFlag(A_WORK4);
	if (selLevel) ip->SetSubObjectLevel(selLevel);
	}

void ClustMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	selLevel = level;
	SimpleMod::ActivateSubobjSel(level,modes);
	}

#define DEFORMER_TM	(ptm * ctm)
static Matrix3 CompTM(
		Matrix3 &ptm, Matrix3 &ctm, Matrix3 *mctm, int i)
	{
	if (mctm) {
		if (i) return  ptm * ctm * Inverse(*mctm);
		else return *mctm * ptm * ctm * Inverse(*mctm);
	} else {
		return ptm * ctm;
		}
	}

void ClustMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState * os, INode *node)
	{
	Interval valid = FOREVER;
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,valid,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,valid,CTRL_RELATIVE);	
	//Matrix3 tm = DEFORMER_TM;
	Matrix3 tm = CompTM(ptm,ctm,mc.tm,0);

	ClustDeformer deformer(tm);
	
	os->obj->Deform(&deformer, TRUE);
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);
	}

// RB 3/31/99: Push mc->box through this to handle the case where an XForm
// mod is applied to an empty sub-object selection.
static Box3& MakeBoxNotEmpty(Box3 &box)
	{
	static Box3 smallBox(Point3(-5,-5,-5),Point3( 5, 5, 5));
	if (box.IsEmpty()) return smallBox;
	else return box;
	}

int ClustMod::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{	
	int savedLimits;
	Matrix3 obtm = inode->GetObjectTM(t);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();	
	gw->setTransform(obtm);

	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	

	if (ip && ip->GetSubObjectLevel() == 1) {		
		//Matrix3 tm = DEFORMER_TM;
		Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);

		ClustDeformer deformer(tm);
		if (mc->box->pmin==mc->box->pmax) {
			Point3 pt = mc->box->pmin * tm;
			gw->marker(&pt,ASTERISK_MRKR);
		} else {
			DoModifiedBox(MakeBoxNotEmpty(*mc->box),deformer,DrawLineProc(gw));
			}
		}

	if (ip && (ip->GetSubObjectLevel() == 1 ||
	           ip->GetSubObjectLevel() == 2)) {		
		//obtm = ctm * obtm;
		if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
		else obtm = ctm * obtm;

		gw->setTransform(obtm);
		DrawCenterMark(DrawLineProc(gw),MakeBoxNotEmpty(*mc->box));
		}

	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int ClustMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flags, ModContext *mc)
	{
	// Transform the gizmo with the node.
	#ifdef DESIGN_VER
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 obtm = inode->GetObjectTM(rt);
	#else
	Matrix3 obtm = inode->GetObjectTM(t);
    #endif

	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(obtm);

	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	//Matrix3 tm = DEFORMER_TM;
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);	

	ClustDeformer deformer(tm);	
	if (ip && ip->GetSubObjectLevel() == 1) {
		//gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);		
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}
	if (mc->box->pmin==mc->box->pmax) {
		Point3 pt = mc->box->pmin * tm;
		gw->marker(&pt,ASTERISK_MRKR);		
	} else {
		DoModifiedBox(MakeBoxNotEmpty(*mc->box),deformer,DrawLineProc(gw));
		}

	//obtm = ctm * obtm;
	if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
	else obtm = ctm * obtm;
	
	gw->setTransform(obtm);
	if ( ip && (ip->GetSubObjectLevel() == 1 ||
	            ip->GetSubObjectLevel() == 2) ) {		
		//gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	DrawCenterMark(DrawLineProc(gw),MakeBoxNotEmpty(*mc->box));	
	return 0;
	}

void ClustMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{

	// Need the correct bound box for proper damage rect calcs.
	#ifdef DESIGN_VER
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 obtm = inode->GetObjectTM(rt);
	#else
	Matrix3 obtm = inode->GetObjectTM(t);
    #endif
	GraphicsWindow *gw = vpt->getGW();
	
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	//Matrix3 tm = DEFORMER_TM;
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);
	ClustDeformer deformer(tm);	
	
	BoxLineProc bp1(&obtm);
	DoModifiedBox(MakeBoxNotEmpty(*mc->box), deformer, bp1);
	box = bp1.Box();

	//obtm = ctm * obtm;
	if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
	else obtm = ctm * obtm;

	BoxLineProc bp2(&obtm);		
	DrawCenterMark(bp2,MakeBoxNotEmpty(*mc->box));
	box += bp2.Box();
	}

void ClustMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 obtm = node->GetObjectTM(t);
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);	
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);

	if (cb->Type()==SO_CENTER_PIVOT) {
		//Matrix3 mat = ctm * obtm;
		Matrix3 mat;		
		if (mc->tm) mat = ctm * Inverse(*mc->tm) * obtm;
		else mat = ctm * obtm;
		cb->Center(mat.GetTrans(),0);
	} else {		
		//Matrix3 tm = DEFORMER_TM;
		Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);
		ClustDeformer deformer(tm);
		BoxLineProc bp1(&obtm);
		DoModifiedBox(MakeBoxNotEmpty(*mc->box), deformer, bp1);
		cb->Center(bp1.Box().Center(),0);
		}
	}

void ClustMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 obtm = node->GetObjectTM(t);
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	//Matrix3 tm = DEFORMER_TM * obtm;	
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1) * obtm;
	cb->TM(tm,0);
	}

void ClustMod::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{
#ifdef DESIGN_VER
	t=0;
#endif
	if (tmControl==NULL) {
		MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}

	if (ip && ip->GetSubObjectLevel()==1) {				
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {		
		if (posControl==NULL) {
			MakeRefByID(FOREVER,1,NewDefaultPositionController()); 
			NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
			}
		Matrix3 ptm = partm;
		Interval valid;
		if (tmControl)
			tmControl->GetValue(t,&ptm,valid,CTRL_RELATIVE);
		posControl->SetValue(t,-VectorTransform(tmAxis*Inverse(ptm),val),TRUE,CTRL_RELATIVE);
		
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}


#endif // NO_MODIFIER_XFORM 