#pragma once

namespace XRay
{
using uint = unsigned int;
using uint32 = unsigned __int32;
using int32 = __int32;
using ushort = unsigned __int16;
using uint16 = unsigned __int16;
using int16 = __int16;
using uint64 = unsigned __int64;
using int64 = __int64;
using sbyte = char;
using int8 = char;
using byte = unsigned char;
using uint8 = unsigned char;

#ifdef WIN64
using intptr = int64;
#else
using intptr = int32;
#endif
}

#define XR_EXPORT __declspec(dllexport)
#define XR_IMPORT __declspec(dllimport)
