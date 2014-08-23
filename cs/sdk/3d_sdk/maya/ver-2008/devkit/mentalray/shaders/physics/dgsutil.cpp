/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	26.01.2001
 * Module:	physics
 * Purpose:	Util functions for dgs_material and dgs_materila_photon
 *
 * Exports:
 *	dgs_refraction_index		compute refraction index via iors
 *	dgs_anis_orientaion		orientation dor anisotropic glossy
 *	dgs_reflect_glossy_dir		compute glossy refl. ray direction
 *	dgs_transmit_glossy_dir		compute glossy transm. ray direction
 *	dgs_set_parameters		eval common material/photon mat. param
 *
 * History:
 *	26.01.01: Split fron dgsshade.c
 *
 * Description:
 *      Utility function for dgs_material / dgs_material_photon shader.
 *****************************************************************************/

#include <assert.h>
#include <math.h>
#include <shader.h>
#include <dgsshade.h>
#include <dgsutil.h>
#include <mi_version.h>


/*
 * Calculate the index of refraction of the incoming (state->ior_in) and the
 * outgoing (state->ior) ray, relative to the current intersection. This is
 * simple and not accurate procedure, for example, children of backside
 * final gather rays are not treated correctly.
 */

void dgs_refraction_index(
	miState			*state,
	struct dgs_material	*m)
{
	int			num = 0;
	miState			*s, *s_in=NULL;	/* for finding enclosing mtl */

	for (s=state; s; s=s->parent)				/* history? */
		if ((s->type == miRAY_TRANSPARENT	   ||
		     s->type == miRAY_REFRACT		   ||
		     s->type == miPHOTON_TRANSMIT_SPECULAR ||
		     s->type == miPHOTON_TRANSMIT_GLOSSY   ||
		     s->type == miPHOTON_TRANSPARENT	   ||
		     s->type == miPHOTON_TRANSMIT_DIFFUSE) &&
		     s->parent && s->parent->shader == state->shader) {
			num++;
			if (!s_in) s_in = s->parent;
		}

	if (!(num & 1)) {					/* entering */
		state->ior = m->ior;
		state->ior_in = state->parent && state->parent->ior != 0
		        ? state->parent->ior : 1;
		if (!state->refraction_volume)
			state->refraction_volume = state->volume;
	} else {						/* exiting */
		state->ior_in = m->ior;
		state->ior = (s_in && s_in->ior_in != 0) ? s_in->ior_in : 1;
		state->refraction_volume = s_in ? s_in->volume
						: state->camera->volume;
	}
}


/*
 * Orientation for unisotropic glossy reflection / transmission.
 * First derivation should be defined for this to work correct.
 */

void dgs_anis_orientation(
	miVector	*u,		/* direction of brushing */
	miVector	*v,		/* perpendicular direction */
	miState		*state)		/* ray tracer state */
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

	/* Set v to be perpendicular to u (in the tangent plane) */
	mi_vector_prod(v, &state->normal, u);
	miASSERT(fabs(mi_vector_norm(u) - 1) < miEPS);
	miASSERT(fabs(mi_vector_norm(v) - 1) < miEPS);
	miASSERT(fabs(mi_vector_dot(u, v)) < miEPS);
}


/*
 * Compute a random direction around the mirror direction
 *  using Ward's BRDF model for importance sampling
 */

void dgs_reflect_glossy_dir(
        miState 	*state,
	miVector 	*dir,
	struct dgs_material *m)
{
        if (m->shiny)			/* isotropic glossy reflection */
	        mi_reflection_dir_glossy(dir, state, m->shiny);
	else {				/* anisotropic glossy reflection */
	        miVector u, v;
		dgs_anis_orientation(&u, &v, state);
		mi_reflection_dir_anisglossy(dir, state, &u, &v,
					     m->shiny_u, m->shiny_v);
	}
}


/*
 * Compute a random direction around transmission direction
 * using Ward's BRDF model for importance sampling.
 */

miBoolean dgs_transmit_glossy_dir(
	miState 	*state,
	miVector 	*dir,
	struct dgs_material *m)
{
        if (m->shiny)			/* isotropic glossy refraction */
	        return(mi_transmission_dir_glossy(dir, state,
					state->ior_in, state->ior, m->shiny));

	else {				/* anisotropic glossy refraction */
	        miVector u, v;
		dgs_anis_orientation(&u, &v, state);
		return(mi_transmission_dir_anisglossy(dir, state,
					state->ior_in, state->ior,
					&u, &v, m->shiny_u, m->shiny_v));
	}
}


/*
 * Computes Ward isotropic / anisotropic glossy distribution value,
 * for gloosy reflections and transmissions. This function may be
 * used with for backside with both unchanged and flipped state->normal.
 */

miScalar dgs_ward_glossy(
	miVector	*in,	/* incidence direction */
	miVector	*out,   /* reflection direction */
	miState		*state,
	struct dgs_material *m)
{
	if (m->shiny > 0)
	        return(mi_ward_glossy(in, out, &state->normal, m->shiny));
	else {
	        miVector u, v;
		dgs_anis_orientation(&u, &v, state);
	        return(mi_ward_anisglossy(in, out, &state->normal,
					  &u, &v, m->shiny_u, m->shiny_v));
	}
}


/*
 * Create local copy of parameters common for dgs_material and
 * dgs_material_photon (the later does not need lights).
 */

void dgs_set_parameters(
        miState			*state,		/* state to use */
	struct dgs_material	*m,		/* fill this local copy */
        struct dgs_material	*paras)		/* mi_eval this */
{
        m->diffuse  = *mi_eval_color  (&paras->diffuse);
	m->glossy   = *mi_eval_color  (&paras->glossy);
	m->specular = *mi_eval_color  (&paras->specular);
	m->shiny    = *mi_eval_scalar (&paras->shiny);
	m->shiny_u  = *mi_eval_scalar (&paras->shiny_u);
	m->shiny_v  = *mi_eval_scalar (&paras->shiny_v);
	m->transp   = *mi_eval_scalar (&paras->transp);
	m->ior	    = *mi_eval_scalar (&paras->ior);
	m->mode	    = *mi_eval_integer(&paras->mode);

	miASSERT(0 <= m->transp && m->transp <= 1);
	/* if glossy>0, either isotropic or anisotropic shiny should be set */
	miASSERT(m->glossy.r == 0 && m->glossy.g == 0 && m->glossy.b == 0 ||
		 m->shiny || m->shiny_u && m->shiny_v);
}
