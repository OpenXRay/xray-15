/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * created:    2003-11-14
 * module:     base  
 * purpose:    lens shaders 
 * requirements:  mental ray 3.0 
 *
 * Exports:
 *   mib_lens_stencil			implement a stencil for rendering
 *   mib_lens_stencil_version
 *   mib_lens_clamp			map color compents within given
 *					interval to [0, 1]
 *   mib_lens_clamp_version
 *
 * Description:
 * 
 *
 *****************************************************************************/

#include "shader.h"

/*
 * This lens shader implements a simple stencil for rendering. 
 * It uses a scalar texture to determine if an eye ray should be cast.
 * Only if the return value of the scalar texture lookup is between
 * the prescribed floor (default: 0) and ceiling (default: 1) values will
 * an eye ray be cast. if the scalar value is below floor, then the
 * floor color will be returned, if the value is above ceiling, then the
 * ceiling color will be returned.
 * If the returned texture value s is between floor and ceiling the
 * result of the cast eye ray will be blended with the floor color,
 * weighted by the relative position of s between floor and ceiling.
 */
typedef struct mibStencil {
	miScalar	floor;		/* only call mi_trace_eye if texture */
	miScalar	ceiling;        /* return is between floor & ceiling */
	miColor		floor_color;    /* color if text. return < floor     */
	miColor		ceil_color;	/* color if text. return > ceiling   */	
	miTag		stencil; 	/* tag for scalar texture            */
} mibStencil_t;

extern "C" DLLEXPORT int mib_lens_stencil_version(void) { return 1; }

extern "C" DLLEXPORT miBoolean mib_lens_stencil(
	miColor*	result,
	miState*	state,
	mibStencil_t*	paras)
{
	miScalar 	f  = *mi_eval_scalar(&paras->floor);
	miScalar 	c  = *mi_eval_scalar(&paras->ceiling);
	miColor  	fc = *mi_eval_color(&paras->floor_color);
	miColor  	cc = *mi_eval_color(&paras->ceil_color);
	miTag    	st = *mi_eval_tag(&paras->stencil);

	miScalar 	s;
	miVector 	coord;

	/* handle default parameter values */
	if (c == 0.0f) {
		c = 1.0f;
	}

	if (!st || f >= c) {
		return mi_trace_eye(result, state, &state->org, &state->dir);
	} 

	coord.x = state->raster_x / state->camera->x_resolution;
	coord.y = state->raster_y / state->camera->y_resolution;
	coord.z = 0.0f;

	mi_lookup_scalar_texture(&s, state, st, &coord);
	s = (s-f)/(c-f);
	if (s < 0.0f) {
		*result = fc;
	} else if ( s > 1.0f) {
		*result = cc;
	} else {
		miColor col = { 0.0, 0.0, 0.0, 1.0};
        	miScalar sn = 1.0f - s;

		(void) mi_trace_eye(&col, state, &state->org, &state->dir);

		result->r = s * col.r + sn * fc.r;
		result->g = s * col.g + sn * fc.g;
		result->b = s * col.b + sn * fc.b;
		result->a =     col.a;
	}
	return miTRUE;
}


/*
 * This lens shader maps color components lying in the interval
 * [floor, ceiling] to the interval [0, 1]. Values below
 * and above those limits are clamped to 0 and 1, respectively.
 * This shader may be used to as a "poor man's tonemapping".
 * Default values are floor = 0 and ceiling = 1.
 */
typedef struct mibClamp {
	miScalar	floor;		/* color interval low bound */
	miScalar	ceiling;	/* color interval high bound */
	miBoolean	luminance;	/* clamp color or luminance */
	miColor		floor_color;	/* return color if lum is below floor*/
	miColor		ceil_color;	/* return color if lum is above ceil*/
} mibClamp_t;

extern "C" DLLEXPORT int mib_lens_clamp_version(void) { return 1; }

extern "C" DLLEXPORT miBoolean mib_lens_clamp(
	miColor*	result,
	miState*	state,
	mibClamp_t*	paras)
{
	miBoolean 	res;
	miScalar 	f = *mi_eval_scalar(&paras->floor);
	miScalar 	c = *mi_eval_scalar(&paras->ceiling);
	miBoolean	b = *mi_eval_boolean(&paras->luminance);

	if (c == 0.0f) {
		c = 1.0f;
	}
	if (f == c) {
		f = 0.0f;
	}

	res = mi_trace_eye(result, state, &state->org, &state->dir);

	if(b) {
		/* clamp based on luminance. */
		miScalar	lum = mi_luminance(state, result);
	
		if(lum < f) {
			*result = *mi_eval_color(&paras->floor_color);
		} else if (lum > c) {
			*result = *mi_eval_color(&paras->ceil_color);
		} else {
			lum = (lum - f)/(c - f);
			result->r *= lum;
			result->g *= lum;
			result->b *= lum;
		}
	} else {	
		/* clamp based on color components */
		result->r = result->r > f ? 
			(result->r < c ? (result->r-f)/(c-f) : 1) : 0;
		result->g = result->g > f ? 
			(result->g < c ? (result->g-f)/(c-f) : 1) : 0;
		result->b = result->b > f ? 
			(result->b < c ? (result->b-f)/(c-f) : 1) : 0;
	}

	return res;
}

