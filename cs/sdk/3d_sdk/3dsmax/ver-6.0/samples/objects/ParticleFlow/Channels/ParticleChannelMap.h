/**********************************************************************
 *<
	FILE: ParticleChannelMap.h

	DESCRIPTION: ParticleChannelMap channel interface
				 This generic channel is used to store map data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELMAP_H_
#define _PARTICLECHANNELMAP_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelMap.h"
#include "ParticleChannelTabUVVert.h"
#include "ParticleChannelTabTVFace.h"

namespace PFChannels {

class ParticleChannelMap:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelMapR,
							public IParticleChannelMapW
{
public:

	ParticleChannelMap();
	~ParticleChannelMap();

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
	enum {	kChunkUVVertChannel	= 1150,
			kChunkTVFaceChannel	= 1151
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

	// from IParticleChannelMapR
	 bool	IsUVVertShared() const;
	 bool	IsTVFaceShared() const;
	 int	GetUVVertCount() const;
	 int	GetUVVertIndex(int particleIndex) const;
	 const TabUVVert* GetUVVertByIndex(int valueIndex) const;
	 const TabUVVert* GetUVVert(int particleIndex) const;
	 const TabUVVert* GetUVVert() const;
	 int	GetTVFaceCount() const;
	 int	GetTVFaceIndex(int particleIndex) const;
	 const TabTVFace* GetTVFaceByIndex(int valueIndex) const;
	 const TabTVFace* GetTVFace(int particleIndex) const;
	 const TabTVFace* GetTVFace() const;
	 void Apply(Mesh* mesh, int particleIndex, int mp) const;

	// from IParticleChannelMapW
	 bool SetUVVert(int particleIndex, const UVVert& value);
	 bool SetUVVert(int particleIndex, const TabUVVert* tab);
	 bool SetUVVert(Tab<int>& particleIndices, const UVVert& value);
	 bool SetUVVert(Tab<int>& particleIndices, const TabUVVert* tab);
	 bool SetUVVert(const UVVert& value);
	 bool SetUVVert(const TabUVVert* tab);
	 bool CopyUVVert(int fromParticle, int toParticle);
	 bool CopyUVVert(int fromParticle, Tab<int>& toParticles);
	 bool CopyUVVert(int fromParticle);
	 bool SetTVFace(int particleIndex, const TabTVFace* tab);
	 bool SetTVFace(Tab<int>& particleIndices, const TabTVFace* tab);
	 bool SetTVFace(const TabTVFace* tab);
	 bool CopyTVFace(int fromParticle, int toParticle);
	 bool CopyTVFace(int fromParticle, Tab<int>& toParticles);
	 bool CopyTVFace(int fromParticle);
	 IObject* GetUVVertChannel() const;
	 IObject* GetTVFaceChannel() const;

private:
	 bool InitUVVertChannel(const ParticleChannelTabUVVert* channel);
	 bool InitTVFaceChannel(const ParticleChannelTabTVFace* channel);

	// const access to class members
	const ParticleChannelTabUVVert* chanUVVert() const { return m_chanUVVert; }
	const ParticleChannelTabTVFace* chanTVFace() const { return m_chanTVFace; }

	// access to class members
	ParticleChannelTabUVVert*&	_chanUVVert()	{ return m_chanUVVert; }
	ParticleChannelTabTVFace*&	_chanTVFace()	{ return m_chanTVFace; }

protected:
	// data
	ParticleChannelTabUVVert* m_chanUVVert;
	ParticleChannelTabTVFace* m_chanTVFace;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELMESHMAP_H_
