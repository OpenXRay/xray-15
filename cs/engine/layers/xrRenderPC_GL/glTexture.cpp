// Texture.cpp: implementation of the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

void fix_texture_name(LPSTR fn)
{
	LPSTR _ext = strext(fn);
	if (_ext &&
		(0 == stricmp(_ext, ".tga") ||
		0 == stricmp(_ext, ".dds") ||
		0 == stricmp(_ext, ".bmp") ||
		0 == stricmp(_ext, ".ogm")))
		*_ext = 0;
}
