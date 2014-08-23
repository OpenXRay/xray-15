/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	22.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_reflect
 *	mib_reflect_version
 *
 * History:
 *	22.01.98: changed type of reflect to miColor
 *	26.01.98: added notrace flag and mi_trace_environment
 *	27.01.98: rewrote the shader, didn't calculate direction and
 *		  didn't check trace depth, didn't save ior
 *
 * Description:
 *	Reflection node
 *
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct mr {
	miColor		input;
	miColor		reflect;
	miBoolean	notrace;
};


extern "C" DLLEXPORT int mib_reflect_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_reflect(
	miColor		*result,
	miState		*state,
	struct mr	*paras)
{
	miBoolean	ok;
	miBoolean	notrace;
	miColor		*reflect =  mi_eval_color(&paras->reflect);
	miColor		inp;
	miVector	dir;
	miScalar	save_ior;

        /* check for illegal calls */
        if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		return(miFALSE);
	}

	if (reflect->r == 0.0 && reflect->g == 0.0 &&
	    reflect->b == 0.0 && reflect->a == 0.0) {
		*result = *mi_eval_color(&paras->input);
		return(miTRUE);
	}
	notrace    = *mi_eval_boolean(&paras->notrace);
	save_ior   = state->ior;
	state->ior = state->ior_in;

	mi_reflection_dir(&dir, state);
	ok = miFALSE;
	if (!notrace &&
	    state->reflection_level < state->options->reflection_depth &&
	    state->reflection_level + state->refraction_level <
						state->options->trace_depth)
		ok = mi_trace_reflection(result, state, &dir);

	if (!ok) {
		miTag savevol = state->volume;
		state->volume = 0;
		ok = mi_trace_environment(result, state, &dir) || !notrace;
		state->volume = savevol;
	}

	if (reflect->r != 1.0 || reflect->g != 1.0 ||
	    reflect->b != 1.0 || reflect->a != 1.0) {
		inp = *mi_eval_color(&paras->input);
		result->r = result->r * reflect->r + inp.r * (1 - reflect->r);
		result->g = result->g * reflect->g + inp.g * (1 - reflect->g);
		result->b = result->b * reflect->b + inp.b * (1 - reflect->b);
		result->a = result->a * reflect->a + inp.a * (1 - reflect->a);
	}
	state->ior = save_ior;
	return(ok);
}
