// Matrix Inversion
// Arnie Cachelin, Copyright 2001 NewTek, Inc.

#include <math.h>
#include <lwsdk/lwtypes.h>
#include <lwsdk/lwmath.h>

#define ND		 3

typedef double		 Vector[ND];
typedef float		FVector[ND];
typedef short		SVector[ND];
typedef double		 Matrix[ND][ND];

void MatrixInit( Matrix m );
int MatrixInvert ( Matrix mInv, const Matrix m );
void MatrixApply( LWDVector v, Matrix mat, LWDVector src );

