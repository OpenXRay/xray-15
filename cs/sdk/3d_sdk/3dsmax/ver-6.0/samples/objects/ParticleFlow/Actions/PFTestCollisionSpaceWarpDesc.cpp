/**********************************************************************
 *<
	FILE:			PFTestCollisionSpaceWarpDesc.cpp

	DESCRIPTION:	CollisionSpaceWarp Test Class Descriptor (definition)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 2-07-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestCollisionSpaceWarpDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestCollisionSpaceWarp.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {


HBITMAP PFTestCollisionSpaceWarpDesc::m_depotIcon = NULL;
HBITMAP PFTestCollisionSpaceWarpDesc::m_depotMask = NULL;

PFTestCollisionSpaceWarpDesc::~PFTestCollisionSpaceWarpDesc()
{
	if (m_depotIcon != NULL) {
		DeleteObject(m_depotIcon);
		m_depotIcon = NULL;
	}
	if (m_depotMask != NULL) {
		DeleteObject(m_depotMask);
		m_depotMask = NULL;
	}
}

int PFTestCollisionSpaceWarpDesc::IsPublic()
{
	return 0;
}

void* PFTestCollisionSpaceWarpDesc::Create(BOOL loading) 
{
	return new PFTestCollisionSpaceWarp();
}

const TCHAR* PFTestCollisionSpaceWarpDesc::ClassName() 
{
	return GetString(IDS_TEST_COLLISIONSPACEWARP_CLASS_NAME);
}

SClass_ID PFTestCollisionSpaceWarpDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestCollisionSpaceWarpDesc::ClassID()
{
	return PFTestCollisionSpaceWarp_Class_ID;
}

Class_ID PFTestCollisionSpaceWarpDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestCollisionSpaceWarpDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestCollisionSpaceWarpDesc::InternalName()
{
	return _T("Collision");
}

HINSTANCE PFTestCollisionSpaceWarpDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestCollisionSpaceWarpDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TESTCOLLISIONSPACEWARP_DESCRIPTION);
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_TEST_CATEGORY);
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COLLISIONSPACEWARP_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_TEST));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	default:
		return 0;
	}
	return 1;
}


} // end of namespace PFActions