//_____________________________________________________________________________
//
//	File: ShaderMat.h
//	
//
//_____________________________________________________________________________


#ifndef SHADERMAT_H
#define SHADERMAT_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "Max.h"

#ifndef   CHANNELMAP_H
#include "ChannelMap.h"
#endif

#ifndef	  UTILITY_H
#include "Utility.h"
#endif

//_____________________________________________________________________________
//
//	Forward declare
//
//_____________________________________________________________________________

class RenderMesh;

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
//	MatChannelUse
//
//_____________________________________

typedef enum 
{
	MAT_NONE_ON			= 0,	             
	MAT_DIFFUSE_ON		= 1,	             
	MAT_NORMAL_ON		= 2,	
	MAT_SPECULAR_ON		= 4,	
	MAT_DETAIL_ON		= 8,
	MAT_MASK_ON			= 16,				 
	MAT_REFLECTION_ON	= 32,	             
	MAT_BUMP_ON			= 64,	             

}RMatChannelUse;


//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________

//_____________________________________
//
//	Material class
//
//_____________________________________

class ShaderMat
{
	public:

		TSTR				m_Name;					// Material name
		int					m_ChannelUse;			// ChannelUse
		int					m_ChannelLoaded;		// ChannelLoaded	
		D3DCOLORVALUE		m_Ambient;				// Ambient	
		D3DCOLORVALUE		m_Diffuse;				// Diffuse
		D3DCOLORVALUE		m_Specular;				// Specular
		TSTR				m_VertexFile;			// Vertex shader file name
		unsigned long		m_VertexHandle;			// Vertex shader handle
		TSTR				m_PixelFile;			// Pixel shader file name	
		unsigned long		m_PixelHandle;			// Pixel shader handle
		TSTR				m_AlphaPixelFile;		// Pixel shader file name	
		unsigned long		m_AlphaPixelHandle;		// Pixel shader handle
		LightGroup			m_LightGroup;			// Light group
		ChannelMap			m_Shader[CHANNEL_MAX];	// Shader channels	
		float				m_BumpScale;			// Bump scale
		float				m_MixScale;				// Mix scale
		float				m_ReflectScale;			// Reflect scale
		float				m_Alpha;				// Alpha blend		
		bool				m_AlphaOn;				// Alpha On					
		//
		//	Constructors
		//
		ShaderMat();
		ShaderMat(const ShaderMat &Other);
		ShaderMat(TSTR &Name);
		ShaderMat(MatShaderInfo &Info);
		//
		//	Destructors
		//
		virtual			~ShaderMat();
		bool			Evaluate(float Time, LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh);

		void			ResetMaterial();
		void			SetName(TSTR &Name);
		void			Set(MatShaderInfo &Info);
		void			SetStatesFromChannels(int ChannelUse);
		bool			LoadTexture(RMatChannel Channel,LPDIRECT3DDEVICE8 Device);
		void			UnLoadTexture(RMatChannel Channel);
		void			SetChannelName(TSTR &Name, bool On, RMatChannel Channel);
		void			ReLoadAll(LPDIRECT3DDEVICE8 Device);
		bool			AllLoaded(int ChannelUse);
		bool			AllLoadedBut(int ChannelUse, int NotChannelUse);
		int				NumLoaded(int &Index);
		void			SetChannelsMat(LPDIRECT3DDEVICE8 Device);
		void			SetChannelsLight(LPDIRECT3DDEVICE8 Device);
		void			SetChannelsSpecular(LPDIRECT3DDEVICE8 Device);
		void			SetLightGroup();
		void			SetAlpha(float Alpha);
		bool			IsNameSet(RMatChannel Channel);
		void			UpdateMaterial(float Time);
		//
		//	Operators
		//
		ShaderMat&		operator = (const ShaderMat &Other);

private:

		void			Init(bool DeleteMaps);

};


//_____________________________________
//
//	SetName 
//
//_____________________________________

inline void	ShaderMat::SetName(TSTR &Name)
{
	m_Name = Name;
}


//_____________________________________
//
//	IsNameSet 
//
//_____________________________________

inline bool	ShaderMat::IsNameSet(RMatChannel Channel)
{
	if(m_Name.Length() && m_Name != TSTR("None"))
	{
		return(true);
	}

	return(false);
}

//_____________________________________
//
//	SetAlpha 
//
//_____________________________________

inline void ShaderMat::SetAlpha(float Alpha)
{
	m_Alpha		= Alpha;
	m_AlphaOn	= false;

	if(m_Alpha != 1.0f)
	{
		m_AlphaOn = true;
	}
	
}

//_____________________________________
//
//	SetChannelName
//
//_____________________________________

inline void	ShaderMat::SetChannelName(TSTR &Name, bool On, RMatChannel Channel)
{
	m_Shader[Channel].m_Name = Name;
	m_Shader[Channel].m_On	 = On;
	m_Shader[Channel].UnLoadTexture();

}

//_____________________________________
//
//	LoadTexture 
//
//_____________________________________

inline bool ShaderMat::LoadTexture(RMatChannel Channel, LPDIRECT3DDEVICE8 Device)
{
	UnLoadTexture(Channel);
	
	if(m_Shader[Channel].LoadTexture(Device))
	{
		m_ChannelLoaded |= (1 << Channel);	

		return(true);
	}

	return(false);
}

//_____________________________________
//
//	UnLoadTexture 
//
//_____________________________________

inline void ShaderMat::UnLoadTexture(RMatChannel Channel)
{
	m_Shader[Channel].UnLoadTexture();

	m_ChannelLoaded &= ~(1 << Channel);	

}

//_____________________________________
//
//	AllLoaded 
//
//_____________________________________

inline bool ShaderMat::AllLoaded(int ChannelUse)
{
	int i;

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(ChannelUse & (1 << i))
		{
			if(!(m_ChannelLoaded & (1 << i)))
			{	
				return(false);
			}				
		}	
	}

	return(true);

}

//_____________________________________
//
//	AllLoadedBut 
//
//_____________________________________

inline bool ShaderMat::AllLoadedBut(int ChannelUse, int NotChannelUse)
{
	int i;

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(ChannelUse & (1 << i))
		{
			if(!(m_ChannelLoaded & (1 << i)))
			{	
				return(false);
			}				
		}	
	}

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(NotChannelUse & (1 << i))
		{
			if(m_ChannelLoaded & (1 << i))
			{	
				return(false);
			}				
		}	
	}

	return(true);

}

//_____________________________________
//
//	NumLoaded 
//
//_____________________________________

inline int ShaderMat::NumLoaded(int &Index)
{
	int i,Num;
	
	Num = 0;

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(m_ChannelLoaded & (1 << i))
		{	
			Index = i;
			Num++;
		}	
	}

	return(Num);

}

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


