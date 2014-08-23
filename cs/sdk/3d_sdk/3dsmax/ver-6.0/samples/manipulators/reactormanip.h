/**********************************************************************
 *<
	FILE: ReactorManip.h

	DESCRIPTION:	Manipulators for the Reactor controller.  
					Includes manipulators for:
					Rotation Angles
					Rotational Influence 
					Current Rotation
					Position Values
					Spherical Influence
					Current Position


	CREATED BY: Adam Felt

	HISTORY: created 6/27/2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ConeAngleManip.h"
#include "manipulator.h"
#include "..\controllers\reactor\ReactAPI.h"

#pragma once

//Cone style manipulator to adjust the influence when reacting to a rotation
//*****************************************************************************
#define ReactorAngleManip_CLASS_ID Class_ID(0x4cdb23bd, 0x4c855590)

class ReactorAngleManipulator: public ConeAngleManipulator
{
public:
	float mfVectorLength;

    ReactorAngleManipulator(IReactor* pReactor, INode* pNode);
    ReactorAngleManipulator();

    void SetAngle(float angle);
    void SetQuatValue(Quat angle);
	void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
	void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    void UpdateShapes(TimeValue t, TSTR& toolTip);

    IReactor* GetReactor() { return (IReactor*)GetManipTarget(); }
};


#define PositionManip_CLASS_ID Class_ID(0x683431b1, 0x36e93ce4)
#define PBLOCK_REF_NO	 0
//ClassDesc* GetPositionManipDesc();

class PositionManipulator : public SimpleManipulator
{
public:
    // block IDs
	enum { kPositionManipParams };

	// Parameter IDs
	enum { kPositionManipPos,
		   kPositionManipRadius,};

	Point3 mInitPos;
	IPoint2 mInitPoint;

	PositionManipulator(INode* pNode);
    PositionManipulator();

    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    TCHAR *GetObjectName() {
        return GetString(IDS_POSITION_MANIP);
    }
    
    void DeleteThis() {
        delete this;
    }
    Class_ID ClassID() {
        return PositionManip_CLASS_ID;
    }
    int	NumParamBlocks() {
        return 1;
    }
    IParamBlock2* GetParamBlock(int i) {
        return mpPblock;
    } 
    IParamBlock2* GetParamBlockByID(BlockID id) {
        return (mpPblock->ID() == id) ? mpPblock : NULL;
    } 
    
    void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    virtual void SetPosition(Point3 pos);
	virtual void SetRadius(float radius);

    void UpdateShapes(TimeValue t, TSTR& toolTip);
    void GenerateShapes(TimeValue t);

    void SetValues(Point3 pos, float radius);

    //static const int kSliderOffset;
};

//Sphere style manipulator used when reacting to Positions
//The sphere displays and adjusts the influence range
//*****************************************************
class SphericalManipulator: public PositionManipulator
{
public:
    SphericalManipulator(IReactor* pReactor, INode* pNode);
    SphericalManipulator();
    
    void SetInfluence(float inf);
	void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    void UpdateShapes(TimeValue t, TSTR& toolTip);
};
//****************************************************

#define ReactorPosValueManip_CLASS_ID Class_ID(0x41e477a1, 0x6c0d48d3)

class ReactorPositionValueManip : public SimpleManipulator
{
public:

    // block IDs
	enum { kPositionManipParams };

	// Parameter IDs
	enum { kPositionManipPosValue, kPositionManipRadius, kPositionManipPosState,};

	Point3 mInitPos;
	IPoint2 mInitPoint;
	float mInitRadius;

	ReactorPositionValueManip(IReactor* react, INode* pNode);
    ReactorPositionValueManip();

    int	NumParamBlocks() { return 1; }
    IParamBlock2* GetParamBlock(int i) { return mpPblock; } 
    IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPblock->ID() == id) ? mpPblock : NULL; } 
    
    void SetPositionValue(Point3 pos, int index);
	void SetPositionState(Point3 pos, int index);
	void SetRadius(float radius, int index);
	void SetReactionValue(Point3 pos, int index);
	
	void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
	void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    void UpdateShapes(TimeValue t, TSTR& toolTip);
	IReactor* GetReactor() { return (IReactor*)GetManipTarget(); }
	CreateMouseCallBack* ReactorPositionValueManip::GetCreateMouseCallBack();
};
