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

#define GIZMOJOINTMORPH_CLASSID Class_ID(1115854,999744)

#define GIZMOTM_CHUNK			0x260
#define GIZMOPOINTCOUNT_CHUNK			0x270
#define GIZMOMORPHCOUNT_CHUNK			0x280
#define GIZMOMORPHMAPS_CHUNK			0x290
#define GIZMOMORPHPOINTS_CHUNK			0x300
#define GIZMOMORPHTMS_CHUNK				0x310
#define GIZMOMORPHINITIALPOINT_CHUNK	0x320


enum { skin_gizmojointmorphparam};

enum { skin_gizmoparam_name,

	   skin_gizmoparam_jointmorph_parent_id,
	   skin_gizmoparam_jointmorph_child_id,

   	   skin_gizmoparam_jointmorph_initial_angle,
	   skin_gizmoparam_jointmorph_enable,


		skin_gizmoparam_jointmorph_name,

	   skin_gizmoparam_jointmorph_count,
	   skin_gizmoparam_jointmorph_angles,
	   skin_gizmoparam_jointmorph_morphpos,
	   skin_gizmoparam_jointmorph_nodes,
	   skin_gizmoparam_jointmorph_names,

	   skin_gizmoparam_jointmorph_maptable,
	   skin_gizmoparam_jointmorph_vecs,
	   skin_gizmoparam_jointmorph_ease
	}; 

class GizmoJointMorphClass;

class GizmoJointMorphBuffer : public IGizmoBuffer
{
public:
//graph data;

GizmoJointMorphClass *mod;


void DeleteThis() { delete this; 	}

};

class GizmoJointMorphClass : public GizmoClass, public ResourceMakerCallback, public IGizmoClass2
	{
public:
	IObjParam *ip;

	Tab<Point3> initialPos, finalPos;
	BOOL noMorphs;

	BOOL addFromStack;
	BOOL addNode;
	Tab<int*> nodeMapList;
	Tab<Point3*> nodePointList;

	Point3 initialPoint;
	Tab<Matrix3> tmList;

//	Point3 *pPointList,*nPointList;
	BOOL dontPropogate;
	int currentSelectedMorph;


	GizmoJointMorphClass();
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOJOINT); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID ClassID() { return GIZMOJOINTMORPH_CLASSID;}      
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

	void Enable(BOOL enable);
	
	IGizmoBuffer *CopyToBuffer(); 
	void PasteFromBuffer(IGizmoBuffer *buffer); 
    // From Animatable
    void DeleteThis() { 
					delete this; 
					}
	BOOL val;
	Matrix3 inverseParentTm0,parentTmT;

	Matrix3 CreateInitialAngles();

	BOOL InitialCreation(int count, Point3 *p, int modCount, int *mapTable) ;


	Matrix3 gizmoTm;
	Matrix3 igizmoTm;
	float currentBendAngle;

	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               
	int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm );
	void PostDeformSetup(TimeValue t);
	void PreDeformSetup(TimeValue t);
	HWND hGizmoParams;
	Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm);

	float GetCurrentAngle(TimeValue t);
	Point3 GetCurrentVec(TimeValue t);

	void AddFromStack();
	Tab<Point3> tempPointList;
	Tab<int> tempMapList;
	Tab<int> tempMapListFromStack;
	int pCount;
	int pKnot, nKnot;
	int numberGizmos;
	float per;
	float overallPer;
	Tab<float> tempDists;
	Tab<float> tempPer;


	void DeleteMorph(int index);
	void AddMorphNode(INode *node);
	void EvalNode(INode *node,int knot, TimeValue t);
	void FilloutListBox();

	Matrix3 GetCurrentTm(TimeValue t);
	Point3 GetEndPoint(TimeValue t);
	void UpdateMorphList();
	void SetMorphName(int id);

	RefTargetHandle Clone(RemapDir& remap)
		{
		GizmoJointMorphClass *buffer = new GizmoJointMorphClass();
		buffer->ReplaceReference(0,pblock_gizmo_data->Clone(remap));
		buffer->gizmoTm = gizmoTm;
		buffer->pCount = pCount;

		int ct = nodeMapList.Count();
		buffer->nodeMapList.SetCount(ct);
		buffer->nodePointList.SetCount(ct);
		buffer->tmList.SetCount(ct);
		for (int i=0; i < ct; i++)
			{
			buffer->nodeMapList[i] = NULL;
			buffer->nodePointList[i] = NULL;
			if (nodeMapList[i])
				{
				int *nMap =  new int[pCount];
				int *cMap = nodeMapList[i];
				for (int j=0; j < pCount; j++)
					nMap[j] = cMap[j];
				buffer->nodeMapList[i] = nMap;
				}
			if (nodePointList[i])
				{
				Point3 *nMap =  new Point3[pCount];
				Point3 *cMap = nodePointList[i];
				for (int j=0; j < pCount; j++)
					nMap[j] = cMap[j];
				buffer->nodePointList[i] = nMap;
				}

			buffer->tmList[i] = tmList[i];
			}

		buffer->initialPoint = initialPoint;

		return (RefTargetHandle) buffer;
		}
//new r5 interfaces
	void SetBackTransFormMatrices(Matrix3 removeDoubleTransform, Matrix3 putbackDoubleTransform);
	Matrix3 removeDoubleTransform, putbackDoubleTransform;

	};

class GizmoParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		GizmoJointMorphClass *giz;		
		GizmoParamsMapDlgProc(GizmoJointMorphClass *m) {giz = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
	
		void DeleteThis() {
				delete this;
				}
		void FilloutText();
//		void FilloutListBox();
	};

class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker)
		{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
	     return 0;              
		}
		  
      INodeTab Nodes;              
	};


//--- CustMod dlg proc ------------------------------

class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		GizmoJointMorphClass *mod;
		PickControlNode() {mod=NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};



#endif
