

#ifndef K_UI_SONG_VIEW_H
#define K_UI_SONG_VIEW_H


#include <stdint.h>

#include <glib.h>

#include <Player.h>


G_BEGIN_DECLS


#define SONG_VIEW_TYPE (Song_view_get_type())
#define SONG_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SONG_VIEW_TYPE, Song_view))
#define SONG_VIEW_CLASS(cl) (G_TYPE_CHECK_CLASS_CAST((cl), SONG_VIEW_TYPE, Song_view_class))
#define IS_SONG_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SONG_VIEW_TYPE))
#define IS_SONG_VIEW_CLASS(cl) (G_TYPE_CHECK_CLASS_TYPE((cl), SONG_VIEW_TYPE))


typedef struct _Song_view
{
    GtkVBox vbox;
    Player* player;
} Song_view;


typedef struct _Song_view_class
{
    GtkVBoxClass parent_class;
} Song_view_class;


GType Song_view_get_type(void);


/**
 * Creates a new Song view.
 *
 * \param player   The Player associated with this Song view -- must not be
 *                 \c NULL.
 *
 * \return   The new Song view.
 */
GtkWidget* Song_view_new(Player* player);


/**
 * Returns the Player associated with the Song view.
 *
 * \param sv   The Song view -- must not be \c NULL.
 *
 * \return   The Player, or \c NULL if the Player has been detached.
 */
Player* Song_view_get_player(Song_view* sv);


/**
 * Detaches the Player associated with the Song view.
 *
 * \param sv   The Song view -- must not be \c NULL.
 *
 * \return   The Player detached, or \c NULL if the Player was already
 *           detached.
 */
Player* Song_view_detach_player(Song_view* sv);


G_END_DECLS


#endif // K_UI_SONG_VIEW_H


