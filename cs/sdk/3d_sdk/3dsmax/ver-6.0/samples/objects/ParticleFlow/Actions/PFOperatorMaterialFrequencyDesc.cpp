/**********************************************************************
 *<
	FILE:			PFOperatorMaterialFrequencyDesc.cpp

	DESCRIPTION:	MaterialFrequency Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorMaterialFrequencyDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorMaterialFrequency.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorMaterialFrequencyDesc::m_depotIcon = NULL;
HBITMAP PFOperatorMaterialFrequencyDesc::m_depotMask = NULL;

PFOperatorMaterialFrequencyDesc::~PFOperatorMaterialFrequencyDesc()
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

int PFOperatorMaterialFrequencyDesc::IsPublic()
{
	return 0;
}

void* PFOperatorMaterialFrequencyDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialFrequency();
}

const TCHAR* PFOperatorMaterialFrequencyDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_MATERIALFREQUENCY_CLASS_NAME);
}

SClass_ID PFOperatorMaterialFrequencyDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorMaterialFrequencyDesc::ClassID()
{
	return PFOperatorMaterialFrequency_Class_ID;
}

Class_ID PFOperatorMaterialFrequencyDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorMaterialFrequencyDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorMaterialFrequencyDesc::InternalName()
{
	return _T("Material_Frequency");
}

HINSTANCE PFOperatorMaterialFrequencyDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorMaterialFrequencyDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_MATERIALFREQUENCY_DESCRIPTION);
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

PFOperatorMaterialFrequencyStateDesc ThePFOperatorMaterialFrequencyStateDesc;

void* PFOperatorMaterialFrequencyStateDesc::Create(BOOL loading) 
{
	return new PFOperatorMaterialFrequencyState();
}

Class_ID PFOperatorMaterialFrequencyStateDesc::ClassID()
{
	return PFOperatorMaterialFrequencyState_Class_ID;
}

const TCHAR* PFOperatorMaterialFrequencyStateDesc::InternalName()
{
	return _T( "PFOperatorMaterialFrequencyState" );
}


} // end of namespace PFActions