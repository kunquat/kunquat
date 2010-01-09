

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
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


