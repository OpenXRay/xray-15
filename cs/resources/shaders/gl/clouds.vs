#include "common.h"
#include "shared/cloudconfig.h"

//
// Structure definitions
//

struct vf {
    vec4 hpos;
    vec4 color;
    vec2 tc0;
    vec2 tc1;
};

struct vi {
    vec4 p;
    vec4 dir;
    vec4 color;
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
    vec2 d0;
    vec2 d1;
    vec2 _0;
    vec2 _1;

    o.hpos = ( m_WVP * v.p );
    d0 = ((v.dir.xy  * 2.00000) - 1.00000);
    d1 = ((v.dir.wz  * 2.00000) - 1.00000);
    _0 = ((v.p.xz  * 0.700000) + ((d0 * timers.z ) * 0.100000));
    _1 = ((v.p.xz  * 2.80000) + ((d1 * timers.z ) * 0.0500000));
    o.tc0 = _0;
    o.tc1 = _1;
    o.color = v.color;
    o.color.w  *= pow( v.p.y , 25.0000);
    return o;
}


//
// Translator's entry point
//
void main() {
    vf xlat_retVal;
    vi xlat_temp_v;
    xlat_temp_v.p = vec4( gl_Vertex);
    xlat_temp_v.dir = vec4( gl_Color);
    xlat_temp_v.color = vec4( gl_SecondaryColor);

    xlat_retVal = xlat_main( xlat_temp_v);

    gl_Position = vec4( xlat_retVal.hpos);
    gl_FrontColor = vec4( xlat_retVal.color);
    gl_TexCoord[0] = vec4( xlat_retVal.tc0, 0.0, 0.0);
    gl_TexCoord[1] = vec4( xlat_retVal.tc1, 0.0, 0.0);
}

