/**********************************************************************
 *<
	FILE:			PFOperatorSimpleScaleDesc.cpp

	DESCRIPTION:	SimpleScale Operator Class Descriptor (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 02-22-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleScaleDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleScale.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSimpleScaleDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleScaleDesc::m_depotMask = NULL;

PFOperatorSimpleScaleDesc::~PFOperatorSimpleScaleDesc()
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

int PFOperatorSimpleScaleDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleScaleDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleScale();
}

const TCHAR* PFOperatorSimpleScaleDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLESCALE_CLASS_NAME);
}

SClass_ID PFOperatorSimpleScaleDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleScaleDesc::ClassID()
{
	return PFOperatorSimpleScale_Class_ID;
}

Class_ID PFOperatorSimpleScaleDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleScaleDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleScaleDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SIMPLESCALE_INTERNAL_NAME);
	return _T("ScaleParticles");
}

HINSTANCE PFOperatorSimpleScaleDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleScaleDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	bool* isPublic;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLESCALE_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLESCALE_PVIEW_NAME);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLESCALE_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_OPERATOR));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
		break;
	default:
		return 0;
	}
	return 1;
}



} // end of namespace PFActions