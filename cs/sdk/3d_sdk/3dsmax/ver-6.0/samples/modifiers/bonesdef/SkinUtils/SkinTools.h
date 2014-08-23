/**********************************************************************
 *<
	FILE: SkinTools.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __SKINTOOLS__H
#define __SKINTOOLS__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "modstack.h"

#include "utilapi.h"
#include "iskin.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


#define PASTESKINWEIGHTS_CLASS_ID		Class_ID(0x31f9c121, 0x3b4a454)


class WeightData
{
public:
	int vertIndex;
	float vertWeight;
};

class BoneData
{
private:
	TSTR name;
public:
	INode *node;
	TSTR matchName;
	Tab<WeightData> weights;
	int mapID, subID;

	TSTR GetDisplayName() { return matchName;}

	TSTR GetOriginalName() { return name;}
	void SetOriginalName(TSTR name) { 
			this->name = name;
			this->matchName = name;
			}
	
};

class TargBones
{
private:
	TSTR name;
public:
	int boneIndex;
	INode *node;
	TSTR matchName;
	Tab<int> matchingBones;

	TSTR GetDisplayName() { return matchName;}

	TSTR GetOriginalName() { return name;}
	void SetOriginalName(TSTR name) { 
			this->name = name;
			this->matchName = name;
			}

};

class PasteDataClass
{
public:
	Tab<int> liveSourceBones;
	Tab<int> deadSourceBones;

	Tab<int> liveTargetBones;
	Tab<int> deadTargetBones;

	void Clear()
	{
		liveSourceBones.ZeroCount();
		deadSourceBones.ZeroCount();
		liveTargetBones.ZeroCount();
		deadTargetBones.ZeroCount();
	}


};

class SkinTools : public UtilityObj {
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

		SkinTools();
		~SkinTools();		

		void ExtractData();
		void ExtractData(INode *node);

		void PasteData();
		void PasteData (INode *snode,INode *tnode);

		void MatchByName();
		void RemoveTargPrefix(BOOL remove);
		void RemoveTargSufix(BOOL remove);

		void RemoveSourcePrefix(BOOL remove);
		void RemoveSourceSufix(BOOL remove);


		Modifier *GetSkin(INode *node);
		Tab<BoneData*> boneList;
		Tab<TargBones*> targBoneList;


		HWND floaterHWND;

		void FilloutListBox();

		void RemoveTargetBones();
		void AddTargetBones();

		void RemoveSourceBones();
		void AddSourceBones();

		void SourceToTarget();
		void TargetToSource();
		
		PasteDataClass pasteData;

		float threshold;
		ISpinnerControl *iThreshold;

		void InitDialog();
		void DestroyDialog(BOOL ok);

		int matchType;
		BOOL nukeMatchType;
//		void MatchByPoints(Mesh *msh, ISkinImportData *skinImp, INode *snode, INode *tnode);
		void MatchByPoints(Mesh *msh, ISkinImportData *skinImp, INode *snode, INode *tnode);
		void MatchByPoints(MNMesh *msh, ISkinImportData *skinImp, INode *snode, INode *tnode);
		void MatchByPoints(PatchMesh *msh, ISkinImportData *skinImp, INode *snode, INode *tnode);
		void MatchByFace(Mesh *msh, ISkinImportData *skinImp, INode *snode, INode *tnode);

};



class PasteSkinWeights : public Modifier {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_PASTESKINWEIGHTS); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_VERTCOLOR|TEXMAP_CHANNEL; }
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
		Class_ID ClassID() {return PASTESKINWEIGHTS_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_PASTESKINWEIGHTS);}

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

		PasteSkinWeights();
		~PasteSkinWeights();	

		Tab<BoneData*> boneList;

		

	private:

};



#endif // __SKINTOOLS__H
