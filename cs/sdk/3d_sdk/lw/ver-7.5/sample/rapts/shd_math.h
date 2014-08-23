/*
 * shd_math.h - Declarations and preprocessor definitions for math functions
 *              used in the shading routines.
 */

#ifndef _SHDMATH_H_
#define _SHDMATH_H_ 1

#ifndef M_PI
#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923		//  (PI / 2)
#endif
#define M_2_PI	6.2831832			//  (PI * 2)
#define LOG05	-0.693147180559945		//  log(0.5)
#define RAD2DEG	57.29578			//  (180 / PI) 

#define B	0x100
#define BM	0xff
#define N	0x1000
#define NM	0xfff
#define NP	12

/*
 * Coefficients of spline basis matrix.
 */
#define CR00     -0.5
#define CR01      1.5
#define CR02     -1.5
#define CR03      0.5
#define CR10      1.0
#define CR11     -2.5
#define CR12      2.0
#define CR13     -0.5
#define CR20     -0.5
#define CR21      0.0
#define CR22      0.5
#define CR23      0.0
#define CR30      0.0
#define CR31      1.0
#define CR32      0.0
#define CR33      0.0

#define Vec3Add(c,a,b)   {(c)[0] = (a)[0] + (b)[0]; (c)[1] = (a)[1] + (b)[1];\
			  (c)[2] = (a)[2] + (b)[2];}
#define Vec3Sub(c,a,b)   {(c)[0] = (a)[0] - (b)[0]; (c)[1] = (a)[1] - (b)[1];\
			  (c)[2] = (a)[2] - (b)[2];}
#define Vec3Mult(c,a,b)  {(c)[0] = (a)[0] * (b)[0]; (c)[1] = (a)[1] * (b)[1];\
			  (c)[2] = (a)[2] * (b)[2];}
#define Vec3Div(c,a,b)   {(c)[0] = (a)[0] / (b)[0]; (c)[1] = (a)[1] / (b)[1];\
			  (c)[2] = (a)[2] / (b)[2];}
#define Vec3AddC(c,a,b)  {(c)[0] = (a)[0] + (b); (c)[1] = (a)[1] + (b);\
			  (c)[2] = (a)[2] + (b);}
#define Vec3SubC(c,a,b)  {(c)[0] = (a)[0] - (b); (c)[1] = (a)[1] - (b);\
			  (c)[2] = (a)[2] - (b);}
#define Vec3MultC(c,a,b) {(c)[0] = (a)[0] * (b); (c)[1] = (a)[1] * (b);\
			  (c)[2] = (a)[2] * (b);}
#define Vec3DivC(c,a,b)  {(c)[0] = (a)[0] / (b); (c)[1] = (a)[1] / (b);\
			  (c)[2] = (a)[2] / (b);}
#define Vec3Neg(c)	 {(c)[0] = -(c)[0]; (c)[1] = -(c)[1]; (c)[2] = -(c)[2];}
#define Vec3Inc(c,a)     {(c)[0] += (a)[0]; (c)[1] += (a)[1]; (c)[2] += (a)[2];}
#define Vec3Dec(c,a)     {(c)[0] -= (a)[0]; (c)[1] -= (a)[1]; (c)[2] -= (a)[2];}
#define Vec3IncC(c,a)    {(c)[0] += (a); (c)[1] += (a); (c)[2] += (a);}
#define Vec3DecC(c,a)    {(c)[0] -= (a); (c)[1] -= (a); (c)[2] -= (a);}
#define Vec3Copy(c,a)    {(c)[0] = (a)[0]; (c)[1] = (a)[1]; (c)[2] = (a)[2];}
#define Vec3CopyC(c,a)   {(c)[0] = (a); (c)[1] = (a); (c)[2] = (a);}
#define Vec3Cross(c,a,b) {(c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1];\
			  (c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2];\
			  (c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0];}
#define Vec3Dot(a,b)      ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define Vec3Assign(c,x,y,z) {(c)[0] = (x); (c)[1] = (y); (c)[2] = (z);}
#define Vec3Xform(c,a,b) {double t[3]; t[0]=(a)[0]; t[1]=(a)[1]; t[2]=(a)[2];\
                          (c)[0] = t[0]*(b)[0] + t[1]*(b)[1] + t[2]*(b)[2];\
                          (c)[1] = t[0]*(b)[3] + t[1]*(b)[4] + t[2]*(b)[5];\
                          (c)[2] = t[0]*(b)[6] + t[1]*(b)[7] + t[2]*(b)[8];}

#define FLOOR(x) ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define CEIL(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))
#define CLAMP(x,a,b) ((x) =< (a) ? (a) : ((x) >= (b) ? (b) : (x)))
#define LERP(t,x0,x1)  ((x0) + (t)*((x1)-(x0)))
#define PULSE(a,b,x) (step((a),(x)) - step((b),(x)))
#define blend(a,b,x) ((a) * (1 - (x)) + (b) * (x))
#define boxstep(a,b,x) clamp(((x)-(a))/((b)-(a)),0,1)
#define complement(a) (1 - (a))
#define difference(a,b) ((a) - (a) * (b))
#define even(x) (mod((x), 2) == 0)
#define intersection(a,b) ((a) * (b))
#define noise(x,y) (0.5 * pnoise((x),(y)) + 0.5)
#define noise1(x) (0.5 * pnoise1((x)) + 0.5)
#define noise2(x) (0.5 * pnoise2((x)) + 0.5)
#define noise3(x) (0.5 * pnoise3((x)) + 0.5)
#define odd(x) (mod((x), 2) == 1)
#define repeat(x,freq) (mod((x) * (freq), 1.0))
#define simplefloor(a) ((double)((long)(a)-((a)<0.0)))
#define s_curve(t) ((t) * (t) * (3. - 2. * (t)))
#define udn(x,lo,hi) (smoothstep(.25, .75, noise1(x)) * ((hi) - (lo)) + (lo))
#define udn2(x,y,lo,hi) (smoothstep(.25, .75, noise(x,y)) * ((hi)-(lo))+(lo))
#define union(a,b) ((a) + (b) - (a) * (b))
#define whichtile(x,freq) (floor((x) * (freq)))
#define xcomp(v) ((v)[0])
#define ycomp(v) ((v)[1])
#define zcomp(v) ((v)[2])

extern double Abs (double);
extern double bias (double, double);
extern double clamp (double, double, double);
extern double distance(double *, double *);
extern double dmax (double, double);
extern double dmin (double, double);
extern double fract (double);
extern double gain (double, double);
extern double gammacorrect (double, double);
extern double length(double *);
extern double mod (double, double);
extern double pnoise (double *, int);
extern double pnoise1 (double);
extern double pnoise2 (double *);
extern double pnoise3 (double *);
extern double pulse (double, double, double, double);
extern double smoothstep (double, double, double);
extern double smoothstepd (double, double, double);
extern double spline (double, int, double *);
extern double step (double, double);
extern void faceforward (double *, double *, double *);
extern void mix (double *, double *, double *, double);
extern void normalize2 (double *);
extern void normalize3 (double *);
extern void Cspline (double *, double, int, double *);

#endif /* _SHDMATH_H_ */
