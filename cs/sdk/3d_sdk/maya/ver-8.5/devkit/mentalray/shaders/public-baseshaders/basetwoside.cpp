/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	29.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_twosided
 *
 * History:
 *	08.12.97: renamed to mib_twosided
 *
 * Description:
 *****************************************************************************/

#include <math.h>
#include <shader.h>


struct mt {
	miColor front;
	miColor back;
};

extern "C" DLLEXPORT int mib_twosided_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_twosided(
	miColor		*result,
	miState		*state,
	struct mt	*paras)
{
	if (state->inv_normal)
		*result = *mi_eval_color(&paras->back);
	else
		*result = *mi_eval_color(&paras->front);
	return(miTRUE);
}
