#pragma once

class ENGINE_API CGameFont;

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/ApplicationRender.h"

class ENGINE_API CApplication :
    public pureFrame,
    public IEventReceiver
{
private:
    friend class dxApplicationRender;
    struct sLevelInfo
    {
        char* folder;
        char* name;
    };

    string256 app_title;
    FactoryPtr<IApplicationRender> m_pRender;
    int max_load_stage;
    int load_stage;
    u32 ll_dwReference;
    EVENT eQuit;
    EVENT eStart;
    EVENT eStartLoad;
    EVENT eDisconnect;
    EVENT eConsole;

public:
    CGameFont* pFontSystem;
    xr_vector<sLevelInfo> Levels;
    u32 Level_Current;

public:
    CApplication();
    ~CApplication();
    
    void Level_Scan();
    int Level_ID(LPCSTR name, LPCSTR ver, bool bSet);
    void Level_Set(u32 ID);
    void LoadAllArchives();
    CInifile* GetArchiveHeader(LPCSTR name, LPCSTR ver);
    void LoadBegin();
    void LoadEnd();
    void LoadTitleInt(LPCSTR str);
    void LoadSwitch();
    void LoadDraw();
    // IEventReceiver
    virtual	void OnEvent(EVENT E, u64 P1, u64 P2) override;
    // ~IEventReceiver
    // pureFrame
    virtual void OnFrame() override;
    // ~pureFrame
    void load_draw_internal();
    void destroy_loading_shaders();

private:
    void Level_Append(LPCSTR lname);
};

extern ENGINE_API CApplication*	pApp;
