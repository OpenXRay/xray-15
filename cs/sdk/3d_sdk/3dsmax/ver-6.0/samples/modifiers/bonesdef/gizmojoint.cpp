#include "mods.h"
#include "bonesdef.h"


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

Point3 GizmoJointClass::InterpSpline2(float u,Point3 *knot)
	{
	Point3 knotsPts[16];

	Point3 vec = (knot[1]-knot[0]) /3.f;

	knotsPts[0] = knot[0] - vec;
	knotsPts[1] = knot[0];
	knotsPts[2] = knot[0] + vec;

	knotsPts[3] = knot[1] - vec;
	knotsPts[4] = knot[1];
	knotsPts[5] = knot[1] + vec;;

	knotsPts[6] = knot[2];
	knotsPts[7] = knot[2];
	knotsPts[8] = knot[2];

	vec = (knot[4]-knot[3]) /3.f;
	knotsPts[9] = knot[3] - vec;
	knotsPts[10] = knot[3];
	knotsPts[11] = knot[3] + vec;

	knotsPts[12] = knot[4] - vec;
	knotsPts[13] = knot[4];
	knotsPts[14] = knot[4] + vec;

	int knotCount = 5;


	Point3 dm,dp;
	Point3 m,m0;
	float ap,am;
	
	int i = 7;
	dm = knotsPts[7] - knotsPts[4];
	dp = knotsPts[10] - knotsPts[7];
	ap =  0.25f / (0.25f + 0.25f);
	am =  (float)1.0-ap;
	m = ( (am / 0.25f)*dm + (ap / 0.25f)*dp )/(float)3.0;
	knotsPts[6] = knotsPts[7] - m * 0.25f;
	knotsPts[8] = knotsPts[7] + m * 0.25f;


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

	float t= tu;
	int where = segment;
	if(t < 0.0f)
		t = 0.0f;
	else
	if(t > 1.0f)
		t = 1.0f;
	int where2 = (where+1) % knotCount;
	float s = (float)1.0-t;
	float t2 = t*t;

	where = where * 3 + 1;
	where2 = where2 * 3 + 1;
	
	
	Point3 p =  ( ( s* knotsPts[where] + (3.0f*t)*knotsPts[where+1])*s 
					 + (3.0f*t2)* knotsPts[where2-1])*s + t*t2*knotsPts[where2];
	
	return p;
	}


IOResult LocalGizmoData::Save(ISave *isave)
{
unsigned long nb;
int c = affectedVerts.Count();
IOResult ior;
ior = isave->Write(&c,sizeof(c),&nb);
for (int i = 0; i < affectedVerts.Count(); i++)
	{
	c = affectedVerts[i];
	ior = isave->Write(&c,sizeof(c),&nb);
	}
return ior;
}




IOResult LocalGizmoData::Load(ILoad *iload)
{
unsigned long nb;
int c;
IOResult ior;
ior = iload->Read(&c,sizeof(c),&nb);
affectedVerts.SetCount(c);
for (int i = 0; i < affectedVerts.Count(); i++)
	{
	int id;
	ior = iload->Read(&id,sizeof(id),&nb);
	affectedVerts[i] = id;
	}


return ior ;
}


BOOL LocalGizmoData::IsAffected(int index)
{
for (int i =0; i < affectedVerts.Count(); i++)
	{
	if (index == affectedVerts[i])
		return TRUE;
	}
return FALSE;
}


BOOL LocalGizmoData::IsInList(int index, int &where)
{
for (int i =0; i < affectedVerts.Count(); i++)
	{
	if (index == affectedVerts[i])
		{
		where = i;
		return TRUE;
		}
	}
return FALSE;
}
void LocalGizmoData::SetVert(int index)
{
int where;
if (!IsInList(index,where))
	affectedVerts.Append(1,&index);
}

void LocalGizmoData::BuildDeformingList()
{
	start = -1;
	int end = -1;
	for (int i = 0; i < affectedVerts.Count(); i++)
		{
		if ((start == -1) || (affectedVerts[i] < start))
			start = affectedVerts[i];
		if ((end == -1) || (affectedVerts[i] > end))
			end = affectedVerts[i];
		}
	if ((start == -1) && (end == -1))
		deformingVerts.SetSize(0);
	else
		{
		deformingVerts.SetSize(end - start+1);
		deformingVerts.ClearAll();
		for (i = 0; i < affectedVerts.Count(); i++)
			{
			int index = affectedVerts[i]-start;
			deformingVerts.Set(index);
			}
		}
}
void LocalGizmoData::FreeDeformingList()
{
	deformingVerts.SetSize(0);
}


 
class AddGizmoModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	Tab<BoneModData *> localModList;
	BOOL ev;
	AddGizmoModEnumProc(BonesDefMod *l, BOOL e)
		{
		lm = l;
		ev = e;
		}
private:
	BOOL proc (ModContext *mc);
};

BOOL AddGizmoModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;

	BoneModData *bmd = (BoneModData *) mc->localData;
	localModList.Append(1,&bmd);
	return TRUE;
}


void BonesDefMod::SelectGizmo(int id)
{
if ( (id >=0) && (id<pblock_gizmos->Count(skin_gizmos_list)) && ip)
	{
	if (currentSelectedGizmo >= 0)
		{
		ReferenceTarget *ref;
		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		GizmoClass *gizmo = (GizmoClass *)ref;
		if (gizmo) gizmo->EndEditParams(ip, END_EDIT_REMOVEUI,NULL);
		}
	SendMessage(GetDlgItem(hParamGizmos, IDC_LIST1), LB_SETCURSEL, id, 0L );
	currentSelectedGizmo = id;
	ReferenceTarget *ref;
	ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
	GizmoClass *gizmo = (GizmoClass *)ref;
	if (gizmo) gizmo->BeginEditParams(ip, flags,NULL);
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}
}

void BonesDefMod::SelectGizmoType(int id)
{
if ( (id >=0) && (id<gizmoIDList.Count()))
	{
	SendMessage(GetDlgItem(hParamGizmos, IDC_DEFORMERS), CB_SETCURSEL, id, 0L );
	}
}




void BonesDefMod::AddGizmo()
{
	
AddGizmoModEnumProc lmdproc(this,TRUE);
EnumModContexts(&lmdproc);
//create a new instance of the current selected gizmo
GizmoClass *gizmo;
currentSelectedGizmoType = SendMessage(GetDlgItem(hParamGizmos,IDC_DEFORMERS),
							           CB_GETCURSEL,0,0);

Class_ID cid = gizmoIDList[currentSelectedGizmoType];
gizmo = (GizmoClass *) CreateInstance(REF_TARGET_CLASS_ID,cid);
//add it to the param block
ReferenceTarget *ref = (ReferenceTarget *) gizmo;

gizmo->bonesMod = this;


//get selection set, the current bone and pass to the gizmo to create the initial gizmo data
//append to local mod data
//set the selected vertices
//set which gizmo
Tab<Point3> pointList;
Tab<int> mapList;

for (int i =0; i<lmdproc.localModList.Count(); i++)
	{
	BoneModData *bmd = lmdproc.localModList[i];
	
	for (int j =0; j<bmd->VertexData.Count(); j++)
		{
		if (bmd->selected[j])
			{
			Point3 p = bmd->VertexData[j]->LocalPos * bmd->BaseTM;
			pointList.Append(1,&p);
			mapList.Append(1,&j);
			}	
		}
	if (pointList.Count() >0)
		{
//create a new gizmo instance
		LocalGizmoData *temp;
		temp = new LocalGizmoData();
		temp->whichGizmo = pblock_gizmos->Count(skin_gizmos_list);
		for (int j =0; j<bmd->VertexData.Count(); j++)
			{
			if (bmd->selected[j])
				{
				temp->SetVert(j);
				}
			}
		bmd->gizmoData.Append(1,&temp);
		}
	}
BOOL result = FALSE;

if (pointList.Count() ==0)
	result = gizmo->InitialCreation(pointList.Count(),NULL,lmdproc.localModList.Count(),NULL);
else result = gizmo->InitialCreation(pointList.Count(),(Point3*)pointList.Addr(0),
									 lmdproc.localModList.Count(),(int*)mapList.Addr(0));

if (!result)
	{
	gizmo->DeleteThis();

	for (i =0; i<lmdproc.localModList.Count(); i++)
		{
		BoneModData *bmd = lmdproc.localModList[i];
		int gct = bmd->gizmoData.Count()-1;
		if (gct >=0)
			{
			delete bmd->gizmoData[gct];
			bmd->gizmoData[gct] = NULL;
			bmd->gizmoData.Delete(gct,1);
			}
		}
	
	return ;
	}

theHold.Begin();



theHold.Put(new UpdateUIRestore(this));

pblock_gizmos->Append(skin_gizmos_list,1,&ref);

//if gizmo list was zero before now pop there rollup
theHold.Suspend();

if (pblock_gizmos->Count(skin_gizmos_list) == 1)
	{
	SendMessage(GetDlgItem(hParamGizmos, IDC_LIST1), LB_SETCURSEL, 0L, 0 );
	gizmo->BeginEditParams(ip, flags,NULL);
	currentSelectedGizmo = 0;
	}
else
	{
	if (currentSelectedGizmo >= 0)
		{
		ReferenceTarget *prev_ref;
		prev_ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		GizmoClass *prev_gizmo = (GizmoClass *)prev_ref;
		prev_gizmo->EndEditParams(ip, END_EDIT_REMOVEUI,NULL);
		}
	currentSelectedGizmo = pblock_gizmos->Count(skin_gizmos_list)-1;
	gizmo->BeginEditParams(ip, flags,NULL);
	

	}	

//add name
gizmo->SetInitialName();

//need to update the list box
UpdateGizmoList();
SendMessage(GetDlgItem(hParamGizmos, IDC_LIST1), LB_SETCURSEL, currentSelectedGizmo, 0 );
NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);

theHold.Resume();




for (i =0; i<lmdproc.localModList.Count(); i++)
	{
	BoneModData *bmd = lmdproc.localModList[i];

	theHold.Put(new AddGizmoLocalDataRestore(this,bmd));

	}
theHold.Put(new AddGizmoRestore(this,gizmo));

theHold.Accept(GetString(IDS_PW_ADDGIZMO));


}

void BonesDefMod::RemoveGizmo()
{
int id = currentSelectedGizmo;
if ( id < 0) return;
AddGizmoModEnumProc lmdproc(this,TRUE);
EnumModContexts(&lmdproc);

theHold.Begin();

for (int i =0; i<lmdproc.localModList.Count(); i++)
	{
	BoneModData *bmd = lmdproc.localModList[i];

	theHold.Put(new RemoveGizmoLocalDataRestore(this,bmd,id));
	}


//create a new instance of the current selected gizmo
GizmoClass *gizmo;
gizmo = NULL;
//add it to the param block
ReferenceTarget *ref = NULL;

ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
gizmo = (GizmoClass *)ref;

theHold.Put(new RemoveGizmoRestore(this,gizmo,id));
theHold.Accept(GetString(IDS_PW_REMOVEGIZMO));


gizmo->EndEditParams(ip, END_EDIT_REMOVEUI,NULL);


pblock_gizmos->Delete(skin_gizmos_list,id,1);




//get selection set, the current bone and pass to the gizmo to create the initial gizmo data
//append to local mod data
//set the selected vertices
//set which gizmo
for (i =0; i<lmdproc.localModList.Count(); i++)
	{
	BoneModData *bmd = lmdproc.localModList[i];
	for (int j = 0; j < bmd->gizmoData.Count(); j++)
		{
		if (bmd->gizmoData[j]->whichGizmo == id)
			{
			delete bmd->gizmoData[j];
			bmd->gizmoData[j] = NULL;
			bmd->gizmoData.Delete(j,1);
			int ct = bmd->gizmoData.Count();
			j--;
			}
		else if (bmd->gizmoData[j]->whichGizmo > id)
			bmd->gizmoData[j]->whichGizmo--;

		}
	}

//add name
//need to update the list box
UpdateGizmoList();

if (currentSelectedGizmo != -1)
	{
	ReferenceTarget *ref;
	ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
	GizmoClass *gizmo = (GizmoClass *)ref;
	if (gizmo) gizmo->BeginEditParams(ip, flags,NULL);
	}

NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);

}


void BonesDefMod::UpdateGizmoList()
{

int sel;

sel = SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);


SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_RESETCONTENT,0,0);

for (int i =0; i < pblock_gizmos->Count(skin_gizmos_list); i++) 
	{
	ReferenceTarget *ref;
	ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
	GizmoClass *gizmo = (GizmoClass *)ref;
	TCHAR *name = gizmo->GetName();
	SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM) name);
	}
int iret = SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_SETCURSEL,sel,0);
if (iret == LB_ERR)
	{
	int sel = pblock_gizmos->Count(skin_gizmos_list)-1;
	int tret = SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_SETCURSEL,sel,0L);
	if (tret != LB_ERR)
		currentSelectedGizmo = sel;
	else currentSelectedGizmo = -1;
	}

}

void BonesDefMod::CopyGizmoBuffer()
{
//get active buffer
int sel;
sel = SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);

if (sel == LB_ERR) return;

//delete buffer if there
if (copyGizmoBuffer) copyGizmoBuffer->DeleteThis();
//get active buffer
ReferenceTarget *ref;
ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,sel);
GizmoClass *gizmo = (GizmoClass *)ref;
//ask for copy data
if (gizmo) copyGizmoBuffer = gizmo->CopyToBuffer();
}


void BonesDefMod::PasteGizmoBuffer()
{
if (!copyGizmoBuffer) return;
//get classid of of selected gizmo
int sel;
sel = SendMessage(GetDlgItem(hParamGizmos,IDC_LIST1),
			LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);
if (sel == LB_ERR) return;

ReferenceTarget *ref;
ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,sel);
GizmoClass *gizmo = (GizmoClass *)ref;

if (gizmo)
	{
//if equal to try paste
	if (copyGizmoBuffer->cid == gizmo->ClassID())
		{
//hold buffer
		theHold.Begin();
		theHold.Put(new GizmoPasteRestore(gizmo));
		gizmo->PasteFromBuffer(copyGizmoBuffer);
//accept
		theHold.Accept(GetString(IDS_PW_PASTE));
		}
	}

}


void GizmoParamsMapDlgProc::FilloutText()
{
TSTR name;
//get parent id
int parent;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,parent, FOREVER);
if (parent < 0) return;
TCHAR *temp = giz->bonesMod->GetBoneName(parent);
name.printf("%s: %s",GetString(IDS_PW_GIZMOPARAM_PARENT),temp);
//set the text of the parent
SetWindowText(GetDlgItem(giz->hGizmoParams,IDC_PARENT_TEXT),
					name.data());
//get child id
int child;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_child_id,0,child, FOREVER);
if (child < 0) return;
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
			giz->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
			if (ref)
				{
				graph = (ICurveCtl *) ref;
				graph->SetCustomParentWnd(GetDlgItem(giz->hGizmoParams, IDC_GRAPH));
				graph->SetMessageSink(giz->hGizmoParams);
				}
			FilloutText();

			
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
			if (!giz->stopReferencing)
				{
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
					giz->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
					if (ref)
						{
						graph = (ICurveCtl *) ref;
						graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWUTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
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





class GizmoJointClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new GizmoJointClass; }
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOSJOINT); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
											   
	Class_ID		ClassID() { return GIZMOJOINT_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_PW_GIZMOCATEGORY);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("gizmoJoint"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle



	};

static GizmoJointClassDesc gizmoJointDesc;
extern ClassDesc* GetGizmoJointDesc() {return &gizmoJointDesc;}

class GizmoJointPBAccessor : public PBAccessor
{ 
public:
	Point3 pt;
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		GizmoJointClass* p = (GizmoJointClass*)owner;

		switch (id)
		{
			case skin_gizmoparam_name:

					if (p->bonesMod)
						p->bonesMod->UpdateGizmoList();
				
				break;

		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		GizmoJointClass* p = (GizmoJointClass*)owner;

	}
};

static GizmoJointPBAccessor gizmoJoint_accessor;


// per instance gizmo block
static ParamBlockDesc2 skin_gizmoparam_blk ( skin_gizmoparam, _T("gizmos"),  0, &gizmoJointDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_BONESDEFGIZMOSJOINT, IDS_PW_GIZMOPARAM, 0, 0, NULL,
	// params
	skin_gizmoparam_name, 	_T("name"),		TYPE_STRING, 	0,  IDS_PW_GIZMOPARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_NAME,
		p_accessor,		&gizmoJoint_accessor,
		end, 
	skin_gizmoparam_joint_twist,  _T("twist"),	TYPE_FLOAT, 	P_RESET_DEFAULT|P_ANIMATABLE, 	IDS_PW_GIZMOPARAM_TWIST, 
		p_default, 		0.f	,
		p_range, 		-360.0f, 360.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARAM9,IDC_PARAM_SPIN9, 1.0f,
		end, 

	skin_gizmoparam_joint_parent_id,  _T("parent"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_PARENT, 
		end, 
	skin_gizmoparam_joint_child_id,  _T("child"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_CHILD, 
		end, 

	skin_gizmoparam_joint_points,  _T("points"),	TYPE_POINT3_TAB,20, 	0, 	IDS_PW_GIZMOPARAM_POINTS, 
		end, 
//	skin_gizmoparam_joint_deformed_points,  _T("deformed_points"),	TYPE_POINT3_TAB,20, 	0, 	IDS_PW_GIZMOPARAM_DEFORMED_POINTS, 
//		end, 
	skin_gizmoparam_joint_weights,  _T("weights"),	TYPE_FLOAT_TAB,20, 	0, 	IDS_PW_GIZMOPARAM_WEIGHTS, 
		end, 
	skin_gizmoparam_joint_initial_angle,  _T("initial_angle"),	TYPE_FLOAT,	0, 	IDS_PW_GIZMOPARAM_INITIALANGLE, 
		p_default, 		180.f	,
		end, 

	skin_gizmoparam_joint_use_volume,  _T("use_volume"),	TYPE_BOOL,	0, 	IDS_PW_GIZMOPARAM_USEVOLUME, 
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_USE_VOLUME,
		end, 

	skin_gizmoparam_joint_enable,  _T("enable"),	TYPE_BOOL,	0, 	IDS_PW_ENABLE, 
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_ENABLE,
		end, 


	skin_gizmoparam_joint_orientation,  _T("orientation"),	TYPE_POINT3,	0, 	IDS_PW_ORIENTATION, 
		p_default, 		Point3(1.0f,0.0f,0.0f),
		end, 


   skin_gizmoparam_joint_selection,  _T("selection"),	TYPE_INT_TAB,20,	0, 	IDS_PW_SELECTION, 
		p_default, 		0,
		end, 


	skin_gizmoparam_joint_editing,  _T("editing"),	TYPE_BOOL,	P_RESET_DEFAULT, 	IDS_PW_GIZMOPARAM_EDITING, 
		p_default, 		FALSE,
		p_ui, 			TYPE_CHECKBUTTON, IDC_EDIT,
		end, 


	skin_gizmoparam_joint_keygraph,  _T("graph"),	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_GIZMOPARAM_GRAPH, 
		end,   



	end
	);
GizmoJointClass::~GizmoJointClass()
{
	DeleteAllRefsFromMe();
}

GizmoJointClass::GizmoJointClass()
{


pblock_gizmo_data = NULL;
GetGizmoJointDesc()->MakeAutoParamBlocks(this);
dim[0] = 2;
dim[1] = 2;
dim[2] = 5;
pts = NULL;
stopReferencing = FALSE;
//NEW FIX
useNewRotationPlane = TRUE;
}
BOOL GizmoJointClass::IsVolumeBased()
{
	if (pblock_gizmo_data)
		{
		BOOL useVol;
		pblock_gizmo_data->GetValue(skin_gizmoparam_joint_use_volume,0,useVol,FOREVER);
		return useVol;
		}
	return FALSE;
}

BOOL GizmoJointClass::IsEnabled()
{
BOOL enabled;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_enable,0,enabled,FOREVER);
return enabled;

}

BOOL GizmoJointClass::IsInVolume(Point3 p, Matrix3 tm)
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

void GizmoJointClass::SetInitialName()
{

if ((pblock_gizmo_data) && (bonesMod))
//if (0)
	{
	int child;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_child_id,0,child,FOREVER);
	TSTR tn;
	tn.printf("%s - %s", GetString(IDS_PW_GIZMOSJOINT),bonesMod->GetBoneFlat(child)->GetName());
	
	pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,tn.data());
	}
else  pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,GetString(IDS_PW_GIZMOSJOINT));
}

TCHAR *GizmoJointClass::GetName()
{
TCHAR *name;
pblock_gizmo_data->GetValue(skin_gizmoparam_name,0,name,FOREVER);
return name;
}
void GizmoJointClass::SetName(TCHAR *name)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,name);

}



void GizmoJointClass::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{

	gizmoJointDesc.BeginEditParams(ip, this, flags, prev);
	skin_gizmoparam_blk.SetUserDlgProc(new GizmoParamsMapDlgProc(this));
	// install a callback for the type in.
	this->ip = ip;

	}

void GizmoJointClass::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{

	ICurveCtl *graph;
	ReferenceTarget *ref;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
	

	if (ref)
		{
		graph = (ICurveCtl *) ref;
		if (graph->IsActive())
			graph->SetActive(FALSE);
		}

	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_editing,0,FALSE);
	
	gizmoJointDesc.EndEditParams(ip, this, flags, next);
	ip = NULL;
	}

BOOL GizmoJointClass::IsEditing()
{
BOOL editing;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_editing,0,editing,FOREVER);
return editing;
}

void GizmoJointClass::EndEditing()
{
BOOL editing=FALSE;
pblock_gizmo_data->SetValue(skin_gizmoparam_joint_editing,0,editing);
}

void GizmoJointClass::EnableEditing(BOOL enable) 
{
ICustButton *iBut = GetICustButton(GetDlgItem(hGizmoParams, IDC_EDIT));
if (iBut)
	{
	iBut->Enable(enable);
	ReleaseICustButton(iBut);
	}

}

void GizmoJointClass::Enable(BOOL enable)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_joint_enable,0,enable);
}


//NEW FIX
void GizmoJointClass::ResetRotationPlane()
{

	Matrix3 parentTm, childTm;
	Point3 l1,l2,l3,l4;
	int parent, child;

	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_child_id,0,child,FOREVER);

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
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_orientation,0,dir);



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
	

	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_initial_angle,0,angle);

}



Matrix3 GizmoJointClass::CreateInitialAngles()
{

	Matrix3 parentTm, childTm;
	Point3 l1,l2,l3,l4;
	int parent, child;

	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_child_id,0,child,FOREVER);

	bonesMod->GetEndPoints(parent, l1, l2);
	bonesMod->GetEndPoints(child, l3, l4);

	Point3 dir;
	Point3 tl3,tl4;
	tl3 = l3 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
	tl4 = l4 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
	Point3 crossProd;
	crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));
	float dotTemp = DotProd(Normalize(l1-l2),Normalize(tl4-tl3));

	float tdot = fabs(1.0f-fabs(dotTemp));

	if (tdot <= 0.00001f)
//	if (fabs(dotTemp) == 1.0f)

		{
		tl3 = l3 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);

		tl4 = l4 * Inverse(bonesMod->GetBoneTm(child)) * bonesMod->GetBoneTm(parent);
		tl4.y += 10.f;
		crossProd = CrossProd(Normalize(l1-l2),Normalize(tl4-tl3));

		}
	dir = Normalize(crossProd);
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_orientation,0,dir);



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

//	if (Length(newDir -dir) < 0.001f)
//		angle = (180-angle)+180;

	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_initial_angle,0,angle);



	l1 = Point3(0.f,0.0f,0.0f) *  Inverse(bonesMod->GetBoneTm(parent));
	l2 = tl2 *  Inverse(bonesMod->GetBoneTm(parent));
	l3 = Point3(0.f,0.0f,0.0f) * Inverse(bonesMod->GetBoneTm(child));


	tdot = fabs(1.0f-fabs(dot));

	if (tdot <= 0.00001f)
//	if (fabs(dot) == 1.0f)
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

BOOL GizmoJointClass::InitialCreation(int count, Point3 *pt, int numbeOfInstances, int *mapTable) 
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
//				childNode = bonesMod->GetBoneFlat(i);
				i = bonesMod->GetNumBonesFlat();
				}
			}
		}

	
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_parent_id,0,parent);
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_child_id,0,child);

	if ((parent <0) || (child<0)) 
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
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_points,0,p[i],i);
	for (i=0; i < 8; i ++)
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_weights,0,1.0f,i);
	for (i=8; i < 12; i ++)
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_weights,0,0.5f,i);
	for (i=12; i < 20; i ++)
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_weights,0,0.0f,i);

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
	graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWUTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
					  CC_DRAWRULER|CC_DRAWGRID|CC_DRAWLTOOLBAR|
					  CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|
					  CC_RCMENU_SCALE|CC_RCMENU_INSERT_CORNER | CC_RCMENU_INSERT_BEZIER|
					CC_RCMENU_DELETE | CC_SHOW_CURRENTXVAL	
					);	

	graph->SetCustomParentWnd(GetDlgItem(hGizmoParams, IDC_GRAPH));
	
	graph->SetMessageSink(hGizmoParams);

	stopReferencing = TRUE;
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_keygraph,0,(ReferenceTarget*)graph);
	float iangle;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_initial_angle,0,iangle,FOREVER);
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

void GizmoJointClass::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{
Point3 p;
Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);
if (id < 0) return;
INode *node = bonesMod->GetBoneFlat(id);
if (!node) return;
toWorld = node->GetObjectTM(t);
toWorld =  gizmoTm*toWorld  ;

for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p,FOREVER,i);
	p = deformedPoints[i];
	p = p * toWorld;
	box += p;
	}
}

//this is just a undo record so I know when we get an undo, and when the moiuse cycle is completed
class MoveRestore : public RestoreObj {
public:	
	Point3 rcp[20];
	Point3 ucp[20];

//	BOOL csel[20];

	GizmoJointClass *mod;

	BOOL update;
	float angle;

	MoveRestore(GizmoJointClass *mod)
		{
		update = FALSE;
		this->mod = mod;
		angle = mod->currentBendAngle;

		ICurveCtl *graph=NULL;
		ReferenceTarget *ref =NULL;
		mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
		if (ref)
			graph = (ICurveCtl *) ref;

		for (int i=0; i < 20; i++)
			{
			int sel = 0;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
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
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
			if (ref)
				graph = (ICurveCtl *) ref;

			for (int i=0; i < 20; i++)
				{
//				if (csel[i])
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
						rcp[i] = p;
						}
					}
				}
			}

			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
			ICurveCtl *graph=NULL;
			ReferenceTarget *ref =NULL;
			mod->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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


void GizmoJointClass::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin)

{
ICurveCtl *graph=NULL;
ReferenceTarget *ref =NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
if (ref)
	graph = (ICurveCtl *) ref;

stopReferencing = TRUE;
if (graph)
	graph->EnableDraw(FALSE);

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);

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
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);

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


void GizmoJointClass::GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);

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
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	if (sel)
		selCount++;
	}


for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	if ( (sel) || (selCount == 0) )
		{
		cp += p[i] * toWorld;
		ct++;
		}
	}
cp = cp /(float) ct;

cb->Center(cp,0);

}

void GizmoJointClass::GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];


for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);

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
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	if (sel)
		selCount++;
	}

for (i=0; i < 20; i++)
	{

	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	if ( (sel) || (selCount == 0) )
		{
		cp += p[i] * toWorld;
		ct++;
		}
	}
cp = cp /(float) ct;

toWorld.SetTrans (cp);

cb->TM(toWorld,0);

}



void GizmoJointClass::ClearSelection(int selLevel)
{

for (int j = 0; j < 20; j++)
	{
	BOOL sel = FALSE;
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}
void GizmoJointClass::SelectAll(int selLevel)
{
for (int j = 0; j < 20; j++)
	{
	BOOL sel = TRUE;
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}

void GizmoJointClass::InvertSelection(int selLevel)
{
for (int j = 0; j < 20; j++)
	{
	BOOL sel = TRUE;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,j);		
	sel = !sel;
	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,j);		
	}
NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}


void GizmoJointClass::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)

{


BOOL add = GetKeyState(VK_CONTROL)<0;
BOOL sub = GetKeyState(VK_MENU)<0;


if (!add && !sub) 
	{
	for (int j = 0; j < 20; j++)
		{
		BOOL sel = FALSE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,j);		
		}
	}

while (hitRec) 
	{
	int state = hitRec->hitInfo;
	if (sub)
		{
		BOOL sel = FALSE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,state);		

		}
	else 
		{
		BOOL sel = TRUE;
		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_selection,0,sel,state);		
			
		}
	hitRec = hitRec->Next();
	}

BitArray ba;
ba.SetSize(60);
ba.ClearAll();
for (int i =0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);		
	if (sel)
		{
		ba.Set(i*3);
		ba.Set(i*3+1);
		ba.Set(i*3+2);
		}
	}

ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
if (ref)
	{
	graph = (ICurveCtl *) ref;
	graph->SetDisplayMode(ba);		
	}
}

int GizmoJointClass::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *ipt, ViewExp *vpt, ModContext* mc, Matrix3 tm)

{

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[21];
//Point3 pdef[21];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);

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
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	
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



int GizmoJointClass::Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm) 
{

int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);
if (id <0) return 1;


Matrix3 twist(1);

float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Point3 p[20];
for (int i=0; i < 20; i++)
	{
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p[i],FOREVER,i);
	p[i] = deformedPoints[i];
	}

Matrix3 toWorld;
INode *node = bonesMod->GetBoneFlat(id);
if (!node) return 1;
toWorld = node->GetObjectTM(t);
toWorld =  twist*gizmoTm*toWorld  ;

//NEW FIX
Point3 dir;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_orientation,0,dir,FOREVER);

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

/*
dlp[1] = rotationPlane * 40.0f* node->GetObjectTM(t)*tm;// dlp[0] + dir*50.0f;
gw->polyline(2, dlp, NULL, NULL, 1,NULL);
gw->marker(&dlp[1],HOLLOW_BOX_MRKR);
*/

for (i=0; i < 20; i++)
	{
	p[i] = p[i] * toWorld * tm;


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

Point3 m[3];
Point3 zero(0.0f,0.0f,0.0f);
Point3 x1(10.0f,0.0f,0.0f);
Point3 y1(0.0f,10.0f,0.0f);
Point3 z1(0.0f,0.0f,10.0f);



for (i=0; i < 20; i++)
	{
	int sel;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);


	if (sel)
		gw->marker(&p[i],HOLLOW_BOX_MRKR);
	else gw->marker(&p[i],PLUS_SIGN_MRKR);

	}


Point3 subSelColor = GetUIColor(COLOR_SUBSELECTION);
float r = subSelColor.x;
float g = subSelColor.y;
float b = subSelColor.z;

gw->setColor(LINE_COLOR, r,g,b);

BOOL hit[20];

//now get curve data
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_selection,0,sel,FOREVER,i);
	if (hit[i])
		{
		if (sel)
			gw->marker(&p[i],HOLLOW_BOX_MRKR);
		else gw->marker(&p[i],PLUS_SIGN_MRKR);
		}
	}




return 1;
}


void GizmoJointClass::PostDeformSetup(TimeValue t)
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
void GizmoJointClass::PreDeformSetup(TimeValue t)
{
BOOL holdStop = stopReferencing;
stopReferencing = TRUE;
//bend the lattice
//points based on weights in local space
//child tm
int parentId, childId;
Interval iv;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,parentId,FOREVER);
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_child_id,0,childId,FOREVER);

if ((parentId <0) || (childId<0)) 
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


Point3 initialPos[20];
Point3 defPos[20];

Matrix3 twist(1);
float twistAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
twistAngle = twistAngle * PI/180.f;

twist.RotateZ(twistAngle);

Matrix3 toWorld;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);
toWorld = bonesMod->GetBoneFlat(id)->GetObjectTM(t);
toWorld =  gizmoTm*toWorld  ;


Point3 p[21];
//Point3 pdef[21];

for (int i = 0; i < 20; i++)
	{
	Point3 p;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_points,0,p,FOREVER,i);

	defPos[i]  = p;
//	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_deformed_points,0,p,i);
	deformedPoints[i] = p;
	}
float currentAngle, initialAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_joint_initial_angle,0,initialAngle,FOREVER);

Point3 l1,l2,l3,l4;
bonesMod->GetEndPoints(parentId, l1, l2);
bonesMod->GetEndPoints(childId, l3, l4);

Point3 tl3,tl4,dir;
tl3 = l3 * childTm * Inverse(parentTm);
tl4 = l4 * childTm * Inverse(parentTm);
Point3 newDir = Normalize(CrossProd(Normalize(l1-l2),Normalize(tl4-tl3)));
//NEW FIX
rotationPlane = newDir;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_orientation,0,dir,FOREVER);


//compute the initial angle between the joints
Point3 tl2 = l2;
l1 = l1 * parentTm;
l2 = l2 * parentTm;
l3 = l3 * childTm;
l4 = l4 * childTm;
float dot = DotProd(Normalize(l1-l2),Normalize(l4-l3));
//float angle;
if (fabs(dot) == 1.0f) 
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
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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




graph = NULL;
ref = NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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



Matrix3 trans = twist*gizmoTm*ParentTm0*inverseChildTmRefFrame*childTm* inverseParentTm*Inverse(gizmoTm)*Inverse(twist);
for ( i = 0; i < 20; i++)
	{
	Point3 p;
	float weight;
	p = defPos[i];
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p,FOREVER,i);
	p = deformedPoints[i];

//	initialPos[i] = p;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_weights,0,weight,FOREVER,i);
	if (weight == 1.0f)
		{
//		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_deformed_points,0,p,i);
		deformedPoints[i] = p;
		}
	else
		{
		Point3 originalPt = p;
		weight = 1.0f - weight;
//now apply the child bones tm to the point
//in world spacebased of parent animation
//new opt
		p = p *trans;

		Point3 vec = p - originalPt;
		vec = vec * weight;
		p = originalPt + vec;

//		pblock_gizmo_data->SetValue(skin_gizmoparam_joint_deformed_points,0,p,i);
		deformedPoints[i] = p;
		}

	}



for (i =0; i < 20; i++)
	{
	Point3 p;
//	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_deformed_points,0,p,FOREVER,i);
	p = deformedPoints[i];
	p += latticeOffsets[i];

//	pblock_gizmo_data->SetValue(skin_gizmoparam_joint_deformed_points,0,p,i);
	deformedPoints[i] = p;


	pt[i] = p;
	offsets[i] = Point3(0.0f,0.0f,0.0f);
	}

InitializeFDD(t);
stopReferencing = holdStop;
}



Point3 GizmoJointClass::DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)

{

if (!val) return p;
//tranform the point into world space

Point3 pWorld;// = p * tm;
Point3 iP = initialP * tm;
// Transform into lattice space
Point3 pp = iP*gizmoTmToLocal;

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



pWorld = InterpSpline2(pp.z,sp);


 

//old opt pWorld = pWorld*Inverse(gizmoTmToLocal);

// initialP;
//then add parent motion
//old opt p = pWorld *Inverse(gizmoTmToLocal)* inverseParentTm0* parentTmT * Inverse(tm);
p = pWorld * cacheTm * Inverse(tm);

//old opt p = pWorld * Inverse(tm);



return p;
}


int GizmoJointClass::GridIndex(int i, int j, int k)
	{
	int ix = k*dim[0]*dim[1] + j*dim[0] + i;
	assert(ix>=0 && ix<20);
	return ix;
	}


Point3 GizmoJointClass::GetPtOR(int i, int j, int k)
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


void GizmoJointClass::InitializeFDD(TimeValue t)
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

	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_twist,0,twistAngle,FOREVER);
	twistAngle = twistAngle * PI/180.f;

	twist.RotateZ(twistAngle);


	int id;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_parent_id,0,id,FOREVER);
	gizmoTmToLocal  = Inverse(bonesMod->GetBoneTm(id));
//	TimeValue rt = bonesMod->GetRefFrame();
//	gizmoTmToLocal  = bonesMod->GetBoneFlat(id)->GetNodeTM(rt);
	gizmoTmToLocal =  twist*gizmoTm*gizmoTmToLocal;
	gizmoTmToLocal = Inverse(gizmoTmToLocal);
	 
	cacheTm = Inverse(gizmoTmToLocal)* inverseParentTm0* parentTmT;

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


Point3 GizmoJointClass::Map(int ii, Point3 p)
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

	return q;
	}

void GizmoJointClass::InsertInCurve(ICurve *cv,float t,float val)
{
//loop through points finding insert point
BOOL hit = FALSE;
int where = 0;
CurvePoint p;
for (int i = 0; i < cv->GetNumPts(); i++)
	{
	p = cv->GetPoint(0, i);
	//get point if equal dump and modify that point
	if (fabs(t - p.p.x) < 0.0001f)		{
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


void GizmoJointClass::KeyGraph(int which, Point3 p)
{

//get the values
//get the curve ref
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
//	graph->Update(0,iv);
//	graph->Redraw();
	}

//loop through points and set keys

}

void GizmoJointClass::KeyGraph(int which, float where, Point3 p)
{

//get the values
//get the curve ref
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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




IOResult GizmoJointClass::Save(ISave *isave)
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

class JointGizmoPostLoad : public PostLoadCallback {
	public:
		GizmoJointClass *n;
		JointGizmoPostLoad(GizmoJointClass *ns) {n = ns;}
		void proc(ILoad *iload) {  
			ICurveCtl *graph;
			ReferenceTarget *ref;
			n->pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
			
			n->pblock_gizmo_data->SetValue(skin_gizmoparam_joint_editing,0,FALSE);
			
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


IOResult GizmoJointClass::Load(ILoad *iload)
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

	iload->RegisterPostLoadCallback(new JointGizmoPostLoad(this));

	return IO_OK;
	}


static HIMAGELIST hToolsImages = NULL;
void GizmoJointClass::LoadResources()
	{

	if (!hToolsImages) {
		HBITMAP hTBitmap, hTMask;	
		hToolsImages = ImageList_Create(16, 15, TRUE, 16, 0);
		hTBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GRAPHBITMAP));
		hTMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GRAPHBITMAPMASK));
		ImageList_Add(hToolsImages,hTBitmap,hTMask);
		DeleteObject(hTBitmap);
		DeleteObject(hTMask);
		}

	}

void *GizmoJointClass::GetInterface(ULONG id)
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


BOOL GizmoJointClass::GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl)
{
	switch(iButton)
	{
		case 0:ToolTip = _T("Red Curve On/Off Toggle");break;
		case 1:ToolTip = _T("Green Curve On/Off Toggle");break;
		case 2:ToolTip = _T("Blue Curve On/Off Toggle");break;
		default:
			ToolTip = _T("Visibility On/Off Toggle");break;
	}
	return TRUE;
}
BOOL GizmoJointClass::SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl)
{

	LoadResources();
	hCTools = hToolsImages;
	return TRUE;
}

Interval GizmoJointClass::LocalValidity(TimeValue t)
{
	ICurveCtl *graph;
	ReferenceTarget *ref;
	pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
	if (ref)
		{
		graph = (ICurveCtl *) ref;
		return graph->GetValidity(t);
		}
	return FOREVER;

}

RefResult GizmoJointClass::NotifyRefChanged(
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{


	switch (message) {
		case REFMSG_CHANGE:
			if ( (hTarget == pblock_gizmo_data))
				{
				ParamID changing_param = pblock_gizmo_data->LastNotifyParamID();
				skin_gizmoparam_blk.InvalidateUI(changing_param);
				if ((changing_param==skin_gizmoparam_joint_keygraph) && (!stopReferencing))
					{
					NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					}				
				}
			break;
		}
	if (stopReferencing)
		return REF_STOP;
	else return REF_SUCCEED;
	}

IGizmoBuffer *GizmoJointClass::CopyToBuffer()
{

//new a class specific version
GizmoJointBuffer *buffer = new GizmoJointBuffer();
buffer->cid = ClassID();
//fillout buffer now

//now get curve data
ICurveCtl *graph;
ReferenceTarget *ref;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
			buffer->joint_graph.Append(1,&cp);
			}
		}


	}


return (IGizmoBuffer *) buffer;

}

void GizmoJointClass::PasteFromBuffer(IGizmoBuffer *buf)
{
//copy buffer to param block and curve graph
GizmoJointBuffer *buffer = (GizmoJointBuffer *) buf;
//set pblocks
//fillout buffer now
stopReferencing = TRUE;


ICurveCtl *graph=NULL;
ReferenceTarget *ref =NULL;
pblock_gizmo_data->GetValue(skin_gizmoparam_joint_keygraph,0,ref, FOREVER);
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
			CurvePoint cp = buffer->joint_graph[ct];
			ct++;
			cv->SetPoint(0, j, &cp,FALSE);
			}
		}


	}
stopReferencing = FALSE;
if (graph)
	{
	graph->EnableDraw(TRUE);
	Interval iv;
	graph->Update(0,iv);
	graph->Redraw();
	}



NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
}


