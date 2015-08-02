#include "common.h"

//
// Structure definitions
//

struct vf {
    vec4 P;
    vec4 C;
};


//
// Global variable definitions
//

uniform vec4 tfactor;

//
// Function declarations
//

vf xlat_main( in vf i );

//
// Function definitions
//

vf xlat_main( in vf i ) {
    vf o;

    o.P = ( m_WVP * i.P );
    o.C = (tfactor * i.C);
    return o;
}


//
// Translator's entry point
//
void main() {
    vf xlat_retVal;
    vf xlat_temp_i;
    xlat_temp_i.P = vec4( gl_Vertex);
    xlat_temp_i.C = vec4( gl_Color);

    xlat_retVal = xlat_main( xlat_temp_i);

    gl_Position = vec4( xlat_retVal.P);
    gl_FrontColor = vec4( xlat_retVal.C);
}
