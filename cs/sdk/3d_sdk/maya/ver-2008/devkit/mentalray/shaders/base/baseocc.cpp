/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	23.07.04
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *	mib_amb_occlusion
 *	mib_amb_occlusion_version
 *      mib_fg_occlusion
 *      mib_fg_occlusion_version
 *	mib_bent_normal_env
 *	mib_bent_normal_env_version
 *
 * History:
 *      02.06.06: bugfix reflective occlusion, improved sampling
 *      13.04.05: bugfix fg occlusion
 *      10.02.05: version 2 of mib_amb_occlusion
 *      30.11.04: added fg_occlusion
 *      30.09.04: added mib_bent_normal_env
 *	11.09.04: added recursion avoidance
 *	08.08.04: added mib_bent_normal_env
 *	23.07.04: copied from misss_simple_occlusion
 *
 * Description:
 *	A simple ambient occlusion shader.
 *
 *****************************************************************************/

#ifdef HPUX
#pragma OPT_LEVEL 1 /* workaround for HP/UX optimizer bug, +O2 and +O3 */
#endif

#include <stdio.h>
#include <stdlib.h>	/* for abs */
#include <float.h>	/* for FLT_MAX */
#include <math.h>
#include <string.h>
#include <assert.h>
#include <shader.h>
#include <geoshader.h>

/* Simple ambient occlusion */

/* Simple ambient occlusion parameter struct */
struct mib_amb_occlusion_p {
	int	    samples;
	miColor     bright;
	miColor     dark;
	miScalar    spread;
	miScalar    max_distance;
	miBoolean   reflective;
	int	    return_type;
	miBoolean   occlusion_in_alpha;
        /* Version 2 parameters */
        miScalar    falloff;
        int         id_includeexclude;
        int         id_nonself;
};

extern "C" DLLEXPORT int mib_amb_occlusion_version(void) {return(2);}

typedef struct miao_trace_info_t {
    miBoolean compatible;
    int id_inclexcl;
    int id_nonself;
} miao_trace_info;

static int miao_get_label(miTag instance)
{
    int result       = 0;
    miInstance* inst = NULL;
    
    if (!instance) return 0;
    inst = (miInstance *)mi_db_access(instance);
    if (inst) result = inst->label;
    mi_db_unpin(instance);
    return result;
}

static miBoolean    miao_trace_the_ray(
        miState    *state,
        miVector   *dir,
        miVector   *point,
        miao_trace_info *ti)
{
    int label = 0, childlabel = 0;
    miVector  base_point = *point;
    miScalar  base_dist  = 0;
    miBoolean result     = miFALSE;

play_it_again:
    /* Trace the ray */
    result = mi_trace_probe(state, dir, &base_point);    
    /* If a miss, or compatible mode - then simply return */
    if (!result || ti->compatible) return result;

    childlabel = miao_get_label(state->child->instance);

    /* Test non-self-occlusion. A hit on this object id that
       originates on the same object id is considered a total 
       miss, i.e. the environment */
    if (ti->id_nonself > 0)
    { 
        int label = miao_get_label(state->instance);

        /* Compare parent AND child labels */
        if (label      == ti->id_nonself &&
            childlabel == ti->id_nonself)
            return miFALSE;
    }

    /* Test the include/exclude. However, a miss here causes
       the ray to continue to be traced, in case of a hit */

    /* Positive inclexcl == include */
    if (ti->id_inclexcl > 0 && childlabel != ti->id_inclexcl)
        result = miFALSE;

    /* Negative inclexcl == exclude */
    if (ti->id_inclexcl < 0 && childlabel == -ti->id_inclexcl)
        result = miFALSE;

    /* Was this hit rejected? */
    if (!result)
    {
        /* Remember distance traced so far */
        base_dist += (miScalar)state->child->dist;

        /* Advance the point */
        base_point.x  += base_dist * dir->x;
        base_point.y  += base_dist * dir->y;
        base_point.z  += base_dist * dir->z;

        /* Keep tracing ray */
        goto play_it_again;
    }

    /* Plug originals back, if necessary */
    /* state->child->dist += base_dist; */

    return result;
}

extern "C" DLLEXPORT miBoolean mib_amb_occlusion(
	miColor     *result,
	miState     *state,
	struct mib_amb_occlusion_p *paras)
{
	double sample[3], near_clip, far_clip;
	int	  counter  = 0;
	miUint	  samples  = *mi_eval_integer(&paras->samples);
	miScalar  clipdist = *mi_eval_scalar(&paras->max_distance);
	miBoolean reflecto = *mi_eval_boolean(&paras->reflective);
	miBoolean ret_type = *mi_eval_integer(&paras->return_type);

	miTag	   org_env = state->environment; /* Original environ. */

	miVector orig_normal, trace_dir;
	miScalar output      = 0.0, 
		 samplesdone = 0.0;
	miScalar spread     = *mi_eval_scalar(&paras->spread);
	miScalar o_m_spread = 1.0f - spread;
	
	miColor   env_total;	/* environment Total */ 	
	miVector  norm_total;	/* Used for adding up normals */
	miBoolean occ_alpha = *mi_eval_boolean(&paras->occlusion_in_alpha);

        miScalar  falloff   = 1.0;
        miao_trace_info ti;
        int       version = 1;

	/* If called as user area light source, return "no more samples" for	
	   any call beyond the first */
	if (state->type == miRAY_LIGHT && state->count > 0) 
		return (miBoolean)2;

        /* Figure out the call version */
        mi_query(miQ_DECL_VERSION, 0, state->shader->function_decl, &version); 
        if (version >= 2)
        {
            falloff = *mi_eval_scalar(&paras->falloff);
            if (falloff <= 0.0) 
                falloff = 1.0;
            ti.id_inclexcl   = *mi_eval_integer(&paras->id_includeexclude);
            ti.id_nonself    = *mi_eval_integer(&paras->id_nonself);

            /* None of these options used, go into compatible mode */
            ti.compatible = (ti.id_inclexcl   == 0 && 
                             ti.id_nonself    == 0 
                             );
        }
        else
        {
            ti.compatible = miTRUE;
        }

	/* Used for adding up environment */
	env_total.r = env_total.g = env_total.b = env_total.a = 0;

	far_clip = near_clip = clipdist;

	orig_normal  = state->normal;
	norm_total   = state->normal; /* Begin by standard normal */

	/* Displacement? Shadow? Makes no sense */
	if (state->type == miRAY_DISPLACE ||
            state->type == miRAY_SHADOW) {
		result->r = result->g = result->b = result->a = 0.0;
		return (miTRUE); 
	}

	if (clipdist > 0.0) 
		mi_ray_falloff(state, &near_clip, &far_clip);

	/* Avoid recursion: If we are designated as environment shader,
	   we will be called with rays of type miRAY_ENVIRON, and if so
	   we switch the environment to the global environment */

	if (state->type == miRAY_ENVIRONMENT)
	    state->environment = state->camera->environment;

	while (mi_sample(sample, &counter, state, 2, &samples)) {
		mi_reflection_dir_diffuse_x(&trace_dir, state, sample);
		trace_dir.x = orig_normal.x*o_m_spread + trace_dir.x*spread;
		trace_dir.y = orig_normal.y*o_m_spread + trace_dir.y*spread;
		trace_dir.z = orig_normal.z*o_m_spread + trace_dir.z*spread;
	
		mi_vector_normalize(&trace_dir);

		if (reflecto) {
			miVector ref;
			miScalar nd = state->dot_nd;
			/* Calculate the reflection direction */
			state->normal = trace_dir;
			state->dot_nd = mi_vector_dot(
						&state->dir,
						&state->normal);
			/* Bugfix: mi_reflection_dir(&ref, state);
			   for some reason gives me the wrong result,
			   doing it "manually" works better */
			ref    = state->dir;
			ref.x -= state->normal.x*state->dot_nd*2.0f;
			ref.y -= state->normal.y*state->dot_nd*2.0f;
			ref.z -= state->normal.z*state->dot_nd*2.0f;
			state->normal = orig_normal;
			state->dot_nd = nd;
			trace_dir = ref;
		}

		if (mi_vector_dot(&trace_dir, &state->normal_geom) < 0.0) 
			continue;

		output	    += 1.0; /* Add one */
		samplesdone += 1.0;

		if (state->options->shadow &&
		    miao_trace_the_ray(state, &trace_dir, &state->point, &ti)) {
			/* we hit something */
    
			if (clipdist == 0.0) 
				output -= 1.0;
			else if (state->child->dist < clipdist) {
				miScalar f = (miScalar)pow(state->child->dist / clipdist,
                                                 (double) falloff);

				output -= (1.0F - f);

				norm_total.x += trace_dir.x * f;
				norm_total.y += trace_dir.y * f;
				norm_total.z += trace_dir.z * f;

				switch (ret_type) {
					case 1: 
					  { 
					    /* Environment sampling */
					    miColor envsample;
					    mi_trace_environment(&envsample, 
						state, &trace_dir);

					    env_total.r += envsample.r * f;
					    env_total.g += envsample.g * f;
					    env_total.b += envsample.b * f;
					  }
					  break;
					
					default: 
					  /* Most return types need no 
						special stuff */
					  break;
				}
			}
		}
		else {
			/* We hit nothing */
			norm_total.x += trace_dir.x;
			norm_total.y += trace_dir.y;
			norm_total.z += trace_dir.z;

			switch (ret_type) {
				case 1: /* Environment sampling */
					{
					   miColor envsample;

					   mi_trace_environment(&envsample, 
						   state, &trace_dir);

					   env_total.r += envsample.r;
					   env_total.g += envsample.g;
					   env_total.b += envsample.b;
					}
					break;
				default:
					/* Most return types need no 
						special treatment */
					break; 
			}
		}
	}

	if (clipdist > 0.0) 
		mi_ray_falloff(state, &near_clip, &far_clip);
    
	if (samplesdone <= 0.0) /* No samples? */
		samplesdone = 1.0;  /* 1.0 to not to break divisons below */

	switch (ret_type) {
		case -1:  /* Plain old occlusion with untouched normal*/
		case 0:   /* Plain old occlusion */
		default:  /* (also the default for out-of-bounds values) */
		{
			miVector old_dir = state->dir;
			output /= (miScalar) samplesdone;

			if (ret_type == -1)
				norm_total = state->normal;
			else {
				mi_vector_normalize(&norm_total);

				/* If the color shaders use the normal....
					give them the bent one... */
				state->normal = norm_total;  
				state->dir    = norm_total;
			}
			if (output == 0.0)
				*result = *mi_eval_color(&paras->dark);
			else if (output >= 1.0)
				*result = *mi_eval_color(&paras->bright);
			else {
				miColor bright, dark;
				bright = *mi_eval_color(&paras->bright);
				dark   = *mi_eval_color(&paras->dark);

				result->r = bright.r * output + dark.r * 
					(1.0F - output);
				result->g = bright.g * output + dark.g * 
					(1.0F - output);
				result->b = bright.b * output + dark.b * 
					(1.0F - output);

				if (occ_alpha)
					result->a = output;
				else
					result->a = bright.a * output 
						   + dark.a * (1.0F - output);
			}

			state->normal = orig_normal;
			state->dir    = old_dir;
		}
		break;
		case 1: /* Sampled environment */
		{
			miColor br	= *mi_eval_color(&paras->bright),
				drk	= *mi_eval_color(&paras->dark);

			result->r = drk.r + (br.r * env_total.r / samplesdone);
			result->g = drk.g + (br.g * env_total.g / samplesdone);
			result->b = drk.b + (br.b * env_total.b / samplesdone);
			if (occ_alpha)
				result->a = output/ samplesdone;
			else
				result->a = 1.0;
		}
		break;
		case 2: /* Bent normals, world */
		case 3: /* Bent normals, camera */
		case 4: /* Bent normals, object */
		{
			miVector retn; /* returned Normal */

			mi_vector_normalize(&norm_total);

			if (ret_type == 2)
				mi_normal_to_world(state, &retn, &norm_total);
			if (ret_type == 3)
				mi_normal_to_camera(state, &retn, &norm_total);
			if (ret_type == 4)
				mi_normal_to_object(state, &retn, &norm_total);

			result->r = (retn.x + 1.0F) / 2.0F;
			result->g = (retn.y + 1.0F) / 2.0F;
			result->b = (retn.z + 1.0F) / 2.0F;
		
			if (occ_alpha)
				result->a = output/ samplesdone;
			else
				result->a = 1.0;
		}
		break;
	}

	if (state->type == miRAY_LIGHT) {
		 /* Are we a light shader? */
		int type;
		mi_query(miQ_FUNC_CALLTYPE, state, 0, &type);

		/* Make sure we are called as light shader */
		if (type == miSHADER_LIGHT) {
			/* If so, move ourselves to above the point... */
			state->org.x = state->point.x + state->normal.x;
			state->org.y = state->point.y + state->normal.y;
			state->org.z = state->point.z + state->normal.z;
			/* ...and set dot_nd to 1.0 to illuminate fully */
			state->dot_nd = 1.0;
		}
	}
	/* Reset environment, if we changed it */
	state->environment = org_env;
	return miTRUE;
}


/* Simple environment sampling */

/* Simple environment sampling parameter struct */
struct mib_bent_normal_env_p {
	miColor     bent_normals;
	miBoolean   occlusion_in_alpha;
	miColor     occlusion;
	miScalar    strength;
	miTag	    environment;
	int	    coordinate_space;
	int	    env_samples;
	miScalar    samples_spread;
	miMatrix    matrix;
};

extern "C" DLLEXPORT int mib_bent_normal_env_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_bent_normal_env(
	miColor     *result,
	miState     *state,
	struct mib_bent_normal_env_p *paras)
{
	miTag  original_env = state->environment;
	miTag  environment  = *mi_eval_tag(&paras->environment);
	miColor   normal    = *mi_eval_color(&paras->bent_normals);
	miColor   occlus    = *mi_eval_color(&paras->occlusion);
	miScalar  strength  = *mi_eval_scalar(&paras->strength);
	miColor   color;      /* Work color */
	miVector  bent;       /* Bent normals */
	miVector  bent_i;     /* Bent normals in internal space */
	miUint	  samples;    /* # of samples */

	/* Displace or light - makes no sense */
	if (state->type == miRAY_DISPLACE || state->type == miRAY_LIGHT)
	    return miFALSE;

	if (strength == 0.0) {
	    result->r = result->g = result->b = 0.0;
	    result->a = 1.0;
	    return miTRUE;
	}

	color.r = color.b = color.g = 0.0;

	/* Does occlusion live in "alpha" component of bn data? */
	if (*mi_eval_boolean(&paras->occlusion_in_alpha))
	    strength *= normal.a;
    
	bent.x = (normal.r * 2.0F) - 1.0F;
	bent.y = (normal.g * 2.0F) - 1.0F;
	bent.z = (normal.b * 2.0F) - 1.0F;

	mi_vector_normalize(&bent);

	/* The different coordinate spaces */
	switch(*mi_eval_integer(&paras->coordinate_space)) {
		case 0: /* No change */
			bent_i = bent;
			break;
		case 1: /* By matrix */
			mi_vector_transform(&bent_i,
				&bent,
				mi_eval_transform(&paras->matrix));
			break;
		case 2: /* World */
			mi_normal_from_world(state, &bent_i, &bent);
			break;
		case 3: /* Camera */
			mi_normal_from_camera(state, &bent_i, &bent);
			break;
		case 4: /* Object */
			mi_normal_from_object(state, &bent_i, &bent);
			break;
	}
    
	samples = *mi_eval_integer(&paras->env_samples);

	/* Temporarily override the environment */
	if (environment)
	    state->environment = environment;

	if (samples <= 1) {
		/* Single sampling */		      
		mi_trace_environment(&color, state, &bent_i);
	} else {
		/* Multisampling */
		double	 sample[3];
		int	 counter = 0;
		miScalar spread  = *mi_eval_scalar(&paras->samples_spread);
		miColor  work_color;

		/* Clear color */
		color.r = color.g = color.b = 0.0;
			
		while (mi_sample(sample, &counter, state, 3, &samples)) {
			miVector trace_dir;
			trace_dir.x = bent_i.x + (miScalar)(sample[0] - 0.5) * spread;
			trace_dir.y = bent_i.y + (miScalar)(sample[1] - 0.5) * spread;
			trace_dir.z = bent_i.z + (miScalar)(sample[2] - 0.5) * spread;
			
			mi_vector_normalize(&trace_dir);

			mi_trace_environment(&work_color, state, &trace_dir);
			color.r += work_color.r;
			color.g += work_color.g;
			color.b += work_color.b;
		}

		color.r /= samples;
		color.g /= samples;
		color.b /= samples;
	}

	/* Reset original environment */
	state->environment = original_env;

	result->r = color.r * occlus.r * strength;
	result->g = color.g * occlus.g * strength;
	result->b = color.b * occlus.b * strength;
	result->a = 1.0;

	return miTRUE;
}


/* Simple final-gather based occlusion shader */
struct mib_fg_occlusion_p {
	miColor     result_when_fg_is_off;
};

extern "C" DLLEXPORT int mib_fg_occlusion_version(void) {return(1);}

extern "C" DLLEXPORT miBoolean mib_fg_occlusion(
	miColor     *result,
	miState     *state,
	struct mib_fg_occlusion_p *paras)
{
    /* Final gather ray gets a return value of black */
    if (state->type == miRAY_FINALGATHER)
    {
        result->r = result->g = result->b = 0.0;
        result->a = 1.0;
        return miTRUE;
    }
    if (state->options->finalgather)
    {
        miColor color;
        mi_compute_irradiance(&color, state);
        result->r = 1.0F - color.a;
        result->g = 1.0F - color.a;
        result->b = 1.0F - color.a;
        result->a = 1.0F;
    }
    else
    {
        *result = *mi_eval_color(&paras->result_when_fg_is_off);
    }

    return miTRUE;
}
