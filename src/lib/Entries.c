

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <AAtree.h>
#include <Entries.h>
#include <Parse_manager.h>

#include <xmemory.h>


struct Entries
{
    int biggest_version;
    AAtree* tree;
};


typedef struct Entry
{
    char key[256];
    void* data;
    int32_t length;
} Entry;


static Entry* new_Entry(const char* key, void* data, int32_t length);

static int Entry_cmp(const Entry* e1, const Entry* e2);

static void del_Entry(Entry* entry);


static char* find_wildcard(char* key);

static void resolve_wildcard(Entries* entries, Entry* key_entry);


static Entry* new_Entry(const char* key, void* data, int32_t length)
{
    assert(key != NULL);
    assert(data != NULL || length == 0);
    assert(length >= 0);
    Entry* entry = xalloc(Entry);
    if (entry == NULL)
    {
        return NULL;
    }
    memset(entry->key, '\0', 256);
    strncpy(entry->key, key, 255);
    entry->data = data;
    entry->length = length;
    return entry;
}


static int Entry_cmp(const Entry* e1, const Entry* e2)
{
    assert(e1 != NULL);
    assert(e2 != NULL);
    return strcmp(e1->key, e2->key);
}


static void del_Entry(Entry* entry)
{
    assert(entry != NULL);
    assert(entry->data != NULL);
    xfree(entry->data);
    xfree(entry);
    return;
}


Entries* new_Entries(void)
{
    Entries* entries = xalloc(Entries);
    if (entries == NULL)
    {
        return NULL;
    }
    entries->biggest_version = 0;
    entries->tree = new_AAtree((int (*)(const void*, const void*))Entry_cmp,
                               (void (*)(void*))del_Entry);
    if (entries->tree == NULL)
    {
        xfree(entries);
        return NULL;
    }
    return entries;
}


bool Entries_set(Entries* entries,
                 const char* key,
                 void* data,
                 int32_t length)
{
    assert(entries != NULL);
    assert(key != NULL);
    assert(data != NULL || length == 0);
    assert(length >= 0);
    Entry* key_entry = &(Entry){ .key[0] = '\0' };
    memset(key_entry->key, '\0', 256);
    strncpy(key_entry->key, key, 255);
    Entry* entry = new_Entry(key, data, length);
    if (entry == NULL)
    {
        return false;
    }
    if (!AAtree_ins(entries->tree, entry))
    {
        del_Entry(entry);
        return false;
    }
    const char* prefix = "kunquatc";
    char* num_str = strstr(entry->key, prefix);
    if (num_str == NULL)
    {
        prefix = "kunquats";
        num_str = strstr(entry->key, prefix);
    }
    if (num_str != NULL)
    {
        int version = parse_index_dir(entry->key, prefix, 2);
        if (version > entries->biggest_version)
        {
            entries->biggest_version = version;
        }
    }
    return true;
}


void* Entries_get_data(Entries* entries, const char* key)
{
    assert(entries != NULL);
    assert(key != NULL);
    Entry* key_entry = &(Entry){ .key[0] = '\0' };
    memset(key_entry->key, '\0', 256);
    strncpy(key_entry->key, key, 255);
    resolve_wildcard(entries, key_entry);
    Entry* entry = AAtree_get_exact(entries->tree, key_entry);
    if (entry == NULL)
    {
        return NULL;
    }
    return entry->data;
}


int32_t Entries_get_length(Entries* entries, const char* key)
{
    assert(entries != NULL);
    assert(key != NULL);
    Entry* key_entry = &(Entry){ .key[0] = '\0' };
    memset(key_entry->key, '\0', 256);
    strncpy(key_entry->key, key, 255);
    resolve_wildcard(entries, key_entry);
    Entry* entry = AAtree_get_exact(entries->tree, key_entry);
    if (entry == NULL)
    {
        return 0;
    }
    return entry->length;
}


static char* find_wildcard(char* key)
{
    char* header_location = strstr(key, "kunquatiXX");
    if (header_location == NULL)
    {
        header_location = strstr(key, "kunquatsXX");
        if (header_location == NULL)
        {
            return NULL;
        }
    }
    char* num_location = strstr(header_location, "XX");
    assert(num_location != NULL);
    return num_location;
}


static void resolve_wildcard(Entries* entries, Entry* key_entry)
{
    assert(entries != NULL);
    assert(key_entry != NULL);
    char* num_location = find_wildcard(key_entry->key);
    if (num_location == NULL)
    {
        return;
    }
    char num_str[3] = "00";
    int last_found = -1;
    for (int i = 0; i <= entries->biggest_version; ++i)
    {
        snprintf(num_str, 3, "%02x", i);
        strncpy(num_location, num_str, 2);
        if (AAtree_get_exact(entries->tree, key_entry) != NULL)
        {
            last_found = i;
        }
    }
    if (last_found == -1)
    {
        strncpy(num_location, "XX", 2);
    }
    else
    {
        snprintf(num_str, 3, "%02x", last_found);
        strncpy(num_location, num_str, 2);
    }
    return;
}


void del_Entries(Entries* entries)
{
    assert(entries != NULL);
    del_AAtree(entries->tree);
    xfree(entries);
    return;
}


