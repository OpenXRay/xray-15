//_____________________________________________________________________________
//
//	File: RenderMesh.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include "RenderMesh.h"
#include "Utility.h"
#include "MeshNormalSpec.h"

//_____________________________________________________________________________
//
//	Globals	
//
//_____________________________________________________________________________
/*
DWORD gVertexDecl[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),		// Position
	D3DVSD_REG(1, D3DVSDT_FLOAT3),		// Normal
	D3DVSD_REG(2, D3DVSDT_FLOAT3),		// S
	D3DVSD_REG(3, D3DVSDT_FLOAT2),		// Texture1
	D3DVSD_REG(4, D3DVSDT_FLOAT2),		// Texture2
	D3DVSD_REG(5, D3DVSDT_FLOAT2),		// Texture3
	D3DVSD_REG(6, D3DVSDT_FLOAT2),		// Texture4
	D3DVSD_END()
};
*/
LPDIRECT3DVERTEXDECLARATION9 m_VertexDec;


int	gVIndex[3];

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

RenderMesh::RenderMesh()
{
	m_VB		= NULL;
	m_IB		= NULL;
	m_NumVert	= 0;
	m_NumFace	= 0;
	m_Valid		= false;
	m_MapChannels.SetCount(4);

}


//______________________________________
//
//	Default destructor 
//
//______________________________________

RenderMesh::~RenderMesh()
{
	SAFE_RELEASE(m_VB);
	SAFE_RELEASE(m_IB);
}

//______________________________________
//
//	Destroy 
//
//______________________________________

void RenderMesh::Destroy()
{
	SAFE_RELEASE(m_VB);
	SAFE_RELEASE(m_IB);

	m_NumVert	= 0;
	m_NumFace	= 0;
	m_Valid		= false;

}


//_____________________________________
//
//	Evaluate 
//
//_____________________________________

bool RenderMesh::Evaluate(IDirect3DDevice9 *Device, Mesh *aMesh, int MatIndex, bool NegScale)
{
	int						i,j;
    unsigned short			*Indice;
	SHADERVERTEX			*Vertices;
	Tab<Vert3>				Verts;
	Tab<Face3>				Faces;
	float					U,V;
	int						A,B,C;

	if(!Device || !aMesh)
	{
		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);

		m_Valid	= false;

		return(false);
	}	

	if(!m_Valid)
	{
		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);

		ConvertFaces(aMesh,MatIndex,Verts,Faces,NegScale);

		m_NumVert = Verts.Count();
		m_NumFace = Faces.Count();

		if(m_NumVert == 0 || m_NumFace == 0)
		{
			m_Valid	= true;

			return(true);
		}

		if(FAILED(Device->CreateVertexBuffer(m_NumVert * sizeof(SHADERVERTEX),
											 D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0, 
											 D3DPOOL_DEFAULT,&m_VB,NULL)))
		{
			goto FAILED;
		}


		if(FAILED(Device->CreateIndexBuffer(m_NumFace * 3 * sizeof(unsigned short),
											D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
											D3DPOOL_DEFAULT,&m_IB,NULL)))
		{
			goto FAILED;
		}

		if(FAILED(m_IB->Lock(0,m_NumFace * 3 * sizeof(unsigned short), 
							 (void**)&Indice,D3DLOCK_DISCARD)))
		{									   
			goto FAILED;
		}
    
		if(FAILED(m_VB->Lock(0,0,(void**)&Vertices,D3DLOCK_DISCARD)))
		{
			goto FAILED;
		}

		for(i=0; i < m_NumVert; i++)
		{
			Vertices[i].m_Pos	 = Verts[i].m_Pos;
			Vertices[i].m_Normal = Verts[i].m_Normal;
			Vertices[i].m_S		 = Verts[i].m_S;

			for(j=0; j < MAX_TMUS; j++)
			{
				U = Verts[i].m_UV[j].x;
				V = Verts[i].m_UV[j].y;
				U = -U;
				Rotate2DPoint(U,V,DEG_RAD(180.0f));

				Vertices[i].m_UV[j].x = U;
				Vertices[i].m_UV[j].y = V;

			}
		}

		for(i=0; i < m_NumFace; i++)
		{
			A = Faces[i].m_Num[0];
			B = Faces[i].m_Num[1];
			C = Faces[i].m_Num[2];

			Indice[i * 3 + 0] = A;
			Indice[i * 3 + 1] = B;
			Indice[i * 3 + 2] = C;
		}

		m_VB->Unlock();
		m_IB->Unlock();

		m_Valid = true;

		return(true);

FAILED:

		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);

		m_Valid = false;
		return(false);
	
	}


	return(true);

}


//_____________________________________
//
//	Render 
//
//_____________________________________

bool RenderMesh::Render(IDirect3DDevice9 *Device)
{
	if(m_Valid && Device && m_VB && m_IB)
	{
		Device->SetVertexDeclaration(m_VertexDec);
		Device->SetStreamSource(0,m_VB,0,sizeof(SHADERVERTEX));
		Device->SetIndices(m_IB);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,m_NumVert,0,m_NumFace);
	
		return(true);
	}

	return(false);
}


//_____________________________________
//
//	ConvertFaces
//
//_____________________________________

void RenderMesh::ConvertFaces(Mesh *Mesh, int MatIndex, Tab<Vert3> &Verts, Tab<Face3> &Faces, bool NegScale)
{
    Face3			TmpFace;
    Vert3			TmpVert;
	BitArray		Written;
	int				i,j,k,NumFace;
	int				NumUV,UVCount,Index;
	int				NumVert,Count,VIndex;
	Face			*aFace;
	Tab<BasisVert>	FNormals;
	Tab<VNormal>	Normals;
	UVVert			*UVVert;
	TVFace			*UVFace;
	Point3			S,T,SxT;
	unsigned long	Sg;

	bool useMeshNorms = false;


	if(NegScale)
	{
		gVIndex[0] = 2;
		gVIndex[1] = 1;
		gVIndex[2] = 0;
	}
	else
	{
		gVIndex[0] = 0;
		gVIndex[1] = 1;
		gVIndex[2] = 2;
	}

	// Do we have an EditNormal modifier present - if so we use those normals instead.
	// We only use this if they have been applied on a face with smoothing groups, otherwise
	// it messes up the tangent space calculation.  Probably not the most obtmized route, but it
	// works...

	MeshNormalSpec * meshNorm = Mesh->GetSpecifiedNormals();
	if(meshNorm && meshNorm->GetNumNormals())
		useMeshNorms = true;

	NumFace = 0;

	for(i=0; i < Mesh->getNumFaces(); i++) 
	{
		if(!Mesh->faces[i].Hidden())
		{
			Index = Mesh->getFaceMtlIndex(i) + 1;

			if(Index == MatIndex || MatIndex == 0)
			{
				NumFace++;
			}
		}

	}

	NumVert = Mesh->getNumVerts();
    Verts.SetCount(NumVert);

    Faces.SetCount(NumFace);

	if(NumVert == 0 || NumFace == 0)
	{
		return;
	}

	ComputeVertexNormals(Mesh,FNormals,Normals,NegScale);

    Written.SetSize(Mesh->getNumVerts());
    Written.ClearAll();

	NumUV = Mesh->getNumMaps();	

	if(NumUV)
	{	
		Count = 0;

		if(NumUV > MAX_TMUS + 1)
		{
			NumUV = MAX_TMUS + 1;
		}

		for(i=0; i < Mesh->getNumFaces(); i++) 
		{
			aFace = &Mesh->faces[i];

			TmpFace.m_Num[0] = aFace->v[gVIndex[0]];
			TmpFace.m_Num[1] = aFace->v[gVIndex[1]];
			TmpFace.m_Num[2] = aFace->v[gVIndex[2]];


			Sg = aFace->smGroup;

			for(j=0; j < 3; j++) 
			{
				VIndex			 = aFace->v[gVIndex[j]];
				TmpVert.m_Pos	 = Mesh->verts[VIndex];

				if(Sg)
				{
					if(useMeshNorms)
					{
						int normID = meshNorm->Face(i).GetNormalID(gVIndex[j]);
						TmpVert.m_Normal = meshNorm->Normal(normID).Normalize();
						Normals[VIndex].GetNormal(Sg,S,T,SxT);
					}
					else
					        TmpVert.m_Normal = Normals[VIndex].GetNormal(Sg,S,T,SxT);
					
					TmpVert.m_S		 = S;
					TmpVert.m_T		 = T;
					TmpVert.m_SxT	 = SxT;

				}
				else
				{
					TmpVert.m_Normal = FNormals[i].m_Normal;
					TmpVert.m_S		 = FNormals[i].m_S;
					TmpVert.m_T		 = FNormals[i].m_T;
					TmpVert.m_SxT	 = FNormals[i].m_SxT;
				}

				UVCount		 = 0;
				TmpVert.m_Sg = Sg;

				for(k=0;k<m_MapChannels.Count();k++)
				{	
					int index = m_MapChannels[k];

					if(Mesh->getNumMapVerts(index))
					{
						UVVert = Mesh->mapVerts(index);
						UVFace = Mesh->mapFaces(index);

						TmpVert.m_UV[k].x = UVVert[UVFace[i].t[gVIndex[j]]].x;
						TmpVert.m_UV[k].y = UVVert[UVFace[i].t[gVIndex[j]]].y;

	
					}
					else
					{
						TmpVert.m_UV[k].x = 0.0f;
						TmpVert.m_UV[k].y = 0.0f;
					}
				}
				
		
				if(Written[VIndex]) 
				{
					if((Sg == 0) || 
					   (Verts[VIndex].m_Sg != TmpVert.m_Sg) ||	
					   (!UVVertEqual(Verts[VIndex].m_UV[0],TmpVert.m_UV[0]))) 
					{
						TmpFace.m_Num[j] = Verts.Count();
						Verts.Append(1,&TmpVert,10);
					}
				} 
				else 
				{
					Verts[VIndex] = TmpVert;
					Written.Set(VIndex);
				}

			}

			if(!Mesh->faces[i].Hidden())
			{
				Index = Mesh->getFaceMtlIndex(i) + 1;

				if(Index == MatIndex || MatIndex == 0)
				{
					Faces[Count++] = TmpFace;
				}

			}

		}

	}
	else
	{
		for(i=0; i < Mesh->getNumFaces(); i++) 
		{
			aFace = &Mesh->faces[i];

			Faces[i].m_Num[0] = aFace->v[gVIndex[0]];
			Faces[i].m_Num[1] = aFace->v[gVIndex[1]];
			Faces[i].m_Num[2] = aFace->v[gVIndex[2]];

			for(j=0; j < 3; j++) 
			{
				VIndex					= aFace->v[gVIndex[j]];
				Verts[VIndex].m_Pos		= Mesh->verts[VIndex];
				Verts[VIndex].m_Normal	= Normals[VIndex].GetNormal(aFace->smGroup,S,T,SxT);
				Verts[VIndex].m_S		= Point3(0.0f,0.0f,0.0f);
				Verts[VIndex].m_T		= Point3(0.0f,0.0f,0.0f);
				Verts[VIndex].m_SxT		= Point3(0.0f,0.0f,0.0f);

				for(k=0; k < MAX_TMUS; k++)
				{
					Verts[VIndex].m_UV[k].x = 0.0f;
					Verts[VIndex].m_UV[k].y = 0.0f;
				}

			}

		}

	}
	Verts.Shrink();
	

}


//_____________________________________
//
//	ComputeVertexNormals
//
//_____________________________________

void RenderMesh::ComputeVertexNormals(Mesh *aMesh, Tab<BasisVert> &FNorms, Tab<VNormal> &VNorms, bool NegScale) 
{
	Face			*Face;	
	Point3			*Verts;
	Point3			Vt0,Vt1,Vt2;
	Point3			Normal;
	int				i,j,k,A,B,C;
	int				NumUV,NumVert,NumFace;
	UVVert			*UVVert;
	TVFace			*UVFace;
    Vert3			Vert[3];
	Point3			S,T;
	Point3			Edge01,Edge02;
	Point3			Cp;
	float			U0,V0,U1,V1,U2,V2;
	int				UVCount;
	unsigned long	Sg;

	NumUV   = aMesh->getNumMaps();	
	NumVert	= aMesh->getNumVerts();
	NumFace = aMesh->getNumFaces(); 
	Face	= aMesh->faces;	
	Verts	= aMesh->verts;

	VNorms.SetCount(NumVert);
	FNorms.SetCount(NumFace);

	if(NumUV > MAX_TMUS + 1)
	{
		NumUV = MAX_TMUS + 1;
	}

	for(i=0; i < NumVert; i++) 
	{
		VNorms[i].Clear();
	}

	for(i=0; i < NumFace; i++, Face++) 
	{
		A = Face->v[gVIndex[0]];
		B = Face->v[gVIndex[1]];
		C = Face->v[gVIndex[2]];

		Vt0 = Verts[A];
		Vt1 = Verts[B];
		Vt2 = Verts[C];

		Normal = (Vt1 - Vt0) ^ (Vt2 - Vt0);

		Point3Normalize(Normal);

		for(j=0; j < 3; j++) 
		{
			UVCount = 0;

			for(k=0;k<m_MapChannels.Count();k++)
			{	
				int index = m_MapChannels[k];
				if(aMesh->getNumMapVerts(index))
				{
					UVVert = aMesh->mapVerts(index);
					UVFace = aMesh->mapFaces(index);

					Vert[j].m_UV[k].x = UVVert[UVFace[i].t[gVIndex[j]]].x;
					Vert[j].m_UV[k].y = UVVert[UVFace[i].t[gVIndex[j]]].y;


				}
				else
				{
					Vert[j].m_UV[k].x = 0.0f;
					Vert[j].m_UV[k].y = 0.0f;

				}
			}

		}

		S.Set(0.0f,0.0f,0.0f);
		T.Set(0.0f,0.0f,0.0f);

		U0 = -Vert[0].m_UV[NORMAL_UV].x;
		V0 = Vert[0].m_UV[NORMAL_UV].y;

		Rotate2DPoint(U0,V0,DEG_RAD(180.0f));

		U1 = -Vert[1].m_UV[NORMAL_UV].x;
		V1 = Vert[1].m_UV[NORMAL_UV].y;

		Rotate2DPoint(U1,V1,DEG_RAD(180.0f));

		U2 = -Vert[2].m_UV[NORMAL_UV].x;
		V2 = Vert[2].m_UV[NORMAL_UV].y;

		Rotate2DPoint(U2,V2,DEG_RAD(180.0f));

		// x, s, t
		Edge01 = Point3(Vt1.x - Vt0.x, 
						U1 - U0, 
						V1 - V0);

		Edge02 = Point3(Vt2.x - Vt0.x, 
						U2 - U0, 
						V2 - V0);


		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.x = -Cp.y / Cp.x;
			T.x = -Cp.z / Cp.x;
		}

		// y, s, t
		Edge01 = Point3(Vt1.y - Vt0.y, 
					    U1 - U0, 
					    V1 - V0);

		Edge02 = Point3(Vt2.y - Vt0.y, 
				   	    U2 - U0, 
						V2 - V0);

		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.y = -Cp.y / Cp.x;
			T.y = -Cp.z / Cp.x;
		}

		// z, s, t
		Edge01 = Point3(Vt1.z - Vt0.z, 
					    U1 - U0, 
					    V1 - V0);

		Edge02 = Point3(Vt2.z - Vt0.z, 
					    U2 - U0, 
					    V2 - V0);

		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.z = -Cp.y / Cp.x;
			T.z = -Cp.z / Cp.x;
		}

		Point3Normalize(S);
		Point3Normalize(T);

		Sg = Face->smGroup;
		
		if(Sg)
		{
			VNorms[A].AddNormal(Normal,Sg,S,T);
			VNorms[B].AddNormal(Normal,Sg,S,T);
			VNorms[C].AddNormal(Normal,Sg,S,T);
		}
		else
		{
			T = CrossProd(S,Normal);
			Point3Normalize(T);

			S = CrossProd(Normal,T);
			Point3Normalize(S);

			FNorms[i].m_Normal = Normal;
			FNorms[i].m_S	   = S;
			FNorms[i].m_T	   = T;
			FNorms[i].m_SxT	   = Normal;
		}

	}

	for(i=0; i < NumVert; i++) 
	{
		VNorms[i].Normalize();
	}

}

void RenderMesh::SetMappingData(int * map)
{
	for(int i=0;i<4;i++)
	{
		m_MapChannels[i] = map[i];
	}
	
}




