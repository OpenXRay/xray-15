 /**********************************************************************
 
	FILE: ISkin.h

	DESCRIPTION:  Skin Bone Deformer API

	CREATED BY: Nikolai Sander, Discreet

	HISTORY: 7/12/99


 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#ifndef __ISKIN__H
#define __ISKIN__H

#include "ISkinCodes.h"

#define I_SKIN 0x00010000
#define I_SKINIMPORTDATA 0x00020000

//New interface for max 6
#define I_SKIN2 0x00030000

#define I_GIZMO 9815854
//Gizmo interface for r5 additions
#define I_GIZMO2 9815855
//Gizmo interface for r5.1 additions
#define I_GIZMO3 9815856

#define SKIN_INVALID_NODE_PTR 0
#define SKIN_OK				  1

//#define SKIN_CLASSID Class_ID(0x68477bb4, 0x28cf6b86)
#define SKIN_CLASSID Class_ID(9815843,87654)

class ISkinContextData
{
public:
	virtual int GetNumPoints()=0;
	virtual int GetNumAssignedBones(int vertexIdx)=0;
	virtual int GetAssignedBone(int vertexIdx, int boneIdx)=0;
	virtual float GetBoneWeight(int vertexIdx, int boneIdx)=0;
	
	// These are only used for Spline animation
	virtual int GetSubCurveIndex(int vertexIdx, int boneIdx)=0;
	virtual int GetSubSegmentIndex(int vertexIdx, int boneIdx)=0;
	virtual float GetSubSegmentDistance(int vertexIdx, int boneIdx)=0;
	virtual Point3 GetTangent(int vertexIdx, int boneIdx)=0;
	virtual Point3 GetOPoint(int vertexIdx, int boneIdx)=0;

};

class ISkin 
{
public:
	virtual int GetBoneInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	virtual int GetSkinInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	virtual int GetNumBones()=0;
	virtual INode *GetBone(int idx)=0;
	virtual DWORD GetBoneProperty(int idx)=0;
	virtual ISkinContextData *GetContextInterface(INode *pNode)=0;
//new stuff
	virtual TCHAR *GetBoneName(int index) = 0;
	virtual int GetSelectedBone() = 0;
	virtual void UpdateGizmoList() = 0;
	virtual void GetEndPoints(int id, Point3 &l1, Point3 &l2) = 0;
	virtual Matrix3 GetBoneTm(int id) = 0;
	virtual INode *GetBoneFlat(int idx)=0;
	virtual int GetNumBonesFlat()=0;
	virtual int GetRefFrame()=0;

};


// The following {class, member, macro, flag} has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.


class ISkinImportData
{
public:


/*** BOOL AddBoneEx(INode *node, BOOL update) ***/
/*
INode *node the bone to be added to skin
BOOL update is used to update the UI

 Adds a bone to the skin system.  Return TRUE if the operation succeeded.
*/
	virtual BOOL AddBoneEx(INode *boneNode, BOOL update)=0;



/**** virtual BOOL SetSkinBaseTm(INode *skinNode, Matrix3 tm) ***/
/*
INode *boneNode is that node that skin is applied to, need this so I can extract the local mod data
Matrix3 objectTm is the object matrix to assign as the new skin object tm
Matrix3 nodeTm is the node matrix to assign as the new skin node tm

When skin is applied to a node, that nodes initial objecttm is stored off so we can recompute the initial position
of the skin object.  This function allows you to change that tm.  It will store tm and the inverse tm of the 
matrix passed to it.  Return TRUE if the operation succeeded.

Below is the actual code that computes this when skin is added (node is the node that skin is applied to)
				d->BaseTM = node->GetObjectTM(RefFrame);
				d->BaseNodeTM = node->GetNodeTM(RefFrame); //ns
				d->InverseBaseTM = Inverse(d->BaseTM);
*/
	virtual BOOL SetSkinTm(INode *skinNode, Matrix3 objectTm, Matrix3 nodeTm)=0;


/**** virtual BOOL SetBoneTm(INode *boneNode, Matrix3 objectTm, Matrix3 nodeTm) ***/
/*
INode *boneNode is the pointer to the bone that is having its initial matrix change
Matrix3 objectTm is the object matrix to assign as the new skin object tm
Matrix3 nodeTm is the node matrix to assign as the new skin node tm

When a bone is added to skin it stores off the initial object and node tm of that bone.  This function allows 
you to change the intial matrix of a particular bone. Return TRUE if the operation succeeded.

Below is the actual code from skin that is called when a bone is added (t.node is the node of the bone being added)

			Matrix3 otm = t.Node->GetObjectTM(RefFrame);  //ns	
			Matrix3 ntm = t.Node->GetStretchTM(RefFrame) * t.Node->GetNodeTM(RefFrame);	
	
			BoneData[current].InitObjectTM = otm;		//ns
			BoneData[current].InitNodeTM = ntm;
			BoneData[current].tm    = Inverse(otm);
*/
	virtual BOOL SetBoneTm(INode *boneNode, Matrix3 objectTm, Matrix3 nodeTm)=0;



/*** BOOL AddWeights(INode *skinNode, int vertexID, Tab<INode*> &boneNodeList, Tab<float> &weights) ***/
/*
INode *skinNode  is the node that has skin applied to it, need this so I can get the local mod data for that node and skin since the same modifier can be applied to mulitple nodes thhrough instancing
int vertexID is the index of the vertex that you want to apply weights to
Tab<INode*> &boneNodeList is the list of bones that will control the vertex, these must already be added to the skin modifier
Tab<float> &weights is the weight of each bone

This adds weight to a specific vertex.  It will replace the current vertex weight data 
with the set supplied here.  The weights should sum to 1.0f
Both boneNodelist and weights must be the same size  Return TRUE if the operation succeeded.
*/
	virtual BOOL AddWeights(INode *skinNode, int vertexID, Tab<INode*> &boneNodeList, Tab<float> &weights)=0;
};


// End of 3ds max 4.2 Extension

//New For Max6
//this exposes the initial stretch matrix that a bone as since we seperated this out from the base
//matrix so we could turn it off.
class ISkin2
{
public:
/*
	This will set the initial stretch tm for a bone in skin
	boneNode  is the node of the bone to set
	stretchTm is the stretch matrix
	The stretch matrix is the bone stretch matrix applied to bone objects. You can get a node stretch tm by calling
	node->GetStretchTM()	
	This returns true if the function succeeds
	
	NOTE SetBoneTm will clear the stretchTM and set it to identity so make sure you call this after SetBoneTm
*/
	virtual BOOL SetBoneStretchTm(INode *boneNode, Matrix3 stretchTm)=0;

/*
	This will retrun  the initial stretch tm for a bone in skin
	boneNode  is the node of the bone to set			
*/

	virtual Matrix3 GetBoneStretchTm(INode *boneNode)=0;

/*
	This will let you get the current vertex selection set for skin
*/
	virtual void GetVertexSelection(INode *skinNode, BitArray &sel) = 0;
/*
	This will let you set the current vertex selection set for skin
*/
	virtual void SetVertexSelection(INode *skinNode, BitArray &sel) = 0;
};

class IGizmoBuffer
{
public:
Class_ID cid;
void DeleteThis() { delete this; 	}

};


class GizmoClass : public ReferenceTarget
	{
// all the refernce stuff and paramblock stuff here
public:

	ISkin *bonesMod;
    IParamBlock2 *pblock_gizmo_data;

	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i)
				{
				if (i == 0) return pblock_gizmo_data;
				else return NULL;
				}
	IParamBlock2* GetParamBlockByID(BlockID id)
				{
				if (pblock_gizmo_data->ID() == id) return pblock_gizmo_data ;
				 else return  NULL; 
				 }

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i)
		{
		if (i==0)
			{
			return (RefTargetHandle)pblock_gizmo_data;
			}
		return NULL;
		}

	void SetReference(int i, RefTargetHandle rtarg)
		{
		if (i==0)
			{
			pblock_gizmo_data = (IParamBlock2*)rtarg;
			}
		}

    void DeleteThis() { 
					delete this; 
					}


	int NumSubs() {return 1;}
    Animatable* SubAnim(int i) { return GetReference(i);}
 

	TSTR SubAnimName(int i)	{return _T("");	}

	int SubNumToRefNum(int subNum) {return -1;}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message)	
		{
		return REF_SUCCEED;
		}

    virtual void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev) {}
    virtual void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next) {}         
    virtual IOResult Load(ILoad *iload) {return IO_OK;}
    virtual IOResult Save(ISave *isave) {return IO_OK;}

//	void* GetInterface(ULONG id);  

//this retrieves the boudng bx of the gizmo in world space
	virtual void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc){}               
// this called in the bonesdef display code to show the gizmo
	virtual int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm ) { return 1;}
	virtual Interval LocalValidity(TimeValue t) {return FOREVER;}
	virtual BOOL IsEnabled() { return TRUE; }
	virtual BOOL IsVolumeBased() {return FALSE;}
	virtual BOOL IsInVolume(Point3 p, Matrix3 tm) { return FALSE;}

//this is what deforms the point
// this is passed in from the Map call in bones def
	virtual  Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)
		{return p;}
//this is the suggested name that the gizmo should be called in the list
	virtual void SetInitialName() {}
//this is the final name of the gizmo in th list
	virtual TCHAR *GetName(){return NULL;}
//this sets the final name of the gizmo in the list
	virtual void SetName(TCHAR *name) {}
// this is called when the gizmo is initially created
// it is passed to the current selected verts in the world space
	//count is the number of vertice in *p
	//*p is the list of point being affected in world space
	//numberOfInstances is the number of times this modifier has been instanced
	//mapTable is an index list into the original vertex table for *p
	virtual BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable) { return TRUE;}
//this is called before the deformation on a frame to allow the gizmo to do some
//initial setupo
	virtual void PreDeformSetup(TimeValue t) {}
	virtual void PostDeformSetup(TimeValue t) {}

	virtual IGizmoBuffer *CopyToBuffer() { return NULL;}
	virtual void PasteFromBuffer(IGizmoBuffer *buffer) {}

	virtual void Enable(BOOL enable) {}
	virtual BOOL IsEditing() { return FALSE;}
	virtual void EndEditing() {}
	virtual void EnableEditing(BOOL enable) {}

// From BaseObject
    virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm) {return 0;}
    virtual void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) {}
    virtual void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE ) {}
    virtual void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}
    virtual void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}
    virtual void ClearSelection(int selLevel) {}
	virtual void SelectAll(int selLevel) {}
    virtual void InvertSelection(int selLevel) {}



		
	};

	
//Gizmo extensions for R5
class IGizmoClass2 
	{
public:
	
	//this lets skin pass some tms to the gizmo so they can deal with the double transform
	//when the skin object and the skeleton are linked to the same node and that node is moved there will be a double transformation
	//in R5 we have an option to remove that transformation,

	//points come into the gizmo with the double trsansform on, this lets the gizmo remove and put back the transfrom
	//skin expects the points to have the double transform when it gets the points back from the gizmo since it willl
	//remove it later on
	//removeDoubleTransform - is a matrix3 that will remove the double transform
	//putbackDoubleTransform - is a matrix3 that will putback the double transform
	virtual void SetBackTransFormMatrices(Matrix3 removeDoubleTransform, Matrix3 putbackDoubleTransform) = 0;
	};


//Gizmo extensions for R5.1
class IGizmoClass3 
	{
public:
	
	//This exposes a tool that lets the gizmo know that the user wants to reset the plane of rotation using
	//the current bone orientation to define the plabe
	virtual void ResetRotationPlane()=0;
	};


#endif 