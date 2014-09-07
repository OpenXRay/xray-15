#include "stdafx.h"
#include "controller.h"
#include "controller_state_manager.h"

#include "controller_animation.h"
#include "controller_direction.h"
#include "AI/Monsters/control_direction_base.h"
#include "AI/Monsters/control_movement_base.h"
#include "AI/Monsters/control_path_builder_base.h"

#include "AI/Monsters/controlled_entity.h"

#include "AI/Monsters/States/monster_state_rest.h"
#include "controller_state_attack.h"
#include "AI/Monsters/States/monster_state_attack_melee.h"
#include "AI/Monsters/States/monster_state_attack_run.h"
#include "AI/Monsters/States/monster_state_eat.h"
#include "AI/Monsters/States/monster_state_panic.h"
#include "AI/Monsters/States/monster_state_hear_int_sound.h"
#include "AI/Monsters/States/monster_state_hear_danger_sound.h"
#include "AI/Monsters/States/monster_state_hitted.h"
#include "AI/Monsters/States/monster_state_attack.h"

#include "EntityCondition.h"

#include "AI/Monsters/States/state_test_state.h"

CStateManagerController::CStateManagerController(CController *obj) : inherited(obj)
{
	add_state(eStateRest,					new CStateMonsterRest<CController> (obj));
	add_state(eStatePanic,					new CStateMonsterPanic<CController> (obj));
	add_state(eStateHearInterestingSound,	new CStateMonsterHearInterestingSound<CController> (obj));
	add_state(eStateHearDangerousSound,		new CStateMonsterHearDangerousSound<CController> (obj));
	add_state(eStateHitted,					new CStateMonsterHitted<CController> (obj));
	
	add_state(eStateAttack_Run,				new CStateMonsterAttackRun<CController> (obj));
	add_state(eStateAttack_Melee,			new CStateMonsterAttackMelee<CController> (obj));

// 	add_state(
// 		eStateAttack, 
// 	    new CStateControllerAttack<CController>
// 		(
// 		 obj, 
// 		 new CStateMonsterAttackRun<CController> (obj), 
// 		 new CStateMonsterAttackMelee<CController> (obj) 
// 		)
// 	);

	add_state(eStateEat,		new CStateMonsterEat<CController> (obj));
	add_state(eStateCustom,		new CStateControlHide<CController> (obj));
}

CStateManagerController::~CStateManagerController()
{
}

void CStateManagerController::reinit()
{
	inherited::reinit();
	object->set_mental_state(CController::eStateIdle);
}


#define FIND_ENEMY_TIME_ENEMY_HIDDEN	5000
#define FIND_ENEMY_MAX_DISTANCE			10.f

void CStateManagerController::execute()
{
	u32 state_id = u32(-1);
		
	const CEntityAlive* enemy	= object->EnemyMan.get_enemy();

	// Lain: changed logic
	if (enemy) {

		if ( object->EnemyMan.get_danger_type() == eStrong )
		{
			state_id = eStatePanic; 
		}
		else
		{
			if ( current_substate == eStateAttack_Melee )
			{
				if ( get_state(eStateAttack_Melee)->check_completion() )
				{
					state_id = eStateAttack_Run;
				}
				else
				{
					state_id = eStateAttack_Melee;
				}
			}
			else
			{
				if ( get_state(eStateAttack_Melee)->check_start_conditions() )
				{
					state_id = eStateAttack_Melee;
				}
				else
				{
					state_id = eStateAttack_Run;
				}
			}
		}		

	} else if (object->HitMemory.is_hit()) {
		state_id = eStateHitted;
	} else if (object->hear_dangerous_sound) {
		state_id = eStateHearDangerousSound;
	} else if (object->hear_interesting_sound) {
		state_id = eStateHearInterestingSound;
	} else {
		if (can_eat())	state_id = eStateEat;
		else			state_id = eStateRest;
	}

	if (enemy) object->set_controlled_task(eTaskAttack);
	else object->set_controlled_task(eTaskFollow);

	select_state(state_id); 

	// выполнить текущее состояние
	get_state_current()->execute();

	prev_substate = current_substate;
}
