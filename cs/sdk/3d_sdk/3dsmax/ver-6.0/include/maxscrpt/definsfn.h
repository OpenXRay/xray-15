/*	
 *		def_instantiate_functions.h - macros for making instantiating declarations
 *									for MAXScript functions
 *
 *	This will typically be used along with protocol definition files in 
 *  distinguished Value subclasses that will host the static instantiation
 *  of MAXScript function objects.
 *
 *  see def_abstract_functions.h for more info.
 *
 *	
 *	Copyright © John Wainwright 1996
 *
 */
/* def_generic macro for instantiation declarations */

#include "Funcs.h"

#ifdef def_generic
#	undef def_generic
#	undef def_node_generic
#	undef def_mapped_generic
#	undef def_visible_generic
#	undef def_struct_generic
#	undef use_generic
#	undef def_primitive
#	undef def_mapped_primitive
#	undef def_lazy_primitive
#	undef def_visible_lazy_primitive
#	undef def_visible_primitive
#	undef def_struct_primitive
#	undef def_property
#	undef def_property_alias
#	undef def_2_prop_path
#	undef def_2_prop_path_alias
#	undef def_nested_prop
#	undef def_nested_prop_alias
#endif
#ifdef def_prop_getter
#	undef def_prop_getter
#	undef def_prop_setter
#endif

#pragma pointers_to_members(full_generality, virtual_inheritance)

#define def_generic(fn, name)											\
	Generic fn##_gf (&_T("%"#name), &Value::fn##_vf)
#define def_visible_generic(fn, name)									\
	Generic fn##_gf (_T(name), &Value::fn##_vf)
#define def_struct_generic(fn, _struct, name)							\
	Generic fn##_gf (_T(name), &Value::fn##_vf, _T(#_struct))
#define def_node_generic(fn, name)										\
	NodeGeneric fn##_gf (_T(name), &Value::fn##_vf)
#define def_mapped_generic(fn, name)									\
	MappedGeneric fn##_gf (_T(name), &Value::fn##_vf)
#define use_generic(fn, name)

#define def_primitive(fn, name)											\
	Value* fn##_cf(Value**,int);										\
	Primitive fn##_pf (&_T("%"#name), fn##_cf)  /* scripter-hidden name */
#define def_visible_primitive(fn, name)									\
	Value* fn##_cf(Value**,int);										\
	Primitive fn##_pf (_T(name), fn##_cf)
#define def_struct_primitive(fn, _struct, name)							\
	Value* fn##_cf(Value**,int);										\
	Primitive fn##_pf (_T(name), _T(#_struct), fn##_cf)
#define def_mapped_primitive(fn, name)									\
	Value* fn##_cf(Value**,int);										\
	MappedPrimitive fn##_pf (_T(name), fn##_cf)
#define def_lazy_primitive(fn, name)									\
	Value* fn##_cf(Value**,int);										\
	Primitive fn##_pf (&_T("%"#name), fn##_cf, LAZY_PRIMITIVE)
#define def_visible_lazy_primitive(fn, name)							\
	Value* fn##_cf(Value**,int);										\
	Primitive fn##_pf (_T(name), fn##_cf, LAZY_PRIMITIVE)

#define def_property(p)													\
	_T(#p), &Value::get_##p, &Value::set_##p,
#define def_property_alias(p, real_p)									\
	_T(#p), &Value::get_##real_p, &Value::set_##real_p,
#define def_2_prop_path(p1, p2)											\
	_T(#p1"."#p2), &Value::get_##p1##_##p2, &Value::set_##p1##_##p2,
#define def_2_prop_path_alias(p1, p2, real_p1, real_p2)					\
	_T(#p1"."#p2), &Value::get_##real_p1##_##real_p2, &Value::set_##real_p1##_##real_p2,
#define def_nested_prop(p1)												\
	_T("*."#p1), &Value::get_nested_##p1, &Value::set_nested_##p1,
#define def_nested_prop_alias(p1, real_p1)								\
	_T("*."#p1), &Value::get_nested_##real_p1, &Value::set_nested_##real_p1,

#undef def_name
#define def_name(name)	ScripterExport Value* n_##name;	
	
#undef def_marker
#define def_marker(var, str) ScripterExport Name var (_T(str));

