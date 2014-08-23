/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	22.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_checkerboard
 *	mib_texture_checkerboard_version
 *
 * History:
 *
 * Description:
 *****************************************************************************/

#include <stddef.h>
#include <shader.h>


struct mtc {
	miVector	coord;
	miScalar	xsize;
	miScalar	ysize;
	miScalar	zsize;
	miColor		color000;
	miColor		color001;
	miColor		color010;
	miColor		color011;
	miColor		color100;
	miColor		color101;
	miColor		color110;
	miColor		color111;
};

static int color_index[] = {
	offsetof(struct mtc, color000),
	offsetof(struct mtc, color001),
	offsetof(struct mtc, color010),
	offsetof(struct mtc, color011),
	offsetof(struct mtc, color100),
	offsetof(struct mtc, color101),
	offsetof(struct mtc, color110),
	offsetof(struct mtc, color111)
};


extern "C" DLLEXPORT int mib_texture_checkerboard_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_checkerboard(
	miColor		*result,
	miState		*state,
	struct mtc	*paras)
{
	miVector	*x;
	miScalar	xsize, ysize, zsize;
	int		index;

	x = mi_eval_vector(&paras->coord);
	xsize = *mi_eval_scalar(&paras->xsize);
	ysize = *mi_eval_scalar(&paras->ysize);
	zsize = *mi_eval_scalar(&paras->zsize);

	index = ((((x->x>xsize)<<1) + (x->y>ysize))<<1) + (x->z>zsize);
	*result = *mi_eval_color((char *)paras+color_index[index]);
	return(miTRUE);
}
