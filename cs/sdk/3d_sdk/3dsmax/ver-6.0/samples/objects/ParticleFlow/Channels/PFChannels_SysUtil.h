/**********************************************************************
 *<
	FILE:			PFChannels_SysUtil.h

	DESCRIPTION:	Collection of useful functions and globals unified 
					by SysUtil class (declaration).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFCHANNELS_SYSUTIL_H_
#define _PFCHANNELS_SYSUTIL_H_

#include "max.h"


namespace PFChannels {

	class SysUtil {
	public:
		// debug/memory_management utilities
		static void NeedToImplementLater();
		static void UnderConstructionMessage(const char* functionName);

	private:
		static int m_needToImplementLater;

	};





} // end of namespace EPDElements

#endif // _PFCHANNELS_SYSUTIL_H_