

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


#define _POSIX_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <keyboard.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


static bool orig_saved = false;
static struct termios orig;


bool set_terminal(bool interactive, bool immediate)
{
    struct termios* term = &(struct termios){ .c_lflag = 0 };
    errno = 0;
    if (tcgetattr(STDIN_FILENO, term) != 0)
    {
        perror("Couldn't retrieve terminal attributes");
        return false;
    }
    if (!orig_saved)
    {
        memcpy(&orig, term, sizeof(struct termios));
        orig_saved = true;
    }
    if (interactive)
    {
        term->c_lflag &= ~(ICANON | ECHO | IEXTEN);
    }
    else
    {
        term->c_lflag = orig.c_lflag;
    }
    if (immediate)
    {
        term->c_cc[VMIN] = 0;
        term->c_cc[VTIME] = 0;
    }
    else
    {
        term->c_cc[VMIN] = orig.c_cc[VMIN];
        term->c_cc[VTIME] = orig.c_cc[VTIME];
    }
    errno = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, term))
    {
        perror("Couldn't set terminal attributes");
        return false;
    }
    return true;
}


int get_key(void)
{
    char key = 0;
    if (read(STDIN_FILENO, &key, 1) <= 0)
    {
        return -1;
    }
    if (key == '\033')
    {
        char keys[2] = { '\0' };
        int ret = read(STDIN_FILENO, keys, 2);
        if (ret < 2)
        {
            return -1;
        }
        if (keys[0] == '[')
        {
            if (keys[1] == 'A')
            {
                return KEY_UP;
            }
            else if (keys[1] == 'B')
            {
                return KEY_DOWN;
            }
            else if (keys[1] == 'C')
            {
                return KEY_RIGHT;
            }
            else if (keys[1] == 'D')
            {
                return KEY_LEFT;
            }
        }
        return -1;
    }
    return key;
}


