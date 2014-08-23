/**********************************************************************
 *<
    FILE: SliderManip.cpp

    DESCRIPTION:  Manipulator for plane angles.

    CREATED BY: Scott Morrison

    HISTORY: created 5/18/00

 *> Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manip.h"
#include "Manipulator.h"
#include "iparamm2.h"
#include "Slider.h"
#include "iparamb2.h"

// This class is superceded by the scripted version.
// It is still here for old file compatibility, and as sample
// code for writing a 2d manipulator.

// The class descriptor for gsphere
class SliderClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new SliderManipulator; }
    const TCHAR *   ClassName() { return GetString(IDS_SLIDER_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return SliderManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("SliderManip"); }  
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
SliderClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    SliderManipulator* pManip = new SliderManipulator(pNode);
    pManip->SetManipTarget(hTarget);

    return pManip;
}

static SliderClassDesc SliderDesc;
ClassDesc* GetSliderManipDesc() { return &SliderDesc; }

// Parameter block description for slider manipulators

// per instance geosphere block
static ParamBlockDesc2 SliderParamBlock (
    kSliderParams,
    _T("SliderParameters"),  IDS_SLIDER_MANIP,
    &SliderDesc,
    P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_SLIDER, IDS_PARAMETERS, 0, 0, NULL,

	// params
	kSliderValue,	_T("value"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_VALUE,
		p_default,		0.0,
		p_range,		-1000.0f,1000.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_VALUE, IDC_VALUESPIN, 1.0f,
		end,
	kSliderMin,	_T("min"),   TYPE_FLOAT,			0,	IDS_MIN,
		p_default,		0.0,
		p_range,		-1000.0f,1000.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_MIN, IDC_MINSPIN, .020f,
		end,
	kSliderMax,	_T("max"),   TYPE_FLOAT,			0,	IDS_MAX,
		p_default,		100.0,
		p_range,		-1000.0f,1000.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_MAX, IDC_MAXSPIN, .020f,
		end,

	kSliderXPos,	_T("xPos"),   TYPE_FLOAT,			0,	IDS_XPOS,
		p_default,		.05,
		p_range,		0.0f,1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_POSX, IDC_POSXSPIN, .010f,
		end,
	kSliderYPos,	_T("yPos"),   TYPE_FLOAT,			0,	IDS_YPOS,
		p_default,		.05,
		p_range,		0.0f,1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_POSY, IDC_POSYSPIN, .010f,
		end,
	kSliderWidth,	_T("width"),   TYPE_FLOAT,			0,	IDS_WIDTH,
		p_default,		100.0,
		p_range,		10.0f,1000.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_WIDTH, IDC_WIDTHSPIN, 1.0f,
		end,
	kSliderHide,	_T("hide"),   TYPE_BOOL,			0,	IDS_HIDE,
		p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_HIDE, 
		end,

	kSliderName,    _T("name"),   TYPE_STRING,		    0,	IDS_NAME,
		p_ui,			TYPE_EDITBOX, IDC_NAME, 
		end,

	end
	);
// SliderManipulator for editing angles, like the IK controller swivel

SliderManipulator::SliderManipulator() :
    SimpleManipulator()
{
    GetSliderManipDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

SliderManipulator::SliderManipulator(INode* pNode) :
    SimpleManipulator(pNode)
{
    GetSliderManipDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 1.0f;
}

#define NUM_CIRC_PTS    28

void
SliderManipulator::SetValues(float value, float min, float max, float xPos,
                             float yPos, float width, BOOL hide, TCHAR* pName)
{
    TimeValue t = GetCOREInterface()->GetTime();

    theHold.Suspend();

    // Store the definitions away
    mpPblock->SetValue(kSliderValue, t, value);
    mpPblock->SetValue(kSliderMin,   t, min);
    mpPblock->SetValue(kSliderMax,   t, max);
    mpPblock->SetValue(kSliderXPos,  t, xPos);
    mpPblock->SetValue(kSliderYPos,  t, yPos);
    mpPblock->SetValue(kSliderWidth, t, width);
    mpPblock->SetValue(kSliderHide,  t, hide);
    mpPblock->SetValue(kSliderName,  t, pName);

    theHold.Resume();

    GenerateShapes(t);
}

void
SliderManipulator::SetFloatValue(float value)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        SliderManipulator* pManip = (SliderManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(kSliderValue, t, value);
        pManip->mpPblock->GetDesc()->InvalidateUI(kSliderValue);
    }
}

void
SliderManipulator::SetWidthValue(float width)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        SliderManipulator* pManip = (SliderManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(kSliderWidth, t, width);
        pManip->mpPblock->GetDesc()->InvalidateUI(kSliderWidth);
    }
}

void
SliderManipulator::SetHideValue(BOOL hide)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        SliderManipulator* pManip = (SliderManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(kSliderHide, t, hide);
        pManip->mpPblock->GetDesc()->InvalidateUI(kSliderHide);
    }
}

void
SliderManipulator::SetPositionValue(float xPos, float yPos)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        SliderManipulator* pManip = (SliderManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(kSliderXPos, t, xPos);
        pManip->mpPblock->SetValue(kSliderYPos, t, yPos);
        pManip->mpPblock->GetDesc()->InvalidateUI(kSliderXPos);
        pManip->mpPblock->GetDesc()->InvalidateUI(kSliderYPos);
    }
}

void
SliderManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    // Check for self-manipulation
    if (mhTarget) {
        SliderManipulator* pTargetManip = (SliderManipulator*) mhTarget;
        // Set all the other parameters from 
        
        float value, min, max, xPos, yPos, width;
        BOOL hide;
        TCHAR* pName;

        pTargetManip->GetPBlock()->GetValue(kSliderValue, t, value, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderMin,   t, min,   FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderMax,   t, max,   FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderXPos,  t, xPos,  FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderYPos,  t, yPos,  FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderWidth, t, width, FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderHide,  t, hide,  FOREVER);
        pTargetManip->GetPBlock()->GetValue(kSliderName,  t, pName, FOREVER);

        theHold.Suspend();
        GetPBlock()->SetValue(kSliderValue, t, value);
        GetPBlock()->SetValue(kSliderMin,   t, min);
        GetPBlock()->SetValue(kSliderMax,   t, max);
        GetPBlock()->SetValue(kSliderXPos,  t, xPos);
        GetPBlock()->SetValue(kSliderYPos,  t, yPos);
        GetPBlock()->SetValue(kSliderWidth, t, width);
        GetPBlock()->SetValue(kSliderHide,  t, hide);
        if (pName)
            GetPBlock()->SetValue(kSliderName,  t, pName);
        theHold.Resume();

        TSTR nodeName;
        nodeName = mpINode->GetName();

        toolTip = _T("");
    }

    
    GenerateShapes(t);
    
}

const int SliderManipulator::kSliderOffset = 10;

void
SliderManipulator::GenerateShapes(TimeValue t)
{
    mValid = FOREVER;

    float value, min, max, xPos, yPos, width;
    BOOL hide;
    TCHAR* pName;

    GetPBlock()->GetValue(kSliderValue, t, value, mValid);
    GetPBlock()->GetValue(kSliderMin,   t, min,   mValid);
    GetPBlock()->GetValue(kSliderMax,   t, max,   mValid);
    GetPBlock()->GetValue(kSliderXPos,  t, xPos,  mValid);
    GetPBlock()->GetValue(kSliderYPos,  t, yPos,  mValid);
    GetPBlock()->GetValue(kSliderWidth, t, width, mValid);
    GetPBlock()->GetValue(kSliderHide,  t, hide,  mValid);
    GetPBlock()->GetValue(kSliderName,  t, pName, mValid);

    ClearPolyShapes();

    ViewExp* pVpt = GetCOREInterface()->GetActiveViewport();
    GraphicsWindow* pGW = pVpt->getGW();
    int xSize = pGW->getWinSizeX();
    int ySize = pGW->getWinSizeY();
    GetCOREInterface()->ReleaseViewport(pVpt);

    xPos *= xSize;
    yPos *= ySize;

    // Create the move box
    Point3 pos(xPos, yPos, 0.0f);
    AppendMarker(HOLLOW_BOX_MRKR, pos,
                 ManipulatorGizmo::kGizmoUseScreenSpace |
                 ManipulatorGizmo::kGizmoActiveViewportOnly,
                 GetUnselectedColor());

    // Hide box
    float hidePosOffset = kSliderOffset;
    Point3 hidePos(xPos + hidePosOffset, yPos, 0.0f);
    MarkerType marker = hide ? PLUS_SIGN_MRKR : CIRCLE_MRKR;
    
    AppendMarker(marker, hidePos,
                 ManipulatorGizmo::kGizmoUseScreenSpace |
                 ManipulatorGizmo::kGizmoActiveViewportOnly,
                 GetUnselectedColor());

    if (hide)
        return;

    // Slider bar
    float barStart = xPos + 2.0f * hidePosOffset;

    Point3 barPos(barStart, yPos, 0.0f);
    Point3 barEnd(barStart + width, yPos, 0.0f);

    GizmoShape gizmo;
    gizmo.AppendPoint(barPos);
    gizmo.AppendPoint(barEnd);

    AppendGizmo(&gizmo,
                ManipulatorGizmo::kGizmoUseScreenSpace |
                ManipulatorGizmo::kGizmoActiveViewportOnly,
                GetUIColor(COLOR_SEL_GIZMOS),
                GetUIColor(COLOR_SEL_GIZMOS));

    // Slider value
    float relVal = (value - min) / (max - min);
    Point3 valPos(barStart + relVal * width, yPos + 10, 0.0f);
    

    AppendMarker(TRIANGLE_MRKR, valPos,
                 ManipulatorGizmo::kGizmoUseScreenSpace |
                 ManipulatorGizmo::kGizmoActiveViewportOnly,
                 GetUnselectedColor());


    // Resize bar
    Point3 resizePos(barStart + width + hidePosOffset, yPos, 0.0f);
    AppendMarker(DIAMOND_MRKR, resizePos,
                 ManipulatorGizmo::kGizmoUseScreenSpace |
                 ManipulatorGizmo::kGizmoActiveViewportOnly,
                 GetUnselectedColor());

    // Text label
    TSTR str;
    if (pName &&_tcslen(pName) > 0) {
        str.printf(_T("%s: %5.2f"), pName, value);
    } else {
        str.printf(_T("%5.2f"), value);
    }
    Point3 textLoc(barStart + hidePosOffset, yPos - 15, 0.0f);
    AppendText(str.data(), textLoc,
               ManipulatorGizmo::kGizmoUseScreenSpace |
               ManipulatorGizmo::kGizmoActiveViewportOnly,
               GetUnselectedColor(),
               GetUIColor(COLOR_SEL_GIZMOS));
}

class SliderManipCreateCallBack : public CreateMouseCallBack {
    IPoint2 sp0;
    SliderManipulator *mpManip;
    Point3 p0;
public:
    int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
    void SetObj(SliderManipulator *pManip) {mpManip = pManip;}
};

int SliderManipCreateCallBack::proc(ViewExp *pVpt, int msg, int point, int flags,
                                       IPoint2 m, Matrix3& mat ) {
    Point3 p1, center;

    if (msg == MOUSE_FREEMOVE)
    {
        pVpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
    }


    if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
        switch(point) {
        case 0:  // only happens with MOUSE_POINT msg
            sp0 = m;
            p0 = pVpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
            mat.SetTrans(p0);
            GraphicsWindow* pGW = pVpt->getGW();
            int xSize = pGW->getWinSizeX();
            int ySize = pGW->getWinSizeY();
            float xPos = float(m.x) / float(xSize);
            float yPos =  float(m.y) / float(ySize);
            TimeValue t = GetCOREInterface()->GetTime();
            mpManip->GetPBlock()->SetValue(kSliderXPos,  t, xPos);
            mpManip->GetPBlock()->SetValue(kSliderYPos,  t, yPos);
            SliderParamBlock.InvalidateUI();
            
            return CREATE_STOP;
        }
    } else {
        if (msg == MOUSE_ABORT) return CREATE_ABORT;
    }

    return TRUE;
}

static SliderManipCreateCallBack SliderManipCreateCB;

CreateMouseCallBack* 
SliderManipulator::GetCreateMouseCallBack()
{
    SliderManipCreateCB.SetObj(this);
    return(&SliderManipCreateCB);
}


void 
SliderManipulator::BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev)
{
    SimpleManipulator::BeginEditParams(ip, flags, prev);
    SliderDesc.BeginEditParams(ip, this, flags, prev);
}

void 
SliderManipulator::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next)
{
    SimpleManipulator::EndEditParams(ip, flags, next);
    SliderDesc.EndEditParams(ip, this, flags, next);
}

RefTargetHandle 
SliderManipulator::Clone(RemapDir& remap)
{
    SliderManipulator* pNewManip = new SliderManipulator(); 
    pNewManip->ReplaceReference(0, mpPblock->Clone(remap));
    pNewManip->mValid.SetEmpty();   
	BaseClone(this, pNewManip, remap);
    return(pNewManip);
}

// 
void
SliderManipulator::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m,
                                DWORD flags, ManipHitData* pHitData)
{
    SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
    int which = pHD->mShapeIndex;

    if (which == 1) {
        BOOL hide;
        GetPBlock()->GetValue(kSliderHide,  t, hide,  FOREVER);
        SetHideValue(!hide);
    }

    SimpleManipulator::OnButtonDown(t, pVpt, m, flags, pHitData);
}

void
SliderManipulator::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m,
                               DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) {
        SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
        int which = pHD->mShapeIndex;

        GraphicsWindow* pGW = pVpt->getGW();
        int xSize = pGW->getWinSizeX();
        int ySize = pGW->getWinSizeY();

        float value, min, max, xPos, yPos, width;
        BOOL hide;
        
        GetPBlock()->GetValue(kSliderValue, t, value, FOREVER);
        GetPBlock()->GetValue(kSliderMin,   t, min,   FOREVER);
        GetPBlock()->GetValue(kSliderMax,   t, max,   FOREVER);
        GetPBlock()->GetValue(kSliderXPos,  t, xPos,  FOREVER);
        GetPBlock()->GetValue(kSliderYPos,  t, yPos,  FOREVER);
        GetPBlock()->GetValue(kSliderWidth, t, width, FOREVER);
        GetPBlock()->GetValue(kSliderHide,  t, hide,  FOREVER);

        if (which == 0) {
            // Set postition
            float xPos = float(m.x) / float(xSize);
            float yPos =  float(m.y) / float(ySize);
            SetPositionValue(xPos, yPos);
        } else if (which == 3) {
            // Set value
            xPos *= xSize;
            float hidePosOffset = kSliderOffset;
            float barStart = xPos + 2.0f * hidePosOffset;
            float newVal = (m.x - barStart) / width;
            newVal = min + newVal * (max - min);
            if (newVal < min)
                newVal = min;
            if (newVal > max)
                newVal = max;

            SetFloatValue(newVal);
        } else if (which == 4) {
            // Set width
            xPos *= xSize;
            float hidePosOffset = kSliderOffset;
            float barStart = xPos + 2.0f * hidePosOffset;
            float newWidth = m.x - hidePosOffset - barStart;
            if (newWidth < 10.0)
                newWidth = 10.0;
            SetWidthValue(newWidth);
        }
        SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
    }
}

