/**********************************************************************
 *<
	FILE: Utilities.h

	DESCRIPTION:  Declaration of utility functions

	CREATED BY: Nikolai Sander
				
	HISTORY: created 6/17/99

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __FLEXUTILS_H_
#define __FLEXUTILS_H_

#include "max.h"
#include "utillib.h"

template <class T> class StrideArray {

protected:

	T *m_pT;
	DWORD m_stride;
public:
	
	StrideArray(T *pT, DWORD stride) { m_pT = pT; m_stride = stride; }
	StrideArray()		{ m_pT = NULL; m_stride = 0;}
	
	StrideArray& operator=(const StrideArray& tb) {
		m_pT = tb.m_pT;
		m_stride = tb.m_stride;
		return *this;
		}
	T& operator[](const int i) const {       // access ith entry.
		assert(m_pT);
		return(   *(T *) &((BYTE *) m_pT)[i*m_stride]); 
		}
};
#endif