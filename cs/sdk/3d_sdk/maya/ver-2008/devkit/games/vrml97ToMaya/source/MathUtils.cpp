// 

#include "MathUtils.h"
#include <string.h>		// memcpy


double Vlength( float V[3] )
{
  double vlen = sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
  return (FPZERO(vlen) ? 0.0 : vlen);
}

void Vdiff( float V[3], float A[3], float B[3] )
{
  V[0] = A[0] - B[0];
  V[1] = A[1] - B[1];
  V[2] = A[2] - B[2];
}

void Vcross( float V[3], float A[3], float B[3] )
{
  float x,y,z;			// Use temps so V can be A or B
  x = A[1]*B[2] - A[2]*B[1];
  y = A[2]*B[0] - A[0]*B[2];
  z = A[0]*B[1] - A[1]*B[0];
  V[0] = x;
  V[1] = y;
  V[2] = z;
}

void Vnorm( float V[3] )
{
  float vlen = (float) sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
  if (! FPZERO(vlen))
    {
      V[0] /= vlen;
      V[1] /= vlen;
      V[2] /= vlen;
    }
}


// Note that these matrices are stored in natural (C) order (the transpose
// of the OpenGL matrix). Could change this someday...

void Midentity( double M[4][4] )
{
  for (int i=0; i<4; ++i)
    for (int j=0; j<4; ++j)
      M[i][j] = (i == j) ? 1.0 : 0.0;
}

// Convert from axis/angle to transformation matrix GG p466

void Mrotation( double M[4][4], float axisAngle[4] )
{
  Vnorm( axisAngle );
  double s = sin(axisAngle[3]);
  double c = cos(axisAngle[3]);
  double t = 1.0 - c;
  double x = axisAngle[0];
  double y = axisAngle[1];
  double z = axisAngle[2];

  M[0][0] = t*x*x + c;
  M[0][1] = t*x*y - s*z;
  M[0][2] = t*x*z + s*y;
  M[0][3] = 0.0;
  M[1][0] = t*x*y + s*z;
  M[1][1] = t*y*y + c;
  M[1][2] = t*y*z - s*x;
  M[1][3] = 0.0;
  M[2][0] = t*x*z - s*y;
  M[2][1] = t*y*z + s*x;
  M[2][2] = t*z*z + c;
  M[2][3] = 0.0;
  M[3][0] = M[3][1] = M[3][2] = 0.0;
  M[3][3] = 1.0;
}

void Mscale( double M[4][4], float scale[3] )
{
  Midentity(M);
  for (int i=0; i<3; ++i)
    M[i][i] = scale[i];
}

void MM( double M[4][4], double N[4][4] )
{
  double m[4][4];

  memcpy(m, M, sizeof(m));
  for (int i=0; i<4; ++i)
    for (int j=0; j<4; ++j)
      M[i][j] = m[i][0]*N[0][j] + m[i][1]*N[1][j] +
	m[i][2]*N[2][j] + m[i][3]*N[3][j];
}


void VM( float V[3], double M[4][4], float A[3] )
{
  float v[3] = { A[0], A[1], A[2] }; // Allow for V/A aliasing
  for (int i=0; i<3; ++i)
    V[i] = (float)(M[i][0] * v[0] + M[i][1] * v[1] + M[i][2] * v[2] + M[i][3]);
}
