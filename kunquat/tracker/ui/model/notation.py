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

class Notation():

    def __init__(self, data):
        self._name = data['name']
        self._notes = dict(data['note_names'])

    def get_name(self):
        return self._name

    def get_note_name_and_offset(self, cents):
        nearest = self._get_nearest_note(cents)
        if not nearest:
            return None

        c, name = nearest
        return (name, cents - c)

    def _get_nearest_note(self, cents):
        nearest = None
        nearest_dist = float('inf')

        for note in self._notes.iteritems():
            c, name = note
            dist = abs(c - cents)
            if dist < nearest_dist or (dist == nearest_dist and name < nearest[1]):
                nearest = note
                nearest_dist = dist

        return nearest


