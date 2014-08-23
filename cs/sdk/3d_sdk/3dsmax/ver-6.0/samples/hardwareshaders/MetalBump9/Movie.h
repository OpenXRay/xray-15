//_____________________________________________________________________________
//
//	File: Movie.h
//	
//
//_____________________________________________________________________________


#ifndef MOVIE_H
#define MOVIE_H

#if _MSC_VER >= 1000
#pragma once
#endif 

//__________________________________________________________________________________
//
//	Include
//__________________________________________________________________________________

#include "Max.h"
#include <Windows.h>
#include <Vfw.h>
#include "IDX9VertexShader.h"


//_____________________________________________________________________________
//
//	Forward declare
//
//_____________________________________________________________________________


//__________________________________________________________________________________
//
//	Defines
//
//__________________________________________________________________________________

//__________________________________________________________________________________
//
//	Types
//
//__________________________________________________________________________________

//__________________________________________________________________________________
//
//	Class definitions
//
//__________________________________________________________________________________


//_____________________________________
//
//	Movie class
//
//_____________________________________

class Movie
{
	public:

		TSTR						m_Name;
		int							m_Handle;
		long						m_Frame;
		long						m_NumFrames;
		bool						m_Loop;
		unsigned long				m_Fps; 
		long						m_Length;			
		long						m_LinePitch;        
		unsigned char				*m_Input;			
		unsigned char				*m_Output;			
		PAVISTREAM					m_AVIHandle;		
		AVISTREAMINFO				m_AVIInfo;		
		HIC							m_Decompressor;     
		LPBITMAPINFOHEADER			m_ScrFmt;			
		LPBITMAPV4HEADER			m_TargetFmt;		
		IDirect3DBaseTexture9		*m_Target;				
		float						m_End;
		int							m_CurFrame;

	public:
		//
		//	Constructors
		//
		Movie();
		//
		//	Destructors
		//
		virtual			~Movie();
		//
		//	Load
		//
		bool			Load(TSTR &FileName, int &Width, int &Height, int &Bpp, int DestBpp);
		void			SetTarget(IDirect3DBaseTexture9 *Img);			
		//
		//
		//
		virtual	void	Evaluate(float Time);

	private:
		
		void			MovieInit();

};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif


