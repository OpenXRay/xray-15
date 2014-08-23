#ifndef __BONESDEF__H
#define __BONESDEF__H

#include "mods.h"
#include "iparamm2.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "ISkin.h"
#include "icurvctl.h"

// This uses the linked-list class templates
#include "linklist.h"

#define GIZMOBULGE_CLASSID Class_ID(9815854,999633)

#define GIZMOTM_CHUNK			0x260
#define USENEWROTATION_CHUNK	0x270


enum { skin_gizmobulgeparam};

enum { skin_gizmoparam_name,
	   skin_gizmoparam_bulge_amount, //nolonger used
	   skin_gizmoparam_bulge_twist,

	   skin_gizmoparam_bulge_parent_id,
	   skin_gizmoparam_bulge_child_id,
	   skin_gizmoparam_bulge_graph,  //nolonger used

   	   skin_gizmoparam_bulge_points,
   	   skin_gizmoparam_bulge_deformed_points, //no longer used all deformed points are stored in Point3 deformedPoints[21];
   	   skin_gizmoparam_bulge_initial_angle,
 	   skin_gizmoparam_bulge_use_graph,  //nolonger used
	   skin_gizmoparam_bulge_use_volume,
	   skin_gizmoparam_bulge_enable,
	   skin_gizmoparam_bulge_orientation,

	   skin_gizmoparam_bulge_selection,
	   skin_gizmoparam_bulge_editing,
	   skin_gizmoparam_bulge_keygraph,
	}; 


class GizmoBulgeBuffer : public IGizmoBuffer
{
public:
float bulge;
BOOL useGraph;
//graph data;

Tab<CurvePoint> bulge_graph;
int counts[60];
void DeleteThis() { delete this; 	}

};

class GizmoBulgeClass : public GizmoClass, public ResourceMakerCallback, public IGizmoClass3
	{
public:
	BOOL stopReferencing;
	BOOL stopProp;
	IObjParam *ip;


	GizmoBulgeClass();
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOBULGE); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID ClassID() { return GIZMOBULGE_CLASSID;}      
	void SetInitialName();
	TCHAR *GetName();
	void SetName(TCHAR *name);
    void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
    void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
	Interval LocalValidity(TimeValue t);
	void LoadResources();
	void* GetInterface(ULONG id);
	BOOL SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl);
	BOOL GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl);

	BOOL IsEnabled();
	BOOL IsVolumeBased();
	BOOL IsInVolume(Point3 p, Matrix3 tm);

	BOOL IsEditing();
	void Enable(BOOL enable);
	void EndEditing();
	void EnableEditing(BOOL enable);

	
	IGizmoBuffer *CopyToBuffer(); 
	void PasteFromBuffer(IGizmoBuffer *buffer); 
    // From Animatable
    void DeleteThis() { 
					delete this; 
					}
	BOOL val;
	Matrix3 inverseParentTm0,parentTmT;

	Matrix3 CreateInitialAngles();

	BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable);

	Matrix3 twistTm;
	Matrix3 gizmoTm;

	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               
	int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm );

    int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm);
    void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
    void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE );

    void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
    void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);

    void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
    void InvertSelection(int selLevel);



	void PostDeformSetup(TimeValue t);
	void PreDeformSetup(TimeValue t);
	HWND hGizmoParams;
	Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm);

	Point3 pt[20], offsets[20];

	Point3 latticeOffsets[20];

	Point3		***pts;

//ffd stuff	
	int dim[3];
	float m1,m2;
	Matrix3 gizmoTmToLocal;
	Point3 GetPtOR(int i, int j, int k);
	int GridIndex(int i, int j, int k);
	void InitializeFDD(TimeValue t);
	Point3 Map(int ii, Point3 p);
	Point3 InterpSpline2(float u,Point3 *knot);
	void KeyGraph(int which, Point3 p);
	void KeyGraph(int which, float where, Point3 p);
	void InsertInCurve(ICurve *cv,float t,float val);

	float currentBendAngle;

	RefTargetHandle Clone(RemapDir& remap)
		{
		stopProp = TRUE;
		GizmoBulgeClass *mod = new GizmoBulgeClass();
		mod->ReplaceReference(0,pblock_gizmo_data->Clone(remap));
		mod->gizmoTm = gizmoTm;
		stopProp = FALSE;
		return mod;
		}

	Point3 deformedPoints[21];

//NEW FIX
	Point3 rotationPlane;
	BOOL useNewRotationPlane;
	void ResetRotationPlane();

	};

class GizmoParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		GizmoBulgeClass *giz;		
		GizmoParamsMapDlgProc(GizmoBulgeClass *m) {giz = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
		void FilloutText();
	};

//key graph undo record
class KeyGraphRestore : public RestoreObj {

	public:

		ICurveCtl *graph;
		Tab<int> uCount;
		Tab<CurvePoint> uData;

		Tab<int> rCount;
		Tab<CurvePoint> rData;

		KeyGraphRestore(ICurveCtl *g) 
			{
			graph = g;
			uCount.SetCount(1);
			rCount.SetCount(1);
			for (int i = 0; i < 1; i ++)
				{
				ICurve *cv = graph->GetControlCurve(i);
				uCount[i] = cv->GetNumPts();
				CurvePoint cp;
				for (int j = 0; j < cv->GetNumPts(); j++)
					{
					cp = cv->GetPoint(0,j);
					uData.Append(1,&cp);
					}
				}
			}   	
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				
				for (int i = 0; i < 1; i ++)
					{
					ICurve *cv = graph->GetControlCurve(i);
					rCount[i] = cv->GetNumPts();
					CurvePoint cp;
					for (int j = 0; j < cv->GetNumPts(); j++)
						{
						cp = cv->GetPoint(0,j);
						rData.Append(1,&cp);
						}
					}
				
				}
			
//delete all the points in the graph and add the old number back in			
			int id = 0;
			for (int i = 0; i < 1; i ++)
				{
				ICurve *cv = graph->GetControlCurve(i);
				for (int j = 0; j < cv->GetNumPts(); j++)
					cv->Delete(0);

				int ct = uCount[i]; 
				cv->SetNumPts(ct);
				CurvePoint cp;
				for (j = 0; j < ct; j++)
					{
					cp = uData[id++];
					cv->SetPoint(0,j,&cp,FALSE);
					}
				}

			graph->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
//delete all the points in the graph and add the old number back in			
			int id = 0;
			for (int i = 0; i < 1; i ++)
				{
				ICurve *cv = graph->GetControlCurve(i);
				for (int j = 0; j < cv->GetNumPts(); j++)
					cv->Delete(0);

				int ct = rCount[i]; 
				cv->SetNumPts(ct);
				CurvePoint cp;
				for (j = 0; j < ct; j++)
					{
					cp = rData[id++];
					cv->SetPoint(0,j,&cp,FALSE);
					}
				}
			
			graph->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
//			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }
	};




#endif
