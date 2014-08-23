/**********************************************************************
 *<
	FILE:			PFTestSplitByAmountDesc.cpp

	DESCRIPTION:	SplitByAmount Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFTestSplitByAmountDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFTestSplitByAmount.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFTestSplitByAmountDesc::m_depotIcon = NULL;
HBITMAP PFTestSplitByAmountDesc::m_depotMask = NULL;

PFTestSplitByAmountDesc::~PFTestSplitByAmountDesc()
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

int PFTestSplitByAmountDesc::IsPublic()
{
	return 0;
}

void* PFTestSplitByAmountDesc::Create(BOOL loading) 
{
	return new PFTestSplitByAmount();
}

const TCHAR* PFTestSplitByAmountDesc::ClassName() 
{
	return GetString(IDS_TEST_SPLITBYAMOUNT_CLASS_NAME);
}

SClass_ID PFTestSplitByAmountDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFTestSplitByAmountDesc::ClassID()
{
	return PFTestSplitByAmount_Class_ID;
}

Class_ID PFTestSplitByAmountDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PFTestSplitByAmountDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFTestSplitByAmountDesc::InternalName()
{
	return _T("Split_Amount");
}

HINSTANCE PFTestSplitByAmountDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFTestSplitByAmountDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
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
		*res = GetString(IDS_TESTSPLITBYAMOUNT_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLITBYAMOUNT_DEPOTICON));
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

PFTestSplitByAmountStateDesc ThePFTestSplitByAmountStateDesc;

void* PFTestSplitByAmountStateDesc::Create(BOOL loading) 
{
	return new PFTestSplitByAmountState();
}

Class_ID PFTestSplitByAmountStateDesc::ClassID()
{
	return PFTestSplitByAmountState_Class_ID;
}

const TCHAR* PFTestSplitByAmountStateDesc::InternalName()
{
	return _T( "PFTestSplitByAmountState" );
}


} // end of namespace PFActions