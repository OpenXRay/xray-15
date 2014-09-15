#pragma once
#include "Common.hpp"

using s8 = XRay::int8;
using u8 = XRay::uint8;
using s16 = XRay::int16;
using u16 = XRay::uint16;
using s32 = XRay::int32;
using u32 = XRay::uint32;
using s64 = XRay::int64;
using u64 = XRay::uint64;
using f32 = float;
using f64 = double;
using pstr = char*;
using pcstr = const char*;

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#define type_max(T) (std::numeric_limits<T>::max())
// XXX nitrocaster: std::numeric_limits<T>::lowest() should be used (testing is required before fix)
#define type_min(T) (-std::numeric_limits<T>::max())
#define type_zero(T) (std::numeric_limits<T>::min())
#define type_epsilon(T) (std::numeric_limits<T>::epsilon())
#pragma pop_macro("min")
#pragma pop_macro("max")

#define int_max	type_max(int)
#define int_min	type_min(int)
#define int_zero type_zero(int)

#define flt_max	type_max(float)
#define flt_min	type_min(float)

#ifdef FLT_MAX
#undef FLT_MAX
#endif
#ifdef FLT_MIN
#undef FLT_MIN
#endif

#define FLT_MAX flt_max
#define FLT_MIN flt_min

#define flt_zero type_zero(float)
#define flt_eps type_epsilon(float)                 
#define dbl_max type_max(double)
#define dbl_min type_min(double)
#define dbl_zero type_zero(double)
#define dbl_eps type_epsilon(double)

using string16 = char[16];
using string32 = char[32];
using string64 = char[64];
using string128 = char[128];
using string256 = char[256];
using string512 = char[512];
using string1024 = char[1024];
using string2048 = char[2048];
using string4096 = char[4096];
using string_path = char[2*_MAX_PATH];
