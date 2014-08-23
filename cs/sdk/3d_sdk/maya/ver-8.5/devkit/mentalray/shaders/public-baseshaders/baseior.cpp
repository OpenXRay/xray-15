/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_refraction_index
 *	mib_refraction_index_version
 *
 * History:
 *
 * Description:
 *	Find the appropriate index of refraction by traversing the state-list.
 *****************************************************************************/

#include <stddef.h>
#include <math.h>
#include <shader.h>


struct mri {
	miScalar mtl_ior;
};


struct mri_result {
	miScalar ior;
	miBoolean enter;
};

static miBoolean find_refraction_index(
	miState *state,
	miScalar *ior_in,
	miScalar *ior_out,
	miScalar mtl_ior);


extern "C" DLLEXPORT int mib_refraction_index_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_refraction_index(
	struct mri_result *result,
	miState		*state,
	struct mri	*paras)
{
	miScalar	ior_in, ior_out;

	result->enter = find_refraction_index(state, &ior_in, &ior_out,
	*mi_eval_scalar(&paras->mtl_ior));
	result->ior = ior_out/ior_in;
	return(miTRUE);
}


/*
 * slightly modified search-routine from softshade.c
 */

static miBoolean find_refraction_index(
	miState		*state,
	miScalar	*ior_in,
	miScalar	*ior_out,
	miScalar	mtl_ior)
{
	register int	num = 0;
	register miState*s, *s_in = NULL;	/* for finding enclosing mtl */

	for (s=state; s; s=s->parent)			/* history? */
		if ((s->type == miRAY_TRANSPARENT		||
		     s->type == miRAY_REFRACT			||
		     s->type == miPHOTON_TRANSMIT_SPECULAR	||
		     s->type == miPHOTON_TRANSMIT_GLOSSY	||
		     s->type == miPHOTON_TRANSMIT_DIFFUSE) &&
		     s->parent && s->parent->shader == state->shader) {
			num++;
			if (!s_in)
				s_in = s->parent;
		}

	if (!(num & 1)) {				/* even: entering */
		*ior_out = mtl_ior;
		*ior_in	= state->parent && state->parent->ior != 0 ?
							state->parent->ior : 1;
		if (!state->refraction_volume)
			state->refraction_volume = state->volume;
	} else {					/* odd: exiting */
		*ior_in	= mtl_ior;
		*ior_out = s_in && s_in->ior_in != 0 ? s_in->ior_in : 1;
		state->refraction_volume = s_in ? s_in->volume
						: state->camera->volume;
	}
	state->ior_in = *ior_in;
	state->ior    = *ior_out;
	return(!(num & 1));
}
