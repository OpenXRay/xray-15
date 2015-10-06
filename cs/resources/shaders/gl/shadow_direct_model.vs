#include "common.h"
#include "skin.h"
#include "iostructs/v_shadow_direct_model.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
v_shadow_direct _main( v_model	I )
{
	v_shadow_direct	O ;
	float4 	hpos 	= mul( m_WVP, I.P );

	O.hpos 			= hpos;
#ifndef USE_HWSMAP
	O.depth 		= O.hpos.z;
#endif
 	return			O ;
}
