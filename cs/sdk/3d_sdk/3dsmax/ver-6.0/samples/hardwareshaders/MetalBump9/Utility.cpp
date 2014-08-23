//_____________________________________________________________________________
//
//	File: Utility.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "Utility.h"
#include "bmmlib.h"
//_____________________________________________________________________________
//
//	Globals	
//
//_____________________________________________________________________________

TCHAR	gFilePath[MAX_PATH];

//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________

//_____________________________________
//
//	GetFileResource
//
//_____________________________________

bool GetFileResource(const char *Name, const char *Type, void **Data, unsigned long &Size)
{
	HRSRC			Rec;
	HGLOBAL			Mem;

	Rec = FindResource(hInstance,
					   Name,Type);

	if(Rec)
	{
		Mem  = LoadResource(hInstance,Rec);
		Size = SizeofResource(hInstance,Rec);

		if(Mem && Size)
		{
			*Data = LockResource(Mem);

			return(true);
		}

	}
	
	return(false);
}

//_____________________________________
//
//	FindFile
//
//_____________________________________

TCHAR *FindFile(TCHAR *File)
{
	int			i;
	BOOL		Found;
	TCHAR		*Part;

	Interface	*I;

	Found = 0;

//	NH 21|04|2002
//  Replaced the current search but used the actual path including drive prefix
//  the original seemed to get stuck in the current path, or the last path used
//  this caused files not to be loaded, even though they were there.
//
//	_splitpath(File,Drive,Dir,FileName,Ext);
//
//	Found = SearchPath(Dir,FileName,Ext,MAX_PATH,gFilePath,&Part);
////////////////////////////////////////////////////////////////////////////////

	TSTR p,f,e;
	TSTR name(File);
	SplitFilename(name, &p, &f, &e);

	name = f + e;
	//left the following in as it is used in other methods
	

//	Found = SearchPath(p.data(),f.data(),e.data(),MAX_PATH,gFilePath,&Part);
	Found = SearchPath(p.data(),name.data(),NULL,MAX_PATH,gFilePath,&Part);
	if(!Found)
	{

		I = GetCOREInterface();

		// Search the from where the file was loaded....

		TSTR CurFileP = I->GetCurFilePath();
//		TSTR CurFile = I->GetCurFileName();
		TSTR tP;
		
		SplitFilename(CurFileP,&tP,NULL,NULL);
		Found = SearchPath(tP.data(),name.data(),NULL,MAX_PATH,gFilePath,&Part);
		
		//
		//	Search maps
		//

		if(!Found)
		{
			for (i=0; i<TheManager->GetMapDirCount(); i++) {
				TCHAR* dir = TheManager->GetMapDir(i);
				if((Found = SearchPath(dir,name.data(),NULL,MAX_PATH,gFilePath,&Part)))
				{
					break;
				}
			}
		}
		// not sure why we search here....
		if(!Found)
		{
			//
			//	Search plugins
			//
			for(i=0; i < I->GetPlugInEntryCount(); i++)
			{
				TCHAR* dir = I->GetPlugInDir(i);
				if((Found = SearchPath(dir,name.data(),NULL,MAX_PATH,gFilePath,&Part)))
				{
					break;
				}
			}

		}

	}

	if(Found)
	{
		return(gFilePath);
	}
	else
	{
		return(NULL);
	}

}


//_____________________________________
//
//	CreateShaderResource
//
//_____________________________________

HRESULT CreateVertexShaderResource(IDirect3DDevice9 *Device, unsigned long Res, LPDIRECT3DVERTEXDECLARATION9 * vdecl,
							   LPDIRECT3DVERTEXSHADER9 *Handle)

{
	HRESULT			Hr;
  	unsigned long	Size;
	unsigned long   *Data;
	char			*Name;
	D3DCAPS9		Caps;
	DWORD Usage = 0;
	
	Hr = S_OK;

	if(!Res)
	{
		Handle = 0;
		return(S_OK);
	}

	Device->GetDeviceCaps(&Caps);

	// for those mad Laptop users
	if(Caps.DeviceType == D3DDEVTYPE_REF)
	{
		Usage = D3DUSAGE_SOFTWAREPROCESSING;
	}


	D3DVERTEXELEMENT9 test[] = 
	{
			{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
			{ 0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   1 },
			{ 0, 36, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 44, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
			{ 0, 52, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
			{ 0, 60, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		D3DDECL_END()
	};


	
	Hr = Device->CreateVertexDeclaration(test, vdecl);
	if (FAILED(Hr)) {
		return Hr;
	}


	Name = MAKEINTRESOURCE(Res);

	if(GetFileResource(Name,"RT_RCDATA",(void **)&Data,Size))
	{
		Hr = Device->CreateVertexShader(Data, Handle);
		if (FAILED(Hr)) {
			return Hr;
		}

	}
	else
	{
		return(E_FAIL);
	}

	return(Hr);

}


HRESULT CreatePixelShaderResource(IDirect3DDevice9 *Device, unsigned long Res, LPDIRECT3DPIXELSHADER9 *Handle)

{
	HRESULT			Hr;
  	unsigned long	Size;
	unsigned long   *Data;
	char			*Name;
	
	D3DCAPS9		Caps;
	DWORD Usage = 0;
	
	Hr = S_OK;

	if(!Res)
	{
		Handle = 0;
		return(S_OK);
	}

	Device->GetDeviceCaps(&Caps);

	// for those mad Laptop users
	if(Caps.DeviceType == D3DDEVTYPE_REF)
	{
		Usage = D3DUSAGE_SOFTWAREPROCESSING;
	}



	Name = MAKEINTRESOURCE(Res);

	if(GetFileResource(Name,"RT_RCDATA",(void **)&Data,Size))
	{
		if(FAILED(Hr = Device->CreatePixelShader(Data,Handle)))
		{
			return(Hr);
		}
	}

	else
	{
		return(E_FAIL);
	}

	return(Hr);

}

//_____________________________________
//
//	CreateShader
//
//_____________________________________


HRESULT CreateShaderVertex(IDirect3DDevice9 *Device, TSTR File,LPDIRECT3DVERTEXDECLARATION9 * vdecl, LPDIRECT3DVERTEXSHADER9 *Handle)
{
	HRESULT			Hr;
	HANDLE			hFile;
	unsigned long	FileSize;
  	unsigned long	*Shader;
	TCHAR			*FileName;

	D3DCAPS9		Caps;
	DWORD Usage = 0;

	Hr = S_OK;

	D3DVERTEXELEMENT9 test[] = 
	{
			{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
			{ 0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	 1 },
			{ 0, 36, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 44, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
			{ 0, 52, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
			{ 0, 60, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		D3DDECL_END()
	};


	
	Hr = Device->CreateVertexDeclaration(test, vdecl);
	if (FAILED(Hr)) {
		return Hr;
	}



	if(!File.Length())
	{
		Handle = 0;
		return(S_OK);
	}

	FileName = FindFile(File);

	if(Device && FileName)
	{

		Device->GetDeviceCaps(&Caps);

		// for those mad Laptop users
		if(Caps.DeviceType == D3DDEVTYPE_REF)
		{
			Usage = D3DUSAGE_SOFTWAREPROCESSING;
		}

		hFile = CreateFile(FileName,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
		if(hFile == INVALID_HANDLE_VALUE) 
		{
			return(E_FAIL);
		}
		
		FileSize = GetFileSize(hFile,NULL);
		
		Shader = (unsigned long*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,FileSize);
		if(!Shader) 
		{
			return(E_FAIL);
		}

		ReadFile(hFile,(void*)Shader,FileSize,&FileSize,NULL);
		CloseHandle(hFile);

		if(FAILED(Hr = Device->CreateVertexShader(Shader, Handle)))
		{
			return(Hr);
		}
	

		HeapFree(GetProcessHeap(),0,(void *)Shader);

	}
	else
	{
		return(E_FAIL);
	}


	return(Hr);
}


HRESULT CreateShaderPixel(IDirect3DDevice9 *Device, TSTR File, LPDIRECT3DPIXELSHADER9 *Handle)
{
	HRESULT			Hr;
	HANDLE			hFile;
	unsigned long	FileSize;
  	unsigned long	*Shader;
	TCHAR			*FileName;

	D3DCAPS9		Caps;
	DWORD Usage = 0;

	Hr = S_OK;



	if(!File.Length())
	{
		Handle = 0;
		return(S_OK);
	}

	FileName = FindFile(File);

	if(Device && FileName)
	{

		Device->GetDeviceCaps(&Caps);

		// for those mad Laptop users
		if(Caps.DeviceType == D3DDEVTYPE_REF)
		{
			Usage = D3DUSAGE_SOFTWAREPROCESSING;
		}

		hFile = CreateFile(FileName,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
		if(hFile == INVALID_HANDLE_VALUE) 
		{
			return(E_FAIL);
		}
		
		FileSize = GetFileSize(hFile,NULL);
		
		Shader = (unsigned long*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,FileSize);
		if(!Shader) 
		{
			return(E_FAIL);
		}

		ReadFile(hFile,(void*)Shader,FileSize,&FileSize,NULL);
		CloseHandle(hFile);

		if(FAILED(Hr = Device->CreatePixelShader(Shader, Handle)))
		{
			return(Hr);
		}
	

		HeapFree(GetProcessHeap(),0,(void *)Shader);

	}
	else
	{
		return(E_FAIL);
	}


	return(Hr);
}

//_____________________________________
//
//	GetTriObjectFromNode
//
//_____________________________________

TriObject* GetTriObjectFromNode(INode *Node, TimeValue T, bool &Delete) 
{
	Delete		= false;
	Object *Obj = Node->EvalWorldState(T).obj;

	if(Obj && Obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0))) 
	{ 
		TriObject *Tri = (TriObject *)Obj->ConvertToType(T,Class_ID(TRIOBJ_CLASS_ID,0));

		if(Obj != Tri) 
		{
			Delete = true;
		}

		return(Tri);
	}
	else 
	{
		return(NULL);
	}

}


//_____________________________________
//
//	FindNodeRef
//
//_____________________________________

INode* FindNodeRef(ReferenceTarget *Rt) 
{
	DependentIterator Di(Rt);
	ReferenceMaker	*Rm;
	INode	*Nd = NULL;

	while (Rm = Di.Next()) 
	{	
		if(Rm->SuperClassID() == BASENODE_CLASS_ID) 
		{
			return (INode *)Rm;
		}

		Nd = FindNodeRef((ReferenceTarget *)Rm);

		if(Nd)
		{
			 return(Nd);
		}
	}

	return(NULL);
}


//_____________________________________
//
//	AddNormal
//
//_____________________________________

void VNormal::AddNormal(Point3 &N, unsigned long Smooth, Point3 &S, Point3 &T) 
{
	if(!(Smooth & m_Smooth) && m_Init) 
	{
		if(m_Next)
		{	
			m_Next->AddNormal(N,Smooth,S,T);
		}
		else 
		{
			m_Next = new VNormal(N,Smooth,S,T);
		}
	} 
	else
	{
		m_Normal += N;
		m_S		 += S;
		m_T		 += T;
		m_Smooth |= Smooth;
		m_Init    = true;
	}
}

//_____________________________________
//
//	GetNormal
//
//_____________________________________

Point3& VNormal::GetNormal(unsigned long Smooth, Point3 &S, Point3 &T, Point3 &SxT)
{
	if((m_Smooth & Smooth) || !m_Next)
	{
		 S	 = m_S;
		 T	 = m_T;
		 SxT = m_SxT;

		 return(m_Normal);
	}
	else
	{
		 return(m_Next->GetNormal(Smooth,S,T,SxT));	
	}
}

//_____________________________________
//
//	Normalize
//
//_____________________________________

void VNormal::Normalize() 
{
	VNormal	*Ptr,*Prev;
	Matrix3	Mat;

	Ptr  = m_Next;
	Prev = this;

	while(Ptr) 
	{
		if(Ptr->m_Smooth & m_Smooth) 
		{
			m_Normal += Ptr->m_Normal;
			m_S		 += Ptr->m_S;
			m_T      += Ptr->m_T;

			Prev->m_Next = Ptr->m_Next;
			Ptr->m_Next = NULL;
			delete Ptr;
			Ptr = Prev->m_Next;
		} 
		else 
		{
			Prev = Ptr;
			Ptr  = Ptr->m_Next;
		}
	}


	Point3Normalize(m_Normal);
	Point3Normalize(m_S);

	m_T	= CrossProd(m_S,m_Normal);
	Point3Normalize(m_T);

	m_S	= CrossProd(m_Normal,m_T);
	Point3Normalize(m_S);

	m_SxT = m_Normal;

	if(m_Next)
	{
		 m_Next->Normalize();
	}

}
