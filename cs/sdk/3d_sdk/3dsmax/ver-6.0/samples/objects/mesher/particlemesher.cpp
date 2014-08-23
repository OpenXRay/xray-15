/**********************************************************************
 *<
	FILE: particlemesher.cpp

	DESCRIPTION:  Particle Mesher Object
				  Takes a particle system and creates a mesh out of it

	CREATED BY: Peter Wathe
				

	HISTORY: created oct 13, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "prim.h"
#include "iparamm2.h"
#include "Simpobj.h"
#include "surf_api.h"
#include "macrorec.h"
#include "IParticleObjectExt.h"

//STUB FOR NOW WE NEED TO REMOVE THIS ONCE WE MAKE THE PF SDK AVAILABLE
#define PARTICLEGROUP_INTERFACE Interface_ID(0x2c712d9f, 0x7bc54cb0) 


#define A_RENDER			A_PLUGIN1

#define PARTICLEMESHER_CLASS_ID 998877
#define PBLOCK_REF_NO	 0

ClassDesc* GetParticleMesherDesc();

// JBW: IParamArray has gone since the class variable UI paramters are stored in static ParamBlocks
//      all corresponding class vars have gone, including the ParamMaps since they are replaced 
//      by the new descriptors

class ParticleMesherObject : public SimpleObject2
{
	public:	

		HWND hParams;
		// Class vars
		static IObjParam *ip;
		float radius;

// JBW: minimal constructor, call MakeAutoParamBlocks() on my ClassDesc to
//      have all the declared per-instance P_AUTO_CONSTRUCT blocks made, initialized and
//      wired in.
		ParticleMesherObject() { 
			GetParticleMesherDesc()->MakeAutoParamBlocks(this); 
			lastTime = -999999;
			}


		int RenderBegin(TimeValue t, ULONG flags);

		int RenderEnd(TimeValue t);

		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_PW_PARTICLEMESHER_OBJECT); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist);

		// From GeomObject
//		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(PARTICLEMESHER_CLASS_ID, 32670); } 
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
// JBW: the Load() post-load callback insertion has gone since versioning is 
//		handled automatically by virtue of permanent parameter IDs.  These IDs
//		are defined in enums and are never retired so that old versions can be
//		automatically re-mapped to new ones
//
//      Note that this is only true in new plug-ins; old plug-ins need to 
//		continue to support version re-mapping as before for version up until
//		converting to the new descriptors
//		IOResult Load(ILoad *iload);
		
// JBW: all the IParamArray methods are gone since we don't need them for the class variables

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
// JBW: the GetParamName() and GetParamDim() function have gone	as this all 
//      is available in the descriptors. REFMSG_GET_PARAM_NAME, etc. become unnecessary as well
		 
		 Tab<Matrix3> tmList;
		 IOResult Load(ILoad *iload);
		 IOResult Save(ISave *isave);
		 TimeValue lastTime;


//		void GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box );
//		void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp, Box3& box );
		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL, BOOL useSel=FALSE );

		void UpdateUI();
		
		Tab<INode*> pfNodes;
		Tab<int>	addPFNodes;
		void PickPFEvents(HWND hWnd);		
};

class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}


// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

//--- ClassDescriptor and class vars ---------------------------------

// The class descriptor for gsphere
class ParticleMesherClassDesc: public ClassDesc2 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ParticleMesherObject; }
	const TCHAR *	ClassName() { return GetString(IDS_PW_PARTICLEMESHER); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(PARTICLEMESHER_CLASS_ID, 32670); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
// JBW:  the ResetClassParams() has gone since this is automatic now
//       using the default values in the descriptors

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("particleMesher"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static ParticleMesherClassDesc particleMesherDesc;
ClassDesc* GetParticleMesherDesc() { return &particleMesherDesc; }

IObjParam* ParticleMesherObject::ip = NULL;

// JBW:  all the old static class variables have gone, most now hosted in 
//       static param blocks

// Misc stuff

// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save

// block IDs
enum { particlemesher_params, };
// param param IDs
enum { particlemesher_pick,particlemesher_rendertimeonly,particlemesher_extranodes, particlemesher_time,particlemesher_radius,
	   particlemesher_usecustombounds,particlemesher_customboundsa,particlemesher_customboundsb,
	   particlemesher_useallpf,particlemesher_pfeventlist
	    };






// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
// JBW: since the old path controller kept all parameters as instance data members, this setter callback
// is implemented to to reduce changes to existing code 



// JBW: here are the two static block descriptors.  This form of 
//      descriptor declaration uses a static NParamBlockDesc instance whose constructor
//      uses a varargs technique to walk through all the param definitions.
//      It has the advantage of supporting optional and variable type definitions, 
//      but may generate a tad more code than a simple struct template.  I'd
//      be interested in opinions about this.

//      I'll briefly describe the first definition so you can figure the others.  Note
//      that in certain places where strings are expected, you supply a string resource ID rather than
//      a string at it does the lookup for you as needed.
//
//		line 1: block ID, internal name, local (subanim) name, flags
//																 AUTO_UI here means the rollout will
//																 be automatically created (see BeginEditParams for details)
//      line 2: since AUTO_UI was set, this line gives: 
//				dialog resource ID, rollout title, flag test, appendRollout flags
//		line 3: required info for a parameter:
//				ID, internal name, type, flags, local (subanim) name
//		lines 4-6: optional parameter declaration info.  each line starts with a tag saying what
//              kind of spec it is, in this case default value, value range, and UI info as would
//              normally be in a ParamUIDesc less range & dimension
//	    the param lines are repeated as needed for the number of parameters defined.



// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 particlemesher_param_blk ( particlemesher_params, _T("ParticleMesherParameters"),  0, &particleMesherDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_PARTICLEMESHER, IDS_PARAMETERS, 0, 0, NULL,
	// params
	particlemesher_pick,  _T("pick"), 	TYPE_INODE, 	0, 	IDS_PW_PICK, 
		p_ui, 			TYPE_PICKNODEBUTTON,  IDC_PICK,
		end, 
	particlemesher_rendertimeonly, 	_T("renderTimeOnly"), 	TYPE_BOOL, 		0,				IDS_PW_RENDER,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RENDER_TIME_ONLY, 
		end, 
	particlemesher_extranodes,  _T(""), 	TYPE_INODE_TAB,0, 	0, 	IDS_PW_EXTRANODES, 
		end, 

	particlemesher_time, 	_T("time"),		TYPE_FLOAT, 		P_RESET_DEFAULT,		IDS_PW_TIME,
		p_default, 		0.0f,	
		p_range, 		-99999999.0f, 9999999999.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_TIME,IDC_TIMESPIN, 1.0f, 
		end, 

	particlemesher_radius, 	_T("radius"), 	TYPE_FLOAT, 		0,				IDS_PW_RADIUS,
		p_default, 		10.0f,
		end,

	particlemesher_usecustombounds, 	_T("useCustomBounds"), 	TYPE_BOOL, 		0,				IDS_PW_CBOUNDS,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BBOX, 
		end, 

	particlemesher_customboundsa, 	_T("boundsMin"), 	TYPE_POINT3, 		0,				IDS_PW_BOUNDSA,
		p_default, 		Point3(1,1,1),	
		end,
	particlemesher_customboundsb, 	_T("boundsMax"), 	TYPE_POINT3, 		0,				IDS_PW_BOUNDSB,
		p_default, 		Point3(-1,-1,-1),	
		end, 
		
	particlemesher_useallpf, 	_T("useAllPFEvents"), 	TYPE_BOOL, 		0,				IDS_USEALLPFEVENTS,
	    p_default,		TRUE,
		p_ui, 			 TYPE_SINGLECHEKBOX, 	IDC_USEALLPF, 
		end, 
	particlemesher_pfeventlist,    _T("pfEventList"),  TYPE_INODE_TAB,		0,	P_AUTO_UI|P_VARIABLE_SIZE,	IDS_PFEVENTLIST,
		p_ui,			 TYPE_NODELISTBOX, IDC_PFLIST,0,0,IDC_PFREMOVE,
		end,
		
	end
	);



//--- CustMod dlg proc ------------------------------

class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		ParticleMesherObject *mod;
		PickControlNode() {mod=NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickControlNode thePickMode;

BOOL PickControlNode::Filter(INode *node)
	{
	return TRUE;
	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		// RB 3/1/99: This should use the node tm not the object TM. See ModifyObject() imp.
		Matrix3 ourTM,ntm = node->GetNodeTM(GetCOREInterface()->GetTime()); //node->GetObjectTM(ip->GetTime());	

		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		if (nodes.Count())
			{
			ourTM = nodes[0]->GetObjectTM(GetCOREInterface()->GetTime());
			ourTM    = Inverse(ourTM);
			Box3 bounds;
			bounds.Init();
			ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
			ViewExp *vp = GetCOREInterface()->GetActiveViewport();
			os.obj->GetWorldBoundBox(GetCOREInterface()->GetTime(), node, vp, bounds );
			GetCOREInterface()->ReleaseViewport(vp);
			Point3 min = bounds.pmin * ourTM;
			Point3 max = bounds.pmax * ourTM;
			theHold.Begin();
			mod->pblock2->SetValue(particlemesher_customboundsa,0,min);
			mod->pblock2->SetValue(particlemesher_customboundsb,0,max);
			theHold.Accept(GetString(IDS_BOUNDS));
			mod->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mod->UpdateUI();


			}

		nodes.DisposeTemporary();
		}
	return TRUE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hParams,IDC_PICKBB));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PickControlNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hParams,IDC_PICKBB));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}


//--- ParticleMesher methods -------------------------------

// JBW: the GeoSphere constructor has gone.  The paramblock creation and wiring and
//		the intial value setting is automatic now.

// JBW: BeginEditParams() becomes much simpler with automatic UI param blocks.
//      you redirect the BeginEditParams() to the ClassDesc instance and it
//      throws up the appropriate rollouts.

class ParticleMesherParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		ParticleMesherObject *mod;		
		ParticleMesherParamsMapDlgProc(ParticleMesherObject *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


BOOL ParticleMesherParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{
			mod->hParams = hWnd;

			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_PICKBB));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);

			if (mod->ip)
				{
				if (mod->ip->GetCommandPanelTaskMode() == TASK_MODE_CREATE)
					{
					iBut->Enable(FALSE);
					}

				}

			ReleaseICustButton(iBut);
			mod->UpdateUI();


			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{
				case IDC_ADDPF_EVENT_BUTTON:
					mod->PickPFEvents(hWnd);
					break;
				case IDC_BBOX:
					{
					mod->UpdateUI();
					break;
					}
				case IDC_UPDATE:
					{
					mod->lastTime = -999999;
					mod->ivalid.SetEmpty();
					mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					break;
					}
				case  IDC_PICKBB:
					{
					if (mod->ip->GetCommandMode() == (CommandMode *) &thePickMode) {
						mod->ip->SetStdCommandMode(CID_OBJMOVE);
						}
					else						
						{
						thePickMode.mod  = mod;					
						mod->ip->SetPickMode(&thePickMode);
						}

					}

				}
			break;
			}



		}
	return FALSE;
	}


void ParticleMesherObject::UpdateUI()
{
	Point3 min,max;
	pblock2->GetValue(particlemesher_customboundsa,0,min,FOREVER);
	pblock2->GetValue(particlemesher_customboundsb,0,max,FOREVER);


	Box3 box;
	box.Init();
	box += min;
	box += max;

	
	TSTR text;
	BOOL useBounds;
	pblock2->GetValue(particlemesher_usecustombounds, 0, useBounds, FOREVER);

	if (useBounds)
		{
		if (box.IsEmpty())
			text.printf("%s",GetString(IDS_EMPTY));
		else 
			{
			text.printf("(%0.0f,%0.0f,%0.0f)",min.x,min.y,min.z);
			SetWindowText(GetDlgItem(hParams,IDC_BOUNDSTEXT),
				text);
			text.printf("(%0.0f,%0.0f,%0.0f)",max.x,max.y,max.z);
			SetWindowText(GetDlgItem(hParams,IDC_BOUNDSTEXT2),
				text);
			}

//make sure we only enable the button in the modify panel
// bug 257747 1/30/01 watje
		if (ip)
			{
			ICustButton *iBut = GetICustButton(GetDlgItem(hParams,IDC_PICKBB));
			if (ip->GetCommandPanelTaskMode()==TASK_MODE_MODIFY)
				{
				if (iBut) iBut->Enable(TRUE);
				}
			ReleaseICustButton(iBut);
			}

		}
	else
		{
		text.printf(" ");
		SetWindowText(GetDlgItem(hParams,IDC_BOUNDSTEXT),
			text);
		text.printf(" ");
		SetWindowText(GetDlgItem(hParams,IDC_BOUNDSTEXT2),
			text);
		ICustButton *iBut = GetICustButton(GetDlgItem(hParams,IDC_PICKBB));
		if (iBut) iBut->Enable(FALSE);
		ReleaseICustButton(iBut);

		}
	
}

void ParticleMesherObject::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	SimpleObject::BeginEditParams(ip, flags, prev);
	this->ip = ip;
	// throw up all the appropriate auto-rollouts
	particleMesherDesc.BeginEditParams(ip, this, flags, prev);
	particlemesher_param_blk.SetUserDlgProc(new ParticleMesherParamsMapDlgProc(this));
	// install a callback for the type in.


}
		
// JBW: similarly for EndEditParams and you also don't have to snapshot
//		current parameter values as initial values for next object as
//		this is automatic for the new ParamBlock params unless disabled.

void ParticleMesherObject::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{		
	SimpleObject::EndEditParams(ip, flags, next);
	this->ip = NULL;
	// tear down the appropriate auto-rollouts
	particleMesherDesc.EndEditParams(ip, this, flags, next);
}

int ParticleMesherObject::RenderBegin(TimeValue t, ULONG flags)
	{
	SetAFlag(A_RENDER);
	ivalid.SetEmpty();
	lastTime =t -99999;
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return 0;
	}

int ParticleMesherObject::RenderEnd(TimeValue t)
	{
	ClearAFlag(A_RENDER);
	lastTime = t - 99999;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return 0;

	}


// CONSTRUCTING THE MESH:

// To construct a geodesic sphere, we take a tetrahedron, subdivide each face into
// segs^2 faces, and project the vertices onto the sphere of the correct radius.

// This subdivision produces 3 kinds of vertices: 4 "corner" vertices, which are the
// original tetrahedral vertices; "edge" vertices, those that lie on the tetrahedron's
// edges, and "face" vertices.  There are 6 edges with (segs-1) verts on each, and
// 4 faces with (segs-1)*(segs-2)/2 verts.

// We construct these vertices in this order: the first four are the corner vertices.
// Then we use spherical interpolation to place edge vertices along each edge.
// Finally, we use the same interpolation to produce face vertices between the edge
// vertices.


// Assumed in the following function: the vertices have the same radius, or
// distance from the origin, and they have nontrivial cross product.



BOOL ParticleMesherObject::HasUVW() { 
	return FALSE; 
	}

void ParticleMesherObject::SetGenUVW(BOOL sw) {  
	}

class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};


// Now put it all together sensibly
#define EPSILON 1e-5f


void ParticleMesherObject::BuildMesh(TimeValue t)
	{
//check if render time
//get node
//mkae ivalid interesect with the 
	int isRendering = 0;
	ivalid = FOREVER;

	TimeValue offset;
	float foffset;
	Interval iv;
	pblock2->GetValue(particlemesher_time, 0, foffset, iv);
	foffset = -foffset;
	pblock2->GetValue(particlemesher_radius, 0, radius, iv);
	foffset *= GetTicksPerFrame();
	offset = (TimeValue) foffset;

//	pblock2->GetValue(particlemesher_time, 0, foffset, iv);
	if ((lastTime == t)  && (offset != 0))
		{
		ivalid.Set(t,t);
		return;
		}


	pblock2->GetValue(particlemesher_rendertimeonly, 0, isRendering, ivalid);
	isRendering = !isRendering;
    if ((isRendering) || (TestAFlag(A_RENDER)))
		{
		INode *node=NULL;
		pblock2->GetValue(particlemesher_pick, 0, node, ivalid);

		
		BOOL reevalGroup = FALSE;
		if ((node != NULL) && (node->IsGroupHead()) )
			{	
			for (int ch=0;ch<node->NumberOfChildren();ch++)
				{	
				INode *cnode= node->GetChildNode(ch);
				Interval iv;
				Matrix3 tm=cnode->GetObjectTM(t,&iv);
		

				if (cnode->IsGroupMember())
					{
					reevalGroup = TRUE;
					for (int groupCount = 0; groupCount < pblock2->Count(particlemesher_extranodes); groupCount++)
						{
						INode *extraNode = pblock2->GetINode(particlemesher_extranodes,t,ch);
						if (cnode == extraNode)
							{
							reevalGroup = FALSE;
							groupCount = pblock2->Count(particlemesher_extranodes);
							}
						}
					if (reevalGroup)
						ch=node->NumberOfChildren();

					}
				}

			if (reevalGroup)
				{
				tmList.ZeroCount();
	
				pblock2->ZeroCount(particlemesher_extranodes);
				for (int ch=0;ch<node->NumberOfChildren();ch++)
					{	
					INode *cnode= node->GetChildNode(ch);
					Interval iv;
					Matrix3 tm=cnode->GetObjectTM(t,&iv);
		

					if (cnode->IsGroupMember())
						{
						pblock2->Append(particlemesher_extranodes,1,&cnode);
						tmList.Append(1,&tm);
						}
					}

				}
			}

		if (node) 
			{

			if ( (node->IsGroupHead()) && (pblock2->Count(particlemesher_extranodes)!=0))
				{

				int ct = 0;
				Matrix3 ident(1), inverseTm(1);
				mesh.setNumVerts(0);
				mesh.setNumFaces(0);
				for (int ch=0;ch<pblock2->Count(particlemesher_extranodes);ch++)
					{	
					INode *cnode = pblock2->GetINode(particlemesher_extranodes,t,ch);
					if (cnode)
						{
						Object *pobj = cnode->EvalWorldState(t+offset).obj;
					
						if ( (pobj->SuperClassID() == GEOMOBJECT_CLASS_ID) ||
							 (pobj->SuperClassID() == SHAPE_CLASS_ID) )							
							{	


							BOOL needDel;
							NullView nullView;
							Mesh *msh = ((GeomObject*)pobj)->GetRenderMesh(t+offset,cnode,nullView,needDel);

							Mesh tmsh = *msh;

							ivalid &= pobj->ObjectValidity(t+offset);

							Matrix3 tm(1);
							if (ch < tmList.Count())
								tm = tmList[ch];
							for (int v = 0; v < msh->numVerts; v++)
								{
								tmsh.verts[v] = tmsh.verts[v] * tm;
								}
							if (tmsh.numVerts != 0)
								{
								if (ct ==0)
									{
									mesh = tmsh;
									}
								else 
									mesh = mesh + tmsh;
						
								ct++;
								}

							if (needDel) delete msh;
							}

							
					
						}
						
					}
				mesh.InvalidateTopologyCache();

				}
			else
				{
//				Object *tobj =  node->GetObjectRef();
//				macroRecorder->FunctionCall(_T("time"), 1, 0, mr_int, t);
//				macroRecorder->EmitScript();
				ObjectState os = node->EvalWorldState(t+offset);

				IParticleObjectExt* epobj;
				epobj = (IParticleObjectExt*) os.obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE);
				
				if (os.obj->IsParticleSystem() && epobj)
					{
		
					if (epobj) 
						{
						BOOL useAllPFEvents;
						pblock2->GetValue(particlemesher_useallpf,0,useAllPFEvents,FOREVER);
						
						pfNodes.ZeroCount();
						
						INode *basenode=NULL;
						pblock2->GetValue(particlemesher_pick, 0, basenode, ivalid);						

						tmList.ZeroCount();

						if (useAllPFEvents)
							{
							MyEnumProc dep;              
							os.obj->EnumDependents(&dep);
							
							for (int i = 0; i < dep.Nodes.Count(); i++)
								{
								Interval valid;
								INode *node = dep.Nodes[i];

								Object *obj = node->GetObjectRef();

								
								if ((obj) && (obj->GetInterface(PARTICLEGROUP_INTERFACE)) )
									{
									pfNodes.Append(1,&node);
									Matrix3 tm = node->GetNodeTM(t+offset);
									tmList.Append(1,&tm);
									}
								}						
							}
						else
							{
							int ct = pblock2->Count(particlemesher_pfeventlist);
							for (int i = 0; i < ct; i++)
								{
								INode *node;
								pblock2->GetValue(particlemesher_pfeventlist,t,node,FOREVER,i);
								if (node)
									{

									Object *obj = node->GetObjectRef();
									if ((obj) && (obj->GetInterface(PARTICLEGROUP_INTERFACE)) )
										{
										pfNodes.Append(1,&node);
										Matrix3 tm(1);// = basenode->GetNodeTM(t+offset);
										Matrix3 ntm = node->GetObjectTM(t+offset);
										tm = ntm;
										tmList.Append(1,&ntm);
										
										}									
									}
								}
							}
							
						mesh.setNumVerts(0);
						mesh.setNumFaces(0);
						int ct = 0;
						for (int ch=0;ch< pfNodes.Count();ch++)
							{	
							INode *cnode = pfNodes[ch];
							if (cnode)
								{
								Object *pobj = cnode->EvalWorldState(t+offset).obj;
							
								if ( (pobj->SuperClassID() == GEOMOBJECT_CLASS_ID) ||
									(pobj->SuperClassID() == SHAPE_CLASS_ID) )							
									{	


									BOOL needDel;
									NullView nullView;
									Mesh *msh = ((GeomObject*)pobj)->GetRenderMesh(t+offset,cnode,nullView,needDel);

									Mesh tmsh = *msh;

									ivalid &= pobj->ObjectValidity(t+offset);

									Matrix3 tm(1);
									if (ch < tmList.Count())
										tm = tmList[ch];
									for (int v = 0; v < msh->numVerts; v++)
										{
										tmsh.verts[v] = tmsh.verts[v] * tm;
										}
									if (tmsh.numVerts != 0)
										{
										if (ct ==0)
											{
											mesh = tmsh;
											}
										else 
											{
											Mesh tempMesh = mesh;
											CombineMeshes(mesh, tempMesh, tmsh);
											}
								
										ct++;
										}

									if (needDel) delete msh;
									}

									
							
								}
								
							}
						mesh.InvalidateTopologyCache();
							
							
													
						}
					}
				else if ( (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) ||
				 (os.obj->SuperClassID() == SHAPE_CLASS_ID) )
					{

					BOOL needDel;
					NullView nullView;
					Mesh *msh = ((GeomObject*)os.obj)->GetRenderMesh(t+offset,node,nullView,needDel);
					ivalid &= os.obj->ObjectValidity(t);

					if (msh)
						{
						mesh = *msh;
						mesh.InvalidateTopologyCache();
			
						if (needDel) delete msh;
						}
					}
				}
			lastTime = t;
			}
		else
			{
//build proxy mesh
			if (node == NULL)
				{
				mesh.setNumVerts(5);
				mesh.setNumFaces(8);

				mesh.setNumMaps(2);
				mesh.setNumMapVerts(0, 0);
				mesh.setNumMapVerts(1, 0);
				mesh.setNumMapFaces(0, 0);
				mesh.setNumMapFaces(1, 0);

				mesh.setVert(0, Point3(-radius,-radius, 0.0f));
				mesh.setVert(1, Point3( radius,-radius, 0.0f));
				mesh.setVert(2, Point3( radius, radius, 0.0f));
				mesh.setVert(3, Point3(-radius, radius, 0.0f));

//				mesh.setVert(4, Point3(0.0f, 0.0f, 0.0f));
				mesh.setVert(4, Point3(0.0f, 0.0f, radius));
	
				mesh.faces[0].setEdgeVisFlags(1,0,1);
				mesh.faces[0].setSmGroup(1);
				mesh.faces[0].setVerts(0,1,3);

				mesh.faces[1].setEdgeVisFlags(1,1,0);
				mesh.faces[1].setSmGroup(1);
				mesh.faces[1].setVerts(1,2,3);	

				mesh.faces[2].setEdgeVisFlags(1,1,1);
				mesh.faces[2].setSmGroup(1);
				mesh.faces[2].setVerts(0,4,1);	

				mesh.faces[3].setEdgeVisFlags(1,1,1);
				mesh.faces[3].setSmGroup(1);
				mesh.faces[3].setVerts(1,4,0);	

				mesh.faces[4].setEdgeVisFlags(1,1,1);
				mesh.faces[4].setSmGroup(1);
				mesh.faces[4].setVerts(2,4,3);	

				mesh.faces[5].setEdgeVisFlags(1,1,1);
				mesh.faces[5].setSmGroup(1);
				mesh.faces[5].setVerts(3,4,2);	

				mesh.faces[6].setEdgeVisFlags(1,0,1);
				mesh.faces[6].setSmGroup(1);
				mesh.faces[6].setVerts(3,1,0);

				mesh.faces[7].setEdgeVisFlags(1,1,0);
				mesh.faces[7].setSmGroup(1);
				mesh.faces[7].setVerts(3,2,1);	


				}

			}
		}	
	else
		{
//build proxy mesh
//build proxy mesh
		INode *node=NULL;
		pblock2->GetValue(particlemesher_pick, 0, node, ivalid);
	//	if (node == NULL)
			{
				mesh.setNumVerts(5);
				mesh.setNumFaces(8);

				mesh.setNumMaps(2);
				mesh.setNumMapVerts(0, 0);
				mesh.setNumMapVerts(1, 0);
				mesh.setNumMapFaces(0, 0);
				mesh.setNumMapFaces(1, 0);

				mesh.setVert(0, Point3(-radius,-radius, 0.0f));
				mesh.setVert(1, Point3( radius,-radius, 0.0f));
				mesh.setVert(2, Point3( radius, radius, 0.0f));
				mesh.setVert(3, Point3(-radius, radius, 0.0f));

//				mesh.setVert(4, Point3(0.0f, 0.0f, 0.0f));
				mesh.setVert(4, Point3(0.0f, 0.0f, radius));
	
				mesh.faces[0].setEdgeVisFlags(1,0,1);
				mesh.faces[0].setSmGroup(1);
				mesh.faces[0].setVerts(0,1,3);

				mesh.faces[1].setEdgeVisFlags(1,1,0);
				mesh.faces[1].setSmGroup(1);
				mesh.faces[1].setVerts(1,2,3);	

				mesh.faces[2].setEdgeVisFlags(1,1,1);
				mesh.faces[2].setSmGroup(1);
				mesh.faces[2].setVerts(0,4,1);	

				mesh.faces[3].setEdgeVisFlags(1,1,1);
				mesh.faces[3].setSmGroup(1);
				mesh.faces[3].setVerts(1,4,0);	

				mesh.faces[4].setEdgeVisFlags(1,1,1);
				mesh.faces[4].setSmGroup(1);
				mesh.faces[4].setVerts(2,4,3);	

				mesh.faces[5].setEdgeVisFlags(1,1,1);
				mesh.faces[5].setSmGroup(1);
				mesh.faces[5].setVerts(3,4,2);	

				mesh.faces[6].setEdgeVisFlags(1,0,1);
				mesh.faces[6].setSmGroup(1);
				mesh.faces[6].setVerts(3,1,0);

				mesh.faces[7].setEdgeVisFlags(1,1,0);
				mesh.faces[7].setSmGroup(1);
				mesh.faces[7].setVerts(3,2,1);	

			}
		}

	mesh.InvalidateTopologyCache();
}

class ParticleMesherObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	ParticleMesherObject *ob;
	Point3 p0;
public:
	int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(ParticleMesherObject *obj) {ob = obj;}
};

int ParticleMesherObjCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1, center;

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
	}


	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0:  // only happens with MOUSE_POINT msg
			mat.IdentityMatrix();
			ob->suspendSnap = TRUE;				
			sp0 = m;
			p0 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			ob->radius = 3.0f;
			ob->pblock2->SetValue(particlemesher_radius, 0, ob->radius);

			break;
		case 1:
			p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
//			center = (p0+p1)/float(2);
//			mat.SetTrans(center);
			r = Length(p0-p1);
//			mat.SetTrans(center);
			 
//			ob->pblock2->SetValue(geo_radius, 0, r);
			ob->pblock2->SetValue(particlemesher_rendertimeonly, 0, 0);
			ob->pblock2->SetValue(particlemesher_radius, 0, r);

			ob->radius = r;
//DebugPrint("radius %f\n",r);
			particlemesher_param_blk.InvalidateUI();

			if (flags&MOUSE_CTRL) {
				float ang = (float)atan2(p1.y-p0.y, p1.x-p0.x);					
				mat.PreRotateZ(ob->ip->SnapAngle(ang));
			}

			if (msg==MOUSE_POINT) {
				ob->suspendSnap = FALSE;
				return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
			}
			break;					   
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static ParticleMesherObjCreateCallBack particleMesherCreateCB;

CreateMouseCallBack* ParticleMesherObject::GetCreateMouseCallBack() 
	{
	particleMesherCreateCB.SetObj(this);
	return(&particleMesherCreateCB);
	}


BOOL ParticleMesherObject::OKtoDisplay(TimeValue t) 
	{
	return TRUE;
	}


void ParticleMesherObject::InvalidateUI() 
{
	particlemesher_param_blk.InvalidateUI();
}

RefTargetHandle ParticleMesherObject::Clone(RemapDir& remap) 
{
	ParticleMesherObject* newob = new ParticleMesherObject();	
	newob->ReplaceReference(0, pblock2->Clone(remap));
	newob->ivalid.SetEmpty();	
	newob->tmList = tmList; 
	return(newob);
}


Object* ParticleMesherObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
		return SimpleObject::ConvertToType(t, obtype);
	}

int ParticleMesherObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID ) {
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}


void ParticleMesherObject::GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
//    clist.Append(1, &id);
 //   nlist.Append(1, &name);
}

#define TMCOUNT_CHUNK	0x0100
#define TMDATA_CHUNK	0x0110

IOResult ParticleMesherObject::Save(ISave *isave)
	{
	SimpleObject2::Save(isave);
	int ct = tmList.Count();
	ULONG nb;
	isave->BeginChunk(TMCOUNT_CHUNK);	
	isave->Write(&ct,sizeof(ct),&nb);
	isave->EndChunk();

	for (int i=0; i<ct; i++)
		{
		Matrix3 tm = tmList[i];
		isave->BeginChunk(TMDATA_CHUNK);	
		tm.Save(isave);
		isave->EndChunk();	
		}
	
	
	return IO_OK;
	}

IOResult ParticleMesherObject::Load(ILoad *iload)
	{
	IOResult res = IO_OK;
	ULONG nb;
	SimpleObject2::Load(iload);
	// Default names
	
	int ct = 0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case TMCOUNT_CHUNK: 
				{
				int ct;
				iload->Read(&ct,sizeof(ct),&nb);
				tmList.SetCount(ct);	
				break;
				}
			case TMDATA_CHUNK: 
				{
				Matrix3 tm;
				tm.Load(iload);
				tmList[ct++] = tm;
				
				break;
				}
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}


/*
void ParticleMesherObject::GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box )	
	{
	box.Init();
	Matrix3 mat = inode->GetObjectTM(t);
	Point3 a(-10.0f,-10.0f,-10.0f);
	Point3 b(10.0f,10.0f,10.0f);
	a = a * mat;
	b = b * mat;
	box+= a;
	box+= b;

	}
void ParticleMesherObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp, Box3& box )	
	{
	box.Init();
	Point3 a(-10.0f,-10.0f,-10.0f);
	Point3 b(10.0f,10.0f,10.0f);
	box+= a;
	box+= b;

	}
	*/
void ParticleMesherObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )	
	{
	BOOL useBounds;
	pblock2->GetValue(particlemesher_usecustombounds, 0, useBounds, FOREVER);
	if (useBounds)
		{
		box.Init();

		Point3 a(-10.0f,-10.0f,-10.0f);
		Point3 b(10.0f,10.0f,10.0f);
		pblock2->GetValue(particlemesher_customboundsa, 0, a, FOREVER);
		pblock2->GetValue(particlemesher_customboundsb, 0, b, FOREVER);
		if (tm)
			{
			a = a * *tm;
			b = b * *tm;
			}
		box+= a;
		box+= b;
		if (box.IsEmpty())
			SimpleObject2::GetDeformBBox(t,box,tm,useSel);
		}
	else
		{
		SimpleObject2::GetDeformBBox(t,box,tm,useSel);
		}

	}


RefResult ParticleMesherObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{

	RefResult res = SimpleObject2::NotifyRefChanged(changeInt, hTarget, partID,  message) ;
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget == pblock2)
				{
				ParamID changing_param = pblock2->LastNotifyParamID();
				if ((ip) && ((changing_param == particlemesher_customboundsb) || (changing_param == particlemesher_customboundsa)))
					UpdateUI();
				}
			break;
		}
	return res;
	}



static INT_PTR CALLBACK AddDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	ParticleMesherObject *mod = (ParticleMesherObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (ParticleMesherObject*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			for (int i=0; i < mod->pfNodes.Count(); i++)
				{
				TCHAR *name;


				name = mod->pfNodes[i]->GetName();

				if (name)
					{
					TCHAR title[200];

					_tcscpy(title,name);

					SendMessage(GetDlgItem(hWnd,IDC_EVENTLIST),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);



					}

				}

			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{

					int listCt = SendMessage(GetDlgItem(hWnd,IDC_EVENTLIST),
						LB_GETCOUNT,0,0);
					int selCt =  SendMessage(GetDlgItem(hWnd,IDC_EVENTLIST),
						LB_GETSELCOUNT ,0,0);
					int *selList;
					selList = new int[selCt];

					SendMessage(GetDlgItem(hWnd,IDC_EVENTLIST),
						LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);
					mod->addPFNodes.SetCount(selCt);
					for (int i=0; i < selCt; i++)
						{
						mod->addPFNodes[i] = selList[i];
						}

					delete [] selList;

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					mod->pfNodes.ZeroCount();
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			mod->pfNodes.ZeroCount();
			EndDialog(hWnd, 0);
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}

void
ParticleMesherObject::PickPFEvents(HWND hWnd)
{
	Tab<INode *> pfEvents;
	int numberOfNodes = 1;;

	TimeValue t = GetCOREInterface()->GetTime();


	pfNodes.ZeroCount();
	addPFNodes.ZeroCount();
	

	for (int i = 0; i < numberOfNodes; i++)
		{
		INode *node;
		pblock2->GetValue(particlemesher_pick,t,node,ivalid,i);

		if (node)
			{
			ObjectState tos =  node->EvalWorldState(t,TRUE);

			if (tos.obj->IsParticleSystem())
				{
				IParticleObjectExt* epobj;
				epobj = (IParticleObjectExt*) tos.obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE);
	
				if (epobj) 
					{
					MyEnumProc dep;              
					tos.obj->EnumDependents(&dep);
					for (int i = 0; i < dep.Nodes.Count(); i++)
						{
						Interval valid;
						INode *node = dep.Nodes[i];

						Object *obj = node->GetObjectRef();

						
						if ((obj) && (obj->GetInterface(PARTICLEGROUP_INTERFACE)) )
							{
							pfNodes.Append(1,&node);
							}
						}

					}

				}

			}
		}
	if (pfNodes.Count() > 0)
		{
		int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_ADD_DIALOG),
				hWnd,AddDlgProc,(LPARAM)this);
		if ((iret) && (addPFNodes.Count() > 0))
			{
			theHold.Begin();
			for (int i = 0; i < addPFNodes.Count(); i++)
				{
				int index = addPFNodes[i];
				INode *node = pfNodes[index];
				pblock2->Append(particlemesher_pfeventlist,1,&node);
				}
			theHold.Accept(GetString(IDS_ADDEVENTS));

			}
		}	

}
