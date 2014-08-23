/**********************************************************************
 *<
	FILE: ParticleChannelTabTVFace.h

	DESCRIPTION: ParticleChannelTabTVFace channel interface
				 This generic channel is used to store Tab<TVFace> data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELTABTVFACE_H_
#define _PARTICLECHANNELTABTVFACE_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelTabTVFace.h"

namespace PFChannels {

class ParticleChannelTabTVFace:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelTabTVFaceR,
							public IParticleChannelTabTVFaceW
{
public:

	ParticleChannelTabTVFace();
	ParticleChannelTabTVFace(const ParticleChannelTabTVFace* channel);
	~ParticleChannelTabTVFace();

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
	// chunk IDs for saving and loading
	enum {	kChunkLocalData		= 1110,
			kChunkSharedData	= 1111,
			kChunkSharedTable	= 1112,
			kChunkGlobalCount	= 1113,
			kChunkGlobalValue	= 1114,
			kChunkDataType		= 1115
	};
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
	bool	Spawn(Tab<int>& spawnTable);
	bool	AppendNum(int num);
	bool	Append(IObject* channel);

	// from IParticleChannelTabTVFaceR
	bool		IsShared() const;
	int			GetValueCount() const;
	int			GetValueIndex(int particleIndex) const;
	const TabTVFace* GetValueByIndex(int valueIndex) const;
	const TabTVFace*	GetValue(int particleIndex) const;
	const TabTVFace*	GetValue() const;

	// from IParticleChannelTabTVFaceW
	bool	SetValue(int particleIndex, const TabTVFace* value);
	bool	SetValue(Tab<int>& particleIndices, const TabTVFace* value);
	bool	SetValue(const TabTVFace* value);
	bool	CopyValue(int fromParticle, int toParticle);
	bool	CopyValue(int fromParticle, Tab<int>& toParticles);
	bool	CopyValue(int fromParticle);

private:
	// data type conversion functions
	bool	ConvertToLocal();
	bool	ConvertToShared();
	bool	ConvertToGlobal(int particleIndex = 0);
	bool	ShrinkSharedData();

	// verifies if a particle shares a value with other particles
	bool	ParticleHasSharing(int particleIndex);

	// const access to class members
	const TabTVFace*		localData(int index)	const	{ return m_localData[index]; }
	const Tab<TabTVFace*>&	localData()				const	{ return m_localData; }
	const TabTVFace*		sharedData(int index)	const	{ return m_sharedData[index]; }
	const Tab<TabTVFace*>&	sharedData()			const	{ return m_sharedData; }
	const TabTVFace*		globalData()			const	{ return m_globalData; }
	int						sharedTable(int index)	const	{ return m_sharedTable[index]; }
	const Tab<int>&			sharedTable()			const	{ return m_sharedTable; }
	int						globalCount()			const	{ return m_globalCount; }
	int						dataType()				const	{ return m_dataType; }

	// access to class members
	TabTVFace*&				_localData(int index)			{ return m_localData[index]; }
	Tab<TabTVFace*>&		_localData()					{ return m_localData; }
	TabTVFace*&				_sharedData(int index)			{ return m_sharedData[index]; }
	Tab<TabTVFace*>&		_sharedData()					{ return m_sharedData; }
	TabTVFace*&				_globalData()					{ return m_globalData; }
	int&					_sharedTable(int index)			{ return m_sharedTable[index]; }
	Tab<int>&				_sharedTable()					{ return m_sharedTable; }
	int&					_globalCount()					{ return m_globalCount; }
	int&					_dataType()						{ return m_dataType; }

protected:
	// data
	Tab<TabTVFace*>	m_localData;
	Tab<TabTVFace*>	m_sharedData;
	TabTVFace*		m_globalData;

	Tab<int>	m_sharedTable;
	int			m_globalCount;

	enum { kLocalData, kSharedData, kGlobalData };
	int			m_dataType;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELTABTVFACE_H_
