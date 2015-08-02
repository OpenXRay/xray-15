#include "common.h"

//
// Structure definitions
//

struct vf {
    vec4 hpos;
    vec4 C;
};


//
// Global variable definitions
//

uniform vec4 tfactor;

//
// Function declarations
//

vf xlat_main( in vec4 P );

//
// Function definitions
//

vf xlat_main( in vec4 P ) {
    vf o;

    o.hpos = ( m_WVP * P );
    o.C = tfactor;
    return o;
}


//
// Translator's entry point
//
void main() {
    vf xlat_retVal;

    xlat_retVal = xlat_main( vec4(gl_Vertex));

    gl_Position = vec4( xlat_retVal.hpos);
    gl_FrontColor = vec4( xlat_retVal.C);
}
