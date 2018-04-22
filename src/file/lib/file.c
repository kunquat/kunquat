

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


typedef struct Module
{
    char error[ERROR_LENGTH_MAX + 1];
    kqt_Handle handle;
} Module;

#define MODULE_AUTO (&(Module){ .error = "", .handle = 0 })


char null_error[ERROR_LENGTH_MAX + 1] = "";


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


const char* kqtfile_get_error(kqt_Module id)
{
    if (!kqt_Module_is_valid(id))
        return null_error;

    Module* module = get_module(id);
    return module->error;
}


static void set_error_va_list(Module* module, const char* message, va_list args)
{
    assert(message != NULL);

    memset(null_error, 0, ERROR_LENGTH_MAX + 1);
    vsnprintf(null_error, ERROR_LENGTH_MAX + 1, message, args);

    if (module != NULL)
    {
        memset(module->error, 0, ERROR_LENGTH_MAX + 1);
        strncpy(module->error, null_error, ERROR_LENGTH_MAX);
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


#define zip_fail_if(cond)                                   \
    if (true)                                               \
    {                                                       \
        if ((cond))                                         \
        {                                                   \
            set_error(module, "%s", zip_strerror(archive)); \
            zip_discard(archive);                           \
            return false;                                   \
        }                                                   \
    }                                                       \
    else (void)0

static bool Module_load(Module* module, const char* path)
{
    assert(module != NULL);
    assert(module->handle != 0);
    assert(path != NULL);

    int error = ZIP_ER_OK;
    zip_t* archive = zip_open(path, ZIP_RDONLY, &error);
    if (archive == NULL)
    {
        set_error(module, "Could not allocate memory for zip reader");
        return false;
    }

    const zip_int64_t entry_count = zip_get_num_entries(archive, 0);
    for (zip_uint64_t i = 0; (zip_int64_t)i < entry_count; ++i)
    {
        zip_stat_t stat;
        error = zip_stat_index(archive, i, 0, &stat);
        zip_fail_if(error != ZIP_ER_OK);

        const char* entry_path = stat.name;
        static const char* header = "kqtc00";

        if (strlen(entry_path) < strlen(header) ||
                strncmp(entry_path, header, strlen(header)) != 0 ||
                (entry_path[strlen(header)] != '/' &&
                 entry_path[strlen(header)] != '\0'))
        {
            fprintf(stderr, "path is %s\n", path);
            fprintf(stderr, "entry_path is %s\n", entry_path);
            set_error(module, "The file %s contains an invalid data entry: %s",
                    path, entry_path);
            zip_discard(archive);
            return false;
        }

        if (stat.size > (zip_uint64_t)LONG_MAX)
        {
            set_error(module, "Entry %s is too large (%lld bytes)",
                    entry_path, (long long)stat.size);
            zip_discard(archive);
            return false;
        }

        const char* key = strchr(entry_path, '/');
        if (key != NULL && (strlen(key) > 0) && (key[strlen(key) - 1] != '/'))
        {
            ++key;

            zip_file_t* f = zip_fopen_index(archive, i, 0);
            zip_fail_if(f == NULL);

            char* data = malloc(sizeof(char) * stat.size);
            if (data == NULL)
            {
                set_error(module, "Could not allocate memory for module data");
                zip_fclose(f);
                zip_discard(archive);
                return false;
            }

            const zip_int64_t read_count = zip_fread(f, data, stat.size);
            if (read_count < (zip_int64_t)stat.size)
            {
                set_error(module,
                        "Unexpected end of entry %s at %lld bytes"
                        " (expected %lld bytes)",
                        entry_path, (long long)read_count, (long long)stat.size);
                zip_fclose(f);
                zip_discard(archive);
                return false;
            }

            zip_fclose(f);

            if (!kqt_Handle_set_data(module->handle, key, data, (long int)stat.size))
            {
                set_error(module,
                        "Could not set data: %s",
                        kqt_Handle_get_error_message(module->handle));
                free(data);
                zip_discard(archive);
                return false;
            }

            free(data); // TODO: store data for read-only access if needed
        }
    }

    zip_discard(archive);

    if (!kqt_Handle_validate(module->handle))
    {
        set_error(module,
                "Could not validate Kunquat file: %s",
                kqt_Handle_get_error_message(module->handle));
        return false;
    }

    return true;
}

#undef zip_fail_if


kqt_Handle kqtfile_load_module(const char* path)
{
    if (path == NULL)
    {
        set_error(NULL, "path must not be NULL");
        return 0;
    }

    Module* module = MODULE_AUTO;
    if (!Module_init(module))
    {
        return 0;
    }

    if (!Module_load(module, path))
    {
        Module_deinit(module);
        return 0;
    }

    kqt_Handle handle = Module_remove_handle(module);
    Module_deinit(module);

    return handle;
}


