/**********************************************************************
 *<
	FILE:			PFOperatorSimpleShapeDesc.cpp

	DESCRIPTION:	SimpleShape Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleShapeDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleShape.h"
#include "PFClassIDs.h"

namespace PFActions {

HBITMAP PFOperatorSimpleShapeDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleShapeDesc::m_depotMask = NULL;

PFOperatorSimpleShapeDesc::~PFOperatorSimpleShapeDesc()
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

int PFOperatorSimpleShapeDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleShapeDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleShape();
}

const TCHAR* PFOperatorSimpleShapeDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLESHAPE_CLASS_NAME);
}

SClass_ID PFOperatorSimpleShapeDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleShapeDesc::ClassID()
{
	return PFOperatorSimpleShape_Class_ID;
}

Class_ID PFOperatorSimpleShapeDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleShapeDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleShapeDesc::InternalName()
{
	return _T("ShapeStandard");
}

HINSTANCE PFOperatorSimpleShapeDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleShapeDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLESHAPE_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLESHAPE_PVIEW_NAME);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLESHAPE_DEPOTICON));
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