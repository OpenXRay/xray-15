/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: colorBalance.cpp

	 DESCRIPTION: balance RGB levels

	 CREATED BY: michael malone (mjm)

	 HISTORY: created October 02, 1998

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// local headers
#include "dllMain.h"

// sdk headers
#include <iparamm2.h>
#include <bmmlib.h>

// IDs to references
#define PBLOCK_REF 0

#define MAX_COLf 65535.0f
#define DEF_OFFSET 0

// parameter blocks IDs
enum { colBal_params };

// parameters for colBal_params
enum { prm_r,
       prm_g,
	   prm_b,
	   prm_pLum,
	   prm_iBack };

// global instance
static const Class_ID colBalClassID(0xd481815, 0x786d799c);


// ----------------------------------------
// color balance effect - class declaration
// ----------------------------------------
class ColorBalance: public Effect
{
public:
	// parameters
	IParamBlock2* pblock;

	ColorBalance();
	~ColorBalance() { }

	// Animatable/Reference
	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	Class_ID ClassID() { return colBalClassID; }
	void GetClassName(TSTR& s) { s = GetString(IDS_COL_BAL); }
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; } // only one
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }
	IOResult Load(ILoad *iload);

	// Effect
	TSTR GetName() { return GetString(IDS_COL_BAL); }
	EffectParamDlg *CreateParamDialog(IRendParams *ip);
	DWORD GBufferChannelsRequired(TimeValue t) { return BMM_CHAN_NONE; }
	void Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort);
};


// --------------------------------------------------
// color balance class descriptor - class declaration
// --------------------------------------------------
class ColBalClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ColorBalance; }
	const TCHAR *	ClassName() { return GetString(IDS_COL_BAL); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return colBalClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("colorBalance"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static ColBalClassDesc colBalCD;
// external access function (through colorBalance.h)
ClassDesc* GetColBalDesc() { return &colBalCD; }


// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 colBal_param_blk(colBal_params, _T("colorBalance parameters"), 0, &colBalCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	prm_r, _T("red"), TYPE_INT, P_ANIMATABLE, IDS_RED,
		p_default, DEF_OFFSET,
		p_ui, TYPE_SLIDER, EDITTYPE_INT, IDC_RED_EDIT, IDC_RED_SLIDER, 10,
		p_range, -100, 100,
		end,
	prm_g, _T("green"), TYPE_INT, P_ANIMATABLE, IDS_GREEN,
		p_default, DEF_OFFSET,
		p_ui, TYPE_SLIDER, EDITTYPE_INT, IDC_GREEN_EDIT, IDC_GREEN_SLIDER, 10,
		p_range, -100, 100,
		end,
	prm_b, _T("blue"), TYPE_INT, P_ANIMATABLE, IDS_BLUE,
		p_default, DEF_OFFSET,
		p_ui, TYPE_SLIDER, EDITTYPE_INT, IDC_BLUE_EDIT, IDC_BLUE_SLIDER, 10,
		p_range, -100, 100,
		end,
	prm_pLum, _T("preserveLum"), TYPE_BOOL, P_ANIMATABLE, IDS_P_LUM,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LUMINOSITY,
		end,
	prm_iBack, _T("ignoreBack"), TYPE_BOOL, P_ANIMATABLE, IDS_I_BACK,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BACKGROUND,
		end,
	end
	);


// -----------------------------------------
// color balance effect - method definitions
// -----------------------------------------
ColorBalance::ColorBalance()
{
	colBalCD.MakeAutoParamBlocks(this);
	assert(pblock);
}

IOResult ColorBalance::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *ColorBalance::CreateParamDialog(IRendParams *ip)
{	
	return colBalCD.CreateParamDialogs(ip, this);
}

TSTR ColorBalance::SubAnimName(int i)
{
	switch (i)
	{
	case 0:
		return GetString(IDS_COL_BAL_PARAMS);
	default:
		return _T("");
	}
}

RefTargetHandle ColorBalance::GetReference(int i)
{
	switch (i)
	{
	case 0:
		return pblock;
	default:
		return NULL;
	}
}

void ColorBalance::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case 0:
		pblock = (IParamBlock2*)rtarg;
		break;
	}
}

RefResult ColorBalance::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message)
	{
	case REFMSG_CHANGE:
		if ( pblock )	// > 11/12/02 - 3:38pm --MQM-- #417502, need "if (pblock)"
			colBal_param_blk.InvalidateUI( pblock->LastNotifyParamID() );
		break;
	}
	return REF_SUCCEED;
}

void ColorBalance::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort)
{
	int type;
	WORD *pS = (WORD*)bm->GetStoragePtr(&type); // rgb
	assert(type == BMM_TRUE_48);
	WORD *pA = (WORD*)bm->GetAlphaPtr(&type);   // alpha
	assert(type == BMM_GRAY_16);

	int chkIndex = 3*bm->Width();
	int imageSz = bm->Width() * bm->Height();

	int iTemp;
	pblock->GetValue(prm_r, t, iTemp, FOREVER);	float rOffset = iTemp * .01f;
	pblock->GetValue(prm_g, t, iTemp, FOREVER);	float gOffset = iTemp * .01f;
	pblock->GetValue(prm_b, t, iTemp, FOREVER);	float bOffset = iTemp * .01f;

	BOOL pLum;
	pblock->GetValue(prm_pLum, t, pLum, FOREVER);

	BOOL iBack;
	pblock->GetValue(prm_iBack, t, iBack, FOREVER);

	AColor c1, c2;

	if(pS&&pA)
	{
		for (int index=0; index<imageSz; index++, pS+=3, pA++)
		{
			if ( iBack && (*pA == 0) )
				continue;

			c1.r = *pS / MAX_COLf;
			c1.g = *(pS+1) / MAX_COLf;
			c1.b = *(pS+2) / MAX_COLf;
			c1.a = *pA / MAX_COLf;

			if (pLum)
			{
				// original luminance (from "A Technical Introduction to Digital Video", C. Poynton)
				float lumIn = c1.r*0.2125f + c1.g*0.7154f + c1.b*0.0721f;

	/*
				float rOff(0.0f), gOff(0.0f), bOff(0.0f);
				rOff += rOffset*.85f; gOff -= rOffset*.15f; bOff -= rOffset*.15f; // split offset with other channels so not to lose
				rOff -= gOffset*.15f; gOff += gOffset*.85f; bOff -= gOffset*.15f; // all color information at extreme offsets
				rOff -= bOffset*.15f; gOff -= bOffset*.15f; bOff += bOffset*.85f;
				c2.r = c1.r + c1.r * rOff;
				c2.g = c1.g + c1.g * gOff;
				c2.b = c1.b + c1.b * bOff;
	*/
				// above code rewritten - slightly compressed
				c2.r = c1.r * (1 + .85f*rOffset - .15f*(gOffset + bOffset));
				c2.g = c1.g * (1 + .85f*gOffset - .15f*(rOffset + bOffset));
				c2.b = c1.b * (1 + .85f*bOffset - .15f*(rOffset + gOffset));

				// new luminance
				float lumOut = c2.r*0.2125f + c2.g*0.7154f + c2.b*0.0721f;

				// scale to preserve luminance
				float ratio = (lumOut <= 0) ? 0 : lumIn / lumOut;
				c2 *= ratio;
			}
			else
			{
				// calculate new color by adding a percentage of maxAdd
				// not preserving luminance, so maxAdd allows full black to full white
				float maxAdd;
				maxAdd = (rOffset < 0) ? c1.r : (1.0f - c1.r);
				c2.r = c1.r + maxAdd*rOffset;

				maxAdd = (gOffset < 0) ? c1.g : (1.0f - c1.g);
				c2.g = c1.g + maxAdd*gOffset;

				maxAdd = (bOffset < 0) ? c1.b : (1.0f - c1.b);
				c2.b = c1.b + maxAdd*bOffset;
			}

			if ( iBack )
				c2 = c2*c1.a + c1*(1.0f-c1.a);
			
			c2.ClampMinMax();

			*pS     = (USHORT)(c2.r * MAX_COLf);
			*(pS+1) = (USHORT)(c2.g * MAX_COLf);
			*(pS+2) = (USHORT)(c2.b * MAX_COLf);

			if ( ( (index & chkIndex) == 0 ) && checkAbort && checkAbort->Progress(index, imageSz) ) 
				return;
		}
	}
	else
	{

		BMM_Color_64* buf = new BMM_Color_64[bm->Width()];
		for(int h=0;h<bm->Height();h++)
		{
			bm->GetPixels(0,h,bm->Width(),buf);

			for (int index=0; index<bm->Width(); index++)
			{
				if ( iBack && (buf[index].a == 0) )
					continue;

				c1.r = buf[index].r / MAX_COLf;
				c1.g = buf[index].g / MAX_COLf;
				c1.b = buf[index].b / MAX_COLf;
				c1.a = buf[index].a / MAX_COLf;

				if (pLum)
				{
					// original luminance (from "A Technical Introduction to Digital Video", C. Poynton)
					float lumIn = c1.r*0.2125f + c1.g*0.7154f + c1.b*0.0721f;

					// above code rewritten - slightly compressed
					c2.r = c1.r * (1 + .85f*rOffset - .15f*(gOffset + bOffset));
					c2.g = c1.g * (1 + .85f*gOffset - .15f*(rOffset + bOffset));
					c2.b = c1.b * (1 + .85f*bOffset - .15f*(rOffset + gOffset));

					// new luminance
					float lumOut = c2.r*0.2125f + c2.g*0.7154f + c2.b*0.0721f;

					// scale to preserve luminance
					float ratio = (lumOut <= 0) ? 0 : lumIn / lumOut;
					c2 *= ratio;
				}
				else
				{
					// calculate new color by adding a percentage of maxAdd
					// not preserving luminance, so maxAdd allows full black to full white
					float maxAdd;
					maxAdd = (rOffset < 0) ? c1.r : (1.0f - c1.r);
					c2.r = c1.r + maxAdd*rOffset;

					maxAdd = (gOffset < 0) ? c1.g : (1.0f - c1.g);
					c2.g = c1.g + maxAdd*gOffset;

					maxAdd = (bOffset < 0) ? c1.b : (1.0f - c1.b);
					c2.b = c1.b + maxAdd*bOffset;
				}

				if ( iBack )
					c2 = c2*c1.a + c1*(1.0f-c1.a);
				
				c2.ClampMinMax();

				buf[index].r = (USHORT)(c2.r * MAX_COLf);
				buf[index].g = (USHORT)(c2.g * MAX_COLf);
				buf[index].b = (USHORT)(c2.b * MAX_COLf);

				if ( ( (index & chkIndex) == 0 ) && checkAbort && checkAbort->Progress(h*bm->Width() +index, imageSz) ) 
				{
					delete [] buf;
					return;
				}
			}
			bm->PutPixels(0,h,bm->Width(),buf);
		}
		delete [] buf;
	}
}
