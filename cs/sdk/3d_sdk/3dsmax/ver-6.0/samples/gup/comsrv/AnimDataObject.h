// AnimDataObject.h: Definition of the CAnimDataObject class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMDATAOBJECT_H__A4A0A2C0_70C4_49CA_AB33_DC398C6AAB63__INCLUDED_)
#define AFX_ANIMDATAOBJECT_H__A4A0A2C0_70C4_49CA_AB33_DC398C6AAB63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "resource.h"       // main symbols
#include <max.h>
#include <stack>

class AnimEnumerator;
/////////////////////////////////////////////////////////////////////////////
// CAnimDataObject
// A wrapper around an animatable which implements IDataObject
class CAnimDataObject : 
	public CComObjectRoot,
	public CComCoClass<CAnimDataObject,&CLSID_AnimDataObject>,
	public IDataObject,
	public ReferenceMaker
{
public:
	CAnimDataObject() {m_dirty = 1;}
	HRESULT FinalConstruct();
	void FinalRelease();
	
	HRESULT ResetStm(IStream* pStream);
BEGIN_COM_MAP(CAnimDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()


DECLARE_REGISTRY_RESOURCEID(IDR_AnimDataObject)

// IDataObject
public:
	STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
	STDMETHOD(EnumFormatEtc)(unsigned long,struct IEnumFORMATETC ** );
	STDMETHOD(GetDataHere)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */)
	{
		ATLTRACENOTIMPL(_T("CAnimDataObject::GetDataHere"));
	}
	STDMETHOD(QueryGetData)(FORMATETC* /* pformatetc */)
	{
		ATLTRACENOTIMPL(_T("CAnimDataObject::QueryGetData"));
	}
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */)
	{
		ATLTRACENOTIMPL(_T("CAnimDataObject::GetCanonicalFormatEtc"));
	}
	STDMETHOD(SetData)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */, BOOL /* fRelease */)
	{
		ATLTRACENOTIMPL(_T("CAnimDataObject::SetData"));
	}

	STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
		DWORD *pdwConnection);

	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

	
	// RefenceMaker

	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);
	

	int NumRefs(){return 1;}

	RefTargetHandle GetReference(int i){return m_pAnimRef;}

	void SetReference(int i, RefTargetHandle rtarg);

	//Data Mgmt
	
	int LocalEnumAnimTree(Animatable *anim, AnimEnumerator *animEnum, Animatable *client, int subNum);

	int UpdateData();

	void DataChange();

private:
	CComPtr<IXMLUtil> m_spXMLUtil;

	HANDLE m_hEnumThread;
	long m_dirty;
	void SetDirty();
	CComPtr<IDataAdviseHolder> m_spDataAdviseHolder;
//	vector<FORMATETC> m_cfArray;     // Array of supported clipboard formats
	CComPtr<IXMLDOMDocument> m_spXMLDoc;
	HANDLE m_hDocMutex;
	ReferenceTarget *m_pAnimRef;
};

class ClearAnimFlags : public AnimEnum
{
public:
	virtual int proc(Animatable *anim, Animatable *client, int subNum)
	{
		anim->ClearAFlag(A_WORK4);
		return ANIM_ENUM_PROCEED;
	}
};


class AnimEnumerator
{
public:
	AnimEnumerator(IXMLDOMDocument* pElem);
	virtual int proc(Animatable *anim, Animatable *client, int subNum);
	virtual void DecDepth();
	virtual void IncDepth(){++depth;}
	int Depth() { return depth; }
	void SetScope(int s){scope = s;}
	int Scope(){return scope;}
protected:
	void PushNodeContext(IXMLDOMNode *pNode);
	void PopNodeContext();
	IXMLDOMNode* Context();//no addref'd 
	HRESULT CreateDOMAnimNode(IXMLDOMNode *parent, Animatable *anim, int subidx, IXMLDOMNode **newnode, Animatable *client);
	HRESULT CreateDOMAnimReference(IXMLDOMNode* Maker, IXMLDOMNode* Target);
	int depth;
	int scope;  
private:
	std::stack<IXMLDOMNode *> m_NodeStack;
	CComPtr<IXMLDOMElement> m_spElem;
	CComPtr<IXMLDOMDocument> m_spDoc;
	DWORD m_dwElementCount;
};
#endif // !defined(AFX_ANIMDATAOBJECT_H__A4A0A2C0_70C4_49CA_AB33_DC398C6AAB63__INCLUDED_)
