/**********************************************************************
 *<
	FILE: gizmo.cpp

	DESCRIPTION: An apparatus object

	CREATED BY: Rolf Berteig

	HISTORY: 4-15-96

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/

//#include "inferno.h"

#include "core.h"
#include "iparamb.h"
#include <plugapi.h>
#include "custcont.h"
#include "coremain.h"
#include "resource.h"
#include "render.h"
#include "plugin.h"
#include "mouseman.h"

#include "gizmo.h"

IParamMap *GizmoObject::pmapParam = NULL;
IObjParam *GizmoObject::ip        = NULL;

GizmoObject *GizmoObject::editOb = NULL;


GizmoObject::GizmoObject()
	{	
	pblock = NULL;
	}

GizmoObject::~GizmoObject()
	{
	DeleteAllRefsFromMe();
	}


void GizmoObject::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	editOb = this;
	}

void GizmoObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	editOb = NULL;
	}

int GizmoObject::HitTest(
		TimeValue t, INode* inode, int type, 
		int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt)
	{
	HitRegion hitRegion;
	DWORD	savedLimits;
	GraphicsWindow *gw = vpt->getGW();	
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	gw->setTransform(inode->GetObjectTM(t));
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) 
		| GW_PICK) & ~GW_ILLUM);
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();	
	DrawGizmo(t,gw);
	int res = gw->checkHitCode();
	gw->setRndLimits(savedLimits);
	return res;
	}

int GizmoObject::Display(
		TimeValue t, INode* inode, 
		ViewExp *vpt, int flags)
	{
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 mat = inode->GetObjectTM(t);
	Point3 color = WireColor();
	gw->setTransform(mat);	
	if (inode->Selected()) {
		gw->setColor(LINE_COLOR,GetSelColor());
	} else if(!inode->IsFrozen()) {
		gw->setColor(LINE_COLOR,color.x,color.y,color.z);
		}
		
	DrawGizmo(t,gw);
	return 0;
	}

int GizmoObject::CanConvertToType(Class_ID obtype)
	{
	return obtype==ClassID();
	}

Object* GizmoObject::ConvertToType(
		TimeValue t, Class_ID obtype)
	{
	if (obtype==ClassID()) return this;
	else return NULL;
	}

void GizmoObject::GetWorldBoundBox(
		TimeValue t, INode* inode, 
		ViewExp* vpt, Box3& box )
	{
	Matrix3 mat = inode->GetObjectTM(t);
	box.Init();
	GetBoundBox(mat,t,box);
	}

void GizmoObject::GetLocalBoundBox(
		TimeValue t, INode* inode, 
		ViewExp* vpt, Box3& box )
	{
	Matrix3 mat(1);
	box.Init();
	GetBoundBox(mat,t,box);
	}

void GizmoObject::GetDeformBBox(
		TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
	{
	Matrix3 mat(1);
	if (tm) mat = *tm;
	box.Init();
	GetBoundBox(mat,t,box);
	}


RefResult GizmoObject::NotifyRefChanged(
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:			
			if (editOb==this) InvalidateUI();
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
	return REF_SUCCEED;
	}

