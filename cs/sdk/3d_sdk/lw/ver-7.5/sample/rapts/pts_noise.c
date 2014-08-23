/*
 * pts_noise.c - Various noise functions for use in Lightwave shaders
 *
 * This file contains source code for procedures described in
 * "Texturing and Modeling: A Procedural Approach" by Ebert, Musgrave,
 * Peachey, Perlin, and Worley; Academic Press, 1998.
 */

#include <math.h>

#include "pts_noise.h"
#include "pts_math.h"

unsigned char perm[TABSIZE] = {
        225,155,210,108,175,199,221,144,203,116, 70,213, 69,158, 33,252,
          5, 82,173,133,222,139,174, 27,  9, 71, 90,246, 75,130, 91,191,
        169,138,  2,151,194,235, 81,  7, 25,113,228,159,205,253,134,142,
        248, 65,224,217, 22,121,229, 63, 89,103, 96,104,156, 17,201,129,
         36,  8,165,110,237,117,231, 56,132,211,152, 20,181,111,239,218,
        170,163, 51,172,157, 47, 80,212,176,250, 87, 49, 99,242,136,189,
        162,115, 44, 43,124, 94,150, 16,141,247, 32, 10,198,223,255, 72,
         53,131, 84, 57,220,197, 58, 50,208, 11,241, 28,  3,192, 62,202,
         18,215,153, 24, 76, 41, 15,179, 39, 46, 55,  6,128,167, 23,188,
        106, 34,187,140,164, 73,112,182,244,195,227, 13, 35, 77,196,185,
         26,200,226,119, 31,123,168,125,249, 68,183,230,177,135,160,180,
         12,  1,243,148,102,166, 38,238,251, 37,240,126, 64, 74,161, 40,
        184,149,171,178,101, 66, 29, 59,146, 61,254,107, 42, 86,154,  4,
        236,232,120, 21,233,209, 45, 98,193,114, 78, 19,206, 14,118,127,
         48, 79,147, 85, 30,207,219, 54, 88,234,190,122, 95, 67,143,109,
        137,214,145, 93, 92,100,245,  0,216,186, 60, 83,105, 97,204, 52
};


static p[B + B + 2];
static double g3[B + B + 2][3];
static double g2[B + B + 2][2];
static double g1[B + B + 2];
static start = 1;

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;

static void
init (void)
{
  int i, j, k;

  sgenrand(665);
  for (i = 0; i < B; i++) {
    p[i] = i;

    g1[i] = 1. - 2.*RANDNBR;

    for (j = 0; j < 2; j++)
      g2[i][j] = 1. - 2.*RANDNBR;
    normalize2 (g2[i]);

    for (j = 0; j < 3; j++)
      g3[i][j] = 1. - 2.*RANDNBR;
    normalize3 (g3[i]);
  }

  while (--i) {
    k = p[i];
    p[i] = p[j = genrand() % B];
    p[j] = k;
  }

  for (i = 0; i < B + 2; i++) {
    p[B + i] = p[i];
    g1[B + i] = g1[i];
    for (j = 0; j < 2; j++)
      g2[B + i][j] = g2[i][j];
    for (j = 0; j < 3; j++)
      g3[B + i][j] = g3[i][j];
  }
}

double
pnoise1 (double arg)
{
  int bx0, bx1;
  double rx0, rx1, sx, t, u, v, vec[1];

  vec[0] = arg;
  if (start) {
    start = 0;
    init ();
  }

  setup (0, bx0, bx1, rx0, rx1);

  sx = s_curve (rx0);

  u = rx0 * g1[p[bx0]];
  v = rx1 * g1[p[bx1]];

  return LERP (sx, u, v);
}

double
pnoise2 (double vec[2])
{
  int bx0, bx1, by0, by1, b00, b10, b01, b11;
  double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
  register i, j;

  if (start) {
    start = 0;
    init ();
  }

  setup (0, bx0, bx1, rx0, rx1);
  setup (1, by0, by1, ry0, ry1);

  i = p[bx0];
  j = p[bx1];

  b00 = p[i + by0];
  b10 = p[j + by0];
  b01 = p[i + by1];
  b11 = p[j + by1];

  sx = s_curve (rx0);
  sy = s_curve (ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

  q = g2[b00];
  u = at2 (rx0, ry0);
  q = g2[b10];
  v = at2 (rx1, ry0);
  a = LERP (sx, u, v);

  q = g2[b01];
  u = at2 (rx0, ry1);
  q = g2[b11];
  v = at2 (rx1, ry1);
  b = LERP (sx, u, v);

  return LERP (sy, a, b);
}

double
pnoise3 (double vec[3])
{
  int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
  double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
  register i, j;

  if (start) {
    start = 0;
    init ();
  }

  setup (0, bx0, bx1, rx0, rx1);
  setup (1, by0, by1, ry0, ry1);
  setup (2, bz0, bz1, rz0, rz1);

  i = p[bx0];
  j = p[bx1];

  b00 = p[i + by0];
  b10 = p[j + by0];
  b01 = p[i + by1];
  b11 = p[j + by1];

  t = s_curve (rx0);
  sy = s_curve (ry0);
  sz = s_curve (rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

  q = g3[b00 + bz0];
  u = at3 (rx0, ry0, rz0);
  q = g3[b10 + bz0];
  v = at3 (rx1, ry0, rz0);
  a = LERP (t, u, v);

  q = g3[b01 + bz0];
  u = at3 (rx0, ry1, rz0);
  q = g3[b11 + bz0];
  v = at3 (rx1, ry1, rz0);
  b = LERP (t, u, v);

  c = LERP (sy, a, b);

  q = g3[b00 + bz1];
  u = at3 (rx0, ry0, rz1);
  q = g3[b10 + bz1];
  v = at3 (rx1, ry0, rz1);
  a = LERP (t, u, v);

  q = g3[b01 + bz1];
  u = at3 (rx0, ry1, rz1);
  q = g3[b11 + bz1];
  v = at3 (rx1, ry1, rz1);
  b = LERP (t, u, v);

  d = LERP (sy, a, b);

  return LERP (sz, c, d);
}

double
pnoise (double vec[], int len)
{
  switch (len) {
  case 0:
    return 0.;
  case 1:
    return pnoise1 (vec[0]);
  case 2:
    return pnoise2 (vec);
  default:
    return pnoise3 (vec);
  }
}

double
catrom2(double d)
{
#define SAMPRATE 100  /* table entries per unit distance */
#define NENTRIES (4*SAMPRATE+1)
    double x;
    int i;
    static double table[NENTRIES];
    static int initialized = 0;

    if (d >= 4)
        return 0;

    if (!initialized) {
        for (i = 0; i < NENTRIES; i++) {
            x = (double) i / (double) SAMPRATE;
            x = sqrt(x);
            if (x < 1)
                table[i] = 0.5 * (2+x*x*(-5+x*3));
            else
                table[i] = 0.5 * (4+x*(-8+x*(5-x)));
        }
        initialized = 1;
    }

    d = d*SAMPRATE + 0.5;
    i = FLOOR(d);
    if (i >= NENTRIES)
        return 0;
    return table[i];
}


static double gradientTab[TABSIZE*3];

static void
gradientTabInit(int seed)
{
    double *table = gradientTab;
    double z, r, theta;
    int i;

    sgenrand(seed);
    for(i = 0; i < TABSIZE; i++) {
        z = 1. - 2.*RANDNBR;
        /* r is radius of x,y circle */
        r = sqrt(1 - z*z);
        /* theta is angle in (x,y) */
        theta = 2 * M_PI * RANDNBR;
        *table++ = r * cos(theta);
        *table++ = r * sin(theta);
        *table++ = z;
    }
}

static double
glattice(int ix, int iy, int iz,
    double fx, double fy, double fz)
{
    double *g = &gradientTab[INDEX(ix,iy,iz)*3];
    return g[0]*fx + g[1]*fy + g[2]*fz;
}

double
gnoise(double x, double y, double z)
{
    int ix, iy, iz;
    double fx0, fx1, fy0, fy1, fz0, fz1;
    double wx, wy, wz;
    double vx0, vx1, vy0, vy1, vz0, vz1;
    double val;
    static int initialized = 0;

    if (!initialized) {
        gradientTabInit(665);
        initialized = 1;
    }

    ix = FLOOR(x);
    fx0 = x - ix;
    fx1 = fx0 - 1;
    wx = SMOOTHSTEP(fx0);

    iy = FLOOR(y);
    fy0 = y - iy;
    fy1 = fy0 - 1;
    wy = SMOOTHSTEP(fy0);

    iz = FLOOR(z);
    fz0 = z - iz;
    fz1 = fz0 - 1;
    wz = SMOOTHSTEP(fz0);

    vx0 = glattice(ix,iy,iz,fx0,fy0,fz0);
    vx1 = glattice(ix+1,iy,iz,fx1,fy0,fz0);
    vy0 = LERP(wx, vx0, vx1);
    vx0 = glattice(ix,iy+1,iz,fx0,fy1,fz0);
    vx1 = glattice(ix+1,iy+1,iz,fx1,fy1,fz0);
    vy1 = LERP(wx, vx0, vx1);
    vz0 = LERP(wy, vy0, vy1);

    vx0 = glattice(ix,iy,iz+1,fx0,fy0,fz1);
    vx1 = glattice(ix+1,iy,iz+1,fx1,fy0,fz1);
    vy0 = LERP(wx, vx0, vx1);
    vx0 = glattice(ix,iy+1,iz+1,fx0,fy1,fz1);
    vx1 = glattice(ix+1,iy+1,iz+1,fx1,fy1,fz1);
    vy1 = LERP(wx, vx0, vx1);
    vz1 = LERP(wy, vy0, vy1);

    val = LERP(wz, vz0, vz1);
    return val;
}


static double valueTab[TABSIZE];

static void
valueTabInit(int seed)
{
    double *table = valueTab;
    int i;

    sgenrand(seed);
    for(i = 0; i < TABSIZE; i++)
        *table++ = 1. - 2.*RANDNBR;
}

static double
vlattice(int ix, int iy, int iz)
{
    return valueTab[INDEX(ix,iy,iz)];
}

double
vnoise(double x, double y, double z)
{
    int ix, iy, iz;
    int i, j, k;
    double fx, fy, fz;
    double xknots[4], yknots[4], zknots[4];
    double val;
    static int initialized = 0;

    if (!initialized) {
        valueTabInit(665);
        initialized = 1;
    }

    ix = FLOOR(x);
    fx = x - ix;

    iy = FLOOR(y);
    fy = y - iy;

    iz = FLOOR(z);
    fz = z - iz;

    for (k = -1; k <= 2; k++) {
        for (j = -1; j <= 2; j++) {
            for (i = -1; i <= 2; i++)
                xknots[i+1] = vlattice(ix+i,iy+j,iz+k);
            yknots[j+1] = spline(fx, 4, xknots);
        }
        zknots[k+1] = spline(fy, 4, yknots);
    }
    val = spline(fz, 4, zknots);
    return val;
}


/*
 * A simple gradient+value noise: a weighted sum of gnoise and vnoise.
 */
double
gvnoise(double x, double y, double z)
{
    double	 val;

    val = 0.3 * vnoise(x,y,z) +  0.7 * gnoise(x,y,z);
    return val;
}


static double impulseTab[TABSIZE*4];
static void impulseTabInit(int seed);

static void
impulseTabInit(int seed)
{
    int i;
    double *f = impulseTab;

    sgenrand(seed); /* Set random number generator seed. */
    for (i = 0; i < TABSIZE; i++) {
        *f++ = RANDNBR;
        *f++ = RANDNBR;
        *f++ = RANDNBR;
        *f++ = 1. - 2.*RANDNBR;
    }
}

double
scnoise(double x, double y, double z)
{
    static int initialized;
    double *fp;
    int i, j, k, h, n;
    int ix, iy, iz;
    double sum = 0;
    double fx, fy, fz, dx, dy, dz, distsq;
    double val;

    /* Initialize the random impulse table if necessary. */
    if (!initialized) {
        impulseTabInit(665);
        initialized = 1;
    }

    ix = FLOOR(x); fx = x - ix;
    iy = FLOOR(y); fy = y - iy;
    iz = FLOOR(z); fz = z - iz;
    
    /* Perform the sparse convolution. */
    for (i = -2; i <= 2; i++) {
      for (j = -2; j <= 2; j++) {
        for (k = -2; k <= 2; k++) {
            /* Compute voxel hash code. */
            h = INDEX(ix+i,iy+j,iz+k);
            
            for (n = NIMPULSES; n > 0; n--, h = NEXT(h)) {
                /* Convolve filter and impulse. */
                fp = &impulseTab[h*4];
                dx = fx - (i + *fp++);
                dy = fy - (j + *fp++);
                dz = fz - (k + *fp++);
                distsq = dx*dx + dy*dy + dz*dz;
                sum += catrom2(distsq) * *fp;
            }
        }
      }
    }

    val = sum / NIMPULSES;
    return val;
}


double
vcnoise(double x, double y, double z)
{
    int ix, iy, iz;
    int i, j, k;
    double fx, fy, fz;
    double dx, dy, dz;
    double sum = 0;
    static int initialized = 0;

    if (!initialized) {
        valueTabInit(665);
        initialized = 1;
    }

    ix = FLOOR(x);
    fx = x - ix;

    iy = FLOOR(y);
    fy = y - iy;

    iz = FLOOR(z);
    fz = z - iz;

    for (k = -1; k <= 2; k++) {
        dz = k - fz;
        dz = dz*dz;
        for (j = -1; j <= 2; j++) {
            dy = j - fy;
            dy = dy*dy;
            for (i = -1; i <= 2; i++){
                dx = i - fx;
                dx = dx*dx;
                sum += vlattice(ix+i,iy+j,iz+k)
                    * catrom2(dx + dy + dz);
            }
        }
    }
    return sum;
}
