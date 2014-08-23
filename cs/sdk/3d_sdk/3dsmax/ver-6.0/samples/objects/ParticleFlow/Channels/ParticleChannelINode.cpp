/**********************************************************************
 *<
	FILE:			ParticleChannelINode.cpp

	DESCRIPTION:	ParticleChannelINode implementation
					This generic channel is used to store pointers at INode objects

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-08-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelINode.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"

namespace PFChannels {

ParticleChannelINode::ParticleChannelINode()
					:IParticleChannel(PARTICLECHANNELINODER_INTERFACE,PARTICLECHANNELINODEW_INTERFACE)
{
	_isGlobal() = false;
	_globalCount() = 0;
	_globalValue() = NULL;
	_globalHandle() = 0;
	_nodesInitialized() = true;
}

ParticleChannelINode::~ParticleChannelINode()
{
	; // implement later
}

TCHAR* ParticleChannelINode::GetIObjectName()
{
	return _T("ParticleChannelINode");
}

BaseInterface* ParticleChannelINode::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelINodeR*)this;
	case 4: return (IParticleChannelINodeW*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelINode::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == GetReadID())
		return (IParticleChannelINodeR*)this;
	if (id == GetWriteID())
		return (IParticleChannelINodeW*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelINode::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelINode::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelINode::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
ParticleChannelINodeDesc TheParticleChannelINodeDesc;

static FPInterfaceDesc pc_INode_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelINodeDesc, FP_MIXIN, 
			
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

static FPInterfaceDesc pc_INode_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelINodeDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_INode_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelINodeDesc, FP_MIXIN,

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

static FPInterfaceDesc pc_INode_INodeR(
			PARTICLECHANNELINODER_INTERFACE,
			_T("INode*Read"), 0,
			&TheParticleChannelINodeDesc, FP_MIXIN,

		IParticleChannelINodeR::kGetValue, _T("getValue"), 0, TYPE_INODE, 0, 1,
			_T("index"), 0, TYPE_INT,
		IParticleChannelINodeR::kIsGlobal, _T("isGlobal"), 0, TYPE_bool, 0, 0,
		IParticleChannelINodeR::kGetValueGlobal, _T("getValueGlobal"), 0, TYPE_INODE, 0, 0,

		end
);

static FPInterfaceDesc pc_INode_INodeW(
			PARTICLECHANNELINODEW_INTERFACE,
			_T("INode*Write"), 0,
			&TheParticleChannelINodeDesc, FP_MIXIN,

		IParticleChannelINodeW::kSetValue, _T("setValue"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_INODE,
		IParticleChannelINodeW::kSetValueGlobal, _T("setValueGlobal"), 0, TYPE_VOID, 0, 1,
			_T("value"), 0, TYPE_INODE,
		
		end
);

FPInterfaceDesc* ParticleChannelINode::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_INode_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_INode_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_INode_AmountW;
	if (id == GetReadID()) return &pc_INode_INodeR;
	if (id == GetWriteID()) return &pc_INode_INodeW;
	return NULL;
}

Class_ID ParticleChannelINode::GetClassID() const
{
	return ParticleChannelINode_Class_ID;
}

IObject*  ParticleChannelINode::Clone() const
{
	ParticleChannelINode* newChannel = (ParticleChannelINode*)CreateInstance(REF_TARGET_CLASS_ID, ParticleChannelINode_Class_ID);
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_globalCount() = globalCount();
	newChannel->_isGlobal() = isGlobal();
	newChannel->_globalValue() = globalValue();
	newChannel->_globalHandle() = globalHandle();
	newChannel->_data() = data();
	newChannel->_handles() = handles();
	newChannel->_nodesInitialized() = nodesInitialized();
	return newChannel;
}

IOResult ParticleChannelINode::Save(ISave* isave) const
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
		if ((res = isave->Write(handles().Addr(0), sizeof(ULONG)*num, &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	isave->BeginChunk(IParticleChannel::kChunkGlobalCount);
	num = globalCount();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkGlobalValue);
	ULONG h = globalHandle();
	if ((res = isave->Write(&h, sizeof(ULONG), &nb)) != IO_OK) return res;
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

IOResult ParticleChannelINode::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int i, num;
	ULONG h;
	Interface_ID id;
	bool isg;

	_nodesInitialized() = false;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) {
				_handles().SetCount(num);
				for(i=0; i<num; i++) _handle(i) = 0;
				_data().SetCount(num);
				for(i=0; i<num; i++) _data(i) = NULL;
			}
			break;
		case IParticleChannel::kChunkData:
			num = handles().Count();
			DbgAssert(num > 0);
			res=iload->Read(_handles().Addr(0), sizeof(ULONG)*num, &nb);
			break;
		case IParticleChannel::kChunkGlobalCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _globalCount() = num;
			break;
		case IParticleChannel::kChunkGlobalValue:
			res=iload->Read(&h, sizeof(ULONG), &nb);
			if (res == IO_OK) {
				_globalHandle() = h;
				_globalValue() = NULL;
			}
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

int ParticleChannelINode::MemoryUsed() const
{
	int mem = sizeof(bool)*2 + sizeof(int) + sizeof(void*) + sizeof(ULONG);
	mem += data().Count()*sizeof(void*);
	mem += handles().Count()*sizeof(ULONG);
	return mem;
}

int ParticleChannelINode::Count() const
{
	if (!nodesInitialized()) initializeNodes();
	if (isGlobal()) return globalCount();
	return data().Count();
}

void ParticleChannelINode::ZeroCount()
{
	if (isGlobal()) _globalCount() = 0;
	else {
		_data().ZeroCount();
		_handles().ZeroCount();
	}
}

bool ParticleChannelINode::SetCount(int n)
{
	if (!nodesInitialized()) initializeNodes();
	if (n < 0) return false;
	if (isGlobal()) _globalCount() = n;
	else {
		int oldCount = data().Count();
		_data().SetCount(n);
		_handles().SetCount(n);
		for(int i=oldCount; i<n; i++) {
			_data(i) = NULL;
			_handle(i) = 0;
		}
	}
	return true;
}

int ParticleChannelINode::Delete(int start, int num)
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
		_data().Delete(start,num);
		return _handles().Delete(start, num);
	}
}

int ParticleChannelINode::Delete(BitArray& toRemove)
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
			if (i != j)	{ _data(j) = data(i); _handle(j) = handle(i); }
			j++;
		}
		if (j < data().Count()) {
			_data().SetCount(j);
			_handles().SetCount(j);
		}
		return data().Count();
	}
}

IObject* ParticleChannelINode::Split(BitArray& toSplit)
{
	ParticleChannelINode* newChannel = (ParticleChannelINode*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	return newChannel;
}

bool ParticleChannelINode::Spawn(Tab<int>& spawnTable)
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
		Tab<INode*> oldData(data());
		Tab<ULONG> oldHandles(handles());
		_data().SetCount(newCount);
		_handles().SetCount(newCount);
		for(i=0, j=0; i<checkCount; i++)
			for(k=0; k<spawnTable[i]; k++) {
				_data(j) = oldData[i];
				_handle(j++) = oldHandles[i];
			}
	}
	return true;
}

bool ParticleChannelINode::AppendNum(int num)
{
	if (num <= 0)	return true;

	if (isGlobal())
	{
		_globalCount() += num;
		return true;
	}

	int oldCount = data().Count();
	int newCount = oldCount + num;

	if ((_data().Resize(newCount) == 0) || (_handles().Resize(newCount) == 0)) return false;
	_data().SetCount(newCount);
	_handles().SetCount(newCount);

	for(int i=oldCount; i<newCount; i++) {
		_data(i) = NULL;
		_handle(i) = 0;
	}

	return true;
}

bool ParticleChannelINode::Append(IObject* channel)
{
	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	DbgAssert(iAmount);
	if (iAmount == NULL) return false;
	int num = iAmount->Count();
	if (num <= 0) return true;

	IParticleChannelINodeR* iINode = (IParticleChannelINodeR*)(channel->GetInterface(GetReadID()));
	DbgAssert(iINode);
	if (iINode == NULL) return false;

	int oldCount = Count();
	if (!AppendNum(num)) return false;

	for(int i=0; i<num; i++)
		SetValue(oldCount + i, iINode->GetValue(i));

	return true;
}

INode* ParticleChannelINode::GetValue(int index) const
{
	if (!nodesInitialized()) initializeNodes();

	if (isGlobal()) return globalValue();
	DbgAssert((index>=0) && (index<data().Count()));
	return data(index);
}

bool ParticleChannelINode::IsGlobal() const
{
	return isGlobal();
}

INode* ParticleChannelINode::GetValue() const
{
	if (!nodesInitialized()) initializeNodes();

	if (isGlobal()) return globalValue();
	if (data().Count() <= 0) return NULL;
	DbgAssert(data().Count()>0);
	return data(0);
}

void ParticleChannelINode::SetValue(int index, INode* value)
{
	if (isGlobal())
	{
		if (value == globalValue()) return; // nothing to change
		_isGlobal() = false;
		_data().SetCount(globalCount());
		_handles().SetCount(globalCount());
		for(int i=0; i<globalCount(); i++) {
			_data(i) = globalValue();
			_handle(i) = globalHandle();
		}
	}
	DbgAssert((index>=0) && (index<data().Count()));
	_data(index) = value;
	_handle(index) = (value != NULL) ? value->GetHandle() : 0;
}

void ParticleChannelINode::SetValue(INode* value)
{
	if (!isGlobal())
	{
		_isGlobal() = true;
		_globalCount() = data().Count();
		_data().Resize(0);
		_handles().Resize(0);
	}
	_globalValue() = value;
	_globalHandle() = (value != NULL) ? value->GetHandle() : 0;
}

void CollectSceneNodes(INode* node, Tab<INode*>& sceneNodes)
{
	if (node == NULL) return;

	sceneNodes.Append(1, &node, 10);
	for(int i=0; i<node->NumberOfChildren(); i++)
		CollectSceneNodes(node->GetChildNode(i), sceneNodes);
}	

void ParticleChannelINode::initializeNodes() const
{
	bool needInit = false;
	if ((globalHandle() != 0) && (globalValue() == NULL)) needInit = true;
	int i, j, numV = min(data().Count(), handles().Count());
	if (!needInit) {
		for(i=0; i<numV; i++) {
			if ((data(i) == NULL) && (handle(i) != 0)) {
				needInit = true;
				break;
			}
		}
	}

	if (needInit) {
		Tab<INode*> nodes;
		CollectSceneNodes(GetCOREInterface()->GetRootNode(), nodes);
		int num = nodes.Count();
		// init global value
		if ((globalHandle() != 0) && (globalValue() == NULL)) {
			for(i=0; i<num; i++) {
				if (nodes[i]->GetHandle() == globalHandle()) {
					m_globalValue = nodes[i];
					break;
				}
			}
		}
		// init node values
		for(j = 0; j<numV; j++) {
			if ((data(j) == NULL) && (handle(j) != 0)) {
				for(i=0; i<num; i++) {
					if (nodes[i]->GetHandle() == handle(j)) {
						m_data[j] = nodes[i];
						break;
					}
				}
			}
		}
	}

	m_nodesInitialized = true;
}

} // end of namespace PFChannels


