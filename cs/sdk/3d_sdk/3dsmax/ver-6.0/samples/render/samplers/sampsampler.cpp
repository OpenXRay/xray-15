////////////////////////////////////////////////////////////////////////
//
//	sampSampler.cpp	
//
//	Simple Sampler....single sample at 0.5, 0.5 pixels
//
//	Created: Kells Elmquist, 1 Dec, 1998
//
//	Copyright (c) 1998, All Rights Reserved.
//

#include "samplersHdr.h"
#include "stdSamplers.h"
#include "samplerUtil.h"
#include "imtl.h"


Class_ID singleSamplerClassID( SINGLE_SAMPLER_CLASS_ID , 0);

//////////////////////////////////////////////////////////////////////////
//	 Parameter Block
//
#define	PB_QUALITY	0


#define ANIMATE		TRUE
#define NO_ANIMATE	FALSE

#define PBLOCK_LENGTH 1

static ParamBlockDescID pbDesc[] = {
	{ TYPE_FLOAT, NULL, NO_ANIMATE, PB_QUALITY },  // Quality, not used but saved
}; 	


#define NUM_OLDVERSIONS	0
#define CURRENT_VERSION	1

static ParamVersionDesc curVersion(pbDesc, PBLOCK_LENGTH, CURRENT_VERSION);

static MASK fullMask = {0xffffffff,0xffffffff};

////////////////////////////////////////////////////////////////////////////
//	Single Sampler
//
class SingleSampler: public Sampler {
	private:
		ShadeContext* pSC;		
		int		n;		// sample count
		BOOL	enable;
		MASK	mask;	// fragment 8x8 mask

	public:
		// Parameters
		IParamBlock *pParamBlk;
		
		SingleSampler();
		RefTargetHandle Clone( RemapDir &remap );
		void DeleteThis() { delete this; };

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		Class_ID ClassID() {return singleSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_SINGLE_SAMPLER ); }
		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		// this starts a sample sequence for the area
//		void DoSamples( Color& c, Color&t, SamplingCallback* cb, ShadeContext* sc, MASK pMask=NULL );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK pMask=NULL );
		
		// This is the function that is called to get the next sample 
		// returns FALSE when out of samples
		BOOL NextSample( Point2* pOutPt, float* pSampleSz );

		// integer number of samples for current quality
		int GetNSamples();	

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
			}
		int SupportsQualityLevels() { return 0; }

		void SetEnable( BOOL on ){ enable = on; }
		BOOL GetEnable(){ return enable; }

		TCHAR* GetDefaultComment();

	};


class SingleSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SingleSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SINGLE_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return singleSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static SingleSamplerClassDesc singleSamplerCD;
ClassDesc* GetSingleSamplerDesc() { return &singleSamplerCD; }



//--- Single Sampler ----------------------------------------------------------


SingleSampler::SingleSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_QUALITY, 0, 0 );	
	n = 1;
	setMask( mask, ALL_ONES );
}

RefTargetHandle SingleSampler::Clone( RemapDir &remap )
{
	SingleSampler*	mnew = new SingleSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


IOResult SingleSampler::Load(ILoad *iload)
{
	Sampler::Load(iload);
	return IO_OK;
}


IOResult SingleSampler::Save(ISave *isave)
{
	Sampler::Save(isave);
	return IO_OK;
}


RefResult SingleSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	GetParamName * gpn;

	switch (message) {
		case REFMSG_CHANGE:
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim * gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_QUALITY: gpd->dim = defaultDim; break;
				default: 	     gpd->dim = defaultDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_QUALITY : gpn->name = _T( GetString(IDS_KE_QUALITY) ); break;
				default:		  gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}

void SingleSampler::DoSamples( ShadeOutput* pOut,
			 SamplingCallback* cb, ShadeContext* sc, MASK pMask )
{
	n = 0;
	if ( pMask ) 
		copyMask( mask, pMask );
	else
		setMask( mask, ALL_ONES );

	float sampleScale;
	Point2	sample;
	NextSample( &sample, &sampleScale );

//	c.r = c.g = c.b = t.r = t.g = t.b = 0;
	pOut->Reset();

	if ( sampleInMask( sample, mask, sc->globContext->fieldRender ) )
		cb->SampleAtOffset( pOut, sample, sampleScale );
	else
		cb->SampleAtOffset( pOut, sample, sampleScale );
}



BOOL SingleSampler::NextSample( Point2* pOut, float* pSampleSz )
{
	if ( n ) return FALSE;
#ifdef CENTER_OF_PIXEL
	pOut->x = pOut->y = 0.5f;
#else
	*pOut = pSC->SurfacePtScreen();	// center of fragment
    pOut->x = frac( pOut->x ); pOut->y = frac( pOut->y );
#endif
	*pSampleSz = 1.0f;	// entire pixel

	++n;
	return TRUE;
}

int SingleSampler::GetNSamples()
{
	return 1;
}


TCHAR* SingleSampler::GetDefaultComment()
{
	return GetString(IDS_KE_SINGLE_COMMENT);
}


//-----------------------------------------------------------------------------------------