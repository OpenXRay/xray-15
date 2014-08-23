/*	
 *		string_protocol.h - def_generics for the string protocol
 *
 *			Copyright © John Wainwright 1996
 *
 */

	use_generic( plus,	"+");
	use_generic( eq,	"=");
	use_generic( ne,	"!=");
	use_generic( gt,	">");
	use_generic( lt,	"<");
	use_generic( ge,	">=");
	use_generic( le,	"<=");
	use_generic( get,	"get");
	use_generic( put,	"put");

	def_visible_generic( findString,	"findString");
	def_visible_generic( findPattern,	"findPattern");
	def_visible_generic( substring,		"substring");
	def_visible_generic( replace,		"replace");

	use_generic( append,		"append"); // LAM - 5/28/02
	use_generic( execute, "execute");
	use_generic( coerce,  "coerce");
	use_generic( copy,    "copy");


