/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	20.10.97
 * Module:	physics
 * Purpose:	Global illumination with participating media using photon maps
 *
 * Exports:
 *
 * History:
 *	14.01.98: removed install function, this is now a library
 *	10.02.98: major revisions.
 *	23.02.98: heavy optimizations: secondary rays need less accuracy.
 *	19.05.98: Additional parameters. New meaning of the 'mode' param.
 *	19.06.98: Two lobes for anisotropic scattering.  Use
 *		mi_scattering_pathlength() and mi_choose_lobe().
 *	99: use mi_eval
 *	22.11.99: commented out body of compute_directional_irrad
 *	22.06.00: moved compute_directional_irradiance into raylib
 *		  to reenable anisotropic volume irradiance, but avoid
 *		  exposure of internal data structures.
 *	06.07.00: major cleanup
 *	10.10.01: parti_volume() offset bug fixed
 *
 * Description:
 * Global illumination volume shaders and volume photon shaders for
 * participating media.  The medium can be homogeneous (uniform density) or
 * nonhomogeneous and the scattering can be isotropic (diffuse) or anisotropic.
 * The volume shaders are used for rays originating from the eye. Volume photon
 * shaders are used to trace photons from the light sources.
 *
 * Anisotropic scattering has two lobes.  Each lobe can be either:
 * - Backscattering:  -1 < g < 0
 * - Diffuse (isotropic/uniform):  g = 0
 * - forward scattering:  0 < g < 1
 * The first lobe is weighted by r, the second lobe by 1-r.
 *
 * Anisotropic scattering is modeled as the two-term Schlick model, see
 * Andrew S. Glassner, "Principles of Digital Image Synthesis", vol 2, p. 762
 * and "A rendering algorithm for discrete volume density objects", Ph. Blasi,
 * B. Le Saec, Ch. Schlick, Computer Graphics Forum, vol. 12, no. 3, pp 201--
 * 210, 1993. (Proceedings of Eurographics '93.)
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <mi_shader_if.h>
#include <partishade.h>

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

#ifndef WIN_NT
#ifndef isnan
extern int isnan(double);
#endif
#endif

#ifdef WIN_NT
#include <float.h>
#define isnan(X) _isnan(X)
#endif

#ifndef IRIX
#define ftan tan
#endif

#define miODD(a)	((a) & 1)
#define miEVEN(a)	(!miODD(a))
#define miEPS		0.0001
#define miEPSF		0.0001F
					/* For 3D texture shading */
#define NTERMS		8		/* max # of terms in spectral sum */
#define COLOR_MAP_SIZE	1024		/* number of colors in color map */
#define COLOR_MAP_MAX	(COLOR_MAP_SIZE-1)
#define TABLE_SIZE	0x10000		/* noise table size */
#define TABLE_MAX	(TABLE_SIZE-1)


/*
 * Structure definitions
 */

typedef struct { float weight; float scale; } Term;
typedef struct {
	float		spacing;	/* how close are bands or rings	*/
	float		taper;		/* tan(angle) */
	float		strength;	/* volume of noise for distortion */
	int		no_terms;	/* number of terms in spectral sum */
	Term		terms[NTERMS];	/* terms of the spectral sum */
	float		color_map[COLOR_MAP_SIZE];   /* actually intens. map */
	int		pos_col1;	/* position of color 1 in color map */
	int		pos_col2;	/* position of color 2 in color map */
	int		pos_col3;	/* position of color 3 in color map */
} Tex3d;

struct color_3d {   /* similar to soft_color_3d() */
	miScalar	spacing;	/* spacing (density) */
	miScalar	angle;		/* rotation */
	miScalar	strength;	/* amount of noise */
	miScalar	power;		/* deformation strength */
	miInteger	iteration;	/* polynomial order */
	miScalar	weight1;	/* weights of middle colors 1,2,3 */
	miScalar	weight2;
	miScalar	weight3;
	miScalar	intens0;		/* 5-intensity color spread */
	miScalar	intens1;
	miScalar	intens2;
	miScalar	intens3;
	miScalar	intens4;
};

typedef struct {	/* help structure for storing thread-local finest levels */
	int curr;
	int last;
} Finest_level;

/*
 * Local variables
 */

static float	*table;
static miColor	black = {0, 0, 0, 1};


inline void tls_get(
	miState		*state,
	Finest_level	**fl)
{
	mi_query(miQ_FUNC_TLS_GET, state, miNULLTAG, fl);
	if (!(*fl)) {
		(*fl) = (Finest_level*) mi_mem_allocate(sizeof(Finest_level));
		mi_query(miQ_FUNC_TLS_SET, state, miNULLTAG, fl);
	}
}

/*
 * Prototypes
 */

static float getnoise(float, float, float);
static float lookup_color(miScalar *, float);
static miBoolean color_3d_init(miState *, struct color_3d *, miBoolean *);
static float color_3d(miState *);


/*
 * Helper functions
 */

inline miBoolean color_contrast(
	miState		*state,
	float		step,
	miColor		*color1,
	miColor		*color2)
{
	step *= 10;
	const miColor* c = &state->options->contrast;
	return(step * fabs(color1->r - color2->r) > c->r ||
	       step * fabs(color1->g - color2->g) > c->g ||
	       step * fabs(color1->b - color2->b) > c->b);
}


/*
 * This function must be called before the first call of density_func_cloud()
 * It is called from both parti_volume_init() and parti_volume_photon_init(),
 * but should only init once, hence the 'first' check.
 */

static void cloud_init(
	miState		*state)
{
	miBoolean	inst_req;
	struct color_3d	texparas;   /* for cloud volume texture */
	static miBoolean first = miTRUE;

	mi_lock(state->global_lock);
	texparas.spacing   = 1.0;
	texparas.angle	   = 0.0;
	texparas.strength  = 1.0;
	texparas.power	   = 1.0;
	texparas.iteration = 5;
	texparas.weight1   = 0.5;
	texparas.weight2   = 0.5;
	texparas.weight3   = 0.5;
	texparas.intens0   = 1.0;
	texparas.intens1   = 0.5;
	texparas.intens2   = 0.5;
	texparas.intens3   = 0.0;
	texparas.intens4   = 0.0;
	if (first)
		color_3d_init(state, NULL, &inst_req);
	color_3d_init(state, &texparas, &inst_req);

	mi_debug("cloud volume texture initialized");
	first = miFALSE;
	mi_unlock(state->global_lock);
}


/*
 * 3D Perlin noise texture looks cloud-like
 */

static float density_func_cloud(
	miState			*state,
	miVector		*point,
	struct parti_volume	*paras)
{
	miVector		tmppoint = *point;
	float			texintens;

	tmppoint = state->point;
	state->point    = *point;
	state->point.x *= 1.5;
	state->point.x -= 0.5;
	state->point.y *= -1.5;
	state->point.z *= 1.5;
	texintens = color_3d(state);
	state->point = tmppoint;
	miASSERT(0.0 <= texintens && texintens <= 1.0);
	return 2.0F * paras->extinction * texintens;
}


/*
 * The mother of all density functions :)
 */

static float density_func(
	miState			*state,
	struct parti_volume	*paras,
	float			d)
{
	miVector		point;
	float			uniform;

	if (paras->mode==1 && state->org.y + d * state->dir.y > paras->height)
		return(0.0);		/* mode 1, above 'height': no fog */

	if (paras->nonuniform == 0.0)	/* homogeneous medium */
		return(paras->extinction);
	else {				/* nonhomogeneous medium */
		miASSERT(0.0 <= paras->nonuniform && paras->nonuniform <= 1.0);
		uniform = 1.0F - paras->nonuniform;
		/* Compute position along the ray */
		point.x = state->org.x + d * state->dir.x;
		point.y = state->org.y + d * state->dir.y;
		point.z = state->org.z + d * state->dir.z;
		return(uniform * paras->extinction + paras->nonuniform *
				density_func_cloud(state, &point, paras));
	}
}


/*
 * integrate extinction along path from light to point
 */

static float density_integral(
	miState			*state,
	struct parti_volume	*paras,
	float			d)
{
	float			s;	/* distance within dense part of vol */
	float			ks;
	float			step = paras->max_step_len;
	double                  jitter; /* store jitter here */

	miASSERT(paras->mode == 0 || paras->mode == 1);
	if (paras->mode == 0) {		/* mode 0: fog everywhere in volume */
		if (paras->nonuniform == 0.0)	/* homogeneous medium */
			return paras->extinction * (float)state->dist;
		else {				/* nonhomogeneous medium */
			ks = 0;
			mi_sample(&jitter, NULL, state, 1, NULL);
			d -= step * (float)jitter;
			while (d > 0) {
				/* Compute density at point along the ray */
				ks += density_func(state, paras, d) * step;
				d -= step;
			}
			miASSERT(ks >= 0.0);
			return ks;
		}
	} else {			/* mode 1: fog below 'height' */
		s = (state->org.y + d * state->dir.y - paras->height) /
								state->dir.y;
		return (s > 0.0) ? paras->extinction * s : 0.0F;
	}
}



/*
 * Volume shaders for participating media: Direct illumination
 */

static void direct_illum(
	miColor			*result,
	miState			*state,
	struct parti_volume	*paras)
{
	const float		g1 = paras->g1;
	const float		g2 = paras->g2;
	const float		r = paras->r;
	const float		ldist = paras->light_dist;
	miBoolean		fast;

	state->pri = 0;			/* avoids rejecting illumination when*/
					/* state->normal faces the wrong way */
	*result = black;

	for (mi::shader::LightIterator iter(state, paras->light, 
				paras->n_light); !iter.at_end(); ++iter){
		miColor sum   = black;
		miVector origin;	/* light source origin */

		mi_query(miQ_LIGHT_ORIGIN, state, *iter, &origin);

		fast = state->reflection_level + state->refraction_level > 1 ||
		       fabs(state->point.x - origin.x) > ldist ||
		       fabs(state->point.y - origin.y) > ldist ||
		       fabs(state->point.z - origin.z) > ldist;
		if (fast)		/* ensure coarse area light sampling */
			state->reflection_level += 3;

		/*
		 * Sample the light source. The number of samples can be
		 * specified in the mi file (default 3 x 3). We don't need
		 * dir or dot_nl.
		 */
		while (iter->sample()) {
			miColor	color;		/* color from light source */
			iter->get_contribution(&color);
			if (g1 == 0.0 && g2 == 0.0) {
				sum.r += color.r;
				sum.g += color.g;
				sum.b += color.b;
			} else {
				float f;
				miVector dir = iter->get_direction();
				if (r == 1.0f)
					f = r * mi_schlick_scatter(
							&state->dir, &dir, g1);
				else if (r == 0.0)
					f = mi_schlick_scatter(&state->dir,
								&dir, g2);
				else
					f =    r  * mi_schlick_scatter(
						       &state->dir, &dir, g1) +
					    (1-r) * mi_schlick_scatter(
						       &state->dir, &dir, g2);
				sum.r += f * color.r;
				sum.g += f * color.g;
				sum.b += f * color.b;
			}
		}
		int samples = iter->get_number_of_samples();
		if (samples) {
			result->r += sum.r / samples;
			result->g += sum.g / samples;
			result->b += sum.b / samples;
		}
		if (fast)
			state->reflection_level -= 3;   /* reset refl level */
	}
	miASSERT(!isnan(result->r) && !isnan(result->g) && !isnan(result->b));
}


static void compute_radiance(
	miColor			*result,
	miState			*state,
	struct parti_volume	*paras,
	float			k,		/* extinction coefficient */
	float			d)		/* distance along the ray */
{
	miColor			illum, irrad;
	miColor			scatter = paras->scatter;
	const miScalar		r  = paras->r;
	const miScalar		g1 = paras->g1;
	const miScalar		g2 = paras->g2;

	if (k != paras->extinction) {		/* modify scattering coeffs */
		float f = k / paras->extinction;
		scatter.r *= f;
		scatter.g *= f;
		scatter.b *= f;
	}
	/* Compute position along the ray */
	state->point.x = state->org.x + d * state->dir.x;
	state->point.y = state->org.y + d * state->dir.y;
	state->point.z = state->org.z + d * state->dir.z;

	/* Compute direct illumination */
	direct_illum(&illum, state, paras);
	result->r = 1.F/(4.F*M_PI_F) * scatter.r * illum.r;
	result->g = 1.F/(4.F*M_PI_F) * scatter.g * illum.g;
	result->b = 1.F/(4.F*M_PI_F) * scatter.b * illum.b;

	/* Optimization: some scenes have no caustic where direct light */
	if (paras->no_globil_where_direct && result->r > 0.0)
		return;

	/* Compute indirect illumination (albedo = scatter / k) */
	if (paras->g1 == 0.0 && paras->g2 == 0.0)
		mi_compute_volume_irradiance(&irrad, state);
	else
		mi_compute_directional_irradiance(&irrad, state, r, g1, g2);
	miASSERT(k > 0.0);

	result->r += 1.F/(4.F*M_PI_F) * scatter.r / k * irrad.r;
	result->g += 1.F/(4.F*M_PI_F) * scatter.g / k * irrad.g;
	result->b += 1.F/(4.F*M_PI_F) * scatter.b / k * irrad.b;

	miASSERT(!isnan(result->r) && !isnan(result->g) && !isnan(result->b));
}


/*
 * Compute direct and indirect illumination with adaptive raymarching,
 * attenuate result --- avoiding sampling to compute direct illumination
 * at each point.
 */

static void attenuate_radiance_nosampling(
	miColor			*result,
	miState			*state,
	struct parti_volume	*paras,
	float			d,
	float			step,
	int			level,
	miColor			*frontcolor,
	miColor			*backcolor,
	Finest_level		*fl)
{
	miColor			midcolor;
	float			k = paras->extinction;
	float			t;		/* transmittance */

	miASSERT(step > 0);

	if (level > fl->curr)
		fl->curr = level;

	/* Recurse further if necessary */
	if (state->reflection_level <= 1 &&
	    step > paras->min_step_len &&
	    (color_contrast(state, step, frontcolor, backcolor) ||
	    level < fl->last - 1)) {

		step *= 0.5;
		compute_radiance(&midcolor, state, paras, k, d+step);
		attenuate_radiance_nosampling(result, state, paras, d+step,
				step, level+1, &midcolor, backcolor, fl);
		attenuate_radiance_nosampling(result, state, paras, d,
				step, level+1, frontcolor, &midcolor, fl);
	} else {
		/*
		 * no recursion needed. Attenuate distant radiance and add the
		 * radiance. Potential optimization: move *scatter/4pi here.
		 */
		t = exp(-k * step);	/* transmittance */
		miASSERT(0 <= t && t <= 1);

		result->r = frontcolor->r * step + t * result->r;
		result->g = frontcolor->g * step + t * result->g;
		result->b = frontcolor->b * step + t * result->b;
	}
}


/*
 * Compute direct and indirect illumination with adaptive raymarching,
 * attenuate result --- sample to compute direct illumination at each
 * point along ray
 */

static void attenuate_radiance_sampling(
	miColor			*result,
	miState			*state,
	struct parti_volume	*paras,
	float			k,		/* extinction coeff at front */
	float			d,
	float			step,
	int			level,
	miColor			*frontcolor,
	miColor			*backcolor,
	Finest_level		*fl)
{
	miColor			midcolor;
	float			k_mid;		/* extinction coeff in middle*/
	float			t;		/* transmittance */

	miASSERT(step > 0);

	if (level > fl->curr)
		fl->curr = level;

	/* Recurse further if necessary */
	if (state->reflection_level + state->refraction_level <= 1 && /*fast*/
	    step > paras->min_step_len &&
	    (color_contrast(state, step, frontcolor, backcolor) ||
	    level < fl->last - 1)) {	/* recurse */

		step *= 0.5;
		k_mid = density_func(state, paras, d+step);	/* extinction*/
		if (k_mid > 0.0) {
			compute_radiance(&midcolor, state, paras,k_mid,d+step);
			attenuate_radiance_sampling(result, state, paras,
						k_mid, d+step, step, level+1,
						&midcolor, backcolor, fl);
			attenuate_radiance_sampling(result, state, paras,
						k, d, step, level+1,
						frontcolor, &midcolor, fl);
		} else
			attenuate_radiance_sampling(result, state, paras,
						k, d, step, level+1,
						frontcolor, backcolor, fl);
	} else {
		/*
		 * no recursion needed. Attenuate distant radiance and add the
		 * radiance. Potential optimization: move *scatter/4pi here.
		 */
		t = exp(-k * step);			/* transmittance */
		miASSERT(0 <= t && t <= 1);
		result->r = frontcolor->r * step + t * result->r;
		result->g = frontcolor->g * step + t * result->g;
		result->b = frontcolor->b * step + t * result->b;
	}
}



/*
 * Volume shaders for nonuniform-density participating media
 */

extern "C" DLLEXPORT int parti_volume_version(void) {return(3);}

extern "C" DLLEXPORT miBoolean parti_volume_init(
	miState		*state,		/* state (unused) */
	Paras		*paras,		/* parameters */
	miBoolean	*inst_req)	/* shader inst requests */
{
	if (paras == NULL)
		*inst_req = miTRUE;
	else if (*mi_eval_scalar(&paras->nonuniform) > 0.0)
		cloud_init(state);
	return(miTRUE);
}


extern "C" DLLEXPORT miBoolean parti_volume_exit(
	miState		*state,		/* state (unused) */
	Paras		*paras,		/* parameters */
	miBoolean	*inst_req)	/* shader inst requests */
{
	if (paras) {
		Tex3d **texp;
		mi_query(miQ_FUNC_USERPTR, state, 0, &texp);
		mi_mem_release(*texp);
		*texp = 0;
		{
			Finest_level	**fls;
			int		n_fls;
			int 		i;
			mi_query(miQ_FUNC_TLS_GETALL, state, miNULLTAG, 
				&fls, &n_fls);
			for (i=0; i<n_fls; i++)
				mi_mem_release(fls[i]);
		}
		
	}
	return(miTRUE);
}


extern "C" DLLEXPORT miBoolean parti_volume(
	miColor			*result,
	miState			*state,
	Paras			*parms)
{
	struct parti_volume	p, *paras = &p;	/* mi_eval'ed parameters */
	miColor			front, back;	/* radiance (direct and */
						/* indirect) at ends of intrv*/
	miVector		point;
	float			t;		/* transmittance */
	float			d;		/* distance */
	float			k;		/* extinction coefficient */
	float			int_ks;		/* integral of k*s */
	float			step;
	miBoolean		uniform;
        double                  jitter;         /* store jitter here */
	Finest_level		*fl;		/* thread-local finest levels */

	if (state->type == miRAY_SHADOW)
		return(miTRUE);			/* common case: ~50% */

	tls_get(state, &fl);

	p.mode		= *mi_eval_integer(&parms->mode);
	p.scatter	= *mi_eval_color  (&parms->scatter);
	p.extinction	= *mi_eval_scalar (&parms->extinction);
	p.r		= *mi_eval_scalar (&parms->r);
	p.g1		= *mi_eval_scalar (&parms->g1);
	p.g2		= *mi_eval_scalar (&parms->g2);
	p.nonuniform	= *mi_eval_scalar (&parms->nonuniform);
	p.height	= *mi_eval_scalar (&parms->height);
	p.min_step_len	= *mi_eval_scalar (&parms->min_step_len);
	p.max_step_len	= *mi_eval_scalar (&parms->max_step_len);
	p.light_dist	= *mi_eval_scalar (&parms->light_dist);
	p.min_level	= *mi_eval_integer(&parms->min_level);
	p.no_globil_where_direct =
			  *mi_eval_boolean(&parms->no_globil_where_direct);
	p.i_light	= *mi_eval_integer(&parms->i_light);
	p.n_light	= *mi_eval_integer(&parms->n_light);
	p.light		= mi_eval_tag(&parms->light) + p.i_light;
	/*
	 * This is a real time-sink for area: light sources since it gets
	 * called for every light ray
	 */
	if (state->type == miRAY_LIGHT) {
		int_ks = density_integral(state, paras, state->dist);
		/* Compute transmittance and multiply result by it */
		if (int_ks > 0.0) {
			t = exp(-int_ks);
			result->r *= t;
			result->g *= t;
			result->b *= t;
		}
		miASSERT(!isnan(result->r) && !isnan(result->g)
					   && !isnan(result->b));
		return(miTRUE);
	}
	if (state->dist == 0.0)		/* infinite dist: outside volume */
		return(miTRUE);

	point = state->point;		/* save state->point */

	/*
	 * Do ray marching
	 */

	step = paras->max_step_len;
	mi_sample(&jitter, NULL, state, 1, NULL);

	k = density_func(state, paras, state->dist-miEPS);	/* extinction*/
	if (k > 0.0) {
		float jitterf = (float)jitter;
		compute_radiance(&back, state, paras, k, state->dist-miEPS);
		result->r += back.r * step * jitterf;
		result->g += back.g * step * jitterf;
		result->b += back.b * step * jitterf;
	} else
		back = black;

	fl->curr = 2;	/* start out conservatively */
	fl->last = 0;
	uniform = paras->nonuniform == 0.0;

	for (d = state->dist - step * jitter; d > 0.0; d -= step) {
		k = density_func(state, paras, d);		/* extinction*/

		/* to speed up things for 0 extinction and scatter */
		if (k > 0.0) {
			/* Compute direct and indirect illumination */
			compute_radiance(&front, state, paras, k, d);
			/* Adaptively subdivide interval where necessary */
			if (uniform)
				attenuate_radiance_nosampling(result, state,
					paras, d, step, 0, 
					&front, &back, fl);
			else
				attenuate_radiance_sampling(result, state,
					paras, k, d, step, 0, 
					&front, &back, fl);
			back = front;
			fl->last = fl->curr;
			fl->curr = 0;
		} else
			back = black;

		miASSERT(0.0 <= result->r && result->r < 100000);
	}

	/*
	 * Frontmost step
	 */
	step = d + step - 2.F * miEPSF;
	if (step > 0.0) {
		k = density_func(state, paras, miEPSF);
		if (k > 0.0) {
			compute_radiance(&front, state, paras, k, miEPSF);
			if (uniform)
				attenuate_radiance_nosampling(result, state,
					paras, miEPSF, step, 0, 
					&front, &back, fl);
			else
				attenuate_radiance_sampling(result, state,
					paras,k,miEPSF, step, 0, 
					&front, &back, fl);
		}
	}
	state->point = point;			/* restore state->point */
	miASSERT(!isnan(result->r) && !isnan(result->g) &&
		 !isnan(result->b) && !isnan(result->a));
	return(miTRUE);
}


extern "C" DLLEXPORT miBoolean parti_volume_photon_init(
	miState		*state,		/* state (unused) */
	Paras		*paras,		/* parameters */
	miBoolean	*inst_req)	/* shader inst requests */
{
	if (paras == NULL)
		*inst_req = miTRUE;
	else if (*mi_eval_scalar(&paras->nonuniform) > 0.0)
		cloud_init(state);
	return(miTRUE);
}


/*
 * It is essential to use mi_photon_path_length(), mi_choose_lobe(),
 * mi_sample() and and mi_scattering_dir*() to get consistent sampling numbers
 * for animations. Consistency is important because otherwise a small change in
 * geometry will give very different sampling numbers after the first photon
 * that hit something different than in the other frame.  This means that
 * most photon positions are completely uncorrelated from frame to frame.
 * Avoiding mi_par_random() and friends eliminates this problem.
 */

extern "C" DLLEXPORT int parti_volume_photon_version(void) {return(3);}

extern "C" DLLEXPORT miBoolean parti_volume_photon(
	miColor			*energy,
	miState			*state,
	Paras			*parms)
{
	struct parti_volume	p, *paras = &p;	/* mi_eval'ed parameters */
	miColor			new_energy;
	miVector		dir;
	float			dx, d, step;
	float			k;		/* extinction coefficient */
	miBoolean		scattered = miFALSE;
	double                  jitter;         /* store juiiter here */


	if (state->dist == 0.0)
		return(miTRUE);				/* leaving volume */

	p.mode		= *mi_eval_integer(&parms->mode);
	p.scatter	= *mi_eval_color  (&parms->scatter);
	p.extinction	= *mi_eval_scalar (&parms->extinction);
	p.r		= *mi_eval_scalar (&parms->r);
	p.g1		= *mi_eval_scalar (&parms->g1);
	p.g2		= *mi_eval_scalar (&parms->g2);
	p.nonuniform	= *mi_eval_scalar (&parms->nonuniform);
	p.height	= *mi_eval_scalar (&parms->height);
	p.min_step_len	= *mi_eval_scalar (&parms->min_step_len);
	p.max_step_len	= *mi_eval_scalar (&parms->max_step_len);
	p.light_dist	= *mi_eval_scalar (&parms->light_dist);
	p.min_level	= *mi_eval_integer(&parms->min_level);
	p.no_globil_where_direct =
			  *mi_eval_boolean(&parms->no_globil_where_direct);
	p.i_light 	= *mi_eval_integer(&parms->i_light);
	p.n_light	= *mi_eval_integer(&parms->n_light);
	p.light		= mi_eval_tag(&parms->light) + p.i_light;

	if (miODD(state->refraction_level) &&
	    state->refraction_level >= paras->min_level) {

		if (paras->mode == 0 && paras->nonuniform == 0.0) {
			step = (float)(state->dist + miEPS);
			d = 0.0;
		} else {
			step = paras->max_step_len;
			mi_sample(&jitter, NULL, state, 1, NULL);
			d = step * (float)jitter;
		}
		while (d < state->dist && !scattered) {

			/* Compute density at point along the photon path */
			k = density_func(state, paras, d);

			if ((dx = mi_scattering_pathlength(state, k)) < step &&
			    d + dx < state->dist) {	/* scattering in vol */

				/* Compute new point along the ray */
				miASSERT(dx >= 0.0);
				d += dx;
				state->point.x = state->org.x + d*state->dir.x;
				state->point.y = state->org.y + d*state->dir.y;
				state->point.z = state->org.z + d*state->dir.z;

				/* Store photons (except direct photons) */
				if (!(state->type==miPHOTON_TRANSPARENT&&
				    state->parent->type == miPHOTON_LIGHT))
					mi_store_volume_photon(energy, state);

				/* Compute energy of the new photon */
				new_energy.r = paras->scatter.r * energy->r;
				new_energy.g = paras->scatter.g * energy->g;
				new_energy.b = paras->scatter.b * energy->b;

				/* Scatter photon in diffuse direction */
				if (paras->g1 == 0.0 && paras->g2 == 0.0)
					mi_scattering_dir_diffuse(&dir, state);
				else if (mi_choose_lobe(state, paras->r) == 1)
					/* first lobe */
					mi_scattering_dir_directional(&dir,
							state, paras->g1);
				else	/* second lobe */
					mi_scattering_dir_directional(&dir,
							state, paras->g2);
				mi_photon_volume_scattering(&new_energy,
								state, &dir);
				scattered = miTRUE;
			} else			/* no scattering in volume */
				d += step;
		}
	}
	if (!scattered) {   /* no scattering in volume */
		/* Call photon material shader */
		mi_call_photon_material(energy, state);
	}
	return(miTRUE);
}


/*
 * Transparent materials for 'dummy' volume surfaces.
 * The material shader lets rays and shadow rays pass.
 * The photon shader lets photons pass.
 */

extern "C" DLLEXPORT int transmat_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean transmat(
	miColor		*result,
	miState		*state,
	void		*paras)		/* no parameters */
{
	if (state->type == miRAY_SHADOW) 
		return(miTRUE);
	else			/* Trace ray further in same direction */
		return(mi_trace_transparent(result, state));
}


extern "C" DLLEXPORT int transmat_photon_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean transmat_photon(
	miColor		*energy,
	miState		*state,
	void		*paras)		/* no parameters */
{
	/* Shoot photon further in same direction */
	return(mi_photon_transparent(energy, state));
}


/*
 * Initialize 3D texture shading. This is called before rendering begins.
 * If this is a shader init at the beginning of a frame, request instance
 * inits and initialize the random-number lattice for the coherent noise
 * function. All shaders share the noise table.
 * If this is an instance init, initialize the context struct used for 3D
 * texture evaluation that avoids having to recompute this for every shader
 * call.
 */

static miBoolean color_3d_init(
	miState		*state,
	struct color_3d	*paras,
	miBoolean	*inst_req)
{
	register int	i, j, k;
	unsigned short seed[3];

	if (!paras) {					/* shader init */
		*inst_req = miTRUE;
		table = (float *)mi_mem_allocate(TABLE_SIZE * sizeof(float));

		/* initialize seed: we want reproducable results */
		seed[0] = 0x330e;
		seed[1] = 1;
		seed[2] = 0;
		for (i=0; i < TABLE_SIZE; i++)
			table[i] = mi_erandom(seed);
	} else {					/* instance init */
		Tex3d **texp, *tex;
		mi_query(miQ_FUNC_USERPTR, state, 0, &texp);
		tex = *texp = (Tex3d*) mi_mem_allocate(sizeof(Tex3d));

		/*
		 * step 1: create and initialize the Tex3d context struct
		 */
		{
		float		weight = 0.5F;
		float		scale  = 1.0F;
		float		tw     = 0.0F;
		tex->spacing  = 2 * 3.141592653589793116 * paras->spacing;
		tex->taper    = ftan(paras->angle);
		tex->strength = paras->strength;
		tex->no_terms = paras->iteration < NTERMS ? paras->iteration
							  : NTERMS;
		for (i=0; i < paras->iteration; i++) {
			tex->terms[i].weight = weight;
			tex->terms[i].scale  = scale;
			tw    += weight;
			scale += scale;
			weight = (float)(0.5 * pow(scale, -paras->power));
		}

		for (i=0; i < tex->no_terms; i++)
			tex->terms[i].weight /= tw;

		}

		/*
		 * step 2: calculate color spread
		 */
		{
		miScalar col[5];
		int	pos_col[5];
		int	delta;
		miScalar step;
		miScalar c;
		miScalar *ptr_col_map = tex->color_map;

		col[0] = paras->intens0;
		col[1] = paras->intens1;
		col[2] = paras->intens2;
		col[3] = paras->intens3;
		col[4] = paras->intens4;

		pos_col[0] = 0;
		pos_col[1] = tex->pos_col1 = paras->weight1 * COLOR_MAP_MAX;
		pos_col[2] = tex->pos_col2 = paras->weight2 * COLOR_MAP_MAX;
		pos_col[3] = tex->pos_col3 = paras->weight3 * COLOR_MAP_MAX;
		pos_col[4] = COLOR_MAP_MAX;

		for (i=0, j=1; j < 5; i++, j++) {
			if ((delta = pos_col[j] - pos_col[i]) > 0) {
				step = (col[j] - col[i]) / delta;
				c = col[i];
				for (k=pos_col[i]; k < pos_col[j]; k++) {
					*ptr_col_map++ = c;
					c += step;
				}
				*ptr_col_map = col[j];
			} else
				ptr_col_map += delta;
		}
		}
	}
	return(miTRUE);
}


/*
 * Cloud turbulence function similar to soft_color_3d() in CLOUD mode
 */

static float color_3d(
	miState		*state)
{
	    miVector        point = state->point;
	float		sk, sum=0;
	int		i;
	int             no_terms;
	Tex3d		*tex, **texp;

	mi_query(miQ_FUNC_USERPTR, state, 0, &texp);
	tex = *texp;
	miASSERT(tex);
	if (state->type == miRAY_LIGHT ||
	    (!miRAY_PHOTON(state->type) && /* some ray type, not a photon */
	     state->reflection_level + state->refraction_level > 3))
		no_terms = 1;
	else
		no_terms = tex->no_terms;

	for (i = 0; i < no_terms; i++) {
		sk   = tex->terms[i].scale;
		sum += tex->terms[i].weight * getnoise(sk * point.x,
						       sk * point.y,
						       sk * point.z);
	}
	if (sum > 1.0)
		sum = 1.0F;
	return(lookup_color(tex->color_map, sum * sum * (3.0F - sum - sum)));
}


/*
 * Do a linear interpolation to get a color map value.
 * Limitations:	the color position must be between 0.0 and 1.0.
 */

static float lookup_color(
	register miScalar *color_map,
	float		pos)
{
	register float	p, lfr, hfr;
	register int	l, h;

	l   = p = pos * COLOR_MAP_MAX;
	hfr = 1.0F - (lfr = p - l);
	h   = l == COLOR_MAP_MAX ? COLOR_MAP_MAX : l+1;

	return hfr * color_map[l] + lfr * color_map[h];
}



/*
 * Compute a band-limited noise value for an (x,y,z) point.
 */

static float getnoise(
	float	x,
	float	y,
	float	z)
{
	float	xx, yy, zz;
	int	ix, iy, iz, ixyz;
	float	fr_x, fr_y, fr_z;
	float	u_0_0, u_0_1, u_0_2;
	float	u_1_0, u_1_1, u_1_2;
	float	u_2_0, u_2_1, u_2_2;
	float	dxyz_0_0_0, dxyz_0_0_1, dxyz_0_0_2;
	float	dxyz_0_1_0, dxyz_0_1_1, dxyz_0_1_2;
	float	dxyz_0_2_0, dxyz_0_2_1, dxyz_0_2_2;
	float	dxyz_1_0_0, dxyz_1_0_1, dxyz_1_0_2;
	float	dxyz_1_1_0, dxyz_1_1_1, dxyz_1_1_2;
	float	dxyz_1_2_0, dxyz_1_2_1, dxyz_1_2_2;
	float	dxyz_2_0_0, dxyz_2_0_1, dxyz_2_0_2;
	float	dxyz_2_1_0, dxyz_2_1_1, dxyz_2_1_2;
	float	dxyz_2_2_0, dxyz_2_2_1, dxyz_2_2_2;

	xx = x + 39999.5F; ix = (int)xx;
	yy = y + 39999.5F; iy = (int)yy;
	zz = z + 39999.5F; iz = (int)zz;

	fr_x = xx - ix;
	fr_y = yy - iy;
	fr_z = zz - iz;

	u_2_0 = 0.5F * fr_x * fr_x;
	u_0_0 = 0.5F - fr_x + u_2_0;
	u_1_0 = 1.0F - u_0_0 - u_2_0;
	u_2_1 = 0.5F * fr_y * fr_y;
	u_0_1 = 0.5F - fr_y + u_2_1;
	u_1_1 = 1.0F - u_0_1 - u_2_1;
	u_2_2 = 0.5F * fr_z * fr_z;
	u_0_2 = 0.5F - fr_z + u_2_2;
	u_1_2 = 1.0F - u_0_2 - u_2_2;

	ixyz = ix * 1341 + iy * 719 + iz * 2031;

	dxyz_0_0_0 = table[(ixyz)        & TABLE_MAX];
	dxyz_0_0_1 = table[(ixyz + 1341) & TABLE_MAX];
	dxyz_0_0_2 = table[(ixyz + 2682) & TABLE_MAX];
	dxyz_0_1_0 = table[(ixyz +  719) & TABLE_MAX];
	dxyz_0_1_1 = table[(ixyz + 2060) & TABLE_MAX];
	dxyz_0_1_2 = table[(ixyz + 3401) & TABLE_MAX];
	dxyz_0_2_0 = table[(ixyz + 1438) & TABLE_MAX];
	dxyz_0_2_1 = table[(ixyz + 2779) & TABLE_MAX];
	dxyz_0_2_2 = table[(ixyz + 4120) & TABLE_MAX];
	dxyz_1_0_0 = table[(ixyz + 2031) & TABLE_MAX];
	dxyz_1_0_1 = table[(ixyz + 3372) & TABLE_MAX];
	dxyz_1_0_2 = table[(ixyz + 4713) & TABLE_MAX];
	dxyz_1_1_0 = table[(ixyz + 2750) & TABLE_MAX];
	dxyz_1_1_1 = table[(ixyz + 4091) & TABLE_MAX];
	dxyz_1_1_2 = table[(ixyz + 5432) & TABLE_MAX];
	dxyz_1_2_0 = table[(ixyz + 3469) & TABLE_MAX];
	dxyz_1_2_1 = table[(ixyz + 4810) & TABLE_MAX];
	dxyz_1_2_2 = table[(ixyz + 6151) & TABLE_MAX];
	dxyz_2_0_0 = table[(ixyz + 4062) & TABLE_MAX];
	dxyz_2_0_1 = table[(ixyz + 5403) & TABLE_MAX];
	dxyz_2_0_2 = table[(ixyz + 6744) & TABLE_MAX];
	dxyz_2_1_0 = table[(ixyz + 4781) & TABLE_MAX];
	dxyz_2_1_1 = table[(ixyz + 6122) & TABLE_MAX];
	dxyz_2_1_2 = table[(ixyz + 7463) & TABLE_MAX];
	dxyz_2_2_0 = table[(ixyz + 5500) & TABLE_MAX];
	dxyz_2_2_1 = table[(ixyz + 6841) & TABLE_MAX];
	dxyz_2_2_2 = table[(ixyz + 8182) & TABLE_MAX];

	return(u_0_2 * (u_0_1 * (u_0_0 * dxyz_0_0_0
			       + u_1_0 * dxyz_0_0_1
			       + u_2_0 * dxyz_0_0_2)
		      + u_1_1 * (u_0_0 * dxyz_0_1_0
			       + u_1_0 * dxyz_0_1_1
			       + u_2_0 * dxyz_0_1_2)
		      + u_2_1 * (u_0_0 * dxyz_0_2_0
			       + u_1_0 * dxyz_0_2_1
			       + u_2_0 * dxyz_0_2_2))
	     + u_1_2 * (u_0_1 * (u_0_0 * dxyz_1_0_0
			       + u_1_0 * dxyz_1_0_1
			       + u_2_0 * dxyz_1_0_2)
		      + u_1_1 * (u_0_0 * dxyz_1_1_0
			       + u_1_0 * dxyz_1_1_1
			       + u_2_0 * dxyz_1_1_2)
		      + u_2_1 * (u_0_0 * dxyz_1_2_0
			       + u_1_0 * dxyz_1_2_1
			       + u_2_0 * dxyz_1_2_2))
	     + u_2_2 * (u_0_1 * (u_0_0 * dxyz_2_0_0
			       + u_1_0 * dxyz_2_0_1
			       + u_2_0 * dxyz_2_0_2)
		      + u_1_1 * (u_0_0 * dxyz_2_1_0
			       + u_1_0 * dxyz_2_1_1
			       + u_2_0 * dxyz_2_1_2)
		      + u_2_1 * (u_0_0 * dxyz_2_2_0
			       + u_1_0 * dxyz_2_2_1
			       + u_2_0 * dxyz_2_2_2)));
}
