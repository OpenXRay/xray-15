/*
 * pts_frctl.h - Declarations and preprocessor definitions for fractal
 *               functions used in the shading routines.
 */

#ifndef _PTSFRCTL_H
#define _PTSFRCTL_H 1

typedef enum {
  PNOISE, VNOISE, GNOISE, GVNOISE, VCNOISE, SCNOISE
} FracNoise;

extern double noisey (double *, FracNoise);
extern double fBm (double *, double, double, double, FracNoise);
extern double turbulence (double *, double, double, double, FracNoise);
extern double multifractal (double *, double, double, double, double,
                            FracNoise);
extern double heteroterrain (double *, double, double, double, double,
                             FracNoise);
extern double HybridMultifractal (double *, double, double, double, double,
                                  FracNoise);
extern double RidgedMultifractal (double *, double, double, double, double,
                                  double, FracNoise);

#endif /* _PTSFRCTL_H */
