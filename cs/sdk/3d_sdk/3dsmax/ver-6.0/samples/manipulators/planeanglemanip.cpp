/**********************************************************************
 *<
    FILE: PlaneAngleManip.cpp

    DESCRIPTION:  Manipulator for plane angles.

    CREATED BY:							Scott Morrison
		IK Swivel Angle Manipulator:	Jianmin Zhao
		IK Twist Angle Manipulators:	Ambarish Goswami+

    HISTORY: created 5/18/00
			IK Swivel Angle Manipulator: ??
			IK Twist Angle Manipulators: Feb, 2002
			

 *> Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manip.h"
#include "Manipulator.h"
#include "iparamm2.h"
#include "PlaneAngleManip.h"
#include "IIKSys.h"
#include "IKSolver.h"
#include "iparamb2.h"

extern const Class_ID kSplineIKChainClassID = Class_ID(0x21c71167, 0x692d0dd7);

// The class descriptor for gsphere
class PlaneAngleClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 1; }
    void *          Create(BOOL loading = FALSE) { return new PlaneAngleManipulator; }
    const TCHAR *   ClassName() { return GetString(IDS_PLANE_ANGLE_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return PlaneAngleManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("PlaneAngleManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    // This manipulator helper object manipulates itself
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        return hTarget->ClassID() == ClassID() &&
            hTarget->SuperClassID() == SuperClassID();
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

Manipulator *
PlaneAngleClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    PlaneAngleManipulator* pManip = new PlaneAngleManipulator(pNode);
    pManip->SetManipTarget(hTarget);

    return pManip;
}

static PlaneAngleClassDesc PlaneAngleDesc;
ClassDesc* GetPlaneAngleDesc() { return &PlaneAngleDesc; }

// Parameter block description for plane angle manipulators

// per instance geosphere block
static ParamBlockDesc2 PlaneAngleManipBlock
( kPlaneAngleParams,
  _T("PlaneAngleManipParameters"),
  IDS_PLANE_ANGLE_MANIP,
  &PlaneAngleDesc,
  P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF_NO, 

  //rollout
  IDD_PLANE_ANGLE_PARAMS,
  IDS_PARAMETERS, 0, 0, NULL, 

  // params
  kPlaneAngleOrigin,   _T("Origin"), TYPE_POINT3, 0, IDS_ORIGIN,
  p_default,      Point3(0, 0, 0), 
  end, 

  kPlaneAngleNormalVec,   _T("NormalVec"), TYPE_POINT3, 0, IDS_NORMAL_VEC,
  p_default,      Point3(0, 0, 1), 
  end, 

  kPlaneAngleUpVec,   _T("UpVec"), TYPE_POINT3, 0, IDS_UP_VEC,
  p_default,      Point3(0, 1, 0), 
  end, 

  kPlaneAngleAngle,   _T("Angle"), TYPE_ANGLE, P_ANIMATABLE, IDS_ANGLE,
  p_default,      0.0, 
  p_range,        -100000.0f, 100000.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ANGLE, IDC_ANGLE_SPIN, 1.0f, 
  end, 

  kPlaneAngleDistance,   _T("Distance"), TYPE_FLOAT, 0, IDS_DISTANCE,
  p_default,      0.0, 
  p_range,        0.0f, 10000.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISTANCE, IDC_DISTANCE_SPIN, SPIN_AUTOSCALE, 
  end,

  kPlaneAngleSize,   _T("Size"), TYPE_FLOAT, 0, IDS_SIZE,
  p_default,      1.0, 
  p_range,        0.0f, 100.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_MANIP_SIZE, IDC_SIZE_SPIN, SPIN_AUTOSCALE, 
  end,

  end
    );

// PlaneAngleManipulator for editing angles, like the IK controller swivel

PlaneAngleManipulator::PlaneAngleManipulator() :
    SimpleManipulator()
{
    GetPlaneAngleDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

PlaneAngleManipulator::PlaneAngleManipulator(INode* pNode) :
    SimpleManipulator(pNode)
{
    GetPlaneAngleDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

#define NUM_CIRC_PTS    28

void
PlaneAngleManipulator::SetValues(Point3& origin, Point3& normalVec, Point3& upVec,
                                 float angle, float distance, float size)
{
    TimeValue t = GetCOREInterface()->GetTime();

    theHold.Suspend();
    // Store the definitions away
    mpPblock->SetValue(kPlaneAngleAngle,     t, angle);
    mpPblock->SetValue(kPlaneAngleSize,      t, size);
    mpPblock->SetValue(kPlaneAngleDistance,  t, distance);
    mpPblock->SetValue(kPlaneAngleOrigin,    t, origin);
    mpPblock->SetValue(kPlaneAngleUpVec,     t, upVec);
    mpPblock->SetValue(kPlaneAngleNormalVec, t, normalVec);
    theHold.Resume();

    GenerateShapes(t);
}

void
PlaneAngleManipulator::SetAngle(float angle)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        PlaneAngleManipulator* pManip = (PlaneAngleManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(kPlaneAngleAngle, t, angle);
        pManip->mpPblock->GetDesc()->InvalidateUI(kPlaneAngleAngle);
    }
}

void
PlaneAngleManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    // Check for self-manipulation
    if (mhTarget) {
        PlaneAngleManipulator* pTargetManip = (PlaneAngleManipulator*) mhTarget;
        // Set all the other parameters from 
        
        Point3 upVec, normalVec, origin;

        float angle, distance, size;
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleAngle,     t, angle,     FOREVER);
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleSize,      t, size,     FOREVER);
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleDistance,  t, distance,  FOREVER);
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleOrigin,    t, origin,    FOREVER);
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleUpVec,     t, upVec,     FOREVER);
        pTargetManip->GetPBlock()->GetValue(kPlaneAngleNormalVec, t, normalVec, FOREVER);

        theHold.Suspend();
        GetPBlock()->SetValue(kPlaneAngleAngle,     t, angle);
        GetPBlock()->SetValue(kPlaneAngleSize,      t, size);
        GetPBlock()->SetValue(kPlaneAngleDistance,  t, distance);
        GetPBlock()->SetValue(kPlaneAngleOrigin,    t, origin);
        GetPBlock()->SetValue(kPlaneAngleUpVec,     t, upVec);
        GetPBlock()->SetValue(kPlaneAngleNormalVec, t, normalVec);
        theHold.Resume();

        TSTR nodeName;
        nodeName = mpINode->GetName();
        
        toolTip.printf("%s [Angle: %5.2f]", nodeName.data(),
                       (double) RadToDeg(angle));
    }

    
    GenerateShapes(t);
    
}

void
PlaneAngleManipulator::GenerateShapes(TimeValue t)
{
    Point3 upVec, normalVec, origin;

    float angle, distance, size;
    mValid = FOREVER;
    mpPblock->GetValue(kPlaneAngleAngle,     t, angle,     mValid);
    mpPblock->GetValue(kPlaneAngleSize,      t, size,     mValid);
    mpPblock->GetValue(kPlaneAngleDistance,  t, distance,  mValid);
    mpPblock->GetValue(kPlaneAngleOrigin,    t, origin,    mValid);
    mpPblock->GetValue(kPlaneAngleUpVec,     t, upVec,     mValid);
    mpPblock->GetValue(kPlaneAngleNormalVec, t, normalVec, mValid);

    ClearPolyShapes();

    PolyShape shape;

    PolyLine line;
    line.Init();

    float cosAng = cos(angle);
    float sinAng = sin(angle);

    upVec = Normalize(upVec);
    normalVec = Normalize(normalVec);
    Point3 axis = normalVec ^ upVec;

    Point3 endPoint = origin + (cosAng * distance) * upVec +
                               (sinAng * distance) * axis;

    size *= 5.0f;

    Point3 handleVec  = Normalize(endPoint - origin);
    Point3 handleAxis = handleVec ^ normalVec;

    Point3 p0 = endPoint - (3.0f * size) * handleVec;
    Point3 p1 = p0 + size * handleVec + size * handleAxis;
    Point3 p2 = p1 + (2.0f * size) * handleVec;
    Point3 p3 = p0 + size * handleVec - size * handleAxis;
    Point3 p4 = p3 + (2.0f * size) * handleVec;

    float offset = size / 4.0f;
    Point3 p5 = p0 + offset * handleVec;
    Point3 p6 = p1 - offset * handleAxis;
    Point3 p7 = p2 - offset * handleAxis - size * handleVec;
    Point3 p8 = p4 + offset * handleAxis - size * handleVec;
    Point3 p9 = p3 + offset * handleAxis;

    line.Append(PolyPt(origin));
    line.Append(PolyPt(p0));
    line.Append(PolyPt(p1));
    line.Append(PolyPt(p2));
    line.Append(PolyPt(p4));
    line.Append(PolyPt(p3));
    line.Append(PolyPt(p0));

    shape.Append(line);

    line.Init();
    line.Append(PolyPt(p5));
    line.Append(PolyPt(p6));
    line.Append(PolyPt(p7));
    line.Append(PolyPt(p8));
    line.Append(PolyPt(p9));
    line.Append(PolyPt(p5));

    shape.Append(line);

    AppendPolyShape(&shape, 0, GetUnselectedColor());

}

class PlaneAngleManipCreateCallBack : public CreateMouseCallBack {
    IPoint2 sp0;
    PlaneAngleManipulator *mpManip;
    Point3 p0;
public:
    int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
    void SetObj(PlaneAngleManipulator *pManip) {mpManip = pManip;}
};

int PlaneAngleManipCreateCallBack::proc(ViewExp *pVpt, int msg, int point, int flags,
                                       IPoint2 m, Matrix3& mat ) {
    float r;
    Point3 p1, center;

    if (msg == MOUSE_FREEMOVE)
    {
        pVpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
    }


    if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
        switch(point) {
        case 0:  // only happens with MOUSE_POINT msg
//            ob->suspendSnap = TRUE;             
            sp0 = m;
            p0 = pVpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
            mat.SetTrans(p0);
            break;
        case 1:
            mat.IdentityMatrix();
            p1 = pVpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
            r = Length(p1-p0);
            mat.SetTrans(p0);

            mpManip->GetPBlock()->SetValue(kPlaneAngleDistance, 0, r);
            PlaneAngleManipBlock.InvalidateUI();

            if (msg==MOUSE_POINT) {
                return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
            }
            break;                     
        }
    } else {
        if (msg == MOUSE_ABORT) return CREATE_ABORT;
    }

    return TRUE;
}

static PlaneAngleManipCreateCallBack PlaneAngleManipCreateCB;

CreateMouseCallBack* 
PlaneAngleManipulator::GetCreateMouseCallBack()
{
    PlaneAngleManipCreateCB.SetObj(this);
    return(&PlaneAngleManipCreateCB);
}


void 
PlaneAngleManipulator::BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev)
{
    SimpleManipulator::BeginEditParams(ip, flags, prev);
    PlaneAngleDesc.BeginEditParams(ip, this, flags, prev);
}

void 
PlaneAngleManipulator::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next)
{
    SimpleManipulator::EndEditParams(ip, flags, next);
    PlaneAngleDesc.EndEditParams(ip, this, flags, next);
}

RefTargetHandle 
PlaneAngleManipulator::Clone(RemapDir& remap)
{
    PlaneAngleManipulator* pNewManip = new PlaneAngleManipulator(); 
    pNewManip->ReplaceReference(0, mpPblock->Clone(remap));
    pNewManip->mValid.SetEmpty();   
	BaseClone(this, pNewManip, remap);
    return(pNewManip);
}

// 
void
PlaneAngleManipulator::OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
    SimpleManipulator::OnButtonUp(t, pVpt, m, flags, pHitData);
}

// Wrap the angle so that winding numbers are preserved.
static float
WrapAngle(float oldAngle, float angle)
{
    float twoPi = float(2.0 * PI);
    int windingNumber = int(oldAngle / ( twoPi));
    if (oldAngle < 0.0)
        windingNumber--;
    float normAngle = oldAngle - windingNumber * twoPi;
    float deltaAngle = angle - normAngle;
    if (deltaAngle < -PI) {
        windingNumber++;
    } else if (deltaAngle > PI) {
        windingNumber--;
    }

    return angle + windingNumber * twoPi;
}

void
PlaneAngleManipulator::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) {
        Point3 upVec, normalVec, origin;
        
        float oldAngle;
        mValid = FOREVER;
        mpPblock->GetValue(kPlaneAngleAngle,     t, oldAngle,  mValid);
        mpPblock->GetValue(kPlaneAngleOrigin,    t, origin,    mValid);
        mpPblock->GetValue(kPlaneAngleUpVec,     t, upVec,     mValid);
        mpPblock->GetValue(kPlaneAngleNormalVec, t, normalVec, mValid);

        upVec = Normalize(upVec);
        normalVec = Normalize(normalVec);

        Ray viewRay;  // The viewing vector in local coords
        GetLocalViewRay(pVpt, m, viewRay);
        
        Plane anglePlane(normalVec, origin);
        
        Point3 newP;
        bool res = anglePlane.Intersect(viewRay, newP);

        if (!res)
            return;

        Point3 axis = upVec ^ normalVec;
        
        float a,b;
        Point3 p = Normalize(newP - origin);
        a = DotProd(upVec, p);
        b = DotProd(axis, p);

        float angle = 2.0 * PI - atan2(b, a);
        if (angle > 2.0 * PI)
            angle -= (float) 2.0 * PI;

        angle = GetCOREInterface()->SnapAngle(angle, FALSE, FALSE);
        angle = WrapAngle(oldAngle, angle);
        SetAngle(angle);

        SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
    }
}

// The class descriptor for gsphere
class IKSwivelManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new IKSwivelManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_IKSWIVEL_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return IKSwivelManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("IKSwivelManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == IKCHAINCONTROL_CLASS_ID &&
             hTarget->SuperClassID() == CTRL_MATRIX3_CLASS_ID))
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipulator
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static IKSwivelManipClassDesc IKSwivelDesc;
ClassDesc* GetIKSwivelManipDesc() { return &IKSwivelDesc; }

Manipulator *
IKSwivelManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    IKSwivelManipulator* pManip = new IKSwivelManipulator((Control*) hTarget, pNode);

    return pManip;
}

IKSwivelManipulator::IKSwivelManipulator(Control* pTMControl, INode* pNode) :
    PlaneAngleManipulator(pNode)
{
    SetManipTarget(pTMControl);
}

IKSwivelManipulator::IKSwivelManipulator():
    PlaneAngleManipulator()
{
}

void
IKSwivelManipulator::SetAngle(float angle)
{
    Control* pControl = (Control*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
    // Set the swivel angle in the param block
    pParamBlock->SetValue(IIKChainControl::kSwivel, t, angle);
    // Update the swivel spinner
    pParamBlock->GetDesc()->InvalidateUI(IIKChainControl::kSwivel);
}

void
IKSwivelManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    Control* pControl = (Control*) mhTarget;

    // Get the IIKChainControl from the controller
    IIKChainControl* pIKSys = (IIKChainControl*) pControl->GetInterface(I_IKCHAINCONTROL);
    assert(pIKSys);

    INode* pStartJoint = pIKSys->StartJoint();
    INode* pEndJoint   = pIKSys->EndJoint();
    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
	IKSolver* ikSolver = pIKSys->Solver();
    BOOL enabled;
    pParamBlock->GetValue(IIKChainControl::kVHDisplay, t, enabled, FOREVER);
	// If the ik solver it uses does not use the swivel angle, we don't
	// proceed.
    if (ikSolver == NULL ||
		pStartJoint == NULL ||
		pEndJoint == NULL ||
		!ikSolver->UseSwivelAngle() ||
		!enabled) {
        ClearPolyShapes();
        mValid = FOREVER;
        return;
    }

    Matrix3 startTM, endTM, nodeTM;
    startTM = pStartJoint->GetNodeTM(t);
    endTM = pEndJoint->GetNodeTM(t);
    assert(mpINode == pIKSys->GetNode());
    nodeTM = mpINode->GetObjectTM(t);
    Matrix3 invNodeTM = Inverse(nodeTM);

    Point3 startOrigin = startTM.GetTrans();
    Point3 endOrigin =  endTM.GetTrans();

    float size;
    pParamBlock->GetValue(IIKChainControl::kVHSize, t, size, FOREVER);
    float length;
    pParamBlock->GetValue(IIKChainControl::kVHLength, t, length, FOREVER);

	// Put vectors/points in the parent space of the start joint
	//
	bool inGoal = pIKSys->SwivelAngleParent() == IIKChainControl::kSAInGoal;
	DbgAssert(inGoal ||
			  pIKSys->SwivelAngleParent() == IIKChainControl::kSAInStartJoint);
	Matrix3 parentTM = inGoal ? mpINode->GetParentTM(t)
	  : pStartJoint->GetParentTM(t);
	Matrix3 invParentTM = Inverse(parentTM);
	Point3 normalVec = VectorTransform(endOrigin - startOrigin, invParentTM);
	if (normalVec.LengthUnify() == 0.0f) {
	  // This is the case when the end effector coincides with the start joint.
	  // What can we do? Use an arbitrary vector:
	  normalVec.Set(1.0f, 0.0f, 0.0f);
	}
    float swivel = pIKSys->SwivelAngle(t, FOREVER);
	// Given an end effector axis, there corresponds to a zero plane normal
	// that forms the reference for the swivel angle. This zero vector
	// is produced by the zero-plane-map. In general, the solver can have
	// its own map. If it doesn't, we use the default zero-plane-map, which
	// is provided by the ik-system.
	//
	const IKSys::ZeroPlaneMap* zmap =
	  ikSolver->GetZeroPlaneMap(pIKSys->InitEEAxis(t), pIKSys->InitPlane(t));
	if (zmap == NULL) {
	  zmap = pIKSys->DefaultZeroPlaneMap(t);
	}
	DbgAssert(zmap);
	Point3 upVec = (*zmap)(normalVec);
	// The above upVec is the normal to the plane. In the following, we
	// put it on the plane.
	upVec = upVec ^ normalVec;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s [%s: %5.2f]", GetString(IDS_SWIVEL_ANGLE), nodeName.data(),
                   (double) RadToDeg(swivel));

    
	// Now transform to the space of mpINode
	//
	// float chainLength = pIKSys->InitChainLength(t);
	startOrigin = startOrigin * invNodeTM;
	Matrix3 transMat = parentTM * invNodeTM;
	normalVec = VectorTransform(normalVec, transMat);
	upVec = VectorTransform(upVec, transMat);

    PlaneAngleManipulator::SetValues(startOrigin,
                                     normalVec,
                                     upVec,
                                     swivel,
									 // chainLength * (length * 0.5),
									 // Use absolute size, per 268204
									 length,
                                     size);
}



//------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

// Implementation of Start IKStartSpTwistManipulator -- Start Spline Twist Manipulator for
// the Spline IK - AG -- Feb 2002


class IKStartSpTwistManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new IKStartSpTwistManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_AG_IKSTART_SPTWIST_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return IKStartSpTwistManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("IKStartSpTwistManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true if the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == kSplineIKChainClassID &&
             hTarget->SuperClassID() == CTRL_MATRIX3_CLASS_ID))
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // an instance and add it to the attached objects of the node, 
    // and will initialize the manipulator
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static IKStartSpTwistManipClassDesc IKStartSpTwistDesc;
ClassDesc* GetIKStartSpTwistManipDesc() { return &IKStartSpTwistDesc; }

Manipulator *
IKStartSpTwistManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    IKStartSpTwistManipulator* pManip = new IKStartSpTwistManipulator((Control*) hTarget, pNode);

    return pManip;
}

IKStartSpTwistManipulator::IKStartSpTwistManipulator(Control* pTMControl, INode* pNode) :
    PlaneAngleManipulator(pNode)
{
    SetManipTarget(pTMControl);
}

IKStartSpTwistManipulator::IKStartSpTwistManipulator():
    PlaneAngleManipulator()
{
}

void
IKStartSpTwistManipulator::SetAngle(float angle)
{
    Control* pControl = (Control*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
    // Set the twist angle in the param block
    pParamBlock->SetValue(IIKChainControl::kTwistHStartAngle, t, angle);
    // Update the twist spinner
    pParamBlock->GetDesc()->InvalidateUI(IIKChainControl::kTwistHStartAngle);
}

void
IKStartSpTwistManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    Control* pControl = (Control*) mhTarget;

    // Get the IIKChainControl from the controller
    IIKChainControl* pIKSys = (IIKChainControl*) pControl->GetInterface(I_IKCHAINCONTROL);
    assert(pIKSys);

    INode* pStartJoint = pIKSys->StartJoint();
    INode* pEndJoint   = pIKSys->EndJoint();

	if(!pStartJoint || !pEndJoint) return;

	INode* pChildOfFirstJoint;
;	//--------------------------------------------------------------
	//Traverse the bone hierarchy and find the child of the startbone
	// the start Twist manipulator will be perpendicular to the axis of the
	// fist bone
	INode* pParent;
	pParent =  pEndJoint->GetParentNode();

	if (pParent == pStartJoint) pChildOfFirstJoint = pEndJoint;
	else{
		while(pParent){
			if (pParent->GetParentNode() == pStartJoint) {
				pChildOfFirstJoint = pParent;
				break;
			}
			else if (pParent->IsRootNode()) { 
				assert(1); //we have reached the rootnode without crossing stJt, something is wrong
			}
			else
				pParent = pParent->GetParentNode();


		}
	}
	//-------------------------------------------------------------------

    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
	IKSolver* ikSolver = pIKSys->Solver();
    BOOL enabled;
    pParamBlock->GetValue(IIKChainControl::kTwistHStartDisplay, t, enabled, FOREVER);
	// If the ik solver it uses does not use the twist angle, we don't
	// proceed.
    if (ikSolver == NULL ||
		pStartJoint == NULL ||
		pEndJoint == NULL ||
		//!ikSolver->UseSwivelAngle() ||
		!enabled) {
        ClearPolyShapes();
        mValid = FOREVER;
        return;
    }

    Matrix3 startTM, endTM, nodeTM, pChildOfFirstJointTM;
    startTM = pStartJoint->GetNodeTM(t);
    endTM = pEndJoint->GetNodeTM(t);
	pChildOfFirstJointTM = pChildOfFirstJoint->GetNodeTM(t);
    assert(mpINode == pIKSys->GetNode());
    nodeTM = mpINode->GetObjectTM(t);
    Matrix3 invNodeTM = Inverse(nodeTM);

    Point3 startOrigin = startTM.GetTrans();
    Point3 endOrigin =  endTM.GetTrans();
	Point3 pChildOfFirstJointOrigin =  pChildOfFirstJointTM.GetTrans();

    float size;
    pParamBlock->GetValue(IIKChainControl::kTwistHStartSize, t, size, FOREVER);
    float length;
    pParamBlock->GetValue(IIKChainControl::kTwistHStartLength, t, length, FOREVER);

	// Put vectors/points in the parent space of the start joint
	//
//	bool inGoal = pIKSys->SwivelAngleParent() == IIKChainControl::kSAInGoal;
//	DbgAssert(inGoal ||
//			  pIKSys->SwivelAngleParent() == IIKChainControl::kSAInStartJoint);
//	Matrix3 parentTM = inGoal ? mpINode->GetParentTM(t)
	Matrix3 parentTM = pStartJoint->GetParentTM(t);
	Matrix3 invParentTM = Inverse(parentTM);
	Point3 normalVec = VectorTransform(pChildOfFirstJointOrigin - startOrigin, invParentTM);
	if (normalVec.LengthUnify() == 0.0f) {
	  // This is the case when the end effector coincides with the start joint.
	  // What can we do? Use an arbitrary vector:
	  normalVec.Set(1.0f, 0.0f, 0.0f);
	}
    float startTwist = pIKSys->TwistHStartAngle(t, FOREVER);
	// Given an end effector axis, there corresponds to a zero plane normal
	// that forms the reference for the swivel angle. This zero vector
	// is produced by the zero-plane-map. In general, the solver can have
	// its own map. If it doesn't, we use the default zero-plane-map, which
	// is provided by the ik-system.
	//
	const IKSys::ZeroPlaneMap* zmap =
	  ikSolver->GetZeroPlaneMap(pIKSys->InitEEAxis(t), pIKSys->InitPlane(t));
	if (zmap == NULL) {
	  zmap = pIKSys->DefaultZeroPlaneMap(t);
	}
	DbgAssert(zmap);
	Point3 upVec = (*zmap)(normalVec);
	// The above upVec is the normal to the plane. In the following, we
	// put it on the plane.
	upVec = upVec ^ normalVec;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s [%s: %5.2f]", GetString(IDS_AG_TWISTST_ANGLE), nodeName.data(),
                   (double) RadToDeg(startTwist));

  
	startOrigin = startOrigin * invNodeTM;
//	Matrix3 transMat = parentTM * invNodeTM;
//	normalVec = VectorTransform(normalVec, transMat);
//	upVec = VectorTransform(upVec, transMat);


    PlaneAngleManipulator::SetValues(startOrigin,
                                     normalVec,
                                     upVec,
                                    startTwist,
									 // chainLength * (length * 0.5),
									 // Use absolute size, per 268204
									 length,
                                     size);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// Implementation of End IKEndSpTwistManipulator -- End Spline Twist Manipulator for
// the Spline IK - AG -- Feb 2002


class IKEndSpTwistManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
	void *          Create(BOOL loading = FALSE) { return new IKEndSpTwistManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_AG_IKEND_SPTWIST_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return IKEndSpTwistManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("IKEndSpTwistManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true if the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == kSplineIKChainClassID &&
             hTarget->SuperClassID() == CTRL_MATRIX3_CLASS_ID))
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // an instance and add it to the attached objects of the node, 
    // and will initialize the manipulator
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static IKEndSpTwistManipClassDesc IKEndSpTwistDesc;
ClassDesc* GetIKEndSpTwistManipDesc() { return &IKEndSpTwistDesc; }

Manipulator *
IKEndSpTwistManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    IKEndSpTwistManipulator* pManip = new IKEndSpTwistManipulator((Control*) hTarget, pNode);

    return pManip;
}

IKEndSpTwistManipulator::IKEndSpTwistManipulator(Control* pTMControl, INode* pNode) :
    PlaneAngleManipulator(pNode)
{
    SetManipTarget(pTMControl);
}

IKEndSpTwistManipulator::IKEndSpTwistManipulator():
    PlaneAngleManipulator()
{
}

void
IKEndSpTwistManipulator::SetAngle(float angle)
{
    Control* pControl = (Control*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
    // Set the twist angle in the param block
    pParamBlock->SetValue(IIKChainControl::kTwistHEndAngle, t, angle);
    // Update the twist spinner
    pParamBlock->GetDesc()->InvalidateUI(IIKChainControl::kTwistHEndAngle);
}

void
IKEndSpTwistManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    Control* pControl = (Control*) mhTarget;

    // Get the IIKChainControl from the controller
    IIKChainControl* pIKSys = (IIKChainControl*) pControl->GetInterface(I_IKCHAINCONTROL);
    assert(pIKSys);

    INode* pStartJoint = pIKSys->StartJoint();
    INode* pEndJoint   = pIKSys->EndJoint();
	if(!pStartJoint || !pEndJoint) return;

	INode* pLastButOneJoint = pEndJoint->GetParentNode();
    IParamBlock2 *pParamBlock = pControl->GetParamBlock(0);
    assert(pParamBlock);
	IKSolver* ikSolver = pIKSys->Solver();
    BOOL enabled;
    pParamBlock->GetValue(IIKChainControl::kTwistHEndDisplay, t, enabled, FOREVER);
	// If the ik solver it uses does not use the twist angle, we don't
	// proceed.
    if (ikSolver == NULL ||
		pStartJoint == NULL ||
		pEndJoint == NULL ||
		//!ikSolver->UseSwivelAngle() ||
		!enabled) {
        ClearPolyShapes();
        mValid = FOREVER;
        return;
    }

    Matrix3 startTM, endTM, nodeTM, pLastButOneJointTM;
    startTM = pStartJoint->GetNodeTM(t);
    endTM = pEndJoint->GetNodeTM(t);
	pLastButOneJointTM = pLastButOneJoint->GetNodeTM(t);
    assert(mpINode == pIKSys->GetNode());
    nodeTM = mpINode->GetObjectTM(t);
    Matrix3 invNodeTM = Inverse(nodeTM);

    Point3 startOrigin = startTM.GetTrans();
    Point3 endOrigin =  endTM.GetTrans();
	Point3 pLastButOneJointOrigin =  pLastButOneJointTM.GetTrans();

    float size;
    pParamBlock->GetValue(IIKChainControl::kTwistHEndSize, t, size, FOREVER);
    float length;
    pParamBlock->GetValue(IIKChainControl::kTwistHEndLength, t, length, FOREVER);

	// Put vectors/points in the parent space of the start joint
	//
//	bool inGoal = pIKSys->SwivelAngleParent() == IIKChainControl::kSAInGoal;
//	DbgAssert(inGoal ||
//			  pIKSys->SwivelAngleParent() == IIKChainControl::kSAInEndJoint);
//	Matrix3 parentTM = inGoal ? mpINode->GetParentTM(t)
	Matrix3 parentTM = pStartJoint->GetParentTM(t);
	Matrix3 invParentTM = Inverse(parentTM);
	Point3 normalVec = VectorTransform(endOrigin - pLastButOneJointOrigin, invParentTM);
	if (normalVec.LengthUnify() == 0.0f) {
	  // This is the case when the end effector coincides with the start joint.
	  // What can we do? Use an arbitrary vector:
	  normalVec.Set(1.0f, 0.0f, 0.0f);
	}
    float endTwist = pIKSys->TwistHEndAngle(t, FOREVER);
	// Given an end effector axis, there corresponds to a zero plane normal
	// that forms the reference for the swivel angle. This zero vector
	// is produced by the zero-plane-map. In general, the solver can have
	// its own map. If it doesn't, we use the default zero-plane-map, which
	// is provided by the ik-system.
	//
	const IKSys::ZeroPlaneMap* zmap =
	  ikSolver->GetZeroPlaneMap(pIKSys->InitEEAxis(t), pIKSys->InitPlane(t));
	if (zmap == NULL) {
	  zmap = pIKSys->DefaultZeroPlaneMap(t);
	}
	DbgAssert(zmap);
	Point3 upVec = (*zmap)(normalVec);
	// The above upVec is the normal to the plane. In the following, we
	// put it on the plane.
	upVec = upVec ^ normalVec;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s [%s: %5.2f]", GetString(IDS_AG_TWISTEND_ANGLE), nodeName.data(),
                   (double) RadToDeg(endTwist));

  
//	startOrigin = startOrigin * invNodeTM;
	endOrigin = endOrigin * invNodeTM;
//	Matrix3 transMat = parentTM * invNodeTM;
//	normalVec = VectorTransform(normalVec, transMat);
//	upVec = VectorTransform(upVec, transMat);


    PlaneAngleManipulator::SetValues(endOrigin,
                                     normalVec,
                                     upVec,
                                    endTwist,
									 // chainLength * (length * 0.5),
									 // Use absolute size, per 268204
									 length,
                                     size);
}

