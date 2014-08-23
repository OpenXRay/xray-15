//Custom MAX include so that libtiff can use the tiff string table tif.rc.

#ifndef __STRTAB_H
#define __STRTAB_H

#include "tifrc.h"

#ifdef __cplusplus
extern "C"
	{
#endif

	//#include "tif.h"
	TCHAR *GetString(int id);

#ifdef __cplusplus
	}
#endif


#endif //ndef __STRTAB_H