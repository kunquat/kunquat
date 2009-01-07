

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
#include <wchar.h>
#include <stdbool.h>
#include <errno.h>


#define REPLACEMENT_CHAR ((wchar_t)0xfffd)


int from_utf8(wchar_t* dest, unsigned char* src, int len)
{
    assert(dest != NULL);
    assert(src != NULL);
    if (len <= 0)
    {
        *dest = L'\0';
        return 0;
    }
    --len;
    int ret = 0;
    int seq_len = 0;
    bool il_chunk = false;
    bool check_first_cont = false;
    while (*src != '\0' && len > 0)
    {
        if (seq_len > 0)
        {
            if ((*src & 0xc0) != 0x80) // unexpected end of continuation bytes
            {
                ret = EILSEQ;
                if (!il_chunk)
                {
                    *dest = REPLACEMENT_CHAR;
                    ++dest;
                    --len;
                }
                seq_len = 0;
                il_chunk = false;
                check_first_cont = false;
                continue;
            }
            else if (!il_chunk && (*src & 0xc0) == 0x80)
            {
                if (check_first_cont)
                {
                    check_first_cont = false;
                    unsigned char mask = 0x80;
                    mask >>= seq_len;
                    --mask;
                    mask = ~mask & 0x7f;
                    if ((*src & mask) == 0) // overlong sequence
                    {
                        ret = EILSEQ;
                        il_chunk = true;
                        *dest = REPLACEMENT_CHAR;
                        ++dest;
                        --len;
                        --seq_len;
                        ++src;
                        continue;
                    }
                }
                *dest <<= 6;
                *dest |= *src & 0x3f;
                if (seq_len == 1)
                {
                    if ((*dest >= 0xd800 && *dest <= 0xdfff)
                            || *dest == 0xfffe || *dest == 0xffff)
                    {
                        ret = EILSEQ; // surrogate or illegal character
                        *dest = REPLACEMENT_CHAR;
                    }
                    ++dest;
                    --len;
                }
            }
            --seq_len;
        }
        else
        {
            il_chunk = false;
            check_first_cont = false;
            if ((*src & 0xc0) == 0x80) // unexpected continuation byte
            {
                ret = EILSEQ;
                *dest = REPLACEMENT_CHAR;
                ++dest;
                --len;
            }
            else if ((*src & 0x80) == 0) // single-byte sequence
            {
                *dest = *src;
                ++dest;
                --len;
            }
            else
            {
                unsigned char mask = 0x80;
                while ((*src & mask) != 0)
                {
                    mask >>= 1;
                    ++seq_len;
                }
                --mask;
                unsigned char bits = *src & mask;
                assert(seq_len >= 2);
                if (seq_len > 6)
                {
                    ret = EILSEQ;
                    il_chunk = true;
                    *dest = REPLACEMENT_CHAR;
                    ++dest;
                    --len;
                }
                else if (seq_len == 2 && (bits & 0xfe) == 0) // overlong sequence
                {
                    ret = EILSEQ;
                    il_chunk = true;
                    *dest = REPLACEMENT_CHAR;
                    ++dest;
                    --len;
                }
                else
                {
                    if (bits == 0)
                    {
                        check_first_cont = true;
                    }
                    il_chunk = false;
                    *dest = *src & mask;
                    --seq_len;
                }
            }
        }
        ++src;
    }
    if (seq_len > 0)
    {
        assert(len > 0);
        ret = EILSEQ;
        *dest = REPLACEMENT_CHAR;
        ++dest;
    }
    *dest = L'\0';
    return ret;
}


int to_utf8(unsigned char* dest, wchar_t* src, int len)
{
    assert(dest != NULL);
    assert(src != NULL);
    if (len <= 0)
    {
        *dest = '\0';
        return 0;
    }
    --len;
    int ret = 0;
    while (*src != L'\0' && len > 0)
    {
        int seq_len = 0;
        wchar_t ch = *src;
        if (ch < 0 || ch > 0x7fffffff
                || (ch >= 0xd800 && ch <= 0xdfff)
                || ch == 0xfffe || ch == 0xffff)
        {
            ret = EILSEQ;
            ch = REPLACEMENT_CHAR;
        }
        if (ch <= 0x7f)
        {
            seq_len = 0;
            *dest = ch;
            ++dest;
            --len;
        }
        else if (ch <= 0x7ff)
        {
            seq_len = 1;
        }
        else if (ch <= 0xffff)
        {
            seq_len = 2;
        }
        else if (ch <= 0x1fffff)
        {
            seq_len = 3;
        }
        else if (ch <= 0x3ffffff)
        {
            seq_len = 4;
        }
        else
        {
            seq_len = 5;
        }
        int shift = seq_len * 6;
        if (seq_len >= len)
        {
            seq_len = len = 0;
            *dest = '\0';
            ++dest;
        }
        else if (seq_len > 0)
        {
            *dest = ~(unsigned char)((0x80 >> seq_len) - 1);
            *dest |= ch >> shift;
            ++dest;
            --len;
        }
        while (seq_len > 0)
        {
            assert(len > 0);
            shift -= 6;
            *dest = 0x80 | (unsigned char)((ch >> shift) & 0x3f);
            ++dest;
            --len;
            --seq_len;
        }
        ++src;
    }
    *dest = '\0';
    return ret;
}


