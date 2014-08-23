/**********************************************************************
 *<
	FILE: ParticleChannelMeshMap.h

	DESCRIPTION: ParticleChannelMeshMap channel interface
				 This generic channel is used to store mesh map data.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELMESHMAP_H_
#define _PARTICLECHANNELMESHMAP_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelMeshMap.h"
#include "ParticleChannelMap.h"

namespace PFChannels {


class ParticleChannelMeshMap:	public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelMeshMapR,
								public IParticleChannelMeshMapW
{
public:
	ParticleChannelMeshMap();
	~ParticleChannelMeshMap();

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

	// from IParticleChannelMeshMapR
	int GetNumMaps() const;
	bool MapSupport(int mp) const;
	IParticleChannelMapR* GetMapReadChannel(int mp);

	// from IParticleChannelMeshMapW
	void SetNumMaps(int ct, bool keep=false);
	void SetMapSupport(int mp, bool support);
	IParticleChannelMapW* GetMapChannel(int mp);
	IObject* GetMapChannelObject(int mp) const;

private:
	// const access to class members
	int							count()		const	{ return m_count; }
	const ParticleChannelMap*	map(int i)	const	{ return m_map[i]; }

	// access to class members
	int&						_count()			{ return m_count; }
	ParticleChannelMap*&		_map(int i)			{ return m_map[i]; }

protected:
	int m_count; // number of particles in the channel; should be in sync
				// with amount in each map channel
	// data
	ParticleChannelMap* m_map[MAX_MESHMAPS];
};


} // end of namespace PFChannels

#endif // _PARTICLECHANNELMESHMAP_H_
