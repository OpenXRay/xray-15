/**********************************************************************
 *<
	FILE:			PFOperatorInstanceShapeDesc.cpp

	DESCRIPTION:	InstanceShape Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorInstanceShapeDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorInstanceShape.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {


HBITMAP PFOperatorInstanceShapeDesc::m_depotIcon = NULL;
HBITMAP PFOperatorInstanceShapeDesc::m_depotMask = NULL;

PFOperatorInstanceShapeDesc::~PFOperatorInstanceShapeDesc()
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

int PFOperatorInstanceShapeDesc::IsPublic()
{
	return 0;
}

void* PFOperatorInstanceShapeDesc::Create(BOOL loading) 
{
	return new PFOperatorInstanceShape();
}

const TCHAR* PFOperatorInstanceShapeDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_INSTANCESHAPE_CLASS_NAME);
}

SClass_ID PFOperatorInstanceShapeDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorInstanceShapeDesc::ClassID()
{
	return PFOperatorInstanceShape_Class_ID;
}

Class_ID PFOperatorInstanceShapeDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorInstanceShapeDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorInstanceShapeDesc::InternalName()
{
	return _T("Shape_Instance");
}

HINSTANCE PFOperatorInstanceShapeDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorInstanceShapeDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_INSTANCESHAPE_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_INSTANCESHAPE_DEPOTICON));
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

PFOperatorInstanceShapeStateDesc ThePFOperatorInstanceShapeStateDesc;

void* PFOperatorInstanceShapeStateDesc::Create(BOOL loading) 
{
	return new PFOperatorInstanceShapeState();
}

Class_ID PFOperatorInstanceShapeStateDesc::ClassID()
{
	return PFOperatorInstanceShapeState_Class_ID;
}

const TCHAR* PFOperatorInstanceShapeStateDesc::InternalName()
{
	return _T( "PFOperatorInstanceShapeState" );
}


} // end of namespace PFActions