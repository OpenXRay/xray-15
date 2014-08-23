/**********************************************************************
 *<
	FILE:			ParticleChannelDesc.h

	DESCRIPTION:	Class Descriptor for Particle Channels (declaration)
					Class Descriptors for specific Particle Channels (declaration)
					It's not a part of the SDK
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-16-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PARTICLECHANNELDESC_H_
#define  _PARTICLECHANNELDESC_H_

#include "max.h"
#include "iparamb2.h"

namespace PFChannels {

//	ParticleChannel-generic Descriptor declarations
class ParticleChannelDesc: public ClassDesc2 {
public:
	int 			IsPublic() { return FALSE; }
	virtual void*	Create(BOOL loading = FALSE) = 0;
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		SubClassID();
	virtual Class_ID	ClassID() = 0;
	const TCHAR* 	Category();
	virtual const TCHAR* InternalName() = 0;
};

class ParticleChannelNewDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelIDDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelPTVDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelINodeDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelBoolDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelIntDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelFloatDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelPoint3Desc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelQuatDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelMatrix3Desc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelVoidDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelMeshDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelAngAxisDesc : public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR*	InternalName();
};

class ParticleChannelTabUVVertDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelTabTVFaceDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelMapDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

class ParticleChannelMeshMapDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};


} // end of namespace PFChannels

#endif // _PARTICLECHANNELDESC_H_