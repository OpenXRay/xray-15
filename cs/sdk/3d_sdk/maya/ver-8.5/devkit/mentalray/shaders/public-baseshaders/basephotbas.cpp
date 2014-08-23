/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	29.01.98
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *
 *	mib_photon_basic
 *	mib_photon_basic_version
 *
 * History:
 *	06.02.98: use mi_choose_simple_scatter_type for Russian roulette
 *	09.02.98: added photon tracing check
 *
 * Description:
 *	Basic photon shader capable of diffusely reflecting a photon
 *	or specularly reflecting or transmitting a photon
 *****************************************************************************/

#include <stddef.h>
#include "shader.h"


struct mib_photon_basic {
	miColor	diffuse;
	miColor	specular;
	miColor	transp;
	miScalar	ior_frac;
};


extern "C" DLLEXPORT int mib_photon_basic_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_photon_basic(
	miColor		*flux,
	miState		*state,
	struct mib_photon_basic *paras)
{
	miColor		rdiff, *spec, *transp, nflux;
	miScalar	ior_frac;
	miVector	dir;
	miRay_type	type;
	miColor		tspec, rspec;

	rdiff   = *mi_eval_color(&paras->diffuse);
	spec    =  mi_eval_color(&paras->specular);
	transp  =  mi_eval_color(&paras->transp);
	tspec.r = spec->r * transp->r;
	tspec.g = spec->g * transp->g;
	tspec.b = spec->b * transp->b;
	rspec.r = spec->r * (1.0 - transp->r);
	rspec.g = spec->g * (1.0 - transp->g);
	rspec.b = spec->b * (1.0 - transp->b);

	if (rdiff.r > 0.0 || rdiff.g > 0.0 || rdiff.b > 0.0)
		mi_store_photon(flux, state);

	type = mi_choose_simple_scatter_type(state, &rdiff, &rspec, 0, &tspec);

	switch(type) {
	  case miPHOTON_REFLECT_SPECULAR:
		nflux.r = flux->r * rspec.r;
		nflux.g = flux->g * rspec.g;
		nflux.b = flux->b * rspec.b;
		mi_reflection_dir_specular(&dir, state);
		return(mi_photon_reflection_specular(&nflux, state, &dir));

	  case miPHOTON_REFLECT_DIFFUSE:
		nflux.r = flux->r * rdiff.r;
		nflux.g = flux->g * rdiff.g;
		nflux.b = flux->b * rdiff.b;
		mi_reflection_dir_diffuse(&dir, state);
		return(mi_photon_reflection_diffuse(&nflux, state, &dir));

	  case miPHOTON_TRANSMIT_SPECULAR:
		nflux.r = flux->r * tspec.r;
		nflux.g = flux->g * tspec.g;
		nflux.b = flux->b * tspec.b;
		ior_frac = *mi_eval_scalar(&paras->ior_frac);
		if (ior_frac == 1.0)
			return(mi_photon_transparent(&nflux, state));

		else if (mi_transmission_dir_specular(&dir, state, 1,ior_frac))
			return(mi_photon_transmission_specular(&nflux, state,
									&dir));
		else
			return(miFALSE);

	  default:
		return(miTRUE);
	}
}
