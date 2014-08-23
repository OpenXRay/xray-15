/******************************************************************************
 * Copyright (C) 1986-2005 mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 * Portions Copyright (C) 1997-2005 Alias Systems Corp.
 ******************************************************************************
/*
// Copyright (C) Alias Systems, a division of Silicon Graphics Limited and/or
// its licensors ("Alias").  All rights reserved.  These coded instructions,
// statements, computer programs, and/or related material (collectively, the
// "Material") contain unpublished information proprietary to Alias, which is
// protected by Canadian and US federal copyright law and by international
// treaties.  This Material may not be disclosed to third parties, or be
// copied or duplicated, in whole or in part, without the prior written
// consent of Alias.  ALIAS HEREBY DISCLAIMS ALL WARRANTIES RELATING TO THE
// MATERIAL, INCLUDING, WITHOUT LIMITATION, ANY AND ALL EXPRESS OR IMPLIED
// WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.  IN NO EVENT SHALL ALIAS BE LIABLE FOR ANY DAMAGES
// WHATSOEVER, WHETHER DIRECT, INDIRECT, SPECIAL, OR PUNITIVE, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, OR IN EQUITY,
// ARISING OUT OF OR RELATED TO THE ACCESS TO, USE OF, OR RELIANCE UPON THE
// MATERIAL.
*/

/******************************************************************************
 * This file shows how to write a mental ray shader
 * that uses maya light linking.
 *
 * This example takes mib_illum_lambert as a starting point
 * (can be found at <maya install directory>/devkit/mentalray/shaders/public-baseshaders/baselambert.c)
 * and adds code for the light linking.
 * A header file mayaapi.h should be included which can be found at
 * <maya install directory>/devkit/mentalray/include
 * If Windows is the working environment, 
 * mayabase.lib needs to be linked in addition to shader.lib.
 * Both libraries are located at
 * <maya install directory>/devkit/mentalray/lib/nt
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
/******************************************************************************
 * MAYA LIGHT LINKING
 *****************************************************************************/
#include "mayaapi.h"

struct mayaLightlink_mib_illum_lambert {
	miColor     ambience;   /* ambient color multiplier */
	miColor     ambient;    /* ambient color */
	miColor     diffuse;    /* diffuse color */
	int         mode;       /* light mode: 0..2 */
	int         i_light;    /* index of first light */
	int         n_light;    /* number of lights */
	miTag       light[1];   /* list of lights */
	/******************************************************************************
	 * MAYA LIGHT LINKING 
	 *****************************************************************************/
	miTag       lightLink;  /* light linker */
	miBoolean	miLightLink;	/* use light linker */
};


DLLEXPORT int mayaLightlink_mib_illum_lambert_version(void) {return(2);}

DLLEXPORT miBoolean mayaLightlink_mib_illum_lambert(
	miColor		*result,
	miState		*state,
	struct mayaLightlink_mib_illum_lambert *paras)
{
	miColor		*ambi, *diff;
	miTag		*light;		/* tag of light instance */
	int		n_l;		/* number of light sources */
	int		i_l;		/* offset of light sources */
	int		m;		/* light mode: 0=all, 1=incl, 2=excl */
	int		n;		/* light counter */
	int		samples;	/* # of samples taken */
	miColor		color;		/* color from light source */
	miColor		sum;		/* summed sample colors */
	miVector	dir;		/* direction towards light */
	miScalar	dot_nl;		/* dot prod of normal and dir*/

	/******************************************************************************
	 * MAYA LIGHT LINKING 
	 *****************************************************************************/
	int numLightLinks;
	miTag lightLink, *lightLinks;

	ambi =  mi_eval_color(&paras->ambient);
	diff =  mi_eval_color(&paras->diffuse);
	m    = *mi_eval_integer(&paras->mode);

	*result    = *mi_eval_color(&paras->ambience);	/* ambient term */
	result->r *= ambi->r;
	result->g *= ambi->g;
	result->b *= ambi->b;

	n_l   = *mi_eval_integer(&paras->n_light);
	i_l   = *mi_eval_integer(&paras->i_light);
	light =  mi_eval_tag(paras->light) + i_l;

	if (m == 1)		/* modify light list (inclusive mode) */
		mi_inclusive_lightlist(&n_l, &light, state);
	else if (m == 2)	/* modify light list (exclusive mode) */
		mi_exclusive_lightlist(&n_l, &light, state);

	/******************************************************************************
	 * MAYA LIGHT LINKING 
	 *****************************************************************************/
	lightLink = *mi_eval_tag(&paras->lightLink);
	if (lightLink) 
	{
		mayabase_lightlink_get(
			lightLink, &numLightLinks, &lightLinks, state);
	}

	/* Loop over all light sources */
	for (n=0; n < n_l; n++, light++) {
		sum.r = sum.g = sum.b = 0;
		samples = 0;

		/******************************************************************************
		 * MAYA LIGHT LINKING 
		 *****************************************************************************/
		if (lightLink && !mayabase_lightlink_check(
			*light, numLightLinks, lightLinks, state))
		{
			/* Light not linked to current instance. */
			continue;
		}

		while (mi_sample_light(&color, &dir, &dot_nl, state,
					*light, &samples)) {
			sum.r += dot_nl * diff->r * color.r;
			sum.g += dot_nl * diff->g * color.g;
			sum.b += dot_nl * diff->b * color.b;
		}
		if (samples) {
			result->r += sum.r / samples;
			result->g += sum.g / samples;
			result->b += sum.b / samples;
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

