/*
======================================================================
geometry.h

Ernie Wright  18 Jan 00

Type definitions and function prototypes for the DNA model plug-in.
====================================================================== */

#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {
   char * name;         // surface name
   float  colr[ 3 ];    // base color
   float  diff;         // diffuse
   float  spec;         // specular
   float  glos;         // glossiness
   float  sman;         // max smoothing angle
   float  tran;         // transparency
   float  rind;         // refraction index
   int    side;         // sidedness
} DNA_SURFACE;


/* item count */
int atom_count( int ibase );
int bond_count( int ibase );
int point_count( int ibase );

/* item info */
void atom_info( int ibase, int iatom, int *vi, int *snum );
void bond_info( int ibase, int ibond, int *vi0, int *vi1, int *snum );
void plate_info( int ibase, int iplate, int *nv, int *vi, int *snum );

/* coordinates */
void vert_coords( int ibase, int ivert, double pt[ 3 ] );
void bond_coords( int ibase, int ibond, double pt[ 3 ], double *h,
        double *xrot, double *yrot, int *snum );

/* surface data */
DNA_SURFACE *surface_data( int snum );
char *surface_name( int snum );

/* degrees and radians */
#define degrad(x) ((x) * 1.7453292519943295769236907685e-2)
#define raddeg(x) ((x) * 5.7295779513082320876798154814e1)

#ifndef PI
#define PI 3.14159265358979323846626433
#endif


#endif