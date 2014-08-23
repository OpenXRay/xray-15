/*	
 *		math_protocol.h - def_generics for the generic function in the Math protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */

	def_generic(plus,	"+");
	def_generic(minus,	"-");
	def_generic(times,	"*");
	def_generic(div,	"/");
	def_generic(pwr,	"^");
	def_generic(uminus,	"u-");

	use_generic(eq,		"=");
	use_generic(ne,		"!=");
	def_generic(gt,		">");
	def_generic(lt,		"<");
	def_generic(ge,		">=");
	def_generic(le,		"<=");

	def_visible_generic ( random,	"random");
	def_visible_generic ( abs,		"abs");

	/* scripter-visible math primitives - implemented as prims since they are type-specific */
	
	def_visible_primitive( include, "include");
	
	def_visible_primitive( acos,	"acos");
	def_visible_primitive( asin,	"asin");
	def_visible_primitive( atan,	"atan");
	def_visible_primitive( ceil,	"ceil");
	def_visible_primitive( cos,		"cos");
	def_visible_primitive( cosh,	"cosh");
	def_visible_generic  ( exp,		"exp");  // exp is polymorphic (floats & quats)
	def_visible_primitive( floor,	"floor");
	def_visible_primitive( log,		"log");
	def_visible_primitive( log10,	"log10");
	def_visible_primitive( sin,		"sin");
	def_visible_primitive( sinh,	"sinh");
	def_visible_primitive( sqrt,	"sqrt");
	def_visible_primitive( tan,		"tan");
	def_visible_primitive( tanh,	"tanh");
 
	def_visible_primitive( atan2,	"atan2");
	def_visible_primitive( fmod,	"mod");
	def_visible_primitive( pow,		"pow");
	
	def_visible_primitive( seed,	"seed");
