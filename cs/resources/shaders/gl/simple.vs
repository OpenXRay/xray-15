#include "common.h"
#include "iostructs\v_model.h"

vf main (v_model v)
{
	vf 		o;

	o.hpos 		= mul	(m_WVP, v.P);		// xform, input in world coords
	o.tc0		= v.tc;				// copy tc

	return o;
}
