/**********************************************************************
 *<
	FILE: ParticleChannelNew.h

	DESCRIPTION: ParticleChannelNew channel interface
				 the channel is used to mark particles that are just
				 come to the current event (either via birth or a jump from
				 another event

	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-08-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELNEW_H_
#define _PARTICLECHANNELNEW_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelNew.h"

namespace PFChannels {

class ParticleChannelNew:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelNewR,
							public IParticleChannelNewW
{
public:

	ParticleChannelNew();
	~ParticleChannelNew();

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

	// from IParticleChannelNewR
	bool	IsNew(int index) const;
	bool	IsAllNew() const;
	bool	IsAllOld() const;

	// from IParticleChannelNewW
	void	SetNew(int index);
	void	SetOld(int index);
	void	SetAllNew();
	void	SetAllOld();

private:
		// const access to class members
	int					globalCount()	const	{ return m_globalCount; }
	bool				allNew()		const	{ return m_allNew; }
	bool				allOld()		const	{ return m_allOld; }
	bool				data(int index)	const	{ return m_data[index]; }
	const Tab<bool>&	data()	const			{ return m_data; }

	// access to class members
	int&	_globalCount()				{ return m_globalCount; }
	bool&	_allNew()					{ return m_allNew; }
	bool&	_allOld()					{ return m_allOld; }
	bool&	_data(int index)			{ return m_data[index]; }
	Tab<bool>&	_data()					{ return m_data; }

protected:
	int m_globalCount; // to know amount of particle if it's either allNew or allOld
						// since in that case m_data is not used
	bool m_allNew, m_allOld;
	Tab<bool> m_data;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELNEW_H_