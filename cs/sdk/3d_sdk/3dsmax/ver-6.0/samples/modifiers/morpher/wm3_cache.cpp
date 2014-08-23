/*===========================================================================*\
 | 
 |  FILE:	wM3_cache.cpp
 |			Weighted Morpher for MAX R3
 |			MorphCache class
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 3-10-98
 | 
\*===========================================================================*/

#include "wM3.h"

void TargetCache::Init(INode *nd)
{	
	if(!nd) return;
	
	ObjectState os = nd->EvalWorldState(GetCOREInterface()->GetTime());

	mNumPoints = os.obj->NumPoints();
	mTargetPoints.resize(mNumPoints);

	for(long i=0; i<mNumPoints; i++) { mTargetPoints[i] = os.obj->GetPoint(i); }
}

void TargetCache::Reset(void)
{
	mTargetPoints.clear();
	mTargetINode = NULL;
}

TargetCache::TargetCache(const TargetCache &tcache)
{
	mTargetINode = tcache.mTargetINode;
	mNumPoints = tcache.mNumPoints;
	mTargetPoints = tcache.mTargetPoints;
	mTargetPercent = tcache.mTargetPercent;
}

void TargetCache::operator=(const TargetCache &tcache)
{
	mTargetINode = tcache.mTargetINode;
	mNumPoints = tcache.mNumPoints;
	mTargetPoints = tcache.mTargetPoints;
}

void TargetCache::operator=(const morphChannel &mchan)
{
	mNumPoints = mchan.mNumPoints;
	mTargetPoints = mchan.mPoints;
	mTargetINode = mchan.mConnection;
}

IOResult TargetCache::Save(ISave* isave)
{
	ULONG nb;

	isave->BeginChunk(MR3_TARGETCACHE_POINTS);
	isave->Write(&mNumPoints,sizeof(long),&nb);
	for(long k=0; k<mNumPoints; k++) { isave->Write(&mTargetPoints[k],sizeof(Point3),&nb); }
	isave->EndChunk();

	isave->BeginChunk(MR3_PROGRESSIVE_TARGET_PERCENT);
	isave->Write(&mTargetPercent,sizeof(float),&nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult TargetCache::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res = IO_OK;
	long k;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
			case MR3_TARGETCACHE_POINTS:
				res = iload->Read(&mNumPoints,sizeof(long),&nb); 
				mTargetPoints.resize(mNumPoints);
				for(k=0; k<mNumPoints; k++) { 
					res = iload->Read(&mTargetPoints[k], sizeof(Point3),&nb);
					if (res!=IO_OK) return res;
				}
			break;

			case MR3_PROGRESSIVE_TARGET_PERCENT:
				res = iload->Read(&mTargetPercent,sizeof(float),&nb); 
			break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	return IO_OK;
}

void morphCache::NukeCache()
{
	if(oPoints) delete [] oPoints;
	oPoints = NULL;
	if(oWeights) delete [] oWeights;
	oWeights = NULL;

	sel.SetSize(0,0);
	sel.ClearAll();

	CacheValid = FALSE;
}

void morphCache::MakeCache(Object *obj)
{
	Count = obj->NumPoints();
	oPoints = new Point3[Count];
	oWeights = new double[Count];

	sel.SetSize(Count);
	sel.ClearAll();

	for(int t=0;t<Count;t++)
	{
		oPoints[t] = obj->GetPoint(t);
		oWeights[t] = obj->GetWeight(t);
		
		sel.Set( t, (obj->PointSelection(t)>0.0f)?1:0 );
	}

	CacheValid = TRUE;
}

BOOL morphCache::AreWeCached()
{
	return CacheValid;
}

morphCache::morphCache()
{
	CacheValid	= FALSE;
	oPoints		= NULL;
	oWeights	= NULL;
	Count = 0;
	sel.SetSize(0,0);
	sel.ClearAll();
}
