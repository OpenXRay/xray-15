//-----------------------------------------------------------------------------
// ----------------------
// File ....: FilmGrain.h
// ----------------------
// Author...: Gus J Grubba
// Date ....: October 1998
// Descr....: Film Grain Render Effect
//
// History .: Oct, 27 1998 - Ported
//            
//-----------------------------------------------------------------------------
        
#ifndef _FGCLASS_
#define _FGCLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Forward References

#include "resource.h"

TCHAR *GetString(int id);
Class_ID fgClassID(470000003,0);

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class FilmGrain: public Effect {

		//-- Parameters

		IParamBlock2*	pblock;
						
	public:
     
		//-- Constructors/Destructors
        
						FilmGrain				( );
						~FilmGrain				( )	{};
               
		//-- Animatable/Reference

		int				NumSubs					( )	{ return 1; }
		Animatable*		SubAnim					( int i );
		TSTR			SubAnimName				( int i );
		int				NumRefs					( )	{ return 1; }
		RefTargetHandle	GetReference			( int i );
		void			SetReference			( int i, RefTargetHandle rtarg );
		
		Class_ID		ClassID					( )	{ return fgClassID; }
		void			GetClassName			( TSTR& s ) { s = GetString(IDS_FILM_GRAIN_EFFECT); }
		void			DeleteThis				( )	{ delete this; }
		RefResult		NotifyRefChanged		( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		int				NumParamBlocks			( ) { return 1; }
		IParamBlock2*	GetParamBlock			( int i ) { return pblock; }
		IParamBlock2*	GetParamBlockByID		( BlockID id ) { return (pblock->ID() == id) ? pblock : NULL; }

		IOResult		Load					( ILoad *iload );

		//-- Effectc

		TSTR			GetName					( ) { return GetString(IDS_FILM_GRAIN_EFFECT); }
		EffectParamDlg*	CreateParamDialog		( IRendParams *ip );
		DWORD			GBufferChannelsRequired	( TimeValue t ) { return BMM_CHAN_Z; }
		int				RenderBegin				( TimeValue t, ULONG flags );
		int				RenderEnd				( TimeValue t );
		void			Update					( TimeValue t, Interval& valid );
		void			Apply					( TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort );

		int				NumGizmos				( )					{ return 0; }
		INode*			GetGizmo				( int i )			{ return NULL; }
		void			DeleteGizmo				( int i )			{ ; }
		void			AppendGizmo				( INode *node )		{ ;	}
		BOOL			OKGizmo					( INode *node )		{ return FALSE; }
		void			EditGizmo				( INode *node )		{ ; }

};

#endif

//-- EOF: FilmGrain.h ---------------------------------------------------------
