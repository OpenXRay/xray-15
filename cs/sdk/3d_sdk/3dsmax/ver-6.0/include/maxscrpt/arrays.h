/*		Arrays.h - the Array family of classes for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_ARRAYS
#define _H_ARRAYS

#include "Collect.h"

/* ------------------------ Array ------------------------------ */

visible_class (Array)

class Array : public Value, public Collection
{
public:
	int			size;					// array size
	int			data_size;				// allocated array buffer size (in Value*'s)
	Value**		data;					// the array elements (uninitialized are set to undefined)

	ScripterExport	static CRITICAL_SECTION array_update;	// for syncing array updates

	ScripterExport	 Array(int init_size);
	ScripterExport	~Array() { if (data) free(data); }

				classof_methods (Array, Value);

	static Value* make(Value** arg_list, int count);
	static void	setup();

	Value*& operator[](const int i) const { return data[i]; } // access ith array entry.
	
#	define		is_array(v) ((v)->tag == class_tag(Array))
	BOOL		_is_collection() { return 1; }
	BOOL		_is_selection() { return 1; }
	void		gc_trace();
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// operations
#include "defimpfn.h"
#	include "arraypro.h"
	use_generic( plus, "+" );
	use_generic( copy, "copy" );
	use_generic( coerce,	"coerce");

	ScripterExport Value* map(node_map& m);
	ScripterExport Value* map_path(PathName* path, node_map& m);
	ScripterExport Value* find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg);
	ScripterExport Value* get_path(PathName* path);

	// built-in property accessors 
	def_property ( count );

	ScripterExport Value* append(Value*);
	ScripterExport Value* join(Value*);
	ScripterExport Value* sort();
	ScripterExport Value* push(Value*);
	ScripterExport Value* drop();
	ScripterExport Value* get(int index);
	ScripterExport BOOL	  deep_eq(Value* other);

	// get selection iterator for an array
	SelectionIterator* selection_iterator();

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	void	 to_fpvalue(FPValue& v);
};

/* ------------------------ BitArray ------------------------------ */

visible_class (BitArrayValue)

class BitArrayValue : public Value
{
public:
	BitArray	bits;		// the bits

	ScripterExport BitArrayValue();
	ScripterExport BitArrayValue(BitArray& b);
        ScripterExport BitArrayValue(int count);

				classof_methods (BitArrayValue, Value);

	static Value* make(Value** arg_list, int count);

#	define		is_BitArrayValue(v) ((v)->tag == class_tag(BitArrayValue))
//	BOOL		_is_collection() { return 1; }
	BOOL		_is_selection() { return 1; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void can_hold(int index) { if (bits.GetSize() <= index) bits.SetSize(index); }

	// operations
#include "defimpfn.h"
#	include "arraypro.h"
	use_generic( plus, "+" );
	use_generic( minus, "-" );
	def_generic( uminus, "u-");
	use_generic( times, "*" );
	use_generic( copy, "copy" );
	use_generic( coerce,	"coerce");

	ScripterExport Value* map(node_map& m);

	// built-in property accessors
	def_property ( count );
	def_property ( numberSet );
	def_property ( isEmpty );

	SelectionIterator* selection_iterator();

	BitArray&	to_bitarray() { return bits; }
	void	    to_fpvalue(FPValue& v) { v.bits = &bits; v.type = TYPE_BITARRAY; }
#	define		is_bitarray(b) ((b)->tag == class_tag(BitArrayValue))

};

#endif
