//_____________________________________________________________________________
//
//	File: TextureMgr.h
//	
//
//_____________________________________________________________________________


#ifndef TEXTUREMGR_H
#define TEXTUREMGR_H

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


//_____________________________________________________________________________
//
//	Defines
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Types
//
//_____________________________________________________________________________

typedef struct
{
	TSTR					m_Name;
	TextureType				m_Type;
	IDirect3DBaseTexture8  *m_Texture;	
	int						m_Ref;
		
}TCache;

//_____________________________________
//
//	RenderLightList 
//
//_____________________________________

typedef std::vector<TCache> TCacheList;


//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________


//_____________________________________
//
// 	TextureMgr 
//
//_____________________________________

class TextureMgr : public Singleton<TextureMgr>
{
	public:

		TCacheList	m_List;
		
		//
		//	Constructors
		//
		TextureMgr();
		//
		//	Destructors
		//
		~TextureMgr();
		//
		//	Methods
		//
		int						Find(const TSTR &Name, TextureType Type);
		void					Add(const TSTR &Name, TextureType Type, IDirect3DBaseTexture8 *Texture);
		IDirect3DBaseTexture8*	Get(int Index);
		void					Release(const TSTR &Name, TextureType Type);
		void					AddRef(const TSTR &Name, TextureType Type);

};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


