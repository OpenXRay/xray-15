/**********************************************************************
 *<
	FILE:			PFTestScaleDesc.cpp

	DESCRIPTION:	Scale Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestScaleDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestScale.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFTestScaleDesc::m_depotIcon = NULL;
HBITMAP PFTestScaleDesc::m_depotMask = NULL;

PFTestScaleDesc::~PFTestScaleDesc()
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

int PFTestScaleDesc::IsPublic()
{
	return 0;
}

void* PFTestScaleDesc::Create(BOOL loading) 
{
	return new PFTestScale();
}

const TCHAR* PFTestScaleDesc::ClassName() 
{
	return GetString(IDS_TEST_SCALE_CLASS_NAME);
}

SClass_ID PFTestScaleDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestScaleDesc::ClassID()
{
	return PFTestScale_Class_ID;
}

Class_ID PFTestScaleDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestScaleDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestScaleDesc::InternalName()
{
	return _T("Scale_Test ");
}

HINSTANCE PFTestScaleDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestScaleDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
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
		*res = GetString(IDS_TESTSCALE_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SCALETEST_DEPOTICON));
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