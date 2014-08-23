//-------------------------------------------------------------
// Access to the Reactor Controller
//
#include "ikctrl.h"
#include "icurvctl.h"
#include "iFnPub.h"


#define REACTORFLOAT 0x717d7d1f
#define REACTORPOS 0x7ac5cae4
#define REACTORP3 0x19080908
#define REACTORROT 0x2a8734eb
#define REACTORSCALE 0x13c4451c
#define REACTORFLOAT_CLASS_ID	Class_ID(REACTORFLOAT, 0x124c173b)
#define REACTORPOS_CLASS_ID		Class_ID(REACTORPOS, 0x904a56b3)
#define REACTORP3_CLASS_ID		Class_ID(REACTORP3, 0x3b617839)
#define REACTORROT_CLASS_ID		Class_ID(REACTORROT, 0x57f47da6)
#define REACTORSCALE_CLASS_ID	Class_ID(REACTORSCALE, 0x2ccb3388)

#define EDITABLE_SURF_CLASS_ID Class_ID(0x76a11646, 0x12a822fb)

// this is the class for all biped controllers except the root and the footsteps
#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)
// this is the class for the center of mass, biped root controller ("Bip01")
#define BIPBODY_CONTROL_CLASS_ID  Class_ID(0x9156,0) 



#define REACTORDLG_CLASS_ID	0x75a847f9

#define FLOAT_VAR		1
#define VECTOR_VAR		2
#define QUAT_VAR		3
#define SCALE_VAR		4

class IReactor;

/***************************************************************
Function Publishing System stuff   
****************************************************************/

#define REACTOR_INTERFACE Interface_ID(0x53b3498a, 0x18ff6fe7)
#define GetIReactorInterface(cd) \
			(IReactor *)(cd)->GetInterface(REACTOR_INTERFACE)

//****************************************************************


class IReactor : public Control, public FPMixinInterface {
	public:
		
		//Function Publishing System
		enum {  react_to, create_reaction, delete_reaction, get_reaction_count, select_reaction, get_selected_reaction, 
				set_reaction_state, set_float_reaction_state, set_p3_reaction_state, set_quat_reaction_state, 
				set_reaction_value, set_float_reaction_value, set_p3_reaction_value, set_quat_reaction_value, 
				set_reaction_influence, set_reaction_strength, set_reaction_falloff, set_reaction_name, 
				get_reaction_influence, get_reaction_strength, get_reaction_falloff, get_reaction_name, 
				get_reaction_type, reaction_type, 
				get_reaction_state, get_reaction_value, 
				get_editing_state, set_editing_state, get_use_curve, set_use_curve, get_curve,
				};
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFNT_1(react_to, reactTo, TYPE_REFTARG);
			FNT_1(create_reaction, TYPE_BOOL, CreateReaction, TYPE_STRING);
			FN_1(delete_reaction, TYPE_BOOL, DeleteReaction, TYPE_INDEX);
			FN_0(get_reaction_count, TYPE_INT, getReactionCount);
			VFN_1(select_reaction, setSelected, TYPE_INDEX);
			FN_0(get_selected_reaction, TYPE_INDEX, getSelected);
			
			//these could use VarArgs
			FNT_1(set_reaction_state, TYPE_BOOL, setState, TYPE_INDEX);
			FN_2(set_float_reaction_state, TYPE_BOOL, setState, TYPE_INDEX, TYPE_FLOAT);
			FN_2(set_p3_reaction_state, TYPE_BOOL, setState, TYPE_INDEX, TYPE_POINT3);
			FN_2(set_quat_reaction_state, TYPE_BOOL, setState, TYPE_INDEX, TYPE_QUAT);
			//these could also use VarArgs
			FNT_1(set_reaction_value, TYPE_BOOL, setReactionValue, TYPE_INDEX);
			FN_2(set_float_reaction_value, TYPE_BOOL, setReactionValue, TYPE_INDEX, TYPE_FLOAT);
			FN_2(set_p3_reaction_value, TYPE_BOOL, setReactionValue, TYPE_INDEX, TYPE_POINT3);
			FN_2(set_quat_reaction_value, TYPE_BOOL, setReactionValue, TYPE_INDEX, TYPE_QUAT);

			FN_2(set_reaction_influence, TYPE_BOOL, setInfluence, TYPE_INDEX, TYPE_FLOAT);
			FN_2(set_reaction_strength, TYPE_BOOL, setStrength, TYPE_INDEX, TYPE_FLOAT);
			FN_2(set_reaction_falloff, TYPE_BOOL, setFalloff, TYPE_INDEX, TYPE_FLOAT);
			VFN_2(set_reaction_name, setReactionName, TYPE_INDEX, TYPE_STRING);

			FN_1(get_reaction_name, TYPE_STRING, getReactionName, TYPE_INDEX);
			FN_1(get_reaction_influence, TYPE_FLOAT, getInfluence, TYPE_INDEX);
			FN_1(get_reaction_strength, TYPE_FLOAT, getStrength, TYPE_INDEX);
			FN_1(get_reaction_falloff, TYPE_FLOAT, getFalloff, TYPE_INDEX);

			FN_1(get_reaction_state, TYPE_FPVALUE_BV, fpGetState, TYPE_INDEX);
			FN_1(get_reaction_value, TYPE_FPVALUE_BV, fpGetReactionValue, TYPE_INDEX);

			FN_0(get_reaction_type, TYPE_ENUM, getReactionType);
			PROP_FNS(get_editing_state, getEditReactionMode, set_editing_state, setEditReactionMode, TYPE_BOOL);
			PROP_FNS(get_use_curve, useCurve, set_use_curve, useCurve, TYPE_BOOL);
			//FN_0(get_curve, TYPE_REFTARG, getCurveControl);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

		virtual void	reactTo(ReferenceTarget* anim, TimeValue t = GetCOREInterface()->GetTime())=0;
		virtual BOOL	CreateReaction(TCHAR *buf=NULL, TimeValue t = GetCOREInterface()->GetTime())=0; 
		virtual BOOL	DeleteReaction(int i=-1)=0;
		
		virtual int		getReactionCount()=0;
		
		virtual TCHAR*	getReactionName(int i)=0;
		virtual void	setReactionName(int i, TSTR name)=0;
		
		virtual int		getSelected()=0;
		virtual void	setSelected(int i)=0;
		
		virtual int 	getReactionType()=0;
		virtual void	setReactionType(int i)=0;

		virtual float	getInfluence(int num)=0;
		virtual BOOL	setInfluence(int num, float inf)=0;
		
		virtual float	getStrength(int num)=0;
		virtual	BOOL	setStrength(int num, float inf)=0;
		
		virtual float	getFalloff(int num)=0;
		virtual BOOL	setFalloff(int num, float inf)=0;
		
		virtual void*	getState(int num)=0;  //cannot make this part of FP yet
		virtual BOOL	setState(int num=-1, TimeValue t=NULL)=0;
		virtual BOOL	setState(int num, float val)=0;
		virtual BOOL	setState(int num, Point3 val)=0;
		virtual BOOL	setState(int num, Quat val)=0;

		virtual void*	getReactionValue(int i)=0; //cannot make this part of FP yet
		virtual BOOL	setReactionValue(int i=-1, TimeValue t=NULL)=0;
		virtual BOOL	setReactionValue(int num, float val)=0;
		virtual BOOL	setReactionValue(int num, Point3 val)=0;
		virtual BOOL	setReactionValue(int num, Quat val)=0;

		virtual void	setEditReactionMode(BOOL edit)=0;
		virtual BOOL	getEditReactionMode()=0;

		virtual Point3	getUpVector()=0;
		virtual void	setUpVector(Point3 up)=0;

		virtual int 	getType()=0;

		virtual BOOL	useCurve()=0;
		virtual void	useCurve(BOOL use)=0;
		virtual ICurveCtl*	getCurveControl()=0;

	private:
		//maxscript wrapper functions
		virtual FPValue fpGetReactionValue(int index)=0;
		virtual FPValue fpGetState(int index)=0;
	};

