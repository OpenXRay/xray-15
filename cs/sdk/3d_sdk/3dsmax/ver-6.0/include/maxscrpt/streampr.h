/*	
 *		stream_protocol.h - def_generics for Stream protocol
 *
 *			Copyright © John Wainwright 1996
 */

	def_visible_generic(read_value,	"readValue");
	def_visible_generic(read_expr,	"readExpr");
	def_visible_generic(read_line,	"readLine");
	def_visible_generic(read_char,	"readChar");
	def_visible_generic(read_chars,	"readChars");
	def_visible_generic(read_delimited_string,	"readDelimitedString");
	def_visible_generic(skip_to_string,	"skipToString");
	def_visible_generic(skip_to_next_line,	"skipToNextLine");
	def_visible_generic(execute,	"execute");
	
	def_visible_generic(file_pos,	"filepos");
	def_visible_generic(seek,		"seek");
	def_visible_generic(eof,		"eof");

	def_visible_generic(close,		"close");
	def_visible_generic(flush,		"flush");
