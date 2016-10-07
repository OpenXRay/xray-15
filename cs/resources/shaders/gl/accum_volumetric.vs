#include "common.h"

uniform vec3	vMinBounds;
uniform vec3	vMaxBounds;
uniform vec4	FrustumClipPlane[6];

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_ClipDistance[6];
};

layout(location = POSITION) in vec3	P;

layout(location = TEXCOORD0) out vec3	lightToPos;	// light center to plane vector
layout(location = TEXCOORD1) out vec3	hPos;		// position in camera space
layout(location = TEXCOORD2) out float	fDensity;	// plane density alon Z axis
//layout(location = TEXCOORD3) out vec2	tNoise;		// projective noise

void main ()
{
	float4	vPos;
	vPos.xyz 	= lerp( vMinBounds, vMaxBounds, P);	//	Position in camera space
	vPos.w 		= 1;
	gl_Position	= mul			(m_P, vPos);		// xform, input in camera coordinates

	lightToPos = vPos.xyz - Ldynamic_pos.xyz;
	hPos = vPos.xyz;

//	fDensity = (vMaxBounds.z-vMinBounds.z)/2000.0f;
//	fDensity = (vMaxBounds.z-vMinBounds.z)/2000.0f*2;
	fDensity = 1.0f/40.0f;
//	fDensity = 1.0f/20.0f;

	for (int i=0; i<6; ++i)
	{
		gl_ClipDistance[i] = dot( vPos, FrustumClipPlane[i]);
	}
}
