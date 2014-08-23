/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	24.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_transparency
 *	mib_transparency_version
 *	mib_opacity
 *	mib_opacity_version
 *	mib_continue
 *	mib_continue_version
 *
 * History:
 *      24.10.97: initial version
 *	12.01.98: added mib_opacity
 *	22.01.98: transp and opacity are now of type miColor
 *      14.05.04: added mib_continue
 *
 * Description:
 *	Transparency nodes
 *****************************************************************************/

#include <shader.h>

struct mt {
	miColor input;
	miColor transp;
};

extern "C" DLLEXPORT int mib_transparency_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_transparency(
	miColor		*result,
	miState		*state,
	struct mt	*paras)
{
	miColor		inp;
        miColor		*transp;

        /* check for illegal calls */
        if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE) {
                result->r = result->g = result->b = result->a = 0.0;
		return miTRUE;
	}
        
	transp = mi_eval_color(&paras->transp);

	if (transp->r == 0.0 && transp->g == 0.0 &&
	    transp->b == 0.0 && transp->a == 0.0)
		*result = *mi_eval_color(&paras->input);
	else {
		mi_trace_transparent(result, state);

		if (transp->r != 1.0 || transp->g != 1.0 ||
		    transp->b != 1.0 || transp->a != 1.0) {
			inp = *mi_eval_color(&paras->input);
			result->r = result->r * transp->r +
						inp.r * (1.0 - transp->r);
			result->g = result->g * transp->g +
						inp.g * (1.0 - transp->g);
			result->b = result->b * transp->b +
						inp.b * (1.0 - transp->b);
			result->a = result->a * transp->a +
						inp.a * (1.0 - transp->a);
		}
	}

	return miTRUE;
}


/*-------------------------------------------------------------------------*/

struct mo {
	miColor		input;
	miColor		opacity;
};

extern "C" DLLEXPORT int mib_opacity_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_opacity(
	miColor		*result,
	miState		*state,
	struct mo	*paras)
{
        miColor         inp;
        miColor*        opacity;

        /* check for illegal calls */
        if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE) {
                result->r = result->g = result->b = result->a = 0.0;
		return miTRUE;
	}

	opacity = mi_eval_color(&paras->opacity);

	if (opacity->r == 1.0 && opacity->g == 1.0 &&
	    opacity->b == 1.0 && opacity->a == 1.0)
		*result = *mi_eval_color(&paras->input);
	else {
		mi_trace_transparent(result, state);

		if (opacity->r != 0.0 || opacity->g != 0.0 ||
		    opacity->b != 0.0 || opacity->a != 0.0) {
			inp = *mi_eval_color(&paras->input);
			result->r = result->r * (1.0 - opacity->r) +
							inp.r * opacity->r;
			result->g = result->g * (1.0 - opacity->g) +
							inp.g * opacity->g;
			result->b = result->b * (1.0 - opacity->b) +
							inp.b * opacity->b;
			result->a = result->a * (1.0 - opacity->a) +
							inp.a * opacity->a;
		}
	}

	return miTRUE;
}


/*-------------------------------------------------------------------------*/

struct mc {
	miColor		input;
	miColor		transp;
};

extern "C" DLLEXPORT int mib_continue_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_continue(
	miColor		*result,
	miState		*state,
	struct mc	*paras)
{
	miColor		*inp;
	miColor		*transp;

	/* check for illegal calls */
	if (state->type == miRAY_DISPLACE) {
		result->r = result->g = result->b = result->a = 0.0;
		return miTRUE;
	}

	transp = mi_eval_color(&paras->transp);

	if (state->type == miRAY_SHADOW) {
		/* special case for shadow rays: modulate the result
		   color and return miTRUE if there is still some
		   transparency, so that intersection finding will
		   continue until full occlusion is reached. */

		if (state->options->shadow == 's') {
			/* a surface using the mib_continue shader is
			   not meant to contain a volume, so we just
			   set the same volume on both sides of the
			   surface. */
			state->refraction_volume = state->volume;
			mi_continue_shadow_seg(result, state);
		}

		if (transp->r != 1.0 || transp->g != 1.0 ||
		    transp->b != 1.0 || transp->a != 1.0) {
			inp = mi_eval_color(&paras->input);
			result->r = result->r * transp->r +
				    inp->r * (1.0 - transp->r);
			result->g = result->g * transp->g +
				    inp->g * (1.0 - transp->g);
			result->b = result->b * transp->b +
				    inp->b * (1.0 - transp->b);
			result->a = result->a * transp->a +
				    inp->a * (1.0 - transp->a);
		}

		return result->r || result->g || result->b;
	}

	if (transp->r == 0.0 && transp->g == 0.0 &&
	    transp->b == 0.0 && transp->a == 0.0)
		*result = *mi_eval_color(&paras->input);
	else {
		mi_trace_continue(result, state);

		if (transp->r != 1.0 || transp->g != 1.0 ||
		    transp->b != 1.0 || transp->a != 1.0) {
			inp = mi_eval_color(&paras->input);
			result->r = result->r * transp->r +
				    inp->r * (1.0 - transp->r);
			result->g = result->g * transp->g +
				    inp->g * (1.0 - transp->g);
			result->b = result->b * transp->b +
				    inp->b * (1.0 - transp->b);
			result->a = result->a * transp->a +
				    inp->a * (1.0 - transp->a);
		}
	}

	return miTRUE;
}
