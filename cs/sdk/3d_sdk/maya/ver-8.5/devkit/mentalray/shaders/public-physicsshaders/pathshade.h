/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	23.4.96
 * Module:	physics
 * Purpose:	Global illumination using path tracing
 *
 * Exports:
 *
 * History:
 *
 * Description:
 * A prototype of a global illumination program using lens shaders
 * and material shaders to do path tracing.  No refraction and no textures
 * for now.
 *****************************************************************************/

/* Looking for dgs_material?  It is defined in dgsshade.h */
extern "C" {
DLLEXPORT int	    oversampling_lens_version	(void);
DLLEXPORT miBoolean oversampling_lens		(miColor *, miState *, int *);
DLLEXPORT int	    oversampling_lens_jitter_version(void);
DLLEXPORT miBoolean oversampling_lens_jitter	(miColor *, miState *, int *);
DLLEXPORT int	    path_material_version	(void);
DLLEXPORT miBoolean path_material		(miColor *, miState *,
						 struct dgs_material *);
}
