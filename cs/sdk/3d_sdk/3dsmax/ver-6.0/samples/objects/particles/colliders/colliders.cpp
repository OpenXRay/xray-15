/**********************************************************************
 *<
	FILE: Colliders.cpp

	DESCRIPTION: Collision engines for particles etc

	CREATED BY: Peter Watje
	MODIFIED BY: Oleg Bayborodin

	HISTORY: 10/15/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ICollision.h"
#include "Colliders.h"


#define PBLOCK_REF_NO 0

#define USE_OLD_COLLISION 0

// We have to avoid some extreme cases for axes sizes (because of inverse matrices)
// There should not be more than 100x difference in axes size
// Length of the largest axis should be more than 0.0001
static float axisDif = 100.0f;
static float axisThreshold = 0.0001f;

void RefillAxes(Point3 &axis1, Point3 &axis2, Point3 &axis3)
{ // creates two axes which combined with axis1 makes orthogonal vector set
	int mc = axis1.MaxComponent();
	axis2[mc] = -axis1[(mc+1)%3];
	axis2[(mc+1)%3] = axis1[mc];
	axis2[(mc+2)%3] = 0.0f;
	axis2.Unify();
	axis3 = Normalize(axis1)^axis2;
	axis3.Unify();
}

void AdjustSmallAxes(Matrix3 &tm)
{
	int i, maxIndex = 0;
	float curLen, maxLen = Length(tm.GetRow(0));
	// find the largest axis
	for(i=1; i<3; i++)
	{
		curLen = Length(tm.GetRow(i));
		if (curLen > maxLen)
		{
			maxIndex = i;
			maxLen = curLen;
		}
	}

	// increase the largest axis if too small
	if (maxLen < axisThreshold)
	{
		if (maxLen == 0.0f) 
		{ // point sphere
			Matrix3 mid;
			mid.IdentityMatrix();
			for(i=0; i<3; i++)
				tm.SetRow(i, axisThreshold*(mid.GetRow(i)));
			return; // small identity TM created
		}
		else
		{
			tm.SetRow(maxIndex, (axisThreshold/maxLen)*tm.GetRow(maxIndex));
		}
		maxLen = axisThreshold;
	}

	// increase other axes if too small (less than 0.01*largest_axis)
	Point3 axis1, axis2, axis3;
	int j2 = (maxIndex+1)%3;
	int j3 = (maxIndex+2)%3;
	float curLen2 = Length(tm.GetRow(j2));
	float curLen3 = Length(tm.GetRow(j3));
	axis1 = tm.GetRow(maxIndex);
	
	// two zero axis: adjust
	if ((curLen2==0.0f) && (curLen3==0.0f))
	{ 
		RefillAxes(axis1, axis2, axis3);
		axis2 *= maxLen/axisDif;
		axis3 *= maxLen/axisDif;
		tm.SetRow(j2, axis2);
		tm.SetRow(j3, axis3);
		return;
	}

	// second axis is too small
	if (curLen2*axisDif < maxLen)
	{
		if (curLen2 == 0.0f) // construct the axis
			axis2 = Normalize(tm.GetRow(j3))^axis1;
		else
			axis2 = tm.GetRow(j2);
		axis2 = (maxLen/axisDif)*Normalize(axis2);
		tm.SetRow(j2, axis2);
	}

	// third axis is too small
	if (curLen3*axisDif < maxLen)
	{
		if (curLen3 == 0.0f) // construct the axis
			axis3 = axis1^Normalize(tm.GetRow(j2));
		else 
			axis3 = tm.GetRow(j3);
		axis3 = (maxLen/axisDif)*Normalize(axis3);
		tm.SetRow(j3, axis3);
	}
}

bool NonUniformScaled(Matrix3 &tm)
// assumption: matrix is orhogonal and doesn't have zero-length axes
{
	if (SCL_IDENT && tm.GetIdentFlags()) return false; // no scaling for the TM
	float len0sq = LengthSquared(tm.GetRow(0));
	float len1sq = LengthSquared(tm.GetRow(1));
	float len2sq = LengthSquared(tm.GetRow(2));

	if (len0sq == 0.0f) return true; // if first axis is zero, it's non-uniform scaled matrix (or at least bad matrix to work with)
	if (fabs(len1sq/len0sq-1.0f) > 0.0001f) return true;
	if (fabs(len2sq/len0sq-1.0f) > 0.0001f) return true;

	return false;
}

class TMInterpolater
{
public:
	void Init(Matrix3 &start, Matrix3 &end);
	Matrix3& Interpolate(float timeRatio);

private:
	Matrix3 startTM, endTM, outTM;
	Point3 startPos, difPos, outPos;
	Point3 startScale, difScale, outScale;
	Quat startQ, endQ, outQ;
};

void TMInterpolater::Init(Matrix3& start, Matrix3 &end)
{
	startTM = start;
	endTM = end;

	startPos = start.GetTrans();
	difPos = end.GetTrans() - startPos;

	startScale = Point3( Length(start.GetRow(0)), Length(start.GetRow(1)), Length(start.GetRow(2)) );
	Point3 endScale = Point3( Length(end.GetRow(0)), Length(end.GetRow(1)), Length(end.GetRow(2)) );
	difScale = endScale - startScale;

	Matrix3 startTmp = start;
	startTmp.NoScale();
	Matrix3 endTmp = end;
	endTmp.NoScale();

	startQ = Quat(startTmp);
	endQ = Quat(endTmp);
}

Matrix3& TMInterpolater::Interpolate(float timeRatio)
{
	outPos = startPos + timeRatio * difPos;
	outScale = startScale + timeRatio * difScale;
	outQ = Slerp(startQ, endQ, timeRatio);

	outTM.SetRotate(outQ);
	outTM.PreScale(outScale,FALSE);
	outTM.SetTrans(outPos);

	return outTM;
}

/*

This will get implemented when we get pass by reference gets put into maxscript

class CollisionOpsImp : public CollisionOps
{
BEGIN_FUNCTION_MAP(ColllisionOpsImp)
	FN_1(collision_supportedcollisions, TYPE_INT, SuppportedCollisions,TYPE_REFTARG)
	VFN_3(collision_preframe, PreFrame,TYPE_REFTARG,TYPE_INT,TYPE_INT)
	VFN_3(collision_postframe, PostFrame,TYPE_REFTARG,TYPE_INT,TYPE_INT)
	FN_10(collision_point_to_surface, TYPE_BOOL, CheckCollision,TYPE_REFTARG, TYPE_INT,TYPE_POINT3_BV, TYPE_POINT3_BV, TYPE_FLOAT, TYPE_FLOAT, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3)
	FN_11(collision_sphere_to_surface, TYPE_BOOL, CheckCollision,TYPE_REFTARG, TYPE_INT,TYPE_POINT3_BV,TYPE_FLOAT,TYPE_POINT3_BV, TYPE_FLOAT, TYPE_FLOAT, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3)
	FN_13(collision_box_to_surface, TYPE_BOOL, CheckCollision,TYPE_REFTARG, TYPE_INT,TYPE_POINT3_BV,TYPE_FLOAT,TYPE_FLOAT,TYPE_FLOAT,TYPE_POINT3_BV, TYPE_FLOAT, TYPE_FLOAT, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3)
	FN_11(collision_edge_to_surface, TYPE_BOOL, CheckCollision,TYPE_REFTARG, TYPE_INT,TYPE_POINT3_BV,TYPE_POINT3_BV,TYPE_POINT3_BV, TYPE_FLOAT, TYPE_FLOAT, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3)
END_FUNCTION_MAP

	int SuppportedCollisions(ReferenceTarget *r);
	
	void PreFrame(ReferenceTarget *r, TimeValue &t, TimeValue &dt);
	void PostFrame(ReferenceTarget *r, TimeValue &t, TimeValue &dt);
	BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *pos, Point3 *vel, float &dt, 
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel);

//sphere to surface collision
	BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *pos, float &radius, Point3 *vel, float &dt, 
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel);
//box to surface collision FIX ME can't publish box3
	BOOL CheckCollision (ReferenceTarget *r, TimeValue &t, 
								 Point3 *boxCenter,float &w, float &h, float &d, Point3 *vel, float &dt,  
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel);

//edge to surface collision
	BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *edgeA,Point3 *edgeB ,Point3 *vel, float &dt,  
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel);



}

*/

class CollisionPlaneClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new CollisionPlane;}
	const TCHAR *	ClassName() {return GetString(IDS_PW_PLANARCOLLISION_CLASS);}
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() {return PLANAR_COLLISION_ID;}
	const TCHAR* 	Category() {return GetString(IDS_PW_COLLISIONCLASS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("PlanarCollision"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static CollisionPlaneClassDesc collisionPlaneDesc;
ClassDesc* GetCollisionPlaneDesc() {return &collisionPlaneDesc;}


//this is the data need to solve for the plane collision
//the width and height of the plane and quality parameter (the max number of iterations for the search)
//also there is a tm that is not yet hooked up.  A temporary Inode is used for now to get the tm until
//the MATRIX3_TYPE gets put into pb2 by John

// per instance geosphere block
static ParamBlockDesc2 collisionplane_param_blk ( collisionplane_params, _T("CollisionPlaneParameters"),  0, &collisionPlaneDesc, P_AUTO_CONSTRUCT, 0, 
	// params

	collisionplane_width,  _T("width"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_WIDTH, 
		p_default, 		10.0,	
		p_ms_default,	10.0,
		p_range, 		0.0f, 9999999.0f,
		end, 

	collisionplane_height,  _T("height"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_HEIGHT, 
		p_default, 		10.0,	
		p_ms_default,	10.0,
		p_range, 		0.0f, 9999999.0f,
		end, 


	collisionplane_quality,  _T("quality"),	TYPE_INT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_QUALITY, 
		p_default, 		20,	
		p_range, 		0, 100,
		end, 

	collisionplane_node,  _T("node"), 			TYPE_INODE, 	0, 	IDS_PW_NODE,   
		end, 


	end
	);

CollisionPlane::CollisionPlane()
	{
	pblock = NULL;
//instaniates the pblock
	collisionPlaneDesc.MakeAutoParamBlocks(this);
	assert(pblock);
	}

CollisionPlane::~CollisionPlane()
{
	DeleteAllRefsFromMe();
}

//this call alls you to do some computation before the frame is solved
//in this case I am createing a table of inverse matrices for each tick
//so we don't have to keep creating inverses I can jus look up the one that is appropriate at the time
//this also retrieves the length, width and quality from the pblock
// Bayboro: for each tick 2 matrices are kept: original and inverse
// we need original to transform particle position back into world
// coordinate exactly at "tick" time
void CollisionPlane::PreFrame(TimeValue t, TimeValue dt) 
{
	Interval iv;
	pblock->GetValue(collisionplane_width,t,width,iv);
	pblock->GetValue(collisionplane_height,t,height,iv);
	width  *= 0.5f;
	height *= 0.5f;
	// Bayboro: we don't need it anymore
//	pblock->GetValue(collisionplane_quality,t,quality,iv);
	INode *tempNode;
	pblock->GetValue(collisionplane_node,t,tempNode,iv); 
	if (tempNode != NULL) node = tempNode;

	validity = FOREVER;
	tm    = node->GetObjTMAfterWSM(t,&validity);
	invtm = Inverse(tm);
	if (validity.InInterval(t-dt)) // static collider for the current frame; no need to init all other matrices
		return;
	prevInvTm = Inverse(node->GetObjTMAfterWSM(t-dt,&iv));

//create a table of matrices
	TimeValue startTime, endTime;
	startTime = t - dt;
	endTime = t;
// Bayboro: since we have "multiple collision per frame" scheme, we may have several 
// calls CheckCollision for the same particle at the same frame with different t and dt.
// However the scheme guarantees that t+dt is constant for the given frame and 
// is equal to "endTime". Therefore we consider endTime as an origin for the 
// matrix list: invTmList[0] = Inverse(GetObjTMAfterWSM(t+dt))
	invTmList.SetCount(2*(endTime-startTime+1));
	int ct = 0;
	Matrix3 itm;
	for (int i = endTime; i >=startTime; i--)
	{
		itm = node->GetObjTMAfterWSM(i,&iv);
		invTmList[ct++] = Inverse(itm);
		invTmList[ct++] = itm;
	}
}

//this is the actual point to surface collision test
//it is a recuresive search across dt looking for th closest point on the surface
//returns the time of the hit, the point that was hit, the bounce velpocity vector component
//friction velocity vector component, and the amount of velocity the particle inherits from the 
//motion of the deflector
BOOL CollisionPlane::CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
									 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel )
{
	if (dt == 0.0f) return FALSE; // no time for collision

	Point3 prevP, curP, v;
	BOOL wasAbove, nowAbove, flip;

	// static collider: greatly simlifies overall calculations
	if (validity.Start() <= t-dt)
	{
		prevP = pos * invtm;	
		v = VectorTransform(invtm,vel);
		curP = prevP + v*dt;

		wasAbove = (prevP.z >= 0.0f);
		nowAbove = (curP.z >= 0.0f);
		if (wasAbove && nowAbove) return FALSE;
		if (!wasAbove && !nowAbove) return FALSE;

	// Define normal situation as "was above and now below";
	// if "flip" it TRUE, then it's a reverse situation
		flip = nowAbove;

		at = dt*prevP.z/(prevP.z-curP.z);
		if (at > 0.02f) at -= 0.01f; else at *= 0.5f;
	// See if the hit point is within our range
		hitPoint = prevP + at*v;
		if (hitPoint.x<-width || hitPoint.x>width || 
			hitPoint.y<-height || hitPoint.y>height) return FALSE;

	// lift hit point above ground
		if (flip) hitPoint.z -= 0.001f; else hitPoint.z += 0.001f;
		hitPoint = hitPoint * tm;
	//compute bounce vec
		norm = VectorTransform(tm, Point3(0.0f, 0.0f, -v.z));
	//compute friction vec in collider space
		friction = VectorTransform(tm, Point3(v.x, v.y, 0.0f));
	//since collider is static, inherited speed is zero
		inheritedVel = Point3::Origin;

		return TRUE;
	} // end of static collider

	int maxRevTime = (int)ceil(dt); // maximum reverse time >= 1

	Point3 posRev = (pos + vel*(dt - maxRevTime)) * invTmList[2*maxRevTime];
	Point3 posRevPlus = (pos + vel*(dt - maxRevTime+1)) * invTmList[2*(maxRevTime-1)];
	prevP = posRev + (posRevPlus - posRev)*(maxRevTime - dt);
	Point3 nextP = curP = (pos + vel*dt) * invTmList[0];

//do a quick check to see if the particle is not intersecting the plane
	wasAbove = (prevP.z >= 0.0f);
	nowAbove = (curP.z >= 0.0f);

	int loTime = maxRevTime;
	int hiTime = 0;
	float loOffset = prevP.z;
	float hiOffset = curP.z;
	// check if there are non-linear crossings
	if ((wasAbove == nowAbove) && (dt > 1.5f))
	{
		int midTime = maxRevTime/2;
		Point3 midP = (pos + vel*(dt - midTime)) * invTmList[2*midTime];
		float midOffset = midP.z;
		BOOL midAbove = (midOffset >= 0.0f);
		if (wasAbove != midAbove) // middle point is on the other side of the initial point
		{
			nowAbove = midAbove;
			hiTime = midTime;
			hiOffset = midOffset;
		}
		else // more verification is needed
		{
			float m = midOffset - hiOffset;
			float l = loOffset - hiOffset;
			float a = m*dt - l*midTime;
			if (fabs(a) > 0.001f) // if a is too small then we have "almost" linear path
			{
				float b = midTime*midTime*l - dt*dt*m;
				float exT = -0.5f*b/a;
				if ((exT > 0.501f) && (exT < dt-0.501f))
				{
					hiTime = (int)ceil(exT - 0.5f);
					hiOffset = ((pos + vel*(dt - hiTime)) * invTmList[2*hiTime]).z;
					nowAbove = (hiOffset >= 0.0f);
				}
			}
		}
	}

	if (wasAbove && nowAbove) return FALSE;
	if (!wasAbove && !nowAbove) return FALSE;

// Define normal situation as "was above and now below";
// if "flip" it TRUE, then it's a reverse situation
	flip = nowAbove;

	int curTime;

	// limit for number of loops; enough to recurse 1 million ticks per frame
	// find TM when the particle crosses the plane
	int recursionLevel = 20; 
	for (int i=0; i<recursionLevel; i++)
	{
		curTime = (loTime+hiTime)/2;
		if ((curTime == loTime) || (curTime==hiTime)) break; // precision is achieved
		curP = (pos + vel*(dt-curTime)) * invTmList[2*curTime];
		if ((curP.z >= 0.0f) == wasAbove) // intermid pos is on the same side as "was"
			{	loTime = curTime; loOffset = curP.z;	}
		else
			{	hiTime = curTime; hiOffset = curP.z;	}
	}

	// back up the hit time a little bit
	at = dt - loTime + loOffset/(loOffset-hiOffset);
	if (at > 0.02f) at -= 0.01f; else at *= 0.5f;
	// See if the hit point is within the range of the plan
	Matrix3 curInvTM = invTmList[2*hiTime];
	hitPoint = pos + at*vel;
	Point3 ph = hitPoint * curInvTM;
	if (ph.x<-width || ph.x>width || ph.y<-height || ph.y>height) return FALSE;

//compute approx.inherited velocity
	float inherit = inheritedVel.x;
	inheritedVel = hitPoint - ph * invTmList[2*loTime+1];
//	v = VectorTransform(curInvTM,vel);
	v = (nextP - prevP)/dt;
	if (inherit < 1.0f)
		v += (1.0f - inherit)*VectorTransform(curInvTM, inheritedVel);
//compute bounce vec
	norm.x = 0.0f;
	norm.y = 0.0f;
	norm.z = fabs(v.z);
	if (flip) norm.z = -norm.z;
	Matrix3 curTM = invTmList[2*hiTime+1];
	norm = VectorTransform(curTM, norm);
	Point3 normUnit = Normalize(norm);
	hitPoint += 0.001f*normUnit; // lift hit point above the ground

//compute friction vec in collider space
	friction = v;
	friction.z = 0.0f;
	friction = VectorTransform(curTM, friction);

	float vertOffset = DotProd(normUnit, inheritedVel);
	// Bayboro: the original version of deflector had the following
	// condition on the inheritedVel. I think it's better to comment
	// it for the property consistency
//	if (vertOffset < 0.0f) inheritedVel = Point3::Origin;
	if (vertOffset > 0.0f) hitPoint += vertOffset*normUnit; // to reduce number of inter-frame collisions

	return TRUE;
}

/*  Sphere Deflection Code  */

class CollisionSphereClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new CollisionSphere;}
	const TCHAR *	ClassName() {return GetString(IDS_PW_SPHERICALCOLLISION_CLASS);}
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() {return SPHERICAL_COLLISION_ID;}
	const TCHAR* 	Category() {return GetString(IDS_PW_COLLISIONCLASS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("SphericalCollision"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static CollisionSphereClassDesc collisionSphereDesc;
ClassDesc* GetCollisionSphereDesc() {return &collisionSphereDesc;}


//this is the data need to solve for the plane collision
//the width and height of the plane and quality parameter (the max number of iterations for the search)
//also there is a tm that is not yet hooked up.  A temporary Inode is used for now to get the tm until
//the MATRIX3_TYPE gets put into pb2 by John

// per instance geosphere block
static ParamBlockDesc2 collisionsphere_param_blk ( collisionsphere_params, _T("CollisionSphereParameters"),  0, &collisionSphereDesc, P_AUTO_CONSTRUCT, 0, 
	// params

	collisionsphere_radius,  _T("radius"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_RADIUS, 
		p_default, 		10.0,	
		p_ms_default,	10.0,
		p_range, 		0.0f, 9999999.0f,
		end, 

	collisionsphere_node,  _T("node"), 			TYPE_INODE, 	0, 	IDS_PW_NODE,   
		end, 

	collisionsphere_scaleFactor,  _T("scaleFactor"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	0, 
		p_default, 		1.0,	
		p_ms_default,	1.0,
		p_range, 		0.0f, 9999999.0f,
		end, 

	end
	);

CollisionSphere::CollisionSphere()
{
	pblock = NULL;
//instaniates the pblock
	collisionSphereDesc.MakeAutoParamBlocks(this);
	assert(pblock);
}

CollisionSphere::~CollisionSphere()
{
	DeleteAllRefsFromMe();
}

// We have to avoid some extreme cases for radius value
// Radius should be larger than 0.0001
static float radiusThreshold = 0.0001f;

//this call allows you to do some computation before the frame is solved
//so we don't have to keep creating inverses
//this also retrieves the radius from the pblock and adjust TMs to track animated radius
void CollisionSphere::PreFrame(TimeValue t, TimeValue dt) 
{
	Interval iv;
	float scaleFactor = pblock->GetFloat(collisionsphere_scaleFactor, t);
	float prevRadius;
	prevRadius = scaleFactor*pblock->GetFloat(collisionsphere_radius,t-dt);
	if (prevRadius < radiusThreshold) prevRadius = radiusThreshold;
	radius = scaleFactor*pblock->GetFloat(collisionsphere_radius,t);
	if (radius < radiusThreshold) radius = radiusThreshold;
	float scaleRatio = prevRadius/radius;

	INode *tempNode;
	pblock->GetValue(collisionsphere_node,t,tempNode,iv);
	if (tempNode != NULL) node = tempNode;

	validity = FOREVER;
	tm    = node->GetObjTMAfterWSM(t,&validity);
	// adjust axis according to thresholds
	AdjustSmallAxes(tm);
	invtm = Inverse(tm);

	Matrix3 prevTM = node->GetObjTMAfterWSM(t-dt,&iv);
	// adjust axes according to radius change to bake it in the prevTM
	prevTM.Scale(Point3(scaleRatio,scaleRatio,scaleRatio), FALSE);
	// adjust axis according to thresholds
	AdjustSmallAxes(prevTM);
	prevInvTm = Inverse(prevTM);

	bool sizeChange = (fabs(scaleRatio - 1.0f) > 0.00001f);
	if (sizeChange) // radius is changing
		validity.SetStart(t); // t-dt is no more in the validity interval
	
	// Point3 Vc is used to keep some specific data for the sphere
	Vc[0] = (float)dt; // integration step size
//	Vc[1] = prevRadius/radius; // compression coefficient
	// Vc[2] - indicator of radius change and deflector's rotating
//		// 0 - radius is constant; no rotation
//		// 1 - radius is changing; no rotation
//		// 2 - radius is constant; has rotation
//		// 3 - radius is changing; has rotation
	// if static (determined by validity interval)
		// 0 - static perfect sphere
		// 1 - static ellipsoid
	// if moving (determined by validity interval)
		// 0 - perfect sphere is moving without rotation and size change
		// 1 - perfect sphere without size change is moving and rotating
		// 2 - perfect sphere is moving and rotating and changing size
		// 3 - no restriction on shape, size change, movement and rotation
	
	if (validity.InInterval(t-dt))
	{ // static
		Vc[2] = NonUniformScaled(tm);
	}
	else // moving
	{
		// non-uniform scaling for the sphere?
		bool nonUniformScaled = NonUniformScaled(tm) || NonUniformScaled(prevTM);
		// rotation ?
		Quat curQ(tm);
		bool rotation = !curQ.Equals(Quat(prevTM));
		// size change
		Point3 scaleSq = Point3(LengthSquared(tm.GetRow(0)),LengthSquared(tm.GetRow(1)),LengthSquared(tm.GetRow(2)));
		Point3 scaleSqPrev = Point3(LengthSquared(prevTM.GetRow(0)),LengthSquared(prevTM.GetRow(1)),LengthSquared(prevTM.GetRow(2)));
		sizeChange |= !scaleSq.Equals(scaleSqPrev);
				
		Vc[2] = 0.0f;
		if (rotation) Vc[2] = 1.0f;
		if (sizeChange) Vc[2] = 2.0f;
		if (nonUniformScaled) Vc[2] = 3.0f;
	}

	// radius change?
//	Vc[2] = (float)(fabs(Vc[1] - 1.0f) > 0.00001f);

	// rotation ?
//	Quat curQ(tm);
//	if (!curQ.Equals(Quat(prevTM))) Vc[2] += 2.0f;

//	Vc=Point3(0.0f,0.0f,0.0f);
}

inline BOOL IntersectSphere(
		Ray &ray,float &t0,float &t1,float r2)
{	
	float a, b, c, ac4, b2;
	float root;	

	a = DotProd(ray.dir,ray.dir);
	b = DotProd(ray.dir,ray.p) * 2.0f;
	c = DotProd(ray.p,ray.p) - r2;
	
	ac4 = 4.0f * a * c;
	b2 = b*b;

	if (ac4 > b2) return FALSE;
	
	root = float(sqrt(b2-ac4));
	t0 = (-b + root) / (2.0f * a);
	t1 = (-b - root) / (2.0f * a);
	if (t0 > t1) {float temp=t0;t0=t1;t1=temp;}

	return TRUE;
}

//this is the actual point to sphere collision test
//it uses just a simple intersect sphere algorithm against the relative velocity of the particle
//returns the time of the hit, the point that was hit, the bounce velpocity vector component
//friction velocity vector component, and the amount of velocity the particle inherits from the 
//motion of the deflector
// this is speicial constants that helps with some numerical precision problems
static float const K1 = 0.0002f;
static float const K2 = 1.0f - K1;
static float const K3 = 0.999f;
static float const K4 = 1.0f + K1;

float VarK2( float radiusSq, Point3 &vel)
{ // the method calculates an adjustment in placing particle after contact above deflector
  // surface if the particle is inside the sphere. The height of the adjustment depends
  // on ratio of radius and the particle velocity to allow particle to travel without
  // collision for at least 1 time unit (NTSC: 160 time units per frame)
	float c = 0.5f*DotProd(vel,vel)/radiusSq;
	if (c > 0.01f) c = 0.01f;
	if (c < K1) c = K1;
	return 1.0f - c;
}

BOOL CollisionSphere::CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
									 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel )
{
	if (dt == 0.0f) return FALSE; // no time for collision

	Ray r;
	float distSquared, velDist, t0, t1, deltaT, varK2=1.0f;
	float radiusSquared = radius*radius;
	Point3 prevP, relativeVel, bounceVec, frictionVec;

//--------------------------------------------------------------//
//		static collider: simplifies overall calculations		//
//--------------------------------------------------------------//
	if (validity.Start() <= t-dt)
	{
		prevP = pos*invtm;
		relativeVel = dt*VectorTransform(invtm, vel);
		distSquared = DotProd(prevP,prevP);
		velDist = Length(relativeVel);

		r.p = prevP;
		r.dir = Normalize(relativeVel);
		//intersect sphere
		if (!IntersectSphere(r,t0,t1,radiusSquared)) return FALSE;

		//check if inside	
		if (distSquared <= radiusSquared)
		{
			if (t1*K3 > velDist) return FALSE;
		//compute hit point		
			r.p += r.dir * t1;
			bounceVec = -Normalize(r.p);
			varK2 = VarK2(radiusSquared, relativeVel/dt);
			r.p = -bounceVec * varK2*radius;  
		//compute dt used
			deltaT = t1/velDist;
		}
		//else outside
		else
		{
			if ((t0*K3 > velDist) || (t0< 0.0f)) return FALSE;
		//compute hit point		
			r.p += r.dir * t0;
			bounceVec = Normalize(r.p);
			r.p = bounceVec * K4*radius;  
		//compute dt used
			deltaT = t0/velDist;
		}

		if (deltaT > 1.0f) deltaT = 1.0f;
		at = deltaT * dt;
		hitPoint = r.p * tm;

		if (Vc[2] < 0.5f) // perfect sphere
		{
			//compute bounce and friction direction
			bounceVec *= -velDist*DotProd(bounceVec,r.dir);
			frictionVec = relativeVel + bounceVec;
		
			norm = VectorTransform(tm,bounceVec)/dt;
			friction = VectorTransform(tm,frictionVec)/(dt*varK2);
		}
		else // ellipsoid
		{
			Point3 axis2, axis3;
			RefillAxes(bounceVec, axis2, axis3);
			axis2 = VectorTransform(tm, axis2);
			axis3 = VectorTransform(tm, axis3);
			bounceVec = Normalize(axis2^axis3);
						
			norm = (-DotProd(bounceVec, vel))*bounceVec;
			friction = (vel + norm)/K2;
		}

		inheritedVel = Point3::Origin;
		return TRUE;
	} // end of static deflector

	Point3 posRev, curP, normUnit, normUnitWorldSpace;
	float vertOffset;

//--------------------------------------------------------------//
//			perfect sphere: no size change						//
//--------------------------------------------------------------//
	if (Vc[2] < 1.5f)
	{
		posRev = pos + vel*(dt-Vc[0]);
		Matrix3 adjPrevInvTm;
		if (Vc[2] > 0.5f) // deflector is rotating
		{  // remove rotation component while calculating relative speed
			// Bayboro: the matrix should be precalculated; but I don't have
			// such a luxury for 4.x
			adjPrevInvTm = Inverse(prevInvTm);
			adjPrevInvTm.SetRow(0, tm.GetRow(0));
			adjPrevInvTm.SetRow(1, tm.GetRow(1));
			adjPrevInvTm.SetRow(2, tm.GetRow(2));
			adjPrevInvTm = Inverse(adjPrevInvTm);

			posRev = posRev * adjPrevInvTm;
		}
		else 
			posRev = posRev * prevInvTm;

		curP = (pos + vel*dt) * invtm;
		prevP = posRev + (curP - posRev)*(Vc[0] - dt)/Vc[0];
		relativeVel = curP - prevP;

		distSquared = DotProd(prevP,prevP);
		velDist = Length(relativeVel);
		if (velDist == 0.0f) return FALSE; // static particle
	
		r.p = prevP;
		r.dir = relativeVel/velDist;
	//intersect sphere
		if (!IntersectSphere(r,t0,t1,radiusSquared)) return FALSE;

		//check if inside	
		if (distSquared <= radiusSquared)
		{
			if (t1*K3 > velDist) return FALSE;
		//compute surface normal
			normUnit = -Normalize(r.p + r.dir * t1);
			varK2 = VarK2(radiusSquared, relativeVel/dt);
		//compute dt used
			deltaT = t1/velDist;
		}
		//else outside
		else
		{
			if ((t0*K3 > velDist) || (t0 < 0.0f)) return FALSE;
		//compute surface normal
			normUnit = Normalize(r.p + r.dir * t0);
			varK2 = K2;
		//compute dt used
			deltaT = t0/velDist;
		}

		if (deltaT > 1.0f) deltaT = 1.0f;


		at = deltaT * dt;
		hitPoint = pos + at*vel;
		float inherit = inheritedVel.x;
		inheritedVel = (hitPoint*prevInvTm*tm - hitPoint)/Vc[0];
		//compute bounce and friction direction
		Point3 corrRelVel = VectorTransform(invtm, dt*(vel - inheritedVel));
		if (inherit < 1.0f) 
			corrRelVel += (1.0f-inherit)*VectorTransform(invtm, dt*inheritedVel);

		bounceVec = -DotProd(normUnit, corrRelVel)*normUnit;
		frictionVec = corrRelVel + bounceVec;

		normUnitWorldSpace = VectorTransform(tm, normUnit);
		// to lift the hit point above the ground
		hitPoint += (1.0f-varK2)*radius*normUnitWorldSpace;

		vertOffset = DotProd(normUnitWorldSpace, inheritedVel);
		// Bayboro: in the original version the deflector had the following
		// condition on the inheritedVel. I think it's better to comment
		// it for the property consistency
		// if (vertOffset < 0.0f) inheritedVel = Point3::Origin;
		if (vertOffset > 0.0f) hitPoint += vertOffset*normUnitWorldSpace;
		else vertOffset = 0.0f;

		norm = VectorTransform(tm,bounceVec)/dt;
		friction = VectorTransform(tm,frictionVec)/dt;
		if (distSquared <= radiusSquared) // only for inside particles to compensate "curve slowdown"
			friction /= K2 - vertOffset/radius;

		return TRUE;
	} // end of "no radius change" case

//--------------------------------------------------------------//
//			not-so-perfect sphere with possible size change		//
//			iterative solution
//--------------------------------------------------------------//

	// we are going to calcuate a lot of stuff here that could be
	// precalculated, but it has to wait for 4.y
	TMInterpolater tmi;
	tmi.Init(Inverse(prevInvTm),tm);
	float timeLo = 1.0f - dt/Vc[0];
	float timeHi = 1.0f;
	Matrix3 tmLo = tmi.Interpolate(timeLo);
	Matrix3 tmHi = tmi.Interpolate(timeHi);

	Matrix3 invTmLo = Inverse(tmLo);
	Matrix3 invTmHi = Inverse(tmHi);

	Point3 posLo = pos * invTmLo;
	Point3 posHi = (pos + vel*dt) * invTmHi;

	float distSqLo = DotProd( posLo, posLo);
	float distSqHi = DotProd( posHi, posHi);
	
	bool wasAbove = (distSqLo > radiusSquared);
	bool nowAbove = (distSqHi > radiusSquared);
	bool crossSphere, midAbove;
	float timeMd, distSqMd, timeLocal;
	Matrix3 tmMd, invTmMd;
	Point3 posMd;

	if (wasAbove == nowAbove)
	{
		crossSphere = false; // preliminary but needs further investigation
		timeMd = 0.5f*(timeLo+timeHi);
		timeLocal = dt-Vc[0]*(1.0f-timeMd);
		tmMd = tmi.Interpolate(timeMd);
		invTmMd = Inverse(tmMd);
		posMd = (pos + vel*timeLocal) * invTmMd;
		distSqMd = DotProd( posMd, posMd);

		midAbove = (distSqMd > radiusSquared);
		
		if (midAbove == wasAbove)
		{ // still needs further verification;
			float m = float(sqrt(distSqMd) - sqrt(distSqHi));
			float l = float(sqrt(distSqLo) - sqrt(distSqHi));
			float a = m - l*0.5f;
			if (fabs(a) > 0.001f) // if a is too small then we have "almost" linear path
			{
				float b = 0.25f*l - m;
				float exT = -0.5f*b/a;
				if ((exT > 0.001f) && (exT < 0.999f))
				{
					timeHi = 1.0f - exT*dt/Vc[0];
					timeLocal = dt-Vc[0]*(1.0f-timeHi);
					tmHi = tmi.Interpolate(timeHi);
					invTmHi = Inverse(tmHi);
					posHi = (pos + vel*timeLocal) * invTmHi;
					distSqHi = DotProd( posHi, posHi);
					nowAbove = (distSqHi > radiusSquared);
					crossSphere = (wasAbove != nowAbove);
				}
			}
		}
		else
		{
			nowAbove = midAbove;
			timeHi = timeMd;
			tmHi = tmMd;
			invTmHi = invTmMd;
			distSqHi = distSqMd;
			posHi = posMd;
			crossSphere = true;
		}
	}
	else
		crossSphere = true;

	// check out if the end point is close enouth to sphere for collision
	if (!crossSphere) 
		if (fabs(1.0f - sqrt(distSqMd/radiusSquared)) >= K1) // not close enough
			return FALSE;

	// check if particle is too fast for the given radius
	// slow down particle if needed (only for inside particles)
	bool particleIsFast = FALSE;
	float slowDownCoef = 1.0f;
	if (!wasAbove)
	{
		float localSpeedLen = Vc[0]*Length(VectorTransform(invtm, vel));
		if (particleIsFast = (localSpeedLen > radius))
			slowDownCoef = radius/localSpeedLen;
	}

	// now find the moment when particle is as above as "wasAbove" but with
	// distance to the sphere is less than K1*radius
	for(int i=0; i<20; i++) // 0.000001f precision
	{
		timeMd = 0.5f*(timeLo+timeHi);
		timeLocal = dt-Vc[0]*(1.0f-timeMd);
		tmMd = tmi.Interpolate(timeMd);
		invTmMd = Inverse(tmMd);
		posMd = (pos + vel*timeLocal) * invTmMd;
		distSqMd = DotProd( posMd, posMd);
		midAbove = (distSqMd > radiusSquared);
		if (midAbove == wasAbove)
		{
			// check out if the mid point is close enough to sphere
			if (fabs(1.0f - sqrt(distSqMd/radiusSquared)) < K1) // close enough
				break;
			timeLo = timeMd;
			tmLo = tmMd;
			invTmLo = invTmMd;
			posLo = posMd;
			distSqLo = distSqMd;
		}
		else
		{
			timeHi = timeMd;
			tmHi = tmMd;
			invTmHi = invTmMd;
			posHi = posMd;
			distSqHi = distSqMd;
		}
	}

	if (wasAbove)
	{
		bounceVec = Normalize(posMd);
		r.p = bounceVec * K4*radius;
	}
	else
	{
		bounceVec = -Normalize(posMd);
		varK2 = VarK2(radiusSquared, VectorTransform(tm,vel));
		r.p = -bounceVec * varK2*radius;
	}

	at = dt - Vc[0]*(1-timeMd);
	hitPoint = r.p * tmMd;

	if (Vc[2] < 2.5f) // perfect sphere
	{
		bounceVec = Normalize(VectorTransform(tmMd, bounceVec));
	}
	else // ellipsoid
	{
		Point3 axis2, axis3;
		RefillAxes(bounceVec, axis2, axis3);
		axis2 = VectorTransform(tmMd, axis2);
		axis3 = VectorTransform(tmMd, axis3);
		bounceVec = Normalize(axis2^axis3);
	}

	float inherit = inheritedVel.x;
	inheritedVel = (hitPoint*prevInvTm*tm - hitPoint)/Vc[0];
	Point3 corrVel = vel - inheritedVel;
	if (inherit < 1.0f)
		corrVel += (1.0f-inherit)*inheritedVel;

	norm = (-DotProd(bounceVec, corrVel))*bounceVec;
	friction = corrVel + norm;

	vertOffset = DotProd(bounceVec, inheritedVel);
	if (vertOffset > 0.0f) hitPoint += vertOffset*bounceVec;
	else vertOffset = 0.0f;

	if (!wasAbove) friction /= K2 - vertOffset/radius;

	if (particleIsFast)
	{
		norm *= slowDownCoef;
		friction *= slowDownCoef;
	}

	return TRUE;
}

