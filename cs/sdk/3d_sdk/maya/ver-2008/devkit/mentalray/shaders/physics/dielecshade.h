/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	15.01.98
 * Module:	physics
 * Purpose:	prototypes for global illumination using photon map
 *
 * Exports:
 *
 * History:
 *
 * Description:
 *****************************************************************************/

struct dielectric_material {
	miColor		col;		/* absorption coefficients */
	miScalar	ior;		/* index of refraction */
	miColor		col_out;	/* outside absorption coef. for i/f */
	miScalar	ior_out;	/* outside ior for interface */
	miBoolean	ignore_normals;	/* ignore normals? */
	miScalar	pcoef;		/* phong coef. for nice highlights */
	int		i_light;	/* the lights to be sampled */
	int		n_light;	/* the lights to be sampled */
	miTag		light[1];	/* ... */
	int		mode;		/* light mode */
};

extern "C" {
DLLEXPORT int	    dielectric_material_photon_version(void);
DLLEXPORT miBoolean dielectric_material_photon	    (miColor *, miState *,
				struct dielectric_material *);
DLLEXPORT int	    dielectric_material_version	    (void);
DLLEXPORT miBoolean dielectric_material		    (miColor *, miState *,
				struct dielectric_material *);
}

