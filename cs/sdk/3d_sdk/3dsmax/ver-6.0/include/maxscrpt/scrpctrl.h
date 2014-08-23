/*	
 *		ScrpCtrl.h - interface to scripter-based expression controllers
 *
 *			John Wainwright
 *			Copyright © Autodesk, Inc. 1997
 */

#ifndef _H_SCRPTCTRL
#define _H_SCRPTCTRL

#define FLOAT_SCRIPT_CONTROL_CNAME			GetString(IDS_RB_SCRIPTFLOAT)
#define FLOAT_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x498702e6, 0x71f11548)

#define POSITION_SCRIPT_CONTROL_CNAME		GetString(IDS_RB_SCRIPTPOSITION)
#define POSITION_SCRIPT_CONTROL_CLASS_ID	Class_ID(0x5065767b, 0x683a42a5)

#define POINT3_SCRIPT_CONTROL_CNAME			GetString(IDS_RB_SCRIPTPOINT3)
#define POINT3_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x46972869, 0x2f7f05ce)

#define POINT4_SCRIPT_CONTROL_CNAME			GetString(IDS_RB_SCRIPTPOINT4)
#define POINT4_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x46972870, 0x2f7f05cf)

#define ROTATION_SCRIPT_CONTROL_CNAME		GetString(IDS_RB_SCRIPTROTATION)
#define ROTATION_SCRIPT_CONTROL_CLASS_ID	Class_ID(0x31381912, 0x3a904166)

#define SCALE_SCRIPT_CONTROL_CNAME			GetString(IDS_RB_SCRIPTSCALE)
#define SCALE_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x7c8f3a2a, 0x1e954d91)

#define PRS_SCRIPT_CONTROL_CNAME			GetString(IDS_RB_SCRIPTPRS)
#define PRS_SCRIPT_CONTROL_CLASS_ID			Class_ID(0x7f56455c, 0x1be66c68)

class IBaseScriptControl : public StdControl {
	public:
		virtual TCHAR*	get_script_text() = 0;
		virtual void	set_script_text(TCHAR* text) = 0;
		virtual bool	update_refs() = 0;
		virtual void	depends_on(ReferenceTarget* ref) = 0;
};

#define push_control(_c)								\
	Control* _save_cc = thread_local(current_controller);	\
	thread_local(current_controller) = _c;
	
#define pop_control()									\
	thread_local(current_controller) = _save_cc;				

#endif
