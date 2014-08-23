//_____________________________________________________________________________
//
//	File: Lighting.h
//	
//
//_____________________________________________________________________________


#ifndef LIGHTING_H
#define LIGHTING_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "Max.h"

#ifndef   __D3DX8_H__
#include <d3dx8.h>
#endif

#ifndef	 UTILITY_H
#include "Utility.h"
#endif


//_____________________________________________________________________________
//
//	Forward declare
//_____________________________________________________________________________

class Mesh;
class RenderMesh;
class ShaderMat;

//_____________________________________________________________________________
//
//	Defines
//_____________________________________________________________________________

//_____________________________________________________________________________
//
//	Types
//_____________________________________________________________________________

typedef enum
{
	NORMALIZE_NONE   =	0,
	NORMALIZE_ZERO   =	1,
	NORMALIZE_ONE    = 	2,
	NORMALIZE_TWO    =	4,
	NORMALIZE_THREE	 =	8,

}NormalizeChannel;

//_____________________________________
//
//	RenderLightType 
//
//_____________________________________

typedef enum
{
	LIGHT_OMNI,
	LIGHT_SPOT,
	LIGHT_DIR,
	LIGHT_AMB

}RenderLightType;

//_____________________________________
//
//	ShaderLightType 
//
//_____________________________________

typedef enum
{
	SHADER_DIR,
	SHADER_DIR_SPEC,
	SHADER_DIR_NORMAL,
	SHADER_DIR_NORMAL_SPEC,

	SHADER_OMNI,
	SHADER_OMNI_SPEC,
	SHADER_OMNI_NORMAL,
	SHADER_OMNI_NORMAL_SPEC,

	SHADER_SPOT,
	SHADER_SPOT_SPEC,
	SHADER_SPOT_NORMAL,
	SHADER_SPOT_NORMAL_SPEC,

	SHADER_SPEC,
	SHADER_AMBIENT,

	SHADER_LIGHT_MAX

}ShaderLightType;


//_____________________________________
//
//	RenderLight 
//
//_____________________________________

typedef struct
{
	RenderLightType	m_Type;
	bool			m_Atten;
	Point3			m_Color;
	Point3			m_Pos;
	Point3			m_Dir;
	float			m_InnerRange;
	float			m_OuterRange;
	float			m_Angle;

}RenderLight;

//_____________________________________
//
//	RenderLightList 
//
//_____________________________________

typedef std::vector<RenderLight> RenderLightList;

//_____________________________________________________________________________
//
//	Class definitions
//_____________________________________________________________________________


//_____________________________________
//
// 	Lighting 
//
//_____________________________________

class Lighting
{
	public:

		TSTR    				m_VertexFile[SHADER_LIGHT_MAX];
		unsigned long			m_VertexRes[SHADER_LIGHT_MAX];
		unsigned long			m_VertexHandle[SHADER_LIGHT_MAX];

		TSTR    				m_PixelFile[SHADER_LIGHT_MAX];
		unsigned long			m_PixelRes[SHADER_LIGHT_MAX];
		unsigned long			m_PixelHandle[SHADER_LIGHT_MAX];

		LPDIRECT3DDEVICE8		m_Device;
		bool					m_InitDone;
		RenderLightList			m_Lights;
		IDirect3DTexture8		*m_Atten;
		IDirect3DTexture8		*m_Spot;
		LPDIRECT3DCUBETEXTURE8	m_CubeNormal;
		bool					m_Ready;
		D3DXMATRIX				m_MatScaleBias;
		bool					m_forceUpdate;

		Tab <INode * > sceneLights;
		//
		//	Constructors
		//
		Lighting();
		//
		//	Destructors
		//
		~Lighting();
		//
		//	Methods
		//
		bool	Init(LPDIRECT3DDEVICE8 Device);
		bool	EvaluateAmbient(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat);
		bool	EvaluateLighting(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat);
		bool	EvaluateSpecular(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat);
		bool	ClearAlpha(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat);

		void	DestroyShaders();
		void	GetLightsFromScene();
		void	AddLight(INode * node);
		void	DeleteLight(INode * node);
		void	UpdateLights();



	private:

		bool	CreateShaders(bool FromResource = false);
		void	SetRenderStates();	
		void	ClearTextures();
		void	SetShader(RenderLightType Type, ShaderMat *Mat);
		void	SetShaderConst(int Index, RenderLight *Light, ShaderMat *Mat);
		void	SetMaterialConst(ShaderMat *Mat);
		void	SetAttenMap();
		void	SetSpotMap();
		bool	LoadAttenTexture(TSTR &Name, LPDIRECT3DDEVICE8 Device);
		bool	LoadSpotTexture(TSTR &Name, LPDIRECT3DDEVICE8 Device);
		bool    CreateNormalizationCubeMap(LPDIRECT3DDEVICE8 Device, unsigned long Size);
		void	SetCubeNormal(int NormlizeChannel);
		void	ComputeSpotMatrix(RenderLight *Light);

};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


