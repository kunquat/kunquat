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

from kunquat.kunquat.kunquat import get_default_value


class Generator():

    def __init__(self, ins_id, gen_id):
        assert ins_id
        assert gen_id
        self._ins_id = ins_id
        self._gen_id = gen_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_key(self, subkey):
        return '{}/{}/{}'.format(self._ins_id, self._gen_id, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)


