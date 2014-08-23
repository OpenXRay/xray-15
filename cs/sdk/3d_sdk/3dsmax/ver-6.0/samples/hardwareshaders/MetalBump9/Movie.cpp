//_____________________________________________________________________________
//
//	File: Movie.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include "Movie.h"
#include "Utility.h"
#include "Resource.h"

//_____________________________________________________________________________
//
//	Functions	
//_____________________________________________________________________________

extern TCHAR *GetString(int id);

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

Movie::Movie()
{
	MovieInit();
}

//_____________________________________
//
//	MovieInit 
//
//_____________________________________

void Movie::MovieInit()
{
	m_Handle			= NULL;
	m_Frame				= 0;
	m_NumFrames			= 0;
	m_Loop				= 0;
	m_Fps				= 0; 
	m_Length			= 0;			
	m_LinePitch			= 0;        
	m_Input				= NULL;			
	m_Output			= NULL;			
	m_AVIHandle			= 0;		
	m_Decompressor		= 0;     
	m_ScrFmt			= NULL;
	m_TargetFmt			= NULL;
	m_Target			= NULL;
	m_End				= 0.0f;
	m_CurFrame			= -1;


}

//______________________________________
//
//	Default destructor 
//
//______________________________________

Movie::~Movie()
{

	if(m_Decompressor)	
	{
		ICDecompressEnd(m_Decompressor);
		ICClose(m_Decompressor);
	}

	if(m_ScrFmt)
	{
		free(m_ScrFmt);
	}

	if(m_TargetFmt)
	{
		free(m_TargetFmt);
	}

	if(m_Input)
	{
		free(m_Input);
	}

	if(m_Output)
	{
		free(m_Output);
	}

	if(m_AVIHandle)
	{
		AVIStreamRelease(m_AVIHandle);
	}

}

//______________________________________
//
//	SetTarget 
//
//______________________________________

void Movie::SetTarget(IDirect3DBaseTexture9 *Img)
{
	m_Target = Img;
}

//______________________________________
//
//	Load 
//
//______________________________________

bool Movie::Load(TSTR &FileName, int &Width, int &Height, int &Bpp, int DestBpp)
{
	long		FmtLength;
	HRESULT		Hr;
	TCHAR error[256];
	sprintf(error,"%s",GetString(IDS_ERROR));

	Hr = AVIStreamOpenFromFile(&m_AVIHandle,FileName.data(),streamtypeVIDEO,0,OF_SHARE_COMPAT | OF_READ,NULL);

	if(FAILED(Hr)) 
	{
		return(false);
	}
	else
	{
		AVIStreamFormatSize(m_AVIHandle,0,&FmtLength);
		m_ScrFmt	= (LPBITMAPINFOHEADER)malloc(FmtLength);
		m_TargetFmt = (LPBITMAPV4HEADER)malloc(max(FmtLength,sizeof(BITMAPV4HEADER)));
		ZeroMemory(m_TargetFmt,sizeof(BITMAPV4HEADER));

		AVIStreamReadFormat(m_AVIHandle,0,m_ScrFmt,&FmtLength);

		m_NumFrames = AVIStreamLength(m_AVIHandle);
		AVIStreamInfo(m_AVIHandle,&m_AVIInfo,sizeof(AVISTREAMINFO));

		Width  = m_ScrFmt->biWidth;
		Height = m_ScrFmt->biHeight;
		Bpp	   = m_ScrFmt->biBitCount;

		if(Bpp != 24)
		{
			TSTR	Str;
			Str = TSTR(GetString(IDS_LOAD_FILES)) + m_Name + TSTR(GetString(IDS_UNSUPPORTED));
			MessageBox(NULL,Str,error,MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);

			if(m_ScrFmt)
			{
				free(m_ScrFmt);
			}

			if(m_TargetFmt)
			{
				free(m_TargetFmt);
			}

			return(false);
		}


		memcpy(m_TargetFmt,m_ScrFmt,FmtLength);

		m_TargetFmt->bV4Size			= max(FmtLength,sizeof(BITMAPV4HEADER));
		m_TargetFmt->bV4BitCount		= DestBpp;
		m_TargetFmt->bV4V4Compression	= BI_RGB;
		m_TargetFmt->bV4ClrUsed			= 0;

		if(DestBpp == 16 || DestBpp == 32)
		{
			 m_TargetFmt->bV4V4Compression = BI_BITFIELDS;
		}

		if(DestBpp == 32)
		{
			m_TargetFmt->bV4AlphaMask	= 0xFF000000;
			m_TargetFmt->bV4RedMask		= 0x00FF0000;
			m_TargetFmt->bV4GreenMask	= 0x0000FF00;
			m_TargetFmt->bV4BlueMask	= 0x000000FF;     
		}
		else if(DestBpp == 24)
		{
			m_TargetFmt->bV4AlphaMask	= 0x00000000;
			m_TargetFmt->bV4RedMask		= 0x00FF0000;
			m_TargetFmt->bV4GreenMask	= 0x0000FF00;
			m_TargetFmt->bV4BlueMask	= 0x000000FF;     
		}
		else if(DestBpp == 16)
		{
			m_TargetFmt->bV4AlphaMask	= 0x0000F000;
			m_TargetFmt->bV4RedMask		= 0x00000F00;
			m_TargetFmt->bV4GreenMask	= 0x000000F0;
			m_TargetFmt->bV4BlueMask	= 0x0000000F;  
		}
		else
		{
			m_TargetFmt->bV4RedMask		= 0x00f800;
			m_TargetFmt->bV4GreenMask	= 0x0007e0;
			m_TargetFmt->bV4BlueMask	= 0x00001f;
		}

		m_TargetFmt->bV4SizeImage = ((m_TargetFmt->bV4Width + 3) & 0xFFFFFFFC) *
									  m_TargetFmt->bV4Height * (m_TargetFmt->bV4BitCount >> 3);


		m_Length = m_ScrFmt->biWidth * m_ScrFmt->biHeight *
				   (m_ScrFmt->biBitCount >> 3);

		if(m_AVIInfo.dwSuggestedBufferSize)
		{
			m_Length = (long)m_AVIInfo.dwSuggestedBufferSize;
		}

		m_Decompressor = ICDecompressOpen(ICTYPE_VIDEO,
										  m_AVIInfo.fccHandler,
										  m_ScrFmt,
										  (LPBITMAPINFOHEADER)m_TargetFmt);

		m_Input  = (unsigned char *)calloc(m_Length,1);
		m_Output = (unsigned char *)calloc(m_TargetFmt->bV4SizeImage,1);

		m_LinePitch = m_TargetFmt->bV4Width * (m_TargetFmt->bV4BitCount >> 3);

		ICDecompressBegin(m_Decompressor,m_ScrFmt,
						 (LPBITMAPINFOHEADER)m_TargetFmt);

		m_Fps = m_AVIInfo.dwRate / m_AVIInfo.dwScale;
		m_End = (1.0f / m_Fps) * m_NumFrames;

		Evaluate(0.0f);	
	}

	return(true);

}

//______________________________________
//
//	Evaluate
//
//______________________________________

void Movie::Evaluate(float Time)
{
	unsigned char			*Src,*Dest;
	int						x,y;
	D3DLOCKED_RECT			Rect;
	IDirect3DTexture9		*Tex;
	HRESULT					Hr;
	D3DSURFACE_DESC			Desc;

	if(m_Target != NULL)
	{
		m_Frame = fmod(Time,m_NumFrames);

		if(m_Frame > m_NumFrames)
		{
			m_Frame = 0;
		}

		if(m_CurFrame == m_Frame)
		{
			return;
		}

		m_CurFrame = m_Frame;

		Tex = (LPDIRECT3DTEXTURE9)m_Target;
		Tex->GetLevelDesc(0,&Desc);

		if(m_Frame < m_NumFrames) 
		{
			if(AVIStreamRead(m_AVIHandle,m_Frame,AVISTREAMREAD_CONVENIENT,m_Input,m_Length,NULL,NULL))
			{
				return;
			}

			if(ICDecompress(m_Decompressor,ICDECOMPRESS_HURRYUP,m_ScrFmt,m_Input, 
							(LPBITMAPINFOHEADER)m_TargetFmt,m_Output) == ICERR_OK)
			{	

				if(m_ScrFmt->biBitCount == 24)
				{
					Hr = Tex->LockRect(0,&Rect,NULL,0);

					if(Hr == S_OK)
					{
						Dest = (unsigned char *)Rect.pBits;
						Src  = m_Output + m_LinePitch * (m_TargetFmt->bV4Height-1);

						if(Src && Dest)
						{
							for(y=0; y < Desc.Height; y++) 
							{
								for(x=0; x < Desc.Width; x++)
								{
									Dest[x * 4 + 0] = Src[x * 3 + 0];
									Dest[x * 4 + 1] = Src[x * 3 + 1];
									Dest[x * 4 + 2] = Src[x * 3 + 2];
									Dest[x * 4 + 3] = 1;

								}
 
								Dest += Rect.Pitch;
								Src  -= m_LinePitch;
							}
						}

						Tex->UnlockRect(0);
					}

				}
				else if(m_ScrFmt->biBitCount == 32)
				{
					Hr = Tex->LockRect(0,&Rect,NULL,0);

					if(Hr == S_OK)
					{
						Dest = (unsigned char *)Rect.pBits;
						Src  = m_Output + m_LinePitch * (m_TargetFmt->bV4Height-1);

						if(Src && Dest)
						{
							for(y=0; y < Desc.Height; y++) 
							{
								memcpy(Dest,Src,m_LinePitch);
								Dest += Rect.Pitch;
								Src  -= m_LinePitch;
							}
						}

						Tex->UnlockRect(0);
					}

				}
	
			}

		}

	}


}