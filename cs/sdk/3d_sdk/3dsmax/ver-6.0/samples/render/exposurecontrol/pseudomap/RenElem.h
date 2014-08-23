/*==============================================================================

  file:	        RendElem.h

  author:       Daniel Levesque

  created:	    21 September 2001

  description:
        Render elements for rendering luminance and illuminance values.

        This file was originally written by Gordon Lueck for the lighting
    analysis exporter, and then I adapted it to fit the needs of the pseudo
    color exposure control.


  modified:	


© 2001 Autodesk
==============================================================================*/

#ifndef _RENELEM_H_
#define _RENELEM_H_

class LuminationRenderElement;
class IlluminationRenderElement;

#include <max.h>
#include <iparamb2.h>
#include "plugin.h"
#include "resource.h"
#include "LegendBitmap.h"

extern ClassDesc2* GetLuminationClassDesc();
extern ClassDesc2* GetIlluminationClassDesc();

//Class ID for this Render Element
#define LUM_CLASS_ID	Class_ID(0x5e1b2ec6, 0x74f96dff)

#define LUM_PARAM_BLOCK		BlockID			(0)
#define LUM_PARAM_REF_NO					0


class LuminationRenderElement: public MaxRenderElement 
{
private:
    
public:
    
    // Parameters
    IParamBlock2 *pParamBlk;
    
    LuminationRenderElement();
    RefTargetHandle Clone( RemapDir &remap );
    void DeleteThis() { delete this; };
    
    // Animatable/Reference
    int NumSubs() { return 1; }
    Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
    TSTR SubAnimName(int i)
    { return i? _T("") : _T(GetString(IDS_LUMINATION_PARAMS)); }
    
    int NumRefs() { return 1;};
    RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
    void SetReference(int i, RefTargetHandle rtarg)
    { if ( i == 0 ) pParamBlk = (IParamBlock2*)rtarg; }
    
    Class_ID ClassID() {return LUM_CLASS_ID;};
    TSTR GetName() { return GetString( IDS_QUANTITY_LUMINANCE ); }
    void GetClassName(TSTR& s) { s = GetName(); }
    
    int	NumParamBlocks() { return 1; }
    IParamBlock2* GetParamBlock(int i) { return pParamBlk; } // only one
    IParamBlock2* GetParamBlockByID(BlockID id) { return (pParamBlk->ID() == id) ? pParamBlk : NULL; }
    
    // enable & disable the render element
    void SetEnabled(BOOL enabled);
    BOOL IsEnabled() const;
    
    // set/get element's filter enabled state
    void SetFilterEnabled(BOOL filterEnabled);
    BOOL IsFilterEnabled() const;
    
    BOOL BlendOnMultipass() const { return TRUE; }
    
    // set/get whether to apply atmosphere
    void SetApplyAtmosphere(BOOL applyAtmosphere);
    BOOL AtmosphereApplied() const;
    
    void SetApplyShadows(BOOL applyShadows);
    BOOL ShadowsApplied() const;
    
    // set/get element's name (as it will appear in render dialog)
    void SetElementName( TCHAR* newName); 
    const TCHAR* ElementName() const;
    
    /*
    // set/get path name for file output
    void SetPathName( TCHAR* newPathName);
    const TCHAR* PathName() const;
    
      void SetBitmap( Bitmap* bitmap);
      Bitmap* GetBitmap() const;
    */
    void SetPBBitmap(PBBitmap* &pPBBitmap) const;
    void GetPBBitmap(PBBitmap* &pPBBitmap) const;
    
    // this is the element specific optional UI, which is a rollup in the render dialog
    IRenderElementParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }
    
    // Implement this if you are using the ParamMap2 AUTO_UI system and the 
    // IRenderElement has secondary dialogs that don't have the IRenderElement as their 'thing'.
    // Called once for each secondary dialog, for you to install the correct thing.
    // Return TRUE if you process the dialog, false otherwise.
    BOOL SetDlgThing(IRenderElementParamDlg* dlg) { return FALSE; }
    
    // ---------------------
    // from class RefMaker
    // ---------------------
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
    
    // the compute functions
    void PostIllum(ShadeContext& sc, IllumParams& ip);
    
    // called after atmospheres are computed, to allow elements to handle atmospheres
    void PostAtmosphere(ShadeContext& sc, float z, float zPrev){};

protected:
    CLegendBitmap m_LegendBitmap;
};


/************************************************************************
/*	
/*	Illumination version of the same
/*	
/************************************************************************/

#define ILLUM_CLASS_ID	Class_ID(0x69f45cf8, 0x7f47324f)

#define ILLUM_PARAM_BLOCK		BlockID			(0)
#define ILLUM_PARAM_REF_NO					0

class IlluminationRenderElement: public MaxRenderElement 
{
private:
public:

    // Parameters
    IParamBlock2 *pParamBlk;
    
    IlluminationRenderElement();
    RefTargetHandle Clone( RemapDir &remap );
    void DeleteThis() { delete this; };
    
    // Animatable/Reference
    int NumSubs() { return 1; }
    Animatable* SubAnim(int i){ return i? NULL : pParamBlk; }
    TSTR SubAnimName(int i)
    { return i? _T("") : _T(GetString(IDS_ILLUMINATION_PARAMS)); }
    
    int NumRefs() { return 1;};
    RefTargetHandle GetReference(int i){ return i? NULL : pParamBlk; }
    void SetReference(int i, RefTargetHandle rtarg)
    { if ( i == 0 ) pParamBlk = (IParamBlock2*)rtarg; }
    
    Class_ID ClassID() {return ILLUM_CLASS_ID;};
    TSTR GetName() { return GetString( IDS_QUANTITY_ILLUMINANCE ); }
    void GetClassName(TSTR& s) { s = GetName(); }
    
    int	NumParamBlocks() { return 1; }
    IParamBlock2* GetParamBlock(int i) { return pParamBlk; } // only one
    IParamBlock2* GetParamBlockByID(BlockID id) { return (pParamBlk->ID() == id) ? pParamBlk : NULL; }
    
    // enable & disable the render element
    void SetEnabled(BOOL enabled);
    BOOL IsEnabled() const;
    
    // set/get element's filter enabled state
    void SetFilterEnabled(BOOL filterEnabled);
    BOOL IsFilterEnabled() const;
    
    BOOL BlendOnMultipass() const { return TRUE; }
    
    // set/get whether to apply atmosphere
    void SetApplyAtmosphere(BOOL applyAtmosphere);
    BOOL AtmosphereApplied() const;
    
    void SetApplyShadows(BOOL applyShadows);
    BOOL ShadowsApplied() const;
    
    // set/get element's name (as it will appear in render dialog)
    void SetElementName( TCHAR* newName); 
    const TCHAR* ElementName() const;
    
    /*
    // set/get path name for file output
    void SetPathName( TCHAR* newPathName);
    const TCHAR* PathName() const;
    
      void SetBitmap( Bitmap* bitmap);
      Bitmap* GetBitmap() const;
    */
    void SetPBBitmap(PBBitmap* &pPBBitmap) const;
    void GetPBBitmap(PBBitmap* &pPBBitmap) const;
    
    // this is the element specific optional UI, which is a rollup in the render dialog
    IRenderElementParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }
    
    // Implement this if you are using the ParamMap2 AUTO_UI system and the 
    // IRenderElement has secondary dialogs that don't have the IRenderElement as their 'thing'.
    // Called once for each secondary dialog, for you to install the correct thing.
    // Return TRUE if you process the dialog, false otherwise.
    BOOL SetDlgThing(IRenderElementParamDlg* dlg) { return FALSE; }
    
    // ---------------------
    // from class RefMaker
    // ---------------------
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
    
    // the compute functions
    void PostIllum(ShadeContext& sc, IllumParams& ip);
    
    // called after atmospheres are computed, to allow elements to handle atmospheres
    void PostAtmosphere(ShadeContext& sc, float z, float zPrev){};

protected:
    CLegendBitmap m_LegendBitmap;
};

#endif //ndef __RENELEM_H