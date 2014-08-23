/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	10.05.96
 * Module:	physics
 * Purpose:	diffuse, glossy, and specular reflection and transmission
 *
 * Exports:
 *	dgs_material_version
 *	dgs_material
 *	dgs_material_photon
 *
 * History:
 *	14.01.98: removed install function, this is now a library
 *	19.05.98: added cylinder light source to physical_light().
 *	26.05.98: changed computation of alpha.
 *	17.06.98: split off physical_light().  Added direct illumination
 *		  translucency (direct diffuse and glossy transmission).
 *	13.10.98: No longer supports textures -- base shaders take care of
 *		  that (also for anisotropic brushing direction). Use mi_eval().
 *	06.07.00: major cleanup
 *      25.01.01: Fixed several bugs. Direct glossy is computed with ray
 *		  tracing instead of mi_sample_light.
 *
 * Description:
 * Material shader and photon shader with the following reflection types:
 * - Diffuse (Lambertian)
 * - Glossy (Ward model, isotropic and anisotropic, for anisotropic first
 * derivations are needed to obtain orientation)
 * - Specular (ideal mirror)
 * The material shader dgs_material() is used for rays originating from the
 * eye/camera. The photon shader dgs_material_photon() is used to trace photons
 * from the light sources. If called as a shadow shader, dgs_material() returns
 * miFALSE (i.e. completely opaque).  This is because we cannot compute shadows
 * along bent rays. Instead, this type of illumination is handled by the photon
 * map.
 *
 * Known limitations:
 * An off-specular peak is not simulated by the Ward glossy reflection model.
 * To get that effect, the Torrance-Sparrow model should be used (but it might
 * be necessary to use rejection sampling for that model).
 *
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <dgsshade.h>
#include <dgsutil.h>
#include <mi_version.h>
#include <mi_shader_if.h>

inline void addprod(
	miColor* c, 
	const miColor* a, 
	const miColor* b, 
	miScalar s = 1.0f)
{
	c->r += a->r * b->r * s;
	c->g += a->g * b->g * s;
	c->b += a->b * b->b * s;
}

char dgsshade_id[] =
	"==@@== compiled for ray " MI_VERSION_STRING " on " MI_DATE_STRING;

static miColor solid_black  = {0, 0, 0, 1};
static miColor transp_black = {0, 0, 0, 0};


/*
 * Material shader; direct and indirect illumination
 *
 * Calculate illumination of a material by its lights. m and paras are both
 * passed because m contains all the colors updated by textures, and paras
 * has the arrays (m is a copy of the fixed struct without appended arrays).
 * Phong and Blinn are glossy, *not* specular according to Hanrahan's
 * definition of terms!  We don't want to model Phong and Blinn *glossy*
 * reflection in globillum --- these are left to mib_illum_*. Phong isn't
 * physical (not energy-preserving), and Blinn is hard to importance-sample.
 * This function calculates the front (reflective) and back (refractive)
 * sides as separate light loops. This may seem inefficient because of the
 * duplicate mi_sample_light calls, but each call rejects lights on the
 * wrong side of state->normal early, and state->normal is flipped.
 */

/*
 * Compute direct diffuse and glossy components of illumination.
 * m->diffuse is reflectance rho (0 <= rho <= 1). BRDF f_r = rho/pi.
 * We also have to multiply by cosine at the receiver. The cosine at the light
 * source and the square of the distance is taken care of by the light shader.
 * The area of the light source is irrelevant since its emission is specified
 * by power, not radiance.
 */

static void direct_illumination(
	miColor			*result,	/* returned illum color */
	miState			*state,		/* ray tracer state */
	struct dgs_material	*m,		/* mi_eval'd material paras */
	miTag			*light)		/* light instance tag */
{
	int			samples;	/* # of samples taken */
	miColor			color;		/* color from light source */
	miColor			sum;		/* summed sample colors */
	miVector		dir;		/* direction towards light */
	miScalar		dot_nl;		/* dot prod of normal and dir */
	miScalar		f;		/* various temp factors */
	miBoolean		diffuse, glossy;/* correspondent parts > 0 */

        *result = solid_black;

	/* return immediately if no diffuse component */
	diffuse = m->diffuse.r > 0 || m->diffuse.g > 0 || m->diffuse.b > 0;
	glossy = m->glossy.r>0 || m->glossy.g>0 || m->glossy.b > 0;
	if (!diffuse && !glossy)
		return;

	/* compute direct illumination on front side for each light source */
	if (m->transp != 1.0 && (m->mode == 4 || m->n_light)) {
	        for (mi::shader::LightIterator iter(state, light, 
					m->n_light); !iter.at_end(); ++iter) {
		        sum = solid_black;

			while (iter->sample()) {
				iter->get_contribution(&color);
				dot_nl = iter->get_dot_nl();
			        if (diffuse) {
				        /* albedo*cos/Pi */
				        f = (1 - m->transp) * dot_nl * (1.F/M_PI_F);
			                addprod(&sum, &m->diffuse, &color, f);
				}
				if (glossy) {
					dir = iter->get_direction();
				        f = (1 - m->transp) * dot_nl *
					  dgs_ward_glossy(&state->dir, &dir,
							  state, m);
					addprod(&sum, &m->glossy, &color, f);
				}
		        }

			/* Add contribution of light source */
			samples = iter->get_number_of_samples();
			if (samples) {
			        result->r += sum.r / samples;
				result->g += sum.g / samples;
				result->b += sum.b / samples;
			}
		}
	}

	/*
	 * Compute direct illumination on back side for each light source
	 */
	if (m->transp != 0.0 && (m->mode == 4 || m->n_light)) {
	        miVector m_dir;	         /* mirror rel. to tangent plane */
		miScalar m_ok = miFALSE; /* not full reflection */

		if (mi_refraction_dir(&m_dir, state,state->ior_in,state->ior)) {
		        /* m_dir = 2 (normal . m_dir) normal - m_dir */
		        miVector tmp = state->normal;
			miScalar dot = mi_vector_dot(&state->normal, &m_dir);
			mi_vector_mul(&tmp, 2 * dot);
			mi_vector_sub(&m_dir , &m_dir, &tmp);
			m_ok = miTRUE;
		}

	        /* flip normals */
		mi_vector_neg(&state->normal);
                mi_vector_neg(&state->normal_geom);

	        for (mi::shader::LightIterator iter(state, light,
					m->n_light); !iter.at_end(); ++iter) {
		        sum = solid_black;

			while (iter->sample()) {
				iter->get_contribution(&color);
				dot_nl = iter->get_dot_nl();
			        if (diffuse) {
				        /* albedo*cos/Pi */
				        f = m->transp * dot_nl * (1.F/M_PI_F);
			                addprod(&sum, &m->diffuse, &color, f);
				}
				if (glossy && m_ok) {
					dir = iter->get_direction();
				        f = m->transp * dot_nl *
						dgs_ward_glossy(&m_dir, &dir,
							  	state, m);
					addprod(&sum, &m->glossy, &color, f);
				}
		        }
			/* Add contribution of light source */
			samples = iter->get_number_of_samples();
			if (samples) {
			        result->r += sum.r / samples;
				result->g += sum.g / samples;
				result->b += sum.b / samples;
			}
		}
		mi_vector_neg(&state->normal);
		mi_vector_neg(&state->normal_geom);
	}
	miASSERT(!isnan(result->r) && !isnan(result->g) && !isnan(result->b));
}


/*
 * Glossy and specular paths are traced if there are non-zero
 * glossy and specular coefficients resp, and not rejected by Russiian roulette.
 * Diffuse reflection is computed from the photon map.
 */

static void indirect_illumination(
	miColor		*result,		/* returned illum color */
	miState		*state,			/* ray tracer state */
	struct dgs_material	*m)		/* textured material paras */
{
	miVector	dir;
	miColor		color;
	miScalar	transp = m->transp;
	miScalar	refl = 1 - m->transp;
	miScalar	refl_spec = refl, refl_glos = refl;
	miScalar	diff = m->diffuse.r  + m->diffuse.g  + m->diffuse.b;
	miScalar	glos = m->glossy.r   + m->glossy.g   + m->glossy.b;
	miScalar	spec = m->specular.r + m->specular.g + m->specular.b;
	miScalar	total = diff + glos + spec;
	miScalar	efftransp = 0; 	/* effective object transparancy */

	*result = solid_black;

	if (transp > 0 && diff > 0) {
	        mi_compute_irradiance_backside(&color, state);
		addprod(result, &color, &m->diffuse, transp / M_PI_F);
	}
	if (refl > 0 && diff > 0) {
	        mi_compute_irradiance(&color, state);
		addprod(result, &color, &m->diffuse, refl / M_PI_F);
	}
        if (transp > 0 && spec > 0) {
	        if (mi_transmission_dir_specular(&dir, state,
						state->ior_in, state->ior)) {
		        if (mi_trace_refraction(&color, state, &dir))
		                addprod(result, &color, &m->specular, transp);
			efftransp += spec / total * (1 - color.a);
		} else
		        refl_spec = 1.0; /* full reflection */
	}
	if (refl_spec > 0 && spec > 0) {
	        mi_reflection_dir_specular(&dir, state);
		if (mi_trace_reflection(&color, state, &dir))
		        addprod(result,&color, &m->specular, refl_spec);
	}
	/* glossy */
	if (transp > 0 && glos > 0) {
	        if (dgs_transmit_glossy_dir(state, &dir, m)) {
		        /* not count area light sources here */
		        if (mi_trace_refraction(&color, state, &dir) &&
                            (state->pri || !mi_vector_norm(&state->normal)))
			        addprod(result, &color, &m->glossy, transp);
			efftransp += glos / total * (1 - color.a);
		} else
		        refl_glos = 1.0; /* full reflection */
	}
	if (refl_glos > 0 && glos > 0) {
                 dgs_reflect_glossy_dir(state, &dir, m);
		 /* not count area light sources here */
		 if (mi_trace_reflection(&color, state, &dir) &&
		    (state->pri || !mi_vector_norm(&state->normal)))
		         addprod(result, &color, &m->glossy, refl_glos);
	}
	result->a = 1 - (m->transp) * efftransp;
	miASSERT(!isnan(result->r) && !isnan(result->g) && !isnan(result->b));
}


/*
 * Material shader: Lambert diffuse + Ward glossy + mirror specular
 */

extern "C" DLLEXPORT int dgs_material_version(void) {return 3;}

extern "C" DLLEXPORT miBoolean dgs_material(
	miColor			*result,
	miState			*state,
	struct dgs_material	*paras)
{
	struct dgs_material	m;		/* work area for material */
	miColor			local, global;	/* local and global illum */
	miTag			*light;		/* tag of light instance */

	/* Material shader called as a photon shader? Convenience only. */
	if (miRAY_PHOTON(state->type))
	        return(dgs_material_photon(result, state, paras));

	/*
	 * Shadow ray: no illumination through object, transparent or not.
	 * Instead, this type of illumination is handled by the photon map.
	 */
	if (state->type == miRAY_SHADOW || state->type == miRAY_DISPLACE ) {
		*result = transp_black;
		return(miTRUE);
	}

	dgs_set_parameters(state, &m, paras);

	if (m.mode == 4) {
		m.n_light = 0;
		light = 0;
	} else {
		/* evaluate light parameters */
		m.i_light  = *mi_eval_integer(&paras->i_light);
		m.n_light  = *mi_eval_integer(&paras->n_light);
		light	   =  mi_eval_tag    (paras->light) + m.i_light;

		if (m.mode == 1)	/* modify light list (inclusive mode) */
			mi_inclusive_lightlist(&m.n_light, &light, state);
		else if (m.mode == 2)	/* modify light list (exclusive mode) */
			mi_exclusive_lightlist(&m.n_light, &light, state);
	}

	dgs_refraction_index(state, &m);

	/*
	 * Direct illumination from light sources. One migth think that it is
	 * sufficient to only compute direct illum when not inside a transpar-
	 * ent object, that is, when state->refraction_level is even. But that
	 * fails when the light source is inside the same transparent object.
	 */
	direct_illumination(&local, state, &m, light);

	/* Indirect illumination. */
	indirect_illumination(&global, state, &m);

	result->r = local.r + global.r;
	result->g = local.g + global.g;
	result->b = local.b + global.b;
	result->a = global.a;
	miASSERT(0 <= result->a && result->a <= 1);

	return(miTRUE);
}



/*
 * The dgs photon shader enforces energy-preserving scattering coefficients,
 * that is, the following three requirements:
 * diffuse.r + glossy.r + specular.r  has to be <= 1,
 * diffuse.g + glossy.g + specular.g  has to be <= 1,
 * diffuse.b + glossy.b + specular.b  has to be <= 1.
 */

extern "C" DLLEXPORT int dgs_material_photon_version(void) {return 3;}

extern "C" DLLEXPORT miBoolean dgs_material_photon(
	miColor			*energy,
	miState			*state,
	struct dgs_material	*paras)
{
	struct dgs_material	m;
	miColor	        color;	/* energy of photon after interaction */
	miVector	dir;
	miRay_type	type;

	dgs_set_parameters(state, &m, paras);

	/*
	 * Insert photon to map if this is a diffuse surface and not direct
	 * illumination from a light source. DGS shoots no trans[arent photons,
	 * but other materials may do so.
	 */
	if (m.diffuse.r > miEPS || m.diffuse.g > miEPS || m.diffuse.b > miEPS) {
	    miState	*parent = state;
	    
            while(parent->type == miPHOTON_TRANSPARENT && parent->parent) {
	            miASSERT(parent->parent != parent);
		    parent = parent->parent;
	    }
	    if (parent->type != miPHOTON_LIGHT)
	            mi_store_photon(energy, state);
	}

	/* compute iors */
	dgs_refraction_index(state, &m);
	type = mi_choose_scatter_type(state, m.transp,
				      &m.diffuse, &m.glossy, &m.specular);

	/* compute new photon color (compensating for Russian roulette) */
	switch (type) {
	  case miPHOTON_ABSORB:		    /* no reflection or transmission */
		return(miTRUE);

	  case miPHOTON_REFLECT_DIFFUSE:
	  case miPHOTON_TRANSMIT_DIFFUSE:
	        color.r = energy->r * m.diffuse.r;
		color.g = energy->g * m.diffuse.g;
		color.b = energy->b * m.diffuse.b;
		break;

	  case miPHOTON_REFLECT_GLOSSY:
	  case miPHOTON_TRANSMIT_GLOSSY:
	        color.r = energy->r * m.glossy.r;
		color.g = energy->g * m.glossy.g;
		color.b = energy->b * m.glossy.b;
		break;

          case miPHOTON_REFLECT_SPECULAR:
	  case miPHOTON_TRANSMIT_SPECULAR:
		color.r = energy->r * m.specular.r;
		color.g = energy->g * m.specular.g;
		color.b = energy->b * m.specular.b;
		break;

	  default:			  /* Unknown scatter type */
		mi_error("unknown scatter type in dgs photon shader");
		return(miFALSE);
	}

	/* Shoot new photon in a direction determined by the scattering type */
	switch (type) {
	  case miPHOTON_TRANSMIT_DIFFUSE:/* diffuse transm. (translucency) */
	        mi_transmission_dir_diffuse(&dir, state);
		return(mi_photon_transmission_diffuse(&color, state, &dir));

	  case miPHOTON_REFLECT_DIFFUSE: /* diffuse reflection (Lambert) */
		mi_reflection_dir_diffuse(&dir, state);
		return(mi_photon_reflection_diffuse(&color, state, &dir));

          case miPHOTON_TRANSMIT_GLOSSY: /* glossy transmission (Ward model) */
	        if (dgs_transmit_glossy_dir(state, &dir, &m))
		        return(mi_photon_transmission_glossy(&color,
							     state, &dir));
		/* else: full reflection case, no break ! */
	  case miPHOTON_REFLECT_GLOSSY:  /* glossy reflection (Ward model)*/
		dgs_reflect_glossy_dir(state, &dir, &m);
		return(mi_photon_reflection_glossy(&color, state, &dir));

	  case miPHOTON_TRANSMIT_SPECULAR:/* specular transmission */
	        if (mi_transmission_dir_specular(&dir, state,
						 state->ior_in, state->ior)) {
			return(mi_photon_transmission_specular(&color,
								state, &dir));
		}
		/* full reflection case, no break !*/
	  case miPHOTON_REFLECT_SPECULAR:   /* specular reflection (mirror) */
		mi_reflection_dir_specular(&dir, state);
		return(mi_photon_reflection_specular(&color, state, &dir));

	  default:
	        miASSERT(0); /* cannot be here */
		return(miFALSE);
	}
}
