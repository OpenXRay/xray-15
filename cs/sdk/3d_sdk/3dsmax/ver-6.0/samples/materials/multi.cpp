/**********************************************************************
 *<
	FILE: multi.cpp

	DESCRIPTION:  Composite material

	CREATED BY: Dan Silva

	HISTORY: UPdated to Param2 1/11/98 Peter Watje

	         Modified to handle sparse arrays without having to have all the
			   intervening nulls. 6/19/00  Dan Silva

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "buildver.h"
#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
// begin - ke/mjm - 03.16.00 - merge reshading code
//#include "iReshade.h"
// end - ke/mjm - 03.16.00 - merge reshading code
#include "macrorec.h"
#include "gport.h"

extern HINSTANCE hInstance;


//###########################################################################
// pblock conversions for maxscript mtl id fixes

class MultiIDDimension : public ParamDimension 
{
public:
	DimType DimensionType() { return DIM_CUSTOM; }
	float Convert(float value) { return value+1; };
	float UnConvert(float value) { return value-1; };
};

static MultiIDDimension theMultiIDDim;



// use hash-table for lookup speed
#define USE_HASHING 1

//###########################################################################
// hash-table template class (taken straight from pview)
// modified:
//   -1 is reserved for "empty key value" instead of 0
//   start with hash table size of 10 instead of 100
//   add "clean" member to clear out keys, but not free the list
#if USE_HASHING

static int exponentialPrimes[] =
	{2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749,
		65521, 131071, 262139, 524287, 1048573, 2097143};

#define N_PRIMES (sizeof(exponentialPrimes) / sizeof(int))

static int findPrimeForSize(int size)
	{
	for (int i = 0; i < N_PRIMES; i++)
		{
		if (exponentialPrimes[i] > size)
			return exponentialPrimes[i];
		}

	return size + 1;
	}

#define UNUSED_KEY -1

template <class K, class V> class MyHashTable
	{
	protected:

	struct Association
		{
		K key;
		V value;
		};

	Association *ht;
	int htSize;
	int nElements;

	int FindPosition(K key)
		{
		int index;
		K indexedObject;

		index = ((DWORD) key) % htSize;
		while (((indexedObject = ht[index].key) != UNUSED_KEY) && (key != indexedObject))
			{
			index++;
			if (index >= htSize)
				index = 0;
			}
		return index;
		}

	void GrowTo(int newSize)
		{
		Association *oldHt = ht;
		int oldSize = htSize;

		ht = new Association[newSize];
		memset(ht, 0, newSize * sizeof(Association));
		for (int x = 0; x < newSize; x++ )						// init to -1's (unused slot)
			ht[x].key = UNUSED_KEY;
		htSize = newSize;

		if (oldHt != NULL)
			{
			for (int i = 0; i < oldSize; i++)
				{
				if (oldHt[i].key != UNUSED_KEY)
					AddAssociation(oldHt[i].key, oldHt[i].value);
				}
			delete[] oldHt;
			}
		}

	public:

	MyHashTable(int size = 10)
		{
		ht = NULL;
		Initialize(size);
		}

	~MyHashTable()
		{
		if (ht != NULL)
			delete[] ht;
		}

	void Clear()
		{
		if (ht != NULL)
			delete[] ht;
		ht = NULL;
		htSize = 0;
		nElements = 0;
		}

	void Clean()
		{
		memset(ht, 0, htSize * sizeof(Association));
		for (int x = 0; x < htSize; x++ )						// init to -1's (unused slot)
			ht[x].key = -1;
		nElements = 0;											// 4/13/01 11:37am --MQM-- oops, need this!
		}

	void Initialize(int size = 100)
		{
		Clear();
		GrowTo(findPrimeForSize(size * 4));
		}

	int AddAssociation(K key, V &value)
		{
		int index;

		if (nElements >= (htSize / 2))
			GrowTo(findPrimeForSize(htSize * 2 + 8));

		index = FindPosition(key);
		if (ht[index].key == UNUSED_KEY)
			{
			ht[index].key = key;
			ht[index].value = value;
			nElements++;
			return -1 - index;
			}

		return index;
		}

	int GetIndex(K key)
		{
		int index = FindPosition(key);
		if (ht[index].key != UNUSED_KEY)
			return index;
		else
			return -1;
		}

	V& operator[](const int i) const
		{
		return ht[i].value; 
		}

	bool GetNextAssociation(int &hashIndex, K* keyP, V* valueP)
		{
		for (int i = hashIndex; i < htSize; i++)
			{
			if (ht[i].key != UNUSED_KEY)
				{
				hashIndex = i + 1;
				*keyP = ht[i].key;
				*valueP = ht[i].value;
				return true;
				}
			}

		return false;
		}

	inline int Count(void)
		{
		return nElements;
		}
 	};

#endif
//###########################################################################

#define NSUBMTLS 10
static Class_ID multiClassID(MULTI_CLASS_ID,0);

class Multi;
class MultiDlg;
class DelSubRestore;


#define PBLOCK_REF	0
#define MTL_REF		1

class MultiDlg: public ParamDlg {
	public:		
		HWND hwmedit;	 // window handle of the materials editor dialog
		HFONT hFont;
		IMtlParams *ip;
		Multi *theMtl;	 // current mtl being edited.
		HWND hPanelBasic; // Rollup pane		
		HWND hScroll;
		TimeValue curTime; 
		int isActive;
		BOOL valid;
		ICustButton *iBut[NSUBMTLS];
		ICustEdit *iName[NSUBMTLS];
		ICustEdit *iIndex[NSUBMTLS];
		BOOL isDup[NSUBMTLS];
		MtlDADMgr dadMgr;
				
		MultiDlg(HWND hwMtlEdit, IMtlParams *imp, Multi *m); 
		~MultiDlg();
		BOOL BasicPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
		void VScroll(int code, short int cpos );
		void DrawPStampBlackBorder(HDC hdc, Rect &rect);
		void DrawPStampHilite( int i, BOOL on);
		void DrawPStampHilite(HDC hdc, BOOL on, Rect &rect );
		void DrawPStamp(HDC hdc, Rect &rect, int i );
		void RemovePStampHilite();
		void DrawDupIndicators();
		void SetIsDup(int i);
		void SelectMtl(int i);
		void UpdateSubMtlNames();
		void UpdateColorSwatches();
		void LoadDialog(BOOL draw);  // stuff params into dialog
		void Invalidate() {
			valid = FALSE;
			Rect rect;
			rect.left = rect.top = 0;
			rect.right = rect.bottom = 10;
			InvalidateRect(hPanelBasic,&rect,FALSE);
			}
		void ReloadDialog();
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void ActivateDlg(BOOL onOff) {}
		void SetNumMats(HWND hWnd);
		void DragAndDrop(int ifrom, int ito);
		int SubMtlNumFromNameID(int id);

		// methods inherited from ParamDlg:
		Class_ID ClassID() {return multiClassID;  }
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() { return (ReferenceTarget *)theMtl; }
		void DeleteThis() { delete this;  }	
		void SetTime(TimeValue t);
		int FindSubMtlFromHWND(HWND hw);
	};


// Parameter block indices
#define PB_THRESH		0
#define PB_WIDTH		1

//-----------------------------------------------------------------------------
//  Multi
//-----------------------------------------------------------------------------

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs

enum { multi_params, };  		// pblock ID

enum							// multi_params param IDs
{ 
	multi_mtls,
	multi_ons,
	multi_names,
	multi_ids,
};


class Multi : public MultiMtl, public IReshading
{
	friend class MultiDlg;
	friend class SetNumMtlsRestore;
	friend class DelSubRestore;
	friend class AddMtlsRestore;
	friend class SetSubRestore;

	// Animatable parameters
	Interval 	ivalid;
	ReshadeRequirements mReshadeRQ; // mjm - 06.02.00
	MultiDlg 	*paramDlg;
	int 		offset;
	int 		selected;

public:

	NameTab 	subNames;
	Tab<Mtl *>	subMtl;

#if USE_HASHING
	// allow fast conversions for sub-material id --> actual submaterial slot #
	// (use int for the key, instead of short int, since -1 is reserved in the table)
	MyHashTable< int, int >  hashTab;
	int			hashTabDirty;
#endif

//	Tab<BOOL>	mapOn;

	BOOL 		Param1;
	BOOL 		ignoreNotify;
	IParamBlock2 *pblock;   // ref #0		
	int 		maxMtlId;	// current max material id used in the list...needs to be updated when anything changes

	BOOL 		loadingOld;
	void 		AddMtl(ReferenceTarget *rt = NULL, int id=0, TCHAR *name=0);
	void 		DeleteSelMtl();
	void 		SortMtls( CompareFnc cmp );
	void 		SortMtlsByName();
	void 		SortMtlsByID();
	void 		SortMtlsBySlotName();
	BOOL 		AnyDupIDs();
//	BOOL 		IsIDDup( int k );
	void 		SetNumSubMtls( int n );
//	void 		SetNSubMtls( int num );
	void 		GetSubMtlName( int mtlid, TSTR &s );
	void 		SetSubMtlAndName( int mtlid, Mtl *m, TSTR &subMtlName );
	void        RemoveMtl (int mtlid);
	void 		ClampOffset();
	Sampler*	GetPixelSampler(int mtlNum, BOOL backFace );

	void 		SetAmbient(   Color c, TimeValue t )  {}		
	void 		SetDiffuse(   Color c, TimeValue t )  {}		
	void 		SetSpecular(  Color c, TimeValue t )  {}
	void 		SetShininess( float v, TimeValue t )  {}		
	void 		SetThresh(    float v, TimeValue t);
	void 		SetWidth(     float v, TimeValue t);

	Mtl *		UseMtl();
	Color 		GetAmbient(   int mtlNum = 0, BOOL backFace = FALSE );
    Color 		GetDiffuse(   int mtlNum = 0, BOOL backFace = FALSE );
	Color 		GetSpecular(  int mtlNum = 0, BOOL backFace = FALSE );
	float 		GetXParency(  int mtlNum = 0, BOOL backFace = FALSE );
	float 		GetShininess( int mtlNum = 0, BOOL backFace = FALSE );		
	float 		GetShinStr(   int mtlNum = 0, BOOL backFace = FALSE );
	float 		WireSize(     int mtlNum = 0, BOOL backFace = FALSE );

	            Multi( BOOL loading, BOOL createDefaultSubMtls = TRUE ); // mjm - 10.11.99 - added createDefaultSubMtls parameter
	void 		SetParamDlg( MultiDlg *pd ) 	{ paramDlg = pd; }
	ParamDlg * 	CreateParamDlg( HWND hwMtlEdit, IMtlParams *imp );
	void 		Shade( ShadeContext &sc );
	float 		EvalDisplacement( ShadeContext &sc ); 
	Interval 	DisplacementValidity( TimeValue t ); 
	void 		Update( TimeValue t, Interval &valid );
	void 		Init();
	void 		Reset();
	Interval 	Validity( TimeValue t );
	void 		NotifyChanged();

	Class_ID 	ClassID() 					{ return multiClassID; }
	SClass_ID 	SuperClassID() 				{ return MATERIAL_CLASS_ID; }
#ifndef GAME_VER // orb 08-21-2001 Fixing Defect 302554
	void 		GetClassName( TSTR &s ) 	{ s = GetString(IDS_RB_MULTISUBOBJECT); }  
#else
	void 		GetClassName( TSTR &s ) 	{ s = GetString(IDS_RB_MULTISUBOBJECT_GMAX); }  
#endif // GAME_VER

	void 		DeleteThis() 				{ delete this; }	


	// Methods to access sub-materials of meta-materials

#if USE_HASHING
	void		UpdateHashTable()
		        {
					int 	count;								// # of sub-materials to search
					int 	id;									// id of the sub material
					Interval iv;								// and interval
					int		i;									// loop var

					// wipe values in the table
					hashTab.Clean();

					// search through all our slots for the MAXIMUM id #
					maxMtlId = -1;						// start with max set to -1
					count = subMtl.Count();
					for ( i = 0;  i < count;  i++ ) 
					{
						// grab the id value from the param block (slow)
						pblock->GetValue( multi_ids, 0, id, iv, i ); 
						if ( id > maxMtlId ) 
							maxMtlId = id;

						// add to hash map
						hashTab.AddAssociation( id, i );
					}

					// hash table not dirty anymore
					hashTabDirty = 0;
				}
#endif

   	int 		MaxSubMtlID() 
				{ 
#if USE_HASHING
					if ( hashTabDirty )
						UpdateHashTable();

					return maxMtlId ;
#else
					int 	mx;									// max id # found
					int 	count;								// # of sub-materials to search
					int 	id;									// id of the sub material
					Interval iv;								// and interval
					int		i;									// loop var

					// search through all our slots for the MAXIMUM id #
					mx = -1;						// start with max set to -1
					count = subMtl.Count();
					for ( i = 0;  i < count;  i++ ) 
					{
						// grab the id value from the param block (slow)
						pblock->GetValue( multi_ids, 0, id, iv, i ); 
						if ( id > mx ) 
							mx = id;
					}

					// return maximum id# found
					return mx; 
#endif
				}

	int 		NumSubMtls() 
				{
#if USE_HASHING
					if ( hashTabDirty )
						UpdateHashTable();

					return maxMtlId + 1;   // why does this return the max id and not count?????
#else
					return MaxSubMtlID() + 1;   // was - return subMtl.Count(); 
#endif
				}

	int  		FindSubMtl( int i ) 
				{
#if USE_HASHING
					// rebuild hash table if dirty
					if ( hashTabDirty )
						UpdateHashTable();

					// use hash table to find the index find the value associated with the material id key
					int idx = hashTab.GetIndex( i );
					if ( idx == -1 )
						return -1;
					else
						return hashTab[idx];					// value
#else
					int 	j;									// loop var
					int 	id;									// id of sub material in loop
					Interval iv;								// and interval

					// find a specific sub-material 'i'
					// this should be in a table instead!
					for ( j = 0;  j < subMtl.Count();  j++ ) 
					{
						pblock->GetValue( multi_ids, 0, id, iv, j ); 
						if ( id == i )
							return j;
					}
					return -1;
#endif
				}

	int 		FindSubMtlMod( int mtlid ) 
				{
#if USE_HASHING
					// rebuild hash table if dirty
					if ( hashTabDirty )
						UpdateHashTable();

					// bail out if no materials
					if ( maxMtlId == -1 )
						return -1;

					// take care of wraparound.
					// note: we need the UpdateHashTable() above to recalculate maxMtlId
					if ( mtlid > maxMtlId ) 		
						mtlid = mtlid % ( maxMtlId + 1 );

					// use hash table to find the index find the value associated with the material id key
					int idx = hashTab.GetIndex( mtlid );
					if ( idx == -1 )
						return -1;
					else
						return hashTab[idx];					// value
#else
					if ( maxMtlId == -1 )
						return -1;
					if ( mtlid > maxMtlId ) 		
						mtlid = mtlid % ( maxMtlId + 1 );
					return FindSubMtl( mtlid );  // IS THIS GOING TO SLOW THINGS DOWN?
#endif
				}

	Mtl * 		GetSubMtl( int i ) 
				{ 
#if USE_HASHING
					// rebuild hash table if dirty
					if ( hashTabDirty )
						UpdateHashTable();

					// get the submaterial ptr
					int idx = hashTab.GetIndex( i );
					return ( idx == -1 ) ? NULL : subMtl[ hashTab[idx] ]; // 4/13/01 4:44pm --MQM-- from subMtl[idx] to subMtl[ hashTab[idx] ]
#else
					int j = FindSubMtl( i );
					return ( j >= 0 ) ? subMtl[j] : NULL;
#endif
				}


	void 		SetSubMtl( int i, Mtl *m );
	TSTR 		GetSubMtlSlotName( int i );
	BOOL 		IsMultiMtl() 		{ return TRUE; }

	int 		NumSubs() 			{ return subMtl.Count(); }  
	Animatable *  SubAnim( int i );
	TSTR 		SubAnimName( int i );
	int 		SubNumToRefNum( int subNum )  { return subNum+1; }

	// From ref
	int 		NumRefs()  { return loadingOld ? subMtl.Count()+1 : subMtl.Count()+1;  }
	RefTargetHandle  GetReference( int i );
	void 		SetReference( int i, RefTargetHandle rtarg );
	int 		RemapRefOnLoad( int iref ) ;

	RefTargetHandle  Clone( RemapDir &remap = NoRemap() );
	RefResult 	NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message );

	// IO
	IOResult 	Save( ISave *isave );
	IOResult 	Load( ILoad *iload );

	// JBW: direct ParamBlock access is added
	int			NumParamBlocks()  					{ return 1; }					// return number of ParamBlocks in this instance
	IParamBlock2 *  GetParamBlock( int i ) 			{ return pblock; } 				// return i'th ParamBlock
	IParamBlock2 *  GetParamBlockByID( BlockID id ) { return ( pblock->ID() == id ) ? pblock : NULL; } // return id'd ParamBlock

	// begin - ke/mjm - 03.16.00 - merge reshading code
	BOOL 		SupportsRenderElements()			{ return TRUE; }
//		BOOL SupportsReShading(ShadeContext& sc);
	ReshadeRequirements  GetReshadeRequirements() 	{ return mReshadeRQ; }			// mjm - 06.02.00
	void 		PreShade(  ShadeContext &sc, IReshadeFragment *pFrag);
	void 		PostShade( ShadeContext &sc, IReshadeFragment *pFrag, int &nextTexIndex, IllumParams *ip );
	// end - ke/mjm - 03.16.00 - merge reshading code

	// From Mtl
	bool 		IsOutputConst(       ShadeContext &sc, int stdID );
	bool 		EvalColorStdChannel( ShadeContext &sc, int stdID, Color &outClr );
	bool 		EvalMonoStdChannel(  ShadeContext &sc, int stdID, float &outVal );

	void * 		GetInterface( ULONG id );
};


int numMultis = 0;
class MultiClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Multi(loading); }
#ifndef GAME_VER // orb 08-21-2001 Fixing Defect 302554
	const TCHAR *	ClassName() { return GetString(IDS_RB_MULTISUBOBJECT_CDESC); } // mjm - 2.3.99
#else
	const TCHAR *	ClassName() { return GetString(IDS_RB_MULTISUBOBJECT_CDESC_GMAX); } // mjm - 2.3.99
#endif //GAME_VER
	SClass_ID		SuperClassID() { return MATERIAL_CLASS_ID; }
	Class_ID 		ClassID() { return multiClassID; }
	const TCHAR* 	Category() { return _T("");  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("multiSubMaterial"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static MultiClassDesc multiCD;

ClassDesc* GetMultiDesc() { 
	return &multiCD;  
	}


// per instance param block
static ParamBlockDesc2 multi_param_blk ( multi_params, _T("parameters"),  0, &multiCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_MULTI, IDS_DS_MULTI_PARAMS, 0, 0, NULL, 
	// params
	multi_mtls,	_T("materialList"),	TYPE_MTL_TAB,	10,		P_OWNERS_REF + P_VARIABLE_SIZE,	IDS_RB_MATERIAL2,	
		p_refno,		MTL_REF, 
		end,
	multi_ons,	_T("mapEnabled"), TYPE_BOOL_TAB,	10,		P_VARIABLE_SIZE,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		end,
	multi_names, _T("names"), TYPE_STRING_TAB,		10,		P_VARIABLE_SIZE,				IDS_DS_MAP,
		end,
	multi_ids,	_T("materialIDList"),	TYPE_INT_TAB,	10,	  P_VARIABLE_SIZE,				IDS_DS_INDEX,	
		p_dim, &theMultiIDDim,									// 4/24/01 3:33pm --MQM-- add ParameterDimension function to fix maxscript #'s 
		end,
	end
);


static INT_PTR CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	MultiDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (MultiDlg*)lParam;
		theDlg->hPanelBasic = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (MultiDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->BasicPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}


int MultiDlg::FindSubMtlFromHWND(HWND hw) {
	for (int i=0; i<NSUBMTLS; i++) {
		if (hw == iBut[i]->GetHwnd()) {
			int j = i+theMtl->offset;
			int id;
			Interval iv;
			theMtl->pblock->GetValue(multi_ids,0,id,iv,j); 
			return id;
			}
		}	
	return -1;
	}

void MultiDlg::DragAndDrop(int ifrom, int ito) {
	theMtl->CopySubMtl(hPanelBasic,ifrom+theMtl->offset, ito+theMtl->offset);
	theMtl->NotifyChanged();
	UpdateMtlDisplay();
	}

//-------------------------------------------------------------------

MultiDlg::MultiDlg(HWND hwMtlEdit, IMtlParams *imp, Multi *m) { 
	dadMgr.Init(this);
	hwmedit = hwMtlEdit;
	ip = imp;
	hPanelBasic = NULL;
	theMtl = m; 
	isActive = 0;
	valid = FALSE;
	theMtl->ClampOffset();
	for (int i=0; i<NSUBMTLS; i++) {	
		iBut[i] = NULL;
		iName[i] = NULL;
		iIndex[i] = NULL;
		isDup[i] = FALSE;
		}
	hFont = CreateFont(14,0,0,0,FW_BOLD,0,0,0,0,0,0,0, VARIABLE_PITCH | FF_SWISS, _T(""));
	hPanelBasic = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_MULTI),
		PanelDlgProc, 
		GetString(IDS_DS_MULTI_PARAMS), 
		(LPARAM)this );		
	curTime = imp->GetTime();
	}

void MultiDlg::ReloadDialog() {
	Interval valid;
	theMtl->Update(curTime,valid);
	LoadDialog(FALSE);
	}

void MultiDlg::SetTime(TimeValue t) {
	if (t!=curTime) {
		Interval valid;
		curTime = t;
		theMtl->Update(curTime,valid);
		// Since nothing is time varying, can skip this
		//InvalidateRect(hPanelBasic,NULL,0);
		}
	}

MultiDlg::~MultiDlg() {
	theMtl->SetParamDlg(NULL);	
	SetWindowLongPtr(hPanelBasic, GWLP_USERDATA, NULL);
	hPanelBasic =  NULL;
	for (int i=0; i<NSUBMTLS; i++) {
		ReleaseICustButton(iBut[i]);
		ReleaseICustEdit(iName[i]);
		ReleaseICustEdit(iIndex[i]);
		iBut[i] = NULL; 
		iName[i] = NULL;
		iIndex[i] = NULL;
		}
	DeleteObject(hFont);
	}


static int subMtlId[NSUBMTLS] = {
	IDC_MULTI_MTL0,
	IDC_MULTI_MTL1,
	IDC_MULTI_MTL2,
	IDC_MULTI_MTL3,
	IDC_MULTI_MTL4,
	IDC_MULTI_MTL5,
	IDC_MULTI_MTL6,
	IDC_MULTI_MTL7,
	IDC_MULTI_MTL8,
	IDC_MULTI_MTL9
	};


static int subNameId[NSUBMTLS] = {
	IDC_MTL_NAME0,
	IDC_MTL_NAME1,
	IDC_MTL_NAME2,
	IDC_MTL_NAME3,
	IDC_MTL_NAME4,
	IDC_MTL_NAME5,
	IDC_MTL_NAME6,
	IDC_MTL_NAME7,
	IDC_MTL_NAME8,
	IDC_MTL_NAME9
	};

static int subIndex[NSUBMTLS] = {
	IDC_MTL_ID0,
	IDC_MTL_ID1,
	IDC_MTL_ID2,
	IDC_MTL_ID3,
	IDC_MTL_ID4,
	IDC_MTL_ID5,
	IDC_MTL_ID6,
	IDC_MTL_ID7,
	IDC_MTL_ID8,
	IDC_MTL_ID9
	};


static int mapOnIDs[] = {
	IDC_MAPON1,
	IDC_MAPON2,
	IDC_MAPON3,
	IDC_MAPON4,
	IDC_MAPON5,
	IDC_MAPON6,
	IDC_MAPON7,
	IDC_MAPON8,
	IDC_MAPON9,
	IDC_MAPON10,
	};
/*
static int numIDs[] = {
	IDC_MULT_NUM1,
	IDC_MULT_NUM2,
	IDC_MULT_NUM3,
	IDC_MULT_NUM4,
	IDC_MULT_NUM5,
	IDC_MULT_NUM6,
	IDC_MULT_NUM7,
	IDC_MULT_NUM8,
	IDC_MULT_NUM9,
	IDC_MULT_NUM10,
	};	
*/

static int mtlPStampIDs[] = {
	IDC_PSTAMP1,
	IDC_PSTAMP2,
	IDC_PSTAMP3,
	IDC_PSTAMP4,
	IDC_PSTAMP5,
	IDC_PSTAMP6,
	IDC_PSTAMP7,
	IDC_PSTAMP8,
	IDC_PSTAMP9,
	IDC_PSTAMP10,
	};

static int indexIDs[] = {
	IDC_MTL_ID0,
	IDC_MTL_ID1,
	IDC_MTL_ID2,
	IDC_MTL_ID3,
	IDC_MTL_ID4,
	IDC_MTL_ID5,
	IDC_MTL_ID6,
	IDC_MTL_ID7,
	IDC_MTL_ID8,
	IDC_MTL_ID9
	};

static int SubMtlNumFromPStampID(int id) {
	for (int i=0; i<NSUBMTLS; i++) {
		if (mtlPStampIDs[i]==id) return i;
		}
	return 0;
	}


int MultiDlg::SubMtlNumFromNameID(int id) {
	for (int i=0; i<NSUBMTLS; i++) {
		if (subNameId[i]==id) return i;
		}
	return 0;
	}


static int SubMtlNumIndexID(int id) {
	for (int i=0; i<NSUBMTLS; i++) {
		if (indexIDs[i]==id) return i;
		}
	return 0;
	}

void MultiDlg::RemovePStampHilite() {
	for (int i=0; i<NSUBMTLS; i++) {
		DrawPStampHilite( i, FALSE);
		}
//	if (theMtl->selected>=0) 
//		DrawPStampHilite( theMtl->selected-theMtl->offset, FALSE);
	}

void MultiDlg::VScroll(int code, short int cpos ) {
	for (int i=0; i<NSUBMTLS; i++) {
		if (iIndex[i]->HasFocus()||iName[i]->HasFocus()) 
			SetFocus(NULL);
		}
	RemovePStampHilite();
	switch (code) {
		case SB_LINEUP: 	theMtl->offset--;		break;
		case SB_LINEDOWN:	theMtl->offset++;		break;
		case SB_PAGEUP:		theMtl->offset -= NSUBMTLS;	break;
		case SB_PAGEDOWN:	theMtl->offset += NSUBMTLS;	break;
		
		case SB_THUMBPOSITION: 
		case SB_THUMBTRACK:
			theMtl->offset = cpos;
			break;
		}

	theMtl->ClampOffset();
	UpdateSubMtlNames();						
	}

#define DORECT(x) Rectangle( hdc, rect.left-(x), rect.top-(x), rect.right+(x), rect.bottom+(x) )

void MultiDlg::DrawPStampBlackBorder(HDC hdc, Rect &rect) {
	SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	DORECT(0);
	}

void MultiDlg::DrawPStampHilite(HDC hdc, BOOL on, Rect &rect) {
	SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
	HPEN hGray = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNFACE ) );
	SelectObject(hdc, hGray);
	if (on) SelectObject(hdc, GetStockObject(WHITE_PEN));
	DORECT(1);
	if (on) SelectObject(hdc, GetStockObject(BLACK_PEN));
	DORECT(2);
	DeleteObject( hGray);
	}

void MultiDlg::DrawPStampHilite( int i, BOOL on) {
	HWND hwStamp = GetDlgItem(hPanelBasic, mtlPStampIDs[i]);
	HDC hdc = GetDC(hwStamp);
	Rect rect;
	GetClientRect(hwStamp,&rect);
	DrawPStampHilite(hdc, on, rect);
	}

void MultiDlg::DrawDupIndicators() {
	Rect rp;
	GetWindowRect(hPanelBasic, &rp);
	HDC hdc = GetDC(hPanelBasic);
	HPEN hGray = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNFACE));
	HPEN hRed = CreatePen( PS_SOLID, 0, RGB(255,0,0));
	SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
	for (int i=0; i<NSUBMTLS; i++) {
		Rect rect;
		HWND hw = iIndex[i]->GetHwnd();
		GetWindowRect( hw, &rect );
		rect.left -= rp.left;		rect.right -= rp.left;
		rect.top -= rp.top;	 		rect.bottom -= rp.top;
		SelectObject(hdc, isDup[i]? hRed: hGray); 
		DORECT(1);
		DORECT(2);
		}
	DeleteObject( hGray);
	DeleteObject( hRed);
	ReleaseDC(hPanelBasic,hdc);
	}

void MultiDlg::SetIsDup(int i) {
	if (i>=0&&i<NSUBMTLS)
		isDup[i] = TRUE;
	}

void MultiDlg::DrawPStamp(HDC hdc, Rect &rect, int i ) {
	i += theMtl->offset; 
	if (i>=theMtl->subMtl.Count()) 
		return;
	Mtl *m = theMtl->subMtl[i];
	if (m==NULL) {
	    HBRUSH hOldbrush = (HBRUSH)SelectObject(hdc,GetCustSysColorBrush(COLOR_BTNFACE));
		DORECT(0);
	    SelectObject(hdc,hOldbrush);
		}
	else {
		PStamp *ps = m->GetPStamp(PS_TINY);
		if (!ps) 
			ps = m->CreatePStamp(PS_TINY,TRUE);
		int w = ps->Width();
		int h = ps->Height();
		int scanw = ByteWidth(w);
		int nb = scanw*h;
		UBYTE *workImg = new UBYTE[nb];
		ps->GetImage(workImg);

		GetGPort()->DisplayMap(hdc, rect, 0, 0, workImg, scanw); 
		delete workImg;

/*
		int h = rect.bottom-rect.top; 
		int w = rect.right-rect.left;
		if (w>h) w = h;
		if (h>w) h = w;
		int scanw = ByteWidth(w);
		int nb = scanw*h;
		UBYTE *workImg = new UBYTE[nb];
		
		// Need to put this into Interface with separate w,h
		GetCOREInterface()->Execute(I_EXEC_RENDER_MTL_SAMPLE,(ULONG_PTR)m, (ULONG_PTR)w, (ULONG_PTR)workImg);
		GetGPort()->DisplayMap(hdc, rect, 0, 0, workImg, scanw); 
		delete workImg;
*/
		}
	DrawPStampBlackBorder(hdc, rect);
	if (i==theMtl->selected) {
		DrawPStampHilite(hdc, TRUE, rect);
		}
	}													   

void MultiDlg::SelectMtl(int i) {
	if (theMtl->selected>=0)
		DrawPStampHilite(theMtl->selected-theMtl->offset,FALSE);
	theMtl->selected = i+theMtl->offset;
	if (i>=0)
		DrawPStampHilite(i, TRUE);
	}

LRESULT CALLBACK PStampWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = SubMtlNumFromPStampID(GetWindowLongPtr(hwnd,GWL_ID));
	HWND hwParent = GetParent(hwnd);
	MultiDlg *theDlg = (MultiDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_CREATE:
		case WM_DESTROY: 
		break;

		case WM_LBUTTONUP: 
			theDlg->SelectMtl( id );
			break;

		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				theDlg->DrawPStamp( hdc, rect, id )	;
				}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 


LRESULT CALLBACK DupMsgWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
    switch (msg) {
		case WM_PAINT: 	
			{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				SetTextColor(hdc, RGB(255,0,0));
				SetBkMode(hdc, TRANSPARENT);
				TCHAR buf[256];
				GetWindowText(hwnd,buf,sizeof(buf));					
				TextOut(hdc, 0, 0, buf, _tcslen(buf));
				}
			EndPaint( hwnd, &ps );
			}													
		break;
		default:
			break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 

static BOOL ignoreKillFocus = FALSE;

BOOL MultiDlg::BasicPanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:	 {		
			hScroll	= GetDlgItem(hwndDlg,IDC_MULTI_SCROLL);
			SetScrollRange(hScroll,SB_CTL,0,theMtl->subMtl.Count()-NSUBMTLS,FALSE);
			SetScrollPos(hScroll,SB_CTL,theMtl->offset,TRUE);
			EnableWindow(hScroll,theMtl->subMtl.Count()>NSUBMTLS);
			SendDlgItemMessage(hwndDlg,IDC_DUP_IDS,WM_SETFONT,(WPARAM)hFont,TRUE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_DUP_IDS),theMtl->AnyDupIDs()?SW_SHOW:SW_HIDE);
			for (int i=0; i<NSUBMTLS; i++) {
				iBut[i] = GetICustButton(GetDlgItem(hwndDlg,subMtlId[i]));
				iBut[i]->SetDADMgr(&dadMgr);
				iName[i] = GetICustEdit( GetDlgItem(hwndDlg,subNameId[i]));
				iName[i]->SetLeading(2); //??
				iIndex[i] = GetICustEdit(GetDlgItem(hwndDlg,subIndex[i]));
				iIndex[i]->SetLeading(2); //??
				iIndex[i]->WantReturn(TRUE);
				if (i+theMtl->offset<theMtl->subMtl.Count()) {
					TCHAR *name;
					int id;
					Interval iv;
					theMtl->pblock->GetValue(multi_names,0,name,iv,i+theMtl->offset);
					iName[i]->SetText(name);
					theMtl->pblock->GetValue(multi_ids,0,id,iv,i+theMtl->offset); //??????
					iIndex[i]->SetText(id+1); //?????

//					iName[i]->SetText(theMtl->subNames[i+theMtl->offset]);
					}
//				TSTR buf;
//				buf.printf("%d:",i+theMtl->offset+1);
//				SetDlgItemText(hwndDlg,numIDs[i],buf);
				int onCount = theMtl->pblock->Count(multi_ons);
				if (i-theMtl->offset<onCount)
					{
					int on;
					Interval iv;
					theMtl->pblock->GetValue(multi_ons,0,on,iv,i+theMtl->offset);
					SetCheckBox(hwndDlg, mapOnIDs[i], on);
					}
//				if (i-theMtl->offset<theMtl->mapOn.Count())
//					SetCheckBox(hwndDlg, mapOnIDs[i], theMtl->mapOn[i+theMtl->offset]);

				HWND hwnd = GetDlgItem(hwndDlg, mtlPStampIDs[i]);
				SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)PStampWndProc);
				SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)this);

				hwnd = GetDlgItem(hwndDlg, IDC_DUP_IDS);
				SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)DupMsgWndProc);

				}
			DrawDupIndicators();
			}
			return TRUE;
		case WM_VSCROLL:
			VScroll(LOWORD(wParam),(short int)HIWORD(wParam));
			break;
			
		case WM_COMMAND:  
		    switch (id) {
				case IDC_MULTI_MTL0: 
				case IDC_MULTI_MTL1: 
				case IDC_MULTI_MTL2: 
				case IDC_MULTI_MTL3: 
				case IDC_MULTI_MTL4: 
				case IDC_MULTI_MTL5: 
				case IDC_MULTI_MTL6: 
				case IDC_MULTI_MTL7: 
				case IDC_MULTI_MTL8: 
				case IDC_MULTI_MTL9: {
					int i = id-IDC_MULTI_MTL0 + theMtl->offset;
					if (i < theMtl->subMtl.Count()) {
						int mtlid;
						Interval iv;
						theMtl->pblock->GetValue(multi_ids,0,mtlid,iv,i); 
						PostMessage(hwmedit,WM_SUB_MTL_BUTTON, mtlid ,(LPARAM)theMtl);
						}
					}
					break;
				
				case IDC_MAPON1:							
				case IDC_MAPON2:							
				case IDC_MAPON3:							
				case IDC_MAPON4:							
				case IDC_MAPON5:							
				case IDC_MAPON6:
				case IDC_MAPON7:
				case IDC_MAPON8:
				case IDC_MAPON9:
				case IDC_MAPON10:
//					theMtl->mapOn[id-IDC_MAPON1+theMtl->offset] = GetCheckBox(hwndDlg, id);
					{
					int on = GetCheckBox(hwndDlg, id);
					Interval iv;
					theMtl->pblock->SetValue(multi_ons,0,on,id-IDC_MAPON1+theMtl->offset);

					theMtl->NotifyChanged();
					break;
					}

				case IDC_MULTI_SETNUM:
					SetNumMats(hwndDlg);
					break;

				case IDC_MULTI_ADD:
					if ( theMtl->MaxSubMtlID() >= 1000 )		// 4/24/01 1:50pm --MQM-- don't allow new materials when id's are above 1000
						break;
					RemovePStampHilite();
					theHold.Begin();
					theMtl->AddMtl();
					theHold.Accept(GetString(IDS_DS_ADD_SUBMTL));
					UpdateSubMtlNames();		
					UpdateMtlDisplay();	  
					break;

				case IDC_MULTI_DELETE:
					if ( theMtl->subMtl.Count() == 1 )			// 4/12/01 11:10am --MQM-- don't let the user delete the last material
						break;
					RemovePStampHilite();
					theHold.Begin();
					theMtl->DeleteSelMtl();
					theHold.Accept(GetString(IDS_DS_DELSUBMAT));
					theMtl->UpdateHashTable();					// 4/10/01 11:57am --MQM-- force update before redisplay
					UpdateSubMtlNames();		
					UpdateMtlDisplay();	  
					break;

				case IDC_MULTI_SORT:
					RemovePStampHilite();
					theMtl->SortMtlsByID();
					UpdateSubMtlNames();
					UpdateMtlDisplay();	  
					break;

				case IDC_MULTI_ALPHASORT:
					RemovePStampHilite();
					theMtl->SortMtlsByName();
					UpdateSubMtlNames();
					UpdateMtlDisplay();	  
					break;

				case IDC_MULTI_NAMESORT:
					RemovePStampHilite();
					theMtl->SortMtlsBySlotName();
					UpdateSubMtlNames();
					UpdateMtlDisplay();	  
					break;


				case IDC_MTL_NAME0: 
				case IDC_MTL_NAME1: 
				case IDC_MTL_NAME2: 
				case IDC_MTL_NAME3: 
				case IDC_MTL_NAME4: 
				case IDC_MTL_NAME5: 
				case IDC_MTL_NAME6: 
				case IDC_MTL_NAME7: 
				case IDC_MTL_NAME8: 
				case IDC_MTL_NAME9: 
					if (HIWORD(wParam)==EN_CHANGE) {
						TCHAR buf[200];
						int n = SubMtlNumFromNameID(id);
						iName[n]->GetText(buf,199);
//						theMtl->subNames.SetName(n+theMtl->offset, buf);
						theMtl->pblock->SetValue(multi_names,0,buf,n+theMtl->offset);
#if USE_HASHING
//						theMtl->UpdateHashTable();
						theMtl->hashTabDirty = 1;
#endif
						}
					break;

				case IDC_MTL_ID0: 
				case IDC_MTL_ID1: 
				case IDC_MTL_ID2: 
				case IDC_MTL_ID3: 
				case IDC_MTL_ID4: 
				case IDC_MTL_ID5: 
				case IDC_MTL_ID6: 
				case IDC_MTL_ID7: 
				case IDC_MTL_ID8: 
				case IDC_MTL_ID9: 

					// if user exits from this material ID box, or the control was
					// changed by someone else, take care of updating the value in the pblock
					if ( ( HIWORD(wParam)==EN_KILLFOCUS && !ignoreKillFocus ) ||
						 HIWORD(wParam)==EN_CHANGE ) 
					{
						int n = SubMtlNumIndexID( id );
						if ( HIWORD(wParam)==EN_KILLFOCUS || iIndex[n]->GotReturn() ) 
						{
							// get the old ID from the pblock for comparison purposes
							Interval iv;
							int oldMtlId;
							theMtl->pblock->GetValue( multi_ids, 0, oldMtlId, iv, n+theMtl->offset );

							// and grab the new ID from the edit control
							int newMtlId = iIndex[n]->GetInt()-1;

							// ignore notify messages
							theMtl->ignoreNotify = TRUE;

							// if the new ID is different than the old one, set it in the pblock
							if ( newMtlId != oldMtlId )
							{
								// disable the macro recorder, since it will spit out the WRONG id#
								macroRec->Disable();

#if USE_HASHING
								// id has changed, we need to recompute hash table!!!
								theMtl->hashTabDirty = 1; 
#endif
								// set the new ID value in the pblock
								theMtl->pblock->SetValue( multi_ids, 0, newMtlId, n+theMtl->offset );

								// now turn the macro recorder back on, and spit out the "correct" material id
								// it was getting it directly from the pblock, so we need to add +1 to it!
								macroRec->Enable();
								macroRecorder->Assign(mr_index, mr_prop, _T("materialIDList"), mr_reftarg, theMtl, mr_int, n+theMtl->offset+1 , mr_int, newMtlId+1 ); 
							}

							// turn back on notify
							theMtl->ignoreNotify = FALSE;

							// check for duplicate id's
							ShowWindow( GetDlgItem(hwndDlg,IDC_DUP_IDS), theMtl->AnyDupIDs() ? SW_SHOW : SW_HIDE );
							DrawDupIndicators();
								
/*
							if (theMtl->IsIDDup(n)) {
								TCHAR msg[128];
								wsprintf(msg,GetString(IDS_DUP_WARNING), newMtlId+1);
								ignoreKillFocus = TRUE;
								MessageBox(NULL, msg, GetString(IDS_RB_MULTISUBOBJECT), MB_TASKMODAL);
								ignoreKillFocus = FALSE;
								}
*/
						}
					}
					break;

				}
			break;
		case WM_PAINT:
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwndDlg, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				DrawDupIndicators();
				}
			EndPaint( hwndDlg, &ps );
			}													
			return FALSE;
		case WM_CLOSE:
			break;       
		case WM_DESTROY: 
			break;		


		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			break;

		case CC_COLOR_CHANGE: {
			if (HIWORD(wParam)) theHold.Begin();
			int i = LOWORD(wParam)-IDC_MULTI_COLOR1+theMtl->offset;
			IColorSwatch *cs = (IColorSwatch*)lParam;
			if (i>=0 && i<theMtl->subMtl.Count()&&theMtl->subMtl[i]) {
				theMtl->subMtl[i]->SetDiffuse(
					Color(cs->GetColor()),curTime);
				}
			UpdateColorSwatches();
			if (HIWORD(wParam)) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			    UpdateMtlDisplay();
				}
			break;
			}

		case WM_CUSTEDIT_ENTER: {
			// mask keyboard input for sub material id#'s
			for (int x=0; x<NSUBMTLS; x++) {	// check all the controls
				if (subIndex[x] == id) {		// this control is the one
					int val = iIndex[x]->GetInt();
					if (val<1)					// restrict to 1..1000     
						val=1;
					else if (val>1000)
						val=1000;
					iIndex[x]->SetText(val);
				}
			}
			UpdateMtlDisplay();
			break;
			}

		case CC_SPINNER_BUTTONUP: 
		    UpdateMtlDisplay();
			break;

    	}
	return FALSE;
	}

static BOOL CALLBACK  BasicPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	MultiDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (MultiDlg*)lParam;
		theDlg->hPanelBasic = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (MultiDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->BasicPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

void MultiDlg::UpdateColorSwatches() {

	for (int i=0; i<theMtl->subMtl.Count()-theMtl->offset && i<NSUBMTLS; i++) {
		Mtl *m = theMtl->subMtl[i+theMtl->offset];
			
		TSTR nm, label;
			if (m) 	nm = m->GetFullName();
			else 	nm = GetString(IDS_DS_NONE);
		if (m) {
			IColorSwatch *cs = GetIColorSwatch(GetDlgItem(hPanelBasic,IDC_MULTI_COLOR1+i),
				m->GetDiffuse(),nm);
			cs->SetColor(m->GetDiffuse());
			ReleaseIColorSwatch(cs);
			}
		}

	}


void MultiDlg::UpdateSubMtlNames() {
	IColorSwatch *cs;	
	int ct = theMtl->pblock->Count(multi_names);

	if (theMtl->selected<0)
		theMtl->selected = theMtl->subMtl.Count()-1;

	ShowWindow(GetDlgItem(hPanelBasic,IDC_DUP_IDS),theMtl->AnyDupIDs()?SW_SHOW:SW_HIDE);
	DrawDupIndicators();

	for (int i=0; i<theMtl->subMtl.Count()-theMtl->offset && i<NSUBMTLS; i++) {
		Mtl *m = theMtl->subMtl[i+theMtl->offset];
		TSTR nm, label;
		if (m) 
			nm = m->GetFullName();
		else 
			nm = GetString(IDS_DS_NONE);
		
		HWND hx = GetDlgItem(hPanelBasic, IDC_PSTAMP1); 
		ShowWindow(GetDlgItem(hPanelBasic, IDC_MULTI_MTL0+i), SW_SHOW);
		ShowWindow(GetDlgItem(hPanelBasic, IDC_MULTI_COLOR1+i), SW_SHOW);

		if ((m==NULL)||(m->IsMultiMtl()||m->NumSubMtls()>0))
			EnableWindow(GetDlgItem(hPanelBasic, IDC_MULTI_COLOR1+i), FALSE);
		else 
			EnableWindow(GetDlgItem(hPanelBasic, IDC_MULTI_COLOR1+i), TRUE);
		ShowWindow(GetDlgItem(hPanelBasic, subNameId[i]), SW_SHOW);
		ShowWindow(GetDlgItem(hPanelBasic,mapOnIDs[i]),SW_SHOW);
		ShowWindow(GetDlgItem(hPanelBasic,subIndex[i]),SW_SHOW); 
		ShowWindow(GetDlgItem(hPanelBasic,mtlPStampIDs[i]),SW_SHOW); 

		int on;
		Interval iv;

		if ((i+theMtl->offset)<ct)
			{
			theMtl->pblock->GetValue(multi_ons,0,on,iv,i+theMtl->offset);

			SetCheckBox(hPanelBasic, mapOnIDs[i], on);
			}
//		SetCheckBox(hPanelBasic, mapOnIDs[i], theMtl->mapOn[i+theMtl->offset]);

//		SetDlgItemText(hPanelBasic, IDC_MULTI_MTL0+i, nm.data());
		iBut[i]->SetText(nm.data());

		if (m) {
			cs = GetIColorSwatch(GetDlgItem(hPanelBasic,IDC_MULTI_COLOR1+i),
				m->GetDiffuse(),nm);
			cs->SetColor(m->GetDiffuse());
			ReleaseIColorSwatch(cs);
			}

		TCHAR *name;
		Interval niv;
		if ((i+theMtl->offset)<ct)
			{
			theMtl->pblock->GetValue(multi_names,0,name,niv,i+theMtl->offset);
			if (name) {
				TCHAR buf[256];
				iName[i]->GetText(buf,255);
				if (_tcscmp(name,buf))
					iName[i]->SetText(name);
				}
			else iName[i]->SetText(_T(""));
			int theid;
			theMtl->pblock->GetValue(multi_ids,0,theid,niv,i+theMtl->offset);
			iIndex[i]->SetText(theid+1); //?????
			}
//		TSTR buf;
//		buf.printf("%d:",i+theMtl->offset+1);
//		SetDlgItemText(hPanelBasic,numIDs[i],buf);

		HWND hwps = GetDlgItem(hPanelBasic,mtlPStampIDs[i]);
		InvalidateRect(hwps, NULL, FALSE);
		}
	for ( ; i<NSUBMTLS; i++) {
		ShowWindow(GetDlgItem(hPanelBasic, IDC_MULTI_MTL0+i), SW_HIDE);
		ShowWindow(GetDlgItem(hPanelBasic, IDC_MULTI_COLOR1+i), SW_HIDE);
		ShowWindow(GetDlgItem(hPanelBasic, subNameId[i]), SW_HIDE);
		ShowWindow(GetDlgItem(hPanelBasic,mapOnIDs[i]),SW_HIDE);
		ShowWindow(GetDlgItem(hPanelBasic,subIndex[i]),SW_HIDE); 
		ShowWindow(GetDlgItem(hPanelBasic,mtlPStampIDs[i]),SW_HIDE); 
		//		TSTR buf;
//		SetDlgItemText(hPanelBasic,numIDs[i],buf);
		}
	TSTR buf;
	buf.printf(_T("%d"),theMtl->subMtl.Count());
	SetDlgItemText(hPanelBasic,IDC_MULTI_NUMMATS,buf);
	SetScrollRange(hScroll,SB_CTL,0,theMtl->subMtl.Count()-NSUBMTLS,FALSE);
	SetScrollPos(hScroll,SB_CTL,theMtl->offset,TRUE);
	EnableWindow(hScroll,theMtl->subMtl.Count()>NSUBMTLS);
	}

void MultiDlg::LoadDialog(BOOL draw) {
	if (theMtl) {
		Interval valid;
		theMtl->Update(curTime,valid);
		UpdateSubMtlNames();		
		}
	}

void MultiDlg::SetThing(ReferenceTarget *m) {
	assert (m->ClassID()==multiClassID);
	assert (m->SuperClassID()==MATERIAL_CLASS_ID);
	RemovePStampHilite();
	if (theMtl) theMtl->paramDlg = NULL;
	theMtl = (Multi *)m;
	if (theMtl) theMtl->paramDlg = this;
	LoadDialog(TRUE);
	}

static int addNum = 5;

static INT_PTR CALLBACK NumMatsDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			ISpinnerControl *spin = 
				SetupIntSpinner(
					hWnd,IDC_MULTI_NUMMATSSPIN,IDC_MULTI_NUMMATS,
					1,1000,(int)lParam);
			ReleaseISpinner(spin);
			CenterWindow(hWnd,GetParent(hWnd));
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ISpinnerControl *spin = 
						GetISpinner(GetDlgItem(hWnd,IDC_MULTI_NUMMATSSPIN));
					EndDialog(hWnd,spin->GetIVal());
					ReleaseISpinner(spin);
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void MultiDlg::SetNumMats(HWND hWnd)
	{
	int res = DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_MULTI_SETNUM),
		hPanelBasic,
		NumMatsDlgProc,
		(LPARAM)theMtl->subMtl.Count());
	if (res>=0) {
		if (res<=0) res = 1;
		if (res>1000) res = 1000;
		if (res==theMtl->subMtl.Count())
			return;
		RemovePStampHilite();
		HCURSOR c = SetCursor(LoadCursor(NULL,IDC_WAIT));
		theHold.Begin();
		theMtl->SetNumSubMtls(res);
		theHold.Accept(GetString(IDS_DS_SET_NSUB));
		SetCursor(c);
		UpdateSubMtlNames();
		UpdateMtlDisplay();
		}
	}



//-----------------------------------------------------------------------------
//  Multi
//-----------------------------------------------------------------------------



static ParamBlockDesc pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE } };   // blend

void Multi::Init() {
	ivalid.SetEmpty();
	offset = 0;
	maxMtlId = -1;
	hashTabDirty = 1;
#if USE_HASHING
	hashTab.Initialize( 10 );
#endif
	}

void Multi::Reset() {
	Init();
//	multiCD.Reset(this, TRUE);	// reset all pb2's   <-- 4/10/01 11:27am --MQM-- this line not needed
//	for (int i=0; i<subMtl.Count(); i++)
//		DeleteReference(i+1);
	SetNumSubMtls(NSUBMTLS);
	for (int i=0; i<subMtl.Count(); i++) {
		ReplaceReference(i+1,(ReferenceTarget*)GetStdMtl2Desc()->Create());
		GetCOREInterface()->AssignNewName(subMtl[i]);
		pblock->SetValue(multi_ids,0,i,i);  
		}
	}

void* Multi::GetInterface(ULONG id)
{
	if( id == IID_IReshading )
		return (IReshading*)( this );
	else if ( id == IID_IValidityToken )
		return (IValidityToken*)( this );
	else
		return Mtl::GetInterface(id);
}

void Multi::NotifyChanged() {
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Multi::Multi(BOOL loading, BOOL createDefaultSubMtls) : mReshadeRQ(RR_None) // mjm - 06.02.00 // mjm - 10.11.99 - added createDefaultSubMtls parameter
{
	paramDlg = NULL;
	Param1 = FALSE;
	ignoreNotify = FALSE;
	selected = -1;
	
	/*
	subMtl.SetCount(NSUBMTLS);
	for (int i=0; i<NSUBMTLS; i++)  subMtl[i] = NULL;
	*/
	pblock = NULL;

	loadingOld = FALSE;
	multiCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();

	if (!loading && createDefaultSubMtls) // mjm - 10.11.99
		SetNumSubMtls(NSUBMTLS);

	pblock->DefineParamAlias(_T("material1"), multi_mtls, 0);  // JBW 5/24/99, add alias for base material to support macroRecording
}

Mtl *Multi::UseMtl() {
	Mtl* m = NULL;
	for (int i=0; i<subMtl.Count(); i++) if (subMtl[i]) { m = subMtl[i]; break; }
	return m;
	}

void Multi::ClampOffset()
	{
	if (offset>subMtl.Count()-NSUBMTLS) {
		offset=subMtl.Count()-NSUBMTLS;
		}
	if (offset<0) offset = 0;
	}

// These allow the real-time renderer to display a material appearance.
Color Multi::GetAmbient(int mtlNum, BOOL backFace) { 
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return Color(0.f,0.f,0.f);
	if (subMtl[j]) return subMtl[j]->GetAmbient(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetAmbient(mtlNum,backFace):Color(0,0,0);
	}		

Color Multi::GetDiffuse(int mtlNum, BOOL backFace){ 
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return Color(0.f,0.f,0.f);
	if (subMtl[j]) return subMtl[j]->GetDiffuse(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetDiffuse():Color(0,0,0);
	}				

Color Multi::GetSpecular(int mtlNum, BOOL backFace){
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return Color(0.f,0.f,0.f);
	if (subMtl[j]) return subMtl[j]->GetSpecular(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetSpecular():Color(0,0,0);
	}		

float Multi::GetXParency(int mtlNum, BOOL backFace) {
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return 0.0f;
	if (subMtl[j]) return subMtl[j]->GetXParency(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetXParency():0.0f;
	}

float Multi::GetShininess(int mtlNum, BOOL backFace) {
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return 0.0f;
	if (subMtl[j]) return subMtl[j]->GetShininess(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetShininess():0.0f;
	}		

float Multi::GetShinStr(int mtlNum, BOOL backFace) {
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return 0.0f;
	if (subMtl[j]) return subMtl[mtlNum]->GetShinStr(mtlNum,backFace);
	return UseMtl()?UseMtl()->GetShinStr():0.0f;
	}

float Multi::WireSize(int mtlNum, BOOL backFace) {
	int j = FindSubMtlMod(mtlNum);
	if(j<0)
		return 0.0f;
	if (subMtl[j]) return subMtl[j]->WireSize(mtlNum,backFace);
	return UseMtl()?UseMtl()->WireSize():0.0f;
	}

RefTargetHandle Multi::Clone(RemapDir &remap) {
	Multi *mnew = new Multi(FALSE, FALSE); // mjm - 10.11.99 - don't create default subMtls
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ivalid.SetEmpty();
	int nsub = subMtl.Count();
	mnew->subMtl.SetCount(nsub);
//	mnew->mapOn.SetCount(nsub);
//	mnew->subNames.SetSize(nsub);
	mnew->offset = offset;
	mnew->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	mnew->pblock->SetCount(multi_mtls, nsub);
	mnew->pblock->SetCount(multi_ons, nsub);
	mnew->pblock->SetCount(multi_names, nsub);
	mnew->pblock->SetCount(multi_ids, nsub);

	for (int i = 0; i<nsub; i++)
	{
		mnew->subMtl[i] = NULL;
		if (subMtl[i]) {
			mnew->ReplaceReference(i+1,remap.CloneRef(subMtl[i]));
//			mnew->pblock->SetValue(multi_mtls,0,mnew->subMtl[i],i);
			}
//		mnew->mapOn[i] = mapOn[i];
	}
#if USE_HASHING
//	mnew->UpdateHashTable();
	mnew->hashTabDirty = 1;										// MQM 3/5/01 - 11:44am - faster?
#endif
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


ParamDlg* Multi::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	MultiDlg *dm = new MultiDlg(hwMtlEdit, imp, this);
	dm->LoadDialog(TRUE);	
	SetParamDlg(dm);
	return dm;	
	}

void Multi::Update( TimeValue t, Interval &valid ) 
{		
	int		count;												// num sub materials
	int		id;													// id of current sub mat
	Interval iv;												// current interval
	int		i;													// loop var

	// update all submaterials
	if ( !ivalid.InInterval( t ) ) 
	{
//#if USE_HASHING
//		hashTab.Clean();
//#endif

		ivalid.SetInfinite();

		// loop through and update all sub materials
		maxMtlId = -1;
		count = subMtl.Count();
		for ( i = 0;  i < count;  i++ ) 
		{
			// tell sub material to update
			if ( subMtl[i] )
				subMtl[i]->Update( t, ivalid );

			// get the material id from the sub material
			pblock->GetValue( multi_ids, 0, id, iv, i ); 
			if ( id > maxMtlId )
				maxMtlId = id;									// UPDATE - cache max id#

//#if USE_HASHING
//			hashTab.AddAssociation( id, i );
//#endif
		}
	}
	valid &= ivalid;
}

Interval Multi::Validity(TimeValue t) {
	Interval valid;
	Update(t,valid);
	return ivalid;
	}

void Multi::GetSubMtlName(int mtlid, TSTR &s) {	
	TCHAR *name;
	int j = FindSubMtl(mtlid);
	if (j>=0) {
		Interval iv;
		pblock->GetValue(multi_names,0,name,iv,j);
		s = name;
		}
	else 
		s = _T("");
	}



struct sortEl{
	int i;
	Mtl *mtl;
	BOOL on;
	int mtlid; 
	TCHAR name[100];
	};

static int __cdecl cmpIDs( const void *arg1, const void *arg2 ) {
	sortEl *s1 = (sortEl *)arg1;
	sortEl *s2 = (sortEl *)arg2;
	int id1 = s1->mtlid;
	int id2 = s2->mtlid;
	return id2<id1? 1: id1<id2? -1:0;
	}

static int __cdecl cmpNames( const void *arg1, const void *arg2 ) {
	sortEl *s1 = (sortEl *)arg1;
	sortEl *s2 = (sortEl *)arg2;
	int id1 = s1->mtlid;
	int id2 = s2->mtlid;
	Mtl * m1 = s1->mtl;
	Mtl * m2 = s2->mtl;
	if (m1&&m2) {
		int res = _tcsicmp(m1->GetFullName(), m2->GetFullName());
		if (res == 0) goto useid;
		return res;
		}
	if (m1) return -1;
	if (m2) return 1;
	useid:
		return id2<id1? 1: id1<id2? -1:0;
	}

static int __cdecl cmpSlotNames( const void *arg1, const void *arg2 ) {
	sortEl *s1 = (sortEl *)arg1;
	sortEl *s2 = (sortEl *)arg2;
	int id1 = s1->mtlid;
	int id2 = s2->mtlid;
	int res = _tcsicmp(s1->name, s2->name);
	if (res != 0) 
		return res;
	return id2<id1? 1: id1<id2? -1:0;
	}

void Multi::SortMtlsByName() {
	SortMtls(cmpNames);
	}

void Multi::SortMtlsByID() {
	SortMtls(cmpIDs);
	}

void Multi::SortMtlsBySlotName() {
	SortMtls(cmpSlotNames);
	}

BOOL Multi::AnyDupIDs() {
	if (paramDlg == NULL)
		return 0;
	for (int j=0; j<NSUBMTLS; j++)
		paramDlg->isDup[j] = FALSE;
	Tab<sortEl> sortlist;
	int n = subMtl.Count();
	sortlist.SetCount(n);
	for (int i=0; i<n; i++) {
		sortEl &s = sortlist[i];
		s.i = i;
		Interval iv;
		pblock->GetValue(multi_ids,0, s.mtlid,iv,i); 
		}
	sortlist.Sort(cmpIDs);
	int lastid = -999;
	int lastIndex = -1;
	BOOL anyDups = FALSE;
	for ( i=0; i<n; i++) {
		sortEl &s = sortlist[i];
		int id = s.mtlid;
		int indx = s.i;
		if (id==lastid) {
			anyDups = TRUE;
			paramDlg->SetIsDup(lastIndex-offset);
			paramDlg->SetIsDup(s.i-offset);
			}
		lastid = id;
		lastIndex = s.i;
		}
	return anyDups;
	}

void Multi::SortMtls(CompareFnc cmp) {
	Tab<sortEl> sortlist;
	int n = subMtl.Count();
	sortlist.SetCount(n);
	for (int i=0; i<n; i++) {
		sortEl s;
		Interval iv;
		pblock->GetValue(multi_ids,0, s.mtlid,iv,i); 
		pblock->GetValue(multi_mtls, 0, s.mtl,iv,i);
		pblock->GetValue(multi_ons, 0, s.on,iv,i);
		TCHAR *name;
		pblock->GetValue(multi_names,0,name,iv,i);
		if (name) 
			_tcscpy(s.name,name);
		else s.name[0] = 0;

		sortlist[i] = s;
		if (subMtl[i]) subMtl[i]->SetAFlag(A_LOCK_TARGET);
		}
	sortlist.Sort(cmp);
	for ( i=0; i<n; i++) {
		sortEl &s = sortlist[i];
		pblock->SetValue(multi_ids,0,s.mtlid,i);
		pblock->SetValue(multi_mtls,0,s.mtl,i);
		pblock->SetValue(multi_ons,0,s.on,i);
		pblock->SetValue(multi_names,0,s.name,i);
		subMtl[i] = s.mtl;
		}		
	for ( i=0; i<n; i++) {
		if (subMtl[i]) subMtl[i]->ClearAFlag(A_LOCK_TARGET);
		}
#if USE_HASHING
	hashTabDirty = 1;											// MQM 3/5/01 - 11:45am - faster?
//	UpdateHashTable();
#endif
	NotifyChanged();
	}



class DelSubRestore: public RestoreObj {
	Multi *mtl;
	SingleRefMaker subm;
	int nsub;
	TSTR name;
	BOOL on;
	int id;
	public:
		DelSubRestore() { }
		DelSubRestore(Multi *m, int i);
		~DelSubRestore(){}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			TCHAR buf[100];
			_stprintf(buf,_T("DelSubMtlRestore"));
			return(TSTR(buf));
			}
	};

DelSubRestore::DelSubRestore(Multi *m,  int i) {
	theHold.Suspend();
	nsub = i;
	mtl = m;
	subm.SetRef(m->subMtl[i]);
	Interval iv;
	mtl->pblock->GetValue(multi_ids,0,id,iv,i);
	mtl->pblock->GetValue(multi_ons,0,on,iv,i);
	TCHAR *pname;
	mtl->pblock->GetValue(multi_names,0,pname,iv,i);
	name = pname;
	theHold.Resume();
	}

void DelSubRestore::Restore(int isUndo) {
	if (isUndo) {
		Mtl *foo = NULL;
		if (mtl->paramDlg)
			mtl->paramDlg->RemovePStampHilite();
		mtl->subMtl.Insert(nsub, 1, &foo);
		Mtl *sm = (Mtl *)subm.GetRef();
		mtl->ReplaceReference(nsub+1, sm);
		mtl->pblock->Insert(multi_mtls, nsub, 1, &sm);
		mtl->pblock->Insert(multi_ons, nsub,  1, &on);
		TCHAR *pname = name.data();
		mtl->pblock->Insert(multi_names, nsub, 1, &pname);
		mtl->pblock->Insert(multi_ids, nsub, 1, &id);
		mtl->selected = nsub;
#if USE_HASHING
		mtl->hashTabDirty = 1;									// MQM 3/5/01 - 11:45am - faster?
//		mtl->UpdateHashTable();
#endif
		mtl->NotifyChanged();
		}
	}

void DelSubRestore::Redo() {
	if (mtl->paramDlg)
		mtl->paramDlg->RemovePStampHilite();
	mtl->ReplaceReference(nsub+1, NULL);
	mtl->subMtl.Delete(nsub, 1);
	mtl->pblock->Delete(multi_mtls, nsub, 1);
	mtl->pblock->Delete(multi_ons, nsub, 1);
	mtl->pblock->Delete(multi_names, nsub, 1);
	mtl->pblock->Delete(multi_ids, nsub, 1);
	if (mtl->selected>=mtl->subMtl.Count())
			mtl->selected = mtl->subMtl.Count() -1;
#if USE_HASHING
	mtl->hashTabDirty = 1;										// MQM 3/5/01 - 11:45am - faster?
//	mtl->UpdateHashTable();
#endif
	mtl->NotifyChanged();
	}

class AddMtlsRestore : public RestoreObj {
	public:
		Multi *m;
		int num;
		SingleRefMaker *subm;
		TSTR *_name;
		int *_id;
		
		
		AddMtlsRestore(Multi *mul, int n) {
			m  = mul;
			num = n;

			subm = new SingleRefMaker[n];
			_name = new TSTR[n];
			_id = new int[num];
			}

		~AddMtlsRestore() {
			delete [] subm;
			delete []_name;
			delete [] _id;
			}

		//Used to store all the missing data for all the materials being added
		//At creation time, only n and num can be set.
		void SetMissingFields(Multi *mul, int n)
		{
			theHold.Suspend();

			for(int i=0;i<num;i++)
			{
 				subm[i].SetRef(mul->subMtl[n+i]);
				Interval iv;
				mul->pblock->GetValue(multi_ids,0,_id[i],iv,n+i);
				TCHAR *pname;
				mul->pblock->GetValue(multi_names,0,pname,iv,n+i);
				_name[i] = pname;
			}
			theHold.Resume();
		}

		void Restore(int isUndo) {
			for (int i=0; i<num; i++) {
				m->selected = m->subMtl.Count()-1;
				m->DeleteSelMtl();
				}			
			}
		void Redo() {
			for (int i=0; i<num; i++) 
			{
				m->AddMtl(subm[i].GetRef(),_id[i],_name[i]);
			}
			m->offset = m->subMtl.Count()-NSUBMTLS;
			m->ClampOffset();
			}
		TSTR Description() {
			TCHAR buf[100];
			_stprintf(buf,_T("AddMtlsRestore"));
			return(TSTR(buf));
			}
	};

void Multi::AddMtl(ReferenceTarget *rt, int id, TCHAR *name) {
	SuspendSetKeyMode();

	AddMtlsRestore *undoData=NULL;

	if (theHold.Holding()) 
		theHold.Put( undoData = new AddMtlsRestore(this,1));
	theHold.Suspend();
	int n = subMtl.Count()+1;
	int maxid = MaxSubMtlID();
	subMtl.SetCount(n);
	
	subMtl[n-1] = NULL;  // orb 08-24-2001 Fixing Defects 307753 & 306999

#if USE_HASHING
	if(rt!=NULL)
	    maxMtlId++;				// 5/2/01 1:26pm --MQM-- moved from #if block below to fix callback/out-of-sync problems
#endif
	macroRec->Disable();	// DS 10/13/00
	pblock->SetCount(multi_ons, n);
	pblock->SetCount(multi_names, n);
	pblock->SetCount(multi_ids, n);
	macroRec->Enable();    // DS 10/13/00
	pblock->SetCount(multi_mtls, n);
	n--;
	
 	pblock->SetValue(multi_ons,0,TRUE,n);

	if(rt)
	{
		pblock->SetValue(multi_ids,0,id,n);
 		pblock->SetValue(multi_names,0,name,n);
		ReplaceReference(n+1,rt);
	}
	else
	{
 		pblock->SetValue(multi_ids,0,maxid+1,n);
		pblock->SetValue(multi_names,0,_T(""),n); // LAM 11/21/00
		ReplaceReference(n+1,(ReferenceTarget*)GetStdMtl2Desc()->Create());
		GetCOREInterface()->AssignNewName(subMtl[n]);
	}

	if(undoData)
		undoData->SetMissingFields(this,n);

	NotifyChanged();
	theHold.Resume();
#if USE_HASHING
	if ( hashTabDirty )
		UpdateHashTable();
	else
		hashTab.AddAssociation( maxMtlId, n );
#endif
	ResumeSetKeyMode();
	}

void Multi::SetNumSubMtls(int num) {
	int n = num-subMtl.Count();
	if (n>0) {
		
		int startOffset = subMtl.Count();

		AddMtlsRestore *undoData=NULL;
		if (theHold.Holding()) 
			theHold.Put(undoData = new AddMtlsRestore(this,n));

		theHold.Suspend();
		for (int i=0; i<n; i++) 
			AddMtl();

		if(undoData)
			undoData->SetMissingFields(this,startOffset);

		theHold.Resume();
		}
	else if (n<=0) {
		n = -n;
		for (int i=0; i<n; i++) {
			if(subMtl.Count()==1)        //az:  #502733 fix  - don't let delete the last material
				break;
			selected = subMtl.Count()-1;
			DeleteSelMtl();
			}			
		selected = subMtl.Count()-1;
		}
	offset = subMtl.Count()-NSUBMTLS;
	ClampOffset();
#if USE_HASHING
	hashTabDirty = 1;										// MQM 3/5/01 - 11:45am - faster?
//	UpdateHashTable();   // clean up when removing mats
#endif
	}
	
void Multi::DeleteSelMtl() {
	if (selected>=0&&selected<subMtl.Count()) {
		if (theHold.Holding())
			theHold.Put(new DelSubRestore(this, selected));
		theHold.Suspend();
		ReplaceReference(selected+1, NULL);
		subMtl.Delete(selected, 1);
		macroRec->Disable();	// DS 10/13/00
		pblock->Delete(multi_ons, selected, 1);
		pblock->Delete(multi_names, selected, 1);
		pblock->Delete(multi_ids, selected, 1);
		macroRec->Enable();	// DS 10/13/00
		pblock->Delete(multi_mtls, selected, 1);
		if (selected>=subMtl.Count())
			selected = subMtl.Count() -1;
#if USE_HASHING
		hashTabDirty = 1;										// MQM 3/5/01 - 11:46am - faster?
//		UpdateHashTable();
#endif
		ClampOffset();
		NotifyChanged();
		theHold.Resume();
		}
	}

 
//need to remap references since we added a paramblock
int Multi::RemapRefOnLoad(int iref) 
{
if (Param1) iref += 1;
return iref;
}

RefTargetHandle Multi::GetReference(int i) {
	if (loadingOld) {
		if (i==0) return NULL;
		else return subMtl[i-1];
		}
	else 
		{
		if (i==PBLOCK_REF) return pblock;
		else return subMtl[i-1];
		}
	}

void Multi::SetReference(int i, RefTargetHandle rtarg) {
	if ((i-1)>=subMtl.Count()) {
		int n = subMtl.Count();
		SetNumSubMtls(i+1);
//		subMtl.SetCount(i+1);
//		for (int j=n; j<=i; j++) // mjm - 10,11.99
//			subMtl[j] = NULL;    // mjm - 10,11.99 - default subMtl created in SetNumSubMtls()
	}
	if (loadingOld) {
		if (i==0|| (rtarg&&!IsMtl(rtarg)))  
			{ } //pblock = (IParamBlock *)rtarg;
		else 
			subMtl[i-1] = (Mtl *)rtarg;
		}
	else 
		{
		if (i==PBLOCK_REF) pblock = (IParamBlock2 *)rtarg;
 		else subMtl[i-1] = (Mtl *)rtarg;
		}
	}



class SetSubRestore: public RestoreObj {
	Multi *multi;
	Mtl *mtl;
	SingleRefMaker subold;
	SingleRefMaker subnew;
	int nsub;
	int mtlid;
	TSTR oldnm;
	TSTR newnm;
	BOOL doname;
	public:
		SetSubRestore() { }
		SetSubRestore(Multi *m, int i, Mtl *mt, int mid, TCHAR *newName=NULL) {
			multi = m;
			nsub = i;
			mtl = mt;
			mtlid = mid;
			doname = FALSE;
			if (newName) {
				doname = TRUE;
				newnm = newName; 
				if (nsub>=0) {
					TCHAR *pname;
					Interval niv;
					multi->pblock->GetValue(multi_names,0,pname,niv,nsub);
					oldnm = pname;
					}
				}
			theHold.Suspend();
			if (i>=0)
				subold.SetRef(multi->subMtl[i]);
			subnew.SetRef(mt);
			theHold.Resume();
			}
		~SetSubRestore(){}
		void Restore(int isUndo) {
			if (nsub>=0) {
				Mtl *sm = (Mtl *)subold.GetRef();
				multi->ReplaceReference(nsub+1, sm);
				if (doname) {
					TCHAR *pname = oldnm.data();
					multi->pblock->SetValue(multi_names,0,pname,nsub);
					}
				}
			else {
				int n = multi->subMtl.Count();
				multi->SetNumSubMtls(n-1);
				}
			if (multi->paramDlg)	  
				multi->paramDlg->UpdateSubMtlNames();
			}
		void Redo() {
			Mtl *sm = (Mtl *)subnew.GetRef();
			multi->SetSubMtlAndName(mtlid,sm,newnm);
			if (doname) 
				multi->SetSubMtlAndName(mtlid,sm,newnm);
			else 
				multi->SetSubMtl(mtlid,sm);
			}
		TSTR Description() {
			TCHAR buf[100];
			_stprintf(buf,_T("SetSubMtlRestore"));
			return(TSTR(buf));
			}
	};

void Multi::SetSubMtlAndName(int mtlid, Mtl *m, TSTR &nm) {
	int j = FindSubMtl(mtlid);
	if (theHold.Holding()) 
		theHold.Put(new SetSubRestore(this,j,m,mtlid,nm.data()));
	theHold.Suspend();
	if (j>=0) {
		ReplaceReference(j+1,m);
		TCHAR *pname = nm.data();
		pblock->SetValue(multi_names,0,pname,j);
		}
	else {
		int n = subMtl.Count();
		SetNumSubMtls(n+1);
		ReplaceReference(n+1,m);
		// set the n-th mtl to have mtlid for its mtl ID.
		pblock->SetValue(multi_ids,0,mtlid,n);
		pblock->SetValue(multi_names,0,nm.data(),n);
#if USE_HASHING
		if ( mtlid > maxMtlId )									// 5/22/01 3:31pm --MQM-- bug #252504, NumSubMtls() getting called but maxMtlId not updated
			maxMtlId = mtlid;
		hashTabDirty = 1;											// MQM 3/5/01 - 11:47am - faster?
#endif
		}
	if (paramDlg)	  
		paramDlg->UpdateSubMtlNames();
#if USE_HASHING
//	UpdateHashTable();
#endif
	theHold.Resume();
	}

void Multi::RemoveMtl(int mtlid)
{
	int found = FindSubMtl(mtlid);
	if (found >= 0 && (subMtl.Count()>1)) {
		selected = found;
		DeleteSelMtl();
		selected = subMtl.Count()-1;
		if(paramDlg){
			paramDlg->RemovePStampHilite();
			paramDlg->Invalidate();
			paramDlg->UpdateMtlDisplay();
		}
	}
}

void Multi::SetSubMtl(int i, Mtl *m) {
	int j = FindSubMtl(i);
	if (theHold.Holding()) 
		theHold.Put(new SetSubRestore(this,j,m,i));
	theHold.Suspend();
	if (j>=0) {
		ReplaceReference(j+1,m);
		}
	else {
		int n = subMtl.Count();
		SetNumSubMtls(n+1);
		ReplaceReference(n+1,m);
		// set the n-th mtl to have i for its mtlID.
		pblock->SetValue(multi_ids,0,i,n);
#if USE_HASHING
		if ( i > maxMtlId )										// 5/22/01 3:31pm --MQM-- bug #252504, NumSubMtls() getting called but maxMtlId not updated
			maxMtlId = i;
		hashTabDirty = 1;											// MQM 3/5/01 - 11:47am - faster?
#endif
		}
	if (paramDlg)	  
		paramDlg->UpdateSubMtlNames();
#if USE_HASHING
//	UpdateHashTable();
#endif
	theHold.Resume();
	}

TSTR Multi::GetSubMtlSlotName(int i) {
	TSTR s;
	TCHAR *name;
	Interval iv;

	int j = FindSubMtl(i);
	if (j>=0) {
		pblock->GetValue(multi_names,0,name,iv,j);
		if (name) 
			s.printf("(%d) %s",i+1,name);
		else 
			s.printf("(%d)",i+1);
		}

	else 
		s.printf("(%d)",i+1);
	return s;
	}

Animatable* Multi::SubAnim(int i) {
//	if (i==PBLOCK_REF) return pblock;
//	else return subMtl[i-1]; 
	return subMtl[i]; 
	}

TSTR Multi::SubAnimName(int i) {
//	return GetSubMtlTVName(i);
	TCHAR *name;
	Interval iv;
	int id;
	pblock->GetValue(multi_names,0,name,iv,i);
	pblock->GetValue(multi_ids,0,id,iv,i);
	TSTR s;
	if (name) 
		s.printf("(%d) %s",id+1,name);
	else 
		s.printf("(%d)",id+1);
	TSTR nm;
	if (subMtl[i]) 
		nm.printf(_T("%s: %s"), s.data(),  subMtl[i]->GetFullName().data() );
	else 
		nm.printf(_T("%s: None"), s.data());
	return nm;
	}

RefResult Multi::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			if (ignoreNotify)
				return REF_SUCCEED; // succeed????
//				return REF_STOP;
			ivalid.SetEmpty();
			if (hTarget == pblock){
				ParamID changing_param = pblock->LastNotifyParamID();
				multi_param_blk.InvalidateUI(changing_param);
				if( (changing_param == multi_ons )
					||(changing_param == multi_ids ))
					mReshadeRQ = RR_NeedReshade;

				// 6/1/01 10:58am --MQM-- 
				// maxscript or other has changed a sub-mtl id
				if ( changing_param == multi_ids )
				{
					// hash table is now dirty, since the
					// max mtl id may be different now
					hashTabDirty = 1;
				}

				// > 10/29/02 - 12:21pm --MQM-- 
				// if the dialog is open, update it to
				// reflect the changes.  this used to be in
				// the "if ( changing_param == multi_ids )" above,
				// but we need to update for names, etc.
				if ( paramDlg != NULL )
					paramDlg->Invalidate();
			}

			if (IsMtl(hTarget)) {
				int n = subMtl.Count();
				for (int i=0; i<n; i++) {
					if (subMtl[i] && ((Mtl *)hTarget==subMtl[i])) {
						subMtl[i]->DiscardPStamp(PS_TINY);
						IReshading* r = static_cast<IReshading*>(subMtl[i]->GetInterface(IID_IReshading));
						mReshadeRQ = r == NULL ? RR_None : r->GetReshadeRequirements();
						break;
					}
				}
			}

			// following by JBW 45/21/99 to allow scripter-setting of submtl counts (any count change updates others)
			if ( pblock )
			{
				int newCount = -1;
				
				if      ( pblock->LastNotifyParamID()  == multi_ons  &&
						  pblock->Count( multi_ons )   != subMtl.Count() )
				{
					newCount = pblock->Count( multi_ons );
				}
				else if ( pblock->LastNotifyParamID()  == multi_names  &&
						  pblock->Count( multi_names ) != subMtl.Count() )
				{
					newCount = pblock->Count( multi_names );
				}
				else if ( pblock->LastNotifyParamID()  == multi_ids &&
						  pblock->Count( multi_ids )   != subMtl.Count() )
				{
					newCount=pblock->Count( multi_ids );
				}
				else if ( pblock->LastNotifyParamID()  == multi_mtls &&
						  pblock->Count( multi_mtls )  != subMtl.Count() )
				{
					newCount=pblock->Count( multi_mtls );
				}

				// count has changed
				if ( newCount != -1 ) 
				{	
					// > 6/12/02 - 1:51pm --MQM-- 
					// don't allow 0 count...maxscript was allowing this, but we don't want it anymore
					if ( newCount == 0 )
						newCount = 1;

					ignoreNotify = TRUE;
					pblock->SetCount( pblock->LastNotifyParamID(), subMtl.Count() ); // to make DelSubRestore happy
					SetNumSubMtls( newCount );
					mReshadeRQ = RR_NeedReshade;
					if ( paramDlg != NULL )
					{
						paramDlg->RemovePStampHilite();
						paramDlg->Invalidate();
						paramDlg->UpdateMtlDisplay();
					}
					ignoreNotify = FALSE;
				}
			}


			if (hTarget != NULL) {
				switch (hTarget->SuperClassID()) {
					case MATERIAL_CLASS_ID: {
						IReshading* r = static_cast<IReshading*>(hTarget->GetInterface(IID_IReshading));
						mReshadeRQ = r == NULL ? RR_None : r->GetReshadeRequirements();
					} break;
				}
			}
			break;

		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			if( hTarget != this )
				return REF_SUCCEED;

			mReshadeRQ = RR_NeedPreshade;
			NotifyChanged();
			break;

//		case REFMSG_SUSPEND_STRUCTURE_CHANGED:
//			ignoreNotify = TRUE;
//			break;
//
//		case REFMSG_RESUME_STRUCTURE_CHANGED:
//			ignoreNotify = FALSE;
//			break;

		case REFMSG_GET_PARAM_DIM: 
		{
			GetParamDim *gpd = (GetParamDim*)partID;
			switch ( gpd->index )								// 4/24/01 3:22pm --MQM-- fix for maxscript #'s to match
			{
			case multi_mtls: 
			case multi_ons:
			case multi_names: 
				gpd->dim = defaultDim;
				break;

			case multi_ids: 
				gpd->dim = &theMultiIDDim;
				break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}


inline void Clamp(Color &c) {
	if (c.r > 1.0f) c.r = 1.0f;
	if (c.g > 1.0f) c.g = 1.0f;
	if (c.b > 1.0f) c.b = 1.0f;
	}

static Color black(0,0,0);

Sampler*  Multi::GetPixelSampler(int mtlNum, BOOL backFace )
{
	int j = FindSubMtlMod( mtlNum );
	Mtl* subM = (j>=0)? subMtl[j] : NULL;
	if ( subM )
		return subM->GetPixelSampler( mtlNum, backFace );
	return NULL;
}

void Multi::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	IReshading* pReshading;
	int mtlnum = sc.mtlNum, ct = subMtl.Count();
	// NOTE: preshade each submaterial to allow user to switch among them during reshading
	if (ct)
	{
		int j = FindSubMtlMod(mtlnum);
		Mtl* subm = (j>=0)?subMtl[j]:NULL;
		if ( subm )
		{
			// store sub-material number and preshade it
			pFrag->AddIntChannel(mtlnum);
			pReshading = (IReshading*)(subm->GetInterface(IID_IReshading));
			if( pReshading ) 
				pReshading->PreShade(sc, pFrag);

		}
		else
		{
			// -1 indicates no submaterial used
			pFrag->AddIntChannel(-1);
		}
	}
	else
	{
		// -1 indicates no submaterial used
		pFrag->AddIntChannel(-1);
	}
}

void Multi::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams*)
{
	int mtlnum = pFrag->GetIntChannel(nextTexIndex++);
	if (mtlnum == -1)
	{
		// no submaterial was used
		sc.out.c.Black();
		sc.out.t.Black();
		return;
	}

	// submaterial used, let it postshade
	int j = FindSubMtlMod(mtlnum);
	Mtl *subm = NULL;

	IReshading* pReshading;
	if (j>=0) {
		subm = subMtl[j];
		if (subm) {
			// always shade to skip
			pReshading = (IReshading*)(subm->GetInterface(IID_IReshading));
			if( pReshading ) 
				pReshading->PostShade(sc, pFrag, nextTexIndex);

			int on;
			// then see if it's on
			pblock->GetValue(multi_ons,0,on,FOREVER,j);
			if (!on) 
				subm = NULL;
		}// if subm
	} // j >= 0

	// black for no or off material
	if (!subm){
		sc.out.c.Black();
		sc.out.t.Black();
	}
}



// if this function changes, please also check SupportsReShading, PreShade and PostShade
// end - ke/mjm - 03.16.00 - merge reshading code
// [attilas|29.5.2000] if this function changes, please also check EvalColorStdChannel
void Multi::Shade(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	int mtlnum = sc.mtlNum, ct = subMtl.Count();
	if (ct)
	{

//		if (mtlnum < 0)	mtlnum = 0;
//		if (mtlnum >= ct) mtlnum = mtlnum % ct;		
//		Mtl* subm = subMtl[mtlnum];

		int j = FindSubMtlMod(mtlnum);
		if (j<0) 
			return;
		Mtl* subm = subMtl[j];
		int on;
		Interval iv;
		pblock->GetValue(multi_ons,0,on,iv,j);
		if (subm&&on) 
			subm->Shade(sc);	//no handling for render elements needed
		}
	}

float Multi::EvalDisplacement( ShadeContext& sc ) 
{
	if ( subMtl.Count() ) 
	{
		// find the submaterial
		int j = FindSubMtlMod( sc.mtlNum );
		if ( j < 0 )											// bail out now if material doesn't exist
			return 0.0f;
		Mtl* subm = subMtl[j];

		int on;
		Interval iv;
		pblock->GetValue( multi_ons, 0, on, iv, j );
		if ( on )
			return subm->EvalDisplacement( sc );		
	}

	return 0.0f;
}

Interval Multi::DisplacementValidity(TimeValue t){
	int ct = subMtl.Count();
	Interval iv;
	iv.SetInfinite();
	for (int i=0; i<ct; i++) {
		Mtl* subm = subMtl[i];
		int on;
		Interval iv;
		pblock->GetValue(multi_ons,0,on,iv,i);
		if (subm&&on) 
			iv &= subm->DisplacementValidity(t);		
		}
	return iv;
	} 

#define MTL_HDR_CHUNK 0x4000
#define MULTI_NUM_OLD 0x4001
#define MULTI_NUM 0x4002
#define MULTI_NAMES 0x4010
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x4003
#define PARAM2_NEW_CHUNK 0x4005  // Started saving this 6/19/00 to indicate the new array structure

IOResult Multi::Save(ISave *isave) { 
	IOResult res;
	ULONG nb;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_NEW_CHUNK);
	isave->EndChunk();

	//int numSubs = NSUBMTLS;
	int numSubs = subMtl.Count();
	isave->BeginChunk(MULTI_NUM);
	isave->Write(&numSubs,sizeof(numSubs),&nb);			
	isave->EndChunk();


//	isave->BeginChunk(MULTI_NAMES);
//	subNames.Save(isave);
//	isave->EndChunk();

/*
	for (int i=0; i<subMtl.Count(); i++) {
		if (mapOn[i]==0) {
			isave->BeginChunk(MAPOFF_CHUNK+i);
			isave->EndChunk();
			}
		}
*/

	return IO_OK;
	}	


//2-18-96
class MultiPostLoad : public PostLoadCallback {
	public:
		Multi *m;
		MultiPostLoad(Multi *b) {m=b;}
		void proc(ILoad *iload) {  
			m->loadingOld = FALSE; 
			delete this; 
			} 
	};

//watje
class Multi2PostLoadCallback:public  PostLoadCallback
{
public:
	Multi      *s;
	Tab<BOOL> ons;
//	NameTab subNames;

	int Param1,oldArray;
	Multi2PostLoadCallback(Multi *r, BOOL b, Tab<BOOL> bl, BOOL oldA/*,	NameTab sNames*/) {s=r;Param1 = b;ons = bl; oldArray = oldA; /*subNames = sNames;*/}

        // 420314 : CR
	// Need to make the priority higher. The tables need to be fixed up as soon as posible.
	// If Update() gets called before the PLCB has run, this will cause crashes
        // I picked the highest number. Since most object PLCB have 5, I think 4 could have done the same job
        // just wanted to be safe.
	int Priority() { return 1; }  
	void proc(ILoad *iload);
};

void Multi2PostLoadCallback::proc(ILoad *iload)
{
	if (Param1)
		{
		s->pblock->SetCount(multi_ons,ons.Count());
		s->pblock->SetCount(multi_names,ons.Count());
		s->pblock->SetCount(multi_mtls,ons.Count());
		s->pblock->SetCount(multi_ids,ons.Count());
		for (int i=0; i<s->subMtl.Count(); i++) {
			s->pblock->SetValue(multi_ons,0,ons[i],i);
			if (s->subNames[i])
				s->pblock->SetValue(multi_names,0,s->subNames[i],i);
			s->pblock->SetValue(multi_ids,0,i,i);
			}
		}
	else
		{
		if (s->pblock->Count(multi_mtls) != s->subMtl.Count())
			{
			s->pblock->SetCount(multi_mtls,s->subMtl.Count());
			s->pblock->SetCount(multi_names,s->subMtl.Count());
			s->pblock->SetCount(multi_ons,s->subMtl.Count());
			}
		if (oldArray) {
			s->pblock->SetCount(multi_ids,s->subMtl.Count());
			for (int i=0; i<s->pblock->Count(multi_mtls); i++) {
				s->pblock->SetValue(multi_ids,0,i,i);
				}
			}
		}
                
#if USE_HASHING
	s->hashTabDirty = 1;										// MQM 3/5/01 - 11:47am - faster?
//	s->UpdateHashTable();
#endif

	delete this;
}
	  
IOResult Multi::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	Param1 = TRUE;
	BOOL oldArray = TRUE;

	Tab<BOOL>mapOn;
//	NameTab subNames;


	while (IO_OK==(res=iload->OpenChunk())) {
		int id = iload->CurChunkID();

		if (id>=MAPOFF_CHUNK&&id<=MAPOFF_CHUNK+0x1000) {
			mapOn[id-MAPOFF_CHUNK] = FALSE; 
			}
		else 

		switch(id)  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MULTI_NUM_OLD: 
				iload->SetObsolete();
				iload->RegisterPostLoadCallback(new MultiPostLoad(this));
				loadingOld = TRUE;
			case MULTI_NUM: {
				int numSubs;
				iload->Read(&numSubs,sizeof(numSubs),&nb);			
				subMtl.SetCount(numSubs);
				mapOn.SetCount(numSubs);
				subNames.SetSize(numSubs);

				for (int i=0; i<numSubs; i++) {	
					subMtl[i] = NULL;
					mapOn[i] = TRUE;
					}
				}
				break;
			case MULTI_NAMES:
				res = subNames.Load(iload);	
				subNames.SetSize(subMtl.Count());
				break;
			case PARAM2_NEW_CHUNK:
				oldArray = FALSE;				
				// fall thru...
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	Multi2PostLoadCallback* multiplcb = new Multi2PostLoadCallback(this,Param1,mapOn,oldArray);
	iload->RegisterPostLoadCallback(multiplcb);
	return IO_OK;
	}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Returns true if the submaterial specified in the shade context is constant

bool Multi::IsOutputConst
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID				// must be ID_AM, ect
)
{
	int mtlnum = sc.mtlNum, ct = subMtl.Count();
	if (ct)
	{
//		if (mtlnum < 0)
//			mtlnum = 0;
//		if (mtlnum >= ct)
//			mtlnum = mtlnum % ct;		
//		Mtl* subm = subMtl[mtlnum];

		int j = FindSubMtlMod(mtlnum);
		if (j >= 0) {
			Mtl* subm = subMtl[j];

			int on;
			Interval iv;
			pblock->GetValue(multi_ons,0,on,iv,j);
			if ( subm && on ) 
				return subm->IsOutputConst( sc, stdID );		
		}
	}
	return true;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
// For a mono channel, the value is copied in all 3 components of the 
// output color.
// 
bool Multi::EvalColorStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	Color& outClr			// output var
)
{
	int mtlnum = sc.mtlNum, ct = subMtl.Count();
	if (ct)
	{
//		if (mtlnum < 0)
//			mtlnum = 0;
//		if (mtlnum >= ct)
//			mtlnum = mtlnum % ct;		
//		Mtl* subm = subMtl[mtlnum];

		int j = FindSubMtlMod(mtlnum);
		if (j >= 0) {
			Mtl* subm = subMtl[j];

			int on;
			Interval iv;
			pblock->GetValue(multi_ons,0,on,iv,j);
			if ( subm && on ) 
				return subm->EvalColorStdChannel( sc, stdID, outClr );
		}
	}
	return false;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
//
bool Multi::EvalMonoStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	float& outVal			// output var
)
{
	int mtlnum = sc.mtlNum, ct = subMtl.Count();
	if (ct)
	{
//		if (mtlnum < 0)
//			mtlnum = 0;
//		if (mtlnum >= ct)
//			mtlnum = mtlnum % ct;		
//		Mtl* subm = subMtl[mtlnum];

		int j = FindSubMtlMod(mtlnum);
		Mtl* subm = j>=0?subMtl[j]:NULL;

		int on;
		Interval iv;
		pblock->GetValue(multi_ons,0,on,iv,j);
		if ( subm && on ) 
			return subm->EvalMonoStdChannel( sc, stdID, outVal );
	}
	return false;
}





/*

class SetNumMtlsRestore : public RestoreObj {
	public:
		Multi *multi;
		Tab<Mtl*> undo, redo;
//		Tab<BOOL> undoMO, redoMO;
		SetNumMtlsRestore(Multi *m) {
			multi  = m;
			undo   = multi->subMtl;
//			undoMO = multi->mapOn;
			}
   		
		void Restore(int isUndo) {
			if (isUndo) {
				redo   = multi->subMtl;
//				redoMO = multi->mapOn;
				}
			multi->subMtl = undo;
//			multi->mapOn  = undoMO;
			}
		void Redo() {
			multi->subMtl = redo;
//			multi->mapOn  = redoMO;
			}
	};

void Multi::SetNumSubMtls(int n)
	{
	int ct = subMtl.Count();
	if (n!=ct) {
		if (n<ct) {
			for (int i=n; i<ct; i++) {
				if (subMtl[i])
					subMtl[i]->DeactivateMapsInTree();
				ReplaceReference(i+1,NULL);
				}
			}
		subMtl.SetCount(n);
//		subNames.SetSize(n);
//		mapOn.SetCount(n);
//		pblock->SetCount(multi_mtls,n);
		if (n>ct) {
			for (int i=ct; i<subMtl.Count(); i++) {
				subMtl[i] = NULL;
				ReplaceReference(i+1,(ReferenceTarget*)GetStdMtl2Desc()->Create());
				GetCOREInterface()->AssignNewName(subMtl[i]);

//				pblock->SetValue(multi_ons,0,TRUE,i);
//				mapOn[i] = TRUE;
				}
//have to do this sepperate because setvalue causes an update and all the references are not in place yet
			macroRec->Disable();	// JBW 4/21/99, just record on count change
			pblock->SetCount(multi_ons,n);
			pblock->SetCount(multi_names,n);
			pblock->SetCount(multi_ids,n);
			macroRec->Enable();
			pblock->SetCount(multi_mtls,n);
			for (i=ct; i<subMtl.Count(); i++) {
				pblock->SetValue(multi_ons,0,TRUE,i);
				pblock->SetValue(multi_ids,0,i,i);  
//				mapOn[i] = TRUE;
				}

			}		
		else 
			{
			macroRec->Disable();	// JBW 4/21/99, just record on count change
			pblock->SetCount(multi_ons,n);
			pblock->SetCount(multi_names,n);
			pblock->SetCount(multi_ids,n);
			macroRec->Enable();
			pblock->SetCount(multi_mtls,n);

			}

		ClampOffset();
		NotifyChanged();
		if (paramDlg&&!paramDlg->isActive) {
			paramDlg->ReloadDialog();
			paramDlg->UpdateMtlDisplay();	  // DS 9/2/99
			}
		}
	}
 
*/
