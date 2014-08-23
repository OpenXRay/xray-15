
/*****************************************************************

  This is just a header that contains all our gizmo classes and the
  Joint Gizmo classes

******************************************************************/


#ifndef __BONESDEF_GIZMO__H
#define __BONESDEF_GIZMO__H


// this is our copy buffer class for the joint gizmo
class GizmoJointBuffer : public IGizmoBuffer
{
public:
	Tab<CurvePoint> joint_graph;		//list of all our curve points for the graph control
	int counts[60];						//the number of curver points per entry above

	void DeleteThis() { delete this; }

};

// this is the joint gizmo class
class GizmoJointClass : public GizmoClass, public ResourceMakerCallback, public IGizmoClass3
	{
public:
	IObjParam *ip;		// just the an interface pointer


	GizmoJointClass();
	~GizmoJointClass();
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOSJOINT); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID ClassID() { return GIZMOJOINT_CLASSID;}    
	
	//Called when the gizmo is created to creat the name of the gizmo
	void SetInitialName();

	//returns the Gizmos Name
	TCHAR *GetName();

	//Sets the Gizmos Name
	void SetName(TCHAR *name);

	//standard Gizmo methods
    void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
    void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
	Interval LocalValidity(TimeValue t);
	void LoadResources();

	//This returns which gizmo interfaces this gizmo supports
	void* GetInterface(ULONG id);

	//some curve editor methods to set the UI
	BOOL SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl);
	BOOL GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl);

	// gizmo interface methods
	BOOL IsEnabled();			//returns whether the gizmo is active
	BOOL IsVolumeBased();		//returns whether the gizmo is volume based
	
	// returns whether a point is in the gizmo volume
	// p is the point in local space
	// tm is the matrix that transform p into world space
	BOOL IsInVolume(Point3 p, Matrix3 tm);  


	//returns whether the gizmo is currently being edited
	BOOL IsEditing();

	//toggles the enable state
	void Enable(BOOL enable);

	//this is called when the system want to end editing for some reason
	void EndEditing();
	//this when the ssytem wants the gizmo to disable all editing tools/ui
	void EnableEditing(BOOL enable);
	
	//asks the gizmo to take its current state and store it off in a copy buffer
	IGizmoBuffer *CopyToBuffer(); 

	//asks the gizmo to take the buffer passed and use that as its data
	void PasteFromBuffer(IGizmoBuffer *buffer); 

    // From Animatable
    void DeleteThis() { 
					delete this; 
	
				}


	// this creates the initial angles and returns the initial gizmo tm
	Matrix3 CreateInitialAngles();

	// this is called when the gizmo is initially created
	// it is passed to the current selected verts in the world space
	// count is the number of vertice in *p
	// *p is the list of point being affected in world space
	// numberOfInstances is the number of times this modifier has been instanced
	// mapTable is an index list into the original vertex table for *p
	BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable);


	//display and bounding box methods from IGizmo
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               
	int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm );

    void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
    void InvertSelection(int selLevel);

    int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm);
    void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
    void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE );

    void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
    void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);

	RefTargetHandle Clone(RemapDir& remap)
		{
		stopReferencing = TRUE;
		GizmoJointClass *mod = new GizmoJointClass();
		mod->ReplaceReference(0,pblock_gizmo_data->Clone(remap));
		mod->gizmoTm = gizmoTm;
		stopReferencing = FALSE;

		return mod;
		}


	//Pre and Post deform calls
	void PreDeformSetup(TimeValue t);
	void PostDeformSetup(TimeValue t);

	//deformation call
	// t the time to do the eval at
	// index the idex of the vertex that is to be deformed
	// initialP is the point in local space before any skin deformation
	// p the current point after the skin deformation in local space
	// tm the matrix to convert p into world space
	Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm);

	//keys the curve graph at the current angle
	// which is which graph is to be keyed
	// p is the value to key
	void KeyGraph(int which, Point3 p);

	//keys the curve graph 
	// which is which graph is to be keyed
	// where at which angle to key the graph
	// p is the value to key
	void KeyGraph(int which, float where, Point3 p);

	//inserts a point on the curve
	// cv the curver to insert on
	// t is where to insert
	// val is the value to insert
	void InsertInCurve(ICurve *cv,float t,float val);


//	ffd stuff	stolen from our ffd modifiers
//  I bastardized this code to make it work slightly differently by passing a spline
//  thtoug the ffd and using that to define the deformation
	Point3 pt[20], offsets[20];
	Point3		***pts;
	Point3 latticeOffsets[20];
	int dim[3];
	float m1,m2;
	Point3 GetPtOR(int i, int j, int k);
	int GridIndex(int i, int j, int k);
	void InitializeFDD(TimeValue t);
	Point3 Map(int ii, Point3 p);
	Point3 InterpSpline2(float u,Point3 *knot);


	
	float currentBendAngle; // the current angle between the parent and child bone
	
	//some bool flags to prevent the reference system to prevent message overload
	BOOL val;
	BOOL stopReferencing;

	//some matices to move points into different spaces
	//need to dig what exactly each of these do
	Matrix3 inverseParentTm0,	// the inverse of the initial parent tm so that converts from  world space to parent space
			parentTmT;			// the matrix of the parent bone at the current time, so the converts from parent space to world space

	Matrix3 gizmoTm;		// gizmo tm in parent bone space, so this goes from parent to gizmo space
	Matrix3 gizmoTmToLocal; // goes from world space to gizmo space

	Matrix3 cacheTm;		// gizmo space to world space then into initial parent space and then back into world space using the current parent tm


	// the HWND to the gizmo rollup window
	HWND hGizmoParams;


//new fix 2
	Point3 deformedPoints[21];  //list of deformed points taken out of the param block for speed

//NEW FIX
	Point3 rotationPlane;		// this defines our rotation plande
	BOOL useNewRotationPlane;	// a flag used to determine whether we can use the new rotation plane data
	void ResetRotationPlane();	// new method the system calls when it wants us to reset our rotation plane

	};



#endif

