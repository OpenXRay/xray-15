/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_lookup
 *	mib_texture_lookup2
 *	mib_texture_lookup_version
 *	mib_texture_filter_lookup
 *	mib_texture_filter_lookup_version
 *
 * History:
 *	19.11.97: filtered texture lookups
 *
 * Description:
 * texture image lookups.
 *****************************************************************************/

#include <math.h>
#include <shader.h>


/*------------------------------------------ mib_texture_lookup -------------*/

struct mib_texture_lookup {
	miTag		tex;
	miVector	coord;
};

extern "C" DLLEXPORT int mib_texture_lookup_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_lookup(
	miColor		*result,
	miState		*state,
	struct mib_texture_lookup *paras)
{
	miTag		tex   = *mi_eval_tag(&paras->tex);
	miVector	*coord = mi_eval_vector(&paras->coord);

	if (tex && coord->x >= 0 && coord->x < 1
		&& coord->y >= 0 && coord->y < 1
		&& mi_lookup_color_texture(result, state, tex, coord))

		return(miTRUE);

	result->r = result->g = result->b = result->a = 0;
	return(miFALSE);
}

/*------------------------------------------ mib_texture_lookup2 -------------*/

struct mib_texture_lookup2 {
    miTag	tex;			/* The texture to lookup */
    miScalar	factor;			/* Repeation factor */
};

/*-----------------------------------------------------------------------------
 * This texture lookup takes the first texture coordinates on the object,
 * apply a multiplication 'factor' on those coordinates
 *---------------------------------------------------------------------------*/
extern "C" DLLEXPORT int mib_texture_lookup2_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_lookup2(
    miColor		*result,
    miState		*state,
    struct mib_texture_lookup2 *paras)
{
    miTag	tex   = *mi_eval_tag(&paras->tex);
    miVector	coord;
    miScalar	factor = *mi_eval_scalar(&paras->factor);

    if (tex )
    {
	coord.x = state->tex_list[0].x * factor;
	coord.y = state->tex_list[0].y * factor;
	mi_lookup_color_texture(result, state, tex, &coord);
	return(miTRUE);
    }
	

    result->r = result->g = result->b = result->a = 0;
    return(miFALSE);
}

/*------------------------------------------ mib_texture_filter_lookup ------*/

struct mib_texture_filter_lookup {
	miTag		tex;
	miVector	coord;
	miScalar	eccmax;
	miScalar	maxminor;
	miScalar	disc_r;
	miBoolean	bilinear;
	miUint		space;
	miTag		remap;
};

extern "C" DLLEXPORT int mib_texture_filter_lookup_version(void) {return(3);}

#define DISC_R		0.3F	/* projection matrix circle radius */
#define CIRCLE_R	0.8F	/* projected screen-space circle */

extern "C" DLLEXPORT miBoolean mib_texture_filter_lookup(
	miColor		*result,
	miState		*state,
	struct mib_texture_filter_lookup *paras)
{
	miTag		tex = *mi_eval_tag(&paras->tex);
	miVector	*coord;
	miUint		space;
	miTag		remap;
	miVector	p[3], t[3];
	miMatrix	ST;
	miTexfilter	ell_opt;
	miScalar	disc_r;

	if (!tex) {
		result->r = result->g = result->b = result->a = 0;
		return(miFALSE);
	}
	coord  = mi_eval_vector(&paras->coord);
	space  = *mi_eval_integer(&paras->space);
	disc_r = *mi_eval_scalar(&paras->disc_r);
	if (disc_r <= 0)
		disc_r = DISC_R;
	if (state->reflection_level == 0 &&
	    mi_texture_filter_project(p, t, state, disc_r, space) &&
	    (remap = *mi_eval_tag(&paras->remap))) {
		mi_call_shader_x((miColor*)&t[0], miSHADER_TEXTURE,
						state, remap, &t[0]);
		mi_call_shader_x((miColor*)&t[1], miSHADER_TEXTURE,
						state, remap, &t[1]);
		mi_call_shader_x((miColor*)&t[2], miSHADER_TEXTURE,
						state, remap, &t[2]);
		if (mi_texture_filter_transform(ST, p, t)) {
			ell_opt.eccmax	  = *mi_eval_scalar(&paras->eccmax);
			ell_opt.max_minor = *mi_eval_scalar(&paras->maxminor);
			ell_opt.bilinear  = *mi_eval_boolean(&paras->bilinear);
			ell_opt.circle_radius = CIRCLE_R;
			/*
			 * when no bump-mapping is used, coord and ST[..]
			 * are identical. for bump mapping, the projection
			 * matrix is calculated for the current raster
			 * position, the ellipse is translated to the
			 * bump position
			 */
			ST[2*4+0] = coord->x;
			ST[2*4+1] = coord->y;
			if (mi_lookup_filter_color_texture(result, state,
							tex, &ell_opt, ST))
				return(miTRUE);
		}
	}
	/* fallback to standard pyramid or nonfiltered texture lookup */
	return(mi_lookup_color_texture(result, state, tex, coord));
}
