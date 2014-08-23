#include "Manip.h"
#include "Manipulator.h"
#include "ReactorManip.h"


const float kRingScaleFactor = 400.0f;

TriObject* GetTriObjectFromNode(INode *node, int &deleteIt) {

	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(GetCOREInterface()->GetTime(), 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;

	}
}

// The class descriptor for reactor angle manipulator
// *****************************************************
class ReactorAngleManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new ReactorAngleManipulator(); }
    const TCHAR *   ClassName() { return GetString(IDS_REACTOR_ANGLE_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return ReactorAngleManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("ReactorAngleManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == REACTORPOS_CLASS_ID) ||
            (hTarget->ClassID() == REACTORROT_CLASS_ID) ||
            (hTarget->ClassID() == REACTORSCALE_CLASS_ID)  )
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static ReactorAngleManipClassDesc ReactorAngleManipDesc;

ClassDesc* GetReactorAngleManipDesc() { return &ReactorAngleManipDesc; }

Manipulator *
ReactorAngleManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
	assert( (hTarget->ClassID() == REACTORPOS_CLASS_ID) ||
            (hTarget->ClassID() == REACTORROT_CLASS_ID) ||
            (hTarget->ClassID() == REACTORSCALE_CLASS_ID) );
    IReactor* pReactor = (IReactor*) hTarget;
	if (pReactor->getReactionType() == QUAT_VAR)	
	{
		ReactorAngleManipulator* pManip = new ReactorAngleManipulator(pReactor, pNode);
		pManip->SetManipTarget(pReactor);
		return pManip;
	} else return NULL;
}

ReactorAngleManipulator::ReactorAngleManipulator(IReactor* pReactor, INode* pNode) :
    ConeAngleManipulator(pNode)
{
}

ReactorAngleManipulator::ReactorAngleManipulator():
    ConeAngleManipulator()
{
}

void
ReactorAngleManipulator::SetAngle(float angle)
{
    IReactor* pReactor = (IReactor*) mhTarget;
    TimeValue t = GetCOREInterface()->GetTime();
	if (pReactor->getReactionType() == QUAT_VAR)
	{
		pReactor->setInfluence(pReactor->getSelected(), angle);
	}
}

void
ReactorAngleManipulator::SetQuatValue(Quat angle)
{
    IReactor* pReactor = (IReactor*) mhTarget;
    //TimeValue t = GetCOREInterface()->GetTime();
	if (pReactor->getReactionType() == QUAT_VAR)
	{
		pReactor->setReactionValue(pReactor->getSelected(), angle);
	}
}

void
ReactorAngleManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{

	int i;
    IReactor* pReactor = (IReactor*) mhTarget;
	int selected = pReactor->getSelected();
 
    TSTR nodeName;
    nodeName = mpINode->GetName();

    toolTip.printf("%s %s %s: %5.2f",	nodeName.data(), 
												pReactor->getReactionName(selected),
												GetString(IDS_INFLUENCE),
												(float) RadToDeg(pReactor->getInfluence(selected)));

	Point3 origin = Point3(0,0,0);
	Matrix3 mat;//, upMat;
	Quat *qt;//, upAxis;

	qt = (Quat*)pReactor->getReactionValue(selected);
	qt->MakeMatrix(mat);
	pReactor->setUpVector(Point3(0.0f, 0.0f, 1.0f));
	Point3 direction = pReactor->getUpVector();
	//upAxis = Quat(direction, 0);
	//upAxis.MakeMatrix(upMat);
	direction = Normalize(direction * mat);
	//mat = mat * upMat;

	INode* actualNode;
	INode* node = (INode*)pReactor->GetReference(0);
    Matrix3 tm;
    tm = mpINode->GetObjectTM(t);
	if (node->ClassID() == Class_ID(BASENODE_CLASS_ID,0) )
	{
	}else 
	{
		node->NotifyDependents(FOREVER, (PartID) &nodeName, REFMSG_GET_NODE_NAME);
		actualNode = GetCOREInterface()->GetINodeByName(nodeName);
		if (actualNode) {
			origin = actualNode->GetNodeTM(t).GetTrans() * Inverse(tm);
		}
	}

	

	Box3 bb;
	Object *obj = actualNode->EvalWorldState(t).obj;
	obj->GetDeformBBox(t, bb);
	mfVectorLength = Length(bb.Width());

    float distance;
	float angle = pReactor->getInfluence(selected);

    ClearPolyShapes();
    PolyShape shape;
	
	float rad = mfVectorLength * sin(angle);
	distance = sqrt( pow(mfVectorLength,2) - pow( rad, 2) );
	float tempAngle = angle;
	int numFlips = 1;
	while (tempAngle >= PI/2.0f)
	{
		numFlips +=1;
		tempAngle -= PI; //3.0f*PI/2.0f;
	}
	if (!(numFlips%2)) distance = -distance;  //direction = -direction; //

	//SetGizmoScale(distance / 400.0f);
	
    mpPblock->SetValue(kConeAngleAngle,     t, angle);
    mpPblock->SetValue(kConeAngleDistance,  t, distance);
    mpPblock->SetValue(kConeAngleDirection, t, direction);
    mpPblock->SetValue(kConeAngleOrigin,    t, origin);

	//Append the Reaction Value manipulator
	PolyLine valueLine;
    valueLine.Init();
       
	Point3 p = origin;
	valueLine.Append(PolyPt(p));
	p = origin - Inverse(tm).GetTrans() + mfVectorLength * Normalize(direction) * Inverse(tm);
	valueLine.Append(PolyPt(p));
	shape.NewShape();
	shape.Append(valueLine);        
    AppendPolyShape(&shape, 0, GetUnselectedColor());

    Mesh* pMesh = MakeSphere(p, 0.75f/*mGizmoScale/5.0f*/, 10);
    AppendMesh(pMesh, 0/*ManipulatorGizmo::kGizmoScaleToViewport*/, GetUnselectedColor());

	//Append the radius gizmo
    pMesh = MakeTorus( Point3(0.0f, 0.0f, distance*Length(direction)), rad, 0.5f/*mGizmoScale*/, 16, 6);

	for (i=0;i < pMesh->getNumVerts(); i++)
	{
		pMesh->setVert( i, origin - Inverse(tm).GetTrans() + (pMesh->getVert(i)*mat*Inverse(tm))); //*Inverse(tm)*actualNode->GetNodeTM(t) );  //(Inverse(tm))*actualNode->GetNodeTM(t)
	}

    AppendMesh(pMesh, 0, GetUnselectedColor());
	
	//Append the wires of the cone.
	PolyLine wires;
    wires.Init();
	shape.NewShape();
	double a;

    for(i = 0; i < 4; i++) {
         
		a = (double)i * 2.0 * PI / (double)4;
       
		Point3 p = origin;
		wires.Append(PolyPt(p));
		//p = origin + (mat * Point3(rad*(float)sin(a), rad*(float)cos(a), distance*Normalize(direction).z));
		p = origin - Inverse(tm).GetTrans() + Point3(rad*(float)sin(a), rad*(float)cos(a), distance*Length(direction)) * mat * Inverse(tm); 
		wires.Append(PolyPt(p));
		shape.Append(wires);        
    }

    AppendPolyShape(&shape, 0, GetUnselectedColor());

	//Append the UpVector Manipulator
 	PolyLine upLine;
	upLine.Init();
       
	p = origin;
	upLine.Append(PolyPt(p));
	p = origin - Inverse(tm).GetTrans() + mfVectorLength * Normalize(pReactor->getUpVector()) * Inverse(tm);
	upLine.Append(PolyPt(p));
	shape.NewShape();
	shape.Append(upLine);        
    AppendPolyShape(&shape, ManipulatorGizmo::kGizmoDontHitTest, GetUIColor(COLOR_END_EFFECTOR));  

    pMesh = MakeSphere(p, 0.5f/*mGizmoScale/5.0f*/, 10);
    AppendMesh(pMesh, ManipulatorGizmo::kGizmoDontHitTest, GetUIColor(COLOR_END_EFFECTOR));  
}

static Point3 reactorManipCenter;
static float origOffset = 0.0f;
static Point3 offsetPoint;

void
ReactorAngleManipulator::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
	origOffset = 0.0f;
	offsetPoint = Point3(0.0f,0.0f,0.0f);

	Point3 direction;
	float distance;
	Point3 origin;
	mpPblock->GetValue(kConeAngleDistance,  t, distance, FOREVER);
	mpPblock->GetValue(kConeAngleDirection, t, direction, FOREVER);
	mpPblock->GetValue(kConeAngleOrigin, t, origin, FOREVER);
	reactorManipCenter = distance * Normalize(direction) + origin;
	//reactorManipCenter = origin + distance;

	SimpleManipulator::OnButtonDown(t,pVpt, m, flags, pHitData);
}


void
ReactorAngleManipulator::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) {
        // Project p back onto the plane of the manipulator circle
        
        Point3 direction;
        float distance, origAngle;
		Point3 origin;
        Interval v = FOREVER;
        
        mpPblock->GetValue(kConeAngleDistance,  t, distance, v);
        mpPblock->GetValue(kConeAngleDirection, t, direction, v);
        mpPblock->GetValue(kConeAngleOrigin, t, origin, v);
        mpPblock->GetValue(kConeAngleAngle, t, origAngle, v);
                
        Ray viewRay;  // The viewing vector in local coords
        GetLocalViewRay(pVpt, m, viewRay);
        
        Plane conePlane(Normalize(direction), reactorManipCenter);
        
        Point3 newP;
        bool res = conePlane.Intersect(viewRay, newP);
        
		IReactor* pReactor = (IReactor*) mhTarget;
		SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
        int which = pHD->mShapeIndex;

        switch (which) 
		{

			case 2:
			case 3:
				{
					float newRadius = Length(newP - reactorManipCenter);
					float curRadius = sin(origAngle)*mfVectorLength;

					if (!res || newRadius > curRadius * 1.2f)
					{
						//AF (3/26/01) if it didn't intersect the plane close to the radius, use the view coordinate space.
						conePlane = Plane(viewRay.dir, reactorManipCenter);
						res = conePlane.Intersect(viewRay, newP);
						
						if (origOffset == 0.0f) origOffset = curRadius - Length(newP - reactorManipCenter);
						newRadius = Length(newP - reactorManipCenter) + origOffset;
						if (!res) return;
					}
					if (newRadius == 0.0f)
						return;
					float extraRot = 0.0f;
					while (newRadius/mfVectorLength > 1.0f)
					{
						extraRot += PI/2;
						newRadius -= mfVectorLength;
					}
					float angle = extraRot + asin(newRadius/mfVectorLength);
					curRadius = sin(angle)*mfVectorLength;

					SetAngle(angle);
				}
				break;
			case 0:
			case 1:
				{
					conePlane = Plane(viewRay.dir, reactorManipCenter);
					res = conePlane.Intersect(viewRay, newP);
					
					if (offsetPoint == Point3(0.0f,0.0f,0.0f)) offsetPoint = newP - reactorManipCenter;

					Point3 v1 = Normalize(newP - origin - offsetPoint);
					//Point3 v1 = newP - origin;
					float v1Len = Length(v1);

					// Can't align if the child pivot is on top of our pivot
					if (v1Len==0.0f) return;

					// We want to rotate the X axis to point at the average child position
					Point3 rotAxis = CrossProd(-pReactor->getUpVector(), v1/v1Len);
					float rotAngle = (float)asin(Length(rotAxis));
					if (v1.z<0.0f) 
					{
						rotAngle = PI-rotAngle;
					}
					rotAxis = rotAxis.Normalize();

					// Turn rotation into a quat
					Quat qt = QFromAngAxis(rotAngle, rotAxis);

					SetQuatValue(qt);
				}
	
				break;
			case 4:
			case 5:
				{
				//Point3 v1 = Normalize(newP - origin);
				//pReactor->setUpVector( pReactor->getUpVector() + (center - (newP)) );
				//pReactor->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}
				break;
			default: break;

		}
    }
	SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
}



//******************************************************************
//Position Manipulator Stuff
// The class descriptor for gsphere
class PositionManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new PositionManipulator; }
    const TCHAR *   ClassName() { return GetString(IDS_POSITION_MANIP); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return PositionManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("PositionManip"); }  
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

Manipulator *
PositionManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    PositionManipulator* pManip = new PositionManipulator(pNode);
    pManip->SetManipTarget(hTarget);
    return pManip;
}

static PositionManipClassDesc PositionManipDesc;
ClassDesc* GetPositionManipDesc() { return &PositionManipDesc; }

// Parameter block description for slider manipulators

// per instance geosphere block
static ParamBlockDesc2 PositionManipParamBlock (
    PositionManipulator::kPositionManipParams,
    _T("PositionManipParameters"),  IDS_POSITION_MANIP,
    &PositionManipDesc,
    P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_POSITION_SPHERE, IDS_PARAMETERS, 0, 0, NULL,

	// params
	PositionManipulator::kPositionManipPos,	_T("position"), TYPE_POINT3,	0,	IDS_POSITION,
		p_default,      Point3(0, 0, 0), 
		p_range, 		-99999999.0, 99999999.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_POSITION_POSX, IDC_POSITION_POSXSPIN, IDC_POSITION_POSY, IDC_POSITION_POSYSPIN, IDC_POSITION_POSZ, IDC_POSITION_POSZSPIN, SPIN_AUTOSCALE, 
		end,
	PositionManipulator::kPositionManipRadius,	_T("radius"),	TYPE_FLOAT,		0,	IDS_RADIUS,
		p_default,		9.0,
		p_range,		-10000.0f,10000.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_POSITION_RADIUS, IDC_POSITION_RADIUSSPIN, SPIN_AUTOSCALE,
		end,
	end
	);

PositionManipulator::PositionManipulator() :
    SimpleManipulator()
{
    GetPositionManipDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 5.0f;
}

PositionManipulator::PositionManipulator(INode* pNode) :
    SimpleManipulator(pNode)
{
    GetPositionManipDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 5.0f;
}

void
PositionManipulator::SetPosition(Point3 pos)
{
    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        PositionManipulator* pManip = (PositionManipulator*) mhTarget;
        // Store the definitions away
        GetPBlock()->SetValue(PositionManipulator::kPositionManipPos, t, pos);
        GetPBlock()->GetDesc()->InvalidateUI(PositionManipulator::kPositionManipPos);
    }
}

void
PositionManipulator::SetRadius(float radius)
{

    // Check for self-manipulation
    if (mhTarget) {
        TimeValue t = GetCOREInterface()->GetTime();
        PositionManipulator* pManip = (PositionManipulator*) mhTarget;
        // Store the definitions away
        pManip->mpPblock->SetValue(PositionManipulator::kPositionManipRadius, t, radius);
        pManip->mpPblock->GetDesc()->InvalidateUI(PositionManipulator::kPositionManipRadius);
    }
}

void PositionManipulator::SetValues(Point3 pos, float radius)
{
	TimeValue t = GetCOREInterface()->GetTime();
    theHold.Suspend();

    // Store the definitions away
    mpPblock->SetValue(kPositionManipPos,     t, pos);
    mpPblock->SetValue(kPositionManipRadius,  t, radius);

    theHold.Resume();
	GenerateShapes(t);
}

void
PositionManipulator::UpdateShapes(TimeValue t, TSTR& toolTip)
{
    // Check for self-manipulation
    if (mhTarget) {
        PositionManipulator* pTargetManip = (PositionManipulator*) mhTarget;
        // Set all the other parameters from 
        
        Point3 pos;
		float radius;
        //TCHAR* pName;
		IParamBlock2* pb2;

		pb2 = pTargetManip->GetPBlock();
		if (pb2)
		{

			pb2->GetValue(PositionManipulator::kPositionManipPos, t, pos, FOREVER);
			pb2->GetValue(PositionManipulator::kPositionManipRadius,   t, radius,   FOREVER);

			GetPBlock()->SetValue(PositionManipulator::kPositionManipPos, t, pos);
			GetPBlock()->SetValue(PositionManipulator::kPositionManipRadius,   t, radius);
			toolTip = _T("");
		}
    }
    GenerateShapes(t);
}

/*
static INode* FindControlOwner(RefTargetHandle m) 
{
	if (!m) return NULL;
	INode *bn;
	RefMakerHandle rm;
	DependentIterator di(m);
	while (NULL!=(rm=di.Next())) 
	{
		if (rm->SuperClassID()==BASENODE_CLASS_ID) 
		{
			bn = (INode*)rm;
			return bn;
		}
		if (rm->IsRefTarget())
		{
			bn = FindControlOwner((RefTargetHandle)rm);
			if (bn)
				return bn;
		}
	}
	return NULL;
} 
*/

void
PositionManipulator::GenerateShapes(TimeValue t)
{
    mValid.SetInstant(t);

    Point3 pos;
	float radius;

	GetPBlock()->GetValue(PositionManipulator::kPositionManipPos, t, pos, mValid);
    GetPBlock()->GetValue(PositionManipulator::kPositionManipRadius,   t, radius,   mValid);

    ClearPolyShapes();
	
	INode* bn = mpINode; 
	if (bn) 
		pos = pos * Inverse(bn->GetNodeTM(t));

	//Create and append the sphere
	SetGizmoScale(radius / 3);
	AppendMarker(PLUS_SIGN_MRKR, pos, 0, GetUnselectedColor());
    //AppendMesh(MakeSphere(pos, radius/3, 12), 0, GetUIColor(COLOR_END_EFFECTOR));
}


class PositionManipCreateCallBack : public CreateMouseCallBack {
    IPoint2 sp0;
    PositionManipulator *mpManip;
    Point3 p0;
public:
    int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
    void SetObj(PositionManipulator *pManip) {mpManip = pManip;}
};

int PositionManipCreateCallBack::proc(ViewExp *pVpt, int msg, int point, int flags,
                                       IPoint2 m, Matrix3& mat ) {
    Point3 p1, center;

    if (msg == MOUSE_FREEMOVE)
    {
        pVpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
    }

	TimeValue t;
	float r;

    if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
        switch(point) {
        case 0:  // only happens with MOUSE_POINT msg
            sp0 = m;
            p0 = pVpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
            mat.SetTrans(p0);
            t = GetCOREInterface()->GetTime();
            mpManip->GetPBlock()->SetValue(PositionManipulator::kPositionManipPos, t, p0);
            mpManip->GetPBlock()->GetDesc()->InvalidateUI();
			break;
		case 1:
			mat.IdentityMatrix();
            p1 = pVpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
            r = Length(p1-p0);
            mat.SetTrans(p0);

            mpManip->GetPBlock()->SetValue(PositionManipulator::kPositionManipRadius, 0, r);
            mpManip->GetPBlock()->GetDesc()->InvalidateUI();

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

static PositionManipCreateCallBack PositionManipCreateCB;

CreateMouseCallBack* 
PositionManipulator::GetCreateMouseCallBack()
{
    PositionManipCreateCB.SetObj(this);
    return(&PositionManipCreateCB);
}


void 
PositionManipulator::BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev)
{
    SimpleManipulator::BeginEditParams(ip, flags, prev);
    PositionManipDesc.BeginEditParams(ip, this, flags, prev);
}

void 
PositionManipulator::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next)
{
    SimpleManipulator::EndEditParams(ip, flags, next);
    PositionManipDesc.EndEditParams(ip, this, flags, next);
}

RefTargetHandle 
PositionManipulator::Clone(RemapDir& remap)
{
    PositionManipulator* pNewManip = new PositionManipulator(); 
    pNewManip->ReplaceReference(0, mpPblock->Clone(remap));
    pNewManip->mValid.SetEmpty();   
	BaseClone(this, pNewManip, remap);
    return(pNewManip);
}

// 
void
PositionManipulator::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m,
                                DWORD flags, ManipHitData* pHitData)
{
    SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
    int which = pHD->mShapeIndex;

    GetPBlock()->GetValue(PositionManipulator::kPositionManipPos, t, mInitPos, FOREVER);
    if (which == 0) 
	{
		mInitPoint = m;
	} 

    SimpleManipulator::OnButtonDown(t, pVpt, m, flags, pHitData);
}

void
PositionManipulator::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m,
                               DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) {
        SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
        int which = pHD->mShapeIndex;

        if (which == 0) 
		{
 			Matrix3 ctm;
			pVpt->GetConstructionTM(ctm);
			Point3 pt = pVpt->SnapPoint(m,m,&ctm);
			pt = pt * ctm;

			Point3 origPt = pVpt->SnapPoint(mInitPoint,mInitPoint,&ctm);
			origPt = origPt * ctm;

			Point3 pos;
			pos = mInitPos + ( pt - origPt );
			GetPBlock()->SetValue(PositionManipulator::kPositionManipPos,   t, pos);
			SetPosition(pos);
		} 
		else  {}
        SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
    }
}

//Reaction Value Manipulator to represent the positional values
//*************************************************************

class ReactorPosValueManipClassDesc: public ClassDesc2 
{
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading = FALSE) { return new ReactorPositionValueManip(); }
    const TCHAR *   ClassName() { return GetString(IDS_POSITION_VALUE_MANIP_CLASS); }
    SClass_ID       SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID        ClassID() { return ReactorPosValueManip_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_MANIPULATORS); }

    const TCHAR*    InternalName() { return _T("PositionValueManip"); }  
    HINSTANCE       HInstance() { return hInstance; }
    
    // Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }
    // Returns true if the class is a manipulator and it manipulates
    // the given class ID for a base object, modifier or controller.
    BOOL CanManipulate(ReferenceTarget* hTarget) {
        if ((hTarget->ClassID() == REACTORPOS_CLASS_ID) ||
            (hTarget->ClassID() == REACTORROT_CLASS_ID) ||
            (hTarget->ClassID() == REACTORSCALE_CLASS_ID)  )
            return TRUE;
        return FALSE;
    }
        
    // If a manipulator applies to a node, this call will create
    // and instance and add it to the attached objects of the node, 
    // and will initialize the manipualtor
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);
};

static ReactorPosValueManipClassDesc ReactorPosValueManipDesc;

ClassDesc* GetReactorPosValueManipDesc() { return &ReactorPosValueManipDesc; }

Manipulator *
ReactorPosValueManipClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
	assert( (hTarget->ClassID() == REACTORPOS_CLASS_ID) ||
            (hTarget->ClassID() == REACTORROT_CLASS_ID) ||
            (hTarget->ClassID() == REACTORSCALE_CLASS_ID) );
    IReactor* pReactor = (IReactor*) hTarget;
	if (pReactor->getReactionType() == VECTOR_VAR)	
	{
		ReactorPositionValueManip* pManip = new ReactorPositionValueManip(pReactor, pNode);
		pManip->SetManipTarget(pReactor);
		return pManip;
	} else return NULL;
}

// Parameter block description for Reactor Position Value Manipulator
//**********************************************************************
static ParamBlockDesc2 ReactorPosValueManipParamBlock (
    ReactorPositionValueManip::kPositionManipParams,
    _T("ReactorPosValueManipParameters"),  IDS_POSITION_MANIP, &ReactorPosValueManipDesc, P_AUTO_CONSTRUCT, PBLOCK_REF_NO, 
	// params
	ReactorPositionValueManip::kPositionManipPosValue,	_T("value"), TYPE_POINT3_TAB, 0, 0,	IDS_POSITION,
		p_default,      Point3(0, 0, 0), 
		p_range, 		-99999999.0, 99999999.0, 
		end,
	ReactorPositionValueManip::kPositionManipRadius,	_T("radius"),	TYPE_FLOAT_TAB,	0, 0,	IDS_RADIUS,
		p_default,		9.0,
		p_range,		-10000.0f,10000.0,
		end,
	ReactorPositionValueManip::kPositionManipPosState,	_T("state"), TYPE_POINT3_TAB, 0, 0,	IDS_POSITION,
		p_default,      Point3(0, 0, 0), 
		p_range, 		-99999999.0, 99999999.0, 
		end,
	end
	);



//Postion Value Manipulator Functions
//********************************************

ReactorPositionValueManip::ReactorPositionValueManip(IReactor* pReactor, INode* pNode): 
	SimpleManipulator(pNode)
{
    GetReactorPosValueManipDesc()->MakeAutoParamBlocks(this);
	Point3* pos;
	float inf;
	for (int i=0; i<pReactor->getReactionCount(); i++)
	{
		pos = (Point3*)pReactor->getReactionValue(i);
		mpPblock->Append(ReactorPositionValueManip::kPositionManipPosValue, 1, &pos);

		inf = pReactor->getInfluence(i);
		mpPblock->Append(ReactorPositionValueManip::kPositionManipRadius, 1, &inf);

		pos = (Point3*)pReactor->getState(i);
		mpPblock->Append(ReactorPositionValueManip::kPositionManipPosState, 1, &pos);

		mGizmoScale = 5.0f;
	}
}


ReactorPositionValueManip::ReactorPositionValueManip(): 
	SimpleManipulator()
{
    GetReactorPosValueManipDesc()->MakeAutoParamBlocks(this);
    mGizmoScale = 5.0f;
}

void ReactorPositionValueManip::SetPositionValue(Point3 pos, int index)
{
    // Store the definitions away
	//theHold.Suspend();
	mpPblock->SetValue(ReactorPositionValueManip::kPositionManipPosValue, 0, pos, index);
	//theHold.Resume();
}

void ReactorPositionValueManip::SetPositionState(Point3 pos, int index)
{
    // Store the definitions away
	mpPblock->SetValue(ReactorPositionValueManip::kPositionManipPosState, 0, pos, index);
}

void ReactorPositionValueManip::SetRadius(float radius, int index)
{
    // Store the definitions away
    mpPblock->SetValue(ReactorPositionValueManip::kPositionManipRadius, 0, radius, index);
}

void ReactorPositionValueManip::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m,
                                DWORD flags, ManipHitData* pHitData)
{
    SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
    int which = pHD->mShapeIndex;

	IReactor* reactor = GetReactor();

	mInitPoint = m;
    
	switch (which)
	{
		case 0:
			mInitRadius = reactor->getInfluence(reactor->getSelected());
			break;
		case 1:
			mpPblock->GetValue(ReactorPositionValueManip::kPositionManipPosValue, t, mInitPos, FOREVER, reactor->getSelected());
			break;
		case 2:
			mpPblock->GetValue(ReactorPositionValueManip::kPositionManipPosState, t, mInitPos, FOREVER, reactor->getSelected());
			break;
		case 3:
			if (reactor->getSelected() < reactor->getReactionCount()-1)
			{
				reactor->setSelected(reactor->getSelected() + 1);
			} else reactor->setSelected(0);
			break;
		default: 
			break;
	}
    SimpleManipulator::OnButtonDown(t, pVpt, m, flags, pHitData);
}

void ReactorPositionValueManip::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
    if (GetMouseState() == kMouseDragging) 
	{
        SimpleManipHitData* pHD = (SimpleManipHitData*) pHitData;
        int which = pHD->mShapeIndex;
		
		IReactor* reactor = GetReactor();
		Point3 pos;
		
		Matrix3 ctm;
		Point3 pt;
		float radius;

		pVpt->GetConstructionTM(ctm);
		Point3 origPt = pVpt->SnapPoint(mInitPoint,mInitPoint,&ctm);

		pt = pVpt->SnapPoint(m,m,&ctm);
		//pt = pt * ctm;

		//origPt = origPt * ctm;


		switch (which)
		{
			case 0:
				radius = mInitRadius + (Length(pt - mInitPos) - Length(origPt - mInitPos));
				SetRadius(radius, GetReactor()->getSelected());
				GetReactor()->setInfluence(GetReactor()->getSelected(), radius);
				break;
			case 1:
				pt = pt - origPt;
				pos = mInitPos + pt; //( pt - origPt );
				SetPositionValue(pos, reactor->getSelected());
				reactor->setReactionValue(reactor->getSelected(), pos);
				break;
			case 2:
				pos = mInitPos + ( pt - origPt );
				SetPositionState(pos, reactor->getSelected());
				reactor->setState(reactor->getSelected(), pos);
				break;
			default:
				break;
		}
		SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
	}
}

void ReactorPositionValueManip::UpdateShapes(TimeValue t, TSTR& toolTip)
{
	IReactor* reactor = GetReactor();
	int selected = reactor->getSelected();

	Point3 pos;
	pos = *((Point3*)reactor->getReactionValue(selected));
	
    TSTR nodeName;
    nodeName = mpINode->GetName();
	//this tooltip is locking the object so it won't move (bug??)
	//toolTip.printf("%s: %s: ReactionValue = [%5f,%5f,%5f]",	nodeName.data(), 
	//											reactor->getReactionName(selected), pos.x, pos.y, pos.z);

	//Update and append the radius sphere
	float radius = reactor->getInfluence(selected);
	
	ClearPolyShapes();

	if (mpINode) 
		pos = pos * Inverse(mpINode->GetNodeTM(t));
    
	//Spherical influence manipulator
	AppendMesh(MakeSphere(pos, radius, 12), 0, GetUnselectedColor());
	//AppendMarker(PLUS_SIGN_MRKR, pos);

	//Position Value Manipulator
	AppendMesh(MakeSphere(pos, radius/5, 12),ManipulatorGizmo::kGizmoScaleToViewport, GetUIColor(COLOR_END_EFFECTOR));
	
	//State manipulators
	if (reactor->getType() == REACTORPOS)
	{
		pos = (*(Point3*)reactor->getState(selected)) * Inverse(mpINode->GetNodeTM(t));
		AppendMarker(HOLLOW_BOX_MRKR, pos, 0, GetUIColor(COLOR_MANIPULATOR_Y));
	}else 
		if (reactor->getType() == REACTORROT)
		{
			//add a rotation manipulator here 
		}

    // Reaction selection manipulator
	float xPos = .11f;
	float yPos = .86f;
    pos = Point3(xPos, yPos, 0.0f);
    AppendMarker(CIRCLE_MRKR, pos,
                 ManipulatorGizmo::kGizmoUseRelativeScreenSpace |
                 ManipulatorGizmo::kGizmoActiveViewportOnly,
                 GetUnselectedColor());

	//Current State Marker
	if (reactor->getType() == REACTORPOS)
	{
		pos = Point3(0,0,0);
		AppendMarker(DOT_MRKR, pos, ManipulatorGizmo::kGizmoDontHitTest, GetUIColor(COLOR_VERT_TICKS));
	} else if (reactor->getType() == REACTORROT)
	{
		//add a rotation manipulator here 
	}
	
	//Current value Marker
	pos = Point3(0,0,0);
	INode* actualNode;
	Animatable* anim = (Animatable*)reactor->GetReference(0);
	if (anim->ClassID() == Class_ID(BASENODE_CLASS_ID,0) )
	{
		actualNode = (INode*)anim;
	}else //it's a controller
	{
		((Control*)anim)->NotifyDependents(FOREVER, (PartID) &nodeName, REFMSG_GET_NODE_NAME);
		actualNode = GetCOREInterface()->GetINodeByName(nodeName);
	}
	if (actualNode) {
		pos = actualNode->GetNodeTM(t).GetTrans() * Inverse(mpINode->GetNodeTM(t));
	}
	AppendMesh(MakeSphere(pos, radius/5, 12),ManipulatorGizmo::kGizmoDontHitTest | ManipulatorGizmo::kGizmoScaleToViewport, GetUIColor(COLOR_VERT_TICKS));

	mValid.SetInstant(t);
}


CreateMouseCallBack* 
ReactorPositionValueManip::GetCreateMouseCallBack()
{
    return NULL;
}

