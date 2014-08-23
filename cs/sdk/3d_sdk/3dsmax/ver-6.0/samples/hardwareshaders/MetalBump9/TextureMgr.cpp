//_____________________________________________________________________________
//
//	File: TextureMgr.cpp
//	
//
//_____________________________________________________________________________


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "TextureMgr.h"

//_____________________________________
//
//	Init Singleton 
//
//_____________________________________

TextureMgr*		Singleton<TextureMgr>::m_Singleton = 0;
TextureMgr		TheTextureMgr;
TextureMgr*		gTextureMgr;

//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________

//_____________________________________
//
//	Default constructor 
//
//_____________________________________

TextureMgr::TextureMgr()
{
	m_List.clear();
}


//______________________________________
//
//	Default destructor 
//
//______________________________________

TextureMgr::~TextureMgr()
{
	int	i,Count;

	Count = m_List.size();

	for(i=0; i < Count; i++)
	{
		SAFE_RELEASE(m_List[i].m_Texture);
	}

	m_List.clear();

}

//______________________________________
//
//	Find 
//
//______________________________________

int TextureMgr::Find(const TSTR &Name, TextureType Type)
{
	int	i,Count;

	if(Name.length())
	{
		Count = m_List.size();

		for(i=0; i < Count; i++)
		{
			if(Name == m_List[i].m_Name &&
			   Type == m_List[i].m_Type)
			{
				return(i);
			}
		}

	}

	return(-1);
}

//______________________________________
//
//	Add 
//
//______________________________________

void TextureMgr::Add(const TSTR &Name, TextureType Type, IDirect3DBaseTexture9 *Texture)
{
	TCache	T;

	T.m_Name	= Name;
	T.m_Type    = Type;
	T.m_Texture = Texture;
	T.m_Ref		= 1;
	T.m_Texture->AddRef();

	m_List.push_back(T);

}

//______________________________________
//
//	Get 
//
//______________________________________

IDirect3DBaseTexture9*	TextureMgr::Get(int Index)
{
//	TCache	T;

	if(Index >=0 && Index < m_List.size())
	{
		TCache	&T = m_List[Index];

		T.m_Texture->AddRef();
		T.m_Ref++;

		return(T.m_Texture);
	}

	return(NULL);

}

//______________________________________
//
//	AddRef 
//
//______________________________________

void TextureMgr::AddRef(const TSTR &Name, TextureType Type)
{
//	TCache T;
	int		Index;

	Index = Find(Name,Type);

	if(Index >=0 && Index < m_List.size())
	{
		TCache & T = m_List[Index];

		T.m_Texture->AddRef();
		T.m_Ref++;
	}

}


//______________________________________
//
//	Release 
//
//______________________________________

void TextureMgr::Release(const TSTR &Name, TextureType Type)
{
	int		Index;

	Index = Find(Name,Type);

	if(Index >=0 && Index < m_List.size())
	{
		TCache	&T = m_List[Index];
//		T = m_List[Index];
		T.m_Ref--;

		if(T.m_Texture)
			T.m_Texture->Release();

//		SAFE_RELEASE(T.m_Texture);

		if(T.m_Ref == 0)
		{
			m_List.erase(m_List.begin() + Index);
		}
	}

}
