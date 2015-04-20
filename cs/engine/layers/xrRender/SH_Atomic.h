#ifndef sh_atomicH
#define sh_atomicH
#pragma once
#include "../../xrCore/xr_resource.h"
#include "tss_def.h"

#ifdef	USE_DX10
#include "../xrRenderDX10/StateManager/dx10State.h"
#endif	//	USE_DX10

#pragma pack(push,4)


//////////////////////////////////////////////////////////////////////////
// Atomic resources
//////////////////////////////////////////////////////////////////////////
#ifdef	USE_DX10
struct ECORE_API SInputSignature : public xr_resource_flagged
{
	ID3DBlob*							signature;
	SInputSignature(ID3DBlob* pBlob);
	~SInputSignature();
};
typedef	resptr_core<SInputSignature,resptr_base<SInputSignature> >	ref_input_sign;
#endif	//	USE_DX10
//////////////////////////////////////////////////////////////////////////
struct ECORE_API SVS : public xr_resource_named							
{
#ifdef USE_OGL
	GLuint								vs;
#else
	ID3DVertexShader*					vs;
#endif // USE_OGL
	R_constant_table					constants;
#ifdef	USE_DX10
	ref_input_sign						signature;
#endif	//	USE_DX10
	SVS				();
	~SVS			();
};
typedef	resptr_core<SVS,resptr_base<SVS> >	ref_vs;

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SPS : public xr_resource_named
{
#ifdef USE_OGL
	GLuint								ps;
#else
	ID3DPixelShader*					ps;
#endif // USE_OGL

	R_constant_table					constants;
	~SPS			();
};
typedef	resptr_core<SPS,resptr_base<SPS> > ref_ps;

#ifdef	USE_DX10
//////////////////////////////////////////////////////////////////////////
struct ECORE_API SGS : public xr_resource_named
{
	ID3DGeometryShader*					gs;
	R_constant_table					constants;
	~SGS			();
};
typedef	resptr_core<SGS,resptr_base<SGS> > ref_gs;
#endif	//	USE_DX10

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SState : public xr_resource_flagged
{
#ifndef	USE_OGL
	ID3DState*							state;
#endif // USE_OGL
	SimulatorStates						state_code;
	~SState			();
};
typedef	resptr_core<SState, resptr_base<SState> >	ref_state;

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SDeclaration : public xr_resource_flagged
{
#ifdef USE_OGL
	GLuint								dcl;
#else

#ifdef	USE_DX10
	//	Maps input signature to input layout
	xr_map<ID3DBlob*, ID3D10InputLayout*>	vs_to_layout;
	xr_vector<D3D10_INPUT_ELEMENT_DESC>		dx10_dcl_code;
#else	//	USE_DX10	//	Don't need it: use ID3D10InputLayout instead
					//	which is per ( declaration, VS input layout) pair
	IDirect3DVertexDeclaration9*		dcl;
#endif	//	USE_DX10

	//	Use this for DirectX10 to cache DX9 declaration for comparison purpose only
	xr_vector<D3DVERTEXELEMENT9>		dcl_code;

#endif // USE_OGL
	~SDeclaration	();
};
typedef	resptr_core<SDeclaration, resptr_base<SDeclaration> >	ref_declaration;

#pragma pack(pop)
#endif //sh_atomicH
