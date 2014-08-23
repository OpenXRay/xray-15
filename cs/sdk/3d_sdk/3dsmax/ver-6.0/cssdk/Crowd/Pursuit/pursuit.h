//*********************************************************************
//	Crowd Pursuit Bhvr / Unreal Pictures / pursuit.h
//	Copyright (c) 2000,2001 All Rights Reserved.
//*********************************************************************

#include "bhvr.h"
#include "delegexp.h"


extern HINSTANCE hResource; //from dll.cpp

#define PURSUITBHVR_CLASS_ID	Class_ID(0x640373e9, 0x380c6813)


//The main pursuit bhvr
class PursuitBhvr : public BaseBehavior{
	private:

	//Target parameters.  We calculate these values once each frame at InitAtThisTime
	//The values are then used from within Perform for each delegate.
	Point3 targetPos; 
	Point3 targetVel;
	Point3 targetPrevPos;
	float targetSpeed;

	TimeValue prevPosTime;//the time that the previous pos was set at. We need this
						  //in case time doesn't move forward one frame at a time.
						  //(Due to backtracking)

	public:	

		//enums for paramblock
		typedef enum {name=0,whom,whom_single,
			target_color,display_target,target_scale,
		force_color,display_force} Params;

		//Data
		IParamBlock2 *pblock;

		//constructur/deconstructor
		PursuitBhvr();
		~PursuitBhvr();

	    // from BaseBehavior
		void   SetName(TCHAR *newname);
		TCHAR *GetName();

		void SetUpDelegates(INodeTab& participants); //called at beginning of each run.. used to set up prevPosTime
		void InitAtThisTime(TimeValue t); //called at beginning of each time tick..used to set up targetPos..

		BEHAVIOR_TYPE BehaviorType(){return FORCE;}
		int Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, float BhvrWeight,
			PerformOut &out);
   
	
		//actions
		//this finds a goal to go to based upon an objects position,velocity and speed
		//and a targets position,velocity and speed.
		Point3 FindGoal(Point3 &pos,Point3 &vel,float Speed, Point3 &targetPos,Point3 &targetVel,float targetSpeed);

		//paramblock access functions
		BOOL   DisplayTarget(TimeValue t);
		Color  GetTargetColor(TimeValue t);
		BOOL	DisplayForce(TimeValue t);
		Color GetForceColor(TimeValue t);

		INode *GetWhom(TimeValue t);
	

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
		Class_ID ClassID() { return PURSUITBHVR_CLASS_ID;}		
	
};

