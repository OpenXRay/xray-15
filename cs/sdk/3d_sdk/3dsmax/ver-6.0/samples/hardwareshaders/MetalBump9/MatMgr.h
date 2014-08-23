//_____________________________________________________________________________
//
//	File: MatMgr.h
//
//
//_____________________________________________________________________________


#ifndef MATMGR_H
#define MATMGR_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#ifndef	 SINGLETON_H
#include "Singleton.h"
#endif

#ifndef	 UTILITY_H
#include "Utility.h"
#endif

//_____________________________________________________________________________
//
//	Forward declare
//
//_____________________________________________________________________________

class ShaderMat;

//_____________________________________________________________________________
//
//	Defines
//
//_____________________________________________________________________________

#define MAX_MATERIALS	100


//_____________________________________________________________________________
//
//	Types
//
//_____________________________________________________________________________

//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________


//_____________________________________
//
// 	MatMgr 
//
//_____________________________________

class MatMgr : public Singleton<MatMgr>
{
	public:

		MatShaderInfo	 m_MatInfos[MAX_MATERIALS];
		bool			 m_Loaded;
		int				 m_NumMat;
		//
		//	Constructors
		//
		MatMgr();
		//
		//	Destructors
		//
		~MatMgr();
		//
		//	Methods
		//
		void			LoadDefaults(LPDIRECT3DDEVICE9 Device, bool FromResource = false);
		bool			FindVertexFile(int Index);
		bool			FindPixelFile(int Index);
		bool			FindAlphaPixelFile(int Index);
		MatShaderInfo*	GetMatInfo(int ChannelUse);
		
};


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


