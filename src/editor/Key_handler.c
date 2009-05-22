

#include <stdlib.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "Key_handler.h"


// static guint Key_handler_signals[LAST_SIGNAL] = { 0 };


static GtkWidgetClass* parent_class = NULL;


static void Key_handler_destroy(GtkObject* obj);


static void Key_handler_class_init(Key_handler_class* cl)
{
/*  Key_handler_signals[KEY_HANDLER_SIGNAL] =
            g_signal_new("Key_handler",
                    G_TYPE_FROM_CLASS(cl),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET(Key_handler_class, Key_handler),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0); */

    parent_class = gtk_type_class(gtk_widget_get_type());
    GtkObjectClass* ocl = (GtkObjectClass*)cl;
    ocl->destroy = Key_handler_destroy;
    return;
}


static void Key_handler_init(Key_handler* handler)
{
    (void)handler;
    return;
}


GType Key_handler_get_type(void)
{
    static GType type = 0;
    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(Key_handler_class),
            NULL, // base_init
            NULL, // base_finalise
            (GClassInitFunc)Key_handler_class_init,
            NULL, // class_finalise
            NULL, // class_data
            sizeof(Key_handler),
            0, // n_preallocs
            (GInstanceInitFunc)Key_handler_init,
            NULL // value_table
        };
        type = g_type_register_static(GTK_TYPE_WINDOW, "Key_handler", &info, 0);
    }
    return type;
}


GtkWidget* Key_handler_new(Songs* songs)
{
    Key_handler* kh = KEY_HANDLER(g_object_new(KEY_HANDLER_TYPE, NULL));
    kh->songs = songs;
    return GTK_WIDGET(kh);
}


gboolean Key_handler_handle(GtkWidget* kh, GdkEventKey* event, gpointer data)
{
    g_assert(kh != NULL);
    g_assert(event != NULL);
    (void)data;
    g_print("%lu\n", (unsigned long)event->keyval);
    return TRUE;
}


static void Key_handler_destroy(GtkObject* obj)
{
    g_return_if_fail(obj != NULL);
    g_return_if_fail(IS_KEY_HANDLER(obj));
//    Key_handler* kh = KEY_HANDLER(obj);
    if (GTK_OBJECT_CLASS(parent_class)->destroy)
    {
        (*GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
    }
    return;
}


