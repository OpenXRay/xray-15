/**********************************************************************
 *<
	FILE:			PFOperatorDisplayDesc.cpp

	DESCRIPTION:	Display Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorDisplayDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorDisplay.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorDisplayDesc::m_depotIcon = NULL;
HBITMAP PFOperatorDisplayDesc::m_depotMask = NULL;

PFOperatorDisplayDesc::~PFOperatorDisplayDesc()
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

int PFOperatorDisplayDesc::IsPublic()
{
	return 0;
}

void* PFOperatorDisplayDesc::Create(BOOL loading) 
{
	return new PFOperatorDisplay();
}

const TCHAR* PFOperatorDisplayDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_DISPLAY_CLASS_NAME);
}

SClass_ID PFOperatorDisplayDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorDisplayDesc::ClassID()
{
	return PFOperatorDisplay_Class_ID;
}

Class_ID PFOperatorDisplayDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorDisplayDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorDisplayDesc::InternalName()
{
	return _T("DisplayParticles");
}

HINSTANCE PFOperatorDisplayDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorDisplayDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	bool* isNonExecutable;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_DISPLAY_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_DISPLAY_PVIEW_NAME);
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
	case kPF_IsNonExecutable:
		if (arg1 == NULL) return 0;
		isNonExecutable = (bool*)arg1;
		*isNonExecutable = true;
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_DEPOTICON));
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