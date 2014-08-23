/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	09.10.97
 * Module:	physics
 * Purpose:	Global illumination with participating media using photon maps
 *
 * Exports:
 *
 * History:
 *
 * Description:
 * Global illumination volume shaders for participating media.
 *****************************************************************************/

/*
 * Parameters of the participating medium volume shader.
 * We cannot typedef parti_volume as there is also a shader by that name.
 * g1 = 0 and g2 = 0:	isotropic scattering
 * g1 <> 0 or g2 <> 0: anisotropic scattering
 * nonuniform = 0: homogeneous medium
 * nonuniform > 0: nonhomogeneous medium
 */

typedef struct {			/* shader parameters */
	int		mode;		/* 0 or 1 */
	miColor		scatter;	/* volume color */
	miScalar	extinction;	/* fog density = extinction coeff. */
	miScalar	r;		/* blending parameter (ignored) */
	miScalar	g1;		/* eccentricity for first lobe */
	miScalar	g2;		/* eccentricity for second lobe */
	miScalar	nonuniform;	/* nonhomogenous medium: cloudiness */
	miScalar	height; 	/* mode 1 only: roof of smoke */
	miScalar	min_step_len;	/* minimum step length for ray march */
	miScalar	max_step_len;	/* maximum step length for ray march */
	miScalar	light_dist;	/* distance for fast light sampling */
	int		min_level;	/* photons stored from min refr. lvl */
	miBoolean	no_globil_where_direct; /* for optimization */
	int		i_light;	/* index of first light */
	int		n_light;	/* number of lights */
	miTag		light[1];	/* list of lights */
} Paras;

struct parti_volume {			/* internal mi_eval'ed copy */
	int		mode;		/* 0 or 1 */
	miColor		scatter;	/* volume color */
	miScalar	extinction;	/* fog density = extinction coeff. */
	miScalar	r;		/* blending parameter (ignored) */
	miScalar	g1;		/* eccentricity for first lobe */
	miScalar	g2;		/* eccentricity for second lobe */
	miScalar	nonuniform;	/* nonhomogenous medium: cloudiness */
	miScalar	height; 	/* mode 1 only: roof of smoke */
	miScalar	min_step_len;	/* minimum step length for ray march */
	miScalar	max_step_len;	/* maximum step length for ray march */
	miScalar	light_dist;	/* distance for fast light sampling */
	int		min_level;	/* photons stored from min refr. lvl */
	miBoolean	no_globil_where_direct; /* for optimization */
	int		i_light;	/* index of first light */
	int		n_light;	/* number of lights */
	miTag		*light;		/* list of lights */
};

extern "C" {
DLLEXPORT miBoolean parti_volume_init	     (miState *, Paras *, miBoolean *);
DLLEXPORT miBoolean parti_volume_exit	     (miState *, Paras *, miBoolean *);
DLLEXPORT miBoolean parti_volume	     (miColor *, miState *, Paras *);
DLLEXPORT miBoolean parti_volume_photon_init (miState *, Paras *, miBoolean *);
DLLEXPORT miBoolean parti_volume_photon	     (miColor *, miState *, Paras *);
DLLEXPORT int	    parti_volume_version     (void);
DLLEXPORT int	    parti_volume_photon_version(void);
DLLEXPORT miBoolean transmat		     (miColor *, miState *, void *);
DLLEXPORT miBoolean transmat_photon	     (miColor *, miState *, void *);
DLLEXPORT int       transmat_version	     (void);
DLLEXPORT int       transmat_photon_version  (void);
}
