# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Instrument():

    def __init__(self, instrument_id):
        assert(instrument_id)
        self._instrument_id = instrument_id
        self._store = None
        self._controller = None
        self._instrument_number = None
        self._existence = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_id(self):
        return self._instrument_id

    def get_existence(self):
        key = '%s/p_manifest.json' % self._instrument_id
        manifest = self._store[key]
        if type(manifest) == type({}):
            return True
        return False

    def get_name(self):
        key = '%s/m_name.json' % self._instrument_id
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name


