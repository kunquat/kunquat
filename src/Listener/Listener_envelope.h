

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


#ifndef K_LISTENER_ENVELOPE_H
#define K_LISTENER_ENVELOPE_H


#include "Listener.h"
#include <Envelope.h>


/**
 * There is no method call specifically for Envelopes. Instead, the following
 * OSC arguments are added into method calls that send Envelope information:
 *
 * \li \c i   Number of nodes.
 * \li        For each node:
 * \li \li \c d   The x coordinate of the node.
 * \li \li \c d   The y coordinate of the node.
 */


/**
 * Adds Envelope information into the message.
 *
 * \param lr        The Listener -- must not be \c NULL.
 * \param m         The message to be sent -- must not be \c NULL.
 * \param path      The OSC method call to indicate the type of the Envelope
 *                  -- must not be \c NULL.
 * \param ins_num   The Instrument number (if applicable, otherwise \c 0).
 * \param env       The Envelope -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool env_info(Listener* lr,
        lo_message m,
        char* path,
        int32_t ins_num,
        Envelope* env);


#endif // K_LISTENER_ENVELOPE_H


