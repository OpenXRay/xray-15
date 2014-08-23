/**********************************************************************
 *<
	FILE: ParticleChannelPTV.cpp

	DESCRIPTION: ParticleChannelPTV implementation
				 This generic channel is used to store time data
				 For precision reasons the time is kept in two
				 components: TimeValue FrameTime and float FractionTime ]-0.5, 0.5]
				 The result time is a sum of these two components.
				 In genenal, Operators only read from the channel.
				 The Birth Operator sets time according to the time a
				 particle is born. To advance particle in time, Integrator-type
				 operator is used.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelPTV.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelPTV::ParticleChannelPTV()
					:IParticleChannel(PARTICLECHANNELPTVR_INTERFACE,PARTICLECHANNELPTVW_INTERFACE)
{
	;
}

ParticleChannelPTV::~ParticleChannelPTV()
{
	; // implement later
}

TCHAR* ParticleChannelPTV::GetIObjectName()
{
	return _T("ParticleChannelPTV");
}

BaseInterface* ParticleChannelPTV::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelPTVR*)this;
	case 4: return (IParticleChannelPTVW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelPTV::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelPTVR*)this;
	if (id == GetWriteID())
		return (IParticleChannelPTVW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelPTV::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelPTV::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelPTV::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelPTVDesc TheParticleChannelPTVDesc;

static FPInterfaceDesc pc_PTV_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelPTVDesc, FP_MIXIN, 
			
		IParticleChannel::kClone, _T("clone"), 0, TYPE_IOBJECT, 0, 0,
		IParticleChannel::kIsSimilarChannel, _T("isSimilarChannel"), 0, TYPE_bool, 0, 1,
			_T("channel"), 0, TYPE_IOBJECT,
		IParticleChannel::kIsTransferable, _T("isTransferable"), 0, TYPE_bool, 0, 0,
		IParticleChannel::kSetTransferable, _T("setTransferable"), 0, TYPE_VOID, 0, 1,
			_T("status"), 0, TYPE_bool,
		IParticleChannel::kIsPrivateChannel, _T("isPrivateChannel"), 0, TYPE_bool, 0, 0,
		IParticleChannel::kGetPrivateOwner, _T("getPrivateOwner"), 0, TYPE_OBJECT, 0, 0,
		IParticleChannel::kSetPrivateOwner, _T("setPrivateOwner"), 0, TYPE_VOID, 0, 1,
			_T("action"), 0, TYPE_OBJECT,
		IParticleChannel::kGetCreatorAction, _T("getCreatorAction"), 0, TYPE_INODE, 0, 0,
		IParticleChannel::kSetCreatorAction, _T("setCreatorAction"), 0, TYPE_VOID, 0, 1,
			_T("action"), 0, TYPE_INODE,
		IParticleChannel::kGetReadID_PartA, _T("getReadID_PartA"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetReadID_PartB, _T("getReadID_PartB"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetWriteID_PartA, _T("getWriteID_PartA"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetWriteID_PartB, _T("getWriteID_PartB"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kSetReadID, _T("setReadID"), 0, TYPE_VOID, 0, 2,
			_T("partA"), 0, TYPE_DWORD,
			_T("partB"), 0, TYPE_DWORD,
		IParticleChannel::kSetWriteID, _T("setWriteID"), 0, TYPE_VOID, 0, 2,
			_T("partA"), 0, TYPE_DWORD,
			_T("partB"), 0, TYPE_DWORD,

		end
); 

static FPInterfaceDesc pc_PTV_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelPTVDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_PTV_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelPTVDesc, FP_MIXIN,

		IParticleChannelAmountW::kZeroCount, _T("zeroCount"), 0, TYPE_VOID, 0, 0,
		IParticleChannelAmountW::kSetCount, _T("setCount"), 0, TYPE_bool, 0, 1,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kDeleteByIndex, _T("delete"), 0, TYPE_INT, 0, 2,
			_T("firstIndex"), 0, TYPE_INT,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kDeleteByArray, _T("delete"), 0, TYPE_INT, 0, 1,
			_T("toDelete"), 0, TYPE_BITARRAY_BR,
		IParticleChannelAmountW::kSplit, _T("split"), 0, TYPE_IOBJECT, 0, 1,
			_T("toSplit"), 0, TYPE_BITARRAY_BR,
		IParticleChannelAmountW::kSpawn, _T("spawn"), 0, TYPE_bool, 0, 1,
			_T("toSpawn"), 0, TYPE_INT_TAB_BR,
		IParticleChannelAmountW::kAppendNum, _T("appendNum"), 0, TYPE_bool, 0, 1,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kAppend, _T("append"), 0, TYPE_bool, 0, 1,
			_T("channel"), 0, TYPE_IOBJECT,

		end
);

static FPInterfaceDesc pc_PTV_PTVR(
			PARTICLECHANNELPTVR_INTERFACE,
			_T("preciseTimeValueRead"), 0,
			&TheParticleChannelPTVDesc, FP_MIXIN,

		IParticleChannelPTVR::kGetTick, _T("getTick"), 0, TYPE_TIMEVALUE, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelPTVR::kGetFraction, _T("getFraction"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelPTVR::kGetTickFraction, _T("getTickFraction"), 0, TYPE_VOID, 0, 3,
			_T("index"), 0, TYPE_INT,
			_T("tick"), 0, TYPE_TIMEVALUE,
			_T("fraction"), 0, TYPE_FLOAT,
		IParticleChannelPTVR::kIsSync, _T("isSync"), 0, TYPE_bool, 0, 0,

		end
);

static FPInterfaceDesc pc_PTV_PTVW(
			PARTICLECHANNELPTVW_INTERFACE,
			_T("preciseTimeValueWrite"), 0,
			&TheParticleChannelPTVDesc, FP_MIXIN,

		IParticleChannelPTVW::kSetTick, _T("setTick"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("tick"), 0, TYPE_TIMEVALUE,
		IParticleChannelPTVW::kSetFraction, _T("setFraction"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("fraction"), 0, TYPE_FLOAT,
		IParticleChannelPTVW::kSetTickFraction, _T("setTickFraction"), 0, TYPE_VOID, 0, 3,
			_T("index"), 0, TYPE_INT,
			_T("tick"), 0, TYPE_TIMEVALUE,
			_T("fraction"), 0, TYPE_FLOAT,

		end
);

FPInterfaceDesc* ParticleChannelPTV::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_PTV_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_PTV_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_PTV_AmountW;
	if (id == GetReadID()) return &pc_PTV_PTVR;
	if (id == GetWriteID()) return &pc_PTV_PTVW;
	return NULL;
}

Class_ID ParticleChannelPTV::GetClassID() const
{
	return ParticleChannelPTV_Class_ID;
}

IObject*  ParticleChannelPTV::Clone() const
{
	ParticleChannelPTV* newChannel = (ParticleChannelPTV*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelPTV_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelPTV::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int i, num;

	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = data().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0)
	{
		PreciseTimeValue* ptvData = new PreciseTimeValue[num];
		for(i=0; i<num; i++) ptvData[i] = data(i);
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = isave->Write(ptvData, sizeof(PreciseTimeValue)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
		delete [] ptvData;
	}

	Interface_ID id;
	isave->BeginChunk(IParticleChannel::kChunkReadID);
	id = GetReadID();
	if ((res = isave->Write(&id, sizeof(Interface_ID), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkWriteID);
	id = GetWriteID();
	if ((res = isave->Write(&id, sizeof(Interface_ID), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkTransferable);
	bool isTransferable = IsTransferable();
	if ((res = isave->Write(&isTransferable, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkPrivate);
	bool isPrivate = IsPrivateChannel();
	if ((res = isave->Write(&isPrivate, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkActionHandle);
	ULONG handle = m_creatorHandle;
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult ParticleChannelPTV::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num;
	PreciseTimeValue* ptvData;
	Interface_ID id;
	bool isg;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _data().SetCount(num);
			break;
		case IParticleChannel::kChunkData:
			num = data().Count();
			DbgAssert(num > 0);
			ptvData = new PreciseTimeValue[num];
			res=iload->Read(ptvData, sizeof(PreciseTimeValue)*num, &nb);
			if (res == IO_OK)
				for(i=0; i<num; i++)
					_data(i) = ptvData[i];
			delete [] ptvData;
			ptvData = NULL;
			break;
		case IParticleChannel::kChunkReadID:
			res=iload->Read(&id, sizeof(Interface_ID), &nb);
			if (res == IO_OK) SetReadID(id);
			break;
		case IParticleChannel::kChunkWriteID:
			res=iload->Read(&id, sizeof(Interface_ID), &nb);
			if (res == IO_OK) SetWriteID(id);
			break;
		case IParticleChannel::kChunkTransferable:
			res=iload->Read(&isg, sizeof(bool), &nb);
			if (res == IO_OK) SetTransferable(isg);
			break;
		case IParticleChannel::kChunkPrivate:
			res=iload->Read(&m_isPrivate, sizeof(bool), &nb);
			break;
		case IParticleChannel::kChunkActionHandle:
			res=iload->Read(&m_creatorHandle, sizeof(ULONG), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelPTV::MemoryUsed() const
{
	int mem = data().Count()*sizeof(PreciseTimeValue);
	return mem;
}

int ParticleChannelPTV::Count() const
{
	return data().Count();
}

void ParticleChannelPTV::ZeroCount()
{
	_data().ZeroCount();
}

bool ParticleChannelPTV::SetCount(int n)
{
	if (n < 0) return false;
	_data().SetCount(n);
	return true;
}

int ParticleChannelPTV::Delete(int start, int num)
{
	return _data().Delete(start,num);
}

int ParticleChannelPTV::Delete(BitArray& toRemove)
{	
	SysUtil::NeedToImplementLater(); // optimize the implementation

	Tab<PreciseTimeValue> oldData(data());

	int newCount = 0;
	int checkCount = min(toRemove.GetSize(), data().Count());
	for(int i=0; i<checkCount; i++)
		if (toRemove[i] == 0)
			_data(newCount++) = oldData[i];
	_data().Resize(newCount);	
	return data().Count();
}

IObject* ParticleChannelPTV::Split(BitArray& toSplit)
{
	ParticleChannelPTV* newChannel = (ParticleChannelPTV*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelPTV::Spawn( Tab<int>& spawnTable )
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	Tab<PreciseTimeValue> oldData(data());
	int i, j, k, newCount = 0, checkCount = min(spawnTable.Count(), data().Count());
	for(i=0; i<checkCount; i++)
		newCount += spawnTable[i];
	_data().SetCount(newCount);
	for(i=0, j=0; i<checkCount; i++)
		for(k=0; k<spawnTable[i]; k++)
			_data(j++) = oldData[i];
	return true;
}

bool ParticleChannelPTV::AppendNum(int num)
{
	if (num <= 0) return true;

	int oldCount = data().Count();
	int newCount = oldCount + num;

	if (_data().Resize(newCount) == 0) return false;
	_data().SetCount(newCount);

	for(int i=oldCount; i<newCount; i++)
		_data(i) = PreciseTimeValue::Origin;

	return true;
}

bool ParticleChannelPTV::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelPTVR* iPTV = (IParticleChannelPTVR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iPTV);
	if (iPTV == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iPTV->GetValue(i) );

	return true;
}

TimeValue ParticleChannelPTV::GetTick(int index) const
{
	DbgAssert((index>=0) && (index<data().Count()));
	return (TimeValue)data(index).tick;
}

float ParticleChannelPTV::GetFraction(int index) const
{
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index).fraction;
}

void ParticleChannelPTV::GetTickFraction(int index, TimeValue& tick, float& fraction) const
{
	DbgAssert((index>=0) && (index<data().Count()));
	tick = data(index).tick;
	fraction = data(index).fraction;
}

const PreciseTimeValue& ParticleChannelPTV::GetValue(int index) const
{
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

const PreciseTimeValue& ParticleChannelPTV::GetValue() const
{
	return data(0);
}

bool ParticleChannelPTV::IsSync() const
{
	if (data().Count() == 0) return true;
	PreciseTimeValue time = data(0);
	for(int i=1; i<data().Count(); i++)
		if (!time.Equals(data(i))) return false;
	return true;
}

void ParticleChannelPTV::SetTick(int index, TimeValue time)
{
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index).tick = time;
}

void ParticleChannelPTV::SetFraction(int index, float time)
{
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index).fraction = time;
}

void ParticleChannelPTV::SetTickFraction(int index, TimeValue tick, float fraction)
{
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = PreciseTimeValue(tick, fraction);
}

void ParticleChannelPTV::SetValue(int index, const PreciseTimeValue& time)
{
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = time;
}

void ParticleChannelPTV::SetValue(const PreciseTimeValue& time)
{
	for(int i=0; i<data().Count(); i++)
		_data(i) = time;
}


} // end of namespace PFChannels


