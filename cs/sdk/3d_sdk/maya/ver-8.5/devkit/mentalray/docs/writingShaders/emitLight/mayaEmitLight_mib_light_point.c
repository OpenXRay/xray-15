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
 * This file shows how to write a mental ray light shader
 * that sets "emit diffuse" and "emit specular" 
 * as maya lights do.
 *
 * This example takes mib_light_point as a starting point
 * (can be found at <maya install directory>/devkit/mentalray/shaders/public-baseshaders/baselight.c)
 * and adds maya specific codes.
 * Changes to the original source code
 * can be found by searching the keyword MAYA
 *
 * A header file mayaapi.h should be included which can be found at
 * <maya install directory>/devkit/mentalray/include
 * If Windows is the working environment, 
 * mayabase.lib needs to be linked in addition to shader.lib.
 * Both libraries are located at
 * <maya install directory>/devkit/mentalray/lib/nt
 *****************************************************************************/

#include <math.h>
#include <shader.h>
#include <geoshader.h>

/******************************************************************************
 * MAYA
 *
 * Necessary header file
 * to access maya specific information
 *****************************************************************************/
#include "mayaapi.h"

#define EPS	 1e-4
#define BLACK(C) ((C).r==0 && (C).g==0 && (C).b==0)


/*
 * Modification of mib_light_point
 * 
 */

struct mayaEmitLight_mib_light_point {
	/* mib_light_point parameters */
	miColor		color;		
	miBoolean	shadow;		
	miScalar	factor;		
	miBoolean	atten;		
	miScalar	start, stop;

	/******************************************************************************
	 * MAYA
	 *
	 * maya specific parameters 
	 *****************************************************************************/
	miBoolean	emitDiffuse;
	miBoolean	emitSpecular;
};

DLLEXPORT int mayaEmitLight_mib_light_point_version(void) {return(1);}

DLLEXPORT miBoolean mayaEmitLight_mib_light_point(
	register miColor	*result,
	register miState	*state,
	register struct mayaEmitLight_mib_light_point *paras)
{
	register miScalar	d, t, start, stop;
	MbStateItem queryItem;

	/******************************************************************************
	 * MAYA
	 *
	 * Pointers to access the shader state
	 *****************************************************************************/
	miBoolean *diffuse, *specular;

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			/* visible area light*/
		return(miTRUE);
	if (*mi_eval_boolean(&paras->atten)) {			/* dist atten*/
		stop = *mi_eval_scalar(&paras->stop);
		if (state->dist >= stop)
			return(miFALSE);

		start = *mi_eval_scalar(&paras->start);
		if (state->dist > start && fabs(stop - start) > EPS) {
			t = 1 - (state->dist - start) / (stop - start);
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
				double omf = 1 - d;
				result->r *= d + omf * filter.r;
				result->g *= d + omf * filter.g;
				result->b *= d + omf * filter.b;
			}
		}
	}

	/******************************************************************************
	 * MAYA
	 *
	 * Get pointers to the shader state variables.
	 *
	 * mayabase_stateitem_get returns pointer to
	 * specified MbStateItem in the shader state.
	 * MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR
	 * contains information about 
	 * whether the light emits diffuse and specualr.
	 * These values are used by material shaders.
	 * The variable-length parameter should terminated
	 * with MBSI_NULL.
	 *****************************************************************************/
	mayabase_stateitem_get(state, 
		MBSI_LIGHTDIFFUSE, &diffuse,
		MBSI_LIGHTSPECULAR, &specular,
		MBSI_NULL
	);

	/******************************************************************************
	 * MAYA
	 *
	 * Sets
	 * MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR
	 * 
	 *****************************************************************************/
	*diffuse	= *mi_eval_boolean(&paras->emitDiffuse);
	*specular	= *mi_eval_boolean(&paras->emitSpecular);

	return(miTRUE);
}


