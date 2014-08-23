/**********************************************************************
 *<
	FILE:			ParticleChannelBool.h

	DESCRIPTION:	ParticleChannelBool channel interface
					This generic channel is used to store bool data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-20-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELBOOL_H_
#define _PARTICLECHANNELBOOL_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelBool.h"

namespace PFChannels {

class ParticleChannelBool:		public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelBoolR,
								public IParticleChannelBoolW
{
public:

	ParticleChannelBool();
	~ParticleChannelBool();

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

	// from IParticleChannelBoolR
	bool	GetValue(int index) const;
	bool	IsGlobal() const;
	bool	GetValue() const;

	// from IParticleChannelBoolW
	void	SetValue(int index, bool value);
	void	SetValue(bool value);

private:
	// const access to class members
	bool				data(int index)	const	{ return m_data[index] != 0; }
	const BitArray&		data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	bool				globalValue()	const	{ return m_globalValue; }

	// access to class members
	BitArray&		_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	bool&			_globalValue()				{ return m_globalValue; }

protected:
	// data
	BitArray	m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	bool		m_globalValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELBOOL_H_
