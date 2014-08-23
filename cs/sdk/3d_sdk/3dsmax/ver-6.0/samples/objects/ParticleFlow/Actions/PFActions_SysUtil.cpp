/**********************************************************************
 *<
	FILE:			PFActions_SysUtil.cpp

	DESCRIPTION:	Collection of useful functions and globals unified 
					by SysUtil class (implementation).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "PFActions_SysUtil.h"

namespace PFActions {

WhiteMtl::WhiteMtl():Material() 
{
	Kd[0] = 1.0f;
	Kd[1] = 1.0f;
	Kd[2] = 1.0f;
	Ks[0] = 1.0f;
	Ks[1] = 1.0f;
	Ks[2] = 1.0f;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME; // + GW_TWO_SIDED + GW_PICK;
	selfIllum = (float)1.0;
	dblSided = 1;
}

int SysUtil::m_needToImplementLater = 0;
WhiteMtl SysUtil::m_whiteMtl;
Material SysUtil::m_particleMtl;

// debug/progress_management utilities
void SysUtil::NeedToImplementLater()
{
	m_needToImplementLater++;
}

void SysUtil::UnderConstructionMessage(const char* functionName)
{
#ifdef _DEBUG
	MessageBox(GetActiveWindow(),
				functionName,
				"Under Construction...",
				MB_OK);
#endif
}






} // end of namespase PFActions