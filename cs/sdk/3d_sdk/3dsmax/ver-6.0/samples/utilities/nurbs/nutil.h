/**********************************************************************
 *<
	FILE: nutil.h

	DESCRIPTION: header file for the Test harness utility plugin for the
	             NURBS API

	CREATED BY: Charlie Thaeler

	HISTORY: created 8/13/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/


#ifndef _NUTIL_H
#define _NUTIL_H

#include "max.h"
#include "iparamm.h"
#include "utilapi.h"
#include "surf_api.h"
#include "resource.h"

extern HINSTANCE hInstance;
extern TCHAR *GetString(int id);

#define API_TEST_UTIL_CLASS_ID Class_ID(0x3a521292, 0x740775fa)
#define SURF_APPROX_UTIL_CLASS_ID Class_ID(0x6216493, 0x131c3958)


enum SurfApproxType {
	kCurveView = 0,
	kCurveRend,
	kSurfView,
	kSurfRend,
	kSurfDisp
};


class SurfApproxUtil : public UtilityObj {
private:
	IUtil* mpIu;
public:
	IObjParam* mpIp;
	SurfApproxType mSettingType;
	TessApprox *mpSurfView;
	TessApprox *mpSurfRend;
	TessApprox *mpDispRend;
	TessApprox *mpCurveView;
	TessApprox *mpCurveRend;
	BOOL mClearSurfaces;

	NURBSDisplay mDisplay;

	SurfApproxUtil(void);
	~SurfApproxUtil(void);
	friend INT_PTR CALLBACK SurfApproxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SetSurfApprox();
	void SetSurfDisplay();
	void SetupDisplayUI(HWND hwnd);
	void DeleteThis() {}
	void ResetTess();
	void ResetDisplay();

	TessApprox *GetTess(SurfApproxType type);
	void SetTess(TessApprox &tess, SurfApproxType type);
	void ClearTess(SurfApproxType type);

    TessApprox* GetPreset(int preset);
    void SetPreset(int preset, TessApprox& tess);
};

class APITestUtil : public UtilityObj {
private:
	IUtil* mpIu;
	IObjParam* mpIp;
public:
	char mFilename[256];
	BOOL mToFile;
	BOOL mRelational;
	BOOL mAsCurve;
	FILE *mFP;

	APITestUtil() { mRelational = TRUE; mAsCurve = FALSE; mToFile = FALSE; mFP = NULL; sprintf(mFilename, "testdump.txt"); }

	friend INT_PTR CALLBACK APITestDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void CombinedTests();
	void PointTests();
	void CurveTests();
	void SurfaceTests();
	void COSTests();

	void DumpPrint(const TCHAR *format, ...);
	void DumpMatrix3(int indent, Matrix3& mat);
	void PrintPoint(int indent, Point3 pt);
	void DumpObject(int indent, NURBSObject* nobj, TimeValue t);
	void DumpSelected();
	void DeleteThis() {}

	int MakeTestPoint(NURBSSet &nset);
	int MakeTestPointCPoint(NURBSSet &nset, int p1);
	int MakeTestCurveCPoint(NURBSSet &nset, int p1);
	int MakeTestSurfCPoint(NURBSSet &nset, int p1);
	int MakeTestCurveCurve(NURBSSet &nset, int p1, int p2, BOOL trim);
	int MakeTestCurveSurface(NURBSSet &nset, int p1, int p2);


	int MakeTestCVCurve(NURBSSet &nset, Matrix3 mat);
	int MakeTestPointCurve(NURBSSet &nset, Matrix3 mat);
	int MakeTestBlendCurve(NURBSSet &nset, int p1, int p2);
	int MakeTestOffsetCurve(NURBSSet &nset, int p);
	int MakeTestXFormCurve(NURBSSet &nset, int p);
	int MakeTestMirrorCurve(NURBSSet &nset, int p);
	int MakeTestFilletCurve(NURBSSet &nset);
	int MakeTestChamferCurve(NURBSSet &nset);
	int MakeTestIsoCurveU(NURBSSet &nset, int p);
	int MakeTestIsoCurveV(NURBSSet &nset, int p);
	int MakeTestSurfaceEdgeCurve(NURBSSet &nset, int p);
	int MakeTestProjectVectorCurve(NURBSSet &nset, int p1, int p2);
	int MakeTestProjectNormalCurve(NURBSSet &nset, int p1, int p2);
	int MakeTestSurfSurfIntersectionCurve(NURBSSet &nset, int p1, int p2);
	int MakeTestCurveOnSurface(NURBSSet &nset, int p);
	int MakeTestPointCurveOnSurface(NURBSSet &nset, int p);
	int MakeTestSurfaceNormalCurve(NURBSSet &nset, int p);


	int MakeTestCVSurface(NURBSSet &nset, Matrix3 mat, BOOL rigid=FALSE);
	int MakeTestPointSurface(NURBSSet &nset, Matrix3 mat);
	int MakeTestBlendSurface(NURBSSet &nset, int p1, int p2);
	int MakeTestOffsetSurface(NURBSSet &nset, int p);
	int MakeTestXFormSurface(NURBSSet &nset, int p);
	int MakeTestMirrorSurface(NURBSSet &nset, int p);
	int MakeTestRuledSurface(NURBSSet &nset, int p1, int p2);
	int MakeTestULoftSurface(NURBSSet &nset, int p1, int p2, int p3);
	int MakeTestExtrudeSurface(NURBSSet &nset, int p1);
	int MakeTestLatheSurface(NURBSSet &nset);
	int MakeTestUVLoftSurface(NURBSSet &nset);
	int MakeTestNBlendSurface(NURBSSet &nset);
	int MakeTest1RailSweepSurface(NURBSSet &nset);
	int MakeTest2RailSweepSurface(NURBSSet &nset);
	int MakeTestCapSurface(NURBSSet &nset);
	int MakeTestMultiCurveTrimSurface(NURBSSet &nset);

};




class SurfApproxUtilDesc : public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new SurfApproxUtil();}
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return SURF_APPROX_UTIL_CLASS_ID; }
	const TCHAR* 	Category();
};
extern ClassDesc* GetSurfApproxUtilDesc();

class APITestUtilDesc : public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new APITestUtil();}
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return API_TEST_UTIL_CLASS_ID; }
	const TCHAR* 	Category();
};
extern ClassDesc* GetAPITestUtilDesc();




#endif
