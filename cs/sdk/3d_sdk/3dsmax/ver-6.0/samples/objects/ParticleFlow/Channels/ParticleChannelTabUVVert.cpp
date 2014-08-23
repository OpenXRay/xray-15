/**********************************************************************
 *<
	FILE: ParticleChannelTabUVVert.cpp

	DESCRIPTION: ParticleChannelTabUVVert implementation
				 This generic channel is used to store Tab<UVVert> data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelTabUVVert.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelTabUVVert::ParticleChannelTabUVVert()
					:IParticleChannel(PARTICLECHANNELTABUVVERTR_INTERFACE,PARTICLECHANNELTABUVVERTW_INTERFACE)
{
	_globalData() = NULL;
	_globalCount() = 0;
	_dataType() = kGlobalData;
}

ParticleChannelTabUVVert::ParticleChannelTabUVVert(const ParticleChannelTabUVVert* channel)
					:IParticleChannel(PARTICLECHANNELTABUVVERTR_INTERFACE,PARTICLECHANNELTABUVVERTW_INTERFACE)
{
	if (channel->globalData() != NULL)
		_globalData() = new TabUVVert(*(channel->globalData()));	
	else
		_globalData() = NULL;

	_globalCount() = channel->globalCount();
	_dataType() = channel->dataType();
	_sharedTable() = channel->sharedTable();

	int i, num = channel->localData().Count();
	_localData().SetCount( num );
	for(i=0; i<num; i++) {
		if (channel->localData(i) != NULL)
			_localData(i) = new TabUVVert(*(channel->localData(i)));
		else
			_localData(i) = NULL;
	}

	num = channel->sharedData().Count();
	_sharedData().SetCount( num );
	for(i=0; i<num; i++) {
		if (channel->sharedData(i) != NULL)
			_sharedData(i) = new TabUVVert(*(channel->sharedData(i)));
		else
			_sharedData(i) = NULL;
	}
}

ParticleChannelTabUVVert::~ParticleChannelTabUVVert()
{
	// free memory allocated for TabUVVertes
	if (globalData())
		delete _globalData();
	for (int i = 0; i < sharedData().Count(); i++)
		if (sharedData(i))	delete _sharedData(i);
	for (i = 0; i < localData().Count(); i++)
		if (localData(i))	delete _localData(i);
}

TCHAR* ParticleChannelTabUVVert::GetIObjectName()
{
	return _T("ParticleChannelTabUVVert");
}

BaseInterface* ParticleChannelTabUVVert::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelTabUVVertR*)this;
	case 4: return (IParticleChannelTabUVVertW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelTabUVVert::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelTabUVVertR*)this;
	if (id == GetWriteID())
		return (IParticleChannelTabUVVertW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelTabUVVert::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelTabUVVert::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelTabUVVert::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelTabUVVertDesc TheParticleChannelTabUVVertDesc;

static FPInterfaceDesc pc_TabUVVert_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelTabUVVertDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_TabUVVert_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelTabUVVertDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_TabUVVert_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelTabUVVertDesc, FP_MIXIN,

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

FPInterfaceDesc* ParticleChannelTabUVVert::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_TabUVVert_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_TabUVVert_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_TabUVVert_AmountW;
	return NULL;
}

Class_ID ParticleChannelTabUVVert::GetClassID() const
{
	return ParticleChannelTabUVVert_Class_ID;
}

IObject* ParticleChannelTabUVVert::Clone() const
{
	ParticleChannelTabUVVert* newChannel = (ParticleChannelTabUVVert*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelTabUVVert_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);

	newChannel->_localData().SetCount(localData().Count());
	for (int i = 0; i < localData().Count(); i++)
		newChannel->_localData(i) = (localData(i)) ? new TabUVVert(*localData(i)) : NULL;

	newChannel->_sharedData().SetCount(sharedData().Count());
	for (i = 0; i < sharedData().Count(); i++)
		newChannel->_sharedData(i) = (sharedData(i)) ? new TabUVVert(*sharedData(i)) : NULL;

	newChannel->_globalData() = (globalData()) ? new TabUVVert(*globalData()) : NULL;

	newChannel->_sharedTable() = sharedTable();
	newChannel->_globalCount() = globalCount();
	newChannel->_dataType() = dataType();

	return newChannel;
}

IOResult ParticleChannelTabUVVert::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int i, num, tabNum;

	// global count
	isave->BeginChunk(kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	// local data
	isave->BeginChunk(kChunkLocalData);
	num = localData().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	for (i=0; i<num; i++) {
		tabNum = (localData(i)) ? localData(i)->Count() : 0;
		if ((res = isave->Write(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
		if (tabNum > 0)
			if ((res = isave->Write(localData(i)->Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
	}
	isave->EndChunk();

	// shared data
	isave->BeginChunk(kChunkSharedData);
	num = sharedData().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	for (i=0; i<num; i++) {
		tabNum = (sharedData(i)) ? sharedData(i)->Count() : 0;
		if ((res = isave->Write(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
		if (tabNum > 0)
			if ((res = isave->Write(sharedData(i)->Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
	}
	isave->EndChunk();

	// shared table
	isave->BeginChunk(kChunkSharedTable);
	num = sharedTable().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	if (num > 0)
		if ((res = isave->Write(sharedTable().Addr(0), sizeof(int)*num, &nb)) != IO_OK) return res;
	isave->EndChunk();

	// global value
	if (globalData()) {
		isave->BeginChunk(kChunkGlobalValue);
		tabNum = globalData()->Count();
		if ((res = isave->Write(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
		if (tabNum > 0)
			if ((res = isave->Write(globalData()->Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	// channel type
	isave->BeginChunk(kChunkDataType);
	num = dataType();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();


	// interface id
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

IOResult ParticleChannelTabUVVert::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num, tabNum;
	Interface_ID id;
	TabUVVert tab; // tab to initiate new tabs
	bool isg;

	while ((res = iload->OpenChunk()) == IO_OK)
	{
		switch(iload->CurChunkID())
		{
		case kChunkGlobalCount: // global count
			if ((res = iload->Read(&num, sizeof(int), &nb)) != IO_OK) return res;
			_globalCount() = num;
			break;
		case kChunkLocalData: // local data
			for (i=0; i<localData().Count(); i++)
				if (localData(i))	delete _localData(i);
			_localData().ZeroCount();
			if ((res = iload->Read(&num, sizeof(int), &nb)) != IO_OK) return res;
			_localData().SetCount(num);
			for(i=0; i<num; i++) {
				_localData(i) = NULL;
				if ((res = iload->Read(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
				if (tabNum > 0) {
					tab.SetCount(tabNum);
					if ((res = iload->Read(tab.Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
					_localData(i) = new TabUVVert(tab);
				}
			}
			break;
		case kChunkSharedData: // shared data
			for (i=0; i<sharedData().Count(); i++)
				if (sharedData(i))	delete _sharedData(i);
			_sharedData().ZeroCount();
			if ((res = iload->Read(&num, sizeof(int), &nb)) != IO_OK) return res;
			_sharedData().SetCount(num);
			for(i=0; i<num; i++) {
				_sharedData(i) = NULL;
				if ((res = iload->Read(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
				if (tabNum > 0) {
					tab.SetCount(tabNum);
					if ((res = iload->Read(tab.Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
					_sharedData(i) = new TabUVVert(tab);
				}
			}
			break;
		case kChunkSharedTable:	// shared table
			if ((res = iload->Read(&num, sizeof(int), &nb)) != IO_OK) return res;
			_sharedTable().SetCount(num);
			if (num > 0)
				if ((res = iload->Read(_sharedTable().Addr(0), sizeof(int)*num, &nb)) != IO_OK) return res;
			break;
		case kChunkGlobalValue: // global value
			if (globalData() != NULL) delete _globalData();
			_globalData() = NULL;
			if ((res = iload->Read(&tabNum, sizeof(int), &nb)) != IO_OK) return res;
			if (tabNum > 0) {
				tab.SetCount(tabNum);
				if ((res = iload->Read(tab.Addr(0), tabNum*sizeof(UVVert), &nb)) != IO_OK) return res;
				_globalData() = new TabUVVert(tab);
			}
			break;
		case kChunkDataType:	// channel type
			if ((res = iload->Read(&num, sizeof(int), &nb)) != IO_OK) return res;
			if (res == IO_OK) {
				switch (num) {
				case kGlobalData:
				case kSharedData:
				case kLocalData:
					_dataType() = num;
				}
			}
			break;
		case IParticleChannel::kChunkReadID:
			res = iload->Read(&id, sizeof(Interface_ID), &nb);
			if (res == IO_OK) SetReadID(id);
			break;
		case IParticleChannel::kChunkWriteID:
			res = iload->Read(&id, sizeof(Interface_ID), &nb);
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

int ParticleChannelTabUVVert::MemoryUsed() const
{
	int i, mem = sizeof(int)*2;
	mem += sharedTable().Count()*sizeof(int);
	if (globalData() != NULL)
		mem += globalData()->Count()*sizeof(UVVert);
	for(i=0; i<sharedData().Count(); i++)
		if (sharedData(i) != NULL)
			mem += sharedData(i)->Count()*sizeof(UVVert);
	for(i=0; i<localData().Count(); i++)
		if (localData(i) != NULL)
			mem += localData(i)->Count()*sizeof(UVVert);
	return mem;
}

int ParticleChannelTabUVVert::Count() const
{
	switch (dataType())
	{
	case kGlobalData:
		return globalCount();
	case kSharedData:
		return sharedTable().Count();
	case kLocalData:
		return localData().Count();
	}
	return 0;
}

void ParticleChannelTabUVVert::ZeroCount()
{
	// This function can be implemented by calling ParticleChannelTabUVVert::SetCount(0),
	// but let's implement it specifically for efficiency.
	int i;

	switch (dataType())
	{
	case kGlobalData:
		if (globalData()) {
			delete _globalData();
			_globalData() = NULL;
		}
		_globalCount() = 0;
		return;
	case kSharedData:
		for (i = 0; i < sharedData().Count(); i++) {
			if (sharedData(i)) {
				delete _sharedData(i);
				_sharedData(i) = NULL;
			}
		}
		_sharedData().ZeroCount();
		_sharedTable().ZeroCount();
		ConvertToGlobal();
		return;
	case kLocalData:
		for (i = 0; i < localData().Count(); i++) {
			if (localData(i)) {
				delete _localData(i);
				_localData(i) = NULL;
			}
		}
		_localData().ZeroCount();
		ConvertToGlobal();
		return;
	}
}

bool ParticleChannelTabUVVert::SetCount(int n)
{
	if (n < 0)	return false;
	if (n == 0)	{
		ZeroCount();
		return true;
	}

	int i;

	switch (dataType())
	{
	case kGlobalData:
		_globalCount() = n;
		return true;
	case kSharedData:
		i = sharedTable().Count();
		if (n != i)	{
			_sharedTable().SetCount(n);
			if (n < i) {					// shrink
				ShrinkSharedData();
			} else {						// expand
				for (; i < n; i++)			_sharedTable(i) = -1;
			}
		}
		return true;
	case kLocalData:
		i = localData().Count();
		if (n != i) {
			if (n < i) {					// shrink
				for (i--; i >= n; i--)
					if (localData(i)) {
						delete _localData(i);
						_localData(i) = NULL;
					}
				_localData().SetCount(n);
			} else {						// expand
				_localData().SetCount(n);
				for (; i<n; i++) _localData(i) = NULL;
			}
		}
		return true;
	}

	return false;
}

int ParticleChannelTabUVVert::Delete(int start, int num)
{
	if (start < 0) {
		num += start;
		start = 0;
	}
	if (num <= 0)	return Count();

	int i, j;

	switch (dataType())
	{
	case kGlobalData:
		if (start < globalCount()) {
			if ((start+num) >= globalCount()) _globalCount() = start;
			else _globalCount() -= num;
			if (globalCount() == 0 && globalData()) {
				delete _globalData();
				_globalData() = NULL;
			}
		}
		return globalCount();
	case kSharedData:
		if (start < sharedTable().Count()) {
			_sharedTable().Delete(start, num);
			ShrinkSharedData();
			return Count();
		}
		return sharedTable().Count();
	case kLocalData:
		if (start < localData().Count()) {
			for (i = start, j = 0; i < localData().Count() && j < num; i++, j++)
				if (localData(i)) {
					delete _localData(i);
					_localData(i) = NULL;
				}
			_localData().Delete(start, num);
		}
		return localData().Count();
	}

	return 0;
}

int ParticleChannelTabUVVert::Delete(BitArray& toRemove)
{	
	int i, j;
	int checkCount = toRemove.GetSize();

	switch (dataType())
	{
	case kGlobalData:
		checkCount = min(checkCount, globalCount());
		for (i = j = 0; i < checkCount; i++)
			if (toRemove[i] != 0) j++;
		if (j > 0) {
			_globalCount() -= j;
			if (globalCount() == 0 && globalData()) {
				delete _globalData();
				_globalData() = NULL;
			}
		}
		return globalCount();
	case kSharedData:
		for (i = j = 0; i < sharedTable().Count(); i++) {
			if (i < checkCount)
				if (toRemove[i] != 0)	continue;
			if (i != j)	_sharedTable(j) = sharedTable(i);
			j++;
		}
		if (j < sharedTable().Count()) {
			_sharedTable().SetCount(j);
			ShrinkSharedData();			// this could change data type
			return Count();
		}
		return sharedTable().Count();
	case kLocalData:
		for (i = j = 0; i < localData().Count(); i++) {
			if (i < checkCount)
				if (toRemove[i] != 0)	{
					if (localData(i)) {
						delete _localData(i);
						_localData(i) = NULL;
					}
					continue;
				}
			if (i != j)	_localData(j) = _localData(i);
			j++;
		}
		if (j < localData().Count())
			_localData().SetCount(j);
		return localData().Count();
	}

	return 0;
}

IObject* ParticleChannelTabUVVert::Split(BitArray& toSplit)
{
	// SysUtil::NeedToImplementLater(); // TODO: optimize the implementation

	ParticleChannelTabUVVert* newChannel = (ParticleChannelTabUVVert*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	if (reverse.GetSize() != newChannel->Count())
		reverse.SetSize(newChannel->Count(), TRUE);
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelTabUVVert::Spawn(Tab<int>& spawnTable)
{
	int i, j, k;
	int checkCount = spawnTable.Count();

	switch (dataType())
	{
	case kGlobalData:
		checkCount = min(checkCount, globalCount());
		for (i = j = 0; i < checkCount; i++)
			if (spawnTable[i] > 0)	j += spawnTable[i];
		_globalCount() = j;
		if (globalCount() == 0 && globalData()) {
			delete _globalData();
			_globalData() = NULL;
		}
		return true;
	case kSharedData:
		{
		Tab<int> oldData(sharedTable());
		checkCount = min(checkCount, sharedTable().Count());
		for (i = j = 0; i < checkCount; i++)
			if (spawnTable[i] > 0)	j += spawnTable[i];
		_sharedTable().SetCount(j);
		for (i = j = 0; i < checkCount; i++)
			if (spawnTable[i] > 0) {
				k = oldData[i];
				for (int x = 0; x < spawnTable[i]; x++)
					_sharedTable(j++) = k;
			}
		ShrinkSharedData();
		}
		return true;
	case kLocalData:
		ConvertToShared();			// convert to sharedData type
		return Spawn(spawnTable);
	}

	return false;
}

bool ParticleChannelTabUVVert::AppendNum(int num)
{
	if (num < 0)	return false;
	if (num == 0)	return true;

	int i;

	switch (dataType())
	{
	case kGlobalData:
		if (globalData()) {
			ConvertToShared();				// convert to sharedData type
			return AppendNum(num);			// call Append on sharedData type
		} else {
			_globalCount() += num;
		}
		return true;
	case kSharedData:
		i = sharedTable().Count();
		_sharedTable().SetCount(i+num);
		for (; i < sharedTable().Count(); i++)	_sharedTable(i) = -1;
		return true;
	case kLocalData:
		ConvertToShared();					// convert to sharedData type
		return AppendNum(num);				// call Append on sharedData type
	}

	return false;
}

bool ParticleChannelTabUVVert::Append(IObject* channel)
{
	// requires more work on optimization
	// needs to work differently with all combinations of sharing

	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelTabUVVertR* iTabUVVert = (IParticleChannelTabUVVertR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iTabUVVert);
	if (iTabUVVert == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, (TabUVVert*)(iTabUVVert->GetValue(i)) );

	return true;
}

bool ParticleChannelTabUVVert::IsShared() const
{
	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
		return sharedData().Count() < sharedTable().Count();
	case kLocalData:
		return false;
	}
	return false;
}

int ParticleChannelTabUVVert::GetValueCount() const
{
	switch (dataType())
	{
	case kGlobalData:
		return globalData() ? 1 : 0;
	case kSharedData:
		return sharedData().Count();
	case kLocalData:
		return localData().Count();
	}
	return 0;
}

int ParticleChannelTabUVVert::GetValueIndex(int particleIndex) const
{
	switch (dataType())
	{
	case kGlobalData:
		DbgAssert(particleIndex >= 0 && particleIndex < globalCount());
		if (particleIndex < 0 || particleIndex >= globalCount())	return -1;
		return globalData() ? 0 : -1;
	case kSharedData:
		DbgAssert(particleIndex >= 0 && particleIndex < sharedTable().Count());
		if (particleIndex < 0 || particleIndex >= sharedTable().Count())	return -1;
		return sharedTable(particleIndex);
	case kLocalData:
		DbgAssert(particleIndex >= 0 && particleIndex < localData().Count());
		if (particleIndex < 0 || particleIndex >= localData().Count())	return -1;
		return particleIndex;
	}
	return -1;
}

const TabUVVert* ParticleChannelTabUVVert::GetValueByIndex(int valueIndex) const
{
	switch (dataType())
	{
	case kGlobalData:
		if (valueIndex != 0)	return NULL;
		return globalData();
	case kSharedData:
		if (valueIndex < 0 || valueIndex >= sharedData().Count())	return NULL;
		return sharedData(valueIndex);
	case kLocalData:
		if (valueIndex < 0 || valueIndex >= localData().Count())	return NULL;
		return localData(valueIndex);
	}
	return NULL;
}

const TabUVVert*	ParticleChannelTabUVVert::GetValue(int particleIndex) const
{
	switch (dataType())
	{
	case kGlobalData:
		DbgAssert(particleIndex >= 0 && particleIndex < globalCount());
		if (particleIndex < 0 || particleIndex >= globalCount())	return NULL;
		return globalData();
	case kSharedData:
		DbgAssert(particleIndex >= 0 && particleIndex < sharedTable().Count());
		if (particleIndex < 0 || particleIndex >= sharedTable().Count())	return NULL;
		particleIndex = sharedTable(particleIndex);
		if (particleIndex < 0 || particleIndex >= sharedData().Count())	return NULL;
		return sharedData(particleIndex);
	case kLocalData:
		DbgAssert(particleIndex >= 0 && particleIndex < localData().Count());
		if (particleIndex < 0 || particleIndex >= localData().Count())	return NULL;
		return localData(particleIndex);
	}
	return NULL;
}

const TabUVVert*	ParticleChannelTabUVVert::GetValue() const
{
	return GetValue(0);
}

bool ParticleChannelTabUVVert::SetValue(int particleIndex, const TabUVVert* value)
{
	int count = Count();
	DbgAssert(particleIndex >= 0 && particleIndex < count);
	if (particleIndex < 0 || particleIndex >= count)	return false;

	int shrIndex;

	switch (dataType())
	{
	case kGlobalData:
		ConvertToShared();
		return SetValue(particleIndex, value);
	case kSharedData:
		if (value) {
			bool hasSharing = ParticleHasSharing(particleIndex);
			shrIndex = sharedTable(particleIndex);
			if (hasSharing || (shrIndex < 0) || (shrIndex >= sharedData().Count())) {
				shrIndex = sharedData().Count();
				_sharedTable(particleIndex) = shrIndex;
				_sharedData().SetCount(shrIndex+1);
				_sharedData(shrIndex) = NULL;
			}
			if (sharedData(shrIndex)) {
				*_sharedData(shrIndex) = *value;
			} else {
				_sharedData(shrIndex) = new TabUVVert(*value);
			}
		} else {
			_sharedTable(particleIndex) = -1;
		}
		ShrinkSharedData();
		return true;
	case kLocalData:
		if (value) {
			if (localData(particleIndex))
				*_localData(particleIndex) = *value;
			else
				_localData(particleIndex) = new TabUVVert(*value);
		} else {
			ConvertToShared();
			return SetValue(particleIndex, value);
		}
		return true;
	}

	return false;
}

bool ParticleChannelTabUVVert::SetValue(int particleIndex, UVVert value)
{
	TabUVVert tab;
	tab.SetCount(1);
	tab[0] = value;
	return SetValue(particleIndex, &tab);
}

bool ParticleChannelTabUVVert::SetValue(Tab<int>& particleIndices, const TabUVVert* value)
{
	if (particleIndices.Count() == 0)	return true;
	
	int firstParticleIndex = particleIndices[0];
	SetValue(firstParticleIndex, value);
	for (int i = 1; i < particleIndices.Count(); i++)
		CopyValue(firstParticleIndex, particleIndices[i]);
	return true;
}

bool ParticleChannelTabUVVert::SetValue(Tab<int>& particleIndices, UVVert value)
{
	TabUVVert tab;
	tab.SetCount(1);
	tab[0] = value;
	return SetValue(particleIndices, &tab);
}

bool ParticleChannelTabUVVert::SetValue(const TabUVVert* value)
{
	switch (dataType())
	{
	case kGlobalData:
		if (value) {
			if (globalData())
				*_globalData() = *value;
			else
				_globalData() = new TabUVVert(*value);
		} else {
			if (globalData()) {
				delete _globalData();
				_globalData() = NULL;
			}
		}
		return true;
	case kSharedData:
		ConvertToGlobal();
		return SetValue(value);
	case kLocalData:
		ConvertToGlobal();
		return SetValue(value);
	}

	return false;
}

bool ParticleChannelTabUVVert::SetValue(UVVert value)
{
	TabUVVert tab;
	tab.SetCount(1);
	tab[0] = value;
	return SetValue(&tab);
}

bool ParticleChannelTabUVVert::CopyValue(int fromParticle, int toParticle)
{
	int count = Count();
	if (fromParticle < 0 || fromParticle >= count || toParticle < 0 || toParticle >= count)
		return false;
	if (fromParticle == toParticle)	return true;

	int fromIdx, toIdx;

	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
		fromIdx = sharedTable(fromParticle);
		toIdx = sharedTable(toParticle);
		if (fromIdx != toIdx) {
			_sharedTable(toParticle) = fromIdx;
			if (toIdx >= 0 && toIdx < sharedData().Count())
				ShrinkSharedData();
		}
		return true;
	case kLocalData:
		ConvertToShared();
		return CopyValue(fromParticle, toParticle);
	}

	return false;
}

bool ParticleChannelTabUVVert::CopyValue(int fromParticle, Tab<int>& toParticles)
{
	int count = Count();
	if (fromParticle < 0 || fromParticle >= count)
		return false;
	
	int i, fromIdx, toIdx;
	bool checkShrink = false;

	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
		fromIdx = sharedTable(fromParticle);
		for (i = 0; i < toParticles.Count(); i++) {
			int toParticle = toParticles[i];
			if (toParticle < 0 || toParticle >= count)	continue;
			toIdx = sharedTable(toParticle);
			if (fromIdx != toIdx) {
				_sharedTable(toParticle) = fromIdx;
				if (toIdx >= 0 && toIdx < sharedData().Count() && !checkShrink)
					checkShrink = true;
			}
		}
		if (checkShrink)	ShrinkSharedData();
		return true;
	case kLocalData:
		ConvertToShared();
		return CopyValue(fromParticle, toParticles);
	}

	return false;
}

bool ParticleChannelTabUVVert::CopyValue(int fromParticle)
{
	int count = Count();
	if (fromParticle < 0 || fromParticle >= count)
		return false;

	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
	case kLocalData:
		ConvertToGlobal(fromParticle);
		return true;
	}

	return false;
}

bool ParticleChannelTabUVVert::ConvertToLocal()
{
	TabUVVert* tabData;
	BitArray valueCopied;
	int i, j, valueCount;

	switch (dataType())
	{
	case kGlobalData:
		DbgAssert(localData().Count() == 0);	// if not, might have memory leaks
		_localData().SetCount(globalCount());
		tabData = _globalData();
		for (i = 0; i < globalCount(); i++)
			_localData(i) = (tabData) ? ((i == 0) ? tabData : new TabUVVert(*tabData)) : NULL;
		_globalData() = NULL;
		_globalCount() = 0;
		_dataType() = kLocalData;
		return true;
	case kSharedData:
		DbgAssert(localData().Count() == 0);	// if not, might have memory leaks
		_localData().SetCount(sharedTable().Count());
		valueCount = sharedData().Count();
		valueCopied.SetSize(valueCount);
		valueCopied.ClearAll();
		for (i = 0; i < sharedTable().Count(); i++) {
			j = sharedTable(i);
			tabData = (j >= 0 && j < valueCount) ? _sharedData(j) : NULL;
			if (tabData && !valueCopied[j]) {
				_localData(i) = tabData;
				valueCopied.Set(j);
			} else {
				_localData(i) = (tabData) ? new TabUVVert(*tabData) : NULL;
			}
		}
		_sharedData().ZeroCount();
		_sharedTable().ZeroCount();
		_dataType() = kLocalData;
		return true;
	case kLocalData:
		return true;
	}

	return false;
}

bool ParticleChannelTabUVVert::ConvertToShared()
{
	int i, j;

	switch (dataType())
	{
	case kGlobalData:
		DbgAssert(sharedData().Count() == 0);	// if not, might have memory leaks
		_sharedTable().SetCount(globalCount());
		if (globalData()) {
			_sharedData().SetCount(1);
			_sharedData(0) = _globalData();
			j = 0;
		} else {
			_sharedData().SetCount(0);
			j = -1;
		}
		for (i = 0; i < globalCount(); i++)		_sharedTable(i) = j;
		_globalData() = NULL;
		_globalCount() = 0;
		_dataType() = kSharedData;
		return true;
	case kSharedData:
		return true;
	case kLocalData:
		DbgAssert(sharedData().Count() == 0);	// if not, might have memory leaks
		_sharedTable().SetCount(localData().Count());
		_sharedData().SetCount(localData().Count());
		for (i = 0; i < localData().Count(); i++) {
			_sharedData(i) = _localData(i);
			_localData(i) = NULL;
			_sharedTable(i) = i;
		}
		_localData().ZeroCount();
		_dataType() = kSharedData;
		return true;
	}

	return false;
}

bool ParticleChannelTabUVVert::ConvertToGlobal(int particleIndex /* = 0 */)
{
	int i, index;
	TabUVVert* tabData;

	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
		DbgAssert(globalData() == NULL);		// if not, might have memory leaks
		if (particleIndex < 0 || particleIndex >= sharedTable().Count()) {
			index = -1;
			tabData = NULL;
		} else {
			index = sharedTable(particleIndex);
			tabData = (index >= 0 && index < sharedData().Count()) ? _sharedData(index) : NULL;
		}
		for (i = 0; i < sharedData().Count(); i++)
			if (sharedData(i) && i != index) {
				delete _sharedData(i);
				_sharedData(i) = NULL;
			}
		_globalData() = tabData;
		_globalCount() = sharedTable().Count();
		_sharedData().ZeroCount();
		_sharedTable().ZeroCount();
		_dataType() = kGlobalData;
		return true;
	case kLocalData:
		DbgAssert(globalData() == NULL);		// if not, might have memory leaks
		index = particleIndex;
		tabData = (index >= 0 && index < localData().Count()) ? _localData(index) : NULL;
		for (i = 0; i < localData().Count(); i++)
			if (localData(i) && i != index) {
				delete _localData(i);
				_localData(i) = NULL;
			}
		_globalData() = tabData;
		_globalCount() = localData().Count();
		_localData().ZeroCount();
		_dataType() = kGlobalData;
		return true;
	}

	return false;
}

bool ParticleChannelTabUVVert::ShrinkSharedData()
{
	if (dataType() != kSharedData)	return true;

	int i, j;
	int valueCount = sharedData().Count();
	bool hasNullValue = false;
	BitArray entryUsed;
	Tab<int> entryMap;
	
	// mark used entries
	entryUsed.SetSize(valueCount);
	entryUsed.ClearAll();
	for (i = 0; i < sharedTable().Count(); i++) {
		j = sharedTable(i);
		if (j >= 0 && j < valueCount && sharedData(j)) {
			if (!entryUsed[j])	entryUsed.Set(j);
		}
	}
	// remove unused entries
	entryMap.SetCount(valueCount);
	for (i = j = 0; i < valueCount; i++) {
		if (entryUsed[i]) {
			entryMap[i] = j;
			if (i != j) _sharedData(j) = _sharedData(i);
			j++;
		} else {
			entryMap[i] = -1;
			if (sharedData(i)) {
				delete _sharedData(i);
				_sharedData(i) = NULL;
			}
		}
	}
	if (j != valueCount)
		_sharedData().SetCount(j);
	// map to new entries
	for (i = 0; i < sharedTable().Count(); i++) {
		j = sharedTable(i);
		j = _sharedTable(i) = (j >= 0 && j < valueCount) ? entryMap[j] : -1;
		if (j < 0 && !hasNullValue)	hasNullValue = true;
	}
	// convert to global
	valueCount = sharedData().Count();
	if (valueCount == 0 || (valueCount == 1 && !hasNullValue)) {
		ConvertToGlobal();
		return true;
	}
	// convert to local
	if (valueCount == sharedTable().Count()) {
		ConvertToLocal();
		return true;
	}
	return true;
}

bool ParticleChannelTabUVVert::ParticleHasSharing(int particleIndex)
{
	switch (dataType()) {
	case kLocalData:
		return false;
	case kSharedData: {
		if ((particleIndex < 0) || (particleIndex >= sharedTable().Count())) return false;
		int shrIndex = sharedTable(particleIndex);
		bool hasSharing = false;
		for(int i=0; i<particleIndex; i++) {
			if (shrIndex == sharedTable(i)) {
				hasSharing = true;
				break;
			}
		}
		if (!hasSharing) {
			for(i=particleIndex+1; i<sharedTable().Count(); i++) {
				if (shrIndex == sharedTable(i)) {
					hasSharing = true;
					break;
				}
			}
		}
		return hasSharing;
					  }
		break;
	case kGlobalData:
		return true;
	}
	return false;
}

} // end of namespace PFChannels
