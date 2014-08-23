/**********************************************************************
 *<
	FILE:			PFOperatorSimpleScale.cpp

	DESCRIPTION:	SimpleScale Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 07-22-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleScale.h"

#include "PFOperatorSimpleScale_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

#include "decomp.h"

namespace PFActions {

// SimpleScale creates a particle channel to store an initial scale value for each particles
// The channel is created for RelativeFirst type only to store the original scale when a 
// particle enters the event
#define PARTICLECHANNELINITIALSCALER_INTERFACE Interface_ID(0x239d1166, 0x1eb34500) 
#define PARTICLECHANNELINITIALSCALEW_INTERFACE Interface_ID(0x239d1166, 0x1eb34501) 

#define GetParticleChannelInitialScaleRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELINITIALSCALER_INTERFACE)) 
#define GetParticleChannelInitialScaleWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELINITIALSCALEW_INTERFACE)) 

// SimpleScale creates a particle channel to store a random vector for each particle
// when using scale variation
// This way the random variation value is calculated only once when a particle enters the event.
// The channel is created for Absolute, RelativeFirst and RelativeSuccessive types
#define PARTICLECHANNELRNDVECTORR_INTERFACE Interface_ID(0x28b8709e, 0x1eb34500) 
#define PARTICLECHANNELRNDVECTORW_INTERFACE Interface_ID(0x28b8709e, 0x1eb34501) 

#define GetParticleChannelRndVectorRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELRNDVECTORR_INTERFACE)) 
#define GetParticleChannelRndVectorWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELRNDVECTORW_INTERFACE)) 


PFOperatorSimpleScale::PFOperatorSimpleScale()
{ 
	_calculatingConstrain() = false;
	_registeredTimeChangeCallback() = false;
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimpleScale::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleScale_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleScale_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleScale_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleScale::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLESCALE_CLASS_NAME);
}

Class_ID PFOperatorSimpleScale::ClassID()
{
	return PFOperatorSimpleScale_Class_ID;
}

void PFOperatorSimpleScale::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleScale::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleScale::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kSimpleScale_Type:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kSimpleScale_XScaleFactor:
					XScaleChanged();
					break;
				case kSimpleScale_YScaleFactor:
					YScaleChanged();
					break;
				case kSimpleScale_ZScaleFactor:
					ZScaleChanged();
					break;
				case kSimpleScale_XScaleVariation:
					XScaleVarChanged();
					break;
				case kSimpleScale_YScaleVariation:
					YScaleVarChanged();
					break;
				case kSimpleScale_ZScaleVariation:
					ZScaleVarChanged();
					break;
				case kSimpleScale_sfConstrain:
				case kSimpleScale_svConstrain:
					UpdatePViewUI(lastUpdateID);
					return REF_STOP;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case kSimpleScale_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kSimpleScale_RefMsg_InitDialog:
			DuplicateScale(GetCOREInterface()->GetTime());
			DuplicateScaleVar(GetCOREInterface()->GetTime());
			if (!registeredTimeChangeCallback()) {
				GetCOREInterface()->RegisterTimeChangeCallback(this);
				_registeredTimeChangeCallback() = true;
			}
			return REF_STOP;
		case kSimpleScale_RefMsg_DestroyDialog:
			if (registeredTimeChangeCallback()) {
				GetCOREInterface()->UnRegisterTimeChangeCallback(this);
				_registeredTimeChangeCallback() = false;
			}
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleScale::Clone(RemapDir &remap)
{
	PFOperatorSimpleScale* newOp = new PFOperatorSimpleScale();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleScale::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLESCALE_OBJECT_NAME);
}

void PFOperatorSimpleScale::TimeChanged(TimeValue t)
{
	DuplicateScale(t);
	DuplicateScaleVar(t);
}

const ParticleChannelMask& PFOperatorSimpleScale::ChannelsUsed(const Interval& time) const
{
									// read										& write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart, PCU_Scale);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		// private channels
		mask.AddChannel(PARTICLECHANNELINITIALSCALER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELINITIALSCALEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELRNDVECTORR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELRNDVECTORW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleScale::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESCALE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleScale::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESCALE_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleScale::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kSimpleScale_Type, 0);
	switch(type) {
	case kSS_OverwriteOnce:
		nameSuffix = GetString(IDS_OVERWRITE_ONCE);
		break;
	case kSS_InheritOnce:
		nameSuffix = GetString(IDS_INHERIT_ONCE);
		break;
	case kSS_Absolute:
		nameSuffix = GetString(IDS_ABSOLUTE);
		break;
	case kSS_RelativeFirst:
		nameSuffix = GetString(IDS_RELATIVE_FIRST);
		break;
	case kSS_RelativeSuccessive:
		nameSuffix = GetString(IDS_RELATIVE_SUCCESSIVE);
		break;
	}
	return true;
}

// detect presence of this operator in the action list
bool PFOperatorSimpleScale::Init(IObject* pCont, Object* pSystem,
								 INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	Class_ID testCID = this->ClassID();

	bool bRet = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);

	bAbsPresent = bRFPresent = bRSPresent = false;
	Interval valid;
	int iType = -1;
	int iCount = 0;

	if((iCount = actions.Count()) > 0) {
		for(int i = 0; i < iCount; i++) {
			Object* po = actions[i];
			if (po == this) break; // check the action in the beginning of the chain only
			if(testCID == po->ClassID()) {
				PFOperatorSimpleScale* poss = (PFOperatorSimpleScale*)po;
				IParamBlock2* ppb = poss->GetParamBlock(0);
				if(ppb) {
					ppb->GetValue(kSimpleScale_Type, 0, iType, valid);
					if(iType == kSS_Absolute)
						bAbsPresent = true;
					if(iType == kSS_RelativeFirst)
						bRFPresent = true;
					if(iType == kSS_RelativeSuccessive)
						bRSPresent = true;
				}
			}
		}
	}

	return bRet;
}

float ssGenerateRandomValue(RandGenerator* prg, int biasType)
{
	float randValue = 0.0f;
	switch(biasType) {
	case kSS_None: 
		randValue = prg->Rand11(); 
		break;
	case kSS_Centered:
		randValue = prg->Rand11(); randValue += prg->Rand11();
		randValue += prg->Rand11(); randValue = 0.25f*(randValue+prg->Rand11());
		break;
	case kSS_TowardsMin:
		randValue = 2.0f*prg->Rand01(); randValue *= prg->Rand01();
		randValue = randValue*prg->Rand01() - 1.0f;
		break;
	case kSS_TowardsMax:
		randValue = 2.0f*prg->Rand01();
		randValue = 1.0f - randValue*prg->Rand01();
		break;
	}
	return randValue;
}

Point3 ssGenerateRandomVector(RandGenerator* prg, int biasType, bool constrainProportion)
{
	Point3 randVector;
	randVector.x = ssGenerateRandomValue(prg, biasType);
	if (constrainProportion) {
		randVector.z = randVector.y = randVector.x;
	} else {
		randVector.y = ssGenerateRandomValue(prg, biasType);
		randVector.z = ssGenerateRandomValue(prg, biasType);
	}
	return randVector;
}

bool PFOperatorSimpleScale::Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd,
									 Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	int iQuant = chAmount->Count();
	if(iQuant < 1) return true; // no particles to modify

	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if(chNew == NULL) return false;
	int scaleType = pblock()->GetInt(kSimpleScale_Type, timeStart);
	bool proceedNewOnly = ((scaleType == kSS_OverwriteOnce) || (scaleType == kSS_InheritOnce));
	if (proceedNewOnly) {
		// there are no new particles; for OverwriteOnce and InheritOnce
		// the operator modifies the new particles only; no need to proceed
		if (chNew->IsAllOld()) return true;
	}

	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;

	int syncType = pblock()->GetInt(kSimpleScale_Sync, timeStart);
	IParticleChannelPTVR* chBirthTime = NULL;
	if (syncType == kSS_ParticleAge) {
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if(chBirthTime == NULL) return false;
	}

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (syncType == kSS_EventDuration) {
		chEventStartR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																		actionNode, NULL, &initEventStart);
		if (chEventStartR == NULL) return false; // can't read event start time
		if (initEventStart) {
			chEventStartW = GetParticleChannelEventStartWInterface(pCont);
			if (chEventStartW == NULL) return false; // can't write event start time
		}
	}

	// the channel of interest
	bool bInitScale = false;
	IParticleChannelPoint3R* chScaleR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSCALER_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSCALER_INTERFACE,
																PARTICLECHANNELSCALEW_INTERFACE, true,
																actionNode, NULL, &bInitScale);
	if(chScaleR == NULL) return false;

	IParticleChannelPoint3W* chScaleW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSCALER_INTERFACE,
																			PARTICLECHANNELSCALEW_INTERFACE, true );
	if (chScaleW == NULL) return false;

	bool useInitialScale = (scaleType == kSS_RelativeFirst);
	bool initInitialScale = false;
	IParticleChannelPoint3R* chInitialScaleR = NULL;
	IParticleChannelPoint3W* chInitialScaleW = NULL;
	if (useInitialScale) {
		chInitialScaleR = (IParticleChannelPoint3R*) chCont->EnsureInterface(PARTICLECHANNELINITIALSCALER_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELINITIALSCALER_INTERFACE,
																	PARTICLECHANNELINITIALSCALEW_INTERFACE, true,
																	actionNode, (Object*)this, &initInitialScale);
		if (chInitialScaleR == NULL) return false; // can't read/create InitialScale channel
		if (initInitialScale) {
			chInitialScaleW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELINITIALSCALEW_INTERFACE, (Object*)this);
			if (chInitialScaleW == NULL) return false; // can't init InitialScale channel
		}
	}

	bool useScaleVar = ((scaleType == kSS_Absolute) || (scaleType == kSS_RelativeFirst) 
													|| (scaleType == kSS_RelativeSuccessive));
	bool initRndVector = false;
	IParticleChannelPoint3R* chRndVectorR = NULL;
	IParticleChannelPoint3W* chRndVectorW = NULL;
	if (useScaleVar) {
		chRndVectorR = (IParticleChannelPoint3R*) chCont->EnsureInterface(PARTICLECHANNELRNDVECTORR_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELRNDVECTORR_INTERFACE,
																	PARTICLECHANNELRNDVECTORW_INTERFACE, true,
																	actionNode, (Object*)this, &initRndVector);
		if (chRndVectorR == NULL) return false; // can't read/create RndVector channel
		if (initRndVector) {
			chRndVectorW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELRNDVECTORW_INTERFACE, (Object*)this);
			if (chRndVectorW == NULL) return false; // can't init RndFloat channel
		}
	}

	RandGenerator* prg = randLinker().GetRandGenerator(pCont);

	Control* paramCtrl = pblock()->GetController(kSimpleScale_XScaleFactor);
	bool xScaleAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float xScale = xScaleAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_XScaleFactor, timeStart);

	paramCtrl = pblock()->GetController(kSimpleScale_YScaleFactor);
	bool yScaleAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float yScale = yScaleAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_YScaleFactor, timeStart);

	paramCtrl = pblock()->GetController(kSimpleScale_ZScaleFactor);
	bool zScaleAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float zScale = zScaleAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_ZScaleFactor, timeStart);

	paramCtrl = pblock()->GetController(kSimpleScale_XScaleVariation);
	bool xVarAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float xVar = xVarAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_XScaleVariation, timeStart);

	paramCtrl = pblock()->GetController(kSimpleScale_YScaleVariation);
	bool yVarAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float yVar = yVarAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_YScaleVariation, timeStart);

	paramCtrl = pblock()->GetController(kSimpleScale_ZScaleVariation);
	bool zVarAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float zVar = zVarAnimated ? 1.0f : GetPFFloat(pblock(), kSimpleScale_ZScaleVariation, timeStart);

	int biasType = pblock()->GetInt(kSimpleScale_svBias, timeStart);
	bool constrainProportion = (pblock()->GetInt(kSimpleScale_svConstrain, timeStart) != 0);
	bool isRelativeScale = ((scaleType == kSS_InheritOnce)
							|| (scaleType == kSS_RelativeFirst)
							|| (scaleType == kSS_RelativeSuccessive));
	
	// scale each particle
	for(int i = 0; i < iQuant; i++) {
		PreciseTimeValue syncTime;
		PreciseTimeValue time = chTime->GetValue(i);
		Point3 randVector;
		Point3 initialScale;

		if (chNew->IsNew(i)) {
			if (initEventStart)
				chEventStartW->SetValue(i, time);

			randVector = ssGenerateRandomVector(prg, biasType, constrainProportion);
			if (initRndVector) 
				chRndVectorW->SetValue(i, randVector);
			else if (chRndVectorR) randVector = chRndVectorR->GetValue(i);

			if (bInitScale)
				chScaleW->SetValue(i, Point3(1.0f, 1.0f, 1.0f));
			if (initInitialScale)
				chInitialScaleW->SetValue(i, chScaleR->GetValue(i));
		} else {
			if (proceedNewOnly) continue;	
			DbgAssert(chRndVectorR);
			if (chRndVectorR) randVector = chRndVectorR->GetValue(i);
			else randVector = Point3::Origin;
		}

		switch(syncType) {
		case kSS_AbsoluteTime:
			syncTime = time;
			break;
		case kSS_ParticleAge:
			syncTime = time - chBirthTime->GetValue(i);
			break;
		case kSS_EventDuration:
			syncTime = time - chEventStartR->GetValue(i);
			break;
		}

		if (xScaleAnimated) xScale = GetPFFloat(pblock(), kSimpleScale_XScaleFactor, syncTime);
		if (yScaleAnimated) yScale = GetPFFloat(pblock(), kSimpleScale_YScaleFactor, syncTime);
		if (zScaleAnimated) zScale = GetPFFloat(pblock(), kSimpleScale_ZScaleFactor, syncTime);

		if (xVarAnimated) xVar = GetPFFloat(pblock(), kSimpleScale_XScaleVariation, syncTime);		
		if (yVarAnimated) yVar = GetPFFloat(pblock(), kSimpleScale_ZScaleVariation, syncTime);		
		if (zVarAnimated) zVar = GetPFFloat(pblock(), kSimpleScale_ZScaleVariation, syncTime);		
		
		float curXVar = ((randVector.x < 0.0f) && (xVar > xScale)) ? xScale : xVar;
		float curYVar = ((randVector.y < 0.0f) && (yVar > yScale)) ? yScale : yVar;
		float curZVar = ((randVector.z < 0.0f) && (zVar > zScale)) ? zScale : zVar;

		Point3 newScale = Point3(xScale + curXVar*randVector.x,
								yScale + curYVar*randVector.y,
								zScale + curZVar*randVector.z);
		if (isRelativeScale) {
			Point3 oldScale = useInitialScale ? chInitialScaleR->GetValue(i) : chScaleR->GetValue(i);
			newScale = Point3(oldScale.x*newScale.x, oldScale.y*newScale.y, oldScale.z*newScale.z);
		}
		chScaleW->SetValue(i, newScale);
	}

	return true;
}

ClassDesc* PFOperatorSimpleScale::GetClassDesc() const
{
	return GetPFOperatorSimpleScaleDesc();
}

int PFOperatorSimpleScale::GetRand()
{
	return pblock()->GetInt(kSimpleScale_Seed);
}

void PFOperatorSimpleScale::SetRand(int seed)
{
	_pblock()->SetValue(kSimpleScale_Seed, 0, seed);
}

bool PFOperatorSimpleScale::DoScaleConstrain()
{
	if (!theHold.Holding()) return false;
	if (calculatingConstrain()) return false;
	return (pblock()->GetInt(kSimpleScale_sfConstrain, 0) != 0);
}

void PFOperatorSimpleScale::XScaleChanged()
{
	if (DoScaleConstrain()) { 
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleDup().x;
		float valueNew = pblock()->GetFloat(kSimpleScale_XScaleFactor, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_YScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_YScaleFactor, time) + valueNew);
			pblock()->SetValue(kSimpleScale_ZScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_ZScaleFactor, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_YScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_YScaleFactor, time));
			pblock()->SetValue(kSimpleScale_ZScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_ZScaleFactor, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScale(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::YScaleChanged()
{
	if (DoScaleConstrain()) { 
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleDup().y;
		float valueNew = pblock()->GetFloat(kSimpleScale_YScaleFactor, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_XScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_XScaleFactor, time) + valueNew);
			pblock()->SetValue(kSimpleScale_ZScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_ZScaleFactor, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_XScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_XScaleFactor, time));
			pblock()->SetValue(kSimpleScale_ZScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_ZScaleFactor, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScale(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::ZScaleChanged()
{
	if (DoScaleConstrain()) { 
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleDup().z;
		float valueNew = pblock()->GetFloat(kSimpleScale_ZScaleFactor, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_XScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_XScaleFactor, time) + valueNew);
			pblock()->SetValue(kSimpleScale_YScaleFactor, time, 
					pblock()->GetFloat(kSimpleScale_YScaleFactor, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_XScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_XScaleFactor, time));
			pblock()->SetValue(kSimpleScale_YScaleFactor, time, 
					factor*pblock()->GetFloat(kSimpleScale_YScaleFactor, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScale(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::DuplicateScale(TimeValue time)
{
	Point3 scale;
	scale.x = pblock()->GetFloat(kSimpleScale_XScaleFactor, time);
	scale.y = pblock()->GetFloat(kSimpleScale_YScaleFactor, time);
	scale.z = pblock()->GetFloat(kSimpleScale_ZScaleFactor, time);
	_scaleDup() = scale;
}

bool PFOperatorSimpleScale::DoScaleVarConstrain()
{
	if (!theHold.Holding()) return false;
	if (calculatingConstrain()) return false;
	return (pblock()->GetInt(kSimpleScale_svConstrain, 0) != 0);
}

void PFOperatorSimpleScale::XScaleVarChanged()
{
	if (DoScaleVarConstrain()) {
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleVarDup().x;
		float valueNew = pblock()->GetFloat(kSimpleScale_XScaleVariation, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_YScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_YScaleVariation, time) + valueNew);
			pblock()->SetValue(kSimpleScale_ZScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_ZScaleVariation, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_YScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_YScaleVariation, time));
			pblock()->SetValue(kSimpleScale_ZScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_ZScaleVariation, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScaleVar(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::YScaleVarChanged()
{
	if (DoScaleVarConstrain()) {
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleVarDup().y;
		float valueNew = pblock()->GetFloat(kSimpleScale_YScaleVariation, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_XScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_XScaleVariation, time) + valueNew);
			pblock()->SetValue(kSimpleScale_ZScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_ZScaleVariation, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_XScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_XScaleVariation, time));
			pblock()->SetValue(kSimpleScale_ZScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_ZScaleVariation, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScaleVar(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::ZScaleVarChanged()
{
	if (DoScaleVarConstrain()) {
		_calculatingConstrain() = true;
		TimeValue time = GetCOREInterface()->GetTime();	
		float valueOld = scaleVarDup().z;
		float valueNew = pblock()->GetFloat(kSimpleScale_ZScaleVariation, time);
		if (valueOld == 0.0f) {
			pblock()->SetValue(kSimpleScale_XScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_XScaleVariation, time) + valueNew);
			pblock()->SetValue(kSimpleScale_YScaleVariation, time, 
					pblock()->GetFloat(kSimpleScale_YScaleVariation, time) + valueNew);
		} else {
			float factor = valueNew/valueOld;
			pblock()->SetValue(kSimpleScale_XScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_XScaleVariation, time));
			pblock()->SetValue(kSimpleScale_YScaleVariation, time, 
					factor*pblock()->GetFloat(kSimpleScale_YScaleVariation, time));
		}
		_calculatingConstrain() = false;
	}
	DuplicateScaleVar(GetCOREInterface()->GetTime());
}

void PFOperatorSimpleScale::DuplicateScaleVar(TimeValue time)
{
	Point3 var;
	var.x = pblock()->GetFloat(kSimpleScale_XScaleVariation, time);
	var.y = pblock()->GetFloat(kSimpleScale_YScaleVariation, time);
	var.z = pblock()->GetFloat(kSimpleScale_ZScaleVariation, time);
	_scaleVarDup() = var;
}

} // end of namespace PFActions