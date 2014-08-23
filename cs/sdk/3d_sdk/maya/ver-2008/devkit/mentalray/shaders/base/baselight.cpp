/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	13.2.98
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_light_point
 *	mib_light_point_version
 *	mib_light_spot
 *	mib_light_spot_version
 *	mib_light_infinite
 *	mib_light_infinite_version
 *	mib_light_photometric
 *	mib_light_photometric_version
 *	mib_cie_d
 *	mib_cie_d_version
 *	mib_blackbody
 * 	mib_blackbody_version
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


/*
 * Point light source. Must have an origin in the input file. Ignores the
 * direction. If this light shader is called because the light source is a
 * visible area light source and was hit, the result is simply paras->color
 * (the color of the light source).
 */

struct mib_light_point {
	miColor		color;		/* color of light source */
	miBoolean	shadow;		/* light casts shadows */
	miScalar	factor;		/* makes opaque objects transparent */
	miBoolean	atten;		/* distance attenuation */
	miScalar	start, stop;	/* if atten, distance range */
};

extern "C" DLLEXPORT int mib_light_point_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_light_point(
	register miColor	*result,
	register miState	*state,
	register struct mib_light_point *paras)
{
	register miScalar	d, t, start, stop;

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			/* visible area light*/
		return(miTRUE);
	if (*mi_eval_boolean(&paras->atten)) {			/* dist atten*/
		stop = *mi_eval_scalar(&paras->stop);
		if (state->dist >= stop)
			return(miFALSE);

		start = *mi_eval_scalar(&paras->start);
		if (state->dist > start && fabs(stop - start) > EPS) {
			t = 1.F - (miScalar)((state->dist - start) / (stop - start));
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
				miScalar omf = 1 - d;
				result->r *= d + omf * filter.r;
				result->g *= d + omf * filter.g;
				result->b *= d + omf * filter.b;
			}
		}
	}
	return(miTRUE);
}


/*
 * Spot light source. Must have an origin and direction in the input file.
 * Takes a variable number of parameters. The first is a boolean which says
 * whether the light casts a shadow or not. The second is the shadow factor.
 */

struct mib_light_spot {
	miColor		color;		/* color of light source */
	miBoolean	shadow;		/* light casts shadows */
	miScalar	factor;		/* makes opaque objects transparent */
	miBoolean	atten;		/* distance attenuation */
	miScalar	start, stop;	/* if atten, distance range */
	miScalar	cone;		/* inner solid cone */
};

extern "C" DLLEXPORT int mib_light_spot_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_light_spot(
	register miColor	*result,
	register miState	*state,
	register struct mib_light_spot *paras)
{
	register miScalar	d, t, start, stop, cone;
	miScalar		spread;
	miVector		ldir, dir;
	miTag			ltag;

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			/* visible area light*/
		return(miTRUE);
								/*angle atten*/
	ltag = ((miInstance *)mi_db_access(state->light_instance))->item;
	mi_db_unpin(state->light_instance);
	mi_query(miQ_LIGHT_DIRECTION, state, ltag, &ldir);
	mi_vector_to_light(state, &dir, &state->dir);
	mi_vector_normalize(&dir);
	d = mi_vector_dot(&dir, &ldir);
	if (d <= 0)
		return(miFALSE);
	mi_query(miQ_LIGHT_SPREAD, state, ltag, &spread);
	if (d < spread)
		return(miFALSE);
	cone = *mi_eval_scalar(&paras->cone);
	if (d < cone) {
		t = 1 - (d - cone) / (spread - cone);
		result->r *= t;
		result->g *= t;
		result->b *= t;
	}
	if (*mi_eval_boolean(&paras->atten)) {			/* dist atten*/
		stop = *mi_eval_scalar(&paras->stop);
		if (state->dist >= stop)
			return(miFALSE);

		start = *mi_eval_scalar(&paras->start);
		if (state->dist > start && fabs(stop - start) > EPS) {
			t = 1.F - (miScalar)((state->dist - start) / (stop - start));
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
				miScalar omf = 1 - d;
				result->r *= d + omf * filter.r;
				result->g *= d + omf * filter.g;
				result->b *= d + omf * filter.b;
			}
		}
	}
	return(miTRUE);
}


/*
 * Infinite light source. Must have a direction in the input file, and
 * may have an optional origin. Takes two parameters. The first is a
 * boolean which says whether the light casts a shadow or not. The second
 * is the shadow factor.
 */

struct mib_light_infinite {
	miColor		color;		/* color of light source */
	miBoolean	shadow;		/* light casts shadows */
	miScalar	factor;		/* makes opaque objects transparent */
};

extern "C" DLLEXPORT int mib_light_infinite_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_light_infinite(
	register miColor	*result,
	register miState	*state,
	register struct mib_light_infinite *paras)
{
	register miScalar	d;

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			/* visible area light*/
		return(miFALSE);
								/* shadows: */
	d = *mi_eval_scalar(&paras->factor);
	if (*mi_eval_boolean(&paras->shadow) && d < 1) {
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
			miScalar omf = 1 - d;
			result->r *= d + omf * filter.r;
			result->g *= d + omf * filter.g;
			result->b *= d + omf * filter.b;
		}
	}
	return(miTRUE);
}


/*
 * A simple photometric light source.
 * Behaves like a point light source. The light distribution is
 * given by the light profile. The color argument is the brightest
 * color in the profile. start and stop allow to limit the
 * attenuation given by the light exponent to a distance interval.
 */

typedef struct {
	miColor		color;		/* color of light source */
	miBoolean	shadow;		/* light casts shadows? */
	miScalar	factor;		/* makes opaque objects transparent */
	miScalar	start;		/* start attenuation */
	miScalar	stop;		/* end attenuation */
	miTag		profile;	/* light profile to use */
} simple_photometric_light_t;

extern "C" DLLEXPORT int mib_light_photometric_version(void) { return 1; }

extern "C" DLLEXPORT miBoolean mib_light_photometric(
	miColor 			*result,
	miState 			*state,
	simple_photometric_light_t 	*paras)
{
	
        miScalar intensity = 1.0f;
        miTag    lp_tag = *mi_eval_tag(&paras->profile); /* light profile tag */
	miScalar d;
	miTag	 ltag = 0;	/* light tag */
	miScalar exponent; /* light decay */
	miScalar sa = *mi_eval_scalar(&paras->start);
	miScalar ea = *mi_eval_scalar(&paras->stop);
	miBoolean atten = (miBoolean) (ea > sa && ea > 0);

	*result = *mi_eval_color(&paras->color);
	if (state->type != miRAY_LIGHT)			
		return(miTRUE);

	mi_query(miQ_INST_ITEM, state, state->light_instance, &ltag);
	mi_query(miQ_LIGHT_EXPONENT, state, ltag, &exponent);
	if(exponent > 0.0f) {
		if (!atten || (atten && sa<state->dist && state->dist<ea)) {
			if (exponent == 2.0f) {
				intensity = 1.0f/(miScalar)(state->dist*state->dist);
			} else if (exponent == 1.0f) {	
				intensity = 1.0f/(miScalar)state->dist;
			} else {
				intensity = 1.0f/(miScalar)pow(
					(double)state->dist, (double)exponent);
			}
		}
	}

	if(lp_tag) {
        	intensity *= mi_lightprofile_sample(state, lp_tag, miTRUE);
	}

       	result->r *= intensity;
       	result->g *= intensity;
       	result->b *= intensity;
								/* shadows: */
	d = *mi_eval_scalar(&paras->factor);
	if (*mi_eval_boolean(&paras->shadow) && d < 1) {
		miColor filter;
		filter.r = filter.g = filter.b = filter.a = 1;
							/* opaque */
		if (!mi_trace_shadow(&filter,state) || BLACK(filter)) {
			result->r *= d;
			result->g *= d;
			result->b *= d;
			if (d == 0)
				return(miFALSE);
		} else {				/* transparent*/
			miScalar omf = 1 - d;
			result->r *= d + omf * filter.r;
			result->g *= d + omf * filter.g;
			result->b *= d + omf * filter.b;
		}
	}
	return(miTRUE);
}


struct miCie_D {
	miScalar	temperature;
	miScalar	intensity; 
};

extern "C" DLLEXPORT int mib_cie_d_version(void) { return 1; }

/*----------------------------------------------------------------------------
 * The cie_d shaders compute the color associated with the CIE D xx 
 * illuminant model. D 65 corresponds to average daylight for an overcast
 * sky. The 65 stems from the fact that this model corresponds to an
 * effective illumunant temperature of 6500 Kelvin. The other CIE D models
 * are derived from this and parametrized by the temperature. We allow
 * the temperature arguments to be given directly in degrees Kelvin or
 * in the CIE forms with the last two digits truncated. The implemented
 * model is valid for temperatures from 4000 Kelvin to 25000 Kelvin.
 * We compute only the chromacity and the user can choose the intensity
 * of the color by herself. 
 *
 * This is an auxiliary function. The computed color may be passed to
 * a light source.
 */
extern "C" DLLEXPORT void mib_cie_d_init(
	miState		*state,
	struct miCie_D	*para,
	miBoolean	*init)
{
	if (!para) {
		*init = miTRUE;
	} else {
		miScalar t = *mi_eval_scalar(&para->temperature);
		miScalar intensity = *mi_eval_scalar(&para->intensity);
		miScalar x, y, z;	/* chromacity CIE xy coords */
		miColor	*c;
		void	**uptr;

		if (intensity == 0) intensity = 1.0f;

		/* Formula from Wyszecki & Stiles, Color Science, p.145.
	 	 * valid between 4000 and 25000 K. Numbers between 40 and
	 	 * 4000 are multiplied by 100. This allows to specify D 6500 as
	 	 * D 65. Values  below 40 are clamped to 40.
	 	 */
		t = (t >= 4000) ? (1000.0f/t) : (t < 40 ? 0.25F : 10.0F/t);
	
		x = (t > 0.1428571)  
			? (((2.9678F-4.6070F*t)*t+0.0991F )*t+0.244063F)
			: (((1.9018F-2.0064F*t)*t+0.24748F)*t+0.237040F);
		y = (2.870F - 3.0F*x)*x - 0.275F;

		/* transform CIE xy, intensity -> CIE XYZ */
		c = (miColor*) mi_mem_allocate(sizeof(miColor));
		z = 1.0f - x - y;
		intensity /= y;
		c->a = 1.0f;
		c->r = x * intensity;
		c->g = y * intensity;
		c->b = z * intensity;
		mi_colorprofile_ciexyz_to_render(c);

		mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);
		*uptr = c; 
	}
}

extern "C" DLLEXPORT void mib_cie_d_exit(
	miState		*state,
	struct miCie_D	*para)
{
	if (para) {
		void **uptr;
		mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);
		if (*uptr) {
			mi_mem_release(*uptr);
			*uptr = 0;
		}
	}
}

extern "C" DLLEXPORT miBoolean mib_cie_d(
	miColor		*result,
	miState		*state,
	struct miCie_D	*para)
{
	void	**uptr;

	mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);

	*result = *((miColor *) *uptr);
	return miTRUE;
}


/*--------------------------------------------------------------------------
 * calculate the spectral power of a blackbody radiator at a given temperature
 * and wavelength.
 */
static miScalar blackbody(
	miScalar	temperature,/* 1/temperature in Kelvin */
	miScalar	wavelength) /* 1/wave length in nm */
{
	/* use formula 
         *     wavelength^(-5) * 1/( exp(c/(wavelength*temperaure)) - 1)
	 * do not need constant up  front, since we later only do ratios
	 * and use user provided intensity
	 */
	static const miScalar c = 1.438786e+7;  /* hc/k * 10^9, [m*K] */
	miScalar w5;

	w5  = wavelength*wavelength;
	w5 *= w5*wavelength;
	return w5/((miScalar)exp((double)(c*wavelength*temperature)) - 1.0f);
}


extern "C" DLLEXPORT int mib_blackbody_version(void) { return 1; }


/*----------------------------------------------------------------------------
 * calculate the chromacity of a black body radiator at a given temperature.
 * The intensity is a user provided parameter. 
 * This is an auxiliary function. The computed color may be passed to a
 * light source.
 */
extern "C" DLLEXPORT void mib_blackbody_init(
	miState		*state,
	struct miCie_D	*para,
	miBoolean	*init)
{

	if (!para) {
		*init = miTRUE;
	} else {
		/* store x_bar, y_bar, z_bar, 1/wavelength in miColor */
		static const miColor ciexyz31_10[] = {
		   { 0.001368000000F, 0.000039000000F, 0.006450001000F, 380.0F },
		   { 0.004243000000F, 0.000120000000F, 0.020050010000F, 390.0F },
		   { 0.014310000000F, 0.000396000000F, 0.067850010000F, 400.0F },
		   { 0.043510000000F, 0.001210000000F, 0.207400000000F, 410.0F },
		   { 0.134380000000F, 0.004000000000F, 0.645600000000F, 420.0F },
		   { 0.283900000000F, 0.011600000000F, 1.385600000000F, 430.0F },
		   { 0.348280000000F, 0.023000000000F, 1.747060000000F, 440.0F },
		   { 0.336200000000F, 0.038000000000F, 1.772110000000F, 450.0F },
		   { 0.290800000000F, 0.060000000000F, 1.669200000000F, 460.0F },
		   { 0.195360000000F, 0.090980000000F, 1.287640000000F, 470.0F },
		   { 0.095640000000F, 0.139020000000F, 0.812950100000F, 480.0F },
		   { 0.032010000000F, 0.208020000000F, 0.465180000000F, 490.0F },
		   { 0.004900000000F, 0.323000000000F, 0.272000000000F, 500.0F },
		   { 0.009300000000F, 0.503000000000F, 0.158200000000F, 510.0F },
		   { 0.063270000000F, 0.710000000000F, 0.078249990000F, 520.0F },
		   { 0.165500000000F, 0.862000000000F, 0.042160000000F, 530.0F },
		   { 0.290400000000F, 0.954000000000F, 0.020300000000F, 540.0F },
		   { 0.433449900000F, 0.994950100000F, 0.008749999000F, 550.0F },
		   { 0.594500000000F, 0.995000000000F, 0.003900000000F, 560.0F },
		   { 0.762100000000F, 0.952000000000F, 0.002100000000F, 570.0F },
		   { 0.916300000000F, 0.870000000000F, 0.001650001000F, 580.0F },
		   { 1.026300000000F, 0.757000000000F, 0.001100000000F, 590.0F },
		   { 1.062200000000F, 0.631000000000F, 0.000800000000F, 600.0F },
		   { 1.002600000000F, 0.503000000000F, 0.000340000000F, 610.0F },
		   { 0.854449900000F, 0.381000000000F, 0.000190000000F, 620.0F },
		   { 0.642400000000F, 0.265000000000F, 0.000049999990F, 630.0F },
		   { 0.447900000000F, 0.175000000000F, 0.000020000000F, 640.0F },
		   { 0.283500000000F, 0.107000000000F, 0.000000000000F, 650.0F },
		   { 0.164900000000F, 0.061000000000F, 0.000000000000F, 660.0F },
		   { 0.087400000000F, 0.032000000000F, 0.000000000000F, 670.0F },
		   { 0.046770000000F, 0.017000000000F, 0.000000000000F, 680.0F },
		   { 0.022700000000F, 0.008210000000F, 0.000000000000F, 690.0F },
		   { 0.011359160000F, 0.004102000000F, 0.000000000000F, 700.0F },
		   { 0.005790346000F, 0.002091000000F, 0.000000000000F, 710.0F },
		   { 0.002899327000F, 0.001047000000F, 0.000000000000F, 720.0F },
		   { 0.001439971000F, 0.000520000000F, 0.000000000000F, 730.0F },
		   { 0.000690078600F, 0.000249200000F, 0.000000000000F, 740.0F },
		   { 0.000332301100F, 0.000120000000F, 0.000000000000F, 750.0F },
		   { 0.000166150500F, 0.000060000000F, 0.000000000000F, 760.0F },
		   { 0.000083075270F, 0.000030000000F, 0.000000000000F, 770.0F },
		   { 0.000041509940F, 0.000014990000F, 0.000000000000F, 780.0F } };
		static const int n = sizeof(ciexyz31_10)/sizeof(miColor) - 1;

		miScalar t = *mi_eval_scalar(&para->temperature);
		miScalar intensity = *mi_eval_scalar(&para->intensity);
		void	**uptr;
		miColor	*c;
		int i;
		miScalar b;
		miScalar w;
		miScalar x;
		miScalar y;
		miScalar z;
		t = 1.0F/((t == 0) ? 6500.0F : t);
		if (intensity == 0) intensity = 1.0f;

		/* integrate using trapzoidal rule */
		w  = 1.0f/ciexyz31_10[0].a;
		b  = blackbody(t, w);		
		x  = 0.5F*b*ciexyz31_10[0].r;
		y  = 0.5F*b*ciexyz31_10[0].g;
		z  = 0.5F*b*ciexyz31_10[0].b;

		for (i=1; i<n; ++i) {
			w  = 1.0f/ciexyz31_10[i].a;
			b  = blackbody(t, w);		
			x += b*ciexyz31_10[i].r;
			y += b*ciexyz31_10[i].g;
			z += b*ciexyz31_10[i].b;
		}

		w  = 1.0f/ciexyz31_10[n].a;
		b  = blackbody(t, w);		
		x += 0.5F*b*ciexyz31_10[n].r;
		y += 0.5F*b*ciexyz31_10[n].g;
		z += 0.5F*b*ciexyz31_10[n].b;

		intensity /= y;
		c = (miColor*) mi_mem_allocate(sizeof(miColor)); 
		c->a = 1.0;
		c->r = x * intensity;
		c->g = y * intensity;
		c->b = z * intensity;
		mi_colorprofile_ciexyz_to_render(c);
		mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);
		*uptr = c;
	}
}

extern "C" DLLEXPORT void mib_blackbody_exit(
	miState		*state,
	struct miCie_D	*para)
{
	if (para) {
		void **uptr;
		mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);
		if (*uptr) {
			mi_mem_release(*uptr);
			*uptr = 0;
		}
	}
}

extern "C" DLLEXPORT miBoolean mib_blackbody(
	miColor		*result,
	miState		*state,
	struct miCie_D	*para)
{
	void	**uptr;

	mi_query(miQ_FUNC_USERPTR, state, 0, &uptr);

	*result = *((miColor *) *uptr);
	return miTRUE;
}


