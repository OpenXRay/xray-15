/**********************************************************************
 *<
	FILE:			PFOperatorSimpleShape.cpp

	DESCRIPTION:	SimpleShape Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleShape.h"

#include "PFOperatorSimpleShape_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

PFOperatorSimpleShape::PFOperatorSimpleShape()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}


FPInterfaceDesc* PFOperatorSimpleShape::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleShape_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleShape_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleShape_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleShape::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLESHAPE_CLASS_NAME);
}

Class_ID PFOperatorSimpleShape::ClassID()
{
	return PFOperatorSimpleShape_Class_ID;
}

void PFOperatorSimpleShape::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleShape::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleShape::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	IParamMap2* map;

	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kSimpleShape_useScale)
					RefreshUI(kSimpleShape_message_useScale);
				if (lastUpdateID == kSimpleShape_shape)
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case kSimpleShape_RefMsg_InitDlg:
			map = (IParamMap2*)partID;
			RefreshUI(kSimpleShape_message_useScale, map);
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleShape::Clone(RemapDir &remap)
{
	PFOperatorSimpleShape* newOp = new PFOperatorSimpleShape();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleShape::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLESHAPE_OBJECT_NAME);
}

bool PFOperatorSimpleShape::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	BuildMesh(pblock()->GetInt(kSimpleShape_shape), pblock()->GetFloat(kSimpleShape_size, 0));
	return PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
}

const ParticleChannelMask& PFOperatorSimpleShape::ChannelsUsed(const Interval& time) const
{
								//  read						 &	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_Shape|PCU_Scale);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleShape::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESHAPE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleShape::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESHAPE_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleShape::HasDynamicName(TSTR& nameSuffix)
{
	int shape = pblock()->GetInt(kSimpleShape_shape, 0);
	int ids;
	switch(shape) {
	case kSimpleShape_shape_vertex:		ids = IDS_SHAPETYPE_VERTEX;		break;
	case kSimpleShape_shape_pyramid:	ids = IDS_SHAPETYPE_PYRAMID;	break;
	case kSimpleShape_shape_cube:		ids = IDS_SHAPETYPE_CUBE;		break;
	case kSimpleShape_shape_sphere:		ids = IDS_SHAPETYPE_SPHERE;		break;
	}
	nameSuffix = GetString(ids);
	return true;
}

bool PFOperatorSimpleShape::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int amount = chAmount->Count();
	if (amount == 0) return true; // no particles to modify or control
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container

	IParticleChannelMeshW* chMesh = NULL;
	IParticleChannelPoint3W* chScale = NULL;
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	chMesh = (IParticleChannelMeshW*)chCont->EnsureInterface(PARTICLECHANNELSHAPEW_INTERFACE,
																ParticleChannelMesh_Class_ID,
																true, PARTICLECHANNELSHAPER_INTERFACE,
																PARTICLECHANNELSHAPEW_INTERFACE, true);
	if (chMesh == NULL) return false; // can't modify Shape channel in the container

	bool useScale = (pblock()->GetInt(kSimpleShape_useScale, timeEnd) != 0);
	if (useScale) {
		chScale = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSCALER_INTERFACE,
																	PARTICLECHANNELSCALEW_INTERFACE, true);
		if (chScale == NULL) return false; // can't modify Scale channel in the container
	}

	if (chNew->IsAllOld()) return true;

	bool isScaleAnimated = false;
	float scaleAtZero = 1.0f;
	if (useScale) {
		Control* scaleCtrl = pblock()->GetController(kSimpleShape_scale);
		isScaleAnimated = (scaleCtrl == NULL) ? false : (scaleCtrl->IsAnimated() == TRUE);
		scaleAtZero = GetPFFloat(pblock(), kSimpleShape_scale, 0);
	}

	// modify shape for "new" particles
	if (chNew->IsAllNew()) {
		PreciseTimeValue time = chTime->GetValue(0);
		float size = GetPFFloat(pblock(), kSimpleShape_size, time);
		chMesh->SetValue(&_mesh());
	} else {
		Tab<int> newIndices;
		for (int i = 0; i < amount; i++)
		{
			if (!chNew->IsNew(i)) continue;
			newIndices.Append(1, &i, 1024);
		}
		if (newIndices.Count() > 0)
			chMesh->SetValue(newIndices, &_mesh());
	}

	// scale value
	if (useScale) {
		float curScale = scaleAtZero;
		Point3 curScaleXYZ = Point3(scaleAtZero,scaleAtZero,scaleAtZero);
		if (chNew->IsAllNew()) {
			if (isScaleAnimated) {
				for(int i=0; i<amount; i++) {
					curScale = GetPFFloat(pblock(), kSimpleShape_scale, (chTime->GetValue(i)).TimeValue());
					chScale->SetValue(i, Point3(curScale,curScale,curScale) );
				}
			} else {
				chScale->SetValue(curScaleXYZ);
			}
		} else {
			if (isScaleAnimated) {
				for(int i=0; i<amount; i++)
					if (chNew->IsNew(i)) {
						curScale = GetPFFloat(pblock(), kSimpleShape_scale, (chTime->GetValue(i)).TimeValue());
						chScale->SetValue(i, Point3(curScale,curScale,curScale) );
					}
			} else {
				for(int i=0; i<amount; i++)
					if (chNew->IsNew(i))
						chScale->SetValue(i, curScaleXYZ );
			}
		}
	}

	return true;
}

ClassDesc* PFOperatorSimpleShape::GetClassDesc() const
{
	return GetPFOperatorSimpleShapeDesc();
}

// TODO: remove the size parameter
void PFOperatorSimpleShape::BuildMesh(int meshType, float size)
{
	switch (meshType)
	{
	case kSimpleShape_shape_vertex:
		BuildMeshVertex();
		break;
	case kSimpleShape_shape_pyramid:
		BuildMeshPyramid(size);
		break;
	case kSimpleShape_shape_cube:
		BuildMeshCube(size);
		break;
	case kSimpleShape_shape_sphere:
		BuildMeshSphere(size);
		break;
	}
}

void PFOperatorSimpleShape::BuildMeshVertex()
{
	_mesh().setNumVerts(1);
	_mesh().setNumFaces(0);
	_mesh().setVert(0, Point3::Origin);
}

void PFOperatorSimpleShape::BuildMeshPyramid(float size)
{
	_mesh().setNumVerts(4);
	_mesh().setNumFaces(4);

	float right_x  = 0.942809 * size;
	float left_x   = -0.471405 * size;
	float front_y  = 0.816497*size;
	float back_y   = -0.816497*size;
	float bottom_z = -0.333333 * size;
	_mesh().setVert(0, Point3(0.0f,    0.0f,    size));
	_mesh().setVert(1, Point3(right_x, 0.0f,    bottom_z));
	_mesh().setVert(2, Point3(left_x,  front_y, bottom_z));
	_mesh().setVert(3, Point3(left_x,  back_y,  bottom_z));

	Face f;
	f.flags = 65607;

	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 2;  	f.smGroup = 2;   _mesh().faces[0] = f;
	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  	f.smGroup = 4;   _mesh().faces[1] = f;
	f.v[0] = 0;  f.v[1] = 3;  f.v[2] = 1;  	f.smGroup = 8;   _mesh().faces[2] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 2;  	f.smGroup = 16;  _mesh().faces[3] = f;
}

void PFOperatorSimpleShape::BuildMeshCube(float size)
{
	_mesh().setNumVerts(8);
	_mesh().setNumFaces(12);

	float hsize = 0.5 * size;
	float nhsize = -0.5 * size;
	_mesh().setVert(0, Point3(nhsize, nhsize, nhsize));
	_mesh().setVert(1, Point3(hsize,  nhsize, nhsize));
	_mesh().setVert(2, Point3(nhsize, hsize,  nhsize));
	_mesh().setVert(3, Point3(hsize,  hsize,  nhsize));
	_mesh().setVert(4, Point3(nhsize, nhsize, hsize));
	_mesh().setVert(5, Point3(hsize,  nhsize, hsize));
	_mesh().setVert(6, Point3(nhsize, hsize,  hsize));
	_mesh().setVert(7, Point3(hsize,  hsize,  hsize));

	Face f;

	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  	f.smGroup = 2;   f.flags = 65603;  _mesh().faces[0] = f;
	f.v[0] = 3;  f.v[1] = 1;  f.v[2] = 0;  	f.smGroup = 2;   f.flags = 65603;  _mesh().faces[1] = f;
	f.v[0] = 4;  f.v[1] = 5;  f.v[2] = 7;  	f.smGroup = 4;   f.flags = 67;     _mesh().faces[2] = f;
	f.v[0] = 7;  f.v[1] = 6;  f.v[2] = 4;  	f.smGroup = 4;   f.flags = 67;     _mesh().faces[3] = f;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 5;  	f.smGroup = 8;   f.flags = 262211; _mesh().faces[4] = f;
	f.v[0] = 5;  f.v[1] = 4;  f.v[2] = 0;  	f.smGroup = 8;	 f.flags = 262211; _mesh().faces[5] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 7;  	f.smGroup = 16;  f.flags = 196675; _mesh().faces[6] = f;
	f.v[0] = 7;  f.v[1] = 5;  f.v[2] = 1;  	f.smGroup = 16;  f.flags = 196675; _mesh().faces[7] = f;
	f.v[0] = 3;  f.v[1] = 2;  f.v[2] = 6;  	f.smGroup = 32;  f.flags = 327747; _mesh().faces[8] = f;
	f.v[0] = 6;  f.v[1] = 7;  f.v[2] = 3;  	f.smGroup = 32;  f.flags = 327747; _mesh().faces[9] = f;
	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 4;  	f.smGroup = 64;  f.flags = 131139; _mesh().faces[10] = f;
	f.v[0] = 4;  f.v[1] = 6;  f.v[2] = 2;  	f.smGroup = 64;  f.flags = 131139; _mesh().faces[11] = f;
}

void PFOperatorSimpleShape::BuildMeshSphere(float size)
{
	_mesh().setNumVerts(12);
	_mesh().setNumFaces(20);

	_mesh().setVert(0, Point3(0.000000,0.000000,0.500000));
	_mesh().setVert(1, Point3(0.447214,0.000000,0.223607));
	_mesh().setVert(2, Point3(0.138197,0.425325,0.223607));
	_mesh().setVert(3, Point3(-0.361803,0.262866,0.223607));
	_mesh().setVert(4, Point3(-0.361803,-0.262866,0.223607));
	_mesh().setVert(5, Point3(0.138197,-0.425325,0.223607));
	_mesh().setVert(6, Point3(0.361803,0.262866,-0.223607));
	_mesh().setVert(7, Point3(-0.138197,0.425325,-0.223607));
	_mesh().setVert(8, Point3(-0.447214,-0.000000,-0.223607));
	_mesh().setVert(9, Point3(-0.138197,-0.425325,-0.223607));
	_mesh().setVert(10, Point3(0.361804,-0.262865,-0.223607));
	_mesh().setVert(11, Point3(0.000000,0.000000,-0.500000));

	size *= 2.0f;
	for (int i = 0; i < 12; i++)
		_mesh().getVert(i) *= size;

	Face f;
	f.smGroup = 1;	f.flags = 65607;

	f.v[0] = 0;f.v[1] = 1;f.v[2] = 2;  	_mesh().faces[0] = f;
	f.v[0] = 0;f.v[1] = 2;f.v[2] = 3;  	_mesh().faces[1] = f;
	f.v[0] = 0;f.v[1] = 3;f.v[2] = 4;  	_mesh().faces[2] = f;
	f.v[0] = 0;f.v[1] = 4;f.v[2] = 5;  	_mesh().faces[3] = f;
	f.v[0] = 0;f.v[1] = 5;f.v[2] = 1;  	_mesh().faces[4] = f;
	f.v[0] = 1;f.v[1] = 10;f.v[2] = 6; 	_mesh().faces[5] = f;
	f.v[0] = 2;f.v[1] = 6;f.v[2] = 7;  	_mesh().faces[6] = f;
	f.v[0] = 3;f.v[1] = 7;f.v[2] = 8;  	_mesh().faces[7] = f;
	f.v[0] = 4;f.v[1] = 8;f.v[2] = 9;  	_mesh().faces[8] = f;
	f.v[0] = 5;f.v[1] = 9;f.v[2] = 10; 	_mesh().faces[9] = f;
	f.v[0] = 6;f.v[1] = 2;f.v[2] = 1;  	_mesh().faces[10] = f;
	f.v[0] = 7;f.v[1] = 3;f.v[2] = 2;  	_mesh().faces[11] = f;
	f.v[0] = 8;f.v[1] = 4;f.v[2] = 3;  	_mesh().faces[12] = f;
	f.v[0] = 9;f.v[1] = 5;f.v[2] = 4;  	_mesh().faces[13] = f;
	f.v[0] = 10;f.v[1] = 1;f.v[2] = 5; 	_mesh().faces[14] = f;
	f.v[0] = 11;f.v[1] = 7;f.v[2] = 6; 	_mesh().faces[15] = f;
	f.v[0] = 11;f.v[1] = 8;f.v[2] = 7; 	_mesh().faces[16] = f;
	f.v[0] = 11;f.v[1] = 9;f.v[2] = 8; 	_mesh().faces[17] = f;
	f.v[0] = 11;f.v[1] = 10;f.v[2] = 9; _mesh().faces[18] = f;
	f.v[0] = 11;f.v[1] = 6;f.v[2] = 10; _mesh().faces[19] = f;

}


} // end of namespace PFActions