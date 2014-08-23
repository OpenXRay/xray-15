/*	Collectable.h - Collectables include
 *
 *			Copyright (c) John Wainwright, 1996
 *
 */

#ifndef _H_COLLECTIBLE
#define _H_COLLECTIBLE

enum col_state {booting, pre_gc, in_mutator, in_mark, in_sweep, closing_down };

typedef struct free_mem free_mem;		// free mem linked list entry
struct free_mem
{
	free_mem*	next;
	free_mem*	prev;
	size_t		size;
};
		
// collection flag bits ...
enum gc_flags
{
	GC_IN_USE			= 0x0001,
	GC_GARBAGE			= 0x0002,
	GC_PERMANENT		= 0x0004,
	GC_IN_HEAP			= 0x0008,
	GC_NOT_NEW			= 0x0010,
	GC_STATIC			= 0x0020,
	GC_ON_STACK			= 0x0040,
	GC_MIGRATED_TO_HEAP	= 0x0080,
};

class Value;
class ValueMapper;

#define ALLOCATOR_STACK_SIZE 1024000     // initial size of allocation stack per thread

extern ScripterExport void push_alloc_frame();					// manage alloc stack...
extern ScripterExport void pop_alloc_frame();
extern ScripterExport void pop_alloc_frame(Value*& result);	// pops & moves result into callers frame if only on stack

#define ENABLE_STACK_ALLOCATE(_class)											\
	ScripterExport void* operator new (size_t sz) { return stack_alloc(sz); }	\
	ScripterExport void* operator new (size_t sz, char flag) { return Collectable::operator new (sz, flag); }

// free-list is kept in a number of separate size-related sub-lists, specifically
// for the high-bandwidth low size allocs.
// the heads of these are in the free_list static array in Collectable.
// each consecutive sub-list is for chunks one GC_ALLOC_MULTIPLE greater than the previous.
// the following defines determine the number of sub-lists.  

#define GC_NUM_SUBLISTS				128 
#define GC_LOW_SUBLIST				16   // <16, 16, 20, 24, 28, 32, ... 512, >512
#define GC_SUBLIST_INDEX_SHIFT		4    // log2(LOW_SUBLIST)

class Collectable
{
public:
	Collectable*	next;					// in heap: links (in whichever collector list this value is in); 
											// on stack: pointer to heap migrated value, NULL if not migrated
	Collectable*	prev;
	static CRITICAL_SECTION heap_update;	// for syncing allocation list updates
	byte			flags;					// collection flags
	byte			flags2;					// general purpose flags - only to be used by Collectable
	short			flags3;					// general purpose flags - can be used by Values

	static Collectable* collectable_list;	// head of the collectable list
	static Collectable* permanent_list;		// head of the permanent list
	static free_mem* free_list[GC_NUM_SUBLISTS];				// head of the free list
	static size_t heap_allocated;			// running count of MAXScript heap usage
	static size_t heap_size;				// alloc'd heap size
	// LAM: 2/23/01 - need to export following for ms_make_collectable (see below) to link in DLXs.
	ScripterExport static col_state state;	// current collector state 
	ScripterExport static bool fullCollectNextHoldFlush;	// if true, perform gc on next Hold system flush
	static bool gc_light;					// if true, no Hold system flush during current gc
	
	ScripterExport Collectable();
	ScripterExport ~Collectable();

	static ScripterExport void for_all_values(void (*map_fn)(Value* val), ValueMapper* mapper = NULL, ValueMetaClass* c = NULL);

	ScripterExport static void* heap_alloc(size_t sz);
	ScripterExport static void* stack_alloc(size_t sz);
	ScripterExport static void  heap_free(void* p);

	ScripterExport void* operator new (size_t sz, char flag);
	ScripterExport void* operator new (size_t sz) { return heap_alloc(sz); }
	ScripterExport void operator delete (void* val);
	ScripterExport void operator delete (void* val, char flag) { Collectable::operator delete(val); }

	static void	mark();
	static void	sweep();
	static void	setup(size_t);
	ScripterExport static void	gc();
	static void	coalesce_free_list();
	virtual void collect() = 0;				  // does the actual collecting, needs to be virtual to get right size to operator delete
	virtual void gc_trace() { mark_in_use(); } // the marking scanner, default is mark me in use
	static void close_down();
	static void drop_maxwrapper_refs();

	ScripterExport void	make_collectable();
				   void	make_permanent();   // no long exported, must use make_heap_permanent AND use its result as the made-permament value
				   void	make_static();		//    "    "          "       make_heap_static AND      "       "      "

	ScripterExport static void push_alloc_stack_frame();
	ScripterExport static void pop_alloc_stack_frame();

	int		is_marked()			{ return (flags & GC_IN_USE); }
	int		is_not_marked()		
{ 
	assert (!is_on_stack());   // debugging new stack-based collector
	return !is_marked(); 
}
	int		is_garbage()		{ return is_not_marked(); }
	int		is_permanent()		{ return (flags & GC_PERMANENT); }
	void	mark_in_use()		{ flags |= GC_IN_USE; }
	void	unmark_in_use()		{ flags &= ~GC_IN_USE; }
	int		has_heap_copy()		{ return (flags & (GC_IN_HEAP | GC_MIGRATED_TO_HEAP | GC_STATIC)); }
	int		is_in_heap()		{ return (flags & GC_IN_HEAP); }
	int		is_on_stack()		{ return (flags & GC_ON_STACK); }

};

// mapping object for Collectable::for_all_values()
class ValueMapper 
{
public:
	virtual void map(Value* val)=0;
};

ScripterExport void ms_free(void* p);
ScripterExport void* ms_malloc(size_t sz);
inline void ms_make_collectable(Collectable* v) { if (v != NULL && Collectable::state != closing_down) v->make_collectable(); }

#endif
