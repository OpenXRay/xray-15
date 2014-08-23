/**********************************************************************
 *<
	FILE: ParticleChannelMap.cpp

	DESCRIPTION: ParticleChannelMap implementation
				 This generic channel is used to store map data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelMap.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelMap::ParticleChannelMap()
					:IParticleChannel(PARTICLECHANNELMAPR_INTERFACE,PARTICLECHANNELMAPW_INTERFACE)
{
	_chanUVVert() = NULL;
	_chanTVFace() = NULL;
}

ParticleChannelMap::~ParticleChannelMap()
{
	// free memory allocated for Maps
	if (chanUVVert()) delete _chanUVVert();
	if (chanTVFace()) delete _chanTVFace();
}

TCHAR* ParticleChannelMap::GetIObjectName()
{
	return _T("ParticleChannelMap");
}

BaseInterface* ParticleChannelMap::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelMapR*)this;
	case 4: return (IParticleChannelMapW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelMap::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelMapR*)this;
	if (id == GetWriteID())
		return (IParticleChannelMapW*)this;
	if ((id == PARTICLECHANNELTABUVVERTR_INTERFACE) || (id == PARTICLECHANNELTABUVVERTW_INTERFACE))
		if (chanUVVert()) return _chanUVVert()->GetInterface(id);
	if ((id == PARTICLECHANNELTABTVFACER_INTERFACE) || (id == PARTICLECHANNELTABTVFACEW_INTERFACE))
		if (chanTVFace()) return _chanTVFace()->GetInterface(id);
	return IObject::GetInterface(id);
}

void ParticleChannelMap::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelMap::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelMap::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelMapDesc TheParticleChannelMapDesc;

static FPInterfaceDesc pc_Map_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelMapDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Map_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelMapDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Map_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelMapDesc, FP_MIXIN,

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

FPInterfaceDesc* ParticleChannelMap::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Map_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Map_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Map_AmountW;
	return NULL;
}

Class_ID ParticleChannelMap::GetClassID() const
{
	return ParticleChannelMap_Class_ID;
}

IObject* ParticleChannelMap::Clone() const
{
	ParticleChannelMap* newChannel = (ParticleChannelMap*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelMap_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	if (chanUVVert() != NULL) {
		IObject* cloneChanUVVert = chanUVVert()->Clone();
		newChannel->InitUVVertChannel((ParticleChannelTabUVVert*)(cloneChanUVVert));
		cloneChanUVVert->DeleteIObject();
	}
	if (chanTVFace() != NULL) {
		IObject* cloneChanTVFace = chanTVFace()->Clone();
		newChannel->InitTVFaceChannel((ParticleChannelTabTVFace*)(cloneChanTVFace));
		cloneChanTVFace->DeleteIObject();
	}
	return newChannel;
}

IOResult ParticleChannelMap::Save(ISave* isave) const
{
	IOResult res;
	ULONG nb;

	// UVVertChannel
	if (chanUVVert() != NULL) {
		isave->BeginChunk(kChunkUVVertChannel);
		if ((res = chanUVVert()->Save(isave)) != IO_OK) return res;
		isave->EndChunk();
	}

	// TVFaceChannel
	if (chanTVFace() != NULL) {
		isave->BeginChunk(kChunkTVFaceChannel);
		if ((res = chanTVFace()->Save(isave)) != IO_OK) return res;
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

IOResult ParticleChannelMap::Load(ILoad* iload)
{
	IOResult res;
	Interface_ID id;
	ULONG nb;
	bool isg;

	while ((res = iload->OpenChunk()) == IO_OK)
	{
		switch(iload->CurChunkID())
		{
		case kChunkUVVertChannel:
			if (chanUVVert() == NULL)
				_chanUVVert() = new ParticleChannelTabUVVert();
			if (chanUVVert() != NULL)
				res = _chanUVVert()->Load(iload);
			else 
				res = IO_ERROR;
			break;
		case kChunkTVFaceChannel:
			if (chanTVFace() == NULL)
				_chanTVFace() = new ParticleChannelTabTVFace();
			if (chanTVFace() != NULL)
				res = _chanTVFace()->Load(iload);
			else 
				res = IO_ERROR;
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

int ParticleChannelMap::MemoryUsed() const
{
	int mem = sizeof(void*)*2;
	if (chanUVVert() != NULL)
		mem += chanUVVert()->MemoryUsed();
	if (chanTVFace() != NULL)
		mem += chanTVFace()->MemoryUsed();
	return mem;
}

int ParticleChannelMap::Count() const
{
	int uvVertCount = chanUVVert() ? chanUVVert()->Count() : 0;
	int tvFaceCount = chanTVFace() ? chanTVFace()->Count() : 0;
	DbgAssert(uvVertCount >= 0);
	DbgAssert(tvFaceCount >= 0);
	if ((uvVertCount < 0) || (tvFaceCount < 0)) return 0;

	if (uvVertCount == 0) return 0;
	if (tvFaceCount == 0) return uvVertCount;
	if (uvVertCount != tvFaceCount) return 0;
	return uvVertCount;
}

void ParticleChannelMap::ZeroCount()
{
	if (chanUVVert() != NULL) _chanUVVert()->ZeroCount();
	if (chanTVFace() != NULL) _chanTVFace()->ZeroCount();
}

bool ParticleChannelMap::SetCount(int n)
{
	if (n < 0)	return false;
	if (n == 0)	{
		ZeroCount();
		return true;
	}

	if (chanUVVert() == NULL)
		_chanUVVert() = new ParticleChannelTabUVVert();
	DbgAssert(chanUVVert());
	if (chanUVVert() == NULL) return false;

	if (chanTVFace() == NULL)
		_chanTVFace() = new ParticleChannelTabTVFace();
	DbgAssert(chanTVFace());
	if (chanTVFace() == NULL) return false;
	
	bool res1 = _chanUVVert()->SetCount(n);
	bool res2 = _chanTVFace()->SetCount(n);
	return (res1 && res2);
}

int ParticleChannelMap::Delete(int start, int num)
{
	if (start < 0) {
		num += start;
		start = 0;
	}
	if (num <= 0)	return Count();

	if (chanUVVert() != NULL) _chanUVVert()->Delete(start, num);
	if (chanTVFace() != NULL) _chanTVFace()->Delete(start, num);

	return Count();
}

int ParticleChannelMap::Delete(BitArray& toRemove)
{	
	if (chanUVVert() != NULL) _chanUVVert()->Delete(toRemove);
	if (chanTVFace() != NULL) _chanTVFace()->Delete(toRemove);
	return Count();
}

IObject* ParticleChannelMap::Split(BitArray& toSplit)
{
	// SysUtil::NeedToImplementLater(); // TODO: optimize the implementation

	ParticleChannelMap* newChannel = (ParticleChannelMap*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	if (reverse.GetSize() != newChannel->Count())
		reverse.SetSize(newChannel->Count(), TRUE);
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelMap::Spawn(Tab<int>& spawnTable)
{
	bool res1 = true;
	if (chanUVVert() != NULL) res1 = _chanUVVert()->Spawn(spawnTable);
	bool res2 = true;
	if (chanTVFace() != NULL) res2 = _chanTVFace()->Spawn(spawnTable);
	return (res1 && res2);
}

bool ParticleChannelMap::AppendNum(int num)
{
	if (num < 0)	return false;
	if (num == 0)	return true;

	if (chanUVVert() == NULL)
		_chanUVVert() = new ParticleChannelTabUVVert();
	DbgAssert(chanUVVert());
	if (chanUVVert() == NULL) return false;

	if (chanTVFace() == NULL)
		_chanTVFace() = new ParticleChannelTabTVFace();
	DbgAssert(chanTVFace());
	if (chanTVFace() == NULL) return false;
	
	bool res1 = _chanUVVert()->AppendNum(num);
	bool res2 = _chanTVFace()->AppendNum(num);
	return (res1 && res2);
}

bool ParticleChannelMap::Append(IObject* channel)
{
	IParticleChannelMapW* iMap = GetParticleChannelMapWInterface(channel);
	DbgAssert(iMap);
	if (iMap == NULL) return false;

	IObject* iUVVert = iMap->GetUVVertChannel();
	IObject* iTVFace = iMap->GetTVFaceChannel();

	bool res1=true, res2=true;

	if (iUVVert != NULL) {
		if (chanUVVert() == NULL)
			_chanUVVert() = new ParticleChannelTabUVVert();
		DbgAssert(chanUVVert());
		if (chanUVVert() == NULL) return false;
		res1 = _chanUVVert()->Append(iUVVert);
	}

	if (iTVFace != NULL) {
		if (chanTVFace() == NULL)
			_chanTVFace() = new ParticleChannelTabTVFace();
		DbgAssert(chanTVFace());
		if (chanTVFace() == NULL) return false;
		res2 = _chanTVFace()->Append(iTVFace);
	}

	return (res1 && res2);
}

bool ParticleChannelMap::IsUVVertShared() const
{
	if (chanUVVert() == NULL) return true;
	return chanUVVert()->IsShared();
}

bool ParticleChannelMap::IsTVFaceShared() const
{
	if (chanTVFace() == NULL) return true;
	return chanTVFace()->IsShared();
}

int	ParticleChannelMap::GetUVVertCount() const
{
	if (chanUVVert() == NULL) return 0;
	return chanUVVert()->GetValueCount();
}

int	ParticleChannelMap::GetUVVertIndex(int particleIndex) const
{
	if (chanUVVert() == NULL) return -1;
	return chanUVVert()->GetValueIndex(particleIndex);
}

const TabUVVert* ParticleChannelMap::GetUVVertByIndex(int valueIndex) const
{
	if (chanUVVert() == NULL) return NULL;
	return chanUVVert()->GetValueByIndex(valueIndex);
}

const TabUVVert* ParticleChannelMap::GetUVVert(int particleIndex) const
{
	if (chanUVVert() == NULL) return NULL;
	return chanUVVert()->GetValue(particleIndex);
}

const TabUVVert* ParticleChannelMap::GetUVVert() const
{
	if (chanUVVert() == NULL) return NULL;
	return chanUVVert()->GetValue();
}

int	ParticleChannelMap::GetTVFaceCount() const
{
	if (chanTVFace() == NULL) return 0;
	return chanTVFace()->GetValueCount();
}

int	ParticleChannelMap::GetTVFaceIndex(int particleIndex) const
{
	if (chanTVFace() == NULL) return 0;
	return chanTVFace()->GetValueIndex(particleIndex);
}

const TabTVFace* ParticleChannelMap::GetTVFaceByIndex(int valueIndex) const
{
	if (chanTVFace() == NULL) return NULL;
	return chanTVFace()->GetValueByIndex(valueIndex);
}

const TabTVFace* ParticleChannelMap::GetTVFace(int particleIndex) const
{
	if (chanTVFace() == NULL) return NULL;
	return chanTVFace()->GetValue(particleIndex);
}

const TabTVFace* ParticleChannelMap::GetTVFace() const
{
	if (chanTVFace() == NULL) return NULL;
	return chanTVFace()->GetValue();
}

// apply the same texture coords to all texture vertices
void ApplyMonoMapping(Mesh* mesh, int mp, UVVert mapCoords)
{
	TVFace tvFace;
	tvFace.setTVerts(0, 0, 0);
	int faceNum = mesh->getNumFaces();
	mesh->setMapSupport(mp, TRUE);
	mesh->setNumMapFaces(mp, faceNum);
	mesh->setNumMapVerts(mp, 1);
	mesh->setMapVert(mp, 0, mapCoords);
	for(int i=0; i<faceNum; i++)
		mesh->mapFaces(mp)[i] = tvFace;
}

// number of texture vertices is the same as the number of geom vertices
void ApplyPlanarMapping(Mesh* mesh, int mp, UVVert* mapCoords)
{
	mesh->setMapSupport(mp, TRUE);
	mesh->MakeMapPlanar(mp);
	int vertNum = mesh->getNumMapVerts(mp);
	for(int i=0; i<vertNum; i++)
		mesh->setMapVert(mp, i, mapCoords[i]);
}

void ApplyCustomMapping(Mesh* mesh, int mp, TabUVVert* uvvert, TabTVFace* tvface)
{
	int i;
	int numVerts = uvvert->Count(); 
	int numFaces = tvface->Count();
	mesh->setMapSupport(mp, TRUE);
	mesh->setNumMapVerts(mp, numVerts);
	for(i=0; i<numVerts; i++)
		mesh->setMapVert(mp, i, (*uvvert)[i]);
	mesh->setNumMapFaces(mp, numFaces);
	for(i=0; i<numFaces; i++)
		mesh->mapFaces(mp)[i] = (*tvface)[i];
}

void ParticleChannelMap::Apply(Mesh* mesh, int particleIndex, int mp) const
{
	TabUVVert* uvvert = const_cast <TabUVVert*>(GetUVVert(particleIndex));
	if (uvvert == NULL) return;
	int tVertCount = uvvert->Count();
	if (tVertCount == 0) return;
	TabTVFace* tvface = const_cast <TabTVFace*>(GetTVFace(particleIndex));
	if (tvface == NULL) {
		if (tVertCount == mesh->getNumVerts())
			ApplyPlanarMapping(mesh, mp, uvvert->Addr(0));
		else
			ApplyMonoMapping(mesh, mp, (*uvvert)[0]);
	} else {
		if (tvface->Count() == mesh->getNumFaces())
			ApplyCustomMapping(mesh, mp, uvvert, tvface);
		else 
			ApplyMonoMapping(mesh, mp, (*uvvert)[0]);
	}
}

bool ParticleChannelMap::SetUVVert(int particleIndex, const UVVert& value)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(particleIndex, value);
}

bool ParticleChannelMap::SetUVVert(int particleIndex, const TabUVVert* tab)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(particleIndex, tab);
}

bool ParticleChannelMap::SetUVVert(Tab<int>& particleIndices, const UVVert& value)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(particleIndices, value);
}

bool ParticleChannelMap::SetUVVert(Tab<int>& particleIndices, const TabUVVert* tab)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(particleIndices, tab);
}

bool ParticleChannelMap::SetUVVert(const UVVert& value)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(value);
}

bool ParticleChannelMap::SetUVVert(const TabUVVert* tab)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->SetValue(tab);
}

bool ParticleChannelMap::CopyUVVert(int fromParticle, int toParticle)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->CopyValue(fromParticle, toParticle);
}

bool ParticleChannelMap::CopyUVVert(int fromParticle, Tab<int>& toParticles)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->CopyValue(fromParticle, toParticles);
}

bool ParticleChannelMap::CopyUVVert(int fromParticle)
{
	if (chanUVVert() == NULL) return false;
	return _chanUVVert()->CopyValue(fromParticle);
}

bool ParticleChannelMap::SetTVFace(int particleIndex, const TabTVFace* tab)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->SetValue(particleIndex, tab);
}

bool ParticleChannelMap::SetTVFace(Tab<int>& particleIndices, const TabTVFace* tab)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->SetValue(particleIndices, tab);
}

bool ParticleChannelMap::SetTVFace(const TabTVFace* tab)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->SetValue(tab);
}

bool ParticleChannelMap::CopyTVFace(int fromParticle, int toParticle)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->CopyValue(fromParticle, toParticle);
}

bool ParticleChannelMap::CopyTVFace(int fromParticle, Tab<int>& toParticles)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->CopyValue(fromParticle, toParticles);
}

bool ParticleChannelMap::CopyTVFace(int fromParticle)
{
	if (chanTVFace() == NULL) return false;
	return _chanTVFace()->CopyValue(fromParticle);
}

IObject* ParticleChannelMap::GetUVVertChannel() const
{
	return (IObject*)chanUVVert();
}

IObject* ParticleChannelMap::GetTVFaceChannel() const
{
	return (IObject*)chanTVFace();
}

bool ParticleChannelMap::InitUVVertChannel(const ParticleChannelTabUVVert* channel)
{
	if (chanUVVert() != NULL) delete _chanUVVert();
	_chanUVVert() = new ParticleChannelTabUVVert(channel);
	return true;
}

bool ParticleChannelMap::InitTVFaceChannel(const ParticleChannelTabTVFace* channel)
{
	if (chanTVFace() != NULL) delete _chanTVFace();
	_chanTVFace() = new ParticleChannelTabTVFace(channel);
	return true;
}

} // end of namespace PFChannels
