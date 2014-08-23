/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: multiPassMotionBlur.cpp

	 DESCRIPTION: definitions for class MultiPassMotionBlur

	 CREATED BY: michael malone (mjm)

	 HISTORY: created July 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// local headers
#include "dllMain.h"
#include "blendTable.h"

// sdk headers
#include <iparamm2.h>
#include <bmmlib.h>

// IDs to references
#define PBLOCK_REF 0

#define MAX_COLf 65535.0f
#define DEF_OFFSET 0

// parameter blocks IDs
enum
{
	multiPassMotionBlur_params,
};

// parameters for multiPassMotionBlur_params
enum
{
	prm_displayPasses,
	prm_totalPasses,
	prm_duration,
	prm_bias,
	prm_normalizeWeights,
	prm_ditherStrength,
	prm_tileSize,
	prm_disableFiltering,
	prm_disableAntialiasing,
};

// global instance
static const Class_ID multiPassMotionBlurClassID(0xd481518, 0x687d7c99);


// ----------------------------------------------
// multiPassMotionBlur camera effect - class declaration
// ----------------------------------------------
class MultiPassMotionBlur : public IMultiPassCameraEffect
{
	IParamBlock2* mpPBlock;
	int mTotalPasses;
	bool mbBlendTableInvalid;
	TNoiseFn mfpNoiseFn;
	BlendTable *mpBlendTable;
	BOOL mbIsScanRenderer, mbDisableFiltering, mbPrevFilter, mbDisableAntialiasing, mbPrevAntialias;
	IScanRenderer *mpIScanRenderer;

public:

	MultiPassMotionBlur();
	~MultiPassMotionBlur() { }

	// from class IMultiPassCameraEffect
	bool IsCompatible(CameraObject *pCameraObject) { return true; }
	bool DisplayPasses(TimeValue renderTime);
	int TotalPasses(TimeValue renderTime);
	ViewParams *Apply(INode *pCameraNode, CameraObject *pCameraObject, int passNum, TimeValue &overrideRenderTime);
	void AccumulateBitmap(Bitmap *pDest, Bitmap *pSrc, int passNum, TimeValue renderTime);
	void PostRenderFrame();

	// from class ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// from class ReferenceMaker
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);

	// from class Animatable
	void GetClassName(TSTR& s) { s = GetString(IDS_MP_MOT_BLUR); }
	Class_ID ClassID() { return multiPassMotionBlurClassID; }
	void DeleteThis() { delete this; }
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev=NULL);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next=NULL);
	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return (i == 0) ? mpPBlock : NULL; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPBlock->ID() == id) ? mpPBlock : NULL; }
};


// -------------------------------------------------
// MultiPassMotionBlur class descriptor - class declaration
// -------------------------------------------------
class MultiPassMotionBlurClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MultiPassMotionBlur; }
	const TCHAR *	ClassName() { return GetString(IDS_MP_MOT_BLUR); }
	SClass_ID		SuperClassID() { return MPASS_CAM_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return multiPassMotionBlurClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("multiPassMotionBlur"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static MultiPassMotionBlurClassDesc multiPassMotionBlurClassDesc;
// external access function (through dllMain.h)
ClassDesc* GetMultiPassMotionBlurClassDesc() { return &multiPassMotionBlurClassDesc; }


// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 multiPassMotionBlur_param_blk(multiPassMotionBlur_params, _T("multiPassMotionBlur parameters"), 0, &multiPassMotionBlurClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_MP_MOT_BLUR_CAM_EFFECT, IDS_MP_MOT_BLUR_PARAMS, 0, 0, NULL,
	// params
	prm_displayPasses, _T("displayPasses"), TYPE_BOOL, P_ANIMATABLE, IDS_DISPLAY_PASSES,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DISPLAY_PASSES,
		end,
	prm_totalPasses, _T("totalPasses"), TYPE_INT, P_ANIMATABLE, IDS_TOTAL_PASSES,
		p_default, 12,
		p_range, 2, 1000,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDEB_TOTAL_PASSES, IDSP_TOTAL_PASSES, SPIN_AUTOSCALE,
		end,
	prm_duration, _T("duration"), TYPE_FLOAT, P_ANIMATABLE, IDS_DURATION,
		p_default, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDEB_DURATION, IDSP_DURATION, SPIN_AUTOSCALE,
		end,
	prm_bias, _T("bias"), TYPE_FLOAT, P_ANIMATABLE, IDS_BIAS,
		p_default, 0.5f,
		p_range, 0.01f, 0.99f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDEB_BIAS, IDSP_BIAS, SPIN_AUTOSCALE,
		end,
	prm_normalizeWeights, _T("normalizeWeights"), TYPE_BOOL, P_ANIMATABLE, IDS_NORMALIZE_WEIGHTS,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_NORMALIZE_WEIGHTS,
		end,
	prm_ditherStrength, _T("ditherStrength"), TYPE_FLOAT, P_ANIMATABLE, IDS_DITHER_STRENGTH,
		p_default, 0.4f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDEB_DITHER_STRENGTH, IDSP_DITHER_STRENGTH, SPIN_AUTOSCALE,
		end,
	prm_tileSize, _T("tileSize"), TYPE_INT, P_ANIMATABLE, IDS_TILE_SIZE,
		p_default, 32,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDEB_TILE_SIZE, IDSP_TILE_SIZE, SPIN_AUTOSCALE,
		end,
	prm_disableFiltering, _T("disableFiltering"), TYPE_BOOL, P_ANIMATABLE, IDS_DISABLE_FILTERING,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DISABLE_FILTERING,
		end,
	prm_disableAntialiasing, _T("disableAntialiasing"), TYPE_BOOL, P_ANIMATABLE, IDS_DISABLE_ANTIALIASING,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DISABLE_ANTIALIASING,
		end,
	end
	);


// -----------------------------------------
// color balance effect - method definitions
// -----------------------------------------
MultiPassMotionBlur::MultiPassMotionBlur() :
	mpPBlock(NULL), mTotalPasses(0), mbBlendTableInvalid(true), mfpNoiseFn(RandomNoise), mpBlendTable(NULL),
	mbIsScanRenderer(FALSE), mbDisableFiltering(FALSE), mbPrevFilter(FALSE), mbDisableAntialiasing(FALSE), mbPrevAntialias(FALSE),
	mpIScanRenderer(NULL)
{
	multiPassMotionBlurClassDesc.MakeAutoParamBlocks(this);
	assert(mpPBlock);
}

void
MultiPassMotionBlur::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	multiPassMotionBlurClassDesc.BeginEditParams(ip, this, flags, prev);
}

void
MultiPassMotionBlur::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	multiPassMotionBlurClassDesc.EndEditParams(ip, this, flags, next);
}

TSTR
MultiPassMotionBlur::SubAnimName(int i)
{
	switch (i)
	{
		case 0:
			return GetString(IDS_MP_MOT_BLUR_PARAMS);
		default:
			return _T("");
	}
}

RefTargetHandle
MultiPassMotionBlur::Clone(RemapDir &remap)
{
	MultiPassMotionBlur *pNewMultiPassMotionBlur = new MultiPassMotionBlur();	
	pNewMultiPassMotionBlur->ReplaceReference( 0, mpPBlock->Clone(remap) );
	BaseClone(this, pNewMultiPassMotionBlur, remap);
	return(pNewMultiPassMotionBlur);
}

RefTargetHandle
MultiPassMotionBlur::GetReference(int i)
{
	switch (i)
	{
		case 0:
			return mpPBlock;
		default:
			return NULL;
	}
}

void
MultiPassMotionBlur::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
		case 0:
			mpPBlock = (IParamBlock2*)rtarg;
			break;
	}
}

RefResult
MultiPassMotionBlur::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message)
	{
		case REFMSG_CHANGE:
		{
			if (hTarget == mpPBlock)
			{
				ParamID paramID = mpPBlock->LastNotifyParamID();
				switch (paramID)
				{
					case prm_totalPasses:
					case prm_normalizeWeights:
					case prm_ditherStrength:
					case prm_tileSize:
					{
						// invalidate blend table
						mbBlendTableInvalid = true;
						break;
					}

					case prm_displayPasses:
					case prm_duration:
					case prm_bias:
					case prm_disableFiltering:
					case prm_disableAntialiasing:
					{
						// do nothing
						break;
					}
				}
				multiPassMotionBlur_param_blk.InvalidateUI(paramID);
			}
			break;
		}
	}
	return REF_SUCCEED;
}

bool MultiPassMotionBlur::DisplayPasses(TimeValue renderTime)
{
	BOOL bDisplayPasses;
	mpPBlock->GetValue(prm_displayPasses, renderTime, bDisplayPasses, FOREVER);
	return (bDisplayPasses != FALSE);
}

int
MultiPassMotionBlur::TotalPasses(TimeValue renderTime)
{
	int oldTotalPasses = mTotalPasses;							// 5/2/01 11:20am --MQM-- 
	mpPBlock->GetValue(prm_totalPasses, renderTime, mTotalPasses, FOREVER);
 	LimitValue(mTotalPasses, 2, 1000);
	if ( mTotalPasses != oldTotalPasses )
		mbBlendTableInvalid = true;
	return mTotalPasses;
}

// Ken Perlin's bias function from "Texturing and Modeling, a Procedural Approach"
// this function lacks symmetry between values of b above and below 0.5
float perlinBias(float a, float b)
{
    return pow((double)a, log((double)b) / log(0.5));
}

// Christophe Schlick's bias function from Graphics Gems IV
// faster and symmetrical
float fastBias(float a, float b)
{
	if (a > 0.0f)
	{
		return b / ( (1.0f / a - 2.0f) * (1.0f - b) + 1.0f );
	}
	return 0.0f;
}

ViewParams *
MultiPassMotionBlur::Apply(INode *pCameraNode, CameraObject *pCameraObject, int passNum, TimeValue &overrideRenderTime)
{
	static float duration;
	static float bias;

	if (passNum == 0)
	{
		mpPBlock->GetValue(prm_totalPasses, overrideRenderTime, mTotalPasses, FOREVER);
		LimitValue(mTotalPasses, 2, 1000);

		mpPBlock->GetValue(prm_duration, overrideRenderTime, duration, FOREVER);
		// convert to ticks
		duration *= GetTicksPerFrame();
		mpPBlock->GetValue(prm_bias, overrideRenderTime, bias, FOREVER);
		// clamp to acceptable values - the paramblock2 range only clamps values typed into ui controls.
		// we must clamp here in case values have been changed in trackview or via script access
		LimitValue(bias, 0.01f, 0.99f);

		mpPBlock->GetValue(prm_disableFiltering, overrideRenderTime, mbDisableFiltering, FOREVER);
		mpPBlock->GetValue(prm_disableAntialiasing, overrideRenderTime, mbDisableAntialiasing, FOREVER);

		mbIsScanRenderer = ( GetCOREInterface()->GetCurrentRenderer()->ClassID() == Class_ID(SREND_CLASS_ID,0) );
		if (mbIsScanRenderer && (mbDisableFiltering || mbDisableAntialiasing) )
		{
			mpIScanRenderer = reinterpret_cast<IScanRenderer *>( GetCOREInterface()->GetCurrentRenderer() );
			if (mbDisableFiltering)
			{
				mbPrevFilter = mpIScanRenderer->GetFilter();
#ifndef WEBVERSION
				mpIScanRenderer->SetFilter(FALSE);
#endif
			}
			if (mbDisableAntialiasing)
			{
				mbPrevAntialias = mpIScanRenderer->GetAntialias();
				mpIScanRenderer->SetAntialias(FALSE);
			}
		}
	}

	// calc new time value
	// mTotalPasses sample points between (mTotalPasses-1) intervals of duration
	// newTime = origTime - (half of duration) + bias * duration
//	overrideRenderTime += duration * (perlinBias( bias, passNum * 1.0f/float(mTotalPasses-1) ) - 0.5f);
	overrideRenderTime += duration * (fastBias( bias, passNum * 1.0f/float(mTotalPasses-1) ) - 0.5f);

	return NULL;
}

void
MultiPassMotionBlur::AccumulateBitmap(Bitmap *pDest, Bitmap *pSrc, int passNum, TimeValue renderTime)
{
	if (mbBlendTableInvalid)
	{
		if (mpBlendTable)
		{
			delete mpBlendTable;
			mpBlendTable = NULL;
		}

		// to test various noise functions from the debugger
		static int noiseFnSelector(1);
		switch (noiseFnSelector)
		{
			case 0:
				mfpNoiseFn = dummyNoise;
				break;
			case 1:
				mfpNoiseFn = RandomNoise;
				break;
			case 2:
				mfpNoiseFn = SumNoise;
				break;
			case 3:
				mfpNoiseFn = CRCNoise;
				break;
			case 4:
				mfpNoiseFn = GaussNoise;
				break;
			default:
				mfpNoiseFn = RandomNoise;
				break;
		}

		int tileSize;
		mpPBlock->GetValue(prm_tileSize, renderTime, tileSize, FOREVER);

		float ditherStrength;
		mpPBlock->GetValue(prm_ditherStrength, renderTime, ditherStrength, FOREVER);

		BOOL normalizeWeights;
		mpPBlock->GetValue(prm_normalizeWeights, renderTime, normalizeWeights, FOREVER);

		mpBlendTable = new BlendTable(tileSize, tileSize, mTotalPasses, ditherStrength, normalizeWeights, mfpNoiseFn);
		mbBlendTableInvalid = false;
	}
	mpBlendTable->BlendImages(pDest, pSrc, passNum);
}

void
MultiPassMotionBlur::PostRenderFrame()
{
	if (mbIsScanRenderer && (mbDisableFiltering || mbDisableAntialiasing) )
	{
		if (mbDisableFiltering)
		{
#ifndef WEBVERSION
			mpIScanRenderer->SetFilter(mbPrevFilter);
#endif
		}
		if (mbDisableAntialiasing)
		{
			mpIScanRenderer->SetAntialias(mbPrevAntialias);
		}
	}
}


// ----------------------------
// end of file multiPassMotionBlur.cpp
// ----------------------------
