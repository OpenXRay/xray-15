/*		LclClass.h - macros for defining local classes and generics in a MAXScript extension .dlx
 *
 *	
 *		Copyright (c) Autodesk, Inc.  1988.  John Wainwright.
 *
 */

#ifndef _H_LOCALCLASS
#define _H_LOCALCLASS

#include "ClassCfg.h"

#define local_visible_class(_cls)										\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
				_cls##Class(TCHAR* name) : ValueMetaClass (name) { }	\
		void	collect() { delete this; }								\
	};																	\
	extern _cls##Class _cls##_class;

#define local_applyable_class(_cls)										\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
				_cls##Class(TCHAR* name) : ValueMetaClass (name) { }	\
		void	collect() { delete this; }								\
		Value* apply(Value** arglist, int count, CallContext* cc=NULL); \
	};																	\
	extern _cls##Class _cls##_class;

#define local_visible_class_instance(_cls, _name)						\
	_cls##Class _cls##_class (_T(_name));

class MS_LOCAL_ROOT_CLASS;
typedef Value* (MS_LOCAL_ROOT_CLASS::*local_value_vf)(Value**, int);
#define cat0(_a) _a
#define cat1(_a) cat0(_a)
#define cat2(_a, _b) cat0(_a)##cat0(_b)
#define MS_LOCAL_ROOT_CLASS_TAG &cat2(MS_LOCAL_ROOT_CLASS, _class)
#define MS_LOCAL_GENERIC_CLASS_TAG &cat2(MS_LOCAL_GENERIC_CLASS, _class)
#define MS_LOCAL_GENERIC_CLASS_CLASS cat2(MS_LOCAL_GENERIC_CLASS, Class)
#define MS_LOCAL_GENERIC_CLASS_class cat2(MS_LOCAL_GENERIC_CLASS, _class)
#define str0(_c) #_c
#define str1(_c) str0(_c)

#define declare_local_generic_class										\
	class MS_LOCAL_GENERIC_CLASS_CLASS : public ValueMetaClass			\
	{																	\
	public:																\
				MS_LOCAL_GENERIC_CLASS_CLASS(TCHAR* name) : ValueMetaClass (name) { } \
		void	collect() { delete this; }								\
	};																	\
	extern MS_LOCAL_GENERIC_CLASS_CLASS MS_LOCAL_GENERIC_CLASS_class;	\
	class MS_LOCAL_GENERIC_CLASS : public Generic						\
	{																	\
	public:																\
				local_value_vf	fn_ptr;									\
				MS_LOCAL_GENERIC_CLASS() { }							\
				MS_LOCAL_GENERIC_CLASS(TCHAR* name, local_value_vf fn);	\
				classof_methods (MS_LOCAL_GENERIC_CLASS, Generic);		\
		void	collect() { delete this; }								\
		Value* apply(Value** arglist, int count, CallContext* cc=NULL); \
	};

#endif
