

#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include <gtk/gtk.h>

#include "Songs.h"

#include "Song_view.h"


/**
 * Creates the header of a Song view tab.
 *
 * \param sv   The Song view -- must not be \c NULL.
 */
static GtkWidget* Songs_create_tab_head(GtkWidget* sv);


/**
 * Removes a Song view from Songs.
 *
 * This is a callback function used for signal "page-removed".
 */
static void Songs_remove_song(GtkNotebook* songs_nb,
                              GtkWidget* song_view,
                              guint page_num,
                              gpointer user_data);


/**
 * Closes a tab of a Song view.
 *
 * This is a callback function for the close button.
 */
static void close_song(GtkButton* button, gpointer user_data);


/**
 * Overrides the style of the Song close button.
 */
static void set_close_style(GtkWidget* close_button, GtkStyle prev_style, gpointer user_data);


static void Songs_destroy(GtkObject* obj);


static GtkWidgetClass* parent_class = NULL;


static void Songs_class_init(Songs_class* cl)
{
    parent_class = gtk_type_class(gtk_widget_get_type());
    GtkObjectClass* ocl = (GtkObjectClass*)cl;
    ocl->destroy = Songs_destroy;

    gtk_rc_parse_string("style \"tab-close-button-style\"\n"
                        "{\n"
                            "GtkWidget::focus-padding = 0\n"
                            "GtkWidget::focus-line-width = 0\n"
                            "xthickness = 0\n"
                            "ythickness = 0\n"
                        "}\n"
                        "widget \"*.tab-close-button\" style \"tab-close-button-style\"");
    return;
}


static void Songs_init(Songs* songs)
{
    songs->playlist = new_Playlist();
    if (songs->playlist == NULL)
    {
        g_error("Couldn't allocate memory for Playlist");
    }
    g_signal_connect(G_OBJECT(songs), "page-removed", G_CALLBACK(Songs_remove_song), NULL);
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


static void close_song(GtkButton* button, gpointer user_data)
{
    (void)button;
    g_assert(user_data != NULL);
    GtkWidget* sv = GTK_WIDGET(user_data);
    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_widget_get_parent(sv));
    gint page_num = gtk_notebook_page_num(notebook, sv);
    g_assert(page_num != -1);
    gtk_notebook_remove_page(notebook, page_num);
    return;
}


static void set_close_style(GtkWidget* close_button, GtkStyle prev_style, gpointer user_data)
{
    (void)prev_style;
    (void)user_data;
    int w = 0;
    int h = 0;
    gtk_icon_size_lookup_for_settings(gtk_widget_get_settings(close_button),
            GTK_ICON_SIZE_MENU, &w, &h);
    gtk_widget_set_size_request(close_button, w + 2, h + 2);
    return;
}


static GtkWidget* Songs_create_tab_head(GtkWidget* sv)
{
    g_assert(sv != NULL);
    g_assert(IS_SONG_VIEW(sv));
    Player* player = Song_view_get_player(SONG_VIEW(sv));
    g_assert(player != NULL);
    int32_t player_id = Player_get_id(player);
    gchar replacement_title[24] = { '\0' };
    g_snprintf(replacement_title, 23, "(untitled #%" PRId32 ")", player_id);
    gchar num[11] = { '\0' };
    g_snprintf(num, 10, "%" PRId32 ": ", player_id);
    gchar* tab_title = g_strconcat(num, replacement_title, NULL);
    GtkWidget* label = gtk_label_new(tab_title);
    g_free(tab_title);

    GtkWidget* image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    GtkWidget* close = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(close), FALSE);
    gtk_container_add(GTK_CONTAINER(close), image);
    gtk_widget_set_name(close, "tab-close-button");
    g_signal_connect(G_OBJECT(close), "style-set", G_CALLBACK(set_close_style), NULL);
    g_signal_connect(G_OBJECT(close), "clicked", G_CALLBACK(close_song), sv);

    GtkWidget* head = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(head), label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(head), close, FALSE, FALSE, 0);
    return head;
}


GtkWidget* Songs_new(void)
{
    Songs* songs = SONGS(g_object_new(SONGS_TYPE, NULL));
    return GTK_WIDGET(songs);
}


bool Songs_add_song(Songs* songs)
{
    g_assert(songs != NULL);
    Song* song = new_Song(Playlist_get_buf_count(songs->playlist),
            Playlist_get_buf_size(songs->playlist),
            16); // TODO: get event count from configuration
    if (song == NULL)
    {
        return false;
    }
    Player* player = new_Player(48000, 64, song); // TODO: get from config
    if (player == NULL)
    {
        del_Song(song);
        return false;
    }
    GtkWidget* sv = Song_view_new(player);
    GtkWidget* tab_label = Songs_create_tab_head(sv);
    if (gtk_notebook_append_page(GTK_NOTEBOOK(songs), sv, tab_label) == -1)
    {
        del_Player(player);
        g_object_unref(sv);
        return false;
    }
    gtk_widget_show_all(sv);
    gtk_widget_show_all(tab_label);
    Playlist_ins_player(songs->playlist, player);
    return true;
}


static void Songs_remove_song(GtkNotebook* songs_nb,
                              GtkWidget* song_view,
                              guint page_num,
                              gpointer user_data)
{
    (void)page_num;
    (void)user_data;
    Songs* songs = SONGS(songs_nb);
    g_assert(songs != NULL);
    g_assert(songs->playlist != NULL);
    Song_view* sv = SONG_VIEW(song_view);
    Player* player = Song_view_detach_player(sv);
    g_assert(player != NULL);
    Playlist_remove_player(songs->playlist, player);
    return;
}


static void Songs_destroy(GtkObject* obj)
{
    g_return_if_fail(obj != NULL);
    g_return_if_fail(IS_SONGS(obj));
    Songs* songs = SONGS(obj);
    if (songs->playlist != NULL)
    {
        del_Playlist(songs->playlist);
        songs->playlist = NULL;
    }
    if (GTK_OBJECT_CLASS(parent_class)->destroy)
    {
        (*GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
    }
    return;
}


