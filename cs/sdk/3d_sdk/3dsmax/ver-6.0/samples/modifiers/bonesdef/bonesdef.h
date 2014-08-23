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
#include "iPainterInterface.h"

#include "meshadj.h"

#include "imenuman.h"


#include "ActionTable.h"
#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"

#include "notify.h"

// This uses the linked-list class templates
#include "linklist.h"

#include "Bonesdef_Constants.h"
#include "Bonesdef_JointGizmo.h"
#include "Bonesdef_VertexWeights.h"
#include "Bonesdef_BoneData.h"
#include "Bonesdef_DlgProcs.h"

MakeLinkedList(BoneDataClass);

#include "Bonesdef_Undo.h"

#include "Bonesdef_WeightTable.h"

//MIRROR
#include "BonesDef_UniformGrid.h"
#include "BonesDef_Mirror.h"


#include "Components\MAXComponents.h"

#ifdef _DEBUG
	#undef _DEBUG
	#include <atlbase.h>
	#define _DEBUG
#else
	#include <atlbase.h>
#endif


//some functions to disable and enable spinners
// hWnd is the window that owns the spinner
// SpinNum is the spinner id
// Winnum is the field id
extern void SpinnerOn(HWND hWnd,int SpinNum,int Winnum);
extern void SpinnerOff(HWND hWnd,int SpinNum,int Winnum);



//just a quick helper function that projects a point from 
//viewspace to screen space
static Point2
ProjectPointF(GraphicsWindow *gw, Point3 fp) {
        IPoint3 out;
        gw->wTransPoint(&fp,&out);
        Point2 work;
        work.x = (float)out.x;
        work.y = (float)out.y;
        return work;
        }


class WeightRestore;
class BonesDefMod;
class CSkinCallback;
class BoneModData;
class GizmoJointClass;
class CreateCrossSectionMode;
class DumpHitDialog;


//this is the copy buffer for each of our cross sections
class CopyCrossClass
{
public:
	float inner, outer; // the inner and outer radius of the cross section
	float u;			// the u position of the cross section between the endpoints
};

//this is the copy buffer for the entire envelope
class CopyClass
{
public:
	Point3 E1, E2;				// the endpoints of the envelope
	BOOL absolute,showEnvelope;	// the properties of the envelopes
	int falloffType;
	Tab<CopyCrossClass> CList;	// this list of cross sections
};

//This is a helper class for loading envelopes
//this is a buffer we store the incoming envelope data
class LoadEnvelopeClass
{
public:
	TCHAR *name;		// the name of the bone
	int id;				// the index of the bone
	Point3 E1, E2;		// the end points of the envelope
	BYTE flags;			// the properties of the bone
	BYTE falloffType;	// the falloff type

	//cross section data
	Tab<float> u;		// list u values between the end points
	Tab<float> inner;	// the inner radius of each cross section
	Tab<float> outer;	// the outer radius of each cross section
};


// this is a helper class for loading vertex weights
// this is the buffer where vertex weights are stored when they are loaded
class LoadVertexDataClass
        {
public:
		TCHAR *name;							//the name of the vertex weight list
												//this is used to identify which node the weights belong to when
												//the modifier is instanced
		Tab<VertexListClass*> vertexData;		//the vertex weight info
		Tab<ExclusionListClass*> exclusionList;	//the exclusion list info
        };

//this class lets us attach different instances of skin
//since is skin is instanced there will be multiple instances of the weight data
//one per node. This class lets tie which instance belongs to which node
//on load a table of this type is created for each instance and then
//the matching weight data is attached
class LoadBaseNodeDataClass
        {
public:
	BoneModData *bmd;				//this is a pointer to local data for this instance
	INode *node;					//this is the node that owns this instance
	LoadVertexDataClass *matchData;	//this is the vertex weight info that belongs to this instance
        };




//this is the gizmo local data
//since every gizmo can use indices into the vertex list there needs to be local 
//data per gizmo in case the skin modifier is instanced
//this basically contains a list of vertices that the gizmo affects
class LocalGizmoData
{
private:
//new fix 2
	int start;				//start is the the first vertex index that is being deformed
							//just used to save some space for the bitarray

	BitArray deformingVerts;	//this is a temporary bitarray containing a list of all
								//vertices that the gizmo will deform.  This is a temporary
								//list that is created before deformation and destroyed rigth after
								//this list is used to speed up look ups of vertices that the gizmo owns

	Tab<int> affectedVerts;		//this is the actual list of all vertices owned by the gizmo

public:
	int whichGizmo;							//this is the gizmo index into the param block that owns this local data

	//this returns whether a vertex is owned by this gizmo
	//	index is the index vertex to be checked
	BOOL IsAffected(int index);

	//this returns whether a vertex is owned by this gizmo
	//	index is the index vertex to be checked
	//	where returns the index into affectedVerts which contains the vertex
	BOOL IsInList(int index,int &where);

	//Adds a vertex to the list
	//	index is the vertex index to be added
	void SetVert(int index);

	//load and save methods
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	//Returns the number of vertices that this gizmo owns
	int Count() { return affectedVerts.Count(); }

	//Clears the vertex list
	void ZeroCount() { affectedVerts.ZeroCount(); }

	//replaces an vert in affectedVerts with another index
	//	where which element to replace
	//	index the new index value
	void Replace(int where, int index ) { affectedVerts[where] = index; }

	//this replaces the entire vertex list
	void Replace(Tab<int> &list ) { affectedVerts = list; }

	//allows you to iterate through affectedVerts
	//	i the index into affectedVerts
	int GetVert(int i ) { return affectedVerts[i]; }

//new fix 2
	//this takes affectedVerts and jams it into the bitarray deformingVerts
	//this lets us speed up vertex look ups
	void BuildDeformingList();
	//this frees up the bitarray
	void FreeDeformingList();

	//this sees if a vertex is in the bitarray deformingVerts
	//BuildDeformingList must be called before using this
	//	int index the vertex to look up
	inline BOOL IsInDeformingList(int index)
		{
		index = index - start;
		if ((index >=0) && (index<deformingVerts.GetSize()))
			return deformingVerts[index];
		return FALSE;
		};
};



// this is local data for skin.  This mainly contains per vertex weight and attributes
class BoneModData : public LocalModData, public ISkinContextData {

	public:
		TimeValue lastTMCacheTime;					//this is the last evalutaion time
		BOOL nukeValidCache;						//this is flag used to nuke the caches

		BitArray autoInteriorVerts;					//this is a bitarray used when the object is a patch
													//flags which vertices are auto interior which should not
													//be deformed

		Tab<int> vecOwnerList;						//the is for patches, it is a list points and if that point is 
													//handle it points to the knot that owns it

		BitArray validVerts;						//this is a list of which vertices have been normalized

//gizmo data
		Tab<LocalGizmoData*> gizmoData;				//this is the table of local gizmo data

//vertex info
		// NS: flag that shows if the InitMesh TM has to be recomputed
		// this is only applicable for older pre max ver 3 files now
		bool recompInitMeshTM;

        float effect;  //this is just a variable to hold the value of the Abs Effect spinner

		// This data value is needed for the CSkinCallabck (Eval)
		TimeValue CurrentTime;
		
		// SkinCallback variables :
		DWORD		cookie;
		CSkinCallback *pSkinCallback;

		// The engine pointer
		ISkinEngine *pSE;


		//This is the list of vertices and their weights
		//each vertex contains a list of bones and how much each bone contributes(weight)
		Tab<VertexListClass*> VertexData;

		Matrix3 BaseTM,InverseBaseTM;   //BaseTM takes vertices from local space(skin) to world space
										//InverseBaseTM transforms from worlds space to local(skin)
		Matrix3 baseNodeOffsetTM;

		//these are 2 helper matrices to help gizmos deal with the double transform problem
		Matrix3 gizmoRemoveDoubleOffset;   //this is the matrix to pull out the double transform
		Matrix3 gizmoPutBackDoubleOffset;	//this is a matric to put back the double transform


		// This is the node TM of the mesh at initialization time
		Matrix3 BaseNodeTM;

		//these are just some flags to tell us what type of object the local date belongs to
		BOOL isMesh;
		BOOL isPatch;
		BOOL inputObjectIsNURBS;			//this is whether the object is a nurbs surface

		//vertex selection info
        BitArray selected;

		//this is the distance cache, basically whenever a bone is selected we compute the 
		//distance of the vertices to the envelope and store it here so we dont have to 
		//keep continually computing this data
		Tab<VertexDistanceClass> DistCache;

		//this is the bone that owns the DistCache
		int CurrentCachePiece;

		//these martix caches, just a bunch of matrix contations to save some time
		Tab<Matrix3> tmCacheToBoneSpace;		//this takes us from local space(skin) to bonespace
		Tab<Matrix3> tmCacheToObjectSpace;		//this takes us from bone space to local space(skin)

		//these are various tables of
		//these are used to compute distance since these use the initial tm
		Tab<Point3> tempTableL1;				//start points in local space(skin) using the initial tms
		Tab<Point3> tempTableL2;				//end points in local space(skin) using the initial tms
		//these are used for display since they use the current tm
		Tab<Point3> tempTableL1ObjectSpace;		//start points in local space(skin) using the current tms
		Tab<Point3> tempTableL2ObjectSpace;		//end points in local space(skin) using the current tms


		Point3 localCenter;					//this is local center of the sub object selection


		BonesDefMod *mod;  //ns Just a pointer back to the modifier that owns this data

//watje 9-7-99  198721 
        BOOL reevaluate;	//this is a flag that will force all computed weights to get recomputed
							//it will rebuild all caches



		//every bone contains a list of vertices which it will not affect
		//these are functions to examine and manipulate these lists
		Tab<ExclusionListClass*> exclusionList;		//these are the tables of exclusion lists

		//this returns whether a vertex is excluded for a particular bone
		//	boneID this is which bone to check
		//	vertID this is which vertex
		BOOL isExcluded(int boneID, int vertID);

		//this adds vertices to a bone exclusion list
		//	boneID which exclusion list to add to
		//	exList the array of vertices to add
		//	cleanUpVerts this goes back reduces the memory footprint of the exclusion lists
		void ExcludeVerts(int boneID, Tab<int> exList,BOOL cleanUpVerts=TRUE);

		//this removes vertices from an exclusion list
		//	boneID which exclusion list
		//	incList this is a list of vertices to take out of the exclusion list
		void IncludeVerts(int boneID, Tab<int> incList);

		//this selects all the vertices for an exclusion list
		//	boneID which exclusion list to use
		void SelectExcludedVerts(int boneID);

		//this goes through the exclusion list and removes dead space
		//reducing their size
		void CleanUpExclusionLists();

		BOOL forceRecomuteBaseNode;	//this causes all the initial bone matrices to be recomputed
									//this is used when the user resets skin or twiddles with 
									//Always Deform check box

		INode *meshNode;			//this is a pointer to the node that owns this local data

		BOOL needTMsRebuilt;		//this forces all the base node(the object that has skin applied to it) matrices to be rebuilt
		BOOL rebuildWeights;		//this forces all the weights to be normalized.  This usually does not need to be set

//edge connection list
		AdjEdgeList *edgeList;
		Tab<float> blurredWeights;
//blurred amount list

		void BuildEdgeList()
		{
			if (meshNode == NULL) return;

			ObjectState sos = meshNode->EvalWorldState(GetCOREInterface()->GetTime());
			blurredWeights.ZeroCount();
			if (sos.obj->IsSubClassOf(triObjectClassID))
			{

				TriObject *tobj = (TriObject*)sos.obj;
				Mesh *msh = &tobj->GetMesh();
			
			
			//if point count different bail
				if (msh->numVerts != VertexData.Count()) return;
				edgeList = new AdjEdgeList(*msh);
				blurredWeights.SetCount(msh->numVerts);
				for (int i = 0; i < msh->numVerts; i++)
					blurredWeights[i] = 0.0f;
			}
			//if mesh
				//get adr edge list
				//clone it local
			//if poly
				//etc
		}
		void BuildBlurData(int whichBone)
		{
		
			if (VertexData.Count() != blurredWeights.Count()) return;
			
			for (int i = 0; i < VertexData.Count(); i++)
			{
//find all the neighbors
				int numberOfNeighors = edgeList->list[i].Count();
//sum there weights and divide
				float weight = 0.0f;

				for (int j = 0; j < numberOfNeighors; j++)
				{
					int neighborEdge = edgeList->list[i][j];
					
					int neighbor = edgeList->edges[neighborEdge].v[0];
					
					if (neighbor == i) neighbor = edgeList->edges[neighborEdge].v[1];

//set that as the blurred weight
//look for a bone 
					int numberOfBones = 0;



					if ((neighbor >= 0) && (neighbor < VertexData.Count()))
					{
						numberOfBones = VertexData[neighbor]->d.Count();
						
						for (int k = 0; k <numberOfBones; k++)
						{

							int boneID = VertexData[neighbor]->d[k].Bones;
							if (boneID == whichBone)
							{
								//found a vertex with some weight
								weight +=  VertexData[neighbor]->d[k].normalizedInfluences;;
							}

						}
					}
					else 
					{
					}

				}


			blurredWeights[i] = weight/numberOfNeighors;
			}
		}
		void  FreeEdgeList()
		{
			if (edgeList) delete edgeList;
			blurredWeights.ZeroCount();
		}

		BOOL unlockVerts;			//this flag causes the selected bones to be reset(they become unmodified and are put back under control of envelopes)


		BoneModData(BonesDefMod *m)
			{
			unlockVerts = FALSE;
			edgeList = NULL;
			
			lastTMCacheTime = -999931;
			recompInitMeshTM = false; //ns
			pSkinCallback = NULL;
			InitSkinEngine();
			isMesh = FALSE;
			effect = -1.0f;
			mod = m;
//watje 9-7-99  198721 
            reevaluate = FALSE;
			nukeValidCache = TRUE;

			meshNode = NULL;
			forceRecomuteBaseNode = FALSE;
			needTMsRebuilt = FALSE;
			rebuildWeights = TRUE;
			}
		~BoneModData()
			{
			if(pSE)
				{
					// Unregister the Connection point
					HRESULT hr	= AtlUnadvise(pSE,IID__ISkinEngineEvents,cookie);
					pSE->Release();
					pSE = NULL;
				}

			for (int i=0;i<VertexData.Count();i++)	//free all our weigths
				{
				VertexData[i]->d.ZeroCount();
				if (VertexData[i] != NULL)
					delete (VertexData[i]);
				VertexData[i] = NULL;

				}


			VertexData.ZeroCount();

			for (i = 0; i < exclusionList.Count(); i++)	//free the exclusions list
				{
				if (exclusionList[i])
					delete exclusionList[i];
				}
			for (i = 0; i < gizmoData.Count(); i++)	//free the local gizmo data
				{
				if (gizmoData[i])
					delete gizmoData[i];
				}


			}	
		LocalModData*	Clone()
			{
			BoneModData* d = new BoneModData(NULL);
            d->reevaluate = FALSE;
			d->meshNode = NULL;
			d->forceRecomuteBaseNode = FALSE;
			d->needTMsRebuilt = TRUE;
			
			d->BaseTM = BaseTM;
			d->BaseNodeTM = BaseNodeTM; //ns
			d->InverseBaseTM = InverseBaseTM;

			int vertexCount = VertexData.Count();
			d->VertexData.SetCount(VertexData.Count());
			for (int i = 0; i < vertexCount; i++)
				{
				if (VertexData[i])
					{
					d->VertexData[i] = new VertexListClass();
					*d->VertexData[i] = *VertexData[i];
					}
				else d->VertexData[i] = NULL;
				}

//need to clone exclusions
			d->exclusionList.SetCount(exclusionList.Count());
			for (i = 0; i < exclusionList.Count(); i++)
				{
				if (exclusionList[i])
					{
					d->exclusionList[i] = new ExclusionListClass();
					*d->exclusionList[i] = *exclusionList[i];
					}
				else d->exclusionList[i]=NULL;
				}

			d->gizmoData.SetCount(gizmoData.Count());
			for (i = 0; i < gizmoData.Count(); i++)
				{
				if (gizmoData[i])
					{
					d->gizmoData[i] = new LocalGizmoData();
					*d->gizmoData[i] = *gizmoData[i];
					}
				else d->gizmoData[i]=NULL;
				}


			return d;

			}

		//this Initializes all the skin component engines
		void InitSkinEngine();
		
		// From ISkinContextData (Skin API) ns
		//returns the number of vertices
		virtual int GetNumPoints();
		//returns the number of bones assigned to a vertex
		//	pointIdx the vertex to look at
		virtual int GetNumAssignedBones(int pointIdx);
		//returns which bone is assigned to the vertex
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual int GetAssignedBone(int pointIdx, int boneIdx);
		//returns the weight of bone which is assigned to the vertex
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual float GetBoneWeight(int pointIdx, int boneIdx);

		//these return any spline info if the vertex is affacted by a spline

		//this returns which sub curve the vertex belongs to 
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual int GetSubCurveIndex(int pointIdx, int boneIdx);
		//this returns which sub segment the vertex belongs to 
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual int GetSubSegmentIndex(int pointIdx, int boneIdx);
		//this returns the sub segment U value
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual float GetSubSegmentDistance(int pointIdx, int boneIdx);
		//this returns the sub segment tangent value
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual Point3 GetTangent(int pointIdx, int boneIdx);
		//this returns the point on the sub segment
		//	pointIdx which vertex
		//	boneIdx the ith entry in the bone list
		virtual Point3 GetOPoint(int pointIdx, int boneIdx);
		
		Tab<BitArray*> namedSelSets;		//named selection sets 
};


//this is a time change call back used to update the UI since
//many of the parameters are not references
class BoneTimeChangeCallback : public TimeChangeCallback
{
	public:
		BonesDefMod* mod;
		void TimeChanged(TimeValue t);
};

//this class is used to save the node vertex color state so we can put it back
//when we are done
class vcSaveData
{
	public:
			INode *node;
			BOOL shade, vcmode;
			int type;
};

//this is a class used to save a node/bmd list of all instances of skin for 
//the painter
class PainterSaveData
{
	public:
			BOOL alt;
			INode *node;
			BoneModData *bmd;
};



//the actual skin modifier
class BonesDefMod : public Modifier, public ISkin, public ISkinImportData, public ISkin2,  public IPainterCanvasInterface_V5, public IPainterCanvasInterface_V5_1{  

		Tab<vcSaveData> vcSaveDataList;	//this is a table of the Vertex Color states of all the nodes so skin 
										//can put them back when it is donw

		static void NotifyPreDeleteNode(void* param, NotifyInfo*); //not sure why I need this has something to do with xrefs

		//these are pre/post save callbacks to set and restore the vertex color states
		static void NotifyPreSave(void* param, NotifyInfo*);
		static void NotifyPostSave(void* param, NotifyInfo*);

        public:


                //Constructor/Destructor
                BonesDefMod();
                ~BonesDefMod();


        // ANIMATABLE METHODS
                void DeleteThis() { 
					delete this; 
					}
                void GetClassName(TSTR& s) {s = GetString(IDS_RB_BONESDEFMOD);}  
                virtual Class_ID ClassID() { return Class_ID(9815843,87654);}           
                void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
                void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);         
                RefTargetHandle Clone(RemapDir& remap = NoRemap());
                TCHAR *GetObjectName() {return GetString(IDS_RB_BONESDEFMOD);}

                int SubNumToRefNum(int subNum);

				int NumRefs(); 
				int RemapRefOnLoad(int iref) ;
                RefTargetHandle GetReference(int i);
                void SetReference(int i, RefTargetHandle rtarg);

                int NumSubs();
                Animatable* SubAnim(int i);

                TSTR SubAnimName(int i);

				BOOL AssignController(Animatable *control, int subAnim);

                RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
                   PartID& partID, RefMessage message);
				void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

                Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);

                IOResult Load(ILoad *iload);
                IOResult Save(ISave *isave);
				
				IOResult SaveLocalData(ISave *isave, LocalModData *pld);
				IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

				// Animatable's Schematic View methods
				SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
				TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);

				void* GetInterface(ULONG id);  //ns

// JBW: direct ParamBlock access is added
//WEIGHTTABLE
				int	NumParamBlocks() { return 6; }					// return number of ParamBlocks in this instance
				IParamBlock2* GetParamBlock(int i) { if (i == 0) return pblock_param; 											
													else if (i == 1) return pblock_display;
													else if (i == 2) return pblock_gizmos;
													else if (i == 3) return pblock_advance;
													else if (i == 4) return pblock_weighttable;
//MIRROR
													else if (i == 5) return pblock_mirror;

													else return NULL;
												} // return i'th ParamBlock
				IParamBlock2* GetParamBlockByID(BlockID id) {if (pblock_param->ID() == id) return pblock_param ;
															 else if (pblock_display->ID() == id) return pblock_display;
															 else if (pblock_gizmos->ID() == id) return pblock_gizmos;
															 else if (pblock_advance->ID() == id) return pblock_advance;
															 else if (pblock_weighttable->ID() == id) return pblock_weighttable;
//MIRROR
															 else if (pblock_mirror->ID() == id) return pblock_mirror;

															 else return  NULL; } // return id'd ParamBlock


       // MODIFIERS METHODS
                ChannelMask ChannelsUsed()  {  
					if (ip)
						return PART_TEXMAP|PART_VERTCOLOR|PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;
					else return PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE;
					}
                ChannelMask ChannelsChanged() { 
					if (ip)
						return PART_TEXMAP|PART_VERTCOLOR|PART_GEOM;
					else return PART_GEOM;
					}
                Class_ID InputType() {return defObjectClassID;}
                void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
                Interval LocalValidity(TimeValue t);
								Interval GetValidity(TimeValue t);

				//the mouse start and end cycles
				void TransformStart(TimeValue t);
				void TransformFinish(TimeValue t);
				void TransformCancel(TimeValue t);

				Tab<TSTR*> namedSel;		
				BOOL SupportsNamedSubSels() {return TRUE;}

				void ActivateSubSelSet(TSTR &setName);
				void NewSetFromCurSel(TSTR &setName);
				void RemoveSubSelSet(TSTR &setName);
				void SetupNamedSelDropDown();
				int NumNamedSelSets();
				TSTR GetNamedSelSetName(int i);
				void SetNamedSelSetName(int i,TSTR &newName);
				void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);

	// Local methods for handling named selection sets
				int FindSelSetIndex(int index) ;
				int FindSet(TSTR &setName);		
				DWORD AddSet(TSTR &setName);
				void RemoveSet(TSTR &setName);
				void ClearSetNames();



		// BASEOBJECT METHODS
                int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
                int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);

                void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               

                void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
                CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
                void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
                void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
                void ActivateSubobjSel(int level, XFormModes& modes);
                void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
                void ClearSelection(int selLevel);
                void SelectAll(int selLevel);
                void InvertSelection(int selLevel);

				// NS: New SubObjType API
				int NumSubObjTypes();
				ISubObjType *GetSubObjType(int i);

		// ISKIN METHODS
				virtual int GetBoneInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset);
				virtual int GetSkinInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset);

				virtual int GetNumBones();
				virtual INode *GetBone(int idx);
				virtual DWORD GetBoneProperty(int idx);
				virtual ISkinContextData *GetContextInterface(INode *pNode);
				void UpdateBoneMap();
				Matrix3 GetBoneTm(int id);		
				INode *GetBoneFlat(int idx);
				int GetNumBonesFlat();
				int GetRefFrame();
//ISKINIMPORT DATA METHODS
				BOOL AddBoneEx(INode *node, BOOL update);
				BOOL SetSkinTm(INode *node, Matrix3 objectTm, Matrix3 nodeTm);
				BOOL SetBoneTm(INode *node, Matrix3 objectTm, Matrix3 nodeTm);
				BOOL AddWeights(INode *node, int vertexID, Tab<INode*> &nodeList, Tab<float> &weights);

//CANVAS INTERFACE FACE METHODS FROM IPAINTER INTERFACE V5
				IPainterInterface_V5 *pPainterInterface;
				BOOL  StartStroke();
				BOOL  PaintStroke(BOOL hit,
								  IPoint2 mousePos, 
								  Point3 worldPoint, Point3 worldNormal,
								  Point3 localPoint, Point3 localNormal,
								  Point3 bary,  int index,
								  BOOL shift, BOOL ctrl, BOOL alt, 
								  float radius, float str,
								  float pressure, INode *node,
								  BOOL mirrorOn,
								  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
								  Point3 localMirrorPoint, Point3 localMirrorNormal
								  ) ;

				BOOL  EndStroke(int ct, 
								  BOOL *hits,
								  IPoint2 *mousePos, 
								  Point3 *worldPoint, Point3 *worldNormal,
								  Point3 *localPoint, Point3 *localNormal,
								  Point3 *bary,  int *index,
								  BOOL *shift, BOOL *ctrl, BOOL *alt, 
								  float *radius, float *str,
								  float *pressure, INode **node,
								  BOOL mirrorOn,
								  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
								  Point3 *localMirrorPoint, Point3 *localMirrorNormal	);

				BOOL  EndStroke();
				BOOL  CancelStroke();
				BOOL  SystemEndPaintSession();

//Canvas Interface from IPainterCanvasInterface_V5_1 methods 
				void CanvasStartPaint();
				void CanvasEndPaint();


				TimeValue currentTime;		//the current eval time

//this is a temporary list of refernce points for fast look up of references
				//this function adds the ref pointer to refHandleList 
				void AddToRefHandleList(int id, RefTargetHandle rtarg);
				Tab<RefTargetHandle> refHandleList;	//this is the array that holds all our dynamic refs for fast look ups

//a look up table to speed up access to the sub anim list which was really dragging things down
				BOOL enableFastSubAnimList;		//this enables  the fast look up of sub anims
				BOOL rebuildSubAnimList;		//this forces the subanim table to be rebuild
												//this needs to be set any time the sub anim or ref struct changes

				Tab<Animatable*> subAnimList;	//a list of all our subanims
				void RebuildSubAnimList();		//this rebuilds our subanim list

				BOOL resolvedModify;			//this is just a flag to prevent some ugly stuff from happening before
												//everything has been put together

				BOOL splinePresent;				//this is a flag that is set if there is a bone that is a spline

//10-9-00 this is the initial starting position of the envelope used to constrain the manipulator in the current selected bones space
				Point3 localStart;
//10-9-00 this is the initial starting position of the crossection used to constrain the manipulator in the current selected bones space
				Point3 localVec;

				BOOL shadeVC;					//this is aflag that is set whether to display the vertex weights through the vertex color channel
				void SelectNextBone();			//this just selects the next bone in our list
				void SelectPreviousBone();		//this just selects the previous bone in our list

				//these are functions to add bones from the viewport by picking them
				void AddFromViewStart();		//this starts the command mode
				void AddFromViewEnd();			//this ends the command mode
				BOOL inAddBoneMode;		//whether or not we are in the addbone mode

				//these are for the multi delete bone command
				Tab<int> removeList;			//list of bones to delele
				void MultiDelete();				//the function to bring up the multi delete dialog


				BoneTimeChangeCallback boneTimeChangeCallback;	

				//this function computes the stretch factor for a bone
				//	t is the evaluation time
				//	node is the node of the bone to check
				//	index is the index of the bone 
				Matrix3 GetStretchTM(TimeValue t, INode *node, int index);

				//this function computes the squash factor for a bone
				//	t is the evaluation time
				//	node is the node that the squash is retrieved from
				float GetSquash(TimeValue t, INode *node);

				
				BOOL editing;			//this is just a flag to tell us if we have the UI rollup panels up

				//copy and paste gizmo functions
				static IGizmoBuffer *copyGizmoBuffer;	//the copy gizmo buffer
				void CopyGizmoBuffer();					//takes the current gizmo and copys it attributes into the buffer
				void PasteGizmoBuffer();				//takes the current gizmo and pastes the copy buffer into it
				
				BOOL inRender;							//flag to tell us whether we are in the render or not
				BOOL fastUpdate;						//flag to turn on/off some optimizations used for debugging

				//Render start/end functions
				int RenderBegin(TimeValue t, ULONG flags);	
				int RenderEnd(TimeValue t);

				// These booleans show, if the BoneMap or the InitTm have to be recomputed ns
				bool recompBoneMap;
				bool recompInitTM;

				//Our param block2 vars
                IParamBlock2 *pblock_param;
                IParamBlock2 *pblock_display;
                IParamBlock2 *pblock_gizmos;
                IParamBlock2 *pblock_advance;
				IParamBlock2 *pblock_weighttable;
//MIRROR
				IParamBlock2 *pblock_mirror;


				//window Rollup Handles
                static HWND hParam;
                static HWND hParamAdvance;
				static HWND hParamGizmos;

				Control *p1Temp;		//this is a point3 controller we used to handle mouse interaction in the viewport

				//this looks for an open entry in our refTable list
				int GetOpenID()
					{
					for (int i = 0; i < RefTable.Count();i++)
						{
						if (RefTable[i] == 0)
							{
							RefTable[i] = 1;
							return i+BONES_REF;
							}
						}
					int u = 1;
					RefTable.Append(1,&u,1);
					return RefTable.Count()-1+BONES_REF;
					}
				Tab<int> RefTable;  //this is an array of all our used dynamic reference ids
				
				// Since the BoneData table can contain entries, that are not used anymore, the bone map is used to provide a continous array of bones.
				// It contains the number of actually used bones. The indices map into the BoneData table.
				// When something in the BoneData table changes, the BoneMap has to be recomputed.
				Tab<int> BoneMap;


//bones info
	            BoneDataClassList BoneData;		//this is the linked list of all our bones
				int numValidBones;  //ns

				BOOL inPaint;					//this is a flag used to check id we are in the paint mode
				BOOL painting;					//this is a flag used to check whther the user is in a current paint stroke
				BOOL reloadSplines;				//this is a flag that will cause all our reference splines to be rebuilt 

				BOOL splineChanged;				//this is a flag that gets set when a spline is changed 
												//we need this the user can do nasty things like deleting
												//knots and curves which forces us to rebuild data
				int whichSplineChanged;			//this is which spline got changed


				//this is info for legacy files < 4
	            int OldVertexDataCount;
				Tab<VertexListClassOld*> OldVertexData;
				Matrix3 OldBaseTM,OldInverseBaseTM;


				BOOL cacheValid;			//this is a flag for the com engines cache

				void UnlockVerts();
				BOOL unlockBone;			//this flag cause all vertices attached to this bone to become unmodifier and back under control of envelopes
				BOOL unlockAllBones;		//this flag cause all vertices to become unmodifier and back under control of envelopes
				//this flag the method in response to the unlockBone flag which cause all vertices attached to this bone to become unmodifier and back under control of envelopes
				//	bmd the local data pointer since we are modifing vertice
				//	t the evaulation time
				//	os the object state of the object
				void UnlockBone(BoneModData *bmd,TimeValue t, ObjectState *os);
				//this flag the method in response to the unlockAllBones flag which all vertices to become unmodifier and back under control of envelopes
				//	bmd the local data pointer since we are modifing vertice
				//	t the evaulation time
				//	os the object state of the object
				void UnlockAllBones(BoneModData *bmd,TimeValue t, ObjectState *os);


				void RegisterClasses();			//this registers a bunch of static data like window classes etc
				static ICustToolbar *iParams;	//this is a pointer to the tool bar control that has the bone properties

				//this clears all the vertex selections
				//	bmd the local data pointer
                void ClearVertexSelections(BoneModData *bmd);
				void ClearBoneEndPointSelections();		//this clears the end point selection				
                void ClearEnvelopeSelections();			//this clears the cross section selection


				//these are helper functions to convert from array indices back and forth to UI indices
				//since we dont not show null entries in the UI, indices in the UI list box will differ
				//from the real indices into the BoneData linked list

				//this converts a UI index into an index into the BoneData linked list
				//	fsel is the UI index
				int ConvertSelectedListToBoneID(int fsel);
				//this converts an index into the BoneData linked list into a UI index
				//	fsel is the BoneData index
				int ConvertSelectedBoneToListID(int fsel);

                

//edit modes to determine what is currently being edited since we only use one sub object
                int ModeEdit;					//no longer used, i am just to much of a chicken to take it out
                int ModeBoneIndex;				//this is which bone is currently selected
                int ModeBoneEndPoint;			//this is which end point is selected -1 = none 0 = start 1 = end
                int ModeBoneEnvelopeIndex;		//this is which cross section is selected
                int ModeBoneEnvelopeSubType;	//this is which handle is selected 0-3 are the inner radius 4 to 7 is the outer

                Point3 Worldl1, Worldl2;		//these are the world space positions of the current selected bone

				//these enables/disables the UI buttons
				void EnableButtons();
				void DisableButtons();

				//these function to handle copying and pasting envelopes
				CopyClass CopyBuffer;		//this is the copy buffer
				void CopyBone();			//this takes the current selected bone and copies its attributes into the buffer
				void PasteBone();			//this takes the current selected bone and paste the copy buffer onto it
				void PasteToAllBones();		//this takes all the bones and paste the copy buffer onto it
				void PasteToSomeBones();	//this brings up a dialog and lets the user paste to selected bones
				Tab<int> pasteList;

				//this adds a cross section to a particular bone
				//	boneid is which bone to add the cross section to
				//	u is where along the bone to add the cross section
				//	inner is the inner radius of the cross section
				//	outer is the outer radius of the cross section
				//  update whethr the system updates after the method is done
                void AddCrossSection(int boneid, float u, float inner, float outer, BOOL update = TRUE);
				//this adds a cross section to the current selected bone, inner and outer
				//radii are compued based on the neighboring cross sections
				//	u is where the cross cross scetion is to be added
                void AddCrossSection(float u);

				//this takes 2 points in world space( segment) and a point in screen space and find the closest point to the segment and returns the U
				//	vpt the view exp pointer
				//	a the start point in world space
				//	b the end point in world space
				//	o the point in screen space to find the closest to the segment
                float GetU(ViewExp *vpt,Point3 a, Point3 b, IPoint2 p);

				//this returns the inner and outer radius of a cross section
				//	inner the inner radius of the cross section
				//	outer the outer radius of the cross section
				//	boneID the id of the bone to look at
				//	crossID the id of the cross section to look at
				void GetCrossSectionRanges(float &inner, float &outer, int BoneID, int CrossID);
				
				//this returns the the endpoints of a bone in local space(skin) at a particular time
				//	bmd the local data pointer
				//	t the evaluation time
				//	l1 the start point
				//	l2 the end point
				//	boneID the bone to look at
				void GetEndPoints(BoneModData *bmd, TimeValue t,Point3 &l1, Point3 &l2, int BoneID);
				//this returns the the endpoints of a bone in local space(skin) at the iniatialization time
				//	bmd the local data pointer
				//	t the evaluation time
				//	l1 the start point
				//	l2 the end point
				//	boneID the bone to look at
				void GetEndPointsLocal(BoneModData *bmd, TimeValue t,Point3 &l1, Point3 &l2, int BoneID);

				//The gives you approximately the U value along a spline given the segment and the how far along the segment you are
				//	t the evaluation time
				//	LineU the u value along the segment
				//	BoneID which bone to inspect
				//	sid the segment 
				float ModifyU(TimeValue t, float LineU,  int BoneID, int sid);

				//This computes the influence of a bone for a vertex
				//	t the evalation time
				//	Influence the distane from the bone
				//	u the U value between end and start points
				//	BoneID the bone that we want to compute the influence from
				//  StartCross and EndCross the cross sections that bound the U
				//	sid the segment ID for splines
				float ComputeInfluence(TimeValue t, float Influence, float u, int BoneID, int StartCross, int EndCross, int sid);

				//This builds are distance cache table for a bone
				//	bmd the local data pointer
				//	BoneIndex the bone that you want to build the cache for
				//	t the evaluation time
				//	os the object state pointer of the skinned object
				void BuildCache(BoneModData *bmd, int BoneIndex, TimeValue t, ObjectState *os);
				//no longer used was used to thread the distance computations
				void BuildCacheThread(BoneModData *bmd, int start, int end, int BoneIndex, TimeValue t, ObjectState *os, ShapeObject *pathOb, Matrix3 ntm );

				//this updates all our matrix cache tables
				//	bmd the local data pointer
				//	t the evaluation time
				//	valid the interval of controllers is returned here
				//	forceCompleteUpdate will force all tables to be updated, otherwise only tables that it thinks needed to be updated will be
				void UpdateTMCacheTable(BoneModData *bmd, TimeValue t, Interval& valid, BOOL forceCompleteUpdate = TRUE);



//distance stuff, that computes all our distance to line and splines
				

                //recursion function to find closest segemnt
                void RecurseDepth(float u1, float u2, float &fu,  Spline3D *s,int Curve,int Piece, int &depth, Point3 fp);
                void PointToPiece(float &tempu, Spline3D *s,int Curve,int Piece, int depth, Point3 fp);

                
				//Given a spline and a point find the closest point on a shape.  The return value is bogus, use the U parameter 
				//	p1 the point in local space
				//	s the spline object to find the closest point on the curve to P1
				//	u the closest u segemnt value
				//	p the closest point on the curve
				//	t the tangent of that point
				//	cid the curver index will always be 1
				//	sid the segment index for the U
				//	tm the matrix that goes from local space(skin) to spline space 
                float SplineToPoint(Point3 p1, Spline3D *s, float &u, Point3 &p, Point3 &t, int &cid, int &sid, Matrix3 tm);

                //this finds the closest distance between a point and a line segment
				//	p1 is the point
				//	l1 and l2 define the line segment
				//	u is the u value between l1 and l2
				float LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u);


				//This takes a node and builds the inner and outer cross section based on the nodes points perpendicular to the envelope
				//	bnode the node of the bone
				//	obj the object pointer of the bone
				//	l1 and l2 are the end points of the envelope in world space
				//	el1 and el2 are the suggested inner and outer cross section widths
				void BuildEnvelopes(INode *bnode, Object *obj, Point3 l1, Point3 l2, float &el1, float &el2);

				//this build the end points of the envelope based on a nodes bounding box
				//	node is the INode pointer of the bone
				//	s is the start point of the envelope in bone space
				//	e is the end poinf of the envelope in bone space
				//	el1 is axis length in world space
				//	tm is the matrix to transfrom from bone space to world space
                void BuildMajorAxis(INode *node, Point3 &s, Point3 &e, float &el1, Matrix3 *tm= NULL);

				//this removes the current selected bone from skin
                void RemoveBone();
				//this removes a particular bone from skin
				//	bid is the bone to be reomved
                void RemoveBone(int bid);

				//this removes the current selected cross section from the envelope
                void RemoveCrossSection();
				//this removes a cross section from a particular bone
				//	bid is the bone to have it cross section removed
				//	eid is which cross section is to be removed
				void RemoveCrossSection(int bid, int eid);
				//this removes a cross section from a particular bone, this one does no notification
				//	bid is the bone to have it cross section removed
				//	eid is which cross section is to be removed
				void RemoveCrossSectionNoNotify(int bid, int eid);

				//this sets all the selected vertices weight for a bone
				//	bmd the local data pointer
				//	BoneID which bone to weight to
				//	amount the absolute amount(weight)
                void SetSelectedVertices(BoneModData *bmd, int BoneID, float amount);


				//this sets all a particular vertex weight for a bone
				//	bmd the local data pointer
				//	VertID the vertex to change
				//	BoneID which bone to weight to
				//	amount the absolute amount(weight)
                void SetVertex(BoneModData *bmd, int vertID, int BoneID, float amount);
				//this replaces the weights of a particular vertex
				//	bmd the local data pointer
				//	VertID the vertex to change
				//	BoneIDList list of bones
				//	amountList list of weights must match in size the BoneIDList
				void SetVertices(BoneModData *bmd,int vertID, Tab<int> BoneIDList, Tab<float> amountList);

				//this compute the falloff of vertex
				//	u is the percent weight
				//	ftype is the falloff type
				//			BONE_FALLOFF_X3_FLAG cubic
				//			BONE_FALLOFF_X2_FLAG squared
				//			BONE_FALLOFF_X_FLAG	linear
				//			BONE_FALLOFF_SINE_FLAG	sinual
				//			BONE_FALLOFF_2X_FLAG	
				//			BONE_FALLOFF_3X_FLAG	
                void ComputeFalloff(float &u, int ftype);


				//this returns the normalized weight of a vertex for a bone
				//	bmd pointer to the local data
				//	vid is which vertex to inspect
				//	bid is the ith weight to get from the bones list
				float RetrieveNormalizedWeight(BoneModData *bmd, int vid, int bid);

				//this goes through and normalizes all our weights
				//	bmd is the local data pointer
				void BuildNormalizedWeight(BoneModData *bmd);

				//this forces all weights to be reevaluated on eevrybone
				//	bmd the local data pointer
				//	t the evaluation time
				//	os is the object state pointer of the skinned object
                void RecomputeAllBones(BoneModData *bmd, TimeValue t, ObjectState *os);

				//this recomputes the weight so just one bone
				//	bmd the local data pointer
				//	BoneIndex the bone to be recomputed
				//	t the evaluation time
				//	os is the object state pointer of the skinned object
                void RecomputeBone(BoneModData *bmd, int BoneIndex,TimeValue t, ObjectState *os);

				//this function does all the spline animation deformation it returns the defomed point
				//this is no longer called directly unless the com engine is dead
                Point3 VertexAnimation(TimeValue t, BoneModData * bmd, int vertex, int bone, Point3 p);

//dialog variables
				//these are variables that are filled out in the modify loop 
				//representing the current UI state

				//select and draw filters
                int FilterVertices, FilterBones,FilterEnvelopes,DrawEnvelopes;
				BOOL drawAllVertices;
				int DrawVertices;
				BOOL displayAllGizmos;

				//the global rigid properties
				BOOL rigidVerts, rigidHandles;

                
                int RefFrame;		//the reference frame for skin
				int AlwaysDeform;	//the always deform state




                BOOL reset;			//this is a variable that if set to true wil reset the entire system on the next modifiy
                BOOL BoneMoved;		//this is just flag to tell us if a bone has moved




                static IObjParam *ip;		//just the interface pointer
				ULONG flags;				//flags to restore the UI in the begin/end edit
				Animatable *prev;

                static BonesDefMod *editMod;		// pointer to ourselve
                static MoveModBoxCMode *moveMode;	// the move mode command pointer


				//starts the insert cross section mode
				//	type not used any more
                void StartCrossSectionMode(int type);
                static CreateCrossSectionMode *CrossSectionMode;  // the cross section mode pointer

                void StartPaintMode();		//starts the paint mode
                void PaintOptions();		//brings up the paint options dialog


				//Pointers to some custom ui button
                static ICustButton*   iCrossSectionButton;
                static ICustButton*   iPaintButton;
                static ICustButton*   iLock;
				static ICustButton*	  iEnvelope;
				static ICustButton*   iAbsolute;
				static ICustButton*   iFalloff;
				static ICustButton*   iCopy;
				static ICustButton*   iPaste;
				static ICustButton*   iEditEnvelopes;


                int LastSelected;	// the last selected bone so when the ui is brought up we can restore the last selected bone


				//given a mod context this lets you find the node that owns it
				//this is real useful since it lets a modifier find the node that 
				//owns it
				//	smd is the modcontext
				//	which is no longer used
				INode* GetNodeFromModContext(ModContext *smd, int &which);




				//This draws a cross section loop and markers on the loop 
				//	a the center of the cross section in view space
				//  Align is the vector that defines the cross section plane(the Perp) in view space
				//	length is the radius of the cross section
				//	tm no longer used
				//	gw the pointer to the graphics window
                void DrawCrossSection(Point3 a, Point3 Align, float length, Matrix3 tm, GraphicsWindow *gw);

				//This draws a cross section loop and but no markers 
				//	a the center of the cross section in view space
				//  Align is the vector that defines the cross section plane(the Perp) in view space
				//	length is the radius of the cross section
				//	gw the pointer to the graphics window
                void DrawCrossSectionNoMarkers(Point3 a, Point3 Align, float length,GraphicsWindow *gw);

				//This draws the end cross section loop, the end capsule loops and markers on the loop 
				//	a the center of the cross section in view space
				//  Align is the vector that defines the cross section plane(the Perp) in view space
				//	length is the radius of the cross section
				//	tm no longer used
				//	gw the pointer to the graphics window
                void DrawEndCrossSection(Point3 a, Point3 align, float length,  Matrix3 tm, GraphicsWindow *gw);

				//This returns the 4 hit points of the cross section
				//	a the center of the cross section in view space
				//  Align is the vector that defines the cross section plane(the Perp) in view space
				//	length is the radius of the cross section
				//	tm no longer used
				//	gw the pointer to the graphics window
                void GetCrossSection(Point3 a, Point3 Align, float length, Matrix3 tm,  Point3 *edge_p);
				//identical to above
                void GetCrossSectionLocal(Point3 a, Point3 Align, float length, Point3 *edge_p);

				//this draws the envelope minus the cross sections
				//	a is a list of cross section points in view space
				//	length is the radius of each cross section
				//	tm no longer used
				//	gw the pointer to the graphics window
                void DrawEnvelope(Tab<Point3> a, Tab<float> length, int count, Matrix3 tm, GraphicsWindow *gw);



				//this zoooms the viewport to the current selected bone
				//	all zooms all the viewports
				void ZoomToBone(BOOL all);
				Box3 currentGizmoBounds;						//current gizmo bounds
				//this zooms the view to the current selected gizmo
				//	bmd the local data pointer
				//	all whether to zoom all the viewports
				void ZoomToGizmo(BoneModData *bmd,BOOL all);



				//this updates the abs weight spinner
				//this needs to be called whenthe  vertex selection changes
				//	bmd the local data pointer
				void UpdateEffectSpinner(BoneModData*bmd);

				
				BOOL updateP;						//flag to tell use whether we need to update our P1 controller
				//this updates all our center and tm data for the view port
				//	bmd the local data pointer
				void UpdateP(BoneModData* bmd);

				//This is used when the user toggles the Always deform option 
				//it tracks the changes in the end points of the envelopes
				Tab<Point3> endPointDelta;
				//This is used when the user toggles the Always deform option 
				//it tracks the changes in the end points of the envelopes
				void UpdateEndPointDelta();

				//this just resets all our selection to the minimum
				void ResetSelection();
				//this syncs up our selection, sice selection is stored in the bone attributes
				//and as a seperate variable this makes sure they all match andif not force them too
				void SyncSelections();


				void EnableRadius(BOOL enable);			//this enables/disables the radius spinner
				void EnableEffect(BOOL enable);			//this enables/disables the weight spinner
				void LimitOuterRadius(float outer);		//this clamps a value to the outer radius val
				void LimitInnerRadius(float inner);		//this clamps a value to the inner radius val

				void UpdatePropInterface();				//this updates the bone properties UI


				//XREF stuff
				Matrix3 initialXRefTM;		//initial XREF tm
				Matrix3 xRefTM;				//the current XREF tm
				INode *bindNode;			// the xref bind node if there is one

				int HoldWeights();					//this holds all our vertex weights
				int AcceptWeights(BOOL accept);		//this accepts or cancels the HoldWeigths


//watje 9-7-99  198721 
				void Reevaluate(BOOL eval);				//this tags or local data to be reevaled
				void ForceRecomuteBaseNode(BOOL eval);	//this tags or local data to recompuet the base node matrices

//watje 10-13-99 212156
				BOOL DependOnTopology(ModContext &mc);  //Depends on topo method

				//this adds a bone to skin
				//	node the bone to be added
				//	update whether to update the stack and the viewport
				void AddBone(INode *node, BOOL update);

				void ExcludeVerts();		//excludes the current selected vertices from the curent selected bone
				void IncludeVerts();		//includes the current selected vertices from the curent selected bone
				void SelectExcludedVerts();	//selects the vertices that are currently excluded by the currenly selected bone

				void RefillListBox();		//this rebuilds the bones list box

				//Brings up the save envelope dialog
				//	defaultToBinary whether the dialog defaults to binray or text format type
				void SaveEnvelopeDialog(BOOL defaultToBinary = TRUE);
				//This saves the envelope and vertex data to disk
				//	name the file name
				//	asTest whther the file is text or binary
				void SaveEnvelope(TCHAR *name, BOOL asText = FALSE);
				int ver;	//the env file version

				//This brings up the load envelope dialog
				//	defaultToBinary whether the dialog defaults to binray or text format type
				void LoadEnvelopeDialog(BOOL defaultToBinary = TRUE);
				//This loads the envelope and vertex data onto the modifier
				//	name the file name
				//	asTest whther the file is text or binary
				BOOL LoadEnvelope(TCHAR *name,BOOL asText=FALSE);

				Tab<LoadEnvelopeClass*> dataList;				//this is where all our envelope data is loaded into temporarily
				Tab<LoadVertexDataClass*>   vertexLoadList;		//this is where all our weight data is loaded into temporarily
				Tab<LoadBaseNodeDataClass>  loadBaseNodeData;	//this is table of all the instances of skin for loading
				//this builds our skin instance list since there can be many of instances of skin
				//that need to be matched up to the correct weight data
				void BuildBaseNodeData();

				//These are functions to match points together when there are different topos

				//This returns the closest points to a point
				//	p the point to match
				//	pointList the list of points to look into
				//	threshold is the max distance to look for matches
				//	count is the maximum number of neighbors to find
				//	hitList	the list of neightboring vertices
				void GetClosestPoints(Point3 p, Tab<Point3> &pointList,float threshold, int count,Tab<int> &hitList, Tab<float> &distList);

				//this remaps our vertex data
				//	bmd the local data pointer
				//	the closest point threshold
				//	loadID is which loadBaseNodeDate entry to use
				//	obj is used when there is no load. This is used when the topo in the stack changes
				void RemapVertData(BoneModData *bmd,  float threshold, int loadID = -1 ,Object *obj=NULL);
				//this remaps our exclusion data
				//	bmd the local data pointer
				//	the closest point threshold
				//	loadID is which loadBaseNodeDate entry to use
				//	obj is used when there is no load. This is used when the topo in the stack changes
				void RemapExclusionData(BoneModData *bmd,  float threshold, int loadID = -1 ,Object *obj=NULL);
				//this remaps our local gizmo data
				//	bmd the local data pointer
				//	the closest point threshold
				//	loadID is which loadBaseNodeDate entry to use
				//	obj is used when there is no load. This is used when the topo in the stack changes
				void RemapLocalGimzoData(BoneModData *bmd,  float threshold, int loadID = -1 ,Object *obj=NULL);

				//these are load properties
				BOOL pasteEnds, pasteCross;		//whther to load end points and cross sections
				BOOL loadVertData;				//whther to load weight data
				BOOL loadExclusionData;			//whether to load exclusion list
//5.1.02
				BOOL loadByIndex;				//whether to match vertices by indices instead of position

//gizmo stuff
				//adds the current selected gizmo type to the list
				//and updates all the local mod data
				void AddGizmo();
				void RemoveGizmo();				//removes the current selected gizmo
				void SelectGizmo(int id);		//selects a gizmo
				void SelectGizmoType(int id);	//selects a gizmo type

				//fills out the list box in the Gizmo Rollout
				void UpdateGizmoList();
				int currentSelectedGizmoType;	//the current gizmo type
				int currentSelectedGizmo;		//the current selected gizmo
				Tab<Class_ID> gizmoIDList;		//a list of gizmo types

				TCHAR *GetBoneName(int index);	//returns the bone name which is the node name now (was her to support our old bones)
				int GetSelectedBone();			//returns the current selected bone
				//get the end points of bone in bone space
				//	id the bone 
				//	l1 and l2 are the end pints
				void GetEndPoints(int id, Point3 &l1, Point3 &l2);


				Tab<Spline3D*> splineList;		//this is a cache to any bone splines at the current eval time

				//This shades the mesh using the vertex colors to show the weights
				//	msh the mesh object
				//	bmd the local data pointer
				void ShadeVerts(Mesh *msh, BoneModData *bmd);
				//This shades the mesh using the vertex colors to show the weights
				//	msh the polumesh object
				//	bmd the local data pointer
				void ShadeVerts(MNMesh *msh, BoneModData *bmd);
#ifndef NO_PATCHES
				//This shades the patch using the vertex colors to show the weights
				//	msh the patch object
				//	bmd the local data pointer
				void ShadeVerts(PatchMesh *msh, BoneModData *bmd);
#endif


				

				BOOL updateListBox;		//flag used to toggle the list box rebuild

				//This returns a local data for skin given a node
				//	pNode the node that has skin on it
				BoneModData *GetBMD(INode *pNode);

//WEIGHTTABLE
//WeightTable Data and Methods		

				//This function needs to be called everytime a vertex selection or bone selection changes
				//it updates the vertex list in the weight table if it is needed				
				void PaintAttribList();

				//this is a class that manages the weight table window
				WeightTableWindow weightTableWindow;

				//this is the windows handle to the weight table
				HWND hWeightTable;

				//this is the function that brings up the weight table
				void fnWeightTable();

				void RefDeleted()	
					{
					if (hWeightTable)
						{
						}
					}

				//just some debug tools
				BOOL GetDebugMode() {
									BOOL value = TRUE;
									if ( pblock_weighttable)
										{
										pblock_weighttable->GetValue(skin_wt_debugmode,0,value,FOREVER);
										}
									return value;
									}

				void SetVCMode();		//this set the vertex color mode for a node
				void RestoreVCMode();	//this restore the vertex color for a node

				//This toggles the property whether vertex weights are normalized or not
				//	bmd the local data pointer
				//	vertID the vertex to normalize
				//	unNormalize whether to normalize or not
		void NormalizeWeight(BoneModData *bmd,int vertID, BOOL unNormalize);

				//this toggles the normalize property of selected vertice
				//	norm whther to normalize or not
		void NormalizeSelected(BOOL norm);

				//this toggle the rigid property of selected vertices
		void RigidSelected(BOOL rigid);
				//this toggle the rigid handle property of selected vertices
		void RigidHandleSelected(BOOL rigid);

				//this bakes all the selected vertice so they are modified and no longer under the influence
				//of an envelope
		void BakeSelectedVertices();

				void InvalidateWeightCache();	//this just nukes any weigth caches we have

//painter stuff
				//Given a center and axis get the mirrored bone match
				//	center in world space
				//	axis the world space mirror axis
		int	  GetMirrorBone(Point3 center, int axis);
		void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) {} //we dont muck with the display so just do nothing
				//Setsup the mirror bone
		void  SetMirrorBone();

				BOOL backTransform;		//whether or not we back transforming the vertices to get rid of the double transform
				

		BOOL stopEvaluation; //this is used to prevent the modifier from getting evaluated.
							 // the problem is that the modifier can get evaled before my post load call back is called
							//which will cause a crash since not everything is initialized.


				BOOL fastGizmo;		//just a toggle for fast gizmo optimizations

				Tab <GizmoClass*> gizmoDefList;	// a list of all our gizmos
				int gizmoDefListCount;			//the number of gizmos


//5.1.03
				BOOL hasStretchTM;		//this is a flag to tell the system whether init node tm has its
										//stretch tm concated in or not
										//5.1 files and below will have this 
										//in 5.5 we seperated them so we could turn off the effect

//MIRROR
				MirrorData mirrorData;  //this is where all the mirror info is stored for bonee 
										//and verts

				//See ISkin2 in ISkin.h for what these do
				BOOL SetBoneStretchTm(INode *boneNode, Matrix3 stretchTm);
				Matrix3 GetBoneStretchTm(INode *boneNode);
				
				void GetVertexSelection(INode *skinNode, BitArray &sel);
				void SetVertexSelection(INode *skinNode, BitArray &sel);				
				
protected:
	private:
//WeightTable Data and Methods		

		//this registers the weight table window class
		void RegisterClasses2();			

		//this checks to see if skin it xrefed andbound to a helper
		void CheckForXRefs(TimeValue t);
		//this is used to load old data
		BOOL InitLocalData(TimeValue t, ModContext &mc, ObjectState *os, INode *node, Interval valid);
		//this rebuilds all our skin node matrices
		BOOL RebuildTMLocalData(TimeValue t, ModContext &mc, BoneModData *bmd);

		BOOL vcState;					//whther or not to us the vertex color display
		BOOL stopMessagePropogation;	//a flag to stop message propogation
		BOOL updateOnMouseUp;			//the flags will force updates only on mouse up
		int boneLimit;					//this is a max number fo bones that will influence a vertex
		

//Painter info
		Tab<PainterSaveData> painterData;	//this is a list of all the skin instances
		int lagRate;
		int lagHit;
		int mirrorIndex;
		void ApplyPaintWeights(BOOL alt, INode *incNode);
		void RebuildPaintNodes();

};




				
			


class CSkinCallback : public _ISkinEngineEvents
{
public:
	// IUnknown
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);

	HRESULT STDMETHODCALLTYPE GetInterpCurvePiece3D( int BoneId,int CurveId,int SegId,float distance,float __RPC_FAR *pPoint);    
    HRESULT STDMETHODCALLTYPE GetTangentPiece3D(int BoneId,int CurveId,int SegId,float distance,float __RPC_FAR *pPoint); 
	
	CSkinCallback(BoneModData *m) : m_cRef(0), bmd(m) {  }
	~CSkinCallback() { }

private:
	long m_cRef;
	BoneModData *bmd;
};





class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  int count;
	};



class BMDModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	Tab<BoneModData *> bmdList;
	BMDModEnumProc(BonesDefMod *l)
		{
		lm = l;
		}
private:
	BOOL proc (ModContext *mc);
};


		

class BonesDefClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BonesDefMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BONESDEFMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
											   
	Class_ID		ClassID() { return Class_ID(9815843,87654); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("skin"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	int             NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i) { return GetActions(); }

//WEIGHTTABLE
	ActionTable* GetActions()
	
			{
		    TSTR name = GetString(IDS_WEIGHTTABLE);
		    ActionTable* pTab;
		    pTab = new ActionTable(kWeightTableActions, kWeightTableContext, name);        

//WEIGHTTABLE
			WeightTableAction *wtActions = NULL;
			for (int i =0; i < actionCount; i++)
				{
				wtActions = new WeightTableAction();
				wtActions->Init(actionIDs[i],GetString(actionNames[i]), GetString(actionNames[i]),
							    GetString(IDS_WEIGHTTABLE), GetString(IDS_WEIGHTTABLE)  );
				pTab->AppendOperation(wtActions);
				}

			GetCOREInterface()->GetActionManager()->RegisterActionContext(kWeightTableContext, name.data());
			return pTab;
			}
			


	};

#endif
