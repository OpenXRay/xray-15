/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	23.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_refract
 *	mib_refract_version
 *
 * History:
 *	22.01.98: changed type of refract to miColor
 *	19.07.00: check for totally internal reflection
 *
 * Description:
 *	Refraction node
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct mr {
	miColor		input;
	miColor		refract;
	miScalar	ior;
};


extern "C" DLLEXPORT int mib_refract_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_refract(
	miColor		*result,
	miState		*state,
	struct mr	*paras)
{
	miColor		*refract = mi_eval_color(&paras->refract);
	miColor		inp;
	miVector	dir;
	miScalar	ior;
        
        /* check for illegal calls */
        if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		return(miFALSE);
	}


	if (refract->r == 0.0 && refract->g == 0.0 &&
	    refract->b == 0.0 && refract->a == 0.0)
		*result = *mi_eval_color(&paras->input);
	else {
		ior = *mi_eval_scalar(&paras->ior);
		if (ior == 0.0 || ior == 1.0)
			mi_trace_transparent(result, state);
		else {
			if (mi_refraction_dir(&dir, state, 1.0, ior))
				mi_trace_refraction(result, state, &dir);
			else {	/* total internal reflection */
				mi_reflection_dir(&dir, state);
				mi_trace_reflection(result, state, &dir);
			}
		}
		if (refract->r != 1.0 || refract->g != 1.0 ||
		    refract->b != 1.0 || refract->a != 1.0) {
			inp = *mi_eval_color(&paras->input);
			result->r = result->r * refract->r +
					inp.r * (1.0F - refract->r);
			result->g = result->g * refract->g +
					inp.g * (1.0F - refract->g);
			result->b = result->b * refract->b +
					inp.b * (1.0F - refract->b);
			result->a = result->a * refract->a +
					inp.a * (1.0F - refract->a);
		}
	}
	return(miTRUE);
}
