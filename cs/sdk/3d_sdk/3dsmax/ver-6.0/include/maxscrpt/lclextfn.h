/*	
 *	lclextfn.h - abstract generic function definitions for MAXScript SDK plug-ins
 */

#include "ClassCfg.h"

#ifdef def_local_generic
#	undef def_local_generic
#	undef use_local_generic
#endif
#ifdef use_generic
#	undef use_generic
#endif


#define def_local_generic(fn, name)	\
	extern MS_LOCAL_GENERIC_CLASS fn##_gf
#define use_local_generic(fn, name) \
	def_local_generic(fn, name)
#define use_generic(fn, name) \
	extern Generic fn##_gf

#undef def_name
#define def_name(name)	extern Value* n_##name;	
