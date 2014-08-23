/**********************************************************************
 *<
	FILE:			ParticleChannelAngAxis.cpp

	DESCRIPTION:	ParticleChannelAngAxis implementation
					This generic channel is used to store angle/axis data

	CREATED BY:		David C. Thompson

	HISTORY:		created 02-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelAngAxis.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

const AngAxis ParticleChannelAngAxis::m_defaultValue(Point3::XAxis, 0.0f);

ParticleChannelAngAxis::ParticleChannelAngAxis()
					:IParticleChannel(PARTICLECHANNELANGAXISR_INTERFACE,PARTICLECHANNELANGAXISW_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;

}

ParticleChannelAngAxis::~ParticleChannelAngAxis()
{
	; // implement later
}

TCHAR* ParticleChannelAngAxis::GetIObjectName()
{
	return _T("ParticleChannelAngAxis");
}

BaseInterface* ParticleChannelAngAxis::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelAngAxisR*)this;
	case 4: return (IParticleChannelAngAxisW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelAngAxis::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelAngAxisR*)this;
	if (id == GetWriteID())
		return (IParticleChannelAngAxisW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelAngAxis::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelAngAxis::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelAngAxis::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelAngAxisDesc TheParticleChannelAngAxisDesc;

static FPInterfaceDesc pc_AngAxis_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelAngAxisDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_AngAxis_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelAngAxisDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_AngAxis_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelAngAxisDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_AngAxis_AngAxisR(
			PARTICLECHANNELANGAXISR_INTERFACE,
			_T("angaxisRead"), 0,
			&TheParticleChannelAngAxisDesc, FP_MIXIN,

		IParticleChannelAngAxisR::kGetValue, _T("getValue"), 0, TYPE_ANGAXIS_BR, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelAngAxisR::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelAngAxisR::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_ANGAXIS_BR, 0, 0,

		end
);

static FPInterfaceDesc pc_AngAxis_AngAxisW(
			PARTICLECHANNELANGAXISW_INTERFACE,
			_T("angaxisWrite"), 0,
			&TheParticleChannelAngAxisDesc, FP_MIXIN,

		IParticleChannelAngAxisW::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_ANGAXIS_BR,
		IParticleChannelAngAxisW::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_ANGAXIS_BR,
		
		end
);

FPInterfaceDesc* ParticleChannelAngAxis::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_AngAxis_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_AngAxis_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_AngAxis_AmountW;
	if (id == GetReadID()) return &pc_AngAxis_AngAxisR;
	if (id == GetWriteID()) return &pc_AngAxis_AngAxisW;
	return NULL;
}

Class_ID ParticleChannelAngAxis::GetClassID() const
{
	return ParticleChannelAngAxis_Class_ID;
}

IObject*  ParticleChannelAngAxis::Clone() const
{
	ParticleChannelAngAxis* newChannel = (ParticleChannelAngAxis*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelAngAxis_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_data() = data();
	return newChannel;
}

IOResult ParticleChannelAngAxis::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int num;

	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = data().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0)
	{
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = isave->Write(data().Addr(0), sizeof(AngAxis)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	const AngAxis &v = globalValue();
	if ((res = isave->Write(&v, sizeof(AngAxis), &nb)) != IO_OK) return res;
	isave->EndChunk();

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

	isave->BeginChunk(IParticleChannel::kChunkValue1);
	bool isg = isGlobal();
	if ((res = isave->Write(&isg, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult ParticleChannelAngAxis::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	AngAxis v;
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
			res=iload->Read(_data().Addr(0), sizeof(AngAxis)*num, &nb);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&v, sizeof(AngAxis), &nb);
			if (res == IO_OK) _globalValue() = v;
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
		case IParticleChannel::kChunkValue1:
			res=iload->Read(&isg, sizeof(bool), &nb);
			if (res == IO_OK) _isGlobal() = isg;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelAngAxis::MemoryUsed() const
{
	int mem = sizeof(bool) + sizeof(int) + sizeof(AngAxis);
	mem += data().Count()*sizeof(AngAxis);
	return mem;
}

int ParticleChannelAngAxis::Count() const
{
	if (isGlobal()) return globalCount();
	return data().Count();
}

void ParticleChannelAngAxis::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else _data().ZeroCount();
}

bool ParticleChannelAngAxis::SetCount(int n)
{
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else _data().SetCount(n);
	return true;
}

int ParticleChannelAngAxis::Delete(int start, int num)
{
	if (start < 0) {
		num += start;
		start = 0;
	}
	if (num <= 0)	return Count();

	if (isGlobal())
	{
		if (start < globalCount())
		{
			if ((start+num) >= globalCount()) _globalCount() = start;
			else _globalCount() -= num;
		}
		return globalCount();
	}
	else
		return _data().Delete(start,num);
}

int ParticleChannelAngAxis::Delete(BitArray& toRemove)
{	
	int checkCount = min(toRemove.GetSize(), Count());

	if (isGlobal())
	{
		// find number of set bit in the "count" range
		int numRemove = 0;
		for(int i=0; i<checkCount; i++)
			if (toRemove[i] != 0) numRemove++;
		_globalCount() -= numRemove;
		return globalCount();
	}
	else
	{
		int i, j;
		for (i = j = 0; i < data().Count(); i++) {
			if (i < checkCount && toRemove[i] != 0)
				continue;
			if (i != j)	_data(j) = data(i);
			j++;
		}
		if (j < data().Count())
			_data().SetCount(j);
		return data().Count();
	}
}

IObject* ParticleChannelAngAxis::Split(BitArray& toSplit)
{
	ParticleChannelAngAxis* newChannel = (ParticleChannelAngAxis*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelAngAxis::Spawn(Tab<int>& spawnTable)
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	int i, checkCount = min(spawnTable.Count(), Count());

	if (isGlobal())
	{
		int newCount = 0;
		for(i=0; i<checkCount; i++)
			if (spawnTable[i] > 0) newCount += spawnTable[i];
		_globalCount() = newCount;
	}
	else
	{
		Tab<AngAxis> oldData(data());
		int j, k, newCount = 0;
		for(i=0; i<checkCount; i++)
			if (spawnTable[i] > 0) newCount += spawnTable[i];
		_data().SetCount(newCount);
		for(i=0, j=0; i<checkCount; i++)
			for(k=0; k<spawnTable[i]; k++)
				_data(j++) = oldData[i];
	}
	return true;
}

bool ParticleChannelAngAxis::AppendNum(int num)
{
	if (num <= 0)	return true;

	if (isGlobal())
	{
		_globalCount() += num;
		return true;
	}

	int oldCount = data().Count();
	int newCount = oldCount + num;

	if (_data().Resize(newCount) == 0) return false;
	_data().SetCount(newCount);

	for(int i=oldCount; i<newCount; i++)
		_data(i) = defaultValue();

	return true;
}

bool ParticleChannelAngAxis::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelAngAxisR* iAngAxis = (IParticleChannelAngAxisR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iAngAxis);
	if (iAngAxis == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iAngAxis->GetValue(i));

	return true;
}

const AngAxis& ParticleChannelAngAxis::GetValue(int index) const
{
	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

bool ParticleChannelAngAxis::IsGlobal() const
{
	return isGlobal();
}

const AngAxis& ParticleChannelAngAxis::GetValue() const
{
	if (isGlobal()) return globalValue();
	if (data().Count() <= 0) return defaultValue();
	return data(0);
}

void ParticleChannelAngAxis::SetValue(int index, const AngAxis& value)
{
	if (isGlobal())
	{
		// there is no comparison operator (==) for AngAxis
		if(Quat(value) == Quat(globalValue())) return;
		_isGlobal() = false;
		_data().SetCount(globalCount());
		for(int i=0; i<globalCount(); i++)
			_data(i) = globalValue();
	}
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = value;
}

void ParticleChannelAngAxis::SetValue(const AngAxis& value)
{
	if (!isGlobal())
	{
		_isGlobal() = true;
		_globalCount() = data().Count();
		_data().Resize(0);
	}
	_globalValue() = value;
}


} // end of namespace PFChannels