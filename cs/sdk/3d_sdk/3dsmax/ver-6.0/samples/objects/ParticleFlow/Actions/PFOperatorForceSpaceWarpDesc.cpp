/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarpDesc.cpp

	DESCRIPTION:	ForceSpaceWarp Operator Class Descriptor (definition)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorForceSpaceWarpDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorForceSpaceWarp.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorForceSpaceWarpDesc::m_depotIcon = NULL;
HBITMAP PFOperatorForceSpaceWarpDesc::m_depotMask = NULL;

PFOperatorForceSpaceWarpDesc::~PFOperatorForceSpaceWarpDesc()
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

int PFOperatorForceSpaceWarpDesc::IsPublic()
{
	return 0;
}

void* PFOperatorForceSpaceWarpDesc::Create(BOOL loading) 
{
	return new PFOperatorForceSpaceWarp();
}

const TCHAR* PFOperatorForceSpaceWarpDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_FORCESPACEWARP_CLASS_NAME);
}

SClass_ID PFOperatorForceSpaceWarpDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorForceSpaceWarpDesc::ClassID()
{
	return PFOperatorForceSpaceWarp_Class_ID;
}

Class_ID PFOperatorForceSpaceWarpDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorForceSpaceWarpDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorForceSpaceWarpDesc::InternalName()
{
	return _T("Force");
}

HINSTANCE PFOperatorForceSpaceWarpDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorForceSpaceWarpDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_FORCESW_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FORCESPACEWARP_DEPOTICON));
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