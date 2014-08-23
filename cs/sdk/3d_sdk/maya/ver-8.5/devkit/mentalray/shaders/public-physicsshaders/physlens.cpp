/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	7.4.99
 * Module:	physics
 * Purpose:	lens depth of field shader
 *
 * Exports:
 *      physical_lens_dof_version
 *      physical_lens_dof
 *
 * Description:
 *	Depth of field effect with lens oversampling.
 *
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <shader.h>

static miColor black = {0.0, 0.0, 0.0, 0.0};

/*
 * New depth of field shader using low discrepancy sampling for the
 * simulation of depth of field. It does supersampling of the lens before
 * the  ordinary oversampling. A fixed number of four samples is used.
 */

struct dof {
	miScalar	plane;		/* distance to focused plane */
	miScalar	radius;		/* size of the lens */
};

#define NO_SAMPLES 4

extern "C" DLLEXPORT int physical_lens_dof_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean physical_lens_dof(
	register miColor	*result,
	register miState	*state,
	register struct dof	*paras)
{
	miScalar	plane = *mi_eval_scalar(&paras->plane);
	miScalar	rads  = *mi_eval_scalar(&paras->radius);
	miVector	org, dir;	/* original state->org, state->dir */
	miVector	rayorg, raydir;	/* ray origin and direction */
	miVector	dest;
	double		t, radius, angle;
	miColor		average;
	int		i;
	double		pad[2];
	miUint		n;		/* number of eye rays to shoot */
	int		dummy[4];	/* for tile size query */


	average = black;
	mi_point_to_camera (state, &org, &state->org);
	mi_vector_to_camera(state, &dir, &state->dir);
	t = (plane - org.z) / dir.z;
	dest.x = org.x + t * dir.x;
	dest.y = org.y + t * dir.y;
	dest.z = plane;

	i = 0;
	
	/* 
	 * In the finalgather precomputing stage multiple eye rays are 
	 * useless. To detect that shader is called im the finalgather
	 * precomputing mode, the fact that there are no "tiles" and thus
	 * mi_query(miQ_TILE_PIXELS) returns miFALSE is used.
	 */
	if (state->options->finalgather &&
	    !mi_query(miQ_TILE_PIXELS, state, 0, dummy)) 
		n = 1;
	else
		n = NO_SAMPLES;

	while(mi_sample(pad, &i, state, 2, &n)) {
		radius = rads * sqrt(pad[0]);
		angle = 2.0 * M_PI * pad[1];

		rayorg.x = org.x + radius * cos(angle);
		rayorg.y = org.y + radius * sin(angle);
		rayorg.z = org.z;
		raydir.x = dest.x - rayorg.x;
		raydir.y = dest.y - rayorg.y;
		raydir.z = dest.z - rayorg.z;
		mi_point_from_camera (state, &rayorg, &rayorg);
		mi_vector_from_camera(state, &raydir, &raydir);

		state->pri = NULL;	/* for chained lens shaders */

		(void)mi_trace_eye(result, state, &rayorg, &raydir);

		average.r += result->r;
		average.g += result->g;
		average.b += result->b;
		average.a += result->a;
        }
	result->r = average.r / NO_SAMPLES;
	result->g = average.g / NO_SAMPLES;
	result->b = average.b / NO_SAMPLES;
	result->a = average.a / NO_SAMPLES;
	return(miTRUE);
}
