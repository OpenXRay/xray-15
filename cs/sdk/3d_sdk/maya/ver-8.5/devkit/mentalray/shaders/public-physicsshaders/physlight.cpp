/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	27.06.98
 * Module:	physics
 * Purpose:	Physically plausible light shader
 *
 * Exports:
 *	physical_light_version
 *	physical_light
 *
 * History:
 *	17.06.98: split off from dgsshade.c
 *	01.09.98: reduced the number of parameters, using mi_query()
 *	02.10.98: spot light can be combined with area lights, that is
 *		  area lights can also be spot lights.
 *	14.01.99: Test occlusion last instead of first.  Introduced two
 *		  new parameters: threshold and cos_exp.  Return (miBoolean)2
 *		  if no further samples should be taken on a flat area light
 *		  source (they would all be on the black back side anyways).
 *	10.06.99: use mi_eval
 *	11.02.00: fix area lights by using light space conversion where
 *		  necessary
 *	06.07.00: major cleanup
 *
 * Description:
 * Light sources for global illumination. Physically correct light source with
 * 1/r^2 falloff. For area light source also cosine term (but no area term as
 * the "color" is energy, not radiance). Must have an origin specified in the
 * input file.  If there also is a direction, it is a spot light. "Physically
 * correct" should of course be taken with a grain of salt since they are just
 * mathematical/physical abstractions of real light sources.
 *
 * The Base and most other light sources are not physically correct since the
 * radiance only falls of linearly, if at all. Also, Base/etc area light
 * sources don't have a cos dependence.  Why can we then use these at all for
 * global illumination?  Because we can just consider the first bounce to be
 * the source, and all reflections from there on are physically plausible.
 * Besides, animators and TDs know and love the built-in light source types.
 * BUT --- the photon density from the light has to correspond to the light
 * shader.  If non-physical emission is used, the photon densities or the
 * photon energies have to correspond exactly to the non-physical light shader.
 *
 * The parameter 'threshold' is for optimization: if the illumination is less
 * than threshold, the illumination can be discarded and no shadow rays need
 * to be cast. The parameter 'cos_exp' is for flat area lights only (rectangle
 * and disc): the default cosine illumination distribution is made more narrow
 * by taking cosine to the cos_exp power.
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <physlight.h>

#define miEPS 0.0001

#define mi_MAX(a,b)	((a) > (b) ? (a) : (b))
#define mi_MAX3(a,b,c)  mi_MAX(mi_MAX(a,b), (c));
#define BLACK(c)	((c).r < miEPS && (c).g < miEPS && (c).b < miEPS)

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

static miColor black = {0, 0, 0, 1};


extern "C" DLLEXPORT int physical_light_version(void) {return(3);}

extern "C" DLLEXPORT miBoolean physical_light(
	miColor			*result,
	miState			*state,
	struct physical_light	*parms)
{
	struct physical_light	p, *paras = &p;	/* mi_eval'ed parameters */
	miTag			light = 0;
	miColor			visibility = {1.0, 1.0, 1.0};
	miVector		normal, u, v, axis, spotdir, dir;
	miScalar		cosine, r=state->dist, r2, f, d, area, radius;
	miScalar		exponent, spread;
	miScalar		maxcolor;
	int			type, areatype;

	p.color     = *mi_eval_color (&parms->color);
	p.cone      = *mi_eval_scalar(&parms->cone);
	p.threshold = *mi_eval_scalar(&parms->threshold);
	p.cos_exp   = *mi_eval_scalar(&parms->cos_exp);

	miASSERT(paras->color.r >= 0.0 && paras->color.g >= 0.0 &&
		 paras->color.b >= 0.0);   /* no 'light suckers', please */

	mi_query(miQ_INST_ITEM,  state, state->light_instance,	&light);
	mi_query(miQ_LIGHT_TYPE, state, light,			&type);
	mi_query(miQ_LIGHT_AREA, state, light,			&areatype);

	if (state->type == miRAY_LIGHT) {
		/*
		 * compute irradiance from this light source at the surface of
		 * object
		 */
		mi_query(miQ_LIGHT_EXPONENT, state, light, &exponent);
		r2 = exponent == 0 || exponent == 2 ? r*r : pow(r, exponent);

		switch(areatype) {
		  case miLIGHT_NONE:		/* point, spot or directional*/
			/*
			 * distance attenuation: 4 Pi r^2 is the area of a
			 * sphere around the point. Same normalization as for
			 * spherical light
			 */
			f = type==miLIGHT_DIRECTION ? 1 : 1 / (4 * M_PI * r2);
			break;

		  case miLIGHT_RECTANGLE:	/* rectangular area light */
			mi_query(miQ_LIGHT_AREA_R_EDGE_U, state, light, &u);
			mi_query(miQ_LIGHT_AREA_R_EDGE_V, state, light, &v);
			mi_vector_prod(&normal, &u, &v);
			mi_vector_normalize(&normal);
			mi_vector_to_light(state, &dir, &state->dir);
			mi_vector_normalize(&dir);
			/*
			 * Compute area-to-point form factor (except cos at
			 * the receiver). <cosine> is cos at sender. Returning
			 * 2 means "no color and stop sampling".
			 */
			cosine = mi_vector_dot(&normal, &dir);
			if (cosine <= 0)
				return((miBoolean)2);
			if (paras->cos_exp != 0 && paras->cos_exp != 1)
				cosine = pow(cosine, paras->cos_exp);
			/*
			 * cos term and distance attenuation. No area term
			 * since "color" of the light is energy, not radiance.
			 */
			f = cosine / (M_PI * r2);
			break;

		  case miLIGHT_DISC:		/* disc area light */
			mi_query(miQ_LIGHT_AREA_D_NORMAL, state,light,&normal);
			mi_vector_to_light(state, &dir, &state->dir);
			mi_vector_normalize(&dir);

			cosine = mi_vector_dot(&normal, &dir);
			if (cosine <= 0)
				return((miBoolean)2);
			if (paras->cos_exp != 0 && paras->cos_exp != 1)
				cosine = pow(cosine, paras->cos_exp);
			f = cosine / (M_PI * r2);
			break;

		  case miLIGHT_SPHERE:		/* spherical area light */
		  case miLIGHT_CYLINDER:	/* cylindrical area light */
			f = 1 / (4 * M_PI * r2);
			break;

		  case miLIGHT_OBJECT:
		        /* two sided */
			/*
			 * this is legal for object lights only: state->child
			 * is set according to the intersection with the light
			 * geometry itself
			 */
		        cosine = -state->child->dot_nd;
			if (cosine <= 0)
				return(miFALSE);
			if (paras->cos_exp != 0 && paras->cos_exp != 1)
				cosine = pow(cosine, paras->cos_exp);
		        f = cosine / (4 * M_PI * r2);
			break;

		  default:
			mi_error("physical_light: unknown light source type");
		}
		if (type == miLIGHT_SPOT) {	/* spot light source */
			mi_query(miQ_LIGHT_DIRECTION, state, light, &spotdir);
			miASSERT(fabs(mi_vector_norm(&spotdir) - 1.) < miEPS);
			mi_vector_to_light(state, &dir, &state->dir);
			mi_vector_normalize(&dir);
			d = mi_vector_dot(&dir, &spotdir);
			if (d <= 0)
				return(miFALSE);
			mi_query(miQ_LIGHT_SPREAD, state, light, &spread);
			if (d < spread)
				return(miFALSE);
			if (d < paras->cone)
				f *= 1 - (d - paras->cone) /
					 (spread - paras->cone);
		}

		/*
		 * Return false without checking occlusion (shadow ray) if
		 * color is very dark. (This introduces bias which could be
		 * avoided if probabilities were used to decide whether to
		 * carry on or return here.)
		 */
		maxcolor = mi_MAX3(paras->color.r, paras->color.g,
							paras->color.b);
		if (f * maxcolor < paras->threshold)
			return(miFALSE);

		/*
		 * Check for occlusion by an opaque object and compute
		 * visibility
		 */
		if (!mi_trace_shadow(&visibility, state) || BLACK(visibility))
			return(miFALSE);

		/*
		 * Compute result. Visibility is always (1 1 1) here for
		 * dgs_material() so the multiplication by visibility could be
		 * avoided. But for base light shaders visibility can be less.
		 */
		result->r = f * paras->color.r * visibility.r;
		result->g = f * paras->color.g * visibility.g;
		result->b = f * paras->color.b * visibility.b;
	} else {		/* Visible area light source: return radiance*/
		switch (areatype) {
		  case miLIGHT_RECTANGLE:	/* rectangular area light */
			mi_query(miQ_LIGHT_AREA_R_EDGE_U, state, light, &u);
			mi_query(miQ_LIGHT_AREA_R_EDGE_V, state, light, &v);
			mi_vector_prod(&normal, &u, &v);
			/* Compute radiance: result = paras->color / area */
			mi_normal_to_light(state, &dir, &state->normal);
			if (mi_vector_dot(&normal, &dir) > 0) {
				area = 4.0 * mi_vector_norm(&normal);
				miASSERT(area > 0.0);
				result->r = paras->color.r / area;
				result->g = paras->color.g / area;
				result->b = paras->color.b / area;
			} else
				*result = black;   /* back side is black */
			break;

		  case miLIGHT_DISC:		/* disc area light source */
			mi_query(miQ_LIGHT_AREA_D_NORMAL, state,light,&normal);
			mi_normal_to_light(state, &dir, &state->normal);
			if (mi_vector_dot(&normal, &dir) > 0) {
				mi_query(miQ_LIGHT_AREA_D_RADIUS, state,
							light, &radius);
				area = M_PI * radius * radius;
				miASSERT(area > 0.0);
				result->r = paras->color.r / area;
				result->g = paras->color.g / area;
				result->b = paras->color.b / area;
			} else
				*result = black;
			break;

		  case miLIGHT_SPHERE:		/* spherical area light */
			mi_query(miQ_LIGHT_AREA_S_RADIUS, state,light,&radius);
			area = 4.0 * M_PI * radius * radius;
			miASSERT(area > 0.0);
			result->r = paras->color.r / area;
			result->g = paras->color.g / area;
			result->b = paras->color.b / area;
			break;

		  case miLIGHT_CYLINDER:	/* cylindrical area light */
			mi_query(miQ_LIGHT_AREA_C_RADIUS, state,light,&radius);
			mi_query(miQ_LIGHT_AREA_C_AXIS, state, light, &axis);
			/* area = pi*radius^2 * h = pi*radius^2 * 2 * |axis| */
			area = 2 * M_PI * radius * radius *
						mi_vector_norm(&axis);
			miASSERT(area > 0.0);
			result->r = paras->color.r / area;
			result->g = paras->color.g / area;
			result->b = paras->color.b / area;
			break;

		  case miLIGHT_OBJECT:
		        mi_query(miQ_INST_AREA, state,
				 state->light_instance, &area);
			result->r = paras->color.r / area;
			result->g = paras->color.g / area;
			result->b = paras->color.b / area;
			break;

		  case miLIGHT_NONE:		/* point, spot or directional*/
			miASSERT(0);
			break;

		  default:
			mi_error("physical_light: unknown light source type");
		}
	}
	miASSERT(result->r >= 0 && result->g >= 0 && result->b >= 0);
	return(miTRUE);
}
