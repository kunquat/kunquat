

#include <stdlib.h>
#include <assert.h>

#include <gtk/gtk.h>

#include <Song.h>

#include "Song_view.h"


static GtkWidgetClass* parent_class = NULL;


static void Song_view_destroy(GtkObject* obj);


static void Song_view_class_init(Song_view_class* cl)
{
    parent_class = gtk_type_class(gtk_widget_get_type());
    GtkObjectClass* ocl = (GtkObjectClass*)cl;
    ocl->destroy = Song_view_destroy;
    return;
}


static void Song_view_init(Song_view* sv)
{
    sv->player = NULL;
    return;
}


GType Song_view_get_type(void)
{
    static GType type = 0;
    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(Song_view_class),
            NULL, // base_init
            NULL, // base_finalise
            (GClassInitFunc)Song_view_class_init,
            NULL, // class_finalise
            NULL, // class_data
            sizeof(Song_view),
            0, // n_preallocs
            (GInstanceInitFunc)Song_view_init,
            NULL // value_table
        };
        type = g_type_register_static(GTK_TYPE_VBOX, "Song_view", &info, 0);
    }
    return type;
}


GtkWidget* Song_view_new(Player* player)
{
    g_assert(player != NULL);
    Song_view* sv = SONG_VIEW(g_object_new(SONG_VIEW_TYPE, NULL));
    sv->player = player;
    gtk_box_set_spacing(GTK_BOX(sv), 0);
    gtk_box_set_homogeneous(GTK_BOX(sv), FALSE);
    return GTK_WIDGET(sv);
}


Player* Song_view_get_player(Song_view* sv)
{
    g_warn_if_fail(sv != NULL);
    return sv->player;
}


Player* Song_view_detach_player(Song_view* sv)
{
    Player* player = sv->player;
    sv->player = NULL;
    return player;
}


static void Song_view_destroy(GtkObject* obj)
{
    g_return_if_fail(obj != NULL);
    g_return_if_fail(IS_SONG_VIEW(obj));
    Song_view* sv = SONG_VIEW(obj);
    sv->player = NULL;
    if (GTK_OBJECT_CLASS(parent_class)->destroy)
    {
        (*GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
    }
    return;
}


