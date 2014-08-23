////////////////////////////////////////////////////////////////////////
//
//	sampRenderElement.cpp	
//
//	"Beauty" render element, has all channels combined
//
//	Created: Kells Elmquist, 15 Apr, 2000
//
//	Copyright (c) 2000, All Rights Reserved.
//

#include "renElemPch.h"
#include "stdRenElems.h"

#include <toneop.h>

Class_ID beautyRenderElementClassID( BEAUTY_RENDER_ELEMENT_CLASS_ID , 0);



////////////////////////////////////////////////////////////////////////////
//	Beauty render element
//
class BeautyRenderElement: public MaxRenderElement {
private:
//	Bitmap*		mpBitmap;

	public:
		// Parameters
		IParamBlock2 *pParamBlk;
		
		BeautyRenderElement();
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
			{ if ( i == 0 ) pParamBlk = (IParamBlock2*)rtarg; }

		Class_ID ClassID() {return beautyRenderElementClassID;};
		TSTR GetName() { return GetString( IDS_KE_BEAUTY_RENDER_ELEMENT ); }
		void GetClassName(TSTR& s) { s = GetName(); }

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pParamBlk; } // only one
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pParamBlk->ID() == id) ? pParamBlk : NULL; }

		// enable & disable the render element
		void SetEnabled(BOOL enabled);
		BOOL IsEnabled() const;

		// set/get element's filter enabled state
		void SetFilterEnabled(BOOL filterEnabled);
		BOOL IsFilterEnabled() const;

		BOOL BlendOnMultipass() const { return TRUE; }

		// set/get whether to apply atmosphere
		void SetApplyAtmosphere(BOOL applyAtmosphere);
		BOOL AtmosphereApplied() const;

		void SetApplyShadows(BOOL applyShadows);
		BOOL ShadowsApplied() const;

		// set/get element's name (as it will appear in render dialog)
		void SetElementName( TCHAR* newName); 
		const TCHAR* ElementName() const;

/*
		// set/get path name for file output
		void SetPathName( TCHAR* newPathName);
		const TCHAR* PathName() const;

		void SetBitmap( Bitmap* bitmap);
		Bitmap* GetBitmap() const;
*/
		void SetPBBitmap(PBBitmap* &pPBBitmap) const;
		void GetPBBitmap(PBBitmap* &pPBBitmap) const;

		// this is the element specific optional UI, which is a rollup in the render dialog
		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }

		// Implement this if you are using the ParamMap2 AUTO_UI system and the 
		// IRenderElement has secondary dialogs that don't have the IRenderElement as their 'thing'.
		// Called once for each secondary dialog, for you to install the correct thing.
		// Return TRUE if you process the dialog, false otherwise.
		BOOL SetDlgThing(IRenderElementParamDlg* dlg) { return FALSE; }

		// ---------------------
		// from class RefMaker
		// ---------------------
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		// called after atmospheres are computed, to allow elements to handle atmospheres
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
	};

// --------------------------------------------------
// color balance class descriptor - class declaration
// --------------------------------------------------
class BeautyElementClassDesc:public ClassDesc2
{
public:
//	int 			IsPublic() { return 1; }
	int 			IsPublic() { return 0; } // this is sample, not public
	void *			Create(BOOL loading) { return new BeautyRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BEAUTY_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return beautyRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("beautyRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static BeautyElementClassDesc beautyCD;
ClassDesc* GetBeautyElementDesc() { return &beautyCD; }

#define PBLOCK_REF	0
#define PBLOCK_NUMBER	0


// pblock parameter ids
enum { enableOn,
       filterOn,
	   atmosphereOn,
	   shadowOn,
	   eleName,
	   pbBitmap };
//	   pathName };


// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 beauty_param_blk(PBLOCK_NUMBER, _T("Beauty render element parameters"), 0, &beautyCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
		p_default, TRUE,
		end,
	atmosphereOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_default, TRUE,
		end,
	eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default,"",
		end,
	pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
/*
	pathName, _T("pathName"), TYPE_STRING, 0, IDS_PATH_NAME,
		p_default, "",
		end,
*/
	end
	);



//--- Beauty Render Element ------------------------------------------------


BeautyRenderElement::BeautyRenderElement()
{
	beautyCD.MakeAutoParamBlocks(this);
	DbgAssert(pParamBlk);
	SetElementName( GetName() );
}

RefTargetHandle BeautyRenderElement::Clone( RemapDir &remap )
{
	BeautyRenderElement*	mnew = new BeautyRenderElement();
	mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
	return (RefTargetHandle)mnew;
}


/*******
IOResult BeautyRenderElement::Load(ILoad *iload)
{
	IRenderElement::Load(iload);
	return IO_OK;
}


IOResult BeautyRenderElement::Save(ISave *isave)
{
	IRenderElement::Save(isave);
	return IO_OK;
}
********/


RefResult BeautyRenderElement::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			;
			break;
/********
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim * gpd = (GetParamDim*)partID;
			switch (gpd->index) {
//>>>>>>				case PB_ENABLE: gpd->dim = defaultDim; break;
				default: 	     gpd->dim = defaultDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
//				case PB_ENABLE : gpn->name = _T( GetString(IDS_KE_ENABLE) ); break;
				default:		  gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
**********/
	}
	return REF_SUCCEED;
}


// enable & disable the render element
void BeautyRenderElement::SetEnabled( BOOL on )
{
	pParamBlk->SetValue( enableOn, 0, on );
}

BOOL BeautyRenderElement::IsEnabled() const
{
	int	on;
	pParamBlk->GetValue( enableOn, 0, on, FOREVER );
	return on;
}

// enable & disable filtering for the render element
void BeautyRenderElement::SetFilterEnabled( BOOL on )
{
	pParamBlk->SetValue( filterOn, 0, on );
}

BOOL BeautyRenderElement::IsFilterEnabled() const
{
	int	on;
	pParamBlk->GetValue( filterOn, 0, on, FOREVER );
	return on;
}

void BeautyRenderElement::SetApplyAtmosphere( BOOL on )
{
	pParamBlk->SetValue( atmosphereOn, 0, on );
}

BOOL BeautyRenderElement::AtmosphereApplied() const
{
	int	on;
	pParamBlk->GetValue( atmosphereOn, 0, on, FOREVER );
	return on;
}

void BeautyRenderElement::SetApplyShadows( BOOL on )
{
	pParamBlk->SetValue( shadowOn, 0, on );
}

BOOL BeautyRenderElement::ShadowsApplied() const
{
	int	on;
	pParamBlk->GetValue( shadowOn, 0, on, FOREVER );
	return on;
}


const TCHAR* BeautyRenderElement::ElementName() const
{
	TCHAR* pStr = NULL;
	pParamBlk->GetValue( eleName, 0, pStr, FOREVER );
	return pStr;
}

// set copies the strings, get returns our pointer
void BeautyRenderElement::SetElementName( TCHAR* pStr ) 
{
	pParamBlk->SetValue( eleName, 0, pStr );
}

/*
// path name for file
// set copies the strings, get returns our pointer
const TCHAR* BeautyRenderElement::PathName() const
{
	TCHAR* pStr = NULL;
	pParamBlk->GetValue( pathName, 0, pStr, FOREVER );
	return pStr;
}

void BeautyRenderElement::SetPathName( TCHAR* pStr ) 
{
	pParamBlk->SetValue( pathName, 0, pStr );
}

// note we don't open the files, user opens file & gives us the bitmap
// does this include rla files?
Bitmap* BeautyRenderElement::GetBitmap() const 
{
	return mpBitmap;
}

void BeautyRenderElement::SetBitmap( Bitmap * pBitmap ) 
{
	mpBitmap = pBitmap;
}
*/
void BeautyRenderElement::SetPBBitmap(PBBitmap* &pPBBitmap) const
{
	pParamBlk->SetValue( pbBitmap, 0, pPBBitmap );
}

void BeautyRenderElement::GetPBBitmap(PBBitmap* &pPBBitmap) const
{
	pParamBlk->GetValue( pbBitmap, 0, pPBBitmap, FOREVER );
}

void BeautyRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	sc.out.elementVals[ mShadeOutputIndex ] = ip.finalC;
	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
	}
}

void BeautyRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
}

//-----------------------------------------------------------------------------------------


