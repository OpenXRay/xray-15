/*	
 *		ObjectSets.h - ObjectSet classes for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_OBJECTSET
#define _H_OBJECTSET

#include "Collect.h"
visible_class (Set)

class Set : public Value, public Collection
{
public:
			classof_methods (Set, Value);
	BOOL	_is_collection() { return 1; }

#include "defimpfn.h"
	def_generic  (coerce,   "coerce"); 
};

#include "PathName.h"

visible_class_s (ObjectSet, Set)

class ObjectSet : public Set
{
protected:
				ObjectSet() { }
public:
	TCHAR*		set_name;
	BOOL		(*selector)(INode* node, int level, void* arg);	// set selector function
	void*		selector_arg;									// selector fn argument

				ObjectSet(TCHAR* name, SClass_ID class_id);
				ObjectSet(TCHAR* init_name, BOOL (*sel_fn)(INode*, int, void*), void* init_arg = NULL);
	void		init(TCHAR* name);

				classof_methods (ObjectSet, Set);
	static void	setup();
	TCHAR*		name() { return set_name; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_T("$%s"), set_name); }
	void		export_to_scripter();

	ScripterExport Value* map(node_map& m);
	ScripterExport Value* map_path(PathName* path, node_map& m);
	ScripterExport Value* find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg);
	ScripterExport Value* get_path(PathName* path);

#include "defimpfn.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
};

class CurSelObjectSet : public ObjectSet
{
public:
				CurSelObjectSet(TCHAR* name);

	void		collect() { delete this; }

	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_generic  (coerce,   "coerce"); 

	def_property ( count );
};

extern ObjectSet all_objects;
extern ObjectSet all_geometry;
extern ObjectSet all_lights;
extern ObjectSet all_cameras;
extern ObjectSet all_helpers;
extern ObjectSet all_shapes;
extern ObjectSet all_systems;
extern ObjectSet all_spacewarps;
extern CurSelObjectSet current_selection;

#endif
