/**********************************************************************
 *<
	FILE: Character.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY:		Ravi Karra

	HISTORY:		Feb 06, 2002

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Character.h"
#include "dummy.h"

#define CHARACTER_CLASS_ID	Class_ID(0x43035494, 0x23532b76)
#define CHARACTER_WIRE_COLOR 0x085a4c92

#define PBLOCK_REF	0

// From core\dummy.cpp
#define MESH_VALID 1
#define CREATING 2
#define DISABLE_DISPLAY 4

//===========================================================================
// CharacterObject class
//===========================================================================
class CharacterObject : public DummyObject 
{
	public:
		// --- Constructor/Destructor
		CharacterObject();
		~CharacterObject();		

		// Loading/Saving

		// --- From Animatable
		void						DeleteThis() { delete this; }		
		Class_ID					ClassID() {return CHARACTER_CLASS_ID;}		
		SClass_ID					SuperClassID() { return HELPER_CLASS_ID; }
		void						GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}
		int							NumSubs() { return 1; }
		TSTR						SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable*					SubAnim(int i) { return mpBlock; }

		// TODO: Maintain the number or references here
		int							NumRefs() { return 1; }
		RefTargetHandle				GetReference(int i) { return mpBlock; }
		void						SetReference(int i, RefTargetHandle rtarg);

		int							NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2*				GetParamBlock(int i) { return mpBlock; } // return i'th ParamBlock
		IParamBlock2*				GetParamBlockByID(BlockID id);
		// UI stuff
		void						BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev);
		void						EndEditParams(IObjParam* ip, ULONG flags, Animatable* next);
		// Persistence
		IOResult					Load(ILoad *iload);
		IOResult					Save(ISave *isave);

	    // > 11/3/02 - 11:57pm --MQM-- need to adjust character size when moving to new units
	    void 						RescaleWorldUnits( float f );
		
		// --- From ReferenceTarget
		RefTargetHandle				Clone(RemapDir& remap = NoRemap());
		
		// --- From Object
		void						InitNodeName(TSTR& s) { s = GetString(IDS_CLASS_NAME); } // Default node name

		// --- From BaseObject
		TCHAR*						GetObjectName() { return GetString(IDS_CLASS_NAME); }	// Appears in Modifier Stack
		// Need to re-implement because Character has a different mesh than its base object
		void						GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox );
		void						GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& abox );
		void						GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel=FALSE );
		int							HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void						Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int							Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack*		GetCreateMouseCallBack();

		
		long&						Flags()  { return dumFlags; }

	public:
		// Should not be localized
		static const TCHAR			mInternalClassName[MAX_PATH];
		static const Box3			mDefBoxSize;
		static bool					colorInitialized;

	private:
		// In order to provide a different mesh, we need to redefine these methods
		void						BuildMesh();
		void						UpdateMesh();

		RefResult					NotifyRefChanged(	Interval changeInt, 
																RefTargetHandle hTarget, 
																PartID& partID, 
																RefMessage message );

		int							UsesWireColor()	{ return TRUE; }

		// Parameter block
		IParamBlock2*				mpBlock;	//ref 0
};


//===========================================================================
// Class Desc
//===========================================================================
class CharacterClassDesc : public ClassDesc2 
{
	public:
		int 					IsPublic() { return FALSE; }
		void *					Create(BOOL loading = FALSE) { return new CharacterObject(); }
		const TCHAR*			ClassName() { return GetString(IDS_CLASS_NAME); }
		SClass_ID				SuperClassID() { return HELPER_CLASS_ID; }
		Class_ID				ClassID() { return CHARACTER_CLASS_ID; }
		const TCHAR* 			Category() { return _T("") /*GetString(IDS_CATEGORY)*/; }
		const TCHAR*			InternalName() { return CharacterObject::mInternalClassName; }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE				HInstance() { return hInstance; }				// returns owning module handle
};

static CharacterClassDesc CharacterDesc;
ClassDesc2* GetCharacterDesc() { return &CharacterDesc; }

//===========================================================================
// Param Block  definition
//===========================================================================
enum 
{ 
	kCHR_PARAMS
};

// Add enums for various parameters
enum 
{ 
	kPB_SIZE,
};

class CharacterPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		CharacterObject* chr = (CharacterObject*)owner;
		if (chr!=NULL)
			{
			switch (id)
				{
				case kPB_SIZE:
					{
						Box3 box = CharacterObject::mDefBoxSize;
						box.Scale(v.f);
						chr->SetBox(box);
						chr->Flags() &= ~MESH_VALID;
					break;

					}
				}			
			}

	}
};

static CharacterPBAccessor chr_accessor;


static ParamBlockDesc2 Character_param_blk ( 
	kCHR_PARAMS, _T("params"),  0, GetCharacterDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	
	// rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	// params
	kPB_SIZE,			_T("Size"), 		TYPE_FLOAT, 	0, 	IDS_SIZE_SPIN, 					// > 11/3/02 - 11:56pm --MQM-- maybe use TYPE_WORLD instead of TYPE_FLOAT
		p_default, 		10.0f, 
		p_range, 		1.0f, 1000.0f, 
	p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_SIZE_EDIT,	IDC_SIZE_SPIN, 1.0f, 	// > 11/3/02 - 11:56pm --MQM-- maybe use EDITTYPE_UNIVERSE instead of EDITTYPE_FLOAT
		p_accessor,		&chr_accessor,
		end,
	end
);

//===========================================================================
// CharacterObject create callback
//===========================================================================
class CharacterObjectCreateCallBack: public CreateMouseCallBack 
{
	private:
		CharacterObject*	mpObject;
		IPoint2				mSp0;
		Point3				mP0;

	public:
		int		proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void	SetObj(CharacterObject *obj) { mpObject = obj; }
};

static CharacterObjectCreateCallBack gCharacterCreateCB;


//===========================================================================
// CharacterObject implementation
//===========================================================================
const TCHAR CharacterObject::mInternalClassName[] = _T("CharacterHelper");
const Box3	CharacterObject::mDefBoxSize(Point3(-10, -10, 0), Point3(10, 10, 20));
bool CharacterObject::colorInitialized = false;

//---------------------------------------------------------------------------
// Constructor/Destructor
CharacterObject::CharacterObject() 
: mpBlock(NULL)
{ 
	GetCharacterDesc()->MakeAutoParamBlocks(this);

	if (!colorInitialized)
	{
		colorInitialized = true;
		TSTR name,category;

		name.printf("%s",_T(GetString(IDS_WIRE_COLOR)));
		category.printf("%s",_T(GetString(IDS_CLASS_NAME)));

		// the following color is actually registered by the character plugin script
//		bool iret;
//		iret = ColorMan()->RegisterColor(CHARACTER_WIRE_COLOR, name, category, RGB(220,220,0));
	}

	dumFlags &= ~MESH_VALID;
}

CharacterObject::~CharacterObject() 
{ 
	DeleteAllRefsFromMe();
}

//---------------------------------------------------------------------------
//
void CharacterObject::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
	GetCharacterDesc()->BeginEditParams(ip, this, flags, prev);	
}

//---------------------------------------------------------------------------
//
void CharacterObject::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	GetCharacterDesc()->EndEditParams(ip, this, flags, next);
}

//---------------------------------------------------------------------------
// return id'd ParamBlock
IParamBlock2*	CharacterObject::GetParamBlockByID(BlockID id) 
{ 
	if (mpBlock != NULL && mpBlock->ID() == id)
		return mpBlock; 
	
	return NULL;
} 

//---------------------------------------------------------------------------
//
IOResult CharacterObject::Load(ILoad *iload) 
{ 
	return DummyObject::Load(iload); 
}
//---------------------------------------------------------------------------
//
IOResult CharacterObject::Save(ISave *isave) 
{ 
	return DummyObject::Save(isave); 
}

//---------------------------------------------------------------------------
//
void CharacterObject::SetReference(int i, RefTargetHandle rtarg) 
{ 
	mpBlock = static_cast<IParamBlock2*>(rtarg); 
}

//---------------------------------------------------------------------------
//
RefTargetHandle CharacterObject::Clone(RemapDir& remap) 
{
	CharacterObject* newObj = new CharacterObject();	
	newObj->ReplaceReference(0, mpBlock->Clone(remap));
	BaseClone(this, newObj, remap);
	return newObj;
}

//---------------------------------------------------------------------------
//
RefResult CharacterObject::NotifyRefChanged(	
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID, 
	RefMessage message ) 
{
	return (REF_SUCCEED);
}

// The following methdos had to be redefined because Characters have 
// a different mesh than its base object
void CharacterObject::UpdateMesh() 
{
	if (!(dumFlags&MESH_VALID)) 
	{
		BuildMesh();
		dumFlags |= MESH_VALID;
	}
}
void CharacterObject::BuildMesh()
{
	mesh.setNumVerts(304);
	mesh.setNumFaces(314);

	float s;
	mpBlock->GetValue(kPB_SIZE, GetCOREInterface()->GetTime(), s, FOREVER);

	s /= 10.0f;

	mesh.setVert(0, s * Point3(1.774505,-4.327305,0.000000));
	mesh.setVert(1, s * Point3(1.915621,-4.853958,0.000000));
	mesh.setVert(2, s * Point3(2.064228,-5.408566,0.000000));
	mesh.setVert(3, s * Point3(2.216580,-5.977152,0.000000));
	mesh.setVert(4, s * Point3(2.368933,-6.545739,0.000000));
	mesh.setVert(5, s * Point3(2.517540,-7.100347,0.000000));
	mesh.setVert(6, s * Point3(2.658656,-7.627000,0.000000));
	mesh.setVert(7, s * Point3(2.788536,-8.111719,0.000000));
	mesh.setVert(8, s * Point3(2.811856,-8.255167,0.000000));
	mesh.setVert(9, s * Point3(2.806280,-8.396137,0.000000));
	mesh.setVert(10, s * Point3(2.773860,-8.531079,0.000000));
	mesh.setVert(11, s * Point3(2.716645,-8.656442,0.000000));
	mesh.setVert(12, s * Point3(2.636686,-8.768672,0.000000));
	mesh.setVert(13, s * Point3(2.536032,-8.864221,0.000000));
	mesh.setVert(14, s * Point3(2.416736,-8.939534,0.000000));
	mesh.setVert(15, s * Point3(2.280847,-8.991062,0.000000));
	mesh.setVert(16, s * Point3(2.137399,-9.014382,0.000000));
	mesh.setVert(17, s * Point3(1.996429,-9.008807,0.000000));
	mesh.setVert(18, s * Point3(1.861486,-8.976386,0.000000));
	mesh.setVert(19, s * Point3(1.736124,-8.919171,0.000000));
	mesh.setVert(20, s * Point3(1.623893,-8.839212,0.000000));
	mesh.setVert(21, s * Point3(1.528345,-8.738559,0.000000));
	mesh.setVert(22, s * Point3(1.453031,-8.619263,0.000000));
	mesh.setVert(23, s * Point3(1.401503,-8.483374,0.000000));
	mesh.setVert(24, s * Point3(1.271623,-7.998654,0.000000));
	mesh.setVert(25, s * Point3(1.130507,-7.472002,0.000000));
	mesh.setVert(26, s * Point3(0.981900,-6.917393,0.000000));
	mesh.setVert(27, s * Point3(0.829548,-6.348807,0.000000));
	mesh.setVert(28, s * Point3(0.677196,-5.780221,0.000000));
	mesh.setVert(29, s * Point3(0.528589,-5.225612,0.000000));
	mesh.setVert(30, s * Point3(0.387473,-4.698959,0.000000));
	mesh.setVert(31, s * Point3(0.257593,-4.214240,0.000000));
	mesh.setVert(32, s * Point3(0.234272,-4.070792,0.000000));
	mesh.setVert(33, s * Point3(0.239848,-3.929822,0.000000));
	mesh.setVert(34, s * Point3(0.272268,-3.794880,0.000000));
	mesh.setVert(35, s * Point3(0.329483,-3.669517,0.000000));
	mesh.setVert(36, s * Point3(0.409443,-3.557287,0.000000));
	mesh.setVert(37, s * Point3(0.510096,-3.461738,0.000000));
	mesh.setVert(38, s * Point3(0.629392,-3.386425,0.000000));
	mesh.setVert(39, s * Point3(0.765282,-3.334897,0.000000));
	mesh.setVert(40, s * Point3(0.908729,-3.311576,0.000000));
	mesh.setVert(41, s * Point3(1.049700,-3.317152,0.000000));
	mesh.setVert(42, s * Point3(1.184642,-3.349572,0.000000));
	mesh.setVert(43, s * Point3(1.310004,-3.406787,0.000000));
	mesh.setVert(44, s * Point3(1.422235,-3.486747,0.000000));
	mesh.setVert(45, s * Point3(1.517783,-3.587400,0.000000));
	mesh.setVert(46, s * Point3(1.593097,-3.706697,0.000000));
	mesh.setVert(47, s * Point3(1.644625,-3.842586,0.000000));
	mesh.setVert(48, s * Point3(1.162056,5.503034,0.000000));
	mesh.setVert(49, s * Point3(1.087533,5.263250,0.000000));
	mesh.setVert(50, s * Point3(0.969636,5.046196,0.000000));
	mesh.setVert(51, s * Point3(0.813525,4.857033,0.000000));
	mesh.setVert(52, s * Point3(0.624363,4.700923,0.000000));
	mesh.setVert(53, s * Point3(0.407309,4.583026,0.000000));
	mesh.setVert(54, s * Point3(0.167526,4.508502,0.000000));
	mesh.setVert(55, s * Point3(-0.089827,4.482515,0.000000));
	mesh.setVert(56, s * Point3(-0.347180,4.508503,0.000000));
	mesh.setVert(57, s * Point3(-0.586964,4.583026,0.000000));
	mesh.setVert(58, s * Point3(-0.804017,4.700923,0.000000));
	mesh.setVert(59, s * Point3(-0.993180,4.857034,0.000000));
	mesh.setVert(60, s * Point3(-1.149290,5.046196,0.000000));
	mesh.setVert(61, s * Point3(-1.267188,5.263250,0.000000));
	mesh.setVert(62, s * Point3(-1.341711,5.503033,0.000000));
	mesh.setVert(63, s * Point3(-1.367699,5.760386,0.000000));
	mesh.setVert(64, s * Point3(-1.341711,6.017739,0.000000));
	mesh.setVert(65, s * Point3(-1.267188,6.257523,0.000000));
	mesh.setVert(66, s * Point3(-1.149291,6.474577,0.000000));
	mesh.setVert(67, s * Point3(-0.993180,6.663740,0.000000));
	mesh.setVert(68, s * Point3(-0.804018,6.819850,0.000000));
	mesh.setVert(69, s * Point3(-0.586964,6.937747,0.000000));
	mesh.setVert(70, s * Point3(-0.347180,7.012269,0.000000));
	mesh.setVert(71, s * Point3(-0.089827,7.038258,0.000000));
	mesh.setVert(72, s * Point3(0.167525,7.012269,0.000000));
	mesh.setVert(73, s * Point3(0.407309,6.937747,0.000000));
	mesh.setVert(74, s * Point3(0.624363,6.819849,0.000000));
	mesh.setVert(75, s * Point3(0.813525,6.663739,0.000000));
	mesh.setVert(76, s * Point3(0.969636,6.474576,0.000000));
	mesh.setVert(77, s * Point3(1.087533,6.257522,0.000000));
	mesh.setVert(78, s * Point3(1.162056,6.017739,0.000000));
	mesh.setVert(79, s * Point3(1.188044,5.760387,0.000000));
	mesh.setVert(80, s * Point3(-1.367699,-1.274458,0.000000));
	mesh.setVert(81, s * Point3(-1.367699,-0.769950,0.000000));
	mesh.setVert(82, s * Point3(-1.367699,-0.265443,0.000000));
	mesh.setVert(83, s * Point3(-1.367699,0.239063,0.000000));
	mesh.setVert(84, s * Point3(-1.367699,0.743570,0.000000));
	mesh.setVert(85, s * Point3(-1.367699,1.248078,0.000000));
	mesh.setVert(86, s * Point3(-1.367699,1.752585,0.000000));
	mesh.setVert(87, s * Point3(-1.367699,2.257092,0.000000));
	mesh.setVert(88, s * Point3(-1.341711,2.514445,0.000000));
	mesh.setVert(89, s * Point3(-1.267188,2.754228,0.000000));
	mesh.setVert(90, s * Point3(-1.149291,2.971282,0.000000));
	mesh.setVert(91, s * Point3(-0.993180,3.160444,0.000000));
	mesh.setVert(92, s * Point3(-0.804018,3.316555,0.000000));
	mesh.setVert(93, s * Point3(-0.586964,3.434452,0.000000));
	mesh.setVert(94, s * Point3(-0.347180,3.508975,0.000000));
	mesh.setVert(95, s * Point3(-0.089828,3.534963,0.000000));
	mesh.setVert(96, s * Point3(0.167525,3.508976,0.000000));
	mesh.setVert(97, s * Point3(0.407309,3.434452,0.000000));
	mesh.setVert(98, s * Point3(0.624363,3.316555,0.000000));
	mesh.setVert(99, s * Point3(0.813525,3.160444,0.000000));
	mesh.setVert(100, s * Point3(0.969636,2.971282,0.000000));
	mesh.setVert(101, s * Point3(1.087533,2.754228,0.000000));
	mesh.setVert(102, s * Point3(1.162056,2.514445,0.000000));
	mesh.setVert(103, s * Point3(1.188044,2.257092,0.000000));
	mesh.setVert(104, s * Point3(1.188044,1.752584,0.000000));
	mesh.setVert(105, s * Point3(1.188044,1.248077,0.000000));
	mesh.setVert(106, s * Point3(1.188044,0.743571,0.000000));
	mesh.setVert(107, s * Point3(1.188044,0.239063,0.000000));
	mesh.setVert(108, s * Point3(1.188044,-0.265444,0.000000));
	mesh.setVert(109, s * Point3(1.188044,-0.769951,0.000000));
	mesh.setVert(110, s * Point3(1.188044,-1.274457,0.000000));
	mesh.setVert(111, s * Point3(1.188044,-1.778965,0.000000));
	mesh.setVert(112, s * Point3(1.162056,-2.036317,0.000000));
	mesh.setVert(113, s * Point3(1.087533,-2.276101,0.000000));
	mesh.setVert(114, s * Point3(0.969636,-2.493155,0.000000));
	mesh.setVert(115, s * Point3(0.813525,-2.682317,0.000000));
	mesh.setVert(116, s * Point3(0.624363,-2.838428,0.000000));
	mesh.setVert(117, s * Point3(0.407309,-2.956325,0.000000));
	mesh.setVert(118, s * Point3(0.167525,-3.030848,0.000000));
	mesh.setVert(119, s * Point3(-0.089827,-3.056836,0.000000));
	mesh.setVert(120, s * Point3(-0.347180,-3.030848,0.000000));
	mesh.setVert(121, s * Point3(-0.586964,-2.956325,0.000000));
	mesh.setVert(122, s * Point3(-0.804017,-2.838428,0.000000));
	mesh.setVert(123, s * Point3(-0.993180,-2.682317,0.000000));
	mesh.setVert(124, s * Point3(-1.149291,-2.493155,0.000000));
	mesh.setVert(125, s * Point3(-1.267188,-2.276101,0.000000));
	mesh.setVert(126, s * Point3(-1.341711,-2.036318,0.000000));
	mesh.setVert(127, s * Point3(-1.367699,-1.778965,0.000000));
	mesh.setVert(128, s * Point3(2.166458,3.469748,0.000000));
	mesh.setVert(129, s * Point3(2.610664,3.644725,0.000000));
	mesh.setVert(130, s * Point3(3.096009,3.835908,0.000000));
	mesh.setVert(131, s * Point3(3.601926,4.035194,0.000000));
	mesh.setVert(132, s * Point3(4.107843,4.234480,0.000000));
	mesh.setVert(133, s * Point3(4.593189,4.425662,0.000000));
	mesh.setVert(134, s * Point3(5.037395,4.600640,0.000000));
	mesh.setVert(135, s * Point3(5.419889,4.751308,0.000000));
	mesh.setVert(136, s * Point3(5.528953,4.782034,0.000000));
	mesh.setVert(137, s * Point3(5.638649,4.790157,0.000000));
	mesh.setVert(138, s * Point3(5.746044,4.776953,0.000000));
	mesh.setVert(139, s * Point3(5.848207,4.743695,0.000000));
	mesh.setVert(140, s * Point3(5.942204,4.691660,0.000000));
	mesh.setVert(141, s * Point3(6.025105,4.622121,0.000000));
	mesh.setVert(142, s * Point3(6.093976,4.536355,0.000000));
	mesh.setVert(143, s * Point3(6.145886,4.435636,0.000000));
	mesh.setVert(144, s * Point3(6.176612,4.326571,0.000000));
	mesh.setVert(145, s * Point3(6.184736,4.216876,0.000000));
	mesh.setVert(146, s * Point3(6.171531,4.109480,0.000000));
	mesh.setVert(147, s * Point3(6.138274,4.007318,0.000000));
	mesh.setVert(148, s * Point3(6.086238,3.913321,0.000000));
	mesh.setVert(149, s * Point3(6.016700,3.830420,0.000000));
	mesh.setVert(150, s * Point3(5.930934,3.761548,0.000000));
	mesh.setVert(151, s * Point3(5.830214,3.709638,0.000000));
	mesh.setVert(152, s * Point3(5.447720,3.558970,0.000000));
	mesh.setVert(153, s * Point3(5.003514,3.383992,0.000000));
	mesh.setVert(154, s * Point3(4.518168,3.192810,0.000000));
	mesh.setVert(155, s * Point3(4.012251,2.993523,0.000000));
	mesh.setVert(156, s * Point3(3.506335,2.794237,0.000000));
	mesh.setVert(157, s * Point3(3.020988,2.603055,0.000000));
	mesh.setVert(158, s * Point3(2.576783,2.428078,0.000000));
	mesh.setVert(159, s * Point3(2.194289,2.277409,0.000000));
	mesh.setVert(160, s * Point3(2.085225,2.246683,0.000000));
	mesh.setVert(161, s * Point3(1.975529,2.238560,0.000000));
	mesh.setVert(162, s * Point3(1.868134,2.251765,0.000000));
	mesh.setVert(163, s * Point3(1.765971,2.285022,0.000000));
	mesh.setVert(164, s * Point3(1.671974,2.337057,0.000000));
	mesh.setVert(165, s * Point3(1.589073,2.406595,0.000000));
	mesh.setVert(166, s * Point3(1.520201,2.492362,0.000000));
	mesh.setVert(167, s * Point3(1.468291,2.593082,0.000000));
	mesh.setVert(168, s * Point3(1.437565,2.702146,0.000000));
	mesh.setVert(169, s * Point3(1.429442,2.811842,0.000000));
	mesh.setVert(170, s * Point3(1.442647,2.919237,0.000000));
	mesh.setVert(171, s * Point3(1.475904,3.021399,0.000000));
	mesh.setVert(172, s * Point3(1.527940,3.115397,0.000000));
	mesh.setVert(173, s * Point3(1.597478,3.198298,0.000000));
	mesh.setVert(174, s * Point3(1.683245,3.267169,0.000000));
	mesh.setVert(175, s * Point3(1.783964,3.319079,0.000000));
	mesh.setVert(176, s * Point3(-2.019994,3.319079,0.000000));
	mesh.setVert(177, s * Point3(-1.919274,3.267170,-0.000000));
	mesh.setVert(178, s * Point3(-1.833508,3.198298,-0.000000));
	mesh.setVert(179, s * Point3(-1.763970,3.115398,-0.000000));
	mesh.setVert(180, s * Point3(-1.711934,3.021400,-0.000000));
	mesh.setVert(181, s * Point3(-1.678676,2.919236,-0.000000));
	mesh.setVert(182, s * Point3(-1.665472,2.811841,-0.000000));
	mesh.setVert(183, s * Point3(-1.673595,2.702146,-0.000000));
	mesh.setVert(184, s * Point3(-1.704321,2.593082,-0.000000));
	mesh.setVert(185, s * Point3(-1.756231,2.492362,-0.000000));
	mesh.setVert(186, s * Point3(-1.825103,2.406596,-0.000000));
	mesh.setVert(187, s * Point3(-1.908003,2.337057,-0.000000));
	mesh.setVert(188, s * Point3(-2.002001,2.285022,-0.000000));
	mesh.setVert(189, s * Point3(-2.104164,2.251764,-0.000000));
	mesh.setVert(190, s * Point3(-2.211559,2.238560,-0.000000));
	mesh.setVert(191, s * Point3(-2.321254,2.246683,-0.000000));
	mesh.setVert(192, s * Point3(-2.430319,2.277409,0.000000));
	mesh.setVert(193, s * Point3(-2.812813,2.428078,0.000000));
	mesh.setVert(194, s * Point3(-3.257019,2.603055,0.000000));
	mesh.setVert(195, s * Point3(-3.742365,2.794237,0.000000));
	mesh.setVert(196, s * Point3(-4.248281,2.993524,0.000000));
	mesh.setVert(197, s * Point3(-4.754198,3.192810,0.000000));
	mesh.setVert(198, s * Point3(-5.239544,3.383993,0.000000));
	mesh.setVert(199, s * Point3(-5.683749,3.558970,0.000000));
	mesh.setVert(200, s * Point3(-6.066244,3.709639,0.000000));
	mesh.setVert(201, s * Point3(-6.166963,3.761550,0.000000));
	mesh.setVert(202, s * Point3(-6.252730,3.830420,0.000000));
	mesh.setVert(203, s * Point3(-6.322268,3.913321,0.000000));
	mesh.setVert(204, s * Point3(-6.374303,4.007318,0.000000));
	mesh.setVert(205, s * Point3(-6.407561,4.109482,0.000000));
	mesh.setVert(206, s * Point3(-6.420765,4.216877,0.000000));
	mesh.setVert(207, s * Point3(-6.412642,4.326572,0.000000));
	mesh.setVert(208, s * Point3(-6.381916,4.435636,0.000000));
	mesh.setVert(209, s * Point3(-6.330006,4.536355,0.000000));
	mesh.setVert(210, s * Point3(-6.261134,4.622123,0.000000));
	mesh.setVert(211, s * Point3(-6.178234,4.691660,0.000000));
	mesh.setVert(212, s * Point3(-6.084236,4.743697,0.000000));
	mesh.setVert(213, s * Point3(-5.982073,4.776953,0.000000));
	mesh.setVert(214, s * Point3(-5.874679,4.790159,0.000000));
	mesh.setVert(215, s * Point3(-5.764983,4.782035,0.000000));
	mesh.setVert(216, s * Point3(-5.655919,4.751309,0.000000));
	mesh.setVert(217, s * Point3(-5.273425,4.600642,0.000000));
	mesh.setVert(218, s * Point3(-4.829219,4.425664,0.000000));
	mesh.setVert(219, s * Point3(-4.343873,4.234480,0.000000));
	mesh.setVert(220, s * Point3(-3.837956,4.035194,0.000000));
	mesh.setVert(221, s * Point3(-3.332040,3.835909,0.000000));
	mesh.setVert(222, s * Point3(-2.846693,3.644725,0.000000));
	mesh.setVert(223, s * Point3(-2.402488,3.469748,0.000000));
	mesh.setVert(224, s * Point3(-1.880656,-3.842586,-0.000000));
	mesh.setVert(225, s * Point3(-1.829127,-3.706697,-0.000000));
	mesh.setVert(226, s * Point3(-1.753814,-3.587400,-0.000000));
	mesh.setVert(227, s * Point3(-1.658266,-3.486747,-0.000000));
	mesh.setVert(228, s * Point3(-1.546035,-3.406788,-0.000000));
	mesh.setVert(229, s * Point3(-1.420673,-3.349573,-0.000000));
	mesh.setVert(230, s * Point3(-1.285730,-3.317152,-0.000000));
	mesh.setVert(231, s * Point3(-1.144760,-3.311577,-0.000000));
	mesh.setVert(232, s * Point3(-1.001312,-3.334897,-0.000000));
	mesh.setVert(233, s * Point3(-0.865423,-3.386425,-0.000000));
	mesh.setVert(234, s * Point3(-0.746127,-3.461739,-0.000000));
	mesh.setVert(235, s * Point3(-0.645474,-3.557287,-0.000000));
	mesh.setVert(236, s * Point3(-0.565514,-3.669518,-0.000000));
	mesh.setVert(237, s * Point3(-0.508299,-3.794879,-0.000000));
	mesh.setVert(238, s * Point3(-0.475879,-3.929822,-0.000000));
	mesh.setVert(239, s * Point3(-0.470303,-4.070792,-0.000000));
	mesh.setVert(240, s * Point3(-0.493623,-4.214240,-0.000000));
	mesh.setVert(241, s * Point3(-0.623503,-4.698959,-0.000000));
	mesh.setVert(242, s * Point3(-0.764620,-5.225612,0.000000));
	mesh.setVert(243, s * Point3(-0.913226,-5.780220,0.000000));
	mesh.setVert(244, s * Point3(-1.065579,-6.348806,0.000000));
	mesh.setVert(245, s * Point3(-1.217931,-6.917393,0.000000));
	mesh.setVert(246, s * Point3(-1.366538,-7.472001,0.000000));
	mesh.setVert(247, s * Point3(-1.507654,-7.998654,0.000000));
	mesh.setVert(248, s * Point3(-1.637534,-8.483373,0.000000));
	mesh.setVert(249, s * Point3(-1.689062,-8.619263,0.000000));
	mesh.setVert(250, s * Point3(-1.764376,-8.738559,0.000000));
	mesh.setVert(251, s * Point3(-1.859924,-8.839211,0.000000));
	mesh.setVert(252, s * Point3(-1.972154,-8.919171,0.000000));
	mesh.setVert(253, s * Point3(-2.097517,-8.976386,0.000000));
	mesh.setVert(254, s * Point3(-2.232459,-9.008806,0.000000));
	mesh.setVert(255, s * Point3(-2.373429,-9.014381,0.000000));
	mesh.setVert(256, s * Point3(-2.516877,-8.991062,0.000000));
	mesh.setVert(257, s * Point3(-2.652766,-8.939533,0.000000));
	mesh.setVert(258, s * Point3(-2.772063,-8.864220,0.000000));
	mesh.setVert(259, s * Point3(-2.872716,-8.768672,0.000000));
	mesh.setVert(260, s * Point3(-2.952675,-8.656441,0.000000));
	mesh.setVert(261, s * Point3(-3.009890,-8.531078,0.000000));
	mesh.setVert(262, s * Point3(-3.042311,-8.396136,0.000000));
	mesh.setVert(263, s * Point3(-3.047886,-8.255166,0.000000));
	mesh.setVert(264, s * Point3(-3.024566,-8.111718,0.000000));
	mesh.setVert(265, s * Point3(-2.894686,-7.626999,0.000000));
	mesh.setVert(266, s * Point3(-2.753570,-7.100347,0.000000));
	mesh.setVert(267, s * Point3(-2.604963,-6.545738,0.000000));
	mesh.setVert(268, s * Point3(-2.452611,-5.977152,0.000000));
	mesh.setVert(269, s * Point3(-2.300259,-5.408566,0.000000));
	mesh.setVert(270, s * Point3(-2.151652,-4.853958,0.000000));
	mesh.setVert(271, s * Point3(-2.010536,-4.327305,-0.000000));
	mesh.setVert(272, s * Point3(9.608726,-0.429434,0.000000));
	mesh.setVert(273, s * Point3(9.410916,1.529427,0.000000));
	mesh.setVert(274, s * Point3(8.843678,3.354559,0.000000));
	mesh.setVert(275, s * Point3(7.946293,5.006680,0.000000));
	mesh.setVert(276, s * Point3(6.758047,6.446506,0.000000));
	mesh.setVert(277, s * Point3(5.318222,7.634751,0.000000));
	mesh.setVert(278, s * Point3(3.666101,8.532136,0.000000));
	mesh.setVert(279, s * Point3(1.840969,9.099375,0.000000));
	mesh.setVert(280, s * Point3(-0.117892,9.297184,0.000000));
	mesh.setVert(281, s * Point3(-2.076752,9.099375,0.000000));
	mesh.setVert(282, s * Point3(-3.901885,8.532136,0.000000));
	mesh.setVert(283, s * Point3(-5.554005,7.634751,0.000000));
	mesh.setVert(284, s * Point3(-6.993831,6.446505,0.000000));
	mesh.setVert(285, s * Point3(-8.182077,5.006679,0.000000));
	mesh.setVert(286, s * Point3(-9.079461,3.354558,0.000000));
	mesh.setVert(287, s * Point3(-9.646700,1.529426,0.000000));
	mesh.setVert(288, s * Point3(-9.844508,-0.429434,0.000000));
	mesh.setVert(289, s * Point3(-9.646699,-2.388294,0.000000));
	mesh.setVert(290, s * Point3(-9.079459,-4.213426,0.000000));
	mesh.setVert(291, s * Point3(-8.182076,-5.865547,0.000000));
	mesh.setVert(292, s * Point3(-6.993830,-7.305372,0.000000));
	mesh.setVert(293, s * Point3(-5.554005,-8.493619,0.000000));
	mesh.setVert(294, s * Point3(-3.901884,-9.391003,0.000000));
	mesh.setVert(295, s * Point3(-2.076752,-9.958241,0.000000));
	mesh.setVert(296, s * Point3(-0.117892,-10.156051,0.000000));
	mesh.setVert(297, s * Point3(1.840969,-9.958241,0.000000));
	mesh.setVert(298, s * Point3(3.666101,-9.391004,0.000000));
	mesh.setVert(299, s * Point3(5.318222,-8.493618,0.000000));
	mesh.setVert(300, s * Point3(6.758047,-7.305372,0.000000));
	mesh.setVert(301, s * Point3(7.946294,-5.865547,0.000000));
	mesh.setVert(302, s * Point3(8.843678,-4.213426,0.000000));
	mesh.setVert(303, s * Point3(9.410916,-2.388294,0.000000));
	Face f;

	f.v[0] = 119;  f.v[1] = 23;  f.v[2] = 24;  	f.smGroup = 1;  f.flags = 66; mesh.faces[0] = f;
	f.v[0] = 119;  f.v[1] = 24;  f.v[2] = 25;  	f.smGroup = 1;  f.flags = 66; mesh.faces[1] = f;
	f.v[0] = 119;  f.v[1] = 25;  f.v[2] = 26;  	f.smGroup = 1;  f.flags = 66; mesh.faces[2] = f;
	f.v[0] = 119;  f.v[1] = 26;  f.v[2] = 27;  	f.smGroup = 1;  f.flags = 66; mesh.faces[3] = f;
	f.v[0] = 119;  f.v[1] = 27;  f.v[2] = 28;  	f.smGroup = 1;  f.flags = 66; mesh.faces[4] = f;
	f.v[0] = 119;  f.v[1] = 28;  f.v[2] = 29;  	f.smGroup = 1;  f.flags = 66; mesh.faces[5] = f;
	f.v[0] = 119;  f.v[1] = 29;  f.v[2] = 30;  	f.smGroup = 1;  f.flags = 66; mesh.faces[6] = f;
	f.v[0] = 119;  f.v[1] = 30;  f.v[2] = 31;  	f.smGroup = 1;  f.flags = 66; mesh.faces[7] = f;
	f.v[0] = 184;  f.v[1] = 127;  f.v[2] = 80;  	f.smGroup = 1;  f.flags = 66; mesh.faces[8] = f;
	f.v[0] = 184;  f.v[1] = 80;  f.v[2] = 81;  	f.smGroup = 1;  f.flags = 66; mesh.faces[9] = f;
	f.v[0] = 184;  f.v[1] = 81;  f.v[2] = 82;  	f.smGroup = 1;  f.flags = 66; mesh.faces[10] = f;
	f.v[0] = 184;  f.v[1] = 82;  f.v[2] = 83;  	f.smGroup = 1;  f.flags = 66; mesh.faces[11] = f;
	f.v[0] = 119;  f.v[1] = 31;  f.v[2] = 32;  	f.smGroup = 1;  f.flags = 66; mesh.faces[12] = f;
	f.v[0] = 184;  f.v[1] = 83;  f.v[2] = 84;  	f.smGroup = 1;  f.flags = 66; mesh.faces[13] = f;
	f.v[0] = 47;  f.v[1] = 79;  f.v[2] = 48;  	f.smGroup = 1;  f.flags = 66; mesh.faces[14] = f;
	f.v[0] = 184;  f.v[1] = 84;  f.v[2] = 85;  	f.smGroup = 1;  f.flags = 66; mesh.faces[15] = f;
	f.v[0] = 184;  f.v[1] = 85;  f.v[2] = 86;  	f.smGroup = 1;  f.flags = 66; mesh.faces[16] = f;
	f.v[0] = 46;  f.v[1] = 47;  f.v[2] = 48;  	f.smGroup = 1;  f.flags = 65; mesh.faces[17] = f;
	f.v[0] = 46;  f.v[1] = 48;  f.v[2] = 49;  	f.smGroup = 1;  f.flags = 66; mesh.faces[18] = f;
	f.v[0] = 119;  f.v[1] = 32;  f.v[2] = 33;  	f.smGroup = 1;  f.flags = 66; mesh.faces[19] = f;
	f.v[0] = 184;  f.v[1] = 86;  f.v[2] = 87;  	f.smGroup = 1;  f.flags = 66; mesh.faces[20] = f;
	f.v[0] = 192;  f.v[1] = 224;  f.v[2] = 225;  	f.smGroup = 1;  f.flags = 66; mesh.faces[21] = f;
	f.v[0] = 45;  f.v[1] = 46;  f.v[2] = 49;  	f.smGroup = 1;  f.flags = 65; mesh.faces[22] = f;
	f.v[0] = 127;  f.v[1] = 184;  f.v[2] = 185;  	f.smGroup = 1;  f.flags = 66; mesh.faces[23] = f;
	f.v[0] = 279;  f.v[1] = 280;  f.v[2] = 135;  	f.smGroup = 1;  f.flags = 65; mesh.faces[24] = f;
	f.v[0] = 278;  f.v[1] = 279;  f.v[2] = 135;  	f.smGroup = 1;  f.flags = 65; mesh.faces[25] = f;
	f.v[0] = 119;  f.v[1] = 33;  f.v[2] = 34;  	f.smGroup = 1;  f.flags = 66; mesh.faces[26] = f;
	f.v[0] = 303;  f.v[1] = 272;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[27] = f;
	f.v[0] = 302;  f.v[1] = 303;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[28] = f;
	f.v[0] = 301;  f.v[1] = 302;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[29] = f;
	f.v[0] = 300;  f.v[1] = 301;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[30] = f;
	f.v[0] = 299;  f.v[1] = 300;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[31] = f;
	f.v[0] = 277;  f.v[1] = 278;  f.v[2] = 135;  	f.smGroup = 1;  f.flags = 65; mesh.faces[32] = f;
	f.v[0] = 298;  f.v[1] = 299;  f.v[2] = 7;  	f.smGroup = 1;  f.flags = 65; mesh.faces[33] = f;
	f.v[0] = 298;  f.v[1] = 7;  f.v[2] = 8;  	f.smGroup = 1;  f.flags = 66; mesh.faces[34] = f;
	f.v[0] = 192;  f.v[1] = 225;  f.v[2] = 226;  	f.smGroup = 1;  f.flags = 66; mesh.faces[35] = f;
	f.v[0] = 298;  f.v[1] = 8;  f.v[2] = 9;  	f.smGroup = 1;  f.flags = 66; mesh.faces[36] = f;
	f.v[0] = 44;  f.v[1] = 45;  f.v[2] = 49;  	f.smGroup = 1;  f.flags = 65; mesh.faces[37] = f;
	f.v[0] = 127;  f.v[1] = 185;  f.v[2] = 186;  	f.smGroup = 1;  f.flags = 66; mesh.faces[38] = f;
	f.v[0] = 276;  f.v[1] = 277;  f.v[2] = 135;  	f.smGroup = 1;  f.flags = 65; mesh.faces[39] = f;
	f.v[0] = 276;  f.v[1] = 135;  f.v[2] = 136;  	f.smGroup = 1;  f.flags = 66; mesh.faces[40] = f;
	f.v[0] = 276;  f.v[1] = 136;  f.v[2] = 137;  	f.smGroup = 1;  f.flags = 66; mesh.faces[41] = f;
	f.v[0] = 192;  f.v[1] = 226;  f.v[2] = 227;  	f.smGroup = 1;  f.flags = 66; mesh.faces[42] = f;
	f.v[0] = 119;  f.v[1] = 34;  f.v[2] = 35;  	f.smGroup = 1;  f.flags = 66; mesh.faces[43] = f;
	f.v[0] = 184;  f.v[1] = 87;  f.v[2] = 88;  	f.smGroup = 1;  f.flags = 66; mesh.faces[44] = f;
	f.v[0] = 298;  f.v[1] = 9;  f.v[2] = 10;  	f.smGroup = 1;  f.flags = 66; mesh.faces[45] = f;
	f.v[0] = 127;  f.v[1] = 186;  f.v[2] = 187;  	f.smGroup = 1;  f.flags = 66; mesh.faces[46] = f;
	f.v[0] = 6;  f.v[1] = 7;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[47] = f;
	f.v[0] = 5;  f.v[1] = 6;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[48] = f;
	f.v[0] = 134;  f.v[1] = 135;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[49] = f;
	f.v[0] = 118;  f.v[1] = 119;  f.v[2] = 35;  	f.smGroup = 1;  f.flags = 65; mesh.faces[50] = f;
	f.v[0] = 118;  f.v[1] = 35;  f.v[2] = 36;  	f.smGroup = 1;  f.flags = 66; mesh.faces[51] = f;
	f.v[0] = 4;  f.v[1] = 5;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[52] = f;
	f.v[0] = 192;  f.v[1] = 227;  f.v[2] = 228;  	f.smGroup = 1;  f.flags = 66; mesh.faces[53] = f;
	f.v[0] = 276;  f.v[1] = 137;  f.v[2] = 138;  	f.smGroup = 1;  f.flags = 66; mesh.faces[54] = f;
	f.v[0] = 133;  f.v[1] = 134;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[55] = f;
	f.v[0] = 3;  f.v[1] = 4;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[56] = f;
	f.v[0] = 191;  f.v[1] = 192;  f.v[2] = 228;  	f.smGroup = 1;  f.flags = 65; mesh.faces[57] = f;
	f.v[0] = 132;  f.v[1] = 133;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[58] = f;
	f.v[0] = 2;  f.v[1] = 3;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[59] = f;
	f.v[0] = 127;  f.v[1] = 187;  f.v[2] = 188;  	f.smGroup = 1;  f.flags = 66; mesh.faces[60] = f;
	f.v[0] = 298;  f.v[1] = 10;  f.v[2] = 11;  	f.smGroup = 1;  f.flags = 66; mesh.faces[61] = f;
	f.v[0] = 297;  f.v[1] = 298;  f.v[2] = 11;  	f.smGroup = 1;  f.flags = 65; mesh.faces[62] = f;
	f.v[0] = 297;  f.v[1] = 11;  f.v[2] = 12;  	f.smGroup = 1;  f.flags = 66; mesh.faces[63] = f;
	f.v[0] = 297;  f.v[1] = 12;  f.v[2] = 13;  	f.smGroup = 1;  f.flags = 66; mesh.faces[64] = f;
	f.v[0] = 297;  f.v[1] = 13;  f.v[2] = 14;  	f.smGroup = 1;  f.flags = 66; mesh.faces[65] = f;
	f.v[0] = 297;  f.v[1] = 14;  f.v[2] = 15;  	f.smGroup = 1;  f.flags = 66; mesh.faces[66] = f;
	f.v[0] = 297;  f.v[1] = 15;  f.v[2] = 16;  	f.smGroup = 1;  f.flags = 66; mesh.faces[67] = f;
	f.v[0] = 275;  f.v[1] = 276;  f.v[2] = 138;  	f.smGroup = 1;  f.flags = 65; mesh.faces[68] = f;
	f.v[0] = 275;  f.v[1] = 138;  f.v[2] = 139;  	f.smGroup = 1;  f.flags = 66; mesh.faces[69] = f;
	f.v[0] = 275;  f.v[1] = 139;  f.v[2] = 140;  	f.smGroup = 1;  f.flags = 66; mesh.faces[70] = f;
	f.v[0] = 275;  f.v[1] = 140;  f.v[2] = 141;  	f.smGroup = 1;  f.flags = 66; mesh.faces[71] = f;
	f.v[0] = 275;  f.v[1] = 141;  f.v[2] = 142;  	f.smGroup = 1;  f.flags = 66; mesh.faces[72] = f;
	f.v[0] = 118;  f.v[1] = 36;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 66; mesh.faces[73] = f;
	f.v[0] = 117;  f.v[1] = 118;  f.v[2] = 37;  	f.smGroup = 1;  f.flags = 65; mesh.faces[74] = f;
	f.v[0] = 117;  f.v[1] = 37;  f.v[2] = 38;  	f.smGroup = 1;  f.flags = 66; mesh.faces[75] = f;
	f.v[0] = 1;  f.v[1] = 2;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[76] = f;
	f.v[0] = 131;  f.v[1] = 132;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[77] = f;
	f.v[0] = 191;  f.v[1] = 228;  f.v[2] = 229;  	f.smGroup = 1;  f.flags = 66; mesh.faces[78] = f;
	f.v[0] = 297;  f.v[1] = 16;  f.v[2] = 17;  	f.smGroup = 1;  f.flags = 66; mesh.faces[79] = f;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[80] = f;
	f.v[0] = 130;  f.v[1] = 131;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[81] = f;
	f.v[0] = 190;  f.v[1] = 191;  f.v[2] = 229;  	f.smGroup = 1;  f.flags = 65; mesh.faces[82] = f;
	f.v[0] = 275;  f.v[1] = 142;  f.v[2] = 143;  	f.smGroup = 1;  f.flags = 66; mesh.faces[83] = f;
	f.v[0] = 47;  f.v[1] = 0;  f.v[2] = 272;  	f.smGroup = 1;  f.flags = 65; mesh.faces[84] = f;
	f.v[0] = 127;  f.v[1] = 188;  f.v[2] = 189;  	f.smGroup = 1;  f.flags = 66; mesh.faces[85] = f;
	f.v[0] = 129;  f.v[1] = 130;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[86] = f;
	f.v[0] = 190;  f.v[1] = 229;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 66; mesh.faces[87] = f;
	f.v[0] = 183;  f.v[1] = 184;  f.v[2] = 88;  	f.smGroup = 1;  f.flags = 65; mesh.faces[88] = f;
	f.v[0] = 183;  f.v[1] = 88;  f.v[2] = 89;  	f.smGroup = 1;  f.flags = 66; mesh.faces[89] = f;
	f.v[0] = 182;  f.v[1] = 183;  f.v[2] = 89;  	f.smGroup = 1;  f.flags = 65; mesh.faces[90] = f;
	f.v[0] = 189;  f.v[1] = 190;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 65; mesh.faces[91] = f;
	f.v[0] = 127;  f.v[1] = 189;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 64; mesh.faces[92] = f;
	f.v[0] = 126;  f.v[1] = 127;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 65; mesh.faces[93] = f;
	f.v[0] = 125;  f.v[1] = 126;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 65; mesh.faces[94] = f;
	f.v[0] = 124;  f.v[1] = 125;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 65; mesh.faces[95] = f;
	f.v[0] = 123;  f.v[1] = 124;  f.v[2] = 230;  	f.smGroup = 1;  f.flags = 65; mesh.faces[96] = f;
	f.v[0] = 123;  f.v[1] = 230;  f.v[2] = 231;  	f.smGroup = 1;  f.flags = 66; mesh.faces[97] = f;
	f.v[0] = 122;  f.v[1] = 123;  f.v[2] = 231;  	f.smGroup = 1;  f.flags = 65; mesh.faces[98] = f;
	f.v[0] = 122;  f.v[1] = 231;  f.v[2] = 232;  	f.smGroup = 1;  f.flags = 66; mesh.faces[99] = f;
	f.v[0] = 121;  f.v[1] = 122;  f.v[2] = 232;  	f.smGroup = 1;  f.flags = 65; mesh.faces[100] = f;
	f.v[0] = 121;  f.v[1] = 232;  f.v[2] = 233;  	f.smGroup = 1;  f.flags = 66; mesh.faces[101] = f;
	f.v[0] = 121;  f.v[1] = 233;  f.v[2] = 234;  	f.smGroup = 1;  f.flags = 66; mesh.faces[102] = f;
	f.v[0] = 120;  f.v[1] = 121;  f.v[2] = 234;  	f.smGroup = 1;  f.flags = 65; mesh.faces[103] = f;
	f.v[0] = 120;  f.v[1] = 234;  f.v[2] = 235;  	f.smGroup = 1;  f.flags = 66; mesh.faces[104] = f;
	f.v[0] = 116;  f.v[1] = 117;  f.v[2] = 38;  	f.smGroup = 1;  f.flags = 65; mesh.faces[105] = f;
	f.v[0] = 116;  f.v[1] = 38;  f.v[2] = 39;  	f.smGroup = 1;  f.flags = 66; mesh.faces[106] = f;
	f.v[0] = 275;  f.v[1] = 143;  f.v[2] = 144;  	f.smGroup = 1;  f.flags = 66; mesh.faces[107] = f;
	f.v[0] = 297;  f.v[1] = 17;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 66; mesh.faces[108] = f;
	f.v[0] = 116;  f.v[1] = 39;  f.v[2] = 40;  	f.smGroup = 1;  f.flags = 66; mesh.faces[109] = f;
	f.v[0] = 296;  f.v[1] = 297;  f.v[2] = 18;  	f.smGroup = 1;  f.flags = 65; mesh.faces[110] = f;
	f.v[0] = 296;  f.v[1] = 18;  f.v[2] = 19;  	f.smGroup = 1;  f.flags = 66; mesh.faces[111] = f;
	f.v[0] = 296;  f.v[1] = 19;  f.v[2] = 20;  	f.smGroup = 1;  f.flags = 66; mesh.faces[112] = f;
	f.v[0] = 296;  f.v[1] = 20;  f.v[2] = 21;  	f.smGroup = 1;  f.flags = 66; mesh.faces[113] = f;
	f.v[0] = 274;  f.v[1] = 275;  f.v[2] = 144;  	f.smGroup = 1;  f.flags = 65; mesh.faces[114] = f;
	f.v[0] = 274;  f.v[1] = 144;  f.v[2] = 145;  	f.smGroup = 1;  f.flags = 66; mesh.faces[115] = f;
	f.v[0] = 274;  f.v[1] = 145;  f.v[2] = 146;  	f.smGroup = 1;  f.flags = 66; mesh.faces[116] = f;
	f.v[0] = 274;  f.v[1] = 146;  f.v[2] = 147;  	f.smGroup = 1;  f.flags = 66; mesh.faces[117] = f;
	f.v[0] = 296;  f.v[1] = 21;  f.v[2] = 22;  	f.smGroup = 1;  f.flags = 66; mesh.faces[118] = f;
	f.v[0] = 115;  f.v[1] = 116;  f.v[2] = 40;  	f.smGroup = 1;  f.flags = 65; mesh.faces[119] = f;
	f.v[0] = 115;  f.v[1] = 40;  f.v[2] = 41;  	f.smGroup = 1;  f.flags = 66; mesh.faces[120] = f;
	f.v[0] = 181;  f.v[1] = 182;  f.v[2] = 89;  	f.smGroup = 1;  f.flags = 65; mesh.faces[121] = f;
	f.v[0] = 181;  f.v[1] = 89;  f.v[2] = 90;  	f.smGroup = 1;  f.flags = 66; mesh.faces[122] = f;
	f.v[0] = 180;  f.v[1] = 181;  f.v[2] = 90;  	f.smGroup = 1;  f.flags = 65; mesh.faces[123] = f;
	f.v[0] = 274;  f.v[1] = 147;  f.v[2] = 148;  	f.smGroup = 1;  f.flags = 66; mesh.faces[124] = f;
	f.v[0] = 47;  f.v[1] = 272;  f.v[2] = 273;  	f.smGroup = 1;  f.flags = 66; mesh.faces[125] = f;
	f.v[0] = 47;  f.v[1] = 273;  f.v[2] = 274;  	f.smGroup = 1;  f.flags = 66; mesh.faces[126] = f;
	f.v[0] = 47;  f.v[1] = 274;  f.v[2] = 148;  	f.smGroup = 1;  f.flags = 64; mesh.faces[127] = f;
	f.v[0] = 47;  f.v[1] = 148;  f.v[2] = 149;  	f.smGroup = 1;  f.flags = 66; mesh.faces[128] = f;
	f.v[0] = 47;  f.v[1] = 149;  f.v[2] = 150;  	f.smGroup = 1;  f.flags = 66; mesh.faces[129] = f;
	f.v[0] = 47;  f.v[1] = 150;  f.v[2] = 151;  	f.smGroup = 1;  f.flags = 66; mesh.faces[130] = f;
	f.v[0] = 47;  f.v[1] = 151;  f.v[2] = 152;  	f.smGroup = 1;  f.flags = 66; mesh.faces[131] = f;
	f.v[0] = 47;  f.v[1] = 152;  f.v[2] = 153;  	f.smGroup = 1;  f.flags = 66; mesh.faces[132] = f;
	f.v[0] = 47;  f.v[1] = 153;  f.v[2] = 154;  	f.smGroup = 1;  f.flags = 66; mesh.faces[133] = f;
	f.v[0] = 47;  f.v[1] = 154;  f.v[2] = 155;  	f.smGroup = 1;  f.flags = 66; mesh.faces[134] = f;
	f.v[0] = 47;  f.v[1] = 155;  f.v[2] = 156;  	f.smGroup = 1;  f.flags = 66; mesh.faces[135] = f;
	f.v[0] = 47;  f.v[1] = 156;  f.v[2] = 157;  	f.smGroup = 1;  f.flags = 66; mesh.faces[136] = f;
	f.v[0] = 47;  f.v[1] = 157;  f.v[2] = 158;  	f.smGroup = 1;  f.flags = 66; mesh.faces[137] = f;
	f.v[0] = 47;  f.v[1] = 158;  f.v[2] = 159;  	f.smGroup = 1;  f.flags = 66; mesh.faces[138] = f;
	f.v[0] = 47;  f.v[1] = 159;  f.v[2] = 160;  	f.smGroup = 1;  f.flags = 66; mesh.faces[139] = f;
	f.v[0] = 47;  f.v[1] = 160;  f.v[2] = 161;  	f.smGroup = 1;  f.flags = 66; mesh.faces[140] = f;
	f.v[0] = 47;  f.v[1] = 161;  f.v[2] = 162;  	f.smGroup = 1;  f.flags = 66; mesh.faces[141] = f;
	f.v[0] = 47;  f.v[1] = 162;  f.v[2] = 163;  	f.smGroup = 1;  f.flags = 66; mesh.faces[142] = f;
	f.v[0] = 23;  f.v[1] = 119;  f.v[2] = 120;  	f.smGroup = 1;  f.flags = 66; mesh.faces[143] = f;
	f.v[0] = 296;  f.v[1] = 22;  f.v[2] = 23;  	f.smGroup = 1;  f.flags = 66; mesh.faces[144] = f;
	f.v[0] = 120;  f.v[1] = 235;  f.v[2] = 236;  	f.smGroup = 1;  f.flags = 66; mesh.faces[145] = f;
	f.v[0] = 224;  f.v[1] = 192;  f.v[2] = 193;  	f.smGroup = 1;  f.flags = 66; mesh.faces[146] = f;
	f.v[0] = 47;  f.v[1] = 163;  f.v[2] = 164;  	f.smGroup = 1;  f.flags = 66; mesh.faces[147] = f;
	f.v[0] = 224;  f.v[1] = 193;  f.v[2] = 194;  	f.smGroup = 1;  f.flags = 66; mesh.faces[148] = f;
	f.v[0] = 296;  f.v[1] = 23;  f.v[2] = 120;  	f.smGroup = 1;  f.flags = 64; mesh.faces[149] = f;
	f.v[0] = 114;  f.v[1] = 115;  f.v[2] = 41;  	f.smGroup = 1;  f.flags = 65; mesh.faces[150] = f;
	f.v[0] = 114;  f.v[1] = 41;  f.v[2] = 42;  	f.smGroup = 1;  f.flags = 66; mesh.faces[151] = f;
	f.v[0] = 224;  f.v[1] = 194;  f.v[2] = 195;  	f.smGroup = 1;  f.flags = 66; mesh.faces[152] = f;
	f.v[0] = 179;  f.v[1] = 180;  f.v[2] = 90;  	f.smGroup = 1;  f.flags = 65; mesh.faces[153] = f;
	f.v[0] = 179;  f.v[1] = 90;  f.v[2] = 91;  	f.smGroup = 1;  f.flags = 66; mesh.faces[154] = f;
	f.v[0] = 178;  f.v[1] = 179;  f.v[2] = 91;  	f.smGroup = 1;  f.flags = 65; mesh.faces[155] = f;
	f.v[0] = 224;  f.v[1] = 195;  f.v[2] = 196;  	f.smGroup = 1;  f.flags = 66; mesh.faces[156] = f;
	f.v[0] = 114;  f.v[1] = 42;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 66; mesh.faces[157] = f;
	f.v[0] = 47;  f.v[1] = 164;  f.v[2] = 165;  	f.smGroup = 1;  f.flags = 66; mesh.faces[158] = f;
	f.v[0] = 224;  f.v[1] = 196;  f.v[2] = 197;  	f.smGroup = 1;  f.flags = 66; mesh.faces[159] = f;
	f.v[0] = 113;  f.v[1] = 114;  f.v[2] = 43;  	f.smGroup = 1;  f.flags = 65; mesh.faces[160] = f;
	f.v[0] = 224;  f.v[1] = 197;  f.v[2] = 198;  	f.smGroup = 1;  f.flags = 66; mesh.faces[161] = f;
	f.v[0] = 224;  f.v[1] = 198;  f.v[2] = 199;  	f.smGroup = 1;  f.flags = 66; mesh.faces[162] = f;
	f.v[0] = 120;  f.v[1] = 236;  f.v[2] = 237;  	f.smGroup = 1;  f.flags = 66; mesh.faces[163] = f;
	f.v[0] = 113;  f.v[1] = 43;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 66; mesh.faces[164] = f;
	f.v[0] = 271;  f.v[1] = 224;  f.v[2] = 199;  	f.smGroup = 1;  f.flags = 65; mesh.faces[165] = f;
	f.v[0] = 271;  f.v[1] = 199;  f.v[2] = 200;  	f.smGroup = 1;  f.flags = 66; mesh.faces[166] = f;
	f.v[0] = 178;  f.v[1] = 91;  f.v[2] = 92;  	f.smGroup = 1;  f.flags = 66; mesh.faces[167] = f;
	f.v[0] = 177;  f.v[1] = 178;  f.v[2] = 92;  	f.smGroup = 1;  f.flags = 65; mesh.faces[168] = f;
	f.v[0] = 270;  f.v[1] = 271;  f.v[2] = 200;  	f.smGroup = 1;  f.flags = 65; mesh.faces[169] = f;
	f.v[0] = 269;  f.v[1] = 270;  f.v[2] = 200;  	f.smGroup = 1;  f.flags = 65; mesh.faces[170] = f;
	f.v[0] = 269;  f.v[1] = 200;  f.v[2] = 201;  	f.smGroup = 1;  f.flags = 66; mesh.faces[171] = f;
	f.v[0] = 47;  f.v[1] = 165;  f.v[2] = 166;  	f.smGroup = 1;  f.flags = 66; mesh.faces[172] = f;
	f.v[0] = 268;  f.v[1] = 269;  f.v[2] = 201;  	f.smGroup = 1;  f.flags = 65; mesh.faces[173] = f;
	f.v[0] = 267;  f.v[1] = 268;  f.v[2] = 201;  	f.smGroup = 1;  f.flags = 65; mesh.faces[174] = f;
	f.v[0] = 266;  f.v[1] = 267;  f.v[2] = 201;  	f.smGroup = 1;  f.flags = 65; mesh.faces[175] = f;
	f.v[0] = 266;  f.v[1] = 201;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 66; mesh.faces[176] = f;
	f.v[0] = 265;  f.v[1] = 266;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[177] = f;
	f.v[0] = 112;  f.v[1] = 113;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[178] = f;
	f.v[0] = 264;  f.v[1] = 265;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[179] = f;
	f.v[0] = 176;  f.v[1] = 177;  f.v[2] = 92;  	f.smGroup = 1;  f.flags = 65; mesh.faces[180] = f;
	f.v[0] = 176;  f.v[1] = 92;  f.v[2] = 93;  	f.smGroup = 1;  f.flags = 66; mesh.faces[181] = f;
	f.v[0] = 47;  f.v[1] = 166;  f.v[2] = 167;  	f.smGroup = 1;  f.flags = 66; mesh.faces[182] = f;
	f.v[0] = 223;  f.v[1] = 176;  f.v[2] = 93;  	f.smGroup = 1;  f.flags = 65; mesh.faces[183] = f;
	f.v[0] = 120;  f.v[1] = 237;  f.v[2] = 238;  	f.smGroup = 1;  f.flags = 66; mesh.faces[184] = f;
	f.v[0] = 296;  f.v[1] = 120;  f.v[2] = 238;  	f.smGroup = 1;  f.flags = 64; mesh.faces[185] = f;
	f.v[0] = 296;  f.v[1] = 238;  f.v[2] = 239;  	f.smGroup = 1;  f.flags = 66; mesh.faces[186] = f;
	f.v[0] = 296;  f.v[1] = 239;  f.v[2] = 240;  	f.smGroup = 1;  f.flags = 66; mesh.faces[187] = f;
	f.v[0] = 296;  f.v[1] = 240;  f.v[2] = 241;  	f.smGroup = 1;  f.flags = 66; mesh.faces[188] = f;
	f.v[0] = 296;  f.v[1] = 241;  f.v[2] = 242;  	f.smGroup = 1;  f.flags = 66; mesh.faces[189] = f;
	f.v[0] = 296;  f.v[1] = 242;  f.v[2] = 243;  	f.smGroup = 1;  f.flags = 66; mesh.faces[190] = f;
	f.v[0] = 296;  f.v[1] = 243;  f.v[2] = 244;  	f.smGroup = 1;  f.flags = 66; mesh.faces[191] = f;
	f.v[0] = 296;  f.v[1] = 244;  f.v[2] = 245;  	f.smGroup = 1;  f.flags = 66; mesh.faces[192] = f;
	f.v[0] = 296;  f.v[1] = 245;  f.v[2] = 246;  	f.smGroup = 1;  f.flags = 66; mesh.faces[193] = f;
	f.v[0] = 296;  f.v[1] = 246;  f.v[2] = 247;  	f.smGroup = 1;  f.flags = 66; mesh.faces[194] = f;
	f.v[0] = 296;  f.v[1] = 247;  f.v[2] = 248;  	f.smGroup = 1;  f.flags = 66; mesh.faces[195] = f;
	f.v[0] = 295;  f.v[1] = 296;  f.v[2] = 248;  	f.smGroup = 1;  f.flags = 65; mesh.faces[196] = f;
	f.v[0] = 295;  f.v[1] = 248;  f.v[2] = 249;  	f.smGroup = 1;  f.flags = 66; mesh.faces[197] = f;
	f.v[0] = 295;  f.v[1] = 249;  f.v[2] = 250;  	f.smGroup = 1;  f.flags = 66; mesh.faces[198] = f;
	f.v[0] = 295;  f.v[1] = 250;  f.v[2] = 251;  	f.smGroup = 1;  f.flags = 66; mesh.faces[199] = f;
	f.v[0] = 295;  f.v[1] = 251;  f.v[2] = 252;  	f.smGroup = 1;  f.flags = 66; mesh.faces[200] = f;
	f.v[0] = 295;  f.v[1] = 252;  f.v[2] = 253;  	f.smGroup = 1;  f.flags = 66; mesh.faces[201] = f;
	f.v[0] = 295;  f.v[1] = 253;  f.v[2] = 254;  	f.smGroup = 1;  f.flags = 66; mesh.faces[202] = f;
	f.v[0] = 294;  f.v[1] = 295;  f.v[2] = 254;  	f.smGroup = 1;  f.flags = 65; mesh.faces[203] = f;
	f.v[0] = 294;  f.v[1] = 254;  f.v[2] = 255;  	f.smGroup = 1;  f.flags = 66; mesh.faces[204] = f;
	f.v[0] = 294;  f.v[1] = 255;  f.v[2] = 256;  	f.smGroup = 1;  f.flags = 66; mesh.faces[205] = f;
	f.v[0] = 294;  f.v[1] = 256;  f.v[2] = 257;  	f.smGroup = 1;  f.flags = 66; mesh.faces[206] = f;
	f.v[0] = 294;  f.v[1] = 257;  f.v[2] = 258;  	f.smGroup = 1;  f.flags = 66; mesh.faces[207] = f;
	f.v[0] = 294;  f.v[1] = 258;  f.v[2] = 259;  	f.smGroup = 1;  f.flags = 66; mesh.faces[208] = f;
	f.v[0] = 294;  f.v[1] = 259;  f.v[2] = 260;  	f.smGroup = 1;  f.flags = 66; mesh.faces[209] = f;
	f.v[0] = 294;  f.v[1] = 260;  f.v[2] = 261;  	f.smGroup = 1;  f.flags = 66; mesh.faces[210] = f;
	f.v[0] = 293;  f.v[1] = 294;  f.v[2] = 261;  	f.smGroup = 1;  f.flags = 65; mesh.faces[211] = f;
	f.v[0] = 293;  f.v[1] = 261;  f.v[2] = 262;  	f.smGroup = 1;  f.flags = 66; mesh.faces[212] = f;
	f.v[0] = 293;  f.v[1] = 262;  f.v[2] = 263;  	f.smGroup = 1;  f.flags = 66; mesh.faces[213] = f;
	f.v[0] = 293;  f.v[1] = 263;  f.v[2] = 264;  	f.smGroup = 1;  f.flags = 66; mesh.faces[214] = f;
	f.v[0] = 293;  f.v[1] = 264;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 64; mesh.faces[215] = f;
	f.v[0] = 292;  f.v[1] = 293;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[216] = f;
	f.v[0] = 291;  f.v[1] = 292;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[217] = f;
	f.v[0] = 290;  f.v[1] = 291;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[218] = f;
	f.v[0] = 289;  f.v[1] = 290;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[219] = f;
	f.v[0] = 288;  f.v[1] = 289;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[220] = f;
	f.v[0] = 287;  f.v[1] = 288;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[221] = f;
	f.v[0] = 286;  f.v[1] = 287;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[222] = f;
	f.v[0] = 285;  f.v[1] = 286;  f.v[2] = 202;  	f.smGroup = 1;  f.flags = 65; mesh.faces[223] = f;
	f.v[0] = 285;  f.v[1] = 202;  f.v[2] = 203;  	f.smGroup = 1;  f.flags = 66; mesh.faces[224] = f;
	f.v[0] = 285;  f.v[1] = 203;  f.v[2] = 204;  	f.smGroup = 1;  f.flags = 66; mesh.faces[225] = f;
	f.v[0] = 285;  f.v[1] = 204;  f.v[2] = 205;  	f.smGroup = 1;  f.flags = 66; mesh.faces[226] = f;
	f.v[0] = 285;  f.v[1] = 205;  f.v[2] = 206;  	f.smGroup = 1;  f.flags = 66; mesh.faces[227] = f;
	f.v[0] = 285;  f.v[1] = 206;  f.v[2] = 207;  	f.smGroup = 1;  f.flags = 66; mesh.faces[228] = f;
	f.v[0] = 284;  f.v[1] = 285;  f.v[2] = 207;  	f.smGroup = 1;  f.flags = 65; mesh.faces[229] = f;
	f.v[0] = 284;  f.v[1] = 207;  f.v[2] = 208;  	f.smGroup = 1;  f.flags = 66; mesh.faces[230] = f;
	f.v[0] = 284;  f.v[1] = 208;  f.v[2] = 209;  	f.smGroup = 1;  f.flags = 66; mesh.faces[231] = f;
	f.v[0] = 284;  f.v[1] = 209;  f.v[2] = 210;  	f.smGroup = 1;  f.flags = 66; mesh.faces[232] = f;
	f.v[0] = 284;  f.v[1] = 210;  f.v[2] = 211;  	f.smGroup = 1;  f.flags = 66; mesh.faces[233] = f;
	f.v[0] = 284;  f.v[1] = 211;  f.v[2] = 212;  	f.smGroup = 1;  f.flags = 66; mesh.faces[234] = f;
	f.v[0] = 284;  f.v[1] = 212;  f.v[2] = 213;  	f.smGroup = 1;  f.flags = 66; mesh.faces[235] = f;
	f.v[0] = 283;  f.v[1] = 284;  f.v[2] = 213;  	f.smGroup = 1;  f.flags = 65; mesh.faces[236] = f;
	f.v[0] = 283;  f.v[1] = 213;  f.v[2] = 214;  	f.smGroup = 1;  f.flags = 66; mesh.faces[237] = f;
	f.v[0] = 283;  f.v[1] = 214;  f.v[2] = 215;  	f.smGroup = 1;  f.flags = 66; mesh.faces[238] = f;
	f.v[0] = 283;  f.v[1] = 215;  f.v[2] = 216;  	f.smGroup = 1;  f.flags = 66; mesh.faces[239] = f;
	f.v[0] = 283;  f.v[1] = 216;  f.v[2] = 217;  	f.smGroup = 1;  f.flags = 66; mesh.faces[240] = f;
	f.v[0] = 282;  f.v[1] = 283;  f.v[2] = 217;  	f.smGroup = 1;  f.flags = 65; mesh.faces[241] = f;
	f.v[0] = 282;  f.v[1] = 217;  f.v[2] = 218;  	f.smGroup = 1;  f.flags = 66; mesh.faces[242] = f;
	f.v[0] = 282;  f.v[1] = 218;  f.v[2] = 219;  	f.smGroup = 1;  f.flags = 66; mesh.faces[243] = f;
	f.v[0] = 282;  f.v[1] = 219;  f.v[2] = 220;  	f.smGroup = 1;  f.flags = 66; mesh.faces[244] = f;
	f.v[0] = 281;  f.v[1] = 282;  f.v[2] = 220;  	f.smGroup = 1;  f.flags = 65; mesh.faces[245] = f;
	f.v[0] = 281;  f.v[1] = 220;  f.v[2] = 221;  	f.smGroup = 1;  f.flags = 66; mesh.faces[246] = f;
	f.v[0] = 281;  f.v[1] = 221;  f.v[2] = 222;  	f.smGroup = 1;  f.flags = 66; mesh.faces[247] = f;
	f.v[0] = 281;  f.v[1] = 222;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 66; mesh.faces[248] = f;
	f.v[0] = 280;  f.v[1] = 281;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 65; mesh.faces[249] = f;
	f.v[0] = 223;  f.v[1] = 93;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 66; mesh.faces[250] = f;
	f.v[0] = 111;  f.v[1] = 112;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[251] = f;
	f.v[0] = 47;  f.v[1] = 167;  f.v[2] = 168;  	f.smGroup = 1;  f.flags = 66; mesh.faces[252] = f;
	f.v[0] = 79;  f.v[1] = 47;  f.v[2] = 168;  	f.smGroup = 1;  f.flags = 64; mesh.faces[253] = f;
	f.v[0] = 79;  f.v[1] = 168;  f.v[2] = 169;  	f.smGroup = 1;  f.flags = 66; mesh.faces[254] = f;
	f.v[0] = 79;  f.v[1] = 169;  f.v[2] = 170;  	f.smGroup = 1;  f.flags = 66; mesh.faces[255] = f;
	f.v[0] = 79;  f.v[1] = 170;  f.v[2] = 171;  	f.smGroup = 1;  f.flags = 66; mesh.faces[256] = f;
	f.v[0] = 79;  f.v[1] = 171;  f.v[2] = 172;  	f.smGroup = 1;  f.flags = 66; mesh.faces[257] = f;
	f.v[0] = 79;  f.v[1] = 172;  f.v[2] = 173;  	f.smGroup = 1;  f.flags = 66; mesh.faces[258] = f;
	f.v[0] = 79;  f.v[1] = 173;  f.v[2] = 174;  	f.smGroup = 1;  f.flags = 66; mesh.faces[259] = f;
	f.v[0] = 79;  f.v[1] = 174;  f.v[2] = 175;  	f.smGroup = 1;  f.flags = 66; mesh.faces[260] = f;
	f.v[0] = 79;  f.v[1] = 175;  f.v[2] = 128;  	f.smGroup = 1;  f.flags = 66; mesh.faces[261] = f;
	f.v[0] = 79;  f.v[1] = 128;  f.v[2] = 129;  	f.smGroup = 1;  f.flags = 66; mesh.faces[262] = f;
	f.v[0] = 78;  f.v[1] = 79;  f.v[2] = 129;  	f.smGroup = 1;  f.flags = 65; mesh.faces[263] = f;
	f.v[0] = 77;  f.v[1] = 78;  f.v[2] = 129;  	f.smGroup = 1;  f.flags = 65; mesh.faces[264] = f;
	f.v[0] = 77;  f.v[1] = 129;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 64; mesh.faces[265] = f;
	f.v[0] = 76;  f.v[1] = 77;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[266] = f;
	f.v[0] = 75;  f.v[1] = 76;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[267] = f;
	f.v[0] = 74;  f.v[1] = 75;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[268] = f;
	f.v[0] = 73;  f.v[1] = 74;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[269] = f;
	f.v[0] = 72;  f.v[1] = 73;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[270] = f;
	f.v[0] = 71;  f.v[1] = 72;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[271] = f;
	f.v[0] = 70;  f.v[1] = 71;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[272] = f;
	f.v[0] = 69;  f.v[1] = 70;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[273] = f;
	f.v[0] = 68;  f.v[1] = 69;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[274] = f;
	f.v[0] = 67;  f.v[1] = 68;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[275] = f;
	f.v[0] = 66;  f.v[1] = 67;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[276] = f;
	f.v[0] = 65;  f.v[1] = 66;  f.v[2] = 280;  	f.smGroup = 1;  f.flags = 65; mesh.faces[277] = f;
	f.v[0] = 65;  f.v[1] = 280;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 64; mesh.faces[278] = f;
	f.v[0] = 64;  f.v[1] = 65;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 65; mesh.faces[279] = f;
	f.v[0] = 63;  f.v[1] = 64;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 65; mesh.faces[280] = f;
	f.v[0] = 62;  f.v[1] = 63;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 65; mesh.faces[281] = f;
	f.v[0] = 61;  f.v[1] = 62;  f.v[2] = 223;  	f.smGroup = 1;  f.flags = 65; mesh.faces[282] = f;
	f.v[0] = 61;  f.v[1] = 223;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 64; mesh.faces[283] = f;
	f.v[0] = 60;  f.v[1] = 61;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 65; mesh.faces[284] = f;
	f.v[0] = 59;  f.v[1] = 60;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 65; mesh.faces[285] = f;
	f.v[0] = 58;  f.v[1] = 59;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 65; mesh.faces[286] = f;
	f.v[0] = 57;  f.v[1] = 58;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 65; mesh.faces[287] = f;
	f.v[0] = 56;  f.v[1] = 57;  f.v[2] = 94;  	f.smGroup = 1;  f.flags = 65; mesh.faces[288] = f;
	f.v[0] = 56;  f.v[1] = 94;  f.v[2] = 95;  	f.smGroup = 1;  f.flags = 66; mesh.faces[289] = f;
	f.v[0] = 55;  f.v[1] = 56;  f.v[2] = 95;  	f.smGroup = 1;  f.flags = 65; mesh.faces[290] = f;
	f.v[0] = 54;  f.v[1] = 55;  f.v[2] = 95;  	f.smGroup = 1;  f.flags = 65; mesh.faces[291] = f;
	f.v[0] = 54;  f.v[1] = 95;  f.v[2] = 96;  	f.smGroup = 1;  f.flags = 66; mesh.faces[292] = f;
	f.v[0] = 54;  f.v[1] = 96;  f.v[2] = 97;  	f.smGroup = 1;  f.flags = 66; mesh.faces[293] = f;
	f.v[0] = 53;  f.v[1] = 54;  f.v[2] = 97;  	f.smGroup = 1;  f.flags = 65; mesh.faces[294] = f;
	f.v[0] = 53;  f.v[1] = 97;  f.v[2] = 98;  	f.smGroup = 1;  f.flags = 66; mesh.faces[295] = f;
	f.v[0] = 52;  f.v[1] = 53;  f.v[2] = 98;  	f.smGroup = 1;  f.flags = 65; mesh.faces[296] = f;
	f.v[0] = 51;  f.v[1] = 52;  f.v[2] = 98;  	f.smGroup = 1;  f.flags = 65; mesh.faces[297] = f;
	f.v[0] = 51;  f.v[1] = 98;  f.v[2] = 99;  	f.smGroup = 1;  f.flags = 66; mesh.faces[298] = f;
	f.v[0] = 51;  f.v[1] = 99;  f.v[2] = 100;  	f.smGroup = 1;  f.flags = 66; mesh.faces[299] = f;
	f.v[0] = 50;  f.v[1] = 51;  f.v[2] = 100;  	f.smGroup = 1;  f.flags = 65; mesh.faces[300] = f;
	f.v[0] = 49;  f.v[1] = 50;  f.v[2] = 100;  	f.smGroup = 1;  f.flags = 65; mesh.faces[301] = f;
	f.v[0] = 49;  f.v[1] = 100;  f.v[2] = 101;  	f.smGroup = 1;  f.flags = 66; mesh.faces[302] = f;
	f.v[0] = 49;  f.v[1] = 101;  f.v[2] = 102;  	f.smGroup = 1;  f.flags = 66; mesh.faces[303] = f;
	f.v[0] = 110;  f.v[1] = 111;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[304] = f;
	f.v[0] = 109;  f.v[1] = 110;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[305] = f;
	f.v[0] = 108;  f.v[1] = 109;  f.v[2] = 44;  	f.smGroup = 1;  f.flags = 65; mesh.faces[306] = f;
	f.v[0] = 49;  f.v[1] = 102;  f.v[2] = 103;  	f.smGroup = 1;  f.flags = 66; mesh.faces[307] = f;
	f.v[0] = 44;  f.v[1] = 49;  f.v[2] = 103;  	f.smGroup = 1;  f.flags = 64; mesh.faces[308] = f;
	f.v[0] = 44;  f.v[1] = 103;  f.v[2] = 104;  	f.smGroup = 1;  f.flags = 66; mesh.faces[309] = f;
	f.v[0] = 108;  f.v[1] = 44;  f.v[2] = 104;  	f.smGroup = 1;  f.flags = 64; mesh.faces[310] = f;
	f.v[0] = 108;  f.v[1] = 104;  f.v[2] = 105;  	f.smGroup = 1;  f.flags = 66; mesh.faces[311] = f;
	f.v[0] = 107;  f.v[1] = 108;  f.v[2] = 105;  	f.smGroup = 1;  f.flags = 65; mesh.faces[312] = f;
	f.v[0] = 106;  f.v[1] = 107;  f.v[2] = 105;  	f.smGroup = 1;  f.flags = 69; mesh.faces[313] = f;
	mesh.InvalidateGeomCache();
	mesh.EnableEdgeList(1);
}

void CharacterObject::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox )
{
	if (dumFlags&DISABLE_DISPLAY)
		return;
	Matrix3 mat = inode->GetObjectTM(t);
	UpdateMesh();
	abox = mesh.getBoundingBox();
	abox = abox * mat;
}

void CharacterObject::GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& abox )
{
	UpdateMesh();	
	abox = mesh.getBoundingBox();	
}

void CharacterObject::GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel )
{
	UpdateMesh();
	abox = mesh.getBoundingBox(tm);
}

int CharacterObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
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

void CharacterObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	if (dumFlags&CREATING)	// If creating this one, don't try to snap to it!
		return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);

	UpdateMesh();
	mesh.snap( gw, snap, p, tm );
}

int CharacterObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
	Matrix3 m;
	if (dumFlags&DISABLE_DISPLAY)
		return 0;
	color = Color(inode->GetWireColor());
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();
	m = inode->GetObjectTM(t);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL/*|GW_Z_BUFFER*/);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())
		gw->setColor( LINE_COLOR, color);

	UpdateMesh();
	mesh.render( gw, mtl, NULL, COMP_ALL);	
	gw->setRndLimits(rlim);
	return 0;
}

//---------------------------------------------------------------------------
// Create callback related methods
//---------------------------------------------------------------------------
CreateMouseCallBack* CharacterObject::GetCreateMouseCallBack()
{
	gCharacterCreateCB.SetObj(this);
	return &gCharacterCreateCB;
}

//---------------------------------------------------------------------------
//
int CharacterObjectCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
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
				// Find the node and plug in the wire color
				ULONG handle;
				mpObject->NotifyDependents(FOREVER, (PartID)&handle, REFMSG_GET_NODE_HANDLE);
				INode *node;
				node = GetCOREInterface()->GetINodeByHandle(handle);
				if (node) {
					node->SetWireColor(ColorMan()->GetColor(CHARACTER_WIRE_COLOR));
					}

				mpObject->Flags() |= CREATING;	// tell object we're building it so we can disable snapping to itself
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
					mpObject->Flags() &= ~CREATING;	
					return 0;
				}
				break;			
			}
	} 
	else if (msg == MOUSE_ABORT) 
	{
		mpObject->Flags() &= ~CREATING;	
		return CREATE_ABORT;
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// > 11/3/02 - 11:57pm --MQM-- 
// need to adjust character size when moving to new units

void CharacterObject::RescaleWorldUnits( float f )
{
	// already scaled to world units
	if ( TestAFlag(A_WORK1) )
		return;
	SetAFlag(A_WORK1);

 	// rescale internal character icon size.
	// note that maxscript that drives this must do
	// the following so that our "rescaled" value
	// gets reflected in the gui!
	//
	//  on iconSize get val do 
	//	(
	//	    iconsize = delegate.size
	//	)
	//
	mpBlock->RescaleParam( kPB_SIZE, -1, f );
}
