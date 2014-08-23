 /**********************************************************************
 
	FILE: DistanceStuff.cpp

	DESCRIPTION:  Bones def methods to find distances from stuff

	CREATED BY: Peter Watje

	HISTORY: 8/5/98




 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/



#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include <process.h>


void BonesDefMod::BuildCacheThread(BoneModData *bmd, int start, int end, int BoneIndex, TimeValue t, ObjectState *os, ShapeObject *pathOb, Matrix3 ntm )
{
	for (int i=start; i<end; i++) 
		{
//get total distance
		float TotalDistance = 0.0f;
		Point3 p,BoneCenter;		

//		if (!bmd->VertexData[i]->modified)
			{

			p = os->obj->GetPoint(i);

			if (BoneData[BoneIndex].Node != NULL)
				{
				float Influence = 1.0f;
				Point3 l1,l2;

				float LineU = 0.0f;
				Point3 op,otan;
				int cid,sid;

				if (!bmd->VertexData[i]->IsModified())
					{
					GetEndPoints(bmd,t,l1, l2, BoneIndex);
					if (BoneData[BoneIndex].flags & BONE_SPLINE_FLAG)
						{
						Influence = SplineToPoint(p,
												  &BoneData[BoneIndex].referenceSpline,
												  LineU,op,otan,cid,sid,ntm);
						}
					else
						{
						Influence = LineToPoint(p,l1,l2,LineU);
						}
					}
				bmd->DistCache[i].dist = Influence;
				bmd->DistCache[i].u = LineU;
				bmd->DistCache[i].SubCurveIds =cid;
				bmd->DistCache[i].SubSegIds =sid;
				bmd->DistCache[i].Tangents =otan;
				bmd->DistCache[i].OPoints =op;
				}

			}
		}
	
}

class ThreadData
{
public:
BonesDefMod *b;
BoneModData *bmd;
int start,end;
int boneIndex;
TimeValue t;
ObjectState *os;
ShapeObject *pathOb;
Matrix3 ntm;
};
void CallBuildCacheThread();

void CallBuildCacheThread(void *data)
{
//DebugPrint("Thread from %d to %d\n",((ThreadData*)(data))->start,((ThreadData*)(data))->end);

((ThreadData*)(data))->b->BuildCacheThread(((ThreadData*)(data))->bmd,
					((ThreadData*)(data))->start,((ThreadData*)(data))->end,
					((ThreadData*)(data))->boneIndex, ((ThreadData*)(data))->t, ((ThreadData*)(data))->os,
					((ThreadData*)(data))->pathOb,
					((ThreadData*)(data))->ntm
					);
}

void BonesDefMod::BuildCache(BoneModData *bmd, int BoneIndex, TimeValue t, ObjectState *os)
{
if ( GetInMouseAbort() && ( (ModeBoneEndPoint == 0) || (ModeBoneEndPoint == 1)) )
	{
	}
else if (cacheValid)
	{
	if (BoneIndex == bmd->CurrentCachePiece) return;
	}


int nv = os->obj->NumPoints();

if (bmd->DistCache.Count() != nv) bmd->DistCache.SetCount(nv);
bmd->CurrentCachePiece = BoneIndex;
cacheValid = TRUE;

//DebugPrint("Building Cache\n");
int DoThread = 0;
if (DoThread)
	{
//get number of processors
	SYSTEM_INFO info;
	GetSystemInfo(  &info );  // address of system information 
    int numProcessors = info.dwNumberOfProcessors; 

//	numProcessors = 2; 
//break the number verts ont group

	ThreadData TData[8];
	int ct =0,w;
	w = nv/numProcessors;
	static HANDLE ThreadHandles[8];

	ShapeObject *pathOb = NULL;
//	ObjectState pos = BoneData[BoneIndex].Node->EvalWorldState(RefFrame);
//	pathOb = (ShapeObject*)pos.obj;

	Interval valid;
	Matrix3 ntm;
//watje 10-7-99 212059
	ntm = bmd->BaseTM*BoneData[BoneIndex].tm;

//	Matrix3 ntm = BoneData[BoneIndex].Node->GetObjTMBeforeWSM(RefFrame,&valid);
//	ntm =bmd->BaseTM * Inverse(ntm);


	for (int nThreads =0; nThreads < numProcessors;nThreads++)
		{
		TData[nThreads].start = ct;
		if (numProcessors == 1)
			TData[nThreads].end = nv;
		else if (nThreads == (numProcessors-1))
			TData[nThreads].end = nv;
		else TData[nThreads].end = ct+w;
		TData[nThreads].t = t;
		TData[nThreads].boneIndex = BoneIndex;
		TData[nThreads].os = os;
		TData[nThreads].b = this;
		TData[nThreads].bmd = bmd;
//		TData[nThreads].pathOb = pathOb;
		TData[nThreads].ntm = ntm;



		ct += w;
		ThreadHandles[nThreads] = (HANDLE) _beginthread(CallBuildCacheThread,0,(void *) &TData[nThreads]);
		}


	int ThredErr = WaitForMultipleObjects(numProcessors,ThreadHandles,TRUE,INFINITE);
	}
else
	{
/*
	ShapeObject *pathOb = NULL;
	if (BoneData[BoneIndex].flags & BONE_SPLINE_FLAG)
		{
		ObjectState pos = BoneData[BoneIndex].Node->EvalWorldState(RefFrame);
		pathOb = (ShapeObject*)pos.obj;
		}
*/
	Matrix3 ntm;
//watje 10-7-99 212059
	ntm = bmd->BaseTM*BoneData[BoneIndex].tm;

	for (int i=0; i<nv; i++) {
//get total distance
		float TotalDistance = 0.0f;
		Point3 p,BoneCenter;		
		if (!bmd->VertexData[i]->IsModified())
			{

			p = os->obj->GetPoint(i);

			if (BoneData[BoneIndex].Node != NULL)
				{
				float Influence = 1.0f;
				Point3 l1,l2;

				float LineU = 0.0f;
				Point3 op,otan;
				int cid,sid;

				if (!bmd->VertexData[i]->IsModified())
					{
					GetEndPoints(bmd,t,l1, l2, BoneIndex);
					if (BoneData[BoneIndex].flags & BONE_SPLINE_FLAG)
						{
						Influence = SplineToPoint(p,
												  &BoneData[BoneIndex].referenceSpline,
												  LineU,op,otan,cid,sid,ntm);

						}
					else
						{
						Influence = LineToPoint(p,l1,l2,LineU);
						}
					}

				bmd->DistCache[i].dist = Influence;
				bmd->DistCache[i].u = LineU;
				bmd->DistCache[i].SubCurveIds =cid;
				bmd->DistCache[i].SubSegIds =sid;
				bmd->DistCache[i].Tangents =otan;
				bmd->DistCache[i].OPoints =op;
				}

			}
		}
	}


}



void BonesDefMod::RecurseDepth(float u1, float u2, float &fu,  Spline3D *s,int Curve,int Piece, int &depth, Point3 fp)
{



for (int i = 0; i < depth; i++)
	{
	float u = (u1+u2)*.5f;
	float midu = (u2-u1)*.25f;
	float tu1 = u - midu; 
	float tu2 = u + midu;
	Point3 p1, p2;
	p1 = s->InterpBezier3D(Piece, tu1);
	p2 = s->InterpBezier3D(Piece, tu2);



	if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
		u1 = u1;
		u2 = u;
		}
	else
		{
		u1 = u;
		u2 = u2;
		}

	}
fu = (u2+u1)*0.5f;
}

void BonesDefMod::PointToPiece(float &tempu,Spline3D *s,int Curve,int Piece, int depth, Point3 fp)

{
//float tu1,tu2,tu3,tu4;
float tu;
float su,eu;
int depth1;

depth1 = depth;

su = 0.0f;
eu = 0.25f;

float fdist = BIGFLOAT;
float fu = 0.0f;

for (int i = 0; i < 4; i++)
	{
	tu = 0.0f;
	depth = depth1;
	RecurseDepth(su,eu,tu,s,Curve,Piece,depth,fp);
	su += 0.25f;
	eu += 0.25f;
	Point3 dp = s->InterpBezier3D(Piece, tu);
	float dist = LengthSquared(fp-dp);
	if (dist<fdist)
		{
		fdist = dist;
		fu = tu;
		}
	}


tempu = fu;
//return fu;
}


float BonesDefMod::SplineToPoint(Point3 p1, 
								 Spline3D *s, 
								 float &finalu, Point3 &op, Point3 &otan, int &cid, int &sid, Matrix3 tm)

{

//brute force for now
p1 = p1 * tm;

int rec_depth = 5;

int piece_count = 0;
float fdist = BIGFLOAT;
int i = 0;
//for (int i = 0; i < s->NumberOfCurves(); i++)
	{
	for (int j = 0; j < s->Segments(); j++)
		{
		float u;
		PointToPiece(u,s,i,j,rec_depth,p1);
		Point3 dp = s->InterpBezier3D( j, u);
		float dist = LengthSquared(p1-dp);
		if (dist<fdist)
			{
			fdist = dist;
			finalu = u;
			op = dp;
			otan = s->TangentBezier3D(j,finalu);
			cid = i;
			sid = j;
			}
		}
	}

return (float)sqrt(fdist);
}



float BonesDefMod::LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u)
{
Point3 VectorA,VectorB,VectorC;
double Angle;
double dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
if (dot == 1.0f) dot = 0.99f;
Angle =  acos(dot);
if (Angle > (3.14/2.0))
	{
	dist = Length(p1-l1);
	u = 0.0f;
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot == 1.0f) dot = 0.99f;
	Angle = acos(dot);
	if (Angle > (3.14/2.0))
		{
		dist = Length(p1-l2);
		u = 1.0f;
		}
		else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;
		double du =  (cos(Angle) * hyp);
		double a = Length(VectorA);
		if ( a== 0.0f)
			return 0.0f;
		else u = (float)((a-du) / a);

		}

	}

return (float) dist;

}



void BonesDefMod::ComputeFalloff(float &u, int ftype)

{

switch (ftype)
	{
	case (BONE_FALLOFF_X3_FLAG) : u = u*u*u; break;
	case (BONE_FALLOFF_X2_FLAG) : u = u*u; break;
	case (BONE_FALLOFF_X_FLAG) : u = u; break;
	case (BONE_FALLOFF_SINE_FLAG) : u = 1.0f-((float)cos(u*PI) + 1.0f)*0.5f; break;
	case (BONE_FALLOFF_2X_FLAG) : u = (float) sqrt(u); break;
	case (BONE_FALLOFF_3X_FLAG) : u = (float) pow(u,0.3); break;

	}

}
