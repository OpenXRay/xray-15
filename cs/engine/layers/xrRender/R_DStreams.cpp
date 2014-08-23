#include "stdafx.h"
#pragma hdrstop

#include "ResourceManager.h"
#include "R_DStreams.h"

#include "../xrRender/dxRenderDeviceRender.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int		rsDVB_Size			= 512+1024;
int		rsDIB_Size			= 512;

void _VertexStream::Create	()
{
	//dxRenderDeviceRender::Instance().Resources->Evict		();
	DEV->Evict();

	mSize					= rsDVB_Size*1024;
#ifdef	USE_DX10
	D3D10_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth        = mSize;
	bufferDesc.Usage            = D3D10_USAGE_DYNAMIC;	
	bufferDesc.BindFlags        = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags   = D3D10_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags        = 0;

	R_CHK					(HW.pDevice->CreateBuffer( &bufferDesc, 0, &pVB ));
#else	//	USE_DX10
	R_CHK					(HW.pDevice->CreateVertexBuffer( mSize, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &pVB, NULL));
#endif	//	USE_DX10

	R_ASSERT				(pVB);

	mPosition				= 0;
	mDiscardID				= 0;

	Msg("* DVB created: %dK", mSize/1024);
}

void _VertexStream::Destroy	()
{
	_RELEASE				(pVB);
	_clear					();
}

void* _VertexStream::Lock	( u32 vl_Count, u32 Stride, u32& vOffset )
{
#ifdef DEBUG
	PGO					(Msg("PGO:VB_LOCK:%d",vl_Count));
	VERIFY				(0==dbg_lock);
	dbg_lock			++;
#endif

	// Ensure there is enough space in the VB for this data
	u32	bytes_need		= vl_Count*Stride;
	R_ASSERT2			((bytes_need<=mSize) && vl_Count, make_string("bytes_need = %d, mSize = %d, vl_Count = %d", bytes_need, mSize, vl_Count));

	// Vertex-local info
	u32 vl_mSize		= mSize/Stride;
	u32 vl_mPosition	= mPosition/Stride + 1;

	// Check if there is need to flush and perform lock
	BYTE* pData			= 0;
	if ((vl_Count+vl_mPosition) >= vl_mSize)
	{
		// FLUSH-LOCK
		mPosition			= 0;
		vOffset				= 0;
		mDiscardID			++;

#ifdef	USE_DX10
		pVB->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&pData);
		pData += vOffset;
#else	//	USE_DX10
		pVB->Lock( mPosition, bytes_need, (void**)&pData, LOCKFLAGS_FLUSH);
#endif	//	USE_DX10
	} else {
		// APPEND-LOCK
		mPosition			= vl_mPosition*Stride;
		vOffset				= vl_mPosition;

#ifdef	USE_DX10
		pVB->Map(D3D10_MAP_WRITE_NO_OVERWRITE, 0, (void**)&pData);
		pData += vOffset*Stride;
#else	//	USE_DX10
		pVB->Lock			( mPosition, bytes_need, (void**)&pData, LOCKFLAGS_APPEND);
#endif	//	USE_DX10
	}
	VERIFY				( pData );

	return LPVOID		( pData );
}

void	_VertexStream::Unlock		( u32 Count, u32 Stride)
{
#ifdef DEBUG
	PGO					(Msg("PGO:VB_UNLOCK:%d",Count));
	VERIFY				(1==dbg_lock);
	dbg_lock			--;
#endif
	mPosition			+=	Count*Stride;

	VERIFY				(pVB);

#ifdef	USE_DX10
	pVB->Unmap();
#else	//	USE_DX10
	pVB->Unlock();
#endif	//	USE_DX10
}

void	_VertexStream::reset_begin	()
{
	old_pVB				= pVB;
	Destroy				();
}
void	_VertexStream::reset_end	()
{
	Create				();
	//old_pVB				= NULL;
}

_VertexStream::_VertexStream()
{
	_clear();
};

void _VertexStream::_clear()
{
    pVB			= NULL;
    mSize		= 0;
    mPosition	= 0;
    mDiscardID	= 0;
#ifdef DEBUG
	dbg_lock	= 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
void	_IndexStream::Create	()
{
	//dxRenderDeviceRender::Instance().Resources->Evict		();
	DEV->Evict();

	mSize					= rsDIB_Size*1024;

#ifdef	USE_DX10
	D3D10_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth        = mSize;
	bufferDesc.Usage            = D3D10_USAGE_DYNAMIC;	
	bufferDesc.BindFlags        = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags   = D3D10_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags        = 0;

	R_CHK					(HW.pDevice->CreateBuffer( &bufferDesc, 0, &pIB ));
#else	//	USE_DX10
	R_CHK					(HW.pDevice->CreateIndexBuffer( mSize, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB, NULL));
#endif	//	USE_DX10
	R_ASSERT				(pIB);

	mPosition				= 0;
	mDiscardID				= 0;

	Msg("* DIB created: %dK", mSize/1024);
}

void	_IndexStream::Destroy()
{
	_RELEASE				(pIB);
	_clear					();
}

u16*	_IndexStream::Lock	( u32 Count, u32& vOffset )
{
	PGO						(Msg("PGO:IB_LOCK:%d",Count));
	vOffset					= 0;
	BYTE* pLockedData		= 0;

	// Ensure there is enough space in the VB for this data
	R_ASSERT				((2*Count<=mSize) && Count);

	// If either user forced us to flush,
	// or there is not enough space for the index data,
	// then flush the buffer contents
	u32 dwFlags = LOCKFLAGS_APPEND;
	if ( 2*( Count + mPosition ) >= mSize )
	{
		mPosition	= 0;						// clear position
		dwFlags		= LOCKFLAGS_FLUSH;			// discard it's contens
		mDiscardID	++;
	}
#ifdef	USE_DX10
	D3D10_MAP MapMode = (dwFlags==LOCKFLAGS_APPEND) ? 
		D3D10_MAP_WRITE_NO_OVERWRITE : D3D10_MAP_WRITE_DISCARD;
	pIB->Map( MapMode, 0, (void**)&pLockedData);
	pLockedData += mPosition * 2;

#else	//	USE_DX10
	pIB->Lock				( mPosition * 2, Count * 2, (void**) &pLockedData, dwFlags);
#endif	//	USE_DX10
	VERIFY					(pLockedData);

	vOffset					=	mPosition;

	return					LPWORD(pLockedData);
}

void	_IndexStream::Unlock(u32 RealCount)
{
	PGO						(Msg("PGO:IB_UNLOCK:%d",RealCount));
	mPosition				+=	RealCount;
	VERIFY					(pIB);
#ifdef	USE_DX10
	pIB->Unmap();
#else	//	USE_DX10
	pIB->Unlock();
#endif	//	USE_DX10
}

void	_IndexStream::reset_begin	()
{
	old_pIB				= pIB;
	Destroy				();
}
void	_IndexStream::reset_end	()
{
	Create				();
	//old_pIB				= NULL;
}
