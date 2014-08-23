/**********************************************************************
 *<
	FILE: PlaneAngleManip.h

	DESCRIPTION:  Manipulator for planar angles.

	CREATED BY: Scott Morrison

	HISTORY: created 5/18/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manipulator.h"

#pragma once

#define PlaneAngleManip_CLASS_ID Class_ID(0x25e1576a, 0x217a2a10)
#define PBLOCK_REF_NO	 0
ClassDesc* GetPlaneAngleManipDesc();

// Manipulator to specify a solid angle, such as a spotlight falloff angle
class PlaneAngleManipulator: public SimpleManipulator 
{
public:
    PlaneAngleManipulator();
    PlaneAngleManipulator(INode* pNode);

    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    TCHAR *GetObjectName() {
        return GetString(IDS_PLANE_ANGLE_MANIP);
    }
    
    void DeleteThis() {
        delete this;
    }
    Class_ID ClassID() {
        return PlaneAngleManip_CLASS_ID;
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
    
    void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    virtual void SetAngle(float angle);

    void UpdateShapes(TimeValue t, TSTR& toolTip);
    void GenerateShapes(TimeValue t);

    void SetValues(Point3& origin, Point3& normalVec, Point3& upVec,
                   float angle, float distance, float size);

};

// Parameter block description for plane angle manipulators

// block IDs
enum { kPlaneAngleParams };

// Parameter IDs
enum { kPlaneAngleOrigin,
       kPlaneAngleNormalVec,
       kPlaneAngleUpVec,
       kPlaneAngleAngle,
       kPlaneAngleDistance,
       kPlaneAngleSize,
};

#define IKSwivelManip_CLASS_ID Class_ID(0x2581576e, 0xb5ea7a19)
#define IKStartSpTwistManip_CLASS_ID Class_ID(0x727c1f1f, 0x549611c1)
#define IKEndSpTwistManip_CLASS_ID Class_ID(0xdc564f2, 0x30235bd4)

class IKSwivelManipulator: public PlaneAngleManipulator
{
public:
    IKSwivelManipulator(Control *pTMControl, INode* pNode);
    IKSwivelManipulator();
    

    void SetAngle(float angle);

    void UpdateShapes(TimeValue t, TSTR& toolTip);

};


class IKStartSpTwistManipulator: public PlaneAngleManipulator
{
public:
    IKStartSpTwistManipulator(Control *pTMControl, INode* pNode);
    IKStartSpTwistManipulator();
    

    void SetAngle(float angle);

    void UpdateShapes(TimeValue t, TSTR& toolTip);

};

class IKEndSpTwistManipulator: public PlaneAngleManipulator
{
public:
    IKEndSpTwistManipulator(Control *pTMControl, INode* pNode);
    IKEndSpTwistManipulator();
    

    void SetAngle(float angle);

    void UpdateShapes(TimeValue t, TSTR& toolTip);

};