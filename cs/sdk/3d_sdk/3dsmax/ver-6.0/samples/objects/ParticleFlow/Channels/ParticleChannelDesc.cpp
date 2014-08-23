/**********************************************************************
 *<
	FILE:			ParticleChannelDesc.cpp

	DESCRIPTION:	ParticleChannel-generic Class Descriptor (definition)
					ParticleChannel-specific Class Descriptors (definition)
					It's not a part of the SDK
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-16-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "ParticleChannelDesc.h"

#include "PFCHANNELS_GlobalVariables.h"
#include "PFChannels_GlobalFunctions.h"
#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"
#include "ParticleChannelNew.h"
#include "ParticleChannelID.h"
#include "ParticleChannelPTV.h"
#include "ParticleChannelBool.h"
#include "ParticleChannelInt.h"
#include "ParticleChannelFloat.h"
#include "ParticleChannelPoint3.h"
#include "ParticleChannelQuat.h"
#include "ParticleChannelMatrix3.h"
#include "ParticleChannelMesh.h"
#include "ParticleChannelAngAxis.h"
#include "ParticleChannelTabUVVert.h"
#include "ParticleChannelTabTVFace.h"
#include "ParticleChannelMap.h"
#include "ParticleChannelMeshMap.h"
#include "ParticleChannelINode.h"
#include "ParticleChannelVoid.h"

namespace PFChannels {


// generic Particle Channel Class Descriptor
const TCHAR* ParticleChannelDesc::ClassName() 
{
	return _T(""); // doesn't appear as a button anywhere
}

SClass_ID ParticleChannelDesc::SuperClassID() 
{
	return REF_TARGET_CLASS_ID;
}

Class_ID ParticleChannelDesc::SubClassID()
{
	return ParticleChannelSubClassID;
}

const TCHAR* ParticleChannelDesc::Category() 
{
	return _T(""); // doesn't appear as a button anywhere
}

//----------------------------------------------------//
// specific Particle Channel Class Descriptors		  //
//----------------------------------------------------//

// ParticleChannelNew Class Descriptor
void* ParticleChannelNewDesc::Create(BOOL loading) 
{
	return new ParticleChannelNew();
}

Class_ID ParticleChannelNewDesc::ClassID()
{
	return ParticleChannelNew_Class_ID;
}

const TCHAR* ParticleChannelNewDesc::InternalName()
{
	return _T( "ParticleChannelNew" );
}

// ParticleChannelID Class Descriptor
void* ParticleChannelIDDesc::Create(BOOL loading) 
{
	return new ParticleChannelID();
}

Class_ID ParticleChannelIDDesc::ClassID()
{
	return ParticleChannelID_Class_ID;
}

const TCHAR* ParticleChannelIDDesc::InternalName()
{
	return _T( "ParticleChannelID" );
}

// ParticleChannelPTV Class Descriptor
void* ParticleChannelPTVDesc::Create(BOOL loading) 
{
	return new ParticleChannelPTV();
}

Class_ID ParticleChannelPTVDesc::ClassID()
{
	return ParticleChannelPTV_Class_ID;
}

const TCHAR* ParticleChannelPTVDesc::InternalName()
{
	return _T( "ParticleChannelPTV" );
}

// ParticleChannelINode Class Descriptor
void* ParticleChannelINodeDesc::Create(BOOL loading) 
{
	return new ParticleChannelINode();
}

Class_ID ParticleChannelINodeDesc::ClassID()
{
	return ParticleChannelINode_Class_ID;
}

const TCHAR* ParticleChannelINodeDesc::InternalName()
{
	return _T( "ParticleChannelINode" );
}

// ParticleChannelBool Class Descriptor
void* ParticleChannelBoolDesc::Create(BOOL loading) 
{
	return new ParticleChannelBool();
}

Class_ID ParticleChannelBoolDesc::ClassID()
{
	return ParticleChannelBool_Class_ID;
}

const TCHAR* ParticleChannelBoolDesc::InternalName()
{
	return _T( "ParticleChannelBool" );
}

// ParticleChannelInt Class Descriptor
void* ParticleChannelIntDesc::Create(BOOL loading) 
{
	return new ParticleChannelInt();
}

Class_ID ParticleChannelIntDesc::ClassID()
{
	return ParticleChannelInt_Class_ID;
}

const TCHAR* ParticleChannelIntDesc::InternalName()
{
	return _T( "ParticleChannelInt" );
}

// ParticleChannelFloat Class Descriptor
void* ParticleChannelFloatDesc::Create(BOOL loading) 
{
	return new ParticleChannelFloat();
}

Class_ID ParticleChannelFloatDesc::ClassID()
{
	return ParticleChannelFloat_Class_ID;
}

const TCHAR* ParticleChannelFloatDesc::InternalName()
{
	return _T( "ParticleChannelFloat" );
}

// ParticleChannelPoint3 Class Descriptor
void* ParticleChannelPoint3Desc::Create(BOOL loading) 
{
	return new ParticleChannelPoint3();
}

Class_ID ParticleChannelPoint3Desc::ClassID()
{
	return ParticleChannelPoint3_Class_ID;
}

const TCHAR* ParticleChannelPoint3Desc::InternalName()
{
	return _T( "ParticleChannelPoint3" );
}

// ParticleChannelQuat Class Descriptor
void* ParticleChannelQuatDesc::Create(BOOL loading) 
{
	return new ParticleChannelQuat();
}

Class_ID ParticleChannelQuatDesc::ClassID()
{
	return ParticleChannelQuat_Class_ID;
}

const TCHAR* ParticleChannelQuatDesc::InternalName()
{
	return _T( "ParticleChannelQuat" );
}

// ParticleChannelMatrix3 Class Descriptor
void* ParticleChannelMatrix3Desc::Create(BOOL loading) 
{
	return new ParticleChannelMatrix3();
}

Class_ID ParticleChannelMatrix3Desc::ClassID()
{
	return ParticleChannelMatrix3_Class_ID;
}

const TCHAR* ParticleChannelMatrix3Desc::InternalName()
{
	return _T( "ParticleChannelMatrix3" );
}

// ParticleChannelVoid Class Descriptor
void* ParticleChannelVoidDesc::Create(BOOL loading) 
{
	return new ParticleChannelVoid();
}

Class_ID ParticleChannelVoidDesc::ClassID()
{
	return ParticleChannelVoid_Class_ID;
}

const TCHAR* ParticleChannelVoidDesc::InternalName()
{
	return _T( "ParticleChannelVoid" );
}

// ParticleChannelMesh Class Descriptor
void* ParticleChannelMeshDesc::Create(BOOL loading) 
{
	return new ParticleChannelMesh();
}

Class_ID ParticleChannelMeshDesc::ClassID()
{
	return ParticleChannelMesh_Class_ID;
}

const TCHAR* ParticleChannelMeshDesc::InternalName()
{
	return _T( "ParticleChannelMesh" );
}

// ParticleChannelAngAxis Class Descriptor
void* ParticleChannelAngAxisDesc::Create(BOOL loading)
{
	return new ParticleChannelAngAxis();
}

Class_ID ParticleChannelAngAxisDesc::ClassID()
{
	return ParticleChannelAngAxis_Class_ID;
}

const TCHAR* ParticleChannelAngAxisDesc::InternalName()
{
	return _T( "ParticleChannelAngAxis" );
}

// ParticleChannelTabUVVert Class Descriptor
void* ParticleChannelTabUVVertDesc::Create(BOOL loading) 
{
	return new ParticleChannelTabUVVert();
}

Class_ID ParticleChannelTabUVVertDesc::ClassID()
{
	return ParticleChannelTabUVVert_Class_ID;
}

const TCHAR* ParticleChannelTabUVVertDesc::InternalName()
{
	return _T( "ParticleChannelTabUVVert" );
}

// ParticleChannelTabTVFace Class Descriptor
void* ParticleChannelTabTVFaceDesc::Create(BOOL loading) 
{
	return new ParticleChannelTabTVFace();
}

Class_ID ParticleChannelTabTVFaceDesc::ClassID()
{
	return ParticleChannelTabTVFace_Class_ID;
}

const TCHAR* ParticleChannelTabTVFaceDesc::InternalName()
{
	return _T( "ParticleChannelTabTVFace" );
}

// ParticleChannelMap Class Descriptor
void* ParticleChannelMapDesc::Create(BOOL loading) 
{
	return new ParticleChannelMap();
}

Class_ID ParticleChannelMapDesc::ClassID()
{
	return ParticleChannelMap_Class_ID;
}

const TCHAR* ParticleChannelMapDesc::InternalName()
{
	return _T( "ParticleChannelMap" );
}

// ParticleChannelMeshMap Class Descriptor
void* ParticleChannelMeshMapDesc::Create(BOOL loading) 
{
	return new ParticleChannelMeshMap();
}

Class_ID ParticleChannelMeshMapDesc::ClassID()
{
	return ParticleChannelMeshMap_Class_ID;
}

const TCHAR* ParticleChannelMeshMapDesc::InternalName()
{
	return _T( "ParticleChannelMeshMap" );
}


} // end of namespace PFChannels