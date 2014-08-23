//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  System.cpp
//  A class to contain system-dependent/non-standard utilities
//

#include "config.h"
#include "System.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// A default System object
static System defaultSystem;

// The global System object
System *theSystem = &defaultSystem;


System::~System() {}

// Should make these iostream objects...

void System::error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void System::warn(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr,"Warning: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}


// Write to the browser status line...

void System::inform(const char *fmt, ...)
{
  static char lastbuf[1024] = { 0 };
  char buf[1024];

  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);
  if (strcmp(lastbuf, buf))
    {
      fprintf(stderr,"%s", buf);
      putc('\n', stderr);
      strcpy(lastbuf, buf);
    }
}

#if DEBUG
void System::debug(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
#else
void System::debug(const char *, ...)
{
#endif
}

#if defined (_WIN32) || defined (OSMac_)

// Windows
#	ifdef _WIN32
#	include <sys/timeb.h>
#	endif //_WIN32

#include <time.h>
#include <string.h>

#ifdef OSMac_
struct _timeb {
	time_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};

void _ftime(struct _timeb *);
#endif // OSMac_

double System::time()	
{
   struct _timeb timebuffer;
#ifndef OSMac_ //MAYAMACTODO
   _ftime( &timebuffer );
#endif
	return (double) timebuffer.time + 1.e-3 * (double) timebuffer.millitm;
}

#else

// Unix/Linux

#include <sys/time.h>
#include <unistd.h>


double System::time()	
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);

  return (double) tv.tv_sec + 1.e-6 * (double) tv.tv_usec;
}

#endif // _WIN32

// This won't work under windows or if netscape isn't running...

bool System::loadUrl(const char *url, int np, char **parameters )
{
  if (! url) return false;
#if 0
  char buf[1024];
  if (np)
    sprintf(buf,"/bin/csh -c \"netscape -remote 'openURL(%s, %s)'\" &",
	    url, parameters[0]);
  else
    sprintf(buf,"/bin/csh -c \"netscape -remote 'openURL(%s)'\" &", url);
  return system(buf) != -1;
#else
  return false;
#endif // _WIN32
}
  

// This won't work under windows...
#if 0
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>		// memset
#endif

int System::connectSocket( const char *host, int port )
{
#if 1
  return -1;
#else
  struct sockaddr_in sin;
  struct hostent *he;

  int sockfd = -1;

  memset( &sin, 0, sizeof(sin) );
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  // Check for dotted number format
  char *s;
  for (s=(char*)host; *s; ++s)
    if ( ! (isdigit(*s) || *s == '.') ) break;

  if (*s)			// Not dotted number
    he = gethostbyname(host);
  else
    {
      u_long addr;
      addr = inet_addr(host);
      he = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    }

  if (he)
    {
      memcpy((char *)&sin.sin_addr, he->h_addr, he->h_length);
      sockfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
      if (sockfd != -1)
	if (connect( sockfd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
	  {
	    close( sockfd );
	    sockfd = -1;
	  }
    }

  return sockfd;
#endif
}


#include <fcntl.h>		// open() modes
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>

const char *System::httpHost( const char *url, int *port )
{
  static char hostname[256];
  const char *s = strstr(url,"//");
  char *p = hostname;

  if (s)
    {
      for (s+=2; *s && *s != '/' && *s != ':'; *p++ = *s++)
	/* empty */ ;
      if (*s == ':' && 
#if defined(OSMac_)
		  std::isdigit(*(s+1)))
#else
		  isdigit(*(s+1)))
#endif
	*port = atoi(s+1);
    }
  *p = '\0';
  return hostname;
}

// This isn't particularly robust or complete...

const char *System::httpFetch( const char *url )
{
#if 1
  return 0;
#else
  int port = 80;
  const char *hostname = httpHost(url, &port);

  if (port == 80)
    System::inform("Connecting to %s ...", hostname);
  else
    System::inform("Connecting to %s:%d ...", hostname, port);

  int sockfd;
  if ((sockfd = System::connectSocket( hostname, port )) != -1)
    System::inform("connected.");
  else
    System::warn("Connect failed: %s (errno %d).\n",
		 strerror(errno), errno);

  // Copy to a local temp file
  char *result = 0;
  if (sockfd != -1 && (result = tempnam(0, "VR")))
    {
      int fd = open(result, O_RDWR|O_CREAT, 0777);
      if (fd != -1)
	{
	  char *abspath = strstr((char *)url, "//");
	  if (abspath) abspath = strchr(abspath+2, '/');
	  if (! abspath) abspath = (char*) url;

	  char request[1024];
	  sprintf(request,"GET %s HTTP/1.0\nAccept: */*\n\r\n", abspath);

	  int nbytes = strlen(request);
	  if (write(sockfd, request, nbytes) != nbytes)
	    System::warn("http GET failed: %s (errno %d)\n",
			 strerror(errno), errno);
	  else
	    {
	      int gothdr = 0, nread = 0, nwrote = 0, nmore;
	      char *start;

	      while ((nmore = read(sockfd, request, sizeof(request)-1)) > 0)
		{
		  nread += nmore;

		  // Skip header (should read return code, content-type...)
		  if (gothdr)
		    start = request;
		  else
		    {
		      start = strstr(request, "\r\n\r\n");
		      if (start)
			start += 4;
		      else
			{
			  start = strstr(request, "\n\n");
			  if (start) start += 2;
			}
		      if (! start) continue;
		      gothdr = 1;
		    }

		  nmore -= (start - request);
		  if (write(fd, start, nmore) != nmore)
		    {
		      System::warn("http: temp file write error\n");
		      break;
		    }
		  nwrote += nmore;
		}

	      System::inform("Read %dk from %s", (nread+1023)/1024, url);
	      //System::debug("Wrote %d bytes to %s\n", nread, result);
	    }

	  close(fd);
	}
    }

  if (sockfd != -1) close(sockfd);
  return result;
#endif
}


void System::removeFile( const char *fn )
{
  remove(fn);
}
