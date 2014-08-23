#ifndef PH_DEBUG_H
#define PH_DEBUG_H
#ifdef DEBUG
struct dContact;
#include "../xrEngine/StatGraph.h"
#define DRAW_CONTACTS



extern	Flags32			ph_dbg_draw_mask						;
extern	Flags32			ph_dbg_draw_mask1						;
extern	bool			draw_frame								;
extern	u32				dbg_tries_num							;
extern	u32				dbg_saved_tries_for_active_objects		;
extern	u32				dbg_total_saved_tries					;
extern	u32 			dbg_reused_queries_per_step				;
extern	u32 			dbg_new_queries_per_step				;
extern	u32 			dbg_bodies_num							;
extern	u32 			dbg_joints_num							;
extern	u32 			dbg_islands_num							;
extern	u32 			dbg_contacts_num						;
extern	float			dbg_vel_collid_damage_to_display		;
extern	LPCSTR			dbg_trace_object						;
class	CObject													;
extern	CObject			*trace_object							;
#ifdef DRAW_CONTACTS


//DEFINE_VECTOR(SPHContactDBGDraw,CONTACT_VECTOR,CONTACT_I);
//extern CONTACT_VECTOR Contacts0;
//extern CONTACT_VECTOR Contacts1;
#endif
///ph_dbg_draw_mask 
enum
{
	phDbgDrawContacts			=		1<<0,
	phDbgDrawEnabledAABBS		=		1<<1,
	phDBgDrawIntersectedTries	=		1<<2,
	phDbgDrawSavedTries			=		1<<3,
	phDbgDrawTriTrace			=		1<<4,
	phDBgDrawNegativeTries		=		1<<5,
	phDBgDrawPositiveTries		=		1<<6,
	phDbgDrawTriTestAABB		=		1<<7,
	phDBgDrawTriesChangesSign	=		1<<8,
	phDbgDrawTriPoint			=		1<<9,
	phDbgDrawExplosionPos		=		1<<10,
	phDbgDrawObjectStatistics	=		1<<11,
	phDbgDrawMassCenters		=		1<<12,
	phDbgDrawDeathActivationBox =		1<<14,
	phHitApplicationPoints		=		1<<15,
	phDbgDrawCashedTriesStat	=		1<<16,
	phDbgDrawCarDynamics		=		1<<17,
	phDbgDrawCarPlots			=		1<<18,
	phDbgLadder					=		1<<19,
	phDbgDrawExplosions			=		1<<20,
	phDbgDrawCarAllTrnsm		=		1<<21,
	phDbgDrawZDisable			=		1<<22,
	phDbgAlwaysUseAiPhMove		=		1<<23,
	phDbgNeverUseAiPhMove		=		1<<24,
	phDbgDispObjCollisionDammage=		1<<25,
	phDbgIK						=		1<<26,
	phDbgDrawIKGoal				=		1<<27,
	phDbgIKLimits				=		1<<28,
	phDbgCharacterControl		=		1<<29,
	phDbgDrawRayMotions			=		1<<30,
	phDbgTrackObject			=		1<<31

};
///ph_dbg_draw_mask1 ne pereputat by blin!
enum
{
	ph_m1_DbgTrackObject		=		1<<0,
	ph_m1_DbgActorRestriction	=		1<<1,
	phDbgIKOff					=		1<<2,
	phDbgHitAnims				=		1<<3,
	phDbgDrawIKLimits			=		1<<4,
	phDbgDrawIKPredict			=		1<<5,
	phDbgDrawIKSHiftObject		=		1<<6,
	phDbgDrawIKCollision		=		1<<7,
	phDbgDrawIKBlending			=		1<<8
};

enum 
{
	dbg_track_obj_blends_bp_0			= 1<< 0,
	dbg_track_obj_blends_bp_1			= 1<< 1,
	dbg_track_obj_blends_bp_2			= 1<< 2,
	dbg_track_obj_blends_bp_3			= 1<< 3,
	dbg_track_obj_blends_motion_name	= 1<< 4,
	dbg_track_obj_blends_time			= 1<< 5,
	dbg_track_obj_blends_ammount		= 1<< 6,
	dbg_track_obj_blends_mix_params		= 1<< 7,
	dbg_track_obj_blends_flags			= 1<< 8,
	dbg_track_obj_blends_state			= 1<< 9,
	dbg_track_obj_blends_dump			= 1<< 10
};
//DEFINE_VECTOR( SPHObjDBGDraw, PHOBJ_DBG_V, PHOBJ_DBG_I );
//extern PHOBJ_DBG_V	dbg_draw_objects0;
//extern PHOBJ_DBG_V	dbg_draw_objects1;
class CPHObject;


struct SPHDBGDrawAbsract
{
	virtual void				render				( )						=0;
	virtual						~SPHDBGDrawAbsract	( )						{ };
};
DEFINE_VECTOR( SPHDBGDrawAbsract*, PHABS_DBG_V, PHABS_DBG_I )					;
extern PHABS_DBG_V	dbg_draw_abstruct0;
extern PHABS_DBG_V	dbg_draw_abstruct1;
void DBG_DrawStatBeforeFrameStep( );
void DBG_DrawStatAfterFrameStep( );
void DBG_RenderUpdate( ); 
void DBG_OpenCashedDraw( );
void DBG_ClosedCashedDraw( u32 remove_time );
void DBG_DrawPHAbstruct( SPHDBGDrawAbsract* a );
void DBG_DrawPHObject( const CPHObject *obj );
void DBG_DrawContact ( const dContact &c );
void DBG_DrawTri( CDB::RESULT *T, u32 c );
void DBG_DrawTri(CDB::TRI *T, const Fvector *V_verts, u32 c );
void DBG_DrawLine( const Fvector &p0, const Fvector &p1, u32 c );
void DBG_DrawAABB( const Fvector &center, const Fvector& AABB, u32 c );
void DBG_DrawOBB( const Fmatrix &m, const Fvector h, u32 c );
void DBG_DrawPoint( const Fvector& p, float size, u32 c );
void DBG_DrawMatrix( const Fmatrix &m, float size, u8 a=255 );
void DBG_DrawRotationX( const Fmatrix &m, float ang0, float ang1, float size, u32 ac, bool solid = false, u32 tessel = 7 );
void DBG_DrawRotationY( const Fmatrix &m, float ang0, float ang1, float size, u32 ac, bool solid = false, u32 tessel = 7 );
void DBG_DrawRotationZ( const Fmatrix &m, float ang0, float ang1, float size, u32 ac, bool solid = false, u32 tessel = 7 );
void _cdecl DBG_OutText( LPCSTR s,... );
void DBG_TextOutSet( float x, float y );
void DBG_TextSetColor( u32 color );
void DBG_DrawBind( CObject &O );
void DBG_PhysBones( CObject &O );
void DBG_DrawBones( CObject &O );
void DBG_DrawFrameStart( );
void PH_DBG_Render( );
void PH_DBG_Clear( );
LPCSTR PH_DBG_ObjectTrackName( );
////////////////////////////////////////////////
std::string dump_string( LPCSTR name, const Fvector &v );
std::string dump_string( LPCSTR name, const Fmatrix &form );
void dump( LPCSTR name, const Fmatrix &form );
void dump( LPCSTR name, const Fvector &v );

/////////////////////////////////////////////////
void DBG_PH_NetRelcase( CObject* obj );


struct CFunctionGraph
{
public:
	typedef fastdelegate::FastDelegate1<float,float> type_function;
private:
	CStatGraph						*m_stat_graph																																					;
	type_function					m_function																																						;
	float x_min,x_max,s;
	//float y_min,y_max;
	//Fvector2 left_bottom;
	//Fvector2 range;
public:

	CFunctionGraph						( )																																								;
	~CFunctionGraph						( )																																								;
	void	Init						( type_function fun, float x0, float x1, int l, int t, int w, int h, int points_num=500, u32 color=D3DCOLOR_XRGB( 0, 255, 0 ), u32 bk_color=D3DCOLOR_XRGB( 255, 255, 255 ) )	;
	void	Clear						( )																																								;
	bool	IsActive					( )																																								;
	void	AddMarker					( CStatGraph::EStyle Style, float pos, u32 Color )																												;
	void	UpdateMarker				( u32 ID, float M1 )																																				;
IC	float	ScaleX						( float x )																												{ VERIFY( IsActive( ) ); return( x-x_min )/s; }	
	void	ScaleMarkerPos				( u32 ID, float &p )																																				;
	void	ScaleMarkerPos				( CStatGraph::EStyle Style, float &p )																															;
IC	float	ResolutionX					( ){ VERIFY( IsActive( ) ); return s; } 
};
#endif
#endif