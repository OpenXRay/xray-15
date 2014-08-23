///////////////////////////////////////////////////////////////
//
//	Standard TextureBaking Render Elements
//

#ifndef STD_BAKE_ELEMENTS_H
#define STD_BAKE_ELEMENTS_H

#include "renderElements.h"
#include "renElemRes.h"


// Class ids

// default is defined in include/renderelements.h
//#define DEFAULT_BAKE_ELEMENT_CLASS_ID			0x00010001
#define COMPLETE_BAKE_ELEMENT_CLASS_ID			0x00010001
#define SPECULAR_BAKE_ELEMENT_CLASS_ID			0x00010002
#define DIFFUSE_BAKE_ELEMENT_CLASS_ID			0x00010003
#define REFLECT_REFRACT_BAKE_ELEMENT_CLASS_ID	0x00010004
#define SHADOW_BAKE_ELEMENT_CLASS_ID			0x00010005
#define LIGHT_BAKE_ELEMENT_CLASS_ID				0x00010006
#define NORMALS_ELEMENT_CLASS_ID					0x00010007
#define BLEND_BAKE_ELEMENT_CLASS_ID				0x00010008
#define ALPHA_BAKE_ELEMENT_CLASS_ID				0x00010009

//#define EMISSION_BAKE_ELEMENT_CLASS_ID		0x00010009
//#define Z_RENDER_ELEMENT_CLASS_ID				0x0001000a
//#define CLR_SHADOW_RENDER_ELEMENT_CLASS_ID	0x0001000c
//#define BGND_RENDER_ELEMENT_CLASS_ID			0x0001000d


// Bake Element Class Descriptors
//extern ClassDesc* GetDefaultBakeElementDesc();
extern ClassDesc* GetCompleteBakeElementDesc();
extern ClassDesc* GetSpecularBakeElementDesc();
extern ClassDesc* GetDiffuseBakeElementDesc();
extern ClassDesc* GetReflectRefractBakeElementDesc();
extern ClassDesc* GetLightBakeElementDesc();
extern ClassDesc* GetShadowBakeElementDesc();
extern ClassDesc* GetBlendBakeElementDesc();
extern ClassDesc* GetNormalsBakeElementDesc();
extern ClassDesc* GetAlphaBakeElementDesc();

////////////////////////////////////////////////////////////////////////////
//	bake render elements shared base class
//
#define BAKE_ELEMENT_INTERFACE Interface_ID(0x94694761, 0x17622221)


class BaseBakeElement: public MaxBakeElement, public FPMixinInterface {
	protected:
		Bitmap*		mpRenderBitmap;	//render to this one, max size
		IParamBlock2 *mpParamBlk;	// save to the pb bitmap: scale on save.

		static FPInterfaceDesc _fpInterfaceDesc; // function publishing

	public:

		// shared pblock parameter ids
		enum { enableOn,
			   filterOn,
			   eleName,
			   pbBitmap,	// this is the bitmap to save to, of outputSz

			   outputSzX, outputSzY,	// save to a bitmap this size.
			   autoSzOn,
			   fileName,
			   fileNameUnique,
			   fileType,
		};

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
		IParamMap2* GetMap(){ return mpParamBlk ? mpParamBlk->GetMap() : NULL; }

		// -- From FPInterface
		virtual FPInterfaceDesc* GetDesc() ; 

		// enable & disable the render element
		void SetEnabled(BOOL on){ mpParamBlk->SetValue( enableOn, 0, on ); }
		BOOL IsEnabled() const{
			int	on;
			mpParamBlk->GetValue( enableOn, 0, on, FOREVER );
			return on;
		}

		// enable & disable auto sizing for the render element
		void SetAutoSzOn(BOOL on){ mpParamBlk->SetValue( autoSzOn, 0, on ); }
		BOOL IsAutoSzOn() const{
			int	on;
			mpParamBlk->GetValue( autoSzOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's filter enabled state
		void SetFilterEnabled(BOOL on){ 
			mpParamBlk->SetValue( filterOn, 0, on ); 
		}
		BOOL IsFilterEnabled() const{
			int	on;
			mpParamBlk->GetValue( filterOn, 0, on, FOREVER );
			return on;
		}

		void SetFileNameUnique(BOOL on){
			mpParamBlk->SetValue( fileNameUnique, 0, on ); 
		}
		BOOL IsFileNameUnique() const {
			int	on;
			mpParamBlk->GetValue( fileNameUnique, 0, on, FOREVER );
			return on;
		}

		// these are overridden by 
		// set/get element's light applied state
		void SetLightApplied(BOOL on){}
		BOOL IsLightApplied() const{ return TRUE; }

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){}
		BOOL IsShadowApplied() const{ return FALSE;	}

		// set/get element's output size, shd always be square
		void SetOutputSz( int  xSz, int ySz ){
			mpParamBlk->SetValue( outputSzX, 0, xSz ); 
			mpParamBlk->SetValue( outputSzY, 0, ySz ); 
		}
		void GetOutputSz( int&  xSz, int& ySz ) const{
			mpParamBlk->GetValue( outputSzX, 0, xSz, FOREVER );
			mpParamBlk->GetValue( outputSzY, 0, ySz, FOREVER );
		}

		// set/get element's atmosphere applied state
		void SetAtmosphereApplied(BOOL on){}
		// think this is true for all the bake elements
		BOOL IsAtmosphereApplied() const{ return FALSE;	}


		// set/get the renderingbitmap, max of sizes
		// no memory management of the bitmap!
		void SetRenderBitmap( Bitmap* pBitmap ){
			mpRenderBitmap = pBitmap;
		}
		Bitmap*  GetRenderBitmap() const{
			return mpRenderBitmap;
		}

		// multipass shd never come w/ texture baking
		BOOL BlendOnMultipass() const { return FALSE; }

		// set/get element's name (as it will appear in render dialog)
		void SetElementName( TCHAR* newName){ mpParamBlk->SetValue( eleName, 0, newName ); }
		const TCHAR* ElementName() const {
			TCHAR* pStr = NULL;
			mpParamBlk->GetValue( eleName, 0, pStr, FOREVER );
			return pStr;
		}

		void SetFileName( TCHAR* newName){
			mpParamBlk->SetValue( fileName, 0, newName ); 
		}
		const TCHAR* GetFileName() const {
			TCHAR* pStr = NULL;
			mpParamBlk->GetValue( fileName, 0, pStr, FOREVER );
			return pStr;
		}
		void SetFileType( TCHAR* newType){ 
			mpParamBlk->SetValue( fileType, 0, newType ); 
		}
		const TCHAR* GetFileType() const {
			TCHAR* pStr = NULL;
			mpParamBlk->GetValue( fileType, 0, pStr, FOREVER );
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

		// these allow us to build dynamic ui for the elements in maxscript
		// currently these are always checkboxs
		// defaults to no params
		// the 1-based index group is implemented in each supporting object
		// the 0-based groups shells thru to the 1-based group
		int  GetNParams() const { return 0; }

		// 1 based param numbering
		virtual const TCHAR* GetParamName1( int nParam ) { return NULL; }
		virtual const int FindParamByName1( TCHAR*  name ) { return 0; }
		// currently only type 0 == undefined, 1 == boolean
		virtual int  GetParamType1( int nParam ) { return 0; }
		virtual int  GetParamValue1( int nParam ){ return -1; }
		virtual void SetParamValue1( int nParam, int newVal ){}

		// 0 based index set. these just shell to the 1 based
		const TCHAR* GetParamName( int nParam ) { 
			return GetParamName1( nParam + 1 );
		}
		const int FindParamByName( TCHAR*  name ) {
			return( FindParamByName1( name ) - 1 );
		}
		int  GetParamType( int nParam ) { 
			return GetParamType1( nParam + 1);
		}
		int  GetParamValue( int nParam ){
			return GetParamValue1( nParam + 1 );
		}
		void SetParamValue( int nParam, int newVal ){
			SetParamValue1( nParam + 1, newVal );
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

		// Function publishing interface	
		enum FPS_ID {
			_nParams,
			_ParamName,
			_ParamType,
			_GetParamValue,
			_SetParamValue,
			_FindParam,
		};

		BEGIN_FUNCTION_MAP
			// Property accessors
//			PROP_FNS(_GetBakeEnabled, GetBakeEnabled, _SetBakeEnabled, SetBakeEnabled, TYPE_BOOL);

			FN_0( _nParams, TYPE_INT, GetNParams );
			FN_1( _ParamName, TYPE_STRING, GetParamName1, TYPE_INT );
			FN_1( _ParamType, TYPE_INT, GetParamType1, TYPE_INT );
			FN_1( _GetParamValue, TYPE_INT, GetParamValue1, TYPE_INT );
			VFN_2( _SetParamValue, SetParamValue1, TYPE_INT, TYPE_INT );
			FN_1( _FindParam, TYPE_INT, FindParamByName1, TYPE_STRING );

		END_FUNCTION_MAP

	};

#define PBLOCK_REF	0
#define PBLOCK_NUMBER	0

#endif

