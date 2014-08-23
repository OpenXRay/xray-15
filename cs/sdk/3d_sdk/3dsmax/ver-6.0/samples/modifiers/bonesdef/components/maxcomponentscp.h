#ifndef _MAXCOMPONENTSCP_H_
#define _MAXCOMPONENTSCP_H_



template <class T>
class CProxy_ISkinEngineEvents : public IConnectionPointImpl<T, &IID__ISkinEngineEvents, CComDynamicUnkArray>
{

public:
	HRESULT Fire_GetInterpCurvePiece3D(INT BoneId, INT CurveId, INT SegId, FLOAT distance, FLOAT * pPoint)
	{
		HRESULT ret;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{				
			// NS 8/24/99 I changed the Automatic creation of CComPtr, since it has to be thread safe.

			CComPtr<IUnknown> *sp = new CComPtr<IUnknown>;
			pT->Lock();
			*sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			_ISkinEngineEvents* p_ISkinEngineEvents = reinterpret_cast<_ISkinEngineEvents*>(sp->p);
			if (p_ISkinEngineEvents != NULL)
				ret = p_ISkinEngineEvents->GetInterpCurvePiece3D(BoneId, CurveId, SegId, distance, pPoint);

			pT->Lock();
			delete sp;
			pT->Unlock();
		}	
		return ret;
	
	}
	HRESULT Fire_GetTangentPiece3D(INT BoneId, INT CurveId, INT SegId, FLOAT distance, FLOAT * pPoint)
	{
		HRESULT ret;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			
			// NS 8/24/99 I changed the Automatic creation of CComPtr, since it has to be thread safe.

			CComPtr<IUnknown> *sp = new CComPtr<IUnknown>;
			
			pT->Lock();
			*sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			
			_ISkinEngineEvents* p_ISkinEngineEvents = reinterpret_cast<_ISkinEngineEvents*>(sp->p);
			
			if (p_ISkinEngineEvents != NULL)
				ret = p_ISkinEngineEvents->GetTangentPiece3D(BoneId, CurveId, SegId, distance, pPoint);
			
			pT->Lock();
			delete sp;
			pT->Unlock();

		}	
		return ret;
	
	}

};
#endif