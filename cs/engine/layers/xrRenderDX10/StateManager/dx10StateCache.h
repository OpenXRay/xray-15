#ifndef	dx10StateCache_included
#define	dx10StateCache_included
#pragma once

template <class IDeviceState, class StateDecs>
class dx10StateCache
{
	//	Public interface
public:
	dx10StateCache();
	~dx10StateCache();

	void			ClearStateArray();

	IDeviceState*	GetState( SimulatorStates& state_code );
	IDeviceState*	GetState( StateDecs& desc );
	//	Can be called on device destruction only!
	//	dx10State holds weak links on manager's states and 
	//	won't understand that state was destroyed
	//void	FlushStates();
	//	Private functionality

	//	Private declarations
private:
	struct StateRecord 
	{
		u32						m_crc;
		IDeviceState*			m_pState;
	};

private:
	void			CreateState( StateDecs desc, IDeviceState** ppIState );
	IDeviceState*	FindState( const StateDecs& desc, u32 StateCRC );

	//	Private data
private:
	//	This must be cleared on device destroy
	xr_vector<StateRecord>	m_StateArray;
};

extern dx10StateCache<ID3D10RasterizerState, D3D10_RASTERIZER_DESC> RSManager;
extern dx10StateCache<ID3D10DepthStencilState, D3D10_DEPTH_STENCIL_DESC> DSSManager;
extern dx10StateCache<ID3D10BlendState, D3D10_BLEND_DESC> BSManager;

#include "dx10StateCacheImpl.h"

#endif	//	dx10StateCache_included