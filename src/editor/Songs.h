

#ifndef K_UI_SONGS_H
#define K_UI_SONGS_H


#include <stdint.h>

#include <glib.h>

#include <Playlist.h>


G_BEGIN_DECLS


#define SONGS_TYPE (Songs_get_type())
#define SONGS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SONGS_TYPE, Songs))
#define SONGS_CLASS(cl) (G_TYPE_CHECK_CLASS_CAST((cl), SONGS_TYPE, Songs_class))
#define IS_SONGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SONGS_TYPE))
#define IS_SONGS_CLASS(cl) (G_TYPE_CHECK_CLASS_TYPE((cl), SONGS_TYPE))


/**
 * A widget that contains all the Song views.
 */
typedef struct _Songs
{
    GtkNotebook notebook;
    Playlist* playlist;
} Songs;


typedef struct _Songs_class
{
    GtkNotebookClass parent_class;
} Songs_class;


GType Songs_get_type(void);


/**
 * Creates a new Songs (collection of Song views).
 *
 * \return   The new widget.
 */
GtkWidget* Songs_new(void);


/**
 * Adds a new Song view into Songs.
 *
 * \param songs   The Songs -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Songs_add_song(Songs* songs);


G_END_DECLS


#endif // K_UI_SONGS_H


