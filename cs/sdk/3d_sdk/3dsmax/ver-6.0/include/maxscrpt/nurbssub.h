/*	
 *		NurbsSub.h - Nurbs sub-object classes & functions
 *
 *	mirrors the mesh sub-object selection classes for NURBS sub-objects.
 *
 *			Copyright © Autodesk, Inc., 1998
 *				John Wainwright
 */

#ifndef _H_NURBSSUB
#define _H_NURBSSUB

// Nurbs selection types
#define NSEL_ALL		1		// whole Nurbs selected 
#define NSEL_CUR		2		// current selection 
#define NSEL_EXP		3		// explicit selection (in vsel) 
#define NSEL_SINGLE		4		// explicit single index  

/* -------------- base class for Nurbs sub-object selections ------------------- */

visible_class (NURBSSelection)

class NURBSSelection : public Value
{
public:
	MAXNode*	owner;			// owner node if any
	Object*		obj;			// NURBS base obj if any
	NURBSSubObjectLevel level;	// subobject level of this selection
	BYTE		sel_type;		// selection type
	BitArray	vsel;			// stand-alone selection if any or copy of current owner level selection
	DWORD		index;			// single vert index 			

	ScripterExport NURBSSelection(MAXNode* own, NURBSSubObjectLevel lvl, BYTE stype, DWORD indx = 0);

				classof_methods (NURBSSelection, Value);
#	define		is_NURBSSelection(v) ((v)->tag == class_tag(NURBSSelection))
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	// utility functions
	BitArray*	get_sel();							// my element selection
	void		get_owner_sel(BitArray& osel);		// owner's element selection
	int			num_elements();
	BOOL		is_same_selection(Value* s) { return is_NURBSSelection(s) && ((NURBSSelection*)s)->level == level; }
	void		setup_xform(BitArray& os, BOOL& local_org, Matrix3& axis);

	DWORD		get_sel_index(BitArray* vs, int n);  // index for n'th item vertex in BitArray
	void		update_sel();
	void		sprin1(TCHAR* type, CharStream* s);

	// operations
#include "defimpfn.h"
#	include "arraypro.h"
	def_generic ( move,			"move");
	def_generic ( scale,		"scale");
	def_generic ( rotate,		"rotate");
	def_generic ( delete,		"delete");
	def_generic ( select,		"select");
	def_generic ( deselect,		"deselect");
	def_generic ( selectmore,	"selectMore");

	ScripterExport Value* map(node_map& m);

	// built-in property accessors
	def_property ( count );
	def_property ( index );
	def_property ( selSetNames );
	def_property ( pos );
};


#endif
