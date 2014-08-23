/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	29.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_lookup_spherical_version
 *	mib_lookup_spherical
 *	mib_lookup_cube1_version
 *	mib_lookup_cube1
 *	mib_lookup_cube6_version
 *	mib_lookup_cube6
 *	mib_lookup_background_version
 *	mib_lookup_background
 *	mib_lookup_cylindrical_version
 *	mib_lookup_cylindrical
 *
 * History:
 *	19.3.1998: added cylindrical lookup
 *	12.1.2001: fixed mib_lookup_background, pan was not evaluated
 *                 zoom was used instead of pan
 *      10.1.2003: fixed torus_v, it sat x instead of y
 *
 * Description:
 * environments, overlays, and underlays. Useful for compositing.
 *****************************************************************************/

#include <math.h>
#include <shader.h>


/*------------------------------------------ mib_lookup_spherical -----------*/

struct mib_texture_lookup_spherical {
	miVector	dir;
	miScalar	rotate;
	miTag		tex;
};

extern "C" DLLEXPORT int mib_lookup_spherical_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lookup_spherical(
	miColor		*result,
	miState		*state,
	struct mib_texture_lookup_spherical *paras)
{
	miTag		tex = *mi_eval_tag(&paras->tex);
	miVector	dir = *mi_eval_vector(&paras->dir);
	double		theta, norm;

	result->r = result->g = result->b = result->a = 0;
	if (!tex)
		return(miFALSE);
	if (dir.x == 0 && dir.y == 0 && dir.z == 0)
		mi_vector_to_world(state, &dir, &state->dir);

	norm = mi_vector_norm(&dir);
	if (!norm)
		return(miFALSE);

	/*
	 * without rotation, 0 is in the direction of the x axis,
	 * increasing in a clockwise direction
	 */

	/* avoid calling atan2(0, 0), which return NaN on some platforms */
	if (!dir.x && !dir.z)
	        theta = 0.0;
	else
	        theta = -atan2(dir.x, dir.z) / (2.0*M_PI);

	theta += *mi_eval_scalar(&paras->rotate) / M_PI;
	theta -= floor(theta);
	dir.x = (miScalar) theta;
	dir.y = (miScalar) (asin(dir.y/norm) / M_PI + 0.5);
	dir.z = 0.0f;
	return(mi_lookup_color_texture(result, state, tex, &dir));
}


/*------------------------------------------ mib_lookup_cube* ---------------*/

#define EPS 1e-6

#define TEX_ERR		-1
#define TEX_MX		0		/* order must agree with */
#define TEX_PX		1		/* mib_texture_cube6 parameter order */
#define TEX_MY		2
#define TEX_PY		3
#define TEX_MZ		4
#define TEX_PZ		5

static int face_select(
	miVector	*coord,		/* returned lookup coordinate */
	miVector	*point,		/* viewpoint (state->org) */
	miVector	*dir,		/* view direction */
	miVector	*size)		/* box size */
{
	miVector	x, sn;
	double		s;

	mi_vector_normalize(dir);
	mi_vector_mul(size, 0.5);
	if (fabs(dir->x) >= EPS) {
		s = (size->x - point->x) / dir->x;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn, s);
			mi_vector_add(&x, point, &sn);
			if (x.y >= -size->y && x.y <= size->y &&
			    x.z >= -size->z && x.z <= size->z && dir->x > 0) {
				coord->x = 1.0 - 0.5 * (x.y / size->y + 1.0);
				coord->y = 0.5 * (x.z / size->z + 1.0);
				coord->z = 0.0;
				return(TEX_PX);
			}
		}
		s = -(size->x + point->x) / dir->x;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn, s);
			mi_vector_add(&x, point, &sn);
			if (x.y >= -size->y && x.y <= size->y &&
			    x.z >= -size->z && x.z <= size->z && dir->x < 0) {
				coord->x = 0.5 * (x.y / size->y + 1.0);
				coord->y = 0.5 * (x.z / size->z + 1.0);
				coord->z = 0.0;
				return(TEX_MX);
			}
		}
	}
	if (fabs(dir->y) >= EPS) {
		s = -(size->y + point->y) / dir->y;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn,s);
			mi_vector_add(&x,point,&sn);
			if (x.x >= -size->x && x.x <= size->x &&
			    x.z >= -size->z && x.z <= size->z && dir->y < 0) {
				coord->x = 1.0 - 0.5 * (x.x / size->x + 1.0);
				coord->y = 0.5 * (x.z / size->z + 1.0);
				coord->z = 0.0;
				return(TEX_MZ);
			}
		}
		s = (size->y - point->y) / dir->y;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn, s);
			mi_vector_add(&x, point, &sn);
			if (x.x >= -size->x && x.x <= size->x &&
			    x.z >= -size->z && x.z <= size->z && dir->y > 0) {
				coord->x = 0.5 * (x.x / size->x + 1.0);
				coord->y = 0.5 * (x.z / size->z + 1.0);
				coord->z = 0.0;
				return(TEX_PZ);
			}
		}
	}
	if (fabs(dir->z) >= EPS) {
		s = (size->z - point->z) / dir->z;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn, s);
			mi_vector_add(&x, point, &sn);
			if (x.x >= -size->x && x.x <= size->x &&
			    x.y >= -size->y && x.y <= size->y && dir->z > 0) {
				coord->x = 0.5 * (x.x / size->x + 1.0);
				coord->y = 0.5 * (x.y / size->y + 1.0);
				coord->z = 0.0;
				return(TEX_PY);
			}
		}
		s = -(point->z + size->z) / dir->z;
		if (s >= 0.0) {
			sn = *dir;
			mi_vector_mul(&sn, s);
			mi_vector_add(&x, point, &sn);
			if (x.x >= -size->x && x.x <=size->x &&
			    x.y >= -size->y && x.y <=size->y && dir->z < 0) {
				coord->x = 0.5 * (x.x / size->x + 1.0);
				coord->y = 0.5 * (x.y / size->y + 1.0);
				coord->z = 0.0;
				return(TEX_MY);
			}
		}
	}
	mi_debug("mib_cube: cube environment mapping failed");
	return(TEX_ERR);
}


/*------------------------------------------ mib_lookup_cube1 ---------------*/

struct mib_lookup_cube1 {
	miVector	point;
	miVector	dir;
	miVector	size;
	miTag		tex;
};

extern "C" DLLEXPORT int mib_lookup_cube1_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lookup_cube1(
	miColor		*result,
	miState		*state,
	struct mib_lookup_cube1 *paras)
{
	miTag		tex = *mi_eval_tag(&paras->tex);
	miVector	dir;
	miVector	size;
	miVector	coord;
	int		face;

	if (!tex) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}
	dir  = *mi_eval_vector(&paras->dir);
	size = *mi_eval_vector(&paras->size);
	face = face_select(&coord, mi_eval_vector(&paras->point), &dir, &size);
	if (face == TEX_ERR) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}
	coord.x -= floor(coord.x);
	coord.y -= floor(coord.y);
	coord.z -= floor(coord.z);
	coord.x  = (coord.x + face) / 6;
	return(mi_lookup_color_texture(result, state, tex, &coord));
}


/*------------------------------------------ mib_lookup_cube6 ---------------*/

struct mib_lookup_cube6 {
	miVector	point;
	miVector	dir;
	miVector	size;
	miTag		tex[6];
};

extern "C" DLLEXPORT int mib_lookup_cube6_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lookup_cube6(
	miColor		*result,
	miState		*state,
	struct mib_lookup_cube6 *paras)
{
	miTag		tex;
	miVector	dir;
	miVector	size;
	miVector	coord;
	int		face;

	dir  = *mi_eval_vector(&paras->dir);
	size = *mi_eval_vector(&paras->size);
	face = face_select(&coord, mi_eval_vector(&paras->point), &dir, &size);
	if (face == TEX_ERR) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}
	coord.x -= floor(coord.x);
	coord.y -= floor(coord.y);
	coord.z -= floor(coord.z);
	if (tex = *mi_eval_tag(paras->tex + face))
		return(mi_lookup_color_texture(result, state, tex, &coord));

	result->r = result->g = result->b = result->a = 0;
	return(miFALSE);
}


/*------------------------------------------ mib_lookup_background ----------*/

struct mib_lookup_background {
	miVector	zoom;
	miVector	pan;
	miBoolean	torus_u;
	miBoolean	torus_v;
	miTag		tex;
};

extern "C" DLLEXPORT int mib_lookup_background_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lookup_background(
	miColor		*result,
	miState		*state,
	struct mib_lookup_background *paras)
{
	miVector	*zoom;
	miVector	*pan;
	miVector	coord;
	miTag		tex = *mi_eval_tag(&paras->tex);

	if (!tex) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}
	zoom = mi_eval_vector(&paras->zoom);
	pan  = mi_eval_vector(&paras->pan);
	coord.x = state->raster_x / state->camera->x_resolution * .9999;
	coord.y = state->raster_y / state->camera->y_resolution * .9999;
	coord.z = 0;
	coord.x = pan->x + (zoom->x ? zoom->x * coord.x : coord.x);
	coord.y = pan->y + (zoom->y ? zoom->y * coord.y : coord.y);
	if (*mi_eval_boolean(&paras->torus_u))
		coord.x -= floor(coord.x);
	if (*mi_eval_boolean(&paras->torus_v))
		coord.y -= floor(coord.y);
	if (coord.x < 0 || coord.y < 0 || coord.x >= 1 || coord.y >= 1) {
		result->r = result->g = result->b = result->a = 0;
		return(miTRUE);
	} else
		return(mi_lookup_color_texture(result, state, tex, &coord));
}


/*------------------------------------------ mib_lookup_cylindrical ---------*/

/*
 * intersect ray (r, dir) in canonical cylinder space with canonical
 * cylinder, which is defined by x^2 + y^2 -r^2 = 0, r = 1, z axis
 * from [-1..+1].
 * calculate texture parameters in intersection point, z[-1..+1]
 * is mapped to v[0..1], rotation counterclockwise around z axis,
 * +x,y=+0 is mapped to u=0, +x,y=-0 is mapped to u=1. the cylinder
 * can be cut with the begin, end parameters in radians.
 */

static miBoolean
cylinder_intersect(
	miVector	*r,
	miVector	*dir,
	miVector	*tex,
	miScalar	begin,
	miScalar	end)
{
	double a, b, c, d, t1, t2, t, x, y, z;

	a = dir->x * dir->x + dir->y * dir->y;
	if (a == 0.)
		return(miFALSE);
	b = 2. * (r->x * dir->x + r->y * dir->y);
	c = r->x * r->x + r->y * r->y - 1.0;

	b /= a;
	c /= a;

	d = b * b * .25 - c;

	if (d < 0.)
		return(miFALSE);

	d = sqrt(d);
	b *= -.5;

	t1 = b + d;
	t2 = b - d;

	if (t1 < 0.)
		return(miFALSE);		/* both negative */

	/* this is true here: t1 > t2 */

	if (t2 >= 0.) {
		/* both positive: use smaller value */
		t = t1; t1 = t2;
		t2 = t;
	}

	z = r->z + t1 * dir->z;

	if (z < -1. || z > 1.) {
		if (t2 >= 0.) {
			/*
			 * ray has two positive intersections:
			 * use the farther, which is inside the
			 * infinite cylinder
			 */
			t1 = t2;
			z = r->z + t1 * dir->z;

			if (z < -1. || z > 1.)
				return(miFALSE);
		} else {
			/*
			 * ray has a negative and a positive
			 * intersection: cylinder not hit within
			 * definition
			 */
			return(miFALSE);
		}
	}

	x = r->x + t1 * dir->x;
	y = r->y + t1 * dir->y;

	if (x == 0.)
		tex->x = (y >= 0.) ? M_PI/2. : 3.*M_PI/2.;
	else {
		if (y >= 0.) {
			if (x > 0.)
				tex->x = atan(y/x);		/* 0..90 */
			else
				tex->x = M_PI - atan(y/(-x));	/* 90..180 */
		} else {
			if (x > 0.)
				tex->x = 2.*M_PI - atan(-y/x);	/* 270..360 */
			else
				tex->x = M_PI + atan(y/x);	/* 180..270 */
		}
	}

	if (!(begin == 0. && end == 0.) &&
		(tex->x < begin || tex->x > end))
		return(miFALSE);

	tex->x *= 1. / (2. * M_PI);
	tex->y = (z + 1.) * 0.5;
	tex->z = 0;

	return(miTRUE);
}

/* this structure is also defined in ivuishader.C */
struct mib_lookup_cylindrical {
	miMatrix	xform;
	miScalar	begin;
	miScalar	end;
	miTag		tex;
};

extern "C" DLLEXPORT int mib_lookup_cylindrical_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_lookup_cylindrical(
	miColor		*result,
	miState		*state,
	struct mib_lookup_cylindrical *paras)
{
	miScalar	*xform;
	miVector	r, dir;
	miScalar	begin;
	miScalar	end;
	miVector	coord;
	miTag		tex = *mi_eval_tag(&paras->tex);

	if (!tex) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}

	begin = *mi_eval_scalar(&paras->begin);
	end   = *mi_eval_scalar(&paras->end);
	xform = mi_eval_transform(&paras->xform);

	mi_point_transform(&r, &state->point, xform);
	mi_vector_transform(&dir, &state->dir, xform);
	mi_vector_normalize(&dir);

	/* intersect ray with canonical cylinder */
	if (cylinder_intersect(&r, &dir, &coord, begin, end))
		return mi_lookup_color_texture(result, state,
						tex, &coord);
	else
		return(miFALSE);		/* no intersection */
}
