/**********************************************************************
 *<
	FILE:			PFOperatorSpeedSurfaceNormalsDesc.cpp

	DESCRIPTION:	SpeedSurfaceNormals Operator Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "resource.h"

#include "PFOperatorSpeedSurfaceNormalsDesc.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_SysUtil.h"
#include "PFOperatorSpeedSurfaceNormals.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PFActions {

HBITMAP PFOperatorSpeedSurfaceNormalsDesc::m_depotIcon = NULL;
HBITMAP PFOperatorSpeedSurfaceNormalsDesc::m_depotMask = NULL;

PFOperatorSpeedSurfaceNormalsDesc::~PFOperatorSpeedSurfaceNormalsDesc()
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

int PFOperatorSpeedSurfaceNormalsDesc::IsPublic()
{
	return 0;
}

void* PFOperatorSpeedSurfaceNormalsDesc::Create(BOOL loading) 
{
	return new PFOperatorSpeedSurfaceNormals();
}

const TCHAR* PFOperatorSpeedSurfaceNormalsDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_SPEEDSURFACENORMALS_CLASS_NAME);
}

SClass_ID PFOperatorSpeedSurfaceNormalsDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PFOperatorSpeedSurfaceNormalsDesc::ClassID()
{
	return PFOperatorSpeedSurfaceNormals_Class_ID;
}

Class_ID PFOperatorSpeedSurfaceNormalsDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PFOperatorSpeedSurfaceNormalsDesc::Category() 
{
	return GetString(IDS_PARTICLEFLOW_CATEGORY);
}

const TCHAR* PFOperatorSpeedSurfaceNormalsDesc::InternalName()
{
	//return GetString(IDS_OPERATOR_SPEEDSURFACENORMALS_INTERNAL_NAME);
	return _T("Speed_By_Surface");
}

HINSTANCE PFOperatorSpeedSurfaceNormalsDesc::HInstance()
{
	return hInstance;
}

INT_PTR PFOperatorSpeedSurfaceNormalsDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_SPEEDSURFACENORMALS_DESCRIPTION);
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
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPEEDSURFACENORMALS_DEPOTICON));
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