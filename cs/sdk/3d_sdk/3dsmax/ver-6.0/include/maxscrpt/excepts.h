/*		Exception.h - exception class for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_EXCEPTION
#define _H_EXCEPTION

class Value;
class Thunk;
class ValueMetaClass;

extern TCHAR* null_string;

class ScripterExport MAXScriptException
{
public:
	virtual void sprin1(CharStream* s);
};

class ScripterExport UnknownSystemException : public MAXScriptException
{
public:
			UnknownSystemException() {}
	void	sprin1(CharStream* s);
};

class ScripterExport SignalException : public MAXScriptException
{
public:
	void	sprin1(CharStream* s);
};

class ScripterExport CompileError : public MAXScriptException
{
public:
	TCHAR*	description;
	TCHAR*	info;
	TCHAR*  line;
	TCHAR*	file;
			CompileError (TCHAR* d, TCHAR* i, TCHAR* l, TCHAR* f = null_string);
			CompileError () { description = NULL; info = null_string; line = null_string; file = null_string; }
		   ~CompileError ();

	void	sprin1(CharStream* s);
	void	set_file(TCHAR* f);
};

class ScripterExport SyntaxError : public CompileError
{
	TCHAR*	wanted;
	TCHAR*	got;
	TCHAR*   line;
public:
			SyntaxError (TCHAR* w, TCHAR* g, TCHAR* l = null_string, TCHAR* f = null_string);
		   ~SyntaxError ();

	void	sprin1(CharStream* s);
};

class ScripterExport TypeError : public MAXScriptException
{
	Value*	target;
	ValueMetaClass* wanted_class;
	TCHAR*	description;
public:
			TypeError (TCHAR* d, Value* t, ValueMetaClass* c = NULL);
		   ~TypeError ();

	void	sprin1(CharStream* s);
};

class ScripterExport NoMethodError : public MAXScriptException
{
	Value*	target;
	TCHAR*	fn_name;
public:
			NoMethodError (TCHAR* fn, Value* t);
		   ~NoMethodError ();

	void	sprin1(CharStream* s);
};

#define unimplemented(m, t) throw NoMethodError (m, t)

class ScripterExport AccessorError : public MAXScriptException
{
	Value*	target;
	Value*	prop;
public:
			AccessorError (Value* t, Value* p) { target = t; prop = p; }

	void	sprin1(CharStream* s);
};

class ScripterExport AssignToConstError : public MAXScriptException
{
	Thunk*	thunk;
public:
			AssignToConstError (Thunk* t) { thunk = t; }

	void	sprin1(CharStream* s);
};

class ScripterExport ArgCountError : public MAXScriptException
{
	int		wanted;
	int		got;
	TCHAR*	fn_name;
public:
			ArgCountError (TCHAR* fn, int w, int g);
		   ~ArgCountError ();

	void	sprin1(CharStream* s);
};

class ScripterExport RuntimeError : public MAXScriptException
{
public:
			TCHAR*	desc1;
			TCHAR*   desc2;
			Value*  info;
			RuntimeError (TCHAR* d1);
			RuntimeError (TCHAR* d1, TCHAR* d2);
			RuntimeError (TCHAR* d1, Value* ii);
			RuntimeError (TCHAR* d1, TCHAR* d2, Value* ii);
			RuntimeError (Value* ii);
		   ~RuntimeError ();

	void	init(TCHAR* d1, TCHAR* d2, Value* ii);
	void	sprin1(CharStream* s);
};

class ScripterExport IncompatibleTypes : public MAXScriptException
{
	Value*	val1;
	Value*  val2;
public:
			IncompatibleTypes (Value* v1, Value* v2) { val1 = v1; val2 = v2; }

	void	sprin1(CharStream* s);
};

class ScripterExport ConversionError : public MAXScriptException
{
	Value*	val;
	TCHAR*   type;
public:
			ConversionError (Value* v, TCHAR* t);
		   ~ConversionError ();

	void	sprin1(CharStream* s);
};

class FunctionReturn : public MAXScriptException
{
public:
	Value*	return_result;
			FunctionReturn (Value* v) { return_result = v; }

	void	sprin1(CharStream* s);
};

class LoopExit : public MAXScriptException
{
public:
	Value*	loop_result;
			LoopExit (Value* v) { loop_result = v; }

	void	sprin1(CharStream* s);
};

class LoopContinue : public MAXScriptException
{
public:
			LoopContinue () { }

	void	sprin1(CharStream* s);
};

#endif
