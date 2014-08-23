/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	30.10.97
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_texture_turbulence
 *	mib_texture_turbulence_init
 *	mib_texture_turbulence_exit
 *	mib_texture_turbulence_version
 *
 * History:
 *
 * Description:
 * interface to noise functions
 *****************************************************************************/

#include <math.h>
#include <assert.h>
#include <shader.h>

#ifndef IRIX
#define fsin sin
#define ftan tan
#define fsqrt sqrt
#endif


/*------------------------------------------ mib_texture_turbulence ---------*/

struct mib_texture_turbulence {
	miVector	coord;
	miScalar	spacing;
	miScalar	strength;
	miScalar	power;
	miInteger	iteration;
	miInteger	polar_dim;
};

#define NTERMS		8		/* max # of terms in spectral sum */
#define TABLE_SIZE	1024		/* noise table size */
#define TABLE_MAX	(TABLE_SIZE-1)

typedef struct {
	miScalar	weight;
	miScalar	scale;
} Term;

typedef struct {
	int		iteration;	/* number of terms in spectral sum */
	Term		terms[NTERMS];	/* terms of the spectral sum */
} Context;

static miScalar	table[TABLE_SIZE];	/* noise table */

static miScalar	getnoise(miScalar, miScalar, miScalar);


/*
 * initialize the context struct used for 3D texture evaluation. Every shader
 * instance has one, attached to the miFunction's user pointer. All shaders
 * share the noise table.
 */

extern "C" DLLEXPORT miBoolean mib_texture_turbulence_init(
	miState			*state,
	struct mib_texture_turbulence *paras,
	miBoolean		*init_req)
{
	Context			**contextp;
	register Context	*context;
	register int		i;
	miScalar		weight = 0.5;
	miScalar		scale = 1.0;
	miScalar		power;
	register miScalar	tw = 0.0;

	if (!paras) {					/* --- global init */
		static miBoolean did_init = miFALSE;	/* only once, ever */
		if (!did_init) {
			unsigned short seed[3];
			seed[0] = 0x330e;
			seed[1] = 1;
			seed[2] = 0;

			for (i=0; i < TABLE_SIZE; i++) {
				table[i] = mi_erandom(seed);
				assert(table[i] >= 0 && table[i] < 1);
			}
			did_init = miTRUE;
		}
		return(*init_req = miTRUE);
	}
							/* --- instance init */
	mi_query(miQ_FUNC_USERPTR, state, 0, (void *)&contextp);
	context = *contextp = (Context*) mi_mem_allocate(sizeof(Context));
	power = *mi_eval_scalar(&paras->power);
	if (power == 0)
		power = 1;
	i = *mi_eval_integer(&paras->iteration);
	context->iteration = i < 1 ? 2 : i < NTERMS ? i : NTERMS;
	for (i=0; i < context->iteration; i++) {
		context->terms[i].weight = weight;
		context->terms[i].scale  = scale;
		tw    += weight;
		scale += scale;
		weight = 0.5f * (float)pow(scale, -power);
	}
	for (i=0; i < context->iteration; i++)
		context->terms[i].weight /= tw;
	return(miTRUE);
}


/*
 * terminate the shader - delete the function instance context, but leave
 * the lattice intact for next time.
 */

extern "C" DLLEXPORT miBoolean mib_texture_turbulence_exit(
	miState			*state,
	struct mib_texture_turbulence *paras)
{
	if (paras) {
		Context **contextp;
		mi_query(miQ_FUNC_USERPTR, state, 0, (void *)&contextp);
		mi_mem_release(*contextp);
		*contextp = 0;
	}
	return(miTRUE);
}


/*
 * turbulence shader
 */

extern "C" DLLEXPORT int mib_texture_turbulence_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_texture_turbulence(
	miScalar	*result,
	miState		*state,
	struct mib_texture_turbulence *paras)
{
	register Context*context;
	Context		**contextp;
	miScalar	spacing   = *mi_eval_scalar (&paras->spacing);
	miScalar	strength  = *mi_eval_scalar (&paras->strength);
	miVector	*coord    =  mi_eval_vector (&paras->coord);
	miScalar	sk, sum=0, r;
	int		i;

	mi_query(miQ_FUNC_USERPTR, state, 0, (void *)&contextp);
	context = *contextp;
	if (spacing == 0)
		spacing = 1;
	if (strength == 0)
		strength = 1;

	switch(*mi_eval_integer(&paras->polar_dim)) {
	  default:
	  case 0:
		for (i=0; i < context->iteration; i++) {
			sk   = context->terms[i].scale;
			sum += context->terms[i].weight *
						getnoise(sk * coord->x,
							 sk * coord->y,
							 sk * coord->z);
		}
		if (sum > 1)
			sum = 1;
		*result = sum * sum * (3 - sum - sum);

	  case 1:
		for (i=0; i < context->iteration; i++) {
			sk = context->terms[i].scale;
			r = getnoise(sk * coord->x,
				     sk * coord->y,
				     sk * coord->z);
			sum += context->terms[i].weight * (r > .5 ? r + r - 1
								  : 1 - r - r);
		}
		r = coord->x + coord->y;
		*result = 0.5F - 0.5F * (float)sin(spacing * 2.F * M_PI_F
						   * (r + strength * sum));
		break;

	  case 2:
		for (i=0; i < context->iteration; i++) {
			sk = context->terms[i].scale;
			sum += context->terms[i].weight *
						getnoise(sk * coord->x,
							 sk * coord->y,
							 sk * coord->z);
		}
		r = fsqrt(coord->y * coord->y + coord->z * coord->z);
		*result = 0.5f - 0.5f * (float)sin(spacing * 2 * M_PI
						   * (r + strength * sum));
		break;
	}
	return(miTRUE);
}


/*
 * Compute a band-limited noise value for an (x,y,z) point.
 */

static miScalar getnoise(
	miScalar	x,
	miScalar	y,
	miScalar	z)
{
	miScalar	xx, yy, zz;
	int		ix, iy, iz, ixyz;
	miScalar	fr_x, fr_y, fr_z;
	miScalar	u_0_0, u_0_1, u_0_2;
	miScalar	u_1_0, u_1_1, u_1_2;
	miScalar	u_2_0, u_2_1, u_2_2;
	miScalar	dxyz_0_0_0, dxyz_0_0_1, dxyz_0_0_2;
	miScalar	dxyz_0_1_0, dxyz_0_1_1, dxyz_0_1_2;
	miScalar	dxyz_0_2_0, dxyz_0_2_1, dxyz_0_2_2;
	miScalar	dxyz_1_0_0, dxyz_1_0_1, dxyz_1_0_2;
	miScalar	dxyz_1_1_0, dxyz_1_1_1, dxyz_1_1_2;
	miScalar	dxyz_1_2_0, dxyz_1_2_1, dxyz_1_2_2;
	miScalar	dxyz_2_0_0, dxyz_2_0_1, dxyz_2_0_2;
	miScalar	dxyz_2_1_0, dxyz_2_1_1, dxyz_2_1_2;
	miScalar	dxyz_2_2_0, dxyz_2_2_1, dxyz_2_2_2;

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
