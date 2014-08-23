/**********************************************************************
 *<
	FILE: RayMeshIntersect.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __RAYMESHINTERSECT__H
#define __RAYMESHINTERSECT__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "ISkin.h"
#include "ISkinCodes.h"
#include "icurvctl.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"

#include "IRayMeshGridIntersect.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define ScriptPrint (the_listener->edit_stream->printf)



enum { raymeshgridintersect_params };


//TODO: Add enums for various parameters
enum { 
	pb_nodelist,
};


class HitList
{
public:
	int faceIndex;
	int nodeIndex;
	float distance;
	float perpDistance;
	Point3 bary, fnorm;
	HitList()
		{
		bary = Point3(0.0f,0.0f,0.0f);
		fnorm = Point3(0.0f,0.0f,0.0f);
		faceIndex = -1;
		nodeIndex = -1;
		distance = 0.0f;

		perpDistance = 0.0f;
		}
};

class Cell
{
public:
	int nodeIndex;
	int faceIndex;
};

class CellList
{
public:
	Tab<Cell> data;
};

class NodeProperties
{
public:
	INode *node;
	Box3 bounds;
	Tab<Point3> verts;
	Tab<Face> faces;
	Tab<Point3> fnorms;
	NodeProperties ()
		{
		bounds.Init();
		node = NULL;
		}
};

class Grid
{
private:
	int width;
	Box3 bounds;
	Tab<NodeProperties*> nodeList;

	Tab<CellList*> gridX;
	Tab<CellList*> gridY;
	Tab<CellList*> gridZ;

	//adds a face to the grid list
	void AddFace(int nodeIndex, int faceIndex, Box3 faceBounds, Point3 a, Point3 b, Point3 c);

	//a quick function to compute the boudns to check through our grid
	void ComputeStartPoints(Point3 min,Point3 max,IPoint3 &pStart,IPoint3 &pEnd);

	Tab<HitList> hitList;

public:
	void FreeGrid();
	void Initialize(int size);
	void AddNode(INode *node);

	void AddMesh(Mesh *msh, Matrix3 tm);

	void BuildGrid();

//	int IntersectRay(Ray r, Cell *hitList);
	int IntersectBox(Box3 b);
	int IntersectSphere(Point3 p, int radius);


	int WalkLine(Point3 start, Point3 end, int l1, int l2, BitArray &potentialHitList,Tab<CellList*> &grid);
//	RecurseLine(IPoint3 pStart, IPoint3 pEnd, int l1, int l2, BitArray &potentialHitList);
	int	IntersectRay(Point3 p, Point3 dir, BOOL segment, BOOL doubleSided, int whichGrid = -1);

	int GetHitFace(int index);

	Point3	GetHitBary(int index); 
	Point3	GetHitNorm(int index); 
	float	GetHitDist(int index); 
	
	int		GetClosestHit();
	int		GetFarthestHit();

	void	GetCell(Point3 p, int l1, int l2, int &x, int &y);
	int		ClosestFace(Point3 p);

	float	GetPerpDist(int index);

	float	numFaces, summedFaces, tally;

	int GetFacesToProcess(int x,int y, int radius,BitArray &cellsToCheck, Tab<CellList*> &grid);

	Point3	GetHitPoint(int index); 

};



class RayMeshGridIntersect : public ReferenceTarget, public IRayMeshGridIntersect_InterfaceV1 {
	public:



	
		//From Animatable
		Class_ID ClassID() {return RAYMESHGRIDINTERSECT_CLASS_ID;}		
		SClass_ID SuperClassID() { return REF_TARGET_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}


		// Parameter block
		IParamBlock2	*pblock;	//ref 0
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i)	{ if (i == 0) return pblock;
											  else return NULL;	}
		IParamBlock2* GetParamBlockByID(BlockID id) { if (pblock->ID() == id) return pblock ;
													  else return  NULL; }

		//reference stuff
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i)	{ if (i==0)	return (RefTargetHandle)pblock;
											  return NULL;	}
		void SetReference(int i, RefTargetHandle rtarg) { if (i==0)	pblock = (IParamBlock2*)rtarg; }
		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) { return GetReference(i);}
		TSTR SubAnimName(int i)	{return _T("");	}
		int SubNumToRefNum(int subNum) {return -1;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == RAYMESHGRIDINTERSECT_V1_INTERFACE) 
				return (IRayMeshGridIntersect_InterfaceV1*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 
		

		void DeleteThis() 
			{ 
			fnFree();
			delete this; 
			}		
		//Constructor/Destructor

		RayMeshGridIntersect();
		~RayMeshGridIntersect();
		
//implementation
		void	fnFree();
		void	fnInitialize(int size);
		void	fnBuildGrid();
		void	fnAddNode(INode *node);

		int		fnIntersectBox(Point3 min, Point3 max);
		int		fnIntersectSphere(Point3 p, float radius);
		int		fnIntersectRay(Point3 p, Point3 dir, BOOL doubleSided);
		int		fnIntersectSegment(Point3 p1, Point3 p2, BOOL doubleSided);
		int		fnGetHitFace(int index); 

		Point3	fnGetHitBary(int index); 
		Point3	fnGetHitNorm(int index); 
		float	fnGetHitDist(int index); 
		int		fnGetClosestHit();
		int		fnIntersectSegmentDebug(Point3 p1, Point3 p2, BOOL doubleSided,int whichGrid);
		int		fnGetFarthestHit();

		void	fnAddMesh(Mesh *msh, Matrix3 tm);
		int		fnClosestFace(Point3 p);
		float	fnGetPerpDist(int index);

		void	fnClearStats();
		void	fnPrintStats();

		Point3	fnGetHitPoint(int index); 


	private:
		Grid gridData;
		

};


class RayMeshGridIntersectClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);// { return new RayMeshGridIntersect(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return RAYMESHGRIDINTERSECT_CLASS_ID; }
	// The Skin modifier checks the category to decide whether the modifier is a Skin Gizmo.  This 
	// must not be changed
	const TCHAR* 	Category() { return GetString(IDS_PW_GIZMOCATEGORY); }

	const TCHAR*	InternalName() { return _T("RayMeshGridIntersect"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



#endif // __RAYMESHINTERSECT__H
