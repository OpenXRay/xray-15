/**********************************************************************
 *<
	FILE:			PFOperatorSimpleOrientationDesc.cpp

	DESCRIPTION:	SimpleOrientation Operator Class Descriptor (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleOrientationDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleOrientation.h"
#include "PFClassIDs.h"

namespace PFActions {

HBITMAP PFOperatorSimpleOrientationDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleOrientationDesc::m_depotMask = NULL;

PFOperatorSimpleOrientationDesc::~PFOperatorSimpleOrientationDesc()
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

int PFOperatorSimpleOrientationDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleOrientationDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleOrientation();
}

const TCHAR* PFOperatorSimpleOrientationDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLEORIENTATION_CLASS_NAME);
}

SClass_ID PFOperatorSimpleOrientationDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleOrientationDesc::ClassID()
{
	return PFOperatorSimpleOrientation_Class_ID;
}

Class_ID PFOperatorSimpleOrientationDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleOrientationDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleOrientationDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SIMPLEORIENTATION_INTERNAL_NAME);
	return _T("rotation");
}

HINSTANCE PFOperatorSimpleOrientationDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleOrientationDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLEORIENTATION_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLEORIENTATION_DEPOTICON));
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