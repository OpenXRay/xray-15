#include "common.h"
#include "iostructs/v_shadow_direct.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
v_shadow_direct _main ( a2v I )
{
	v_shadow_direct	O;

	O.hpos 	= mul		(m_WVP,	I.P	);
#ifndef USE_HWSMAP
	O.depth = O.hpos.z;
#endif
 	return	O;
}
