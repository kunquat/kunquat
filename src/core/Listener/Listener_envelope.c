

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


#include <stdlib.h>
#include <assert.h>

#include "Listener.h"
#include "Listener_envelope.h"


bool env_info(Listener* lr,
        lo_message m,
        char* path,
        int32_t ins_num,
        Envelope* env)
{
    assert(lr != NULL);
    assert(m != NULL);
    assert(path != NULL);
    assert(ins_num >= 0);
    assert(env != NULL);
    int nodes = Envelope_node_count(env);
    lo_message_add_int32(m, nodes);
    for (int i = 0; i < nodes; ++i)
    {
        double* node = Envelope_get_node(env, i);
        assert(node != NULL);
        lo_message_add_double(m, node[0]);
        lo_message_add_double(m, node[1]);
    }
    return true;
}


