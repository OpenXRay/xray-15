/////////////////////////////////////////////////////////////////////////
//
//
//	SSE (Streaming SIMD extentions)
//
//	Created 9/15/2001	Claude Robillard
//

#ifndef	MAX_SSE_H
#define MAX_SSE_H

//////////////////////////////////////////////////////////////
//
//		IRendSSEControl compatibility interface
//
//
class IRenderSSEControl
{
public:
	// this interface's ID
	enum { IID = 0xcafdface };

	// This function returns TRUE when user whishes to use SSE for rendering optimizations
	virtual BOOL IsSSEEnabled() = 0;
};



#endif