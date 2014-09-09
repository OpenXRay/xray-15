#pragma once

using uint = unsigned int;
using uint32 = unsigned int;
using int32 = signed int;
using ushort = unsigned short;
using uint16 = unsigned short;
using int16 = signed short;
using uint64 = unsigned long long;
using int64 = signed long long;
using sbyte = signed char;
using int8 = signed char;
using byte = unsigned char;
using uint8 = unsigned char;

#ifdef WIN64
    using intptr = int64;
#else
    using intptr = int32;
#endif

//

#define xrEXPORT __declspec(dllexport)
#define xrIMPORT __declspec(dllimport)
