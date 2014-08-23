// GTK Viewer for Vrml 97 library
//
// Copyright (C) 1998 by Erik Andersen <andersee@debian.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtkglarea.h>

/* Now some good stuff */
#include "VrmlScene.h"
#include "VrmlNodeType.h"
#include "ViewerGtk.h"

VrmlScene *vrmlScene = 0;
ViewerGtk *viewer = 0;
GtkWidget* mainWnd;
GtkWidget* sceneWnd;
static GtkWidget *FileSelDialog = NULL;

struct NodeInfo {
  GtkWidget *nodeType;
};

static void
file_new(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
    g_message("file_new is not yet implemented because I don't know\n"
	"what it is supposed to do...  Any ideas?");
}

void file_open_ok (GtkWidget *w, GtkFileSelection *fs)
{
    const char *filename=gtk_file_selection_get_filename (fs);
    if (vrmlScene && filename)
	if (vrmlScene->load( filename))
	    g_print ("Ok, file \"%s\" was loaded!\n", filename);
	else 
	    g_print ("Error loading file \"%s\"\n", filename);
    gtk_widget_destroy (GTK_WIDGET (fs));
}

static void
file_open(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
  if (!FileSelDialog)
    {
      FileSelDialog = gtk_file_selection_new ("file open");
      gtk_window_set_position (GTK_WINDOW (FileSelDialog), GTK_WIN_POS_MOUSE);
      gtk_signal_connect (GTK_OBJECT (FileSelDialog), "destroy",
                          GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                          &FileSelDialog);
      gtk_signal_connect (GTK_OBJECT ( 
			  GTK_FILE_SELECTION (FileSelDialog)->ok_button),
                          "clicked", GTK_SIGNAL_FUNC(file_open_ok),
                          FileSelDialog);
      gtk_signal_connect_object (GTK_OBJECT (
			GTK_FILE_SELECTION (FileSelDialog)->cancel_button),
			"clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
                        GTK_OBJECT (FileSelDialog));
  }

  if (!GTK_WIDGET_VISIBLE (FileSelDialog)) {
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(FileSelDialog));
    gtk_widget_show (FileSelDialog);
  }
  else
    gtk_widget_destroy (FileSelDialog);

}

static void
file_save(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
   //Doc *theDoc=vrmlScene->url();
   const char *filename=NULL;
   // Wierd.  Using this won't compile...
   //const char *filename=theDoc->localName();
   g_print ("file_save doesn't work right now.  Try file_save_as\n");
    
   if (vrmlScene && filename) {
	if (vrmlScene->save( filename))
	    g_print ("Ok, file \"%s\" saved!\n", filename);
	else 
	    g_print ("Error saving file %s\n", filename);
   }
}

void file_save_ok (GtkWidget *w, GtkFileSelection *fs)
{
    const char *filename=gtk_file_selection_get_filename (fs);
    if (vrmlScene && filename)
	if (vrmlScene->save( filename))
	    g_print ("Ok, file \"%s\" saved!\n", filename);
	else 
	    g_print ("Error saving file %s\n", filename);
    gtk_widget_destroy (GTK_WIDGET (fs));
}

static void
file_save_as(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
  //Doc *theDoc=vrmlScene->url();
  const char *filename="file.wrl";
  // Wierd.  Using this won't compile...
  //const char *filename=theDoc->localName();

  if (!FileSelDialog)
    {
      FileSelDialog = gtk_file_selection_new ("file save as");
      gtk_window_set_position (GTK_WINDOW (FileSelDialog), GTK_WIN_POS_MOUSE);
      gtk_signal_connect (GTK_OBJECT (FileSelDialog), "destroy",
                          GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                          &FileSelDialog);
      gtk_signal_connect (GTK_OBJECT ( 
			  GTK_FILE_SELECTION (FileSelDialog)->ok_button),
                          "clicked", GTK_SIGNAL_FUNC(file_save_ok),
                          FileSelDialog);
      gtk_signal_connect_object (GTK_OBJECT (
			GTK_FILE_SELECTION (FileSelDialog)->cancel_button),
			"clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
                        GTK_OBJECT (FileSelDialog));
    }

  if (!GTK_WIDGET_VISIBLE (FileSelDialog)) {
    gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(FileSelDialog));
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(FileSelDialog),
	filename);
    gtk_widget_show (FileSelDialog);
  }
  else
    gtk_widget_destroy (FileSelDialog);

}

static void
file_quit(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
    delete viewer;
    delete vrmlScene;
    gtk_exit(0);
}

static void
view_reset(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
    viewer->resetUserNavigation();
}

static void tree_itemselect (GtkWidget *item, NodeInfo *info)
{
  VrmlNode *node = (VrmlNode *)gtk_object_get_user_data(GTK_OBJECT(item));
  VrmlNodeType *type = node->nodeType();
  const char *name = type->getName();
  gtk_label_set_text (GTK_LABEL(info->nodeType), name);

  return;
}


/* for all the GtkItem:: and GtkTreeItem:: signals */
static void tree_itemsignal (GtkWidget *item, gchar *signame)
{
  gchar *name;
  GtkLabel *label;

  /* It's a GtkBin, so it has one child, which we know to be a
     label, so get that */
  label = GTK_LABEL (GTK_BIN (item)->child);
  /* Get the text of the label */
  gtk_label_get (label, &name);
  /* Get the level of the tree which the item is in */
  g_print ("%s called for item %s->%p, level %d\n", signame, name,
	   item, GTK_TREE (item->parent)->level);
}

/* Note that this is never called */
static void tree_unselect_child (GtkWidget *root_tree, GtkWidget *child,
			       GtkWidget *subtree)
{
  g_print ("unselect_child called for root tree %p, subtree %p, child %p\n",
	   root_tree, subtree, child);
}

/* Note that this is called every time the user clicks on an item,
   whether it is already selected or not. */
static void tree_select_child (GtkWidget *root_tree, GtkWidget *child,
			     GtkWidget *subtree)
{
  g_print ("select_child called for root tree %p, subtree %p, child %p\n",
	   root_tree, subtree, child);
}

static void tree_selection_changed (GtkWidget *tree)
{
  GList *i;
  
  g_print ("selection_change called for tree %p\n", tree);
  g_print ("selected objects are:\n");

  i = GTK_TREE_SELECTION(tree);
  while (i){
    gchar *name;
    GtkLabel *label;
    GtkWidget *item;

    /* Get a GtkWidget pointer from the list node */
    item = GTK_WIDGET (i->data);
    label = GTK_LABEL (GTK_BIN (item)->child);
    gtk_label_get (label, &name);
    g_print ("\t%s on level %d\n", name, GTK_TREE
	     (item->parent)->level);
    i = i->next;
  }
}

gint view_structure_done( GtkWidget *win, NodeInfo *info )
{
  g_message("view_structure_done!!");
  /* Turn back on the viewer animations */
  delete info;
  if (viewer) {
    viewer->SetStop( false);
    viewer->timerUpdate();
  }
  return TRUE;
}

static void
view_structure_recursive(VrmlNodeGroup *group, GtkWidget *tree, NodeInfo *info)
{
  for (int i = 0; i < group->size(); i++){
    GtkWidget *item;

    /* Create a tree item */
    if (strcmp(group->child(i)->name(),"") != 0)
      item = gtk_tree_item_new_with_label ((gchar*)group->child(i)->name());
    else {
      item = gtk_tree_item_new_with_label ("(untitled)");
    }
    gtk_object_set_user_data (GTK_OBJECT(item), group->child(i));
    /* Connect all GtkItem:: and GtkTreeItem:: signals */
    gtk_signal_connect (GTK_OBJECT(item), "select",
			GTK_SIGNAL_FUNC(tree_itemselect), info);
    gtk_signal_connect (GTK_OBJECT(item), "deselect",
			GTK_SIGNAL_FUNC(tree_itemsignal), "deselect");
    gtk_signal_connect (GTK_OBJECT(item), "toggle",
			GTK_SIGNAL_FUNC(tree_itemsignal), "toggle");
    gtk_signal_connect (GTK_OBJECT(item), "expand",
			GTK_SIGNAL_FUNC(tree_itemsignal), "expand");
    gtk_signal_connect (GTK_OBJECT(item), "collapse",
			GTK_SIGNAL_FUNC(tree_itemsignal), "collapse");
    /* Add it to the parent tree */
    gtk_tree_append (GTK_TREE(tree), item);
    /* Show it - this can be done at any time */
    gtk_widget_show (item);

    VrmlNodeGroup *subgroup = group->child(i)->toGroup();

    if (subgroup) {
      /* Create this item's subtree */
      GtkWidget *subtree = gtk_tree_new();
//       g_print ("-> item %s->%p, subtree %p\n", itemnames[i], item,
// 	       subtree);

      /* This is still necessary if you want these signals to be called
         for the subtree's children.  Note that selection_change will be 
         signalled for the root tree regardless. */
      gtk_signal_connect (GTK_OBJECT(subtree), "select_child",
			  GTK_SIGNAL_FUNC(tree_select_child), subtree);
      gtk_signal_connect (GTK_OBJECT(subtree), "unselect_child",
			  GTK_SIGNAL_FUNC(tree_unselect_child), subtree);
      /* Set this item's subtree - note that you cannot do this until
         AFTER the item has been added to its parent tree! */
      gtk_tree_item_set_subtree (GTK_TREE_ITEM(item), subtree);

      view_structure_recursive(subgroup, subtree, info);
    }
  }
}

static void
view_structure(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
  gint i;
  GtkWidget *scrolled_win, *tree;
  static gchar *itemnames[] = {"Foo", "Bar", "Baz", "Quux",
			       "Maurice"};

  VrmlNodeGroup *nodeGroup;

  NodeInfo *info = new NodeInfo;

  /* Have to turn off the viewer animations, or it will screw 
     up event handling for the tree window */
  if (viewer) {
    viewer->SetStop( true);
    VrmlScene *scene = viewer->scene();
    if (scene) {
      VrmlNode *node = scene->getRoot();
      if (node)
	nodeGroup = node->toGroup();
    }
  }

  /* Scene Structure window */
  sceneWnd = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(sceneWnd), "GtkLookat -- scene structure");
  gtk_signal_connect (GTK_OBJECT(sceneWnd), "destroy",
                          GTK_SIGNAL_FUNC (view_structure_done), NULL);
  gtk_quit_add_destroy(1, GTK_OBJECT(sceneWnd));
  gtk_container_border_width (GTK_CONTAINER(sceneWnd), 5);

  /* an hbox */
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add (GTK_CONTAINER(sceneWnd), hbox);

  /* a vbox for the node data labels */
  GtkWidget *labels = gtk_vbox_new(FALSE, 0);
  GtkWidget *nodeTypeLabel = gtk_label_new("Node type:");
  gtk_box_pack_start (GTK_BOX(labels), nodeTypeLabel, TRUE, TRUE, 0);

  /* a vbox for the node information */
  GtkWidget *data = gtk_vbox_new(FALSE, 0);
  info->nodeType = gtk_label_new("Click on a node to show its type");
  gtk_box_pack_start (GTK_BOX(data), info->nodeType, TRUE, TRUE, 0);

  /* A generic scrolled window */
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrolled_win, 200, 300);
  gtk_box_pack_start (GTK_BOX(hbox), scrolled_win, TRUE, TRUE, 0);

  /* pack the node info */
  gtk_box_pack_start (GTK_BOX(hbox), labels, TRUE, TRUE, 5);
  gtk_box_pack_start (GTK_BOX(hbox), data, TRUE, TRUE, 0);

  /* show the damn thing */
  gtk_widget_show_all (scrolled_win);
  
  /* Create the root tree */
  tree = gtk_tree_new();
  g_print ("root tree is %p\n", tree);
  /* connect all GtkTree:: signals */
  gtk_signal_connect (GTK_OBJECT(tree), "select_child",
		      GTK_SIGNAL_FUNC(tree_select_child), tree);
  gtk_signal_connect (GTK_OBJECT(tree), "unselect_child",
		      GTK_SIGNAL_FUNC(tree_unselect_child), tree);
  gtk_signal_connect (GTK_OBJECT(tree), "selection_changed",
		      GTK_SIGNAL_FUNC(tree_selection_changed), tree);
  /* Add it to the scrolled window */
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_win),
                                         tree);
  /* Set the selection mode */
  gtk_tree_set_selection_mode (GTK_TREE(tree),
			       GTK_SELECTION_BROWSE);
  /* Show it */
  gtk_widget_show (tree);

  if (nodeGroup) {
    view_structure_recursive(nodeGroup, tree, info);
  }

  /* Show the window */
  gtk_widget_show_all (sceneWnd);

//   viewer->SetStop(false);
//   viewer->timerUpdate();

}

gint viewpoint_dialog_done(GtkWidget *dialog, int *row)
{
  delete row;
  return TRUE;
}

void viewpoint_selection( GtkWidget      *clist,
			  gint            newrow,
			  gint            column,
			  GdkEventButton *event,
			  int            *row )
{
  *row = newrow;
}

void viewpoint_dialog_clicked(GtkWidget *view_button, int *row)
{
  if (*row >= 0)
    vrmlScene->setViewpoint(*row);
}

static void
viewpoint_dialog(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
  GtkWidget *dialog;
  gchar **titles;

  int *row = new int;
  *row = -1;

  titles = new (gchar *)[2];
  titles[0] = g_strdup("Name");
  titles[1] = g_strdup("Description");

  dialog = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_signal_connect(GTK_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(viewpoint_dialog_done), row);
  gtk_quit_add_destroy(1, GTK_OBJECT(dialog));
  gtk_container_border_width (GTK_CONTAINER(dialog), 5);

  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(dialog), vbox);

  GtkWidget *hbox = gtk_hbox_new(TRUE, 0);
  GtkWidget *view = gtk_button_new_with_label("Go to viewpoint");
  GtkWidget *close = gtk_button_new_with_label("Close window");
  gtk_box_pack_start(GTK_BOX(hbox), view, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(hbox), close, TRUE, TRUE, 5);    
  gtk_signal_connect_object(GTK_OBJECT (close), "clicked", 
			    GTK_SIGNAL_FUNC (gtk_widget_destroy), 
			    GTK_OBJECT (dialog));

  GtkWidget *clist = gtk_clist_new_with_titles(2, titles);
  gtk_signal_connect(GTK_OBJECT(clist), "select_row", 
		     GTK_SIGNAL_FUNC(viewpoint_selection), row);
  gchar **data = new (gchar *)[2];

  for (int i=0; i<vrmlScene->nViewpoints(); ++i) {
    char *name, *description;
    vrmlScene->getViewpoint(i, &name, &description);
    data[0] = g_strdup(name);
    data[1] = g_strdup(description);
    gtk_clist_append(GTK_CLIST(clist), data);
    delete [] data[0];
    delete [] data[1];
  }

  delete [] data;
  delete [] titles[1];
  delete [] titles[0];
  delete [] titles;

  gtk_signal_connect(GTK_OBJECT(view), "clicked", GTK_SIGNAL_FUNC(viewpoint_dialog_clicked), row);

  gtk_box_pack_start(GTK_BOX(vbox), clist, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);
}

static void
about_dialog(gpointer callback_data, guint callback_action, GtkWidget *widget)
{
    g_message("about_dialog is not yet implemented");
}


static void worldChangedCB( int reason )
{
  switch (reason)
    {
    case VrmlScene::DESTROY_WORLD:
      delete viewer;
      delete vrmlScene;
      exit(0);
      break;
      
    }

}


static GtkItemFactoryEntry menu_items[] =
{
  { "/_File",            NULL,         0,                   0, "<Branch>" },
  { "/File/tearoff1",    NULL,         0,                   0, "<Tearoff>" },
  { "/File/_New",        "<control>N", file_new,            0 },
  { "/File/_Open",       "<control>O", file_open,           0 },
  { "/File/_Save",       "<control>S", file_save,           0 },
  { "/File/Save _As...", "<control>A", file_save_as,        0 },
  { "/File/sep1",        NULL,         0,                   0, "<Separator>" },
  { "/File/_Quit",       "<control>Q", file_quit,           0 },
  { "/_View",            NULL,         0,                   0, "<Branch>" },
  { "/View/_Reset",      NULL,         view_reset,          0 },
  { "/View/Scene Structure", NULL,     view_structure,      0 },
  { "/View/Set Virtual Camera...", NULL, viewpoint_dialog,  0 },
  { "/_Help",            NULL,         0,                   0, "<LastBranch>" },
  { "/Help/_About",      NULL,         about_dialog,        0 },
};


static int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);




/* Sample application */
int main(int argc, char *argv[])
{

  /* initialize gtk */
  gtk_init( &argc, &argv );

  /* Check for extension; for Mesa, this is always cool */
  if(!glXQueryExtension(GDK_DISPLAY(),NULL,NULL)){
    fprintf(stderr,"The specified display does not support the "
	    "OpenGL extension\n");
    exit(1);
    }

  /* Main window */
  mainWnd = gtk_window_new( GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(mainWnd), "GtkLookat");
  gtk_container_border_width(GTK_CONTAINER(mainWnd), 0);
  gtk_quit_add_destroy(1, GTK_OBJECT(mainWnd));

  GtkWidget *mainVbox = gtk_vbox_new(FALSE, 1);
  gtk_widget_show(mainVbox);
  gtk_container_border_width(GTK_CONTAINER(mainVbox), 4);
  gtk_container_add(GTK_CONTAINER(mainWnd), mainVbox);
    
  GtkAccelGroup *accel_group = gtk_accel_group_new ();
  GtkItemFactory *item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, 
	"<main>", accel_group);
  gtk_item_factory_create_items (item_factory, nmenu_items, 
	menu_items, NULL);
  gtk_accel_group_attach (accel_group, GTK_OBJECT (mainWnd));      

  gtk_box_pack_start (GTK_BOX (mainVbox),
	gtk_item_factory_get_widget (item_factory, "<main>"),
	FALSE, FALSE, 0);

  /* create a frame to put the GL window into  */
  GtkWidget *frame = gtk_aspect_frame_new(NULL, 0.5,0.5, 1.3, TRUE);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (mainVbox), frame, TRUE, TRUE, 0);


  /* Vrml viewer */
  vrmlScene = new VrmlScene( argc > 1 ? argv[1] : "xx.wrl" );
  /* Add scene callback, call it once since the initial world has 
  already loaded */
  vrmlScene->addWorldChangedCallback( worldChangedCB );
  worldChangedCB( VrmlScene::REPLACE_WORLD );
  viewer = new ViewerGtk( vrmlScene, "GL Widget", frame );
  if (! viewer) {
      fprintf(stderr,"Can't create OpenGL viewer.\n");
      exit(1);
  }

  gtk_widget_show_all (mainWnd);

  // This calls the viewer's update method, which gets everything
  // working right and animating.
  viewer->timerUpdate();

  /* Loop until we're done */

  gtk_main();

  return 0;
}



