#include "stdafx.h"

#include "../xrLC_Light/net_task_manager.h"

#include "../xrLC_Light/lcnet_task_manager.h"

#include "../xrLC_Light/lcnet_execution_tasks_add.h"
void net_light ()
{
  


  create_net_task_manager();
  get_net_task_manager()->run();
  destroy_net_task_manager();

}

namespace lc_net
{

	void net_lightmaps ()
	{
		get_task_manager().create_global_data_write("");
		net_lightmaps_add_all_tasks(  );
		get_task_manager().run();

	}
}