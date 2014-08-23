#include "stdafx.h"
#include "net_stream.h"


INetReader::~INetReader	()
{

}

void		INetReaderGenStream::	r			(void *p,int cnt)
{
	stream->Read( p, cnt );
}

void INetBlockReader::r(void *p,int cnt)
{
	if(cnt==0)
		return;
	u8* pointer = (u8*)p;
	add( cnt );
	for(;;)
	{
		if( mem_reader )
		{
			u32 read_cnt = _min( cnt, mem_reader->count() ); 
			mem_reader->r( pointer, read_cnt );
			pointer+=read_cnt;
			cnt-=read_cnt;
		}
		if( mem_reader && mem_reader->count() == 0 )
				xr_delete( mem_reader );

		if( cnt==0 )
				return;
		R_ASSERT( stream );
		u32 block_size ;
		stream->Read(&block_size, sizeof(block_size) );
		create_block( block_size );
		stream->Read( mem_reader->pdata(), block_size );
	}
}

void		INetBlockReader::			create_block	( u32 size )
{
	VERIFY(!mem_reader);

	mem_reader = xr_new<CMemoryReadBlock>( size );
}

INetBlockReader::~INetBlockReader		()
{
	R_ASSERT(!mem_reader||mem_reader->count()==0);
	xr_delete(mem_reader);
}
/*
	IC void			w_string(const char *p)			{	w(p,(u32)xr_strlen(p));w_u8(13);w_u8(10);	}
	IC void			w_stringZ(const char *p)		{	w(p,(u32)xr_strlen(p)+1);			}
	IC void			w_stringZ(const shared_str& p) 	{	w(*p?*p:"",p.size());w_u8(0);		}
	IC void			w_stringZ(shared_str& p)		{	w(*p?*p:"",p.size());w_u8(0);		}
*/

void	INetReader::r_string	(char *dest, u32 tgt_sz)
{
	
	R_ASSERT( tgt_sz < 1024 );
	char buf[ 1024 ];
	
	buf[ 0 ] = r_u8();
	u32 i = 1;
	for( ; i < 1024 ; ++i )
	{
		
		buf[ i ] = r_u8();
		if( buf[ i-1 ] == 13 && buf[ i ] == 10 )
			break;
	}
	u32 lenght = i-1;
	R_ASSERT2( lenght < (tgt_sz-1) ,"Dest string less than needed." );
	//R_ASSERT	(!IsBadReadPtr((void*)src,sz));

	
	buf[lenght] = 0;

	strncpy_s	(dest,tgt_sz, buf, lenght+1 );

    //dest[sz]	= 0;
}


void	INetReader::r_stringZ	( char *dest )
{
	
	//R_ASSERT( tgt_sz < 1024 );
	
	u32 i = 0;
	for( ; i < 1024 ; ++i )
	{
		dest[i] = r_u8();

		if( dest[i] == 0  )
			break;
	}

}
void	INetReader::r_stringZ	( shared_str &dest )
{
	char buf[ 1024 ];
	r_stringZ( buf );
	dest._set( buf );
}
void	INetBlockReader::load_buffer (LPCSTR fn)
{
	xr_delete(mem_reader);
	IReader* fs			= FS.r_open(fn);
	 if(fs)
	 {
		//mem_reader = xr_new<CMemoryReader>( fs->length() );
	    create_block( fs->length() );
		fs->r( mem_reader->pdata(), fs->length() );
		FS.r_close(fs);// ->close();
	 }
}
void	INetMemoryBuffWriter::	send_and_clear()
{
	if( !mem_writter )
		return;
	send_not_clear( stream );
	clear();
}

void	INetWriter::	send_not_clear(IGenericStream* _stream)
{
 	R_ASSERT(mem_writter);
	mem_writter->send( _stream );
}

void	INetWriter::	clear		  ()
{
	xr_delete( mem_writter );
}

void INetMemoryBuffWriter::w(const void* ptr, u32 count)
{
	if(count==0)
		return;
	add( count );
	const u8* pointer = (const u8*)ptr;
	for(;;)
	{
		if( mem_writter )
		{
			u32 write_cnt = _min( count,  mem_writter->rest()  ); 
			mem_writter->w( pointer, write_cnt );
			count-=write_cnt;
			pointer+=write_cnt;
		} else
			create_block();

		VERIFY(mem_writter->rest()>=0);
		if( mem_writter && mem_writter->rest()==0 )
		{
			R_ASSERT(u32(-1)!=net_block_write_data_size);

			send_and_clear ();
			create_block();

		}
			
		if( count == 0 )
				return;
		
	}
}

void INetFileBuffWriter::w(const void* ptr, u32 count)
{
	VERIFY( mem_writter );
	add( count );
	mem_writter->w( ptr, count );
}

void INetMemoryBuffWriter::create_block()
{
	VERIFY(!mem_writter);
	mem_writter = xr_new<CMemoryWriteBlock>( net_block_write_data_size );
}
INetMemoryBuffWriter::			~INetMemoryBuffWriter	()
{
	if(mem_writter)
		send_and_clear ();
}
INetWriter::~INetWriter()
{
	R_ASSERT(!mem_writter);
}

void	INetWriter::save_buffer		( LPCSTR fn )const
{
	if(mem_writter)
		mem_writter->save_to(fn);
}

INetFileBuffWriter::INetFileBuffWriter(LPCSTR	_file_name, u32 block_size, bool _reopen ):INetWriter()
{
	
	mem_writter = xr_new<CFileWriteBlock>( _file_name, block_size, _reopen );
}

INetFileBuffWriter::~INetFileBuffWriter()
{
	xr_delete(mem_writter);
}

void CMemoryReader::r(void *p,int cnt)const
{	
	CopyMemory	( p, data+position, cnt );
	position	+=cnt;
	R_ASSERT(position<=file_size);
}

CMemoryReader::CMemoryReader	( const u32	file_size_ ): file_size( file_size_ ), position( 0 )
{
	 data = (u8*)	Memory.mem_alloc	(file_size_
#ifdef DEBUG_MEMORY_NAME
			,		"CMemoryReader - storage"
#endif // DEBUG_MEMORY_NAME
			);
}

CMemoryReader::	~CMemoryReader	()
{
	Memory.mem_free( data );
}

#include "../../xrCore/FS_impl.h"
u32 INetReader::find_chunk				(u32 ID, BOOL* bCompressed)
{
	R_ASSERT( false );
	return inherited::find_chunk(ID, bCompressed);
}




void CMemoryWriteBlock::send(IGenericStream	 *_stream)
{
	u32 block_size = tell();
	_stream->Write( &block_size, sizeof( block_size) );
	_stream->Write( pointer(), block_size );
}
CFileWriteBlock::CFileWriteBlock ( LPCSTR fn, u32 _size, bool _reopen  ):IWriteBlock( _size ), file_name( fn ),file(0),file_map(0),reopen(_reopen)
{
	
	if(reopen)
		return;
	string_path		lfile_name;
	FS.update_path	( lfile_name, "$level$", fn );
	file			= fopen( lfile_name, "wb");
	VERIFY			(file);

}

CFileWriteBlock::~CFileWriteBlock ( )
{

	fclose				(file);
	if (file_map)
		fclose			(file_map);
	if(reopen)
		return;
	string_path			 N;
	FS.update_path		( N, "$level$", file_name );
	if(FS.exist( N ))
		FS.file_delete( N );
}


void	CFileWriteBlock::w(const void* ptr, u32 count)
{
	
	fwrite	(const_cast<void*>(ptr), 1, count, file);

}

void	CFileWriteBlock::send( IGenericStream	 *_stream )	
{


	R_ASSERT				(file_map);
	fseek					( file_map, 0, SEEK_SET );
	u32 const length		= _filelength( _fileno( file_map ) );
	R_ASSERT				( length );

	u32 const position		= _stream->GetPos();
	u32 block_size			= size;
	//_stream->SetLength		(position + length + ((int(length) - 1)/block_size + 1)*sizeof(block_size));
	_stream->SetLength		( position + length );
	_stream->Seek			(position);

	
	void* block				= _alloca(block_size);

	for (int n = length/block_size, i = 0; i<n; ++i) {
		fread				( block, 1, block_size, file_map );
	//	_stream->Write		( &block_size, sizeof( block_size) );
		_stream->Write		( block, block_size );
	}

	block_size				= length % block_size;
	if (block_size==0) {
//		xr_free				(block);
		return;
	}

	fread					( block, 1, block_size, file_map );
//	_stream->Write			( &block_size, sizeof( block_size) );
	_stream->Write			( block, block_size );
}

u32		CFileWriteBlock::rest()
{
	VERIFY( file );
	return size - ftell( file_map ) ;
}

void	CFileWriteBlock::w_close()
{
	R_ASSERT			(!file_map);
	if(!reopen)
	{
		VERIFY				(file);
		fclose				(file);
	}
	string_path			 lfile_name;
	FS.update_path		( lfile_name, "$level$", file_name );
	file_map			= fopen( lfile_name, "rb" );
}


INetReaderFile::INetReaderFile( LPCSTR file_name ): file(0)
{
	file = fopen( file_name, "rb" );// FS.r_open( file_name );
}

INetReaderFile::~INetReaderFile( )
{
	 fclose(file);//FS.r_close( file );
}

void	INetReaderFile::r(void *p,int cnt)
{
	//file->r( p, cnt );
	fread					( p, 1, cnt, file );
}

CGenStreamOnFile::CGenStreamOnFile( CVirtualFileRW	*_file ):file(_file)
{
	VERIFY(file);
	//string_path			 lfile_name;
	//FS.update_path		( lfile_name, "$level$", file_name );
	//file = fopen( lfile_name, "rb" );
}
CGenStreamOnFile::~CGenStreamOnFile( )
{
	// fclose(file);
}
DWORD __stdcall CGenStreamOnFile::GetLength()
{
	return file->length();
	//return _filelength( _fileno( file ) );
}
DWORD __stdcall CGenStreamOnFile::Read(void* Data, DWORD count)
{
	R_ASSERT(false);
	//fread( Data, 1, count, file );
	return count;
}