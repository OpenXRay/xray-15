//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  Doc.cpp
//  A class to contain document references. This is just a shell until
//  a real http protocol library is found...
//

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Doc.h"
#include "System.h"

#if defined AW_NEW_IOSTREAMS
#  include <fstream>
#else
#  include <fstream.h>
#endif


Doc::Doc(const char *url, Doc *relative) :
  d_url(0),
  d_ostream(0),
  d_fp(0),
#if HAVE_LIBPNG || HAVE_ZLIB
  d_gz(0),
#endif
  d_tmpfile(0)
{
  if ( url )
    seturl(url, relative);
}

Doc::Doc(Doc *doc) :
  d_url(0),
  d_ostream(0),
  d_fp(0),
#if HAVE_LIBPNG || HAVE_ZLIB
  d_gz(0),
#endif
  d_tmpfile(0)
{
  if ( doc )
    seturl( doc->url() );
}

Doc::~Doc()
{
  delete [] d_url;
  delete d_ostream;
  if (d_tmpfile)
    {
      theSystem->removeFile(d_tmpfile);
      delete [] d_tmpfile;
      d_tmpfile = 0;
    }
}


void Doc::seturl(const char *url, Doc *relative)
{
  delete [] d_url;
  d_url = 0;

  if (url)
    {
      const char *path = "";

      if ( relative && ! isAbsolute(url) )
	path = relative->urlPath();

      d_url = new char[strlen(path) + strlen(url) + 1];
      strcpy(d_url, path);
      strcat(d_url, url);
    }
}


const char *Doc::url() { return d_url; }

const char *Doc::urlBase() 
{
  if (! d_url) return "";

  static char path[1024];
  char *p, *s = path;
  strncpy(path, d_url, sizeof(path)-1);
  path[sizeof(path)-1] = '\0';
  if ((p = strrchr(s, SLASH)) != 0)
    s = p+1;
  else if ((p = strchr(s, COLON)) != 0)
    s = p+1;

  if ((p = strrchr(s, '.')) != 0)
    *p = '\0';
  
  return s;
}

const char *Doc::urlExt() 
{
  if (! d_url) return "";

  static char ext[20];
  char *p;

  if ((p = strrchr(d_url, '.')) != 0)
    {
      strncpy(ext, p+1, sizeof(ext)-1);
      ext[sizeof(ext)-1] = '\0';
    }
  else
    ext[0] = '\0';
  
  return &ext[0];
}

const char *Doc::urlPath() 
{
  if (! d_url) return "";

  static char path[1024];

  strcpy(path, d_url);
  char *slash;
  if ((slash = strrchr(path, SLASH)) != 0)
    *(slash+1) = '\0';
  else
    path[0] = '\0';
  return &path[0]; 
}


const char *Doc::urlProtocol()
{
  if (d_url)
    {
      static char protocol[12];
      const char *s = d_url;

#ifdef _WIN32
      if (strncmp(s+1,":\\",2) == 0) return "file";
#endif

      for (unsigned int i=0; i<sizeof(protocol); ++i, ++s)
	{
	  if (*s == 0 || ! isalpha(*s))
	    {
	      protocol[i] = '\0';
	      break;
	    }
	  protocol[i] = tolower(*s);
	}
      protocol[sizeof(protocol)-1] = '\0';
      if (*s == COLON)
	return protocol;
    }
    
  return "file";
}

const char *Doc::urlModifier()
{
  char *mod = d_url ? strrchr(d_url,'#') : 0;
  return mod ? mod : "";
}

const char *Doc::localName()
{
  static char buf[1024];
  if (filename(buf, sizeof(buf)))
    return &buf[0];
  return 0;
}

const char *Doc::localPath()
{
  static char buf[1024];
  if (filename(buf, sizeof(buf)))
    {
      char *s = strrchr(buf, SLASH);
      if (s) *(s+1) = '\0';
      return &buf[0];
    }
  return 0;
}

const char *Doc::stripProtocol(const char *url)
{
  const char *s = url;

#ifdef _WIN32
  if (strncmp(s+1,":\\",2) == 0) return url;
#endif

  // strip off protocol if any
  while (*s && isalpha(*s)) ++s;

  if (*s == COLON)
    return s + 1;

  return url;
}


// Converts a url into a local filename

bool Doc::filename( char *fn, int nfn )
{
  fn[0] = '\0';

  char *e = 0, *s = (char*) stripProtocol(d_url);

  if ((e = strrchr(s,'#')) != 0)
    *e = '\0';

  const char *protocol = urlProtocol();

  // Get a local copy of http files
  if (strcmp(protocol, "http") == 0)
    {
      if (d_tmpfile)		// Already fetched it
	s = d_tmpfile;
      else if ((s = (char *) theSystem->httpFetch(d_url)))
	{
	  d_tmpfile = new char[strlen(s)+1];
	  strcpy(d_tmpfile, s);
	  free(s);		// assumes tempnam or equiv...
	  s = d_tmpfile;
	}
    }

  // Unrecognized protocol (need ftp here...)
  else if (strcmp(protocol, "file") != 0)
    s = 0;

  if (s)
    {
      strncpy( fn, s, nfn-1 );
      fn[nfn-1] = '\0';
    }

  if (e) *e = '#';

  return s && *s;
}

// Having both fopen and outputStream is dumb...

FILE *Doc::fopen(const char *mode)
{
  if (d_fp)
    theSystem->error("Doc::fopen: %s is already open.\n", d_url ? d_url : "");

  char fn[256];
  if (filename(fn, sizeof(fn)))
    {
      if (strcmp(fn, "-") == 0)
	{
	  if (*mode == 'r')
	    d_fp = stdin;
	  else if (*mode == 'w')
	    d_fp = stdout;
	}
      else
	{
	  d_fp = ::fopen( fn, mode );
	}
    }

  return d_fp;
}

void Doc::fclose()
{
  if (d_fp && (strcmp(d_url, "-") != 0) && (strncmp(d_url, "-#", 2) != 0))
    ::fclose(d_fp);

  d_fp = 0;
  if (d_tmpfile)
    {
      theSystem->removeFile(d_tmpfile);
      delete [] d_tmpfile;
      d_tmpfile = 0;
    }
}

#if HAVE_LIBPNG || HAVE_ZLIB

// For (optionally) gzipped files

gzFile Doc::gzopen(const char *mode)
{
  if (d_fp || d_gz)
    theSystem->error("Doc::gzopen: %s is already open.\n", d_url ? d_url : "");

  char fn[256];
  if (filename(fn, sizeof(fn)))
    {
      d_gz = ::gzopen( fn, mode );
    }

  return d_gz;
}

void Doc::gzclose()
{
  if (d_gz)
    ::gzclose(d_gz);

  d_gz = 0;
  if (d_tmpfile)
    {
      theSystem->removeFile(d_tmpfile);
      delete [] d_tmpfile;
      d_tmpfile = 0;
    }
}

#endif

ostream &Doc::outputStream()
{
  d_ostream = new ofstream(stripProtocol(d_url), ios::out);
  return *d_ostream;
}


bool Doc::isAbsolute(const char *url)
{
  const char *s = stripProtocol(url);
  return ( *s == SLASH || *(s+1) == ':' );
}

    
