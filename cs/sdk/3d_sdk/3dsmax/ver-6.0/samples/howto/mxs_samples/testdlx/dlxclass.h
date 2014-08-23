/*	dlxClass.h - Sample MAXScript SDK code for adding a new Value class
 *
 *  John Wainwright, 1998
 */
 
#include "Funcs.h"

declare_local_generic_class

local_applyable_class (ListValue)

class ListValue : public Value
{
public:
	Value*		head;	// head item
	ListValue*	tail;	// rest of list

				ListValue();
				ListValue(Value* head, ListValue* tail);

	ValueMetaClass* local_base_class() { return class_tag(ListValue); } // local base class in this class's plug-in
				classof_methods(ListValue, Value);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void		gc_trace();
#	define		is_list(p) ((p)->tag == class_tag(ListValue))
	BOOL		_is_collection() { return 1; }

	// collection mapping
	Value*		map(node_map& m);

	// internal methods
	virtual void append(Value* v);

	// operations 
#include "lclabsfn.h"
#	include "setpro.h"

#include "lclimpfn.h"
#	include "listpro.h"

	//properties
	def_property ( count );

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};


local_applyable_class (SetValue)

class SetValue : public ListValue
{
public:
				SetValue();

				classof_methods(SetValue, ListValue);
	void		collect() { delete this; }
#	define		is_set(p) ((p)->tag == class_tag(SetValue))

	// internal methods
	void		append(Value* v);
	BOOL		is_member(Value* v);

	// operations 
#include "defimpfn.h"
#	include "setpro.h"
};


