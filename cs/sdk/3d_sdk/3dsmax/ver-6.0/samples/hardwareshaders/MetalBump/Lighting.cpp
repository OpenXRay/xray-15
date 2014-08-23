//_____________________________________________________________________________
//
//	File: Lighting.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files
//	
//_____________________________________________________________________________

#include "Lighting.h"
#include "RenderMesh.h"
#include "ShaderConst.h"
#include "ShaderMat.h"
#include "Utility.h"
#include "resource.h"
#include <Dxerr8.h>


extern TCHAR *GetString(int id);

//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________


//_____________________________________
//
//	Default constructor 
//
//_____________________________________

Lighting::Lighting()
{
	int				i;
	float			Bias,X,Y;
	unsigned int	Range;

	for(i=0; i < SHADER_LIGHT_MAX; i++)
	{
		m_VertexHandle[i] = 0;
		m_PixelHandle[i]  = 0;
	}

	m_Atten		 = NULL;
	m_Spot		 = NULL;
	m_CubeNormal = NULL;
	m_InitDone	 = false;
	m_Ready		 = false;

	//
	//	Set a default projection matrix
	//	
	X	  = 0.5f + (0.5f / 512.0f);
	Y	  = 0.5f + (0.5f / 512.0f);
	Range = 0xFFFFFFFF >> (32 - 24);
	Bias  = -0.001f * (float)Range;

	m_MatScaleBias = D3DXMATRIX(0.5f,  0.0f,  0.0f,         0.0f,
							    0.0f, -0.5f,  0.0f,         0.0f,
							    0.0f,  0.0f,  (float)Range, 0.0f,
							    X,     Y,     Bias,         1.0f);

	m_forceUpdate = true;
	sceneLights.SetCount(0);
	//build the initial list of lights
//	GetLightsFromScene();
	
}


//______________________________________
//
//	Default destructor 
//
//______________________________________

Lighting::~Lighting()
{
	DestroyShaders();
	sceneLights.SetCount(0);

	SAFE_RELEASE(m_Atten);
	SAFE_RELEASE(m_Spot);
	SAFE_RELEASE(m_CubeNormal);

}

//______________________________________
//
//	DestroyShaders 
//
//______________________________________

void Lighting::DestroyShaders()
{
	int i;

	SAFE_RELEASE(m_Atten);
	SAFE_RELEASE(m_Spot);
	SAFE_RELEASE(m_CubeNormal);

	for(i=0; i < SHADER_LIGHT_MAX; i++)
	{
		if(m_VertexHandle[i]) 
		{
			m_Device->DeleteVertexShader(m_VertexHandle[i]);
			m_VertexHandle[i] = 0;
		}

		if(m_PixelHandle[i]) 
		{
			m_Device->DeletePixelShader(m_PixelHandle[i]);
			m_PixelHandle[i] = 0;
		}

	}

}

//______________________________________
//
//	Init 
//
//______________________________________

bool Lighting::Init(LPDIRECT3DDEVICE8 Device)
{

	if(!m_InitDone)
	{
		m_Device = Device;

		m_VertexFile[SHADER_DIR]				= "DL1.nvo";
		m_VertexRes[SHADER_DIR]					=  IDR_DL1_NVO;

		m_PixelFile[SHADER_DIR]					= "DL1.pso";
		m_PixelRes[SHADER_DIR]					=  IDR_DL1_PSO;

		m_VertexFile[SHADER_DIR_SPEC]			= "DL2.nvo";
		m_VertexRes[SHADER_DIR_SPEC]			=  IDR_DL2_NVO;

		m_PixelFile[SHADER_DIR_SPEC]			= "DL2.pso";
		m_PixelRes[SHADER_DIR_SPEC]				=  IDR_DL2_PSO;

		m_VertexFile[SHADER_DIR_NORMAL]			= "DL3.nvo";
		m_VertexRes[SHADER_DIR_NORMAL]			=  IDR_DL3_NVO;

		m_PixelFile[SHADER_DIR_NORMAL]			= "DL3.pso";
		m_PixelRes[SHADER_DIR_NORMAL]			=  IDR_DL3_PSO;

		m_VertexFile[SHADER_DIR_NORMAL_SPEC]	= "DL4.nvo";
		m_VertexRes[SHADER_DIR_NORMAL_SPEC]		=  IDR_DL4_NVO;

		m_PixelFile[SHADER_DIR_NORMAL_SPEC]		= "DL4.pso";
		m_PixelRes[SHADER_DIR_NORMAL_SPEC]		=  IDR_DL4_PSO;

		m_VertexFile[SHADER_OMNI]				= "OL1.nvo";
		m_VertexRes[SHADER_OMNI]				=  IDR_OL1_NVO;

		m_PixelFile[SHADER_OMNI]				= "OL1.pso";
		m_PixelRes[SHADER_OMNI]					=  IDR_OL1_PSO;

		m_VertexFile[SHADER_OMNI_SPEC]			= "OL2.nvo";
		m_VertexRes[SHADER_OMNI_SPEC]			=  IDR_OL2_NVO;

		m_PixelFile[SHADER_OMNI_SPEC]			= "OL2.pso";
		m_PixelRes[SHADER_OMNI_SPEC]			=  IDR_OL2_PSO;

		m_VertexFile[SHADER_OMNI_NORMAL]		= "OL3.nvo";
		m_VertexRes[SHADER_OMNI_NORMAL]			=  IDR_OL3_NVO;

		m_PixelFile[SHADER_OMNI_NORMAL]			= "OL3.pso";
		m_PixelRes[SHADER_OMNI_NORMAL]			=  IDR_OL3_PSO;

		m_VertexFile[SHADER_OMNI_NORMAL_SPEC]	= "OL4.nvo";
		m_VertexRes[SHADER_OMNI_NORMAL_SPEC]	=  IDR_OL4_NVO;

		m_PixelFile[SHADER_OMNI_NORMAL_SPEC]	= "OL4.pso";
		m_PixelRes[SHADER_OMNI_NORMAL_SPEC]		=  IDR_OL4_PSO;

		m_VertexFile[SHADER_SPOT]				= "SL1.nvo";
		m_VertexRes[SHADER_SPOT]				=  IDR_SL1_NVO;

		m_PixelFile[SHADER_SPOT]				= "SL1.pso";
		m_PixelRes[SHADER_SPOT]					=  IDR_SL1_PSO;

		m_VertexFile[SHADER_SPOT_SPEC]			= "SL2.nvo";
		m_VertexRes[SHADER_SPOT_SPEC]			=  IDR_SL2_NVO;

		m_PixelFile[SHADER_SPOT_SPEC]			= "SL2.pso";
		m_PixelRes[SHADER_SPOT_SPEC]			=  IDR_SL2_PSO;

		m_VertexFile[SHADER_SPOT_NORMAL]		= "SL3.nvo";
		m_VertexRes[SHADER_SPOT_NORMAL]			=  IDR_SL3_NVO;

		m_PixelFile[SHADER_SPOT_NORMAL]			= "SL3.pso";
		m_PixelRes[SHADER_SPOT_NORMAL]			=  IDR_SL3_PSO;

		m_VertexFile[SHADER_SPOT_NORMAL_SPEC]	= "SL4.nvo";
		m_VertexRes[SHADER_SPOT_NORMAL_SPEC]	=  IDR_SL4_NVO;

		m_PixelFile[SHADER_SPOT_NORMAL_SPEC]	= "SL4.pso";
		m_PixelRes[SHADER_SPOT_NORMAL_SPEC]		=  IDR_SL4_PSO;

		m_VertexFile[SHADER_SPEC]				= "SP.nvo";
		m_VertexRes[SHADER_SPEC]				=  IDR_SP_NVO;

		m_PixelFile[SHADER_SPEC]				= "SP.pso";
		m_PixelRes[SHADER_SPEC]					=  IDR_SP_PSO;

		m_VertexFile[SHADER_AMBIENT]			= "A.nvo";
		m_VertexRes[SHADER_AMBIENT]				=  IDR_A_NVO;

		m_PixelFile[SHADER_AMBIENT]				= "A.pso";
		m_PixelRes[SHADER_AMBIENT]				=  IDR_A_PSO;


		DestroyShaders();

		if(!CreateShaders(true))
		{
			return(false);
		}

		if(!LoadAttenTexture(TSTR("GAM.tga"),Device))
		{
			return(false);
		}

		if(!LoadSpotTexture(TSTR("GSPM.tga"),Device))
		{
			return(false);
		}

		if(!CreateNormalizationCubeMap(Device,256))
		{
			return(false);
		}

		// do it when we get going......Should only be one time
		m_InitDone = true;
		m_Ready    = true;

	}

	return(true);

}

//______________________________________
//
//	CreateShaders
//
//______________________________________

bool Lighting::CreateShaders(bool FromResource)
{
	HRESULT Hr;
	TSTR	Str;
	int		i;
	TCHAR error[256];


	sprintf(error,"%s",GetString(IDS_ERROR));

	DestroyShaders();

	if(FromResource)
	{	
		for(i = 0; i < SHADER_LIGHT_MAX; i++)
		{
			if(FAILED(Hr = CreateShaderResource(m_Device,m_VertexRes[i],gVertexDecl,
												SHADER_VERTEXSHADER,&m_VertexHandle[i])))
			{
				Str = TSTR(GetString(IDS_ERROR_VS)) + m_VertexFile[i];
				MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
						   MB_APPLMODAL);

				return(false);
			}

			if(FAILED(Hr = CreateShaderResource(m_Device,m_PixelRes[i],gVertexDecl,
												SHADER_PIXELSHADER,&m_PixelHandle[i])))
			{
				Str = TSTR(GetString(IDS_ERROR_PS)) + m_PixelFile[i];
				MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
						   MB_APPLMODAL);

				return(false);
			}

		}
	}
	else
	{
		for(i = 0; i < SHADER_LIGHT_MAX; i++)
		{
			if(FAILED(Hr = CreateShader(m_Device,m_VertexFile[i],gVertexDecl,
										SHADER_VERTEXSHADER,&m_VertexHandle[i])))
			{
				Str = TSTR(GetString(IDS_ERROR_VS)) + m_VertexFile[i] + TSTR(GetString(IDS_CHECK_PATHS));
				MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
						   MB_APPLMODAL);

				return(false);
			}

			if(FAILED(Hr = CreateShader(m_Device,m_PixelFile[i],gVertexDecl,
										SHADER_PIXELSHADER,&m_PixelHandle[i])))
			{
				Str = TSTR(GetString(IDS_ERROR_PS)) + m_PixelFile[i] + TSTR(GetString(IDS_CHECK_PATHS));
				MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
						   MB_APPLMODAL);

				return(false);
			}

		}
	}

	return(true);
}

//______________________________________
//
//	GetLightsFromScene 
//
//______________________________________

void Lighting::GetLightsFromScene()
{
	Interface		*Ip;
//	INode			*Node,*Root;
	Matrix3			Mat;
	int				i,Count;
	ObjectState		Os;
//	LightObject		*LightObj;
//	LightState		Ls;
//	RenderLight		Light;	
	TimeValue		Time;
	Interval		valid;
//	Point3			Pos,Target;

	Ip	  = GetCOREInterface();
	Time  = Ip->GetTime();
	INode * Root  = Ip->GetRootNode();
	Count = Root->NumberOfChildren();

	sceneLights.SetCount(0);
	
	for(i=0; i < Count; i++) 
	{
		INode * Node = Root->GetChildNode(i);
		Os   = Node->EvalWorldState(Time);
		
		if(Os.obj && Os.obj->SuperClassID() == LIGHT_CLASS_ID) 
		{
			sceneLights.Append(1,&Node);
/*			LightObj = (LightObject *)Os.obj;
			LightObj->EvalLightState(Time,valid,&Ls);

			Mat				= Node->GetNodeTM(Time);			
			Pos				= Mat.GetTrans();
			Light.m_Angle   = Ls.hotsize;

			if(Light.m_Angle < 2.0f)
			{
				Light.m_Angle = 2.0f;
			}

			Mat = Node->GetNodeTM(Time);
			Light.m_Dir = Mat.GetRow(2);

			Light.m_Type	= (RenderLightType)Ls.type;
			Light.m_Pos		= Pos;
			Light.m_Dir		= Light.m_Dir.Normalize();
			Light.m_Color	= Ls.color * Ls.intens;

			if(Ls.useAtten)
			{
				Light.m_Atten = true;

				if(Ls.attenStart <= 0.0f)
				{
					Light.m_InnerRange	= 1.0f;
				}
				else
				{
					Light.m_InnerRange	= 1.0f / (Ls.attenStart * 2.0f);
				}

				if(Ls.attenEnd <= 0.0f)
				{
					Light.m_OuterRange	= 0.0f;
				}
				else
				{	
					Light.m_OuterRange	= 1.0f / (Ls.attenEnd * 2.0f);
				}	
			}
			else
			{
				Light.m_Atten		= false;
				Light.m_InnerRange	= 0.1f;
				Light.m_OuterRange	= 0.00001f;
			}

			m_Lights.push_back(Light);
*/
		}
	}
	m_forceUpdate = false;

}


//______________________________________
//
//	EvaluateAmbient 
//
//______________________________________

bool Lighting::EvaluateAmbient(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat)
{
	int i;

	if(!Device || !RMesh || !m_Ready)
	{
		return(false);
	}
	

	Device->SetVertexShaderConstant(CV_GLOBAL_AMBIENT,&D3DXVECTOR4(0.0f,
															       0.0f,
														           0.0f,
														           0.0f),1);


	Device->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);

	//
	//	Clear texture stages
	//
	for(i=0; i < MAX_TMUS; i++)
	{
		Device->SetTexture(i,NULL);
		Device->SetTextureStageState(i,D3DTSS_TEXCOORDINDEX,i);
		Device->SetTextureStageState(i,D3DTSS_COLOROP,D3DTOP_DISABLE);
		Device->SetTextureStageState(i,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		Device->SetTextureStageState(i,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
		Device->SetTextureStageState(i,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
		Device->SetTextureStageState(i,D3DTSS_ADDRESSW,D3DTADDRESS_WRAP);
		Device->SetRenderState((D3DRENDERSTATETYPE)(D3DRS_WRAP0 + i),D3DWRAPCOORD_3);
	}

	Device->SetVertexShader(m_VertexHandle[SHADER_AMBIENT]);

	if(Mat->m_AlphaOn)
	{
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

		Mat->m_Shader[CHANNEL_DIFFUSE].SetTexture(Device);

		Device->SetVertexShaderConstant(CV_MAT_AMBIENT,&D3DXVECTOR4(Mat->m_Ambient.r,
																	Mat->m_Ambient.g,
																	Mat->m_Ambient.b,
																	1.0f),1);


		Device->SetPixelShader(m_PixelHandle[SHADER_AMBIENT]);

	}
	else
	{
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO);

		Device->SetVertexShaderConstant(CV_MAT_AMBIENT,&D3DXVECTOR4(Mat->m_Ambient.r,
																	Mat->m_Ambient.g,
																	Mat->m_Ambient.b,
																	0.0f),1);

		Device->SetPixelShader(0);

	}



	RMesh->Render(Device);


	return(true);

}


//______________________________________
//
//	EvaluateLighting 
//
//______________________________________

bool Lighting::EvaluateLighting(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat)
{
	D3DLIGHT8		DLight;
	D3DXVECTOR3		Pos,Dir;
	RenderLight		Light;
	int				i;

	if(!Device || !RMesh || !m_Ready)
	{
		return(false);
	}

	m_Device = Device;
	//
	//	Setup
	//
//	The force update is used when the scene is in an unknwon state - usual after a an undo of a light
	if(m_forceUpdate)
		GetLightsFromScene();

	UpdateLights();
	SetRenderStates();

//	m_Lights.clear();

	if(m_Lights.size())
	{
		for(i=0; i < m_Lights.size(); i++)
		{
			SetShader(m_Lights[i].m_Type,Mat);
			SetMaterialConst(Mat);
			SetShaderConst(i,&m_Lights[i],Mat);
			RMesh->Render(m_Device);
		}
	}
	else
	{
		m_Device->GetLight(0,&DLight);

		Light.m_Dir = Point3(-DLight.Direction.x,
							 -DLight.Direction.y,
							 -DLight.Direction.z);
				
		Light.m_Color.x	= DLight.Diffuse.r;
		Light.m_Color.y	= DLight.Diffuse.g;
		Light.m_Color.z	= DLight.Diffuse.b;

		Light.m_Pos.x	= DLight.Position.x;
		Light.m_Pos.y	= DLight.Position.y;
		Light.m_Pos.z	= DLight.Position.z;

		Light.m_InnerRange	= 1.0f;
		Light.m_OuterRange	= 1.0f / (DLight.Range * 2.0f);

		SetShader(LIGHT_DIR,Mat);
		SetMaterialConst(Mat);
		SetShaderConst(0,&Light,Mat);
		RMesh->Render(m_Device);
	}


	return(true);

}


//______________________________________
//
//	EvaluateSpecular
//
//______________________________________

bool Lighting::EvaluateSpecular(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat)
{

	if(!Device || !RMesh || !m_Ready)
	{
		return(false);
	}

	if(!Mat->m_AlphaOn && Mat->m_ChannelLoaded & MAT_SPECULAR_ON)
	{
		SetRenderStates();

		Device->SetVertexShader(m_VertexHandle[SHADER_SPEC]);
		Device->SetPixelShader(m_PixelHandle[SHADER_SPEC]);

		SetMaterialConst(Mat);
		
		Mat->SetChannelsSpecular(Device);

		Device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);

		RMesh->Render(m_Device);

	}

	return(true);
}


//______________________________________
//
//	ClearAlpha
//
//______________________________________

bool Lighting::ClearAlpha(LPDIRECT3DDEVICE8 Device, RenderMesh *RMesh, ShaderMat *Mat)
{
	int i;

	if(!Device || !RMesh || !m_Ready)
	{
		return(false);
	}

	//
	//	Clear texture stages
	//
	for(i=0; i < MAX_TMUS; i++)
	{
		Device->SetTexture(i,NULL);
		Device->SetTextureStageState(i,D3DTSS_TEXCOORDINDEX,i);
		Device->SetTextureStageState(i,D3DTSS_COLOROP,D3DTOP_DISABLE);
		Device->SetTextureStageState(i,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
	}

	Device->SetVertexShader(m_VertexHandle[SHADER_AMBIENT]);
	Device->SetPixelShader(0);

	Device->SetVertexShaderConstant(CV_MAT_AMBIENT,&D3DXVECTOR4(0.0f,
																0.0f,
																0.0f,
																0.0f),1);

	Device->SetVertexShaderConstant(CV_GLOBAL_AMBIENT,&D3DXVECTOR4(0.0f,
															       0.0f,
														           0.0f,
														           0.0f),1);


	Device->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);
	Device->SetRenderState(D3DRS_DITHERENABLE,FALSE);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO);
	Device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	//
	//	Clear the alpha values, for alpha blending passes
	//
	m_Device->SetRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA);

	RMesh->Render(m_Device);

	m_Device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | 
							 D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	return(true);

	
}



//______________________________________
//
//	SetShader 
//
//______________________________________

void Lighting::SetShader(RenderLightType Type, ShaderMat *Mat)
{
	
	switch(Mat->m_LightGroup)
	{

		default:
						if(Type == LIGHT_OMNI)
						{	
							m_Device->SetVertexShader(m_VertexHandle[SHADER_OMNI]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_OMNI]);

							ClearTextures();
							SetAttenMap();
							SetCubeNormal(NORMALIZE_ZERO | NORMALIZE_ONE);

						}
						else if(Type == LIGHT_SPOT)
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_SPOT]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_SPOT]);

							ClearTextures();
							SetSpotMap();
							SetCubeNormal(NORMALIZE_ZERO | NORMALIZE_ONE);
						}
						else
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_DIR]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_DIR]);

							ClearTextures();
							SetCubeNormal(NORMALIZE_ZERO | NORMALIZE_ONE);
						}

						break;


		case LG_NORMALSPEC:

						if(Type == LIGHT_OMNI) 
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_OMNI_NORMAL_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_OMNI_NORMAL_SPEC]);

							ClearTextures();
							SetAttenMap();
							Mat->SetChannelsLight(m_Device);
						}
						else if(Type == LIGHT_SPOT)
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_SPOT_NORMAL_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_SPOT_NORMAL_SPEC]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetSpotMap();
						}
						else
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_DIR_NORMAL_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_DIR_NORMAL_SPEC]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
						}
						break;
		
		case LG_NORMAL:
						if(Type == LIGHT_OMNI) 
						{	
							m_Device->SetVertexShader(m_VertexHandle[SHADER_OMNI_NORMAL]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_OMNI_NORMAL]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetAttenMap();
							SetCubeNormal(NORMALIZE_ONE);

						}
						else if(Type == LIGHT_SPOT)
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_SPOT_NORMAL]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_SPOT_NORMAL]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetSpotMap();
							SetCubeNormal(NORMALIZE_ONE);

						}
						else 
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_DIR_NORMAL]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_DIR_NORMAL]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetCubeNormal(NORMALIZE_ONE);
						}						

						break;

		case LG_SPEC:
						if(Type == LIGHT_OMNI) 
						{	
							m_Device->SetVertexShader(m_VertexHandle[SHADER_OMNI_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_OMNI_SPEC]);

							ClearTextures();
							SetAttenMap();
							Mat->SetChannelsLight(m_Device);
							SetCubeNormal(NORMALIZE_ZERO);

						}
						else if(Type == LIGHT_SPOT) 
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_SPOT_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_SPOT_SPEC]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetSpotMap();
							SetCubeNormal(NORMALIZE_ZERO);

						}
						else 
						{
							m_Device->SetVertexShader(m_VertexHandle[SHADER_DIR_SPEC]);
							m_Device->SetPixelShader(m_PixelHandle[SHADER_DIR_SPEC]);

							ClearTextures();
							Mat->SetChannelsLight(m_Device);
							SetCubeNormal(NORMALIZE_ZERO);
						}

						break;

	}


}


//______________________________________
//
//	SetMaterialConst 
//
//______________________________________

void Lighting::SetMaterialConst(ShaderMat *Mat)
{

	m_Device->SetVertexShaderConstant(CV_SCALE,D3DXVECTOR4(Mat->m_BumpScale,Mat->m_BumpScale,
														   Mat->m_BumpScale,Mat->m_BumpScale),1);


	m_Device->SetPixelShaderConstant(CP_SPECULAR_COLOR,&D3DXVECTOR4(Mat->m_Specular.r,
															        Mat->m_Specular.g,
															        Mat->m_Specular.b,
															        1.0f),1);

}



//______________________________________
//
//	SetShaderConst 
//
//______________________________________

void Lighting::SetShaderConst(int Index, RenderLight *Light, ShaderMat *Mat)
{
	float Avg;
	float Range;
	float	R,G,B;
	
	m_Device->SetVertexShaderConstant(CV_LIGHT_POSITION,&Light->m_Pos,1);
	m_Device->SetVertexShaderConstant(CV_LIGHT_DIR,&Light->m_Dir,1);

	if(Light->m_Type == LIGHT_SPOT)
	{
		ComputeSpotMatrix(Light);
	}


	Range = Light->m_OuterRange;


	m_Device->SetVertexShaderConstant(CV_LIGHT_OUTER_RANGE,&D3DXVECTOR4(Range,
																		Range,
																		Range,
																		Range),1);

	R = Light->m_Color.x * Mat->m_Diffuse.r;
	G = Light->m_Color.y * Mat->m_Diffuse.g;
	B = Light->m_Color.z * Mat->m_Diffuse.b;

	if(Mat->m_AlphaOn)
	{
		m_Device->SetPixelShaderConstant(CP_DLIGHT_COLOR,&D3DXVECTOR4(R,G,B,1.0f),1);

		m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		m_Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);

	}
	else
	{
		Avg = MAX3(Light->m_Color.x, 
				   Light->m_Color.y,  
			       Light->m_Color.z);
		
		m_Device->SetPixelShaderConstant(CP_DLIGHT_COLOR,&D3DXVECTOR4(R,G,B,Avg),1);

		m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);

	}


	m_Device->SetPixelShaderConstant(CP_REFLECT_AMT,&D3DXVECTOR4(Mat->m_ReflectScale,
																 Mat->m_ReflectScale,
																 Mat->m_ReflectScale,
																 Mat->m_ReflectScale),1);
	

}


//______________________________________
//
//	ClearTextures 
//
//______________________________________

void Lighting::ClearTextures()
{
	int i;

	for(i=0; i < MAX_TMUS; i++)
	{
		m_Device->SetTexture(i,NULL);
	}
}

//______________________________________
//
//	SetRenderStates 
//
//______________________________________

void Lighting::SetRenderStates()
{
	int i;

	for(i=0; i < MAX_TMUS; i++)
	{
		m_Device->SetTexture(i,NULL);
		m_Device->SetTextureStageState(i,D3DTSS_TEXCOORDINDEX,i);
		m_Device->SetTextureStageState(i,D3DTSS_COLOROP,D3DTOP_DISABLE);
		m_Device->SetTextureStageState(i,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		m_Device->SetTextureStageState(i,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		m_Device->SetTextureStageState(i,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
		m_Device->SetTextureStageState(i,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
		m_Device->SetTextureStageState(i,D3DTSS_ADDRESSW,D3DTADDRESS_WRAP);
		m_Device->SetRenderState((D3DRENDERSTATETYPE)(D3DRS_WRAP0 + i),D3DWRAPCOORD_3);
		m_Device->SetTextureStageState(i,D3DTSS_MIPFILTER,D3DTEXF_LINEAR);

	}


	m_Device->SetTextureStageState(2,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
	m_Device->SetTextureStageState(2,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);	
	m_Device->SetTextureStageState(2,D3DTSS_ADDRESSW,D3DTADDRESS_CLAMP);	
	m_Device->SetRenderState(D3DRS_WRAP2,0);

	m_Device->SetTextureStageState(3,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
	m_Device->SetTextureStageState(3,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);	
	m_Device->SetTextureStageState(3,D3DTSS_ADDRESSW,D3DTADDRESS_CLAMP);	
	m_Device->SetRenderState(D3DRS_WRAP3,0);

	m_Device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);


}

//______________________________________
//
//	ComputeSpotMatrix
//
//______________________________________

void Lighting::ComputeSpotMatrix(RenderLight *Light)
{
	D3DXVECTOR3 Test,At,Up,Pos;
	D3DXMATRIX  MatTemp,MatTex,MatWorld,MatView, MatProj;
	D3DXMATRIX  MatWorldViewProj;
	
	Pos.x = Light->m_Pos.x;
	Pos.y = Light->m_Pos.y;
	Pos.z = Light->m_Pos.z;

	At.x = Pos.x - Light->m_Dir.x; 
	At.y = Pos.y - Light->m_Dir.y;
	At.z = Pos.z - Light->m_Dir.z;

	Up.x =  0.0f;          
	Up.y =  1.0f;          
	Up.z =  0.0f;

	D3DXMatrixLookAtLH(&MatView,&Pos,&At,&Up);
	
	D3DXMatrixPerspectiveFovLH(&MatProj,
							   DEG_RAD(Light->m_Angle),
							   1.0f,
							   1.0f,
							   50.0f);

	m_Device->GetTransform(D3DTS_WORLD,&MatWorld);

	D3DXMatrixMultiply(&MatTemp,&MatWorld,&MatView);
	D3DXMatrixMultiply(&MatWorldViewProj,&MatTemp,&MatProj);
	D3DXMatrixIdentity(&MatTex);
	D3DXMatrixMultiply(&MatTex,&MatWorldViewProj,&m_MatScaleBias);
	D3DXMatrixTranspose(&MatTex,&MatTex);

	m_Device->SetVertexShaderConstant(CV_TEXMAT_0,&MatTex(0,0),4);

}


//______________________________________
//
//	SetAttenMap
//
//______________________________________

void Lighting::SetAttenMap()
{		
	m_Device->SetTexture(2,m_Atten);
	m_Device->SetTexture(3,m_Atten);
}

//______________________________________
//
//	SetSpotMap
//
//______________________________________

void Lighting::SetSpotMap()
{		
	m_Device->SetTexture(3,m_Spot);

}

//______________________________________
//
//      SetCubeNormal
//
//______________________________________

void Lighting::SetCubeNormal(int NormalizeChannel)
{               

        switch(NormalizeChannel)
        {

                case 1:
                                m_Device->SetTexture(0,m_CubeNormal);
                                break;

                case 2:
                                m_Device->SetTexture(1,m_CubeNormal);
                                break;

                case 3:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(1,m_CubeNormal);
                                break;
                
                case 4:
                                m_Device->SetTexture(2,m_CubeNormal);
                                break;

                case 5:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                break;

                case 6:
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                break;

                case 7:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                break;

                case 8:
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 9:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 10:
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 11:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 12:
                                m_Device->SetTexture(2,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 13:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 14:
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

                case 15:
                                m_Device->SetTexture(0,m_CubeNormal);
                                m_Device->SetTexture(1,m_CubeNormal);
                                m_Device->SetTexture(2,m_CubeNormal);
                                m_Device->SetTexture(3,m_CubeNormal);
                                break;

        }

}

//______________________________________
//
//	LoadAttenTexture
//
//______________________________________

bool Lighting::LoadAttenTexture(TSTR &Name, LPDIRECT3DDEVICE8 Device)
{		
	HRESULT			Hr;
	void			*Data;
	unsigned long	Size;
	TCHAR error[256];
	sprintf(error,"%s",GetString(IDS_ERROR));

	Hr = S_OK;

	SAFE_RELEASE(m_Atten);

	if(GetFileResource(MAKEINTRESOURCE(IDR_ATTENUATE),"RT_RCDATA",
					   &Data,Size))
	{
		Hr = D3DXCreateTextureFromFileInMemoryEx(Device, 
												 Data,
												 Size,
												 D3DX_DEFAULT,
												 D3DX_DEFAULT,
												 1,
												 0,
												 D3DFMT_A8R8G8B8,
												 D3DPOOL_MANAGED,
												 D3DX_FILTER_NONE,
												 D3DX_FILTER_NONE,
												 0,
												 NULL,
												 NULL,
												 (IDirect3DTexture8 **)&m_Atten);

	}
	else
	{
		Hr = E_FAIL;
	}

	if(Hr != S_OK)
	{
		TSTR	Str;
		Str = TSTR(GetString(IDS_LOAD_FILES)) + Name + TSTR(GetString(IDS_CHECK_PATHS)) + 
			  TSTR(GetString(IDS_ERROR_CODE)) + TSTR(DXGetErrorString8(Hr));

		MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
				   MB_APPLMODAL);

		return(false);
	}


	return(true);
}


//______________________________________
//
//	LoadSpotTexture
//
//______________________________________

bool Lighting::LoadSpotTexture(TSTR &Name, LPDIRECT3DDEVICE8 Device)
{		
	HRESULT			Hr;
	void			*Data;
	unsigned long	Size;
	D3DSURFACE_DESC	Desc;
	float			Bias,X,Y;
	unsigned int	Range;
	TCHAR error[256];
	sprintf(error,"%s",GetString(IDS_ERROR));

	Hr = S_OK;

	SAFE_RELEASE(m_Spot);

	if(GetFileResource(MAKEINTRESOURCE(IDR_SPOTLIGHT),"RT_RCDATA",
					   &Data,Size))
	{
		Hr = D3DXCreateTextureFromFileInMemoryEx(Device, 
												 Data,
												 Size,
												 D3DX_DEFAULT,
												 D3DX_DEFAULT,
												 1,
												 0,
												 D3DFMT_A8R8G8B8,
												 D3DPOOL_MANAGED,
												 D3DX_FILTER_NONE,
												 D3DX_FILTER_NONE,
												 0,
												 NULL,
												 NULL,
												 (IDirect3DTexture8 **)&m_Spot);


	}
	else
	{
		Hr = E_FAIL;
	}

	if(Hr != S_OK)
	{
		TSTR	Str;
		Str = TSTR(GetString(IDS_LOAD_FILES)) + Name + TSTR(GetString(IDS_CHECK_PATHS)) + 
			  TSTR(GetString(IDS_ERROR_CODE)) + TSTR(DXGetErrorString8(Hr));

		MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
				   MB_APPLMODAL);

		return(false);
	}

	//
	//	Set projection matrix
	//	
	((LPDIRECT3DTEXTURE8)m_Spot)->GetLevelDesc(0,&Desc);

	X	  = 0.5f + (0.5f / (float)Desc.Width);
	Y	  = 0.5f + (0.5f / (float)Desc.Height);
	Range = 0xFFFFFFFF >> (32 - 24);
	Bias  = -0.001f * (float)Range;

	m_MatScaleBias = D3DXMATRIX(0.5f,  0.0f,  0.0f,         0.0f,
							    0.0f, -0.5f,  0.0f,         0.0f,
							    0.0f,  0.0f,  (float)Range, 0.0f,
							    X,     Y,     Bias,         1.0f);

	return(true);

}


//______________________________________
//
//	CreateNormalizationCubeMap
//
//______________________________________

bool Lighting::CreateNormalizationCubeMap(LPDIRECT3DDEVICE8 Device, 
										  unsigned long Size)
{
    HRESULT				Hr;
	unsigned long		i,x,y;
    LPDIRECT3DSURFACE8  CubeMapFace;
	D3DXVECTOR3			N;
    float				W,H;
	unsigned long		*Pixel;
    D3DLOCKED_RECT		Lock;


    if(FAILED(Hr = Device->CreateCubeTexture(Size,1,0,D3DFMT_X8R8G8B8, 
                                             D3DPOOL_MANAGED,&m_CubeNormal)))
	{
		return(false);
	}
    

    for(i=0; i < 6; i++)
    {
        m_CubeNormal->GetCubeMapSurface((D3DCUBEMAP_FACES)i,0,&CubeMapFace);

		Hr = CubeMapFace->LockRect(&Lock,NULL,0);

		if(FAILED(Hr))
		{
			return(false);
		}

        Pixel = (unsigned long *)Lock.pBits;

        for(y = 0; y < Size; y++)
        {
			H  = (float)y / (float)(Size - 1);			
			H  = (H * 2.0f) - 1.0f;					
            
			for(x = 0; x < Size; x++)
			{
				W = (float)x / (float)(Size - 1);		
				W = (W * 2.0f) - 1.0f;					

				switch(i)
                {
                    case D3DCUBEMAP_FACE_POSITIVE_X:    
							N.x = +1.0f;
							N.y = -H;
							N.z = -W;
							break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_X:    
							N.x = -1.0f;
							N.y = -H;
							N.z = +W;
							break;
                        
                    case D3DCUBEMAP_FACE_POSITIVE_Y:    
							N.x = +W;
							N.y = +1.0f;
							N.z = +H;
							break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_Y:    
							N.x = +W;
							N.y = -1.0f;
							N.z = -H;
							break;
                        
                    case D3DCUBEMAP_FACE_POSITIVE_Z:    
							N.x = +W;
							N.y = -H;
							N.z = +1.0f;
							break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_Z:    
							N.x = -W;
							N.y = -H;
							N.z = -1.0f;
							break;
                }

                D3DXVec3Normalize(&N,&N);

                *Pixel++ = VectorToRGBA(&N);

            }

        }
        

        CubeMapFace->UnlockRect();
        CubeMapFace->Release();
    }

    return(true);

}

void Lighting::UpdateLights()
{

	Matrix3			Mat;
	int				i;
	ObjectState		Os;
	LightObject		*LightObj;
	LightState		Ls;
	RenderLight		Light;	
	TimeValue		Time = GetCOREInterface()->GetTime();
	Interval		valid;
	Point3			Pos,Target;

	m_Lights.clear();

	for(i=0;i<sceneLights.Count();i++)
	{

		INode * Node = sceneLights[i];
		Os   = Node->EvalWorldState(Time);
			
		if(Os.obj ) 
		{
			LightObj = (LightObject *)Os.obj;
			LightObj->EvalLightState(Time,valid,&Ls);

			Mat				= Node->GetNodeTM(Time);			
			Pos				= Mat.GetTrans();
			Light.m_Angle   = Ls.hotsize;

			if(Light.m_Angle < 2.0f)
			{
				Light.m_Angle = 2.0f;
			}

			Mat = Node->GetNodeTM(Time);
			Light.m_Dir = Mat.GetRow(2);

			Light.m_Type	= (RenderLightType)Ls.type;
			Light.m_Pos		= Pos;
			Light.m_Dir		= Light.m_Dir.Normalize();
			Light.m_Color	= Ls.color * Ls.intens;

			if(Ls.useAtten)
			{
				Light.m_Atten = true;

				if(Ls.attenStart <= 0.0f)
				{
					Light.m_InnerRange	= 1.0f;
				}
				else
				{
					Light.m_InnerRange	= 1.0f / (Ls.attenStart * 2.0f);
				}

				if(Ls.attenEnd <= 0.0f)
				{
					Light.m_OuterRange	= 0.0f;
				}
				else
				{	
					Light.m_OuterRange	= 1.0f / (Ls.attenEnd * 2.0f);
				}	
			}
			else
			{
				Light.m_Atten		= false;
				Light.m_InnerRange	= 0.1f;
				Light.m_OuterRange	= 0.00001f;
			}
			m_Lights.push_back(Light);
		}	
	}
}

void Lighting::DeleteLight(INode * node)
{
	for(int i=0; i < sceneLights.Count(); i++)
	{
		if(sceneLights[i] == node)
		{
			sceneLights.Delete(i,1);
			i=0;
		}
	}

	sceneLights.Shrink();
}

void Lighting::AddLight(INode * node)
{
	sceneLights.Append(1,&node);
}