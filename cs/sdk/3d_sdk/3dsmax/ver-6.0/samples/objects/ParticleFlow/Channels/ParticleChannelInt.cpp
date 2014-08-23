/**********************************************************************
 *<
	FILE:			ParticleChannelInt.cpp

	DESCRIPTION:	ParticleChannelInt implementation
					This generic channel is used to store integer data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelInt.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelInt::ParticleChannelInt()
					:IParticleChannel(PARTICLECHANNELINTR_INTERFACE,PARTICLECHANNELINTW_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;
	_globalValue() = 0;
}

ParticleChannelInt::~ParticleChannelInt()
{
	; // implement later
}

TCHAR* ParticleChannelInt::GetIObjectName()
{
	return _T("ParticleChannelInt");
}

BaseInterface* ParticleChannelInt::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelIntR*)this;
	case 4: return (IParticleChannelIntW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelInt::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelIntR*)this;
	if (id == GetWriteID())
		return (IParticleChannelIntW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelInt::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelInt::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelInt::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelIntDesc TheParticleChannelIntDesc;

static FPInterfaceDesc pc_Int_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelIntDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Int_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelIntDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Int_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelIntDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_Int_IntR(
			PARTICLECHANNELINTR_INTERFACE,
			_T("intRead"), 0,
			&TheParticleChannelIntDesc, FP_MIXIN,

		IParticleChannelIntR::kGetValue, _T("getValue"), 0, TYPE_INT, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelIntR::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelIntR::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Int_IntW(
			PARTICLECHANNELINTW_INTERFACE,
			_T("intWrite"), 0,
			&TheParticleChannelIntDesc, FP_MIXIN,

		IParticleChannelIntW::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_INT,
		IParticleChannelIntW::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_INT,
		
		end
);

FPInterfaceDesc* ParticleChannelInt::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Int_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Int_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Int_AmountW;
	if (id == GetReadID()) return &pc_Int_IntR;
	if (id == GetWriteID()) return &pc_Int_IntW;
	return NULL;
}

Class_ID ParticleChannelInt::GetClassID() const
{
	return ParticleChannelInt_Class_ID;
}

IObject*  ParticleChannelInt::Clone() const
{
	ParticleChannelInt* newChannel = (ParticleChannelInt*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelInt_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelInt::Save(ISave* isave) const
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
		if ((res = isave->Write(data().Addr(0), sizeof(int)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	int v = globalValue();
	if ((res = isave->Write(&v, sizeof(int), &nb)) != IO_OK) return res;
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

IOResult ParticleChannelInt::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num, v;
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
			res=iload->Read(_data().Addr(0), sizeof(int)*num, &nb);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&v, sizeof(int), &nb);
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

int ParticleChannelInt::MemoryUsed() const
{
	int mem = sizeof(bool) + sizeof(int)*2;
	mem += data().Count()*sizeof(int);
	return mem;
}

int ParticleChannelInt::Count() const
{
	if (isGlobal()) return globalCount();
	return data().Count();
}

void ParticleChannelInt::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else _data().ZeroCount();
}

bool ParticleChannelInt::SetCount(int n)
{
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else _data().SetCount(n);
	return true;
}

int ParticleChannelInt::Delete(int start, int num)
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

int ParticleChannelInt::Delete(BitArray& toRemove)
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

IObject* ParticleChannelInt::Split(BitArray& toSplit)
{
	ParticleChannelInt* newChannel = (ParticleChannelInt*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelInt::Spawn(Tab<int>& spawnTable)
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
		Tab<int> oldData(data());
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

bool ParticleChannelInt::AppendNum(int num)
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
		_data(i) = 0;

	return true;
}

bool ParticleChannelInt::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelIntR* iInt = (IParticleChannelIntR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iInt);
	if (iInt == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iInt->GetValue(i));

	return true;
}

int ParticleChannelInt::GetValue(int index) const
{
	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

bool ParticleChannelInt::IsGlobal() const
{
	return isGlobal();
}

int ParticleChannelInt::GetValue() const
{
	if (isGlobal()) return globalValue();
	if (data().Count() <= 0) return 0;
	DbgAssert(data().Count()>0);
	return data(0);
}

void ParticleChannelInt::SetValue(int index, int value)
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

void ParticleChannelInt::SetValue(int value)
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


