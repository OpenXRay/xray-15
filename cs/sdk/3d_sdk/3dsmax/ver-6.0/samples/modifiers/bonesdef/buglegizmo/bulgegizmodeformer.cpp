#include "mods.h"
#include "BulgeGizmo.h"



static void ComputeTCBMults(float tens, float cont, float &m1, float &m2)
	{ 	
	float tm,cm,cp;	
	tm = 0.5f*(1.0f - tens);
	cm = 1.0f - cont;       
	cp = 2.0f - cm;
	m1 = tm*cm;
	m2 = tm*cp;
	}
static Point3 Reflect(Point3 axis, Point3 vect)
	{
   	axis = Normalize(axis);
	Point3 perp = Normalize(axis^vect) ^ axis;
	return (DotProd(vect,axis)*axis) - (DotProd(vect,perp)*perp);
	}

//------------------------------------------------------------------- 
// TCB ... except we're not going to use bias

static void ComputeHermiteBasis(float u, float *v) 
	{
	float u2,u3,a;
	
	u2 = u*u;
	u3 = u2*u;
	a  = 2.0f*u3 - 3.0f*u2;
	v[0] = 1.0f + a;
	v[1] = -a;
	v[2] = u - 2.0f*u2 + u3;
	v[3] = -u2 + u3;
	}
static Point3 InterpSpline(float u,Point3 knot[4],float m1, float m2)
	{
	float v[4];
	Point3 p;
	ComputeHermiteBasis(u,v);
	float c[4];
	c[0] = -v[2]*m2;
	c[1] = v[0] + v[2]*(m2-m1) - v[3]*m1;
	c[2] = v[1] + v[2]*m1 + v[3]*(m1-m2);
	c[3] = v[3]*m2;
	p =  knot[0]*c[0] + knot[1]*c[1] + knot[2]*c[2] + knot[3]*c[3];

	return p;
	}




void GizmoParamsMapDlgProc::FilloutText()
{
TSTR name;
//get parent id
int parent;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,parent, FOREVER);
TCHAR *temp = giz->bonesMod->GetBoneName(parent);
name.printf("%s: %s",GetString(IDS_PW_GIZMOPARAM_PARENT),temp);
//set the text of the parent
SetWindowText(GetDlgItem(giz->hGizmoParams,IDC_PARENT_TEXT),
					name.data());
//get child id
int child;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_child_id,0,child, FOREVER);
//set the text of the child
temp = giz->bonesMod->GetBoneName(child);
name.printf("%s: %s",GetString(IDS_PW_GIZMOPARAM_CHILD),temp);
SetWindowText(GetDlgItem(giz->hGizmoParams,IDC_CHILD_TEXT),
					name.data());

}

BOOL GizmoParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{

			giz->hGizmoParams = hWnd;
			ICurveCtl *graph;
			ReferenceTarget *ref;

			giz->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
			if (ref)
				{
				graph = (ICurveCtl *) ref;
				graph->SetCustomParentWnd(GetDlgItem(giz->hGizmoParams, IDC_GRAPH));
				graph->SetMessageSink(giz->hGizmoParams);
				}

			FilloutText();
//			BOOL enable;
//			giz->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_use_graph,0,enable, FOREVER);
//			enable = !enable;
//			IParamMap2 *pmap = giz->pblock_gizmo_data->GetMap();
	
//			pmap->Enable(skin_gizmoparam_bulge_amount, enable);

			ICustButton *iBut = GetICustButton(GetDlgItem(giz->hGizmoParams, IDC_EDIT));
			if (GetCOREInterface()->GetSubObjectLevel() == 0)
				iBut->Enable(FALSE);
			else iBut->Enable(TRUE);
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);

			break;
			}
		case WM_CC_CHANGE_CURVEPT:
		case WM_CC_CHANGE_CURVETANGENT:
		case WM_CC_DEL_CURVEPT:
			if (!giz->stopProp)
				{
//				giz->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}	
			break;
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{


				case IDC_GRAPH:
					{
					ICurveCtl *graph;
					ReferenceTarget *ref;

					giz->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
					if (ref)
						{
						graph = (ICurveCtl *) ref;
						graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|CC_DRAWUTOOLBAR|
						  CC_DRAWRULER|CC_DRAWGRID|CC_DRAWLTOOLBAR|CC_SHOW_CURRENTXVAL|CC_ALL_RCMENU|CC_SINGLESELECT|CC_NOFILTERBUTTONS);
						if (graph->IsActive())
							graph->SetActive(FALSE);
						else 
							{
							graph->SetActive(TRUE);
							graph->ZoomExtents();
							}

						}

					break;
					}
				}
			break;
			}



		}
	return FALSE;
	}





Point3 GizmoBulgeClass::InterpSpline2(float u,Point3 *knot)
	{
	Spline3D spl;
	Point3 knotsPts[14];

	Point3 vec = (knot[1]-knot[0]) /3.f;

	knotsPts[0] = knot[0] - vec;
	knotsPts[1] = knot[0];
	knotsPts[2] = knot[0] + vec;

	knotsPts[3] = knot[1] - vec;
	knotsPts[4] = knot[1];
	knotsPts[5] = knot[1] + vec;;

	knotsPts[6] = knot[2];

	vec = (knot[4]-knot[3]) /3.f;
	knotsPts[7] = knot[3] - vec;
	knotsPts[8] = knot[3];
	knotsPts[9] = knot[3] + vec;

	knotsPts[10] = knot[4] - vec;
	knotsPts[11] = knot[4];
	knotsPts[12] = knot[4] + vec;

	spl.NewSpline();

	SplineKnot k1(KTYPE_BEZIER, LTYPE_CURVE, knotsPts[1], knotsPts[0], knotsPts[2]);
	SplineKnot k2(KTYPE_BEZIER, LTYPE_CURVE, knotsPts[4], knotsPts[3], knotsPts[5]);
	SplineKnot k3(KTYPE_AUTO, LTYPE_CURVE, knotsPts[6], knotsPts[6], knotsPts[6]);
	SplineKnot k4(KTYPE_BEZIER, LTYPE_CURVE, knotsPts[8], knotsPts[7], knotsPts[9]);
	SplineKnot k5(KTYPE_BEZIER, LTYPE_CURVE, knotsPts[11], knotsPts[10], knotsPts[12]);

	spl.AddKnot(k1);
	spl.AddKnot(k2);
	spl.AddKnot(k3);
	spl.AddKnot(k4);
	spl.AddKnot(k5);
	spl.SetKnotType(0,KTYPE_BEZIER);
	spl.SetKnotType(1,KTYPE_BEZIER);
	spl.SetKnotType(2,KTYPE_AUTO);
	spl.SetKnotType(3,KTYPE_BEZIER);
	spl.SetKnotType(4,KTYPE_BEZIER);
	spl.SetOpen();

	spl.InvalidateGeomCache();
	spl.ComputeBezPoints();
	int segment = 0;
	float tu;
	if (u <= 0.25f)
		{
		segment = 0;
		tu = u/0.25f;
		}
	else if (u <= 0.5f)
		{
		segment = 1;
		tu = (u-0.25f)/0.25f;
		}
	else if (u <= 0.75f)
		{
		segment = 2;
		tu = (u-0.5f)/0.25f;
		}
	else 
		{
		segment = 3;
		tu = (u-0.75f)/0.25f;
		}
	Point3 p = spl.InterpBezier3D(segment,tu);


	return p;
	}









class GizmoBulgeClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new GizmoBulgeClass; }
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOBULGE); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
											   
	Class_ID		ClassID() { return GIZMOBULGE_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_PW_GIZMOCATEGORY);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("gizmoBulge"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle



	};

static GizmoBulgeClassDesc gizmoBulgeDesc;
extern ClassDesc* GetGizmoBulgeDesc() {return &gizmoBulgeDesc;}

class GizmoBulgePBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		GizmoBulgeClass* p = (GizmoBulgeClass*)owner;

		switch (id)
		{
			case skin_gizmoparam_name:

					if (p->bonesMod)
						p->bonesMod->UpdateGizmoList();
				
				break;

		}
	}
};

static GizmoBulgePBAccessor gizmoJoint_accessor;


// per instance gizmo block
static ParamBlockDesc2 skin_gizmobulgeparam_blk ( skin_gizmobulgeparam, _T("gizmos"),  0, &gizmoBulgeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_BONESDEFGIZMOSBULGE, IDS_PW_GIZMOPARAM, 0,0, NULL,
	// params
	skin_gizmoparam_name, 	_T("name"),		TYPE_STRING, 	0,  IDS_PW_GIZMOPARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_NAME,
		p_accessor,		&gizmoJoint_accessor,
		end, 

/*  skin_gizmoparam_bulge_amount,  _T("bulge_amount"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PW_GIZMOPARAM_BULGE, 
		p_default, 		0.5f	,
		p_range, 		-10.f, 10.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARAM1,IDC_PARAM_SPIN1,  0.05f,
		end, 
*/

	skin_gizmoparam_bulge_twist,  _T("twist"),	TYPE_FLOAT, 	P_RESET_DEFAULT|P_ANIMATABLE, 	IDS_PW_GIZMOPARAM_TWIST, 
		p_default, 		0.f	,
		p_range, 		-360.0f, 360.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARAM2,IDC_PARAM_SPIN2,  1.0f,
		end, 

	skin_gizmoparam_bulge_parent_id,  _T("parent"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_PARENT, 
		end, 
	skin_gizmoparam_bulge_child_id,  _T("child"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_CHILD, 
		end, 
/*	skin_gizmoparam_bulge_graph,  _T("graph"),	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_GIZMOPARAM_GRAPH, 
		end, 

	skin_gizmoparam_bulge_use_graph,  _T("use_graph"),	TYPE_BOOL,	0, 	IDS_PW_GIZMOPARAM_USEGRAPH, 
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_GRAPH_ENABLE,
		p_accessor,		&gizmoJoint_accessor,
		end, 
	*/

	skin_gizmoparam_bulge_points,  _T("points"),	TYPE_POINT3_TAB,20, 	0, 	IDS_PW_GIZMOPARAM_POINTS, 
		end, 
//	skin_gizmoparam_bulge_deformed_points,  _T("deformed_points"),	TYPE_POINT3_TAB,20, 	0, 	IDS_PW_GIZMOPARAM_DEFORMED_POINTS, 
//		end, 
	skin_gizmoparam_bulge_initial_angle,  _T("initial_angle"),	TYPE_FLOAT,	0, 	IDS_PW_GIZMOPARAM_INITIALANGLE, 
		p_default, 		180.f	,
		end, 

	skin_gizmoparam_bulge_use_volume,  _T("use_volume"),	TYPE_BOOL,	0, 	IDS_PW_GIZMOPARAM_USEVOLUME, 
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_USE_VOLUME,
		end, 

	skin_gizmoparam_bulge_enable,  _T("enable"),	TYPE_BOOL,	0, 	IDS_PW_ENABLE, 
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_ENABLE,
		end, 

	skin_gizmoparam_bulge_orientation,  _T("orientation"),	TYPE_POINT3,	0, 	IDS_PW_ORIENTATION, 
		p_default, 		Point3(1.0f,0.0f,0.0f),
		end, 

   skin_gizmoparam_bulge_selection,  _T("selection"),	TYPE_INT_TAB,20,	0, 	IDS_PW_SELECTION, 
		p_default, 		0,
		end, 

	skin_gizmoparam_bulge_editing,  _T("editing"),	TYPE_BOOL,	P_RESET_DEFAULT, 	IDS_PW_GIZMOPARAM_EDITING, 
		p_default, 		FALSE,
		p_ui, 			TYPE_CHECKBUTTON, IDC_EDIT,
		end, 


	skin_gizmoparam_bulge_keygraph,  _T("graph"),	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_GIZMOPARAM_GRAPH, 
		end,   


	end
	);

GizmoBulgeClass::GizmoBulgeClass()
{
pblock_gizmo_data = NULL;
GetGizmoBulgeDesc()->MakeAutoParamBlocks(this);
dim[0] = 2;
dim[1] = 2;
dim[2] = 5;
pts = NULL;
stopReferencing = FALSE;
stopProp = FALSE;

//NEW FIX
useNewRotationPlane = TRUE;

}
BOOL GizmoBulgeClass::IsVolumeBased()
{
	if (pblock_gizmo_data)
		{
		BOOL useVol;
		pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_use_volume,0,useVol,FOREVER);
		return useVol;
		}
	return FALSE;
}

BOOL GizmoBulgeClass::IsEnabled()
{
BOOL enabled;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_enable,0,enabled,FOREVER);
return enabled;

}

BOOL GizmoBulgeClass::IsInVolume(Point3 p, Matrix3 tm)
{
//put us in world space
p = p *tm;
//now put us in gizmo space
p = p * gizmoTmToLocal;
p.x += 0.5f;
p.y += 0.5f;

if ((p.x < 0.0f) || (p.x > 1.0f)) return FALSE;
if ((p.y < 0.0f) || (p.y > 1.0f)) return FALSE;
if ((p.z < 0.0f) || (p.z > 1.0f)) return FALSE;
//transform to joint space
return TRUE;
}

void GizmoBulgeClass::SetInitialName()
{

if ((pblock_gizmo_data) && (bonesMod))
	{
	int parent;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,parent,FOREVER);
	TSTR name;
	name.printf("%s - %s",GetString(IDS_PW_GIZMOBULGE),bonesMod->GetBoneFlat(parent)->GetName());
	pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,name.data());
	}
else pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,GetString(IDS_PW_GIZMOBULGE));
}

TCHAR *GizmoBulgeClass::GetName()
{
TCHAR *name;
pblock_gizmo_data->GetValue(skin_gizmoparam_name,0,name,FOREVER);
return name;
}
void GizmoBulgeClass::SetName(TCHAR *name)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,name);

}



void GizmoBulgeClass::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{

	gizmoBulgeDesc.BeginEditParams(ip, this, flags, prev);
	skin_gizmobulgeparam_blk.SetUserDlgProc(new GizmoParamsMapDlgProc(this));
	// install a callback for the type in.
	this->ip = ip;

	}

void GizmoBulgeClass::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	EndEditing();
	ICurveCtl *graph;
	ReferenceTarget *ref;

	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
	

	if (ref)
		{
		graph = (ICurveCtl *) ref;
		if (graph->IsActive())
			graph->SetActive(FALSE);
		}

	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_editing,0,FALSE);			
	
	gizmoBulgeDesc.EndEditParams(ip, this, flags, next);
	ip = NULL;
	}

BOOL GizmoBulgeClass::IsEditing()
{
BOOL editing;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_editing,0,editing,FOREVER);
return editing;
}

void GizmoBulgeClass::EndEditing()
{
BOOL editing=FALSE;
pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_editing,0,editing);
}

void GizmoBulgeClass::EnableEditing(BOOL enable) 
{
ICustButton *iBut = GetICustButton(GetDlgItem(hGizmoParams, IDC_EDIT));
if (iBut)
	{
	iBut->Enable(enable);
	ReleaseICustButton(iBut);
	}

}

void GizmoBulgeClass::Enable(BOOL enable)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_enable,0,enable);
}

//NEW FIX
void GizmoBulgeClass::ResetRotationPlane()
{

	Matrix3 parentTm, childTm;
	Point3 l1,l2,l3,l4;
	int parent, child;

	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_child_id,0,child,FOREVER);

	bonesMod->GetEndPoints(parent, l1, l2);
	bonesMod->GetEndPoints(child, l3, l4);

	Point3 dir;
	Point3 tl3,tl4;
	Interval iv;

	Matrix3 tempChildTm = bonesMod->GetBoneFlat(child)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	Matrix3 tempParentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	tl3 = l3 * tempChildTm * Inverse(tempParentTm);
	tl4 = l4 * tempChildTm * Inverse(tempParentTm);

//	tempChildTm = bonesMod->GetBoneTm(child);
//	tempParentTm = bonesMod->GetBoneTm(parent);
//	tl3 = l3 * Inverse(tempChildTm) * tempParentTm;
//	tl4 = l4 * Inverse(tempChildTm) * tempParentTm;
	Point3 crossProd;
	crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));
	float dotTemp = DotProd(Normalize(l1-l2),Normalize(tl4-tl3));

	float tdot = fabs(1.0f-fabs(dotTemp));

	if (tdot <= 0.00001f)
//	if (fabs(dotTemp) == 1.0f)

		{
		return;
/*
		tl3 = l3 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);

		tl4 = l4 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
		tl4.y += 10.f;
		crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));
*/
		}
	dir = Normalize(crossProd);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_orientation,0,dir);



	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	childTm = bonesMod->GetBoneFlat(child)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	tl3 = l3 * childTm * Inverse(parentTm);
	tl4 = l4 * childTm * Inverse(parentTm);
	Point3 newDir = Normalize(CrossProd(Normalize(l1-l2),Normalize(tl4-tl3)));
//NEW FIX	
	rotationPlane = newDir;

//compute the initial angle between the joints
	Point3 tl2 = l2;
	l1 = l1 *  Inverse(bonesMod->GetBoneTm(parent));
	l2 = l2 *  Inverse(bonesMod->GetBoneTm(parent));
	l3 = l3 * Inverse(bonesMod->GetBoneTm(child));
	l4 = l4 * Inverse(bonesMod->GetBoneTm(child));
	float dot = DotProd(Normalize(l1-l2),Normalize(l4-l3));
	float angle;

	tdot = fabs(1.0f-fabs(dot));

	if (tdot <= 0.00001f)
//	if (fabs(dot) == 1.0f) 
		angle = 180.f;
	else angle = acos(dot) * 180.f/PI;


//NEW FIX
	dot = DotProd(newDir,dir);
	if (useNewRotationPlane)
		{
		if (dot < 0.0f)	angle = (180-angle)+180;
		}
	else
		{
		if (Length(newDir -dir) < 0.001f) angle = (180-angle)+180;
		}
	

	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_initial_angle,0,angle);

}




Matrix3 GizmoBulgeClass::CreateInitialAngles()
{

	Matrix3 parentTm, childTm;
	Point3 l1,l2,l3,l4;
	int parent, child;

	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_child_id,0,child,FOREVER);

	bonesMod->GetEndPoints(parent, l1, l2);
	bonesMod->GetEndPoints(child, l3, l4);

	Point3 dir;
	Point3 tl3,tl4;
	tl3 = l3 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
	tl4 = l4 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
	Point3 crossProd;
	crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));
	float dotTemp = DotProd(Normalize(l1-l2),Normalize(tl4-tl3));
	if (fabs(dotTemp) == 1.0f)
		{
		tl3 = l3 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);

		tl4 = l4 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
		tl4.y += 10.f;
		crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));

		}
	dir = Normalize(crossProd);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_orientation,0,dir);

	Interval iv;
	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	childTm = bonesMod->GetBoneFlat(child)->GetObjectTM(GetCOREInterface()->GetTime(),&iv);
	tl3 = l3 * childTm * Inverse(parentTm);
	tl4 = l4 * childTm * Inverse(parentTm);
	Point3 newDir = Normalize(CrossProd(Normalize(l1-l2),Normalize(tl4-tl3)));
//NEW FIX	
	rotationPlane = newDir;

//compute the initial angle between the joints
	Point3 tl2 = l2;
	l1 = l1 *  Inverse(bonesMod->GetBoneTm(parent));
	l2 = l2 *  Inverse(bonesMod->GetBoneTm(parent));
	l3 = l3 * Inverse(bonesMod->GetBoneTm(child));
	l4 = l4 * Inverse(bonesMod->GetBoneTm(child));
	float dot = DotProd(Normalize(l1-l2),Normalize(l4-l3));
	float angle;
	if (fabs(dot) == 1.0f) 
		angle = 180.f;
	else angle = acos(dot) * 180.f/PI;


//NEW FIX
	dot = DotProd(newDir,dir);
	if (useNewRotationPlane)
		{
		if (dot < 0.0f)	angle = (180-angle)+180;
		}
	else
		{
		if (Length(newDir -dir) < 0.001f) angle = (180-angle)+180;
		}

//	if (Length(newDir -dir) < 0.001f)
//		angle = (180-angle)+180;

	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_initial_angle,0,angle);



	l1 = Point3(0.f,0.0f,0.0f) *  Inverse(bonesMod->GetBoneTm(parent));
	l2 = tl2 *  Inverse(bonesMod->GetBoneTm(parent));
	l3 = Point3(0.f,0.0f,0.0f) * Inverse(bonesMod->GetBoneTm(child));

	if (fabs(dot) == 1.0f)
		l2.y += 10.f;
//get its transform
//create a matric aligned to the l1 and l2 of the bone
	Point3 x,y,z,pos(.0f,0.0f,0.0f);
	pos = l2;
	z = Normalize(l4-l1);
	if (Normalize(l4-l1) == Normalize(l2-l1))
		{
		y = z;
		y.y += 10.0f;
		y = CrossProd(Normalize(y),Normalize(l2-l1));
		x = CrossProd(Normalize(l4-l1),Normalize(y));
		}
	else
		{
		y = CrossProd(Normalize(l4-l1),Normalize(l2-l1));
		x = CrossProd(Normalize(l4-l1),Normalize(y));
		}


	Matrix3 tm(Normalize(y),Normalize(x),Normalize(z),pos);
	gizmoTm = tm * bonesMod->GetBoneTm(parent);

	return tm;

}

BOOL GizmoBulgeClass::InitialCreation(int count, Point3 *pt, int numbeOfInstances, int *mapTable) 
{
	if (count == 0) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),GetString(IDS_PW_VERTEXERROR),NULL,MB_OK);
		return FALSE;
		}
	int parent = -1, child = -1;
//get selected bone
	child = bonesMod->GetSelectedBone();
	INode *childNode = bonesMod->GetBoneFlat(child);
	INode *parentNode;
	parentNode = childNode->GetParentNode();
//figure out the parnet
//loop through bones looking for any that have the matching parent
	for (int i = 0; i < bonesMod->GetNumBonesFlat(); i++)
		{
		if (bonesMod->GetBoneFlat(i))
			{
			if (bonesMod->GetBoneFlat(i) == parentNode)
				{
				parent = i;
				i = bonesMod->GetNumBonesFlat();
				}
			}
		}

	
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_parent_id,0,parent);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_child_id,0,child);

	if ((parent < 0) || (child < 0)) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),GetString(IDS_PW_PARENTERROR),NULL,MB_OK);
		return FALSE;
		}


	Matrix3 tm = CreateInitialAngles();
//align it to the plane of l1 parent to l2 child
//transform the points in the space
//compute the bounding box
	
	Box3 bbox;
	bbox.Init();
	Point3 tp;
	for (i = 0; i < count; i++)
		{
		tp = pt[i] * Inverse(tm);
		bbox += tp;
		}
	gizmoTm = tm * bonesMod->GetBoneTm(parent);

	bbox.Scale(1.1f);
	Point3 pos;
	pos.x = (bbox.pmax.x + bbox.pmin.x)*0.5f;
	pos.y = (bbox.pmax.y + bbox.pmin.y)*0.5f;
	pos.z = bbox.pmin.z;
	
	tm.PreTranslate(pos);

	bbox.pmin -= pos;
	bbox.pmax -= pos;

	Point3 scale;
	scale = bbox.pmax - bbox.pmin;
	tm.PreScale(scale);

	
	gizmoTm = tm * bonesMod->GetBoneTm(parent);
//	tm.PreTranslate(pos);

	bbox.pmin.x =-0.5f;
	bbox.pmin.y =-0.5f;
	bbox.pmin.z =0.0f;

	bbox.pmax.x =.5f;
	bbox.pmax.y =.5f;
	bbox.pmax.z =1.0f;

//create points now 
	Point3 p[20];
	p[0] = bbox[0];
	p[1] = bbox[1];
	p[2] = bbox[2];
	p[3] = bbox[3];


	p[16] = bbox[4];
	p[17] = bbox[5];
	p[18] = bbox[6];
	p[19] = bbox[7];

	Point3 vec[4];
	vec[0] = p[16]-p[0];
	vec[1] = p[17]-p[1];
	vec[2] = p[18]-p[2];
	vec[3] = p[19]-p[3];

	p[4] = bbox[0] + (vec[0] *0.25f);
	p[5] = bbox[1] + (vec[1] *0.25f);
	p[6] = bbox[2] + (vec[2] *0.25f);
	p[7] = bbox[3] + (vec[3] *0.25f);

	p[8] = bbox[0]+ (vec[0] *0.5f);
	p[9] = bbox[1]+ (vec[0] *0.5f);
	p[10] = bbox[2]+ (vec[0] *0.5f);
	p[11] = bbox[3]+ (vec[0] *0.5f);

	p[12] = bbox[0]+ (vec[0] *0.75f);
	p[13] = bbox[1]+ (vec[0] *0.75f);
	p[14] = bbox[2]+ (vec[0] *0.75f);
	p[15] = bbox[3]+ (vec[0] *0.75f);

	for (i=0; i < 20; i ++)
		pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_points,0,p[i],i);

	ICurveCtl *graph;
	
	graph = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);

	graph->SetXRange(0,360);
	graph->SetYRange(-10.0f,10.0f);

	graph->RegisterResourceMaker((ReferenceMaker *)this);
	graph->SetNumCurves(60);
	

	graph->SetTitle(GetString(IDS_PW_GRAPH));
	graph->SetZoomValues( 2.7f, 80.0f);
	
	BitArray ba;
	ba.SetSize(60);
	ba.ClearAll();
	graph->SetDisplayMode(ba);
	graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
					  CC_DRAWRULER|CC_DRAWGRID|
					  CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|
					  CC_RCMENU_SCALE|CC_RCMENU_INSERT_CORNER | CC_RCMENU_INSERT_BEZIER|
					CC_RCMENU_DELETE | CC_SHOW_CURRENTXVAL	
					);	

	graph->SetCustomParentWnd(GetDlgItem(hGizmoParams, IDC_GRAPH));
	
	stopReferencing = TRUE;
	graph->SetMessageSink(hGizmoParams);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_keygraph,0,(ReferenceTarget*)graph);

	float iangle;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_initial_angle,0,iangle,FOREVER);
	stopReferencing = FALSE;
	for (i=0; i < 20; i++)
		{
		ICurve *curve = graph->GetControlCurve(i*3);
		curve->SetCanBeAnimated(FALSE);
		AColor acol;
		acol.r = 1.0f;
		acol.g = 0.0f;
		acol.b = 0.0f;
		COLORREF col = acol.toRGB();
		curve->SetPenProperty(col);

		curve = graph->GetControlCurve(i*3+1);
		curve->SetCanBeAnimated(FALSE);
		
		acol.r = 0.0f;
		acol.g = 1.0f;
		acol.b = 0.0f;
		col = acol.toRGB();
		curve->SetPenProperty(col);


		curve = graph->GetControlCurve(i*3+2);
		curve->SetCanBeAnimated(FALSE);
		
		acol.r = 0.0f;
		acol.g = 0.0f;
		acol.b = 1.0f;
		col = acol.toRGB();
		curve->SetPenProperty(col);


		KeyGraph(i, iangle,Point3(0.0f,0.0f,0.0f));
		}
	
	return TRUE;
		
}



void GizmoBulgeClass::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{
Point3 p;
Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return;
INode *node = bonesMod->GetBoneFlat(id);
if (!node) return;

toWorld = node->GetObjectTM(t);
toWorld =  gizmoTm*toWorld  ;

for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p,FOREVER,i);
	p = deformedPoints[i];
	p = p * toWorld;
	box += p;
	}

}

class MoveRestore : public RestoreObj {
public:	
	Point3 rcp[20];
	Point3 ucp[20];

//	BOOL csel[20];

	GizmoBulgeClass *mod;

	BOOL update;
	float angle;

	MoveRestore(GizmoBulgeClass *mod)
		{
		update = FALSE;
		this->mod = mod;
		angle = mod->currentBendAngle;

		ICurveCtl *graph=NULL;
		ReferenceTarget *ref =NULL;
		mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
		if (ref)
			graph = (ICurveCtl *) ref;

		for (int i=0; i < 20; i++)
			{
			int sel = 0;
//			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
//			csel[i] = i;


//			if (csel[i])
				{
				if (graph)
					{
					ICurve *cv = graph->GetControlCurve(i*3);
					Point3 p;
					p.x = cv->GetValue(0,angle);

					cv = graph->GetControlCurve(i*3+1);
					p.y = cv->GetValue(0,angle);

					cv = graph->GetControlCurve(i*3+2);
					p.z = cv->GetValue(0,angle);
					ucp[i] = p;
					}
				}
			}


		}
	~MoveRestore () { }
	void Restore(int isUndo)
		{
		if (isUndo)
			{
			
			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
			if (ref)
				graph = (ICurveCtl *) ref;

			for (int i=0; i < 20; i++)
				{
//				if (csel[i])
					{
//					if (graph)
						{
						ICurve *cv = graph->GetControlCurve(i*3);
						Point3 p;
						p.x = cv->GetValue(0,angle);

						cv = graph->GetControlCurve(i*3+1);
						p.y = cv->GetValue(0,angle);

						cv = graph->GetControlCurve(i*3+2);
						p.z = cv->GetValue(0,angle);
						rcp[i] = p;
						}
					}
				}
			}

			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
			if (ref)
				graph = (ICurveCtl *) ref;

			mod->stopReferencing = TRUE;
			for (int i=0; i < 20; i++)
				{
//				if (csel[i])
					{
					if (graph)
						{
						mod->KeyGraph(i,angle,ucp[i]);
						
						}
					}
				}
			mod->stopReferencing = FALSE;

		}
	void Redo()
		{
			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
			if (ref)
				graph = (ICurveCtl *) ref;

			mod->stopReferencing = TRUE;
			for (int i=0; i < 20; i++)
				{
//				if (csel[i])
					{
					if (graph)
						{
						mod->KeyGraph(i,angle,rcp[i]);
						
						}
					}
				}
			mod->stopReferencing = FALSE;
		}
	void EndHold() {
			update = TRUE;
			mod->ClearAFlag(A_HELD);
			mod->stopReferencing = FALSE;
			mod->stopProp = FALSE;
			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
			if (ref)
				graph = (ICurveCtl *) ref;

			if (graph)
				{
				graph->EnableDraw(TRUE);
				graph->Redraw();
			}
			mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
	TSTR Description() {return TSTR(_T("Move Lattice Point"));}
};


void GizmoBulgeClass::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin)

{
ICurveCtl *graph=NULL;
ReferenceTarget *ref =NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	graph = (ICurveCtl *) ref;

stopProp = TRUE;
if (graph)
	graph->EnableDraw(FALSE);

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return ;
INode *tnode = bonesMod->GetBoneFlat(id);
if (!tnode) return ;
toWorld = tnode->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld;

val = VectorTransform(tmAxis*Inverse(toWorld),val);



if ( ( theHold.Holding() )  &&  (!TestAFlag(A_HELD)) )
	{
	theHold.Put(new MoveRestore(this));
	SetAFlag(A_HELD);	
	}

for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);

	if (sel)
		{
		if (graph)
			{
			ICurve *cv = graph->GetControlCurve(i*3);
			Point3 p;
			p.x = cv->GetValue(0,currentBendAngle);

			cv = graph->GetControlCurve(i*3+1);
			p.y = cv->GetValue(0,currentBendAngle);

			cv = graph->GetControlCurve(i*3+2);
			p.z = cv->GetValue(0,currentBendAngle);
			
			theHold.Suspend();

			KeyGraph(i,currentBendAngle,val+p);
			theHold.Resume();

			}

		}
	}


if (graph)
	{
	Interval iv;
	graph->Update(0,iv);
	graph->EnableDraw(TRUE);
	graph->Redraw();
	graph->EnableDraw(FALSE);
	}
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
}


void GizmoBulgeClass::GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return ;
INode *tnode = bonesMod->GetBoneFlat(id);
if (!tnode) return ;
toWorld = tnode->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld  ;

int ct =0;
Point3 cp(0.0f,0.0f,0.0f);

int selCount = 0;
for (i=0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
	if (sel)
		selCount++;
	}

for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
	if ( (sel) || (selCount == 0) )
		{
		cp += p[i] * toWorld;
		ct++;
		}
	}
cp = cp /(float) ct;

cb->Center(cp,0);

}

void GizmoBulgeClass::GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return ;
INode *tnode = bonesMod->GetBoneFlat(id);
if (!tnode) return ;
toWorld = tnode->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld  ;

int ct =0;
Point3 cp(0.0f,0.0f,0.0f);

int selCount = 0;
for (i=0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
	if (sel)
		selCount++;
	}

for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
	if ( (sel) || (selCount ==0) )
		{
		cp += p[i] * toWorld;
		ct++;
		}
	}
cp = cp /(float) ct;

toWorld.SetTrans (cp);

cb->TM(toWorld,0);

}

void GizmoBulgeClass::ClearSelection(int selLevel)
{

for (int j = 0; j < 20; j++)
	{
	BOOL sel = FALSE;
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}
void GizmoBulgeClass::SelectAll(int selLevel)
{
for (int j = 0; j < 20; j++)
	{
	BOOL sel = TRUE;
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}

void GizmoBulgeClass::InvertSelection(int selLevel)
{
for (int j = 0; j < 20; j++)
	{
	BOOL sel = TRUE;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,j);		
	sel = !sel;
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}

void GizmoBulgeClass::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)

{


BOOL add = GetKeyState(VK_CONTROL)<0;
BOOL sub = GetKeyState(VK_MENU)<0;


if (!add && !sub) 
	{
	for (int j = 0; j < 20; j++)
		{
		BOOL sel = FALSE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,j);		
		}
	}

while (hitRec) 
	{
	int state = hitRec->hitInfo;
	if (sub)
		{
		BOOL sel = FALSE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,state);		

		}
	else 
		{
		BOOL sel = TRUE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_selection,0,sel,state);		
			
		}
	hitRec = hitRec->Next();
	}

BitArray ba;
ba.SetSize(60);
ba.ClearAll();
for (int i =0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);		
	if (sel)
		{
		ba.Set(i*3);
		ba.Set(i*3+1);
		ba.Set(i*3+2);
		}
	}

ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;
	graph->SetDisplayMode(ba);		
	}
}

int GizmoBulgeClass::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *ipt, ViewExp *vpt, ModContext* mc, Matrix3 tm)

{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return 1;
INode *node = bonesMod->GetBoneFlat(id);
if (!node) return 1;
toWorld = node->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld  ;


GraphicsWindow *gw = vpt->getGW();
Point3 pt;
HitRegion hr;
int savedLimits, res = 0;

MakeHitRegion(hr,type, crossing,4,ipt);


gw->setHitRegion(&hr);	
gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
gw->setTransform(toWorld);



for (i=0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
	
	if ((flags&HIT_SELONLY   &&  sel) ||
		(flags&HIT_UNSELONLY && !sel) ||
		!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
		{
		gw->clearHitCode();
		gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
		gw->marker(&p[i],POINT_MRKR);
		if (gw->checkHitCode()) 
			{
			vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL); 
			res = 1;
			}
		}
	}

//	gw->marker(&p[i],HOLLOW_BOX_MRKR);
//	}

return res;

}



int GizmoBulgeClass::Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm) 
{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);




Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);

if (id < 0) return 1;
INode *node = bonesMod->GetBoneFlat(id);
if (!node) return 1;
toWorld = node->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld  ;


//NEW FIX
Point3 dir;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_orientation,0,dir,FOREVER);

Matrix3 upVecTM(1);
Matrix3 iupVecTM(1);
ArbAxis(dir, upVecTM);
iupVecTM = Inverse(upVecTM);

Point3 dlp[3];
Point3 circle[17];
float angle = 0;
float inc = PI*2.0f/15.0f;
dlp[0] = Point3(0.0f,0.0f,0.0f)* node->GetObjectTM(t)*tm;
float radius = Length(VectorTransform(toWorld,Point3(1.0f,0.0f,0.0f)))*0.5f;
Point3 axis = Normalize(upVecTM.GetRow(1));
for (i = 0 ; i < 16; i++)
	{
	circle[i] = axis  * iupVecTM * upVecTM  *radius;// * node->GetObjectTM(t)*tm;;	
	upVecTM.PreRotateZ(inc);
	}
for (i = 0 ; i < 16; i++)
	circle[i] = circle[i] * node->GetObjectTM(t)*tm;
dlp[1] = dir * 50.0f* node->GetObjectTM(t)*tm;// dlp[0] + dir*50.0f;

//gw->polyline(2, dlp, NULL, NULL, 1,NULL);
gw->polyline(15, circle, NULL, NULL, 1,NULL);


for (i=0; i < 20; i++)
	{
	p[i] = p[i] * toWorld * tm;

//	gw->marker(&p[i],HOLLOW_BOX_MRKR);

	}
int start = 0;
for (i=0; i < 5; i++)
	{
	Point3 lp[5];
	lp[0] = p[start];
	lp[1] = p[start+1];
	lp[2] = p[start+3];
	lp[3] = p[start+2];

	gw->polyline(4, lp, NULL, NULL, 1,NULL);
	start+=4;
	}

start = 0;
for (i=0; i < 4; i++)
	{
	Point3 lp[6];
	lp[0] = p[start];
	lp[1] = p[start+4];
	lp[2] = p[start+8];
	lp[3] = p[start+12];
	lp[4] = p[start+16];

	gw->polyline(5, lp, NULL, NULL, 0,NULL);
	start+=1;
	}

for (i=0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);


	if (sel)
		gw->marker(&p[i],HOLLOW_BOX_MRKR);
	else gw->marker(&p[i],PLUS_SIGN_MRKR);

	}


Point3 subSelColor = GetUIColor(COLOR_SUBSELECTION);
float r = subSelColor.x;
float g = subSelColor.y;
float b = subSelColor.z;

gw->setColor(LINE_COLOR, r,g,b);
//TimeValue ct = (TimeValue)(currentBendAngle * 160.f);

BOOL hit[20];

//now get curve data
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;
//get points
	for (int i = 0; i < 20; i++)
		{
		hit[i] = FALSE;
		ICurve *cv = graph->GetControlCurve(i*3);
		for	(int j = 0; j < cv->GetNumPts(); j++)
			{
			CurvePoint cp = cv->GetPoint(0, j);
			if (currentBendAngle == cp.p.x)
				hit[i] = TRUE;
			}
		cv = graph->GetControlCurve(i*3+1);
		for	(j = 0; j < cv->GetNumPts(); j++)
			{
			CurvePoint cp = cv->GetPoint(0, j);
			if (currentBendAngle == cp.p.x)
				hit[i] = TRUE;
			}
		cv = graph->GetControlCurve(i*3+2);
		for	(j = 0; j < cv->GetNumPts(); j++)
			{
			CurvePoint cp = cv->GetPoint(0, j);
			if (currentBendAngle == cp.p.x)
				hit[i] = TRUE;
			}

		}
	}


for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_selection,0,sel,FOREVER,i);
//	BOOL hit = pblock_gizmo_data->KeyFrameAtTime(skin_gizmoparam_bulge_offsets, ct, i);
//	BOOL hit = FALSE;
	if (hit[i])
		{
		if (sel)
			gw->marker(&p[i],HOLLOW_BOX_MRKR);
		else gw->marker(&p[i],PLUS_SIGN_MRKR);
		}
	}



Point3 m[3];
Point3 zero(0.0f,0.0f,0.0f);
Point3 x1(10.0f,0.0f,0.0f);
Point3 y1(0.0f,10.0f,0.0f);
Point3 z1(0.0f,0.0f,10.0f);

return 1;
}


void GizmoBulgeClass::PostDeformSetup(TimeValue t)
{
if (pts) 
	{ 				
	for (int i=0; i<dim[0]+2; i++) 
		{
		for (int j=0; j<dim[1]+2; j++) 
			{
			delete[] pts[i][j];						
			}
		delete[] pts[i];
		}
	delete[] pts;			
	pts = NULL;
	}

}


void GizmoBulgeClass::PreDeformSetup(TimeValue t)
{

//bend the lattice
//points based on weights in local space
//child tm
int parentId, childId;
Interval iv;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,parentId,FOREVER);
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_child_id,0,childId,FOREVER);

if ((childId<0) || (parentId<0)) 
	{
	val = FALSE;
	return;
	}

INode *pNode = bonesMod->GetBoneFlat(parentId);
INode *cNode = bonesMod->GetBoneFlat(childId);
if (pNode && cNode)
	val= TRUE;
else
	{
	val = FALSE;
	return;
	}
Matrix3 ParentTm0 = Inverse(bonesMod->GetBoneTm(parentId));
//Matrix3 ParentTm0 = bonesMod->GetBoneFlat(parentId)->GetNodeTM(0,&iv);
inverseParentTm0 = Inverse(ParentTm0);

Matrix3 parentTm = bonesMod->GetBoneFlat(parentId)->GetObjectTM(t,&iv);
parentTmT = parentTm;

Matrix3 inverseParentTm = Inverse(parentTm);
Matrix3 childTmRefFrame = Inverse(bonesMod->GetBoneTm(childId));
Matrix3 inverseChildTmRefFrame = Inverse(childTmRefFrame);
Matrix3 childTm = bonesMod->GetBoneFlat(childId)->GetObjectTM(t,&iv);

//float bulge = 0.0f;
//BOOL useGraph = FALSE;

//pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_amount,0,bulge,FOREVER);
//pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_use_graph,0,useGraph,FOREVER);

Point3 initialPos[20];
Point3 defPos[20];

Matrix3 twist(1);
float twistAngle = 0.0f;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);
toWorld = bonesMod->GetBoneFlat(id)->GetObjectTM(t);
toWorld =  gizmoTm*toWorld  ;


Point3 p[21];
//Point3 pdef[21];

for (int i = 0; i < 20; i++)
	{
	Point3 p;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_points,0,p,FOREVER,i);

	defPos[i]  = p;
//	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,p,i);
	deformedPoints[i] = p;
	}
float currentAngle, initialAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_initial_angle,0,initialAngle,FOREVER);

Point3 l1,l2,l3,l4;
bonesMod->GetEndPoints(parentId, l1, l2);
bonesMod->GetEndPoints(childId, l3, l4);


Point3 tl3,tl4,dir;
tl3 = l3 * childTm * Inverse(parentTm);
tl4 = l4 * childTm * Inverse(parentTm);
Point3 newDir = Normalize(CrossProd(Normalize(l1-l2),Normalize(tl4-tl3)));
//NEW FIX
rotationPlane = newDir;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_orientation,0,dir,FOREVER);

//DebugPrint("%d Dir %f %f %f \n",t/160, dir.x, dir.y, dir.z);
//DebugPrint("%d NewDir %f %f %f \n",t/160, newDir.x, newDir.y, newDir.z);

//compute the initial angle between the joints
Point3 tl2 = l2;
l1 = l1 * parentTm;
l2 = l2 * parentTm;
l3 = l3 * childTm;
l4 = l4 * childTm;



float dot = DotProd(Normalize(l1-l2),Normalize(l4-l3));
if (fabs(dot)== 1.0f)
	currentAngle = 180.f;
else currentAngle = fabs(acos(dot) * 180.f/PI);

//NEW FIX
dot = DotProd(newDir,dir);
if (useNewRotationPlane)
	{
	if (dot < 0.0f)	currentAngle = (180-currentAngle)+180;
	}
else
	{
	if (Length(newDir -dir) < 0.001f) currentAngle = (180-currentAngle)+180;
	}
//if (Length(newDir -dir) < 0.001f)
//	currentAngle = (180-currentAngle)+180;
float per = 0.f;
float ang = currentAngle;

currentBendAngle = ang;

currentAngle = initialAngle - currentAngle;
per = currentAngle/180.f;


ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;
	if (graph->IsActive())
		{
		graph->SetCurrentXValue(currentBendAngle);
		}
	}

if (ip)
	{
	TSTR angleText;	
	angleText.printf(GetString(IDS_CURRENT_ANGLE),currentBendAngle);
//set the text of the parent
	SetWindowText(GetDlgItem(hGizmoParams,IDC_ANGLE_TEXT),
					angleText.data());

	}


/*

if (useGraph)
	{
	ICurveCtl *graph;
	ReferenceTarget *ref;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
	if (ref)
		{
		graph = (ICurveCtl *) ref;
//get value based on angle
		ICurve *cv = graph->GetControlCurve(0);
		bulge = cv->GetValue(t, ang);

		per = 1.0f;

		}
	}

*/
/*
//now compute the bias positions
Point3  biasVec;
biasVec = (defPos[8] - defPos[10]) *per;
if (bulge != 0.0f)
	{
	Point3 a,b;
	a = defPos[10] - biasVec * bulge;
	b = defPos[11] - biasVec * bulge;


	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,a,10);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,b,11);

	a = defPos[10-4] - biasVec * (bulge*.75f);
	b = defPos[11-4] - biasVec * (bulge*.75f);


	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,a,10-4);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,b,11-4);

	a = defPos[10+4] - biasVec * (bulge*.75f);
	b = defPos[11+4] - biasVec * (bulge*.75f);


	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,a,10+4);
	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,b,11+4);

	}
*/


graph = NULL;
ref = NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;

//get value based on angle
	for (int i =0; i < 20; i++)
		{
		ICurve *cv = graph->GetControlCurve(i*3);
		float off;
		latticeOffsets[i] = Point3(0.0f,0.0f,0.0f);

		off = cv->GetValue(0,currentBendAngle);
		latticeOffsets[i].x = off;

		cv = graph->GetControlCurve(i*3+1);
		off = cv->GetValue(0,currentBendAngle);
		latticeOffsets[i].y = off;

		cv = graph->GetControlCurve(i*3+2);
		off = cv->GetValue(0,currentBendAngle);
		latticeOffsets[i].z = off;

		}
	}


for (i =0; i < 20; i++)
	{
	Point3 p;
//	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_deformed_points,0,p,FOREVER,i);
	p = deformedPoints[i];
	p += latticeOffsets[i];

//	pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_deformed_points,0,p,i);
	deformedPoints[i] = p;
	pt[i] = p;
	offsets[i] = Point3(0.0f,0.0f,0.0f);
	}

InitializeFDD(t);

}



Point3 GizmoBulgeClass::DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)

{

if (!val) return p;
//tranform the point into world space

Point3 pWorld = p * tm;
Point3 iP = initialP * tm;
// Transform into lattice space
Point3 pp = iP*gizmoTmToLocal;

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);
Point3 initPoint = pp;
pp = pp ;
pp.x += 0.5f;
pp.y += 0.5f;

//defrom based on lattice
//we are going to deform a spline then use that spline to get the point
Point3 sp[5];
sp[0] = pp;
sp[1] = pp;
sp[2] = pp;
sp[3] = pp;
sp[4] = pp;

sp[0].z = 0.0f;
sp[1].z = 0.25f;
sp[2].z = 0.5f;
sp[3].z = 0.75f;
sp[4].z = 1.f;
for (int i = 0; i < 5; i++)
	{
	sp[i] = Map(index, sp[i]);
	}
if (pp.z <= 0.0f) pp.z = 0.001f;
if (pp.z >= 1.0f) pp.z = 0.999f;



Point3 tp = InterpSpline2(pp.z,sp);


pWorld = tp-initPoint;
Matrix3 concat = Inverse(gizmoTmToLocal) * inverseParentTm0 * parentTmT *Inverse(tm);
pWorld = VectorTransform(concat,pWorld);
/*
pWorld = VectorTransform(Inverse(gizmoTmToLocal),pWorld);
pWorld = VectorTransform(inverseParentTm0,pWorld);
pWorld = VectorTransform(parentTmT,pWorld);
*/
p+= pWorld;


/*
pWorld = tp;


pWorld = pWorld*Inverse(gizmoTmToLocal);

// initialP;
//then add parent motion
pWorld = pWorld * inverseParentTm0* parentTmT;

p += (pWorld * Inverse(tm)) - initialP;
*/


return p;
}


int GizmoBulgeClass::GridIndex(int i, int j, int k)
	{
	int ix = k*dim[0]*dim[1] + j*dim[0] + i;
	assert(ix>=0 && ix<20);
	return ix;
	}


Point3 GizmoBulgeClass::GetPtOR(int i, int j, int k)
	{
	int ii=i, jj=j, kk=k;
	if (i<0) i = 0; if (i>dim[0]-1) i = dim[0]-1;
	if (j<0) j = 0; if (j>dim[1]-1) j = dim[1]-1;
	if (k<0) k = 0; if (k>dim[2]-1) k = dim[2]-1;
	int gi = GridIndex(i,j,k);
	Point3 p = pt[gi]-offsets[gi];
	if (ii!=i || jj!=j || kk!=k) {
		if (TRUE) {
			Point3 pp=p, pp1, pp2;
			if (ii<0) {

					pp  = GetPtOR(i  ,jj,kk);
					pp1 = GetPtOR(i+1,jj,kk);
					if (dim[0]>2) {
						pp2 = GetPtOR(i+2,jj,kk);
						p += Reflect(pp-pp1,pp1-pp2);
					} else {
						p += pp-pp1;
						}
					
				}
			if (ii>dim[0]-1) {
					pp  = GetPtOR(i  ,jj,kk);
					pp1 = GetPtOR(i-1,jj,kk);
					if (dim[0]>2) {
						pp2 = GetPtOR(i-2,jj,kk);
						p += Reflect(pp-pp1,pp1-pp2);
					} else {
						p += pp-pp1;
						}
					
				}

			if (jj<0) {
				pp  = GetPtOR(ii,j  ,kk);
				pp1 = GetPtOR(ii,j+1,kk);
				if (dim[1]>2) {
					pp2 = GetPtOR(ii,j+2,kk);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
			if (jj>dim[1]-1) {
				pp  = GetPtOR(ii,j  ,kk);
				pp1 = GetPtOR(ii,j-1,kk);
				if (dim[1]>2) {
					pp2 = GetPtOR(ii,j-2,kk);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}

			if (kk<0) {
				pp  = GetPtOR(ii,jj,k  );
				pp1 = GetPtOR(ii,jj,k+1);
				if (dim[2]>2) {
					pp2 = GetPtOR(ii,jj,k+2);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
			if (kk>dim[2]-1) {
				pp  = GetPtOR(ii,jj,k  );
				pp1 = GetPtOR(ii,jj,k-1);
				if (dim[2]>2) {
					pp2 = GetPtOR(ii,jj,k-2);
					p += Reflect(pp-pp1,pp1-pp2);
				} else {
					p += pp-pp1;
					}
				}
		} else { 			
			float x = 1.0f/float(dim[0]-1);
			float y = 1.0f/float(dim[1]-1);
			float z = 1.0f/float(dim[2]-1);
			if (ii<0) {
				p.x -= x; 
				}
			if (ii>dim[0]-1) {
				p.x += x;
				}
			if (jj<0) p.y -= y; if (jj>dim[1]-1) p.y += y;
			if (kk<0) p.z -= z; if (kk>dim[2]-1) p.z += z;
			}
		}
	return p;
	}


void GizmoBulgeClass::InitializeFDD(TimeValue t)
	{
	
	
	// Build a cache of all points
	pts = new Point3**[dim[0]+2];
	for (int i=0; i<dim[0]+2; i++) {
		pts[i] = new Point3*[dim[1]+2];
		for (int j=0; j<dim[1]+2; j++) {
			pts[i][j] = new Point3[dim[2]+2];
			for (int k=0; k<dim[2]+2; k++) {				
				pts[i][j][k] = GetPtOR(i-1,j-1,k-1);
				}
			}
		}


	// Compute the TCB multipliers	
	float tens = 25.0f;
	float cont = 25.0f;
	tens = (tens-25.0f)/25.0f;
	cont = (cont-25.0f)/25.0f;
	ComputeTCBMults(tens,cont,m1,m2);

	Matrix3 twist(1);
	float twistAngle;

	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_twist,0,twistAngle,FOREVER);
	twistAngle = twistAngle * PI/180.f;

	twist.RotateZ(twistAngle);


	int id;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_parent_id,0,id,FOREVER);
	gizmoTmToLocal  = Inverse(bonesMod->GetBoneTm(id));
	gizmoTmToLocal =  twist*gizmoTm*gizmoTmToLocal;
	gizmoTmToLocal = Inverse(gizmoTmToLocal);
	 
	}

inline float BPoly4(int i, float u)
	{
	float s = 1.0f-u;
	switch (i) {
		case 0: return s*s*s;
		case 1: return 3.0f*u*s*s;
		case 2: return 3.0f*u*u*s;
		case 3: return u*u*u;
		default: return 0.0f;
		}
	}


Point3 GizmoBulgeClass::Map(int ii, Point3 p)
	{

	Point3 q(0,0,0), pp;
	
	pp =  p;

	// Compute distance for falloff
//if outside volume return the initial point
	if (pp.x<-0.001f || pp.x>1.001f ||
		pp.y<-0.001f || pp.y>1.001f ||
		pp.z<-0.001f || pp.z>1.001f) {
		return p;
		}
			
	// Find the cell we're in	
	pp.x = pp.x*float(dim[0]-1);
	pp.y = pp.y*float(dim[1]-1);
	pp.z = pp.z*float(dim[2]-1);
	int i = int(pp.x);
	int j = int(pp.y);
	int k = int(pp.z);
	int io=i, jo=j, ko=k;
	if (i<0) i = 0; if (i>dim[0]-2) i = dim[0]-2;
	if (j<0) j = 0; if (j>dim[1]-2) j = dim[1]-2;
	if (k<0) k = 0; if (k>dim[2]-2) k = dim[2]-2;	

	// Make pp relative to our cell
	pp.x -= float(i);
	pp.y -= float(j);
	pp.z -= float(k);

	// We are going to consider all surrounding cells. Make i,j,k
	// refer to the corner of a 3x3x3 cell array with the current cell at the center.
	i -= 1; j -= 1; k -= 1;

	
	// Interpolate the 64 control points using Z to get 16 points
	Point3 pt[4][4];
	for (int ix=0; ix<4; ix++) {
		for (int jx=0; jx<4; jx++) {
			Point3 knots[4];
			for (int kx=0; kx<4; kx++) { 				
				int ii;
				ii = i+ix+1;
				knots[kx] = pts[ii][j+jx+1][k+kx+1];					
				}
			pt[ix][jx] = InterpSpline(pp.z,knots,m1,m2);
			}
		}

	// Now interpolate 16 points to get a single 4 point spline
	Point3 knots[4];
	for (ix=0; ix<4; ix++) {
		knots[ix] = InterpSpline(pp.y,pt[ix],m1,m2);
		}

	// Finally we get the point
	q = InterpSpline(pp.x,knots,m1,m2);
/*
	if (falloff>0.0f && dist>0.0f) {
		float u=dist/falloff;
		u = (u*u*(3-2*u));
		q = u*p + (1.0f-u)*q;
		}	 
*/
	return q;
	}

void GizmoBulgeClass::InsertInCurve(ICurve *cv,float t,float val)
{
//loop through points finding insert point
BOOL hit = FALSE;
int where = 0;
CurvePoint p;
for (int i = 0; i < cv->GetNumPts(); i++)
	{
	p = cv->GetPoint(0, i);
	//get point if equal dump and modify that point
	if (fabs(t - p.p.x) < 0.0001f)
		{
		hit = TRUE;
		where = i;
		i = cv->GetNumPts();
		}
	//else look for when the t is greater than ang
	else if (t < p.p.x)
		{
		where = i;
		i = cv->GetNumPts();
		}
	}
if (hit)
	{
	p.p.y = val;
	cv->SetPoint(0,where, &p);
	}
else
	{
	p.p.x = t;
	p.p.y = val;
	p.flags = CURVEP_CORNER|CURVEP_NO_X_CONSTRAINT;
	cv->Insert(where, p,TestAFlag(A_HELD));
	}
}

void GizmoBulgeClass::KeyGraph(int which, Point3 p)
{

//get the values
//get the curve ref
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	
	graph = (ICurveCtl *) ref;
	float vals[1];

	vals[0] = p.x;
//get value based on angle
	ICurve *cv = graph->GetControlCurve(which*3);
	InsertInCurve(cv,currentBendAngle,vals[0]);

	vals[0] = p.y;
//get value based on angle
	cv = graph->GetControlCurve(which*3+1);
	InsertInCurve(cv,currentBendAngle,vals[0]);

	vals[0] = p.z;
//get value based on angle
	cv = graph->GetControlCurve(which*3+2);
	InsertInCurve(cv,currentBendAngle,vals[0]);


	Interval iv;
	graph->Update(0,iv);
	graph->Redraw();
	}

//loop through points and set keys

}

void GizmoBulgeClass::KeyGraph(int which, float where, Point3 p)
{

//get the values
//get the curve ref
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	
	graph = (ICurveCtl *) ref;
	float vals[1];

	vals[0] = p.x;
//get value based on angle
	ICurve *cv = graph->GetControlCurve(which*3);
	InsertInCurve(cv,where,vals[0]);

	vals[0] = p.y;
//get value based on angle
	cv = graph->GetControlCurve(which*3+1);
	InsertInCurve(cv,where,vals[0]);

	vals[0] = p.z;
//get value based on angle
	cv = graph->GetControlCurve(which*3+2);
	InsertInCurve(cv,where,vals[0]);



	}

//loop through points and set keys

}




IOResult GizmoBulgeClass::Save(ISave *isave)
	{



	isave->BeginChunk(GIZMOTM_CHUNK);
	gizmoTm.Save(isave);
	isave->EndChunk();

//NEW FIX
	if (useNewRotationPlane)
		{
		isave->BeginChunk(USENEWROTATION_CHUNK);
		isave->EndChunk();
		}

	return IO_OK;
	}

class BulgeGizmoPostLoad : public PostLoadCallback {
	public:
		GizmoBulgeClass *n;
		BulgeGizmoPostLoad(GizmoBulgeClass *ns) {n = ns;}
		void proc(ILoad *iload) {  
			ICurveCtl *graph;
			ReferenceTarget *ref;
			n->pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);

			n->pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_editing,0,FALSE);			
			
			if (ref)
				{
				graph = (ICurveCtl *) ref;
				if (graph->GetNumCurves() != 60)
					{
					graph->SetNumCurves(60);
					for (int i=0; i < 20; i++)
						{
						ICurve *curve = graph->GetControlCurve(i*3);
						curve->SetCanBeAnimated(FALSE);
						AColor acol;
						acol.r = 1.0f;
						acol.g = 0.0f;
						acol.b = 0.0f;
						COLORREF col = acol.toRGB();
						curve->SetPenProperty(col);

						curve = graph->GetControlCurve(i*3+1);
						curve->SetCanBeAnimated(FALSE);
		
						acol.r = 0.0f;
						acol.g = 1.0f;
						acol.b = 0.0f;
						col = acol.toRGB();
						curve->SetPenProperty(col);


						curve = graph->GetControlCurve(i*3+2);
						curve->SetCanBeAnimated(FALSE);
		
						acol.r = 0.0f;
						acol.g = 0.0f;
						acol.b = 1.0f;
						col = acol.toRGB();
						curve->SetPenProperty(col);
						}
	
					}
				}

			delete this; 


			} 
	};


IOResult GizmoBulgeClass::Load(ILoad *iload)
	{

	
	IOResult res = IO_OK;
	
//NEW FIX
	useNewRotationPlane = FALSE;


	while (IO_OK==(res=iload->OpenChunk())) {
		int id = iload->CurChunkID();
		switch(id)  {

			case GIZMOTM_CHUNK: 
				{
				gizmoTm.Load(iload);
				break;
				}
			case USENEWROTATION_CHUNK: 
				{
				useNewRotationPlane = TRUE;
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	iload->RegisterPostLoadCallback(new BulgeGizmoPostLoad(this));

	return IO_OK;
	}


static HIMAGELIST hToolsImages = NULL;
void GizmoBulgeClass::LoadResources()
	{

	if (!hToolsImages) {
		HBITMAP hTBitmap, hTMask;	
		hToolsImages = ImageList_Create(16, 15, TRUE, 16, 0);
		hTBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP2));
		hTMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GRAPHBITMAPMASK));
		ImageList_Add(hToolsImages,hTBitmap,hTMask);
		DeleteObject(hTBitmap);
		DeleteObject(hTMask);
		}

	}

void *GizmoBulgeClass::GetInterface(ULONG id)
{
	if(id == I_RESMAKER_INTERFACE)
		return (void *) (ResourceMakerCallback *) this;
	else if(id == I_GIZMO)
		return (GizmoClass *) this;
	else if(id == I_GIZMO3)
		return (IGizmoClass3 *) this;
	else
		return (void *) NULL;
}


BOOL GizmoBulgeClass::GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl)
{
	switch(iButton)
	{
		case 0:ToolTip = _T("Bulge");break;
		default:
			ToolTip = _T(" ");break;
	}
	return TRUE;
}
BOOL GizmoBulgeClass::SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl)
{

//	LoadResources();
//	hCTools = hToolsImages;
	return FALSE;
}

Interval GizmoBulgeClass::LocalValidity(TimeValue t)
{
	ICurveCtl *graph;
	ReferenceTarget *ref;
	pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
	if (ref)
		{
		graph = (ICurveCtl *) ref;
		return graph->GetValidity(t);
		}
	return FOREVER;

}

RefResult GizmoBulgeClass::NotifyRefChanged(
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{
	
	if (stopProp) return REF_STOP;

	switch (message) {
		case REFMSG_CHANGE:
			if ( (hTarget == pblock_gizmo_data))
				{
				ParamID changing_param = pblock_gizmo_data->LastNotifyParamID();
				skin_gizmobulgeparam_blk.InvalidateUI(changing_param);
				if ( (changing_param==skin_gizmoparam_bulge_keygraph) && (!stopReferencing) )
					{
					NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					}				

				}
			break;
		}
	
	return REF_SUCCEED;
	}

IGizmoBuffer *GizmoBulgeClass::CopyToBuffer()
{

//new a class specific version
GizmoBulgeBuffer *buffer = new GizmoBulgeBuffer();
buffer->cid = ClassID();
//fillout buffer now
//pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_amount,0,buffer->bulge,FOREVER);

//pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_use_graph,0,buffer->useGraph,FOREVER);

//now get curve data
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;
//get points
	for (int i =0; i < 60; i++)
		{
		ICurve *cv = graph->GetControlCurve(i);
		buffer->counts[i] = cv->GetNumPts();
		for (int j = 0; j < cv->GetNumPts(); j++)
			{
			CurvePoint cp = cv->GetPoint(0, j);
			buffer->bulge_graph.Append(1,&cp);
			}
		}


	}

return (IGizmoBuffer *) buffer;

}

void GizmoBulgeClass::PasteFromBuffer(IGizmoBuffer *buf)
{
//copy buffer to param block and curve graph
GizmoBulgeBuffer *buffer = (GizmoBulgeBuffer *) buf;
//set pblocks
//fillout buffer now
//pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_amount,0,buffer->bulge);

//pblock_gizmo_data->SetValue(skin_gizmoparam_bulge_use_graph,0,buffer->useGraph);

//set graphs
//now get curve data
stopProp = TRUE;

ICurveCtl *graph=NULL;
ReferenceTarget *ref =NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_bulge_keygraph,0,ref, FOREVER);
if (ref)
	graph = (ICurveCtl *) ref;

if (graph)
	graph->EnableDraw(FALSE);

if (ref)
	{
	graph = (ICurveCtl *) ref;
	int ct = 0;
	for (int i =0; i < 60; i++)
		{

		ICurve *cv = graph->GetControlCurve(i);
		for (int j = 0; j < cv->GetNumPts(); j++)
			cv->Delete(0);
		cv->SetNumPts(buffer->counts[i]);
		for (j = 0; j < buffer->counts[i]; j++)
			{
			CurvePoint cp = buffer->bulge_graph[ct];
			ct++;
			cv->SetPoint(0, j, &cp,FALSE);
			}
		}


	}
stopProp = FALSE;
if (graph)
	{
	graph->EnableDraw(TRUE);
	Interval iv;
	graph->Update(0,iv);
	graph->Redraw();
	}
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

}


