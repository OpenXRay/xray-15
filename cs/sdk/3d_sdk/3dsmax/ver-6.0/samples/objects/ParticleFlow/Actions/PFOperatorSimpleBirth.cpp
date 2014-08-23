/**********************************************************************
 *<
	FILE:			PFOperatorSimpleBirth.cpp

	DESCRIPTION:	SimpleBirth Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleBirth.h"

#include "PFOperatorSimpleBirth_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

#include "notify.h"

namespace PFActions {

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorSimpleBirthState				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorSimpleBirthState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimpleBirthState::IObject					 |
//+--------------------------------------------------------------------------+
void PFOperatorSimpleBirthState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimpleBirthState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorSimpleBirthState::GetClassID() 
{ 
	return PFOperatorSimpleBirthState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimpleBirthState::IPFActionState			 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorSimpleBirthState::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;

	isave->BeginChunk(IPFActionState::kChunkActionHandle);
	ULONG handle = actionHandle();
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData);
	float aL = accumLevel();
	if ((res = isave->Write(&aL, sizeof(float), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData2);
	int pG = particlesGenerated();
	if ((res = isave->Write(&pG, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorSimpleBirthState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorSimpleBirthState::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	float aL = 0.0f;
	int pG = 0;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IPFActionState::kChunkActionHandle:
			res=iload->Read(&_actionHandle(), sizeof(ULONG), &nb);
			break;

		case IPFActionState::kChunkData:
			aL = 0.0f;
			res=iload->Read(&aL, sizeof(float), &nb);
			if (res == IO_OK) _accumLevel() = aL;
			break;

		case IPFActionState::kChunkData2:
			pG = 0;
			res=iload->Read(&pG, sizeof(int), &nb);
			if (res == IO_OK) _particlesGenerated() = pG;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorSimpleBirth						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorSimpleBirth::PFOperatorSimpleBirth()
{ 
	invalidateTotalAmount();
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimpleBirth::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleBirth_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleBirth_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleBirth_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleBirth::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLEBIRTH_CLASS_NAME);
}

Class_ID PFOperatorSimpleBirth::ClassID()
{
	return PFOperatorSimpleBirth_Class_ID;
}

void PFOperatorSimpleBirth::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleBirth::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleBirth::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
												   PartID& partID, RefMessage message)
{
	TimeValue emitStart, emitFinish;
	switch (message)
	{
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				invalidateTotalAmount();
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSimpleBirth_type:
					RefreshBirthTypeUI();
					RefreshTotalAmountUI();
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kSimpleBirth_amount:
					if (pblock()->GetInt(kSimpleBirth_type) == kSimpleBirth_type_amount)
						RefreshTotalAmountUI();
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kSimpleBirth_start:
					emitStart = pblock()->GetTimeValue(kSimpleBirth_start, 0);
					emitFinish = pblock()->GetTimeValue(kSimpleBirth_finish, 0);
					if (emitStart > emitFinish)
						pblock()->SetValue(kSimpleBirth_finish, 0, emitStart);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					if (pblock()->GetInt(kSimpleBirth_type) == kSimpleBirth_type_rate)
						RefreshTotalAmountUI();
					break;
				case kSimpleBirth_finish:
					emitStart = pblock()->GetTimeValue(kSimpleBirth_start, 0);
					emitFinish = pblock()->GetTimeValue(kSimpleBirth_finish, 0);
					if (emitStart > emitFinish)
						pblock()->SetValue(kSimpleBirth_start, 0, emitFinish);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					if (pblock()->GetInt(kSimpleBirth_type) == kSimpleBirth_type_rate)
						RefreshTotalAmountUI();
					break;
				case kSimpleBirth_rate:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kSimpleBirth_subframe:
					if (pblock()->GetInt(kSimpleBirth_type) == kSimpleBirth_type_rate)
						RefreshTotalAmountUI();
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSimpleBirth_RefMsg_InitDlg:
			RefreshBirthTypeUI();
			RefreshTotalAmountUI();
			return REF_STOP;
	}
	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleBirth::Clone(RemapDir &remap)
{
	PFOperatorSimpleBirth* newOp = new PFOperatorSimpleBirth();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleBirth::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLEBIRTH_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFOperatorSimpleBirth::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_accumLevel(pCont) = 1.0f;
	_particlesGenerated(pCont) = 0;
	return true;
}

bool PFOperatorSimpleBirth::Release(IObject* pCont)
{
	return true;
}

const ParticleChannelMask& PFOperatorSimpleBirth::ChannelsUsed(const Interval& time) const
{
								//  read    &	write channels
	static ParticleChannelMask mask(PCU_Amount, PCU_Amount|PCU_New|PCU_ID|PCU_Time|PCU_BirthTime);
	return mask;
}

const Interval PFOperatorSimpleBirth::ActivityInterval() const
{
	Interval interval;
	interval.SetStart(pblock()->GetTimeValue(kSimpleBirth_start, 0));
	interval.SetEnd(pblock()->GetTimeValue(kSimpleBirth_finish, 0));
	return interval;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorSimpleBirth::GetCurrentState(IObject* pContainer)
{
	PFOperatorSimpleBirthState* actionState = (PFOperatorSimpleBirthState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorSimpleBirthState_Class_ID);
	actionState->_accumLevel() = _accumLevel(pContainer);
	actionState->_particlesGenerated() = _particlesGenerated(pContainer);
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorSimpleBirth::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorSimpleBirthState* actionState = (PFOperatorSimpleBirthState*)aSt;
	_accumLevel(pContainer) = actionState->accumLevel();
	_particlesGenerated(pContainer) = actionState->particlesGenerated();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleBirth::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEBIRTH_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleBirth::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEBIRTH_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleBirth::HasDynamicName(TSTR& nameSuffix)
{
	int tpf = GetTicksPerFrame();
	int start	= pblock()->GetTimeValue(kSimpleBirth_start, 0)/tpf;
	int finish	= pblock()->GetTimeValue(kSimpleBirth_finish, 0)/tpf;
	TCHAR buf[64];
	if (start == finish) {
		nameSuffix = GetString(IDS_AT);
		sprintf(buf, " %d ", start);
		nameSuffix += buf;
	} else {
		sprintf(buf, "%d-%d ", start, finish);
		nameSuffix = TSTR(buf);
	}
	nameSuffix += GetString(IDS_TOTAL_ABBR);
	sprintf(buf, ":%d", EmitTotalAmount());
	nameSuffix += buf;
	return true;
}

void ClampToFrame(PreciseTimeValue& time, int tpf)
{
	double fTime = double(time.tick) + time.fraction;
	double inFrameTime = fTime/tpf;
	if (inFrameTime >= 0.0)
		time = PreciseTimeValue(tpf*int(ceil(inFrameTime + 0.5)));
	else
		time = PreciseTimeValue(-tpf*int(ceil(-inFrameTime + 0.5)));
}

void ClampToHalfFrame(PreciseTimeValue& time, int tpf)
{
	PreciseTimeValue frameTime = time;
	ClampToFrame(frameTime, tpf);
	if (float(time - frameTime) >= 0.0f)
		frameTime.tick += tpf/2;
	else
		frameTime.tick -= tpf - tpf/2;
	time = frameTime;
}

bool PFOperatorSimpleBirth::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart,	// interval start to produce particles (exclusive)
									 PreciseTimeValue& timeEnd,		// interval end to produce particles (inclusive)
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	IParticleObjectExt* poExt = GetParticleObjectExtInterface(GetPFObject(pSystem));
	if (poExt == NULL) return false; // no handle for ParticleObjectExt interface
	IPFSystem* pfSys = GetPFSystemInterface(pSystem);
	if (pfSys == NULL) return false; // no handle for PFSystem interface

	int numParticles = poExt->NumParticles();
	int numParticlesGen = poExt->NumParticlesGenerated();
	int particlesLimit = pfSys->GetBornAllowance();
	if (numParticlesGen >= particlesLimit) return true; // limit has been reached already
	float mult = pfSys->GetMultiplier(timeStart);
	if (mult <= 0.0f)	return true;
	float indexRatio = 1.0f/mult;

	int tpf = GetTicksPerFrame();
	TimeValue tvTimeStart = TimeValue(timeStart);
	// birth time of the first particle
	TimeValue emitStart = pblock()->GetTimeValue(kSimpleBirth_start, tvTimeStart);
	// birth time of the last particle
	TimeValue emitFinish = pblock()->GetTimeValue(kSimpleBirth_finish, tvTimeStart);
	if (emitFinish < emitStart) return true;

	// check if the proceed interval falls out of the scope
	if (timeStart > PreciseTimeValue(emitFinish+tpf)) return true;
	if (timeEnd < PreciseTimeValue(emitStart-tpf)) return true;

	int birthType = pblock()->GetInt(kSimpleBirth_type, tvTimeStart);
	bool useSubframe = (pblock()->GetInt(kSimpleBirth_subframe, tvTimeStart) != 0);
	bool isRateAnimated = false;
	float birthRate = GetPFFloat(pblock(), kSimpleBirth_rate, tvTimeStart);
	if (birthType == kSimpleBirth_type_rate) {
		Control* rateControl = pblock()->GetController(kSimpleBirth_rate);
		if (rateControl != NULL)
			if (rateControl->IsAnimated() != 0)
				isRateAnimated = true;
		if (!isRateAnimated && (birthRate <= 0.0f)) return true;
	}

	int i, j, num, amount = EmitTotalAmount();
	// total amount after taking into account the multiplier coefficient
	int multAmount = (int)floor(mult * amount + 0.5f);
	if (multAmount <= 0) return true; // nothing to be born

	PreciseTimeValue originalTimeStart = timeStart;
	PreciseTimeValue originalTimeEnd = timeEnd;
	if (!useSubframe) { // clamp the proceed start and finish to the closest half-frame value
		PreciseTimeValue clampTimeStart = timeStart;
		PreciseTimeValue clampTimeEnd = timeEnd;
		ClampToHalfFrame(clampTimeStart, tpf);
		ClampToHalfFrame(clampTimeEnd, tpf);
		if (clampTimeStart == clampTimeEnd) return true; // no whole frame values in between timeStart and timeEnd
		timeStart = clampTimeStart;
		timeEnd = clampTimeEnd;
	}

	Tab<PreciseTimeValue> time;
	// if a single particle to be born by amount type then
	// adjust emitFinish to emitStart to fall into the category of "impulse" birth
	if ((birthType == kSimpleBirth_type_amount) && (multAmount == 1)) emitFinish = emitStart;

	if (emitFinish == emitStart) { // single birth impulse
		// check if the birth impulse falls into the proceed interval
		PreciseTimeValue birthMoment(emitFinish);
		if ((birthMoment > timeStart) && (birthMoment <= timeEnd)) {
			if (birthType == kSimpleBirth_type_rate) multAmount = 1;
			if ((multAmount+numParticlesGen) > particlesLimit) 
				multAmount = particlesLimit - numParticlesGen;			
			time.SetCount(multAmount);
			for(i=0; i<multAmount; i++) time[i] = birthMoment;
		}
	} else { // birth by amount
		if (birthType == kSimpleBirth_type_amount) {
			double birthInterval = double(emitFinish - emitStart)/(multAmount-1);
			double fGeneratedBefore = floor((timeStart.tick - emitStart + double(timeStart.fraction))/birthInterval);
			int generatedBefore = (fGeneratedBefore >= 0.0) ? int(fGeneratedBefore)+1 : 0;
			if (generatedBefore > multAmount) generatedBefore = multAmount;
			double fGeneratedAfter = floor((timeEnd.tick - emitStart + double(timeStart.fraction))/birthInterval);
			int generatedAfter = (fGeneratedAfter >= 0.0) ? int(fGeneratedAfter)+1 : 0;
			if (generatedAfter > multAmount) generatedAfter = multAmount;
			num = generatedAfter - generatedBefore;
			if ((num+numParticlesGen) > particlesLimit) num = particlesLimit - numParticlesGen;			
			if (num > 0) {
				PreciseTimeValue firstTime = PreciseTimeValue(emitStart) + PreciseTimeValue(float(generatedBefore*birthInterval));
				time.SetCount(num);
				for(i=0; i<num; i++)
					time[i] = firstTime + PreciseTimeValue(float(i*birthInterval));
			}
		} else { // birth by rate
			if (isRateAnimated) { // non-constant rate of birth
				int alreadyGenerated = _particlesGenerated(pCont);
				double curAccum = _accumLevel(pCont);
				PreciseTimeValue curTime = PreciseTimeValue(emitStart);
				if (curTime < timeStart) curTime = timeStart;
				PreciseTimeValue curNextClick = curTime;
				curNextClick.fraction = 0;
				curNextClick.tick += 1;
				TimeValue curTick = curTime.tick;
				if (!useSubframe) {
					PreciseTimeValue adjCurTime = curTime;
					ClampToFrame(adjCurTime, tpf);
					curTick = adjCurTime.tick;
				}
				float curRate = GetPFFloat(pblock(), kSimpleBirth_rate, curTick);
				double accumRate = mult*double(curRate)/TIME_TICKSPERSEC;
				PreciseTimeValue timeGenEnd = PreciseTimeValue(emitFinish);
				if (timeEnd < timeGenEnd) timeGenEnd = timeEnd;
				while (curTime <= timeGenEnd) {
					if ((time.Count() + numParticlesGen) >= particlesLimit) break;
					if (time.Count() + alreadyGenerated >= multAmount) break;
					if (curAccum >= 1.0) { // a particle is ripe to harvest
						time.Append(1, &curTime, 1000);
						curAccum -= 1.0;
					}
					if (accumRate <= 0.0) { // nothing to produce in this tick but may be next?
						curTime = curNextClick;
						curTime.fraction = -0.5f;
						curNextClick.tick += 1;
						TimeValue newCurTick = curTime.tick;
						if (!useSubframe) {
							PreciseTimeValue adjCurTime = curTime;
							ClampToFrame(adjCurTime, tpf);
							newCurTick = adjCurTime.tick;
						}
						if (newCurTick != curTick) {
							curTick = newCurTick;
							curRate = GetPFFloat(pblock(), kSimpleBirth_rate, curTick);
							accumRate = mult*double(curRate)/TIME_TICKSPERSEC;
						}
						continue;
					}
					double deficit = 1.0f - curAccum;
					double timeToNext = deficit/accumRate;
					if ((curTime + PreciseTimeValue(float(timeToNext))) > timeGenEnd) {
						// next particle generated falls out of the current proceed interval
						curAccum += float(timeGenEnd - curTime)*accumRate;
						break;
					}
					bool inTheSameTick = ((curTime.fraction + timeToNext) <= 0.5);
					if (inTheSameTick) {
						curTime.fraction += float(timeToNext);
						time.Append(1, &curTime, 1000);
						curAccum = 0.0;
					} else {
						curAccum += (0.5 - curTime.fraction)*accumRate;
						curTime = curNextClick;
						curTime.fraction = -0.5f;
						curNextClick.tick += 1;
						TimeValue newCurTick = curTime.tick;
						if (!useSubframe) {
							PreciseTimeValue adjCurTime = curTime;
							ClampToFrame(adjCurTime, tpf);
							newCurTick = adjCurTime.tick;
						}
						if (newCurTick != curTick) {
							curTick = newCurTick;
							curRate = GetPFFloat(pblock(), kSimpleBirth_rate, curTick);
							accumRate = mult*double(curRate)/TIME_TICKSPERSEC;
						}
					}
				}
				if (timeEnd >= PreciseTimeValue(emitFinish)) { // last chance to add particles
					int amountToAdd = multAmount - alreadyGenerated - time.Count();
					if (amountToAdd > 0) {
						for(i=0; i<amountToAdd; i++)
							time.Append(1, &timeEnd, amountToAdd);
					}
				}
				_accumLevel(pCont) = float(curAccum);
				_particlesGenerated(pCont) = alreadyGenerated + time.Count();
			} else { // constant birth rate
				double birthInterval = double(TIME_TICKSPERSEC)/(birthRate*mult);
				double fGeneratedBefore = floor((timeStart.tick - emitStart + double(timeStart.fraction))/birthInterval);
				int generatedBefore = (fGeneratedBefore >= 0.0) ? int(fGeneratedBefore)+1 : 0;
				if (generatedBefore > multAmount) generatedBefore = multAmount;
				double fGeneratedAfter = floor((timeEnd.tick - emitStart + double(timeStart.fraction))/birthInterval);
				int generatedAfter = (fGeneratedAfter >= 0.0) ? int(fGeneratedAfter)+1 : 0;
				if (generatedAfter > multAmount) generatedAfter = multAmount;
				num = generatedAfter - generatedBefore;
				if ((num+numParticlesGen) > particlesLimit) num = particlesLimit - numParticlesGen;			
				if (num > 0) {
					PreciseTimeValue firstTime = PreciseTimeValue(emitStart) + PreciseTimeValue(float(generatedBefore*birthInterval));
					time.SetCount(num);
					for(i=0; i<num; i++)
						time[i] = firstTime + PreciseTimeValue(float(i*birthInterval));
				}
			}
		}
	}
	
	timeEnd = originalTimeEnd;
	timeStart = originalTimeStart;

	int count = time.Count();
	if (count == 0) return true;
	if (!useSubframe) { // clamp resulting time to the closest frame value
		for(i=0; i<count; i++)
			ClampToFrame(time[i], tpf);
	}
	Tab<int> index;
	Tab<int> born;
	index.SetCount(count);
	born.SetCount(count);
	for(i=0; i<count; i++) {
		index[i] = int(floor((numParticlesGen+i)*indexRatio + 0.5f));
		born[i] = numParticlesGen + i;
	}
	
	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmountR = GetParticleChannelAmountRInterface(pCont);
	if (chAmountR == NULL) return false; // can't find number of particles in the container
	IParticleChannelAmountW* chAmountW = GetParticleChannelAmountWInterface(pCont);
	if (chAmountW == NULL) return false; // can't change number of particles in the container

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	IParticleChannelNewW* chNew = (IParticleChannelNewW*)chCont->EnsureInterface(PARTICLECHANNELNEWW_INTERFACE, 
																	ParticleChannelNew_Class_ID,
																	false, Interface_ID(0,0), Interface_ID(0,0) );
	if (chNew == NULL) return false;

	IParticleChannelPTVW* chTime = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELTIMEW_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELTIMER_INTERFACE,
																	PARTICLECHANNELTIMEW_INTERFACE);
	if (chTime == NULL) return false;

	IParticleChannelIDW* chID = (IParticleChannelIDW*)chCont->EnsureInterface(PARTICLECHANNELIDW_INTERFACE,
																	ParticleChannelID_Class_ID,
																	false, Interface_ID(0,0), Interface_ID(0,0) );
	if (chID == NULL) return false;

	IParticleChannelPTVW* chBirthTime = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELBIRTHTIMEW_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELBIRTHTIMER_INTERFACE,
																	PARTICLECHANNELBIRTHTIMEW_INTERFACE);
	if (chBirthTime == NULL) return false;

	IParticleChannelBoolW* chSelect = NULL;
	if (pfSys->NumParticlesSelected() > 0)
		chSelect = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELSELECTIONW_INTERFACE,
																	ParticleChannelBool_Class_ID,
																	true, PARTICLECHANNELSELECTIONR_INTERFACE,
																	PARTICLECHANNELSELECTIONW_INTERFACE);
	// modify channels to include new born particles
	int oldAmount = chAmountR->Count();
	if (!chAmountW->AppendNum(count)) return false; // can't append new particles
	for(i=oldAmount, j=0; i<oldAmount+count; i++, j++)
	{
		chNew->SetNew(i);
		chTime->SetValue(i, time[j]);
		chID->SetID(i, index[j], born[j]);
		chBirthTime->SetValue(i, time[j]);
		if (chSelect != NULL)
			chSelect->SetValue(i, pfSys->IsParticleSelected(born[j]));
	}

	return true;
}

ClassDesc* PFOperatorSimpleBirth::GetClassDesc() const
{
	return GetPFOperatorSimpleBirthDesc();
}

int PFOperatorSimpleBirth::EmitTotalAmount() const
{
	if (emitTotalAmount() >= 0) return emitTotalAmount();

	int amount = 0;

	switch (pblock()->GetInt(kSimpleBirth_type))
	{
	case kSimpleBirth_type_amount:
		amount = GetPFInt( pblock(), kSimpleBirth_amount, 0);
		break;
	case kSimpleBirth_type_rate:
	{
		TimeValue emitStart = pblock()->GetTimeValue(kSimpleBirth_start);
		TimeValue emitFinish = pblock()->GetTimeValue(kSimpleBirth_finish);
		bool useSubframe = (pblock()->GetInt(kSimpleBirth_subframe, 0) != 0);
		if (emitFinish < emitStart) break;
		bool hasNonZeroRate = false;
		bool isRateAnimated = false;
		Control* ctrl = pblock()->GetController(kSimpleBirth_rate);
		if (ctrl != NULL) isRateAnimated = (ctrl->IsAnimated() != 0);
		double totalAmount = 0.0;
		float rate = 0.0f;
		if (isRateAnimated) {
			int tpf = GetTicksPerFrame();
			int numKeys = ctrl->NumKeys();
			if (numKeys == NOT_KEYFRAMEABLE) {
				for(TimeValue t = emitStart; t < emitFinish; t += 1) {
					if (useSubframe) {
						rate = GetPFFloat(pblock(), kSimpleBirth_rate, t);
					} else {
						PreciseTimeValue getTime = PreciseTimeValue(t);
						ClampToFrame(getTime, tpf);
						rate = GetPFFloat(pblock(), kSimpleBirth_rate, getTime.TimeValue());
					}
					totalAmount += rate / TIME_TICKSPERSEC;
					if (rate > 0.0f) hasNonZeroRate = true;
				}
			} else {
				TimeValue firstKeyAt = ctrl->GetKeyTime(0);
				if (firstKeyAt < emitStart) firstKeyAt = emitStart;
				if (useSubframe) {
					rate = GetPFFloat(pblock(), kSimpleBirth_rate, firstKeyAt);
				} else {
					PreciseTimeValue getTime = PreciseTimeValue(firstKeyAt);
					ClampToFrame(getTime, tpf);
					rate = GetPFFloat(pblock(), kSimpleBirth_rate, getTime.TimeValue());
				}
				totalAmount += (firstKeyAt - emitStart)*rate/TIME_TICKSPERSEC;
				if (rate > 0.0f) hasNonZeroRate = true;

				TimeValue lastKeyAt = ctrl->GetKeyTime(numKeys-1);
				if (lastKeyAt > emitFinish) lastKeyAt = emitFinish;
				if (useSubframe) {
					rate = GetPFFloat(pblock(), kSimpleBirth_rate, lastKeyAt);
				} else {
					PreciseTimeValue getTime = PreciseTimeValue(lastKeyAt);
					ClampToFrame(getTime, tpf);
					rate = GetPFFloat(pblock(), kSimpleBirth_rate, getTime.TimeValue());
				}
				totalAmount += (emitFinish - lastKeyAt)*rate/TIME_TICKSPERSEC;
				if (rate > 0.0f) hasNonZeroRate = true;

				for(TimeValue t = firstKeyAt; t < lastKeyAt; t += 1) {
					if (useSubframe) {
						rate = GetPFFloat(pblock(), kSimpleBirth_rate, t);
					} else {
						PreciseTimeValue getTime = PreciseTimeValue(t);
						ClampToFrame(getTime, tpf);
							rate = GetPFFloat(pblock(), kSimpleBirth_rate, getTime.TimeValue());
					}
					totalAmount += rate / TIME_TICKSPERSEC;
					if (rate > 0.0f) hasNonZeroRate = true;
				}
			}
			if (useSubframe)
				totalAmount += 0.5*double(GetPFFloat(pblock(), kSimpleBirth_rate, emitFinish) 
							- GetPFFloat(pblock(), kSimpleBirth_rate, emitStart)) / TIME_TICKSPERSEC;
		} else {
			rate = pblock()->GetFloat(kSimpleBirth_rate, 0);
			totalAmount = (rate*(emitFinish-emitStart))/TIME_TICKSPERSEC;
			hasNonZeroRate = (rate >= 0.0f);
		}
		if (totalAmount > double(kSimpleBirth_amountMax)) totalAmount = kSimpleBirth_amountMax;
		amount = int(ceil(totalAmount));
		if ((double(amount) == totalAmount) && hasNonZeroRate) amount += 1;
	}
		break;
	}

	if (amount > kSimpleBirth_amountMax) amount = kSimpleBirth_amountMax;
	m_emitTotalAmount = amount;
	return amount;
}

void PFOperatorSimpleBirth::RefreshBirthTypeUI() const
{
	if ( pblock() == NULL ) return;

	IParamMap2* map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	int birthType = pblock()->GetInt(kSimpleBirth_type);

	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, kSimpleBirth_message_type, (LPARAM)birthType );
		}
	}

	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		if (hWnd)
			SendMessage( hWnd, WM_COMMAND, kSimpleBirth_message_type, (LPARAM)birthType );
	}
}

void PFOperatorSimpleBirth::RefreshTotalAmountUI() const
{
	if ( pblock() == NULL ) return;

	IParamMap2* map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	int amount = EmitTotalAmount();

	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, kSimpleBirth_message_amount, (LPARAM)amount );
		}
	}

	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		if (hWnd)
		SendMessage( hWnd, WM_COMMAND, kSimpleBirth_message_amount, (LPARAM)amount );
	}
}




} // end of namespace PFActions