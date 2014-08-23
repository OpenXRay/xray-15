/*	
 *		kernel_protocol.h - def_generics for the MAXScript kernel protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */

	def_mapped_generic( print,			"print");

	def_generic        ( eq,			"==");
	def_generic        ( ne,			"!=");

	def_generic        ( coerce,		"coerce");

	def_visible_generic( classOf,		"classOf");
	def_visible_generic( superClassOf,	"superClassOf");
	def_visible_generic( isKindOf,		"isKindOf");
