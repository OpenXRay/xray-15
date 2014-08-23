/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	28.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_vector
 *	mib_texture_vector_version
 *	mib_texture_remap
 *	mib_texture_remap_version
 *	mib_texture_rotate
 *	mib_texture_rotate_version
 *	mib_bump_basis
 *	mib_bump_basis_version
 *	mib_bump_map
 *	mib_bump_map2
 *	mib_bump_map_version
 *	mib_passthrough_bump_map
 *	mib_passthrough_bump_map_version
 *
 * History:
 *	26.05.98: introduced project in texture_vector and bump_basis
 *	07.10.99: introduced lollipop projection (all corners at S pole)
 *	03.01.00: introduced TS_LIGHT coordinate space
 *	11.01.00: added offset parameter to mib_texture_remap
 *	26.01.00: added clamp parameter to mib_bump_map*
 *	23.11.00: removed unused local variable has_prev
 *
 * Description:
 * texture vector and bump basis vector related base shaders.
 * UV is not implemented because there is no way to tell GAP to generate UV
 * in a known location that can be checked for.
 *****************************************************************************/

#include <math.h>
#include <shader.h>
#include <mi_version.h>

#define TV_POINT	-1	/* texture vector selection */
#define TV_NORMAL	-2
#define TV_MOTION	-3
#define TV_DIR		-4
#define TV_DUDP		-5
#define TV_DVDP		-6
#define TV_DUUDDP	-7
#define TV_DVVDDP	-8
#define TV_DUVDDP	-9
#define TV_BGROUND	-10
#define TV_TEX		-11

#define TP_NONE		0	/* texture/bump-basis vector projection */
#define TP_UV		1
#define TP_XY		2
#define TP_XZ		3
#define TP_YZ		4
#define TP_SPHERE	5
#define TP_CYL		6
#define TP_LOLLIPOP	7

#define TS_INTERNAL	0	/* texture coordinate spaces (selspace) */
#define TS_OBJECT	1
#define TS_WORLD	2
#define TS_CAMERA	3
#define TS_SCREEN	4
#define TS_LIGHT	5

#define EPS		1e-4

struct uv {miVector u, v;};

char basetexgen_id[] = 
	"==@@== compiled for ray " MI_VERSION_STRING " on " MI_DATE_STRING;


/*------------------------------------------ mib_texture_vector -------------*/

struct mib_texture_vector {
	miInteger	select;
	miInteger	selspace;
	miInteger	vertex;
	miInteger	project;
};


extern "C" DLLEXPORT int mib_texture_vector_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_texture_vector(
	miVector	*result,
	miState		*state,
	struct mib_texture_vector *paras)
{
	miVector	v;		/* working vector */
	miVector	*vp;		/* pointer to working vector */
	miVector	tmp, *tri[3];	/* temporaries */
	miBoolean	point_conv = miTRUE;
	miInteger	vertex;
	miInteger	selspace = *mi_eval_integer(&paras->selspace);
	miInteger	select;
	miInteger	project;
	miBoolean	success;	/* is mapping successful */

	if (selspace == TS_SCREEN) {
		tmp.x = state->raster_x / state->camera->x_resolution *.99999f;
		tmp.y = state->raster_y / state->camera->y_resolution *.99999f;
		tmp.z = 0;
		state->tex = *result = tmp;
		return(miTRUE);
	}
	vertex  = *mi_eval_integer(&paras->vertex);
	select  = *mi_eval_integer(&paras->select);
	project = *mi_eval_integer(&paras->project);
	switch(select) {
	  case TV_POINT:
		if (vertex >= 1 && vertex <= 3) {
			if (!mi_tri_vectors(state,'p', select,
					tri+0, tri+1, tri+2)){
				result->x = result->y = result->z = 0;
				state->tex = *result;
				return(miFALSE);
			}
			vp = tri[vertex-1];
		} else
			vp = &state->point;
		break;

	  case TV_NORMAL:
		point_conv = miFALSE;
		if (vertex >= 1 && vertex <= 3) {
			if (!mi_tri_vectors(state,'n', select,
					tri+0, tri+1, tri+2)){
				result->x = result->y = result->z = 0;
				state->tex = *result;
				return(miFALSE);
			}
			vp = tri[vertex-1];
		} else
			vp = &state->normal;
		break;

	  case TV_MOTION:
		point_conv = miFALSE;
		if (vertex >= 1 && vertex <= 3) {
			if (!mi_tri_vectors(state,'m', select,
					tri+0, tri+1, tri+2)){
				result->x = result->y = result->z = 0;
				state->tex = *result;
				return(miFALSE);
			}
			vp = tri[vertex-1];
		} else
			vp = &state->motion;
		break;

	  case TV_DIR:
		point_conv = miFALSE;
		vp = &state->dir;
		break;

	  case TV_DUDP:
	  case TV_DVDP:
	  case TV_DUUDDP:
	  case TV_DVVDDP:
	  case TV_DUVDDP:
		point_conv = miFALSE;
		vp = &state->derivs[-select + TV_DUDP];
		break;

	  case TV_BGROUND:
		v.x = state->raster_x / state->camera->x_resolution * .9999f;
		v.y = state->raster_y / state->camera->y_resolution * .9999f;
		v.z = 0;
		vp = &v;
		selspace = TS_INTERNAL;
		break;

	  case TV_TEX:
		vp = &state->tex;
		point_conv = miFALSE;
		selspace = TS_INTERNAL;
		break;

	  default:
		if (vertex >= 1 && vertex <= 3) {
			if (!mi_tri_vectors(state,'t', select,
					tri+0, tri+1, tri+2)){
				result->x = result->y = result->z = 0;
				state->tex = *result;
				return(miFALSE);
			}
			vp = tri[vertex-1];
		} else {
			if (select < 0 || select >= MAX_TEX) {
				result->x = result->y = result->z = 0;
				state->tex = *result;
				return(miFALSE);
			}
			vp = &state->tex_list[select];
		}
	}
	if (point_conv && selspace && selspace != TS_SCREEN && state->time!=0){
		v.x = vp->x + state->motion.x * state->time;
		v.y = vp->y + state->motion.y * state->time;
		v.z = vp->z + state->motion.z * state->time;
		vp = &v;
	}
	switch(selspace) {
	  case TS_OBJECT:
		if (point_conv)
			mi_point_to_object(state, &v, vp);
		else
			mi_vector_to_object(state, &v, vp);
		vp = &v;
		break;

	  case TS_WORLD:
		if (point_conv)
			mi_point_to_world(state, &v, vp);
		else
			mi_vector_to_world(state, &v, vp);
		vp = &v;
		break;

	  case TS_CAMERA:
		if (point_conv)
			mi_point_to_camera(state, &v, vp);
		else
			mi_vector_to_camera(state, &v, vp);
		vp = &v;
		break;

	  case TS_LIGHT:
		if (point_conv)
			mi_point_to_light(state, &v, vp);
		else
			mi_vector_to_light(state, &v, vp);
		vp = &v;
		break;
	}
	success = miTRUE;
	switch(project) {
	  case TP_XY:
		v.x = vp->x;
		v.y = vp->y;
		v.z = 0;
		vp = &v;
		break;

	  case TP_XZ:
		v.x = vp->x;
		v.y = vp->z;
		v.z = 0;
		vp = &v;
		break;

	  case TP_YZ:
		v.x = vp->y;
		v.y = vp->z;
		v.z = 0;
		vp = &v;
		break;

	  case TP_SPHERE: {
		float length;
		length = mi_vector_norm(vp);
		/* put seam in x direction */
		if (!vp->z && !vp->x) {
			/* singularity point */
			v.x = 0.0;
			success = miFALSE;
		} else
			v.x = (miScalar)atan2(-vp->z, vp->x) / 
				    (miScalar)M_PI / 2.0f;

		if (v.x < 0) v.x += 1;
		/* y comes from asin */
		if (length)
			v.y = (miScalar)asin(vp->y / length) / 
				    (miScalar)M_PI + 0.5f;
		else
			v.y = 0.5f;
		v.z = 0;
		vp = &v;
		break; }

	  case TP_CYL:
		if (!vp->z && !vp->x) {
			v.x = 0.0;
			success = miFALSE;
		} else
			v.x = (miScalar)atan2(-vp->z, vp->x) / 
				    (miScalar)M_PI / 2.0f;

		if (v.x < 0) v.x += 1;
		v.y = vp->y;
		v.z = 0;
		vp = &v;
		break;

	  case TP_LOLLIPOP: {
		float hyp, rad;
		hyp = (miScalar)hypot(vp->x, vp->z);

		if (!hyp && !vp->y)
			rad = 0;
		else
			rad = (miScalar)atan2(hyp, vp->y) * (0.5f/(miScalar)M_PI);

		if (hyp>0.0f) {
			v.y = 0.5f - rad/hyp * vp->x;
			v.x = 0.5f - rad/hyp * vp->z;
		} else {
			v.y = 0.5f;
			v.x = 0.5f + rad;
			if (rad)
				success = miFALSE;	/* singularity point */
		}
		v.z = 0;
		vp = &v;
		break; }
	}
	*result = *vp;
	state->tex = *result;
	return(success);
}


/*------------------------------------------ mib_texture_remap --------------*/

struct mib_texture_remap {
	miVector	input;
	miMatrix	transform;
	miVector	repeat;
	miBoolean	alt_x;
	miBoolean	alt_y;
	miBoolean	alt_z;
	miBoolean	torus_x;
	miBoolean	torus_y;
	miBoolean	torus_z;
	miVector	min;
	miVector	max;
	miVector	offset;
};


extern "C" DLLEXPORT int mib_texture_remap_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_remap(
	miVector	*result,
	miState		*state,
	struct mib_texture_remap *paras,
	void		*x_arg)
{
	miVector	tex, *vp, *vq;	/* parameter space coord */
	miScalar	*transform;	/* transformation parameter */
	int		i;

	if (x_arg)
		tex = *(miVector*)x_arg;
	else
		tex = *mi_eval_vector(&paras->input);
	transform  = mi_eval_scalar(&paras->transform);		/* transform */
	if (transform[15])
		mi_point_transform(&tex, &tex, transform);

	if (*mi_eval_boolean(&paras->torus_x))			/* torus */
		tex.x -= (miScalar)floor(tex.x);
	if (*mi_eval_boolean(&paras->torus_y))
		tex.y -= (miScalar)floor(tex.y);
	if (*mi_eval_boolean(&paras->torus_z))
		tex.z -= (miScalar)floor(tex.z);

	vp = mi_eval_vector(&paras->repeat);			/* repeat/alt*/
	if (vp->x && tex.x >= 0 && tex.x < 1) {
		tex.x *= vp->x;
		if (*mi_eval_boolean(&paras->alt_x)) {
			i = (int)floor(tex.x);
			if (!(i & 1))
				tex.x = 2*i + 1 - tex.x;
		}
		tex.x -= (miScalar)floor(tex.x);
	}
	if (vp->y && tex.y >= 0 && tex.y < 1) {
		tex.y *= vp->y;
		if (*mi_eval_boolean(&paras->alt_y)) {
			i = (int)floor(tex.y);
			if (!(i & 1))
				tex.y = 2*i + 1 - tex.y;
		}
		tex.y -= (miScalar)floor(tex.y);
	}
	if (vp->z && tex.z >= 0 && tex.z < 1) {
		tex.z *= vp->z;
		if (*mi_eval_boolean(&paras->alt_z)) {
			i = (int)floor(tex.z);
			if (!(i & 1))
				tex.z = 2*i + 1 - tex.z;
		}
		tex.z -= (miScalar)floor(tex.z);
	}

	vp = mi_eval_vector(&paras->min);			/* crop */
	vq = mi_eval_vector(&paras->max);
	if (vp->x != vq->x)
		tex.x = vp->x + tex.x * (vq->x - vp->x);
	if (vp->y != vq->y)
		tex.y = vp->y + tex.y * (vq->y - vp->y);
	if (vp->z != vq->z)
		tex.z = vp->z + tex.z * (vq->z - vp->z);

	vp = mi_eval_vector(&paras->offset);			/* offset */
	mi_vector_add(result, &tex, vp);
	state->tex = *result;
	return(miTRUE);
}


/*------------------------------------------ mib_texture_rotate -------------*/

struct mib_texture_rotate {
	miVector	input;
	miScalar	angle;
	miScalar	min;
	miScalar	max;
};

extern "C" DLLEXPORT int mib_texture_rotate_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_rotate(
	struct uv	*result,
	miState		*state,
	struct mib_texture_rotate *paras)
{
	miScalar	angle, sin_angle, cos_angle;
	miScalar	min, max, d;
	miVector	tmp_u;

	angle = *mi_eval_scalar(&paras->angle);
	min   = *mi_eval_scalar(&paras->min);
	max   = *mi_eval_scalar(&paras->max);

	if (min == 0 && max == 0)
		max = 0.25;
	angle = (min + angle * (max - min)) * 2 * (miScalar)M_PI;
	sin_angle = (miScalar)sin(angle);
	cos_angle = (miScalar)cos(angle);

	result->u = *mi_eval_vector(&paras->input);
	d = mi_vector_dot(&result->u, &state->normal);
	result->u.x -= d * state->normal.x;
	result->u.y -= d * state->normal.y;
	result->u.z -= d * state->normal.z;
	mi_vector_normalize(&result->u);

	/* Set v to be perpendicular to u (in the tangent plane) */
	mi_vector_prod(&result->v, &state->normal, &result->u);

	/* u' = cos(angle) * u + sin(angle) * v */
	tmp_u.x = cos_angle * result->u.x + sin_angle * result->v.x;
	tmp_u.y = cos_angle * result->u.y + sin_angle * result->v.y;
	tmp_u.z = cos_angle * result->u.z + sin_angle * result->v.z;

	/* v' = -sin(angle) * u + cos(angle) * v */
	result->v.x = -sin_angle * result->u.x + cos_angle * result->v.x;
	result->v.y = -sin_angle * result->u.y + cos_angle * result->v.y;
	result->v.z = -sin_angle * result->u.z + cos_angle * result->v.z;

	result->u = tmp_u;
	return(miTRUE);
}


/*------------------------------------------ mib_bump_basis -----------------*/
/*
 * The parts excluded with #if 0/1 below replace the fancier local space
 * determination with something simple but stable, even at singularities.
 */

struct mib_bump_basis {
	miInteger	project;
	miInteger	ntex;
};

extern "C" DLLEXPORT int mib_bump_basis_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_bump_basis(
	struct uv	*result,
	miState		*state,
	struct mib_bump_basis *paras)
{
	miInteger	project = *mi_eval_integer(&paras->project);
	miVector	*v0, *v1, *v2;	/* triangle points in object space */
	miVector	*t0, *t1, *t2;	/* triangle texture verts */
	miVector	refdir;
	double		fabs01x, fabs02x;

	double		t;
	miBoolean	have_v = miTRUE;

#if 0	/* see #if 1 under TP_SPHERE etc below */
	if (project != TP_XY && project != TP_XZ && project != TP_YZ) {
		miInteger ntex = *mi_eval_integer(&paras->ntex);
		if (!mi_tri_vectors(state, 'p', 0,    &v0, &v1, &v2) ||
		    !mi_tri_vectors(state, 't', ntex, &t0, &t1, &t2)) {
			result->u.x = result->u.y = result->u.z =
			result->v.x = result->v.y = result->v.z = 0;
			return(miFALSE);
		}
	}
#endif
	switch(project) {
	  default:
	  case TP_UV: {						/* UV */
#if 1		/* see #if 1 under TP_SPHERE etc below */
		miInteger ntex = *mi_eval_integer(&paras->ntex);
		if (!mi_tri_vectors(state, 'p', 0,    &v0, &v1, &v2) ||
		    !mi_tri_vectors(state, 't', ntex, &t0, &t1, &t2)) {
			result->u.x = result->u.y = result->u.z =
			result->v.x = result->v.y = result->v.z = 0;
			return(miFALSE);
		}
#endif
		/* find a line of constant v on the triangle */
		if (fabs(t0->y - t1->y) < EPS) {
			have_v = miFALSE;
			if (t0->x > t1->x)
				mi_vector_sub(&result->u, v0, v1);
			else
				mi_vector_sub(&result->u, v1, v0);
			/* calc a reference dir roughly
			   in the positive v direction */
			if (t0->y > t2->y)
				mi_vector_sub(&refdir, v0, v2);
			else
				mi_vector_sub(&refdir, v2, v0);
		} else if (fabs(t0->y - t2->y) < EPS) {
			have_v = miFALSE;
			if (t0->x > t2->x)
				mi_vector_sub(&result->u, v0, v2);
			else
				mi_vector_sub(&result->u, v2, v0);
			/* calc a reference dir roughly
			   in the positive v direction */
			if (t0->y > t1->y)
				mi_vector_sub(&refdir, v0, v1);
			else
				mi_vector_sub(&refdir, v1, v0);
		} else if (fabs(t1->y - t2->y) < EPS) {
			have_v = miFALSE;
			if (t1->x > t2->x)
				mi_vector_sub(&result->u, v1, v2);
			else
				mi_vector_sub(&result->u, v2, v1);
			/* calc a reference dir roughly
			   in the positive v direction */
			if (t1->y > t0->y)
				mi_vector_sub(&refdir, v1, v0);
			else
				mi_vector_sub(&refdir, v0, v1);
		/* find a line of constant u on the triangle */
		} else if ((fabs01x = fabs(t0->x - t1->x)) < EPS) {
			if (t0->y > t1->y)
				mi_vector_sub(&result->v, v0, v1);
			else
				mi_vector_sub(&result->v, v1, v0);
			/* calc a reference dir roughly
			   in the positive u direction */
			if (t0->x > t2->x)
				mi_vector_sub(&refdir, v0, v2);
			else
				mi_vector_sub(&refdir, v2, v0);
		} else if ((fabs02x = fabs(t0->x - t2->x)) < EPS) {
			if (t0->y > t2->y)
				mi_vector_sub(&result->v, v0, v2);
			else
				mi_vector_sub(&result->v, v2, v0);
			/* calc a reference dir roughly
			   in the positive u direction */
			if (t0->x > t1->x)
				mi_vector_sub(&refdir, v0, v1);
			else
				mi_vector_sub(&refdir, v1, v0);
		} else if (fabs(t1->x - t2->x) < EPS) {
			if (t1->y > t2->y)
				mi_vector_sub(&result->v, v1, v2);
			else
				mi_vector_sub(&result->v, v2, v1);
			/* calc a reference dir roughly
			   in the positive u direction */
			if (t1->x > t0->x)
				mi_vector_sub(&refdir, v1, v0);
			else
				mi_vector_sub(&refdir, v0, v1);
		} else {
			miVector tmp;
			double	a, b;
			tmp.x = t0->x;
			tmp.y = (miScalar)(t0->y + 0.1);
			t = ((t1->x - t0->x) * (t2->y - t0->y) -
			     (t1->y - t0->y) * (t2->x - t0->x));
			if (fabs(t) > miSCALAR_EPSILON)
			    t = 1.f/t;

			a = t *-((t2->x - t0->x) * (tmp.y - t0->y));
			b = t * ((t1->x - t0->x) * (tmp.y - t0->y));
			result->v.x = (miScalar)(a * (v1->x - v0->x)
					+ b * (v2->x - v0->x));
			result->v.y = (miScalar)(a * (v1->y - v0->y)
					+ b * (v2->y - v0->y));
			result->v.z = (miScalar)(a * (v1->z - v0->z)
					+ b * (v2->z - v0->z));
			if (fabs02x > fabs01x) {
				if (t0->x > t2->x)
					mi_vector_sub(&refdir, v0, v2);
				else
					mi_vector_sub(&refdir, v2, v0);
			} else {
				if (t0->x > t1->x)
					mi_vector_sub(&refdir, v0, v1);
				else
					mi_vector_sub(&refdir, v1, v0);
			}
		}
		if (have_v) {
			mi_vector_normalize(&result->v);
			mi_vector_prod(&result->u, &state->normal, &result->v);
			/* The result should be less than 90 degrees
			   from refdir. if it isn't, flip it */
			if (mi_vector_dot(&result->u, &refdir) < 0)
				mi_vector_neg(&result->u);
		} else {
			mi_vector_normalize(&result->u);
			mi_vector_prod(&result->v, &result->u, &state->normal);
			/* The result should be less than 90 degrees
			   from refdir. if it isn't, flip it */
			if (mi_vector_dot(&result->v, &refdir) < 0)
				mi_vector_neg(&result->v);
		}
		break;
	  }

	  case TP_XY:						/* XY */
		result->u.x = 1.0;
		result->u.y = 0.0;
		result->u.z = 0.0;
		result->v.x = 0.0;
		result->v.y = 1.0;
		result->v.z = 0.0;
		break;

	  case TP_XZ:						/* XZ */
		result->u.x = 1.0;
		result->u.y = 0.0;
		result->u.z = 0.0;
		result->v.x = 0.0;
		result->v.y = 0.0;
		result->v.z = 1.0;
		break;

	  case TP_YZ:						/* YZ */
		result->u.x = 0.0;
		result->u.y = 1.0;
		result->u.z = 0.0;
		result->v.x = 0.0;
		result->v.y = 0.0;
		result->v.z = 1.0;
		break;

	  case TP_SPHERE:					/* spherical */
	  case TP_CYL:						/* cylindric */
	  case TP_LOLLIPOP: {					/* lollipop */
#if 1		/*
		 * avoids triangle discontinuities. Real cheap but doesn't
		 * require triangle vertices. Assumes object center at 0/0/0
		 * in object coords. Might fail at poles. 5.11.99
		 */
		miVector point, normal;
		mi_point_to_object(state, &point, &state->point);
		mi_vector_to_object(state, &normal, &state->normal);
		result->u.x = -point.z;
		result->u.y = 0;
		result->u.z = point.x;
		mi_vector_normalize(&result->u);
		mi_vector_prod(&result->v, &result->u, &state->normal);
		mi_vector_prod(&result->u, &state->normal, &result->v);
#else	
		assert(false);
		This code is out of date.  The code searching for constant
		u probably needs to be updated like TP_UV above.
		/*
		 * causes a bug, triangles are plainly visible
		 * when bumpmapping. Disabled. Warning - when
		 * re-enabling, call mi_tri_vectors above. 5.11.99
		 */
		miVector *tmp;
		if (!mi_tri_vectors(state, 'p', 0, &v0, &v1, &v2)) {
			result->u.x = result->u.y = result->u.z =
			result->v.x = result->v.y = result->v.z = 0;
			return(miFALSE);
		}
		miScalar y0 = v0->y, y1 = v1->y, y2 = v2->y;

		/* sort the verts */
		if (y0 < y1 && y1 < y2 || y2 < y1 && y1 < y0) {
			tmp = v0; v0 = v1; v1 = v2; v2 = tmp;
			tmp = t0; t0 = t1; t1 = t2; t2 = tmp;

		} else if (y0 < y2 && y2 < y1 || y1 < y2 && y2 < y0) {
			tmp = v2; v2 = v1; v1 = v0; v0 = tmp;
			tmp = t2; t2 = t1; t1 = t0; t0 = tmp;
		}

		/* find a line of constant u on the triangle */
		if (fabs(t1->x - t2->x) > EPS) {
			t = t0->x / (t2->x - t1->x);
			result->v.x = v0->x - v1->x - t * (v2->x - v1->x);
			result->v.y = v0->y - v1->y - t * (v2->y - v1->y);
			result->v.z = v0->z - v1->z - t * (v2->z - v1->z);
		} else if (fabs(t0->x - t2->x) > EPS) {
			t = t1->x / (t0->x - t2->x);
			result->v.x = v1->x - v2->x - t * (v0->x - v2->x);
			result->v.y = v1->y - v2->y - t * (v0->y - v2->y);
			result->v.z = v1->z - v2->z - t * (v0->z - v2->z);
		} else {
			result->v.x = 0;
			result->v.y = 0;
			result->v.z = 0;
		}
		mi_vector_normalize(&result->v);

		/* check for consistent orientation of bump_v */
		if (result->v.x < 0)
			mi_vector_neg(&result->v);

		/* find a line of constant v on the triangle */
		mi_vector_prod(&result->u, &state->normal, &result->v);
#endif
		}
	}
	return(miTRUE);
}


/*------------------------------------------ mib_bump_map -------------------*/

struct mib_bump_map {
	miVector	u;
	miVector	v;
	miVector	coord;
	miVector	step;
	miScalar	factor;
	miBoolean	torus_u;
	miBoolean	torus_v;
	miBoolean	alpha;
	miTag		tex;
	miBoolean	clamp;
};

extern "C" DLLEXPORT int mib_bump_map_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_bump_map(
	miVector	*result,
	miState		*state,
	struct mib_bump_map *paras)
{
	miTag		tex	= *mi_eval_tag    (&paras->tex);
	miBoolean	alpha	= *mi_eval_boolean(&paras->alpha);
	miVector	coord	= *mi_eval_vector (&paras->coord);
	miVector	step	= *mi_eval_vector (&paras->step);
	miVector	u	= *mi_eval_vector (&paras->u);
	miVector	v	= *mi_eval_vector (&paras->v);
	miScalar	factor	= *mi_eval_scalar (&paras->factor);
	miBoolean	clamp	= *mi_eval_boolean(&paras->clamp);
	miVector	coord_u, coord_v;
	miScalar	val, val_u, val_v;
	miColor		color;

	coord_u.x = coord.x + (step.x ? step.x : 0.001f);
	coord_u.y = coord.y;
	coord_u.z = coord.z;
	coord_v.x = coord.x;
	coord_v.y = coord.y + (step.y ? step.y : 0.001f);
	coord_v.z = coord.z;
	if (clamp && (coord.x   < 0 || coord.x   >= 1 ||
		      coord.y   < 0 || coord.y   >= 1 ||
		      coord.z   < 0 || coord.z   >= 1 ||
		      coord_u.x < 0 || coord_u.x >= 1 ||
		      coord_v.y < 0 || coord_v.y >= 1)) {
		*result = state->normal;
		return(miTRUE);
	}
	if (!tex || !mi_lookup_color_texture(&color, state, tex, &coord)) {
		*result = state->normal;
		return(miFALSE);
	}
	val = alpha ? color.a : (color.r + color.g + color.b) / 3;

	if (*mi_eval_boolean(&paras->torus_u)) {
		coord_u.x -= (miScalar)floor(coord_u.x);
		coord_u.y -= (miScalar)floor(coord_u.y);
		coord_u.z -= (miScalar)floor(coord_u.z);
	}
	mi_flush_cache(state);
	val_u = mi_lookup_color_texture(&color, state, tex, &coord_u)
		? alpha ? color.a : (color.r + color.g + color.b) / 3 : val;

	if (*mi_eval_boolean(&paras->torus_v)) {
		coord_v.x -= (miScalar)floor(coord_v.x);
		coord_v.y -= (miScalar)floor(coord_v.y);
		coord_v.z -= (miScalar)floor(coord_v.z);
	}
	mi_flush_cache(state);
	val_v = mi_lookup_color_texture(&color, state, tex, &coord_v)
		? alpha ? color.a : (color.r + color.g + color.b) / 3 : val;

	val_u -= val;
	val_v -= val;
	state->normal.x += factor * (u.x * val_u + v.x * val_v);
	state->normal.y += factor * (u.y * val_u + v.y * val_v);
	state->normal.z += factor * (u.z * val_u + v.z * val_v);
	mi_vector_normalize(&state->normal);
	state->dot_nd = mi_vector_dot(&state->normal, &state->dir);
	*result = state->normal;
	return(miTRUE);
}

/*----------------------------------------- mib_bump_map2 -------------------*/

struct mib_bump_map2 {
    miScalar	factor;	    /* How strong the bumps are */
    miScalar	scale;	    /* How many time the bumps are repeated */
    miTag	tex;	    /* The Texture to lookup */
    miColor	color;	    /* The attach illumination branch (ex. Phong) */
};

extern "C" DLLEXPORT int mib_bump_map2_version(void) {return(1);}

/*-----------------------------------------------------------------------------
 * The bump map will look at 3 texture samples from the texture, using the
 * texture space 0. Base on the lighting differences, this will indicate the 
 * bending of the normal.
 * The normal will be bend in the direction of the bump_vectors if they
 * exist, or in the direction of the derivatives.
 *
 * The modify normal is save in the "state", which then call the evaluation
 * of color. All attach shaders will then used this modify normal.
 *
 * The normal is set back to its original value at the end.
 *---------------------------------------------------------------------------*/
extern "C" DLLEXPORT miBoolean mib_bump_map2(
    miColor		    *result,
    miState		    *state,
    struct mib_bump_map2    *paras)
{
    miVector	temp = state->normal;
    miVector	normal = state->normal;
    miVector	coord_u, coord_v;
    miColor	color;
    float	val, val_u, val_v;
    miTag	tex	= *mi_eval_tag    (&paras->tex);
    miScalar	scale	= *mi_eval_scalar (&paras->scale);
    float	factor	= *mi_eval_scalar (&paras->factor);
    miVector	*tangent;
    miVector	*binormal;
    int		num;

    miVector coord;
    coord.x = state->tex_list[0].x * scale;
    coord.y = state->tex_list[0].y * scale;
    coord.x -= (miScalar)floor(coord.x);
    coord.y -= (miScalar)floor(coord.y);
    coord_u = coord_v = coord;
    coord_u.x += 0.001f;
    coord_v.y += 0.001f;

    mi_lookup_color_texture(&color, state, tex, &coord);
    val = (color.r + color.g + color.b) / 3;

    mi_lookup_color_texture(&color, state, tex, &coord_u);
    val_u = (color.r + color.g + color.b) / 3;

    mi_lookup_color_texture(&color, state, tex, &coord_v);
    val_v = (color.r + color.g + color.b) / 3;

    val_u -= val;
    val_v -= val;

    /* First choose the bump vectors if they exist or take the derivatives */
    if (mi_query(miQ_NUM_BUMPS, state, 0, &num) && num) {
	tangent = &state->bump_x_list[0];
	binormal = &state->bump_y_list[0];
    } else {
	tangent = &state->derivs[0];
	binormal = &state->derivs[1];
    }


    normal.x += factor * (tangent->x  * val_u + 
			  binormal->x * val_v);
    normal.y += factor * (tangent->y  * val_u + 
			  binormal->y * val_v);
    normal.z += factor * (tangent->z  * val_u + 
			  binormal->z * val_v);
    mi_vector_normalize(&normal);

    /* The evaluation of "color" will use the modify state normal */
    state->normal = normal;
    *result = *mi_eval_color(&paras->color);
    state->normal = temp;	/* return to original state */

    return miTRUE;
}


/*------------------------------------------ mib_passthrough_bump_map -------*/

extern "C" DLLEXPORT int mib_passthrough_bump_map_version(void) {return(2);}

extern "C" DLLEXPORT miBoolean mib_passthrough_bump_map(
    miColor		*result,
    miState		*state,
struct mib_bump_map *paras)
{
    miVector	dummy;

    return(mib_bump_map(&dummy, state, paras));
}
