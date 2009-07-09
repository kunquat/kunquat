

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


#ifndef KEYBOARD_H
#define KEYBOARD_H


#include <stdbool.h>


typedef enum
{
    KEY_NONE = -1,
    KEY_LEFT = 256,
    KEY_DOWN,
    KEY_UP,
    KEY_RIGHT,
    KEY_RETURN,
    KEY_BACKSPACE
} Key;


/**
 * Gets a key.
 *
 * \return   The key if one pressed, otherwise \c -1.
 */
int get_key(void);


/**
 * Sets terminal attributes.
 *
 * \param interactive   If \c true, keys will be read as soon as they're pressed.
 * \param immediate     If \c true, \a get_key will not block.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool set_terminal(bool interactive, bool immediate);


#endif // KEYBOARD_H


