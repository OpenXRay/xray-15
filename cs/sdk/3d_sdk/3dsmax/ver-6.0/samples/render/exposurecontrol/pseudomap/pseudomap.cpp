//*****************************************************************************/
// Copyright 2000 Autodesk, Inc.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc. and
// are protected by Federal copyright law. They may not be disclosed
// to third parties or copied or duplicated in any form, in whole or
// in part, without the prior written consent of Autodesk, Inc.
//*****************************************************************************/

#include <max.h>
#include "pseudoMap.h"
#include <iparamb2.h>
#include "resource.h"
#include <iparamm2.h>
#include <notify.h>
#include <hsv.h>
#include <float.h>
#include <units.h>
#include "plugin.h"
#include "RenElem.h"
 
#include <ILightingUnits.h>

#define PSEUDOEXPOSURECONTROLDIALOG_CLASSID Class_ID(0x64307bf0, 0x29934cb3)

class PseudoExposureControlDialog;

//==============================================================================
// class PseudoExposureControlClassDesc
//==============================================================================

class PseudoExposureControlClassDesc : public ClassDesc2 {
public:

    // -- from ClassDesc2
    int IsPublic() { return true; }
    void* Create(BOOL loading = FALSE);
    const TCHAR* ClassName() { return GetString(IDS_PSEUDO_EXP_CTL_CLASS_NAME); }
    SClass_ID SuperClassID() { return TONE_OPERATOR_CLASS_ID; }
    Class_ID ClassID() { return PSEUDO_EXP_CTL_CLASS_ID; }
    const TCHAR* Category() { return _T(""); }
    const TCHAR* InternalName() { return _T("pseudoColorExposureControl"); }
    HINSTANCE HInstance() { return hInstance; }
};


//==============================================================================
// class PseudoExposureControl
//
// This is the implementation of the tone operator plugin.
//==============================================================================
class PseudoExposureControl : public PseudoMap {

    class ToneOpChangeHandler;
    friend ToneOpChangeHandler;

public:

    virtual RefResult NotifyRefChanged(
        Interval          changeInt,
        RefTargetHandle   hTarget,
        PartID&           partID,
        RefMessage        message
        );
    
    static PseudoExposureControlDialog* GetDialog() { return _dialog; }
    static void SetDialog(PseudoExposureControlDialog* dlg) { _dialog = dlg; }
    
    PseudoExposureControl(BOOL loading);
    virtual ~PseudoExposureControl();

    IParamBlock2* getParams() { return _params; } 

    // Get the energy interval that is mapped to the specified color index.
    // This is an index in the _pseudoColorMap[] array.
    virtual void GetEnergyRangeFromIndex(int colorIndex, float& floor, float& ceiling);

   
    // -- from IToneOperatorExtension
    virtual Quantity GetUsedQuantity() const { return _quantity; }
    virtual void SetUsedQuantity(Quantity q) { _params->SetValue(PB_QUANTITY, 0, static_cast<int>(q)); }

    // -- from InterfaceServer
    virtual BaseInterface* GetInterface(Interface_ID id);

    // -- from ToneOperator
    virtual ToneOpParamDlg *CreateParamDialog(IRendParams *ip);
    virtual void ScaledToRGB(float energy[3]);
    virtual float ScaledToRGB(float energy);
    
    virtual float GetPhysicalUnit(
        TimeValue   t,
        Interval&   valid = Interval(0,0)
        ) const;
    virtual void SetPhysicalUnit(
        float    value,
        TimeValue   t
        );

    virtual void SetActive(bool	active,TimeValue t);
    
    virtual void ScalePhysical(float energy[3]) const;
    virtual float ScalePhysical(float energy) const;
    virtual void ScaleRGB(float color[3]) const;
    virtual float ScaleRGB(float color) const;
    
    virtual TSTR GetName() { return name; }		
    
    virtual void Update(TimeValue t, Interval& valid);
    virtual RefTargetHandle Clone(RemapDir &remap = NoRemap());
    void BaseClone(
        ReferenceTarget*  from,
        ReferenceTarget*  to,
        RemapDir&         remap
        );

    virtual int NumRefs();
    virtual RefTargetHandle GetReference(int i);
    virtual void SetReference(int i, RefTargetHandle rtarg);
    virtual void GetClassName(TSTR& s);
    virtual Class_ID ClassID();
    virtual SClass_ID SuperClassID();
    virtual void DeleteThis();
    
    virtual int NumSubs();
    virtual Animatable* SubAnim(int i);
    virtual TSTR SubAnimName(int i);
    virtual int SubNumToRefNum(int subNum);
    
    virtual int	NumParamBlocks();                     // return number of ParamBlocks in this instance
    virtual IParamBlock2* GetParamBlock(int i);        // return i'th ParamBlock
    virtual IParamBlock2* GetParamBlockByID(short id); // return ParamBlock given ID
    
    virtual void SetMaximum(
        TimeValue   t,
        float    max
        );
    virtual float GetMaximum(
        TimeValue   t,
        Interval&   valid
        ) const;
    
    virtual void SetMinimum(
        TimeValue   t,
        float min
        );
    virtual float GetMinimum(
        TimeValue   t,
        Interval&   valid
        ) const;
    
    virtual void          SetDisplay        ( Display dsp );
    virtual Display       GetDisplay        ( ) const;
    virtual void          SetAutomatic      ( bool on );
    virtual bool          GetAutomatic      ( ) const;     
    virtual void          GetPseudoColorMap (ULONG * pPseudoColorMap, int iSizeOf);

    virtual bool BuildMaps(TimeValue t, RenderMapsContext& rmc);   
    virtual void SubRenderSample(float energy[3]);
       
private:
  
    void buildColorTable();
    bool renderMap(TimeValue t, RenderMapsContext& rmc);

    // Sets the current render element depending on the Quantity (luminance or illuminance)
    void SetCurrentRenderElement(bool setToNone = false);

    // Enables/Disables the render elements
    void EnableRenderElement(bool enable);

private:
   
    class ToneOpChangeHandler {
    public:
        ToneOpChangeHandler();
        ~ToneOpChangeHandler();

        // Notification callback for tone operator changes
        static void ToneOpChanged(
		    ToneOperator*  newOp,
		    ToneOperator*  oldOp,
		    void*          param
	    );
    };


    enum ChunkIDs {
        kChunkID_RenderElement = 0x0001
    };

private:

    /////////////////////
    // Constants
    //
    static const float COLOR_TABLE_SATURATION       ;
    static const float COLOR_TABLE_VALUE            ;
    static const float COLOR_TABLE_START_HUE        ;  
    static const float COLOR_TABLE_HUE_RANGE        ;    
    static const float COLOR_TABLE_NUMBER_OF_GRAY   ;

    static PseudoExposureControlDialog* _dialog;
    static ToneOpChangeHandler toneOpChangeHandler;
    static ILightingUnits* lightSystem;

    bool _active;
    bool _samplingRender;

    ScaleFunction _scaleFunction;
    IToneOperatorExtension::Quantity _quantity;
    
    float _physScale;
    float _conversionFactor;
    float _cfB;
    float _cfA;
    float _maxBrightness;
    float _minBrightness;
    
    IParamBlock2*    _params;
   
    MaxRenderElement* _currentRenderElement;

    unsigned long _pseudoColorMap[NB_MAX_PSEUDO_COLORS];

};

PseudoExposureControlDialog* PseudoExposureControl::_dialog = NULL;
PseudoExposureControl::ToneOpChangeHandler PseudoExposureControl::toneOpChangeHandler;


//==============================================================================
// class PseudoExposureControlDialog
//
// Handles the creation and deletion of the pseudo color esposure control's
// rollout
//==============================================================================
class PseudoExposureControlDialog : public ToneOpParamDlg {

public:

    // The constructor does not initialize anything. Call Initialize() right after
    // calling the constructor. This scheme makes sure the object is built and a
    // pointer to it can be retrieved before the rollout is create by Initialize()
    PseudoExposureControlDialog();
    ~PseudoExposureControlDialog();

    void Initialize(PseudoExposureControl* exposureControl, IRendParams* rendParam);

    BOOL DialogProc(TimeValue t, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Updates the bitmap used in the legend
    void UpdateLegendBitmap(unsigned long* pseudoColorMap);

    // Updates the values under the legend
    void UpdateLegendValues();

    
    // -- from ToneOpParamDlg
    Class_ID ClassID() { return PSEUDOEXPOSURECONTROLDIALOG_CLASSID; }
    ReferenceTarget* GetThing() { return mp_exposureControl; }
    void SetThing(ReferenceTarget* m);
    void DeleteThis() { delete this; }
    
private:

    // Size of the bitmap fo the legend
    int m_legendWidth;
    int m_legendHeight;

    PseudoExposureControl* mp_exposureControl;
    IRendParams* mp_rendParam;
    IParamMap2* mp_mainParamMap;
    ICustImage* mp_colorLegendControl;
    HIMAGELIST m_hImageList;    // handle to the list of images used with the color legend

    HWND m_hWnd;
};


//==============================================================================
// class MainPBDialogProc
//
// Custom dialog procedure for the main param block rollout. Necessary because
// of the combo boxes in the UI, no supported by PB2
//==============================================================================
class MainPBDialogProc : public ParamMap2UserDlgProc {

public:

    static MainPBDialogProc* GetInstance() { return &m_theInstance; }
    // notification callback for lighting unit system changes
    static void LightingUnitsChanged(void *param, NotifyInfo *info);

    MainPBDialogProc();
    ~MainPBDialogProc();

    // -- from ParamMap2UserDlgProc
    virtual BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void SetThing(ReferenceTarget *m) { exposureControl = static_cast<PseudoExposureControl*>(m); }
    void DeleteThis() { exposureControl = NULL; }
    virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap);
    
private:

    static MainPBDialogProc m_theInstance;

    PseudoExposureControl* exposureControl;
    ILightingUnits* lightSystem;
};

MainPBDialogProc MainPBDialogProc::m_theInstance;


//==============================================================================
// class QuantityValidator
//
// Validator for the quantity property of the main param block.
//==============================================================================
class MainPBValidator : public PBValidator {

public:

    static MainPBValidator* GetInstance() { return &m_theInstance; }

    // -- from PBValidator
    virtual BOOL Validate(PB2Value& v) { return true; }
    virtual BOOL Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex);

private:

    static MainPBValidator m_theInstance;
};

MainPBValidator MainPBValidator::m_theInstance;

class FlagAccessor : public PBAccessor {
public:

   virtual void Get(
      PB2Value&         v,
      ReferenceMaker*   owner,
      ParamID           id,
      int               tabIndex,
      TimeValue         t,
      Interval&         valid
   ) {
      switch (id) {
         case PseudoExposureControl::PB_ACTIVE: {
            v.i = static_cast<PseudoExposureControl*>(owner)->Active(t);
         } break;

         case PseudoExposureControl::PB_PROC_BG: {
            v.i = static_cast<PseudoExposureControl*>(owner)->GetProcessBackground();
         } break;
      }
   }

   virtual void Set(
      PB2Value&         v,
      ReferenceMaker*   owner,
      ParamID           id,
      int               tabIndex,
      TimeValue         t
   ) {
      switch (id) {
         case PseudoExposureControl::PB_ACTIVE: {
            static_cast<PseudoExposureControl*>(owner)->SetActive(v.i != 0, t);
         } break;

         case PseudoExposureControl::PB_PROC_BG: {
            static_cast<PseudoExposureControl*>(owner)->SetProcessBackground(v.i != 0);
         } break;
      }
   }
};

FlagAccessor flagAccessor;

//==============================================================================
// PB2 Descriptor for main parameter block
//==============================================================================
ParamBlockDesc2 mainPBDesc2(
    PseudoExposureControl::PB_MAIN, _T("Pseudo color Radiance Map Parameters"), IDS_PSEUDO_EXP_CTL_PARAMETERS,
    PseudoMap::GetClassDesc(), P_AUTO_CONSTRUCT | P_AUTO_UI,

    // Auto Construct reference number
    PseudoExposureControl::REF_PARAMS,

    // Rollout
    IDD_PSEUDOMAP_MAINDIALOG, IDS_PSEUDO_EXP_CTL_TITLE, 0, 0, MainPBDialogProc::GetInstance(),

    // Parameters
        // Minimum
        PseudoExposureControl::PB_MINIMUM, _T("minimum"), TYPE_FLOAT, P_ANIMATABLE, IDS_MINIMUM,
            p_default, 0.0f,
            p_range, 0.0f, 200000.0f,
            p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_MAIN_MINIMUM_EDIT, IDC_MAIN_MINIMUM_SPINNER, SPIN_AUTOSCALE,
        end,

        // Maximum
        PseudoExposureControl::PB_MAXIMUM, _T("maximum"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAXIMUM,
            p_default, 50.0f,
            p_range, 0.0f, 200000.0f,
            p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_MAIN_MAXIMUM_EDIT, IDC_MAIN_MAXIMUM_SPINNER, SPIN_AUTOSCALE,
        end,

        // Quantity
        PseudoExposureControl::PB_QUANTITY, _T("quantity"), TYPE_INT, 0, IDS_QUANTITY,
        p_default, IToneOperatorExtension::kQuantity_Illuminance,
            p_validator, MainPBValidator::GetInstance(),
        end,

        // Display
        PseudoExposureControl::PB_DISPLAY, _T("display"), TYPE_INT, 0, IDS_DISPLAY,
            p_default, PseudoExposureControl::DSP_COLORED,
            p_range, PseudoExposureControl::DSP_COLORED, PseudoExposureControl::DSP_GRAY,
        end,

        // Physical Scale
        PseudoExposureControl::PB_SCALE, _T("physicalScale"), TYPE_FLOAT, P_ANIMATABLE, IDS_SCALE,
            p_default, 1500.0f,
            p_range, 0.001f, 200000.0f,
            p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_PHYSICALSCALE_EDIT, IDS_PHYSICALSCALE_SPINNER, SPIN_AUTOSCALE,
        end,

        // Automatic
        PseudoExposureControl::PB_AUTOMATIC, _T("automatic"), TYPE_BOOL, 0, IDS_AUTOMATIC,
            p_default, false,
            p_ui, TYPE_SINGLECHEKBOX, IDC_MAIN_AUTOMATIC,
        end,

        // Scale function
        PseudoExposureControl::PB_SCALE_FUNC, _T("scaleFunction"), TYPE_INT, 0, IDS_SCALE_FUNC,
            p_default, PseudoExposureControl::SCALE_LOGARITHMIC,
            p_validator, MainPBValidator::GetInstance(),
        end,

        // Unit system in use (used for converting min/max when unit system changes)
        PseudoExposureControl::PB_UNITSYSTEM_USED, _T("unitSystemUsed"), TYPE_INT, 0, IDS_UNITSYSTEM_USED,
        // I've just set the default to zero (SI), since we can't assume that the ID for the 
        // lighting system will exist.
            p_default, ILightingUnits::DISPLAY_INTERNATIONAL,
            p_validator, MainPBValidator::GetInstance(),
        end,

	   PseudoExposureControl::PB_ACTIVE, _T("active"), TYPE_BOOL, P_TRANSIENT, IDS_ACTIVE,
		  p_accessor, &flagAccessor,
		  end,

	   PseudoExposureControl::PB_PROC_BG, _T("processBG"), TYPE_BOOL, P_TRANSIENT, IDS_PROC_BG,
		  p_accessor, &flagAccessor,
		  end,

    end
);


//==============================================================================
// class PseudoMap
//==============================================================================

ClassDesc2* PseudoMap::GetClassDesc() {
    static PseudoExposureControlClassDesc theClassDesc;

    return &theClassDesc;
}


//==============================================================================
// class PseudoExposureControlClassDesc
//==============================================================================

void* PseudoExposureControlClassDesc::Create(BOOL loading) { 
    return new PseudoExposureControl(loading); 
}


//==============================================================================
// CLASS ToneOpChangeHandler
//==============================================================================

PseudoExposureControl::ToneOpChangeHandler::ToneOpChangeHandler() {

    ToneOperatorInterface* toneOpInterface = 
        static_cast<ToneOperatorInterface*>(GetCOREInterface(TONE_OPERATOR_INTERFACE));
    if(toneOpInterface != NULL)
        toneOpInterface->RegisterToneOperatorChangeNotification(ToneOpChanged, this);

}

PseudoExposureControl::ToneOpChangeHandler::~ToneOpChangeHandler() {

    ToneOperatorInterface* toneOpInterface = 
        static_cast<ToneOperatorInterface*>(GetCOREInterface(TONE_OPERATOR_INTERFACE));
    if(toneOpInterface != NULL)
        toneOpInterface->UnRegisterToneOperatorChangeNotification(ToneOpChanged, this);

}

void PseudoExposureControl::ToneOpChangeHandler::ToneOpChanged(
	ToneOperator*  newOp,
	ToneOperator*  oldOp,
	void*          param
)
{
    // Remove the render element of the old tone operator if it is a pseudo color toneop
    // This ensures that we do not get several instances of the render element
    if((oldOp != NULL) && (oldOp->ClassID() == PSEUDO_EXP_CTL_CLASS_ID)) {

        PseudoExposureControl* pseudoToneOp = 
            static_cast<PseudoExposureControl*>(oldOp);

        pseudoToneOp->SetCurrentRenderElement(true);
    }

    // Add the render element of the new tone operator if it is a pseudo color toneop
    // This ensures that we do not get several instances of the render element
    if((newOp != NULL) && (newOp->ClassID() == PSEUDO_EXP_CTL_CLASS_ID)) {

        PseudoExposureControl* pseudoToneOp = 
            static_cast<PseudoExposureControl*>(newOp);

        pseudoToneOp->SetCurrentRenderElement();
    }
}


//==============================================================================
// CLASS PseudoExposureControl
//==============================================================================

ILightingUnits* PseudoExposureControl::lightSystem = static_cast<ILightingUnits*>
                                                        (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));

const float PseudoExposureControl::COLOR_TABLE_SATURATION      = 255.0f;
const float PseudoExposureControl::COLOR_TABLE_VALUE           = 255.0f;
const float PseudoExposureControl::COLOR_TABLE_START_HUE       = 170.0f;  
const float PseudoExposureControl::COLOR_TABLE_HUE_RANGE       = 170.0f;    
const float PseudoExposureControl::COLOR_TABLE_NUMBER_OF_GRAY  = 256.0f;

PseudoExposureControl::PseudoExposureControl(BOOL loading)
: _params( NULL ),
  PseudoMap(),
  _currentRenderElement(NULL)
{
    if(!loading) {
        PseudoMap::GetClassDesc()->MakeAutoParamBlocks(this);
        Update(GetCOREInterface()->GetTime(), Interval(0,0));
    }
}

PseudoExposureControl::~PseudoExposureControl()
{
    DeleteAllRefsFromMe();
    _params = NULL;
}

RefResult PseudoExposureControl::NotifyRefChanged(
    Interval          changeInt,
    RefTargetHandle   hTarget,
    PartID&           partID,
    RefMessage        message
)
{
    switch(message) {
    case REFMSG_CHANGE: 
        if (hTarget == _params) {          
            IParamMap2* map = _params->GetMap();
            if (map != NULL)
                map->Invalidate();
            
            Interval v;
			int tabindex;
            Update(GetCOREInterface()->GetTime(), v);
			if (_params->LastNotifyParamID(tabindex) == PB_QUANTITY)
				SetCurrentRenderElement(_currentRenderElement == NULL);
        }
		else {
			SetCurrentRenderElement(_currentRenderElement == NULL);
		}
        break;
    }
    
    return REF_SUCCEED;
}

int PseudoExposureControl::NumRefs()
{
    return LAST_REF + 1;
}

RefTargetHandle PseudoExposureControl::GetReference(int i)
{
    switch(i) {
    case REF_PARAMS:
        return _params;
    case REF_RENDER_ELEMENT:
        return _currentRenderElement;
    default:
        return NULL;
    }
}

void PseudoExposureControl::SetReference(int i, RefTargetHandle rtarg)
{
    switch(i) {
    case REF_PARAMS:
        {
            _params = static_cast<IParamBlock2*>(rtarg);
            Interval v;
            Update(GetCOREInterface()->GetTime(), v);
        }
        break;
    case REF_RENDER_ELEMENT:
        _currentRenderElement = static_cast<MaxRenderElement*>(rtarg);
        break;
    }
}

void PseudoExposureControl::GetClassName(TSTR& s)
{
    s = GetString(IDS_PSEUDO_EXP_CTL_CLASS_NAME);
}

Class_ID PseudoExposureControl::ClassID()
{
    return PSEUDO_EXP_CTL_CLASS_ID;
}

SClass_ID PseudoExposureControl::SuperClassID()
{
    return TONE_OPERATOR_CLASS_ID;
}

void PseudoExposureControl::DeleteThis()
{
    delete this;
}

void PseudoExposureControl::SetActive(bool active, TimeValue t)
{
    // Activate/Deactivate the render element
    EnableRenderElement(active);

    ToneOperator::SetActive(active, t);
}

void PseudoExposureControl::Update(TimeValue t, Interval& valid)
{
    _active = Active( t ) != 0;
    if(_params==NULL)
        return;
    
    _params->GetValue(PB_MAXIMUM, t, _maxBrightness, valid);
    _params->GetValue(PB_MINIMUM, t, _minBrightness, valid);
    _params->GetValue(PB_SCALE, t, _physScale, valid);
    _quantity = static_cast<IToneOperatorExtension::Quantity>(_params->GetInt(PB_QUANTITY, t));
    _scaleFunction = static_cast<ScaleFunction>(_params->GetInt(PB_SCALE_FUNC, t));
   
    // Convert min/max values to SI
    int unitSystemUsed = _params->GetInt(PB_UNITSYSTEM_USED);
    if(lightSystem && unitSystemUsed != ILightingUnits::DISPLAY_INTERNATIONAL) {
        if(_quantity == IToneOperatorExtension::kQuantity_Illuminance) {
            _maxBrightness = lightSystem->ConvertIlluminanceToSI(_maxBrightness);
            _minBrightness = lightSystem->ConvertIlluminanceToSI(_minBrightness);
        }
        else {
            _maxBrightness = lightSystem->ConvertLuminanceToSI(_maxBrightness);
            _minBrightness = lightSystem->ConvertLuminanceToSI(_minBrightness);
        }
    }

    buildColorTable();

    // Enable/Disable the render element depending on whether the tone operator is active.
    EnableRenderElement(Active(t) != 0);
}

void PseudoExposureControl::buildColorTable()
{
    int dsp = _params->GetInt(PB_DISPLAY);
    //  int qt = _params->GetInt(PB_QUANTITY);
    
    if (dsp == DSP_COLORED) 
    {
        Color col;
        int   hue; 
        float deltaHue = -COLOR_TABLE_HUE_RANGE / NB_PSEUDO_COLORS;

        for(int i = 0; i < NB_PSEUDO_COLORS; i++) {
            hue = COLOR_TABLE_START_HUE + i * deltaHue;
            _pseudoColorMap[i] = 0xff000000lu | HSVtoRGB(hue, COLOR_TABLE_SATURATION, COLOR_TABLE_VALUE);
        }
    } 
    else 
    {
        // gray scale

        ULONG a;
        float delta = COLOR_TABLE_NUMBER_OF_GRAY / NB_PSEUDO_COLORS;

        for (int i = 0; i < NB_PSEUDO_COLORS; i++) {
            a = (ULONG)(delta * i + 0.5);
            _pseudoColorMap[i] = 0xff000000lu | ((((a << 8) | a) << 8) | a);
        }
    }
    
    /* use for linear mapping */
    _conversionFactor = _maxBrightness > _minBrightness ?
        1.0 / (_maxBrightness - _minBrightness) : 1.0;
    _conversionFactor *= NB_PSEUDO_COLORS;
    /* used for log10 mapping */
    _cfB = 1.0 / (1.0 + _minBrightness);
    _cfA = NB_PSEUDO_COLORS;
    if (_maxBrightness > _minBrightness)
        _cfA /= log10((1 + _maxBrightness) / (1 + _minBrightness));

    // Build the bitmap for the legend
    if(_dialog != NULL)
        _dialog->UpdateLegendBitmap(_pseudoColorMap);
}

int PseudoExposureControl::NumSubs()
{
    return LAST_REF + 1;
}

Animatable* PseudoExposureControl::SubAnim(int i)
{
    switch(i) {
    case REF_PARAMS:
        return _params;
    case REF_RENDER_ELEMENT:
        return _currentRenderElement;
    default:
        return NULL;
    }
}

TSTR PseudoExposureControl::SubAnimName(int i)
{
    switch(i) {
    case REF_PARAMS:
        return GetString(IDS_PSEUDO_EXP_CTL_CLASS_NAME);
    case REF_RENDER_ELEMENT:
        return (_currentRenderElement != NULL) ? _currentRenderElement->GetName() : _T("");
    default:
        return _T("");
    }
}

int PseudoExposureControl::SubNumToRefNum(int subNum)
{
    switch(subNum) {
    case REF_PARAMS:
    case REF_RENDER_ELEMENT:
        return subNum;
    default:
        return -1;
    }
}

int	PseudoExposureControl::NumParamBlocks()
{
    return 1;
}

IParamBlock2* PseudoExposureControl::GetParamBlock(int i)
{
    return i == 0 ? _params : NULL;
}

IParamBlock2* PseudoExposureControl::GetParamBlockByID(short id)
{
    return id == PB_MAIN ? _params : NULL;
}

RefTargetHandle PseudoExposureControl::Clone( RemapDir &remap )
{
    PseudoExposureControl* map = new PseudoExposureControl( false );
    BaseClone( this, map, remap );
    return map;
}

void PseudoExposureControl::BaseClone(
                                      ReferenceTarget*  from,
                                      ReferenceTarget*  to,
                                      RemapDir&         remap
                                      )
{
    if ( from == NULL || to == NULL || from == to ) {
        return;
    }
    
    PseudoMap::BaseClone(from, to, remap);
    
    PseudoExposureControl* fromMap = static_cast< PseudoExposureControl* >( from );
    PseudoExposureControl* toMap = static_cast< PseudoExposureControl* >( to );
    
    toMap->ReplaceReference( REF_PARAMS, fromMap->_params->Clone( remap ) );
    toMap->ReplaceReference( REF_RENDER_ELEMENT, fromMap->_currentRenderElement->Clone( remap ) );
}

ToneOpParamDlg* PseudoExposureControl::CreateParamDialog(IRendParams* ip)
{
    _dialog = new PseudoExposureControlDialog();
    _dialog->Initialize(this, ip);
    _dialog->UpdateLegendBitmap(_pseudoColorMap);
    return _dialog;
}

static float calcBrightness(float value[3])
{
    return fabsf( value[0] ) * 0.263f
        +   fabsf( value[1] ) * 0.655f
        +   fabsf( value[2] ) * 0.082f;
}

// Convert radiance to color.
void PseudoExposureControl::ScaledToRGB( float* radiance )
{
    if ( _active ) {
        int i;
        float brightness;
        
        // [attilas|24.11.1998] DLMpd12458
        // Initialize a with a value that makes sense
        
        // I need to query the surface's alpha(transparency)
        // a = 255.0 * rdispAlphaScale;
        // a = 0;
        
        // It would be nice to include the undistributed energy
        //  spd_plus(&totalIrradiance, irradiance, &displayAmbient);
        
        //    if(GetQuantity()==QT_LUMINANCE) 
        //    {
        // spd_x_times_x(&radiance, &clusterDiffuseBRDF, &totalIrradiance);
        
        // It would be nice to support glowing
        //if(glowingCluster) {
        //  spd_plus(&radiance, &radiance, &glowingColor);
        //  a = 255.0;
        //}
        
        //    spd_to_RGB(&radiance, &c);
        //    } 
        //    else 
        //    {
        //    spd_to_RGB(&totalIrradiance, &c);
        //    a = 255.0 * rdispAlphaScale;
        //    }
        
        brightness = calcBrightness(radiance) * _physScale;
        if(_quantity == kQuantity_Luminance)
            brightness /= PI;
               
        if (brightness < _minBrightness) 
            brightness = _minBrightness;
        else if (brightness > _maxBrightness) 
            brightness = _maxBrightness;
        
        if (_scaleFunction == SCALE_LOGARITHMIC)
            i = (int)(_cfA * log10(_cfB * (1.0 + brightness)));
        else
            i = (int)((brightness - _minBrightness) * _conversionFactor);
        if (i < 0) 
            i = 0;
        else if (i >= NB_PSEUDO_COLORS) 
            i = NB_PSEUDO_COLORS - 1;
        
        radiance[0] = (float)(_pseudoColorMap[i] & 0xff) / 255.0;
        radiance[1] = (float)((_pseudoColorMap[i] >> 8) & 0xff) / 255.0;
        radiance[2] = (float)((_pseudoColorMap[i] >> 16) & 0xff) / 255.0;
    }
}

void PseudoExposureControl::GetEnergyRangeFromIndex(int colorIndex, float& floor, float& ceiling) {
    // This function is the inverse of ScaledToRGB()
    //
    // Log scale:
    //      i = _cfA * log10(_cfB * (1.0 + brightness))
    //    =>(pow(10, (i/_cfA)) / _cfB) - 1.0 = brightness
    //
    // Linear scaled:
    //      i = (brightness - _minBrightness) * _conversionFactor
    //    =>(i / _conversionFactor) + _minBrightness = brightness


    // If the color does not correspond to one in the pseudo color map
    if((colorIndex < 0) || (colorIndex >= NB_PSEUDO_COLORS)) {
        floor = ceiling = 0;
    }
    else {
        float floorIndex = static_cast<float>(colorIndex);
        float ceilingIndex = static_cast<float>(colorIndex) + 1.0f;

        if(_scaleFunction == SCALE_LOGARITHMIC) {
            floor = (pow(10, floorIndex / _cfA) / _cfB) - 1.0f;
            ceiling = (pow(10, ceilingIndex / _cfA) / _cfB) - 1.0f;
        }
        else {
            floor = (floorIndex / _conversionFactor) + _minBrightness;
            ceiling = (ceilingIndex / _conversionFactor) + _minBrightness;
        }
    }

    // Clamp the values which can be negative when evaluating at index 0
    if(floor < 0)
        floor = 0;
    if(ceiling < 0)
        ceiling = 0;

    ILightingUnits* ls = static_cast<ILightingUnits*>
                                    (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));
    if(ls)  {
    // Convert the values to the display unit system
    if(_quantity == kQuantity_Illuminance) {
            floor = ls->ConvertIlluminanceToCurrSystem(floor);
            ceiling = ls->ConvertIlluminanceToCurrSystem(ceiling);
    }
    else {
            floor = ls->ConvertLuminanceToCurrSystem(floor);
            ceiling = ls->ConvertLuminanceToCurrSystem(ceiling);
        }
    }

}

float PseudoExposureControl::ScaledToRGB(float energy)
{
    if ( _active ) {
        Color color( energy, energy, energy );
        PseudoExposureControl::ScaledToRGB( color );
        return calcBrightness(color) / PI;
    }
    
    return energy;
}

float PseudoExposureControl::GetPhysicalUnit(
                                             TimeValue   t,
                                             Interval&   valid
                                             ) const
{
    float value;
    _params->GetValue( PB_SCALE, t, value, valid );
    return value;
}

void PseudoExposureControl::SetPhysicalUnit(
                                            float    value,
                                            TimeValue   t
                                            )
{
    _params->SetValue( PB_SCALE, t, value );
}

void PseudoExposureControl::ScalePhysical(float energy[3]) const
{
    energy[0] /= _physScale;
    energy[1] /= _physScale;
    energy[2] /= _physScale;
}

float PseudoExposureControl::ScalePhysical(float energy) const
{
    return energy / _physScale;
}

void PseudoExposureControl::ScaleRGB(float color[3]) const
{
    color[0] *= _physScale;
    color[1] *= _physScale;
    color[2] *= _physScale;
}

float PseudoExposureControl::ScaleRGB(float color) const
{
    return color * _physScale;
}

void PseudoExposureControl::SetMaximum(
                                       TimeValue   t,
                                       float       max
                                       )
{
    _params->SetValue( PB_MAXIMUM, t, max );
}

float PseudoExposureControl::GetMaximum(
                                        TimeValue   t,
                                        Interval&   valid
                                        ) const
{
    float b;
    _params->GetValue( PB_MAXIMUM, t, b, valid );
    return b;
}

void PseudoExposureControl::SetMinimum(
                                       TimeValue   t,
                                       float min
                                       )
{
    _params->SetValue( PB_MINIMUM, t, min );
}

float PseudoExposureControl::GetMinimum(
                                        TimeValue   t,
                                        Interval&   valid
                                        ) const
{
    float c;
    _params->GetValue( PB_MINIMUM, t, c, valid );
    return c;
}

void PseudoExposureControl::SetDisplay( Display dsp )
{
    _params->SetValue( PB_DISPLAY, 0, dsp );
}

PseudoExposureControl::Display PseudoExposureControl::GetDisplay( ) const
{
    int dsp;
    Interval valid;
    _params->GetValue( PB_DISPLAY, 0, dsp, valid );
    return (Display)dsp;
}

void PseudoExposureControl::SetAutomatic( bool on )
{
    _params->SetValue( PB_AUTOMATIC, 0, int( on ) );
}

bool PseudoExposureControl::GetAutomatic( ) const
{
    int on;
    Interval valid;
    _params->GetValue( PB_AUTOMATIC, 0, on, valid );
    return on != 0;
}


void PseudoExposureControl::GetPseudoColorMap (ULONG * pPseudoColorMap, int iSizeOf)
{
    assert(iSizeOf >= sizeof(ULONG)*NB_PSEUDO_COLORS);

    memcpy(pPseudoColorMap, _pseudoColorMap, iSizeOf);  
}

bool PseudoExposureControl::BuildMaps(TimeValue t, RenderMapsContext& rmc)
{
    bool succeed = true;
    
    if (GetAutomatic( )) {
        _maxBrightness = 0.0f;
        _minBrightness = FLT_MAX;
        _samplingRender = true;
        succeed = renderMap(t, rmc);
        _samplingRender = false;
        if (succeed && _maxBrightness >= _minBrightness) {
            SetMaximum(t, _maxBrightness);
            SetMinimum(t, _minBrightness);
        }
        else
            Update(t, Interval(0,0));
        buildColorTable( );
    }
    
    return succeed;
}

void PseudoExposureControl::SubRenderSample(float energy[3])
{
    if (_samplingRender) {
        float brightness = calcBrightness(energy) * _physScale / PI;
        if (_maxBrightness < brightness)
            _maxBrightness = brightness;
        if (_minBrightness > brightness)
            _minBrightness = brightness;
    }
}

bool PseudoExposureControl::renderMap(
                                      TimeValue            t,
                                      RenderMapsContext&   rmc
                                      )
{
    const float TAN_HALF_DEGREE = .0087268779f;
    ViewParams vp;
    SubRendParams srp;
    
    rmc.GetCurrentViewParams(vp);
    rmc.GetSubRendParams(srp);
    
    srp.rendType = RENDTYPE_NORMAL;
    srp.fieldRender = false;
    srp.evenLines = false;
    srp.doingMirror = false;
    
    // Sample the image on 1 degree solid angles
    int width = tan(vp.fov / 2.0f) / TAN_HALF_DEGREE;
    int height = width * srp.devHeight / srp.devWidth;
    
    // Never over sample the image
    srp.devWidth = (srp.devWidth + 1) / 2;
    if (width < srp.devWidth) {
        srp.devWidth = width;
    }
    srp.devHeight = (srp.devHeight + 1) / 2;
    if (height < srp.devHeight) {
        srp.devHeight = height;
    }
    
    srp.xorg = 0;
    srp.yorg = 0;
    srp.xmin = 0;
    srp.xmax = srp.devWidth;
    srp.ymin = 0;
    srp.ymax = srp.devHeight;
    
    BitmapInfo bi;
    
    bi.SetType(BMM_GRAY_8);
    bi.SetWidth(srp.devWidth);
    bi.SetHeight(srp.devHeight);
    
    Bitmap* bm = TheManager->Create(&bi);
    if (bm == NULL) {
        return false;
    }

   // > 10/31/01 - 12:52pm --MQM-- 
   // let radiosity know we're doing a small tone-mapping image pass
   RenderGlobalContext *gc = rmc.GetGlobalContext();
   if ( gc && !gc->inMtlEdit )
	   BroadcastNotification( NOTIFY_BEGIN_RENDERING_TONEMAPPING_IMAGE, (void*)gc );
    
    bool rval = rmc.Render(bm, vp, srp) != 0;
    bm->DeleteThis();
    return rval;
}


BaseInterface* PseudoExposureControl::GetInterface(Interface_ID id) {

    if(id == ITONEOPERATOR_EXTENSION_INTERFACE)
        return static_cast<IToneOperatorExtension*>(this);
    else
        return ToneOperator::GetInterface(id);
}

void PseudoExposureControl::SetCurrentRenderElement(bool setToNone) {

    IRenderElementMgr* rendElemMgr = GetCOREInterface()->GetCurRenderElementMgr();
	IRenderElement* kept = NULL;
	bool keepCurrent;
	bool inUndo = theHold.RestoreOrRedoing() != 0;

	// If the _quantity isn't valid remove all render elements
	if (_quantity != IToneOperatorExtension::kQuantity_Illuminance
			&& _quantity != IToneOperatorExtension::kQuantity_Luminance)
		setToNone = true;

	// First synchronize the elements to the state of the exposure control
	{
		int n = rendElemMgr->NumRenderElements();
		Class_ID keep = setToNone || _quantity == IToneOperatorExtension::kQuantity_Illuminance
			? ILLUM_CLASS_ID : LUM_CLASS_ID;
		Class_ID remove = setToNone || _quantity == IToneOperatorExtension::kQuantity_Illuminance
			? LUM_CLASS_ID : ILLUM_CLASS_ID;
		keepCurrent = _currentRenderElement != NULL && inUndo
			&& !setToNone && _currentRenderElement->ClassID() == keep;

		// Loop through the render elements. Remove the ones that should
		// be removed. Keep one of the ones that should be kept. If we
		// are keeping _currentRenderElement, then we remove it and
		// put it back, to patch up some problems in undo with render elements.
		while (--n >= 0) {
			IRenderElement* elem = rendElemMgr->GetRenderElement(n);
			Class_ID id = elem->ClassID();
			if (!setToNone && id == keep) {
				if (keepCurrent) {
					rendElemMgr->RemoveRenderElement(elem);
				}
				else if (elem == _currentRenderElement) {
					if (kept != NULL)
						rendElemMgr->RemoveRenderElement(kept);
					kept = elem;
				}
				else if (kept == _currentRenderElement) {
					rendElemMgr->RemoveRenderElement(elem);
				}
				else {
					if (kept != NULL)
						rendElemMgr->RemoveRenderElement(kept);
					kept = elem;
				}
			}
			else if (id == remove || (setToNone && id == keep)) {
				rendElemMgr->RemoveRenderElement(elem);
			}
		}
	}

    if(setToNone) {
		// Make sure the reference is deleted
		if (!inUndo)
			DeleteReference(REF_RENDER_ELEMENT);
	}
	else {
		if (kept == NULL) {
			if (keepCurrent) {
				kept = _currentRenderElement;
			}
			else {
				// add the appropriate render element
				if(_quantity == IToneOperatorExtension::kQuantity_Illuminance) {
					kept = new IlluminationRenderElement;
				}
				else {
					kept = new LuminationRenderElement;
				}
			}

			/*
			// Create the bitmap for the render element
			PBBitmap* reBitmap = new PBBitmap;

			// Set the bitmap info using the renderer's bitmap info
			BitmapInfo bitmapInfo = GetCOREInterface()->GetRendDeviceBI();

			if(bitmapInfo.Type() == BMM_NO_TYPE)
				bitmapInfo.SetType(BMM_TRUE_32);

			reBitmap->bm = TheManager->Create(&bitmapInfo);
			reBitmap->bi.CopyImageInfo(&bitmapInfo);
			_currentRenderElement->SetPBBitmap(reBitmap);
			*/
			rendElemMgr->AddRenderElement(kept);
		}

		if (kept != _currentRenderElement && !inUndo)
			ReplaceReference(REF_RENDER_ELEMENT, kept);
    }
}

void PseudoExposureControl::EnableRenderElement(bool enable) {

    if((_currentRenderElement != NULL) 
        && ((_currentRenderElement->IsEnabled() != 0) ^ enable)) 
    {
            _currentRenderElement->SetEnabled(enable);
    }
}


//==============================================================================
// class PseudoExposureControlDialog
//==============================================================================

PseudoExposureControlDialog::PseudoExposureControlDialog()
: mp_exposureControl(NULL),
  mp_rendParam(NULL),
  mp_mainParamMap(NULL),
  mp_colorLegendControl(NULL),
  m_hImageList(NULL),
  m_legendWidth(0),
  m_legendHeight(0),
  m_hWnd(NULL) 
{

}


PseudoExposureControlDialog::~PseudoExposureControlDialog() {
    DestroyRParamMap2(mp_mainParamMap);
    ReleaseICustImage(mp_colorLegendControl);

    // Delete the image list
    ImageList_Destroy(m_hImageList);

    if(PseudoExposureControl::GetDialog() == this)
        PseudoExposureControl::SetDialog(NULL);
}


void PseudoExposureControlDialog::Initialize(PseudoExposureControl* exposureControl, IRendParams* rendParam) {

    mp_exposureControl = exposureControl;
    mp_rendParam = rendParam;
    mp_colorLegendControl = NULL;

    mp_mainParamMap = CreateRParamMap2(
        mp_exposureControl->GetParamBlockByID(PseudoMap::PB_MAIN),
        mp_rendParam,
        hInstance,
        MAKEINTRESOURCE(mainPBDesc2.dlg_template),
        GetString(mainPBDesc2.title),
        mainPBDesc2.rollup_flags,
        mainPBDesc2.dlgProc
    );

    assert(mp_mainParamMap != NULL);

    // Create the image list for the legend, with a single image
    m_hImageList = ImageList_Create(m_legendWidth, m_legendHeight, ILC_COLOR32, 1, 0);
}


BOOL PseudoExposureControlDialog::DialogProc(TimeValue t, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch(msg) {
    case WM_INITDIALOG:
        {
            // Fill the combo boxes with the text items
        
            // Quantity
            SendDlgItemMessage(hWnd, IDC_MAIN_QUANTITY, CB_RESETCONTENT, 0, 0);
            SendDlgItemMessage(hWnd, IDC_MAIN_QUANTITY, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_QUANTITY_ILLUMINANCE)));
            SendDlgItemMessage(hWnd, IDC_MAIN_QUANTITY, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_QUANTITY_LUMINANCE)));

            // Style
            SendDlgItemMessage(hWnd, IDC_MAIN_STYLE, CB_RESETCONTENT, 0, 0);
            SendDlgItemMessage(hWnd, IDC_MAIN_STYLE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_STYLE_COLORED)));
            SendDlgItemMessage(hWnd, IDC_MAIN_STYLE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_STYLE_GRAYSCALE)));

            // Scale
            SendDlgItemMessage(hWnd, IDC_MAIN_SCALE, CB_RESETCONTENT, 0, 0);
            SendDlgItemMessage(hWnd, IDC_MAIN_SCALE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_SCALE_LOGARITHMIC)));
            SendDlgItemMessage(hWnd, IDC_MAIN_SCALE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_SCALE_LINEAR)));

            // Set legend query values to zero
            // The legend query feature is disabled because it is not fully implemented
            //SetWindowTextFloat(GetDlgItem(hWnd, IDC_LEGEND_MIN), 0);
            //SetWindowTextFloat(GetDlgItem(hWnd, IDC_LEGEND_MAX), 0);

            // Initialize the custom image control of the color legend
            mp_colorLegendControl = GetICustImage(GetDlgItem(hWnd, IDC_COLOR_LEGEND));

            // Get the size of the legend window
            RECT rect;
            GetWindowRect(GetDlgItem(hWnd, IDC_COLOR_LEGEND), &rect);       
            m_legendWidth = rect.right - rect.left;
            m_legendHeight = rect.bottom - rect.top;

            m_hWnd = hWnd;
        }
        break;

    case WM_COMMAND:

        // Process changes in the combo box selection
        if(HIWORD(wParam) == CBN_SELCHANGE) {

            IParamBlock2* pblock = mp_mainParamMap->GetParamBlock();
            TCHAR buf[64];
        
            switch(LOWORD(wParam)) {
            case IDC_MAIN_QUANTITY:
                {
					theHold.Begin();
                    GetDlgItemText(hWnd, IDC_MAIN_QUANTITY, buf, 64);
            
                    if(_tcscmp(buf, GetString(IDS_QUANTITY_ILLUMINANCE)) == 0)
                        pblock->SetValue(PseudoMap::PB_QUANTITY, t, IToneOperatorExtension::kQuantity_Illuminance);
                    else if(_tcscmp(buf, GetString(IDS_QUANTITY_LUMINANCE)) == 0)
                        pblock->SetValue(PseudoMap::PB_QUANTITY, t, IToneOperatorExtension::kQuantity_Luminance);
					theHold.Accept(GetString(IDS_PARAM_CHANGE));
                }            
                break;

            case IDC_MAIN_STYLE:
				theHold.Begin();
                GetDlgItemText(hWnd, IDC_MAIN_STYLE, buf, 64);
            
                if(_tcscmp(buf, GetString(IDS_STYLE_COLORED)) == 0)
                    pblock->SetValue(PseudoMap::PB_DISPLAY, t, PseudoMap::DSP_COLORED);
                else if(_tcscmp(buf, GetString(IDS_STYLE_GRAYSCALE)) == 0)
                    pblock->SetValue(PseudoMap::PB_DISPLAY, t, PseudoMap::DSP_GRAY);
				theHold.Accept(GetString(IDS_PARAM_CHANGE));
            
                break;

            case IDC_MAIN_SCALE:
				theHold.Begin();
                GetDlgItemText(hWnd, IDC_MAIN_SCALE, buf, 64);
            
                if(_tcscmp(buf, GetString(IDS_SCALE_LOGARITHMIC)) == 0)
                    pblock->SetValue(PseudoMap::PB_SCALE_FUNC, t, PseudoMap::SCALE_LOGARITHMIC);
                else if(_tcscmp(buf, GetString(IDS_SCALE_LINEAR)) == 0)
                    pblock->SetValue(PseudoMap::PB_SCALE_FUNC, t, PseudoMap::SCALE_LINEAR);
				theHold.Accept(GetString(IDS_PARAM_CHANGE));
            
                break;

            default:
                return false;
            }
        }
        else {
            // The legend query feature is disabled because it is not fully implemented
            /*
            switch(LOWORD(wParam)) {
            case IDC_COLOR_LEGEND:
                {
                    HWND hLegendWnd = GetDlgItem(hWnd, IDC_COLOR_LEGEND);
                    // Get the point that was clicked in the legend
                    POINT point;
                    GetCursorPos(&point);

                    // Get the coordinates of the legend
                    RECT rect;
                    GetWindowRect(hLegendWnd, &rect);

                    // Get the coordinates of the point inside the legend window
                    point.x -= rect.left;
                    point.y -= rect.top;

                    // Make sure the cursor is inside the legend window
                    if((point.x >= 0) && (point.y >= 0)) {
                        // Determine the color of the bitmap at the point selected
                        // and get the energy corresponding to that color
                        IMAGEINFO imageInfo;
                        if(ImageList_GetImageInfo(m_hImageList, 0, &imageInfo)) {

                            int colorWidth = m_legendWidth / NB_PSEUDO_COLORS;
                            int gapSize = m_legendWidth - (colorWidth * NB_PSEUDO_COLORS);
                            int startGapSize = gapSize / 2;
                            int endGapIndex = m_legendWidth - (gapSize - startGapSize);
                            int colorIndex;

                            if(point.x < startGapSize)
                                colorIndex = 0;
                            else if(point.x >= endGapIndex)
                                colorIndex = NB_PSEUDO_COLORS - 1;
                            else
                                colorIndex = (point.x - startGapSize) / colorWidth;

                            float min;
                            float max;

                            mp_exposureControl->GetEnergyRangeFromIndex(colorIndex, min, max);
                            
                            SetWindowTextFloat(GetDlgItem(hWnd, IDC_LEGEND_MIN), min, 3);
                            SetWindowTextFloat(GetDlgItem(hWnd, IDC_LEGEND_MAX), max, 3);
                        }
                    }
                }
                break;
            
            default:
                return false;
            }
            */
        }
        break;

    case WM_DESTROY: {
        m_hWnd = NULL;
		if (mp_exposureControl != NULL) {
			IParamBlock2* pb = mp_exposureControl->getParams();
			TimeValue t = GetCOREInterface()->GetTime();

			if (pb != NULL) {
				ParamBlockDesc2* des = pb->GetDesc();

				if (des != NULL) {
					ParamDef* p = des->paramdefs;
					for (int i = des->count; --i >= 0; ) {
						// Set the current defaults from the current values.
						// This codes was taken straight from ParamBlk2.
						ParamDef& pd = p[i];

						if (!(pd.flags & P_RESET_DEFAULT)) {
							switch (pd.type) {
								case TYPE_ANGLE:
								case TYPE_PCNT_FRAC:
								case TYPE_WORLD:
								case TYPE_COLOR_CHANNEL:
								case TYPE_FLOAT:
									pd.cur_def.f = pb->GetFloat(pd.ID, t); 
									pd.flags |= P_HAS_CUR_DEFAULT;
									break;
								case TYPE_BOOL:
								case TYPE_TIMEVALUE:
								case TYPE_RADIOBTN_INDEX:
								case TYPE_INT: 	
								case TYPE_INDEX:
									pd.cur_def.i = pb->GetInt(pd.ID, t); 
									pd.flags |= P_HAS_CUR_DEFAULT;
									break;
								case TYPE_HSV:
								case TYPE_RGBA:
								case TYPE_POINT3: {
									if (pd.cur_def.p != NULL)
										delete pd.cur_def.p;
									pd.cur_def.p = new Point3(pb->GetPoint3(pd.ID, t)); 
									pd.flags |= P_HAS_CUR_DEFAULT;
								} break;
								case TYPE_FILENAME:
								case TYPE_STRING: {
									TCHAR* s = pb->GetStr(pd.ID, t);
									if (s != NULL) {
										if (pd.cur_def.s != NULL)
											free(pd.cur_def.s);
										pd.cur_def.s = _tcsdup(s); 
										pd.flags |= P_HAS_CUR_DEFAULT;
									}
									break;
								}
							}
						}
					}
				}
			}
		}
    } break;
        
    default:
        return false;
    }

    return true;
}


void PseudoExposureControlDialog::UpdateLegendBitmap(unsigned long* pseudoColorMap) {

    // Set the bitmap used in the legend custom control
    if((mp_colorLegendControl != NULL) && (m_legendWidth > 0) 
        && (m_legendHeight > 0) && (m_hWnd != NULL) && (m_hImageList != NULL) ) 
    {

        // Make sure the resolution of the color map is not greater than the size of the image
        // control
        DbgAssert(NB_PSEUDO_COLORS <= m_legendWidth);

        // Create a compatible (memory) device context
        HDC windowDC = GetDC(m_hWnd);
        assert((windowDC != NULL));

        BITMAPINFO bitmapInfo;
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = m_legendWidth;
        bitmapInfo.bmiHeader.biHeight = m_legendHeight;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;
        bitmapInfo.bmiHeader.biSizeImage = 0;

        RGBQUAD* bitValues = NULL;
        
        // Create the bitmap and its mask
        HBITMAP hBitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_PAL_COLORS, 
            reinterpret_cast<void**>(&bitValues), NULL, 0);
        DbgAssert((hBitmap != NULL) && (bitValues != NULL));

        if((hBitmap != NULL) && (bitValues != NULL)) {

            // Synchronise access to the bitmap
            GdiFlush();

            // The width in pixels to which a single color maps in the final bitmap
            int colorWidth = m_legendWidth / NB_PSEUDO_COLORS;
        
            // If there the bitmap does not fill the entire control, insert a gap at the beginning
            // and the end. That gap is simply set to black.
            int gapSize = m_legendWidth - (colorWidth * NB_PSEUDO_COLORS);
            int startGapSize = gapSize / 2;
            int endGapIndex = m_legendWidth - (gapSize - startGapSize);  // starting index of ending gap
            int lastIndex = (m_legendWidth * m_legendHeight) - 1;

            // Set the pixels of the bitmap
            for(int y = 0; y < m_legendHeight; ++y) {
                for(int x = 0; x < m_legendWidth; ++x) {

                    RGBQUAD rgbQuad;

                    // If not in the gap
                    if((x >= startGapSize) && (x < endGapIndex)) {                     
                        int pseudoColorIndex = (x - startGapSize) / colorWidth;
                        rgbQuad.rgbBlue  = GetBValue(pseudoColorMap[pseudoColorIndex]);
                        rgbQuad.rgbGreen = GetGValue(pseudoColorMap[pseudoColorIndex]);
                        rgbQuad.rgbRed   = GetRValue(pseudoColorMap[pseudoColorIndex]);
                    }
                    else {
                        // If in the gap, set to black
                        rgbQuad.rgbBlue = 0;
                        rgbQuad.rgbGreen = 0;
                        rgbQuad.rgbRed = 0;
                    }

                    bitValues[x + (m_legendWidth * y)] = rgbQuad;
                }
            }
        
            bool success;
            // Put the bitmap in the image list
            if(ImageList_GetImageCount(m_hImageList) == 0)
                success = (ImageList_Add(m_hImageList, hBitmap, NULL) != -1);
            else
                success = (ImageList_Replace(m_hImageList, 0, hBitmap, NULL) != 0);
            DbgAssert(success);

            // Update the image custom control
            if(success)
                mp_colorLegendControl->SetImage(m_hImageList, 0, m_legendWidth, m_legendHeight);

        }

        // Delete the bitmap
        if(hBitmap != NULL)
            DeleteObject(hBitmap);
    }
}


void PseudoExposureControlDialog::UpdateLegendValues() {

    if((mp_mainParamMap != NULL) && (mp_exposureControl != NULL)) {

        TCHAR* formatString;

        {
            // If the range of values is <= 10, display one decimal.
            TimeValue t = GetCOREInterface()->GetTime();
            Interval valid = FOREVER;

            float minimum = mp_exposureControl->GetMinimum(t, valid);
            float maximum = mp_exposureControl->GetMaximum(t, valid);

            if(fabs(maximum - minimum) <= 10.0f)
                formatString = _T("%.2f");
            else
                formatString = _T("%.f");
            
        }

        float min;
        float max;
        float avg;
        HWND hWnd = mp_mainParamMap->GetHWnd();
        TCHAR buf[12];

        mp_exposureControl->GetEnergyRangeFromIndex(0, min, max);
        _sntprintf(buf, 12, formatString, min);
        SetDlgItemText(hWnd, IDC_LEGEND_0, buf);

        mp_exposureControl->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS / 4, min, max);
        avg = (min + max) / 2.0f;
        _sntprintf(buf, 12, formatString, avg);
        SetDlgItemText(hWnd, IDC_LEGEND_25, buf);

        mp_exposureControl->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS / 2, min, max);
        avg = (min + max) / 2.0f;
        _sntprintf(buf, 12, formatString, avg);
        SetDlgItemText(hWnd, IDC_LEGEND_50, buf);

        mp_exposureControl->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS * 3 / 4, min, max);
        avg = (min + max) / 2.0f;
        _sntprintf(buf, 12, formatString, avg);
        SetDlgItemText(hWnd, IDC_LEGEND_75, buf);

        mp_exposureControl->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS - 1, min, max);
        _sntprintf(buf, 12, formatString, max);
        SetDlgItemText(hWnd, IDC_LEGEND_100, buf);
    }
}


void PseudoExposureControlDialog::SetThing(ReferenceTarget* m) {

    assert(m->ClassID() == mp_exposureControl->ClassID());

    mp_exposureControl = static_cast<PseudoExposureControl*>(m);
    mp_mainParamMap->SetParamBlock(mp_exposureControl->GetParamBlockByID(PseudoMap::PB_MAIN));

    ParamMap2UserDlgProc* dialogProc = mp_mainParamMap->GetUserDlgProc();
    dialogProc->SetThing(m);
    dialogProc->Update(GetCOREInterface()->GetTime());
}


//==============================================================================
// class MainPBDialogProc
//==============================================================================

void MainPBDialogProc::LightingUnitsChanged(void *param, NotifyInfo *info) {

    Interval valid(FOREVER);
    MainPBDialogProc* dialogProc = static_cast<MainPBDialogProc*>(param);
    if(dialogProc->exposureControl != NULL) {
        if(dialogProc->exposureControl->ClassID() == PSEUDO_EXP_CTL_CLASS_ID) {
            PseudoExposureControl* toneOp = static_cast<PseudoExposureControl*>(dialogProc->exposureControl);
            IParamBlock2* pBlock = toneOp->GetParamBlockByID(PseudoMap::PB_MAIN);
            if(pBlock != NULL) {
                IParamMap2* pmap = pBlock->GetMap();
                dialogProc->Update(GetCOREInterface()->GetTime(), valid, pmap);
            }
        }
    }
}


MainPBDialogProc::MainPBDialogProc() : exposureControl(NULL) {

    lightSystem = static_cast<ILightingUnits*>
                                    (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));
    RegisterNotification(LightingUnitsChanged, this, NOTIFY_LIGHTING_UNIT_DISPLAY_SYSTEM_CHANGE);
}


MainPBDialogProc::~MainPBDialogProc() {

    UnRegisterNotification(LightingUnitsChanged, this, NOTIFY_LIGHTING_UNIT_DISPLAY_SYSTEM_CHANGE);
}


BOOL MainPBDialogProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    PseudoExposureControlDialog* dialog = PseudoExposureControl::GetDialog();

    if(dialog != NULL)
        return dialog->DialogProc(t, hWnd, msg, wParam, lParam);

    return false;
}


void MainPBDialogProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap) {

    HWND hDlg = pmap->GetHWnd();
    IParamBlock2* pBlock = pmap->GetParamBlock();
    TSTR unitString;

    IToneOperatorExtension::Quantity quantity = 
        static_cast<IToneOperatorExtension::Quantity>(pBlock->GetInt(PseudoMap::PB_QUANTITY, t));

    // Convert the min/max values if the unit system changed
    int unitSystemUsed = pBlock->GetInt(PseudoMap::PB_UNITSYSTEM_USED);
    if(lightSystem && unitSystemUsed != lightSystem->GetLightingSystem()) {
        float max = pBlock->GetFloat(PseudoMap::PB_MAXIMUM);
        float min = pBlock->GetFloat(PseudoMap::PB_MINIMUM);

        if(quantity == IToneOperatorExtension::kQuantity_Illuminance) {
            max = lightSystem->ConvertIlluminanceToCurrSystem(max, unitSystemUsed);
            min = lightSystem->ConvertIlluminanceToCurrSystem(min, unitSystemUsed);
        }
        else {
            max = lightSystem->ConvertLuminanceToCurrSystem(max, unitSystemUsed);
            min = lightSystem->ConvertLuminanceToCurrSystem(min, unitSystemUsed);
        }

        pBlock->SetValue(PseudoMap::PB_MAXIMUM, 0, max);
        pBlock->SetValue(PseudoMap::PB_MINIMUM, 0, min);
        if(lightSystem)
            pBlock->SetValue(PseudoMap::PB_UNITSYSTEM_USED, 0, lightSystem->GetLightingSystem());
        else
        // then assume SI
            pBlock->SetValue(PseudoMap::PB_UNITSYSTEM_USED, 0, ILightingUnits::DISPLAY_INTERNATIONAL);
    }

    // Set the Quantity combo box
    switch(quantity) {
    case IToneOperatorExtension::kQuantity_Illuminance:
        SendDlgItemMessage(hDlg, IDC_MAIN_QUANTITY, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_QUANTITY_ILLUMINANCE)));
        if(lightSystem)
            unitString = lightSystem->GetIlluminanceUnits();
        break;
    case IToneOperatorExtension::kQuantity_Luminance:
        SendDlgItemMessage(hDlg, IDC_MAIN_QUANTITY, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_QUANTITY_LUMINANCE)));
        if(lightSystem)
            unitString = lightSystem->GetLuminanceUnits();
        break;
    }

    // Set the Style combo box
    switch(pBlock->GetInt(PseudoMap::PB_DISPLAY, t)) {
    case PseudoMap::DSP_COLORED:
        SendDlgItemMessage(hDlg, IDC_MAIN_STYLE, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_STYLE_COLORED)));
        break;
    case PseudoMap::DSP_GRAY:
        SendDlgItemMessage(hDlg, IDC_MAIN_STYLE, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_STYLE_GRAYSCALE)));
        break;
    }

    // Set the Scale Function combo box
    switch(pBlock->GetInt(PseudoMap::PB_SCALE_FUNC, t)) {
    case PseudoMap::SCALE_LOGARITHMIC:
        SendDlgItemMessage(hDlg, IDC_MAIN_SCALE, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_SCALE_LOGARITHMIC)));
        break;
    case PseudoMap::SCALE_LINEAR:
        SendDlgItemMessage(hDlg, IDC_MAIN_SCALE, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(GetString(IDS_SCALE_LINEAR)));
        break;
    }

    // Change the units being displayed
    SetDlgItemText(hDlg, IDC_MINIMUM_UNIT, unitString.data());
    SetDlgItemText(hDlg, IDC_MAXIMUM_UNIT, unitString.data());
    // The legend query feature is disabled because it is not fully implemented
    //SetDlgItemText(hDlg, IDC_LEGEND_MINUNITS, unitString.data());
    //SetDlgItemText(hDlg, IDC_LEGEND_MAXUNITS, unitString.data());
       
    // Enable/Disable Min & Max depending on whether the 'Automatic' checkbox is checked
    bool enable = (pBlock->GetInt(PseudoMap::PB_AUTOMATIC, t) == 0);
    EnableWindow(GetDlgItem(hDlg, IDC_MAIN_MINIMUM_EDIT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_MAIN_MINIMUM_SPINNER), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_MAIN_MAXIMUM_EDIT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_MAIN_MAXIMUM_SPINNER), enable);

    PseudoExposureControlDialog* dialog = PseudoExposureControl::GetDialog();
    if(dialog != NULL)
        dialog->UpdateLegendValues();
}


//==============================================================================
// class QuantityValidator
//==============================================================================

BOOL MainPBValidator::Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex) {

    switch(id) {
    case PseudoExposureControl::PB_QUANTITY:
        return (v.i == IToneOperatorExtension::kQuantity_Illuminance) 
            || (v.i == IToneOperatorExtension::kQuantity_Luminance);

    case PseudoExposureControl::PB_SCALE_FUNC:
        return (v.i == PseudoMap::SCALE_LOGARITHMIC)
            || (v.i == PseudoMap::SCALE_LINEAR);

    case PseudoExposureControl::PB_UNITSYSTEM_USED:
        return (v.i == ILightingUnits::DISPLAY_INTERNATIONAL)
            || (v.i == ILightingUnits::DISPLAY_AMERICAN);

    default:

        return true;
    }
}


