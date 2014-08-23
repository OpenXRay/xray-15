#include "stdafx.h"
#include "bone.h"






//////////////////////////////////////////////////////////////////////////
// BoneInstance methods
void	ENGINE_API	CBoneInstance::construct	()
{
	ZeroMemory					(this,sizeof(*this));
	mTransform.identity			();

	mRenderTransform.identity	();
	Callback_overwrite			= FALSE;
}
void	ENGINE_API	CBoneInstance::set_callback	(u32 Type, BoneCallback C, void* Param, BOOL overwrite)
{	
	Callback			= C; 
	Callback_Param		= Param; 
	Callback_overwrite	= overwrite;
	Callback_type		= Type;
}
void	ENGINE_API	CBoneInstance::reset_callback()
{
	Callback			= 0; 
	Callback_Param		= 0; 
	Callback_overwrite	= FALSE;
	Callback_type		= 0;
}

void	ENGINE_API	CBoneInstance::set_param	(u32 idx, float data)
{
	VERIFY		(idx<MAX_BONE_PARAMS);
	param[idx]	= data;
}
float	ENGINE_API	CBoneInstance::get_param	(u32 idx)
{
	VERIFY		(idx<MAX_BONE_PARAMS);
	return		param[idx];
}

#ifdef	DEBUG
void ENGINE_API	CBoneData::DebugQuery		(BoneDebug& L)
{
	for (u32 i=0; i<children.size(); i++)
	{
		L.push_back(SelfID);
		L.push_back(children[i]->SelfID);
		children[i]->DebugQuery(L);
	}
}
#endif

void ENGINE_API	CBoneData::CalculateM2B(const Fmatrix& parent)
{
	// Build matrix
	m2b_transform.mul_43	(parent,bind_transform);

	// Calculate children
	for (xr_vector<CBoneData*>::iterator C=children.begin(); C!=children.end(); C++)
		(*C)->CalculateM2B	(m2b_transform);

	m2b_transform.invert	();            
}