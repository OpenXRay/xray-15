/*	
 *		quat_protocol.h - def_generics for quaternion protocol
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */

	use_generic(coerce,	"coerce");

	use_generic( plus,			"+" );
	use_generic( minus,			"-" );
	use_generic( times,			"*" );
	use_generic( div,			"/" );
	use_generic( uminus,		"u-" );

	use_generic( eq,			"=" );
	use_generic( ne,			"!=" );

	use_generic( random,		"random" );

	def_visible_generic( isIdentity,	"isIdentity" );
	use_generic        ( normalize,		"normalize" );
	def_visible_generic( inverse,		"Inverse" );
	def_visible_generic( conjugate,		"Conjugate" );
	def_visible_generic( logN,			"LogN" );
	use_generic        ( exp,			"Exp" );
	def_visible_generic( slerp,			"Slerp" );
	def_visible_generic( lnDif,			"LnDif" );
	def_visible_generic( qCompA,		"QCompA" );
	def_visible_generic( squad,			"Squad" );
	def_visible_generic( qorthog,		"qorthog" );
	def_visible_generic( transform,		"transform" );

	def_visible_primitive( squadrev, "squadrev" );
