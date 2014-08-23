/*		MSTime.h - the time family of classes for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_MSTIME
#define _H_MSTIME

#include "Max.h"

/* ------------------------ Time ------------------------------ */

visible_class (MSTime)

class MSTime : public Value
{
public:
	TimeValue	time;

	ENABLE_STACK_ALLOCATE(MSTime);

				MSTime (TimeValue t);
	static ScripterExport Value* intern(TimeValue t);

#	define		is_time(o) ((o)->tag == class_tag(MSTime))
				classof_methods (MSTime, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

#include "defimpfn.h"
#	include "timepro.h"
	def_generic  ( coerce,	"coerce");

	def_property ( ticks );
	def_property ( frame );
	def_property ( normalized );

	TimeValue	to_timevalue() { return time; }
	float	    to_float() { return (float)time / GetTicksPerFrame(); }
	int			to_int() { return (int)time / GetTicksPerFrame(); }
	void		to_fpvalue(FPValue& v) { v.i = time; v.type = TYPE_TIMEVALUE; }

	Value*	widen_to(Value* arg, Value** arg_list);
	BOOL	comparable(Value* arg);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Interval ------------------------------ */

applyable_class (MSInterval)

class MSInterval : public Value
{
public:
	Interval	interval;

	ENABLE_STACK_ALLOCATE(MSInterval);


				MSInterval () {};
 ScripterExport MSInterval (Interval i);
 ScripterExport MSInterval (TimeValue s, TimeValue e);

#	define		is_interval(o) ((o)->tag == class_tag(MSInterval))
				classof_methods (MSInterval, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

#include "defimpfn.h"
	def_property ( start );
	def_property ( end );

	Interval	to_interval() { return interval; }
	void		to_fpvalue(FPValue& v) { v.intvl = new Interval (interval.Start(), interval.End()); v.type = TYPE_INTERVAL; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

#endif
