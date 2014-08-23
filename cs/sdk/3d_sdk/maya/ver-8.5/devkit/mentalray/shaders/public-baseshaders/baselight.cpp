/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
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
			t = 1 - (state->dist - start) / (stop - start);
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
				double omf = 1 - d;
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
			t = 1 - (state->dist - start) / (stop - start);
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
				double omf = 1 - d;
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
			double omf = 1 - d;
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
       				intensity = 1.0/(state->dist*state->dist);
			} else if (exponent == 1.0f) {	
				intensity = 1.0/state->dist;
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
			double omf = 1 - d;
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
		t = (t >= 4000) ? (1000.0/t) : (t < 40 ? 0.25 : 10.0/t);
	
		x = (t > 0.1428571)  
				? (((2.9678-4.6070*t)*t+0.0991 )*t+0.244063)
			    	: (((1.9018-2.0064*t)*t+0.24748)*t+0.237040);
		y = (2.870 - 3.0*x)*x - 0.275;

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
		   { 0.001368000000, 0.000039000000, 0.006450001000, 380.0 },
		   { 0.004243000000, 0.000120000000, 0.020050010000, 390.0 },
		   { 0.014310000000, 0.000396000000, 0.067850010000, 400.0 },
		   { 0.043510000000, 0.001210000000, 0.207400000000, 410.0 },
		   { 0.134380000000, 0.004000000000, 0.645600000000, 420.0 },
		   { 0.283900000000, 0.011600000000, 1.385600000000, 430.0 },
		   { 0.348280000000, 0.023000000000, 1.747060000000, 440.0 },
		   { 0.336200000000, 0.038000000000, 1.772110000000, 450.0 },
		   { 0.290800000000, 0.060000000000, 1.669200000000, 460.0 },
		   { 0.195360000000, 0.090980000000, 1.287640000000, 470.0 },
		   { 0.095640000000, 0.139020000000, 0.812950100000, 480.0 },
		   { 0.032010000000, 0.208020000000, 0.465180000000, 490.0 },
		   { 0.004900000000, 0.323000000000, 0.272000000000, 500.0 },
		   { 0.009300000000, 0.503000000000, 0.158200000000, 510.0 },
		   { 0.063270000000, 0.710000000000, 0.078249990000, 520.0 },
		   { 0.165500000000, 0.862000000000, 0.042160000000, 530.0 },
		   { 0.290400000000, 0.954000000000, 0.020300000000, 540.0 },
		   { 0.433449900000, 0.994950100000, 0.008749999000, 550.0 },
		   { 0.594500000000, 0.995000000000, 0.003900000000, 560.0 },
		   { 0.762100000000, 0.952000000000, 0.002100000000, 570.0 },
		   { 0.916300000000, 0.870000000000, 0.001650001000, 580.0 },
		   { 1.026300000000, 0.757000000000, 0.001100000000, 590.0 },
		   { 1.062200000000, 0.631000000000, 0.000800000000, 600.0 },
		   { 1.002600000000, 0.503000000000, 0.000340000000, 610.0 },
		   { 0.854449900000, 0.381000000000, 0.000190000000, 620.0 },
		   { 0.642400000000, 0.265000000000, 0.000049999990, 630.0 },
		   { 0.447900000000, 0.175000000000, 0.000020000000, 640.0 },
		   { 0.283500000000, 0.107000000000, 0.000000000000, 650.0 },
		   { 0.164900000000, 0.061000000000, 0.000000000000, 660.0 },
		   { 0.087400000000, 0.032000000000, 0.000000000000, 670.0 },
		   { 0.046770000000, 0.017000000000, 0.000000000000, 680.0 },
		   { 0.022700000000, 0.008210000000, 0.000000000000, 690.0 },
		   { 0.011359160000, 0.004102000000, 0.000000000000, 700.0 },
		   { 0.005790346000, 0.002091000000, 0.000000000000, 710.0 },
		   { 0.002899327000, 0.001047000000, 0.000000000000, 720.0 },
		   { 0.001439971000, 0.000520000000, 0.000000000000, 730.0 },
		   { 0.000690078600, 0.000249200000, 0.000000000000, 740.0 },
		   { 0.000332301100, 0.000120000000, 0.000000000000, 750.0 },
		   { 0.000166150500, 0.000060000000, 0.000000000000, 760.0 },
		   { 0.000083075270, 0.000030000000, 0.000000000000, 770.0 },
		   { 0.000041509940, 0.000014990000, 0.000000000000, 780.0 } };
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
		t = 1.0/((t == 0) ? 6500.0 : t);
		if (intensity == 0) intensity = 1.0f;

		/* integrate using trapzoidal rule */
		w  = 1.0f/ciexyz31_10[0].a;
		b  = blackbody(t, w);		
		x  = 0.5*b*ciexyz31_10[0].r;
		y  = 0.5*b*ciexyz31_10[0].g;
		z  = 0.5*b*ciexyz31_10[0].b;

		for (i=1; i<n; ++i) {
			w  = 1.0f/ciexyz31_10[i].a;
			b  = blackbody(t, w);		
			x += b*ciexyz31_10[i].r;
			y += b*ciexyz31_10[i].g;
			z += b*ciexyz31_10[i].b;
		}

		w  = 1.0f/ciexyz31_10[n].a;
		b  = blackbody(t, w);		
		x += 0.5*b*ciexyz31_10[n].r;
		y += 0.5*b*ciexyz31_10[n].g;
		z += 0.5*b*ciexyz31_10[n].b;

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


