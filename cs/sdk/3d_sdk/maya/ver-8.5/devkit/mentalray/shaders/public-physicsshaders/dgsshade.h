/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	23.4.96
 * Module:	physics
 * Purpose:	diffuse-glossy-specular reflection models.
 *
 * Exports:
 *
 * History:
 *
 * Description:
 * Material shader and photon shader with the following reflection types:
 *     - Diffuse (Lambertian)
 *     - Glossy (Ward model, isotropic and anisotropic)
 *     - Specular (ideal mirror)
 * The material shader dgs_material() is used for rays originating from
 * the eye/camera.
 * The photon shader dgs_material_photon() is used to trace photons from
 * the light sources.
 *****************************************************************************/

struct dgs_material {
	miColor		diffuse;	/* diffuse color */
	miColor		glossy;	        /* glossy color */
	miColor		specular;	/* specular color */
	miScalar	shiny;		/* glossy narrowness (decay) */
	miScalar	shiny_u;        /* glossiness for anis. glossy refl. */
	miScalar	shiny_v;        /* (same as above) */
	miScalar	transp;		/* transparency */
	miScalar	ior;		/* index of refraction */
	int		i_light;	/* index of first light */
	int		n_light;	/* number of lights */
	miTag		light[1];	/* list of lights */
};

extern "C" {
DLLEXPORT int       dgs_material_version(void);
DLLEXPORT miBoolean dgs_material(miColor *, miState *, struct dgs_material *);
DLLEXPORT miBoolean dgs_material_photon(miColor *, miState *,
							struct dgs_material *);
DLLEXPORT int       dgs_material_photon_version(void);
}
