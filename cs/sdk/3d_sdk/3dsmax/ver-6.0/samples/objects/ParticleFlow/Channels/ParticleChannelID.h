/**********************************************************************
 *<
	FILE: ParticleChannelID.h

	DESCRIPTION: ParticleChannelID channel interface
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

#ifndef _PARTICLECHANNELID_H_
#define _PARTICLECHANNELID_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelID.h"

namespace PFChannels {

class ParticleChannelID:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelIDR,
							public IParticleChannelIDW
{
public:

	ParticleChannelID();
	~ParticleChannelID();

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

	// from IParticleChannelIDR
	void	GetID(int index, int& particleIndex, int& particleBorn) const;
	int	GetParticleIndex(int index) const;
	int	GetParticleBorn(int index) const;
	const ParticleID& GetID(int index) const;

	// from IParticleChannelIDW
	void	SetID(int index, int particleIndex, int particleBorn);
	void	SetID(int index, const ParticleID& id);

private:
		// const access to class members
	const ParticleID&		data(int index)	const { return m_data[index]; }
	const Tab<ParticleID>&	data()			const { return m_data; }

	// access to class members
	ParticleID&			_data(int index)	{ return m_data[index]; }
	Tab<ParticleID>&	_data()				{ return m_data; }

protected:
	Tab<ParticleID> m_data;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELID_H_