/*********************************************************************NVMH3****
Path:  E:\Documents and Settings\sdomine\My Documents\CGFX_Beta_Runtime\include
File:  ICgFXEffect.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:




******************************************************************************/
#ifndef __ICGFX_EFFECT_H
#define __ICGFX_EFFECT_H

#include "cgfx_stddefs.h"
#define CGFX_MAX_DIMENSIONS 4

#ifndef CGFXDLL_API
#define CGFXDLL_API
#endif

#ifdef WIN32
#define CGFXDLL_EXPORT __cdecl
#else
#define CGFXDLL_EXPORT
#endif // WIN32

//////////////////////////////////////////////////////////////////////////////
// Types /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef LPCSTR CGFXHANDLE;

enum CgFXPARAMETERTYPE {
    CgFXPT_UNKNOWN			= 0,
    CgFXPT_BOOL,
    CgFXPT_INT,
    CgFXPT_FLOAT,
    CgFXPT_DWORD,
    CgFXPT_STRING,

    CgFXPT_TEXTURE1D,
    CgFXPT_TEXTURE2D,
    CgFXPT_TEXTURERECT,
    CgFXPT_TEXTURECUBE,
    CgFXPT_TEXTURE3D,

    CgFXPT_SAMPLER1D,
    CgFXPT_SAMPLER2D,
    CgFXPT_SAMPLERRECT,
    CgFXPT_SAMPLERCUBE,
    CgFXPT_SAMPLER3D,

    CgFXPT_VERTEXSHADER,
    CgFXPT_PIXELSHADER,

    CgFXPT_STRUCT,
    
    // force 32-bit size enum
    CgFXPT_FORCE_DWORD      = 0x7fffffff
};

enum CgFXPARAMETERCLASS {
    CgFXPC_SCALAR,
    CgFXPC_VECTOR,
    CgFXPC_MATRIX_ROWS,
    CgFXPC_MATRIX_COLUMNS,
    CgFXPC_OBJECT,
    CgFXPC_STRUCT,

    CgFXPC_FORCE_DWORD      = 0x7fffffff
};

struct CgFXEFFECT_DESC {
    LPCSTR Creator;
    UINT Parameters;
    UINT Techniques;
    UINT Functions;
};

// CgFXPARAMETER_DESC::Flags
#define CGFX_PARAMETER_SHARED 1
#define CGFX_PARAMETER_LITERAL 2
#define CGFX_PARAMETER_ANNOTATION 4 

struct CgFXPARAMETER_DESC {
    LPCSTR Name;
    LPCSTR Semantic;
    CgFXPARAMETERCLASS Class;
    CgFXPARAMETERTYPE Type;
    UINT Dimension[CGFX_MAX_DIMENSIONS]; // 
    UINT Rows, Columns, Elements;
    UINT Annotations;
    UINT StructMembers;
    UINT Bytes;
    DWORD Flags;
};

struct CgFXTECHNIQUE_DESC {
    LPCSTR Name;
    UINT Annotations;
    UINT Passes;
};

struct CgFXVARYING {
    LPCSTR Name;
    UINT Resource;
    UINT Elements;
};

struct CgFXSAMPLER_INFO {
    UINT Type;
};

#define MAX_CGFX_DECL_LENGTH 16

struct CgFXPASS_DESC {
    LPCSTR Name;
    UINT Annotations;
    DWORD VSVersion;
    DWORD PSVersion;
    UINT VertexVaryingUsed;
    CgFXVARYING VertexVarying[MAX_CGFX_DECL_LENGTH];
    UINT FragmentVaryingUsed;
    CgFXVARYING FragmentVarying[MAX_CGFX_DECL_LENGTH];
    UINT PSSamplersUsed;
    CgFXSAMPLER_INFO PSSamplers[16];
};

enum CgFXMode {
    CgFX_Unknown,
    CgFX_OpenGL,
    CgFX_Direct3D8,
    CgFX_Direct3D9
};

//////////////////////////////////////////////////////////////////////////////
// Base Effect ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// base interface. extended by Effect and EffectCompiler.
//////////////////////////////////////////////////////////////////////////////
class ICgFXBaseEffect
{
public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;

    virtual CGFXHANDLE GetAnnotation(CGFXHANDLE object, UINT num) = 0;
    virtual CGFXHANDLE GetAnnotationByName(CGFXHANDLE object, LPCSTR name) = 0;

    virtual CGFXHANDLE GetParameter(CGFXHANDLE parent, UINT index) = 0;
    virtual CGFXHANDLE GetParameterByName(CGFXHANDLE parent, LPCSTR name) = 0;
    virtual CGFXHANDLE GetParameterBySemantic(CGFXHANDLE parent, LPCSTR name) = 0;
    virtual CGFXHANDLE GetParameterElement(CGFXHANDLE parent, UINT element) = 0;

    virtual CGFXHANDLE GetPass(CGFXHANDLE technique, UINT index) = 0;
    virtual CGFXHANDLE GetPassByName(CGFXHANDLE technique, LPCSTR name) = 0;

    virtual CGFXHANDLE GetTechnique(UINT index) = 0;
    virtual CGFXHANDLE GetTechniqueByName(LPCSTR name) = 0;

    virtual HRESULT GetDesc(CgFXEFFECT_DESC* pDesc) = 0;
    virtual HRESULT GetParameterDesc(CGFXHANDLE pParameter, CgFXPARAMETER_DESC* pDesc) = 0;

    virtual HRESULT GetTechniqueDesc(CGFXHANDLE pTechnique, CgFXTECHNIQUE_DESC* pDesc) = 0;
    virtual HRESULT GetPassDesc(CGFXHANDLE pPass, CgFXPASS_DESC* pDesc) = 0;

    virtual HRESULT SetValue(CGFXHANDLE pName, LPCVOID pData, UINT Bytes) = 0;
    virtual HRESULT GetValue(CGFXHANDLE pName, LPVOID pData, UINT Bytes) = 0;

    virtual HRESULT SetFloat(CGFXHANDLE pName, FLOAT f) = 0;
    virtual HRESULT GetFloat(CGFXHANDLE pName, FLOAT* f) = 0;
    virtual HRESULT GetFloatArray(CGFXHANDLE pName, FLOAT* f, UINT count) = 0;
    virtual HRESULT SetFloatArray(CGFXHANDLE pName, const FLOAT* f, UINT count) = 0;

    virtual HRESULT GetInt(CGFXHANDLE pName, int *value) = 0;
    virtual HRESULT SetInt(CGFXHANDLE pName, int value) = 0;
    virtual HRESULT GetIntArray(CGFXHANDLE pName, int *value, UINT count) = 0;
    virtual HRESULT SetIntArray(CGFXHANDLE pName, const int *value, UINT count) = 0;

    virtual HRESULT SetVector(CGFXHANDLE pName, const float *pVector, UINT vecSize) = 0;
    virtual HRESULT GetVector(CGFXHANDLE pName, float *pVector, UINT vecSize) = 0;
    virtual HRESULT SetVectorArray(CGFXHANDLE pName, const float *pVector, UINT vecSize, 
                                   UINT count) = 0;
    virtual HRESULT GetVectorArray(CGFXHANDLE pName, float *pVector, UINT vecSize, UINT count) = 0;

    virtual HRESULT SetMatrix(CGFXHANDLE pName, const float* pMatrix, UINT nRows, UINT nCols) = 0;
    virtual HRESULT GetMatrix(CGFXHANDLE pName, float* pMatrix, UINT nRows, UINT nCols) = 0;
    virtual HRESULT SetMatrixArray(CGFXHANDLE pName, const float* pMatrix, UINT nRows, UINT nCols,
                                   UINT count) = 0;
    virtual HRESULT GetMatrixArray(CGFXHANDLE pName, float* pMatrix, UINT nRows, UINT nCols,
                                   UINT count) = 0;

    virtual HRESULT SetMatrixTranspose(CGFXHANDLE pName, const float* pMatrix, UINT nRows, UINT nCols) = 0;
    virtual HRESULT GetMatrixTranspose(CGFXHANDLE pName, float* pMatrix, UINT nRows, UINT nCols) = 0;
    virtual HRESULT SetMatrixTransposeArray(CGFXHANDLE pName, const float *pMatrix, UINT nRows, UINT nCols,
                                            UINT count) = 0;
    virtual HRESULT GetMatrixTransposeArray(CGFXHANDLE pName, float *pMatrix, UINT nRows, UINT nCols,
                                            UINT count) = 0;
	
    virtual HRESULT SetDWORD(CGFXHANDLE pName, DWORD dw) = 0;
    virtual HRESULT GetDWORD(CGFXHANDLE pName, DWORD* dw) = 0;

    virtual HRESULT SetBool(CGFXHANDLE pName, bool bvalue) = 0;
    virtual HRESULT GetBool(CGFXHANDLE pName, bool* bvalue) = 0;
    virtual HRESULT GetBoolArray(CGFXHANDLE pName, bool *bvalue, UINT count) = 0;
    virtual HRESULT SetBoolArray(CGFXHANDLE pName, const bool *bvalue, UINT count) = 0;
	
    virtual HRESULT SetString(CGFXHANDLE pName, LPCSTR pString) = 0;
    virtual HRESULT GetString(CGFXHANDLE pName, LPCSTR* ppString) = 0;

    virtual HRESULT SetTexture(CGFXHANDLE pName, DWORD textureHandle) = 0;
    virtual HRESULT GetTexture(CGFXHANDLE pName, DWORD* textureHandle) = 0;

    virtual HRESULT SetVertexShader(CGFXHANDLE pName, DWORD vsHandle) = 0;
    virtual HRESULT GetVertexShader(CGFXHANDLE pName, DWORD* vsHandle) = 0;

    virtual HRESULT SetPixelShader(CGFXHANDLE pName, DWORD psHandle) = 0;
    virtual HRESULT GetPixelShader(CGFXHANDLE pName, DWORD* psHandle) = 0;
};

enum CgFXPassFlags {
    CGFX_DONOTSAVESTATE = 1,
    CGFX_DONOTSAVESHADERSTATE = 2,
};

class ICgFXEffect : public ICgFXBaseEffect
{
public:
    virtual HRESULT SetTechnique(CGFXHANDLE pTechnique) = 0;
    virtual CGFXHANDLE GetCurrentTechnique() = 0;

    virtual HRESULT ValidateTechnique(CGFXHANDLE technique) = 0;
    virtual HRESULT FindNextValidTechnique(CGFXHANDLE hTechnique, CGFXHANDLE *pTechnique) = 0;

    virtual HRESULT Begin(UINT* pPasses, DWORD Flags) = 0;
    virtual HRESULT Pass(UINT passNum) = 0;
    virtual HRESULT End() = 0;

    virtual HRESULT CloneEffect(ICgFXEffect** ppNewEffect) = 0; 

    virtual HRESULT GetDevice(LPVOID* ppDevice) = 0;

    virtual HRESULT OnLostDevice() = 0;
    virtual HRESULT OnResetDevice() = 0;
};

class ICgFXEffectCompiler : public ICgFXBaseEffect
{
public:
    virtual HRESULT CompileEffect(const char **compilerArgs, ICgFXEffect** ppEffect, 
                                  const char** ppCompilationErrors) = 0;
};

extern "C" {

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXCreateEffect(
        LPCSTR               pSrcData,
        const char            **compilerArguments,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXCreateEffectFromFileA(
        LPCSTR               pSrcFile,
        const char            **compilerArguments,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXCreateEffectCompiler(
        LPCSTR                pSrcData,
        const char            **compilerArguments,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXCreateEffectCompilerFromFileA(
        LPCSTR                pSrcFile,
        const char            **compilerArguments,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXSetDevice(const char* pDeviceName, LPVOID pDevice);
HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXFreeDevice(const char* pDeviceName, LPVOID pDevice);

HRESULT CGFXDLL_EXPORT CGFXDLL_API CgFXGetErrors(const char** ppErrors);

} // extern "C"

#endif
