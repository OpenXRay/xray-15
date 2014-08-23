// Copyright (C) 2000 by Autodesk, Inc.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
///////////////////////////////////////////////////////////////////////////////
// XREFUTIL.H
//
// DESCR:
//     
//
// CHANGE LOG:
//     03/2000 : DY : Created
//		7 Nov 2000: rudy cazabon : extending the sample to support IObjXRefManager 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __XREFUTIL__H
#define __XREFUTIL__H

///////////////////////////////////////
// includes
///////////////////////////////////////

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "IXref.h"

#include "utilapi.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

// TODO: Change ClassID if re-used
#define XREFUTIL_CLASS_ID   Class_ID(0x1b3e9d52, 0x582bf699)


///////////////////////////////////////
// Xrefutil class declaration -- our main utility plugin class
///////////////////////////////////////

class Xrefutil : public UtilityObj 
{

public:

    IUtil * m_pUtil;
    Interface * m_pInterface;
    HWND m_hPanel;  // Windows handle of our UI

    // some "cache" member vars for the xref obj dialog
    Tab<TSTR *> m_objnamesholder;
    TSTR m_picknameholder;
    bool m_proxyholder;
    bool m_ignoreanimholder;

// Adding support for IObjXRefManager -- 7 Nov 2000 rudy cazabon
// Just a pointer to the interface class IObjXRefManager.

	IObjXRefManager * m_pIobjrefmgr;

     
    // Constructor/Destructor
    Xrefutil();
    ~Xrefutil();        

    // from UtilityObj
    void BeginEditParams(Interface *ip,IUtil *iu);
    void EndEditParams(Interface *ip,IUtil *iu);
    void DeleteThis();

    // internal (public) utility methods
    void Init(HWND hWnd);
    void Destroy(HWND hWnd);


    bool DoOpenSaveDialog(TSTR &fileName, bool bOpen = false);
    bool DoPickObjDialog();

    // xref utility methods

    // (xref scene methods)

    void AddNewXrefScene(HWND hWnd);
    void ConvertSelectedToXrefScene(HWND hWnd);
    void RefreshAllXrefScenes(HWND hWnd);
    void MergeAllXrefScenes(HWND hWnd);

    // (xref object methods)

    void NodeToXref(INode * pNode, TSTR &filename, bool bProxy, bool bIgnoreAnim = false);
    void DeleteAllAnimation(ReferenceTarget *ref);
    void AddNewXrefObject(HWND hWnd);
    void ConvertSelectedToXrefObject(HWND hWnd);
    void ExportXrefObjects(HWND hWnd);

	// IObjXRefManager methods -- 7 Nov 2000 rudy cazabon
	
	void AddNewXrefScene_IXROMGR(HWND hWnd);  // wrapper around GetXRefObject(TCHAR *fname, int i)
	void RefreshXRefObject_IXROMGR(HWND hWnd);  // wrapper around ReloadFile(TCHAR *fname)
	void GetAllXRefsObjects_IXROMGR(HWND hWnd);  // wrapper around GetAllXRefObjects(Tab<IXRefObject*> &objs)
};

static Xrefutil theXrefutil;


///////////////////////////////////////
// Handy enumerator base classes 
///////////////////////////////////////

class NodeEnumerator 
{
protected:
    bool m_continue;

public:

    NodeEnumerator() : m_continue(true) {};
    virtual ~NodeEnumerator() {};

    void reset() { m_continue = true; };

    virtual bool Proc(INode * pNode) = 0; // return true if should continue processing

    virtual void Enumerate(INode * pNode, bool procfirst = false)
    {
        if (procfirst) {
            m_continue = Proc(pNode);
        }
        for (int c = 0; c < pNode->NumberOfChildren(); c++) {
            if (m_continue) Enumerate(pNode->GetChildNode(c));
        }
        if (!procfirst && m_continue) {
            m_continue = Proc(pNode);
        }                   
    };
};


class NodeFlagger : public NodeEnumerator
{
protected:
    int m_flag;
    bool m_clear;

public:

    NodeFlagger() : m_flag(0), m_clear(false) {};
    NodeFlagger(int flag) { m_flag = flag; m_clear = false; };

    virtual void set_flag(int flag) { m_flag = flag; }; 
    virtual void set_clear(bool clear) { m_clear = clear; };

    virtual bool Proc(INode * pNode) 
    { m_clear ? pNode->ClearAFlag(m_flag) : pNode->SetAFlag(m_flag); return true; };

};




#endif // __XREFUTIL__H
