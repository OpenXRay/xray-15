//_____________________________________________________________________________
//
//	File: ShaderMat.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include "ShaderMat.h"
#include "RenderMesh.h"
#include "ShaderConst.h"

//_____________________________________________________________________________
//
//	Functions	
//_____________________________________________________________________________


//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ShaderMat::ShaderMat()
{
	Init(false);

	m_Name = "default";
}

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ShaderMat::ShaderMat(TSTR &Name)
{
	Init(false);

	m_Name = Name;
}

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ShaderMat::ShaderMat(MatShaderInfo &Info)
{
	Init(false);

	Set(Info);
	
}

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ShaderMat::ShaderMat(const ShaderMat &Other)
{
	*this = Other;
}

//_____________________________________
//
//	Operator = 
//
//_____________________________________

ShaderMat&	ShaderMat::operator = (const ShaderMat &Other)
{
	int i;

	if(this != &Other)
	{
		m_Name				=	Other.m_Name;			
		m_ChannelUse		=	Other.m_ChannelUse;		
		m_ChannelLoaded		=	Other.m_ChannelLoaded;		
		m_Ambient			=	Other.m_Ambient;		 	
		m_Diffuse			=	Other.m_Diffuse;		 	
		m_Specular			=	Other.m_Specular;		
		m_VertexFile		=	Other.m_VertexFile;		
		m_VertexHandle		=	Other.m_VertexHandle;	
		m_PixelFile			=	Other.m_PixelFile;		
		m_PixelHandle		=	Other.m_PixelHandle;	 	
		m_AlphaPixelFile	=	Other.m_PixelFile;		
		m_AlphaPixelHandle	=	Other.m_PixelHandle;	 	
		m_LightGroup		=   Other.m_LightGroup;
		m_BumpScale			=	Other.m_BumpScale;		
		m_MixScale			=	Other.m_MixScale;
		m_ReflectScale		=	Other.m_ReflectScale;
		m_Alpha				=	Other.m_Alpha;
		m_AlphaOn			=	Other.m_AlphaOn;
		
		for(i=0; i < CHANNEL_MAX; i++)
		{	
			m_Shader[i] = Other.m_Shader[i];	
		}

	}

	return(*this);

}

//_____________________________________
//
//	Set
//
//_____________________________________

void ShaderMat::Set(MatShaderInfo &Info)
{
	m_ChannelUse		= Info.m_ChannelUse;		
	m_VertexFile		= Info.m_VertexFile;	
	m_VertexHandle		= Info.m_VertexHandle;	
	m_PixelFile			= Info.m_PixelFile;			
	m_PixelHandle		= Info.m_PixelHandle;	
	m_AlphaPixelFile	= Info.m_AlphaPixelFile;			
	m_AlphaPixelHandle	= Info.m_AlphaPixelHandle;	
	
}


//_____________________________________
//
//	Default destructor 
//
//_____________________________________

ShaderMat::~ShaderMat()
{
}

//_____________________________________
//
//	Init 
//
//_____________________________________

void ShaderMat::Init(bool DeleteMaps)
{
	int i;

	m_Ambient.r = 0.0f;
	m_Ambient.g = 0.0f;
	m_Ambient.b = 0.0f;
	m_Ambient.a = 0.0f; 
     
	m_Diffuse.r	= 1.0f;
	m_Diffuse.g	= 1.0f;
	m_Diffuse.b	= 1.0f;
	m_Diffuse.a	= 1.0f;
      
	m_Specular.r = 1.0f;
	m_Specular.g = 1.0f;
	m_Specular.b = 1.0f;
	m_Specular.a = 1.0f;
    

	for(i=0; i < CHANNEL_MAX; i++)
	{
		UnLoadTexture((RMatChannel)i);
		m_Shader[i].SetChannel((RMatChannel)i);
	}

	for(i=0; i < CHANNEL_MAX; i++)
	{
		m_Shader[i].m_Stage = SHADER_STAGE_UNDEFINED;
	}

	m_ChannelUse		= MAT_NONE_ON;
	m_ChannelLoaded		= MAT_NONE_ON;
	m_LightGroup		= LG_DEFAULT;
	m_VertexHandle		= 0;
	m_PixelHandle		= 0;
	m_AlphaPixelHandle	= 0;
	m_BumpScale			= 1.0f;
	m_MixScale			= 0.5f;
	m_ReflectScale		= 0.5f;
	m_Alpha				= 1.0f;
	m_AlphaOn			= false;

}


//_____________________________________
//
//	ResetMaterial 
//
//_____________________________________

void ShaderMat::ResetMaterial()
{
	Init(true);
}


//_____________________________________
//
//	SetLightGroup 
//
//_____________________________________

void ShaderMat::SetLightGroup()
{
	int	Num,Index;

	Num = NumLoaded(Index);

	m_LightGroup = LG_DEFAULT;

	if(AllLoaded(MAT_NORMAL_ON | MAT_SPECULAR_ON) || 
	   AllLoaded(MAT_BUMP_ON   | MAT_SPECULAR_ON))
	{
		m_LightGroup = LG_NORMALSPEC;
	}
	else if((AllLoaded(MAT_NORMAL_ON | MAT_REFLECTION_ON) || 
			 AllLoaded(MAT_BUMP_ON   | MAT_REFLECTION_ON)) && 
			 Num == 2)
	{
		m_LightGroup = LG_DEFAULT;
	}
	else if(AllLoaded(MAT_NORMAL_ON) || 
			AllLoaded(MAT_BUMP_ON))
	{
		m_LightGroup = LG_NORMAL;
	}
	else if(AllLoaded(MAT_SPECULAR_ON))
	{
		m_LightGroup = LG_SPEC;
	}

}

//_____________________________________
//
//	ReLoadAll 
//
//_____________________________________

void ShaderMat::ReLoadAll(LPDIRECT3DDEVICE9 Device)
{
	int i;

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(m_Shader[i].m_On)
		{
			if(!m_Shader[i].IsLoaded())
			{
				LoadTexture((RMatChannel)i,Device);
			}
			else
			{
				m_ChannelLoaded |= (1 << i);
			}	
		}
		else
		{
			UnLoadTexture((RMatChannel)i);
		}
	}

}


//_____________________________________
//
//	SetChannelsSpecular 
//
//_____________________________________

void ShaderMat::SetChannelsSpecular(LPDIRECT3DDEVICE9 Device)
{
	m_Shader[CHANNEL_MASK].SetTexture(Device);

}

//_____________________________________
//
//	SetChannelsLight 
//
//_____________________________________

void ShaderMat::SetChannelsLight(LPDIRECT3DDEVICE9 Device)
{
	m_Shader[CHANNEL_NORMAL].SetTexture(Device);
	m_Shader[CHANNEL_BUMP].SetTexture(Device);
	m_Shader[CHANNEL_SPECULAR].SetTexture(Device);

}

//_____________________________________
//
//	SetChannelsMat 
//
//_____________________________________

void ShaderMat::SetChannelsMat(LPDIRECT3DDEVICE9 Device)
{
	int		i,Num,Index;

	for(i=0; i < MAX_TMUS; i++)
	{
		Device->SetTexture(i,NULL);
		Device->SetTextureStageState(i,D3DTSS_TEXCOORDINDEX,i);
		Device->SetTextureStageState(i,D3DTSS_COLOROP,D3DTOP_DISABLE);
		Device->SetTextureStageState(i,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		Device->SetSamplerState(i,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
		Device->SetSamplerState(i,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
		Device->SetSamplerState(i,D3DSAMP_ADDRESSW,D3DTADDRESS_WRAP);
		Device->SetRenderState((D3DRENDERSTATETYPE)(D3DRS_WRAP0 + i),D3DWRAPCOORD_3);

	}

	Device->SetRenderState(D3DRS_TEXTUREFACTOR,FloatToRGBA(m_MixScale));

	Num = NumLoaded(Index);

	if(Num == 1)
	{
		m_Shader[Index].SetTexture(Device,m_AlphaOn);
	}
	else 
	{
		for(i=0; i < CHANNEL_MAX; i++)
		{
			switch(i)
			{
				case CHANNEL_NORMAL:
				case CHANNEL_BUMP:
									if(m_ChannelLoaded & MAT_REFLECTION_ON && 
									   Num == 2)
									{				
										m_Shader[i].SetTexture(Device,m_AlphaOn);
									}
									break;

				case CHANNEL_DIFFUSE:
				case CHANNEL_DETAIL:
				case CHANNEL_REFLECTION:
									m_Shader[i].SetTexture(Device,m_AlphaOn);
									break;
										
			}

		}

	}

}

//_____________________________________
//
//	UpdateMaterial 
//
//_____________________________________

void ShaderMat::UpdateMaterial(float Time)
{
	int i;

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(m_Shader[i].IsMovie())
		{
			m_Shader[i].m_Movie->SetTarget(m_Shader[i].m_Texture);
			m_Shader[i].m_Movie->Evaluate(Time);
		}
	}

}

//_____________________________________
//
//	Evaluate 
//
//_____________________________________

bool ShaderMat::Evaluate(float Time, LPDIRECT3DDEVICE9 Device, RenderMesh *RMesh)
{

	if(m_VertexHandle)
	{
		Device->SetVertexShader(m_VertexHandle);
	}
	else
	{
		return(false);
	}

	SetChannelsMat(Device);

	Device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE); 

	if(m_AlphaOn)
	{
		Device->SetPixelShader(m_AlphaPixelHandle);

		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	}
	else
	{
		Device->SetPixelShader(m_PixelHandle);

		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO);
	}

	if(m_PixelHandle || m_AlphaPixelHandle)
	{
		Device->SetPixelShaderConstantF(CP_MIX_AMT,(float*)&D3DXVECTOR4(m_MixScale,
															   m_MixScale,
															   m_MixScale,
															   m_MixScale),1);

		Device->SetPixelShaderConstantF(CP_REFLECT_AMT,(float*)&D3DXVECTOR4(m_ReflectScale,
																   m_ReflectScale,
																   m_ReflectScale,
																   m_ReflectScale),1);
	}

//	m_Device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_MAX);

	RMesh->Render(Device);

	return(true);

}




//_____________________________________
//
//	SetStatesFromChannels	 
//
//_____________________________________

void ShaderMat::SetStatesFromChannels(int ChannelUse)
{
	int i;

	for(i=0; i < CHANNEL_MAX; i++)
	{
//		m_Shader[i].m_On		= false;	
		m_Shader[i].m_Stage		= SHADER_STAGE_UNDEFINED;

		m_Shader[i].m_Color1	= SHADER_TEXTURE;
		m_Shader[i].m_Color2	= SHADER_DIFFUSE;
		m_Shader[i].m_ColorOp	= SHADER_DISABLE;
		m_Shader[i].m_Alpha1	= SHADER_TEXTURE;
		m_Shader[i].m_Alpha2	= SHADER_DIFFUSE;
		m_Shader[i].m_AlphaOp	= SHADER_SELECTARG1;

	}

	switch(ChannelUse)
	{
		//
		//	Diffuse
		//
		case MAT_DIFFUSE_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
				break;

			
		//
		//	Normal
		//
		case MAT_NORMAL_ON:

				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_SELECTARG1;
				break;

		//
		//	Specular
		//
		case MAT_SPECULAR_ON:

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;
				break;

		//
		//	Detail
		//
		case MAT_DETAIL_ON:

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_SELECTARG1;
				break;

		//
		//	Mask
		//
		case MAT_MASK_ON:

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;
				break;
		//
		//	Reflection
		//
		case MAT_REFLECTION_ON:

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_SELECTARG1;
				break;

		//
		//	Bump
		//
		case MAT_BUMP_ON:

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_SELECTARG1;
				break;

		//
		//	Diffuse + Normal
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;
				break;

		//
		//	Diffuse + Specular
   		//
		case MAT_DIFFUSE_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Normal + Specular
   		//
		case MAT_NORMAL_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Diffuse + Normal + Specular
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;




		//
		//	Diffuse + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;
				break;


			
		//
		//	Diffuse + Normal + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;
				break;


		//
		//	Diffuse + Specular + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_SPECULAR_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Diffuse + Normal + Specular + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;
						
				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Diffuse + Normal + Reflection	+ Specular
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_REFLECTION_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				break;

		//
		//	Diffuse + Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Normal + Specular + Mask
   		//
		case MAT_NORMAL_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Normal + Specular	+ Mask
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Specular + Detail	+ Mask
   		//
		case MAT_DIFFUSE_ON + MAT_SPECULAR_ON + MAT_DETAIL_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Normal + Specular + Detail + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON + 
			 MAT_DETAIL_ON  + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;
						
				m_Shader[CHANNEL_NORMAL].m_On		= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;

		//
		//	Diffuse + Normal + Reflection + Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_REFLECTION_ON + 
			 MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;


				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Reflection
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;
				break;

		//
		//	Normal + Reflection
   		//
		case MAT_NORMAL_ON + MAT_REFLECTION_ON:

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_THREE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				break;


		//
		//	Diffuse + Normal + Reflection
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_REFLECTION_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				break;

		//
		//	Diffuse + Reflection + Specular
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Reflection + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				break;


		//
		//	Diffuse + Normal + Reflection + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_REFLECTION_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				break;


		//
		//	Diffuse + Reflection + Detail + Specular
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON + MAT_DETAIL_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Normal + Specular + Reflection + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON + 
			 MAT_REFLECTION_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				break;


		//
		//	Diffuse + Reflection + Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;

		//
		//	Diffuse + Reflection + Detail + Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_REFLECTION_ON + MAT_DETAIL_ON + 
			 MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Normal + Specular + Reflection + Detail + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_NORMAL_ON + MAT_SPECULAR_ON + MAT_REFLECTION_ON + 
			 MAT_DETAIL_ON  + MAT_MASK_ON:

				m_Shader[CHANNEL_NORMAL].m_On			= true;	
				m_Shader[CHANNEL_NORMAL].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_NORMAL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_NORMAL].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_NORMAL].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Bump
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;
				break;

		//
		//	Bump + Specular
   		//
		case MAT_BUMP_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Diffuse + Bump + Specular
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Diffuse + Bump + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;
				break;


		//
		//	Diffuse + Bump + Specular + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;
				break;


		//
		//	Bump + Specular + Mask
   		//
		case MAT_BUMP_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Bump + Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;


		//
		//	Diffuse + Bump + Specular + Detail + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON + 
			 MAT_DETAIL_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On		= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage	= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2	= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp	= SHADER_SELECTARG1;
						
				m_Shader[CHANNEL_DETAIL].m_On		 = true;	
				m_Shader[CHANNEL_DETAIL].m_Stage	 = SHADER_STAGE_ONE;
				m_Shader[CHANNEL_DETAIL].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2	 = SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp	 = SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_BUMP].m_On			= true;	
				m_Shader[CHANNEL_BUMP].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp	= SHADER_DOTPRODUCT3;

				m_Shader[CHANNEL_SPECULAR].m_On		 = true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage	 = SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1	 = SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2	 = SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp = SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;

		//
		//	Bump + Reflection
   		//
		case MAT_BUMP_ON + MAT_REFLECTION_ON:

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_THREE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				break;

		//
		//	Diffuse + Bump + Reflection
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_REFLECTION_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				break;


		//
		//	Diffuse + Bump + Reflection	+ Specular
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_REFLECTION_ON + MAT_SPECULAR_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;


				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				break;



		//
		//	Diffuse + Bump + Reflection + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_REFLECTION_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				break;


		//
		//	Diffuse + Bump + Specular + Reflection + Detail
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON + 
			 MAT_REFLECTION_ON + MAT_DETAIL_ON:

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				break;

		//
		//	Diffuse + Bump + Reflection	+ Specular + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_REFLECTION_ON + 
			 MAT_SPECULAR_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;


				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;

		//
		//	Diffuse + Bump + Specular + Reflection + Detail + Mask
   		//
		case MAT_DIFFUSE_ON + MAT_BUMP_ON + MAT_SPECULAR_ON + MAT_REFLECTION_ON + 
			 MAT_DETAIL_ON + MAT_MASK_ON:

				m_Shader[CHANNEL_BUMP].m_On				= true;	
				m_Shader[CHANNEL_BUMP].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_BUMP].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_BUMP].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_BUMP].m_ColorOp		= SHADER_DOTPRODUCT3;
						
				m_Shader[CHANNEL_SPECULAR].m_On			= true;	
				m_Shader[CHANNEL_SPECULAR].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_SPECULAR].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_SPECULAR].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_SPECULAR].m_ColorOp	= SHADER_SELECTARG1;

				m_Shader[CHANNEL_DIFFUSE].m_On			= true;	
				m_Shader[CHANNEL_DIFFUSE].m_Stage		= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_DIFFUSE].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DIFFUSE].m_Color2		= SHADER_DIFFUSE;
				m_Shader[CHANNEL_DIFFUSE].m_ColorOp		= SHADER_SELECTARG1;

				m_Shader[CHANNEL_REFLECTION].m_On		= true;	
				m_Shader[CHANNEL_REFLECTION].m_Stage	= SHADER_STAGE_ONE;
				m_Shader[CHANNEL_REFLECTION].m_Color1	= SHADER_TEXTURE;
				m_Shader[CHANNEL_REFLECTION].m_Color2	= SHADER_CURRENT;
				m_Shader[CHANNEL_REFLECTION].m_ColorOp	= SHADER_ADDSIGNED;

				m_Shader[CHANNEL_DETAIL].m_On			= true;	
				m_Shader[CHANNEL_DETAIL].m_Stage		= SHADER_STAGE_TWO;
				m_Shader[CHANNEL_DETAIL].m_Color1		= SHADER_TEXTURE;
				m_Shader[CHANNEL_DETAIL].m_Color2		= SHADER_CURRENT;
				m_Shader[CHANNEL_DETAIL].m_ColorOp		= SHADER_BLENDFACTORALPHA;

				m_Shader[CHANNEL_MASK].m_On				= true;	
				m_Shader[CHANNEL_MASK].m_Stage			= SHADER_STAGE_ZERO;
				m_Shader[CHANNEL_MASK].m_Color1			= SHADER_TEXTURE;
				m_Shader[CHANNEL_MASK].m_Color2			= SHADER_DIFFUSE;
				m_Shader[CHANNEL_MASK].m_ColorOp		= SHADER_SELECTARG1;

				break;



		default:

				break;
					
	}

}

