/**********************************************************************
 *<
	FILE:			ParticleChannelAngAxis.h

	DESCRIPTION:	ParticleChannelAngAxis channel interface
					This generic channel is used to store angle/axis data

	CREATED BY:		David C. Thompson

	HISTORY:		created 02-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELANGAXIS_H_
#define _PARTICLECHANNELANGAXIS_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelAngAxis.h"

namespace PFChannels {

class ParticleChannelAngAxis:	public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelAngAxisR,
								public IParticleChannelAngAxisW
{
public:

	ParticleChannelAngAxis();
	~ParticleChannelAngAxis();

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
	const AngAxis& GetValue(int index) const;
	bool IsGlobal() const;
	const AngAxis&	GetValue() const;

	// from IParticleChannelQuatW
	void	SetValue(int index, const AngAxis& value);
	void	SetValue(const AngAxis& value);

private:
	// const access to class members
	const AngAxis&		data(int index)	const	{ return m_data[index]; }
	const Tab<AngAxis>&	data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	const AngAxis&		globalValue()	const	{ return m_globalValue; }
	static const AngAxis& defaultValue()		{ return m_defaultValue; }

	// access to class members
	AngAxis&		_data(int index)			{ return m_data[index]; }
	Tab<AngAxis>&	_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	AngAxis&		_globalValue()				{ return m_globalValue; }

protected:
	// data
	Tab<AngAxis> m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	AngAxis		m_globalValue;
	static const AngAxis	m_defaultValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELANGAXIS_H_