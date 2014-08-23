/**********************************************************************
 *<
	FILE: SplineIKSolver.cpp

	DESCRIPTION: 
	
	This is a new (and the 4th) IK Solver. The Spline IK Solver will help animators 
	animate objects such as tails, spines, worms, long necks, tentacles, whips and snakes.  
	Spline IK can be imagined as a high-level shape controller for any chain of objects 
	(particularly a bone structure).  With this controller the selected chain essentially 
	conforms to the geometry of a spline curve. As we change the shape of the spline, the 
	shape of the chain changes accordingly.


	CREATED BY:		Ambarish Goswami
					January, 2002

	HISTORY: 

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "SplineIKSolver.h"
#include "decomp.h" // included here such that the structure "AffineParts" can be used.
#include "TRIG.H"
#include "math.h"
#include "modstack.h" // need here to support IDerivedObject 
#include "splshape.h"  // need here to support splines
#include "IIKSys.h"
#include "surf_api.h"



#define SPLINEIKSOLVER_CLASS_ID	Class_ID(0x72d01454, 0x65b8b595)
// References for the splineIKSolver controller
#define SPLINEIKSOLVER_PBLOCK_REF		0

using namespace IKSys;

inline Quat MakeRowQuat(const Point3& axis, float angle)
{
  double a = 0.5 * angle;
  double c = cos(a);
  double s = -sin(a);
  return Quat((float)s*axis.x, (float)s*axis.y, (float)s*axis.z, (float)c);
}

static Quat operator*(const Quat& q, const Point3& v)
{
  Quat ret;
  ret.w = - q.x * v.x - q.y * v.y - q.z * v.z;
  ret.x = q.w * v.x + q.y * v.z - q.z * v.y;
  ret.y = q.w * v.y + q.z * v.x - q.x * v.z;
  ret.z = q.w * v.z + q.x * v.y - q.y * v.x;
  return ret;
}

inline Point3 ApplyRowQuat(const Point3& p, const Quat& q)
{
	Quat result = (q.Conjugate() * p) * q;
	return Point3(result.x, result.y, result.z);
}

enum {  
	spik_point_node_list,
};

class SplineIKSolver : public IKSolver { //Added after automatic generation of the project
	
	public:

		Point3 ee_axis_unit_vector_world;
		Point3 perp_vector;
		Point3 firstJointPositionInWorld, lastJointPositionInWorld;
		float chainLength, splLen; //sum of the bone lengths, total spline length
		Tab<float> boneLengths;
		/*
		Tab<Point3> initialJointEulers;
		Point3 veryStPoint;
		*/
		Tab<Point3> boneAxes;
		Tab<Point3> boneCoAxes;

		SplineIKSolver(BOOL loading){}	
		inline double acos_safe(double x)
		{
			return x <= -1.0 ? PI :
			x >= 1.0 ? 0.0 : acos(x);
		}

		inline double acos_safe(float x)
		{
			return x <= -1.0f ? PI :
			x >= 1.0f ? 0.0 : acos((double)x);
		}


		void SplineIKSolverInitializer (IKSys::LinkChain& linkChain);
		Interval ivalid;

		// IKSolver methods down

		bool IsHistoryDependent() const{return false; } // this solver is not histroy dependent
		bool DoesOneChainOnly() const{return true; }	// it is a single chain solver

		// Interactive solver does need initial pose. It needs current pose.
		bool IsInteractive() const{return false; }
		bool UseSlidingJoint() const{return false; }	// there is no sliding joint
		bool UseSwivelAngle() const{return false; }		// this solver uses (needs) swivel angle data
  		bool IsAnalytic() const { return true; }		// this is an analytical soilver

		Interface_ID ExpectGoal() const { return IKSys::kSplineIKGoalID; }
		// RotThreshold() is not relevant to solver that does not SolveEERotation().
		// UseSwivelAngle() and SolveEERotation() cannot both be true.
		bool SolveEERotation() const{return false; }

		// A solver may have its own zero map. If so, the IK system may want
		// to know through this method.
  		const IKSys::ZeroPlaneMap*
			GetZeroPlaneMap(const Point3& a0, const Point3& n0) const
			{ return NULL; }

		float		GetPosThreshold() const{return 0.0f; }
		float		GetRotThreshold() const{return 0.0f; }
		unsigned	GetMaxIteration() const{return 0; }
		void		SetPosThreshold(float){}
		void		SetRotThreshold(float){}
		void		SetMaxIteration(unsigned){}
		ReturnCondition Solve(IKSys::LinkChain&);

  		// IKSolver methods up

		Class_ID ClassID() {return SPLINEIKSOLVER_CLASS_ID;}		
		SClass_ID SuperClassID() { return IK_SOLVER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefResult NotifyRefChanged(		Interval changeInt, 
										RefTargetHandle hTarget,
										PartID& partID,
										RefMessage msg){
		//watje
			switch (msg) 
			{
				case REFMSG_MOUSE_CYCLE_STARTED:
//					DebugPrint("Mouse cycle started should lower the tolerances\n");
				break;

				case REFMSG_MOUSE_CYCLE_COMPLETED:
//					DebugPrint("Mouse cycle end should put back the tolerances\n");
				break;
			}

			return REF_DONTCARE;
		}

		//Constructor/Destructor
		SplineIKSolver(){}
		~SplineIKSolver(){}	//added after automatic generation of the project	
};



class SplineIKClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new SplineIKSolver(loading);}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return IK_SOLVER_CLASS_ID;}
	Class_ID		ClassID() {return SPLINEIKSOLVER_CLASS_ID;}
	const TCHAR* 	Category() {return _T(""); }
	const TCHAR*	InternalName() { return GetString(IDS_INTERNAL_NAME); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }		// returns owning module handle
};


static SplineIKClassDesc SplineIKDesc;
ClassDesc2* GetSplineIKDesc() {return &SplineIKDesc;}


void SplineIKSolver::SplineIKSolverInitializer (IKSys::LinkChain& linkChain){

		BaseInterface* gi = linkChain.GetIKGoal();
	ISplineIKGoal* spline_goal = gi != NULL
		? reinterpret_cast<ISplineIKGoal*>(gi->GetInterface(kSplineIKGoalID))
		: NULL;

//	INode* spGoal  = spline_goal->GetGoalNode();
	if (spline_goal == NULL) return;
	boneLengths.SetCount(0);
	//initialJointEulers.SetCount(0);
	boneAxes.SetCount(0);
	boneCoAxes.SetCount(0);

	//Point3 euls;
	IterJoint iter(linkChain);
	iter.InitJointAngles();
	float bl = 0.0;
	chainLength = 0.0;

#if 0
	iter.Begin(true);
	firstJointPositionInWorld = iter.Pivot();
	do{
	} while (iter.Next()); // just iterate through all the joints
	lastJointPositionInWorld =  iter.Pivot(); // get the world position of the last joint
	ee_axis_unit_vector_world = lastJointPositionInWorld - firstJointPositionInWorld;
	perp_vector = (*spline_goal->DefaultZeroMap())(ee_axis_unit_vector_world);
	perp_vector = spline_goal->TwistParent().VectorTransform(perp_vector);
	iter.Begin(false);
	do {
		iter.Pivot();			//proximalFrame.GetTrans
		iter.DistalEnd();
		
		bl = FLength(iter.DistalEnd() - iter.Pivot());
		chainLength = chainLength + bl;
		boneLengths.Append(1, &bl, 1);   // forming a tab of bonelengths to be used later

//		euls = iter.GetJointAngles();

//		initialJointEulers.Append(1, &euls, 1);
	}	while (iter.Next());
#else
	iter.Begin(false);
	firstJointPositionInWorld = iter.Pivot();
	do { } while (iter.Next());
	lastJointPositionInWorld =  iter.Pivot();
	ee_axis_unit_vector_world = lastJointPositionInWorld - firstJointPositionInWorld;
	perp_vector = (*spline_goal->DefaultZeroMap())(ee_axis_unit_vector_world);
	Point3 up_vec0 = perp_vector;
	perp_vector = spline_goal->TwistParent().VectorTransform(perp_vector);
	perp_vector = VectorTransform(perp_vector, Inverse(linkChain.parentMatrix));
	up_vec0 = VectorTransform(up_vec0, Inverse(linkChain.parentMatrix));
	perp_vector.Unify();
	up_vec0.Unify();

	iter.Begin(false);
	Point3 p_pivot = iter.Pivot();
	int ip = 0;
	int ei = 0;
	do {
		Matrix3 mat = iter.ProximalFrame();
		IterJoint::JointAxes jax = iter.GetJointAxes();
		Point3 jang = iter.GetJointAngles();
		for (int i = 2; i >= 0; --i) {
			if (jax[i] == '_') break;
			switch (jax[i]) {
			case 'x':
				mat.PreRotateX(jang[0]);
				break;
			case 'y':
				mat.PreRotateY(jang[1]);
				break;
			case 'z':
				mat.PreRotateZ(jang[2]);
				break;
			}
		}
		mat.Invert();
		Point3 ba = iter.DistalEnd() * mat;
		ba.Unify();
		boneAxes.Append(1, &ba, 4);

		ei = 0;
		float max_x = fabs(boneAxes[ip].x);
		float ff = fabs(boneAxes[ip].y);
		if (max_x < ff) {
			max_x = ff;
			ei = 1;
		}
		ff = fabs(boneAxes[ip].z);
		if (max_x < ff) {
			max_x = ff;
			ei = 2;
		}
		Point3 up_vec = VectorTransform(up_vec0, mat);
		Point3 bca = CrossProd(up_vec, ba);
		if (bca.LengthUnify() < 100.0f*FLT_EPSILON) {
			bca.Set(0.0f, 0.0f, 0.0f);
			int ek = (ei + 2) % 3;
			bca[ek] = 1.0f;
			bca = CrossProd(bca, ba);
			bca.Unify();
		}
		bca = CrossProd(ba, bca);
		boneCoAxes.Append(1, &bca, 4);
		bl = FLength(iter.DistalEnd() - iter.Pivot());
		chainLength = chainLength + bl;
		// forming a tab of bonelengths to be used later
		boneLengths.Append(1, &bl, 1);
		p_pivot = iter.Pivot();
		++ip;
	}	while (iter.Next());
#endif

	// -----------
	//need to add the length of the last bone to chainLength
	// -----------
}


#define marq_up_lim		1000000.0f		// max value of L-M parameter
#define marq_lo_lim		0.0000001f		// min value of L-M parameter
#define max_iteration	200			//max # of L-M iterations

#define errCriter1		0.0000001f

IKSolver::ReturnCondition SplineIKSolver::Solve(IKSys::LinkChain& linkChain)
{
	BaseInterface* gi = linkChain.GetIKGoal();
	ISplineIKGoal* spline_goal = gi != NULL
		? reinterpret_cast<ISplineIKGoal*>(gi->GetInterface(kSplineIKGoalID))
		: NULL;
	if (spline_goal == NULL) return bInvalidArgument;

	INode* spGoal  = spline_goal->GetGoalNode();
	IIKChainControl*  spChainControl = spline_goal->GetChainControl();
	if (spGoal == NULL || spChainControl == NULL) return bInvalidArgument;

	// get the initial joint Euler angles
	SplineIKSolverInitializer(linkChain);
	/*	
	 * Don't use this system time!! The renderer, for example, evaluates the
	 * scene without setting it. (472418)
	 * J.Zhao, 12/11/02.
	TimeValue t = GetCOREInterface()->GetTime();
	*/

//	splLen = spline_goal->GetSplineLength();
	float startTwistAngle, endTwistAngle;
	startTwistAngle = spline_goal->TwistHStartAngle();
	endTwistAngle = spline_goal->TwistHEndAngle();

	BOOL isClosed = spline_goal->IsClosed();


//	if (splLen <= 0.0f) return 0;  // there is a problem, we still don't have the actual spline
	
	int boneCount = boneLengths.Count();
	//get the position on spline for u = 0.0f
//	stPoint = spline_goal->SplinePosAt(0.0f, 0); //startpoint coordinates of the spline in world
	Point3  stPoint, runPoint, tang, del_tang, third_row, norm;
	Point3 jointToJointVec;
	Matrix3 localTMMatrix, globalTMMatrix;
	Point3 p3, remp3;
	float dist;//, dist2;//, uNormal;
	float eps1, eps2, eps3;//position of spline has to match with bone length with this accuracy
	float uIncrement1, uIncrement2, uIncrement3, uIncrement4;
	Tab<Point3> eulersOnSpline;
	Tab<Matrix3> frameOnSpline;
	Tab<float> uValues;
	Tab<Point3> pointOnSpline;
//	int flag1, flag2, flag3;
//	float     	obfunc = 0.0f;
//	float     	obfunci = 0.0f;
//	float     	obfunc_prime = 0.0f;
//	float		lamda = 0.01f;
//	float		dist_pert;
//	float		jacobian;
//	float		resid;
//	float		gradient;

	float Eul[3] = {0.0f ,0.0f ,0.0f};

	uIncrement1 = 0.001f;
	uIncrement2 = 0.0001f;
	uIncrement3 = 0.000001f;
	uIncrement4 = 0.005f;

	float u = 0.0f;
	eps1 = boneLengths[0]/7.0f;
	eps2 = boneLengths[0]/50.0f;
	eps3 = boneLengths[0]/100.0f;
//watje first ask we need to rebuild cache
	spline_goal->SplinePosAt(0.0f, 1, TRUE);
	Matrix3 toJointMat = Inverse(linkChain.parentMatrix);
#define	GET_SPLINE_POS(u) (spline_goal->SplinePosAt(u, 0)*toJointMat)

	int iteration2 = 0;
/*  
	//this implements getting "percent" directly from the path_constraint on the
	// rootbone

	INode* stJoint = spline_goal->StartJoint();
	INode* endJoint = spline_goal->EndJoint();
	Control *cont = stJoint->GetTMController()->GetPositionController();
	
	IParamBlock2 *sikcPB2;
	float percent;
	sikcPB2 = cont->GetParamBlockByID(path_params);
	sikcPB2->GetValue(path_percent, t, percent, FOREVER);
*/

//	DebugPrint("Spline Solve\n");
	float dist_min = 1.0e30f;
	float u_min = 0.0f;
	Point3 runPoint_min;
	while (u <= 1.0f){
		iteration2 = iteration2 + 1;
		runPoint = GET_SPLINE_POS(u);
//	DebugPrint("Spline Solve  %f\n",u);
		dist = FLength(runPoint - firstJointPositionInWorld);
//		dist2 = FLength(runPoint - veryStPoint);
		if (dist < dist_min) {
			u_min = u;
			dist_min = dist;
			runPoint_min = runPoint;
		}
		if (dist > eps1)
			u = u + uIncrement1;
		else {
			if (dist > eps2)
				u = u + uIncrement2;
			else {
				if (dist > eps3)
					u = u + uIncrement3;
				else {
					uValues.Append(1, &u, 1);
					pointOnSpline.Append(1, &runPoint, 1);
					stPoint = runPoint;						
					break;
				}				
			}
		}	
	}

/*  
	//this implements getting "percent" directly from the path_constraint on the
	// rootbone

	u = percent;
	runPoint = spline_goal->SplinePosAt(u, 0);
	uValues.Append(1, &u, 1);
	pointOnSpline.Append(1, &runPoint, 1);
	stPoint = runPoint;	

*/

	Point3 offset(0,0,0);
	if (u > 1.0f) {
		u = u_min;
		stPoint = runPoint_min;
		offset = firstJointPositionInWorld - stPoint;
		uValues.Append(1, &u, 1);
		pointOnSpline.Append(1, &stPoint, 1);
	}

	int j = 0; //running jointCount
	int iteration = 0; //checking to see how many iterations it takes
	/*
	ObjectState os;
	os = spGoal->EvalWorldState(t);

	Class_ID cid;
	SClass_ID sid;
	cid = os.obj->ClassID();
	sid = os.obj->SuperClassID();
	*/
	// We use two different algorithms for splines and NURBS

	// TEMPORARILY TURNING THE LEVENBERG-MARQUARDT ALGORITHM OFF!
/*
	if (cid == EDITABLE_SURF_CLASS_ID){

		int aggregate_iteration = 0;
		bool init_guess_flag = FALSE;
	//	int inter_j = -1;

		while(u <= 1.0f && j < boneCount && aggregate_iteration < 5000){
	//		while (flag1 != 1 && flag2 != 1 && flag3 != 1){ 

			//compute an initial guess
			int iteration1 = 0; //checking to see how many iterations it takes
			while(u <= 1.0f && init_guess_flag == FALSE){
				iteration1 = iteration1 + 1;
				// compute the spline position
				runPoint = spline_goal->SplinePosAt(u, 0);
				// compare dist between startPoint to current point with ith  bone length 
				dist = FLength(runPoint - stPoint);
				float diff = fabs(dist - boneLengths[j]);
				// if the difference is small enough....
				if (fabs(dist - boneLengths[j]) > eps1){
					u = u +  0.001f;
				}
				else {
					init_guess_flag = TRUE;
	//				inter_j = inter_j+1;
					break;
				}
			}

			if (u > 1.0f) break;

				iteration = iteration + 1;
				aggregate_iteration = aggregate_iteration + 1;


				// compute the spline position
				runPoint = spline_goal->SplinePosAt(u, 0);
				// compare dist between startPoint to current point with ith  bone length 
				dist = FLength(runPoint - stPoint);
				resid = dist - boneLengths[j];

				obfunc = resid * resid;

				if (obfunc < errCriter1){

					if (u < uValues[j] ){
						u = uValues[j] + 0.1;
						if(u > 1.0f) u = 1.0f;
						if(u < 0.0f) u = 0.0f;
						lamda = 0.01f;
						iteration = 0;
						continue;

					}
					uValues.Append(1, &u, 1);
					pointOnSpline.Append(1, &runPoint, 1);
					j = j + 1;
					init_guess_flag = FALSE;
					u = u +  uIncrement2;
					stPoint = runPoint;
					lamda = 0.01f;
					iteration = 0;
					continue;
				}

     			obfunc_prime = obfunc;
				

				// Compute Marquardt's parameter depending on how objective function is changing 
				// Set max and min limits to Marquardt's parameter : marq_up_lim and  marq_lo_lim 
				if (iteration > 1) {
					if (obfunc < obfunc_prime) lamda = lamda/10.0;
  					else lamda = lamda*10.0;
				}
  				if (lamda > marq_up_lim) lamda = marq_up_lim;
  				else if (lamda < marq_lo_lim) lamda = marq_lo_lim;


				if(u > 1.0f) u = 1.0f;
				if(u < 0.0f) u = 0.0f;
				dist_pert = FLength(spline_goal->SplinePosAt(u + uIncrement4, 0) - stPoint);
				jacobian = (dist_pert - dist)/uIncrement4;
				
				// Calculate update vector from Jacobian and residual  	
				
				float hess = jacobian * jacobian;
				gradient = jacobian * resid;
	//			float vect_mult = gradient * (1.0f/hess);
	//			float mult = (gradient * vect_mult)/obfunc;
				float hess_inv = 1.0f/(hess + lamda);
				float u_update = -(hess_inv * gradient);
						
				if(iteration > max_iteration){
					uValues.Append(1, &u, 1);
					pointOnSpline.Append(1, &runPoint, 1);
					j = j + 1;
					init_guess_flag = FALSE;
					u = u +  uIncrement2;
					stPoint = runPoint;
					iteration = 0;
					lamda = 0.01f;
					continue;
				}
				
				// Updating parameter vector // 			
				u = u +  0.3f * u_update;

				

	//		}

		}
	}

	else if (sid == SHAPE_CLASS_ID){
*/
		if(isClosed != 1){
			float u0 = u;
			while(u <= 1.0f && j < boneCount){

				// compute the spline position
				runPoint = GET_SPLINE_POS(u);
				// compare dist between startPoint to current point with ith  bone length 
				dist = FLength(runPoint - stPoint);
				// if the difference is small enough....
				if (boneLengths[j] - dist > eps1){
					u0 = u;
					u = u +  uIncrement1;
					iteration = iteration + 1;
				}
				else{
					if (boneLengths[j] - dist > eps2){
						u0 = u;
						u = u + uIncrement2;
						iteration = iteration + 1;
					}
					else{
						if (boneLengths[j] - dist > eps3){
							u0 = u;
							u = u + uIncrement3;
							iteration = iteration + 1;
						}
						else if (dist - boneLengths[j] > eps3) {
							// Backtrack
							//
							float del_u = u - u0;
							if (del_u == uIncrement1) {
							  u = u0 + uIncrement2;
							} else if (del_u == uIncrement2) {
							  u = u0 + uIncrement3;
							} else {
								// Give up:
								uValues.Append(1, &u, 1);
								pointOnSpline.Append(1, &runPoint, 1);
								stPoint = runPoint;
								u0 = u;

								j = j + 1;	
								if (j < boneCount) {
								  eps1 = boneLengths[j]/5.0f;
								  eps2 = boneLengths[j]/50.0f;
								  eps3 = boneLengths[j]/100.0f;
								}
							}
							iteration = iteration + 1;
						}
						else{
							uValues.Append(1, &u, 1);
							pointOnSpline.Append(1, &runPoint, 1);
							stPoint = runPoint;
							u0 = u;

							j = j + 1;	
							if (j < boneCount) {
								eps1 = boneLengths[j]/5.0f;
								eps2 = boneLengths[j]/50.0f;
								eps3 = boneLengths[j]/100.0f;
							}
							iteration = iteration + 1;

						}

					}

				}
					
			}
		}
		else if (isClosed == 1){ // isClosed == 1
			float du = 0.0f;
			float u0 = u;
			while(du <= 1.0f && j < boneCount){

				// compute the spline position
				runPoint = GET_SPLINE_POS(u);
				// compare dist between startPoint to current point with ith  bone length 
				dist = FLength(runPoint - stPoint);
				// if the difference is small enough....
				if (boneLengths[j] - dist > eps1){
					u0 = u;
					u = u +  uIncrement1;
					du += uIncrement1;
					if (u > 1.0) u = u - 1.00f;
					iteration = iteration + 1;
				}
				else{
					if (boneLengths[j] - dist > eps2){
						u0 = u;
						u = u + uIncrement2;
						du += uIncrement2;
						if (u > 1.0) u = u - 1.00f;
						iteration = iteration + 1;
					}
					else{
						if (boneLengths[j] - dist > eps3){
							u0 = u;
							u = u + uIncrement3;
							du += uIncrement3;
							if (u > 1.0) u = u - 1.00f;
							iteration = iteration + 1;
						}
						else if (dist - boneLengths[j] > eps3) {
							// Backtrack:
							//
							float del_u = u > u0 ? u - u0 : 1.0f + u - u0;
							if (del_u == uIncrement1) {
								u = u0 + uIncrement2;
								du += (uIncrement2 - uIncrement1);
								if (u > 1.0f) u -= 1.0f;
							} else if (del_u == uIncrement2) {
								u = u0 + uIncrement3;
								du += (uIncrement3 - uIncrement2);
								if (u > 1.0f) u -= 1.0f;
							} else {
								// Give up:
								//
								uValues.Append(1, &u, 1);
								pointOnSpline.Append(1, &runPoint, 1);
								stPoint = runPoint;

								j = j + 1;
								du = 0.0f;
								if (j < boneCount) {
									eps1 = boneLengths[j]/5.0f;
									eps2 = boneLengths[j]/50.0f;
									eps3 = boneLengths[j]/100.0f;
								}
							}
							iteration = iteration + 1;
						}
						else{
							uValues.Append(1, &u, 1);
							pointOnSpline.Append(1, &runPoint, 1);
							stPoint = runPoint;
							
							j = j + 1;
							du = 0.0f;
							if (j < boneCount) {
								eps1 = boneLengths[j]/5.0f;
								eps2 = boneLengths[j]/50.0f;
								eps3 = boneLengths[j]/100.0f;
							}
							iteration = iteration + 1;

						}

					}

				}
					
			}

		}

//  }

	int uCount = uValues.Count();
	if (!uCount)  return 0;
//	DebugPrint("%d  %d  %d \n", iteration, uCount, boneCount);
#if 0
	if(uCount != boneCount + 1){
//		if(uValues.Count() < boneLengths.Count()+1){	//means the curve is shorter than the 
														//total linkchain length
			//if the assert is hit => we missed at least one valid bone intersection point
//		if (chainLength > splLen){
			//boneLengths.SetCount(0);
			//return 0;

//		}
		if(uValues[0] == 0.0f){

		}
		else{
//			boneLengths.SetCount(0);
//			return 0;
		}	

	}// we have a problem some of the u_values weren't detected
#endif
	
	//We compute the local coordinate frame and assign to the bones
	for(int kk = 0; kk < boneCount; kk = kk+1){
		float twistAng = startTwistAngle + (endTwistAngle/(boneCount-1)) * (kk);
#if 0
		if(kk >= uCount - 1){
			// if we satisfy this condition => typically means that we have some 
			// bones hanging out of the spline. We set the "direction" of these bones to be 
			// equal to the tangent at u=1.0 of the chain.

			if (kk == uCount - 1) {		// for first bone that is just falling off of the spline
				globalTMMatrix.IdentityMatrix();
				jointToJointVec = Normalize(spline_goal->SplinePosAt(1.00f, 0) - pointOnSpline[kk]);
				globalTMMatrix.SetRow(0, jointToJointVec);
				tang = perp_vector;//spline_goal->SplineTangentAt(uValues[kk], 0);
				norm = Normalize(CrossProd(tang, jointToJointVec));
				globalTMMatrix.SetRow(1, norm);
				third_row = CrossProd(jointToJointVec, norm); 
				globalTMMatrix.SetRow(2, third_row);				

				if(kk) {
					localTMMatrix = globalTMMatrix * Inverse(frameOnSpline[kk-1]);
					localTMMatrix.PreRotateX(twistAng);
					globalTMMatrix = localTMMatrix * frameOnSpline[kk-1];
				}
				else{
					globalTMMatrix.PreRotateX(twistAng);
					localTMMatrix = globalTMMatrix;
				}	
				frameOnSpline.Append(1, &globalTMMatrix, 1);			
//				
//				localTMMatrix.SetRow(2, third_row);
//				frameOnSpline.Append(1, &localTMMatrix, 1);
//				if (kk == 0)
//					localTMMatrix = frameOnSpline[kk];
//				else
//					localTMMatrix = frameOnSpline[kk]* Inverse(frameOnSpline[kk-1]);
//
				MatrixToEuler(localTMMatrix, Eul, EULERTYPE_XYZ);
				remp3 = Point3(Eul[0], Eul[1], Eul[2]);
				eulersOnSpline.Append(1, &remp3, 1);
			}
			else if (kk == uCount) {// for first bone that is completely outside the spline
				globalTMMatrix.IdentityMatrix();
				jointToJointVec = Normalize(spline_goal->SplinePosAt(1.00f, 0) - spline_goal->SplinePosAt(0.99f, 0));
				globalTMMatrix.SetRow(0, jointToJointVec);
				tang = perp_vector;//spline_goal->SplineTangentAt(0.99f, 0);
				norm = Normalize(CrossProd(tang, jointToJointVec));
				globalTMMatrix.SetRow(1, norm);
				third_row = CrossProd(jointToJointVec, norm); 
				globalTMMatrix.SetRow(2, third_row);
				if(kk) {
					localTMMatrix = globalTMMatrix * Inverse(frameOnSpline[kk-1]);
					localTMMatrix.PreRotateX(twistAng);
					globalTMMatrix = localTMMatrix * frameOnSpline[kk-1];
				}
				else{
					globalTMMatrix.PreRotateX(twistAng);
					localTMMatrix = globalTMMatrix;
				}	
				frameOnSpline.Append(1, &globalTMMatrix, 1);	


//				frameOnSpline.Append(1, &localTMMatrix, 1);
//				if (kk == 0)
//					localTMMatrix = frameOnSpline[kk];
//				else
//					localTMMatrix = frameOnSpline[kk]* Inverse(frameOnSpline[kk-1]);


				MatrixToEuler(localTMMatrix, Eul, EULERTYPE_XYZ);
				remp3 = Point3(Eul[0], Eul[1], Eul[2]);
				eulersOnSpline.Append(1, &remp3, 1);
			}
			else{// for the rest of the bones
				remp3 = Point3(0.0f, 0.0f, 0.0f);
				eulersOnSpline.Append(1, &remp3, 1);
			}

		}
		else{
			globalTMMatrix.IdentityMatrix();
			jointToJointVec = Normalize((pointOnSpline[kk+1] - pointOnSpline[kk]));
			globalTMMatrix.SetRow(0, jointToJointVec);
			tang = perp_vector;//spline_goal->SplineTangentAt(uValues[kk], 0);
			norm = Normalize(CrossProd(tang, jointToJointVec));
			globalTMMatrix.SetRow(1, norm);
			third_row = CrossProd(jointToJointVec, norm); 
			globalTMMatrix.SetRow(2, third_row);
			if(kk) {
				localTMMatrix = globalTMMatrix * Inverse(frameOnSpline[kk-1]);
				localTMMatrix.PreRotateX(twistAng);
				globalTMMatrix = localTMMatrix * frameOnSpline[kk-1];
			}
			else{
				globalTMMatrix.PreRotateX(twistAng);
				localTMMatrix = globalTMMatrix;
			}
			frameOnSpline.Append(1, &globalTMMatrix, 1);

			MatrixToEuler(localTMMatrix, Eul, EULERTYPE_XYZ);
			p3 = Point3(Eul[0], Eul[1], Eul[2]);
			eulersOnSpline.Append(1, &p3, 1);
		}
#else
		// Pre-condition: uCount > 0.
		globalTMMatrix.IdentityMatrix();
		if (kk <= uCount) {
			jointToJointVec = (kk == uCount - 1)
				? Normalize(GET_SPLINE_POS(1.00f) - pointOnSpline[kk])
				: (kk == uCount)
				? Normalize(GET_SPLINE_POS(1.0f) - GET_SPLINE_POS(0.99f))
				: Normalize((pointOnSpline[kk+1] - pointOnSpline[kk]));
		}

		int i = 0;
		float max_x = fabs(boneAxes[kk].x);
		float ff = fabs(boneAxes[kk].y);
		if (max_x < ff) {
			max_x = ff;
			i = 1;
		}
		ff = fabs(boneAxes[kk].z);
		if (max_x < ff) {
			max_x = ff;
			i = 2;
		}
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;
		Matrix3 ax_mat;
		ax_mat.SetRow(i, boneAxes[kk]);
		ax_mat.SetRow(k, boneCoAxes[kk]);
		ax_mat.SetRow(j, CrossProd(boneCoAxes[kk], boneAxes[kk]));
		ax_mat.NoTrans();
		ax_mat.Invert();
		globalTMMatrix.SetRow(i, jointToJointVec);
		tang = perp_vector;
		norm = CrossProd(tang, jointToJointVec);
		if (norm.LengthUnify() < 100.0f*FLT_EPSILON) {
			norm.Set(0.0f, 0.0f, 0.0f);
			norm[k] = 1.0f;
			norm = CrossProd(norm, jointToJointVec);
			norm.Unify();
		}
		globalTMMatrix.SetRow(j, norm);
		third_row = CrossProd(jointToJointVec, norm); 
		globalTMMatrix.SetRow(k, third_row);
		globalTMMatrix = ax_mat * globalTMMatrix;
		Quat qt(AngAxis(boneAxes[kk], twistAng));
		RotationValue rt(qt.Conjugate());
		if(kk) {
			localTMMatrix = globalTMMatrix * Inverse(frameOnSpline[kk-1]);
			rt.PreApplyTo(localTMMatrix);
			globalTMMatrix = localTMMatrix * frameOnSpline[kk-1];
		}
		else{
			rt.PreApplyTo(globalTMMatrix);
			localTMMatrix = globalTMMatrix;
		}
		frameOnSpline.Append(1, &globalTMMatrix, 1);

		MatrixToEuler(localTMMatrix, Eul, EULERTYPE_XYZ);
		p3 = Point3(Eul[0], Eul[1], Eul[2]);
		eulersOnSpline.Append(1, &p3, 1);
#endif
	}

	IterJoint iter(linkChain);
	
	int  ii = 0;
	iter.Begin(false);
	do {
		iter.SetJointAngles(eulersOnSpline[ii]);
		ii = ii + 1;
	}	while (iter.Next());

	uValues.SetCount(0);
	boneLengths.SetCount(0);
	//initialJointEulers.SetCount(0);
	boneAxes.SetCount(0);
	boneCoAxes.SetCount(0);

	return 0;
}



