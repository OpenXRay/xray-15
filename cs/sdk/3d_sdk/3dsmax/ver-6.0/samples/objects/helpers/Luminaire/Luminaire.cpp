/**********************************************************************
 *<
	FILE: Luminaire.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: Attila Szabo

	HISTORY: Aug 12, 2001

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Luminaire.h"
#include "dummy.h"
#include "iLuminaire.h"

#define LUMINAIRE_CLASS_ID	Class_ID(0x43174610, 0x15c76b2f)

#define PBLOCK_REF	0

// From core\dummy.cpp
#define MESH_VALID 1
#define CREATING 2
#define DISABLE_DISPLAY 4

//===========================================================================
// LuminaireObject class
//===========================================================================
class LuminaireObject : public DummyObject, public ILuminaire 
{
	public:
		// --- Constructor/Destructor
		LuminaireObject();
		~LuminaireObject();		

		// Loading/Saving

		// --- From Animatable
		void						DeleteThis() { delete this; }		
		Class_ID				ClassID() {return LUMINAIRE_CLASS_ID;}		
		SClass_ID				SuperClassID() { return HELPER_CLASS_ID; }
		void						GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}
		int							NumSubs() { return 1; }
		TSTR						SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable*			SubAnim(int i) { return mpBlock; }

		// TODO: Maintain the number or references here
		int							NumRefs() { return 1; }
		RefTargetHandle	GetReference(int i) { return mpBlock; }
		void						SetReference(int i, RefTargetHandle rtarg);

		int							NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2*		GetParamBlock(int i) { return mpBlock; } // return i'th ParamBlock
		IParamBlock2*		GetParamBlockByID(BlockID id);
		// UI stuff
		void						BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev);
		void						EndEditParams(IObjParam* ip, ULONG flags, Animatable* next);
		// Persistence
		IOResult				Load(ILoad *iload);
		IOResult				Save(ISave *isave);
		
		// --- From ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap = NoRemap());

		// --- From Object
		void						InitNodeName(TSTR& s) { s = GetString(IDS_OBJ_NAME); } // Default node name

		// --- From BaseObject
		TCHAR*					GetObjectName() { return GetString(IDS_OBJ_NAME); }	// Appears in Modifier Stack
		// Need to re-implement because Luminaire has a different mesh than its base object
		void						GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox );
		void						GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& abox );
		void						GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel=FALSE );
		int							HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void						Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int							Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();

		// --- From ILuminaire
		virtual void		SetDimmer(float value, TimeValue time);
		virtual float		GetDimmer(TimeValue time, Interval& valid = FOREVER) const;
		virtual void		SetRGBFilterColor(Point3& value, TimeValue& time);
		virtual Point3	GetRGBFilterColor(TimeValue& time, Interval& valid = FOREVER) const;
		virtual void		SetUseState(bool onOff, TimeValue& time);
		virtual bool		GetUseState(TimeValue& time, Interval& valid = FOREVER) const;

		// --- Function publishing of ILuminaire interface
		BEGIN_FUNCTION_MAP 
 			PROP_TFNS(kLUM_GET_DIMMER, GetDimmer, kLUM_SET_DIMMER, SetDimmer, TYPE_FLOAT); 
 			PROP_TFNS(kLUM_GET_RGB_FILTER_COLOR, GetRGBFilterColor, kLUM_SET_RGB_FILTER_COLOR, SetRGBFilterColor, TYPE_RGBA); 
		END_FUNCTION_MAP 

		FPInterfaceDesc*	GetDesc() { return &mInterfaceDesc; }
		BaseInterface*		GetInterface(Interface_ID id);


	public:
		// Should not be localized
		static const TCHAR			mInternalClassName[MAX_PATH];
		static FPInterfaceDesc	mInterfaceDesc;
		static const Box3				mDefBoxSize;

	private:
		// In order to provide a different mesh, we need to redefine these methods
		void			BuildMesh();
		void			UpdateMesh();

		RefResult NotifyRefChanged(	Interval changeInt, 
																RefTargetHandle hTarget, 
																PartID& partID, 
																RefMessage message );

		// Parameter block
		IParamBlock2*					mpBlock;	//ref 0
};


//===========================================================================
// Class Desc
//===========================================================================
class LuminaireClassDesc : public ClassDesc2 
{
	public:
// aszabo - june.02.03 - Luminaire helper is exposed in MAX 6
// aszabo|Mar.08.02|Assembly related UI is not exposed in MAX 5, 
// so we hide Luminaire helper objects from the Create panel.
// This won't prevent Luminaire assemblies created in VIZ 4 to load
// and work properly, but the user won't be able to create Luminaire 
// assemblies through the UI or maxscript.
#ifdef PRIVATE_LUMINAIRE_HELPER_OBJECT
		int 					IsPublic() { return FALSE; }
#else
		int 					IsPublic() { return TRUE; }
#endif // DESIGN_VER

		void *				Create(BOOL loading = FALSE) { return new LuminaireObject(); }
		const TCHAR*	ClassName() { return GetString(IDS_CLASS_NAME); }
		SClass_ID			SuperClassID() { return HELPER_CLASS_ID; }
		Class_ID			ClassID() { return LUMINAIRE_CLASS_ID; }
		const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
		const TCHAR*	InternalName() { return LuminaireObject::mInternalClassName; }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE			HInstance() { return hInstance; }				// returns owning module handle
};

static LuminaireClassDesc LuminaireDesc;
ClassDesc2* GetLuminaireDesc() { return &LuminaireDesc; }

//===========================================================================
// ILuminare Interface descriptor
//===========================================================================
// Assembly Manager interface instance and descriptor
FPInterfaceDesc LuminaireObject::mInterfaceDesc(
	LUMINAIRE_INTERFACE,	// Interface id
	_T("ILuminaire"),			// Interface name used by maxscript - don't localize it!
	0,										// Res ID of description string
	&LuminaireDesc,				// Class descriptor						
	FP_MIXIN, 

	// - Methods -
	
	// - Properties -
	properties,
		ILuminaire::kLUM_GET_DIMMER, ILuminaire::kLUM_SET_DIMMER, _T("Dimmer"),	0, TYPE_FLOAT, 
		ILuminaire::kLUM_GET_RGB_FILTER_COLOR, ILuminaire::kLUM_SET_RGB_FILTER_COLOR, _T("FilterColor"),	0, TYPE_RGBA, 
	end 
); 

//===========================================================================
// Param Block  definition
//===========================================================================
enum 
{ 
	kLUM_PARAMS
};

// Add enums for various parameters
enum 
{ 
	kPB_DIMMER,
	kPB_FILTER_COLOR,
};

static ParamBlockDesc2 luminaire_param_blk ( 
	kLUM_PARAMS, _T("params"),  0, GetLuminaireDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	
	// rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	// params
	kPB_DIMMER,				_T("Dimmer"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_DIMMER_SPIN, 
		p_default, 	100.0f, 
		p_range, 		0.0f, 1000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_DIMMER_EDIT,	IDC_DIMMER_SPIN, 0.1f, 
		end,
	kPB_FILTER_COLOR,	 _T("FilterColor"),	TYPE_RGBA,		P_ANIMATABLE,	IDS_FILTER_COLOR,	
		p_default,	Color(1, 1, 1), 
		p_ui,				TYPE_COLORSWATCH, IDC_FILTER_COLOR, 
		end,

	end
);

//===========================================================================
// LuminaireObject create callback
//===========================================================================
class LuminaireObjectCreateCallBack: public CreateMouseCallBack 
{
	private:
		LuminaireObject*	mpObject;
		IPoint2						mSp0;
		Point3						mP0;

	public:
		int		proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void	SetObj(LuminaireObject *obj) { mpObject = obj; }
};

static LuminaireObjectCreateCallBack gLuminaireCreateCB;


//===========================================================================
// LuminaireObject implementation
//===========================================================================
const TCHAR LuminaireObject::mInternalClassName[] = _T("LuminaireHelper");
const Box3	LuminaireObject::mDefBoxSize(Point3(-10, -10, 0), Point3(10, 10, 20));


//---------------------------------------------------------------------------
// Constructor/Destructor
LuminaireObject::LuminaireObject() 
: mpBlock(NULL)
{ 
	GetLuminaireDesc()->MakeAutoParamBlocks(this);
	SetBox(const_cast<Box3&>(mDefBoxSize));
	dumFlags &= ~MESH_VALID;
}

LuminaireObject::~LuminaireObject() 
{ 
	DeleteAllRefsFromMe();
}

//---------------------------------------------------------------------------
//
void LuminaireObject::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
	GetLuminaireDesc()->BeginEditParams(ip, this, flags, prev);	
}

//---------------------------------------------------------------------------
//
void LuminaireObject::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	GetLuminaireDesc()->EndEditParams(ip, this, flags, next);
}

//---------------------------------------------------------------------------
// return id'd ParamBlock
IParamBlock2*	LuminaireObject::GetParamBlockByID(BlockID id) 
{ 
	if (mpBlock != NULL && mpBlock->ID() == id)
		return mpBlock; 
	
	return NULL;
} 

//---------------------------------------------------------------------------
//
IOResult LuminaireObject::Load(ILoad *iload) 
{ 
	return DummyObject::Load(iload); 
}
//---------------------------------------------------------------------------
//
IOResult LuminaireObject::Save(ISave *isave) 
{ 
	return DummyObject::Save(isave); 
}

//---------------------------------------------------------------------------
//
void LuminaireObject::SetReference(int i, RefTargetHandle rtarg) 
{ 
	mpBlock = static_cast<IParamBlock2*>(rtarg); 
}

//---------------------------------------------------------------------------
//
RefTargetHandle LuminaireObject::Clone(RemapDir& remap) 
{
	LuminaireObject* newObj = new LuminaireObject();	
	newObj->ReplaceReference(0, mpBlock->Clone(remap));
	BaseClone(this, newObj, remap);
	return newObj;
}

//---------------------------------------------------------------------------
//
RefResult LuminaireObject::NotifyRefChanged(	
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID, 
	RefMessage message ) 
{
	return (REF_SUCCEED);
}

// The following methdos had to be redefined because Luminaires have 
// a different mesh than its base object
void LuminaireObject::UpdateMesh() 
{
	if (!(dumFlags&MESH_VALID)) 
	{
		BuildMesh();
		dumFlags |= MESH_VALID;
	}
}
void LuminaireObject::BuildMesh()
{
	mesh.setNumVerts(120);
	mesh.setNumFaces(106);

	// aszabo|Feb.10.03|Compute scale of mesh. DummyObject::box has been scaled,
	// while our mDefBoxSize wasn't, but it was used to initialize the box.
	float s = box.Max().x/mDefBoxSize.Max().x;

	mesh.setVert(0, s*Point3(2.812824,-4.221277,0.000000));
	mesh.setVert(1, s*Point3(-2.731174,-4.221277,0.000000));
	mesh.setVert(2, s*Point3(-2.731174,-4.972452,0.000000));
	mesh.setVert(3, s*Point3(2.812824,-4.972452,0.000000));
	mesh.setVert(4, s*Point3(0.430951,-0.504076,0.000000));
	mesh.setVert(5, s*Point3(0.373688,-0.540217,0.000000));
	mesh.setVert(6, s*Point3(0.312884,-0.570737,0.000000));
	mesh.setVert(7, s*Point3(0.248868,-0.595212,0.000000));
	mesh.setVert(8, s*Point3(0.181967,-0.613220,0.000000));
	mesh.setVert(9, s*Point3(0.112510,-0.624335,0.000000));
	mesh.setVert(10, s*Point3(0.040825,-0.628134,0.000000));
	mesh.setVert(11, s*Point3(-0.030860,-0.624335,0.000000));
	mesh.setVert(12, s*Point3(-0.100318,-0.613220,0.000000));
	mesh.setVert(13, s*Point3(-0.167219,-0.595213,0.000000));
	mesh.setVert(14, s*Point3(-0.231235,-0.570737,0.000000));
	mesh.setVert(15, s*Point3(-0.292039,-0.540217,0.000000));
	mesh.setVert(16, s*Point3(-0.349302,-0.504076,0.000000));
	mesh.setVert(17, s*Point3(-0.402695,-0.462737,0.000000));
	mesh.setVert(18, s*Point3(-0.402695,-4.025318,0.000000));
	mesh.setVert(19, s*Point3(0.484345,-4.025318,0.000000));
	mesh.setVert(20, s*Point3(0.484345,-0.462738,0.000000));
	mesh.setVert(21, s*Point3(0.373465,-4.025318,0.000000));
	mesh.setVert(22, s*Point3(-0.291815,-4.025318,0.000000));
	mesh.setVert(23, s*Point3(-0.291815,-4.221277,0.000000));
	mesh.setVert(24, s*Point3(0.373465,-4.221277,0.000000));
	mesh.setVert(25, s*Point3(4.419012,5.754055,0.000000));
	mesh.setVert(26, s*Point3(2.964259,4.637785,0.000000));
	mesh.setVert(27, s*Point3(3.340581,4.051188,0.000000));
	mesh.setVert(28, s*Point3(3.598334,3.467271,0.000000));
	mesh.setVert(29, s*Point3(3.784944,2.884963,0.000000));
	mesh.setVert(30, s*Point3(3.947842,2.303191,0.000000));
	mesh.setVert(31, s*Point3(4.134453,1.720884,0.000000));
	mesh.setVert(32, s*Point3(4.392206,1.136967,0.000000));
	mesh.setVert(33, s*Point3(4.768528,0.550370,0.000000));
	mesh.setVert(34, s*Point3(7.900172,2.953366,0.000000));
	mesh.setVert(35, s*Point3(7.430962,3.468688,0.000000));
	mesh.setVert(36, s*Point3(6.933653,3.868786,0.000000));
	mesh.setVert(37, s*Point3(6.419484,4.199750,0.000000));
	mesh.setVert(38, s*Point3(5.899698,4.507671,0.000000));
	mesh.setVert(39, s*Point3(5.385530,4.838635,0.000000));
	mesh.setVert(40, s*Point3(4.888222,5.238734,0.000000));
	mesh.setVert(41, s*Point3(2.805657,2.557596,0.000000));
	mesh.setVert(42, s*Point3(2.664196,2.574949,0.000000));
	mesh.setVert(43, s*Point3(2.533501,2.619650,0.000000));
	mesh.setVert(44, s*Point3(2.416477,2.688673,0.000000));
	mesh.setVert(45, s*Point3(2.316031,2.778996,0.000000));
	mesh.setVert(46, s*Point3(2.235066,2.887594,0.000000));
	mesh.setVert(47, s*Point3(2.176486,3.011443,0.000000));
	mesh.setVert(48, s*Point3(2.143197,3.147520,0.000000));
	mesh.setVert(49, s*Point3(-0.028124,0.736024,0.000000));
	mesh.setVert(50, s*Point3(-0.018348,0.737188,0.000000));
	mesh.setVert(51, s*Point3(-0.008572,0.738606,0.000000));
	mesh.setVert(52, s*Point3(0.001220,0.740119,0.000000));
	mesh.setVert(53, s*Point3(0.011041,0.741571,0.000000));
	mesh.setVert(54, s*Point3(0.020906,0.742804,0.000000));
	mesh.setVert(55, s*Point3(0.030829,0.743659,0.000000));
	mesh.setVert(56, s*Point3(0.040825,0.743979,0.000000));
	mesh.setVert(57, s*Point3(0.174745,0.730856,0.000000));
	mesh.setVert(58, s*Point3(0.299803,0.693174,0.000000));
	mesh.setVert(59, s*Point3(0.413514,0.633459,0.000000));
	mesh.setVert(60, s*Point3(0.513398,0.554243,0.000000));
	mesh.setVert(61, s*Point3(0.596971,0.458053,0.000000));
	mesh.setVert(62, s*Point3(0.661752,0.347418,0.000000));
	mesh.setVert(63, s*Point3(0.705257,0.224866,0.000000));
	mesh.setVert(64, s*Point3(0.726882,0.057922,0.000000));
	mesh.setVert(65, s*Point3(0.708744,0.215121,0.000000));
	mesh.setVert(66, s*Point3(0.657090,0.359483,0.000000));
	mesh.setVert(67, s*Point3(0.576055,0.486872,0.000000));
	mesh.setVert(68, s*Point3(0.469775,0.593152,0.000000));
	mesh.setVert(69, s*Point3(0.342386,0.674188,0.000000));
	mesh.setVert(70, s*Point3(0.198024,0.725842,0.000000));
	mesh.setVert(71, s*Point3(0.040825,0.743979,0.000000));
	mesh.setVert(72, s*Point3(-0.116374,0.725842,0.000000));
	mesh.setVert(73, s*Point3(-0.260736,0.674188,0.000000));
	mesh.setVert(74, s*Point3(-0.388125,0.593152,0.000000));
	mesh.setVert(75, s*Point3(-0.494405,0.486872,0.000000));
	mesh.setVert(76, s*Point3(-0.575440,0.359483,0.000000));
	mesh.setVert(77, s*Point3(-0.627095,0.215121,0.000000));
	mesh.setVert(78, s*Point3(-0.645232,0.057922,0.000000));
	mesh.setVert(79, s*Point3(-0.627095,-0.099276,0.000000));
	mesh.setVert(80, s*Point3(-0.575440,-0.243638,0.000000));
	mesh.setVert(81, s*Point3(-0.494405,-0.371027,0.000000));
	mesh.setVert(82, s*Point3(-0.388125,-0.477308,0.000000));
	mesh.setVert(83, s*Point3(-0.260736,-0.558343,0.000000));
	mesh.setVert(84, s*Point3(-0.116374,-0.609997,0.000000));
	mesh.setVert(85, s*Point3(0.040825,-0.628134,0.000000));
	mesh.setVert(86, s*Point3(0.198024,-0.609997,0.000000));
	mesh.setVert(87, s*Point3(0.342386,-0.558343,0.000000));
	mesh.setVert(88, s*Point3(0.469775,-0.477308,0.000000));
	mesh.setVert(89, s*Point3(0.576055,-0.371027,0.000000));
	mesh.setVert(90, s*Point3(0.657090,-0.243638,0.000000));
	mesh.setVert(91, s*Point3(0.708744,-0.099276,0.000000));
	mesh.setVert(92, s*Point3(3.504380,3.242251,0.000000));
	mesh.setVert(93, s*Point3(3.486243,3.399450,0.000000));
	mesh.setVert(94, s*Point3(3.434589,3.543813,0.000000));
	mesh.setVert(95, s*Point3(3.353553,3.671200,0.000000));
	mesh.setVert(96, s*Point3(3.247273,3.777481,0.000000));
	mesh.setVert(97, s*Point3(3.119884,3.858516,0.000000));
	mesh.setVert(98, s*Point3(2.975523,3.910170,0.000000));
	mesh.setVert(99, s*Point3(2.818324,3.928308,0.000000));
	mesh.setVert(100, s*Point3(2.661125,3.910171,0.000000));
	mesh.setVert(101, s*Point3(2.516763,3.858516,0.000000));
	mesh.setVert(102, s*Point3(2.389374,3.777481,0.000000));
	mesh.setVert(103, s*Point3(2.283094,3.671201,0.000000));
	mesh.setVert(104, s*Point3(2.202058,3.543812,0.000000));
	mesh.setVert(105, s*Point3(2.150404,3.399450,0.000000));
	mesh.setVert(106, s*Point3(2.132267,3.242251,0.000000));
	mesh.setVert(107, s*Point3(2.150404,3.085053,0.000000));
	mesh.setVert(108, s*Point3(2.202059,2.940691,0.000000));
	mesh.setVert(109, s*Point3(2.283093,2.813301,0.000000));
	mesh.setVert(110, s*Point3(2.389374,2.707022,0.000000));
	mesh.setVert(111, s*Point3(2.516763,2.625986,0.000000));
	mesh.setVert(112, s*Point3(2.661125,2.574332,0.000000));
	mesh.setVert(113, s*Point3(2.818324,2.556195,0.000000));
	mesh.setVert(114, s*Point3(2.975523,2.574332,0.000000));
	mesh.setVert(115, s*Point3(3.119885,2.625987,0.000000));
	mesh.setVert(116, s*Point3(3.247273,2.707021,0.000000));
	mesh.setVert(117, s*Point3(3.353554,2.813302,0.000000));
	mesh.setVert(118, s*Point3(3.434589,2.940691,0.000000));
	mesh.setVert(119, s*Point3(3.486243,3.085052,0.000000));
	Face f;

	f.v[0] = 2;  f.v[1] = 3;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 67; mesh.faces[0] = f; 
	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 1;  	f.smGroup = 1;  f.flags = 70; mesh.faces[1] = f;
	f.v[0] = 16;  f.v[1] = 17;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 67; mesh.faces[2] = f;
	f.v[0] = 19;  f.v[1] = 20;  f.v[2] = 4;  	f.smGroup = 1;  f.flags = 67; mesh.faces[3] = f;
	f.v[0] = 15;  f.v[1] = 16;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 65; mesh.faces[4] = f;
	f.v[0] = 19;  f.v[1] = 4;  f.v[2] = 5;  	f.smGroup = 1;  f.flags = 66; mesh.faces[5] = f;
	f.v[0] = 19;  f.v[1] = 5;  f.v[2] = 6;  	f.smGroup = 1;  f.flags = 66; mesh.faces[6] = f;
	f.v[0] = 14;  f.v[1] = 15;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 65; mesh.faces[7] = f;
	f.v[0] = 13;  f.v[1] = 14;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 65; mesh.faces[8] = f;
	f.v[0] = 19;  f.v[1] = 6;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 66; mesh.faces[9] = f;
	f.v[0] = 19;  f.v[1] = 7;  f.v[2] = 8;  	f.smGroup = 1;  f.flags = 66; mesh.faces[10] = f;
	f.v[0] = 12;  f.v[1] = 13;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 65; mesh.faces[11] = f;
	f.v[0] = 18;  f.v[1] = 19;  f.v[2] = 8;  	f.smGroup = 1;  f.flags = 65; mesh.faces[12] = f;
	f.v[0] = 18;  f.v[1] = 8;  f.v[2] = 9;  	f.smGroup = 1;  f.flags = 66; mesh.faces[13] = f;
	f.v[0] = 18;  f.v[1] = 9;  f.v[2] = 10;  	f.smGroup = 1;  f.flags = 66; mesh.faces[14] = f;
	f.v[0] = 18;  f.v[1] = 10;  f.v[2] = 11;  	f.smGroup = 1;  f.flags = 66; mesh.faces[15] = f;
	f.v[0] = 12;  f.v[1] = 18;  f.v[2] = 11;  	f.smGroup = 1;  f.flags = 68; mesh.faces[16] = f;
	f.v[0] = 23;  f.v[1] = 24;  f.v[2] = 21;  	f.smGroup = 1;  f.flags = 67; mesh.faces[17] = f;
	f.v[0] = 23;  f.v[1] = 21;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 70; mesh.faces[18] = f;
	f.v[0] = 32;  f.v[1] = 33;  f.v[2] = 34;  	f.smGroup = 1;  f.flags = 67; mesh.faces[19] = f;
	f.v[0] = 32;  f.v[1] = 34;  f.v[2] = 35;  	f.smGroup = 1;  f.flags = 66; mesh.faces[20] = f;
	f.v[0] = 32;  f.v[1] = 35;  f.v[2] = 36;  	f.smGroup = 1;  f.flags = 66; mesh.faces[21] = f;
	f.v[0] = 31;  f.v[1] = 32;  f.v[2] = 36;  	f.smGroup = 1;  f.flags = 65; mesh.faces[22] = f;
	f.v[0] = 31;  f.v[1] = 36;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 66; mesh.faces[23] = f;
	f.v[0] = 30;  f.v[1] = 31;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 65; mesh.faces[24] = f;
	f.v[0] = 29;  f.v[1] = 30;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 65; mesh.faces[25] = f;
	f.v[0] = 29;  f.v[1] = 37;  f.v[2] = 38;  	f.smGroup = 1;  f.flags = 66; mesh.faces[26] = f;
	f.v[0] = 28;  f.v[1] = 29;  f.v[2] = 38;  	f.smGroup = 1;  f.flags = 65; mesh.faces[27] = f;
	f.v[0] = 28;  f.v[1] = 38;  f.v[2] = 39;  	f.smGroup = 1;  f.flags = 66; mesh.faces[28] = f;
	f.v[0] = 27;  f.v[1] = 28;  f.v[2] = 39;  	f.smGroup = 1;  f.flags = 65; mesh.faces[29] = f;
	f.v[0] = 27;  f.v[1] = 39;  f.v[2] = 40;  	f.smGroup = 1;  f.flags = 66; mesh.faces[30] = f;
	f.v[0] = 27;  f.v[1] = 40;  f.v[2] = 25;  	f.smGroup = 1;  f.flags = 66; mesh.faces[31] = f;
	f.v[0] = 27;  f.v[1] = 25;  f.v[2] = 26;  	f.smGroup = 1;  f.flags = 70; mesh.faces[32] = f;
	f.v[0] = 48;  f.v[1] = 49;  f.v[2] = 50;  	f.smGroup = 1;  f.flags = 67; mesh.faces[33] = f;
	f.v[0] = 48;  f.v[1] = 50;  f.v[2] = 51;  	f.smGroup = 1;  f.flags = 66; mesh.faces[34] = f;
	f.v[0] = 48;  f.v[1] = 51;  f.v[2] = 52;  	f.smGroup = 1;  f.flags = 66; mesh.faces[35] = f;
	f.v[0] = 48;  f.v[1] = 52;  f.v[2] = 53;  	f.smGroup = 1;  f.flags = 66; mesh.faces[36] = f;
	f.v[0] = 48;  f.v[1] = 53;  f.v[2] = 54;  	f.smGroup = 1;  f.flags = 66; mesh.faces[37] = f;
	f.v[0] = 48;  f.v[1] = 54;  f.v[2] = 55;  	f.smGroup = 1;  f.flags = 66; mesh.faces[38] = f;
	f.v[0] = 48;  f.v[1] = 55;  f.v[2] = 56;  	f.smGroup = 1;  f.flags = 66; mesh.faces[39] = f;
	f.v[0] = 48;  f.v[1] = 56;  f.v[2] = 57;  	f.smGroup = 1;  f.flags = 66; mesh.faces[40] = f;
	f.v[0] = 47;  f.v[1] = 48;  f.v[2] = 57;  	f.smGroup = 1;  f.flags = 65; mesh.faces[41] = f;
	f.v[0] = 63;  f.v[1] = 41;  f.v[2] = 42;  	f.smGroup = 1;  f.flags = 67; mesh.faces[42] = f;
	f.v[0] = 62;  f.v[1] = 63;  f.v[2] = 42;  	f.smGroup = 1;  f.flags = 65; mesh.faces[43] = f;
	f.v[0] = 47;  f.v[1] = 57;  f.v[2] = 58;  	f.smGroup = 1;  f.flags = 66; mesh.faces[44] = f;
	f.v[0] = 46;  f.v[1] = 47;  f.v[2] = 58;  	f.smGroup = 1;  f.flags = 65; mesh.faces[45] = f;
	f.v[0] = 62;  f.v[1] = 42;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 66; mesh.faces[46] = f;
	f.v[0] = 61;  f.v[1] = 62;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 65; mesh.faces[47] = f;
	f.v[0] = 46;  f.v[1] = 58;  f.v[2] = 59;  	f.smGroup = 1;  f.flags = 66; mesh.faces[48] = f;
	f.v[0] = 45;  f.v[1] = 46;  f.v[2] = 59;  	f.smGroup = 1;  f.flags = 65; mesh.faces[49] = f;
	f.v[0] = 61;  f.v[1] = 43;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 66; mesh.faces[50] = f;
	f.v[0] = 60;  f.v[1] = 61;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[51] = f;
	f.v[0] = 45;  f.v[1] = 59;  f.v[2] = 60;  	f.smGroup = 1;  f.flags = 66; mesh.faces[52] = f;
	f.v[0] = 45;  f.v[1] = 60;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 68; mesh.faces[53] = f;
	f.v[0] = 91;  f.v[1] = 64;  f.v[2] = 65;  	f.smGroup = 1;  f.flags = 67; mesh.faces[54] = f;
	f.v[0] = 91;  f.v[1] = 65;  f.v[2] = 66;  	f.smGroup = 1;  f.flags = 66; mesh.faces[55] = f;
	f.v[0] = 90;  f.v[1] = 91;  f.v[2] = 66;  	f.smGroup = 1;  f.flags = 65; mesh.faces[56] = f;
	f.v[0] = 90;  f.v[1] = 66;  f.v[2] = 67;  	f.smGroup = 1;  f.flags = 66; mesh.faces[57] = f;
	f.v[0] = 89;  f.v[1] = 90;  f.v[2] = 67;  	f.smGroup = 1;  f.flags = 65; mesh.faces[58] = f;
	f.v[0] = 88;  f.v[1] = 89;  f.v[2] = 67;  	f.smGroup = 1;  f.flags = 65; mesh.faces[59] = f;
	f.v[0] = 88;  f.v[1] = 67;  f.v[2] = 68;  	f.smGroup = 1;  f.flags = 66; mesh.faces[60] = f;
	f.v[0] = 88;  f.v[1] = 68;  f.v[2] = 69;  	f.smGroup = 1;  f.flags = 66; mesh.faces[61] = f;
	f.v[0] = 88;  f.v[1] = 69;  f.v[2] = 70;  	f.smGroup = 1;  f.flags = 66; mesh.faces[62] = f;
	f.v[0] = 88;  f.v[1] = 70;  f.v[2] = 71;  	f.smGroup = 1;  f.flags = 66; mesh.faces[63] = f;
	f.v[0] = 88;  f.v[1] = 71;  f.v[2] = 72;  	f.smGroup = 1;  f.flags = 66; mesh.faces[64] = f;
	f.v[0] = 88;  f.v[1] = 72;  f.v[2] = 73;  	f.smGroup = 1;  f.flags = 66; mesh.faces[65] = f;
	f.v[0] = 88;  f.v[1] = 73;  f.v[2] = 74;  	f.smGroup = 1;  f.flags = 66; mesh.faces[66] = f;
	f.v[0] = 88;  f.v[1] = 74;  f.v[2] = 75;  	f.smGroup = 1;  f.flags = 66; mesh.faces[67] = f;
	f.v[0] = 88;  f.v[1] = 75;  f.v[2] = 76;  	f.smGroup = 1;  f.flags = 66; mesh.faces[68] = f;
	f.v[0] = 88;  f.v[1] = 76;  f.v[2] = 77;  	f.smGroup = 1;  f.flags = 66; mesh.faces[69] = f;
	f.v[0] = 88;  f.v[1] = 77;  f.v[2] = 78;  	f.smGroup = 1;  f.flags = 66; mesh.faces[70] = f;
	f.v[0] = 88;  f.v[1] = 78;  f.v[2] = 79;  	f.smGroup = 1;  f.flags = 66; mesh.faces[71] = f;
	f.v[0] = 88;  f.v[1] = 79;  f.v[2] = 80;  	f.smGroup = 1;  f.flags = 66; mesh.faces[72] = f;
	f.v[0] = 88;  f.v[1] = 80;  f.v[2] = 81;  	f.smGroup = 1;  f.flags = 66; mesh.faces[73] = f;
	f.v[0] = 88;  f.v[1] = 81;  f.v[2] = 82;  	f.smGroup = 1;  f.flags = 66; mesh.faces[74] = f;
	f.v[0] = 88;  f.v[1] = 82;  f.v[2] = 83;  	f.smGroup = 1;  f.flags = 66; mesh.faces[75] = f;
	f.v[0] = 88;  f.v[1] = 83;  f.v[2] = 84;  	f.smGroup = 1;  f.flags = 66; mesh.faces[76] = f;
	f.v[0] = 88;  f.v[1] = 84;  f.v[2] = 85;  	f.smGroup = 1;  f.flags = 66; mesh.faces[77] = f;
	f.v[0] = 88;  f.v[1] = 85;  f.v[2] = 86;  	f.smGroup = 1;  f.flags = 66; mesh.faces[78] = f;
	f.v[0] = 88;  f.v[1] = 86;  f.v[2] = 87;  	f.smGroup = 1;  f.flags = 70; mesh.faces[79] = f;
	f.v[0] = 105;  f.v[1] = 106;  f.v[2] = 107;  	f.smGroup = 1;  f.flags = 67; mesh.faces[80] = f;
	f.v[0] = 105;  f.v[1] = 107;  f.v[2] = 108;  	f.smGroup = 1;  f.flags = 66; mesh.faces[81] = f;
	f.v[0] = 104;  f.v[1] = 105;  f.v[2] = 108;  	f.smGroup = 1;  f.flags = 65; mesh.faces[82] = f;
	f.v[0] = 103;  f.v[1] = 104;  f.v[2] = 108;  	f.smGroup = 1;  f.flags = 65; mesh.faces[83] = f;
	f.v[0] = 103;  f.v[1] = 108;  f.v[2] = 109;  	f.smGroup = 1;  f.flags = 66; mesh.faces[84] = f;
	f.v[0] = 103;  f.v[1] = 109;  f.v[2] = 110;  	f.smGroup = 1;  f.flags = 66; mesh.faces[85] = f;
	f.v[0] = 102;  f.v[1] = 103;  f.v[2] = 110;  	f.smGroup = 1;  f.flags = 65; mesh.faces[86] = f;
	f.v[0] = 102;  f.v[1] = 110;  f.v[2] = 111;  	f.smGroup = 1;  f.flags = 66; mesh.faces[87] = f;
	f.v[0] = 102;  f.v[1] = 111;  f.v[2] = 112;  	f.smGroup = 1;  f.flags = 66; mesh.faces[88] = f;
	f.v[0] = 102;  f.v[1] = 112;  f.v[2] = 113;  	f.smGroup = 1;  f.flags = 66; mesh.faces[89] = f;
	f.v[0] = 102;  f.v[1] = 113;  f.v[2] = 114;  	f.smGroup = 1;  f.flags = 66; mesh.faces[90] = f;
	f.v[0] = 102;  f.v[1] = 114;  f.v[2] = 115;  	f.smGroup = 1;  f.flags = 66; mesh.faces[91] = f;
	f.v[0] = 102;  f.v[1] = 115;  f.v[2] = 116;  	f.smGroup = 1;  f.flags = 66; mesh.faces[92] = f;
	f.v[0] = 102;  f.v[1] = 116;  f.v[2] = 117;  	f.smGroup = 1;  f.flags = 66; mesh.faces[93] = f;
	f.v[0] = 102;  f.v[1] = 117;  f.v[2] = 118;  	f.smGroup = 1;  f.flags = 66; mesh.faces[94] = f;
	f.v[0] = 102;  f.v[1] = 118;  f.v[2] = 119;  	f.smGroup = 1;  f.flags = 66; mesh.faces[95] = f;
	f.v[0] = 102;  f.v[1] = 119;  f.v[2] = 92;  	f.smGroup = 1;  f.flags = 66; mesh.faces[96] = f;
	f.v[0] = 102;  f.v[1] = 92;  f.v[2] = 93;  	f.smGroup = 1;  f.flags = 66; mesh.faces[97] = f;
	f.v[0] = 102;  f.v[1] = 93;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 66; mesh.faces[98] = f;
	f.v[0] = 102;  f.v[1] = 94;  f.v[2] = 95;  	f.smGroup = 1;  f.flags = 66; mesh.faces[99] = f;
	f.v[0] = 102;  f.v[1] = 95;  f.v[2] = 96;  	f.smGroup = 1;  f.flags = 66; mesh.faces[100] = f;
	f.v[0] = 102;  f.v[1] = 96;  f.v[2] = 97;  	f.smGroup = 1;  f.flags = 66; mesh.faces[101] = f;
	f.v[0] = 102;  f.v[1] = 97;  f.v[2] = 98;  	f.smGroup = 1;  f.flags = 66; mesh.faces[102] = f;
	f.v[0] = 102;  f.v[1] = 98;  f.v[2] = 99;  	f.smGroup = 1;  f.flags = 66; mesh.faces[103] = f;
	f.v[0] = 102;  f.v[1] = 99;  f.v[2] = 100;  	f.smGroup = 1;  f.flags = 66; mesh.faces[104] = f;
	f.v[0] = 102;  f.v[1] = 100;  f.v[2] = 101;  	f.smGroup = 1;  f.flags = 70; mesh.faces[105] = f;
	// This makes the mesh "float" on top of other objects
	mesh.InvalidateGeomCache();
	mesh.EnableEdgeList(1);
}

void LuminaireObject::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox )
{
	if (dumFlags&DISABLE_DISPLAY)
		return;
	Matrix3 mat = inode->GetObjectTM(t);
	UpdateMesh();
	abox = mesh.getBoundingBox();
	abox = abox * mat;
}

void LuminaireObject::GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& abox )
{
	UpdateMesh();
	abox = mesh.getBoundingBox();
}

void LuminaireObject::GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel )
{
	UpdateMesh();
	abox = mesh.getBoundingBox(tm);
}

int LuminaireObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
{
	HitRegion hitRegion;
	Matrix3 m;
	if (dumFlags&DISABLE_DISPLAY)
		return 0;
	GraphicsWindow *gw = vpt->getGW();
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	Material *mtl = gw->getMaterial();
	MakeHitRegion(hitRegion,type,crossing,4,p);	
	m = inode->GetObjectTM(t);
	gw->setTransform(m);
	UpdateMesh();
	int res =mesh.select( gw,mtl, &hitRegion, flags & HIT_ABORTONHIT );
	gw->setRndLimits(rlim);
	return res;
}

void LuminaireObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	if (dumFlags&CREATING)	// If creating this one, don't try to snap to it!
		return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);

	UpdateMesh();
	mesh.snap( gw, snap, p, tm );
}

int LuminaireObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
	Matrix3 m;
	if (dumFlags&DISABLE_DISPLAY)
		return 0;
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();
	m = inode->GetObjectTM(t);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())
		gw->setColor( LINE_COLOR, color.x, color.y, color.z);

	UpdateMesh();
	mesh.render( gw, mtl, NULL, COMP_ALL);	
	gw->setRndLimits(rlim);
	return 0;
}

//---------------------------------------------------------------------------
//
void LuminaireObject::SetDimmer(float value, TimeValue time)
{
	DbgAssert(mpBlock != NULL);
	mpBlock->SetValue(kPB_DIMMER, time, value);
}

float LuminaireObject::GetDimmer(TimeValue time, Interval& valid) const
{
	DbgAssert(mpBlock != NULL);
	float value = 0.0f;
	mpBlock->GetValue(kPB_DIMMER, time, value, valid);
	return value;
}

//---------------------------------------------------------------------------
//
void LuminaireObject::SetRGBFilterColor(Point3& value, TimeValue& time)
{
	DbgAssert(mpBlock != NULL);
	mpBlock->SetValue(kPB_FILTER_COLOR, time, value);
}

Point3 LuminaireObject::GetRGBFilterColor(TimeValue& time, Interval& valid) const
{
	DbgAssert(mpBlock != NULL);
	Point3 value = Point3(0.0f, 0.0f, 0.0f);
	mpBlock->GetValue(kPB_FILTER_COLOR, time, value, valid);
	return value;
}

//---------------------------------------------------------------------------
//
void LuminaireObject::SetUseState(bool onOff, TimeValue& time)
{
}
bool LuminaireObject::GetUseState(TimeValue& time, Interval& valid) const
{
	return true;
}

//---------------------------------------------------------------------------
//
BaseInterface* LuminaireObject::GetInterface(Interface_ID id)
{
	if (id == LUMINAIRE_INTERFACE) 
		return static_cast<ILuminaire*>(this); 
	else 
		return DummyObject::GetInterface(id); 
} 

//---------------------------------------------------------------------------
// Create callback related methods
//---------------------------------------------------------------------------
CreateMouseCallBack* LuminaireObject::GetCreateMouseCallBack()
{
	gLuminaireCreateCB.SetObj(this);
	return &gLuminaireCreateCB;
}

//---------------------------------------------------------------------------
//
int LuminaireObjectCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{
#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
#ifdef _3D_CREATE
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
#else
		vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
#endif
	}
#endif // _OSNAP

	if (msg == MOUSE_POINT || msg == MOUSE_MOVE) 
	{
		switch(point) 
		{
			case 0: 
			{
				mpObject->dumFlags |= CREATING;	// tell object we're building it so we can disable snapping to itself
				#ifdef _3D_CREATE	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
				#else	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
				#endif				
				break;
			}

			case 1:
				#ifdef _3D_CREATE	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
				#else	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
				#endif
				if (msg == MOUSE_POINT) 
				{
					mpObject->dumFlags &= ~CREATING;	
					return 0;
				}
				break;			
			}
	} 
	else if (msg == MOUSE_ABORT) 
	{
		mpObject->dumFlags &= ~CREATING;	
		return CREATE_ABORT;
	}

	return TRUE;
}

