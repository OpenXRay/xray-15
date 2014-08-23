//_____________________________________________________________________________
//
//	File: ChannelMap.h
//	
//
//_____________________________________________________________________________


#ifndef CHANNELMAP_H
#define CHANNELMAP_H

#if _MSC_VER >= 1000
#pragma once
#endif 

//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#ifndef	  MOVIE_H
#include "Movie.h"
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

//_____________________________________
//
//	ShaderStages
//
//_____________________________________

typedef enum
{
	SHADER_STAGE_UNDEFINED	= -1,
	SHADER_STAGE_ZERO		= 0,
	SHADER_STAGE_ONE		= 1,
	SHADER_STAGE_TWO		= 2,
	SHADER_STAGE_THREE		= 3,
	SHADER_STAGE_FOUR		= 4,
	SHADER_STAGE_FIVE		= 5,
	SHADER_STAGE_SIX		= 6,
	SHADER_STAGE_SEVEN		= 7,
	SHADER_STAGE_MAX
	
}ShaderStages;

//_____________________________________
//
//	ShaderOp 
//
//_____________________________________

typedef enum
{
    SHADER_DISABLE						= 1,    
    SHADER_SELECTARG1					= 2,    
    SHADER_SELECTARG2					= 3,
    SHADER_MODULATE						= 4,    
    SHADER_MODULATE2X					= 5,    
    SHADER_MODULATE4X					= 6,    
    SHADER_ADD							= 7,   
    SHADER_ADDSIGNED					= 8,   
    SHADER_ADDSIGNED2X					= 9,   
    SHADER_SUBTRACT						= 10,   
    SHADER_ADDSMOOTH					= 11, 
    SHADER_BLENDDIFFUSEALPHA			= 12, 
    SHADER_BLENDTEXTUREALPHA			= 13, 
    SHADER_BLENDFACTORALPHA				= 14, 
    SHADER_BLENDTEXTUREALPHAPM			= 15, 
    SHADER_BLENDCURRENTALPHA			= 16, 
    SHADER_PREMODULATE					= 17, 
    SHADER_MODULATEALPHA_ADDCOLOR		= 18, 
    SHADER_MODULATECOLOR_ADDALPHA		= 19, 
    SHADER_MODULATEINVALPHA_ADDCOLOR	= 20,
    SHADER_MODULATEINVCOLOR_ADDALPHA	= 21,
    SHADER_BUMPENVMAP					= 22, 
    SHADER_BUMPENVMAPLUMINANCE			= 23, 
    SHADER_DOTPRODUCT3					= 24,
	SHADER_MULTIPLYADD					= 25,
    SHADER_LERP							= 26 

}ShaderOp;


//_____________________________________
//
//	ShaderInput 
//
//_____________________________________

typedef enum
{
	SHADER_SELECTMASK		= 0x0000000f,  
	SHADER_DIFFUSE			= 0x00000000,  
	SHADER_CURRENT			= 0x00000001,  
	SHADER_TEXTURE			= 0x00000002,  
	SHADER_TFACTOR			= 0x00000003,  
	SHADER_SPECULAR			= 0x00000004,  
	SHADER_TEMP				= 0x00000005,  
	SHADER_COMPLEMENT       = 0x00000010,  
	SHADER_ALPHAREPLICATE	= 0x00000020  

}ShaderInput;


//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________


//_____________________________________
//
// 	ChannelMap 
//
//_____________________________________

class ChannelMap
{
	public:
	
		TSTR					m_Name;					// Texture name
		ShaderStages			m_Stage;				// Texture stage used
		RMatChannel				m_Channel;				// Channel
		IDirect3DBaseTexture8  *m_Texture;				// Texture
		bool					m_On;					// Use
		int						m_UVIndex;				// UV Coordinate
		TextureType				m_Type;
		Movie					*m_Movie;
		//
		//	Color operations
		//
		ShaderInput				m_Color1;
		ShaderInput				m_Color2;
		ShaderOp				m_ColorOp;
		//
		//	Alpha operations
		//
		ShaderInput				m_Alpha1;
		ShaderInput				m_Alpha2;
		ShaderOp				m_AlphaOp;
		//
		//	Constructors
		//
		ChannelMap();
		ChannelMap(const ChannelMap &Other);
		//
		//	Destructors
		//
		~ChannelMap();
		//
		//		
		//
		void					SetChannel(RMatChannel Channel);
		bool					IsMovie();
		//
		//	Texture
		//
		bool					LoadTexture(LPDIRECT3DDEVICE8 Device);
		void					UnLoadTexture();
		bool					IsLoaded();
		IDirect3DBaseTexture8*	GetTexture();
		void					SetTexture(LPDIRECT3DDEVICE8 Device, bool AlphaOn = false);
		ChannelMap&				operator = (const ChannelMap &Other);
		unsigned char *			GetBitmapPtr(int &Pitch, int &Height);
		unsigned long			GetBitmapPixel(float X, float Y);
		void					ReleaseBitmapPtr();
	
	private:

		unsigned long			GetMapData(D3DLOCKED_RECT &LockData, 
										   unsigned long i, unsigned long j);
		void					SetMapData(D3DLOCKED_RECT &LockData, unsigned long i, 
										   unsigned long j, unsigned long Value);

		bool					LoadSingleTexture(LPDIRECT3DDEVICE8 Device, bool MipMap = true);
		bool					LoadSpecTexture(LPDIRECT3DDEVICE8 Device);
		bool					LoadCubeTexture(LPDIRECT3DDEVICE8 Device);
		bool					LoadBumpTexture(LPDIRECT3DDEVICE8 Device);
		bool					LoadNormalTexture(LPDIRECT3DDEVICE8 Device);
		bool					ConvertToBias(LPDIRECT3DDEVICE8 Device, LPDIRECT3DTEXTURE8 Source);


};

//_____________________________________
//
// 	IsLoaded 
//
//_____________________________________

inline bool ChannelMap::IsLoaded()
{
	if(m_Texture)
	{
		return(true);
	}

	return(false);

}

//_____________________________________
//
// 	IsMovie 
//
//_____________________________________

inline bool ChannelMap::IsMovie()
{
	if(m_Movie)
	{
		return(true);
	}

	return(false);
}


//_____________________________________
//
// 	GetMapData 
//
//_____________________________________

inline unsigned long ChannelMap::GetMapData(D3DLOCKED_RECT &LockData, 
										    unsigned long i, unsigned long j)
{
	return(*(unsigned long*)(((unsigned char*)LockData.pBits) + (j * LockData.Pitch) + 
		  (i * sizeof(unsigned long))));
}

//_____________________________________
//
// 	SetMapData 
//
//_____________________________________

inline void ChannelMap::SetMapData(D3DLOCKED_RECT &LockData, unsigned long i, 
								   unsigned long j, unsigned long Value)
{
	*(unsigned long*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + 
	 (i * sizeof(unsigned long))) = Value;
}


//_____________________________________________________________________________
//
//	Globals
//
//_____________________________________________________________________________

void GrayScaleAlpha(LPDIRECT3DTEXTURE8 Texture);


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


