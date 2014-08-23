/*	
 *		atmspro.h - def_generics for the operations on MAX atmosphere objects
 *
 *			Copyright © John Wainwright 1996
 *
 */
 
/* gizmo operations */

#ifndef NO_ATMOSPHERICS	// russom - 04/11/02
	def_visible_generic  ( getGizmo,	"getGizmo");
	def_visible_generic  ( deleteGizmo,	"deleteGizmo");
	def_visible_generic  ( appendGizmo,	"appendGizmo");
#endif // NO_ATMOSPHERICS