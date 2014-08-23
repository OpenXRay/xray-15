/**********************************************************************
 *<
	FILE: SolidifyPW.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __SOLIDIFYPW__H
#define __SOLIDIFYPW__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "meshadj.h"
#include "XTCObject.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


class ShapeData
{
public:
	float uPer;
	float vPer;
};

class BevelData
{
public:
	int ct;
	Point3 vec[2];
	Point3 cross[2];
	Point3 norm;
	Point3 vnorm;
	Point3 p;

	float realLength;

};

class EdgeInfo
{
public:
	float angle;
	Point3 normal;
};

class PlaneInfo
{
public:
	Point3 vec, p;
		
	Point3 v[3],center;

	float x,y,z,d;
	void BuildPlaneEquation()
	{
		vec = Normalize(vec);
		x = vec.x;
		y = vec.y;
		z = vec.z;

		d = DotProd(p,vec);
	};

};

class VertexPlanes
{
public:

	BOOL display;
	Point3 debugP1,debugP2;
	Point3 debugBase, debugNorm;


	int ct;
	PlaneInfo planes[3];

	Point3 vnorm;
	Point3 op;
	Point3 vec;
	float dist;

	BOOL isEdgeVert;


	VertexPlanes()
	{
		ct = 0;
		isEdgeVert = FALSE;
	};

	Point3 IntersectPlaneLine(Point3 norm, float w, Point3 p1, Point3  p2)
	{
		float td = w * -1.0f;

			 /* Calculate the position on the line that intersects the plane */
		float denom = norm.x * (p2.x - p1.x) + norm.y * (p2.y - p1.y) + norm.z * (p2.z - p1.z);

		if (fabs(denom) < 0.0001f)         /* Line and plane don't intersect */
		{
			ct = 0;
			return Point3(0.0f,0.0f,0.0f);
		}

		float mu = - (td + norm.x * p1.x + norm.y * p1.y + norm.z * p1.z) / denom;
		Point3 tp;

		tp.x = p1.x + mu * (p2.x - p1.x);
		tp.y = p1.y + mu * (p2.y - p1.y);
		tp.z = p1.z + mu * (p2.z - p1.z);

//		Point3 vecIntersect = (tp - this->op);
//		vecIntersect = Normalize(vecIntersect);

		return tp;
	}

	

	void AddPlane(Point3 p, Point3 vec, Point3 a, Point3 b, Point3 c)
	{
		if (ct == 3) return;
		BOOL add = FALSE;
		for (int i = 0; i < ct; i++)
		{
			float dot = DotProd(planes[i].vec ,vec);
			if (dot == 1.0f) return;

			if (dot != 0.0f)
				{
				float angle = acos(dot);
				
				angle = angle * 180.0f/PI;
				if (angle < 25.0f) return;
				}
//			float dist = Length(planes[i].vec - vec);
//			if ( dist < 0.1f) 
//				return;

		}
		planes[ct].p = p;
		planes[ct].vec = vec;

		planes[ct].v[0] = a;
		planes[ct].v[1] = b;
		planes[ct].v[2] = c;

		planes[ct].center = (a+b+c)/3.0f;

		ct++;

	};

	void BuildPlaneEquations()
	{
		if (isEdgeVert)
			{
			if (ct < 2) 
				{
				isEdgeVert = FALSE;
				return;
				}
			display = TRUE;


//intersect them that is our vect
			planes[0].BuildPlaneEquation();
			planes[1].BuildPlaneEquation();

			Point3 p1,p2;
			p1 = planes[1].v[0] + planes[1].vec;
			p2 = planes[1].v[1] + planes[1].vec;
			debugP1 = p1;
			debugP2 = p2;
			debugBase = op;
			debugNorm = planes[0].vec;


/*			Point3 pPlaneIntersect = IntersectPlaneLine(planes[0].vec, planes[0].d, p1,p2);
*/
			vnorm = (planes[0].vec + planes[1].vec)*0.5f;
			vnorm = Normalize(vnorm);



			return;
			}

		if (ct != 3) return;

		for (int i = 0; i < ct; i++)
		{
			planes[i].BuildPlaneEquation();
		}

		PlaneInfo pi1,pi2,pi3;
		pi1 = planes[0];
		pi2 = planes[1];
		pi3 = planes[2];

				//intersect the 2 planes to get the vec
		Point3 pl1,pl2;
		Point3 lp,lvec;

//		pl1.w = planes[0].d * -1.0f;
		pl1.x = planes[0].x;
		pl1.y = planes[0].y;
		pl1.z = planes[0].z;

//		pl2.w = planes[1].d * -1.0f;
		pl2.x = planes[1].x;
		pl2.y = planes[1].y;
		pl2.z = planes[1].z;

		//Get our line that intersects the 2 plane
		Point3 vecPlaneIntersect;
		vecPlaneIntersect = CrossProd(planes[0].vec,planes[1].vec);
		//Create a new line on plane 0
		Point3 point1OnPlane0,point2OnPlane0;
		point1OnPlane0 = planes[0].v[0] + planes[0].vec;
		point2OnPlane0 = planes[0].center + planes[0].vec;

		//intersect it on plane 1
		//that is our point and line
		Point3 pPlaneIntersect = IntersectPlaneLine(planes[1].vec, planes[1].d, point1OnPlane0,point2OnPlane0);

		Point3 p1,p2;
		p1 = pPlaneIntersect;
		p2 = pPlaneIntersect + vecPlaneIntersect;

		//intersect that line with plane plane 2
		Point3 finalP = IntersectPlaneLine(planes[2].vec, planes[2].d, p1,p2);

		vnorm = finalP - op;
		vnorm = Normalize(vnorm);


	};



	void GetDirection(Point3 &vec, float &d)
	{
		if (ct < 2) 
			{
			vec = vnorm;
			d = this->dist;
			}
		else
			{
			vec = this->vec;
			d = this->dist;
			}
	}


};

class WeightedNormals
{
public:
	float a,b,c;
};

class CornerData
{
public:
	Point3 p,norm,hyp;
};

class SolidifyEdge
{
public:
	int a,b;

	int aIndexToEList;
	int bIndexToEList;

	BOOL flip;
	int ownerFace;
	int ithVertA,ithVertB;
	int tvFaceA,tvFaceB;
	float length;
	
};


class AvData
{
public:
	int a,b;
	float per;
};

class AverageDirection
{
public:
	int owner;
	Tab<AvData> a;

};

class EdgeLoop
{
public:
	float totalLength;
	Tab<int> loop;
};


class BuildMeshInfo
{
public:

		void BevelBySpline(Mesh *msh,PolyLine *shape,int eMatID=-1, int oMatID=-1,
					  int sg=-1,
					  BOOL autoSmooth = TRUE, float autoSmoothAngle = 45.0f,
					  int map=0,
					  BOOL selEgdes=FALSE, BOOL selTop=TRUE);
					
		void MakeSolid(Mesh *msh, int segments, float amount, float oamount,int matid = -1, 
							 int sg = -1, int map = 0, float tvOffset = 0.05f,
							 int innerMatID = -1,int outerMatID = -1,
							 BOOL selEdges = FALSE, BOOL selFaces = FALSE,BOOL selOuterFaces = FALSE,
							 BOOL fixupEdges = TRUE,
							 BOOL autoSmooth = TRUE, float autoSmoothAngle = 45.0f,
							 PolyLine *shape = NULL
							 );


		void BuildData(Mesh *mesh,PolyLine *shape, BOOL fixupEdges);

		Point3 GetOutlineDir(Point3 a, Point3 b);
		void BuildShapeData(PolyLine *shape);


		Tab<CornerData> cdata;

		Tab<VertexPlanes> vPlanes;

		Tab<ShapeData> shapeData;
		Tab<BevelData> bevelData;
		Tab<Point3> vnorms;
		//build normals
		Tab<Point3> fnorms;

		Tab<float> realLengths;
		Tab <SolidifyEdge> sEdges;

		Tab<Point3> op;

		void Free()
		{
			cdata.ZeroCount();

			vPlanes.ZeroCount();

			shapeData.ZeroCount();
			
			bevelData.ZeroCount();
			
			vnorms.ZeroCount();
			//build normals
			Tab<Point3> fnorms;

			realLengths.ZeroCount();
			sEdges.ZeroCount();
		}

};



class SolidifyPW;

class SolidifyPWDlgProc : public ParamMap2UserDlgProc {
	private:
		SolidifyPW *mod;
	public:
		SolidifyPWDlgProc(SolidifyPW *s) { mod = s; }
		 BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
	};


#endif // __SOLIDIFYPW__H
