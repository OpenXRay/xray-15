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
 * This file shows how to write a mental ray light shader
 * that sets "emit diffuse" and "emit specular" as Maya lights do.
 *
 * This example takes mib_light_point as a starting point (can be found at
 * <maya install directory>/devkit/mentalray/shaders/public-baseshaders/baselight.c)
 * and adds maya specific codes. Changes to the original source code
 * can be found by searching the keyword MAYA.
 *
 * To use extentions provided by the mayabase shader library,
 * the header file mayaapi.h needs to be included which can be found at
 * <maya install directory>/devkit/mentalray/include.
 * If Windows is the working environment, the mayabase.lib needs
 * to be linked in addition to shader.lib. Both libraries are located at
 * <maya install directory>/devkit/mentalray/lib/<ntplatform>.
 *****************************************************************************/

#include <math.h>
#include <shader.h>
#include <geoshader.h>

/***************************************************************
 * MAYA
 *
 * Necessary header file
 * to access maya specific information
 ***************************************************************/
#include "mayaapi.h"

#define EPS	 1e-4
#define BLACK(C) ((C).r==0 && (C).g==0 && (C).b==0)


/*
 * Modification of mib_light_point
 * 
 */

struct maya_light_point {
	miColor		color;		/* color of light source */
	miBoolean	shadow;		/* light casts shadows */
	miScalar	factor;		/* makes opaque objects transparent */
	miBoolean	atten;		/* distance attenuation */
	miScalar	start, stop;	/* if atten, distance range */

	/***********************************************************
	 * MAYA
	 *
	 * Maya specific parameters.
	 ***********************************************************/
	miBoolean	emitDiffuse;
	miBoolean	emitSpecular;
};

extern "C" DLLEXPORT int maya_light_point_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean maya_light_point(
	miColor		*result,
	miState		*state,
	struct maya_light_point *paras)
{
	miScalar	d, t, start, stop;

	/***********************************************************
	 * MAYA
	 *
	 * Pointers to access the shader state
	 ***********************************************************/
	miBoolean *diffuse = NULL, *specular = NULL;

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			/* visible area light*/
		return(miTRUE);
	if (*mi_eval_boolean(&paras->atten)) {			/* dist atten*/
		stop = *mi_eval_scalar(&paras->stop);
		if (state->dist >= stop)
			return(miFALSE);

		start = *mi_eval_scalar(&paras->start);
		if (state->dist > start && fabs(stop - start) > EPS) {
			t = 1.0F - (
			((float)(state->dist) - start) / (stop - start));
			result->r *= t;
			result->g *= t;
			result->b *= t;
		}
	}
	if (*mi_eval_boolean(&paras->shadow)) {			/* shadows: */
		d = *mi_eval_scalar(&paras->factor);
		if (d < 1) {
			miColor filter;
			filter.r = filter.g = filter.b = filter.a = 1;
								/* opaque */
			if (!mi_trace_shadow(&filter,state) || BLACK(filter)) {
				result->r *= d;
				result->g *= d;
				result->b *= d;
				if (d == 0)
					return(miFALSE);
			} else {				/* transparnt*/
				float omf = 1 - d;
				result->r *= d + omf * filter.r;
				result->g *= d + omf * filter.g;
				result->b *= d + omf * filter.b;
			}
		}
	}

	/***********************************************************
	 * MAYA
	 *
	 * Set custom Maya light properties.
	 *
	 * mayabase_stateitem_get returns pointer to
	 * specified MbStateItem in the shader state.
	 * MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR
	 * contains information whether the light
	 * emits diffuse and/or specular.
	 * These values are used by material shaders.
	 * The variable-length parameter should terminated
	 * with MBSI_NULL.
	 ***********************************************************/
	if (mayabase_stateitem_get(state,
		MBSI_LIGHTDIFFUSE, &diffuse,
		MBSI_LIGHTSPECULAR, &specular,
		MBSI_NULL)) {

		/***********************************************************
		 * MAYA
		 *
		 * Sets
		 * MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR
		 * 
		 ***********************************************************/
		*diffuse  = *mi_eval_boolean(&paras->emitDiffuse);
		*specular = *mi_eval_boolean(&paras->emitSpecular);
	}

	return(miTRUE);
}

