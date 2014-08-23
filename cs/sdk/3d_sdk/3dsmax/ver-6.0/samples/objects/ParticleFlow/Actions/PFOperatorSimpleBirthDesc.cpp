/**********************************************************************
 *<
	FILE:			PFOperatorSimpleBirthDesc.cpp

	DESCRIPTION:	SimpleBirth Operator Class Descriptor (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSimpleBirthDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSimpleBirth.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSimpleBirthDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSimpleBirthDesc::m_depotMask = NULL;

PFOperatorSimpleBirthDesc::~PFOperatorSimpleBirthDesc()
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

int PFOperatorSimpleBirthDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSimpleBirthDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleBirth();
}

const TCHAR* PFOperatorSimpleBirthDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SIMPLEBIRTH_CLASS_NAME);
}

SClass_ID PFOperatorSimpleBirthDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSimpleBirthDesc::ClassID()
{
	return PFOperatorSimpleBirth_Class_ID;
}

Class_ID PFOperatorSimpleBirthDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSimpleBirthDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSimpleBirthDesc::InternalName()
{
	return _T("Birth");
}

HINSTANCE PFOperatorSimpleBirthDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSimpleBirthDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	bool* isFertile;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SIMPLEBIRTH_DESCRIPTION);
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_OPERATOR_CATEGORY);
		break;
	case kPF_IsFertile:
		if (arg1 == NULL) return 0;
		isFertile = (bool*)arg1;
		*isFertile = true;
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SIMPLEBIRTH_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_BIRTH));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	default:
		return 0;
	}
	return 1;
}

PFOperatorSimpleBirthStateDesc ThePFOperatorSimpleBirthStateDesc;

void* PFOperatorSimpleBirthStateDesc::Create(BOOL loading) 
{
	return new PFOperatorSimpleBirthState();
}

Class_ID PFOperatorSimpleBirthStateDesc::ClassID()
{
	return PFOperatorSimpleBirthState_Class_ID;
}

const TCHAR* PFOperatorSimpleBirthStateDesc::InternalName()
{
	return _T( "PFOperatorSimpleBirthState" );
}


} // end of namespace PFActions