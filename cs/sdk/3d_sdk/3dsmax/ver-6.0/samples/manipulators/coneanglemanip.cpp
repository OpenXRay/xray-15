/**********************************************************************
 *<
    FILE: ConeAngleManip.cpp

    DESCRIPTION:  Manipulator for cone angles.

    CREATED BY: Scott Morrison

    HISTORY: created 5/18/00

 *> Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manip.h"
#include "Manipulator.h"
#include "iparamm2.h"
#include "ConeAngleManip.h"
#include "lslights.h"

// The class descriptor for gsphere
class ConeAngleClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 1; }
    void *          Create(BOOL loading = FALSE) { return new ConeAngleManipulator; }
    const TCHAR *   ClassName() { return GetString(IDS_CONE_ANGLE_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return ConeAngleManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("ConeAngleManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    // This manipulator helper object manipulates itself
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        return hTarget->ClassID() == ClassID() && hTarget->SuperClassID() == SuperClassID();
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static ConeAngleClassDesc ConeAngleDesc;
ClassDesc* GetConeAngleDesc() { return &ConeAngleDesc; }

Manipulator *
ConeAngleClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    ConeAngleManipulator* pManip = new ConeAngleManipulator(pNode);
    pManip->SetManipTarget(hTarget);

    return pManip;
}

class ConeAngleManipDlgProc : public ParamMap2UserDlgProc 
{
	public:
		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg) 
			{
				case WM_INITDIALOG:
				{
					int val = 0;
					map->GetParamBlock()->GetValue(kConeAngleUseSquare, t, val, FOREVER);
					map->Enable(kConeAngleAspect, val);
				}
				break;

				case WM_COMMAND:
				break;

				case CC_SPINNER_CHANGE:
				case CC_SPINNER_BUTTONDOWN:
				case CC_SPINNER_BUTTONUP:
				break;
			}
			return FALSE;
		}

		void DeleteThis() { }
};
static ConeAngleManipDlgProc theConeAngleManipDlgProc;

// Parameter block accessor
class ConeManipAccessor : public PBAccessor
{
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == kConeAngleUseSquare)
		{
			IParamMap2* pmap = ((IParamBlock2*)((ConeAngleManipulator*)owner)->GetReference(PBLOCK_REF_NO))->GetMap();
			if (pmap) pmap->Enable(kConeAngleAspect, v.i);
		}
			
	}
};
static ConeManipAccessor theConeManipAccessor;

// Parameter block description for cone angle manipulators

// per instance coneAngleManip block
static ParamBlockDesc2 ConeAngleManipBlock
( kConeAngleParams,
  _T("ConeAngleManipParameters"),
  IDS_CONE_ANGLE_MANIP,
  &ConeAngleDesc,
  P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF_NO, 

  //rollout
  IDD_CONE_ANGLE_PARAMS,
  IDS_PARAMETERS, 0, 0, &theConeAngleManipDlgProc, 

  // params
  kConeAngleOrigin,   _T("Origin"), TYPE_POINT3, 0, IDS_ORIGIN,
  p_default,      Point3(0, 0, 0), 
  end, 

  kConeAngleDirection,   _T("Direction"), TYPE_POINT3, 0, IDS_DIRECTION,
  p_default,      Point3(0, 0, -1), 
  end, 

  kConeAngleAngle,   _T("Angle"), TYPE_ANGLE, P_ANIMATABLE, IDS_ANGLE,
  p_default,      PI/12.0, // 15 degrees
  p_range,        0.0f, 180.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ANGLE, IDC_ANGLE_SPIN, 1.0f, 
  end, 

  kConeAngleDistance,   _T("Distance"), TYPE_FLOAT, 0, IDS_DISTANCE,
  p_default,      0.0, 
  p_range,        0.0f, 10000.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISTANCE, IDC_DISTANCE_SPIN, SPIN_AUTOSCALE,
  end,

  kConeAngleUseSquare, 	_T("UseSquare"),		TYPE_BOOL, 		0,  IDS_USE_SQUARE,
  p_default,      FALSE, 
  p_ui,           TYPE_SINGLECHEKBOX, 	IDC_USE_SQUARE, 
  p_accessor,     &theConeManipAccessor,
  end, 

  kConeAngleAspect,   _T("Aspect"), TYPE_FLOAT, 0, IDS_ASPECT,
  p_default,      1.0, 
  p_range,        0.0001f, 10000.0f, 
  p_ui,           TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ASPECT, IDC_ASPECT_SPIN, SPIN_AUTOSCALE,
  p_enabled,      false,
  end,

  end
    );

// ConeAngleManipulator for editing solid angles, like a spotlight cone

ConeAngleManipulator::ConeAngleManipulator() :
    SimpleManipulator()
{
    GetConeAngleDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

ConeAngleManipulator::ConeAngleManipulator(INode *pNode) :
    SimpleManipulator(pNode)
{
    GetConeAngleDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

BOOL
ConeAngleManipulator::GetTargetPoint(TimeValue t, Point3& p) 
{
    Matrix3 tmat;
    if (mpINode->GetTargetTM(t,tmat)) {
        p = tmat.GetTrans();
        return TRUE;
    }
    else 
        return FALSE;
}

#define NUM_CIRC_PTS    28

void
ConeAngleManipulator::SetValues(Point3& origin, Point3& direction,
                                float dist, float angle, BOOL useSquare,
                                float aspect)
{
    TimeValue t = GetCOREInterface()->GetTime();

    theHold.Suspend();

    // Store the definitions away
    mpPblock->SetValue(kConeAngleAngle,     t, angle);
    mpPblock->SetValue(kConeAngleDistance,  t, dist);
    mpPblock->SetValue(kConeAngleDirection, t, direction);
    mpPblock->SetValue(kConeAngleOrigin,    t, origin);
    mpPblock->SetValue(kConeAngleUseSquare, t, useSquare);
    mpPblock->SetValue(kConeAngleAspect,    t, aspect);

    theHold.Resume();
    GenerateShapes(t);
}

void
ConeAngleManipulator::SetAngle(float angle)
{


    // Check for self-manipulation
    if (mhTarget) {

        TimeValue t = GetCOREInterface()->GetTime();
        ConeAngleManipulator* pManip = (ConeAngleManipulator*) mhTarget;
        angle = DegToRad(angle);
        pManip->mpPblock->SetValue(kConeAngleAngle,     t, angle);
        pManip->mpPblock->GetDesc()->InvalidateUI(kConeAngleAngle);
    }
}

void
ConeAngleManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    // Check for self-manipulation
    if (mhTarget) {
        ConeAngleManipulator* pTargetManip = (ConeAngleManipulator*) mhTarget;
        // Set all the other parameters from 
        
        Point3 direction;
        float distance, angle, aspect;
        BOOL useSquare;
        pTargetManip->GetPBlock()->GetValue(kConeAngleAngle,     t, angle, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kConeAngleDistance,  t, distance, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kConeAngleDirection, t, direction, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kConeAngleUseSquare, t, useSquare, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kConeAngleAspect, t, aspect, FOREVER);
        
        theHold.Suspend();
        GetPBlock()->SetValue(kConeAngleAngle,  t, angle);
        GetPBlock()->SetValue(kConeAngleDistance,  t, distance);
        GetPBlock()->SetValue(kConeAngleDirection, t, direction);
        GetPBlock()->SetValue(kConeAngleUseSquare, t, useSquare);
        GetPBlock()->SetValue(kConeAngleAspect, t, aspect);
        theHold.Resume();
        // Create a tooltip if we are manipulating something
        TSTR nodeName;
        nodeName = mpINode->GetName();
        
        toolTip.printf("%s [%s: %5.2f]", nodeName.data(), GetString(IDS_ANGLE),
                       (double) RadToDeg(angle));
    }

    
    GenerateShapes(t);
    
}

void
ConeAngleManipulator::GenerateShapes(TimeValue t)
{
    Point3 direction;

    float distance, angle, aspect;
    BOOL  useSquare;
    mValid = FOREVER;
    mpPblock->GetValue(kConeAngleAngle,     t, angle, mValid);
    mpPblock->GetValue(kConeAngleDistance,  t, distance, mValid);
    mpPblock->GetValue(kConeAngleDirection, t, direction, mValid);
    mpPblock->GetValue(kConeAngleUseSquare, t, useSquare, mValid);
    mpPblock->GetValue(kConeAngleAspect,    t, aspect, mValid);

    ClearPolyShapes();
    // Create the circle at the base of the cone

    float ta = (float)tan(0.5 * angle);   
    float rad = distance * ta;

    IManipulatorMgr* pMM = (IManipulatorMgr*) GetCOREInterface(MANIP_MGR_INTERFACE);
    assert(pMM);

    if (!useSquare) {
        // Use a circle gizmo
        GizmoShape* pGizmo = pMM->MakeCircle(Point3(0.0f, 0.0f, -distance), rad, 28);
        
        AppendGizmo(pGizmo, 0, GetUnselectedColor());
        
        delete pGizmo;

        // if a stand-alone manip, draw the struts
        if (!mhTarget) {
            pGizmo = pMM->MakeGizmoShape();
            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(rad, 0.0f, -distance));
            pGizmo->StartNewLine();

            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(-rad, 0.0f, -distance));
            pGizmo->StartNewLine();

            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(0.0f, rad, -distance));
            pGizmo->StartNewLine();

            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(0.0f, -rad, -distance));
            
            AppendGizmo(pGizmo, 0, GetUnselectedColor(), GetUnselectedColor());
            delete pGizmo;
        }
    } else {
        // Use a square gizmo
        float radAspect = rad * (float) sqrt(aspect);
        float radInvAspect = radAspect / aspect;

        GizmoShape* pGizmo = pMM->MakeGizmoShape();
        pGizmo->AppendPoint(Point3(-radAspect, -radInvAspect, -distance));
        pGizmo->AppendPoint(Point3(-radAspect,  radInvAspect, -distance));
        pGizmo->StartNewLine();
        pGizmo->AppendPoint(Point3( radAspect, -radInvAspect, -distance));
        pGizmo->AppendPoint(Point3( radAspect,  radInvAspect, -distance));
        
        AppendGizmo(pGizmo, 0, GetUnselectedColor());
        delete pGizmo;
        
        pGizmo = pMM->MakeGizmoShape();
        pGizmo->AppendPoint(Point3(-radAspect,  radInvAspect, -distance));
        pGizmo->AppendPoint(Point3( radAspect,  radInvAspect, -distance));
        pGizmo->StartNewLine();
        pGizmo->AppendPoint(Point3(-radAspect, -radInvAspect, -distance));
        pGizmo->AppendPoint(Point3( radAspect, -radInvAspect, -distance));

        AppendGizmo(pGizmo, 0, GetUnselectedColor());
        delete pGizmo;

        // if a stand-alone manip, draw the struts
        if (!mhTarget) {
            pGizmo = pMM->MakeGizmoShape();
            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(-radAspect, -radInvAspect, -distance));
            pGizmo->StartNewLine();
            
            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3(-radAspect,  radInvAspect, -distance));
            pGizmo->StartNewLine();
            
            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3( radAspect, -radInvAspect, -distance));
            pGizmo->StartNewLine();
            
            pGizmo->AppendPoint(Point3(0,0,0));
            pGizmo->AppendPoint(Point3( radAspect,  radInvAspect, -distance));
            
            AppendGizmo(pGizmo, 0, GetUnselectedColor(), GetUnselectedColor());
            delete pGizmo;
        }
    }
}

class ConeAngleManipCreateCallBack : public CreateMouseCallBack {
    IPoint2 sp0;
    ConeAngleManipulator *mpManip;
    Point3 p0;
public:
    int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
    void SetObj(ConeAngleManipulator *pManip) {mpManip = pManip;}
};

int ConeAngleManipCreateCallBack::proc(ViewExp *pVpt, int msg, int point, int flags,
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

            mpManip->GetPBlock()->SetValue(kConeAngleDistance, 0, r);
            ConeAngleManipBlock.InvalidateUI();

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

static ConeAngleManipCreateCallBack ConeAngleManipCreateCB;

CreateMouseCallBack* 
ConeAngleManipulator::GetCreateMouseCallBack()
{
    ConeAngleManipCreateCB.SetObj(this);
    return(&ConeAngleManipCreateCB);
}


void 
ConeAngleManipulator::BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev)
{
    SimpleManipulator::BeginEditParams(ip, flags, prev);
    ConeAngleDesc.BeginEditParams(ip, this, flags, prev);
}

void 
ConeAngleManipulator::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next)
{
    SimpleManipulator::EndEditParams(ip, flags, next);
    ConeAngleDesc.EndEditParams(ip, this, flags, next);
}

RefTargetHandle 
ConeAngleManipulator::Clone(RemapDir& remap)
{
    ConeAngleManipulator* pNewManip = new ConeAngleManipulator(); 
    pNewManip->ReplaceReference(0, mpPblock->Clone(remap));
    pNewManip->mValid.SetEmpty();   
	BaseClone(this, pNewManip, remap);
    return(pNewManip);
}

static float origOffset = 0.0f;

void
ConeAngleManipulator::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
	origOffset = 0.0f;
	SimpleManipulator::OnButtonDown(t,pVpt, m, flags, pHitData);
}

void
ConeAngleManipulator::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) {
        // Project p back onto the plane of the manipulator circle
        
        Point3 direction;
        float distance, aspect, curAngle;
        BOOL useSquare;

        Interval v = FOREVER;
        
        mpPblock->GetValue(kConeAngleDistance,  t, distance, v);
        mpPblock->GetValue(kConeAngleDirection, t, direction, v);
        mpPblock->GetValue(kConeAngleUseSquare, t, useSquare, v);
        mpPblock->GetValue(kConeAngleAspect,    t, aspect, v);
        mpPblock->GetValue(kConeAngleAngle,     t, curAngle, FOREVER);
       
        Point3 center = distance * Normalize(direction);
        
        Ray viewRay;  // The viewing vector in local coords
        GetLocalViewRay(pVpt, m, viewRay);
        
        Plane conePlane(direction, center);
        
        Point3 newP;
        bool res = conePlane.Intersect(viewRay, newP);
        float newRadius = Length(newP - center);
		float curRadius = distance * tan(curAngle/2.0f);

        if (!res || newRadius > curRadius * 1.2f)
		{
			//AF (3/26/01) if it didn't intersect the plane close to the radius, use the view coordinate space.
			conePlane = Plane(viewRay.dir, center);
			res = conePlane.Intersect(viewRay, newP);
			
			if (origOffset == 0.0f) origOffset = curRadius - Length(newP - center);
			newRadius = Length(newP - center) + origOffset;
            if (!res) return;
		}
        if (newRadius == 0.0f)
            return;

        if (useSquare) {
            aspect = (float) sqrt(aspect);
            int gizmoIndex = pHitData->mShapeIndex;
            if (gizmoIndex == 0) {
                float deltaX = fabs(newP.x - center.x);
                newRadius = deltaX / aspect;
            } else {
                float deltaY = fabs(newP.y - center.y);
                newRadius = deltaY * aspect;
            }
        }

        float angle = 2.0f * atan(newRadius/distance);

        angle = RadToDeg(angle);
        SetAngle(angle);

        SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
    }
}

static void
SetHotAndFall(float hotSize, float fallSize, GenLight* gl, BOOL hot)
{
    
    float tmpHot = hotSize;
    float tmpFall = fallSize;
    TimeValue t = GetCOREInterface()->GetTime();
    
    if((tmpHot > tmpFall - GetCOREInterface()->GetLightConeConstraint())) {
        if (hot)
            tmpFall = tmpHot + GetCOREInterface()->GetLightConeConstraint();
        else
            tmpHot = tmpFall - GetCOREInterface()->GetLightConeConstraint();
        gl->SetHotspot(t, tmpHot);
        gl->SetFallsize(t, tmpFall);
    }
    else {
        if(hot)
            gl->SetHotspot(t, tmpHot);
        else
            gl->SetFallsize(t, tmpFall);
    }
}

// The class descriptor for gsphere
class HotspotManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new SpotLightHotSpotManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_HOTSPOT_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return HotspotManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("HotspotManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID) ||
            (hTarget->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID) ||
			(hTarget->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID &&
			static_cast<GenLight*>(hTarget)->IsSpot()))
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static HotspotManipClassDesc HotspotManiDesc;
ClassDesc* GetHotsotManipDesc() { return &HotspotManiDesc; }

Manipulator *
HotspotManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    assert(hTarget->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
        hTarget->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0) ||
		(hTarget->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS) &&
		static_cast<GenLight*>(hTarget)->IsSpot()));
    LightObject* pLight = (LightObject*) hTarget;
    SpotLightHotSpotManipulator* pManip = new SpotLightHotSpotManipulator(pLight, pNode);

    return pManip;
}

SpotLightHotSpotManipulator::SpotLightHotSpotManipulator(LightObject* pLight, INode* pNode) :
    ConeAngleManipulator(pNode)
{
    SetManipTarget(pLight);
}

SpotLightHotSpotManipulator::SpotLightHotSpotManipulator():
    ConeAngleManipulator()
{
}

void
SpotLightHotSpotManipulator::SetAngle(float angle)
{
    GenLight* pLight = (GenLight*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    float fall = pLight->GetFallsize(t);
    SetHotAndFall(angle, fall, pLight, TRUE);
}

const float kRingScaleFactor = 400.0f;

void
SpotLightHotSpotManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    GenLight* pLight = (GenLight*) mhTarget;

    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);

    Point3 pt;
    float dist;
    BOOL b = GetTargetPoint(t, pt);

    if (!b) {
        dist = pLight->GetTDist(t);
    } else {
        float den = FLength(tm.GetRow(2));
        dist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;
    }

    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s [Hotspot: %5.2f]", nodeName.data(),
                   (double) pLight->GetHotspot(t));

    SetGizmoScale(dist / kRingScaleFactor);

    ConeAngleManipulator::SetValues(Point3(0,0,0),
                                    Point3(0,0,-1),
                                    dist,
                                    DegToRad(pLight->GetHotspot(t)),
                                    pLight->GetSpotShape() == RECT_LIGHT,
                                    pLight->GetAspect(t));
}

// The class descriptor for gsphere
class FalloffManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new SpotLightFalloffManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_FALLOFF_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return FalloffManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("FalloffManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID) ||
            (hTarget->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID) ||
			(hTarget->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS) &&
            hTarget->SuperClassID() == LIGHT_CLASS_ID &&
			static_cast<GenLight*>(hTarget)->IsSpot()))
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static FalloffManipClassDesc FalloffManiDesc;
ClassDesc* GetFalloffManipDesc() { return &FalloffManiDesc; }

Manipulator *
FalloffManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    assert(hTarget->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
        hTarget->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0) ||
		(hTarget->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS) &&
		static_cast<GenLight*>(hTarget)->IsSpot()));
    LightObject* pLight = (LightObject*) hTarget;
    SpotLightFalloffManipulator* pManip = new SpotLightFalloffManipulator(pLight, pNode);

    return pManip;
}

SpotLightFalloffManipulator::SpotLightFalloffManipulator(LightObject* pLight, INode* pNode) :
    ConeAngleManipulator(pNode)
{
    SetManipTarget(pLight);
}

SpotLightFalloffManipulator::SpotLightFalloffManipulator():
    ConeAngleManipulator()
{
}

void
SpotLightFalloffManipulator::SetAngle(float angle)
{
    GenLight* pLight = (GenLight*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    float hot = pLight->GetHotspot(t);
    SetHotAndFall(hot, angle, pLight, FALSE);
}

void
SpotLightFalloffManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    GenLight* pLight = (GenLight*) mhTarget;

    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);

    Point3 pt;
    float dist;
    BOOL b = GetTargetPoint(t, pt);

    if (!b) {
        dist = pLight->GetTDist(t);
    } else {
        float den = FLength(tm.GetRow(2));
        dist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;
    }

    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s [Falloff: %5.2f]", nodeName.data(),
                   (double) pLight->GetFallsize(t));

    SetGizmoScale(dist / kRingScaleFactor);

    ConeAngleManipulator::SetValues(Point3(0,0,0),
                                    Point3(0,0,-1),
                                    dist,
                                    DegToRad(pLight->GetFallsize(t)),
                                    pLight->GetSpotShape() == RECT_LIGHT,
                                    pLight->GetAspect(t));
}


#if 0

SpotLightFalloffManipulator::SpotLightFalloffManipulator(LightObject* pLight)
{
}

SpotLightFalloffManipulator::SpotLightFalloffManipulator():
    ConeAngleManipulator()
{
}

void
SpotLightFalloffManipulator::SetAngle(float angle)
{
    GenLight* pLight = (GenLight*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    float hot = pLight->GetFalloff(t);
    SetHotAndFall(hot, angle, pLight, FALSE);
}

void
SpotLightFalloffManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    GenLight* pLight = (GenLight*) mhTarget;

    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);

    Point3 pt;
    b = GetTargetPoint(t, pt);

    if (!b)
        return;

    float den = FLength(tm.GetRow(2));
    float dist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    tm = Inverse(tm);
    toolTip.printf("Falloff: %5.2f", (double) pLight->GetFallsize(t));

    SetGizmoScale(dist / kRingScaleFactor);

    ConeAngleManipulator::UpdateShapes(Point3(0,0,0),
                                       Point3(0,0,-1),
                                       dist,
                                       pLight->GetFallsize(t));
}


// Set the attenuation value for the light.  This mainatains the ordering
// constraint on all 4 attenuation values.
static void 
ChangeAtten(GenLight* pLight, TimeValue t, int atype, float val) 
{
    pLight->SetAtten(t, atype, val);    
    
    for (int i = atype+1; i<4; i++) {
        float f = pLight->GetAtten(t, i);
        if (val>f) {
            pLight->SetAtten(t, i, val);    
        }
    }
    for (i = atype-1; i>=0; i--) {
        float f = pLight->GetAtten(t, i);
        if (val<f) {
            pLight->SetAtten(t, i, val);    
        }
    }
}

SpotLightAttenuationManipulator::SpotLightAttenuationManipulator(LightObject* pLight, int attenType, TCHAR* pName):
    mAttenuationType(attenType), ConeDistanceManipulator(pLight), mAttenName(pName)
{
}

SpotLightAttenuationManipulator::SpotLightAttenuationManipulator():
    mAttenuationType(ATTEN1_START), ConeDistanceManipulator()
{
}

void
SpotLightAttenuationManipulator::SetDistance(float distance)
{
    GenLight* pLight = (GenLight*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
    ChangeAtten(pLight, t, mAttenuationType, distance);
}

void
SpotLightAttenuationManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    GenLight* pLight = (GenLight*) mhTarget;

    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);

    Point3 pt;
    b = GetTargetPoint(t, pt);

    if (!b)
        return;

    float dist = pLight->GetAtten(t, mAttenuationType);

    TSTR nodeName;
    nodeName = mpINode->GetName();

    float den = FLength(tm.GetRow(2));
    float targetDist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;

    toolTip.printf("%s: %5.2f", mAttenName.data(), (double) dist);

    SetGizmoScale(targetDist / 40.0);

    ConeDistanceManipulator::UpdateShapes(Point3(0,0,0),
                                          Point3(0,0,-1),
                                          dist,
                                          pLight->GetFallsize(t),
                                          false);
}


SpotLightMultiplierManipulator::SpotLightMultiplierManipulator(LightObject* pLight):
    ConeDistanceManipulator(pLight)
{
}

SpotLightMultiplierManipulator::SpotLightMultiplierManipulator():
    ConeDistanceManipulator()
{
}

void
SpotLightMultiplierManipulator::SetDistance(float distance)
{
    GenLight* pLight = (GenLight*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();

    Matrix3 tm;
    tm = mpINode->GetObjectTM(GetCOREInterface()->GetTime());

    Point3 pt;
    b = GetTargetPoint(t, pt);

    if (!b)
        return;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    float den = FLength(tm.GetRow(2));
    float targetDist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;

    float mult = 2.0 * distance / targetDist;
    pLight->SetIntensity(t, mult);
}

void
SpotLightMultiplierManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    GenLight* pLight = (GenLight*) mhTarget;

    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);

    Point3 pt;
    b = GetTargetPoint(t, pt);

    if (!b)
        return;

    TSTR nodeName;
    nodeName = mpINode->GetName();

    float den = FLength(tm.GetRow(2));
    float targetDist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;

    toolTip.printf("%s: %5.2f", "Multiplier", (double) pLight->GetIntensity(t));

    SetGizmoScale(targetDist / 40.0);

    float dist = (targetDist / 2.0) * pLight->GetIntensity(t);

    ConeDistanceManipulator::UpdateShapes(Point3(0,0,0),
                                          Point3(0,0,-1),
                                          dist,
                                          pLight->GetFallsize(t),
                                          true);
}

#endif

#if 0

ConeDistanceManipulator::ConeDistanceManipulator()
{
}

ConeDistanceManipulator::ConeDistanceManipulator(RefTargetHandle hTarget) :
    SimpleManipulator(hTarget)
{
}

void
ConeDistanceManipulator::Transform(TimeValue t, Point3& p, ViewExp* pVpt)
{
    Ray viewRay;  // The viewing vector in local coords

    IPoint2 screenPt = GetMousePos();
    pVpt->MapScreenToWorldRay((float)screenPt.x, (float)screenPt.y, viewRay);
    Matrix3 tm;

    tm = mpINode->GetObjectTM(t);

    // Transform view ray to local coords
    tm = Inverse(tm);
    viewRay.p = viewRay.p * tm;
    tm.SetTrans(Point3(0,0,0));
    viewRay.dir = viewRay.dir * tm;

    Point3 orthogViewDir = viewRay.dir;
    orthogViewDir.z = 0.0;
//    Plane& projPlane = Plane::msXZPlane.MostOrthogonal(viewRay, Plane::msYZPlane);
    Plane projPlane(orthogViewDir, mDistance * mDirection);

    Point3 newP;
    bool b = projPlane.Intersect(viewRay, newP);
    if (!b)
        return;

    float newDist = -newP.z;
    SetDistance(newDist);

    // Add a "handle"
    Point3 center(0.0f, 0.0f, -newDist);
    Plane conePlane(mDirection, center);
    bool res = conePlane.Intersect(viewRay, newP);

    if (!res)
        return;

    AddHandle(center, newP, pVpt);
}

void
ConeDistanceManipulator::UpdateShapes(Point3& origin, Point3& direction,
                                   float dist, float angle, bool useCube)
{
    ClearPolyShapes();
    // Create the circle at the base of the cone

    PolyShape shape;

//    AddCubeShape(shape, Point3(0.0f, 0.0f, -dist), mGizmoScale);
//    AppendPolyShape(&shape);
    Mesh* pMesh;
    Point3 center(0.0f, 0.0f, -dist);
    if (useCube)
        pMesh = MakeBox(center, mGizmoScale, mGizmoScale, mGizmoScale, 1, 1, 1);
    else
        pMesh = MakeSphere(center, mGizmoScale, 10);

    AppendMesh(pMesh, ManipulatorGizmo::kGizmoScaleToViewport, GetUnselectedColor());

    // Store the definitions away
    mOrigin = origin;
    mDirection = direction;
    mAngle = angle;
    mDistance = dist;
}




#endif

