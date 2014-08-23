/**********************************************************************
 *<
	FILE:			PFOperatorSpeedSurfaceNormals.cpp

	DESCRIPTION:	SpeedSurfaceNormals Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSpeedSurfaceNormals.h"

#include "PFOperatorSpeedSurfaceNormals_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

// SpeedBySurface creates a particle channel to store a random float for each particle
// when using speed variation
// This way the random variation value is calculated only once when a particle enters the event.
#define PARTICLECHANNELRNDFLOATR_INTERFACE Interface_ID(0x38410659, 0x1eb34500) 
#define PARTICLECHANNELRNDFLOATW_INTERFACE Interface_ID(0x38410659, 0x1eb34501) 

#define GetParticleChannelRndFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELRNDFLOATR_INTERFACE)) 
#define GetParticleChannelRndFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELRNDFLOATW_INTERFACE)) 

// SpeedBySurface creates a particle channel to store a speed magnitude when a particle 
// enters the event. The value is used later if it is necessary to have the same as incoming speed
#define PARTICLECHANNELSPEEDVALR_INTERFACE Interface_ID(0x38410660, 0x1eb34500) 
#define PARTICLECHANNELSPEEDVALW_INTERFACE Interface_ID(0x38410660, 0x1eb34501) 

#define GetParticleChannelSpeedValRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELSPEEDVALR_INTERFACE)) 
#define GetParticleChannelSpeedValWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELSPEEDVALW_INTERFACE)) 

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorSpeedBySurfaceLocalData				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorSpeedBySurfaceLocalData::FreeAll()
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) delete _geomData(i);
	_geomData().ZeroCount();
}

void PFOperatorSpeedBySurfaceLocalData::InitNodeGeometry(int num)
{
	_geomData().SetCount(num);
	for(int i=0; i<num; i++) _geomData(i) = NULL;
}

int PFOperatorSpeedBySurfaceLocalData::NodeGeometryCount() const
{
	return geomData().Count();
}

PFNodeGeometry* PFOperatorSpeedBySurfaceLocalData::GetNodeGeometry(int i)
{
	if ((i < 0) || (i >= geomData().Count())) return NULL;
	return geomData(i);
}

void PFOperatorSpeedBySurfaceLocalData::SetNodeGeometry(int i, PFNodeGeometry* nodeG)
{
	if ((i>=0) && (i<geomData().Count())) _geomData(i) = nodeG;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorSpeedBySurface						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
GeomObjectValidatorClass PFOperatorSpeedSurfaceNormals::validator;
bool PFOperatorSpeedSurfaceNormals::validatorInitiated = false;

PFOperatorSpeedSurfaceNormals::PFOperatorSpeedSurfaceNormals()
{ 
	_numSurfaces() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kSpeedBySurface_objects;
		speedSurfaceNormals_paramBlockDesc.ParamOption(kSpeedBySurface_objects,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSpeedSurfaceNormals::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &speedSurfaceNormals_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &speedSurfaceNormals_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &speedSurfaceNormals_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSpeedSurfaceNormals::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SPEEDSURFACENORMALS_CLASS_NAME);
}

Class_ID PFOperatorSpeedSurfaceNormals::ClassID()
{
	return PFOperatorSpeedSurfaceNormals_Class_ID;
}

void PFOperatorSpeedSurfaceNormals::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSpeedSurfaceNormals::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSpeedSurfaceNormals::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kSpeedBySurface_objectsMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSSurfaces();
					return REF_STOP;
				} 
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kSpeedBySurface_objects:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealSurfaces();
					if (updateObjectNames(kSpeedBySurface_objects)) {
						RefreshUI(kSpeedSurfaceNormals_message_update);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				case kSpeedBySurface_type:
				case kSpeedBySurface_setSpeed:
				case kSpeedBySurface_speedByMtl:
				case kSpeedBySurface_materialType:
				case kSpeedBySurface_useSubMtl:
				case kSpeedBySurface_directionType:
				case kSpeedBySurface_unlimitedRange:
					RefreshUI(kSpeedSurfaceNormals_message_update);
					break;
				} 
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(kSpeedBySurface_objects);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealSurfaces();
			break;
		case kSpeedSurfaceNormals_RefMsg_InitDlg:
			RefreshUI(kSpeedSurfaceNormals_message_update, (IParamMap2*)partID);
			return REF_STOP;
		case kSpeedSurfaceNormals_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kSpeedSurfaceNormals_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kSpeedSurfaceNormals_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSpeedSurfaceNormals::Clone(RemapDir &remap)
{
	PFOperatorSpeedSurfaceNormals* newOp = new PFOperatorSpeedSurfaceNormals();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSpeedSurfaceNormals::GetObjectName()
{
	return GetString(IDS_OPERATOR_SPEEDSURFACENORMALS_OBJECT_NAME);
}

void CollectPFNodeObjects(INode* inode, Tab<INode*>& collectedNodes)
{
	if (inode == NULL) return;
	// check out if the node is already collected
	bool alreadyCollected = false;
	int i;
	for(i=0; i<collectedNodes.Count(); i++) {
		if (inode == collectedNodes[i]) {
			alreadyCollected = true;
			break;
		}
	}

	if (!alreadyCollected) collectedNodes.Append(1, &inode);
	for(i=0; i<inode->NumberOfChildren(); i++)
		CollectPFNodeObjects(inode->GetChildNode(i), collectedNodes);
}

bool PFOperatorSpeedSurfaceNormals::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	// collect all nodes with surface geometry
	Tab<INode*> nodeList;
	int i;
	for(i=0; i<pblock()->Count(kSpeedBySurface_objects); i++)
		CollectPFNodeObjects(pblock()->GetINode(kSpeedBySurface_objects, 0, i), nodeList);
	if (nodeList.Count() > 0) {
		_localData(pCont).InitNodeGeometry(nodeList.Count());
		for(i=0; i<nodeList.Count(); i++) {
			PFNodeGeometry* nodeG = new PFNodeGeometry();
			nodeG->Init(nodeList[i], pblock()->GetInt(kSpeedBySurface_animated, 0),
									pblock()->GetInt(kSpeedBySurface_subframe, 0),
									kLocationType_undefined, NULL);
			_localData(pCont).SetNodeGeometry(i, nodeG);
		}
	}
	return initRes;
}

bool PFOperatorSpeedSurfaceNormals::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_localData(pCont).FreeAll();
	_localData().erase( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorSpeedSurfaceNormals::ChannelsUsed(const Interval& time) const
{
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Position|PCU_Speed|PCU_Amount, // read channels
									PCU_Speed); // write channel
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedSurfaceNormals::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDSURFACENORMALS_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedSurfaceNormals::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDSURFACENORMALS_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSpeedSurfaceNormals::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kSpeedBySurface_objects);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kSpeedBySurface_objects, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	if (count > 0) {
		INode* force = pblock()->GetINode(kSpeedBySurface_objects, 0, firstIndex);
		nameSuffix = force->GetName();
		if (count > 1) {
			TCHAR buf[32];
			sprintf(buf, " +%d", count-1);
			nameSuffix += buf;
		}
	} else {
		nameSuffix = GetString(IDS_NONE);
	}
	return true;
}

bool PFOperatorSpeedSurfaceNormals::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	IPFSystem* pfSys = GetPFSystemInterface(pSystem);
	if (pfSys == NULL) return false; // no handle for PFSystem interface

	PFOperatorSpeedBySurfaceLocalData* localData = &(_localData(pCont));
	Tab<PFNodeGeometry*> nodeGeom;
	nodeGeom.SetCount(localData->NodeGeometryCount());
	if (nodeGeom.Count() == 0) return true; // no geometry to work with
	for(int i=0; i<nodeGeom.Count(); i++)
		nodeGeom[i] = localData->GetNodeGeometry(i);

// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int amount = chAmount->Count();
	if (amount == 0) return true; // no particles to deal with
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	if (chPos == NULL) return false; // can't read particle position

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	bool initSpeed = false;
	IParticleChannelPoint3W* chSpeedW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSPEEDR_INTERFACE,
																			PARTICLECHANNELSPEEDW_INTERFACE, true,
																			actionNode, NULL, &initSpeed);
	if (chSpeedW == NULL) return false; // can't modify Speed channel in the container
	IParticleChannelPoint3R* chSpeedR = GetParticleChannelSpeedRInterface(pCont);
	if (chSpeedR == NULL) return false; // can't read particle speed

	int syncType = pblock()->GetInt(kSpeedBySurface_syncBy);
	bool initEventStart = false;
	IParticleChannelPTVR* chBirth = NULL;
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	if (syncType == kSpeedBySurface_syncBy_age) {
		chBirth = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirth == NULL) return false; // can't read particle age
	}
	if (syncType == kSpeedBySurface_syncBy_event) {
		chEventStartR = (IParticleChannelPTVR*) chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																	actionNode, NULL, &initEventStart);
		if (chEventStartR == NULL) return false; // can't read/create EventStart channel
		if (initEventStart) {
			chEventStartW = GetParticleChannelEventStartWInterface(pCont);
			if (chEventStartW == NULL) return false; // can't init EventStart channel
		}
	}

	int speedType = pblock()->GetInt(kSpeedBySurface_type, timeStart);
	int setSpeed = pblock()->GetInt(kSpeedBySurface_setSpeed, timeStart);
	bool useSpeed = (speedType == kSpeedBySurface_type_once) || (setSpeed != 0);
	bool useRndFloat = ((speedType == kSpeedBySurface_type_continuous) && (setSpeed != 0));
	bool useIncomingSpeed = !useSpeed;
	int subframeSampling = pblock()->GetInt(kSpeedBySurface_subframe, timeStart);
	int speedByMtl = pblock()->GetInt(kSpeedBySurface_speedByMtl, timeStart);
	int mtlType = pblock()->GetInt(kSpeedBySurface_materialType, timeStart);
	int useSubMtl = pblock()->GetInt(kSpeedBySurface_useSubMtl, timeStart);
	int mtlID = pblock()->GetInt(kSpeedBySurface_mtlID, timeStart) - 1; // offset to 0-based indices
	int directionType = pblock()->GetInt(kSpeedBySurface_directionType, timeStart);
	int unlimitedRange = pblock()->GetInt(kSpeedBySurface_unlimitedRange, timeStart);
	bool useDirection = (speedByMtl == 0) || (mtlType == kSpeedBySurface_materialType_grayscale) 
										|| (mtlType == kSpeedBySurface_materialType_lumCenter);

	bool initRndFloat = false;
	IParticleChannelFloatR* chRndFloatR = NULL;
	IParticleChannelFloatW* chRndFloatW = NULL;
	if (useRndFloat) {
		chRndFloatR = (IParticleChannelFloatR*) chCont->EnsureInterface(PARTICLECHANNELRNDFLOATR_INTERFACE,
																	ParticleChannelFloat_Class_ID,
																	true, PARTICLECHANNELRNDFLOATR_INTERFACE,
																	PARTICLECHANNELRNDFLOATW_INTERFACE, true,
																	actionNode, (Object*)this, &initRndFloat);
		if (chRndFloatR == NULL) return false; // can't read/create RndFloat channel
		if (initRndFloat) {
			chRndFloatW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELRNDFLOATW_INTERFACE, (Object*)this);
			if (chRndFloatW == NULL) return false; // can't init RndFloat channel
		}
	}

	bool initSpeedVal = false;
	IParticleChannelFloatR* chSpeedValR = NULL;
	IParticleChannelFloatW* chSpeedValW = NULL;
	if (useIncomingSpeed) {
		chSpeedValR = (IParticleChannelFloatR*) chCont->EnsureInterface(PARTICLECHANNELSPEEDVALR_INTERFACE,
																	ParticleChannelFloat_Class_ID,
																	true, PARTICLECHANNELSPEEDVALR_INTERFACE,
																	PARTICLECHANNELSPEEDVALW_INTERFACE, true,
																	actionNode, (Object*)this, &initSpeedVal);
		if (chSpeedValR == NULL) return false; // can't read/create SpeedVal channel
		if (initSpeedVal) {
			chSpeedValW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELSPEEDVALW_INTERFACE, (Object*)this);
			if (chSpeedValW == NULL) return false; // can't init SpeedVal channel
		}
	}

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;

	float speed = 0.0f;
	float divergence = 0.0f;
	float accelLimit = 10000.0f;
	float range = 10000.0f;
	float falloff = 10000.0f;
	Point3 curPos;
	float minDistance2=0.0f, curDistance2;
	Point3 localCoords, curLocalCoords;
	int faceIndex, curFaceIndex, nodeIndex;
	Point3 worldPoint, curWorldPoint, difPoint, normal = Point3::XAxis;
	Point3 speedToBe = Point3::Origin, faceNormal, curSpeed;
	bool hasMin;

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();
	bool loadedMapFiles = false;

	for(i=0; i<amount; i++)
	{
		bool isNew = chNew->IsNew(i);
		if (!isNew && (speedType == kSpeedBySurface_type_once)) continue;
		if (isNew && initSpeed) chSpeedW->SetValue(i, Point3::ZAxis/TIME_TICKSPERSEC);

		PreciseTimeValue time = chTime->GetValue(i);
		if (isNew && initEventStart) chEventStartW->SetValue(i, time);
		PreciseTimeValue syncTime = time;
		if (syncType == kSpeedBySurface_syncBy_age)
			syncTime -= chBirth->GetValue(i);
		else if (syncType == kSpeedBySurface_syncBy_event)
			syncTime -= chEventStartR->GetValue(i);

		if (useSpeed) {
			speed = GetPFFloat(pblock(), kSpeedBySurface_speed, syncTime);
			float curRand = 0.0f;
			if (useRndFloat) {
				if (initRndFloat && isNew) {
					curRand = randGen->Rand11();
					chRndFloatW->SetValue(i, curRand);
				} else {
					curRand = chRndFloatR->GetValue(i);
				}
			} else {
				curRand = randGen->Rand11();
			}
			speed += curRand*GetPFFloat(pblock(), kSpeedBySurface_speedVariation, syncTime);
			speed /= TIME_TICKSPERSEC;
		} else {
			if (initSpeedVal && isNew) {
				speed = Length(chSpeedR->GetValue(i));
				chSpeedValW->SetValue(i, speed);
			} else {
				speed = chSpeedValR->GetValue(i);
			}
		}

		// find closest point on the surface geometry
		curPos = chPos->GetValue(i);
		hasMin = false;
		for(int j=0; j<nodeGeom.Count(); j++) {
			if (!nodeGeom[j]->GetClosestPoint(time, curPos, curWorldPoint, curLocalCoords, curFaceIndex))
				continue;
			difPoint = curPos - curWorldPoint;
			curDistance2 = DotProd(difPoint, difPoint);
			if ((curDistance2 < minDistance2) || !hasMin) {
				hasMin = true;
				minDistance2 = curDistance2;
				nodeIndex = j;
				worldPoint = curWorldPoint;
				localCoords = curLocalCoords;
				faceIndex = curFaceIndex;
			}
		}

		if (!hasMin) continue; // can't define a closest point for the current position of particle
				
		// find normal at the closest point
		INode* inode = nodeGeom[nodeIndex]->GetNode();
		Mesh* mesh = nodeGeom[nodeIndex]->GetMesh(time);
		Matrix3* tm = nodeGeom[nodeIndex]->GetTM(time);
		if ((inode == NULL) || (mesh == NULL) || (tm == NULL)) continue;

		Mtl* mtl = NULL;
		if (speedByMtl != 0) {
			mtl = inode->GetMtl();
			if ((mtl != NULL) && useSubMtl)
				if (mtlID < mtl->NumSubMtls()) mtl = mtl->GetSubMtl(mtlID);
		}

		if ((speedByMtl != 0) && 
			((mtlType == kSpeedBySurface_materialType_grayscale) ||
			(mtlType == kSpeedBySurface_materialType_lumCenter)) && 
			(mtl != NULL)) 
		{
			if (!loadedMapFiles) {
				int numSubs = mtl->NumSubTexmaps();
				for(int subIndex = 0; subIndex<numSubs; subIndex++) {
					Texmap* subTexmap = mtl->GetSubTexmap(subIndex);
					if (subTexmap != NULL)
						subTexmap->LoadMapFiles(timeEnd);
				}
			loadedMapFiles = true;
			}
			PFObjectMaterialShadeContext sc;
			LocationPoint lp;
			lp.init = true;
			lp.location = localCoords;
			lp.node = inode;
			lp.pointIndex = faceIndex;
			sc.Init(nodeGeom[nodeIndex], lp, worldPoint, time);
			// pre-init colors to avoid under-initialized colors
			sc.out.c = Point3(1.0f, 1.0f, 1.0f);
			mtl->Shade(sc);
			if (mtlType == kSpeedBySurface_materialType_lumCenter)
				speed *= (sc.out.c.r+sc.out.c.g+sc.out.c.b)/1.5f - 1.0f;
			else
				speed *= (sc.out.c.r+sc.out.c.g+sc.out.c.b)/3.0f;
		}

		float distance = (minDistance2 > 0.0f) ? sqrt(minDistance2) : 0.0f;
		curSpeed = chSpeedR->GetValue(i);	
		if (useDirection) {
			Box3 box = mesh->getBoundingBox();
			box = box*(*tm);
			float layerSize = 0.01f*Length(box.pmax - box.pmin);
			if (layerSize <= 0.0f) continue; // invalid surface object
			faceNormal = VectorTransform(*tm, mesh->FaceNormal(faceIndex, TRUE));
			if (distance > layerSize) {
				normal = Normalize(curPos - worldPoint);
				if (directionType == kSpeedBySurface_directionType_normals) {
					if (DotProd(faceNormal, normal) < 0.0f)
						normal *= -1.0f;
				}
			} else {
				Point3 normal1 = Point3::XAxis;
				if (distance > 0.0f) normal1 = Normalize(curPos - worldPoint);
				float ratio = distance/layerSize;
				if (DotProd(faceNormal, normal) < 0.0f) {
					if (directionType == kSpeedBySurface_directionType_normals)
						normal *= -1.0f;
					else
						faceNormal *= -1.0f;
				}
				normal = Normalize((1.0f-ratio)*faceNormal + ratio*normal1);
			}
			if (directionType == kSpeedBySurface_directionType_parallel) {
				speedToBe = curSpeed - DotProd(curSpeed, normal)*normal;
				while (LengthSquared(speedToBe) <= 0.0f) {
					Point3 randomVector = RandSphereSurface(randGen);
					speedToBe = randomVector - DotProd(randomVector, normal)*normal;
				}
				speedToBe = Normalize(speedToBe);
			} else {
				speedToBe = normal;
			}
		} else {
			if ((speedByMtl != 0) && 
				(mtlType != kSpeedBySurface_materialType_grayscale) && 
				(mtlType != kSpeedBySurface_materialType_lumCenter) && 
				(mtl != NULL)) 
			{
				PFObjectMaterialShadeContext sc;
				LocationPoint lp;
				lp.init = true;
				lp.location = localCoords;
				lp.node = inode;
				lp.pointIndex = faceIndex;
				sc.Init(nodeGeom[nodeIndex], lp, worldPoint, time);
				// pre-init colors to avoid under-initialized colors
				sc.out.c = Point3(1.0f, 1.0f, 1.0f);
				mtl->Shade(sc);
				speedToBe = 2.0f*Point3(sc.out.c.r - 0.5f, sc.out.c.g - 0.5f, sc.out.c.b - 0.5f);
				if (mtlType == kSpeedBySurface_materialType_localRGB)
					speedToBe = tm->VectorTransform(speedToBe);
			}
		}

		speedToBe *= speed;

		if (speedType == kSpeedBySurface_type_once) {
			float divergence = GetPFFloat(pblock(), kSpeedBySurface_divergence, syncTime);
			speedToBe = DivergeVectorRandom(speedToBe, randGen, divergence);
			chSpeedW->SetValue(i, speedToBe); 
		} else {
			if (unlimitedRange == 0) {
				range = GetPFFloat(pblock(), kSpeedBySurface_range, syncTime);
				falloff = GetPFFloat(pblock(), kSpeedBySurface_falloff, syncTime);
			}
			// change current speed if...
			if ((unlimitedRange != 0) || (distance < (range+falloff))) {
				accelLimit = GetPFFloat(pblock(), kSpeedBySurface_accelLimit, syncTime);
				if (unlimitedRange == 0) {
					if (distance > range)
						accelLimit *= ((range+falloff) - distance)/falloff;
				}
				accelLimit /= TIME_TICKSPERSEC;
				curSpeed = chSpeedR->GetValue(i);
				Point3 speedDif = speedToBe - curSpeed;
				float speedDifLen = Length(speedDif);
				if (speedDifLen > 0.0f) {
					if (accelLimit < Length(speedDif))
						speedDif = speedDif*accelLimit/speedDifLen;
					chSpeedW->SetValue(i, curSpeed + speedDif);
				}
			}
		}
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

int PFOperatorSpeedSurfaceNormals::getNumSurfaces()
{
	int num = pblock()->Count(kSpeedBySurface_objects);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kSpeedBySurface_objects, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateSurfacesInProgress = false;
bool PFOperatorSpeedSurfaceNormals::updateFromRealSurfaces()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kSpeedBySurface_objects, 
				kSpeedBySurface_objectsMaxscript, gUpdateSurfacesInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kSpeedBySurface_objects, _T("Surface_Objects"));
	return res;
}

bool PFOperatorSpeedSurfaceNormals::updateFromMXSSurfaces()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kSpeedBySurface_objects, 
				kSpeedBySurface_objectsMaxscript, gUpdateSurfacesInProgress, this);
}

bool PFOperatorSpeedSurfaceNormals::updateObjectNames(int pblockID)
{
	if (pblock() == NULL) return false;
	bool needUpdate = false;
	int count = pblock()->Count(pblockID);
	if (count != objectsToShow().Count()) needUpdate = true;
	if (!needUpdate) {
		for(int i=0; i<count; i++)
			if (pblock()->GetINode(pblockID, 0, i) != objectToShow(i))
			{	needUpdate = true; 	break;	}
	}
	if (!needUpdate) return false;
	_objectsToShow().SetCount(count);
	for(int i=0; i<count; i++)
		_objectToShow(i) = pblock()->GetINode(pblockID, 0, i);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From HitByNameDlgCallback						 |
//+--------------------------------------------------------------------------+
int PFOperatorSpeedSurfaceNormals::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFOperatorSpeedSurfaceNormals::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kSpeedBySurface_objects, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}

ClassDesc* PFOperatorSpeedSurfaceNormals::GetClassDesc() const
{
	return GetPFOperatorSpeedSurfaceNormalsDesc();
}

int PFOperatorSpeedSurfaceNormals::GetRand()
{
	return pblock()->GetInt(kSpeedBySurface_seed);
}

void PFOperatorSpeedSurfaceNormals::SetRand(int seed)
{
	_pblock()->SetValue(kSpeedBySurface_seed, 0, seed);
}

} // end of namespace PFActions