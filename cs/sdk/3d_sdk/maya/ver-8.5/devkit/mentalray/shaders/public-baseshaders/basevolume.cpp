/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.1.02
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *      mib_volume()
 *
 * History:
 *
 * Description:
 * simple volume shader
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "shader.h"

/*
 * a very simple fog shader. The shader receives a color from *result
 * (probably from a material shader at the far end of the ray) and fades
 * it toward a fog color based on ray length. At a maximum distance, the
 * fog color is considered solid. However, the fog is allowed to have an
 * alpha value which limits the maximum opacity of the fog. By attaching
 * the fog color to a texture shader, this allows transparent volume
 * effects such as smoke or fire (set max to 0 to avoid fading). If the
 * "lightrays" parameter is true, the shader also applies to light rays;
 * this is the correct thing to do but often unwanted or too slow if the
 * number of light rays is very large.
 */

extern "C" DLLEXPORT int mib_volume_version(void) {return(1);}

struct mib_volume {
	miColor		color;		/* fog color */
	miScalar	max;		/* distance where fog is opaque */
	miBoolean	lightrays;	/* applies to light rays too */
};

extern "C" DLLEXPORT miBoolean mib_volume(
	miColor		*result,	/* in: color at far end, out */
	miState		*state,
	struct mib_volume *paras)
{
	miColor		*color;		/* fog color */
	miScalar	max, fade;	/* max opcity distance, fade factor */

	if (!*mi_eval_boolean(&paras->lightrays) && state->type==miRAY_LIGHT)
		return(miTRUE);				/* ignore light rays?*/

	color =  mi_eval_color (&paras->color);
	max   = *mi_eval_scalar(&paras->max);
	if (state->dist <= 0 ||				/* infinite distance */
	    state->dist >= max) {			/* full-opacity dist */
		fade = 1 - color->a;
		result->r = fade * result->r + color->r;
		result->g = fade * result->g + color->g;
		result->b = fade * result->b + color->b;
		result->a = fade * result->a + color->a;
		return(miTRUE);
	}

	fade = state->dist / max;			/* fade to fog color */
	fade = (1 - fade * fade) * color->a;

	result->r = fade * result->r + (1-fade) * color->r;
	result->g = fade * result->g + (1-fade) * color->g;
	result->b = fade * result->b + (1-fade) * color->b;
	result->a = fade * result->a + (1-fade) * color->a;
	return(miTRUE);
}
