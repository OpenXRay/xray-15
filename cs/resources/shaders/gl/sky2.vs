#include "common.h"

in  vec4 iP;
in  vec4 iC;
in  vec3 iTC0;
in  vec3 iTC1;

out vec4 oC;
out vec3 oTC0;
out vec3 oTC1;

void main (void)
{
        gl_Position	        = iP * m_WVP;										// xform, input in world coords
        oTC0                = iTC0;                        						// copy tc
        oTC1                = iTC1;                        						// copy tc
#ifdef USE_VTF
        float	scale		= textureLod	(s_tonemap,vec3(.5,.5,.5),.5).x ;
        oC                	= vec4	( iC.rgb*scale*2, iC.a );      				// copy color, pre-scale by tonemap //float4 ( iC.rgb*scale*2, iC.a );
#else
        oC                	= iC;												// copy color, low precision
#endif
}
