/**********************************************************************
 *<
	FILE: SkinEngine.j

	DESCRIPTION:  Declaration of the SkinEngine

	CREATED BY: Nikolai Sander
				
	HISTORY: created 7/10/99

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __SKINENGINE_H_
#define __SKINENGINE_H_

#include "resource.h"       // main symbols
#include <math.h>
#include "geom.h"
#include "utilities.h"
#include "MAXComponentsCP.h"

class BoneDataClass;
class PointDataClass;

/////////////////////////////////////////////////////////////////////////////
// CSkinEngine
class ATL_NO_VTABLE CSkinEngine : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSkinEngine, &CLSID_SkinEngine>,
	public IConnectionPointContainerImpl<CSkinEngine>,
	public IDispatchImpl<ISkinEngine, &IID_ISkinEngine, &LIBID_MAXCOMPONENTSLib>,
	public CProxy_ISkinEngineEvents< CSkinEngine >
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SKINENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSkinEngine)
	COM_INTERFACE_ENTRY(ISkinEngine)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CSkinEngine)
CONNECTION_POINT_ENTRY(IID__ISkinEngineEvents)
END_CONNECTION_POINT_MAP()


public:
	
	CSkinEngine();
	~CSkinEngine();

// ISkinEngine
public:
	STDMETHOD(SetBoneFlags)(/*[in]*/int boneIdx, /*[in]*/ DWORD flags);
	STDMETHOD(SetInitTM)(/*[in, size_is(12)]*/ float *InitTM);
	STDMETHOD(MapPoint)(/*[in]*/ int idx, /*[out]*/ float *pin, /*[out]*/ float *pout);
	STDMETHOD(SetBoneTM)(/*[in]*/ int boneIdx, /*[in, size_is(12)]*/ float *currentTM);
	STDMETHOD(SetInitBoneTM)(/*[in]*/ int boneIdx, /*[in, size_is(12)]*/ float *InitTM);
	STDMETHOD(SetPointData)(int pointIdx, int numData, 
							DWORD b_stride, int *BoneIndexArray, 
							DWORD w_stride, float *WeightArray, 
							DWORD sci_stride, int *SubCurveIdxArray, 
							DWORD ssi_stride, int *SubSegIdxArray, 
							DWORD ssd_stride, float *SubSegDistArray, 
							DWORD t_stride, float *TangentsArray, 
							DWORD op_stride, float *OPointsArray);
	STDMETHOD(SetNumBones)(/*[in]*/ int numBones);
	STDMETHOD(SetNumPoints)(/*[in]*/ int numPoints);
protected:
	Point3 SplineAnimation(int vertex, int bone, Point3 p);

	Tab<BoneDataClass> BoneData;
	Tab<PointDataClass> PointData;

	Matrix3 m_MeshTM;
	StrideArray<Point3> m_Points;
};

class BoneDataClass
{
public:
	void SetInitTM(float *ptm);
	void SetCurrentTM(Matrix3 &tm);
	Matrix3 const &GetCurrentTM() {return m_CurrentTM;}
	void SetFlags(DWORD flags);
	DWORD const GetFlags(){return m_flags;}
	Matrix3 &GetXFormTM(Matrix3 &MeshTM);
protected:
    Matrix3 m_XFormTM;
	Matrix3 m_InitTM;
    Matrix3 m_CurrentTM;
    DWORD m_flags;
	BOOL bCacheValid;
};

class PointDataClass
{
public:
	int m_numBones;
	Point3 m_InitialPos;
	StrideArray<int> m_BoneIndices;
	StrideArray<float> m_BoneWeights;
	
	// This stuff is only needed for Spline bones interpolation
	StrideArray<int> m_SubCurveIndices;
	StrideArray<int> m_SubSegIndices;
	StrideArray<float> m_SubSegDistance;
	StrideArray<Point3> m_Tangents;
    StrideArray<Point3> m_OPoints;
};

#endif //__SKINENGINE_H_