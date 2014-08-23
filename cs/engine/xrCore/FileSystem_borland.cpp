//----------------------------------------------------
// file: FileSystem.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include "FileSystem.h"

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

bool EFS_Utils::GetOpenName(LPCSTR initial, xr_string& buffer, bool bMulti, LPCSTR offset, int start_flt_ext )
{
	char			buf	[255*255]; //max files to select
	strcpy_s			(buf, buffer.c_str());

	bool bRes		= GetOpenNameInternal(initial, buf, sizeof(buf), bMulti, offset, start_flt_ext);

	if (bRes)
		buffer=(char*)buf;

	return bRes;
}


bool EFS_Utils::GetSaveName( LPCSTR initial, xr_string& buffer, LPCSTR offset, int start_flt_ext )
{
	string_path				buf;
	strcpy_s				(buf,sizeof(buf), buffer.c_str());
	bool bRes				= GetSaveName(initial,buf,offset,start_flt_ext);
	if (bRes) 
		buffer				= buf;

	return bRes;
}
//----------------------------------------------------

void EFS_Utils::MarkFile(LPCSTR fn, bool bDeleteSource)
{
	xr_string ext = strext(fn);
	ext.insert		(1,"~");
	xr_string backup_fn = EFS.ChangeFileExt(fn,ext.c_str());
	if (bDeleteSource){
		FS.file_rename(fn,backup_fn.c_str(),true);
	}else{
		FS.file_copy(fn,backup_fn.c_str());
	}
}

xr_string	EFS_Utils::AppendFolderToName(xr_string& tex_name, int depth, BOOL full_name)
{
	string1024 nm;
	strcpy_s(nm,tex_name.c_str());
	tex_name = AppendFolderToName(nm,depth,full_name);
	return tex_name;
}

BOOL EFS_Utils::CheckLocking(LPCSTR fname, bool bOnlySelf, bool bMsg)//, shared_str* owner)
{
	string256 fn; strcpy_s(fn,fname);

	if (bOnlySelf) return (m_LockFiles.find(fn)!=m_LockFiles.end());
	if (FS.exist(fn)){
		HANDLE handle=CreateFile(fn,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		CloseHandle(handle);
        if (INVALID_HANDLE_VALUE==handle){
			if (bMsg)	Msg("#!Access denied. File: '%s' currently locked by any user.",fn);
//.            if (owner) 	*owner = GetLockOwner(initial,fname);
        }
		return (INVALID_HANDLE_VALUE==handle);
	}
    return FALSE;
}

BOOL EFS_Utils::LockFile(LPCSTR fname, bool bLog)
{
	string256 fn; strcpy_s(fn,fname);

	BOOL bRes=false;
	if (m_LockFiles.find(fn)==m_LockFiles.end()){
		HANDLE handle=CreateFile(fn,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if (INVALID_HANDLE_VALUE!=handle){
			LPSTR lp_fn			= fn;
			std::pair<HANDLEPairIt, bool> I=m_LockFiles.insert(mk_pair(lp_fn,handle));
			R_ASSERT(			I.second);

			bRes				= true;
		}
	}
	return bRes;
}

BOOL EFS_Utils::UnlockFile(LPCSTR fname, bool bLog)
{
	string256 fn; strcpy_s(fn,fname);

	HANDLEPairIt it 			= m_LockFiles.find(fn);
	if (it!=m_LockFiles.end()){
    	void* handle 			= it->second;
		m_LockFiles.erase		(it);
		return CloseHandle		(handle);
	}
	return false;
}
