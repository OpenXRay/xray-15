/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObjectDesc.cpp

	DESCRIPTION:	PositionOnObject Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorPositionOnObjectDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorPositionOnObject.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorPositionOnObjectDesc::m_depotIcon = NULL;
HBITMAP PFOperatorPositionOnObjectDesc::m_depotMask = NULL;

PFOperatorPositionOnObjectDesc::~PFOperatorPositionOnObjectDesc()
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

int PFOperatorPositionOnObjectDesc::IsPublic()
{
	return 0;
}

void* PFOperatorPositionOnObjectDesc::Create(BOOL loading) 
{
	return new PFOperatorPositionOnObject();
}

const TCHAR* PFOperatorPositionOnObjectDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_POSITIONONOBJECT_CLASS_NAME);
}

SClass_ID PFOperatorPositionOnObjectDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorPositionOnObjectDesc::ClassID()
{
	return PFOperatorPositionOnObject_Class_ID;
}

Class_ID PFOperatorPositionOnObjectDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorPositionOnObjectDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorPositionOnObjectDesc::InternalName()
{
	return _T("Position_Object");
}

HINSTANCE PFOperatorPositionOnObjectDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorPositionOnObjectDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_POSITIONONOBJECT_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_POSITIONONOBJECT_DEPOTICON));
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


PFOperatorPositionOnObjectStateDesc ThePFOperatorPositionOnObjectStateDesc;

void* PFOperatorPositionOnObjectStateDesc::Create(BOOL loading) 
{
	return new PFOperatorPositionOnObjectState();
}

Class_ID PFOperatorPositionOnObjectStateDesc::ClassID()
{
	return PFOperatorPositionOnObjectState_Class_ID;
}

const TCHAR* PFOperatorPositionOnObjectStateDesc::InternalName()
{
	return _T( "PFOperatorPositionOnObjectState" );
}


} // end of namespace PFActions