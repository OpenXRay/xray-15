/**********************************************************************
 *<
	FILE: ParticleChannelPoint3.h

	DESCRIPTION: ParticleChannelPoint3 channel interface
				 This generic channel is used to store 3D vector data

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELPOINT3_H_
#define _PARTICLECHANNELPOINT3_H_

#include "IParticleChannel.h"
#include "IParticleChannelAmount.h"
#include "IParticleChannelPosition.h"

namespace PFChannels {

class ParticleChannelPoint3:	public IObject, 
								public IParticleChannel,
								public IParticleChannelAmountR,
								public IParticleChannelAmountW,
								public IParticleChannelPoint3R,
								public IParticleChannelPoint3W
{
public:

	ParticleChannelPoint3();
	~ParticleChannelPoint3();

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

	// from IParticleChannelPoint3R
	const Point3&	GetValue(int index) const;
	bool			IsGlobal() const;
	const Point3&	GetValue() const;
	const Box3&		GetBoundingBox() const;
	float			GetMaxLengthValue() const;

	// from IParticleChannelPoint3W
	void	SetValue(int index, const Point3& v);
	void	SetValue(const Point3& v);
	void	BuildBoundingBox();
	void	UpdateMaxLengthValue();

private:
		// const access to class members
	const Point3&		data(int index)	const	{ return m_data[index]; }
	const Tab<Point3>&	data()			const	{ return m_data; }
	bool				isGlobal()		const	{ return m_isGlobal; }
	int					globalCount()	const	{ return m_globalCount; }
	const Point3&		globalValue()	const	{ return m_globalValue; }
	const Box3&			boundingBox()	const	{ return m_boundingBox; }
	float				maxLength()		const	{ return m_maxLength; }

	// access to class members
	Point3&			_data(int index)			{ return m_data[index]; }
	Tab<Point3>&	_data()						{ return m_data; }
	bool&			_isGlobal()					{ return m_isGlobal; }
	int&			_globalCount()				{ return m_globalCount; }
	Point3&			_globalValue()				{ return m_globalValue; }
	Box3&			_boundingBox()				{ return m_boundingBox; }
	float&			_maxLength()				{ return m_maxLength; }

protected:
	// data
	Tab<Point3> m_data;

	bool		m_isGlobal;
	int			m_globalCount; // for global values
	Point3		m_globalValue;
	Box3		m_boundingBox;
	float		m_maxLength;
};

} // end of namespace PFChannels

#endif // _PARTICLECHANNELPOSITION_H_