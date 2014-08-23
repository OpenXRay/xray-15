/**********************************************************************
 *<
	FILE: ParticleChannelMesh.cpp

	DESCRIPTION: ParticleChannelMesh implementation
				 This generic channel is used to store mesh data.

	CREATED BY: Chung-An Lin

	HISTORY: created 12-04-01
			 modified 12-18-01 (methods IsSimilarChannel, AppendNum,
							Append added - Bayboro)

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelMesh.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelMesh::ParticleChannelMesh()
					:IParticleChannel(PARTICLECHANNELMESHR_INTERFACE,PARTICLECHANNELMESHW_INTERFACE)
{
	_globalData() = NULL;
	_globalCount() = 0;
	_dataType() = kGlobalData;
	_maxBoundingBox().Init();
}

ParticleChannelMesh::~ParticleChannelMesh()
{
	// free memory allocated for Meshes
	if (globalData())
		delete _globalData();
	for (int i = 0; i < sharedData().Count(); i++)
		if (sharedData(i))	delete _sharedData(i);
	for (i = 0; i < localData().Count(); i++)
		if (localData(i))	delete _localData(i);
}

TCHAR* ParticleChannelMesh::GetIObjectName()
{
	return _T("ParticleChannelMesh");
}

BaseInterface* ParticleChannelMesh::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelMeshR*)this;
	case 4: return (IParticleChannelMeshW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelMesh::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelMeshR*)this;
	if (id == GetWriteID())
		return (IParticleChannelMeshW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelMesh::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelMesh::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelMesh::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelMeshDesc TheParticleChannelMeshDesc;

static FPInterfaceDesc pc_Mesh_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelMeshDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_Mesh_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelMeshDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Mesh_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelMeshDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_Mesh_MeshR(
			PARTICLECHANNELMESHR_INTERFACE,
			_T("meshRead"), 0,
			&TheParticleChannelMeshDesc, FP_MIXIN,

		IParticleChannelMeshR::kIsShared, _T("isShared"), 0, TYPE_bool, 0, 0,
		IParticleChannelMeshR::kGetValueCount, _T("getValueCount"), 0, TYPE_INT, 0, 0,
		IParticleChannelMeshR::kGetValueIndex, _T("getValueIndex"), 0, TYPE_INT, 0, 1,
			_T("particleIndex"), 0, TYPE_INT,
		IParticleChannelMeshR::kGetValueByIndex, _T("getValueByIndex"), 0, TYPE_MESH, 0, 1,
			_T("valueIndex"), 0, TYPE_INT,
		IParticleChannelMeshR::kGetValue, _T("getValue"), 0, TYPE_MESH, 0, 1,
			_T("particleIndex"), 0, TYPE_INT,
		IParticleChannelMeshR::kGetValueFirst, _T("getValueFirst"), 0, TYPE_MESH, 0, 0,
		IParticleChannelMeshR::kGetMaxBoundingBox, _T("getMaxBoundingBox"), 0, TYPE_VOID, 0, 2,
			_T("minCorner"), 0, TYPE_POINT3_BR,
			_T("maxCorner"), 0, TYPE_POINT3_BR,

		end
);

static FPInterfaceDesc pc_Mesh_MeshW(
			PARTICLECHANNELMESHW_INTERFACE,
			_T("meshWrite"), 0,
			&TheParticleChannelMeshDesc, FP_MIXIN,

		IParticleChannelMeshW::kSetValue, _T("setValue"), 0, TYPE_bool, 0, 2,
			_T("particleIndex"), 0, TYPE_INT,
			_T("mesh"), 0, TYPE_MESH,
		IParticleChannelMeshW::kSetValueMany, _T("setValueMany"), 0, TYPE_bool, 0, 2,
			_T("particleIndices"), 0, TYPE_INT_TAB_BR,
			_T("mesh"), 0, TYPE_MESH,
		IParticleChannelMeshW::kSetValueAll, _T("setValueAll"), 0, TYPE_bool, 0, 1,
			_T("mesh"), 0, TYPE_MESH,
		IParticleChannelMeshW::kBuildMaxBoundingBox, _T("buildMaxBoundingBox"), 0, TYPE_VOID, 0, 0,

		end
);

FPInterfaceDesc* ParticleChannelMesh::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Mesh_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Mesh_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Mesh_AmountW;
	if (id == GetReadID()) return &pc_Mesh_MeshR;
	if (id == GetWriteID()) return &pc_Mesh_MeshW;
	return NULL;
}

Class_ID ParticleChannelMesh::GetClassID() const
{
	return ParticleChannelMesh_Class_ID;
}

IObject* ParticleChannelMesh::Clone() const
{
	ParticleChannelMesh* newChannel = (ParticleChannelMesh*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelMesh_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);

	newChannel->_localData().SetCount(localData().Count());
	for (int i = 0; i < localData().Count(); i++)
		newChannel->_localData(i) = (localData(i)) ? new Mesh(*localData(i)) : NULL;

	newChannel->_sharedData().SetCount(sharedData().Count());
	for (i = 0; i < sharedData().Count(); i++)
		newChannel->_sharedData(i) = (sharedData(i)) ? new Mesh(*sharedData(i)) : NULL;

	newChannel->_globalData() = (globalData()) ? new Mesh(*globalData()) : NULL;

	newChannel->_sharedTable() = sharedTable();
	newChannel->_globalCount() = globalCount();
	newChannel->_dataType() = dataType();
	newChannel->_maxBoundingBox() = maxBoundingBox();

	return newChannel;
}

IOResult ParticleChannelMesh::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	Mesh* meshData;
	int num;

	// local data
	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = localData().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0) {
		isave->BeginChunk(IParticleChannel::kChunkData);
		for (int i = 0; i < num; i++) {
			isave->BeginChunk(IParticleChannel::kChunkValue1);
			if (localData(i)) {
				meshData = const_cast <Mesh*> (localData(i));
				if ((res = meshData->Save(isave)) != IO_OK) return res;
			}
			isave->EndChunk();
		}
		isave->EndChunk();
	}

	// shared data
	isave->BeginChunk(IParticleChannel::kChunkSharedCount);
	num = sharedData().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0) {
		isave->BeginChunk(IParticleChannel::kChunkSharedValue);
		for (int i = 0; i < num; i++) {
			isave->BeginChunk(IParticleChannel::kChunkValue1);
			if (sharedData(i)) {
				meshData = const_cast <Mesh*> (sharedData(i));
				if ((res = meshData->Save(isave)) != IO_OK) return res;
			}
			isave->EndChunk();
		}
		isave->EndChunk();
	}

	// global data
	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	if (globalData()) {
		meshData = const_cast <Mesh*> (globalData());
		if ((res = meshData->Save(isave)) != IO_OK) return res;
	}
	isave->EndChunk();

	// shared table
	isave->BeginChunk(IParticleChannel::kChunkValue2);
	num = sharedTable().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	if (num > 0)
		if ((res = isave->Write(sharedTable().Addr(0), sizeof(int)*num, &nb)) != IO_OK) return res;
	isave->EndChunk();

	// channel type
	isave->BeginChunk(IParticleChannel::kChunkValue3);
	num = dataType();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	// maximum bounding box
	isave->BeginChunk(IParticleChannel::kChunkValue4);
	Box3 box = maxBoundingBox();
	if ((res = isave->Write(&box, sizeof(Box3), &nb)) != IO_OK) return res;
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

IOResult ParticleChannelMesh::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num;
	Box3 box;
	Interface_ID id;
	bool isg;

	while ((res = iload->OpenChunk()) == IO_OK)
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res = iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _localData().SetCount(num);
			break;
		case IParticleChannel::kChunkData:
			num = localData().Count();
			DbgAssert(num > 0);
			for (i = 0; i < num; i++) {
				if ((res = iload->OpenChunk()) == IO_OK) {
					if (iload->CurChunkLength() == 0) {
						_localData(i) = NULL;
					} else {
						_localData(i) = new Mesh();
						_localData(i)->Load(iload);
					}
					iload->CloseChunk();
				} else {
					_localData(i) = NULL;
				}
			}
			break;
		case IParticleChannel::kChunkSharedCount:
			res = iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _sharedData().SetCount(num);
			break;
		case IParticleChannel::kChunkSharedValue:
			num = sharedData().Count();
			DbgAssert(num > 0);
			for (i = 0; i < num; i++) {
				if ((res = iload->OpenChunk()) == IO_OK) {
					if (iload->CurChunkLength() == 0) {
						_sharedData(i) = NULL;
					} else {
						_sharedData(i) = new Mesh();
						_sharedData(i)->Load(iload);
					}
					iload->CloseChunk();
				} else {
					_sharedData(i) = NULL;
				}
			}
			break;
		case IParticleChannel::kChunkGlobalCount:
			res = iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			if (iload->CurChunkLength() == 0) {
				_globalData() = NULL;
			} else {
				_globalData() = new Mesh();
				_globalData()->Load(iload);
			}
			break;
		case IParticleChannel::kChunkValue2:	// shared table
			res = iload->Read(&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			_sharedTable().SetCount(num);
			if (num > 0)
				res = iload->Read(_sharedTable().Addr(0), sizeof(int)*num, &nb);
			break;
		case IParticleChannel::kChunkValue3:	// channel type
			res = iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) {
				switch (num) {
				case kGlobalData:
				case kSharedData:
				case kLocalData:
					_dataType() = num;
				}
			}
			break;
		case IParticleChannel::kChunkValue4:
			res=iload->Read(&box, sizeof(Box3), &nb);
			if (res == IO_OK) _maxBoundingBox() = box;
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

int MeshMapMemoryUsed(MeshMap* map)
{
	if (map == NULL) return 0;
	int mem = sizeof(DWORD) + sizeof(int)*2;
	mem += map->vnum*sizeof(UVVert) + map->fnum*sizeof(TVFace);
	return mem;
}

int PerDataMemoryUsed(PerData* pd)
{
	if (pd == NULL) return 0;
	int mem = sizeof(int)*3 + sizeof(void*);
	mem += pd->dnum*pd->DataSize();
	return mem;
}


int MeshMemoryUsed(const Mesh* mesh)
{
	if (mesh == NULL) return 0;

	Mesh* m = (Mesh*)mesh;
	int i, mem = sizeof(int)*2 + m->numVerts*sizeof(Point3) + m->numFaces*sizeof(Face);
	mem += sizeof(int) + sizeof(void*)*2;
	if (m->numTVerts)
		mem += m->numTVerts*sizeof(UVVert) + m->numFaces*sizeof(TVFace);
	mem += sizeof(int) + sizeof(void*)*2;
	if (m->numCVerts) 
		mem += m->numCVerts*sizeof(VertColor) + m->numFaces*sizeof(TVFace);
	mem += sizeof(int) + sizeof(void*)*2;
	if (m->curVCChan)
		mem += m->curVCChan*sizeof(VertColor) + m->numFaces*sizeof(TVFace);
	mem += sizeof(void*)*2;
	mem += sizeof(int) + sizeof(void*);
	for(i=0; i<m->numMaps; i++)
		mem += MeshMapMemoryUsed(&(m->maps[i]));
	
	mem += sizeof(void*) + m->vdSupport.GetSize()/8;
	if (!m->vdSupport.IsEmpty())
		for(i=0; i<m->vdSupport.GetSize(); i++)
			if (m->vdSupport[i])
				mem += PerDataMemoryUsed(&(m->vData[i]));

	mem += sizeof(MtlID);
	mem += m->vertSel.GetSize()/8 + m->faceSel.GetSize()/8 + m->edgeSel.GetSize()/8 + m->vertHide.GetSize()/8;
	mem += sizeof(DWORD)*2;
	mem += sizeof(int);

	return mem;
}

int ParticleChannelMesh::MemoryUsed() const
{
	int mem = sizeof(int)*2 + sizeof(Box3);
	mem += MeshMemoryUsed(globalData());
	mem += sharedTable().Count()*sizeof(int);
	int i;
	for(i=0; i<sharedData().Count(); i++)
		mem += MeshMemoryUsed(sharedData(i));
	for(i=0; i<localData().Count(); i++)
		mem += MeshMemoryUsed(localData(i));
	return mem;
}

int ParticleChannelMesh::Count() const
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

void ParticleChannelMesh::ZeroCount()
{
	// This function can be implemented by calling ParticleChannelMesh::SetCount(0),
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

bool ParticleChannelMesh::SetCount(int n)
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
		if (n != globalCount()) {
			if (n > globalCount() && globalData()) {
				ConvertToShared();			// convert to sharedData type
				return SetCount(n);			// call SetCount on sharedData type
			} else {
				_globalCount() = n;
			}
		}
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
				ConvertToShared();			// convert to sharedData type
				return SetCount(n);			// call SetCount on sharedData type
			}
		}
		return true;
	}

	return false;
}

int ParticleChannelMesh::Delete(int start, int num)
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

int ParticleChannelMesh::Delete(BitArray& toRemove)
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
			if (i < checkCount && toRemove[i] != 0)	continue;
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
			if (i < checkCount && toRemove[i] != 0)	{
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

IObject* ParticleChannelMesh::Split(BitArray& toSplit)
{
	// SysUtil::NeedToImplementLater(); // TODO: optimize the implementation

	ParticleChannelMesh* newChannel = (ParticleChannelMesh*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	if (reverse.GetSize() != newChannel->Count())
		reverse.SetSize(newChannel->Count(), TRUE);
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelMesh::Spawn(Tab<int>& spawnTable)
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

bool ParticleChannelMesh::AppendNum(int num)
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

bool ParticleChannelMesh::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int i, num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelMeshR* iMesh = (IParticleChannelMeshR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iMesh);
	if (iMesh == NULL) return false;

	int oldCount = Count();

	if (dataType() == kGlobalData) ConvertToShared();

	if (iMesh->IsShared()) {
		ConvertToShared();
		// merge two shared tables
		int newSharedValuesNum = iMesh->GetValueCount();
		int oldSharedValuesNum = sharedData().Count();
		_sharedTable().SetCount(oldCount + num);
		_sharedData().SetCount(oldSharedValuesNum + newSharedValuesNum);
		for(i=0; i<newSharedValuesNum; i++)
			_sharedData(i+oldSharedValuesNum) = new Mesh(*(iMesh->GetValueByIndex(i)));
		for(i=0; i<num; i++)
			_sharedTable(i+oldCount) = iMesh->GetValueIndex(i) + oldSharedValuesNum;
	} else {
		if (!AppendNum(num)) return false;
		for(i=0; i<num; i++)
			SetValue(oldCount + i, (Mesh*)(iMesh->GetValue(i)) );
	}

	return true;
}

bool ParticleChannelMesh::IsShared() const
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

int ParticleChannelMesh::GetValueCount() const
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

int ParticleChannelMesh::GetValueIndex(int particleIndex) const
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

const Mesh* ParticleChannelMesh::GetValueByIndex(int valueIndex) const
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

const Mesh*	ParticleChannelMesh::GetValue(int particleIndex) const
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

const Mesh*	ParticleChannelMesh::GetValue() const
{
	return GetValue(0);
}

bool ParticleChannelMesh::SetValue(int particleIndex, Mesh* mesh)
{
	int count = Count();
	DbgAssert(particleIndex >= 0 && particleIndex < count);
	if (particleIndex < 0 || particleIndex >= count)	return false;

	int shrIndex;

	switch (dataType())
	{
	case kGlobalData:
		ConvertToShared();
		return SetValue(particleIndex, mesh);
	case kSharedData:
		if (mesh) {
			// removed the condition [bayboro 06-20-02]
			// otherwise the new value is set for all particles with the same sharing index
		/*	if (shrIndex < 0 || shrIndex >= sharedData().Count()) { 
				shrIndex = sharedData().Count();
				_sharedTable(particleIndex) = shrIndex;
				_sharedData().SetCount(shrIndex+1);
				_sharedData(shrIndex) = NULL;
			} */
			bool hasSharing = ParticleHasSharing(particleIndex);
			shrIndex = sharedTable(particleIndex);
			if (hasSharing || (shrIndex < 0) || (shrIndex >= sharedData().Count())) {
				shrIndex = sharedData().Count();
				_sharedTable(particleIndex) = shrIndex;
				_sharedData().SetCount(shrIndex+1);
				_sharedData(shrIndex) = NULL;
			}
			if (sharedData(shrIndex)) {
				*_sharedData(shrIndex) = *mesh;
			} else {
				_sharedData(shrIndex) = new Mesh(*mesh);
			}
		} else {
			_sharedTable(particleIndex) = -1;
		}
		ShrinkSharedData();
		return true;
	case kLocalData:
		if (mesh) {
			if (localData(particleIndex))
				*_localData(particleIndex) = *mesh;
			else
				_localData(particleIndex) = new Mesh(*mesh);
		} else {
			ConvertToShared();
			return SetValue(particleIndex, mesh);
		}
		return true;
	}

	return false;
}

const Box3&	ParticleChannelMesh::GetMaxBoundingBox() const
{
	return maxBoundingBox();
}

bool ParticleChannelMesh::SetValue(Tab<int>& particleIndices, Mesh* mesh)
{
	if (particleIndices.Count() == 0)	return true;
	
	int firstParticleIndex = particleIndices[0];
	SetValue(firstParticleIndex, mesh);
	for (int i = 1; i < particleIndices.Count(); i++)
		CopyValue(firstParticleIndex, particleIndices[i]);
	return true;
}

bool ParticleChannelMesh::SetValue(Mesh* mesh)
{
	switch (dataType())
	{
	case kGlobalData:
		if (mesh) {
			if (globalData())
				*_globalData() = *mesh;
			else
				_globalData() = new Mesh(*mesh);
		} else {
			if (globalData()) {
				delete _globalData();
				_globalData() = NULL;
			}
		}
		return true;
	case kSharedData:
		ConvertToGlobal();
		return SetValue(mesh);
	case kLocalData:
		ConvertToGlobal();
		return SetValue(mesh);
	}

	return false;
}

bool ParticleChannelMesh::CopyValue(int fromParticle, int toParticle)
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

bool ParticleChannelMesh::CopyValue(int fromParticle, Tab<int>& toParticles)
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

bool ParticleChannelMesh::CopyValue(int fromParticle)
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

bool ParticleChannelMesh::ConvertToLocal()
{
	Mesh* meshData;
	BitArray valueCopied;
	int i, j, valueCount;

	switch (dataType())
	{
	case kGlobalData:
		DbgAssert(localData().Count() == 0);	// if not, might have memory leaks
		_localData().SetCount(globalCount());
		meshData = _globalData();
		for (i = 0; i < globalCount(); i++)
			_localData(i) = (meshData) ? ((i == 0) ? meshData : new Mesh(*meshData)) : NULL;
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
			meshData = (j >= 0 && j < valueCount) ? _sharedData(j) : NULL;
			if (meshData && !valueCopied[j]) {
				_localData(i) = meshData;
				valueCopied.Set(j);
			} else {
				_localData(i) = (meshData) ? new Mesh(*meshData) : NULL;
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

bool ParticleChannelMesh::ConvertToShared()
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

bool ParticleChannelMesh::ConvertToGlobal(int particleIndex /* = 0 */)
{
	int i, index;
	Mesh* meshData;

	switch (dataType())
	{
	case kGlobalData:
		return true;
	case kSharedData:
		DbgAssert(globalData() == NULL);		// if not, might have memory leaks
		if (particleIndex < 0 || particleIndex >= sharedTable().Count()) {
			index = -1;
			meshData = NULL;
		} else {
			index = sharedTable(particleIndex);
			meshData = (index >= 0 && index < sharedData().Count()) ? _sharedData(index) : NULL;
		}
		for (i = 0; i < sharedData().Count(); i++)
			if (sharedData(i) && i != index) {
				delete _sharedData(i);
				_sharedData(i) = NULL;
			}
		_globalData() = meshData;
		_globalCount() = sharedTable().Count();
		_sharedData().ZeroCount();
		_sharedTable().ZeroCount();
		_dataType() = kGlobalData;
		return true;
	case kLocalData:
		DbgAssert(globalData() == NULL);		// if not, might have memory leaks
		index = particleIndex;
		meshData = (index >= 0 && index < localData().Count()) ? _localData(index) : NULL;
		for (i = 0; i < localData().Count(); i++)
			if (localData(i) && i != index) {
				delete _localData(i);
				_localData(i) = NULL;
			}
		_globalData() = meshData;
		_globalCount() = localData().Count();
		_localData().ZeroCount();
		_dataType() = kGlobalData;
		return true;
	}

	return false;
}

bool ParticleChannelMesh::ShrinkSharedData()
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

void ParticleChannelMesh::BuildMaxBoundingBox()
{
	int i;
	_maxBoundingBox().Init();

	switch (dataType())
	{
	case kGlobalData:
		if (globalData() != NULL) {
			_globalData()->buildBoundingBox();
			_maxBoundingBox() = _globalData()->getBoundingBox();
		}
		break;
	case kSharedData:
		for(i=0; i<sharedData().Count(); i++)
			if (sharedData(i) != NULL)	{
				_sharedData(i)->buildBoundingBox();
				_maxBoundingBox() += _sharedData(i)->getBoundingBox();
			}
		break;
	case kLocalData:
		for(i=0; i<localData().Count(); i++)
			if (localData(i) != NULL)	{
				_localData(i)->buildBoundingBox();
				_maxBoundingBox() += _localData(i)->getBoundingBox();
			}
		break;
	}
}

bool ParticleChannelMesh::ParticleHasSharing(int particleIndex)
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
