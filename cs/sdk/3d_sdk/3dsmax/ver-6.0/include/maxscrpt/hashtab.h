/*	
 *		HashTable.h - HashTable class for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_HASHTABLE
#define _H_HASHTABLE

typedef struct 
{
	void*	key;
	void*	value;
} binding;

typedef struct						/* secondary extent struct			*/
{
	size_t		size;				/* size of secondary extent			*/
	binding*	bindings;			/* table of bindings				*/
} secondary;
	
#define KEY_IS_OBJECT	0x0001		/* init flags that indicate whether keys & values are full MXS collectable objects */
#define VALUE_IS_OBJECT	0x0002

int	default_eq_fn(void* key1, void* key2);  /* default comparator & hash fns */
INT_PTR default_hash_fn(void* key);

class HashTabMapper;

visible_class (HashTable)

class HashTable : public Value
{
	secondary	**table;			/* primary extent: tbl of second's	*/
	size_t		size;				/* table size						*/
	int			n_entries;			/* no. entries in primary extent	*/
	int			(*eq_fn)(void*, void*); /* key equivalence function		*/
	INT_PTR			(*hash_fn)(void*);	/* key hgashing function			*/
				// Win64 Cleanup: Shuler
	int			cursor;				/* cursors used for sequencing...	*/
	int			secondCursor;			
	short		flags;
	HashTable*	inner;				/* links to next & prev tables when */
	HashTable*	outer;				/* used as a lexical scope table	*/
	int			level;				// scope nesting level

	static CRITICAL_SECTION hash_update;	// for syncing allocation hashtable updates

public:
	ScripterExport HashTable(size_t primary_size, int (*key_eq_fn)(void*, void*), INT_PTR (*key_hash_fn)(void*), int flags);
				// Win64 Cleanup: Shuler
				HashTable() { init(17, default_eq_fn, default_hash_fn, KEY_IS_OBJECT + VALUE_IS_OBJECT); }
				HashTable(size_t primary_size) { init(primary_size, default_eq_fn, default_hash_fn, KEY_IS_OBJECT + VALUE_IS_OBJECT); }
				~HashTable();
	void		init(size_t primary_size, int (*key_eq_fn)(void*, void*), INT_PTR (*key_hash_fn)(void*), int flags);
				// Win64 Cleanup: Shuler

	static void	setup();

				classof_methods (HashTable, Value);
	void		collect() { delete this;}
	void		gc_trace();

	ScripterExport Value*	get(void* key);
	ScripterExport Value*	put(void* key, void* val);
	ScripterExport Value*	put_new(void* key, void* val);
	ScripterExport Value*	find_key(void *val);
	ScripterExport Value*	set(void* key, void* val);
	ScripterExport void		remove(void* key);
	ScripterExport void		map_keys_and_vals(void (*fn)(void* key, void* val, void* arg), void* arg);
	ScripterExport void		map_keys_and_vals(HashTabMapper* mapper);
	ScripterExport int		num_entries() { return n_entries; }


	HashTable*	enter_scope();
	HashTable*	leave_scope();
	HashTable*	next_scope();
	int			scope_level() { return level; }
};

class HashTabMapper 
{
public:
	virtual void map(void* key, void* val)=0;
};

#define SECONDARY_BUCKET	5

#endif
