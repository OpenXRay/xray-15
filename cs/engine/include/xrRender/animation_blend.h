#pragma once

#include	"animation_motion.h"
//*** Run-time Blend definition *******************************************************************
class  CBlend {
public:
	enum ECurvature
	{
		eFREE_SLOT=0,
		//		eFixed,
		eAccrue,
		eFalloff,
		eFORCEDWORD = u32(-1)
	};
public:
	float			blendAmount;
	float			timeCurrent;
	float			timeTotal;
	MotionID		motionID;
	u16				bone_or_part;	// startup parameters
	u8				channel;
	ECurvature		blend;
	float			blendAccrue;	// increasing
	float			blendFalloff;	// decreasing
	float			blendPower;			
	float			speed;

	BOOL			playing;
	BOOL			stop_at_end_callback;
	BOOL			stop_at_end;
	BOOL			fall_at_end;
	PlayCallback	Callback;
	void*			CallbackParam;

	u32				dwFrame;

	u32				mem_usage			(){ return sizeof(*this); }
IC	bool			update_time			( float dt );
IC  void			update_play			( float dt, PlayCallback _Callback );
IC	bool			update_falloff		( float dt );
IC	bool			update				( float dt, PlayCallback _Callback );
};



IC void CBlend::update_play( float dt, PlayCallback _Callback )
{

	float pow_dt = dt;
	if( pow_dt < 0.f )
	{
		pow_dt = 0;
		if( stop_at_end )
		{
			VERIFY( blendAccrue>0.f );
			pow_dt = timeCurrent + dt - 1.f/blendAccrue;
			clamp( pow_dt, dt, 0.f );
		}
	}
	
	blendAmount 		+= pow_dt*blendAccrue*blendPower;

	clamp				( blendAmount, 0.f, blendPower); 


	if( !update_time( dt ) )//reached end 
		return;

	if ( _Callback &&  stop_at_end_callback )	
		_Callback( this );		// callback only once

	stop_at_end_callback		= FALSE;

	if( fall_at_end )
	{
		blend = eFalloff;
		blendFalloff = 2.f;
		//blendAccrue = timeCurrent;
	}
	return ;
}

IC	bool CBlend::update_time			( float dt )
{
	if (!playing) 
			return false;
	float quant = dt*speed;
	timeCurrent += quant; // stop@end - time is not going

	bool	running_fwrd	=  ( quant > 0 );
	float	const END_EPS	=	SAMPLE_SPF+EPS;
	bool	at_end			=	running_fwrd && ( timeCurrent > ( timeTotal-END_EPS ) );
	bool	at_begin		=	!running_fwrd && ( timeCurrent < 0.f );
	
	if( !stop_at_end )
	{
		if( at_begin )
			timeCurrent+= timeTotal;
		if( at_end )
			timeCurrent -= ( timeTotal-END_EPS );
		VERIFY( timeCurrent>=0.f );
		return false;
	}
	if( !at_end && !at_begin )
					return false;

	if( at_end )
	{
		timeCurrent	= timeTotal-END_EPS;		// stop@end - time frozen at the end
		if( timeCurrent<0.f ) timeCurrent =0.f; 
	}
	else
		timeCurrent	= 0.f;

	VERIFY( timeCurrent>=0.f );
	return true;
}

IC bool CBlend::update_falloff( float dt )
{
	update_time( dt );
	
	//if(  dt<0.f || timeCurrent >= blendAccrue )
		blendAmount 		-= dt*blendFalloff*blendPower;

	bool ret			= blendAmount<=0;
	clamp				( blendAmount, 0.f, blendPower);
	return ret;
}

IC bool CBlend::update( float dt, PlayCallback _Callback )
{
	switch (blend) 
	{
		case eFREE_SLOT: 
			NODEFAULT;
		case eAccrue:
			update_play( dt, _Callback );
			break;
		case eFalloff:
			if( update_falloff( dt ) )
				return true;
			break;
		default: 
			NODEFAULT;
	}
	return false;
}

class IBlendDestroyCallback
{
	public:
		virtual void BlendDestroy( CBlend& blend )	= 0;
};