#include "stdafx.h"
#include "UIActorMenu.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIActorStateInfo.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIMessageBoxEx.h"
#include "UIPropertiesBox.h"
#include "UI3tButton.h"

#include "UIInventoryUpgradeWnd.h"
#include "UIInvUpgradeInfo.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "object_broker.h"
#include "UIWndCallback.h"
#include "UIHelper.h"


CUIActorMenu::CUIActorMenu()
{
	m_currMenuMode					= mmUndefined;
	m_trade_partner_inventory_state = 0;
	Construct						();
}

CUIActorMenu::~CUIActorMenu()
{
	xr_delete			(m_message_box_yes_no);
	xr_delete			(m_message_box_ok);
	xr_delete			(m_UIPropertiesBox);
	xr_delete			(m_hint_wnd);
	xr_delete			(m_ItemInfo);

	ClearAllLists		();
}

void CUIActorMenu::Construct()
{
	CUIXml								uiXml;
	uiXml.Load							(CONFIG_PATH, UI_PATH, "actor_menu.xml");

	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);
	m_hint_wnd = UIHelper::CreateHint	(uiXml, "hint_wnd");

	m_LeftBackground					= xr_new<CUIStatic>();
	m_LeftBackground->SetAutoDelete		(true);
	AttachChild							(m_LeftBackground);
	xml_init.InitStatic					(uiXml, "left_background", 0, m_LeftBackground);

	m_pUpgradeWnd						= xr_new<CUIInventoryUpgradeWnd>(); 
	AttachChild							(m_pUpgradeWnd);
	m_pUpgradeWnd->SetAutoDelete		(true);
	m_pUpgradeWnd->Init					();

	m_ActorCharacterInfo = xr_new<CUICharacterInfo>();
	m_ActorCharacterInfo->SetAutoDelete( true );
	AttachChild( m_ActorCharacterInfo );
	m_ActorCharacterInfo->InitCharacterInfo( &uiXml, "actor_ch_info" );

	m_PartnerCharacterInfo = xr_new<CUICharacterInfo>();
	m_PartnerCharacterInfo->SetAutoDelete( true );
	AttachChild( m_PartnerCharacterInfo );
	m_PartnerCharacterInfo->InitCharacterInfo( &uiXml, "partner_ch_info" );
	
	m_RightDelimiter		= UIHelper::CreateStatic(uiXml, "right_delimiter", this);
	m_ActorTradeCaption		= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_caption", m_RightDelimiter);
	m_ActorTradePrice		= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_price", m_RightDelimiter);
	m_ActorTradeWeightMax	= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_weight_max", m_RightDelimiter);
	m_ActorTradeCaption->AdjustWidthToText();
	
	m_LeftDelimiter			= UIHelper::CreateStatic(uiXml, "left_delimiter", this);
	m_PartnerTradeCaption	= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_caption", m_LeftDelimiter);
	m_PartnerTradePrice		= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_price", m_LeftDelimiter);
	m_PartnerTradeWeightMax	= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_weight_max", m_LeftDelimiter);
	m_PartnerTradeCaption->AdjustWidthToText();

	m_ActorBottomInfo	= UIHelper::CreateStatic(uiXml, "actor_weight_caption", this);
	m_ActorWeight		= UIHelper::CreateStatic(uiXml, "actor_weight", this);
	m_ActorWeightMax	= UIHelper::CreateStatic(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	m_PartnerBottomInfo	= UIHelper::CreateStatic(uiXml, "partner_weight_caption", this);
	m_PartnerWeight		= UIHelper::CreateStatic(uiXml, "partner_weight", this);
	m_PartnerBottomInfo->AdjustWidthToText();
	m_PartnerWeight_end_x = m_PartnerWeight->GetWndPos().x;

	m_pInventoryBagList			= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bag", this);
	m_pInventoryBeltList		= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_belt", this);
	m_pInventoryOutfitList		= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_outfit", this);
	m_pInventoryDetectorList	= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_detector", this);
	m_pInventoryPistolList		= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_pistol", this);
	m_pInventoryAutomaticList	= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_automatic", this);
	m_pTradeActorBagList		= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade_bag", this);
	m_pTradeActorList			= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade", this);
	m_pTradePartnerBagList		= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_bag", this);
	m_pTradePartnerList			= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_trade", this);

	m_belt_list_over[0] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
	Fvector2 pos;
	pos = m_belt_list_over[0]->GetWndPos();
	float dy = uiXml.ReadAttribFlt("belt_list_over", 0, "dy", 10.0f);
	for ( u8 i = 1; i < e_af_count; ++i )
	{
		pos.y += dy;
		m_belt_list_over[i] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
		m_belt_list_over[i]->SetWndPos( pos );
	}

	m_ActorMoney	= UIHelper::CreateStatic(uiXml, "actor_money_static", this);
	m_PartnerMoney	= UIHelper::CreateStatic(uiXml, "partner_money_static", this);
	
	m_trade_button		= UIHelper::Create3tButtonEx(uiXml, "trade_button", this);
	m_takeall_button	= UIHelper::Create3tButtonEx(uiXml, "takeall_button", this);
	m_exit_button		= UIHelper::Create3tButtonEx(uiXml, "exit_button", this);

	m_clock_value						= UIHelper::CreateStatic(uiXml, "clock_value", this);

	m_pDeadBodyBagList					= xr_new<CUIDragDropListEx>(); 
	AttachChild							(m_pDeadBodyBagList);
	m_pDeadBodyBagList->SetAutoDelete	(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_deadbody_bag", 0, m_pDeadBodyBagList);

	m_ActorStateInfo					= xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml		(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete		(true);
	AttachChild							(m_ActorStateInfo); 

	XML_NODE* stored_root				= uiXml.GetLocalRoot	();
	uiXml.SetLocalRoot					(uiXml.NavigateToNode	("action_sounds",0));
	::Sound->create						(sounds[eSndOpen],		uiXml.Read("snd_open",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eSndClose],		uiXml.Read("snd_close",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToSlot],	uiXml.Read("snd_item_to_slot",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToBelt],	uiXml.Read("snd_item_to_belt",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToRuck],	uiXml.Read("snd_item_to_ruck",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eProperties],	uiXml.Read("snd_properties",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eDropItem],		uiXml.Read("snd_drop_item",		0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eAttachAddon],	uiXml.Read("snd_attach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eDetachAddon],	uiXml.Read("snd_detach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemUse],		uiXml.Read("snd_item_use",		0,	NULL),st_Effect,sg_SourceType);
	uiXml.SetLocalRoot					(stored_root);

	m_ItemInfo							= xr_new<CUIItemInfo>();
//-	m_ItemInfo->SetAutoDelete			(true);
//-	AttachChild							(m_ItemInfo);
	m_ItemInfo->InitItemInfo			("actor_menu_item.xml");

	m_upgrade_info						= NULL;
	if ( ai().get_alife() )
	{
		m_upgrade_info						= xr_new<UIInvUpgradeInfo>();
		m_upgrade_info->SetAutoDelete		(true);
		AttachChild							(m_upgrade_info);
		m_upgrade_info->init_from_xml		("actor_menu_item.xml");
	}

	m_message_box_yes_no				= xr_new<CUIMessageBoxEx>();	
	m_message_box_yes_no->InitMessageBox( "message_box_yes_no" );
	m_message_box_yes_no->SetAutoDelete	(true);
	m_message_box_yes_no->SetText		( "" );

	m_message_box_ok					= xr_new<CUIMessageBoxEx>();	
	m_message_box_ok->InitMessageBox	( "message_box_ok" );
	m_message_box_ok->SetAutoDelete		(true);
	m_message_box_ok->SetText			( "" );

	m_UIPropertiesBox					= xr_new<CUIPropertiesBox>();
	AttachChild							(m_UIPropertiesBox);
	m_UIPropertiesBox->InitPropertiesBox(Fvector2().set(0,0),Fvector2().set(300,300));
	m_UIPropertiesBox->Hide				();
	m_UIPropertiesBox->SetWindowName	( "property_box" );

	InitCallbacks						();

	BindDragDropListEvents				(m_pInventoryBeltList);		
	BindDragDropListEvents				(m_pInventoryPistolList);		
	BindDragDropListEvents				(m_pInventoryAutomaticList);	
	BindDragDropListEvents				(m_pInventoryOutfitList);	
	BindDragDropListEvents				(m_pInventoryDetectorList);	
	BindDragDropListEvents				(m_pInventoryBagList);
	BindDragDropListEvents				(m_pTradeActorBagList);
	BindDragDropListEvents				(m_pTradeActorList);
	BindDragDropListEvents				(m_pTradePartnerBagList);
	BindDragDropListEvents				(m_pTradePartnerList);
	BindDragDropListEvents				(m_pDeadBodyBagList);

	m_allowed_drops[iActorSlot].push_back(iActorBag);
	m_allowed_drops[iActorSlot].push_back(iActorTrade);
	m_allowed_drops[iActorSlot].push_back(iDeadBodyBag);

	m_allowed_drops[iActorBag].push_back(iActorSlot);
	m_allowed_drops[iActorBag].push_back(iActorBelt);
	m_allowed_drops[iActorBag].push_back(iActorTrade);
	m_allowed_drops[iActorBag].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBag].push_back(iActorBag);
	
	m_allowed_drops[iActorBelt].push_back(iActorBag);
	m_allowed_drops[iActorBelt].push_back(iActorTrade);
	m_allowed_drops[iActorBelt].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBelt].push_back(iActorBelt);

	m_allowed_drops[iActorTrade].push_back(iActorSlot);
	m_allowed_drops[iActorTrade].push_back(iActorBag);
	m_allowed_drops[iActorTrade].push_back(iActorBelt);
	m_allowed_drops[iActorTrade].push_back(iActorTrade);

	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTrade);
	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTrade);

	m_allowed_drops[iDeadBodyBag].push_back(iActorSlot);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBag);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBelt);
	m_allowed_drops[iDeadBodyBag].push_back(iDeadBodyBag);

	m_upgrade_selected					= NULL;
	SetCurrentItem						(NULL);
	SetActor							(NULL);
	SetPartner							(NULL);
	SetInvBox							(NULL);

	m_actor_trade						= NULL;
	m_partner_trade						= NULL;
	m_repair_mode						= false;

	DeInitInventoryMode					();
	DeInitTradeMode						();
	DeInitUpgradeMode					();
	DeInitDeadBodySearchMode			();
}

void CUIActorMenu::BindDragDropListEvents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemRButtonClick);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemFocusReceive);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemFocusLost);
	lst->m_f_item_focused_update	= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUIActorMenu::OnItemFocusedUpdate);
}

void CUIActorMenu::InitCallbacks()
{
	Register( m_trade_button );
	Register( m_takeall_button );
	Register( m_exit_button );
	Register( m_UIPropertiesBox );
	VERIFY( m_pUpgradeWnd );
	Register( m_pUpgradeWnd->m_btn_repair );

	AddCallback( m_trade_button->WindowName(),    BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::OnBtnPerformTrade ) );
	AddCallback( m_takeall_button->WindowName(),  BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::TakeAllFromPartner ) );
	AddCallback( m_exit_button->WindowName(),     BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::OnBtnExitClicked ) );
	AddCallback( m_UIPropertiesBox->WindowName(), PROPERTY_CLICKED, CUIWndCallback::void_function( this, &CUIActorMenu::ProcessPropertiesBoxClicked ) );
	AddCallback( m_pUpgradeWnd->m_btn_repair->WindowName(), BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::TryRepairItem ) );
}

void CUIActorMenu::UpdateButtonsLayout()
{
	Fvector2 btn_exit_pos;
	if(m_trade_button->IsShown() || m_takeall_button->IsShown())
	{
		btn_exit_pos	= m_trade_button->GetWndPos();
		btn_exit_pos.x	+=m_trade_button->GetWndSize().x;
	}else
	{
		btn_exit_pos	= m_trade_button->GetWndPos();
		btn_exit_pos.x	+=m_trade_button->GetWndSize().x/2.0f;
	}
	
	m_exit_button->SetWndPos(btn_exit_pos);
}

void CUIActorMenu::SetSimpleHintText(LPCSTR text)
{
//m_hint_wnd;
}
