/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	16.2.98
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_shadow_transparency
 *	mib_shadow_transparency_version
 *
 * History:
 *
 * Description:
 *****************************************************************************/

#include <math.h>
#include <shader.h>
#include <geoshader.h>

#define EPS	 1e-4
#define BLACK(C) ((C).r==0 && (C).g==0 && (C).b==0)


static void choose_volume(miState *);


/*
 * Simple shadow shader that controls transparency with a color (which is
 * probably connected to a texture shader). Can deal with all shadow modes.
 */

struct mib_shadow_transparency {
	miColor		color;		/* RGBA material color */
	miColor		transp;		/* RGB transparency */
	int		mode;		/* 0=all, 1=inclusive, 2=exclusive */
	int		i_light;	/* index of first light */
	int		n_light;	/* number of lights */
	miTag		light[1];	/* list of lights */
};


extern "C" DLLEXPORT int mib_shadow_transparency_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_shadow_transparency(
	register miColor	*result,
	register miState	*state,
	register struct mib_shadow_transparency *paras)
{
	int			n, i, mode;
	miTag			*light;
	register miColor	transp;
	register miColor	*color;

	if (state->type != miRAY_SHADOW)
		return(miFALSE);

	mode = *mi_eval_integer(&paras->mode);
	if (mode < 3) {
		n = *mi_eval_integer(&paras->n_light);
		light = mi_eval_tag(paras->light) +
					*mi_eval_integer(&paras->i_light);
		switch(mode) {
		  case 1: mi_inclusive_lightlist(&n, &light, state); break;
		  case 2: mi_exclusive_lightlist(&n, &light, state); break;
		}
		for (i=0; i < n; i++, light++)
			if (state->light_instance == *light)
				break;
		if (i == n)
			return(miTRUE);
	}
	if (state->options->shadow == 's') {
		choose_volume(state);
		mi_trace_shadow_seg(result, state);
	}
	color  =  mi_eval_color(&paras->color);
	transp = *mi_eval_color(&paras->transp);
	transp.r *= color->a * 2;
	transp.g *= color->a * 2;
	transp.b *= color->a * 2;
	result->r *= transp.r < 1 ? transp.r * color->r
				  : (transp.r-1) + (2-transp.r) * color->r;
	result->g *= transp.g < 1 ? transp.g * color->g
				  : (transp.g-1) + (2-transp.g) * color->g;
	result->b *= transp.b < 1 ? transp.b * color->b
				  : (transp.b-1) + (2-transp.b) * color->b;
	return(result->r || result->g || result->b);
}


static void choose_volume(
	miState			*state)
{
	register int		n = 0;		/* same-material counter */
	register miState	*s, *s_in = 0;	/* for finding enclosing mtl */

	for (n=0, s=state; s; s=s->parent)		/* history? */
		if ((s->type == miRAY_TRANSPARENT	   ||
		     s->type == miRAY_REFRACT		   ||
		     s->type == miPHOTON_TRANSMIT_SPECULAR ||
		     s->type == miPHOTON_TRANSMIT_GLOSSY   ||
		     s->type == miPHOTON_TRANSMIT_DIFFUSE) &&
		     s->parent && s->parent->shader == state->shader) {
			n++;
			if (!s_in)
				s_in = s->parent;
		}

	if (n & 1)					/* odd: exiting */
		state->refraction_volume = s_in ? s_in->volume
						: state->camera->volume;
	else						/* even: entering */
		if (!state->refraction_volume)
			state->refraction_volume = state->volume;
}
