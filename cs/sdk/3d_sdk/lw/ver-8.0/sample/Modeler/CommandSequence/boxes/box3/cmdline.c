/*
======================================================================
cmdline.c

Process the command line for our box plug-in.

Ernie Wright  10 Jun 01
====================================================================== */

#include <lwdyna.h>
#include <stdlib.h>
#include <string.h>


/*
======================================================================
free_argv()

Free memory allocated by get_argv().
====================================================================== */

static void free_argv( int argc, char **argv )
{
   int i;

   if ( argv ) {
      for ( i = 0; i < argc; i++ )
         if ( argv[ i ] ) free( argv[ i ] );
      free( argv );
   }
}


/*
======================================================================
strdup()

Duplicate a string.  Some C runtimes provide this function, but some
don't.
====================================================================== */

static char *strdup( const char *str )
{
   char *dup;

   if ( !str ) return NULL;
   dup = malloc( strlen( str ) + 1 );
   if ( dup )
      strcpy( dup, str );
   return dup;
}


/*
======================================================================
get_argv()

Tokenize a command line, copying each token into an element of a
string array.  Tokens are separated by one or more spaces.  Arguments
that contain spaces can be enclosed in double quotes.  Characters
inside angle brackets (< and >) are part of a single vector token.
====================================================================== */

static char **get_argv( const char *cmdline, int maxarg, int *argc )
{
   enum { TOK_SKIP, TOK_ADD, TOK_END };
   int ntok = 0, i, j = 0, end, intok = 0, inquote = 0, inbracket = 0, op;
   char buf[ 128 ], **argv;


   argv = calloc( maxarg, sizeof( char * ));
   if ( !argv ) return NULL;

   end = strlen( cmdline );

   for ( i = 0; i < end && ntok < maxarg; i++ ) {
      switch ( cmdline[ i ] )
      {
         case ' ':
            if ( !intok )
               op = TOK_SKIP;
            else if ( inquote || inbracket )
               op = TOK_ADD;
            else
               op = TOK_END;
            break;

         case '"':
            if ( inbracket )
               op = TOK_ADD;
            else if ( inquote ) {
               op = TOK_END;
               inquote = 0;
            }
            else {
               op = TOK_SKIP;
               intok = inquote = 1;
            }
            break;

         case '<':
            if ( inquote )
               op = TOK_ADD;
            else {
               op = TOK_SKIP;
               intok = inbracket = 1;
            }
            break;

         case '>':
            if ( inquote )
               op = TOK_ADD;
            else {
               op = TOK_END;
               inbracket = 0;
            }
            break;

         default:
            op = TOK_ADD;
            break;
      }

      switch ( op )
      {
         case TOK_ADD:
            buf[ j++ ] = cmdline[ i ];
            break;

         case TOK_END:
            buf[ j ] = 0;
            argv[ ntok ] = strdup( buf );
            j = 0;
            intok = 0;
            ++ntok;
            break;

         default:
            break;
      }
   }

   *argc = ntok;
   return argv;
}


/*
======================================================================
parse_cmdline()

Extract box parameters from a command line.
====================================================================== */

int parse_cmdline( DynaConvertFunc *convert, const char *cmdline,
   double *size, double *center, char *surfname, char *vmapname )
{
   DynaValue from = { DY_STRING }, to = { DY_VDIST };
   int argc;
   char **argv;

   argv = get_argv( cmdline, 4, &argc );

   if ( argc == 4 ) {
      from.str.buf = argv[ 0 ];
      to.fvec.defVal = 1.0;
      convert( &from, &to, NULL );

      size[ 0 ] = to.fvec.val[ 0 ];
      size[ 1 ] = to.fvec.val[ 1 ];
      size[ 2 ] = to.fvec.val[ 2 ];

      from.str.buf = argv[ 1 ];
      to.fvec.defVal = 0.0;
      convert( &from, &to, NULL );

      center[ 0 ] = to.fvec.val[ 0 ];
      center[ 1 ] = to.fvec.val[ 1 ];
      center[ 2 ] = to.fvec.val[ 2 ];

      strcpy( surfname, argv[ 2 ] );
      strcpy( vmapname, argv[ 3 ] );
   }

   free_argv( argc, argv );
   return ( argc == 4 );
}
