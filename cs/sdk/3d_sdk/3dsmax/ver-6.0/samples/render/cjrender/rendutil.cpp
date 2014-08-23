//***************************************************************************
// CJRender - [rendutil.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Utilitiy functions
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "rendutil.h"

//***************************************************************************
// Since Matrix3::NoScale() was not implemented in Max SDK 1.0,
// here's a manual implementation
//***************************************************************************
void RemoveScaling(Matrix3 &m)
{
	for (int i=0; i<3; i++) 
		m.SetRow(i,Normalize(m.GetRow(i)));
}

//***************************************************************************
// Helper function to see if the face is 'facing' our way.
//***************************************************************************
int isFacing(Point3& p0, Point3& p1, Point3& p2 , int projType) {
	if (projType==PROJ_PERSPECTIVE) {
		FLOAT t0,t1,t2,d;
		t0 = p1.y*p2.z - p1.z*p2.y;
		t1 = p2.y*p0.z - p2.z*p0.y;
		t2 = p0.y*p1.z - p0.z*p1.y;
		d = -(p0.x*t0 + p1.x*t1 + p2.x*t2);
		return((d<0.0)?0:1);
	}
	else {
		float d = (p1.x-p0.x)*(p2.y-p0.y) - (p2.x-p0.x)*(p1.y-p0.y);
		return((d<0.0)?0:1);
	}
}

//***************************************************************************
// Calculate the determinant
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

static float det2x2( float a, float b, float c, float d )
{
    float ans;
    ans = a * d - b * c;
    return ans;
}

//***************************************************************************
// 
// float = det3x3(  a1, a2, a3, b1, b2, b3, c1, c2, c3 )
//   
// calculate the determinant of a 3x3 matrix
// in the form
//
//     | a1,  b1,  c1 |
//     | a2,  b2,  c2 |
//     | a3,  b3,  c3 |
//
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

static float det3x3( Point3 a,Point3 b,Point3 c )

{
   float a1, a2, a3, b1, b2, b3, c1, c2, c3;    
   float ans;
   
   a1 = a.x ; a2 = a.y ; a3 = a.z ;   
   b1 = b.x ; b2 = b.y ; b3 = b.z ;   
   c1 = c.x ; c2 = c.y ; c3 = c.z ;   

   ans = a1 * det2x2( b2, b3, c2, c3 )
       - b1 * det2x2( a2, a3, c2, c3 )
       + c1 * det2x2( a2, a3, b2, b3 );
   return ans;
}


//***************************************************************************
// Given three points in space forming a triangle (p0,p1,p2), 
// and a fourth point in the plane of that triangle, returns the
// barycentric coords of that point relative to the triangle.
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

Point3 CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p)
{ 
	Point3 tpos[3];
	Point3 cpos;
	Point3 bary;

	tpos[0] = p0;
	tpos[1] = p1;
	tpos[2] = p2;
	cpos = p;

 /*
 
 S.304 Curves+Surfaces for Computer aided design:

     u + v + w = 1

         area(p,b,c)    area(a,p,c)   area(a,b,p)
 u = -----------, v = -----------, w = -----------, 
    area(a,b,c)    area(a,b,c)   area(a,b,c)
 
   ax  bx  cx
   
 area(a,b,c) = 0.5 * ay  by  cy
   
   az  bz  cz
    
   
 */

	float area_abc, area_pbc, area_apc, area_abp; 
	
	area_abc= det3x3(tpos[0],tpos[1],tpos[2]);
	area_abc=1.0f/area_abc;
	
	area_pbc =det3x3(cpos   ,tpos[1],tpos[2]); 
	area_apc =det3x3(tpos[0],cpos   ,tpos[2]);
	area_abp =det3x3(tpos[0],tpos[1],cpos   );
	
	bary.x = area_pbc *area_abc ;
	bary.y = area_apc *area_abc ;
	bary.z = area_abp *area_abc ;
	
	return bary;
}


//***************************************************************************
// This is cut directly from the SDK documentation.
// This is how UV coords are generated for face mapped materials
//***************************************************************************

static Point3 basic_tva[3] = { 
	Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)
};
static Point3 basic_tvb[3] = { 
	Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)
};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void MakeFaceUV(Face *f, Point3 *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0;
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb; 
	for (i=0; i<3; i++) {  
		tv[i] = basetv[na];
		na = nextpt[na];
	}
}


//***************************************************************************
// Utility function to convert a Color to a BMM_Color_64 structure
//***************************************************************************

BMM_Color_64 colTo64(Color c)
{
	BMM_Color_64 bc;

	// Clamp the colors
	c.ClampMinMax();

	bc.r = (WORD)(c.r * 65535.0);
	bc.g = (WORD)(c.g * 65535.0);
	bc.b = (WORD)(c.b * 65535.0);

	return bc;
}

//***************************************************************************
// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale
// so when calculating the normal we take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'
//***************************************************************************

BOOL TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}
