

#include <stdlib.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "Songs.h"


static void Songs_class_init(Songs_class* cl)
{
    (void)cl;
    return;
}


static void Songs_init(Songs* songs)
{
    (void)songs;
    return;
}


GType Songs_get_type(void)
{
    static GType type = 0;
    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(Songs_class),
            NULL, // base_init
            NULL, // base_finalise
            (GClassInitFunc)Songs_class_init,
            NULL, // class_finalise
            NULL, // class_data
            sizeof(Songs),
            0, // n_preallocs
            (GInstanceInitFunc)Songs_init,
            NULL // value_table
        };
        type = g_type_register_static(GTK_TYPE_NOTEBOOK, "Songs", &info, 0);
    }
    return type;
}


GtkWidget* Songs_new(void)
{
    Songs* songs = SONGS(g_object_new(SONGS_TYPE, NULL));
    return GTK_WIDGET(songs);
}


