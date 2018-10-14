

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


static bool Zip_state_init(Zip_state* zstate, const char* path)
{
    assert(zstate != NULL);
    assert(path != NULL);

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


static void Zip_state_deinit(Zip_state* zstate)
{
    assert(zstate != NULL);

    if (zstate->archive == NULL)
        return;

    zip_discard(zstate->archive);
    zstate->archive = NULL;
}


typedef struct Module
{
    char error[ERROR_LENGTH_MAX + 1];
    kqt_Handle handle;

    Zip_state zip_state;
} Module;

#define MODULE_AUTO (&(Module){ .error = "", .handle = 0 })


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


static bool Module_is_error_set(const Module* module)
{
    assert(module != NULL);
    return (module->error[0] != '\0');
}


static bool Module_init(Module* module)
{
    assert(module != NULL);

    if (module->handle != 0)
    {
        set_error(module, "Cannot initialise a module more than once");
        return false;
    }

    memset(module->error, 0, ERROR_LENGTH_MAX);

    module->handle = kqt_new_Handle();
    if (module->handle == 0)
    {
        set_error(module, "Could not create a Kunquat Handle for module");
        return false;
    }

    module->zip_state = *ZIP_STATE_AUTO;

    return true;
}


static kqt_Handle Module_remove_handle(Module* module)
{
    assert(module != NULL);

    kqt_Handle ret = module->handle;
    module->handle = 0;

    return ret;
}


static void Module_deinit(Module* module)
{
    assert(module != NULL);

    if (module->handle != 0)
    {
        kqt_del_Handle(module->handle);
        module->handle = 0;
    }

    return;
}


static bool Module_is_reading(const Module* module)
{
    assert(module != NULL);
    return (module->zip_state.archive != NULL) &&
        (module->zip_state.entry_index < module->zip_state.entry_count);
}


static bool Module_load_step(Module* module)
{
    assert(module != NULL);
    assert(module->handle != 0);
    assert(module->zip_state.archive != NULL);

    if (Module_is_error_set(module))
        return false;

    Zip_state* zstate = &module->zip_state;

    if (zstate->entry_index >= zstate->entry_count)
    {
        assert(zstate->entry_index == zstate->entry_count);
        return true;
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

        free(data); // TODO: store data for read-only access if needed
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

    while (Module_is_reading(module))
    {
        if (!Module_load_step(module))
            break;
    }

    Zip_state_deinit(&module->zip_state);

    if (Module_is_error_set(module))
        return false;

    if (!kqt_Handle_validate(module->handle))
    {
        set_error(module,
                "Could not validate Kunquat file: %s",
                kqt_Handle_get_error_message(module->handle));
        return false;
    }

    return true;
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

    return handle;
}


