//_____________________________________________________________________________
//
//	File: Utility.h
//	
//
//_____________________________________________________________________________


#ifndef UTILITY_H
#define UTILITY_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "Max.h"
#include <d3dx9.h>
//#include <d3d9.h>

#pragma warning(disable:4530)
#include <vector>

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

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);	(p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p)=NULL; } }
#define SMALL_FLOAT				(1e-5)

#define DEG_RAD(A)				(((float)(A)) *  0.01745328f)
#define RAD_DEG(A)				(((float)(A)) *  57.2957823f)
#define MAX3(A,B,C)				(A > B && A > C  ? A : (B > C ? B : C))

#define MAX_TMUS				4

//_____________________________________________________________________________
//
//	Types
//
//_____________________________________________________________________________

//_____________________________________
//
//	INodeVector
//
//_____________________________________

typedef std::vector<INode *> INodeList;

//_____________________________________
//
//	Vert3
//
//_____________________________________

typedef struct
{
    Point3			m_Pos;
	Point3			m_Normal;
    Point3			m_S;
    Point3			m_T;
    Point3			m_SxT;
    Point2			m_UV[MAX_TMUS];
	unsigned long	m_Sg;

}Vert3;

//_____________________________________
//
//	Face3
//
//_____________________________________

typedef struct
{
    int		m_Num[3];

}Face3;

//_____________________________________
//
//	ShaderType
//
//_____________________________________

typedef enum 
{
	SHADER_VERTEXSHADER  = 0,
	SHADER_PIXELSHADER   = 1

}ShaderType;

//_____________________________________
//
//	ShaderLightGroup 
//
//_____________________________________

typedef enum
{
	LG_DEFAULT,
	LG_SPEC,
	LG_NORMAL,
	LG_NORMALSPEC,
	LG_MAX

}LightGroup;

//_____________________________________
//
//	RMatChannel	
//	(Update MatChannelUse if changed) 
//
//_____________________________________

typedef enum
{
	CHANNEL_DIFFUSE,
	CHANNEL_NORMAL,
	CHANNEL_SPECULAR,
	CHANNEL_DETAIL,
	CHANNEL_MASK,
	CHANNEL_REFLECTION,
	CHANNEL_BUMP,
	CHANNEL_MAX


}RMatChannel;


//_____________________________________
//
//	RMatChannel
//
//_____________________________________

typedef enum
{
	DIFFUSE_UV			= 0,
	NORMAL_UV			= 1,
	DETAIL_UV			= 2,
	FX_EMISSIVE_UV		= 3

}UVChannels;

//_____________________________________
//
//	TextureType
//
//_____________________________________

typedef enum
{
	TEX_UNDEFINED,
	TEX_STANDARD,
	TEX_NORMAL,
	TEX_BUMP,
	TEX_CUBE

}TextureType;

//_____________________________________
//
//	MatShaderInfo
//
//_____________________________________

typedef struct
{
	int				m_ChannelUse;		// ChannelUse
	TSTR			m_VertexFile;		// Vertex shader file name
	unsigned long	m_VertexRes;		// Vertex shader resource name
	LPDIRECT3DVERTEXSHADER9	m_VertexHandle;			// Vertex shader handle
	TSTR			m_PixelFile;		// Pixel shader file name	
	unsigned long	m_PixelRes;			// Pixel shader resource name
	LPDIRECT3DPIXELSHADER9	m_PixelHandle;		// Pixel shader handle
	TSTR			m_AlphaPixelFile;		// Pixel shader file name	
	unsigned long	m_AlphaPixelRes;		// Pixel shader resource name
	LPDIRECT3DPIXELSHADER9	m_AlphaPixelHandle;		// Pixel shader handle
	int				m_BestChannelUse;

}MatShaderInfo;


//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________

//_____________________________________
//
//	VNormal
//
//_____________________________________


class VNormal 
{
	public:

		Point3	m_Normal;
		Point3	m_S;
		Point3	m_T;
		Point3	m_SxT;
		DWORD	m_Smooth;
		VNormal *m_Next;
		BOOL	m_Init;

		VNormal()				   
		{	
			m_Smooth = 0; 
			m_Next   = NULL; 
			m_Init   = false; 

			m_Normal.Set(0.0f,0.0f,0.0f);
			m_S.Set(0.0f,0.0f,0.0f);
			m_T.Set(0.0f,0.0f,0.0f);
			m_SxT.Set(0.0f,0.0f,0.0f);
		}

		
		VNormal(Point3 &N,unsigned long Smooth, Point3 &S, Point3 &T) 
		{	
			m_Next	 = NULL;
			m_Init	 = true;
			m_Normal = N;
			m_Smooth = Smooth;
			m_S	  	 = S;
			m_T		 = T;
		}

		~VNormal() 
		{
			delete m_Next;
		}

		void Clear()
		{
			m_Smooth = 0; 
			m_Next   = NULL; 
			m_Init   = false; 

			m_Normal.Set(0.0f,0.0f,0.0f);
			m_S.Set(0.0f,0.0f,0.0f);
			m_T.Set(0.0f,0.0f,0.0f);
			m_SxT.Set(0.0f,0.0f,0.0f);

		}

		void	AddNormal(Point3 &N, unsigned long Smooth, Point3 &S, Point3 &T);
		Point3& GetNormal(unsigned long Smooth, Point3 &S, Point3 &T, Point3 &SxT);
		void	Normalize();

};


//_____________________________________________________________________________
//
//	Functions
//
//_____________________________________________________________________________

//_____________________________________
//
//	VectorToRGBA
//
//_____________________________________

inline D3DCOLOR VectorToRGBA(const D3DXVECTOR3 *V, float Height = 1.0f)
{
	unsigned long R,G,B,A;

    R = (unsigned long)((V->x + 1.0f ) * 127.5f);
    G = (unsigned long)((V->y + 1.0f ) * 127.5f);
    B = (unsigned long)((V->z + 1.0f ) * 127.5f);
    A = (unsigned long) (255.0f * Height);

    return((A << 24L) + (R << 16L) + (G << 8L) + (B << 0L));
}


//_____________________________________
//
// 	VectorToQ8W8V8U8 
//
//_____________________________________

inline unsigned long VectorToQ8W8V8U8(const D3DXVECTOR3 &Vector)
{
	D3DXVECTOR3 Scaled;
    signed char	Red,Green,Blue,Alpha;

	Scaled = Vector * 127.5f;
    Red    = (signed char)Scaled.x;
    Green  = (signed char)Scaled.y;
	Blue   = (signed char)Scaled.z;
	Alpha  = 0.0f;

    return (((unsigned long)(unsigned char)Alpha << 24 ) | 
			((unsigned long)(unsigned char)Blue  << 16 ) | 
			((unsigned long)(unsigned char)Green << 8  ) | 
			((unsigned long)(unsigned char)Red   << 0));
}


//_____________________________________
//
//	FloatToRGBA
//
//_____________________________________

inline D3DCOLOR FloatToRGBA(float V)
{
	unsigned long R,G,B,A;

    R = (unsigned long)(V * 255.5f);
    G = (unsigned long)(V * 255.5f);
    B = (unsigned long)(V * 255.5f);
    A = (unsigned long)(V * 255.5f);

    return((A << 24L) + (R << 16L) + (G << 8L) + (B << 0L));
}

//_____________________________________
//
//	Point3Normalize
//
//_____________________________________

inline void Point3Normalize(Point3 &Normal)
{	
	float	Dist;

	Dist = Normal.Length();

	if(Dist)
	{
		Dist = 1.0f / Dist;
	
		Normal.x *= Dist;
		Normal.y *= Dist;
		Normal.z *= Dist;
	}
	else
	{
		Normal.x = 0.0f;
		Normal.y = 0.0f;
		Normal.z = 0.0f;
	}
	
}

//_____________________________________
//
//	Rotate2DPoint
//
//_____________________________________

inline void Rotate2DPoint(float &X, float &Y, float A)
{	
	float NewX,NewY,TempX,TempY;

	TempX = X;
	TempY = Y;

	NewX = TempX * (float)cos(-A) - TempY * (float)sin(-A);
	NewY = TempX * (float)sin(-A) + TempY * (float)cos(-A);

	X = NewX;
	Y = NewY;

}


//_____________________________________
//
//	TMNegParity
//
//_____________________________________

inline bool TMNegParity(Matrix3 &Mat)
{
	return (DotProd(CrossProd(Mat.GetRow(0),Mat.GetRow(1)),Mat.GetRow(2)) < 0.0) ? 1 : 0;
}

//_____________________________________
//
//	LargestPower2 
//
//_____________________________________

inline int LargestPower2(int X)
{
	int i;

	for(i=31; i > 0; i--)
	{
		if(X & (1 << i))
		{
			return(1 << i);
		}
	}

	return(0);

}


extern TCHAR			*FindFile(TCHAR *File);

extern HRESULT CreateShaderVertex(IDirect3DDevice9 *Device, TSTR File,LPDIRECT3DVERTEXDECLARATION9 * vdecl, LPDIRECT3DVERTEXSHADER9 *Handle);
extern HRESULT CreateShaderPixel(IDirect3DDevice9 *Device, TSTR File, LPDIRECT3DPIXELSHADER9 *Handle);

bool					GetFileResource(const char *Name, const char *Type, 
										void **Data,unsigned long &Size);

HRESULT					CreateShaderResource(IDirect3DDevice9 *Device, unsigned long Res, const unsigned long* Decl,
											 ShaderType Type,  unsigned long* Handle);

HRESULT CreateVertexShaderResource(IDirect3DDevice9 *Device, unsigned long Res, LPDIRECT3DVERTEXDECLARATION9 * vdecl,
							   LPDIRECT3DVERTEXSHADER9 *Handle);

HRESULT CreatePixelShaderResource(IDirect3DDevice9 *Device, unsigned long Res,  LPDIRECT3DPIXELSHADER9 *Handle);



TriObject*				GetTriObjectFromNode(INode *Node, TimeValue T, bool &Delete);
INode*					FindNodeRef(ReferenceTarget *Rt);

#endif


