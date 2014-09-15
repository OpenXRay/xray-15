#ifndef __LIGHTTHREAD_H__
#define __LIGHTTHREAD_H__


#include "xrthread.h"
using DWORDVec = xr_vector<u32>;
using DWORDIt = DWORDVec::iterator;
class	LightThread : public CThread
{
	u32		Nstart, Nend;
	DWORDVec	box_result;
public:
	LightThread			(u32 ID, u32 _start, u32 _end) : CThread(ID)
	{
		Nstart	= _start;
		Nend	= _end;
	}
	virtual void		Execute();

};
#endif //__LIGHTTHREAD_H__