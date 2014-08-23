///////////////////////////////////////////////////////////////
//
//	Standard Pre-filter kernel functions
//

#ifndef STDKERNELS_H
#define STDKERNELS_H

// comment out this define to remove the r 2.5 pixelSize filter
#define INCLUDE_PIXELSIZE	


// Get the Kernel Class Descriptors
ClassDesc* GetAreaKernelDesc();
ClassDesc* GetPixelSizeKernelDesc();
ClassDesc* GetNTSCKernelDesc();
ClassDesc*  GetConeKernelDesc();
ClassDesc*  GetQuadraticKernelDesc();
ClassDesc* GetCubicKernelDesc();
ClassDesc* GetCatRomKernelDesc();
ClassDesc* GetGaussNarrowKernelDesc();
ClassDesc* GetGaussMediumKernelDesc();
ClassDesc* GetGaussWideKernelDesc();
ClassDesc* GetPavicUnitVolKernelDesc();
ClassDesc* GetPavicOpKernelDesc();
ClassDesc* GetHanningKernelDesc();
ClassDesc* GetHammingKernelDesc();
ClassDesc* GetBlackmanKernelDesc();
ClassDesc* GetBlackman361KernelDesc();
ClassDesc*  GetBlackman367KernelDesc();
ClassDesc* GetBlackman474KernelDesc();
ClassDesc* GetBlackman492KernelDesc();
ClassDesc* GetMax1KernelDesc();
ClassDesc* GetMax2KernelDesc();
ClassDesc* GetMax3KernelDesc();
ClassDesc* GetMitNetOpKernelDesc();
ClassDesc* GetMitNetNotchKernelDesc();
ClassDesc* GetNTSCKernelDesc();
ClassDesc* GetCookOpKernelDesc();
ClassDesc* GetCookVarKernelDesc();
ClassDesc* GetGaussVarKernelDesc();
ClassDesc* GetMitNetVarKernelDesc();
ClassDesc* GetCylinderVarKernelDesc();
ClassDesc* GetConeVarKernelDesc();
ClassDesc* GetQuadraticVarKernelDesc();
ClassDesc* GetCubicVarKernelDesc();
ClassDesc* GetStockingKernelDesc();
ClassDesc* GetBlendKernelDesc();






#endif

