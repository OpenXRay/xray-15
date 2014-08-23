// AnimDataObject.cpp : Implementation of CComsrvApp and DLL registration.

#include "stdafx.h"

#include "comsrv.h"

#include "AnimDataObject.h"
//#include <vector>
//using namespace std;
/////////////////////////////////////////////////////////////////////////////
//
static ClearAnimFlags theClearAnimFlags;

//const TCHAR pszXMLDocumentProgID[] = _T("MSXML2.DomDocument.3.0");
//const TCHAR pszTestFile[]      = "http://localhost/xml/sampleVIZML.xml";
//const TCHAR pszTestFile[]      = "http://localhost/xml/min2.xml";

HRESULT CAnimDataObject::ResetStm(IStream* pStream)
{
	if(!pStream)
		return E_POINTER;

	LARGE_INTEGER nPos;
	ZeroMemory(&nPos, sizeof(nPos));
	return pStream->Seek(nPos, STREAM_SEEK_SET, NULL);
}

HRESULT CAnimDataObject::FinalConstruct()
{
	USES_CONVERSION;
	/*
    static FORMATETC cfArray[] = { 
		{ CF_TEXT,      NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM }};

	// Set up the predefined clipboard formats
	for(int i = 0; i < sizeof(cfArray)/sizeof(cfArray[0]); i++)
		m_cfArray.push_back(cfArray[i]);
		*/
	
	m_spXMLUtil.CoCreateInstance(__uuidof(XMLUtil), NULL, CLSCTX_INPROC_SERVER);
	ATLASSERT(m_spXMLUtil);

//	CComBSTR testdata(pszTestFile);
//	HRESULT hr = m_spXMLUtil->CreateDocument(&m_spXMLDoc, CComVariant(testdata), 1);
	HRESULT hr = m_spXMLUtil->CreateDocument(&m_spXMLDoc, CComVariant(0), 1);
	ATLASSERT(m_spXMLDoc);
#ifdef _DEBUG
	m_spXMLUtil->WalkTree(m_spXMLDoc, 0, L"CAnimDataObject::FinalConstruct");
#endif

	m_hDocMutex = CreateMutex(NULL, false, NULL);
	ATLASSERT(m_hDocMutex);

	hr = CreateDataAdviseHolder(&m_spDataAdviseHolder.p);
	return hr;

}

void CAnimDataObject::FinalRelease()
{
	CloseHandle(m_hDocMutex);
	m_hDocMutex = NULL;
}



HRESULT STDMETHODCALLTYPE CAnimDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
	if(!pformatetcIn || !pmedium)
		return E_POINTER;
	if(pformatetcIn->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	
	// Output some debugging info...
	#ifdef _DEBUG
	TCHAR szClip[MAX_PATH];
	ZeroMemory(szClip, sizeof(szClip));
	GetClipboardFormatName(pformatetcIn->cfFormat, szClip, MAX_PATH-1);
//	ATLTRACE("CAnimDataObject::GetData(%s)\n", szClip);
	#endif
	
	HRESULT hr;
	
	if(pformatetcIn->cfFormat == CF_TEXT )
	{
		// Directory data as a stream
		if((pformatetcIn->tymed & TYMED_ISTREAM) == 0)
			return DV_E_TYMED;
		
		CComPtr<IStream> spXMLStream;
		hr = CreateStreamOnHGlobal(NULL, TRUE, &spXMLStream);

		UpdateData();

		DWORD dwWaitResult = WaitForSingleObject(m_hDocMutex, INFINITE);
		if(dwWaitResult != WAIT_OBJECT_0)
			return E_UNEXPECTED;
		
		CComVariant vStream(spXMLStream);
		hr = m_spXMLDoc->save(vStream);
		hr = ResetStm(spXMLStream);

		ReleaseMutex(m_hDocMutex);

		if(hr != S_OK)
			return hr;
	
		pmedium->tymed = TYMED_ISTREAM;
		pmedium->pUnkForRelease = NULL;
		hr = spXMLStream->Clone(&pmedium->pstm);
		if(FAILED(hr))
			return STG_E_MEDIUMFULL;
		return S_OK;
	}

	return S_OK;
}



HRESULT STDMETHODCALLTYPE CAnimDataObject::EnumFormatEtc(unsigned long dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
	if(dwDirection == DATADIR_SET) 
		return E_NOTIMPL;

	return E_NOTIMPL;
	
	/*
	typedef CComObject <CComEnumOnSTL <IEnumFORMATETC, &IID_IEnumFORMATETC, FORMATETC, _Copy<FORMATETC>, std::vector<FORMATETC> > > EnumFORMATETC;
	
	CComObject<EnumFORMATETC>* pEnum = NULL;
	HRESULT hr = CComObject<EnumFORMATETC>::CreateInstance(&pEnum);
	if(!SUCCEEDED(hr))
		return hr;
	
	hr = pEnum->Init(NULL, m_cfArray);
	if(!SUCCEEDED(hr))
	{
		delete pEnum;
		return hr;
	}
	
	return pEnum->QueryInterface(ppEnumFormatEtc);
	*/
}

STDMETHODIMP CAnimDataObject::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
	DWORD *pdwConnection)
{
	return m_spDataAdviseHolder->Advise(this, pformatetc, advf, pAdvSink, pdwConnection);
}
STDMETHODIMP CAnimDataObject::DUnadvise(DWORD dwConnection)
{
	return m_spDataAdviseHolder->Unadvise(dwConnection);
}
STDMETHODIMP CAnimDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
	return m_spDataAdviseHolder->EnumAdvise(ppenumAdvise);
}


//Reference Maker implementation
RefResult CAnimDataObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
{
	if(hTarget == m_pAnimRef)
	{
		switch(message)
		{
		case REFMSG_TARGET_DELETED:
			{
				m_pAnimRef = NULL;
				DataChange();
				break;
			}
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		case REFMSG_NODE_LINK:
			{
				DataChange();
				break;
			}
		default:
			break;
		}
	}
	return REF_SUCCEED;
}

void CAnimDataObject::SetReference(int i, RefTargetHandle rtarg)
{
	if(i==0)
	{
		m_pAnimRef = rtarg;
		//set us to dirty
		InterlockedIncrement(&m_dirty);
	}
};

void CAnimDataObject::DataChange()
{
	SetDirty();
	m_spDataAdviseHolder->SendOnDataChange(this, 0, 0);
}

void CAnimDataObject::SetDirty()
{
	InterlockedIncrement(&m_dirty);

	//This should also signal any rebuilding threads to stop
}


//could this be a thread proc?
int CAnimDataObject::UpdateData()
{
	int res;
	if(m_dirty)
	{
		HRESULT hr;
		//lock up the document
		DWORD dwWaitResult = WaitForSingleObject(m_hDocMutex, INFINITE);
		if(dwWaitResult != WAIT_OBJECT_0)
			return E_UNEXPECTED;

		//Need to empty the document
		CComVariant vBootstrapXML = "http://localhost/xml/min.xml";
#if 0
//		short success;
//		hr = m_spXMLDoc->load(vBootstrapXML, &success);
//		ATLASSERT(success);
#endif
		m_spXMLDoc.Release();
		hr = m_spXMLUtil->CreateDocument(&m_spXMLDoc, vBootstrapXML, 1);
		ATLASSERT(m_spXMLDoc);

#ifdef _XXDEBUG
		CComBSTR caller = "CAnimDataObject::UpdateData:PREtraversal";
		hr = m_spXMLUtil->WalkTree(m_spXMLDoc, 0, caller);
#endif
		
		if(m_pAnimRef)
		{
			//clear the flag we'll use 
			theClearAnimFlags.SetScope(SCOPE_ALL);
			res = m_pAnimRef->EnumAnimTree(&theClearAnimFlags, this, 0);

			//Build a DOM tree
			AnimEnumerator enumerator(m_spXMLDoc);
			res = LocalEnumAnimTree(m_pAnimRef, &enumerator, this, 0);
	
			//clear the flag we'll use 
			res = m_pAnimRef->EnumAnimTree(&theClearAnimFlags, this, 0);

		}

#ifdef _DEBUG
		CComBSTR caller = "CAnimDataObject::UpdateData:POSTtraversal";
//		hr = m_spXMLUtil->WalkTree(m_spXMLDoc, 0, caller);
		int never = 0;
		if(never)
		{
			caller = "C:\\Temp\\vizml2.xml";
			CComVariant vPath(caller);
			hr = m_spXMLDoc->save(vPath);
		}
		m_spXMLDoc->get_xml(&caller);
//		ATLTRACE(caller);
#endif
		m_dirty = 0;
		ReleaseMutex(m_hDocMutex);
	}
	return res;
}


int CAnimDataObject::LocalEnumAnimTree(Animatable *anim, AnimEnumerator *animEnum, Animatable *client, int subNum) 
{
	int res = ANIM_ENUM_PROCEED,i;	
//	int index;	
//	if(animEnum->Depth()>3)	return res;

	// This is just for debugging to look at the ClassName	
	TSTR name;	
	anim->GetClassName(name);	
#ifdef _XXDEBUG
	for(int k = 0; k<animEnum->Depth();k++)
		ATLTRACE(" ");
	ATLTRACE("%s\n", name);
#endif
		
	// process this 	
	if ((res = animEnum->proc(anim,client,subNum))!=ANIM_ENUM_PROCEED) {	
		// Stop means 'don't do my children' but keep going with the rest of the tree	
		if (res==ANIM_ENUM_STOP) {	
			return ANIM_ENUM_PROCEED;	
		} else {	
			return res;	
			}	
		}	

	// process Subs
	if (animEnum->Scope()&SCOPE_SUBANIM)
	{	
		animEnum->IncDepth();	
		// Process regular sub-anims.	
		for (i = 0; i<anim->NumSubs(); i++)
		{	
			if ( anim->SubAnim(i) ) 
			{	
				if ( (res = LocalEnumAnimTree(anim->SubAnim(i), animEnum,anim,i))!=ANIM_ENUM_PROCEED)	
					break;	
			}	
		}	

			
		// Then process the node hierarchy	
		if (animEnum->Scope()&SCOPE_CHILDREN) 
		{	
			for (i=0; i<anim->NumChildren(); i++) 
			{	
				if ( (res = LocalEnumAnimTree(anim->ChildAnim(i), animEnum,anim,i)) !=ANIM_ENUM_PROCEED)	
					break;	
			}	
		}		

		animEnum->DecDepth();	
	}	
		

	return res;	
}

void AnimEnumerator::DecDepth()
{
	PopNodeContext();
	--depth;
}


int AnimEnumerator::proc(Animatable *anim, Animatable *client, int subNum)
{
	HRESULT hr = S_OK;
	CComPtr<IXMLDOMNode> spNewDOMNode;

	//process an animatable in the context of the current DOM node
	CComPtr<IXMLDOMNode> spDOMNodeCtx = Context();


	//Create a DOM node as a child element of the current DOM Node
	//the new DOM Node becomes the current context
	if (anim->SuperClassID() == BASENODE_CLASS_ID)
	{
		hr = CreateDOMAnimNode(spDOMNodeCtx, anim, subNum, &spNewDOMNode, client);
		ATLASSERT(SUCCEEDED(hr));
		
		//when processing a MAX node we push the context twice
		//this is because IncDepth and DecDepth get called twise for MAX nodes
		//Now we implement our own enumerator
		//PushNodeContext(spNewDOMNode);
	}
	//process everything else
	else
	{
		//add DOM node to doc root referenced by the current context

		//this may already be there
		//when created, the new DOM Node becomes the current context
		if(!anim->TestAFlag(A_WORK4))
		{
			hr = CreateDOMAnimNode(m_spElem, anim, subNum, &spNewDOMNode, client);
			ATLASSERT(SUCCEEDED(hr));
			anim->SetAFlag(A_WORK4);
		}
		else
		{
			//FIXME JH
			//it already exists and we need to locate the DOM node that corresponds
			return ANIM_ENUM_STOP;
			ATLASSERT(spNewDOMNode == NULL);
			//spNewDOMNode.Release();
		}

		//Add a reference to this new DOMNode from the context node
//		if(spNewDOMNode)
//			hr = CreateDOMAnimReference(spDOMNodeCtx, spNewDOMNode);
	}

	if(spNewDOMNode)
		PushNodeContext(spNewDOMNode);
	
	return ANIM_ENUM_PROCEED;
}

HRESULT AnimEnumerator::CreateDOMAnimNode(IXMLDOMNode *parent, Animatable *anim,  int subidx, IXMLDOMNode **ppnewnode, Animatable *client)
{
	HRESULT hr;

	if(!(parent && anim && ppnewnode))
		return E_INVALIDARG;

	CComBSTR bstrNS = "http://localhost/xml/schemas/vizML.xdr";
	CComBSTR bstrQName = "viz:Animatable";
	CComBSTR bstrNameTag;
	CComBSTR bstrAttName;
	CComVariant vAttValue;
	CComVariant vOtherValue;
	ATLASSERT(m_spDoc);

	//create the node
	CComPtr<IXMLDOMNode> spTempNode;
	hr = m_spDoc->createNode(CComVariant(NODE_ELEMENT), bstrQName, bstrNS, &spTempNode.p);
	ATLASSERT(spTempNode);

	//set some attributes
	CComQIPtr<IXMLDOMElement> spElem = spTempNode;
	ATLASSERT(spElem);

	//set tthe class name
	bstrAttName = "ClassName";
	CStr name;
	anim->GetClassName(name);
	vAttValue = name.data();
	hr = spElem->setAttribute(bstrAttName, vAttValue);
	ATLASSERT(SUCCEEDED(hr));

	//since name is required set that also override with a real name below
	bstrNameTag = "name";
	hr = spElem->setAttribute(bstrNameTag, vAttValue);
	ATLASSERT(SUCCEEDED(hr));
	vAttValue.Clear();

	//set the id
	bstrAttName = "id";
	CComBSTR bstrID = bstrAttName;
	char buffer[10];
	itoa(++m_dwElementCount, buffer, 10);
	bstrID.Append(buffer);
	vAttValue = bstrID;
	hr = spElem->setAttribute(bstrAttName, vAttValue);
	ATLASSERT(SUCCEEDED(hr));
	vAttValue.Clear();

	//set the moniker
	bstrAttName = "Moniker";
	CComBSTR bstrMk;
	if(anim->SuperClassID() == BASENODE_CLASS_ID)
	{
		INode *pNode = (INode *)anim;
		unsigned long nh = pNode->GetHandle();
		bstrMk = "handle:";
		char buffer[10];
		itoa(nh, buffer, 10);
		bstrMk.Append(buffer);
		vAttValue = bstrMk;
	}
	/*
	else if(anim->SuperClassID() == MATERIAL_CLASS_ID)
	{
		MtlBase *pMat= (MtlBase *)anim;
		bstrMk = "mat:";
		CStr name = pMat->GetName();
		bstrMk.Append(name.data());
		vAttValue = bstrMk;
	}*/
	else
	{
		CComQIPtr<IXMLDOMElement> spContext = Context();
		ATLASSERT(spContext);
		CComVariant vCtxMk;
		hr = spContext->getAttribute(bstrAttName, &vCtxMk);
		bstrMk = vCtxMk.bstrVal;
		bstrMk.Append("|");//delimiter
		char buffer[10];
		itoa(subidx, buffer, 10);
		bstrMk.Append(buffer);
		vAttValue = bstrMk;
	}
	if(vAttValue.vt == VT_BSTR)
	{
#ifdef _XXDEBUG
		for(int k = 0; k<Depth();k++)
			ATLTRACE(" ");
		ATLTRACE("%S\n", vAttValue.bstrVal);
#endif
		hr = spElem->setAttribute(bstrAttName, vAttValue);
		ATLASSERT(SUCCEEDED(hr));
		vAttValue.Clear();
	}

	//Override the name for nodes and materials
	CComBSTR bstrName;
	if(anim->SuperClassID() == BASENODE_CLASS_ID)
	{
		INode *pNode = (INode *)anim;
		unsigned long nh = pNode->GetHandle();
		CStr name = pNode->NodeName();
		bstrName.Append(name.data());
		vAttValue = bstrName;
	}
	else if(client->SuperClassID() == BASENODE_CLASS_ID && subidx == 3)
	{
		INode *pNode = (INode *)client;
		unsigned long nh = pNode->GetHandle();
		CStr name = pNode->NodeName();
		bstrName.Append(name.data());
		vAttValue = bstrName;
	}
	else if(anim->SuperClassID() == MATERIAL_CLASS_ID)
	{
		MtlBase *pMat= (MtlBase *)anim;
		CStr name = pMat->GetName();
		bstrName.Append(name.data());
		vAttValue = bstrName;
	}
	if(vAttValue.vt == VT_BSTR)
	{
		hr = spElem->setAttribute(bstrNameTag, vAttValue);
		ATLASSERT(SUCCEEDED(hr));
		vAttValue.Clear();
	}

	//add the new node to the scene
	if(parent)
	{
		hr = parent->appendChild(spTempNode, ppnewnode);
		ATLASSERT(SUCCEEDED(hr));
		ATLASSERT(*ppnewnode);
	}

	return hr;
}

HRESULT AnimEnumerator::CreateDOMAnimReference(IXMLDOMNode* pMaker, IXMLDOMNode* pTarget)
{
	//this needs to actually get the id attribute of the Target and add it to the subanims 
	//attribute on the Maker
	HRESULT hr;
	CComQIPtr<IXMLDOMElement> spElemMaker = pMaker;
	ATLASSERT(spElemMaker);
	CComQIPtr<IXMLDOMElement> spElemTarget = pTarget;
	ATLASSERT(spElemTarget);

	//get the id attribute of teh maker
	CComBSTR bstrAttName = "id";
	CComBSTR bstrAttNameNumsubs = "numsubs";
	CComBSTR bstrResult;
	CComVariant vNewRef;
	CComVariant vNumSubs;
	hr = spElemTarget->getAttribute(bstrAttName, &vNewRef);
	ATLASSERT(hr == S_OK);

	bstrAttName = "subanims";
	CComVariant vMakerRefs;
	hr = spElemMaker->getAttribute(bstrAttName, &vMakerRefs);
	if(hr == S_OK)
	{
		bstrResult = vMakerRefs.bstrVal;
		bstrResult.Append(", ");
		bstrResult.Append(vNewRef.bstrVal);
		hr = spElemMaker->getAttribute(bstrAttNameNumsubs, &vNumSubs);
		vNumSubs.intVal = vNumSubs.intVal +1;
	}
	else if (hr == S_FALSE)
	{
		bstrResult = vNewRef.bstrVal;
		vNumSubs = 1;
	}

	vNewRef.Clear();
	vNewRef = bstrResult;
	hr = spElemMaker->setAttribute(bstrAttNameNumsubs, vNumSubs);
	hr = spElemMaker->setAttribute(bstrAttName, vNewRef);


	return hr;
}


AnimEnumerator::AnimEnumerator(IXMLDOMDocument *pDoc)
{
	m_dwElementCount = 0;
	depth = 0;
	m_spDoc = pDoc;
	HRESULT hr = pDoc->get_documentElement(&m_spElem);
	ATLASSERT(m_spElem);
	PushNodeContext(m_spElem);
	SetScope(SCOPE_ALL);
}

void AnimEnumerator::PushNodeContext(IXMLDOMNode *pNode)
{
	pNode->AddRef();
	m_NodeStack.push(pNode);
}

void AnimEnumerator::PopNodeContext()
{
	IXMLDOMNode *pNode = m_NodeStack.top();
	m_NodeStack.pop();
	pNode->Release();
}

IXMLDOMNode* AnimEnumerator::Context()
{
	return m_NodeStack.top();
}
