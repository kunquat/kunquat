

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GEN_TYPE_H
#define K_GEN_TYPE_H


#include <stdint.h>

#include <Generator.h>


typedef Generator* Generator_cons(uint32_t buffer_size, uint32_t mix_rate);


typedef char* Generator_property(Generator* gen, const char* property_type);


typedef struct Gen_type
{
    char* type;
    Generator_cons* cons;
    Generator_property* property;
} Gen_type;


/**
 * Finds a Generator constructor.
 *
 * \param type   The Generator type -- must not be \c NULL.
 *
 * \return   The property function if \a type is supported, otherwise \c NULL.
 */
Generator_cons* Gen_type_find_cons(char* type);


/**
 * Finds a Generator property function.
 *
 * \param type   The Generator type -- must be a valid type.
 *
 * \return   The property function if one exists, otherwise \c NULL.
 */
Generator_property* Gen_type_find_property(char* type);


#endif // K_GEN_TYPE_H


