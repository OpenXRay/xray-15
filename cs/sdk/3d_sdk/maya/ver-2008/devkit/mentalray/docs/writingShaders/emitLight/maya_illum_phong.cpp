/******************************************************************************
 * Copyright (C) 1986-2007 mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 * Portions Copyright (C) 1997-2007 Alias Systems Corp.
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
 * This file shows how to write a mental ray material shader
 * that uses "emit diffuse" and "emit specular" of Maya lights.
 *
 * As a result of changes to the way light linking is handled, this shader
 * now supports Maya light linking *with no addditional code.* See
 * devkit/docs/writingShaders/README.txt for more information.
 *
 * This example takes mib_illum_phong as a starting point
 * (can be found at <maya install directory> in
 * ./devkit/mentalray/shaders/public-baseshaders/basephong.c)
 * and adds maya specific codes. Changes to the original source code
 * can be found by searching the keyword MAYA.
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

/***************************************************************
 * MAYA
 *
 * Necessary header file
 * to access maya specific information
 ***************************************************************/
#include "mayaapi.h"

struct maya_illum_phong {
	/* same as mib_illum_phong parameters */
	miColor		ambience;	
	miColor		ambient;	
	miColor		diffuse;	
	miColor		specular;	
	miScalar	exponent;	
	int		mode;           
	int		i_light;	
	int		n_light;	
	miTag		light[1];
};


extern "C" DLLEXPORT int maya_illum_phong_version(void) {return(3);}

/*
 * Modification of mib_illum_phong.
 * Keep the same interface but extend internal behavior.
 */
extern "C" DLLEXPORT miBoolean maya_illum_phong(
	miColor		*result,
	miState		*state,
	struct maya_illum_phong *paras)
{
	miColor		*ambi, *diff, *spec;
	miTag		*light;		/* tag of light instance */
	int		n_l;		/* number of light sources */
	int		i_l;		/* offset of light sources */
	int		m;		/* light mode: 0=all, 1=incl, 2=excl */
	int		samples;	/* # of samples taken */
	miColor		color;		/* color from light source */
	miColor		sum;		/* summed sample colors */
	miVector	dir;		/* direction towards light */
	miScalar	dot_nl;		/* dot prod of normal and dir */
	miScalar	expo;		/* Phong exponent (cosine power) */
	miScalar	s;		/* amount of specular reflection */

	/***********************************************************
	 * MAYA
	 *
	 * Pointers to access the shader state, and actual values.
	 ***********************************************************/

	miBoolean *diffuse = NULL, *specular = NULL;
	miBoolean emitDiffuse = miTRUE;	/* default emits diffuse */
	miBoolean emitSpecular = miTRUE;/* default emits specular */

	/* check for illegal calls */
	if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		return(miFALSE);
	}
 
	ambi =  mi_eval_color(&paras->ambient);
	diff =  mi_eval_color(&paras->diffuse);
	spec =  mi_eval_color(&paras->specular);
	expo = *mi_eval_scalar(&paras->exponent);
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

	/* Loop over all light sources */
	if (m == 4 || n_l)
	for (mi::shader::LightIterator iter(state, light, n_l);
	     !iter.at_end(); ++iter) {
		sum.r = sum.g = sum.b = 0;

		while (iter->sample()) {
			dot_nl = iter->get_dot_nl();
			dir = iter->get_direction();
			iter->get_contribution(&color);

			/***************************************************
			* MAYA
			*
			* Get custom Maya light properties.
			*
			* mayabase_stateitem_get returns pointer to
			* specified MbStateItem in the shader state.
			* MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR
			* contains information whether the last sampled
			* light emits diffuse and/or specular.
			* The variable-length parameter should terminated
			* with MBSI_NULL.
			*
			* mayabase light shaders always set the shader state
			* value to inform that whether the light shader emits
			* diffuse and specular lights.
			****************************************************/
			if (mayabase_stateitem_get(state,
					MBSI_LIGHTDIFFUSE, &diffuse,
					MBSI_LIGHTSPECULAR, &specular,
					MBSI_NULL)) {

				emitDiffuse = *diffuse;
				emitSpecular = *specular;
			}

			/***************************************************
			* MAYA
			*
			* Add diffuse component
			* only if the light emits diffuse lights
			****************************************************/
			if (emitDiffuse) {
				/* Lambert's cosine law */
				sum.r += dot_nl * diff->r * color.r;
				sum.g += dot_nl * diff->g * color.g;
				sum.b += dot_nl * diff->b * color.b;
			}

			/***************************************************
			* MAYA
			*
			* Add specular component
			* only if the light emits specular lights
			****************************************************/
			if (emitSpecular) {
				/* Phong's cosine power */
				s = mi_phong_specular(expo, state, &dir);
				if (s > 0.0) {
					sum.r += s * spec->r * color.r;
					sum.g += s * spec->g * color.g;
					sum.b += s * spec->b * color.b;
				}
			}
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
