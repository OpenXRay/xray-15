/**********************************************************************
 *<
	FILE:			PFOperatorSimpleMappingDesc.cpp

	DESCRIPTION:	SimpleMapping Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleMappingDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleMapping.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSimpleMappingDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleMappingDesc::m_depotMask = NULL;

PFOperatorSimpleMappingDesc::~PFOperatorSimpleMappingDesc()
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

int PFOperatorSimpleMappingDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleMappingDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleMapping();
}

const TCHAR* PFOperatorSimpleMappingDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLEMAPPING_CLASS_NAME);
}

SClass_ID PFOperatorSimpleMappingDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleMappingDesc::ClassID()
{
	return PFOperatorSimpleMapping_Class_ID;
}

Class_ID PFOperatorSimpleMappingDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleMappingDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleMappingDesc::InternalName()
{
	return _T("mapping");
}

HINSTANCE PFOperatorSimpleMappingDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleMappingDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLEMAPPING_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLEMAPPING_DEPOTICON));
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