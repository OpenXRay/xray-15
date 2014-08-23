/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	31.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_wave
 *	mib_texture_wave_version
 *
 * History:
 *
 * Description:
 * cosine waves
 *****************************************************************************/

#include <math.h>
#include <assert.h>
#include <shader.h>


/*------------------------------------------ mib_texture_wave ---------------*/

struct mib_texture_wave {
	miVector	coord;
	miScalar	amplitude_x;
	miScalar	amplitude_y;
	miScalar	amplitude_z;
	miScalar	offset;
};

extern "C" DLLEXPORT int mib_texture_wave_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_wave(
	miColor		*result,
	miState		*state,
	struct mib_texture_wave *paras)
{
	miVector	*t = mi_eval_vector(&paras->coord);

	result->r = result->g = result->b = result->a =
		  *mi_eval_scalar(&paras->offset)
		+ *mi_eval_scalar(&paras->amplitude_x) * (float)cos(t->x * 2*M_PI)
		+ *mi_eval_scalar(&paras->amplitude_y) * (float)cos(t->y * 2*M_PI)
		+ *mi_eval_scalar(&paras->amplitude_z) * (float)cos(t->z * 2*M_PI);
	return(miTRUE);
}
