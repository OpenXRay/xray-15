/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_ray_marcher
 *
 * History:
 *	23.11.00: mib_texture_vector used its vertex param incorrectly
 *
 * Description:
 *	This is the ray marching code
 *****************************************************************************/

#include <math.h>
#include <stddef.h>
#include <shader.h>


struct mrm {
	miTag		s;
	miScalar	distance;
	miInteger	num;
	miInteger	subdiv;
	miColor		contrast;
};


/*
 * return true if contrast found
 */

static miBoolean component_contrast(
	miScalar	c1,
	miScalar	c2,
	miScalar	contrast)
{
	miScalar	denom = c1 + c2;

	if (denom > 0.0)
		return(fabs(c1 - c2) / denom >= contrast);
	else
		return(miFALSE);
}


/*
 * return true if contrast found
 */

static miBoolean color_contrast(
	miColor		*c1,
	miColor		*c2,
	miColor		*contrast)
{
	miBoolean	has_contrast = miFALSE;

	has_contrast  = component_contrast(c1->r, c2->r, contrast->r);
	has_contrast |= component_contrast(c1->g, c2->g, contrast->g);
	has_contrast |= component_contrast(c1->b, c2->b, contrast->b);
	has_contrast |= component_contrast(c1->a, c2->a, contrast->a);
	return(has_contrast);
}


/*
 * the recursive evaluation in high contrast regions
 */

static void recurse(
	miColor		*result,
	miState		*state,
	miVector	from,
	miVector	dir,
	miScalar	dist,
	miColor		*prev,
	miColor		*next,
	miInteger	level,
	struct mrm	*p)
{
	miVector	step, pos;
	miColor		col;

	if (level > p->subdiv)
		return;

	step = dir;
	mi_vector_mul(&step, dist*0.5F);

	pos = from;
	mi_vector_add(&pos, &pos, &step);

	state->point  = pos;
	state->normal = dir;
	state->pri    = 0;	/* required for ray casting from empty space */

	mi_call_shader_x(&col, miSHADER_MATERIAL, state, p->s, NULL);

	/* recurse further if necessary */
	if (color_contrast(prev, &col, &p->contrast))
		recurse(result, state, from, dir, dist*0.5F, prev, &col,
								level+1, p);
	if (color_contrast(next, &col, &p->contrast))
		recurse(result, state, pos, dir, dist*0.5F, &col, next,
								level+1, p);
	/* accumulate result */
	result->r += 0.5F*(col.r - 0.5F*(prev->r - next->r)) * dist;
	result->g += 0.5F*(col.g - 0.5F*(prev->g - next->g)) * dist;
	result->b += 0.5F*(col.b - 0.5F*(prev->b - next->b)) * dist;
	result->a += 0.5F*(col.a - 0.5F*(prev->a - next->a)) * dist;
}


/*
 * the basic raymarcher
 */

static void raymarch(
	miColor		*result,
	miState		*state,
	struct mrm	*p)
{
	miVector	step, pos, ppos;
	miVector	old_point, old_normal;
	miScalar	step_size;
	miColor		col, prev;
	miBoolean	has_prev = miFALSE;
	int		i;

	step       = state->dir;
	step_size  = state->dist / ((miScalar)p->num - 1);
	mi_vector_mul(&step, step_size);

	old_point  = state->point;
	old_normal = state->normal;
	pos        = state->org;

	/* begin raymarch */
	for (i=0; i < p->num; i++) {
		state->point  = pos;
		state->normal = state->dir;
		state->pri    = 0;	/* cats rays from empty space */

		mi_call_shader_x(&col, miSHADER_MATERIAL, state, p->s, NULL);

		if (has_prev && p->subdiv &&
				color_contrast(&prev, &col, &p->contrast))
		/* adaptive sampling */
			recurse(result, state, ppos, state->dir, step_size,
							&prev, &col, 1, p);
		result->r += col.r;
		result->g += col.g;
		result->b += col.b;
		result->a += col.a;

		prev     = col;
		ppos     = pos;
		has_prev = miTRUE;
		mi_vector_add(&pos, &pos, &step);
	}
	state->point  = old_point;
	state->normal = old_normal;
}


/*
 * the ray marching entry point
 */

extern "C" DLLEXPORT int mib_ray_marcher_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_ray_marcher(
	miColor		*result,
	miState		*state,
	struct mrm	*p)
{
	struct mrm	pe;
	miScalar	scale;

	result->r = result->g = result->b = result->a = 0.0;

	/*
	 * copy all parameters to a static structure to
	 * avoid all the mi_eval_* calls in the code
	 */
	pe.s        = *mi_eval_tag(&p->s);
	pe.distance = *mi_eval_scalar(&p->distance);
	pe.num      = *mi_eval_integer(&p->num);
	pe.subdiv   = *mi_eval_integer(&p->subdiv);
	pe.contrast = *mi_eval_color(&p->contrast);

	if (pe.num == 0.0)
		if (pe.distance > 0.0)
			pe.num = state->dist / pe.distance;
		else
			pe.num = 4; /* default #samples */
	if (pe.num < 2)
		pe.num = 2;

	/* go! */
	raymarch(result, state, &pe);

	/* normalize result */
	scale = 1.0F / (miScalar)pe.num;
	result->r *= scale;
	result->g *= scale;
	result->b *= scale;
	result->a *= scale;
	return(miTRUE);
}
