/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_dielectric
 *	mib_dielectric_version
 *
 * History:
 *      26.08.02 pa: check for totally internal reflection
 *      11.11.03 alf: fixed absorption == 0.0
 *
 * Description:
 *	Dielectric material
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct md {
	miColor		input;
	miColor		absorb;
	miScalar	refract;
	miScalar	ior;
};


extern "C" DLLEXPORT int mib_dielectric_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_dielectric(
	miColor		*result,
	miState		*state,
	struct md	*paras)
{
	miScalar	refract;
	miColor		inp, absorb;
	miVector	dir;
	miScalar	ior;
	double	        dist;
 
        /* check for illegal calls */
	if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE)
       		return(miFALSE);
	
	refract = *mi_eval_scalar(&paras->refract);
	if (refract == 0.0)
		*result = *mi_eval_color(&paras->input);
	else {
		ior = *mi_eval_scalar(&paras->ior);
		if (ior==0.0 || ior==1.0)
			mi_trace_transparent(result, state);
		else {
			if (mi_refraction_dir(&dir, state, 1.0, ior))
				mi_trace_refraction(result, state, &dir);
			else {	/* total internal reflection */
				mi_reflection_dir(&dir, state);
				mi_trace_reflection(result, state, &dir);
			}
		}
                dist = state->child ? state->child->dist : 0.0;
		absorb = *mi_eval_color(&paras->absorb);
		if(absorb.r > 0.0f)
			result->r *= (miScalar) exp(log(absorb.r) * dist);
		if(absorb.g > 0.0f)
			result->g *= (miScalar) exp(log(absorb.g) * dist);
		if(absorb.b > 0.0f)
			result->b *= (miScalar) exp(log(absorb.b) * dist);
		if(absorb.a > 0.0f)
			result->a *= (miScalar) exp(log(absorb.a) * dist);

		if (refract < 1.0f) {
			inp = *mi_eval_color(&paras->input);
			result->r = result->r * refract + inp.r * (1-refract);
			result->g = result->g * refract + inp.g * (1-refract);
			result->b = result->b * refract + inp.b * (1-refract);
			result->a = result->a * refract + inp.a * (1-refract);
		}
	}

	return(miTRUE);
}
