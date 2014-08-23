/**********************************************************************
 *<
	FILE:			ParticleChannelInt.h

	DESCRIPTION:	ParticleChannelInt channel interface
					This generic channel is used to store integer data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELINT_H_
#define _PARTICLECHANNELINT_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelInt.h"

namespace PFChannels {

class ParticleChannelInt:		public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelIntR,
								public IParticleChannelIntW
{
public:

	ParticleChannelInt();
	~ParticleChannelInt();

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

	// from IParticleChannelIntR
	int		GetValue(int index) const;
	bool	IsGlobal() const;
	int		GetValue() const;

	// from IParticleChannelIntW
	void	SetValue(int index, int value);
	void	SetValue(int value);

private:
	// const access to class members
	int					data(int index)	const	{ return m_data[index]; }
	const Tab<int>&		data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	int					globalValue()	const	{ return m_globalValue; }

	// access to class members
	int&			_data(int index)			{ return m_data[index]; }
	Tab<int>&		_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	int&			_globalValue()				{ return m_globalValue; }

protected:
	// data
	Tab<int>	m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	int			m_globalValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELINT_H_
