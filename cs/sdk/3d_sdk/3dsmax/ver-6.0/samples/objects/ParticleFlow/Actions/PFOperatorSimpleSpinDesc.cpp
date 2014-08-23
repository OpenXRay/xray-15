/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpinDesc.cpp

	DESCRIPTION:	SimpleSpin Operator Class Descriptor (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 02-01-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleSpinDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleSpin.h"
#include "PFClassIDs.h"

namespace PFActions {

HBITMAP PFOperatorSimpleSpinDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleSpinDesc::m_depotMask = NULL;

PFOperatorSimpleSpinDesc::~PFOperatorSimpleSpinDesc()
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

int PFOperatorSimpleSpinDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleSpinDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleSpin();
}

const TCHAR* PFOperatorSimpleSpinDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLESPIN_CLASS_NAME);
}

SClass_ID PFOperatorSimpleSpinDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleSpinDesc::ClassID()
{
	return PFOperatorSimpleSpin_Class_ID;
}

Class_ID PFOperatorSimpleSpinDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleSpinDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleSpinDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SIMPLESPIN_INTERNAL_NAME);
	return _T("Spin");
}

HINSTANCE PFOperatorSimpleSpinDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleSpinDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLESPIN_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLESPIN_DEPOTICON));
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