// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
//   FILE: blurMgr.cpp
//
//	 DESCRIPTION: BlurMgr - class definitions
//				  main interface to blur render effect
//
//	 CREATED BY: michael malone (mjm)
//
//	 HISTORY: created November 4, 1998
//
//	 Copyright (c) 1998, All Rights Reserved
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// precompiled header
#include "pch.h"

// system includes
#include  <math.h>

// local includes
#include "blurMgr.h"

// maxsdk includes
#include <iparamm2.h>
#include <iparamb2.h>
#include <bmmlib.h>
#include <sse.h>

// SSE global flag
BOOL gBlurSSE_Enabled = FALSE; // Martell 2/19/01: Add SSE Support
BOOL IsSSE();
// ---------------------
// class ID & descriptor
// ---------------------
const Class_ID BlurMgr::blurMgrClassID(0xd481816, 0x786d799c);
BlurMgrClassDesc blurMgrCD;
ClassDesc* GetBlurMgrDesc() { return &blurMgrCD; }

// -----------------
// paramMap pointers
// -----------------
IParamMap2 *BlurMgr::pmMaster   = NULL;
IParamMap2 *BlurMgr::pmBlurData = NULL;
IParamMap2 *BlurMgr::pmSelData  = NULL;

// ------------
// dialog procs
// ------------
MasterDlgProc	BlurMgr::masterDlgProc;
BlurDataDlgProc	BlurMgr::blurDataDlgProc(&masterDlgProc);
SelDataDlgProc	BlurMgr::selDataDlgProc(&masterDlgProc);

// ----------------------
// paramblock descriptors
// ----------------------
ParamBlockDesc2 BlurMgr::pbdMaster(idMaster, _T("blur effect parameters"), 0, &blurMgrCD, P_AUTO_CONSTRUCT + P_AUTO_UI, idMaster,
	//rollout
	IDD_MASTER, IDS_PARAMS, 0, 0, &masterDlgProc,
	// no params - used only for auto ui
	end
	);

ParamBlockDesc2 BlurMgr::pbdBlurData(idBlurData, _T("blurData"), 0, &blurMgrCD, P_AUTO_CONSTRUCT, idBlurData,
	// params
	prmBlurType, _T("blur_type"), TYPE_RADIOBTN_INDEX, P_ANIMATABLE, IDS_BLUR_TYPE,
		p_default, idBlurUnif,
		p_range, idBlurUnif, idBlurRadial,
		p_ui, TYPE_RADIO, 3, IDR_BLUR_UNIF, IDR_BLUR_DIR, IDR_BLUR_RADIAL,
		p_enable_ctrls,
			// IDR_BLUR_UNIF toggles:
			2, prmUnifPixRad, prmUnifAlpha,
			// IDR_BLUR_DIR toggles:
			6, prmDirUPixRad, prmDirVPixRad, prmDirUTrail, prmDirVTrail, prmDirRot, prmDirAlpha,
			// IDR_BLUR_RADIAL toggles:
			7, prmRadialPixRad, prmRadialTrail, prmRadialAlpha, prmRadialXOrig, prmRadialYOrig, prmRadialUseNode, prmRadialNode,
		end,
	// BlurUniform
	prmUnifPixRad, _T("bUnifPixRad"), TYPE_FLOAT, P_ANIMATABLE, IDS_BUNIF_PIXEL_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BUNIF_RAD, IDSP_BUNIF_RAD, SPIN_AUTOSCALE,
		end,
	prmUnifAlpha, _T("bUnifAlpha"), TYPE_BOOL, P_ANIMATABLE, IDS_BUNIF_ALPHA,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BUNIF_ALPHA,
		end,
	// BlurDirectional
	prmDirUPixRad, _T("bDirUPixRad"), TYPE_FLOAT, P_ANIMATABLE, IDS_BDIR_UPIX_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BDIR_URAD, IDSP_BDIR_URAD, SPIN_AUTOSCALE,
		end,
	prmDirVPixRad, _T("bDirVPixRad"), TYPE_FLOAT, P_ANIMATABLE, IDS_BDIR_VPIX_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BDIR_VRAD, IDSP_BDIR_VRAD, SPIN_AUTOSCALE,
		end,
	prmDirUTrail, _T("bDirUTrail"), TYPE_FLOAT, P_ANIMATABLE, IDS_BDIR_UTRAIL,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BDIR_UTRAIL, IDSP_BDIR_UTRAIL, SPIN_AUTOSCALE,
		end,
	prmDirVTrail, _T("bDirVTrail"), TYPE_FLOAT, P_ANIMATABLE, IDS_BDIR_VTRAIL,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BDIR_VTRAIL, IDSP_BDIR_VTRAIL, SPIN_AUTOSCALE,
		end,
	prmDirRot, _T("bDirRotation"), TYPE_INT, P_ANIMATABLE, IDS_BDIR_ROTATION,
		p_default, 0,
		p_range, -180, 180,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDE_BDIR_ROT, IDSP_BDIR_ROT, SPIN_AUTOSCALE,
		end,
	prmDirAlpha, _T("bDirAlpha"), TYPE_BOOL, P_ANIMATABLE, IDS_BDIR_ALPHA,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BDIR_ALPHA,
		end,
	// BlurRadial
	prmRadialPixRad, _T("bRadialPixRad"), TYPE_FLOAT, P_ANIMATABLE, IDS_BRADIAL_PIXEL_RAD,
		p_default, 20.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BRADIAL_RAD, IDSP_BRADIAL_RAD, SPIN_AUTOSCALE,
		end,
	prmRadialTrail, _T("bRadialTrail"), TYPE_FLOAT, P_ANIMATABLE, IDS_BRADIAL_TRAIL,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_BRADIAL_TRAIL, IDSP_BRADIAL_TRAIL, SPIN_AUTOSCALE,
		end,
	prmRadialAlpha, _T("bRadialAlpha"), TYPE_BOOL, P_ANIMATABLE, IDS_BRADIAL_ALPHA,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BRADIAL_ALPHA,
		end,
	prmRadialXOrig, _T("bRadialXOrig"), TYPE_INT, P_ANIMATABLE, IDS_BRADIAL_XORIGIN,
		p_range, -999999, 999999,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDE_BRADIAL_XORIGIN, IDSP_BRADIAL_XORIGIN, SPIN_AUTOSCALE,
		end,
	prmRadialYOrig, _T("bRadialYOrig"), TYPE_INT, P_ANIMATABLE, IDS_BRADIAL_YORIGIN,
		p_range, -999999, 999999,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDE_BRADIAL_YORIGIN, IDSP_BRADIAL_YORIGIN, SPIN_AUTOSCALE,
		end,
	prmRadialUseNode, _T("bRadialUseNode"), TYPE_BOOL, P_ANIMATABLE, IDS_BRADIAL_USE_NODE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BRADIAL_USE_NODE,
		end,
	prmRadialNode, _T("bRadialNode"), TYPE_INODE, 0, IDS_BRADIAL_NODE,	// not animatable
		p_ui, TYPE_PICKNODEBUTTON, IDB_BRADIAL_NODE,
		p_prompt, IDS_BRADIAL_NODE_PROMPT,
		end,
	end
	);

ParamBlockDesc2 BlurMgr::pbdSelData(idSelData, _T("selectionData"), 0, &blurMgrCD, P_AUTO_CONSTRUCT, idSelData,
	// params
	// SelImage
	prmImageActive, _T("selImageActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SIMAGE_ACTIVE,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SIMAGE_ACTIVE,
		p_enable_ctrls, 2, prmImageBrighten, prmImageBlend,
		end,
	prmImageBrighten, _T("selImageBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SIMAGE_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SIMAGE_BRIGHTEN, IDSP_SIMAGE_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmImageBlend, _T("selImageBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SIMAGE_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SIMAGE_BLEND, IDSP_SIMAGE_BLEND, SPIN_AUTOSCALE,
		end,
	// SelIBack
	prmIBackActive, _T("selIBackActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SIBACK_ACTIVE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SIBACK_ACTIVE,
		p_enable_ctrls, 3, prmIBackBrighten, prmIBackBlend, prmIBackFeathRad,
		end,
	prmIBackBrighten, _T("selIBackBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SIBACK_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SIBACK_BRIGHTEN, IDSP_SIBACK_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmIBackBlend, _T("selIBackBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SIBACK_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SIBACK_BLEND, IDSP_SIBACK_BLEND, SPIN_AUTOSCALE,
		end,
	prmIBackFeathRad, _T("selIBackFRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SIBACK_FEATHER_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SIBACK_FRAD, IDSP_SIBACK_FRAD, SPIN_AUTOSCALE,
		end,
	// SelLum
	prmLumActive, _T("selLumActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SLUM_ACTIVE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SLUM_ACTIVE,
		p_enable_ctrls, 5, prmLumBrighten, prmLumBlend, prmLumMin, prmLumMax, prmLumFeathRad,
		end,
	prmLumBrighten, _T("selLumBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SLUM_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SLUM_BRIGHTEN, IDSP_SLUM_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmLumBlend, _T("selLumBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SLUM_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SLUM_BLEND, IDSP_SLUM_BLEND, SPIN_AUTOSCALE,
		end,
	prmLumMin, _T("selLumMin"), TYPE_FLOAT, P_ANIMATABLE, IDS_SLUM_MIN,
		p_default, 90.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SLUM_MIN, IDSP_SLUM_MIN, SPIN_AUTOSCALE,
		end,
	prmLumMax, _T("selLumMax"), TYPE_FLOAT, P_ANIMATABLE, IDS_SLUM_MAX,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SLUM_MAX, IDSP_SLUM_MAX, SPIN_AUTOSCALE,
		end,
	prmLumFeathRad, _T("selLumFRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SLUM_FEATHER_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SLUM_FRAD, IDSP_SLUM_FRAD, SPIN_AUTOSCALE,
		end,
	// SelMask
	prmMaskActive, _T("selMaskActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SMASK_ACTIVE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SMASK_ACTIVE,
		p_enable_ctrls, 6, prmMaskMap, prmMaskBrighten, prmMaskBlend, prmMaskMin, prmMaskMax, prmMaskFeathRad,
		end,
	prmMaskMap, _T("selMaskMap"), TYPE_TEXMAP, 0, IDS_SMASK_MAP,
		p_ui, TYPE_TEXMAPBUTTON, IDB_SMASK_MAP,
		end,
	prmMaskChannel, _T("selMaskChannel"), TYPE_INT, P_ANIMATABLE, IDS_SMASK_CHANNEL,
		p_default, 4,
		end,
	prmMaskBrighten, _T("selMaskBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMASK_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMASK_BRIGHTEN, IDSP_SMASK_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmMaskBlend, _T("selMaskBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMASK_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMASK_BLEND, IDSP_SMASK_BLEND, SPIN_AUTOSCALE,
		end,
	prmMaskMin, _T("selMaskMin"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMASK_MIN,
		p_default, 90.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMASK_MIN, IDSP_SMASK_MIN, SPIN_AUTOSCALE,
		end,
	prmMaskMax, _T("selMaskMax"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMASK_MAX,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMASK_MAX, IDSP_SMASK_MAX, SPIN_AUTOSCALE,
		end,
	prmMaskFeathRad, _T("selMaskFRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMASK_FEATHER_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMASK_FRAD, IDSP_SMASK_FRAD, SPIN_AUTOSCALE,
		end,
	// SelObjIds
	prmObjIdsActive, _T("selObjIdsActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SOBJIDS_ACTIVE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SOBJIDS_ACTIVE,
		p_enable_ctrls, 6, prmObjIdsIds, prmObjIdsBrighten, prmObjIdsBlend, prmObjIdsFeathRad, prmObjIdsLumMin, prmObjIdsLumMax,
		end,
	prmObjIdsIds, _T("selObjIdsIds"), TYPE_INT_TAB, 0, P_ANIMATABLE | P_VARIABLE_SIZE, IDS_SOBJIDS_IDS,
		p_range, 0, USHRT_MAX,
		p_ui, TYPE_INTLISTBOX, IDLB_SOBJIDS, IDB_SOBJIDS_ADD, IDB_SOBJIDS_REPLACE, IDB_SOBJIDS_DELETE, EDITTYPE_INT, IDE_SOBJIDS_NEWVAL, IDSP_SOBJIDS_NEWVAL, SPIN_AUTOSCALE,
		end,
	prmObjIdsBrighten, _T("selObjIdsBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SOBJIDS_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SOBJIDS_BRIGHTEN, IDSP_SOBJIDS_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmObjIdsBlend, _T("selObjIdsBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SOBJIDS_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SOBJIDS_BLEND, IDSP_SOBJIDS_BLEND, SPIN_AUTOSCALE,
		end,
	prmObjIdsFeathRad, _T("selObjIdsFRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SOBJIDS_FEATHER_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SOBJIDS_FRAD, IDSP_SOBJIDS_FRAD, SPIN_AUTOSCALE,
		end,
	prmObjIdsLumMin, _T("selObjIdsLumMin"), TYPE_FLOAT, P_ANIMATABLE, IDS_SOBJIDS_LUM_MIN,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SOBJIDS_LUM_MIN, IDSP_SOBJIDS_LUM_MIN, SPIN_AUTOSCALE,
		end,
	prmObjIdsLumMax, _T("selObjIdsLumMax"), TYPE_FLOAT, P_ANIMATABLE, IDS_SOBJIDS_LUM_MAX,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SOBJIDS_LUM_MAX, IDSP_SOBJIDS_LUM_MAX, SPIN_AUTOSCALE,
		end,
	// SelMatIds
	prmMatIdsActive, _T("selMatIdsActive"), TYPE_BOOL, P_ANIMATABLE, IDS_SMATIDS_ACTIVE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SMATIDS_ACTIVE,
		p_enable_ctrls, 6, prmMatIdsIds, prmMatIdsBrighten, prmMatIdsBlend, prmMatIdsFeathRad, prmMatIdsLumMin, prmMatIdsLumMax,
		end,
	prmMatIdsIds, _T("selMatIdsIds"), TYPE_INT_TAB, 0, P_ANIMATABLE | P_VARIABLE_SIZE, IDS_SMATIDS_IDS,
		p_range, 0, 255,
		p_ui, TYPE_INTLISTBOX, IDLB_SMATIDS, IDB_SMATIDS_ADD, IDB_SMATIDS_REPLACE, IDB_SMATIDS_DELETE, EDITTYPE_INT, IDE_SMATIDS_NEWVAL, IDSP_SMATIDS_NEWVAL, SPIN_AUTOSCALE,
		end,
	prmMatIdsBrighten, _T("selMatIdsBrighten"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMATIDS_BRIGHTEN,
		p_default, 0.0f,
		p_range, 0.0f, 9999.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMATIDS_BRIGHTEN, IDSP_SMATIDS_BRIGHTEN, SPIN_AUTOSCALE,
		end,
	prmMatIdsBlend, _T("selMatIdsBlend"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMATIDS_BLEND,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMATIDS_BLEND, IDSP_SMATIDS_BLEND, SPIN_AUTOSCALE,
		end,
	prmMatIdsFeathRad, _T("selMatIdsFRadius"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMATIDS_FEATHER_RAD,
		p_default, 10.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMATIDS_FRAD, IDSP_SMATIDS_FRAD, SPIN_AUTOSCALE,
		end,
	prmMatIdsLumMin, _T("selMatIdsLumMin"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMATIDS_LUM_MIN,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMATIDS_LUM_MIN, IDSP_SMATIDS_LUM_MIN, SPIN_AUTOSCALE,
		end,
	prmMatIdsLumMax, _T("selMatIdsLumMax"), TYPE_FLOAT, P_ANIMATABLE, IDS_SMATIDS_LUM_MAX,
		p_default, 100.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDE_SMATIDS_LUM_MAX, IDSP_SMATIDS_LUM_MAX, SPIN_AUTOSCALE,
		end,
	prmGenBrightType, _T("selGenBrightType"), TYPE_RADIOBTN_INDEX, P_ANIMATABLE, IDS_SEL_BRIGHT_TYPE,
		p_default, idBrightenMult,
		p_range, idBrightenAdd, idBrightenMult,
		p_ui, TYPE_RADIO, 2, IDR_SEL_BRT_ADD, IDR_SEL_BRT_MULT,
		end,
	end
	);


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// class constructor

BlurMgr::BlurMgr() :
	pbMaster(NULL), pbBlurData(NULL), pbSelData(NULL), mp_CCtl(NULL),
	m_lastBMModifyID(0xFFFFFFFF), m_imageW(0), m_imageH(0), m_imageSz(0), m_compValid(false)

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
	m_blurs[idBlurUnif]		= new BlurUniform(this);
	m_blurs[idBlurDir]		= new BlurDirectional(this);
	m_blurs[idBlurRadial]	= new BlurRadial(this);

	m_sels[idSelImage]		= new SelImage(this);
	m_sels[idSelIBack]		= new SelIgnBack(this);
	m_sels[idSelLum]		= new SelLum(this);
	m_sels[idSelMask]		= new SelMaps(this);
	m_sels[idSelObjIds]		= new SelObjIds(this);
	m_sels[idSelMatIds]		= new SelMatIds(this);

	// this added to allow extension of seltypes -- adding new param active ids breaks the original sequential enumeration
	// i should have enumerated the param ids sparsely to allow for extensibility.
	// if they are added sequentially, it changes the value of old param ids and will break loading of old versions
	m_selActiveIds[idSelImage]	= prmImageActive;
	m_selActiveIds[idSelIBack]	= prmIBackActive;
	m_selActiveIds[idSelLum]	= prmLumActive;
	m_selActiveIds[idSelMask]	= prmMaskActive;
	m_selActiveIds[idSelObjIds]	= prmObjIdsActive;
	m_selActiveIds[idSelMatIds]	= prmMatIdsActive;

	// set default blur radial origin to coincide with current render settings
	pbdBlurData.ParamOption(prmRadialXOrig, p_default, GetCOREInterface()->GetRendWidth()/2);
	pbdBlurData.ParamOption(prmRadialYOrig, p_default, GetCOREInterface()->GetRendHeight()/2);
	blurMgrCD.MakeAutoParamBlocks(this);
	assert(	pbMaster && pbBlurData && pbSelData);

	// create a curve control and reference it
	ICurveCtl *pICurveCtl = (ICurveCtl*)CreateInstance(REF_MAKER_CLASS_ID, CURVE_CONTROL_CLASS_ID);
	assert (pICurveCtl);
	ReplaceReference(idSelCurveCtrl, pICurveCtl);
	pICurveCtl->RegisterResourceMaker(this);
	pICurveCtl->SetCCFlags(CC_DRAWUTOOLBAR | CC_CONSTRAIN_Y | CC_SHOWRESET/*| CC_RCMENU_MOVE_XY | CC_RCMENU_MOVE_X | CC_RCMENU_MOVE_Y | CC_RCMENU_SCALE | CC_RCMENU_INSERT_CORNER | CC_RCMENU_INSERT_BEZIER | CC_RCMENU_DELETE*/);
	pICurveCtl->SetXRange(0.0f,1.0f);
	pICurveCtl->SetYRange(0.0f,1.0f);
	BitArray ba;
	ba.SetSize(32);
	ba.Set(0);
	ba.Clear(1);
	pICurveCtl->SetDisplayMode(ba);
	pICurveCtl->SetNumCurves(2);
	pICurveCtl->SetScrollValues(2, -44);
	pICurveCtl->SetZoomValues(133, 117);
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// class destructor

BlurMgr::~BlurMgr()

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
	for (int i=0; i<numBlurs; i++)
		if (m_blurs[i])
			delete m_blurs[i];
	for (i=0; i<numSels; i++)
		if (m_sels[i])
			delete m_sels[i];
	DeleteAllRefs();
//	if (mp_CCtl)	// Should be deleted by DeleteAllRefs
//	{
//		mp_CCtl->DeleteThis();
//	}
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// class destructor

IOResult BlurMgr::Load(ILoad *iload)

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *BlurMgr::CreateParamDialog(IRendParams *ip)
{	
    // create the master rollout dialog
    IAutoEParamDlg* masterDlg = blurMgrCD.CreateParamDialog(pbMaster->ID(), ip, this);
	pmMaster = masterDlg->GetMap();

	// create the blur types child dialogs
	pmBlurData = CreateChildRParamMap2(pbBlurData, ip, hInstance,
									   pmMaster, MAKEINTRESOURCE(IDD_BLUR_TYPES),
									   GetString(IDS_BLUR_TYPE), &blurDataDlgProc);

	// create the compositing selections child dialogs
	pmSelData = CreateChildRParamMap2(pbSelData, ip, hInstance,
									  pmMaster, MAKEINTRESOURCE(IDD_SEL_TYPES),
									  GetString(IDS_SEL_TYPES), &selDataDlgProc);

	// tell the master dialog we're done adding children
	masterDlgProc.notifyChildrenCreated(this);

    return masterDlg;
}

DWORD BlurMgr::GBufferChannelsRequired(TimeValue t)
{
	BOOL selActive;
	DWORD channels(BMM_CHAN_NONE);

	pbSelData->GetValue(prmObjIdsActive, t, selActive, FOREVER);
	if (selActive)
		channels |= BMM_CHAN_NODE_ID | BMM_CHAN_COLOR;

	pbSelData->GetValue(prmMatIdsActive, t, selActive, FOREVER);
	if (selActive)
		channels |= BMM_CHAN_MTL_ID | BMM_CHAN_COLOR;

	return channels;
}

TSTR BlurMgr::SubAnimName(int i)
{
	switch (i)
	{
	case idMaster:
		return GetString(IDS_BLUR_PARAMS);
	case idBlurData:
		return GetString(IDS_BLUR_TYPE);
	case idSelData:
		return GetString(IDS_SEL_TYPES);
	case idSelCurveCtrl:
		return GetString(IDS_SEL_FALLOFF_CURVE);
	default:
		return _T("");
	}
}

RefTargetHandle BlurMgr::GetReference(int i)
{
	switch (i)
	{
	case idMaster:
		return pbMaster;
	case idBlurData:
		return pbBlurData;
	case idSelData:
		return pbSelData;
	case idSelCurveCtrl:
		return mp_CCtl;
	default:
		return NULL;
	}
}

void BlurMgr::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case idMaster:
		pbMaster = (IParamBlock2*)rtarg;
		break;
	case idBlurData:
		pbBlurData = (IParamBlock2*)rtarg;
		break;
	case idSelData:
		pbSelData = (IParamBlock2*)rtarg;
		break;
	case idSelCurveCtrl:
		mp_CCtl = (ICurveCtl*)rtarg;
		break;
	}
}

RefResult BlurMgr::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	int prmID;

	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget == pbMaster)
			{
				pbdMaster.InvalidateUI(pbMaster->LastNotifyParamID());
				return REF_SUCCEED;
			}

			// blur types
			if (hTarget == pbBlurData)
			{
				switch (prmID = pbBlurData->LastNotifyParamID())
				{
					case prmUnifPixRad:
					case prmUnifAlpha:
						m_blurs[idBlurUnif]->notifyPrmChanged(prmID);
						break;
					case prmDirUPixRad:
					case prmDirVPixRad:
					case prmDirUTrail:
					case prmDirVTrail:
					case prmDirRot:
					case prmDirAlpha:
						m_blurs[idBlurDir]->notifyPrmChanged(prmID);
						break;
					case prmRadialPixRad:
					case prmRadialTrail:
					case prmRadialAlpha:
					case prmRadialXOrig:
					case prmRadialYOrig:
					case prmRadialUseNode:
					case prmRadialNode:
						m_blurs[idBlurRadial]->notifyPrmChanged(prmID);
						break;
				}
				IParamMap2 *map = pbBlurData->GetMap();
				if (map)
					map->Invalidate(prmID);
				return REF_SUCCEED;
			}

			// selection types
			if (hTarget == pbSelData)
			{
				switch (prmID = pbSelData->LastNotifyParamID())
				{
					case prmImageActive:
					case prmImageBrighten:
					case prmImageBlend:
						m_sels[idSelImage]->notifyPrmChanged(prmID);
						break;
					case prmIBackActive:
					case prmIBackBrighten:
					case prmIBackBlend:
					case prmIBackFeathRad:
						m_sels[idSelIBack]->notifyPrmChanged(prmID);
						break;
					case prmLumActive:
					case prmLumBrighten:
					case prmLumBlend:
					case prmLumMin:
					case prmLumMax:
					case prmLumFeathRad:
						m_sels[idSelLum]->notifyPrmChanged(prmID);
						break;
					case prmMaskActive:
					case prmMaskMap:
					case prmMaskChannel:
					case prmMaskBrighten:
					case prmMaskBlend:
					case prmMaskMin:
					case prmMaskMax:
					case prmMaskFeathRad:
						m_sels[idSelMask]->notifyPrmChanged(prmID);
						break;
					case prmObjIdsActive:
					case prmObjIdsIds:
					case prmObjIdsBrighten:
					case prmObjIdsBlend:
					case prmObjIdsFeathRad:
					case prmObjIdsLumMin:
					case prmObjIdsLumMax:
						m_sels[idSelObjIds]->notifyPrmChanged(prmID);
						break;
					case prmMatIdsActive:
					case prmMatIdsIds:
					case prmMatIdsBrighten:
					case prmMatIdsBlend:
					case prmMatIdsFeathRad:
					case prmMatIdsLumMin:
					case prmMatIdsLumMax:
						m_sels[idSelMatIds]->notifyPrmChanged(prmID);
						break;
				}
				IParamMap2 *map = pbSelData->GetMap();
				if (map)
					map->Invalidate(prmID);
				return REF_SUCCEED;
			}
	}
	return REF_SUCCEED;
}

void BlurMgr::updateSelections(TimeValue t, Bitmap *bm, RenderGlobalContext *gc)
{
	BOOL selActive;
	for (int i=0; i<numSels; i++)
	{
		pbSelData->GetValue(m_selActiveIds[i], t, selActive, FOREVER); // check selection's activity
		if (!m_compValid)
			m_activeSels[i] = selActive; // record activity
		else if (m_activeSels[i] == selActive) // activity hasn't changed
		{
			if (m_activeSels[i]) // if active
				m_compValid = m_sels[i]->checkValid( bm->GetModifyID() ); // check if its valid internally
		}
		else // activity has changed
		{
			m_activeSels[i] = selActive;
			m_compValid = false;
		}
	}

	// rebuild the compositing map
	if (!m_compValid)
	{
		if ( m_activeSels[idSelImage] ) // whole image selection
		{
			float brighten = m_sels[idSelImage]->getBrighten(t);
			float blend = m_sels[idSelImage]->getBlend(t);
			m_compMap.set(brighten, blend, m_imageSz);
		}
		else
		{
			m_compMap.set(0.0f, 0.0f, m_imageSz);
		}

		for (int i=1; i<numSels; i++)
			if (m_activeSels[i])
				m_sels[i]->select(t, m_compMap, bm, gc);
	}
}

void BlurMgr::blur(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, BOOL doComposite)
{
	int activeBlur;
	pbBlurData->GetValue(prmBlurType, t, activeBlur, FOREVER);
	m_blurs[activeBlur]->blur(t, ((doComposite) ? &m_compMap : NULL), bm, gc);
}

void BlurMgr::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *_checkAbort)
{
	m_checkAbort = _checkAbort;
	int type;
	WORD *map = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);


	if(!map)
	{
		TSTR s1(GetString(IDS_STORAGE_ERROR_CONTENT));
		TSTR s2(GetString(IDS_STORAGE_ERROR_TITLE));
		MessageBox(NULL, s1.data(), s2.data(), MB_OK|MB_ICONEXCLAMATION);
		return;
	}




	if(gc && gc->renderer)
	{
		//Check for SSE support  
		IRenderSSEControl* rSSE = (IRenderSSEControl *)( gc->renderer->GetInterface( IRenderSSEControl::IID ) );

		if(rSSE && rSSE->IsSSEEnabled() && IsSSE())
			gBlurSSE_Enabled = TRUE;
		else
			gBlurSSE_Enabled = FALSE;
	}
	else
	{
		if(IsSSE())
			gBlurSSE_Enabled = TRUE;
		else
			gBlurSSE_Enabled = FALSE;
	}



	// check for active selections
	BOOL selsActive = false;
	for (int i=0; (!selsActive && i<numSels); i++)
		pbSelData->GetValue(m_selActiveIds[i], t, selsActive, FOREVER);

	if (!selsActive)
		return;

	// if source bitmap has changed since last call
	if ( m_lastBMModifyID != bm->GetModifyID() )
	{
		m_lastBMModifyID = bm->GetModifyID();
		m_compValid = false;
		if ( (bm->Width() != m_imageW) || (bm->Height() != m_imageH) )
		{
			m_imageW = bm->Width();
			m_imageH = bm->Height();
			m_compMap.resize(m_imageSz = m_imageW * m_imageH);
		}
	}

	updateSelections(t, bm, gc);
	blur(t, bm, gc, selsActive);
	return;
}

void *BlurMgr::GetInterface(ULONG id)
{
	if (id == I_RESMAKER_INTERFACE)
		return (ResourceMakerCallback *)this;
	else
		return (void *)NULL;
	}

void BlurMgr::ResetCallback(int curvenum, ICurveCtl *pCCtl)
{
	NewCurveCreatedCallback(curvenum, pCCtl);
	pCCtl->Redraw();
}

	
void BlurMgr::NewCurveCreatedCallback(int curvenum, ICurveCtl *pCCtl)
{
	assert (pCCtl == mp_CCtl);

	ICurve *pCurve = mp_CCtl->GetControlCurve(curvenum);
	if(pCurve)
	{
		pCurve->SetNumPts(2);
		CurvePoint pt = pCurve->GetPoint(0, 0);
		pt.p.y = 1.0f;
		pt.flags |= CURVEP_LOCKED_X;
		pCurve->SetPoint(0, 0, &pt, FALSE);
		
		pt = pCurve->GetPoint(0, 1);
		pt.p.y = 0.0f;
		pt.flags |= CURVEP_LOCKED_X;
		pCurve->SetPoint(0, 1, &pt);
		pCurve->SetLookupTableSize(100);
		pCurve->SetPenProperty( (curvenum) ? RGB(0,255,0) : RGB(255,0,0) );
	}
}
