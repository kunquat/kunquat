

#ifndef K_UI_SONG_VIEW_H
#define K_UI_SONG_VIEW_H


#include <glib.h>


G_BEGIN_DECLS


#define SONG_VIEW_TYPE (Song_view_get_type())
#define SONG_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SONG_VIEW_TYPE, Song_view))
#define SONG_VIEW_CLASS(cl) (G_TYPE_CHECK_CLASS_CAST((cl), SONG_VIEW_TYPE, Song_view_class))
#define IS_SONG_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SONG_VIEW_TYPE))
#define IS_SONG_VIEW_CLASS(cl) (G_TYPE_CHECK_CLASS_TYPE((cl), SONG_VIEW_TYPE))


typedef struct _Song_view
{
    GtkVBox vbox;
} Song_view;


typedef struct _Song_view_class
{
    GtkVBoxClass parent_class;
} Song_view_class;


GType Song_view_get_type(void);


GtkWidget* Song_view_new(void);


G_END_DECLS


#endif // K_UI_SONG_VIEW_H


