/*  ARBAXIS.C    9/15/88 */


/*  This module defines the ARBAXIS routine for obtaining an
    arbitrary but repeatable coordinate axis from a given entity
    normal (extrusion direction).  In this manner, a complete coordinate
    system can be (reliably) built from only 3 real numbers.

    Support routines are provided here for creating a 4x4 coordinate
    system matrix from an entity normal, and for transforming a point
    given the normal.  Some general vector geometry support routines
    are also included.


*/


#include <math.h>

#define X  0			   /* Coordinate indices */
#define Y  1
#define Z  2
#define T  3

#define TRUE  1
#define FALSE 0

typedef double point[3];	   /* Three dimensional point */
typedef double vector[4];	   /* Homogeneous coordinate vector */
typedef double matrix[4][4];	   /* Transformation matrix */

#define ARBBOUND  0.015625  /* aka "1/64" */
#define ZERO 1e-10

/*  Forward decls */


/*  Local stuff */

static double yaxis[3] = {0.0, 1.0, 0.0};
static double zaxis[3] = {0.0, 0.0, 1.0};

/*  MATIDENT  --  Set a matrix to the identity matrix  */

void matident(matrix a)
{
	register int i, j;

	for (i = 0; i < 4; i++) {
	   for (j = 0; j < 4; j++) {
	      a[i][j] = (i == j) ? 1.0 : 0.0;
	   }
	}
}

/*  POINTCOPY  --  Copy point to another  */

void pointcopy(point po, point p)
{
	po[X] = p[X];
	po[Y] = p[Y];
	po[Z] = p[Z];
}

/*  VECCROSS  --  Get the cross product of two vectors.  Return
                  the result in o.  */

void veccross(point o, point a, point b)
{
	point r;

	r[X] = a[Y] * b[Z] - a[Z] * b[Y];
	r[Y] = a[Z] * b[X] - a[X] * b[Z];
	r[Z] = a[X] * b[Y] - a[Y] * b[X];

	pointcopy(o, r);
}

/*  VECSCAL  --  Multiply vector by a scalar and store the result
		 in a second vector.  Expects points.  */

void vecscal(point o, point a, double s)
{
	o[X] = a[X] * s;
	o[Y] = a[Y] * s;
	o[Z] = a[Z] * s;
}

/*  VECMAG  --	Returns magnitude of a vector.	This expects a point
		and uses only the first three terms.  */

double vecmag(point a)
{
	return sqrt(a[X] * a[X] + a[Y] * a[Y] + a[Z] * a[Z]);
}

/*  VECNORM  --  Normalise vector and store normalised result in
		 a second vector.  Works on points.  */

void vecnorm(point o, point a)
{
	vecscal(o, a, 1.0 / vecmag(a));
}

/*  MATORIE  --  Specify explicit orientation  */

void matorie(matrix m, double a, double b, double c, double d, double e, double f, double p, double q, double r)
{
	matident(m);
	m[0][0] = a;
	m[1][0] = b;
	m[2][0] = c;
	m[0][1] = d;
	m[1][1] = e;
	m[2][1] = f;
	m[0][2] = p;
	m[1][2] = q;
	m[2][2] = r;
}

/*  ARBAXIS -- Given a unit vector to be used as one of the axes of a
               coordinate system, choose a second axis.  The choice is
               in principal arbitrary; we just want to make it pre-
               dictable.  Our method is to examine the given Z axis
               and see if it is close to the world Z axis or the
               negative of the world Z axis.  If it is, we cross the
               given Z axis with the world Y axis to arrive at the
               arbitrary X axis.  If not, we cross the given Z axis with
               the world Z axis to arrive at the arbitrary X axis.
               The boundary at which the decision is made was designed to
               be both inexpensive to calculate and to be completely
               portable across machines.  This is done by having a
               sort of "square" polar cap, the bounds of which is
               1/64, which is fully specifiable in 6 decimal fraction
               digits and in 6 binary fraction bits.

               To get a right-handed system, treat the axis returned by
               this function as the cyclically next one after the given
               axis (e.g., if you give it a Z-axis, it returns a
               suitable X-axis).

*/
void arbaxis(double *newaxis, double *givenaxis)
{
    if (fabs(givenaxis[X]) < ARBBOUND && fabs(givenaxis[Y]) < ARBBOUND)
        veccross(newaxis, yaxis, givenaxis);
    else
        veccross(newaxis, zaxis, givenaxis);

    vecnorm(newaxis, newaxis);
}

/*  GETA4BY4 -- Given the entity's normal (extrusion direction vector),
                return a 4x4 matrix that defines the transform in 3D.
                mat is assumed to be matrix[4][4], where the
                the first subscript represents X,Y,Z,Dummy.

                Speed is sacrificed here in favor of comprehensibility.
*/

void geta4by4(point normaxis, matrix mat)
{
    point xdir, ydir, zdir;

    pointcopy(zdir, normaxis);

    /* First get the arbitrary x axis of the entity given its norm (Z) */

    arbaxis(xdir, zdir);

    /* Now we cross our new X-axis with our norm (Z) to get the Y-axis */

    veccross(ydir, zdir, xdir);

    /* And we normalize that to a unit vector */

    vecnorm(ydir, ydir);

    /* We now have all the components of the transformation matrix. */
    
    matorie(mat, xdir[X], xdir[Y], xdir[Z],
                 ydir[X], ydir[Y], ydir[Z],
                 zdir[X], zdir[Y], zdir[Z]);

}

/*  TRANS  --   Transform a point, given a normal vector (3 reals).
                This uses geta4by4, so it's not super fast.
                Returns the point in o.  */

void trans(point o, point pt, point vec)
{
    register int i;
    matrix m;
    point sum;

    /* First, get the transformation matrix from the normal */

    geta4by4(vec, m);

    /* Now, do a standard matrix transformation on the point. */

    for (i=0; i<3; i++)
        sum[i] = m[i][0] * pt[X] +
                 m[i][1] * pt[Y] +
                 m[i][2] * pt[Z] +
                 m[i][3];

    o[X] = sum[X];
    o[Y] = sum[Y];
    o[Z] = sum[Z];

}

