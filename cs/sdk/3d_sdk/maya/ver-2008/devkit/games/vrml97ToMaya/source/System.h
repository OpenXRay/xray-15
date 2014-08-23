//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef SYSTEM_H
#define SYSTEM_H
//
//  System dependent utilities class
//

class System {

public:
  virtual ~System();

  virtual double time();

  virtual void error(const char *, ...);

  virtual void warn(const char *, ...);

  virtual void inform(const char *, ...);

  virtual void debug(const char *, ...);

  virtual bool loadUrl(const char *url, int np, char **parameters );

  virtual int connectSocket( const char *host, int port );

  virtual const char *httpHost(const char *url, int *port);
  virtual const char *httpFetch( const char *url );

  virtual void removeFile( const char *fn );

};

extern System *theSystem;

#endif // SYSTEM_H
