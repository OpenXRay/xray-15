/**********************************************************************
 *<
	FILE:			PFActions_SysUtil.h

	DESCRIPTION:	Collection of useful functions and globals unified 
					by SysUtil class (declaration).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFACTIONS_SYSUTIL_H_
#define _PFACTIONS_SYSUTIL_H_

#include "max.h"


namespace PFActions {

	class WhiteMtl: public Material {
	public:
		WhiteMtl();
	};

	class SysUtil {
	public:
		// debug/memory_management utilities
		static void NeedToImplementLater();
		static void UnderConstructionMessage(const char* functionName);
		static Material* GetWhiteMtl() { return &m_whiteMtl; }
		static Material* GetParticleMtl() { return &m_particleMtl; }

	private:
		static int m_needToImplementLater;
		static WhiteMtl m_whiteMtl;
		static Material m_particleMtl;

	};





} // end of namespace EPDElements

#endif // _PFACTIONS_SYSUTIL_H_