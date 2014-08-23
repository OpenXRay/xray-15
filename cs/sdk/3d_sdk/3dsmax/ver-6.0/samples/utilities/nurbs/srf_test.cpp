/**********************************************************************
 *<
    FILE: sdk_test.cpp

    DESCRIPTION:  Test Utility for the API

    CREATED BY: Charlie Thaeler

    HISTORY: created 24 July, 1997

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "nutil.h"

static HWND hPanel;

static APITestUtilDesc sAPITestUtilDesc;

ClassDesc*
GetAPITestUtilDesc()
{
	return &sAPITestUtilDesc;
}

const TCHAR *
APITestUtilDesc::ClassName()
{
	return GetString(IDS_API_TEST);
}

const TCHAR *
APITestUtilDesc::Category()
{
	return GetString(IDS_NURBS);
}












static int
AddTestPointSurface(NURBSSet &nset)
{
	NURBSPointSurface *s = new NURBSPointSurface();
	s->SetName(GetString(IDS_ADDED_POINT_SURF));
	s->SetNumPts(2, 2);
	NURBSIndependentPoint pt;
	pt.SetPosition(0, Point3(20, 0, -100));
	s->SetPoint(0, 0, pt);
	pt.SetPosition(0, Point3(20, 0, -200));
	s->SetPoint(0, 1, pt);
	pt.SetPosition(0, Point3(120, 0, -100));
	s->SetPoint(1, 0, pt);
	pt.SetPosition(0, Point3(120, 0, -200));
	s->SetPoint(1, 1, pt);

	s->FlipNormals(TRUE);

	return nset.AppendObject(s);
}


static int
AddTestIsoCurve(NURBSSet &nset, NURBSId id)
{
	NURBSIsoCurve *c = new NURBSIsoCurve();
	c->SetName(GetString(IDS_ADDED_ISO_CURVE));
	c->SetNSet(&nset);

	c->SetParentId(id);
	c->SetParam(0, 0.6);
	c->SetDirection(FALSE); // not U
	return nset.AppendObject(c);
}


static void
AddObjectsForJoinTests(NURBSSet &nset, int &c1, int &c2, int &s1, int &s2)
{
	NURBSIndependentPoint pt;

	NURBSPointCurve *c = new NURBSPointCurve();
	c->SetName(GetString(IDS_J_PT_CRV1));
	c->SetNumPts(3);
	pt.SetPosition(0, Point3(100, 0, -100));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(200, 0, -100));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(200, 100, -100));
	c->SetPoint(2, pt);
	c1 = nset.AppendObject(c);

	c = new NURBSPointCurve();
	c->SetName(GetString(IDS_J_PT_CRV2));
	c->SetNumPts(3);
	pt.SetPosition(0, Point3(210, 110, -100));
	c->SetPoint(0, pt);
	pt.SetPosition(0, Point3(300, 200, -100));
	c->SetPoint(1, pt);
	pt.SetPosition(0, Point3(300, 100, -100));
	c->SetPoint(2, pt);
	c2 = nset.AppendObject(c);

	NURBSPointSurface *s = new NURBSPointSurface();
	s->SetName(GetString(IDS_J_PT_SRF1));
	s->SetNumPts(2, 2);
	pt.SetPosition(0, Point3(200, 0, -100));
	s->SetPoint(0, 0, pt);
	pt.SetPosition(0, Point3(200, 100, -100));
	s->SetPoint(0, 1, pt);
	pt.SetPosition(0, Point3(300, 0, -100));
	s->SetPoint(1, 0, pt);
	pt.SetPosition(0, Point3(300, 100, -100));
	s->SetPoint(1, 1, pt);
	s1 = nset.AppendObject(s);

	s = new NURBSPointSurface();
	s->SetName(GetString(IDS_J_PT_SRF2));
	s->SetNumPts(2, 2);
	pt.SetPosition(0, Point3(310, 0, -100));
	s->SetPoint(0, 0, pt);
	pt.SetPosition(0, Point3(310, 100, -100));
	s->SetPoint(0, 1, pt);
	pt.SetPosition(0, Point3(400, 0, -100));
	s->SetPoint(1, 0, pt);
	pt.SetPosition(0, Point3(400, 100, -100));
	s->SetPoint(1, 1, pt);
	s2 = nset.AppendObject(s);
}

static void
AddObjectsForBreakTests(NURBSSet &nset, int &c1, int &s1)
{
	NURBSCVCurve *c = new NURBSCVCurve();
	c->SetName(GetString(IDS_BREAK_CURVE));
	c->SetNumCVs(4);

	c->SetOrder(4);
	c->SetNumKnots(8);
	for (int k = 0; k < 4; k++) {
		c->SetKnot(k, 0.0);
		c->SetKnot(k+4, 1.0);
	}

	NURBSControlVertex cv;
	cv.SetPosition(0, Point3(200, 0, 50));
	c->SetCV(0, cv);
	cv.SetPosition(0, Point3(300, 0, 50));
	c->SetCV(1, cv);
	cv.SetPosition(0, Point3(300, -100, 50));
	c->SetCV(2, cv);
	cv.SetPosition(0, Point3(200, -100, 50));
	c->SetCV(3, cv);

	c1 = nset.AppendObject(c);
	NURBSCVSurface *s = new NURBSCVSurface();
	s->SetName(GetString(IDS_BREAK_SURFACE));
	s->SetNumCVs(4, 4);

	s->SetUOrder(4);
	s->SetVOrder(4);
	s->SetNumUKnots(8);
	s->SetNumVKnots(8);
	for (k = 0; k < 4; k++) {
		s->SetUKnot(k, 0.0);
		s->SetVKnot(k, 0.0);
		s->SetUKnot(k+4, 1.0);
		s->SetVKnot(k+4, 1.0);
	}

	for (int u = 0; u < 4; u++) {
		float up = 100.0f * ((float)u/3.0f);
		for (int v = 0; v < 4; v++) {
			float vp = 100.0f * ((float)v/3.0f);
			cv.SetPosition(0, Point3(-150.0f + up, -100.0f + vp, -200.0f));
			s->SetCV(u, v, cv);
		}
	}
	s1 = nset.AppendObject(s);
}


void
APITestUtil::CombinedTests()
{
	// now let's build a test object
	NURBSSet nset;
	Matrix3 mat;
	mat.IdentityMatrix();


	// build an independent point
	int indPnt = MakeTestPoint(nset);

	// now a constrained point
	int ptPnt = MakeTestPointCPoint(nset, indPnt);



	// build a cv curve
	int cvCrv = MakeTestCVCurve(nset, mat);

	// and a constrained point on that curve
	int crvPnt = MakeTestCurveCPoint(nset, cvCrv);

	// now a point curve
	int ptCrv = MakeTestPointCurve(nset, mat);

	// Blend the two curves
	int blendCrv = MakeTestBlendCurve(nset, cvCrv, ptCrv);

	// make an offset of the CV curve
	int offCrv = MakeTestOffsetCurve(nset, cvCrv);

	// make a Transform curve of the point curve
	int xformCrv = MakeTestXFormCurve(nset, ptCrv);

	// make a mirror of the blend
	int mirCrv = MakeTestMirrorCurve(nset, blendCrv);

	// make a fillet curve (It makes it's own point curves to fillet)
	int fltCrv = MakeTestFilletCurve(nset);

	// make a chamfer curve (It makes it's own point curves to fillet)
	int chmCrv = MakeTestChamferCurve(nset);




	// build a cv surface
	int cvSurf = MakeTestCVSurface(nset, mat);


	// and a constrained point on that surface
	int srfPnt = MakeTestSurfCPoint(nset, cvSurf);

	// Curve Surface intersection point.
	int cvCrv2 = MakeTestCVCurve(nset, RotateXMatrix(PI/2.0f) * TransMatrix(Point3(0, 0, -175)));
	int srfIntPoint = MakeTestCurveSurface(nset, cvSurf, cvCrv2);

	// Now an Iso Curve on the CV surface
	int isoCrv1 = MakeTestIsoCurveU(nset, cvSurf);

	// Now an Iso Curve on the CV surface
	int isoCrv2 = MakeTestIsoCurveV(nset, cvSurf);

	// Now a Surface Edge Curve on the CV surface
	int surfEdgeCrv = MakeTestSurfaceEdgeCurve(nset, cvSurf);

	// build a CV Curve on Surface
	int cvCOS = MakeTestCurveOnSurface(nset, cvSurf);

	// build a Point Curve on Surface
	int pntCOS = MakeTestPointCurveOnSurface(nset, cvSurf);

	// build a Surface Normal Offset Curve
	int cnoCrf = MakeTestSurfaceNormalCurve(nset, cvCOS);

    // Make a curve-curve intersection point
    int curveCurve = MakeTestCurveCurve(nset, isoCrv1, isoCrv2, TRUE);

	// build a point surface
	int ptSurf = MakeTestPointSurface(nset, mat);

	// Blend the two surfaces
	int blendSurf = MakeTestBlendSurface(nset, cvSurf, ptSurf);

	// Offset of the blend
	int offSurf = MakeTestOffsetSurface(nset, blendSurf);

	// Transform of the Offset
	int xformSurf = MakeTestXFormSurface(nset, offSurf);

	// Mirror of the transform surface
	int mirSurf = MakeTestMirrorSurface(nset, xformSurf);

	// Make a Ruled surface between two curves
	int rulSurf = MakeTestRuledSurface(nset, cvCrv, ptCrv);

	// Make a ULoft surface
	int uLoftSurf = MakeTestULoftSurface(nset, ptCrv, offCrv, xformCrv);

	// Make a Extrude surface
	int extSurf = MakeTestExtrudeSurface(nset, xformCrv);

	// Make a lathe
	int lthSurf = MakeTestLatheSurface(nset);

	// these will build their own curves to work with
	// UV Loft
	int uvLoftSurf = MakeTestUVLoftSurface(nset);


	// 1 Rail Sweep
	int oneRailSurf = MakeTest1RailSweepSurface(nset);

	// 2 Rail Sweep
	int twoRailSurf = MakeTest2RailSweepSurface(nset);

	// MultiCurveTrim Surface
	int multiTrimSurf = MakeTestMultiCurveTrimSurface(nset);


	// Now make the curves and surfaces that we'll use later for the join tests
	int jc1, jc2, js1, js2;
	AddObjectsForJoinTests(nset, jc1, jc2, js1, js2);

	int bc, bs;
	AddObjectsForBreakTests(nset, bc, bs);


	Object *obj = CreateNURBSObject(mpIp, &nset, mat);
	INode *node = mpIp->CreateObjectNode(obj);
	node->SetName(GetString(IDS_TEST_OBJECT));




	NURBSSet addNset;
	// build a point surface
	int addptSurf = AddTestPointSurface(addNset);

	// add an iso curve to the previously created CV Surface
	NURBSId id = nset.GetNURBSObject(cvSurf)->GetId();
	int addIsoCrv = AddTestIsoCurve(addNset, id);

	AddNURBSObjects(obj, mpIp, &addNset);




	// now test some changing functionality
	// Let's change the name of the CVSurface
	NURBSObject* nObj = nset.GetNURBSObject(cvSurf);
	nObj->SetName(_T("New CVSurf Name"));  // testing only, no need to localize

	// now let's change the position of one of the points in the point curve
	NURBSPointCurve* ptCrvObj = (NURBSPointCurve*)nset.GetNURBSObject(ptCrv);
	ptCrvObj->GetPoint(0)->SetPosition(0, Point3(10, 160, 0)); // moved from 0,150,0

	// now let's change the position and weight of one of the CVs
	// in the CV Surface
	NURBSCVSurface* cvSurfObj = (NURBSCVSurface*)nset.GetNURBSObject(cvSurf);
	cvSurfObj->GetCV(0, 0)->SetPosition(0, Point3(-150.0, -100.0, 20.0)); // moved from 0,0,0
	cvSurfObj->GetCV(0, 0)->SetWeight(0, 2.0); // from 1.0


	// now let's do a transform of a curve.
	NURBSIdTab xfmTab;
	NURBSId nid = nset.GetNURBSObject(jc1)->GetId();
	xfmTab.Append(1, &nid);
	Matrix3 xfmMat;
	xfmMat = TransMatrix(Point3(10, 10, -10));
	SetXFormPacket xPack(xfmMat);
	NURBSResult res = Transform(obj, xfmTab, xPack, xfmMat, 0);




	// Now let's Join two curves
	NURBSId jc1id = nset.GetNURBSObject(jc1)->GetId(),
			jc2id = nset.GetNURBSObject(jc2)->GetId();
	JoinCurves(obj, jc1id, jc2id, FALSE, TRUE, 20.0, 1.0f, 1.0f, 0);

	// Now let's Join two surfaces
	NURBSId js1id = nset.GetNURBSObject(js1)->GetId(),
			js2id = nset.GetNURBSObject(js2)->GetId();
	JoinSurfaces(obj, js1id, js2id, 1, 0, 20.0, 1.0f, 1.0f, 0);

	// Break a Curve
	NURBSId bcid = nset.GetNURBSObject(bc)->GetId();
	BreakCurve(obj, bcid, .5, 0);

	// Break a Surface
	NURBSId bsid = nset.GetNURBSObject(bs)->GetId();
	BreakSurface(obj, bsid, TRUE, .5, 0);

	mpIp->RedrawViews(mpIp->GetTime());
	nset.DeleteObjects();
	addNset.DeleteObjects();


	// now do a detach
	NURBSSet detset;
	Matrix3 detmat;
	detmat.IdentityMatrix();
	// build a cv curve
	int detcvCrv = MakeTestCVCurve(detset, detmat);

	// now a point curve
	int detptCrv = MakeTestPointCurve(detset, detmat);

	// Blend the two curves
	int detblendCrv = MakeTestBlendCurve(detset, detcvCrv, detptCrv);

	Object *detobj = CreateNURBSObject(mpIp, &detset, detmat);
	INode *detnode = mpIp->CreateObjectNode(detobj);
	detnode->SetName("Detach From");

	BOOL copy = TRUE;
	BOOL relational = TRUE;
	NURBSIdList detlist;
	NURBSId oid = detset.GetNURBSObject(detblendCrv)->GetId();
	detlist.Append(1, &oid);
	DetachObjects(GetCOREInterface()->GetTime(), detnode, detobj,
					detlist, "Detach Test", copy, relational);
	mpIp->RedrawViews(mpIp->GetTime());

}

void
APITestUtil::PointTests()
{
	// now let's build a test object
	NURBSSet nset;
	Matrix3 mat;
	mat.IdentityMatrix();


	// 6 types of points...

	// 1: build an independent point
	int indPnt = MakeTestPoint(nset);


	// 2: now a constrained point
	int ptPnt = MakeTestPointCPoint(nset, indPnt);



	// build a cv curve
	int cvCrv = MakeTestCVCurve(nset, mat);

	// 3: a constrained point on that curve
	int crvPnt = MakeTestCurveCPoint(nset, cvCrv);



	// build a cv surface
	int cvSurf = MakeTestCVSurface(nset, mat);

	// 4: a constrained point on that surface
	int srfPnt = MakeTestSurfCPoint(nset, cvSurf);

	// 5: Curve Curve intersection point
	int cvCrv1 = MakeTestCVCurve(nset, TransMatrix(Point3(65, 0, 0)) * RotateZMatrix(0.5));
	int intPoint = MakeTestCurveCurve(nset, cvCrv, cvCrv1, FALSE);

	// 6: Curve Surface intersection point.
	int cvCrv2 = MakeTestCVCurve(nset, RotateXMatrix(PI/2.0f) * TransMatrix(Point3(0, 0, -175)));
	int srfIntPoint = MakeTestCurveSurface(nset, cvSurf, cvCrv2);


	Object *obj = CreateNURBSObject(mpIp, &nset, mat);
	INode *node = mpIp->CreateObjectNode(obj);
	node->SetName(GetString(IDS_PNT_TEST_OBJECT));
	mpIp->RedrawViews(mpIp->GetTime());
}


void
APITestUtil::CurveTests()
{
	// now let's build a test object
	NURBSSet nset;
	Matrix3 mat;
	mat.IdentityMatrix();

	// build a cv curve
	int cvCrv = MakeTestCVCurve(nset, mat);

	// now a point curve
	int ptCrv = MakeTestPointCurve(nset, mat);

	// Blend the two curves
	int blendCrv = MakeTestBlendCurve(nset, cvCrv, ptCrv);

	// make an offset of the CV curve
	int offCrv = MakeTestOffsetCurve(nset, cvCrv);

	// make a Transform curve of the point curve
	int xformCrv = MakeTestXFormCurve(nset, ptCrv);

	// make a mirror of the blend
	int mirCrv = MakeTestMirrorCurve(nset, blendCrv);

	// make a fillet curve (It makes it's own point curves to fillet)
	int fltCrv = MakeTestFilletCurve(nset);

	// make a chamfer curve (It makes it's own point curves to fillet)
	int chmCrv = MakeTestChamferCurve(nset);

	Object *obj = CreateNURBSObject(mpIp, &nset, mat);
	INode *node = mpIp->CreateObjectNode(obj);
	node->SetName(GetString(IDS_CRV_TEST_OBJECT));
	mpIp->RedrawViews(mpIp->GetTime());
}

void
APITestUtil::SurfaceTests()
{
	// now let's build a test object
	NURBSSet nset;
	Matrix3 mat;
	mat.IdentityMatrix();

	// we'll need these curves later for contruction of some surfaces
	// build a cv curve
	int cvCrv = MakeTestCVCurve(nset, mat);

	// now a point curve
	int ptCrv = MakeTestPointCurve(nset, mat);

	// make a Transform curve of the point curve
	int xformCrv = MakeTestXFormCurve(nset, ptCrv);

	// make an offset of the CV curve
	int offCrv = MakeTestOffsetCurve(nset, cvCrv);



	// build a cv surface
	int cvSurf = MakeTestCVSurface(nset, mat, TRUE);


	// build a point surface
	int ptSurf = MakeTestPointSurface(nset, mat);

	// Blend the two surfaces
	int blendSurf = MakeTestBlendSurface(nset, cvSurf, ptSurf);

	// Offset of the blend
	int offSurf = MakeTestOffsetSurface(nset, blendSurf);

	// Transform of the Offset
	int xformSurf = MakeTestXFormSurface(nset, offSurf);

	// Mirror of the transform surface
	int mirSurf = MakeTestMirrorSurface(nset, xformSurf);

	// Make a Ruled surface between two curves
	int rulSurf = MakeTestRuledSurface(nset, cvCrv, ptCrv);

	// Make a ULoft surface
	int uLoftSurf = MakeTestULoftSurface(nset, ptCrv, offCrv, xformCrv);

	// Make a Extrude surface
	int extSurf = MakeTestExtrudeSurface(nset, xformCrv);

	// Make a lathe
	int lthSurf = MakeTestLatheSurface(nset);

	// these will build their own curve to work with
	// UV Loft
	int uvLoftSurf = MakeTestUVLoftSurface(nset);


	// 1 Rail Sweep
	int oneRailSurf = MakeTest1RailSweepSurface(nset);

	// 2 Rail Sweep
	int twoRailSurf = MakeTest2RailSweepSurface(nset);

	// MultiCurveTrim Surface
	int multiTrimSurf = MakeTestMultiCurveTrimSurface(nset);

	// N Blend

	// Cap Surface

	Object *obj = CreateNURBSObject(mpIp, &nset, mat);
	INode *node = mpIp->CreateObjectNode(obj);
	node->SetName(GetString(IDS_SRF_TEST_OBJECT));
	mpIp->RedrawViews(mpIp->GetTime());
}

void
APITestUtil::COSTests()
{
	// now let's build a test object
	NURBSSet nset;
	Matrix3 mat;
	mat.IdentityMatrix();

	// build a cv surface
	int cvSurf = MakeTestCVSurface(nset, mat);

	// Now an Iso Curve on the CV surface
	int isoCrv1 = MakeTestIsoCurveU(nset, cvSurf);

	// Now an Iso Curve on the CV surface
	int isoCrv2 = MakeTestIsoCurveV(nset, cvSurf);

	// build a CV Curve on Surface
	int cvCOS = MakeTestCurveOnSurface(nset, cvSurf);

	// build a Point Curve on Surface
	int pntCOS = MakeTestPointCurveOnSurface(nset, cvSurf);

	// build a Surface Normal Offset Curve
	int cnoCrf = MakeTestSurfaceNormalCurve(nset, cvCOS);

	// build a surface surface intersection curve
	int cvSurf1 = MakeTestCVSurface(nset, TransMatrix(Point3(0.0, 0.0, -30.0)) * RotateYMatrix(0.5));
	int intCrv = MakeTestSurfSurfIntersectionCurve(nset, cvSurf, cvSurf1);

	// Vector Projection Curve
	int cvCrv1 = MakeTestCVCurve(nset, TransMatrix(Point3(-100, -200, 40)));
	int vecCrv = MakeTestProjectVectorCurve(nset, cvSurf, cvCrv1);

	// Normal Normal Curve
	int cvCrv2 = MakeTestCVCurve(nset, TransMatrix(Point3(0, -250, 10)));
	int nrmCrv = MakeTestProjectNormalCurve(nset, cvSurf1, cvCrv2);


	Object *obj = CreateNURBSObject(mpIp, &nset, mat);
	INode *node = mpIp->CreateObjectNode(obj);
	node->SetName(GetString(IDS_COS_TEST_OBJECT));
	mpIp->RedrawViews(mpIp->GetTime());
}





static INT_PTR CALLBACK
APITestDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	APITestUtil *pUtil = (APITestUtil*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
        pUtil = (APITestUtil*) lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		CheckDlgButton(hWnd, IDC_RELATIONAL, pUtil->mRelational);
		CheckDlgButton(hWnd, IDC_AS_CURVE, pUtil->mAsCurve);
		CheckDlgButton(hWnd, IDC_FILE, pUtil->mToFile);
		SetDlgItemText(hWnd, IDC_FILENAME, pUtil->mFilename); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RUN_TEST:
			pUtil->CombinedTests();
			break;

		case IDC_POINT_TEST:
			pUtil->PointTests();
			break;
		case IDC_CURVE_TEST:
			pUtil->CurveTests();
			break;
		case IDC_SURF_TEST:
			pUtil->SurfaceTests();
			break;
		case IDC_COS_TEST:
			pUtil->COSTests();
			break;

		case IDC_DUMP:
			pUtil->DumpSelected();
			break;

		case IDC_AS_CURVE:
			pUtil->mAsCurve = IsDlgButtonChecked(hWnd, IDC_AS_CURVE);
			break;

		case IDC_RELATIONAL:
			pUtil->mRelational = IsDlgButtonChecked(hWnd, IDC_RELATIONAL);
			break;
		case IDC_FILE:
			pUtil->mToFile = IsDlgButtonChecked(hWnd, IDC_FILE);
			break;
		case IDC_FILENAME:
			switch(HIWORD(wParam)) {
			case EN_SETFOCUS:
				DisableAccelerators();					
				break;
			case EN_KILLFOCUS:
				EnableAccelerators();
				break;
			}
			GetDlgItemText(hWnd, IDC_FILENAME, pUtil->mFilename, 255); 
			break;
		}
		break;
	case WM_DESTROY:
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		pUtil->mpIp->RollupMouseMessage(hWnd,msg,wParam,lParam); 
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void
APITestUtil::BeginEditParams(Interface *ip,IUtil *iu)
{
	this->mpIu = iu;
	this->mpIp = (IObjParam*)ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_NURBS_API_TEST),
		APITestDlgProc,
		GetString(IDS_API_TEST),
		(LPARAM)this);
}


void
APITestUtil::EndEditParams(Interface *ip,IUtil *iu)
{
	this->mpIu = NULL;
	this->mpIp = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}
