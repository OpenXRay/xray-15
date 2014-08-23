/**********************************************************************
 *<
	FILE:			PFOperatorMaterialStaticDesc.cpp

	DESCRIPTION:	MaterialStatic Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorMaterialStaticDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorMaterialStatic.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorMaterialStaticDesc::m_depotIcon = NULL;
HBITMAP PFOperatorMaterialStaticDesc::m_depotMask = NULL;

PFOperatorMaterialStaticDesc::~PFOperatorMaterialStaticDesc()
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

int PFOperatorMaterialStaticDesc::IsPublic()
{
	return 0;
}

void* PFOperatorMaterialStaticDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialStatic();
}

const TCHAR* PFOperatorMaterialStaticDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_MATERIALSTATIC_CLASS_NAME);
}

SClass_ID PFOperatorMaterialStaticDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorMaterialStaticDesc::ClassID()
{
	return PFOperatorMaterialStatic_Class_ID;
}

Class_ID PFOperatorMaterialStaticDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorMaterialStaticDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorMaterialStaticDesc::InternalName()
{
	return _T("Material_Static");
}

HINSTANCE PFOperatorMaterialStaticDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorMaterialStaticDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_MATERIALSTATIC_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_DEPOTICON));
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

PFOperatorMaterialStaticStateDesc ThePFOperatorMaterialStaticStateDesc;

void* PFOperatorMaterialStaticStateDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialStaticState();
}

Class_ID PFOperatorMaterialStaticStateDesc::ClassID()
{
	return PFOperatorMaterialStaticState_Class_ID;
}

const TCHAR* PFOperatorMaterialStaticStateDesc::InternalName()
{
	return _T( "PFOperatorMaterialStaticState" );
}


} // end of namespace PFActions