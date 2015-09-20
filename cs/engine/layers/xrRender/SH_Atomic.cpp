#include "stdafx.h"
#pragma hdrstop

#include "sh_atomic.h"
#include "ResourceManager.h"

#ifdef USE_OGL
#	include "glRenderDeviceRender.h"
#else
#	include "dxRenderDeviceRender.h"
#endif // USE_OGL

// Atomic
//SVS::~SVS								()			{	_RELEASE(vs);		dxRenderDeviceRender::Instance().Resources->_DeleteVS			(this);	}
//SPS::~SPS								()			{	_RELEASE(ps);		dxRenderDeviceRender::Instance().Resources->_DeletePS			(this);	}
//SState::~SState							()			{	_RELEASE(state);	dxRenderDeviceRender::Instance().Resources->_DeleteState		(this);	}
//SDeclaration::~SDeclaration				()			{	_RELEASE(dcl);		dxRenderDeviceRender::Instance().Resources->_DeleteDecl		(this);	}

///////////////////////////////////////////////////////////////////////
//	SVS
SVS::SVS() :
	vs(0)
#ifdef	USE_DX10
//	,signature(0)
#endif	//	USE_DX10
{
	;
}


SVS::~SVS()
{
	DEV->_DeleteVS(this);
#ifdef	USE_DX10
	//_RELEASE(signature);
	//	Now it is release automatically
#endif	//	USE_DX10
#ifdef USE_OGL
	CHK_GL(glDeleteProgram(vs));
#else
	_RELEASE(vs);
#endif // USE_OGL
}


///////////////////////////////////////////////////////////////////////
//	SPS
#ifdef USE_OGL
SPS::~SPS								()			{	 CHK_GL(glDeleteProgram(ps));	DEV->_DeletePS			(this);	}
#else
SPS::~SPS								()			{	_RELEASE(ps);		DEV->_DeletePS			(this);	}
#endif // USE_OGL

#ifdef	USE_DX10
///////////////////////////////////////////////////////////////////////
//	SGS
SGS::~SGS								()			{	_RELEASE(gs);		DEV->_DeleteGS			(this);	}

///////////////////////////////////////////////////////////////////////
//	SInputSignature
SInputSignature::SInputSignature(ID3DBlob* pBlob)	{ VERIFY(pBlob); signature=pBlob; signature->AddRef();};
SInputSignature::~SInputSignature		()			{	_RELEASE(signature); DEV->_DeleteInputSignature(this); }
#endif	//	USE_DX10

///////////////////////////////////////////////////////////////////////
//	SState
#ifdef USE_OGL
SState::~SState							()			{	DEV->_DeleteState		(this);	}
#else
SState::~SState							()			{	_RELEASE(state);	DEV->_DeleteState		(this);	}
#endif // USE_OGL

///////////////////////////////////////////////////////////////////////
//	SDeclaration
SDeclaration::~SDeclaration()
{	
	DEV->_DeleteDecl(this);	
#ifdef	USE_DX10
	xr_map<ID3DBlob*, ID3D10InputLayout*>::iterator iLayout;
	iLayout = vs_to_layout.begin();
	for( ; iLayout != vs_to_layout.end(); ++iLayout)
	{
		//	Release vertex layout
		_RELEASE(iLayout->second);
	}
#else	//	USE_DX10
	//	Release vertex layout
#ifdef USE_OGL
	glDeleteBuffers(1, &vao);
#else
	_RELEASE(dcl);
#endif // USE_OGL
#endif	//	USE_DX10
}
