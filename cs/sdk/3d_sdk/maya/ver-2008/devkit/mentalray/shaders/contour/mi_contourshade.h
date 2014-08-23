/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Author:      Per Henrik Christensen
 * Created:     96-01-16.
 * Module:      -
 * Purpose:     These functions are examples of the flexible user-specified 
 *              contour shaders possible in ray 2.0
 *
 * Exports: 
 *      contour_store_function()
 *      contour_contrast_function()
 *      contour_shader_simple()
 *      contour_shader_depthfade()
 *      contour_shader_curvature()
 *      etc.
 *
 * History: 
 *	10.07.07 steve:	add silhouette and maxcolor, del _random variants
 *
 * Description:
 *   The contour store function stores information (from the state and
 * material color) needed to compute the location, color, and width of
 * the contours.  The contour contrast function gets two such infos,
 * and decides whether there should be a contour there.  Each material
 * can have a contour shader, and this computes color and width of the
 * contour based on two infos (from neighboring points).
 *****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* 
 * Definitions of data structures and function prototypes for contours
 */

/* Information that is needed by the contour shaders */
typedef struct miStdInfo {
    miVector   point;      /* ray intersection point */
    miVector   normal;     /* ray intersection normal */
    miColor    color;      /* color computed by material shader */
    miTag      material;   /* material tag */
    int        level;      /* refraction level */
    int        label;      /* object label */
    int        index;      /* triangle index */
    miVector   normal_geom;/* geometric normal */
} miStdInfo;

/* Parameters to the contour contrast functions */

typedef struct Contour_Contrast_Parameters {
    miScalar   zdelta;   /* depth difference to cause contour */
    miScalar   ndelta;   /* normal difference to cause contour (in degrees) */
    int        levels;   /* how many levels (layers) of contours? */
} Contour_Contrast_Parameters;

typedef struct Contour_Contrast_Parameters_Levels {
    miScalar   zdelta;    /* depth difference to cause contour */
    miScalar   ndelta;    /* normal difference to cause contour (in degrees) */
    miBoolean  diff_mat;  /* Should borders between materials get contours? */
    miBoolean  diff_label;/* Should different object labels cause contours */
    miBoolean  diff_index;/* Should different triangle indices cause contours*/
    miBoolean  contrast;  /* Should color contrasts give contours? */
    int        min_level; /* nearest level (layer) to get contours */
    int        max_level; /* deepest level (layer) to get contours */
} Contour_Contrast_Parameters_Levels;

/* The parameters to the contour shaders */

typedef struct Simple_Parameters {
    miColor    color;         /* color of contour (default white?) */
    miScalar   width;         /* width of contour (default 0.5?) */
} Simple_Parameters;

typedef struct Depthfade_Parameters {
    miScalar   near_z;        /* linear interpolation from this ... */
    miColor    near_color; 
    miScalar   near_width; 
    miScalar   far_z;         /* ... to this */
    miColor    far_color; 
    miScalar   far_width; 
} Depthfade_Parameters;

typedef struct Framefade_Parameters {
    int        frame1;        /* linear interpol. from this frame number ...*/
    miColor    color1; 
    miScalar   width1; 
    int        frame2;        /* ... to this */
    miColor    color2; 
    miScalar   width2; 
} Framefade_Parameters;

typedef struct Factorcolor_Parameters {
    miScalar    factor;        /* factor of material color */
    miScalar    width;         /* width of contour */
} Factorcolor_Parameters;

typedef struct Layerthinner_Parameters {
    miColor     color;         /* contour color */
    miScalar    width;         /* width of contour at top layer */
    miScalar    factor;        /* factor for each layer */
} Layerthinner_Parameters;

typedef struct Widthfromcolor_Parameters {
    miColor     color;         /* contour color */
    miScalar    min_width;     /* min width of contour */
    miScalar    max_width;     /* max width of contour */
} Widthfromcolor_Parameters;

typedef struct Lightdir_Parameters {
    miColor     color;         /* contour color */
    miScalar    min_width;     /* min width of contour */
    miScalar    max_width;     /* max width of contour */
    miVector    light_dir;     /* direction to main light source */
} Lightdir_Parameters;

typedef struct Light_Parameters {
    miColor     color;         /* contour color */
    miScalar    min_width;     /* min width of contour */
    miScalar    max_width;     /* max width of contour */
    miTag       light;         /* light source */
} Light_Parameters;

typedef struct Combi_Parameters {
    /* Depthfade: */
    miScalar   near_z;        /* linear interpolation from this ... */
    miColor    near_color; 
    miScalar   near_width; 
    miScalar   far_z;         /* ... to this */
    miColor    far_color; 
    miScalar   far_width; 
    /* Layerthinner: */
    miScalar   factor;        /* factor to make thinner for each layer */
    /* Widthfromlight: */
    miTag      light;         /* light source name */
    miScalar   light_min_factor;  /* min width factor for light dependency */
} Combi_Parameters;


/* Parameters to output shaders */

typedef struct Contour_Composite_Parameters {
    miBoolean  glow;         /* glowing contours? */
    miBoolean  maxcomp;      /* composite by alpha or max color? */
} Contour_Composite_Parameters;

typedef struct Contour_Only_Parameters {
    miColor    background;   /* background color (default black) */
    miBoolean  glow;         /* glowing contours? */
    miBoolean  maxcomp;      /* composite by alpha or max color? */
} Contour_Only_Parameters;

typedef struct Postscript_Parameters {
    int        paper_size;
    miScalar   paper_scale;
    miScalar   paper_transform_b;
    miScalar   paper_transform_d;
    miBoolean  title;
    miBoolean  landscape;
    miVector   ink_stroke_dir;
    miScalar   ink_min_frac;
    miTag      file_name;    /* tag of string for file name */
} Postscript_Parameters;




/*
 * Built-in contour store functions.  They store the information
 * needed by the contour contrast function and the contour shaders.
 * This information can depend on the state and the color computed by
 * the material shader.
 */

DLLEXPORT int contour_store_function_version(void);

DLLEXPORT miBoolean contour_store_function(
    void     *info_void,
    int      *info_size,
    miState  *state,
    miColor  *color
);

DLLEXPORT int contour_store_function_simple_version(void);

DLLEXPORT miBoolean contour_store_function_simple(
    miTag    *info,
    int      *info_size,
    miState  *state,
    miColor  *color
);


/* 
 * Built-in contour contrast functions.  Based on the information
 * stored by the contour store function for two adjacent samples, it
 * determines whether there should be a contour between them or not. 
 */

DLLEXPORT int contour_contrast_function_levels_version(void);

DLLEXPORT miBoolean contour_contrast_function_levels(
    miStdInfo  *info1,
    miStdInfo  *info2,
    int         level,
    miState    *state,	   
    Contour_Contrast_Parameters_Levels *paras
);

DLLEXPORT int contour_contrast_function_simple_version(void);

DLLEXPORT miBoolean contour_contrast_function_simple(
    miTag    *info1,
    miTag    *info2,
    int       level,
    miState  *state,	   
    void     *paras   /* no parameters */
);


/* 
 * Built-in contour shaders.  Based on the information stored by
 * the contour store function for two adjacent samples, the contour
 * shader determines what the contour color and width should be.
 *
 * A contour shader return color and width of the contour, along with
 * optional information.  These built-in contour shaders do not return
 * optional information.
 * 
 * A possible extension would be to let the contour shader functions 
 * recursively create a linked list of depth-sorted contour widths 
 * and colors. 
 */

/* Fixed color and width of the contour, corresponding to ray 1.9. */
DLLEXPORT int contour_shader_simple_version(void);

DLLEXPORT miBoolean contour_shader_simple(
    miContour_endpoint *result,
    miStdInfo          *info_near,
    miStdInfo          *info_far,
    miState            *state,
    Simple_Parameters  *paras
);

/* Contours at silhouette. */
DLLEXPORT int contour_shader_silhouette_version(void);

DLLEXPORT miBoolean contour_shader_silhouette(
    miContour_endpoint *result,
    miStdInfo          *info_near,
    miStdInfo          *info_far,
    miState            *state,	   
    Simple_Parameters  *paras
);

/* Contours with max color. */
DLLEXPORT int contour_shader_maxcolor_version(void);

DLLEXPORT miBoolean contour_shader_maxcolor(
    miContour_endpoint *result,
    miStdInfo          *info_near,
    miStdInfo          *info_far,
    miState            *state,	   
    float	       *width
);

/* The width of the contour fades into the background (from near_width
 * to far_width), and the color fades from near_color to far_color 
 * The contour width and color changes with a ramp function between 
 * distances near_z and far_z */
DLLEXPORT int contour_shader_depthfade_version(void);

DLLEXPORT miBoolean contour_shader_depthfade(
    miContour_endpoint    *result, 
    miStdInfo             *info_near,
    miStdInfo             *info_far,
    miState               *state,	   
    Depthfade_Parameters  *paras
);

/* The color and width of the contour depends linearly on the frame 
 * number (ramp function). */
DLLEXPORT int contour_shader_framefade_version(void);

DLLEXPORT miBoolean contour_shader_framefade(
    miContour_endpoint    *result, 
    miStdInfo             *info_near,
    miStdInfo             *info_far,
    miState               *state,	   
    Framefade_Parameters  *paras
);

/* The width of the contour changes by a factor each time the ray
 * gets deeper */
DLLEXPORT int contour_shader_layerthinner_version(void);

DLLEXPORT miBoolean contour_shader_layerthinner(
    miContour_endpoint       *result,
    miStdInfo                *info_near,
    miStdInfo                *info_far,
    miState                  *state,	   
    Layerthinner_Parameters  *paras
);

/* The width of the contour depends on the curvature, i.e. the
 * difference between the two normals. */
DLLEXPORT int contour_shader_curvature_version(void);

DLLEXPORT miBoolean contour_shader_curvature(
    miContour_endpoint         *result,
    miStdInfo                  *info_near,
    miStdInfo                  *info_far,
    miState                    *state,	   
    Widthfromcolor_Parameters  *paras
); 

/* The color of the contour is a factor times the material color.
   factor = 0 gives black contours, 
   0 < factor < 1 gives dark contours,
   factor = 1 gives contours the same color as the object material,
   factor > 1 gives bright contours.
   The width is constant. */
DLLEXPORT int contour_shader_factorcolor_version(void);

DLLEXPORT miBoolean contour_shader_factorcolor(
    miContour_endpoint      *result,
    miStdInfo               *info_near,
    miStdInfo               *info_far,
    miState                 *state,	   
    Factorcolor_Parameters  *paras
);

/* The color of the contour is constant.  The width depends on the
   material color. */
DLLEXPORT int contour_shader_widthfromcolor_version(void);

DLLEXPORT miBoolean contour_shader_widthfromcolor(
    miContour_endpoint         *result,
    miStdInfo                  *info_near,
    miStdInfo                  *info_far,
    miState                    *state,	   
    Widthfromcolor_Parameters  *paras
); 

/* The color of the contour is a parameter.  The width depends on the
   surface normal relative to a light source direction. */
DLLEXPORT int contour_shader_widthfromlightdir_version(void);

DLLEXPORT miBoolean contour_shader_widthfromlightdir(
    miContour_endpoint   *result,
    miStdInfo            *info_near,
    miStdInfo            *info_far,
    miState              *state,	   
    Lightdir_Parameters  *paras
); 

/* The color of the contour is a parameter.  The width depends on the
 * surface normal relative to a light source direction. 
 * light dir = normal:   width = max_width 
 * light dir = -normal:  width = min_width */
DLLEXPORT int contour_shader_widthfromlight_version(void);

DLLEXPORT miBoolean contour_shader_widthfromlight(
    miContour_endpoint  *result,
    miStdInfo           *info_near,
    miStdInfo           *info_far,
    miState             *state,	   
    Light_Parameters    *paras 
);

/* This is a combination of the depthfade, layerthinner, and widthfromlight
 * contour shaders.
 * The width of the contour fades into the background (from near_width
 * to far_width), and the color fades from near_color to far_color.
 * The contour width and color changes with a ramp function between 
 * distances near_z and far_z.
 * For each layer the ray has passed through, a factor is multiplied on
 * to the width.
 * If the light source is different from the NULLtag, the width also 
 * depends on the surface normal relative to a light source direction. */
DLLEXPORT int contour_shader_combi_version(void);

DLLEXPORT miBoolean contour_shader_combi(
    miContour_endpoint  *result,
    miStdInfo           *info_near,
    miStdInfo           *info_far,
    miState             *state,	   
    Combi_Parameters    *paras
);


/* 
 * Built-in contour output shaders.
 */
DLLEXPORT int contour_composite_version(void);

DLLEXPORT miBoolean contour_composite(
    void                          *result,   /* unused */
    miState                       *state,
    Contour_Composite_Parameters  *paras
);

DLLEXPORT int contour_only_version(void);

DLLEXPORT miBoolean contour_only(
    void                     *result,   /* unused */
    miState                  *state,
    Contour_Only_Parameters  *paras
);

DLLEXPORT int contour_ps_version(void);

DLLEXPORT miBoolean contour_ps (
    miColor                *result,   /* unused */
    miState                *state,
    Postscript_Parameters  *paras
);

#if defined(__cplusplus)
} // extern "C"
#endif
