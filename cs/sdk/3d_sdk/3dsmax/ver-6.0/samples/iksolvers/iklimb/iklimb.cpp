/**********************************************************************
 *<
	FILE: IKLimb.cpp

	DESCRIPTION:	The analytical Human Limb (ie Human Arm and Human Leg) or the 2-bone 
					IK solver. The solver only works on a specific type of link chain, one 
					that is composed of a spherical joint (shoulder or hip) followed by 
					a revolute joint (elbow or knee). The user will not be able to engage
					this solver on an inappropriate type of chain. The solver computes 
					the joint angles directly from mathematical equations -- no iterations 
					are necessary.
					
					The solver is a plug-in into the general IK system and is identified by
					"IK Limb Solver". The solver can be engaged from the 
					Animation->IK Solvers menu.

					The variable in this module are deliberately given long explicit
					names for better understanding. The variable names correspond to the 
					human arm consisiting of shoulder, elbow, wrist, upper arm and lower arm.
					The names whould be self-explanatory.


	CREATED BY:		Ambarish Goswami
					April, 2000, modifed to finally hook up with the IK System, August 2000

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "IKLimb.h"
#include "decomp.h" // included here such that the structure "AffineParts" can be used.
#include "TRIG.H"
#include "math.h"

#define IKLIMB_CLASS_ID	Class_ID(0x31c2f677, 0x2930d2b6)


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

class IKLimb : public IKSolver { //Added after automatic generation of the project
	
	public:	
		IKLimb(BOOL loading){}	
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


		float arm_swivel_angle; // used by the IK system to revolve the chaiin about the ee axis.

// **** constants for the chain -- set up during initialization ****

		float upper_arm_length, lower_arm_length;
		float upper_arm_plus_lower_arm_length, upper_arm_minus_lower_arm_length;
		Matrix3 initial_shoulder_frame;

// declaration of variables for the initial arm set-up
		Matrix3 initial_elbow_transformation_matrix_in_shoulder, initial_elbow_transformation_matrix_in_world;
		Point3 initial_elbow_position_in_shoulder, initial_elbow_position_in_world;
// ee_axis is the end-effector axis:  a line connecting the root (shouilder) and 
// the end-effector (wrist)
		Point3 initial_ee_axis_unit_vector_world, initial_ee_axis_unit_vector_shoulder;
		Point3 initial_wrist_position_in_shoulder, initial_wrist_position_in_world;

// chain_plane_angle_at shoulder is the angle made by the ee-axis and the upper_arm 
		float initial_chain_plane_ang_at_shoulder;

		Point3 initial_elbow_position_unit_vector_world; // the unit vector is calculated in world frame
		Point3 initial_elbow_position_unit_vector_shoulder; // the unit vector is calculated in shoulder frame
		Point3 initial_normal_to_chain_plane_shoulder;		// the normal is calculated in shoulder frame

		float initial_elbow_joint_angle; // elbow_joint_angle is the angle between the upper_arm and the lower_arm.
		
		void IKLimbSolverInitializer (IKSys::LinkChain& linkChain);
		void IKLimb_1Bone_SolverInitializer (IKSys::LinkChain& linkChain);

		Interval ivalid;

		// IKSolver methods down

		bool IsHistoryDependent() const{return false; } // this solver is not histroy dependent
		bool DoesOneChainOnly() const{return true; }	// it is a single chain solver

		// Interactive solver does need initial pose. It needs current pose.
		bool IsInteractive() const{return false; }
		bool UseSlidingJoint() const{return false; }	// there is no sliding joint
		bool UseSwivelAngle() const{return true; }		// this solver uses (needs) swivel angle data
  		bool IsAnalytic() const { return true; }		// this is an analytical soilver

		Interface_ID ExpectGoal() const { return IKSys::kHIIKGoalID; }
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

		Class_ID ClassID() {return IKLIMB_CLASS_ID;}		
		SClass_ID SuperClassID() { return IK_SOLVER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}
		
		//Constructor/Destructor
		IKLimb(){}
		~IKLimb() {}	//added after automatic generation of the project	
};


class IKLimbClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new IKLimb(loading);}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
//	SClass_ID		SuperClassID() {return REF_TARGET_CLASS_ID;}	// will need if the solver has its own UI
	SClass_ID		SuperClassID() {return IK_SOLVER_CLASS_ID;}
	Class_ID		ClassID() {return IKLIMB_CLASS_ID;}
//	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR* 	Category() {return _T(""); }
	const TCHAR*	InternalName() { return GetString(IDS_INTERNAL_NAME); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }		// returns owning module handle
};

static IKLimbClassDesc IKLimbDesc;
ClassDesc2* GetIKLimbDesc() {return &IKLimbDesc;}


// The data from the nodes (shoulder, elbow, wrist and
// goal are transformed to the LinkChain data structure so that this data structure can be tested.




void IKLimb :: IKLimb_1Bone_SolverInitializer (IKSys::LinkChain& linkChain){

	Point3 zeroPoint(0.0f, 0.0f, 0.0f);

	BaseInterface* binf = linkChain.GetIKGoal();
	IHIIKGoal* goal_inf = NULL;
	if (binf != NULL) {
		goal_inf = reinterpret_cast<IHIIKGoal*>(binf->GetInterface(kHIIKGoalID));
	}
	if (goal_inf == NULL) {
		DbgAssert(0);
		return;
	}

// Comment: Set the variables to their initial values to compute the initial pose of the chain.

		linkChain.rootLink.rotXYZ = linkChain.rootLink.initXYZ;

		initial_elbow_transformation_matrix_in_world = 
		linkChain.rootLink.LinkMatrix(true);			// this is the elbow_position_in_world

		initial_elbow_position_in_world = initial_elbow_transformation_matrix_in_world.GetTrans();

		initial_elbow_position_unit_vector_world = Normalize(initial_elbow_position_in_world);
		initial_ee_axis_unit_vector_world = initial_elbow_position_unit_vector_world;

		Point3 initial_shoulder_rot_x = initial_ee_axis_unit_vector_world;
		Point3 initial_shoulder_rot_y = (*goal_inf->DefaultZeroMap())(initial_ee_axis_unit_vector_world);
		Point3 initial_shoulder_rot_z = Normalize(CrossProd(initial_shoulder_rot_x, initial_shoulder_rot_y));	

//		The initial_shoulder frame is set up below
		initial_shoulder_frame.SetRow(0, initial_shoulder_rot_x);
		initial_shoulder_frame.SetRow(1, initial_shoulder_rot_y);
		initial_shoulder_frame.SetRow(2, initial_shoulder_rot_z);
		initial_shoulder_frame.SetRow(3, zeroPoint);

}




void IKLimb :: IKLimbSolverInitializer (IKSys::LinkChain& linkChain){

	Point3 zeroPoint(0.0f, 0.0f, 0.0f);

	BaseInterface* binf = linkChain.GetIKGoal();
	IHIIKGoal* goal_inf = NULL;
	if (binf != NULL) {
		goal_inf = reinterpret_cast<IHIIKGoal*>(binf->GetInterface(kHIIKGoalID));
	}
	if (goal_inf == NULL) {
		DbgAssert(0);
		return;
	}

	// We do not reset any chain variable in this code segment.
	// Only the chain initialization will be done.
	// We calculate:
	//		1. upper arm length
	//		2. lower arm length
	//		3. elbow joint angle on chain plane -- so that we can later calculate by how much it has changed
	//		4. set up a coordinate frame "initial_shoulder_frame" based on the initial chain pose -- this is
	//				needed to calculate the shoulder joint angles 

// Comment: Set the variables to their initial values to compute the initial pose of the chain.

		linkChain.rootLink.rotXYZ = linkChain.rootLink.initXYZ;

		linkChain.LinkOf(0).dofValue = linkChain.LinkOf(0).initValue;
		linkChain.LinkOf(1).dofValue = linkChain.LinkOf(1).initValue;
		linkChain.LinkOf(2).dofValue = linkChain.LinkOf(2).initValue;


		// everything below is in the world frame
		initial_elbow_transformation_matrix_in_world = 
			linkChain.LinkOf(2).DofMatrix() *				// these are elbow rotation transformations
			linkChain.LinkOf(1).LinkMatrix(true) *			// these are elbow rotation transformations
			linkChain.LinkOf(0).LinkMatrix(true) *			// these are elbow rotation transformations
			linkChain.rootLink.LinkMatrix(true);			// this is the elbow_position_in_world

		initial_elbow_position_in_world = initial_elbow_transformation_matrix_in_world.GetTrans();
		initial_elbow_position_unit_vector_world = Normalize(initial_elbow_position_in_world);

		Matrix3 matr1 = linkChain.rootLink.LinkMatrix(true);
		linkChain.LinkOf(0).ApplyLinkMatrix(matr1, true);
		linkChain.LinkOf(1).ApplyLinkMatrix(matr1, true);
		linkChain.LinkOf(2).ApplyLinkMatrix(matr1, true);

		initial_wrist_position_in_world =  matr1.GetTrans();
		initial_ee_axis_unit_vector_world = Normalize(initial_wrist_position_in_world); // because shoulder position is always (0,0,0)

		upper_arm_length = FLength(initial_elbow_position_in_world);
		lower_arm_length = FLength(initial_wrist_position_in_world - initial_elbow_position_in_world);

		upper_arm_plus_lower_arm_length = upper_arm_length + lower_arm_length;
		upper_arm_minus_lower_arm_length = upper_arm_length - lower_arm_length;

		
		float shoulder_to_wrist_distance = FLength(initial_wrist_position_in_world);

//		"gg" below is an intermediate variable for computing the "elbow_joint_angle"
		float gg = upper_arm_length * upper_arm_length + 
				lower_arm_length * lower_arm_length -
				shoulder_to_wrist_distance * shoulder_to_wrist_distance;
		float cosine_of_elbow_joint_angle = gg/(2 * upper_arm_length * lower_arm_length);

		initial_elbow_joint_angle = acos_safe(cosine_of_elbow_joint_angle);

//		"gg1" below is an intermediate variable for computing the "chain_plane_ang_at_shoulder"
		float gg1 = shoulder_to_wrist_distance * shoulder_to_wrist_distance + 
			 upper_arm_length * upper_arm_length - 
			 lower_arm_length * lower_arm_length;
		float cosine_of_chain_plane_ang_at_shoulder = gg1/(2 * shoulder_to_wrist_distance * upper_arm_length);

		initial_chain_plane_ang_at_shoulder = acos_safe(cosine_of_chain_plane_ang_at_shoulder);

		Point3 initial_shoulder_rot_x = initial_ee_axis_unit_vector_world;
		Point3 initial_shoulder_rot_y = (*goal_inf->DefaultZeroMap())(initial_ee_axis_unit_vector_world);
		Point3 initial_shoulder_rot_z = Normalize(CrossProd(initial_shoulder_rot_x, initial_shoulder_rot_y));	

//		The initial_shoulder frame is set up below
		initial_shoulder_frame.SetRow(0, initial_shoulder_rot_x);
		initial_shoulder_frame.SetRow(1, initial_shoulder_rot_y);
		initial_shoulder_frame.SetRow(2, initial_shoulder_rot_z);
		initial_shoulder_frame.SetRow(3, zeroPoint);

// everything above is in the world frame


// everything below is in the shoulder frame
		Matrix3 matr2 = linkChain.rootLink.LinkMatrix(false);
		initial_elbow_transformation_matrix_in_shoulder = matr2;

		initial_elbow_position_in_shoulder = initial_elbow_transformation_matrix_in_shoulder.GetTrans();
		initial_elbow_position_unit_vector_shoulder = Normalize(initial_elbow_position_in_shoulder);

		linkChain.LinkOf(0).ApplyLinkMatrix(matr2, true);
		linkChain.LinkOf(1).ApplyLinkMatrix(matr2, true);
		linkChain.LinkOf(2).ApplyLinkMatrix(matr2, true);
		initial_wrist_position_in_shoulder =  matr2.GetTrans();
		initial_ee_axis_unit_vector_shoulder = Normalize(initial_wrist_position_in_shoulder);
		initial_normal_to_chain_plane_shoulder = Normalize(CrossProd(initial_ee_axis_unit_vector_shoulder, initial_elbow_position_unit_vector_shoulder));
// everything above is in the shoulder frame
}


IKSolver :: ReturnCondition IKLimb :: Solve (IKSys::LinkChain& linkChain){
		Point3 zeroPoint(0.0f, 0.0f, 0.0f);

	if (!(linkChain.LinkCount() == 0 || linkChain.LinkCount() == 3)) { // If this is true, you are applying IK Limb to a wrong chain type
		return IKSolver::bInvalidInitialValue;
	}

	BaseInterface* binf = linkChain.GetIKGoal();
	IHIIKGoal* goal_inf = NULL;
	if (binf != NULL) {
		goal_inf = reinterpret_cast<IHIIKGoal*>(binf->GetInterface(kHIIKGoalID));
	}
	if (goal_inf == NULL) {
		return IKSolver::bInvalidArgument;
	}

	switch (linkChain.LinkCount()) 
	{
		
	case 3:
		{ 
				

		IKLimbSolverInitializer(linkChain);

		Point3 wrist_position_in_world =  zeroPoint;		// initializing
		Point3 elbow_position_in_world = zeroPoint;			// initializing
		Point3 ee_axis_unit_vector_world = zeroPoint;		// initializing

		Point3 goal_position_in_world = goal_inf->Goal().GetTrans();
		float goal_distance = FLength(goal_position_in_world);
		Point3 goal_unit_vector_world = Normalize(goal_position_in_world);
		float swivel;



	// Here we check to see if the goal is within the workspace of the chain. For a human limb chain 
	// without joint limits, the reachable (position) workspace (of the wrist) is a spherical shell 
	// with 
	// outer radius = upper_arm_plus_lower_arm_length and 
	// inner radius = upper_arm_minus_lower_arm_length

	// If the goal is OUTSIDE the shell, position the wrist at the outer shell boundary on the
	// ee_axis_unit_vector.

		if (goal_distance > upper_arm_plus_lower_arm_length){
			wrist_position_in_world = goal_unit_vector_world * upper_arm_plus_lower_arm_length;
		}

	// ELSE If, the goal is INSIDE the shell, position the wrist at the inner shell boundary on the
	// ee_axis_unit_vector.

		else if (goal_distance < (float)fabs(upper_arm_minus_lower_arm_length)){
			wrist_position_in_world = goal_unit_vector_world * (float)fabs(upper_arm_minus_lower_arm_length);
		}

	// ELSE, place the wrist at the goal.

		else{
			wrist_position_in_world = goal_position_in_world;
		}
	
	// Since the wrist position might have been changed due to the workspace checking code above,
	// we need to re-set the wrist postion and re-calculate everything that depends on the
	// wrist position.
		ee_axis_unit_vector_world = Normalize(wrist_position_in_world);
		float shoulder_to_wrist_distance = FLength(wrist_position_in_world);

	// some local variables to compute the initial elbow joint angle

//		"gg" below is an intermediate variable for computing the "elbow_joint_angle"
		float gg = upper_arm_length * upper_arm_length + lower_arm_length * lower_arm_length - 
				shoulder_to_wrist_distance * shoulder_to_wrist_distance;
		float cosine_of_elbow_joint_angle = gg/(2 * upper_arm_length * lower_arm_length);

		float elbow_joint_angle = acos_safe(cosine_of_elbow_joint_angle);

//		"gg1" below is an intermediate variable for computing the "chain_plane_ang_at_shoulder"
		float gg1 = shoulder_to_wrist_distance * shoulder_to_wrist_distance + 
			 upper_arm_length * upper_arm_length - 
			 lower_arm_length * lower_arm_length;
		float cosine_of_chain_plane_ang_at_shoulder = gg1/(2 * shoulder_to_wrist_distance * upper_arm_length);

		float chain_plane_ang_at_shoulder = acos_safe(cosine_of_chain_plane_ang_at_shoulder);
		Point3 perpend_to_chain_plane = (*goal_inf->DefaultZeroMap())(ee_axis_unit_vector_world);
		Point3 target_normal;

		if (goal_inf->UseVHTarget()){ // Use the target to compute the Swivel angle

			// linkChain.vhTarget[0] etc are with respect to the IK Chain root
			target_normal = goal_inf->VHTarget();
			target_normal = Normalize(target_normal);

			target_normal -= (ee_axis_unit_vector_world % target_normal) * ee_axis_unit_vector_world;
			if (target_normal.LengthUnify() == 0.0f) {
				target_normal = perpend_to_chain_plane;
			} 
			else {
				target_normal = ee_axis_unit_vector_world ^ target_normal;
			}
		}

		else{

			if (goal_inf->SwivelAngleParent() == kSAInStartJoint) {

				Quat qee = MakeRowQuat(ee_axis_unit_vector_world, goal_inf->SwivelAngle());
				target_normal = ApplyRowQuat((*goal_inf->DefaultZeroMap())(ee_axis_unit_vector_world), qee);
			} 
			else {

				DbgAssert(goal_inf->SwivelAngleParent() == kSAInGoal);
				target_normal.Set(0.0f, 0.0f, 0.0f);
				Point3 ee_goal = Inverse(goal_inf->Goal()).VectorTransform(ee_axis_unit_vector_world);
				if (ee_goal.LengthUnify() != 0.0f) {
					Quat qee = MakeRowQuat(ee_goal, goal_inf->SwivelAngle());
					Point3 t_goal = ApplyRowQuat((*goal_inf->DefaultZeroMap())(ee_goal), qee);
					target_normal = goal_inf->Goal().VectorTransform(t_goal);
					target_normal -= (ee_axis_unit_vector_world % target_normal) * ee_axis_unit_vector_world;
				}
				if (target_normal.LengthUnify() == 0.0f) {
//					// This is of singular situation.
					target_normal = perpend_to_chain_plane;;
				}
			} // kSAInGoal
		}


		swivel = perpend_to_chain_plane % target_normal;
		swivel = acos_safe(swivel);
		Point3 cross = perpend_to_chain_plane ^ target_normal;
		if (cross % ee_axis_unit_vector_world < 0.0f) {
			swivel = 2.0 * PI - swivel;
		}

		Matrix3 swivel_matrix1 = RotAngleAxisMatrix(perpend_to_chain_plane, chain_plane_ang_at_shoulder - initial_chain_plane_ang_at_shoulder);
		Matrix3 swivel_matrix2 = RotAngleAxisMatrix(ee_axis_unit_vector_world, swivel);
		Point3 intermediate_shoulder_rot_x = VectorTransform(swivel_matrix1, ee_axis_unit_vector_world);
		
		Point3 shoulder_rot_x = VectorTransform(swivel_matrix2, intermediate_shoulder_rot_x);
		Point3 shoulder_rot_y = VectorTransform(swivel_matrix2, perpend_to_chain_plane);
		Point3 shoulder_rot_z = Normalize(CrossProd(shoulder_rot_x, shoulder_rot_y));

		Matrix3 shoulder_joint_frame(1);
		
		shoulder_joint_frame.SetRow(0, shoulder_rot_x);
		shoulder_joint_frame.SetRow(1, shoulder_rot_y);
		shoulder_joint_frame.SetRow(2, shoulder_rot_z);
		shoulder_joint_frame.SetRow(3, zeroPoint);

		Matrix3 shoulder_rotation_matrix = Inverse(initial_shoulder_frame) * shoulder_joint_frame;

		// Has to pre-rotate the following angles to put back the
		// shoulder_rotation_matrix - JZhao.
		shoulder_rotation_matrix.PreRotateZ(linkChain.rootLink.initXYZ.z);
		shoulder_rotation_matrix.PreRotateY(linkChain.rootLink.initXYZ.y);
		shoulder_rotation_matrix.PreRotateX(linkChain.rootLink.initXYZ.x);

	// update the linkchain rootlink (the shoulder joint) joint variables
		float Eul[3] = {0.0f ,0.0f ,0.0f};
		MatrixToEuler(shoulder_rotation_matrix, Eul, EULERTYPE_XYZ);
		linkChain.rootLink.rotXYZ.Set(Eul[0], Eul[1], Eul[2]); // will not work if the initial angles are nonzero : Jianmin

	// update the elbow joint variables

		float elbow_euler_angles[3] = {0.0f, 0.0f, 0.0f};
		Matrix3 effective_elbow_rotate = 
			RotAngleAxisMatrix(initial_normal_to_chain_plane_shoulder, (elbow_joint_angle - initial_elbow_joint_angle));

		// Has to pre-rotate the following angles to put back the
		// initial)elbow_joint_angle that you substracted in the
		// preceding line. - JZhao.
		effective_elbow_rotate.PreRotateZ(linkChain.LinkOf(0).initValue);
		effective_elbow_rotate.PreRotateY(linkChain.LinkOf(1).initValue);
		effective_elbow_rotate.PreRotateX(linkChain.LinkOf(2).initValue);

		MatrixToEuler(effective_elbow_rotate, elbow_euler_angles, EULERTYPE_XYZ);

		linkChain.LinkOf(0).dofValue = elbow_euler_angles[2];
		linkChain.LinkOf(1).dofValue = elbow_euler_angles[1];
		linkChain.LinkOf(2).dofValue = elbow_euler_angles[0];

		return 0;


	}
	break;

	case 0:
	{
		IKLimb_1Bone_SolverInitializer(linkChain);

		Point3 elbow_position_in_world = zeroPoint;			// initializing
		Point3 ee_axis_unit_vector_world = zeroPoint;		// initializing

		Point3 goal_position_in_world = goal_inf->Goal().GetTrans();
		Point3 goal_unit_vector_world = Normalize(goal_position_in_world);

		ee_axis_unit_vector_world = goal_unit_vector_world;

		float angle_to_turn;
		angle_to_turn = acos_safe(DotProd(initial_ee_axis_unit_vector_world, ee_axis_unit_vector_world));
		Point3 about_axis;
		about_axis = Normalize(CrossProd(initial_ee_axis_unit_vector_world, ee_axis_unit_vector_world));


		Matrix3 swivel_matrix1 = RotAngleAxisMatrix(about_axis, angle_to_turn);
		
		Point3 shoulder_rot_x = VectorTransform(swivel_matrix1, initial_ee_axis_unit_vector_world);
		Point3 shoulder_rot_y = VectorTransform(swivel_matrix1, initial_shoulder_frame.GetRow(1));
		Point3 shoulder_rot_z = Normalize(CrossProd(shoulder_rot_x, shoulder_rot_y));

		Matrix3 shoulder_joint_frame(1);
		
		shoulder_joint_frame.SetRow(0, shoulder_rot_x);
		shoulder_joint_frame.SetRow(1, shoulder_rot_y);
		shoulder_joint_frame.SetRow(2, shoulder_rot_z);
		shoulder_joint_frame.SetRow(3, zeroPoint);

		Matrix3 shoulder_rotation_matrix = Inverse(initial_shoulder_frame) * shoulder_joint_frame;

		// Has to pre-rotate the following angles to put back the
		// shoulder_rotation_matrix - JZhao.
		shoulder_rotation_matrix.PreRotateZ(linkChain.rootLink.initXYZ.z);
		shoulder_rotation_matrix.PreRotateY(linkChain.rootLink.initXYZ.y);
		shoulder_rotation_matrix.PreRotateX(linkChain.rootLink.initXYZ.x);

	// update the linkchain rootlink (the shoulder joint) joint variables
		float Eul[3] = {0.0f ,0.0f ,0.0f};
		MatrixToEuler(shoulder_rotation_matrix, Eul, EULERTYPE_XYZ);
		linkChain.rootLink.rotXYZ.Set(Eul[0], Eul[1], Eul[2]); // will not work if the initial angles are nonzero : Jianmin
		
		return 0;
	}
	break;

	default:
		return IKSolver::bInvalidInitialValue;;

};


}

