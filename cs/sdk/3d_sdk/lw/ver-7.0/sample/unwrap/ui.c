/*
======================================================================
ui.c

User interface for the unwrap SDK sample.

Ernie Wright  21 Aug 01
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwserver.h>
#include <lwrender.h>
#include <lwsurf.h>
#include <lwtxtr.h>
#include <lwhost.h>
#include <lwpanel.h>
#include "unwrap.h"
#include "tree.h"


#define LVL_OBJECT  0
#define LVL_SURFACE 1
#define LVL_TEXTURE 2
#define LVL_TLAYER  3

/* some globals (declared in unwrap.c) */

extern LWItemInfo *iteminfo;
extern LWObjectInfo *objinfo;
extern LWMessageFuncs *msgf;
extern LWSurfaceFuncs *surff;
extern LWTextureFuncs *txtrf;
extern LWPanelFuncs *panf;
extern LWImageList *imglist;
extern LWImageUtil *imgutil;

/* surface channel names for the Surface Functions global */

static char *surfchan[] = {
   SURF_COLR,
   SURF_LUMI,
   SURF_DIFF,
   SURF_SPEC,
   SURF_GLOS,
   SURF_REFL,
   SURF_TRAN,
   SURF_TRNL,
   SURF_RIND,
   SURF_BUMP,
   NULL
};

/* surface channel names for the user */

static char *surfchan_label[] = {
   "Color",
   "Luminosity",
   "Diffuse",
   "Specularity",
   "Glossiness",
   "Reflection",
   "Transparency",
   "Translucency",
   "Refraction Index",
   "Bump"
};


/*
======================================================================
count_imagemaps()

Returns the number of TLT_IMAGE layers in a texture.
====================================================================== */

static int count_imagemaps( LWTextureID tex )
{
   LWTLayerID tlayer;
   int n = 0;

   if ( !tex ) return 0;

   tlayer = txtrf->firstLayer( tex );
   while ( tlayer ) {
      if ( TLT_IMAGE == txtrf->layerType( tlayer ))
         ++n;
      tlayer = txtrf->nextLayer( tex, tlayer );
   }
   return n;
}


/*
======================================================================
count_imagetxtrs()

For a given surface, returns the number of textures with TLT_IMAGE
layers.
====================================================================== */

static int count_imagetxtrs( LWSurfaceID surf )
{
   int i, n = 0;

   if ( !surf ) return 0;
   for ( i = 0; surfchan[ i ]; i++ )
      if ( count_imagemaps( surff->getTex( surf, surfchan[ i ] )))
         ++n;

   return n;
}


/*
======================================================================
count_imagesurfs()

Given an array of surfaces, returns the number of surfaces containing
textures with TLT_IMAGE layers.
====================================================================== */

static int count_imagesurfs( LWSurfaceID *surfid )
{
   int i, n = 0;

   if ( !surfid ) return 0;
   for ( i = 0; surfid[ i ]; i++ )
      if ( count_imagetxtrs( surfid[ i ] ))
         ++n;

   return n;
}


/*
======================================================================
get_image()

Returns the LWImageID of the image applied to a TLT_IMAGE texture
layer.
====================================================================== */

static LWImageID get_image( LWTLayerID tlayer )
{
   LWImageID image;

   txtrf->getParam( tlayer, TXTAG_IMAGE, &image );
   return image;
}


/*
======================================================================
image_name()

Returns the name of the image applied to a TLT_IMAGE texture layer, or
"(none)" if an image hasn't been applied yet.
====================================================================== */

static const char *image_name( LWTLayerID tlayer )
{
   LWImageID image;

   if ( image = get_image( tlayer ))
      return imglist->name( image );
   else
      return "(none)";
}


/*
======================================================================
get_surflist()

Fill in the tree with surfaces containing image maps.  The children of
the surface nodes are textures, and the grandchildren (the leaf nodes)
are texture layers.

Only TLT_IMAGE (image map) texture layers are included.  If a texture
has no image map layers, the texture is excluded, and if a surface has
no textures with image map layers, the whole surface is excluded.

Returns TRUE if successful or FALSE if an error occurs.
====================================================================== */

static int get_surflist( Node *node, LWSurfaceID *surfid )
{
   Node *surf, *tnode;
   LWTextureID tex;
   LWTLayerID tlayer;
   char *p, name[ 200 ];
   int ntex, nlayer, i, j, si, tj, k, proj;


   surf = node->child;

   /* look at all of the surfaces */

   for ( i = 0, si = 0; surfid[ i ]; i++ ) {

      /* if there are no image mapped textures, keep moving */

      ntex = count_imagetxtrs( surfid[ i ] );
      if ( !ntex ) continue;

      /* init surface node, which has texture children */

      if ( !init_node( &surf[ si ], surff->name( surfid[ i ] ), surfid[ i ], ntex ))
         return 0;

      /* look at each of the surface's channels */

      for ( j = 0, tj = 0; surfchan[ j ]; j++ ) {

         /* if the channel isn't textured, keep moving */

         tex = surff->getTex( surfid[ i ], surfchan[ j ] );
         if ( !tex ) continue;

         /* if the texture has no image maps, keep moving */

         nlayer = count_imagemaps( tex );
         if ( !nlayer ) continue;

         /* init texture node, which has texture layer children */

         tnode = &surf[ si ].child[ tj ];
         if ( !init_node( tnode, surfchan_label[ j ], tex, nlayer ))
            return 0;

         /* look at all of the texture layers */

         k = 0;
         tlayer = txtrf->firstLayer( tex );
         while ( tlayer ) {

            /* if the layer is an image map */

            if ( TLT_IMAGE == txtrf->layerType( tlayer )) {

               /* init texture layer node */

               txtrf->getParam( tlayer, TXTAG_PROJ, &proj );
               switch ( proj ) {
                  case TXPRJ_PLANAR:       p = "Planar";       break;
                  case TXPRJ_CYLINDRICAL:  p = "Cylindrical";  break;
                  case TXPRJ_SPHERICAL:    p = "Spherical";    break;
                  case TXPRJ_CUBIC:        p = "Cubic";        break;
                  case TXPRJ_FRONT:        p = "Front";        break;
                  case TXPRJ_UVMAP:        p = "UVMap";        break;
                  default:                 p = "Unknown";
               }
               sprintf( name, "%s: %s", p, image_name( tlayer ));
               if ( !init_node( &tnode->child[ k ], name, tlayer, 0 ))
                  return 0;
               ++k;
            }
            tlayer = txtrf->nextLayer( tex, tlayer );
         }
         ++tj;
      }
      ++si;
   }

   return 1;
}


/*
======================================================================
make_list()

Create a node tree for use with a Panels TREE_CTL.  The control is a
hierarchical list of the selected object's surfaces with image maps.
Displays an error message and returns NULL if no object is currently
selected in Layout's interface, or none of its surfaces contains an
image-mapped texture layer.
====================================================================== */

static Node *make_list( GlobalFunc *global )
{
   Node *node;
   LWInterfaceInfo *intinfo;
   const LWItemID *obj;
   LWSurfaceID *surfid;
   int nsurf;


   /* get the Interface Info global */

   intinfo = global( LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !intinfo ) return NULL;

   /* find the currently selected object, or the first in the group */

   obj = intinfo->selItems;
   while ( *obj ) {
      if ( iteminfo->type( *obj ) == LWI_OBJECT ) break;
      ++obj;
   }
   if ( !*obj ) {
      msgf->info( "Please select an object and try again.", NULL );
      return NULL;
   }

   /* get the object's surfaces, count the ones with image maps */

   surfid = surff->byObject( objinfo->filename( *obj ));
   nsurf = count_imagesurfs( surfid );
   if ( nsurf <= 0 ) {
      msgf->info( "No image-mapped surfaces found", "in the selected object." );
      return NULL;
   }

   /* create the root of the tree */

   node = calloc( 1, sizeof( Node ));
   if ( !node ) return NULL;
   if ( !init_node( node, iteminfo->name( *obj ), *obj, nsurf ))
      return NULL;

   /* get the rest of the tree */

   if ( !get_surflist( node, surfid )) {
      msgf->error( "An error occurred while building", "the surfaces list." );
      free_tree( node, 1 );
      return NULL;
   }

   return node;
}


/*
======================================================================
scount()
sname()

CUSTPOPUP_CTL callbacks.
====================================================================== */

static int scount( void *data )
{
   return imgutil->saverCount();
}


static char *sname( void *data, int index )
{
   return imgutil->saverName( index );
}


/*
======================================================================
find_uwpnode()

Given an UnwrapParams, returns the corresponding tree node.  We don't
call this, but we'll keep it in case it's useful in the future.
====================================================================== */

static Node *find_uwpnode( UnwrapParams *uwp, Node *root )
{
   Node *node;
   int i;

   node = NULL;
   if ( root->id == uwp->object ) {
      node = root;
      for ( i = 0; i < node->nchildren; i++ )
         if ( node->child[ i ].id == uwp->surface )
            break;
      if ( i < node->nchildren ) {
         node = &node->child[ i ];
         for ( i = 0; i < node->nchildren; i++ )
            if ( node->child[ i ].id == uwp->texture )
               break;
         if ( i < node->nchildren ) {
            node = &node->child[ i ];
            for ( i = 0; i < node->nchildren; i++ )
               if ( node->child[ i ].id == uwp->tlayer ) {
                  node = &node->child[ i ];
                  break;
               }
         }
      }
   }

   return node;
}


/*
======================================================================
getsel_tree()

Get the Unwrap params from the selected tree node.
====================================================================== */

static int getsel_tree( UnwrapParams *uwp, Node *node )
{
   if ( !node ) return 0;
   if ( node->level != 3 ) return 0;

   uwp->tlayer  = ( LWTLayerID )  node->id;   node = node->parent;
   uwp->texture = ( LWTextureID ) node->id;   node = node->parent;
   uwp->surface = ( LWSurfaceID ) node->id;   node = node->parent;
   uwp->object  = ( LWItemID )    node->id;

   txtrf->getParam( uwp->tlayer, TXTAG_IMAGE, &uwp->image );
   txtrf->getParam( uwp->tlayer, TXTAG_PROJ, &uwp->proj );
   if ( uwp->proj == TXPRJ_FRONT ) return 0;
   if ( uwp->proj == TXPRJ_UVMAP )
      txtrf->getParam( uwp->tlayer, TXTAG_VMAP, &uwp->vmap );

   return 1;
}


/*
======================================================================
filename_ext()

Append the filename extension for the selected image file format.

We search from the end of the filename for a '.' (a dot).  If it's
among the last five characters, we assume it's the separator for an
existing extension, which will be replaced.  By convention, the
extension for an image saver is stored within parentheses at the end
of the saver's server name.
====================================================================== */

static void filename_ext( int saver, char *filename )
{
   char *p, *s;
   const char *q;

   p = filename + strlen( filename );
   s = strrchr( filename, '.' );
   if ( s && ( p - s <= 5 )) p = s;

   q = imgutil->saverName( saver );
   q = strrchr( q, '(' );
   if ( !q ) return;
   q++;
   while ( *q != ')' ) *p++ = *q++;
   *p = 0;
}


/*
======================================================================
saver_event()

Callback for the image file format popup.  Calls filename_ext() to
update the extension in the filename edit field.
====================================================================== */

static void saver_event( LWControl *sctl, LWControl *ectl )
{
   LWValue
      ival = { LWT_INTEGER },
      sval = { LWT_STRING };
   char filename[ 260 ];
   int saver;

   GET_INT( sctl, saver );
   GET_STR( ectl, filename, sizeof( filename ));
   filename_ext( saver, filename );
   SET_STR( ectl, filename, sizeof( filename ));
   RENDER_CON( ectl );
}


/*
======================================================================
sizebtn_event()

Callback for the "From Image Map" button, which sets the size of the
Unwrap image to that of the texture layer's image map.
====================================================================== */

static void sizebtn_event( LWControl *bctl, LWControl *ctl[] )
{
   LWValue ival = { LWT_INTEGER };
   LWImageID image;
   Node *node;
   int w, h;

   ctl[ 0 ]->get( ctl[ 0 ], CTL_VALUE, &ival );
   node = ( Node * ) ival.ptr.ptr;
   if ( node ) {
      if ( node->level == 3 ) {
         if ( image = get_image(( LWTLayerID ) node->id )) {
            imglist->size( image, &w, &h );
            SET_INT( ctl[ 3 ], w );
            SET_INT( ctl[ 4 ], h );
            RENDER_CON( ctl[ 3 ] );
            RENDER_CON( ctl[ 4 ] );
         }
      }
   }
}


/*
======================================================================
get_user()

Display the panel.
====================================================================== */

int get_user( UnwrapParams *uwp )
{
   static char *text[] = {
      "Draws polygons in the 2D coordinate system of the selected texture layer.",
      NULL
   };
   static char *fgopt[] = {
      "Use Color",
      "Invert Background",
      "Brighten",
      "Darken",
      NULL
   };
   static char *bgopt[] = {
      "Use Color",
      "Copy Image Map",
      NULL
   };

   LWPanControlDesc desc;
   LWValue
      ival    = { LWT_INTEGER },
      sval    = { LWT_STRING },
      ivecval = { LWT_VINT };
   LWPanelID panel;
   LWControl *ctl[ 10 ], *bdr[ 2 ];
   Node *root;
   int i, x, y, w, ph, ok;


   root = make_list( panf->globalFun );
   if ( !root ) return 0;

   if( !( panel = PAN_CREATE( panf, "Unwrap" ))) {
      free_tree( root, 1 );
      return 0;
   }

   if ( !uwp->filename[ 0 ] ) {
      strcpy( uwp->filename, "unwrapped" );
      filename_ext( uwp->saver, uwp->filename );
      uwp->bgcolor[ 0 ] = uwp->bgcolor[ 1 ] = uwp->bgcolor[ 2 ] = 255;
   }

   TEXT_CTL( panf, panel, "", text );

   ctl[ 0 ] = TREE_CTL( panf, panel, "Texture Layer", 200, 200, tree_info,
      tree_count, tree_child );

   ph = PAN_GETH( panf, panel );
   ph -= CON_X( ctl[ 0 ] );
   ph -= CON_H( ctl[ 0 ] );

   ctl[ 1 ] = SAVE_CTL( panf, panel, "Save As", 40 );
   ctl[ 2 ] = CUSTPOPUP_CTL( panf, panel, "", 150, sname, scount );
   ctl[ 3 ] = INT_CTL( panf, panel, "Width" );
   ctl[ 4 ] = INT_CTL( panf, panel, "Height" );
   ctl[ 5 ] = BUTTON_CTL( panf, panel, "From Image Map" );
   ctl[ 6 ] = WPOPUP_CTL( panf, panel, "Edges", fgopt, 150 );
   ctl[ 7 ] = MINIRGB_CTL( panf, panel, "" );
   ctl[ 8 ] = WPOPUP_CTL( panf, panel, "Background", bgopt, 150 );
   ctl[ 9 ] = MINIRGB_CTL( panf, panel, "" );

   w = CON_W( ctl[ 1 ] );
   w -= CON_LW( ctl[ 1 ] );

   bdr[ 0 ] = BORDER_CTL( panf, panel, "", w, 2 );
   bdr[ 1 ] = BORDER_CTL( panf, panel, "", w, 2 );

   x = CON_X( ctl[ 0 ] );
   x += CON_W( ctl[ 0 ] );
   x += CON_LW( ctl[ 8 ] ) + 8;
   y = CON_Y( ctl[ 0 ] );

   w = CON_LW( ctl[ 1 ] );
   MOVE_CON( ctl[ 1 ], x - w, y );

   w = CON_LW( ctl[ 2 ] );
   y += CON_HOTH( ctl[ 1 ] ) + 4;
   MOVE_CON( ctl[ 2 ], x - w, y );

   y += CON_HOTH( ctl[ 2 ] ) + 6;
   MOVE_CON( bdr[ 0 ], x, y );

   w = CON_LW( ctl[ 3 ] );
   y += 6;
   MOVE_CON( ctl[ 3 ], x - w, y );

   w = CON_X( ctl[ 3 ] );
   w += CON_W( ctl[ 3 ] );
   MOVE_CON( ctl[ 5 ], w + 16, y );

   w = CON_LW( ctl[ 4 ] );
   y += CON_HOTH( ctl[ 3 ] ) + 4;
   MOVE_CON( ctl[ 4 ], x - w, y );

   y += CON_HOTH( ctl[ 2 ] ) + 6;
   MOVE_CON( bdr[ 1 ], x, y );

   y += 6;
   for ( i = 6; i <= 9; i++ ) {
      w = CON_LW( ctl[ i ] );
      MOVE_CON( ctl[ i ], x - w, y );
      y += CON_HOTH( ctl[ i ] ) + 4;
   }

   y = CON_Y( ctl[ 9 ] );
   y += CON_H( ctl[ 9 ] );
   PAN_SETH( panf, panel, y + ph );

   SET_STR( ctl[ 1 ], uwp->filename, sizeof( uwp->filename ));
   SET_INT( ctl[ 2 ], uwp->saver );
   SET_INT( ctl[ 3 ], uwp->width );
   SET_INT( ctl[ 4 ], uwp->height );
   SET_INT( ctl[ 6 ], uwp->fgoptions );
   SET_INT( ctl[ 8 ], uwp->bgoptions );
   SETV_IVEC( ctl[ 7 ], uwp->fgcolor );
   SETV_IVEC( ctl[ 9 ], uwp->bgcolor );

   CON_SETEVENT( ctl[ 0 ], tree_event, root );
   CON_SETEVENT( ctl[ 2 ], saver_event, ctl[ 1 ] );
   CON_SETEVENT( ctl[ 5 ], sizebtn_event, ctl );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      GET_STR( ctl[ 1 ], uwp->filename, sizeof( uwp->filename ));
      GET_INT( ctl[ 2 ], uwp->saver );
      GET_INT( ctl[ 3 ], uwp->width );
      GET_INT( ctl[ 4 ], uwp->height );
      GET_INT( ctl[ 6 ], uwp->fgoptions );
      GET_INT( ctl[ 8 ], uwp->bgoptions );
      GETV_IVEC( ctl[ 7 ], uwp->fgcolor );
      GETV_IVEC( ctl[ 9 ], uwp->bgcolor );

      ctl[ 0 ]->get( ctl[ 0 ], CTL_VALUE, &ival );
      if ( !getsel_tree( uwp, ( Node * ) ival.ptr.ptr )) {
         msgf->error( "No texture layer selected", NULL );
         ok = 0;
      }
   }

   PAN_KILL( panf, panel );

   free_tree( root, 1 );
   return ok;
}
