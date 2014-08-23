/**********************************************************************
 *<
	FILE:			PFOperatorSimplePositionDesc.cpp

	DESCRIPTION:	SimplePosition Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimplePositionDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimplePosition.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSimplePositionDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimplePositionDesc::m_depotMask = NULL;

PFOperatorSimplePositionDesc::~PFOperatorSimplePositionDesc()
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

int PFOperatorSimplePositionDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimplePositionDesc::Create(BOOL loading) 
{
	return new PFOperatorSimplePosition();
}

const TCHAR* PFOperatorSimplePositionDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLEPOSITION_CLASS_NAME);
}

SClass_ID PFOperatorSimplePositionDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimplePositionDesc::ClassID()
{
	return PFOperatorSimplePosition_Class_ID;
}

Class_ID PFOperatorSimplePositionDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimplePositionDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimplePositionDesc::InternalName()
{
	return _T("Position_Icon");
}

HINSTANCE PFOperatorSimplePositionDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimplePositionDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLEPOSITION_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLEPOSITION_DEPOTICON));
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


PFOperatorSimplePositionStateDesc ThePFOperatorSimplePositionStateDesc;

void* PFOperatorSimplePositionStateDesc::Create(BOOL loading) 
{
	return new PFOperatorSimplePositionState();
}

Class_ID PFOperatorSimplePositionStateDesc::ClassID()
{
	return PFOperatorSimplePositionState_Class_ID;
}

const TCHAR* PFOperatorSimplePositionStateDesc::InternalName()
{
	return _T( "PFOperatorSimplePositionState" );
}


} // end of namespace PFActions