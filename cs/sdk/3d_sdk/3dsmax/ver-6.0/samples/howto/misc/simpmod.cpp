/**********************************************************************
 *<
	FILE: simpmod.cpp

	DESCRIPTION:  Simple modifier base class

	CREATED BY: Dan Silva & Rolf Berteig

	HISTORY: created 30 Jauary, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "core.h"
#include "control.h"
#include "mouseman.h"
#include "paramblk.h"
#include "coremain.h"
#include "custcont.h"
#include "triobj.h"
#include "IParamM.h"
#include "objmode.h"
#include "simpmod.h"
#include "simpobj.h"
#include "buildver.h"
#include "resource.h"
#include "iparamb2.h"
#include "macrorec.h"
#include "MaxIcon.h"

//--- SimpleMod -----------------------------------------------------------

TCHAR *GetString(int id);

extern HINSTANCE hInstance;

class MaxIcon;

IObjParam*          SimpleMod::ip          = NULL;
MoveModBoxCMode*    SimpleMod::moveMode    = NULL;
RotateModBoxCMode*  SimpleMod::rotMode 	   = NULL;
UScaleModBoxCMode*  SimpleMod::uscaleMode  = NULL;
NUScaleModBoxCMode* SimpleMod::nuscaleMode = NULL;
SquashModBoxCMode*  SimpleMod::squashMode  = NULL;
SimpleMod*          SimpleMod::editMod     = NULL;
	
static GenSubObjType SOT_Aparatus(14);
static GenSubObjType SOT_Center(15);

int SimpleMod::NumSubObjTypes() 
{ 
	return 2;
}

ISubObjType *SimpleMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Aparatus.SetName(GetResString(IDS_RB_APPARATUS));
		SOT_Center.SetName(GetResString(IDS_RB_CENTER));
	}

	if(i == -1)
	{
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
	}
	
	if(i == 0) 
		return &SOT_Aparatus;
	if(i == 1)
		return &SOT_Center;
	
	return NULL;
}


SimpleMod::SimpleMod()
	{
	tmControl  = NULL; 
	posControl = NULL;	
	pblock     = NULL;
	MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
	MakeRefByID(FOREVER,1,NewDefaultPositionController()); 
	}

SimpleMod::~SimpleMod()
	{	
	}

IParamArray *SimpleMod::GetParamBlock()
	{
	return pblock;
	}

int SimpleMod::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

RefTargetHandle SimpleMod::GetReference(int i) 
	{ 
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock;
		default: return NULL;
		}
	}

void SimpleMod::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock = (IParamBlock*)rtarg; break;
		}
	}

Animatable* SimpleMod::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock;		
		default: return NULL;
		}
	}

TSTR SimpleMod::SubAnimName(int i) 
	{ 
	switch (i) {
		case 0: return TSTR(GetResString(IDS_RB_CENTER));
		case 1: return TSTR(GetResString(IDS_RB_APPARATUS));
		case 2: return TSTR(GetResString(IDS_RB_PARAMETERS));
		default: return TSTR(_T(""));
		}	
	}

int SimpleMod::SubNumToRefNum(int subNum)
	{
	switch (subNum) {
		case 0: return 1;
		case 1: return 0;
		default: return -1;
		}
	}

BOOL SimpleMod::AssignController(Animatable *control,int subAnim)
	{
	ReplaceReference(SubNumToRefNum(subAnim),(ReferenceTarget*)control);
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	return TRUE;
	}

Interval SimpleMod::LocalValidity(TimeValue t)
	{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	Interval valid = GetValidity(t);	
	if ( tmControl ) {
		Matrix3 mat;	
		mat.IdentityMatrix();		
		tmControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
		}
	if ( posControl ) {
		Matrix3 mat;	
		mat.IdentityMatrix();
		posControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
		}
	return valid;
	}


void SimpleMod::SimpleModClone( SimpleMod *smodSource )
	{
	if ( smodSource->tmControl ) {		
		MakeRefByID(FOREVER,0,smodSource->tmControl->Clone()); 
		}
	if ( smodSource->posControl ) {		
		MakeRefByID(FOREVER,1,smodSource->posControl->Clone()); 
		}
	}


//  Move, Rotate, and Scale

void SimpleMod::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
#ifdef DESIGN_VER
	t=0;
#endif
	if ( ip && ip->GetSubObjectLevel()==1 ) {
		if (tmControl==NULL) {
			MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
			NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
			}		
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
		macroRecorder->OpAssign(_T("+="), mr_prop, _T("gizmo.pos"), mr_reftarg, this, mr_point3, &val);
	} else {		
		if (posControl==NULL) {
			MakeRefByID(FOREVER,1,NewDefaultPositionController()); 
			NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
			}
		Matrix3 ptm = partm;
		Interval valid;
		if ( tmControl )
			tmControl->GetValue(t,&ptm,valid,CTRL_RELATIVE);
		posControl->SetValue(t,VectorTransform(tmAxis*Inverse(ptm),val),TRUE,CTRL_RELATIVE);
		macroRecorder->OpAssign(_T("+="), mr_prop, _T("center"), mr_reftarg, this, mr_point3, &val);
		}
	}


void SimpleMod::Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin ) {
#ifdef DESIGN_VER
	t=0;
#endif
	if (tmControl==NULL) {
		MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}	
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	macroRecorder->OpAssign(_T("+="), mr_prop, _T("gizmo.rotation"), mr_reftarg, this, mr_quat, &val);
	}

void SimpleMod::Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
#ifdef DESIGN_VER
	t=0;
#endif
	if (tmControl==NULL) {
		MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}		
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	macroRecorder->OpAssign(_T("*="), mr_prop, _T("gizmo.scale"), mr_reftarg, this, mr_point3, &val);
	}



Matrix3 SimpleMod::CompMatrix(TimeValue t, ModContext& mc, Matrix3& ntm, 
		Interval& valid, BOOL needOffset) {
	Matrix3 tm;
	
	if (mc.tm) {
		tm = *mc.tm;		
		}
	else 
		tm.IdentityMatrix();	
	if (tmControl) { 
		Matrix3 mat;
		mat.IdentityMatrix();
		tmControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
		tm = tm*Inverse(mat);		
		}
	if (posControl && needOffset) {
		Matrix3 mat;
		mat.IdentityMatrix();		
		posControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
		tm = tm*Inverse(mat);		
		}
	
	return Inverse(tm)*ntm;	
	}

void SimpleMod::CompOffset( TimeValue t, Matrix3& offset, Matrix3& invoffset)
	{	
	Interval valid;
	offset.IdentityMatrix();
	invoffset.IdentityMatrix();
	if ( posControl ) {				
		posControl->GetValue(t,&offset,valid,CTRL_RELATIVE);
		invoffset = Inverse(offset);
		}
	}

void SimpleMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{	
	Interval valid = GetValidity(t);
	Matrix3 modmat,minv;
	
	// These are inverted becuase that's what is usually needed for displaying/hit testing
	minv   = CompMatrix(t,mc,idTM,valid,TRUE);
	modmat = Inverse(minv);
	
	os->obj->Deform(&GetDeformer(t,mc,modmat, minv), TRUE /*TestAFlag(A_MOD_USE_SEL)*/);   //DS
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
	}


int SimpleMod::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits;	
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(t);
	float zmin, zmax;
	int axis;

	if (mc->box->IsEmpty()) return 0;

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if (ip && ip->GetSubObjectLevel() == 1) {
		Matrix3 off, invoff;
		modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
		CompOffset(t,off,invoff);
		gw->setTransform(modmat);		
		if (mc->box->pmin==mc->box->pmax) {
			gw->marker(&mc->box->pmin,ASTERISK_MRKR);
		} else {
			DoModifiedBox(
				*mc->box,GetDeformer(t,*mc,invoff,off),
				DrawLineProc(gw));
			}
		}

	if (ip && ip->GetSubObjectLevel() == 1 ||
	          ip->GetSubObjectLevel() == 2 ) {
		modmat = CompMatrix(t,*mc,ntm,valid,TRUE);
		gw->setTransform(modmat);
		DrawCenterMark(DrawLineProc(gw),*mc->box);
		if (GetModLimits(t,zmin,zmax,axis)) {
			Matrix3 id(1);
			DoModifiedLimit(*mc->box,zmin,axis,GetDeformer(t,*mc,id,id),DrawLineProc(gw));
			DoModifiedLimit(*mc->box,zmax,axis,GetDeformer(t,*mc,id,id),DrawLineProc(gw));
			}
		}

	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int SimpleMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	Interval valid;
	GraphicsWindow *gw = vpt->getGW();
#ifdef DESIGN_VER
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(rt), off, invoff;
#else
	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(t), off, invoff;
#endif
	float zmin, zmax;
	int axis;

	if (mc->box->IsEmpty()) return 0;

	modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
	CompOffset(t,off,invoff);
	gw->setTransform(modmat);
	if ( ip && ip->GetSubObjectLevel() == 1 ) {		
		//gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	if (mc->box->pmin==mc->box->pmax) {
		gw->marker(&mc->box->pmin,ASTERISK_MRKR);
	} else {
		DoModifiedBox(
			*mc->box,GetDeformer(t,*mc,invoff,off),
			DrawLineProc(gw));
		}
	
	modmat = CompMatrix(t,*mc,ntm,valid,TRUE);
	gw->setTransform(modmat);
	if ( ip && (ip->GetSubObjectLevel() == 1 ||
	            ip->GetSubObjectLevel() == 2) ) {		
		gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
	} else {
		gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		}	
	DrawCenterMark(DrawLineProc(gw),*mc->box);	
	if (GetModLimits(t,zmin,zmax,axis)) {
		Matrix3 id(1);
		DoModifiedLimit(*mc->box,zmin,axis,GetDeformer(t,*mc,id,id),DrawLineProc(gw));
		DoModifiedLimit(*mc->box,zmax,axis,GetDeformer(t,*mc,id,id),DrawLineProc(gw));
		}

	return 0;	
	}

void SimpleMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	Interval valid;
	GraphicsWindow *gw = vpt->getGW();
#ifdef DESIGN_VER
	// Needed for damage rectangle calcs
		TimeValue rt = GetCOREInterface()->GetTime();
		Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(rt), off, invoff;
#else
	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(t), off, invoff;
#endif
	float zmin, zmax;
	int axis;

	if (mc->box->IsEmpty()) return;

	modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
	CompOffset(t,off,invoff);	
	BoxLineProc bp1(&modmat);	
	DoModifiedBox(*mc->box, GetDeformer(t,*mc,invoff,off), bp1);
	box = bp1.Box();

	modmat = CompMatrix(t,*mc,ntm,valid,TRUE);	
	BoxLineProc bp2(&modmat);		
	DrawCenterMark(bp2,*mc->box);
	if (GetModLimits(t,zmin,zmax,axis)) {
		Matrix3 id(1);
		DoModifiedLimit(*mc->box,zmin,axis,GetDeformer(t,*mc,id,id),bp2);
		DoModifiedLimit(*mc->box,zmax,axis,GetDeformer(t,*mc,id,id),bp2);
		}
	box += bp2.Box();
	}

void SimpleMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 modmat, tm, ntm = node->GetObjTMBeforeWSM(t,&valid), off, invoff;

	if (cb->Type()==SO_CENTER_PIVOT) {
		tm = CompMatrix(t,*mc,ntm,valid,TRUE);
		cb->Center(tm.GetTrans(),0);
	} else {
		modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
		CompOffset(t,off,invoff);
		BoxLineProc bp1(&modmat);	
		DoModifiedBox(*mc->box, GetDeformer(t,*mc,invoff,off), bp1);
		cb->Center(bp1.Box().Center(),0);
		}
	}

void SimpleMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 ntm = node->GetObjTMBeforeWSM(t,&valid);
	Matrix3 tm = CompMatrix(t,*mc,ntm,valid,TRUE);
	cb->TM(tm,0);
	}


void SimpleMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;
	editMod  = this;

	// Add our sub object type
	// TSTR type1( GetResString(IDS_RB_APPARATUS) );	
	// TSTR type2( GetResString(IDS_RB_CENTER) );	
	// const TCHAR *ptype[] = { type1, type2 };
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes( ptype, 2 );

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	rotMode     = new RotateModBoxCMode(this,ip);
	uscaleMode  = new UScaleModBoxCMode(this,ip);
	nuscaleMode = new NUScaleModBoxCMode(this,ip);
	squashMode  = new SquashModBoxCMode(this,ip);	
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
	}

void SimpleMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);	
	if ( moveMode ) delete moveMode;
	moveMode = NULL;
	if ( rotMode ) delete rotMode;
	rotMode = NULL;
	if ( uscaleMode ) delete uscaleMode;
	uscaleMode = NULL;
	if ( nuscaleMode ) delete nuscaleMode;
	nuscaleMode = NULL;
	if ( squashMode ) delete squashMode;
	squashMode = NULL;
	}

void SimpleMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	switch ( level ) {
		case 1: // Modifier box
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;
		case 2: // Modifier Center
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}

RefResult SimpleMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}
//--- SimpleMod2 -----------------------------------------------------------
// JBW 2/9/99

RefTargetHandle SimpleMod2::GetReference(int i) 
	{ 
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock2;
		default: return NULL;
		}
	}

void SimpleMod2::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock2 = (IParamBlock2*)rtarg; break;
		}
	}

Animatable* SimpleMod2::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock2;		
		default: return NULL;
		}
	}

//--- SimpleWSMMod -----------------------------------------------------------


IObjParam*    SimpleWSMMod::ip			= NULL;
SimpleWSMMod* SimpleWSMMod::editMod	= NULL;


SimpleWSMMod::SimpleWSMMod()
	{	
	obRef   = NULL;
	nodeRef = NULL;
	pblock  = NULL;
	}

SimpleWSMMod::~SimpleWSMMod()
	{	
	}

IParamArray *SimpleWSMMod::GetParamBlock()
	{
	return pblock;
	}

int SimpleWSMMod::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

RefTargetHandle SimpleWSMMod::GetReference(int i) 
	{ 
	switch (i) {		
		case SIMPWSMMOD_OBREF: 		return obRef;
		case SIMPWSMMOD_NODEREF:	return nodeRef;
		case SIMPWSMMOD_PBLOCKREF: 	return pblock;
		default: return NULL;
		}
	}

void SimpleWSMMod::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case SIMPWSMMOD_OBREF:		obRef   = (WSMObject*)rtarg; break;
		case SIMPWSMMOD_NODEREF:	nodeRef = (INode*)rtarg; break;
		case SIMPWSMMOD_PBLOCKREF: 	pblock  = (IParamBlock*)rtarg; break;
		}
	}

Animatable* SimpleWSMMod::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return pblock;		
		default: return NULL;
		}
	}

TSTR SimpleWSMMod::SubAnimName(int i) 
	{ 
	switch (i) {
		case 0: return TSTR(GetResString(IDS_RB_PARAMETERS));
		default: return TSTR(_T(""));
		}	
	}

Interval SimpleWSMMod::LocalValidity(TimeValue t)
	{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	return GetValidity(t);	
	}

CoreExport WSMObject *SimpleWSMMod::GetWSMObject(TimeValue t)
	{
	if (nodeRef) {
		ObjectState os = nodeRef->EvalWorldState(t);
		//assert(os.obj && os.obj->SuperClassID()==WSM_OBJECT_CLASS_ID);
		if (os.obj && os.obj->SuperClassID()==WSM_OBJECT_CLASS_ID)
			 return (WSMObject*)os.obj;
		else return NULL;
	} else {
		return NULL;
		}
	}

void SimpleWSMMod::SimpleWSMModClone(SimpleWSMMod *smodSource)
	{	
	if (smodSource->pblock) ReplaceReference(SIMPWSMMOD_PBLOCKREF,smodSource->pblock->Clone());
	ReplaceReference(SIMPWSMMOD_OBREF,smodSource->obRef);
	ReplaceReference(SIMPWSMMOD_NODEREF,smodSource->nodeRef);
	}


void SimpleWSMMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{	
	Interval valid = GetValidity(t);
	Matrix3 modmat(1);
	Matrix3 minv(1);
	
	os->obj->Deform(&GetDeformer(t,mc,modmat, minv), TRUE);
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
	}




void SimpleWSMMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;	
	editMod  = this;

#if 0
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
#endif
	}

void SimpleWSMMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod  = NULL;

#if 0	
	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);	
#endif
	}


RefResult SimpleWSMMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this) InvalidateUI();
			break;

		case REFMSG_TARGET_DELETED:
			// THis means the WSM node is being deleted. As a result,
			// we must delete ourselves. 
			DeleteMe();  // also deletes all refs and 
						 // sends REFMSG_TARGET_DELETED to all Dependents
			return REF_STOP;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

SvGraphNodeReference SimpleWSMMod::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
	{
	SvGraphNodeReference nodeRef;
	SvGraphNodeReference childNodeRef;

//	gom->PushLevel(this);

	nodeRef = gom->AddAnimatable(this, owner, id, flags);
	if (nodeRef.stat == SVT_PROCEED)
		{
		if (GetReference(SIMPWSMMOD_NODEREF))
			{
			gom->PushLevel(GetReference(SIMPWSMMOD_NODEREF));
			childNodeRef = gom->AddAnimatable(GetReference(SIMPWSMMOD_NODEREF), this, SIMPWSMMOD_NODEREF, flags);
			if (childNodeRef.stat != SVT_DO_NOT_PROCEED)
				gom->AddReference(nodeRef.gNode, childNodeRef.gNode, REFTYPE_SUBANIM);
			gom->PopLevel();
			}
		if (GetReference(SIMPWSMMOD_PBLOCKREF))
			{
			childNodeRef = GetReference(SIMPWSMMOD_PBLOCKREF)->SvTraverseAnimGraph(gom, this, SIMPWSMMOD_PBLOCKREF, flags);
			if (childNodeRef.stat != SVT_DO_NOT_PROCEED)
				gom->AddReference(nodeRef.gNode, childNodeRef.gNode, REFTYPE_SUBANIM);
			}
		}

//	gom->PopLevel();

	return nodeRef;
	}

//---SimpleOSMToWSMMod--------------------------------------------------

SimpleOSMToWSMMod::SimpleOSMToWSMMod()
	{
	obRef   = NULL;
	nodeRef = NULL;
	pblock  = NULL;
	}

SimpleOSMToWSMMod::SimpleOSMToWSMMod(INode *node)
	{
	obRef   = NULL;
	nodeRef = NULL;
	pblock  = NULL;
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	}

RefTargetHandle SimpleOSMToWSMMod::Clone(RemapDir& remap)
	{
	SimpleOSMToWSMMod *mod = new SimpleOSMToWSMMod(nodeRef);
	mod->SimpleWSMModClone(this);
	BaseClone(this, mod, remap);
	return mod;
	}

TCHAR *SimpleOSMToWSMMod::GetObjectName()
	{
	SimpleOSMToWSMObject *obj = (SimpleOSMToWSMObject*)GetWSMObject(0);
	static TSTR name;	
	if(obj)
		name = TSTR(obj->mod->GetObjectName()) + TSTR(GetString(IDS_RB_BBINDING));
	return name;
	}

Deformer& SimpleOSMToWSMMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	SimpleOSMToWSMObject *obj = (SimpleOSMToWSMObject*)GetWSMObject(t);
	if (!mc.box) mc.box = new Box3;
	float l, w, h;
	Interval ivalid = FOREVER;
	obj->pblock->GetValue(PB_OSMTOWSM_LENGTH,t,l,ivalid);
	obj->pblock->GetValue(PB_OSMTOWSM_WIDTH,t,w,ivalid);
	obj->pblock->GetValue(PB_OSMTOWSM_HEIGHT,t,h,ivalid);
	w = w/2.0f; l = l/2.0f;
	mc.box->pmin = Point3(-w,-l,0.0f);
	mc.box->pmax = Point3( w, l,h);
	//invmat = nodeRef->GetObjTMBeforeWSM(t,&ivalid);
	invmat = nodeRef->GetObjTMAfterWSM(t,&ivalid);
	mat = Inverse(invmat);
	return obj->GetDecayDeformer(t,obj->mod->GetDeformer(t,mc,mat,invmat),invmat.GetTrans(),FOREVER);
	}

Interval SimpleOSMToWSMMod::GetValidity(TimeValue t)
	{
	Interval iv = FOREVER;
	SimpleOSMToWSMObject *obj = (SimpleOSMToWSMObject*)GetWSMObject(t);
	iv &= obj->mod->GetValidity(t);
	float l, w, h, d;	
	obj->pblock->GetValue(PB_OSMTOWSM_LENGTH,t,l,iv);
	obj->pblock->GetValue(PB_OSMTOWSM_WIDTH,t,w,iv);
	obj->pblock->GetValue(PB_OSMTOWSM_HEIGHT,t,h,iv);
	obj->pblock->GetValue(PB_OSMTOWSM_DECAY,t,d,iv);
	//nodeRef->GetObjTMBeforeWSM(t,&iv);
	nodeRef->GetObjTMAfterWSM(t,&iv);
	return iv;
	}

class SimpleOSMToWSMModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) {return new SimpleOSMToWSMMod;}
	const TCHAR *	ClassName() {return _T("SimpleOSMToWSMMod");}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return SIMPLEOSMTOWSM_CLASSID;}
	const TCHAR* 	Category() {return _T("");}
	};

static SimpleOSMToWSMModClassDesc osmTowsmDesc;
ClassDesc* GetSimpleOSMToWSMModDesc() {return &osmTowsmDesc;}
