

#include <stdlib.h>
#include <assert.h>

#include <gtk/gtk.h>

#include <Song.h>

#include "Song_view.h"


static void Song_view_class_init(Song_view_class* cl)
{
    (void)cl;
    return;
}


static void Song_view_init(Song_view* sv)
{
    (void)sv;
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


GtkWidget* Song_view_new(void)
{
    Song_view* sv = SONG_VIEW(g_object_new(SONG_VIEW_TYPE, NULL));
    GTK_BOX(sv)->spacing = 0;
    GTK_BOX(sv)->homogeneous = FALSE;
    return GTK_WIDGET(sv);
}


