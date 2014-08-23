#include	"pch_script.h"

#include	"animation_script_callback.h"
#include	"script_callback_ex.h"
#include	"gameobject.h"
#include	"game_object_space.h"
#include	"../Include/xrRender/KinematicsAnimated.h"

CBlend	*PlayMotionByParts(IKinematicsAnimated* sa, MotionID motion_ID, BOOL bMixIn, PlayCallback Callback, LPVOID CallbackParam)
{
	CBlend	*ret = 0;
	for (u16 i=0; i<MAX_PARTS; ++i) 
	{

		CBlend	*blend	= sa->LL_PlayCycle( i, motion_ID, bMixIn, Callback, CallbackParam );
		if(blend && !ret)
				ret = blend;
			//m_anim_blend = PKinematicsAnimated->PlayCycle(*visual->startup_animation);
	}
	return ret;
}

CBlend*	anim_script_callback::play_cycle( IKinematicsAnimated* sa,const shared_str& anim )
{

	MotionID	m			=	sa->LL_MotionID		( *anim )			;
	if(sa->LL_GetMotionDef	( m )->StopAtEnd())
	{
		on_end		= false;
		on_begin	= false;
		is_set		= true;
		return PlayMotionByParts( sa, m, FALSE, anim_callback, this );
	} else
	{
		on_end		= false;
		on_begin	= false;
		is_set		= false;
		return PlayMotionByParts( sa, m, FALSE, 0, 0 );
	}
}

void	anim_script_callback::anim_callback		(CBlend*		B)
{
	
	anim_script_callback* ths = ( ( anim_script_callback*) B->CallbackParam );
	VERIFY( ths );
	VERIFY( ths->is_set );
	if( B->timeTotal - B->timeCurrent < B->timeCurrent )
	{	
		VERIFY( B->speed > 0.f );
		ths->on_end = true;
	} else
	{
		VERIFY( B->speed < 0.f );
		ths->on_begin = true;
	}
}

void	anim_script_callback::update( CGameObject &O )
{
	if( !is_set )
			return;
	if(!on_end && !on_begin)
		return;
	O.callback		(GameObject::eScriptAnimation)	( on_end );
	on_end		= false;
	on_begin	= false;
}