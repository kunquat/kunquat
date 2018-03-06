# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .kunquat import get_event_info


"""Kunquat event type descriptions.

"""


EVENT_ARG_BOOL = 'bool'
EVENT_ARG_INT = 'int'
EVENT_ARG_FLOAT = 'float'
EVENT_ARG_TSTAMP = 'tstamp'
EVENT_ARG_STRING = 'string'
EVENT_ARG_PAT = 'pat'
EVENT_ARG_PITCH = 'pitch'
EVENT_ARG_REALTIME = 'realtime'
EVENT_ARG_MAYBE_STRING = 'maybe_string'
EVENT_ARG_MAYBE_REALTIME = 'maybe_realtime'


"""Event information indexed by event name."""
all_events_by_name = get_event_info()


"""Trigger event information indexed by event name."""
trigger_events_by_name = dict(
        e for e in all_events_by_name.items() if not e[0].startswith('A'))


