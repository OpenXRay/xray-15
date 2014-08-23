/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	14.08.2004 
 * Module:	baseshader
 * Purpose:	base shaders for Phenomenon writers
 *
 * Exports:
 *      mib_glossy_reflection
 *      mib_glossy_refraction
 *      mib_glossy_reflection_version
 *      mib_glossy_refraction_version
 *
 * History:
 *	14.08.2004 created
 *      13.05.2005 added to central version
 *      20.05.2005 fixed bug in reflection dir rejection code
 *      24.05.2005 Added shader state logic
 *      24.05.2005 Changed "_material" parameters to type "shader"
 *      xx.06.2005 Fixed bug in UV vector generation,
 *      26.07.2005 Improved finalgather mode test (made it separate
 *                 function) and improved shader state logic.
 *      23.10.2006 Better reflection ray rejection
 *      24.10.2006 Fixed above for single ray (first ray) case
 *
 * Description:
 *	multisampled glossy refractions, with optional distance limiting,
 *      dispersion and fresnel effect. The nodes can be used in phenomena
 *      to do glossy reflection and refraction efficiently. This includes
 *      an optimized distribution of samples, and fresnel like weighting
 *      for each individual sample to obtain better shading.
 *      Distance limiting can be used to heavily optimize performance,
 *      which is very well suited for glossy materials.
 *      
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <shader.h>


/* Test if we are already multisampling */

typedef struct mi_glossy_state_t {
    int multisampling;
} mi_glossy_state;

static miBoolean test_multisampling(miState *state)
{
    int keyz = 4;
    mi_glossy_state *ss = (mi_glossy_state*)mi_shaderstate_get(state, "miMS", &keyz);

    if (!ss) return miFALSE;

    return ss->multisampling;
}

static void set_multisampling(miState *state, miBoolean mode)
{
    int keyz = 4;
    mi_glossy_state *ss = (mi_glossy_state*)mi_shaderstate_get(state, "miMS", &keyz);

    if (!ss)
    {
        mi_glossy_state ns;
        ns.multisampling = mode;
        mi_shaderstate_set(state, "miMS", &ns, sizeof(ns), miSS_LIFETIME_EYERAY);
    }
    else
    {
        ss->multisampling = mode;
    }
}

/* Calculate the u/v deviation vectors */

static void calculate_vectors(miState *state, miVector *u_out, 
                              miVector *v_out, miVector *u_in, 
                              miVector *v_in, miBoolean anisotropic)
{
    miBoolean have_v = miFALSE;

    if (anisotropic)
    {
        /* Eval the param */
        u_in = mi_eval_vector(u_in);

        /* If the user specified nothing */
        if (u_in->x == 0.0 && u_in->y == 0.0 && u_in->z == 0)
        {
            /* Try a derivative, if one exists */
            *u_out = state->derivs[0];

            /* If it was a null vector */
            if (u_out->x == 0.0 && u_out->y == 0.0 && u_out->z == 0.0)
            {
                miVector  foo;      /* Arb. vector */
                miBoolean loop = 2; /* Loop count  */
                miScalar  dot_prod; /* Dot product */
                /* Pick an arbitrary vector, object space Z-axis */
                foo.x = 0.0;
                foo.y = 0.0;
                foo.z = 1.0;

                /* Loop exists to handle degenerate case 
                   normally executed once only */
                while (loop-- > 0) {
                    /* Convert from object space */
                    mi_vector_from_object(state, v_out, &foo);

                    dot_prod = mi_vector_dot(&state->normal, v_out);

                    /* Force vector to be perpendicular */
                    v_out->x -= state->normal.x * dot_prod;
                    v_out->y -= state->normal.y * dot_prod;
                    v_out->z -= state->normal.z * dot_prod;

                    /* Low quality case */
                    if (dot_prod > 0.999)
                    {
                        /* Pick different vector, Y */
                        foo.x = 0.0;
                        foo.y = 1.0;
                        foo.z = 0.0;
                        continue; /* Re-loop */
                    }
                    break;                   
                }
                /* We calculated v, so no need to re-do that */
                have_v = miTRUE;
                /* Create u_out perp to v_out and normal */
                mi_vector_prod(u_out, &state->normal, v_out); 
            }
        }
        else /* Use users vector */
        {
            mi_vector_from_world(state, u_out, u_in);
           
            /* Eval the param */
            v_in = mi_eval_vector(v_in);

            /* If the user specified something */
            if (v_in->x != 0.0 || v_in->y != 0.0 || v_in->z == 0)
            {
                mi_vector_from_world(state, v_out, v_in);
                mi_vector_normalize(v_out);
                have_v = miTRUE;
            }
        }

        mi_vector_normalize(u_out);

        if (!have_v)
        {
            /* Create v_out perp to u_out and normal */
            mi_vector_prod(v_out, &state->normal, u_out);
        }
    }
    else /* Isotropic, direction doesnt matter */
    {
        /* Build arbitrary perp vectors by transposing 
           x, y and z components */
        u_out->x = -state->normal.z;
        u_out->y = state->normal.x;
        u_out->z = state->normal.y;

        /* Create v_out perp to u_out and normal */
        mi_vector_prod(v_out, &state->normal, u_out);
        /* Create u_out perp to v_out and normal */
        mi_vector_prod(u_out, &state->normal, v_out);    
    }
}

miColor rainbowdef[] = 
{
    { 1, 0, 0} ,  /* Red */
    { 1, 1, 0} ,  /* Yellow */
    { 1, 1, 1} ,  /* White */
    { .5, 1, 1} , /* Cyan-ish */
    { 0, 0, 1} ,  /* Blue */
    { 1, 0, 1}    /* Indigo (Magenta) */
};

static void affect_rainbow(miColor *color, miScalar colorfac,
                           miScalar strength, miColor* rainbow, int n_items)
{
    int idx, idx2;
    miScalar fac1, fac2;
    miColor color1, color2, color3;

    /* We want to adress UP TO n_items, so subtract one, i.e. for
       6 array items we want to go from 0 to 5, not 6 */
    n_items--;

    /* change +/- 0.5 range to a 0.0-1.0 range) */
    colorfac += 0.5;

    /* Assure 0 to 1 range */
    if (colorfac < 0) colorfac = 0;
    if (colorfac > 1) colorfac = 1;

    colorfac *= (float)n_items; 

    idx  = (int)floor(colorfac);
    idx2 = idx+1;

    if (idx >  n_items) idx = n_items;
    if (idx2 > n_items) idx2 = n_items;

    fac2 = colorfac - idx;
    fac1 = 1.0f - fac2;

    color1 = rainbowdef[idx];
    color2 = rainbowdef[idx2];

    color3.r = ((color1.r * fac1 + color2.r * fac2) * strength) 
               + (1.0f - strength);
    color3.g = ((color1.g * fac1 + color2.g * fac2) * strength) 
               + (1.0f - strength);
    color3.b = ((color1.b * fac1 + color2.b * fac2) * strength) 
               + (1.0f - strength);

    color->r *= color3.r;
    color->g *= color3.g;
    color->b *= color3.b;
}

miBoolean final_gather_mode(miState *state)
{
    int fg, tile[4];

    /* If miQ_TILE_PIXELS returns false, we are in the
       precompute stage */

    if (!mi_query(miQ_TILE_PIXELS, state, miNULLTAG, tile))
        return miTRUE;

    if (state->type == miRAY_FINALGATHER) 
        return miTRUE;

    mi_query(miQ_FINALGATHER_STATE, state, miNULLTAG, &fg);
    if (fg) return miTRUE;

    return miFALSE;
}

typedef struct {
        miTag           surface;
	miColor		reflection_color;
	miScalar	reflection_depth;
        miScalar        falloff;
        miColor         environment_color;
        miScalar        base_weight;
        miScalar        edge_weight;
        miScalar        edge_factor;
        miTag           environment;
        miBoolean       single_env_sample;
	miInteger	samples;
	miScalar	u_spread;
        miScalar        v_spread;
        miVector        u_axis;
        miVector        v_axis;
        miScalar        rainbowness;
        int             i_spectrum;
        int             n_spectrum;
        miColor         spectrum[1];
} mib_glossy_reflection_t;


extern "C" DLLEXPORT int mib_glossy_reflection_version(void) {return(3);}


extern "C" DLLEXPORT miBoolean mib_glossy_reflection(
	miColor *result,
	miState *state,
	mib_glossy_reflection_t *param)
{
	miColor   reflection_color = *mi_eval_color(&param->reflection_color);
	miScalar  reflection_depth = *mi_eval_scalar(&param->reflection_depth);
 	miColor   environment_color = *mi_eval_color(&param->environment_color);
	miScalar  u_spread = *mi_eval_scalar(&param->u_spread);
	miScalar  v_spread = *mi_eval_scalar(&param->v_spread);
        miScalar  rainbow  = *mi_eval_scalar(&param->rainbowness);
	miUint    samples  = *mi_eval_integer(&param->samples);
        miBoolean sing_env = *mi_eval_boolean(&param->single_env_sample);
	miColor   environment;
        miTag     env     = *mi_eval_tag(&param->environment);
        miTag     org_env = state->environment;
        miColor   work;
        double    fo_start, fo_end;
        miScalar  environment_weight = 0.0;
        miVector  u, v, original_normal;
        double   sample[2];  /* mi_sample vector */
        int      count = 0;  /* mi_sample counter */
        int      actualsamples = 0;
        miScalar dir_dot_u, dir_dot_v;
        double   dot_nd = state->dot_nd;
        miScalar edge_factor  = *mi_eval_scalar(&param->edge_factor);
        miScalar base_weight = *mi_eval_scalar(&param->base_weight);
        miScalar edge_weight = *mi_eval_scalar(&param->edge_weight);
        miScalar falloff     = *mi_eval_scalar(&param->falloff);
        int      n_spectrum   = 0;
        miColor *spectrum     =  NULL;        
        miTag    tag          =  miNULLTAG;
        miBoolean multi       =  miFALSE; /* Are we multisampling */

        miBoolean first_ray   =  miTRUE; /* First ray case */

        if (state->type == miRAY_SHADOW ||
            state->type == miRAY_DISPLACE ||
            state->type == miRAY_LIGHT)
            return miFALSE;

        /* 1 ray for final gathering */
        if (final_gather_mode(state))
            samples = 1;

        if (samples <= 0) 
        {
            samples = 1;
            u_spread = v_spread = 0;
        }

        if (rainbow > 0.0)
        {
            n_spectrum = *mi_eval_integer(&param->n_spectrum);
            spectrum = param->spectrum + *mi_eval_integer(&param->i_spectrum);
            if (n_spectrum == 0)
            {
                spectrum   = rainbowdef;
                n_spectrum = 6;
            }            
        }

        if (falloff <= 0.0) falloff = 2.0; /* Default to distance squared */


        /* Compute the u and v deviation vectors */
        calculate_vectors(state, &u, &v, &param->u_axis, &param->v_axis, 
                          u_spread != v_spread);

        if (rainbow > 0.0)
        {
            dir_dot_v = mi_vector_dot(&state->dir, &u);
            dir_dot_u = mi_vector_dot(&state->dir, &u);
        }

        /* Is there a base material? Eval it! */
        if (miNULLTAG != (tag = *mi_eval_tag(&param->surface)))
        {
            mi_call_shader(result, miSHADER_TEXTURE, state, tag);
        }

        /* if (state->reflection_level > 1) 
            samples = 1; */

        /* Clear work colors */
        work.r = work.g = work.b = 0.0;
        environment.r = environment.g = environment.b = 0.0;

        if (reflection_depth > 0.0)
        {
            fo_start = fo_end = reflection_depth;
            mi_ray_falloff(state, &fo_start, &fo_end);
        }

        /* Remember original normal */
        original_normal = state->normal;

        /* Use custom environment if passed */
        org_env = state->environment;
        if (env) state->environment = env;

        if (samples > 40000) samples = 1;

        /* Test if we are already multisampling? */
        multi = test_multisampling(state);
        
        if (multi) /* If so, sample once */
            samples = 1;
        else /* If not, mark that we ARE multisampling if samples > 1 */
            set_multisampling(state, (samples > 1)?miTRUE:miFALSE);

        /* Optimize away disabled reflection */
        if (edge_weight <= 0.0 && base_weight <= 0.0) 
            samples = 0;

        while (samples && mi_sample(sample, &count, state, 2, &samples))   
        {
            miColor  thiscolor;
            miVector thisdir;
            miScalar angle = (miScalar)sample[0] * 2.0F * M_PI_F;          
            miScalar u_dev = (miScalar)(sin(angle) * sqrt(sample[1]));
            miScalar v_dev = (miScalar)(cos(angle) * sqrt(sample[1])); 
            miScalar rbf   = 0.0; /* Rainbowfactor */
            miScalar weight= 1.0;
            miScalar edge_level;
            /* Modify normal */
            state->normal.x = original_normal.x + u_dev * u.x * u_spread 
                              + v_dev * v.x * v_spread;
            state->normal.y = original_normal.y + u_dev * u.y * u_spread 
                              + v_dev * v.y * v_spread;
            state->normal.z = original_normal.z + u_dev * u.z * u_spread 
                              + v_dev * v.z * v_spread;
            /* Calculate refracted ray dir */
            mi_vector_normalize(&state->normal);

            state->dot_nd = mi_vector_dot(&state->normal, &state->dir);

            /* Do not use mr reflection dir, but do manually with rejection */
            thisdir.x = state->dir.x - state->normal.x * state->dot_nd * 2.0f;
            thisdir.y = state->dir.y - state->normal.y * state->dot_nd * 2.0f;
            thisdir.z = state->dir.z - state->normal.z * state->dot_nd * 2.0f;
            /* Test for below-the-plane */
            if (mi_vector_dot(&thisdir, &state->normal_geom) < 0.0)
            {
                /* First ray that goes below the plane is treated traditionally 
                   (to ensure at least one ray actually gets sent) */
                if (first_ray)
                {
                    mi_reflection_dir(&thisdir, state); 
                    first_ray = miFALSE;
                }
                else
            	    continue; /* ...the rest are rejected */
            }
            
            actualsamples++;

            /* Edge multiplier */
            edge_level = (miScalar)pow(1.0 + (dot_nd<0.0?dot_nd:0.0), 
                            (double) edge_factor);

            weight = edge_weight * edge_level + base_weight * (1.0F - edge_level);


            rbf = v_dev * u_dev; /* dir_dot_u +
                  u_dev * dir_dot_v; */

            if (mi_trace_reflection(&thiscolor, state, &thisdir))
            {
                miScalar depth = state->child->dist;

                if  (depth == 0.0 || 
                    (reflection_depth > 0.0 && depth > reflection_depth))
                {
                    if (sing_env) 
                    {
                        environment_weight += 1.0;
                    }
                    else
                    {
                        miColor ref;
                        mi_trace_environment(&ref, state, &thisdir);

                        if (rainbow > 0.0)
                            affect_rainbow(&ref, rbf, rainbow, spectrum,
                                           n_spectrum);

                        environment.r += ref.r * weight;
                        environment.g += ref.g * weight;
                        environment.b += ref.b * weight;
                    }
                }
                else
                {
                    miScalar factor = 1.0;
                    miColor ref; /* environment reflection color */
                    
                    if (reflection_depth > 0.0)
                    {
                        factor = 1.0F - (depth / reflection_depth);
                        /* Tweak fallof by power funcion */
                        factor = pow(factor, falloff);
                    }

                    if (rainbow > 0.0)
                        affect_rainbow(&thiscolor, rbf, rainbow, spectrum, 
                                       n_spectrum);

                    if (sing_env)
                    {
                        environment_weight += (1.0F - factor);
                    }
                    else
                    {
                        mi_trace_environment(&ref, state, &thisdir);
                        if (rainbow > 0.0)
                            affect_rainbow(&ref, rbf, rainbow, spectrum, 
                                           n_spectrum);
                    }


                    work.r += thiscolor.r * factor * weight;
                    work.g += thiscolor.g * factor * weight;
                    work.b += thiscolor.b * factor * weight;

                    if (!sing_env)
                    {
                        environment.r += ref.r * (1.0F - factor) * weight;
                        environment.g += ref.g * (1.0F - factor) * weight;
                        environment.b += ref.b * (1.0F - factor) * weight;
                    }
                }
            }
            else /* Missed ray? */
            {
                if (sing_env)
                {
                    environment_weight += 1.0;
                }
                else
                {
                    miColor ref;
                    mi_trace_environment(&ref, state, &thisdir);

                    if (rainbow > 0.0)
                        affect_rainbow(&ref, rbf, rainbow, spectrum,
                                       n_spectrum);

                    environment.r += ref.r * weight;
                    environment.g += ref.g * weight;
                    environment.b += ref.b * weight;
                }
            }
        }

        /* Reset the shaderstate */
        if (!multi) set_multisampling(state, miFALSE);

        if (actualsamples > 0)
        {
            work.r /= actualsamples;
            work.g /= actualsamples;
            work.b /= actualsamples;
        
            if (sing_env)
            {
                environment_weight /= actualsamples;
            }
            else
            {
                environment.r /= actualsamples;
                environment.g /= actualsamples;
                environment.b /= actualsamples;
            }
        }

        /* Reset trace depth */
        if (reflection_depth > 0.0)
            mi_ray_falloff(state, &fo_start, &fo_end);

        state->dot_nd = dot_nd;
        state->normal = original_normal;
        state->environment = org_env;

        /* Single environment sample mode */
        if (sing_env && environment_weight > 0.0)
        {
            miVector thisdir;
            miScalar edge_level, weight;
            /* Calculate reflected ray dir */
            mi_reflection_dir(&thisdir, state);
            mi_trace_environment(&environment, state, &thisdir);

            /* Edge multiplier */
            edge_level = (miScalar)pow(1.0 + (state->dot_nd<0.0?state->dot_nd:0.0), 
                            (double) edge_factor);
            weight = edge_weight * edge_level + base_weight * 
				(1.0F - edge_level);

            environment.r *= environment_weight * weight;
            environment.g *= environment_weight * weight;
            environment.b *= environment_weight * weight;
        }
 
        result->r += (work.r * reflection_color.r) + 
                     (environment.r * environment_color.r);
        result->g += (work.g * reflection_color.g) + 
                     (environment.g * environment_color.g);
        result->b += (work.b * reflection_color.b) + 
                     (environment.b * environment_color.b);
	return(miTRUE);
}


typedef struct {
	miTag           top_surface;
	miTag           deep_surface;
        miTag           back_surface;
        miBoolean       flip_normal_for_back;
	miColor		refraction_color;
	miScalar	refraction_depth;
        miScalar        falloff;
        miScalar        base_weight;
        miScalar        edge_weight;
        miScalar        edge_factor;
        miScalar        ior;
	miInteger	samples;
	miScalar	u_spread;
        miScalar        v_spread;
        miVector        u_axis;
        miVector        v_axis;
        miScalar        rainbowness;
        int             i_spectrum;
        int             n_spectrum;
        miColor         spectrum[1];
} mib_glossy_refraction_t;


extern "C" DLLEXPORT int mib_glossy_refraction_version(void) {return(3);}


extern "C" DLLEXPORT miBoolean mib_glossy_refraction(
	miColor *result,
	miState *state,
	mib_glossy_refraction_t *param)
{
	miColor   refraction_color = *mi_eval_color(&param->refraction_color);
	miScalar  refraction_depth = *mi_eval_scalar(&param->refraction_depth);
	miScalar  u_spread = *mi_eval_scalar(&param->u_spread);
	miScalar  v_spread = *mi_eval_scalar(&param->v_spread);
        miScalar  ior      = *mi_eval_scalar(&param->ior);
        miScalar  rainbow  = *mi_eval_scalar(&param->rainbowness);
	miUint    samples = *mi_eval_integer(&param->samples);
	miColor   deep_surface;
        miColor   work;
        double    fo_start, fo_end;
        miScalar  deep_surface_weight = 0.0;
        miVector  u, v, original_normal;
        double   sample[2];  /* mi_sample vector */
        int      count = 0;  /* mi_sample counter */
        int      actualsamples = 0;
        miScalar dir_dot_u, dir_dot_v;
        miScalar dot_nd = state->dot_nd;
        miScalar edge_factor  = *mi_eval_scalar(&param->edge_factor);
        miScalar base_weight = *mi_eval_scalar(&param->base_weight);
        miScalar edge_weight = *mi_eval_scalar(&param->edge_weight);
        miScalar falloff     = *mi_eval_scalar(&param->falloff);

        int      n_spectrum   = 0;
        miColor *spectrum     =  NULL;
        miTag    tag          =  miNULLTAG;
        miBoolean multi       =  miFALSE; /* Are we multisampling */

        if (state->type == miRAY_SHADOW ||
            state->type == miRAY_DISPLACE ||
            state->type == miRAY_LIGHT)
            return miFALSE;

        /* 1 ray for final gathering */
        if (final_gather_mode(state))
            samples = 1;

        if (samples <= 0) 
        {
            samples = 1;
            u_spread = v_spread = 0;
        }

        if (rainbow > 0.0)
        {
            n_spectrum = *mi_eval_integer(&param->n_spectrum);
            spectrum = param->spectrum + *mi_eval_integer(&param->i_spectrum);
            if (n_spectrum == 0)
            {
                spectrum   = rainbowdef;
                n_spectrum = 6;
            }            
        }

        if (falloff <= 0.0) falloff = 2.0; /* Default to distance squared */

        /* Backside? Calculate "deep material" reversed */
        if (state->inv_normal  &&
            (miNULLTAG != (tag = *mi_eval_tag(&param->back_surface)))
            )
        {
            int  flip = *mi_eval_boolean(&param->flip_normal_for_back);

            if (flip)
            {
                mi_vector_neg(&state->normal);
                mi_vector_neg(&state->normal_geom);
                state->inv_normal = miFALSE;
                state->dot_nd = mi_vector_dot(&state->dir, &state->normal);
            }

            mi_call_shader(result, miSHADER_TEXTURE, state, tag);
            
            if (flip)
            {
                mi_vector_neg(&state->normal);
                mi_vector_neg(&state->normal_geom);
                state->inv_normal = miTRUE;
                state->dot_nd = dot_nd;
            }
            return miTRUE;
        }

        /* Backside, reverse IOR */
        if (ior > 0.0 && state->inv_normal) ior = 1.0f / ior;

        /* Compute the u and v deviation vectors */
        calculate_vectors(state, &u, &v, &param->u_axis, &param->v_axis, 
                          u_spread != v_spread);

        if (rainbow > 0.0)
        {
            dir_dot_v = mi_vector_dot(&state->dir, &u);
            dir_dot_u = mi_vector_dot(&state->dir, &u);
        }

        /* Is there a base material? Eval it! */
        if (miNULLTAG != (tag = *mi_eval_tag(&param->top_surface)))
        {
            mi_call_shader(result, miSHADER_TEXTURE, state, tag);
        }

        /* if (state->refraction_level > 1) 
            samples = 1; */

        work.r = work.g = work.b = 0.0;

        if (refraction_depth > 0.0)
        {
            fo_start = fo_end = refraction_depth;
            mi_ray_falloff(state, &fo_start, &fo_end);
        }

        /* Remember original normal */
        original_normal = state->normal;

        /* Test if we are already multisampling? */
        multi = test_multisampling(state);
        
        if (multi) /* If so, sample once */
            samples = 1;
        else /* If not, mark that we ARE multisampling if samples > 1 */
            set_multisampling(state, (samples > 1)?miTRUE:miFALSE);

        /* Optimize away disabled reflection */
        if (edge_weight <= 0.0 && base_weight <= 0.0) 
            samples = 0;

        while (samples && mi_sample(sample, &count, state, 2, &samples))   
        {
            miColor  thiscolor;
            miVector thisdir;
            miScalar angle = (miScalar)sample[0] * 2.0F * M_PI_F;          
            miScalar u_dev = (miScalar)(sin(angle) * sqrt(sample[1]));
            miScalar v_dev = (miScalar)(cos(angle) * sqrt(sample[1]));  
            miScalar rbf   = 0.0; /* Rainbowfactor */
            miScalar weight= 1.0;
            miScalar edge_level;

            if (ior > 0.0)
            {
                state->normal.x = original_normal.x + u_dev * u.x * u_spread + 
                                  v_dev * v.x * v_spread;
                state->normal.y = original_normal.y + u_dev * u.y * u_spread + 
                                  v_dev * v.y * v_spread;
                state->normal.z = original_normal.z + u_dev * u.z * u_spread + 
                                  v_dev * v.z * v_spread;
                mi_vector_normalize(&state->normal);
                state->dot_nd   = mi_vector_dot(&state->dir, &state->normal);
                /* Calculate refracted ray dir */
                if (!mi_refraction_dir(&thisdir, state, 1.0, ior))
                    continue;
            }
            else
            {
                thisdir = state->dir;
                thisdir.x += u_dev * u.x * u_spread + v_dev * v.x * v_spread;
                thisdir.y += u_dev * u.y * u_spread + v_dev * v.y * v_spread;
                thisdir.z += u_dev * u.z * u_spread + v_dev * v.z * v_spread;
                mi_vector_normalize(&thisdir);
            }

            /* Edge multiplier */
            edge_level = (miScalar)pow(1.0 + (state->dot_nd<0.0?state->dot_nd:0.0), 
		            (double) edge_factor);
            weight = edge_weight * edge_level + base_weight * 
				(1.0F - edge_level);

            actualsamples++;

            rbf = u_dev /* dir_dot_u*/ *
                  v_dev /* dir_dot_v*/;

            if (mi_trace_refraction(&thiscolor, state, &thisdir))
            {
                miScalar depth = (miScalar)state->child->dist;

                if (refraction_depth > 0.0 && 
                    (depth > refraction_depth || depth == 0.0))
                {
                    deep_surface_weight += 1.0;
                }
                else
                {
                    miScalar factor = 1.0;
                    
                    if (refraction_depth > 0.0)
                    {
                        factor = 1.0F - (depth / refraction_depth);
                        /* Tweak fallof by power funcion */
                        factor = pow(factor, falloff);
                    }

                    deep_surface_weight += (1.0F - factor);

                    if (rainbow > 0.0)
                    {
                        affect_rainbow(&thiscolor, rbf, rainbow, 
					spectrum, n_spectrum);
                    }

                    work.r += thiscolor.r * factor * weight;
                    work.g += thiscolor.g * factor * weight;
                    work.b += thiscolor.b * factor * weight;
                }
            }
            else /* Missed ray? */
            {
                if (refraction_depth > 0.0) 
                    deep_surface_weight += 1.0;
            }
        }

        /* Reset the shaderstate */
        if (!multi) set_multisampling(state, miFALSE);

        state->normal = original_normal;
        state->dot_nd = dot_nd;

        if (actualsamples > 0)
        {
            work.r /= actualsamples;
            work.g /= actualsamples;
            work.b /= actualsamples;
            deep_surface_weight /= actualsamples;
        }

        /* Reset trace depth */
        if (refraction_depth > 0.0)
            mi_ray_falloff(state, &fo_start, &fo_end);
        else
            deep_surface_weight = 0.0;
        
        if (deep_surface_weight > 0.0)
        {
            deep_surface.r = deep_surface.g = deep_surface.b = 0.0;
            /* Is there a base material? Eval it! */
            if (miNULLTAG != (tag = *mi_eval_tag(&param->deep_surface)))
            {
                mi_call_shader(&deep_surface, miSHADER_TEXTURE, state, tag);
            }
            work.r += deep_surface.r * deep_surface_weight;
            work.g += deep_surface.g * deep_surface_weight;
            work.b += deep_surface.b * deep_surface_weight;
        }

        result->r += (work.r * refraction_color.r);
        result->g += (work.g * refraction_color.g);
        result->b += (work.b * refraction_color.b);
	return(miTRUE);
}
