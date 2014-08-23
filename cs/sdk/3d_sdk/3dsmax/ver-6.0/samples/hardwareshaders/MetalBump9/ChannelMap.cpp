//_____________________________________________________________________________
//
//	File: ChannelMap.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include "Movie.h"
#include "Max.h"
#include "ChannelMap.h"
#include "TextureMgr.h"
#include "NVTexture.h"
#include "resource.h"


extern TCHAR *GetString(int id);
//_____________________________________________________________________________
//
//	Functions	
//_____________________________________________________________________________

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ChannelMap::ChannelMap()
{
	m_Texture	 = NULL;
	m_On		 = false;
	m_Channel	 = CHANNEL_DIFFUSE;	
	m_Stage		 = SHADER_STAGE_UNDEFINED;
	m_UVIndex	 = 0;
	m_Type		 = TEX_UNDEFINED;
	m_Movie		 = NULL;

	m_Color1	 = SHADER_TEXTURE;
	m_Color2	 = SHADER_DIFFUSE;
	m_ColorOp	 = SHADER_DISABLE;

	m_Alpha1	 = SHADER_TEXTURE;
	m_Alpha2	 = SHADER_DIFFUSE;
	m_AlphaOp	 = SHADER_DISABLE;
}

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

ChannelMap::ChannelMap(const ChannelMap &Other)
{
	*this = Other;
}

//_____________________________________
//
//	Operator =  
//
//_____________________________________

ChannelMap& ChannelMap::operator = (const ChannelMap &Other)
{

	if(this != &Other)
	{
		m_Name		= Other.m_Name; 		
		m_Stage		= Other.m_Stage;		
		m_Channel	= Other.m_Channel; 
		m_Texture	= Other.m_Texture; 
		m_On		= Other.m_On;		 
		m_Type		= Other.m_Type;	
		m_Color1  	= Other.m_Color1; 
		m_Color2  	= Other.m_Color2; 
		m_ColorOp 	= Other.m_ColorOp; 
		m_Alpha1  	= Other.m_Alpha1; 
		m_Alpha2  	= Other.m_Alpha2; 
		m_AlphaOp 	= Other.m_AlphaOp;
		m_UVIndex	= Other.m_UVIndex;
		m_Movie		= Other.m_Movie;

		if(!m_Movie && m_Texture)
		{
			TextureMgr::GetIPtr()->AddRef(m_Name,m_Type);
		}

	}

	return(*this);
}

//______________________________________
//
//	Default destructor 
//
//______________________________________

ChannelMap::~ChannelMap()
{

	if(m_Movie)
	{
		delete m_Movie;
		SAFE_RELEASE(m_Texture);
	}
	else
	{
		TextureMgr::GetIPtr()->Release(m_Name,m_Type);
	}
		
}


//_____________________________________
//
//	SetChannel
//
//_____________________________________

void ChannelMap::SetChannel(RMatChannel Channel)
{
	UnLoadTexture();
	m_Channel = Channel;

	switch(Channel)
	{
		case CHANNEL_DIFFUSE:
								m_UVIndex = 0;
								break;

		case CHANNEL_DETAIL:
								m_UVIndex = 1;
								break;

		case CHANNEL_SPECULAR:
		case CHANNEL_MASK:
								m_UVIndex = 2;
								break;

		case CHANNEL_NORMAL:
		case CHANNEL_BUMP:
								m_UVIndex = 3;
								break;

		case CHANNEL_REFLECTION:
		default:				
								m_UVIndex = 0;
								break;

							
	}

}

//______________________________________
//
//	UnLoadTexture
//
//______________________________________

void ChannelMap::UnLoadTexture()
{
	if(m_Movie)
	{
		delete m_Movie;
		SAFE_RELEASE(m_Texture);
	}
	else
	{
		TextureMgr::GetIPtr()->Release(m_Name,m_Type);
	}

	m_Texture = NULL;
	m_Movie = NULL;

}


//______________________________________
//
//	SetTexture
//
//______________________________________

void ChannelMap::SetTexture(LPDIRECT3DDEVICE9 Device, bool AlphaOn)
{

	if(m_Stage != SHADER_STAGE_UNDEFINED)
	{
		Device->SetTexture(m_Stage,m_Texture);
		// DirectX 9.0 requires that this be set to match the texture stage.
		// A Shader will ignore this information anyway, but the API needs to
		// set it this way because of some IHV HW limitations.
		// Device->SetTextureStageState(m_Stage,D3DTSS_TEXCOORDINDEX,m_UVIndex);
		Device->SetTextureStageState(m_Stage,D3DTSS_TEXCOORDINDEX,m_Stage);
	}

	if(m_Movie)
	{
		Device->SetSamplerState(m_Stage,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
	}

	if(m_On)
	{
		Device->SetTextureStageState(m_Stage,D3DTSS_COLORARG1,m_Color1);
		Device->SetTextureStageState(m_Stage,D3DTSS_COLORARG2,m_Color2);
		Device->SetTextureStageState(m_Stage,D3DTSS_COLOROP,  m_ColorOp);

		if(AlphaOn)
		{
			Device->SetTextureStageState(m_Stage,D3DTSS_ALPHAARG1, m_Alpha1);
			Device->SetTextureStageState(m_Stage,D3DTSS_ALPHAARG2, m_Alpha2);
			Device->SetTextureStageState(m_Stage,D3DTSS_ALPHAOP,   m_AlphaOp);
		}

	}
	
}

//______________________________________
//
//	GetTexture
//
//______________________________________

IDirect3DBaseTexture9* ChannelMap::GetTexture()
{
	return(m_Texture);
}



//______________________________________
//
//	LoadTexture
//
//______________________________________

bool ChannelMap::LoadTexture(LPDIRECT3DDEVICE9 Device)
{		
	bool		Ret;
	int			Width,Height,Bpp;
	HRESULT		Hr;
	_D3DFORMAT	Format;

	TCHAR error[256];

	Ret = false;

	if(m_Name.Length() && m_Name != TSTR("None"))
	{
		if(strstr(m_Name,".AVI") || strstr(m_Name,".avi"))
		{ 
			m_Movie = new Movie;
			m_Type	= TEX_STANDARD;

			if(m_Movie->Load(m_Name,Width,Height,Bpp,24))
			{
				Width  = LargestPower2(Width);
				Height = LargestPower2(Height);


				if(m_Channel == CHANNEL_NORMAL)
				{
					Format = D3DFMT_Q8W8V8U8;
				}
				else
				{
					Format = D3DFMT_A8R8G8B8; 	
				}

				Hr = Device->CreateTexture(Width,Height,
										   1,0,Format,
										   D3DPOOL_MANAGED,(IDirect3DTexture9 **)&m_Texture,NULL);

				if(Hr == S_OK)
				{
					m_Movie->SetTarget(m_Texture);
				}
				else
				{
					delete m_Movie;

					m_Movie		= NULL;
					m_Texture	= NULL;
					Ret			= false;
				}
		
				Ret = true;
			}
			else
			{
				delete m_Movie;
				m_Movie = NULL;
				Ret		= false;
			}
		}	
		else
		{
			switch(m_Channel)
			{
				case CHANNEL_DIFFUSE:
				case CHANNEL_DETAIL:
				default:					
										Ret = LoadSingleTexture(Device);
										break;

				case CHANNEL_SPECULAR:

										Ret = LoadSpecTexture(Device);
										break;

				case CHANNEL_MASK:
										Ret = LoadSingleTexture(Device);

										if(Ret)
										{
											GrayScaleAlpha((IDirect3DTexture9 *)m_Texture);
										}	

										break;



				case CHANNEL_NORMAL:
										Ret = LoadNormalTexture(Device);
										break;

				case CHANNEL_BUMP:
										Ret = LoadBumpTexture(Device);
										break;

				case CHANNEL_REFLECTION:
										Ret = LoadCubeTexture(Device);
										break;

			}

		}

		if(Ret == false)
		{
			SAFE_RELEASE(m_Texture);

			TSTR	Str;
			sprintf(error,"%s",GetString(IDS_ERROR));
			Str = TSTR(GetString(IDS_LOAD_FILES)) + m_Name + TSTR(GetString(IDS_CHECK_PATHS));
			MessageBox(NULL,Str,error,MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);
		}


	}

	return(Ret);

}



//______________________________________
//
//	LoadSingleTexture
//
//______________________________________

bool ChannelMap::LoadSingleTexture(LPDIRECT3DDEVICE9 Device, bool MipMap)
{		
	TCHAR   *MapPath;
	HRESULT  Hr;
	int		 NumMip,Index;

	Hr		= S_OK;
	m_Type	= TEX_STANDARD;
	Index	= TextureMgr::GetIPtr()->Find(m_Name,m_Type);

	if(Index < 0)
	{
		SAFE_RELEASE(m_Texture);

		MapPath = FindFile(m_Name);

		if(MipMap)
		{
			NumMip = 0;
		}
		else
		{
			NumMip = 1;
		}

		Hr = D3DXCreateTextureFromFileEx(Device, 
										 MapPath,
										 D3DX_DEFAULT,
										 D3DX_DEFAULT,
										 D3DX_DEFAULT,
										 NumMip,
										 D3DFMT_A8R8G8B8,
										 D3DPOOL_MANAGED,
										 D3DX_FILTER_LINEAR,
										 D3DX_FILTER_LINEAR,
										 0,
										 NULL,
										 NULL,
										 (IDirect3DTexture9 **)&m_Texture);

	


		if(Hr == S_OK)
		{
			TextureMgr::GetIPtr()->Add(m_Name,m_Type,m_Texture);

			return(true);
		}

	}
	else
	{
		m_Texture = TextureMgr::GetIPtr()->Get(Index);

		return(true);
	}
	
	return(false);

}



//______________________________________
//
//	LoadSpecTexture
//
//______________________________________

bool ChannelMap::LoadSpecTexture(LPDIRECT3DDEVICE9 Device)
{		
	HRESULT			Hr;
	int				Index;
	void			*Data;
	unsigned long	Size;

	Hr		= S_OK;
	m_Type	= TEX_STANDARD;
	Index	= TextureMgr::GetIPtr()->Find(m_Name,m_Type);

	if(Index < 0)
	{
		SAFE_RELEASE(m_Texture);

		if(GetFileResource(MAKEINTRESOURCE(IDR_SPECULAR),"RT_RCDATA",
						   &Data,Size))
		{
			Hr = D3DXCreateTextureFromFileInMemoryEx(Device, 
													 Data,
													 Size,
													 D3DX_DEFAULT,
													 D3DX_DEFAULT,
													 0,
													 0,
													 D3DFMT_A8R8G8B8,
													 D3DPOOL_MANAGED,
													 D3DX_FILTER_LINEAR | D3DX_FILTER_MIRROR,
													 D3DX_FILTER_LINEAR | D3DX_FILTER_MIRROR,
													 0,
													 NULL,
													 NULL,
													 (IDirect3DTexture9 **)&m_Texture);

		}
		else
		{
			Hr = E_FAIL;
		}

		if(Hr == S_OK)
		{
			TextureMgr::GetIPtr()->Add(m_Name,m_Type,m_Texture);

			return(true);
		}

	}
	else
	{
		m_Texture = TextureMgr::GetIPtr()->Get(Index);

		return(true);
	}

	return(false);

}

//______________________________________
//
//	LoadNormalTexture
//
//______________________________________

bool ChannelMap::LoadNormalTexture(LPDIRECT3DDEVICE9 Device)
{		
	TCHAR   *MapPath;
	HRESULT  Hr;
	int		 Index;

	Hr		= S_OK;
	m_Type	= TEX_NORMAL;
	Index	= TextureMgr::GetIPtr()->Find(m_Name,m_Type);

	if(Index < 0)
	{
		SAFE_RELEASE(m_Texture);

		MapPath = FindFile(m_Name);

		Hr = D3DXCreateTextureFromFileEx(Device, 
										 MapPath,
										 D3DX_DEFAULT,
										 D3DX_DEFAULT,
										 0,
										 0,
										 D3DFMT_A8R8G8B8,
										 D3DPOOL_MANAGED,
										 D3DX_FILTER_LINEAR,
										 D3DX_FILTER_LINEAR,
										 0,
										 NULL,
										 NULL,
										 (IDirect3DTexture9 **)&m_Texture);

		if(Hr == S_OK)
		{
			ConvertToBias(Device,(IDirect3DTexture9 *)m_Texture);
			TextureMgr::GetIPtr()->Add(m_Name,m_Type,m_Texture);

			return(true);
		}

	}
	else
	{
		m_Texture = TextureMgr::GetIPtr()->Get(Index);

		return(true);
	}

	return(false);

}

//______________________________________
//
//	LoadCubeTexture
//
//______________________________________

bool ChannelMap::LoadCubeTexture(LPDIRECT3DDEVICE9 Device)
{
	TCHAR   *MapPath;
	HRESULT  Hr;
	int		 Index;

	Hr		= S_OK;
	m_Type	= TEX_CUBE;
	Index	= TextureMgr::GetIPtr()->Find(m_Name,m_Type);

	if(Index < 0)
	{
		SAFE_RELEASE(m_Texture);

		MapPath = FindFile(m_Name);

		Hr = D3DXCreateCubeTextureFromFileExA(Device,
											  MapPath, 
											  D3DX_DEFAULT,
											  D3DX_DEFAULT,
											  0,
											  D3DFMT_A8R8G8B8,
											  D3DPOOL_MANAGED,
											  D3DX_FILTER_LINEAR,
											  D3DX_FILTER_LINEAR,
											  0,
											  NULL,
											  NULL,
											  (IDirect3DCubeTexture9 **)&m_Texture);


		if(Hr == S_OK)
		{
			TextureMgr::GetIPtr()->Add(m_Name,m_Type,m_Texture);

			return(true);
		}

	}
	else
	{
		m_Texture = TextureMgr::GetIPtr()->Get(Index);

		return(true);
	}

	return(false);
}


//______________________________________
//
//	LoadBumpTexture
//
//______________________________________

bool ChannelMap::LoadBumpTexture(LPDIRECT3DDEVICE9 Device)
{
	TCHAR				*MapPath;
	HRESULT				Hr;
	IDirect3DTexture9	*Texture;
	int					Index;

	Hr		= S_OK;
	m_Type	= TEX_BUMP;
	Texture = NULL;
	Index   = TextureMgr::GetIPtr()->Find(m_Name,m_Type);

	if(Index < 0)
	{
		SAFE_RELEASE(m_Texture);
		
		MapPath = FindFile(m_Name);

		Hr = D3DXCreateTextureFromFileEx(Device, 
										 MapPath,
										 D3DX_DEFAULT,
										 D3DX_DEFAULT,
										 0,
										 0,
										 D3DFMT_A8R8G8B8,
										 D3DPOOL_MANAGED,
    									  D3DX_FILTER_LINEAR,
										  D3DX_FILTER_LINEAR,
										 0,
										 NULL,
										 NULL,
										 &Texture);



		if(Hr == S_OK)
		{
			m_Texture = NVTexture2::CreateNormalMap(Device,Texture);
			TextureMgr::GetIPtr()->Add(m_Name,m_Type,m_Texture);

			SAFE_RELEASE(Texture)

			return(true);
		}
	}
	else
	{
		m_Texture = TextureMgr::GetIPtr()->Get(Index);

		return(true);
	}

	return(false);
}

//______________________________________
//
//	GetBitmapPtr
//
//______________________________________

unsigned char *ChannelMap::GetBitmapPtr(int &Pitch, int &Height)
{
	D3DLOCKED_RECT	Rect;
	D3DSURFACE_DESC	Desc;
	HRESULT			Hr;

	Pitch  = 0;
	Height = 0;

	if(m_Texture)
	{
		((LPDIRECT3DTEXTURE9)m_Texture)->GetLevelDesc(0,&Desc);

		Hr = ((LPDIRECT3DTEXTURE9)m_Texture)->LockRect(0,&Rect,NULL,0);
		
		if(Hr == S_OK)
		{
			Pitch  = Rect.Pitch;
			Height = Desc.Height;

			return((unsigned char *)Rect.pBits);
		}

	}

	return(NULL);

}

//______________________________________
//
//	GetBitmapPixel
//
//______________________________________

unsigned long ChannelMap::GetBitmapPixel(float U, float V)
{
	unsigned long		Color;
	D3DLOCKED_RECT		Rect;
	D3DSURFACE_DESC		Desc;
	IDirect3DTexture9	*Tex;
	int					Tx,Ty,Width,Height;
	HRESULT				Hr;

	Color = 0;
		
	if(m_Texture)
	{
		Tex = (LPDIRECT3DTEXTURE9) m_Texture;
		Tex->GetLevelDesc(0,&Desc);
		
		Hr = Tex->LockRect(0,&Rect,NULL,0);
		
		if(Hr == S_OK)
		{
			Width  = Desc.Width;
			Height = Desc.Height;

			while(U < 0.0f)
			{
				U += 1.0f;
			}

			while(V < 0.0f)
			{
				V += 1.0f;
			}

			Tx = (int)(U * Width);
			Ty = (int)(V * Height);

			if(Tx < 0)
			{	
				Tx = -Tx;
			}	
			if(Tx >= Width) 
			{	
				Tx %= Width;
			}
			if(Ty < 0)
			{	
				Ty = -Ty;
			}	
			if(Ty >= Height) 
			{	
				Ty %= Height;
			}

			Color = (*(unsigned long*) (((unsigned char*)Rect.pBits) + (Ty * Rect.Pitch) + 
					(Tx * sizeof(unsigned long))));

			Tex->UnlockRect(0);

		}
	}

	return(Color);

}

//______________________________________
//
//	ReleaseBitmapPtr
//
//______________________________________

void ChannelMap::ReleaseBitmapPtr()
{
	if(m_Texture)
	{
		((LPDIRECT3DTEXTURE9)m_Texture)->UnlockRect(0);
	}

}

//______________________________________
//
//	ConvertToBias
//
//______________________________________

bool ChannelMap::ConvertToBias(LPDIRECT3DDEVICE9 Device, LPDIRECT3DTEXTURE9 Source)
{
	LPDIRECT3DTEXTURE9	NormalMap;
	LPDIRECT3DSURFACE9	SourceTex,DestTex;
	D3DLOCKED_RECT		LockedDest;
	D3DLOCKED_RECT		LockedSource;
	D3DSURFACE_DESC		DescDest;
	D3DSURFACE_DESC		DescSource;
	D3DXVECTOR3			Normal;
	HRESULT				Hr;
	unsigned long		i,j,Width,Height;
	int					Level;

	assert(Source);

	NormalMap = NULL;
	Source->GetLevelDesc(0, &DescSource);

	Hr = Device->CreateTexture(DescSource.Width, DescSource.Height,
							   Source->GetLevelCount(),0, 
							   D3DFMT_Q8W8V8U8, D3DPOOL_MANAGED, &NormalMap,NULL);
	if(FAILED(Hr))
	{
		return(false);
	}

	for(Level=0; Level < NormalMap->GetLevelCount(); Level++)
	{
		Source->GetSurfaceLevel(Level,&SourceTex);
		NormalMap->GetSurfaceLevel(Level,&DestTex);

		NormalMap->GetLevelDesc(Level,&DescDest);

		Width  = DescDest.Width;
		Height = DescDest.Height;

		Hr = SourceTex->LockRect(&LockedSource,NULL,0);
		if(FAILED(Hr))
		{
			return(false);
		}

		Hr = DestTex->LockRect(&LockedDest,NULL,0);
		if(FAILED(Hr))
		{
			return(false);
		}		

		ZeroMemory(LockedDest.pBits,(LockedDest.Pitch * Height));

  		for(i=0; i < Width; i++)
		{
			for(j=0; j < Height; j++)
			{
				Normal.x = ((float)((GetMapData(LockedSource,i,j) >> 16) & 0xFF)) / 255.0f;
				Normal.y = ((float)((GetMapData(LockedSource,i,j) >> 8)  & 0xFF)) / 255.0f;
				Normal.z = ((float)((GetMapData(LockedSource,i,j) >> 0)  & 0xFF)) / 255.0f;
				  
				Normal.x = ((Normal.x - 0.5f) * 2.0f);
				Normal.y = ((Normal.y - 0.5f) * 2.0f);
				Normal.z = ((Normal.z - 0.5f) * 2.0f);
				
				SetMapData(LockedDest,i,j,VectorToQ8W8V8U8(Normal));
			}
		}

		DestTex->UnlockRect();
		SourceTex->UnlockRect();

	}

	SAFE_RELEASE(Source);

	m_Texture = NormalMap;

	return(true);
}

//______________________________________
//
//	GrayScaleAlpha
//
//______________________________________

void GrayScaleAlpha(LPDIRECT3DTEXTURE9 Texture)
{
	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT  Locked;
	int				Level,x,y;
	unsigned long	*Bits,Width,Height;
	float			R,G,B,A,Lum;
	HRESULT			Hr;

	if(Texture)
	{
		Texture->GetLevelDesc(0,&Desc);

		if(Desc.Format != D3DFMT_A8R8G8B8)
		{
			return;
		}

		for(Level=0; Level < Texture->GetLevelCount(); Level++)
		{
			Texture->GetLevelDesc(Level,&Desc);

			Width  = Desc.Width;
			Height = Desc.Height;

			Hr = Texture->LockRect(Level,&Locked,NULL,0);

			if(FAILED(Hr))
			{
				return;
			}

			for(y=0; y < Height; y++)
			{
				for(x=0; x < Width; x++)
				{
					Bits = (unsigned long*)((unsigned char *)Locked.pBits + (y * Locked.Pitch));
					Bits += x;

					B = ((float)( *Bits & 0xFF))        / 255.0f;
					G = ((float)((*Bits >> 8) & 0xFF))  / 255.0f;
					R = ((float)((*Bits >> 16) & 0xFF)) / 255.0f;
					A = ((float)((*Bits >> 24) & 0xFF)) / 255.0f;
					 
					Lum = (((R * 0.3f) + (G * 0.59f) + (B * 0.11f)) * 255.0f);

					*Bits &= 0x00FFFFFF;

					*Bits |= (((int)Lum & 0xFF) << 24);

				}
			}

			Texture->UnlockRect(Level);
		}

	}

}
