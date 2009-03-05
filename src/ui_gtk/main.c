

#include <stdlib.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "Key_handler.h"
#include "Songs.h"


static gboolean quit(GtkWidget* widget, GdkEvent* event, gpointer data)
{
    (void)widget;
    (void)event;
    (void)data;
    return FALSE;
}


static void destroy(GtkWidget* widget, gpointer data)
{
    (void)widget;
    (void)data;
    gtk_main_quit();
    return;
}


static void create_song(GtkToolButton* new_song_button, gpointer user_data)
{
    g_assert(new_song_button != NULL);
    g_assert(user_data != NULL);
    Songs* songs = SONGS(user_data);
    if (!Songs_add_song(songs))
    {
        g_warning("Couldn't allocate memory for new Song");
        return;
    }
    return;
}


int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(quit), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
    gtk_window_set_title(GTK_WINDOW(window), "Kunquat");

    GtkWidget* main_controls = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_controls);

    GtkWidget* tools = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(main_controls), tools, FALSE, FALSE, 0);

    GtkToolItem* new_song_button = gtk_tool_button_new(NULL, "New Song");
    gtk_toolbar_insert(GTK_TOOLBAR(tools), new_song_button, -1);

    GtkWidget* songs = Songs_new();
    g_signal_connect(G_OBJECT(new_song_button), "clicked", G_CALLBACK(create_song), songs);
    gtk_box_pack_start(GTK_BOX(main_controls), songs, TRUE, TRUE, 0);

    GtkWidget* kh = Key_handler_new(SONGS(songs));
    g_signal_connect_after(G_OBJECT(kh), "key-press-event",
            G_CALLBACK(Key_handler_handle), kh);
    g_signal_connect_after(G_OBJECT(window), "key-press-event",
            G_CALLBACK(Key_handler_handle), kh);

    gtk_widget_show_all(window);
//    gtk_widget_show_all(kh);
    gtk_main();
    g_object_unref(kh);
    exit(EXIT_SUCCESS);
}


