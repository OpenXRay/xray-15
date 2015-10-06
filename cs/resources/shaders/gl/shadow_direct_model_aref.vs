#include "common.h"
#include "skin.h"
#include "iostructs/v_shadow_direct_model_aref.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
v_shadow_direct_aref _main( v_model	I )
{
	v_shadow_direct_aref	O;
	float4 	hpos 	= mul	(m_WVP,	I.P	);
	O.hpos 	= hpos;
	O.tc0 	= I.tc;
#ifndef USE_HWSMAP
	O.depth = O.hpos.z;
#endif
 	return	O;
}
