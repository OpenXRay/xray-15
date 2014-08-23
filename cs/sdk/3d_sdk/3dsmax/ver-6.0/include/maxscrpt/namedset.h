/*	
 *		NamedSet.h - scripter access to named node selection sets 
 *
 *			John Wainwright
 *			Copyright © Autodesk, Inc. 1997
 *
 */

#ifndef _H_NAMEDSET
#define _H_NAMEDSET

/* ---------------------- MAXNamedSetArray ----------------------- */

// provides array-like access to the table of named selection sets

visible_class (MAXNamedSetArray)

class MAXNamedSetArray : public Value, public Collection
{
public:
				MAXNamedSetArray();

				classof_methods (MAXNamedSetArray, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors
	def_property ( count );

};

/* ---------------------- MAXNamedSet ----------------------- */

visible_class (MAXNamedSet)

class MAXNamedSet : public Value, public Collection
{
public:
	TSTR		name;

				MAXNamedSet(TCHAR* iname);

				classof_methods (MAXNamedSet, Value);
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
};

extern MAXNamedSetArray theNamedSetArray;

#endif

