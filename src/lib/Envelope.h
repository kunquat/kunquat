

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


#ifndef K_ENVELOPE_H
#define K_ENVELOPE_H


#include <stdbool.h>
#include <math.h>

#include <File_base.h>
#include <File_tree.h>


#define ENVELOPE_MARKS_MAX (4)


/**
 * The interpolation method used for an Envelope.
 */
typedef enum
{
    // Nearest neighbour.
    ENVELOPE_INT_NEAREST,
    // Linear interpolation.
    ENVELOPE_INT_LINEAR,
    // Sentinel value -- not a valid setting.
    ENVELOPE_INT_LAST
} Envelope_int;


/**
 * Envelope defines a mapping from one type of a value to another. They are
 * used mostly in Instruments.
 */
typedef struct Envelope Envelope;


/**
 * Creates a new Envelope.
 *
 * \param nodes_max   The maximum number of nodes -- must be > \c 1.
 * \param min_x       The minimum value of x -- must not be \c NAN.
 * \param max_x       The maximum value of x -- must not be \c NAN.
 * \param step_x      The step of the x coordinate -- must be >= \c 0.
 * \param min_y       The minimum value of y -- must not be \c NAN.
 * \param max_y       The maximum value of y -- must not be \c NAN.
 * \param step_y      The step of the y coordinate -- must be >= \c 0.
 *
 * \return   The new Envelope if successful, or \c NULL if memory allocation
 *           failed.
 */
Envelope* new_Envelope(int nodes_max,
        double min_x, double max_x, double step_x,
        double min_y, double max_y, double step_y);


/**
 * Parses an Envelope from a string.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The position in the string after the parsing. The caller must
 *           check for errors through \a state.
 */
char* Envelope_read(Envelope* env, char* str, Read_state* state);


/**
 * Gets the number of nodes in the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 *
 * \return   The number of nodes.
 */
int Envelope_node_count(Envelope* env);


/**
 * Sets the interpolation method used in the Envelope.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param interp   The interpolation method -- must be a valid identifier.
 */
void Envelope_set_interp(Envelope* env, Envelope_int interp);


/**
 * Gets the interpolation method used in the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 *
 * \return   The current interpolation method.
 */
Envelope_int Envelope_get_interp(Envelope* env);


/**
 * Sets a mark on the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The mark index -- must be >= \c 0 and
 *                < \c ENVELOPE_MARKS_MAX.
 * \param value   The value of the mark (a negative value unsets the mark).
 */
void Envelope_set_mark(Envelope* env, int index, int value);


/**
 * Gets a mark of the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The mark index -- must be >= \c 0 and
 *                < \c ENVELOPE_MARKS_MAX.
 *
 * \return   The value of the mark, or a negative value if unset.
 */
int Envelope_get_mark(Envelope* env, int index);


/**
 * Sets a node in the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 * \param x     The x coordinate of the node -- must be finite.
 * \param y     The y coordinate of the node -- must be finite.
 *
 * \return   The index of the new node, or \c -1 if the configuration of the
 *           Envelope prevents a node to be inserted here, or \c -2 if memory
 *           allocation failed. Memory allocation will not fail if the
 *           envelope contains 4 nodes or less.
 */
int Envelope_set_node(Envelope* env, double x, double y);


/**
 * Removes a node from the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The index of the node -- must be >= \c 0.
 *
 * \return   \c true if the node was actually removed, otherwise \c false.
 *           The node will not be removed if it doesn't exist or at least
 *           one of its coordinates is locked.
 */
bool Envelope_del_node(Envelope* env, int index);


/**
 * Gets a node from the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The index of the node -- must be >= \c 0.
 *
 * \return   The array containing the x and y coordinates of the node, or
 *           \c NULL if the node doesn't exist.
 */
double* Envelope_get_node(Envelope* env, int index);


/**
 * Moves a node inside the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The index of the node -- must be >= \c 0.
 * \param x       The new x coordinate -- must be finite.
 * \param y       The new y coordinate -- must be finite.
 *
 * \return   The new coordinates of the node, or \c NULL if the node doesn't
 *           exist.
 */
double* Envelope_move_node(Envelope* env, int index, double x, double y);


/**
 * Gets a value from the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 * \param x     The x coordinate -- must be finite.
 *
 * \return   The value of y at the position \a x, or \c NAN if the Envelope
 *           is undefined at \a x.
 */
double Envelope_get_value(Envelope* env, double x);


/**
 * Sets the locking of the first node.
 *
 * The Envelope must contain at least one node.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param lock_x   Whether the x coordinate is locked.
 * \param lock_y   Whether the y coordinate is locked.
 */
void Envelope_set_first_lock(Envelope* env, bool lock_x, bool lock_y);


/**
 * Sets the locking of the last node.
 *
 * The Envelope must contain at least one node.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param lock_x   Whether the x coordinate is locked.
 * \param lock_y   Whether the y coordinate is locked.
 */
void Envelope_set_last_lock(Envelope* env, bool lock_x, bool lock_y);


/**
 * Destroys an existing Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 */
void del_Envelope(Envelope* env);


#endif // K_ENVELOPE_H


