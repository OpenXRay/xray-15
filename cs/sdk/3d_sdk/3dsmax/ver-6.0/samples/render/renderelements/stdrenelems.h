///////////////////////////////////////////////////////////////
//
//	Standard Render Elements
//

#ifndef STD_RENDER_ELEMENTS_H
#define STD_RENDER_ELEMENTS_H

#include "renderElements.h"
#include "renElemRes.h"

// Class ids

// default is defined in include/renderelements.h
//#define BEAUTY_RENDER_ELEMENT_CLASS_ID		0x00000001
#define SPECULAR_RENDER_ELEMENT_CLASS_ID		0x00000002
#define SPECULAR_SELECT_RENDER_ELEMENT_CLASS_ID	0x00000003
#define DIFFUSE_RENDER_ELEMENT_CLASS_ID			0x00000004
#define EMISSION_RENDER_ELEMENT_CLASS_ID		0x00000005
#define REFLECTION_RENDER_ELEMENT_CLASS_ID		0x00000006
#define REFRACTION_RENDER_ELEMENT_CLASS_ID		0x00000007
#define SHADOWS_RENDER_ELEMENT_CLASS_ID			0x00000008
#define ATMOSPHERE_RENDER_ELEMENT_CLASS_ID		0x00000009
#define ATMOSPHERE2_RENDER_ELEMENT_CLASS_ID		0x0000000a
#define BLEND_RENDER_ELEMENT_CLASS_ID			0x0000000b
#define Z_RENDER_ELEMENT_CLASS_ID				0x0000000c
#define ALPHA_RENDER_ELEMENT_CLASS_ID			0x0000000d
#define CLR_SHADOW_RENDER_ELEMENT_CLASS_ID		0x0000000e
#define BGND_RENDER_ELEMENT_CLASS_ID			0x0000000f
#define SPECULAR_COMP_RENDER_ELEMENT_CLASS_ID	0x00000010
#define LIGHTING_RENDER_ELEMENT_CLASS_ID		0x00000011
#define MATTE_RENDER_ELEMENT_CLASS_ID			0x00000012


// Render Element Class Descriptors
extern ClassDesc* GetBeautyElementDesc();
extern ClassDesc* GetSpecularElementDesc();
extern ClassDesc* GetSpecularSelectElementDesc();
extern ClassDesc* GetSpecularCompElementDesc();
extern ClassDesc* GetDiffuseElementDesc();
extern ClassDesc* GetLightingElementDesc();
extern ClassDesc* GetEmissionElementDesc();
extern ClassDesc* GetReflectionElementDesc();
extern ClassDesc* GetRefractionElementDesc();
extern ClassDesc* GetShadowElementDesc();
extern ClassDesc* GetAtmosphereElementDesc();
extern ClassDesc* GetAtmosphere2ElementDesc();
extern ClassDesc* GetBlendElementDesc();
extern ClassDesc* GetZElementDesc();
extern ClassDesc* GetAlphaElementDesc();
extern ClassDesc* GetBgndElementDesc();
extern ClassDesc* GetMatteElementDesc();

////////////////////////////////////////////////////////////////////////////
//	render element shared base class
//
class BaseRenderElement: public MaxRenderElement {
	protected:
//		Bitmap*		mpBitmap;
		IParamBlock2 *mpParamBlk;

	public:

		// shared pblock parameter ids
		enum { enableOn,
			   filterOn,
			   eleName,
			   pbBitmap };

		void DeleteThis() { delete this; };

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i){ return i? NULL : mpParamBlk; }
		TSTR SubAnimName(int i)
			{ return i? _T("") : _T(GetString(IDS_KE_PARAMETERS)); }

		int NumRefs() { return 1;};
		RefTargetHandle GetReference(int i){ return i? NULL : mpParamBlk; }
		void SetReference(int i, RefTargetHandle rtarg)
			{ if ( i == 0 ) mpParamBlk = (IParamBlock2*)rtarg; }

		void GetClassName(TSTR& s) { s = GetName(); }

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return mpParamBlk; } // only one
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpParamBlk->ID() == id) ? mpParamBlk : NULL; }

		// enable & disable the render element
		void SetEnabled(BOOL on){ mpParamBlk->SetValue( enableOn, 0, on ); }
		BOOL IsEnabled() const{
			int	on;
			mpParamBlk->GetValue( enableOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's filter enabled state
		void SetFilterEnabled(BOOL on){ mpParamBlk->SetValue( filterOn, 0, on ); }
		BOOL IsFilterEnabled() const{
			int	on;
			mpParamBlk->GetValue( filterOn, 0, on, FOREVER );
			return on;
		}
		BOOL BlendOnMultipass() const { return TRUE; }

		BOOL AtmosphereApplied() const{ return FALSE; }

		BOOL ShadowsApplied() const{ return FALSE; }

		// set/get element's name (as it will appear in render dialog)
		void SetElementName( TCHAR* newName){ mpParamBlk->SetValue( eleName, 0, newName ); }
		const TCHAR* ElementName() const {
			TCHAR* pStr = NULL;
			mpParamBlk->GetValue( eleName, 0, pStr, FOREVER );
			return pStr;
		}

		void SetPBBitmap(PBBitmap* &pPBBitmap) const
		{
			mpParamBlk->SetValue( pbBitmap, 0, pPBBitmap );
		}

		void GetPBBitmap(PBBitmap* &pPBBitmap) const
		{
			mpParamBlk->GetValue( pbBitmap, 0, pPBBitmap, FOREVER );
		}


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
		RefResult NotifyRefChanged(Interval changeInt, 
			RefTargetHandle hTarget, PartID& partID, RefMessage message){ 
			return REF_SUCCEED;
		}

		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip){}

		// called after atmospheres are computed, to allow elements to handle atmospheres
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev){};
	};

#define PBLOCK_REF	0
#define PBLOCK_NUMBER	0

#endif

