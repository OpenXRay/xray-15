/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObject.cpp

	DESCRIPTION:	PositionOnObject Operator implementation
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorPositionOnObject.h"

#include "PFOperatorPositionOnObject_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

// PositionOnObject creates a particle channel to store a local position for each particle
// when "Lock On Emitter" is turned ON.
// This way the value is calculated only once when a particle enters the event.
#define PARTICLECHANNELREFNODER_INTERFACE Interface_ID(0x7e2c3071, 0x1eb34500) 
#define PARTICLECHANNELREFNODEW_INTERFACE Interface_ID(0x7e2c3071, 0x1eb34501) 

#define GetParticleChannelRefNodeRInterface(obj) ((IParticleChannelINodeR*)obj->GetInterface(PARTICLECHANNELREFNODER_INTERFACE)) 
#define GetParticleChannelRefNodeWInterface(obj) ((IParticleChannelINodeW*)obj->GetInterface(PARTICLECHANNELREFNODEW_INTERFACE)) 

#define PARTICLECHANNELLOCALPOSITIONR_INTERFACE Interface_ID(0x7e2c3072, 0x1eb34500) 
#define PARTICLECHANNELLOCALPOSITIONW_INTERFACE Interface_ID(0x7e2c3072, 0x1eb34501) 

#define GetParticleChannelLocalPositionRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELLOCALPOSITIONR_INTERFACE)) 
#define GetParticleChannelLocalPositionWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELLOCALPOSITIONW_INTERFACE)) 

#define PARTICLECHANNELPOINTINDEXR_INTERFACE Interface_ID(0x7e2c3073, 0x1eb34500) 
#define PARTICLECHANNELPOINTINDEXW_INTERFACE Interface_ID(0x7e2c3073, 0x1eb34501) 

#define GetParticleChannelPointIndexRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELPOINTINDEXR_INTERFACE)) 
#define GetParticleChannelPointIndexWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELPOINTINDEXW_INTERFACE)) 


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorPositionOnObjectLocalData				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorPositionOnObjectLocalData::FreeAll()
{
	_points().ZeroCount();
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) delete _geomData(i);
	_geomData().ZeroCount();
}

void PFOperatorPositionOnObjectLocalData::InitDistinctPoints(int num)
{
	_points().SetCount(num);
	for(int i=0; i<num; i++) _point(i).init = false;
}

const LocationPoint& PFOperatorPositionOnObjectLocalData::GetDistinctPoint(int i)
{
	// if index exceed the range of tab then adjust number of point in the tab
	if (i>=0) {
		if (i>=points().Count()) {
			int oldCount = points().Count();
			_points().SetCount(i+1);
			for(int j=oldCount; j<=i; j++) { _point(j).init = false; _point(j).node = NULL; }
		}
		return point(i);
	}
	static LocationPoint aPoint;
	aPoint.init = false;
	aPoint.node = NULL;
	return aPoint;
}

void PFOperatorPositionOnObjectLocalData::SetDistinctPoint(int i, const LocationPoint& p)
{
	// if index exceed the range of tab then adjust number of point in the tab
	if (i>=0) {
		if (i>=points().Count()) {
			int oldCount = points().Count();
			_points().SetCount(i);
			for(int j=oldCount; j<i; j++) _point(i).init = false;
		}
		_point(i) = p;
	}
}

void PFOperatorPositionOnObjectLocalData::InitNodeGeometry(int num)
{
	_geomData().SetCount(num);
	for(int i=0; i<num; i++) _geomData(i) = NULL;
}

//int PFOperatorPositionOnObjectLocalData::NodeGeometryCount() const
//{
//	return geomData().Count();
//}

PFNodeGeometry* PFOperatorPositionOnObjectLocalData::GetNodeGeometry(TimeValue time, float randValue)
{
	double totalShare = 0.0;
	int i;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			totalShare += _geomData(i)->GetProbabilityShare(time);
	if (totalShare == 0.0f) return NULL;
	double curShare = 0.0;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) {
			curShare += _geomData(i)->GetProbabilityShare(time)/totalShare;
			if (curShare >= randValue) return geomData(i);
		}
	return NULL;
}

PFNodeGeometry* PFOperatorPositionOnObjectLocalData::GetNodeGeometry(INode* node)
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			if (geomData(i)->GetNode() == node)
				return geomData(i);
	return NULL;
}

void PFOperatorPositionOnObjectLocalData::SetNodeGeometry(int i, PFNodeGeometry* nodeG)
{
	if ((i>=0) && (i<geomData().Count())) _geomData(i) = nodeG;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorPositionOnObjectState			 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorPositionOnObjectState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorPositionOnObjectState::IObject				 |
//+--------------------------------------------------------------------------+
void PFOperatorPositionOnObjectState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorPositionOnObjectState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorPositionOnObjectState::GetClassID() 
{ 
	return PFOperatorPositionOnObjectState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorPositionOnObjectState::IPFActionState		 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorPositionOnObjectState::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;

	isave->BeginChunk(IPFActionState::kChunkActionHandle);
	ULONG handle = actionHandle();
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen);
	if ((res = isave->Write(randGen(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData);
	int num = points().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0) {
		isave->BeginChunk(IPFActionState::kChunkData2);
		if ((res = isave->Write(points().Addr(0), num*sizeof(LocationPoint), &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorPositionOnObjectState::IPFActionState			 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorPositionOnObjectState::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IPFActionState::kChunkActionHandle:
			res=iload->Read(&_actionHandle(), sizeof(ULONG), &nb);
			break;

		case IPFActionState::kChunkRandGen:
			res=iload->Read(_randGen(), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkData:
			num = 0;
			res=iload->Read(&num, sizeof(int), &nb);
			if (num > 0) _points().SetCount(num);
			break;

		case IPFActionState::kChunkData2:
			res=iload->Read(_points().Addr(0), sizeof(LocationPoint)*points().Count(), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorPositionOnObject						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
GeomObjectValidatorClass PFOperatorPositionOnObject::validator;
bool PFOperatorPositionOnObject::validatorInitiated = false;

PFOperatorPositionOnObject::PFOperatorPositionOnObject()
{ 
	_numEmitters() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kPositionOnObject_emitters;
		positionOnObject_paramBlockDesc.ParamOption(kPositionOnObject_emitters,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorPositionOnObject::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &positionOnObject_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &positionOnObject_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &positionOnObject_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorPositionOnObject::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_POSITIONONOBJECT_CLASS_NAME);
}

Class_ID PFOperatorPositionOnObject::ClassID()
{
	return PFOperatorPositionOnObject_Class_ID;
}

void PFOperatorPositionOnObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorPositionOnObject::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorPositionOnObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	float offsetMin, offsetMax;
	TimeValue currentTime;

	switch (message)
	{
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();	
				if (lastUpdateID == kPositionOnObject_emittersMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSEmitters();
					return REF_STOP;
				}
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kPositionOnObject_emitters:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealEmitters();
					if (updateObjectNames(kPositionOnObject_emitters)) {
						RefreshUI(kPositionOnObject_message_objects);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				case kPositionOnObject_lock:
				case kPositionOnObject_inherit:
					RefreshUI(kPositionOnObject_message_lock);
					break;
				case kPositionOnObject_type:
				case kPositionOnObject_useOffset:
				case kPositionOnObject_useNonUniform:
				case kPositionOnObject_useSubMtl:
				case kPositionOnObject_apartPlacement:
				case kPositionOnObject_distinctOnly:
					RefreshUI(kPositionOnObject_message_type);
					break;
				case kPositionOnObject_offsetMin:
					currentTime = GetCOREInterface()->GetTime();
					offsetMin = pblock()->GetFloat(kPositionOnObject_offsetMin, currentTime);
					offsetMax = pblock()->GetFloat(kPositionOnObject_offsetMax, currentTime);
					if (offsetMin > offsetMax)
						pblock()->SetValue(kPositionOnObject_offsetMax, currentTime, offsetMin);
					break;
				case kPositionOnObject_offsetMax:
					currentTime = GetCOREInterface()->GetTime();
					offsetMin = pblock()->GetFloat(kPositionOnObject_offsetMin, currentTime);
					offsetMax = pblock()->GetFloat(kPositionOnObject_offsetMax, currentTime);
					if (offsetMin > offsetMax)
						pblock()->SetValue(kPositionOnObject_offsetMin, currentTime, offsetMax);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(kPositionOnObject_emitters);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealEmitters();
			break;
		// Initialization of Dialog
		case kPositionOnObject_RefMsg_InitDlg:
			RefreshUI(kPositionOnObject_message_lock, (IParamMap2*)partID);
			RefreshUI(kPositionOnObject_message_type, (IParamMap2*)partID);
			RefreshUI(kPositionOnObject_message_objects, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kPositionOnObject_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kPositionOnObject_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kPositionOnObject_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}
	return REF_SUCCEED;
}

RefTargetHandle PFOperatorPositionOnObject::Clone(RemapDir &remap)
{
	PFOperatorPositionOnObject* newOp = new PFOperatorPositionOnObject();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorPositionOnObject::GetObjectName()
{
	return GetString(IDS_OPERATOR_POSITIONONOBJECT_OBJECT_NAME);
}

void CollectPFNodeEmitters(INode* inode, Tab<INode*>& collectedNodes)
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
		CollectPFNodeEmitters(inode->GetChildNode(i), collectedNodes);
}

bool PFOperatorPositionOnObject::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	// reserve distinct points if necessary
	int numDistinctPoints = 0;
	if (pblock()->GetInt(kPositionOnObject_distinctOnly, 0) != 0)
		numDistinctPoints = GetPFInt(pblock(), kPositionOnObject_distinctTotal, 0);
	_localData(pCont).InitDistinctPoints(numDistinctPoints);
	// collect all nodes as emitters
	Tab<INode*> nodeList;
	int i;
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	for(i=0; i<pblock()->Count(kPositionOnObject_emitters); i++)
		CollectPFNodeEmitters(pblock()->GetINode(kPositionOnObject_emitters, 0, i), nodeList);
	if (nodeList.Count() > 0) {
		_localData(pCont).InitNodeGeometry(nodeList.Count());
		for(i=0; i<nodeList.Count(); i++) {
			PFNodeGeometry* nodeG = new PFNodeGeometry();
			nodeG->Init(nodeList[i], pblock()->GetInt(kPositionOnObject_animated, 0),
									pblock()->GetInt(kPositionOnObject_subframe, 0),
									pblock()->GetInt(kPositionOnObject_type, 0),
									randGen );
			_localData(pCont).SetNodeGeometry(i, nodeG);
		}
	}
	return initRes;
}

bool PFOperatorPositionOnObject::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_localData(pCont).FreeAll();
	_localData().erase( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorPositionOnObject::ChannelsUsed(const Interval& time) const
{
								//  read	                       &	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_Position|PCU_Speed);
	return mask;
}

int	PFOperatorPositionOnObject::GetRand()
{
	return pblock()->GetInt(kPositionOnObject_randomSeed);
}

void PFOperatorPositionOnObject::SetRand(int seed)
{
	pblock()->SetValue(kPositionOnObject_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorPositionOnObject::GetCurrentState(IObject* pContainer)
{
	PFOperatorPositionOnObjectState* actionState = (PFOperatorPositionOnObjectState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorPositionOnObjectState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	actionState->_points() = _localData(pContainer).points();
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorPositionOnObject::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorPositionOnObjectState* actionState = (PFOperatorPositionOnObjectState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
	int num = actionState->points().Count();
	_localData(pContainer).InitDistinctPoints(num);
	for(int i=0; i<num; i++)
		_localData(pContainer).SetDistinctPoint(i, actionState->points()[i]);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorPositionOnObject::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_POSITIONONOBJECT_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorPositionOnObject::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_POSITIONONOBJECT_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorPositionOnObject::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kPositionOnObject_emitters);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kPositionOnObject_emitters, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	if (count > 0) {
		INode* force = pblock()->GetINode(kPositionOnObject_emitters, 0, firstIndex);
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

class SCDensity: public ShadeContext {
public:
	ShadeContext *origsc;
	TimeValue curtime;
	Point3 ltPos; // position of point in light space
	Point3 view;  // unit vector from light to point, in light space
	Point3 dp; 
	Point2 uv,duv;
	IPoint2 scrpos;
	float curve;
	int projType;

	BOOL 	  InMtlEditor() { return origsc->InMtlEditor(); }
	LightDesc* Light(int n) { return NULL; }
	TimeValue CurTime() { return curtime; }
	int NodeID() { return -1; }
	int FaceNumber() { return 0; }
	int ProjType() { return projType; }
	Point3 Normal() { return Point3(0,0,0); }
	Point3 GNormal() { return Point3(0,0,0); }
	Point3 ReflectVector(){ return Point3(0,0,0); }
	Point3 RefractVector(float ior){ return Point3(0,0,0); }
	Point3 CamPos() { return Point3(0,0,0); }
	Point3 V() { return view; }
	void SetView(Point3 v) { view = v; }
	Point3 P() { return ltPos; }	
	Point3 DP() { return dp; }
	Point3 PObj() { return ltPos; }
	Point3 DPObj(){ return Point3(0,0,0); } 
	Box3 ObjectBox() { return Box3(Point3(-1,-1,-1),Point3(1,1,1));}   	  	
	Point3 PObjRelBox() { return view; }
	Point3 DPObjRelBox() { return Point3(0,0,0); }
	void ScreenUV(Point2& UV, Point2 &Duv) { UV = uv; Duv = duv; }
	IPoint2 ScreenCoord() { return scrpos;} 
	Point3 UVW(int chan) { return Point3(uv.x, uv.y, 0.0f); }
	Point3 DUVW(int chan) { return Point3(duv.x, duv.y, 0.0f);  }
	void DPdUVW(Point3 dP[3], int chan) {}  // dont need bump vectors
	void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) {}   // returns Background color, bg transparency
		
	float Curve() { return curve; }

	// Transform to and from internal space
	Point3 PointTo(const Point3& p, RefFrame ito) { return p; } 
	Point3 PointFrom(const Point3& p, RefFrame ifrom) { return p; } 
	Point3 VectorTo(const Point3& p, RefFrame ito) { return p; } 
	Point3 VectorFrom(const Point3& p, RefFrame ifrom) { return p; } 
	SCDensity(){ doMaps = TRUE; 	curve = 0.0f;	dp = Point3(0.0f,0.0f,0.0f); }
};

bool PFOperatorPositionOnObject::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	IPFSystem* pfSys = GetPFSystemInterface(pSystem);
	if (pfSys == NULL) return false; // no handle for PFSystem interface

// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	bool initPos = false;
	IParticleChannelPoint3W* chPosW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELPOSITIONW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELPOSITIONR_INTERFACE,
																			PARTICLECHANNELPOSITIONW_INTERFACE, true,
																			actionNode, NULL, &initPos);
	if (chPosW == NULL) return false; // can't modify Position channel in the container
	IParticleChannelPoint3R* chPosR = GetParticleChannelPositionRInterface(pCont);
	if (chPosR == NULL) return false; // can't read particle position

	BOOL lockOnEmitter = pblock()->GetInt(kPositionOnObject_lock);
	BOOL inheritSpeed = pblock()->GetInt(kPositionOnObject_inherit);

	IParticleChannelPoint3W* chSpeed = NULL;
	bool initSpeed = false;
	if (lockOnEmitter || inheritSpeed) {
		chSpeed = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
															ParticleChannelPoint3_Class_ID,
															true, PARTICLECHANNELSPEEDR_INTERFACE,
															PARTICLECHANNELSPEEDW_INTERFACE, true,
															actionNode, NULL, &initSpeed);
		if (chSpeed == NULL) return false; // can't modify Speed channel in the container
	}

	IParticleChannelINodeR* chRefNodeR = NULL;
	IParticleChannelINodeW* chRefNodeW = NULL;
	bool initRefNode = false;
	IParticleChannelPoint3R* chLocalPosR = NULL;
	IParticleChannelPoint3W* chLocalPosW = NULL;
	bool initLocalPos = false;
	IParticleChannelIntR* chPointIndexR = NULL;
	IParticleChannelIntW* chPointIndexW = NULL;
	bool initPointIndex = false;

	if (lockOnEmitter) {
		chRefNodeR = (IParticleChannelINodeR*)chCont->EnsureInterface(PARTICLECHANNELREFNODER_INTERFACE,
															ParticleChannelINode_Class_ID,
															true, PARTICLECHANNELREFNODER_INTERFACE,
															PARTICLECHANNELREFNODEW_INTERFACE, true, // transferrable to lock particles on emitter
															actionNode, (Object*)this, &initRefNode);
		if (chRefNodeR == NULL) return false;
		chRefNodeW = (IParticleChannelINodeW*)chCont->GetPrivateInterface(PARTICLECHANNELREFNODEW_INTERFACE, (Object*)this);
		if (chRefNodeW == NULL) return false;

		chLocalPosR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELLOCALPOSITIONR_INTERFACE,
															ParticleChannelPoint3_Class_ID,
															true, PARTICLECHANNELLOCALPOSITIONR_INTERFACE,
															PARTICLECHANNELLOCALPOSITIONW_INTERFACE, true, // transferrable to lock particles on emitter
															actionNode, (Object*)this, &initLocalPos);
		if (chLocalPosR == NULL) return false;
		chLocalPosW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALPOSITIONW_INTERFACE, (Object*)this);
		if (chLocalPosW == NULL) return false;

		chPointIndexR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELPOINTINDEXR_INTERFACE,
															ParticleChannelInt_Class_ID,
															true, PARTICLECHANNELPOINTINDEXR_INTERFACE,
															PARTICLECHANNELPOINTINDEXW_INTERFACE, true, // transferrable to lock particles on emitter
															actionNode, (Object*)this, &initPointIndex);
		if (chPointIndexR == NULL) return false;
		chPointIndexW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELPOINTINDEXW_INTERFACE, (Object*)this);
		if (chPointIndexW == NULL) return false;
	}

	int subframeSampling = pblock()->GetInt(kPositionOnObject_subframe, timeStart);
	int locationType = pblock()->GetInt(kPositionOnObject_type, timeStart);
	bool selectedSet = ((locationType == kPositionOnObject_locationType_selVertex) 
						|| (locationType == kPositionOnObject_locationType_selEdge)
						|| (locationType == kPositionOnObject_locationType_selFaces));
	int useOffset = pblock()->GetInt(kPositionOnObject_useOffset, timeStart);
	float offsetMin = GetPFFloat(pblock(), kPositionOnObject_offsetMin, timeStart);
	float offsetMax = GetPFFloat(pblock(), kPositionOnObject_offsetMax, timeStart);
	int useNonUniform = pblock()->GetInt(kPositionOnObject_useNonUniform, timeStart);
	int densityType = pblock()->GetInt(kPositionOnObject_densityType, timeStart);
	int useSubMtl = pblock()->GetInt(kPositionOnObject_useSubMtl, timeStart);
	int mtlID = pblock()->GetInt(kPositionOnObject_mtlID, timeStart) - 1;
	int useApart = pblock()->GetInt(kPositionOnObject_apartPlacement, timeStart);
	float apartDistance = GetPFFloat(pblock(), kPositionOnObject_distance, timeStart);
	int distinctOnly = pblock()->GetInt(kPositionOnObject_distinctOnly, timeStart);
	int distinctTotal = GetPFInt(pblock(), kPositionOnObject_distinctTotal, timeStart);
	int attemptsMax = pblock()->GetInt(kPositionOnObject_attempts, timeStart);
	float surfaceOffset = 0.0f;
	if ((locationType == kPositionOnObject_locationType_pivot) ||
		(locationType == kPositionOnObject_locationType_volume)) useOffset = false;
	bool deleteInvalidParticles = (pblock()->GetInt(kPositionOnObject_delete, timeStart) != 0);
	int numObj = getNumEmitters();

	int amount = chAmount->Count();
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	PFOperatorPositionOnObjectLocalData* localData = &(_localData(pCont));
	PFNodeGeometry* nodeGeom;

	Control* paramCtrl = pblock()->GetController(kPositionOnObject_inheritAmount);
	bool isInheritAmountAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float inheritAmount = isInheritAmountAnimated ? 1.0f : GetPFFloat(pblock(), kPositionOnObject_inheritAmount, timeStart);
	paramCtrl = pblock()->GetController(kPositionOnObject_inheritAmountVar);
	bool isInheritAmountVarAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float inheritAmountVar = isInheritAmountVarAnimated ? 1.0f : GetPFFloat(pblock(), kPositionOnObject_inheritAmountVar, timeStart);


	IParticleChannelAmountW* chAmountW = NULL;
	if (deleteInvalidParticles) { 
		// may need a channel to delete particles
		chAmountW = GetParticleChannelAmountWInterface(pCont);
		if (chAmountW == NULL) return false; // can't write number of particles in the container
	}

	// check out if it has valid emitters
	if (getNumEmitters() == 0) {
		if (deleteInvalidParticles) {
			chAmountW->SetCount(0);
		} else {
			for(int i=0; i<amount; i++)
				if (chNew->IsNew(i)) {
					if (initPos) chPosW->SetValue(i, Point3::Origin);
					if (initSpeed) chSpeed->SetValue(i, Point3::Origin);
				}
		}
		return true;
	}

	BitArray particlesToDelete;
	particlesToDelete.SetSize(amount);
	particlesToDelete.ClearAll();

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();
	bool loadedMapFiles = false;

	for(int i=0; i<amount; i++)
	{
		bool isNew = chNew->IsNew(i);
		PreciseTimeValue time = chTime->GetValue(i);
		if (lockOnEmitter || isNew)
		{
			// update position
			LocationPoint locationPoint;
			locationPoint.init = false;
			locationPoint.node = NULL;
			Point3 worldLocation(Point3::Origin);
			if (isNew) 
			{
				bool validPointGenerated = false;
				int distinctIndex = 0;
				if (distinctOnly) {
					distinctIndex = randGen->Rand0X(distinctTotal-1);
					locationPoint = localData->GetDistinctPoint(distinctIndex);
					if (locationPoint.init == true) {
						nodeGeom = localData->GetNodeGeometry(locationPoint.node);
						if (nodeGeom != NULL) {
							nodeGeom->GetLocationPoint(time, worldLocation, locationPoint.location, locationPoint.pointIndex);
							validPointGenerated = true;
						}
					}
				}
				if (!validPointGenerated) {
					if (useOffset) {
						float ratio = randGen->Rand01();
						surfaceOffset = (1.0f-ratio)*offsetMin + ratio*offsetMax;
					} else {
						surfaceOffset = 0.0f;
					}						
					Point3 farWorldLocation; // for useApart: store the farest point
					LocationPoint farLocationPoint;
					farLocationPoint.node = NULL;
					float farDistance = -1.0f;
					for(int j=0; j<attemptsMax; j++)
					{
						nodeGeom = localData->GetNodeGeometry(time, randGen->Rand01());										
						if (nodeGeom != NULL) {
							nodeGeom->GetLocationPoint(time, worldLocation, locationPoint.location, locationPoint.pointIndex,						
														true, surfaceOffset, attemptsMax);
							locationPoint.init = true;
							locationPoint.node = nodeGeom->GetNode();
						} else {
							continue; // try another nodeGeometry
						}
						// check basic condition on point location
						Mesh* placeMesh = nodeGeom->GetMesh(time);
						bool ableToPlace = true;
						switch (locationType) {
						case kPositionOnObject_locationType_vertex:
							if (placeMesh->numVerts == 0) ableToPlace = false;
							break;
						case kPositionOnObject_locationType_edge:
						case kPositionOnObject_locationType_surface:
						case kPositionOnObject_locationType_volume:
							if (placeMesh->numFaces == 0) ableToPlace = false;
							break;
						case kPositionOnObject_locationType_selVertex:
							if (placeMesh->vertSel.IsEmpty()) ableToPlace = false;
							break;
						case kPositionOnObject_locationType_selEdge:
							if (placeMesh->edgeSel.IsEmpty()) ableToPlace = false;
							break;
						case kPositionOnObject_locationType_selFaces:
							if (placeMesh->faceSel.IsEmpty()) ableToPlace = false;
							break;
						}
						if (!ableToPlace) continue;

						validPointGenerated = true;
						if (locationType == kPositionOnObject_locationType_pivot) break;

						if (useNonUniform) { // may reject the point because of the material density
							INode* inode = nodeGeom->GetNode();
							if (inode != NULL) {
								Mtl* mtl = inode->GetMtl();
								Mesh* mesh = nodeGeom->GetMesh(time);
								PFObjectMaterialShadeContext sc;
								if ((mtl != NULL) && useSubMtl)
									if (mtlID < mtl->NumSubMtls()) mtl = mtl->GetSubMtl(mtlID);
								if ((mtl != NULL) && (mesh != NULL)) {
									if (!loadedMapFiles) {
										int numSubs = mtl->NumSubTexmaps();
										for(int subIndex = 0; subIndex<numSubs; subIndex++) {
											Texmap* subTexmap = mtl->GetSubTexmap(subIndex);
											if (subTexmap != NULL)
												subTexmap->LoadMapFiles(timeEnd);
										}
										loadedMapFiles = true;
									}
									sc.Init(nodeGeom, locationPoint, worldLocation, time);
									mtl->Shade(sc);
									float densityValue = 1.0f;
									switch( densityType ) {
									case kPositionOnObject_densityType_grayscale:
										densityValue = sc.out.c.r+sc.out.c.g+sc.out.c.b;
										break;
									case kPositionOnObject_densityType_opacity:
										densityValue = 3.0f - (sc.out.t.r+sc.out.t.g+sc.out.t.b);
										break;
									case kPositionOnObject_densityType_grayscaleOpacity:
										densityValue = min(sc.out.c.r+sc.out.c.g+sc.out.c.b,
															3.0f-(sc.out.t.r+sc.out.t.g+sc.out.t.b));
										break;
									case kPositionOnObject_densityType_red:
										densityValue = 3*sc.out.c.r;
										break;
									case kPositionOnObject_densityType_green:
										densityValue = 3*sc.out.c.g;
										break;
									case kPositionOnObject_densityType_blue:
										densityValue = 3*sc.out.c.b;
										break;
									}
									if (densityValue < 3*randGen->Rand01())
										validPointGenerated = false;
								}
							}
						}
						if (!validPointGenerated) continue; // try again
						if (useApart && ((j+1)<attemptsMax)) { // may reject if the new point is too close to other points
							float apartDistance2 = (attemptsMax-j)*apartDistance/attemptsMax;
							apartDistance2 *= apartDistance2; // distance squared
							float pointDistance = apartDistance2 + 1000.0f;
							bool firstDistance = true;
							for(int k=0; k<amount; k++) {
								if (chNew->IsNew(k) && (k>=i)) continue; // don't compare with particle without a proper location
								Point3 difVec = worldLocation - chPosR->GetValue(k);
								float curDistance = difVec.x*difVec.x + difVec.y*difVec.y + difVec.z*difVec.z;
								if (firstDistance) {
									pointDistance = curDistance;
									firstDistance = false;
								} else if (curDistance < pointDistance) {
									pointDistance = curDistance;
								}
							}
							if (pointDistance < apartDistance2) validPointGenerated = false;
							if (pointDistance > farDistance) {
								farDistance = pointDistance;
								farWorldLocation = worldLocation;
								farLocationPoint = locationPoint;
							}
						}
						if (validPointGenerated) break;
					}
					if (useApart && !validPointGenerated && (farDistance > 0.0f)) {
						worldLocation = farWorldLocation;
						locationPoint = farLocationPoint;
					}
					if (distinctOnly)
						localData->SetDistinctPoint(distinctIndex, locationPoint);
					if (deleteInvalidParticles && !validPointGenerated) 
						particlesToDelete.Set(i);
				}

				if (lockOnEmitter) { // write location info into channels for locking
					chRefNodeW->SetValue(i, locationPoint.init ? locationPoint.node : NULL);
					chLocalPosW->SetValue(i, locationPoint.location);
					chPointIndexW->SetValue(i, locationPoint.pointIndex);
				}
			} else { // lock for old points
				locationPoint.node = chRefNodeR->GetValue(i);
				locationPoint.location = chLocalPosR->GetValue(i);
				locationPoint.pointIndex = chPointIndexR->GetValue(i);
				locationPoint.init = true;
				nodeGeom = localData->GetNodeGeometry(locationPoint.node);
				if (nodeGeom != NULL)
					nodeGeom->GetLocationPoint(time, worldLocation, locationPoint.location, locationPoint.pointIndex);
			}

			chPosW->SetValue(i, worldLocation);

			// update speed
			if (lockOnEmitter || inheritSpeed) {
				Point3 speedVec(Point3::Origin);
				PreciseTimeValue otherTime = time;
				if (inheritSpeed && subframeSampling) { // calculate speed by as small interval as possible
					if (time <= timeEnd-2)
						otherTime = time + 2;
					else
						otherTime = time - 2;
				} else { // calculate speed by as big interval as possible
					if (time.TimeValue() == timeEnd.TimeValue())
						otherTime = timeStart;
					else
						otherTime = timeEnd;
				}
				Point3 otherLocation(Point3::Origin);
				float timeDif = float(time) - float(otherTime);
				if (timeDif != 0.0) {
					Point3 otherLocation(Point3::Origin);
					nodeGeom = localData->GetNodeGeometry(locationPoint.node);
					if (nodeGeom != NULL) {
						nodeGeom->GetLocationPoint(otherTime, otherLocation, locationPoint.location, locationPoint.pointIndex);
						speedVec = (worldLocation - otherLocation)/timeDif;
					}
				}
				if (inheritSpeed && !lockOnEmitter) {
					if (isInheritAmountAnimated) inheritAmount = GetPFFloat(pblock(), kPositionOnObject_inheritAmount, time);
					if (isInheritAmountVarAnimated) inheritAmountVar = GetPFFloat(pblock(), kPositionOnObject_inheritAmountVar, time);
					speedVec *= inheritAmount + inheritAmountVar*randGen->Rand11();
				}
				chSpeed->SetValue(i, speedVec);
			}
		}
	}

	if (deleteInvalidParticles && (particlesToDelete.NumberSet() > 0))
			chAmountW->Delete(particlesToDelete);

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

int PFOperatorPositionOnObject::getNumEmitters()
{
	int num = pblock()->Count(kPositionOnObject_emitters);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kPositionOnObject_emitters, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateEmittersInProgress = false;
bool PFOperatorPositionOnObject::updateFromRealEmitters()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kPositionOnObject_emitters, 
						kPositionOnObject_emittersMaxscript, gUpdateEmittersInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kPositionOnObject_emitters, _T("Emitter_Objects"));
	return res;
}

bool PFOperatorPositionOnObject::updateFromMXSEmitters()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kPositionOnObject_emitters, 
					kPositionOnObject_emittersMaxscript, gUpdateEmittersInProgress, this);
}

bool PFOperatorPositionOnObject::updateObjectNames(int pblockID)
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
int PFOperatorPositionOnObject::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFOperatorPositionOnObject::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kPositionOnObject_emitters, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}

ClassDesc* PFOperatorPositionOnObject::GetClassDesc() const
{
	return GetPFOperatorPositionOnObjectDesc();
}


} // end of namespace PFActions