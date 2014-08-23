/**********************************************************************
 *<
	FILE:			PFOperatorExitDesc.cpp

	DESCRIPTION:	Exit Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorExitDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorExit.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorExitDesc::m_depotIcon = NULL;
HBITMAP PFOperatorExitDesc::m_depotMask = NULL;

PFOperatorExitDesc::~PFOperatorExitDesc()
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

int PFOperatorExitDesc::IsPublic()
{
	return 0;
}

void* PFOperatorExitDesc::Create(BOOL loading) 
{
	return new PFOperatorExit();
}

const TCHAR* PFOperatorExitDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_EXIT_CLASS_NAME);
}

SClass_ID PFOperatorExitDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorExitDesc::ClassID()
{
	return PFOperatorExit_Class_ID;
}

Class_ID PFOperatorExitDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorExitDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorExitDesc::InternalName()
{
	return _T("DeleteParticles");
}

HINSTANCE PFOperatorExitDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorExitDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
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
		*res = GetString(IDS_OPERATOR_EXIT_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_EXIT_PVIEW_NAME);
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_OPERATOR_CATEGORY);
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EXIT_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_OPERATOR));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	default:
		return 0;
	}
	return 1;
}



} // end of namespace PFActions