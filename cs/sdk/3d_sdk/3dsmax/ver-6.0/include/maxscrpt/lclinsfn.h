/*	
 *	lclinsfn.h -  generic function instantiation macros for MAXScript SDK plug-ins
 */

#include "ClassCfg.h"

#ifdef def_local_generic
#	undef def_local_generic
#	undef use_local_generic
#endif
#ifdef use_generic
#	undef use_generic
#endif


#pragma pointers_to_members(full_generality, virtual_inheritance)

#define def_local_generic(fn, name)										\
	MS_LOCAL_GENERIC_CLASS fn##_gf (_T(name), &MS_LOCAL_ROOT_CLASS::fn##_vf)
#define use_generic(fn, name)
#define use_local_generic(fn, name)

#undef def_name
#define def_name(name) Value* n_##name;	

#define define_local_generic_class 													\
	MS_LOCAL_GENERIC_CLASS::MS_LOCAL_GENERIC_CLASS(TCHAR*fn_name, local_value_vf fn)\
	{																				\
		tag = MS_LOCAL_GENERIC_CLASS_TAG;											\
		fn_ptr = fn;																\
		name = save_string(fn_name);												\
	}																				\
	Value* MS_LOCAL_GENERIC_CLASS::apply(Value** arg_list, int count, CallContext* cc) \
	{																				\
		Value*  result;																\
		Value**	evald_args;															\
		Value	**ap, **eap;														\
		int		i;																	\
		if (count < 1)																\
			throw ArgCountError("Generic apply", 1, count);							\
		value_local_array(evald_args, count);										\
		for (i = count, ap = arg_list, eap = evald_args; i--; eap++, ap++)			\
			*eap = (*ap)->eval();													\
		if (evald_args[0]->local_base_class() == MS_LOCAL_ROOT_CLASS_TAG)			\
			result = (((MS_LOCAL_ROOT_CLASS*)evald_args[0])->*fn_ptr)(&evald_args[1], count - 1); \
		else																		\
			throw NoMethodError (name, evald_args[0]);								\
		pop_value_local_array(evald_args);											\
		return result;																\
	}																				\
	MS_LOCAL_GENERIC_CLASS_CLASS MS_LOCAL_GENERIC_CLASS_class (str1(MS_LOCAL_GENERIC_CLASS));



