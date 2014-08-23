/*===========================================================================*\
 |    File: CamMap.cpp
 |
 | Purpose: A Modifier and World Space Modifier for camera mapping.
 |
 |          This file contains two plug-ins.  A Object Space Modifier version
 |          and a World Space Modifier (Space Warp) version.
 |          The Modifier code appears after all the Space Warp code.
 |
 | History: Mark Meier, Began 12/15/96.
 |          MM, Modifier added, 12/18/96.
 |          MM, Last Change 12/21/96.
 |          michael malone (mjm) - 2.3.99
 |            made obj & ws mods have similar dlgs
 |            implemented undo/redo in objmod similar to wsmod
 |          michael malone (mjm) - 3.10.99
 |            added multi-channel support
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "Max.h"			// Main MAX include file
#include "CamMap.h"			// Resource editor include file
#include "CamMapAPI.h"

TCHAR *GetString(int id);

/*===========================================================================*\
 | Miscellaneous Defines
\*===========================================================================*/
// These are the names on the creation buttons
#define OBJ_CLASSNAME		GetString(IDS_CAMMAPCLASSNAME)
#define WSMOD_CLASSNAME		GetString(IDS_CAMERAMAP)
#define CLASSNAME			GetString(IDS_CAMERAMAP)

// These are the categories the buttons go into
#define OBJ_CATEGORY		GetString(IDS_CAMERAMAPPING)
#define WSMOD_CATEGORY		_T("")
#define MOD_CATEGORY		GetString(IDS_MAXSURFACE)

// These are the names that will appear in the Modifier stack
#define OBJ_OBJECT_NAME		GetString(IDS_CAMMAPCLASSNAME)
#define WSMOD_OBJECT_NAME	GetString(IDS_CAMERAMAPBINDING)
#define MOD_OBJECT_NAME		GetString(IDS_CAMERAMAPMODIFIER)

// This is the initial node name for the camera map apparatus object
#define INIT_NODE_NAME		GetString(IDS_CAMMAPCLASSNAME)

// The description that appears in the plug-in summary info dialog box
#define LIBDESCRIPTION		GetString(IDS_CAMMAPTITLE)

// Dialog box message title
#define MESSAGE_TITLE		GetString(IDS_CAMERAMAPPING)

// Load/Save chunk IDs
#define HAVE_CAMERA_DATA_CHUNK		1000
#define VERSION_CHUNK				1005
#define MATRIX3_CHUNK				1010
#define FOV_CHUNK					1020
#define ASPECT_CHUNK				1030
#define CHANNEL_CHUNK				1040 // mjm - 3.10.99

#define NEWDEPTH_CHUNK				1050 // watje 10-1

// This is the camera reference index
#define CAM_REF				0

// These are the default colors for the apparatus object in the viewports
#define CAMMAP_R			float(0.7)
#define CAMMAP_G			float(0.0)
#define CAMMAP_B			float(0.0)

// This is the ID of the pick command mode
#define CID_PICK_CAMERA		CID_USER+0x7055

// This is the DLL instance handle
HINSTANCE hInstance;

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

/*===========================================================================*\
 | Space Warp Code...
\*===========================================================================*/
/*===========================================================================*\
 | Class definitions
\*===========================================================================*/
class PickWSModCameraMode;
// This is the apparatus object for the space warp
class CamMapObj : public WSMObject {
  public:
	// MAX function interface pointer
	static Interface *ip;

	// Rollup page window handle
	static HWND hObjRollup;

	// This is the apparatus object's mesh that shows up in the viewports...
	Mesh mesh;

	// --- Methods From Animatable ---
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s=GetString(IDS_CAMMAPOBJECT); }
	virtual Class_ID ClassID() { return OBJ_CLASS_ID;}
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);		

	// --- Methods From ReferenceMaker ---
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// --- Methods From ReferenceTarget ---
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// --- Methods From BaseObject ---
	TCHAR *GetObjectName() { return OBJ_OBJECT_NAME; }
	CreateMouseCallBack *GetCreateMouseCallBack();
	int HitTest(TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, 
		IPoint2 *p, ViewExp *vpt);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& box);
	void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& box);

	// --- Methods From Object ---
	void InitNodeName(TSTR& s) {s = INIT_NODE_NAME;}
	int DoOwnSelectHilite() { return TRUE; }
	int IsRenderable() { return FALSE; }
	Interval ObjectValidity(TimeValue t) { return FOREVER; }
	ObjectState Eval(TimeValue t) { return ObjectState(this); }

	// --- Methods From WSMObject ---
	Modifier *CreateWSMMod(INode *node);

	// --- Methods From CamMapObj ---
	CamMapObj::CamMapObj();
	CamMapObj::~CamMapObj();
	void BuildMesh(TimeValue t, Mesh &mesh);

};

// Init the class variables (these are shared by each instance of the class).
// This is okay since only one object can be edited at a time.
Interface *CamMapObj::ip = NULL;
HWND CamMapObj::hObjRollup = NULL;

// This is the modifier portion of the space warp
class CamMapWSMod : public ICamMapWSMod {
  public:
	// MAX function interface pointer
	static Interface *ip;

	// Rollup page window handle
	static HWND hModRollup;

	// ui controls
	static ICustButton *iPick;
	static ISpinnerControl *iMapID; // mjm - 3.10.99

	// This is the command mode put into effect when the user selects
	// the iPick button.
	static PickWSModCameraMode *pickMode;

	// This is the Modifier being edited...
	static CamMapWSMod *editMod;

	// The camera node we reference
	INode *camRef;

	// the coordinate channel
	int channel; // mjm - 3.10.99

	// --- Methods From Animatable ---
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s=GetString(IDS_CAMERAMAPPINGMODIFIER); }
	virtual Class_ID ClassID() { return WSMOD_CLASS_ID;}
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);		

	// --- Methods From ReferenceMaker ---
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	int NumRefs() { return 1; } // camera node
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// --- Methods From ReferenceTarget ---
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// --- Methods From BaseObject ---
	TCHAR *GetObjectName() { return WSMOD_OBJECT_NAME; }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 

	// --- Methods From Modifier ---
	ChannelMask ChannelsUsed()
		{ return PART_GEOM|PART_TOPO|PART_SELECT|TEXMAP_CHANNEL|PART_VERTCOLOR; } // mjm - 3.10.99
	ChannelMask ChannelsChanged()
		{ return PART_VERTCOLOR|TEXMAP_CHANNEL|PART_GEOM; } // mjm - 3.10.99
	Class_ID InputType() 
		{ return Class_ID(TRIOBJ_CLASS_ID, 0); }
	void ModifyObject(TimeValue t, ModContext &mc, 
		ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// --- Methods From CamMapWSMod ---
	CamMapWSMod::CamMapWSMod();
	CamMapWSMod::~CamMapWSMod();
	BOOL SetCameraNode(INode *node);

	// --- Methods for FPMixinInterface --- 
	INode*	getCamNode();
	void	setCamNode(INode* cam);
	int		getMapChannel();
	void	setMapChannel(int mapChan);

	//Function Publishing method (Mixin Interface)
	//******************************
	BaseInterface* GetInterface(Interface_ID id) 
	{ 
		if (id == CAMMAP_WSM_INTERFACE) 
			return (ICamMapWSMod*)this; 
		else 
			return FPMixinInterface::GetInterface(id);
	} 
	//******************************
	BOOL useNewDepthMethod;

};

// Init the class variables (these are shared by each instance of the class).
// This is okay since only one modifier can be edited at a time
Interface *CamMapWSMod::ip = NULL;
HWND CamMapWSMod::hModRollup = NULL;
ICustButton *CamMapWSMod::iPick = NULL;
ISpinnerControl *CamMapWSMod::iMapID = NULL; // mjm - 3.10.99
PickWSModCameraMode *CamMapWSMod::pickMode = NULL;
CamMapWSMod *CamMapWSMod::editMod = NULL;

// This class is derived from both PickModeCallback (whose various methods
// are called at times during the picking), and PickNodeCallback (which 
// provides a Filter() method used to screen possible hits).
class PickWSModCameraMode : public PickModeCallback, public PickNodeCallback {
  public:		
	CamMapWSMod *cm;
	
	PickWSModCameraMode(CamMapWSMod *c) {cm = c;}
	// --- Methods from PickModeCallback ---
	BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt,
		IPoint2 m, int flags);
	BOOL Pick(IObjParam *ip, ViewExp *vpt);
	void EnterMode(IObjParam *ip)
		{ cm->iPick->SetCheck(TRUE); }
	void ExitMode(IObjParam *ip)
		{ if (cm->iPick) cm->iPick->SetCheck(FALSE); }
	BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
	// This class is itself derived from PickNodeCallback, so it
	// simply returns itself as the GetFilter() pointer.
	PickNodeCallback *GetFilter() { return this; }

	// --- Methods from PickNodeCallback ---
	// This method is used to screen the various nodes passed. It returns
	// TRUE if the node is valid; otherwise FALSE.
	BOOL Filter(INode *node);
};

// This is the restore object used to allow the user to undo or redo the
// selection of a new camera.
class PickWSModCameraRestore : public RestoreObj {
	public:
		CamMapWSMod *cm;
		// This constructor is called when the user has assigned a camera
		// and we register a restore object with the undo system.
		PickWSModCameraRestore(CamMapWSMod *c) { cm = c; }
// mjm - begin - 2.3.99
		void Restore(int isUndo) {
			if (cm->editMod == cm && cm->hModRollup) {
				if (cm->camRef)
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), cm->camRef->GetName());
				else
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), GetString(IDS_NONE));
			}
		}
		void Redo() {
			if (cm->editMod == cm && cm->hModRollup && cm->camRef) {
				if (cm->camRef)
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), cm->camRef->GetName());
				else
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), GetString(IDS_NONE));
			}
		}
// mjm - end
		TSTR Description() { 
			return TSTR(GetString(IDS_PICKCAMERA)); 
		}
};

// This is the material for the apparatus -- it draws itself rather than
// letting MAX do it so it can always appear in wireframe mode...
class CamMapMtl : public Material {
  public:
	CamMapMtl();
};
static CamMapMtl swMtl;

// This is the callback used in the creation of the apparatus in the viewports
class CamMapObjCreateCallBack : public CreateMouseCallBack {	
  private:
	CamMapObj *ob;	

  public:
	int proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, 
		Matrix3 &mat);
	void SetObj(CamMapObj *obj) { ob = obj; }
};
static CamMapObjCreateCallBack camMapCreateCB;

/*===========================================================================*\
 | Class Descriptors
\*===========================================================================*/
class CamMapObjClassDesc : public ClassDesc {
  public:
	int				IsPublic() {return 0;}
	void			*Create(BOOL loading = FALSE) {return new CamMapObj;}
	const TCHAR		*ClassName() {return OBJ_CLASSNAME;}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID;}
	Class_ID		ClassID() {return OBJ_CLASS_ID;}
	const TCHAR		*Category() {return OBJ_CATEGORY;}
};
static CamMapObjClassDesc camMapObjDesc;

class CamMapWSModClassDesc : public ClassDesc {
  public:
	int 			IsPublic() {return 1;}
	void			*Create(BOOL loading = FALSE) {return new CamMapWSMod;}
	const TCHAR		*ClassName() {return WSMOD_CLASSNAME;}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
	Class_ID		ClassID() {return WSMOD_CLASS_ID;}
	const TCHAR		*Category() {return WSMOD_CATEGORY;}
};
static CamMapWSModClassDesc camMapWSModDesc;

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc CamMapWSMod_interface(
    CAMMAP_WSM_INTERFACE, _T("interface"), 0, &camMapWSModDesc, FP_MIXIN,
		properties,
		CamMapWSMod::get_cam_node, CamMapWSMod::set_cam_node, _T("cameraNode"), 0, TYPE_INODE,
		CamMapWSMod::get_map_channel, CamMapWSMod::set_map_channel, _T("channel"), 0, TYPE_INT,
      end
      );

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* ICamMapWSMod::GetDesc()
  {
     return &CamMapWSMod_interface;
  }

// End of Function Publishing Code


/*===========================================================================*\
 | Dialog Procs
\*===========================================================================*/
// This is the dialog proc for the 'Current Camera Object' rollup
static INT_PTR CALLBACK CamMapWSModDlgProc(HWND hWnd, UINT msg, 
	WPARAM wParam, LPARAM lParam) {

	CamMapWSMod *cm = (CamMapWSMod *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!cm && msg != WM_INITDIALOG ) return FALSE;

	switch (msg)
	{
		case WM_INITDIALOG:
			cm = (CamMapWSMod *)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) cm);
			cm->iPick = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			cm->iPick->SetText(GetString(IDS_PICKCAMERA));
			cm->iPick->SetType(CBT_CHECK);
			cm->iPick->SetHighlightColor(GREEN_WASH);
			cm->iPick->SetCheckHighlight(TRUE);
			// Display the camera node name or "None"
			if (cm->camRef)
				SetWindowText(GetDlgItem(hWnd, IDC_CAMERA_NAME), cm->camRef->GetName());
			else
				SetWindowText(GetDlgItem(hWnd, IDC_CAMERA_NAME), GetString(IDS_NONE));
// mjm - begin - 3.10.99
			cm->iMapID = GetISpinner(GetDlgItem(hWnd,IDC_MAP_CHAN_SPIN));
			cm->iMapID->LinkToEdit(GetDlgItem(hWnd,IDC_MAP_CHAN_EDIT),EDITTYPE_INT);
			cm->iMapID->SetLimits(1, 99, FALSE);
			cm->iMapID->SetAutoScale();
			cm->iMapID->SetValue(cm->channel, FALSE);
			cm->iMapID->Enable(cm->channel);
			CheckRadioButton(hWnd, IDC_MAP_CHAN_1, IDC_MAP_CHAN_0, (cm->channel) ? IDC_MAP_CHAN_1 : IDC_MAP_CHAN_0);
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
		{
			int chan = cm->iMapID->GetIVal();
			if (chan != cm->channel)
			{
				cm->channel = chan;
				cm->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				cm->ip->RedrawViews(cm->ip->GetTime());
			}
			return TRUE;
		}
// mjm - end

		case WM_DESTROY:
			ReleaseICustButton(cm->iPick);
// mjm - begin - 3.10.99
			cm->iPick = NULL;
			ReleaseISpinner(cm->iMapID);
			cm->iMapID = NULL;
// mjm - end
			return TRUE;

		case WM_LBUTTONDOWN: case WM_LBUTTONUP:	case WM_MOUSEMOVE:
			cm->ip->RollupMouseMessage(hWnd, msg, wParam, lParam);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
// mjm - begin - 3.10.99
				case IDC_PICK:
					if (cm->ip->GetCommandMode()->ID() == CID_PICK_CAMERA)
						cm->ip->SetStdCommandMode(CID_OBJMOVE); // cancel the pick
					else
						cm->ip->SetPickMode(cm->pickMode); // do the pick
					return TRUE;

				case IDC_MAP_CHAN_1:
				case IDC_MAP_CHAN_0:
				{
					int chan = IsDlgButtonChecked(hWnd,IDC_MAP_CHAN_1);
					if (chan != cm->channel)
					{
						cm->channel = chan;
						if (cm->channel == 0)
							cm->iMapID->Enable(FALSE);
						else
						{
							cm->channel = cm->iMapID->GetIVal();
							cm->iMapID->Enable(TRUE);
						}
						cm->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
						cm->ip->RedrawViews(cm->ip->GetTime());
					}
					return TRUE;
				}
// mjm - end
			}
	}
	return FALSE; 
}

// This is the dialog proc for the 'Supports Objects of Type' rollup
INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			break;

		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd, msg, wParam, lParam);
			return FALSE;

		default: 
			return FALSE;
	}
	return TRUE;
}

/*===========================================================================*\
 | Static global functions
\*===========================================================================*/
// This is used in building the apparatus mesh
static void MakeTri(Face *f, int a, int b, int c) {
	f[0].setVerts(a,b,c);
	f[0].setSmGroup(0);
	f[0].setEdgeVisFlags(1,1,1);	
}

/*===========================================================================*\
 | CamMapObj MAX Methods
\*===========================================================================*/
// --- Methods From Animatable ---
void CamMapObj::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;
	// Add the Main rollup pages to the command panel.
	hObjRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_OBJ),
		DefaultSOTProc, GetString(IDS_SOT), (LPARAM)ip, 0);		
}

void CamMapObj::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	// Delete the rollup pages
	ip->DeleteRollupPage(hObjRollup);
	hObjRollup = NULL;
}

// --- Methods From ReferenceTarget ---
RefTargetHandle CamMapObj::Clone(RemapDir& remap) {
	CamMapObj* newob = new CamMapObj();	
	BaseClone(this, newob, remap);
	return(newob);
}

// --- Methods From BaseObject ---
int CamMapObj::HitTest(TimeValue t, INode* inode, int type, int crossing, 
	int flags, IPoint2 *p, ViewExp *vpt) {
	
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = &swMtl; 
	Matrix3 mat = inode->GetObjectTM(t);	
	gw->setTransform(mat);

	MakeHitRegion(hitRegion, type, crossing, 4, p);

	return mesh.select(gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
}

void CamMapObj::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, 
	ViewExp *vpt) {

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(tm);
	mesh.snap(gw, snap, p, tm);
}

int CamMapObj::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = &swMtl;
	Matrix3 mat = inode->GetObjectTM(t);
 	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(
		GW_WIREFRAME|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0));//removed BC 2/16/99 DB
	gw->setTransform(mat);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
//		gw->setColor( LINE_COLOR, swMtl.Kd[0], swMtl.Kd[1], swMtl.Kd[2]);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SPACE_WARPS));
	mesh.render(gw, mtl, NULL, COMP_ALL);
	gw->setRndLimits(rlim);
	return(0);
}

void CamMapObj::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, 
	Box3& box) {

	Box3 meshBox;
	Matrix3 mat = inode->GetObjectTM(t);	
	GetLocalBoundBox(t, inode, vpt, meshBox);
	box.Init();
	for(int i = 0; i < 8; i++)
		box += mat * meshBox[i];
}

void CamMapObj::GetLocalBoundBox(TimeValue t, INode* inode, 
	ViewExp* vpt, Box3& box ) {		

	box = mesh.getBoundingBox();
}

// --- Methods From WSMObject ---
// When the user binds a node to a space warp, a new modifier must be 
// created and added to the node's WSM derived object. This method creates 
// the new modifier.
Modifier *CamMapObj::CreateWSMMod(INode *node) {
	return new CamMapWSMod();
}

/*===========================================================================*\
 | CamMapWSMod MAX Methods
\*===========================================================================*/
// --- Methods From Animatable ---
void CamMapWSMod::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;

	// Create a new instance of the command mode to handle the 
	// camera node selection and initialize our pickMode pointer.
	pickMode = new PickWSModCameraMode(this);

	// Indicate this is the current Modifier being edited
	editMod = this;

	// Add the Main rollup pages to the command panel.
	hModRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_WSMOD),
		CamMapWSModDlgProc, GetString(IDS_CAMERAMAPPING), (LPARAM)this, 0);		
}

void CamMapWSMod::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	// Delete the rollup pages
	ip->DeleteRollupPage(hModRollup);
	// Clear the pick command mode from the command stack.
	ip->ClearPickMode();
	delete pickMode;
	pickMode = NULL;
	editMod = NULL;
	hModRollup = NULL;
}

// --- Methods From ReferenceMaker ---
RefTargetHandle CamMapWSMod::GetReference(int i) {
	switch(i) {
		case CAM_REF: return (RefTargetHandle)camRef;
		default: return NULL;
	}
}

void CamMapWSMod::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case CAM_REF: camRef = (INode *) rtarg; return;
	}
}

// This method is called when one of the items we reference changes.
RefResult CamMapWSMod::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message) {
	switch (message) {
		case REFMSG_TARGET_DELETED:	
			if (hTarget == camRef) {
				// The user has deleted the camera node we reference.
				// Set our reference pointer to NULL.
				camRef = NULL;
				// If the modifier edit rollup is up, make sure the 
				// camera node name reads "None".
				if (hModRollup)
					SetWindowText(GetDlgItem(hModRollup, 
						IDC_CAMERA_NAME), GetString(IDS_NONE));
			}
			break;
	}
	return REF_SUCCEED; 
}

// mjm - begin - 3.10.99
IOResult CamMapWSMod::Save(ISave* isave)
{
	ULONG nb;
	int version(3);

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&version, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(CHANNEL_CHUNK);
	isave->Write(&channel, sizeof(int), &nb);
	isave->EndChunk();

	if (useNewDepthMethod)
		{
		isave->BeginChunk(NEWDEPTH_CHUNK);
		isave->EndChunk();
		}



	return IO_OK;
}

IOResult CamMapWSMod::Load(ILoad* iload) {
	ULONG nb;
	IOResult res;

	useNewDepthMethod = FALSE;


	while ( IO_OK == (res = iload->OpenChunk()) )
	{
		switch ( iload->CurChunkID() )
		{
			case CHANNEL_CHUNK: // mjm - 3.10.99
				res=iload->Read(&channel, sizeof(int), &nb);
				break;
			case NEWDEPTH_CHUNK: // watje
				useNewDepthMethod = TRUE;
				break;


		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	return IO_OK;
}
// mjm - end

// --- Methods From ReferenceTarget ---
RefTargetHandle CamMapWSMod::Clone(RemapDir& remap) {
	// Create a new modifier and init the copied references and vars...
	CamMapWSMod *newMod = new CamMapWSMod();

	if (camRef)
		newMod->ReplaceReference(CAM_REF, camRef);
	else
		newMod->camRef = NULL;

	newMod->pickMode = NULL;
	newMod->editMod = NULL;
	BaseClone(this, newMod, remap);
	return newMod;
}

// --- Methods From Modifier ---

// The basic idea in this method is to take the vertex coordinates of the
// object this modifier is applied to, and transform these vertices into
// screen space. The X,Y screen coordinates then become the UV coordinates.
void CamMapWSMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	// If we don't have a camera yet, we can't do anything...
	if (!camRef) return;

	// Make sure this is indeed a TriObject so we can cast it as one...
	assert(os->obj->IsSubClassOf(triObjectClassID));
	TriObject *triObj = (TriObject *)os->obj;
// mjm - begin - 3.10.99
	Mesh &mesh = triObj->GetMesh();
	int nVerts( mesh.getNumVerts() );
	int nFaces( mesh.getNumFaces() );

	// Make sure the object's points are in world space. We do this
	// by multiplying by the ObjectState TM. If the points are
	// already in world space this matrix will be NULL so
	// there is no need to do this. Otherwise we will transform
	// the points by this TM thus putting them in world space.
	if (os->GetTM()) {
		Matrix3 tm = *(os->GetTM());
		for (int i=0; i<nVerts; i++)
			mesh.verts[i] = mesh.verts[i]*tm;

		// Set the geometry channel interval to be the same
		// as the ObjectState TM interval since its validity
		// now governs the interval of the modified points
		os->obj->UpdateValidity(GEOM_CHAN_NUM, os->tmValid());

		// Once the points are transformed the matrix needs to set to NULL
		os->SetTM(NULL, FOREVER);
	}

	// ensure there are tverts for the channel
	mesh.setMapSupport(channel);

	if ( nVerts != mesh.getNumMapVerts(channel) )
		mesh.setNumMapVerts(channel, nVerts);

	TVFace *tvFaces = mesh.mapFaces(channel);
	for (int i=0; i<nFaces; i++)
		tvFaces[i].setTVerts(mesh.faces[i].getAllVerts());

	// Get the transformation into camera space
	Matrix3 invCamTM = Inverse(camRef->GetObjectTM(t));

	// Create a copy of the world space vertex coordinates. These will 
	// be transformed into screen space and used as the UVWs 
	// for the TriObject.
	Point3 *UVW = (Point3 *) LocalAlloc(LPTR, nVerts*sizeof(Point3));
	for (i=0; i<nVerts; i++)
		UVW[i] = mesh.verts[i]*invCamTM;

	// Get the Field of View parameter from the camera we reference
	ObjectState camOState = camRef->EvalWorldState(t);
	Interval valid = FOREVER;
	float fov = ((CameraObject *)camOState.obj)->GetFOV(t, valid);

	// Compute the scale factors
	float xScale = -0.5f / ((float) tan(0.5*(double)fov));
	Interface *ip = GetCOREInterface();
	float aspectRatio = ip->GetRendImageAspect();
	float yScale = xScale*aspectRatio;

	// Transform the points into screen space

	float zScale = 1.0f;
	if (useNewDepthMethod)
		{
		float minZ = 0.0f;
		float maxZ = 0.0f;


		for (i=0; i<nVerts; i++) 
			{
			if ((i == 0) || (UVW[i].z < minZ))
				minZ = UVW[i].z;
			if ((i == 0) || (UVW[i].z > maxZ))
				maxZ = UVW[i].z;
			}
		zScale = maxZ-minZ;
		if (zScale == 0.0f) zScale = 1.0f;

		}


	float distance, x, y, z;
	for (i=0; i<nVerts; i++) {
		x = UVW[i].x; y = UVW[i].y; z = UVW[i].z;
		distance = (float) sqrt(x*x + y*y + z*z);
		UVW[i].x = UVW[i].x*xScale/z + 0.5f;
		UVW[i].y = UVW[i].y*yScale/z + 0.5f;
		if (useNewDepthMethod)
			UVW[i].z = UVW[i].z/zScale ;
		else UVW[i].z = distance;
	}

	// We have the UVWs ... set them into the tVerts of the mesh
	UVVert *uvVerts = mesh.mapVerts(channel);
	for (i=0; i<nVerts; i++)
		uvVerts[i] = UVW[i];

	// Free the UVWs we allocated...
	LocalFree(UVW);

	// The texture mapping depends on the geometry and topology so make sure
	// the validity interval reflects this.
	Interval iv = LocalValidity(t);

	iv &= triObj->ChannelValidity(t, GEOM_CHAN_NUM);
	iv &= triObj->ChannelValidity(t, TOPO_CHAN_NUM);
	if (!channel)
	{
		iv &= triObj->ChannelValidity (t, VERT_COLOR_CHAN_NUM);
		os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, iv);
	}
	else
	{
		iv &= triObj->ChannelValidity (t, TEXMAP_CHAN_NUM);
		os->obj->UpdateValidity(TEXMAP_CHAN_NUM, iv);
	}
}
// mjm - end

// Returns the validity of the modifier. We depend only on the camera
// validity -- its ObjectTM and its FOV parameter. If we don't have a
// camera node assigned, we are valid FOREVER.
Interval CamMapWSMod::LocalValidity(TimeValue t) {
	if (camRef) {
		Interval valid = FOREVER;
		Matrix3 tmCam = camRef->GetObjectTM(t, &valid);
		ObjectState camOState = camRef->EvalWorldState(t);
		((CameraObject *) camOState.obj)->GetFOV(t, valid);
		return valid;
	} 
	else
		return FOREVER;
}

/*===========================================================================*\
 | Camera Picking -- Various Methods
\*===========================================================================*/
BOOL PickWSModCameraMode::Filter(INode *node) {
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			SetCursor(cm->ip->GetSysCursor(SYSCUR_SELECT));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PickWSModCameraMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt,
	IPoint2 m, int flags) {

	// Here a method of class Interface is called to handle the 
	// actual node pick. If a node was found it is returned, 
	// otherwise NULL is returned.
	INode *node = cm->ip->PickNode(hWnd, m);
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			SetCursor(cm->ip->GetSysCursor(SYSCUR_SELECT));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PickWSModCameraMode::Pick(IObjParam *ip, ViewExp *vpt) {
	INode *node = vpt->GetClosestHit();
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			if (cm->SetCameraNode(node)) {
				SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), node->GetName());
				cm->ip->SetStdCommandMode(CID_OBJMOVE);
				cm->ip->RedrawViews(cm->ip->GetTime());						
			} 
			else {
				TSTR buf = GetString(IDS_ILLEGALCAM);
				MessageBox(ip->GetMAXHWnd(), buf,
					MESSAGE_TITLE, MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
	return TRUE;
}

/*===========================================================================*\
 | CamMapObj Methods
\*===========================================================================*/
CamMapObj::CamMapObj() {
	BuildMesh(TimeValue(0), mesh);
	mesh.EnableEdgeList(1);
}

CamMapObj::~CamMapObj() {
	DeleteAllRefsFromMe();
}

void CamMapObj::BuildMesh(TimeValue t,Mesh &mesh) {		
	float s = 12.0f;
	mesh.setNumVerts(4);
	mesh.setNumFaces(4);
	mesh.setVert(0,s*Point3(0.0f,1.0f,0.0f));
	mesh.setVert(1,s*Point3(-float(cos(DegToRad(30.0f))),
		-float(sin(DegToRad(30.0f))),0.0f));
	mesh.setVert(2,s*Point3(float(cos(DegToRad(30.0f))),
		-float(sin(DegToRad(30.0f))),0.0f));
	mesh.setVert(3,s*Point3(0.0f,0.0f,1.0f));
	MakeTri(&(mesh.faces[0]),2,1,0);
	MakeTri(&(mesh.faces[1]),3,0,1);
	MakeTri(&(mesh.faces[2]),3,1,2);
	MakeTri(&(mesh.faces[3]),3,2,0);
	mesh.InvalidateGeomCache();	
}

CreateMouseCallBack* CamMapObj::GetCreateMouseCallBack() {
	camMapCreateCB.SetObj(this);
	return &camMapCreateCB;
}

/*===========================================================================*\
 | CamMapWSMod Methods
\*===========================================================================*/
CamMapWSMod::CamMapWSMod() {
	channel = 1; // mjm - 3.10.99
	camRef = NULL;
	useNewDepthMethod = TRUE;
}

CamMapWSMod::~CamMapWSMod() {
	DeleteAllRefsFromMe();	
}

// This method is called to save the camera node.
BOOL CamMapWSMod::SetCameraNode(INode *node) {
	if (node->TestForLoop(FOREVER,this)==REF_SUCCEED) {
		theHold.Begin();

		// Set the camera reference to the node.
		ReplaceReference(CAM_REF, (RefTargetHandle)node);

		// Register a restore object with the undo system
		theHold.Put(new PickWSModCameraRestore(this));
		theHold.Accept(GetString(IDS_PICKCAMERA));

		// We have just changed -- notify our dependents
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		return TRUE;
	} 
	else {
		// Cyclic reference... cannot assign this node.
		return FALSE;
	}
}

INode* CamMapWSMod::getCamNode() {
	return camRef;
}

void CamMapWSMod::setCamNode(INode* cam) {
	if (cam) {
		ObjectState os = cam->EvalWorldState(0); // make sure a camera
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			if (SetCameraNode(cam)) {
				camRef = cam;
				if (hModRollup)
					SetWindowText(GetDlgItem(hModRollup, IDC_CAMERA_NAME), cam->GetName()); // mjm - 2.3.99
			}
		}	
	}
}

int CamMapWSMod::getMapChannel() {
	return channel;
}

void CamMapWSMod::setMapChannel(int mapChan) {
	channel = (mapChan < 0) ? 0 : mapChan;
	if (iMapID) {
		if (channel) {
			iMapID->Enable(TRUE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_0, FALSE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_1, TRUE);
			iMapID->SetValue(mapChan,FALSE);
		}
		else {
			iMapID->Enable(FALSE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_0, TRUE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_1, FALSE);
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

/*===========================================================================*\
 | CreateMouseCallback Methods
\*===========================================================================*/
// This method handles the user/mouse interaction when the apparatus is 
// being placed and positioned in the viewport by the user
int CamMapObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, 
	int flags, IPoint2 m, Matrix3& mat ) {

	Point3 pt;

	if ((msg == MOUSE_POINT) || (msg == MOUSE_MOVE)) {
		switch(point) {
			case 0:
				pt = vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				break;
			case 1:								
				pt = vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				if (msg == MOUSE_POINT) 
					return CREATE_STOP;
				break;
		}
	}
	else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
	}
	return TRUE;
}

/*===========================================================================*\
 | CamMapMtl Methods
\*===========================================================================*/
// Initialize the apparatus material
CamMapMtl::CamMapMtl() : Material() {
	// Diffuse color
	Kd[0] = CAMMAP_R;
	Kd[1] = CAMMAP_G;
	Kd[2] = CAMMAP_B;

	// Specular color
	Ks[0] = CAMMAP_R;
	Ks[1] = CAMMAP_G;
	Ks[2] = CAMMAP_B;
	shininess = 0.0f;

	// Rendering limit -- keep it wireframe always, and remove 
	// back facing polygons
	shadeLimit = GW_WIREFRAME|GW_BACKCULL;

	// Fully self-illuminated
	selfIllum = 1.0f;
}

/*===========================================================================*\
 | Modifier Code...
\*===========================================================================*/
/*===========================================================================*\
 | Class definitions
\*===========================================================================*/
class PickCameraMode;

class CamMapMod : public ICamMapMod { // mjm - 3.10.99
  public:
	// MAX function interface pointer
	static Interface *ip;

	// Rollup page window handle
	static HWND hModRollup;

	// ui controls
	static ICustButton *iPick;
	static ISpinnerControl *iMapID; // mjm - 3.10.99

	// This is the command mode put into effect when the user selects
	// the iPick button.
	static PickCameraMode *pickMode;

// mjm - begin - 2.3.99
	// This is the Modifier being edited...
	static CamMapMod *editMod;

	// The camera node we reference
	INode *camRef;
// mjm - end

	// the coordinate channel
	int channel; // mjm - 3.10.99

	// This flag indicates the user has choosen a camera and we have
	// retrieved the info we need from it.
	int haveCameraData;

	// The field of view of the camera in radians at the time
	// they did the shapshot
	float fov;
	
	// This is the matrix that represents the relative position of the 
	// object to the camera.
	Matrix3 mat;

	// These two are grabbed once when the camera is assigned and then
	// loaded and saved in the MAX file.
	float aspectRatio;

	// --- Methods From Animatable ---
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s=GetString(IDS_CAMERAMAPPINGMODIFIER); }
	virtual Class_ID ClassID() { return MOD_CLASS_ID; }
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);		

	// --- Methods From ReferenceMaker ---
// mjm - begin - 2.3.99
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	int NumRefs() { return 1; } // camera node
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
// mjm - end
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// --- Methods From ReferenceTarget ---
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// --- Methods From BaseObject ---
	TCHAR *GetObjectName() { return MOD_OBJECT_NAME; }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 

	// --- Methods From Modifier ---
	ChannelMask ChannelsUsed()
		{ return PART_GEOM|PART_TOPO|PART_SELECT|TEXMAP_CHANNEL|PART_VERTCOLOR; } // mjm - 3.10.99
	ChannelMask ChannelsChanged()
		{ return PART_VERTCOLOR|TEXMAP_CHANNEL; } // mjm - 3.10.99
	Class_ID InputType() 
		{ return Class_ID(TRIOBJ_CLASS_ID, 0); }
	void ModifyObject(TimeValue t, ModContext &mc, 
		ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// --- Methods From CamMapMod ---
	CamMapMod::CamMapMod();
	CamMapMod::~CamMapMod();
	BOOL StoreCameraData(INode *node, INode *myNode, TimeValue t);

	// --- Methods for FPMixinInterface --- 
	INode*	getCamNode(TimeValue t=0);
	void	setCamNode(INode* cam, TimeValue t);
	int		getMapChannel();
	void	setMapChannel(int mapChan);

	//Function Publishing method (Mixin Interface)
	//******************************
	BaseInterface* GetInterface(Interface_ID id) 
	{ 
		if (id == CAMMAP_MOD_INTERFACE) 
			return (ICamMapMod*)this; 
		else 
			return FPMixinInterface::GetInterface(id);
	} 
	//******************************

	BOOL useNewDepthMethod;
};

// Init the class variables (these are shared by each instance of the class).
// This is okay since only one modifier can be edited at a time
Interface *CamMapMod::ip = NULL;
HWND CamMapMod::hModRollup = NULL;
ICustButton *CamMapMod::iPick = NULL;
ISpinnerControl *CamMapMod::iMapID = NULL; // mjm - 3.10.99
PickCameraMode *CamMapMod::pickMode = NULL;
CamMapMod *CamMapMod::editMod = NULL; // mjm - 2.3.99

// This class is derived from both PickModeCallback (whose various methods
// are called at times during the picking), and PickNodeCallback (which 
// provides a Filter() method used to screen possible hits).
class PickCameraMode : public PickModeCallback, public PickNodeCallback {
  public:		
	CamMapMod *cm;
	
	PickCameraMode(CamMapMod *c) {cm = c;}
	// --- Methods from PickModeCallback ---
	BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt,
		IPoint2 m, int flags);
	BOOL Pick(IObjParam *ip, ViewExp *vpt);
	void EnterMode(IObjParam *ip)
		{ cm->iPick->SetCheck(TRUE); }
	void ExitMode(IObjParam *ip)
		{ if (cm->iPick) cm->iPick->SetCheck(FALSE); }
	BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }

	// This class is itself derived from PickNodeCallback, so it
	// simply returns itself as the GetFilter() pointer.
	PickNodeCallback *GetFilter() {return this;}

	// --- Methods from PickNodeCallback ---
	// This method is used to screen the various nodes passed. It returns
	// TRUE if the node is valid; otherwise FALSE.
	BOOL Filter(INode *node);
};

// mjm - begin - 2.3.99
// This is the restore object used to allow the user to undo or redo the
// selection of a new camera.
class PickModCameraRestore : public RestoreObj {
	public:
		CamMapMod *cm;
		// This constructor is called when the user has assigned a camera
		// and we register a restore object with the undo system.
		PickModCameraRestore(CamMapMod *c) { cm = c; }
		void Restore(int isUndo) {
			if (cm->editMod == cm && cm->hModRollup) {
				if (cm->camRef)
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), cm->camRef->GetName());
				else
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), GetString(IDS_NONE));
			}									
		}
		void Redo() {
			if (cm->editMod == cm && cm->hModRollup && cm->camRef) {
				if (cm->camRef)
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), cm->camRef->GetName());
				else
					SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), GetString(IDS_NONE));
			}
		}
		TSTR Description() { 
			return TSTR(GetString(IDS_PICKCAMERA)); 
		}
};
// mjm - end

/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/
class CamMapModClassDesc : public ClassDesc {
  public:
	int 			IsPublic() {return 1;}
	void			*Create(BOOL loading = FALSE) {return new CamMapMod;}
	const TCHAR		*ClassName() {return CLASSNAME;}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return MOD_CLASS_ID;}
	const TCHAR		*Category() {return MOD_CATEGORY;}
};
static CamMapModClassDesc camMapModDesc;

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc CamMapMod_interface(
    CAMMAP_MOD_INTERFACE, _T("interface"), 0, &camMapModDesc, FP_MIXIN,
		properties,
		CamMapMod::get_cam_node, CamMapMod::set_cam_node, _T("cameraNode"), 0, TYPE_INODE,
		CamMapMod::get_map_channel, CamMapMod::set_map_channel, _T("channel"), 0, TYPE_INT,
      end
      );

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* ICamMapMod::GetDesc()
  {
     return &CamMapMod_interface;
  }

// End of Function Publishing Code




/*===========================================================================*\
 | Dialog Procs
\*===========================================================================*/
// This is the dialog proc for the 'Snapshot Camera' rollup
static INT_PTR CALLBACK CamMapModDlgProc(HWND hWnd, UINT msg, 
	WPARAM wParam, LPARAM lParam) {

	CamMapMod *cm = (CamMapMod *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!cm && msg != WM_INITDIALOG ) return FALSE;

	switch (msg)
	{
		case WM_INITDIALOG:
			cm = (CamMapMod *)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) cm);
			cm->iPick = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			cm->iPick->SetText(GetString(IDS_PICKCAMERA));
			cm->iPick->SetType(CBT_CHECK);
			cm->iPick->SetHighlightColor(GREEN_WASH);
			cm->iPick->SetCheckHighlight(TRUE);
// mjm - begin - 2.3.99
			// Display the camera node name or "None"
			if (cm->camRef)
				SetWindowText(GetDlgItem(hWnd, IDC_CAMERA_NAME), cm->camRef->GetName());
			else
				SetWindowText(GetDlgItem(hWnd, IDC_CAMERA_NAME), GetString(IDS_NONE));
// mjm - end
// mjm - begin - 3.10.99
			cm->iMapID = GetISpinner(GetDlgItem(hWnd,IDC_MAP_CHAN_SPIN));
			cm->iMapID->LinkToEdit(GetDlgItem(hWnd,IDC_MAP_CHAN_EDIT),EDITTYPE_INT);
			cm->iMapID->SetLimits(1, 99, FALSE);
			cm->iMapID->SetAutoScale();	
			cm->iMapID->SetValue(cm->channel, FALSE);
			cm->iMapID->Enable(cm->channel);
			CheckRadioButton(hWnd, IDC_MAP_CHAN_1, IDC_MAP_CHAN_0, (cm->channel) ? IDC_MAP_CHAN_1 : IDC_MAP_CHAN_0);
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
		{
			int chan = cm->iMapID->GetIVal();
			if (chan != cm->channel)
			{
				cm->channel = chan;
				cm->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				cm->ip->RedrawViews(cm->ip->GetTime());
			}
			return TRUE;
		}
// mjm - end

		case WM_DESTROY:
			ReleaseICustButton(cm->iPick);
// mjm - begin - 3.10.99
			cm->iPick = NULL;
			ReleaseISpinner(cm->iMapID);
			cm->iMapID = NULL;
// mjm - end
			return TRUE;

		case WM_LBUTTONDOWN: case WM_LBUTTONUP:	case WM_MOUSEMOVE:
			cm->ip->RollupMouseMessage(hWnd, msg, wParam, lParam);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
// mjm - begin - 3.10.99
				case IDC_PICK:
					if (cm->ip->GetCommandMode()->ID() == CID_PICK_CAMERA)
						// cancel pick
						cm->ip->SetStdCommandMode(CID_OBJMOVE);
					else
						// do the pick
						cm->ip->SetPickMode(cm->pickMode);
					return TRUE;

				case IDC_MAP_CHAN_1:
				case IDC_MAP_CHAN_0:
				{
					int chan = IsDlgButtonChecked(hWnd,IDC_MAP_CHAN_1);
					if (chan != cm->channel)
					{
						cm->channel = chan;
						if (cm->channel == 0)
							cm->iMapID->Enable(FALSE);
						else
						{
							cm->channel = cm->iMapID->GetIVal();
							cm->iMapID->Enable(TRUE);
						}
						cm->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
						cm->ip->RedrawViews(cm->ip->GetTime());
					}
					return TRUE;
				}
// mjm - end
			}	
	}
	return FALSE; 
}

/*===========================================================================*\
 | CamMapMod MAX Methods
\*===========================================================================*/
// --- Methods From Animatable ---
void CamMapMod::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;

	// Create a new instance of the command mode to handle the 
	// camera node selection and initialize our pickMode pointer.
	pickMode = new PickCameraMode(this);

// mjm - begin - 2.3.99
	// Indicate this is the current Modifier being edited
	editMod = this;
// mjm - end

	// Add the Main rollup pages to the command panel.
	hModRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_MOD),
		CamMapModDlgProc, GetString(IDS_CAMERAMAPPING), (LPARAM)this, 0);		
}

void CamMapMod::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	// Delete the rollup pages
	ip->DeleteRollupPage(hModRollup);
	// Clear the pick command mode from the command stack.
	ip->ClearPickMode();
	delete pickMode;
	pickMode = NULL;
	editMod = NULL; // mjm - 2.3.99
	hModRollup = NULL;
	this->ip = NULL;
}

// --- Methods From ReferenceMaker ---
// Save the flag indicating we have the camera data, the matrix we need 
// for the transformation, and the field of view.
IOResult CamMapMod::Save(ISave* isave) {
	ULONG nb;
	int version(3); // mjm - 3.10.99

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&version, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(HAVE_CAMERA_DATA_CHUNK);
	isave->Write(&haveCameraData, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(MATRIX3_CHUNK);
	isave->Write(mat.GetAddr(), sizeof(Matrix3), &nb);
	isave->EndChunk();

	isave->BeginChunk(FOV_CHUNK);
	isave->Write(&fov, sizeof(float), &nb);
	isave->EndChunk();

	isave->BeginChunk(ASPECT_CHUNK);
	isave->Write(&aspectRatio, sizeof(float), &nb);
	isave->EndChunk();

	isave->BeginChunk(CHANNEL_CHUNK); // mjm - 3.10.99
	isave->Write(&channel, sizeof(int), &nb);
	isave->EndChunk();


	if (useNewDepthMethod)
		{
		isave->BeginChunk(NEWDEPTH_CHUNK);
		isave->EndChunk();
		}

	return IO_OK;
}

IOResult CamMapMod::Load(ILoad* iload) {
	ULONG nb;
	IOResult res;

	// Set some default values in case old files are loaded. The older
	// files didn't save these value so they won't otherwise be loaded.
	Interface *ip = GetCOREInterface();
	aspectRatio = ip->GetRendImageAspect();
	// Load the chunks...

	useNewDepthMethod = FALSE;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch(iload->CurChunkID()) {
			case HAVE_CAMERA_DATA_CHUNK:
				res=iload->Read(&haveCameraData, sizeof(int), &nb);
				break;
			case MATRIX3_CHUNK:
				res=iload->Read(mat.GetAddr(), sizeof(Matrix3), &nb);
				break;
			case FOV_CHUNK:
				res=iload->Read(&fov, sizeof(float), &nb);
				break;
			case ASPECT_CHUNK:
				res=iload->Read(&aspectRatio, sizeof(float), &nb);
				break;
			case CHANNEL_CHUNK: // mjm - 3.10.99
				res=iload->Read(&channel, sizeof(int), &nb);
				break;
			case NEWDEPTH_CHUNK: // watje
				useNewDepthMethod = TRUE;
				break;

		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	return IO_OK;
}

// --- Methods From Modifier ---

void CamMapMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	// If the user hasn't choosen a camera yet, don't do anything.
	if (!camRef) return; // mjm - 2.3.99

	// Make sure this is indeed a TriObject so we can cast it as one...
	assert(os->obj->IsSubClassOf(Class_ID(TRIOBJ_CLASS_ID, 0)));
	TriObject *triObj = (TriObject *)os->obj;
// mjm - begin - 3.10.99
	Mesh &mesh = triObj->GetMesh();
	int nVerts( mesh.getNumVerts() );
	int nFaces( mesh.getNumFaces() );

	// ensure there are tverts for the channel
	mesh.setMapSupport(channel);

	if ( nVerts != mesh.getNumMapVerts(channel) )
		mesh.setNumMapVerts(channel, nVerts);

	TVFace *tvFaces = mesh.mapFaces(channel);
	for (int i=0; i<nFaces; i++)
		tvFaces[i].setTVerts(mesh.faces[i].getAllVerts());

	// Create a copy of the vertex coordinates and transform them.
	// This first part of the transformation puts the coords into 
	// the space of the camera 
	Point3 *UVW = (Point3 *) LocalAlloc(LPTR, nVerts*sizeof(Point3));
	for (i=0; i<nVerts; i++)
		UVW[i] = mesh.verts[i]*mat;

	// Compute the scale factors
	float xScale = -0.5f / ((float) tan(0.5*(double)fov));
	float yScale = xScale*aspectRatio;

	// Transform the points into screen space
	float zScale = 1.0f;
	if (useNewDepthMethod)
		{
		float minZ = 0.0f;
		float maxZ = 0.0f;


		for (i=0; i<nVerts; i++) 
			{
			if ((i == 0) || (UVW[i].z < minZ))
				minZ = UVW[i].z;
			if ((i == 0) || (UVW[i].z > maxZ))
				maxZ = UVW[i].z;
			}
		zScale = maxZ-minZ;
		if (zScale == 0.0f) zScale = 1.0f;

		}

	float distance, x, y, z;
	for (i=0; i<nVerts; i++) {
		x = UVW[i].x; y = UVW[i].y; z = UVW[i].z;
		distance = (float) sqrt(x*x + y*y + z*z);
		UVW[i].x = UVW[i].x*xScale/z + 0.5f;
		UVW[i].y = UVW[i].y*yScale/z + 0.5f;
		if (useNewDepthMethod)
			UVW[i].z = UVW[i].z/zScale ;
		else UVW[i].z = distance;
	}

	// We have the UVWs ... set them into the tVerts of the mesh
	UVVert *uvVerts = mesh.mapVerts(channel);
	for (i=0; i<nVerts; i++)
		uvVerts[i] = UVW[i];

	// Free the UVWs we allocated...
	LocalFree(UVW);

	// The texture mapping depends on the geometry and topology so make sure 
	// the validity interval reflects this.
	Interval iv = LocalValidity(t);

	iv &= triObj->ChannelValidity(t, GEOM_CHAN_NUM);
	iv &= triObj->ChannelValidity(t, TOPO_CHAN_NUM);
	if (!channel)
	{
		iv &= triObj->ChannelValidity (t, VERT_COLOR_CHAN_NUM);
		os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, iv);
	}
	else
	{
		iv &= triObj->ChannelValidity (t, TEXMAP_CHAN_NUM);
		os->obj->UpdateValidity(TEXMAP_CHAN_NUM, iv);
	}
}
// mjm - end

// Returns the validity of the modifier.
Interval CamMapMod::LocalValidity(TimeValue t) {
	return FOREVER;
}

// --- Methods From ReferenceMaker ---
RefTargetHandle CamMapMod::GetReference(int i) {
	switch(i) {
		case CAM_REF: return (RefTargetHandle)camRef;
		default: return NULL;
	}
}

void CamMapMod::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case CAM_REF: camRef = (INode *) rtarg; return;
	}
}

// This method is called when one of the items we reference changes.
RefResult CamMapMod::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message) {
	switch (message) {
		case REFMSG_TARGET_DELETED:	
			if (hTarget == camRef) {
				// The user has deleted the camera node we reference.
				// Set our reference pointer to NULL.
				camRef = NULL;
				// If the modifier edit rollup is up, make sure the 
				// camera node name reads "None".
				if (hModRollup)
					SetWindowText(GetDlgItem(hModRollup, 
						IDC_CAMERA_NAME), GetString(IDS_NONE));
			}
			break;
	}
	return REF_SUCCEED; 
}

// --- Methods From ReferenceTarget ---
RefTargetHandle CamMapMod::Clone(RemapDir& remap) {
	// Create a new modifier and init the copied vars...
	CamMapMod *newMod = new CamMapMod();

	if (camRef)
		newMod->ReplaceReference(CAM_REF, camRef);
	else
		newMod->camRef = NULL;

	newMod->pickMode = NULL;
	newMod->editMod = NULL; // mjm - 2.3.99
	newMod->haveCameraData = haveCameraData;
	newMod->fov = fov;
	newMod->mat = mat;
	BaseClone(this, newMod, remap);
	return newMod;
}

// --- Methods From CamMapMod ---
CamMapMod::CamMapMod() {
	channel = 1; // mjm - 3.10.99
	camRef = NULL; // mjm - 2.3.99
	haveCameraData = FALSE;
	useNewDepthMethod = TRUE;
}

CamMapMod::~CamMapMod() {
	DeleteAllRefsFromMe();	
}

BOOL CamMapMod::StoreCameraData(INode *camNode, INode* myNode, TimeValue currentTime) {
	ModContextList mcList;
	INodeTab nodeTab;
	Interval valid = FOREVER;
	Matrix3 objMat, camMat;

// mjm - begin - 2.3.99
	if (camNode->TestForLoop(FOREVER,this)==REF_SUCCEED) {
		theHold.Begin();

		// Set the camera reference to the node.
		ReplaceReference(CAM_REF, (RefTargetHandle)camNode);

		// Register a restore object with the undo system
		theHold.Put(new PickModCameraRestore(this));
		theHold.Accept(GetString(IDS_PICKCAMERA));
// mjm - end

		// Get the INode* of the _first_ object we modify. Anyone in their 
		// right mind will apply this to only one object, but since it's an
		// OSM they can instance the modifier...
		INode *n0;
		if (myNode == NULL) {
			ip->GetModContexts(mcList, nodeTab);
			n0 = nodeTab[0];
		}
		else
			n0=myNode;

		// Get the matrix to transform the objects points to world space
		objMat = n0->GetObjectTM(currentTime, &valid);
		// Get the matrix to transform the camera into world space
		camMat = camNode->GetObjectTM(currentTime, &valid);

		// Create the matrix that represents the relative position of the 
		// object to the camera.
		mat = objMat*Inverse(camMat);

		// Get the field of view from the camera
		ObjectState camOState = camNode->EvalWorldState(currentTime);
		fov = ((CameraObject *) camOState.obj)->GetFOV(currentTime, valid);

		// Set the flag to indicate we got it...
		haveCameraData = TRUE;

		// Grab the aspect and pixel aspect ratios
		aspectRatio = GetCOREInterface()->GetRendImageAspect();

		// We have just changed -- notify our dependents
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		return TRUE;
// mjm - begin - 2.3.99
	} 
	else {
		// Cyclic reference... cannot assign this node.
		return FALSE;
	}
// mjm - end
}

INode* CamMapMod::getCamNode(TimeValue t) {
	return camRef;
}

void CamMapMod::setCamNode(INode* cam, TimeValue t) {
	if (cam) {
		ObjectState os = cam->EvalWorldState(0); // make sure a camera
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			INode* myNode=NULL;
			if (ip==NULL) {// if ip != NULL, up in modify panel
				myNode = find_scene_obj_node((ReferenceTarget*)this); 
				if (!myNode) return;
			}
			if (StoreCameraData(cam, myNode, t)) {
				camRef = cam;
				if (hModRollup)
					SetWindowText(GetDlgItem(hModRollup, IDC_CAMERA_NAME), cam->GetName()); // mjm - 2.3.99
			}
		}	
	}
}

int CamMapMod::getMapChannel() {
	return channel;
}

void CamMapMod::setMapChannel(int mapChan) {
	channel = (mapChan < 0) ? 0 : mapChan;
	if (iMapID) {
		if (channel) {
			iMapID->Enable(TRUE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_0, FALSE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_1, TRUE);
			iMapID->SetValue(mapChan,FALSE);
		}
		else {
			iMapID->Enable(FALSE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_0, TRUE);
			CheckDlgButton(hModRollup, IDC_MAP_CHAN_1, FALSE);
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

/*===========================================================================*\
 | Camera Picking -- Various Methods
\*===========================================================================*/
BOOL PickCameraMode::Filter(INode *node) {
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			SetCursor(cm->ip->GetSysCursor(SYSCUR_SELECT));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PickCameraMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt,
	IPoint2 m, int flags) {

	// Here a method of class Interface is called to handle the 
	// actual node pick. If a node was found it is returned, 
	// otherwise NULL is returned.
	INode *node = cm->ip->PickNode(hWnd, m);
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			SetCursor(cm->ip->GetSysCursor(SYSCUR_SELECT));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PickCameraMode::Pick(IObjParam *ip, ViewExp *vpt) {
	INode *node = vpt->GetClosestHit();
	if (node) {
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID() == CAMERA_CLASS_ID) {
			if (cm->StoreCameraData(node, NULL, ip->GetTime())) {
				SetWindowText(GetDlgItem(cm->hModRollup, IDC_CAMERA_NAME), node->GetName()); // mjm - 2.3.99
				cm->ip->SetStdCommandMode(CID_OBJMOVE);
				cm->ip->RedrawViews(cm->ip->GetTime());
			} 
			else {
				TSTR buf = GetString(IDS_ILLEGALCAM);
				MessageBox(ip->GetMAXHWnd(), buf,
					MESSAGE_TITLE, MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
	return TRUE;
}

/*===========================================================================*\
 | Dll/Lib Functions
\*===========================================================================*/
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) {	
	hInstance = hinstDLL;
	if (! controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	return(TRUE);
}

__declspec(dllexport) const TCHAR *LibDescription() {
	return (LIBDESCRIPTION);
}

__declspec(dllexport) int LibNumberClasses() { 
	return 3;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i) { 
	switch(i) {
		case 0: // Space warp object
			return &camMapObjDesc;
		case 1: // Space warp modifier
			return &camMapWSModDesc;
		case 2: // Modifier
			return &camMapModDesc; 
		default:
			return 0;
	}
}

__declspec(dllexport) ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

