

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <Bit_array.h>
#include <Connections_search.h>
#include <Event.h>
#include <Event_handler.h>
#include <events/Event_global_jump.h>
#include <memory.h>
#include <Pattern.h>
#include <Pattern_location.h>
#include <xassert.h>


struct Pattern
{
    Column* global;
    Column* aux;
    Column* cols[KQT_COLUMNS_MAX];
    AAtree* locations;
    AAiter* locations_iter;
    Tstamp length;
    Bit_array* existents;
};


Pattern* new_Pattern(void)
{
    Pattern* pat = memory_alloc_item(Pattern);
    if (pat == NULL)
    {
        return NULL;
    }

    pat->global = NULL;
    pat->aux = NULL;
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
        pat->cols[i] = NULL;
    pat->locations = NULL;
    pat->locations_iter = NULL;
    pat->existents = NULL;

    pat->global = new_Column(NULL);
    if (pat->global == NULL)
    {
        del_Pattern(pat);
        return NULL;
    }
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        pat->cols[i] = new_Column(NULL);
        if (pat->cols[i] == NULL)
        {
            del_Pattern(pat);
            return NULL;
        }
    }
    pat->aux = new_Column_aux(NULL, pat->cols[0], 0);
    pat->locations = new_AAtree(
            (int (*)(const void*, const void*))Pattern_location_cmp,
            memory_free);
    pat->locations_iter = new_AAiter(pat->locations);
    pat->existents = new_Bit_array(KQT_PAT_INSTANCES_MAX);
    if (pat->aux == NULL ||
            pat->locations == NULL ||
            pat->locations_iter == NULL ||
            pat->existents == NULL)
    {
        del_Pattern(pat);
        return NULL;
    }
    Tstamp_set(&pat->length, 16, 0);
    return pat;
}


bool Pattern_parse_header(Pattern* pat, char* str, Read_state* state)
{
    assert(pat != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Tstamp* len = PATTERN_DEFAULT_LENGTH;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        str = read_const_string(str, "length", state);
        str = read_const_char(str, ':', state);
        str = read_tstamp(str, len, state);
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            return false;
        }
    }
    if (Tstamp_get_beats(len) < 0)
    {
        Read_state_set_error(state, "Pattern length is negative");
        return false;
    }
    Pattern_set_length(pat, len);
    return true;
}


void Pattern_set_inst_existent(Pattern* pat, int index, bool existent)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_PAT_INSTANCES_MAX);

    Bit_array_set(pat->existents, index, existent);

    return;
}


bool Pattern_get_inst_existent(const Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_PAT_INSTANCES_MAX);

    return Bit_array_get(pat->existents, index);
}


Column* Pattern_get_column(const Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);

    return pat->cols[index];
}


bool Pattern_set_location(Pattern* pat, int song, Pat_inst_ref* piref)
{
    assert(pat != NULL);
    assert(song >= 0);
    assert(song < KQT_SONGS_MAX);
    assert(piref != NULL);
    Pattern_location* key = PATTERN_LOCATION_AUTO;
    key->song = song;
    key->piref = *piref;
    if (AAtree_get_exact(pat->locations, key) != NULL)
    {
        return true;
    }
    key = new_Pattern_location(song, piref);
    if (key == NULL || !AAtree_ins(pat->locations, key))
    {
        memory_free(key);
        return false;
    }
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Column_update_locations(pat->cols[i],
                                     pat->locations, pat->locations_iter))
        {
            return false;
        }
    }
    return true;
}


AAtree* Pattern_get_locations(Pattern* pat, AAiter** iter)
{
    assert(pat != NULL);
    assert(iter != NULL);
    *iter = pat->locations_iter;
    return pat->locations;
}


void Pattern_set_length(Pattern* pat, Tstamp* length)
{
    assert(pat != NULL);
    assert(length != NULL);
    assert(length->beats >= 0);
    Tstamp_copy(&pat->length, length);
    return;
}


const Tstamp* Pattern_get_length(const Pattern* pat)
{
    assert(pat != NULL);
    return &pat->length;
}


bool Pattern_set_col(Pattern* pat, int index, Column* col)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(col != NULL);
    Column* new_aux = new_Column_aux(pat->aux, col, index);
    if (new_aux == NULL)
    {
        return false;
    }
    Column* old_aux = pat->aux;
    pat->aux = new_aux;
    del_Column(old_aux);
    Column* old_col = pat->cols[index];
    pat->cols[index] = col;
    del_Column(old_col);
    return true;
}


Column* Pattern_get_col(Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    return pat->cols[index];
}


void Pattern_set_global(Pattern* pat, Column* col)
{
    assert(pat != NULL);
    assert(col != NULL);
    Column* old_col = pat->global;
    pat->global = col;
    del_Column(old_col);
    return;
}


Column* Pattern_get_global(Pattern* pat)
{
    assert(pat != NULL);
    return pat->global;
}


void del_Pattern(Pattern* pat)
{
    if (pat == NULL)
    {
        return;
    }
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        del_Column(pat->cols[i]);
    }
    del_Column(pat->global);
    del_Column(pat->aux);
    del_AAtree(pat->locations);
    del_AAiter(pat->locations_iter);
    del_Bit_array(pat->existents);
    memory_free(pat);
    return;
}


