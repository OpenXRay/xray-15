/**********************************************************************
 *<
	FILE:			ParticleChannelINode.h

	DESCRIPTION:	ParticleChannelINode channel interface
					This generic channel is used to store pointers at INode objects

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-08-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELINODE_H_
#define _PARTICLECHANNELINODE_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelINode.h"

namespace PFChannels {

class ParticleChannelINode:		public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelINodeR,
								public IParticleChannelINodeW
{
public:

	ParticleChannelINode();
	~ParticleChannelINode();

	// from IObject interface
	TCHAR* GetIObjectName();
	int NumInterfaces() const { return 5; }
	BaseInterface* GetInterfaceAt(int index) const;
	BaseInterface* GetInterface(Interface_ID id);
	void AcquireIObject();
	void ReleaseIObject();
	void DeleteIObject();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// from IParticleChannel
	Class_ID GetClassID() const;
	IObject*  Clone() const;
	IOResult Save(ISave* isave) const;
	IOResult Load(ILoad* iload);
	int MemoryUsed() const;

	// from IParticleChannelAmountR
	int Count() const;

	// from IParticleChannelAmountW
	void	ZeroCount();
	bool	SetCount(int n);
	int	Delete(int start,int num);
	int	Delete(BitArray& toRemove);
	IObject*	Split(BitArray& toSplit);
	bool	Spawn( Tab<int>& spawnTable );
	bool	AppendNum(int num);
	bool	Append(IObject* channel);

	// from IParticleChannelINodeR
	INode*	GetValue(int index) const;
	bool	IsGlobal() const;
	INode*	GetValue() const;

	// from IParticleChannelINodeW
	void	SetValue(int index, INode* value);
	void	SetValue(INode* value);

protected:
	void initializeNodes() const;

	// const access to class members
	INode*				data(int index)	const	{ return m_data[index]; }
	const Tab<INode*>&	data()			const	{ return m_data; }
	ULONG				handle(int index) const	{ return m_handles[index]; }
	const Tab<ULONG>&	handles()		const	{ return m_handles; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	INode*				globalValue()	const	{ return m_globalValue; }
	ULONG				globalHandle()	const	{ return m_globalHandle; }
	bool				nodesInitialized() const{ return m_nodesInitialized; }

	// access to class members
	INode*&			_data(int index)			{ return m_data[index]; }
	Tab<INode*>&	_data()						{ return m_data; }
	ULONG&			_handle(int index)			{ return m_handles[index]; }
	Tab<ULONG>&		_handles()					{ return m_handles; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	INode*&			_globalValue()				{ return m_globalValue; }
	ULONG&			_globalHandle()				{ return m_globalHandle; }
	bool&			_nodesInitialized()			{ return m_nodesInitialized; }

private:
	// data
	mutable Tab<INode*>	m_data; // mutable to be able to initialize node values from the handle values
	Tab<ULONG>  m_handles; // node handles are used to store the node information in file
						   // since ptr to node is not a constant data when loading the file

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	mutable INode* m_globalValue; // mutable to be able to initialize node value from the handle value
	ULONG		m_globalHandle;

	// after loading from a file; the first call for a node value initializes the node ptr values
	// from the node handle values
	mutable bool m_nodesInitialized;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELINODE_H_
