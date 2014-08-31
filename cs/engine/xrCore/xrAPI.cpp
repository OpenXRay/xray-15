#include "stdafx.h"
#include "xrAPI.hpp"

IRender_interface* Render = nullptr;
IRenderFactory* RenderFactory = nullptr;
CDUInterface* DU = nullptr;
xr_token* vid_mode_token = nullptr;
IUIRender* UIRender = nullptr;

#ifdef DEBUG
IDebugRender* DRender = nullptr;
#endif
