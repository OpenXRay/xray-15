/////////////////////////////////////////////////////////////////////////
//
//
//	Targeted IO Utility
//
//	Created 3-13-03: Tom Burke
//

#ifndef	I_TARGETED_IO_H
#define I_TARGETED_IO_H

// includes
#include "sfx.h"

class ITargetedIO
{
public:
	virtual void AddSaveTarget( int targetIndex, ReferenceTarget * rt ) = 0;
	virtual ReferenceTarget * GetSaveTarget( int targetIndex ) = 0;

	virtual int SaveToFile( TCHAR * fileName, FileIOType context ) = 0;
	virtual int LoadFromFile( TCHAR * fileName, FileIOType context ) = 0;

	// these utility methods can be used by renderers to hang on to reference targets between
	// the RenderPresetsPreLoad and RenderPresetsPostLoad calls.
	virtual void Store( int targetIndex, ReferenceTarget * rt ) = 0;
	virtual ReferenceTarget * Retrieve( int targetIndex ) = 0;

};

#endif