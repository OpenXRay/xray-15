/**********************************************************************
 *<
	FILE:			ParticleChannelBool.cpp

	DESCRIPTION:	ParticleChannelBool implementation
					This generic channel is used to store bool data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-20-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelBool.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelBool::ParticleChannelBool()
					:IParticleChannel(PARTICLECHANNELBOOLR_INTERFACE,PARTICLECHANNELBOOLW_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;
	_globalValue() = false;
}

ParticleChannelBool::~ParticleChannelBool()
{
	; // implement later
}

TCHAR* ParticleChannelBool::GetIObjectName()
{
	return _T("ParticleChannelBool");
}

BaseInterface* ParticleChannelBool::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelBoolR*)this;
	case 4: return (IParticleChannelBoolW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelBool::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelBoolR*)this;
	if (id == GetWriteID())
		return (IParticleChannelBoolW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelBool::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelBool::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelBool::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelBoolDesc TheParticleChannelBoolDesc;

static FPInterfaceDesc pc_Bool_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelBoolDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Bool_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelBoolDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Bool_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelBoolDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_Bool_BoolR(
			PARTICLECHANNELBOOLR_INTERFACE,
			_T("boolRead"), 0,
			&TheParticleChannelBoolDesc, FP_MIXIN,

		IParticleChannelBoolR::kGetValue, _T("getValue"), 0, TYPE_bool, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelBoolR::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelBoolR::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_bool, 0, 0,

		end
);

static FPInterfaceDesc pc_Bool_BoolW(
			PARTICLECHANNELBOOLW_INTERFACE,
			_T("boolWrite"), 0,
			&TheParticleChannelBoolDesc, FP_MIXIN,

		IParticleChannelBoolW::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_bool,
		IParticleChannelBoolW::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_bool,
		
		end
);

FPInterfaceDesc* ParticleChannelBool::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Bool_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Bool_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Bool_AmountW;
	if (id == GetReadID()) return &pc_Bool_BoolR;
	if (id == GetWriteID()) return &pc_Bool_BoolW;
	return NULL;
}

Class_ID ParticleChannelBool::GetClassID() const
{
	return ParticleChannelBool_Class_ID;
}

IObject*  ParticleChannelBool::Clone() const
{
	ParticleChannelBool* newChannel = (ParticleChannelBool*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelBool_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelBool::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int num;

	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = data().GetSize();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0)
	{
		isave->BeginChunk(IParticleChannel::kChunkData);
		BitArray &bitData = const_cast <BitArray&> (data());
		if ((res = bitData.Save(isave)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	bool v = globalValue();
	if ((res = isave->Write(&v, sizeof(bool), &nb)) != IO_OK) return res;
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

IOResult ParticleChannelBool::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	bool v;
	Interface_ID id;
	bool isg;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _data().SetSize(num);
			break;
		case IParticleChannel::kChunkData:
			num = data().GetSize();
			DbgAssert(num > 0);
			res=_data().Load(iload);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&v, sizeof(bool), &nb);
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

int ParticleChannelBool::MemoryUsed() const
{
	int mem = sizeof(bool)*2 + sizeof(int);
	mem += data().GetSize()/8; // assuming 8 bits per byte.
	return mem;
}

int ParticleChannelBool::Count() const
{
	if (isGlobal()) return globalCount();
	return data().GetSize();
}

void ParticleChannelBool::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else _data().SetSize(0);
}

bool ParticleChannelBool::SetCount(int n)
{
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else _data().SetSize(n, TRUE);
	return true;
}

int ParticleChannelBool::Delete(int start, int num)
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
	{
		int size = data().GetSize();
		if (start < size)
		{
			if ((start+num) >= size) {
				size = start;
			} else {
				_data().Shift(LEFT_BITSHIFT, num, start);
				size -= num;
			}
			_data().SetSize(size, TRUE);
		}
		return size;
	}
}

int ParticleChannelBool::Delete(BitArray& toRemove)
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
		int size = data().GetSize();
		for (i = j = 0; i < size; i++) {
			if (i < checkCount && toRemove[i] != 0)
				continue;
			if (i != j)	_data().Set(j, data(i));
			j++;
		}
		if (j < size)
			_data().SetSize(j, TRUE);
		return data().GetSize();
	}
}

IObject* ParticleChannelBool::Split(BitArray& toSplit)
{
	ParticleChannelBool* newChannel = (ParticleChannelBool*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelBool::Spawn(Tab<int>& spawnTable)
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
		int j, k, v;
		BitArray oldData(data());
		_data().SetSize(newCount);
		for(i=0, j=0; i<checkCount; i++) {
			v = oldData[i];
			for(k=0; k<spawnTable[i]; k++)
				_data().Set(j++, v);
		}
	}
	return true;
}

bool ParticleChannelBool::AppendNum(int num)
{
	if (num <= 0)	return true;

	if (isGlobal())
	{
		_globalCount() += num;
		return true;
	}

	int oldCount = data().GetSize();
	int newCount = oldCount + num;

	_data().SetSize(newCount, TRUE);

	for(int i=oldCount; i<newCount; i++)
		_data().Clear(i);

	return true;
}

bool ParticleChannelBool::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelBoolR* iBool = (IParticleChannelBoolR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iBool);
	if (iBool == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iBool->GetValue(i));

	return true;
}

bool ParticleChannelBool::GetValue(int index) const
{
	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().GetSize()));
	return data(index);
}

bool ParticleChannelBool::IsGlobal() const
{
	return isGlobal();
}

bool ParticleChannelBool::GetValue() const
{
	if (isGlobal()) return globalValue();
	if (data().GetSize() <= 0) return false;
	DbgAssert(data().GetSize()>0);
	return data(0);
}

void ParticleChannelBool::SetValue(int index, bool value)
{
	if (isGlobal())
	{
		if (value == globalValue()) return; // nothing to change
		_isGlobal() = false;
		_data().SetSize(globalCount());
		if (globalValue())
			_data().SetAll();
		else
			_data().ClearAll();
	}
	DbgAssert((index>=0) && (index<data().GetSize()));
	_data().Set(index, value);
}

void ParticleChannelBool::SetValue(bool value)
{
	if (!isGlobal())
	{
		_isGlobal() = true;
		_globalCount() = data().GetSize();
		_data().SetSize(0);
	}
	_globalValue() = value;
}


} // end of namespace PFChannels


