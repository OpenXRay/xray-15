/*
 * shd_frctl.c - Common functions for fractal terrain models.
 *
 * This file contains source code for procedures described in
 * "Texturing and Modeling: A Procedural Approach" by Ebert, Musgrave,
 * Peachey, Perlin, and Worley; Academic Press, 1998.
 */

#include <math.h>

#include "shd_frctl.h"
#include "shd_math.h"
#include "shd_noise.h"

/*
 * Call the appropriate basis function.
 */
double
noisey (double pt[3], FracNoise basis)
{
  double	 val;

  switch (basis) {
    case PNOISE:
      val = pnoise3(pt);
     break;
    case VNOISE:
      val = vnoise(pt[0], pt[1], pt[2]);
      break;
    case GNOISE:
      val = gnoise(pt[0], pt[1], pt[2]);
      break;
    case GVNOISE:
      val = gvnoise(pt[0], pt[1], pt[2]);
      break;
    case VCNOISE:
      val = vcnoise(pt[0], pt[1], pt[2]);
      break;
    case SCNOISE:
      val = scnoise(pt[0], pt[1], pt[2]);
      break;
  }

  return (val);
}


/* 
 * Ridged multifractal - F. Kenton Musgrave
 */
double 
RidgedMultifractal (double point[3], double increment, double lacunarity,
		    double octaves, double offset, double threshold,
                    FracNoise basis)
{
  double	 result, frequency, signal, weight;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  /* get first octave */
  signal = noisey (tpt, basis);
  /* get absolute value of signal (this creates the ridges) */
  signal = Abs (signal);
  /* invert and translate (note that "offset" should be ~= 1.0) */
  signal = offset - signal;
  /* square the signal, to increase "sharpness" of ridges */
  signal *= signal;
  /* assign initial values */
  result = signal;
  weight = 1.0;

  frequency = lacunarity;
  for(i = 1; weight > 0.001 && i < octaves; i++ ) {

    Vec3MultC (tpt, tpt, lacunarity);

    /* weight successive contributions by previous signal */
    weight = signal * threshold;
    weight = clamp (weight, 0.0, 1.0);
    signal = noisey (tpt, basis);
    signal = Abs (signal);
    signal = offset - signal;
    signal *= signal;
    /* weight the contribution */
    signal *= weight;
    result += signal * pow(frequency, -increment);
    frequency *= lacunarity;
  }

  return (result);
} /* RidgedMultifractal() */


/* 
 * Stats-by-Position multifractal - F. Kenton Musgrave
 */
double 
HybridMultifractal (double point[3], double increment, double lacunarity, 
		    double octaves, double offset, FracNoise basis)
{
  double	 value, frequency, signal, weight, remainder;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  /* get first octave of function; later octaves are weighted */
  value = noisey (tpt, basis) + offset;
  weight = value;

  Vec3MultC (tpt, tpt, lacunarity);
  frequency = lacunarity;
  /* inner loop of spectral construction, where the fractal is built */
  for (i = 1; weight > 0.001 && i < octaves; i++) {
    /* prevent divergence */
    if (weight > 1.0)  weight = 1.0;

    /* get next higher frequency */
    signal = (noisey (tpt, basis) + offset) * pow(frequency, -increment);
    /* add it in, weighted by previous freq's local value */
    value += weight * signal;
    /* update the (monotonically decreasing) weighting value */
    weight *= signal;

    Vec3MultC (tpt, tpt, lacunarity);
    frequency *= lacunarity;
  } /* for */

  /* take care of remainder in "octaves" */
  remainder = octaves - (int)octaves;
  if (remainder)
    /* "i" and spatial freq. are preset in loop above */
    value += remainder * noisey (tpt, basis) * pow(frequency, -increment);

  return( value );
} /* HybridMultifractal() */


/*
 * Heterogeneous terrain function - F. Kenton Musgrave
 */
double
HeteroTerrain (double point[3], double increment, double lacunarity,
               double octaves, double offset, FracNoise basis)
{
  double	 value, frequency, signal, remainder;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  /* first unscaled octave of function; later octaves are scaled */
  value = offset + noisey (tpt, basis);
  Vec3MultC (tpt, tpt, lacunarity);

  frequency = lacunarity;
  /* inner loop of spectral construction, where the fractal is built */
  for (i = 1; i < octaves; i++) {
    /* obtain displaced noise value */
    signal = (noisey (tpt, basis) + offset) * pow(frequency, -increment);
    /* scale signal bcurrent altitude function */
    signal *= value;
    /* add signal to "value" */
    value += signal;
    Vec3MultC (tpt, tpt, lacunarity);
    frequency *= lacunarity;
  } /* for */

  /* take care of remainder in "octaves" */
  remainder = octaves - (int)octaves;
  if (remainder) {
    /* "i" and spatial freq. are preset in loop above */
    signal = (noisey (tpt, basis) + offset) * pow(frequency, -increment);
    value += remainder * signal * value;
  }

  return( value );
} /* Hetero_Terrain() */


/*
 * Procedural multifractal evaluated at "point" - F. Kenton Musgrave
 * 
 * Parameters:
 * 	"lacunarity" is gap between successive frequencies
 * 	"octaves" is the number of frequencies in the fBm
 * 	"scale" scales the basis function & determines multifractality
 */
double
multifractal (double point[3], double increment, double lacunarity,
              double octaves, double offset, FracNoise basis)
{
  double	 value, frequency, remainder;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  value = 1.0;
  frequency = 1.0;
  /* inner loop of spectral construction, where the fractal is built */
  for (i = 0; i < octaves; i++) {
    value *= (noisey (tpt, basis) + offset) * pow(frequency, -increment);
    Vec3MultC (tpt, tpt, lacunarity);
    frequency *= lacunarity;
  } /* for */

  /* take care of remainder in "octaves" */
  remainder = octaves - (int)octaves;
  if (remainder)
    /* "i" and spatial freq. are preset in loop above */
    value += remainder * (noisey (tpt, basis) + offset) * pow(frequency, -increment);

  return value;
} /* multifractal() */


/*
 * Procedural fBm function - F. Kenton Musgrave
 */
double
fBm (double point[3], double increment, double lacunarity, double octaves,
     FracNoise basis)
{
  double	 value, frequency, remainder;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  value = 0.0;
  frequency = 1.0;
  /* inner loop of spectral construction, where the fractal is built */
  for (i = 0; i < octaves; i++) {
    value += noisey (tpt, basis) * pow(frequency, -increment);
    frequency *= lacunarity;
    Vec3MultC (tpt, tpt, lacunarity);
  } /* for */

  /* take care of remainder in "octaves" */
  remainder = octaves - (int)octaves;
  if (remainder)
    value += remainder * noisey (tpt, basis) * pow(frequency, -increment);

  return( value );
} /* fBm */


/*
 * Procedural turbulence function - F. Kenton Musgrave
 */
double
turbulence (double point[3], double increment, double lacunarity,
            double octaves, FracNoise basis)
{
  double	 temp, frequency, value, remainder;
  double	 tpt[3];
  int		 i;

  Vec3Copy(tpt, point);

  value = 0.0;
  frequency = 1.0;
  /* inner loop of spectral construction, where the fractal is built */
  for (i = 0; i < octaves; i++) {
    temp = noisey (tpt, basis) * pow(frequency, -increment);
    value += Abs(temp);
    frequency *= lacunarity;
    Vec3MultC (tpt, tpt, lacunarity);
  } /* for */

  /* take care of remainder in "octaves" */
  remainder = octaves - (int)octaves;
  if (remainder) {
    /* "i" and spatial freq. are preset in loop above */
    temp = remainder * noisey (tpt, basis) * pow(frequency, -increment);
    value += Abs(temp);
  }

  return( value );
} /* turbulence() */

