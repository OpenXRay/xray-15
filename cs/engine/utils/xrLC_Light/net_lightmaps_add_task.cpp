#include "stdafx.h"
#include "net_execution_lightmaps.h"
#include "net_execution_factory.h"
#include "lcnet_task_manager.h"
#include "lcnet_execution_tasks_add.h"
#include "xrlc_globaldata.h"
namespace lc_net
{
	void net_lightmaps_add_task( u32 deflector_id )
	{
		tnet_execution_base< et_lightmaps > *el = lc_net::execution_factory.create<et_lightmaps>();
		el->implementation( ).construct(deflector_id);
		get_task_manager().add_task( el );
	}
	void net_lightmaps_add_all_tasks(  )
	{
		u32 size = inlc_global_data()->g_deflectors().size();
		for	(u32 dit = 0; dit<size; dit++)
			net_lightmaps_add_task( dit );
	}
}