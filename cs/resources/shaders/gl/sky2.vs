#include "common.h"
//
// Translator library functions
//

vec4 xlat_lib_tex2Dlod(sampler2D s, vec4 coord) {
   return texture2DLod( s, coord.xy, coord.w);
}

//
// Structure definitions
//

struct vf {
    vec4 hpos;
    vec4 c;
    vec3 tc0;
    vec3 tc1;
};

struct vi {
    vec4 p;
    vec4 c;
    vec3 tc0;
    vec3 tc1;
};

//
// Function declarations
//

vf xlat_main( in vi v );

//
// Function definitions
//

vf xlat_main( in vi v ) {
    vf o;

    o.hpos = ( m_WVP * v.p );
    o.tc0 = v.tc0;
    o.tc1 = v.tc1;
#ifdef USE_VTF
    scale = xlat_lib_tex2Dlod( s_tonemap, vec4( 0.500000, 0.500000, 0.500000, 0.500000)).x ;
    o.c = vec4( ((v.c.xyz  * scale) * 2.00000), v.c.w );
#else
    o.c = v.c;
#endif
    return o;
}


//
// Translator's entry point
//
void main() {
    vf xlat_retVal;
    vi xlat_temp_v;
    xlat_temp_v.p = vec4( gl_Vertex);
    xlat_temp_v.c = vec4( gl_Color);
    xlat_temp_v.tc0 = vec3( gl_MultiTexCoord0);
    xlat_temp_v.tc1 = vec3( gl_MultiTexCoord1);

    xlat_retVal = xlat_main( xlat_temp_v);

    gl_Position = vec4( xlat_retVal.hpos);
    gl_FrontColor = vec4( xlat_retVal.c);
    gl_TexCoord[0] = vec4( xlat_retVal.tc0, 0.0);
    gl_TexCoord[1] = vec4( xlat_retVal.tc1, 0.0);
}
