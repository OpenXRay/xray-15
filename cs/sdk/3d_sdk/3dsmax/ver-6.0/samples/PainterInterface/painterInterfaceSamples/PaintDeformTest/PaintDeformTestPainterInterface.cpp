#include "PaintDeformTest.h"


// This is called when the user tart a pen stroke
BOOL  PaintDeformTest::StartStroke()
{
//start holding
	theHold.Begin();
	for (int i =0; i < painterNodeList.Count(); i++)
		{
		theHold.Put(new PointRestore(this, painterNodeList[i].pmd));
		}
//put our hold records down
	lagRate = pPainter->GetLagRate();
	lagCount = 1;
	return TRUE;
}

//This is called as the user strokes across the mesh or screen with the mouse down
BOOL  PaintDeformTest::PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) 
{
//restore hold
	theHold.Restore();
	for (int i =0; i < painterNodeList.Count(); i++)
		{
		int ct;
		Point3 lNormal = worldNormal * painterNodeList[i].tmToLocalSpace;
		float *str = pPainter->RetrievePointGatherStr(painterNodeList[i].node, ct);
		Point3 *normals = pPainter->RetrievePointGatherNormals(painterNodeList[i].node, ct);
		if (normals)
			{
			for (int j = 0; j < ct; j++)
				{
				if (*str > 0.0f)
					{
					
					painterNodeList[i].pmd->offsetList[j] += (normals[j] * (*str));
					}
				str++;
				}
			}
		
		}


	if ((lagRate == 0) || (lagCount%lagRate) == 0)
		NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);

	lagCount++;

	return TRUE;
}
		// This is called as the user ends a strokes when the users has it set to always update
BOOL  PaintDeformTest::EndStroke()
{
//accept hold
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	theHold.Accept(_T(GetString(IDS_PW_PAINT)));
//since this changes the mesh we need to update the mesh
 	pPainter->UpdateMeshes(TRUE);

	return TRUE;
}
		// This is called as the user ends a strokes when the users has it set to update on mouse up only
		// the canvas gets a list of all points, normals etc instead of one at a time
		//		int ct - the number of elements in the following arrays
		//  <...> see paintstroke() these are identical except they are arrays of values
BOOL  PaintDeformTest::EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal)
{

	for (int i =0; i < painterNodeList.Count(); i++)
		{
		int ct;

		float *str = pPainter->RetrievePointGatherStr(painterNodeList[i].node, ct);
		Point3 *normals = pPainter->RetrievePointGatherNormals(painterNodeList[i].node, ct);
		for (int j = 0; j < ct; j++)
			{
			if (*str > 0.0f)
				{
				painterNodeList[i].pmd->offsetList[j] += (normals[j] * (*str));
				}
			str++;
			}
		
		}

	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//accept hold

	theHold.Accept(_T(GetString(IDS_PW_PAINT)));
//since this changes the mesh we need to update the mesh
	pPainter->UpdateMeshes(TRUE);
	return TRUE;
}
		// This is called as the user cancels a stroke by right clicking
BOOL  PaintDeformTest::CancelStroke()
{
//cancel hold
	theHold.Cancel();
	return TRUE;
}
		//This is called when the painter want to end a paint session for some reason.
BOOL  PaintDeformTest::SystemEndPaintSession()
{
return TRUE;
}
