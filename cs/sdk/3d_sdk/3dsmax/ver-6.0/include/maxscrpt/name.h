/*	
 *		Name.h - Name class for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_NAME
#define _H_NAME

#include "HashTab.h"

visible_class (Name)

class Name : public Value
{
public:
	TCHAR*		string;
	static		HashTable* intern_table;
				Name(TCHAR *init_string);
			   ~Name() { if (string) free(string); }

#	define		is_name(o) ((o)->tag == class_tag(Name))
	static void	setup();
	static ScripterExport Value* intern(TCHAR* str);
	static ScripterExport Value* find_intern(TCHAR* str);
				classof_methods (Name, Value);
	
	ScripterExport void sprin1(CharStream* s);
	void		collect() { delete this; }
	TCHAR*		to_string();
	TSTR		to_filename();
	void		to_fpvalue(FPValue& v) { v.s = to_string(); v.type = TYPE_NAME; }

#include "defimpfn.h"
	use_generic( coerce,	"coerce");
	use_generic( gt,		">");
	use_generic( lt,		"<");
	use_generic( ge,		">=");
	use_generic( le,		"<=");

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

};

/* core interned names */

#include "defextfn.h"
#	include "corename.h"


#endif
