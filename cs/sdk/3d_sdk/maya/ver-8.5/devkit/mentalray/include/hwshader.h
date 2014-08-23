/******************************************************************************
 * Copyright 2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin
 * All rights reserved.
 ******************************************************************************
 * Author:	jurgen
 * Created:	09.06.2006
 * Purpose:	extended header file for mental ray hardware shaders
 *
 * Note: This file is generated automatically from the mental ray sources, do
 * not edit. Some definitions exist for internal reasons and are subject to
 * change between mental ray versions. See the mental ray user's manual for
 * a definitive description of the shader interface.
 *****************************************************************************/

#ifndef HWSHADER_H
#define HWSHADER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GEOSHADER_H 
 #error you must include shader.h and geoshader.h before hwshader.h
#endif


/*------------ mi_raylib.h ------------*/

/*begin_geoshader_h*/
/*begin_mirelay_h*/

/** define C linkage for function prototypes */
#ifndef miC_LINKAGE
#if defined(__cplusplus)
#define miC_LINKAGE extern "C"
#else
#define miC_LINKAGE
#endif
#endif

/*end_mirelay_h*/
/*end_geoshader_h*/

/*------------ mi_scene.h ------------*/

/*
 * Macro for assigning a material tag (first argument) from the triangle
 * material tag (second argument), or, if this is null, from the material
 * in the instance (third argument). If the instance material is a material
 * list, index it with the label if available. All material tag accesses in
 * RC and GAP go through this macro. It implements material inheritance.
 */

/* ip is a pointer to an instance */
#define miSCENE_GET_MATERIAL_P(mtl, tri_mtl, ip, mtl_is_label)		     \
	do {								     \
            int _tri_mtl = (int)(tri_mtl);				     \
	    if (!(mtl=(miTag)(_tri_mtl)) || mtl_is_label || ip->mtl_override)\
		    if (!ip->mtl_array_size)				     \
			    mtl = ip->material;				     \
		    else {						     \
			    miTag *lp =					     \
				(miTag *)mi_db_access(ip->material);	     \
			    mtl = lp[mtl_is_label  &&		             \
				     _tri_mtl >= 0 &&		             \
				     _tri_mtl < ip->mtl_array_size ?         \
						    _tri_mtl : 0];	     \
			    mi_db_unpin(ip->material);			     \
		    }							     \
	} while (0)

/*------------ mi_scene_db.h ------------*/

/*****************************************************************************
 **			  hardware shaders				    **
 *****************************************************************************
 *
 * Element type:    miSCENE_HARDWARE
 * Data type:	    miHardware
 * Sizes:	    ---
 * Defaults:	    all nulls
 */

typedef enum miHardware_vendor {
	miHWV_GENERIC	= 0,
	miHWV_NVIDIA	= 1,
	miHWV_ATI	= 2
} miHardware_vendor;

typedef enum miHardware_return {
	miHWR_COL	= 0,		/* return fp color */
	miHWR_COL_Z	= 1,		/* return fp color and z-depth*/
	miHWR_COLCLIP	= 2,		/* return clipped color */
	miHWR_COLCLIP_Z = 3,		/* return clipped color and z depth */
	miHWR_DRAWTRI   = 4             /* tells that the triangles were draw*/
} miHardware_return;

typedef struct miHardware {
	miBoolean		hardware;	/* 1 true if HW rendering */
	miHardware_vendor	vendor; 	/* 2 identify vendor */
	int			version;	/* 3 hardware version id */
	miBoolean		strips; 	/* 4 has triangle strips */
	int			height; 	/* 5 image height in pixels */
	int			width;		/* 6 image width in pixels */
	int			max_size;	/* 7 max img size in bytes */
	int			no_images;	/* 8 number of frame buffers */
	enum miImg_type		image_types[2];	/* rgba, z */
	miTag			spare[8];
} miHardware;



typedef struct miVendor {
	miHardware_vendor	vendor;
	int			version;
	miTag			vendor_lib;	/* as reported by mi */
	miTag			vendor_prefix;	/* prepend to all shadernames*/
	miTag			render_init;	/* setup hw renderer */
	miTag			render_fini;	/* shut down hw renderer */
	miTag			render_call;	/* call hw renderer */
	miTag			render_join;	/* merge hw and sw FBs */
	miTag			shader_load;	/* find hw shader and preload*/
	miTag			spare[23];
} miVendor;


/*------------ mi_api.h ------------*/

typedef struct miApi_hardware_attr {
    	miTag	uniform_shader;
	miTag	vertex_shader;
	miTag	fragment_shader;
} miApi_hardware_attr;
void	    *mi_api_hardware_begin	(void);
void	     mi_api_hardware_attr	(void*, char, char*);
miTag        mi_api_hardware_end        (void*);

/*------------ mi_job.h ------------*/

miBoolean mi_job_advise(
	const miTag	*tags,		/* tags to execute */
	int		ntags);		/* number of tags to execute */

/*------------ mi_link.h ------------*/

/*begin_matter_system_h*/
void mi_link_config(	/* initialize: */
    char* l_cmd,		/* ld command (or 0) */
    char* l_options,		/* ld options (or 0) */
    char* c_cmd,		/* cc command (or 0) */
    char* c_options,		/* cc options (or 0) */
    char* l_path1,		/* user-defined path list (or 0) */
    char* l_path2,		/* paths from environment variable (or 0) */
    char* l_path3);		/* mental ray default path list */

void mi_link_set_module_handle(
    void	*mod);		/* handle for shader.lib (really HMODULE) */

void mi_link_register_builtin(
    const char* fname,		/* name of builtin function to be registered */
    const miFunction_ptr func); /* pointer to builtin function */
/*begin_matter_system_h*/
void mi_link_file_add(
    const char*   file,          /* name of a .c, .o or .so/.dll file */
    miBoolean     source,        /* .c instead of .o/.so? */
    miBoolean     verbatim,      /* came from $code .. $end code */
    miBoolean	  delaylink);    /* delay linking until mi_link_delayed() */

void mi_link_file_prefix_add(
    const char*   file,          /* name of a .c, .o or .so/.dll file */
    const char*   prefix,        /* string to be prepended to base file name*/
    miBoolean     source,        /* .c instead of .o/.so? */
    miBoolean     verbatim,      /* came from $code .. $end code */
    miBoolean	  delaylin,      /* delay linking until mi_link_delayed() */
    miBoolean     quiet);        /* keep quiet about load errors */

void mi_link_file_remove(
    const char*   file);         /* name of a .c, .o or .so/.dll file */
void mi_link_file_prefix_remove(
    const char*   fileR,         /* name of a .c, .o or .so/.dll file */
    const char*   prefix,        /* prepend base file name with this string */
    miBoolean     quiet);        /* don't report unload errors */
/*end_matter_system_h*/

/*------------ mi_rc.h ------------*/

/* a structure keeping the base tags needed for rendering */
typedef struct miRc_render_req {
	miTag		options;	/* miOptions */
	miTag		camera;		/* miCamera */
	miTag		camera_inst;	/* miInstance */
	miTag		photonmaps;
        miTag           fg_points;
	miTag		boxes;		/* miRcb_boxes_db */
	miTag		light_data;	/* miRc_light_db */
	miTag		rci_data;	/* intersection processing data */
	miTag		phen_data;	/* function caching */
	miTag		rci_grid;	/* grid acceleration in rci */
	miTag		org_volume;	/* cam./light volume for autovolumes */
        miTag           hardware;       /* hardware rendering job */
	miTag		shmap_inst;	/* shadow map */
	miUint		texsize;	/* texture size for state */
	miTag		spare[2];	/* for future expansion */
} miRc_render_req;
#define miRC_RENDER_REQ_SWAP "15i"
struct miRc_light_data;

/* get local light data from SCENE type */
miC_LINKAGE struct miRc_light_data *mi_rc_init_lights(
	miTag		lights_db,
	miBoolean	*visible_lights);	/* there are visible lights*/

/* release local light data */
miC_LINKAGE void mi_rc_fini_lights(
	struct miRc_light_data *lights);

miC_LINKAGE miBoolean mi_rc_light_info(
	struct miRc_light_data	*lidat,
	const miTag		tag,
	miVector *const		org,
	miVector *const		dir,
	miScalar		time);

/*------------ mi_rcb.h ------------*/

                                                /********************/

/*! \brief bitmaps for the miRcb_leaf::flag member */

#define miRCB_BOX_VISIBLE	(1u << 0)   /*!< object is visible */
#define miRCB_BOX_SHADOW_CAST	(1u << 1)   /*!< cast shadows */
#define miRCB_BOX_SHADOW_RECV	(1u << 2)   /*!< receive shadows */
#define miRCB_BOX_REFLECT_CAST	(1u << 3)   /*!< cast reflection rays */
#define miRCB_BOX_REFLECT_RECV	(1u << 4)   /*!< receive reflection rays */
#define miRCB_BOX_REFRACT_CAST	(1u << 5)   /*!< cast refraction rays */
#define miRCB_BOX_REFRACT_RECV	(1u << 6)   /*!< receive refraction rays */
#define miRCB_BOX_TRANSP_CAST	(1u << 7)   /*!< cast transparanct rays */
#define miRCB_BOX_TRANSP_RECV	(1u << 8)   /*!< receive transparancey rays */

#define miRCB_BOX_CAUSTIC	(1u << 9)   /*!< interact with caustic photon*/
#define miRCB_BOX_GLOBILLUM	(1u << 10)  /*!< interact with glob. photon */ 
#define miRCB_BOX_FINALGATHER	(1u << 11)  /*!< interact with final gather */

#define miRCB_BOX_MOVING	(1u << 12)  /*!< is moving */
#define miRCB_BOX_MOVING_INT	(1u << 13)  /*!< internal motion */
#define miRCB_BOX_MOVING_INST	(1u << 14)  /*!< instance motion */
#define miRCB_BOX_HARDWARE	(1u << 15)  /*!< hardware render */
#define miRCB_BOX_SHADOWMAP	(1u << 16)  /*!< cast shmap shadows*/
#define miRCB_BOX_LOCAL_TESS	(1u << 17)  /*!< local tesselation */

/*! \brief A typedef for a bounding box */
typedef struct miRcb_bbox {
	miVector	min, max;
} miRcb_bbox;
#define miRCB_SWAP_BBOX "5f"
                                                /******************/

/*------------ mi_rchw.h ------------*/


/*-----------------------------------------------------------------------------
 * Describe state of hardware rendering
 */
typedef enum miHardware_render {
        miHW_RENDER_NONE        = 0, /* not enabled */
	miHW_RENDER_TRY		= 1, /* found hw libs, not yet initialized*/
        miHW_RENDER_POSSIBLE    = 2, /* hardware exists and is initialized */
        miHW_RENDER_DONE        = 3, /* we finished hw rendering */
	miHW_RENDER_JOINED	= 4, /* hw and sw fb's were merged */

	miHW_RENDER_STATUS      = 7, /* and with previous to check status */

	/* additional flags, to be or-ed with the others */
        miHW_RENDER_ALL         = 1 << 8, /* render everything using hardware */
        miHW_RENDER_KEEP        = 1 << 9,
        miHW_RENDER_DIAG_ONLY   = 1 << 10,
	miHW_RENDER_DIAG_MESH   = 1 << 11,
	miHW_RENDER_DIAG_LIGHT	= 1 << 12,
	miHW_RENDER_DIAG_BOX	= 1 << 13,
	miHW_RENDER_DIAG_SUBBOX = 1 << 14
} miHardware_render;

/*
 * All hardware renderer options can be added here
 */
typedef struct miHardware_options {
    int		multi_samples;		/* Number of multi-samples */
    int		super_samples;		/* Number of super-samples */
    miBoolean	echo_enable;		/* Echo the Cg? */
    char	*echo_path;		/* Output path */
    miBoolean	echo_error;		/* Output errors only */
    miBoolean   return_display;		/* Push back the old display
					   When using multi-threading, this
					   should be FALSE */
} miHardware_options;


void mi_rchw_init(void);
void mi_rchw_exit(void);
void mi_rchw_abort(void);
const char* mi_rchw_version(void);
miHardware_render mi_hardware_rendering(void);
void mi_rchw_flag_reset(
        miTag           root,
        int             nb_obj,
        const int*      obj_idx);

miBoolean mi_rchw_render(
        miHardware_render   hw_render,
        miTag               hw_tags);

miBoolean mi_rchw_join_image(
	miTag               fbm_tag,
        miHardware_render   hw_render,
        miTag               hw_tags,
	miTag		    render_cprof);

miTag mi_hardware_job_setup(const struct miRc_render_req*);
int mi_vendor_string_add(miTag*, const char*);
int mi_hardware_tags_add(miTag*, const miTag*, int);
miImg_image* mi_hardware_image_begin(miTag*, miImg_type, int, int, int*);
void mi_hardware_image_end(miTag);
miBoolean mi_hardware_enable(void);
const char *mi_vendor_name(const char* name);
void mi_hardware_link_file_add(char*);
miBoolean mi_hardware_shader_name(const struct miInstance*);
miFunction_ptr mi_hardware_shader_get(const struct miInstance*, int, void**);
miFunction_ptr mi_hardware_material_shader_get(const miTag*, void** );
miBoolean mi_rchw_hwpath_config (
	char		*path0,		/* from -T command-line option */
	char		*path1,		/* from miAPI_hwpath_ENV env var */
	char		*path2);		/* default miAPI_DEFAULT_hwpath */

miBoolean mi_hardware_file_get(char* file, char* newfile);
miBoolean mi_rchw_hwpath_get(char ***hwpath_arr);

/* Echo */
miBoolean mi_rchw_echo_path(miBoolean error, char* path);
miBoolean mi_rchw_echo_get(miBoolean *error, char** path);

/* Shader Compiler Options */
miBoolean mi_rchw_compile_opt_set(const char** options);
miBoolean mi_rchw_compile_opt_get(char*** options);
miBoolean mi_rchw_compile_opt_set_from_str(const char* opt_str);
miHardware_options* mi_rchw_get_options(void);

/*begin_mirelay_h*/
/* Configure, if a dedicated thread is used for hardware rendering. This is
   necessary for multithreaded access to hardware rendering. The default is
   off.
*/
void mi_rchw_hardware_thread_enabled(miBoolean enabled);
/*end_mirelay_h*/

/*
 * hardware search paths. Passed to mi_rchw_hwpath_config by ray/ray_option.c
 */

#define miHW_HWPATH_ENV	"MI_HARDWARE_PATH"
#define miHW_DEFAULT_HWPATH "{_MI_REG_HARDWARE};/usr/local/mi/hardware;."



#ifdef __cplusplus
}
#endif

#endif
