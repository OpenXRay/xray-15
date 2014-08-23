/**********************************************************************
 *<
	FILE: VertexPaint.h

	DESCRIPTION: Modifier definition

	CREATED BY:  Christer Janson, Nikolai Sander

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __VERTEXPAINT__H
#define __VERTEXPAINT__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "modstack.h"


#define VERTEXPAINT_CLASS_ID	Class_ID(0x663b6fab, 0x73f47385)
#define CID_PAINT				(CID_USER+0x439c)
#define NUMPALETTES				16
#define WM_POSTINIT				(WM_USER+0x1177)

extern TCHAR *GetString(int id);
extern ClassDesc* GetVertexPaintDesc();
extern HINSTANCE hInstance;

class NVert
{
public:
	NVert();
	NVert& operator= (NVert &nvert);
	Tab<int> faces;
	Tab<int> whichVertex;
};

class ColorData {
public:
	ColorData(DWORD col);
	ColorData();

	COLORREF color;
	float	 bary;
	DWORD    fi;
	int		 vi;
};

class AlphaData {
public:
	AlphaData(float alph);
	AlphaData();

	float alpha;
	float	 bary;
	DWORD    fi;
	int		 vi;
};

class VertexPaintData : public LocalModData {
	friend class VertexPaintColorRestore;
	friend class VertexPaintAlphaRestore;
public:
	VertexPaintData(Mesh& m);
	VertexPaintData(MNMesh& m);
	VertexPaintData();
	~VertexPaintData();
	LocalModData*	Clone();

	Mesh*	GetMesh();
	MNMesh*	GetMNMesh();
	NVert&  GetNVert(int i);
	NVert&  GetNVCVert(int i, int chan);
	void	SetCache(Mesh& m);
	void	SetCache(MNMesh& m);
	void	FreeCache();

	COLORREF&	GetColor (int i, int chan);
	float GetAlpha (int i);
	ColorData&  GetColorData (int i, int chan);
	AlphaData & GetAlphaData (int i);
	void		SetColor (int i,float bary,DWORD fi, int vi, COLORREF c, int chan);
	void		SetAlpha (int i,float bary,DWORD fi, int vi, float alph);
	int			GetNumColors(int chan);
	int			GetMaxNumColors(int chan);
	void		AllocColorData(int numcols, int channel);
	void		SynchVerts(Mesh &m);
	void		SynchVerts(MNMesh &m);
	bool ChannelActive (int chan) const { return channelActive[chan]; }
	void SetChannelActive (int chan, bool active=true) { channelActive[chan] = active; }

private:
	Mesh*	mesh;
	MNMesh*	mpMNMesh;
	ColorData* colordata, *illumdata;
	AlphaData* alphadata;
	bool channelActive[3];
	int		maxNumColors[3];
	int		numColors[3];
	NVert    *nverts;
	int      numnverts;
	NVert    *nvcverts[3];
	int      numnvcverts[3];
};

class VertexPaint : public Modifier {
	
	friend class PaintMouseProc;
	friend INT_PTR CALLBACK VertexPaintDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:

		static IObjParam*		ip;
		static VertexPaint*		editMod;
		static HWND				hParams;
		static ICustButton*		iPaintButton;
		static ICustButton*		iPickButton;
		static IColorSwatch*	iColor;
		static COLORREF			lastColor;
		static COLORREF			palColors[NUMPALETTES];
		bool affectChannel[3];
		float alpha;

		//Constructor/Destructor
		VertexPaint();
		~VertexPaint();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(GetString(IDS_CLASS_NAME)); }  
		virtual Class_ID ClassID() { return VERTEXPAINT_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
		IOResult SaveLocalData(ISave *isave, LocalModData *ld);

		//From Modifier
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL|VERTCOLOR_CHANNEL|PART_SUBSEL_TYPE|SELECT_CHANNEL|PART_DISPLAY|TEXMAP_CHANNEL; }
		ChannelMask ChannelsChanged() { return VERTCOLOR_CHANNEL|TOPO_CHANNEL|GEOM_CHANNEL; }
		void ModifyTriObject(TriObject* tobj, TimeValue t, ModContext &mc);
		void ModifyPolyObject(PolyObject* pPolyObj, TimeValue t, ModContext &mc);
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Class_ID InputType() {return mapObjectClassID;}
		Interval LocalValidity(TimeValue t);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);


		// From BaseObject
		BOOL ChangeTopology() {return FALSE;}
		BOOL DependOnTopology(ModContext &mc);

		int GetParamBlockIndex(int id) {return id;}

		//From ReferenceMaker
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		
		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		BOOL	ActivatePaint(BOOL bOnOff, BOOL bPick = FALSE);
		void	EnterMode();
		void	ExitMode();
		BOOL	IsValidNode(INode* node);
		ModContext* ModContextFromNode(INode* node);
		COLORREF GetActiveColor();
		void	TurnVCOn(BOOL shaded);
		void SetActiveAlpha (float alphaValue);



		void	PaletteButton(HWND hWnd);
		void	InitPalettes();
		void	SavePalettes();
	
	private :

		void SetFlag (DWORD fl) { flags |= fl; }
		void ClearFlag (DWORD fl) { flags &= ~fl; }
		void SetFlag (DWORD fl, bool set) { if (set) SetFlag (fl); else ClearFlag (fl); }
		bool GetFlag (DWORD fl) { return (fl&flags) ? TRUE : FALSE; }

	private:
		ModContextList	modContexts;
		INodeTab		nodeTab;
		HWND			hPaletteWnd[NUMPALETTES];
		float			fTint;
		ISpinnerControl *iTint;
		DWORD flags;

	};

/****************************************************************************
 *
 * 3D Paint Implementation
 *
 ***************************************************************************/

// Mouse proc
class PaintMouseProc : public MouseCallBack {
public:
	int		proc(HWND hWnd, int msg, int point, int flags, IPoint2 m); // Mouse callback
	void	SetModifier(VertexPaint* pMod) { pModifier = pMod; }
	void	SetPickMode(BOOL bPick){bPickMode = bPick;}
	BOOL	GetPickMode(){return bPickMode;}
	void	DoPainting(HWND hWnd, IPoint2 m);
	void	DoPickColor(HWND hWnd, IPoint2 m);
	void	MaybeStartHold();
	void	MaybeEndHold();
	BOOL	IsValidPickNode(INode *node);


private:
	VertexPaint*	pModifier;
	BOOL			wasHolding;
	BOOL			bPickMode;
};


class PaintCommandMode : public CommandMode {
public:
	PaintCommandMode();

	PaintMouseProc mouseProc;

	int							Class();
	virtual int					ID();
	MouseCallBack*				MouseProc(int *numPoints);
	BOOL						ChangeFG(CommandMode *oldMode);
	ChangeForegroundCallback*	ChangeFGProc();
	void						EnterMode();
	void						ExitMode();

	void						SetInterface(Interface* ip) { iInterface = ip; }
	void						SetModifier(VertexPaint* pMod)		{ pModifier = pMod; mouseProc.SetModifier(pMod); }

private:
	Interface*					iInterface;
	VertexPaint*				pModifier;
};

class VertexPaintColorRestore : public RestoreObj {
private:
	VertexPaintData *pPaintData;
	VertexPaint *pMod;

	ColorData *colordata;
	ColorData *redoColordata;
	int numcolors;
	int maxnumcolors;
	int redonumcolors;
	int redomaxnumcolors;
	int channel;	// 0 = vertex colors, 1 = vertex illumination.
public:	
	VertexPaintColorRestore(VertexPaintData *pLocalData, VertexPaint *pVPaint, int chan);
	~VertexPaintColorRestore();
	void Restore(int isUndo);
	void Redo();
	int  Size();
};

class VertexPaintAlphaRestore : public RestoreObj
{
private:
	VertexPaintData *pPaintData;
	VertexPaint *pMod;

	AlphaData *alphadata;
	AlphaData *redoAlphadata;
	int numcolors;
	int maxnumcolors;
	int redonumcolors;
	int redomaxnumcolors;

public:	
	VertexPaintAlphaRestore(VertexPaintData *pLocalData, VertexPaint *pVPaint);
	~VertexPaintAlphaRestore();
	void Restore(int isUndo);
	void Redo();
	int  Size();
};


#endif // __VERTEXPAINT__H
