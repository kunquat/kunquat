# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value
from kunquat.kunquat.limits import *
from . import tstamp


class Pattern():

    def __init__(self, pattern_id):
        assert pattern_id
        self._pattern_id = pattern_id
        self._store = None
        self._controller = None
        self._session = None
        self._existence = None

    def __eq__(self, other):
        assert isinstance(other, Pattern)
        return self._pattern_id == other._pattern_id

    def __ne__(self, other):
        return not (self == other)

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller
        self._session = controller.get_session()

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._pattern_id)
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def get_id(self):
        return self._pattern_id

    def get_length(self):
        key = '{}/p_length.json'.format(self._pattern_id)
        return tstamp.Tstamp(self._store.get(key, get_default_value(key)))

    def get_edit_set_length(self, length):
        key = '{}/p_length.json'.format(self._pattern_id)
        transaction = { key: list(length) }
        return transaction

    def get_instance_ids(self):
        instance_ids = set()
        for key in self._store:
            start = '{}/instance_'.format(self._pattern_id)
            if key.startswith(start):
                instance_id = key.split('/')[1]
                manifest_key = '{}/{}/p_manifest.json'.format(
                        self._pattern_id, instance_id)
                if manifest_key in self._store:
                    instance_ids.add(instance_id)
        return instance_ids

    def get_name(self):
        key = '{}/m_name.json'.format(self._pattern_id)
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name

    def get_edit_create_pattern(self):
        key = '{}/p_manifest.json'.format(self._pattern_id)
        edit = { key: {} }
        gp_id = self._session.get_default_grid_pattern_id()
        if gp_id != None:
            edit['{}/i_base_grid.json'.format(self._pattern_id)] = gp_id
        return edit

    def get_edit_remove_pattern(self):
        edit = {}
        start = '{}/'.format(self._pattern_id)
        for key in self._store:
            if key.startswith(start):
                edit[key] = None
        return edit

    def get_edit_set_base_grid_pattern_id(self, gp_id):
        assert (gp_id == None) or isinstance(gp_id, unicode)
        key = '{}/i_base_grid.json'.format(self._pattern_id)
        transaction = { key: gp_id }
        return transaction

    def get_base_grid_pattern_id(self):
        key = '{}/i_base_grid.json'.format(self._pattern_id)
        return self._store.get(key, u'0')

    def get_edit_set_base_grid_pattern_offset(self, offset):
        raise NotImplementedError

    def get_base_grid_pattern_offset(self):
        return tstamp.Tstamp(0) # TODO


