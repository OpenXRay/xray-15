/*
 * shd_noise.c - Various noise functions for use in Lightwave shaders
 *
 * This file contains source code for the procedures described in 
 * "Texturing and Modeling: A Procedural Approach" by Ebert, Musgrave,
 * Peachey, Perlin, and Worley (copyright 1994 AP Professional).
 */

#include <math.h>

#include "shd_noise.h"
#include "shd_math.h"

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
