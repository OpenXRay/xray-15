/**********************************************************************
 *<
	FILE: ParticleChannelID.cpp

	DESCRIPTION: ParticleChannelID implementation
				 the channel is used to identify particles
				 particle ID consists of two integers: particleIndex and
				 particleBorn. ParticleIndex gives relative correspondense
				 to the whole amount of particles. If PF ParticleAmountMultiplier
				 is set to 100% then the given particleIndex are successive 
				 ordinal numbers. If it's set to 50% then the given particle indeces
				 are 0, 2, 4, 6 etc. If it is a Birth by Total Number then the
				 last particle born has an index of the total number whatever
				 multiplier is. ParticleBorn number always are successive
				 ordinal numbers when particles were born: 0, 1, 2, 3 etc.
				 If ParticleAmountMultiplier equals 100% then for a 
				 particular particle particleIndex and particleBorn are the
				 same number. If ParticleAmountMultiplier is greater then
				 100% then you may have several particles with the same
				 particleIndex. It is recommended to link non-random properties to
				 particleIndex and random properties to particleBorn.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-16-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelID.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelID::ParticleChannelID()
					:IParticleChannel(PARTICLECHANNELIDR_INTERFACE,PARTICLECHANNELIDW_INTERFACE)
{
	;
}

ParticleChannelID::~ParticleChannelID()
{
	; // implement later!
}

TCHAR* ParticleChannelID::GetIObjectName()
{
	return _T("ParticleChannelID");
}

BaseInterface* ParticleChannelID::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelIDR*)this;
	case 4: return (IParticleChannelIDW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelID::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == PARTICLECHANNELIDR_INTERFACE)
		return (IParticleChannelIDR*)this;
	if (id == PARTICLECHANNELIDW_INTERFACE)
		return (IParticleChannelIDW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelID::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelID::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelID::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelIDDesc TheParticleChannelIDDesc;

static FPInterfaceDesc pc_ID_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelIDDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_ID_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelIDDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_ID_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelIDDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_ID_IDR(
			PARTICLECHANNELIDR_INTERFACE,
			_T("idRead"), 0,
			&TheParticleChannelIDDesc, FP_MIXIN,

		particleChannelID_getID, _T("getID"), 0, TYPE_bool, 0, 3,
			_T("index"), 0, TYPE_INT,
			_T("particleIndex"), 0, TYPE_INT,
			_T("particleBorn"), 0, TYPE_INT,
		particleChannelID_getParticleIndex, _T("getParticleIndex"), 0, TYPE_INT, 0, 1,
			_T("index"), 0, TYPE_INT,
		particleChannelID_getParticleBorn, _T("getParticleBorn"), 0, TYPE_INT, 0, 1,
			_T("index"), 0, TYPE_INT,

		end
);

static FPInterfaceDesc pc_ID_IDW(
			PARTICLECHANNELIDW_INTERFACE,
			_T("idWrite"), 0,
			&TheParticleChannelIDDesc, FP_MIXIN,

		particleChannelID_setID, _T("setID"), 0, TYPE_VOID, 0, 3,
			_T("index"), 0, TYPE_INT,
			_T("particleIndex"), 0, TYPE_INT,
			_T("particleBorn"), 0, TYPE_INT,

		end
);

FPInterfaceDesc* ParticleChannelID::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_ID_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_ID_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_ID_AmountW;
	if (id == GetReadID()) return &pc_ID_IDR;
	if (id == GetWriteID()) return &pc_ID_IDW;
	return NULL;
}

Class_ID ParticleChannelID::GetClassID() const
{
	return ParticleChannelID_Class_ID;
}

IObject*  ParticleChannelID::Clone() const
{
	ParticleChannelID* newChannel = (ParticleChannelID*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelID_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_data() = data();	
	return newChannel;
}

IOResult ParticleChannelID::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int i, num;

	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = data().Count();
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

	if (num > 0)
	{
		ParticleID* idData = new ParticleID[num];
		for(i=0; i<num; i++) idData[i] = data(i);
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = isave->Write(idData, sizeof(ParticleID)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
		delete [] idData;
	}

	return IO_OK;
}

IOResult ParticleChannelID::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num;
	ParticleID* idData;
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
		case IParticleChannel::kChunkData:
			num = data().Count();
			DbgAssert(num > 0);
			idData = new ParticleID[num];
			res=iload->Read(idData, sizeof(ParticleID)*num, &nb);
			if (res == IO_OK)
				for(i=0; i<num; i++)
					_data(i) = idData[i];
			delete [] idData;
			idData = NULL;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelID::MemoryUsed() const
{
	return (data().Count()*sizeof(ParticleID));
}

int ParticleChannelID::Count() const
{
	return data().Count();
}

void ParticleChannelID::ZeroCount()
{
	_data().ZeroCount();
}

bool ParticleChannelID::SetCount(int n)
{
	if (n < 0) return false;
	_data().SetCount(n);
	return true;
}

int ParticleChannelID::Delete(int start, int num)
{
	return _data().Delete(start,num);
}

int ParticleChannelID::Delete(BitArray& toRemove)
{	
	SysUtil::NeedToImplementLater(); // optimize the implementation

	Tab<ParticleID> oldData(data());

	int newCount = 0;
	int checkCount = min(toRemove.GetSize(), data().Count());
	for(int i=0; i<checkCount; i++)
		if (toRemove[i] == 0)
			_data(newCount++) = oldData[i];
	_data().Resize(newCount);	
	return data().Count();
}

IObject* ParticleChannelID::Split(BitArray& toSplit)
{
	ParticleChannelID* newChannel = (ParticleChannelID*)Clone();
	Delete(toSplit);

	int n1 = toSplit.GetSize();
	int n2 = toSplit.NumberSet();




	BitArray reverse = ~toSplit;


	int n3 = reverse.GetSize();
	int n4 = reverse.NumberSet();



	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelID::Spawn( Tab<int>& spawnTable )
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	Tab<ParticleID> oldData(data());
	int i, j, k, newCount = 0, checkCount = min(spawnTable.Count(), data().Count());
	for(i=0; i<checkCount; i++)
		newCount += spawnTable[i];
	_data().SetCount(newCount);
	for(i=0, j=0; i<checkCount; i++)
		for(k=0; k<spawnTable[i]; k++)
			_data(j++) = oldData[i];
	return true;
}

bool ParticleChannelID::AppendNum(int num)
{
	if (num <= 0) return true;

	int oldCount = data().Count();
	int newCount = oldCount + num;

	if (_data().Resize(newCount) == 0) return false;
	_data().SetCount(newCount);

	ParticleID idToAppend;
	idToAppend.index = idToAppend.born = 0;
	for(int i=oldCount; i<newCount; i++)
		_data(i) = idToAppend;

	return true;
}

bool ParticleChannelID::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelIDR* iID = GetParticleChannelIDRInterface(channel);
	DbgAssert(iID);
	if (iID == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetID(oldCount + i, iID->GetID(i) );

	return true;
}

void ParticleChannelID::GetID(int index, int& particleIndex, int& particleBorn) const
{
	DbgAssert((index>=0) && (index < data().Count()));
	particleIndex = data(index).index;
	particleBorn = data(index).born;
}

int	ParticleChannelID::GetParticleIndex(int index) const
{
	DbgAssert((index>=0) && (index < data().Count()));
	return data(index).index;
}

int	ParticleChannelID::GetParticleBorn(int index) const
{
	DbgAssert((index>=0) && (index < data().Count()));
	return data(index).born;
}

const ParticleID& ParticleChannelID::GetID(int index) const
{
	DbgAssert((index>=0) && (index < data().Count()));
	return data(index);
}

void ParticleChannelID::SetID(int index, int particleIndex, int particleBorn)
{
	DbgAssert((index>=0) && (index < data().Count()));
	_data(index).index = particleIndex;
	_data(index).born = particleBorn;
}

void ParticleChannelID::SetID(int index, const ParticleID& id)
{
	DbgAssert((index>=0) && (index < data().Count()));
	_data(index) = id;
}


} // end of namespace PFChannels