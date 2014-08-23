/**********************************************************************
 *<
	FILE: BlobMesh.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __BLOBMESH__H
#define __BLOBMESH__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "Simpobj.h"
#include "particle.h"
#include "IBlobMesh.h"

#include "macrorec.h"


enum { blobmesh_params };
enum { blobmesh_baseparams,blobmesh_pfparams };

//TODO: Add enums for various parameters
enum { 
	pb_size,
	pb_tension,
	pb_render,
	pb_viewport,
	pb_relativecoarseness,
	pb_oldmetaballmethod,
	pb_nodelist,
	pb_usesoftsel,
	pb_minsize,

	pb_useallpf,
	pb_pfeventlist,
	pb_offinview

};


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

class MetaParticleFast {
	public:
		int CreateMetas(ParticleSys parts,Mesh *mesh,float threshold,float res,float strength,int many=1);
		int CreatePodMetas(SphereData *data,int num,Mesh *mesh,float threshold,float res,int many=1);
};


class Grid
{

public:
	bool hit;
	Tab<int> index;
};

class UniformGrid
{
public:
	UniformGrid();
	~UniformGrid();
	void InitializeGrid(int size);
	void FreeGrid();
	void LoadPoints(Point3 *p, float *radius, int count);
	void ClosestPoint(Point3 p, float radius, int &i, float &d);
	void ClosestPoints(Point3 *p, float *radius,  int *i, float *d, int count);

	void InRadius(Point3 p, Tab<int> &index);

	int FindCell(float x, float y, int whichGrid, int &ix, int &iy);

	void DrawBounds(GraphicsWindow *gw, Box3 bounds);
	void Display(GraphicsWindow *gw);

	BOOL debug;
	int debugI;
	Point3 debugP;


private:

	float largestRadius;

	void TagCells(Point3 p,float radius, int whichGrid);

	Box3 bounds;
	float fXWidth,fYWidth,fZWidth;


	int width;
	int widthXwidth;
	Tab<Point3> pointBase;
	Tab<float> radiusBase;

	Tab<Grid*> xGrid;
	Tab<Grid*> yGrid;
	Tab<Grid*> zGrid;

	Tab<int> hitList;
	BitArray xHitList, yHitList, zHitList;


};



#define BLOBMESH_CLASS_ID	Class_ID(0x6cb0b38d, 0x575b97a9)


#define A_RENDER			A_PLUGIN1

#define PBLOCK_REF	0

//STUB FOR NOW WE NEED TO REMOVE THIS ONCE WE MAKE THE PF SDK AVAILABLE

#define PARTICLEGROUP_INTERFACE Interface_ID(0x2c712d9f, 0x7bc54cb0) 




class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};


class BlobMesh;

class BlobMeshValidatorClass : public PBValidator
{
	public:
		BlobMesh *mod;
	private:
		BOOL Validate(PB2Value &v);
};


class BlobMesh : public SimpleObject2 , public IBlobMesh {
	public:


		// Parameter block handled by parent


		//Class vars
		static IObjParam *ip;			//Access to the interface
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		
		// From Object
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		//TODO: Evaluate the object and return the ObjectState
		ObjectState Eval(TimeValue t) { return ObjectState(this); };		
		//TODO: Return the validity interval of the object as a whole
		Interval ObjectValidity(TimeValue t) { Interval iv; iv.Set(t,t); return iv; }

//		void RescaleWorldUnits(float f);

		// From Animatable
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From SimpleObject
		
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }		
		
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();

		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);

		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return BLOBMESH_CLASS_ID;}		
		SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );




		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		

		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == BLOBMESH_INTERFACE) 
				return (IBlobMesh*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 


		//Constructor/Destructor

		BlobMesh();
		~BlobMesh();

		HWND paramsHWND;
		HWND pfparamsHWND;


		Tab<INode*> pfNodes;
		Tab<int>	addPFNodes;
		void PickPFEvents(HWND hWnd);

		void DisableButtons(HWND hWnd);


		BOOL inPickMode;
		void PickNodesMode();

		void	fnAddNode(INode *node);
		void	fnRemoveNode(INode *node);

		void	fnAddPFNode(INode *node);
		void	fnRemovePFNode(INode *node);

		void	fnPickMode();
		void	fnAddMode();
		void	fnAddPFMode();

		void	RemoveSelectedNode();


	private:
		BOOL inRender;
		INode *selfNode;
		BlobMeshValidatorClass validator;

};


class BlobMeshParamsDlgProc : public ParamMap2UserDlgProc {
	private:
		BlobMesh *obj;
	public:
		BlobMeshParamsDlgProc(BlobMesh *s) { obj = s; }
		 BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
	};


class BlobMeshDlgProc : public ParamMap2UserDlgProc {
	private:
		BlobMesh *obj;
	public:
		BlobMeshDlgProc(BlobMesh *s) { obj = s; }
		 BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
	};



class BlobMeshClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return BLOBMESH_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("BlobMesh"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};


class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};



//Pick mode to pick/add a bone from the viewport
class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		BlobMesh *obj;
		HWND hWnd;
		PickControlNode() {obj=NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
		HCURSOR GetDefCursor(IObjParam *ip);
		HCURSOR GetHitCursor(IObjParam *ip);


	};


class DumpHitDialog : public HitByNameDlgCallback {
public:
	BlobMesh *eo;
	DumpHitDialog(BlobMesh *e) {eo=e;};
	TCHAR *dialogTitle() {return _T(GetString(IDS_ADD));};
	TCHAR *buttonText() {return _T(GetString(IDS_ADD));};
	BOOL singleSelect() {return FALSE;};
	BOOL useProc() {return TRUE;};
	int filter(INode *node);
	void proc(INodeTab &nodeTab);
	};

#endif // __BLOBMESH__H
