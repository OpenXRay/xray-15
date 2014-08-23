/**********************************************************************
 *<
	FILE:			PFOperatorMarkShapeDesc.cpp

	DESCRIPTION:	MarkShape Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-29-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorMarkShapeDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorMarkShape.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorMarkShapeDesc::m_depotIcon = NULL;
HBITMAP PFOperatorMarkShapeDesc::m_depotMask = NULL;

PFOperatorMarkShapeDesc::~PFOperatorMarkShapeDesc()
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

int PFOperatorMarkShapeDesc::IsPublic()
{
	return 0;
}

void* PFOperatorMarkShapeDesc::Create(BOOL loading) 
{
	return new PFOperatorMarkShape();
}

const TCHAR* PFOperatorMarkShapeDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_MARKSHAPE_CLASS_NAME);
}

SClass_ID PFOperatorMarkShapeDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorMarkShapeDesc::ClassID()
{
	return PFOperatorMarkShape_Class_ID;
}

Class_ID PFOperatorMarkShapeDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorMarkShapeDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorMarkShapeDesc::InternalName()
{
	return _T("Shape_Mark");
}

HINSTANCE PFOperatorMarkShapeDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorMarkShapeDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_MARKSHAPE_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MARKSHAPE_DEPOTICON));
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

PFOperatorMarkShapeStateDesc ThePFOperatorMarkShapeStateDesc;

void* PFOperatorMarkShapeStateDesc::Create(BOOL loading) 
{
	return new PFOperatorMarkShapeState();
}

Class_ID PFOperatorMarkShapeStateDesc::ClassID()
{
	return PFOperatorMarkShapeState_Class_ID;
}

const TCHAR* PFOperatorMarkShapeStateDesc::InternalName()
{
	return _T( "PFOperatorMarkShapeState" );
}


} // end of namespace PFActions