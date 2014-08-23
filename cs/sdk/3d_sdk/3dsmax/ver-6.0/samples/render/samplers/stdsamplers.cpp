/**********************************************************************
 *<
	FILE: StdSamplers.cpp	

	DESCRIPTION:Standard sampler plugins

	Created Kells Elmquist, 1 dec 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "samplersHdr.h"
#include "stdSamplers.h"
#include "samplersRes.h"
#include "samplerUtil.h"
#include "macrorec.h"

// change sample sz depending on density
static BOOL sampleSzOn = TRUE;

//////////////////////////////////////////////////////////////////////////
//	 Parameter Block
//

#define ANIMATE		TRUE
#define NO_ANIMATE	FALSE

#define PBLOCK_LENGTH 3

static ParamBlockDescID pbDesc[ PBLOCK_LENGTH ] = {
	{ TYPE_FLOAT, NULL, 0, PB_QUALITY }, // Quality, not used but saved
	{ TYPE_BOOL,  NULL, 0, PB_ENABLE },  // Enable
	{ TYPE_BOOL,  NULL, 0, PB_SUBSAMP_TEX },  // Enable texture subsampling
}; 	


#define NUM_OLDVERSIONS	2
static ParamVersionDesc oldVersions[NUM_OLDVERSIONS] = {
	ParamVersionDesc(pbDesc,2, 0),
	ParamVersionDesc(pbDesc,2, 1),
};

#define PB_CURRENT_VERSION	2

static ParamVersionDesc curVersion(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION);

static MASK fullMask = {0xffffffff,0xffffffff};

////////////////////////////////////////////////////////////////////////////
//	Rev 2.5:  A x5 Center-Corner Sampler
//
Class_ID R25SamplerClassID( R25_SAMPLER_CLASS_ID , 0);

class R25Sampler: public Sampler {
//	ShadeContext* pSC;
	BOOL texSuperSampleOn;

	public:
		// Parameters
		IParamBlock *pParamBlk;

		R25Sampler();
		void DeleteThis() { delete this; };
		RefTargetHandle Clone( RemapDir &remap=NoRemap() );

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		Class_ID ClassID() {return R25SamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_R25_SAMPLER ); }
		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

		// this starts a sample sequence loop for the area of the mask
//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//								ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );
		
		// integer number of samples for current quality
		int GetNSamples(){ return 5; }	

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

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_R25_COMMENT); }
		ULONG SupportsStdParams(){ return SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

	};


class R25SamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new R25Sampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_R25_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return R25SamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static R25SamplerClassDesc r25SamplerCD;
ClassDesc* GetR25SamplerDesc() { return &r25SamplerCD; }



//--- Rev 2.5 Sampler, frag corners & center -------------------------------


R25Sampler::R25Sampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	GetCOREInterface()->GetMacroRecorder()->Disable();
	pParamBlk->SetValue(PB_ENABLE, 0, 0 );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	GetCOREInterface()->GetMacroRecorder()->Enable();
	texSuperSampleOn = TRUE; 
}

RefTargetHandle R25Sampler::Clone( RemapDir &remap )
{
	R25Sampler*	mnew = new R25Sampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


RefResult R25Sampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return intNotifyRefChanged( changeInt, hTarget, partID,  message);
}

/*********
RefResult R25Sampler::NotifyRefChanged(
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
				case PB_SUBSAMP_TEX: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				case PB_ENABLE: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				case PB_QUALITY: gpd->dim = defaultDim; break;
				default: 	     gpd->dim = defaultDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_ENABLE : gpn->name = _T( GetString(IDS_KE_QUALITY) ); break;
				case PB_QUALITY : gpn->name = _T( GetString(IDS_KE_QUALITY) ); break;
				case PB_SUBSAMP_TEX : gpn->name = _T( GetString(IDS_KE_SUBSAMP_TEX) ); break;
				default:		  gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}
**********/
IOResult R25Sampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = PB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class R25SamplerCB: public PostLoadCallback {
	public:
		R25Sampler *s;
		int loadVersion;
	    R25SamplerCB(R25Sampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {

			 s->SetTextureSuperSampleOn( FALSE );
			 delete this;
		}
};

IOResult R25Sampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < PB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUM_OLDVERSIONS, &curVersion, this,0));
		iload->RegisterPostLoadCallback(new R25SamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


#define FAC 0.3125f
#define N_R25SAMPLES	5

void R25Sampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{
	Point2 samplePt;
	float sampleScale;
	float nSamples = 0.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	sampleScale = GetTextureSuperSampleOn() ? 0.5f : 1.0f;
	for( int n = 0; n < 5; ++n ){

		samplePt.x = 0.5f;	samplePt.y = 0.5f;	
		switch( n ) {
			case 0 : samplePt.x += FAC; samplePt.y += FAC; break;
			case 1 : samplePt.x += FAC; samplePt.y += -FAC; break;
			case 2 : samplePt.x += -FAC; samplePt.y += -FAC; break;
			case 3 : samplePt.x += -FAC; samplePt.y += FAC; break;
			case 4 : { samplePt = sc->SurfacePtScreen(); } break;
		}
		if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender  ) ) {
			// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
				(*pOut) += sampOut;
				nSamples += 1.0f;
			}
		}
		
	} // end, for samples

	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}




///////////////////////////////////////////////////////////////


Class_ID UniformSamplerClassID( UNIFORM_SAMPLER_CLASS_ID , 0);



////////////////////////////////////////////////////////////////////////////
//	Regular, variable quality Sampler: n x n samples w/ n== 1..6
//
#define	N_UNIFORM_MAX_SAMPLES	6	// 6x6 samples max
#define	N_UNIFORM_MAX_SAMPLES2  (N_UNIFORM_MAX_SAMPLES*N_UNIFORM_MAX_SAMPLES)

class UniformSampler: public Sampler {
	public:
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;

		UniformSampler();
		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
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

		Class_ID ClassID() {return UniformSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_UNIFORM_SAMPLER ); }
		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

		// this starts a sample sequence loop for the area of the mask
//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//								ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );
		
		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
			}
		int SupportsQualityLevels() { return N_UNIFORM_MAX_SAMPLES; }

		// integer number of samples for current quality
		int GetNSamples();
		int GetSideSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_UNIFORM_COMMENT); }
		ULONG SupportsStdParams(){ return SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

	};


class UniformSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new UniformSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_UNIFORM_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return UniformSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static UniformSamplerClassDesc uniformSamplerCD;
ClassDesc* GetUniformSamplerDesc() { return &uniformSamplerCD; }



//--- UniformSampler -------------------------------


UniformSampler::UniformSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, 0 );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle UniformSampler::Clone( RemapDir &remap )
{
	UniformSampler*	mnew = new UniformSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


int UniformSampler::GetSideSamples()
{
	int side = int( GetQuality() * N_UNIFORM_MAX_SAMPLES );
	if (side < 2 ) side = 2;
	if (side > N_UNIFORM_MAX_SAMPLES ) side = N_UNIFORM_MAX_SAMPLES;
	return side;
}	

int UniformSampler::GetNSamples()
{
	int side = GetSideSamples();
	return side * side;
}	


RefResult UniformSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return intNotifyRefChanged( changeInt, hTarget, partID,  message);
}

IOResult UniformSampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = PB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class UniformSamplerCB: public PostLoadCallback {
	public:
		UniformSampler *s;
		int loadVersion;
	    UniformSamplerCB(UniformSampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult UniformSampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < PB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUM_OLDVERSIONS, &curVersion, this,0));
		iload->RegisterPostLoadCallback(new UniformSamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


void UniformSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
						ShadeContext* sc, MASK mask )
{
	int sideSamples = GetSideSamples();
//	int numSamples = sideSamples * sideSamples;
	DbgAssert( sideSamples > 0 );
	// we map 0...sideSz into 0..1
	float sideSzInv = 1.0f / float(sideSamples);
	float sampleScale = texSuperSampleOn ? sideSzInv : 1.0f;	

	Point2 samplePt;
	float nSamples = 0.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	for( int y = 0; y < sideSamples; ++y ) {
		samplePt.y = (float(y) + 0.5f) * sideSzInv;

		for( int x = 0; x < sideSamples; ++x ) {
			samplePt.x =  (float(x) + 0.5f) * sideSzInv;

			if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
				// NB, returns true for unclipped samples
				if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
					(*pOut) += sampOut;
					nSamples += 1.0f;
				}
			}
		}
	} // end, while samples

	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}


/******
	// Sampling loop
	for( int y = 0; y < sideSamples; ++y ) {
		sample.y = (float(y) + 0.5f) * sideSzInv;

		for( int x = 0; x < sideSamples; ++x ) {
			sample.x =  (float(x) + 0.5f) * sideSzInv;
	
			if ( sampleInMask( sample, mask ) ) {
				Color c, t;
				// NB, returns true for unclipped samples
				if (cb->SampleAtOffset( c, t, sample, sampleScale )) {
					clr += c;
					trans += t;
					nSamples += 1.0f;
				}
			}
		}
	}

	// Check for 0 samples
	if ( nSamples == 0.0f ){
		// use frag center if no other samples
		sample = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( clr, trans, sample, 1.0f );
	} else {
		clr /= nSamples;
		trans /= nSamples;
	}
}

********/
////////////////////////////////////////////////////////////////////////////////
//	Canonical Multi-Jitter: variable quality Sampler: n x n samples w/ n== 1..6
//
#define	N_CMJ_MAX_SIDESAMPLES	6	// 6x6 samples max

Class_ID CMJSamplerClassID( CMJ_SAMPLER_CLASS_ID , 0);

class CMJSampler: public Sampler {

	public:
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;

		CMJSampler();
		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
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

		Class_ID ClassID() {return CMJSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_CMJ_SAMPLER ); }
		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

		// this starts a sample sequence loop for the area of the mask
//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//								ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
			}
		int SupportsQualityLevels() { return N_CMJ_MAX_SIDESAMPLES; }

		// integer number of samples for current quality
		int GetNSamples();
		int GetSideSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_CMJ_COMMENT); }
		ULONG SupportsStdParams(){ return SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

	};


class CMJSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new CMJSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CMJ_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return CMJSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static CMJSamplerClassDesc CMJSamplerCD;
ClassDesc* GetCMJSamplerDesc() { return &CMJSamplerCD; }



//--- Canonical Multi-Jitter (w/ no jitter) Sampler -------------------------------


CMJSampler::CMJSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, 0 );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle CMJSampler::Clone( RemapDir &remap )
{
	CMJSampler*	mnew = new CMJSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int CMJSampler::GetSideSamples()
{
	int side = int( GetQuality() * N_CMJ_MAX_SIDESAMPLES );
	if (side < 2 ) side = 2;
	if (side > N_CMJ_MAX_SIDESAMPLES ) side = N_CMJ_MAX_SIDESAMPLES;
	return side;
}	

int CMJSampler::GetNSamples()
{
	int side = GetSideSamples();
	return side * side;
}	


RefResult CMJSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return intNotifyRefChanged( changeInt, hTarget, partID,  message);
}
IOResult CMJSampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = PB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class CMJSamplerCB: public PostLoadCallback {
	public:
		CMJSampler *s;
		int loadVersion;
	    CMJSamplerCB(CMJSampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult CMJSampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < PB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUM_OLDVERSIONS, &curVersion, this,0));
		iload->RegisterPostLoadCallback(new CMJSamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


void CMJSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{
	int sideSamples = GetSideSamples();
	int numSamples = sideSamples * sideSamples;
	DbgAssert( sideSamples > 0 );

	float sampleScale = texSuperSampleOn ? 1.0f / float(sideSamples) : 1.0f;	

	float subCellSz = 1.0f / float( numSamples );
	float halfSubCell = subCellSz * 0.5f;

	Point2 samplePt;
	float nSamples = 0.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	for( int y = 0; y < sideSamples; ++y ) {
		for( int x = 0; x < sideSamples; ++x ) {

			samplePt.x = (float(x * sideSamples)+ float(y) + 0.5f) * subCellSz;
			samplePt.y = (float(y * sideSamples)+ float(x) + 0.5f) * subCellSz;

			if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
				// NB, returns true for unclipped samples
				if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
					(*pOut) += sampOut;
					nSamples += 1.0f;
				}
			}
		}
	} // end, while samples

	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}
/***********************
	// Sampling loop
	for( int y = 0; y < sideSamples; ++y ) {
		for( int x = 0; x < sideSamples; ++x ) {

			sample.x = (float(x * sideSamples)+ float(y) + 0.5f) * subCellSz;
			sample.y = (float(y * sideSamples)+ float(x) + 0.5f) * subCellSz;
	
			if ( sampleInMask( sample, mask ) ) {
				Color c, t;
				// NB, returns true for unclipped samples
				if (cb->SampleAtOffset( c, t, sample, sampleScale )) {
					clr += c;
					trans += t;
					nSamples += 1.0f;
				}
			}
		}
	}

	// Check for 0 samples
	if ( nSamples == 0.0f ){
		// use frag center if no other samples
		sample = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( clr, trans, sample, 1.0f );
	} else {
		clr /= nSamples;
		trans /= nSamples;
	}
}
****************/

////////////////////////////////////////////////////////////////////////////////
//	Hammersley: variable quality Sampler: n cols of 1 sample
//
#define	N_HAMM_MAX_SAMPLES	40	

Class_ID HammersleySamplerClassID( HAMMERSLEY_SAMPLER_CLASS_ID , 0);

class HammersleySampler: public Sampler {

	public:
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;

		HammersleySampler();
		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
		void DeleteThis() { delete this; };

		Class_ID ClassID() {return HammersleySamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_HAMM_SAMPLER ); }

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//								ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
			}
		int SupportsQualityLevels() { return N_HAMM_MAX_SAMPLES; }

		// integer number of samples for current quality
		int GetNSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_HAMM_COMMENT); }
		ULONG SupportsStdParams(){ return SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }
	};


class HammersleySamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new HammersleySampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_HAMM_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return HammersleySamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static HammersleySamplerClassDesc HammersleySamplerCD;
ClassDesc* GetHammersleySamplerDesc() { return &HammersleySamplerCD; }



//--- Hammersley Sampler -------------------------------


HammersleySampler::HammersleySampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, 0 );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle HammersleySampler::Clone( RemapDir &remap )
{
	HammersleySampler*	mnew = new HammersleySampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


int HammersleySampler::GetNSamples()
{
	int n = int( GetQuality() * N_HAMM_MAX_SAMPLES );
	return (n < 4) ? 4 : n;
}	


RefResult HammersleySampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return intNotifyRefChanged( changeInt, hTarget, partID,  message);
}

IOResult HammersleySampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = PB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class HammersleySamplerCB: public PostLoadCallback {
	public:
		HammersleySampler *s;
		int loadVersion;
	    HammersleySamplerCB(HammersleySampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult HammersleySampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < PB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUM_OLDVERSIONS, &curVersion, this,0));
		iload->RegisterPostLoadCallback(new HammersleySamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


void HammersleySampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{
	int numSamples = GetNSamples();
	DbgAssert( numSamples > 0 );

	float sampleScale = texSuperSampleOn ? 1.0f / sqrtf( float(numSamples) ) : 1.0f;	

	IPoint2 s = sc->ScreenCoord(); 
	int nx = sc->globContext->devWidth;
	int nSample = ( s.y * nx + s.x ) * numSamples;

	Point2 samplePt;
	float nSamples = 0.0f;
	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	sc->out.Reset(nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	// Sampling loop
	for( int n = 0; n < numSamples; ++n ) {

		samplePt.x = radicalInverse2( nSample + n  );
		samplePt.y = (float(n)+0.5f)/numSamples;

		if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
			// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
				(*pOut) += sampOut;
				nSamples += 1.0f;
			}
		}
	} // end, while samples

	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}
/**************
	// Sampling loop
	for( int n = 0; n < numSamples; ++n ) {

		sample.x = radicalInverse2( nSample + n  );
		sample.y = (float(n)+0.5f)/numSamples;

		if ( sampleInMask( sample, mask ) ) {
			Color c, t;
			// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( c, t, sample, sampleScale )) {
				clr += c;
				trans += t;
				nSamples += 1.0f;
			}
		}
	}

	// Check for 0 samples
	if ( nSamples == 0.0f ){
		// use frag center if no other samples
		sample = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( clr, trans, sample, 1.0f );
	} else {
		clr /= nSamples;
		trans /= nSamples;
	}
}
**************/


////////////////////////////////////////////////////////////////////////////////
//	Halton: variable quality Sampler: a double hammersley
//
#define	N_HALTON_MAX_SAMPLES	40	

static int baseA = 2;
static int baseB = 3;

Class_ID HaltonSamplerClassID( HALTON_SAMPLER_CLASS_ID , 0);

class HaltonSampler: public Sampler {

	public:
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;

		HaltonSampler();
		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
		void DeleteThis() { delete this; };

		Class_ID ClassID() {return HaltonSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_HALTON_SAMPLER ); }

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//						ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );

		// integer number of samples for current quality
		int GetNSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
		}

		int SupportsQualityLevels() { return N_HALTON_MAX_SAMPLES; }
		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_HALTON_COMMENT); }
		ULONG SupportsStdParams(){ return SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

	};


class HaltonSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new HaltonSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_HALTON_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return HaltonSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static HaltonSamplerClassDesc HaltonSamplerCD;
ClassDesc* GetHaltonSamplerDesc() { return &HaltonSamplerCD; }



//--- Halton Sampler -------------------------------


HaltonSampler::HaltonSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(pbDesc, PBLOCK_LENGTH, PB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, 0 );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle HaltonSampler::Clone( RemapDir &remap )
{
	HaltonSampler*	mnew = new HaltonSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


int HaltonSampler::GetNSamples()
{
	int n = int( GetQuality() * N_HAMM_MAX_SAMPLES );
	return (n < 4) ? 4 : n;
}	


RefResult HaltonSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return intNotifyRefChanged( changeInt, hTarget, partID,  message);
}
IOResult HaltonSampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = PB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class HaltonSamplerCB: public PostLoadCallback {
	public:
		HaltonSampler *s;
		int loadVersion;
	    HaltonSamplerCB(HaltonSampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult HaltonSampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < PB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUM_OLDVERSIONS, &curVersion, this,0));
		iload->RegisterPostLoadCallback(new HaltonSamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


void HaltonSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{

	int numSamples = GetNSamples();
	DbgAssert( numSamples > 0 );

	float sampleScale = texSuperSampleOn ? 1.0f / sqrtf( float(numSamples) ) : 1.0f;	

	IPoint2 s = sc->ScreenCoord(); 
	int nx = sc->globContext->devWidth;
	int nSample = ( s.y * nx + s.x ) * numSamples;

	Point2 samplePt;
	float nSamples = 0.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	// Sampling loop
	for( int n = 0; n < numSamples; ++n ) {

		samplePt.x = radicalInverse2( nSample + n  );
		samplePt.y = radicalInverse( nSample + n, baseB );

		if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
			// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
				(*pOut) += sampOut;
				nSamples += 1.0f;
			}
		}
	} // end, while samples

	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}
/********************
	// Sampling loop
	for( int n = 0; n < numSamples; ++n ) {

		sample.x = radicalInverse2( nSample + n  );
		sample.y = radicalInverse( nSample + n, baseB );

		if ( sampleInMask( sample, mask ) ) {
			Color c, t;
			// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( c, t, sample, sampleScale )) {
				clr += c;
				trans += t;
				nSamples += 1.0f;
			}
		}
	}

	// Check for 0 samples
	if ( nSamples == 0.0f ){
		// use frag center if no other samples
		sample = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( clr, trans, sample, 1.0f );
	} else {
		clr /= nSamples;
		trans /= nSamples;
	}
}
*********************/

////////////////////////////////////////////////////////////////////////////////
//	Adaptive Parameter Block
//
#define APBLOCK_LENGTH 5

static ParamBlockDescID apbDesc[ APBLOCK_LENGTH ] = {
	{ TYPE_FLOAT, NULL, 0, PB_QUALITY },			// Quality
	{ TYPE_BOOL,  NULL, 0, PB_ENABLE },		 		// Enable
	{ TYPE_BOOL,  NULL, 0, PB_SUBSAMP_TEX },		// subsample textures on
	{ TYPE_BOOL,  NULL, 0, PB_ADAPT_ENABLE },		// AdaptiveEnable
	{ TYPE_FLOAT, NULL, 0, PB_ADAPT_THRESHOLD },	// Threshold
}; 	

#define NUM_OLDAVERSIONS	2
static ParamVersionDesc oldAVersions[NUM_OLDAVERSIONS] = {
	ParamVersionDesc(apbDesc,4, 0),
	ParamVersionDesc(apbDesc,4, 1),
};

#define APB_CURRENT_VERSION	2

static ParamVersionDesc curAVersion(apbDesc, APBLOCK_LENGTH, APB_CURRENT_VERSION);

//------------------------------------------------------------------
static inline float Max( Color& c )
{ return c.r > c.g ? (c.r > c.b ? c.r : c.b) : (c.g > c.b ? c.g : c.b); }
static inline float Max( float a, float b ){ return a > b ? a : b; }
static inline float Max( float a, float b, float c  )
{ return a>b ? (a > c ? a : c) : (b > c ? b : c); }

static inline void Abs( Color& c )
{ if (c.r < 0.0f) c.r = -c.r; if (c.g < 0.0f) c.g = -c.g; if (c.b < 0.0f) c.b = -c.b;}
static inline float Abs( float a ){ return a < 0.0f ? -a : a; }

////////////////////////////////////////////////////////////////////////////////
//	Adaptive Halton: variable quality Sampler: a double hammersley
//
static int nAdaptiveSamples = 4;


Class_ID AHaltonSamplerClassID( AHALTON_SAMPLER_CLASS_ID , 0);

class AdaptiveHaltonSampler: public Sampler {
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;
	public:

		AdaptiveHaltonSampler();

		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
		void DeleteThis() { delete this; };

		Class_ID ClassID() {return AHaltonSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_AHALTON_SAMPLER ); }

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//						ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );

		// integer number of samples for current quality
		int GetNSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
		}

		int SupportsQualityLevels() { return N_HALTON_MAX_SAMPLES; }
		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_AHALTON_COMMENT); }

		// Adaptive Sampling
		ULONG SupportsStdParams(){ return R3_ADAPTIVE+SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetAdaptiveOn( BOOL on ){ pParamBlk->SetValue( PB_ADAPT_ENABLE, 0, on ); }
		BOOL IsAdaptiveOn(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ADAPT_ENABLE, 0, b, valid );
				return b;
			}

		void SetAdaptiveThreshold( float val )
			{ pParamBlk->SetValue( PB_ADAPT_THRESHOLD, 0, val ); }
		float GetAdaptiveThreshold() { float q; Interval valid;
				pParamBlk->GetValue( PB_ADAPT_THRESHOLD, 0, q, valid );
				return q;
			}

		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

	};


class AdaptiveHaltonSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AdaptiveHaltonSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_AHALTON_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return AHaltonSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static AdaptiveHaltonSamplerClassDesc AHaltonSamplerCD;
ClassDesc* GetAHaltonSamplerDesc() { return &AHaltonSamplerCD; }



//--- Adaptive Halton Sampler -------------------------------


AdaptiveHaltonSampler::AdaptiveHaltonSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(apbDesc, APBLOCK_LENGTH, APB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, FALSE );	
	pParamBlk->SetValue(PB_ADAPT_ENABLE, 0, TRUE );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_ADAPT_THRESHOLD, 0, 0.020f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle AdaptiveHaltonSampler::Clone( RemapDir &remap )
{
	AdaptiveHaltonSampler*	mnew = new AdaptiveHaltonSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


int AdaptiveHaltonSampler::GetNSamples()
{
	int n = int( GetQuality() * N_HAMM_MAX_SAMPLES );
	return (n < 4) ? 4 : n;
}	


RefResult AdaptiveHaltonSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return adaptNotifyRefChanged( changeInt, hTarget, partID,  message);
}

IOResult AdaptiveHaltonSampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = APB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class AdaptiveHaltonSamplerCB: public PostLoadCallback {
	public:
		AdaptiveHaltonSampler *s;
		int loadVersion;
	    AdaptiveHaltonSamplerCB(AdaptiveHaltonSampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult AdaptiveHaltonSampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < APB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldAVersions,NUM_OLDAVERSIONS, &curAVersion, this,0));
		iload->RegisterPostLoadCallback(new AdaptiveHaltonSamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}



void AdaptiveHaltonSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{
	int numSamples = GetNSamples();
	DbgAssert( numSamples > 0 );

	BOOL adaptOn = IsAdaptiveOn();
	float thresh = GetAdaptiveThreshold();

	// scale of edge of sample
	float sampleScale = texSuperSampleOn ? 1.0f / sqrtf( float(numSamples) ) : 1.0f;	

	IPoint2 s = sc->ScreenCoord(); 
	int nx = sc->globContext->devWidth;
	int nSample = ( s.y * nx + s.x ) * numSamples;

	float max = -1.0f;
	Point2 samplePt;
	float nSamples = 0.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	// Sampling loop
	Color ref;
	for( int n = 0, ns = 0; n < numSamples; ++n ) {

		samplePt.x = radicalInverse2( nSample + n  );
		samplePt.y = radicalInverse( nSample + n, baseB );

		if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
				// NB, returns true for unclipped samples
			if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
				(*pOut) += sampOut;
				nSamples += 1.0f;

				if ( adaptOn && ns < nAdaptiveSamples ){
					if ( ns == 0 ) {
						ref = pOut->c; 
					} else {
						Color d = pOut->c - ref;
						Abs( d );
						float m = Max( d );
						if ( m > max ) max = m;
						adaptOn = max > thresh ? FALSE: TRUE; //once were over, no more tests
						if( (ns == nAdaptiveSamples-1) && adaptOn )
								goto doneSampling;
					}
				}
				++ns;
			}
		}
	}

doneSampling: 
	// Check for 0 samples
	if ( nSamples == 0.0f ){
		// gets center of frag in screen space
		samplePt = sc->SurfacePtScreen(); 
		cb->SampleAtOffset( pOut, samplePt, 1.0f );
	} else {
//		*pOut *= 1.0f / nSamples;
		pOut->Scale( 1.0f / nSamples );
	}
}



////////////////////////////////////////////////////////////////////////////////
//	Adaptive CMJ: variable quality Sampler w/ simple adaption 

static int nCMJAdaptiveSamples = 3;

Class_ID ACMJSamplerClassID( ACMJ_SAMPLER_CLASS_ID , 0);

class ACMJSampler: public Sampler {
		// Parameters
		IParamBlock *pParamBlk;
		BOOL texSuperSampleOn;
	public:

		ACMJSampler();

		RefTargetHandle Clone( RemapDir &remap=NoRemap() );
		void DeleteThis() { delete this; };

		Class_ID ClassID() {return ACMJSamplerClassID;};
		TSTR GetName() { return GetString( IDS_KE_ACMJ_SAMPLER ); }

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) pParamBlk = (IParamBlock*)rtarg; }

		void GetClassName(TSTR& s) { s = GetName(); }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							         PartID& partID,  RefMessage message);

		IOResult Load(ILoad *iload); //{ return Sampler::Load(iload); }
		IOResult Save(ISave *isave); //{ return	Sampler::Save(isave); }

//		virtual void DoSamples( Color& c, Color&t, SamplingCallback* cb, 
//						ShadeContext* sc, MASK pMask );
		virtual void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, 
								MASK mask=NULL );

		// integer number of samples for current quality
		int GetNSamples();

		void SetEnable( BOOL on )
			{ pParamBlk->SetValue( PB_ENABLE, 0, on ); }

		BOOL GetEnable(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ENABLE, 0, b, valid );
				return b;
			}

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		void SetQuality( float q )
			{ pParamBlk->SetValue( PB_QUALITY, 0, q ); }

		float GetQuality() { float q; Interval valid;
				pParamBlk->GetValue( PB_QUALITY, 0, q, valid );
				return q;
		}

		int SupportsQualityLevels() { return N_UNIFORM_MAX_SAMPLES; }
		TCHAR* GetDefaultComment(){	return GetString(IDS_KE_ACMJ_COMMENT); }

		// Adaptive Sampling, non-reqd methods
		ULONG SupportsStdParams(){ return R3_ADAPTIVE+SUPER_SAMPLE_TEX_CHECK_BOX; }

		void SetAdaptiveOn( BOOL on ){ pParamBlk->SetValue( PB_ADAPT_ENABLE, 0, on ); }
		BOOL IsAdaptiveOn(){ BOOL b; Interval valid;
				pParamBlk->GetValue( PB_ADAPT_ENABLE, 0, b, valid );
				return b;
			}

		void SetAdaptiveThreshold( float val )
			{ pParamBlk->SetValue( PB_ADAPT_THRESHOLD, 0, val ); }
		float GetAdaptiveThreshold() { float q; Interval valid;
				pParamBlk->GetValue( PB_ADAPT_THRESHOLD, 0, q, valid );
				return q;
			}
		void SetTextureSuperSampleOn( BOOL on )
		{
			texSuperSampleOn = on; 
			pParamBlk->SetValue( PB_SUBSAMP_TEX, 0, on ); 
		}
		BOOL GetTextureSuperSampleOn(){ return texSuperSampleOn; }

		int GetSideSamples();
	};


class ACMJSamplerClassDesc : public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ACMJSampler; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ACMJ_SAMPLER); }
	SClass_ID		SuperClassID() { return SAMPLER_CLASS_ID; }
	Class_ID 		ClassID() { return ACMJSamplerClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static ACMJSamplerClassDesc ACMJSamplerCD;
ClassDesc* GetACMJSamplerDesc() { return &ACMJSamplerCD; }



//--- Adaptive CMJ Sampler -------------------------------


ACMJSampler::ACMJSampler()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(apbDesc, APBLOCK_LENGTH, APB_CURRENT_VERSION));
	DbgAssert(pParamBlk);
	pParamBlk->SetValue(PB_ENABLE, 0, FALSE );	
	pParamBlk->SetValue(PB_ADAPT_ENABLE, 0, TRUE );	
	pParamBlk->SetValue(PB_QUALITY, 0, 0.5f );	
	pParamBlk->SetValue(PB_ADAPT_THRESHOLD, 0, 0.020f );	
	pParamBlk->SetValue(PB_SUBSAMP_TEX, 0, TRUE );	
	texSuperSampleOn = TRUE; 
}

RefTargetHandle ACMJSampler::Clone( RemapDir &remap )
{
	ACMJSampler*	mnew = new ACMJSampler();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	mnew->texSuperSampleOn =	texSuperSampleOn; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int ACMJSampler::GetSideSamples()
{
	int side = int( GetQuality() * N_CMJ_MAX_SIDESAMPLES );
	if (side < 2 ) side = 2;
	if (side > N_CMJ_MAX_SIDESAMPLES ) side = N_CMJ_MAX_SIDESAMPLES;
	return side;
}	

int ACMJSampler::GetNSamples()
{
	int side = GetSideSamples();
	return side * side;
}	



RefResult ACMJSampler::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return adaptNotifyRefChanged( changeInt, hTarget, partID,  message);
}

IOResult ACMJSampler::Save(ISave *isave)
{ 
	ULONG nb;
	isave->BeginChunk(SAMPLER_VERS_CHUNK);
	int version = APB_CURRENT_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	return	Sampler::Save(isave); 
}

class ACMJSamplerCB: public PostLoadCallback {
	public:
		ACMJSampler *s;
		int loadVersion;
	    ACMJSamplerCB(ACMJSampler *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			 s->SetTextureSuperSampleOn( TRUE );
			 delete this;
		}
};

IOResult ACMJSampler::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SAMPLER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < APB_CURRENT_VERSION ) {
		iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldAVersions,NUM_OLDAVERSIONS, &curAVersion, this,0));
		iload->RegisterPostLoadCallback(new ACMJSamplerCB(this, version));
		iload->SetObsolete();
	}

	return	Sampler::Load(iload); 
}


void ACMJSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, 
					ShadeContext* sc, MASK mask )
{
	BOOL adaptOn = IsAdaptiveOn();
	float thresh = GetAdaptiveThreshold();

	int sideSamples = GetSideSamples();
	int numSamples = sideSamples * sideSamples;
	DbgAssert( sideSamples > 0 );

	float sampleScale = texSuperSampleOn ? 1.0f / float(sideSamples) : 1.0f;	

	float subCellSz = 1.0f / float( numSamples );
	float halfSubCell = subCellSz * 0.5f;

	// Sampling loop
	int ns = 0;
	float max = -1.0f;

	float nSamples = 1.0f;
//	int nEle = sc->NRenderElements();
//	pOut->Reset( nEle );
//	ShadeOutput sampOut( nEle );
	pOut->Reset();
	ShadeOutput sampOut( sc->NRenderElements() );

	// use frag center as ref for other samples
	Point2 samplePt = sc->SurfacePtScreen(); 
	cb->SampleAtOffset( pOut, samplePt, 1.0f );
	Color ref = pOut->c;

	for( int y = 0; y < sideSamples; ++y ) {
		for( int x = 0; x < sideSamples; ++x ) {

			samplePt.x = (float(x * sideSamples)+ float(y) + 0.5f) * subCellSz;
			samplePt.y = (float(y * sideSamples)+ float(x) + 0.5f) * subCellSz;
	
			if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
				// NB, returns true for unclipped samples
				if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
					(*pOut) += sampOut;
					nSamples += 1.0f;
	
					if ( adaptOn && ns < nCMJAdaptiveSamples ){
						// use the max component of the difference between c & the ref clr
						Color d = sampOut.c - ref;
						Abs( d );
						float m = Max( d );
						if ( m > max ) max = m;
						adaptOn = max > thresh ? FALSE: TRUE; //once were over, no more tests
						if( (ns == nCMJAdaptiveSamples-1) && adaptOn )
								goto doneSampling;
					}
					++ns;
				}
			}
		}
	}

doneSampling: 
//	(*pOut) *= 1.0f/nSamples;
	pOut->Scale( 1.0f / nSamples );

}






