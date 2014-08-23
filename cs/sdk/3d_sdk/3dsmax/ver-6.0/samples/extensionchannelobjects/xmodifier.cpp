/**********************************************************************
 *<
	FILE: XModifier.cpp

	DESCRIPTION:	Sample modifier, that adds an extension channel to the pipeline

	CREATED BY:		Nikolai Sander
	
	HISTORY:		Created: 3/22/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "XModifier.h"
#include "Modstack.h"
#include "XGSphere.h"

#define MODPBLOCK_REF	0

class XModifier;


static XModifierClassDesc XModifierDesc;
ClassDesc2* GetXModifierDesc() {return &XModifierDesc;}

static ParamBlockDesc2 xmodifier_param_blk ( x_params, _T("params"),  0, &XModifierDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, MODPBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_fn_spin, 			_T("FN spin"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_FN_SPIN, 
		p_default, 		0.1f, 
		p_range, 		0.0f,1000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_FN_EDIT,	IDC_FN_SPIN, 0.01f, 
		end,
	pb_fa_spin, 			_T("Face_Area_Ratio"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_FA_SPIN, 
		p_default, 		1.85f, 
		p_range, 		0.00001f,100000000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_FA_EDIT,	IDC_FA_SPIN, 0.01f, 
		end,
	pb_nf_spin, 			_T("Number_of_Faces"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_NF_SPIN,
		p_default, 		300, 
		p_range, 		1,100000,
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_NF_EDIT,	IDC_NF_SPIN, SPIN_AUTOSCALE, 
		end,
	pb_suspdisp, 	_T("suspdisp"), 		TYPE_BOOL, 		P_ANIMATABLE,				IDS_SUSP_DISP,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_SUSP_DISP, 
		end, 
	pb_fn_onoff, 	_T("facenormalonoff"), 		TYPE_BOOL, 		P_ANIMATABLE,				IDS_FN_ONOFF,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_FN_ONOFF, 
		end, 
	pb_nf_onoff, 	_T("numberoffacesonoff"), 		TYPE_BOOL, 		P_ANIMATABLE,				IDS_NF_ONOFF,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_NF_ONOFF, 
		end, 
	pb_fa_onoff, 	_T("faceareaonoff"), 		TYPE_BOOL, 		P_ANIMATABLE,				IDS_FA_ONOFF,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_FA_ONOFF, 
		end, 
	end
	);



IObjParam *XModifier::ip			= NULL;

// ----------------------------------------------------------------------------------------------------------
// XTCSample Implementation
// ----------------------------------------------------------------------------------------------------------

XTCSample::XTCSample(BaseObject *BaseObj, float size, BOOL suspDisp, BOOL fn_OnOff, BOOL nf_OnOff, BOOL fa_OnOff) 
: bo(BaseObj), size(size), bSuspDisp(suspDisp), bFN_OnOff(fn_OnOff), bNF_OnOff(nf_OnOff), bFA_OnOff(fa_OnOff) {};

XTCSample::XTCSample(XTCSample *mFrom)
{
	bo = mFrom->bo;
	size = mFrom->size;
	bSuspDisp = mFrom->bSuspDisp;
	
	bFN_OnOff = mFrom->bFN_OnOff;
	bNF_OnOff = mFrom->bNF_OnOff;
	bFA_OnOff = mFrom->bFA_OnOff;
}

XTCSample::~XTCSample()
{
	size = 0;
}
// This method will be called before a modifier in the pipleine changes any channels that this XTCObject depends on
// (See DependsOn() )

void XTCSample::PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline)
{
		
}

// This method will be called after a modifier in the pipleine changed any channels that this XTCObject depends on
// (See DependsOn() )

void XTCSample::PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline)
{
	DeleteFaces(t,os->obj);
}

int XTCSample::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj)
{
	
	if(pObj->ClassID() == XGSPHERE_CLASS_ID || pObj->IsSubClassOf(triObjectClassID))
	{
		return DisplayMesh(t, inode, vpt, flags, GetMesh(pObj));
	}
#ifndef NO_PATCHES
	else if( pObj->IsSubClassOf(patchObjectClassID) )
	{
		return DisplayPatch(t, inode, vpt, flags, (PatchObject *) pObj);
	}
#endif
	else if(pObj->CanConvertToType(triObjectClassID))
	{
		TriObject *pTri = (TriObject *) pObj->ConvertToType(t,triObjectClassID);
		DisplayMesh(t, inode, vpt, flags, &pTri->mesh);
		if(pTri != pObj)
			pTri->DeleteThis();
		
	}
	
	return 0;
}

int  XTCSample::DisplayMesh(TimeValue t, INode* inode, ViewExp *vpt, int flags, Mesh *mesh)
{
	if(!bFN_OnOff && !bFA_OnOff)
		return 0;

	if(!mesh)
		return 0;

	Interval ivalid = FOREVER;

	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(inode->GetObjectTM(t));
	DWORD oldMode = gw->getRndLimits();
	gw->setRndLimits(oldMode|GW_ILLUM);

	if(bFN_OnOff)
	{
		mesh->checkNormals(TRUE);	
		Point3 pt[3];
		Point3 rgb[3];
		int edges[3] = {GW_EDGE_VIS,GW_EDGE_VIS,GW_EDGE_VIS};
		gw->setColor(LINE_COLOR, (float)0.0, (float)0.0, (float)0.8);
		rgb[0] = Point3((float)0.0, (float)0.0, (float)0.8);
		rgb[1] = Point3((float)1.0, (float)1.0, (float)1.0);
		for(int i = 0; i < mesh->getNumFaces(); i++) {
			pt[0] = (mesh->getVert(mesh->faces[i].v[0]) + mesh->getVert(mesh->faces[i].v[1]) + mesh->getVert(mesh->faces[i].v[2]))/3.0f;
			pt[1] = pt[0] + mesh->getFaceNormal(i) * size;
			gw->polyline(2, pt,rgb, NULL, 0, edges);
		}
	}
	
	if(bFA_OnOff)
	{
		float faceArea;
		bo->GetParamBlockByID(x_params)->GetValue(pb_fa_spin,t,faceArea, ivalid);
		BitArray ba;
		ba.SetSize(mesh->getNumFaces());
		Point3 vtx[4];
		Point3 col[4];
		
		for(int i = 0 ; i < mesh->getNumFaces() ; i++ )
		{
			Point3 va = mesh->verts[mesh->faces[i].v[1]]-mesh->verts[mesh->faces[i].v[0]];
			Point3 vb = mesh->verts[mesh->faces[i].v[2]]-mesh->verts[mesh->faces[i].v[1]];
			Point3 vc = mesh->verts[mesh->faces[i].v[0]]-mesh->verts[mesh->faces[i].v[2]];

			float CosAngleC = DotProd(va,vb)/(Length(va)*Length(vb));
			float area = 0.5f*Length(va)*Length(vb)*sin(acos(CosAngleC));;
			float edgeLength = Length(va)+Length(vb)+Length(vc);

			if(area/(edgeLength*edgeLength) < faceArea/100.0f)
			{
				vtx[0] = mesh->verts[mesh->faces[i].v[0]];
				vtx[1] = mesh->verts[mesh->faces[i].v[1]];
				vtx[2] = mesh->verts[mesh->faces[i].v[2]];
				col[0] = col[1] = col[2] = col[3] = Point3(1,0,0);
				gw->polygon(3, vtx, col, NULL);
			}		
		}
	}

	gw->setRndLimits(oldMode);

	return 0;		
}

#ifndef NO_PATCHES
int  XTCSample::DisplayPatch(TimeValue t, INode* inode, ViewExp *vpt, int flags, PatchObject *patch)
{
	patch->UpdatePatchMesh(t);
	Mesh& mesh = patch->GetMesh(t);
	DisplayMesh(t, inode, vpt, flags, &mesh);
	
	return 0;
}
#endif

void XTCSample::MaybeEnlargeViewportRect(GraphicsWindow *gw, Rect &rect)
{
	Interval ivalid = FOREVER;
	int i = (int) size*5;
	rect.top -= i;
	rect.bottom += i;
	rect.left -= i;
	rect.right += i;
}

BOOL XTCSample::SuspendObjectDisplay() {
	return bSuspDisp;
}


Mesh *XTCSample::GetMesh(Object *obj)
{
	Mesh *mesh = NULL;
	
	if(obj->ClassID() == XGSPHERE_CLASS_ID)
		mesh = &((SimpleObject2 *) obj)->mesh;
	else
	{
		if (!obj->IsSubClassOf(triObjectClassID)) 
			return NULL;
		else
			mesh = &((TriObject*)obj)->mesh;
	}
	return mesh;
}

void XTCSample::DeleteFaces(TimeValue t,Object *obj)
{
	if(bNF_OnOff)
	{
		Mesh *mesh = GetMesh(obj);
		
		if(!mesh)
			return;
		
		Interval ivalid = FOREVER;
		int nf;
		bo->GetParamBlockByID(x_params)->GetValue(pb_nf_spin,t,nf, ivalid);
		BitArray ba;
		ba.SetSize(mesh->getNumFaces());
		ba.ClearAll();
		
		for(int i = nf ; i < mesh->getNumFaces() ; i++ )
		{
			ba.Set(i);
		}
		
		if(!ba.IsEmpty())
			mesh->DeleteFaceSet(ba);
	}
}

// ----------------------------------------------------------------------------------------------------------
// XModifier implementation
// ----------------------------------------------------------------------------------------------------------

XModifier::XModifier()
{
	XModifierDesc.MakeAutoParamBlocks(this);
	bModDisabled = false;
}

XModifier::~XModifier()
{
}

Interval XModifier::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	else {		
		Interval ivalid = FOREVER;
		float f;
		BOOL suspDisp;
		pblock->GetValue(pb_fn_spin,t,f, ivalid);
		pblock->GetValue(pb_suspdisp,t,suspDisp, ivalid);
		pblock->GetValue(pb_fn_onoff,t,bFN_OnOff, ivalid);
		pblock->GetValue(pb_nf_onoff,t,bNF_OnOff, ivalid);
		pblock->GetValue(pb_fa_onoff,t,bFA_OnOff, ivalid);
		return ivalid;
	}
}

RefTargetHandle XModifier::Clone(RemapDir& remap)
{
	XModifier* newmod = new XModifier();	
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}

void XModifier::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	Interval ivalid = FOREVER;
	float f;
	BOOL suspDisp;
	pblock->GetValue(pb_fn_spin,t,f, ivalid);
	pblock->GetValue(pb_suspdisp,t,suspDisp, ivalid);
	pblock->GetValue(pb_fn_onoff,t,bFN_OnOff, ivalid);
	pblock->GetValue(pb_nf_onoff,t,bNF_OnOff, ivalid);
	pblock->GetValue(pb_fa_onoff,t,bFA_OnOff, ivalid);
	
	XTCSample *pObj = NULL;

	if(bModDisabled)
		pObj = new XTCSample(this,f,suspDisp,bFN_OnOff,FALSE,bFA_OnOff);
	else
		pObj = new XTCSample(this,f,suspDisp,bFN_OnOff,bNF_OnOff,bFA_OnOff);

	os->obj->AddXTCObject(pObj);
	
	teststruct *ts = new teststruct;
	ts->a = 0;
	ts->b = 1;
	testtab.Append(1,&ts);
	
	os->obj->SetChannelValidity(EXTENSION_CHAN_NUM, GetValidity(t));
}

void XModifier::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	XModifierDesc.BeginEditParams(ip, this, flags, prev);
	
	// aszabo|feb.05.02 
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	//NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
}

void XModifier::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	//NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	XModifierDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
}


//From ReferenceMaker 
RefResult XModifier::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

//From Object
BOOL XModifier::HasUVW() 
{ 
	return TRUE; 
}

void XModifier::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
}

IOResult XModifier::Load(ILoad *iload)
{
	return IO_OK;
}

IOResult XModifier::Save(ISave *isave)
{	
	return IO_OK;
}

Interval XModifier::GetValidity(TimeValue t) {
	Interval ivalid = FOREVER;
	float f;
	int i;
	BOOL suspDisp;
	pblock->GetValue(pb_fn_spin,t,f, ivalid);
	pblock->GetValue(pb_nf_spin,t,i, ivalid);
	pblock->GetValue(pb_fa_spin,t,f, ivalid);
	pblock->GetValue(pb_suspdisp,t,suspDisp, ivalid);
	pblock->GetValue(pb_fn_onoff,t,bFN_OnOff, ivalid);
	pblock->GetValue(pb_nf_onoff,t,bNF_OnOff, ivalid);
	pblock->GetValue(pb_fa_onoff,t,bFA_OnOff, ivalid);
	return ivalid;
}

void XModifier::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	bModDisabled = true;
	TimeValue t = GetCOREInterface()->GetTime();
	NotifyDependents(Interval(t,t),PART_ALL,REFMSG_CHANGE);
}

void XModifier::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	// We don't allow a stack collapse, to delete us. 
	// We're going to apply ourselves to the collapsed object

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	derob->AddModifier(this,NULL,derob->NumModifiers());
	bModDisabled = false;
}
