/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	18.1.96
 * Module:	contour
 * Purpose:	standard contour shaders: output
 *
 * Exports:
 *	contour_composite_version
 *	contour_composite
 *	contour_only_version
 *	contour_only
 *
 * History:
 *	14.9.01: reindented in manual style for publication
 *
 * Description:
 * Get contour line segments and convert them into polygons. The polygons are
 * rasterized at high resolution and then downsampled (to avoid aliasing).
 *
 * It would be fairly easy to parallelize the filtering of the high-resolution
 * contour image (it is already filtered in little blocks/tasks at a time).
 * Unfortunately the computation of the contour image cannot be done at the
 * individual tasks without generating incorrect contours when the contours
 * are wide and at a grazing angle to the task boundary. Therefore this has
 * to be done at the master host.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <shader.h>
#include <mi_contourshade.h>

#define REGIONSIZEX	64		/* Region size for filter (in pixels)*/
#define REGIONSIZEY	64
#define SUBYRES		8		/* Subpixel resolution per pixel */
#define SUBXRES		8
#define MODRES(y)	((y) & 7)	/* Subpixel Y modulo */
#define miEPS		0.0001

#define mi_MIN(a, b)	((a) < (b) ? (a) : (b))
#define mi_MAX(a, b)	((a) > (b) ? (a) : (b))

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif


typedef struct {	/* polygon vertex */
	double x, y;	/* subpixel display coordinate */
} miPolygonVertex;

typedef struct {
	miColor color;
	double	depth;
} miColorDepth;

/* A high-resolution pixel consists of SUBXRES*SUBYRES colors and depths */
typedef miColorDepth	miPixel[SUBXRES*SUBYRES];



/*
 * Global variables
 */

static miMemblk	*pixel_pages = NULL;	/* Pages for mi_mem_blkcreate() */
static miMemblk *region_pixel_pages = NULL;
static int	 image_xsize;		/* image size in pixels */
static int	 image_ysize;
static miBoolean max_composite;		/* max composite or alpha composite? */
static miBoolean glow;			/* glowing contours? */
static int	 npixels=0, npixels2=0;	/* for debugging */

/* Color high-resolution images: 2d array of pointers to miColorDepth */
static miColorDepth ***hires_image;
static miColorDepth ***region_image2;
static miColorDepth ***region_image3;


/* these are all static (i.e. local) */

static void init_hires_contourimg(int xsize, int ysize);
static void fini_hires_contourimg(void);

static void draw_line(miContour_endpoint *p1, miContour_endpoint *p2);
static void draw_polygon_screen(miPolygonVertex polygon[], int numVertex,
			miContour_endpoint *p1, miContour_endpoint *p2);
static void render_scanline(int y_pixel, int xLmin, int xLmax, int xRmin,
			int xRmax, int *sp_xLeft, int *sp_xRight,
			miContour_endpoint *p1, miContour_endpoint *p2);
static void compute_subpixel_mask(unsigned char mask[], int x, int xLmax,
			int xRmin, int *sp_xLeft, int *sp_xRight);
static void interpolate_color_depth(miColor *color, double *depth,
			miContour_endpoint *p1, miContour_endpoint *p2,
			int x_pixel, int y_pixel);
static void set_subpixels(unsigned char *mask, int i, int j, miColor *color,
			double depth);

static void filter_contour_image(miImg_image *image);
static double lanczos_2( double arg);
static void tabulate_filter(void);
static void init_region_images(void);
static void fini_region_images(void);
static void lanczos2_filter_y(int xmin, int ymin, int xmax, int ymax);
static void lanczos2_filter_x(int xmin, int ymin, int xmax, int ymax);
static void box_filter(miImg_image *image, int xmin, int ymin, int xmax,
			int ymax );



/*
 * Computing the contour image:
 * Output shaders for computing contour images.	These shaders (like all
 * output shaders) are on the master only.
 *
 * This procedure allocates a high-resolution image, filters it (using
 * lanczos_2 filter to avoid aliasing) downsamples it, writes the resulting
 * image, and releases the high-resolution image.
 */

int contour_composite_version(void) {return(2);}

miBoolean contour_composite(
	void		   *result,	/* unused */
	miState		   *state,
	Contour_Composite_Parameters *paras)
{
	static const miScalar	sum_max = 20000.0;
	miScalar		cost = 0.0;			
	miScalar		dcost;
	miContour_endpoint 	p1;
	miContour_endpoint 	p2;
	miBoolean		aborted = miFALSE;
	int			line_count = 0;

	mi_info("computing contours");

	image_xsize	= state->camera->x_resolution;
	image_ysize	= state->camera->y_resolution;
	glow		= paras->glow;
	max_composite	= paras->maxcomp;	/* max or alpha blend? */

	/* Allocate memory for high-resolution contour image */
	init_hires_contourimg(image_xsize, image_ysize);

	/* Write the contours in the high-resolution contour image */
	while (!aborted && mi_get_contour_line(&p1, &p2)) {
		/* cost of line drawing i dependent on line thickness. */
		dcost = p1.width + p2.width;
		dcost *= dcost;
		cost += mi_MAX(0.01, dcost);
		++line_count;
		draw_line(&p1, &p2);
		if(cost > sum_max) {
			/* chat up user, so he knows we are still working */
			mi_progress("processed %d contour lines", line_count);
			aborted = mi_par_aborted();
			cost = 0.0;
		}
	}

	if( !aborted ) {
		miImg_image	   	*img;
		/* Filter contour subpixel image to image resolution. 
		 * Composite over image 
		 */
		img = mi_output_image_open(state, miRC_IMAGE_RGBA);
		filter_contour_image(img);
		mi_output_image_close(state, miRC_IMAGE_RGBA);
	}

	/* Finish the contour computation for this task: free memory */
	fini_hires_contourimg();

	mi_info("contours computed");
	return(miTRUE);
}


/*
 * Same as contour_composite() above, except the image is cleared first
 */

int contour_only_version(void) {return(3);}

miBoolean contour_only(
	void			*result,	/* unused */
	miState			*state,
	Contour_Only_Parameters	*paras)
{
	miImg_image		*img;
	Contour_Composite_Parameters compparas;
	int			i, j;

	/* Set the image background color (default black) */
	img = mi_output_image_open(state, miRC_IMAGE_RGBA);
	for (i=0; i < state->camera->y_resolution; i++)
		for (j=0; j < state->camera->x_resolution; j++)
			mi_img_put_color(img, &paras->background, j, i);
	mi_output_image_close(state, miRC_IMAGE_RGBA);
	/* Compute contour and composite it over the background color */
	compparas.glow    = paras->glow;
	compparas.maxcomp = paras->maxcomp;
	return(contour_composite(result, state, &compparas));
}


/*
 * Initialize the contour computations on master:
 * If the output is an image, allocate the contour image. Initialize the
 * filter used for downsampling the contour image.
 *
 * Line drawing routines.
 */

#define NUM_VERTEX	10			/* decagon */
#define SQRT1_2		0.7071067811865475

static void draw_line(
	miContour_endpoint *p1,
	miContour_endpoint *p2)
{
	double		r1, r2;
	double		r1_sqrt1_2, r2_sqrt1_2;
	double		t_x, t_y;
	double		fs_x, fs_y, ls_x, ls_y;
	double		r_norm;
	miPolygonVertex	polygon[NUM_VERTEX];

	r1 = 0.5 * p1->width;
	r2 = 0.5 * p2->width;
	r1_sqrt1_2 = r1 * SQRT1_2;
	r2_sqrt1_2 = r2 * SQRT1_2;

	fs_x = p1->point.x;
	fs_y = p1->point.y;
	ls_x = p2->point.x;
	ls_y = p2->point.y;

	/* Unit direction vector */
	t_x    = ls_x - fs_x;
	t_y    = ls_y - fs_y;
	miASSERT(t_x*t_x + t_y*t_y > miEPS);
	r_norm = 1.0 / sqrt(t_x*t_x + t_y*t_y);
	t_x   *= r_norm;
	t_y   *= r_norm;

	/*
	 * Construction of the wide line with rounded edges as a decagon.
	 * The circle is approximated by an octogon, which allows
	 * some optimizations. Optimize further by using the sum and the
	 * difference of t_x and t_y.
	 */

	/* vertex 0; angle = 3 Pi / 2 */
	polygon[0].x = fs_x + r1 *   t_y;
	polygon[0].y = fs_y + r1 * (-t_x);

	/* vertex 1; angle = 5 Pi / 4 */
	polygon[1].x = fs_x + r1_sqrt1_2 * (-t_x + t_y);
	polygon[1].y = fs_y + r1_sqrt1_2 * (-t_x - t_y);

	/* vertex 2; angle = Pi */
	polygon[2].x = fs_x + r1 * (-t_x);
	polygon[2].y = fs_y + r1 * (-t_y);

	/* vertex 3; angle = 3 Pi / 4 */
	polygon[3].x = fs_x + r1_sqrt1_2 * (-t_x - t_y);
	polygon[3].y = fs_y + r1_sqrt1_2 * ( t_x - t_y);

	/* vertex 4; angle = Pi / 2 */
	polygon[4].x = fs_x + r1 * (-t_y);
	polygon[4].y = fs_y + r1 *   t_x;

	/* vertex 5; angle = Pi / 2 */
	polygon[5].x = ls_x + r2 * (-t_y);
	polygon[5].y = ls_y + r2 *   t_x;

	/* vertex 6; angle = Pi / 4 */
	polygon[6].x = ls_x + r2_sqrt1_2 * (t_x - t_y);
	polygon[6].y = ls_y + r2_sqrt1_2 * (t_x + t_y);

	/* vertex 7; angle = 0 */
	polygon[7].x = ls_x + r2 * t_x;
	polygon[7].y = ls_y + r2 * t_y;

	/* vertex 8; angle = - Pi / 4 */
	polygon[8].x = ls_x + r2_sqrt1_2 * ( t_x + t_y);
	polygon[8].y = ls_y + r2_sqrt1_2 * (-t_x + t_y);

	/* vertex 9; angle = - Pi / 2 */
	polygon[9].x = ls_x + r2 *   t_y;
	polygon[9].y = ls_y + r2 * (-t_x);

	draw_polygon_screen(polygon, NUM_VERTEX, p1, p2);
}


/*
 * Write polygons into high-resolution contour image:
 * This code renders a polygon, computing subpixel coverage at SUBYRES times Y
 * and SUBXRES times X display resolution for anti-aliasing.
 */

/* Subpixel X beyond right edge */
#define MAX_X	miHUGE_INT

/* Linear interpolation */
#define ILERP(t, x, y)	(int)(x + t * (y-x))

/* Rasterize a convex polygon */

static void draw_polygon_screen(
	miPolygonVertex	polygon[],	/* clockwise clipped vertex list */
	int		numVertex,	/* number of vertices in polygon */
	miContour_endpoint *p1,
	miContour_endpoint *p2)
{
	miPolygonVertex	*Vleft, *VnextLeft;	/* current left edge */
	miPolygonVertex	*Vright, *VnextRight;	/* current right edge */
	miPolygonVertex	*endPoly;		/* end of polygon vertex list */
	double		aLeft, aRight;		/* interpolation ratios */
	double		xLeft, xNextLeft;	/* subpixel coordinates for */
	double		xRight, xNextRight;	/* active polygon edges */
	int		i, y, mod;
	int		y_max = image_ysize * SUBYRES;
	int		sp_xLeft[SUBYRES],sp_xRight[SUBYRES]; /* subpixel */
						/* extents for scanline*/

	int		xLmin, xLmax;		/* subpixel x extremes for */
						/* scanline */
	int		xRmax, xRmin;		/* for optimization shortcut */
	int		done = 0;

	/* Convert to subpixel-resolution coordinates */
	for (i=0; i < numVertex; i++) {
		polygon[i].x = SUBXRES * polygon[i].x - 0.5;
		polygon[i].y = SUBYRES * polygon[i].y - 0.5;
	}

	/* find vertex with minimum y (display coordinate) */
	Vleft = polygon;
	for (i=1; i < numVertex; i++)
		if (polygon[i].y < Vleft->y)
			Vleft = &polygon[i];
	endPoly = &polygon[numVertex-1];

	/* initialize scanning edges */
	Vright = VnextRight = VnextLeft = Vleft;

	/* prepare bottom of initial scanline - no coverage by polygon */
	for (i=0; i<SUBYRES; i++)
		sp_xLeft[i] = sp_xRight[i] = -1;
	xLmin = xRmin = -1;
	xLmax = xRmax = MAX_X;

	/* scan convert for each subpixel from bottom to top */
	for (y=(int)Vleft->y + 1; ; y++) {
		while ((int)VnextLeft->y < y) { /* reached next left vertex */
			VnextLeft = (Vleft=VnextLeft) + 1;	/* advance */
			if (VnextLeft > endPoly)		/* wraparound*/
				VnextLeft = polygon;
			if (VnextLeft == Vright) {
				done = 1;
				break;
			}
			xLeft     = Vleft->x;
			xNextLeft = VnextLeft->x;
		}

		if (!done)
			while ((int)VnextRight->y < y) {
				/* reached next right vertex */
				VnextRight = (Vright=VnextRight) -1;
				if (VnextRight < polygon)	/* wraparound*/
					VnextRight = endPoly;
				xRight     = Vright->x;
				xNextRight = VnextRight->x;
			}

		if (done || (int)VnextLeft->y < y || (int)VnextRight->y < y) {
			/* done, mark uncovered part of last scanline */
			if (0 <= y && y < y_max) {
				if (!(mod=MODRES(y)))
					return;
				for (; mod < SUBYRES; mod++)
					sp_xLeft[mod] = sp_xRight[mod] = -1;
				xLmin = xRmin = -1;
				xLmax = xRmax = MAX_X;
				render_scanline(y/SUBYRES,
						xLmin, xLmax, xRmin, xRmax,
						sp_xLeft, sp_xRight, p1, p2);
			}
			return;
		}

		/*
		 * Interpolate sub-pixel x endpoints at this y,
		 * and update extremes for pixel coherence optimization
		 */

		if (0 <= y && y < y_max) {
			mod = MODRES(y);
			aLeft = (y - Vleft->y) / (VnextLeft->y - Vleft->y);
			sp_xLeft[mod] = ILERP(aLeft, xLeft, xNextLeft);
			if (sp_xLeft[mod] < xLmin)
				xLmin = sp_xLeft[mod];
			if (sp_xLeft[mod] > xLmax)
				xLmax = sp_xLeft[mod];

			aRight = (y - Vright->y) / (VnextRight->y - Vright->y);
			sp_xRight[mod] = ILERP(aRight, xRight, xNextRight);
			if (sp_xRight[mod] < xRmin)
				xRmin = sp_xRight[mod];
			if (sp_xRight[mod] > xRmax)
				xRmax = sp_xRight[mod];

			if (mod == SUBYRES-1) { /* end of scanline */
				/* interpolate edges to this scanline */
				render_scanline(y/SUBYRES,
						xLmin, xLmax, xRmin, xRmax,
						sp_xLeft, sp_xRight, p1, p2);
				xLmin = xRmin = MAX_X;	/* reset extremes */
				xLmax = xRmax = -1;
			}
		}
	}
}


/*
 * Rasterize one scanline of polygon
 */

static void render_scanline(
	int		y_pixel,	/* scanline coordinate */
	int		xLmin,
	int		xLmax,
	int		xRmin,
	int		xRmax,
	int		*sp_xLeft,	/* sub-pixel coordinates */
	int		*sp_xRight,
	miContour_endpoint *p1,
	miContour_endpoint *p2)
{
	miColor		color;
	double		depth;
	unsigned char	mask[SUBYRES];	/* pixel coverage bitmask */
	int		x;		/* leftmost subpixel of current pixel*/
	int		x_pixel, x_min, x_max;

	miASSERT(0 <= y_pixel && y_pixel < image_ysize);
	x_min = SUBXRES*(int)(xLmin/SUBXRES);
	if (0 > x_min)
		x_min = 0;
	x_max = xRmax;
	if (image_xsize * SUBXRES <= x_max)
		x_max = image_xsize*SUBXRES - 1;
	for (x=x_min; x <= x_max; x += SUBXRES) {
		x_pixel = x/SUBXRES;
		compute_subpixel_mask(mask, x, xLmax, xRmin,
					sp_xLeft, sp_xRight);
		/* or_subpixel_mask(mask, y_pixel, x_pixel); */
		if (mask[0] || mask[1] || mask[2] || mask[3] ||
		    mask[4] || mask[5] || mask[6] || mask[7]) {
			interpolate_color_depth(&color, &depth,
						p1, p2, x_pixel, y_pixel);
			set_subpixels(mask, y_pixel, x_pixel, &color, depth);
		}
	}
}


/*
 * Compute bitmask indicating which subpixels are covered by polygon
 * at current pixel.
 */

static void compute_subpixel_mask(
	unsigned char	mask[],
	int		x,			/* left subpixel of pixel */
	int		xLmax,
	int		xRmin,
	int		*sp_xLeft,
	int		*sp_xRight)
{
	unsigned int	leftMask, rightMask;	/* partial masks */
	int		xr = x+SUBXRES-1;	/* right subpixel of pixel */
	int		y;

	static unsigned int leftMaskTable[] =
			{ 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };
	static unsigned int rightMaskTable[] =
			{ 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };

	/* shortcut for the case of fully covered pixel */
	if (xLmax < x && xr <= xRmin) {
		for (y=0; y < SUBYRES; y++)
			mask[y] = 0xff;
		return;
	}

	for (y = 0; y < SUBYRES; y++) {
		if (sp_xLeft[y] < x)		/* completely left of pixel */
			leftMask = 0xFF;
		else if (xr <= sp_xLeft[y])	/* completely right */
			leftMask = 0;
		else
			leftMask = leftMaskTable[sp_xLeft[y] -x];

		if (xr <= sp_xRight[y])		/* completely right of pixel */
			rightMask = 0xFF;
		else if (sp_xRight[y] < x)	/* completely left */
			rightMask = 0;
		else
			rightMask = rightMaskTable[sp_xRight[y] -x];

		mask[y] = leftMask & rightMask;
	}
}


/*
 * Determine the color at a pixel along a line segment by
 * interpolation of the colors at the endpoints of the line segment
 */

static void interpolate_color_depth(
	miColor		*color,
	double		*depth,
	miContour_endpoint *p1,
	miContour_endpoint *p2,
	int		x_pixel,
	int		y_pixel)
{
	miColor		*color0 = &p1->color;
	miColor		*color1 = &p2->color;
	/*
	 * Convert screen coordinates to raster coordinates at max_samples
	 * resolution
	 */
	double		x = (x_pixel + 0.5);
	double		y = (y_pixel + 0.5);
	double		P12x, P12y, P0ix, P0iy;
	double		w0, w1;
	double		nx, ny, dist, atten, width, ext, len;

	/*
	 * Find the relative length of the pixel-center projection onto the
	 * line segment, this is the weight w1 of endpoint 1
	 */
	P12x = p2->point.x - p1->point.x;
	P12y = p2->point.y - p1->point.y;
	P0ix = x - p1->point.x;
	P0iy = y - p1->point.y;

	w1 = (P12x * P0ix + P12y * P0iy) / (P12x * P12x + P12y * P12y);
	/*
	 * w1 might be slightly less than 0 or larger than 1 if the pixel
	 * center is in one of the rounded ends of the polygon (beyond the
	 * line segment)
	 */
	if (w1 < 0.0)
		w1 = 0.0;
	if (w1 > 1.0)
		w1 = 1.0;
	w0 = 1.0 - w1;
	miASSERT(0.0 <= w0 && w0 <= 1.0);

	color->r = w0 * color0->r + w1 * color1->r;
	color->g = w0 * color0->g + w1 * color1->g;
	color->b = w0 * color0->b + w1 * color1->b;
	color->a = w0 * color0->a + w1 * color1->a;
	*depth = w0 * p1->point.z + w1 * p2->point.z;

	if (glow) {
		width = w0 * p1->width + w1 * p2->width;
		len = sqrt(P12x*P12x + P12y*P12y); /* == sqrt(nx*nx + ny*ny) */

		/* Find the distance of the pixel-center to the line segment */
		nx = -P12y;
		ny = P12x;
		dist = fabs(nx*P0ix + ny*P0iy) / len;

		/* Attenuate color with distance */
		ext = (P0ix*P12x + P0iy*P12y) / len;
		if (ext < 0.0)
			dist = sqrt(P0ix*P0ix + P0iy*P0iy);
		if (ext > len) {
			float P1ix = x - p2->point.x;
			float P1iy = y - p2->point.y;
			dist = sqrt(P1ix*P1ix + P1iy*P1iy);
		}
		atten = 1.0 - 2.0*dist/width;
		if (atten < 0.0)
			atten = 0.0;
		if (atten > 1.0)
			atten = 1.0;
		miASSERT(atten >= 0.0);
		color->r *= atten;
		color->g *= atten;
		color->b *= atten;
		color->a *= atten;
	}
}


/*
 * Operations on sparse high-resolution image:
 * The contour image is huge, it is eight times the regular image in
 * each dimension, that is 64 times as big!	This requires an enormous
 * amount of memory at the master.	Fortunately, the contour image is
 * also very sparse, so we use a representation where there's an image-
 * resolution array of pointers.	If an entire pixel is black, the
 * corresponding pointer is NULL.	If some subpixels of the pixel are
 * non-black, the pixel points to an 8x8 array of subpixels.
 */

/* Initialize high-resolution contour image */
static void init_hires_contourimg(
	int		xsize,
	int		ysize)
{
	miColorDepth	**hires_pixels, **pixel_ptr;
	int		i;

	mi_debug("allocate high-resolution contourimage");

	/* Sparse color hires image (8*8 miColorDepths) */
	hires_image  = (miColorDepth ***)
			mi_mem_allocate(ysize * sizeof(miColorDepth **));
	hires_pixels = (miColorDepth **)
			mi_mem_allocate(xsize * ysize * sizeof(miColorDepth*));
	for (i=0, pixel_ptr=hires_pixels; i < ysize; i++, pixel_ptr += xsize)
		hires_image[i] = pixel_ptr;

	pixel_pages  = mi_mem_blkcreate(sizeof(miPixel));
	/* free'd in fini_hires_contourimg() */
	mi_debug("high-resolution contourimage allocated");
}


/*
 * Rendering is finished, deallocate sparse high-resolution contour image
 */

static void fini_hires_contourimg(void)
{
	mi_debug("deallocating high-resolution contourimage");
	mi_mem_release(hires_image[0]);	/* hires_pixels */
	mi_mem_release(hires_image);
	mi_mem_blkdelete(pixel_pages);
	pixel_pages = NULL;
}


/*
 * Put subpixel mask, that is, write pixel to high-resolution image
 */

static void set_subpixels(
	unsigned char	*mask,		/* mask has 8 elements, each 8 bits */
	int		i,		/* y */
	int		j,		/* x */
	miColor		*color,
	double		depth)
{
	miColorDepth	*pixel, *subpixel;
	int		k, l, idx = 0;

	/*
	 * If pixel is unwritten, allocate memory for its 8x8 subpixels
	 * and set these to black and infinite distant (depth 0).
	 */
	if (hires_image[i][j] == NULL)
		hires_image[i][j] = (miColorDepth *)
					mi_mem_blkallocate(pixel_pages);

	/* Blend with existing color */
	pixel = hires_image[i][j];
	for (k=0, idx=0; k < SUBYRES; k++)
		for (l=0; l < SUBYRES; l++, idx++) {
			if (!(mask[k] & (0x80 >> l)))
				continue;
			subpixel = &(pixel[idx]);
			if (subpixel->depth == 0.0) {
				/* New color (no previous color) */
				subpixel->color = *color;
				subpixel->depth = depth;

			} else if (max_composite) {
				/* Max compositing */
				subpixel->color.r = mi_MAX(subpixel->color.r,
								     color->r);
				subpixel->color.g = mi_MAX(subpixel->color.g,
								     color->g);
				subpixel->color.b = mi_MAX(subpixel->color.b,
								     color->b);
				subpixel->color.a = mi_MAX(subpixel->color.a,
								     color->a);
			} else {
				/* Alpha compositing */
				if (depth > subpixel->depth) {
					/* New color is in front of old color */
					float a = color->a;
					subpixel->color.r = color->r +
						(1-a) * subpixel->color.r;
					subpixel->color.g = color->g +
						(1-a) * subpixel->color.g;
					subpixel->color.b = color->b +
						(1-a) * subpixel->color.b;
					subpixel->color.a = color->a +
						(1-a) * subpixel->color.a;
					subpixel->depth = depth;
				} else {
					/* New color is behind old color */
					float a = subpixel->color.a;
					subpixel->color.r = subpixel->color.r +
						(1-a) * color->r;
					subpixel->color.g = subpixel->color.g +
						(1-a) * color->g;
					subpixel->color.b = subpixel->color.b +
						(1-a) * color->b;
					subpixel->color.a = subpixel->color.a +
						(1-a) * color->a;
				}
			}
	}
}


static void set_subpixel2(
	int		i,		/* y */
	int		j,		/* x */
	int		k,		/* subpixel y */
	int		l,		/* subpixel x */
	miColor		*color)
{
	miColorDepth	*pixel, *subpixel;

	pixel = region_image2[i][j];
	if (pixel == NULL) {
		/* Allocate a pixel and set all subpixels to black and inf
		   distance */
		pixel = (miColorDepth *)mi_mem_blkallocate(region_pixel_pages);
		region_image2[i][j] = pixel;
		++npixels2;
	}

	/* Set this subpixel */
	subpixel = &pixel[k*SUBXRES+l];
	subpixel->color.r = color->r;
	subpixel->color.g = color->g;
	subpixel->color.b = color->b;
	subpixel->color.a = color->a;
	subpixel->depth   = 0.0;
}


static void set_subpixel3(		/* region_image, */
	int		i,		/* y */
	int		j,		/* x */
	int		k,		/* subpixel y */
	int		l,		/* subpixel x */
	miColor		*color)
{
	miColorDepth	*pixel, *subpixel;

	pixel = region_image3[i][j];
	if (pixel == NULL) {
		/* Allocate a pixel and set all subpixels to black and inf.
		   distance */
		pixel = (miColorDepth *)mi_mem_blkallocate(region_pixel_pages);
		region_image3[i][j] = pixel;
		++npixels;
	}

	/* Set this subpixel */
	subpixel = &pixel[k*SUBXRES+l];
	subpixel->color.r = color->r;
	subpixel->color.g = color->g;
	subpixel->color.b = color->b;
	subpixel->color.a = color->a;
	subpixel->depth   = 0.0;
}


/*
 * Are the five nearest pixels all black (NULL)?
 */

static void test_nearby_pixels(
	miBoolean	*nearby_nonblack_pixel,
	miColorDepth	**pixels,
	int		n)	/* # of elements in 'pixels' (array of ptrs) */
{
	int		i;

	miASSERT(n >= 2);
	nearby_nonblack_pixel[0] = (miBoolean)
			(pixels[0] || pixels[1] || pixels[2]);
	nearby_nonblack_pixel[1] = (miBoolean)
			(pixels[0] || pixels[1] || pixels[2] || pixels[3]);
	for (i=2; i < n-2; i++)
		nearby_nonblack_pixel[i] = (miBoolean)
			(pixels[i-2] || pixels[i-1] || pixels[i] ||
			 pixels[i+1] || pixels[i+2]);

	nearby_nonblack_pixel[n-2] = (miBoolean)
		(pixels[n-4] || pixels[n-3] || pixels[n-2] || pixels[n-1]);
	nearby_nonblack_pixel[n-1] = (miBoolean)
		(pixels[n-3] || pixels[n-2] || pixels[n-1]);
}



/*
 * Filtering:
 * Filters (using lanczos_2 filter to avoid aliasing) and downsamples
 * the high-resolution contour image and writes the resulting image.
 * This procedure is on the master only.
 */


#define FILTER_R	16	/* filter radius: 16 subpixels = 2 pixels */
#define FILTER_SUPPORT	32	/* Filter support: $2r+1$ */
#define OLAP		2	/* image filtering task overlap: 2 pixels */

static double filter_table[FILTER_SUPPORT];

/* The constant ${1 \over \pi} {2 \over \pi}$ */
#define C_1_PI_2_PI	0.202642367284675542888

/*
 * The two-lobed Lanczos-windowed sinc function.
 * This filter function gives the best compromise between aliasing,
 * sharpness, and minimal ringing (Graphics Gems, page 158).
 * It is significantly superior to box filtering for lines thinner than
 * one pixel.
 */

static double lanczos_2(double arg)
{
	double x = fabs(arg), inv_x;

	miASSERT(x <= 2.5 && x != 0.0);
	if (x < 2.0) {
		inv_x = 1.0 / x;
		return(C_1_PI_2_PI * sin(M_PI*x)   * inv_x
				   * sin(M_PI/2*x) * inv_x);
	} else
		return(0.0);
}


/*
 * Filter the high-resolution contours in hires_image to form a contour image.
 * Then alpha-composite "over" the regular image.
 *
 * The filtering and compositing is done in regions of size REGIONSIZEX *
 * REGIONSIZEY pixels. The outermost 2 pixels are not filtered correctly, so
 * only the inner pixels are assigned to the output image. The tasks overlap
 * by 4 pixels. Filtering the whole image at once is simpler, but requires a
 * *lot* of memory even though only the areas covered by contours are filtered.
 * The two procedures lanczos2_filter_y() and lanczos2_filter_x() take up to
 * 9% of total runtime.
 */

static void filter_contour_image(
	miImg_image	*image)
{
	int		size = image_xsize * image_ysize;
	int		done = 0;
	int		xmin, ymin, xmax, ymax;

	mi_progress("filtering high-resolution contour image");

	tabulate_filter();
	for (ymin=-OLAP; ymin < image_ysize-OLAP; ymin+=REGIONSIZEY-2*OLAP)
		for (xmin=-OLAP; xmin < image_xsize-OLAP;
						  xmin+=REGIONSIZEX-2*OLAP) {
 			xmax = mi_MIN(xmin + REGIONSIZEX, image_xsize+OLAP);
			ymax = mi_MIN(ymin + REGIONSIZEY, image_ysize+OLAP);
			mi_debug("filtering contours in region [%i:%i] x "
					"[%i:%i]", xmin, xmax, ymin, ymax);
			init_region_images();
			/* hires_image -> region_image2 */
			lanczos2_filter_y(xmin, ymin, xmax, ymax);
			/* region_image2 -> region_image3 */
			lanczos2_filter_x(xmin, ymin, xmax, ymax);
			/* region_image3 -> output image */
			box_filter(image, xmin+OLAP, ymin+OLAP,
					  xmax-OLAP, ymax-OLAP);
			fini_region_images();
			done += (xmax - xmin - 2*OLAP)*(ymax - ymin - 2*OLAP);
			mi_progress("%5.1f%%", 100.0 * done / size);
			if(mi_par_aborted()) {
				mi_progress("contour filtering aborted");
				return;
			}
		}
	mi_progress("filtering finished");
}


/*
 * Precomputations for faster convolution: fill filter_table
 */

static void tabulate_filter(void)
{
	double		sum = 0, inv_norm;
	int		i;

	for (i=0; i < FILTER_SUPPORT; i++) {
		filter_table[i] = lanczos_2(2.0/FILTER_R * (i-FILTER_R + 0.5));
		sum += filter_table[i];
	}
	inv_norm = 1.0 / sum;
	for (i = 0; i < FILTER_SUPPORT; i++)
		filter_table[i] *= inv_norm;
}


static void init_region_images(void)
{
	miColorDepth	**hires_pixels, **pixel_ptr;
	int		i;

	/* Init region_image2[][] --- could move to init_hires_contourimg() */
	region_image2 = (miColorDepth ***)
			mi_mem_allocate(REGIONSIZEY * sizeof(miColorDepth **));
	hires_pixels = (miColorDepth **)
			mi_mem_allocate(REGIONSIZEX *
					REGIONSIZEY * sizeof(miColorDepth *));
	for (i=0, pixel_ptr=hires_pixels; i < REGIONSIZEY;
						i++, pixel_ptr += REGIONSIZEX)
		region_image2[i] = pixel_ptr;

	/* Init region_image3[][] --- could move to init_hires_contourimg() */
	region_image3 = (miColorDepth ***)
			mi_mem_allocate(REGIONSIZEY * sizeof(miColorDepth **));
	hires_pixels = (miColorDepth **)
			mi_mem_allocate(REGIONSIZEX *
					REGIONSIZEY * sizeof(miColorDepth *));
	for (i=0, pixel_ptr=hires_pixels; i < REGIONSIZEY;
						i++, pixel_ptr += REGIONSIZEX)
		region_image3[i] = pixel_ptr;

	region_pixel_pages = mi_mem_blkcreate(sizeof(miPixel));
}


static void fini_region_images(void)
{
	mi_mem_release(region_image2[0]);
	mi_mem_release(region_image2);
	mi_mem_release(region_image3[0]);
	mi_mem_release(region_image3);
	mi_mem_blkdelete(region_pixel_pages);
	region_pixel_pages = NULL;
}


static void lanczos2_filter_y(
	int		xmin,
	int		ymin,
	int		xmax,
	int		ymax)
{
	/* 'register' since these are used in the inner convolution loop: */
	register miColorDepth **pixels, **subpixels, *pixel, *subpixel;
	register miColor contourcolor, *color;
	register double	weight;
	register int	ii, i_min, i_max;
	/* Not used in the inner loop: */
	miColor		contourcolor2;
	int		i, j, i_pixel, j_pixel, j_subpixel, offset, i_subpixel;
	int		xsize = xmax - xmin;	/* size of image region */
	int		ysize = ymax - ymin;
	int		hires_xsize = SUBXRES * xsize;	/* size of highres
							   image region */
	int		hires_ysize = SUBYRES * ysize;
	miBoolean	*nearby_nonblack_pixel;

	mi_debug("filtering in y direction using Lanczos_2 filter");

	/* Local arrays --- released at the end of this procedure */
	pixels = (miColorDepth **)
			mi_mem_allocate(hires_ysize * sizeof(miColorDepth *));
	nearby_nonblack_pixel = (miBoolean *)
			mi_mem_allocate(hires_ysize * sizeof(miBoolean));
	subpixels = (miColorDepth **)
			mi_mem_allocate(hires_ysize * sizeof(miColorDepth *));

	for (j=0; j < hires_xsize; j++) {
		j_pixel = j/SUBXRES;
		j_subpixel = j%SUBXRES;

		/*
		 * Compute array of pointers to pixels. This is faster and
		 * simpler to look-up in than the original array
		 */
		if (j % SUBXRES == 0) {
			for (i=0; i < ysize; i++)
				pixels[i] = 0		 <= ymin+i	 &&
					    ymin+i	 <  image_ysize  &&
					    0		 <= j_pixel+xmin &&
					    j_pixel+xmin <  image_xsize ?
					hires_image[ymin+i][j_pixel+xmin] : 0;
			test_nearby_pixels(nearby_nonblack_pixel,
								pixels, ysize);
		}

		/*
		 * Compute array of pointers to subpixels. The subpixel is
		 * NULL if the subpixel color in the hires image is black
		 */
		for (i=ii=0; i < ysize; i++, ii+=8) {
			pixel = pixels[i];
			if (pixel) {
				for (i_subpixel=0; i_subpixel < SUBYRES;
								i_subpixel++) {
					subpixel = &pixel[(i_subpixel<<3) +
								j_subpixel];
					color = &subpixel->color;
					subpixels[ii+i_subpixel] =
						color->r > miEPS ||
						color->g > miEPS ||
						color->b > miEPS ||
						color->a > miEPS ?
								subpixel : 0;
				}
			}
		}

		/* Convolution */
		for (i=0; i < hires_ysize; i++) {
			i_pixel = i/SUBYRES;
			offset  = -i+FILTER_R-1;

			if (nearby_nonblack_pixel[i_pixel]) {

				/* Convolute hires_image with filter in i
				   direction */
				i_min = i - FILTER_R + 1;
				if (i_min < 0)
					i_min = 0;
				i_max = i + FILTER_R;
				if (i_max > hires_ysize - 1)
					i_max = hires_ysize - 1;

				contourcolor.r = contourcolor.g =
				contourcolor.b = contourcolor.a = 0.0;

				pixel = pixels[i_min>>3];
				ii = i_min;
				while (ii <= i_max) {
					if (!(ii & 0x7))
						pixel = pixels[ii>>3];
					if (pixel) {
						subpixel = subpixels[ii];
						if (subpixel) {
							color  =
							     &subpixel->color;
							weight =
							     filter_table[ii +
							     	       offset];
							contourcolor.r +=
							     weight * color->r;
							contourcolor.g +=
							     weight * color->g;
							contourcolor.b +=
							     weight * color->b;
							contourcolor.a +=
							     weight * color->a;
						}
						ii++;
					} else
						/* skip ahead to next pixel (8
						   subpixels or less) */
						ii = (ii | 0x7) + 1;
				}

				/* Write result in region_image2[][] */
				if (contourcolor.r > miEPS ||
				    contourcolor.g > miEPS ||
				    contourcolor.b > miEPS ||
				    contourcolor.a > miEPS) {
					/*
					 * This assignment is necessary since
					 * we want contourcolor to be register,
					 * and then we can't pass a ptr to it
					 */
					contourcolor2 = contourcolor;
					set_subpixel2(i_pixel, j_pixel, i%8,
						   j_subpixel, &contourcolor2);
				}
			} else
				/* skip ahead to next pixel (8 subpixels
				   or less) */
				i = (i | 0x7) + 1;
		}
	}

	/* Release local arrays */
	mi_mem_release(pixels);
	mi_mem_release(nearby_nonblack_pixel);
	mi_mem_release(subpixels);
	mi_debug("npixels2 = %i", npixels2);
}


static void lanczos2_filter_x(
	int		xmin,
	int		ymin,
	int		xmax,
	int		ymax)
{
	register miColorDepth	*pixel, *subpixel;
	register miColorDepth	**pixels, **subpixels;
	register miColor	contourcolor, *color;
	miColor		contourcolor2;
	register	double weight;
	register int	i,j, jj, j_min, j_max, i_pixel, j_pixel, idx_i, offset;
	register int	j_subpixel;
	int		xsize = xmax - xmin;	/* size of image region */
	int		ysize = ymax - ymin;
	int		hires_xsize = SUBXRES * xsize;	/* size of highres
							   image region */
	int		hires_ysize = SUBYRES * ysize;
	miBoolean	*nearby_nonblack_pixel;

	mi_debug("filtering in x direction using Lanczos_2 filter");

	/* Local arrays --- released at the end of this procedure */
	pixels = (miColorDepth **)
			mi_mem_allocate(xsize * sizeof(miColorDepth *));
	nearby_nonblack_pixel = (miBoolean *)
			mi_mem_allocate(xsize * sizeof(miBoolean));
	subpixels = (miColorDepth **)
			mi_mem_allocate(hires_xsize * sizeof(miColorDepth *));

	for (i=0; i < hires_ysize; i++) {
		i_pixel = i/SUBYRES;
		idx_i = (i % SUBYRES) * SUBXRES;

		/*
		 * Compute array of pointers to pixels. This is faster
		 * and simpler to look-up in than the original array
		 */
		if (i % SUBXRES == 0) {
			for (j=0; j < xsize; j++)
				pixels[j] = region_image2[i_pixel][j];
			test_nearby_pixels(nearby_nonblack_pixel,pixels,xsize);
		}

		/*
		 * Compute array of Booleans, saying whether subpixels
		 * are black
		 */
		for (j=jj=0; j < xsize; j++, jj += 8) {
			pixel = pixels[j];
			if (pixel) {
				for (j_subpixel=0; j_subpixel < SUBXRES;
								j_subpixel++) {
					subpixel = &pixel[idx_i + j_subpixel];
					color = &(subpixel->color);
					subpixels[jj+j_subpixel] =
						color->r > miEPS ||
						color->g > miEPS ||
				 		color->b > miEPS ||
						color->a > miEPS ? subpixel :0;
				}
			}
		}

		/* Convolution */
		for (j=0; j < hires_xsize; j++) {
			j_pixel = j/SUBXRES;
			offset  = -j+FILTER_R-1;

			if (nearby_nonblack_pixel[j_pixel]) {
				/* Convolute hires_image2 with filter in j
				   direction (x dir) */
				j_min = j - FILTER_R + 1;
				if (j_min < 0)
					j_min = 0;
				j_max = j + FILTER_R;
				if (j_max > hires_xsize - 1)
					j_max = hires_xsize - 1;

				contourcolor.r = contourcolor.g =
				contourcolor.b = contourcolor.a = 0.0;

				pixel = pixels[j_min>>3];
				jj = j_min;
				while (jj <= j_max) {
					if (!(jj & 0x7))
						pixel = pixels[jj>>3];
					if (pixel) {
						subpixel = subpixels[jj];
						if (subpixel) {
							color=&subpixel->color;
							weight = filter_table[
								    jj+offset];
							contourcolor.r +=
							     weight * color->r;
							contourcolor.g +=
							     weight * color->g;
							contourcolor.b +=
							     weight * color->b;
							contourcolor.a +=
							     weight * color->a;
						}
						jj++;
					} else {
						/* skip ahead to next pixel
						   (8 subpixels or less) */
						jj = (jj | 0x7) + 1;
					}
				}

				/* Write result in region_image3[][] */
				if (contourcolor.r > miEPS ||
				    contourcolor.g > miEPS ||
				    contourcolor.b > miEPS ||
				    contourcolor.a > miEPS) {
					/*
					 * This assignment is necessary since
					 * we want contourcolor to be register,
					 * and then we cant pass a ptr to it
					 */
					contourcolor2 = contourcolor;
					set_subpixel3(i_pixel, j_pixel, i%8,
							j%8, &contourcolor2);
				}
			} else
				/* skip ahead to next pixel (8 subpixels or
				   less) */
				j = (j | 0x7) + 1;
		}
	}

	/* Release local arrays */
	mi_mem_release(pixels);
	mi_mem_release(nearby_nonblack_pixel);
	mi_mem_release(subpixels);
	mi_debug("npixels = %i", npixels);
}


/*
 * Compute average of 8x8 subpixels and composite over image
 */

static void box_filter(
	miImg_image	*image,
	int		xmin,
	int		ymin,
	int		xmax,
	int		ymax)
{
	miColorDepth	*pixel;
	miColor		contourcolor;
	miColor		imagecolor, *subpixel_color;
	double		alpha;
	int		xsize = xmax - xmin;
	int		ysize = ymax - ymin;
	int		i, j, k, l, idx;

	mi_debug("box filter");
	for (i=0; i < ysize; i++) {
		for (j=0; j < xsize; j++) {
			pixel = region_image3[i+OLAP][j+OLAP];
			if (pixel == NULL)
				continue;

			/* Compute average over SUBXRES*SUBYRES (8x8) subpix */
			contourcolor.r = contourcolor.g =
			contourcolor.b = contourcolor.a = 0.0;
			idx = 0;
			for (k=0; k < SUBXRES; k++)
				for (l = 0; l < SUBYRES; l++, idx++) {
					subpixel_color = &pixel[idx].color;
					contourcolor.r += subpixel_color->r;
					contourcolor.g += subpixel_color->g;
					contourcolor.b += subpixel_color->b;
					contourcolor.a += subpixel_color->a;
				}
			contourcolor.r *= 1.0 / (SUBXRES*SUBYRES);
			contourcolor.g *= 1.0 / (SUBXRES*SUBYRES);
			contourcolor.b *= 1.0 / (SUBXRES*SUBYRES);
			contourcolor.a *= 1.0 / (SUBXRES*SUBYRES);

			/*
			 * Composite contour color with image color: contour
			 * "over" image. Alpha has to be blended just like
			 * rgb, otherwise alpha is too low along the edges of
			 * contours when the contours are composited over an
			 * image with alpha 1.
			 */
			if (contourcolor.a > miEPS) {
				mi_img_get_color(image, &imagecolor,
							xmin+j, ymin+i);
				alpha = contourcolor.a;
				contourcolor.r = contourcolor.r +
						(1-alpha) * imagecolor.r;
				contourcolor.g = contourcolor.g +
						(1-alpha) * imagecolor.g;
				contourcolor.b = contourcolor.b +
						(1-alpha) * imagecolor.b;
				contourcolor.a = contourcolor.a +
						(1-alpha) * imagecolor.a;

				mi_img_put_color(image, &contourcolor,
							xmin+j, ymin+i);
			}
		}
	}
}
