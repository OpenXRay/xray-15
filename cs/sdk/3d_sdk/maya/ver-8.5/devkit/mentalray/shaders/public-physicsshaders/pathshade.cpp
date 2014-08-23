/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	23.4.96
 * Module:	physics
 * Purpose:	Global illumination using path tracing
 *
 * Exports:
 *	path_material_version
 *	path_material
 *	oversampling_lens_version
 *	oversampling_lens
 *
 * History:
 *	14.01.98: removed install function, this is now a library
 *	13.10.98: Use mi_eval() et al.
 *	06.07.00: major cleanup
 *
 * Description:
 * A prototype of a global illumination program using lens shaders and material
 * shaders to do path tracing.
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <dgsshade.h>   /* for dgs_material */
#include <pathshade.h>
#include <mi_shader_if.h>

#define miEPS 0.0001

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

#define mi_MIN(a,b)	((a) < (b) ? (a) : (b))
#define mi_MAX(a,b)	((a) > (b) ? (a) : (b))


/*
 * calculate the index of refraction of the incoming (ior_in) and the
 * outgoing (ior_out) ray, relative to the current intersection. This
 * is derived from mi_mtl_refraction_index in softshade.c and is identical
 * to the function in dgsshade.c
 */

static void refraction_index(
	miState			*state,
	struct dgs_material	*paras,
	miScalar		*ior_in,
	miScalar		*ior_out)
{
	int			num = 0;
	miState			*s, *s_in = NULL;   /* finding enclosing mtl */

	for (s=state; s; s=s->parent)				/* history? */
		if ((s->type == miRAY_TRANSPARENT	   ||
		     s->type == miRAY_REFRACT		   ||
		     s->type == miPHOTON_TRANSMIT_SPECULAR ||
		     s->type == miPHOTON_TRANSMIT_GLOSSY   ||
		     s->type == miPHOTON_TRANSMIT_DIFFUSE) &&
		    s->parent && s->parent->shader == state->shader) {
			num++;
			if (!s_in)
			s_in = s->parent;
		}
	if (!(num & 1)) {					/* entering */
		*ior_out = paras->ior;
		*ior_in  = state->parent && state->parent->ior != 0.0 ?
						state->parent->ior : 1.0;
		if (!state->refraction_volume)
			state->refraction_volume = state->volume;
	} else {						/* exiting */
		*ior_in  = paras->ior;
		*ior_out = (s_in && s_in->ior_in != 0.0) ? s_in->ior_in : 1.0;
		state->refraction_volume = s_in ? s_in->volume
						: state->camera->volume;
	}
	state->ior_in = *ior_in;
	state->ior    = *ior_out;
}


/*
 * Set u to follow surface derivative, v to be perpendicular.
 * This does not have the fancy direction-from-texture capabilities
 * of anis_orientation in dgsshade.c.
 */

static void anis_orientation(
	miVector	*u,             /* direction of brushing */
	miVector	*v,             /* perpendicular direction */
	miState		*state)
{
	float		d;

	/*
	 * Set u to be the normalized projection of the first 1st derivative
	 * onto the tangent plane
	 */
	*u = state->derivs[0];
	d  = mi_vector_dot(u, &state->normal);
	u->x -= d * state->normal.x;
	u->y -= d * state->normal.y;
	u->z -= d * state->normal.z;
	miASSERT(mi_vector_dot(u, &state->normal) < miEPS);
	mi_vector_normalize(u);

	/*
	 * Set v to be perpendicular to u (in the tangent plane)
	 */
	mi_vector_prod(v, &state->normal, u);

	miASSERT(fabs(mi_vector_norm(u) - 1.0) < miEPS);
	miASSERT(fabs(mi_vector_norm(v) - 1.0) < miEPS);
	miASSERT(fabs(mi_vector_dot(u, v)) < miEPS);	/* u,v perpendicular */
}


/*
 * Calculate illumination of a material by its lights. m and paras are both
 * passed because m contains all the colors updated by textures, and paras
 * has the arrays (m is a copy of the fixed struct without appended arrays).
 */

static void direct_illumination(
	miColor			*result,	/* returned illum color */
	miState			*state,		/* ray tracer state */
	struct dgs_material	*m,		/* textured material paras */
	miTag			*light)
{
	miColor			color;		/* color from light source */
	miColor			sum;		/* summed sample colors */
	miVector		dir;		/* direction towards light */
	miScalar		dot_nl;		/* dot prod of normal and dir*/
	miScalar		f;		/* for mi_ward_glossy */
	miScalar		refl = 1.0 - m->transp;
	miBoolean		diff, glos;

	diff = m->diffuse.r > 0.0 || m->diffuse.g > 0.0 || m->diffuse.b > 0.0;
	glos = m->glossy.r  > 0.0 || m->glossy.g  > 0.0 || m->glossy.b  > 0.0;

	for (mi::shader::LightIterator iter(state, light, m->n_light); 
						!iter.at_end(); ++iter) {
		sum.r = sum.g = sum.b = 0;
		/*
		 * Sample the light source.  (samples_u and samples_v should
		 * be set to 1 in the mi file for efficiency)
		 */
		while (iter->sample()) {
			/*
			 * Diffuse (Lambert) reflection: m->diffuse is reflect-
			 * ance rho (0 <= rho <= 1), so we have to divide by pi
			 * here to get the value of the BRDF. We also have to
			 * multiply by the cosine at the receiver. The cosine
			 * at the light source, the area of * the light source,
			 * and the square of the * distance are taken care of
			 * by the physical light source shader.
			 */
			iter->get_contribution(&color);
			dot_nl = iter->get_dot_nl();
			if (diff) {
				f = dot_nl * refl;
				sum.r += f * m->diffuse.r * (1/M_PI) * color.r;
				sum.g += f * m->diffuse.g * (1/M_PI) * color.g;
				sum.b += f * m->diffuse.b * (1/M_PI) * color.b;
			}

			/*
			 * Glossy reflection
			 */
			if (glos) {
				dir = iter->get_direction();
				if (m->shiny > 0.0) {	/* isotropic refl. */
					f = dot_nl * mi_ward_glossy(
						&state->dir, &dir,
						&state->normal, m->shiny);
				} else {		/* anisotropic refl. */
					miVector u, v;
					miASSERT(m->shiny_u > 0.0 &&
						 m->shiny_v > 0.0);
					anis_orientation(&u, &v, state);
					f = dot_nl * mi_ward_anisglossy(
							&state->dir, &dir,
							&state->normal, &u, &v,
							m->shiny_u,m->shiny_v);
				}
				sum.r += f * refl * m->glossy.r * color.r;
				sum.g += f * refl * m->glossy.g * color.g;
				sum.b += f * refl * m->glossy.b * color.b;
			}

			/*
			 * Specular (mirror) reflection is taken care of in
			 * indirect_illumination() since the specular ray will
			 * hit the light source if there is one in the mirror
			 * direction
			 */
		}
		int samples = iter->get_number_of_samples();
		if (samples) {
			result->r += sum.r / samples;
			result->g += sum.g / samples;
			result->b += sum.b / samples;
		}
	}
	result->a = 1;
}


/*
 * Compute contribution from reflection ray
 */

static void indirect_illumination(
	miColor			*result,	/* returned illum color */
	miState		 	*state,		/* ray tracer state */
	struct dgs_material	*m)		/* textured material paras */
{
	miColor		 	color;		/* color from reflected ray */
	miVector		dir;		/* new ray direction */
	float			ior_in,ior_out;	/* index of refraction */

	state->type = mi_choose_scatter_type(state, m->transp,
					&m->diffuse, &m->glossy, &m->specular);
	switch (state->type) {
	  case miPHOTON_ABSORB:		    /* no reflection or transmission */
		return;

	  case miPHOTON_REFLECT_SPECULAR:   /* specular reflection (mirror) */
		mi_reflection_dir(&dir, state);
		if (mi_trace_reflection(&color, state, &dir)) {
			/* Multiply by spec. refl. coeffs and add to result */
			result->r += m->specular.r * color.r;
			result->g += m->specular.g * color.g;
			result->b += m->specular.b * color.b;
		}
		break;

	  case miPHOTON_REFLECT_GLOSSY:	    /* glossy reflection (Ward model)*/
		/*
		 * Compute a new direction around the mirror direction
		 * using Ward's BRDF model for importance sampling
		 */
		if (m->shiny)		    /* isotropic glossy reflection */
			mi_reflection_dir_glossy(&dir, state, m->shiny);
		else {			    /* anisotropic glossy reflection */
			miVector u, v;
			miASSERT(m->shiny_u > 0.0 && m->shiny_v > 0.0);
			anis_orientation(&u, &v, state);
			mi_reflection_dir_anisglossy(&dir, state, &u, &v,
						     m->shiny_u, m->shiny_v);
		}

		/*
		 * Trace new ray if direction is above horizon. Add the
		 * contribution unless the ray hit a visible area light source.
		 * state->child->pri is 0 for intersections with area lights.
		 */
		if (mi_vector_dot(&dir, &state->normal) > 0  &&
		    mi_trace_reflection(&color, state, &dir) &&
							state->child->pri) {
			result->r += m->glossy.r * color.r;
			result->g += m->glossy.g * color.g;
			result->b += m->glossy.b * color.b;
		}
		break;

	  case miPHOTON_REFLECT_DIFFUSE:    /* diffuse refl. (Lambert) */
		/*
		 * Compute a new direction using cos for importance sampling
		 */
		mi_reflection_dir_diffuse(&dir, state);

		/*
		 * Trace reflection in this direction (in rctrace.c) and add
		 * contribution unless the ray hit a visible area light source.
		 * Multiply by diffuse reflection and add to result. m->diffuse
		 * is reflectance rho (0 <= rho <= 1), so we have to divide by
		 * pi here to get the value of the BRDF. But we also have to
		 * multiply by the (projected) area of integration which is Pi,
		 * so the Pi's cancel.
		 */
		if (mi_trace_reflection(&color, state, &dir) &&
							state->child->pri) {
			result->r += m->diffuse.r * color.r;
			result->g += m->diffuse.g * color.g;
			result->b += m->diffuse.b * color.b;
		}
		break;

	  case miPHOTON_TRANSMIT_SPECULAR:  /* specular transmission */
		refraction_index(state, m, &ior_in, &ior_out);
		miASSERT(ior_in >= 1.0 && ior_out >= 1.0);
		if (mi_transmission_dir_specular(&dir, state, ior_in,ior_out)&&
		    mi_trace_refraction(&color, state, &dir)) {
			result->r += m->transp * m->specular.r * color.r;
			result->g += m->transp * m->specular.g * color.g;
			result->b += m->transp * m->specular.b * color.b;
		}
		break;

	  case miPHOTON_TRANSMIT_GLOSSY:    /* glossy transmission (Ward) */
		refraction_index(state, m, &ior_in, &ior_out);
		miASSERT(ior_in >= 1.0 && ior_out >= 1.0);

		/*
		 * Compute a new direction around the refraction direction
		 * using Ward's BRDF model for importance sampling
		 */
		if (m->shiny)		    /* isotropic glossy reflection */
			mi_transmission_dir_glossy(&dir, state, ior_in,
							ior_out, m->shiny);
		else {			    /* anisotropic glossy refraction */
			miVector u, v;
			miASSERT(m->shiny_u > 0.0 && m->shiny_v > 0.0);
			anis_orientation(&u, &v, state);
			mi_transmission_dir_anisglossy(&dir, state, ior_in,
				ior_out, &u, &v, m->shiny_u, m->shiny_v);
		}

		/*
		 * Only trace new ray if direction is below horizon
		 */
		if (mi_vector_dot(&dir, &state->normal) < 0.0 &&
		    mi_trace_refraction(&color, state, &dir)) {
			result->r += m->glossy.r * color.r;
			result->g += m->glossy.g * color.g;
			result->b += m->glossy.b * color.b;
		}
		break;

	  case miPHOTON_TRANSMIT_DIFFUSE:   /* diffuse transm. (translucency)*/
		/*
		 * Compute a new direction using cos for importance sampling
		 */
		mi_transmission_dir_diffuse(&dir, state);
		if (mi_trace_refraction(&color, state, &dir)) {
			result->r += m->diffuse.r * color.r;
			result->g += m->diffuse.g * color.g;
			result->b += m->diffuse.b * color.b;
		}
		break;

	  default:
		mi_error("Unknown scattering type for path-tracing shader");
	}
}


/*
 * Shaders for path tracing
 */

int path_material_version(void) {return(1);}

miBoolean path_material(
	miColor			*result,
	miState			*state,
	struct dgs_material	*paras)
{
	struct dgs_material	m;	/* work area for material */
	miColor			local, global;
	static miColor		black = {0.0, 0.0, 0.0, 0.0};
	miTag* 			light;

	if (state->type == miRAY_SHADOW)
		return(miFALSE);	/* black shadow for global illum. */

	m.diffuse  = *mi_eval_color  (&paras->diffuse);
	m.glossy   = *mi_eval_color  (&paras->glossy);
	m.specular = *mi_eval_color  (&paras->specular);
	m.shiny    = *mi_eval_scalar (&paras->shiny);
	m.shiny_u  = *mi_eval_scalar (&paras->shiny_u);
	m.shiny_v  = *mi_eval_scalar (&paras->shiny_v);
	m.transp   = *mi_eval_scalar (&paras->transp);
	m.i_light  = *mi_eval_integer(&paras->i_light);
	m.n_light  = *mi_eval_integer(&paras->n_light);
        light = mi_eval_tag(&paras->light) + m.i_light;

	local = global = black;
	direct_illumination  (&local,  state, &m, light);
	indirect_illumination(&global, state, &m);

	result->r = local.r + global.r;
	result->g = local.g + global.g;
	result->b = local.b + global.b;
	result->a = 1.0;
	return(miTRUE);
}


/*
 * Lens shader for oversampling
 */

int oversampling_lens_version(void) {return(1);}

miBoolean oversampling_lens(
	miColor		*result,
	miState		*state,
	int		*paras)		/* number of samples */
{
	miColor		sum = {0.0, 0.0, 0.0, 0.0};
	int		i;

	/*
	 * Trace a number of rays, all in the same direction. Add the results.
	 */
	for (i=0; i < *paras; i++) {
		mi_trace_eye(result, state, &state->org, &state->dir);
		sum.r += mi_MIN(result->r, 1.0);
		sum.g += mi_MIN(result->g, 1.0);
		sum.b += mi_MIN(result->b, 1.0);
		sum.a += mi_MIN(result->a, 1.0);
	}

	/* The result is the average color */
	result->r = sum.r / *paras;
	result->g = sum.g / *paras;
	result->b = sum.b / *paras;
	result->a = sum.a / *paras;
	return(miTRUE);
}
