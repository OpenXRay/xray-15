/**********************************************************************
 *<
	FILE:			PFOperatorSpeedCopyDesc.cpp

	DESCRIPTION:	SpeedCopy Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSpeedCopyDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSpeedCopy.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSpeedCopyDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSpeedCopyDesc::m_depotMask = NULL;

PFOperatorSpeedCopyDesc::~PFOperatorSpeedCopyDesc()
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

int PFOperatorSpeedCopyDesc::IsPublic()
{
	return 1;
}

void* PFOperatorSpeedCopyDesc::Create(BOOL loading) 
{
	return new PFOperatorSpeedCopy();
}

const TCHAR* PFOperatorSpeedCopyDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SPEEDCOPY_CLASS_NAME);
}

SClass_ID PFOperatorSpeedCopyDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSpeedCopyDesc::ClassID()
{
	return PFOperatorSpeedCopy_Class_ID;
}

Class_ID PFOperatorSpeedCopyDesc::SubClassID()
{
	return PFOperator3DSubClassID;
}

const TCHAR* PFOperatorSpeedCopyDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSpeedCopyDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SPEEDCOPY_INTERNAL_NAME);
	return _T("SpeedByIcon");
}

HINSTANCE PFOperatorSpeedCopyDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSpeedCopyDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SPEEDCOPY_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SPEEDCOPY_PVIEW_NAME);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPEEDCOPY_DEPOTICON));
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