/*		Structs.h - the MAXSript struct definition classes
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#ifndef _H_STRUCT
#define _H_STRUCT

visible_class (StructDef)

class StructDef : public Value
{
public:
	Value*		name;						/* struct's global var name */
	Value**		member_inits;				/* member init vals			*/
	int			member_count;				/*   "    count				*/
	HashTable*	members;					/* member name to index table */

	ScripterExport StructDef(Value* name, int member_count, Value** inits, HashTable* members);
				~StructDef();
				classof_methods (StructDef, Value);
#	define		is_structdef(o) ((o)->tag == class_tag(StructDef))

	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

#include "defimpfn.h"
	def_generic ( get_props,	"getPropNames"); // LAM: added 4/27/00

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
	ScripterExport Value* get_property(Value** arg_list, int count);
	ScripterExport Value* set_property(Value** arg_list, int count);
};

class Struct : public Value
{
public:
	StructDef*	definition;					/* pointer to my struct def							*/
	Value**		member_data;				/* data elements, indexed via struct def hashtable	*/

	ScripterExport Struct(StructDef* idef, int mem_count);
				~Struct();

#	define		is_struct(o) ((o)->tag == INTERNAL_STRUCT_TAG)
	Value*		classOf_vf(Value** arg_list, int count);
	Value*		superClassOf_vf(Value** arg_list, int count);
	Value*		isKindOf_vf(Value** arg_list, int count);
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(StructDef)) ? 1 : Value::is_kind_of(c); }

	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

#include "defimpfn.h"
	use_generic ( copy, "copy" );
	def_generic ( get_props,	"getPropNames"); // LAM: added 4/27/00

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

// StructMethods wrap member functions accessed on a struct instance
// their apply() sets up the appropriate struct instance thread-local
// for member data access thunks
class StructMethod : public Value
{
public:
	Struct* this_struct;
	Value*	fn;

			StructMethod(Struct* t, Value* f);

	void	gc_trace();
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { fn->sprin1(s); }
	BOOL	_is_function() { return fn->_is_function(); }

#	define is_structMethod(o) ((o)->tag == INTERNAL_STRUCT_METHOD_TAG)  // LAM - defect 307069
	Value* classOf_vf(Value** arg_list, int count) { return fn->classOf_vf(arg_list, count); }
	Value* superClassOf_vf(Value** arg_list, int count) { return fn->superClassOf_vf(arg_list, count); }
	Value* isKindOf_vf(Value** arg_list, int count) { return fn->isKindOf_vf(arg_list, count); }
	BOOL   is_kind_of(ValueMetaClass* c) { return fn->is_kind_of(c); }
	Value* get_property(Value** arg_list, int count) { return fn->get_property(arg_list, count); }
	Value* eval() { return fn->eval(); }
	Value* apply(Value** arglist, int count, CallContext* cc=NULL);
	Value* apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL); // LAM - 11/16/02
};


#endif