#ifdef _EDITOR
#	include "skeletonX.h"
#	include "skeletoncustom.h"
#else // _EDITOR
#include <xrEngine/bone.h>
#include "SkeletonXVertRender.h"
//#	include "../xrEngine/skeletonX.h"
//#	include "../Layers/xrRender/skeletonX.h"
//#	include "../Layers/xrRender/skeletoncustom.h"
#endif // _EDITOR

void skin1W(vertRender* D, vertBoned1W* S, u32 vCount, CBoneInstance* Bones);

void skin2W(vertRender* D, vertBoned2W* S, u32 vCount, CBoneInstance* Bones);

void skin3W(vertRender* D, vertBoned3W* S, u32 vCount, CBoneInstance* Bones);

void skin4W(vertRender* D, vertBoned4W* S, u32 vCount, CBoneInstance* Bones);
