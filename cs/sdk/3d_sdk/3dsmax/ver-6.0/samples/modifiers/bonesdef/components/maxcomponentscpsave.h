#ifndef _MAXCOMPONENTSCP_H_
#define _MAXCOMPONENTSCP_H_



template <class T>
class CProxy_IFlexEngineEvents : public IConnectionPointImpl<T, &IID__IFlexEngineEvents, CComDynamicUnkArray>
{
	//Warning this class may be recreated by the wizard.
public:
	HRESULT Fire_GetPoints(INT time, INT num, FLOAT * point)
	{
		HRESULT ret;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			// I changed the Automatic creation of CComPtr, since it has to be thread safe.
			
			CComPtr<IUnknown> *sp = new CComPtr<IUnknown>;
			
			pT->Lock();
			*sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
				
			_IFlexEngineEvents* p_IFlexEngineEvents = reinterpret_cast<_IFlexEngineEvents*>(sp->p);
			if (p_IFlexEngineEvents != NULL)
				ret = p_IFlexEngineEvents->GetPoints(time, num, point);

			pT->Lock();
			delete sp;
			pT->Unlock();


		}	return ret;
	
	}
	HRESULT Fire_GetForces(INT time, INT num, ULONG p_stride, FLOAT * pPos, ULONG v_stride, FLOAT * pVel, FLOAT * pForces)
	{
		HRESULT ret;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			// I changed the Automatic creation of CComPtr, since it has to be thread safe.

			CComPtr<IUnknown> *sp = new CComPtr<IUnknown>;
			
			pT->Lock();
			*sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			_IFlexEngineEvents* p_IFlexEngineEvents = reinterpret_cast<_IFlexEngineEvents*>(sp->p);
			if (p_IFlexEngineEvents != NULL)
				ret = p_IFlexEngineEvents->GetForces(time, num, p_stride, pPos, v_stride, pVel, pForces);

			pT->Lock();
			delete sp;
			pT->Unlock();

		}	
		return ret;
	
	}
};



template <class T>
class CProxy_ISkinEngineEvents : public IConnectionPointImpl<T, &IID__ISkinEngineEvents, CComDynamicUnkArray>
{
	//Warning this class may be recreated by the wizard.
public:
	HRESULT Fire_GetInterpCurvePiece3D(INT BoneId, INT CurveId, INT SegId, FLOAT distance, FLOAT * pPoint)
	{
		HRESULT ret;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{				
			// I changed the Automatic creation of CComPtr, since it has to be thread safe.

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
			
			// I changed the Automatic creation of CComPtr, since it has to be thread safe.

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