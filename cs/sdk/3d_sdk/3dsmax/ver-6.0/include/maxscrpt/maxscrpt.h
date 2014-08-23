/*		MAXScript.h - main include for MAXScript sources
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_MAXSCRIPT
#define _H_MAXSCRIPT

#ifndef STRICT
#define STRICT					// strict type-checking - conformance with MAX SDK libs
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// trims win32 includes
#endif

#ifdef BLD_MAXSCRIPT
#	define ScripterExport __declspec( dllexport )
#else
#	define ScripterExport __declspec( dllimport )
#	define IMPORTING
#endif
#include "export.h"


#include <stdlib.h> 
#include <stdio.h>
#include <windows.h>
#include <float.h>

#include "Max.h"

#define NOT_SUPPORTED_BY_PRODUCT(fn)	\
    throw RuntimeError (_T("Feature not available: "), _T(#fn)); 

/* utility defines */

#define END				NULL					// null varargs arg list terminator

class Value;
class CharStream;
class Rollout;
class MAXScriptException;
class MSPlugin;
class Struct;
class MSZipPackage;
class String;

#define MAXSCRIPT_UTILITY_CLASS_ID	Class_ID(0x4d64858, 0x16d1751d)
#define MAX_SCRIPT_DIR				_T("scripts")
#define SCRIPT_AUTOLOAD_DIR			_T("Startup\\")

// check whether we are UNICODE or Code page 0 (==> no mbcs code)
#ifdef _UNICODE
#	define	no_mb_chars		TRUE
#	define	bytelen(s)		(sizeof(wchar_t) * wcslen(s))
#else
#	define	no_mb_chars     (MB_CUR_MAX == 1)
#	define	bytelen(s)		strlen(s)
#endif

inline double EPS(double v) { return _isnan(v) ? (v) : fabs(v) < FLT_EPSILON ? 0.0 : (v); }  // small number round down for %g float printing

/* MAXScript-specific window messages */

#define MXS_ADD_ROLLOUT_PAGE		(WM_USER + 0x100)
#define MXS_DELETE_ROLLOUT_PAGE		(WM_USER + 0x101)
#define MXS_REDRAW_VIEWS			(WM_USER + 0x102)
#define MXS_EDIT_SCRIPT				(WM_USER + 0x103)
#define MXS_NEW_SCRIPT				(WM_USER + 0x104)
#define MXS_DISPLAY_BITMAP			(WM_USER + 0x105)
#define MXS_ERROR_MESSAGE_BOX		(WM_USER + 0x106)
#define MXS_PRINT_STRING			(WM_USER + 0x107)
#define MXS_LISTENER_EVAL			(WM_USER + 0x108)
#define MXS_MESSAGE_BOX				(WM_USER + 0x109)
#define MXS_INITIALIZE_MAXSCRIPT	(WM_USER + 0x10A)
#define MXS_KEYBOARD_INPUT			(WM_USER + 0x10B)
#define MXS_SHOW_SOURCE				(WM_USER + 0x10C)
#define MXS_TAKE_FOCUS				(WM_USER + 0x10D)
#define MXS_STOP_CREATING			(WM_USER + 0x10E)
#define MXS_CLOSE_DOWN				(WM_USER + 0x10F)
#define MXS_STOP_EDITING			(WM_USER + 0x110)
#define MXS_LOAD_STARTUP_SCRIPTS	(WM_USER + 0x111)
#define MXS_EXECUTE_MACRO			(WM_USER + 0x112)
#define MXS_RESTART_EDITING			(WM_USER + 0x113)

typedef struct			// LPARAM for MXS_MESSAGE_BOX contains a pointer to this structure
{
	TCHAR* title;
	TCHAR* message;
	BOOL beep;
	int flags;
	BOOL result;
} message_box_data;

/* thread-local storage struct decl & access macros */

typedef struct MAXScript_TLS MAXScript_TLS;
struct MAXScript_TLS
{
	MAXScript_TLS*	next;				/* links...							*/
	MAXScript_TLS*	prev;
	HANDLE			my_thread;			/* thread that owns this TLS struct	*/
	DWORD			my_thread_id;

#undef def_thread_local
#define def_thread_local(type, lcl, init_val) type lcl
#	include "thrdlcls.h"

};

#define thread_local(x)			(((MAXScript_TLS*)TlsGetValue(thread_locals_index))->x)

/* index, tls struct list globals */

extern ScripterExport int thread_locals_index;
extern int thread_id_index;
extern MAXScript_TLS* MAXScript_TLS_list;

#define needs_redraw_set()			thread_local(needs_redraw) = 1
#define needs_complete_redraw_set()	thread_local(needs_redraw) = 2
#define needs_redraw_clear()		thread_local(needs_redraw) = 0

#define MAXScript_time()	\
	(thread_local(use_time_context) ? thread_local(current_time) : MAXScript_interface->GetTime())

			   void alloc_thread_locals();
ScripterExport void init_thread_locals();
			   void free_thread_locals();

/* error handlers */

void out_of_memory();
void bad_delete();

/* arg count check & keyword arg accessors (assume conventional names for arg_list & count in using function) */

extern ScripterExport Value* _get_key_arg(Value** arg_list, int count, Value* key_name);
extern ScripterExport Value* _get_key_arg_or_default(Value** arg_list, int count, Value* key_name, Value* def);

#define key_arg(key)					_get_key_arg(arg_list, count, n_##key)
#define key_arg_or_default(key, def)	_get_key_arg_or_default(arg_list, count, n_##key##, def)
#define int_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_int())
#define float_key_arg(key, var, def)	((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_float())
#define bool_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_bool())
#define interval_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_interval())
#define timevalue_key_arg(key, var, def)	((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_timevalue())
#define node_key_arg(key, var, def)			((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_node())

#define check_arg_count(fn, w, g)			if ((w) != (g)) throw ArgCountError (_T(#fn), w, g)
#define check_gen_arg_count(fn, w, g)		if ((w) != (g + 1)) throw ArgCountError (_T(#fn), w, g + 1)
#define check_arg_count_with_keys(fn, w, g) if (!(g == w || (g > w && arg_list[w] == &keyarg_marker))) throw ArgCountError (_T(#fn), w, count_with_keys())
#define check_gen_arg_count_with_keys(fn, w, g) if (!(g == w || (g > w && arg_list[w-1] == &keyarg_marker))) throw ArgCountError (_T(#fn), w, count_with_keys() + 1)
#define count_with_keys()					_count_with_keys(arg_list, count)

/* for functions that return a boolean */

#define bool_result(val)					((val) ? &true_value : &false_value)

/* for testing a value to ensure that it is within range */

// following only valid for integer and float range checking
#define MXS_range_check(_val, _lowerLimit, _upperLimit, _desc)				\
	if (_val < _lowerLimit || _val > _upperLimit) {							\
		TCHAR buf[256];														\
		TCHAR buf2[128];													\
		_tcscpy(buf,_desc);													\
		_tcscat(buf,_T(" < "));												\
		_sntprintf(buf2, 128, _T("%g"), EPS(_lowerLimit));					\
		_tcscat(buf,buf2);													\
		_tcscat(buf,_T(" or > "));											\
		_sntprintf(buf2, 128, _T("%g"), EPS(_upperLimit));					\
		_tcscat(buf,buf2);													\
		_tcscat(buf,_T(": "));												\
		_sntprintf(buf2, 128, _T("%g"), EPS(_val));							\
		_tcscat(buf,buf2);													\
		throw RuntimeError (buf);											\
	}


/* value local macros - for managing C local variable references to Value*'s for the collector - see Collectable.cpp */

#define one_value_local(n1)											\
	struct { int count; Value** link; Value* n1; } vl =				\
		{ 1, NULL, NULL };											\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define one_typed_value_local(n1)									\
	struct { int count; Value** link; n1; } vl =					\
		{ 1, NULL, NULL };											\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define two_value_locals(n1, n2)									\
	struct { int count; Value** link; Value *n1, *n2; } vl =		\
		{ 2, NULL, NULL, NULL };									\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define two_typed_value_locals(n1, n2)								\
	struct { int count; Value** link; n1; n2; } vl =				\
		{ 2, NULL, NULL, NULL };									\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define three_value_locals(n1, n2, n3)								\
	struct { int count; Value** link; Value *n1, *n2, *n3; } vl =	\
		{ 3, NULL, NULL, NULL, NULL };								\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define three_typed_value_locals(n1, n2, n3)						\
	struct { int count; Value** link; n1; n2; n3; } vl =			\
		{ 3, NULL, NULL, NULL, NULL };								\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define four_value_locals(n1, n2, n3, n4)								\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4; } vl =	\
		{ 4, NULL, NULL, NULL, NULL, NULL };							\
	vl.link = thread_local(current_locals_frame);						\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define four_typed_value_locals(n1, n2, n3, n4)						\
	struct { int count; Value** link; n1; n2; n3; n4; } vl =		\
		{ 4, NULL, NULL, NULL, NULL, NULL };						\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define five_value_locals(n1, n2, n3, n4, n5)								\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5; } vl =	\
		{ 5, NULL, NULL, NULL, NULL, NULL, NULL };						    \
	vl.link = thread_local(current_locals_frame);						    \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define five_typed_value_locals(n1, n2, n3, n4, n5)							\
	struct { int count; Value** link; n1; n2; n3; n4; n5; } vl =			\
		{ 5, NULL, NULL, NULL, NULL, NULL, NULL };						    \
	vl.link = thread_local(current_locals_frame);						    \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define six_value_locals(n1, n2, n3, n4, n5, n6)								 \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6; } vl = \
		{ 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						 \
	vl.link = thread_local(current_locals_frame);								 \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define six_typed_value_locals(n1, n2, n3, n4, n5, n6)						\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; } vl =		\
		{ 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					\
	vl.link = thread_local(current_locals_frame);							\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define seven_value_locals(n1, n2, n3, n4, n5, n6, n7)								  \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7; } vl = \
		{ 7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						  \
	vl.link = thread_local(current_locals_frame);									  \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define seven_typed_value_locals(n1, n2, n3, n4, n5, n6, n7)				\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; } vl =	\
		{ 7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };				\
	vl.link = thread_local(current_locals_frame);							\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define eight_value_locals(n1, n2, n3, n4, n5, n6, n7, n8)								   \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8; } vl = \
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					   \
	vl.link = thread_local(current_locals_frame);										   \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define eight_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8)				\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; } vl =	\
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };			\
	vl.link = thread_local(current_locals_frame);								\
	thread_local(current_locals_frame) = (Value**)&vl;

// LAM - 6/07/02 - added new defines - no SDK impact	
#define nine_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9)									\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9; } vl = \
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						\
	vl.link = thread_local(current_locals_frame);												\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define nine_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9)					\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; n9; } vl =	\
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };			\
	vl.link = thread_local(current_locals_frame);									\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define value_local_array(var, count) {								\
	var = &((Value**)_alloca(((count) + 2) * sizeof(Value*)))[2];	\
	memset(var, 0, (count) * sizeof(Value*));						\
	var[-2] = (Value*)(count);										\
	var[-1] = (Value*)thread_local(current_locals_frame);			\
	thread_local(current_locals_frame) = &var[-2]; }

#define pop_value_local_array(var)									\
	thread_local(current_locals_frame) = (Value**)var[-1];
	
#define value_temp_array(var, count)	{							\
	var = &((Value**)malloc(((count) + 2) * sizeof(Value*)))[2];	\
	memset(var, 0, (count) * sizeof(Value*));						\
	var[-2] = (Value*)(count);										\
	var[-1] = (Value*)thread_local(current_locals_frame);			\
	thread_local(current_locals_frame) = &var[-2];}

// LAM - 6/07/02 - fix for when multiple value_temp_arrays in same frame
#define realloc_value_temp_array(var, count, old_count)	{						\
	Value **oldPointer = &var[-2];												\
	Value **sframe = thread_local(current_locals_frame);						\
	var = &((Value**)realloc(&var[-2], ((count) + 2) * sizeof(Value*)))[2];		\
	if ((count) > (old_count))													\
		memset(&var[(old_count)], 0, ((count) - (old_count)) * sizeof(Value*));	\
	var[-2] = (Value*)(count);													\
	Value **newPointer = &var[-2];												\
	if (sframe == oldPointer)													\
	{	thread_local(current_locals_frame) = newPointer;						\
		DbgAssert( newPointer != (Value**)newPointer[1] );						\
	}																			\
	else																		\
	{	Value **frame;															\
		for (frame = sframe; frame && (Value**)frame[1] != oldPointer && frame != (Value**)frame[1]; frame = (Value**)frame[1]);	\
		if (frame) {															\
			DbgAssert( frame != (Value**)frame[1] );							\
			frame[1] = (Value*)newPointer;										\
		}																		\
	}																			\
	}

// LAM - 6/07/02 - fix for when multiple value_temp_arrays in same frame
#define pop_value_temp_array(var)	{							\
	Value **oldPointer = &var[-2];								\
/*	Value **sframe = thread_local(current_locals_frame); */		\
	Value **head = (Value**)var[-1];							\
	thread_local(current_locals_frame) = head;					\
	free(oldPointer);											\
	}
	
#define return_value(r)	{										\
	thread_local(current_result) = r;							\
	thread_local(current_locals_frame) = vl.link;				\
	return r; }
	 
#define return_value_no_pop(r) {								\
	thread_local(current_result) = r;							\
	return r; }

#define return_protected(r) {									\
	thread_local(current_result) = r;							\
	return r; }

#define pop_value_locals()										\
	thread_local(current_locals_frame) = vl.link;

#define reset_locals_frame()									\
	thread_local(current_locals_frame) = (Value**)&vl;

#define reset_locals_array_frame(var)							\
	thread_local(current_locals_frame) = &var[-2]; 

#define clear_current_frames()									\
	thread_local(current_locals_frame) = NULL;					\
	thread_local(current_frame) = NULL;

#define save_current_frames()									\
	Value** _sclf = thread_local(current_locals_frame);			\
	Value** _scsf = thread_local(current_scan_frame);			\
	Value** _scf = thread_local(current_frame);
	
#define restore_current_frames()								\
	thread_local(current_locals_frame) = _sclf;					\
	thread_local(current_scan_frame) = _scsf;					\
	thread_local(current_frame) = _scf;

/* general utilities */

ScripterExport TCHAR* save_string(TCHAR* str);
TCHAR  wputch(HWND w, TCHAR* buf, TCHAR* bufp, const TCHAR c);					/* edit window output... */
TCHAR* wputs(HWND w, TCHAR* buf, TCHAR* bufp, const TCHAR *str);		
int    wprintf(HWND w, TCHAR* buf, TCHAR* bufp, const TCHAR *format, ...);
void   wflush(HWND w, TCHAR* buf, TCHAR* bufp);		
#define mputs	thread_local(current_stdout)->puts	/* current MAXScript stdout output... */
#define mprintf	thread_local(current_stdout)->printf
#define mflush	thread_local(current_stdout)->flush
extern TCHAR *GetString(int id);
 
class Rollout;
class HashTable;
ScripterExport void  install_utility_page(Rollout* rollout);

class Value;
typedef Value* (Value::*value_vf)(Value**, int);
typedef Value* (*value_cf)(Value**, int);
typedef Value* (Value::*getter_vf)(Value**, int);
typedef Value* (Value::*setter_vf)(Value**, int);
typedef Value* (*max_getter_cf)(ReferenceTarget*, Value*, TimeValue, Interval&);
typedef void   (*max_setter_cf)(ReferenceTarget*, Value*, TimeValue, Value*);

/* MAXScript signal flags */

#define INTERRUPT_EVAL		0x0001
#define EXIT_LISTENER		0x0002

extern ScripterExport Interface* MAXScript_interface;
extern ScripterExport int		MAXScript_signals;
extern ScripterExport BOOL		escape_enabled; // RK: 05/20/02, 5.0 or later only
extern ScripterExport BOOL		check_maxscript_interrupt;
extern ScripterExport void		escape_checker();
extern ScripterExport BOOL		MAXScript_detaching;
extern ScripterExport int		mxs_rand();
extern ScripterExport void		mxs_seed(int);
extern ScripterExport void		dlx_detaching(HINSTANCE hinstance);
extern ScripterExport void		define_system_global(TCHAR* name, Value* (*getter)(), Value* (*setter)(Value*));
// LAM 4/1/00 - added following to be able to overwrite existing global value in hash table.
extern ScripterExport void		define_system_global_replace(TCHAR* name, Value* (*getter)(), Value* (*setter)(Value*));
extern ScripterExport void		define_struct_global(TCHAR* name, TCHAR* struct_name, Value* (*getter)(), Value* (*setter)(Value*));
extern ScripterExport HashTable* english_to_local;
extern ScripterExport HashTable* local_to_english;
extern ScripterExport BOOL		non_english_numerics;
extern ScripterExport void	    printable_name(TSTR& name);
extern ScripterExport void		show_source_pos();
extern ScripterExport void		show_listener();

extern ScripterExport void		init_MAXScript();
extern ScripterExport BOOL		MAXScript_running;
extern ScripterExport HWND		main_thread_window;
extern ScripterExport BOOL		progress_bar_up;
extern ScripterExport BOOL		trace_back_active;
extern ScripterExport BOOL		disable_trace_back;
typedef void (*utility_installer)(Rollout* ro);
extern ScripterExport void		set_utility_installer(utility_installer ui);
extern ScripterExport void		reset_utility_installer();
extern ScripterExport void		error_message_box(MAXScriptException& e, TCHAR* caption);
typedef Value* (*autocad_point_reader)(TCHAR* str);
extern ScripterExport void		set_autocad_point_reader(autocad_point_reader apr);

// LAM - 4/28/03
// Returns TRUE if script was executed successfully. If quietErrors == false and net rendering, errors are logged to LogSys. 
// If not net rendering, errors are logged to Listener. Return value from script stored in fpv, if specified.
extern ScripterExport BOOL		ExecuteMAXScriptScript(TCHAR *s, BOOL quietErrors = FALSE, FPValue *fpv = NULL);

// LAM - 6/24/03
// Method for processing input value or array for default action value. Current recognized values are:
// #logMsg, #logToFile, #abort, and integer values. Actions are converted to DWORD where bit 0 is log to Default Action 
// system log, bit 1 is log to log file, and bit 2 is abort/cancel (if applicable).
extern ScripterExport DWORD		ProcessDefaultActionVal (Value* inpActionVal, DWORD defaultAction = DEFAULTACTIONS_LOGMSG);

#define check_interrupts()	if (check_maxscript_interrupt) escape_checker(); if (MAXScript_signals) throw SignalException()

#define type_check(val, cl, where) if (val->tag != class_tag(cl)) throw TypeError (where, val, &cl##_class);

// macros for setting numeric printing to English locale and back again - all numeric output in MAXScript is English
#define set_english_numerics()						\
	TCHAR* locale;									\
	TCHAR slocale[256];								\
	if (non_english_numerics != NULL)				\
	{												\
		locale = setlocale(LC_NUMERIC, NULL);		\
		_tcsncpy(slocale, locale, sizeof(slocale));	\
		setlocale(LC_NUMERIC, "C");					\
	}

#define reset_numerics()							\
	if (non_english_numerics != NULL)				\
		setlocale(LC_NUMERIC, slocale);	

#include "Excepts.h"
#include "iFnPub.h"
#include "Value.h"
#include "Streams.h"
#include "SceneIO.h"
#include "IParamm2.h"

class HashTable;
class Listener;
extern ScripterExport HashTable* globals;
extern ScripterExport HashTable* persistents;
extern ScripterExport Listener*  the_listener;
extern ScripterExport HWND		  the_listener_window;
extern                HINSTANCE  hInstance;
extern ScripterExport void       listener_message(UINT iMsg, WPARAM wParam, LPARAM lParam, BOOL block_flag);

class RandGenerator;
extern ScripterExport RandGenerator *ClassIDRandGenerator;

inline int _count_with_keys(Value** arg_list, int count)
{
	// compute # args before any key-args
	for (int i = 0; i < count; i++)
		if (arg_list[i] == (Value*)&keyarg_marker)
			return i;
	return count;
}

#endif
