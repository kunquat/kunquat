

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


#ifndef K_ERROR_H
#define K_ERROR_H


#include <stdbool.h>


#define KQT_ERROR_LENGTH (256)


typedef struct kqt_Error
{
    bool error;
    char message[KQT_ERROR_LENGTH];
} kqt_Error;


/**
 * A new instance of an uninitialised Error object with automatic storage
 * allocation.
 */
#define KQT_ERROR_AUTO (&(kqt_Error){ .error = false, .message = { '\0' } })


#endif // K_ERROR_H


