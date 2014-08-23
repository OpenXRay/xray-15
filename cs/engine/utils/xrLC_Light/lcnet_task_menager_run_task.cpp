#include "stdafx.h"

#include "lcnet_task_manager.h"
#include "net_execution.h"

namespace lc_net
{
	bool	task_manager::run_task		(	IAgent* agent,
											DWORD sessionId,
											IGenericStream* inStream,
											IGenericStream* outStream )
	{
		net_execution * e = receive_task( agent, sessionId, inStream );
		if(!e || ! e->execute() )
			return false;
		send_result( outStream, *e  );
		return true;
	}

};