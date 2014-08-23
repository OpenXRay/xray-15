/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	13.11.00
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_lightmap_write
 *	mib_lightmap_write_version
 *	mib_lightmap_sample
 *	mib_lightmap_sample_version
 *
 * History:
 *	13.11.00: Created
 *
 * Description:
 * mib_lightmap_write() is a lightmap shader that gets called in two
 * circumstances:
 * When state->type == miRAY_LM_VERTEX, it samples vertex data (
 * position, normal, texture coordinates) and stores in a result buffer.
 * When state->type == miRAY_LM_MESH it operates on a mesh.  For
 * each triangle in the mesh it finds the mapping into pixel space
 * of a given output texture and for each pixel samples a shader
 * and writes into the texture.
 *
 * mib_lightmap_sample() is typically used as the sampler.  It computes
 * the flux from global illumination and an array of lights and
 * returns the value which mib_lightmap_write() then stores.
 *****************************************************************************/

#include <shader.h>
#include <mi_shader_if.h>

#define mi_MIN(a,b)	((a) < (b) ? (a) : (b))
#define mi_MAX(a,b)	((a) > (b) ? (a) : (b))
#define mi_MIN3(a,b,c)	((a) < (b) ? mi_MIN(a,c) : mi_MIN(b,c))
#define mi_MAX3(a,b,c)	((a) > (b) ? mi_MAX(a,c) : mi_MAX(b,c))


/* result data type for mib_lightmap_write */
typedef struct mib_lightmap_write_result{
	miVector	point;		/* point in space */
	miVector	normal;		/* vertex normal */
	miVector	tex;		/* texture coordinates of vertex */
} mib_lightmap_write_result;

/* parameter type for mib_lightmap_write */
typedef struct mib_lightmap_write_param{
	miTag		texture;	/* writable texture */
	miTag		coord;		/* texture coordinate shader */
	miTag		sample_sh;	/* sampling shader */
} mib_lightmap_write_param;


/* parameter type for mib_lightmap_sample */
typedef struct mib_lightmap_sample_param{
	miBoolean	indirect;	/* do indirect illumination? */
	int		flip;		/* flip normals? */
	int		i_light;
	int		n_light;
	miTag		light[1];	/* lights to sample */
} mib_lightmap_sample_param;


/* A function for processing a single triangle from the mesh */
static void mib_lightmap_do_triangle(
	miState				*state,
	miImg_image			*img,
	mib_lightmap_write_result const	*a,
	mib_lightmap_write_result const	*b,
	mib_lightmap_write_result const	*c,
	miTag				shader_tag);


/*
 * This function weights together three vectors using the normalized weights
 * (barycentric coordinates) given.
 */

static void mib_lightmap_combine_vectors(
	miVector			*res,	/* result */
	miVector const			*a,	/* source vectors */
	miVector const			*b,
	miVector const			*c,
	miVector const			*bary);	/* weights */

/*
 * Constrains a barycentric vector to the triangle.
 */

static void mib_lightmap_bary_fixup(
	miVector			*bary);

/*
 * The main lightmapping shader. For every light-mapped object, it gets
 * called once per vertex with state->type set to miRAY_LM_VERTEX, and
 * once with state->type set to miRAY_LM_MESH for the entire object after
 * all the vertices have been samples. The per-vertex calls collect the
 * return values, which are used by the single per-object call at the end
 * to paint triangles into the writable map. Note the mi_lightmap_edit
 * and mi_lightmap_edit_end calls, which open and finish the light map.
 */

extern "C" DLLEXPORT int mib_lightmap_write_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lightmap_write(
	mib_lightmap_write_result	*result,
	miState				*state,
	mib_lightmap_write_param	*param,
	miRclm_mesh_render const	*arg)	/* argument */
{
	int				i;
	mib_lightmap_write_result const	*resdata;
	miImg_image			*img;
	miTag				tex_tag;
	miTag				shader_tag;
	miTag				coordshader_tag;
	void				*handle;
	miBoolean			success;

	switch (state->type) {
	  case miRAY_LM_VERTEX:
		/* Gathering vertex data */
		result->point  = state->point;
		result->normal = state->normal;
		mi_vector_normalize(&result->normal);
		coordshader_tag  = *mi_eval_tag(&param->coord);
		/* need co call the shader to get access to the success value*/
		success = mi_call_shader_x((miColor*)&result->tex,
				miSHADER_TEXTURE, state, coordshader_tag, 0);
		if (!success)
			/* mark this vertex as bad */
			result->tex.x = -1;
		break;

	  case miRAY_LM_MESH:
		if (!arg)
			return(miFALSE);

		tex_tag    = *mi_eval_tag(&param->texture);
		shader_tag = *mi_eval_tag(&param->sample_sh);

		if (!tex_tag || !shader_tag)
			return(miFALSE);

		if (!(img = mi_lightmap_edit(&handle, tex_tag)))
			return(miFALSE);

		resdata = (mib_lightmap_write_result const *)arg->vertex_data;

		for (i=0; i < arg->no_triangles; i++) {
			mi_state_set_pri(state, arg->pri, arg->triangles[i].pri_idx);

			mib_lightmap_do_triangle(state, img,
				&resdata[arg->triangles[i].a],
				&resdata[arg->triangles[i].b],
				&resdata[arg->triangles[i].c],
				shader_tag);
		}
		mi_lightmap_edit_end(handle);
	}
	return(miTRUE);
}


/*
 * this material shader simply computes the light irradiance at the current
 * intersection point.  It is normally used as a source shader for the
 * mib_lightmap_write, but can be replaced by anything else.
 */

extern "C" DLLEXPORT int mib_lightmap_sample_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lightmap_sample(
	miColor				*result,
	miState				*state,
	mib_lightmap_sample_param	*param)
{
	int				i_light;
	int				n_light;
	miTag				*light;
	miColor				color, sum;
	int				m;
	int				flip;
	int				times;

	flip    = *mi_eval_integer(&param->flip);
	i_light = *mi_eval_integer(&param->i_light);
	n_light = *mi_eval_integer(&param->n_light);
	light   =  n_light ? mi_eval_tag(param->light) + i_light : 0;
	
	times = flip==2? 2 : 1;

	result->r = result->g = result->b = 0.0f;
	for (m=0; m<times; m++) {
		if (flip == 1 || m==1) {
			mi_vector_neg(&state->normal);
			mi_vector_neg(&state->normal_geom);
		}
		for (mi::shader::LightIterator iter(state, light, n_light);
						!iter.at_end(); ++iter) {
			sum.r = sum.g = sum.b = 0.0f;
			while (iter->sample()) {
				miScalar dot_nl = iter->get_dot_nl();
				iter->get_contribution(&color);
				sum.r += dot_nl * color.r;
				sum.g += dot_nl * color.g;
				sum.b += dot_nl * color.b;
			}
			int samples = iter->get_number_of_samples();
			if (samples) {
				result->r += sum.r / samples;
				result->g += sum.g / samples;
				result->b += sum.b / samples;
			}
		}
		/* indirect illumination */
		if (*mi_eval_boolean(&param->indirect)) {
			mi_compute_irradiance(&color, state);
			result->r += color.r;
			result->g += color.g;
			result->b += color.b;
		}
	}
	if (flip >= 0) {
		mi_vector_neg(&state->normal);
		mi_vector_neg(&state->normal_geom);
	}
	result->a = 1.0f;
	return(miTRUE);
}


/******************** static functions ***********************/
/*
 * sample lightmap for a single triangle.  We determine the
 * triangle extents in pixel space, compute a mapping from pixel
 * space to barycentric coordinates, and loop over the pixels whose
 * centres fall within the triangle.
 * We do this by using a scanline approach where a linepair is
 * generate for the upper and lower part of the triangle.  This
 * makes certain that 'inside' detection for adjacent triangles
 * is handled identically and so no pixels are missed.
 * For each such center an intersection
 * is computed and a source shader called to get a value which is then
 * stored in the writable texture.
 */

typedef struct Line2d {
	float		s;	/* slope */
	float		o;	/* offset */
} Line2d;

static void mib_lightmap_do_triangle(
	miState				*state,
	miImg_image			*img,
	mib_lightmap_write_result const	*a,
	mib_lightmap_write_result const	*b,
	mib_lightmap_write_result const	*c,
	miTag				shader_tag)
{

	miVector			pixa, pixb, pixc;
	miMatrix			tmp1, tmp2;
	miMatrix			pixel_to_bary;
	miVector			p;
	miVector			d1, d2;
	miVector const			*pix_y[3], *tmp;
	Line2d				line[3];
	Line2d const			*left[2], *right[2];
	float				y_min, y_max;
	miBoolean			long_right;

	/* give up if any of the vertices was marked as not-to-use */
	if (a->tex.x < 0 ||
	    b->tex.x < 0 ||
	    c->tex.x < 0)
		return;
	/*
	 * compute pixel coordinates from texture coordinates. They are offset
	 * by half so that integer values land in the center of the pixels.
	 * pix = u * width - 0.5;
	 */
	pixa.x = a->tex.x * img->width  - 0.5f;
	pixb.x = b->tex.x * img->width  - 0.5f;
	pixc.x = c->tex.x * img->width  - 0.5f;
	pixa.y = a->tex.y * img->height - 0.5f;
	pixb.y = b->tex.y * img->height - 0.5f;
	pixc.y = c->tex.y * img->height - 0.5f;

	/* sort vertices in y increasing order */
	pix_y[0] = &pixa;
	pix_y[1] = &pixb;
	pix_y[2] = &pixc;
	if (pix_y[0]->y > pix_y[1]->y) {
		tmp = pix_y[0];
		pix_y[0] = pix_y[1];
		pix_y[1] = tmp;
	}
	if (pix_y[1]->y > pix_y[2]->y) {
		tmp = pix_y[1];
		pix_y[1] = pix_y[2];
		pix_y[2] = tmp;
	}
	if (pix_y[0]->y > pix_y[1]->y) {
		tmp = pix_y[0];
		pix_y[0] = pix_y[1];
		pix_y[1] = tmp;
	}

	/* avoid empty triangles */
	if (pix_y[0]->y >= pix_y[2]->y)
		return;

	/* compute lines */
	line[0].s = (pix_y[1]->x-pix_y[0]->x)/(pix_y[1]->y-pix_y[0]->y);
	line[0].o = pix_y[0]->x - pix_y[0]->y*line[0].s;

	line[1].s = (pix_y[2]->x-pix_y[1]->x)/(pix_y[2]->y-pix_y[1]->y);
	line[1].o = pix_y[1]->x - pix_y[1]->y*line[1].s;

	line[2].s = (pix_y[2]->x-pix_y[0]->x)/(pix_y[2]->y-pix_y[0]->y);
	line[2].o = pix_y[0]->x - pix_y[0]->y*line[2].s;

	/* remove degenerate line */
	if (pix_y[1]->y == pix_y[0]->y) {
		line[0] = line[1];
		long_right = line[1].s > line[2].s;
	} else if (pix_y[2]->y == pix_y[1]->y) {
		line[1] = line[0];
		long_right = line[0].s < line[2].s;
	} else
		long_right = line[0].s < line[2].s;

	/* arrange the lines */
	if (long_right) {
		left[0]  = &line[0];
		left[1]  = &line[1];
		right[0] = &line[2];
		right[1] = &line[2];
	} else {
		left[0]  = &line[2];
		left[1]  = &line[2];
		right[0] = &line[0];
		right[1] = &line[1];
	}


	/*
	 * pixel to barycentric coordinate transform. This is a 2D homogeneous
	 * problem (to allow for translation) so the third component is set to
	 * 1 and we have a 3-by-3 matrix equation.
	 */
	mi_matrix_ident(tmp1);
	tmp1[ 0] = pixa.x;
	tmp1[ 4] = pixb.x;
	tmp1[ 8] = pixc.x;
	tmp1[ 1] = pixa.y;
	tmp1[ 5] = pixb.y;
	tmp1[ 9] = pixc.y;
	tmp1[ 2] = 1.0f;
	tmp1[ 6] = 1.0f;
	tmp1[10] = 1.0f;
	mi_matrix_ident(tmp2);	/* corresponds to barycentric vectors */
	/* solve pix * pix_to_space = bary */
	if (!mi_matrix_solve(pixel_to_bary, tmp1, tmp2, 4))
		return;

	/* compute geometric normal of the triangle */
	mi_vector_sub(&d1, &b->point, &a->point);
	mi_vector_sub(&d2, &c->point, &a->point);
	mi_vector_prod(&state->normal_geom, &d1, &d2);
	mi_vector_normalize(&state->normal_geom);

	/* Loop over the texture y range */
	p.z = 1.0f;
	y_min = ceil(pix_y[0]->y);
	if (y_min < 0)
		y_min = 0;

	y_max = floor(pix_y[2]->y);
	if (y_min >= img->height)
		y_min = img->height-1.F;

	for (p.y=y_min; p.y <= y_max; p.y++) {
		float	left_x, right_x;
		int	i = p.y < pix_y[1]->y ? 0 : 1;

		/* Loop over texture X range */
		left_x  = ceil(left[i]->o  + p.y*left[i]->s);
		if (left_x<0)
			left_x = 0;

		right_x = floor(right[i]->o + p.y*right[i]->s);
		if (right_x>=img->width)
			right_x = img->width-1.F;

		for (p.x=left_x; p.x <= right_x; p.x++) {
			miVector bary;
			miColor  color;
			int	 j;
			miVector q=p;		/* jittered raster location */
			double	 jitter[2];
			state->raster_x	= p.x;
			state->raster_y = p.y;
			/* 143 is miQ_PIXEL_SAMPLE */
			if (mi_query(miQ_PIXEL_SAMPLE, state, 0, jitter)/* this && order */
			    && state->options->jitter) {
				q.x += (miScalar)jitter[0];
				q.y += (miScalar)jitter[1];
			}
			mi_vector_transform(&bary, &q, pixel_to_bary);

			/* constrain barycentric coordinates to triangle */
			mib_lightmap_bary_fixup(&bary);

			/* pixel center is inside triangle */
			mib_lightmap_combine_vectors(&state->point,
					&a->point, &b->point, &c->point,
					&bary);
			mib_lightmap_combine_vectors(&state->normal,
					&a->normal, &b->normal, &c->normal,
					&bary);
			mi_vector_normalize(&state->normal);

			/* set state->dir to directly incoming */
			state->dir.x = -state->normal.x;
			state->dir.y = -state->normal.y;
			state->dir.z = -state->normal.z;
			state->dot_nd = 1.0f;

			/* Fill out state->tex_list */
			for(j=0; ; j++) {
				const miVector *ta, *tb, *tc;
				if (!mi_tri_vectors(state, 't', j, 
						&ta, &tb, &tc))
					break;
				mib_lightmap_combine_vectors(
						&state->tex_list[j],
						ta, tb, tc, &bary);
			}

			if (state->options->shadow && state->type != miRAY_SHADOW ) {
			    double d_shadow_tol, tmp_d;

			    d_shadow_tol = mi_vector_dot_d(&state->normal, &a->point);
			    tmp_d = mi_vector_dot_d(&state->normal, &b->point);
			    if (d_shadow_tol < tmp_d)
				d_shadow_tol = tmp_d;
			    tmp_d = mi_vector_dot_d(&state->normal, &c->point);
			    if (d_shadow_tol < tmp_d)
				d_shadow_tol = tmp_d;
			    state->shadow_tol = d_shadow_tol
				- mi_vector_dot_d(&state->normal, &state->point);
			}

			/* get the color to write */
			mi_call_shader_x(&color, miSHADER_MATERIAL,
						state, shader_tag, 0);
			/* write to the image */
			mi_img_put_color(img, &color, (int)p.x, (int)p.y);
		}
	}
}


/*
 * combine vectors using weights
 */

static void mib_lightmap_combine_vectors(
	miVector	*res,
	miVector const	*a,
	miVector const	*b,
	miVector const	*c,
	miVector const	*bary)
{
	res->x = bary->x * a->x + bary->y * b->x + bary->z * c->x;
	res->y = bary->x * a->y + bary->y * b->y + bary->z * c->y;
	res->z = bary->x * a->z + bary->y * b->z + bary->z * c->z;
}


/*
 * Correct barycentric coordinates by projecting them to the
 * barycentric plane.  The plane equation is (P-u)*n = 0, where
 * 'u' is e.g. (1 0 0) and 'n' is the plane normal (1 1 1).
 * We seek a scalar s so that
 * (B-sn-u)*n = 0 => s = ((u-B)*n) / (n*n)
 * and then add s*n to B.
 *
 * We then clip the barycentric coordinates and as a final touch,
 * compute z as a function of x and y since they are not independent.
 * This means that we can leave z out of the projection and
 * clipping phase
 */

static void mib_lightmap_bary_fixup(
	miVector	*bary)
{
	float		s;

	s = (1.0f - bary->x - bary->y - bary->z)/3.0f;
	bary->x += s;
	bary->y += s;

	/* now clip coordinates */
	if (bary->x < 0.0f)
		bary->x = 0.0f;
	else if (bary->x > 1.0f)
		bary->x = 1.0f;

	if (bary->y < 0.0f)
		bary->y = 0.0f;
	else if (bary->y + bary->x > 1.0f)
		bary->y = 1.0f-bary->x;

	/* Finally, compute the dependent z */
	bary->z = 1.0f - bary->x - bary->y;
}

