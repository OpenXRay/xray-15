/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	31.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *      mib_illum_cooktoor
 *      mib_illum_cooktoor_version
 *
 * History:
 *	17.11.97: added ambience parameter
 *	27.01.98: added global illumination capabilities
 *
 * Description:
 *      Perform illumination with the following reflection model:
 *      - Cook-Torrance specular (off-specular peak and color-shift) plus
 *      - Lambert diffuse (cosine) plus
 *      - ambient (constant)
 *
 * The Cook-Torrance reflection model is described in Foley et al,
 * "Computer Graphics", pp. 764--770 and by Roy Hall, "Illumination
 * and Color", p. 96 plus appendix III.5:
 *
 *        F D G
 *    --------------
 *    pi (n.v) (n.l)
 *
 * where
 *
 * D is the Beckmann microfacet distribution function,
 * G is the geometrical attenuation factor,
 * F is the Fresnel term (wavelength-dependent).
 *
 * The two main features of the Cook-Torrance reflection model are:
 *   - the reflected color changes with angle.
 *   - it has an off-specular peak.
 *****************************************************************************/

#ifdef HPUX
#pragma OPT_LEVEL 1	/* workaround for HP/UX optimizer bug, +O2 and +O3 */
#endif

#include <stdio.h>
#include <stdlib.h>		/* for abs */
#include <float.h>		/* for FLT_MAX */
#include <math.h>
#include <string.h>
#include <assert.h>
#include "shader.h"
#include "mi_shader_if.h"

struct mib_illum_cooktorr {
	miColor		ambience;	/* ambient color multiplier */
	miColor		ambient;	/* ambient color */
	miColor		diffuse;	/* diffuse color */
	miColor		specular;	/* specular color */
	miScalar	roughness;	/* roughness: avg. microfacet slope */
	miColor		ior;		/* relative ior for 3 wavelengths */
	int		mode;		/* light mode: 0..2 */
	int		i_light;	/* index of first light */
	int		n_light;	/* number of lights */
	miTag		light[1];	/* list of lights */
};


extern "C" DLLEXPORT int mib_illum_cooktorr_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_illum_cooktorr(
	miColor		*result,
	miState		*state,
	struct mib_illum_cooktorr *paras)
{
	miColor		*ambi, *diff, *spec;
	miScalar	roughness;	/* average microfacet slope */
	miColor		*ior;		/* index of refraction */
	miTag		*light;		/* tag of light instance */
	int		n_l;		/* number of light sources */
	int		i_l;		/* offset of light sources */
	int		mode;		/* light mode: 0=all, 1=incl, 2=excl */
	int		samples;	/* # of samples taken */
	miColor		color;		/* color from light source */
	miColor		sum;		/* summed sample colors */
	miVector	dir;		/* direction towards light */
	miScalar	dot_nl;		/* dot prod of normal and dir */
	miColor		refl;		/* specular reflection color */

        /* check for illegal calls */
        if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		return(miFALSE);
	}

        
	ambi      = mi_eval_color(&paras->ambient);
	diff      = mi_eval_color(&paras->diffuse);
	spec      = mi_eval_color(&paras->specular);
	roughness = *mi_eval_scalar(&paras->roughness);
	ior       = mi_eval_color(&paras->ior);

	*result    = *mi_eval_color(&paras->ambience);	/* ambient term */
	result->r *= ambi->r;
	result->g *= ambi->g;
	result->b *= ambi->b;

	mode  = *mi_eval_integer(&paras->mode);
	n_l   = *mi_eval_integer(&paras->n_light);
	i_l   = *mi_eval_integer(&paras->i_light);
	light =  mi_eval_tag(paras->light) + i_l;

	if (mode == 1)		/* modify light list (inclusive mode) */
		mi_inclusive_lightlist(&n_l, &light, state);
	else if (mode == 2)	/* modify light list (exclusive mode) */
		mi_exclusive_lightlist(&n_l, &light, state);
	else if (mode == 4) {
		n_l = 0;
		light = 0;
	}

	/* Loop over all light sources */
	if (mode == 4 || n_l) {
		for (mi::shader::LightIterator iter(state, light, n_l);
		     !iter.at_end(); ++iter) {
			sum.r = sum.g = sum.b = 0;
			samples = 0;
			while (iter->sample()) {
				iter->get_contribution(&color);
				dot_nl = iter->get_dot_nl();
				/* Diffuse reflection: Lambert's cosine law */
				sum.r += dot_nl * diff->r * color.r;
				sum.g += dot_nl * diff->g * color.g;
				sum.b += dot_nl * diff->b * color.b;

				/* Specular reflection: Cook-Torrance reflection */
				dir = iter->get_direction();
				if (mi_cooktorr_specular(&refl, &state->dir, &dir,
							 &state->normal, roughness, ior)) {
					sum.r += refl.r * spec->r * color.r;
					sum.g += refl.g * spec->g * color.g;
					sum.b += refl.b * spec->b * color.b;
				}
			}
			samples = iter->get_number_of_samples();
			if (samples) {
				result->r += sum.r / samples;
				result->g += sum.g / samples;
				result->b += sum.b / samples;
			}
		}
	}

	/* add contribution from indirect illumination (caustics) */
	mi_compute_irradiance(&color, state);
	result->r += color.r * diff->r;
	result->g += color.g * diff->g;
	result->b += color.b * diff->b;
	result->a  = 1;
	return(miTRUE);
}
