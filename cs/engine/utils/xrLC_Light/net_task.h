#pragma once

#include "light_execute.h"
#include "hxgrid/Interface/IAgent.h"
//class IWriter;
//class INetReader;
/*
class net_task
{
	INetReader		*in_stream  ;
	IWriter			*out_stream ;
public:
	net_task(  INetReader* inStream,  IWriter* outStream ): in_stream( inStream ), out_stream( outStream )
	{
		VERIFY(in_stream);
		VERIFY(out_stream);
	};
	virtual void run()	= 0;
};

*/


	interface IAgent;
	class net_task
	{
		IAgent&			_agent		;
		DWORD			_session	;
		u16				_beak_count	;
	static const u16	_break_connection_times = 1;
		CDeflector *_D		;
		u32		_id			;
		light_execute _execute;
	public:
		void run				( );
		bool test_connection	( );
	IC	bool break_all			( )	{ return _beak_count==0; } 
		bool receive			( IGenericStream* inStream) ;
		bool send				( IGenericStream* outStream );
		
		net_task				( IAgent *agent, DWORD session);
		~net_task				( );
	};



