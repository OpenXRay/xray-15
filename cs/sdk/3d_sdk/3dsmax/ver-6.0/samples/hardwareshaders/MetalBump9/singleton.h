//_____________________________________________________________________________
//
//	File: Singleton.h
//	
//
//_____________________________________________________________________________


#ifndef SINGLETON_H
#define SINGLETON_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//_____________________________________________________________________________

#include <cassert>

//_____________________________________________________________________________
//
//	Forward declare
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Defines
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Types
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Class definitions
//_____________________________________________________________________________

template<typename T> class Singleton
{
	static T *m_Singleton;

public:

	//
	//	Constructor
	//
	Singleton()
	{
        assert(!m_Singleton);

        int Offset = (int)(T *)1 - (int)(Singleton <T> *)(T*)1;

        m_Singleton = (T*)((int)this + Offset);
    }
	//
	//	Destructor
	//
	~Singleton()
    {  
		assert(m_Singleton);  
		m_Singleton = 0;  
	}
	//
	//	Methods
	//
    static T&	GetI(void);
    static T*	GetIPtr(void);
    static bool Exists();

};


//_____________________________________
//
//	GetI 
//
//_____________________________________


template <typename T> inline T& Singleton<T>::GetI(void)
{  
	assert(m_Singleton);  
	return(*m_Singleton);  
}

//_____________________________________
//
//	GetIPtr 
//
//_____________________________________

template <typename T> inline T* Singleton<T>::GetIPtr(void)
{  
	assert(m_Singleton);  
	return(m_Singleton);  
}

//_____________________________________
//
//	Exists 
//
//_____________________________________

template <typename T> inline bool Singleton<T>::Exists()
{
    return(m_Singleton != 0);
}

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


