/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	18.1.96 
 * Module:	shaders/contour
 * Purpose:	standard contour shaders: output
 *
 * Exports: 
 *	contour_ps
 *	contour_mi
 *
 * History: 
 *	14.9.01: reindented in manual style for publication
 *
 * Description:
 * Get the contour line segments and write them into a PostScript file.
 * Like all output shaders, this computation takes place on the master host
 * only. Color PostScript is not supported, so all contours are black.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <assert.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <shader.h>
#include <geoshader.h>
#include <mi_contourshade.h>

#define miPATHSIZE	1024		/* length of file paths */
#define miEPS		0.0001

#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif


typedef struct {			/* polygon vertex */
	double x, y;			/* subpixel display coordinate */
} miPolygonVertex;

static double	paper_transform_b = 0.0; 
static double	paper_transform_d = 1.0; 
static char	*file_name;
static FILE	*ps_file, *mi_file;
static double	ps_scale;
static int	image_xsize, image_ysize; /* image size in pixels */


/* 
 * Procedure prototypes 
 */

static miBoolean init_ps_file(char *file_name, char *prog, char *input,
                              int frame, int paper_size, double paper_scale,
                              miBoolean label, miBoolean landscape);
static void draw_line_ps(miContour_endpoint *p1, miContour_endpoint *p2);
static void draw_polygon_ps(miPolygonVertex polygon[], int numVertex);
static void fini_ps_file(void);
static void init_mi_file(char *file_name);
static void fini_mi_file(void);


/*
 * Write PostScript contour file:
 * Output shader for computing PostScript contour file.	
 * This shader (like all other output shaders) is on the master only. 
 */

int contour_ps_version(void) {return(4);}

miBoolean contour_ps(
	miColor			*result,	/* unused */
	miState			*state,
	Postscript_Parameters	*paras) 
{
	miContour_endpoint	p1;
	miContour_endpoint	p2;
	miVector		stroke_dir = paras->ink_stroke_dir;
	miVector		contour_dir;
	float			d, f;
	miTag			tag;
	static const miScalar	cost_max = 20000.0;
	miScalar		cost = 0.0;
	miScalar		dcost = 0.0;
	miBoolean		aborted = miFALSE;
	int			line_count = 0;

	mi_info("computing contours");
	mi_debug("paras->paper_size = %s", &paras->paper_size);

	image_xsize = state->camera->x_resolution;
	image_ysize = state->camera->y_resolution;

	paper_transform_b = paras->paper_transform_b;
	paper_transform_d = paras->paper_transform_d;

	/* Use reasonable values if the user forgot to give parameters */
	if ((paper_transform_b == 0.0 && paper_transform_d == 0.0) ||
				fabs(paras->paper_transform_b) > 1e6 || 
				fabs(paras->paper_transform_d) > 1e6) { 
		paper_transform_b = 0.0;
		paper_transform_d = 1.0;
	}
	if (paras->paper_scale == 0.0 || fabs(paras->paper_scale) > 1e6)
		paras->paper_scale = 1.0;

	/* Get filename */
	if (paras->file_name) {		/* file name was a parameter */
		tag = *mi_eval_tag(&paras->file_name);
		file_name = mi_db_access(tag);
	} 
        if(!paras->file_name || !file_name || !file_name[0]) {
		if(!paras->file_name) 
			mi_warning("contour_ps: no file name specified.");
		else
			mi_warning("contour_ps: empty file name specified.");
		return(miFALSE);
	}
	miASSERT(file_name);
	miASSERT(file_name[0]);

	/* Write the prolog of the PostScript file */
	if(init_ps_file(file_name, "mental ray", "Contour image", 
			 state->camera->frame, paras->paper_size,
			 paras->paper_scale, paras->title, paras->landscape)
           == miFALSE)
                return miFALSE;


	/* Write the contour line segments as PostScript */
	if (mi_vector_norm(&stroke_dir) < miEPS) {	/* regular stroke */
		while (!aborted && mi_get_contour_line(&p1, &p2)) {
			dcost = p1.width + p2.width;
			dcost *= dcost;
			cost += dcost > 0.01 ? dcost : 0.01;
			++line_count;
			draw_line_ps(&p1, &p2);
			if( cost > cost_max) {
				mi_progress("processed %d contour lines", 
								line_count);
				aborted = mi_par_aborted();
				cost = 0.0;
			}
		}

	} else {					/* ink pen stroke */
		/* Normalize stroke direction (ignore z component) */
		stroke_dir.z = 0.0;
		mi_vector_normalize(&stroke_dir);

		while (!aborted && mi_get_contour_line(&p1, &p2)) {

			/* Compute direction of contour seg in image plane */
			contour_dir.x = p1.point.x - p2.point.x;
			contour_dir.y = p1.point.y - p2.point.y;
			contour_dir.z = 0.0;
			mi_vector_normalize(&contour_dir);

			/* The contour gets wider the more the contour
			   direction is parallel to the stroke direction */
			d = fabs(mi_vector_dot(&stroke_dir, &contour_dir));
			f = paras->ink_min_frac + (1-paras->ink_min_frac) * d;
			p1.width *= f;
			p2.width *= f;

			dcost = p1.width + p2.width;
			dcost *= dcost;
			cost += dcost > 0.01 ? dcost : 0.01;
			++line_count;
			draw_line_ps(&p1, &p2);
			if( cost > cost_max) {
				mi_progress("processed %d contour lines", 
								line_count);
				aborted = mi_par_aborted();
				cost = 0.0;
			}
		}
	}

	/* Write end of PostScript file */
	fini_ps_file();

	mi_db_unpin(tag);
	mi_info("contours computed");
	return(miTRUE);
}



/*
 * Initialize the contour computations on master:
 * If the output is a PostScript file, the file header is written.
 * This runs once on the master.
 */

/* Font size of the label, in points (1/72 inch). */
#define FONT_SIZE	10
#define PAPER_NSIZES	13


/*
 * Headers and footers for output files:
 */

static miBoolean init_ps_file(
	char		*file_name, 
	char		*prog, 
	char		*input,
	int		frame,
	int		paper_size,
	double		paper_scale,
	miBoolean	title,
	miBoolean	landscape)
{
	double		x_ps_scale, y_ps_scale;
	double		x_margin, y_margin;
	double		x_inches, y_inches;
	time_t		cal_t = time((time_t *)NULL);

	static char *paper_size_name[PAPER_NSIZES] = {
		"letter", "executive", "legal", "a3", "a4", "a5", "a6",
		"b4", "b5", "b6", "11x17"
	};
	static double x_inches_table[PAPER_NSIZES] = {
		8.5,	7.25,	8.5,	11.69,	8.27,	5.83,	4.13,
		9.84,	6.93,	4.92,	11.0
	};
	static double y_inches_table[PAPER_NSIZES] = {
		11.0,	10.5,	14.0,	16.54,	11.69,	8.27,	5.83,	
		13.90,	 9.84,	 6.93,	17.0
	};

	if (paper_size < 0 || PAPER_NSIZES < paper_size) {
		paper_size = 4;
		mi_error("illegal paper size, using %s",
						paper_size_name[paper_size]);
	}

	mi_info("using %s paper format", paper_size_name[paper_size]);
	mi_info("paper scale %f", paper_scale);

	if (landscape) {			/* swap x and y size */
		x_inches = y_inches_table[paper_size];
		y_inches = x_inches_table[paper_size];
	} else {				/* x is x and y is y */
		x_inches = x_inches_table[paper_size];
		y_inches = y_inches_table[paper_size];
	}

	x_ps_scale = x_inches * 72.0 / image_xsize;
	y_ps_scale = y_inches * 72.0 / image_ysize;
	ps_scale   = (x_ps_scale < y_ps_scale ? x_ps_scale : y_ps_scale)
							* 0.90 * paper_scale;
	mi_debug("ps_scale = %f", ps_scale);
	x_margin = (x_inches*72.0 - ps_scale*image_xsize) / 2;
	y_margin = (y_inches*72.0 - ps_scale*image_ysize) / 2;

	ps_file = fopen(file_name, "w");
	if (!ps_file) {
		mi_error("failed to open ps file \"%s\" for writing",
								file_name);
                return miFALSE;
        }
	mi_progress("writing PostScript file \"%s\"", file_name);

	/* Write PostScript header */
	(void)fprintf(ps_file, "%%!PS-Adobe-1.0\n");
	(void)fprintf(ps_file, "%%%%Title: %s, frame %04d\n", input, frame);
	(void)fprintf(ps_file, "%%%%Creator: %s\n", prog);
	(void)fprintf(ps_file, "%%%%CreationDate: %s", ctime(&cal_t));
	(void)fprintf(ps_file, "%%%%Pages: 1\n");
	if (landscape) {
		(void)fprintf(ps_file,"%%%%BoundingBox: %.2f %.2f %.2f %.2f\n",
			y_margin, x_margin,
			y_margin + ps_scale*image_ysize,
			x_margin + ps_scale*image_xsize); 
	} else {
		(void)fprintf(ps_file,"%%%%BoundingBox: %.2f %.2f %.2f %.2f\n",
			x_margin, y_margin, 
			x_margin + ps_scale*image_xsize, 
			y_margin + ps_scale*image_ysize);
	}
	(void)fprintf(ps_file, "%%%%EndComments\n");
	(void)fprintf(ps_file, "%%%%EndProlog\n");
	(void)fprintf(ps_file, "%%%%Page: 1 1\n\n");

	if (title) {
		/* Label the picture */
		(void)fprintf(ps_file, "/Times-Roman findfont %d scalefont "
					"setfont\n", FONT_SIZE);
		if (landscape)
			(void)fprintf(ps_file, "%.2f %.2f moveto\n", 
				y_margin, ps_scale * image_xsize + x_margin+5);
		else
			(void)fprintf(ps_file, "%.2f %.2f moveto\n", 
				x_margin, ps_scale * image_ysize + y_margin+5);
		(void)fprintf(ps_file, "(%s	%04d) show\n\n",
				file_name, frame);
	}

	/* Switch to landscape */
	if (landscape) {
		(void)fprintf(ps_file, "%.2f %.2f translate\n",
					y_inches*72.0 - y_margin, x_margin);
		(void)fprintf(ps_file, "90 rotate\n\n");
	} else
		(void)fprintf(ps_file, "%.2f %.2f translate\n",
					x_margin, y_margin);
	
	if (title) {
		/* Draw frame (bounding box) around image */
		(void)fprintf(ps_file, "0.2 setlinewidth\n");
		(void)fprintf(ps_file, "newpath\n");
		(void)fprintf(ps_file, "	0.00 0.00 moveto\n");
		(void)fprintf(ps_file, "	%.2f 0.00 lineto\n", 
				ps_scale * image_xsize);
		(void)fprintf(ps_file, "	%.2f %.2f lineto\n",
				ps_scale * (image_xsize +
						paper_transform_b*image_ysize),
				ps_scale * paper_transform_d * image_ysize); 
		(void)fprintf(ps_file, "	%.2f %.2f lineto\n", 
				ps_scale * paper_transform_b * image_ysize,
				ps_scale * paper_transform_d * image_ysize);
		(void)fprintf(ps_file, "closepath stroke\n\n");
	}

	/* Set the line width */
	(void)fprintf(ps_file, "%.2f setlinewidth\n", 0.05);
        return miTRUE;
}


static void fini_ps_file(void) {
	(void)fprintf(ps_file,"showpage\n");
	(void)fprintf(ps_file, "%%%%Trailer\n");
	(void)fclose(ps_file);
}



/*
 * Write polygons approximating contours as PostScript
 * 
 * Line drawing routines.
 */

#define NUM_VERTEX	10	/* decagon */
#define SQRT1_2		0.7071067811865475

static void draw_line_ps(
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
	t_x = ls_x - fs_x;
	t_y = ls_y - fs_y;
	miASSERT(t_x*t_x + t_y*t_y > miEPS);
	r_norm = 1.0 / sqrt(t_x*t_x + t_y*t_y);
	t_x *= r_norm;
	t_y *= r_norm;

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
	polygon[4].y = fs_y + r1 *   t_x ;
	
	/* vertex 5; angle = Pi / 2 */
	polygon[5].x = ls_x + r2 * (-t_y);
	polygon[5].y = ls_y + r2 *   t_x ;
	
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
	
	draw_polygon_ps(polygon, NUM_VERTEX);
}


static void draw_polygon_ps(
	miPolygonVertex	polygon[],	/* clockwise clipped vertex list */
	int		numVertex)	/* number of vertices in polygon */
{
	double		x, y;
	int		i;

	/* Convert to PostScript coordinates (point units) */
	for (i=0; i < numVertex; i++) {
		x = polygon[i].x;
		y = polygon[i].y; 
		polygon[i].x = ps_scale * (x + paper_transform_b * y);
		polygon[i].y = ps_scale * paper_transform_d * y; 
	}

	/* Start a new path */
	(void)fprintf(ps_file, "newpath\n");

	/* Move to first vertex */
	(void)fprintf(ps_file, "	%.2f %.2f moveto\n",
						polygon[0].x, polygon[0].y);
	/* Line to all the other vertices */
	for (i = 1; i <numVertex; i++) {
		(void)fprintf(ps_file, "	%.2f %.2f lineto\n",
						polygon[i].x, polygon[i].y);
	}
	/* Close path and fill */
	(void)fprintf(ps_file, "closepath fill\n");
}



/*
 * Write contours in mi file:
 * Output shader for computing mi contour file.	
 * This shader (like all other output shaders) is on the master only. 
 * Output contours as lines in a mi file.
 */

int contour_mi_version(void) {return(1);}

miBoolean contour_mi(
	miColor		*result,	/* unused */
	miState		*state)		/* unused */
	/* no parameters */
{
	miContour_endpoint p1;
	miContour_endpoint p2;
	static const miScalar cost_max = 20000.0;
	miScalar cost = 0.0;
	miScalar dcost = 0.0;
	miBoolean aborted = miFALSE;
	int line_count = 0;

	mi_info("computing contours");

	/* Write the prolog of the mi file */
	init_mi_file(file_name);

	/* Write the contours in mi line format */
	while (!aborted && mi_get_contour_line(&p1, &p2)) {
		fprintf(mi_file, "	%f %f	%f %f\n", 
			p1.point.x, p1.point.y, p2.point.x, p2.point.y);
		dcost = p1.width + p2.width;
		dcost *= dcost;
		cost += dcost > 0.01 ? dcost : 0.01;
		++line_count;
		draw_line_ps(&p1, &p2);
		if( cost > cost_max) {
			mi_progress("processed %d contour lines", line_count);
			aborted = mi_par_aborted();
			cost = 0.0;
		}
	}

	/* Write end of mi file */
	fini_mi_file();
	mi_info("contours computed");
	return(miTRUE);
}


static void init_mi_file(char *file_name)
{
	mi_file = fopen(file_name, "w");
	if (!mi_file)
		mi_error("failed to open mi file \"%s\" for writing",
								file_name);
	mi_progress("writing mi file \"%s\"", file_name);

	/* Write mi header */
	(void)fprintf(mi_file, "# contour lines");
	(void)fprintf(mi_file, " (special curves for optimal triangulation)"
								"\n\n");
}


static void fini_mi_file(void)
{
	(void)fprintf(mi_file, "# end of contour lines\n\n");
}
