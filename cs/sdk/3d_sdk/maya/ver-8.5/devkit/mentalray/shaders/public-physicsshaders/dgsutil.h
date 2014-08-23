/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	26.01.2001
 * Module:	physics
 * Purpose:	dgs material util functions headers
 *
 * Exports:	(None)
 *
 * History:
 *	26.01.01: created
 *
 * Description:
 *      Utility function headers for dgs_material / dgs_material_photon shader.
 *
 *****************************************************************************/


#define miEPS 0.0001

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif

/*
 * Calculate the index of refraction of the incoming (state->ior_in) and the
 * outgoing (state->ior) ray, relative to the current intersection. This is
 * simple and not accurate procedure, for example, children of backside
 * final gather rays are not treated correctly.
 */
void dgs_refraction_index(
	miState			*state,
	struct dgs_material	*m);

/*
 * Orientation for unisotropic glossy reflection / transmission.
 * First derivation should be defined for this to work correct.
 */
void dgs_anis_orientation(
	miVector	*u,		/* direction of brushing */
	miVector	*v,		/* perpendicular direction */
	miState		*state);	/* ray tracer state */

/*
 * Compute a random direction around the mirror direction
 *  using Ward's BRDF model for importance sampling
 */
void dgs_reflect_glossy_dir(
        miState 	*state,
	miVector 	*dir,
	struct dgs_material *m);

/*
 * Compute a random direction around transmission direction
 * using Ward's BRDF model for importance sampling.
 */
miBoolean dgs_transmit_glossy_dir(
	miState 	*state,
	miVector 	*dir,
	struct dgs_material *m);


/*
 * Computes Ward isotropic / anisotropic glossy distribution value,
 * for gloosy reflections and transmissions. This function may be
 * used with for backside with both unchanged and flipped state->normal.
 */
miScalar dgs_ward_glossy(
	miVector	*in,	/* incidence direction */
	miVector	*out,   /* reflection direction */
	miState		*state,
	struct dgs_material *m);


/*
 * Create local copy of parameters common for dgs_material and
 * dgs_material_photon (the later does not need lights).
 */
void dgs_set_parameters(
        miState			*state,		/* state to use */
	struct dgs_material	*m,		/* fill this local copy */
        struct dgs_material	*paras);	/* mi_eval this */
