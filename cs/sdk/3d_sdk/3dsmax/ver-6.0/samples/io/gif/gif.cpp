//-----------------------------------------------------------------------------
// --------------------
// File ....: gif.cpp
// --------------------
// Author...: Tom Hudson
// Date ....: Dec. 09, 1995
// Descr....: GIF File I/O Module
//
// History .: Dec. 09 1995 - Started
//
// This source contains:
//
// DECODE.C - An LZW decoder for GIF
// Copyright (C) 1987, by Steven A. Bennett
//
// Permission is given by the author to freely redistribute and include
// this code in any program as long as this credit is given where due.
//
// In accordance with the above, I want to credit Steve Wilhite who wrote
// the code which this is heavily inspired by...
//
// GIF and 'Graphics Interchange Format' are trademarks (tm) of
// Compuserve, Incorporated, an H&R Block Company.
//
// Release Notes: This file contains a decoder routine for GIF images
// which is similar, structurally, to the original routine by Steve Wilhite.
// It is, however, somewhat noticably faster in most cases.
//            
//-----------------------------------------------------------------------------

//-- Includes -----------------------------------------------------------------

#include <Max.h>
#include <bmmlib.h>
#include <setjmp.h>
#include "gif.h"
#include "gifrc.h"
#include "pixelbuf.h"

//-- Turn on this define for debug printout -----------------------------------

//#define DBG_GIF 1

//-- The following disable various portions for testing -----------------------

//#define DISABLE_GIFOUTLINE

//-- Handy macros -------------------------------------------------------------

#define GIFREAD(ptr,sz) ((fread((void *)ptr, sz, 1, inStream)!=1) ? 0:1) 
#define GIFWRITE(ptr,sz) ((fwrite((void *)ptr, sz, 1, outStream)!=1) ? 0:1)

// Misc GIF support stuff

static char gifsig[] = "GIF87a";
static char *gifsigs[]={"87a","89a",NULL};
static long code_mask[13] = {
     0,
     0x0001, 0x0003,
     0x0007, 0x000F,
     0x001F, 0x003F,
     0x007F, 0x00FF,
     0x01FF, 0x03FF,
     0x07FF, 0x0FFF
     };


//#include <stdarg.h>

//-----------------------------------------------------------------------------
//-- File Class

class File {
     public:
        FILE *stream;
        File(const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
        ~File() { Close(); }
        void Close() { if(stream) fclose(stream); stream = NULL; }
     };

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
     switch (fdwReason) {
         case DLL_PROCESS_ATTACH:
              if (hInst)
                 return(FALSE);
              hInst = hDLLInst;
              break;
         case DLL_PROCESS_DETACH:
              hInst  = NULL;
              break;
         case DLL_THREAD_ATTACH:
              break;
         case DLL_THREAD_DETACH:
              break;
     }
     return TRUE;
}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// GIF Class Description

class GIFClassDesc:public ClassDesc {
     public:
        int           IsPublic     ( ) { return 1; }
        void         *Create       ( BOOL loading=FALSE) { return new BitmapIO_GIF; }
        const TCHAR  *ClassName    ( ) { return GetString(IDS_GIF); }
        SClass_ID     SuperClassID ( ) { return BMM_IO_CLASS_ID; }
        Class_ID      ClassID      ( ) { return Class_ID(GIFCLASSID,0); }
        const TCHAR  *Category     ( ) { return GetString(IDS_BITMAP_IO);  }
};

static GIFClassDesc GIFDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
     return GetString(IDS_GIF_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
     return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
     switch(i) {
        case  0: return &GIFDesc; break;
        default: return 0;        break;
     }
}

__declspec( dllexport ) ULONG LibVersion ( )  { 
     return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

BitmapIO_GIF::BitmapIO_GIF() {
    loadMap = NULL;
	loadStorage = saveStorage = NULL;
	inStream = outStream = NULL;
	badGIFReason = 0;
	bad_code_count = 0;
	gif_line = 0;
	gif_colors = 0;
	iphase = 0;
	iy = 0;
	curr_size = 0;
	clear = 0;
	ending = 0;
	newcodes = 0;
	top_slot = 0;
	slot = 0; 
	navail_bytes = 0;
	nbits_left = 0;
	b1 = 0;
	pbytes = NULL;
	for(int i = 0; i < 259; ++i)
		gif_byte_buff[i] = 0;
	gif_wpt = NULL;
	gif_wcount = 0;
	prior_codes = NULL;
	code_ids = NULL;
	added_chars = NULL;
	code_size = 0;
	clear_code = 0;
	eof_code = 0;
	bit_offset = 0;
	max_code = 0;
	free_code = 0;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::LongDesc()

const TCHAR *BitmapIO_GIF::LongDesc()  {
     return GetString(IDS_GIF_FILE);
}
     
//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::ShortDesc()

const TCHAR *BitmapIO_GIF::ShortDesc() {
     return GetString(IDS_GIF);
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

     switch (message) {
        
        case WM_INITDIALOG: 
             CenterWindow(hWnd,GetParent(hWnd));
             return 1;
        

        case WM_COMMAND:
             switch (LOWORD(wParam)) {
                case IDOK:              
                case IDCANCEL:
                     EndDialog(hWnd,1);
                     break;
             }
             return 1;

     }
     
     return 0;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::ShowAbout()

void BitmapIO_GIF::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)0);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::GetImageInfo()

BMMRES BitmapIO_GIF::GetImageInfo ( BitmapInfo *fbi ) {
     
     //-- Get File Header
     
     File file(fbi->Name(), _T("rb"));

     if(!(inStream = file.stream))
		return (ProcessImageIOError(fbi));

     if(ReadHeader()) {
        fbi->SetWidth(image.w);
        fbi->SetHeight(image.h);
//       fbi->SetGamma (1.0f);
        fbi->SetAspect (1.0f);
        fbi->SetFirstFrame(0);
        fbi->SetLastFrame(0);
   		fbi->SetType(storageType);
        return BMMRES_SUCCESS;
     } else
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

}

int
BitmapIO_GIF::GIFGetByte() {
	uchar buf = 0;
	
	if(!GIFREAD(&buf,1))
		return(READ_ERROR);
	return((unsigned int)buf);
	}

/* Send a line of pixels to the screen */

int
BitmapIO_GIF::GIFOutLine(BYTE *pixels, int linelen) {
#ifndef DISABLE_GIFOUTLINE
	int y;

	y = gif_line;
	if (image.flags&ITLV_BIT)
		{
		y = iy;
		switch (iphase)
			{
			case 0:
			case 1:
				iy+=8;
				break;
			case 2:
				iy += 4;
				break;
			case 3:
				iy += 2;
				break;
			}
		if (iy >= image.h)
			{
			switch (iphase)
				{
				case 0:
					iy = 4;
					break;
				case 1:
					iy = 2;
					break;
				case 2:
					iy = 1;
					break;
				}
			iphase++;
			}
		}
	gif_line++;
	
	/* Now load pixels into our memory */
	
	// Watch out!  If it's a bad file, it might try to overrun our buffer.
	// Just return failure.
	if(y >= image.h || linelen > image.w) {
#ifdef DBG_GIF
		DebugPrint("<BUFFER OVERRUN, y:%d (%d) linelen:%d (%d)>\n", y, image.h, linelen, image.w);
#endif // DBG_GIF
		return IMAGE_BUFFER_FULL;
		}

	if(storageType == BMM_PALETTED)
		loadStorage->PutIndexPixels(0,y,linelen,pixels);
	else {
		PixelBuf buf(linelen);
		BMM_Color_64 *ptr = buf.Ptr();
		for(int i = 0; i < linelen; ++i, ++ptr) {
			BMM_Color_24 *c = &gif_cmap[pixels[i]];
			ptr->r = c->r << 8;
			ptr->g = c->g << 8;
			ptr->b = c->b << 8;
			ptr->a = (pixels[i] == GCB.xparent) ? 0 : 0xffff;
			}
		loadStorage->PutPixels(0,y,linelen, buf.Ptr());
		}
#endif
	return 1;
	}

/* This function initializes the decoder for reading a new image.
 */
WORD
BitmapIO_GIF::InitExp(WORD size)
   {
   curr_size = size + 1;
   top_slot = 1 << curr_size;
   clear = 1 << size;
   ending = clear + 1;
   slot = newcodes = ending + 1;
   navail_bytes = nbits_left = 0;
   return(0);
   }

/* get_next_code()
 * - gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors...
 */
WORD
BitmapIO_GIF::GetNextCode() {
   WORD i, x;
   unsigned long ret;

   if (nbits_left == 0)
      {
      if (navail_bytes <= 0)
         {

         /* Out of bytes in current block, so read next block
          */
         pbytes = gif_byte_buff;
         if ((navail_bytes = GIFGetByte()) < 0)
            return(navail_bytes);
         else if (navail_bytes)
            {
            for (i = 0; i < navail_bytes; ++i)
               {
               if ((x = GIFGetByte()) < 0)
                  return(x);
               gif_byte_buff[i] = (BYTE)x;
               }
            }
         }
      b1 = *pbytes++;
      nbits_left = 8;
      --navail_bytes;
      }

   ret = b1 >> (8 - nbits_left);
   while (curr_size > nbits_left)
      {
      if (navail_bytes <= 0)
         {

         /* Out of bytes in current block, so read next block
          */
         pbytes = gif_byte_buff;
         if ((navail_bytes = GIFGetByte()) < 0)
            return(navail_bytes);
         else if (navail_bytes)
            {
            for (i = 0; i < navail_bytes; ++i)
               {
               if ((x = GIFGetByte()) < 0)
                  return(x);
               gif_byte_buff[i] = (BYTE)x;
               }
            }
         }
      b1 = *pbytes++;
      ret |= b1 << nbits_left;
      nbits_left += 8;
      --navail_bytes;
      }
   nbits_left -= curr_size;
   ret &= code_mask[curr_size];
   return((WORD)(ret));
   }



/* WORD decoder(linewidth)
 *    WORD linewidth;               * Pixels per line of image *
 *
 * - This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to gif_out_line(), which is a user specific function
 * to display a line of pixels.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * seperating them into the proper size codes.  Finally, gif_get_byte() is
 * the global routine to read the next byte from the GIF file.
 *
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 *
 * Returns: 0 if successful, else negative.  (See ERRS.H)
 *
 */

int
BitmapIO_GIF::Decoder(WORD linewidth, BYTE *buf, BYTE *stack, BYTE *suffix, USHORT *prefix)
   {
   BYTE *sp = NULL, *bufptr=NULL;
   WORD code=0, fc=0, oc=0, bufcnt=0;
   WORD c=0, size=0;
   int ret=0;

   /* Initialize for decoding a new image...
    */
   if ((size = GIFGetByte()) < 0)
      return(size);
   if (size < 2 || 9 < size) {
#ifdef DBG_GIF
		DebugPrint("<BAD DECODE SIZE:%d>\n", size);
#endif // DBG_GIF
       return(BAD_CODE_SIZE);
	   }
   InitExp(size);

   /* Initialize in case they forgot to put in a clear code.
    * (This shouldn't happen, but we'll try and decode it anyway...)
    */
   oc = fc = 0;


   /* Set up the stack pointer and decode buffer pointer
    */
   sp = stack;
#ifdef DBG_DIG
   DebugPrint("Initial stack:%p sp:%p\n",stack, sp);
#endif //DBG_GIF
   bufptr = buf;
   bufcnt = linewidth;

   /* This is the main loop.  For each code we get we pass through the
    * linked list of prefix codes, pushing the corresponding "character" for
    * each code onto the stack.  When the list reaches a single "character"
    * we push that on the stack too, and then start unstacking each
    * character for output in the correct order.  Special handling is
    * included for the clear code, and the whole thing ends when we get
    * an ending code.
    */
   while ((c = GetNextCode()) != ending)
      {

      /* If we had a file error, return without completing the decode
       */
      if (c < 0)
         {
         return(c);
         }

      /* If the code is a clear code, reinitialize all necessary items.
       */
      if (c == clear)
         {
         curr_size = size + 1;
         slot = newcodes;
         top_slot = 1 << curr_size;

         /* Continue reading codes until we get a non-clear code
          * (Another unlikely, but possible case...)
          */
         while ((c = GetNextCode()) == clear)
            ;

         /* If we get an ending code immediately after a clear code
          * (Yet another unlikely case), then break out of the loop.
          */
         if (c == ending)
            break;

         /* Finally, if the code is beyond the range of already set codes,
          * (This one had better NOT happen...  I have no idea what will
          * result from this, but I doubt it will look good...) then set it
          * to color zero.
          */
         if (c >= slot)
            c = 0;

         oc = fc = c;

         /* And let us not forget to put the char into the buffer... And
          * if, on the off chance, we were exactly one pixel from the end
          * of the line, we have to send the buffer to the gif_out_line()
          * routine...
          */
         *bufptr++ = (BYTE)c;
         if (--bufcnt == 0)
            {
			if((ret = GIFOutLine(buf, linewidth)) < 0)
			    return(ret);
            bufptr = buf;
            bufcnt = linewidth;
            }
         }
      else
         {

         /* In this case, it's not a clear code or an ending code, so
          * it must be a code code...  So we can now decode the code into
          * a stack of character codes. (Clear as mud, right?)
          */
         code = c;

         /* Here we go again with one of those off chances...  If, on the
          * off chance, the code we got is beyond the range of those already
          * set up (Another thing which had better NOT happen...) we trick
          * the decoder into thinking it actually got the last code read.
          * (Hmmn... I'm not sure why this works...  But it does...)
          */
         if (code >= slot)
            {
            if (code > slot)
               ++bad_code_count;
            code = oc;

			if((sp - stack) <= MAX_CODES)
				*sp++ = (BYTE)fc;
            }

         /* Here we scan back along the linked list of prefixes, pushing
          * helpless characters (ie. suffixes) onto the stack as we do so.
          */
         while (code >= newcodes)
            {
			if((sp - stack) > MAX_CODES)
				break;
			*sp++ = suffix[code];
            code = prefix[code];
            }

         /* Push the last character on the stack, and set up the new
          * prefix and suffix, and if the required slot number is greater
          * than that allowed by the current bit size, increase the bit
          * size.  (NOTE - If we are all full, we *don't* save the new
          * suffix and prefix...  I'm not certain if this is correct...
          * it might be more proper to overwrite the last code...
          */
         if((sp - stack) <= MAX_CODES)
			*sp++ = (BYTE)code;
         if (slot < top_slot)
            {
            fc = code;
            suffix[slot] = (BYTE)code;
            prefix[slot++] = oc;
            oc = c;
            }
         if (slot >= top_slot)
            if (curr_size < 12)
               {
               top_slot <<= 1;
               ++curr_size;
               } 

         /* Now that we've pushed the decoded string (in reverse order)
          * onto the stack, lets pop it off and put it into our decode
          * buffer...  And when the decode buffer is full, write another
          * line...
          */
         while (sp > stack)
            {
            *bufptr++ = *(--sp);
            if (--bufcnt == 0)
				{
				if ((ret = GIFOutLine(buf, linewidth)) < 0)
                    return(ret);
				bufptr = buf;
                bufcnt = linewidth;
                }
            }
         }
      }
   if (bufcnt != linewidth)
       ret = GIFOutLine(buf, (linewidth - bufcnt));
   return(ret);
   }

/* basically just allocate memory for buffers and tables, and then
   call Steve B.'s decoder */
int
BitmapIO_GIF::GIFDecoder(WORD linewidth) {
	PixelBuf8 bufBuf(linewidth+1),stackBuf(MAX_CODES+1),suffixBuf(MAX_CODES+1);
	PixelBuf16 prefixBuf(MAX_CODES+1);
	BYTE *buf = bufBuf.Ptr();
	BYTE *stack = stackBuf.Ptr();
	BYTE *suffix = suffixBuf.Ptr();
	UWORD *prefix = prefixBuf.Ptr();
	for(int i = 0; i <= MAX_CODES; ++i) {
		stackBuf[i] = 0;
		suffixBuf[i] = 0;
		prefixBuf[i] = 0;
		}
	for(i = 0; i <= linewidth; ++i)
		bufBuf[i] = 0;

	int ret;

	ret = OUT_OF_MEMORY;
	if(buf && stack && suffix && prefix)
		ret = Decoder(linewidth,buf,stack,suffix,prefix);
	return(ret);
	}

int
BitmapIO_GIF::ReadHeader() {
	unsigned char c;

	if(fread(&header,sizeof(GIFHeader),1,inStream)!=1) {
		error:
		badGIFReason = GIF_TRUNCATED;
		return 0;
		}
	if(memcmp(header.giftype,gifsig,3)) {
		badGIFReason = GIF_INVALID_FILE;
		return 0;
		}
	
	/* Check for valid GIF signatures */
	
	for(int ix=0; gifsigs[ix]!=NULL; ++ix)
		{
		if(!memcmp(&header.giftype[3],gifsigs[ix],3))
			goto sig_okay;
		}
	badGIFReason = GIF_INVALID_FILE;
	return 0;	

	sig_okay:
	gif_colors = (1<<((header.colpix&PIXMASK)+1));
	if(header.colpix&COLTAB) {
		if(!GIFREAD(gif_cmap,gif_colors*3)) 
			goto error;
		}

#ifdef DBG_GIF
	DebugPrint("Header read OK, W:%d H:%d COLPIX:%X BGCOLOR:%d\n", header.w,header.h,
		header.colpix, header.bgcolor);
#endif // DBG_GIF

	// Reset optional stuff
	GCB.flags = 0;

	for (;;)	/* skip over extension blocks and other junk til get ',' */
	{
		if ((c = GIFGetByte()) == READ_ERROR) 
			goto error;
#ifdef DBG_GIF
			DebugPrint("Got block %X(%d)\n", c, c);
#endif // DBG_GIF
		if (c == ',') {		
#ifdef DBG_GIF
			DebugPrint("<IMAGE FOUND>\n");
#endif // DBG_GIF
			break;
			}
		if (c == ';') {	/* semi-colon is end of piccie */
#ifdef DBG_GIF
			DebugPrint("<END OF PIC -- ERROR>\n");
#endif // DBG_GIF
			goto error;
			}

		if (c == '!')	/* extension block */
		{
			if ((c = GIFGetByte()) == READ_ERROR)	/* skip extension type */
				goto error;
#ifdef DBG_GIF
			DebugPrint("<Found extension block type %X(%d)>\n", c, c);
#endif // DBG_GIF
			if ( c == 0xf9)
			{
#ifdef DBG_GIF
				DebugPrint("<FOUND GRAPHIC CONTROL BLOCK>\n");
#endif // DBG_GIF
				if ((c = GIFGetByte()) == READ_ERROR) goto error;
				if (c != 4)	goto error;		// This is a fixed-length block
				if(!GIFREAD(&GCB,sizeof(GIFGraphicControlBlock)))
					goto error;
#ifdef DBG_GIF
				DebugPrint("GCB flags:%X Delay:%d Xparent:%d\n",GCB.flags,GCB.delay,GCB.xparent);
#endif // DBG_GIF
				for (;;)	 
				{
					if ((c = GIFGetByte()) == READ_ERROR) goto error;
					if (c == 0)	{/* zero 'count' means end of extension */
#ifdef DBG_GIF
						DebugPrint("<END OF EXT BLOCK>\n");
#endif // DBG_GIF
						break;
						}
#ifdef DBG_GIF
					DebugPrint("<SKIPPING EXT BLOCK, %d BYTES>\n", c);
#endif // DBG_GIF
					for(uchar i=0;i<c;i++)
					{
						if (GIFGetByte() == READ_ERROR) goto error;
					}
				}
				continue;				
			}
			else
			{
#ifdef DBG_GIF
				if(c == 0xfe)
					DebugPrint("<SKIPPING COMMENT>\n");
				else
				if(c == 0xff)
					DebugPrint("<SKIPPING APPLICATION EXTENSION>\n");
				else
				if(c == 0x01)
					DebugPrint("<SKIPPING PLAIN TEXT>\n");
				else
					DebugPrint("<SKIPPING UNKNOWN CONTROL BLOCK>\n");
#endif // DBG_GIF
				for (;;)	 
				{
					if ((c = GIFGetByte()) == READ_ERROR) goto error;
					if (c == 0)	{/* zero 'count' means end of extension */
#ifdef DBG_GIF
						DebugPrint("<END OF EXT BLOCK>\n");
#endif // DBG_GIF
						break;
						}
#ifdef DBG_GIF
					DebugPrint("<SKIPPING EXT BLOCK, %d BYTES>\n", c);
#endif // DBG_GIF
					for(uchar i=0;i<c;i++)
					{
						if (GIFGetByte() == READ_ERROR) goto error;
					}
				}
			}
		}
	}
	
#ifdef DBG_GIF
			DebugPrint("<READING IMAGE INFO>\n");
#endif // DBG_GIF
	if(!GIFREAD(&image,sizeof(GIFImage)))
		goto error;

	// Determine the storage type
	storageType = BMM_PALETTED;	//default
	if(GCB.flags & GCB_FLAGS_XPARENT) {
#ifdef DBG_GIF
		DebugPrint("<IMAGE HAS ALPHA, USING TRUECOLOR>\n");
#endif // DBG_GIF
		storageType = BMM_TRUE_32;
		}
	return 1;
	}

BitmapStorage * 
BitmapIO_GIF::LoadGIFStuff(BitmapInfo *fbi, BitmapManager *manager ){
	BitmapStorage *s = NULL;
		
	gif_line = 0;
	iphase=0;
	iy=0;
	
	if(image.flags&COLTAB) {
		gif_colors = (1<<((image.flags&PIXMASK)+1));
#ifdef DBG_GIF
	DebugPrint("Image has color table, %d colors\n", gif_colors);
#endif // DBG_GIF
		if(!GIFREAD(gif_cmap,gif_colors*3)) {
#ifdef DBG_GIF
			DebugPrint("<ERROR READING COLOR MAP>\n");
#endif // DBG_GIF
TRUNCOUT:
			badGIFReason = GIF_INVALID_FILE;
			if(s)
			   delete s;
			return NULL;
			}
		}
	
	assert(fbi);
//	if (!fbi) return(-5);

     //-- Create a storage for this bitmap ------------------------------------

	s = BMMCreateStorage(manager,storageType);

     if(!s)
        return NULL;

	if(storageType != BMM_PALETTED)
		fbi->SetFlags(MAP_HAS_ALPHA);

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        if(s)
           delete s;
        return NULL;
     }

	loadStorage = s;
	int GDResult = GIFDecoder(image.w);
	switch(GDResult)
		{
		case READ_ERROR:
#ifdef DBG_GIF
			DebugPrint("<READ_ERROR>\n");
#endif // DBG_GIF
			goto TRUNCOUT;
		case BAD_CODE_SIZE:
#ifdef DBG_GIF
			DebugPrint("<BAD_CODE_SIZE>\n");
#endif // DBG_GIF
			goto TRUNCOUT;
		case OUT_OF_MEMORY:
#ifdef DBG_GIF
			DebugPrint("<OUT_OF_MEMORY>\n");
#endif // DBG_GIF
			badGIFReason = GIF_OUT_OF_MEMORY;
			delete s;
			return NULL;
		default:
			break;
		}
	
	if(storageType == BMM_PALETTED) {
		// The load went OK!  Make the color palette 64 bits and stuff it into the storage
		PixelBuf48 palette(256);
		BMM_Color_48 *palout = palette.Ptr();
		BMM_Color_24 *palin = &gif_cmap[0];
		for(int i = 0; i < gif_colors; ++i,++palin,++palout) {
			palout->r = (USHORT)palin->r << 8;
			palout->g = (USHORT)palin->g << 8;
			palout->b = (USHORT)palin->b << 8;
			}
		s->SetPalette(0, gif_colors, palette.Ptr());
		}
	return s;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::Load()

BitmapStorage *BitmapIO_GIF::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

     openMode = BMM_OPEN_R;
     loadMap  = map;
     
     BitmapStorage *s = ReadGIFFile(fbi, map->Manager(), status);

     if(!s)
        return NULL;

     //-- Set the storage's BitmapInfo

     s->bi.CopyImageInfo(fbi);

     return s;
}

//-----------------------------------------------------------------------------
// *> BitmapIO_GIF::ReadGIFFile()
//
//    Load an TGA file, returning the storage location

BitmapStorage *BitmapIO_GIF::ReadGIFFile(BitmapInfo *fbi, BitmapManager *manager, BMMRES *status) {

     BitmapStorage *s = NULL;

     File file(fbi->Name(), _T("rb"));

    // Init all class vars
	loadMap = NULL;
	loadStorage = saveStorage = NULL;
	inStream = outStream = NULL;
	badGIFReason = 0;
	bad_code_count = 0;
	gif_line = 0;
	gif_colors = 0;
	iphase = 0;
	iy = 0;
	curr_size = 0;
	clear = 0;
	ending = 0;
	newcodes = 0;
	top_slot = 0;
	slot = 0; 
	navail_bytes = 0;
	nbits_left = 0;
	b1 = 0;
	pbytes = NULL;
	for(int i = 0; i < 259; ++i)
		gif_byte_buff[i] = 0;
	gif_wpt = NULL;
	gif_wcount = 0;
	prior_codes = NULL;
	code_ids = NULL;
	added_chars = NULL;
	code_size = 0;
	clear_code = 0;
	eof_code = 0;
	bit_offset = 0;
	max_code = 0;
	free_code = 0;

	if(!(inStream = file.stream)) {
		*status = ProcessImageIOError(fbi);
        return NULL;
	}

     if (ReadHeader()) {

        fbi->SetWidth (image.w);
        fbi->SetHeight(image.h);
//      fbi->SetGamma (1.0f);
        fbi->SetAspect(1.0f);
        fbi->SetFirstFrame(0);
        fbi->SetLastFrame(0);
   		fbi->SetType(storageType);
#ifdef DBG_GIF
	DebugPrint("Image info: W:%d H:%d FLAGS:%X\n", image.w,image.h,image.flags);
#endif // DBG_GIF

		s = LoadGIFStuff(fbi, manager);
		if(s)
			return s;
		
		//-- If we get here, something went wrong!

		*status = ProcessImageIOError(fbi,BMMRES_INVALIDFORMAT);
		return NULL;
     }

     //-- If we get here, something went wrong!

	 *status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
     return NULL;

}


//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::OpenOutput()

BMMRES BitmapIO_GIF::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
     //-- Save Image Info Data

     bi.CopyImageInfo(fbi);    

     this->map   = map;
     openMode    = BMM_OPEN_W;

     return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::Write()
//

BMMRES BitmapIO_GIF::Write(int frame) {
     
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

     //-- Resolve Filename --------------------------------

     TCHAR filename[MAX_PATH];

     if (frame == BMM_SINGLEFRAME) {
        _tcscpy(filename,bi.Name());
     } else {
        if (!BMMCreateNumberedFilename(bi.Name(),frame,filename)) {
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
        }
     }
     
     //-- Create Image File -------------------------------
     
     File file(filename, _T("wb"));
     
     if (!file.stream) {
		return (ProcessImageIOError(&bi));
     }

	// Below this line formatted for Tom's editor (sorry.)

	outStream = file.stream;

	// Find out what kind of output file we're dealing with

	saveStorage = map->Storage();
	if(!saveStorage)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	int result = SaveGIF(map);

	switch(result) {
		case 1:
			return BMMRES_SUCCESS;
		case 0:
		default:
			return (ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR)));
		}

}

/*----------------------------------------------------------------------*/
/* Copyright (c) 1987							*/
/* by CompuServe Inc., Columbus, Ohio.  All Rights Reserved		*/
/*----------------------------------------------------------------------*/

/*
 * ABSTRACT:
 *	The compression algorithm builds a string translation table that maps
 *	substrings from the input string into fixed-length codes.  These codes
 *	are used by the expansion algorithm to rebuild the compressor's table
 *	and reconstruct the original data stream.  In it's simplest form, the
 *	algorithm can be stated as:
 *
 *		"if <w>k is in the table, then <w> is in the table"
 *
 *	<w> is a code which represents a string in the table.  When a new
 *	character k is read in, the table is searched for <w>k.  If this
 *	combination is found, <w> is set to the code for that combination
 *	and the next character is read in.  Otherwise, this combination is
 *	added to the table, the code <w> is written to the output stream and
 *	<w> is set to k.
 *
 *	The expansion algorithm builds an identical table by parsing each
 *	received code into a prefix string and suffix character.  The suffix
 *	character is pushed onto the stack and the prefix string translated
 *	again until it is a single character.  This completes the expansion.
 *	The expanded code is then output by popping the stack and a new entry
 *	is made in the table.
 *
 *	The algorithm used here has one additional feature.  The output codes
 *	are variable length.  They start at a specified number of bits.  Once
 *	the number of codes exceeds the current code size, the number of bits
 *	in the code is incremented.  When the table is completely full, a
 *	clear code is transmitted for the expander and the table is reset.
 *	This program uses a maximum code size of 12 bits for a total of 4096
 *	codes.
 *
 *	The expander realizes that the code size is changing when it's table
 *	size reaches the maximum for the current code size.  At this point,
 *	the code size in increased.  Remember that the expander's table is
 *	identical to the compressor's table at any point in the original data
 *	stream.
 *
 *	The compressed data stream is structured as follows:
 *		first byte denoting the minimum code size
 *		one or more counted byte strings. The first byte contains the
 *		length of the string. A null string denotes "end of data"
 *
 *	This format permits a compressed data stream to be embedded within a
 *	non-compressed context.
 *
 * AUTHOR: Steve Wilhite
 *
 * REVISION HISTORY:
 *   Speed tweaked a bit by Jim Kent 8/29/88
 *
 */

void
BitmapIO_GIF::InitTable(short min_code_size)
    {
    code_size = min_code_size + 1;
    clear_code = 1 << min_code_size;
    eof_code = clear_code + 1;
    free_code = clear_code + 2;
    max_code = 1 << code_size;

    memset(code_ids, 0, TABLE_SIZE * sizeof(short));
    }

void
BitmapIO_GIF::Flush(short n) {
	if (!GIFWRITE(&n,1))
		longjmp(recover, -3);
	if(n) {
		if(!GIFWRITE(gif_byte_buff, n))
		longjmp(recover, -3);
		}
	}

void
BitmapIO_GIF::WriteCode(short code) {
	long temp;
	register short byte_offset; 
	register short bits_left;

	byte_offset = bit_offset >> 3;
	bits_left = bit_offset & 7;

	if (byte_offset >= 254)
		{
		Flush(byte_offset);
		gif_byte_buff[0] = gif_byte_buff[byte_offset];
		bit_offset = bits_left;
		byte_offset = 0;
		}

	if (bits_left > 0)
		{
		temp = ((long) code << bits_left) | gif_byte_buff[byte_offset];
		gif_byte_buff[byte_offset] = (BYTE)temp;
		gif_byte_buff[byte_offset + 1] = (BYTE)(temp >> 8);
		gif_byte_buff[byte_offset + 2] = (BYTE)(temp >> 16);
		}
	else
		{
		gif_byte_buff[byte_offset] = (BYTE)code;
		gif_byte_buff[byte_offset + 1] = (BYTE)(code >> 8);
		}
	bit_offset += code_size;
	}


/*
 * Function:
 *	Compress a stream of data bytes using the LZW algorithm.
 *
 * Inputs:
 *	min_code_size
 *		the field size of an input value.  Should be in the range from
 *		1 to 9.
 *
 * Returns:
 *	 0	normal completion
 *	-1	(not used)
 *	-2	insufficient dynamic memory
 *	-3	bad "min_code_size"
 *	< -3	error status from either the get_byte or put_byte routine
 */
/*static*/
short
BitmapIO_GIF::CompressData(int min_code_size) {
    short status;
	short prefix_code;
	short d;
	register int hx;
	register short suffix_char;

    if (min_code_size < 2 || min_code_size > 9)
	if (min_code_size == 1)
	    min_code_size = 2;
	else
	    return -3;

    status = setjmp(recover);

    if (status != 0)
		return status;

    bit_offset = 0;
    InitTable(min_code_size);
    WriteCode(clear_code);
    suffix_char = *gif_wpt++;
	gif_wcount -= 1;

	prefix_code = suffix_char;

	while (--gif_wcount >= 0)
	    {
		suffix_char = *gif_wpt++;
		hx = prefix_code ^ suffix_char << 5;
	    d = 1;

	    for (;;)
		{
		if (code_ids[hx] == 0)
		    {
		    WriteCode(prefix_code);

		    d = free_code;

		    if (free_code <= LARGEST_CODE)
			{
			prior_codes[hx] = prefix_code;
			added_chars[hx] = (BYTE)suffix_char;
			code_ids[hx] = free_code;
			free_code++;
			}

		    if (d == max_code)
			if (code_size < 12)
			    {
			    code_size++;
			    max_code <<= 1;
			    }
			else
			    {
			    WriteCode(clear_code);
			    InitTable(min_code_size);
			    }

		    prefix_code = suffix_char;
		    break;
		    }

		if (prior_codes[hx] == prefix_code &&
			added_chars[hx] == suffix_char)
		    {
			prefix_code = code_ids[hx];
		    break;
		    }

		hx += d;
		d += 2;
		if (hx >= TABLE_SIZE)
		    hx -= TABLE_SIZE;
		}
	    }

	WriteCode(prefix_code);

    WriteCode(eof_code);


    /* Make sure the code buffer is flushed */

    if (bit_offset > 0)
		Flush((bit_offset + 7)/8);

    Flush(0);				/* end-of-data */
    return 0;
    }

short
BitmapIO_GIF::GIFCompressData(int min_code_size) {
	int ret;

	ret = -2;	/* out of memory default */
	prior_codes = code_ids = NULL;
	added_chars = NULL;
	
	PixelBuf16 pc(TABLE_SIZE), cid(TABLE_SIZE);
	PixelBuf8 ach(TABLE_SIZE);
	prior_codes = (short *)pc.Ptr();
	code_ids = (short *)cid.Ptr();
	added_chars = ach.Ptr();

	if(prior_codes && code_ids && added_chars)
		ret = CompressData(min_code_size);

	return(ret);
	}

int
BitmapIO_GIF::SaveGIF(Bitmap *map) {
	int i;
	BMM_Color_24 c;
	static char eight=8,semicolon=';';
	
	memset(&header, 0, sizeof(GIFHeader));
	
	strcpy(header.giftype, gifsig);
	header.w = image.w = saveStorage->Width();
	header.h = image.h = saveStorage->Height();
	image.x = image.y = image.flags = 0;
	header.colpix = COLPIXVGA13;
	if(!GIFWRITE(&header, sizeof(GIFHeader))) {
		TRUNCOUT:
		BADOUT:
		badGIFReason = GIF_WRITE_ERROR;
		return(0);
		}

	// Set up a buffer for indexed pixels!
	// (We're going to string all the pixels in the image together in one big buffer and zap 'em out at one time)
	gif_wcount = (int)header.w * (int)header.h;

	PixelBuf8 imagebuf(gif_wcount);
	BYTE *ibuf = imagebuf.Ptr();
	if(!ibuf)
		goto TRUNCOUT;
	
	// Set the pointer used by the write function
	gif_wpt = ibuf;

	// Set up a palette buffer
	PixelBuf48 palettebuf(256);
	BMM_Color_48 *pal = palettebuf.Ptr();

	// See if the storage is paletted.  If it is, we can just rock 'n' roll!,
	if(saveStorage->Paletted()) {
		saveStorage->GetPalette(0, 256, pal);
		BYTE *wptr = ibuf;
		for(int y = 0; y < header.h; ++y,wptr += header.w)
			saveStorage->GetIndexPixels(0,y,header.w,wptr);
		}
	else {	// Arrggh.  Must compute a color palette, and reduce the image to 256 colors!
		// this calculates a palette based on the gamma corrected values, which
		// corresponds to what GetOutputPixels returns.
		if(CalcOutputPalette(256,pal) == 0)
			return 0;
		PixelBuf64 line(header.w);
        BYTE *pixel = ibuf;
		ColorPacker* cpack = BMMNewColorPacker(header.w,pal,256);
        int y;
        for(y=0; y<header.h; ++y) {
           if(!GetOutputPixels(0,y,header.w,line.Ptr()))
              return 0;
		   cpack->PackLine(line.Ptr(),pixel,header.w);
		   pixel +=	header.w;
           }
		cpack->DeleteThis();
		}

	/* write global color map */
	
	for (i=0; i<256; i++)
		{
		c.r = (BYTE)(pal[i].r >> 8);
		c.g = (BYTE)(pal[i].g >> 8);
		c.b = (BYTE)(pal[i].b >> 8);
		if(!GIFWRITE(&c,3))
			goto TRUNCOUT;
		}
	if(!GIFWRITE(",",1))
		goto TRUNCOUT;
	
	if(!GIFWRITE(&image,sizeof(image)))
		goto TRUNCOUT;
	if(!GIFWRITE(&eight,1))
		goto TRUNCOUT;
	if(fflush(outStream))
		goto TRUNCOUT;
	i = GIFCompressData(8);
	switch (i)
		{
		case -2:
//			continu_line(progstr(STR2116));
			goto BADOUT;
		case -3:
			goto TRUNCOUT;
		default:
			break;
		}
	if(!GIFWRITE(&semicolon,1))
		goto TRUNCOUT;
	return(1);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_GIF::Close()
//

int  BitmapIO_GIF::Close( int flag ) {
     if(openMode != BMM_OPEN_W)
        return 0;
     return 1;
}

//-- EOF: gif.cpp -----------------------------------------------------------
