////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_movement_manager_obstacles.cpp
//	Created 	: 27.03.2007
//  Modified 	: 27.03.2007
//	Author		: Dmitriy Iassenev
//	Description : Stalker movement manager: dynamic obstacles avoidance
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stalker_movement_manager_obstacles.h"
#include "stalker_movement_manager_space.h"
#include "ai_space.h"
#include "ai/stalker/ai_stalker.h"
#include "restricted_object_obstacle.h"
#include "level_path_manager.h"
#include "ai_object_location.h"
#include "detail_path_manager.h"
#include "stalker_animation_manager.h"

#ifndef MASTER_GOLD
#	include "ai_debug.h"
#endif // MASTER_GOLD

static const u32	fail_check_time				= 1000;

stalker_movement_manager_obstacles::stalker_movement_manager_obstacles	(CAI_Stalker *object) :
	inherited						(object),
	m_last_dest_vertex_id			(u32(-1)),
	m_last_fail_time				(0),
	m_failed_to_build_path			(false)
{
	m_static_obstacles.construct	(this, m_failed_to_build_path);
	m_dynamic_obstacles.construct	(this, m_failed_to_build_path);
}

CRestrictedObject *stalker_movement_manager_obstacles::create_restricted_object	()
{
	m_restricted_object				= 
		xr_new<CRestrictedObjectObstacle>(
			&object(),
			m_static_obstacles.active_query(),
			m_dynamic_obstacles.active_query()
		);

	return							(m_restricted_object);
}

void stalker_movement_manager_obstacles::rebuild_path				()
{
#if 1
	level_path().make_inactual		();
	
	set_build_path_at_once			();
	update_path						();
#else
	level_path().select_intermediate_vertex();
	detail().make_inactual			();
#endif
	
	set_build_path_at_once			();
	update_path						();
}

bool stalker_movement_manager_obstacles::apply_border				(const obstacles_query &query)
{
	u32								start_vertex_id = object().ai_location().level_vertex_id();
	u32								dest_vertex_id = level_path().dest_vertex_id();
	CLevelGraph						&graph = ai().level_graph();

	restricted_object().CRestrictedObject::add_border(start_vertex_id,dest_vertex_id);

	AREA::const_iterator			B = query.area().begin(), I = B;
	AREA::const_iterator			E = query.area().end();
	for ( ; I != E; ++I) {
		if ((*I == dest_vertex_id))
			continue;

		if (*I == start_vertex_id)
			continue;
		
		graph.set_mask_no_check		(*I);
	}

	return							(true);
}

void stalker_movement_manager_obstacles::remove_border				(const obstacles_query &query)
{
	restricted_object().CRestrictedObject::remove_border();
	CLevelGraph						&graph = ai().level_graph();
	AREA::const_iterator			I = query.area().begin();
	AREA::const_iterator			E = query.area().end();
	for ( ; I != E; ++I)
		graph.clear_mask_no_check	(*I);
}

bool stalker_movement_manager_obstacles::can_build_restricted_path	(const obstacles_query &query)
{
	if (!apply_border(query)) {
		VERIFY						(!restricted_object().applied());
		return						(false);
	}

	typedef SBaseParameters<float,u32,u32>	evaluator_type;

	m_failed_to_build_path			= 
		!ai().graph_engine().search(
			ai().level_graph(),
			object().ai_location().level_vertex_id(),
			level_path().dest_vertex_id(),
			&m_temp_path,
			evaluator_type(
				type_max(_dist_type),
				_iteration_type(-1),
				4096
			)
		);

	remove_border					(query);

	VERIFY							(!restricted_object().applied());

	return							(!m_failed_to_build_path);
}

void stalker_movement_manager_obstacles::move_along_path_impl				(CPHMovementControl *movement_control, Fvector &dest_position, float time_delta)
{
#ifndef MASTER_GOLD
	if (psAI_Flags.test(aiObstaclesAvoidingStatic))
#endif // MASTER_GOLD
	{
		m_dynamic_obstacles.update		();
		if (!m_dynamic_obstacles.movement_enabled()) {
			float						desirable_speed = old_desirable_speed();
			set_desirable_speed			(0.f);

			inherited::move_along_path	(movement_control, dest_position, time_delta);

			set_desirable_speed			(desirable_speed);
			return;
		}
	}

	m_static_obstacles.update		();

	if (
#ifndef MASTER_GOLD
		(!psAI_Flags.test(aiObstaclesAvoidingStatic) &&
			m_dynamic_obstacles.need_path_to_rebuild()
		) ||
#endif // MASTER_GOLD
		m_static_obstacles.need_path_to_rebuild())
		rebuild_path				();

	inherited::move_along_path		(movement_control, dest_position, time_delta);
}

void stalker_movement_manager_obstacles::move_along_path					(CPHMovementControl *movement_control, Fvector &dest_position, float time_delta)
{
#ifndef MASTER_GOLD
	if (!psAI_Flags.test(aiObstaclesAvoiding)) {
		inherited::move_along_path	( movement_control, dest_position, time_delta);
		return;
	}
#endif // MASTER_GOLD

	if (Device.dwTimeGlobal < (m_last_fail_time + fail_check_time)) {
		inherited::move_along_path	( movement_control, dest_position, time_delta);
		return;
	}

	if (!move_along_path()) {
		inherited::move_along_path	( movement_control, dest_position, time_delta);
		return;
	}

	move_along_path_impl			( movement_control, dest_position, time_delta);
}

const float &stalker_movement_manager_obstacles::prediction_speed	() const
{
	return		(object().animation().target_speed());
}

void stalker_movement_manager_obstacles::remove_links				(CObject *object)
{
	inherited::remove_links			(object);
	m_static_obstacles.remove_links	(object);
	m_dynamic_obstacles.remove_links(object);
}