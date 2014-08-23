#include "stdafx.h"
#include "lcnet_task_manager.h"
#include "net_execution.h"
#include "xrlc_globaldata.h"
#include "file_compress.h"
#include "serialize.h"
#include "net_execution_factory.h"

namespace	lc_net{

	task_manager g_task_manager;

	task_manager & get_task_manager()
	{
		return g_task_manager;
	}

	// void				  create_net_task_manager()
	//{
	//	g_task_manager = xr_new<task_manager>();
	//}
	// void				  destroy_net_task_manager()
	//{
	//	xr_delete( g_task_manager );
	//}

	XRLC_LIGHT_API net_task_interface *g_net_task_interface = &g_task_manager;

	void __cdecl Finalize(IGenericStream* outStream)
	{
	
		get_task_manager().receive_result( outStream );
	 }

	task_manager:: task_manager( )
	{
		tasks_completed	= 0;	
		//create_global_data_write("");
	}

	void	task_manager::		send_task( IGridUser& user, u32 id  )
	{
		clMsg( "send task : %d", id );
		//
		VERIFY( id>=0 );
		VERIFY( id < pool.size() );
		net_execution *e = pool[id];
		VERIFY( e != 0 );
		IGenericStream* stream  = CreateGenericStream();
		write_task_caption( stream, id, e->type() );
		e->send_task( user, stream, id );
		
		DWORD t_id = id;
		string_path data;
		strconcat( sizeof(data),data,libraries,",",e->data_files());
		user.RunTask( data ,"RunTask",stream,Finalize,&t_id,true);
	}

	xr_vector<net_execution*>::iterator task_manager::find( u32 id )
	{
		struct find_execution
		{
			u32 _id	;
			find_execution( u32 id ): _id( id ){}
			bool operator () ( const net_execution* e )
			{
				return e->id() == _id;
			}
		} f(id);
		xr_vector<net_execution*>::iterator e = pool.end();
		xr_vector<net_execution*>::iterator i = std::find_if( pool.begin(), e, f );
		return i;
	}

	net_execution* 	task_manager::receive_task	( IAgent* agent,DWORD sessionId, IGenericStream* inStream  )
	{
		__try{
		u32 id =u32(-1),  type = u32(-1);
		read_task_caption( inStream, id, type );
		pool_lock.Enter();
		xr_vector<net_execution*>::iterator i = find( id );
		
		if( i!= pool.end() )
		{
			pool_lock.Leave();
			return 0;
		}
		
		net_execution* e = execution_factory.create( type, id );
		e->receive_task( agent, sessionId, inStream );
		pool.push_back( e );
		pool_lock.Leave();
		return e;

		}
		 __except( EXCEPTION_EXECUTE_HANDLER )
		 {
			 Msg( "accept!" );
			 return 0;
		 }
		
	}


	void	task_manager::		receive_result( IGenericStream* inStream )
	{

		u32 id =	 u32(-1), type = u32(-1);//r.r_u32();
		read_task_caption( inStream, id, type );
		//xr_vector<u32>::iterator it =std::find( pool.begin(), pool.end(), id );
		const u32 size = pool.size();
		VERIFY( size>0 );
		VERIFY( id>=0 );
		VERIFY( id < size );

		pool_lock.Enter();
		net_execution *e = pool[id];
		R_ASSERT( e->type() == type );
		if( e == 0 )
		{
			pool_lock.Leave();
			return;
		}
		pool[id] = 0;
		++tasks_completed;
		pool_lock.Leave();

		e->receive_result( inStream );
		xr_delete( e );
		clMsg( "received task : %d", id );
		
		Progress( float( tasks_completed )/float( size ) );
		clMsg( "num task complited : %d , num task left %d  (task num %d)", tasks_completed, size - tasks_completed, size );
		if( tasks_completed == size )
		{
			clMsg	( "calculation complited" );
			clMsg	("%f net lightmaps calculation seconds",start_time.GetElapsed_sec());
		}
	
	}
	void	task_manager::send_result		( IGenericStream* outStream, net_execution &e  )
	{
		pool_lock.Enter();
		xr_vector<net_execution*>::iterator i = find( e.id() );
		R_ASSERT( i != pool.end() );
		net_execution *pe = *i;
		R_ASSERT( pe == &e );
		pool.erase( i );
		pool_lock.Leave();

		write_task_caption( outStream, e.id(), e.type() );
		e.send_result( outStream );
		xr_delete( pe );
	}


	void	task_manager::run()
	{
		start_time.Start();
		tasks_completed  = 0;
		
		inlc_global_data()->create_read_faces();
		IGridUser* user = CreateGridUserObject(IGridUser::VERSION);
		VERIFY( user );
		FPU::m64r		();
		Memory.mem_compact	();
		u32 size = pool.size();
		for	(u32 dit = 0; dit<size; dit++)
			send_task( *user, dit );
		user->WaitForCompletion();
		inlc_global_data()->destroy_read_faces();
		user->Release();
		pool.clear();
		
	}

	void	task_manager::add_task( net_execution* task )
	{
		pool.push_back( task );
	}

	void	task_manager::create_global_data_write( LPCSTR save_path )
	{
		FPU::m64r			();
		Memory.mem_compact	();
		std::random_shuffle	(inlc_global_data()->g_deflectors().begin(),inlc_global_data()->g_deflectors().end());
		clMsg( "create_global_data_write:  start" );
		string_path			 global_data_file_name ;
		FS.update_path		( global_data_file_name, "$app_root$", gl_data_net_file_name  );
		IWriter * file = FS.w_open(global_data_file_name);
		inlc_global_data()->write( *file );
		FS.w_close(file);
		compress( global_data_file_name ); 
		clMsg( "create_global_data_write:  end" );
	}

};