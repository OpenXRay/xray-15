/**********************************************************************
 *<
	FILE: ParticleChannelPoint3.cpp

	DESCRIPTION: ParticleChannelPoint3 implementation
				 This generic channel is used to store 3D vector data

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-30-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelPoint3.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelPoint3::ParticleChannelPoint3()
					:IParticleChannel(PARTICLECHANNELPOINT3R_INTERFACE,PARTICLECHANNELPOINT3W_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;
	_globalValue() = Point3::Origin;
	_boundingBox().Init();
	_maxLength() = 0.0f;
}

ParticleChannelPoint3::~ParticleChannelPoint3()
{
	; // implement later
}

TCHAR* ParticleChannelPoint3::GetIObjectName()
{
	return _T("ParticleChannelPoint3");
}

BaseInterface* ParticleChannelPoint3::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelPoint3R*)this;
	case 4: return (IParticleChannelPoint3W*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelPoint3::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelPoint3R*)this;
	if (id == GetWriteID())
		return (IParticleChannelPoint3W*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelPoint3::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelPoint3::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelPoint3::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelPoint3Desc TheParticleChannelPoint3Desc;

static FPInterfaceDesc pc_Point3_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelPoint3Desc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Point3_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelPoint3Desc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Point3_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelPoint3Desc, FP_MIXIN,

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

static FPInterfaceDesc pc_Point3_Point3R(
			PARTICLECHANNELPOINT3R_INTERFACE,
			_T("point3Read"), 0,
			&TheParticleChannelPoint3Desc, FP_MIXIN,

		IParticleChannelPoint3R::kGetValue, _T("getValue"), 0, TYPE_POINT3_BR, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelPoint3R::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelPoint3R::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_POINT3_BR, 0, 0,
		IParticleChannelPoint3R::kGetBoundingBox, _T("getBoundingBox"), 0, TYPE_VOID, 0, 2,
			_T("minCorner"), 0, TYPE_POINT3_BR,
			_T("maxCorner"), 0, TYPE_POINT3_BR,
		IParticleChannelPoint3R::kGetMaxLengthValue, _T("getMaxLength"), 0, TYPE_FLOAT, 0, 0,

		end
);

static FPInterfaceDesc pc_Point3_Point3W(
			PARTICLECHANNELPOINT3W_INTERFACE,
			_T("point3Write"), 0,
			&TheParticleChannelPoint3Desc, FP_MIXIN,

		IParticleChannelPoint3W::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_POINT3_BR,
		IParticleChannelPoint3W::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_POINT3_BR,
		IParticleChannelPoint3W::kBuildBoundingBox, _T("buildBoundingBox"), 0, TYPE_VOID, 0, 0,
		IParticleChannelPoint3W::kUpdateMaxLengthValue, _T("updateMaxLength"), 0, TYPE_VOID, 0, 0,

		end
);

FPInterfaceDesc* ParticleChannelPoint3::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Point3_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Point3_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Point3_AmountW;
	if (id == GetReadID()) return &pc_Point3_Point3R;
	if (id == GetWriteID()) return &pc_Point3_Point3W;
	return NULL;
}

Class_ID ParticleChannelPoint3::GetClassID() const
{
	return ParticleChannelPoint3_Class_ID;
}

IObject*  ParticleChannelPoint3::Clone() const
{
	ParticleChannelPoint3* newChannel = (ParticleChannelPoint3*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelPoint3_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_data() = data();	
	newChannel->_boundingBox() = boundingBox();
	newChannel->_maxLength() = maxLength();
	return newChannel;
}

IOResult ParticleChannelPoint3::Save(ISave* isave) const
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
		if ((res = isave->Write(data().Addr(0), sizeof(Point3)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	Point3 p = globalValue();
	if ((res = isave->Write(&p, sizeof(Point3), &nb)) != IO_OK) return res;
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

	isave->BeginChunk(IParticleChannel::kChunkValue2);
	Box3 box = boundingBox();
	if ((res = isave->Write(&box, sizeof(Box3), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkValue3);
	float maxL = maxLength();
	if ((res = isave->Write(&maxL, sizeof(float), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult ParticleChannelPoint3::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	Point3 p;
	Interface_ID id;
	bool isg;
	Box3 box;
	float maxL;

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
			res=iload->Read(_data().Addr(0), sizeof(Point3)*num, &nb);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&p, sizeof(Point3), &nb);
			if (res == IO_OK) _globalValue() = p;
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
		case IParticleChannel::kChunkValue2:
			res=iload->Read(&box, sizeof(Box3), &nb);
			if (res == IO_OK) _boundingBox() = box;
			break;
		case IParticleChannel::kChunkValue3:
			res=iload->Read(&maxL, sizeof(float), &nb);
			if (res == IO_OK) _maxLength() = maxL;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelPoint3::MemoryUsed() const
{
	int mem = sizeof(bool) + sizeof(int) + sizeof(Point3) + sizeof(Box3) + sizeof(float);
	mem += data().Count()*sizeof(Point3);
	return mem;
}

int ParticleChannelPoint3::Count() const
{
	if (isGlobal()) return globalCount();
	return data().Count();
}

void ParticleChannelPoint3::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else _data().ZeroCount();
}

bool ParticleChannelPoint3::SetCount(int n)
{
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else _data().SetCount(n);
	return true;
}

int ParticleChannelPoint3::Delete(int start, int num)
{
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

int ParticleChannelPoint3::Delete(BitArray& toRemove)
{	
	SysUtil::NeedToImplementLater(); // optimize the implementation

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
		Tab<Point3> oldData(data());

		int newCount = 0;
		for(int i=0; i<checkCount; i++)
			if (toRemove[i] == 0)
				_data(newCount++) = oldData[i];
		_data().Resize(newCount);	
		return data().Count();
	}
}

IObject* ParticleChannelPoint3::Split(BitArray& toSplit)
{
	ParticleChannelPoint3* newChannel = (ParticleChannelPoint3*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelPoint3::Spawn( Tab<int>& spawnTable )
{
	SysUtil::NeedToImplementLater(); // optimize the implementation

	int i, checkCount = min(spawnTable.Count(), Count());

	if (isGlobal())
	{
		int newCount = 0;
		for(i=0; i<checkCount; i++)
			newCount += spawnTable[i];
		_globalCount() = newCount;
	}
	else
	{
		Tab<Point3> oldData(data());
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

bool ParticleChannelPoint3::AppendNum(int num)
{
	if (isGlobal())
	{
		_globalCount() += num;
		return true;
	}

	if (num <= 0) return true;

	int oldCount = data().Count();
	int newCount = oldCount + num;

	if (_data().Resize(newCount) == 0) return false;
	_data().SetCount(newCount);

	for(int i=oldCount; i<newCount; i++)
		_data(i) = Point3::Origin;

	return true;
}

bool ParticleChannelPoint3::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelPoint3R* iPoint3 = (IParticleChannelPoint3R*)(channel->GetInterface(GetReadID()));
	DbgAssert(iPoint3);
	if (iPoint3 == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iPoint3->GetValue(i) );

//	BuildBoundingBox();
//	UpdateMaxLengthValue();

	return true;
}

const Point3& ParticleChannelPoint3::GetValue(int index) const
{
	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

bool ParticleChannelPoint3::IsGlobal() const
{
	return isGlobal();
}

const Point3& ParticleChannelPoint3::GetValue() const
{
	if (isGlobal()) return globalValue();
	if (data().Count() <= 0)
	{
		return Point3::Origin;
	}
	DbgAssert(data().Count()>0);
	return data(0);
}

const Box3&	ParticleChannelPoint3::GetBoundingBox() const
{
	return boundingBox();
}

float ParticleChannelPoint3::GetMaxLengthValue() const
{
	return maxLength();
}

void ParticleChannelPoint3::SetValue(int index, const Point3& v)
{
	if (isGlobal())
	{
		if (v.Equals(globalValue())) return; // nothing to change
		_isGlobal() = false;
		DbgAssert((index>=0) && (index < globalCount()));
		_data().SetCount(globalCount());
		for(int i=0; i<globalCount(); i++)
			_data(i) = globalValue();
	}
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = v;
}

void ParticleChannelPoint3::SetValue(const Point3& v)
{
	if (!isGlobal())
	{
		_isGlobal() = true;
		_globalCount() = data().Count();
		_data().Resize(0);
	}
	_globalValue() = v;
}

void ParticleChannelPoint3::BuildBoundingBox()
{
	_boundingBox().Init();
	if (isGlobal())
	{
		_boundingBox() += globalValue();
	}
	else
	{
		for(int i=0; i<data().Count(); i++)
			_boundingBox() += data(i);
	}
}

void ParticleChannelPoint3::UpdateMaxLengthValue()
{
	if (isGlobal())
		_maxLength() = Length(globalValue());
	else
	{
		float curVal;
		_maxLength() = 0.0f;
		for(int i=0; i<data().Count(); i++)
		{
			curVal = Length(_data(i));
			if (curVal > maxLength())
				_maxLength() = curVal;
		}
	}
}


} // end of namespace PFChannels


