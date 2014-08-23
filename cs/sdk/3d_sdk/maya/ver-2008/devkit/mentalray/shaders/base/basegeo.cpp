/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	27.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_geo_instance
 *	mib_geo_instance_version
 *	mib_geo_instance_mlist
 *	mib_geo_instance_mlist_version
 *	mib_geo_sphere
 *	mib_geo_sphere_version
 *	mib_geo_cone
 *	mib_geo_cone_version
 *	mib_geo_cylinder
 *	mib_geo_cylinder_version
 *	mib_geo_cube
 *	mib_geo_cube_version
 *	mib_geo_square
 *	mib_geo_square_version
 *      mib_geo_torus
 *      mib_geo_torus_version
 *
 * History:
 *	19.11.97: added mib_geo_instance_mlist
 *      02.07.01: torusd added.
 *
 * Description:
 *	This module provides geometry shaders for the simple shapes listed
 *	above. All geometry is created with polygons, and normals are
 *	generated when required.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <shader.h>
#include <geoshader.h>

#define DEFSUB_U	8	/* default u subdivisions */
#define DEFSUB_V	8	/* default v subdivisions */


static void add_vector(miScalar	a, miScalar b, miScalar c)
{
	miVector v;
	v.x=a; v.y=b; v.z=c;
	mi_api_vector_xyz_add(&v);
}

static void add_triangle(int a, int b, int c)
{
	mi_api_poly_begin_tag(1, 0);
	mi_api_poly_index_add(a);
	mi_api_poly_index_add(b);
	mi_api_poly_index_add(c);
	mi_api_poly_end();
}

static void init_object_flags(miObject * object)
{
	object->visible = miTRUE;
	object->shadow     =
	object->reflection =
	object->refraction =
	object->finalgather= 0x03;
}

/*
 * instance geometry shader
 */

typedef struct {
	miTag		object;
	miMatrix	matrix;
	miTag		material;
} miMIBInstance;


extern "C" DLLEXPORT int mib_geo_instance_version(void) { return(1); }

extern "C" DLLEXPORT miBoolean mib_geo_instance(
	miTag		*result,
	miState		*state,
	miMIBInstance	*paras)
{
	static int	namecounter;	/* geoshaders are single-threaded */
	char		namebuf[64];
	miInstance	*instance;
	miTag		object   = *mi_eval_tag(&paras->object);
	miTag		material = *mi_eval_tag(&paras->material);

	sprintf(namebuf, "geoinstance_mib_%d", namecounter++);

	if (!(instance = mi_api_instance_begin(mi_mem_strdup(namebuf))))
		return(miFALSE);

	mi_matrix_copy(instance->tf.global_to_local,
		       mi_eval_transform(&paras->matrix));
	mi_matrix_invert(instance->tf.local_to_global,
			 instance->tf.global_to_local);
	instance->material = mi_phen_clone(state, material);

	return(mi_geoshader_add_result(result,
			mi_api_instance_end(0, object, 0)));
}


/*
 * instance geometry shader with material array, which gets turned into
 * an instance material list if it has more than one array member.
 */

typedef struct {
	miTag		object;
	miMatrix	matrix;
	int		i_material;
	int		n_material;
	miTag		material[1];
} miMIBInstanceML;


extern "C" DLLEXPORT int mib_geo_instance_mlist_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_instance_mlist(
	miTag		*result,
	miState		*state,
	miMIBInstanceML	*paras)
{
	static int	namecounter;	/* geoshaders are single-threaded */
	char		namebuf[64];
	miInstance	*instance;
	miTag		object     = *mi_eval_tag    (&paras->object);
	miTag		*material  =  mi_eval_tag    (&paras->material);
	int		i_material = *mi_eval_integer(&paras->i_material);
	int		n_material = *mi_eval_integer(&paras->n_material);
	int		i;

	sprintf(namebuf, "geoinstance_m_mib_%d", namecounter++);

	if (!(instance = mi_api_instance_begin(mi_mem_strdup(namebuf))))
		return(miFALSE);

	mi_matrix_copy(instance->tf.global_to_local,
		       mi_eval_transform(&paras->matrix));
	mi_matrix_invert(instance->tf.local_to_global,
			 instance->tf.global_to_local);
	if (n_material == 1)
		instance->material = mi_phen_clone(state,
						   material[i_material]);

	else if (n_material > 1) {
		miDlist *list = mi_api_dlist_create(miDLIST_TAG);
		for (i=0; i < n_material; i++)
			mi_api_dlist_add(list,
					(void *)(long)mi_phen_clone(state,
						material[i_material+i]));
		instance->mtl_array_size = n_material;
		instance->material = mi_api_taglist(list);
		mi_api_dlist_delete(list);
	}
	return(mi_geoshader_add_result(result,
			mi_api_instance_end(0, object, 0)));
}


/*
 * polygonal torus geometry shader
 * creates a torus with inner radius and thickness.
 */

typedef struct {
	miScalar	radius;
	miScalar	thickness;
	miInteger	uSpans;
	miInteger	vSpans;
} miMIBTorus;


extern "C" DLLEXPORT int mib_geo_torus_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_torus(

	miTag		*result,
	miState		*state,
	miMIBTorus	*parms)
{
	miScalar	radius = *mi_eval_scalar(&parms->radius);
	miScalar	thickness = *mi_eval_scalar(&parms->thickness);
	miInteger	uSpans = *mi_eval_integer(&parms->uSpans);
	miInteger	vSpans = *mi_eval_integer(&parms->vSpans);
	miVector	vector;
	int		u, v, numVectors;
	miObject	*object;

	if (radius <= 0.0)
		radius = 2.0;
	if (thickness <= 0.0)
		thickness = 1.0;
	if (!uSpans)
		uSpans = 32;
	if (!vSpans)
		vSpans = 32;

	/* Here we initialize our torus. */
	object = mi_api_object_begin(NULL);
	init_object_flags(object);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	/* Create torus vertices + normals. */
	for (v=0, numVectors=0; v < vSpans; v++) {
		double vAngle = 2.0*M_PI*((double) v/vSpans);
		double thicknessCosVAngle = thickness*cos(vAngle);
		for (u=0; u < uSpans; u++,numVectors++) {
			double uAngle = 2.0*M_PI*((double) u/uSpans);
			vector.x  = cos(uAngle)*(radius+thicknessCosVAngle);
			vector.y  = sin(uAngle)*(radius+thicknessCosVAngle);
			vector.z  = sin(vAngle)*thickness;
			mi_api_vector_xyz_add(&vector);
			vector.x *= thicknessCosVAngle;
			vector.y *= thicknessCosVAngle;
			vector.z *= radius+thicknessCosVAngle;
			mi_api_vector_xyz_add(&vector);
		}
	}
	for (v=0; v<numVectors; v++) {
		mi_api_vertex_add(2*v);
		mi_api_vertex_normal_add(2*v+1);
	}

	/* Create faces. */
	for (v=0; v < vSpans; v++)
		for (u=0; u<uSpans; u++) {
			int p1 = u              + uSpans*v;
			int p2 = ((u+1)%uSpans) + uSpans*v;
			int p3 = ((u+1)%uSpans) + uSpans*((v+1)%vSpans);
			int p4 = u              + uSpans*((v+1)%vSpans);
			mi_api_poly_begin_tag(1, 0);
			mi_api_poly_index_add(p1);
			mi_api_poly_index_add(p2);
			mi_api_poly_index_add(p3);
			mi_api_poly_index_add(p4);
			mi_api_poly_end();
		}

	/* Finish da shit. */
	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}


/*
 * polygonal sphere geometry shader
 * creates origin centered unit sphere, v_subdiv subdivisions
 * in z axis (longitude), u_subdiv in xy plane (latitude)
 */

typedef struct {
	miInteger	u_subdiv;
	miInteger	v_subdiv;
} miMIBSphere;


extern "C" DLLEXPORT int mib_geo_sphere_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_sphere(
	miTag		*result,
	miState		*state,
	miMIBSphere	*paras)
{
	miInteger	u_sub = *mi_eval_integer(&paras->u_subdiv);
	miInteger	v_sub = *mi_eval_integer(&paras->v_subdiv);
	miVector	v;
	miInteger	i, j, nv=0;
	miScalar	p, r;
	miObject	*obj;

	if (u_sub < 3)
		u_sub = DEFSUB_U;
	if (v_sub < 1)
		v_sub = DEFSUB_V;

	obj = mi_api_object_begin(NULL);
	init_object_flags(obj);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	for (i=1; i <= v_sub; i++) {
		v.z = -(miScalar)cos(i*M_PI/(v_sub+1));
		r   = (miScalar)sqrt(1.-v.z*v.z);
		for (j=0; j < u_sub; j++, nv++) {
			p = j*2.F*M_PI_F/u_sub;
			v.x = r * cos(p);
			v.y = r * sin(p);
			mi_api_vector_xyz_add(&v);	/* point */
			mi_api_vector_xyz_add(&v);	/* normal */
		}
	}

	add_vector(0, 0, -1);	/* south pole */
	add_vector(0, 0, -1);

	add_vector(0, 0, +1);	/* north pole */
	add_vector(0, 0, +1);

	for (i=0; i < nv; i++) {
		mi_api_vertex_add(i*2);
		mi_api_vertex_normal_add(i*2+1);
	}

	mi_api_vertex_add(nv*2+0);
	mi_api_vertex_normal_add(nv*2+1);

	mi_api_vertex_add(nv*2+2);
	mi_api_vertex_normal_add(nv*2+3);

	/* southpole */
	for (j=0; j < u_sub; j++)
		add_triangle(nv, (j+1)%u_sub, j);

	/* northpole */
	for (j=0; j < u_sub; j++)
		add_triangle(nv+1, nv-u_sub+j, nv-u_sub+((j+1)%u_sub));

	for (i=0; i < v_sub-1; i++) {
		for (j=0; j < u_sub; j++) {
			int p1 = i*u_sub+j;
			int p2 = i*u_sub+((j+1)%u_sub);
			int p3 = (i+1)*u_sub+((j+1)%u_sub);
			int p4 = (i+1)*u_sub+j;
			add_triangle(p1, p2, p4);
			add_triangle(p2, p3, p4);
		}
	}
	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}


/*
 * polygonal cone geometry shader
 * creates origin centered unit cone around z-axis,
 * bottom at z=-1, apex at z=0, bottom size 2 units.
 * the bottom disc is subdivided in u_subdiv segments,
 * the z-axis side is subdivided in v_subdiv segments.
 */

typedef struct {
	miInteger	u_subdiv;
	miInteger	v_subdiv;
	miBoolean	capped;
} miMIBCone;


extern "C" DLLEXPORT int mib_geo_cone_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_cone(
	miTag		*result,
	miState		*state,
	miMIBCone	*paras)
{
	miInteger	u_sub = *mi_eval_integer(&paras->u_subdiv);
	miInteger	v_sub = *mi_eval_integer(&paras->v_subdiv);
	miBoolean	capped= *mi_eval_boolean(&paras->capped);
	miVector	v, w = { 2, 0, 1};
	miInteger	i, j, apex, normals;
	miScalar	r, p;
	miMatrix	tm;
	miObject	*obj;

	if (u_sub < 3)
		u_sub = DEFSUB_U;
	if (v_sub < 0)
		v_sub = DEFSUB_V;

	obj = mi_api_object_begin(NULL);
	init_object_flags(obj);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	for (i=0; i <= v_sub; i++) {
		v.z = -1.F + i/(v_sub+1.F);
		r   =  1.F - i/(v_sub+1.F);
		for (j=0; j < u_sub; j++) {
			p = j*2.F*M_PI_F/u_sub;
			v.x = r * cos(p);
			v.y = r * sin(p);
			mi_api_vector_xyz_add(&v);
		}
	}

	normals = (v_sub+1)*u_sub;

	for (j=0; j < u_sub; j++) {
		p = j*2.F*M_PI_F/u_sub;
		mi_matrix_ident(tm);
		tm[0*4+0] =  cos(p);	/* z rotation(p) */
		tm[0*4+1] =  sin(p);
		tm[1*4+0] = -sin(p);
		tm[1*4+1] =  cos(p);
		mi_vector_transform(&v,&w,tm);
		mi_vector_normalize(&v);
		mi_api_vector_xyz_add(&v);
	}

	apex = normals + u_sub;		/* apex index */

	add_vector(0, 0, 0);		/* apex point */

	if (capped) {
		add_vector(0, 0, -1);	/* apex+1: bottom point */
		add_vector(0, 0, -1);	/* apex+2: bottom normal */
	}

	/* cone outer side rings */
	for (i=0; i <= v_sub; i++)
		for (j=0; j < u_sub; j++) {
			mi_api_vertex_add(i*u_sub+j);
			mi_api_vertex_normal_add(normals+j);
		}

	/* cone top ring */
	for (j=0; j < u_sub; j++) {
		mi_api_vertex_add(apex);/* shared apex point */
		mi_api_vertex_normal_add(normals+j);
	}

	if (capped) {
		int disc = (v_sub+1)*u_sub + u_sub;

		/* cone bottom disc */
		for (j=0; j < u_sub; j++) {
			mi_api_vertex_add(j);
			mi_api_vertex_normal_add(apex+2);
		}

		/* disc center */
		mi_api_vertex_add(apex+1);
		mi_api_vertex_normal_add(apex+2);

		for (j=0; j < u_sub; j++)
			add_triangle(disc+u_sub, disc+(j+1)%u_sub, disc+j);
	}

	for (i=0; i <= v_sub; i++)
		for (j=0; j < u_sub; j++) {
			int p1 = i*u_sub+j;
			int p2 = i*u_sub+(j+1)%u_sub;
			int p3 = (i+1)*u_sub+(j+1)%u_sub;
			int p4 = (i+1)*u_sub+j;
			add_triangle(p1, p2, p4);
			add_triangle(p2, p3, p4);
		}

	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}


/*
 * polygonal cylinder geometry shader
 * creates origin centered polygonal cylinder around z-axis,
 * bottom in z=-1, top in z=0, radius=1. u_subdiv subdivisions
 * in the xy plane, v_subdiv in the z axis direction.
 */

typedef struct {
	miInteger	u_subdiv;
	miInteger	v_subdiv;
	miBoolean	bottom_capped;
	miBoolean	top_capped;
} miMIBCylinder;


extern "C" DLLEXPORT int mib_geo_cylinder_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_cylinder(
	miTag		*result,
	miState		*state,
	miMIBCylinder	*paras)
{
	miInteger	u_sub = *mi_eval_integer(&paras->u_subdiv);
	miInteger	v_sub = *mi_eval_integer(&paras->v_subdiv);
	miBoolean	bot_cap = *mi_eval_boolean(&paras->bottom_capped);
	miBoolean	top_cap = *mi_eval_boolean(&paras->top_capped);
	miVector	v;
	miInteger	i, j, nv=0, index, bot_index, top_index;
	miInteger	bot_disc, top_disc, normal_offset=0;
	miScalar	p;
	miObject	*obj;

	if (u_sub < 3)
		u_sub = DEFSUB_U;
	if (v_sub < 0)
		v_sub = DEFSUB_V;

	obj = mi_api_object_begin(NULL);
	init_object_flags(obj);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	for (i=0; i <= v_sub+1; i++) {
		v.z = -1.F + i/(v_sub+1.);
		for (j=0; j < u_sub; j++, nv++) {
			p = j*2.F*M_PI_F/u_sub;
			v.x = cos(p);
			v.y = sin(p);
			mi_api_vector_xyz_add(&v);
		}
	}

	v.z = 0;
	for (j=0; j < u_sub; j++) {
		p = j*2.F*M_PI_F/u_sub;
		v.x = cos(p);
		v.y = sin(p);
		mi_api_vector_xyz_add(&v);
	}
	normal_offset = u_sub;

	index = nv + normal_offset;

	if (bot_cap) {
		add_vector(0, 0, -1);	/* bottom disc center */
		add_vector(0, 0, -1);	/* bottom disc normal */
		bot_index = index;
		index += 2;
	}

	if (top_cap) {
		add_vector(0, 0, 0);	/* top disc center */
		add_vector(0, 0, 1);	/* top disc normal */
		top_index = index;
		index += 2;
	}

	/* cylinder part vertices */
	for (i=0; i <= v_sub+1; i++)
		for (j=0; j < u_sub; j++) {
			mi_api_vertex_add(i*u_sub+j);
			mi_api_vertex_normal_add(nv+j);
		}

	/* bottom cap vertices */
	if (bot_cap) {
		bot_disc = (v_sub+2)*u_sub;

		for (j=0; j < u_sub; j++) {
			mi_api_vertex_add(j);
			mi_api_vertex_normal_add(bot_index+1);
		}

		/* disc center */
		mi_api_vertex_add(bot_index);
		mi_api_vertex_normal_add(bot_index+1);
	}

	/* top cap vertices */
	if (top_cap) {
		top_disc = (v_sub+2)*u_sub;

		if (bot_cap)
			top_disc += u_sub + 1;

		for (j=0; j < u_sub; j++) {
			mi_api_vertex_add(nv-u_sub+j);
			mi_api_vertex_normal_add(top_index+1);
		}

		/* disc center */
		mi_api_vertex_add(top_index);
		mi_api_vertex_normal_add(top_index+1);
	}

	/* bottom cap triangles */
	if (bot_cap)
		for (j=0; j < u_sub; j++)
			add_triangle(bot_disc+u_sub,
				     bot_disc+(j+1)%u_sub, bot_disc+j);

	/* top cap triangles */
	if (top_cap)
		for (j=0; j < u_sub; j++)
			add_triangle(top_disc+u_sub,
				     top_disc+j, top_disc+(j+1)%u_sub);

	/* cylinder part triangles */
	for (i=0; i <= v_sub; i++)
		for (j=0; j < u_sub; j++) {
			int p1 = i*u_sub+j;
			int p2 = i*u_sub+(j+1)%u_sub;
			int p3 = (i+1)*u_sub+(j+1)%u_sub;
			int p4 = (i+1)*u_sub+j;
			add_triangle(p1, p2, p4);
			add_triangle(p2, p3, p4);
		}

	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}


/*
 * polygonal cube geometry shader
 * creates origin centered unit cube, volume 1
 */

extern "C" DLLEXPORT int mib_geo_cube_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_cube(
	miTag		*result,
	miState		*state,
	void		*paras)
{
	int		i, k;
	miObject	*obj;
	miVector	v[] = {{-.5,-.5,-.5},{-.5, .5,-.5},
			       { .5, .5,-.5},{ .5,-.5,-.5}};
	int		poly[][4] = {{0,3,7,4},{3,2,6,7},{2,1,5,6},
				     {1,0,4,5},{0,1,2,3},{4,7,6,5}};

	obj = mi_api_object_begin(NULL);
	init_object_flags(obj);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	for (i=0; i < 4; i++)
		add_vector(v[i].x, v[i].y, v[i].z);
	for (i=0; i < 4; i++)
		add_vector(v[i].x, v[i].y, -v[i].z);
	for (i=0; i < 8; i++)
		mi_api_vertex_add(i);
	for (i=0; i < 6; i++) {
		mi_api_poly_begin_tag(1, 0);
		for (k=0; k < 4; k++)
			mi_api_poly_index_add(poly[i][k]);
		mi_api_poly_end();
	}

	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}


/*
 * polygonal square geometry shader
 * creates origin centered rectangle with area 1
 */

extern "C" DLLEXPORT int mib_geo_square_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_geo_square(
	miTag		*result,
	miState		*state,
	void		*paras)
{
	int		i;
	miObject	*obj;
	miVector	v[] = {{-.5,-.5, 0},{ .5,-.5, 0},
			       { .5, .5, 0},{-.5, .5, 0}};

	obj = mi_api_object_begin(NULL);
	init_object_flags(obj);
	mi_api_basis_list_clear();
	mi_api_object_group_begin(0.0);

	for (i=0; i < 4; i++)
		add_vector(v[i].x, v[i].y, v[i].z);
	for (i=0; i < 4; i++)
		mi_api_vertex_add(i);
	mi_api_poly_begin_tag(1, 0);
	for (i=0; i < 4; i++)
		mi_api_poly_index_add(i);
	mi_api_poly_end();

	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end()));
}
