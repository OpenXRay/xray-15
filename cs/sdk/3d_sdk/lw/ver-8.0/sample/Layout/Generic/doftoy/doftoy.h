/*
======================================================================
doftoy.h

Definitions and function prototypes for DOF Toy.

Ernie Wright  11 Jul 00
====================================================================== */

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define INVALID_NUMBER 1e12

/* calculate a parameter in terms of others */

double nx_s( double s, double norx );
double s_nx( double n, double x );
double s_n( double f, double a, double c, double n );
double s_x( double f, double a, double c, double x );
double a_n( double f, double s, double n, double c );
double f_n( double a, double c, double n, double s );
double n_h( double f, double a, double c, double s );
double z_vfov( double vfov );
double z_hfov( double hfov, double faspect );
double vfov_z( double z );
double hfov_z( double z, double faspect );
double z_fp( double f, double p );
double f_pz( double p, double z );
double p_fz( double f, double z );

/* restore mathematical consistency to the parameters */

int update_params( int changed, double *param, int locka, int lockb );

/* read and write edit field strings */

double sget_dist( char *a );
double sget_angle( char *a );
double sget_double( char *a );
char *sput_dist( char *a, double d );
char *sput_angle( char *a, double d );
char *sput_double( char *a, double d );
