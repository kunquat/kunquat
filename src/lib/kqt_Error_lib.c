

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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include <kqt_Error.h>
#include <kqt_Error_lib.h>


void kqt_Error_set(kqt_Error* error, char* message, ...)
{
    assert(error != NULL);
    assert(message != NULL);
    error->error = true;
    va_list args;
    va_start(args, message);
    vsnprintf(error->message, KQT_ERROR_LENGTH, message, args);
    va_end(args);
    error->message[KQT_ERROR_LENGTH - 1] = '\0';
    return;
}


