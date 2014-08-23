/*	
 *	defabs21.h - macros for making abstract declarations for MAXScript functions
 *
 *     abstract def_ macros for MAX R2.1
 *
 *	John Wainwright
 *	Copyright © Autodesk, Inc. 1998
 *
 */

/* def_generic macro for abstract declaration in Value class */

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

#define def_generic(fn, name)
#define def_visible_generic(fn, name) def_generic(fn, name)
#define def_struct_generic(fn, name) def_generic(fn, name)
#define def_node_generic(fn, name) def_generic(fn, name)
#define def_mapped_generic(fn, name) def_generic(fn, name)
#define use_generic(fn, name)

#define def_primitive(fn, name)		// no member function declarations for primitives
#define def_visible_primitive(fn, name)
#define def_mapped_primitive(fn, name)
#define def_lazy_primitive(fn, name)
#define def_visible_lazy_primitive(fn, name)
#define def_struct_primitive(fn, _struct, name)
#define def_property(p) 
#define def_property_alias(p, real_p)
#define def_2_prop_path(p1, p2) 
#define def_2_prop_path_alias(p1, p2, real_p1, real_p2)
#define def_nested_prop(p1) 
#define def_nested_prop_alias(p1, real_p1)									

