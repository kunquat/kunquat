

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


#ifndef K_PATTERN_H
#define K_PATTERN_H


#include <stdint.h>

#include <Column.h>
#include <Connections.h>
#include <Channel.h>
#include <Reltime.h>
#include <Event_handler.h>
#include <kunquat/limits.h>


/**
 * This object contains a section of music.
 */
typedef struct Pattern
{
    Column* global;
    Column* aux;
    Column* cols[KQT_COLUMNS_MAX];
    Reltime length;
} Pattern;


#define PATTERN_DEFAULT_LENGTH (Reltime_set(RELTIME_AUTO, 16, 0))


/**
 * Creates a new Pattern object.
 * The caller shall eventually call del_Pattern() to destroy the Pattern
 * returned.
 *
 * \see del_Pattern()
 *
 * \return   The new Pattern object if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern* new_Pattern(void);


/**
 * Parses the header of a Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Pattern_parse_header(Pattern* pat, char* str, Read_state* state);


/**
 * Sets the length of the Pattern.
 *
 * No Events will be deleted if the new length is shorter than the old length.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL and must be
 *                 non-negative.
 */
void Pattern_set_length(Pattern* pat, Reltime* length);


/**
 * Gets the length of the Pattern.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 *
 * \return   The length -- must not be freed.
 */
Reltime* Pattern_get_length(Pattern* pat);


/**
 * Replaces a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 * \param col     The Column -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pattern_set_col(Pattern* pat, int index, Column* col);


/**
 * Returns a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 *
 * \return   The Column.
 */
Column* Pattern_get_col(Pattern* pat, int index);


/**
 * Replaces the global Column of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 * \param col   The Column -- must not be \c NULL.
 */
void Pattern_set_global(Pattern* pat, Column* col);


/**
 * Gets the global Column of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 *
 * \return   The global Column.
 */
Column* Pattern_get_global(Pattern* pat);


/**
 * Mixes a portion of the Pattern. TODO: params
 *
 * \param pat           The Pattern -- must not be \c NULL.
 * \param nframes       The amount of frames to be mixed.
 * \param offset        The mixing buffer offset to be used -- must be
 *                      < \a nframes.
 * \param eh            The Event handler -- must not be \c NULL.
 * \param channels      The Channels -- must not be \c NULL.
 * \param connections   The Connections, or \c NULL if not applicable.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes. A value that is < \a nframes indicates that the
 *           mixing of the Pattern is complete.
 */
uint32_t Pattern_mix(Pattern* pat,
                     uint32_t nframes,
                     uint32_t offset,
                     Event_handler* eh,
                     Channel** channels,
                     Connections* connections);


/**
 * Destroys an existing Pattern.
 *
 * \param pat   The Pattern, or \c NULL.
 */
void del_Pattern(Pattern* pat);


#endif // K_PATTERN_H


