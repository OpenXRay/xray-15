////////////////////////////////////////////////////////////////////////
//
//	Standard TextureBake Render Elements 	
//
//	Created: Kells Elmquist, 2 December, 2001
//
//	Copyright (c) 2001, All Rights Reserved.
//
// local includes
#include "renElemPch.h"
#include "renElemUtil.h"
#include "stdBakeElem.h"

// maxsdk includes
#include <iparamm2.h>
#include <toneop.h>


Class_ID completeBakeElementClassID( COMPLETE_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID specularBakeElementClassID( SPECULAR_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID diffuseBakeElementClassID( DIFFUSE_BAKE_ELEMENT_CLASS_ID , 0);
//Class_ID emissionRenderElementClassID( EMISSION_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID reflectRefractBakeElementClassID( REFLECT_REFRACT_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID shadowBakeElementClassID( SHADOW_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID blendBakeElementClassID( BLEND_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID lightBakeElementClassID( LIGHT_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID normalsBakeElementClassID( NORMALS_ELEMENT_CLASS_ID , 0);
Class_ID alphaBakeElementClassID( ALPHA_BAKE_ELEMENT_CLASS_ID , 0);


////////////////////////////////////
// color utility
inline void ClampMax( Color& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
}

inline void ClampMax( AColor& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
	if( c.a > 1.0f ) c.a = 1.0f;
}

static TCHAR* shadowIllumOutStr = "shadowIllumOut";
static TCHAR* inkIllumOutStr = "Ink";
static TCHAR* paintIllumOutStr = "Paint";


///////////////////////////////////////////////////////////////////////////////
//
//	Function publishing interface descriptor
//
FPInterfaceDesc BaseBakeElement::_fpInterfaceDesc(	
	BAKE_ELEMENT_INTERFACE,				// interface id
	_T("BakeElementProperties"),					// internal name
	0,											// res id of description string
	NULL,										// pointer to class desc of the publishing plugin
	FP_MIXIN + FP_CORE,							// flag bits; FP_CORE needed to allow maxscript to enumerate it 
												// (see src\dll\maxscrpt\maxclass.cpp - MAXSuperClass::show_interfaces)
	//item ID,  internal_name, description_string, return type, flags, num arguments
		//Paramter internal name,  description, type
	_nParams,		_T("nParams"), 0, TYPE_INT, 0, 0,
	_ParamName,	_T("paramName"), 0, TYPE_STRING, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_ParamType,	_T("paramType"), 0, TYPE_INT, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_GetParamValue,	_T("getParamValue"), 0, TYPE_INT, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_SetParamValue,	_T("setParamValue"), 0, TYPE_VOID, 0, 2,
		_T("paramIndex"), 0, TYPE_INT,
		_T("newValue"), 0, TYPE_INT,
	_FindParam,	_T("findParam"), 0, TYPE_INT, 0, 1,
		_T("paramName"), 0, TYPE_STRING,
	
//	properties,
//		_GetFlags,  _SetFlags,  _T("flags"),  0, TYPE_INT,
	end 
);

FPInterfaceDesc* BaseBakeElement::GetDesc() {
	return & _fpInterfaceDesc; 
}


///////////////////////////////////////////////////////////////////////////////
//
//	Diffuse render element
//
// parameters

class DiffuseBakeElement : public BaseBakeElement {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};
		static int paramNames[];

		DiffuseBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return diffuseBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_DIFFUSE_BAKE_ELEMENT ); }

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		int  GetNParams() const { return 2; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam < 3 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			if( strcmp( name, GetString( paramNames[1] )) == 0 )
				return 2;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 || nParam==2)? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsLightApplied();
			if( nParam == 2 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetLightApplied( newVal );
			else if( nParam == 2 ) 
				SetShadowApplied( newVal);
		}
};

int DiffuseBakeElement::paramNames[] = { IDS_LIGHTING_ON, IDS_SHADOWS_ON };

// --------------------------------------------------
// Diffuse element class descriptor
// --------------------------------------------------
class DiffuseBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new DiffuseBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_DIFFUSE_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return diffuseBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("DiffuseMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static DiffuseBakeElementClassDesc diffuseBakeCD;
ClassDesc* GetDiffuseBakeElementDesc() { return &diffuseBakeCD; }

IRenderElementParamDlg *DiffuseBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = diffuseBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class DiffusePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case DiffuseBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(DiffuseBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static DiffusePBAccessor diffusePBAccessor;


// ------------------------------------------------------
// Diffuse parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 diffuseBakeParamBlk(PBLOCK_NUMBER, _T("Diffuse TextureBake Parameters"), 0, &diffuseBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_SHADOW, IDS_DIFFUSE_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	DiffuseBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, FALSE,
		p_accessor,		&diffusePBAccessor,
		end,
	DiffuseBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, FALSE,
		end,
	DiffuseBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);

//--- Diffuse Render Element ------------------------------------------------
DiffuseBakeElement::DiffuseBakeElement()
{
	diffuseBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle DiffuseBakeElement::Clone( RemapDir &remap )
{
	DiffuseBakeElement*	newEle = new DiffuseBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void DiffuseBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// not bgnd, do we want shaded or unshaded textures?
		if( IsLightApplied() ){
			// shading is on
			sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);
			int n;
			if( (n = ip.FindUserIllumName( paintIllumOutStr ))>=0 )
				sc.out.elementVals[ mShadeOutputIndex ] += ip.GetUserIllumOutput( n );
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
			}

		}// end, want shaded output
		else {
			// unshaded, raw-but-blended texture
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, use black instead
			sc.out.elementVals[ mShadeOutputIndex ] = ip.stdIDToChannel ? ip.channels[ ip.stdIDToChannel[ ID_DI ]] : AColor(0,0,0);
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;
		}
	}
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Specular bake element
//
class SpecularBakeElement : public BaseBakeElement {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};

		static int paramNames[];

		SpecularBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_BAKE_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 2; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam < 3 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			if( strcmp( name, GetString( paramNames[1] )) == 0 )
				return 2;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 || nParam==2)? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsLightApplied();
			if( nParam == 2 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetLightApplied( newVal );
			else if( nParam == 2 ) 
				SetShadowApplied( newVal);
		}
};

int SpecularBakeElement::paramNames[] = {IDS_LIGHTING_ON, IDS_SHADOWS_ON };


// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SpecularBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("SpecularMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularBakeElementClassDesc specularBakeCD;
ClassDesc* GetSpecularBakeElementDesc() { return &specularBakeCD; }


IRenderElementParamDlg *SpecularBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = specularBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class SpecularPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case SpecularBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(SpecularBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static SpecularPBAccessor specularPBAccessor;



// ---------------------------------------------
// Specular parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularBakeParamBlk(PBLOCK_NUMBER, _T("Specular bake element parameters"), 0, &specularBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_SHADOW, IDS_SPECULAR_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	SpecularBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, FALSE,
		p_accessor,		&specularPBAccessor,
		end,
	SpecularBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, FALSE,
		end,
	SpecularBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);

//--- Specular Render Element ------------------------------------------------
SpecularBakeElement::SpecularBakeElement()
{
	specularBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle SpecularBakeElement::Clone( RemapDir &remap )
{
	SpecularBakeElement*	newEle = new SpecularBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void SpecularBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// not bgnd, do we want shaded or unshaded textures?
		if( IsLightApplied() ){
			// shading is on
			Color c = ip.specIllumOut;

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(c);
			}

			ClampMax( c );
			sc.out.elementVals[ mShadeOutputIndex ] = c;
			// always want full alpha w/ bake elements, for dilation
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;
		}// end, want shaded output
		else {
			// unshaded, raw-but-blended texture
			// could select among: specular color map, shininess & glossiness
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, use black instead
			sc.out.elementVals[ mShadeOutputIndex ] = ip.stdIDToChannel ? ip.channels[ ip.stdIDToChannel[ ID_SS ]] : AColor(0,0,0);
			// copy 
			sc.out.elementVals[ mShadeOutputIndex ].g = sc.out.elementVals[ mShadeOutputIndex ].b = sc.out.elementVals[ mShadeOutputIndex ].r;
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;
		}

	} // end, not bgnd

}

//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Illuminance bake render element -- Light
//
class LightBakeElement : public BaseBakeElement {
public:
		enum{
			shadowsOn = BaseBakeElement::fileType+1, // last element + 1
			directOn,
			indirectOn,
			targetMapSlot
		};

		static int paramNames[];

		LightBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return lightBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_LIGHT_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return TRUE; }

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's direct light applied state
		void SetDirectLightOn(BOOL on){
			mpParamBlk->SetValue( directOn, 0, on ); 
		}
		BOOL IsDirectLightOn() const{
			int	on;
			mpParamBlk->GetValue( directOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's indirect light applied state
		void SetIndirectLightOn(BOOL on){
			mpParamBlk->SetValue( indirectOn, 0, on ); 
		}
		BOOL IsIndirectLightOn() const{
			int	on;
			mpParamBlk->GetValue( indirectOn, 0, on, FOREVER );
			return on;
		}

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 3; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam < 4 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			for( int i = 0; i < 3; ++i ){
				if( strcmp( name, GetString( paramNames[i] ) ) == 0 )
					return i+1;
			}
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 )? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			switch( nParam ) {
				case 1: return IsShadowApplied();
				case 2: return IsDirectLightOn();
				case 3: return IsIndirectLightOn();
				default: return -1;
			}
		}
		void SetParamValue1( int nParam, int newVal ){
			switch( nParam ) {
				case 1: SetShadowApplied( newVal ); break;
				case 2: SetDirectLightOn( newVal ); break;
				case 3: SetIndirectLightOn( newVal ); break;
			}
		}

};

int LightBakeElement::paramNames[] = { IDS_SHADOWS_ON, IDS_DIRECT_ILLUM_ON, IDS_INDIRECT_ILLUM_ON };

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class LightBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new LightBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_LIGHT_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return lightBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("lightMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static LightBakeElementClassDesc lightBakeCD;
ClassDesc* GetLightBakeElementDesc() { return &lightBakeCD; }

IRenderElementParamDlg *LightBakeElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)lightBakeCD.CreateParamDialogs(ip, this);
}


// ---------------------------------------------
// Illuminance parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 lightBakeParamBlk(PBLOCK_NUMBER, _T("Lighting bake element parameters"), 0, &lightBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_BAKE, IDS_LIGHT_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	LightBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	LightBakeElement::directOn, _T("directOn"), TYPE_BOOL, 0, IDS_DIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIRECT_ON,
		p_default, TRUE,
		end,
	LightBakeElement::indirectOn, _T("indirectOn"), TYPE_BOOL, 0, IDS_INDIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_INDIRECT_ON,
		p_default, TRUE,
		end,
	LightBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);

//--- Illuminance Render Element ------------------------------------------------
LightBakeElement::LightBakeElement()
{
	lightBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle LightBakeElement::Clone( RemapDir &remap )
{
	LightBakeElement*	newEle = new LightBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void LightBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL || ! IsLightApplied() ){ // bgnd or no lighting = black
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		BOOL shadowSave = sc.shadow;
		sc.shadow = IsShadowApplied() ? TRUE : FALSE;

		sc.out.elementVals[ mShadeOutputIndex ] 
			= computeIlluminance( sc, IsIndirectLightOn(), IsDirectLightOn() );

		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;


		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
		}

		sc.shadow = shadowSave; // restore shadow
	}
}


//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Reflect / Refract bake render element
//
class ReflectRefractBakeElement : public BaseBakeElement {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		ReflectRefractBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return reflectRefractBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFLECTREFRACT_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return TRUE; }
		BOOL IsShadowApplied() const{ return TRUE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ReflectRefractBakeElementClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ReflectRefractBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFLECTREFRACT_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return reflectRefractBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("reflectRefractMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ReflectRefractBakeElementClassDesc reflectRefractBakeCD;
ClassDesc* GetReflectRefractBakeElementDesc() { return &reflectRefractBakeCD; }


// ---------------------------------------------
// Reflect-Refract parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 reflectRefractBakeParamBlk(PBLOCK_NUMBER, _T("Reflect/Refract bake element parameters"), 0, &reflectRefractBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	ReflectRefractBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);


//--- Reflect / Refract Bake Element ------------------------------------------------
ReflectRefractBakeElement::ReflectRefractBakeElement()
{
	reflectRefractBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle ReflectRefractBakeElement::Clone( RemapDir &remap )
{
	ReflectRefractBakeElement*	newEle = new ReflectRefractBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void ReflectRefractBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// reflection
 		Color c = ip.reflIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;

		// add in refraction
 		c = ip.transIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] += c;

		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Shadow baking element - high res, no color, alpha only
//
class ShadowBakeElement : public BaseBakeElement {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		ShadowBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		// note, we don't override the ShadowsApplied(){ return FALSE; }
		// we control it ourselves

		Class_ID ClassID() {return shadowBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SHADOW_BAKE_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		BOOL IsLightApplied() const{ return TRUE; }
		BOOL IsShadowApplied() const{ return TRUE; }
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ShadowBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ShadowBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SHADOW_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return shadowBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("shadowsMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ShadowBakeElementClassDesc shadowBakeCD;
ClassDesc* GetShadowBakeElementDesc() { return &shadowBakeCD; }

  
// ---------------------------------------------
// Shadow parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 shadowBakeParamBlk(PBLOCK_NUMBER, _T("Shadow render element parameters"), 0, &shadowBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	ShadowBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);

//--- Shadow Bake Element ------------------------------------------------
ShadowBakeElement::ShadowBakeElement()
{
	shadowBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle ShadowBakeElement::Clone( RemapDir &remap )
{
	ShadowBakeElement* newEle = new ShadowBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}

inline void Clamp( float& val ){ 
	if( val < 0.0f ) val = 0.0f;
	else if( val > 1.0f ) val = 1.0f;
}


void ShadowBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		Color sClr;
		float shadowAtten = IllumShadow( sc, sClr );
		float a = 1.0f-shadowAtten;

		float opac = 1.0f - Intens( sc.out.t );
		if( opac < 0.0f ) opac = 0.0f; 
		else if( opac > 1.0f ) opac = 1.0f;

		if(ip.pMtl->ClassID() != Class_ID(MATTE_CLASS_ID,0) )
			a *= opac;

		float a1 = 1.0f - a;

		if( a1 < 0.0f ) a1 = 0.0f; 
		else if( a1 > 1.0f ) a1 = 1.0f;

		sc.out.elementVals[ mShadeOutputIndex ]	= AColor( a1, a1, a1, opac );
	}
}

//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Normals Bake element
//		- texture bake the normals
//
class NormalsBakeElement : public BaseBakeElement {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		NormalsBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return normalsBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_NORMALS_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return FALSE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
//texture bake the normals class descriptor 
class NormalsBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new NormalsBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_NORMALS_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return normalsBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("normalsMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static NormalsBakeElementClassDesc normalsBakeCD;
ClassDesc* GetNormalsBakeElementDesc() { return &normalsBakeCD; }


// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 normalsBakeParamBlk(PBLOCK_NUMBER, _T("Normal map bake element parameters"), 0, &normalsBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	NormalsBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);


//--- Normal Map Baking Element ------------------------------------------------
NormalsBakeElement::NormalsBakeElement()
{
	normalsBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle NormalsBakeElement::Clone( RemapDir &remap )
{
	NormalsBakeElement* newEle = new NormalsBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void NormalsBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		Point3 delta = sc.OrigNormal() - sc.Normal();
		Point3 N(0.0,0.0,1.0);
		N = N +delta;
		N = N.Normalize();
		N = N + 1.0;
		N *= 0.5;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f; else if( a > 1.0f ) a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(N.x, N.y, N.z, a);
	}
}



///////////////////////////////////////////////////////////////////////////////
//
//	Complete Bake element
//
class CompleteBakeElement : public BaseBakeElement {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};

		static int paramNames[];

		CompleteBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return completeBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_COMPLETE_BAKE_ELEMENT ); }

				// set/get element's light applied state
//		void SetLightApplied(BOOL on){ 
//			mpParamBlk->SetValue( lightingOn, 0, on ); 
//		}
//		BOOL IsLightApplied() const{
//			int	on;
//			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
//			return on;
//		}
		BOOL IsLightApplied() const{ return TRUE; }


		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 1; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam == 1 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return ( nParam==1 )? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetShadowApplied( newVal);
		}
};

int CompleteBakeElement::paramNames[] = { IDS_SHADOWS_ON };

// --------------------------------------------------
//texture bake the complete illumination class descriptor 
class CompleteBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new CompleteBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_COMPLETE_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return completeBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("completeMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static CompleteBakeElementClassDesc completeBakeCD;
ClassDesc* GetCompleteBakeElementDesc() { return &completeBakeCD; }

IRenderElementParamDlg *CompleteBakeElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)completeBakeCD.CreateParamDialogs(ip, this);
}

// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 completeBakeParamBlk(PBLOCK_NUMBER, _T("Full Illumination bake element parameters"), 0, &completeBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_SHADOW, IDS_COMPLETE_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	CompleteBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	CompleteBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);


//--- Complete Illumination Baking Element ------------------------------------------------
CompleteBakeElement::CompleteBakeElement()
{
	completeBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle CompleteBakeElement::Clone( RemapDir &remap )
{
	CompleteBakeElement* newEle = new CompleteBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void CompleteBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{

	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		AColor c(0,0,0,0);
		c += ip.diffIllumOut + ip.ambIllumOut + ip.selfIllumOut;
		c *= ip.finalAttenuation;
		c += ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut;
		int n;
		if( IsShadowApplied() && (n = ip.FindUserIllumName( shadowIllumOutStr ))>=0 )
				c += ip.GetUserIllumOutput( n );

		// now add ink & paint
//		if( (n = ip.FindUserIllumName( inkIllumOutStr ))>=0 )
//				c += ip.GetUserIllumOutput( n );
		if( (n = ip.FindUserIllumName( paintIllumOutStr ))>=0 )
				c += ip.GetUserIllumOutput( n );

		c.a = Intens( sc.out.t );

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
//		sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;

		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f; else if( a > 1.0f ) a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Blend render element
//

class BlendBakeElement : public BaseBakeElement {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn, diffuseOn, ambientOn, specularOn, 
			emissionOn, reflectionOn, refractionOn, targetMapSlot
		};

		static int paramNames[];

		BlendBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return blendBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BLEND_BAKE_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		//attributes of the blend
		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		
		void SetDiffuseOn(BOOL on){ mpParamBlk->SetValue( diffuseOn, 0, on ); }
		BOOL IsDiffuseOn() const{
			int	on;
			mpParamBlk->GetValue( diffuseOn, 0, on, FOREVER );
			return on;
		}

		void SetAmbientOn(BOOL on){ mpParamBlk->SetValue( ambientOn, 0, on ); }
		BOOL IsAmbientOn() const{
			int	on;
			mpParamBlk->GetValue( ambientOn, 0, on, FOREVER );
			return on;
		}

		void SetSpecularOn(BOOL on){ mpParamBlk->SetValue( specularOn, 0, on ); }
		BOOL IsSpecularOn() const{
			int	on;
			mpParamBlk->GetValue( specularOn, 0, on, FOREVER );
			return on;
		}

		void SetEmissionOn(BOOL on){ mpParamBlk->SetValue( emissionOn, 0, on ); }
		BOOL IsEmissionOn() const{
			int	on;
			mpParamBlk->GetValue( emissionOn, 0, on, FOREVER );
			return on;
		}

		void SetReflectionOn(BOOL on){ mpParamBlk->SetValue( reflectionOn, 0, on ); }
		BOOL IsReflectionOn() const{
			int	on;
			mpParamBlk->GetValue( reflectionOn, 0, on, FOREVER );
			return on;
		}
		void SetRefractionOn(BOOL on){ mpParamBlk->SetValue( refractionOn, 0, on ); }
		BOOL IsRefractionOn() const{
			int	on;
			mpParamBlk->GetValue( refractionOn, 0, on, FOREVER );
			return on;
		}

		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 8; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam <= 8 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			for( int i = 0; i < 8; ++i ){
				if( strcmp( name, GetString( paramNames[i] )) == 0 )
					return i+1;
			}
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 )? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			switch( nParam ) {
				case 1: return IsShadowApplied();
				case 2: return IsLightApplied();
				case 3: return IsDiffuseOn();
				case 4: return IsAmbientOn();
				case 5: return IsSpecularOn();
				case 6: return IsEmissionOn();
				case 7: return IsReflectionOn();
				case 8: return IsRefractionOn();
				default: return -1;
			}
		}
		void SetParamValue1( int nParam, int newVal ){
			switch( nParam ) {
				case 1: SetShadowApplied( newVal ); break;
				case 2: SetLightApplied( newVal ); break;
				case 3: SetDiffuseOn( newVal ); break;
				case 4: SetAmbientOn( newVal ); break;
				case 5: SetSpecularOn( newVal ); break;
				case 6: SetEmissionOn( newVal ); break;
				case 7: SetReflectionOn( newVal ); break;
				case 8: SetRefractionOn( newVal ); break;
			}
		}

};

int BlendBakeElement::paramNames[] =	{ IDS_LIGHTING_ON, IDS_SHADOWS_ON,
										  IDS_DIFFUSE_ON, IDS_AMBIENT_ON,
										  IDS_SPECULAR_ON, IDS_EMISSION_ON,
										  IDS_REFLECTION_ON, IDS_REFRACTION_ON
										};


// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class BlendBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BlendBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLEND_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return blendBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("blendMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BlendBakeElementClassDesc blendBakeCD;
ClassDesc* GetBlendBakeElementDesc() { return &blendBakeCD; }

IRenderElementParamDlg *BlendBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = blendBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class BlendPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case BlendBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(BlendBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static BlendPBAccessor blendPBAccessor;



// ---------------------------------------------
// Blend parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 blendBakeParamBlk(PBLOCK_NUMBER, _T("Blend bake element parameters"), 0, &blendBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BLEND_BAKE_ELEMENT, IDS_BLEND_BAKE_PARAMS, 0, 0, NULL,
	// params
	// these from the base bake element
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,

	// blend specific
	BlendBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, TRUE,
		p_accessor,		&blendPBAccessor,
		end,
	BlendBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	BlendBakeElement::diffuseOn, _T("diffuseOn"), TYPE_BOOL, 0, IDS_DIFFUSE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIFFUSE_ON,
		end,
	BlendBakeElement::ambientOn, _T("ambientOn"), TYPE_BOOL, 0, IDS_AMBIENT_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_AMBIENT_ON,
		end,
	BlendBakeElement::specularOn, _T("specularOn"), TYPE_BOOL, 0, IDS_SPECULAR_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SPECULAR_ON,
		end,
	BlendBakeElement::emissionOn, _T("emissionOn"), TYPE_BOOL, 0, IDS_EMISSION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EMISSION_ON,
		end,
	BlendBakeElement::reflectionOn, _T("reflectionOn"), TYPE_BOOL, 0, IDS_REFLECTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFLECTION_ON,
		end,
	BlendBakeElement::refractionOn, _T("refractionOn"), TYPE_BOOL, 0, IDS_REFRACTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFRACTION_ON,
		end,
	BlendBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	); 

//--- Blend Render Element ------------------------------------------------
BlendBakeElement::BlendBakeElement()
{
	blendBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle BlendBakeElement::Clone( RemapDir &remap )
{
	BlendBakeElement*	newEle = new BlendBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void BlendBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	AColor c(0,0,0,0);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = c;
	} else {
		if( IsLightApplied() ){
			if( IsDiffuseOn() ) c += ip.diffIllumOut;
			if( IsAmbientOn() ) c += ip.ambIllumOut;
			if( IsEmissionOn() ) c += ip.selfIllumOut;
			c *= ip.finalAttenuation;
			if( IsSpecularOn() ) c += ip.specIllumOut;
			if( IsReflectionOn() ) c += ip.reflIllumOut;
			if( IsRefractionOn() ) c += ip.transIllumOut;
			int n;
			if( ShadowsApplied() && (n = ip.FindUserIllumName( shadowIllumOutStr ))>=0 )
				c += ip.GetUserIllumOutput( n );

			c.a = Intens( sc.out.t );

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(c);
			}

			ClampMax( c );

			sc.out.elementVals[ mShadeOutputIndex ] = c;
		} // end, lighting applied
		else {
			// no lighting, use sum of unshaded channels
			// >>>>>> shd these be overs rather than sums???
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, ignore if so
			if( IsDiffuseOn() && ip.stdIDToChannel)
				c += ip.channels[ ip.stdIDToChannel[ ID_DI ]];
			else if( IsAmbientOn() )
				c += ip.channels[ ip.stdIDToChannel[ ID_AM ]];
			else if( IsEmissionOn() ) 
				c += ip.channels[ ip.stdIDToChannel[ ID_SI ]];
			else if( IsSpecularOn() )
				c += ip.channels[ ip.stdIDToChannel[ ID_SP ]];
			else if( IsReflectionOn() )
				c += ip.channels[ ip.stdIDToChannel[ ID_RL ]];
			else if( IsRefractionOn() )
				c += ip.channels[ ip.stdIDToChannel[ ID_RR ]];

			sc.out.elementVals[ mShadeOutputIndex ] = c;
		}

		// apply alpha regardless of lighting on/off
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

void BlendBakeElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())
			&& sc.globContext->pToneOp->GetProcessBackground()) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
	}

	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}

///////////////////////////////////////////////////////////////////////////////
//
//	Alpha Bake element
//		- texture bake the alpha channel
//
class AlphaBakeElement : public BaseBakeElement {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		AlphaBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return alphaBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ALPHA_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return FALSE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
//texture bake the Alpha class descriptor 
class AlphaBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AlphaBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ALPHA_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return alphaBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("alphaMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AlphaBakeElementClassDesc alphaBakeCD;
ClassDesc* GetAlphaBakeElementDesc() { return &alphaBakeCD; }


// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 alphaBakeParamBlk(PBLOCK_NUMBER, _T("Alpha map bake element parameters"), 0, &alphaBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	AlphaBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, 0,
		p_default, "",
		end,
	end
	);


//--- Normal Map Baking Element ------------------------------------------------
AlphaBakeElement::AlphaBakeElement()
{
	alphaBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle AlphaBakeElement::Clone( RemapDir &remap )
{
	AlphaBakeElement* newEle = new AlphaBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void AlphaBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f; else if( a > 1.0f ) a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(a,a,a,a);
	}
}


//-----------------------------------------------------------------------------------------
