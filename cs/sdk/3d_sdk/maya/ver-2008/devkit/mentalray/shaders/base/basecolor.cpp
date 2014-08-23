/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	26.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_color_alpha
 *	mib_color_alpha_version
 *	mib_color_intensity
 *	mib_color_intensity_version
 *	mib_color_average
 *	mib_color_average_version
 *
 * History:
 *	13.01.98: added factor parameters
 *
 * Description:
 *	Color conversion routines
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct mc {
	miColor		input;
	miScalar	factor;
};


extern "C" DLLEXPORT int mib_color_alpha_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_color_alpha(
	miColor		*result,
	miState		*state,
	struct mc	*paras)
{
	register miScalar factor = *mi_eval_scalar(&paras->factor);
	if (factor != 0.0)
		*result = *mi_eval_color(&paras->input);
	result->r = result->g = result->b = result->a *= factor;
	return(miTRUE);
}



extern "C" DLLEXPORT int mib_color_average_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_color_average(
	miColor		*result,
	miState		*state,
	struct mc	*paras)
{
	register miScalar factor = *mi_eval_scalar(&paras->factor);
	if (factor != 0.0)
		*result = *mi_eval_color(&paras->input);

	result->r = result->g = result->b = result->a = factor *
				(result->r + result->g + result->b) / 3.0f;
	return(miTRUE);
}


extern "C" DLLEXPORT int mib_color_intensity_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_color_intensity(
	miColor		*result,
	miState		*state,
	struct mc	*paras)
{
	register miScalar factor = *mi_eval_scalar(&paras->factor);
	if (factor != 0.0)
		*result = *mi_eval_color(&paras->input);

	result->r = 
	result->g = 
	result->b = 
	result->a = factor * mi_luminance(state, result);
	return(miTRUE);
}
