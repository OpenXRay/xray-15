/**********************************************************************
 *<
	FILE:			ParticleChannelVoid.h

	DESCRIPTION:	ParticleChannelVoid channel interface
					This generic channel is used to store void*ing point numbers

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELVOID_H_
#define _PARTICLECHANNELVOID_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelVoid.h"

namespace PFChannels {

class ParticleChannelVoid:		public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelVoidR,
								public IParticleChannelVoidW
{
public:

	ParticleChannelVoid();
	~ParticleChannelVoid();

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
	IObject* Clone() const;
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

	// from IParticleChannelVoidR
	void*	GetValue(int index) const;
	bool	IsGlobal() const;
	void*	GetValue() const;

	// from IParticleChannelVoidW
	void	SetValue(int index, void* value);
	void	SetValue(void* value);

private:
	// const access to class members
	void*				data(int index)	const	{ return m_data[index]; }
	const Tab<void*>&	data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	void*				globalValue()	const	{ return m_globalValue; }

	// access to class members
	void*&			_data(int index)			{ return m_data[index]; }
	Tab<void*>&		_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	void*&			_globalValue()				{ return m_globalValue; }

protected:
	// data
	Tab<void*>	m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	void*		m_globalValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELVOID_H_
