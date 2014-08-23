/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	4.11.96
 * Module:	physics
 * Purpose:	Physically based shading of dielectric materials
 *
 * Exports:
 *	dielectric_material			material shader
 *	dielectric_material_version		material shader version
 *	dielectric_material_photon		photon shader
 *	dielectric_material_photon_version	photon shader version
 *
 * History:
 *	14.01.98: removed install function, this is now a library
 *	16.01.98: use mi_choose_scatter_type() (a must now)
 *	21.01.98: re-added the *_photon_init() functions
 *	26.05.98: changed computation of alpha.	Added assertions.
 *	02.10.98: use mi_eval() & co.
 *	15.02.99: fixed a bug: photon trace level was compared to
 *		  max ray trace depth in the photon shader.
 *	06.07.00: major cleanup
 *	25.03.04: fixed multithreaded bug, fixed non-const arguments bug for
 *		  using inside phenomena.
 *	19.01.05: fixed refraction in rcrm
 *      
 *
 * Description:
 * - This file handles shading of dielectric materials such as glass, liquids
 *   etc. It uses Fresnel's formulas for computing the specular reflection and
 *   transmission which depends on the indices of refraction and the angle of
 *   the incoming ray.
 * - The colour of the material is computed using Beer's law which is based on
 *   absorption coefficients colours the light based on the distance traversed
 *   through the material.
 * - If called as a shadow shader, it returns miFALSE (i.e. completely opaque).
 *   This is because we cannot compute shadows along bent rays. Instead, this
 *   type of illumination is handled by the photon map.
 *****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <mi_shader_if.h>
#include "dielecshade.h"

#define miEPS 0.0001

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

/* Absorbion coeff. from a color clipped to (0, 1] */
inline void absorb(miColor* c, const miColor* a, miScalar dist)
{
   c->r *= (a->r <= 0.0f ? 0.0f : a->r >= 1.0f ? 1.0f : pow(a->r, dist));
   c->g *= (a->g <= 0.0f ? 0.0f : a->g >= 1.0f ? 1.0f : pow(a->g, dist));
   c->b *= (a->b <= 0.0f ? 0.0f : a->b >= 1.0f ? 1.0f : pow(a->b, dist));
}

struct material_coef {
	miScalar	ior;		/* index of refraction */
	miScalar	ior_out;	/* outside ior for interface */
	miScalar	pcoef;		/* phong coef. for nice highlights */
	miCBoolean	ignore_normals;	/* ignore normals? */
	miCBoolean	is_interface;	/* simulate interface */
	int		russian_level;
};

static miColor black = {0, 0, 0, 0};


static void coef_init(
	miState			   *state,	/* render state */
	struct dielectric_material *paras,	/* parameters */
	struct material_coef	   *mc)		/* evvaluated params. */		
{
	miScalar		ior, ior_out;

	ior	= *mi_eval_scalar(&paras->ior);
	ior_out	= *mi_eval_scalar(&paras->ior_out);

	if (ior <= 0) {
		mi_warning("dielectric material cannot use zero or negative "
			   "index of refraction %f (using 1.0)", ior);
		mc->ior = 1;
	} else
		mc->ior = ior;

	if (ior_out < 0) {
		mi_warning("cannot use negative index of refraction %f for "
			   "dielectric material (ignoring)", ior_out);
		mc->is_interface = 0;
		mc->ior_out	 = 1;

	} else if (ior_out == 0) {
		mc->is_interface = 0;
		mc->ior_out	 = 1;
	} else {
		mc->is_interface = 1;
		mc->ior_out	 = ior_out;
	}
	
	mc->pcoef	   = *mi_eval_scalar (&paras->pcoef);
	mc->ignore_normals = *mi_eval_boolean(&paras->ignore_normals);

	if (state->options->max_samples >= 1) {
		mc->russian_level = 5 - state->options->max_samples;
		if (mc->russian_level <= 1)
			mc->russian_level = 1;
	} else
		mc->russian_level = 4;
}


static miCBoolean locate_volume(
	miState			*state,
	struct material_coef	*mc)
{
	miState			*s, *s_in = NULL;
	miCBoolean		enter_surf;
	int			num = 0;

	enter_surf = !state->inv_normal;
	for (s=state; s; s=s->parent)			/* history? */
		if ((s->type == miRAY_TRANSPARENT	   ||
		     s->type == miRAY_REFRACT		   ||
		     s->type == miPHOTON_TRANSMIT_SPECULAR ||
		     s->type == miPHOTON_TRANSMIT_GLOSSY   ||
		     s->type == miPHOTON_TRANSPARENT	   ||
		     s->type == miPHOTON_TRANSMIT_DIFFUSE) &&
		     s->parent && s->parent->shader == state->shader) {
			num++;
			if (!s_in)
				s_in = s->parent;
		}
	if (mc->ignore_normals)
		enter_surf = !(num & 1);

	if (enter_surf) {			/* entering */
		if (!state->refraction_volume)
			state->refraction_volume = state->volume;
		if (mc->is_interface)
			mc->ior_out = state->parent && state->parent->ior != 0 ?
							state->parent->ior : 1;
		state->ior_in = mc->ior_out;
		state->ior = mc->ior;
	} else {
		state->refraction_volume = s_in ? s_in->volume
						: state->camera->volume;
		if (mc->is_interface)
			mc->ior_out = s_in && s_in->ior_in != 0 ? 
				                        s_in->ior_in : 1;
		state->ior = mc->ior_out;
		state->ior_in = mc->ior;
	}

	return(enter_surf);
}


/*****************************************************************************
 *				photon shader				    **
 *****************************************************************************/

extern "C" DLLEXPORT int dielectric_material_photon_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean dielectric_material_photon(
	register miColor	   *energy,	/* propagated energy */
	register miState	   *state,	/* active state */
	struct dielectric_material *paras)	/* incoming parameters */
{
	float			specular;
	miVector		dir;
	miColor			color;
	float			dist;
	short			enter_surf;
	struct material_coef	mc;
	miBoolean		do_reflect=miTRUE, do_refract=miTRUE;
	miColor			rspec, tspec;
	miRay_type		rt;
	miColor			abs_col;

	if (!miRAY_PHOTON(state->type))
		return(dielectric_material(energy, state, paras));

	coef_init(state, paras, &mc);

	enter_surf = locate_volume(state, &mc);
	specular   = mi_fresnel_reflection(state, state->ior_in, state->ior);

	/*
	 * use Russian roulette to determine whether we reflect or transmit the
	 * photon
	 */
	rspec.r = rspec.g = rspec.b = specular;
	tspec.r = tspec.g = tspec.b = 1-specular;

	rt = mi_choose_simple_scatter_type(state, NULL, &rspec, NULL, &tspec);
	if (rt == miPHOTON_REFLECT_SPECULAR)
		do_refract = miFALSE;

	else if (rt == miPHOTON_TRANSMIT_SPECULAR)
		do_reflect = miFALSE;
	else
		do_reflect =
		do_refract = miFALSE;

	if (do_reflect || do_refract) {
		color = *energy;
		dist = state->dist;
		/* 
		 * unlike in material shader, this is attenuation 
		 * for interval "before" interaction.
		 */
		if (!enter_surf) {
			abs_col = *mi_eval_color(&paras->col);
			absorb(&color, &abs_col, dist);
		} else if (mc.is_interface) {
			abs_col = *mi_eval_color(&paras->col_out);
			absorb(&color, &abs_col, dist);
		}
	}

	if (do_reflect) {			/* trace reflected photon */
		color.r *= rspec.r;
		color.g *= rspec.g;
		color.b *= rspec.b;
		mi_reflection_dir(&dir,state);
		mi_photon_reflection_specular(&color, state, &dir);
	} else if (do_refract) {		/* trace refracted photon */
		color.r *= tspec.r;
		color.g *= tspec.g;
		color.b *= tspec.b;
		if (mi_refraction_dir(&dir, state, state->ior_in, state->ior))
			mi_photon_transmission_specular(&color, state, &dir);
	}
	return(miTRUE);
}


/*****************************************************************************
 *				material shader				    **
 *****************************************************************************/

extern "C" DLLEXPORT int dielectric_material_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean dielectric_material(
	miColor			   *res,	/* returned colour */
	miState			   *state,	/* render state */
	struct dielectric_material *paras)	/* incoming parameters */
{
	float			specular;	/* specular reflection */
	miVector		dir;
	miColor			color;
	float			dist, scale;
	short			enter_surf;
	miTag			*lgt;
	int			n_lights;
	int			i_lights;
	miBoolean		do_reflect = miTRUE, do_refract = miTRUE;
	float			rr_scale = 1;
	struct material_coef	mc;

	miColor			abs_col;	/* absorbtion */

	/* Material shader called as a photon shader? */
	if (miRAY_PHOTON(state->type))
		return(dielectric_material_photon(res, state, paras));

	*res = black;

	/* Material shader called as a shadow or displace shader? */
	if (state->type == miRAY_SHADOW
            || state->type == miRAY_LIGHT
            || state->type == miRAY_DISPLACE )
		return(miFALSE);	/* no transparent shadows here */

	coef_init(state, paras, &mc);

	enter_surf = locate_volume(state, &mc);
	specular   = mi_fresnel_reflection(state, state->ior_in, state->ior);

	/*
	 * add fake contribution from lightsources - L(S|D)
	 * We'll just do a normalized phong highlight
	 */
	n_lights = *mi_eval_integer(&paras->n_lights);
	i_lights = *mi_eval_integer(&paras->i_lights);
	lgt = mi_eval_tag(paras->lights) + i_lights;

	if (mc.pcoef > 0) {
		for (mi::shader::LightIterator iter(state, lgt, n_lights);
						!iter.at_end(); ++iter) {
			miColor		csum;
			miScalar	ns;

			csum.r = csum.g = csum.b = 0;
			while (iter->sample()) {
				dir = iter->get_direction();
				iter->get_contribution(&color);
				ns = specular * (mc.pcoef+1) * (1/(2*M_PI)) *
				     mi_phong_specular(mc.pcoef, state, &dir);
				csum.r += ns * color.r;
				csum.g += ns * color.g;
				csum.b += ns * color.b;
			}
			
			miInteger smp = iter->get_number_of_samples();
			if (smp) {
				res->r += csum.r / smp;
				res->g += csum.g / smp;
				res->b += csum.b / smp;
			}
		}
	}

	/*
	 * prune ray-tree using russian roulette
	 */
	if (state->reflection_level + state->refraction_level >=
							mc.russian_level) {
		miColor		rspec, tspec;
		miRay_type	rt;

		rspec.r = rspec.g = rspec.b = specular;
		tspec.r = tspec.g = tspec.b = 1-specular;
		rt = mi_choose_simple_scatter_type(state, NULL, &rspec,
							  NULL, &tspec);
		/*
		 * mi_choose_simple_scatter_type scales
		 * r,g,b by the same factor
		 */
		if (rt == miPHOTON_REFLECT_SPECULAR) {
			do_refract = miFALSE;
			rr_scale   = rspec.r / specular;

		} else if (rt == miPHOTON_TRANSMIT_SPECULAR) {
			do_reflect = miFALSE;
			rr_scale   = tspec.r / (1-specular);
		} else
			do_reflect = do_refract = miFALSE;
	}

	/*
	 * trace reflected ray
	 */
	res->a = specular;
	if (do_reflect && specular > miEPS) {
		mi_reflection_dir(&dir,state);
		if (mi_trace_reflection(&color, state, &dir) ||
		    mi_trace_environment(&color, state, &dir)) {
			dist = 0.0f;
			if (state->child)
				dist = state->child->dist;
			scale = specular*rr_scale;
			
			color.r *= scale;
			color.g *= scale;
			color.b *= scale;

			if (!enter_surf && dist) {
				abs_col = *mi_eval_color(&paras->col);
				absorb(&color, &abs_col, dist);
			} else if (mc.is_interface && dist) {
				abs_col = *mi_eval_color(&paras->col_out);
				absorb(&color, &abs_col, dist);
			}

			res->r += color.r;
			res->g += color.g;
			res->b += color.b;
		}
	}

	/*
	 * trace refracted ray
	 */
	if (do_refract && specular < (1-miEPS)) {
		if (enter_surf)			/* We are entering the object*/
			scale = (1-specular) * mc.ior_out * mc.ior_out /
						(mc.ior * mc.ior) * rr_scale;
		else				/* We are leaving the surface*/
			scale = (1-specular) * mc.ior * mc.ior /
						(mc.ior_out * mc.ior_out) *
						rr_scale;

		if (mi_refraction_dir(&dir, state, state->ior_in, state->ior) &&
		    (mi_trace_refraction(&color, state, &dir) ||
		     mi_trace_environment(&color, state, &dir))) {
			dist = 0.0f;
			if (state->child)
				dist = state->child->dist;

			color.r *= scale;
			color.g *= scale;
			color.b *= scale;

			if (enter_surf && dist) {	/* entering surface */
				abs_col = *mi_eval_color(&paras->col);
				absorb(&color, &abs_col, dist);
			} else if (mc.is_interface && dist) {
				abs_col = *mi_eval_color(&paras->col_out);
				absorb(&color, &abs_col, dist);
			}

			res->r += color.r;
			res->g += color.g;
			res->b += color.b;
			res->a += (1-specular)*color.a;
		}
	}

        if(state->options->scanline == 'r') {
                /* Set the opacity - for rapid scanline rendering.
                   For a transparent material rapid scanline would
                   otherwise also shade the triangles on the backside
                   of a volume (geometry), and later add both results.
                   Set opacity to tell rcrm that it should only take
                   the frontmost tri. */           
                miColor opacity;
                opacity.r = 1.0f;
                opacity.g = 1.0f;
                opacity.b = 1.0f;
                opacity.a = 1.0f;  
                mi_opacity_set(state, &opacity);
        }
        
	return(miTRUE);
}
