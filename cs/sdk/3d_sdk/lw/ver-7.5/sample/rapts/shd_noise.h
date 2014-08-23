/*
 * shd_noise.h - Declarations for the various noise functions.
 */

#ifndef _SHDNOISE_H_
#define _SHDNOISE_H_ 1

#include "shd_rand.h"

#define TABSIZE		256
#define TABMASK		(TABSIZE-1)
#define PERM(x)		perm[(x)&TABMASK]
#define INDEX(ix,iy,iz)	PERM((ix)+PERM((iy)+PERM(iz)))
#define SMOOTHSTEP(x)	((x)*(x)*(3 - 2*(x)))
#define RANDMASK	0x7fffffff
#define RANDNBR		((genrand() & RANDMASK)/(double) RANDMASK)
#define NEXT(h)		(((h)+1) & TABMASK)
#define NIMPULSES	3


extern double gnoise(double x, double y, double z);
extern double vnoise(double x, double y, double z);
extern double gvnoise(double x, double y, double z);
extern double scnoise(double x, double y, double z);
extern double vcnoise(double x, double y, double z);

#endif /* _SHDNOISE_H_ */
