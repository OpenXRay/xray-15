/**********************************************************************
 *<
	FILE: $$root$$.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "$$root$$.h"

#define $$CLASS_NAME_UPPER$$_CLASS_ID	Class_ID($$CLASSID1$$, $$CLASSID2$$)
$$IF(EXTENSION)
#define XTC$$CLASS_NAME_UPPER$$_CLASS_ID	Class_ID($$EXTCLASSID1$$, $$EXTCLASSID2$$)
$$ENDIF

$$IF(SPACE_WARP_TYPE)
#define WARPOBJ_CLASS_ID	Class_ID($$WARPID1$$, $$WARPID2$$)
$$ENDIF

$$IF(IK_TYPE)
using namespace IKSys;
$$ENDIF

$$IF(MATERIAL_TYPE)
#define NSUBMTL		1 // TODO: number of sub-materials supported by this plugin 
$$ELIF(TEX_TYPE)
#define NSUBTEX		1 // TODO: number of sub-textures supported by this plugin 
#define COORD_REF	0
$$ENDIF // TEX_TYPE

$$IF(PARAM_MAPS)
$$IF(MODIFIER_TYPE)
$$IF(SIMPLE_TYPE)
#define PBLOCK_REF  SIMPMOD_PBLOCKREF
$$ELSE
#define PBLOCK_REF	$$PB_REF$$
$$ENDIF
$$ELIF(SPACE_WARP_TYPE)
#define PBLOCK_REF SIMPWSMMOD_PBLOCKREF
$$ELIF(!MATERIAL_TYPE)
#define PBLOCK_REF	$$PB_REF$$
$$ELSE
#define PBLOCK_REF	NSUBMTL
$$ENDIF
$$ENDIF

$$IF(SPACE_WARP_TYPE)

//Definition of the Object Representation of the Deformer

class $$CLASS_NAME$$Object : public SimpleWSMObject2
{	
	public:		
		IObjParam *ip;
		HWND hSot;
					
		$$CLASS_NAME$$Object();		

		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return WARPOBJ_CLASS_ID;}		
		TCHAR *GetObjectName() {return GetString(IDS_CLASS_NAME);}		

		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		
		void InvalidateUI();

		int DoOwnSelectHilite() {return TRUE;}
		CreateMouseCallBack* GetCreateMouseCallBack();

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);

		Modifier *CreateWSMMod(INode *node);
		void BuildMesh(TimeValue t);
		
		// Direct paramblock access - Use the Paramblock maintained by SimpleWSMObject2

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
};

class $$CLASS_NAME$$ObjCreateCallBack : public CreateMouseCallBack 
{
	$$CLASS_NAME$$Object *swo;
	IPoint2 sp0;
	Point3 p0;

	public:
	
	int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj($$CLASS_NAME$$Object *obj) {swo = obj;}
};
$$ENDIF

$$//=================================START DIALOG DEFS====================================
$$IF(SHADER_TYPE)
inline float PcToFrac(int pc) { return (float)pc/100.0f; }

inline int FracToPc(float f) {
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
}

inline float Bound( float x, float min = 0.0f, float max = 1.0f ){ return x < min? min:( x > max? max : x); }
inline Color Bound( Color& c )
	{ return Color( Bound(c.r), Bound(c.g), Bound(c.b) ); }
/*===========================================================================*\
 |	Definition of our UI and map parameters for the shader
\*===========================================================================*/

// Number of Map Buttons on our UI and number of texmaps
#define NMBUTS 3
#define SHADER_NTEXMAPS	3
// Channels used by this shader
#define S_DI	0
#define S_BR	1
#define S_TR	2


// Texture Channel number --> IDC resource ID
static int texMButtonsIDC[] = {
	IDC_MAPON_CLR, IDC_MAPON_BR, IDC_MAPON_TR,
};
		
// Map Button --> Texture Map number
static int texmapFromMBut[] = { 0, 1, 2 };


// Channel Name array
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_MN_DIFFUSE,		IDS_MN_BRIGHTNESS,	IDS_MN_OPACITY,		IDS_MN_NONE, 
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
};	

// Channel Name array (INTERNAL NAMES)
static int intertexNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_MN_DIFFUSE_I,	IDS_MN_BRIGHTNESS_I,IDS_MN_OPACITY_I,		IDS_MN_NONE, 
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
};	

// Internal channel names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("diffuseMap"),_T("BrightnessMap"), _T("opacityMap"), _T(""), 	
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
};	

// Type of map channels supported
static int chanType[STD2_NMAX_TEXMAPS] = {
	CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};	


// What channel in our shader maps to one of the StdMat channel IDs?
//
//
// The following are the StdMat channel IDs:
//
// 0   --	ambient
// 1   --	diffuse
// 2   --	specular
// 3   --	shininesNs
// 4   --	shininess strength
// 5   --	self-illumination
// 6   --	opacity
// 7   --	filter color
// 8   --	bump 
// 9   --	reflection
// 10  --	refraction 
// 11  --	displacement

static int stdIDToChannel[N_ID_CHANNELS] = { -1, 0, -1, -1, -1, -1, 2, -1, -1, -1, -1, -1 };



/*===========================================================================*\
 |	Class definition for the shader itself
\*===========================================================================*/

#define SHADER_PARAMS (STD_EXTRA)

class $$CLASS_NAME$$Dlg;
$$ENDIF //SHADER_TYPE

$$IF(SHADOW_TYPE)
class $$CLASS_NAME$$;
class $$CLASS_NAME$$Gen: public ShadowGenerator 
{ 
public:
	$$CLASS_NAME$$ *theShad;

	Matrix3 lightToWorld;
	Matrix3 worldToLight;
	Matrix3 camToLight;

	LightObject *light;
	ObjLightDesc *ltDesc;


	$$CLASS_NAME$$Gen($$CLASS_NAME$$ *shad, LightObject *l,  ObjLightDesc *ld, ULONG flags) { 
		theShad = shad;
		light = l;
		ltDesc = ld;
		} 

	
	$$CLASS_NAME$$Gen() {} 
	~$$CLASS_NAME$$Gen() { FreeBuffer();  }

	// Update the shadow generator per frame
	int Update(TimeValue t,const RendContext& rendCntxt,RenderGlobalContext *rgc,
		Matrix3& lightToWorld,float aspect,float param,float clipDist);

	int UpdateViewDepParams(const Matrix3& worldToCam) { 
		camToLight = Inverse(lightToWorld * worldToCam);
		return 1;
		}

	// Clean up methods
	void FreeBuffer() {}
	void DeleteThis() {delete this; }

	// Perform the shadow sampling
	float Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope);
	float Sample(ShadeContext &sc, Point3 &norm, Color& color);
};

class $$CLASS_NAME$$Dlg: public ShadowParamDlg 
{
public:
	$$CLASS_NAME$$ *theShad;
	Interface *ip;
	IParamMap2 *pmap;

	// Constructor
	$$CLASS_NAME$$Dlg($$CLASS_NAME$$ *shad, Interface *iface);
	~$$CLASS_NAME$$Dlg();

	void DeleteThis() { delete this; }
};
$$ENDIF //SHADOW_TYPE

$$IF(COLPICK_TYPE)
class $$CLASS_NAME$$ModelessDlg;
class $$CLASS_NAME$$Dlg
{

public:
	HWND	hwOwner;
	HWND	hwPanel;
	
	DWORD	origRGB;
    DWORD	currentRGB;

	IPoint2 curPos;
	TSTR	colName;

	HSVCallback *callback;
	static IPoint2 initPos;
	BOOL	objColor;
	$$CLASS_NAME$$ModelessDlg *sm;


	// Modeless and Modal support
	BOOL	Modeless;
	int		StartModal();
	HWND	StartModeless();


	// Construct and initialize
	$$CLASS_NAME$$Dlg(HWND hOwner, DWORD col,  IPoint2* pos,
			HSVCallback *cb, TCHAR *name, int objClr = 0, $$CLASS_NAME$$ModelessDlg *smp = NULL);
	~$$CLASS_NAME$$Dlg();
	void DoPaint(HWND hWnd);


	// Modify the dialog's settings
	void SetNewColor (DWORD color, TCHAR *name);
	void ModifyColor (DWORD color);
	DWORD GetColor();
	IPoint2 GetPosition();
	void InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name);
};

class $$CLASS_NAME$$ModelessDlg: public ColorPicker 
{
	HWND hwnd;
	$$CLASS_NAME$$Dlg *colDlg;

public:
		$$CLASS_NAME$$ModelessDlg(HWND hwndOwner, DWORD initColor, IPoint2* pos, HSVCallback* callback, TCHAR *name, int objClr);
		~$$CLASS_NAME$$ModelessDlg();

		void SetNewColor (DWORD color, TCHAR *name);
		void ModifyColor (DWORD color);
		void InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name);

		DWORD GetColor();
		IPoint2 GetPosition();
		void Destroy();
};
$$ELIF(TEX_TYPE)
class $$CLASS_NAME$$;
$$ENDIF

$$IF(TEXTURE_2D_TYPE)
class $$CLASS_NAME$$Sampler: public MapSampler {
	$$CLASS_NAME$$	*tex;
	public:
		$$CLASS_NAME$$Sampler() { tex= NULL; }
		$$CLASS_NAME$$Sampler($$CLASS_NAME$$ *c) { tex= c; }
		void Set($$CLASS_NAME$$ *c) { tex = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
		float SampleMono(ShadeContext& sc, float u,float v);
		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;
$$ENDIF //TEXTURE_2D_TYPE

$$IF(EXTENSION)
class XTC$$CLASS_NAME$$ : public XTCObject
{
public:
	XTC$$CLASS_NAME$$();
	~XTC$$CLASS_NAME$$();

	Class_ID ExtensionID(){return XTC$$CLASS_NAME_UPPER$$_CLASS_ID;}

	XTCObject *Clone();

	void DeleteThis(){delete this;}
	int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj);

	ChannelMask DependsOn(){return GEOM_CHANNEL|TOPO_CHANNEL;}
	ChannelMask ChannelsChanged(){return GEOM_CHANNEL;}

	void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod);
	void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod);
	
	BOOL SuspendObjectDisplay();
	
};

$$ENDIF

$$// Start:================================= Start: CLASS DEFS =====================
$$IF(SKIN_GIZMO_TYPE)
class $$CLASS_NAME$$ : public $$SUPER_CLASS_NAME$$ , public ResourceMakerCallback {
	public:
$$ELSE
class $$CLASS_NAME$$ : public $$SUPER_CLASS_NAME$$ {
	public:
$$ENDIF
$$//----- Start: Common stuff


$$IF(PARAM_MAPS )
$$IF(SIMPLE_OBJ || SIMPLE_MANIP)
		// Parameter block handled by parent
$$ELSE
		// Parameter block
		IParamBlock2	*$$PBLOCK$$;	//ref 0

$$ENDIF // !SIMPLE_TYPE
$$ELIF(!STATIC_TYPE || GUP_TYPE || IK_TYPE)
		static HWND hParams;

$$ENDIF  //PARAM_MAPS

$$//----- End: Common stuff
$$IF(ATMOSPHERIC_TYPE)//mcr_cd_atmos================================================

		// From Atmospheric
		TSTR GetName() {return GetString(IDS_CLASS_NAME);}
		AtmosParamDlg *CreateParamDialog(IRendParams *ip);
		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);
		void Update(TimeValue t, Interval& valid);
		void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG);
		
		// Support for gizmos in our atmospheric
		int NumGizmos() ;
		INode *GetGizmo(int i);
		void DeleteGizmo(int i);
		void InsertGizmo(int i, INode *node);
		void AppendGizmo(INode *node);
		BOOL OKGizmo(INode *node); 
 		void EditGizmo(INode *node);
$$ELIF(CAMERA_TYPE)//===============================================================
$$ELIF(COLPICK_TYPE)//mcr_cd_colpick================================================

	// Do Modal dialog
	int ModalColorPicker(
		HWND hwndOwner, 		// owning window
		DWORD *lpc,				// pointer to color to be edited
	    IPoint2 *spos, 			// starting position, set to ending position
	    HSVCallback *callBack,	// called when color changes
		TCHAR *name				// name of color being edited
	    );

	// Create Modeless dialog.
	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   
		DWORD initColor,  
		IPoint2* spos,    
		HSVCallback *pcallback,
		TCHAR *name, 	  
		BOOL isObjectColor);

	const TCHAR *	ClassName() {  return GetString(IDS_CLASS_NAME); 	}	

	INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) {return 0; } 	
	Class_ID ClassID() { return $$CLASS_NAME_UPPER$$_CLASS_ID; }
	void DeleteThis(){delete this;}
$$ELIF(GRID_OBJECT_TYPE)//==========================================================
$$ELIF(CONTROLLER_TYPE)//===========================================================
$$ELIF(FILE_IMPORT_TYPE)//mcr_cd_import=============================================

		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
		int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi, BOOL suppressPrompts=FALSE);	// Import file
$$ELIF(FILE_EXPORT_TYPE)//mcr_cd_export=============================================
		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);


$$ELIF(FILE_LIST_TYPE)//============================================================
$$ELIF(FILTER_KERNEL_TYPE)//mcr_cd_filter===========================================

	private:
		// We store 2 parameters
		double param1, param2;
	public:
		TSTR GetName() { return GetString(IDS_CLASS_NAME); }

		// Kernel parameter setup and handling
		long GetNFilterParams();
		TCHAR * GetFilterParamName( long nParam );
		double GetFilterParam( long nParam );
		void SetFilterParam( long nParam, double val );
		TCHAR * GetDefaultComment();

		void Update(TimeValue t, Interval& valid);

		// Actual kernel function
		double KernelFn( double x, double y );

		// Kernel functionality queries
		long GetKernelSupport();
		long GetKernelSupportY();
		bool Is2DKernel();
		bool IsVariableSz();
		void SetKernelSz( double x, double y = 0.0 );
		void GetKernelSz( double& x, double& y );
		bool IsNormalized();
		bool HasNegativeLobes();
$$ELIF(FRONT_END_CONTROLLER_TYPE)//mcr_cd_FrontEnd==================================
		IFrontEnd		*ife;
		Interface		*ip;

	//This method is called to delete this instance of the plug-in class 
		void Initialize(IFrontEnd *ife,Interface *ip);
		DWORD GetLayout();
		void Resize();
		
		DWORD ProcessToolButton(int id, int notify);
		DWORD ProcessMenuItem(int id, int notify);
		DWORD ProcessInitMenu(HMENU hMenu);
		DWORD ProcessViewportRightClick(HWND hWnd, IPoint2 m);
		DWORD ProcessViewportLabelClick(HWND hWnd, IPoint2 m);
		DWORD ProcessViewportMenuItem(int id, int notify);
		DWORD ProcessViewportInitMenu(HMENU hMenu);
$$ELIF(GUP_TYPE)//mcr_cd_gup========================================================	

		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

$$ELIF(IK_TYPE)//===================================================================

		bool IsHistoryDependent() const;
		bool DoesOneChainOnly()const;

		bool IsInteractive() const;
		bool UseSlidingJoint() const;
		bool UseSwivelAngle() const;
  		bool IsAnalytic() const ;

		bool SolveEERotation() const;


  		const IKSys::ZeroPlaneMap*
			GetZeroPlaneMap(const Point3& a0, const Point3& n0) const
			{ return NULL; }

		float		GetPosThreshold() const;
		float		GetRotThreshold() const;
		unsigned	GetMaxIteration() const;
		void		SetPosThreshold(float);
		void		SetRotThreshold(float);
		void		SetMaxIteration(unsigned);

		ReturnCondition Solve(IKSys::LinkChain&);

		Class_ID ClassID() {return $$CLASS_NAME_UPPER$$_CLASS_ID;}		
		SClass_ID SuperClassID() { return $$SUPER_CLASS_ID$$; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}


$$ELIF(HELPER_OBJECT_TYPE)//========================================================
$$ELIF(IMAGE_FILTER_COMPOSITOR_TYPE)//mcr_cd_vpost==================================

		const TCHAR   	*Description        ( ) ;
		
		//TODO: Implement the following methods
		const TCHAR   	*AuthorName         ( ) { return _T("");}
		const TCHAR   	*CopyrightMessage   ( ) { return _T("");}
		UINT           	Version             ( ) { return (VERSION_3DSMAX);}
		//TODO: Return the flags that decsribe the capabilities of the plugin
		DWORD          	Capability          ( ) { return( IMGFLT_FILTER | IMGFLT_MASK | IMGFLT_CONTROL ); }
		
		void           	ShowAbout           ( HWND hWnd );  
		BOOL           	ShowControl         ( HWND hWnd );  
		BOOL           	Render              ( HWND hWnd );
		void			FilterUpdate		( );
		BOOL  			Control	  			(HWND ,UINT ,WPARAM ,LPARAM );

$$ELIF(IMAGE_LOADER_SAVER_TYPE)//===================================================
$$ELIF(IMAGE_VIEWER_TYPE)//=========================================================
$$ELIF(LIGHT_TYPE)//================================================================
$$ELIF(MANIP_TYPE)

$$IF(!SIMPLE_MANIP)
		TSTR name;
		int HitTest(TimeValue t, INode* pNode, int type, int crossing,
							int flags, IPoint2 *pScreenPoint, ViewExp *pVpt);
		int Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags);

	   // Used for manipulator set manager, which is always active.
		bool AlwaysActive();



		DisplayState  MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);
		DisplayState  MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);
$$ENDIF    

		void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
		void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
		void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData); 
$$IF(!SIMPLE_MANIP)
		INode* GetINode();
		TSTR& GetManipName(); 

		//FPInterface* GetInterface(Interface_ID id);
$$ENDIF 
		//from BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack(void);

$$IF(SIMPLE_MANIP)
		void UpdateShapes(TimeValue t, TSTR& toolTip);
		void GenerateShapes(TimeValue t);
$$ENDIF

$$IF(!SIMPLE_MANIP)
		//from Object
		ObjectState Eval(int);
		void InitNodeName(TSTR& s);
		Interval ObjectValidity(TimeValue t);

		
		//from GeomObject
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL, BOOL useSel=FALSE );
$$ENDIF
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

$$ELIF(MATERIAL_TYPE)//mcr_cd_mtl===================================================
		Mtl				*submtl[NSUBMTL];  //array of sub-materials
		BOOL			mapOn[NSUBMTL];
		float			spin;
		Interval		ivalid;
		
		ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		Interval Validity(TimeValue t);
		void Reset();

		void NotifyChanged();

		// From MtlBase and Mtl
		void SetAmbient(Color c, TimeValue t);		
		void SetDiffuse(Color c, TimeValue t);		
		void SetSpecular(Color c, TimeValue t);
		void SetShininess(float v, TimeValue t);
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
		float WireSize(int mtlNum=0, BOOL backFace=FALSE);
				

		// Shade and displacement calculation
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 	

		// SubMaterial access methods
		int NumSubMtls() {return NSUBMTL;}
		Mtl* GetSubMtl(int i);
		void SetSubMtl(int i, Mtl *m);
		TSTR GetSubMtlSlotName(int i);
		TSTR GetSubMtlTVName(int i);

		// SubTexmap access methods
		int NumSubTexmaps() {return 0;}
		Texmap* GetSubTexmap(int i);
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		TSTR GetSubTexmapTVName(int i);
		
		BOOL SetDlgThing(ParamDlg* dlg);
		$$CLASS_NAME$$(BOOL loading);

$$ELIF(MODIFIER_TYPE)//mcr_cd_mod=================================================
		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
$$IF(!SIMPLE_TYPE)
$$IF(EXTENSION)
		//TODO: Add the channels that the modifier needs to perform its modification
		ChannelMask ChannelsUsed()  { return EXTENSION_CHANNEL; }
		
		// We have to include the channels, that the extension object changes, so the 
		// PostChanChangedNotify will be called after the modifier added the extension objects
		// to the object flowing up the stack.

		ChannelMask ChannelsChanged() { return EXTENSION_CHANNEL|GEOM_CHANNEL; }

$$ELSE
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
$$ENDIF
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);

$$ENDIF //Simple_type

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

$$IF(SIMPLE_TYPE)
		// From SimpleMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
$$ENDIF //SIMPLE_TYPE					

		Interval GetValidity(TimeValue t);

		// Automatic texture support
$$ELIF(NURBS_OBJECT_TYPE)//=========================================================
$$ELIF(PARTICLE_TYPE)//=============================================================
$$ELIF(PATCH_OBJECT_TYPE)//=========================================================
$$ELIF(PROCEDURAL_OBJECT_TYPE)//mcr_cd_object=======================================

		//Class vars
		static IObjParam *ip;			//Access to the interface
$$IF(!SIMPLE_TYPE)
		BOOL suspendSnap;				//A flag for setting snapping on/off
$$ENDIF		
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
$$IF(!SIMPLE_TYPE)
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		//TODO: Return the name that will appear in the history browser (modifier stack)
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }
		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		//TODO: Return the default name of the node when it is created.
		void InitNodeName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
$$ENDIF //!SIMPLE_TYPE
		
		// From Object
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		//TODO: Evaluate the object and return the ObjectState
		ObjectState Eval(TimeValue t) { return ObjectState(this); };		
		//TODO: Return the validity interval of the object as a whole
		Interval ObjectValidity(TimeValue t) { return FOREVER; }

		// From Animatable
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

$$IF(SIMPLE_TYPE)		
		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
$$ELSE
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
$$ENDIF //SIMPLE_TYPE

$$ELIF(SKIN_GIZMO_TYPE)
  
		IObjParam *ip;
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		
		void SetInitialName();
		TCHAR *GetName();
		void SetName(TCHAR *name);
		BOOL IsEnabled();
		BOOL IsVolumeBased();
		BOOL IsInVolume(Point3 p, Matrix3 tm);

		BOOL IsEditing();
		void Enable(BOOL enable);
		void EndEditing();
		void EnableEditing(BOOL enable);

		
		IGizmoBuffer *CopyToBuffer(); 
		void PasteFromBuffer(IGizmoBuffer *buffer); 
		
		BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable); 
		void PostDeformSetup(TimeValue t);
		void PreDeformSetup(TimeValue t);
		HWND hGizmoParams;
		Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm);

// From Base Object
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               
		int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm );
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE );
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);


$$ELIF(RENDERER_TYPE)//=============================================================
$$ELIF(RENDER_EFFECT_TYPE)//mcr_cd_effect===========================================

		// Effect class methods
		TSTR GetName() { return GetString(IDS_CLASS_NAME); }
		EffectParamDlg *CreateParamDialog( IRendParams *pParams );
		int RenderBegin( TimeValue t, ULONG flags );
		int RenderEnd( TimeValue t );
		void Update( TimeValue t, Interval& valid );
		void Apply( TimeValue t, Bitmap *pBM, RenderGlobalContext *pGC, CheckAbortCallback *checkAbort);
$$ELIF(SAMPLER_TYPE)//mcr_cd_sampler================================================
		
		TCHAR* GetDefaultComment();
		// This samples a sequence for the area
		void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK pMask=NULL );		// This is the function that is called to get the next sample 
		// returns FALSE when out of samples
		BOOL NextSample( Point2* pOutPt, float* pSampleSz, int n );
		// Integer number of samples for current quality setting
		int GetNSamples();	

		// Get/Set quality and enable status
		int SupportsQualityLevels();
		void SetQuality( float q );
		float GetQuality();

		void SetEnable( BOOL on );
		BOOL GetEnable();

		ULONG SupportsStdParams(){ return R3_ADAPTIVE; }

		void SetAdaptiveOn( BOOL on );
		BOOL IsAdaptiveOn();

		void SetAdaptiveThreshold( float val );
		float GetAdaptiveThreshold();
$$ELIF(SHADER_TYPE)//mcr_cd_shader
	private:
	friend class $$CLASS_NAME$$Dlg;
	protected:
		Interval		ivalid;
		TimeValue		curTime;

		// Pointer to the dialog handler
		$$CLASS_NAME$$Dlg*	paramDlg;

		// Storage for our parameters
		Color			diffuse;
		float			brightness;

	public:
		TSTR GetName() { return GetString( IDS_CLASS_NAME ); }

			// Tell MAX what standard parameters that we support
		ULONG SupportStdParams(){ return SHADER_PARAMS; }

		// copy std params, for switching shaders
		void CopyStdParams( Shader* pFrom );
		void ConvertParamBlk( ParamBlockDescID *oldPBDesc, int oldCount, IParamBlock *oldPB ){};

		// Texture map channel support
		long nTexChannelsSupported(){ return SHADER_NTEXMAPS; }
		TSTR GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
		TSTR GetTexChannelInternalName( long nTex ){ return GetString( intertexNameIDS[ nTex ] ); }
		long ChannelType( long nChan ) { return chanType[nChan]; }
		long StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

		// Find out if we have a key at time value t
		BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }

		// Get the requirements for this material (supersampling, etc)
		ULONG GetRequirements( int subMtlNum ){ return MTLREQ_PHONG; }

		// Support for the dialog UI
		ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen,int );
		ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
		void SetParamDlg( ShaderParamDlg* newDlg,int ){ paramDlg = ($$CLASS_NAME$$Dlg*)newDlg; }

			// Shader state methods
		void Update(TimeValue t, Interval& valid);
		void Reset();

		// Fill the IllumParams with our data
		void GetIllumParams( ShadeContext &sc, IllumParams &ip );

		// Shader specific implimentations
		void Illum(ShadeContext &sc, IllumParams &ip);
		void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol);

		void CombineComponents( ShadeContext &sc, IllumParams& ip );

		float EvalHiliteCurve(float x);

		// Metal support
		BOOL IsMetal();

		// This plugin specific
		void SetBrightness(float v, TimeValue t);
		float GetBrightness(int mtlNum=0, BOOL backFace=FALSE);
		float GetBrightness( TimeValue t);

		// DIFFUSE
		void SetDiffuseClr(Color c, TimeValue t);
		Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetDiffuseClr(TimeValue t);		

		// AMBIENT
		void SetAmbientClr(Color c, TimeValue t);
		Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetAmbientClr(TimeValue t);		

		// SPECULAR
		void SetSpecularClr(Color c, TimeValue t);
		void SetSpecularLevel(float v, TimeValue t);		
		Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE);
		float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecularClr(TimeValue t);
		float GetSpecularLevel(TimeValue t);

		// SELFILLUM
		void SetSelfIllum(float v, TimeValue t);
		float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE);
		void SetSelfIllumClrOn( BOOL on );
		BOOL IsSelfIllumClrOn();
		BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace);
		void SetSelfIllumClr(Color c, TimeValue t);
		Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE);
		float GetSelfIllum(TimeValue t);	
		Color GetSelfIllumClr(TimeValue t);		

		// SOFTEN
		void SetSoftenLevel(float v, TimeValue t);
		float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE);
		float GetSoftenLevel(TimeValue t);

		void SetGlossiness(float v, TimeValue t);
		float GetGlossiness(int mtlNum, BOOL backFace);
		float GetGlossiness( TimeValue t);

		// Standard locks not supported (Diffuse+Specular)/(Ambient+Diffuse)
		void SetLockDS(BOOL lock){ }
		BOOL GetLockDS(){ return FALSE; }
		void SetLockAD(BOOL lock){ }
		BOOL GetLockAD(){ return FALSE; }
		void SetLockADTex(BOOL lock){ }
		BOOL GetLockADTex(){ return FALSE; }

		void NotifyChanged(){ NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); }
$$ELIF(SHADOW_TYPE)//mcr_cd_shadow
		$$CLASS_NAME$$Dlg *theParam;
		// Create the shadow system's UI
		ShadowParamDlg *CreateShadowParamDlg(Interface *ip) { 
			theParam = new $$CLASS_NAME$$Dlg(this, ip); return theParam;
			}

		// Create a shadow generator instance - only exists during a render
		ShadowGenerator* CreateShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags) {
			return new $$CLASS_NAME$$Gen(this,l,ld,flags);
			}

		BOOL SupportStdMapInterface() { return FALSE; }


$$ELIF(SPLINE_SHAPE_TYPE)//=========================================================
$$ELIF(SOUND_TYPE)//================================================================
$$ELIF(SPACE_WARP_TYPE)//===========================================================

		IObjParam *ip;

		$$CLASS_NAME$$(INode *node,$$CLASS_NAME$$Object *obj);		

		

		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		

		Interval GetValidity(TimeValue t);
		void InvalidateUI();

		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		Animatable* SubAnim(int i);


$$ELIF(TEX_TYPE)//mcr_cd_texmap===========================
		Texmap			*subtex[NSUBTEX]; //array of sub-materials
$$IF(TEXTURE_3D_TYPE)
		static ParamDlg *xyzGenDlg;
		XYZGen			*xyzGen;
$$ELSE
		static ParamDlg *uvGenDlg;
		UVGen			*uvGen;
$$ENDIF //TEXTURE_3D_TYPE		
		Interval		ivalid;

		//From MtlBase
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		BOOL SetDlgThing(ParamDlg* dlg);
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t);
		ULONG LocalRequirements(int subMtlNum);

		//TODO: Return the number of sub-textures
		int NumSubTexmaps() { return NSUBTEX; }
		//TODO: Return the pointer to the 'i-th' sub-texmap
		Texmap* GetSubTexmap(int i) { return subtex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		
		//From Texmap
		RGBA EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);

$$IF(TEXTURE_2D_TYPE)
		//TODO: Returns TRUE if this texture can be used in the interactive renderer
		BOOL SupportTexDisplay() { return FALSE; }
		void ActivateTexDisplay(BOOL onoff);
		DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
		//TODO: Return UV transformation matrix for use in the viewports
		void GetUVTransform(Matrix3 &uvtrans) { uvGen->GetUVTransform(uvtrans); }
		//TODO: Return the tiling state of the texture for use in the viewports
		int GetTextureTiling() { return  uvGen->GetTextureTiling(); }
		int GetUVWSource() { return uvGen->GetUVWSource(); }
		UVGen *GetTheUVGen() { return uvGen; }
$$ELSE
		XYZGen *GetTheXYZGen() { return xyzGen; } 
$$ENDIF //TEXTURE_2D_TYPE		
		
		//TODO: Return anim index to reference index
		int SubNumToRefNum(int subNum) { return subNum; }
		
$$IF(TEXTURE_3D_TYPE)
		//TODO: If your class is derived from Tex3D then you should also 
		//implement ReadSXPData for 3D Studio/DOS SXP texture compatibility
		void ReadSXPData(TCHAR *name, void *sxpdata) { }
$$ENDIF //TEXTURE_3D_TYPE		
$$ELIF(TRACK_VIEW_UTILITY_TYPE)////mcr_cd_tvutil====================================
		HWND			hPanel;
		ITVUtility		*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,ITVUtility *iu);
		void EndEditParams(Interface *ip,ITVUtility *iu);
		
		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void TrackSelectionChanged();
		void NodeSelectionChanged();
		void KeySelectionChanged();
		void TimeSelectionChanged();
		void MajorModeChanged();
		void TrackListChanged();
$$ELSE	////mcr_cd_util=============================================================
		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
$$ENDIF //FOR VARIOUS PLUGINTYPES
		
$$IF(!NON_ANIM_TYPE)
$$IF(!MANIP_TYPE)
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
$$ENDIF

		//From Animatable
		Class_ID ClassID() {return $$CLASS_NAME_UPPER$$_CLASS_ID;}		
		SClass_ID SuperClassID() { return $$SUPER_CLASS_ID$$; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

$$IF(!SKIN_GIZMO_TYPE)		
		RefTargetHandle Clone( RemapDir &remap );
$$ENDIF
$$IF(!SIMPLE_MANIP) 
$$IF(!SIMPLE_OBJ)
$$IF(!SPACE_WARP_TYPE)
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);
$$ENDIF
$$ENDIF
$$ENDIF

$$IF(!SKIN_GIZMO_TYPE) 
$$IF(!SIMPLE_OBJ)
$$IF(!SIMPLE_MANIP)

		int NumSubs() { return $$NUM_REFS$$; }
$$ENDIF
$$ENDIF
$$ENDIF
$$IF(TEX_TYPE || MATERIAL_TYPE) 
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);

		// TODO: Maintain the number or references here 
		int NumRefs() { return $$NUM_REFS$$; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
$$ELSE
$$IF(!SKIN_GIZMO_TYPE)
$$IF(!SIMPLE_OBJ)
$$IF(!SIMPLE_TYPE)
$$IF(!SIMPLE_MANIP)
		TSTR SubAnimName(int i) { return $$PARAM_NAME$$; }				
		Animatable* SubAnim(int i) { return $$PBLOCK$$; }

		// TODO: Maintain the number or references here
		int NumRefs() { return $$NUM_REFS$$; }
		RefTargetHandle GetReference(int i) { return $$PBLOCK$$; }
$$IF(PARAM_MAPS)		
		void SetReference(int i, RefTargetHandle rtarg) { $$PBLOCK$$=(IParamBlock2*)rtarg; }
$$ELSE
		void SetReference(int i, RefTargetHandle rtarg);
$$ENDIF // PARAM_MAPS

$$ENDIF
$$ENDIF
$$ENDIF
$$ENDIF
$$ENDIF


$$IF(PARAM_MAPS)
$$IF(!SKIN_GIZMO_TYPE)
$$//
$$IF(!SIMPLE_MANIP)

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return $$PBLOCK$$; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return ($$PBLOCK$$->ID() == id) ? $$PBLOCK$$ : NULL; } // return id'd ParamBlock
$$ELSE

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return mpPblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPblock->ID() == id) ? mpPblock : NULL; } // return id'd ParamBlock
$$ENDIF
$$ENDIF
$$ENDIF	

		void DeleteThis() { delete this; }		
$$ENDIF //!NON_ANIM_TYPE
$$IF(STATIC_TYPE)

		void DeleteThis() { }		
$$ENDIF //STATIC_TYPE
		//Constructor/Destructor

$$IF(!MANIP_TYPE)
$$IF(!MATERIAL_TYPE)
		$$CLASS_NAME$$();
		~$$CLASS_NAME$$();		
$$ENDIF
$$ELSE
		$$CLASS_NAME$$();
		$$CLASS_NAME$$(INode *pnode);
		~$$CLASS_NAME$$();		
$$ENDIF

};
$$// ================================= End: CLASS DEFS =====================

$$IF(ATMOSPHERIC_TYPE)
class $$CLASS_NAME$$ParamDlg : public AtmosParamDlg {
	public:
		$$CLASS_NAME$$ *atmos;
		IRendParams *ip;
$$IF(PARAM_MAPS)
		IParamMap2 *pmap;
$$ENDIF

		$$CLASS_NAME$$ParamDlg($$CLASS_NAME$$ *a,IRendParams *i);
		Class_ID ClassID() {return $$CLASS_NAME_UPPER$$_CLASS_ID;}
		ReferenceTarget* GetThing() {return atmos;}
		void SetThing(ReferenceTarget *m);		
		void DeleteThis();
	};
$$ENDIF //ATMOSPHERIC_TYPE
$$IF(MODIFIER_TYPE)
$$IF(SIMPLE_TYPE)
//This is the callback object used by modifiers to deform "Deformable" objects.
class $$CLASS_NAME$$Deformer: public Deformer {
	public:
		$$CLASS_NAME$$Deformer();
		//TODO: Add other plugin specific constructors and member functions
		Point3 Map(int i, Point3 p); 
	};
$$ENDIF //SIMPLE_TYPE
$$ENDIF //MODIFIER_TYPE
$$IF(STATIC_TYPE) //(FRONT_END_CONTROLLER_TYPE || TRACK_VIEW_UTILITY_TYPE || UTILITY_TYPE || COLPICK_TYPE)
static $$CLASS_NAME$$ the$$CLASS_NAME$$;
$$ENDIF //STATIC_TYPE

class $$CLASS_NAME$$ClassDesc:public ClassDesc2 {
	public:
$$IF(SPACE_WARP_TYPE)
	int 			IsPublic() { return FALSE; }
$$ELSE
	int 			IsPublic() { return TRUE; }
$$ENDIF
$$IF(STATIC_TYPE)	
	void *			Create(BOOL loading = FALSE) { return &the$$CLASS_NAME$$; }
$$ELIF(MATERIAL_TYPE)
	void *			Create(BOOL loading) { return new $$CLASS_NAME$$(loading); }

$$ELSE
	void *			Create(BOOL loading = FALSE) { return new $$CLASS_NAME$$(); }
$$ENDIF //STATIC_TYPE
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return $$SUPER_CLASS_ID$$; }
	Class_ID		ClassID() { return $$CLASS_NAME_UPPER$$_CLASS_ID; }
$$IF(SKIN_GIZMO_TYPE)
	// The Skin modifier checks the category to decide whether the modifier is a Skin Gizmo.  This 
	// must not be changed
	const TCHAR* 	Category() { return GetString(IDS_PW_GIZMOCATEGORY); }
$$ELSE
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
$$ENDIF

	const TCHAR*	InternalName() { return _T("$$CLASS_NAME$$"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

$$IF(MANIP_TYPE)
$$IF(ADD_COMMENTS)
/*******************************************************************************************************
*	
	Returns TRUE if this helper object implements a Manipulator Object
*
\*******************************************************************************************************/
$$ENDIF
    BOOL IsManipulator() { return TRUE; }

$$IF(ADD_COMMENTS)
/********************************************************************************************************
*
	Returns true if the class is a manipulator and it can manipulates the ReferenceTarget passed 
	to it.  In this case it simple states that it can manipulate itself.    
*
\********************************************************************************************************/
$$ENDIF
    
	BOOL CanManipulate(ReferenceTarget* hTarget) {
		//TODO: Return TRUE if it can manipulate the ReferenceTarget.
        return hTarget->ClassID() == ClassID() && hTarget->SuperClassID() == SuperClassID();
    }

        
    Manipulator* CreateManipulator(RefTargetHandle hTarget, INode* pNode);

$$ENDIF
};

$$IF(MANIP_TYPE)
$$IF(ADD_COMMENTS)
/*********************************************************************************************************
*
	If a manipulator applies to a node, this call will create an instance of it and add it to the 
	attached objects of the node.  It will then initialize the manipualtor
*
\*****************************************************************************************************/
$$ENDIF
Manipulator * $$CLASS_NAME$$ClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    $$CLASS_NAME$$* pManip = new $$CLASS_NAME$$();
    return pManip;
}

$$ENDIF

$$IF(SPACE_WARP_TYPE)
/*===========================================================================*\
 |	Class Descriptor for the World Space Object
\*===========================================================================*/

class $$CLASS_NAME$$ObjClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic()							{ return TRUE; }
	void *			Create(BOOL loading = FALSE)		{ return new $$CLASS_NAME$$Object; }
	const TCHAR *	ClassName()							{ return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID()						{ return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID()							{ return WARPOBJ_CLASS_ID; }
	const TCHAR* 	Category()							{ return _T("");}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()						{ return _T("$$CLASS_NAME$$Object"); }
	HINSTANCE		HInstance()							{ return hInstance; }
};
$$ENDIF

static $$CLASS_NAME$$ClassDesc $$CLASS_NAME$$Desc;
ClassDesc2* Get$$CLASS_NAME$$Desc() { return &$$CLASS_NAME$$Desc; }

$$IF(SPACE_WARP_TYPE)
static $$CLASS_NAME$$ObjClassDesc $$CLASS_NAME$$ObjDesc;
ClassDesc2* Get$$CLASS_NAME$$ObjDesc() { return &$$CLASS_NAME$$ObjDesc; }
$$ENDIF

$$IF(PARAM_MAPS)
enum { $$CLASS_NAME_LOWER$$_params };

$$IF(SPACE_WARP_TYPE)
enum {obj_params};
$$ENDIF

//TODO: Add enums for various parameters
enum { 
$$IF(!UI_BY_MAX)
	pb_spin,
$$ENDIF
$$IF(TEX_TYPE)
	pb_coords,
$$ELIF(FILTER_KERNEL_TYPE)
	pb_param1,
	pb_param2,
$$ELIF(SAMPLER_TYPE)
	pb_quality,
	pb_enable,
	pb_adapt_enable,
	pb_adapt_threshold,
$$ELIF(SHADER_TYPE)
	pb_diffuse, 
	pb_brightness, 
$$ELIF(SKIN_GIZMO_TYPE)
	pb_gizmoparam_name,
$$ELIF(ATMOSPHERIC_TYPE)
	pb_gizmos,
$$ELIF(MATERIAL_TYPE)
	mtl_mat1,
	mtl_mat1_on,

$$ENDIF //TEX_TYPE
};

$$IF(SPACE_WARP_TYPE)
// For the Space Warp Object
enum{
	pb_spin_obj
};
$$ENDIF

$$IF(SKIN_GIZMO_TYPE)
$$IF(ADD_COMMENTS)
/************************************************************************************************
*
	The following PBAccessor allows the Skin Modifier's Gizmo list to be kept upto date when the 
	name changes in the gizmo rollout
*
\************************************************************************************************/
$$ENDIF

class $$CLASS_NAME$$PBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		$$CLASS_NAME$$* p = ($$CLASS_NAME$$*)owner;

		switch (id)
		{
			//Check all name changes and pass them onto the Skin modifier
			case pb_gizmoparam_name:

					if (p->bonesMod)
						p->bonesMod->UpdateGizmoList();
				
				break;

		}
	}
};

static $$CLASS_NAME$$PBAccessor gizmoJoint_accessor;
$$ENDIF

static ParamBlockDesc2 $$CLASS_NAME_LOWER$$_param_blk ( $$CLASS_NAME_LOWER$$_params, _T("params"),  0, &$$CLASS_NAME$$Desc, 
$$IF(UI_BY_MAX)
	P_AUTO_CONSTRUCT, PBLOCK_REF,
$$ELSE
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_spin, 			_T("spin"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN, 
		p_default, 		0.1f, 
		p_range, 		0.0f,1000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT,	IDC_SPIN, 0.01f, 
		end,
$$ENDIF //UI_BY_MAX
$$IF(TEX_TYPE)
	pb_coords,			_T("coords"),		TYPE_REFTARG,	P_OWNERS_REF,	IDS_COORDS,
		p_refno,		COORD_REF, 
		end,
$$ELIF(FILTER_KERNEL_TYPE)
	pb_param1, 		_T("param1"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_PARAM1,
		p_default,		0.3f,
		end,
	pb_param2, 		_T("param2"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_PARAM2,
		p_default,		0.6f,
		end,
$$ELIF(SAMPLER_TYPE)
	pb_quality, 		_T("quality"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_QUALITY,
		p_default,		0.5f,
		end,
	pb_enable, 			_T("enable"), 		TYPE_BOOL, 		0, 				IDS_ENABLE,
		p_default,		FALSE,
		end,
	pb_adapt_enable, 	_T("adaptive"), 	TYPE_BOOL, 		0, 				IDS_AD_ENABLE,
		p_default,		TRUE,
		end,
	pb_adapt_threshold, _T("threshold"), 	TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_AD_THRESH,
		p_default,		0.02f,
		end,
$$ELIF(SHADER_TYPE)
	// params
	pb_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_MN_DIFFUSE, 
		p_default, Color(0.8f, 0.5f, 0.5f), 
		end,
	pb_brightness, _T("brightness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_MN_BRIGHTNESS,
		p_default,		0.2f,
		p_range,		0.0f, 1.0f,
		end,
$$ENDIF //TEX_TYPE

$$IF(SKIN_GIZMO_TYPE)
	pb_gizmoparam_name, 	_T("name"),		TYPE_STRING, 	0,  IDS_GIZMOPARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_NAME,
		p_accessor,		&gizmoJoint_accessor,
		end, 		
$$ENDIF //SKIN_TYPE

$$IF(ATMOSPHERIC_TYPE)
	pb_gizmos,    _T("Gizmos"),  TYPE_INODE_TAB, 0, P_AUTO_UI,	IDS_GIZMOS,
		p_ui,	TYPE_NODELISTBOX, IDC_GIZMOLIST,	IDC_ADDGIZMO,	0,	IDC_DELGIZMO,
		end,

$$ENDIF

$$IF(MATERIAL_TYPE)
	mtl_mat1,			_T("mtl_mat1"),			TYPE_MTL,	P_OWNERS_REF,	IDS_MTL1,
		p_refno,		0,
		p_submtlno,		0,		
		p_ui,			TYPE_MTLBUTTON, IDC_MTL1,
		end,
	mtl_mat1_on,		_T("mtl_mat1_on"),		TYPE_BOOL,		0,				IDS_MTL1ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MTLON1,
		end,

$$ENDIF
	end
	);

$$IF(SPACE_WARP_TYPE)
static ParamBlockDesc2 $$CLASS_NAME_LOWER$$obj_param_blk ( obj_params, _T("$$CLASS_NAME$$ObjParams"),  0, &$$CLASS_NAME$$ObjDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PANEL_OBJ, IDS_PARAMS, 0, 0, NULL, 
	// params
	pb_spin_obj, 			_T("spin"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN_OBJ, 
		p_default, 		0.1f, 
		p_range, 		0.0f,1000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT_OBJ,	IDC_SPIN_OBJ, 0.01f, 
		end,

	end
	);

$$ENDIF

$$//------------DIALOG PROCS FOR VARIOUS TYPES OF PLUGINS--------------
$$ELIF(ATMOSPHERIC_TYPE) //mcr_dp_atmos
static BOOL CALLBACK $$CLASS_NAME$$DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	$$CLASS_NAME$$ParamDlg *ins = ($$CLASS_NAME$$ParamDlg*)GetWindowLong(hWnd,GWL_USERDATA);
	if (!ins && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			ins = ($$CLASS_NAME$$ParamDlg*)lParam;
			SetWindowLong(hWnd,GWL_USERDATA,lParam);
			ins->atmos->hParams = hWnd;
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			ins->ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}
$$ELIF(COLPICK_TYPE || FILE_EXPORT_TYPE || FILE_IMPORT_TYPE ) //mcr_dp_colpick mcr_dp_export mcr_dp_import
BOOL CALLBACK $$CLASS_NAME$$OptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static $$CLASS_NAME$$ *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = ($$CLASS_NAME$$ *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}
$$ELIF(FRONT_END_CONTROLLER_TYPE)//mcr_dp_FrontEnd
static BOOL CALLBACK $$CLASS_NAME$$DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			the$$CLASS_NAME$$.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}
$$ELIF(IMAGE_FILTER_COMPOSITOR_TYPE) //mcr_dp_vpost
BOOL CALLBACK $$CLASS_NAME$$CtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static $$CLASS_NAME$$ *f = NULL;
	if (message == WM_INITDIALOG) 
		f = ($$CLASS_NAME$$ *)lParam;
	if (f) 
		return (f->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

$$ELIF(TRACK_VIEW_UTILITY_TYPE || UTILITY_TYPE) //mcr_dp_tvutil mcr_dp_util
static BOOL CALLBACK $$CLASS_NAME$$DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			the$$CLASS_NAME$$.Init(hWnd);
$$IF(TRACK_VIEW_UTILITY_TYPE)
			CenterWindow(hWnd,GetParent(hWnd));
$$ENDIF //TRACK_VIEW_UTILITY_TYPE
			break;

		case WM_DESTROY:
			the$$CLASS_NAME$$.Destroy(hWnd);
			break;

		case WM_COMMAND:
			break;

$$IF(TRACK_VIEW_UTILITY_TYPE)
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
$$ENDIF //TRACK_VIEW_UTILITY_TYPE

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			the$$CLASS_NAME$$.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

$$ELIF(TEX_TYPE) //mcr_dp_texmap
static BOOL CALLBACK  $$CLASS_NAME$$DlgProc(
		HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	$$CLASS_NAME$$Dlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = ($$CLASS_NAME$$Dlg*)lParam;
		theDlg->hPanel = hwnd;
		SetWindowLong(hwnd, GWL_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = ($$CLASS_NAME$$Dlg *)GetWindowLong(hwnd, GWL_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = TRUE;
	int	res = theDlg->PanelProc(hwnd,msg,wParam,lParam);
	theDlg->isActive = FALSE;
	return res;
};

$$ELIF(GUP_TYPE || IK_TYPE)
$$ELSE //mcr_dp_other
static BOOL CALLBACK $$CLASS_NAME$$DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	$$CLASS_NAME$$ *ins = ($$CLASS_NAME$$*)GetWindowLong(hWnd,GWL_USERDATA);
	if (!ins && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			ins = ($$CLASS_NAME$$*)lParam;
			SetWindowLong(hWnd,GWL_USERDATA,lParam);
			ins->hParams = hWnd;
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			ins->ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}
$$ENDIF //PARAM_MAPS

$$//--------------------------------------
$$//ATMOSPHERIC_TYPE //mcr_mf_atmos
$$//--------------------------------------
$$IF(ATMOSPHERIC_TYPE)

//--- $$CLASS_NAME$$ParamDlg -------------------------------------------------------

$$CLASS_NAME$$ParamDlg::$$CLASS_NAME$$ParamDlg($$CLASS_NAME$$ *a,IRendParams *i) 
{
	//TODO: Add the dialog and parameter map creation code here 
	atmos = a;
	ip    = i;
$$IF(PARAM_MAPS)	
	pmap = CreateRParamMap2(
		atmos->$$PBLOCK$$,
		i,
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		GetString(IDS_PARAMS),
		0);
$$ELSE
	if (!atmos->hParams) {
		atmos->hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PANEL),
				$$CLASS_NAME$$DlgProc, 
				GetString(IDS_PARAMS), 
				(LPARAM)this);		
	}	else {
$$// MYNOTE: SOMETHING SHOULD GO IN HERE
	}
	
$$ENDIF
}

void $$CLASS_NAME$$ParamDlg::SetThing(ReferenceTarget *m)
{
	
}

void $$CLASS_NAME$$ParamDlg::DeleteThis()
{
	//TODO: Add the dialog and parameter map destruction code here 
	delete this;
}

$$IF(!PARAM_MAPS)
HWND $$CLASS_NAME$$::hParams = NULL;
$$ENDIF  //PARAM_MAPS


$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	Atmospheric::Load(iload);

	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	Atmospheric::Save(isave);

	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}


RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_DELETED:
			{
				for(int i=0;i<NumGizmos();i++)
				{
					if((INode*)hTarget==GetGizmo(i)) DeleteGizmo(i);
				}
			}
			break;

		case REFMSG_CHANGE:
			if (hTarget == pblock && pblock->GetMap() != NULL)
				pblock->GetMap()->Invalidate();
			break;
		}
	return REF_SUCCEED;
	}


RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* newObj = new $$CLASS_NAME$$();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}

AtmosParamDlg *$$CLASS_NAME$$::CreateParamDialog(IRendParams *ip)
{
	return new $$CLASS_NAME$$ParamDlg(this,ip);
}

int $$CLASS_NAME$$::RenderBegin(TimeValue t,ULONG flags)
{		
  	//TODO: Add intiliazing stuff required by plugin prior to rendering
	return 0;
}

int $$CLASS_NAME$$::RenderEnd(TimeValue t)
{	
	//TODO: Add stuff required after each render
	return 0;
}

void $$CLASS_NAME$$::Update(TimeValue t, Interval& valid)
{
	
}

void $$CLASS_NAME$$::Shade(
		ShadeContext& sc,const Point3& p0,const Point3& p1,
		Color& color, Color& trans, BOOL isBG)
{
	//TODO: Add the code that is called to apply the atmospheric effect
}

/*===========================================================================*\
 |	Support for getting/setting gizmos  
\*===========================================================================*/

int $$CLASS_NAME$$::NumGizmos() 
{
	return $$PBLOCK$$->Count(pb_gizmos);
}

INode *$$CLASS_NAME$$::GetGizmo(int i) 
{
	INode *node = NULL;
	Interval iv;
	$$PBLOCK$$->GetValue(pb_gizmos,0,node,iv, i);
	return node;
}

void $$CLASS_NAME$$::DeleteGizmo(int i)
{
	$$PBLOCK$$->Delete(pb_gizmos, i,1); 
	$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI();

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void $$CLASS_NAME$$::InsertGizmo(int i, INode *node)
{
	$$PBLOCK$$->SetValue(pb_gizmos, 0, node, i);
}

void $$CLASS_NAME$$::AppendGizmo(INode *node)
{
	$$PBLOCK$$->Append(pb_gizmos, 1, &node);
}

BOOL $$CLASS_NAME$$::OKGizmo(INode *node)
{
	// check for duplicates in the gizmo list
	for(int i=0;i<NumGizmos();i++) { if(node==GetGizmo(i)) return FALSE; }

	ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
	if (os.obj->ClassID()==SPHEREGIZMO_CLASSID) return TRUE;
	if (os.obj->ClassID()==CYLGIZMO_CLASSID) return TRUE;
	if (os.obj->ClassID()==BOXGIZMO_CLASSID) return TRUE;
	return FALSE;
}

void $$CLASS_NAME$$::EditGizmo(INode *node)
{
}

$$//--------------------------------------
$$//CAMERA_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//COLPICK_TYPE mcr_mf_colpick
$$//--------------------------------------
$$ELIF(COLPICK_TYPE)

$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

IPoint2 $$CLASS_NAME$$Dlg::initPos(354,94);

void $$CLASS_NAME$$Dlg::DoPaint(HWND hWnd)
{
	// TODO: Handle the painting here
}

int $$CLASS_NAME$$Dlg::StartModal()
{
	Modeless = FALSE;
	return (int)DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_PANEL), 
		hwOwner, 
		$$CLASS_NAME$$OptionsDlgProc,
		(LPARAM)this);
}

HWND $$CLASS_NAME$$Dlg::StartModeless()
{
	Modeless = TRUE;
	return (HWND)CreateDialogParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_PANEL), 
		hwOwner, 
		$$CLASS_NAME$$OptionsDlgProc,
		(LPARAM)this);
}



$$CLASS_NAME$$Dlg::$$CLASS_NAME$$Dlg(HWND hOwner, DWORD col,  IPoint2* pos,
								HSVCallback *cb, TCHAR *name, int objClr, $$CLASS_NAME$$ModelessDlg *smp)
{
	hwOwner = hOwner;
	origRGB = currentRGB = col;
	if(pos) curPos = initPos = *pos;
	callback = cb;
	colName = name;
	Modeless = FALSE;
	hwPanel = NULL;
	objColor = objClr?TRUE:FALSE;
	sm = smp;
}

$$CLASS_NAME$$Dlg::~$$CLASS_NAME$$Dlg()
{
	if(hwPanel) hwPanel = NULL;
}


void $$CLASS_NAME$$Dlg::SetNewColor (DWORD color, TCHAR *name)
{
	colName = name;
	origRGB = currentRGB = color;
	SetWindowText(hwPanel,colName);
	InvalidateRect(hwPanel,NULL,FALSE);
}

void $$CLASS_NAME$$Dlg::ModifyColor (DWORD color)
{
	currentRGB=color;
	InvalidateRect(hwPanel,NULL,FALSE);
}

DWORD $$CLASS_NAME$$Dlg::GetColor()
{
	return currentRGB;
}

IPoint2 $$CLASS_NAME$$Dlg::GetPosition()
{
	return curPos;
}

void $$CLASS_NAME$$Dlg::InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name)
{
	if (callback) 
		callback->ColorChanged(currentRGB,1);
	callback = pcb;
    currentRGB = origRGB = col;
	colName = name;
	SetWindowText(hwPanel,colName);
	InvalidateRect(hwPanel,NULL,FALSE);
}

ColorPicker* $$CLASS_NAME$$::CreateColorPicker(HWND hwndOwner, DWORD initColor,
													IPoint2* spos, HSVCallback *pcallback,
													TCHAR *name, int objClr) 
{
	return new $$CLASS_NAME$$ModelessDlg(hwndOwner,initColor,spos,pcallback,name,objClr);
}	


int $$CLASS_NAME$$::ModalColorPicker(HWND hwndOwner, DWORD *lpc, IPoint2 *spos,
										  HSVCallback *callBack, TCHAR *name) 
{
	int res = 0;
	$$CLASS_NAME$$Dlg colDlg(hwndOwner,*lpc,spos,callBack,name);
	res = colDlg.StartModal();

	if(lpc)		*lpc = colDlg.GetColor();
	if(spos)	*spos = colDlg.GetPosition();

	return res;
}


/*===========================================================================*\
 |	$$CLASS_NAME$$ModelessDlg methods
\*===========================================================================*/


$$CLASS_NAME$$ModelessDlg::$$CLASS_NAME$$ModelessDlg(HWND hwndOwner, DWORD initColor, 
								   IPoint2* pos, HSVCallback* callback, 
								   TCHAR *name, int objClr)
{
	hwnd = NULL; colDlg = NULL;
	colDlg = new $$CLASS_NAME$$Dlg(hwndOwner,initColor,pos,callback,name,objClr,this);
	hwnd = colDlg->StartModeless();
}


void $$CLASS_NAME$$ModelessDlg::Destroy() {
	DestroyWindow(hwnd);
}

$$CLASS_NAME$$ModelessDlg::~$$CLASS_NAME$$ModelessDlg() {
	Destroy();
	if(colDlg) delete colDlg;
	hwnd = NULL; colDlg = NULL;
}


// Transactions with the dialog handler
void $$CLASS_NAME$$ModelessDlg::SetNewColor (DWORD color, TCHAR *name)
{
	colDlg->SetNewColor(color,name);
}

void $$CLASS_NAME$$ModelessDlg::ModifyColor (DWORD color)
{
	colDlg->ModifyColor(color);
}

void $$CLASS_NAME$$ModelessDlg::InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name)
{
	colDlg->InstallNewCB(col,pcb,name);
}

DWORD $$CLASS_NAME$$ModelessDlg::GetColor()
{
	return colDlg->GetColor();
}

IPoint2 $$CLASS_NAME$$ModelessDlg::GetPosition()
{
	return colDlg->GetPosition();
}
$$//--------------------------------------
$$//GRID_OBJECT_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//CONTROLLER_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//FILE_IMPORT_TYPE
$$//--------------------------------------
$$ELIF(FILE_IMPORT_TYPE)//mcr_mf_import

//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$() 
{

}

int $$CLASS_NAME$$::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *$$CLASS_NAME$$::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return _T("");
}
	
const TCHAR *$$CLASS_NAME$$::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::AuthorName()
{			
	//TODO: Return ASCII Author name
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int $$CLASS_NAME$$::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void $$CLASS_NAME$$::ShowAbout(HWND hWnd)
{			
	// Optional
}

int $$CLASS_NAME$$::DoImport(const TCHAR *filename,ImpInterface *i,
						Interface *gi, BOOL suppressPrompts)
{
	//TODO: Implement the actual file import here and 
	//		return TRUE If the file is imported properly

	if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				$$CLASS_NAME$$OptionsDlgProc, (LPARAM)this);
	return FALSE;
}
	
$$//--------------------------------------
$$//FILE_EXPORT_TYPE
$$//--------------------------------------
$$ELIF(FILE_EXPORT_TYPE)//mcr_mf_export

//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$() 
{

}

int $$CLASS_NAME$$::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *$$CLASS_NAME$$::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return _T("");
}
	
const TCHAR *$$CLASS_NAME$$::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::AuthorName()
{			
	//TODO: Return ASCII Author name
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *$$CLASS_NAME$$::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int $$CLASS_NAME$$::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void $$CLASS_NAME$$::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL $$CLASS_NAME$$::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}


int	$$CLASS_NAME$$::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	//TODO: Implement the actual file Export here and 
	//		return TRUE If the file is exported properly

	if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				$$CLASS_NAME$$OptionsDlgProc, (LPARAM)this);
	return FALSE;
}


$$//--------------------------------------
$$//FILE_LIST_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//FILTER_KERNEL_TYPE
$$//--------------------------------------
$$ELIF(FILTER_KERNEL_TYPE) //mcr_mf_filter
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

/*===========================================================================*\
 |	Kernel Parameter returns
\*===========================================================================*/

// Number of parameters we support
long $$CLASS_NAME$$::GetNFilterParams() { return 2; }

// Get the name of parameter #nParam
TCHAR * $$CLASS_NAME$$::GetFilterParamName( long nParam )
{ 
	return GetString( nParam ? IDS_PARAM2:IDS_PARAM1 );
}

// Get the value of parameter #nParam
double $$CLASS_NAME$$::GetFilterParam( long nParam )
{
	return nParam ? param2 : param1;
}

// Set our parameter variables
void $$CLASS_NAME$$::SetFilterParam( long nParam, double val )
{
	if (nParam) {  
		param2 = val;	
$$IF(PARAM_MAPS)
		pblock->SetValue( pb_param2, 0, float( val ) );		
$$ENDIF
 	} else { 
		param1 = val; 
$$IF(PARAM_MAPS)
		pblock->SetValue( pb_param1, 0, float( val ) );		
$$ENDIF
	} 
}

void $$CLASS_NAME$$::Update(TimeValue t, Interval& valid){
$$IF(PARAM_MAPS)
	float val;
	pblock->GetValue( pb_param1, t, val, valid ); param1 = val;
	pblock->GetValue( pb_param2, t, val, valid ); param2 = val;
$$ENDIF //PARAM_MAPS
}

TCHAR * $$CLASS_NAME$$::GetDefaultComment() 
{
	return GetString( IDS_COMMENT); 
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	FilterKernel::Load(iload);
	return IO_OK;
}
IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	FilterKernel::Save(isave);
	return IO_OK;
}


RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI();
			break;
		}
	return REF_SUCCEED;
}

RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* newObj = new $$CLASS_NAME$$();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}



/*===========================================================================*\
 |	Calculate a result
\*===========================================================================*/

double $$CLASS_NAME$$::KernelFn( double x, double y )
{
	if ( x < param1 ) return 1.0f;
	if ( x > param2 ) return 1.0f;
	else return 0.0f;
}


/*===========================================================================*\
 |	Kernel functionality queries
\*===========================================================================*/

// Integer number of pixels from center to filter 0 edge.
// Must not truncate filter x dimension for 2D filters
long $$CLASS_NAME$$::GetKernelSupport(){ return 1; }

// For 2d returns y support, for 1d returns 0
long $$CLASS_NAME$$::GetKernelSupportY(){ return 0; }

// Are we 2D or Variable Size?
bool $$CLASS_NAME$$::Is2DKernel(){ return FALSE; }
bool $$CLASS_NAME$$::IsVariableSz(){ return FALSE; }

// 1-D filters ignore the y parameter, return it as 0.0
void $$CLASS_NAME$$::SetKernelSz( double x, double y ){}
void $$CLASS_NAME$$::GetKernelSz( double& x, double& y ){ x = 0.5; y = 0.0; }

// Returning true will disable the built-in normalizer
bool $$CLASS_NAME$$::IsNormalized(){ return FALSE; }

// This is for possible future optimizations
bool $$CLASS_NAME$$::HasNegativeLobes(){ return FALSE; }
$$//--------------------------------------
$$//FRONT_END_CONTROLLER_TYPE
$$//--------------------------------------
$$ELIF(FRONT_END_CONTROLLER_TYPE) //mcr_mf_FrontEnd

$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	ip         = NULL;
	ife        = NULL;
	hPanel     = NULL;
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
	
}

void $$CLASS_NAME$$::Initialize(IFrontEnd *ife,Interface *ip)
{
	//TODO: Add the plugin initializing code here
	this->ife = ife;
	this->ip  = ip;

		hPanel = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		ife->GetCommandPanelHWND(),
		$$CLASS_NAME$$DlgProc);
}

DWORD $$CLASS_NAME$$::GetLayout()
{
	//TODO: Return a value used to configure the MAX user environment
	return FE_LAYOUT_CONTROL_COMMAND_PANEL;
}

//This method is called when the MAX application window is made larger or smaller
void $$CLASS_NAME$$::Resize()
{
	
}

//This method is called when the user operates the toolbar buttons
DWORD $$CLASS_NAME$$::ProcessToolButton(int id, int notify)
{
	return 0;
}

//This method is called when the user operates the pulldown menus
DWORD $$CLASS_NAME$$::ProcessMenuItem(int id, int notify)
{

	return 0;
}

//This method is called when the user operates the pulldown menus
DWORD $$CLASS_NAME$$::ProcessInitMenu(HMENU hMenu)
{
	return 0;
}

//This method is called when the user right clicks in one of the viewports
DWORD $$CLASS_NAME$$::ProcessViewportRightClick(HWND hWnd, IPoint2 m)
{
	return 0;
}

//This method is called when the user clicks on one of the MAX viewport labels 
DWORD $$CLASS_NAME$$::ProcessViewportLabelClick(HWND hWnd, IPoint2 m)
{
	return 0;
}

//This method is called when the user operates the viewport right-click menus
DWORD $$CLASS_NAME$$::ProcessViewportMenuItem(int id, int notify)
{
	return 0;
}

//This method is called when the user operates the right-click viewport menus
DWORD $$CLASS_NAME$$::ProcessViewportInitMenu(HMENU hMenu)
{
	return 0;
}

$$//--------------------------------------
$$//IMAGE_FILTER_COMPOSITOR_TYPE
$$//--------------------------------------
$$ELIF(IMAGE_FILTER_COMPOSITOR_TYPE) //mcr_mf_vpost
static BOOL CALLBACK AboutCtrlDlgProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;
		
		case WM_COMMAND:
		  switch (LOWORD(wParam)) {
			 
			 case IDOK:              
				  EndDialog(hWnd,1);
				  break;
		  }
		  return TRUE;
	}
	return FALSE;
}

//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

const TCHAR *$$CLASS_NAME$$::Description()
{
	//TODO: Return the description of the filter
	return _T("");
}

BOOL $$CLASS_NAME$$::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		hWnd,
		(DLGPROC)$$CLASS_NAME$$CtrlDlgProc,
		(LPARAM)this));
}

BOOL $$CLASS_NAME$$::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
	//TODO: Implement the message handling code here
	switch (message) {
		case WM_INITDIALOG: {
			//-- Make Dialogue Interactive				
			MakeDlgInteractive(hWnd);
			HWND hWndParent = TheManager->AppWnd();
			CenterWindow(hWnd,hWndParent);
			break;
			}
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		return TRUE;
	}
		

		return FALSE;
}

void $$CLASS_NAME$$::FilterUpdate()
{
	//This method is called when an instance of the filter ir created or updated
}

void $$CLASS_NAME$$::ShowAbout(HWND hWnd)
{
	  DialogBoxParam(
		 hInstance,
		 MAKEINTRESOURCE(IDD_ABOUT),
		 hWnd,
		 (DLGPROC)AboutCtrlDlgProc,
		 (LPARAM)this);
}


BOOL $$CLASS_NAME$$::Render(HWND hWnd)
{
	//TODO: Implement the image altering stuff here
	return TRUE;
}
$$//--------------------------------------
$$//IMAGE_LOADER_SAVER_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//IMAGE_VIEWER_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//HELPER_OBJECT_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//GUP_TYPE //mcr_mf_gup
$$//--------------------------------------
$$ELIF(GUP_TYPE)

$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

// Activate and Stay Resident
DWORD $$CLASS_NAME$$::Start( ) {
	
	// TODO: Do plugin initialization here
	
	// TODO: Return if you want remain loaded or not
	return GUPRESULT_KEEP;
}

void $$CLASS_NAME$$::Stop( ) {
	// TODO: Do plugin un-initialization here
}

DWORD $$CLASS_NAME$$::Control( DWORD parameter ) {
	return 0;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	return IO_OK;
}

$$//--------------------------------------
$$//IK_TYPE
$$//--------------------------------------
$$ELIF(IK_TYPE)

$$CLASS_NAME$$::$$CLASS_NAME$$()
{

}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}


bool $$CLASS_NAME$$::IsHistoryDependent() const
{
	//TODO:  Let the IK system know whether you a History dependent or not.
	// Return TRUE for History Dependent
	return false;
}

bool $$CLASS_NAME$$::DoesOneChainOnly() const
{
	//TODO:  Specify whether you can work on more than one chain or not
	return true; 
}


bool $$CLASS_NAME$$::IsInteractive() const
{
	//TODO: Return whether you support Interactive ?
	return false; 
}

bool $$CLASS_NAME$$::UseSlidingJoint() const
{
	//TODO: Do you use a sliding joint

	return false; 
}
	
bool $$CLASS_NAME$$::UseSwivelAngle() const
{
	//TODO: DO you use the Swivel angle

	return true; 
}
  		
bool $$CLASS_NAME$$::IsAnalytic() const 
{
	//TODO: Is this an Analytic solver - Return TRUE for yes
	return true; 
}


$$IF(ADD_COMMENTS)
/******************************************************************************************************
*

	RotThreshold() is not relevant to solver that does not SolveEERotation().
	UseSwivelAngle() and SolveEERotation() cannot both be true.

*
\******************************************************************************************************/
$$ENDIF
bool $$CLASS_NAME$$::SolveEERotation() const{

	//TODO: 

	return false; 
}

float $$CLASS_NAME$$::GetPosThreshold() const
{
	//TODO: Return the Position Threshold of the solver

	return 0.0f; 
}
	
float $$CLASS_NAME$$::GetRotThreshold() const
{

	//TODO: Return the Rotation Threshold of the solver


	return 0.0f; 
}
		
unsigned $$CLASS_NAME$$::GetMaxIteration() const
{
	return 0; 
}

void $$CLASS_NAME$$::SetPosThreshold(float)
{
	//TODO: Set the Position Threshold

}

void $$CLASS_NAME$$::SetRotThreshold(float)
{
	//TODO: Set the Rotation Threshold
}

void $$CLASS_NAME$$::SetMaxIteration(unsigned)
{

}

$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	This the method that is actually called to perform the IK specific task.
   
	This only use this method if you answers true to DoesOneChainOnly() and false to 
	HistoryDependent(). The solver is not designed to be invoked recursively. The
    recursion logic existing among the ik chains is taken care of by
    the Max IK (sub-)System.
*
\*************************************************************************************************/
$$ENDIF

IKSolver :: ReturnCondition $$CLASS_NAME$$ :: Solve (IKSys::LinkChain& linkChain){

	//TODO: Implement the actual IK Solver

	return 0;
}

$$//--------------------------------------
$$//LIGHT_TYPE
$$//--------------------------------------

$$//--------------------------------------
$$//MANIP_TYPE
$$//--------------------------------------
$$ELIF(MANIP_TYPE)

$$IF(ADD_COMMENTS)
/**************************************************************************************************
*
	Define a Mouse Callback to implement the creation of the gizmo.  If the Manipulator does not
	have any user interface or creation method, then this is not need and ClassDesc2::Ispublic() 
	can return FALSE.
*
\**************************************************************************************************/
$$ENDIF

class $$CLASS_NAME$$CreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;			//First point in screen coordinates
	$$CLASS_NAME$$ *ob;		//Pointer to the object 
	Point3 p0;				//First point in world coordinates
public:	
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj($$CLASS_NAME$$ *obj) {ob = obj;}
};


int $$CLASS_NAME$$CreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	//TODO: Implement the mouse creation code here
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0: // only happens with MOUSE_POINT msg
			sp0 = m;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
			break;
		//TODO: Add for the rest of points
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static $$CLASS_NAME$$CreateCallBack $$CLASS_NAME$$CreateCB;



$$IF(!SIMPLE_MANIP)
$$CLASS_NAME$$::$$CLASS_NAME$$():Manipulator(NULL)
{
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$IF(!SIMPLE_MANIP)
	//TODO: Define the name of the Manipulator
	name = TSTR("Wizard Manipulator");
$$ENDIF	
}

$$CLASS_NAME$$::$$CLASS_NAME$$(INode *pnode):Manipulator(pnode)
{
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
}
$$ENDIF

$$IF(SIMPLE_MANIP)
$$CLASS_NAME$$::$$CLASS_NAME$$():SimpleManipulator(NULL)
{
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$IF(!SIMPLE_MANIP)
	//TODO: Define the name of the Manipulator
	name = TSTR("Wizard Manipulator");
$$ENDIF	
}

$$CLASS_NAME$$::$$CLASS_NAME$$(INode *pnode):SimpleManipulator(pnode)
{
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
}
$$ENDIF


$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

void $$CLASS_NAME$$::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
{

$$IF(SIMPLE_MANIP)
	$$SUPER_CLASS_NAME$$::BeginEditParams(ip,flags,prev);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.BeginEditParams(ip, this, flags, prev);
$$ELSE
	if (!hParams) {
		hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PANEL),
				$$CLASS_NAME$$DlgProc, 
				GetString(IDS_PARAMS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams);
	} else {
		SetWindowLong(hParams,GWL_USERDATA,(LONG)this);
	}
$$ENDIF //PARAM_MAPS


}
		
void $$CLASS_NAME$$::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
$$IF(SIMPLE_MANIP)
	$$SUPER_CLASS_NAME$$::EndEditParams(ip,flags,next);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.EndEditParams(ip, this, flags, next);
$$ELSE
	ip->UnRegisterDlgWnd(hParams);
	ip->DeleteRollupPage(hParams);
	hParams = NULL;
$$ENDIF

}

$$IF(!SIMPLE_MANIP)
int $$CLASS_NAME$$::HitTest(TimeValue t, INode* pNode, int type, int crossing,
							int flags, IPoint2 *pScreenPoint, ViewExp *pVpt)
{
	// Return the result of the HitTest
	return 0;
}
		
int $$CLASS_NAME$$::Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags)
{

	return 0;
}	

 // Used for manipulator set manager, which is always active.
bool $$CLASS_NAME$$::AlwaysActive() 
{ 
	//TODO: Return whether the Manilpulator can remain active
	return false; 
}

TSTR& $$CLASS_NAME$$::GetManipName() 
{
	//TODO: Return the name of the Manipulator
	return  name;
}

DisplayState  $$CLASS_NAME$$::MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
{
	return kFullRedrawNeeded;
}
DisplayState  $$CLASS_NAME$$::MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
{
	return kFullRedrawNeeded;
}

$$ENDIF

$$IF(SIMPLE_MANIP)
void $$CLASS_NAME$$::UpdateShapes(TimeValue t, TSTR& toolTip)
{
	GenerateShapes(t);
}

void $$CLASS_NAME$$::GenerateShapes(TimeValue t)
{

}
$$ENDIF    

void $$CLASS_NAME$$::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
$$IF(SIMPLE_MANIP)
	SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
$$ENDIF
}

void $$CLASS_NAME$$::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
$$IF(SIMPLE_MANIP)
	SimpleManipulator::OnButtonDown(t, pVpt, m, flags, pHitData);
$$ENDIF

}
	
void $$CLASS_NAME$$::OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
$$IF(SIMPLE_MANIP)
	SimpleManipulator::OnButtonUp(t, pVpt, m, flags, pHitData);
$$ENDIF

}

$$IF(!SIMPLE_MANIP)
INode* $$CLASS_NAME$$::GetINode() 
{
	return mpINode; 
}

void $$CLASS_NAME$$::InitNodeName(TSTR& s) 
{
	s = GetObjectName();
}

ObjectState  $$CLASS_NAME$$::Eval(int)
{
	return ObjectState(this); 
}


Interval $$CLASS_NAME$$::ObjectValidity(TimeValue t)
{
	return FOREVER;
}
    

void $$CLASS_NAME$$::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{

}

void $$CLASS_NAME$$::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{

}

void $$CLASS_NAME$$::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{

}

$$ENDIF

CreateMouseCallBack* $$CLASS_NAME$$::GetCreateMouseCallBack(void) 
{ 
	$$CLASS_NAME$$CreateCB.SetObj(this);
	return(&$$CLASS_NAME$$CreateCB);
}



$$IF(!SIMPLE_MANIP)
RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}
$$ENDIF

RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir& remap) 
{
	$$CLASS_NAME$$* newob = new $$CLASS_NAME$$();	
	//TODO: Make a copy all the data and also clone all the references
$$IF(PARAM_MAPS)
$$IF(!SIMPLE_MANIP)	
	newob->ReplaceReference(0,$$PBLOCK$$->Clone(remap));
$$ELSE
	newob->ReplaceReference(0,mpPblock->Clone(remap));

$$ENDIF //PARAM_MAPS	
$$ENDIF //SIMPLE_MANIP
$$IF(SIMPLE_MANIP)
	newob->mValid.SetEmpty();
$$ENDIF //SIMPLE_TYPE	
	BaseClone(this, newob, remap);
	return(newob);
}

$$//--------------------------------------
$$//MATERIAL_TYPE //mcr_mf_mtl
$$//--------------------------------------
$$ELIF(MATERIAL_TYPE)

$$CLASS_NAME$$::$$CLASS_NAME$$(BOOL loading) 
{
	for (int i=0; i<NSUBMTL; i++) submtl[i] = NULL;
	pblock = NULL;

	if (!loading) 
		Reset();
}


void $$CLASS_NAME$$::Reset() 
{
	ivalid.SetEmpty();
	for (int i=0; i<NSUBMTL; i++) {
		if( submtl[i] ){ 
			DeleteReference(i);
			submtl[i] = NULL;
		}

	}

$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
}



ParamDlg* $$CLASS_NAME$$::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = $$CLASS_NAME$$Desc.CreateParamDlgs(hwMtlEdit, imp, this);
	
	// TODO: Set param block user dialog if necessary
	return masterDlg;
}

BOOL $$CLASS_NAME$$::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval $$CLASS_NAME$$::Validity(TimeValue t)
{
	Interval valid = FOREVER;		

	for (int i=0; i<NSUBMTL; i++) 
	{
		if (submtl[i]) 
			valid &= submtl[i]->Validity(t);
	}
	
	float u;
$$IF(PARAM_MAPS)
	pblock->GetValue(pb_spin,t,u,valid);
$$ENDIF //PARAM_MAPS
	return valid;
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

RefTargetHandle $$CLASS_NAME$$::GetReference(int i) 
{
	if (i < NSUBMTL )
		return submtl[i];
	else return pblock;
}

void $$CLASS_NAME$$::SetReference(int i, RefTargetHandle rtarg) 
{
	if (i < NSUBMTL)
		submtl[i] = (Mtl *)rtarg; 
	else pblock = (IParamBlock2 *)rtarg; 
}

TSTR $$CLASS_NAME$$::SubAnimName(int i) 
{
	if (i < NSUBMTL)
		return GetSubMtlTVName(i);
	else return TSTR(_T(""));
}

Animatable* $$CLASS_NAME$$::SubAnim(int i) {
	if (i < NSUBMTL)
		return submtl[i]; 
	else return pblock;
	}

RefResult $$CLASS_NAME$$::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI(changing_param);
				}
			break;

		}
	return REF_SUCCEED;
}

/*===========================================================================*\
 |	SubMtl get and set
\*===========================================================================*/

Mtl* $$CLASS_NAME$$::GetSubMtl(int i)
{
	if (i < NSUBMTL )
		return submtl[i];
	return NULL;
}

void $$CLASS_NAME$$::SetSubMtl(int i, Mtl *m)
{
	ReplaceReference(i,m);
	// TODO: Set the material and update the UI	
}

TSTR $$CLASS_NAME$$::GetSubMtlSlotName(int i)
{
	// Return i'th sub-material name 
	return _T(""); 
}

TSTR $$CLASS_NAME$$::GetSubMtlTVName(int i)
{
	return GetSubMtlSlotName(i);
}

/*===========================================================================*\
 |	Texmap get and set
 |  By default, we support none
\*===========================================================================*/

Texmap* $$CLASS_NAME$$::GetSubTexmap(int i)
{
	return NULL;
}

void $$CLASS_NAME$$::SetSubTexmap(int i, Texmap *m)
{
}

TSTR $$CLASS_NAME$$::GetSubTexmapSlotName(int i)
{
	return _T("");
}

TSTR $$CLASS_NAME$$::GetSubTexmapTVName(int i)
{
	// Return i'th sub-texture name 
	return GetSubTexmapSlotName(i);
}



/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult $$CLASS_NAME$$::Save(ISave *isave) { 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
	}	

IOResult $$CLASS_NAME$$::Load(ILoad *iload) { 
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	return IO_OK;
	}


/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir &remap) {
	$$CLASS_NAME$$ *mnew = new $$CLASS_NAME$$(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	mnew->ReplaceReference(NSUBMTL,remap.CloneRef(pblock));

	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBMTL; i++) {
		mnew->submtl[i] = NULL;
		if (submtl[i])
			mnew->ReplaceReference(i,remap.CloneRef(submtl[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

void $$CLASS_NAME$$::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void $$CLASS_NAME$$::Update(TimeValue t, Interval& valid) 
{	
	if (!ivalid.InInterval(t)) {

		ivalid.SetInfinite();
$$IF(PARAM_MAPS)
		pblock->GetValue( mtl_mat1_on, t, mapOn[0], ivalid);
		pblock->GetValue( pb_spin, t, spin, ivalid);

$$ENDIF
		for (int i=0; i<NSUBMTL; i++) {
			if (submtl[i]) 
				submtl[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void $$CLASS_NAME$$::SetAmbient(Color c, TimeValue t) {}		
void $$CLASS_NAME$$::SetDiffuse(Color c, TimeValue t) {}		
void $$CLASS_NAME$$::SetSpecular(Color c, TimeValue t) {}
void $$CLASS_NAME$$::SetShininess(float v, TimeValue t) {}
				
Color $$CLASS_NAME$$::GetAmbient(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetAmbient(mtlNum,backFace):Color(0,0,0);
}

Color $$CLASS_NAME$$::GetDiffuse(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetDiffuse(mtlNum,backFace):Color(0,0,0);
}

Color $$CLASS_NAME$$::GetSpecular(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetSpecular(mtlNum,backFace):Color(0,0,0);
}

float $$CLASS_NAME$$::GetXParency(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetXParency(mtlNum,backFace):Color(0,0,0);
}

float $$CLASS_NAME$$::GetShininess(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetShininess(mtlNum,backFace):Color(0,0,0);
}

float $$CLASS_NAME$$::GetShinStr(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetShinStr(mtlNum,backFace):Color(0,0,0);
}

float $$CLASS_NAME$$::WireSize(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->WireSize(mtlNum,backFace):Color(0,0,0);
}

		
/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void $$CLASS_NAME$$::Shade(ShadeContext& sc) 
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;
	if (gbufID) sc.SetGBufferID(gbufID);

	if(sm1) sm1->Shade(sc);
	
	// TODO: compute the color and transparency output returned in sc.out.
}

float $$CLASS_NAME$$::EvalDisplacement(ShadeContext& sc)
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;
	return (sm1)?sm1->EvalDisplacement(sc):0.0f;
}

Interval $$CLASS_NAME$$::DisplacementValidity(TimeValue t)
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;

	Interval iv; iv.SetInfinite();
	if(sm1) iv &= sm1->DisplacementValidity(t);

	return iv;	
}
	

$$//--------------------------------------
$$//MODIFIER_TYPE
$$//--------------------------------------
$$ELIF(MODIFIER_TYPE) //mcr_mf_mod

IObjParam *$$CLASS_NAME$$::ip			= NULL;
$$IF(!PARAM_MAPS)
HWND $$CLASS_NAME$$::hParams			= NULL;		
$$ENDIF //!PARAM_MAPS

$$IF(EXTENSION)

// TODO: Perform any Class Setup here.

XTC$$CLASS_NAME$$::XTC$$CLASS_NAME$$()
{

}

XTC$$CLASS_NAME$$::~XTC$$CLASS_NAME$$()
{

}

XTCObject * XTC$$CLASS_NAME$$::Clone()
{
	//TODO: Perform any class initialisation ready for a clone.

	return new XTC$$CLASS_NAME$$();
}	

int XTC$$CLASS_NAME$$::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj)
{
	//TODO: Add Extension Objects drawing routine here
	return 0;
}


$$IF(ADD_COMMENTS)
/******************************************************************************************************************
*
	This method will be called before a modifier in the pipleine changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/
$$ENDIF

void XTC$$CLASS_NAME$$::PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod)
{

}

$$IF(ADD_COMMENTS)
/******************************************************************************************************************
*
	This method will be called after a modifier in the pipleine changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/
$$ENDIF

void XTC$$CLASS_NAME$$::PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod)
{

}
	
BOOL XTC$$CLASS_NAME$$::SuspendObjectDisplay()
{
	//TODO:  Tell the system to display the Object or not
	return FALSE;
}

	
$$ENDIF



$$IF(SIMPLE_TYPE)

$$CLASS_NAME$$Deformer::$$CLASS_NAME$$Deformer() 
{ 
	
}

$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
 	Map is called for every deformable point in the object
*
\*************************************************************************************************/

$$ENDIF
Point3 $$CLASS_NAME$$Deformer::Map(int i, Point3 p)
{
	//TODO: Add code to deform or alter a single point
	return p;
}

$$ENDIF //SIMPLE_TYPE
//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF	
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
}

$$IF(!SIMPLE_TYPE)
$$IF(ADD_COMMENTS)
/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
$$ENDIF
Interval $$CLASS_NAME$$::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/
$$ENDIF

void $$CLASS_NAME$$::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}


$$IF(ADD_COMMENTS)

/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/
$$ENDIF

void $$CLASS_NAME$$::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}

$$IF(ADD_COMMENTS)

/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


$$ENDIF
void $$CLASS_NAME$$::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
$$IF(!EXTENSION)
	//TODO: Add the code for actually modifying the object
$$ENDIF
$$IF(EXTENSION)

	//TODO: Add the extension Object each time the object is modified
	XTC$$CLASS_NAME$$ *pObj = NULL;

	pObj = new XTC$$CLASS_NAME$$();

	os->obj->AddXTCObject(pObj);
	
	os->obj->SetChannelValidity(EXTENSION_CHAN_NUM, GetValidity(t));

$$ENDIF
}

$$ENDIF //Simple_type

void $$CLASS_NAME$$::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

$$IF(SIMPLE_TYPE)
	$$SUPER_CLASS_NAME$$::BeginEditParams(ip,flags,prev);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.BeginEditParams(ip, this, flags, prev);
$$ELSE
	if (!hParams) {
		hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PANEL),
				$$CLASS_NAME$$DlgProc, 
				GetString(IDS_PARAMS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams);
	} else {
		SetWindowLong(hParams,GWL_USERDATA,(LONG)this);
	}
$$ENDIF //PARAM_MAPS
}

void $$CLASS_NAME$$::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
$$IF(SIMPLE_TYPE)
	$$SUPER_CLASS_NAME$$::EndEditParams(ip,flags,next);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.EndEditParams(ip, this, flags, next);
$$ELSE
	ip->UnRegisterDlgWnd(hParams);
	ip->DeleteRollupPage(hParams);
	hParams = NULL;
$$ENDIF

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;
}



Interval $$CLASS_NAME$$::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}

$$IF(SIMPLE_TYPE)
Deformer& $$CLASS_NAME$$::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	static $$CLASS_NAME$$Deformer deformer;
	//TODO: Add code to modify the deformer object
	return deformer;
}
$$ENDIF //SIMPLE_TYPE



RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir& remap)
{
	$$CLASS_NAME$$* newmod = new $$CLASS_NAME$$();	
	//TODO: Add the cloning code here
$$IF(PARAM_MAPS)
	newmod->ReplaceReference(PBLOCK_REF,$$PBLOCK$$->Clone(remap));
$$ENDIF //PARAM_MAPS
$$IF(SIMPLE_TYPE)
	newmod->SimpleModClone(this);
$$ENDIF
	BaseClone(this, newmod, remap);
	return(newmod);
}


$$IF(!SIMPLE_TYPE)
//From ReferenceMaker 
RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

$$IF(ADD_COMMENTS)
/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/
$$ENDIF

void $$CLASS_NAME$$::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL $$CLASS_NAME$$::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void $$CLASS_NAME$$::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

$$ENDIF //!SIMPLE_TYPE
IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

$$IF(SIMPLE_TYPE)
/*===========================================================================*\
 |	SimpleMod2 overloads
\*===========================================================================*/

RefTargetHandle SimpleMod2::GetReference(int i) 
	{ 
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock2;
		default: return NULL;
		}
	}

void SimpleMod2::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock2 = (IParamBlock2*)rtarg; break;
		}
	}

Animatable* SimpleMod2::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock2;		
		default: return NULL;
		}
	}
$$ENDIF
$$//--------------------------------------
$$//NURBS_OBJECT_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//PARTICLE_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//PATCH_OBJECT_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//PROCEDURAL_OBJECT_TYPE mcr_mf_object
$$//--------------------------------------
$$ELIF(PROCEDURAL_OBJECT_TYPE)

IObjParam *$$CLASS_NAME$$::ip			= NULL;
$$IF(!PARAM_MAPS)
HWND $$CLASS_NAME$$::hParams			= NULL; 
$$ENDIF //PARAM_MAPS

//--- $$CLASS_NAME$$ -------------------------------------------------------

$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

void $$CLASS_NAME$$::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	this->ip = ip;

$$IF(SIMPLE_TYPE)
	$$SUPER_CLASS_NAME$$::BeginEditParams(ip,flags,prev);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.BeginEditParams(ip, this, flags, prev);
$$ELSE
	if (!hParams) {
		hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PANEL),
				$$CLASS_NAME$$DlgProc, 
				GetString(IDS_PARAMS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams);
	} else {
		SetWindowLong(hParams,GWL_USERDATA,(LONG)this);
	}
$$ENDIF //PARAM_MAPS
}

void $$CLASS_NAME$$::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	//TODO: Save plugin parameter values into class variables, if they are not hosted in ParamBlocks. 
	

$$IF(SIMPLE_TYPE)
	$$SUPER_CLASS_NAME$$::EndEditParams(ip,flags,next);
$$ENDIF //SIMPLE_TYPE
$$IF(PARAM_MAPS)	
	$$CLASS_NAME$$Desc.EndEditParams(ip, this, flags, next);
$$ELSE
	if (flags&END_EDIT_REMOVEUI) {
		ip->UnRegisterDlgWnd(hParams);
		ip->DeleteRollupPage(hParams);
		hParams = NULL;
	} else {
		SetWindowLong(hParams,GWL_USERDATA,(LONG)NULL);
		}	
$$ENDIF //PARAM_MAPS

	this->ip = NULL;
}

//From Object
BOOL $$CLASS_NAME$$::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void $$CLASS_NAME$$::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin's internal value to sw				
}

//Class for interactive creation of the object using the mouse
class $$CLASS_NAME$$CreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;		//First point in screen coordinates
	$$CLASS_NAME$$ *ob;		//Pointer to the object 
	Point3 p0;			//First point in world coordinates
public:	
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj($$CLASS_NAME$$ *obj) {ob = obj;}
};

int $$CLASS_NAME$$CreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	//TODO: Implement the mouse creation code here
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0: // only happens with MOUSE_POINT msg
			ob->suspendSnap = TRUE;
			sp0 = m;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
			break;
		//TODO: Add for the rest of points
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static $$CLASS_NAME$$CreateCallBack $$CLASS_NAME$$CreateCB;

//From BaseObject
CreateMouseCallBack* $$CLASS_NAME$$::GetCreateMouseCallBack() 
{
	$$CLASS_NAME$$CreateCB.SetObj(this);
	return(&$$CLASS_NAME$$CreateCB);
}

$$IF(!SIMPLE_TYPE)
int $$CLASS_NAME$$::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	//TODO: Implement the displaying of the object here
	return 0;
}

int $$CLASS_NAME$$::HitTest(TimeValue t, INode* inode, int type, int crossing, 
	int flags, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Implement the hit testing here
	return 0;
}

void $$CLASS_NAME$$::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Check the point passed for a snap and update the SnapInfo structure
}

void $$CLASS_NAME$$::GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the world space bounding box of the object
}

void $$CLASS_NAME$$::GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the local space bounding box of the object
}

void $$CLASS_NAME$$::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	//TODO: Compute the bounding box in the objects local coordinates 
	//		or the optional space defined by tm.
}

//From ReferenceMaker
RefResult $$CLASS_NAME$$::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID, RefMessage message )
{
	//TODO: Implement, if the object makes references to other things
	return(REF_SUCCEED);
}

Mesh* $$CLASS_NAME$$::GetRenderMesh(TimeValue t, 
						INode *inode, View& view, BOOL& needDelete)
{
	//TODO: Return the mesh representation of the object used by the renderer
	return NULL;
}

$$ELSE
//From SimpleObject
void $$CLASS_NAME$$::BuildMesh(TimeValue t)
{
	//TODO: Implement the code to build the mesh representation of the object
	//		using its parameter settings at the time passed. The plug-in should 
	//		use the data member mesh to store the built mesh.
}

BOOL $$CLASS_NAME$$::OKtoDisplay(TimeValue t) 
{
	//TODO: Check whether all the parameters have valid values and return the state
	return TRUE;
}

void $$CLASS_NAME$$::InvalidateUI() 
{
	// Hey! Update the param blocks
}
$$ENDIF //!SIMPLE_TYPE

Object* $$CLASS_NAME$$::ConvertToType(TimeValue t, Class_ID obtype)
{
	//TODO: If the plugin can convert to a nurbs surface then check to see 
	//		whether obtype == EDITABLE_SURF_CLASS_ID and convert the object
	//		to nurbs surface and return the object
	
$$IF(SIMPLE_TYPE)		
	return SimpleObject::ConvertToType(t,obtype);
$$ENDIF //SIMPLE_TYPE
	return NULL;
}

int $$CLASS_NAME$$::CanConvertToType(Class_ID obtype)
{
	//TODO: Check for any additional types the plugin supports
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID) {
		return 1;
	} else {		
$$IF(SIMPLE_TYPE)		
	return SimpleObject::CanConvertToType(obtype);
$$ELSE
	return 0;
$$ENDIF //SIMPLE_TYPE
		}
}

// From Object
int $$CLASS_NAME$$::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
{
	//TODO: Return TRUE after you implement this method
	return FALSE;
}

void $$CLASS_NAME$$::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
	//TODO: Append any any other collapse type the plugin supports
}

// From ReferenceTarget
RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir& remap) 
{
	$$CLASS_NAME$$* newob = new $$CLASS_NAME$$();	
	//TODO: Make a copy all the data and also clone all the references
$$IF(PARAM_MAPS)	
	newob->ReplaceReference(0,$$PBLOCK$$->Clone(remap));
$$ENDIF //PARAM_MAPS	
$$IF(SIMPLE_TYPE)
	newob->ivalid.SetEmpty();
$$ENDIF //SIMPLE_TYPE	
	BaseClone(this, newob, remap);
	return(newob);
}

$$ELIF(SKIN_GIZMO_TYPE)

$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)	
$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF	

}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

void $$CLASS_NAME$$::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	$$CLASS_NAME$$Desc.BeginEditParams(ip, this, flags, prev);
}

void $$CLASS_NAME$$::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	$$CLASS_NAME$$Desc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;

}

$$IF(ADD_COMMENTS)
/*******************************************************************************************
*
	This is called by the Skin Modifier to receive the name at creation of the gizmo
	It will appear in Skin's Gizmo list box
*
********************************************************************************************/

$$ENDIF
void $$CLASS_NAME$$::SetInitialName()
{
	//TODO: Set the initial name to appear in the gizmo list
	pblock_gizmo_data->SetValue(pb_gizmoparam_name,0,_T("Wizard Gizmo"));

}

$$IF(ADD_COMMENTS)
/********************************************************************************************
*
	The following functions give access to the name of the Gizmo by accessing the
	Parameter block storing the name
*
*********************************************************************************************/

$$ENDIF
TCHAR * $$CLASS_NAME$$::GetName()
{
	TCHAR *name;
	pblock_gizmo_data->GetValue(pb_gizmoparam_name,0,name,FOREVER);
	return name;
}

void $$CLASS_NAME$$::SetName(TCHAR *name)
{
	pblock_gizmo_data->SetValue(pb_gizmoparam_name,0,name);
	
}


BOOL $$CLASS_NAME$$::IsEnabled()
{
	//TODO:  Tell the system whether the gizmo is Enabled or not
	return TRUE;
}
		
BOOL $$CLASS_NAME$$::IsVolumeBased()
{
	//TODO: Tell the system whether the gizmo works on a volume basis
	return FALSE;
}

BOOL $$CLASS_NAME$$::IsInVolume(Point3 p, Matrix3 tm)
{
	//TODO:  Check whether the past in point lies in the volume
	return TRUE;
}

BOOL $$CLASS_NAME$$::IsEditing()
{
	return TRUE;
}
		
void $$CLASS_NAME$$::Enable(BOOL enable)
{
	//TODO:  Enable or disable the Gizmo
}

void $$CLASS_NAME$$::EndEditing()
{

}

void $$CLASS_NAME$$::EnableEditing(BOOL enable)
{

}

		
IGizmoBuffer * $$CLASS_NAME$$::CopyToBuffer() 
{
	return 0;
}
		
void $$CLASS_NAME$$::PasteFromBuffer(IGizmoBuffer *buffer)
{

}

$$IF(ADD_COMMENTS)
/************************************************************************************************
*
	This is called when the gizmo is initially created it is passed to the current selected 
	verts in the world space.

	count is the number of vertice in *p

	*p is the list of point being affected in world space

	numberOfInstances is the number of times this modifier has been instanced

	mapTable is an index list into the original vertex table for *p
*
**************************************************************************************************/

$$ENDIF

BOOL $$CLASS_NAME$$::InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable)
{

	return TRUE;
}

$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	This is called before the deformation on a frame to allow the gizmo to do some
	initial setup
*
**************************************************************************************************/
$$ENDIF

void $$CLASS_NAME$$::PreDeformSetup(TimeValue t)
{
	//TODO:Perform any pre deform initialization

}

void $$CLASS_NAME$$::PostDeformSetup(TimeValue t)
{
	//TODO: Perform any clean up

}	

$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	This is what deforms the point
	It is passed in from the Map call from the Skin modifier
*
**************************************************************************************************/	
$$ENDIF

Point3 $$CLASS_NAME$$::DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)
{
	//TODO: Defrom the Point
	return Point3(0,0,0);
}

$$IF(ADD_COMMENTS)
/**************************************************************************************************
*
  This retrieves the boudng box of the gizmo in world space
*
**************************************************************************************************/

$$ENDIF

void $$CLASS_NAME$$::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{
	//TODO: Calculate the bounding box of the Gizmo
}

$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	This is called in the skin modifier's display code to show the actual gizmo
*
***************************************************************************************************/

$$ENDIF	

int $$CLASS_NAME$$::Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm )
{
	//TODO: Draw the physical representation of the Gizmo
	return 1;
}


$$IF(ADD_COMMENTS)
/*************************************************************************************************
*
	The following functions are standard subobject selection and manipulation routines
*
**************************************************************************************************/

$$ENDIF

int $$CLASS_NAME$$::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm)
{
	//TODO:  Perform the gizmo's hit testing and return the result
	return 0;
}

void $$CLASS_NAME$$::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
{
	//TODO: Perform the actual selection based on the HitRecord
	
}

void $$CLASS_NAME$$::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin ) 
{
	//TODO: Perform the transformation of the selected object
}

void $$CLASS_NAME$$::GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{
	//TODO: Update the centres of the selected Objected
}

void $$CLASS_NAME$$::GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) 
{
	//TODO: Update the Tranformation matrices of the selected objects
}

void $$CLASS_NAME$$::ClearSelection(int selLevel)
{
	//TODO: Clear the current selection 
}

void $$CLASS_NAME$$::SelectAll(int selLevel)
{
	//TODO: Select all the subobject components at the currect sub object level
}

void $$CLASS_NAME$$::InvertSelection(int selLevel)
{
	//TODO: Invert the current selection at the currect sub object level
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	//TODO: Standard loading of data
	return IO_OK;
}


IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	//TODO: standard saving of data
	return IO_OK;
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}


$$//--------------------------------------
$$//RENDERER_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//RENDER_EFFECT_TYPE
$$//--------------------------------------
$$ELIF(RENDER_EFFECT_TYPE)//mcr_mf_effect
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
	assert(pblock);
$$ENDIF //PARAM_MAPS	
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
}


/*===========================================================================*\
 |	Standard Load
\*===========================================================================*/

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	Effect::Save(isave);
	return IO_OK;
}

RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* newObj = new $$CLASS_NAME$$();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}
/*===========================================================================*\
 |	Ask the ClassDesc2 to make the AUTO_UI $$CLASS_NAME$$DlgProc
\*===========================================================================*/

EffectParamDlg *$$CLASS_NAME$$::CreateParamDialog(IRendParams *ip)
{		
	return $$CLASS_NAME$$Desc.CreateParamDialogs(ip, this);
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI();
			break;
	}
	return REF_SUCCEED;
}


/*===========================================================================*\
 |	This method is called once per frame when the renderer begins.  
\*===========================================================================*/

void $$CLASS_NAME$$::Update(TimeValue t, Interval& valid)
{
	//TODO: Implement the actual effect
}


/*===========================================================================*\
 |	Called to initialize or clear up before and after rendering  
\*===========================================================================*/

int $$CLASS_NAME$$::RenderBegin(TimeValue t, ULONG flags)
{
	return 0;
}

int $$CLASS_NAME$$::RenderEnd(TimeValue t)
{
	return 0;
}

/*===========================================================================*\
 |	Apply the actual changes to the rendered bitmap at time 't'
 *===========================================================================*/

void $$CLASS_NAME$$::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc,CheckAbortCallback *checkAbort) 
{

}
$$//--------------------------------------
$$//SAMPLER_TYPE
$$//--------------------------------------
$$ELIF(SAMPLER_TYPE)//mcr_mf_sampler
/*===========================================================================*\
 |	Sampler Implimentation
\*===========================================================================*/


$$CLASS_NAME$$::$$CLASS_NAME$$()
{
$$IF(PARAM_MAPS)
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* sksNew = new $$CLASS_NAME$$();
	sksNew->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, sksNew, remap);
	return (RefTargetHandle)sksNew;
}


IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	Sampler::Load(iload);
	return IO_OK;
}


IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	Sampler::Save(isave);
	return IO_OK;
}

TCHAR* $$CLASS_NAME$$::GetDefaultComment()
{
	return GetString(IDS_COMMENT);
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

void $$CLASS_NAME$$::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK pMask)
{
// TODO: Perform the actual sampling
}

BOOL $$CLASS_NAME$$::NextSample( Point2* pOut, float* pSampleSz, int n )
{
	//TODO: Find next sample point
	return TRUE;
}


int $$CLASS_NAME$$::GetNSamples()
{
	// TODO: Return number of samples
	return 1;
}

/*===========================================================================*\
 |	Quality and Enabling support
\*===========================================================================*/

void $$CLASS_NAME$$::SetQuality( float q )
{ 
	pblock->SetValue( pb_quality, 0, q );
}

float $$CLASS_NAME$$::GetQuality()
{ 
	return pblock->GetFloat( pb_quality, 0);
}

int $$CLASS_NAME$$::SupportsQualityLevels() 
{
	return 1; 
}

void $$CLASS_NAME$$::SetEnable( BOOL on )
{ 
	pblock->SetValue( pb_enable, 0, on );
}

BOOL $$CLASS_NAME$$::GetEnable()
{ 
	BOOL b; Interval valid;
	pblock->GetValue( pb_enable, 0, b, valid );
	return b;
}

/*===========================================================================*\
 |	Adaptive Sampling support
\*===========================================================================*/


void $$CLASS_NAME$$::SetAdaptiveOn( BOOL on )
{ 
	pblock->SetValue( pb_adapt_enable, 0, on ); 
}

BOOL $$CLASS_NAME$$::IsAdaptiveOn()
{ 
	BOOL b; Interval valid;
	pblock->GetValue( pb_adapt_enable, 0, b, valid );
	return b;
}

void $$CLASS_NAME$$::SetAdaptiveThreshold( float val )
{ 
	pblock->SetValue( pb_adapt_threshold, 0, val ); 
}

float $$CLASS_NAME$$::GetAdaptiveThreshold() 
{ 
	return pblock->GetFloat( pb_adapt_threshold, 0);
}
$$//--------------------------------------
$$//SPLINE_SHAPE_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//SHADER_TYPE_TYPE
$$//--------------------------------------
$$ELIF(SHADER_TYPE)//mcr_mf_shader
#define SHADER_NCOLBOX 1

class $$CLASS_NAME$$Dlg : public ShaderParamDlg {
public:
	
	$$CLASS_NAME$$*	pShader;
	StdMat2*		pMtl;
	HPALETTE		hOldPal;
	HWND			hwmEdit;
	IMtlParams*		pMtlPar;
	HWND			hwHilite;
	HWND			hRollup;
	TimeValue		curTime;
	BOOL			valid;
	BOOL			isActive;


	IColorSwatch *cs[SHADER_NCOLBOX];
	ISpinnerControl *brSpin, *trSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	

	$$CLASS_NAME$$Dlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~$$CLASS_NAME$$Dlg(); 


	// DnD: Required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i<NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return texmapFromMBut[i];
		}	
		return -1;
	}


	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return $$CLASS_NAME_UPPER$$_CLASS_ID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	pShader = ($$CLASS_NAME$$*)theShader; 
		if (pShader)pShader->SetParamDlg(this,0); 
		pMtl = theMtl; 
	}


	ReferenceTarget* GetThing(){ return (ReferenceTarget*)pMtl; }
	Shader* GetShader(){ return pShader; }
	
	void SetTime(TimeValue t) 
	{ 
		if (!pShader->ivalid.InInterval(t)) {
			Interval v;
			pShader->Update(t,v);
			LoadDialog(TRUE); 
		}
		curTime = t; 
	}		
	BOOL KeyAtCurTime(int id) { return pShader->KeyAtTime(id,curTime); } 

	void DeleteThis() { delete this; }
	void ActivateDlg( BOOL dlgOn ){ isActive = dlgOn; }
	HWND GetHWnd(){ return hRollup; }

	void NotifyChanged(){ pShader->NotifyChanged(); }
	void LoadDialog(BOOL draw);
	void ReloadDialog(){ Interval v; pShader->Update(pMtlPar->GetTime(), v); LoadDialog(FALSE);}
	void UpdateDialog( ParamID paramId ){ ReloadDialog(); }

	void UpdateMtlDisplay(){ pMtlPar->MtlChanged(); }
    void UpdateHilite( );
	void UpdateColSwatches();
	void UpdateMapButtons();
	void UpdateOpacity();

	void SelectEditColor(int i) { cs[ i ]->EditThis(FALSE); }
};

static BOOL CALLBACK  $$CLASS_NAME$$DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	$$CLASS_NAME$$Dlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = ($$CLASS_NAME$$Dlg*)lParam;
		SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
	} else {
	    if ( (theDlg = ($$CLASS_NAME$$Dlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* $$CLASS_NAME$$::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen,int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	$$CLASS_NAME$$Dlg *pDlg = new $$CLASS_NAME$$Dlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );


	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			$$CLASS_NAME$$DlgProc, 
			GetString(IDS_PARAMS),
			(LPARAM)pDlg , 
			rollupOpen
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			$$CLASS_NAME$$DlgProc, 
			GetString(IDS_PARAMS),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									  PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock){
				// update UI if paramblock changed, possibly from scripter
				ParamID changingParam = pblock->LastNotifyParamID();
				// reload the dialog if present
				if (paramDlg){
					paramDlg->UpdateDialog(changingParam);
				}
				// notify our dependents that we've changed
				NotifyChanged();
			}
			break;
	}
	return(REF_SUCCEED);
}

static void VertLine(HDC hdc,int x, int ystart, int yend) 
{
	MoveToEx(hdc, x, ystart, NULL); 
	if (ystart <= yend)
		LineTo(hdc, x, yend+1);
	else 
		LineTo(hdc, x, yend-1);
}

void DrawHilite(HDC hdc, Rect& rect, Shader* pShader )
{
int w,h,npts,xcen,ybot,ytop,ylast,i,iy;

	HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
	HPEN fgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));
	HPEN bgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNSHADOW));

	w = rect.w();
	h = rect.h()-3;
	npts = (w-2)/2;
	xcen = rect.left+npts;
	ybot = rect.top+h;
	ytop = rect.top+2;
	ylast = -1;
	for (i=0; i<npts; i++) {
		float v = pShader->EvalHiliteCurve( (float)i/((float)npts*2.0f) );
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = ybot-(int)(v*((float)h-2.0f));

		if (iy<ytop) iy = ytop;

		SelectPen(hdc, fgPen);
		VertLine(hdc,xcen+i,ybot,iy);
		VertLine(hdc,xcen-i,ybot,iy);

		if (iy-1>ytop) {
			// Fill in above curve
			SelectPen(hdc,bgPen);
			VertLine(hdc,xcen+i, ytop, iy-1);
			VertLine(hdc,xcen-i, ytop, iy-1);
			}
		if (ylast>=0) {
			SelectPen(hdc,linePen);
			VertLine(hdc,xcen+i-1,iy-1,ylast);
			VertLine(hdc,xcen-i+1,iy-1,ylast);
			}

		ylast = iy;
	}

	SelectObject( hdc, linePen );
	DeleteObject(fgPen);
	DeleteObject(bgPen);
	WhiteRect3D(hdc, rect, 1);
}

LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLong(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLong(hwParent, GWL_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_LBUTTONUP: 
		case WM_CREATE:
		case WM_DESTROY: 
		break;

		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = (Shader*)(theDlg->GetShader());
				DrawHilite(hdc, rect, pShader );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 


$$CLASS_NAME$$Dlg::$$CLASS_NAME$$Dlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	brSpin = trSpin = NULL;
	hRollup = hwHilite = NULL;
	curTime = 0;
	isActive = valid = FALSE;

	for( long i = 0; i < SHADER_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

$$CLASS_NAME$$Dlg::~$$CLASS_NAME$$Dlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}
	
	ReleaseISpinner(brSpin);
	ReleaseISpinner(trSpin);

	SetWindowLong(hRollup, GWL_USERDATA, NULL);
	SetWindowLong(hwHilite, GWL_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}


void $$CLASS_NAME$$Dlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		brSpin->SetValue( pShader->GetBrightness() ,FALSE);
		brSpin->SetKeyBrackets(KeyAtCurTime(pb_brightness));
		
		trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		UpdateColSwatches();
		UpdateHilite();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void $$CLASS_NAME$$Dlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}


void $$CLASS_NAME$$Dlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}

void $$CLASS_NAME$$Dlg::UpdateColSwatches() 
{
	cs[0]->SetKeyBrackets( pShader->KeyAtTime(pb_diffuse,curTime) );
	cs[0]->SetColor( pShader->GetDiffuseClr() );
}


void $$CLASS_NAME$$Dlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}



static int ColorIDCToIndex(int idc) {
	switch (idc) {
		case IDC_COLOR: return 0;
		default: return 0;
	}
}


BOOL $$CLASS_NAME$$Dlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			HWND hwndCS = GetDlgItem(hwndDlg, IDC_COLOR);
			cs[0] = GetIColorSwatch( hwndCS, pShader->GetDiffuseClr(), GetString(IDS_COLOR) );

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLong( hwHilite, GWL_WNDPROC, (LONG)HiliteWndProc);

			brSpin = SetupFloatSpinner(hwndDlg, IDC_BR_SPIN, IDC_BR_EDIT, 0.0f,1.0f, 0);
			brSpin->SetScale(0.01f);

			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}
			LoadDialog(TRUE);
		}
		break;

		case WM_COMMAND: 
			{
			for ( int i=0; i<NMBUTS; i++) {
				if (id == texMButtonsIDC[i]) {
					PostMessage(hwmEdit,WM_TEXMAP_BUTTON, texmapFromMBut[i],(LPARAM)pMtl );
					UpdateMapButtons();
					goto exit;
					}
				}
			}

			break;

		case CC_COLOR_SEL: {
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
		}			
		break;
		case CC_COLOR_DROP:	{
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
		}
		break;
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
		 break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_PARAMCHG));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE: {			
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = ColorIDCToIndex(id);
			if (buttonUp) theHold.Begin();
			DWORD curRGB = cs[n]->GetColor();
			pShader->SetDiffuseClr(curRGB, curTime); 
			if (buttonUp) {
				theHold.Accept(GetString(IDS_PARAMCHG));
				UpdateMtlDisplay();				
				}
		} break;
		case WM_PAINT: 
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_CLOSE:
		case WM_DESTROY: 
			break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_BR_SPIN: 
					pShader->SetBrightness( brSpin->GetFVal() , curTime); 
					UpdateHilite();
					break;
				case IDC_TR_SPIN: 
					pMtl->SetOpacity(PcToFrac( trSpin->GetIVal()),curTime); 
					break;
			}
			UpdateMtlDisplay();
		break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_PARAMCHG));
			else 
				theHold.Cancel();
			UpdateMtlDisplay();
			break;

    }
	exit:
	return FALSE;
	}

/*===========================================================================*\
 |	Constructor
\*===========================================================================*/

$$CLASS_NAME$$::~$$CLASS_NAME$$() 
{ 

}

$$CLASS_NAME$$::$$CLASS_NAME$$() 
{ 
	pblock = NULL; 
	paramDlg = NULL; 
	curTime = 0;
	ivalid.SetEmpty(); 
}



/*===========================================================================*\
 |	Cloning and coping standard parameters
\*===========================================================================*/

void $$CLASS_NAME$$::CopyStdParams( Shader* pFrom )
{
	// We don't want to see this parameter copying in macrorecorder
	macroRecorder->Disable(); 

		SetAmbientClr( pFrom->GetAmbientClr(0,0), curTime );
		SetDiffuseClr( pFrom->GetDiffuseClr(0,0), curTime );

	macroRecorder->Enable();
	ivalid.SetEmpty();	
}


RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* mnew = new $$CLASS_NAME$$();
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->diffuse = diffuse;
	mnew->brightness = brightness;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}



/*===========================================================================*\
 |	Shader state
\*===========================================================================*/

void $$CLASS_NAME$$::Update(TimeValue t, Interval &valid) {
	Point3 p;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		pblock->GetValue( pb_diffuse, t, p, ivalid );
		diffuse= Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( pb_brightness, t, brightness, ivalid );
		brightness = Bound(brightness );
		curTime = t;
	}
	valid &= ivalid;
}

void $$CLASS_NAME$$::Reset()
{
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);	// Create and intialize paramblock2

	ivalid.SetEmpty();
	SetDiffuseClr( Color(0.8f,0.5f,0.5f), 0 );
	SetBrightness( 0.2f,0);   
}



/*===========================================================================*\
 |	Shader load/save
\*===========================================================================*/


#define SHADER_VERS_CHUNK 0x6500

IOResult $$CLASS_NAME$$::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = 1;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	return IO_OK;
}		



IOResult $$CLASS_NAME$$::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

/*===========================================================================*\
 |	Actually shade the surface
\*===========================================================================*/

void $$CLASS_NAME$$::GetIllumParams( ShadeContext &sc, IllumParams &ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[S_DI] = diffuse;
	ip.channels[S_BR].r = brightness;
}

void $$CLASS_NAME$$::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *l;
	Color lightCol;

	Color Cd		= ip.channels[ S_DI ];
	float bright	= ip.channels[ S_BR ].r;

	BOOL is_shiny	= (bright > 0.0f) ? 1:0; 
	double phExp	= pow(2.0, bright * 10.0) * 4.0; 

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (l->Illuminate(sc,sc.Normal(),lightCol,L,NL,diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			if (l->affectDiffuse)
				ip.diffIllumOut += diffCoef * lightCol;

			if (is_shiny&&l->affectSpecular) {
				Point3 H = Normalize(L-sc.V());
				float c = DotProd(sc.Normal(),H);	 
				if (c>0.0f) {
					c = (float)pow((double)c, phExp); 
					ip.specIllumOut += c * bright * lightCol;
					}
				}
 			}
		}



	// now we can multiply by the clrs
	ip.ambIllumOut *= Cd * 0.5f; 
	ip.diffIllumOut *= Cd;
	CombineComponents( sc, ip ); 

}


void $$CLASS_NAME$$::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) 
{ 
}


void $$CLASS_NAME$$::CombineComponents( ShadeContext &sc, IllumParams& ip )
{ 
	ip.finalC = ip.finalOpac * (ip.ambIllumOut + ip.diffIllumOut  + ip.selfIllumOut)
			+ ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut; 
}


float $$CLASS_NAME$$::EvalHiliteCurve(float x) 
{
	double phExp	= pow(2.0, brightness * 10.0) * 4.0; 
	return 1.0f*(float)pow((double)cos(x*PI), phExp );  
}


BOOL $$CLASS_NAME$$::IsMetal() 
{ 
	return FALSE; 
}

/*===========================================================================*\
 |	Shader specific transaction
\*===========================================================================*/

void $$CLASS_NAME$$::SetBrightness(float v, TimeValue t)
{ 
	brightness= v; 
	pblock->SetValue( pb_brightness, t, v); 
}

float $$CLASS_NAME$$::GetBrightness(int mtlNum, BOOL backFace)
{ 
	return brightness; 
}	

float $$CLASS_NAME$$::GetBrightness( TimeValue t)
{
	return pblock->GetFloat(pb_brightness,t);  
}		

void $$CLASS_NAME$$::SetGlossiness(float v, TimeValue t)
{ 
}

float $$CLASS_NAME$$::GetGlossiness(int mtlNum, BOOL backFace)
{ 
	return 40.0f; 
}	

float $$CLASS_NAME$$::GetGlossiness( TimeValue t)
{
	return 40.0f;  
}		


/*===========================================================================*\
 |	Diffuse Transactions
\*===========================================================================*/

void $$CLASS_NAME$$::SetDiffuseClr(Color c, TimeValue t)		
{ 
	diffuse = c; pblock->SetValue( pb_diffuse, t, c); 
}

Color $$CLASS_NAME$$::GetDiffuseClr(int mtlNum, BOOL backFace)
{ 
	return diffuse;
}

Color $$CLASS_NAME$$::GetDiffuseClr(TimeValue t)
{ 
	return pblock->GetColor(pb_diffuse,t); 
}


/*===========================================================================*\
 |	Ambient Transactions
\*===========================================================================*/

void $$CLASS_NAME$$::SetAmbientClr(Color c, TimeValue t)
{
}

Color $$CLASS_NAME$$::GetAmbientClr(int mtlNum, BOOL backFace)
{ 
	return diffuse * 0.5f;
}

Color $$CLASS_NAME$$::GetAmbientClr(TimeValue t)
{ 
	return diffuse * 0.5f; 
}


/*===========================================================================*\
 |	Specular Transactions
\*===========================================================================*/

void $$CLASS_NAME$$::SetSpecularClr(Color c, TimeValue t)
{
}

void $$CLASS_NAME$$::SetSpecularLevel(float v, TimeValue t)
{
}
		
Color $$CLASS_NAME$$::GetSpecularClr(int mtlNum, BOOL backFace)
{ 
	return Color(0.9f,0.9f,0.9f); 
}

float $$CLASS_NAME$$::GetSpecularLevel(int mtlNum, BOOL backFace)
{ 
	return 1.0f; 
}

Color $$CLASS_NAME$$::GetSpecularClr(TimeValue t)
{ 
	return Color(0.9f,0.9f,0.9f);
}

float $$CLASS_NAME$$::GetSpecularLevel(TimeValue t)
{ 
	return 1.0f; 
}


/*===========================================================================*\
 |	SelfIllum Transactions
\*===========================================================================*/

void $$CLASS_NAME$$::SetSelfIllum(float v, TimeValue t)	
{
}

float $$CLASS_NAME$$::GetSelfIllum(int mtlNum, BOOL backFace)
{ 
	return 0.0f; 
}

void $$CLASS_NAME$$::SetSelfIllumClrOn( BOOL on )
{
}

BOOL $$CLASS_NAME$$::IsSelfIllumClrOn()
{ 
	return FALSE; 
}

BOOL $$CLASS_NAME$$::IsSelfIllumClrOn(int mtlNum, BOOL backFace)
{ 
	return FALSE; 
}

void $$CLASS_NAME$$::SetSelfIllumClr(Color c, TimeValue t)
{
}

Color $$CLASS_NAME$$::GetSelfIllumClr(int mtlNum, BOOL backFace)
{ 
	return Color(0,0,0); 
}

float $$CLASS_NAME$$::GetSelfIllum(TimeValue t)
{ 
	return 0.0f;
}		

Color $$CLASS_NAME$$::GetSelfIllumClr(TimeValue t)
{ 
	return Color(0,0,0);
}		


/*===========================================================================*\
 |	Soften Transactions
\*===========================================================================*/

void $$CLASS_NAME$$::SetSoftenLevel(float v, TimeValue t)
{
}

float $$CLASS_NAME$$::GetSoftenLevel(int mtlNum, BOOL backFace)
{ 
	return 0.0f; 
}

float $$CLASS_NAME$$::GetSoftenLevel(TimeValue t)
{ 
	return  0.0f; 
}

$$//--------------------------------------
$$//SHADOW_TYPE mcr_mf_shadow
$$//--------------------------------------
$$ELIF(SHADOW_TYPE)
$$CLASS_NAME$$::$$CLASS_NAME$$()
	{
	theParam = NULL;
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
	assert(pblock);
	}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{
	if (theParam) theParam->theShad = NULL;
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI();
			break;
		}
	return REF_SUCCEED;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

RefTargetHandle $$CLASS_NAME$$::Clone( RemapDir &remap )
{
	$$CLASS_NAME$$* newObj = new $$CLASS_NAME$$();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}

int $$CLASS_NAME$$Gen::Update(TimeValue t,const RendContext& rendCntxt,RenderGlobalContext *rgc,
		Matrix3& ltToWorld,float aspect,float param,float clipDist)
{
	lightToWorld  = ltToWorld;
	worldToLight = Inverse(lightToWorld);

	return 1;
}

float $$CLASS_NAME$$Gen::Sample(ShadeContext &sc, Point3 &norm, Color& color)
{ 
	return 1.0f;
}

float $$CLASS_NAME$$Gen::Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope)
{
	return 1.0f;
}

/*===========================================================================*\
 |	Support the Parammap UI
\*===========================================================================*/

$$CLASS_NAME$$Dlg::$$CLASS_NAME$$Dlg($$CLASS_NAME$$ *shad, Interface *intface) {
	theShad = shad;
	ip = intface;

	pmap = CreateCPParamMap2(
		theShad->pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		GetString(IDS_PARAMS),
		0);
	}

$$CLASS_NAME$$Dlg::~$$CLASS_NAME$$Dlg(){
	if(theShad) theShad->theParam = NULL;
	DestroyCPParamMap2(pmap);
	}

$$//--------------------------------------
$$//SOUND_TYPE
$$//--------------------------------------
$$//--------------------------------------
$$//SPACE_WARP_TYPE
$$//--------------------------------------
$$ELIF(SPACE_WARP_TYPE)

/*===========================================================================*\
 |	$$CLASS_NAME$$Deformer class defn
 |  This is the system that gets applied to each deformable point
 |
\*===========================================================================*/

class $$CLASS_NAME$$Deformer: public Deformer 
{
	public:
		
		$$CLASS_NAME$$Deformer() {}
		Point3 Map(int i, Point3 p); 
};

/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

$$CLASS_NAME$$Object::$$CLASS_NAME$$Object()
{
	$$CLASS_NAME$$ObjDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
}



/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void $$CLASS_NAME$$Object::InvalidateUI()
{
	$$CLASS_NAME_LOWER$$obj_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}




/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void $$CLASS_NAME$$Object::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	$$CLASS_NAME$$ObjDesc.BeginEditParams(ip, this, flags, prev);


}
		
void $$CLASS_NAME$$Object::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleWSMObject::EndEditParams(ip,flags,next);
	$$CLASS_NAME$$ObjDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}




/*===========================================================================*\
 |	Standard Load and clone
\*===========================================================================*/

IOResult $$CLASS_NAME$$Object::Load(ILoad *iload)
{
	SimpleWSMObject::Load(iload);
	return IO_OK;
}

RefTargetHandle $$CLASS_NAME$$Object::Clone(RemapDir& remap) 
{	
	$$CLASS_NAME$$Object* newob = new $$CLASS_NAME$$Object();	
	newob->ReplaceReference(0,pblock2->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
}



/*===========================================================================*\
 |	Create the WSM modifier
\*===========================================================================*/

Modifier *$$CLASS_NAME$$Object::CreateWSMMod(INode *node)
{
	return new $$CLASS_NAME$$(node,this);
}

// Default constructor/Destructor

$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	//TODO Add any construction Code here
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}
/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

$$CLASS_NAME$$::$$CLASS_NAME$$(INode *node,$$CLASS_NAME$$Object *obj)
{	
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
	assert(pblock);

	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	
	obRef = NULL;
}


/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void $$CLASS_NAME$$::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	SimpleWSMMod::BeginEditParams(ip,flags,prev);
	$$CLASS_NAME$$Desc.BeginEditParams(ip, this, flags, prev);

}
		
void $$CLASS_NAME$$::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleWSMMod::EndEditParams(ip,flags,next);
	$$CLASS_NAME$$Desc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;	
}



/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void $$CLASS_NAME$$::InvalidateUI()
{
	$$CLASS_NAME_LOWER$$_param_blk.InvalidateUI(pblock->LastNotifyParamID());
}



/*===========================================================================*\
 |	Standard Load Save and clone
\*===========================================================================*/

IOResult $$CLASS_NAME$$::Load(ILoad *iload)
{
	Modifier::Load(iload);
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Save(ISave *isave)
{
	Modifier::Save(isave);
	return IO_OK;
}

RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir& remap) 
{
	$$CLASS_NAME$$ *newob = new $$CLASS_NAME$$(nodeRef,($$CLASS_NAME$$Object*)obRef);
	newob->ReplaceReference(SIMPWSMMOD_PBLOCKREF,pblock->Clone(remap));
	newob->ReplaceReference(SIMPWSMMOD_OBREF,this->obRef);
	newob->ReplaceReference(SIMPWSMMOD_NODEREF,this->nodeRef);
	BaseClone(this, newob, remap);
	return newob;
}



/*===========================================================================*\
 |	The validity of our parameters
 |	Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/

Interval $$CLASS_NAME$$::GetValidity(TimeValue t) 
{
	if (nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;

		$$CLASS_NAME$$Object *obj = ($$CLASS_NAME$$Object*)GetWSMObject(t);
		pblock->GetValue(pb_spin, t, f, valid);
		obj->pblock2->GetValue(pb_spin_obj, t, f, valid);

		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	}
	else 
	{
		return FOREVER;
	}
}




/*===========================================================================*\
 |	Get the actual deformer to modify the object
\*===========================================================================*/

Deformer& $$CLASS_NAME$$::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	Interval valid;
	static $$CLASS_NAME$$Deformer wsmd;

//TODO: Define the Deformer
	return wsmd;
}

/*===========================================================================*\
 |	Deform the actual point
\*===========================================================================*/


Point3 $$CLASS_NAME$$Deformer::Map(int i, Point3 p)
{
	//TODO: Deform the actual Point
	return p;
}

int $$CLASS_NAME$$ObjCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1, center;

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
	}


	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {

		case 0:
			sp0 = m;
			p0 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			break;


		case 1:
			mat.IdentityMatrix();
			
			p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			r = Length(p1-p0);
			mat.SetTrans(p0);

			if (msg==MOUSE_POINT) {
				return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
			}
			break;					   
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static $$CLASS_NAME$$ObjCreateCallBack WSMCreateCB;

CreateMouseCallBack* $$CLASS_NAME$$Object::GetCreateMouseCallBack() 
{
	WSMCreateCB.SetObj(this);
	return(&WSMCreateCB);
}




/*===========================================================================*\
 |	Mesh creation - create the actual object
\*===========================================================================*/

void $$CLASS_NAME$$Object::BuildMesh(TimeValue t)
{
	//TODO: Build the Space Warp Gizmo
}


/*===========================================================================*\
 |	SimpleWSMMod2 overloads
\*===========================================================================*/

RefTargetHandle $$CLASS_NAME$$::GetReference(int i) 
{ 
	switch (i) {
		case SIMPWSMMOD_OBREF: return obRef;
		case SIMPWSMMOD_NODEREF: return nodeRef;
		case SIMPWSMMOD_PBLOCKREF: return pblock;
		default: return NULL;
		}
}

void $$CLASS_NAME$$::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch (i) {
		case SIMPWSMMOD_OBREF: obRef = (WSMObject*)rtarg; break;
		case SIMPWSMMOD_NODEREF: nodeRef = (INode*)rtarg; break;
		case SIMPWSMMOD_PBLOCKREF: pblock = (IParamBlock2*)rtarg; SimpleWSMMod::SetReference(i, rtarg);break;
		}
}

Animatable* $$CLASS_NAME$$::SubAnim(int i) 
{ 
	switch (i) {
		case 0: return pblock;		
		default: return NULL;
		}
}


$$//--------------------------------------
$$//TEXTURE_2D_TYPE or TEXTURE_3D_TYPE mcr_mf_texmap
$$//--------------------------------------
$$ELIF(TEX_TYPE)
$$IF(TEXTURE_3D_TYPE)
ParamDlg* $$CLASS_NAME$$::xyzGenDlg;
$$ELIF(TEXTURE_2D_TYPE)
ParamDlg* $$CLASS_NAME$$::uvGenDlg;
$$ENDIF

//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	for (int i=0; i<NSUBTEX; i++) subtex[i] = NULL;
	//TODO: Add all the initializing stuff
$$IF(PARAM_MAPS)	
	$$PBLOCK$$ = NULL;
	$$CLASS_NAME$$Desc.MakeAutoParamBlocks(this);
$$ENDIF //PARAM_MAPS
$$IF(TEXTURE_3D_TYPE)
	xyzGen = NULL;
$$ELIF(TEXTURE_2D_TYPE)
	uvGen = NULL;
$$ENDIF
	Reset();
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

//From MtlBase
void $$CLASS_NAME$$::Reset() 
{
$$IF(TEXTURE_3D_TYPE)
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference( 0, GetNewDefaultXYZGen());	
$$ELIF(TEXTURE_2D_TYPE)
	if (uvGen) uvGen->Reset();
	else ReplaceReference( 0, GetNewDefaultUVGen());	
$$ENDIF
	//TODO: Reset texmap back to its default values
$$IF(PARAM_MAPS)	
$$// Hey! handle this
$$ENDIF //PARAM_MAPS

	ivalid.SetEmpty();

}

void $$CLASS_NAME$$::Update(TimeValue t, Interval& valid) 
{	
	//TODO: Add code to evaluate anything prior to rendering
}

Interval $$CLASS_NAME$$::Validity(TimeValue t)
{
	//TODO: Update ivalid here
	return ivalid;
}

ParamDlg* $$CLASS_NAME$$::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = $$CLASS_NAME$$Desc.CreateParamDlgs(hwMtlEdit, imp, this);
$$IF(TEXTURE_3D_TYPE)
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(xyzGenDlg);
$$ELIF(TEXTURE_2D_TYPE)
	uvGenDlg = uvGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(uvGenDlg);
$$ENDIF
	//TODO: Set the user dialog proc of the param block, and do other initialization	
	return masterDlg;	
}

BOOL $$CLASS_NAME$$::SetDlgThing(ParamDlg* dlg)
{	
$$IF(TEXTURE_3D_TYPE)	
	if (dlg == xyzGenDlg)
		xyzGenDlg->SetThing(xyzGen);
$$ELIF(TEXTURE_2D_TYPE)
	if (dlg == uvGenDlg)
		uvGenDlg->SetThing(uvGen);
$$ENDIF
	else 
		return FALSE;
	return TRUE;
}

void $$CLASS_NAME$$::SetSubTexmap(int i, Texmap *m) 
{
$$IF(PARAM_MAPS)
	ReplaceReference(i+2,m);
$$ELSE
	ReplaceReference(i+1,m);
$$ENDIF //PARAM_MAPS
	//TODO Store the 'i-th' sub-texmap managed by the texture
}

TSTR $$CLASS_NAME$$::GetSubTexmapSlotName(int i) 
{	
	//TODO: Return the slot name of the 'i-th' sub-texmap
	return TSTR(_T(""));
}


//From ReferenceMaker
RefTargetHandle $$CLASS_NAME$$::GetReference(int i) 
{
	//TODO: Return the references based on the index	
	switch (i) {
$$IF(TEXTURE_3D_TYPE)
		case 0: return xyzGen;
$$ELIF(TEXTURE_2D_TYPE)
		case 0: return uvGen;
$$ENDIF		
$$IF(PARAM_MAPS)		
		case 1: return $$PBLOCK$$;
		default: return subtex[i-2];
$$ELSE
		default: return subtex[i-1];
$$ENDIF //PARAM_MAPS	
		}
}

void $$CLASS_NAME$$::SetReference(int i, RefTargetHandle rtarg) 
{
	//TODO: Store the reference handle passed into its 'i-th' reference
	switch(i) {
$$IF(TEXTURE_3D_TYPE)
		case 0: xyzGen = (XYZGen *)rtarg; break;
$$ELIF(TEXTURE_2D_TYPE)
		case 0: uvGen = (UVGen *)rtarg; break;
$$ENDIF		
$$IF(PARAM_MAPS)		
		case 1:	$$PBLOCK$$ = (IParamBlock2 *)rtarg; break;
		default: subtex[i-2] = (Texmap *)rtarg; break;
$$ELSE
		default: subtex[i-1] = (Texmap *)rtarg; break;
$$ENDIF //PARAM_MAPS
	}
}

//From ReferenceTarget 
RefTargetHandle $$CLASS_NAME$$::Clone(RemapDir &remap) 
{
	$$CLASS_NAME$$ *mnew = new $$CLASS_NAME$$();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	//TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

	 
Animatable* $$CLASS_NAME$$::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i) {
$$IF(TEXTURE_3D_TYPE)
		case 0: return xyzGen;
$$ELIF(TEXTURE_2D_TYPE)
		case 0: return uvGen;
$$ENDIF		
$$IF(PARAM_MAPS)		
		case 1: return $$PBLOCK$$;
		default: return subtex[i-2];
$$ELSE
		default: return subtex[i-1];
$$ENDIF //PARAM_MAPS	
		}
}

TSTR $$CLASS_NAME$$::SubAnimName(int i) 
{
	//TODO: Return the sub-anim names
	switch (i) {
		case 0: return GetString(IDS_COORDS);		
$$IF(PARAM_MAPS)
		case 1: return GetString(IDS_PARAMS);
		default: return GetSubTexmapTVName(i-1);
$$ELSE
		default: return GetSubTexmapTVName(i-1);
$$ENDIF
		}
}

RefResult $$CLASS_NAME$$::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	//TODO: Handle the reference changed messages here	
	return(REF_SUCCEED);
}

IOResult $$CLASS_NAME$$::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult $$CLASS_NAME$$::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

AColor $$CLASS_NAME$$::EvalColor(ShadeContext& sc)
{
	//TODO: Evaluate the color of texture map for the context.
	return AColor (0.0f,0.0f,0.0f,0.0f);
}

float $$CLASS_NAME$$::EvalMono(ShadeContext& sc)
{
	//TODO: Evaluate the map for a "mono" channel
	return Intens(EvalColor(sc));
}

Point3 $$CLASS_NAME$$::EvalNormalPerturb(ShadeContext& sc)
{
	//TODO: Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG $$CLASS_NAME$$::LocalRequirements(int subMtlNum)
{
	//TODO: Specify various requirements for the material
$$IF(TEXTURE_3D_TYPE)
	return xyzGen->Requirements(subMtlNum); 
$$ELIF(TEXTURE_2D_TYPE)
	return uvGen->Requirements(subMtlNum); 
$$ENDIF //TEXTURE_3D_TYPE
}

$$IF(TEXTURE_2D_TYPE)
void $$CLASS_NAME$$::ActivateTexDisplay(BOOL onoff)
{
	//TODO: Implement this only if SupportTexDisplay() returns TRUE
}

DWORD $$CLASS_NAME$$::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	//TODO: Return the texture handle to this texture map
	return NULL;
}
$$ENDIF
$$//--------------------------------------
$$//TRACK_VIEW_UTILITY_TYPE mcr_mf_tvutil
$$//--------------------------------------
$$ELIF(TRACK_VIEW_UTILITY_TYPE)
	
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

void $$CLASS_NAME$$::BeginEditParams(Interface *ip,ITVUtility *iu)
{
	this->iu = iu;
	this->ip = ip;
	hPanel = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		iu->GetTrackViewHWnd(),
		$$CLASS_NAME$$DlgProc,
		(LONG)this);
}

void $$CLASS_NAME$$::EndEditParams(Interface *ip,ITVUtility *iu)
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void $$CLASS_NAME$$::TrackSelectionChanged()
{

}

void $$CLASS_NAME$$::NodeSelectionChanged()
{

}

void $$CLASS_NAME$$::KeySelectionChanged()
{

}

void $$CLASS_NAME$$::TimeSelectionChanged()
{

}

void $$CLASS_NAME$$::MajorModeChanged()
{

}

void $$CLASS_NAME$$::TrackListChanged()
{

}

void $$CLASS_NAME$$::Init(HWND hWnd)
{

}

void $$CLASS_NAME$$::Destroy(HWND hWnd)
{

}

$$//--------------------------------------
$$//UTILITY_TYPE mcr_mf_util
$$//--------------------------------------
$$ELIF(UTILITY_TYPE)

//--- $$CLASS_NAME$$ -------------------------------------------------------
$$CLASS_NAME$$::$$CLASS_NAME$$()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

$$CLASS_NAME$$::~$$CLASS_NAME$$()
{

}

void $$CLASS_NAME$$::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		$$CLASS_NAME$$DlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void $$CLASS_NAME$$::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void $$CLASS_NAME$$::Init(HWND hWnd)
{

}

void $$CLASS_NAME$$::Destroy(HWND hWnd)
{

}

$$ENDIF //FOR ALL PLUGIN TYPES
