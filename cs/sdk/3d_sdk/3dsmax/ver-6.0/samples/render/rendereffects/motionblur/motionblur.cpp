/* -----------------------------------------------------------------------------

   FILE: motionBlur.cpp

	 DESCRIPTION: Image(velocity) motion blur

	 CREATED BY: dan silva (dds)

	 HISTORY: created 3/3/2000

   	 Copyright (c) 2000, All Rights Reserved

// -------------------------------------------------------------------------- */

// local headers
#include "dllMain.h"

// sdk headers
#include <iparamm2.h>
#include <bmmlib.h>

// IDs to references
#define PBLOCK_REF 0

// parameter blocks IDs
enum { motBlur_params };

// parameters for motBlur_params
enum { prm_dur, prm_transp };

// global instance
static const Class_ID motBlurClassID(MBLUR_CLASS_ID);


// ----------------------------------------
// Motion blur effect - class declaration
// ----------------------------------------
class MotionBlur: public Effect
{
public:
	// parameters
	IParamBlock2* pblock;
	float duration;

	MotionBlur();
	~MotionBlur() { }

	// Animatable/Reference
	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	Class_ID ClassID() { return motBlurClassID; }
	void GetClassName(TSTR& s) { s = ::GetString(IDS_MOT_BLUR); }
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; } // only one
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }
	IOResult Load(ILoad *iload);

	// Effect
	TSTR GetName() { return ::GetString(IDS_MOT_BLUR); }
	EffectParamDlg *CreateParamDialog(IRendParams *ip);
	DWORD GBufferChannelsRequired(TimeValue t);
	void Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort);
};


// --------------------------------------------------
// Motion blur class descriptor - class declaration
// --------------------------------------------------
class MotBlurClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MotionBlur; }
	const TCHAR *	ClassName() { return ::GetString(IDS_MOT_BLUR); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return motBlurClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("ImageMotionBlur"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static MotBlurClassDesc motBlurCD;

// ClassDesc external access function 
ClassDesc* GetMotBlurDesc() { return &motBlurCD; }


// main IMBPosImp fp interface
static IMBOpsImp imb_interface(IMB_INTERFACE, _T("ImageMotionBlur"), 0, &motBlurCD, 0,	end	);


// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 motBlur_param_blk(motBlur_params, _T("imageMotionBlur parameters"), 0, &motBlurCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_MOT_BLUR_EFFECT, IDS_MOT_BLUR_PARAMS, 0, 0, NULL,
	// params
	prm_dur, _T("duration"), TYPE_FLOAT, P_ANIMATABLE, IDS_DUR,
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DUR_EDIT, IDC_DUR_SPIN, .01,
		end,
	prm_transp, _T("transparency"), TYPE_BOOL, 0, IDS_TRANSP,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX,  IDC_TRANSP,
		end,
	end
	);


// -----------------------------------------
// Motion blur effect - method definitions
// -----------------------------------------
MotionBlur::MotionBlur()
{
	motBlurCD.MakeAutoParamBlocks(this);
	assert(pblock);
}

IOResult MotionBlur::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *MotionBlur::CreateParamDialog(IRendParams *ip)
{	
	return motBlurCD.CreateParamDialogs(ip, this);
}

TSTR MotionBlur::SubAnimName(int i)
{
	switch (i)
	{
	case 0:
		return GetString(IDS_MOT_BLUR_PARAMS);
	default:
		return _T("");
	}
}

RefTargetHandle MotionBlur::GetReference(int i)
{
	switch (i)
	{
	case 0:
		return pblock;
	default:
		return NULL;
	}
}

void MotionBlur::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case 0:
		pblock = (IParamBlock2*)rtarg;
		break;
	}
}

RefResult MotionBlur::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message)
	{
	case REFMSG_CHANGE:
		if ( pblock )	// > 11/12/02 - 3:38pm --MQM-- #417502, need "if (pblock)"
			motBlur_param_blk.InvalidateUI( pblock->LastNotifyParamID() );
		break;
	}
	return REF_SUCCEED;
}

void MotionBlur::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort) {
	float dur;
	BOOL useTransp;
	pblock->GetValue(prm_dur, t, dur, FOREVER);
	pblock->GetValue( prm_transp, 0, useTransp, FOREVER );
	imb_interface.ApplyMotionBlur(bm, checkAbort,dur,useTransp?IMB_TRANSP:0,NULL);
	}

DWORD MotionBlur::GBufferChannelsRequired(TimeValue t) {	
	BOOL useTransp;
	pblock->GetValue( prm_transp, 0, useTransp, FOREVER );
	return imb_interface.ChannelsRequired(useTransp?IMB_TRANSP:0);
	} 
