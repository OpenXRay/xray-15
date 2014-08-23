/*
 * pts_math.c - Common functions that assist in converting Renderman shaders
 *              Lightwave shaders. 
 *
 * This file contains source code for the procedures described in 
 * "Texturing and Modeling: A Procedural Approach" by Ebert, Musgrave,
 * Peachey, Perlin, and Worley (copyright 1994 AP Professional).
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#include "pts_math.h"


double
Abs (double x)
{
  return (x < 0 ? -x : x);
}

double
bias (double b, double x)
{
  return pow (x, log (b) / LOG05);
}

double
clamp (double x, double a, double b)
{
  return (x < a ? a : (x > b ? b : x));
}

double
distance(double V1[3], double V2[3])
{
  double V[3];

  V[0] = V1[0] - V2[0];
  V[1] = V1[1] - V2[1];
  V[2] = V1[2] - V2[2];
  return(length(V));
}

double
length(double *V1)
{
  double value;

  value = sqrt(V1[0]*V1[0] + V1[1]*V1[1] + V1[2]*V1[2]);
  return(value);
}

void
mix (double *C, double *C0, double *C1, double f)
{
  C[0] = ((1.0 - f) * C0[0]) + (f * C1[0]);
  C[1] = ((1.0 - f) * C0[1]) + (f * C1[1]);
  C[2] = ((1.0 - f) * C0[2]) + (f * C1[2]);
}

void
faceforward (double ffn[3], double n1[3], double n2[3])
{
  if (Vec3Dot (n1, n2) > 0) {
    ffn[0] = -n1[0];
    ffn[1] = -n1[1];
    ffn[2] = -n1[2];
  }
  else {
    ffn[0] = n1[0];
    ffn[1] = n1[1];
    ffn[2] = n1[2];
  }
}

double
fract (double x)
{
  return ((double) (x - simplefloor (x)));
}

double
gain (double g, double x)
{
  if (x < 0.5)
    return bias (1 - g, 2 * x) / 2;
  else
    return 1 - bias (1 - g, 2 - 2 * x) / 2;
}

double
gammacorrect (double gamma, double x)
{
  return pow (x, 1 / gamma);
}

double
dmax (double a, double b)
{
  return (a < b ? b : a);
}

double
dmin (double a, double b)
{
  return (a < b ? a : b);
}

double
mod (double a, double b)
{
  int n = (int) (a / b);

  a -= n * b;
  if (a < 0)
    a += b;
  return a;
}

void
normalize2 (double v[2])
{
  double s;

  s = sqrt (v[0] * v[0] + v[1] * v[1]);
  v[0] = v[0] / s;
  v[1] = v[1] / s;
}

void
normalize3 (double v[3])
{
  double s;

  s = sqrt (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] = v[0] / s;
  v[1] = v[1] / s;
  v[2] = v[2] / s;
}

//begin eetu = beware! :)

double
pulse(double a, double b, double fuzz, double x)
{
        return smoothstep(a - fuzz, a, x) - smoothstep(b - fuzz, b, x);
}

//end eetu

double
smoothstep (double a, double b, double x)
{
  if (x < a)
    return 0;
  if (x >= b)
    return 1;
  x = (x - a) / (b - a);	/* normalize to [0:1] */
  return (x * x * (3 - 2 * x));
}

double
smoothstepd (double a, double b, double x)
{
  if ((x < a) || (x >= b))
    return 0;
  x = (x - a) / (b - a);	/* normalize to [0:1] */
  return (6 * x * (1 - x));
}

double
spline (double x, int nknots, double *knot)
{
  int span;
  int nspans = nknots - 3;
  double c0, c1, c2, c3;	/* coefficients of the cubic. */

  if (nspans < 1) {		/* illegal */
    return 0;
  }

  /* Find the appropriate 4-point span of the spline. */
  x = clamp (x, 0, 1) * nspans;
  span = (int) x;
  if (span >= nknots - 3)
    span = nknots - 3;
  x -= span;
  knot += span;

  /* Evaluate the span cubic at x using Horner's rule. */
  c3 = CR00 * knot[0] + CR01 * knot[1]
    + CR02 * knot[2] + CR03 * knot[3];
  c2 = CR10 * knot[0] + CR11 * knot[1]
    + CR12 * knot[2] + CR13 * knot[3];
  c1 = CR20 * knot[0] + CR21 * knot[1]
    + CR22 * knot[2] + CR23 * knot[3];
  c0 = CR30 * knot[0] + CR31 * knot[1]
    + CR32 * knot[2] + CR33 * knot[3];

  return ((c3 * x + c2) * x + c1) * x + c0;
}

void
Cspline (double *C, double x, int nknots, double *knots)
{
  double *newknot, *knotPtr;
  int i;

  newknot = calloc (nknots, sizeof (double));

  for (i = 0, knotPtr = knots; i < nknots; i++, knotPtr += 3)
    newknot[i] = *knotPtr;
  C[0] = spline (x, nknots, newknot);
  for (i = 0, knotPtr = knots + 1; i < nknots; i++, knotPtr += 3)
    newknot[i] = *knotPtr;
  C[1] = spline (x, nknots, newknot);
  for (i = 0, knotPtr = knots + 2; i < nknots; i++, knotPtr += 3)
    newknot[i] = *knotPtr;
  C[2] = spline (x, nknots, newknot);

  free (newknot);

  return;
}

double
step (double a, double x)
{
  return (double) (x >= a);
}

