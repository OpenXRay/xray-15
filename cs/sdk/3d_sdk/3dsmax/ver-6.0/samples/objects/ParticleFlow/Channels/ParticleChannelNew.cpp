/**********************************************************************
 *<
	FILE: ParticleChannelNew.cpp

	DESCRIPTION: ParticleChannelNew implementation
				 the channel is used to mark particles that are just
				 come to the current actionList (either via birth or a jump from
				 another actionList

	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-08-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelNew.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelNew::ParticleChannelNew()
					:IParticleChannel(PARTICLECHANNELNEWR_INTERFACE,PARTICLECHANNELNEWW_INTERFACE)
{
	_globalCount() = 0;
	_allNew() = _allOld() = false;
}

ParticleChannelNew::~ParticleChannelNew()
{
	; // implement later!
}

TCHAR* ParticleChannelNew::GetIObjectName()
{
	return _T("ParticleChannelNew");
}

BaseInterface* ParticleChannelNew::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelNewR*)this;
	case 4: return (IParticleChannelNewW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelNew::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == PARTICLECHANNELNEWR_INTERFACE)
		return (IParticleChannelNewR*)this;
	if (id == PARTICLECHANNELNEWW_INTERFACE)
		return (IParticleChannelNewW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelNew::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelNew::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelNew::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelNewDesc TheParticleChannelNewDesc;

static FPInterfaceDesc pc_New_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelNewDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_New_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelNewDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_New_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelNewDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_New_NewR(
			PARTICLECHANNELNEWR_INTERFACE,
			_T("newRead"), 0,
			&TheParticleChannelNewDesc, FP_MIXIN,

		particleChannelNew_isNew, _T("isNew"), 0, TYPE_bool, 0, 1,
			_T("index"), 0, TYPE_INT,
		particleChannelNew_isAllNew, _T("isAllNew"), 0, TYPE_bool, 0, 0,
		particleChannelNew_isAllOld, _T("isAllOld"), 0, TYPE_bool, 0, 0,

		end
);

static FPInterfaceDesc pc_New_NewW(
			PARTICLECHANNELNEWW_INTERFACE,
			_T("newWrite"), 0,
			&TheParticleChannelNewDesc, FP_MIXIN,

		particleChannelNew_setNew, _T("setNew"), 0, TYPE_VOID, 0, 1,
			_T("index"), 0, TYPE_INT,
		particleChannelNew_setOld, _T("setOld"), 0, TYPE_VOID, 0, 1,
			_T("index"), 0, TYPE_INT,
		particleChannelNew_setAllNew, _T("setAllNew"), 0, TYPE_VOID, 0, 0,
		particleChannelNew_setAllOld, _T("setAllOld"), 0, TYPE_VOID, 0, 0,

		end
);


FPInterfaceDesc* ParticleChannelNew::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_New_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_New_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_New_AmountW;
	if (id == GetReadID()) return &pc_New_NewR;
	if (id == GetWriteID()) return &pc_New_NewW;
	return NULL;
}

Class_ID ParticleChannelNew::GetClassID() const
{
	return ParticleChannelNew_Class_ID;
}

IObject*  ParticleChannelNew::Clone() const
{
	ParticleChannelNew* newChannel = (ParticleChannelNew*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelNew_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_allNew() = allNew();
	newChannel->_allOld() = allOld();
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelNew::Save(ISave* isave) const
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
		bool* boolData = new bool[num];
		for(i=0; i<num; i++) boolData[i] = data(i);
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = isave->Write(boolData, sizeof(bool)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
		delete [] boolData;
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
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

	bool allValue;

	isave->BeginChunk(IParticleChannel::kChunkValue1);
	allValue = allNew();
	if ((res = isave->Write(&allValue, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkValue2);
	allValue = allOld();
	if ((res = isave->Write(&allValue, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult ParticleChannelNew::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num;
	bool* boolData;
	bool allValue;
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
			boolData = new bool[num];
			res=iload->Read(boolData, sizeof(bool)*num, &nb);
			if (res == IO_OK)
				for(i=0; i<num; i++)
					_data(i) = boolData[i];
			delete [] boolData;
			boolData = NULL;
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
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
			res=iload->Read(&allValue, sizeof(bool), &nb);
			if (res == IO_OK) _allNew() = allValue;
			break;
		case IParticleChannel::kChunkValue2:
			res=iload->Read(&allValue, sizeof(bool), &nb);
			if (res == IO_OK) _allOld() = allValue;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelNew::MemoryUsed() const
{
	int mem = sizeof(int) + sizeof(bool)*2;
	mem += data().Count()*sizeof(bool);
	return mem;
}

int ParticleChannelNew::Count() const
{
	if (allNew() || allOld()) return globalCount();
	return data().Count();
}

void ParticleChannelNew::ZeroCount()
{
	if (allNew() || allOld()) _globalCount()=0;
	else _data().ZeroCount();
}

bool ParticleChannelNew::SetCount(int n)
{
	if (n < 0) return false;
	if (allNew() || allOld()) _globalCount()=n;
	else _data().SetCount(n);
	return true;
}

int ParticleChannelNew::Delete(int start, int num)
{
	if (allNew() || allOld())
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

int ParticleChannelNew::Delete(BitArray& toRemove)
{	
	SysUtil::NeedToImplementLater(); // optimize the implementation

	int checkCount = min(toRemove.GetSize(), Count());

	if (allNew() || allOld())
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
		Tab<bool> oldData(data());

		int newCount = 0;
		for(int i=0; i<checkCount; i++)
			if (toRemove[i] == 0)
				_data(newCount++) = oldData[i];
		_data().Resize(newCount);	
		return data().Count();
	}
}

IObject* ParticleChannelNew::Split(BitArray& toSplit)
{
	ParticleChannelNew* newChannel = (ParticleChannelNew*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelNew::Spawn( Tab<int>& spawnTable )
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	int i, checkCount = min(spawnTable.Count(), Count());

	if (allNew() || allOld())
	{
		int newCount = 0;
		for(i=0; i<checkCount; i++)
			newCount += spawnTable[i];
		_globalCount() = newCount;
	}
	else
	{
		Tab<bool> oldData(data());
		int j, k, newCount = 0;
		for(i=0; i<checkCount; i++)
			newCount += spawnTable[i];
		_data().SetCount(newCount);
		for(i=0, j=0; i<checkCount; i++)
			for(k=0; k<spawnTable[i]; k++)
				_data(j++) = oldData[i];
	}
	return true;
}

bool ParticleChannelNew::AppendNum(int num)
{
	if (allNew() || allOld())
	{
		_globalCount() += num;
		return true;
	}

	if (num <= 0) return true;

	int i, oldCount = data().Count();
	int newCount = oldCount + num;

	if (_data().Resize(newCount) == 0) return false;
	_data().SetCount(newCount);
	if (allOld())
	{
		_allOld() = false;
		for(i=0; i<oldCount; i++)
			_data(i) = false; // old
	}
	for(i=oldCount; i<newCount; i++)
		_data(i) = true; // new

	return true;
}

bool ParticleChannelNew::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelNewR* iNew = GetParticleChannelNewRInterface(channel);
	DbgAssert(iNew);
	if (iNew == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	if (IsAllNew() && iNew->IsAllNew()) return true;
	if (IsAllOld() && iNew->IsAllOld()) return true;
	for(int i=0; i<num; i++)
	{
		if (iNew->IsNew(i)) SetNew(oldCount + i);
		else				SetOld(oldCount + i);
	}

	return true;
}

bool ParticleChannelNew::IsNew(int index) const
{
	if (allNew()) return true;
	if (allOld()) return false;
	DbgAssert((index>=0) && (index < data().Count()));
	return data()[index];
}

bool ParticleChannelNew::IsAllNew() const
{
	return allNew();
}

bool ParticleChannelNew::IsAllOld() const
{
	return allOld();
}

void ParticleChannelNew::SetNew(int index)
{
	if (allNew()) return; // nothing to change
	if (allOld())
	{	
		_allOld() = false;
		DbgAssert((index>=0) && (index < globalCount()));
		_data().SetCount(globalCount());
		for(int i=0; i<globalCount(); i++)
			_data(i) = false;
	}
	DbgAssert((index>=0) && (index < data().Count()));
	_data(index) = true;
	return;
}

void ParticleChannelNew::SetOld(int index)
{
	if (allOld()) return; // nothing to change
	if (allNew())
	{
		_allNew() = false;
		DbgAssert((index>=0) && (index < globalCount()));
		_data().SetCount(globalCount());
		for(int i = 0; i<globalCount(); i++)
			_data(i) = true;
	}
	DbgAssert((index>=0) && (index < data().Count()));
	_data(index) = false;
	return;
}

void ParticleChannelNew::SetAllNew()
{
	if (allNew()) return;
	_allNew() = true;
	if (allOld())
	{
		_allOld() = false;
		return;
	}
	_globalCount() = data().Count();
	_data().Resize(0);
}

void ParticleChannelNew::SetAllOld()
{
	if (allOld()) return;
	_allOld() = true;
	if (allNew())
	{
		_allNew() = false;
		return;
	}
	_globalCount() = data().Count();
	_data().Resize(0);
}


} // end of namespace PFChannels