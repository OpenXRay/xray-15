/*	
 *		PathName.h - PathName class for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_PATHNAME
#define _H_PATHNAME

#include "Collect.h"
#include "Thunks.h"
#include "ObjSets.h"

enum path_flags { rooted_path = 1, wild_card_path = 2 };

visible_class (PathName)

class PathName : public Set
{
public:
	int			flags;
	short		n_levels;
	TCHAR**		path_levels;
	Thunk*		root_set_thunk;
	Value*		root_set;

				PathName();
				~PathName();

				classof_methods (PathName, Set);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);

	ScripterExport Value* eval();
	void		append(TCHAR* level_name);
	Value*		find_object(TCHAR* name);
	Value*		get_object();

	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
};

extern TCHAR* ellipsis_level_name;
extern TCHAR* parent_level_name;

#endif
