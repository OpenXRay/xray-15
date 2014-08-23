/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamicDesc.cpp

	DESCRIPTION:	MaterialDynamic Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorMaterialDynamicDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorMaterialDynamic.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorMaterialDynamicDesc::m_depotIcon = NULL;
HBITMAP PFOperatorMaterialDynamicDesc::m_depotMask = NULL;

PFOperatorMaterialDynamicDesc::~PFOperatorMaterialDynamicDesc()
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

int PFOperatorMaterialDynamicDesc::IsPublic()
{
	return 0;
}

void* PFOperatorMaterialDynamicDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialDynamic();
}

const TCHAR* PFOperatorMaterialDynamicDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_MATERIALDYNAMIC_CLASS_NAME);
}

SClass_ID PFOperatorMaterialDynamicDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorMaterialDynamicDesc::ClassID()
{
	return PFOperatorMaterialDynamic_Class_ID;
}

Class_ID PFOperatorMaterialDynamicDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorMaterialDynamicDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorMaterialDynamicDesc::InternalName()
{
	return _T("Material_Dynamic");
}

HINSTANCE PFOperatorMaterialDynamicDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorMaterialDynamicDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_MATERIALDYNAMIC_DESCRIPTION);
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

PFOperatorMaterialDynamicStateDesc ThePFOperatorMaterialDynamicStateDesc;

void* PFOperatorMaterialDynamicStateDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialDynamicState();
}

Class_ID PFOperatorMaterialDynamicStateDesc::ClassID()
{
	return PFOperatorMaterialDynamicState_Class_ID;
}

const TCHAR* PFOperatorMaterialDynamicStateDesc::InternalName()
{
	return _T( "PFOperatorMaterialDynamicState" );
}


} // end of namespace PFActions