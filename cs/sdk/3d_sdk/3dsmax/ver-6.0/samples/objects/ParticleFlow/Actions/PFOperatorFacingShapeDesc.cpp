/**********************************************************************
 *<
	FILE:			PFOperatorFacingShapeDesc.cpp

	DESCRIPTION:	FacingShape Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorFacingShapeDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorFacingShape.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {


HBITMAP PFOperatorFacingShapeDesc::m_depotIcon = NULL;
HBITMAP PFOperatorFacingShapeDesc::m_depotMask = NULL;

PFOperatorFacingShapeDesc::~PFOperatorFacingShapeDesc()
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

int PFOperatorFacingShapeDesc::IsPublic()
{
	return 0;
}

void* PFOperatorFacingShapeDesc::Create(BOOL loading) 
{
	return new PFOperatorFacingShape();
}

const TCHAR* PFOperatorFacingShapeDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_FACINGSHAPE_CLASS_NAME);
}

SClass_ID PFOperatorFacingShapeDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorFacingShapeDesc::ClassID()
{
	return PFOperatorFacingShape_Class_ID;
}

Class_ID PFOperatorFacingShapeDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorFacingShapeDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorFacingShapeDesc::InternalName()
{
	return _T("Shape_Facing");
}

HINSTANCE PFOperatorFacingShapeDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorFacingShapeDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_FACINGSHAPE_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FACINGSHAPE_DEPOTICON));
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

PFOperatorFacingShapeStateDesc ThePFOperatorFacingShapeStateDesc;

void* PFOperatorFacingShapeStateDesc::Create(BOOL loading) 
{
	return new PFOperatorFacingShapeState();
}

Class_ID PFOperatorFacingShapeStateDesc::ClassID()
{
	return PFOperatorFacingShapeState_Class_ID;
}

const TCHAR* PFOperatorFacingShapeStateDesc::InternalName()
{
	return _T( "PFOperatorFacingShapeState" );
}


} // end of namespace PFActions