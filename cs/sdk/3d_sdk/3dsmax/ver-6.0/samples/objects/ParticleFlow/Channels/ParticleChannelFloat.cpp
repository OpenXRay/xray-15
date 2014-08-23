/**********************************************************************
 *<
	FILE:			ParticleChannelFloat.cpp

	DESCRIPTION:	ParticleChannelFloat implementation
					This generic channel is used to store floating point numbers

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelFloat.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelFloat::ParticleChannelFloat()
					:IParticleChannel(PARTICLECHANNELFLOATR_INTERFACE,PARTICLECHANNELFLOATW_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;
	_globalValue() = 0.0f;
}

ParticleChannelFloat::~ParticleChannelFloat()
{
	; // implement later
}

TCHAR* ParticleChannelFloat::GetIObjectName()
{
	return _T("ParticleChannelFloat");
}

BaseInterface* ParticleChannelFloat::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelFloatR*)this;
	case 4: return (IParticleChannelFloatW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelFloat::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelFloatR*)this;
	if (id == GetWriteID())
		return (IParticleChannelFloatW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelFloat::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelFloat::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelFloat::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelFloatDesc TheParticleChannelFloatDesc;

static FPInterfaceDesc pc_Float_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelFloatDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Float_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelFloatDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Float_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelFloatDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_Float_FloatR(
			PARTICLECHANNELFLOATR_INTERFACE,
			_T("floatRead"), 0,
			&TheParticleChannelFloatDesc, FP_MIXIN,

		IParticleChannelFloatR::kGetValue, _T("getValue"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelFloatR::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelFloatR::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_FLOAT, 0, 0,

		end
);

static FPInterfaceDesc pc_Float_FloatW(
			PARTICLECHANNELFLOATW_INTERFACE,
			_T("floatWrite"), 0,
			&TheParticleChannelFloatDesc, FP_MIXIN,

		IParticleChannelFloatW::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_FLOAT,
		IParticleChannelFloatW::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_FLOAT,
		
		end
);

FPInterfaceDesc* ParticleChannelFloat::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Float_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Float_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Float_AmountW;
	if (id == GetReadID()) return &pc_Float_FloatR;
	if (id == GetWriteID()) return &pc_Float_FloatW;
	return NULL;
}

Class_ID ParticleChannelFloat::GetClassID() const
{
	return ParticleChannelFloat_Class_ID;
}

IObject*  ParticleChannelFloat::Clone() const
{
	ParticleChannelFloat* newChannel = (ParticleChannelFloat*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelFloat_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelFloat::Save(ISave* isave) const
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
		if ((res = isave->Write(data().Addr(0), sizeof(float)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	float v = globalValue();
	if ((res = isave->Write(&v, sizeof(float), &nb)) != IO_OK) return res;
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

IOResult ParticleChannelFloat::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	float v;
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
			res=iload->Read(_data().Addr(0), sizeof(float)*num, &nb);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&v, sizeof(float), &nb);
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

int ParticleChannelFloat::MemoryUsed() const
{
	int mem = sizeof(bool) + sizeof(int) + sizeof(float);
	mem += data().Count()*sizeof(float);
	return mem;
}

int ParticleChannelFloat::Count() const
{
	if (isGlobal()) return globalCount();
	return data().Count();
}

void ParticleChannelFloat::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else _data().ZeroCount();
}

bool ParticleChannelFloat::SetCount(int n)
{
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else _data().SetCount(n);
	return true;
}

int ParticleChannelFloat::Delete(int start, int num)
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

int ParticleChannelFloat::Delete(BitArray& toRemove)
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

IObject* ParticleChannelFloat::Split(BitArray& toSplit)
{
	ParticleChannelFloat* newChannel = (ParticleChannelFloat*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelFloat::Spawn(Tab<int>& spawnTable)
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	int checkCount = min(spawnTable.Count(), Count());
	int newCount = 0;

	for(int i=0; i<checkCount; i++)
		if (spawnTable[i] > 0) newCount += spawnTable[i];

	if (isGlobal())
	{
		_globalCount() = newCount;
	}
	else
	{
		int j, k;
		Tab<float> oldData(data());
		_data().SetCount(newCount);
		for(i=0, j=0; i<checkCount; i++)
			for(k=0; k<spawnTable[i]; k++)
				_data(j++) = oldData[i];
	}
	return true;
}

bool ParticleChannelFloat::AppendNum(int num)
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
		_data(i) = 0.0f;

	return true;
}

bool ParticleChannelFloat::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelFloatR* iFloat = (IParticleChannelFloatR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iFloat);
	if (iFloat == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iFloat->GetValue(i));

	return true;
}

float ParticleChannelFloat::GetValue(int index) const
{
	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

bool ParticleChannelFloat::IsGlobal() const
{
	return isGlobal();
}

float ParticleChannelFloat::GetValue() const
{
	if (isGlobal()) return globalValue();
	if (data().Count() <= 0) return 0.0f;
	DbgAssert(data().Count()>0);
	return data(0);
}

void ParticleChannelFloat::SetValue(int index, float value)
{
	if (isGlobal())
	{
		if (value == globalValue()) return; // nothing to change
		_isGlobal() = false;
		_data().SetCount(globalCount());
		for(int i=0; i<globalCount(); i++)
			_data(i) = globalValue();
	}
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = value;
}

void ParticleChannelFloat::SetValue(float value)
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


