/**********************************************************************
 *<
	FILE:			PFTestDurationDesc.cpp

	DESCRIPTION:	Duration Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestDurationDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestDuration.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFTestDurationDesc::m_depotIcon = NULL;
HBITMAP PFTestDurationDesc::m_depotMask = NULL;

PFTestDurationDesc::~PFTestDurationDesc()
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

int PFTestDurationDesc::IsPublic()
{
	return 0;
}

void* PFTestDurationDesc::Create(BOOL loading) 
{
	return new PFTestDuration();
}

const TCHAR* PFTestDurationDesc::ClassName() 
{
	return GetString(IDS_TEST_DURATION_CLASS_NAME);
}

SClass_ID PFTestDurationDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestDurationDesc::ClassID()
{
	return PFTestDuration_Class_ID;
}

Class_ID PFTestDurationDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestDurationDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestDurationDesc::InternalName()
{
	return _T("Age_Test");
}

HINSTANCE PFTestDurationDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestDurationDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TESTDURATION_DESCRIPTION);
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DURATION_DEPOTICON));
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