/**********************************************************************
 *<
	FILE: ParticleChannelMesh.h

	DESCRIPTION: ParticleChannelMesh channel interface
				 This generic channel is used to store mesh data.

	CREATED BY: Chung-An Lin

	HISTORY: created 12-04-01
			 modified 12-18-01 (methods IsSimilarChannel, AppendNum,
							Append added - Bayboro)

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELMESH_H_
#define _PARTICLECHANNELMESH_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelMesh.h"

namespace PFChannels {

class ParticleChannelMesh:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelMeshR,
							public IParticleChannelMeshW
{
public:

	ParticleChannelMesh();
	~ParticleChannelMesh();

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
	bool	Spawn(Tab<int>& spawnTable);
	bool	AppendNum(int num);
	bool	Append(IObject* channel);

	// from IParticleChannelMeshR
	bool		IsShared() const;
	int			GetValueCount() const;
	int			GetValueIndex(int particleIndex) const;
	const Mesh* GetValueByIndex(int valueIndex) const;
	const Mesh*	GetValue(int particleIndex) const;
	const Mesh*	GetValue() const;
	const Box3&	GetMaxBoundingBox() const;

	// from IParticleChannelMeshW
	bool	SetValue(int particleIndex, Mesh* mesh);
	bool	SetValue(Tab<int>& particleIndices, Mesh* mesh);
	bool	SetValue(Mesh* mesh);
	bool	CopyValue(int fromParticle, int toParticle);
	bool	CopyValue(int fromParticle, Tab<int>& toParticles);
	bool	CopyValue(int fromParticle);
	void	BuildMaxBoundingBox();

private:
	// data type conversion functions
	bool	ConvertToLocal();
	bool	ConvertToShared();
	bool	ConvertToGlobal(int particleIndex = 0);
	bool	ShrinkSharedData();

	// verifies if a particle shares a value with other particles
	bool	ParticleHasSharing(int particleIndex);

	// const access to class members
	const Mesh*			localData(int index)	const	{ return m_localData[index]; }
	const Tab<Mesh*>&	localData()				const	{ return m_localData; }
	const Mesh*			sharedData(int index)	const	{ return m_sharedData[index]; }
	const Tab<Mesh*>&	sharedData()			const	{ return m_sharedData; }
	const Mesh*			globalData()			const	{ return m_globalData; }
	int					sharedTable(int index)	const	{ return m_sharedTable[index]; }
	const Tab<int>&		sharedTable()			const	{ return m_sharedTable; }
	int					globalCount()			const	{ return m_globalCount; }
	int					dataType()				const	{ return m_dataType; }
	const Box3&			maxBoundingBox()		const	{ return m_maxBoundingBox; }

	// access to class members
	Mesh*&				_localData(int index)			{ return m_localData[index]; }
	Tab<Mesh*>&			_localData()					{ return m_localData; }
	Mesh*&				_sharedData(int index)			{ return m_sharedData[index]; }
	Tab<Mesh*>&			_sharedData()					{ return m_sharedData; }
	Mesh*&				_globalData()					{ return m_globalData; }
	int&				_sharedTable(int index)			{ return m_sharedTable[index]; }
	Tab<int>&			_sharedTable()					{ return m_sharedTable; }
	int&				_globalCount()					{ return m_globalCount; }
	int&				_dataType()						{ return m_dataType; }
	Box3&				_maxBoundingBox()				{ return m_maxBoundingBox; }

protected:
	// data
	Tab<Mesh*>	m_localData;
	Tab<Mesh*>	m_sharedData;
	Mesh*		m_globalData;

	Tab<int>	m_sharedTable;
	int			m_globalCount;

	Box3		m_maxBoundingBox;

	enum { kLocalData, kSharedData, kGlobalData };
	int			m_dataType;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELMESH_H_
