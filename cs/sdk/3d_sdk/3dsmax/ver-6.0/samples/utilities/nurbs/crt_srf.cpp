/**********************************************************************
 *<
    FILE: crt_srf.cpp

    DESCRIPTION:  Test Utility for the API

    CREATED BY: Charlie Thaeler

    HISTORY: created 18 Feb, 1998

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "nutil.h"



int
APITestUtil::MakeTestCVSurface(NURBSSet &nset, Matrix3 mat, BOOL rigid)
{
	NURBSCVSurface *s = new NURBSCVSurface();
	s->SetRigid(rigid);
	s->SetName(GetString(IDS_CV_SURFACE));
	s->SetNumCVs(4, 4);

	s->SetUOrder(4);
	s->SetVOrder(4);
	s->SetNumUKnots(8);
	s->SetNumVKnots(8);
	for (int k = 0; k < 4; k++) {
		s->SetUKnot(k, 0.0);
		s->SetVKnot(k, 0.0);
		s->SetUKnot(k+4, 1.0);
		s->SetVKnot(k+4, 1.0);
	}

	NURBSControlVertex cv;
	for (int u = 0; u < 4; u++) {
		float up = 100.0f * ((float)u/3.0f);
		for (int v = 0; v < 4; v++) {
			float vp = 100.0f * ((float)v/3.0f);
			cv.SetPosition(0, mat * Point3(-150.0f + up, -100.0f + vp, 0.0f));
			char name[20];
			sprintf(name, "%s[%d,%d]", GetString(IDS_CV), u, v);
			cv.SetName(name);
			s->SetCV(u, v, cv);
		}
	}
	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestPointSurface(NURBSSet &nset, Matrix3 mat)
{
	NURBSPointSurface *s = new NURBSPointSurface();
	s->SetName(GetString(IDS_POINT_SURFACE));
	s->SetNumPts(2, 2);
	NURBSIndependentPoint pt;
	pt.SetPosition(0, mat * Point3(20, 0, 0));
	char name[32];
	sprintf(name, "%p [%d,%d]", GetString(IDS_POINT), 0, 0);
	pt.SetName(name);
	s->SetPoint(0, 0, pt);
	pt.SetPosition(0, mat * Point3(20, 0, 100));
	sprintf(name, "%p [%d,%d]", GetString(IDS_POINT), 0, 1);
	pt.SetName(name);
	s->SetPoint(0, 1, pt);
	pt.SetPosition(0, mat * Point3(120, 0, 0));
	sprintf(name, "%p [%d,%d]", GetString(IDS_POINT), 1, 0);
	pt.SetName(name);
	s->SetPoint(1, 0, pt);
	pt.SetPosition(0, mat * Point3(120, 0, 100));
	sprintf(name, "%p [%d,%d]", GetString(IDS_POINT), 1, 1);
	pt.SetName(name);
	s->SetPoint(1, 1, pt);

	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestBlendSurface(NURBSSet &nset, int p1, int p2)
{
	NURBSBlendSurface *s = new NURBSBlendSurface();
	s->SetName(GetString(IDS_BLEND_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(0, p1);
	s->SetParent(1, p2);

	s->SetEdge(0, 1); // make it the High U edge

	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestOffsetSurface(NURBSSet &nset, int p)
{
	NURBSOffsetSurface *s = new NURBSOffsetSurface();
	s->SetName(GetString(IDS_OFFSET_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(p);
	s->SetDistance(0, 20.0);
	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestXFormSurface(NURBSSet &nset, int p)
{
	NURBSXFormSurface *s = new NURBSXFormSurface();
	s->SetName(GetString(IDS_XFORM_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(p);
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.SetTrans(Point3(-150, 150, 0));
	s->SetXForm(0, mat);
	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestMirrorSurface(NURBSSet &nset, int p)
{
	NURBSMirrorSurface *s = new NURBSMirrorSurface();
	s->SetName(GetString(IDS_MIRROR_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(p);
	Matrix3 mat;
	mat.IdentityMatrix();
	s->SetXForm(0, mat);
	s->SetAxis(kMirrorZ);
	s->SetDistance(0, 100.0);
	s->FlipNormals(TRUE);
	return nset.AppendObject(s);
}


int
APITestUtil::MakeTestRuledSurface(NURBSSet &nset, int p1, int p2)
{
	NURBSRuledSurface *s = new NURBSRuledSurface();
	s->SetName(GetString(IDS_RULED_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(0, p1);
	s->SetParent(1, p2);
	s->SetFlip(0, FALSE);
	s->SetFlip(1, FALSE);
	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestULoftSurface(NURBSSet &nset, int p1, int p2, int p3)
{
	NURBSULoftSurface *s = new NURBSULoftSurface();
	s->SetName(GetString(IDS_ULOFT_SURFACE));
	s->SetNSet(&nset);

	s->AppendCurve(p1, FALSE);
	s->AppendCurve(p2, FALSE);
	s->AppendCurve(p3, FALSE);
	s->FlipNormals(TRUE);
	return nset.AppendObject(s);
}


int
APITestUtil::MakeTestExtrudeSurface(NURBSSet &nset, int p1)
{
	NURBSExtrudeSurface *s = new NURBSExtrudeSurface();
	s->SetName(GetString(IDS_EXTRUDE_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(p1);
	s->SetDistance(0, 50);
	s->FlipNormals(TRUE);

	return nset.AppendObject(s);
}


static int
MakeCurveToLathe(NURBSSet &nset)
{
	NURBSPointCurve *c = new NURBSPointCurve();
	c->SetName(GetString(IDS_POINT_CURVE));
	c->SetNumPts(3);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(200, 200, 0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(250, 200, 100));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(200, 200, 150));
	c->SetPoint(2, pt);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestLatheSurface(NURBSSet &nset)
{
	int p = MakeCurveToLathe(nset);

	NURBSLatheSurface *s = new NURBSLatheSurface();
	s->SetName(GetString(IDS_LATHE_SURFACE));
	s->SetNSet(&nset);

	s->SetParent(p);
	Matrix3 mat = TransMatrix(Point3(200, 200, 0));
	s->SetAxis(0, mat);

	s->FlipNormals(TRUE);

	return nset.AppendObject(s);
}



static int
P1Curve(NURBSSet &nset, Matrix3 mat)
{
	NURBSPointCurve *c = new NURBSPointCurve();
	c->SetName(GetString(IDS_POINT_CURVE));
	c->SetNumPts(4);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, mat * Point3(1, 1, 0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, mat * Point3(-1, 1, 0));
	c->SetPoint(1, pt);
	pt.SetPosition(0, mat * Point3(-1, -1, 0));
	c->SetPoint(2, pt);
	pt.SetPosition(0, mat * Point3(1, -1, 0));
	c->SetPoint(3, pt);

	return nset.AppendObject(c);
}
static int
P2Curve(NURBSSet &nset, Matrix3 mat)
{
	NURBSPointCurve *c = new NURBSPointCurve();
	c->SetName(GetString(IDS_POINT_CURVE));
	c->SetNumPts(3);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, mat * Point3(0, 0, 0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, mat * Point3(0, 0, 20));
	c->SetPoint(1, pt);
	pt.SetPosition(0, mat * Point3(0, 0, 40));
	c->SetPoint(2, pt);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestUVLoftSurface(NURBSSet &nset)
{
	Matrix3 scale = ScaleMatrix(Point3(50, 50, 50));
	Matrix3 rotate = RotateZMatrix(PI/2.0f);
	int cu0 = P1Curve(nset, scale * TransMatrix(Point3(0, 0, 0)));
	int cu1 = P1Curve(nset, scale * TransMatrix(Point3(0, 0, 20)));
	int cu2 = P1Curve(nset, scale * TransMatrix(Point3(0, 0, 40)));

	int cv0 = P2Curve(nset, TransMatrix(Point3(50, 50, 0)));
	int cv1 = P2Curve(nset, TransMatrix(Point3(-50, 50, 0)));
	int cv2 = P2Curve(nset, TransMatrix(Point3(-50, -50, 0)));
	int cv3 = P2Curve(nset, TransMatrix(Point3(50, -50, 0)));

	NURBSUVLoftSurface *s = new NURBSUVLoftSurface();
	s->SetName(GetString(IDS_UVLOFT_SURFACE));
	s->SetNSet(&nset);
	s->AppendUCurve(cu0);
	s->AppendUCurve(cu1);
	s->AppendUCurve(cu2);

	s->AppendVCurve(cv0);
	s->AppendVCurve(cv1);
	s->AppendVCurve(cv2);
	s->AppendVCurve(cv3);

	return nset.AppendObject(s);
}

int
APITestUtil::MakeTest1RailSweepSurface(NURBSSet &nset)
{
	int rail = P1Curve(nset, ScaleMatrix(Point3(50, 50, 50)));
	int cross = P1Curve(nset, ScaleMatrix(Point3(5, 5, 5)) *
							RotateYMatrix(PI/2.0f) *
							TransMatrix(Point3(50, 50, 0)));
	NURBS1RailSweepSurface *s = new NURBS1RailSweepSurface();
	s->SetName(GetString(IDS_1RAIL_SURFACE));
	s->SetNSet(&nset);

	s->SetParentRail(rail);
	s->AppendCurve(cross, FALSE);
	s->SetParallel(FALSE);
	s->FlipNormals(TRUE);

	return nset.AppendObject(s);
}

int
APITestUtil::MakeTest2RailSweepSurface(NURBSSet &nset)
{
	int rail1 = P1Curve(nset, ScaleMatrix(Point3(70, 70, 70)));
	int rail2 = P1Curve(nset, ScaleMatrix(Point3(90, 90, 90)));
	int cross = P1Curve(nset, ScaleMatrix(Point3(5, 5, 5)) *
							RotateYMatrix(PI/2.0f) *
							TransMatrix(Point3(70, 70, 0)));
	NURBS2RailSweepSurface *s = new NURBS2RailSweepSurface();
	s->SetName(GetString(IDS_2RAIL_SURFACE));
	s->SetNSet(&nset);

	s->SetRailParent(0, rail1);
	s->SetRailParent(1, rail2);
	s->AppendCurve(cross, FALSE);

	return nset.AppendObject(s);
}

static int
COS1(NURBSSet &nset, int p1)
{
	NURBSPointCurveOnSurface *c = new NURBSPointCurveOnSurface();
	c->SetName(GetString(IDS_POINT_COS_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p1);
	c->SetNumPts(3);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(0.1, 0.1, 0.0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(0.25, 0.75, 0.0));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(0.9, 0.9, 0.0));
	c->SetPoint(2, pt);

	return nset.AppendObject(c);
}

static int
COS2(NURBSSet &nset, int p1)
{
	NURBSPointCurveOnSurface *c = new NURBSPointCurveOnSurface();
	c->SetName(GetString(IDS_POINT_COS_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p1);
	c->SetNumPts(3);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(0.1, 0.1, 0.0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(0.75, 0.25, 0.0));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(0.9, 0.9, 0.0));
	c->SetPoint(2, pt);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestMultiCurveTrimSurface(NURBSSet &nset)
{
	int srf = MakeTestCVSurface(nset, TransMatrix(Point3(100, 100, 150)));
	int cos1 = COS1(nset, srf);
	int cos2 = COS2(nset, srf);

	NURBSMultiCurveTrimSurface *s = new NURBSMultiCurveTrimSurface();
	s->SetName(GetString(IDS_MULTI_TRIM_SURFACE));
	s->SetNSet(&nset);

	s->SetSurfaceParent(srf);
	s->AppendCurve(cos1);
	s->AppendCurve(cos2);

    // Tell the surface to flip the orientation of the trim.
	s->SetFlipTrim(TRUE);

	return nset.AppendObject(s);
}

int
APITestUtil::MakeTestCapSurface(NURBSSet &nset)
{
	// NOT YET IMPLEMENTED
	return 0;
}

int
APITestUtil::MakeTestNBlendSurface(NURBSSet &nset)
{
	// NOT YET IMPLEMENTED
	return 0;
}
