/**********************************************************************
 *<
	FILE: ParticleChannelMeshMap.cpp

	DESCRIPTION: ParticleChannelMeshMap implementation
				 This generic channel is used to store mesh map data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelMeshMap.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelMeshMap::ParticleChannelMeshMap()
					:IParticleChannel(PARTICLECHANNELMESHMAPR_INTERFACE,PARTICLECHANNELMESHMAPW_INTERFACE)
{
	_count() = 0;
	for(int i=0; i<MAX_MESHMAPS; i++) _map(i) = NULL;
}

ParticleChannelMeshMap::~ParticleChannelMeshMap()
{
	// free memory allocated for mapping data
	for(int i=0; i<MAX_MESHMAPS; i++)
		if (map(i) != NULL)
			delete _map(i);
}

TCHAR* ParticleChannelMeshMap::GetIObjectName()
{
	return _T("ParticleChannelMeshMap");
}

BaseInterface* ParticleChannelMeshMap::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelMeshMapR*)this;
	case 4: return (IParticleChannelMeshMapW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelMeshMap::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelMeshMapR*)this;
	if (id == GetWriteID())
		return (IParticleChannelMeshMapW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelMeshMap::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelMeshMap::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelMeshMap::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelMeshMapDesc TheParticleChannelMeshMapDesc;

static FPInterfaceDesc pc_MeshMap_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelMeshMapDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_MeshMap_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelMeshMapDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_MeshMap_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelMeshMapDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_MeshMap_MeshMapR(
			PARTICLECHANNELMESHMAPR_INTERFACE,
			_T("meshMapRead"), 0,
			&TheParticleChannelMeshMapDesc, FP_MIXIN,

		IParticleChannelMeshMapR::kGetNumMaps, _T("getNumMaps"), 0, TYPE_INT, 0, 0,
		IParticleChannelMeshMapR::kMapSupport, _T("mapSupport"), 0, TYPE_bool, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,
		IParticleChannelMeshMapR::kGetMapReadChannel, _T("getMapReadChannel"), 0, TYPE_INTERFACE, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,

		end
);

static FPInterfaceDesc pc_MeshMap_MeshMapW(
			PARTICLECHANNELMESHMAPW_INTERFACE,
			_T("meshMapWrite"), 0,
			&TheParticleChannelMeshMapDesc, FP_MIXIN,

		IParticleChannelMeshMapW::kSetNumMaps, _T("setNumMaps"), 0, TYPE_VOID, 0, 2,
			_T("count"), 0, TYPE_INT,
			_T("keep"), 0, TYPE_bool,
		IParticleChannelMeshMapW::kSetMapSupport, _T("setMapSupport"), 0, TYPE_VOID, 0, 2,
			_T("mapChannel"), 0, TYPE_INT,
			_T("support"), 0, TYPE_bool, 
		IParticleChannelMeshMapW::kGetMapChannel, _T("getMapChannel"), 0, TYPE_INTERFACE, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,
		IParticleChannelMeshMapW::kGetMapChannelObject, _T("getMapChannelObject"), 0, TYPE_IOBJECT, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,

		end
);

FPInterfaceDesc* ParticleChannelMeshMap::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_MeshMap_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_MeshMap_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_MeshMap_AmountW;
	if (id == GetReadID()) return &pc_MeshMap_MeshMapR;
	if (id == GetWriteID()) return &pc_MeshMap_MeshMapW;
	return NULL;
}

Class_ID ParticleChannelMeshMap::GetClassID() const
{
	return ParticleChannelMeshMap_Class_ID;
}

IObject* ParticleChannelMeshMap::Clone() const
{
	ParticleChannelMeshMap* newChannel = (ParticleChannelMeshMap*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelMeshMap_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);

	newChannel->_count() = count();
	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			newChannel->_map(i) = (ParticleChannelMap*)map(i)->Clone();
	}

	return newChannel;
}

IOResult ParticleChannelMeshMap::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int num;

	// channel count
	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	// channel data
	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) == NULL) continue;
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = isave->Write(&i, sizeof(int), &nb)) != IO_OK) return res;
		isave->EndChunk();
		isave->BeginChunk(IParticleChannel::kChunkValue1);
		if ((res = map(i)->Save(isave)) != IO_OK) return res;
		isave->EndChunk();
	}

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

IOResult ParticleChannelMeshMap::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	static int channelNum;
	Interface_ID id;
	bool isg;

	while ((res = iload->OpenChunk()) == IO_OK)
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res = iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _count() = num;
			break;
		case IParticleChannel::kChunkData:
			res = iload->Read(&channelNum, sizeof(int), &nb);
			if (res == IO_OK) {
				if (map(channelNum) != NULL) delete _map(channelNum);
				_map(channelNum) = new ParticleChannelMap();
			}
			break;
		case IParticleChannel::kChunkValue1:
			if (map(channelNum) != NULL)
				res = _map(channelNum)->Load(iload);
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

int ParticleChannelMeshMap::MemoryUsed() const
{
	int mem = sizeof(int) + sizeof(void*)*MAX_MESHMAPS;
	for(int i=0; i<MAX_MESHMAPS; i++)
		if (map(i) != NULL)
			mem += map(i)->MemoryUsed();
	return mem;
}

int ParticleChannelMeshMap::Count() const
{
	// check if all channels are in sync
	int num = count();
	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL) {
			if (num != map(i)->Count()) {
				num = 0;
				break;
			}
		}
	}
	return num;
}

void ParticleChannelMeshMap::ZeroCount()
{
	_count() = 0;
	for(int i=0; i<MAX_MESHMAPS; i++)
		if (map(i) != NULL)
			_map(i)->ZeroCount();
}

bool ParticleChannelMeshMap::SetCount(int n)
{
	if (n < 0)	return false;
	if (n == 0)	{
		ZeroCount();
		return true;
	}

	_count() = n;
	bool res = true;
	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			res = res && _map(i)->SetCount(n);
	}

	return res;
}

int ParticleChannelMeshMap::Delete(int start, int num)
{
	if (start < 0) {
		num += start;
		start = 0;
	}
	if (num <= 0)	return Count();

	if (start+num < count())
		_count() = count() - num;
	else
		_count() = start;

	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			_map(i)->Delete(start, num);
	}

	return Count();
}

int ParticleChannelMeshMap::Delete(BitArray& toRemove)
{	
	int i, removeNum=0;
	for(i=0; i<count(); i++)
		if (toRemove[i] != 0) removeNum++;
	_count() = count() - removeNum;

	for(i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			_map(i)->Delete(toRemove);
	}

	return Count();
}

IObject* ParticleChannelMeshMap::Split(BitArray& toSplit)
{
	ParticleChannelMeshMap* newChannel = (ParticleChannelMeshMap*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	if (reverse.GetSize() != newChannel->Count())
		reverse.SetSize(newChannel->Count(), TRUE);
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelMeshMap::Spawn(Tab<int>& spawnTable)
{
	bool res = true;

	int i, newCount=0;

	for(i=0; i<count() && i<spawnTable.Count(); i++)
		newCount += spawnTable[i];
	_count() = newCount;

	for(i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			res = res && _map(i)->Spawn(spawnTable);
	}

	return res;
}

bool ParticleChannelMeshMap::AppendNum(int num)
{
	if (num < 0)	return false;
	if (num == 0)	return true;

	_count() = count() + num;
	bool res = true;
	for(int i=0; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL)
			res = res && _map(i)->AppendNum(num);
	}

	return res;
}

bool ParticleChannelMeshMap::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int i, num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelMeshMapR* iMeshMapR = (IParticleChannelMeshMapR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iMeshMapR);
	IParticleChannelMeshMapW* iMeshMapW = (IParticleChannelMeshMapW*)(channel->GetInterface(GetWriteID()));
	DbgAssert(iMeshMapW);
	if ((iMeshMapR == NULL) || (iMeshMapW == NULL)) return false;

	int oldCount = Count();
	_count() = count() + num;
	// add new map channels if they are present in the object appended
	for(i=0; i<MAX_MESHMAPS; i++) {
		if ((iMeshMapR->MapSupport(i) == true) && (MapSupport(i) == false)) {
			_map(i) = new ParticleChannelMap(); assert(map(i));
			_map(i)->SetCount(oldCount);
		}
	}

	// append new data
	bool res = true;
	for(i=0; i<MAX_MESHMAPS; i++) {
		if (iMeshMapR->MapSupport(i) == true)
			res = res && _map(i)->Append(iMeshMapW->GetMapChannelObject(i));
		else if (MapSupport(i) == true)
			res = res && _map(i)->AppendNum(num);
	}

	return res;
}

int ParticleChannelMeshMap::GetNumMaps() const
{
	int num=0;
	for(int i=0; i<MAX_MESHMAPS; i++)
		if (map(i) != NULL) num = i;

	return num+1;
}

bool ParticleChannelMeshMap::MapSupport(int mp) const
{
	return (map(mp) != NULL);
}

IParticleChannelMapR* ParticleChannelMeshMap::GetMapReadChannel(int mp) 
{
	if (map(mp) != NULL) return GetParticleChannelMapRInterface(_map(mp));
	return NULL;
}

void ParticleChannelMeshMap::SetNumMaps(int ct, bool keep)
{
	int i;
	for(i=0; i<ct; i++) {
		if (!keep && (map(i) != NULL)) {
			delete _map(i);
			_map(i) = NULL;
		}
		if (map(i) == NULL) {
			_map(i) = new ParticleChannelMap();
			_map(i)->SetCount(count());
		}
	}

	for(i=ct; i<MAX_MESHMAPS; i++) {
		if (map(i) != NULL) {
			delete _map(i);
			_map(i) = NULL;
		}
	}
}

void ParticleChannelMeshMap::SetMapSupport(int mp, bool support)
{
	if (support) {
		if (map(mp) == NULL) {
			_map(mp) = new ParticleChannelMap();
			_map(mp)->SetCount(count());
		}
	} else {
		if (map(mp) != NULL) {
			delete _map(mp);
			_map(mp) = NULL;
		}
	}
}

IParticleChannelMapW* ParticleChannelMeshMap::GetMapChannel(int mp)
{
	if (map(mp) != NULL) return GetParticleChannelMapWInterface(_map(mp));
	return NULL;
}

IObject* ParticleChannelMeshMap::GetMapChannelObject(int mp) const
{
	if (map(mp) != NULL) return (IObject*)(map(mp));
	return NULL;
}



} // end of namespace PFChannels
