/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	28.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_color_interpolate
 *	mib_color_interpolate_version
 *	mib_color_mix
 *	mib_color_mix_version
 *	mib_color_spread
 *	mib_color_spread_version
 *
 * History:
 *	29.01.98: added mode 6 to mib_color_mix
 *	16.09.99: added color_base to mib_color_mix (now version 2)
 *	03.01.99: mib_color_interpolate: do the right thing if num < 2
 *	16.07.02: incorrect color mixer mode=0.
 *
 * Description:
 * interpolators and mixers
 *****************************************************************************/

#include <math.h>
#include <shader.h>


/*------------------------------------------ mib_color_interpolate ----------*/

struct mib_color_interpolate {
	miScalar	input;
	miInteger	num;
	miScalar	weight[6];
	miColor		color[8];
};

extern "C" DLLEXPORT int mib_color_interpolate_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_color_interpolate(
	miColor		*result,
	miState		*state,
	struct mib_color_interpolate *paras)
{
	miInteger	num = *mi_eval_integer(&paras->num);
	miScalar	input;
	miScalar	ominput;
	miScalar	prev = 0;
	miScalar	next = 1;
	int		i;

	if (num < 1) {
		result->r = result->g = result->b = result->a = 0;
		return(miTRUE);
	}
	if (num == 1) {
		*result = *mi_eval_color(&paras->color[0]);
		return(miTRUE);
	}
	if (num > 8)
		num = 8;

	input = *mi_eval_scalar(&paras->input);
	if (input <= 0) {
		*result = *mi_eval_color(&paras->color[0]);
		return(miTRUE);
	}
	if (input >= 1) {
		*result = *mi_eval_color(&paras->color[num-1]);
		return(miTRUE);
	}
	for (i=0; i < num-2; i++) {
		next = *mi_eval_scalar(&paras->weight[i]);
		if (input <= next)
			break;
		prev = next;
		next = 1;
	}
	input	= (input - prev) / (next - prev);
	ominput	= 1 - input;
	if (input == 0)
		*result = *mi_eval_color(&paras->color[i]);
	else if (input == 1)
		*result = *mi_eval_color(&paras->color[i+1]);
	else {
		miColor *pcol = mi_eval_color(&paras->color[i]);
		miColor *ncol = mi_eval_color(&paras->color[i+1]);
		result->r = ominput * pcol->r + input * ncol->r;
		result->g = ominput * pcol->g + input * ncol->g;
		result->b = ominput * pcol->b + input * ncol->b;
		result->a = ominput * pcol->a + input * ncol->a;
	}
	return(miTRUE);
}


/*------------------------------------------ mib_color_mix ------------------*/

struct mib_color_mix {
	miInteger	num;
	miInteger	mode[8];
	miScalar	weight[8];
	miColor		color[8];
	miColor		base;
};

extern "C" DLLEXPORT int mib_color_mix_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_color_mix(
	miColor		  *result,
	miState		  *state,
	struct mib_color_mix *paras)
{
	register int	  i, n = *mi_eval_integer(&paras->num), mode;
	register miScalar r, g, b, a;
	register miScalar w, weight;
	register miColor  *color;
	miColor		  *base = mi_eval_color(&paras->base);

	r = base->r;
	g = base->g;
	b = base->b;
	a = base->a;
	if (n > 8) n = 8;
	for (i=0; i < n; i++) {
		mode   = *mi_eval_integer(&paras->mode  [i]);
		color  =  mi_eval_color  (&paras->color [i]);
		weight = *mi_eval_scalar (&paras->weight[i]);

		switch(mode) {
		  default:
		  case 0:
			w = ( 1 - color->a ) * weight;
			r = r * w + color->r * weight;
			g = g * w + color->g * weight;
			b = b * w + color->b * weight;
			a = a * w + color->a * weight;
			break;

		  case 1:
			w = 1 - weight;
			r = r * w + color->r * weight;
			g = g * w + color->g * weight;
			b = b * w + color->b * weight;
			a = a * w + color->a * weight;
			break;

		  case 2:
		  case 3:
			r += color->r * weight;
			g += color->g * weight;
			b += color->b * weight;
			a += color->a * weight;
			break;

		  case 4:
		  case 5:
			r *= color->r * weight;
			g *= color->g * weight;
			b *= color->b * weight;
			a *= color->a * weight;
			break;

		  case 6:
			r *= weight;
			g *= weight;
			b *= weight;
			a  = color->a;
			break;
		}
		if (mode == 3 || mode == 5) {
			if (r < 0) r = 0;	else if (r > 1) r = 1;
			if (g < 0) g = 0;	else if (g > 1) g = 1;
			if (b < 0) b = 0;	else if (b > 1) b = 1;
			if (a < 0) a = 0;	else if (a > 1) a = 1;
		}
	}
	result->r = r;
	result->g = g;
	result->b = b;
	result->a = a;
	return(miTRUE);
}


/*------------------------------------------ mib_color_spread ---------------*/

struct mib_color_spread {
	miColor		input;
	miInteger	num;
	miInteger	mode[8];
	miScalar	weight[8];
};

extern "C" DLLEXPORT int mib_color_spread_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_color_spread(
	register miColor  *result,
	miState		  *state,
	struct mib_color_spread *paras)
{
	register int	  i, n = *mi_eval_integer(&paras->num);
	register miScalar r, g, b, a;
	register miScalar weight;
	miColor		  *input;

	input = mi_eval_color(&paras->input);
	r = input->r;
	g = input->g;
	b = input->b;
	a = input->a;
	if (n > 8) n = 8;

	for (i=0; i < n; i++, result++) {
		weight = *mi_eval_scalar(&paras->weight[i]);
		switch(*mi_eval_integer(&paras->mode[i])) {
		  default:
		  case 0:
			result->r = r * weight;
			result->g = g * weight;
			result->b = b * weight;
			result->a = a * weight;
			break;

		  case 1:
			result->r =
			result->g =
			result->b =
			result->a = a * weight;
			break;

		  case 2:
			result->r =
			result->g =
			result->b =
			result->a = (a + r + g)/3 * weight;
			break;

		  case 3:
			result->r =
			result->g =
			result->b =
			result->a = (.299F * r + .587F * g + .114F * b) * weight;
			break;

		  case 4:
			result->r =
			result->g =
			result->b =
			result->a = r * weight;
			break;
		}
	}
	return(miTRUE);
}
