

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ENVELOPE_H
#define KQT_ENVELOPE_H


#include <string/Streader.h>

#include <decl.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


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
    // Smooth curve interpolation.
    ENVELOPE_INT_CURVE,
    // Sentinel value -- not a valid setting.
    ENVELOPE_INT_LAST
} Envelope_int;


/**
 * Create a new Envelope.
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
 * Parse an Envelope from a string.
 *
 * \param env   The Envelope -- must not be \c NULL.
 * \param sr    The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Envelope_read(Envelope* env, Streader* sr);


/**
 * Get the number of nodes in the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 *
 * \return   The number of nodes.
 */
int Envelope_node_count(const Envelope* env);


/**
 * Set the interpolation method used in the Envelope.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param interp   The interpolation method -- must be a valid identifier.
 */
void Envelope_set_interp(Envelope* env, Envelope_int interp);


/**
 * Get the interpolation method used in the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 *
 * \return   The current interpolation method.
 */
Envelope_int Envelope_get_interp(const Envelope* env);


/**
 * Set a mark on the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The mark index -- must be >= \c 0 and
 *                < \c ENVELOPE_MARKS_MAX.
 * \param value   The value of the mark (a negative value unsets the mark).
 */
void Envelope_set_mark(Envelope* env, int index, int value);


/**
 * Get a mark of the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The mark index -- must be >= \c 0 and
 *                < \c ENVELOPE_MARKS_MAX.
 *
 * \return   The value of the mark, or a negative value if unset.
 */
int Envelope_get_mark(const Envelope* env, int index);


/**
 * Set a node in the Envelope.
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
 * Remove a node from the Envelope.
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
 * Get a node from the Envelope.
 *
 * \param env     The Envelope -- must not be \c NULL.
 * \param index   The index of the node -- must be >= \c 0.
 *
 * \return   The array containing the x and y coordinates of the node, or
 *           \c NULL if the node doesn't exist.
 */
double* Envelope_get_node(const Envelope* env, int index);


/**
 * Move a node inside the Envelope.
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
 * Get a value from the Envelope.
 *
 * \param env   The Envelope -- must not be \c NULL.
 * \param x     The x coordinate -- must be finite.
 *
 * \return   The value of y at the position \a x, or \c NAN if the Envelope
 *           is undefined at \a x.
 */
double Envelope_get_value(const Envelope* env, double x);


/**
 * Set the locking of the first node.
 *
 * The Envelope must contain at least one node.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param lock_x   Whether the x coordinate is locked.
 * \param lock_y   Whether the y coordinate is locked.
 */
void Envelope_set_first_lock(Envelope* env, bool lock_x, bool lock_y);


/**
 * Set the locking of the last node.
 *
 * The Envelope must contain at least one node.
 *
 * \param env      The Envelope -- must not be \c NULL.
 * \param lock_x   Whether the x coordinate is locked.
 * \param lock_y   Whether the y coordinate is locked.
 */
void Envelope_set_last_lock(Envelope* env, bool lock_x, bool lock_y);


/**
 * Destroy an existing Envelope.
 *
 * \param env   The Envelope, or \c NULL.
 */
void del_Envelope(Envelope* env);


#endif // KQT_ENVELOPE_H


