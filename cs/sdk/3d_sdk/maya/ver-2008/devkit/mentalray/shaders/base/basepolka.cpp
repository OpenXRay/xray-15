/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	17.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_polka
 *	mib_texture_polka_version
 *	mib_texture_polkasphere
 *	mib_texture_polkasphere_version
 *
 * History:
 *
 * Description:
 *	If you want polka-dots then you are looking in the right place.
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct mtp {
	miVector	coord;
	miScalar	radius;
	miColor		fg;
	miColor		bg;
};


static miBoolean polka_dot(
	miColor		*result,
	miState		*state,
	struct mtp	*paras,
	double		x,
	double		y,
	double		z)
{
	*result = sqrt(x*x + y*y + z*z) < *mi_eval_scalar(&paras->radius)
						? *mi_eval_color(&paras->fg)
						: *mi_eval_color(&paras->bg);

	return(miTRUE);
}


extern "C" DLLEXPORT int mib_texture_polkadot_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_polkadot(
	miColor		*result,
	miState		*state,
	struct mtp	*paras)
{
	miVector	*x;

	x = mi_eval_vector(&paras->coord);
	return(polka_dot(result, state, paras, x->x, x->y, 0.0));
}


extern "C" DLLEXPORT int mib_texture_polkasphere_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_polkasphere(
	miColor		*result,
	miState		*state,
	struct mtp	*paras)
{
	miVector	*x;

	x = mi_eval_vector(&paras->coord);
	return(polka_dot(result, state, paras, x->x, x->y, x->z));
}
