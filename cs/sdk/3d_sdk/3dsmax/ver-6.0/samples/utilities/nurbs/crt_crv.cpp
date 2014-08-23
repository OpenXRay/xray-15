/**********************************************************************
 *<
    FILE: crt_crv.cpp

    DESCRIPTION:  Test Utility for the API

    CREATED BY: Charlie Thaeler

    HISTORY: created 18 Feb, 1998

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "nutil.h"


int
APITestUtil::MakeTestCVCurve(NURBSSet &nset, Matrix3 mat)
{
	NURBSCVCurve *c = new NURBSCVCurve();
	c->SetName(GetString(IDS_CV_CURVE));
	c->SetNumCVs(4);

	c->SetOrder(4);
	c->SetNumKnots(8);
	for (int k = 0; k < 4; k++) {
		c->SetKnot(k, 0.0);
		c->SetKnot(k+4, 1.0);
	}

	NURBSControlVertex cv;
	cv.SetSelected(TRUE); // make all the CVs selected
	cv.SetPosition(0, mat * Point3(0, 150, 50));
	c->SetCV(0, cv);
	cv.SetPosition(0, mat * Point3(-100, 150, 50));
	c->SetCV(1, cv);
	cv.SetPosition(0, mat * Point3(-100, 250, 50));
	c->SetCV(2, cv);
	cv.SetPosition(0, mat * Point3(0, 250, 50));
	c->SetCV(3, cv);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestPointCurve(NURBSSet &nset, Matrix3 mat)
{
	NURBSPointCurve *c = new NURBSPointCurve();
	c->SetName(GetString(IDS_POINT_CURVE));
	c->SetNumPts(4);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, mat * Point3(0, 150, 0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, mat * Point3(-100, 150, 0));
	c->SetPoint(1, pt);
	pt.SetPosition(0, mat * Point3(-100, 250, 0));
	c->SetPoint(2, pt);
	pt.SetPosition(0, mat * Point3(0, 250, 0));
	c->SetPoint(3, pt);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestBlendCurve(NURBSSet &nset, int p1, int p2)
{
	NURBSBlendCurve *c = new NURBSBlendCurve();
	c->SetName(GetString(IDS_BLEND_CURVE));
	c->SetNSet(&nset);

	c->SetParent(0, p1);
	c->SetParent(1, p2);
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestOffsetCurve(NURBSSet &nset, int p)
{
	NURBSOffsetCurve *c = new NURBSOffsetCurve();
	c->SetName(GetString(IDS_OFFSET_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
	c->SetDistance(0, 50.0);
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestXFormCurve(NURBSSet &nset, int p)
{
	NURBSXFormCurve *c = new NURBSXFormCurve();
	c->SetName(GetString(IDS_XFORM_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.SetTrans(Point3(10, 10, 100));
	c->SetXForm(0, mat);
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestMirrorCurve(NURBSSet &nset, int p)
{
	NURBSMirrorCurve *c = new NURBSMirrorCurve();
	c->SetName(GetString(IDS_MIRROR_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
	Matrix3 mat;
	mat.IdentityMatrix();
	c->SetXForm(0, mat);
	c->SetAxis(kMirrorX);
	c->SetDistance(0, 25.0);
	return nset.AppendObject(c);
}


int
APITestUtil::MakeTestFilletCurve(NURBSSet &nset)
{
	NURBSPointCurve *ec0 = new NURBSPointCurve();
	ec0->SetName(GetString(IDS_F_EDGE_CRV1));
	ec0->SetNumPts(2);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(0, 0, 0));
	ec0->SetPoint(0, pt);
	pt.SetPosition(0, Point3(100, -100, 0));
	ec0->SetPoint(1, pt);
	int f1 = nset.AppendObject(ec0);

	NURBSPointCurve *ec1 = new NURBSPointCurve();
	ec1->SetName(GetString(IDS_F_EDGE_CRV2));
	ec1->SetNumPts(2);

	pt.SetPosition(0, Point3(100, 0, 0));
	ec1->SetPoint(0, pt);
	pt.SetPosition(0, Point3(100, -100, 0));
	ec1->SetPoint(1, pt);
	int f2 = nset.AppendObject(ec1);

	NURBSFilletCurve *fc = new NURBSFilletCurve();
	fc->SetName(GetString(IDS_FILLET_CURVE));
	fc->SetParent(0, f1);
	fc->SetParent(1, f2);
	fc->SetRadius(0, 20.0);
	fc->SetEnd(0, TRUE);
	fc->SetEnd(1, TRUE);

	return nset.AppendObject(fc);
}



int
APITestUtil::MakeTestChamferCurve(NURBSSet &nset)
{
	NURBSPointCurve *ec0 = new NURBSPointCurve();
	ec0->SetName(GetString(IDS_C_EDGE_CRV1));
	ec0->SetNumPts(2);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(0, 0, 50));
	ec0->SetPoint(0, pt);
	pt.SetPosition(0, Point3(90, -80, 50));
	ec0->SetPoint(1, pt);
	int f1 = nset.AppendObject(ec0);

	NURBSPointCurve *ec1 = new NURBSPointCurve();
	ec1->SetName(GetString(IDS_C_EDGE_CRV2));
	ec1->SetNumPts(2);

	pt.SetPosition(0, Point3(100, 0, 50));
	ec1->SetPoint(0, pt);
	pt.SetPosition(0, Point3(90, -80, 50));
	ec1->SetPoint(1, pt);
	int f2 = nset.AppendObject(ec1);

	NURBSChamferCurve *fc = new NURBSChamferCurve();
	fc->SetName(GetString(IDS_CHAMFER_CURVE));
	fc->SetParent(0, f1);
	fc->SetParent(1, f2);
	fc->SetLength(0, 0, 20.0);
	fc->SetLength(0, 1, 30.0);
	fc->SetEnd(0, TRUE);
	fc->SetEnd(1, TRUE);

	return nset.AppendObject(fc);
}

int
APITestUtil::MakeTestIsoCurveU(NURBSSet &nset, int p)
{
	NURBSIsoCurve *c = new NURBSIsoCurve();
	c->SetName(GetString(IDS_U_ISO_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
	c->SetParam(0, 0.5);
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestIsoCurveV(NURBSSet &nset, int p)
{
	NURBSIsoCurve *c = new NURBSIsoCurve();
	c->SetName(GetString(IDS_V_ISO_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
    c->SetDirection(FALSE);
	c->SetParam(0, 0.5);
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestSurfaceEdgeCurve(NURBSSet &nset, int p)
{
	NURBSSurfaceEdgeCurve *c = new NURBSSurfaceEdgeCurve();
	c->SetName(GetString(IDS_SURFACE_EDGE_CURVE));
	c->SetNSet(&nset);

	c->SetParent(p);
	c->SetSeed(Point2(0.0f, 1.0f));
	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestProjectVectorCurve(NURBSSet &nset, int p1, int p2)
{
	NURBSProjectVectorCurve *c = new NURBSProjectVectorCurve();
	c->SetName(GetString(IDS_PROJ_VECT_CURVE));
	c->SetNSet(&nset);

	c->SetParent(0, p1);
	c->SetParent(1, p2);

	c->SetPVec(0, Point3(0, 0, 1));

	c->SetSeed(Point2(0.8, 0.8));

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestProjectNormalCurve(NURBSSet &nset, int p1, int p2)
{
	NURBSProjectNormalCurve *c = new NURBSProjectNormalCurve();
	c->SetName(GetString(IDS_PROJ_NORM_CURVE));
	c->SetNSet(&nset);

	c->SetParent(0, p1);
	c->SetParent(1, p2);

	c->SetSeed(Point2(0.2, 0.2));
	c->SetTrim(TRUE);

	return nset.AppendObject(c);
}
int
APITestUtil::MakeTestSurfSurfIntersectionCurve(NURBSSet &nset, int p1, int p2)
{
	NURBSSurfSurfIntersectionCurve *c = new NURBSSurfSurfIntersectionCurve();
	c->SetName(GetString(IDS_SURF_SURF_CURVE));
	c->SetNSet(&nset);

	c->SetParent(0, p1);
	c->SetParent(1, p2);

	c->SetSeed(Point2(0.8, 0.8));

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestCurveOnSurface(NURBSSet &nset, int p1)
{
	NURBSCurveOnSurface *c = new NURBSCurveOnSurface();
	c->SetName(GetString(IDS_CV_COS_CURVE));
	c->SetParent(p1);
	c->SetNumCVs(4);

	c->SetOrder(4);
	c->SetNumKnots(8);
	for (int k = 0; k < 4; k++) {
		c->SetKnot(k, 0.0);
		c->SetKnot(k+4, 1.0);
	}

	NURBSControlVertex cv;
	cv.SetSelected(TRUE);
	cv.SetPosition(0, Point3(0.0, 0.0, 0.0));
	c->SetCV(0, cv);
	cv.SetPosition(0, Point3(0.5, 0.25, 0.0));
	c->SetCV(1, cv);
	cv.SetPosition(0, Point3(0.25, 0.75, 0.0));
	c->SetCV(2, cv);
	cv.SetPosition(0, Point3(1.0, 1.0, 0.0));
	c->SetCV(3, cv);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestPointCurveOnSurface(NURBSSet &nset, int p1)
{
	NURBSPointCurveOnSurface *c = new NURBSPointCurveOnSurface();
	c->SetName(GetString(IDS_POINT_COS_CURVE));
	c->SetParent(p1);
	c->SetNumPts(4);

	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(0.0, 0.0, 0.0));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(0.5, 0.25, 0.0));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(0.25, 0.75, 0.0));
	c->SetPoint(2, pt);
	pt.SetPosition(0, Point3(1.0, 1.0, 0.0));
	c->SetPoint(3, pt);

	c->SetTrim(TRUE);

	return nset.AppendObject(c);
}

int
APITestUtil::MakeTestSurfaceNormalCurve(NURBSSet &nset, int p1)
{
	NURBSSurfaceNormalCurve *c = new NURBSSurfaceNormalCurve();
	c->SetName(GetString(IDS_SURF_NORM_CURVE));
	c->SetParent(p1);
	c->SetDistance(0, 25.0f);

	return nset.AppendObject(c);
}

