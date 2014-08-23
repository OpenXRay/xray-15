//////////////////////////////////////////////////////////////
//
//	Render element utilities
//

// compute diffuse illumination w/ shadow clr
float IllumShadow( ShadeContext& sc, Color& shadowClr ) ;

//	Compute Illuminance - utility to compute illuminance for light maps
Color computeIlluminance( ShadeContext& sc, BOOL ambOn, BOOL diffOn );
