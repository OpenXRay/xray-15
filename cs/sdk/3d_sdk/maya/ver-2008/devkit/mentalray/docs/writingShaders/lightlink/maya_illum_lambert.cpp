/******************************************************************************
 * Copyright (C) 1986-2007 mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 * Portions Copyright (C) 1997-2006 Alias Systems Corp.
 ******************************************************************************
// Copyright (C) 1997-2006 Autodesk, Inc., and/or its licensors.
// All rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files contain unpublished
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its licensors,
// which is protected by U.S. and Canadian federal copyright law and by
// international treaties.
//
// The Data is provided for use exclusively by You. You have the right to use,
// modify, and incorporate this Data into other products for purposes authorized
// by the Autodesk software license agreement, without fee.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. AUTODESK
// DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES
// INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF NON-INFRINGEMENT,
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR ARISING FROM A COURSE
// OF DEALING, USAGE, OR TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS
// LICENSORS BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL,
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS
// LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.
*/

/******************************************************************************
 * This file shows how to write a mental ray shader using Maya light linking.
 * Note that this has changed as of Maya 2008. If the light mode is set to
 * 4, light linking information from Maya is translated to native mental ray
 * light linking which gets handled automatically by the mental ray core.
 * Shaders now receive the relevant light information automatically.
 *
 * This example takes mib_illum_lambert as a starting point
 * (can be found at <maya install directory> in
 * ./devkit/mentalray/shaders/public-baseshaders/baselambert.c)
 * and adds code for the light linking.
 *
 * To use extentions provided by the mayabase shader library,
 * the header file mayaapi.h needs to be included which can be found at
 * <maya install directory>/devkit/mentalray/include.
 * In a Windows working environment, the library mayabase.lib
 * needs to be linked in addition to shader.lib. They are located at
 * <maya install directory>/devkit/mentalray/lib/<ntplatform>. Additionally,
 * the location of mayabase.lib needs to be you PATH environment variable
 * when using the shader.
 *****************************************************************************/

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

struct maya_illum_lambert {
	miColor     ambience;   /* ambient color multiplier */
	miColor     ambient;    /* ambient color */
	miColor     diffuse;    /* diffuse color */
	int         mode;       /* light mode: 0..2, 4 */
	int         i_light;    /* index of first light */
	int         n_light;    /* number of lights */
	miTag       light[1];   /* list of lights */
	/*********************************************************************
	 * MAYA LIGHT LINKING
	 *********************************************************************/
	// No longer necessary as of Maya 2008, as long as light link mode
	// is set to 4.
	//
	//miTag       lightLink;  /* light linker */
	//miBoolean   miLightLink;/* enable auto light links on light array? */
};


extern "C" DLLEXPORT int maya_illum_lambert_version(void) {return(3);}

extern "C" DLLEXPORT miBoolean maya_illum_lambert(
	miColor		*result,
	miState		*state,
	struct maya_illum_lambert *paras)
{
	miColor		*ambi, *diff;
	miTag		*light;		/* tag of light instance */
	int		n_l;		/* number of light sources */
	int		i_l;		/* offset of light sources */
	int		m;		/* light mode: 0=all, 1=incl, 2=excl, 4=native mental ray */
	int		samples;	/* # of samples taken */
	miColor		color;		/* color from light source */
	miColor		sum;		/* summed sample colors */
	miScalar	dot_nl;		/* dot prod of normal and dir*/

	/*********************************************************************
	 * MAYA LIGHT LINKING 
	 *********************************************************************/
	// No longer necessary as of Maya 2008, as long as light link mode
	// is set to 4.
	//
	//int numLightLinks;
	//miTag lightLink, *lightLinks;

	/* check for illegal calls */
	if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		return(miFALSE);
	}

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
	else if (m == 4) {
		n_l = 0;
		light = 0;
	}

	/*********************************************************************
	 * MAYA LIGHT LINKING
	 * Get light linker shader.
	 *********************************************************************/
	// No longer necessary as of Maya 2008, as long as light link mode
	// is set to 4.
	//
	//lightLink = *mi_eval_tag(&paras->lightLink);
	//if (lightLink) {
	//	mayabase_lightlink_get(
	//		lightLink, &numLightLinks, &lightLinks, state);
	//}

	/* Loop over all light sources */
	if (m==4 || n_l)
	for (mi::shader::LightIterator iter(state, light, n_l);
	     !iter.at_end(); ++iter) {
		sum.r = sum.g = sum.b = 0;

		/*************************************************************
		 * MAYA LIGHT LINKING
		 * Check linking of current light.
		 *************************************************************/
		// No longer necessary as of Maya 2008, as long as light link mode
		// is set to 4.
		//
		//if (lightLink && !mayabase_lightlink_check(
		//	*iter, numLightLinks, lightLinks, state)) {
		//	/* Light not linked to current instance. */
		//	continue;
		//}

		while (iter->sample()) {
			dot_nl = iter->get_dot_nl();
			iter->get_contribution(&color);
			sum.r += dot_nl * diff->r * color.r;
			sum.g += dot_nl * diff->g * color.g;
			sum.b += dot_nl * diff->b * color.b;
		}
		samples = iter->get_number_of_samples();
		if (samples > 0) {
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

