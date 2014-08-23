//_____________________________________________________________________________
//
//	File: MatMgr.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "MatMgr.h"
#include "ShaderMat.h"
#include "Utility.h"
#include "RenderMesh.h"
#include "resource.h"

//_____________________________________
//
//	Init Singleton 
//
//_____________________________________

MatMgr*		Singleton<MatMgr>::m_Singleton = 0;
MatMgr		TheMatMgr;
MatMgr*		gMatMgr;

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

MatMgr::MatMgr()
{
	m_NumMat = 1;
	m_Loaded = false;
}


//______________________________________
//
//	Default destructor 
//
//______________________________________

MatMgr::~MatMgr()
{
/*
	for(i=0; i < m_NumMat; i++)
	{
		m_MatInfos[]
	}

*/

}


//_____________________________________
//
//	FindVertexFile 
//
//_____________________________________

bool MatMgr::FindVertexFile(int Index)
{
	int i;

	for(i=0; i < m_NumMat; i++)
	{
		if(m_MatInfos[Index].m_VertexFile == m_MatInfos[i].m_VertexFile &&
		   m_MatInfos[i].m_VertexHandle)
		{
			m_MatInfos[Index].m_VertexHandle = m_MatInfos[i].m_VertexHandle;

			return(true);
		}
	}

	return(false);
}

//_____________________________________
//
//	FindPixelFile 
//
//_____________________________________

bool MatMgr::FindPixelFile(int Index)
{
	int i;

	for(i=0; i < m_NumMat; i++)
	{
		if(m_MatInfos[Index].m_PixelFile == m_MatInfos[i].m_PixelFile &&
		   m_MatInfos[i].m_PixelHandle)
		{
			m_MatInfos[Index].m_PixelHandle = m_MatInfos[i].m_PixelHandle;

			return(true);
		}
	}

	return(false);
}


//_____________________________________
//
//	FindAlphaPixelFile 
//
//_____________________________________

bool MatMgr::FindAlphaPixelFile(int Index)
{
	int i;

	for(i=0; i < m_NumMat; i++)
	{
		if(m_MatInfos[Index].m_AlphaPixelFile == m_MatInfos[i].m_AlphaPixelFile &&
		   m_MatInfos[i].m_AlphaPixelHandle)
		{
			m_MatInfos[Index].m_AlphaPixelHandle = m_MatInfos[i].m_AlphaPixelHandle;

			return(true);
		}
	}

	return(false);
}


//_____________________________________
//
//	GetInfo 
//
//_____________________________________

MatShaderInfo* MatMgr::GetMatInfo(int ChannelUse)
{
	int				i,j;
	MatShaderInfo	*Ret;
	int				Num;
	int				Channel;
	int				ChannelOn[CHANNEL_MAX];
	int				BestMatch;
	int				BestIndex;
	int				BestChannel;


	Num			= 0;
	Channel		= ChannelUse;
	BestMatch	= -1;
	BestIndex   = 0;
	BestChannel = 0;

	memset(ChannelOn,0,sizeof(int) * CHANNEL_MAX);

	for(i=0; i < CHANNEL_MAX; i++)
	{
		if(Channel & (1 << i))
		{ 
			ChannelOn[Num++] = (1 << i);
		}
	}

	for(i=Num-1; i >= 0; i--)
	{
		for(j=0; j < m_NumMat; j++)
		{
			if(m_MatInfos[j].m_ChannelUse == Channel)
			{
				if(Num > BestMatch)
				{
					BestIndex   = j;	
					BestMatch   = Num;
					BestChannel = Channel;

				}	
			}

		}

		Channel &= ~(ChannelOn[i]);	
	}

	if(BestMatch > -1)
	{
		Ret = &m_MatInfos[BestIndex];

		Ret->m_BestChannelUse = BestChannel;

		return(Ret);

	}

	return(&m_MatInfos[0]);
}


//_____________________________________
//
//	LoadDefaults 
//
//_____________________________________

void MatMgr::LoadDefaults(LPDIRECT3DDEVICE9 Device, bool FromResource)
{
	int		i;
	HRESULT Hr;
	TSTR	Str;

	TCHAR error[256];
	sprintf(error,"%s",GetString(IDS_ERROR));

	  
	Hr = S_OK;


	if(!m_Loaded)
	{
		//
		//	Load from file latter
		//
		for(i=0; i < MAX_MATERIALS; i++)
		{
			m_MatInfos[i].m_ChannelUse			= 0;
			m_MatInfos[i].m_VertexFile			= "";		
			m_MatInfos[i].m_VertexHandle		= 0;		
			m_MatInfos[i].m_PixelFile			= "";	
			m_MatInfos[i].m_PixelHandle			= 0;
			m_MatInfos[i].m_AlphaPixelFile		= "";	
			m_MatInfos[i].m_AlphaPixelHandle	= 0;
			m_MatInfos[i].m_BestChannelUse		= 0;		

		}

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_NORMAL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "R.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_R_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_BUMP_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_NORMAL_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_BUMP_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;	
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;	
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_BUMP_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;	
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;	
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;

		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_NORMAL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;	
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_BUMP_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_NORMAL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								

												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_DETAIL_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;		
		m_MatInfos[m_NumMat].m_PixelFile		= "DMR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DMR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;								
												
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_REFLECTION_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "DR.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_DR_NVO;
		m_MatInfos[m_NumMat].m_PixelFile		= "DR.pso";		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_DR_PSO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DRA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DRA_PSO;		
		m_NumMat++;								
												
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";	
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_MatInfos[m_NumMat].m_AlphaPixelFile	= "DMA.pso";		
		m_MatInfos[m_NumMat].m_AlphaPixelRes	= IDR_DMA_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;	
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_BUMP_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_DIFFUSE_ON | MAT_NORMAL_ON | MAT_SPECULAR_ON | MAT_DETAIL_ON | MAT_MASK_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "D.nvo";		
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_D_NVO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_BUMP_ON | MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "NR.nvo";		
		m_MatInfos[m_NumMat].m_PixelFile		= "NR.pso";	
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_NR_NVO;		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_NR_PSO;		
		m_NumMat++;								
												
		m_MatInfos[m_NumMat].m_ChannelUse		= MAT_NORMAL_ON | MAT_REFLECTION_ON;
		m_MatInfos[m_NumMat].m_VertexFile		= "NR.nvo";		
		m_MatInfos[m_NumMat].m_PixelFile		= "NR.pso";	
		m_MatInfos[m_NumMat].m_VertexRes		= IDR_NR_NVO;		
		m_MatInfos[m_NumMat].m_PixelRes			= IDR_NR_PSO;		
		m_NumMat++;								

		
		if(FromResource)
		{
			for(i=0; i < m_NumMat; i++)
			{
				m_MatInfos[i].m_VertexHandle = 0;
				m_MatInfos[i].m_PixelHandle  = 0;

				if(m_MatInfos[i].m_VertexFile.Length())
				{
					if(!FindVertexFile(i))
					{
						if(FAILED(Hr = CreateVertexShaderResource(Device,m_MatInfos[i].m_VertexRes,&m_VertexDec,
															&m_MatInfos[i].m_VertexHandle)))
						{
							m_MatInfos[i].m_VertexHandle = 0;

							Str = TSTR(GetString(IDS_ERROR_VS)) + m_MatInfos[i].m_VertexFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}
				
				if(m_MatInfos[i].m_PixelFile.Length())
				{
					if(!FindPixelFile(i))
					{
						if(FAILED(Hr = CreatePixelShaderResource(Device,m_MatInfos[i].m_PixelRes,&m_MatInfos[i].m_PixelHandle)))
						{
							m_MatInfos[i].m_PixelHandle  = 0;

							Str = TSTR(GetString(IDS_ERROR_PS)) + m_MatInfos[i].m_PixelFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}


				if(m_MatInfos[i].m_AlphaPixelFile.Length())
				{
					if(!FindAlphaPixelFile(i))
					{
						if(FAILED(Hr = CreatePixelShaderResource(Device,m_MatInfos[i].m_AlphaPixelRes,&m_MatInfos[i].m_AlphaPixelHandle)))
						{
							m_MatInfos[i].m_AlphaPixelHandle  = 0;

							Str = TSTR(GetString(IDS_ERROR_PS)) + m_MatInfos[i].m_AlphaPixelFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}

			}

		}
/*		else
		{
			for(i=0; i < m_NumMat; i++)
			{
				m_MatInfos[i].m_VertexHandle = 0;
				m_MatInfos[i].m_PixelHandle  = 0;

				if(m_MatInfos[i].m_VertexFile.Length())
				{
					if(!FindVertexFile(i))
					{
						if(FAILED(Hr = CreateShader(Device,m_MatInfos[i].m_VertexFile,gVertexDecl,
													SHADER_VERTEXSHADER,&m_MatInfos[i].m_VertexHandle)))
						{
							m_MatInfos[i].m_VertexHandle = 0;

							Str = TSTR(GetString(IDS_ERROR_VS)) + m_MatInfos[i].m_VertexFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}
				
				if(m_MatInfos[i].m_PixelFile.Length())
				{
					if(!FindPixelFile(i))
					{
						if(FAILED(Hr = CreateShader(Device,m_MatInfos[i].m_PixelFile,NULL,
													SHADER_PIXELSHADER,&m_MatInfos[i].m_PixelHandle)))
						{
							m_MatInfos[i].m_PixelHandle  = 0;

							Str = TSTR(GetString(IDS_ERROR_PS)) + m_MatInfos[i].m_PixelFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}

				if(m_MatInfos[i].m_AlphaPixelFile.Length())
				{
					if(!FindAlphaPixelFile(i))
					{
						if(FAILED(Hr = CreateShader(Device,m_MatInfos[i].m_AlphaPixelFile,NULL,
													SHADER_PIXELSHADER,&m_MatInfos[i].m_AlphaPixelHandle)))
						{
							m_MatInfos[i].m_AlphaPixelHandle  = 0;

							Str = TSTR(GetString(IDS_ERROR_PS)) + m_MatInfos[i].m_AlphaPixelFile + TSTR(GetString(IDS_CHECK_PATHS));
							MessageBox(NULL,Str,error,MB_OK | MB_ICONERROR | MB_SETFOREGROUND | 
									   MB_APPLMODAL);

						}
					}
				}


			}

		}

*/
		m_Loaded = true;
	}

}
