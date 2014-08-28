#include "stdafx.hpp"
#include "xrAPI.hpp"

IRender_interface* Render = nullptr;
IRenderFactory* RenderFactory = nullptr;
CDUInterface* DU = nullptr;
xr_token* vid_mode_token = nullptr;
IUIRender* UIRender = nullptr;

#ifdef DEBUG
    IDebugRender* DRender = nullptr;
#endif // DEBUG

/*
// This is an example of an exported variable
XRAPI_API int nxrAPI=0;

// This is an example of an exported function.
XRAPI_API int fnxrAPI(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see xrAPI.h for the class definition
CxrAPI::CxrAPI()
{
    return;
}
*/
