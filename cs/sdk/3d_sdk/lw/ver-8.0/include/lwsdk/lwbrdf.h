/*
 * LWSDK Header File
 * Copyright 1999,  NewTek, Inc.
 *
 * LWBRDF.H -- LightWave Reflection Distribution Shaders
 *
 * The interaction of a light with a given surface is encapsulated in a
 * single function, commonly referred to as a Bidirectional Reflectance
 * Distribution Function (BRDF). This function is used in rendering a
 * surface which has already been shaded.  Using the vectors from the
 * surface to the viewer, and from the surface to the light, and the
 * surface normal, this function computes a light's contribution to the
 * final surface specular, diffuse, and translucent values. In rendering,
 * the shaders for a spot are called to determine the surface attributes
 * at that spot and time, then for each light, the diffuse reflection,
 * specular reflection, and transmission values are computed, and summed
 * into the final spot color (or pre-luminosity). If any BRDF plugins are
 * present, the BRDF plugins are called on each each light, with the
 * internally computed values for the specular and diffuse reflected, and
 * transmitted light color.  The BRDF evaluation is given an LWBRDFAccess
 * structure.  It includes a read-only LWShaderAccess structure which
 * reflects (as it were) the final, shaded state of the surface. 
 */
#ifndef LWSDK_BRDF_H
#define LWSDK_BRDF_H

#include <lwsdk/lwrender.h>
#include <lwsdk/lwshader.h>

#define LWBRDF_HCLASS	"BRDFHandler"
#define LWBRDF_ICLASS	"BRDFInterface"
#define LWBRDF_VERSION	4


typedef struct st_BRDFAccess {          
	const LWShaderAccess *shdrAcc;
	int		 axis;
	double		 viewVec[3];  /* Normalized */
	LWItemID	 lightID;
	double		 liteVec[3];
	double		 liteRGB[3];
	/* These parameters should be be changed, for a plugin to do anything */ 
	double		 diffRGB[3];  /* diffuse reflected value from this light at this spot */
	double		 specRGB[3];  /* specular reflected value from this light at this spot */
	double		 transRGB[3];  /* transmitted value from this light at this spot */
} LWBRDFAccess;

typedef struct st_LWBRDFHandler {
	LWInstanceFuncs	 *inst;
	LWItemFuncs	 *item;
	LWRenderFuncs	 *rend;
	void		(*evaluate) (LWInstance, LWBRDFAccess *);
	unsigned int	(*flags)    (LWInstance);
} LWBRDFHandler;

#endif
