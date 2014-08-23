#ifndef	_LCNET_TASK_MANAGER_H_
#define	_LCNET_TASK_MANAGER_H_
#include "hxgrid/Interface/IAgent.h"
#include "hxgrid/Interface/hxgridinterface.h"
#include "lightstab_interface.h"
namespace	lc_net
{
	
	#include "hxgrid/Interface/IAgent.h"
//interface IGenericStream;



	class net_execution;
	class XRLC_LIGHT_API task_manager:
		public net_task_interface
	{
		friend void  Finalize(IGenericStream* outStream);
		xr_vector<net_execution*>	pool;
		CTimer						start_time;
		
		u32							tasks_completed;
		xrCriticalSection			pool_lock;
	private:
		void			send_task		( IGridUser& user, u32 id  );
		void			receive_result	( IGenericStream* inStream );

		void			send_result		( IGenericStream* outStream,  net_execution &e );
		net_execution* 	receive_task	( IAgent* agent, DWORD sessionId, IGenericStream* inStream  );
virtual bool			run_task		(	 IAgent* agent,
											 DWORD sessionId,
											 IGenericStream* inStream,
											 IGenericStream* outStream );
	xr_vector<net_execution*>::iterator find( u32 id );
	public:
				task_manager( );
		void	run();
		void	add_task( net_execution* task );
		void	create_global_data_write( LPCSTR save_path );
	};


	XRLC_LIGHT_API  task_manager &get_task_manager();
};
#endif