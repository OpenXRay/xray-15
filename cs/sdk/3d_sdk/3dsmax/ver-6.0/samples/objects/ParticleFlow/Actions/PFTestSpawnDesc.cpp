/**********************************************************************
 *<
	FILE:			PFTestSpawnDesc.cpp

	DESCRIPTION:	Duration Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestSpawnDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestSpawn.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFTestSpawnDesc::m_depotIcon = NULL;
HBITMAP PFTestSpawnDesc::m_depotMask = NULL;

PFTestSpawnDesc::~PFTestSpawnDesc()
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

int PFTestSpawnDesc::IsPublic()
{
	return 0;
}

void* PFTestSpawnDesc::Create(BOOL loading) 
{
	return new PFTestSpawn();
}

const TCHAR* PFTestSpawnDesc::ClassName() 
{
	return GetString(IDS_TEST_SPAWN_CLASS_NAME);
}

SClass_ID PFTestSpawnDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestSpawnDesc::ClassID()
{
	return PFTestSpawn_Class_ID;
}

Class_ID PFTestSpawnDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestSpawnDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestSpawnDesc::InternalName()
{
	return _T("Spawn");
}

HINSTANCE PFTestSpawnDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestSpawnDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TESTSPAWN_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPAWN_DEPOTICON));
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