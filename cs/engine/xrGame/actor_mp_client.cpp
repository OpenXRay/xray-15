#include "stdafx.h"
#include "actor_mp_client.h"
#include "actorcondition.h"
#include "../xrEngine/CameraBase.h"

CActorMP::CActorMP			()
{
	//m_i_am_dead				= false;
}

void CActorMP::OnEvent		( NET_Packet &P, u16 type)
{
	inherited::OnEvent		(P,type);
#ifdef DEBUG
	if (type == GE_ACTOR_MAX_HEALTH)
	{
		Msg("--- CActorMP after GE_ACTOR_MAX_HEALTH health is: %2.04f", m_state_holder.state().health);
	}
#endif // #ifdef DEBUG
}

void CActorMP::Die			(CObject *killer)
{
	//m_i_am_dead				= true;
	//conditions().health()	= 0.f;
	conditions().SetHealth( 0.f );
	inherited::Die			(killer);

	if(OnServer())
	{ //transfer all items to bag
	
	}
}

void CActorMP::cam_Set		(EActorCameras style)
{
	if (style != eacFirstEye)
		return;
	CCameraBase* old_cam = cam_Active();
	cam_active = style;
	old_cam->OnDeactivate();
	cam_Active()->OnActivate(old_cam);
}
