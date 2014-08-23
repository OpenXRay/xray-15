//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef _DOC_
#define _DOC_
//
//  Document class
//

// Need to get rid of stdio (used by flex)...
//struct FILE; gcc doesn't like this so #include <stdio.h> is necessary
#include <stdio.h>
#include "config.h"

#if defined AW_NEW_IOSTREAMS
#  include <iosfwd>
#else
#  include <iostream.h>  // for IRIX - no iosfwd.h, just use the big header
#endif


#if HAVE_LIBPNG || HAVE_ZLIB
# include "zlib.h"
#endif

class Doc {

public:

  Doc(const char *url = 0, Doc *relative = 0);
  Doc(Doc *);
  ~Doc();

  void seturl(const char *url, Doc *relative = 0);

  const char *url();		// "http://www.foo.com/dir/file.xyz#Viewpoint"
  const char *urlBase();	// "file" or ""
  const char *urlExt();		// "xyz" or ""
  const char *urlPath();	// "http://www.foo.com/dir/" or ""
  const char *urlProtocol();	// "http"
  const char *urlModifier();	// "#Viewpoint" or ""

  const char *localName();	// "/tmp/file.xyz" or NULL
  const char *localPath();	// "/tmp/" or NULL


  FILE *fopen(const char *mode);
  void fclose();

#if HAVE_LIBPNG || HAVE_ZLIB
  // For (optionally) compressed files
  gzFile gzopen(const char *mode);
  void gzclose();
#endif

  ostream &outputStream();

protected:

  static const char *stripProtocol(const char *url);
  static bool isAbsolute(const char *url);
  bool filename( char *fn, int nfn );

  char *d_url;
  ostream *d_ostream;
  FILE *d_fp;
#if HAVE_LIBPNG || HAVE_ZLIB
  gzFile d_gz;
#endif
  char *d_tmpfile;		// Local copy of http: files

};
#endif // _DOC_
