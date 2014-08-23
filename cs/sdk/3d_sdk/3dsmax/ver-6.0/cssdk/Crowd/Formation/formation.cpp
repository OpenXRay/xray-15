
//*********************************************************************
//	Crowd / Unreal Pictures / formation.cpp
//	Copyright (c) 2000, All Rights Reserved.
//*********************************************************************

#include "formation.h"
#include "trig.h"
#include "geom.h"
#include "resource.h"
#include "dll.h"
#include "maxicon.h"
#include "notify.h"

#define PBLK		0



/****************************************************************
*																*
*				Local Class and Static Definitions				*
*																*
****************************************************************/

static HIMAGELIST hPickNodeImages = NULL; //image list of pick node images

//static callback function that is called whenever the color changes in the GUI.
static void ColorChangeNotifyProc(void* param, NotifyInfo* pInfo)
{
    ImageList_RemoveAll(hPickNodeImages);
	LoadMAXFileIcon("crwd_pick",  hPickNodeImages, kBackground, FALSE);
}


ICustButton *FormationBhvr::iMultPick = NULL;//static instance of the pick button

static INodeTab SelMaxNodes;  //used to pass nodes in and out from DoHitObjInMax.


//Used for selecting and filtering multiple delegates when setting
//up a formation.

class SelectObjectsInMAX : public HitByNameDlgCallback
{
public:
   BOOL singsel;
   BOOL cds_only;
   INode *leaderNode; //we want to not pick the leader.
   TCHAR *dialogTitle() {return GetString(IDS_SELECT);}
   TCHAR *buttonText() {return GetString(IDS_SELECT);}
   int filter(INode *node);
   void proc(INodeTab &nodeTab);
   BOOL doCustomHilite() {return TRUE;}
   BOOL doHilite(INode *node);
   BOOL singleSelect() {return singsel;}
   void SetSingleSelect(BOOL onoff) {singsel = onoff;}
   void OnlyCrowdDelegates(BOOL onoff) {cds_only = onoff;}

   SelectObjectsInMAX() {singsel = FALSE; cds_only = FALSE;leaderNode = NULL;}
};

static SelectObjectsInMAX DoHitObjInMax;//instance of this structure


// only show these in the dialog
int SelectObjectsInMAX::filter(INode *node)
{
	Object *obj = node->GetObjectRef();
    if (obj->ClassID()!=DELEG_CLASSID) return FALSE;
	return TRUE;

}

// what to do after the dialog is closed and the nodes are selected
void SelectObjectsInMAX::proc(INodeTab &nodeTab)
{
    SelMaxNodes = nodeTab;
}

// which names to hilite in the dialog when it comes up
BOOL SelectObjectsInMAX::doHilite(INode *node)
{
    for (int i=0; i<SelMaxNodes.Count(); i++)
        if (SelMaxNodes[i] == node) return TRUE;
    return FALSE;
}


static SelectObjectsInMAX DoHitObjInMAX; //selection filter when selecting multiple
//objects to form the formation.




//used for validating that a selecting object is an delegate.
class FormationValidatorClass : public PBValidator
{
	private:
		BOOL Validate(PB2Value &v)
		{
	        INode *node = (INode*) v.r;
			Object *obj = node->GetObjectRef();
            if (obj->ClassID() != DELEG_CLASSID) return FALSE;
	       
			return TRUE;
		};
};

//instance of the validator that is used with the parablock desc
static FormationValidatorClass FormationValidator;



//accessor class used by paramdesc to set the button text correctly for the 
//formation button and to add the nodes propertly to the follower list.
class FollowAccessor : public PBAccessor
{
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		FormationBhvr *formbhvr = (FormationBhvr *) owner;
		formbhvr->pblock->ZeroCount(FormationBhvr::follower);
		INode *SurfNode = formbhvr->pblock->GetINode(FormationBhvr::follower_single);
		if (SurfNode) 
		{
		    formbhvr->pblock->Append(FormationBhvr::follower,1,&SurfNode);
		    SurfNode->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
		formbhvr->SetFollowerNodeText();
	}	
};


static FollowAccessor FollowAcc; 


//accessor class used by paramdesc to set the button text correctly for the 
//leader button
class LeaderAccessor : public PBAccessor
{
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		FormationBhvr *formbhvr = (FormationBhvr *) owner;
		formbhvr->SetLeaderNodeText();
	}	

};

static LeaderAccessor LeaderAcc;


//Class Desc
class FormationBhvrClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new FormationBhvr;}
	const TCHAR *	ClassName() {return GetString(IDS_FORMATION_CLASSNAME);}
	SClass_ID		SuperClassID() { return BEHAVIOR_SUPER_CLASS_ID; } 
	Class_ID		ClassID() { return FORMATIONBHVR_CLASS_ID;}
	const TCHAR* 	Category() { return _T("CrowdBehavior");  }	
	void			ResetClassParams(BOOL fileReset) {}
	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("FormationBehavior"); }
	HINSTANCE		HInstance()					{ return hResource; }
	};

FormationBhvrClassDesc FormationClassCD;

ClassDesc* GetFormationBhvrDesc() {return &FormationClassCD;}//return desc for 



//The dialog for the behavior..
class FormationDlgProc : public ParamMap2UserDlgProc 
{
	public:
		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg) 
			{ 
		       case WM_INITDIALOG:	{
			        FormationBhvr *formationBhvr = (FormationBhvr*)map->GetParamBlock()->GetOwner();        	 
					formationBhvr->InitDialog(hWnd);
					} break;

		       case WM_DESTROY:	{
			        FormationBhvr *formationBhvr = (FormationBhvr*)map->GetParamBlock()->GetOwner();        	 
					ReleaseICustButton(formationBhvr->iMultPick); 
					formationBhvr->iMultPick = NULL;
					} break;

		       case WM_COMMAND:
			        switch (LOWORD(wParam)) 
			        {	
						//Select the formation that the delegates are currently in.
						case IDC_SETFORMATION:
							{
								FormationBhvr *formationBhvr = (FormationBhvr*)map->GetParamBlock()->GetOwner();
					     		formationBhvr->SetFormation(t);
							}
							break;
						//Pick multiple followers
					     case IDC_NAMESEL: {
			                  FormationBhvr *formationBhvr = (FormationBhvr*)map->GetParamBlock()->GetOwner();
					          formationBhvr->PickWhom();
			                  formationBhvr->SetFollowerNodeText();
							} break;


			        }				
			}
			return FALSE;
		}
		void DeleteThis() 
		{}
};

static FormationDlgProc  FormationDlgProc; //instance of the dialog


//used for describing an unit sphere that is used to draw the formations...
#define MAXSPHEREPTS	64
#define NUMAROUND		16
static Point3 SpherePts[MAXSPHEREPTS], //originally unit sphere..
			  ScaledPts[MAXSPHEREPTS], //scaled sphere points based upon the displayScale.
			  CurPts[MAXSPHEREPTS];    //the drawn points that are put into world space..
static int numpts = 0;			//how many pts that have been allocated.. used as a check also




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



//static function to set up an sphere array for drawing
static void GetSpherePoints(Point3& ctr, float radius, Point3 *pts)
{
	int i,around;
	float angle;
	static int done = FALSE;
	static float sins[NUMAROUND],coss[NUMAROUND];

    if (!done)
	    for (i=0; i<NUMAROUND; i++)
		{
		    angle = (i / (float) NUMAROUND) * 360.0;
			angle = DegToRad(angle);
			sins[i] = sin(angle); coss[i] = cos(angle);
		}

    // z axis
	i = 0;
	for (around=0; around<NUMAROUND; around++)
	{
		pts[i].x = radius * sins[around] + ctr.x; 
		pts[i].y = radius * coss[around] + ctr.y; 
		pts[i].z = ctr.z;
		i++;
	}
    // y axis
	for (around=0; around<NUMAROUND; around++)
	{
		pts[i].x = radius * sins[around] + ctr.x; 
		pts[i].z = radius * coss[around] + ctr.z; 
		pts[i].y = ctr.y;
		i++;
	}
    // x axis
	for (around=0; around<NUMAROUND; around++)
	{
		pts[i].y = radius * sins[around] + ctr.y; 
		pts[i].z = radius * coss[around] + ctr.z; 
		pts[i].x = ctr.x;
		i++;
	}
	numpts = i;
}



/****************************************************************
*																*
*				ParamBlock Desc									*
*																*
****************************************************************/

static ParamBlockDesc2 formation_pblk (0, _T("{FormationBehaviorParams"), 0, &FormationClassCD, P_AUTO_UI + P_AUTO_CONSTRUCT, 0,
	//rollout
	IDD_FORMATION, IDS_FORMATION_CLASSNAME, 0, 0, &FormationDlgProc,

	FormationBhvr::name,		_T("name"), 	TYPE_STRING,	0,	IDS_NAME,
		end,
	FormationBhvr::leader,	_T(""), 		TYPE_INODE,  P_RESET_DEFAULT | P_NO_AUTO_LABELS, IDS_EMPTY,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_PICKLEADER, 
      	p_accessor,		&LeaderAcc,
		p_prompt,    	IDS_PICKPROMPT,
		end,
	FormationBhvr::follower, _T("formationObjects"), TYPE_INODE_TAB,  0,	P_VARIABLE_SIZE, IDS_OBJECTS,
		end,		
	FormationBhvr::follower_single,	_T(""), 		TYPE_INODE,  P_RESET_DEFAULT | P_NO_AUTO_LABELS, IDS_EMPTY,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_PICKNODE, 
        p_validator,	&FormationValidator,
		p_accessor,		&FollowAcc,
		p_prompt,    	IDS_PICKPROMPT,
		end,

	FormationBhvr::	display_scale,	_T("displayScale"),TYPE_FLOAT, P_RESET_DEFAULT + P_ANIMATABLE, IDS_DISPLAYSCALE,
		p_default, 		5.0,	
		p_range, 		0.001f, 99999999.9f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DISPLAYSCALE, IDC_DISPLAYSCALESPIN, 0.1f, 
		end,


	FormationBhvr::follower_matrix1,	_T("formationMatrices"), TYPE_POINT3_TAB,  0,	P_VARIABLE_SIZE, IDS_OBJECTS,
		end,	
	FormationBhvr::follower_matrix2,	_T("formationMatrices"), TYPE_POINT3_TAB,  0,	P_VARIABLE_SIZE, IDS_OBJECTS,
		end,		
	FormationBhvr::follower_matrix3,	_T("formationMatrices"), TYPE_POINT3_TAB,  0,	P_VARIABLE_SIZE, IDS_OBJECTS,
		end,	
	FormationBhvr::follower_matrix4,	_T("formationMatrices"), TYPE_POINT3_TAB,  0,	P_VARIABLE_SIZE, IDS_OBJECTS,
		end,
	FormationBhvr::display_formation,		_T("displayFormation"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DISPLAYFORMATION, 
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_DISPLAYFORMATION, 
		end,		
	FormationBhvr::target_color,		_T("targetColor"),TYPE_RGBA, P_RESET_DEFAULT, IDS_TARGETCLR, 
		p_default, 		Point3(0.0,0.0,0.5),
		p_ui, 			TYPE_COLORSWATCH, IDC_TARGETCLR,
		end,
	FormationBhvr::display_target,		_T("displayTarget"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DISPLAYTARGET, 
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_TARGETDSPLY, 
		end,
	FormationBhvr::target_scale,		_T("targetScale"),	TYPE_FLOAT, P_RESET_DEFAULT, IDS_TARGETSCL, 
		p_default, 		5.0,	
		p_range, 		0.001f, 100000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_TARGSCALE, IDC_TARGSCALESPIN, 0.1f, 
		end,

	FormationBhvr::force_color,		_T("forceColor"),TYPE_RGBA, P_RESET_DEFAULT, IDS_FORCECLR, 
		p_default, 		Point3(0.0,1.0,1.0),
		p_ui, 			TYPE_COLORSWATCH, IDC_FORCECLR,
		end,
	FormationBhvr::display_force,		_T("displayForce"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DSPLYFRC, 
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_FORCEDSPLY, 
		end,
	end
);



/****************************************************************
*																*
*				class FormationBhvr								*
*																*
****************************************************************/



//Constructur/Deconstructor
FormationBhvr::FormationBhvr()
{
	FormationClassCD.MakeAutoParamBlocks(this);
	SetName(GetString(IDS_FORMATION_NAME));
    pblock->ZeroCount(follower);
	pblock->ZeroCount(follower_matrix1);
	//register the color callback
	RegisterNotification(ColorChangeNotifyProc, NULL, NOTIFY_COLOR_CHANGE);
}

FormationBhvr::~FormationBhvr()
{
	//unregister the color callback
	UnRegisterNotification(ColorChangeNotifyProc, NULL, NOTIFY_COLOR_CHANGE);
}

//From BaseBehavior

void FormationBhvr::SetName(TCHAR *newname) 
{
    pblock->SetValue(name,0,newname);
}

TCHAR *FormationBhvr::GetName()
{
    TCHAR *n;
	pblock->GetValue(name, 0, n, FOREVER);
	return n;
}





//Called before the simulation starts.
//Here we set up the prevPosTime so that when InitAtThisTime is called
//we would have properly set up the targetPrevPos value.
void FormationBhvr::SetUpDelegates(INodeTab& )
{

	prevPosTime = TIME_NegInfinity; //set to negative infinity.
}




//called every frame.  Here we set up the target paramters so that they
//can be used by each node passed into the perform function.
void FormationBhvr::InitAtThisTime(TimeValue t)
{

	INode *leaderNode = GetLeader(t);
	if(leaderNode)
	{
	
		if(prevPosTime==TIME_NegInfinity||prevPosTime != t - GetTicksPerFrame())
		{
			//get the right leaderPrevPos.
			leaderPrevPos = GetCurrentMatrix(leaderNode,t-GetTicksPerFrame()).GetTrans();

		}
		leaderPos = GetCurrentMatrix(leaderNode,t).GetTrans();
		leaderVel = leaderPos - leaderPrevPos;//use the preivous position to get the velocity.

		leaderSpeed = leaderVel.FLength();
		//normalize the velocity.
		if(leaderSpeed!=0.0f)
			leaderVel /= leaderSpeed;
		else
		{
			leaderVel.x = leaderVel.y = leaderVel.z = 0.0f;
		}
		//set up the previous for the next time.
		leaderPrevPos = leaderPos;
		prevPosTime = t;
	}
}


//The main function that is called every tick.  This is where the behavior
//sets the force,goal and speed.

int FormationBhvr::Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, float BhvrWeight,
			PerformOut &out)
{
	Object *o = node->GetObjectRef();
    if (o->ClassID() != DELEG_CLASSID) return FALSE; // this should never happen
    IDelegate *IDeleg = (IDelegate *) o->GetInterface(I_DELEGINTERFACE);
	Point3 vel= IDeleg->GetCurrentVelocity();

	Point3 pos = IDeleg->GetCurrentPosition();

	INode *leader = GetLeader(t);
	if(leader==NULL)
		return 0;

	
	Matrix3 formationMat;
	//Get the local formation matrix and check

	if(FindFollowerMatrix(t,node,formationMat)==FALSE)
		return 0; //that node doesn't exist in the formation so exit.

	//returned values.
	Point3 frc,goal;
	float speedwt, speedAtGoalwt;

	//Find the Formation Position Target in World Space.

 	Matrix3 currentLeaderMat = GetCurrentMatrix(leader,t);
	currentLeaderMat.NoScale();
	Matrix3 worldSpace = formationMat*currentLeaderMat;
	Point3 target =  worldSpace.GetTrans();

	//set the goal as the target
	goal = target;

	//set the force as the direction to move towards the target
	frc =  goal - pos;

	float length = frc.FLength();

	if(length!=0.0f) //we are not at the goal
	{
		frc  /=length;
		
		//set up the leader Vector
		Point3 leaderVec = leaderVel*leaderSpeed;

	
	
		//If the  target is behind you but moving towards you don't turn around to
		//go toward it.  Instead move in the direction of the leader.
		if(frc%vel<0.0f &&  //if you are behind it
			length/IDeleg->GetAverageSpeed(t)<10 && //AND less than 20 frames
			vel%leaderVel>0.0f) //AND it is going toward you.
		{
			
			//set frc as leader's Velocity.
			frc = leaderVel;

			//move at half the leaderSpeed.  Leader will still catch
			//up to you and you'll have some speed when it does.
			
			//set the speewt
			speedwt = (leaderSpeed*0.5f)/IDeleg->GetAverageSpeed(t);
		}
		else //we should just move towards the target.
		{
			//We need to find the speed to be it.  We do this by
			//finding the time we will intersect our target based
			//upon the leader's velocity and our own velocity.

			vel -= leaderVec;
			
			//find time to intersect..
			float newSpeed = vel.FLength();
			float timeToIntersect = length/newSpeed;

			//from that time.. figure out what it speed should be..
			newSpeed = timeToIntersect * IDeleg->GetMaxAccel(t);
			if(newSpeed>IDeleg->GetAverageSpeed(t))
				newSpeed = IDeleg->GetAverageSpeed(t);


			//check to see if we will move past the goal.. if so
			//move the goal along the leader vec.  This reduces overshooting
			//the goal and decreases wobbling.
			if((newSpeed+leaderSpeed)>length) 	
			{
				goal += leaderVel*(leaderSpeed);  
				frc = goal -pos;
				float newLength = frc.FLength();

				if(newLength!=0.0f) //check for if zero to avoid divide by zero..
				{
					frc /= newLength;
					speedwt = (newSpeed+leaderSpeed)/IDeleg->GetAverageSpeed(t);
										
				}
				else
				{
					speedwt = 0.0f;
				}

			}
			else //far enough away.. leave the ole frc value..
				speedwt = (newSpeed+leaderSpeed)/IDeleg->GetAverageSpeed(t);
			
		}
	}
	else  //we are at the goal
	{
		frc.x = frc.y = frc.z = 0.0f;
	
		//set the speedwt to be the leaderSpeed
		speedwt = leaderSpeed/IDeleg->GetAverageSpeed(t);
	}

	frc *= BhvrWeight *IDeleg->GetAverageSpeed(t); //scale by weight and average speed..

	//set the speedWtAtGoal.. We want it to be the speed of the leader
	speedAtGoalwt = leaderSpeed/IDeleg->GetAverageSpeed(t);

	//Display Any Helpers.
	if (DisplayHelpers && IDeleg->OkToDisplayMyForces())
	{
		if (DisplayTarget(t)) IDeleg->SphereDisplay(goal,pblock->GetFloat(target_scale),GetTargetColor(t));
		if (DisplayForce(t))  IDeleg->LineDisplay(IDeleg->GetCurrentPosition(),IDeleg->GetCurrentPosition()+frc,GetForceColor(t),TRUE);
	}

	//set up the out structure..
	out.frc = frc;
	out.goal = goal;
	out.speedwt = speedwt;
	out.speedAtGoalwt = speedAtGoalwt;

	return BHVR_SETS_FORCE | BHVR_SETS_GOAL | BHVR_SETS_SPEED;
}

//Get a bounding box.  This is used for when we display the formation continously.
void FormationBhvr::GetWorldBoundBox(TimeValue t, ViewExp *vpt, Box3& box)
{

	//make sure we have everything we need...
	if(DisplayFormation(t)==FALSE) return;
	if(GetFollowerCount(t)<=0) return;
	if(GetFollowerMatrixCount(t)<=0) return; //possible to not have this set when the follower is set..

	INode *leaderNode;
	leaderNode = GetLeader(t);
	if(leaderNode==NULL) return;



	//for each follower we need to increase the bounding box by it's
	//world position location...
	for(int i =0;i<GetFollowerCount(t);i++)
	{
		if(GetFollower(t,i)) //if we have a a node...
		{
			Matrix3 worldSpace = GetFollowerMatrix(t,i)*GetCurrentMatrix(leaderNode,t);
			Point3 trans(worldSpace.GetTrans());
			//expand the box by the worldposition...
			box += trans;
		}
	}
}

//The display function that is used to display the formation.
int FormationBhvr::Display(TimeValue t, ViewExp *vpt) 
{
    // setup

	int i,j;
   	
	if(DisplayFormation(t)==FALSE) return FALSE;

	if(GetFollowerCount(t)<=0) return FALSE;
	if(GetFollowerMatrixCount(t)<=0) return FALSE;

	INode *leaderNode;
	leaderNode = GetLeader(t);
	if(leaderNode==NULL) return FALSE;
	
	
	//check tgo see if we have created a default sphere for drawing yet...
    //if we haven't then create it..
	if (numpts == 0) 
		GetSpherePoints(Point3(0.0f,0.0f,0.0f), 1.0, SpherePts);

	
	GraphicsWindow *gw = vpt->getGW();
   
	//set the identity matrix...
	Matrix3 idMat;
	idMat.IdentityMatrix();
    gw->setTransform(idMat);

	gw->setColor(LINE_COLOR,.815f,.976f,1.0f);

	float scaleRadius = GetDisplayScale(t);
	
	//set the drawing radius values based upon what the radius size is.
	for (i=0; i<NUMAROUND * 3; i++) ScaledPts[i] = ((SpherePts[i] * scaleRadius));
	 
	//for each follower we need to increase the bounding box by it's
	//world position location...
	for(i =0;i<GetFollowerCount(t);i++)
	{
		INode *followerNode = GetFollower(t,i);
		if(followerNode) //if we have a a node...
		{
	
			Matrix3 leaderMat = GetCurrentMatrix(leaderNode,t);
			leaderMat.NoScale();
			Matrix3 followerMat = GetFollowerMatrix(t,i);
			Matrix3 worldSpace = followerMat *leaderMat;
			for (j=0; j<NUMAROUND * 3; j++) CurPts[j] = worldSpace*ScaledPts[j]; //adding the center to the point positions
			
		   	gw->polyline(NUMAROUND,&CurPts[0],NULL,NULL,TRUE,NULL);
		 	gw->polyline(NUMAROUND,&CurPts[NUMAROUND],NULL,NULL,TRUE,NULL);
    		gw->polyline(NUMAROUND,&CurPts[NUMAROUND * 2],NULL,NULL,TRUE,NULL);

		
		}
	}

	return TRUE;
}



//paramblock access functions

BOOL FormationBhvr::DisplayFormation(TimeValue t)
{
    BOOL b;
	pblock->GetValue(display_formation, t, b, FOREVER);
	return b;
}

float FormationBhvr::GetDisplayScale(TimeValue t)
{
	float val;
	pblock->GetValue(display_scale, t, val, FOREVER);
	return val;

}

BOOL FormationBhvr::DisplayTarget(TimeValue t)
{
    BOOL b;
	pblock->GetValue(display_target, t, b, FOREVER);
	return b;
}


Color FormationBhvr::GetTargetColor(TimeValue t)
{
    Color clr;
	pblock->GetValue(target_color, t, clr, FOREVER);
	return clr;
}

BOOL FormationBhvr::DisplayForce(TimeValue t)
{
    BOOL b;
	pblock->GetValue(display_force, t, b, FOREVER);
	return b;
}

Color FormationBhvr::GetForceColor(TimeValue t)
{
    Color clr;
	pblock->GetValue(force_color, t, clr, FOREVER);
	return clr;
}

//The leader which all delegate's will follow..
INode *FormationBhvr::GetLeader(TimeValue t)
{
	INode *n;
	pblock->GetValue(leader,t,n,FOREVER);
	return n;

}

//Get the follower node
INode *FormationBhvr::GetFollower(TimeValue t,int which)
{
    if(which<0||which>=GetFollowerCount(t))
		return NULL;

	INode *n;
	pblock->GetValue(follower,t,n,FOREVER,which);
	return n;
}


//Remove the follower node
void FormationBhvr::DeleteFollower(TimeValue t,int which)
{
    if(which<0||which>=GetFollowerCount(t))
		return;
	pblock->Delete(follower,which,1);
}

//the number of followers.
int FormationBhvr::GetFollowerCount(TimeValue t)
{
	return pblock->Count(follower);
}




Matrix3 FormationBhvr::GetFollowerMatrix(TimeValue t,int which)
{
	//we are using 4 Point3's to represent the matrix..
	Point3 point;
	Matrix3 matrix;
	
	pblock->GetValue(follower_matrix1,	0,point,FOREVER,which);
	matrix.SetRow(0,point);
	
	pblock->GetValue(follower_matrix2,	0,point,FOREVER,which);
	matrix.SetRow(1,point);

	
	pblock->GetValue(follower_matrix3,	0,point,FOREVER,which);
	matrix.SetRow(2,point);
	
	pblock->GetValue(follower_matrix4,	0,point,FOREVER,which);
	matrix.SetRow(3,point);

	matrix.ValidateFlags();
	return matrix;

	//Note that when this code was written that the Matrix3 paramblock
	//parameter wasn't working correctly in R4 at the time this was written.
	/*
    if(which<0||which>=GetFollowerCount(t))
		return NULL;

	Matrix3 mat;
	pblock->GetValue(follower_matrix,t,mat,FOREVER,which);
	return mat;
	*/
}

int FormationBhvr::GetFollowerMatrixCount(TimeValue t)
{
	return pblock->Count(follower_matrix1);
}

void FormationBhvr::AppendFollowerMatrix(TimeValue t,Matrix3 &mat)
{
	Point3 *points;
	points = new Point3;
	*points = mat.GetRow(0);
	pblock->Append(follower_matrix1,1,&points,1);
	*points = mat.GetRow(1);
	pblock->Append(follower_matrix2,1,&points,1);
	*points = mat.GetRow(2);
	pblock->Append(follower_matrix3,1,&points,1);	
	*points = mat.GetRow(3);
	pblock->Append(follower_matrix4,1,&points,1);
	delete points;
}


// picking nodes and setting the GUI

     
void FormationBhvr::InitDialog(HWND hDlg)
{

	SetLeaderNodeText();
    SetFollowerNodeText();
	if (!hPickNodeImages ) 
	{
		hPickNodeImages = ImageList_Create(16, 15, ILC_COLOR16 | ILC_MASK, 2, 0);
		BOOL ret = LoadMAXFileIcon("crwd_pick", hPickNodeImages, kBackground, FALSE);

	}
	iMultPick = GetICustButton(GetDlgItem(hDlg,IDC_NAMESEL));
	iMultPick->SetType(CBT_PUSH);
	iMultPick->SetImage(hPickNodeImages,0,0,1,1,16,15);
	iMultPick->SetTooltip(TRUE,(LPSTR)GetString(IDS_MULTSEL));
}

void FormationBhvr::SetLeaderNodeText()
{

	if (!editing) return;
    IParamMap2 *pmap = pblock->GetMap();
	if (!pmap) return;
	INode *node;
    node = pblock->GetINode(leader,0,0);
	if(node)
		pmap->SetText(leader,node->GetName());
	else
		pmap->SetText(leader,GetString(IDS_NONE));
}

void FormationBhvr::SetFollowerNodeText()
{
	if (!editing) return;
    IParamMap2 *pmap = pblock->GetMap();
	if (!pmap) return;
	if (pblock->Count(follower) == 0)
         pmap->SetText(follower_single,GetString(IDS_NONE));
	else if (pblock->Count(follower) == 1)
	     {
	          INode *node;
              node = pblock->GetINode(follower,0,0);
    	      if (node) pmap->SetText(follower_single,node->GetName());
         }
	     else pmap->SetText(follower_single,GetString(IDS_MULTIPLE));
}
 



void FormationBhvr::PickWhom()
{
   int i;

	// first set the nodetab to equal the existing nodes to flee
	// so that they are selected in the dialog
	if (pblock->Count(follower) > 0)
	{
	    INode *node;
		SelMaxNodes.Resize(pblock->Count(follower));
		SelMaxNodes.SetCount(0);
		for (i=0; i<pblock->Count(follower); i++)
		{
		    pblock->GetValue(follower,0,node,FOREVER,i);
			SelMaxNodes.Append(1,&node);
		}
	}
	else SelMaxNodes.ZeroCount(); 

	// let the user pick
	DoHitObjInMAX.SetSingleSelect(FALSE); // allow multiple selection
	if (!GetCOREInterface()->DoHitByNameDialog(&DoHitObjInMAX)) return;

    // Set follower to the returned nodes
	theHold.Begin();
	pblock->Resize(follower,SelMaxNodes.Count());
	pblock->SetCount(follower,0);
	for (i=0; i<SelMaxNodes.Count(); i++)
		pblock->Append(follower,1,&SelMaxNodes[i]);


	//zero out the formation matrix that's used for saving it out.
	pblock->ZeroCount(follower_matrix1);
	pblock->ZeroCount(follower_matrix2);
	pblock->ZeroCount(follower_matrix3);
	pblock->ZeroCount(follower_matrix4);

    theHold.Accept(GetString(IDS_UN_WHOM));
}


//actions
//make sure the leader isn't in the formation itself.
void FormationBhvr::RemoveLeaderFromFormation(TimeValue t)
{
	
	INode *leader =  GetLeader(t);
	int numDelegates = GetFollowerCount(t);	
	//we count backwards so we can simply remove it.
	for(int i=numDelegates-1;i>-1;--i)
	{
		if(GetFollower(t,i)==leader)
		{
			DeleteFollower(t,i);
			break;//only one
		}
	}

}

//This will set the formation
void FormationBhvr::SetFormation(TimeValue t)
{
	INode *node;
	Matrix3 tempMatrix;


	//Make sure that the leader is not part of the follower array..
	RemoveLeaderFromFormation(t);


	INode *leader =  GetLeader(t);

	if(leader==NULL)
		return;
	Matrix3 leaderPosition = GetCurrentMatrix(leader,t);
	leaderPosition.NoScale(); //kill any scale if we have it 
	leaderPosition.Invert();  //it's inverted...


	int numDelegates = GetFollowerCount(t);	

	//zero out the formation matrix that's used for saving it out.
	pblock->ZeroCount(follower_matrix1);
	pblock->ZeroCount(follower_matrix2);
	pblock->ZeroCount(follower_matrix3);
	pblock->ZeroCount(follower_matrix4);

	for(int i =0;i<numDelegates;i++)
	{

		node = GetFollower(t,i);
		if(node)
		{
			tempMatrix = GetCurrentMatrix(node,t);
			tempMatrix.NoScale();
			Matrix3 leaderMat =tempMatrix*leaderPosition;
			AppendFollowerMatrix(t,leaderMat);

			//killed because matrix3 wasn't working ...pblock->Append(follower_matrix,1,&leaderMat);
		 
		
		}
		else
		{
			//we still set up follower_matrix so that the counts
			//of the follower_matrix tab and the follower tab are equal.
			tempMatrix.IdentityMatrix();
			AppendFollowerMatrix(t,tempMatrix);

			//pblock->Append(follower_matrix,1,&tempMat);
	
		}
	}

}


//Given a node find the local formation matrix for it.
//If the node isn't in the formation, then return a FALSE.
BOOL FormationBhvr::FindFollowerMatrix(TimeValue t,INode *node,Matrix3 &mat)
{
	//linear search through the nodes. We could get fancy
	//and sort the node list and do a binary search,or create a hash table
	//but we aren't :)

	int followerCount = GetFollowerCount(t);
	if(followerCount==0||GetFollowerMatrixCount(t)==0)
		return FALSE;
	int i;
	for(i=0;i<followerCount;i++)
	{
		if(GetFollower(t,i)==node)
			break;
	}

	if(i!=followerCount) //we found it.
	{
		mat = GetFollowerMatrix(t,i);
		return TRUE;
	}
	return FALSE;
}


// from ReferenceTarget, Animatable
void FormationBhvr::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	//need to set up this param on a object by object basis.
	//formation_pblk.ParamOption(follower,p_validator,&validator);
	editing = TRUE;
	FormationClassCD.BeginEditParams(ip, this, flags, prev);
}

void FormationBhvr::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	editing = FALSE;
	FormationClassCD.EndEditParams(ip, this, flags, next);
}

void FormationBhvr::InvalidateUI()
{
	formation_pblk.InvalidateUI(pblock->LastNotifyParamID());
}

RefTargetHandle FormationBhvr::Clone(RemapDir& remap) 
{
	FormationBhvr* newformation = new FormationBhvr();	
	newformation->ReplaceReference(PBLK,pblock->Clone(remap));

	return(newformation);
}

int FormationBhvr::NumSubs() {return 1;}
int	FormationBhvr::NumParamBlocks() {return 1;}
int FormationBhvr::NumRefs() {return 1;}

Animatable* FormationBhvr::SubAnim(int i) 	
{
    if (i==0) return pblock; else return NULL;
}

TSTR FormationBhvr::SubAnimName(int i) 
{
	switch (i) 
	{
		case 0: return GetString(IDS_PARAMETERS);
		default: return _T("");
	}
}

IParamBlock2 *FormationBhvr::GetParamBlock(int i)
{
    if (i==0) return pblock; else return NULL;
}

IParamBlock2 *FormationBhvr::GetParamBlockByID(BlockID id)
{ 
    if (pblock->ID() == id) return pblock;
	else return NULL;
}

RefTargetHandle FormationBhvr::GetReference(int i)
{
	if(i==PBLK)
	{
		return(RefTargetHandle)pblock;
	}

	return NULL;

}

void FormationBhvr::SetReference(int i, RefTargetHandle rtarg)
{
	if(i==PBLK)
	{
		pblock=(IParamBlock2*)rtarg; 
		return;
	}
}

RefResult FormationBhvr::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message) 
{

	switch (message) 
	{		
	
		case REFMSG_CHANGE:
			//something changed perhaps a name?.. reset the text.
			SetFollowerNodeText();
			SetLeaderNodeText();
			formation_pblk.InvalidateUI();	
		break;

		case REFMSG_TAB_ELEMENT_NULLED:
			 if (hTarget == pblock)
			 {
				 //was is something in the formation?
				 if (pblock->LastNotifyParamID() == follower)
				 {
					 ///okay somebody in the formation was deleted..
	                 INode *n;
                 	 int i;
                 	 int start = 0;
                 	 while (TRUE)
                 	 {
         	             for(i=start; i<pblock->Count(follower); i++)
                 	     {
                 	    	 pblock->GetValue(follower, 0, n, FOREVER, i);
         	            	 if(!n) 
							 { 
								 //This one was NULLed so remove from Tabs.
								 pblock->Delete(follower,i,1); 
								 pblock->Delete(follower_matrix1,i,1); 
								 pblock->Delete(follower_matrix2,i,1); 
								 pblock->Delete(follower_matrix3,i,1); 
								 pblock->Delete(follower_matrix4,i,1); 
								 
								 start = i; 
								 break;
							 }

         		         }
                 		 if (i >= pblock->Count(follower)) break;
                 	 }
				 }
				
			}		

			 //set up the button text due to possible changes.
			 SetFollowerNodeText();
			 SetLeaderNodeText();
			 formation_pblk.InvalidateUI();	
		 break;

	
	}
	return REF_SUCCEED;
}


void FormationBhvr::GetClassName(TSTR& s) {s = GetString(IDS_FORMATION_CLASSNAME);}








