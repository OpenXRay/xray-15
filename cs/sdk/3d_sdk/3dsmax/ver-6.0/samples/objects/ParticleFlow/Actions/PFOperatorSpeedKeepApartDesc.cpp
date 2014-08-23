/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApartDesc.cpp

	DESCRIPTION:	SpeedKeepApart Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSpeedKeepApartDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSpeedKeepApart.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSpeedKeepApartDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSpeedKeepApartDesc::m_depotMask = NULL;

PFOperatorSpeedKeepApartDesc::~PFOperatorSpeedKeepApartDesc()
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

int PFOperatorSpeedKeepApartDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSpeedKeepApartDesc::Create(BOOL loading) 
{
	return new PFOperatorSpeedKeepApart();
}

const TCHAR* PFOperatorSpeedKeepApartDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SPEEDKEEPAPART_CLASS_NAME);
}

SClass_ID PFOperatorSpeedKeepApartDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSpeedKeepApartDesc::ClassID()
{
	return PFOperatorSpeedKeepApart_Class_ID;
}

Class_ID PFOperatorSpeedKeepApartDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSpeedKeepApartDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSpeedKeepApartDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SPEEDKEEPAPART_INTERNAL_NAME);
	return _T("Keep_Apart");
}

HINSTANCE PFOperatorSpeedKeepApartDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSpeedKeepApartDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SPEEDKEEPAPART_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPEEDKEEPAPART_DEPOTICON));
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