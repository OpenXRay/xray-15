//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  Viewer.cpp
//  Abstract base class for display of VRML models
//

#include "config.h"
#include "Viewer.h"


#include "MathUtils.h"
#include "VrmlScene.h"



//  Empty destructor for derived classes to call.

Viewer::~Viewer() {}

//  Build a cylinder object. It might be smarter to do just one, and reference
//  it with scaling (but the world creator could just as easily do that with 
//  DEF/USE ...).

void Viewer::computeCylinder(double height,
			     double radius,
			     int numFacets,
			     float c[][3],
			     float tc[][3],
			     int faces[])
{
  double angle, x, y;
  int i, polyIndex;

  // Compute coordinates, texture coordinates:
  for (i = 0; i < numFacets; ++i) {
    angle = i * 2 * M_PI / numFacets;
    x = cos(angle);
    y = sin(angle);
    c[i][0] = (float)(radius * x);
    c[i][1] = (float)(0.5 * height);
    c[i][2] = (float)(radius * y);
    c[numFacets+i][0] = (float)(radius * x);
    c[numFacets+i][1] = (float)(-0.5 * height);
    c[numFacets+i][2] = (float)(radius * y);

    if (tc)
      {
	tc[i][0] = ((float) i) / numFacets;
	tc[i][1] = 0.0;
	tc[i][2] = 0.0;
	tc[numFacets+i][0] = ((float) i) / numFacets;
	tc[numFacets+i][1] = 1.0;
	tc[numFacets+i][2] = 0.0;
      }
  }

  // And compute indices:
  for (i = 0; i < numFacets; ++i) {
    polyIndex = 5*i;
    faces[polyIndex + 0] = i;
    faces[polyIndex + 1] = (i+1) % numFacets;
    faces[polyIndex + 2] = (i+1) % numFacets + numFacets;
    faces[polyIndex + 3] = i + numFacets;
    faces[polyIndex + 4] = -1;
  }
}

//  Build an extrusion.

void Viewer::computeExtrusion(int nOrientation,
			      float *orientation,
			      int nScale,
			      float *scale,
			      int nCrossSection,
			      float *crossSection,
			      int nSpine,
			      float *spine,
			      float *c,   // OUT: coordinates
			      float *tc,  // OUT: texture coords
			      int *faces)     // OUT: face list
{
  int i, j, ci;

  // Xscp, Yscp, Zscp- columns of xform matrix to align cross section
  // with spine segments.
  float Xscp[3] = { 1.0, 0.0, 0.0};
  float Yscp[3] = { 0.0, 1.0, 0.0};
  float Zscp[3] = { 0.0, 0.0, 1.0};
  float lastZ[3];

  // Is the spine a closed curve (last pt == first pt)?
  bool spineClosed = (FPZERO(spine[ 3*(nSpine-1)+0 ] - spine[0]) &&
		      FPZERO(spine[ 3*(nSpine-1)+1 ] - spine[1]) &&
		      FPZERO(spine[ 3*(nSpine-1)+2 ] - spine[2]));
  
  // Is the spine a straight line?
  bool spineStraight = true;
  for (i = 1; i < nSpine-1; ++i)
    {
      float v1[3], v2[3];
      v1[0] = spine[3*(i-1)+0] - spine[3*(i)+0];
      v1[1] = spine[3*(i-1)+1] - spine[3*(i)+1];
      v1[2] = spine[3*(i-1)+2] - spine[3*(i)+2];
      v2[0] = spine[3*(i+1)+0] - spine[3*(i)+0];
      v2[1] = spine[3*(i+1)+1] - spine[3*(i)+1];
      v2[2] = spine[3*(i+1)+2] - spine[3*(i)+2];
      Vcross(v1, v2, v1);
      if (Vlength(v1) != 0.0)
	{
	  spineStraight = false;
	  Vnorm( v1 );
	  Vset( lastZ, v1 );
	  break;
	}
    }

  // If the spine is a straight line, compute a constant SCP xform
  if (spineStraight)
    {
      float V1[3] = { 0.0, 1.0, 0.0}, V2[3], V3[3];
      V2[0] = spine[3*(nSpine-1)+0] - spine[0];
      V2[1] = spine[3*(nSpine-1)+1] - spine[1];
      V2[2] = spine[3*(nSpine-1)+2] - spine[2];
      Vcross( V3, V2, V1 );
      float len = (float)Vlength(V3);
      if (len != 0.0)		// Not aligned with Y axis
	{
	  Vscale(V3, 1.0f/len);

	  float orient[4];		// Axis/angle
	  Vset(orient, V3);
	  orient[3] = acos(Vdot(V1,V2));
	  double scp[4][4];	        // xform matrix
	  Mrotation( scp, orient );
	  for (int k=0; k<3; ++k) {
	    Xscp[k] = (float)scp[0][k];
	    Yscp[k] = (float)scp[1][k];
	    Zscp[k] = (float)scp[2][k];
	  }
	}
    }

  // Orientation matrix
  double om[4][4];
  if (nOrientation == 1 && ! FPZERO(orientation[3]) )
    Mrotation( om, orientation );

  // Compute coordinates, texture coordinates:
  for (i = 0, ci = 0; i < nSpine; ++i, ci+=nCrossSection) {

    // Scale cross section
    for (j = 0; j < nCrossSection; ++j) {
      c[3*(ci+j)+0] = scale[0] * crossSection[ 2*j ];
      c[3*(ci+j)+1] = 0.0;
      c[3*(ci+j)+2] = scale[1] * crossSection[ 2*j+1 ];
    }

    // Compute Spine-aligned Cross-section Plane (SCP)
    if (! spineStraight)
      {
	float S1[3], S2[3];	// Spine vectors [i,i-1] and [i,i+1]
	int yi1, yi2, si1, s1i2, s2i2;

	if (spineClosed && (i == 0 || i == nSpine-1))
	  {
	    yi1 = 3*(nSpine-2);
	    yi2 = 3;
	    si1 = 0;
	    s1i2 = 3*(nSpine-2);
	    s2i2 = 3;
	  }
	else if (i == 0)
	  {
	    yi1 = 0;
	    yi2 = 3;
	    si1 = 3;
	    s1i2 = 0;
	    s2i2 = 6;
	  }
	else if (i == nSpine-1)
	  {
	    yi1 = 3*(nSpine-2);
	    yi2 = 3*(nSpine-1);
	    si1 = 3*(nSpine-2);
	    s1i2 = 3*(nSpine-3);
	    s2i2 = 3*(nSpine-1);
	  }
	else
	  {
	    yi1 = 3*(i-1);
	    yi2 = 3*(i+1);
	    si1 = 3*i;
	    s1i2 = 3*(i-1);
	    s2i2 = 3*(i+1);
	  }

	Vdiff( Yscp, &spine[yi2], &spine[yi1] );
	Vdiff( S1, &spine[s1i2], &spine[si1] );
	Vdiff( S2, &spine[s2i2], &spine[si1] );

	Vnorm( Yscp );
	Vset(lastZ, Zscp);	// Save last Zscp
	Vcross( Zscp, S2, S1 );

	float VlenZ = (float)Vlength(Zscp);
	if ( VlenZ == 0.0 )
	  Vset(Zscp, lastZ);
	else
	  Vscale( Zscp, 1.0f/VlenZ );

	if ((i > 0) && (Vdot( Zscp, lastZ ) < 0.0))
	  Vscale( Zscp, -1.0 );

	Vcross( Xscp, Yscp, Zscp );
      }

    // Rotate cross section into SCP
    for (j = 0; j < nCrossSection; ++j) {
      float cx, cy, cz;
      cx = c[3*(ci+j)+0]*Xscp[0]+c[3*(ci+j)+1]*Yscp[0]+c[3*(ci+j)+2]*Zscp[0];
      cy = c[3*(ci+j)+0]*Xscp[1]+c[3*(ci+j)+1]*Yscp[1]+c[3*(ci+j)+2]*Zscp[1];
      cz = c[3*(ci+j)+0]*Xscp[2]+c[3*(ci+j)+1]*Yscp[2]+c[3*(ci+j)+2]*Zscp[2];
      c[3*(ci+j)+0] = cx;
      c[3*(ci+j)+1] = cy;
      c[3*(ci+j)+2] = cz;
    }

    // Apply orientation
    if (! FPZERO(orientation[3]) )
      {
	if (nOrientation > 1)
	  Mrotation( om, orientation );

	for (j = 0; j < nCrossSection; ++j) {
	  float cx, cy, cz;
	  cx = (float)(c[3*(ci+j)+0]*om[0][0]+c[3*(ci+j)+1]*om[1][0]+c[3*(ci+j)+2]*om[2][0]);
	  cy = (float)(c[3*(ci+j)+0]*om[0][1]+c[3*(ci+j)+1]*om[1][1]+c[3*(ci+j)+2]*om[2][1]);
	  cz = (float)(c[3*(ci+j)+0]*om[0][2]+c[3*(ci+j)+1]*om[1][2]+c[3*(ci+j)+2]*om[2][2]);
	  c[3*(ci+j)+0] = cx;
	  c[3*(ci+j)+1] = cy;
	  c[3*(ci+j)+2] = cz;
	}
      }

    // Translate cross section
    for (j = 0; j < nCrossSection; ++j) {
      c[3*(ci+j)+0] += spine[3*i+0];
      c[3*(ci+j)+1] += spine[3*i+1];
      c[3*(ci+j)+2] += spine[3*i+2];

      // Texture coords
      tc[3*(ci+j)+0] = ((float) j) / (nCrossSection-1);
      tc[3*(ci+j)+1] = 1.0f - ((float) i) / (nSpine-1);
      tc[3*(ci+j)+2] = 0.0f;
    }

    if (nScale > 1) scale += 2;
    if (nOrientation > 1) orientation += 4;
  }

  // And compute face indices:
  if (faces)
    {
      int polyIndex = 0;
      for (i = 0, ci = 0; i < nSpine-1; ++i, ci+=nCrossSection) {
	for (j = 0; j < nCrossSection-1; ++j) {
	  faces[polyIndex + 0] = ci+j;
	  faces[polyIndex + 1] = ci+j+1;
	  faces[polyIndex + 2] = ci+j+1 + nCrossSection;
	  faces[polyIndex + 3] = ci+j + nCrossSection;
	  faces[polyIndex + 4] = -1;
	  polyIndex += 5;
	}
      }
    }
}


void Viewer::computeSphere(double radius,
			   int numLatLong,
			   float c[][3],
			   float tc[][3],
			   int *faces)
{
  double r, angle, x, y, z;
  int i, j, polyIndex;

  // Compute coordinates, texture coordinates:
  for (i = 0; i < numLatLong; ++i) {
    /*y = 2.0 * ( ((double)i) / (numLatLong-1) ) - 1.0;*/
    angle = ( i * M_PI / (numLatLong-1) ) - M_PI_2;
    y = sin( angle );
    r = sqrt( 1.0 - y*y );
    for (j = 0; j < numLatLong; ++j) {
      angle = 2 * M_PI * ((double)j) / numLatLong;
      x = - sin(angle)*r;
      z = - cos(angle)*r;
      c[i*numLatLong+j][0] = (float)(radius * x);
      c[i*numLatLong+j][1] = (float)(radius * y);
      c[i*numLatLong+j][2] = (float)(radius * z);
      if (tc)
	{
	  tc[i*numLatLong+j][0] = ((float) j)/numLatLong;
	  tc[i*numLatLong+j][1] = ((float) i)/(numLatLong-1);
	  tc[i*numLatLong+j][2] = 0.0;
	}
    }
  }

  // And compute indices:
  if (faces)
    for (i = 0; i < numLatLong-1; ++i) {
      for (j = 0; j < numLatLong; ++j) {
	polyIndex = 5*(i*numLatLong+j);
	faces[polyIndex + 0] = i*numLatLong+j;
	faces[polyIndex + 1] = i*numLatLong+(j+1)%numLatLong;
	faces[polyIndex + 2] = (i+1)*numLatLong+(j+1)%numLatLong;
	faces[polyIndex + 3] = (i+1)*numLatLong+j;
	faces[polyIndex + 4] = -1;  // quad
      }
    }
}


//
// Compute a target and up vector from position/orientation/distance.
//

void Viewer::computeView(float position[3],
			 float orientation[3],
			 float distance,
			 float target[3],
			 float up[3])
{
  // Graphics Gems, p 466. Convert between axis/angle and rotation matrix
  float len = (float)sqrt( orientation[0]*orientation[0] +
		     orientation[1]*orientation[1] +
		     orientation[2]*orientation[2] );
  if (len > 0.0f)
    {
      orientation[0] /= len;
      orientation[1] /= len;
      orientation[2] /= len;
    }

  float s = (float)sin(orientation[3]);
  float c = (float)cos(orientation[3]);
  float t = 1.0f - c;

  // Transform [0,0,1] by the orientation to determine sight line
  target[0] = t * orientation[0] * orientation[2] + s * orientation[1];
  target[1] = t * orientation[1] * orientation[2] - s * orientation[0];
  target[2] = t * orientation[2] * orientation[2] + c;

  // Move along that vector the specified distance away from position[]
  target[0] = -distance*target[0] + position[0];
  target[1] = -distance*target[1] + position[1];
  target[2] = -distance*target[2] + position[2];

  // Transform [0,1,0] by the orientation to determine up vector
  up[0] = t * orientation[0] * orientation[1] - s * orientation[2];
  up[1] = t * orientation[1] * orientation[1] + c;
  up[2] = t * orientation[1] * orientation[2] + s * orientation[0];
}


void Viewer::setColor(float , float , float , float ) {}

