/*	
 *		Strings.h - string family for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_STRING
#define _H_STRING

#include "streams.h"

visible_class (String)

// Collectable::flags3 - bit 0 set if string is a literal
class String : public Value
{
	TCHAR*		string;

public:
 ScripterExport String(TCHAR *init_string);
			   ~String();

				classof_methods (String, Value);
#	define		is_string(o) ((o)->tag == class_tag(String))
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);

	Value*		append(TCHAR* str);
	Value*		append(Value* str_val) { return append(str_val->to_string()); }

#include "defimpfn.h"
#	include "strngpro.h"

	def_property( count );

	TCHAR*		to_string();
	TSTR		to_filename();
	void		to_fpvalue(FPValue& v) { v.s = to_string(); v.type = TYPE_STRING; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	friend class StringStream;
};

applyable_class (StringStream)

class StringStream : public CharStream
{
public:
	TCHAR*	content_string;		/* the content string				*/
	TCHAR*	cursor;				/* current read/write cursor		*/
	size_t	buf_len;			/* allocated buffer length			*/
 	int		ungetchar_count;
 	Parser* reader;

	ScripterExport StringStream();
	ScripterExport ~StringStream();
	ScripterExport StringStream(TCHAR* init_string);
	ScripterExport StringStream(int ilen);
	ScripterExport StringStream(Value* init_string_value);
	ScripterExport void init(TCHAR* init_string);

			classof_methods (StringStream, CharStream);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

 #include "defimpfn.h"
 #	include "streampr.h"
 	use_generic( coerce,  "coerce");
	use_generic( copy,    "copy");

	ScripterExport TCHAR	get_char();
	ScripterExport void		unget_char(TCHAR c);
	ScripterExport TCHAR	peek_char();
 	ScripterExport int		pos();
				   void		seek(int pos);
	ScripterExport int		at_eos();
	ScripterExport void		rewind();
	ScripterExport void		set_size(int size);
	ScripterExport void		flush_to_eol();
	ScripterExport void		flush_to_eobuf();
			  	   void		undo_lookahead();

	ScripterExport	TCHAR*	puts(TCHAR* str);
	ScripterExport	TCHAR	putch(TCHAR c);
	ScripterExport	int		printf(const TCHAR *format, ...);

	TCHAR*	to_string();
	void	to_fpvalue(FPValue& v) { v.s = to_string(); v.type = TYPE_STRING; }
};

ScripterExport TSTR expand_file_name(TCHAR* file_name);
ScripterExport TSTR expand_file_dir(TCHAR* dir);

#endif
