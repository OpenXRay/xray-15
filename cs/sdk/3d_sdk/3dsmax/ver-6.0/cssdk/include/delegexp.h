/******************************************************************************
 *<
	FILE: delegexp.h
				  
	DESCRIPTION:  Export Interface Functionality for Crowd Delegate

	CREATED BY: Susan Amkraut

	HISTORY: created December, 1999

 *>     Copyright (c) Unreal Pictures, Inc. 1999 All Rights Reserved.
 *******************************************************************************/

#ifndef DELEGEXP_H
#define DELEGEXP_H

#ifdef BLD_DELEG
#define DELEGexport __declspec( dllexport )
#else
#define DELEGexport __declspec( dllimport )
#endif

#include "max.h"

// This is the interface ID for a Delegate Interface
#define I_DELEGINTERFACE	0x00100101


// This is the Class ID for Vector Field Objects.
#ifndef BLD_DELEG

#define	DELEG_CLASSID		Class_ID(0x40c07baa, 0x245c7fe6)
#define CROWD_CLASS_ID		Class_ID(0x60144302, 0x43455584) // to avoid selecting a crowd

#endif

// Sample Code, starting with an INode(node)
//
//     Object *o = node->GetObjectRef();
//     if ((o->ClassID() == DELEG_CLASS_ID)
//     {
//
//         // Get the Delegate Export Interface from the node 
//         IDelegate *Iface = (IDelegate *) o->GetInterface(I_DELEGINTERFACE);
//
//         // Get the delegate's average speed at time t
//         float AverageSpeed = Iface->GetAverageSpeed(t);
//     }
//		
//		// Release the interface. NOTE that this function is currently inactive under MAX.
//		//o->ReleaseInterface(I_DELEGINTERFACE,Iface);
// }


// IDelegate: This class can be returned by calling the method GetInterface() from a Delegate node

class IDelegate
{
	public:
		DELEGexport virtual ~IDelegate() {}
		DELEGexport virtual BOOL IsComputing() {return FALSE;}
		DELEGexport virtual BOOL IsConstrainedInZ() {return FALSE;}
		DELEGexport virtual Point3 GetCurrentPosition() {return Point3(0.0,0.0,0.0);}
		DELEGexport virtual Point3 GetCurrentVelocity() {return Point3(0.0,0.0,0.0);}
		DELEGexport virtual Point3 GetPreviousVelocity() {return Point3(0.0,0.0,0.0);}
		DELEGexport virtual float GetCurrentSpeed() {return 1.0;}
		DELEGexport virtual float GetAverageSpeed(TimeValue t) {return 1.0;}
		DELEGexport virtual float GetMaxAccel(TimeValue t){return 1.0;}
		DELEGexport virtual float GetMaxHeadingVel(TimeValue t) {return 1.0;}
		DELEGexport virtual float GetMaxHeadingAccel(TimeValue t){return 1.0;}
		DELEGexport virtual float GetMaxPitchVel(TimeValue t) {return 1.0;}
		DELEGexport virtual float GetMaxPitchAccel(TimeValue t){return 1.0;}
		DELEGexport virtual float GetMaxIncline(TimeValue t){return 1.0;}
		DELEGexport virtual float GetMaxDecline(TimeValue t){return 1.0;}
		DELEGexport virtual Point3 GetSimStartVelocity(INode *n, TimeValue StartTime) {return Point3(0.0,0.0,0.0);}
		DELEGexport virtual int GetIndex() {return 0;}
		DELEGexport virtual BOOL OkToDisplayMyForces() {return 1;}
		DELEGexport virtual BOOL OkToDisplayMyVelocity() {return 1;}
		DELEGexport virtual BOOL OkToDisplayMyCogStates() {return 1;}
        DELEGexport virtual void LineDisplay(Point3& pt1, Point3& pt2, Color clr, BOOL scale) {}
        DELEGexport virtual void BboxDisplay(Point3& pt1, Point3& pt2, Color clr) {}
        DELEGexport virtual void SphereDisplay(Point3& pt1, float radius, Color clr) {}
        DELEGexport virtual void TextDisplay(Point3& pt1, Color clr, TCHAR *str) {}
        DELEGexport virtual BOOL  IsAssignmentActive(int AssignIndex, TimeValue t) {return TRUE;}
		DELEGexport virtual void ClearBacktracking() {}
		DELEGexport virtual BOOL  NeedsBacktracking() {return FALSE;}
		DELEGexport virtual void SetBacktracking(int frame) {}
		DELEGexport virtual int GetRandId() {return 0;}
		DELEGexport virtual Matrix3 GetTM(INode *node,TimeValue t){return Matrix3();}
		DELEGexport virtual BOOL ComputingBiped() {return FALSE;}
		DELEGexport virtual BOOL ReactToMe() {return FALSE;}
};


#endif

