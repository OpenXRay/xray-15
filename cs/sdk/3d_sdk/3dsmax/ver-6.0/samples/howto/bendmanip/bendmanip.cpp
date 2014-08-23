/**********************************************************************
 *<
	FILE: BendManip.cpp

	DESCRIPTION:	Example of a Full Manipulator.  It uses the Bend Modifier to provide
					a gizmo.  It also handles setting  up of the HitData

	CREATED BY:		DCG/QE

	HISTORY:		Oct 2000
					Nov 2000  Turned into sample by Neil Hazzard DCG

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "BendManip.h"

#define BENDMANIP_CLASS_ID	Class_ID(0x25ea8e7f, 0x29e0e4e3)

#define PBLOCK_REF	0


class BendManip;
class BendManipCreateCallBack;



ModContext* FindModContext(INode* pNode, Modifier* mod);




class BendManipCreateCallBack : public CreateMouseCallBack 
{
public:
    int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat) { return CREATE_STOP; }

}BendManipCreateCallBack_instance;



class BendManip : public Manipulator {
protected:
	SimpleMod2*	modifier;
	INode*		node;
	IPoint2		mousePos;
	BOOL		mouseWithin;

    MouseState	mState;
    Interval	mValid; // Validity of reference
	ReferenceTarget* target;
    INode*		mpINode;   // The node being manipulated
	TSTR		myName;
	IParamBlock2* pblock;

	
public:
	

	BendManip(SimpleMod2* modifier, INode* node);
    
	int HitTest(TimeValue t, INode* pNode, int type, int crossing,
                        int flags, IPoint2 *pScreenPoint, ViewExp *pVpt);
    int Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags);

    bool AlwaysActive() { return false; }

    TSTR& GetManipName() {return myName;}

    DisplayState  MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);
    DisplayState  MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);
    
    void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) {}
    void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) {}

    void SetINode(INode* pINode) { mpINode = pINode; }
    INode* GetINode() { return mpINode; }

	void DeleteThis() { delete this; }

    //from Animatable
    void GetClassName(TSTR& s) {s = GetObjectName();}		
    int NumSubs() { return 0; }  
    Animatable* SubAnim(int i) { return (i==0 ? pblock : NULL); }
    TSTR SubAnimName(int i) { return (i==0 ? GetString(IDS_PARAMS) : _T("")); }
 
	//from ReferenceMaker
    int NumRefs();
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

	//from BaseObject

	CreateMouseCallBack* GetCreateMouseCallBack(void) { return &BendManipCreateCallBack_instance; }

	//from Object
	ObjectState Eval(int);
    void InitNodeName(TSTR& s) {s = GetObjectName();}
    Interval ObjectValidity(TimeValue t);
    
    //from GeomObject
    void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
	void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL, BOOL useSel=FALSE );
    void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
};

class BendManipClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return NULL; } //{ return new BendManip(NULL); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return BENDMANIP_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName() { return _T("BendManip"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	// Returns true of the class implements a manipulator object.
    BOOL IsManipulator() { return TRUE; }

    
	BOOL CanManipulate(ReferenceTarget* hTarget) {
		if( (hTarget->ClassID() == Class_ID(BENDOSM_CLASS_ID, 0)) && (hTarget->SuperClassID() == OSM_CLASS_ID) )
			return TRUE;
		
		return FALSE;
    }

    Manipulator* CreateManipulator(ReferenceTarget* hTarget, INode* node) {
		if( hTarget->ClassID() != Class_ID(BENDOSM_CLASS_ID, 0) )
			return NULL;
		BendManip* pManip = new BendManip( (SimpleMod2*)hTarget, node );
		return pManip;
	}

};

/*****************************************************************************************************************
*
*	Helper Routine to return the ModContext of the actual modifier we are using
*
\*****************************************************************************************************************/

ModContext* FindModContext(INode* pNode, Modifier* mod) 
{
	IDerivedObject *derivObj;

	if( pNode->GetObjectRef()->SuperClassID() == GEN_DERIVOB_CLASS_ID )
		derivObj = (IDerivedObject*)pNode->GetObjectRef();
	else return NULL;

	int i=0, count=derivObj->NumModifiers();

	for( i=0; i<count; i++ )
		if( (derivObj->GetModifier(i)) == mod ) break;
	if( i==count ) return NULL; //the search failed

	return derivObj->GetModContext(i);
}


static BendManipClassDesc BendManipDesc;

ClassDesc2* GetBendManipDesc() { return &BendManipDesc; }



BendManip::BendManip(SimpleMod2 *modifier, INode *node)	: Manipulator(NULL) //(node)
{
	myName = TSTR("BendManip");
	this->modifier = modifier;
	mouseWithin = FALSE;
	this->node = node;		// Used for the NodeInvalidateRect() call
}


/***********************************************************************************************************
*
*	As this manipulator return FALSE to IsPublic() in its ClassDesc2, there is no interface to handle
*
\***********************************************************************************************************/

void BendManip::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
{
}

void BendManip::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ClearAFlag(A_OBJ_CREATING);
}

/*************************************************************************************************************
*
*	HitTest Routine  - "Borrowed from SimpleMod"  We use the actual Bend Modifier to provide
*	the hit testing.  If wanted to, you could call the modifier->HitTest Code directly
*	but you would need to be in subobject mode on the modifier
*
\**************************************************************************************************************/

int BendManip::HitTest(TimeValue t, INode* pNode, int type, int crossing,
			int flags, IPoint2 *p, ViewExp *pVpt)
{
	ModContext* mc = FindModContext( pNode, modifier );
	if( mc==NULL ) return 0;

// From SimpleMod - Removed the centre drawing section
	Interval valid;
	int savedLimits;	
	GraphicsWindow *gw = pVpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = pNode->GetObjTMBeforeWSM(t);

	if (mc->box->IsEmpty()) return 0;

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();

	//Lets get the Modifier do all the work here

	Matrix3 off, invoff;
	modmat = modifier->CompMatrix(t,*mc,ntm,valid,FALSE);
	modifier->CompOffset(t,off,invoff);
	gw->setTransform(modmat);		
	DoModifiedBox(*mc->box,modifier->GetDeformer(t,*mc,invoff,off),DrawLineProc(gw));
	

	gw->setRndLimits(savedLimits);	

	if (gw->checkHitCode()) {
		pVpt->LogHit(pNode, mc, gw->getHitDistance(), 0, NULL); 

	}

// Setup the Hit data from the Manipulator system into the HitRecord
	if(  pVpt->NumSubObjHits() ) {
		ManipHitData* pHitData = new ManipHitData(this);
		HitRecord* hit = pVpt->GetSubObjHitList().First();

		hit->hitData = pHitData; //Add the manip hit data
		return TRUE;
	}

	else 
	{
		return FALSE;
	}
}


/*************************************************************************************************************
*
*	Display Routine  - "Borrowed from SimpleMod"  We use the actual Bend Modifier to provide
*	the gizmo to be displayed.  If wanted to, you could call the modifier->Display Code 
*	directly but you would need to be in subobject mode on the modifier
*
\**************************************************************************************************************/

int BendManip::Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags)
{

	ModContext* mc = FindModContext( pNode, modifier );
	if( mc==NULL ) return 0;


	Interval valid;
	GraphicsWindow *gw = pVpt->getGW();
	Matrix3 modmat, ntm = pNode->GetObjTMBeforeWSM(t), off, invoff;

	if (mc->box->IsEmpty()) return 0;

	//Lets get the Modifier do all the work here
	modmat = modifier->CompMatrix(t,*mc,ntm,valid,FALSE);
	modifier->CompOffset(t,off,invoff);
	gw->setTransform(modmat);
	if(mouseWithin)
		gw->setColor( LINE_COLOR, (float)1.0, (float)0.0, (float)0.0);
	else
		gw->setColor( LINE_COLOR, (float)0.0, (float)1.0, (float)0.0);

	DoModifiedBox(*mc->box,modifier->GetDeformer(t,*mc,invoff,off),DrawLineProc(gw));

	
	return 1;
	
}


DisplayState BendManip::MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
{
	mousePos = m;
	mouseWithin = TRUE;
	GetCOREInterface()->NodeInvalidateRect(node);  
	return kFullRedrawNeeded;
}

DisplayState BendManip::MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
{
	mouseWithin = FALSE;
	GetCOREInterface()->NodeInvalidateRect(node);
	return kFullRedrawNeeded;
}


void BendManip::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) 
{
	int deltaX = m.x - mousePos.x;
	int deltaY = m.y - mousePos.y;

#define PB_ANGLE	0
#define PB_DIR		1

	IParamBlock2* pblock2 = modifier->pblock2;

	float angle, direction;
	if( !pblock2->GetValue( PB_ANGLE, t, angle, FOREVER ) )		return;
	if( !pblock2->GetValue( PB_DIR, t, direction, FOREVER ) )	return;

	angle += deltaY;
	direction += deltaX;

	pblock2->SetValue( PB_ANGLE, t, angle ); //ignore return value
	pblock2->SetValue( PB_DIR, t, direction ); //ignore return value

	mousePos = m;
}


// This Manipulator does not maintain any references so it simply returns zero
int BendManip::NumRefs() 
{
	return 0;
}

// Keep everybody happy by returning NULL
RefTargetHandle BendManip::GetReference(int i) 
{
	return NULL;
}

// Nothing to Set
void BendManip::SetReference(int i, RefTargetHandle rtarg) {}
	
	

ObjectState BendManip::Eval(int t) {

    return ObjectState(this);
}

Interval BendManip::ObjectValidity(TimeValue t)
{
 	return mValid;	
}





/*************************************************************************************************************
*
*	GetLocalBoundBox  - "Borrowed from SimpleMod"  This prevents unwanted clipping of the gizmo in the viewport
*
/**************************************************************************************************************/

void BendManip::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp,  Box3& box )
{
	BOOL needRelease = FALSE;

	if(inode == NULL)
		return;

	ModContext* mc = FindModContext( inode, modifier );
	
	if( mc==NULL ) 
		return;

	Interval valid;

    if (!vp) {
        vp = GetCOREInterface()->GetActiveViewport();
        needRelease = TRUE;
    }

	GraphicsWindow *gw = vp->getGW();

	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(t), off, invoff;

	if (mc->box->IsEmpty()) {
		if (needRelease)
			GetCOREInterface()->ReleaseViewport(vp);
		return;
	}

	//Lets get the Modifier do all the work here

	modmat = modifier->CompMatrix(t,*mc,ntm,valid,FALSE);
	modifier->CompOffset(t,off,invoff);	
	BoxLineProc bp1(&modmat);	
	DoModifiedBox(*mc->box, modifier->GetDeformer(t,*mc,invoff,off), bp1);
	box = bp1.Box();

	box = *(mc->box);


    if (needRelease)
        GetCOREInterface()->ReleaseViewport(vp);

	Eval(t);

}


void BendManip::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
    Matrix3 mat = inode->GetObjectTM(t);
    GetLocalBoundBox(t, inode, vpt, box);
    box = box * mat;

}

void BendManip::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
{
	GetLocalBoundBox(t, NULL, NULL, box);
}




RefResult BendManip::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message)
{
    switch (message) {
    case REFMSG_CHANGE:
        if (hTarget == pblock || hTarget == target) {
            mValid = NEVER;
        }
        break;
    }
    return REF_SUCCEED;
}







