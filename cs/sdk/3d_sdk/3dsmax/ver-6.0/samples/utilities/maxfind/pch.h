/****************************************************************************
 MAX File Finder
 Christer Janson
 September 19, 1998
 Pch.h - Precompiled header
 ***************************************************************************/
#define STRICT

#include "Windows.h"
#include "commctrl.h"
#include "stdio.h"
#include "shlobj.h"

#include "../../../include/udmIA64.h"
// WIN64 Cleanup: Shuler
// Until the MSVC 7.0 compiler is available, this
// header file is needed to complete the UDM implementation.
// Delete this once MSVC 7.0 is available.
