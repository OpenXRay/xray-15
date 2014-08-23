//_____________________________________________________________________________
//
//	File: RenderMesh.h
//	
//
//_____________________________________________________________________________


#ifndef RENDERMESH_H
#define RENDERMESH_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include "Max.h"
#include "IDX8PixelShader.h"

#ifndef	 UTILITY_H
#include "Utility.h"
#endif

//_____________________________________________________________________________
//
//	Forward declare
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Defines
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Types
//
//_____________________________________________________________________________


//_____________________________________
//
// 	ShaderVertex 
//
//_____________________________________

typedef struct 
{
    Point3		m_Pos;
	Point3		m_Normal;
	Point3		m_S;
	Point2		m_UV[MAX_TMUS];

}SHADERVERTEX;

//_____________________________________
//
//	BasisVert
//
//_____________________________________

typedef struct
{
	Point3	m_Normal;
	Point3	m_S;
	Point3	m_T;
	Point3	m_SxT;

}BasisVert;

//_____________________________________
//
// 	LineVertex 
//
//_____________________________________

typedef struct 
{
    D3DXVECTOR3		m_Pos;
    unsigned long	m_Color;      

}LINEVERTEX;


extern unsigned long gVertexDecl[];

//_____________________________________________________________________________
//
//	Class definitions
//_____________________________________________________________________________


//_____________________________________
//
// 	RenderMesh 
//
//_____________________________________

class RenderMesh
{
	public:

		LPDIRECT3DVERTEXBUFFER8		m_VB;
		LPDIRECT3DINDEXBUFFER8		m_IB;
		int							m_NumVert;
		int							m_NumFace;
		bool						m_Valid;
		Tab<int>					m_MapChannels;
		//
		//	Constructors
		//
		RenderMesh();
		//
		//	Destructors
		//
		~RenderMesh();
		//
		//	Methods
		//
		bool		Evaluate(IDirect3DDevice8 *Device, Mesh *aMesh, int MatIndex, bool NegScale);
		void		SetMappingData(int * map);
		bool		Render(IDirect3DDevice8 *Device);
		void		Invalidate();
		void		Destroy();
		void		ConvertFaces(Mesh *mesh, int MatIndex, Tab<Vert3>& verts, Tab<Face3>& faces, bool NegScale);
		void		ComputeVertexNormals(Mesh *aMesh, Tab<BasisVert> &FNormal, Tab<VNormal> &VNorms,bool NegScale);
		BOOL		UVVertEqual(Point2 tv0, Point2 tv1); 

};

//_____________________________________
//
// 	Invalidate
//
//_____________________________________

inline void RenderMesh::Invalidate()
{
	m_Valid = false;
}


//_____________________________________
//
//	UVVertEqual
//
//_____________________________________

inline BOOL RenderMesh::UVVertEqual(Point2 TV0, Point2 TV1) 
{
	if(TV0.x == TV1.x &&
	   TV0.y == TV1.y)
	{
		return(true);
	}

	return(false);	
}


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


