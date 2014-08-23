/**********************************************************************
 *<
	FILE:			PFChannels_SysUtil.cpp

	DESCRIPTION:	Collection of useful functions and globals unified 
					by SysUtil class (implementation).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "PFChannels_SysUtil.h"

namespace PFChannels {


int SysUtil::m_needToImplementLater = 0;

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






} // end of namespase PFChannels