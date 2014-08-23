

#ifndef __IRAYMESHGRIDINTERSECT__H
#define __IRAYMESHGRIDINTERSECT__H

#include "iFnPub.h"

#define RAYMESHGRIDINTERSECT_CLASS_ID	Class_ID(0x3770ad28, 0x271d1308)
#define RAYMESHGRIDINTERSECT_V1_INTERFACE Interface_ID(0xDE17A11A, 0x8A41F2F2)

#define PBLOCK_REF	0


enum {   grid_free, grid_initialize, grid_buildgrid, grid_addnode, 
		 grid_intersectbox, grid_intersectsphere, grid_intersectray,
		 grid_intersectsegment,
	 	 grid_gethitface,grid_gethitbary,grid_gethitnorm,grid_gethitdist,
		 grid_getclosesthit,
		 grid_intersectsegmentdebug,
		 grid_getfarthesthit,
		 grid_closestface ,
		 grid_getperpdist,
		 grid_clearstats, grid_printstats,
		 grid_gethitpoint
	 };

class IRayMeshGridIntersect_InterfaceV1: public FPMixinInterface 
{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

			VFN_0(grid_free, fnFree);
			VFN_1(grid_initialize, fnInitialize,TYPE_INT);
			VFN_0(grid_buildgrid, fnBuildGrid);
			VFN_1(grid_addnode, fnAddNode, TYPE_INODE);
			FN_2(grid_intersectbox, TYPE_INT, fnIntersectBox, TYPE_POINT3, TYPE_POINT3);
			FN_2(grid_intersectsphere, TYPE_INT, fnIntersectSphere, TYPE_POINT3, TYPE_FLOAT);
			FN_3(grid_intersectray, TYPE_INT, fnIntersectRay, TYPE_POINT3, TYPE_POINT3,TYPE_BOOL);
			FN_3(grid_intersectsegment, TYPE_INT, fnIntersectSegment, TYPE_POINT3, TYPE_POINT3,TYPE_BOOL);
			FN_1(grid_gethitface, TYPE_INT, fnGetHitFace, TYPE_INT);
			FN_1(grid_gethitbary, TYPE_POINT3, fnGetHitBary, TYPE_INT);
			FN_1(grid_gethitnorm, TYPE_POINT3, fnGetHitNorm, TYPE_INT);
			FN_1(grid_gethitdist, TYPE_FLOAT, fnGetHitDist, TYPE_INT);
			FN_0(grid_getclosesthit, TYPE_INT, fnGetClosestHit);
			FN_4(grid_intersectsegmentdebug, TYPE_INT, fnIntersectSegmentDebug, TYPE_POINT3, TYPE_POINT3,TYPE_BOOL,TYPE_INT);
			FN_0(grid_getfarthesthit, TYPE_INT, fnGetFarthestHit);
			FN_1(grid_closestface, TYPE_INT, fnClosestFace, TYPE_POINT3);
			FN_1(grid_getperpdist, TYPE_FLOAT, fnGetPerpDist, TYPE_INT);
			VFN_0(grid_clearstats, fnClearStats);
			VFN_0(grid_printstats, fnPrintStats);
			FN_1(grid_gethitpoint, TYPE_POINT3, fnGetHitPoint, TYPE_INT);


			
		END_FUNCTION_MAP


		FPInterfaceDesc* GetDesc();    // <-- must implement 
		virtual void	fnFree()=0;  //this nukes all your hit data,node lists,and grid data
		virtual void	fnInitialize(int gridSize)=0;  //this nukes all your hit data,node lists,and grid data
											//then it creates 1000by 1000 grid 
		virtual void	fnBuildGrid()=0;   //this builds your grid data
		virtual void	fnAddNode(INode *node)=0;
		virtual	int		fnIntersectBox(Point3 min, Point3 max)=0;
		virtual	int		fnIntersectSphere(Point3 p, float radius)=0;
		virtual	int		fnIntersectRay(Point3 p, Point3 dir, BOOL doubleSided)=0;
		virtual	int		fnIntersectSegment(Point3 p1, Point3 p2, BOOL doubleSided)=0;
		virtual int		fnGetHitFace(int index) = 0; 
		virtual Point3	fnGetHitBary(int index) = 0; 
		virtual Point3	fnGetHitNorm(int index) = 0; 
		virtual float	fnGetHitDist(int index) = 0; 

		virtual int		fnGetClosestHit() = 0; 

		virtual	int		fnIntersectSegmentDebug(Point3 p1, Point3 p2, BOOL doubleSided,int whichGrid)=0;

		virtual int		fnGetFarthestHit() = 0; 

		virtual void	fnAddMesh(Mesh *msh, Matrix3 tm)=0;

		virtual int		fnClosestFace(Point3 p)=0;

		virtual float	fnGetPerpDist(int index) = 0; 

		virtual void	fnClearStats()=0;
		virtual void	fnPrintStats()=0;

		virtual Point3	fnGetHitPoint(int index) = 0; 


//int &faceIndex, INode *node, Point3 &bary, Point3 &norm, float &dist)=0;

};



#endif // __IRAYMESHGRIDINTERSECT__H