//
// Some useful math stuff that should someday be made into C++...
//
#include <math.h>


// Define single precision floating point "zero".

#define FPTOLERANCE 1.0e-7
#define FPZERO(n) (fabs(double(n))<=FPTOLERANCE)
#define FPEQUAL(a,b) FPZERO((a)-(b))

// Vector ops

#define Vset( V, A ) \
 ((V[0]=A[0]),(V[1]=A[1]),(V[2]=A[2]))             // V <= A
#define Vscale( V, s) \
 ((V[0]*=(s)),(V[1]*=(s)),(V[2]*=s)) 		   // V *= s
#define Vdot( A, B ) \
 (A[0]*B[0]+A[1]*B[1]+A[2]*B[2])		   // A.B

double Vlength( float V[3] );	                   // |V|

void Vdiff( float V[3], float A[3], float B[3] );  // V <= A - B
void Vcross( float V[3], float A[3], float B[3] ); // V <= A x B
void Vnorm( float V[3] );	                   // V <= V / |V|

void VM(float V[3], double M[4][4], float A[3]);   // V <= MA

// Matrix ops

void Midentity(double M[4][4]);	                   // M <= I
void Mrotation(double M[4][4], float r[4]);        // M <= rotation about axis/angle
void Mscale(double M[4][4], float s[3]);           // M <= scale by s

void MM(double M[4][4], double N[4][4]);           // M <= MN

