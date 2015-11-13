#include "common.h"
#include "iostructs/v_particle.h"

vf _main (vv v)
{
	vf 		o;

	o.hpos 		= mul	(m_WVP, v.P);		// xform, input in world coords
//	o.hpos 		= mul	(m_VP, v.P);		// xform, input in world coords
	o.tc		= v.tc;				// copy tc
	o.c		= v.c;				// copy color

//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	o.tctexgen 	= mul( mVPTexgen, v.P);
	o.tctexgen.z	= o.hpos.z;
#endif	//	USE_SOFT_PARTICLES

	return o;
}
