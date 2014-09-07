#include "stdafx.h"
#include "boar.h"
#include "boar_state_manager.h"

#include "AI/Monsters/control_animation_base.h"
#include "AI/Monsters/control_direction_base.h"
#include "AI/Monsters/control_movement_base.h"
#include "AI/Monsters/control_path_builder_base.h"

#include "AI/Monsters/States/monster_state_rest.h"
#include "AI/Monsters/States/monster_state_attack.h"
#include "AI/Monsters/States/monster_state_panic.h"
#include "AI/Monsters/States/monster_state_eat.h"
#include "AI/Monsters/States/monster_state_hear_int_sound.h"
#include "AI/Monsters/States/monster_state_hear_danger_sound.h"
#include "AI/Monsters/States/monster_state_hitted.h"
#include "AI/Monsters/States/monster_state_controlled.h"
#include "AI/Monsters/States/monster_state_help_sound.h"
#include "AI/Monsters/group_states/group_state_home_point_attack.h"

CStateManagerBoar::CStateManagerBoar(CAI_Boar *monster) : inherited(monster)
{
	add_state(eStateRest,					new CStateMonsterRest<CAI_Boar> (monster));
	add_state(eStatePanic,					new CStateMonsterPanic<CAI_Boar> (monster));

 	CStateMonsterAttackMoveToHomePoint<CAI_Boar>* move2home = 
 		new CStateMonsterAttackMoveToHomePoint<CAI_Boar> (monster, true);
 
 	add_state(eStateAttack,					new CStateMonsterAttack<CAI_Boar> (monster, move2home));
//	add_state(eStateAttack,					new CStateMonsterAttack<CAI_Boar> (monster));

	add_state(eStateEat,					new CStateMonsterEat<CAI_Boar> (monster));
	add_state(eStateHearInterestingSound,	new CStateMonsterHearInterestingSound<CAI_Boar> (monster));
	add_state(eStateHearDangerousSound,		new CStateMonsterHearDangerousSound<CAI_Boar> (monster));
	add_state(eStateHitted,					new CStateMonsterHitted<CAI_Boar> (monster));
	add_state(eStateControlled,				new CStateMonsterControlled<CAI_Boar> (monster));
	add_state(eStateHearHelpSound,			new CStateMonsterHearHelpSound<CAI_Boar> (monster));
}

void CStateManagerBoar::execute()
{
	u32 state_id = u32(-1);

	if (!object->is_under_control()) {
	
		const CEntityAlive* enemy	= object->EnemyMan.get_enemy();

		if (enemy) {
			switch (object->EnemyMan.get_danger_type()) {
				case eStrong:	state_id = eStatePanic; break;
				case eWeak:		state_id = eStateAttack; break;
			}
		} else if (object->HitMemory.is_hit()) {
			state_id = eStateHitted;
		} else if (check_state(eStateHearHelpSound)) {
			state_id = eStateHearHelpSound;
		} else if (object->hear_interesting_sound) {
			state_id = eStateHearInterestingSound;
		} else if (object->hear_dangerous_sound) {
			state_id = eStateHearDangerousSound;	
		} else {
			if (can_eat())	state_id = eStateEat;
			else			state_id = eStateRest;
		}
	} else state_id = eStateControlled;
	
	select_state(state_id); 

	// выполнить текущее состояние
	get_state_current()->execute();

	prev_substate = current_substate;
}
