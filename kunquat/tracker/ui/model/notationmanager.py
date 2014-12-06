# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from notation import Notation


class NotationManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._share = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()
        self._updater = controller.get_updater()

    def get_selected_notation_id(self):
        notation_ids = self.get_notation_ids()
        selected_id = self._session.get_selected_notation_id()
        if len(notation_ids) < 1:
            return None
        if not selected_id in notation_ids:
            some_id = sorted(notation_ids)[0]
            return some_id
        return selected_id

    def get_notation(self, notation_id):
        notations = self._share.get_notations()
        return Notation(notations[notation_id])

    def get_selected_notation(self):
        notation_id = self.get_selected_notation_id()
        notation = self.get_notation(notation_id)
        return notation

    def get_notation_ids(self):
        notations = self._share.get_notations()
        notation_ids = notations.keys()
        return notation_ids

    def set_selected_notation_id(self, notation_id):
        self._session.set_selected_notation_id(notation_id)
        self._updater.signal_update(set(['signal_notation']))

