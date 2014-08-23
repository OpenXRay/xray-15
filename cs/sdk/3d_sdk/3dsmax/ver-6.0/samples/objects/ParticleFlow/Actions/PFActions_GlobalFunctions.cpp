/**********************************************************************
 *<
	FILE:			PFActions_GlobalFunctions.cpp

	DESCRIPTION:	Collection of useful functions (definition).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"
#include "IPViewManager.h"
#include "IPFSystem.h"

#include "meshadj.h"
#include "macrorec.h"

namespace PFActions {

TCHAR* GetString(int id)
{
	enum { kBufSize = 1024 };
	static TCHAR buf1[kBufSize], buf2[kBufSize], 
				 buf3[kBufSize], buf4[kBufSize];
	static TCHAR* bp[4] = {buf1, buf2, buf3, buf4};
	static int active = 0;
	TCHAR* result = NULL;
	if (hInstance)
		result = LoadString(hInstance, id, bp[active], sizeof(buf1)) ? bp[active] : NULL;
	active = (active+1)%4; // twiddle between buffers to help multi-getstring users (up to 4)
	return result;
}

// generate random point of the surface of sphere with radius 1
Point3 RandSphereSurface(RandGenerator* randGen)
{
	float theta = randGen->Rand01() * TWOPI;
	float z = randGen->Rand11();
	float r = sqrt(1 - z*z);
	float x = r * cos(theta);
	float y = r * sin(theta);
	return Point3( x, y, z);
}

// generate a random vector with the same length as the given vector but direction
// differs for no more than maxAngle
Point3 DivergeVectorRandom(Point3 vec, RandGenerator* randGen, float maxAngle)
{
	float lenSq = LengthSquared(vec);
	if (lenSq <= 0) return vec;

	if (maxAngle <= 0.0f)
		return vec; // no divergence

	Point3 p3RS = RandSphereSurface(randGen);
	Point3 p3Orth = p3RS - vec * DotProd(vec, p3RS) / lenSq;
	while (LengthSquared(p3Orth) <= 0.0f) {
		p3RS = RandSphereSurface(randGen);
		p3Orth = p3RS - vec * DotProd(vec, p3RS) / lenSq;
	}
	Point3 p3Norm = Length(vec) * Normalize(p3Orth);
//	float fh = 1.0f - sin(maxAngle) * randGen->Rand01();
	float fh = cos(maxAngle * randGen->Rand01());
	float fq = sqrt(1.0f - fh * fh);
	return (fh*vec + fq*p3Norm);
}

// create matrix for speed space. Axis X is the same as the speed vector, axis Z is
// perp to axis x and the most upward
Matrix3 SpeedSpaceMatrix(Point3 speedVec)
{
	if (LengthSquared(speedVec) > 0.0f) {
		speedVec.Unify();
		float xComp = sqrt(1.0f - speedVec.z*speedVec.z);
		Point3 z;
		if (xComp > 0.0f) {
			z = (speedVec.z < 0.0f) ? speedVec : -speedVec;
			z *= (float)fabs(speedVec.z/xComp);
			z.z = xComp;
		} else {
			z = Point3::XAxis;
		}
		Point3 y = CrossProd(z, speedVec);
		return Matrix3( speedVec, y, z, Point3::Origin);
	}
	return Matrix3(Point3::XAxis, Point3::YAxis, Point3::ZAxis, Point3::Origin);
}

BOOL IsControlAnimated(Control* control)
{
	if (control == NULL) return FALSE;
	return control->IsAnimated();
}

ClassDesc* GetPFOperatorDisplayDesc() 
{ return &ThePFOperatorDisplayDesc; }

ClassDesc* GetPFOperatorRenderDesc() 
{ return &ThePFOperatorRenderDesc; }

ClassDesc* GetPFOperatorSimpleBirthDesc() 
{ return &ThePFOperatorSimpleBirthDesc; }

ClassDesc* GetPFOperatorSimplePositionDesc() 
{ return &ThePFOperatorSimplePositionDesc; }

ClassDesc* GetPFOperatorSimpleShapeDesc() 
{ return &ThePFOperatorSimpleShapeDesc; }

ClassDesc* GetPFOperatorInstanceShapeDesc() 
{ return &ThePFOperatorInstanceShapeDesc; }

ClassDesc* GetPFOperatorFacingShapeDesc() 
{ return &ThePFOperatorFacingShapeDesc; }

ClassDesc* GetPFOperatorMarkShapeDesc() 
{ return &ThePFOperatorMarkShapeDesc; }

ClassDesc* GetPFOperatorSimpleSpeedDesc()
{ return &ThePFOperatorSimpleSpeedDesc; }

ClassDesc* GetPFOperatorForceSpaceWarpDesc()
{ return &ThePFOperatorForceSpaceWarpDesc; }

ClassDesc* GetPFOperatorSimpleOrientationDesc()
{ return &ThePFOperatorSimpleOrientationDesc; }

ClassDesc* GetPFOperatorSimpleSpinDesc()
{ return &ThePFOperatorSimpleSpinDesc; }

ClassDesc* GetPFOperatorSimpleMappingDesc()
{ return &ThePFOperatorSimpleMappingDesc; }

ClassDesc* GetPFOperatorExitDesc()
{ return &ThePFOperatorExitDesc; }

ClassDesc* GetPFOperatorMaterialStaticDesc()
{ return &ThePFOperatorMaterialStaticDesc; }

ClassDesc* GetPFOperatorMaterialDynamicDesc()
{ return &ThePFOperatorMaterialDynamicDesc; }

ClassDesc* GetPFOperatorMaterialFrequencyDesc()
{ return &ThePFOperatorMaterialFrequencyDesc; }

ClassDesc* GetPFOperatorPositionOnObjectDesc()
{ return &ThePFOperatorPositionOnObjectDesc; }

ClassDesc* GetPFOperatorSpeedSurfaceNormalsDesc()
{ return &ThePFOperatorSpeedSurfaceNormalsDesc; }

ClassDesc* GetPFOperatorSpeedCopyDesc()
{ return &ThePFOperatorSpeedCopyDesc; }

ClassDesc* GetPFOperatorSpeedKeepApartDesc()
{ return &ThePFOperatorSpeedKeepApartDesc; }

ClassDesc* GetPFOperatorCommentsDesc()
{ return &ThePFOperatorCommentsDesc; }

ClassDesc* GetPFOperatorSimpleScaleDesc()
{ return &ThePFOperatorSimpleScaleDesc; }

ClassDesc* GetPFTestDurationDesc()
{ return &ThePFTestDurationDesc; }

ClassDesc* GetPFTestSpawnDesc()
{ return &ThePFTestSpawnDesc; }

ClassDesc* GetPFTestSpawnOnCollisionDesc()
{ return &ThePFTestSpawnOnCollisionDesc; }

ClassDesc* GetPFTestCollisionSpaceWarpDesc()
{ return &ThePFTestCollisionSpaceWarpDesc; }

ClassDesc* GetPFTestSpeedGoToTargetDesc()
{ return &ThePFTestSpeedGoToTargetDesc; }

ClassDesc* GetPFTestGoToNextEventDesc()
{ return &ThePFTestGoToNextEventDesc; }

ClassDesc* GetPFTestSplitByAmountDesc()
{ return &ThePFTestSplitByAmountDesc; }

ClassDesc* GetPFTestSplitBySourceDesc()
{ return &ThePFTestSplitBySourceDesc; }

ClassDesc* GetPFTestSplitSelectedDesc()
{ return &ThePFTestSplitSelectedDesc; }

ClassDesc* GetPFTestGoToRotationDesc()
{ return &ThePFTestGoToRotationDesc; }

ClassDesc* GetPFTestScaleDesc()
{ return &ThePFTestScaleDesc; }

ClassDesc* GetPFTestSpeedDesc()
{ return &ThePFTestSpeedDesc; }



ClassDesc* GetPFOperatorSimpleBirthStateDesc() 
{ return &ThePFOperatorSimpleBirthStateDesc; }

ClassDesc* GetPFOperatorSimplePositionStateDesc() 
{ return &ThePFOperatorSimplePositionStateDesc; }

ClassDesc* GetPFOperatorInstanceShapeStateDesc() 
{ return &ThePFOperatorInstanceShapeStateDesc; }

ClassDesc* GetPFOperatorFacingShapeStateDesc() 
{ return &ThePFOperatorFacingShapeStateDesc; }

ClassDesc* GetPFOperatorMarkShapeStateDesc() 
{ return &ThePFOperatorMarkShapeStateDesc; }

ClassDesc* GetPFOperatorMaterialStaticStateDesc()
{ return &ThePFOperatorMaterialStaticStateDesc; }

ClassDesc* GetPFOperatorMaterialDynamicStateDesc()
{ return &ThePFOperatorMaterialDynamicStateDesc; }

ClassDesc* GetPFOperatorMaterialFrequencyStateDesc()
{ return &ThePFOperatorMaterialFrequencyStateDesc; }

ClassDesc* GetPFOperatorPositionOnObjectStateDesc()
{ return &ThePFOperatorPositionOnObjectStateDesc; }

ClassDesc* GetPFTestSplitByAmountStateDesc()
{ return &ThePFTestSplitByAmountStateDesc; }


//--------------------------------------------------------------//
//		For multiple collisions per integration step (frame)	//
//--------------------------------------------------------------//

int CollisionCollection::MAX_COLLISIONS_PER_STEP = 2000;

void CollisionCollection::Init(const Tab<CollisionObject*> &cobjs)
{
	m_cobjs.SetCount(cobjs.Count());
	for(int i=0; i<cobjs.Count(); i++) m_cobjs[i] = cobjs[i];
}

Object* CollisionCollection::GetSWObject()
{
	if (m_cobjs.Count()) return m_cobjs[0]->GetSWObject();
	 else return NULL;
}

BOOL CollisionCollection::CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct, BOOL UpdatePastCollide)
{
	BOOL collide=FALSE, maybeStuck=FALSE;
	
	if (UpdatePastCollide)
	{
		for(int i=0; i<MAX_COLLISIONS_PER_STEP; i++)
		{
			if (FindClosestCollision(t, pos, vel, dt, index, ct))
			{
				collide = TRUE;
				dt -= *ct;
				if (dt <= 0.0f) break; // time limit for the current integration step
			}
			else break;
			// particle may still have a collision in the current integration step;
			// since particle reaches the limit of collision check per integration step,
			// we'd better hold on the particle movements for the current frame
			if (i==MAX_COLLISIONS_PER_STEP-1) maybeStuck = TRUE;
		}
		if ((dt > 0.0f) && (!maybeStuck)) // final interval without collisions
			pos += vel*dt;
	}
	else
		collide = FindClosestCollision(t, pos, vel, dt, index, ct);		

	return collide;
}

BOOL CollisionCollection::FindClosestCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct)
{
	Point3 curPos, curVel, resPos, resVel;
	float curTime, minTime = dt+1.0f;
	BOOL collide = FALSE;

	for(int i=0; i<m_cobjs.Count(); i++)
	{
		curPos = pos; curVel = vel;
		if (m_cobjs[i]->CheckCollision(t, curPos, curVel, dt, index, &curTime, FALSE))
			if (curTime < minTime) // the collision is the closest one
			{
				collide = TRUE;
				minTime = curTime;
				resPos = curPos;
				resVel = curVel;
			}
	}
	if (collide)
	{
		pos = resPos;
		vel = resVel;
		*ct = minTime;
	}
	return collide;
}

INode* GetHighestClosedGroupNode(INode *iNode)
{
	if (iNode == NULL) return NULL;

	INode *node = iNode;
	while (node->IsGroupMember()) {
		node = node->GetParentNode();
		while (!(node->IsGroupHead())) node = node->GetParentNode();
		if (!(node->IsOpenGroupHead())) iNode = node;
	}
	return iNode;
}

bool IsExactIntegrationStep(PreciseTimeValue time, Object* pSystem)
{
	bool exactStep = false;
	IPFSystem* iSystem = PFSystemInterface(pSystem);
	if (iSystem == NULL) return false;
	TimeValue stepSize = iSystem->GetIntegrationStep();
	if (time.tick % stepSize == 0) {
		if (fabs(time.fraction) < 0.0001) exactStep = true;
	}
	return exactStep;
}

BOOL CollisionSpaceWarpValidatorClass::Validate(PB2Value &v) 
{
	if (action == NULL) return FALSE;
	INode *node = (INode*) v.r;
	if (node->TestForLoop(FOREVER,action)!=REF_SUCCEED) return FALSE;

	// check if the item is already included
	if (paramID >= 0) {
		IParamBlock2* pblock = action->GetParamBlock(0);
		if (pblock != NULL) {
			int numItems = pblock->Count(paramID);
			for(int i=0; i<numItems; i++)
				if (pblock->GetINode(paramID, 0, i) == node)
					return FALSE;
		}
	}

	Object* ob = GetPFObject(node->GetObjectRef());
	if (ob!=NULL) 
	{	
		int id=(ob?ob->SuperClassID():SClass_ID(0));
		if (id==WSM_OBJECT_CLASS_ID)
		{
			WSMObject *obref=(WSMObject*)ob;
			CollisionObject *col = obref->GetCollisionObject(node);
			if (col)
			{
				col->DeleteThis();
				return TRUE;
			}
			else return FALSE;
		}
		else return FALSE;
	}
	return FALSE;
};

BOOL ForceSpaceWarpValidatorClass::Validate(PB2Value &v) 
{
	if (action == NULL) return FALSE;
	INode *node = (INode*) v.r;
	if (node->TestForLoop(FOREVER,action)!=REF_SUCCEED) return FALSE;

	// check if the item is already included
	if (paramID >= 0) {
		IParamBlock2* pblock = action->GetParamBlock(0);
		if (pblock != NULL) {
			int numItems = pblock->Count(paramID);
			for(int i=0; i<numItems; i++)
				if (pblock->GetINode(paramID, 0, i) == node)
					return FALSE;
		}
	}

	Object* ob = GetPFObject(node->GetObjectRef());
	if (ob!=NULL) 
	{	
		int id=(ob?ob->SuperClassID():SClass_ID(0));
		if (id==WSM_OBJECT_CLASS_ID)
		{
			WSMObject *obref=(WSMObject*)ob;
			ForceField *ff = obref->GetForceField(node);
			if (ff)
			{
				ff->DeleteThis();
				return TRUE;
			}
			else return FALSE;
		}
		else return FALSE;
	}
	return FALSE;
};

BOOL IsGEOM(Object *obj)
{ if (obj!=NULL) 
  { if (GetPFObject(obj)->IsParticleSystem()) return FALSE;
    if (obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
    { if (obj->IsSubClassOf(triObjectClassID)) 
        return TRUE;
      else 
	  { if (obj->CanConvertToType(triObjectClassID)) 
	  	return TRUE;			
	  }
	}
  }
  return FALSE;
}

BOOL GeomObjectValidatorClass::Validate(PB2Value &v) 
{
	if (action == NULL) return FALSE;
	INode *node = (INode*) v.r;
	if (node == NULL) return FALSE;
	if (node->TestForLoop(FOREVER,action)!=REF_SUCCEED) return FALSE;

	// check if the item is already included
	if (paramID >= 0) {
		IParamBlock2* pblock = action->GetParamBlock(0);
		if (pblock != NULL) {
			int numItems = pblock->Count(paramID);
			for(int i=0; i<numItems; i++)
				if (pblock->GetINode(paramID, 0, i) == node)
					return FALSE;
		}
	}

	if (node->IsGroupMember())		// get highest closed group node
		node = GetHighestClosedGroupNode(node);
	v.r = node;

	Tab<INode*> stack;
	stack.Append(1, &node, 10);
	TimeValue time = GetCOREInterface()->GetTime();
	while (stack.Count())
	{
		INode* iNode = stack[stack.Count()-1];
		stack.Delete(stack.Count()-1, 1);

		Object* obj = iNode->EvalWorldState(time).obj;
//		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) // <-- picks up splines which is not good
		if (IsGEOM(obj))
			return TRUE;

		// add children to the stack
		for (int i=0; i<iNode->NumberOfChildren(); i++) {
			INode *childNode = iNode->GetChildNode(i);
			if (childNode) stack.Append(1, &childNode, 10);
		}
	}

	return FALSE;
};

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							utility functions								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL IsBadFace(Mesh* mesh, int index)
{
	DbgAssert(mesh);
	DbgAssert(index >= 0); DbgAssert(index < mesh->numFaces);
	return ((mesh->faces[index].flags & FACE_WORK) ? TRUE : FALSE);
}

float FaceArea(Mesh* mesh, int faceIndex)
{
	DbgAssert(mesh);
	DbgAssert(faceIndex >= 0);
	DbgAssert(faceIndex < mesh->getNumFaces());
	if (IsBadFace(mesh, faceIndex)) return 0.0f;

	float resValue = 0.0f;
	Point3 vertex[3];
	for(int i=0; i<3; i++)
		vertex[i] = mesh->verts[mesh->faces[faceIndex].v[i]];
	Point3 a = vertex[1] - vertex[0];
	Point3 b = vertex[2] - vertex[0];
	resValue = 0.5f*Length(CrossProd(a, b));
	return resValue;
}

float EdgeLength(Mesh* mesh, int vertex1, int vertex2)
{
	DbgAssert(mesh);
	DbgAssert(min(vertex1,vertex2) >= 0);
	DbgAssert(max(vertex1,vertex2) < mesh->getNumVerts());

	float resValue = Length(mesh->verts[vertex1] - mesh->verts[vertex2]);
	return resValue;
}

float FaceVolume(Mesh* mesh, int faceIndex, Point3 origin)
{
	DbgAssert(mesh);
	DbgAssert(faceIndex >= 0);
	DbgAssert(faceIndex < mesh->getNumFaces());
	if (IsBadFace(mesh, faceIndex)) return 0.0f;

	Point3 a = mesh->verts[mesh->faces[faceIndex].v[0]] - origin;
	Point3 b = mesh->verts[mesh->faces[faceIndex].v[1]] - origin;
	Point3 c = mesh->verts[mesh->faces[faceIndex].v[2]] - origin;

	float det = a.x * (b.y*c.z - b.z*c.y) + 
				a.y * (b.z*c.x - b.x*c.z) +
				a.z * (b.x*c.y - b.y*c.x);
	return det;
}

float MeshVolume(Mesh* mesh)
{
	if (mesh == NULL) return 0.0f;
	int numVerts = mesh->getNumVerts();
	if (numVerts < 4) return 0.0f;
	int numFaces = mesh->getNumFaces();
	if (numFaces < 4) return 0.0f;

	Point3 origin = Point3::Origin;
	for(int i=0; i<numVerts; i++)
		origin += mesh->verts[i];
	origin /= numVerts;
	double volume = 0.0;
	for(i=0; i<numFaces; i++)
		volume += FaceVolume(mesh, i, origin);

	volume = fabs(volume);
	return float(volume);
}

bool IsFaceAboveOrigin(Point3 v1, Point3 v2, Point3 v3)
// defines if interior of the given face is above Point3::Origin
{
	Point3 a = v2 - v1;
	Point3 b = v3 - v1;
	double det0 = double(a.x)*b.y - double(a.y)*b.x;
	double det1 = double(v1.y)*b.x - double(v1.x)*b.y;
	if (det1 == 0.0) return false;
	double x = det1/det0;
	if (x <= 0.0) return false;
	double det2 = double(a.y)*v1.x - double(a.x)*v1.y;
	if (det2 == 0.0) return false;
	double y = det2/det0;
	if (y <= 0.0) return false;
	if (x+y >= 1.0) return false;
	double z = v1.z + x*a.z + y*b.z;
	return ( z > 0.0);
}

bool IsPointInsideMesh(Mesh* mesh, Point3 p)
// the method finds number of faces that are above the given point
// if the number is odd then the point is considered "inside" the mesh
// therefore the mesh doesn't have to have closed surface
{
	int numAbove = 0;
	for(int i=0; i<mesh->getNumFaces(); i++) {
		if (IsBadFace(mesh, i)) continue;
		if (IsFaceAboveOrigin(	mesh->verts[mesh->faces[i].v[0]] - p,
								mesh->verts[mesh->faces[i].v[1]] - p,
								mesh->verts[mesh->faces[i].v[2]] - p) )
			numAbove++;
	}

	return ((numAbove % 2) == 1);
}

Point3 TransformLocalFaceIntoLocalMeshCoord(Mesh* mesh, Point3 coord, int faceIndex)
{
	if (faceIndex < 0) return coord;
	if (faceIndex >= mesh->getNumFaces()) { // vertex based point
		int vertexIndex = faceIndex - mesh->getNumFaces();
//		return (mesh->verts[vertexIndex] + coord.z * mesh->getNormal(vertexIndex)); // the normal could be NaN value
		Point3 normal = mesh->getNormal(vertexIndex);
		float lenSq = LengthSquared(normal);
		Point3 localP = mesh->verts[vertexIndex];
		if ((lenSq > 0.0f) && (lenSq < 2.0f)) localP += coord.z*normal;
		return localP;
	}

	if (IsBadFace(mesh, faceIndex)) return Point3::Origin;
	Face* f = &(mesh->faces[faceIndex]);
	Point3 a = mesh->verts[f->v[0]];
	a += coord.x * (mesh->verts[f->v[1]] - a) + coord.y * (mesh->verts[f->v[2]] - a) +
			coord.z * Normalize( (1-coord.x-coord.y)*mesh->getNormal(f->v[0]) +
									coord.x*mesh->getNormal(f->v[1]) +
									coord.y*mesh->getNormal(f->v[2]) );
	return a;
}

bool ClosestPointOnFace(const Point3& toPoint, const Point3& v1, const Point3& v2, const Point3& v3, Point2& localCoords, Point3& worldPoint)
{
	Point3 p = toPoint	- v1;
	Point3 a = v2		- v1;
	Point3 b = v3		- v1;
	double PA = DotProd(p, a);
	double PB = DotProd(p, b);
	double AA = DotProd(a, a);
	double AB = DotProd(a, b);
	double BB = DotProd(b, b);
	double det = AA*BB - AB*AB;
	if (det == 0.0) return false;
	double x = (PA*BB - AB*PB)/det;
	if (x < 0.0) return false;
	double y = (AA*PB - PA*AB)/det;
	if (y < 0.0) return false;
	if (x+y > 1.0) return false;
	localCoords.x = float(x);
	localCoords.y = float(y);
	worldPoint = v1 + localCoords.x*a + localCoords.y*b;
	return true;
}

bool ClosestPointOnEdge(const Point3& toPoint, const Point3& v1, const Point3& v2, float& ratio, Point3& worldPoint)
{
	Point3 p = toPoint	- v1;
	Point3 a = v2		- v1;
	float AA = DotProd(a, a);
	if (AA == 0.0) return false;
	ratio = DotProd(p, a)/AA;
	if ((ratio < 0.0f) || (ratio > 1.0f)) return false;
	worldPoint = v1 + ratio*a;
	return true;
}

// fast measurement how close two vectors are (angle)
// the higher the value the closer two vectors are (1 is maximum, -1 is minimum)
float CustomDeviation(const Point3& pointDif, const Point3& faceNormal)
{
	double fn2 = double(faceNormal.x*faceNormal.x) + double(faceNormal.y*faceNormal.y) + double(faceNormal.z*faceNormal.z);
	if (fn2 <= 0.0) return -1.0f;
	double pd2 = double(pointDif.x*pointDif.x) + double(pointDif.y*pointDif.y) + double(pointDif.z*pointDif.z);
	if (pd2 <= 0.0) return 1.0f;
	float pf2 = double(pointDif.x*faceNormal.x) + double(pointDif.y*faceNormal.y) + double(pointDif.z*faceNormal.z);
	if (pf2 < 0) return (-(pf2*pf2)/(fn2*pd2));
	return float((pf2*pf2)/(fn2*pd2));
}

// finds closest point on mesh. Isolated vertices are not considered for proximity calculations
// closest point is defined by the real face geometry but not the smoothing groups and normals
// returns a distance (squared) to the closest point
// returns true if the poing was found
bool ClosestPointOnMesh(const Point3& toPoint, Mesh* mesh, Point3& worldLocation, Point2& localCoords, int& faceIndex, float& dist2)
{
	if (mesh == NULL) return false;
	int numF = mesh->getNumFaces();
	if (numF == 0) return false;

	localCoords = Point2(0.0f, 0.0f);
	worldLocation = mesh->verts[mesh->faces[0].v[0]];
	faceIndex = 0;
	Point3 dif = toPoint - worldLocation;
	dist2 = DotProd(dif, dif);
	float minDeviation = CustomDeviation(dif, mesh->FaceNormal(0));

	Point2 curLocalCoords;
	float curRatio;
	Point3 curWorldPoint;
	float curDistance2, curDeviation;
	bool goodEdge[3];

	Face* f = mesh->faces;
	for(int i=0; i<numF; i++, f++) {

		if (IsBadFace(mesh, i)) continue;

		Point3 v0 = mesh->verts[f->v[0]];
		Point3 v1 = mesh->verts[f->v[1]];
		Point3 v2 = mesh->verts[f->v[2]];

		// closest point on the inside of the face
		if (ClosestPointOnFace(toPoint, v0, v1, v2, curLocalCoords, curWorldPoint)) {
			dif = toPoint - curWorldPoint;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 < dist2) {
				dist2 = curDistance2;
				faceIndex = i;
				localCoords = curLocalCoords;
				worldLocation = curWorldPoint;
				minDeviation = 1.0f;
			}
			if (dist2 <= 0.0f) break;
			continue;
		}

		Point3 faceNormal = mesh->FaceNormal(i);

		// closest point on the first edge of the face
		if (goodEdge[0] = ClosestPointOnEdge(toPoint, v0, v1, curRatio, curWorldPoint)) {
			dif = toPoint - curWorldPoint;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(curRatio, 0.0f);
					worldLocation = curWorldPoint;
					minDeviation = curDeviation;
				}
			}
		}		

		// closest point on the second edge of the face
		if (goodEdge[1] = ClosestPointOnEdge(toPoint, v0, v2, curRatio, curWorldPoint)) {
			dif = toPoint - curWorldPoint;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(0.0f, curRatio);
					worldLocation = curWorldPoint;
					minDeviation = curDeviation;
				}
			}
		}		

		// closest point on the third edge of the face
		if (goodEdge[2] = ClosestPointOnEdge(toPoint, v1, v2, curRatio, curWorldPoint)) {
			dif = toPoint - curWorldPoint;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(1.0f - curRatio, curRatio);
					worldLocation = curWorldPoint;
					minDeviation = curDeviation;
				}
			}
		}		

		// first vertex as a candidate for the closest point
		if (!(goodEdge[0] && goodEdge[1])) {
			dif = toPoint - v0;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(0.0f, 0.0f);
					worldLocation = v0;
					minDeviation = curDeviation;
				}
			}
		}		

		// second vertex as a candidate for the closest point
		if (!(goodEdge[0] && goodEdge[2])) {
			dif = toPoint - v1;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(1.0f, 0.0f);
					worldLocation = v1;
					minDeviation = curDeviation;
				}
			}
		}		

		// third vertex as a candidate for the closest point
		if (!(goodEdge[1] && goodEdge[2])) {
			dif = toPoint - v2;
			curDistance2 = DotProd(dif, dif);
			if (curDistance2 <= dist2) {
				curDeviation = CustomDeviation(dif, faceNormal);
				if ((curDistance2 < dist2) || (curDeviation > minDeviation)) {
					dist2 = curDistance2;
					faceIndex = i;
					localCoords = Point2(0.0f, 1.0f);
					worldLocation = v2;
					minDeviation = curDeviation;
				}
			}
		}		

		if (dist2 <= 0.0f) break;
	}

	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFProbabilityData								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFProbabilityData::PFProbabilityData()
{
	_probabilityType() = kLocationType_undefined;
	_totalShare() = 0.0f;
}

void PFProbabilityData::Init(Mesh* inMesh, Matrix3* tm, int probType, TimeValue t)
{
	_probabilityType() = probType;
	_totalShare() = 0.0f;
	_probNodes().ZeroCount();
	_time() = t;

	if (inMesh == NULL) return;
	if (tm == NULL) return;

	if (probType == kLocationType_undefined) {
		_totalShare() = 0.0f;
		return;
	}

	if (probType == kLocationType_pivot) {
		_totalShare() = 1.0f;
		return;
	}
	if (probType == kLocationType_vertex) {
		_totalShare() = inMesh->getNumVerts();
		return;
	}

	int i, j;
	if (probType == kLocationType_selVertex) {
		if (inMesh->vertSel.IsEmpty()) {
			_probabilityType() = kLocationType_vertex;
			_totalShare() = inMesh->getNumVerts();
		} else {
			_selVertices().SetCount(inMesh->vertSel.NumberSet());
			for(i=0, j=0; i<inMesh->getNumVerts(); i++)
				if (inMesh->vertSel[i]) _selVertex(j++) = i;
			_totalShare() = selVertices().Count();
		}
		return;
	}

	Mesh curMesh = *inMesh;
	Mesh* mesh = &curMesh;
	for(i=0; i<mesh->getNumVerts(); i++)
		mesh->verts[i] = (*tm)*mesh->verts[i];

	if (probType == kLocationType_volume) {
		_totalShare() = MeshVolume(mesh);
		return;
	}

	int count = mesh->getNumFaces();
	_probNodes().SetCount(count);
		
	if (probType == kLocationType_selEdge)
		if (inMesh->edgeSel.IsEmpty()) _probabilityType() = kLocationType_edge;
	if (probType == kLocationType_selFaces)
		if (inMesh->faceSel.IsEmpty()) _probabilityType() = kLocationType_surface;

	Face* face;
	if ((probabilityType() == kLocationType_edge) || (probabilityType() == kLocationType_selEdge)) {
		bool useSelOnly = (probabilityType() == kLocationType_selEdge);
		MeshTempData* mtd = new MeshTempData(mesh);
		DbgAssert(mtd);
		AdjEdgeList* ael = mtd->AdjEList();
		DbgAssert(ael);
		int numEdges = ael->edges.Count();
		Tab<bool> edgeUsed;
		edgeUsed.SetCount(numEdges);
		for(i=0; i<numEdges; i++) edgeUsed[i] = false;
		for(i=0, face=mesh->faces; i<count; i++, face++) {
			_probNode(i).faceIndex = i;			
			_probNode(i).accumValue = totalShare();
			_probNode(i).probValue = 0.0f;
			for(j=0; j<3; j++) {
				_probNode(i).useEdge[j] = false;
				_probNode(i).edgeLength[j] = 0.0f;
				if (IsBadFace(mesh, i)) continue; 
				if (useSelOnly) 
					if (inMesh->edgeSel[3*i+j] == 0) continue; // the edge is not selected
				if (face->getEdgeVis(j)) {
					int edgeIndex = ael->FindEdge(face->v[j], face->v[(j+1)%3]);
					DbgAssert(edgeIndex >= 0);
					if (!edgeUsed[edgeIndex]) {
						edgeUsed[edgeIndex] = 1;
						_probNode(i).useEdge[j] = true;
						_probNode(i).edgeLength[j] = EdgeLength(mesh, face->v[j], face->v[(j+1)%3]);
						_probNode(i).probValue += _probNode(i).edgeLength[j];
					}
				} 
			}
			_totalShare() += _probNode(i).probValue;
		}
		if (mtd) delete mtd;
	} else if (probabilityType() == kLocationType_selFaces) { // selected faces
		for(i=0, face=mesh->faces; i<count; i++, face++) {
			_probNode(i).faceIndex = i;
			_probNode(i).accumValue = totalShare();
			_probNode(i).probValue = (inMesh->faceSel[i]) ? FaceArea(mesh, i) : 0.0f;
			_totalShare() += _probNode(i).probValue;
		}
	} else { // surface
		for(i=0, face=mesh->faces; i<count; i++, face++) {
			_probNode(i).faceIndex = i;
			_probNode(i).accumValue = totalShare();
			_probNode(i).probValue = FaceArea(mesh, i);
			_totalShare() += _probNode(i).probValue;
		}
	}
}

float PFProbabilityData::GetTotalShare() const
{
	return totalShare();
}

bool PFProbabilityData::GetFace(float randValue, int& faceIndex) const
{
	float value = randValue*totalShare();
	if (value < 0.0f) value = 0.0f;
	if (value > totalShare()) value = totalShare();
	if (value == 0.0f) return 0;
	// find probNode with accumValue < value and accumValue+probValue >= value
	int loIndex = 0;
	int lastIndex = probNodes().Count()-1;
	int hiIndex = lastIndex;
	int midIndex = (loIndex + hiIndex)/2;
	float loBound = probNode(midIndex).accumValue;
	float hiBound = (midIndex == lastIndex) ? totalShare() : probNode(midIndex+1).accumValue;
	bool bingo = ((loBound <= value) && (hiBound >= value));
	while (!bingo && (loIndex!=hiIndex)) {
		if (loBound > value) {
			hiIndex = midIndex;
			midIndex = (loIndex + hiIndex)/2;
		} else {
			loIndex = midIndex;
			midIndex = (loIndex + hiIndex)/2;
			if (midIndex == loIndex) midIndex = hiIndex;
		}
		loBound = probNode(midIndex).accumValue;
		hiBound = (midIndex == lastIndex) ? totalShare() : probNode(midIndex+1).accumValue;
		bingo = ((loBound <= value) && (hiBound >= value));
	}
	faceIndex = midIndex;	
	return bingo;
}

bool PFProbabilityData::GetEdge(float randValue, int& faceIndex, int& edgeIndex) const
{
	if ((probabilityType() != kLocationType_edge) 
		&& (probabilityType() != kLocationType_selEdge)) return false;
	if (!GetFace(randValue, faceIndex)) return false;
	float value = randValue*totalShare();
	if (value < 0.0f) value = 0.0f;
	if (value > totalShare()) value = totalShare();
	if (value == 0.0f) return 0;
	for(int j=0; j<3; j++) {
		edgeIndex = j;
		if (!probNode(faceIndex).useEdge[j]) continue;
		value -= probNode(faceIndex).edgeLength[j];
		if (probNode(faceIndex).accumValue >= value) break;
	}
	return true;
}

bool PFProbabilityData::GetVertex(float randValue, int& vertexIndex) const
{
	if (probabilityType() == kLocationType_vertex) {
		vertexIndex = int(randValue*totalShare());
		if (vertexIndex >= totalShare()) vertexIndex = int(totalShare()-1);
		return true;
	}
	if (probabilityType() == kLocationType_selVertex) {
		int vertCount = selVertices().Count();
		if (vertCount == 0) return false;
		vertexIndex = int(randValue*vertCount);
		if (vertexIndex >= vertCount) vertexIndex = vertCount-1;
		vertexIndex = selVertex(vertexIndex);
		return true;
	}
	return false;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFNodeGeometry									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFNodeGeometry::PFNodeGeometry()
{
	_node() = NULL;
	_timeStart() = 0;
	_animatedShape() = _subframeSampling() = 0;
	_locationType() = 0;
	_randGen() = NULL;
	_frameSize() = 100;
}

PFNodeGeometry::~PFNodeGeometry()
{
	int i;
	for(i=0; i<meshes().Count(); i++) {
		if (mesh(i) != NULL) {
			_mesh(i)->FreeAll();
			delete _mesh(i);
		}
	}
	_meshes().ZeroCount();

	for(i=0; i<probData().Count(); i++) {
		if (probData(i) != NULL) delete _probData(i);
	}
	_probData().ZeroCount();

	
	for(i=0; i<tm().Count(); i++) {
		if (tm(i) != NULL) delete _tm(i);
	}
	_tm().ZeroCount();

}

void PFNodeGeometry::Init(INode* inode, int animated, int subframe, int location, RandGenerator* randG)
{
	_node() = inode;
	_animatedShape() = animated;
	_subframeSampling() = subframe;
	_locationType() = location;
	_randGen() = randG;
	_frameSize() = GetTicksPerFrame();
	int meshCount = 1;
	if (animatedShape()) meshCount += frameSize();
	_meshes().SetCount(meshCount);
	int i;
	for(i=0; i<meshCount; i++) _mesh(i) = NULL;
	_tm().SetCount(frameSize() + 1);
	for(i=0; i<tm().Count(); i++) _tm(i) = NULL;
	_probData().SetCount(meshCount);
	for(i=0; i<meshCount; i++) _probData(i) = NULL;
}

bool PFNodeGeometry::GetLocationPoint(PreciseTimeValue t, Point3& worldLocation, Point3& localLocation, int& pointIndex,
							bool newPoint, float surfaceOffset, int attempsNum)
{
	DbgAssert(node());
	DbgAssert(randGen());

	Mesh* mesh = NULL;
	if (locationType() != kLocationType_pivot) {
		mesh = GetMesh(t); 
		if (mesh == NULL) return false;
	}
	PFProbabilityData* pData = NULL;

	if (newPoint) {
		switch(locationType()) {
		case kLocationType_pivot:
			localLocation = Point3::Origin;
			pointIndex = -1; // local mesh coordinates
			break;
		case kLocationType_vertex:
		case kLocationType_selVertex:
			localLocation = Point3(0.0f, 0.0f, surfaceOffset);
			pointIndex = 0;
			pData = GetProbData(t);			
			if (pData)
				pData->GetVertex(randGen()->Rand01(), pointIndex);
			pointIndex += mesh->getNumFaces(); // to distinguish from face based points
			break;
		case kLocationType_edge: 
		case kLocationType_selEdge: {
			pData = GetProbData(t);			
			pointIndex = 0;
			if (pData) {
				int edgeIndex=0;
				pData->GetEdge(randGen()->Rand01(), pointIndex, edgeIndex);
				float edgeRatio = randGen()->Rand01();
				if (edgeIndex == 0) {
					localLocation = Point3(edgeRatio, 0.0f, surfaceOffset);
				} else if (edgeIndex == 1) {
					localLocation = Point3(edgeRatio, 1.0f-edgeRatio, surfaceOffset);
				} else {
					localLocation = Point3(0.0f, edgeRatio, surfaceOffset);
				}
			}
												  }
			break;
		case kLocationType_surface:
		case kLocationType_selFaces:
			pData = GetProbData(t);
			pointIndex = 0;
			if (pData) {
				pData->GetFace(randGen()->Rand01(), pointIndex);
				float vec1 = randGen()->Rand01();
				float vec2 = randGen()->Rand01();
				if (vec1 + vec2 > 1.0f) {
					vec1 = 1.0f - vec1;
					vec2 = 1.0f - vec2;
				}
				localLocation = Point3(vec1, vec2, surfaceOffset);
			}
			break;
		case kLocationType_volume:
			pointIndex = -1; // local mesh coordinates
			localLocation = Point3::Origin;
			Box3 box = mesh->getBoundingBox();
			for(int i=0; i<attempsNum; i++) {
				float x = box.pmin.x + randGen()->Rand01()*(box.pmax.x - box.pmin.x);
				float y = box.pmin.y + randGen()->Rand01()*(box.pmax.y - box.pmin.y);
				float z = box.pmin.z + randGen()->Rand01()*(box.pmax.z - box.pmin.z);
				Point3 dicePoint = Point3( x, y, z);
				if (IsPointInsideMesh(mesh, dicePoint)) {
					localLocation = dicePoint;
					break;
				}
			}
			break;
		}
	}

	// check if the mesh is valid: enough faces and vertices
	if (pointIndex < -1) return false; // -1 is reserved for local mesh coordinates; no vertices or faces involved
	else if (pointIndex >= 0) {
		if (pointIndex >= mesh->getNumFaces())
		if (pointIndex - mesh->getNumFaces() >= mesh->getNumVerts())
			return false;
	}

	worldLocation = TransformLocalFaceIntoLocalMeshCoord(mesh, localLocation, pointIndex);
	if (tm().Count() != 0) {
		if (!EnsureTimeInterval(t)) return false;
		TimeValue t1 = t.TimeValue();
		if (!subframeSampling()) t1 = (frameSize()/2)*(t1/(frameSize()/2));
		if (t < PreciseTimeValue(t1))
			t1 -= subframeSampling() ? 1 : frameSize()/2;
		DbgAssert((t1 >= timeStart()) && (t1 <= timeStart()+frameSize()));
		TimeValue t2 = t1 + (subframeSampling() ? 1 : frameSize()/2);
		if (t2 > timeStart()+frameSize()) {
			t1 -= frameSize()/2;
			t2 -= frameSize()/2;
		}
		float timeRatio = (float(t) - t1)/(t2 - t1);
		Matrix3* tm1 = GetTM(t1);
		Matrix3* tm2 = GetTM(t2);
		if ((tm1 != NULL) && (tm2 != NULL)) {
			worldLocation = (1-timeRatio)*(worldLocation*(*tm1)) + timeRatio*(worldLocation*(*tm2));
		} else if (tm1 != NULL) {
			worldLocation = worldLocation*(*tm1);
		} else if (tm2 != NULL) {
			worldLocation = worldLocation*(*tm2);
		}
	}

	return true;
}

bool PFNodeGeometry::GetClosestPoint(PreciseTimeValue t, const Point3& toPoint, Point3& worldLocation, 
									 Point3& localLocation, int& faceIndex)
{
	Matrix3* tm = GetTM(t);
	Mesh* mesh = GetMesh(t);
	if ((tm == NULL) || (mesh == NULL)) return false;

	Point3 P = toPoint*Inverse(*tm);
	Point3 worldPoint;
	Point2 localCoords;
	float distance2;
	bool res = ClosestPointOnMesh(P, mesh, worldPoint, localCoords, faceIndex, distance2);
	if (res) {
		worldLocation = worldPoint*(*tm);
		localLocation = Point3(localCoords.x, localCoords.y, 0.0f);
	}
	return res;
}

float PFNodeGeometry::GetProbabilityShare(TimeValue time)
{
	float probShare = 0.0f;
	PFProbabilityData* pd = GetProbData(time);
	if (pd) probShare = pd->GetTotalShare();
	return probShare;
}

Matrix3* PFNodeGeometry::GetTM(TimeValue time)
{
	if (node() == NULL) return NULL;
	if (tm().Count() == 0) return NULL;
	if (!EnsureTimeInterval(time)) return NULL;
	int index = time - timeStart();
	if (!subframeSampling())  {
		int halfFrame = frameSize()/2;
		index = halfFrame*(index/halfFrame); // round down to half-frame
	}
	DbgAssert((index >=0) && (index <= frameSize()));
	if (tm(index) == NULL) {
		TimeValue indexTime = timeStart() + index;
		Matrix3* newTM = new Matrix3();
		*newTM = _node()->GetObjTMAfterWSM(indexTime);
		_tm(index) = newTM;
	}
	return tm(index);
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* GetTriObjectFromNode(INode *iNode, const TimeValue t, bool &deleteIt)
{
	deleteIt = false;
	if (iNode == NULL) return NULL;
	Object *obj = iNode->EvalWorldState(t).obj;
	obj = GetPFObject(obj);
	if (obj == NULL) return NULL;
	if (obj->IsParticleSystem()) return NULL;
    if (obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		if (obj->IsSubClassOf(triObjectClassID)) {
		  return (TriObject*)obj;
		} else if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
			TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (obj != tri) deleteIt = true;
			return tri;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

// marks degenerate and illegal faces
BOOL MarkBadFaces(Mesh* mesh)
{
	BOOL found = FALSE;
	for (int i=0; i<mesh->numFaces; i++) mesh->faces[i].flags &= ~FACE_WORK;
	DWORD numVerts = (DWORD)(mesh->numVerts);
	for (i=0; i<mesh->numFaces; i++) 
	{
		if (mesh->faces[i].v[0]==mesh->faces[i].v[1] ||
			mesh->faces[i].v[0]==mesh->faces[i].v[2] ||
			mesh->faces[i].v[2]==mesh->faces[i].v[1]) 
		{
			mesh->faces[i].flags |= FACE_WORK;
			found = TRUE;
		} else if (mesh->faces[i].v[0]>=(DWORD)numVerts ||
			mesh->faces[i].v[1]>=(DWORD)numVerts ||
			mesh->faces[i].v[2]>=(DWORD)numVerts) 
		{
			mesh->faces[i].flags |= FACE_WORK;
			found = TRUE;
		}
	}
	return found;
}

Mesh* PFNodeGeometry::GetMesh(TimeValue time)
{
	if (node() == NULL) return NULL;
	if (meshes().Count() == 0) return NULL;
	if (!EnsureTimeInterval(time)) return NULL;
	int index = animatedShape() ? time - timeStart() : 0;
	if (!subframeSampling()) index = int(index/(frameSize()/2)); // round down to half-frame
	DbgAssert((index >=0) && (index <= frameSize()));
	if (mesh(index) == NULL) {
		int wasHolding = theHold.Holding();
		if (wasHolding) theHold.Suspend();
		TimeValue indexTime = timeStart() + index;
		bool deleteIt;
		TriObject *triObj = GetTriObjectFromNode(node(), indexTime, deleteIt);
		if (triObj) {
			Mesh *newMesh = new Mesh();
			*newMesh = triObj->GetMesh();
			// unify face-smooth data
			for(int i=0; i<newMesh->getNumFaces(); i++)
				newMesh->faces[i].setSmGroup(1);
			// unify normals
			MarkBadFaces(newMesh);
			// newMesh->UnifyNormals(FALSE); // may disturb the actual mesh normals [bayboro 07-14-2003]
			newMesh->buildBoundingBox();
			newMesh->buildRenderNormals();

			if (deleteIt) triObj->DeleteMe();
			_mesh(index) = newMesh;
		}
		if (wasHolding) theHold.Resume();
	}
	return mesh(index);
}

PFProbabilityData* PFNodeGeometry::GetProbData(TimeValue time)
{
	if (node() == NULL) return NULL;
	if (probData().Count() == 0) return NULL;
	if (!EnsureTimeInterval(time)) return NULL;
	int index = animatedShape() ? time - timeStart() : 0;
	if (!subframeSampling()) index = int(index/(frameSize()/2)); // round down to half-frame
	DbgAssert((index >=0) && (index <= frameSize()));
	if (probData(index) == NULL) {
		Mesh* curMesh = GetMesh(time);
		Matrix3 *tm = GetTM(time);
		if ((curMesh != NULL) && (tm != NULL)) {
			_probData(index) = new PFProbabilityData();
			_probData(index)->Init(curMesh, tm, locationType(), time);
		}
	} else if (animatedShape() && (time != _probData(index)->GetTime())) {
		Mesh* curMesh = GetMesh(time);
		Matrix3 *tm = GetTM(time);
		if ((curMesh != NULL) && (tm != NULL)) {
			_probData(index)->Init(curMesh, tm, locationType(), time);
		}
	}
	
	return probData(index);
}

bool PFNodeGeometry::EnsureTimeInterval(TimeValue time)
{
	bool res = true;
	int index = time - timeStart();
	if ((index < 0) || (index > frameSize())) { // out of time scope; time interval needs adjustment
		TimeValue newStartTime;
		int halfFrame = frameSize()/2;
		if (index > 0) { // move interval forward
			if (time >= 0)
				newStartTime = halfFrame * (int((time-1)/halfFrame)-1);
			else
				newStartTime = halfFrame * int((time/halfFrame)-2);
		} else {// move interval backward
			if (time >= 0)
				newStartTime = halfFrame * int(time/halfFrame);
			else
				newStartTime = halfFrame * (int((time+1)/halfFrame) - 1);
		}
		res = SetStartTime(newStartTime);
	}
	return res;
}

bool PFNodeGeometry::SetStartTime(TimeValue time)
{
	int i, indexDif = time - timeStart();
	if (indexDif == 0) return true;

	// shift/renew tm
	if (abs(indexDif) > frameSize()) { // total refresh
		for(i=0; i<tm().Count(); i++) {
			if (tm(i) != NULL) {
				delete _tm(i);
				_tm(i) = NULL;
			}
		}
	} else {
		if (indexDif > 0) {
			for(i=0; i<indexDif; i++)
				if (tm(i) != NULL) delete _tm(i);
			for(i=0; i<=(frameSize()-indexDif); i++)
				_tm(i) = tm(i+indexDif);
			for(i=(frameSize()-indexDif+1); i<=frameSize(); i++)
				_tm(i) = NULL;
		} else {
			for(i=frameSize(); i>(frameSize()+indexDif); i--)
				if (tm(i) != NULL) delete _tm(i);
			for(i=frameSize(); i>=(-indexDif); i--)
				_tm(i) = tm(i+indexDif);
			for(i=0; i<(-indexDif); i++)
				_tm(i) = NULL;
		}
	}

	// shift/renew meshes
	if (animatedShape()) {
		if (abs(indexDif) > frameSize()) { // total refresh
			for(i=0; i<meshes().Count(); i++) {
				if (mesh(i) != NULL) {
					_mesh(i)->FreeAll();
					delete _mesh(i);
					_mesh(i) = NULL;
				}
			}
		} else {
			if (indexDif > 0) {
				for(i=0; i<indexDif; i++) {
					if (mesh(i) != NULL) {
						_mesh(i)->FreeAll();
						delete _mesh(i);
					}
				}
				for(i=0; i<=(frameSize()-indexDif); i++)
					_mesh(i) = mesh(i+indexDif);
				for(i=(frameSize()-indexDif+1); i<=frameSize(); i++)
					_mesh(i) = NULL;
			} else {
				for(i=frameSize(); i>(frameSize()+indexDif); i--) {
					if (mesh(i) != NULL) {
						_mesh(i)->FreeAll();
						delete _mesh(i);
					}
				}
				for(i=frameSize(); i>=(-indexDif); i--)
					_mesh(i) = mesh(i+indexDif);
				for(i=0; i<(-indexDif); i++)
					_mesh(i) = NULL;
			}
		}
	}

	_timeStart() = time;
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFObjectMaterialShadeContext						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFObjectMaterialShadeContext::PFObjectMaterialShadeContext()
{
	mode = SCMODE_NORMAL;
	doMaps = TRUE;
	filterMaps = TRUE;
	mtlNum = 0;
	shadow = FALSE;
	backFace = FALSE;
	ambientLight = Color(1.0f, 1.0f, 1.0f);
	nLights = 0;
	rayLevel = 0;
	viewVec = Point3(0.0f, 1.0f, 0.0f);
	normal = gNormal = Point3::ZAxis;

	nodeGeom = NULL;
	locationPoint.init = false;
	worldLocation = Point3::Origin;
	currentTime = 0;
}

void PFObjectMaterialShadeContext::Init(PFNodeGeometry* nodeG, LocationPoint& locP, Point3& worldL, TimeValue t)
{
	nodeGeom = nodeG;
	locationPoint = locP;
	worldLocation = worldL;
	currentTime = t;
	mtlNum = 0;
	normal = gNormal = Point3::ZAxis;
	localLocation = Point3::Origin;
	if ((nodeGeom != NULL) && locP.init) {
		Mesh* mesh = nodeGeom->GetMesh(t);
		if (mesh != NULL) {
			if (locP.pointIndex >= 0) {
				if (locP.pointIndex < mesh->getNumFaces()) {
					mtlNum = mesh->getFaceMtlIndex(locP.pointIndex);
					Face* f = &(mesh->faces[locP.pointIndex]);
					localLocation = mesh->verts[f->v[0]] + 
									locP.location.x*(mesh->verts[f->v[1]] - mesh->verts[f->v[0]]) +
									locP.location.y*(mesh->verts[f->v[2]] - mesh->verts[f->v[0]]);
					gNormal = Normalize( (1.0f - locP.location.x - locP.location.y)*mesh->getNormal(f->v[0]) +
										 locP.location.x*mesh->getNormal(f->v[1]) +
										 locP.location.y*mesh->getNormal(f->v[2]) );
				} else {
					int vertexIndex = locP.pointIndex - mesh->getNumFaces();
					localLocation = mesh->verts[vertexIndex];
					gNormal = mesh->getNormal(vertexIndex);
					float lenSq = LengthSquared(gNormal);
					if (!((lenSq > 0.0f) && (lenSq < 2.0f))) gNormal = Point3::XAxis;
				}
				gNormal = nodeGeom->GetTM(t)->VectorTransform(gNormal);
				normal = Point3(gNormal.x, gNormal.z, -gNormal.y);
			} else {
				localLocation = locP.location;
			}
		}
	}
}

INode* PFObjectMaterialShadeContext::Node() 
{ 
	return ( nodeGeom ? nodeGeom->GetNode() : NULL ); 
}

int PFObjectMaterialShadeContext::FaceNumber()
{
	if (locationPoint.init && (nodeGeom != NULL)) {
		Mesh* mesh = nodeGeom->GetMesh(currentTime);
		if (mesh != NULL)
			if ((locationPoint.pointIndex >= 0) && (locationPoint.pointIndex < mesh->getNumFaces()))		
				return locationPoint.pointIndex;
	}
	return 0;
}

Box3 PFObjectMaterialShadeContext::ObjectBox()
{
	Box3 box;
	if (nodeGeom != NULL) {
		Mesh* mesh = nodeGeom->GetMesh(currentTime);
		if (mesh != NULL)
			box = nodeGeom->GetMesh(currentTime)->getBoundingBox();
	}
	return box;
}

Point3 PFObjectMaterialShadeContext::PObjRelBox()
{
	Point3 relLocation = localLocation;
	if (nodeGeom != NULL) {
		Mesh* mesh = nodeGeom->GetMesh(currentTime);
		if (mesh != NULL) {
			Box3 box = mesh->getBoundingBox();
			Point3 difDim = box.pmax - box.pmax;
			for(int i=0; i<3; i++) {
				if (difDim[i] == 0.0f) difDim[i] = 1.0f;
				relLocation[i] = (relLocation[i] - box.pmin[i])/difDim[i];
			}
		}
	}
	return relLocation;
}

Point3 PFObjectMaterialShadeContext::UVW(int channel)
{
	Point3 texPos = PObjRelBox();
	if ((nodeGeom != NULL) && locationPoint.init) {
		Mesh* mesh = nodeGeom->GetMesh(currentTime);
		if (mesh != NULL) {
			if (locationPoint.pointIndex >= 0) {
				if (locationPoint.pointIndex < mesh->getNumFaces()) {
					if (mesh->mapFaces(channel) != NULL) {
						TVFace f = mesh->mapFaces(channel)[locationPoint.pointIndex];
						texPos = (1.0f - locationPoint.location.x - locationPoint.location.y)*mesh->mapVerts(channel)[f.t[0]] +
									locationPoint.location.x*mesh->mapVerts(channel)[f.t[1]] +
									locationPoint.location.y*mesh->mapVerts(channel)[f.t[2]];
					}
				} else {
					int vertexIndex = locationPoint.pointIndex - mesh->getNumFaces();
					bool foundVertex = false;
					if ((mesh->mapFaces(channel) != NULL) && (mesh->mapVerts(channel) != NULL)) {
						for(int i=0; i<mesh->getNumFaces(); i++) {
							for(int j=0; j<3; j++) {
								if (vertexIndex == mesh->faces[i].v[j]) {
									texPos = mesh->mapVerts(channel)[mesh->mapFaces(channel)[i].t[j]];
									foundVertex = true;
									break;
								}
							}
							if (foundVertex) break;
						}
					}
				}
			}
		}
	}
	return texPos;
}

Point3 PFObjectMaterialShadeContext::PointTo(const Point3& p, RefFrame ito)
{
	Point3 res = p;
	if ((ito == REF_WORLD) || (ito == REF_CAMERA))
		if (nodeGeom)
			res = res*(*(nodeGeom->GetTM(currentTime)));
	if (ito == REF_CAMERA)
		res = Point3(res.x, res.z, -res.y);
	return res;
}

Point3 PFObjectMaterialShadeContext::PointFrom(const Point3& p, RefFrame ifrom)
{
	Point3 res = p;
	if (ifrom == REF_CAMERA)
		res = Point3(res.x, -res.z, res.y);
	if ((ifrom == REF_CAMERA) || (ifrom == REF_WORLD))
		if (nodeGeom)
			res = res*Inverse(*(nodeGeom->GetTM(currentTime)));
	return res;
}

Point3 PFObjectMaterialShadeContext::VectorTo(const Point3& p, RefFrame ito)
{
	Point3 res = p;
	if ((ito == REF_WORLD) || (ito == REF_CAMERA))
		if (nodeGeom)
			res = nodeGeom->GetTM(currentTime)->VectorTransform(res);
	if (ito == REF_CAMERA)
		res = Point3(res.x, res.z, -res.y);
	return res;

}

Point3 PFObjectMaterialShadeContext::VectorFrom(const Point3& p, RefFrame ifrom)
{
	Point3 res = p;
	if (ifrom == REF_CAMERA)
		res = Point3(res.x, -res.z, res.y);
	if ((ifrom == REF_CAMERA) || (ifrom == REF_WORLD))
		if (nodeGeom)
			res = Inverse(*(nodeGeom->GetTM(currentTime))).VectorTransform(res);
	return res;
}

void ApplyMtlIndex(Mesh* mesh, int mtlIndex)
{
	if (mtlIndex < 0) return;
	for(int i=0; i<mesh->getNumFaces(); i++)
		mesh->setFaceMtlIndex(i, mtlIndex);
}

bool VerifyEmittersMXSSync(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID)
{
	if (pblock == NULL) return true;
	int count = pblock->Count(kRealRefObjectsID);
	if (count != pblock->Count(kMXSRefObjectsID)) return false;
	for(int i=0; i<count; i++) {
		if (pblock->GetINode(kRealRefObjectsID, 0, i) != 
			pblock->GetINode(kMXSRefObjectsID, 0, i)) 
			return false;
	}
	return true;
}

bool UpdateFromRealRefObjects(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID, 
							  bool& gUpdateInProgress)
{
	if (pblock == NULL) return false;
	if (gUpdateInProgress) return false;
	if (VerifyEmittersMXSSync(pblock, kRealRefObjectsID, kMXSRefObjectsID)) return false;
	gUpdateInProgress = true;
	pblock->ZeroCount(kMXSRefObjectsID);
	for(int i=0; i<pblock->Count(kRealRefObjectsID); i++) {
		INode* refNode = pblock->GetINode(kRealRefObjectsID, 0, i);
		pblock->Append(kMXSRefObjectsID, 1, &refNode);
	}
	gUpdateInProgress = false;
	return true;
}

bool UpdateFromMXSRefObjects(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID, 
							 bool& gUpdateInProgress, HitByNameDlgCallback* callback)
{
	if (pblock == NULL) return false;
	if (gUpdateInProgress) return false;
	if (VerifyEmittersMXSSync(pblock, kRealRefObjectsID, kMXSRefObjectsID)) return false;
	gUpdateInProgress = true;
	pblock->ZeroCount(kRealRefObjectsID);
	for(int i=0; i<pblock->Count(kMXSRefObjectsID); i++) {
		INode* node = pblock->GetINode(kMXSRefObjectsID, 0, i);
		if (node == NULL) {
			pblock->Append(kRealRefObjectsID, 1, &node);
		} else {
			if (callback->filter(node) == TRUE) {
				pblock->Append(kRealRefObjectsID, 1, &node);
			} else {
				node = NULL;
				pblock->Append(kRealRefObjectsID, 1, &node);
				pblock->SetValue(kMXSRefObjectsID, 0, node, i);
			}
		}
	}
	gUpdateInProgress = false;
	return true;
}

void MacrorecObjects(ReferenceTarget* rtarg, IParamBlock2* pblock, int paramID, TCHAR* paramName)
{
	if ((pblock == NULL) || (rtarg == NULL)) return;
	TSTR selItemNames = _T("(");
	int numItems = pblock->Count(paramID);
	bool nonFirst = false;
	for(int i=0; i<numItems; i++) {
		INode* selNode = pblock->GetINode(paramID, 0, i);
		if (selNode == NULL) continue;
		if (nonFirst) selItemNames = selItemNames + _T(", ");
		selItemNames = selItemNames + _T("$'");
		selItemNames = selItemNames + selNode->GetName();
		selItemNames = selItemNames + _T("'");
		nonFirst = true;
	}
	selItemNames = selItemNames + _T(")");
	TCHAR selNames[8192];
	sprintf(selNames,"%s", selItemNames);
	macroRec->EmitScript();
	macroRec->SetProperty(rtarg, paramName, mr_name, selNames);
}

} // end of namespace PFActions

