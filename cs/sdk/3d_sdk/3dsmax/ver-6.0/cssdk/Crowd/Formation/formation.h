//*********************************************************************
//	Crowd Formation Bhvr / Unreal Pictures / formation.h
//	Copyright (c) 2000,2001 All Rights Reserved.
//*********************************************************************

#include "bhvr.h"
#include "delegexp.h"



extern HINSTANCE hResource; //from dll.cpp

#define FORMATIONBHVR_CLASS_ID	Class_ID(0x3bb825fb, 0x5b0d64fa)

#define CACHE_SIZE	7 //size of the cache for smoothing out velocity changes..


//The main formation bhvr

class FormationBhvr : public BaseBehavior{
protected:

	//Target parameters.  We calculate these values once each frame at InitAtThisTime
	//We use these values when the delegate is within the 'radius' and needs to move the
	//same way the leader is.
	Point3 leaderPos; 
	Point3 leaderVel;
	Point3 leaderPrevPos;
	float leaderSpeed;
	TimeValue prevPosTime;//the time that the previous pos was set at. We need this
						  //in case time doesn't move forward one frame at a time.


	BOOL editing; //flag that is set to true if within BeginEditParams..EndEditParams block. 
	
public:	

		//enums for paramblock
		typedef enum {name,leader,follower,follower_single,follower_matrix1,follower_matrix2,
			follower_matrix3,follower_matrix4,display_formation,display_scale,
			target_color,display_target,target_scale,
		force_color,display_force} Params;

		//Data
		IParamBlock2 *pblock;
		static ICustButton *iMultPick;

		//constructur/deconstructor
		FormationBhvr();
		~FormationBhvr();

	    // from BaseBehavior
		void   SetName(TCHAR *newname);
		TCHAR *GetName();

		int    CanConvertToMaxScript() {return FALSE;}
		void   GetMaxScriptString(TSTR& str){};

		void SetUpDelegates(INodeTab& participants);
		void InitAtThisTime(TimeValue t);
	
		BEHAVIOR_TYPE BehaviorType(){return FORCE;}
		
		int Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, float BhvrWeight,
			PerformOut &out);
    	
		//Display
		int    Display(TimeValue t, ViewExp *vpt);
		void   GetWorldBoundBox(TimeValue t, ViewExp *vpt, Box3& box);


		//paramblock access functions
		BOOL DisplayFormation(TimeValue t);
		BOOL   DisplayTarget(TimeValue t);
		Color  GetTargetColor(TimeValue t);
		BOOL	DisplayForce(TimeValue t);
		Color GetForceColor(TimeValue t);
		float GetDisplayScale(TimeValue t);
		INode *GetLeader(TimeValue t);
		INode *GetFollower(TimeValue t,int i);
		void DeleteFollower(TimeValue t,int i);
		int GetFollowerCount(TimeValue t);
		Matrix3 GetFollowerMatrix(TimeValue t,int i);
		int GetFollowerMatrixCount(TimeValue t);
		void AppendFollowerMatrix(TimeValue t, Matrix3 & matrix);


		// picking nodes and setting the GUI.
        void   InitDialog(HWND hDlg);
        void   SetLeaderNodeText();
        void   SetFollowerNodeText();
		void   PickWhom();
		
		//actions
		
		void RemoveLeaderFromFormation(TimeValue t);
		void SetFormation(TimeValue t);
		BOOL FindFollowerMatrix(TimeValue t,INode *node,Matrix3 &mat);	

		// from ReferenceTarget, Animatable
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
        void InvalidateUI();
		int NumSubs();  
        Animatable* SubAnim(int i);
        TSTR SubAnimName(int i);
		int	NumParamBlocks();
		IParamBlock2 *GetParamBlock(int i);
		IParamBlock2 *GetParamBlockByID(BlockID id);
 		int NumRefs();
        RefTargetHandle GetReference(int i);
        void SetReference(int i, RefTargetHandle rtarg);
        RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
		RefTargetHandle Clone(RemapDir& remap);
		void GetClassName(TSTR& s);
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return FORMATIONBHVR_CLASS_ID;}		
	
};

