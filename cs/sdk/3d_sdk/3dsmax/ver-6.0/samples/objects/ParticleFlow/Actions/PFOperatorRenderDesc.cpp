/**********************************************************************
 *<
	FILE:			PFOperatorRenderDesc.cpp

	DESCRIPTION:	Render Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorRenderDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorRender.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorRenderDesc::m_depotIcon = NULL;
HBITMAP PFOperatorRenderDesc::m_depotMask = NULL;

PFOperatorRenderDesc::~PFOperatorRenderDesc()
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

int PFOperatorRenderDesc::IsPublic()
{
	return 0;
}

void* PFOperatorRenderDesc::Create(BOOL loading) 
{
	return new PFOperatorRender();
}

const TCHAR* PFOperatorRenderDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_RENDER_CLASS_NAME);
}

SClass_ID PFOperatorRenderDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorRenderDesc::ClassID()
{
	return PFOperatorRender_Class_ID;
}

Class_ID PFOperatorRenderDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorRenderDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorRenderDesc::InternalName()
{
	return _T("RenderParticles");
}

HINSTANCE PFOperatorRenderDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorRenderDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	bool* isNonExecutable;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_RENDER_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_RENDER_PVIEW_NAME);
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
	case kPF_IsNonExecutable:
		if (arg1 == NULL) return 0;
		isNonExecutable = (bool*)arg1;
		*isNonExecutable = true;
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RENDER_DEPOTICON));
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