/**********************************************************************
 *<
	FILE:			PFOperatorCommentsDesc.cpp

	DESCRIPTION:	Comments Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorCommentsDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorComments.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorCommentsDesc::m_depotIcon = NULL;
HBITMAP PFOperatorCommentsDesc::m_depotMask = NULL;

PFOperatorCommentsDesc::~PFOperatorCommentsDesc()
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

int PFOperatorCommentsDesc::IsPublic()
{
	return 0;
}

void* PFOperatorCommentsDesc::Create(BOOL loading) 
{
	return new PFOperatorComments();
}

const TCHAR* PFOperatorCommentsDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_COMMENTS_CLASS_NAME);
}

SClass_ID PFOperatorCommentsDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorCommentsDesc::ClassID()
{
	return PFOperatorComments_Class_ID;
}

Class_ID PFOperatorCommentsDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorCommentsDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorCommentsDesc::InternalName()
{
	return _T("Notes");
}

HINSTANCE PFOperatorCommentsDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorCommentsDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	bool* isNonExecutable;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_COMMENTS_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMMENTS_DEPOTICON));
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