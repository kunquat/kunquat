

#ifndef K_UI_SONGS_H
#define K_UI_SONGS_H


#include <glib.h>


G_BEGIN_DECLS


#define SONGS_TYPE (Songs_get_type())
#define SONGS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SONGS_TYPE, Songs))
#define SONGS_CLASS(cl) (G_TYPE_CHECK_CLASS_CAST((cl), SONGS_TYPE, Songs_class))
#define IS_SONGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SONGS_TYPE))
#define IS_SONGS_CLASS(cl) (G_TYPE_CHECK_CLASS_TYPE((cl), SONGS_TYPE))


typedef struct _Songs
{
    GtkNotebook notebook;
} Songs;


typedef struct _Songs_class
{
    GtkNotebookClass parent_class;
} Songs_class;


GType Songs_get_type(void);


GtkWidget* Songs_new(void);


G_END_DECLS


#endif // K_UI_SONGS_H


