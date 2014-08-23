/*==============================================================================

  file:	        RendElem.cpp

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

#include "RenElem.h"
#include "resource.h"
#include <toneop.h>
#include <hsv.h>

static const double M_PI = 3.14159265358979323846;

//same as in radiositysolution.h
#define DEFAULT_MAX_TO_PHYSICAL_LIGHTING_SCALE  1500

static const float PERCENTAGE_TO_KEEP = 0.9f;

class LuminationClassDesc:public ClassDesc2
{
public:
    int 			IsPublic() { return FALSE; }//This render element doesn't work well with non-LogL images.
    //This way we disable it so that the user cannot see it - to get unexpected results.
    void *			Create(BOOL loading) { return new LuminationRenderElement; }
    const TCHAR *	ClassName() { return GetString(IDS_LUMINATION_RE); }
    SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
    Class_ID 		ClassID() { return LUM_CLASS_ID; }
    const TCHAR* 	Category() { return _T(""); }
    const TCHAR*	InternalName() { return _T("lumRenderElement"); } // hard-coded for scripter
    HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static LuminationClassDesc lumCD;
ClassDesc2* GetLuminationClassDesc() { return &lumCD; }

enum { enableOn,
filterOn,
atmosphereOn,
shadowOn,
eleName,
pbBitmap };

static ParamBlockDesc2 lumParamBlk(LUM_PARAM_BLOCK, 
                                   _T("lumREParameters"), 
                                   IDS_LUMINATION_RE, 
                                   &lumCD, 
                                   P_AUTO_CONSTRUCT, 
                                   LUM_PARAM_REF_NO,
                                   enableOn, _T("enabledOn"), TYPE_BOOL, 0, IDS_ENABLED,
                                   p_default, TRUE,
                                   end,
                                   filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
                                   p_default, TRUE,
                                   end,
                                   atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
                                   p_default, TRUE,
                                   end,
                                   shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
                                   p_default, TRUE,
                                   end,
                                   eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
                                   p_default,"",
                                   end,
                                   pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
                                   end,
                                   end);


LuminationRenderElement::LuminationRenderElement() {
    lumCD.MakeAutoParamBlocks(this);
    DbgAssert(pParamBlk);
    SetElementName( GetName());
}


RefTargetHandle LuminationRenderElement::Clone( RemapDir &remap )
{
    LuminationRenderElement*	mnew = new LuminationRenderElement();
    mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
    return (RefTargetHandle)mnew;
}

RefResult LuminationRenderElement::NotifyRefChanged(
                                                    Interval changeInt, RefTargetHandle hTarget,
                                                    PartID& partID,  RefMessage message) 
{
    return REF_SUCCEED;
}

void LuminationRenderElement::SetEnabled( BOOL on )
{
    pParamBlk->SetValue( enableOn, 0, on );
}

BOOL LuminationRenderElement::IsEnabled() const
{
    int	on;
    pParamBlk->GetValue( enableOn, 0, on, FOREVER );
    return on;
}

void LuminationRenderElement::SetFilterEnabled( BOOL on )
{
    pParamBlk->SetValue( filterOn, 0, on );
}

BOOL LuminationRenderElement::IsFilterEnabled() const
{
    int	on;
    pParamBlk->GetValue( filterOn, 0, on, FOREVER );
    return on;
}

void LuminationRenderElement::SetApplyAtmosphere( BOOL on )
{
    pParamBlk->SetValue( atmosphereOn, 0, on );
}

BOOL LuminationRenderElement::AtmosphereApplied() const
{
    int	on;
    pParamBlk->GetValue( atmosphereOn, 0, on, FOREVER );
    return on;
}

void LuminationRenderElement::SetApplyShadows( BOOL on )
{
    pParamBlk->SetValue( shadowOn, 0, on );
}

BOOL LuminationRenderElement::ShadowsApplied() const
{
    int	on;
    pParamBlk->GetValue( shadowOn, 0, on, FOREVER );
    return on;
}


const TCHAR* LuminationRenderElement::ElementName() const
{
    TCHAR* pStr = NULL;
    pParamBlk->GetValue( eleName, 0, pStr, FOREVER );
    return pStr;
}


void LuminationRenderElement::SetElementName( TCHAR* pStr ) 
{
    pParamBlk->SetValue( eleName, 0, pStr );
}

void LuminationRenderElement::SetPBBitmap(PBBitmap* &pPBBitmap) const
{
    pParamBlk->SetValue( pbBitmap, 0, pPBBitmap);
}

void LuminationRenderElement::GetPBBitmap(PBBitmap* &pPBBitmap) const
{
    pParamBlk->GetValue( pbBitmap, 0, pPBBitmap, FOREVER );
}

void LuminationRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
    //not corrected for human eye sensitivity
    Color lum = ip.finalC;//this is the colour that hits my eye luminance.

    if( (sc.ScreenCoord().y  == 0) && (sc.ScreenCoord().x  == 0))
    {
        m_LegendBitmap.BuildLegend(sc.globContext->devWidth, (sc.globContext->devHeight - (int)(sc.globContext->devHeight * PERCENTAGE_TO_KEEP)));   
    }
   
    if(sc.ScreenCoord().y < (sc.globContext->devHeight * PERCENTAGE_TO_KEEP))
    {
        // Scale color to RGB using the tone operator
        if((sc.globContext != NULL) && (sc.globContext->pToneOp != NULL))
            sc.ScaledToRGB(lum);
    }
    else
    {
        int iBitmapLegendY = sc.ScreenCoord().y - (sc.globContext->devHeight * PERCENTAGE_TO_KEEP);
        lum = m_LegendBitmap.GetPixelColor(sc.ScreenCoord().x, iBitmapLegendY);
    }
  
    sc.out.elementVals[ mShadeOutputIndex ] = lum;
}


/************************************************************************
/*	
/*	The Illumination Render Element Stuff.
/*	
/************************************************************************/

class IlluminationClassDesc:public ClassDesc2
{
public:
    int 			IsPublic() { return FALSE; }//This render element doesn't work well with non-LogL images.
    //This way we disable it so that the user cannot see it - to get unexpected results.
    void *			Create(BOOL loading) { return new IlluminationRenderElement; }
    const TCHAR *	ClassName() { return GetString(IDS_ILLUMINATION_RE); }
    SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
    Class_ID 		ClassID() { return ILLUM_CLASS_ID; }
    const TCHAR* 	Category() { return _T(""); }
    const TCHAR*	InternalName() { return _T("illumRenderElement"); } // hard-coded for scripter
    HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static IlluminationClassDesc illumCD;
ClassDesc2* GetIlluminationClassDesc() { return &illumCD; }

static ParamBlockDesc2 illumParamBlk(ILLUM_PARAM_BLOCK, 
                                     _T("illumREParameters"), 
                                     IDS_ILLUMINATION_RE, 
                                     &illumCD, 
                                     P_AUTO_CONSTRUCT, 
                                     ILLUM_PARAM_REF_NO,
                                     enableOn, _T("enabledOn"), TYPE_BOOL, 0, IDS_ENABLED,
                                     p_default, TRUE,
                                     end,
                                     filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
                                     p_default, TRUE,
                                     end,
                                     atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
                                     p_default, TRUE,
                                     end,
                                     shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
                                     p_default, TRUE,
                                     end,
                                     eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
                                     p_default,"",
                                     end,
                                     pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
                                     end,
                                     end);


IlluminationRenderElement::IlluminationRenderElement() {
    illumCD.MakeAutoParamBlocks(this);
    DbgAssert(pParamBlk);
    SetElementName( GetName());
}


RefTargetHandle IlluminationRenderElement::Clone( RemapDir &remap )
{
    IlluminationRenderElement*	mnew = new IlluminationRenderElement();
    mnew->ReplaceReference(0,remap.CloneRef(pParamBlk));
    return (RefTargetHandle)mnew;
}

RefResult IlluminationRenderElement::NotifyRefChanged(
                                                      Interval changeInt, RefTargetHandle hTarget,
                                                      PartID& partID,  RefMessage message) 
{
    return REF_SUCCEED;
}

void IlluminationRenderElement::SetEnabled( BOOL on )
{
    pParamBlk->SetValue( enableOn, 0, on );
}

BOOL IlluminationRenderElement::IsEnabled() const
{
    int	on;
    pParamBlk->GetValue( enableOn, 0, on, FOREVER );
    return on;
}

void IlluminationRenderElement::SetFilterEnabled( BOOL on )
{
    pParamBlk->SetValue( filterOn, 0, on );
}

BOOL IlluminationRenderElement::IsFilterEnabled() const
{
    int	on;
    pParamBlk->GetValue( filterOn, 0, on, FOREVER );
    return on;
}

void IlluminationRenderElement::SetApplyAtmosphere( BOOL on )
{
    pParamBlk->SetValue( atmosphereOn, 0, on );
}

BOOL IlluminationRenderElement::AtmosphereApplied() const
{
    int	on;
    pParamBlk->GetValue( atmosphereOn, 0, on, FOREVER );
    return on;
}

void IlluminationRenderElement::SetApplyShadows( BOOL on )
{
    pParamBlk->SetValue( shadowOn, 0, on );
}

BOOL IlluminationRenderElement::ShadowsApplied() const
{
    int	on;
    pParamBlk->GetValue( shadowOn, 0, on, FOREVER );
    return on;
}


const TCHAR* IlluminationRenderElement::ElementName() const
{
    TCHAR* pStr = NULL;
    pParamBlk->GetValue( eleName, 0, pStr, FOREVER );
    return pStr;
}


void IlluminationRenderElement::SetElementName( TCHAR* pStr ) 
{
    pParamBlk->SetValue( eleName, 0, pStr );
}

void IlluminationRenderElement::SetPBBitmap(PBBitmap* &pPBBitmap) const
{
    pParamBlk->SetValue( pbBitmap, 0, pPBBitmap);
}

void IlluminationRenderElement::GetPBBitmap(PBBitmap* &pPBBitmap) const
{
    pParamBlk->GetValue( pbBitmap, 0, pPBBitmap, FOREVER );
}

void IlluminationRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
    if( (sc.ScreenCoord().y  == 0) && (sc.ScreenCoord().x  == 0))
    {         
        m_LegendBitmap.BuildLegend(sc.globContext->devWidth, (sc.globContext->devHeight - (int)(sc.globContext->devHeight * PERCENTAGE_TO_KEEP)));   
    }
        
    Color illum = Color(0,0,0);
  
    if(sc.ScreenCoord().y < (sc.globContext->devHeight * PERCENTAGE_TO_KEEP))
    {

        // [dl | 1oct2001] Ignore background. (pMtl == NULL) => processing background.
        // This was causing a problem when this function was called on the background. The contents
        // of the ShadeContext (INode* and location) were still indicating the last location where
        // an actual hit took place. These cases need to be ignored or the color from the object would
        // leak to the background.
        if(ip.pMtl != NULL) {
            Color illumIncr;
            LightDesc* l;
            Point3 dummyDir;
            float dotnl;
            float dummyDiffuseCoef;
            for (int i=0; i<sc.nLights; i++) 
            {
                l = sc.Light(i);
                DbgAssert(l);
                if (l->Illuminate(sc, sc.Normal(), illumIncr, dummyDir,
					    dotnl, dummyDiffuseCoef)) {
				    // Don't add in illumIncr if the light doesn't illuminate the point.
				    if ( !l->ambientOnly ) {
					    if ( !l->affectDiffuse )
						    continue;
					    illumIncr *= dotnl;
				    }
				    illum += illumIncr;
			    }
            }
        } 

        // Scale color to RGB using the tone operator
        if((sc.globContext != NULL) && (sc.globContext->pToneOp != NULL))
            sc.ScaledToRGB(illum);
    }
    else
    {
        int iBitmapLegendY = sc.ScreenCoord().y - (sc.globContext->devHeight * PERCENTAGE_TO_KEEP);
        illum = m_LegendBitmap.GetPixelColor(sc.ScreenCoord().x, iBitmapLegendY);
    }
    
    sc.out.elementVals[ mShadeOutputIndex ] = illum; //illuminance.
} 
