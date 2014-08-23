/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	16.1.96
 * Module:	contour
 * Purpose:	standard contour shaders: store, contrast, contour
 *
 * Exports:
 *	contour_store_function_version
 *	contour_store_function
 *	contour_store_function_simple_version
 *	contour_store_function_simple
 *
 *	contour_contrast_function_levels_version
 *	contour_contrast_function_levels
 *	contour_contrast_function_simple_version
 *	contour_contrast_function_simple
 *
 *	contour_shader_simple_version
 *	contour_shader_simple
 *	contour_shader_depthfade_version
 *	contour_shader_depthfade
 *	contour_shader_framefade_version
 *	contour_shader_framefade
 *	contour_shader_layerthinner_version
 *	contour_shader_layerthinner
 *	contour_shader_curvature_version
 *	contour_shader_curvature
 *	contour_shader_factorcolor_version
 *	contour_shader_factorcolor
 *	contour_shader_widthfromcolor_version
 *	contour_shader_widthfromcolor
 *	contour_shader_widthfromlightdir_version
 *	contour_shader_widthfromlightdir
 *	contour_shader_widthfromlight_version
 *	contour_shader_widthfromlight
 *	contour_shader_combi_version
 *	contour_shader_combi
 *	contour_shader_maxcolor_version
 *	contour_shader_maxcolor
 *	contour_shader_silhouette_version
 *	contour_shader_silhouette
 *
 * History:
 *	14. 9.01: reindented in manual style for publication
 *	27.11.03: added mi_eval_ calls. bumped up version numbers.
 *
 * Description:
 * The contour store function stores information (from the state and
 * material color) needed to compute the location, color, and width of
 * the contours.  The contour contrast function gets two such infos,
 * and decides whether there should be a contour there.  Each material
 * can have a contour shader, and this computes color and width of the
 * contour based on two infos (from neighboring points).
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <mi_contourshade.h>
#include <mi_version.h>

#define miEPS 0.0001

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

#define mi_MAX(a,b)  ((a) > (b) ? (a) : (b))

char id[] = "==@@== compiled for ray " MI_VERSION_STRING " on " MI_DATE_STRING;

char *mi_contour_shader_version(void)
		{return("=@@= contour.so:\tversion 1.2" + 5);}


/*------------------------------------------------ contour store --------------
 *
 * Contour store functions.  They store the information needed by the
 * contour shaders.  This information can depend on the state and the
 * color computed by the material shader.
 */

int contour_store_function_version(void) {return(3);}

miBoolean contour_store_function(
	void		*info_void,
	int		*info_size,
	miState		*state,
	miColor		*color)
{
	miState		*s;

	/*
	 * The type of info is unknown to raylib, therefore a void * is passed
	 * and needs to be cast to miStdInfo * here.
	 */
	struct miStdInfo *info = (miStdInfo *) info_void;

	/*
	 * Ray intersection point, normal, material tag, label, triangle index.
	 */
	info->point       = state->point;
	info->normal      = state->normal;
	info->normal_geom = state->normal_geom;
	info->material    = state->material;
	info->label       = state->label;
	info->index       = state->pri_idx;

	/*
	 * Material color at the intersection.  This color is not just the
	 * color of the material (with texture) under a given illumination:
	 * if the material is transparent it also depends on deeper material
	 * colors.
	 */
	info->color = *mi_eval_color(color);

	/*
	 * Compute the refraction level.  Why is state->refraction_level not
	 * what we want?  Because some shaders dont increment it as they
	 * should.
	 */
	info->level = 0;
	for (s=state; s; s=s->parent, info->level++);
	miASSERT(info->level > 0);

	/*
	 * The size of miStdInfo. NOTE: this is obsolete and ignored in
	 * mental ray 3.0 but required for 2.1. In 3.0 the size is derived
	 * from the declaration, which must correctly declare the return type!
	 */
	*info_size = sizeof(miStdInfo);
	return(miTRUE);
}


/*
 * Fast, simple contour store function.  It only stores the material tag ---
 * used with fast, simple contour contrast functions that only needs the
 * material tag.
 */

int contour_store_function_simple_version(void) {return(2);}

miBoolean contour_store_function_simple(
	miTag		*info,
	int		*info_size,
	miState		*state,
	miColor		*color)
{
	*info = state->material;
	*info_size = sizeof(miTag);	/* for 2.1 only, ignored in 3.0 */
	return(miTRUE);
}


/*------------------------------------------------ contour contrast -----------
 *
 * Contour contrast functions.  Based on the information
 * stored by the contour store function for two adjacent samples, they
 * determine whether there should be a contour between them or not.
 */

int contour_contrast_function_levels_version(void) {return(3);}

miBoolean contour_contrast_function_levels(
	miStdInfo	*info1,
	miStdInfo	*info2,
	int		level,
	miState		*state,
	Contour_Contrast_Parameters_Levels *paras)
{

	/*
	 * No contour if level too near or too deep
	 */
	if (level < *mi_eval_integer(&paras->min_level) || 
			*mi_eval_integer(&paras->max_level) < level)
		return(miFALSE);

	/*
	 * Contour if one ray intersected an object and one ray hit background
	 */
	miASSERT(info1 || info2);
	if ((info1 == NULL) != (info2 == NULL))
		return(miTRUE);

	miASSERT(info1 && info2);

	/*
	 * Contour if sufficiently large difference in depth
	 */
	if (fabs(info1->point.z - info2->point.z) > 
				*mi_eval_scalar(&paras->zdelta))
		return(miTRUE);

	/*
	 * Contour if sufficiently large change in normal
	 */
	if (mi_vector_dot(&info1->normal, &info2->normal) <
			cos(*mi_eval_scalar(&paras->ndelta) * M_PI/180.0))
		return(miTRUE);

	/*
	 * Contour if different materials (if specified)
	 */
	if (*mi_eval_boolean(&paras->diff_mat)
				&& info1->material != info2->material)
		return(miTRUE);

	/*
	 * Contour if different object labels (if specified)
	 */
	if (*mi_eval_boolean(&paras->diff_label) 
				&& info1->label != info2->label)
		return(miTRUE);

	/*
	 * Contour if different triangle indices (if specified)
	 */
	if (*mi_eval_boolean(&paras->diff_index) && 
				(info1->index != info2->index ||
				  mi_vector_dot(&info1->normal_geom,
						&info2->normal_geom) < 0.9999))
		return(miTRUE);

	/*
	 * Contour if color contrast (if specified) --- doesn't work properly
	 * on objects behind semitransparent objects since there can be a
	 * contrast on the semistransparent object caused by the color
	 * difference behind it (as a result, it looks like the contrast
	 * shader of the semitransparent object is used on the object behind
	 * even though it isn't).
	 */
	if (*mi_eval_boolean(&paras->contrast) &&
	   (fabs(info1->color.r-info2->color.r) > state->options->contrast.r ||
	    fabs(info1->color.g-info2->color.g) > state->options->contrast.g ||
	    fabs(info1->color.b-info2->color.b) > state->options->contrast.b))
		return(miTRUE);

	/*
	 * No contour otherwise
	 */
	return(miFALSE);
}


/*
 * Used with contour_store_function_simple() for fast, simple contours
 */

int contour_contrast_function_simple_version(void) {return(2);}

miBoolean contour_contrast_function_simple(
	miTag		*info1,
	miTag		*info2,
	int		level,
	miState		*state,
	void		*paras)   /* no parameters */
{
	/*
	 * Contour if one ray intersected an object and one ray hit background
	 */
	if ((info1 == NULL) != (info2 == NULL))
		return(miTRUE);

	/*
	 * Contour if different materials
	 */
	return(*info1 != *info2);
}


/*------------------------------------------------ contour --------------------
 *
 * Contour shaders.  Based on the information stored by
 * the contour store function for two adjacent samples, the contour
 * shader determines what the contour color and width should be.
 * Use fixed color and width of the contour, corresponding to ray 1.9.
 */

int contour_shader_simple_version(void) {return(3);}

miBoolean contour_shader_simple(
	miContour_endpoint *result,
	miStdInfo	   *info_near,
	miStdInfo	   *info_far,
	miState		   *state,
	Simple_Parameters  *paras)
{
	miASSERT(paras->color.r >= 0.0 && paras->color.g >= 0.0);
	miASSERT(paras->color.b >= 0.0 && paras->width >= 0.0);

	/*
	 * Contour color given by a parameter
	 */
	result->color = *mi_eval_color(&paras->color);

	/*
	 * Contour width given by a parameter
	 */
	result->width = *mi_eval_scalar(&paras->width);
	return(miTRUE);
}


/*
 * The width of the contour fades into the background (from near_width
 * to far_width), and the color fades from near_color to far_color
 * The contour width and color changes with a ramp function between
 * distances near_z and far_z
 */

int contour_shader_depthfade_version(void) {return(3);}

miBoolean contour_shader_depthfade(
	miContour_endpoint   *result,
	miStdInfo	     *info_near,
	miStdInfo	     *info_far,
	miState		     *state,
	Depthfade_Parameters *paras)
{
	miScalar	     depth = info_near->point.z;
	double		     near_z, far_z, w_near, w_far;

	/* Ensure that near_z and far_z are negative as they should be */
	near_z = -fabs(*mi_eval_scalar(&paras->near_z));
	far_z  = -fabs(*mi_eval_scalar(&paras->far_z));

	if (depth > near_z) {		/* contour is closer than near_z */
		result->color = *mi_eval_color(&paras->near_color);
		result->width = *mi_eval_scalar(&paras->near_width);
	} else if (depth < far_z) {	/* contour is more distant than far_z*/
		result->color = *mi_eval_color(&paras->far_color);
		result->width = *mi_eval_scalar(&paras->far_width);
	} else {			/* contour is betwn near_z and far_z */
		miColor near_color = *mi_eval_color(&paras->near_color);
		miColor far_color  = *mi_eval_color(&paras->far_color);

		/* Weights w_near and w_far depend on depth */
		w_near = (depth - far_z) / (near_z - far_z);
		miASSERT(0.0 <= w_near && w_near <= 1.0);
		w_far = 1.0 - w_near;

		/* Mix of near_color and far_color according to weights */
		result->color.r = w_near * near_color.r + w_far * far_color.r;
		result->color.g = w_near * near_color.g + w_far * far_color.g;
		result->color.b = w_near * near_color.b + w_far * far_color.b;
		result->color.a = w_near * near_color.a + w_far * far_color.a;
		/* Width depending on weights */
		result->width = w_near * *mi_eval_scalar(&paras->near_width) 
			      + w_far  * *mi_eval_scalar(&paras->far_width);
	}
	return(miTRUE);
}


/*
 * The color and width of the contour depends linearly on the frame
 * number (ramp function).
 */

int contour_shader_framefade_version(void) {return(2);}

miBoolean contour_shader_framefade(
	miContour_endpoint   *result,
	miStdInfo	     *info_near,
	miStdInfo	     *info_far,
	miState		     *state,
	Framefade_Parameters *paras)
{
	int		     frame = state->camera->frame;
	double		     w1, w2;
	int		     frame1, frame2;

	miASSERT(frame >= 0);

	frame1 = *mi_eval_integer(&paras->frame1);
	frame2 = *mi_eval_integer(&paras->frame2);

	miASSERT(frame1 >= 0 && frame2 >= 0);

	if (frame <= frame1) {		/* before frame1 */
		result->color = *mi_eval_color(&paras->color1);
		result->width = *mi_eval_scalar(&paras->width1);
	} else if (frame2 <= frame) {	/* after frame2 */
		result->color = *mi_eval_color(&paras->color2);
		result->width = *mi_eval_scalar(&paras->width2);
	} else {				/* between frame1 and frame2 */
		/* Weights w1 and w2 depend on frame number */
		miColor color1 = *mi_eval_color(&paras->color1);
		miColor color2 = *mi_eval_color(&paras->color2);

		miASSERT(frame1 < frame2);
		w1 = ((double)frame2 - frame) / (frame2 - frame1);
		miASSERT(0.0 <= w1 && w1 <= 1.0);
		w2 = 1.0 - w1;

		/* Mix of color1 and color2 according to weights */
		result->color.r = w1 * color1.r + w2 * color2.r;
		result->color.g = w1 * color1.g + w2 * color2.g;
		result->color.b = w1 * color1.b + w2 * color2.b;
		result->color.a = w1 * color1.a + w2 * color2.a;

		/* Width depending on weights */
		result->width   = w1 * *mi_eval_scalar(&paras->width1)
				+ w2 * *mi_eval_scalar(&paras->width2);
	}
	return(miTRUE);
}


/*
 * The width of the contour changes by a factor each time the ray gets deeper
 */

int contour_shader_layerthinner_version(void) {return(2);}

miBoolean contour_shader_layerthinner(
	miContour_endpoint	*result,
	miStdInfo		*info_near,
	miStdInfo		*info_far,
	miState			*state,
	Layerthinner_Parameters *paras)
{
	/* Constant contour color */
	result->color = *mi_eval_color(&paras->color);

	/* Width decreases by factor for each refraction_level */
	result->width = *mi_eval_scalar(&paras->width) *
			pow( (double)(*mi_eval_scalar(&paras->factor)), 
					(double)info_near->level - 1.0);
	return(miTRUE);
}


/*
 * The width of the contour depends on the curvature, i.e. the
 * difference between the two normals.
 */

int contour_shader_curvature_version(void) {return(2);}

miBoolean contour_shader_curvature(
	miContour_endpoint	  *result,
	miStdInfo		  *info_near,
	miStdInfo		  *info_far,
	miState			  *state,
	Widthfromcolor_Parameters *paras)
{
	double			  d;
	miScalar		  min_width;
	miScalar		  max_width;
	miASSERT(info_near || info_far);

	/* Constant contour color */
	result->color = *mi_eval_color(&paras->color);
	max_width     = *mi_eval_scalar(&paras->max_width);
	min_width     = *mi_eval_scalar(&paras->min_width);

	if ((info_near == NULL) != (info_far == NULL)) {
		/* Max contour width if one point hit background */
		result->width = max_width;

	} else if (fabs(info_near->point.z - info_far->point.z) > 1.0) {
		/* Max contour width if large difference in depth */
		result->width = max_width;
	} else {
		/* Otherwise, the contour width depends on the curvature */
		d = mi_vector_dot(&info_near->normal, &info_far->normal);
		miASSERT(-1.0 <= d && d <= 1.0);
		result->width = min_width + 
				0.5 * (1.0 - d) * (max_width-min_width);
	}
	miASSERT(min_width <= result->width && result->width <= max_width);
	return(miTRUE);
}


/*
 * The color of the contour is a factor times the material color.
 * factor = 0 gives black contours,
 * 0 < factor < 1 gives dark contours,
 * factor = 1 gives contours the same color as the object material,
 * factor > 1 gives bright contours.
 * The width is constant.
 */

int contour_shader_factorcolor_version(void) {return(2);}

miBoolean contour_shader_factorcolor(
	miContour_endpoint	*result,
	miStdInfo		*info_near,
	miStdInfo		*info_far,
	miState			*state,
	Factorcolor_Parameters  *paras)
{
	miScalar	f = *mi_eval_scalar(&paras->factor);
	miScalar	w = *mi_eval_scalar(&paras->width);

	miASSERT(info_near->color.r >= 0.0 && info_near->color.g >= 0.0);
	miASSERT(info_near->color.b >= 0.0);
	miASSERT(f >= 0.0 && w >= 0.0);

	/*
	 * Set contour color to a factor times material color.  The opacity
	 * color->a is set to 1.0, otherwise the material will shine through
	 * the contour.
	 */
	result->color.r = f * info_near->color.r;
	result->color.g = f * info_near->color.g;
	result->color.b = f * info_near->color.b;
	result->color.a = 1.0;

	/* The contour width is a parameter */
	result->width = w;
	return(miTRUE);
}


/*
 * The color of the contour is constant.  The width depends on the
 * material color.
 */

int contour_shader_widthfromcolor_version(void) {return(2);}

miBoolean contour_shader_widthfromcolor(
	miContour_endpoint	  *result,
	miStdInfo		  *info_near,
	miStdInfo		  *info_far,
	miState			  *state,
	Widthfromcolor_Parameters *paras)
{
	double			  maxcolor;
	miScalar		  min_width;

	/* Contour color given by a parameter */
	result->color = *mi_eval_color(&paras->color);

	/* The contour gets wider when the material color is dark */
	maxcolor = mi_MAX(info_near->color.r, info_near->color.g);
	maxcolor = mi_MAX(info_near->color.b, maxcolor);

	/* Softimage likes to have rgb colors larger than 1.0 */
	if (maxcolor > 1.0)
		maxcolor = 1.0;
	miASSERT(0.0 <= maxcolor && maxcolor <= 1.0);

	min_width = *mi_eval_scalar(&paras->min_width);
	result->width = (*mi_eval_scalar(&paras->max_width) - min_width) *
					(1.0 - maxcolor) + min_width;
	return(miTRUE);
}


/*
 * The color of the contour is a parameter.  The width depends on the
 * surface normal relative to a light source direction.
 * light dir = normal:   width = max_width
 * light dir = -normal:  width = min_width
 */

int contour_shader_widthfromlightdir_version(void) {return(3);}

miBoolean contour_shader_widthfromlightdir(
	miContour_endpoint  *result,
	miStdInfo	    *info_near,
	miStdInfo	    *info_far,
	miState		    *state,
	Lightdir_Parameters *paras)
{
	double		    d;
	miVector	    dir;
	miScalar	    max_width;
	miScalar	    min_width;

	/* Contour color given by a parameter */
	result->color = *mi_eval_color(&paras->color);

	/* Normalize light direction (just in case user didn't) */
	dir = *mi_eval_vector(&paras->light_dir);
	mi_vector_normalize(&dir);

	/* The contour gets wider the more the normal differs from the light
	   source direction */
	d = mi_vector_dot(&dir, &info_near->normal);

	min_width     = *mi_eval_scalar(&paras->min_width);
	max_width     = *mi_eval_scalar(&paras->max_width);
	result->width = min_width + 0.5 * (max_width - min_width) * (1.0 - d);

	miASSERT(min_width <= result->width && result->width <= max_width);
	return(miTRUE);
}


/*
 * The color of the contour is a parameter.  The width depends on the
 * surface normal relative to a light source direction.
 * light dir = normal:   width = max_width
 * light dir = -normal:  width = min_width
 */

int contour_shader_widthfromlight_version(void) {return(2);}

miBoolean contour_shader_widthfromlight(
	miContour_endpoint *result,
	miStdInfo	   *info_near,
	miStdInfo	   *info_far,
	miState		   *state,
	Light_Parameters   *paras)
{
	miVector	   orgp;	/* light source origin */
	miVector	   dirp;	/* light source direction */
	miVector	   dir;
	double		   d;
	miScalar	   min_width;
	miScalar	   max_width;

	/* Contour color given by a parameter */
	result->color = *mi_eval_color(&paras->color);

	/* Get light origin or direction */
	mi_light_info(*mi_eval_tag(&paras->light), &orgp, &dirp, 0);
	/* Now orgp or dirp is different from the null vector */

	/* Compute direction from light to point */
	if (mi_vector_dot(&orgp, &orgp) > miEPS) {   /* point light source */
		mi_vector_sub(&dir, &info_near->point, &orgp);

	} else {				/* directional light source */
		miASSERT(mi_vector_dot(&dirp, &dirp) > miEPS);
		dir = dirp;
	}
	mi_vector_normalize(&dir);

	/* The contour gets wider the more the normal is pointing in the same
	   direction as the light source */
	d = mi_vector_dot(&dir, &info_near->normal);
	min_width = *mi_eval_scalar(&paras->min_width);
	max_width = *mi_eval_scalar(&paras->max_width);
	result->width = min_width + 0.5 * (max_width - min_width) * (d+1.0);

	miASSERT(min_width <= result->width && result->width <= max_width);
	return(miTRUE);
}


/*
 * This is a combination of the depthfade, layerthinner, and widthfromlight
 * contour shaders.
 * The width of the contour fades into the background (from near_width
 * to far_width), and the color fades from near_color to far_color.
 * The contour width and color changes with a ramp function between
 * distances near_z and far_z.
 * For each layer the ray has passed through, a factor is multiplied on
 * to the width.
 * If the light source is different from the NULLtag, the width also
 * depends on the surface normal relative to a light source direction.
 */

int contour_shader_combi_version(void) {return(3);}

miBoolean contour_shader_combi(
	miContour_endpoint *result,
	miStdInfo	   *info_near,
	miStdInfo	   *info_far,
	miState		   *state,
	Combi_Parameters   *paras)
{
	miScalar	   depth = info_near->point.z;
	double		   near_z, far_z, w_near, w_far;
	miVector	   orgp;		/* light source origin */
	miVector	   dirp;		/* light source direction */
	miVector	   dir;
	double		   d;
	double		   factor;
	miTag		   light;

	/* Ensure that near_z and far_z are negative as they should be */
	near_z = -fabs(*mi_eval_scalar(&paras->near_z));
	far_z  = -fabs(*mi_eval_scalar(&paras->far_z));

	if (depth > near_z) {		/* contour is closer than near_z */
		result->color = *mi_eval_color(&paras->near_color);
		result->width = *mi_eval_scalar(&paras->near_width);

	} else if (depth < far_z) {	/* contour is more distant than far_z*/
		result->color = *mi_eval_color(&paras->far_color);
		result->width = *mi_eval_scalar(&paras->far_width);
	} else {			/* contour is betwn near_z and far_z */
		miColor near_color = *mi_eval_color(&paras->near_color);
		miColor far_color  = *mi_eval_color(&paras->far_color);
		/* Weights w_near and w_far depend on depth */
		w_near = (depth - far_z) / (near_z - far_z);
		miASSERT(0.0 <= w_near && w_near <= 1.0);
		w_far = 1.0 - w_near;

		/* Mix of near_color and far_color according to weights */
		result->color.r = w_near * near_color.r + w_far * far_color.r;
		result->color.g = w_near * near_color.g + w_far * far_color.g;
		result->color.b = w_near * near_color.b + w_far * far_color.b;
		result->color.a = w_near * near_color.a + w_far * far_color.a;

		/* Width depending on weights */
		result->width   = w_near * *mi_eval_scalar(&paras->near_width) 
				+ w_far  * *mi_eval_scalar(&paras->far_width);
	}

	/* Width decreases by factor for each refraction_level */
	factor = *mi_eval_scalar(&paras->factor);
	if (factor > miEPS)
		result->width *= pow(factor, (double)info_near->level - 1.0);

	light = *mi_eval_tag(&paras->light);
	if (light) {
		miScalar light_min_factor = 
				*mi_eval_scalar(&paras->light_min_factor);
		/* Get light origin or direction */
		mi_light_info(light, &orgp, &dirp, 0);
		/* Now orgp or dirp is different from the null vector */

		/* Compute direction from light to point */
		if (mi_vector_dot(&orgp, &orgp) > miEPS) /* point light */
			mi_vector_sub(&dir, &info_near->point, &orgp);

		else {				/* directional light source */
			miASSERT(mi_vector_dot(&dirp, &dirp) > miEPS);
			dir = dirp;
		}
		mi_vector_normalize(&dir);

		/* The contour gets wider the more the normal is pointing in
		   the same direction as the light source */
		d = mi_vector_dot(&dir, &info_near->normal);
		result->width *= 0.5 * (d + 1.0) * (1.0 - light_min_factor) +
						light_min_factor;
	}
	miASSERT(result->width <= *mi_eval_scalar(&paras->near_width));
	return(miTRUE);
}


/*
 * The color of the contour is the max of the two point colors.
 * The width is a parameter.
 */

int contour_shader_maxcolor_version(void) {return(2);}

miBoolean contour_shader_maxcolor(
	miContour_endpoint *result,
	miStdInfo	   *info_near,
	miStdInfo	   *info_far,
	miState		   *state,
	float		   *width)		/* only one parameter */
{
	/* Contour color is max of info_near->color and info_far->color */
	if (info_far) {
		result->color.r = mi_MAX(info_near->color.r,info_far->color.r);
		result->color.g = mi_MAX(info_near->color.g,info_far->color.g);
		result->color.b = mi_MAX(info_near->color.b,info_far->color.b);
		result->color.a = 1.0;
	} else
		result->color = info_near->color;

	/* Contour width given by a parameter */
	result->width = *mi_eval_scalar(width);
	return(miTRUE);
}


/*
 * Fixed color and width of the contour, only at silhouette lines
 */

int contour_shader_silhouette_version(void) {return(2);}

miBoolean contour_shader_silhouette(
	miContour_endpoint *result,
	miStdInfo	   *info_near,
	miStdInfo	   *info_far,
	miState		   *state,
	Simple_Parameters  *paras)
{
	if (info_far) {
		result->color.a = 0.0;
		result->width   = 0.0;
	} else {
		result->color   = *mi_eval_color(&paras->color);
		result->width   = *mi_eval_scalar(&paras->width);
	}
	miASSERT(result->color.r >= 0.0 && result->color.g >= 0.0);
	miASSERT(result->color.b >= 0.0 && result->width >= 0.0);

	return(miTRUE);
}

