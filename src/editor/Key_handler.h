

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_UI_KEY_HANDLER_H
#define K_UI_KEY_HANDLER_H


#include <glib.h>

#include <AAtree.h>

#include "Songs.h"


G_BEGIN_DECLS


#define KEY_HANDLER_TYPE (Key_handler_get_type())
#define KEY_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KEY_HANDLER_TYPE, Key_handler))
#define KEY_HANDLER_CLASS(cl) (G_TYPE_CHECK_CLASS_CAST((cl), KEY_HANDLER_TYPE, Key_handler_class))
#define IS_KEY_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KEY_HANDLER_TYPE))
#define IS_KEY_HANDLER_CLASS(cl) (G_TYPE_CHECK_CLASS_TYPE((cl), KEY_HANDLER_TYPE))


typedef struct _Key_handler
{
    GtkWindow window;
    AAtree* code_to_action;
    Songs* songs;
} Key_handler;


typedef struct _Key_handler_class
{
    GtkWindowClass parent_class;
} Key_handler_class;


GType Key_handler_get_type(void);


GtkWidget* Key_handler_new(Songs* songs);


gboolean Key_handler_handle(GtkWidget* kh, GdkEventKey* event, gpointer data);


G_END_DECLS


#endif // K_UI_KEY_HANDLER_H


