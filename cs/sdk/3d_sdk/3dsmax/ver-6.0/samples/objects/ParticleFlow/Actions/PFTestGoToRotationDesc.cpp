/**********************************************************************
 *<
	FILE:			PFTestGoToRotationDesc.cpp

	DESCRIPTION:	GoToRotation Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-14-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestGoToRotationDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestGoToRotation.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {


HBITMAP PFTestGoToRotationDesc::m_depotIcon = NULL;
HBITMAP PFTestGoToRotationDesc::m_depotMask = NULL;

PFTestGoToRotationDesc::~PFTestGoToRotationDesc()
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

int PFTestGoToRotationDesc::IsPublic()
{
	return 0;
}

void* PFTestGoToRotationDesc::Create(BOOL loading) 
{
	return new PFTestGoToRotation();
}

const TCHAR* PFTestGoToRotationDesc::ClassName() 
{
	return GetString(IDS_TEST_GOTOROTATION_CLASS_NAME);
}

SClass_ID PFTestGoToRotationDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestGoToRotationDesc::ClassID()
{
	return PFTestGoToRotation_Class_ID;
}

Class_ID PFTestGoToRotationDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestGoToRotationDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestGoToRotationDesc::InternalName()
{
	return _T("Go_To_Rotation");
}

HINSTANCE PFTestGoToRotationDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestGoToRotationDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TESTGOTOROTATION_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_GOTOROTATION_DEPOTICON));
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