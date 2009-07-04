

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
#include <errno.h>

#include <keyboard.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


bool set_terminal(bool interactive, bool immediate)
{
    struct termios* term = &(struct termios){ .c_lflag = 0 };
    errno = 0;
    if (tcgetattr(STDIN_FILENO, term) != 0)
    {
        perror("Couldn't retrieve terminal attributes");
        return false;
    }
    if (interactive)
    {
        term->c_lflag &= ~ICANON;
        term->c_lflag &= ~ECHO;
    }
    else
    {
        term->c_lflag |= ICANON;
        term->c_lflag |= ECHO;
    }
    errno = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, term))
    {
        perror("Couldn't set terminal attributes");
        return false;
    }
    errno = 0;
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    if (flags == -1)
    {
        perror("Couldn't retrieve standard input attributes");
        return false;
    }
    if (immediate)
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1)
    {
        perror("Couldn't set standard input attributes");
        return false;
    }
    return true;
}


int get_key(void)
{
    char key = 0;
    if (fread(&key, 1, 1, stdin) == 0)
    {
        return -1;
    }
    if (key == '\033')
    {
        char keys[2] = { '\0' };
        int ret = fread(keys, 1, 2, stdin);
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


