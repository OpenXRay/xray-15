/**********************************************************************
 *<
	FILE: TreeViewUtil.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __TREEVIEWUTIL__H
#define __TREEVIEWUTIL__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "notify.h"
#include "modstack.h"
#include "macrorec.h"
#include "utilapi.h"
#include "iFnPub.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;









#define MAPCHANNELDELETE_CLASS_ID	Class_ID(0x31f9c666, 0x3b4a555)
#define MAPCHANNELPASTE_CLASS_ID	Class_ID(0x31f9c666, 0x3b4a566)
#define MAPCHANNELADD_CLASS_ID		Class_ID(0x31f9c666, 0x3b4a577)

#define SELECTBYCHANNEL_CLASS_ID	Class_ID(0x31f9c666, 0x3b4a588)

#define TREEVIEWUTIL_CLASS_ID	Class_ID(0x4ae35082, 0x282bf350)


#define TRIGEOMCHANNEL	1
#define TRIMAPCHANNEL	2
#define TRISELCHANNEL	3

#define POLYGEOMCHANNEL	4
#define POLYMAPCHANNEL	5
#define POLYSELCHANNEL	6

#define PATCHGEOMCHANNEL	7
#define PATCHMAPCHANNEL		8
#define PATCHSELCHANNEL		9


#define CHANNEL_GEOM	10
#define CHANNEL_SEL		11
#define CHANNEL_MAP		12



class UVWData
{
public:
	INode *node;
	int channelType;
	int channelID;
	int subID;
	int numOfDeadVerts;

	TSTR nodeName;
	TSTR channelName;
	int numVerts, numFaces;
	int numRealFaces;
	int kbsize;
	
};


#define EMPTY			0
#define TRIMESH_GEOM	1
#define TRIMESH_MAP		2
#define TRIMESH_SEL		3

#define POLYMESH_GEOM	4
#define POLYMESH_MAP	5
#define POLYMESH_SEL	6

#define PATCHMESH_GEOM	7
#define PATCHMESH_MAP	8
#define PATCHMESH_SEL	9


class CopyBuffer
{
public:

	~CopyBuffer()
	{
		Clear();
	}
	int copyType;
	int subID;
	int numRealFaces;
	int numFaces;


	Tab<float>	w;

	Tab<Point3> verts;
	Tab<MNVert> mnVerts;

	Tab<TVFace> uvwFaces;
	Tab<Face>	geomFaces;

	Tab<MNMapFace*> uvwMNFaces;
	Tab<MNFace*>	geomMNFaces;
	

	Tab<TVPatch> uvwPatchFaces;
	Tab<int> patchDeg;


	void Clear();

	int pasteToChannelType;
	int pasteToChannel;
	int pasteToSubID;

    float mFalloff, mPinch, mBubble;
	int   mEdgeDist, mUseEdgeDists, mAffectBackface, mUseSoftSelections;


	TSTR name;

	

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	void SetGeomMNFaceCount(int count)
	{
		
		geomMNFaces.SetCount(count);
		for (int i = 0; i < count; i++)
			geomMNFaces[i] = new MNFace();
	}

	void SetMapMNFaceCount(int count)
	{
		
		uvwMNFaces.SetCount(count);
		for (int i = 0; i < count; i++)
			uvwMNFaces[i] = new MNMapFace();
	}

	void Copy(CopyBuffer *from);


};



class TreeViewUtil : public UtilityObj {
	public:



		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		

		void DeleteThis() { }		
		//Constructor/Destructor

		TreeViewUtil();
		~TreeViewUtil();	
		
		void CreateNewFloater();

		HWND floaterHWND;
		HWND listViewHWND;
		HTREEITEM hTRoot;

		Tab<UVWData*> nodeList;

		void AddSelectedNodesToList();

		LRESULT NotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void InitListView();
		void UpdateViewItems();
		void AddListViewItems();

		void InitWindow();
		void DestroyWindow();

		static void NotifyPreDeleteNode(void* param, NotifyInfo* arg);
		static void NotifySelectionChange(void* param, NotifyInfo*);

		BOOL lockSelection;
		BOOL collapse;

		BOOL subComponents;

		ICustButton* iLockButton;
		ICustButton* iClearButton;
		ICustButton* iNameButton;

		ICustButton* iCopyButton;
		ICustButton* iPasteButton;

		ICustButton* iAddButton;

		ICustButton* iSubCompButton;

		ICustButton* iCollapseButton;


		void DeleteChannel(INode *node, int mapID);

		void ShowRMenu(HWND hwnd, int x, int y) ;
		void UpdateUI();

		void NameChannel();

		void NameChannel(int whichChannel);
		void NameChannel(INode *node, int channelType, int channel, int subChannel, TCHAR *name);

		void AddNames();

		TSTR channelName;

		CopyBuffer buffer;

		int GetSel();
		void CopyToBuffer(int whichChannel);

		void CopyToBuffer(INode *node, int channelType, int channel, int subChannel);

		void PasteToNode(INode *node, int channelType, int channel, int subChannel);
		void PasteToNode(int whichChannel);

		void AddChannel(INode *node);
		void AddChannel(int whichChannel);

		void Collapse();
		void Collapse(INode *node);

		int GetDeadGeomVerts(Mesh *msh);
		int GetDeadMapVerts(Mesh *msh, int mp);

		int GetDeadGeomVerts(MNMesh *msh);
		int GetDeadMapVerts(MNMesh *msh, int mp);

		int GetDeadGeomVerts(PatchMesh *msh);
		int GetDeadMapVerts(PatchMesh *msh, int mp);


		void AddMesh(Mesh *mesh, INode *node);
		void AddPoly(MNMesh *mesh, INode *node);
		void AddPatch(PatchMesh *mesh, INode *node);
};




#define PBLOCK_REF 0

class MapChannelDelete : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_MAPCHANNELDELETENAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR|TEXMAP_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO|TEXMAP_CHANNEL|PART_VERTCOLOR; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return MAPCHANNELDELETE_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_MAPCHANNELDELETENAME);}

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

		MapChannelDelete();
		~MapChannelDelete();	

		int SetMapChannel(int id);
		

	private:

};


class MapChannelPaste : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_MAPCHANNELPASTENAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR|TEXMAP_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO|TEXMAP_CHANNEL|PART_VERTCOLOR|PART_SELECT|PART_SUBSEL_TYPE;; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return MAPCHANNELPASTE_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_MAPCHANNELPASTENAME);}

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

		MapChannelPaste();
		~MapChannelPaste();	

		int SetMapChannel(int id);
		int SetUseMapChannel(BOOL use);
		
		CopyBuffer buffer;


	private:

};



class MapChannelAdd : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_MAPCHANNELADDNAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR|TEXMAP_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO|TEXMAP_CHANNEL|PART_VERTCOLOR; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return MAPCHANNELADD_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_MAPCHANNELADDNAME);}

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

		MapChannelAdd();
		~MapChannelAdd();	



	private:

};



class SelectByChannel;

class SelectByChannelDlgProc : public ParamMap2UserDlgProc {
	private:
		SelectByChannel *mod;
	public:
		SelectByChannelDlgProc(SelectByChannel *s) { mod = s; }
		 BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
	};

class ChannelInfo
{
public:
	int mapID;
	int subID;
};


class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  int count;
	};



class SelectByChannel : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_SELECTBYCHANNELNAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

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
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return SELECTBYCHANNEL_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_SELECTBYCHANNELNAME);}

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

		SelectByChannel();
		~SelectByChannel();	


		HWND hWnd;

		void FillOutListBox();

		Tab<ChannelInfo> channelList;


	private:

};


class UpdateUIRestore : public RestoreObj {
	public:

		UpdateUIRestore() {
			}
		
		void Restore(int isUndo);
		void Redo() {};


		int Size() {return 1;}
		void EndHold() {}
	};




#endif // __TREEVIEWUTIL__H
