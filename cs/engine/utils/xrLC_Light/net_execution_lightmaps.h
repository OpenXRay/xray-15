#ifndef _NET_EXECUTION_LIGHTMAPS_H_
#define _NET_EXECUTION_LIGHTMAPS_H_
#include "net_execution.h"
class net_task;
namespace lc_net
{
	

	typedef class tnet_execution_base<et_lightmaps> ::net_execution_impl 	execution_lightmaps;
	template<>
	class tnet_execution_base<et_lightmaps> ::net_execution_impl
		{
			u32				deflector_id;
			net_task		*task;
	public:
					net_execution_impl( ): deflector_id(-1),task(0){} 
		void		construct		( u32 d_id ) { deflector_id = d_id; }
		void		send_task		( IGridUser& user, IGenericStream* outStream, u32  id )	;
		LPCSTR		data_files		();
		void		receive_result	( IGenericStream* outStream );
		bool		receive_task	( IAgent* agent, DWORD sessionId, IGenericStream* inStream );
		void		send_result		( IGenericStream* outStream )	;
		bool		execute			()	;

	};
	//execution_lightmaps
};
#endif