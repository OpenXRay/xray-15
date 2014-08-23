/**********************************************************************
 *<
	FILE: SolidifyPW.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

  hook up shape and option

  add bevel edge direction

	get the shape polyline
	count = segments
	normalize or shape


 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "SolidifyPW.h"

#define SOLIDIFYPW_CLASS_ID	Class_ID(0x3b9b1a16, 0x6d84e8d0)


#define PBLOCK_REF	0


static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Polygon(4);




class SolidifyPW : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR|PART_SUBSEL_TYPE;}
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return triObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		

		//From Animatable
		Class_ID ClassID() {return SOLIDIFYPW_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }




		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		SolidifyPW();
		~SolidifyPW();		

		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);
		void ActivateSubobjSel(int level, XFormModes& modes);

		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);


		BuildMeshInfo meshInfo;




	
		HWND hWnd;
		void EnableUIControls();

};


class SolidifyPWClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SolidifyPW(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return SOLIDIFYPW_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("Shell"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static SolidifyPWClassDesc SolidifyPWDesc;
ClassDesc2* GetSolidifyPWDesc() { return &SolidifyPWDesc; }


enum { solidifypw_params };


//TODO: Add enums for various parameters
enum { 
	pb_amount,
	pb_oamount,
	pb_overridematid,
	pb_matid,
	pb_overridesg,
	pb_sg,
	pb_edgemap,
	pb_tvoffset,

	pb_overrideinnermatid,
	pb_innermatid,

	pb_seledges,
	pb_selinner,
	pb_segments,

	pb_fixupcorners,
	pb_autosmooth, pb_autosmoothangle,

	pb_selouter,

	pb_overrideoutermatid,
	pb_outermatid,

	pb_bevel, pb_bevelshape

};




class SolidifyPWPBAccessor : public PBAccessor
{ 
	public:


	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		SolidifyPW* p = (SolidifyPW*)owner;
		switch (id)
		{
			case pb_seledges:
			case pb_selinner:
			case pb_selouter:
			{
				if (v.i)
				{	
					if (p->ip)
					{
						p->ip->PipeSelLevelChanged();
						p->NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);						
						p->NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
						p->ip->SetSubObjectLevel(0,TRUE);
//						p->ip->SetSubObjectLevel(1,TRUE);
//						p->ip->SetSubObjectLevel(0,TRUE);


						
					}

				}
				break;
			}



		}
	}
};


static SolidifyPWPBAccessor solidifypw_accessor;


static ParamBlockDesc2 solidifypw_param_blk ( solidifypw_params, _T("params"),  0, &SolidifyPWDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_amount, 			_T("innerAmount"), 		TYPE_WORLD, 	P_ANIMATABLE, 	IDS_IAMOUNT, 
		p_default, 		0.0f, 
		p_range, 		0.0f,999000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT,	IDC_SPIN, 0.01f, 
		end,

	pb_oamount, 			_T("outerAmount"), 		TYPE_WORLD, 	P_ANIMATABLE, 	IDS_OAMOUNT, 
		p_default, 		1.0f, 
		p_range, 		0.0f,999000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT2,	IDC_SPIN2, 0.01f, 
		end,

	pb_overridematid, 	_T("overrideMatID"), 	TYPE_BOOL, 		0,				IDS_OVERRIDEMATID,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_MATIDCHECK, 
		end, 

	pb_matid, 			_T("matID"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_MATID, 
		p_default, 		1, 
		p_range, 		1,255, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_MATID,	IDC_MATIDSPIN, 0.1f, 
		end,


	pb_overridesg, 	_T("overrideSmoothingGroup"), 	TYPE_BOOL, 		0,				IDS_OVERRIDESG,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SMOOTHCHECK, 
		end, 

	pb_sg, 			_T("smoothGroup"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_SG, 
		p_default, 		0, 
		p_range, 		0,32, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SMOOTHGROUP,	IDC_SMOOTHGROUPSPIN, 0.01f, 
		end,

	pb_edgemap, 			_T("edgeMapping"), 		TYPE_INT, 	0, 	IDS_EDGEMAPPING, 
		p_default, 		0, 
		p_ui, TYPE_INTLISTBOX, IDC_EDGEMAPCOMBO, 4, IDS_COPY,
													IDS_NONE,
													IDS_STRIP,
													IDS_INTERPOLATE,
		end,


	pb_tvoffset, 			_T("tvOffset"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_TVOFFSET, 
		p_default, 		0.05f, 
		p_range, 		0.001f,1.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_TVOFFSET,	IDC_TVOFFSETSPIN, 0.01f, 
		end,

	pb_overrideinnermatid, 	_T("overrideInnerMatID"), 	TYPE_BOOL, 		0,				IDS_OVERRIDEINNERMATID,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_MATIDCHECK2, 
		end, 

	pb_innermatid, 			_T("matInnerID"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_INNERMATID, 
		p_default, 		1, 
		p_range, 		1,255, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_MATID2,	IDC_MATIDSPIN2, 0.1f, 
		end,

	pb_seledges, 	_T("selectEdgeFaces"), 	TYPE_BOOL, 		0,				IDS_SELECTEDGES,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SELECTEDGESCHECK, 
		p_accessor,		&solidifypw_accessor,
		end, 

	pb_selinner, 	_T("selectInnerFaces"), 	TYPE_BOOL, 		0,				IDS_SELECTINNER,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SELECTFACESCHECK, 
		p_accessor,		&solidifypw_accessor,
		end, 

	

	pb_segments, 			_T("segments"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_SEGMENTS, 
		p_default, 		1, 
		p_range, 		1,255, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_EDIT3,	IDC_SPIN3, 0.1f, 
		end,

	pb_fixupcorners, 	_T("straightenCorners"), 	TYPE_BOOL, 		0,				IDS_FIXUPCORNERS,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FIXCORNERSCHECK, 
		end, 


	pb_autosmooth, 	_T("autoSmooth"), 	TYPE_BOOL, 		0,				IDS_AUTOSMOOTH,
	    p_default,		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_AUTOSMOOTHCHECK, 
		end, 

	pb_autosmoothangle, 			_T("autoSmoothAngle"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SG, 
		p_default, 		45.0f, 
		p_range, 		0.0f,180.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_AUTOSMOOTH,	IDC_AUTOSMOOTHSPIN, 1.0f, 
		end,


	pb_selouter, 	_T("selectOuterFaces"), 	TYPE_BOOL, 		0,				IDS_SELECTOUTER,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SELECTOUTERFACESCHECK, 
		p_accessor,		&solidifypw_accessor,
		end, 

	pb_overrideoutermatid, 	_T("overrideOuterMatID"), 	TYPE_BOOL, 		0,				IDS_OVERRIDEOUTERMATID,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_MATIDCHECK3, 
		end, 

	pb_outermatid, 			_T("matOuterID"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_OUTERMATID, 
		p_default, 		3, 
		p_range, 		1,255, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_MATID3,	IDC_MATIDSPIN3, 0.1f, 
		end,

	pb_bevel, 	_T("bevel"), 	TYPE_BOOL, 		0,				IDS_BEVEL,
	    p_default,		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BEVELEDGESCHECK, 
		end, 

	pb_bevelshape, 	_T("bevelShape"),		TYPE_INODE, 		0,				IDS_BEVELSHAPE,
		p_ui, 			TYPE_PICKNODEBUTTON, 	IDC_SHAPE, 
		p_sclassID,  SHAPE_CLASS_ID,
		end, 





	end
	);




IObjParam *SolidifyPW::ip			= NULL;








//--- SolidifyPW -------------------------------------------------------
SolidifyPW::SolidifyPW()
{
	hWnd = NULL;
	SolidifyPWDesc.MakeAutoParamBlocks(this);
}

SolidifyPW::~SolidifyPW()
{
}


void SolidifyPW::ActivateSubobjSel(int level, XFormModes& modes)
{
	if (ip)
	{
		ip->PipeSelLevelChanged();
		NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);						
		NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
	}

}

int SolidifyPW::NumSubObjTypes() 
{ 
	return 0;
}

ISubObjType *SolidifyPW::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Polygon.SetName(GetString(IDS_EM_POLY));

	}

			int edge,inner,outer;
			pblock->GetValue (pb_seledges,0,edge,FOREVER);
			pblock->GetValue (pb_selinner,0,inner,FOREVER);
			pblock->GetValue (pb_selouter,0,outer,FOREVER);

			if (edge || inner || outer)
				return &SOT_Polygon;
			else return NULL;
			

	return NULL;
}


/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval SolidifyPW::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/

void SolidifyPW::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/


void SolidifyPW::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}





/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void SolidifyPW::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{

	//TODO: Add the code for actually modifying the object
	meshInfo.Free();

	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		Mesh &mesh = tobj->GetMesh();
		Interval iv = FOREVER;
		float a,oa;
		
		pblock->GetValue(pb_amount,t,a,iv);
		pblock->GetValue(pb_oamount,t,oa,iv);

		BOOL overrideMatID;
		int matid;

		pblock->GetValue(pb_overridematid,t,overrideMatID,iv);
		pblock->GetValue(pb_matid,t,matid,iv);
		matid--;

		if (!overrideMatID) matid = -1;


		BOOL overridesg;
		int sg;
		pblock->GetValue(pb_overridesg,t,overridesg,iv);
		pblock->GetValue(pb_sg,t,sg,iv);
		

		if (!overridesg) sg = -1;

		int edgeMap;
		pblock->GetValue(pb_edgemap,t,edgeMap,iv);

		float tvOffset;
		pblock->GetValue(pb_tvoffset,t,tvOffset,iv);


		BOOL ioverrideMatID;
		int imatid;

		pblock->GetValue(pb_overrideinnermatid,t,ioverrideMatID,iv);
		pblock->GetValue(pb_innermatid,t,imatid,iv);
		imatid--;

		if (!ioverrideMatID) imatid = -1;


		BOOL ooverrideMatID;
		int omatid;

		pblock->GetValue(pb_overrideoutermatid,t,ooverrideMatID,iv);
		pblock->GetValue(pb_outermatid,t,omatid,iv);
		omatid--;

		if (!ooverrideMatID) omatid = -1;


		BOOL selEdges, selInner,selOuter;

		static BOOL selEdgesPrev = FALSE;
		static BOOL selInnerPrev = FALSE;
		static BOOL selOuterPrev = FALSE;
		
		BOOL updateUI = FALSE;
		
		pblock->GetValue(pb_seledges,t,selEdges,iv);
		pblock->GetValue(pb_selinner,t,selInner,iv);
		pblock->GetValue(pb_selouter,t,selOuter,iv);
		
		if (selEdges && (!selEdgesPrev))
			updateUI = TRUE;
		if (selInner && (!selInnerPrev))
			updateUI = TRUE;
		if (selOuter && (!selOuterPrev))
			updateUI = TRUE;
			
		selEdgesPrev = selEdges;
		selInnerPrev = selInner;			
		selOuterPrev = selOuter;			

		if (selEdges || selInner|| selOuter)
			{
			mesh.dispFlags = DISP_SELFACES;
			mesh.selLevel = MESH_FACE;
			}



		int segments = 1;

		if (segments < 1) segments = 1;
		pblock->GetValue(pb_segments,t,segments,iv);


		BOOL fixupCorners;
		pblock->GetValue(pb_fixupcorners,t,fixupCorners,iv);


		BOOL autoSmooth;
		float smoothAngle;
		pblock->GetValue(pb_autosmooth,t,autoSmooth,iv);
		pblock->GetValue(pb_autosmoothangle,t,smoothAngle,iv);

		BOOL bevel;
		INode *node;
		pblock->GetValue(pb_bevel,t,bevel,iv);
		pblock->GetValue(pb_bevelshape,t,node,iv);

		PolyShape shape;
		if ((bevel) && node)
		{
			ObjectState nos = node->EvalWorldState(t);
			if (nos.obj->IsShapeObject()) 
			{
				ShapeObject *pathOb = (ShapeObject*)nos.obj;

				if (!pathOb->NumberOfCurves()) 
				{
					bevel = FALSE;
				}
				else
				{
					pathOb->MakePolyShape(t, shape,PSHAPE_BUILTIN_STEPS,TRUE);
					if (shape.lines[0].IsClosed())
						bevel = FALSE;

;
				}
			
			}
		}


		DWORD selLevel = mesh.selLevel;
		
		mesh.faceSel.ClearAll();

		if (bevel)
			meshInfo.MakeSolid(&mesh,segments, a,oa,matid, sg,edgeMap,tvOffset,imatid,omatid,selEdges,selInner,selOuter,fixupCorners,autoSmooth,smoothAngle,&shape.lines[0]);
		else meshInfo.MakeSolid(&mesh,segments, a,oa,matid, sg,edgeMap,tvOffset,imatid,omatid,selEdges,selInner,selOuter,fixupCorners,autoSmooth,smoothAngle,NULL);

		
		mesh.selLevel = selLevel;

		mesh.InvalidateTopologyCache ();


		for (int i = 0; i < mesh.numFaces; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int index = mesh.faces[j].v[j];
				if ((index < 0) || (index >= mesh.numVerts))
					DebugPrint("Invalid face %d(%d) %d\n",i,j,index);
			}
		}
		
		int numMaps = mesh.getNumMaps();


		for (int mp = 0; mp < numMaps; mp++)
		{

			if (!mesh.mapSupport(mp)) continue;
			Point3 *uvw = mesh.mapVerts(mp);
			TVFace *uvwFace = mesh.mapFaces(mp);

			if ((uvw) && (uvwFace))
			{
				int numberTVVerts = mesh.getNumMapVerts(mp);
				for (int i = 0; i < mesh.numFaces; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						int index = uvwFace[i].t[j];
						if ((index < 0) || (index >= numberTVVerts))
							DebugPrint("Invalid Map %d tvface %d(%d) %d\n",mp,i,j,index);
					}
				}

			}
		}



		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);
		os->obj->UpdateValidity(TOPO_CHAN_NUM,iv);
		
		if ((updateUI) && (ip))
		{
			ip->PipeSelLevelChanged();
		}
		}
	EnableUIControls();

}


void SolidifyPW::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	SolidifyPWDesc.BeginEditParams(ip, this, flags, prev);

	solidifypw_param_blk.SetUserDlgProc(solidifypw_params,new SolidifyPWDlgProc(this));

}

void SolidifyPW::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	hWnd = NULL;
	SolidifyPWDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;
}



Interval SolidifyPW::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle SolidifyPW::Clone(RemapDir& remap)
{
	SolidifyPW* newmod = new SolidifyPW();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult SolidifyPW::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	if (message == REFMSG_CHANGE)
	{
		if (hTarget == pblock)
		{
			ParamID changing_param = pblock->LastNotifyParamID();
			if (changing_param == pb_bevelshape)
			{
				if ((partID == PART_TOPO) || (partID == PART_GEOM))
					return REF_SUCCEED;
				else return REF_STOP;
			}
		}

	}	
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void SolidifyPW::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL SolidifyPW::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void SolidifyPW::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}




int SolidifyPW::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags, 
		ModContext *mc) 
	{	

	return 0;

	GraphicsWindow *gw = vpt->getGW();
	// Transform the gizmo with the node.
	Matrix3 ntm = inode->GetObjectTM(t);
	gw->setTransform(ntm);	

	Point3 selGizmoColor = GetUIColor(COLOR_SEL_GIZMOS);
	Point3 gizmoColor = GetUIColor(COLOR_GIZMOS);

	Point3 green(0.0f,1.0f,0.0f);
	Point3 blue(0.0f,0.0f,1.0f);
	Point3 red(1.0f,0.0f,0.0f);

	gw->setColor(LINE_COLOR, gizmoColor);

	for (int i = 0; i < meshInfo.bevelData.Count(); i++)
	{
		if (meshInfo.bevelData[i].ct == 2)

		{
			Point3 l[3];

			gw->setColor(LINE_COLOR, gizmoColor);

			l[0] = meshInfo.bevelData[i].p;
			
			l[1] = meshInfo.bevelData[i].p + meshInfo.bevelData[i].vec[0]*1.f;
			gw->polyline(2, l, NULL, NULL, 0);

			l[1] = meshInfo.bevelData[i].p + meshInfo.bevelData[i].vec[1]*1.f;
			gw->polyline(2, l, NULL, NULL, 0);

			gw->setColor(LINE_COLOR, selGizmoColor);

			
			l[1] = meshInfo.bevelData[i].p + meshInfo.bevelData[i].vnorm*10.f;
			gw->polyline(2, l, NULL, NULL, 0);

			gw->setColor(LINE_COLOR, red);
			
			l[1] = meshInfo.bevelData[i].p + meshInfo.bevelData[i].norm*1.f*meshInfo.bevelData[i].realLength;
			gw->polyline(2, l, NULL, NULL, 0);

/*
			gw->setColor(LINE_COLOR, green);

			l[1] = bevelData[i].p + bevelData[i].cross[0]*5.f;
			gw->polyline(2, l, NULL, NULL, 0);

			gw->setColor(LINE_COLOR, blue );

			l[1] = bevelData[i].p + bevelData[i].cross[1]*5.f;
			gw->polyline(2, l, NULL, NULL, 0);
*/

		}

	}

		return 0;


/*
	for (int i = 0; i < vPlanes.Count(); i++)
		{
		if (vPlanes[i].display)
			{
		
			gw->setColor(LINE_COLOR, gizmoColor);
			gw->marker(&vPlanes[i].debugP1,PLUS_SIGN_MRKR);
			gw->marker(&vPlanes[i].debugP2,PLUS_SIGN_MRKR);


			Point3 l[3];
			l[0] = vPlanes[i].debugP1;
			l[1] = vPlanes[i].debugP2;
			gw->polyline(2, l, NULL, NULL, 0);

			gw->setColor(LINE_COLOR, selGizmoColor);
			gw->marker(&vPlanes[i].debugBase,PLUS_SIGN_MRKR);

			l[0] = vPlanes[i].debugBase;
			l[1] = vPlanes[i].debugBase+(vPlanes[i].debugNorm);
			gw->polyline(2, l, NULL, NULL, 0);

			gw->setColor(LINE_COLOR, red);
			l[0] = vPlanes[i].debugBase;
			l[1] = vPlanes[i].debugBase+(vPlanes[i].vnorm);
			gw->polyline(2, l, NULL, NULL, 0);


			}

		}

*/
	

	for (i = 0; i < meshInfo.op.Count(); i++)
		{

		gw->setColor(LINE_COLOR, selGizmoColor);
		
		gw->marker(&meshInfo.cdata[i].p,PLUS_SIGN_MRKR);


		Point3 l[3];
		l[0] = meshInfo.op[i];
		l[1] = meshInfo.op[i] + meshInfo.vnorms[i];

		gw->polyline(2, l, NULL, NULL, 0);

		gw->setColor(LINE_COLOR, gizmoColor);

//		l[0] = cdata[i].p;
//		l[1] = cdata[i].p + cdata[i].hyp;
//
//		gw->polyline(2, l, NULL, NULL, 0);

//		l[0] = cdata[i].p + cdata[i].norm;;
//		l[1] = cdata[i].p + cdata[i].hyp;

//		gw->setColor(LINE_COLOR, red);
//		gw->polyline(2, l, NULL, NULL, 0);

		}

	return 0;	
	}



void SpinnerOn(HWND hWnd,int SpinNum,int Winnum, BOOL enable)
{	
	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	if (spin2 != NULL)
		{
		spin2->Enable(enable);
		EnableWindow(GetDlgItem(hWnd,Winnum),enable);
		ReleaseISpinner(spin2);
		}

};

void SolidifyPW::EnableUIControls()

{
	if ((ip) && hWnd)
	{
		TimeValue t = 0;
		Interval iv;

		BOOL ioverrideMatID;
		pblock->GetValue(pb_overrideinnermatid,t,ioverrideMatID,iv);
		SpinnerOn(hWnd,IDC_MATIDSPIN2,IDC_MATID2, ioverrideMatID);
		EnableWindow(GetDlgItem(hWnd,IDC_IMATIDSTATIC),ioverrideMatID);


		BOOL ooverrideMatID;
		pblock->GetValue(pb_overrideoutermatid,t,ooverrideMatID,iv);
		SpinnerOn(hWnd,IDC_MATIDSPIN3,IDC_MATID3, ooverrideMatID);
		EnableWindow(GetDlgItem(hWnd,IDC_OMATIDSTATIC),ooverrideMatID);


		BOOL overrideMatID;
		pblock->GetValue(pb_overridematid,t,overrideMatID,iv);
		SpinnerOn(hWnd,IDC_MATIDSPIN,IDC_MATID, overrideMatID);
		EnableWindow(GetDlgItem(hWnd,IDC_EMATIDSTATIC),overrideMatID);



		BOOL autoSmooth;
		pblock->GetValue(pb_autosmooth,t,autoSmooth,iv);
		SpinnerOn(hWnd,IDC_AUTOSMOOTHSPIN,IDC_AUTOSMOOTH, autoSmooth);
		EnableWindow(GetDlgItem(hWnd,IDC_ANGLESTATIC),autoSmooth);



		if (autoSmooth)
			{
			EnableWindow(GetDlgItem(hWnd,IDC_SMOOTHCHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_SGSTATIC),FALSE);
			SpinnerOn(hWnd,IDC_SMOOTHGROUPSPIN,IDC_SMOOTHGROUP, FALSE);
			}
		else
			{
			EnableWindow(GetDlgItem(hWnd,IDC_SMOOTHCHECK),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_SGSTATIC),TRUE);

			BOOL overridesg;
			pblock->GetValue(pb_overridesg,t,overridesg,iv);
			SpinnerOn(hWnd,IDC_SMOOTHGROUPSPIN,IDC_SMOOTHGROUP, overridesg);
			}

		int edgeMap;
		pblock->GetValue(pb_edgemap,t,edgeMap,iv);
		if (edgeMap < 2 )
			{
			EnableWindow(GetDlgItem(hWnd,IDC_TVSTATIC),FALSE);
			SpinnerOn(hWnd,IDC_TVOFFSETSPIN,IDC_TVOFFSET, FALSE);
			}
		else 
			{
			EnableWindow(GetDlgItem(hWnd,IDC_TVSTATIC),TRUE);
			SpinnerOn(hWnd,IDC_TVOFFSETSPIN,IDC_TVOFFSET, TRUE);
			}



	}	

}

BOOL SolidifyPWDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			mod->hWnd = hWnd;
			mod->EnableUIControls();
			break;
		}
	return FALSE;
	}
