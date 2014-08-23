/*
======================================================================
vecmat.h

Basic vector and matrix functions.
====================================================================== */

#include <lwtypes.h>

#define vecangle(a,b) (float)acos(dot(a,b))    /* a and b must be unit vectors */

float dot( LWFVector a, LWFVector b );
void cross( LWFVector a, LWFVector b, LWFVector c );
void normalize( LWFVector v );
