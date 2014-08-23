/**********************************************************************
 *<
	FILE:			ParticleChannelMatrix3.h

	DESCRIPTION:	ParticleChannelMatrix3 channel interface
					This generic channel is used to store matrix data

	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-03-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELMATRIX3_H_
#define _PARTICLECHANNELMATRIX3_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelMatrix3.h"

namespace PFChannels {

class ParticleChannelMatrix3:	public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelMatrix3R,
								public IParticleChannelMatrix3W
{
public:

	ParticleChannelMatrix3();
	~ParticleChannelMatrix3();

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

	// from IParticleChannelMatrix3R
	const Matrix3&	GetValue(int index) const;
	bool			IsGlobal() const;
	const Matrix3&	GetValue() const;

	// from IParticleChannelMatrix3W
	void	SetValue(int index, const Matrix3& value);
	void	SetValue(const Matrix3& value);

private:
	// const access to class members
	const Matrix3&		data(int index)	const	{ return m_data[index]; }
	const Tab<Matrix3>&	data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	const Matrix3&		globalValue()	const	{ return m_globalValue; }
	static const Matrix3&	defaultValue()		{ return m_defaultValue; }

	// access to class members
	Matrix3&		_data(int index)			{ return m_data[index]; }
	Tab<Matrix3>&	_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	Matrix3&		_globalValue()				{ return m_globalValue; }

protected:
	// data
	Tab<Matrix3>	m_data;
	bool			m_isGlobal;
	int				m_globalCount; // for global values
	Matrix3			m_globalValue;
	static const Matrix3	m_defaultValue;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELMATRIX3_H_
