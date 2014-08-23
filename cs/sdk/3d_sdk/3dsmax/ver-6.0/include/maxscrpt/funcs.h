/*		Functions.h - the Function family class - primitives, generics
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#ifndef _H_FUNCTION
#define _H_FUNCTION

#undef def_generic
#define def_generic(fn, name)	\
			ScripterExport Value* fn##_vf(Value** arglist, int arg_count)

#define FPS_CACHE_SIZE 512

/* --- function base class -- */

visible_class (Function)

class Function : public Value
{
public:
	TCHAR*		name;
	TCHAR*		struct_name;	// packaged in a struct if non-null

				   Function() { name = NULL; struct_name = NULL; }
	ScripterExport Function(TCHAR* name, TCHAR* struct_name=NULL);
	ScripterExport ~Function();

				classof_methods (Function, Value);
#	define		is_function(o) ((o)->_is_function())
	BOOL		_is_function() { return 1; }

	ScripterExport void sprin1(CharStream* s);
	ScripterExport void export_to_scripter();
};

/* ----------------  call context base class ----------------- */

class CallContext 
{
	CallContext* previous;
public:
	CallContext() : previous(NULL) { }
	CallContext(CallContext* previous) : previous(previous) { }

	// called by fn applier to establish context AFTER arguments eval'd	
	virtual void push_context() { if (previous) previous->push_context(); }
	virtual void pop_context() { if (previous) previous->pop_context(); }
};

/* ----------------------- Generics ------------------------- */

visible_class (Generic)

class Generic : public Function
{
public:
	value_vf	fn_ptr;

				Generic() { }
 ScripterExport Generic(TCHAR* name, value_vf fn, TCHAR* struct_name = NULL);
			    Generic(TCHAR* name) : Function(name) { }

				classof_methods (Generic, Function);
	BOOL		_is_function() { return 1; }
	ScripterExport void init(TCHAR* name, value_vf fn);
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (MappedGeneric)

class MappedGeneric : public Generic
{
public:
				MappedGeneric() { }
 ScripterExport MappedGeneric(TCHAR* name, value_vf fn);
			    MappedGeneric(TCHAR* name) : Generic(name) { }

				classof_methods (MappedGeneric, Generic);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (NodeGeneric)

class NodeGeneric : public MappedGeneric
{
public:
 ScripterExport NodeGeneric(TCHAR* name, value_vf fn);
			    NodeGeneric(TCHAR* name) : MappedGeneric(name) { }

				classof_methods (NodeGeneric, MappedGeneric);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

/* -------------------------- Primitives ------------------------------ */

#define LAZY_PRIMITIVE	0x0001

visible_class (Primitive)

class Primitive : public Function
{ 
public:
	short		flags;
	value_cf	fn_ptr;

			    Primitive() { }
 ScripterExport Primitive(TCHAR* name, value_cf fn, short init_flags=0);
 ScripterExport Primitive(TCHAR* name, TCHAR* structure, value_cf fn, short init_flags=0);
			    Primitive(TCHAR* name) : Function(name) { }

				classof_methods (Primitive, Function);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (MappedPrimitive)

class MappedPrimitive : public Primitive
{ 
public:
 ScripterExport MappedPrimitive(TCHAR* name, value_cf fn);

				classof_methods (MappedPrimitive, Primitive);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

/* ----- */

visible_class (MAXScriptFunction)

class MAXScriptFunction : public Function
{
public:
	short		parameter_count;
	short		local_count;
	short		keyparm_count;
	short		flags;
	Value**		keyparms;
	Value*		body;
	HashTable*	local_scope;
	value_cf	c_callable_fn;

 ScripterExport MAXScriptFunction(TCHAR* name, int parm_count, int keyparm_count, Value** keyparms,
								  int local_count, Value* body, HashTable* local_scope, short flags = 0);
				~MAXScriptFunction();

				classof_methods (MAXScriptFunction, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
	Value*		apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL);

	value_cf	get_c_callable_fn();

	Value*		operator()(Value** arg_list, int count);
};

#define FN_MAPPED_FN	0x0001		// declared a collection-mapped function
#define FN_BODY_FN		0x0002		// a loop or other body function, don't trap exits here
#define FN_HAS_REFARGS	0x0004		// function has reference arguments
#define FN_MAPPED_EVAL	0x0008		// set while evaluating a mapped function on each item


// UserProp & UserGeneric instances represent dynamically-added, user-defined generics
//  on built-in classes.  They are kept in sorted tables in ValueMetaClass instances,
// suitable for bsearching.
class UserProp
{
public:
	Value*		prop;
	value_cf	getter;
	value_cf	setter;
				UserProp (Value* p, value_cf g, value_cf s) { prop = p; getter = g; setter = s; }
};

class UserGeneric
{
public:
	Value*		name;
	value_cf	fn;
				
				UserGeneric(Value* n, value_cf f) { name = n; fn = f; }
};

// UserGenericValue is the scripter-visible generic fn value that dispatches the
// UserGeneric 'methods' in a target object's class

visible_class (UserGenericValue)

class UserGenericValue : public Function
{
public:
	Value*		fn_name;
	Value*		old_fn;   // if non-NULL, the original global fn that this usergeneric replaced
				
 ScripterExport UserGenericValue(Value* name, Value* old_fn);

				classof_methods (UserGenericValue, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

#define	def_user_prop(_prop, _cls, _getter, _setter)		\
	_cls##_class.add_user_prop(#_prop, _getter, _setter)

#define	def_user_generic(_fn, _cls, _name)					\
	_cls##_class.add_user_generic(#_name, _fn)


// ------- MAXScript Function Publishing interface ----------------------

#include "iFnPub.h"

class InterfaceMethod;
class FPMixinInterfaceValue;
class FPEnum;

// FnPub function, a function published by a plugin using theFnPub system
//     automatically exposed by MAXScript boot code during intial plugin scan

visible_class (InterfaceFunction)

class InterfaceFunction : public Function
{
public:
	FPInterfaceDesc* fpid;
	FPFunctionDef*   fd;

				InterfaceFunction(FPInterface* fpi, FPFunctionDef* fd);
				~InterfaceFunction();

				classof_methods (InterfaceFunction, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
	Value*		get_property(Value** arg_list, int count);

	// parameter conversion utilities
	static void	  val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e=NULL);
	static Value* FPValue_to_val(FPValue& fpv, FPEnum* e=NULL);
	static void	  release_param(FPValue& fpv, ParamType2 type, Value* v, FPEnum* e=NULL);
	static void	  init_param(FPValue& fpv, ParamType2 type);
	static void	  validate_params(FPInterface* fpi, FunctionID fid, FPParamDef* pd, ParamType2 type, int paramNum, FPValue& val, Value* v);
	static FPEnum* FindEnum(short id, FPInterfaceDesc* fpid);
};

// InterfaceMethod - wraps an InterfaceFunction and its target object for shorthand mixin calls
class InterfaceMethod : public InterfaceFunction
{
private:
				InterfaceMethod(FPMixinInterfaceValue* fpiv, FPFunctionDef* fd);
	static		InterfaceMethod* interface_method_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPMixinInterfaceValue* fpiv;

	static ScripterExport InterfaceMethod* intern(FPMixinInterfaceValue* fpiv, FPFunctionDef* fd);
				~InterfaceMethod();

				def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

// Action predicate function wrappers...
visible_class (ActionPredicate)

class ActionPredicate : public InterfaceFunction
{
public:
	short		pred;

	ActionPredicate(FPInterface* fpi, FPFunctionDef* fd, short pred);

				classof_methods (ActionPredicate, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

// IObject, generic wrapper for objects that inherit from IObject
//    this is a way to give simple interface-based wrappers to 
//    classes in MAX or 3rd-party plugins that MAXScript knows
//    nothing about.  The IObject instance must implement GetInterface()

visible_class (IObjectValue)

class IObjectValue : public Value
{
public:
	IObject* iobj;					// the IObject pointer

			IObjectValue(IObject* io);
		   ~IObjectValue();
#	define	is_iobject(c) ((c)->tag == class_tag(IObjectValue))

			classof_methods (IObjectValue, Value);
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

// The following methods have been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

	void	to_fpvalue(FPValue& v) { v.iobj = iobj; v.type = TYPE_IOBJECT; }
	// LAM; added following 2/15/01 - defect 275812. Per conversation with AdamF and Ravi,
	// adding this doesn't break binary compatability
	def_generic (show_interfaces,  "showInterfaces"); 

// End of 3ds max 4.2 Extension

	BaseInterface* GetInterface(Interface_ID id) { return iobj->GetInterface(id); }

	//accesses interfaces on the IObject
	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// FPInterfaceValue, represents  FPInterfaces in MAXScript

visible_class (FPInterfaceValue)

class FPInterfaceValue : public Value, public InterfaceNotifyCallback
{
public:
	FPInterface* fpi;			// interface
	HashTable*	 fns;			// interface fn lookup
	HashTable*	 props;			// interface prop lookup
	FPInterface::LifetimeType lifetime;	// interface lifetime control type
	static bool	 enable_test_interfaces;  // test interface enable flag
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0
			ScripterExport FPInterfaceValue(FPInterface* fpi);
		   ~FPInterfaceValue();
#	define	is_fpstaticinterface(c) ((c)->tag == class_tag(FPInterfaceValue))

			classof_methods (FPInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void	collect() { delete this; }
	void	gc_trace();
	void	sprin1(CharStream* s);

// The following method has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

	void	to_fpvalue(FPValue& v) { v.fpi = fpi; v.type = TYPE_INTERFACE; }

// End of 3ds max 4.2 Extension

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi) { fpi = NULL; }  

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

extern ScripterExport void print_FP_interface(CharStream* out, FPInterface* fpi, bool getPropNames = true, 
			   bool getMethodNames = true, bool getInterfaceNames = true, bool getActionTables = true);

// FPMixinInterfaceValue provides wrappers for mixin interfaces on individual target objects
//  stored in a cache for fast retrieval and to minimize over-consing
// Warning: FPMixinInterfaceValue can wrap a FPStaticInterface. If accessing FPMixinInterface
// specific items, test 'fpi->GetDesc()->flags & FP_MIXIN' first.

visible_class (FPMixinInterfaceValue)

class FPMixinInterfaceValue : public Value, public InterfaceNotifyCallback
{
private:
			FPMixinInterfaceValue(FPInterface* fpi);
		   ~FPMixinInterfaceValue();
	static  FPMixinInterfaceValue* FPMixinInterfaceValue::interface_cache[128];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPInterface* fpi;						// interface
	FPInterface::LifetimeType lifetime;		// interface lifetime control type
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0

	static  ScripterExport FPMixinInterfaceValue* intern(Value* prop_name, Value* target);
	static  ScripterExport FPMixinInterfaceValue* intern(FPInterface* fpi);
#	define	is_fpmixininterface(c) ((c)->tag == class_tag(FPMixinInterfaceValue))

			classof_methods (FPMixinInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

// The following method has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

	void	to_fpvalue(FPValue& v) { v.fpi = fpi; v.type = TYPE_INTERFACE; }

// End of 3ds max 4.2 Extension

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi); 

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// FPStaticMethodInterfaceValue provides wrappers for static interfaces that
// have been registered with individual Value metaclasses as property
// interfaces on instances of the metaclasses' class, such that calls
// on methods in these interfaces pass the intance along as the first
// argument wrapped in an FPValue.

// these are used to allow factored static interfaces (such as meshOps)
// to appear as though they are mixin interfaces on several MAXScript value
// classes (such as node, baseobject, meshvalue), in which the target object
// is sent as a polymorphic first argument (via FPValue) to static interface
// method call, rather than as a 'this' pointer to a virtual mixin interface method

visible_class (FPStaticMethodInterfaceValue)

class FPStaticMethodInterfaceValue : public Value, public InterfaceNotifyCallback
{
private:
			FPStaticMethodInterfaceValue(FPInterface* fpi, ParamType2 type, void* object);
		   ~FPStaticMethodInterfaceValue();
	static  FPStaticMethodInterfaceValue* interface_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPInterface* fpi;			// interface
	FPValue		 value;         // the target object as FPValue first argument
	FPInterface::LifetimeType lifetime;	// interface lifetime control type
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0

	static  ScripterExport FPStaticMethodInterfaceValue* intern(FPInterface* fpi, ParamType2 type, void* object);
#	define	is_fpstaticmethodinterface(c) ((c)->tag == class_tag(FPStaticMethodInterfaceValue))

			classof_methods (FPStaticMethodInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi) { fpi = NULL; }  

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// StaticInterfaceMethod - wraps an FPStaticMethodInterfaceValue and its target object for property-based calls
class StaticInterfaceMethod : public InterfaceFunction
{
private:
				StaticInterfaceMethod(FPStaticMethodInterfaceValue* fpiv, FPFunctionDef* fd);
	static		StaticInterfaceMethod* interface_method_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPStaticMethodInterfaceValue* fpiv;

	static ScripterExport StaticInterfaceMethod* intern(FPStaticMethodInterfaceValue* fpiv, FPFunctionDef* fd);
				~StaticInterfaceMethod();

				def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

#endif
