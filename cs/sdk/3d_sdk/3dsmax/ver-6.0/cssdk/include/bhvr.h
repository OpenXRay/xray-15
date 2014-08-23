
//*********************************************************************
//	Crowd / Unreal Pictures / bhvr.h
//	Copyright (c) 1999, All Rights Reserved.
//*********************************************************************

#ifndef BHVR_H
#define BHVR_H

#include "max.h"
#include "iparamm2.h"

#define MAXBHVRNAME	256

// values to set the flag in Perform
#define BHVR_SETS_FORCE	0x00000001
#define BHVR_SETS_GOAL	0x00000002
#define BHVR_SETS_SPEED	0x00000004

#define BEHAVIOR_SUPER_CLASS_ID REF_TARGET_CLASS_ID


// data that is returned by the Perform function
struct PerformOut
{
	Point3 frc;
	Point3 goal;
	float speedwt;
	float speedAtGoalwt;
};

// data that is passed in  by the Constraint function
struct ConstraintInOut
{
	Point3 vel;
	float speed;
	Point3 pos;
	Point3 desGoal;

};

// All behaviors must inherit from this base class
class BaseBehavior: public ReferenceTarget {
    public:
		typedef enum {FORCE=0,CONSTRAINT,ORIENTATION} BEHAVIOR_TYPE;

		BaseBehavior() {}
		SClass_ID SuperClassID() { return BEHAVIOR_SUPER_CLASS_ID; }
		virtual void SetName(TCHAR *newname) {}
		virtual TCHAR *GetName() {return NULL;}
		virtual int Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, float BhvrWeight,
			PerformOut &out) {return FALSE;}
		virtual int CanConvertToMaxScript() {return FALSE;}
		virtual void InitBeforeSim(int FirstFrame, INode *n, int SimStart, int AssignIndex) {}

		// These provide an option to display an apparatus along with the behavior
		// at all times - not just during solve.  The behavior can offer an option
		// to turn this on and off.  Keep in mind this apparatus will be displayed
		// as part of the crowd object, and so may enlarge the damaged rectangle significantly...
		virtual int  Display(TimeValue t, ViewExp *vpt) {return 0;}
		virtual void GetWorldBoundBox(TimeValue t, ViewExp *vpt, Box3& box ) {}

		virtual void BehaviorStarted(TimeValue t, INode *node){}; //called only from cog control(currently) whenever a behavior gets restarted.
		virtual void SetUpDelegates(INodeTab& participants){}; //called at beginning of simulation.
		virtual void InitAtThisTime(TimeValue t){} //called at beginning of each simulation frame

		virtual BEHAVIOR_TYPE BehaviorType() {return FORCE;}
		virtual IsWeightable() 
		        {if (BehaviorType() == BaseBehavior::CONSTRAINT)  return FALSE;
	             if (BehaviorType() == BaseBehavior::ORIENTATION) return FALSE;
				 return TRUE;}
		
		//FOR CONSTRAINTS.
		virtual int Constraint(INode *node, TimeValue t, int numsubsamples, BOOL DesGoalExists,BOOL DisplayHelpers,
			                   BOOL finalSet,ConstraintInOut &inOut) {return FALSE;}

		//FOR ORIENTATIONS
		virtual int Orient(const Point3 &vel,INode *node, TimeValue t,Quat &quat) {return FALSE;}
};

#endif

