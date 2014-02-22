# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import types


class Trigger():

    def __init__(self, trigger_type, argument):
        assert (argument == None) or (type(argument) == unicode)
        self._type = trigger_type
        self._argument = argument

    def get_type(self):
        return self._type

    def get_argument(self):
        return self._argument

    def get_argument_type(self):
        try:
            return _ARG_TYPES[self._type]
        except KeyError:
            return types.NoneType


# FIXME: use event IDs instead of raw strings as keys
# TODO: figure out how to represent note type
# TODO: this list is incomplete

_Note = 'note'

_ARG_TYPES = {
        'n+': _Note,
        '/p': _Note,
        '.arpn': _Note,
    }


