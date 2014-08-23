/**********************************************************************
 *<
    FILE: crt_pnt.cpp

    DESCRIPTION:  Test Utility for the API

    CREATED BY: Charlie Thaeler

    HISTORY: created 18 Feb, 1998

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "nutil.h"





int
APITestUtil::MakeTestPoint(NURBSSet &nset)
{
	NURBSIndependentPoint *p = new NURBSIndependentPoint();
	p->SetName(GetString(IDS_POINT));
	p->SetPosition(0, Point3(75, -75, 0));

	return nset.AppendObject(p);
}

int
APITestUtil::MakeTestPointCPoint(NURBSSet &nset, int p1)
{
	NURBSPointConstPoint *p = new NURBSPointConstPoint();
	p->SetName(GetString(IDS_POINT_CONST_POINT));
	p->SetParent(p1);
	p->SetPointType(kNConstOffset);
	p->SetOffset(0, Point3(20, -20, 20));

	return nset.AppendObject(p);
}


int
APITestUtil::MakeTestCurveCPoint(NURBSSet &nset, int p1)
{
	NURBSCurveConstPoint *p = new NURBSCurveConstPoint();
	p->SetName(GetString(IDS_CURVE_CONST_POINT));
	p->SetParent(p1);
	p->SetUParam(0, 0.5);
	p->SetPointType(kNConstNormal);
	p->SetNormal(0, 15.0f);

	return nset.AppendObject(p);
}


int
APITestUtil::MakeTestSurfCPoint(NURBSSet &nset, int p1)
{
	NURBSSurfConstPoint *p = new NURBSSurfConstPoint();
	p->SetName(GetString(IDS_SURF_CONST_POINT));
	p->SetParent(p1);
	p->SetUParam(0, 0.5);
	p->SetVParam(0, 0.6);
	p->SetPointType(kNConstTangent);
	p->SetUTangent(0, 15.0f);
	p->SetVTangent(0, 25.0f);

	return nset.AppendObject(p);
}

int
APITestUtil::MakeTestCurveCurve(NURBSSet &nset, int p1, int p2, BOOL trim)
{
	NURBSCurveCurveIntersectionPoint *p = new NURBSCurveCurveIntersectionPoint();
	p->SetName(GetString(IDS_CURVE_CURVE_POINT));
	p->SetNSet(&nset);

	p->SetParent(0, p1);
	p->SetParent(1, p2);
    p->SetCurveParam(0, 0.0);
    p->SetCurveParam(1, 0.0);
    p->SetTrimCurve(1, trim);

	return nset.AppendObject(p);
}

int
APITestUtil::MakeTestCurveSurface(NURBSSet &nset, int p1, int p2)
{
	NURBSCurveSurfaceIntersectionPoint *p = new NURBSCurveSurfaceIntersectionPoint();
	p->SetName(GetString(IDS_CURV_SURF_INT_POINT));
	p->SetNSet(&nset);

	p->SetParent(0, p1);
	p->SetParent(1, p2);

	p->SetSeed(0.5);

	return nset.AppendObject(p);
}

