/**********************************************************************
 *<
	FILE: ParticleChannelTime.h

	DESCRIPTION: ParticleChannelPTV (PreciseTimeValue) channel interface
				 This generic channel is used to store time data
				 For precision reasons the time is kept in two
				 components: TimeValue TickTime and float FractionTime ]-0.5, 0.5]
				 The result time is a sum of these two components.
				 In genenal, Operators only read from the channel.
				 The Birth Operator sets time according to the time a
				 particle is born. To advance particle in time, Integrator-type
				 operator is used.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELPTV_H_
#define _PARTICLECHANNELPTV_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelPTV.h"

namespace PFChannels {

class ParticleChannelPTV:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelPTVR,
							public IParticleChannelPTVW
{
public:

	ParticleChannelPTV();
	~ParticleChannelPTV();

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

	// from IParticleChannelPTVR
	TimeValue			GetTick(int index) const;
	float				GetFraction(int index) const;
	void				GetTickFraction(int index, TimeValue& tick, float& fraction) const;
	const PreciseTimeValue& GetValue(int index) const;
	bool				IsSync() const;
	const PreciseTimeValue& GetValue() const;

	// from IParticleChannelPTVW
	void	SetTick(int index, TimeValue time);
	void	SetFraction(int index, float time);
	void	SetTickFraction(int index, TimeValue tick, float fraction);
	void	SetValue(int index, const PreciseTimeValue& time);
	void	SetValue(const PreciseTimeValue& time);

private:
		// const access to class members
	const PreciseTimeValue&			data(int index)	const	{ return m_data[index]; }
	const Tab<PreciseTimeValue>&	data()	const			{ return m_data; }

	// access to class members
	PreciseTimeValue&		_data(int index)				{ return m_data[index]; }
	Tab<PreciseTimeValue>&	_data()							{ return m_data; }

protected:
	// data
	Tab<PreciseTimeValue> m_data;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELPTV_H_
