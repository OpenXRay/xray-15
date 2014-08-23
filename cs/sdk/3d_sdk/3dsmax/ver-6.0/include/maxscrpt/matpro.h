/*	
 *		matrix_protocol.h - def_generics for matrix protocol
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */

	use_generic(coerce,			"coerce");

	use_generic( plus,			"+" );
	use_generic( minus,			"-" );
	use_generic( times,			"*" );

	use_generic( isIdentity,	"isIdentity" );
	use_generic( inverse,		"Inverse" );

	def_visible_primitive( rotateXMatrix,	"RotateXMatrix");   
	def_visible_primitive( rotateYMatrix,	"RotateYMatrix");
	def_visible_primitive( rotateZMatrix,	"RotateZMatrix");
	def_visible_primitive( transMatrix,		"TransMatrix");
	def_visible_primitive( scaleMatrix,		"ScaleMatrix");
	def_visible_primitive( rotateYPRMatrix, "RotateYPRMatrix");

	def_visible_generic( xFormMat,		"XFormMat" );
	def_mapped_generic ( identity,		"Identity" );
	def_mapped_generic ( zero,			"Zero" );
	def_mapped_generic ( orthogonalize,	"Orthogonalize" );

	def_mapped_generic ( translate,		"Translate" );
	def_mapped_generic ( rotateX,		"RotateX" );
	def_mapped_generic ( rotateY,		"RotateY" );
	def_mapped_generic ( rotateZ,		"RotateZ" );
	use_generic        ( scale,			"Scale" );
	def_mapped_generic ( preTranslate,	"PreTranslate" );
	def_mapped_generic ( preRotateX,	"PreRotateX" );
	def_mapped_generic ( preRotateY,	"PreRotateY" );
	def_mapped_generic ( preRotateZ,	"PreRotateZ" );
	def_mapped_generic ( preScale,		"PreScale" );

	use_generic        ( rotate,		"Rotate" );
	def_mapped_generic ( preRotate,		"PreRotate" );
