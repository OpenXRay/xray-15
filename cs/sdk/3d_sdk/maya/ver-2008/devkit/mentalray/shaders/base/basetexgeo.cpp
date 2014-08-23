/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	9.3.99
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_geo_add_uv_texsurf
 *	mib_geo_add_uv_texsurf_version
 *
 * History:
 *
 * Description:
 *	This shader returns a copy of the input object, which must be
 *	of freeform-surface type. It loops over all faces and adds a
 *	Bezier texture surface of degree 1 to each face as the last
 *	texture surface. The parameters and control points of the texture
 *	surfaces are chosen in such a way that there is an exact mapping
 *	of the uv parametric coordinates of the geometric approximation to
 *	the texture vertex coordinates, i.e. the texture coordinates of the
 *	triangle vertices are the uv coordinates of the triangle vertex
 *	positions.
 *	The Bezier-1 basis is always added to the copy of the object.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <shader.h>
#include <geoshader.h>

/* should rewrite this without need for integration API. Should use mi_api_. */
extern "C" void *mi_scene_create(miTag * const tag, const miScene_types type, ...);

typedef struct {
	miTag		object;
} miMIBAddTex;


/*
 * This function initializes a bezier degree 1 texture surface for
 * a geometric surface. It uses the parameter range of the face
 * for texture vertex assignment. The face is updated since a
 * surface was added.
 */

static void create_uvtexsurface(
	miSurface	*rsurf,		/* result surface */
	miFace		*face,		/* in: face */
	miSurface	*surf,		/* in: geometric surface 0 */
	miGeoScalar	*scalars,	/* surface scalars */
	miGeoIndex	scalar_idx,	/* start index for texsurf scalars */
	int		b1index)	/* basis index to be used */
{
	miGeoScalar	*ctls;
	miGeoScalar	*params;
	miUchar		offset = 1;
	miVertex_content  svi =  face->gap_vert_info;
	miVertex_content *rvi = &face->gap_vert_info;

	memcpy(&rsurf->approx, &surf->approx, sizeof(miApprox));
	memcpy(&rsurf->disp_approx, &surf->disp_approx, sizeof(miApprox));

	rsurf->no_parms[miU]  = 2;
	rsurf->no_parms[miV]  = 2;
	rsurf->no_ctls	      = 4;
	rsurf->no_specpnts    = 0;
	rsurf->scalar_idx     = scalar_idx;
	rsurf->specpnt_idx    = 0;
	rsurf->basis_idx[miU] = b1index;
	rsurf->basis_idx[miV] = b1index;
	rsurf->type	      = miSURFACE_TEXTURE_2D;
	rsurf->degree[miU]    = 1;
	rsurf->degree[miV]    = 1;
	rsurf->ctl_dim	      = 3;

	memset(rvi, 0, sizeof(miVertex_content));
	if (svi.normal_offset)
		rvi->normal_offset = offset++;
	if (svi.motion_offset)
		rvi->motion_offset = offset++;
	if (svi.derivs_offset) {
		rvi->derivs_offset = offset;
		offset += 2;
	}
	if (svi.derivs2_offset) {
		rvi->derivs2_offset = offset;
		offset += 3;
	}
	rvi->texture_offset = offset;
	rvi->no_textures = svi.no_textures + 1;
	offset += rvi->no_textures;
	if (svi.bump_offset) {
		rvi->bump_offset = offset;
		rvi->no_bumps	 = rvi->no_textures * 2;
		offset += rvi->no_textures * 2;
	}
	if (svi.user_offset) {
		rvi->user_offset = offset;
		rvi->no_users	 = svi.no_users;
		offset += rvi->no_users;
	}
	rvi->sizeof_vertex = offset;

	params = scalars + scalar_idx;

	*params++ = face->range[miU].min;
	*params++ = face->range[miU].max;
	*params++ = face->range[miV].min;
	*params   = face->range[miV].max;

	ctls = scalars + miSURFACE_CTLS(rsurf);

	*ctls++ = face->range[miU].min;
	*ctls++ = face->range[miU].max;
	*ctls++ = face->range[miU].min;
	*ctls++ = face->range[miU].max;

	*ctls++ = face->range[miV].min;
	*ctls++ = face->range[miV].min;
	*ctls++ = face->range[miV].max;
	*ctls++ = face->range[miV].max;

	*ctls++ = 0;
	*ctls++ = 0;
	*ctls++ = 0;
	*ctls   = 0;
}

/*
 * See description above.
 */

extern "C" DLLEXPORT int mib_geo_add_uv_texsurf_version(void) { return(1); }

extern "C" DLLEXPORT miBoolean mib_geo_add_uv_texsurf(
	miTag		*result,
	miState		*state,
	miMIBAddTex	*paras)
{
	miTag		src_obj_tag = *mi_eval_tag(&paras->object);
	miObject	*src_obj, *object;
	miTag		obj_tag;
	int		i, k;
	miFace_list	*fl, *src_fl;
	miFace		*faces;
	miSurface	*surfaces, *src_surfaces;
	miBasis_list	*basis_list, *src_basis_list;
	miBasis		*bbasis;
	miGeoScalar	*surf_scalars, *src_surf_scalars;
	int		cur_surf=0, add_scalars, b1index, sbase;

	src_obj = (miObject*)mi_db_access(src_obj_tag);
	if (src_obj->type != miOBJECT_FACES) {
		mi_error("mib_geo_add_uv_texsurf: wrong input object type");
		mi_db_unpin(src_obj_tag);
		return(miFALSE);
	}
	src_fl = &src_obj->geo.face_list;
	if (!src_fl->faces || !src_fl->surfaces || !src_fl->basis_list ||
	    !src_fl->surf_scalars) {
		mi_error("mib_geo_add_uv_texsurf: invalid object");
		mi_db_unpin(src_obj_tag);
		return(miFALSE);
	}

	/* create object */
	object = (miObject*)mi_scene_create(&obj_tag, miSCENE_OBJECT);
	memcpy(object, src_obj, sizeof(miObject));
	fl = &object->geo.face_list;

	/* copy constant geometry */
	if (src_fl->curves)
		fl->curves = mi_db_copy(src_fl->curves);
	if (src_fl->specpnts)
		fl->specpnts = mi_db_copy(src_fl->specpnts);
	if (src_fl->curve_scalars)
		fl->curve_scalars = mi_db_copy(src_fl->curve_scalars);

	/* some fields in each face will be edited later below */
	fl->faces = mi_db_copy(src_fl->faces);

	/* ----- one basis added at the end */
	src_basis_list = (miBasis_list*)mi_db_access(src_fl->basis_list);
	basis_list = (miBasis_list*)mi_scene_create(&fl->basis_list,
						miSCENE_BASIS_LIST,
						src_basis_list->no_bases + 1,
						src_basis_list->no_scalars);
	b1index = src_basis_list->no_bases;
	memcpy(basis_list->bases, src_basis_list->bases,
		src_basis_list->no_bases * sizeof(miBasis) +
		src_basis_list->no_scalars * sizeof(miGeoScalar));
	mi_db_unpin(src_fl->basis_list);
	bbasis = &basis_list->bases[b1index];
	bbasis->type = miBASIS_BEZIER;
	bbasis->degree = 1;
	mi_scene_edit_end(fl->basis_list);

	/* ----- several texture surfaces added (one for each face) */
	src_surfaces = (miSurface*)mi_db_access(src_fl->surfaces);
	surfaces = (miSurface*)mi_scene_create(&fl->surfaces,
						miSCENE_SURFACE,
						src_fl->no_surfaces +
						src_fl->no_faces);
	fl->no_surfaces += src_fl->no_faces;

	/* ----- several scalars for the texsurfaces added at the end */
	/* 4 parameter scalars + 4 3d control point scalar vectors */
	add_scalars = fl->no_faces * (4 + 4 * 3);
	src_surf_scalars = (miGeoScalar*)mi_db_access(src_fl->surf_scalars);
	surf_scalars = (miGeoScalar*)mi_scene_create(&fl->surf_scalars,
						miSCENE_GEOSCALAR,
						src_fl->no_surf_scalars +
						add_scalars);
	memcpy(surf_scalars, src_surf_scalars,
		sizeof(miGeoScalar) * src_fl->no_surf_scalars);
	mi_db_unpin(src_fl->surf_scalars);
	/* base index for new scalars */
	sbase = src_fl->no_surf_scalars;
	fl->no_surf_scalars += add_scalars;

	faces = (miFace*)mi_scene_edit(fl->faces);

	for (i=0; i < src_fl->no_faces; i++) {
		miFace *face = &faces[i];
		/* copy original surfaces */
		int f1 = cur_surf;
		for (k=0; k < face->no_surfaces; k++) {
			memcpy(&surfaces[cur_surf++],
			       &src_surfaces[face->surface_idx+k],
			       sizeof(miSurface));
		}
		face->surface_idx = f1;
		/* create a texture surface for the first (geometric)
		   surface */
		create_uvtexsurface(&surfaces[cur_surf++],	/* result */
				face,				/* modify */
				&surfaces[face->surface_idx],	/* input  */
				surf_scalars,			/* result */
				sbase+i*(4+4*3),		/* input  */
				b1index);			/* input  */
		face->no_surfaces += 1;
	}

	mi_scene_edit_end(fl->faces);
	mi_scene_edit_end(fl->surf_scalars);
	mi_scene_edit_end(fl->surfaces);
	mi_db_unpin(src_fl->surfaces);

	mi_scene_edit_end(obj_tag);
	mi_db_unpin(src_obj_tag);

	*result = obj_tag;
	return(miTRUE);
}
