//////////////////////////////////////////////////
//  All comments by Nivenhbro are preceded by !
/////////////////////////////////////////////////


#ifndef SHARED_COMMON_H
#define SHARED_COMMON_H
//
uniform mat3x4	    m_W;
uniform mat3x4	    m_V;
uniform mat4x4 	    m_P;
uniform mat3x4	    m_WV;
uniform mat4x4  	m_VP;
uniform mat4x4 	    m_WVP;
uniform mat4x4 	    m_texgen;
uniform mat4x4 	    mVPTexgen;
uniform vec4		timers;
uniform vec4		fog_plane;
uniform vec4		fog_params;		// x=near*(1/(far-near)), ?,?, w = -1/(far-near)
uniform vec4		fog_color;
uniform vec3		L_sun_color;
uniform vec3		L_sun_dir_w;
uniform vec3		L_sun_dir_e;
uniform vec4		L_hemi_color;
uniform vec4		L_ambient;		// L_ambient.w = skynbox-lerp-factor
uniform vec3 		eye_position;
uniform vec3		eye_direction;
uniform vec3		eye_normal;
uniform	vec4 		dt_params;

vec3 	unpack_normal	(vec3 v)	{ return 2*v-1;			}
vec3 	unpack_bx2	(vec3 v)	{ return 2*v-1; 		}
vec3 	unpack_bx4	(vec3 v)	{ return 4*v-2; 		} //!reduce the amount of stretching from 4*v-2 and increase precision

vec2 	unpack_tc_base	(vec2 tc, float du, float dv)		{
		return (tc.xy + vec2	(du,dv))*(32.f/32768.f);	//!Increase from 32bit to 64bit floating point
}

vec2 	unpack_tc_lmap	(vec2 tc)	{ return tc*(1.f/32768.f);	} // [-1  .. +1 ] 

float 	calc_cyclic 	(float x)				{
	float 	phase 	= 1/(2*3.141592653589f);
	float 	sqrt2	= 1.4142136f;
	float 	sqrt2m2	= 2.8284271f;
	float 	f 	= sqrt2m2*frac(x)-sqrt2;	// [-sqrt2 .. +sqrt2] !No changes made, but this controls the grass wave (which is violent if I must say)
	return 	f*f - 1.f;				// [-1     .. +1]
}
vec2 	calc_xz_wave 	(vec2 dir2D, float frac)		{
	// Beizer
	vec2  ctrl_A	= vec2(0.f,		0.f	);
	vec2 	ctrl_B	= vec2(dir2D.x,	dir2D.y	);
	return  lerp	(ctrl_A, ctrl_B, frac);			//!This calculates tree wave. No changes made
}

#endif
