#pragma once
#include "../state.h"

template<typename _Object>
class CStateMonsterAttackMoveToHomePoint : public CState<_Object> {
protected:
	typedef CState<_Object>		inherited;
	typedef CState<_Object>*	state_ptr;

	u32					m_target_node;
	bool				m_skip_camp;
	bool                m_ignore_enemies_while_hide;

public:
						CStateMonsterAttackMoveToHomePoint(_Object *obj);
						CStateMonsterAttackMoveToHomePoint(_Object *obj, bool ignore_enemies_while_hide);

	virtual	void		initialize				();
	virtual void 		finalize				();
	virtual void 		critical_finalize		();
	virtual void		remove_links			(CObject* object) { inherited::remove_links(object);}

	virtual bool		check_start_conditions	();
	virtual bool		check_completion		();

	virtual	void		reselect_state			();
	virtual	void		setup_substates			();
};

#include "monster_state_home_point_attack_inline.h"
