/**********************************************************************
 *<
	FILE: sctex.h

	DESCRIPTION: A ShadeContext for rendering texture maps

	CREATED BY: Rolf Berteig (took code from mtlrend.cpp

	HISTORY: 2/02/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#ifndef __SCTEXT_H__
#define __SCTEXT_H__

class LightDescImp: public LightDesc {
	public:
	Point3 pos;
	Color col;
    BOOL Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3 &dir, float &dot_nl) {
		dir = Normalize(pos-sc.P());
		dot_nl = DotProd(normal,dir);
		color = col;
		return 1;		
		}
	};
class SCTex: public ShadeContext {
	public:
	float tiling;
	float scale;
	Color ambientLight;
	LightDescImp* lights[2];
	Point3 uvw,duvw,norm,view,pt,dpt;
	IPoint2 scrPos;
	TimeValue curTime;
	Renderer *GetRenderer() { return NULL; }
	BOOL 	  InMtlEditor() { return TRUE; }
	LightDesc* Light(int n) { return lights[n]; }
	int ProjType() { return 1;} // returns: 0: perspective, 1: parallel
	int FaceNumber() { return 0; }
	TimeValue CurTime() { return curTime; }
	Point3 Normal() { return norm; }  	// interpolated normal
	void SetNormal(Point3 p) { norm = p;} 	// for perturbing normal
	Point3 GNormal() { return norm;} 	// geometric (face) normal
	Point3 ReflectVector() { return Point3(0,0,1); }
	Point3 RefractVector(float ior) {return Point3(0,0,1);	}
    Point3 CamPos() { return Point3(0,0,0); }			// camera position
	Point3 V() { return view; }       	// Unit view vector: from camera towards P 
	void SetView(Point3 v) { view =v; }
	Point3 P();			// point to be shaded in camera space;
	Point3 DP();   		// deriv of P, relative to pixel, for AA
	Point3 PObj();					  	// point in obj coords
	Point3 DPObj();   	// deriv of PObj, rel to pixel, for AA
	Box3 ObjectBox(); 	 			 	// Object extents box in obj coords
	Point3 PObjRelBox();   				// Point rel to obj box [-1 .. +1 ] 
	Point3 DPObjRelBox();  				// Point rel to obj box [-1 .. +1 ] 
	Point3 UVW(int chan) { return uvw;	};
   	Point3 DUVW(int chan) {	return duvw;	}
	void DPdUVW(Point3 dP[3],int chan); 			// Bump vectors for UVW: in Camera space
	AColor EvalEnvironMap(Texmap *map, Point3 viewd) {
		AColor rcol;
        rcol.Black();
		return rcol;
		}

	void ScreenUV(Point2& uv, Point2 &duv); // screen coordinate
	IPoint2 SCTex::ScreenCoord() {return scrPos;}

	Point3 PointTo(const Point3& p, RefFrame ito) { return p; }
	Point3 PointFrom(const Point3& p, RefFrame ifrom) { return p; } 
	Point3 VectorTo(const Point3& p, RefFrame ito) { return p; } 
	Point3 VectorFrom(const Point3& p, RefFrame ifrom){ return p; } 
	void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) {	}
	SCTex();
	void SetTiling(float t) { tiling = t; }
	};

// Allocates and renders a 3 byte per pixel image
UBYTE *RenderTexMap(Texmap *tex,int w, int h);

class Grid
{

public:
//	bool hit;
	Tab<int> index;
};

class UniformGrid
{
public:
	UniformGrid();
	~UniformGrid();
	void InitializeGrid(int size);
	void FreeGrid();
	void LoadPoints(Point3 *p, int count);
	void ClosestPoint(Point3 p, float radius, int &i, float &d);
	void ClosestPoints(Point3 *p, float *radius,  int *i, float *d, int count);

	int FindCell(float x, float y, int whichGrid, int &ix, int &iy);

	void DrawBounds(GraphicsWindow *gw, Box3 bounds);
	void Display(GraphicsWindow *gw);

	BOOL debug;
	int debugI;
	Point3 debugP;


private:

	void TagCells(Point3 p,float radius, int whichGrid);

	Box3 bounds;
	float fXWidth,fYWidth,fZWidth;


	int width;
	int widthXwidth;
	Tab<Point3> pointBase;

	Tab<Grid*> xGrid;
	Tab<Grid*> yGrid;
	Tab<Grid*> zGrid;

	Tab<int> hitList;
	BitArray xHitList, yHitList, zHitList;


};


#endif //__SCTEXT_H__

