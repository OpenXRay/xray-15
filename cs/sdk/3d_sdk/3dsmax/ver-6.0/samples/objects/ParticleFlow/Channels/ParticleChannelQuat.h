/**********************************************************************
 *<
	FILE:			ParticleChannelQuat.h

	DESCRIPTION:	ParticleChannelQuat channel interface
					This generic channel is used to store quaternion data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-22-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELQUAT_H_
#define _PARTICLECHANNELQUAT_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelQuat.h"

namespace PFChannels {

class ParticleChannelQuat:		public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelQuatR,
								public IParticleChannelQuatW
{
public:

	ParticleChannelQuat();
	~ParticleChannelQuat();

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

	// from IParticleChannelQuatR
	const Quat&	GetValue(int index) const;
	bool		IsGlobal() const;
	const Quat&	GetValue() const;

	// from IParticleChannelQuatW
	void	SetValue(int index, const Quat& value);
	void	SetValue(const Quat& value);

private:
	// const access to class members
	const Quat&			data(int index)	const	{ return m_data[index]; }
	const Tab<Quat>&	data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	const Quat&			globalValue()	const	{ return m_globalValue; }
	static const Quat&	defaultValue()			{ return m_defaultValue; }

	// access to class members
	Quat&			_data(int index)			{ return m_data[index]; }
	Tab<Quat>&		_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	Quat&			_globalValue()				{ return m_globalValue; }

protected:
	// data
	Tab<Quat>	m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	Quat		m_globalValue;
	static const Quat	m_defaultValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELQUAT_H_
