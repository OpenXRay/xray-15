/**********************************************************************
 *<
	FILE:			PFTestSpeedGoToTargetDesc.cpp

	DESCRIPTION:	SpeedGoToTarget Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 07-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestSpeedGoToTargetDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestSpeedGoToTarget.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFTestSpeedGoToTargetDesc::m_depotIcon = NULL;
HBITMAP PFTestSpeedGoToTargetDesc::m_depotMask = NULL;

PFTestSpeedGoToTargetDesc::~PFTestSpeedGoToTargetDesc()
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

int PFTestSpeedGoToTargetDesc::IsPublic()
{
	return 1;
}

void* PFTestSpeedGoToTargetDesc::Create(BOOL loading) 
{
	return new PFTestSpeedGoToTarget();
}

const TCHAR* PFTestSpeedGoToTargetDesc::ClassName() 
{
	return GetString(IDS_TEST_SPEEDGOTOTARGET_CLASS_NAME);
}

SClass_ID PFTestSpeedGoToTargetDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestSpeedGoToTargetDesc::ClassID()
{
	return PFTestSpeedGoToTarget_Class_ID;
}

Class_ID PFTestSpeedGoToTargetDesc::SubClassID()
{
	return PFTest3DSubClassID;
}

const TCHAR* PFTestSpeedGoToTargetDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestSpeedGoToTargetDesc::InternalName()
{
	return _T("Find_Target");
}

HINSTANCE PFTestSpeedGoToTargetDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestSpeedGoToTargetDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
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
		*res = GetString(IDS_TEST_SPEEDGOTOTARGET_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TEST_SPEEDGOTOTARGET_PVIEW_NAME);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPEEDGOTOTARGET_DEPOTICON));
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