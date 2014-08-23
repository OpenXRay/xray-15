/*	
 *		WireCtrl.h - interface to parameter-wiring controllers
 *
 *			John Wainwright
 *			Copyright © Autodesk, Inc. 1999
 */

#ifndef _H_WIRETCTRL
#define _H_WIRETCTRL

#define FLOAT_WIRE_CONTROL_CNAME		_T("Float Wire") //GetString(IDS_RB_WIREFLOAT)
#define FLOAT_WIRE_CONTROL_CLASS_ID		Class_ID(0x498702e7, 0x71f11549)

#define POSITION_WIRE_CONTROL_CNAME		_T("Position Wire") //GetString(IDS_RB_WIREPOSITION)
#define POSITION_WIRE_CONTROL_CLASS_ID	Class_ID(0x5065767c, 0x683a42a6)

#define POINT3_WIRE_CONTROL_CNAME		_T("Point3 Wire") //GetString(IDS_RB_WIREPOINT3)
#define POINT3_WIRE_CONTROL_CLASS_ID	Class_ID(0x4697286a, 0x2f7f05cf)

#define ROTATION_WIRE_CONTROL_CNAME		_T("Rotation Wire") //GetString(IDS_RB_WIREROTATION)
#define ROTATION_WIRE_CONTROL_CLASS_ID	Class_ID(0x31381913, 0x3a904167)

#define SCALE_WIRE_CONTROL_CNAME		_T("Scale Wire") //GetString(IDS_RB_WIRESCALE)
#define SCALE_WIRE_CONTROL_CLASS_ID		Class_ID(0x7c8f3a2b, 0x1e954d92)

typedef short ParamID;

class IBaseWireControl : public StdControl {
	public:
		// number of wires, wire param access
		virtual int		get_num_wires()=0;  // how many wires out of this controller (number of dependent params)
		virtual Animatable* get_wire_anim(int i)=0;  // get ith dependent parameter animatable
		virtual int		get_wire_subnum(int i)=0;   // get ith dependent parameter subanim num in the animatable

		// transfer expression script
		virtual TCHAR*	get_expr_text(int i)=0;
		virtual void	set_expr_text(int i, TCHAR* text)=0;

		// animation sub-controller
		virtual void	set_slave_animation(Control* c)=0;
		virtual Control* get_animation()=0;
		virtual bool    is_master()=0;
		virtual bool	is_slave()=0;
};

#endif