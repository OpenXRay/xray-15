#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 	P;

layout(location = TEXCOORD0) out vec4 	tc;
#ifdef 	USE_SJITTER
layout(location = TEXCOORD1) out vec4 	tcJ;
#endif

//////////////////////////////////////////////////////////////////////////////////////////
uniform float4x4	m_texgen;
#ifdef	USE_SJITTER
uniform float4x4	m_texgen_J;
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position	= mul( m_WVP, P );
	tc 			= mul( m_texgen, P );
#ifdef	USE_SJITTER
	tcJ	 		= mul( m_texgen_J, P );
#endif
}
