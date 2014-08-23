/*
 * SuperQ.h 
 */

#include <lwmeshedt.h>
#include <lwxpanel.h>
#include <lwdyna.h>
#include <lwmath.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define VCPY_F(a,b)           \
   ((a)[0] = (float)((b)[0]), \
    (a)[1] = (float)((b)[1]), \
    (a)[2] = (float)((b)[2]))

typedef struct st_sqData {
   int         nsides, nsegments, shape, axis, uvs;
   LWDVector   org, rad;
   LWFVector   top[ 4 ], bot[ 4 ], center, holex, holez;
   double      diam, bf1, bf2;
   int         update, active, dirty;
} sqData;

int ellipsoid( MeshEditOp *edit, sqData *dat );
int toroid( MeshEditOp *edit, sqData *dat );

