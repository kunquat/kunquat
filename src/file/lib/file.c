

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <kunquat/File.h>

#include <kunquat/Handle.h>
#include <kunquat/limits.h>

#include <zip.h>

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ERROR_LENGTH_MAX 512
#define MODULES_MAX 256


typedef struct Zip_state
{
    zip_t* archive;
    zip_uint64_t entry_index;
    zip_uint64_t entry_count;
} Zip_state;


#define ZIP_STATE_AUTO (&(Zip_state){ \
        .archive = NULL, .entry_index = 0, .entry_count = 0 })


static void Zip_state_deinit(Zip_state* zstate)
{
    assert(zstate != NULL);

    if (zstate->archive != NULL)
        zip_discard(zstate->archive);

    zstate->archive = NULL;
    zstate->entry_index = 0;
    zstate->entry_count = 0;

    return;
}


static bool Zip_state_init(Zip_state* zstate, const char* path)
{
    assert(zstate != NULL);
    assert(path != NULL);

    if (zstate->archive != NULL)
        Zip_state_deinit(zstate);

    int error = ZIP_ER_OK;
    zstate->archive = zip_open(path, ZIP_RDONLY, &error);
    if (zstate->archive == NULL)
        return false;

    zstate->entry_index = 0;
    const zip_int64_t entry_count = zip_get_num_entries(zstate->archive, 0);
    assert(entry_count >= 0);
    zstate->entry_count = (zip_uint64_t)entry_count;

    return true;
}


typedef struct Vector
{
    size_t size;
    size_t capacity;
    size_t elem_size;
    void (*destroy)(void* data);

    void* data;
} Vector;


static void Vector_deinit(Vector* vector)
{
    assert(vector != NULL);

    char* data_bytes = vector->data;

    if ((data_bytes != NULL) && (vector->destroy != NULL))
    {
        assert(sizeof(char*) == vector->elem_size);

        for (size_t i = 0; i < vector->size; ++i)
        {
            char* p = NULL;
            memcpy(&p, &data_bytes[vector->elem_size * i], vector->elem_size);

            vector->destroy(p);
        }
    }

    free(vector->data);
    vector->data = NULL;

    vector->size = 0;
    vector->capacity = 0;
    vector->elem_size = 0;
    vector->destroy = NULL;

    return;
}


static bool Vector_inc_capacity(Vector* vector)
{
    assert(vector != NULL);

    const size_t new_cap = (vector->capacity == 0) ? 4 : vector->capacity * 2;

    void* new_data = realloc(vector->data, new_cap * vector->elem_size);
    if (new_data == NULL)
        return false;

    vector->data = new_data;

    char* data_bytes = vector->data;

    const size_t cap_inc = new_cap - vector->capacity;
    memset(data_bytes + (vector->capacity * vector->elem_size),
            0,
            cap_inc * vector->elem_size);

    vector->capacity = new_cap;

    return true;
}


static bool Vector_add(Vector* vector, const void* elem)
{
    assert(vector != NULL);
    assert(elem != NULL);

    if (vector->size >= vector->capacity)
    {
        assert(vector->size == vector->capacity);
        if (!Vector_inc_capacity(vector))
            return false;
    }

    char* data_bytes = vector->data;

    memcpy(&data_bytes[vector->size * vector->elem_size], elem, vector->elem_size);
    ++vector->size;

    return true;
}


static long Vector_get_size(const Vector* vector)
{
    assert(vector != NULL);
    return (long)vector->size;
}


static void* Vector_get_data(Vector* vector)
{
    assert(vector != NULL);
    return vector->data;
}


static void Vector_init(Vector* vector, long elem_size, void (*destroy)(void*))
{
    assert(vector != NULL);
    assert(elem_size > 0);

    vector->size = 0;
    vector->capacity = 0;
    vector->elem_size = (size_t)elem_size;
    vector->destroy = destroy;
    vector->data = NULL;

    return;
}


typedef struct Kept_entries
{
    Vector keys;
    Vector sizes;
    Vector values;
} Kept_entries;


static void Kept_entries_init(Kept_entries* entries)
{
    assert(entries != NULL);

    Vector_init(&entries->keys, sizeof(char*), free);
    Vector_init(&entries->sizes, sizeof(long), NULL);
    Vector_init(&entries->values, sizeof(char*), free);

    return;
}


static void Kept_entries_deinit(Kept_entries* entries)
{
    assert(entries != NULL);

    Vector_deinit(&entries->keys);
    Vector_deinit(&entries->sizes);
    Vector_deinit(&entries->values);

    return;
}


// NOTE: This function assumes ownership of value, thus it must not be freed
//       after a successful call.
static bool Kept_entries_add_entry(
        Kept_entries* entries, const char* key, long size, char* value)
{
    assert(entries != NULL);
    assert(key != NULL);
    assert(size > 0);
    assert(value != NULL);

    const size_t key_length = strlen(key);
    assert(key_length > 0);
    char* copied_key = calloc(key_length + 1, sizeof(char));
    if (copied_key == NULL)
    {
        Kept_entries_deinit(entries);
        return false;
    }
    strcpy(copied_key, key);

    if (!Vector_add(&entries->keys, &copied_key))
    {
        free(copied_key);
        Kept_entries_deinit(entries);
        return false;
    }

    if (!Vector_add(&entries->sizes, &size) || !Vector_add(&entries->values, &value))
    {
        Kept_entries_deinit(entries);
        return false;
    }

    return true;
}


static long Kept_entries_get_entry_count(const Kept_entries* entries)
{
    assert(entries != NULL);
    assert(Vector_get_size(&entries->keys) == Vector_get_size(&entries->sizes));
    assert(Vector_get_size(&entries->keys) == Vector_get_size(&entries->values));

    return Vector_get_size(&entries->keys);
}


static const char** Kept_entries_get_keys(Kept_entries* entries)
{
    assert(entries != NULL);
    return (const char**)Vector_get_data(&entries->keys);
}


static const long* Kept_entries_get_sizes(Kept_entries* entries)
{
    assert(entries != NULL);
    return (const long*)Vector_get_data(&entries->sizes);
}


static const char** Kept_entries_get_values(Kept_entries* entries)
{
    assert(entries != NULL);
    return (const char**)Vector_get_data(&entries->values);
}


typedef struct Module
{
    char error[ERROR_LENGTH_MAX + 1];
    kqt_Handle handle;

    Zip_state zip_state;

    Kqtfile_keep_flags keep_flags;
    Kept_entries kept_entries;
} Module;

#define MODULE_AUTO (&(Module){             \
        .error = "",                        \
        .handle = 0,                        \
        .zip_state = *ZIP_STATE_AUTO,       \
        .keep_flags = KQTFILE_KEEP_NONE,    \
    })


static char null_error[ERROR_LENGTH_MAX + 1] = "";


static Module* modules[MODULES_MAX] = { NULL };


static bool kqt_Module_is_valid(kqt_Module module)
{
    if (module <= 0)
        return false;

    module -= 1;
    return (module < MODULES_MAX) && (modules[module] != NULL);
}


static Module* get_module(kqt_Module id)
{
    assert(kqt_Module_is_valid(id));

    Module* module = modules[id - 1];
    assert(module != NULL);

    return module;
}


const char* kqt_Module_get_error(kqt_Module id)
{
    if (!kqt_Module_is_valid(id))
        return null_error;

    Module* module = get_module(id);
    return module->error;
}


void kqt_Module_clear_error(kqt_Module id)
{
    if (!kqt_Module_is_valid(id))
    {
        memset(null_error, 0, ERROR_LENGTH_MAX + 1);
        return;
    }

    Module* module = get_module(id);
    assert(module != NULL);
    memset(module->error, 0, ERROR_LENGTH_MAX + 1);

    return;
}


static void set_error_va_list(Module* module, const char* message, va_list args)
{
    assert(message != NULL);

    memset(null_error, 0, ERROR_LENGTH_MAX + 1);
    vsnprintf(null_error, ERROR_LENGTH_MAX + 1, message, args);

    if (module != NULL)
    {
        strncpy(module->error, null_error, ERROR_LENGTH_MAX + 1);
    }

    return;
}


static void set_error(Module* module, const char* message, ...)
{
    assert(message != NULL);

    va_list args;
    va_start(args, message);
    set_error_va_list(module, message, args);
    va_end(args);

    return;
}


static kqt_Module add_module(Module* module)
{
    assert(module != NULL);

    for (int i = 0; i < MODULES_MAX; ++i)
        assert(modules[i] != module);

    static int next_try = 0;

    for (int i = 0; i < MODULES_MAX; ++i)
    {
        const int try = (i + next_try) % MODULES_MAX;
        if (modules[try] == NULL)
        {
            modules[try] = module;
            next_try = try + 1;
            return try + 1; // shift kqt_Module range to [1, MODULES_MAX]
        }
    }

    set_error(NULL, "Maximum number of Kunquat Modules reached");
    return 0;
}


static bool remove_module(kqt_Module module)
{
    assert(kqt_Module_is_valid(module));

    const bool was_null = (modules[module - 1] == NULL);
    modules[module - 1] = NULL;

    return !was_null;
}


static bool Module_init_with_handle(Module* module, kqt_Handle handle)
{
    assert(module != NULL);
    assert(handle != 0);

    if (module->handle != 0)
    {
        set_error(module, "Cannot initialise a module more than once");
        return false;
    }

    memset(module->error, 0, ERROR_LENGTH_MAX);
    module->zip_state = *ZIP_STATE_AUTO;

    module->keep_flags = KQTFILE_KEEP_NONE;
    Kept_entries_init(&module->kept_entries);

    module->handle = handle;

    return true;
}


static bool Module_init(Module* module)
{
    assert(module != NULL);

    if (module->handle != 0)
    {
        set_error(module, "Cannot initialise a module more than once");
        return false;
    }

    kqt_Handle handle = kqt_new_Handle();
    if (handle == 0)
    {
        set_error(module, "Could not create a Kunquat Handle for module");
        return false;
    }

    return Module_init_with_handle(module, handle);
}


static bool Module_is_error_set(const Module* module)
{
    assert(module != NULL);
    return (module->error[0] != '\0');
}


static kqt_Handle Module_remove_handle(Module* module)
{
    assert(module != NULL);

    kqt_Handle ret = module->handle;
    module->handle = 0;

    return ret;
}


static bool Module_is_loading(const Module* module)
{
    assert(module != NULL);
    return (module->zip_state.archive != NULL) &&
        (module->zip_state.entry_index < module->zip_state.entry_count) &&
        !Module_is_error_set(module);
}


static bool Module_should_keep_key(const Module* module, const char* key)
{
    assert(module != NULL);
    assert(key != NULL);

    if (module->keep_flags == KQTFILE_KEEP_NONE)
        return false;

    const char* last_pos = strrchr(key, '/');
    if (last_pos != NULL)
        ++last_pos;
    else
        last_pos = key;

    if ((module->keep_flags & KQTFILE_KEEP_RENDER_DATA) != 0)
    {
        if (strncmp(last_pos, "p_", 2) == 0)
            return true;
    }

    if ((module->keep_flags & KQTFILE_KEEP_PLAYER_DATA) != 0)
    {
        if ((strncmp(last_pos, "m_", 2) == 0) ||
                (strcmp(key, "album/p_tracks.json") == 0))
            return true;
    }

    if ((module->keep_flags & KQTFILE_KEEP_INTERFACE_DATA) != 0)
    {
        if (strncmp(last_pos, "i_", 2) == 0)
            return true;
    }

    return false;
}


static bool Module_load_step(Module* module)
{
    assert(module != NULL);
    assert(module->handle != 0);
    assert(module->zip_state.archive != NULL);

    if (Module_is_error_set(module))
        return false;

    Zip_state* zstate = &module->zip_state;
    assert(zstate->archive != NULL);

    if (zstate->entry_index >= zstate->entry_count)
    {
        assert(zstate->entry_index == zstate->entry_count);
        return false;
    }

    int error = ZIP_ER_OK;

    zip_stat_t stat;
    error = zip_stat_index(zstate->archive, zstate->entry_index, 0, &stat);
    if (error != ZIP_ER_OK)
    {
        set_error(module, "%s", zip_strerror(zstate->archive));
        return false;
    }

    const char* entry_path = stat.name;
    static const char* header = "kqtc00";

    if (strlen(entry_path) < strlen(header) ||
            strncmp(entry_path, header, strlen(header)) != 0 ||
            (entry_path[strlen(header)] != '/' &&
             entry_path[strlen(header)] != '\0'))
    {
        set_error(module, "The file contains an invalid data entry: `%s`", entry_path);
        return false;
    }

    if (stat.size > (zip_uint64_t)LONG_MAX)
    {
        set_error(module, "Entry %s is too large (%lld bytes)",
                entry_path, (long long)stat.size);
        return false;
    }

    const char* key = strchr(entry_path, '/');
    if (key != NULL && (strlen(key) > 0) && (key[strlen(key) - 1] != '/'))
    {
        ++key;

        zip_file_t* f = zip_fopen_index(zstate->archive, zstate->entry_index, 0);
        if (f == NULL)
        {
            set_error(module, "%s", zip_strerror(zstate->archive));
            return false;
        }

        char* data = malloc(sizeof(char) * stat.size);
        if (data == NULL)
        {
            set_error(module, "Could not allocate memory for module data");
            zip_fclose(f);
            return false;
        }

        const zip_int64_t read_count = zip_fread(f, data, stat.size);
        if (read_count < (zip_int64_t)stat.size)
        {
            set_error(module,
                    "Unexpected end of entry %s at %lld bytes"
                    " (expected %lld bytes)",
                    entry_path, (long long)read_count, (long long)stat.size);
            free(data);
            zip_fclose(f);
            return false;
        }

        zip_fclose(f);

        if (!kqt_Handle_set_data(module->handle, key, data, (long int)stat.size))
        {
            set_error(module,
                    "Could not set data: %s",
                    kqt_Handle_get_error_message(module->handle));
            free(data);
            return false;
        }

        if (Module_should_keep_key(module, key))
        {
            if (!Kept_entries_add_entry(
                        &module->kept_entries, key, (long)stat.size, data))
            {
                set_error(module,
                        "Could not allocate memory for key %s", key);
                free(data);
            }
        }
        else
        {
            free(data);
        }
    }

    ++zstate->entry_index;

    return true;
}


static bool Module_load(Module* module, const char* path)
{
    assert(module != NULL);
    assert(module->handle != 0);
    assert(path != NULL);

    if (!Zip_state_init(&module->zip_state, path))
    {
        set_error(module, "Could not open `%s`", path);
        return false;
    }

    while (Module_is_loading(module))
        Module_load_step(module);

    Zip_state_deinit(&module->zip_state);

    return !Module_is_error_set(module);
}


static void Module_deinit(Module* module)
{
    assert(module != NULL);

    if (module->handle != 0)
    {
        kqt_del_Handle(module->handle);
        module->handle = 0;
    }

    Zip_state_deinit(&module->zip_state);
    Kept_entries_deinit(&module->kept_entries);

    return;
}


kqt_Module kqt_new_Module_with_handle(kqt_Handle handle)
{
    Module* m = malloc(sizeof(Module));
    if (m == NULL)
    {
        set_error(NULL, "Could not allocate memory for a new Kunquat Module");
        return 0;
    }

    *m = *MODULE_AUTO;

    if (!Module_init_with_handle(m, handle))
    {
        free(m);
        return 0;
    }

    kqt_Module module = add_module(m);
    if (module == 0)
    {
        free(m);
        return 0;
    }

    return module;
}


#define check_module(module, ret)                                       \
    if (true)                                                           \
    {                                                                   \
        if (!kqt_Module_is_valid(module))                               \
        {                                                               \
            set_error(NULL, "Module is not valid: %d", (int)module);    \
            return (ret);                                               \
        }                                                               \
    }                                                                   \
    else (void)0


#define check_module_void(module)                                       \
    if (true)                                                           \
    {                                                                   \
        if (!kqt_Module_is_valid(module))                               \
        {                                                               \
            set_error(NULL, "Module is not valid: %d", (int)module);    \
            return;                                                     \
        }                                                               \
    }                                                                   \
    else (void)0


void kqt_del_Module(kqt_Module module)
{
    check_module_void(module);

    Module* m = get_module(module);

    if (!remove_module(module))
    {
        set_error(NULL, "Invalid Kunquat Module: %d", module);
        return;
    }

    m->handle = 0;
    Module_deinit(m);
    free(m);

    return;
}


int kqt_Module_set_keep_flags(kqt_Module module, Kqtfile_keep_flags flags)
{
    check_module(module, 0);
    Module* m = get_module(module);

    if (flags > KQTFILE_KEEP_ALL_DATA)
    {
        set_error(m, "Flags must be a valid combination of Kqtfile_keep_flags");
        return 0;
    }

    m->keep_flags = flags;

    return 1;
}


int kqt_Module_open_file(kqt_Module module, const char* path)
{
    check_module(module, 0);
    Module* m = get_module(module);

    if (path == NULL)
    {
        set_error(m, "path must not be NULL");
        return 0;
    }

    if (!Zip_state_init(&m->zip_state, path))
    {
        set_error(m, "Could not open `%s`", path);
        return 0;
    }

    return 1;
}


int kqt_Module_load_step(kqt_Module module)
{
    check_module(module, 0);
    Module* m = get_module(module);

    if (m->zip_state.archive == NULL)
    {
        set_error(m, "Module %d has no file open for reading", (int)module);
        return 0;
    }

    return Module_load_step(m);
}


double kqt_Module_get_loading_progress(kqt_Module module)
{
    check_module(module, 0);
    Module* m = get_module(module);

    if (m->zip_state.archive == NULL)
    {
        set_error(m, "Module %d has no file open for reading", (int)module);
        return 0;
    }

    if (m->zip_state.entry_count == 0)
        return 1;

    return ((double)m->zip_state.entry_index / (double)m->zip_state.entry_count);
}


void kqt_Module_close_file(kqt_Module module)
{
    check_module_void(module);
    Module* m = get_module(module);

    Zip_state_deinit(&m->zip_state);

    return;
}


long kqt_Module_get_kept_entry_count(kqt_Module module)
{
    check_module(module, -1);
    Module* m = get_module(module);

    return Kept_entries_get_entry_count(&m->kept_entries);
}


const char** kqt_Module_get_kept_keys(kqt_Module module)
{
    check_module(module, NULL);
    Module* m = get_module(module);

    return Kept_entries_get_keys(&m->kept_entries);
}


const long* kqt_Module_get_kept_entry_sizes(kqt_Module module)
{
    check_module(module, NULL);
    Module* m = get_module(module);

    return Kept_entries_get_sizes(&m->kept_entries);
}


const char** kqt_Module_get_kept_entries(kqt_Module module)
{
    check_module(module, NULL);
    Module* m = get_module(module);

    return Kept_entries_get_values(&m->kept_entries);
}


int kqt_Module_free_kept_entries(kqt_Module module)
{
    check_module(module, 0);
    Module* m = get_module(module);

    Kept_entries_deinit(&m->kept_entries);

    return 1;
}


kqt_Handle kqtfile_load_module(const char* path, int thread_count)
{
    if (path == NULL)
    {
        set_error(NULL, "path must not be NULL");
        return 0;
    }

    if (thread_count < 1)
    {
        set_error(NULL, "thread_count must be positive");
        return 0;
    }
    else if (thread_count > KQT_THREADS_MAX)
    {
        set_error(NULL, "thread_count must not be greater than %d", KQT_THREADS_MAX);
        return 0;
    }

    Module* module = MODULE_AUTO;
    if (!Module_init(module))
    {
        return 0;
    }

    kqt_Handle_set_loader_thread_count(module->handle, thread_count);

    if (!Module_load(module, path))
    {
        Module_deinit(module);
        return 0;
    }

    kqt_Handle handle = Module_remove_handle(module);
    Module_deinit(module);

    if (!kqt_Handle_validate(handle))
    {
        set_error(NULL,
                "Could not validate Kunquat file: %s",
                kqt_Handle_get_error_message(handle));
        kqt_del_Handle(handle);
        return false;
    }

    return handle;
}


