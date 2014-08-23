/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: multiPassDOF.cpp

	 DESCRIPTION: definitions for class MultiPassDOF

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
	multiPassDOF_params,
};

// parameters for multiPassDOF_params
enum
{
	prm_useTargetDistance,
	prm_focalDepth,
	prm_displayPasses,
	prm_totalPasses,
	prm_sampleRadius,
	prm_sampleBias,
	prm_normalizeWeights,
	prm_ditherStrength,
	prm_tileSize,
	prm_disableFiltering,
	prm_disableAntialiasing,
	prm_useOriginalLocation,
};

// global instance
static const Class_ID multiPassDOFClassID(0xd481815, 0x687d799c);


// ----------------------------------------------
// multiPassDOF camera effect - class declaration
// ----------------------------------------------
class MultiPassDOF : public IMultiPassCameraEffect
{
	IParamBlock2* mpPBlock;
	int mTotalPasses;
	bool mbBlendTableInvalid;
	TNoiseFn mfpNoiseFn;
	BlendTable *mpBlendTable;
	BOOL mbIsScanRenderer, mbDisableFiltering, mbPrevFilter, mbDisableAntialiasing, mbPrevAntialias;
	IScanRenderer *mpIScanRenderer;

public:

	MultiPassDOF();
	~MultiPassDOF() { }

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
	void GetClassName(TSTR& s) { s = GetString(IDS_MP_DOF); }
	Class_ID ClassID() { return multiPassDOFClassID; }
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
// MultiPassDOF class descriptor - class declaration
// -------------------------------------------------
class MultiPassDOFClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MultiPassDOF; }
	const TCHAR *	ClassName() { return GetString(IDS_MP_DOF); }
	SClass_ID		SuperClassID() { return MPASS_CAM_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return multiPassDOFClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("multiPassDOF"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static MultiPassDOFClassDesc multiPassDOFClassDesc;
// external access function (through dllMain.h)
ClassDesc* GetMultiPassDOFClassDesc() { return &multiPassDOFClassDesc; }

class MultiPassDOFDialogHandler : public ParamMap2UserDlgProc {
public:
	virtual BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void DeleteThis() { }
};

BOOL MultiPassDOFDialogHandler::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, 
										UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG: {
		BOOL enb = map->GetParamBlock()->GetInt(prm_useTargetDistance, t) == 0;
		EnableWindow(GetDlgItem(hWnd, IDEB_FOCAL_DEPTH), enb);
		EnableWindow(GetDlgItem(hWnd, IDSP_FOCAL_DEPTH), end);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USE_TARGET_DIST:
			BOOL enb = map->GetParamBlock()->GetInt(prm_useTargetDistance, t) == 0;
			EnableWindow(GetDlgItem(hWnd, IDEB_FOCAL_DEPTH), enb);
			EnableWindow(GetDlgItem(hWnd, IDSP_FOCAL_DEPTH), end);
			break;
		}
		break;
	}
	return FALSE;
}

MultiPassDOFDialogHandler dlgHandler;

// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 multiPassDOF_param_blk(multiPassDOF_params, _T("multiPassDOF parameters"), 0, &multiPassDOFClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_MP_DOF_CAM_EFFECT, IDS_MP_DOF_PARAMS, 0, 0, &dlgHandler,
	// params
	prm_useTargetDistance, _T("useTargetDistance"), TYPE_BOOL, P_ANIMATABLE, IDS_USE_TARGET_DIST,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_TARGET_DIST,
		end,
	prm_focalDepth, _T("focalDepth"), TYPE_FLOAT, P_ANIMATABLE, IDS_FOCAL_DEPTH,
		p_default, 100.0f,
		p_range, 0.0f, 9999999.0f,
   		p_dim, 			stdWorldDim,
		p_ui, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDEB_FOCAL_DEPTH, IDSP_FOCAL_DEPTH, SPIN_AUTOSCALE,
		end,
	prm_displayPasses, _T("displayPasses"), TYPE_BOOL, P_ANIMATABLE, IDS_DISPLAY_PASSES,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DISPLAY_PASSES,
		end,
	prm_useOriginalLocation, _T("useOriginalLocation"), TYPE_BOOL, P_ANIMATABLE, IDS_USE_ORIG_LOCATION,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_ORIG_LOCATION,
		end,
	prm_totalPasses, _T("totalPasses"), TYPE_INT, P_ANIMATABLE, IDS_TOTAL_PASSES,
		p_default, 12,
		p_range, 2, 1000,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDEB_TOTAL_PASSES, IDSP_TOTAL_PASSES, SPIN_AUTOSCALE,
		end,
	prm_sampleRadius, _T("sampleRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SAMPLE_RADIUS,
		p_default, 1.0f,
		p_range, 0.0f, 999999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDEB_SAMPLE_RADIUS, IDSP_SAMPLE_RADIUS, SPIN_AUTOSCALE,
		p_dim, 			stdWorldDim,
		end,
	prm_sampleBias, _T("sampleBias"), TYPE_FLOAT, P_ANIMATABLE, IDS_SAMPLE_BIAS,
		p_default, 0.5f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDEB_SAMPLE_BIAS, IDSP_SAMPLE_BIAS, SPIN_AUTOSCALE,
		p_range, 0.0f, 1.0f,
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
MultiPassDOF::MultiPassDOF() :
	mpPBlock(NULL), mTotalPasses(0), mbBlendTableInvalid(true), mfpNoiseFn(RandomNoise), mpBlendTable(NULL),
	mbIsScanRenderer(FALSE), mbDisableFiltering(FALSE), mbPrevFilter(FALSE), mbDisableAntialiasing(FALSE), mbPrevAntialias(FALSE),
	mpIScanRenderer(NULL)
{
	multiPassDOFClassDesc.MakeAutoParamBlocks(this);
	assert(mpPBlock);
}

void
MultiPassDOF::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	multiPassDOFClassDesc.BeginEditParams(ip, this, flags, prev);
}

void
MultiPassDOF::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	multiPassDOFClassDesc.EndEditParams(ip, this, flags, next);
}

TSTR
MultiPassDOF::SubAnimName(int i)
{
	switch (i)
	{
		case 0:
			return GetString(IDS_MP_DOF_PARAMS);
		default:
			return _T("");
	}
}

RefTargetHandle
MultiPassDOF::Clone(RemapDir &remap)
{
	MultiPassDOF *pNewMultiPassDOF = new MultiPassDOF();	
	pNewMultiPassDOF->ReplaceReference( 0, mpPBlock->Clone(remap) );
	BaseClone(this, pNewMultiPassDOF, remap);
	return(pNewMultiPassDOF);
}

RefTargetHandle
MultiPassDOF::GetReference(int i)
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
MultiPassDOF::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
		case 0:
			mpPBlock = (IParamBlock2*)rtarg;
			break;
	}
}

RefResult
MultiPassDOF::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
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

					case prm_useTargetDistance:
					case prm_focalDepth:
					case prm_displayPasses:
					case prm_useOriginalLocation:
					case prm_sampleRadius:
					case prm_sampleBias:
					case prm_disableFiltering:
					case prm_disableAntialiasing:
					{
						// do nothing
						break;
					}
				}
				multiPassDOF_param_blk.InvalidateUI(paramID);
			}
			break;
		}
	}
	return REF_SUCCEED;
}

bool MultiPassDOF::DisplayPasses(TimeValue renderTime)
{
	BOOL bDisplayPasses;
	mpPBlock->GetValue(prm_displayPasses, renderTime, bDisplayPasses, FOREVER);
	return (bDisplayPasses != FALSE);
}

int
MultiPassDOF::TotalPasses(TimeValue renderTime)
{
	int oldTotalPasses = mTotalPasses;
	mpPBlock->GetValue(prm_totalPasses, renderTime, mTotalPasses, FOREVER);
	LimitValue(mTotalPasses, 2, 1000);
	if ( mTotalPasses != oldTotalPasses )						// 5/2/01 12:51pm --MQM-- need to update the table if # passes changed
		mbBlendTableInvalid = true;
	return mTotalPasses;
}

ViewParams *
MultiPassDOF::Apply(INode *pCameraNode, CameraObject *pCameraObject, int passNum, TimeValue &overrideRenderTime)
{
	static CameraState cameraState;
	static Matrix3 originalCameraTM;
	static Point3 originalCameraLocation;
	static Point3 originalCameraUVector;
	static Point3 originalCameraVVector;
	static Point3 originalCameraNVector;
	static BOOL useOriginalLocation(TRUE);
	static BOOL useTargetDistance(FALSE);
	static float focalDepth(0.0f);
	static Point3 targetLocation;
	static float sampleRadius;
	static float sampleBias;
	static ViewParams viewParams;
	static const float VIEW_DEFAULT_WIDTH(400.0f);
	static const float BIGFLOAT(1.0e30f);

	if (passNum == 0)
	{
		mbBlendTableInvalid = true; // new weights at each rendered frame to avoid 'screen door' dithering

		pCameraObject->EvalCameraState(overrideRenderTime, FOREVER, &cameraState);
		originalCameraTM = pCameraNode->GetObjTMBeforeWSM(overrideRenderTime);
		originalCameraUVector = originalCameraTM.GetRow(0).Unify();
		originalCameraVVector = originalCameraTM.GetRow(1).Unify();
		originalCameraNVector = originalCameraTM.GetRow(2).Unify();
		originalCameraLocation = originalCameraTM.GetRow(3);

		mpPBlock->GetValue(prm_useOriginalLocation, overrideRenderTime, useOriginalLocation, FOREVER);
		mpPBlock->GetValue(prm_useTargetDistance, overrideRenderTime, useTargetDistance, FOREVER);
		if (useTargetDistance)
		{
			if ( pCameraNode->GetTarget() )
			{
				focalDepth = Length(pCameraNode->GetTarget()->GetObjTMBeforeWSM(overrideRenderTime).GetTrans() - originalCameraLocation);
			}
			else
			{
				focalDepth = pCameraObject->GetTDist(overrideRenderTime);
			}
		}
		else
		{
			mpPBlock->GetValue(prm_focalDepth, overrideRenderTime, focalDepth, FOREVER);
		}
		targetLocation = originalCameraLocation - ( originalCameraNVector * focalDepth );

		mpPBlock->GetValue(prm_totalPasses, overrideRenderTime, mTotalPasses, FOREVER);
		LimitValue(mTotalPasses, 2, 1000);
		mpPBlock->GetValue(prm_sampleRadius, overrideRenderTime, sampleRadius, FOREVER);
		mpPBlock->GetValue(prm_sampleBias, overrideRenderTime, sampleBias, FOREVER);
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

	int totalSegments = (useOriginalLocation) ? mTotalPasses - 1 : mTotalPasses;
	int segment = (useOriginalLocation) ? passNum - 1 : passNum;

	if (!useOriginalLocation || passNum > 0)
	{
		// calc new camera position
		Point3 newCameraLocation = originalCameraLocation;
		float theta = float(segment) / float(totalSegments) * TWOPI;
		float radius = (segment%2) ? sampleRadius * sampleBias : sampleRadius;
		float uOffset = radius * cos(theta);
		float vOffset = radius * sin(theta);
		newCameraLocation += uOffset * originalCameraUVector;
		newCameraLocation += vOffset * originalCameraVVector;

		// calc new orientation vectors
		Point3 newNVector = (newCameraLocation - targetLocation).Unify();
		Point3 newUVector = CrossProd(originalCameraVVector, newNVector).Unify();
		Point3 newVVector = CrossProd(newNVector, newUVector).Unify();

		viewParams.affineTM.Set(newUVector, newVVector, newNVector, newCameraLocation);
	}
	else if (useOriginalLocation && passNum == 0)
	{
		viewParams.affineTM = originalCameraTM;
	}
	viewParams.prevAffineTM = viewParams.affineTM;

	viewParams.affineTM = Inverse(viewParams.affineTM);
	viewParams.prevAffineTM = Inverse(viewParams.prevAffineTM);

	viewParams.projType = cameraState.isOrtho ? PROJ_PARALLEL : PROJ_PERSPECTIVE;
	if (cameraState.manualClip)
	{
		viewParams.hither = cameraState.hither;
		viewParams.yon = cameraState.yon;
	}
	else
	{
		viewParams.hither = 0.1f;
		viewParams.yon = -BIGFLOAT;  // so  it doesn't get used
	}
	viewParams.distance = focalDepth;

	if (cameraState.isOrtho)
	{
//		viewParams.zoom = 2.0f * cameraState.tdist * (float)tan(cameraState.fov / 2.0) / VIEW_DEFAULT_WIDTH;
		viewParams.zoom = 2.0f * focalDepth * (float)tan(pCameraObject->GetFOV(overrideRenderTime) / 2.0) / VIEW_DEFAULT_WIDTH;
	}
	else
	{
//		viewParams.fov = cameraState.fov;
		viewParams.fov = pCameraObject->GetFOV(overrideRenderTime);
	}
	viewParams.nearRange = cameraState.nearRange;
	viewParams.farRange = cameraState.farRange;
	return &viewParams;
}

void
MultiPassDOF::AccumulateBitmap(Bitmap *pDest, Bitmap *pSrc, int passNum, TimeValue renderTime)
{
	if (mbBlendTableInvalid)
	{
		if (mpBlendTable)
		{
			delete mpBlendTable;
			mpBlendTable = NULL;
		}

		// to allow various noise functions from the debugger
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
MultiPassDOF::PostRenderFrame()
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
// end of file multiPassDOF.cpp
// ----------------------------
