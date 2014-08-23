
//*********************************************************************
//	Crowd / Unreal Pictures / pursuit.cpp
//	Copyright (c) 2000,2001 All Rights Reserved.
//*********************************************************************

#include "pursuit.h"
#include "trig.h"
#include "geom.h"
#include "resource.h"
#include "dll.h"
#include "BipExp.h"
#define PBLK		0



/****************************************************************
*																*
*				Local Class and Static Definitions				*
*																*
****************************************************************/


//Class Desc
class PursuitBhvrClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PursuitBhvr;}
	const TCHAR *	ClassName() {return GetString(IDS_PURSUIT_CLASSNAME);}
	SClass_ID		SuperClassID() { return BEHAVIOR_SUPER_CLASS_ID; } 
	Class_ID		ClassID() { return PURSUITBHVR_CLASS_ID;}
	const TCHAR* 	Category() { return _T("CrowdBehavior");  }	
	void			ResetClassParams(BOOL fileReset) {}
	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("PursuitBehavior"); }
	HINSTANCE		HInstance()					{ return hResource; }
	};

PursuitBhvrClassDesc PursuitClassCD;

ClassDesc* GetPursuitBhvrDesc() {return &PursuitClassCD;}//return desc for 



/****************************************************************
*																*
*				Local Static Functions							*
*																*
****************************************************************/


//Get the current position for the purpose of computing the next frame
//This function checks to see whether or not the node is a delegate, in which
//case the GetTM function  is called.
static Matrix3 GetCurrentMatrix(INode *node, TimeValue t)
{
	Object *o = node->GetObjectRef();
    if (o->ClassID() == DELEG_CLASSID) 
	{
        IDelegate *cd = (IDelegate *) o->GetInterface(I_DELEGINTERFACE);
		if (cd->IsComputing())
			return cd->GetTM(node,t);
	}

    return node->GetNodeTM(t);
}



/****************************************************************
*																*
*				ParamBlock Desc									*
*																*
****************************************************************/

static ParamBlockDesc2 pursuit_pblk (0, _T("{PursuitBehaviorParams"), 0, &PursuitClassCD, P_AUTO_UI + P_AUTO_CONSTRUCT, 0,
	//rollout
	IDD_PURSUIT, IDS_PURSUIT_CLASSNAME, 0, 0, NULL,

	PursuitBhvr::name,		_T("name"), 	TYPE_STRING,	0,	IDS_NAME,
	end,

	PursuitBhvr::whom_single,	_T("target"), 	TYPE_INODE,  P_RESET_DEFAULT,IDS_PURSUITTARGET, 
	p_ui, 			TYPE_PICKNODEBUTTON, IDC_PICKNODE, 
		p_prompt,    	IDS_PICKPROMPT,
		end,
		
	PursuitBhvr::target_color,		_T("targetColor"),TYPE_RGBA, P_RESET_DEFAULT, IDS_TARGETCLR, 
		p_default, 		Point3(0.0,0.0,0.5),
		p_ui, 			TYPE_COLORSWATCH, IDC_TARGETCLR,
		end,
	PursuitBhvr::display_target,		_T("displayTarget"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DISPLAYTARGET, 
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_TARGETDSPLY, 
		end,
	PursuitBhvr::target_scale,		_T("targetScale"),	TYPE_FLOAT, P_RESET_DEFAULT, IDS_TARGETSCL, 
		p_default, 		5.0,	
		p_range, 		0.001f, 100000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_TARGSCALE, IDC_TARGSCALESPIN, 0.1f, 
		end,

	PursuitBhvr::force_color,		_T("forceColor"),TYPE_RGBA, P_RESET_DEFAULT, IDS_FORCECLR, 
		p_default, 		Point3(0.0,1.0,1.0),
		p_ui, 			TYPE_COLORSWATCH, IDC_FORCECLR,
		end,
	PursuitBhvr::display_force,		_T("displayForce"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DSPLYFRC, 
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_FORCEDSPLY, 
		end,
	end
);



/****************************************************************
*																*
*				class PursuitBhvr								*
*																*
****************************************************************/



//Constructur/Deconstructor
PursuitBhvr::PursuitBhvr()
{
	PursuitClassCD.MakeAutoParamBlocks(this);
	SetName(GetString(IDS_PURSUIT));
    
}

PursuitBhvr::~PursuitBhvr()
{
}

//From BaseBehavior

void PursuitBhvr::SetName(TCHAR *newname) 
{
    pblock->SetValue(name,0,newname);
}

TCHAR *PursuitBhvr::GetName()
{
    TCHAR *n;
	pblock->GetValue(name, 0, n, FOREVER);
	return n;
}



//Called before the simulation starts.
//Here we set up the prevPosTime so that when InitAtThisTime is called
//we would have properly set up the targetPrevPos value.
void PursuitBhvr:: SetUpDelegates(INodeTab&)
{
	prevPosTime = TIME_NegInfinity; //set to negative infinity.
}


//called every frame.  Here we set up the target paramters so that they
//can be used by each node passed into the perform function.
void PursuitBhvr::InitAtThisTime(TimeValue t)
{

	INode *pursuitTarget = GetWhom(t);
	if(pursuitTarget)
	{
	
		if(prevPosTime==TIME_NegInfinity||prevPosTime != t - GetTicksPerFrame())
		{
			//get the right targetPrevPos.
			targetPrevPos = GetCurrentMatrix(pursuitTarget,t-GetTicksPerFrame()).GetTrans();

		}
		targetPos = GetCurrentMatrix(pursuitTarget,t).GetTrans();
		targetVel = targetPos - targetPrevPos;//use the preivous position to get the velocity.

		targetSpeed = targetVel.FLength();
		//normalize the velocity.
		if(targetSpeed!=0.0f)
			targetVel /= targetSpeed;
		else
		{
			targetVel.x = targetVel.y = targetVel.z = 0.0f;
		}
		//set up the previous for the next time.
		targetPrevPos = targetPos;
		prevPosTime = t;
	}
}


//The main function that is called every tick.  This is where the behavior
//sets the force,goal and speed.
int PursuitBhvr::Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, float BhvrWeight,
			PerformOut &out)
{
	INode *pursuitTarget = GetWhom(t);
	if(pursuitTarget==NULL||pursuitTarget==node)
		return FALSE; //okay we don't have anybody to pursue or the target is the node..so exit out..


	//get the delegate and it's position and velocity.
	Object *o = node->GetObjectRef();
    if (o->ClassID() != DELEG_CLASSID) return FALSE; // this should never happen
    IDelegate *IDeleg = (IDelegate *) o->GetInterface(I_DELEGINTERFACE);
	Point3 pos = IDeleg->GetCurrentPosition();
	Point3 vel= IDeleg->GetCurrentVelocity();
	vel = vel.FNormalize();

	float speed = IDeleg->GetCurrentSpeed();

	Point3 goal,frc;


	//given the pursuer and the target find a target.

	goal = FindGoal(pos,vel,speed,targetPos,targetVel,targetSpeed);
		

	//set the frc to point in the direction of the goal.
	frc =  goal - pos;

	//normalize the frc.
	frc = frc.FNormalize();

	//scale the frc by the Behavior weight and the Delegate's average speed.
	frc *= BhvrWeight *IDeleg->GetAverageSpeed(t);


	if (DisplayHelpers && IDeleg->OkToDisplayMyForces())
	{
		if (DisplayTarget(t)) IDeleg->SphereDisplay(goal,pblock->GetFloat(target_scale),GetTargetColor(t));
		if (DisplayForce(t))  IDeleg->LineDisplay(IDeleg->GetCurrentPosition(),IDeleg->GetCurrentPosition()+frc,GetForceColor(t),TRUE);
	}

	out.frc = frc;
	out.goal = goal;

	return BHVR_SETS_FORCE | BHVR_SETS_GOAL;

}



//actions and helpers
//return a goal to go to given the pursuer and target info.
Point3 PursuitBhvr::FindGoal(Point3 &pos,Point3 &vel,float speed, Point3 &targetPos,Point3 &targetVel,float targetSpeed)
{
	//Based on the distance between the two, and their respective speeds, we find the maximum distance
	
	Point3 diff = targetPos - pos;
	float distance = diff.FLength();
	float timeToReach = 1.0f;
	//if the speed is less than the distance..
	if(speed<distance)
	{
		//find how long it would take to reach the target.
		if(speed!=0)
			timeToReach = distance/speed;
		else
			timeToReach = 0.0f; //not moving just put it where it should go..

		//get the dotProduct of the velocities..
		float dotProduct = vel%targetVel;

		//scale the timeToReach by the dotProduct value..
		//if dotProduct is negative(so that they are going in opposite directions)
		//flip it's sign and scale it so that it has more influence.
		if(dotProduct<0.0)
			dotProduct *= -2.0f;

		//scale the time by the dot product
		timeToReach *= dotProduct;

	}
	//else if speed is greater than the distance then just move to targetPos + targetVel*targetSpeed
	
	return(targetPos + targetVel*targetSpeed*timeToReach);
}



//paramblock access functions

BOOL PursuitBhvr::DisplayTarget(TimeValue t)
{
    BOOL b;
	pblock->GetValue(display_target, t, b, FOREVER);
	return b;
}


Color PursuitBhvr::GetTargetColor(TimeValue t)
{
    Color clr;
	pblock->GetValue(target_color, t, clr, FOREVER);
	return clr;
}

BOOL PursuitBhvr::DisplayForce(TimeValue t)
{
    BOOL b;
	pblock->GetValue(display_force, t, b, FOREVER);
	return b;
}

Color PursuitBhvr::GetForceColor(TimeValue t)
{
    Color clr;
	pblock->GetValue(force_color, t, clr, FOREVER);
	return clr;
}

//Get the whom node
INode *PursuitBhvr::GetWhom(TimeValue t)
{
	INode *n;
	pblock->GetValue(whom_single,t,n,FOREVER);
	return n;
}


// from ReferenceTarget, Animatable
void PursuitBhvr::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	//need to set up this param on a object by object basis.
	//pursuit_pblk.ParamOption(whom,p_validator,&validator);
	PursuitClassCD.BeginEditParams(ip, this, flags, prev);
}

void PursuitBhvr::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PursuitClassCD.EndEditParams(ip, this, flags, next);
}

void PursuitBhvr::InvalidateUI()
{
	pursuit_pblk.InvalidateUI(pblock->LastNotifyParamID());
}

RefTargetHandle PursuitBhvr::Clone(RemapDir& remap) 
{
	PursuitBhvr* newpursuit = new PursuitBhvr();	
	newpursuit->ReplaceReference(PBLK,pblock->Clone(remap));

	return(newpursuit);
}

int PursuitBhvr::NumSubs() {return 1;}
int	PursuitBhvr::NumParamBlocks() {return 1;}
int PursuitBhvr::NumRefs() {return 1;}

Animatable* PursuitBhvr::SubAnim(int i) 	
{
    if (i==0) return pblock; else return NULL;
}

TSTR PursuitBhvr::SubAnimName(int i) 
{
	switch (i) 
	{
		case 0: return GetString(IDS_PARAMETERS);
		default: return _T("");
	}
}

IParamBlock2 *PursuitBhvr::GetParamBlock(int i)
{
    if (i==0) return pblock; else return NULL;
}

IParamBlock2 *PursuitBhvr::GetParamBlockByID(BlockID id)
{ 
    if (pblock->ID() == id) return pblock;
	else return NULL;
}

RefTargetHandle PursuitBhvr::GetReference(int i)
{
	if(i==PBLK)
	{
		return(RefTargetHandle)pblock;
	}

	return NULL;

}

void PursuitBhvr::SetReference(int i, RefTargetHandle rtarg)
{
	if(i==PBLK)
	{
		pblock=(IParamBlock2*)rtarg; 
		return;
	}
}

RefResult PursuitBhvr::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message) 
{

	switch (message) 
	{		
	
		case REFMSG_CHANGE:
			//something changed perhaps a name?.. reset the text.
			pursuit_pblk.InvalidateUI();	
		break;

		
	}
	return REF_SUCCEED;
}




void PursuitBhvr::GetClassName(TSTR& s) {s = GetString(IDS_PURSUIT_CLASSNAME);}





