#include "stdafx.h"

#include "net_task.h"
#include "xrdeflector.h"
#include "xrlc_globaldata.h"

extern xr_vector<u32> net_pool;

net_task::net_task( IAgent *agent, DWORD session ):
_agent(*agent),
_session(session),
_id(u32(-1)),
_beak_count( _break_connection_times ),
_D(0)
{


}

net_task::~net_task( )
{
	VERIFY(!_D||!_D->_net_session);
}
bool net_task::	receive		( IGenericStream* inStream )
{
	INetBlockReader r( inStream );
	_id = r.r_u32();
	if( std::find( net_pool.begin(), net_pool.end(), _id ) != net_pool.end() )
		return false;
	net_pool.push_back( _id );
	_D = inlc_global_data()->g_deflectors()[_id];
	_D->_net_session = this;
	return true;
}
bool net_task::send	( IGenericStream* outStream )
{
	INetMemoryBuffWriter w( outStream, 1024*1024 );
	VERIFY(_id!= u32(-1));
	xr_vector<u32>::iterator it = std::find( net_pool.begin(), net_pool.end(), _id );
	net_pool.erase( it );
	_D->_net_session = 0;
	if(break_all())
		return false;
	inlc_global_data()->create_write_faces();
	w.w_u32( _id );
	_D->write( w );
	inlc_global_data()->destroy_write_faces();
	return true;
}

void net_task::run()
{
	VERIFY(_id!= u32(-1));
	VERIFY(_D);
	_execute.run( *_D );
}

bool net_task::test_connection( )
{
	VERIFY( _session != DWORD(-1));
#ifdef	NET_CMP
	_agent.TestConnection(_session);
#else
	if( !break_all() && _agent.TestConnection(_session)== S_FALSE )
		_beak_count--;
#endif
	return !break_all();
}

