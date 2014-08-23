/**********************************************************************
 *<
	FILE: briteCon.cpp	

	DESCRIPTION: Simple Brightness & Contrast Post Effect

	CREATED BY: Kells Elmquist

	HISTORY: 7/9/98
	         1.5.99 mjm - update to paramblock2 and add ignore background option

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "dllMain.h"
#include <iparamm2.h>
#include <bmmlib.h>

// prototype for gain & brightness functions
typedef	float (*BriteConFn)( float value, float param );

// proposed functions for brightness & gain
float pivotGain( float val, float param );
float dcGain( float val, float param );

float perlinContrast( float val, float param );
float perlinGainPrim( float val, float param );
float perlinBrightness( float val, float param );
float perlinBiasPrim( float val, float param );
float keBrightness( float val, float param );

float schickContrast( float val, float param );
float schickBrightness( float val, float param );
float schickGainPrim( float val, float param );
float schickBiasPrim( float val, float param );

#define BRITECON_CLASS_ID 0x76912330 // dans * 16
Class_ID BriteConClassID(BRITECON_CLASS_ID,0);

// IDs to references
#define PBLOCK_REF 0

// parameter blocks IDs
enum { briteCon_params };

// parameters for colBal_params
enum { prm_brightness,
       prm_contrast,
	   prm_iBack };


// Brightness & contrast effect
class BriteConEffect: public Effect {
	public:
		IParamBlock2* pblock;
		
		BriteConEffect();
		~BriteConEffect() { }

		// Animatable/Reference
		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) { return GetReference(i); }
		TSTR SubAnimName(int i);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		Class_ID ClassID() { return BriteConClassID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
		void DeleteThis() { delete this; }
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock; } // only one
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }
		IOResult Load(ILoad *iload);

		// Effect methods
		TSTR GetName() { return GetString(IDS_NAME); }
		EffectParamDlg *CreateParamDialog( IRendParams *pParams );
		DWORD GBufferChannelsRequired(TimeValue t) { return BMM_CHAN_NONE; }
		void Apply( TimeValue t, Bitmap *pBM, RenderGlobalContext *pGC, CheckAbortCallback *checkAbort);
	};


// Class Descriptor
class BriteConClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BriteConEffect; }
	const TCHAR *	ClassName() { return GetString(IDS_CDESC_CLASS_NAME); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return BriteConClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("briteCon"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

static BriteConClassDesc briteConCD;
ClassDesc* GetBriteConDesc() {return &briteConCD;}


// Parameter Description
static ParamBlockDesc2 briteCon_param_blk(briteCon_params, _T("briteCon parameters"), 0, &briteConCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BRITECON_EFFECT, IDS_PARAMS, 0, 0, NULL,
	// params
	prm_brightness, _T("brightness"), TYPE_FLOAT, P_ANIMATABLE, IDS_BRIGHTNESS,
		p_default, 0.5f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BRIGHTNESS, IDC_BRIGHTNESS_SPIN, SPIN_AUTOSCALE,
		p_range, 0.0f, 1.0f,
		end,
	prm_contrast, _T("contrast"), TYPE_FLOAT, P_ANIMATABLE, IDS_CONTRAST,
		p_default, 0.5f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CONTRAST, IDC_CONTRAST_SPIN, SPIN_AUTOSCALE,
		p_range, 0.0f, 1.0f,
		end,
	prm_iBack, _T("ignoreBack"), TYPE_BOOL, P_ANIMATABLE, IDS_IGN_BACK,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_IGN_BACKGROUND,
		end,
	end
	);

///////////////////////////////
//
// Brightness & Contrast Effect
//
//
BriteConEffect::BriteConEffect()
{
	briteConCD.MakeAutoParamBlocks(this);
	assert(pblock);
}

IOResult BriteConEffect::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *BriteConEffect::CreateParamDialog(IRendParams *ip)
{	
	return briteConCD.CreateParamDialogs(ip, this);
}

TSTR BriteConEffect::SubAnimName(int i) 
{
	switch (i)
	{
	case 0:
		return GetString(IDS_PARAMS);
	default:
		return _T("");
	}
}

RefTargetHandle BriteConEffect::GetReference(int i)
{
	switch (i)
	{
	case 0:
		return pblock;
	default:
		return NULL;
	}
}

void BriteConEffect::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case 0:
		pblock = (IParamBlock2*)rtarg; break;
	}
}

RefResult BriteConEffect::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	switch (message)
	{
	case REFMSG_CHANGE:
		if ( pblock )	// > 11/12/02 - 3:38pm --MQM-- #417502, need "if (pblock)"
			briteCon_param_blk.InvalidateUI( pblock->LastNotifyParamID() );
		break;
	}
	return REF_SUCCEED;
}

////////////////////////////////////////////
//
// Apply the brightness & contrast functions
//	
#define	BCTAB_SHIFT	4 // 16 bits into 12		
#define	BCTAB_SZ	4096		
#define MAX_COLf	65535.0f

// The current brightness & contrast functions
static BriteConFn pContrastFn = perlinContrast;
static BriteConFn pBrightnessFn = keBrightness;

void BriteConEffect::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc,CheckAbortCallback *checkAbort)
{
	long w = bm->Width();
	long h = bm->Height();

	float brightness;
	float contrast;
	BOOL iBack;
	pblock->GetValue(prm_brightness, t, brightness, FOREVER);
	pblock->GetValue(prm_contrast, t, contrast, FOREVER);
	pblock->GetValue(prm_iBack, t, iBack, FOREVER);

	// build contrast table
	unsigned short contrastTable[ BCTAB_SZ ];
	float vInc = 1.0f / float(BCTAB_SZ);
	float v = 0.5f * vInc;	// center of the first bin
	for ( long i = 0; i < BCTAB_SZ; ++i, v += vInc ) {

		// apply contrast then brightness to the result
		float f = (*pContrastFn)( v, contrast );
		f = (*pBrightnessFn)( f, brightness );
		if ( f > 1.0f )
			f = 1.0f;
		else if ( f < 0.0 )
			f = 0.0f;
		contrastTable[i] = unsigned short(f * MAX_COLf + 0.5f);
	}

	// modify current frame buffer, into itself
	float alpha;
	BMM_Color_64 tempCol;
	PixelBuf lineBuf(w);
	BMM_Color_64 *pPix = lineBuf.Ptr();
	// for each line
	for (long y = 0; y < h; y++) {
		bm->GetPixels(0, y, w, pPix);
		// for each pixel
		for ( long x = 0; x < w; ++x ){
			// if ignoring background ...
			if ( iBack )
			{
				// skip transparent pixel
				if (pPix[x].a == 0)
					continue;

				// otherwise, blend on alpha value
				alpha = pPix[x].a / MAX_COLf;
				tempCol.r = contrastTable[ pPix[x].r >> BCTAB_SHIFT ];
				tempCol.g = contrastTable[ pPix[x].g >> BCTAB_SHIFT ];
				tempCol.b = contrastTable[ pPix[x].b >> BCTAB_SHIFT ];

				pPix[x].r = (USHORT)(tempCol.r*alpha + pPix[x].r*(1.0f-alpha));
				pPix[x].g = (USHORT)(tempCol.g*alpha + pPix[x].g*(1.0f-alpha));
				pPix[x].b = (USHORT)(tempCol.b*alpha + pPix[x].b*(1.0f-alpha));
			}
			else
			{
				pPix[x].r = contrastTable[ pPix[x].r >> BCTAB_SHIFT ];
				pPix[x].g = contrastTable[ pPix[x].g >> BCTAB_SHIFT ];
				pPix[x].b = contrastTable[ pPix[x].b >> BCTAB_SHIFT ];
			}
		}
		bm->PutPixels(0, y, w, pPix);
		if (((y&3)==0)&&checkAbort&&checkAbort->Progress(y,h)) 
			return;
	}
}

//////////////////////////////////////////////////////////////
//
//	The brightness & contrast functions
//	
//	These are all defined only over the val range 0..1 
//  & the parameter range 0..1
//	they shd return values in the same range, tho checking is not required
//
//	if your function needs a value in range, clamp it
//

// this probably wants to be centered at the perceptual midpoint
// this looked best in my first tests
static float pivot = 0.35f;

// linear gain about the pivot point
// map contrast 0 to gain 0.25, contrast .5 to gain 1, & contrast 1 to gain 1.7
float pivotGain( float val, float param )
{
	float gain = param <= 0.5f ? param * 1.5f + 0.25f : (param - 0.5f) * 1.4f + 1.0f;
	return (val - pivot) * gain + pivot;
}

// dc gain for brightness
// 0.5 to gain of 0, 0 to gain of -.3, 1 to gain of +.3
float dcGain( float val, float param )
{
	return (val + (param - 0.5f) * 0.6f);
}

// perlin gain as contrast fn
// map 0.5 -> 0.5; 0.0 ->.25; 1.0 -> .75
float perlinContrast( float val, float param )
{
	float gain = ( 1.0f - param) * 0.5f + 0.25f;
	return perlinGainPrim( val, gain );
}

// perlin gain
float perlinGainPrim( float val, float param )
{
	if ( val < 0.5f )
		return 0.5f * perlinBiasPrim( 2.0f * val, param );
	else
		return 1.0f - 0.5f * perlinBiasPrim( 2.0f - 2.0f * val, param );
}

static float hiliteBias = 0.9f;

// my brightness function
float keBrightness( float val, float param )
{
	float hiliteCorr;
	if ( param < 0.5f ) {
		float p = 1.8f * param + 0.1f;
	    hiliteCorr = perlinBiasPrim( p , hiliteBias );
	} else hiliteCorr = 1.0f;

	float bias = param * 0.7f + 0.15f;
	return hiliteCorr * perlinBiasPrim( val, bias );
}

// perlin based brightness fn
float perlinBrightness( float val, float param )
{
	float bias = param * 0.7f + 0.1f;
	return perlinBiasPrim( val, bias );
}

// perlin bias primitive
float perlinBiasPrim( float val, float param )
{
	const double ln2R = -1.0 / log( 2.0 );
	if ( param < 0.0000001f )
		param = 0.0000001f;
	double lnA = log( param );
	return float( pow( double(val), lnA * ln2R ) );
}

// schick gain as contrast fn
// map 0.5 -> 0.5; 0.0 ->.25; 1.0 -> .75
float schickContrast( float val, float param )
{
	float gain = ( 1.0f - param) * 0.5f + 0.25f;
	return schickGainPrim( val, gain );
}

// schick gain
float schickGainPrim( float val, float param )
{
	float p = 1.0f/param - 2.0f;
	float t = 1.0f - 2.0f * val;
	if ( val < 0.5f )
		return ( val / (p * t + 1.0f) );
	else
		return ( (p * t - val)/(p * t - 1.0f) );
}

// perlin based brightness fn
float schickBrightness( float val, float param )
{
	float bias = param * 0.7f + 0.1f;
	return schickBiasPrim( val, bias );
}

// schick bias
float schickBiasPrim( float val, float param )
{
	return ( val/( (1.0f/param - 2.0f) * (1.0f - val) + 1.0f ) );
}
