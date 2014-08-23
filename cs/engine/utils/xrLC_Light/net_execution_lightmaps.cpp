#include "stdafx.h"

#include "net_execution_lightmaps.h"
#include "net_task.h"
#include "xrlc_globaldata.h"
#include "xrdeflector.h"

extern xr_vector<u32>	net_pool;

void decompress( LPCSTR f_in_out );
void  DataReadCreate( LPCSTR fn )
{
	 {
		 decompress( fn );
		 INetReaderFile r_global( fn );
		 create_global_data();
		
		 VERIFY( inlc_global_data() );
		 inlc_global_data()->read( r_global );
	 }
	 //unlink( fn );

	 FPU::m64r		();
	 Memory.mem_compact	();
}


bool  GetGlobalData( IAgent* agent,
				    DWORD sessionId )
 {
	 
	 if(!inlc_global_data())
	 {

		  VERIFY( agent );
		  net_pool.clear();
		  
/*
		  IGenericStream* globalDataStream=0;
		  HRESULT rz = agent->GetData(sessionId, dataDesc, &globalDataStream);
		 
		  if (rz!=S_OK) 
				  return false;
*/		  
		  string_path cache_dir;
		  HRESULT rz = agent->GetSessionCacheDirectory( sessionId, cache_dir );
		  if (rz!=S_OK) 
				  return false;
		  strconcat(sizeof(cache_dir),cache_dir,cache_dir,gl_data_net_file_name);

 /*
		 IWriter* file = FS.w_open( cache_dir );
		 R_ASSERT(file);
		 file->w( globalDataStream->GetCurPointer(), globalDataStream->GetLength() );

		 free(globalDataStream->GetCurPointer());
		 FS.w_close(file);
		
		 agent->FreeCachedData(sessionId, dataDesc);
		 ULONG r =globalDataStream->AddRef();
		 r = globalDataStream->Release();
		 if(r>0)
				globalDataStream->Release();
		 agent->FreeCachedData(sessionId, dataDesc);
		 Memory.mem_compact	();
*/

		 DataReadCreate( cache_dir );


			 return true;


		

	 }
	 return true;

 }

 bool  TaskReceive( net_task &task,IAgent* agent,
					DWORD sessionId, 
					IGenericStream* inStream )
 {
	
	 bool ret = false;
	 __try{
		ret = GetGlobalData( agent, sessionId ) && task.receive( inStream ) ;
	 }
	 __except( EXCEPTION_EXECUTE_HANDLER )
	 {
		 Msg( "accept!" );
		 return ret;
	 }
	FPU::m64r();
	return ret;
 }

namespace lc_net
{
	LPCSTR	execution_lightmaps::		data_files		()
	{
		return gl_data_net_file_name;
	}

	void	execution_lightmaps::		send_task		( IGridUser& user, IGenericStream* outStream, u32  id )	
		{
			{
				INetMemoryBuffWriter w( outStream, 100 );
				w.w_u32( id );
			}
		}
	void	execution_lightmaps	::	receive_result	( IGenericStream* outStream )	
		{
			INetBlockReader r(outStream);
			u32 id = r.r_u32();
			VERIFY(id==deflector_id);
			inlc_global_data()->g_deflectors()[ deflector_id ]->read( r );
		}
	bool	execution_lightmaps	::	receive_task	( IAgent* agent, DWORD sessionId, IGenericStream* inStream ) 
		{
			R_ASSERT(!task);
			task	= xr_new<net_task >( agent, sessionId );
			if( TaskReceive( *task, agent, sessionId, inStream ) )
				return true;
			xr_delete(task);
			return false;
		}
	void	execution_lightmaps	::	send_result		( IGenericStream* outStream )	
		{
			R_ASSERT( task );
			R_ASSERT(!task->break_all());
			task->send( outStream ) ;
		}
	bool	execution_lightmaps	::	execute			()	
		{
			R_ASSERT( task );
			task->run( );
			return !task->break_all();
		}
	

};

void send_net_lightmap_task( u32 deflector_id )
{
	
}